/*
 * Copyright (c) 2015-2020 The Khronos Group Inc.
 * Copyright (c) 2015-2020 Valve Corporation
 * Copyright (c) 2015-2020 LunarG, Inc.
 * Copyright (c) 2015-2020 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Author: Chia-I Wu <olvaffe@gmail.com>
 * Author: Chris Forbes <chrisf@ijw.co.nz>
 * Author: Courtney Goeltzenleuchter <courtney@LunarG.com>
 * Author: Mark Lobodzinski <mark@lunarg.com>
 * Author: Mike Stroyan <mike@LunarG.com>
 * Author: Tobin Ehlis <tobine@google.com>
 * Author: Tony Barbour <tony@LunarG.com>
 * Author: Cody Northrop <cnorthrop@google.com>
 * Author: Dave Houlton <daveh@lunarg.com>
 * Author: Jeremy Kniager <jeremyk@lunarg.com>
 * Author: Shannon McPherson <shannon@lunarg.com>
 * Author: John Zulauf <jzulauf@lunarg.com>
 */
#include "cast_utils.h"
#include "layer_validation_tests.h"

// Global list of sType,size identifiers
std::vector<std::pair<uint32_t, uint32_t>> custom_stype_info{};

VkFormat FindSupportedDepthOnlyFormat(VkPhysicalDevice phy) {
    const VkFormat ds_formats[] = {VK_FORMAT_D16_UNORM, VK_FORMAT_X8_D24_UNORM_PACK32, VK_FORMAT_D32_SFLOAT};
    for (uint32_t i = 0; i < size(ds_formats); ++i) {
        VkFormatProperties format_props;
        vk::GetPhysicalDeviceFormatProperties(phy, ds_formats[i], &format_props);

        if (format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            return ds_formats[i];
        }
    }
    return VK_FORMAT_UNDEFINED;
}

VkFormat FindSupportedStencilOnlyFormat(VkPhysicalDevice phy) {
    const VkFormat ds_formats[] = {VK_FORMAT_S8_UINT};
    for (uint32_t i = 0; i < size(ds_formats); ++i) {
        VkFormatProperties format_props;
        vk::GetPhysicalDeviceFormatProperties(phy, ds_formats[i], &format_props);

        if (format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            return ds_formats[i];
        }
    }
    return VK_FORMAT_UNDEFINED;
}

VkFormat FindSupportedDepthStencilFormat(VkPhysicalDevice phy) {
    const VkFormat ds_formats[] = {VK_FORMAT_D16_UNORM_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT_S8_UINT};
    for (uint32_t i = 0; i < size(ds_formats); ++i) {
        VkFormatProperties format_props;
        vk::GetPhysicalDeviceFormatProperties(phy, ds_formats[i], &format_props);

        if (format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            return ds_formats[i];
        }
    }
    return VK_FORMAT_UNDEFINED;
}

bool ImageFormatIsSupported(VkPhysicalDevice phy, VkFormat format, VkImageTiling tiling, VkFormatFeatureFlags features) {
    VkFormatProperties format_props;
    vk::GetPhysicalDeviceFormatProperties(phy, format, &format_props);
    VkFormatFeatureFlags phy_features =
        (VK_IMAGE_TILING_OPTIMAL == tiling ? format_props.optimalTilingFeatures : format_props.linearTilingFeatures);
    return (0 != (phy_features & features));
}

bool ImageFormatAndFeaturesSupported(VkPhysicalDevice phy, VkFormat format, VkImageTiling tiling, VkFormatFeatureFlags features) {
    VkFormatProperties format_props;
    vk::GetPhysicalDeviceFormatProperties(phy, format, &format_props);
    VkFormatFeatureFlags phy_features =
        (VK_IMAGE_TILING_OPTIMAL == tiling ? format_props.optimalTilingFeatures : format_props.linearTilingFeatures);
    return (features == (phy_features & features));
}

bool ImageFormatAndFeaturesSupported(const VkInstance inst, const VkPhysicalDevice phy, const VkImageCreateInfo info,
                                     const VkFormatFeatureFlags features) {
    // Verify physical device support of format features
    if (!ImageFormatAndFeaturesSupported(phy, info.format, info.tiling, features)) {
        return false;
    }

    // Verify that PhysDevImageFormatProp() also claims support for the specific usage
    VkImageFormatProperties props;
    VkResult err =
        vk::GetPhysicalDeviceImageFormatProperties(phy, info.format, info.imageType, info.tiling, info.usage, info.flags, &props);
    if (VK_SUCCESS != err) {
        return false;
    }

#if 0  // Convinced this chunk doesn't currently add any additional info, but leaving in place because it may be
       // necessary with future extensions

    // Verify again using version 2, if supported, which *can* return more property data than the original...
    // (It's not clear that this is any more definitive than using the original version - but no harm)
    PFN_vkGetPhysicalDeviceImageFormatProperties2KHR p_GetPDIFP2KHR =
        (PFN_vkGetPhysicalDeviceImageFormatProperties2KHR)vk::GetInstanceProcAddr(inst,
                                                                                "vkGetPhysicalDeviceImageFormatProperties2KHR");
    if (NULL != p_GetPDIFP2KHR) {
        VkPhysicalDeviceImageFormatInfo2KHR fmt_info{};
        fmt_info.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2_KHR;
        fmt_info.pNext = nullptr;
        fmt_info.format = info.format;
        fmt_info.type = info.imageType;
        fmt_info.tiling = info.tiling;
        fmt_info.usage = info.usage;
        fmt_info.flags = info.flags;

        VkImageFormatProperties2KHR fmt_props = {};
        fmt_props.sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2_KHR;
        err = p_GetPDIFP2KHR(phy, &fmt_info, &fmt_props);
        if (VK_SUCCESS != err) {
            return false;
        }
    }
#endif

    return true;
}

bool BufferFormatAndFeaturesSupported(VkPhysicalDevice phy, VkFormat format, VkFormatFeatureFlags features) {
    VkFormatProperties format_props;
    vk::GetPhysicalDeviceFormatProperties(phy, format, &format_props);
    VkFormatFeatureFlags phy_features = format_props.bufferFeatures;
    return (features == (phy_features & features));
}

VkPhysicalDevicePushDescriptorPropertiesKHR GetPushDescriptorProperties(VkInstance instance, VkPhysicalDevice gpu) {
    // Find address of extension call and make the call -- assumes needed extensions are enabled.
    PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2KHR =
        (PFN_vkGetPhysicalDeviceProperties2KHR)vk::GetInstanceProcAddr(instance, "vkGetPhysicalDeviceProperties2KHR");
    assert(vkGetPhysicalDeviceProperties2KHR != nullptr);

    // Get the push descriptor limits
    auto push_descriptor_prop = lvl_init_struct<VkPhysicalDevicePushDescriptorPropertiesKHR>();
    auto prop2 = lvl_init_struct<VkPhysicalDeviceProperties2KHR>(&push_descriptor_prop);
    vkGetPhysicalDeviceProperties2KHR(gpu, &prop2);
    return push_descriptor_prop;
}

VkPhysicalDeviceSubgroupProperties GetSubgroupProperties(VkInstance instance, VkPhysicalDevice gpu) {
    auto subgroup_prop = lvl_init_struct<VkPhysicalDeviceSubgroupProperties>();

    auto prop2 = lvl_init_struct<VkPhysicalDeviceProperties2>(&subgroup_prop);
    vk::GetPhysicalDeviceProperties2(gpu, &prop2);
    return subgroup_prop;
}

VkPhysicalDeviceDescriptorIndexingProperties GetDescriptorIndexingProperties(VkInstance instance, VkPhysicalDevice gpu) {
    auto descriptor_indexing_prop = lvl_init_struct<VkPhysicalDeviceDescriptorIndexingProperties>();

    auto prop2 = lvl_init_struct<VkPhysicalDeviceProperties2>(&descriptor_indexing_prop);
    vk::GetPhysicalDeviceProperties2(gpu, &prop2);
    return descriptor_indexing_prop;
}

bool operator==(const VkDebugUtilsLabelEXT &rhs, const VkDebugUtilsLabelEXT &lhs) {
    bool is_equal = (rhs.color[0] == lhs.color[0]) && (rhs.color[1] == lhs.color[1]) && (rhs.color[2] == lhs.color[2]) &&
                    (rhs.color[3] == lhs.color[3]);
    if (is_equal) {
        if (rhs.pLabelName && lhs.pLabelName) {
            is_equal = (0 == strcmp(rhs.pLabelName, lhs.pLabelName));
        } else {
            is_equal = (rhs.pLabelName == nullptr) && (lhs.pLabelName == nullptr);
        }
    }
    return is_equal;
}

VKAPI_ATTR VkBool32 VKAPI_CALL DebugUtilsCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                  VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                                  const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData) {
    auto *data = reinterpret_cast<DebugUtilsLabelCheckData *>(pUserData);
    data->callback(pCallbackData, data);
    return VK_FALSE;
}

#if GTEST_IS_THREADSAFE
extern "C" void *AddToCommandBuffer(void *arg) {
    struct thread_data_struct *data = (struct thread_data_struct *)arg;

    for (int i = 0; i < 80000; i++) {
        vk::CmdSetEvent(data->commandBuffer, data->event, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
        if (*data->bailout) {
            break;
        }
    }
    return NULL;
}

extern "C" void *UpdateDescriptor(void *arg) {
    struct thread_data_struct *data = (struct thread_data_struct *)arg;

    VkDescriptorBufferInfo buffer_info = {};
    buffer_info.buffer = data->buffer;
    buffer_info.offset = 0;
    buffer_info.range = 1;

    VkWriteDescriptorSet descriptor_write;
    memset(&descriptor_write, 0, sizeof(descriptor_write));
    descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_write.dstSet = data->descriptorSet;
    descriptor_write.dstBinding = data->binding;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_write.pBufferInfo = &buffer_info;

    for (int i = 0; i < 80000; i++) {
        vk::UpdateDescriptorSets(data->device, 1, &descriptor_write, 0, NULL);
        if (*data->bailout) {
            break;
        }
    }
    return NULL;
}

#endif  // GTEST_IS_THREADSAFE

extern "C" void *ReleaseNullFence(void *arg) {
    struct thread_data_struct *data = (struct thread_data_struct *)arg;

    for (int i = 0; i < 40000; i++) {
        vk::DestroyFence(data->device, VK_NULL_HANDLE, NULL);
        if (*data->bailout) {
            break;
        }
    }
    return NULL;
}

void TestRenderPassCreate(ErrorMonitor *error_monitor, const VkDevice device, const VkRenderPassCreateInfo *create_info,
                          bool rp2_supported, const char *rp1_vuid, const char *rp2_vuid) {
    VkRenderPass render_pass = VK_NULL_HANDLE;
    VkResult err;

    if (rp1_vuid) {
        // Some tests mismatch attachment type with layout
        error_monitor->SetUnexpectedError("VUID-VkSubpassDescription-None-04437");

        error_monitor->SetDesiredFailureMsg(kErrorBit, rp1_vuid);
        err = vk::CreateRenderPass(device, create_info, nullptr, &render_pass);
        if (err == VK_SUCCESS) vk::DestroyRenderPass(device, render_pass, nullptr);
        error_monitor->VerifyFound();
    }

    if (rp2_supported && rp2_vuid) {
        PFN_vkCreateRenderPass2KHR vkCreateRenderPass2KHR =
            (PFN_vkCreateRenderPass2KHR)vk::GetDeviceProcAddr(device, "vkCreateRenderPass2KHR");
        safe_VkRenderPassCreateInfo2 create_info2;
        ConvertVkRenderPassCreateInfoToV2KHR(*create_info, &create_info2);

        // aspectMasks might never get set in ConvertVkRenderPassCreateInfoToV2KHR
        error_monitor->SetUnexpectedError("VUID-VkAttachmentReference2-attachment-03311");
        error_monitor->SetUnexpectedError("VUID-VkAttachmentReference2-attachment-03312");
        // Some tests mismatch attachment type with layout
        error_monitor->SetUnexpectedError("VUID-VkSubpassDescription2-None-04439");

        error_monitor->SetDesiredFailureMsg(kErrorBit, rp2_vuid);
        err = vkCreateRenderPass2KHR(device, create_info2.ptr(), nullptr, &render_pass);
        if (err == VK_SUCCESS) vk::DestroyRenderPass(device, render_pass, nullptr);
        error_monitor->VerifyFound();

        // For api version >= 1.2, try core entrypoint
        PFN_vkCreateRenderPass2 vkCreateRenderPass2 = (PFN_vkCreateRenderPass2)vk::GetDeviceProcAddr(device, "vkCreateRenderPass2");
        if (vkCreateRenderPass2) {
            // aspectMasks might never get set in ConvertVkRenderPassCreateInfoToV2KHR
            error_monitor->SetUnexpectedError("VUID-VkAttachmentReference2-attachment-03311");
            error_monitor->SetUnexpectedError("VUID-VkAttachmentReference2-attachment-03312");
            // Some tests mismatch attachment type with layout
            error_monitor->SetUnexpectedError("VUID-VkSubpassDescription2-None-04439");

            error_monitor->SetDesiredFailureMsg(kErrorBit, rp2_vuid);
            err = vkCreateRenderPass2(device, create_info2.ptr(), nullptr, &render_pass);
            if (err == VK_SUCCESS) vk::DestroyRenderPass(device, render_pass, nullptr);
            error_monitor->VerifyFound();
        }
    }
}

void PositiveTestRenderPassCreate(ErrorMonitor *error_monitor, const VkDevice device, const VkRenderPassCreateInfo *create_info,
                                  bool rp2_supported) {
    VkRenderPass render_pass = VK_NULL_HANDLE;
    VkResult err;

    error_monitor->ExpectSuccess();
    err = vk::CreateRenderPass(device, create_info, nullptr, &render_pass);
    if (err == VK_SUCCESS) vk::DestroyRenderPass(device, render_pass, nullptr);
    error_monitor->VerifyNotFound();

    if (rp2_supported) {
        PFN_vkCreateRenderPass2KHR vkCreateRenderPass2KHR =
            (PFN_vkCreateRenderPass2KHR)vk::GetDeviceProcAddr(device, "vkCreateRenderPass2KHR");
        safe_VkRenderPassCreateInfo2 create_info2;
        ConvertVkRenderPassCreateInfoToV2KHR(*create_info, &create_info2);

        error_monitor->ExpectSuccess();
        err = vkCreateRenderPass2KHR(device, create_info2.ptr(), nullptr, &render_pass);
        if (err == VK_SUCCESS) vk::DestroyRenderPass(device, render_pass, nullptr);
        error_monitor->VerifyNotFound();
    }
}

void TestRenderPass2KHRCreate(ErrorMonitor *error_monitor, const VkDevice device, const VkRenderPassCreateInfo2KHR *create_info,
                              const char *rp2_vuid) {
    VkRenderPass render_pass = VK_NULL_HANDLE;
    VkResult err;
    PFN_vkCreateRenderPass2KHR vkCreateRenderPass2KHR =
        (PFN_vkCreateRenderPass2KHR)vk::GetDeviceProcAddr(device, "vkCreateRenderPass2KHR");

    error_monitor->SetDesiredFailureMsg(kErrorBit, rp2_vuid);
    err = vkCreateRenderPass2KHR(device, create_info, nullptr, &render_pass);
    if (err == VK_SUCCESS) vk::DestroyRenderPass(device, render_pass, nullptr);
    error_monitor->VerifyFound();
}

void TestRenderPassBegin(ErrorMonitor *error_monitor, const VkDevice device, const VkCommandBuffer command_buffer,
                         const VkRenderPassBeginInfo *begin_info, bool rp2Supported, const char *rp1_vuid, const char *rp2_vuid) {
    VkCommandBufferBeginInfo cmd_begin_info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr,
                                               VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr};

    if (rp1_vuid) {
        vk::BeginCommandBuffer(command_buffer, &cmd_begin_info);
        error_monitor->SetDesiredFailureMsg(kErrorBit, rp1_vuid);
        vk::CmdBeginRenderPass(command_buffer, begin_info, VK_SUBPASS_CONTENTS_INLINE);
        error_monitor->VerifyFound();
        vk::ResetCommandBuffer(command_buffer, 0);
    }
    if (rp2Supported && rp2_vuid) {
        PFN_vkCmdBeginRenderPass2KHR vkCmdBeginRenderPass2KHR =
            (PFN_vkCmdBeginRenderPass2KHR)vk::GetDeviceProcAddr(device, "vkCmdBeginRenderPass2KHR");
        VkSubpassBeginInfoKHR subpass_begin_info = {VK_STRUCTURE_TYPE_SUBPASS_BEGIN_INFO_KHR, nullptr, VK_SUBPASS_CONTENTS_INLINE};
        vk::BeginCommandBuffer(command_buffer, &cmd_begin_info);
        error_monitor->SetDesiredFailureMsg(kErrorBit, rp2_vuid);
        vkCmdBeginRenderPass2KHR(command_buffer, begin_info, &subpass_begin_info);
        error_monitor->VerifyFound();
        vk::ResetCommandBuffer(command_buffer, 0);

        // For api version >= 1.2, try core entrypoint
        PFN_vkCmdBeginRenderPass2KHR vkCmdBeginRenderPass2 =
            (PFN_vkCmdBeginRenderPass2KHR)vk::GetDeviceProcAddr(device, "vkCmdBeginRenderPass2");
        if (vkCmdBeginRenderPass2) {
            vk::BeginCommandBuffer(command_buffer, &cmd_begin_info);
            error_monitor->SetDesiredFailureMsg(kErrorBit, rp2_vuid);
            vkCmdBeginRenderPass2(command_buffer, begin_info, &subpass_begin_info);
            error_monitor->VerifyFound();
            vk::ResetCommandBuffer(command_buffer, 0);
        }
    }
}

void ValidOwnershipTransferOp(ErrorMonitor *monitor, VkCommandBufferObj *cb, VkPipelineStageFlags src_stages,
                              VkPipelineStageFlags dst_stages, const VkBufferMemoryBarrier *buf_barrier,
                              const VkImageMemoryBarrier *img_barrier) {
    monitor->ExpectSuccess();
    cb->begin();
    uint32_t num_buf_barrier = (buf_barrier) ? 1 : 0;
    uint32_t num_img_barrier = (img_barrier) ? 1 : 0;
    cb->PipelineBarrier(src_stages, dst_stages, 0, 0, nullptr, num_buf_barrier, buf_barrier, num_img_barrier, img_barrier);
    cb->end();
    cb->QueueCommandBuffer();  // Implicitly waits
    monitor->VerifyNotFound();
}

void ValidOwnershipTransfer(ErrorMonitor *monitor, VkCommandBufferObj *cb_from, VkCommandBufferObj *cb_to,
                            VkPipelineStageFlags src_stages, VkPipelineStageFlags dst_stages,
                            const VkBufferMemoryBarrier *buf_barrier, const VkImageMemoryBarrier *img_barrier) {
    ValidOwnershipTransferOp(monitor, cb_from, src_stages, dst_stages, buf_barrier, img_barrier);
    ValidOwnershipTransferOp(monitor, cb_to, src_stages, dst_stages, buf_barrier, img_barrier);
}

VkResult GPDIFPHelper(VkPhysicalDevice dev, const VkImageCreateInfo *ci, VkImageFormatProperties *limits) {
    VkImageFormatProperties tmp_limits;
    limits = limits ? limits : &tmp_limits;
    return vk::GetPhysicalDeviceImageFormatProperties(dev, ci->format, ci->imageType, ci->tiling, ci->usage, ci->flags, limits);
}

