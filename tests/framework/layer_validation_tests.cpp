/*
 * Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (c) 2015-2023 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */
#include "utils/cast_utils.h"
#include "generated/enum_flag_bits.h"
#include "layer_validation_tests.h"
#include "utils/vk_layer_utils.h"

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
#include "wayland-client.h"
#endif

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
    assert(false);  // Vulkan drivers are guaranteed to have at least one supported format
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
    assert(false);  // Vulkan drivers are guaranteed to have at least one supported format
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
        VkPhysicalDeviceImageFormatInfo2KHR fmt_info = LvlInitStruct<VkPhysicalDeviceImageFormatInfo2KHR>();
        fmt_info.format = info.format;
        fmt_info.type = info.imageType;
        fmt_info.tiling = info.tiling;
        fmt_info.usage = info.usage;
        fmt_info.flags = info.flags;

        VkImageFormatProperties2KHR fmt_props = LvlInitStruct<VkImageFormatProperties2KHR>();
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
void AddToCommandBuffer(ThreadTestData *data) {
    for (int i = 0; i < 80000; i++) {
        vk::CmdSetEvent(data->commandBuffer, data->event, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
        if (*data->bailout) {
            break;
        }
    }
}

void UpdateDescriptor(ThreadTestData *data) {
    VkDescriptorBufferInfo buffer_info = {};
    buffer_info.buffer = data->buffer;
    buffer_info.offset = 0;
    buffer_info.range = 1;

    VkWriteDescriptorSet descriptor_write = LvlInitStruct<VkWriteDescriptorSet>();
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
}

#endif  // GTEST_IS_THREADSAFE

bool ThreadTimeoutHelper::WaitForThreads(int timeout_in_seconds) {
    std::unique_lock lock(mutex_);
    return cv_.wait_for(lock, std::chrono::seconds{timeout_in_seconds}, [this] {
        std::lock_guard lock_guard(active_thread_mutex_);
        return active_threads_ == 0;
    });
}

void ThreadTimeoutHelper::OnThreadDone() {
    bool last_worker = false;
    {
        std::lock_guard lock(active_thread_mutex_);
        active_threads_--;
        assert(active_threads_ >= 0);
        if (!active_threads_) {
            last_worker = true;
        }
    }
    if (last_worker) {
        cv_.notify_one();
    }
}

void ReleaseNullFence(ThreadTestData *data) {
    for (int i = 0; i < 40000; i++) {
        vk::DestroyFence(data->device, VK_NULL_HANDLE, NULL);
        if (*data->bailout) {
            break;
        }
    }
}

void TestRenderPassCreate(ErrorMonitor *error_monitor, const vk_testing::Device &device, const VkRenderPassCreateInfo &create_info,
                          bool rp2_supported, const char *rp1_vuid, const char *rp2_vuid) {
    if (rp1_vuid) {
        // If the second VUID is not provided, set it equal to the first VUID.  In this way,
        // we can check both vkCreateRenderPass and vkCreateRenderPass2 with the same VUID
        // if rp2_supported is true;
        if (rp2_supported && !rp2_vuid) {
            rp2_vuid = rp1_vuid;
        }

        error_monitor->SetDesiredFailureMsg(kErrorBit, rp1_vuid);
        vk_testing::RenderPass rp(device, create_info);
        error_monitor->VerifyFound();
    }

    if (rp2_supported && rp2_vuid) {
        safe_VkRenderPassCreateInfo2 create_info2 = ConvertVkRenderPassCreateInfoToV2KHR(create_info);

        const auto vkCreateRenderPass2KHR =
            reinterpret_cast<PFN_vkCreateRenderPass2KHR>(vk::GetDeviceProcAddr(device, "vkCreateRenderPass2KHR"));
        // For API version < 1.2 where the extension was not enabled
        if (vkCreateRenderPass2KHR) {
            error_monitor->SetDesiredFailureMsg(kErrorBit, rp2_vuid);
            vk_testing::RenderPass rp2_khr(device, *create_info2.ptr(), true);
            error_monitor->VerifyFound();
        }

        const auto vkCreateRenderPass2 =
            reinterpret_cast<PFN_vkCreateRenderPass2>(vk::GetDeviceProcAddr(device, "vkCreateRenderPass2"));
        // For API version >= 1.2, try core entrypoint
        if (vkCreateRenderPass2) {
            error_monitor->SetDesiredFailureMsg(kErrorBit, rp2_vuid);
            vk_testing::RenderPass rp2_core(device, *create_info2.ptr(), false);
            error_monitor->VerifyFound();
        }
    }
}

void PositiveTestRenderPassCreate(ErrorMonitor *error_monitor, const vk_testing::Device &device,
                                  const VkRenderPassCreateInfo &create_info, bool rp2_supported) {
    vk_testing::RenderPass rp(device, create_info);
    if (rp2_supported) {
        vk_testing::RenderPass rp2(device, *ConvertVkRenderPassCreateInfoToV2KHR(create_info).ptr(), true);
    }
}

void PositiveTestRenderPass2KHRCreate(const vk_testing::Device &device, const VkRenderPassCreateInfo2KHR &create_info) {
    vk_testing::RenderPass rp(device, create_info, true);
}

void TestRenderPass2KHRCreate(ErrorMonitor &error_monitor, const vk_testing::Device &device,
                              const VkRenderPassCreateInfo2KHR &create_info, const std::initializer_list<const char *> &vuids) {
    for (auto vuid : vuids) {
        error_monitor.SetDesiredFailureMsg(kErrorBit, vuid);
    }
    vk_testing::RenderPass rp(device, create_info, true);
    error_monitor.VerifyFound();
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
        VkSubpassBeginInfoKHR subpass_begin_info = {VK_STRUCTURE_TYPE_SUBPASS_BEGIN_INFO_KHR, nullptr, VK_SUBPASS_CONTENTS_INLINE};
        vk::BeginCommandBuffer(command_buffer, &cmd_begin_info);
        error_monitor->SetDesiredFailureMsg(kErrorBit, rp2_vuid);
        vk::CmdBeginRenderPass2KHR(command_buffer, begin_info, &subpass_begin_info);
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
    cb->begin();
    uint32_t num_buf_barrier = (buf_barrier) ? 1 : 0;
    uint32_t num_img_barrier = (img_barrier) ? 1 : 0;
    cb->PipelineBarrier(src_stages, dst_stages, 0, 0, nullptr, num_buf_barrier, buf_barrier, num_img_barrier, img_barrier);
    cb->end();
    cb->QueueCommandBuffer();  // Implicitly waits
}

void ValidOwnershipTransfer(ErrorMonitor *monitor, VkCommandBufferObj *cb_from, VkCommandBufferObj *cb_to,
                            VkPipelineStageFlags src_stages, VkPipelineStageFlags dst_stages,
                            const VkBufferMemoryBarrier *buf_barrier, const VkImageMemoryBarrier *img_barrier) {
    ValidOwnershipTransferOp(monitor, cb_from, src_stages, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, buf_barrier, img_barrier);
    ValidOwnershipTransferOp(monitor, cb_to, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, dst_stages, buf_barrier, img_barrier);
}

void ValidOwnershipTransferOp(ErrorMonitor *monitor, VkCommandBufferObj *cb, const VkBufferMemoryBarrier2KHR *buf_barrier,
                              const VkImageMemoryBarrier2KHR *img_barrier) {
    cb->begin();
    auto dep_info = LvlInitStruct<VkDependencyInfoKHR>();
    dep_info.bufferMemoryBarrierCount = (buf_barrier) ? 1 : 0;
    dep_info.pBufferMemoryBarriers = buf_barrier;
    dep_info.imageMemoryBarrierCount = (img_barrier) ? 1 : 0;
    dep_info.pImageMemoryBarriers = img_barrier;
    cb->PipelineBarrier2KHR(&dep_info);
    cb->end();
    cb->QueueCommandBuffer();  // Implicitly waits
}

void ValidOwnershipTransfer(ErrorMonitor *monitor, VkCommandBufferObj *cb_from, VkCommandBufferObj *cb_to,
                            const VkBufferMemoryBarrier2KHR *buf_barrier, const VkImageMemoryBarrier2KHR *img_barrier) {
    VkBufferMemoryBarrier2KHR fixup_buf_barrier;
    VkImageMemoryBarrier2KHR fixup_img_barrier;
    if (buf_barrier) {
        fixup_buf_barrier = *buf_barrier;
        fixup_buf_barrier.dstStageMask = VK_PIPELINE_STAGE_2_NONE_KHR;
        fixup_buf_barrier.dstAccessMask = 0;
    }
    if (img_barrier) {
        fixup_img_barrier = *img_barrier;
        fixup_img_barrier.dstStageMask = VK_PIPELINE_STAGE_2_NONE_KHR;
        fixup_img_barrier.dstAccessMask = 0;
    }

    ValidOwnershipTransferOp(monitor, cb_from, buf_barrier ? &fixup_buf_barrier : nullptr,
                             img_barrier ? &fixup_img_barrier : nullptr);

    if (buf_barrier) {
        fixup_buf_barrier = *buf_barrier;
        fixup_buf_barrier.srcStageMask = VK_PIPELINE_STAGE_2_NONE_KHR;
        fixup_buf_barrier.srcAccessMask = 0;
    }
    if (img_barrier) {
        fixup_img_barrier = *img_barrier;
        fixup_img_barrier.srcStageMask = VK_PIPELINE_STAGE_2_NONE_KHR;
        fixup_img_barrier.srcAccessMask = 0;
    }
    ValidOwnershipTransferOp(monitor, cb_to, buf_barrier ? &fixup_buf_barrier : nullptr,
                             img_barrier ? &fixup_img_barrier : nullptr);
}

VkResult GPDIFPHelper(VkPhysicalDevice dev, const VkImageCreateInfo *ci, VkImageFormatProperties *limits) {
    VkImageFormatProperties tmp_limits;
    limits = limits ? limits : &tmp_limits;
    return vk::GetPhysicalDeviceImageFormatProperties(dev, ci->format, ci->imageType, ci->tiling, ci->usage, ci->flags, limits);
}

VkFormat FindFormatWithoutFeatures(VkPhysicalDevice gpu, VkImageTiling tiling, VkFormatFeatureFlags undesired_features) {
    const VkFormat first_vk_format = static_cast<VkFormat>(1);
    const VkFormat last_vk_format = static_cast<VkFormat>(130);  // avoid compressed/feature protected, otherwise 184
    VkFormat return_format = VK_FORMAT_UNDEFINED;
    for (VkFormat format = first_vk_format; format <= last_vk_format; format = static_cast<VkFormat>(format + 1)) {
        VkFormatProperties format_props;
        vk::GetPhysicalDeviceFormatProperties(gpu, format, &format_props);

        const auto features =
            (tiling == VK_IMAGE_TILING_LINEAR) ? format_props.linearTilingFeatures : format_props.optimalTilingFeatures;
        if ((features & undesired_features) == 0) {
            return_format = format;
            break;
        }
    }

    return return_format;
}

VkFormat FindFormatWithoutFeatures2(VkPhysicalDevice gpu, VkImageTiling tiling, VkFormatFeatureFlags2 undesired_features) {
    const VkFormat first_compressed_format = VK_FORMAT_BC1_RGB_UNORM_BLOCK;  // avoid compressed/feature protected, otherwise 184
    const VkFormat first_vk_format = VK_FORMAT_R4G4_UNORM_PACK8;
    VkFormat return_format = VK_FORMAT_UNDEFINED;
    for (VkFormat format = first_vk_format; format < first_compressed_format; format = static_cast<VkFormat>(format + 1)) {
        auto fmt_props_3 = LvlInitStruct<VkFormatProperties3KHR>();
        auto fmt_props_2 = LvlInitStruct<VkFormatProperties2>(&fmt_props_3);
        vk::GetPhysicalDeviceFormatProperties2(gpu, format, &fmt_props_2);
        auto features = (tiling == VK_IMAGE_TILING_LINEAR) ? fmt_props_3.linearTilingFeatures : fmt_props_3.optimalTilingFeatures;
        if ((features & undesired_features) == 0) {
            return_format = format;
            break;
        }
    }

    return return_format;
}

bool SemaphoreExportImportSupported(VkPhysicalDevice gpu, VkExternalSemaphoreHandleTypeFlagBits handle_type) {
    constexpr auto export_import_flags =
        VK_EXTERNAL_SEMAPHORE_FEATURE_EXPORTABLE_BIT_KHR | VK_EXTERNAL_SEMAPHORE_FEATURE_IMPORTABLE_BIT_KHR;

    auto info = LvlInitStruct<VkPhysicalDeviceExternalSemaphoreInfo>();
    info.handleType = handle_type;
    auto properties = LvlInitStruct<VkExternalSemaphoreProperties>();
    vk::GetPhysicalDeviceExternalSemaphoreProperties(gpu, &info, &properties);
    return (properties.externalSemaphoreFeatures & export_import_flags) == export_import_flags;
}

void AllocateDisjointMemory(VkDeviceObj *device, PFN_vkGetImageMemoryRequirements2KHR fp, VkImage mp_image,
                            VkDeviceMemory *mp_image_mem, VkImageAspectFlagBits plane) {
    VkImagePlaneMemoryRequirementsInfo image_plane_req = LvlInitStruct<VkImagePlaneMemoryRequirementsInfo>();
    image_plane_req.planeAspect = plane;

    VkImageMemoryRequirementsInfo2 mem_req_info2 = LvlInitStruct<VkImageMemoryRequirementsInfo2>(&image_plane_req);
    mem_req_info2.image = mp_image;

    VkMemoryRequirements2 mp_image_mem_reqs2 = LvlInitStruct<VkMemoryRequirements2>();

    fp(device->device(), &mem_req_info2, &mp_image_mem_reqs2);

    VkMemoryAllocateInfo mp_image_alloc_info = LvlInitStruct<VkMemoryAllocateInfo>();
    mp_image_alloc_info.allocationSize = mp_image_mem_reqs2.memoryRequirements.size;
    ASSERT_TRUE(device->phy().set_memory_type(mp_image_mem_reqs2.memoryRequirements.memoryTypeBits, &mp_image_alloc_info, 0));
    ASSERT_VK_SUCCESS(vk::AllocateMemory(device->device(), &mp_image_alloc_info, NULL, mp_image_mem));
}

void CreateSamplerTest(VkLayerTest &test, const VkSamplerCreateInfo *create_info, const std::string &code) {
    if (code.length()) {
        test.Monitor().SetDesiredFailureMsg(kErrorBit | kWarningBit, code);
    }

    vk_testing::Sampler sampler(*test.DeviceObj(), *create_info);

    if (code.length()) {
        test.Monitor().VerifyFound();
    }
}

void CreateBufferTest(VkLayerTest &test, const VkBufferCreateInfo *create_info, const std::string &code) {
    if (code.length()) {
        test.Monitor().SetDesiredFailureMsg(kErrorBit, code);
    }
    vk_testing::Buffer buffer(*test.DeviceObj(), *create_info, vk_testing::no_mem);
    if (code.length()) {
        test.Monitor().VerifyFound();
    }
}

void CreateImageTest(VkLayerTest &test, const VkImageCreateInfo *create_info, const std::string &code) {
    if (code.length()) {
        test.Monitor().SetDesiredFailureMsg(kErrorBit, code);
    }
    vk_testing::Image image(*test.DeviceObj(), *create_info, vk_testing::no_mem);
    if (code.length()) {
        test.Monitor().VerifyFound();
    }
}

void CreateBufferViewTest(VkLayerTest &test, const VkBufferViewCreateInfo *create_info, const std::vector<std::string> &codes) {
    if (codes.size()) {
        std::for_each(codes.begin(), codes.end(), [&](const std::string &s) { test.Monitor().SetDesiredFailureMsg(kErrorBit, s); });
    }
    vk_testing::BufferView view(*test.DeviceObj(), *create_info);
    if (codes.size()) {
        test.Monitor().VerifyFound();
    }
}

void CreateImageViewTest(VkLayerTest &test, const VkImageViewCreateInfo *create_info, const std::string &code) {
    if (code.length()) {
        test.Monitor().SetDesiredFailureMsg(kErrorBit, code);
    }
    vk_testing::ImageView view(*test.DeviceObj(), *create_info);
    if (code.length()) {
        test.Monitor().VerifyFound();
    }
}

VkSamplerCreateInfo SafeSaneSamplerCreateInfo() {
    VkSamplerCreateInfo sampler_create_info = LvlInitStruct<VkSamplerCreateInfo>();
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

void VkLayerTest::VKTriangleTest(BsoFailSelect failCase) {
    ASSERT_TRUE(m_device && m_device->initialized());  // VKTriangleTest assumes Init() has finished

    ASSERT_NO_FATAL_FAILURE(InitViewport());

    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj ps(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineObj pipelineobj(m_device);
    pipelineobj.AddDefaultColorAttachment();
    pipelineobj.AddShader(&vs);
    pipelineobj.AddShader(&ps);

    bool failcase_needs_depth = false;  // to mark cases that need depth attachment

    VkBufferObj index_buffer;

    switch (failCase) {
        case BsoFailLineWidth: {
            pipelineobj.MakeDynamic(VK_DYNAMIC_STATE_LINE_WIDTH);
            VkPipelineInputAssemblyStateCreateInfo ia_state = LvlInitStruct<VkPipelineInputAssemblyStateCreateInfo>();
            ia_state.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
            pipelineobj.SetInputAssembly(&ia_state);
            break;
        }
        case BsoFailLineStipple: {
            pipelineobj.MakeDynamic(VK_DYNAMIC_STATE_LINE_STIPPLE_EXT);
            VkPipelineInputAssemblyStateCreateInfo ia_state = LvlInitStruct<VkPipelineInputAssemblyStateCreateInfo>();
            ia_state.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
            pipelineobj.SetInputAssembly(&ia_state);

            VkPipelineRasterizationLineStateCreateInfoEXT line_state =
                LvlInitStruct<VkPipelineRasterizationLineStateCreateInfoEXT>();
            line_state.lineRasterizationMode = VK_LINE_RASTERIZATION_MODE_BRESENHAM_EXT;
            line_state.stippledLineEnable = VK_TRUE;
            line_state.lineStippleFactor = 1;
            line_state.lineStipplePattern = 0;
            pipelineobj.SetLineState(&line_state);
            break;
        }
        case BsoFailDepthBias: {
            pipelineobj.MakeDynamic(VK_DYNAMIC_STATE_DEPTH_BIAS);
            VkPipelineRasterizationStateCreateInfo rs_state = LvlInitStruct<VkPipelineRasterizationStateCreateInfo>();
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
            VkBufferCreateInfo buffer_info = LvlInitStruct<VkBufferCreateInfo>();
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

        m_depthStencil->Init(m_device, m_width, m_height, m_depth_stencil_fmt,
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
        VkClearRect clear_rect = {{{0, 0}, {m_width, m_height}}, 0, 1};

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

    VkPipelineDepthStencilStateCreateInfo ds_ci = LvlInitStruct<VkPipelineDepthStencilStateCreateInfo>();
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
#if !defined(VK_USE_PLATFORM_ANDROID_KHR)
    m_instance_extension_names.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#else
    m_instance_extension_names.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#endif

    instance_layers_.push_back(kValidationLayerName);

    if (InstanceLayerSupported("VK_LAYER_LUNARG_device_profile_api")) {
        instance_layers_.push_back("VK_LAYER_LUNARG_device_profile_api");
    }

    if (InstanceLayerSupported(kSynchronization2LayerName)) {
        instance_layers_.push_back(kSynchronization2LayerName);
    }

    app_info_ = LvlInitStruct<VkApplicationInfo>();
    app_info_.pApplicationName = "layer_tests";
    app_info_.applicationVersion = 1;
    app_info_.pEngineName = "unittest";
    app_info_.engineVersion = 1;
    app_info_.apiVersion = VK_API_VERSION_1_0;

    // Find out what version the instance supports and record the default target instance
    auto enumerateInstanceVersion = (PFN_vkEnumerateInstanceVersion)vk::GetInstanceProcAddr(nullptr, "vkEnumerateInstanceVersion");
    if (enumerateInstanceVersion) {
        uint32_t instance_api_version;
        enumerateInstanceVersion(&instance_api_version);
        m_instance_api_version = instance_api_version;
    } else {
        m_instance_api_version = VK_API_VERSION_1_0;
    }
    m_target_api_version = app_info_.apiVersion;
}

void VkLayerTest::AddSurfaceExtension() {
    AddRequiredExtensions(VK_KHR_SURFACE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

#if defined(VK_USE_PLATFORM_WIN32_KHR)
    AddWsiExtensions(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    AddWsiExtensions(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#endif

#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
    AddWsiExtensions(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#endif
#if defined(VK_USE_PLATFORM_XLIB_KHR)
    AddWsiExtensions(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#endif
#if defined(VK_USE_PLATFORM_XCB_KHR)
    AddWsiExtensions(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#endif
}

void VkLayerTest::SetTargetApiVersion(APIVersion target_api_version) {
    if (target_api_version == 0) target_api_version = VK_API_VERSION_1_0;
    // If we set target twice, make sure higest version always wins
    if (target_api_version < m_attempted_api_version) return;

    m_attempted_api_version = target_api_version;  // used to know if request failed

    m_target_api_version = std::min(target_api_version, m_instance_api_version);
    app_info_.apiVersion = m_target_api_version.value();
}

APIVersion VkLayerTest::DeviceValidationVersion() const {
    // The validation layers assume the version we are validating to is the apiVersion unless the device apiVersion is lower
    return std::min(m_target_api_version, APIVersion(physDevProps().apiVersion));
}

template <>
VkPhysicalDeviceFeatures2 VkLayerTest::GetPhysicalDeviceFeatures2(VkPhysicalDeviceFeatures2 &features2) {
    if (DeviceValidationVersion() >= VK_API_VERSION_1_1) {
        vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    } else {
        auto vkGetPhysicalDeviceFeatures2KHR = reinterpret_cast<PFN_vkGetPhysicalDeviceFeatures2KHR>(
            vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR"));
        assert(vkGetPhysicalDeviceFeatures2KHR);
        vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);
    }
    return features2;
}

template <>
VkPhysicalDeviceProperties2 VkLayerTest::GetPhysicalDeviceProperties2(VkPhysicalDeviceProperties2 &props2) {
    if (DeviceValidationVersion() >= VK_API_VERSION_1_1) {
        vk::GetPhysicalDeviceProperties2(gpu(), &props2);
    } else {
        auto vkGetPhysicalDeviceProperties2KHR = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties2KHR>(
            vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceProperties2KHR"));
        assert(vkGetPhysicalDeviceProperties2KHR);
        vkGetPhysicalDeviceProperties2KHR(gpu(), &props2);
    }
    return props2;
}

bool VkLayerTest::IsDriver(VkDriverId driver_id) {
    if (VkRenderFramework::IgnoreDisableChecks()) {
        return false;
    } else {
        auto driver_properties = LvlInitStruct<VkPhysicalDeviceDriverProperties>();
        auto physical_device_properties2 = LvlInitStruct<VkPhysicalDeviceProperties2>(&driver_properties);
        GetPhysicalDeviceProperties2(physical_device_properties2);
        return (driver_properties.driverID == driver_id);
    }
}

bool VkLayerTest::LoadDeviceProfileLayer(
    PFN_vkSetPhysicalDeviceFormatPropertiesEXT &fpvkSetPhysicalDeviceFormatPropertiesEXT,
    PFN_vkGetOriginalPhysicalDeviceFormatPropertiesEXT &fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT) {
    if (IsPlatform(kMockICD)) {
        printf("Device Profile layer is for real GPU, if using MockICD with profiles, just adjust the profile json file instead\n");
        return false;
    }

    // Load required functions
    fpvkSetPhysicalDeviceFormatPropertiesEXT =
        (PFN_vkSetPhysicalDeviceFormatPropertiesEXT)vk::GetInstanceProcAddr(instance(), "vkSetPhysicalDeviceFormatPropertiesEXT");
    fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT = (PFN_vkGetOriginalPhysicalDeviceFormatPropertiesEXT)vk::GetInstanceProcAddr(
        instance(), "vkGetOriginalPhysicalDeviceFormatPropertiesEXT");

    if (!(fpvkSetPhysicalDeviceFormatPropertiesEXT) || !(fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT)) {
        printf(
            "Can't find device_profile_api functions; make sure VK_LAYER_PATH is set correctly to where the validation layers "
            "are built, the device profile layer should be in the same directory.\n");
        return false;
    }

    return true;
}

bool VkLayerTest::LoadDeviceProfileLayer(
    PFN_vkSetPhysicalDeviceFormatProperties2EXT &fpvkSetPhysicalDeviceFormatProperties2EXT,
    PFN_vkGetOriginalPhysicalDeviceFormatProperties2EXT &fpvkGetOriginalPhysicalDeviceFormatProperties2EXT) {
    if (IsPlatform(kMockICD)) {
        printf("Device Profile layer is for real GPU, if using MockICD with profiles, just adjust the profile json file instead\n");
        return false;
    }

    // Load required functions
    fpvkSetPhysicalDeviceFormatProperties2EXT =
        (PFN_vkSetPhysicalDeviceFormatProperties2EXT)vk::GetInstanceProcAddr(instance(), "vkSetPhysicalDeviceFormatProperties2EXT");
    fpvkGetOriginalPhysicalDeviceFormatProperties2EXT =
        (PFN_vkGetOriginalPhysicalDeviceFormatProperties2EXT)vk::GetInstanceProcAddr(
            instance(), "vkGetOriginalPhysicalDeviceFormatProperties2EXT");

    if (!(fpvkSetPhysicalDeviceFormatProperties2EXT) || !(fpvkGetOriginalPhysicalDeviceFormatProperties2EXT)) {
        printf(
            "Can't find device_profile_api functions; make sure VK_LAYER_PATH is set correctly to where the validation layers "
            "are built, the device profile layer should be in the same directory.\n");
        return false;
    }

    return true;
}

bool VkLayerTest::LoadDeviceProfileLayer(PFN_vkSetPhysicalDeviceLimitsEXT &fpvkSetPhysicalDeviceLimitsEXT,
                                         PFN_vkGetOriginalPhysicalDeviceLimitsEXT &fpvkGetOriginalPhysicalDeviceLimitsEXT) {
    if (IsPlatform(kMockICD)) {
        printf("Device Profile layer is for real GPU, if using MockICD with profiles, just adjust the profile json file instead\n");
        return false;
    }

    // Load required functions
    fpvkSetPhysicalDeviceLimitsEXT =
        (PFN_vkSetPhysicalDeviceLimitsEXT)vk::GetInstanceProcAddr(instance(), "vkSetPhysicalDeviceLimitsEXT");
    fpvkGetOriginalPhysicalDeviceLimitsEXT =
        (PFN_vkGetOriginalPhysicalDeviceLimitsEXT)vk::GetInstanceProcAddr(instance(), "vkGetOriginalPhysicalDeviceLimitsEXT");

    if (!(fpvkSetPhysicalDeviceLimitsEXT) || !(fpvkGetOriginalPhysicalDeviceLimitsEXT)) {
        printf(
            "Can't find device_profile_api functions; make sure VK_LAYER_PATH is set correctly to where the validation layers "
            "are built, the device profile layer should be in the same directory.\n");
        return false;
    }

    return true;
}

bool VkLayerTest::LoadDeviceProfileLayer(PFN_vkSetPhysicalDeviceFeaturesEXT &fpvkSetPhysicalDeviceFeaturesEXT,
                                         PFN_vkGetOriginalPhysicalDeviceFeaturesEXT &fpvkGetOriginalPhysicalDeviceFeaturesEXT) {
    if (IsPlatform(kMockICD)) {
        printf("Device Profile layer is for real GPU, if using MockICD with profiles, just adjust the profile json file instead\n");
        return false;
    }

    // Load required functions
    fpvkSetPhysicalDeviceFeaturesEXT =
        (PFN_vkSetPhysicalDeviceFeaturesEXT)vk::GetInstanceProcAddr(instance(), "vkSetPhysicalDeviceFeaturesEXT");
    fpvkGetOriginalPhysicalDeviceFeaturesEXT =
        (PFN_vkGetOriginalPhysicalDeviceFeaturesEXT)vk::GetInstanceProcAddr(instance(), "vkGetOriginalPhysicalDeviceFeaturesEXT");

    if (!(fpvkSetPhysicalDeviceFeaturesEXT) || !(fpvkGetOriginalPhysicalDeviceFeaturesEXT)) {
        printf(
            "Can't find device_profile_api functions; make sure VK_LAYER_PATH is set correctly to where the validation layers "
            "are built, the device profile layer should be in the same directory.\n");
        return false;
    }

    return true;
}

bool VkLayerTest::LoadDeviceProfileLayer(PFN_VkSetPhysicalDeviceProperties2EXT &fpvkSetPhysicalDeviceProperties2EXT) {
    if (IsPlatform(kMockICD)) {
        printf("Device Profile layer is for real GPU, if using MockICD with profiles, just adjust the profile json file instead\n");
        return false;
    }

    // Load required functions
    fpvkSetPhysicalDeviceProperties2EXT =
        (PFN_VkSetPhysicalDeviceProperties2EXT)vk::GetInstanceProcAddr(instance(), "vkSetPhysicalDeviceProperties2EXT");

    if (!fpvkSetPhysicalDeviceProperties2EXT) {
        printf(
            "Can't find device_profile_api functions; make sure VK_LAYER_PATH is set correctly to where the validation layers "
            "are built, the device profile layer should be in the same directory.\n");
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
        VkBufferCreateInfo buffer_create_info = LvlInitStruct<VkBufferCreateInfo>();
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
        VkMemoryAllocateInfo memory_allocate_info = LvlInitStruct<VkMemoryAllocateInfo>();
        memory_allocate_info.allocationSize = 1;   // fake size -- shouldn't matter for the test
        memory_allocate_info.memoryTypeIndex = 0;  // fake type -- shouldn't matter for the test
        vk::AllocateMemory(VulkanDevice, &memory_allocate_info, nullptr, &VulkanMemory);

        VulkanBuffer = (aTestFlag == eBindNullBuffer) ? VK_NULL_HANDLE : (VkBuffer)0xCDCDCDCDCDCDCDCD;

        vk::BindBufferMemory(VulkanDevice, VulkanBuffer, VulkanMemory, 0);
    } else {
        VkBufferCreateInfo buffer_create_info = LvlInitStruct<VkBufferCreateInfo>();
        buffer_create_info.size = 32;
        buffer_create_info.usage = aBufferUsage;

        vk::CreateBuffer(VulkanDevice, &buffer_create_info, nullptr, &VulkanBuffer);

        CreateCurrent = true;

        VkMemoryRequirements memory_requirements;
        vk::GetBufferMemoryRequirements(VulkanDevice, VulkanBuffer, &memory_requirements);

        VkMemoryAllocateInfo memory_allocate_info = LvlInitStruct<VkMemoryAllocateInfo>();
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

void SetImageLayout(VkDeviceObj *device, VkImageAspectFlags aspect, VkImage image, VkImageLayout image_layout) {
    VkCommandPoolObj pool(device, device->graphics_queue_node_index_);
    VkCommandBufferObj cmd_buf(device, &pool);

    cmd_buf.begin();
    VkImageMemoryBarrier layout_barrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                                        nullptr,
                                        0,
                                        VK_ACCESS_MEMORY_READ_BIT,
                                        VK_IMAGE_LAYOUT_UNDEFINED,
                                        image_layout,
                                        VK_QUEUE_FAMILY_IGNORED,
                                        VK_QUEUE_FAMILY_IGNORED,
                                        image,
                                        {aspect, 0, 1, 0, 1}};

    cmd_buf.PipelineBarrier(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1,
                            &layout_barrier);
    cmd_buf.end();

    cmd_buf.QueueCommandBuffer();
}

std::unique_ptr<VkImageObj> VkArmBestPracticesLayerTest::CreateImage(VkFormat format, const uint32_t width, const uint32_t height,
                                                                     VkImageUsageFlags attachment_usage) {
    auto img = std::unique_ptr<VkImageObj>(new VkImageObj(m_device));
    img->Init(width, height, 1, format,
              VK_IMAGE_USAGE_SAMPLED_BIT | attachment_usage | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
              VK_IMAGE_TILING_OPTIMAL);
    return img;
}

VkRenderPass VkArmBestPracticesLayerTest::CreateRenderPass(VkFormat format, VkAttachmentLoadOp load_op,
                                                           VkAttachmentStoreOp store_op) {
    VkRenderPass renderpass{VK_NULL_HANDLE};

    // Create renderpass
    VkAttachmentDescription attachment = {};
    attachment.format = format;
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp = load_op;
    attachment.storeOp = store_op;
    attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkAttachmentReference attachment_reference = {};
    attachment_reference.attachment = 0;
    attachment_reference.layout = VK_IMAGE_LAYOUT_GENERAL;

    VkSubpassDescription subpass = {};
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &attachment_reference;

    VkRenderPassCreateInfo rpinf = LvlInitStruct<VkRenderPassCreateInfo>();
    rpinf.attachmentCount = 1;
    rpinf.pAttachments = &attachment;
    rpinf.subpassCount = 1;
    rpinf.pSubpasses = &subpass;
    rpinf.dependencyCount = 0;
    rpinf.pDependencies = nullptr;

    VkResult result = vk::CreateRenderPass(m_device->handle(), &rpinf, nullptr, &renderpass);
    assert(result == VK_SUCCESS);
    (void)result;

    return renderpass;
}

VkFramebuffer VkArmBestPracticesLayerTest::CreateFramebuffer(const uint32_t width, const uint32_t height, VkImageView image_view,
                                                             VkRenderPass renderpass) {
    VkFramebuffer framebuffer{VK_NULL_HANDLE};

    VkFramebufferCreateInfo framebuffer_create_info = LvlInitStruct<VkFramebufferCreateInfo>();
    framebuffer_create_info.renderPass = renderpass;
    framebuffer_create_info.attachmentCount = 1;
    framebuffer_create_info.pAttachments = &image_view;
    framebuffer_create_info.width = width;
    framebuffer_create_info.height = height;
    framebuffer_create_info.layers = 1;

    VkResult result = vk::CreateFramebuffer(m_device->handle(), &framebuffer_create_info, nullptr, &framebuffer);
    assert(result == VK_SUCCESS);
    (void)result;

    return framebuffer;
}

VkSampler VkArmBestPracticesLayerTest::CreateDefaultSampler() {
    VkSampler sampler{VK_NULL_HANDLE};

    VkSamplerCreateInfo sampler_create_info = LvlInitStruct<VkSamplerCreateInfo>();
    sampler_create_info.magFilter = VK_FILTER_NEAREST;
    sampler_create_info.minFilter = VK_FILTER_NEAREST;
    sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    sampler_create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_create_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    sampler_create_info.maxLod = VK_LOD_CLAMP_NONE;

    VkResult result = vk::CreateSampler(m_device->handle(), &sampler_create_info, nullptr, &sampler);
    assert(result == VK_SUCCESS);
    (void)result;

    return sampler;
}

bool VkBufferTest::GetBufferCurrent() { return AllocateCurrent && BoundCurrent && CreateCurrent; }

const VkBuffer &VkBufferTest::GetBuffer() { return VulkanBuffer; }

void VkBufferTest::TestDoubleDestroy() {
    // Destroy the buffer but leave the flag set, which will cause
    // the buffer to be destroyed again in the destructor.
    vk::DestroyBuffer(VulkanDevice, VulkanBuffer, nullptr);
}

OneOffDescriptorSet::OneOffDescriptorSet(VkDeviceObj *device, const Bindings &bindings,
                                         VkDescriptorSetLayoutCreateFlags layout_flags, void *layout_pnext,
                                         VkDescriptorPoolCreateFlags poolFlags, void *allocate_pnext, int buffer_info_size,
                                         int image_info_size, int buffer_view_size)
    : device_{device}, pool_{}, layout_(device, bindings, layout_flags, layout_pnext), set_(VK_NULL_HANDLE) {
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

void OneOffDescriptorSet::Clear() {
    buffer_infos.clear();
    image_infos.clear();
    buffer_views.clear();
    descriptor_writes.clear();
}

void OneOffDescriptorSet::WriteDescriptorBufferInfo(int binding, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range,
                                                    VkDescriptorType descriptorType, uint32_t arrayElement, uint32_t count) {
    const auto index = buffer_infos.size();

    VkDescriptorBufferInfo buffer_info = {};
    buffer_info.buffer = buffer;
    buffer_info.offset = offset;
    buffer_info.range = range;

    for (uint32_t i = 0; i < count; ++i) {
        buffer_infos.emplace_back(buffer_info);
    }

    VkWriteDescriptorSet descriptor_write = LvlInitStruct<VkWriteDescriptorSet>();
    descriptor_write.dstSet = set_;
    descriptor_write.dstBinding = binding;
    descriptor_write.dstArrayElement = arrayElement;
    descriptor_write.descriptorCount = count;
    descriptor_write.descriptorType = descriptorType;
    descriptor_write.pBufferInfo = &buffer_infos[index];
    descriptor_write.pImageInfo = nullptr;
    descriptor_write.pTexelBufferView = nullptr;

    descriptor_writes.emplace_back(descriptor_write);
}

void OneOffDescriptorSet::WriteDescriptorBufferView(int binding, VkBufferView buffer_view, VkDescriptorType descriptorType,
                                                    uint32_t arrayElement, uint32_t count) {
    const auto index = buffer_views.size();

    for (uint32_t i = 0; i < count; ++i) {
        buffer_views.emplace_back(buffer_view);
    }

    VkWriteDescriptorSet descriptor_write = LvlInitStruct<VkWriteDescriptorSet>();
    descriptor_write.dstSet = set_;
    descriptor_write.dstBinding = binding;
    descriptor_write.dstArrayElement = arrayElement;
    descriptor_write.descriptorCount = count;
    descriptor_write.descriptorType = descriptorType;
    descriptor_write.pTexelBufferView = &buffer_views[index];
    descriptor_write.pImageInfo = nullptr;
    descriptor_write.pBufferInfo = nullptr;

    descriptor_writes.emplace_back(descriptor_write);
}

void OneOffDescriptorSet::WriteDescriptorImageInfo(int binding, VkImageView image_view, VkSampler sampler,
                                                   VkDescriptorType descriptorType, VkImageLayout imageLayout,
                                                   uint32_t arrayElement, uint32_t count) {
    const auto index = image_infos.size();

    VkDescriptorImageInfo image_info = {};
    image_info.imageView = image_view;
    image_info.sampler = sampler;
    image_info.imageLayout = imageLayout;

    for (uint32_t i = 0; i < count; ++i) {
        image_infos.emplace_back(image_info);
    }

    VkWriteDescriptorSet descriptor_write = LvlInitStruct<VkWriteDescriptorSet>();
    descriptor_write.dstSet = set_;
    descriptor_write.dstBinding = binding;
    descriptor_write.dstArrayElement = arrayElement;
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

CreatePipelineHelper::CreatePipelineHelper(VkLayerTest &test, uint32_t color_attachments_count)
    : cb_attachments_(color_attachments_count), layer_test_(test) {}

CreatePipelineHelper::~CreatePipelineHelper() {
    VkDevice device = layer_test_.device();
    vk::DestroyPipelineCache(device, pipeline_cache_, nullptr);
    vk::DestroyPipeline(device, pipeline_, nullptr);
}

void CreatePipelineHelper::InitDescriptorSetInfo() {
    dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
}

void CreatePipelineHelper::InitInputAndVertexInfo() {
    vi_ci_ = LvlInitStruct<VkPipelineVertexInputStateCreateInfo>();

    ia_ci_ = LvlInitStruct<VkPipelineInputAssemblyStateCreateInfo>();
    ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
}

void CreatePipelineHelper::InitMultisampleInfo() {
    pipe_ms_state_ci_ = LvlInitStruct<VkPipelineMultisampleStateCreateInfo>();
    pipe_ms_state_ci_.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    pipe_ms_state_ci_.sampleShadingEnable = VK_FALSE;
    pipe_ms_state_ci_.minSampleShading = 1.0;
    pipe_ms_state_ci_.pSampleMask = NULL;
}

void CreatePipelineHelper::InitPipelineLayoutInfo() {
    pipeline_layout_ci_ = LvlInitStruct<VkPipelineLayoutCreateInfo>();
    pipeline_layout_ci_.setLayoutCount = 1;     // Not really changeable because InitState() sets exactly one pSetLayout
    pipeline_layout_ci_.pSetLayouts = nullptr;  // must bound after it is created
}

void CreatePipelineHelper::InitViewportInfo() {
    viewport_ = {0.0f, 0.0f, 64.0f, 64.0f, 0.0f, 1.0f};
    scissor_ = {{0, 0}, {64, 64}};

    vp_state_ci_ = LvlInitStruct<VkPipelineViewportStateCreateInfo>();
    vp_state_ci_.viewportCount = 1;
    vp_state_ci_.pViewports = &viewport_;  // ignored if dynamic
    vp_state_ci_.scissorCount = 1;
    vp_state_ci_.pScissors = &scissor_;  // ignored if dynamic
}

void CreatePipelineHelper::InitDynamicStateInfo() {
    // Use a "validity" check on the {} initialized structure to detect initialization
    // during late bind
}

void CreatePipelineHelper::InitShaderInfo() { ResetShaderInfo(kVertexMinimalGlsl, kFragmentMinimalGlsl); }

void CreatePipelineHelper::ResetShaderInfo(const char *vertex_shader_text, const char *fragment_shader_text) {
    vs_ = std::make_unique<VkShaderObj>(&layer_test_, vertex_shader_text, VK_SHADER_STAGE_VERTEX_BIT);
    fs_ = std::make_unique<VkShaderObj>(&layer_test_, fragment_shader_text, VK_SHADER_STAGE_FRAGMENT_BIT);
    // We shouldn't need a fragment shader but add it to be able to run on more devices
    shader_stages_ = {vs_->GetStageCreateInfo(), fs_->GetStageCreateInfo()};
}

void CreatePipelineHelper::InitRasterizationInfo() {
    rs_state_ci_ = LvlInitStruct<VkPipelineRasterizationStateCreateInfo>(&line_state_ci_);
    rs_state_ci_.flags = 0;
    rs_state_ci_.depthClampEnable = VK_FALSE;
    rs_state_ci_.rasterizerDiscardEnable = VK_FALSE;
    rs_state_ci_.polygonMode = VK_POLYGON_MODE_FILL;
    rs_state_ci_.cullMode = VK_CULL_MODE_BACK_BIT;
    rs_state_ci_.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rs_state_ci_.depthBiasEnable = VK_FALSE;
    rs_state_ci_.lineWidth = 1.0F;
}

void CreatePipelineHelper::InitLineRasterizationInfo() {
    line_state_ci_ = LvlInitStruct<VkPipelineRasterizationLineStateCreateInfoEXT>();
    line_state_ci_.lineRasterizationMode = VK_LINE_RASTERIZATION_MODE_DEFAULT_EXT;
    line_state_ci_.stippledLineEnable = VK_FALSE;
    line_state_ci_.lineStippleFactor = 0;
    line_state_ci_.lineStipplePattern = 0;
}

void CreatePipelineHelper::InitBlendStateInfo() {
    cb_ci_ = LvlInitStruct<VkPipelineColorBlendStateCreateInfo>();
    cb_ci_.logicOpEnable = VK_FALSE;
    cb_ci_.logicOp = VK_LOGIC_OP_COPY;  // ignored if enable is VK_FALSE above
    cb_ci_.attachmentCount = cb_attachments_.size();
    ASSERT_TRUE(IsValidVkStruct(layer_test_.RenderPassInfo()));
    cb_ci_.pAttachments = cb_attachments_.data();
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
    gp_ci_ = LvlInitStruct<VkGraphicsPipelineCreateInfo>();
    gp_ci_.flags = VK_PIPELINE_CREATE_DISABLE_OPTIMIZATION_BIT;
    gp_ci_.pVertexInputState = &vi_ci_;
    gp_ci_.pInputAssemblyState = &ia_ci_;
    gp_ci_.pTessellationState = nullptr;
    gp_ci_.pViewportState = &vp_state_ci_;
    gp_ci_.pMultisampleState = &pipe_ms_state_ci_;
    gp_ci_.pRasterizationState = &rs_state_ci_;
    gp_ci_.pDepthStencilState = nullptr;
    gp_ci_.pColorBlendState = &cb_ci_;
    gp_ci_.pDynamicState = nullptr;
    gp_ci_.renderPass = layer_test_.renderPass();
}

void CreatePipelineHelper::InitPipelineCacheInfo() {
    pc_ci_ = LvlInitStruct<VkPipelineCacheCreateInfo>();
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

void CreatePipelineHelper::InitVertexInputLibInfo(void *p_next) {
    InitDescriptorSetInfo();
    InitInputAndVertexInfo();
    InitMultisampleInfo();
    InitPipelineLayoutInfo();
    InitViewportInfo();
    InitDynamicStateInfo();
    InitRasterizationInfo();
    InitLineRasterizationInfo();
    InitBlendStateInfo();

    gpl_info.emplace(LvlInitStruct<VkGraphicsPipelineLibraryCreateInfoEXT>(p_next));
    gpl_info->flags = VK_GRAPHICS_PIPELINE_LIBRARY_VERTEX_INPUT_INTERFACE_BIT_EXT;

    gp_ci_ = LvlInitStruct<VkGraphicsPipelineCreateInfo>(&gpl_info);
    gp_ci_.flags = VK_PIPELINE_CREATE_LIBRARY_BIT_KHR;
    gp_ci_.pVertexInputState = &vi_ci_;
    gp_ci_.pInputAssemblyState = &ia_ci_;

    InitPipelineCacheInfo();
}

void CreatePipelineHelper::InitPreRasterLibInfo(uint32_t count, const VkPipelineShaderStageCreateInfo *info, void *p_next) {
    InitDescriptorSetInfo();
    InitInputAndVertexInfo();
    InitMultisampleInfo();
    InitPipelineLayoutInfo();
    InitViewportInfo();
    InitDynamicStateInfo();
    InitRasterizationInfo();
    InitLineRasterizationInfo();
    InitBlendStateInfo();

    gpl_info.emplace(LvlInitStruct<VkGraphicsPipelineLibraryCreateInfoEXT>(p_next));
    gpl_info->flags = VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT;

    gp_ci_ = LvlInitStruct<VkGraphicsPipelineCreateInfo>(&gpl_info);
    gp_ci_.flags = VK_PIPELINE_CREATE_LIBRARY_BIT_KHR;
    gp_ci_.pViewportState = &vp_state_ci_;
    gp_ci_.pRasterizationState = &rs_state_ci_;

    // If using Dynamic Rendering, will need to be set to null
    // otherwise needs to be shared across libraries in the same executable pipeline
    gp_ci_.renderPass = layer_test_.renderPass();
    gp_ci_.subpass = 0;

    gp_ci_.stageCount = count;
    gp_ci_.pStages = info;

    InitPipelineCacheInfo();
}

void CreatePipelineHelper::InitFragmentLibInfo(uint32_t count, const VkPipelineShaderStageCreateInfo *info, void *p_next) {
    InitDescriptorSetInfo();
    InitInputAndVertexInfo();
    InitMultisampleInfo();
    InitPipelineLayoutInfo();
    InitViewportInfo();
    InitDynamicStateInfo();
    InitRasterizationInfo();
    InitLineRasterizationInfo();
    InitBlendStateInfo();

    gpl_info.emplace(LvlInitStruct<VkGraphicsPipelineLibraryCreateInfoEXT>(p_next));
    gpl_info->flags = VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT;

    gp_ci_ = LvlInitStruct<VkGraphicsPipelineCreateInfo>(&gpl_info);
    gp_ci_.flags = VK_PIPELINE_CREATE_LIBRARY_BIT_KHR;
    //  gp_ci_.pTessellationState = nullptr; // TODO
    gp_ci_.pViewportState = &vp_state_ci_;

    // If using Dynamic Rendering, will need to be set to null
    // otherwise needs to be shared across libraries in the same executable pipeline
    gp_ci_.renderPass = layer_test_.renderPass();
    gp_ci_.subpass = 0;

    // TODO if renderPass is null, MS info is not needed
    gp_ci_.pMultisampleState = &pipe_ms_state_ci_;

    gp_ci_.stageCount = count;
    gp_ci_.pStages = info;

    InitPipelineCacheInfo();
}

void CreatePipelineHelper::InitFragmentOutputLibInfo(void *p_next) {
    InitDescriptorSetInfo();
    InitInputAndVertexInfo();
    InitMultisampleInfo();
    InitPipelineLayoutInfo();
    InitViewportInfo();
    InitDynamicStateInfo();
    InitRasterizationInfo();
    InitLineRasterizationInfo();
    InitBlendStateInfo();

    gpl_info.emplace(LvlInitStruct<VkGraphicsPipelineLibraryCreateInfoEXT>(p_next));
    gpl_info->flags = VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT;

    gp_ci_ = LvlInitStruct<VkGraphicsPipelineCreateInfo>(&gpl_info);
    gp_ci_.flags = VK_PIPELINE_CREATE_LIBRARY_BIT_KHR | VK_PIPELINE_CREATE_RETAIN_LINK_TIME_OPTIMIZATION_INFO_BIT_EXT;
    gp_ci_.pColorBlendState = &cb_ci_;
    gp_ci_.pMultisampleState = &pipe_ms_state_ci_;
    gp_ci_.pRasterizationState = &rs_state_ci_;

    // If using Dynamic Rendering, will need to be set to null
    // otherwise needs to be shared across libraries in the same executable pipeline
    gp_ci_.renderPass = layer_test_.renderPass();
    gp_ci_.subpass = 0;

    InitPipelineCacheInfo();
}

void CreatePipelineHelper::InitState() {
    descriptor_set_.reset(new OneOffDescriptorSet(layer_test_.DeviceObj(), dsl_bindings_));
    ASSERT_TRUE(descriptor_set_->Initialized());

    const std::vector<VkPushConstantRange> push_ranges(
        pipeline_layout_ci_.pPushConstantRanges,
        pipeline_layout_ci_.pPushConstantRanges + pipeline_layout_ci_.pushConstantRangeCount);
    pipeline_layout_ =
        VkPipelineLayoutObj(layer_test_.DeviceObj(), {&descriptor_set_->layout_}, push_ranges, pipeline_layout_ci_.flags);

    InitPipelineCache();
}

void CreatePipelineHelper::InitPipelineCache() {
    if (pipeline_cache_ != VK_NULL_HANDLE) {
        vk::DestroyPipelineCache(layer_test_.device(), pipeline_cache_, nullptr);
    }
    VkResult err = vk::CreatePipelineCache(layer_test_.device(), &pc_ci_, NULL, &pipeline_cache_);
    ASSERT_VK_SUCCESS(err);
}

void CreatePipelineHelper::LateBindPipelineInfo() {
    // By value or dynamically located items must be late bound
    gp_ci_.layout = pipeline_layout_.handle();
    if (gp_ci_.stageCount == 0) {
        gp_ci_.stageCount = shader_stages_.size();
        gp_ci_.pStages = shader_stages_.data();
    }
    if ((gp_ci_.pTessellationState == nullptr) && IsValidVkStruct(tess_ci_)) {
        gp_ci_.pTessellationState = &tess_ci_;
    }
    if ((gp_ci_.pDynamicState == nullptr) && IsValidVkStruct(dyn_state_ci_)) {
        gp_ci_.pDynamicState = &dyn_state_ci_;
    }
    if ((gp_ci_.pDepthStencilState == nullptr) && IsValidVkStruct(ds_ci_)) {
        gp_ci_.pDepthStencilState = &ds_ci_;
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
    pipeline_layout_ci_ = LvlInitStruct<VkPipelineLayoutCreateInfo>();
    pipeline_layout_ci_.setLayoutCount = 1;     // Not really changeable because InitState() sets exactly one pSetLayout
    pipeline_layout_ci_.pSetLayouts = nullptr;  // must bound after it is created
}

void CreateComputePipelineHelper::InitShaderInfo() {
    cs_ = std::make_unique<VkShaderObj>(&layer_test_, kMinimalShaderGlsl, VK_SHADER_STAGE_COMPUTE_BIT);
    // We shouldn't need a fragment shader but add it to be able to run on more devices
}

void CreateComputePipelineHelper::InitComputePipelineInfo() {
    cp_ci_ = LvlInitStruct<VkComputePipelineCreateInfo>();
    cp_ci_.flags = 0;
}

void CreateComputePipelineHelper::InitPipelineCacheInfo() {
    pc_ci_ = LvlInitStruct<VkPipelineCacheCreateInfo>();
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
    descriptor_set_.reset(new OneOffDescriptorSet(layer_test_.DeviceObj(), dsl_bindings_));
    ASSERT_TRUE(descriptor_set_->Initialized());

    const std::vector<VkPushConstantRange> push_ranges(
        pipeline_layout_ci_.pPushConstantRanges,
        pipeline_layout_ci_.pPushConstantRanges + pipeline_layout_ci_.pushConstantRangeCount);
    pipeline_layout_ = VkPipelineLayoutObj(layer_test_.DeviceObj(), {&descriptor_set_->layout_}, push_ranges);

    InitPipelineCache();
}

void CreateComputePipelineHelper::InitPipelineCache() {
    if (pipeline_cache_ != VK_NULL_HANDLE) {
        vk::DestroyPipelineCache(layer_test_.device(), pipeline_cache_, nullptr);
    }
    VkResult err = vk::CreatePipelineCache(layer_test_.device(), &pc_ci_, NULL, &pipeline_cache_);
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

BarrierQueueFamilyBase::QueueFamilyObjs::~QueueFamilyObjs() {
    delete command_buffer2;
    delete command_buffer;
    delete command_pool;
    delete queue;
}

void BarrierQueueFamilyBase::QueueFamilyObjs::Init(VkDeviceObj *device, uint32_t qf_index, VkQueue qf_queue,
                                                   VkCommandPoolCreateFlags cp_flags) {
    index = qf_index;
    queue = new VkQueueObj(qf_queue, qf_index);
    command_pool = new VkCommandPoolObj(device, qf_index, cp_flags);
    command_buffer = new VkCommandBufferObj(device, command_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, queue);
    command_buffer2 = new VkCommandBufferObj(device, command_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, queue);
};

BarrierQueueFamilyBase::Context::Context(VkLayerTest *test, const std::vector<uint32_t> &queue_family_indices) : layer_test(test) {
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

void BarrierQueueFamilyBase::Context::Reset() {
    layer_test->DeviceObj()->wait();
    for (auto &qf : queue_families) {
        vk::ResetCommandPool(layer_test->device(), qf.second.command_pool->handle(), 0);
    }
}

void BarrierQueueFamilyTestHelper::Init(std::vector<uint32_t> *families, bool image_memory, bool buffer_memory) {
    VkDeviceObj *device_obj = context_->layer_test->DeviceObj();

    image_.Init(32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0, families,
                image_memory);

    ASSERT_TRUE(image_.initialized());

    image_barrier_ = image_.image_memory_barrier(VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_TRANSFER_READ_BIT, image_.Layout(),
                                                 image_.Layout(), image_.subresource_range(VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1));

    VkMemoryPropertyFlags mem_prop = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    auto buffer_ci = VkBufferObj::create_info(256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, families);
    if (buffer_memory) {
        buffer_.init(*device_obj, buffer_ci, mem_prop);
    } else {
        buffer_.init_no_mem(*device_obj, buffer_ci);
    }
    ASSERT_TRUE(buffer_.initialized());
    buffer_barrier_ = buffer_.buffer_memory_barrier(VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_TRANSFER_READ_BIT, 0, VK_WHOLE_SIZE);
}

void Barrier2QueueFamilyTestHelper::Init(std::vector<uint32_t> *families, bool image_memory, bool buffer_memory) {
    VkDeviceObj *device_obj = context_->layer_test->DeviceObj();

    image_.Init(32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0, families,
                image_memory);

    ASSERT_TRUE(image_.initialized());

    image_barrier_ = image_.image_memory_barrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                                 VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_TRANSFER_READ_BIT, image_.Layout(),
                                                 image_.Layout(), image_.subresource_range(VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1));

    VkMemoryPropertyFlags mem_prop = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    auto buffer_ci = VkBufferObj::create_info(256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, families);
    if (buffer_memory) {
        buffer_.init(*device_obj, buffer_ci, mem_prop);
    } else {
        buffer_.init_no_mem(*device_obj, buffer_ci);
    }
    ASSERT_TRUE(buffer_.initialized());
    buffer_barrier_ = buffer_.buffer_memory_barrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                                    VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_TRANSFER_READ_BIT, 0, VK_WHOLE_SIZE);
}

BarrierQueueFamilyBase::QueueFamilyObjs *BarrierQueueFamilyBase::GetQueueFamilyInfo(Context *context, uint32_t qfi) {
    QueueFamilyObjs *qf;

    auto qf_it = context->queue_families.find(qfi);
    if (qf_it != context->queue_families.end()) {
        qf = &(qf_it->second);
    } else {
        qf = &(context->queue_families[context->default_index]);
    }
    return qf;
}

void BarrierQueueFamilyTestHelper::operator()(const std::string &img_err, const std::string &buf_err, uint32_t src, uint32_t dst,
                                              uint32_t queue_family_index, Modifier mod) {
    auto &monitor = context_->layer_test->Monitor();
    const bool has_img_err = img_err.size() > 0;
    const bool has_buf_err = buf_err.size() > 0;
    bool positive = !has_img_err && !has_buf_err;
    if (has_img_err) monitor.SetDesiredFailureMsg(kErrorBit | kWarningBit, img_err);
    if (has_buf_err) monitor.SetDesiredFailureMsg(kErrorBit | kWarningBit, buf_err);

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

    if (!positive) {
        monitor.VerifyFound();
    }
    context_->Reset();
};

void Barrier2QueueFamilyTestHelper::operator()(const std::string &img_err, const std::string &buf_err, uint32_t src, uint32_t dst,
                                               uint32_t queue_family_index, Modifier mod) {
    auto &monitor = context_->layer_test->Monitor();
    bool positive = true;
    if (img_err.length()) {
        monitor.SetDesiredFailureMsg(kErrorBit | kWarningBit, img_err);
        positive = false;
    }
    if (buf_err.length()) {
        monitor.SetDesiredFailureMsg(kErrorBit | kWarningBit, buf_err);
        positive = false;
    }

    image_barrier_.srcQueueFamilyIndex = src;
    image_barrier_.dstQueueFamilyIndex = dst;
    buffer_barrier_.srcQueueFamilyIndex = src;
    buffer_barrier_.dstQueueFamilyIndex = dst;

    auto dep_info = LvlInitStruct<VkDependencyInfoKHR>();
    dep_info.bufferMemoryBarrierCount = 1;
    dep_info.pBufferMemoryBarriers = &buffer_barrier_;
    dep_info.imageMemoryBarrierCount = 1;
    dep_info.pImageMemoryBarriers = &image_barrier_;

    QueueFamilyObjs *qf = GetQueueFamilyInfo(context_, queue_family_index);

    VkCommandBufferObj *command_buffer = qf->command_buffer;
    for (int cb_repeat = 0; cb_repeat < (mod == Modifier::DOUBLE_COMMAND_BUFFER ? 2 : 1); cb_repeat++) {
        command_buffer->begin();
        for (int repeat = 0; repeat < (mod == Modifier::DOUBLE_RECORD ? 2 : 1); repeat++) {
            command_buffer->PipelineBarrier2KHR(&dep_info);
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

    if (!positive) {
        monitor.VerifyFound();
    }
    context_->Reset();
};

void VkSyncValTest::InitSyncValFramework(bool enable_queue_submit_validation) {
    // Enable synchronization validation

    // Optional feature definition, add if requested (but they can't be defined at the conditional scope)
    const char *kEnableQueuSubmitSyncValidation = "VALIDATION_CHECK_ENABLE_SYNCHRONIZATION_VALIDATION_QUEUE_SUBMIT";
    VkLayerSettingValueDataEXT qs_setting_string_value{};
    qs_setting_string_value.arrayString.pCharArray = kEnableQueuSubmitSyncValidation;
    qs_setting_string_value.arrayString.count = strlen(qs_setting_string_value.arrayString.pCharArray);
    VkLayerSettingValueEXT qs_enable_setting_val = {"enables", VK_LAYER_SETTING_VALUE_TYPE_STRING_ARRAY_EXT,
                                                    qs_setting_string_value};
    VkLayerSettingsEXT qs_settings{VK_STRUCTURE_TYPE_INSTANCE_LAYER_SETTINGS_EXT, nullptr, 1, &qs_enable_setting_val};

    if (enable_queue_submit_validation) {
        features_.pNext = &qs_settings;
    }
    InitFramework(m_errorMonitor, &features_);
}

void print_android(const char *c) {
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    __android_log_print(ANDROID_LOG_INFO, "VulkanLayerValidationTests", "%s", c);
#endif  // VK_USE_PLATFORM_ANDROID_KHR
}

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
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
        } else if (info.result()->Skipped()) {
            result.append("[  SKIPPED ]");
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

static void destroyActivity(struct android_app *app) {
    ANativeActivity_finish(app->activity);

    // Wait for APP_CMD_DESTROY
    while (app->destroyRequested == 0) {
        struct android_poll_source *source = nullptr;
        int events = 0;
        int result = ALooper_pollAll(-1, nullptr, &events, reinterpret_cast<void **>(&source));

        if ((result >= 0) && (source)) {
            source->process(app, source);
        } else {
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

            destroyActivity(app);
            raise(SIGTERM);
            return;
        }
    }
}
#endif

#if defined(_WIN32) && !defined(NDEBUG)
#include <crtdbg.h>
#endif

// Defining VVL_TESTS_USE_CUSTOM_TEST_FRAMEWORK allows downstream users
// to inject custom test framework changes. This includes the ability
// to override the main entry point of the test executable in order to
// add custom command line arguments and use a custom test environment
// class. This #ifndef thus makes sure that when the definition is
// present we do not include the default main entry point.
#ifndef VVL_TESTS_USE_CUSTOM_TEST_FRAMEWORK
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
#endif
