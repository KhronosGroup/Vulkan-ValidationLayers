/*
 * Copyright (c) 2015-2019 The Khronos Group Inc.
 * Copyright (c) 2015-2019 Valve Corporation
 * Copyright (c) 2015-2019 LunarG, Inc.
 * Copyright (c) 2015-2019 Google, Inc.
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

VkFormat FindSupportedDepthStencilFormat(VkPhysicalDevice phy) {
    const VkFormat ds_formats[] = {VK_FORMAT_D16_UNORM_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT_S8_UINT};
    for (uint32_t i = 0; i < size(ds_formats); ++i) {
        VkFormatProperties format_props;
        vkGetPhysicalDeviceFormatProperties(phy, ds_formats[i], &format_props);

        if (format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            return ds_formats[i];
        }
    }
    return VK_FORMAT_UNDEFINED;
}

bool ImageFormatIsSupported(VkPhysicalDevice phy, VkFormat format, VkImageTiling tiling, VkFormatFeatureFlags features) {
    VkFormatProperties format_props;
    vkGetPhysicalDeviceFormatProperties(phy, format, &format_props);
    VkFormatFeatureFlags phy_features =
        (VK_IMAGE_TILING_OPTIMAL == tiling ? format_props.optimalTilingFeatures : format_props.linearTilingFeatures);
    return (0 != (phy_features & features));
}

bool ImageFormatAndFeaturesSupported(VkPhysicalDevice phy, VkFormat format, VkImageTiling tiling, VkFormatFeatureFlags features) {
    VkFormatProperties format_props;
    vkGetPhysicalDeviceFormatProperties(phy, format, &format_props);
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
        vkGetPhysicalDeviceImageFormatProperties(phy, info.format, info.imageType, info.tiling, info.usage, info.flags, &props);
    if (VK_SUCCESS != err) {
        return false;
    }

#if 0  // Convinced this chunk doesn't currently add any additional info, but leaving in place because it may be
       // necessary with future extensions

    // Verify again using version 2, if supported, which *can* return more property data than the original...
    // (It's not clear that this is any more definitive than using the original version - but no harm)
    PFN_vkGetPhysicalDeviceImageFormatProperties2KHR p_GetPDIFP2KHR =
        (PFN_vkGetPhysicalDeviceImageFormatProperties2KHR)vkGetInstanceProcAddr(inst,
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

VKAPI_ATTR VkBool32 VKAPI_CALL myDbgFunc(VkFlags msgFlags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject, size_t location,
                                         int32_t msgCode, const char *pLayerPrefix, const char *pMsg, void *pUserData) {
    ErrorMonitor *errMonitor = (ErrorMonitor *)pUserData;
    if (msgFlags & errMonitor->GetMessageFlags()) {
        return errMonitor->CheckForDesiredMsg(pMsg);
    }
    return VK_FALSE;
}

VkPhysicalDevicePushDescriptorPropertiesKHR GetPushDescriptorProperties(VkInstance instance, VkPhysicalDevice gpu) {
    // Find address of extension call and make the call -- assumes needed extensions are enabled.
    PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2KHR =
        (PFN_vkGetPhysicalDeviceProperties2KHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceProperties2KHR");
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
    vkGetPhysicalDeviceProperties2(gpu, &prop2);
    return subgroup_prop;
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
        vkCmdSetEvent(data->commandBuffer, data->event, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
        if (data->bailout) {
            break;
        }
    }
    return NULL;
}
#endif  // GTEST_IS_THREADSAFE

extern "C" void *ReleaseNullFence(void *arg) {
    struct thread_data_struct *data = (struct thread_data_struct *)arg;

    for (int i = 0; i < 40000; i++) {
        vkDestroyFence(data->device, VK_NULL_HANDLE, NULL);
        if (data->bailout) {
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
        error_monitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, rp1_vuid);
        err = vkCreateRenderPass(device, create_info, nullptr, &render_pass);
        if (err == VK_SUCCESS) vkDestroyRenderPass(device, render_pass, nullptr);
        error_monitor->VerifyFound();
    }

    if (rp2_supported && rp2_vuid) {
        PFN_vkCreateRenderPass2KHR vkCreateRenderPass2KHR =
            (PFN_vkCreateRenderPass2KHR)vkGetDeviceProcAddr(device, "vkCreateRenderPass2KHR");
        safe_VkRenderPassCreateInfo2KHR create_info2;
        ConvertVkRenderPassCreateInfoToV2KHR(create_info, &create_info2);

        error_monitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, rp2_vuid);
        err = vkCreateRenderPass2KHR(device, create_info2.ptr(), nullptr, &render_pass);
        if (err == VK_SUCCESS) vkDestroyRenderPass(device, render_pass, nullptr);
        error_monitor->VerifyFound();
    }
}

void PositiveTestRenderPassCreate(ErrorMonitor *error_monitor, const VkDevice device, const VkRenderPassCreateInfo *create_info,
                                  bool rp2_supported) {
    VkRenderPass render_pass = VK_NULL_HANDLE;
    VkResult err;

    error_monitor->ExpectSuccess();
    err = vkCreateRenderPass(device, create_info, nullptr, &render_pass);
    if (err == VK_SUCCESS) vkDestroyRenderPass(device, render_pass, nullptr);
    error_monitor->VerifyNotFound();

    if (rp2_supported) {
        PFN_vkCreateRenderPass2KHR vkCreateRenderPass2KHR =
            (PFN_vkCreateRenderPass2KHR)vkGetDeviceProcAddr(device, "vkCreateRenderPass2KHR");
        safe_VkRenderPassCreateInfo2KHR create_info2;
        ConvertVkRenderPassCreateInfoToV2KHR(create_info, &create_info2);

        error_monitor->ExpectSuccess();
        err = vkCreateRenderPass2KHR(device, create_info2.ptr(), nullptr, &render_pass);
        if (err == VK_SUCCESS) vkDestroyRenderPass(device, render_pass, nullptr);
        error_monitor->VerifyNotFound();
    }
}

void TestRenderPass2KHRCreate(ErrorMonitor *error_monitor, const VkDevice device, const VkRenderPassCreateInfo2KHR *create_info,
                              const char *rp2_vuid) {
    VkRenderPass render_pass = VK_NULL_HANDLE;
    VkResult err;
    PFN_vkCreateRenderPass2KHR vkCreateRenderPass2KHR =
        (PFN_vkCreateRenderPass2KHR)vkGetDeviceProcAddr(device, "vkCreateRenderPass2KHR");

    error_monitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, rp2_vuid);
    err = vkCreateRenderPass2KHR(device, create_info, nullptr, &render_pass);
    if (err == VK_SUCCESS) vkDestroyRenderPass(device, render_pass, nullptr);
    error_monitor->VerifyFound();
}

void TestRenderPassBegin(ErrorMonitor *error_monitor, const VkDevice device, const VkCommandBuffer command_buffer,
                         const VkRenderPassBeginInfo *begin_info, bool rp2Supported, const char *rp1_vuid, const char *rp2_vuid) {
    VkCommandBufferBeginInfo cmd_begin_info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr,
                                               VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr};

    if (rp1_vuid) {
        vkBeginCommandBuffer(command_buffer, &cmd_begin_info);
        error_monitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, rp1_vuid);
        vkCmdBeginRenderPass(command_buffer, begin_info, VK_SUBPASS_CONTENTS_INLINE);
        error_monitor->VerifyFound();
        vkResetCommandBuffer(command_buffer, 0);
    }
    if (rp2Supported && rp2_vuid) {
        PFN_vkCmdBeginRenderPass2KHR vkCmdBeginRenderPass2KHR =
            (PFN_vkCmdBeginRenderPass2KHR)vkGetDeviceProcAddr(device, "vkCmdBeginRenderPass2KHR");
        VkSubpassBeginInfoKHR subpass_begin_info = {VK_STRUCTURE_TYPE_SUBPASS_BEGIN_INFO_KHR, nullptr, VK_SUBPASS_CONTENTS_INLINE};
        vkBeginCommandBuffer(command_buffer, &cmd_begin_info);
        error_monitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, rp2_vuid);
        vkCmdBeginRenderPass2KHR(command_buffer, begin_info, &subpass_begin_info);
        error_monitor->VerifyFound();
        vkResetCommandBuffer(command_buffer, 0);
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
    return vkGetPhysicalDeviceImageFormatProperties(dev, ci->format, ci->imageType, ci->tiling, ci->usage, ci->flags, limits);
}

VkFormat FindFormatLinearWithoutMips(VkPhysicalDevice gpu, VkImageCreateInfo image_ci) {
    image_ci.tiling = VK_IMAGE_TILING_LINEAR;

    const VkFormat first_vk_format = static_cast<VkFormat>(1);
    const VkFormat last_vk_format = static_cast<VkFormat>(130);  // avoid compressed/feature protected, otherwise 184

    for (VkFormat format = first_vk_format; format <= last_vk_format; format = static_cast<VkFormat>(format + 1)) {
        image_ci.format = format;

        // WORKAROUND for dev_sim and mock_icd not containing valid format limits yet
        VkFormatProperties format_props;
        vkGetPhysicalDeviceFormatProperties(gpu, format, &format_props);
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
        vkGetPhysicalDeviceFormatProperties(gpu, format, &format_props);
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
            vkGetPhysicalDeviceFormatProperties(gpu, format, &format_props);

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
        vkGetPhysicalDeviceFormatProperties(gpu, format, &format_props);

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
                m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                                     "is less than VkPhysicalDeviceLimits::viewportBoundsRange[0]");
            else
                m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, vuid);
        }
        vkCmdSetViewport(m_commandBuffer->handle(), 0, 1, &test_case.vp);
        m_errorMonitor->VerifyFound();
    }
}

void CreateSamplerTest(VkLayerTest &test, const VkSamplerCreateInfo *pCreateInfo, std::string code) {
    VkResult err;
    VkSampler sampler = VK_NULL_HANDLE;
    if (code.length())
        test.Monitor()->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT, code);
    else
        test.Monitor()->ExpectSuccess();

    err = vkCreateSampler(test.device(), pCreateInfo, NULL, &sampler);
    if (code.length())
        test.Monitor()->VerifyFound();
    else
        test.Monitor()->VerifyNotFound();

    if (VK_SUCCESS == err) {
        vkDestroySampler(test.device(), sampler, NULL);
    }
}

void CreateBufferTest(VkLayerTest &test, const VkBufferCreateInfo *pCreateInfo, std::string code) {
    VkResult err;
    VkBuffer buffer = VK_NULL_HANDLE;
    if (code.length())
        test.Monitor()->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, code);
    else
        test.Monitor()->ExpectSuccess();

    err = vkCreateBuffer(test.device(), pCreateInfo, NULL, &buffer);
    if (code.length())
        test.Monitor()->VerifyFound();
    else
        test.Monitor()->VerifyNotFound();

    if (VK_SUCCESS == err) {
        vkDestroyBuffer(test.device(), buffer, NULL);
    }
}

void CreateImageTest(VkLayerTest &test, const VkImageCreateInfo *pCreateInfo, std::string code) {
    VkResult err;
    VkImage image = VK_NULL_HANDLE;
    if (code.length())
        test.Monitor()->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, code);
    else
        test.Monitor()->ExpectSuccess();

    err = vkCreateImage(test.device(), pCreateInfo, NULL, &image);
    if (code.length())
        test.Monitor()->VerifyFound();
    else
        test.Monitor()->VerifyNotFound();

    if (VK_SUCCESS == err) {
        vkDestroyImage(test.device(), image, NULL);
    }
}

void CreateBufferViewTest(VkLayerTest &test, const VkBufferViewCreateInfo *pCreateInfo, const std::vector<std::string> &codes) {
    VkResult err;
    VkBufferView view = VK_NULL_HANDLE;
    if (codes.size())
        std::for_each(codes.begin(), codes.end(),
                      [&](const std::string &s) { test.Monitor()->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, s); });
    else
        test.Monitor()->ExpectSuccess();

    err = vkCreateBufferView(test.device(), pCreateInfo, NULL, &view);
    if (codes.size())
        test.Monitor()->VerifyFound();
    else
        test.Monitor()->VerifyNotFound();

    if (VK_SUCCESS == err) {
        vkDestroyBufferView(test.device(), view, NULL);
    }
}

void CreateImageViewTest(VkLayerTest &test, const VkImageViewCreateInfo *pCreateInfo, std::string code) {
    VkResult err;
    VkImageView view = VK_NULL_HANDLE;
    if (code.length())
        test.Monitor()->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, code);
    else
        test.Monitor()->ExpectSuccess();

    err = vkCreateImageView(test.device(), pCreateInfo, NULL, &view);
    if (code.length())
        test.Monitor()->VerifyFound();
    else
        test.Monitor()->VerifyNotFound();

    if (VK_SUCCESS == err) {
        vkDestroyImageView(test.device(), view, NULL);
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
    renderFramework->InitFramework(myDbgFunc, userData, features);
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

ErrorMonitor::ErrorMonitor() {
    test_platform_thread_create_mutex(&mutex_);
    test_platform_thread_lock_mutex(&mutex_);
    Reset();
    test_platform_thread_unlock_mutex(&mutex_);
}

ErrorMonitor::~ErrorMonitor() { test_platform_thread_delete_mutex(&mutex_); }

void ErrorMonitor::Reset() {
    message_flags_ = VK_DEBUG_REPORT_ERROR_BIT_EXT;
    bailout_ = NULL;
    message_found_ = VK_FALSE;
    failure_message_strings_.clear();
    desired_message_strings_.clear();
    ignore_message_strings_.clear();
    other_messages_.clear();
}

void ErrorMonitor::SetDesiredFailureMsg(const VkFlags msgFlags, const std::string msg) {
    SetDesiredFailureMsg(msgFlags, msg.c_str());
}

void ErrorMonitor::SetDesiredFailureMsg(const VkFlags msgFlags, const char *const msgString) {
    test_platform_thread_lock_mutex(&mutex_);
    desired_message_strings_.insert(msgString);
    message_flags_ |= msgFlags;
    test_platform_thread_unlock_mutex(&mutex_);
}

void ErrorMonitor::SetUnexpectedError(const char *const msg) {
    test_platform_thread_lock_mutex(&mutex_);

    ignore_message_strings_.emplace_back(msg);

    test_platform_thread_unlock_mutex(&mutex_);
}

VkBool32 ErrorMonitor::CheckForDesiredMsg(const char *const msgString) {
    VkBool32 result = VK_FALSE;
    test_platform_thread_lock_mutex(&mutex_);
    if (bailout_ != nullptr) {
        *bailout_ = true;
    }
    string errorString(msgString);
    bool found_expected = false;

    if (!IgnoreMessage(errorString)) {
        for (auto desired_msg_it = desired_message_strings_.begin(); desired_msg_it != desired_message_strings_.end();
             ++desired_msg_it) {
            if ((*desired_msg_it).length() == 0) {
                // An empty desired_msg string "" indicates a positive test - not expecting an error.
                // Return true to avoid calling layers/driver with this error.
                // And don't erase the "" string, so it remains if another error is found.
                result = VK_TRUE;
                found_expected = true;
                message_found_ = true;
                failure_message_strings_.insert(errorString);
            } else if (errorString.find(*desired_msg_it) != string::npos) {
                found_expected = true;
                failure_message_strings_.insert(errorString);
                message_found_ = true;
                result = VK_TRUE;
                // Remove a maximum of one failure message from the set
                // Multiset mutation is acceptable because `break` causes flow of control to exit the for loop
                desired_message_strings_.erase(desired_msg_it);
                break;
            }
        }

        if (!found_expected) {
            printf("Unexpected: %s\n", msgString);
            other_messages_.push_back(errorString);
        }
    }

    test_platform_thread_unlock_mutex(&mutex_);
    return result;
}

vector<string> ErrorMonitor::GetOtherFailureMsgs() const { return other_messages_; }

VkDebugReportFlagsEXT ErrorMonitor::GetMessageFlags() const { return message_flags_; }

bool ErrorMonitor::AnyDesiredMsgFound() const { return message_found_; }

bool ErrorMonitor::AllDesiredMsgsFound() const { return desired_message_strings_.empty(); }

void ErrorMonitor::SetError(const char *const errorString) {
    message_found_ = true;
    failure_message_strings_.insert(errorString);
}

void ErrorMonitor::SetBailout(bool *bailout) { bailout_ = bailout; }

void ErrorMonitor::DumpFailureMsgs() const {
    vector<string> otherMsgs = GetOtherFailureMsgs();
    if (otherMsgs.size()) {
        cout << "Other error messages logged for this test were:" << endl;
        for (auto iter = otherMsgs.begin(); iter != otherMsgs.end(); iter++) {
            cout << "     " << *iter << endl;
        }
    }
}

void ErrorMonitor::ExpectSuccess(VkDebugReportFlagsEXT const message_flag_mask) {
    // Match ANY message matching specified type
    SetDesiredFailureMsg(message_flag_mask, "");
    message_flags_ = message_flag_mask;  // override mask handling in SetDesired...
}

void ErrorMonitor::VerifyFound() {
    // Not receiving expected message(s) is a failure. /Before/ throwing, dump any other messages
    if (!AllDesiredMsgsFound()) {
        DumpFailureMsgs();
        for (const auto desired_msg : desired_message_strings_) {
            ADD_FAILURE() << "Did not receive expected error '" << desired_msg << "'";
        }
    } else if (GetOtherFailureMsgs().size() > 0) {
        // Fail test case for any unexpected errors
#if defined(ANDROID)
        // This will get unexpected errors into the adb log
        for (auto msg : other_messages_) {
            __android_log_print(ANDROID_LOG_INFO, "VulkanLayerValidationTests", "[ UNEXPECTED_ERR ] '%s'", msg.c_str());
        }
#else
        ADD_FAILURE() << "Received unexpected error(s).";
#endif
    }
    Reset();
}

void ErrorMonitor::VerifyNotFound() {
    // ExpectSuccess() configured us to match anything. Any error is a failure.
    if (AnyDesiredMsgFound()) {
        DumpFailureMsgs();
        for (const auto msg : failure_message_strings_) {
            ADD_FAILURE() << "Expected to succeed but got error: " << msg;
        }
    } else if (GetOtherFailureMsgs().size() > 0) {
        // Fail test case for any unexpected errors
#if defined(ANDROID)
        // This will get unexpected errors into the adb log
        for (auto msg : other_messages_) {
            __android_log_print(ANDROID_LOG_INFO, "VulkanLayerValidationTests", "[ UNEXPECTED_ERR ] '%s'", msg.c_str());
        }
#else
        ADD_FAILURE() << "Received unexpected error(s).";
#endif
    }
    Reset();
}

bool ErrorMonitor::IgnoreMessage(std::string const &msg) const {
    if (ignore_message_strings_.empty()) {
        return false;
    }

    return std::find_if(ignore_message_strings_.begin(), ignore_message_strings_.end(), [&msg](std::string const &str) {
               return msg.find(str) != std::string::npos;
           }) != ignore_message_strings_.end();
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

        vkCmdClearAttachments(m_commandBuffer->handle(), 1, &color_attachment, 1, &clear_rect);
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
    vkCmdBindPipeline(commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineobj.handle());
    commandBuffer->BindDescriptorSet(descriptorSet);
}

void VkLayerTest::Init(VkPhysicalDeviceFeatures *features, VkPhysicalDeviceFeatures2 *features2,
                       const VkCommandPoolCreateFlags flags, void *instance_pnext) {
    InitFramework(myDbgFunc, m_errorMonitor, instance_pnext);
    InitState(features, features2, flags);
}

ErrorMonitor *VkLayerTest::Monitor() { return m_errorMonitor; }

VkCommandBufferObj *VkLayerTest::CommandBuffer() { return m_commandBuffer; }

VkLayerTest::VkLayerTest() {
    m_enableWSI = false;

    m_instance_layer_names.clear();
    m_instance_extension_names.clear();
    m_device_extension_names.clear();

    // Add default instance extensions to the list
    m_instance_extension_names.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

    if (VkTestFramework::m_khronos_layer_disable) {
        m_instance_layer_names.push_back("VK_LAYER_GOOGLE_threading");
        m_instance_layer_names.push_back("VK_LAYER_LUNARG_parameter_validation");
        m_instance_layer_names.push_back("VK_LAYER_LUNARG_object_tracker");
        m_instance_layer_names.push_back("VK_LAYER_LUNARG_core_validation");
        m_instance_layer_names.push_back("VK_LAYER_GOOGLE_unique_objects");
    } else {
        m_instance_layer_names.push_back("VK_LAYER_KHRONOS_validation");
    }
    if (VkTestFramework::m_devsim_layer) {
        if (InstanceLayerSupported("VK_LAYER_LUNARG_device_simulation")) {
            m_instance_layer_names.push_back("VK_LAYER_LUNARG_device_simulation");
        } else {
            VkTestFramework::m_devsim_layer = false;
            printf("             Did not find VK_LAYER_LUNARG_device_simulation layer so it will not be enabled.\n");
        }
    }

    this->app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    this->app_info.pNext = NULL;
    this->app_info.pApplicationName = "layer_tests";
    this->app_info.applicationVersion = 1;
    this->app_info.pEngineName = "unittest";
    this->app_info.engineVersion = 1;
    this->app_info.apiVersion = VK_API_VERSION_1_0;

    m_errorMonitor = new ErrorMonitor;

    // Find out what version the instance supports and record the default target instance
    auto enumerateInstanceVersion = (PFN_vkEnumerateInstanceVersion)vkGetInstanceProcAddr(nullptr, "vkEnumerateInstanceVersion");
    if (enumerateInstanceVersion) {
        enumerateInstanceVersion(&m_instance_api_version);
    } else {
        m_instance_api_version = VK_API_VERSION_1_0;
    }
    m_target_api_version = app_info.apiVersion;
}

bool VkLayerTest::AddSurfaceInstanceExtension() {
    m_enableWSI = true;
    if (!InstanceExtensionSupported(VK_KHR_SURFACE_EXTENSION_NAME)) {
        printf("%s VK_KHR_SURFACE_EXTENSION_NAME extension not supported\n", kSkipPrefix);
        return false;
    }
    m_instance_extension_names.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

    bool bSupport = false;
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    if (!InstanceExtensionSupported(VK_KHR_WIN32_SURFACE_EXTENSION_NAME)) {
        printf("%s VK_KHR_WIN32_SURFACE_EXTENSION_NAME extension not supported\n", kSkipPrefix);
        return false;
    }
    m_instance_extension_names.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
    bSupport = true;
#endif

#if defined(VK_USE_PLATFORM_ANDROID_KHR) && defined(VALIDATION_APK)
    if (!InstanceExtensionSupported(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME)) {
        printf("%s VK_KHR_ANDROID_SURFACE_EXTENSION_NAME extension not supported\n", kSkipPrefix);
        return false;
    }
    m_instance_extension_names.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
    bSupport = true;
#endif

#if defined(VK_USE_PLATFORM_XLIB_KHR)
    if (!InstanceExtensionSupported(VK_KHR_XLIB_SURFACE_EXTENSION_NAME)) {
        printf("%s VK_KHR_XLIB_SURFACE_EXTENSION_NAME extension not supported\n", kSkipPrefix);
        return false;
    }
    if (XOpenDisplay(NULL)) {
        m_instance_extension_names.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
        bSupport = true;
    }
#endif

#if defined(VK_USE_PLATFORM_XCB_KHR)
    if (!InstanceExtensionSupported(VK_KHR_XCB_SURFACE_EXTENSION_NAME)) {
        printf("%s VK_KHR_XCB_SURFACE_EXTENSION_NAME extension not supported\n", kSkipPrefix);
        return false;
    }
    if (!bSupport && xcb_connect(NULL, NULL)) {
        m_instance_extension_names.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
        bSupport = true;
    }
#endif

    if (bSupport) return true;
    printf("%s No platform's surface extension supported\n", kSkipPrefix);
    return false;
}

bool VkLayerTest::AddSwapchainDeviceExtension() {
    if (!DeviceExtensionSupported(gpu(), nullptr, VK_KHR_SWAPCHAIN_EXTENSION_NAME)) {
        printf("%s VK_KHR_SWAPCHAIN_EXTENSION_NAME extension not supported\n", kSkipPrefix);
        return false;
    }
    m_device_extension_names.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    return true;
}

uint32_t VkLayerTest::SetTargetApiVersion(uint32_t target_api_version) {
    if (target_api_version == 0) target_api_version = VK_API_VERSION_1_0;
    if (target_api_version <= m_instance_api_version) {
        m_target_api_version = target_api_version;
        app_info.apiVersion = m_target_api_version;
    }
    return m_target_api_version;
}
uint32_t VkLayerTest::DeviceValidationVersion() {
    // The validation layers, assume the version we are validating to is the apiVersion unless the device apiVersion is lower
    VkPhysicalDeviceProperties props;
    GetPhysicalDeviceProperties(&props);
    return std::min(m_target_api_version, props.apiVersion);
}

bool VkLayerTest::LoadDeviceProfileLayer(
    PFN_vkSetPhysicalDeviceFormatPropertiesEXT &fpvkSetPhysicalDeviceFormatPropertiesEXT,
    PFN_vkGetOriginalPhysicalDeviceFormatPropertiesEXT &fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT) {
    // Load required functions
    fpvkSetPhysicalDeviceFormatPropertiesEXT =
        (PFN_vkSetPhysicalDeviceFormatPropertiesEXT)vkGetInstanceProcAddr(instance(), "vkSetPhysicalDeviceFormatPropertiesEXT");
    fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT = (PFN_vkGetOriginalPhysicalDeviceFormatPropertiesEXT)vkGetInstanceProcAddr(
        instance(), "vkGetOriginalPhysicalDeviceFormatPropertiesEXT");

    if (!(fpvkSetPhysicalDeviceFormatPropertiesEXT) || !(fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT)) {
        printf("%s Can't find device_profile_api functions; skipped.\n", kSkipPrefix);
        return 0;
    }

    return 1;
}

VkLayerTest::~VkLayerTest() {
    // Clean up resources before we reset
    delete m_errorMonitor;
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

        vkCreateBuffer(aVulkanDevice->device(), &buffer_create_info, nullptr, &vulkanBuffer);
        VkMemoryRequirements memory_reqs = {};

        vkGetBufferMemoryRequirements(aVulkanDevice->device(), vulkanBuffer, &memory_reqs);
        vkDestroyBuffer(aVulkanDevice->device(), vulkanBuffer, nullptr);
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
        vkAllocateMemory(VulkanDevice, &memory_allocate_info, nullptr, &VulkanMemory);

        VulkanBuffer = (aTestFlag == eBindNullBuffer) ? VK_NULL_HANDLE : (VkBuffer)0xCDCDCDCDCDCDCDCD;

        vkBindBufferMemory(VulkanDevice, VulkanBuffer, VulkanMemory, 0);
    } else {
        VkBufferCreateInfo buffer_create_info = {};
        buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_create_info.size = 32;
        buffer_create_info.usage = aBufferUsage;

        vkCreateBuffer(VulkanDevice, &buffer_create_info, nullptr, &VulkanBuffer);

        CreateCurrent = true;

        VkMemoryRequirements memory_requirements;
        vkGetBufferMemoryRequirements(VulkanDevice, VulkanBuffer, &memory_requirements);

        VkMemoryAllocateInfo memory_allocate_info = {};
        memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memory_allocate_info.allocationSize = memory_requirements.size + eOffsetAlignment;
        bool pass = aVulkanDevice->phy().set_memory_type(memory_requirements.memoryTypeBits, &memory_allocate_info,
                                                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        if (!pass) {
            CreateCurrent = false;
            vkDestroyBuffer(VulkanDevice, VulkanBuffer, nullptr);
            return;
        }

        vkAllocateMemory(VulkanDevice, &memory_allocate_info, NULL, &VulkanMemory);
        // NB: 1 is intentionally an invalid offset value
        const bool offset_en = eInvalidDeviceOffset == aTestFlag || eInvalidMemoryOffset == aTestFlag;
        vkBindBufferMemory(VulkanDevice, VulkanBuffer, VulkanMemory, offset_en ? eOffsetAlignment : 0);
        BoundCurrent = true;

        InvalidDeleteEn = (eFreeInvalidHandle == aTestFlag);
    }
}

VkBufferTest::~VkBufferTest() {
    if (CreateCurrent) {
        vkDestroyBuffer(VulkanDevice, VulkanBuffer, nullptr);
    }
    if (AllocateCurrent) {
        if (InvalidDeleteEn) {
            auto bad_memory = CastFromUint64<VkDeviceMemory>(CastToUint64(VulkanMemory) + 1);
            vkFreeMemory(VulkanDevice, bad_memory, nullptr);
        }
        vkFreeMemory(VulkanDevice, VulkanMemory, nullptr);
    }
}

bool VkBufferTest::GetBufferCurrent() { return AllocateCurrent && BoundCurrent && CreateCurrent; }

const VkBuffer &VkBufferTest::GetBuffer() { return VulkanBuffer; }

void VkBufferTest::TestDoubleDestroy() {
    // Destroy the buffer but leave the flag set, which will cause
    // the buffer to be destroyed again in the destructor.
    vkDestroyBuffer(VulkanDevice, VulkanBuffer, nullptr);
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

    vkCmdBindVertexBuffers(aCommandBuffer, BindId, offsetCount, &VulkanMemoryBuffer.handle(), offsetList);
    BoundCurrent = true;

    if (!aOffsetCount) {
        delete[] offsetList;
    }
}

OneOffDescriptorSet::OneOffDescriptorSet(VkDeviceObj *device, const Bindings &bindings,
                                         VkDescriptorSetLayoutCreateFlags layout_flags, void *layout_pnext,
                                         VkDescriptorPoolCreateFlags poolFlags, void *allocate_pnext)
    : device_{device}, pool_{}, layout_(device, bindings, layout_flags, layout_pnext), set_{} {
    VkResult err;

    std::vector<VkDescriptorPoolSize> sizes;
    for (const auto &b : bindings) sizes.push_back({b.descriptorType, std::max(1u, b.descriptorCount)});

    VkDescriptorPoolCreateInfo dspci = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO, nullptr, poolFlags, 1, uint32_t(sizes.size()), sizes.data()};
    err = vkCreateDescriptorPool(device_->handle(), &dspci, nullptr, &pool_);
    if (err != VK_SUCCESS) return;

    VkDescriptorSetAllocateInfo alloc_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, allocate_pnext, pool_, 1,
                                              &layout_.handle()};
    err = vkAllocateDescriptorSets(device_->handle(), &alloc_info, &set_);
}

OneOffDescriptorSet::~OneOffDescriptorSet() {
    // No need to destroy set-- it's going away with the pool.
    vkDestroyDescriptorPool(device_->handle(), pool_, nullptr);
}

bool OneOffDescriptorSet::Initialized() { return pool_ != VK_NULL_HANDLE && layout_.initialized() && set_ != VK_NULL_HANDLE; }

void OneOffDescriptorSet::WriteDescriptorBufferInfo(int blinding, VkBuffer buffer, VkDeviceSize size,
                                                    VkDescriptorType descriptorType) {
    VkDescriptorBufferInfo buffer_info = {};
    buffer_info.buffer = buffer;
    buffer_info.offset = 0;
    buffer_info.range = size;
    buffer_infos.emplace_back(buffer_info);
    size_t index = buffer_infos.size() - 1;

    VkWriteDescriptorSet descriptor_write;
    memset(&descriptor_write, 0, sizeof(descriptor_write));
    descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_write.dstSet = set_;
    descriptor_write.dstBinding = blinding;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = descriptorType;
    descriptor_write.pBufferInfo = &buffer_infos[index];
    descriptor_write.pImageInfo = nullptr;
    descriptor_write.pTexelBufferView = nullptr;

    descriptor_writes.emplace_back(descriptor_write);
}

void OneOffDescriptorSet::WriteDescriptorBufferView(int blinding, VkBufferView &buffer_view, VkDescriptorType descriptorType) {
    VkWriteDescriptorSet descriptor_write;
    memset(&descriptor_write, 0, sizeof(descriptor_write));
    descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_write.dstSet = set_;
    descriptor_write.dstBinding = blinding;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = descriptorType;
    descriptor_write.pTexelBufferView = &buffer_view;
    descriptor_write.pImageInfo = nullptr;
    descriptor_write.pBufferInfo = nullptr;

    descriptor_writes.emplace_back(descriptor_write);
}

void OneOffDescriptorSet::WriteDescriptorImageInfo(int blinding, VkImageView image_view, VkSampler sampler,
                                                   VkDescriptorType descriptorType) {
    VkDescriptorImageInfo image_info = {};
    image_info.imageView = image_view;
    image_info.sampler = sampler;
    image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    image_infos.emplace_back(image_info);
    size_t index = image_infos.size() - 1;

    VkWriteDescriptorSet descriptor_write;
    memset(&descriptor_write, 0, sizeof(descriptor_write));
    descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_write.dstSet = set_;
    descriptor_write.dstBinding = blinding;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = descriptorType;
    descriptor_write.pImageInfo = &image_infos[index];
    descriptor_write.pBufferInfo = nullptr;
    descriptor_write.pTexelBufferView = nullptr;

    descriptor_writes.emplace_back(descriptor_write);
}

void OneOffDescriptorSet::UpdateDescriptorSets() {
    vkUpdateDescriptorSets(device_->handle(), descriptor_writes.size(), descriptor_writes.data(), 0, NULL);
}

CreatePipelineHelper::CreatePipelineHelper(VkLayerTest &test) : layer_test_(test) {}

CreatePipelineHelper::~CreatePipelineHelper() {
    VkDevice device = layer_test_.device();
    vkDestroyPipelineCache(device, pipeline_cache_, nullptr);
    vkDestroyPipeline(device, pipeline_, nullptr);
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

    err = vkCreatePipelineCache(layer_test_.device(), &pc_ci_, NULL, &pipeline_cache_);
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
        vkDestroyPipeline(layer_test_.device(), pipeline_, nullptr);
        pipeline_ = VK_NULL_HANDLE;
    }
    err = vkCreateGraphicsPipelines(layer_test_.device(), pipeline_cache_, 1, &gp_ci_, NULL, &pipeline_);
    return err;
}

CreateComputePipelineHelper::CreateComputePipelineHelper(VkLayerTest &test) : layer_test_(test) {}

CreateComputePipelineHelper::~CreateComputePipelineHelper() {
    VkDevice device = layer_test_.device();
    vkDestroyPipelineCache(device, pipeline_cache_, nullptr);
    vkDestroyPipeline(device, pipeline_, nullptr);
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

    err = vkCreatePipelineCache(layer_test_.device(), &pc_ci_, NULL, &pipeline_cache_);
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
        vkDestroyPipeline(layer_test_.device(), pipeline_, nullptr);
        pipeline_ = VK_NULL_HANDLE;
    }
    err = vkCreateComputePipelines(layer_test_.device(), pipeline_cache_, 1, &cp_ci_, NULL, &pipeline_);
    return err;
}

CreateNVRayTracingPipelineHelper::CreateNVRayTracingPipelineHelper(VkLayerTest &test) : layer_test_(test) {}
CreateNVRayTracingPipelineHelper::~CreateNVRayTracingPipelineHelper() {
    VkDevice device = layer_test_.device();
    vkDestroyPipelineCache(device, pipeline_cache_, nullptr);
    vkDestroyPipeline(device, pipeline_, nullptr);
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

    rp_ci_.stageCount = shader_stages_.size();
    rp_ci_.pStages = shader_stages_.data();
    rp_ci_.groupCount = groups_.size();
    rp_ci_.pGroups = groups_.data();
}

void CreateNVRayTracingPipelineHelper::InitPipelineCacheInfo() {
    pc_ci_.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    pc_ci_.pNext = nullptr;
    pc_ci_.flags = 0;
    pc_ci_.initialDataSize = 0;
    pc_ci_.pInitialData = nullptr;
}

void CreateNVRayTracingPipelineHelper::InitInfo() {
    InitShaderGroups();
    InitDescriptorSetInfo();
    InitPipelineLayoutInfo();
    InitShaderInfo();
    InitNVRayTracingPipelineInfo();
    InitPipelineCacheInfo();
}

void CreateNVRayTracingPipelineHelper::InitState() {
    VkResult err;
    descriptor_set_.reset(new OneOffDescriptorSet(layer_test_.DeviceObj(), dsl_bindings_));
    ASSERT_TRUE(descriptor_set_->Initialized());

    pipeline_layout_ = VkPipelineLayoutObj(layer_test_.DeviceObj(), {&descriptor_set_->layout_});

    err = vkCreatePipelineCache(layer_test_.device(), &pc_ci_, NULL, &pipeline_cache_);
    ASSERT_VK_SUCCESS(err);
}

void CreateNVRayTracingPipelineHelper::LateBindPipelineInfo() {
    // By value or dynamically located items must be late bound
    rp_ci_.layout = pipeline_layout_.handle();
    rp_ci_.stageCount = shader_stages_.size();
    rp_ci_.pStages = shader_stages_.data();
}

VkResult CreateNVRayTracingPipelineHelper::CreateNVRayTracingPipeline(bool implicit_destroy, bool do_late_bind) {
    VkResult err;
    if (do_late_bind) {
        LateBindPipelineInfo();
    }
    if (implicit_destroy && (pipeline_ != VK_NULL_HANDLE)) {
        vkDestroyPipeline(layer_test_.device(), pipeline_, nullptr);
        pipeline_ = VK_NULL_HANDLE;
    }

    PFN_vkCreateRayTracingPipelinesNV vkCreateRayTracingPipelinesNV =
        (PFN_vkCreateRayTracingPipelinesNV)vkGetInstanceProcAddr(layer_test_.instance(), "vkCreateRayTracingPipelinesNV");
    err = vkCreateRayTracingPipelinesNV(layer_test_.device(), pipeline_cache_, 1, &rp_ci_, nullptr, &pipeline_);
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
        vkResetCommandPool(layer_test->device(), qf.second.command_pool->handle(), 0);
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
    auto monitor = context_->layer_test->Monitor();
    if (img_err.length()) monitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT, img_err);
    if (buf_err.length()) monitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT, buf_err);

    image_barrier_.srcQueueFamilyIndex = src;
    image_barrier_.dstQueueFamilyIndex = dst;
    buffer_barrier_.srcQueueFamilyIndex = src;
    buffer_barrier_.dstQueueFamilyIndex = dst;

    QueueFamilyObjs *qf = GetQueueFamilyInfo(context_, queue_family_index);

    VkCommandBufferObj *command_buffer = qf->command_buffer;
    for (int cb_repeat = 0; cb_repeat < (mod == Modifier::DOUBLE_COMMAND_BUFFER ? 2 : 1); cb_repeat++) {
        command_buffer->begin();
        for (int repeat = 0; repeat < (mod == Modifier::DOUBLE_RECORD ? 2 : 1); repeat++) {
            vkCmdPipelineBarrier(command_buffer->handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
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
        monitor->VerifyNotFound();
    } else {
        monitor->VerifyFound();
    }
    context_->Reset();
};

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
    int vulkanSupport = InitVulkan();
    if (vulkanSupport == 0) {
        __android_log_print(ANDROID_LOG_INFO, appTag, "==== FAILED ==== No Vulkan support found");
        return;
    }

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

#ifdef ANDROID
    int vulkanSupport = InitVulkan();
    if (vulkanSupport == 0) return 1;
#endif

#if defined(_WIN32) && !defined(NDEBUG)
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
#endif

    ::testing::InitGoogleTest(&argc, argv);
    VkTestFramework::InitArgs(&argc, argv);

    ::testing::AddGlobalTestEnvironment(new TestEnvironment);

    result = RUN_ALL_TESTS();

    VkTestFramework::Finish();
    return result;
}