VkFormat FindFormatLinearWithoutMips(VkPhysicalDevice gpu, VkImageCreateInfo image_ci) {
    image_ci.tiling = VK_IMAGE_TILING_LINEAR;

    const VkFormat first_vk_format = static_cast<VkFormat>(1);
    const VkFormat last_vk_format = static_cast<VkFormat>(130);  // avoid compressed/feature protected, otherwise 184

    for (VkFormat format = first_vk_format; format <= last_vk_format; format = static_cast<VkFormat>(format + 1)) {
        image_ci.format = format;

        // WORKAROUND for dev_sim and mock_icd not containing valid format limits yet
        VkFormatProperties format_props;
        vk::GetPhysicalDeviceFormatProperties(gpu, format, &format_props);
        const VkFormatFeatureFlags core_filter = 0x1FFF;
        const auto features = (image_ci.tiling == VK_IMAGE_TILING_LINEAR) ? format_props.linearTilingFeatures & core_filter
                                                                          : format_props.optimalTilingFeatures & core_filter;
        if (!(features & core_filter)) continue;

        VkImageFormatProperties img_limits;
        if (VK_SUCCESS == GPDIFPHelper(gpu, &image_ci, &img_limits) && img_limits.maxMipLevels == 1) return format;
    }

    return VK_FORMAT_UNDEFINED;
}

bool FindFormatWithoutSamples(VkPhysicalDevice gpu, VkImageCreateInfo &image_ci) {
    const VkFormat first_vk_format = static_cast<VkFormat>(1);
    const VkFormat last_vk_format = static_cast<VkFormat>(130);  // avoid compressed/feature protected, otherwise 184

    for (VkFormat format = first_vk_format; format <= last_vk_format; format = static_cast<VkFormat>(format + 1)) {
        image_ci.format = format;

        // WORKAROUND for dev_sim and mock_icd not containing valid format limits yet
        VkFormatProperties format_props;
        vk::GetPhysicalDeviceFormatProperties(gpu, format, &format_props);
        const VkFormatFeatureFlags core_filter = 0x1FFF;
        const auto features = (image_ci.tiling == VK_IMAGE_TILING_LINEAR) ? format_props.linearTilingFeatures & core_filter
                                                                          : format_props.optimalTilingFeatures & core_filter;
        if (!(features & core_filter)) continue;

        for (VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_64_BIT; samples > 0;
             samples = static_cast<VkSampleCountFlagBits>(samples >> 1)) {
            image_ci.samples = samples;
            VkImageFormatProperties img_limits;
            if (VK_SUCCESS == GPDIFPHelper(gpu, &image_ci, &img_limits) && !(img_limits.sampleCounts & samples)) return true;
        }
    }

    return false;
}

bool FindUnsupportedImage(VkPhysicalDevice gpu, VkImageCreateInfo &image_ci) {
    const VkFormat first_vk_format = static_cast<VkFormat>(1);
    const VkFormat last_vk_format = static_cast<VkFormat>(130);  // avoid compressed/feature protected, otherwise 184

    const std::vector<VkImageTiling> tilings = {VK_IMAGE_TILING_LINEAR, VK_IMAGE_TILING_OPTIMAL};
    for (const auto tiling : tilings) {
        image_ci.tiling = tiling;

        for (VkFormat format = first_vk_format; format <= last_vk_format; format = static_cast<VkFormat>(format + 1)) {
            image_ci.format = format;

            VkFormatProperties format_props;
            vk::GetPhysicalDeviceFormatProperties(gpu, format, &format_props);

            const VkFormatFeatureFlags core_filter = 0x1FFF;
            const auto features = (tiling == VK_IMAGE_TILING_LINEAR) ? format_props.linearTilingFeatures & core_filter
                                                                     : format_props.optimalTilingFeatures & core_filter;
            if (!(features & core_filter)) continue;  // We wand supported by features, but not by ImageFormatProperties

            // get as many usage flags as possible
            image_ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
            if (features & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) image_ci.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
            if (features & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT) image_ci.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
            if (features & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT) image_ci.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            if (features & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
                image_ci.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

            VkImageFormatProperties img_limits;
            if (VK_ERROR_FORMAT_NOT_SUPPORTED == GPDIFPHelper(gpu, &image_ci, &img_limits)) {
                return true;
            }
        }
    }

    return false;
}

VkFormat FindFormatWithoutFeatures(VkPhysicalDevice gpu, VkImageTiling tiling, VkFormatFeatureFlags undesired_features) {
    const VkFormat first_vk_format = static_cast<VkFormat>(1);
    const VkFormat last_vk_format = static_cast<VkFormat>(130);  // avoid compressed/feature protected, otherwise 184

    for (VkFormat format = first_vk_format; format <= last_vk_format; format = static_cast<VkFormat>(format + 1)) {
        VkFormatProperties format_props;
        vk::GetPhysicalDeviceFormatProperties(gpu, format, &format_props);

        const VkFormatFeatureFlags core_filter = 0x1FFF;
        const auto features = (tiling == VK_IMAGE_TILING_LINEAR) ? format_props.linearTilingFeatures & core_filter
                                                                 : format_props.optimalTilingFeatures & core_filter;

        const auto valid_features = features & core_filter;
        if (undesired_features == UINT32_MAX) {
            if (!valid_features) return format;
        } else {
            if (valid_features && !(valid_features & undesired_features)) return format;
        }
    }

    return VK_FORMAT_UNDEFINED;
}

void AllocateDisjointMemory(VkDeviceObj *device, PFN_vkGetImageMemoryRequirements2KHR fp, VkImage mp_image,
                            VkDeviceMemory *mp_image_mem, VkImageAspectFlagBits plane) {
    VkImagePlaneMemoryRequirementsInfo image_plane_req = {};
    image_plane_req.sType = VK_STRUCTURE_TYPE_IMAGE_PLANE_MEMORY_REQUIREMENTS_INFO;
    image_plane_req.pNext = nullptr;
    image_plane_req.planeAspect = plane;

    VkImageMemoryRequirementsInfo2 mem_req_info2 = {};
    mem_req_info2.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2;
    mem_req_info2.pNext = (void *)&image_plane_req;
    mem_req_info2.image = mp_image;

    VkMemoryRequirements2 mp_image_mem_reqs2 = {};
    mp_image_mem_reqs2.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
    mp_image_mem_reqs2.pNext = nullptr;

    fp(device->device(), &mem_req_info2, &mp_image_mem_reqs2);

    VkMemoryAllocateInfo mp_image_alloc_info;
    mp_image_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mp_image_alloc_info.pNext = nullptr;
    mp_image_alloc_info.allocationSize = mp_image_mem_reqs2.memoryRequirements.size;
    ASSERT_TRUE(device->phy().set_memory_type(mp_image_mem_reqs2.memoryRequirements.memoryTypeBits, &mp_image_alloc_info, 0));
    ASSERT_VK_SUCCESS(vk::AllocateMemory(device->device(), &mp_image_alloc_info, NULL, mp_image_mem));
}

void NegHeightViewportTests(VkDeviceObj *m_device, VkCommandBufferObj *m_commandBuffer, ErrorMonitor *m_errorMonitor) {
    const auto &limits = m_device->props.limits;

    m_commandBuffer->begin();

    using std::vector;
    struct TestCase {
        VkViewport vp;
        vector<std::string> vuids;
    };

    // not necessarily boundary values (unspecified cast rounding), but guaranteed to be over limit
    const auto one_before_min_h = NearestSmaller(-static_cast<float>(limits.maxViewportDimensions[1]));
    const auto one_past_max_h = NearestGreater(static_cast<float>(limits.maxViewportDimensions[1]));

    const auto min_bound = limits.viewportBoundsRange[0];
    const auto max_bound = limits.viewportBoundsRange[1];
    const auto one_before_min_bound = NearestSmaller(min_bound);
    const auto one_past_max_bound = NearestGreater(max_bound);

    const vector<TestCase> test_cases = {{{0.0, 0.0, 64.0, one_before_min_h, 0.0, 1.0}, {"VUID-VkViewport-height-01773"}},
                                         {{0.0, 0.0, 64.0, one_past_max_h, 0.0, 1.0}, {"VUID-VkViewport-height-01773"}},
                                         {{0.0, 0.0, 64.0, NAN, 0.0, 1.0}, {"VUID-VkViewport-height-01773"}},
                                         {{0.0, one_before_min_bound, 64.0, 1.0, 0.0, 1.0}, {"VUID-VkViewport-y-01775"}},
                                         {{0.0, one_past_max_bound, 64.0, -1.0, 0.0, 1.0}, {"VUID-VkViewport-y-01776"}},
                                         {{0.0, min_bound, 64.0, -1.0, 0.0, 1.0}, {"VUID-VkViewport-y-01777"}},
                                         {{0.0, max_bound, 64.0, 1.0, 0.0, 1.0}, {"VUID-VkViewport-y-01233"}}};

    for (const auto &test_case : test_cases) {
        for (const auto vuid : test_case.vuids) {
            if (vuid == "VUID-Undefined")
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "is less than VkPhysicalDeviceLimits::viewportBoundsRange[0]");
            else
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, vuid);
        }
        vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &test_case.vp);
        m_errorMonitor->VerifyFound();
    }
}

void CreateSamplerTest(VkLayerTest &test, const VkSamplerCreateInfo *pCreateInfo, std::string code) {
    VkResult err;
    VkSampler sampler = VK_NULL_HANDLE;
    if (code.length())
        test.Monitor().SetDesiredFailureMsg(kErrorBit | kWarningBit, code);
    else
        test.Monitor().ExpectSuccess();

    err = vk::CreateSampler(test.device(), pCreateInfo, NULL, &sampler);
    if (code.length())
        test.Monitor().VerifyFound();
    else
        test.Monitor().VerifyNotFound();

    if (VK_SUCCESS == err) {
        vk::DestroySampler(test.device(), sampler, NULL);
    }
}

void CreateBufferTest(VkLayerTest &test, const VkBufferCreateInfo *pCreateInfo, std::string code) {
    VkResult err;
    VkBuffer buffer = VK_NULL_HANDLE;
    if (code.length())
        test.Monitor().SetDesiredFailureMsg(kErrorBit, code);
    else
        test.Monitor().ExpectSuccess();

    err = vk::CreateBuffer(test.device(), pCreateInfo, NULL, &buffer);
    if (code.length())
        test.Monitor().VerifyFound();
    else
        test.Monitor().VerifyNotFound();

    if (VK_SUCCESS == err) {
        vk::DestroyBuffer(test.device(), buffer, NULL);
    }
}

void CreateImageTest(VkLayerTest &test, const VkImageCreateInfo *pCreateInfo, std::string code) {
    VkResult err;
    VkImage image = VK_NULL_HANDLE;
    if (code.length()) {
        test.Monitor().SetDesiredFailureMsg(kErrorBit, code);
        // Very possible a test didn't check for VK_ERROR_FORMAT_NOT_SUPPORTED
        test.Monitor().SetUnexpectedError("UNASSIGNED-CoreValidation-Image-FormatNotSupported");
    } else {
        test.Monitor().ExpectSuccess();
    }

    err = vk::CreateImage(test.device(), pCreateInfo, NULL, &image);
    if (code.length())
        test.Monitor().VerifyFound();
    else
        test.Monitor().VerifyNotFound();

    if (VK_SUCCESS == err) {
        vk::DestroyImage(test.device(), image, NULL);
    }
}

void CreateBufferViewTest(VkLayerTest &test, const VkBufferViewCreateInfo *pCreateInfo, const std::vector<std::string> &codes) {
    VkResult err;
    VkBufferView view = VK_NULL_HANDLE;
    if (codes.size())
        std::for_each(codes.begin(), codes.end(), [&](const std::string &s) { test.Monitor().SetDesiredFailureMsg(kErrorBit, s); });
    else
        test.Monitor().ExpectSuccess();

    err = vk::CreateBufferView(test.device(), pCreateInfo, NULL, &view);
    if (codes.size())
        test.Monitor().VerifyFound();
    else
        test.Monitor().VerifyNotFound();

    if (VK_SUCCESS == err) {
        vk::DestroyBufferView(test.device(), view, NULL);
    }
}

void CreateImageViewTest(VkLayerTest &test, const VkImageViewCreateInfo *pCreateInfo, std::string code) {
    VkResult err;
    VkImageView view = VK_NULL_HANDLE;
    if (code.length())
        test.Monitor().SetDesiredFailureMsg(kErrorBit, code);
    else
        test.Monitor().ExpectSuccess();

    err = vk::CreateImageView(test.device(), pCreateInfo, NULL, &view);
    if (code.length())
        test.Monitor().VerifyFound();
    else
        test.Monitor().VerifyNotFound();

    if (VK_SUCCESS == err) {
        vk::DestroyImageView(test.device(), view, NULL);
    }
}

VkSamplerCreateInfo SafeSaneSamplerCreateInfo() {
    VkSamplerCreateInfo sampler_create_info = {};
    sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_create_info.pNext = nullptr;
    sampler_create_info.magFilter = VK_FILTER_NEAREST;
    sampler_create_info.minFilter = VK_FILTER_NEAREST;
    sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    sampler_create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_create_info.mipLodBias = 0.0;
    sampler_create_info.anisotropyEnable = VK_FALSE;
    sampler_create_info.maxAnisotropy = 1.0;
    sampler_create_info.compareEnable = VK_FALSE;
    sampler_create_info.compareOp = VK_COMPARE_OP_NEVER;
    sampler_create_info.minLod = 0.0;
    sampler_create_info.maxLod = 16.0;
    sampler_create_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    sampler_create_info.unnormalizedCoordinates = VK_FALSE;

    return sampler_create_info;
}

VkImageViewCreateInfo SafeSaneImageViewCreateInfo(VkImage image, VkFormat format, VkImageAspectFlags aspect_mask) {
    VkImageViewCreateInfo image_view_create_info = {};
    image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    image_view_create_info.image = image;
    image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_create_info.format = format;
    image_view_create_info.subresourceRange.layerCount = 1;
    image_view_create_info.subresourceRange.baseMipLevel = 0;
    image_view_create_info.subresourceRange.levelCount = 1;
    image_view_create_info.subresourceRange.aspectMask = aspect_mask;

    return image_view_create_info;
}

VkImageViewCreateInfo SafeSaneImageViewCreateInfo(const VkImageObj &image, VkFormat format, VkImageAspectFlags aspect_mask) {
    return SafeSaneImageViewCreateInfo(image.handle(), format, aspect_mask);
}

