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
#include "pipeline_helper.h"
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

bool FormatIsSupported(VkPhysicalDevice phy, VkFormat format, VkImageTiling tiling, VkFormatFeatureFlags features) {
    VkFormatProperties format_props;
    vk::GetPhysicalDeviceFormatProperties(phy, format, &format_props);
    VkFormatFeatureFlags phy_features =
        (VK_IMAGE_TILING_OPTIMAL == tiling ? format_props.optimalTilingFeatures : format_props.linearTilingFeatures);
    return (0 != (phy_features & features));
}

bool FormatFeaturesAreSupported(VkPhysicalDevice phy, VkFormat format, VkImageTiling tiling, VkFormatFeatureFlags features) {
    VkFormatProperties format_props;
    vk::GetPhysicalDeviceFormatProperties(phy, format, &format_props);
    VkFormatFeatureFlags phy_features =
        (VK_IMAGE_TILING_OPTIMAL == tiling ? format_props.optimalTilingFeatures : format_props.linearTilingFeatures);
    return (features == (phy_features & features));
}

bool ImageFormatIsSupported(const VkInstance inst, const VkPhysicalDevice phy, const VkImageCreateInfo info,
                            const VkFormatFeatureFlags features) {
    // Verify physical device support of format features
    if (!FormatFeaturesAreSupported(phy, info.format, info.tiling, features)) {
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
        VkPhysicalDeviceImageFormatInfo2KHR fmt_info = vku::InitStructHelper();
        fmt_info.format = info.format;
        fmt_info.type = info.imageType;
        fmt_info.tiling = info.tiling;
        fmt_info.usage = info.usage;
        fmt_info.flags = info.flags;

        VkImageFormatProperties2KHR fmt_props = vku::InitStructHelper();
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

    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
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

void ReleaseNullFence(ThreadTestData *data) {
    for (int i = 0; i < 40000; i++) {
        vk::DestroyFence(data->device, VK_NULL_HANDLE, NULL);
        if (*data->bailout) {
            break;
        }
    }
}

void TestRenderPassCreate(ErrorMonitor *error_monitor, const vkt::Device &device, const VkRenderPassCreateInfo &create_info,
                          bool rp2_supported, const char *rp1_vuid, const char *rp2_vuid) {
    if (rp1_vuid) {
        // If the second VUID is not provided, set it equal to the first VUID.  In this way,
        // we can check both vkCreateRenderPass and vkCreateRenderPass2 with the same VUID
        // if rp2_supported is true;
        if (rp2_supported && !rp2_vuid) {
            rp2_vuid = rp1_vuid;
        }

        error_monitor->SetDesiredFailureMsg(kErrorBit, rp1_vuid);
        vkt::RenderPass rp(device, create_info);
        error_monitor->VerifyFound();
    }

    if (rp2_supported && rp2_vuid) {
        safe_VkRenderPassCreateInfo2 create_info2 = ConvertVkRenderPassCreateInfoToV2KHR(create_info);

        const auto vkCreateRenderPass2KHR =
            reinterpret_cast<PFN_vkCreateRenderPass2KHR>(vk::GetDeviceProcAddr(device, "vkCreateRenderPass2KHR"));
        // For API version < 1.2 where the extension was not enabled
        if (vkCreateRenderPass2KHR) {
            error_monitor->SetDesiredFailureMsg(kErrorBit, rp2_vuid);
            vkt::RenderPass rp2_khr(device, *create_info2.ptr(), true);
            error_monitor->VerifyFound();
        }

        const auto vkCreateRenderPass2 =
            reinterpret_cast<PFN_vkCreateRenderPass2>(vk::GetDeviceProcAddr(device, "vkCreateRenderPass2"));
        // For API version >= 1.2, try core entrypoint
        if (vkCreateRenderPass2) {
            error_monitor->SetDesiredFailureMsg(kErrorBit, rp2_vuid);
            vkt::RenderPass rp2_core(device, *create_info2.ptr(), false);
            error_monitor->VerifyFound();
        }
    }
}

void PositiveTestRenderPassCreate(ErrorMonitor *error_monitor, const vkt::Device &device, const VkRenderPassCreateInfo &create_info,
                                  bool rp2_supported) {
    vkt::RenderPass rp(device, create_info);
    if (rp2_supported) {
        vkt::RenderPass rp2(device, *ConvertVkRenderPassCreateInfoToV2KHR(create_info).ptr(), true);
    }
}

void PositiveTestRenderPass2KHRCreate(const vkt::Device &device, const VkRenderPassCreateInfo2KHR &create_info) {
    vkt::RenderPass rp(device, create_info, true);
}

void TestRenderPass2KHRCreate(ErrorMonitor &error_monitor, const vkt::Device &device, const VkRenderPassCreateInfo2KHR &create_info,
                              const std::initializer_list<const char *> &vuids) {
    for (auto vuid : vuids) {
        error_monitor.SetDesiredFailureMsg(kErrorBit, vuid);
    }
    vkt::RenderPass rp(device, create_info, true);
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

void ValidOwnershipTransferOp(ErrorMonitor *monitor, vkt::CommandBuffer *cb, VkPipelineStageFlags src_stages,
                              VkPipelineStageFlags dst_stages, const VkBufferMemoryBarrier *buf_barrier,
                              const VkImageMemoryBarrier *img_barrier) {
    cb->begin();
    uint32_t num_buf_barrier = (buf_barrier) ? 1 : 0;
    uint32_t num_img_barrier = (img_barrier) ? 1 : 0;
    vk::CmdPipelineBarrier(cb->handle(), src_stages, dst_stages, 0, 0, nullptr, num_buf_barrier, buf_barrier, num_img_barrier,
                           img_barrier);
    cb->end();
    cb->QueueCommandBuffer();  // Implicitly waits
}

void ValidOwnershipTransfer(ErrorMonitor *monitor, vkt::CommandBuffer *cb_from, vkt::CommandBuffer *cb_to,
                            VkPipelineStageFlags src_stages, VkPipelineStageFlags dst_stages,
                            const VkBufferMemoryBarrier *buf_barrier, const VkImageMemoryBarrier *img_barrier) {
    ValidOwnershipTransferOp(monitor, cb_from, src_stages, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, buf_barrier, img_barrier);
    ValidOwnershipTransferOp(monitor, cb_to, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, dst_stages, buf_barrier, img_barrier);
}

void ValidOwnershipTransferOp(ErrorMonitor *monitor, vkt::CommandBuffer *cb, const VkBufferMemoryBarrier2KHR *buf_barrier,
                              const VkImageMemoryBarrier2KHR *img_barrier) {
    cb->begin();
    VkDependencyInfoKHR dep_info = vku::InitStructHelper();
    dep_info.bufferMemoryBarrierCount = (buf_barrier) ? 1 : 0;
    dep_info.pBufferMemoryBarriers = buf_barrier;
    dep_info.imageMemoryBarrierCount = (img_barrier) ? 1 : 0;
    dep_info.pImageMemoryBarriers = img_barrier;
    vk::CmdPipelineBarrier2KHR(cb->handle(), &dep_info);
    cb->end();
    cb->QueueCommandBuffer();  // Implicitly waits
}

void ValidOwnershipTransfer(ErrorMonitor *monitor, vkt::CommandBuffer *cb_from, vkt::CommandBuffer *cb_to,
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
        VkFormatProperties3KHR fmt_props_3 = vku::InitStructHelper();
        VkFormatProperties2 fmt_props_2 = vku::InitStructHelper(&fmt_props_3);
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

    VkPhysicalDeviceExternalSemaphoreInfo info = vku::InitStructHelper();
    info.handleType = handle_type;
    VkExternalSemaphoreProperties properties = vku::InitStructHelper();
    vk::GetPhysicalDeviceExternalSemaphoreProperties(gpu, &info, &properties);
    return (properties.externalSemaphoreFeatures & export_import_flags) == export_import_flags;
}

void AllocateDisjointMemory(vkt::Device *device, PFN_vkGetImageMemoryRequirements2KHR fp, VkImage mp_image,
                            VkDeviceMemory *mp_image_mem, VkImageAspectFlagBits plane) {
    VkImagePlaneMemoryRequirementsInfo image_plane_req = vku::InitStructHelper();
    image_plane_req.planeAspect = plane;

    VkImageMemoryRequirementsInfo2 mem_req_info2 = vku::InitStructHelper(&image_plane_req);
    mem_req_info2.image = mp_image;

    VkMemoryRequirements2 mp_image_mem_reqs2 = vku::InitStructHelper();

    fp(device->device(), &mem_req_info2, &mp_image_mem_reqs2);

    VkMemoryAllocateInfo mp_image_alloc_info = vku::InitStructHelper();
    mp_image_alloc_info.allocationSize = mp_image_mem_reqs2.memoryRequirements.size;
    ASSERT_TRUE(device->phy().set_memory_type(mp_image_mem_reqs2.memoryRequirements.memoryTypeBits, &mp_image_alloc_info, 0));
    ASSERT_EQ(VK_SUCCESS, vk::AllocateMemory(device->device(), &mp_image_alloc_info, NULL, mp_image_mem));
}

void CreateSamplerTest(VkLayerTest &test, const VkSamplerCreateInfo *create_info, const std::string &code) {
    if (code.length()) {
        test.Monitor().SetDesiredFailureMsg(kErrorBit | kWarningBit, code);
    }

    vkt::Sampler sampler(*test.DeviceObj(), *create_info);

    if (code.length()) {
        test.Monitor().VerifyFound();
    }
}

void CreateBufferTest(VkLayerTest &test, const VkBufferCreateInfo *create_info, const std::string &code) {
    if (code.length()) {
        test.Monitor().SetDesiredFailureMsg(kErrorBit, code);
    }
    vkt::Buffer buffer(*test.DeviceObj(), *create_info, vkt::no_mem);
    if (code.length()) {
        test.Monitor().VerifyFound();
    }
}

void CreateImageTest(VkLayerTest &test, const VkImageCreateInfo *create_info, const std::string &code) {
    if (code.length()) {
        test.Monitor().SetDesiredFailureMsg(kErrorBit, code);
    }
    vkt::Image image(*test.DeviceObj(), *create_info, vkt::no_mem);
    if (code.length()) {
        test.Monitor().VerifyFound();
    }
}

void CreateBufferViewTest(VkLayerTest &test, const VkBufferViewCreateInfo *create_info, const std::vector<std::string> &codes) {
    if (codes.size()) {
        std::for_each(codes.begin(), codes.end(), [&](const std::string &s) { test.Monitor().SetDesiredFailureMsg(kErrorBit, s); });
    }
    vkt::BufferView view(*test.DeviceObj(), *create_info);
    if (codes.size()) {
        test.Monitor().VerifyFound();
    }
}

void CreateImageViewTest(VkLayerTest &test, const VkImageViewCreateInfo *create_info, const std::string &code) {
    if (code.length()) {
        test.Monitor().SetDesiredFailureMsg(kErrorBit, code);
    }
    vkt::ImageView view(*test.DeviceObj(), *create_info);
    if (code.length()) {
        test.Monitor().VerifyFound();
    }
}

VkSamplerCreateInfo SafeSaneSamplerCreateInfo() {
    VkSamplerCreateInfo sampler_create_info = vku::InitStructHelper();
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

void VkLayerTest::Init(VkPhysicalDeviceFeatures *features, VkPhysicalDeviceFeatures2 *features2, void *instance_pnext) {
    RETURN_IF_SKIP(InitFramework(instance_pnext));
    RETURN_IF_SKIP(InitState(features, features2));
}

vkt::CommandBuffer *VkLayerTest::CommandBuffer() { return m_commandBuffer; }

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

    app_info_ = vku::InitStructHelper();
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
    m_target_api_version = target_api_version;
    app_info_.apiVersion = m_target_api_version.Value();
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
        VkPhysicalDeviceDriverProperties driver_properties = vku::InitStructHelper();
        VkPhysicalDeviceProperties2 physical_device_properties2 = vku::InitStructHelper(&driver_properties);
        GetPhysicalDeviceProperties2(physical_device_properties2);
        return (driver_properties.driverID == driver_id);
    }
}

bool VkLayerTest::LoadDeviceProfileLayer(
    PFN_vkSetPhysicalDeviceFormatPropertiesEXT &fpvkSetPhysicalDeviceFormatPropertiesEXT,
    PFN_vkGetOriginalPhysicalDeviceFormatPropertiesEXT &fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT) {
    if (IsPlatformMockICD()) {
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
    if (IsPlatformMockICD()) {
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
    if (IsPlatformMockICD()) {
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
    if (IsPlatformMockICD()) {
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
    if (IsPlatformMockICD()) {
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

void SetImageLayout(vkt::Device *device, VkImageAspectFlags aspect, VkImage image, VkImageLayout image_layout) {
    vkt::CommandPool pool(*device, device->graphics_queue_node_index_);
    vkt::CommandBuffer cmd_buf(device, &pool);

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

    vk::CmdPipelineBarrier(cmd_buf.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr,
                           0, nullptr, 1, &layout_barrier);
    cmd_buf.end();

    cmd_buf.QueueCommandBuffer();
}

void print_android(const char *c) {
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    __android_log_print(ANDROID_LOG_INFO, "VulkanLayerValidationTests", "%s", c);
#endif  // VK_USE_PLATFORM_ANDROID_KHR
}

#if defined(VK_USE_PLATFORM_ANDROID_KHR) && !defined(VVL_MOCK_ANDROID)
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

// Makes any failed assertion throw, allowing for graceful cleanup of resources instead of hard aborts
class ThrowListener : public testing::EmptyTestEventListener {
    void OnTestPartResult(const testing::TestPartResult &result) override {
        if (result.type() == testing::TestPartResult::kFatalFailure) {
            // We need to make sure an exception wasn't already thrown so we dont throw another exception at the same time
            std::exception_ptr ex = std::current_exception();
            if (ex) {
                return;
            }
            throw testing::AssertionException(result);
        }
    }
};

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
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
#endif
    // Avoid "Abort, Retry, Ignore" dialog boxes
    _set_error_mode(_OUT_TO_STDERR);
    _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
#endif

    ::testing::InitGoogleTest(&argc, argv);
    VkTestFramework::InitArgs(&argc, argv);

    ::testing::AddGlobalTestEnvironment(new TestEnvironment);
    ::testing::UnitTest::GetInstance()->listeners().Append(new ThrowListener);

    result = RUN_ALL_TESTS();

    VkTestFramework::Finish();
    return result;
}
#endif