bool CheckCreateRenderPass2Support(VkRenderFramework *renderFramework, std::vector<const char *> &device_extension_names) {
    if (renderFramework->DeviceExtensionSupported(renderFramework->gpu(), nullptr, VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME)) {
        device_extension_names.push_back(VK_KHR_MULTIVIEW_EXTENSION_NAME);
        device_extension_names.push_back(VK_KHR_MAINTENANCE2_EXTENSION_NAME);
        device_extension_names.push_back(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
        return true;
    }
    return false;
}

bool CheckDescriptorIndexingSupportAndInitFramework(VkRenderFramework *renderFramework,
                                                    std::vector<const char *> &instance_extension_names,
                                                    std::vector<const char *> &device_extension_names,
                                                    VkValidationFeaturesEXT *features, void *userData) {
    bool descriptor_indexing = renderFramework->InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    if (descriptor_indexing) {
        instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }
    renderFramework->InitFramework(userData, features);
    descriptor_indexing = descriptor_indexing && renderFramework->DeviceExtensionSupported(renderFramework->gpu(), nullptr,
                                                                                           VK_KHR_MAINTENANCE3_EXTENSION_NAME);
    descriptor_indexing = descriptor_indexing && renderFramework->DeviceExtensionSupported(
                                                     renderFramework->gpu(), nullptr, VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    if (descriptor_indexing) {
        device_extension_names.push_back(VK_KHR_MAINTENANCE3_EXTENSION_NAME);
        device_extension_names.push_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
        return true;
    }
    return false;
}

bool CheckTimelineSemaphoreSupportAndInitState(VkRenderFramework *renderFramework) {
    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(renderFramework->instance(),
                                                                     "vkGetPhysicalDeviceFeatures2KHR");
    auto timeline_semaphore_features = lvl_init_struct<VkPhysicalDeviceTimelineSemaphoreFeatures>();
    auto features2 = lvl_init_struct<VkPhysicalDeviceFeatures2KHR>(&timeline_semaphore_features);
    vkGetPhysicalDeviceFeatures2KHR(renderFramework->gpu(), &features2);
    if (!timeline_semaphore_features.timelineSemaphore) {
        return false;
    }
    renderFramework->InitState(nullptr, &features2);
    return true;
}

void VkLayerTest::VKTriangleTest(BsoFailSelect failCase) {
    ASSERT_TRUE(m_device && m_device->initialized());  // VKTriangleTest assumes Init() has finished

    ASSERT_NO_FATAL_FAILURE(InitViewport());

    VkShaderObj vs(m_device, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT, this);
    VkShaderObj ps(m_device, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT, this);

    VkPipelineObj pipelineobj(m_device);
    pipelineobj.AddDefaultColorAttachment();
    pipelineobj.AddShader(&vs);
    pipelineobj.AddShader(&ps);

    bool failcase_needs_depth = false;  // to mark cases that need depth attachment

    VkBufferObj index_buffer;

    switch (failCase) {
        case BsoFailLineWidth: {
            pipelineobj.MakeDynamic(VK_DYNAMIC_STATE_LINE_WIDTH);
            VkPipelineInputAssemblyStateCreateInfo ia_state = {};
            ia_state.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            ia_state.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
            pipelineobj.SetInputAssembly(&ia_state);
            break;
        }
        case BsoFailLineStipple: {
            pipelineobj.MakeDynamic(VK_DYNAMIC_STATE_LINE_STIPPLE_EXT);
            VkPipelineInputAssemblyStateCreateInfo ia_state = {};
            ia_state.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            ia_state.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
            pipelineobj.SetInputAssembly(&ia_state);

            VkPipelineRasterizationLineStateCreateInfoEXT line_state = {};
            line_state.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_LINE_STATE_CREATE_INFO_EXT;
            line_state.lineRasterizationMode = VK_LINE_RASTERIZATION_MODE_BRESENHAM_EXT;
            line_state.stippledLineEnable = VK_TRUE;
            line_state.lineStippleFactor = 0;
            line_state.lineStipplePattern = 0;
            pipelineobj.SetLineState(&line_state);
            break;
        }
        case BsoFailDepthBias: {
            pipelineobj.MakeDynamic(VK_DYNAMIC_STATE_DEPTH_BIAS);
            VkPipelineRasterizationStateCreateInfo rs_state = {};
            rs_state.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            rs_state.depthBiasEnable = VK_TRUE;
            rs_state.lineWidth = 1.0f;
            pipelineobj.SetRasterization(&rs_state);
            break;
        }
        case BsoFailViewport: {
            pipelineobj.MakeDynamic(VK_DYNAMIC_STATE_VIEWPORT);
            break;
        }
        case BsoFailScissor: {
            pipelineobj.MakeDynamic(VK_DYNAMIC_STATE_SCISSOR);
            break;
        }
        case BsoFailBlend: {
            pipelineobj.MakeDynamic(VK_DYNAMIC_STATE_BLEND_CONSTANTS);
            VkPipelineColorBlendAttachmentState att_state = {};
            att_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_CONSTANT_COLOR;
            att_state.blendEnable = VK_TRUE;
            pipelineobj.AddColorAttachment(0, att_state);
            break;
        }
        case BsoFailDepthBounds: {
            failcase_needs_depth = true;
            pipelineobj.MakeDynamic(VK_DYNAMIC_STATE_DEPTH_BOUNDS);
            break;
        }
        case BsoFailStencilReadMask: {
            failcase_needs_depth = true;
            pipelineobj.MakeDynamic(VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK);
            break;
        }
        case BsoFailStencilWriteMask: {
            failcase_needs_depth = true;
            pipelineobj.MakeDynamic(VK_DYNAMIC_STATE_STENCIL_WRITE_MASK);
            break;
        }
        case BsoFailStencilReference: {
            failcase_needs_depth = true;
            pipelineobj.MakeDynamic(VK_DYNAMIC_STATE_STENCIL_REFERENCE);
            break;
        }

        case BsoFailIndexBuffer:
            break;
        case BsoFailIndexBufferBadSize:
        case BsoFailIndexBufferBadOffset:
        case BsoFailIndexBufferBadMapSize:
        case BsoFailIndexBufferBadMapOffset: {
            // Create an index buffer for these tests.
            // There is no need to populate it because we should bail before trying to draw.
            uint32_t const indices[] = {0};
            VkBufferCreateInfo buffer_info = {};
            buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            buffer_info.size = 1024;
            buffer_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
            buffer_info.queueFamilyIndexCount = 1;
            buffer_info.pQueueFamilyIndices = indices;
            index_buffer.init(*m_device, buffer_info, (VkMemoryPropertyFlags)VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        } break;
        case BsoFailCmdClearAttachments:
            break;
        case BsoFailNone:
            break;
        default:
            break;
    }

    VkDescriptorSetObj descriptorSet(m_device);

    VkImageView *depth_attachment = nullptr;
    if (failcase_needs_depth) {
        m_depth_stencil_fmt = FindSupportedDepthStencilFormat(gpu());
        ASSERT_TRUE(m_depth_stencil_fmt != VK_FORMAT_UNDEFINED);

        m_depthStencil->Init(m_device, static_cast<uint32_t>(m_width), static_cast<uint32_t>(m_height), m_depth_stencil_fmt,
                             VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
        depth_attachment = m_depthStencil->BindInfo();
    }

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget(1, depth_attachment));
    m_commandBuffer->begin();

    GenericDrawPreparation(m_commandBuffer, pipelineobj, descriptorSet, failCase);

    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    // render triangle
    if (failCase == BsoFailIndexBuffer) {
        // Use DrawIndexed w/o an index buffer bound
        m_commandBuffer->DrawIndexed(3, 1, 0, 0, 0);
    } else if (failCase == BsoFailIndexBufferBadSize) {
        // Bind the index buffer and draw one too many indices
        m_commandBuffer->BindIndexBuffer(&index_buffer, 0, VK_INDEX_TYPE_UINT16);
        m_commandBuffer->DrawIndexed(513, 1, 0, 0, 0);
    } else if (failCase == BsoFailIndexBufferBadOffset) {
        // Bind the index buffer and draw one past the end of the buffer using the offset
        m_commandBuffer->BindIndexBuffer(&index_buffer, 0, VK_INDEX_TYPE_UINT16);
        m_commandBuffer->DrawIndexed(512, 1, 1, 0, 0);
    } else if (failCase == BsoFailIndexBufferBadMapSize) {
        // Bind the index buffer at the middle point and draw one too many indices
        m_commandBuffer->BindIndexBuffer(&index_buffer, 512, VK_INDEX_TYPE_UINT16);
        m_commandBuffer->DrawIndexed(257, 1, 0, 0, 0);
    } else if (failCase == BsoFailIndexBufferBadMapOffset) {
        // Bind the index buffer at the middle point and draw one past the end of the buffer
        m_commandBuffer->BindIndexBuffer(&index_buffer, 512, VK_INDEX_TYPE_UINT16);
        m_commandBuffer->DrawIndexed(256, 1, 1, 0, 0);
    } else {
        m_commandBuffer->Draw(3, 1, 0, 0);
    }

    if (failCase == BsoFailCmdClearAttachments) {
        VkClearAttachment color_attachment = {};
        color_attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        color_attachment.colorAttachment = 2000000000;  // Someone who knew what they were doing would use 0 for the index;
        VkClearRect clear_rect = {{{0, 0}, {static_cast<uint32_t>(m_width), static_cast<uint32_t>(m_height)}}, 0, 1};

        vk::CmdClearAttachments(m_commandBuffer->handle(), 1, &color_attachment, 1, &clear_rect);
    }

    // finalize recording of the command buffer
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer(true);
    DestroyRenderTarget();
}

void VkLayerTest::GenericDrawPreparation(VkCommandBufferObj *commandBuffer, VkPipelineObj &pipelineobj,
                                         VkDescriptorSetObj &descriptorSet, BsoFailSelect failCase) {
    commandBuffer->ClearAllBuffers(m_renderTargets, m_clear_color, m_depthStencil, m_depth_clear_color, m_stencil_clear_color);

    commandBuffer->PrepareAttachments(m_renderTargets, m_depthStencil);
    // Make sure depthWriteEnable is set so that Depth fail test will work
    // correctly
    // Make sure stencilTestEnable is set so that Stencil fail test will work
    // correctly
    VkStencilOpState stencil = {};
    stencil.failOp = VK_STENCIL_OP_KEEP;
    stencil.passOp = VK_STENCIL_OP_KEEP;
    stencil.depthFailOp = VK_STENCIL_OP_KEEP;
    stencil.compareOp = VK_COMPARE_OP_NEVER;

    VkPipelineDepthStencilStateCreateInfo ds_ci = {};
    ds_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    ds_ci.pNext = NULL;
    ds_ci.depthTestEnable = VK_FALSE;
    ds_ci.depthWriteEnable = VK_TRUE;
    ds_ci.depthCompareOp = VK_COMPARE_OP_NEVER;
    ds_ci.depthBoundsTestEnable = VK_FALSE;
    if (failCase == BsoFailDepthBounds) {
        ds_ci.depthBoundsTestEnable = VK_TRUE;
        ds_ci.maxDepthBounds = 0.0f;
        ds_ci.minDepthBounds = 0.0f;
    }
    ds_ci.stencilTestEnable = VK_TRUE;
    ds_ci.front = stencil;
    ds_ci.back = stencil;

    pipelineobj.SetDepthStencil(&ds_ci);
    pipelineobj.SetViewport(m_viewports);
    pipelineobj.SetScissor(m_scissors);
    descriptorSet.CreateVKDescriptorSet(commandBuffer);
    VkResult err = pipelineobj.CreateVKPipeline(descriptorSet.GetPipelineLayout(), renderPass());
    ASSERT_VK_SUCCESS(err);
    vk::CmdBindPipeline(commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineobj.handle());
    commandBuffer->BindDescriptorSet(descriptorSet);
}

void VkLayerTest::Init(VkPhysicalDeviceFeatures *features, VkPhysicalDeviceFeatures2 *features2,
                       const VkCommandPoolCreateFlags flags, void *instance_pnext) {
    InitFramework(m_errorMonitor, instance_pnext);
    InitState(features, features2, flags);
}

VkCommandBufferObj *VkLayerTest::CommandBuffer() { return m_commandBuffer; }

VkLayerTest::VkLayerTest() {
    m_enableWSI = false;

    // TODO: not quite sure why most of this is here instead of in super

    // Add default instance extensions to the list
    instance_extensions_.push_back(debug_reporter_.debug_extension_name);

    instance_layers_.push_back(kValidationLayerName);

    if (VkTestFramework::m_devsim_layer) {
        if (InstanceLayerSupported("VK_LAYER_LUNARG_device_simulation")) {
            instance_layers_.push_back("VK_LAYER_LUNARG_device_simulation");
        } else {
            VkTestFramework::m_devsim_layer = false;
            printf("             Did not find VK_LAYER_LUNARG_device_simulation layer so it will not be enabled.\n");
        }
    } else {
        if (InstanceLayerSupported("VK_LAYER_LUNARG_device_profile_api"))
            instance_layers_.push_back("VK_LAYER_LUNARG_device_profile_api");
    }

    app_info_.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info_.pNext = NULL;
    app_info_.pApplicationName = "layer_tests";
    app_info_.applicationVersion = 1;
    app_info_.pEngineName = "unittest";
    app_info_.engineVersion = 1;
    app_info_.apiVersion = VK_API_VERSION_1_0;

    // Find out what version the instance supports and record the default target instance
    auto enumerateInstanceVersion = (PFN_vkEnumerateInstanceVersion)vk::GetInstanceProcAddr(nullptr, "vkEnumerateInstanceVersion");
    if (enumerateInstanceVersion) {
        enumerateInstanceVersion(&m_instance_api_version);
    } else {
        m_instance_api_version = VK_API_VERSION_1_0;
    }
    m_target_api_version = app_info_.apiVersion;
}

bool VkLayerTest::AddSurfaceInstanceExtension() {
    m_enableWSI = true;
    if (!InstanceExtensionSupported(VK_KHR_SURFACE_EXTENSION_NAME)) {
        printf("%s %s extension not supported\n", kSkipPrefix, VK_KHR_SURFACE_EXTENSION_NAME);
        return false;
    }
    instance_extensions_.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

    bool bSupport = false;
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    if (!InstanceExtensionSupported(VK_KHR_WIN32_SURFACE_EXTENSION_NAME)) {
        printf("%s %s extension not supported\n", kSkipPrefix, VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
        return false;
    }
    instance_extensions_.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
    bSupport = true;
#endif

#if defined(VK_USE_PLATFORM_ANDROID_KHR) && defined(VALIDATION_APK)
    if (!InstanceExtensionSupported(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME)) {
        printf("%s %s extension not supported\n", kSkipPrefix, VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
        return false;
    }
    instance_extensions_.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
    bSupport = true;
#endif

#if defined(VK_USE_PLATFORM_XLIB_KHR)
    if (!InstanceExtensionSupported(VK_KHR_XLIB_SURFACE_EXTENSION_NAME)) {
        printf("%s %s extension not supported\n", kSkipPrefix, VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
        return false;
    }
    if (XOpenDisplay(NULL)) {
        instance_extensions_.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
        bSupport = true;
    }
#endif

#if defined(VK_USE_PLATFORM_XCB_KHR)
    if (!InstanceExtensionSupported(VK_KHR_XCB_SURFACE_EXTENSION_NAME)) {
        printf("%s %s extension not supported\n", kSkipPrefix, VK_KHR_XCB_SURFACE_EXTENSION_NAME);
        return false;
    }
    if (!bSupport && xcb_connect(NULL, NULL)) {
        instance_extensions_.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
        bSupport = true;
    }
#endif

    if (bSupport) return true;
    printf("%s No platform's surface extension supported\n", kSkipPrefix);
    return false;
}

bool VkLayerTest::AddSwapchainDeviceExtension() {
    if (!DeviceExtensionSupported(gpu(), nullptr, VK_KHR_SWAPCHAIN_EXTENSION_NAME)) {
        printf("%s %s extension not supported\n", kSkipPrefix, VK_KHR_SWAPCHAIN_EXTENSION_NAME);
        return false;
    }
    m_device_extension_names.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    return true;
}

uint32_t VkLayerTest::SetTargetApiVersion(uint32_t target_api_version) {
    if (target_api_version == 0) target_api_version = VK_API_VERSION_1_0;
    if (target_api_version <= m_instance_api_version) {
        m_target_api_version = target_api_version;
        app_info_.apiVersion = m_target_api_version;
    }
    return m_target_api_version;
}

uint32_t VkLayerTest::DeviceValidationVersion() {
    // The validation layers assume the version we are validating to is the apiVersion unless the device apiVersion is lower
    return std::min(m_target_api_version, physDevProps().apiVersion);
}

bool VkLayerTest::LoadDeviceProfileLayer(
    PFN_vkSetPhysicalDeviceFormatPropertiesEXT &fpvkSetPhysicalDeviceFormatPropertiesEXT,
    PFN_vkGetOriginalPhysicalDeviceFormatPropertiesEXT &fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT) {
    // Load required functions
    fpvkSetPhysicalDeviceFormatPropertiesEXT =
        (PFN_vkSetPhysicalDeviceFormatPropertiesEXT)vk::GetInstanceProcAddr(instance(), "vkSetPhysicalDeviceFormatPropertiesEXT");
    fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT = (PFN_vkGetOriginalPhysicalDeviceFormatPropertiesEXT)vk::GetInstanceProcAddr(
        instance(), "vkGetOriginalPhysicalDeviceFormatPropertiesEXT");

    if (!(fpvkSetPhysicalDeviceFormatPropertiesEXT) || !(fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT)) {
        printf("%s Can't find device_profile_api functions; skipped.\n", kSkipPrefix);
        return 0;
    }

    return 1;
}

bool VkLayerTest::LoadDeviceProfileLayer(
    PFN_vkSetPhysicalDeviceFormatProperties2EXT &fpvkSetPhysicalDeviceFormatProperties2EXT,
    PFN_vkGetOriginalPhysicalDeviceFormatProperties2EXT &fpvkGetOriginalPhysicalDeviceFormatProperties2EXT) {
    // Load required functions
    fpvkSetPhysicalDeviceFormatProperties2EXT =
        (PFN_vkSetPhysicalDeviceFormatProperties2EXT)vk::GetInstanceProcAddr(instance(), "vkSetPhysicalDeviceFormatProperties2EXT");
    fpvkGetOriginalPhysicalDeviceFormatProperties2EXT =
        (PFN_vkGetOriginalPhysicalDeviceFormatProperties2EXT)vk::GetInstanceProcAddr(
            instance(), "vkGetOriginalPhysicalDeviceFormatProperties2EXT");

    if (!(fpvkSetPhysicalDeviceFormatProperties2EXT) || !(fpvkGetOriginalPhysicalDeviceFormatProperties2EXT)) {
        printf("%s Can't find device_profile_api functions; skipped.\n", kSkipPrefix);
        return false;
    }

    return true;
}

bool VkBufferTest::GetTestConditionValid(VkDeviceObj *aVulkanDevice, eTestEnFlags aTestFlag, VkBufferUsageFlags aBufferUsage) {
    if (eInvalidDeviceOffset != aTestFlag && eInvalidMemoryOffset != aTestFlag) {
        return true;
    }
    VkDeviceSize offset_limit = 0;
    if (eInvalidMemoryOffset == aTestFlag) {
        VkBuffer vulkanBuffer;
        VkBufferCreateInfo buffer_create_info = {};
        buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_create_info.size = 32;
        buffer_create_info.usage = aBufferUsage;

        vk::CreateBuffer(aVulkanDevice->device(), &buffer_create_info, nullptr, &vulkanBuffer);
        VkMemoryRequirements memory_reqs = {};

        vk::GetBufferMemoryRequirements(aVulkanDevice->device(), vulkanBuffer, &memory_reqs);
        vk::DestroyBuffer(aVulkanDevice->device(), vulkanBuffer, nullptr);
        offset_limit = memory_reqs.alignment;
    } else if ((VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT) & aBufferUsage) {
        offset_limit = aVulkanDevice->props.limits.minTexelBufferOffsetAlignment;
    } else if (VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT & aBufferUsage) {
        offset_limit = aVulkanDevice->props.limits.minUniformBufferOffsetAlignment;
    } else if (VK_BUFFER_USAGE_STORAGE_BUFFER_BIT & aBufferUsage) {
        offset_limit = aVulkanDevice->props.limits.minStorageBufferOffsetAlignment;
    }
    return eOffsetAlignment < offset_limit;
}

VkBufferTest::VkBufferTest(VkDeviceObj *aVulkanDevice, VkBufferUsageFlags aBufferUsage, eTestEnFlags aTestFlag)
    : AllocateCurrent(true),
      BoundCurrent(false),
      CreateCurrent(false),
      InvalidDeleteEn(false),
      VulkanDevice(aVulkanDevice->device()) {
    if (eBindNullBuffer == aTestFlag || eBindFakeBuffer == aTestFlag) {
        VkMemoryAllocateInfo memory_allocate_info = {};
        memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memory_allocate_info.allocationSize = 1;   // fake size -- shouldn't matter for the test
        memory_allocate_info.memoryTypeIndex = 0;  // fake type -- shouldn't matter for the test
        vk::AllocateMemory(VulkanDevice, &memory_allocate_info, nullptr, &VulkanMemory);

        VulkanBuffer = (aTestFlag == eBindNullBuffer) ? VK_NULL_HANDLE : (VkBuffer)0xCDCDCDCDCDCDCDCD;

        vk::BindBufferMemory(VulkanDevice, VulkanBuffer, VulkanMemory, 0);
    } else {
        VkBufferCreateInfo buffer_create_info = {};
        buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_create_info.size = 32;
        buffer_create_info.usage = aBufferUsage;

        vk::CreateBuffer(VulkanDevice, &buffer_create_info, nullptr, &VulkanBuffer);

        CreateCurrent = true;

        VkMemoryRequirements memory_requirements;
        vk::GetBufferMemoryRequirements(VulkanDevice, VulkanBuffer, &memory_requirements);

        VkMemoryAllocateInfo memory_allocate_info = {};
        memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memory_allocate_info.allocationSize = memory_requirements.size + eOffsetAlignment;
        bool pass = aVulkanDevice->phy().set_memory_type(memory_requirements.memoryTypeBits, &memory_allocate_info,
                                                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        if (!pass) {
            CreateCurrent = false;
            vk::DestroyBuffer(VulkanDevice, VulkanBuffer, nullptr);
            return;
        }

        vk::AllocateMemory(VulkanDevice, &memory_allocate_info, NULL, &VulkanMemory);
        // NB: 1 is intentionally an invalid offset value
        const bool offset_en = eInvalidDeviceOffset == aTestFlag || eInvalidMemoryOffset == aTestFlag;
        vk::BindBufferMemory(VulkanDevice, VulkanBuffer, VulkanMemory, offset_en ? eOffsetAlignment : 0);
        BoundCurrent = true;

        InvalidDeleteEn = (eFreeInvalidHandle == aTestFlag);
    }
}

VkBufferTest::~VkBufferTest() {
    if (CreateCurrent) {
        vk::DestroyBuffer(VulkanDevice, VulkanBuffer, nullptr);
    }
    if (AllocateCurrent) {
        if (InvalidDeleteEn) {
            auto bad_memory = CastFromUint64<VkDeviceMemory>(CastToUint64(VulkanMemory) + 1);
            vk::FreeMemory(VulkanDevice, bad_memory, nullptr);
        }
        vk::FreeMemory(VulkanDevice, VulkanMemory, nullptr);
    }
}

bool VkBufferTest::GetBufferCurrent() { return AllocateCurrent && BoundCurrent && CreateCurrent; }

const VkBuffer &VkBufferTest::GetBuffer() { return VulkanBuffer; }

void VkBufferTest::TestDoubleDestroy() {
    // Destroy the buffer but leave the flag set, which will cause
    // the buffer to be destroyed again in the destructor.
    vk::DestroyBuffer(VulkanDevice, VulkanBuffer, nullptr);
}

uint32_t VkVerticesObj::BindIdGenerator;

VkVerticesObj::VkVerticesObj(VkDeviceObj *aVulkanDevice, unsigned aAttributeCount, unsigned aBindingCount, unsigned aByteStride,
                             VkDeviceSize aVertexCount, const float *aVerticies)
    : BoundCurrent(false),
      AttributeCount(aAttributeCount),
      BindingCount(aBindingCount),
      BindId(BindIdGenerator),
      PipelineVertexInputStateCreateInfo(),
      VulkanMemoryBuffer(aVulkanDevice, static_cast<int>(aByteStride * aVertexCount), reinterpret_cast<const void *>(aVerticies),
                         VK_BUFFER_USAGE_VERTEX_BUFFER_BIT) {
    BindIdGenerator++;  // NB: This can wrap w/misuse

    VertexInputAttributeDescription = new VkVertexInputAttributeDescription[AttributeCount];
    VertexInputBindingDescription = new VkVertexInputBindingDescription[BindingCount];

    PipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = VertexInputAttributeDescription;
    PipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = AttributeCount;
    PipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = VertexInputBindingDescription;
    PipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = BindingCount;
    PipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    unsigned i = 0;
    do {
        VertexInputAttributeDescription[i].binding = BindId;
        VertexInputAttributeDescription[i].location = i;
        VertexInputAttributeDescription[i].format = VK_FORMAT_R32G32B32_SFLOAT;
        VertexInputAttributeDescription[i].offset = sizeof(float) * aByteStride;
        i++;
    } while (AttributeCount < i);

    i = 0;
    do {
        VertexInputBindingDescription[i].binding = BindId;
        VertexInputBindingDescription[i].stride = aByteStride;
        VertexInputBindingDescription[i].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        i++;
    } while (BindingCount < i);
}

VkVerticesObj::~VkVerticesObj() {
    if (VertexInputAttributeDescription) {
        delete[] VertexInputAttributeDescription;
    }
    if (VertexInputBindingDescription) {
        delete[] VertexInputBindingDescription;
    }
}

bool VkVerticesObj::AddVertexInputToPipe(VkPipelineObj &aPipelineObj) {
    aPipelineObj.AddVertexInputAttribs(VertexInputAttributeDescription, AttributeCount);
    aPipelineObj.AddVertexInputBindings(VertexInputBindingDescription, BindingCount);
    return true;
}

bool VkVerticesObj::AddVertexInputToPipeHelpr(CreatePipelineHelper *pipelineHelper) {
    pipelineHelper->vi_ci_.pVertexBindingDescriptions = VertexInputBindingDescription;
    pipelineHelper->vi_ci_.vertexBindingDescriptionCount = BindingCount;
    pipelineHelper->vi_ci_.pVertexAttributeDescriptions = VertexInputAttributeDescription;
    pipelineHelper->vi_ci_.vertexAttributeDescriptionCount = AttributeCount;
    return true;
}

void VkVerticesObj::BindVertexBuffers(VkCommandBuffer aCommandBuffer, unsigned aOffsetCount, VkDeviceSize *aOffsetList) {
    VkDeviceSize *offsetList;
    unsigned offsetCount;

    if (aOffsetCount) {
        offsetList = aOffsetList;
        offsetCount = aOffsetCount;
    } else {
        offsetList = new VkDeviceSize[1]();
        offsetCount = 1;
    }

    vk::CmdBindVertexBuffers(aCommandBuffer, BindId, offsetCount, &VulkanMemoryBuffer.handle(), offsetList);
    BoundCurrent = true;

    if (!aOffsetCount) {
        delete[] offsetList;
    }
}

OneOffDescriptorSet::OneOffDescriptorSet(VkDeviceObj *device, const Bindings &bindings,
                                         VkDescriptorSetLayoutCreateFlags layout_flags, void *layout_pnext,
                                         VkDescriptorPoolCreateFlags poolFlags, void *allocate_pnext, int buffer_info_size,
                                         int image_info_size, int buffer_view_size)
    : device_{device}, pool_{}, layout_(device, bindings, layout_flags, layout_pnext), set_{} {
    VkResult err;
    buffer_infos.reserve(buffer_info_size);
    image_infos.reserve(image_info_size);
    buffer_views.reserve(buffer_view_size);
    std::vector<VkDescriptorPoolSize> sizes;
    for (const auto &b : bindings) sizes.push_back({b.descriptorType, std::max(1u, b.descriptorCount)});

    VkDescriptorPoolCreateInfo dspci = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO, nullptr, poolFlags, 1, uint32_t(sizes.size()), sizes.data()};
    err = vk::CreateDescriptorPool(device_->handle(), &dspci, nullptr, &pool_);
    if (err != VK_SUCCESS) return;

    if ((layout_flags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR) == 0) {
        VkDescriptorSetAllocateInfo alloc_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, allocate_pnext, pool_, 1,
                                                  &layout_.handle()};
        err = vk::AllocateDescriptorSets(device_->handle(), &alloc_info, &set_);
    }
}

OneOffDescriptorSet::~OneOffDescriptorSet() {
    // No need to destroy set-- it's going away with the pool.
    vk::DestroyDescriptorPool(device_->handle(), pool_, nullptr);
}

bool OneOffDescriptorSet::Initialized() { return pool_ != VK_NULL_HANDLE && layout_.initialized() && set_ != VK_NULL_HANDLE; }

void OneOffDescriptorSet::WriteDescriptorBufferInfo(int blinding, VkBuffer buffer, VkDeviceSize size,
                                                    VkDescriptorType descriptorType, uint32_t count) {
    const auto index = buffer_infos.size();

    VkDescriptorBufferInfo buffer_info = {};
    buffer_info.buffer = buffer;
    buffer_info.offset = 0;
    buffer_info.range = size;

    for (uint32_t i = 0; i < count; ++i) {
        buffer_infos.emplace_back(buffer_info);
    }

    VkWriteDescriptorSet descriptor_write;
    memset(&descriptor_write, 0, sizeof(descriptor_write));
    descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_write.dstSet = set_;
    descriptor_write.dstBinding = blinding;
    descriptor_write.descriptorCount = count;
    descriptor_write.descriptorType = descriptorType;
    descriptor_write.pBufferInfo = &buffer_infos[index];
    descriptor_write.pImageInfo = nullptr;
    descriptor_write.pTexelBufferView = nullptr;

    descriptor_writes.emplace_back(descriptor_write);
}

void OneOffDescriptorSet::WriteDescriptorBufferView(int blinding, VkBufferView &buffer_view, VkDescriptorType descriptorType,
                                                    uint32_t count) {
    const auto index = buffer_views.size();

    for (uint32_t i = 0; i < count; ++i) {
        buffer_views.emplace_back(buffer_view);
    }

    VkWriteDescriptorSet descriptor_write;
    memset(&descriptor_write, 0, sizeof(descriptor_write));
    descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_write.dstSet = set_;
    descriptor_write.dstBinding = blinding;
    descriptor_write.descriptorCount = count;
    descriptor_write.descriptorType = descriptorType;
    descriptor_write.pTexelBufferView = &buffer_views[index];
    descriptor_write.pImageInfo = nullptr;
    descriptor_write.pBufferInfo = nullptr;

    descriptor_writes.emplace_back(descriptor_write);
}

void OneOffDescriptorSet::WriteDescriptorImageInfo(int blinding, VkImageView image_view, VkSampler sampler,
                                                   VkDescriptorType descriptorType, VkImageLayout imageLayout, uint32_t count) {
    const auto index = image_infos.size();

    VkDescriptorImageInfo image_info = {};
    image_info.imageView = image_view;
    image_info.sampler = sampler;
    image_info.imageLayout = imageLayout;

    for (uint32_t i = 0; i < count; ++i) {
        image_infos.emplace_back(image_info);
    }

    VkWriteDescriptorSet descriptor_write;
    memset(&descriptor_write, 0, sizeof(descriptor_write));
    descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_write.dstSet = set_;
    descriptor_write.dstBinding = blinding;
    descriptor_write.descriptorCount = count;
    descriptor_write.descriptorType = descriptorType;
    descriptor_write.pImageInfo = &image_infos[index];
    descriptor_write.pBufferInfo = nullptr;
    descriptor_write.pTexelBufferView = nullptr;

    descriptor_writes.emplace_back(descriptor_write);
}

void OneOffDescriptorSet::UpdateDescriptorSets() {
    vk::UpdateDescriptorSets(device_->handle(), descriptor_writes.size(), descriptor_writes.data(), 0, NULL);
}

CreatePipelineHelper::CreatePipelineHelper(VkLayerTest &test) : layer_test_(test) {}

CreatePipelineHelper::~CreatePipelineHelper() {
    VkDevice device = layer_test_.device();
    vk::DestroyPipelineCache(device, pipeline_cache_, nullptr);
    vk::DestroyPipeline(device, pipeline_, nullptr);
}

void CreatePipelineHelper::InitDescriptorSetInfo() {
    dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
}

void CreatePipelineHelper::InitInputAndVertexInfo() {
    vi_ci_.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    ia_ci_.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
}

void CreatePipelineHelper::InitMultisampleInfo() {
    pipe_ms_state_ci_.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    pipe_ms_state_ci_.pNext = nullptr;
    pipe_ms_state_ci_.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    pipe_ms_state_ci_.sampleShadingEnable = VK_FALSE;
    pipe_ms_state_ci_.minSampleShading = 1.0;
    pipe_ms_state_ci_.pSampleMask = NULL;
}

void CreatePipelineHelper::InitPipelineLayoutInfo() {
    pipeline_layout_ci_.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_ci_.setLayoutCount = 1;     // Not really changeable because InitState() sets exactly one pSetLayout
    pipeline_layout_ci_.pSetLayouts = nullptr;  // must bound after it is created
}

void CreatePipelineHelper::InitViewportInfo() {
    viewport_ = {0.0f, 0.0f, 64.0f, 64.0f, 0.0f, 1.0f};
    scissor_ = {{0, 0}, {64, 64}};

    vp_state_ci_.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vp_state_ci_.pNext = nullptr;
    vp_state_ci_.viewportCount = 1;
    vp_state_ci_.pViewports = &viewport_;  // ignored if dynamic
    vp_state_ci_.scissorCount = 1;
    vp_state_ci_.pScissors = &scissor_;  // ignored if dynamic
}

void CreatePipelineHelper::InitDynamicStateInfo() {
    // Use a "validity" check on the {} initialized structure to detect initialization
    // during late bind
}

void CreatePipelineHelper::InitShaderInfo() {
    vs_.reset(new VkShaderObj(layer_test_.DeviceObj(), bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT, &layer_test_));
    fs_.reset(new VkShaderObj(layer_test_.DeviceObj(), bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT, &layer_test_));
    // We shouldn't need a fragment shader but add it to be able to run on more devices
    shader_stages_ = {vs_->GetStageCreateInfo(), fs_->GetStageCreateInfo()};
}

void CreatePipelineHelper::InitRasterizationInfo() {
    rs_state_ci_.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rs_state_ci_.pNext = &line_state_ci_;
    rs_state_ci_.flags = 0;
    rs_state_ci_.depthClampEnable = VK_FALSE;
    rs_state_ci_.rasterizerDiscardEnable = VK_FALSE;
    rs_state_ci_.polygonMode = VK_POLYGON_MODE_FILL;
    rs_state_ci_.cullMode = VK_CULL_MODE_BACK_BIT;
    rs_state_ci_.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rs_state_ci_.depthBiasEnable = VK_FALSE;
    rs_state_ci_.lineWidth = 1.0F;
}

void CreatePipelineHelper::InitLineRasterizationInfo() {
    line_state_ci_.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_LINE_STATE_CREATE_INFO_EXT;
    line_state_ci_.pNext = nullptr;
    line_state_ci_.lineRasterizationMode = VK_LINE_RASTERIZATION_MODE_DEFAULT_EXT;
    line_state_ci_.stippledLineEnable = VK_FALSE;
    line_state_ci_.lineStippleFactor = 0;
    line_state_ci_.lineStipplePattern = 0;
}

void CreatePipelineHelper::InitBlendStateInfo() {
    cb_ci_.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    cb_ci_.logicOpEnable = VK_FALSE;
    cb_ci_.logicOp = VK_LOGIC_OP_COPY;  // ignored if enable is VK_FALSE above
    cb_ci_.attachmentCount = layer_test_.RenderPassInfo().subpassCount;
    ASSERT_TRUE(IsValidVkStruct(layer_test_.RenderPassInfo()));
    cb_ci_.pAttachments = &cb_attachments_;
    for (int i = 0; i < 4; i++) {
        cb_ci_.blendConstants[0] = 1.0F;
    }
}

void CreatePipelineHelper::InitGraphicsPipelineInfo() {
    // Color-only rendering in a subpass with no depth/stencil attachment
    // Active Pipeline Shader Stages
    //    Vertex Shader
    //    Fragment Shader
    // Required: Fixed-Function Pipeline Stages
    //    VkPipelineVertexInputStateCreateInfo
    //    VkPipelineInputAssemblyStateCreateInfo
    //    VkPipelineViewportStateCreateInfo
    //    VkPipelineRasterizationStateCreateInfo
    //    VkPipelineMultisampleStateCreateInfo
    //    VkPipelineColorBlendStateCreateInfo
    gp_ci_.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    gp_ci_.pNext = nullptr;
    gp_ci_.flags = VK_PIPELINE_CREATE_DISABLE_OPTIMIZATION_BIT;
    gp_ci_.pVertexInputState = &vi_ci_;
    gp_ci_.pInputAssemblyState = &ia_ci_;
    gp_ci_.pTessellationState = nullptr;
    gp_ci_.pViewportState = &vp_state_ci_;
    gp_ci_.pRasterizationState = &rs_state_ci_;
    gp_ci_.pMultisampleState = &pipe_ms_state_ci_;
    gp_ci_.pDepthStencilState = nullptr;
    gp_ci_.pColorBlendState = &cb_ci_;
    gp_ci_.pDynamicState = nullptr;
    gp_ci_.renderPass = layer_test_.renderPass();
}

void CreatePipelineHelper::InitPipelineCacheInfo() {
    pc_ci_.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    pc_ci_.pNext = nullptr;
    pc_ci_.flags = 0;
    pc_ci_.initialDataSize = 0;
    pc_ci_.pInitialData = nullptr;
}

void CreatePipelineHelper::InitTesselationState() {
    // TBD -- add shaders and create_info
}

void CreatePipelineHelper::InitInfo() {
    InitDescriptorSetInfo();
    InitInputAndVertexInfo();
    InitMultisampleInfo();
    InitPipelineLayoutInfo();
    InitViewportInfo();
    InitDynamicStateInfo();
    InitShaderInfo();
    InitRasterizationInfo();
    InitLineRasterizationInfo();
    InitBlendStateInfo();
    InitGraphicsPipelineInfo();
    InitPipelineCacheInfo();
}

void CreatePipelineHelper::InitState() {
    VkResult err;
    descriptor_set_.reset(new OneOffDescriptorSet(layer_test_.DeviceObj(), dsl_bindings_));
    ASSERT_TRUE(descriptor_set_->Initialized());

    const std::vector<VkPushConstantRange> push_ranges(
        pipeline_layout_ci_.pPushConstantRanges,
        pipeline_layout_ci_.pPushConstantRanges + pipeline_layout_ci_.pushConstantRangeCount);
    pipeline_layout_ = VkPipelineLayoutObj(layer_test_.DeviceObj(), {&descriptor_set_->layout_}, push_ranges);

    err = vk::CreatePipelineCache(layer_test_.device(), &pc_ci_, NULL, &pipeline_cache_);
    ASSERT_VK_SUCCESS(err);
}

void CreatePipelineHelper::LateBindPipelineInfo() {
    // By value or dynamically located items must be late bound
    gp_ci_.layout = pipeline_layout_.handle();
    gp_ci_.stageCount = shader_stages_.size();
    gp_ci_.pStages = shader_stages_.data();
    if ((gp_ci_.pTessellationState == nullptr) && IsValidVkStruct(tess_ci_)) {
        gp_ci_.pTessellationState = &tess_ci_;
    }
    if ((gp_ci_.pDynamicState == nullptr) && IsValidVkStruct(dyn_state_ci_)) {
        gp_ci_.pDynamicState = &dyn_state_ci_;
    }
}

VkResult CreatePipelineHelper::CreateGraphicsPipeline(bool implicit_destroy, bool do_late_bind) {
    VkResult err;
    if (do_late_bind) {
        LateBindPipelineInfo();
    }
    if (implicit_destroy && (pipeline_ != VK_NULL_HANDLE)) {
        vk::DestroyPipeline(layer_test_.device(), pipeline_, nullptr);
        pipeline_ = VK_NULL_HANDLE;
    }
    err = vk::CreateGraphicsPipelines(layer_test_.device(), pipeline_cache_, 1, &gp_ci_, NULL, &pipeline_);
    return err;
}

CreateComputePipelineHelper::CreateComputePipelineHelper(VkLayerTest &test) : layer_test_(test) {}

CreateComputePipelineHelper::~CreateComputePipelineHelper() {
    VkDevice device = layer_test_.device();
    vk::DestroyPipelineCache(device, pipeline_cache_, nullptr);
    vk::DestroyPipeline(device, pipeline_, nullptr);
}

void CreateComputePipelineHelper::InitDescriptorSetInfo() {
    dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
}

void CreateComputePipelineHelper::InitPipelineLayoutInfo() {
    pipeline_layout_ci_.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_ci_.setLayoutCount = 1;     // Not really changeable because InitState() sets exactly one pSetLayout
    pipeline_layout_ci_.pSetLayouts = nullptr;  // must bound after it is created
}

void CreateComputePipelineHelper::InitShaderInfo() {
    cs_.reset(new VkShaderObj(layer_test_.DeviceObj(), bindStateMinimalShaderText, VK_SHADER_STAGE_COMPUTE_BIT, &layer_test_));
    // We shouldn't need a fragment shader but add it to be able to run on more devices
}

void CreateComputePipelineHelper::InitComputePipelineInfo() {
    cp_ci_.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    cp_ci_.pNext = nullptr;
    cp_ci_.flags = 0;
}

void CreateComputePipelineHelper::InitPipelineCacheInfo() {
    pc_ci_.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    pc_ci_.pNext = nullptr;
    pc_ci_.flags = 0;
    pc_ci_.initialDataSize = 0;
    pc_ci_.pInitialData = nullptr;
}

void CreateComputePipelineHelper::InitInfo() {
    InitDescriptorSetInfo();
    InitPipelineLayoutInfo();
    InitShaderInfo();
    InitComputePipelineInfo();
    InitPipelineCacheInfo();
}

void CreateComputePipelineHelper::InitState() {
    VkResult err;
    descriptor_set_.reset(new OneOffDescriptorSet(layer_test_.DeviceObj(), dsl_bindings_));
    ASSERT_TRUE(descriptor_set_->Initialized());

    const std::vector<VkPushConstantRange> push_ranges(
        pipeline_layout_ci_.pPushConstantRanges,
        pipeline_layout_ci_.pPushConstantRanges + pipeline_layout_ci_.pushConstantRangeCount);
    pipeline_layout_ = VkPipelineLayoutObj(layer_test_.DeviceObj(), {&descriptor_set_->layout_}, push_ranges);

    err = vk::CreatePipelineCache(layer_test_.device(), &pc_ci_, NULL, &pipeline_cache_);
    ASSERT_VK_SUCCESS(err);
}

void CreateComputePipelineHelper::LateBindPipelineInfo() {
    // By value or dynamically located items must be late bound
    cp_ci_.layout = pipeline_layout_.handle();
    cp_ci_.stage = cs_.get()->GetStageCreateInfo();
}

VkResult CreateComputePipelineHelper::CreateComputePipeline(bool implicit_destroy, bool do_late_bind) {
    VkResult err;
    if (do_late_bind) {
        LateBindPipelineInfo();
    }
    if (implicit_destroy && (pipeline_ != VK_NULL_HANDLE)) {
        vk::DestroyPipeline(layer_test_.device(), pipeline_, nullptr);
        pipeline_ = VK_NULL_HANDLE;
    }
    err = vk::CreateComputePipelines(layer_test_.device(), pipeline_cache_, 1, &cp_ci_, NULL, &pipeline_);
    return err;
}

CreateNVRayTracingPipelineHelper::CreateNVRayTracingPipelineHelper(VkLayerTest &test) : layer_test_(test) {}
CreateNVRayTracingPipelineHelper::~CreateNVRayTracingPipelineHelper() {
    VkDevice device = layer_test_.device();
    vk::DestroyPipelineCache(device, pipeline_cache_, nullptr);
    vk::DestroyPipeline(device, pipeline_, nullptr);
}

bool CreateNVRayTracingPipelineHelper::InitInstanceExtensions(VkLayerTest &test,
                                                              std::vector<const char *> &instance_extension_names) {
    if (test.InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return false;
    }
    return true;
}

bool CreateNVRayTracingPipelineHelper::InitDeviceExtensions(VkLayerTest &test, std::vector<const char *> &device_extension_names) {
    std::array<const char *, 2> required_device_extensions = {
        {VK_NV_RAY_TRACING_EXTENSION_NAME, VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME}};
    for (auto device_extension : required_device_extensions) {
        if (test.DeviceExtensionSupported(test.gpu(), nullptr, device_extension)) {
            device_extension_names.push_back(device_extension);
        } else {
            printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix, device_extension);
            return false;
        }
    }
    return true;
}

void CreateNVRayTracingPipelineHelper::InitShaderGroups() {
    {
        VkRayTracingShaderGroupCreateInfoNV group = {};
        group.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
        group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        group.generalShader = 0;
        group.closestHitShader = VK_SHADER_UNUSED_NV;
        group.anyHitShader = VK_SHADER_UNUSED_NV;
        group.intersectionShader = VK_SHADER_UNUSED_NV;
        groups_.push_back(group);
    }
    {
        VkRayTracingShaderGroupCreateInfoNV group = {};
        group.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
        group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV;
        group.generalShader = VK_SHADER_UNUSED_NV;
        group.closestHitShader = 1;
        group.anyHitShader = VK_SHADER_UNUSED_NV;
        group.intersectionShader = VK_SHADER_UNUSED_NV;
        groups_.push_back(group);
    }
    {
        VkRayTracingShaderGroupCreateInfoNV group = {};
        group.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
        group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        group.generalShader = 2;
        group.closestHitShader = VK_SHADER_UNUSED_NV;
        group.anyHitShader = VK_SHADER_UNUSED_NV;
        group.intersectionShader = VK_SHADER_UNUSED_NV;
        groups_.push_back(group);
    }
}

void CreateNVRayTracingPipelineHelper::InitShaderGroupsKHR() {
    {
        VkRayTracingShaderGroupCreateInfoKHR group = {};
        group.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        group.generalShader = 0;
        group.closestHitShader = VK_SHADER_UNUSED_KHR;
        group.anyHitShader = VK_SHADER_UNUSED_KHR;
        group.intersectionShader = VK_SHADER_UNUSED_KHR;
        groups_KHR_.push_back(group);
    }
    {
        VkRayTracingShaderGroupCreateInfoKHR group = {};
        group.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
        group.generalShader = VK_SHADER_UNUSED_KHR;
        group.closestHitShader = 1;
        group.anyHitShader = VK_SHADER_UNUSED_KHR;
        group.intersectionShader = VK_SHADER_UNUSED_KHR;
        groups_KHR_.push_back(group);
    }
    {
        VkRayTracingShaderGroupCreateInfoKHR group = {};
        group.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        group.generalShader = 2;
        group.closestHitShader = VK_SHADER_UNUSED_KHR;
        group.anyHitShader = VK_SHADER_UNUSED_KHR;
        group.intersectionShader = VK_SHADER_UNUSED_KHR;
        groups_KHR_.push_back(group);
    }
}
void CreateNVRayTracingPipelineHelper::InitDescriptorSetInfo() {
    dsl_bindings_ = {
        {0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_RAYGEN_BIT_NV, nullptr},
        {1, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV, 1, VK_SHADER_STAGE_RAYGEN_BIT_NV, nullptr},
    };
}

void CreateNVRayTracingPipelineHelper::InitPipelineLayoutInfo() {
    pipeline_layout_ci_.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_ci_.setLayoutCount = 1;     // Not really changeable because InitState() sets exactly one pSetLayout
    pipeline_layout_ci_.pSetLayouts = nullptr;  // must bound after it is created
}

void CreateNVRayTracingPipelineHelper::InitShaderInfo() {  // DONE
    static const char rayGenShaderText[] =
        "#version 460 core                                                \n"
        "#extension GL_NV_ray_tracing : require                           \n"
        "layout(set = 0, binding = 0, rgba8) uniform image2D image;       \n"
        "layout(set = 0, binding = 1) uniform accelerationStructureNV as; \n"
        "                                                                 \n"
        "layout(location = 0) rayPayloadNV float payload;                 \n"
        "                                                                 \n"
        "void main()                                                      \n"
        "{                                                                \n"
        "   vec4 col = vec4(0, 0, 0, 1);                                  \n"
        "                                                                 \n"
        "   vec3 origin = vec3(float(gl_LaunchIDNV.x)/float(gl_LaunchSizeNV.x), "
        "float(gl_LaunchIDNV.y)/float(gl_LaunchSizeNV.y), "
        "1.0); \n"
        "   vec3 dir = vec3(0.0, 0.0, -1.0);                              \n"
        "                                                                 \n"
        "   payload = 0.5;                                                \n"
        "   traceNV(as, gl_RayFlagsCullBackFacingTrianglesNV, 0xff, 0, 1, 0, origin, 0.0, dir, 1000.0, 0); \n"
        "                                                                 \n"
        "   col.y = payload;                                              \n"
        "                                                                 \n"
        "   imageStore(image, ivec2(gl_LaunchIDNV.xy), col);              \n"
        "}\n";

    static char const closestHitShaderText[] =
        "#version 460 core                              \n"
        "#extension GL_NV_ray_tracing : require         \n"
        "layout(location = 0) rayPayloadInNV float hitValue;             \n"
        "                                               \n"
        "void main() {                                  \n"
        "    hitValue = 1.0;                            \n"
        "}                                              \n";

    static char const missShaderText[] =
        "#version 460 core                              \n"
        "#extension GL_NV_ray_tracing : require         \n"
        "layout(location = 0) rayPayloadInNV float hitValue; \n"
        "                                               \n"
        "void main() {                                  \n"
        "    hitValue = 0.0;                            \n"
        "}                                              \n";

    rgs_.reset(new VkShaderObj(layer_test_.DeviceObj(), rayGenShaderText, VK_SHADER_STAGE_RAYGEN_BIT_NV, &layer_test_));
    chs_.reset(new VkShaderObj(layer_test_.DeviceObj(), closestHitShaderText, VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV, &layer_test_));
    mis_.reset(new VkShaderObj(layer_test_.DeviceObj(), missShaderText, VK_SHADER_STAGE_MISS_BIT_NV, &layer_test_));

    shader_stages_ = {rgs_->GetStageCreateInfo(), chs_->GetStageCreateInfo(), mis_->GetStageCreateInfo()};
}

void CreateNVRayTracingPipelineHelper::InitNVRayTracingPipelineInfo() {
    rp_ci_.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV;
    rp_ci_.maxRecursionDepth = 0;
    rp_ci_.stageCount = shader_stages_.size();
    rp_ci_.pStages = shader_stages_.data();
    rp_ci_.groupCount = groups_.size();
    rp_ci_.pGroups = groups_.data();
}

void CreateNVRayTracingPipelineHelper::InitKHRRayTracingPipelineInfo() {
    rp_ci_KHR_.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV;
    rp_ci_KHR_.maxRecursionDepth = 0;
    rp_ci_KHR_.stageCount = shader_stages_.size();
    rp_ci_KHR_.pStages = shader_stages_.data();
    rp_ci_KHR_.groupCount = groups_KHR_.size();
    rp_ci_KHR_.pGroups = groups_KHR_.data();
}

void CreateNVRayTracingPipelineHelper::InitPipelineCacheInfo() {
    pc_ci_.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    pc_ci_.pNext = nullptr;
    pc_ci_.flags = 0;
    pc_ci_.initialDataSize = 0;
    pc_ci_.pInitialData = nullptr;
}

void CreateNVRayTracingPipelineHelper::InitInfo(bool isKHR) {
    isKHR ? InitShaderGroupsKHR() : InitShaderGroups();
    InitDescriptorSetInfo();
    InitPipelineLayoutInfo();
    InitShaderInfo();
    isKHR ? InitKHRRayTracingPipelineInfo() : InitNVRayTracingPipelineInfo();
    InitPipelineCacheInfo();
}

void CreateNVRayTracingPipelineHelper::InitState() {
    VkResult err;
    descriptor_set_.reset(new OneOffDescriptorSet(layer_test_.DeviceObj(), dsl_bindings_));
    ASSERT_TRUE(descriptor_set_->Initialized());

    pipeline_layout_ = VkPipelineLayoutObj(layer_test_.DeviceObj(), {&descriptor_set_->layout_});

    err = vk::CreatePipelineCache(layer_test_.device(), &pc_ci_, NULL, &pipeline_cache_);
    ASSERT_VK_SUCCESS(err);
}

void CreateNVRayTracingPipelineHelper::LateBindPipelineInfo(bool isKHR) {
    // By value or dynamically located items must be late bound
    if (isKHR) {
        rp_ci_KHR_.layout = pipeline_layout_.handle();
        rp_ci_KHR_.stageCount = shader_stages_.size();
        rp_ci_KHR_.pStages = shader_stages_.data();
    } else {
        rp_ci_.layout = pipeline_layout_.handle();
        rp_ci_.stageCount = shader_stages_.size();
        rp_ci_.pStages = shader_stages_.data();
    }
}

VkResult CreateNVRayTracingPipelineHelper::CreateNVRayTracingPipeline(bool implicit_destroy, bool do_late_bind) {
    VkResult err;
    if (do_late_bind) {
        LateBindPipelineInfo();
    }
    if (implicit_destroy && (pipeline_ != VK_NULL_HANDLE)) {
        vk::DestroyPipeline(layer_test_.device(), pipeline_, nullptr);
        pipeline_ = VK_NULL_HANDLE;
    }

    PFN_vkCreateRayTracingPipelinesNV vkCreateRayTracingPipelinesNV =
        (PFN_vkCreateRayTracingPipelinesNV)vk::GetInstanceProcAddr(layer_test_.instance(), "vkCreateRayTracingPipelinesNV");
    err = vkCreateRayTracingPipelinesNV(layer_test_.device(), pipeline_cache_, 1, &rp_ci_, nullptr, &pipeline_);
    return err;
}

VkResult CreateNVRayTracingPipelineHelper::CreateKHRRayTracingPipeline(bool implicit_destroy, bool do_late_bind) {
    VkResult err;
    if (do_late_bind) {
        LateBindPipelineInfo(true /*isKHR*/);
    }
    if (implicit_destroy && (pipeline_ != VK_NULL_HANDLE)) {
        vk::DestroyPipeline(layer_test_.device(), pipeline_, nullptr);
        pipeline_ = VK_NULL_HANDLE;
    }
    PFN_vkCreateRayTracingPipelinesKHR vkCreateRayTracingPipelinesKHR =
        (PFN_vkCreateRayTracingPipelinesKHR)vk::GetInstanceProcAddr(layer_test_.instance(), "vkCreateRayTracingPipelinesKHR");
    err = vkCreateRayTracingPipelinesKHR(layer_test_.device(), pipeline_cache_, 1, &rp_ci_KHR_, nullptr, &pipeline_);
    return err;
}

namespace chain_util {
const void *ExtensionChain::Head() const { return head_; }
}  // namespace chain_util

BarrierQueueFamilyTestHelper::QueueFamilyObjs::~QueueFamilyObjs() {
    delete command_buffer2;
    delete command_buffer;
    delete command_pool;
    delete queue;
}

void BarrierQueueFamilyTestHelper::QueueFamilyObjs::Init(VkDeviceObj *device, uint32_t qf_index, VkQueue qf_queue,
                                                         VkCommandPoolCreateFlags cp_flags) {
    index = qf_index;
    queue = new VkQueueObj(qf_queue, qf_index);
    command_pool = new VkCommandPoolObj(device, qf_index, cp_flags);
    command_buffer = new VkCommandBufferObj(device, command_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, queue);
    command_buffer2 = new VkCommandBufferObj(device, command_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, queue);
};

BarrierQueueFamilyTestHelper::Context::Context(VkLayerTest *test, const std::vector<uint32_t> &queue_family_indices)
    : layer_test(test) {
    if (0 == queue_family_indices.size()) {
        return;  // This is invalid
    }
    VkDeviceObj *device_obj = layer_test->DeviceObj();
    queue_families.reserve(queue_family_indices.size());
    default_index = queue_family_indices[0];
    for (auto qfi : queue_family_indices) {
        VkQueue queue = device_obj->queue_family_queues(qfi)[0]->handle();
        queue_families.emplace(std::make_pair(qfi, QueueFamilyObjs()));
        queue_families[qfi].Init(device_obj, qfi, queue, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    }
    Reset();
}

void BarrierQueueFamilyTestHelper::Context::Reset() {
    layer_test->DeviceObj()->wait();
    for (auto &qf : queue_families) {
        vk::ResetCommandPool(layer_test->device(), qf.second.command_pool->handle(), 0);
    }
}

BarrierQueueFamilyTestHelper::BarrierQueueFamilyTestHelper(Context *context)
    : context_(context), image_(context->layer_test->DeviceObj()) {}

void BarrierQueueFamilyTestHelper::Init(std::vector<uint32_t> *families, bool image_memory, bool buffer_memory) {
    VkDeviceObj *device_obj = context_->layer_test->DeviceObj();

    image_.Init(32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0, families,
                image_memory);

    ASSERT_TRUE(image_.initialized());

    image_barrier_ = image_.image_memory_barrier(VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_TRANSFER_READ_BIT, image_.Layout(),
                                                 image_.Layout(), image_.subresource_range(VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1));

    VkMemoryPropertyFlags mem_prop = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    buffer_.init_as_src_and_dst(*device_obj, 256, mem_prop, families, buffer_memory);
    ASSERT_TRUE(buffer_.initialized());
    buffer_barrier_ = buffer_.buffer_memory_barrier(VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_TRANSFER_READ_BIT, 0, VK_WHOLE_SIZE);
}

BarrierQueueFamilyTestHelper::QueueFamilyObjs *BarrierQueueFamilyTestHelper::GetQueueFamilyInfo(Context *context, uint32_t qfi) {
    QueueFamilyObjs *qf;

    auto qf_it = context->queue_families.find(qfi);
    if (qf_it != context->queue_families.end()) {
        qf = &(qf_it->second);
    } else {
        qf = &(context->queue_families[context->default_index]);
    }
    return qf;
}

void BarrierQueueFamilyTestHelper::operator()(std::string img_err, std::string buf_err, uint32_t src, uint32_t dst, bool positive,
                                              uint32_t queue_family_index, Modifier mod) {
    auto &monitor = context_->layer_test->Monitor();
    if (img_err.length()) monitor.SetDesiredFailureMsg(kErrorBit | kWarningBit, img_err);
    if (buf_err.length()) monitor.SetDesiredFailureMsg(kErrorBit | kWarningBit, buf_err);

    image_barrier_.srcQueueFamilyIndex = src;
    image_barrier_.dstQueueFamilyIndex = dst;
    buffer_barrier_.srcQueueFamilyIndex = src;
    buffer_barrier_.dstQueueFamilyIndex = dst;

    QueueFamilyObjs *qf = GetQueueFamilyInfo(context_, queue_family_index);

    VkCommandBufferObj *command_buffer = qf->command_buffer;
    for (int cb_repeat = 0; cb_repeat < (mod == Modifier::DOUBLE_COMMAND_BUFFER ? 2 : 1); cb_repeat++) {
        command_buffer->begin();
        for (int repeat = 0; repeat < (mod == Modifier::DOUBLE_RECORD ? 2 : 1); repeat++) {
            vk::CmdPipelineBarrier(command_buffer->handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                   VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 1, &buffer_barrier_, 1, &image_barrier_);
        }
        command_buffer->end();
        command_buffer = qf->command_buffer2;  // Second pass (if any) goes to the secondary command_buffer.
    }

    if (queue_family_index != kInvalidQueueFamily) {
        if (mod == Modifier::DOUBLE_COMMAND_BUFFER) {
            // the Fence resolves to VK_NULL_HANLE... i.e. no fence
            qf->queue->submit({{qf->command_buffer, qf->command_buffer2}}, vk_testing::Fence(), positive);
        } else {
            qf->command_buffer->QueueCommandBuffer(positive);  // Check for success on positive tests only
        }
    }

    if (positive) {
        monitor.VerifyNotFound();
    } else {
        monitor.VerifyFound();
    }
    context_->Reset();
};

bool InitFrameworkForRayTracingTest(VkRenderFramework *renderFramework, bool isKHR,
                                    std::vector<const char *> &instance_extension_names,
                                    std::vector<const char *> &device_extension_names, void *user_data, bool need_gpu_validation,
                                    bool need_push_descriptors, bool deferred_state_init) {
    const std::array<const char *, 1> required_instance_extensions = {{VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}};
    for (const char *required_instance_extension : required_instance_extensions) {
        if (renderFramework->InstanceExtensionSupported(required_instance_extension)) {
            instance_extension_names.push_back(required_instance_extension);
        } else {
            printf("%s %s instance extension not supported, skipping test\n", kSkipPrefix, required_instance_extension);
            return false;
        }
    }

    VkValidationFeatureEnableEXT enables[] = {VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT};
    VkValidationFeatureDisableEXT disables[] = {
        VK_VALIDATION_FEATURE_DISABLE_THREAD_SAFETY_EXT, VK_VALIDATION_FEATURE_DISABLE_API_PARAMETERS_EXT,
        VK_VALIDATION_FEATURE_DISABLE_OBJECT_LIFETIMES_EXT, VK_VALIDATION_FEATURE_DISABLE_CORE_CHECKS_EXT};
    VkValidationFeaturesEXT features = {};
    features.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
    features.enabledValidationFeatureCount = 1;
    features.pEnabledValidationFeatures = enables;
    features.disabledValidationFeatureCount = 4;
    features.pDisabledValidationFeatures = disables;

    VkValidationFeaturesEXT *enabled_features = need_gpu_validation ? &features : nullptr;

    renderFramework->InitFramework(user_data, enabled_features);

    if (renderFramework->IsPlatform(kMockICD) || renderFramework->DeviceSimulation()) {
        printf("%s Test not supported by MockICD, skipping tests\n", kSkipPrefix);
        return false;
    }

    std::vector<const char *> required_device_extensions;
    required_device_extensions.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
    if (isKHR) {
        required_device_extensions.push_back(VK_KHR_RAY_TRACING_EXTENSION_NAME);
        required_device_extensions.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
        required_device_extensions.push_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
        required_device_extensions.push_back(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
        required_device_extensions.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
        required_device_extensions.push_back(VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME);
    } else {
        required_device_extensions.push_back(VK_NV_RAY_TRACING_EXTENSION_NAME);
    }
    if (need_push_descriptors) {
        required_device_extensions.push_back(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    }

    for (const char *required_device_extension : required_device_extensions) {
        if (renderFramework->DeviceExtensionSupported(renderFramework->gpu(), nullptr, required_device_extension)) {
            device_extension_names.push_back(required_device_extension);
        } else {
            printf("%s %s device extension not supported, skipping test\n", kSkipPrefix, required_device_extension);
            return false;
        }
    }
    if (!deferred_state_init) renderFramework->InitState();
    return true;
}

void GetSimpleGeometryForAccelerationStructureTests(const VkDeviceObj &device, VkBufferObj *vbo, VkBufferObj *ibo,
                                                    VkGeometryNV *geometry) {
    vbo->init(device, 1024, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
              VK_BUFFER_USAGE_RAY_TRACING_BIT_NV);
    ibo->init(device, 1024, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
              VK_BUFFER_USAGE_RAY_TRACING_BIT_NV);

    const std::vector<float> vertices = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f};
    const std::vector<uint32_t> indicies = {0, 1, 2};

    uint8_t *mapped_vbo_buffer_data = (uint8_t *)vbo->memory().map();
    std::memcpy(mapped_vbo_buffer_data, (uint8_t *)vertices.data(), sizeof(float) * vertices.size());
    vbo->memory().unmap();

    uint8_t *mapped_ibo_buffer_data = (uint8_t *)ibo->memory().map();
    std::memcpy(mapped_ibo_buffer_data, (uint8_t *)indicies.data(), sizeof(uint32_t) * indicies.size());
    ibo->memory().unmap();

    *geometry = {};
    geometry->sType = VK_STRUCTURE_TYPE_GEOMETRY_NV;
    geometry->geometryType = VK_GEOMETRY_TYPE_TRIANGLES_NV;
    geometry->geometry.triangles.sType = VK_STRUCTURE_TYPE_GEOMETRY_TRIANGLES_NV;
    geometry->geometry.triangles.vertexData = vbo->handle();
    geometry->geometry.triangles.vertexOffset = 0;
    geometry->geometry.triangles.vertexCount = 3;
    geometry->geometry.triangles.vertexStride = 12;
    geometry->geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
    geometry->geometry.triangles.indexData = ibo->handle();
    geometry->geometry.triangles.indexOffset = 0;
    geometry->geometry.triangles.indexCount = 3;
    geometry->geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
    geometry->geometry.triangles.transformData = VK_NULL_HANDLE;
    geometry->geometry.triangles.transformOffset = 0;
    geometry->geometry.aabbs = {};
    geometry->geometry.aabbs.sType = VK_STRUCTURE_TYPE_GEOMETRY_AABB_NV;
}

void VkLayerTest::OOBRayTracingShadersTestBody(bool gpu_assisted) {
    std::array<const char *, 1> required_instance_extensions = {{VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}};
    for (auto instance_extension : required_instance_extensions) {
        if (InstanceExtensionSupported(instance_extension)) {
            m_instance_extension_names.push_back(instance_extension);
        } else {
            printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix, instance_extension);
            return;
        }
    }

    VkValidationFeatureEnableEXT validation_feature_enables[] = {VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT};
    VkValidationFeatureDisableEXT validation_feature_disables[] = {
        VK_VALIDATION_FEATURE_DISABLE_THREAD_SAFETY_EXT, VK_VALIDATION_FEATURE_DISABLE_API_PARAMETERS_EXT,
        VK_VALIDATION_FEATURE_DISABLE_OBJECT_LIFETIMES_EXT, VK_VALIDATION_FEATURE_DISABLE_CORE_CHECKS_EXT};
    VkValidationFeaturesEXT validation_features = {};
    validation_features.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
    validation_features.enabledValidationFeatureCount = 1;
    validation_features.pEnabledValidationFeatures = validation_feature_enables;
    validation_features.disabledValidationFeatureCount = 4;
    validation_features.pDisabledValidationFeatures = validation_feature_disables;
    bool descriptor_indexing = CheckDescriptorIndexingSupportAndInitFramework(
        this, m_instance_extension_names, m_device_extension_names, gpu_assisted ? &validation_features : nullptr, m_errorMonitor);

    if (IsPlatform(kMockICD) || DeviceSimulation()) {
        printf("%s Test not supported by MockICD, skipping tests\n", kSkipPrefix);
        return;
    }

    std::array<const char *, 2> required_device_extensions = {
        {VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME, VK_NV_RAY_TRACING_EXTENSION_NAME}};
    for (auto device_extension : required_device_extensions) {
        if (DeviceExtensionSupported(gpu(), nullptr, device_extension)) {
            m_device_extension_names.push_back(device_extension);
        } else {
            printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix, device_extension);
            return;
        }
    }

    VkPhysicalDeviceFeatures2KHR features2 = {};
    auto indexing_features = lvl_init_struct<VkPhysicalDeviceDescriptorIndexingFeaturesEXT>();
    if (descriptor_indexing) {
        PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
            (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
        ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

        features2 = lvl_init_struct<VkPhysicalDeviceFeatures2KHR>(&indexing_features);
        vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);

        if (!indexing_features.runtimeDescriptorArray || !indexing_features.descriptorBindingPartiallyBound ||
            !indexing_features.descriptorBindingSampledImageUpdateAfterBind ||
            !indexing_features.descriptorBindingVariableDescriptorCount) {
            printf("Not all descriptor indexing features supported, skipping descriptor indexing tests\n");
            descriptor_indexing = false;
        }
    }
    VkCommandPoolCreateFlags pool_flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2, pool_flags));

    PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2KHR =
        (PFN_vkGetPhysicalDeviceProperties2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceProperties2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceProperties2KHR != nullptr);

    auto ray_tracing_properties = lvl_init_struct<VkPhysicalDeviceRayTracingPropertiesNV>();
    auto properties2 = lvl_init_struct<VkPhysicalDeviceProperties2KHR>(&ray_tracing_properties);
    vkGetPhysicalDeviceProperties2KHR(gpu(), &properties2);
    if (ray_tracing_properties.maxTriangleCount == 0) {
        printf("%s Did not find required ray tracing properties; skipped.\n", kSkipPrefix);
        return;
    }

    VkQueue ray_tracing_queue = m_device->m_queue;
    uint32_t ray_tracing_queue_family_index = 0;

    // If supported, run on the compute only queue.
    uint32_t compute_only_queue_family_index = m_device->QueueFamilyMatching(VK_QUEUE_COMPUTE_BIT, VK_QUEUE_GRAPHICS_BIT);
    if (compute_only_queue_family_index != UINT32_MAX) {
        const auto &compute_only_queues = m_device->queue_family_queues(compute_only_queue_family_index);
        if (!compute_only_queues.empty()) {
            ray_tracing_queue = compute_only_queues[0]->handle();
            ray_tracing_queue_family_index = compute_only_queue_family_index;
        }
    }

    VkCommandPoolObj ray_tracing_command_pool(m_device, ray_tracing_queue_family_index,
                                              VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    VkCommandBufferObj ray_tracing_command_buffer(m_device, &ray_tracing_command_pool);

    struct AABB {
        float min_x;
        float min_y;
        float min_z;
        float max_x;
        float max_y;
        float max_z;
    };

    const std::vector<AABB> aabbs = {{-1.0f, -1.0f, -1.0f, +1.0f, +1.0f, +1.0f}};

    struct VkGeometryInstanceNV {
        float transform[12];
        uint32_t instanceCustomIndex : 24;
        uint32_t mask : 8;
        uint32_t instanceOffset : 24;
        uint32_t flags : 8;
        uint64_t accelerationStructureHandle;
    };

    VkDeviceSize aabb_buffer_size = sizeof(AABB) * aabbs.size();
    VkBufferObj aabb_buffer;
    aabb_buffer.init(*m_device, aabb_buffer_size, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, {ray_tracing_queue_family_index});

    uint8_t *mapped_aabb_buffer_data = (uint8_t *)aabb_buffer.memory().map();
    std::memcpy(mapped_aabb_buffer_data, (uint8_t *)aabbs.data(), static_cast<std::size_t>(aabb_buffer_size));
    aabb_buffer.memory().unmap();

    VkGeometryNV geometry = {};
    geometry.sType = VK_STRUCTURE_TYPE_GEOMETRY_NV;
    geometry.geometryType = VK_GEOMETRY_TYPE_AABBS_NV;
    geometry.geometry.triangles = {};
    geometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_GEOMETRY_TRIANGLES_NV;
    geometry.geometry.aabbs = {};
    geometry.geometry.aabbs.sType = VK_STRUCTURE_TYPE_GEOMETRY_AABB_NV;
    geometry.geometry.aabbs.aabbData = aabb_buffer.handle();
    geometry.geometry.aabbs.numAABBs = static_cast<uint32_t>(aabbs.size());
    geometry.geometry.aabbs.offset = 0;
    geometry.geometry.aabbs.stride = static_cast<VkDeviceSize>(sizeof(AABB));
    geometry.flags = 0;

    VkAccelerationStructureInfoNV bot_level_as_info = {};
    bot_level_as_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
    bot_level_as_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
    bot_level_as_info.instanceCount = 0;
    bot_level_as_info.geometryCount = 1;
    bot_level_as_info.pGeometries = &geometry;

    VkAccelerationStructureCreateInfoNV bot_level_as_create_info = {};
    bot_level_as_create_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV;
    bot_level_as_create_info.info = bot_level_as_info;

    VkAccelerationStructureObj bot_level_as(*m_device, bot_level_as_create_info);

    const std::vector<VkGeometryInstanceNV> instances = {
        VkGeometryInstanceNV{
            {
                // clang-format off
                1.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f,
                // clang-format on
            },
            0,
            0xFF,
            0,
            VK_GEOMETRY_INSTANCE_TRIANGLE_CULL_DISABLE_BIT_NV,
            bot_level_as.opaque_handle(),
        },
    };

    VkDeviceSize instance_buffer_size = sizeof(VkGeometryInstanceNV) * instances.size();
    VkBufferObj instance_buffer;
    instance_buffer.init(*m_device, instance_buffer_size,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, {ray_tracing_queue_family_index});

    uint8_t *mapped_instance_buffer_data = (uint8_t *)instance_buffer.memory().map();
    std::memcpy(mapped_instance_buffer_data, (uint8_t *)instances.data(), static_cast<std::size_t>(instance_buffer_size));
    instance_buffer.memory().unmap();

    VkAccelerationStructureInfoNV top_level_as_info = {};
    top_level_as_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
    top_level_as_info.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV;
    top_level_as_info.instanceCount = 1;
    top_level_as_info.geometryCount = 0;

    VkAccelerationStructureCreateInfoNV top_level_as_create_info = {};
    top_level_as_create_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV;
    top_level_as_create_info.info = top_level_as_info;

    VkAccelerationStructureObj top_level_as(*m_device, top_level_as_create_info);

    VkDeviceSize scratch_buffer_size = std::max(bot_level_as.build_scratch_memory_requirements().memoryRequirements.size,
                                                top_level_as.build_scratch_memory_requirements().memoryRequirements.size);
    VkBufferObj scratch_buffer;
    scratch_buffer.init(*m_device, scratch_buffer_size, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV);

    ray_tracing_command_buffer.begin();

    // Build bot level acceleration structure
    ray_tracing_command_buffer.BuildAccelerationStructure(&bot_level_as, scratch_buffer.handle());

    // Barrier to prevent using scratch buffer for top level build before bottom level build finishes
    VkMemoryBarrier memory_barrier = {};
    memory_barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    memory_barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV;
    memory_barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV;
    ray_tracing_command_buffer.PipelineBarrier(VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV,
                                               VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, 0, 1, &memory_barrier, 0,
                                               nullptr, 0, nullptr);

    // Build top level acceleration structure
    ray_tracing_command_buffer.BuildAccelerationStructure(&top_level_as, scratch_buffer.handle(), instance_buffer.handle());

    ray_tracing_command_buffer.end();

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &ray_tracing_command_buffer.handle();
    vk::QueueSubmit(ray_tracing_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(ray_tracing_queue);
    m_errorMonitor->VerifyNotFound();

    VkTextureObj texture(m_device, nullptr);
    VkSamplerObj sampler(m_device);

    VkDeviceSize storage_buffer_size = 1024;
    VkBufferObj storage_buffer;
    storage_buffer.init(*m_device, storage_buffer_size, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, {ray_tracing_queue_family_index});

    VkDeviceSize shader_binding_table_buffer_size = ray_tracing_properties.shaderGroupBaseAlignment * 4ull;
    VkBufferObj shader_binding_table_buffer;
    shader_binding_table_buffer.init(*m_device, shader_binding_table_buffer_size,
                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                     VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, {ray_tracing_queue_family_index});

    // Setup descriptors!
    const VkShaderStageFlags kAllRayTracingStages = VK_SHADER_STAGE_RAYGEN_BIT_NV | VK_SHADER_STAGE_ANY_HIT_BIT_NV |
                                                    VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV | VK_SHADER_STAGE_MISS_BIT_NV |
                                                    VK_SHADER_STAGE_INTERSECTION_BIT_NV | VK_SHADER_STAGE_CALLABLE_BIT_NV;

    void *layout_pnext = nullptr;
    void *allocate_pnext = nullptr;
    VkDescriptorPoolCreateFlags pool_create_flags = 0;
    VkDescriptorSetLayoutCreateFlags layout_create_flags = 0;
    VkDescriptorBindingFlagsEXT ds_binding_flags[3] = {};
    VkDescriptorSetLayoutBindingFlagsCreateInfoEXT layout_createinfo_binding_flags[1] = {};
    if (descriptor_indexing) {
        ds_binding_flags[0] = 0;
        ds_binding_flags[1] = 0;
        ds_binding_flags[2] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT;

        layout_createinfo_binding_flags[0].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT;
        layout_createinfo_binding_flags[0].pNext = NULL;
        layout_createinfo_binding_flags[0].bindingCount = 3;
        layout_createinfo_binding_flags[0].pBindingFlags = ds_binding_flags;
        layout_create_flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;
        pool_create_flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;
        layout_pnext = layout_createinfo_binding_flags;
    }

    // Prepare descriptors
    OneOffDescriptorSet ds(m_device,
                           {
                               {0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV, 1, kAllRayTracingStages, nullptr},
                               {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, kAllRayTracingStages, nullptr},
                               {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 6, kAllRayTracingStages, nullptr},
                           },
                           layout_create_flags, layout_pnext, pool_create_flags);

    VkDescriptorSetVariableDescriptorCountAllocateInfoEXT variable_count = {};
    uint32_t desc_counts;
    if (descriptor_indexing) {
        layout_create_flags = 0;
        pool_create_flags = 0;
        ds_binding_flags[2] =
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT | VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT;
        desc_counts = 6;  // We'll reserve 8 spaces in the layout, but the descriptor will only use 6
        variable_count.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO_EXT;
        variable_count.descriptorSetCount = 1;
        variable_count.pDescriptorCounts = &desc_counts;
        allocate_pnext = &variable_count;
    }

    OneOffDescriptorSet ds_variable(m_device,
                                    {
                                        {0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV, 1, kAllRayTracingStages, nullptr},
                                        {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, kAllRayTracingStages, nullptr},
                                        {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 8, kAllRayTracingStages, nullptr},
                                    },
                                    layout_create_flags, layout_pnext, pool_create_flags, allocate_pnext);

    VkAccelerationStructureNV top_level_as_handle = top_level_as.handle();
    VkWriteDescriptorSetAccelerationStructureNV write_descript_set_as = {};
    write_descript_set_as.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_NV;
    write_descript_set_as.accelerationStructureCount = 1;
    write_descript_set_as.pAccelerationStructures = &top_level_as_handle;

    VkDescriptorBufferInfo descriptor_buffer_info = {};
    descriptor_buffer_info.buffer = storage_buffer.handle();
    descriptor_buffer_info.offset = 0;
    descriptor_buffer_info.range = storage_buffer_size;

    VkDescriptorImageInfo descriptor_image_infos[6] = {};
    for (int i = 0; i < 6; i++) {
        descriptor_image_infos[i] = texture.DescriptorImageInfo();
        descriptor_image_infos[i].sampler = sampler.handle();
        descriptor_image_infos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }

    VkWriteDescriptorSet descriptor_writes[3] = {};
    descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_writes[0].dstSet = ds.set_;
    descriptor_writes[0].dstBinding = 0;
    descriptor_writes[0].descriptorCount = 1;
    descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV;
    descriptor_writes[0].pNext = &write_descript_set_as;

    descriptor_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_writes[1].dstSet = ds.set_;
    descriptor_writes[1].dstBinding = 1;
    descriptor_writes[1].descriptorCount = 1;
    descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_writes[1].pBufferInfo = &descriptor_buffer_info;

    descriptor_writes[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_writes[2].dstSet = ds.set_;
    descriptor_writes[2].dstBinding = 2;
    if (descriptor_indexing) {
        descriptor_writes[2].descriptorCount = 5;  // Intentionally don't write index 5
    } else {
        descriptor_writes[2].descriptorCount = 6;
    }
    descriptor_writes[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_writes[2].pImageInfo = descriptor_image_infos;
    vk::UpdateDescriptorSets(m_device->device(), 3, descriptor_writes, 0, NULL);
    if (descriptor_indexing) {
        descriptor_writes[0].dstSet = ds_variable.set_;
        descriptor_writes[1].dstSet = ds_variable.set_;
        descriptor_writes[2].dstSet = ds_variable.set_;
        vk::UpdateDescriptorSets(m_device->device(), 3, descriptor_writes, 0, NULL);
    }

    const VkPipelineLayoutObj pipeline_layout(m_device, {&ds.layout_});
    const VkPipelineLayoutObj pipeline_layout_variable(m_device, {&ds_variable.layout_});

    const auto SetImagesArrayLength = [](const std::string &shader_template, const std::string &length_str) {
        const std::string to_replace = "IMAGES_ARRAY_LENGTH";

        std::string result = shader_template;
        auto position = result.find(to_replace);
        assert(position != std::string::npos);
        result.replace(position, to_replace.length(), length_str);
        return result;
    };

    const std::string rgen_source_template = R"(#version 460
        #extension GL_EXT_nonuniform_qualifier : require
        #extension GL_EXT_samplerless_texture_functions : require
        #extension GL_NV_ray_tracing : require

        layout(set = 0, binding = 0) uniform accelerationStructureNV topLevelAS;
        layout(set = 0, binding = 1, std430) buffer RayTracingSbo {
	        uint rgen_index;
	        uint ahit_index;
	        uint chit_index;
	        uint miss_index;
	        uint intr_index;
	        uint call_index;

	        uint rgen_ran;
	        uint ahit_ran;
	        uint chit_ran;
	        uint miss_ran;
	        uint intr_ran;
	        uint call_ran;

	        float result1;
	        float result2;
	        float result3;
        } sbo;
        layout(set = 0, binding = 2) uniform texture2D textures[IMAGES_ARRAY_LENGTH];

        layout(location = 0) rayPayloadNV vec3 payload;
        layout(location = 3) callableDataNV vec3 callableData;

        void main() {
            sbo.rgen_ran = 1;

	        executeCallableNV(0, 3);
	        sbo.result1 = callableData.x;

	        vec3 origin = vec3(0.0f, 0.0f, -2.0f);
	        vec3 direction = vec3(0.0f, 0.0f, 1.0f);

	        traceNV(topLevelAS, gl_RayFlagsNoneNV, 0xFF, 0, 1, 0, origin, 0.001, direction, 10000.0, 0);
	        sbo.result2 = payload.x;

	        traceNV(topLevelAS, gl_RayFlagsNoneNV, 0xFF, 0, 1, 0, origin, 0.001, -direction, 10000.0, 0);
	        sbo.result3 = payload.x;

            if (sbo.rgen_index > 0) {
                // OOB here:
                sbo.result3 = texelFetch(textures[sbo.rgen_index], ivec2(0, 0), 0).x;
            }
        }
        )";

    const std::string rgen_source = SetImagesArrayLength(rgen_source_template, "6");
    const std::string rgen_source_runtime = SetImagesArrayLength(rgen_source_template, "");

    const std::string ahit_source_template = R"(#version 460
        #extension GL_EXT_nonuniform_qualifier : require
        #extension GL_EXT_samplerless_texture_functions : require
        #extension GL_NV_ray_tracing : require

        layout(set = 0, binding = 1, std430) buffer StorageBuffer {
	        uint rgen_index;
	        uint ahit_index;
	        uint chit_index;
	        uint miss_index;
	        uint intr_index;
	        uint call_index;

	        uint rgen_ran;
	        uint ahit_ran;
	        uint chit_ran;
	        uint miss_ran;
	        uint intr_ran;
	        uint call_ran;

	        float result1;
	        float result2;
	        float result3;
        } sbo;
        layout(set = 0, binding = 2) uniform texture2D textures[IMAGES_ARRAY_LENGTH];

        hitAttributeNV vec3 hitValue;

        layout(location = 0) rayPayloadInNV vec3 payload;

        void main() {
	        sbo.ahit_ran = 2;

	        payload = vec3(0.1234f);

            if (sbo.ahit_index > 0) {
                // OOB here:
                payload.x = texelFetch(textures[sbo.ahit_index], ivec2(0, 0), 0).x;
            }
        }
    )";
    const std::string ahit_source = SetImagesArrayLength(ahit_source_template, "6");
    const std::string ahit_source_runtime = SetImagesArrayLength(ahit_source_template, "");

    const std::string chit_source_template = R"(#version 460
        #extension GL_EXT_nonuniform_qualifier : require
        #extension GL_EXT_samplerless_texture_functions : require
        #extension GL_NV_ray_tracing : require

        layout(set = 0, binding = 1, std430) buffer RayTracingSbo {
	        uint rgen_index;
	        uint ahit_index;
	        uint chit_index;
	        uint miss_index;
	        uint intr_index;
	        uint call_index;

	        uint rgen_ran;
	        uint ahit_ran;
	        uint chit_ran;
	        uint miss_ran;
	        uint intr_ran;
	        uint call_ran;

	        float result1;
	        float result2;
	        float result3;
        } sbo;
        layout(set = 0, binding = 2) uniform texture2D textures[IMAGES_ARRAY_LENGTH];

        layout(location = 0) rayPayloadInNV vec3 payload;

        hitAttributeNV vec3 attribs;

        void main() {
            sbo.chit_ran = 3;

            payload = attribs;
            if (sbo.chit_index > 0) {
                // OOB here:
                payload.x = texelFetch(textures[sbo.chit_index], ivec2(0, 0), 0).x;
            }
        }
        )";
    const std::string chit_source = SetImagesArrayLength(chit_source_template, "6");
    const std::string chit_source_runtime = SetImagesArrayLength(chit_source_template, "");

    const std::string miss_source_template = R"(#version 460
        #extension GL_EXT_nonuniform_qualifier : enable
        #extension GL_EXT_samplerless_texture_functions : require
        #extension GL_NV_ray_tracing : require

        layout(set = 0, binding = 1, std430) buffer RayTracingSbo {
	        uint rgen_index;
	        uint ahit_index;
	        uint chit_index;
	        uint miss_index;
	        uint intr_index;
	        uint call_index;

	        uint rgen_ran;
	        uint ahit_ran;
	        uint chit_ran;
	        uint miss_ran;
	        uint intr_ran;
	        uint call_ran;

	        float result1;
	        float result2;
	        float result3;
        } sbo;
        layout(set = 0, binding = 2) uniform texture2D textures[IMAGES_ARRAY_LENGTH];

        layout(location = 0) rayPayloadInNV vec3 payload;

        void main() {
            sbo.miss_ran = 4;

            payload = vec3(1.0, 0.0, 0.0);

            if (sbo.miss_index > 0) {
                // OOB here:
                payload.x = texelFetch(textures[sbo.miss_index], ivec2(0, 0), 0).x;
            }
        }
    )";
    const std::string miss_source = SetImagesArrayLength(miss_source_template, "6");
    const std::string miss_source_runtime = SetImagesArrayLength(miss_source_template, "");

    const std::string intr_source_template = R"(#version 460
        #extension GL_EXT_nonuniform_qualifier : require
        #extension GL_EXT_samplerless_texture_functions : require
        #extension GL_NV_ray_tracing : require

        layout(set = 0, binding = 1, std430) buffer StorageBuffer {
	        uint rgen_index;
	        uint ahit_index;
	        uint chit_index;
	        uint miss_index;
	        uint intr_index;
	        uint call_index;

	        uint rgen_ran;
	        uint ahit_ran;
	        uint chit_ran;
	        uint miss_ran;
	        uint intr_ran;
	        uint call_ran;

	        float result1;
	        float result2;
	        float result3;
        } sbo;
        layout(set = 0, binding = 2) uniform texture2D textures[IMAGES_ARRAY_LENGTH];

        hitAttributeNV vec3 hitValue;

        void main() {
	        sbo.intr_ran = 5;

	        hitValue = vec3(0.0f, 0.5f, 0.0f);

	        reportIntersectionNV(1.0f, 0);

            if (sbo.intr_index > 0) {
                // OOB here:
                hitValue.x = texelFetch(textures[sbo.intr_index], ivec2(0, 0), 0).x;
            }
        }
    )";
    const std::string intr_source = SetImagesArrayLength(intr_source_template, "6");
    const std::string intr_source_runtime = SetImagesArrayLength(intr_source_template, "");

    const std::string call_source_template = R"(#version 460
        #extension GL_EXT_nonuniform_qualifier : require
        #extension GL_EXT_samplerless_texture_functions : require
        #extension GL_NV_ray_tracing : require

        layout(set = 0, binding = 1, std430) buffer StorageBuffer {
	        uint rgen_index;
	        uint ahit_index;
	        uint chit_index;
	        uint miss_index;
	        uint intr_index;
	        uint call_index;

	        uint rgen_ran;
	        uint ahit_ran;
	        uint chit_ran;
	        uint miss_ran;
	        uint intr_ran;
	        uint call_ran;

	        float result1;
	        float result2;
	        float result3;
        } sbo;
        layout(set = 0, binding = 2) uniform texture2D textures[IMAGES_ARRAY_LENGTH];

        layout(location = 3) callableDataInNV vec3 callableData;

        void main() {
	        sbo.call_ran = 6;

	        callableData = vec3(0.1234f);

            if (sbo.call_index > 0) {
                // OOB here:
                callableData.x = texelFetch(textures[sbo.call_index], ivec2(0, 0), 0).x;
            }
        }
    )";
    const std::string call_source = SetImagesArrayLength(call_source_template, "6");
    const std::string call_source_runtime = SetImagesArrayLength(call_source_template, "");

    struct TestCase {
        const std::string &rgen_shader_source;
        const std::string &ahit_shader_source;
        const std::string &chit_shader_source;
        const std::string &miss_shader_source;
        const std::string &intr_shader_source;
        const std::string &call_shader_source;
        bool variable_length;
        uint32_t rgen_index;
        uint32_t ahit_index;
        uint32_t chit_index;
        uint32_t miss_index;
        uint32_t intr_index;
        uint32_t call_index;
        const char *expected_error;
    };

    std::vector<TestCase> tests;
    tests.push_back({rgen_source, ahit_source, chit_source, miss_source, intr_source, call_source, false, 25, 0, 0, 0, 0, 0,
                     "Index of 25 used to index descriptor array of length 6."});
    tests.push_back({rgen_source, ahit_source, chit_source, miss_source, intr_source, call_source, false, 0, 25, 0, 0, 0, 0,
                     "Index of 25 used to index descriptor array of length 6."});
    tests.push_back({rgen_source, ahit_source, chit_source, miss_source, intr_source, call_source, false, 0, 0, 25, 0, 0, 0,
                     "Index of 25 used to index descriptor array of length 6."});
    tests.push_back({rgen_source, ahit_source, chit_source, miss_source, intr_source, call_source, false, 0, 0, 0, 25, 0, 0,
                     "Index of 25 used to index descriptor array of length 6."});
    tests.push_back({rgen_source, ahit_source, chit_source, miss_source, intr_source, call_source, false, 0, 0, 0, 0, 25, 0,
                     "Index of 25 used to index descriptor array of length 6."});
    tests.push_back({rgen_source, ahit_source, chit_source, miss_source, intr_source, call_source, false, 0, 0, 0, 0, 0, 25,
                     "Index of 25 used to index descriptor array of length 6."});

    if (descriptor_indexing) {
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 25, 0, 0, 0, 0, 0, "Index of 25 used to index descriptor array of length 6."});
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 0, 25, 0, 0, 0, 0, "Index of 25 used to index descriptor array of length 6."});
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 0, 0, 25, 0, 0, 0, "Index of 25 used to index descriptor array of length 6."});
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 0, 0, 0, 25, 0, 0, "Index of 25 used to index descriptor array of length 6."});
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 0, 0, 0, 0, 25, 0, "Index of 25 used to index descriptor array of length 6."});
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 0, 0, 0, 0, 0, 25, "Index of 25 used to index descriptor array of length 6."});

        // For this group, 6 is less than max specified (max specified is 8) but more than actual specified (actual specified is 5)
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 6, 0, 0, 0, 0, 0, "Index of 6 used to index descriptor array of length 6."});
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 0, 6, 0, 0, 0, 0, "Index of 6 used to index descriptor array of length 6."});
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 0, 0, 6, 0, 0, 0, "Index of 6 used to index descriptor array of length 6."});
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 0, 0, 0, 6, 0, 0, "Index of 6 used to index descriptor array of length 6."});
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 0, 0, 0, 0, 6, 0, "Index of 6 used to index descriptor array of length 6."});
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 0, 0, 0, 0, 0, 6, "Index of 6 used to index descriptor array of length 6."});

        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 5, 0, 0, 0, 0, 0, "Descriptor index 5 is uninitialized."});
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 0, 5, 0, 0, 0, 0, "Descriptor index 5 is uninitialized."});
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 0, 0, 5, 0, 0, 0, "Descriptor index 5 is uninitialized."});
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 0, 0, 0, 5, 0, 0, "Descriptor index 5 is uninitialized."});
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 0, 0, 0, 0, 5, 0, "Descriptor index 5 is uninitialized."});
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 0, 0, 0, 0, 0, 5, "Descriptor index 5 is uninitialized."});
    }

    PFN_vkCreateRayTracingPipelinesNV vkCreateRayTracingPipelinesNV = reinterpret_cast<PFN_vkCreateRayTracingPipelinesNV>(
        vk::GetDeviceProcAddr(m_device->handle(), "vkCreateRayTracingPipelinesNV"));
    ASSERT_TRUE(vkCreateRayTracingPipelinesNV != nullptr);

    PFN_vkGetRayTracingShaderGroupHandlesNV vkGetRayTracingShaderGroupHandlesNV =
        reinterpret_cast<PFN_vkGetRayTracingShaderGroupHandlesNV>(
            vk::GetDeviceProcAddr(m_device->handle(), "vkGetRayTracingShaderGroupHandlesNV"));
    ASSERT_TRUE(vkGetRayTracingShaderGroupHandlesNV != nullptr);

    PFN_vkCmdTraceRaysNV vkCmdTraceRaysNV =
        reinterpret_cast<PFN_vkCmdTraceRaysNV>(vk::GetDeviceProcAddr(m_device->handle(), "vkCmdTraceRaysNV"));
    ASSERT_TRUE(vkCmdTraceRaysNV != nullptr);

    // Iteration 0 tests with no descriptor set bound (to sanity test "draw" validation). Iteration 1
    // tests what's in the test case vector.
    for (const auto &test : tests) {
        if (gpu_assisted) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, test.expected_error);
        }

        VkShaderObj rgen_shader(m_device, test.rgen_shader_source.c_str(), VK_SHADER_STAGE_RAYGEN_BIT_NV, this, "main");
        VkShaderObj ahit_shader(m_device, test.ahit_shader_source.c_str(), VK_SHADER_STAGE_ANY_HIT_BIT_NV, this, "main");
        VkShaderObj chit_shader(m_device, test.chit_shader_source.c_str(), VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV, this, "main");
        VkShaderObj miss_shader(m_device, test.miss_shader_source.c_str(), VK_SHADER_STAGE_MISS_BIT_NV, this, "main");
        VkShaderObj intr_shader(m_device, test.intr_shader_source.c_str(), VK_SHADER_STAGE_INTERSECTION_BIT_NV, this, "main");
        VkShaderObj call_shader(m_device, test.call_shader_source.c_str(), VK_SHADER_STAGE_CALLABLE_BIT_NV, this, "main");

        VkPipelineShaderStageCreateInfo stage_create_infos[6] = {};
        stage_create_infos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_NV;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_create_infos[1].stage = VK_SHADER_STAGE_ANY_HIT_BIT_NV;
        stage_create_infos[1].module = ahit_shader.handle();
        stage_create_infos[1].pName = "main";

        stage_create_infos[2].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_create_infos[2].stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;
        stage_create_infos[2].module = chit_shader.handle();
        stage_create_infos[2].pName = "main";

        stage_create_infos[3].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_create_infos[3].stage = VK_SHADER_STAGE_MISS_BIT_NV;
        stage_create_infos[3].module = miss_shader.handle();
        stage_create_infos[3].pName = "main";

        stage_create_infos[4].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_create_infos[4].stage = VK_SHADER_STAGE_INTERSECTION_BIT_NV;
        stage_create_infos[4].module = intr_shader.handle();
        stage_create_infos[4].pName = "main";

        stage_create_infos[5].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_create_infos[5].stage = VK_SHADER_STAGE_CALLABLE_BIT_NV;
        stage_create_infos[5].module = call_shader.handle();
        stage_create_infos[5].pName = "main";

        VkRayTracingShaderGroupCreateInfoNV group_create_infos[4] = {};
        group_create_infos[0].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        group_create_infos[0].generalShader = 0;  // rgen
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_NV;

        group_create_infos[1].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        group_create_infos[1].generalShader = 3;  // miss
        group_create_infos[1].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].intersectionShader = VK_SHADER_UNUSED_NV;

        group_create_infos[2].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
        group_create_infos[2].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_NV;
        group_create_infos[2].generalShader = VK_SHADER_UNUSED_NV;
        group_create_infos[2].closestHitShader = 2;
        group_create_infos[2].anyHitShader = 1;
        group_create_infos[2].intersectionShader = 4;

        group_create_infos[3].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
        group_create_infos[3].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        group_create_infos[3].generalShader = 5;  // call
        group_create_infos[3].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[3].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[3].intersectionShader = VK_SHADER_UNUSED_NV;

        VkRayTracingPipelineCreateInfoNV pipeline_ci = {};
        pipeline_ci.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV;
        pipeline_ci.stageCount = 6;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 4;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.maxRecursionDepth = 2;
        pipeline_ci.layout = test.variable_length ? pipeline_layout_variable.handle() : pipeline_layout.handle();

        VkPipeline pipeline = VK_NULL_HANDLE;
        ASSERT_VK_SUCCESS(vkCreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline));

        std::vector<uint8_t> shader_binding_table_data;
        shader_binding_table_data.resize(static_cast<std::size_t>(shader_binding_table_buffer_size), 0);
        ASSERT_VK_SUCCESS(vkGetRayTracingShaderGroupHandlesNV(m_device->handle(), pipeline, 0, 4,
                                                              static_cast<std::size_t>(shader_binding_table_buffer_size),
                                                              shader_binding_table_data.data()));

        uint8_t *mapped_shader_binding_table_data = (uint8_t *)shader_binding_table_buffer.memory().map();
        std::memcpy(mapped_shader_binding_table_data, shader_binding_table_data.data(), shader_binding_table_data.size());
        shader_binding_table_buffer.memory().unmap();

        ray_tracing_command_buffer.begin();

        vk::CmdBindPipeline(ray_tracing_command_buffer.handle(), VK_PIPELINE_BIND_POINT_RAY_TRACING_NV, pipeline);

        if (gpu_assisted) {
            vk::CmdBindDescriptorSets(ray_tracing_command_buffer.handle(), VK_PIPELINE_BIND_POINT_RAY_TRACING_NV,
                                      test.variable_length ? pipeline_layout_variable.handle() : pipeline_layout.handle(), 0, 1,
                                      test.variable_length ? &ds_variable.set_ : &ds.set_, 0, nullptr);
        } else {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysNV-None-02697");
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-DescriptorSetNotBound");
        }

        if (gpu_assisted) {
            // Need these values to pass mapped storage buffer checks
            vkCmdTraceRaysNV(ray_tracing_command_buffer.handle(), shader_binding_table_buffer.handle(),
                             ray_tracing_properties.shaderGroupHandleSize * 0ull, shader_binding_table_buffer.handle(),
                             ray_tracing_properties.shaderGroupHandleSize * 1ull, ray_tracing_properties.shaderGroupHandleSize,
                             shader_binding_table_buffer.handle(), ray_tracing_properties.shaderGroupHandleSize * 2ull,
                             ray_tracing_properties.shaderGroupHandleSize, shader_binding_table_buffer.handle(),
                             ray_tracing_properties.shaderGroupHandleSize * 3ull, ray_tracing_properties.shaderGroupHandleSize,
                             /*width=*/1, /*height=*/1, /*depth=*/1);
        } else {
            // offset shall be multiple of shaderGroupBaseAlignment and stride of shaderGroupHandleSize
            vkCmdTraceRaysNV(ray_tracing_command_buffer.handle(), shader_binding_table_buffer.handle(),
                             ray_tracing_properties.shaderGroupBaseAlignment * 0ull, shader_binding_table_buffer.handle(),
                             ray_tracing_properties.shaderGroupBaseAlignment * 1ull, ray_tracing_properties.shaderGroupHandleSize,
                             shader_binding_table_buffer.handle(), ray_tracing_properties.shaderGroupBaseAlignment * 2ull,
                             ray_tracing_properties.shaderGroupHandleSize, shader_binding_table_buffer.handle(),
                             ray_tracing_properties.shaderGroupBaseAlignment * 3ull, ray_tracing_properties.shaderGroupHandleSize,
                             /*width=*/1, /*height=*/1, /*depth=*/1);
        }

        ray_tracing_command_buffer.end();
        // Update the index of the texture that the shaders should read
        uint32_t *mapped_storage_buffer_data = (uint32_t *)storage_buffer.memory().map();
        mapped_storage_buffer_data[0] = test.rgen_index;
        mapped_storage_buffer_data[1] = test.ahit_index;
        mapped_storage_buffer_data[2] = test.chit_index;
        mapped_storage_buffer_data[3] = test.miss_index;
        mapped_storage_buffer_data[4] = test.intr_index;
        mapped_storage_buffer_data[5] = test.call_index;
        mapped_storage_buffer_data[6] = 0;
        mapped_storage_buffer_data[7] = 0;
        mapped_storage_buffer_data[8] = 0;
        mapped_storage_buffer_data[9] = 0;
        mapped_storage_buffer_data[10] = 0;
        mapped_storage_buffer_data[11] = 0;
        storage_buffer.memory().unmap();

        vk::QueueSubmit(ray_tracing_queue, 1, &submit_info, VK_NULL_HANDLE);
        vk::QueueWaitIdle(ray_tracing_queue);
        m_errorMonitor->VerifyFound();

        if (gpu_assisted) {
            mapped_storage_buffer_data = (uint32_t *)storage_buffer.memory().map();
            ASSERT_TRUE(mapped_storage_buffer_data[6] == 1);
            ASSERT_TRUE(mapped_storage_buffer_data[7] == 2);
            ASSERT_TRUE(mapped_storage_buffer_data[8] == 3);
            ASSERT_TRUE(mapped_storage_buffer_data[9] == 4);
            ASSERT_TRUE(mapped_storage_buffer_data[10] == 5);
            ASSERT_TRUE(mapped_storage_buffer_data[11] == 6);
            storage_buffer.memory().unmap();
        } else {
            ray_tracing_command_buffer.begin();
            vk::CmdBindPipeline(ray_tracing_command_buffer.handle(), VK_PIPELINE_BIND_POINT_RAY_TRACING_NV, pipeline);
            vk::CmdBindDescriptorSets(ray_tracing_command_buffer.handle(), VK_PIPELINE_BIND_POINT_RAY_TRACING_NV,
                                      test.variable_length ? pipeline_layout_variable.handle() : pipeline_layout.handle(), 0, 1,
                                      test.variable_length ? &ds_variable.set_ : &ds.set_, 0, nullptr);

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysNV-callableShaderBindingOffset-02462");
            VkDeviceSize stride_align = ray_tracing_properties.shaderGroupHandleSize;
            VkDeviceSize invalid_max_stride = ray_tracing_properties.maxShaderGroupStride +
                                              (stride_align - (ray_tracing_properties.maxShaderGroupStride %
                                                               stride_align));  // should be less than maxShaderGroupStride
            VkDeviceSize invalid_stride =
                ray_tracing_properties.shaderGroupHandleSize >> 1;  // should  be multiple of shaderGroupHandleSize
            VkDeviceSize invalid_offset =
                ray_tracing_properties.shaderGroupBaseAlignment >> 1;  // should be multiple of shaderGroupBaseAlignment

            vkCmdTraceRaysNV(ray_tracing_command_buffer.handle(), shader_binding_table_buffer.handle(),
                             ray_tracing_properties.shaderGroupBaseAlignment * 0ull, shader_binding_table_buffer.handle(),
                             ray_tracing_properties.shaderGroupBaseAlignment * 1ull, ray_tracing_properties.shaderGroupHandleSize,
                             shader_binding_table_buffer.handle(), ray_tracing_properties.shaderGroupBaseAlignment * 2ull,
                             ray_tracing_properties.shaderGroupHandleSize, shader_binding_table_buffer.handle(), invalid_offset,
                             ray_tracing_properties.shaderGroupHandleSize,
                             /*width=*/1, /*height=*/1, /*depth=*/1);
            m_errorMonitor->VerifyFound();

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysNV-callableShaderBindingStride-02465");
            vkCmdTraceRaysNV(ray_tracing_command_buffer.handle(), shader_binding_table_buffer.handle(),
                             ray_tracing_properties.shaderGroupBaseAlignment * 0ull, shader_binding_table_buffer.handle(),
                             ray_tracing_properties.shaderGroupBaseAlignment * 1ull, ray_tracing_properties.shaderGroupHandleSize,
                             shader_binding_table_buffer.handle(), ray_tracing_properties.shaderGroupBaseAlignment * 2ull,
                             ray_tracing_properties.shaderGroupHandleSize, shader_binding_table_buffer.handle(),
                             ray_tracing_properties.shaderGroupBaseAlignment, invalid_stride,
                             /*width=*/1, /*height=*/1, /*depth=*/1);
            m_errorMonitor->VerifyFound();

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysNV-callableShaderBindingStride-02468");
            vkCmdTraceRaysNV(ray_tracing_command_buffer.handle(), shader_binding_table_buffer.handle(),
                             ray_tracing_properties.shaderGroupBaseAlignment * 0ull, shader_binding_table_buffer.handle(),
                             ray_tracing_properties.shaderGroupBaseAlignment * 1ull, ray_tracing_properties.shaderGroupHandleSize,
                             shader_binding_table_buffer.handle(), ray_tracing_properties.shaderGroupBaseAlignment * 2ull,
                             ray_tracing_properties.shaderGroupHandleSize, shader_binding_table_buffer.handle(),
                             ray_tracing_properties.shaderGroupBaseAlignment, invalid_max_stride,
                             /*width=*/1, /*height=*/1, /*depth=*/1);
            m_errorMonitor->VerifyFound();

            // hit shader
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysNV-hitShaderBindingOffset-02460");
            vkCmdTraceRaysNV(ray_tracing_command_buffer.handle(), shader_binding_table_buffer.handle(),
                             ray_tracing_properties.shaderGroupBaseAlignment * 0ull, shader_binding_table_buffer.handle(),
                             ray_tracing_properties.shaderGroupBaseAlignment * 1ull, ray_tracing_properties.shaderGroupHandleSize,
                             shader_binding_table_buffer.handle(), invalid_offset, ray_tracing_properties.shaderGroupHandleSize,
                             shader_binding_table_buffer.handle(), ray_tracing_properties.shaderGroupBaseAlignment,
                             ray_tracing_properties.shaderGroupHandleSize,
                             /*width=*/1, /*height=*/1, /*depth=*/1);
            m_errorMonitor->VerifyFound();

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysNV-hitShaderBindingStride-02464");
            vkCmdTraceRaysNV(ray_tracing_command_buffer.handle(), shader_binding_table_buffer.handle(),
                             ray_tracing_properties.shaderGroupBaseAlignment * 0ull, shader_binding_table_buffer.handle(),
                             ray_tracing_properties.shaderGroupBaseAlignment * 1ull, ray_tracing_properties.shaderGroupHandleSize,
                             shader_binding_table_buffer.handle(), ray_tracing_properties.shaderGroupBaseAlignment * 2ull,
                             invalid_stride, shader_binding_table_buffer.handle(), ray_tracing_properties.shaderGroupBaseAlignment,
                             ray_tracing_properties.shaderGroupHandleSize,
                             /*width=*/1, /*height=*/1, /*depth=*/1);
            m_errorMonitor->VerifyFound();

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysNV-hitShaderBindingStride-02467");
            vkCmdTraceRaysNV(ray_tracing_command_buffer.handle(), shader_binding_table_buffer.handle(),
                             ray_tracing_properties.shaderGroupBaseAlignment * 0ull, shader_binding_table_buffer.handle(),
                             ray_tracing_properties.shaderGroupBaseAlignment * 1ull, ray_tracing_properties.shaderGroupHandleSize,
                             shader_binding_table_buffer.handle(), ray_tracing_properties.shaderGroupBaseAlignment * 2ull,
                             invalid_max_stride, shader_binding_table_buffer.handle(),
                             ray_tracing_properties.shaderGroupBaseAlignment, ray_tracing_properties.shaderGroupHandleSize,
                             /*width=*/1, /*height=*/1, /*depth=*/1);
            m_errorMonitor->VerifyFound();

            // miss shader
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysNV-missShaderBindingOffset-02458");
            vkCmdTraceRaysNV(ray_tracing_command_buffer.handle(), shader_binding_table_buffer.handle(),
                             ray_tracing_properties.shaderGroupBaseAlignment * 0ull, shader_binding_table_buffer.handle(),
                             invalid_offset, ray_tracing_properties.shaderGroupHandleSize, shader_binding_table_buffer.handle(),
                             ray_tracing_properties.shaderGroupBaseAlignment * 2ull, ray_tracing_properties.shaderGroupHandleSize,
                             shader_binding_table_buffer.handle(), ray_tracing_properties.shaderGroupBaseAlignment,
                             ray_tracing_properties.shaderGroupHandleSize,
                             /*width=*/1, /*height=*/1, /*depth=*/1);
            m_errorMonitor->VerifyFound();

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysNV-missShaderBindingStride-02463");
            vkCmdTraceRaysNV(ray_tracing_command_buffer.handle(), shader_binding_table_buffer.handle(),
                             ray_tracing_properties.shaderGroupBaseAlignment * 0ull, shader_binding_table_buffer.handle(),
                             ray_tracing_properties.shaderGroupBaseAlignment * 1ull, invalid_stride,
                             shader_binding_table_buffer.handle(), ray_tracing_properties.shaderGroupBaseAlignment * 2ull,
                             ray_tracing_properties.shaderGroupHandleSize, shader_binding_table_buffer.handle(),
                             ray_tracing_properties.shaderGroupBaseAlignment, ray_tracing_properties.shaderGroupHandleSize,
                             /*width=*/1, /*height=*/1, /*depth=*/1);
            m_errorMonitor->VerifyFound();

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysNV-missShaderBindingStride-02466");
            vkCmdTraceRaysNV(ray_tracing_command_buffer.handle(), shader_binding_table_buffer.handle(),
                             ray_tracing_properties.shaderGroupBaseAlignment * 0ull, shader_binding_table_buffer.handle(),
                             ray_tracing_properties.shaderGroupBaseAlignment * 1ull, invalid_max_stride,
                             shader_binding_table_buffer.handle(), ray_tracing_properties.shaderGroupBaseAlignment * 2ull,
                             ray_tracing_properties.shaderGroupHandleSize, shader_binding_table_buffer.handle(),
                             ray_tracing_properties.shaderGroupBaseAlignment, ray_tracing_properties.shaderGroupHandleSize,
                             /*width=*/1, /*height=*/1, /*depth=*/1);
            m_errorMonitor->VerifyFound();

            // raygenshader
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysNV-raygenShaderBindingOffset-02456");
            vkCmdTraceRaysNV(ray_tracing_command_buffer.handle(), shader_binding_table_buffer.handle(), invalid_offset,
                             shader_binding_table_buffer.handle(), ray_tracing_properties.shaderGroupBaseAlignment * 1ull,
                             ray_tracing_properties.shaderGroupHandleSize, shader_binding_table_buffer.handle(),
                             ray_tracing_properties.shaderGroupBaseAlignment * 2ull, ray_tracing_properties.shaderGroupHandleSize,
                             shader_binding_table_buffer.handle(), ray_tracing_properties.shaderGroupBaseAlignment,
                             ray_tracing_properties.shaderGroupHandleSize,
                             /*width=*/1, /*height=*/1, /*depth=*/1);

            m_errorMonitor->VerifyFound();
            const auto &limits = m_device->props.limits;

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysNV-width-02469");
            uint32_t invalid_width = limits.maxComputeWorkGroupCount[0] + 1;
            vkCmdTraceRaysNV(ray_tracing_command_buffer.handle(), shader_binding_table_buffer.handle(),
                             ray_tracing_properties.shaderGroupBaseAlignment * 0ull, shader_binding_table_buffer.handle(),
                             ray_tracing_properties.shaderGroupBaseAlignment * 1ull, ray_tracing_properties.shaderGroupHandleSize,
                             shader_binding_table_buffer.handle(), ray_tracing_properties.shaderGroupBaseAlignment * 2ull,
                             ray_tracing_properties.shaderGroupHandleSize, shader_binding_table_buffer.handle(),
                             ray_tracing_properties.shaderGroupBaseAlignment, ray_tracing_properties.shaderGroupHandleSize,
                             /*width=*/invalid_width, /*height=*/1, /*depth=*/1);
            m_errorMonitor->VerifyFound();

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysNV-height-02470");
            uint32_t invalid_height = limits.maxComputeWorkGroupCount[1] + 1;
            vkCmdTraceRaysNV(ray_tracing_command_buffer.handle(), shader_binding_table_buffer.handle(),
                             ray_tracing_properties.shaderGroupBaseAlignment * 0ull, shader_binding_table_buffer.handle(),
                             ray_tracing_properties.shaderGroupBaseAlignment * 1ull, ray_tracing_properties.shaderGroupHandleSize,
                             shader_binding_table_buffer.handle(), ray_tracing_properties.shaderGroupBaseAlignment * 2ull,
                             ray_tracing_properties.shaderGroupHandleSize, shader_binding_table_buffer.handle(),
                             ray_tracing_properties.shaderGroupBaseAlignment, ray_tracing_properties.shaderGroupHandleSize,
                             /*width=*/1, /*height=*/invalid_height, /*depth=*/1);
            m_errorMonitor->VerifyFound();

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysNV-depth-02471");
            uint32_t invalid_depth = limits.maxComputeWorkGroupCount[2] + 1;
            vkCmdTraceRaysNV(ray_tracing_command_buffer.handle(), shader_binding_table_buffer.handle(),
                             ray_tracing_properties.shaderGroupBaseAlignment * 0ull, shader_binding_table_buffer.handle(),
                             ray_tracing_properties.shaderGroupBaseAlignment * 1ull, ray_tracing_properties.shaderGroupHandleSize,
                             shader_binding_table_buffer.handle(), ray_tracing_properties.shaderGroupBaseAlignment * 2ull,
                             ray_tracing_properties.shaderGroupHandleSize, shader_binding_table_buffer.handle(),
                             ray_tracing_properties.shaderGroupBaseAlignment, ray_tracing_properties.shaderGroupHandleSize,
                             /*width=*/1, /*height=*/1, /*depth=*/invalid_depth);
            m_errorMonitor->VerifyFound();

            ray_tracing_command_buffer.end();
        }
        vk::DestroyPipeline(m_device->handle(), pipeline, nullptr);
    }
}

void VkSyncValTest::InitSyncValFramework() {
    // Enable synchronization validation
    InitFramework(m_errorMonitor, &features_);
}

void print_android(const char *c) {
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    __android_log_print(ANDROID_LOG_INFO, "VulkanLayerValidationTests", "%s", c);
#endif  // VK_USE_PLATFORM_ANDROID_KHR
}

#if defined(ANDROID) && defined(VALIDATION_APK)
const char *appTag = "VulkanLayerValidationTests";
static bool initialized = false;
static bool active = false;

// Convert Intents to argv
// Ported from Hologram sample, only difference is flexible key
std::vector<std::string> get_args(android_app &app, const char *intent_extra_data_key) {
    std::vector<std::string> args;
    JavaVM &vm = *app.activity->vm;
    JNIEnv *p_env;
    if (vm.AttachCurrentThread(&p_env, nullptr) != JNI_OK) return args;

    JNIEnv &env = *p_env;
    jobject activity = app.activity->clazz;
    jmethodID get_intent_method = env.GetMethodID(env.GetObjectClass(activity), "getIntent", "()Landroid/content/Intent;");
    jobject intent = env.CallObjectMethod(activity, get_intent_method);
    jmethodID get_string_extra_method =
        env.GetMethodID(env.GetObjectClass(intent), "getStringExtra", "(Ljava/lang/String;)Ljava/lang/String;");
    jvalue get_string_extra_args;
    get_string_extra_args.l = env.NewStringUTF(intent_extra_data_key);
    jstring extra_str = static_cast<jstring>(env.CallObjectMethodA(intent, get_string_extra_method, &get_string_extra_args));

    std::string args_str;
    if (extra_str) {
        const char *extra_utf = env.GetStringUTFChars(extra_str, nullptr);
        args_str = extra_utf;
        env.ReleaseStringUTFChars(extra_str, extra_utf);
        env.DeleteLocalRef(extra_str);
    }

    env.DeleteLocalRef(get_string_extra_args.l);
    env.DeleteLocalRef(intent);
    vm.DetachCurrentThread();

    // split args_str
    std::stringstream ss(args_str);
    std::string arg;
    while (std::getline(ss, arg, ' ')) {
        if (!arg.empty()) args.push_back(arg);
    }

    return args;
}

void addFullTestCommentIfPresent(const ::testing::TestInfo &test_info, std::string &error_message) {
    const char *const type_param = test_info.type_param();
    const char *const value_param = test_info.value_param();

    if (type_param != NULL || value_param != NULL) {
        error_message.append(", where ");
        if (type_param != NULL) {
            error_message.append("TypeParam = ").append(type_param);
            if (value_param != NULL) error_message.append(" and ");
        }
        if (value_param != NULL) {
            error_message.append("GetParam() = ").append(value_param);
        }
    }
}

// Inspired by https://github.com/google/googletest/blob/master/googletest/docs/AdvancedGuide.md
class LogcatPrinter : public ::testing::EmptyTestEventListener {
    // Called before a test starts.
    virtual void OnTestStart(const ::testing::TestInfo &test_info) {
        __android_log_print(ANDROID_LOG_INFO, appTag, "[ RUN      ] %s.%s", test_info.test_case_name(), test_info.name());
    }

    // Called after a failed assertion or a SUCCEED() invocation.
    virtual void OnTestPartResult(const ::testing::TestPartResult &result) {
        // If the test part succeeded, we don't need to do anything.
        if (result.type() == ::testing::TestPartResult::kSuccess) return;

        __android_log_print(ANDROID_LOG_INFO, appTag, "%s in %s:%d %s", result.failed() ? "*** Failure" : "Success",
                            result.file_name(), result.line_number(), result.summary());
    }

    // Called after a test ends.
    virtual void OnTestEnd(const ::testing::TestInfo &info) {
        std::string result;
        if (info.result()->Passed()) {
            result.append("[       OK ]");
        } else {
            result.append("[  FAILED  ]");
        }
        result.append(info.test_case_name()).append(".").append(info.name());
        if (info.result()->Failed()) addFullTestCommentIfPresent(info, result);

        if (::testing::GTEST_FLAG(print_time)) {
            std::ostringstream os;
            os << info.result()->elapsed_time();
            result.append(" (").append(os.str()).append(" ms)");
        }

        __android_log_print(ANDROID_LOG_INFO, appTag, "%s", result.c_str());
    };
};

static int32_t processInput(struct android_app *app, AInputEvent *event) { return 0; }

static void processCommand(struct android_app *app, int32_t cmd) {
    switch (cmd) {
        case APP_CMD_INIT_WINDOW: {
            if (app->window) {
                initialized = true;
                VkTestFramework::window = app->window;
            }
            break;
        }
        case APP_CMD_GAINED_FOCUS: {
            active = true;
            break;
        }
        case APP_CMD_LOST_FOCUS: {
            active = false;
            break;
        }
    }
}

void android_main(struct android_app *app) {
    app->onAppCmd = processCommand;
    app->onInputEvent = processInput;

    while (1) {
        int events;
        struct android_poll_source *source;
        while (ALooper_pollAll(active ? 0 : -1, NULL, &events, (void **)&source) >= 0) {
            if (source) {
                source->process(app, source);
            }

            if (app->destroyRequested != 0) {
                VkTestFramework::Finish();
                return;
            }
        }

        if (initialized && active) {
            // Use the following key to send arguments to gtest, i.e.
            // --es args "--gtest_filter=-VkLayerTest.foo"
            const char key[] = "args";
            std::vector<std::string> args = get_args(*app, key);

            std::string filter = "";
            if (args.size() > 0) {
                __android_log_print(ANDROID_LOG_INFO, appTag, "Intent args = %s", args[0].c_str());
                filter += args[0];
            } else {
                __android_log_print(ANDROID_LOG_INFO, appTag, "No Intent args detected");
            }

            int argc = 2;
            char *argv[] = {(char *)"foo", (char *)filter.c_str()};
            __android_log_print(ANDROID_LOG_DEBUG, appTag, "filter = %s", argv[1]);

            // Route output to files until we can override the gtest output
            freopen("/sdcard/Android/data/com.example.VulkanLayerValidationTests/files/out.txt", "w", stdout);
            freopen("/sdcard/Android/data/com.example.VulkanLayerValidationTests/files/err.txt", "w", stderr);

            ::testing::InitGoogleTest(&argc, argv);

            ::testing::TestEventListeners &listeners = ::testing::UnitTest::GetInstance()->listeners();
            listeners.Append(new LogcatPrinter);

            VkTestFramework::InitArgs(&argc, argv);
            ::testing::AddGlobalTestEnvironment(new TestEnvironment);

            int result = RUN_ALL_TESTS();

            if (result != 0) {
                __android_log_print(ANDROID_LOG_INFO, appTag, "==== Tests FAILED ====");
            } else {
                __android_log_print(ANDROID_LOG_INFO, appTag, "==== Tests PASSED ====");
            }

            VkTestFramework::Finish();

            fclose(stdout);
            fclose(stderr);

            ANativeActivity_finish(app->activity);
            return;
        }
    }
}
#endif

#if defined(_WIN32) && !defined(NDEBUG)
#include <crtdbg.h>
#endif

int main(int argc, char **argv) {
    int result;

#if defined(_WIN32)
#if !defined(NDEBUG)
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
#endif
    // Avoid "Abort, Retry, Ignore" dialog boxes
    _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
#endif

    ::testing::InitGoogleTest(&argc, argv);
    VkTestFramework::InitArgs(&argc, argv);

    ::testing::AddGlobalTestEnvironment(new TestEnvironment);

    result = RUN_ALL_TESTS();

    VkTestFramework::Finish();
    return result;
}
