/* Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
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

#include "best_practices/best_practices_validation.h"
#include "layer_chassis_dispatch.h"
#include "best_practices/best_practices_error_enums.h"
#include "core_checks/shader_validation.h"
#include "sync/sync_utils.h"
#include "state_tracker/cmd_buffer_state.h"
#include "state_tracker/device_state.h"
#include "state_tracker/render_pass_state.h"

#include <string>
#include <bitset>
#include <memory>

struct VendorSpecificInfo {
    EnableFlags vendor_id;
    std::string name;
};

const std::map<BPVendorFlagBits, VendorSpecificInfo> kVendorInfo = {{kBPVendorArm, {vendor_specific_arm, "Arm"}},
                                                                    {kBPVendorAMD, {vendor_specific_amd, "AMD"}},
                                                                    {kBPVendorIMG, {vendor_specific_img, "IMG"}},
                                                                    {kBPVendorNVIDIA, {vendor_specific_nvidia, "NVIDIA"}}};

const SpecialUseVUIDs kSpecialUseInstanceVUIDs{
    kVUID_BestPractices_CreateInstance_SpecialUseExtension_CADSupport,
    kVUID_BestPractices_CreateInstance_SpecialUseExtension_D3DEmulation,
    kVUID_BestPractices_CreateInstance_SpecialUseExtension_DevTools,
    kVUID_BestPractices_CreateInstance_SpecialUseExtension_Debugging,
    kVUID_BestPractices_CreateInstance_SpecialUseExtension_GLEmulation,
};

const SpecialUseVUIDs kSpecialUseDeviceVUIDs{
    kVUID_BestPractices_CreateDevice_SpecialUseExtension_CADSupport,
    kVUID_BestPractices_CreateDevice_SpecialUseExtension_D3DEmulation,
    kVUID_BestPractices_CreateDevice_SpecialUseExtension_DevTools,
    kVUID_BestPractices_CreateDevice_SpecialUseExtension_Debugging,
    kVUID_BestPractices_CreateDevice_SpecialUseExtension_GLEmulation,
};

static constexpr std::array<VkFormat, 12> kCustomClearColorCompressedFormatsNVIDIA = {
    VK_FORMAT_R8G8B8A8_UNORM,           VK_FORMAT_B8G8R8A8_UNORM,           VK_FORMAT_A8B8G8R8_UNORM_PACK32,
    VK_FORMAT_A2R10G10B10_UNORM_PACK32, VK_FORMAT_A2B10G10R10_UNORM_PACK32, VK_FORMAT_R16G16B16A16_UNORM,
    VK_FORMAT_R16G16B16A16_SNORM,       VK_FORMAT_R16G16B16A16_UINT,        VK_FORMAT_R16G16B16A16_SINT,
    VK_FORMAT_R16G16B16A16_SFLOAT,      VK_FORMAT_R32G32B32A32_SFLOAT,      VK_FORMAT_B10G11R11_UFLOAT_PACK32,
};

ReadLockGuard BestPractices::ReadLock() const {
    if (fine_grained_locking) {
        return ReadLockGuard(validation_object_mutex, std::defer_lock);
    } else {
        return ReadLockGuard(validation_object_mutex);
    }
}

WriteLockGuard BestPractices::WriteLock() {
    if (fine_grained_locking) {
        return WriteLockGuard(validation_object_mutex, std::defer_lock);
    } else {
        return WriteLockGuard(validation_object_mutex);
    }
}

std::shared_ptr<CMD_BUFFER_STATE> BestPractices::CreateCmdBufferState(VkCommandBuffer cb,
                                                                      const VkCommandBufferAllocateInfo* pCreateInfo,
                                                                      const COMMAND_POOL_STATE* pool) {
    return std::static_pointer_cast<CMD_BUFFER_STATE>(std::make_shared<bp_state::CommandBuffer>(this, cb, pCreateInfo, pool));
}

bp_state::CommandBuffer::CommandBuffer(BestPractices* bp, VkCommandBuffer cb, const VkCommandBufferAllocateInfo* pCreateInfo,
                                       const COMMAND_POOL_STATE* pool)
    : CMD_BUFFER_STATE(bp, cb, pCreateInfo, pool) {}

bool BestPractices::VendorCheckEnabled(BPVendorFlags vendors) const {
    for (const auto& vendor : kVendorInfo) {
        if (vendors & vendor.first && enabled[vendor.second.vendor_id]) {
            return true;
        }
    }
    return false;
}

const char* VendorSpecificTag(BPVendorFlags vendors) {
    // Cache built vendor tags in a map
    static vvl::unordered_map<BPVendorFlags, std::string> tag_map;

    auto res = tag_map.find(vendors);
    if (res == tag_map.end()) {
        // Build the vendor tag string
        std::stringstream vendor_tag;

        vendor_tag << "[";
        bool first_vendor = true;
        for (const auto& vendor : kVendorInfo) {
            if (vendors & vendor.first) {
                if (!first_vendor) {
                    vendor_tag << ", ";
                }
                vendor_tag << vendor.second.name;
                first_vendor = false;
            }
        }
        vendor_tag << "]";

        tag_map[vendors] = vendor_tag.str();
        res = tag_map.find(vendors);
    }

    return res->second.c_str();
}

const char* DepReasonToString(ExtDeprecationReason reason) {
    switch (reason) {
        case kExtPromoted:
            return "promoted to";
            break;
        case kExtObsoleted:
            return "obsoleted by";
            break;
        case kExtDeprecated:
            return "deprecated by";
            break;
        default:
            return "";
            break;
    }
}

bool BestPractices::ValidateDeprecatedExtensions(const char* api_name, const char* extension_name, uint32_t version,
                                                 const char* vuid) const {
    bool skip = false;
    auto dep_info_it = deprecated_extensions.find(extension_name);
    if (dep_info_it != deprecated_extensions.end()) {
        auto dep_info = dep_info_it->second;
        if (((dep_info.target.compare("VK_VERSION_1_1") == 0) && (version >= VK_API_VERSION_1_1)) ||
            ((dep_info.target.compare("VK_VERSION_1_2") == 0) && (version >= VK_API_VERSION_1_2)) ||
            ((dep_info.target.compare("VK_VERSION_1_3") == 0) && (version >= VK_API_VERSION_1_3))) {
            skip |=
                LogWarning(instance, vuid, "%s(): Attempting to enable deprecated extension %s, but this extension has been %s %s.",
                           api_name, extension_name, DepReasonToString(dep_info.reason), (dep_info.target).c_str());
        } else if (dep_info.target.find("VK_VERSION") == std::string::npos) {
            if (dep_info.target.length() == 0) {
                skip |= LogWarning(instance, vuid,
                                   "%s(): Attempting to enable deprecated extension %s, but this extension has been deprecated "
                                   "without replacement.",
                                   api_name, extension_name);
            } else {
                skip |= LogWarning(instance, vuid,
                                   "%s(): Attempting to enable deprecated extension %s, but this extension has been %s %s.",
                                   api_name, extension_name, DepReasonToString(dep_info.reason), (dep_info.target).c_str());
            }
        }
    }
    return skip;
}

bool BestPractices::ValidateSpecialUseExtensions(const char* api_name, const char* extension_name,
                                                 const SpecialUseVUIDs& special_use_vuids) const {
    bool skip = false;
    auto dep_info_it = special_use_extensions.find(extension_name);

    if (dep_info_it != special_use_extensions.end()) {
        const char* const format =
            "%s(): Attempting to enable extension %s, but this extension is intended to support %s "
            "and it is strongly recommended that it be otherwise avoided.";
        auto& special_uses = dep_info_it->second;

        if (special_uses.find("cadsupport") != std::string::npos) {
            skip |= LogWarning(instance, special_use_vuids.cadsupport, format, api_name, extension_name,
                               "specialized functionality used by CAD/CAM applications");
        }
        if (special_uses.find("d3demulation") != std::string::npos) {
            skip |= LogWarning(instance, special_use_vuids.d3demulation, format, api_name, extension_name,
                               "D3D emulation layers, and applications ported from D3D, by adding functionality specific to D3D");
        }
        if (special_uses.find("devtools") != std::string::npos) {
            skip |= LogWarning(instance, special_use_vuids.devtools, format, api_name, extension_name,
                               "developer tools such as capture-replay libraries");
        }
        if (special_uses.find("debugging") != std::string::npos) {
            skip |= LogWarning(instance, special_use_vuids.debugging, format, api_name, extension_name,
                               "use by applications when debugging");
        }
        if (special_uses.find("glemulation") != std::string::npos) {
            skip |= LogWarning(
                instance, special_use_vuids.glemulation, format, api_name, extension_name,
                "OpenGL and/or OpenGL ES emulation layers, and applications ported from those APIs, by adding functionality "
                "specific to those APIs");
        }
    }
    return skip;
}

bool BestPractices::PreCallValidateCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                                  VkInstance* pInstance) const {
    bool skip = false;

    for (uint32_t i = 0; i < pCreateInfo->enabledExtensionCount; i++) {
        if (white_list(pCreateInfo->ppEnabledExtensionNames[i], kDeviceExtensionNames)) {
            skip |= LogWarning(instance, kVUID_BestPractices_CreateInstance_ExtensionMismatch,
                               "vkCreateInstance(): Attempting to enable Device Extension %s at CreateInstance time.",
                               pCreateInfo->ppEnabledExtensionNames[i]);
        }
        uint32_t specified_version =
            (pCreateInfo->pApplicationInfo ? pCreateInfo->pApplicationInfo->apiVersion : VK_API_VERSION_1_0);
        skip |= ValidateDeprecatedExtensions("CreateInstance", pCreateInfo->ppEnabledExtensionNames[i], specified_version,
                                             kVUID_BestPractices_CreateInstance_DeprecatedExtension);
        skip |= ValidateSpecialUseExtensions("CreateInstance", pCreateInfo->ppEnabledExtensionNames[i], kSpecialUseInstanceVUIDs);
    }

    return skip;
}

bool BestPractices::PreCallValidateCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkDevice* pDevice) const {
    bool skip = false;

    // get API version of physical device passed when creating device.
    VkPhysicalDeviceProperties physical_device_properties{};
    DispatchGetPhysicalDeviceProperties(physicalDevice, &physical_device_properties);
    auto device_api_version = physical_device_properties.apiVersion;

    // Check api versions and log an info message when instance api Version is higher than version on device.
    if (api_version > device_api_version) {
        std::string inst_api_name = StringAPIVersion(api_version);
        std::string dev_api_name = StringAPIVersion(device_api_version);

        skip |= LogInfo(device, kVUID_BestPractices_CreateDevice_API_Mismatch,
                        "vkCreateDevice(): API Version of current instance, %s is higher than API Version on device, %s",
                        inst_api_name.c_str(), dev_api_name.c_str());
    }

    std::vector<std::string> extensions;
    {
        uint32_t property_count = 0;
        if (DispatchEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &property_count, nullptr) == VK_SUCCESS) {
            std::vector<VkExtensionProperties> property_list(property_count);
            if (DispatchEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &property_count, property_list.data()) ==
                VK_SUCCESS) {
                extensions.reserve(property_list.size());
                for (const VkExtensionProperties& properties : property_list) {
                    extensions.push_back(properties.extensionName);
                }
            }
        }
    }

    for (uint32_t i = 0; i < pCreateInfo->enabledExtensionCount; i++) {
        const char* extension_name = pCreateInfo->ppEnabledExtensionNames[i];

        uint32_t extension_api_version = std::min(api_version, device_api_version);

        if (white_list(extension_name, kInstanceExtensionNames)) {
            skip |=
                LogWarning(instance, kVUID_BestPractices_CreateDevice_ExtensionMismatch,
                           "vkCreateDevice(): Attempting to enable Instance Extension %s at CreateDevice time.", extension_name);
            extension_api_version = api_version;
        }

        skip |= ValidateDeprecatedExtensions("CreateDevice", extension_name, extension_api_version,
                                             kVUID_BestPractices_CreateDevice_DeprecatedExtension);
        skip |= ValidateSpecialUseExtensions("CreateDevice", extension_name, kSpecialUseDeviceVUIDs);
    }

    const auto bp_pd_state = Get<bp_state::PhysicalDevice>(physicalDevice);
    if ((bp_pd_state->vkGetPhysicalDeviceFeaturesState == UNCALLED) && (pCreateInfo->pEnabledFeatures != NULL)) {
        skip |= LogWarning(device, kVUID_BestPractices_CreateDevice_PDFeaturesNotCalled,
                           "vkCreateDevice() called before getting physical device features from vkGetPhysicalDeviceFeatures().");
    }

    if ((VendorCheckEnabled(kBPVendorArm) || VendorCheckEnabled(kBPVendorAMD) || VendorCheckEnabled(kBPVendorIMG)) &&
        (pCreateInfo->pEnabledFeatures != nullptr) && (pCreateInfo->pEnabledFeatures->robustBufferAccess == VK_TRUE)) {
        skip |= LogPerformanceWarning(
            device, kVUID_BestPractices_CreateDevice_RobustBufferAccess,
            "%s %s %s: vkCreateDevice() called with enabled robustBufferAccess. Use robustBufferAccess as a debugging tool during "
            "development. Enabling it causes loss in performance for accesses to uniform buffers and shader storage "
            "buffers. Disable robustBufferAccess in release builds. Only leave it enabled if the application use-case "
            "requires the additional level of reliability due to the use of unverified user-supplied draw parameters.",
            VendorSpecificTag(kBPVendorArm), VendorSpecificTag(kBPVendorAMD), VendorSpecificTag(kBPVendorIMG));
    }

    const bool enabled_pageable_device_local_memory = IsExtEnabled(device_extensions.vk_ext_pageable_device_local_memory);
    if (VendorCheckEnabled(kBPVendorNVIDIA) && !enabled_pageable_device_local_memory &&
        std::find(extensions.begin(), extensions.end(), VK_EXT_PAGEABLE_DEVICE_LOCAL_MEMORY_EXTENSION_NAME) != extensions.end()) {
        skip |=
            LogPerformanceWarning(device, kVUID_BestPractices_CreateDevice_PageableDeviceLocalMemory,
                                  "%s vkCreateDevice() called without pageable device local memory. "
                                  "Use pageableDeviceLocalMemory from VK_EXT_pageable_device_local_memory when it is available.",
                                  VendorSpecificTag(kBPVendorNVIDIA));
    }

    return skip;
}

bool BestPractices::PreCallValidateCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer) const {
    bool skip = false;

    if ((pCreateInfo->queueFamilyIndexCount > 1) && (pCreateInfo->sharingMode == VK_SHARING_MODE_EXCLUSIVE)) {
        std::stringstream buffer_hex;
        buffer_hex << "0x" << std::hex << HandleToUint64(pBuffer);

        skip |= LogWarning(
            device, kVUID_BestPractices_SharingModeExclusive,
            "Warning: Buffer (%s) specifies a sharing mode of VK_SHARING_MODE_EXCLUSIVE while specifying multiple queues "
            "(queueFamilyIndexCount of %" PRIu32 ").",
            buffer_hex.str().c_str(), pCreateInfo->queueFamilyIndexCount);
    }

    return skip;
}

bool BestPractices::PreCallValidateCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkImage* pImage) const {
    bool skip = false;

    if ((pCreateInfo->queueFamilyIndexCount > 1) && (pCreateInfo->sharingMode == VK_SHARING_MODE_EXCLUSIVE)) {
        std::stringstream image_hex;
        image_hex << "0x" << std::hex << HandleToUint64(pImage);

        skip |=
            LogWarning(device, kVUID_BestPractices_SharingModeExclusive,
                       "Warning: Image (%s) specifies a sharing mode of VK_SHARING_MODE_EXCLUSIVE while specifying multiple queues "
                       "(queueFamilyIndexCount of %" PRIu32 ").",
                       image_hex.str().c_str(), pCreateInfo->queueFamilyIndexCount);
    }

    if ((pCreateInfo->flags & VK_IMAGE_CREATE_EXTENDED_USAGE_BIT) && !(pCreateInfo->flags & VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT)) {
        skip |= LogWarning(device, kVUID_BestPractices_ImageCreateFlags,
                           "vkCreateImage(): pCreateInfo->flags has VK_IMAGE_CREATE_EXTENDED_USAGE_BIT set, but not "
                           "VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT, therefore image views created from this image will have to use the "
                           "same format and VK_IMAGE_CREATE_EXTENDED_USAGE_BIT will not have any effect.");
    }

    if (VendorCheckEnabled(kBPVendorArm) || VendorCheckEnabled(kBPVendorIMG)) {
        if (pCreateInfo->samples > VK_SAMPLE_COUNT_1_BIT && !(pCreateInfo->usage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT)) {
            skip |= LogPerformanceWarning(
                device, kVUID_BestPractices_CreateImage_NonTransientMSImage,
                "%s %s vkCreateImage(): Trying to create a multisampled image, but createInfo.usage did not have "
                "VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT set. Multisampled images may be resolved on-chip, "
                "and do not need to be backed by physical storage. "
                "TRANSIENT_ATTACHMENT allows tiled GPUs to not back the multisampled image with physical memory.",
                VendorSpecificTag(kBPVendorArm), VendorSpecificTag(kBPVendorIMG));
        }
    }

    if (VendorCheckEnabled(kBPVendorArm) && pCreateInfo->samples > kMaxEfficientSamplesArm) {
        skip |= LogPerformanceWarning(
            device, kVUID_BestPractices_CreateImage_TooLargeSampleCount,
            "%s vkCreateImage(): Trying to create an image with %u samples. "
            "The hardware revision may not have full throughput for framebuffers with more than %u samples.",
            VendorSpecificTag(kBPVendorArm), static_cast<uint32_t>(pCreateInfo->samples), kMaxEfficientSamplesArm);
    }

    if (VendorCheckEnabled(kBPVendorIMG) && pCreateInfo->samples > kMaxEfficientSamplesImg) {
        skip |= LogPerformanceWarning(
            device, kVUID_BestPractices_CreateImage_TooLargeSampleCount,
            "%s vkCreateImage(): Trying to create an image with %u samples. "
            "The device may not have full support for true multisampling for images with more than %u samples. "
            "XT devices support up to 8 samples, XE up to 4 samples.",
            VendorSpecificTag(kBPVendorIMG), static_cast<uint32_t>(pCreateInfo->samples), kMaxEfficientSamplesImg);
    }

    if (VendorCheckEnabled(kBPVendorIMG) && (pCreateInfo->format == VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG ||
                                             pCreateInfo->format == VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG ||
                                             pCreateInfo->format == VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG ||
                                             pCreateInfo->format == VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG ||
                                             pCreateInfo->format == VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG ||
                                             pCreateInfo->format == VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG ||
                                             pCreateInfo->format == VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG ||
                                             pCreateInfo->format == VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG)) {
        skip |= LogPerformanceWarning(device, kVUID_BestPractices_Texture_Format_PVRTC_Outdated,
                                      "%s vkCreateImage(): Trying to create an image with a PVRTC format. Both PVRTC1 and PVRTC2 "
                                      "are slower than standard image formats on PowerVR GPUs, prefer ETC, BC, ASTC, etc.",
                                      VendorSpecificTag(kBPVendorIMG));
    }

    if (VendorCheckEnabled(kBPVendorAMD)) {
        std::stringstream image_hex;
        image_hex << "0x" << std::hex << HandleToUint64(pImage);

        if ((pCreateInfo->usage & (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) &&
            (pCreateInfo->sharingMode == VK_SHARING_MODE_CONCURRENT)) {
            skip |= LogPerformanceWarning(
                device, kVUID_BestPractices_vkImage_AvoidConcurrentRenderTargets,
                "%s Performance warning: image (%s) is created as a render target with VK_SHARING_MODE_CONCURRENT. "
                "Using a SHARING_MODE_CONCURRENT "
                "is not recommended with color and depth targets",
                VendorSpecificTag(kBPVendorAMD), image_hex.str().c_str());
        }

        if ((pCreateInfo->usage &
             (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) &&
            (pCreateInfo->flags & VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT)) {
            skip |= LogPerformanceWarning(
                device, kVUID_BestPractices_vkImage_DontUseMutableRenderTargets,
                "%s Performance warning: image (%s) is created as a render target with VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT. "
                "Using a MUTABLE_FORMAT is not recommended with color, depth, and storage targets",
                VendorSpecificTag(kBPVendorAMD), image_hex.str().c_str());
        }

        if ((pCreateInfo->usage & (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) &&
            (pCreateInfo->usage & VK_IMAGE_USAGE_STORAGE_BIT)) {
            skip |= LogPerformanceWarning(
                device, kVUID_BestPractices_vkImage_DontUseStorageRenderTargets,
                "%s Performance warning: image (%s) is created as a render target with VK_IMAGE_USAGE_STORAGE_BIT. Using a "
                "VK_IMAGE_USAGE_STORAGE_BIT is not recommended with color and depth targets",
                VendorSpecificTag(kBPVendorAMD), image_hex.str().c_str());
        }
    }

    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        std::stringstream image_hex;
        image_hex << "0x" << std::hex << HandleToUint64(pImage);

        if (pCreateInfo->tiling == VK_IMAGE_TILING_LINEAR) {
            skip |= LogPerformanceWarning(device, kVUID_BestPractices_CreateImage_TilingLinear,
                                          "%s Performance warning: image (%s) is created with tiling VK_IMAGE_TILING_LINEAR. "
                                          "Use VK_IMAGE_TILING_OPTIMAL instead.",
                                          VendorSpecificTag(kBPVendorNVIDIA), image_hex.str().c_str());
        }

        if (pCreateInfo->format == VK_FORMAT_D32_SFLOAT || pCreateInfo->format == VK_FORMAT_D32_SFLOAT_S8_UINT) {
            skip |= LogPerformanceWarning(
                device, kVUID_BestPractices_CreateImage_Depth32Format,
                "%s Performance warning: image (%s) is created with a 32-bit depth format. Use VK_FORMAT_D24_UNORM_S8_UINT or "
                "VK_FORMAT_D16_UNORM instead, unless the extra precision is needed.",
                VendorSpecificTag(kBPVendorNVIDIA), image_hex.str().c_str());
        }
    }

    return skip;
}

bool BestPractices::PreCallValidateCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain) const {
    bool skip = false;

    const auto* bp_pd_state = GetPhysicalDeviceState();
    if (bp_pd_state) {
        if (bp_pd_state->vkGetPhysicalDeviceSurfaceCapabilitiesKHRState == UNCALLED) {
            skip |= LogWarning(device, kVUID_BestPractices_Swapchain_GetSurfaceNotCalled,
                               "vkCreateSwapchainKHR() called before getting surface capabilities from "
                               "vkGetPhysicalDeviceSurfaceCapabilitiesKHR().");
        }

        if ((pCreateInfo->presentMode != VK_PRESENT_MODE_FIFO_KHR) &&
            (bp_pd_state->vkGetPhysicalDeviceSurfacePresentModesKHRState != QUERY_DETAILS)) {
            skip |= LogWarning(device, kVUID_BestPractices_Swapchain_GetSurfaceNotCalled,
                               "vkCreateSwapchainKHR() called before getting surface present mode(s) from "
                               "vkGetPhysicalDeviceSurfacePresentModesKHR().");
        }

        if (bp_pd_state->vkGetPhysicalDeviceSurfaceFormatsKHRState != QUERY_DETAILS) {
            skip |= LogWarning(
                device, kVUID_BestPractices_Swapchain_GetSurfaceNotCalled,
                "vkCreateSwapchainKHR() called before getting surface format(s) from vkGetPhysicalDeviceSurfaceFormatsKHR().");
        }
    }

    if ((pCreateInfo->queueFamilyIndexCount > 1) && (pCreateInfo->imageSharingMode == VK_SHARING_MODE_EXCLUSIVE)) {
        skip |=
            LogWarning(device, kVUID_BestPractices_SharingModeExclusive,
                       "Warning: A Swapchain is being created which specifies a sharing mode of VK_SHARING_MODE_EXCLUSIVE while "
                       "specifying multiple queues (queueFamilyIndexCount of %" PRIu32 ").",
                       pCreateInfo->queueFamilyIndexCount);
    }

    const auto present_mode = pCreateInfo->presentMode;
    if (((present_mode == VK_PRESENT_MODE_MAILBOX_KHR) || (present_mode == VK_PRESENT_MODE_FIFO_KHR)) &&
        (pCreateInfo->minImageCount == 2)) {
        skip |= LogPerformanceWarning(
            device, kVUID_BestPractices_SuboptimalSwapchainImageCount,
            "Warning: A Swapchain is being created with minImageCount set to %" PRIu32
            ", which means double buffering is going "
            "to be used. Using double buffering and vsync locks rendering to an integer fraction of the vsync rate. In turn, "
            "reducing the performance of the application if rendering is slower than vsync. Consider setting minImageCount to "
            "3 to use triple buffering to maximize performance in such cases.",
            pCreateInfo->minImageCount);
    }

    if (VendorCheckEnabled(kBPVendorArm) && (pCreateInfo->presentMode != VK_PRESENT_MODE_FIFO_KHR)) {
        skip |= LogWarning(device, kVUID_BestPractices_CreateSwapchain_PresentMode,
                           "%s Warning: Swapchain is not being created with presentation mode \"VK_PRESENT_MODE_FIFO_KHR\". "
                           "Prefer using \"VK_PRESENT_MODE_FIFO_KHR\" to avoid unnecessary CPU and GPU load and save power. "
                           "Presentation modes which are not FIFO will present the latest available frame and discard other "
                           "frame(s) if any.",
                           VendorSpecificTag(kBPVendorArm));
    }

    return skip;
}

bool BestPractices::PreCallValidateCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount,
                                                             const VkSwapchainCreateInfoKHR* pCreateInfos,
                                                             const VkAllocationCallbacks* pAllocator,
                                                             VkSwapchainKHR* pSwapchains) const {
    bool skip = false;

    for (uint32_t i = 0; i < swapchainCount; i++) {
        if ((pCreateInfos[i].queueFamilyIndexCount > 1) && (pCreateInfos[i].imageSharingMode == VK_SHARING_MODE_EXCLUSIVE)) {
            skip |= LogWarning(
                device, kVUID_BestPractices_SharingModeExclusive,
                "Warning: A shared swapchain (index %" PRIu32
                ") is being created which specifies a sharing mode of VK_SHARING_MODE_EXCLUSIVE while specifying multiple "
                "queues (queueFamilyIndexCount of %" PRIu32 ").",
                i, pCreateInfos[i].queueFamilyIndexCount);
        }
    }

    return skip;
}

bool BestPractices::PreCallValidateCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass) const {
    bool skip = false;

    for (uint32_t i = 0; i < pCreateInfo->attachmentCount; ++i) {
        VkFormat format = pCreateInfo->pAttachments[i].format;
        if (pCreateInfo->pAttachments[i].initialLayout == VK_IMAGE_LAYOUT_UNDEFINED) {
            if ((FormatIsColor(format) || FormatHasDepth(format)) &&
                pCreateInfo->pAttachments[i].loadOp == VK_ATTACHMENT_LOAD_OP_LOAD) {
                skip |= LogWarning(device, kVUID_BestPractices_RenderPass_Attatchment,
                                   "Render pass has an attachment with loadOp == VK_ATTACHMENT_LOAD_OP_LOAD and "
                                   "initialLayout == VK_IMAGE_LAYOUT_UNDEFINED.  This is probably not what you "
                                   "intended.  Consider using VK_ATTACHMENT_LOAD_OP_DONT_CARE instead if the "
                                   "image truely is undefined at the start of the render pass.");
            }
            if (FormatHasStencil(format) && pCreateInfo->pAttachments[i].stencilLoadOp == VK_ATTACHMENT_LOAD_OP_LOAD) {
                skip |= LogWarning(device, kVUID_BestPractices_RenderPass_Attatchment,
                                   "Render pass has an attachment with stencilLoadOp == VK_ATTACHMENT_LOAD_OP_LOAD "
                                   "and initialLayout == VK_IMAGE_LAYOUT_UNDEFINED.  This is probably not what you "
                                   "intended.  Consider using VK_ATTACHMENT_LOAD_OP_DONT_CARE instead if the "
                                   "image truely is undefined at the start of the render pass.");
            }
        }

        const auto& attachment = pCreateInfo->pAttachments[i];
        if (attachment.samples > VK_SAMPLE_COUNT_1_BIT) {
            bool access_requires_memory =
                attachment.loadOp == VK_ATTACHMENT_LOAD_OP_LOAD || attachment.storeOp == VK_ATTACHMENT_STORE_OP_STORE;

            if (FormatHasStencil(format)) {
                access_requires_memory |= attachment.stencilLoadOp == VK_ATTACHMENT_LOAD_OP_LOAD ||
                                          attachment.stencilStoreOp == VK_ATTACHMENT_STORE_OP_STORE;
            }

            if (access_requires_memory) {
                skip |= LogPerformanceWarning(
                    device, kVUID_BestPractices_CreateRenderPass_ImageRequiresMemory,
                    "Attachment %u in the VkRenderPass is a multisampled image with %u samples, but it uses loadOp/storeOp "
                    "which requires accessing data from memory. Multisampled images should always be loadOp = CLEAR or DONT_CARE, "
                    "storeOp = DONT_CARE. This allows the implementation to use lazily allocated memory effectively.",
                    i, static_cast<uint32_t>(attachment.samples));
            }
        }
    }

    for (uint32_t dependency = 0; dependency < pCreateInfo->dependencyCount; dependency++) {
        skip |= CheckPipelineStageFlags("vkCreateRenderPass", pCreateInfo->pDependencies[dependency].srcStageMask);
        skip |= CheckPipelineStageFlags("vkCreateRenderPass", pCreateInfo->pDependencies[dependency].dstStageMask);
    }

    return skip;
}

bool BestPractices::ValidateAttachments(const VkRenderPassCreateInfo2* rpci, uint32_t attachmentCount,
                                        const VkImageView* image_views) const {
    bool skip = false;

    // Check for non-transient attachments that should be transient and vice versa
    for (uint32_t i = 0; i < attachmentCount; ++i) {
        const auto& attachment = rpci->pAttachments[i];
        bool attachment_should_be_transient =
            (attachment.loadOp != VK_ATTACHMENT_LOAD_OP_LOAD && attachment.storeOp != VK_ATTACHMENT_STORE_OP_STORE);

        if (FormatHasStencil(attachment.format)) {
            attachment_should_be_transient &= (attachment.stencilLoadOp != VK_ATTACHMENT_LOAD_OP_LOAD &&
                                               attachment.stencilStoreOp != VK_ATTACHMENT_STORE_OP_STORE);
        }

        auto view_state = Get<IMAGE_VIEW_STATE>(image_views[i]);
        if (view_state) {
            const auto& ici = view_state->image_state->createInfo;

            const bool image_is_transient = (ici.usage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT) != 0;

            // The check for an image that should not be transient applies to all GPUs
            if (!attachment_should_be_transient && image_is_transient) {
                skip |= LogPerformanceWarning(
                    device, kVUID_BestPractices_CreateFramebuffer_AttachmentShouldNotBeTransient,
                    "Attachment %u in VkFramebuffer uses loadOp/storeOps which need to access physical memory, "
                    "but the image backing the image view has VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT set. "
                    "Physical memory will need to be backed lazily to this image, potentially causing stalls.",
                    i);
            }

            bool supports_lazy = false;
            for (uint32_t j = 0; j < phys_dev_mem_props.memoryTypeCount; j++) {
                if (phys_dev_mem_props.memoryTypes[j].propertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) {
                    supports_lazy = true;
                }
            }

            // The check for an image that should be transient only applies to GPUs supporting
            // lazily allocated memory
            if (supports_lazy && attachment_should_be_transient && !image_is_transient) {
                skip |= LogPerformanceWarning(
                    device, kVUID_BestPractices_CreateFramebuffer_AttachmentShouldBeTransient,
                    "Attachment %u in VkFramebuffer uses loadOp/storeOps which never have to be backed by physical memory, "
                    "but the image backing the image view does not have VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT set. "
                    "You can save physical memory by using transient attachment backed by lazily allocated memory here.",
                    i);
            }
        }
    }
    return skip;
}

bool BestPractices::PreCallValidateCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer) const {
    bool skip = false;

    auto rp_state = Get<RENDER_PASS_STATE>(pCreateInfo->renderPass);
    if (rp_state && !(pCreateInfo->flags & VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT)) {
        skip = ValidateAttachments(rp_state->createInfo.ptr(), pCreateInfo->attachmentCount, pCreateInfo->pAttachments);
    }

    return skip;
}

bool BestPractices::PreCallValidateAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo,
                                                          VkDescriptorSet* pDescriptorSets, void* ads_state_data) const {
    bool skip = false;
    skip |= ValidationStateTracker::PreCallValidateAllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets, ads_state_data);

    if (!skip) {
        const auto pool_state = Get<bp_state::DescriptorPool>(pAllocateInfo->descriptorPool);
        // if the number of freed sets > 0, it implies they could be recycled instead if desirable
        // this warning is specific to Arm
        if (VendorCheckEnabled(kBPVendorArm) && pool_state && (pool_state->freed_count > 0)) {
            skip |= LogPerformanceWarning(
                device, kVUID_BestPractices_AllocateDescriptorSets_SuboptimalReuse,
                "%s Descriptor set memory was allocated via vkAllocateDescriptorSets() for sets which were previously freed in the "
                "same logical device. On some drivers or architectures it may be most optimal to re-use existing descriptor sets.",
                VendorSpecificTag(kBPVendorArm));
        }

        if (IsExtEnabled(device_extensions.vk_khr_maintenance1)) {
            // Track number of descriptorSets allowable in this pool
            if (pool_state->GetAvailableSets() < pAllocateInfo->descriptorSetCount) {
                skip |= LogWarning(pool_state->Handle(), kVUID_BestPractices_EmptyDescriptorPool,
                                   "vkAllocateDescriptorSets(): Unable to allocate %" PRIu32
                                   " descriptorSets from %s"
                                   ". This pool only has %" PRIu32 " descriptorSets remaining.",
                                   pAllocateInfo->descriptorSetCount, report_data->FormatHandle(pool_state->Handle()).c_str(),
                                   pool_state->GetAvailableSets());
            }

            for (uint32_t i = 0; i < pAllocateInfo->descriptorSetCount; i++) {
                auto layout = Get<cvdescriptorset::DescriptorSetLayout>(pAllocateInfo->pSetLayouts[i]);
                if (layout) {  // if null, this is validated/logged in object_tracker
                    const uint32_t binding_count = layout->GetBindingCount();
                    for (uint32_t j = 0; j < binding_count; ++j) {
                        const VkDescriptorType type = layout->GetTypeFromIndex(j);
                        if (!pool_state->IsAvailableType(type)) {
                            // This check would be caught by validation if VK_KHR_maintenance1 was not enabled
                            skip |= LogWarning(pool_state->Handle(), kVUID_BestPractices_DescriptorTypeNotInPool,
                                               "vkAllocateDescriptorSets(): pSetLayouts[%" PRIu32 "] binding %" PRIu32
                                               " was created with %s but the "
                                               "Descriptor Pool was not created with this type",
                                               i, j, string_VkDescriptorType(type));
                        }
                    }
                }
            }
        }
    }

    return skip;
}

void BestPractices::ManualPostCallRecordAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo,
                                                               VkDescriptorSet* pDescriptorSets, VkResult result, void* ads_state) {
    if (result == VK_SUCCESS) {
        auto pool_state = Get<bp_state::DescriptorPool>(pAllocateInfo->descriptorPool);
        if (pool_state) {
            // we record successful allocations by subtracting the allocation count from the last recorded free count
            const auto alloc_count = pAllocateInfo->descriptorSetCount;
            // clamp the unsigned subtraction to the range [0, last_free_count]
            if (pool_state->freed_count > alloc_count) {
                pool_state->freed_count -= alloc_count;
            } else {
                pool_state->freed_count = 0;
            }
        }
    }
}

void BestPractices::PostCallRecordFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t descriptorSetCount,
                                                     const VkDescriptorSet* pDescriptorSets, VkResult result) {
    ValidationStateTracker::PostCallRecordFreeDescriptorSets(device, descriptorPool, descriptorSetCount, pDescriptorSets, result);
    if (result == VK_SUCCESS) {
        auto pool_state = Get<bp_state::DescriptorPool>(descriptorPool);
        // we want to track frees because we're interested in suggesting re-use
        if (pool_state) {
            pool_state->freed_count += descriptorSetCount;
        }
    }
}

void BestPractices::PreCallRecordAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory) {
    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        WriteLockGuard guard{memory_free_events_lock_};

        // Release old allocations to avoid overpopulating the container
        const auto now = std::chrono::high_resolution_clock::now();
        const auto last_old = std::find_if(
            memory_free_events_.rbegin(), memory_free_events_.rend(),
            [now](const MemoryFreeEvent& event) { return now - event.time > kAllocateMemoryReuseTimeThresholdNVIDIA; });
        memory_free_events_.erase(memory_free_events_.begin(), last_old.base());
    }
}

bool BestPractices::PreCallValidateAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory) const {
    bool skip = false;

    if ((Count<DEVICE_MEMORY_STATE>() + 1) > kMemoryObjectWarningLimit) {
        skip |= LogPerformanceWarning(device, kVUID_BestPractices_AllocateMemory_TooManyObjects,
                                      "Performance Warning: This app has > %" PRIu32 " memory objects.", kMemoryObjectWarningLimit);
    }

    if (pAllocateInfo->allocationSize < kMinDeviceAllocationSize) {
        skip |= LogPerformanceWarning(device, kVUID_BestPractices_AllocateMemory_SmallAllocation,
                                      "vkAllocateMemory(): Allocating a VkDeviceMemory of size %" PRIu64
                                      ". This is a very small allocation (current "
                                      "threshold is %" PRIu64
                                      " bytes). "
                                      "You should make large allocations and sub-allocate from one large VkDeviceMemory.",
                                      pAllocateInfo->allocationSize, kMinDeviceAllocationSize);
    }

    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        if (!IsExtEnabled(device_extensions.vk_ext_pageable_device_local_memory) &&
            !LvlFindInChain<VkMemoryPriorityAllocateInfoEXT>(pAllocateInfo->pNext)) {
            skip |= LogPerformanceWarning(
                device, kVUID_BestPractices_AllocateMemory_SetPriority,
                "%s Use VkMemoryPriorityAllocateInfoEXT to provide the operating system information on the allocations that "
                "should stay in video memory and which should be demoted first when video memory is limited. "
                "The highest priority should be given to GPU-written resources like color attachments, depth attachments, "
                "storage images, and buffers written from the GPU.",
                VendorSpecificTag(kBPVendorNVIDIA));
        }

        {
            // Size in bytes for an allocation to be considered "compatible"
            static constexpr VkDeviceSize size_threshold = VkDeviceSize{1} << 20;

            ReadLockGuard guard{memory_free_events_lock_};

            const auto now = std::chrono::high_resolution_clock::now();
            const VkDeviceSize alloc_size = pAllocateInfo->allocationSize;
            const uint32_t memory_type_index = pAllocateInfo->memoryTypeIndex;
            const auto latest_event =
                std::find_if(memory_free_events_.rbegin(), memory_free_events_.rend(), [&](const MemoryFreeEvent& event) {
                    return (memory_type_index == event.memory_type_index) && (alloc_size <= event.allocation_size) &&
                           (alloc_size - event.allocation_size <= size_threshold) &&
                           (now - event.time < kAllocateMemoryReuseTimeThresholdNVIDIA);
                });

            if (latest_event != memory_free_events_.rend()) {
                const auto time_delta = std::chrono::duration_cast<std::chrono::milliseconds>(now - latest_event->time);
                if (time_delta < std::chrono::milliseconds{5}) {
                    skip |= LogPerformanceWarning(
                        device, kVUID_BestPractices_AllocateMemory_ReuseAllocations,
                        "%s Reuse memory allocations instead of releasing and reallocating. A memory allocation "
                        "has just been released, and it could have been reused in place of this allocation.",
                        VendorSpecificTag(kBPVendorNVIDIA));
                } else {
                    const uint32_t seconds = static_cast<uint32_t>(time_delta.count() / 1000);
                    const uint32_t milliseconds = static_cast<uint32_t>(time_delta.count() % 1000);

                    skip |= LogPerformanceWarning(
                        device, kVUID_BestPractices_AllocateMemory_ReuseAllocations,
                        "%s Reuse memory allocations instead of releasing and reallocating. A memory allocation has been released "
                        "%" PRIu32 ".%03" PRIu32 " seconds ago, and it could have been reused in place of this allocation.",
                        VendorSpecificTag(kBPVendorNVIDIA), seconds, milliseconds);
                }
            }
        }
    }

    // TODO: Insert get check for GetPhysicalDeviceMemoryProperties once the state is tracked in the StateTracker

    return skip;
}

void BestPractices::ManualPostCallRecordAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo,
                                                       const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory,
                                                       VkResult result) {
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY, VK_ERROR_OUT_OF_DEVICE_MEMORY, VK_ERROR_TOO_MANY_OBJECTS,
                                            VK_ERROR_INVALID_EXTERNAL_HANDLE, VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS};
        ValidateReturnCodes("vkAllocateMemory", result, error_codes, {});
        return;
    }
}

void BestPractices::ValidateReturnCodes(const char* api_name, VkResult result, vvl::span<const VkResult> error_codes,
                                        vvl::span<const VkResult> success_codes) const {
    auto error = std::find(error_codes.begin(), error_codes.end(), result);
    if (error != error_codes.end()) {
        constexpr std::array common_failure_codes = {VK_ERROR_OUT_OF_DATE_KHR, VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT};

        auto common_failure = std::find(common_failure_codes.begin(), common_failure_codes.end(), result);
        if (common_failure != common_failure_codes.end()) {
            LogInfo(instance, kVUID_BestPractices_Failure_Result, "%s(): Returned error %s.", api_name, string_VkResult(result));
        } else {
            LogWarning(instance, kVUID_BestPractices_Error_Result, "%s(): Returned error %s.", api_name, string_VkResult(result));
        }
        return;
    }
    auto success = std::find(success_codes.begin(), success_codes.end(), result);
    if (success != success_codes.end()) {
        LogInfo(instance, kVUID_BestPractices_NonSuccess_Result, "%s(): Returned non-success return code %s.", api_name,
                string_VkResult(result));
    }
}

void BestPractices::PreCallRecordFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator) {
    if (memory != VK_NULL_HANDLE && VendorCheckEnabled(kBPVendorNVIDIA)) {
        auto mem_info = Get<DEVICE_MEMORY_STATE>(memory);

        // Exclude memory free events on dedicated allocations, or imported/exported allocations.
        if (!mem_info->IsDedicatedBuffer() && !mem_info->IsDedicatedImage() && !mem_info->IsExport() && !mem_info->IsImport()) {
            MemoryFreeEvent event;
            event.time = std::chrono::high_resolution_clock::now();
            event.memory_type_index = mem_info->alloc_info.memoryTypeIndex;
            event.allocation_size = mem_info->alloc_info.allocationSize;

            WriteLockGuard guard{memory_free_events_lock_};
            memory_free_events_.push_back(event);
        }
    }

    ValidationStateTracker::PreCallRecordFreeMemory(device, memory, pAllocator);
}

bool BestPractices::PreCallValidateFreeMemory(VkDevice device, VkDeviceMemory memory,
                                              const VkAllocationCallbacks* pAllocator) const {
    if (memory == VK_NULL_HANDLE) return false;
    bool skip = false;

    auto mem_info = Get<DEVICE_MEMORY_STATE>(memory);

    for (const auto& item : mem_info->ObjectBindings()) {
        const auto& obj = item.first;
        const LogObjectList objlist(device, obj, mem_info->mem());
        skip |= LogWarning(objlist, layer_name.c_str(), "VK Object %s still has a reference to mem obj %s.",
                           report_data->FormatHandle(obj).c_str(), report_data->FormatHandle(mem_info->mem()).c_str());
    }

    return skip;
}

bool BestPractices::ValidateBindBufferMemory(VkBuffer buffer, VkDeviceMemory memory, const char* api_name) const {
    bool skip = false;
    auto buffer_state = Get<BUFFER_STATE>(buffer);

    if (!buffer_state->memory_requirements_checked && !buffer_state->external_memory_handle) {
        skip |= LogWarning(device, kVUID_BestPractices_BufferMemReqNotCalled,
                           "%s: Binding memory to %s but vkGetBufferMemoryRequirements() has not been called on that buffer.",
                           api_name, report_data->FormatHandle(buffer).c_str());
    }

    auto mem_state = Get<DEVICE_MEMORY_STATE>(memory);

    if (mem_state && mem_state->alloc_info.allocationSize == buffer_state->createInfo.size &&
        mem_state->alloc_info.allocationSize < kMinDedicatedAllocationSize) {
        skip |= LogPerformanceWarning(device, kVUID_BestPractices_SmallDedicatedAllocation,
                                      "%s: Trying to bind %s to a memory block which is fully consumed by the buffer. "
                                      "The required size of the allocation is %" PRIu64
                                      ", but smaller buffers like this should be sub-allocated from "
                                      "larger memory blocks. (Current threshold is %" PRIu64 " bytes.)",
                                      api_name, report_data->FormatHandle(buffer).c_str(), mem_state->alloc_info.allocationSize,
                                      kMinDedicatedAllocationSize);
    }

    skip |= ValidateBindMemory(device, memory);

    return skip;
}

bool BestPractices::PreCallValidateBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory,
                                                    VkDeviceSize memoryOffset) const {
    bool skip = false;
    const char* api_name = "BindBufferMemory()";

    skip |= ValidateBindBufferMemory(buffer, memory, api_name);

    return skip;
}

bool BestPractices::PreCallValidateBindBufferMemory2(VkDevice device, uint32_t bindInfoCount,
                                                     const VkBindBufferMemoryInfo* pBindInfos) const {
    char api_name[64];
    bool skip = false;

    for (uint32_t i = 0; i < bindInfoCount; i++) {
        snprintf(api_name, sizeof(api_name), "vkBindBufferMemory2() pBindInfos[%u]", i);
        skip |= ValidateBindBufferMemory(pBindInfos[i].buffer, pBindInfos[i].memory, api_name);
    }

    return skip;
}

bool BestPractices::PreCallValidateBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                                        const VkBindBufferMemoryInfo* pBindInfos) const {
    char api_name[64];
    bool skip = false;

    for (uint32_t i = 0; i < bindInfoCount; i++) {
        snprintf(api_name, sizeof(api_name), "vkBindBufferMemory2KHR() pBindInfos[%u]", i);
        skip |= ValidateBindBufferMemory(pBindInfos[i].buffer, pBindInfos[i].memory, api_name);
    }

    return skip;
}

bool BestPractices::ValidateBindImageMemory(VkImage image, VkDeviceMemory memory, const char* api_name) const {
    bool skip = false;
    auto image_state = Get<IMAGE_STATE>(image);

    if (image_state->disjoint == false) {
        if (!image_state->memory_requirements_checked[0] && !image_state->external_memory_handle) {
            skip |= LogWarning(device, kVUID_BestPractices_ImageMemReqNotCalled,
                               "%s: Binding memory to %s but vkGetImageMemoryRequirements() has not been called on that image.",
                               api_name, report_data->FormatHandle(image).c_str());
        }
    } else {
        // TODO If binding disjoint image then this needs to check that VkImagePlaneMemoryRequirementsInfo was called for each
        // plane.
    }

    auto mem_state = Get<DEVICE_MEMORY_STATE>(memory);

    if (mem_state->alloc_info.allocationSize == image_state->requirements[0].size &&
        mem_state->alloc_info.allocationSize < kMinDedicatedAllocationSize) {
        skip |= LogPerformanceWarning(device, kVUID_BestPractices_SmallDedicatedAllocation,
                                      "%s: Trying to bind %s to a memory block which is fully consumed by the image. "
                                      "The required size of the allocation is %" PRIu64
                                      ", but smaller images like this should be sub-allocated from "
                                      "larger memory blocks. (Current threshold is %" PRIu64 " bytes.)",
                                      api_name, report_data->FormatHandle(image).c_str(), mem_state->alloc_info.allocationSize,
                                      kMinDedicatedAllocationSize);
    }

    // If we're binding memory to a image which was created as TRANSIENT and the image supports LAZY allocation,
    // make sure this type is actually used.
    // This warning will only trigger if this layer is run on a platform that supports LAZILY_ALLOCATED_BIT
    // (i.e.most tile - based renderers)
    if (image_state->createInfo.usage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT) {
        bool supports_lazy = false;
        uint32_t suggested_type = 0;

        for (uint32_t i = 0; i < phys_dev_mem_props.memoryTypeCount; i++) {
            if ((1u << i) & image_state->requirements[0].memoryTypeBits) {
                if (phys_dev_mem_props.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) {
                    supports_lazy = true;
                    suggested_type = i;
                    break;
                }
            }
        }

        uint32_t allocated_properties = phys_dev_mem_props.memoryTypes[mem_state->alloc_info.memoryTypeIndex].propertyFlags;

        if (supports_lazy && (allocated_properties & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) == 0) {
            skip |= LogPerformanceWarning(
                device, kVUID_BestPractices_NonLazyTransientImage,
                "%s: Attempting to bind memory type %u to VkImage which was created with TRANSIENT_ATTACHMENT_BIT,"
                "but this memory type is not LAZILY_ALLOCATED_BIT. You should use memory type %u here instead to save "
                "%" PRIu64 " bytes of physical memory.",
                api_name, mem_state->alloc_info.memoryTypeIndex, suggested_type, image_state->requirements[0].size);
        }
    }

    skip |= ValidateBindMemory(device, memory);

    return skip;
}

bool BestPractices::PreCallValidateBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory,
                                                   VkDeviceSize memoryOffset) const {
    bool skip = false;
    const char* api_name = "vkBindImageMemory()";

    skip |= ValidateBindImageMemory(image, memory, api_name);

    return skip;
}

bool BestPractices::PreCallValidateBindImageMemory2(VkDevice device, uint32_t bindInfoCount,
                                                    const VkBindImageMemoryInfo* pBindInfos) const {
    char api_name[64];
    bool skip = false;

    for (uint32_t i = 0; i < bindInfoCount; i++) {
        snprintf(api_name, sizeof(api_name), "vkBindImageMemory2() pBindInfos[%u]", i);
        if (!LvlFindInChain<VkBindImageMemorySwapchainInfoKHR>(pBindInfos[i].pNext)) {
            skip |= ValidateBindImageMemory(pBindInfos[i].image, pBindInfos[i].memory, api_name);
        }
    }

    return skip;
}

bool BestPractices::PreCallValidateBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                                       const VkBindImageMemoryInfo* pBindInfos) const {
    char api_name[64];
    bool skip = false;

    for (uint32_t i = 0; i < bindInfoCount; i++) {
        snprintf(api_name, sizeof(api_name), "vkBindImageMemory2KHR() pBindInfos[%u]", i);
        skip |= ValidateBindImageMemory(pBindInfos[i].image, pBindInfos[i].memory, api_name);
    }

    return skip;
}

void BestPractices::PreCallRecordSetDeviceMemoryPriorityEXT(VkDevice device, VkDeviceMemory memory, float priority) {
    auto mem_info = std::static_pointer_cast<bp_state::DeviceMemory>(Get<DEVICE_MEMORY_STATE>(memory));
    mem_info->dynamic_priority.emplace(priority);
}

bool BestPractices::PreCallValidateGetVideoSessionMemoryRequirementsKHR(
    VkDevice device, VkVideoSessionKHR videoSession, uint32_t* pMemoryRequirementsCount,
    VkVideoSessionMemoryRequirementsKHR* pMemoryRequirements) const {
    bool skip = false;

    auto vs_state = Get<VIDEO_SESSION_STATE>(videoSession);
    if (vs_state) {
        if (pMemoryRequirements != nullptr && !vs_state->memory_binding_count_queried) {
            skip |= LogWarning(videoSession, kVUID_BestPractices_GetVideoSessionMemReqCountNotRetrieved,
                               "vkGetVideoSessionMemoryRequirementsKHR(): querying list of memory requirements of %s "
                               "but the number of memory requirements has not been queried before by calling this "
                               "command with pMemoryRequirements set to NULL.",
                               report_data->FormatHandle(videoSession).c_str());
        }
    }

    return skip;
}

bool BestPractices::PreCallValidateBindVideoSessionMemoryKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                             uint32_t bindSessionMemoryInfoCount,
                                                             const VkBindVideoSessionMemoryInfoKHR* pBindSessionMemoryInfos) const {
    bool skip = false;

    auto vs_state = Get<VIDEO_SESSION_STATE>(videoSession);
    if (vs_state) {
        if (!vs_state->memory_binding_count_queried) {
            skip |= LogWarning(videoSession, kVUID_BestPractices_BindVideoSessionMemReqCountNotRetrieved,
                               "vkBindVideoSessionMemoryKHR(): binding memory to %s but "
                               "vkGetVideoSessionMemoryRequirementsKHR() has not been called to retrieve the "
                               "number of memory requirements for the video session.",
                               report_data->FormatHandle(videoSession).c_str());
        } else if (vs_state->memory_bindings_queried < vs_state->GetMemoryBindingCount()) {
            skip |= LogWarning(videoSession, kVUID_BestPractices_BindVideoSessionMemReqNotAllBindingsRetrieved,
                               "vkBindVideoSessionMemoryKHR(): binding memory to %s but "
                               "not all memory requirements for the video session have been queried using "
                               "vkGetVideoSessionMemoryRequirementsKHR().",
                               report_data->FormatHandle(videoSession).c_str());
        }
    }

    return skip;
}

static inline bool FormatHasFullThroughputBlendingArm(VkFormat format) {
    switch (format) {
        case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
        case VK_FORMAT_R16_SFLOAT:
        case VK_FORMAT_R16G16_SFLOAT:
        case VK_FORMAT_R16G16B16_SFLOAT:
        case VK_FORMAT_R16G16B16A16_SFLOAT:
        case VK_FORMAT_R32_SFLOAT:
        case VK_FORMAT_R32G32_SFLOAT:
        case VK_FORMAT_R32G32B32_SFLOAT:
        case VK_FORMAT_R32G32B32A32_SFLOAT:
            return false;

        default:
            return true;
    }
}

bool BestPractices::ValidateMultisampledBlendingArm(uint32_t createInfoCount,
                                                    const VkGraphicsPipelineCreateInfo* pCreateInfos) const {
    bool skip = false;

    for (uint32_t i = 0; i < createInfoCount; i++) {
        auto create_info = &pCreateInfos[i];

        if (!create_info->pColorBlendState || !create_info->pMultisampleState ||
            create_info->pMultisampleState->rasterizationSamples == VK_SAMPLE_COUNT_1_BIT ||
            create_info->pMultisampleState->sampleShadingEnable) {
            return skip;
        }

        auto rp_state = Get<RENDER_PASS_STATE>(create_info->renderPass);
        const auto& subpass = rp_state->createInfo.pSubpasses[create_info->subpass];

        // According to spec, pColorBlendState must be ignored if subpass does not have color attachments.
        uint32_t num_color_attachments = std::min(subpass.colorAttachmentCount, create_info->pColorBlendState->attachmentCount);

        for (uint32_t j = 0; j < num_color_attachments; j++) {
            const auto& blend_att = create_info->pColorBlendState->pAttachments[j];
            uint32_t att = subpass.pColorAttachments[j].attachment;

            if (att != VK_ATTACHMENT_UNUSED && blend_att.blendEnable && blend_att.colorWriteMask) {
                if (!FormatHasFullThroughputBlendingArm(rp_state->createInfo.pAttachments[att].format)) {
                    skip |= LogPerformanceWarning(device, kVUID_BestPractices_CreatePipelines_MultisampledBlending,
                                                  "%s vkCreateGraphicsPipelines() - createInfo #%u: Pipeline is multisampled and "
                                                  "color attachment #%u makes use "
                                                  "of a format which cannot be blended at full throughput when using MSAA.",
                                                  VendorSpecificTag(kBPVendorArm), i, j);
                }
            }
        }
    }

    return skip;
}

void BestPractices::ManualPostCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache,
                                                               uint32_t createInfoCount,
                                                               const VkComputePipelineCreateInfo* pCreateInfos,
                                                               const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                               VkResult result, void* pipe_state) {
    // AMD best practice
    pipeline_cache_ = pipelineCache;
}

bool BestPractices::PreCallValidateCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                           const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                                           const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                           void* cgpl_state_data) const {
    bool skip = StateTracker::PreCallValidateCreateGraphicsPipelines(device, pipelineCache, createInfoCount, pCreateInfos,
                                                                     pAllocator, pPipelines, cgpl_state_data);
    if (skip) {
        return skip;
    }
    create_graphics_pipeline_api_state* cgpl_state = reinterpret_cast<create_graphics_pipeline_api_state*>(cgpl_state_data);

    if ((createInfoCount > 1) && (!pipelineCache)) {
        skip |= LogPerformanceWarning(
            device, kVUID_BestPractices_CreatePipelines_MultiplePipelines,
            "Performance Warning: This vkCreateGraphicsPipelines call is creating multiple pipelines but is not using a "
            "pipeline cache, which may help with performance");
    }

    for (uint32_t i = 0; i < createInfoCount; i++) {
        const auto& create_info = pCreateInfos[i];

        if (!(cgpl_state->pipe_state[i]->active_shaders & VK_SHADER_STAGE_MESH_BIT_NV) && create_info.pVertexInputState) {
            const auto& vertex_input = *create_info.pVertexInputState;
            uint32_t count = 0;
            for (uint32_t j = 0; j < vertex_input.vertexBindingDescriptionCount; j++) {
                if (vertex_input.pVertexBindingDescriptions[j].inputRate == VK_VERTEX_INPUT_RATE_INSTANCE) {
                    count++;
                }
            }
            if (count > kMaxInstancedVertexBuffers) {
                skip |= LogPerformanceWarning(
                    device, kVUID_BestPractices_CreatePipelines_TooManyInstancedVertexBuffers,
                    "The pipeline is using %u instanced vertex buffers (current limit: %u), but this can be inefficient on the "
                    "GPU. If using instanced vertex attributes prefer interleaving them in a single buffer.",
                    count, kMaxInstancedVertexBuffers);
            }
        }

        if ((pCreateInfos[i].pRasterizationState) && (pCreateInfos[i].pRasterizationState->depthBiasEnable) &&
            (pCreateInfos[i].pRasterizationState->depthBiasConstantFactor == 0.0f) &&
            (pCreateInfos[i].pRasterizationState->depthBiasSlopeFactor == 0.0f) && VendorCheckEnabled(kBPVendorArm)) {
            skip |= LogPerformanceWarning(
                device, kVUID_BestPractices_CreatePipelines_DepthBias_Zero,
                "%s Performance Warning: This vkCreateGraphicsPipelines call is created with depthBiasEnable set to true "
                "and both depthBiasConstantFactor and depthBiasSlopeFactor are set to 0. This can cause reduced "
                "efficiency during rasterization. Consider disabling depthBias or increasing either "
                "depthBiasConstantFactor or depthBiasSlopeFactor.",
                VendorSpecificTag(kBPVendorArm));
        }

        skip |= VendorCheckEnabled(kBPVendorArm) && ValidateMultisampledBlendingArm(createInfoCount, pCreateInfos);
    }
    if (VendorCheckEnabled(kBPVendorAMD) || VendorCheckEnabled(kBPVendorNVIDIA)) {
        auto prev_pipeline = pipeline_cache_.load();
        if (pipelineCache && prev_pipeline && pipelineCache != prev_pipeline) {
            skip |= LogPerformanceWarning(device, kVUID_BestPractices_CreatePipelines_MultiplePipelineCaches,
                                          "%s %s Performance Warning: A second pipeline cache is in use. "
                                          "Consider using only one pipeline cache to improve cache hit rate.",
                                          VendorSpecificTag(kBPVendorAMD), VendorSpecificTag(kBPVendorNVIDIA));
        }
    }
    if (VendorCheckEnabled(kBPVendorAMD)) {
        if (num_pso_ > kMaxRecommendedNumberOfPSOAMD) {
            skip |= LogPerformanceWarning(device, kVUID_BestPractices_CreatePipelines_TooManyPipelines,
                                          "%s Performance warning: Too many pipelines created, consider consolidation",
                                          VendorSpecificTag(kBPVendorAMD));
        }

        if (pCreateInfos->pInputAssemblyState && pCreateInfos->pInputAssemblyState->primitiveRestartEnable) {
            skip |= LogPerformanceWarning(device, kVUID_BestPractices_CreatePipelines_AvoidPrimitiveRestart,
                                          "%s Performance warning: Use of primitive restart is not recommended",
                                          VendorSpecificTag(kBPVendorAMD));
        }

        // TODO: this might be too aggressive of a check
        if (pCreateInfos->pDynamicState && pCreateInfos->pDynamicState->dynamicStateCount > kDynamicStatesWarningLimitAMD) {
            skip |= LogPerformanceWarning(
                device, kVUID_BestPractices_CreatePipelines_MinimizeNumDynamicStates,
                "%s Performance warning: Dynamic States usage incurs a performance cost. Ensure that they are truly needed",
                VendorSpecificTag(kBPVendorAMD));
        }
    }

    return skip;
}

static std::vector<bp_state::AttachmentInfo> GetAttachmentAccess(const safe_VkGraphicsPipelineCreateInfo& create_info,
                                                                 std::shared_ptr<const RENDER_PASS_STATE>& rp) {
    std::vector<bp_state::AttachmentInfo> result;
    if (!rp || rp->UsesDynamicRendering()) {
        return result;
    }

    const auto& subpass = rp->createInfo.pSubpasses[create_info.subpass];

    // NOTE: see PIPELINE_LAYOUT and safe_VkGraphicsPipelineCreateInfo constructors. pColorBlendState and pDepthStencilState
    // are only non-null if they are enabled.
    if (create_info.pColorBlendState) {
        // According to spec, pColorBlendState must be ignored if subpass does not have color attachments.
        uint32_t num_color_attachments = std::min(subpass.colorAttachmentCount, create_info.pColorBlendState->attachmentCount);
        for (uint32_t j = 0; j < num_color_attachments; j++) {
            if (create_info.pColorBlendState->pAttachments[j].colorWriteMask != 0) {
                uint32_t attachment = subpass.pColorAttachments[j].attachment;
                if (attachment != VK_ATTACHMENT_UNUSED) {
                    result.push_back({attachment, VK_IMAGE_ASPECT_COLOR_BIT});
                }
            }
        }
    }

    if (create_info.pDepthStencilState &&
        (create_info.pDepthStencilState->depthTestEnable || create_info.pDepthStencilState->depthBoundsTestEnable ||
         create_info.pDepthStencilState->stencilTestEnable)) {
        uint32_t attachment = subpass.pDepthStencilAttachment ? subpass.pDepthStencilAttachment->attachment : VK_ATTACHMENT_UNUSED;
        if (attachment != VK_ATTACHMENT_UNUSED) {
            VkImageAspectFlags aspects = 0;
            if (create_info.pDepthStencilState->depthTestEnable || create_info.pDepthStencilState->depthBoundsTestEnable) {
                aspects |= VK_IMAGE_ASPECT_DEPTH_BIT;
            }
            if (create_info.pDepthStencilState->stencilTestEnable) {
                aspects |= VK_IMAGE_ASPECT_STENCIL_BIT;
            }
            result.push_back({attachment, aspects});
        }
    }
    return result;
}

bp_state::Pipeline::Pipeline(const ValidationStateTracker* state_data, const VkGraphicsPipelineCreateInfo* pCreateInfo,
                             uint32_t create_index, std::shared_ptr<const RENDER_PASS_STATE>&& rpstate,
                             std::shared_ptr<const PIPELINE_LAYOUT_STATE>&& layout, CreateShaderModuleStates* csm_states)
    : PIPELINE_STATE(state_data, pCreateInfo, create_index, std::move(rpstate), std::move(layout), csm_states),
      access_framebuffer_attachments(GetAttachmentAccess(create_info.graphics, rp_state)) {}

std::shared_ptr<PIPELINE_STATE> BestPractices::CreateGraphicsPipelineState(const VkGraphicsPipelineCreateInfo* pCreateInfo,
                                                                           uint32_t create_index,
                                                                           std::shared_ptr<const RENDER_PASS_STATE>&& render_pass,
                                                                           std::shared_ptr<const PIPELINE_LAYOUT_STATE>&& layout,
                                                                           CreateShaderModuleStates* csm_states) const {
    return std::static_pointer_cast<PIPELINE_STATE>(std::make_shared<bp_state::Pipeline>(
        this, pCreateInfo, create_index, std::move(render_pass), std::move(layout), csm_states));
}

void BestPractices::ManualPostCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                                const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                                                const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                                VkResult result, void* cgpl_state_data) {
    // AMD best practice
    pipeline_cache_ = pipelineCache;
}

bool BestPractices::PreCallValidateCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                          const VkComputePipelineCreateInfo* pCreateInfos,
                                                          const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                          void* ccpl_state_data) const {
    bool skip = StateTracker::PreCallValidateCreateComputePipelines(device, pipelineCache, createInfoCount, pCreateInfos,
                                                                    pAllocator, pPipelines, ccpl_state_data);

    if ((createInfoCount > 1) && (!pipelineCache)) {
        skip |= LogPerformanceWarning(
            device, kVUID_BestPractices_CreatePipelines_MultiplePipelines,
            "Performance Warning: This vkCreateComputePipelines call is creating multiple pipelines but is not using a "
            "pipeline cache, which may help with performance");
    }

    if (VendorCheckEnabled(kBPVendorAMD)) {
        auto prev_pipeline = pipeline_cache_.load();
        if (pipelineCache && prev_pipeline && pipelineCache != prev_pipeline) {
            skip |= LogPerformanceWarning(
                device, kVUID_BestPractices_CreatePipelines_MultiplePipelines,
                "%s Performance Warning: A second pipeline cache is in use. Consider using only one pipeline cache to "
                "improve cache hit rate",
                VendorSpecificTag(kBPVendorAMD));
        }
    }

    for (uint32_t i = 0; i < createInfoCount; i++) {
        const VkComputePipelineCreateInfo& createInfo = pCreateInfos[i];
        if (VendorCheckEnabled(kBPVendorArm)) {
            skip |= ValidateCreateComputePipelineArm(createInfo);
        }

        if (IsExtEnabled(device_extensions.vk_khr_maintenance4)) {
            auto module_state = Get<SHADER_MODULE_STATE>(createInfo.stage.module);
            if (module_state) {  // No module if creating from module identifier
                for (const Instruction* inst : module_state->GetBuiltinDecorationList()) {
                    if (inst->GetBuiltIn() == spv::BuiltInWorkgroupSize) {
                        skip |= LogWarning(device, kVUID_BestPractices_SpirvDeprecated_WorkgroupSize,
                                           "vkCreateComputePipelines(): pCreateInfos[ %" PRIu32
                                           "] is using the Workgroup built-in which SPIR-V 1.6 deprecated. The VK_KHR_maintenance4 "
                                           "extension exposes a new LocalSizeId execution mode that should be used instead.",
                                           i);
                    }
                }
            }
        }
    }

    return skip;
}

bool BestPractices::ValidateCreateComputePipelineArm(const VkComputePipelineCreateInfo& createInfo) const {
    bool skip = false;
    auto module_state = Get<SHADER_MODULE_STATE>(createInfo.stage.module);
    if (!module_state) {  // No module if creating from module identifier
        return false;
    }
    // Generate warnings about work group sizes based on active resources.
    auto entrypoint_optional = module_state->FindEntrypoint(createInfo.stage.pName, createInfo.stage.stage);
    if (!entrypoint_optional) return false;

    const Instruction& entrypoint = *entrypoint_optional;
    uint32_t x = 1, y = 1, z = 1;
    module_state->FindLocalSize(entrypoint, x, y, z);

    uint32_t thread_count = x * y * z;

    // Generate a priori warnings about work group sizes.
    if (thread_count > kMaxEfficientWorkGroupThreadCountArm) {
        skip |= LogPerformanceWarning(
            device, kVUID_BestPractices_CreateComputePipelines_ComputeWorkGroupSize,
            "%s vkCreateComputePipelines(): compute shader with work group dimensions (%u, %u, "
            "%u) (%u threads total), has more threads than advised in a single work group. It is advised to use work "
            "groups with less than %u threads, especially when using barrier() or shared memory.",
            VendorSpecificTag(kBPVendorArm), x, y, z, thread_count, kMaxEfficientWorkGroupThreadCountArm);
    }

    if (thread_count == 1 || ((x > 1) && (x & (kThreadGroupDispatchCountAlignmentArm - 1))) ||
        ((y > 1) && (y & (kThreadGroupDispatchCountAlignmentArm - 1))) ||
        ((z > 1) && (z & (kThreadGroupDispatchCountAlignmentArm - 1)))) {
        skip |= LogPerformanceWarning(device, kVUID_BestPractices_CreateComputePipelines_ComputeThreadGroupAlignment,
                                      "%s vkCreateComputePipelines(): compute shader with work group dimensions (%u, "
                                      "%u, %u) is not aligned to %u "
                                      "threads. On Arm Mali architectures, not aligning work group sizes to %u may "
                                      "leave threads idle on the shader "
                                      "core.",
                                      VendorSpecificTag(kBPVendorArm), x, y, z, kThreadGroupDispatchCountAlignmentArm,
                                      kThreadGroupDispatchCountAlignmentArm);
    }

    auto variables = module_state->GetResourceInterfaceVariable(entrypoint);
    if (variables) {
        unsigned dimensions = 0;
        if (x > 1) dimensions++;
        if (y > 1) dimensions++;
        if (z > 1) dimensions++;
        // Here the dimension will really depend on the dispatch grid, but assume it's 1D.
        dimensions = std::max(dimensions, 1u);

        // If we're accessing images, we almost certainly want to have a 2D workgroup for cache reasons.
        // There are some false positives here. We could simply have a shader that does this within a 1D grid,
        // or we may have a linearly tiled image, but these cases are quite unlikely in practice.
        bool accesses_2d = false;
        for (const auto& variable : *variables) {
            auto dim = module_state->GetShaderResourceDimensionality(variable);
            if (dim != spv::Dim1D && dim != spv::DimBuffer) {
                accesses_2d = true;
                break;
            }
        }

        if (accesses_2d && dimensions < 2) {
            LogPerformanceWarning(device, kVUID_BestPractices_CreateComputePipelines_ComputeSpatialLocality,
                                  "%s vkCreateComputePipelines(): compute shader has work group dimensions (%u, %u, %u), which "
                                  "suggests a 1D dispatch, but the shader is accessing 2D or 3D images. The shader may be "
                                  "exhibiting poor spatial locality with respect to one or more shader resources.",
                                  VendorSpecificTag(kBPVendorArm), x, y, z);
        }
    }

    return skip;
}

bool BestPractices::CheckPipelineStageFlags(const std::string& api_name, VkPipelineStageFlags flags) const {
    bool skip = false;

    if (flags & VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT) {
        skip |= LogWarning(device, kVUID_BestPractices_PipelineStageFlags,
                           "You are using VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT when %s is called\n", api_name.c_str());
    } else if (flags & VK_PIPELINE_STAGE_ALL_COMMANDS_BIT) {
        skip |= LogWarning(device, kVUID_BestPractices_PipelineStageFlags,
                           "You are using VK_PIPELINE_STAGE_ALL_COMMANDS_BIT when %s is called\n", api_name.c_str());
    }

    return skip;
}

bool BestPractices::CheckPipelineStageFlags(const std::string& api_name, VkPipelineStageFlags2KHR flags) const {
    bool skip = false;

    if (flags & VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT_KHR) {
        skip |= LogWarning(device, kVUID_BestPractices_PipelineStageFlags,
                           "You are using VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT_KHR when %s is called\n", api_name.c_str());
    } else if (flags & VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT_KHR) {
        skip |= LogWarning(device, kVUID_BestPractices_PipelineStageFlags,
                           "You are using VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT_KHR when %s is called\n", api_name.c_str());
    }

    return skip;
}

bool BestPractices::CheckDependencyInfo(const std::string& api_name, const VkDependencyInfoKHR& dep_info) const {
    bool skip = false;
    auto stage_masks = sync_utils::GetGlobalStageMasks(dep_info);

    skip |= CheckPipelineStageFlags(api_name, stage_masks.src);
    skip |= CheckPipelineStageFlags(api_name, stage_masks.dst);
    for (uint32_t i = 0; i < dep_info.imageMemoryBarrierCount; ++i) {
        skip |= ValidateImageMemoryBarrier(
            api_name, dep_info.pImageMemoryBarriers[i].oldLayout, dep_info.pImageMemoryBarriers[i].newLayout,
            dep_info.pImageMemoryBarriers[i].srcAccessMask, dep_info.pImageMemoryBarriers[i].dstAccessMask,
            dep_info.pImageMemoryBarriers[i].subresourceRange.aspectMask);
    }

    return skip;
}

void BestPractices::ManualPostCallRecordQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo, VkResult result) {
    for (uint32_t i = 0; i < pPresentInfo->swapchainCount; ++i) {
        auto swapchains_result = pPresentInfo->pResults ? pPresentInfo->pResults[i] : result;
        if (swapchains_result == VK_SUBOPTIMAL_KHR) {
            LogPerformanceWarning(
                pPresentInfo->pSwapchains[i], kVUID_BestPractices_SuboptimalSwapchain,
                "vkQueuePresentKHR: %s :VK_SUBOPTIMAL_KHR was returned. VK_SUBOPTIMAL_KHR - Presentation will still succeed, "
                "subject to the window resize behavior, but the swapchain is no longer configured optimally for the surface it "
                "targets. Applications should query updated surface information and recreate their swapchain at the next "
                "convenient opportunity.",
                report_data->FormatHandle(pPresentInfo->pSwapchains[i]).c_str());
        }
    }

    // AMD best practice
    // end-of-frame cleanup
    num_queue_submissions_ = 0;
    num_barriers_objects_ = 0;
    ClearPipelinesUsedInFrame();
}

bool BestPractices::PreCallValidateQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits,
                                               VkFence fence) const {
    bool skip = false;

    for (uint32_t submit = 0; submit < submitCount; submit++) {
        for (uint32_t semaphore = 0; semaphore < pSubmits[submit].waitSemaphoreCount; semaphore++) {
            skip |= CheckPipelineStageFlags("vkQueueSubmit", pSubmits[submit].pWaitDstStageMask[semaphore]);
        }
        if (pSubmits[submit].signalSemaphoreCount == 0 && pSubmits[submit].pSignalSemaphores != nullptr) {
            skip |=
                LogWarning(device, kVUID_BestPractices_SemaphoreCount,
                           "pSubmits[%" PRIu32 "].pSignalSemaphores is set, but pSubmits[%" PRIu32 "].signalSemaphoreCount is 0.",
                           submit, submit);
        }
        if (pSubmits[submit].waitSemaphoreCount == 0 && pSubmits[submit].pWaitSemaphores != nullptr) {
            skip |= LogWarning(device, kVUID_BestPractices_SemaphoreCount,
                               "pSubmits[%" PRIu32 "].pWaitSemaphores is set, but pSubmits[%" PRIu32 "].waitSemaphoreCount is 0.",
                               submit, submit);
        }
    }

    return skip;
}

bool BestPractices::PreCallValidateQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR* pSubmits,
                                                   VkFence fence) const {
    bool skip = false;

    for (uint32_t submit = 0; submit < submitCount; submit++) {
        for (uint32_t semaphore = 0; semaphore < pSubmits[submit].waitSemaphoreInfoCount; semaphore++) {
            skip |= CheckPipelineStageFlags("vkQueueSubmit2KHR", pSubmits[submit].pWaitSemaphoreInfos[semaphore].stageMask);
        }
    }

    return skip;
}

bool BestPractices::PreCallValidateQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits,
                                                VkFence fence) const {
    bool skip = false;

    for (uint32_t submit = 0; submit < submitCount; submit++) {
        for (uint32_t semaphore = 0; semaphore < pSubmits[submit].waitSemaphoreInfoCount; semaphore++) {
            skip |= CheckPipelineStageFlags("vkQueueSubmit2", pSubmits[submit].pWaitSemaphoreInfos[semaphore].stageMask);
        }
    }

    return skip;
}

bool BestPractices::PreCallValidateCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool) const {
    bool skip = false;

    if (pCreateInfo->flags & VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT) {
        skip |= LogPerformanceWarning(
            device, kVUID_BestPractices_CreateCommandPool_CommandBufferReset,
            "vkCreateCommandPool(): VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT is set. Consider resetting entire "
            "pool instead.");
    }

    return skip;
}

bool BestPractices::PreCallValidateAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* pAllocateInfo,
                                                          VkCommandBuffer* pCommandBuffers) const {
    bool skip = false;

    auto cp_state = Get<COMMAND_POOL_STATE>(pAllocateInfo->commandPool);
    if (!cp_state) return false;

    const VkQueueFlags queue_flags = physical_device_state->queue_family_properties[cp_state->queueFamilyIndex].queueFlags;
    const VkQueueFlags sec_cmd_buf_queue_flags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;

    if (pAllocateInfo->level == VK_COMMAND_BUFFER_LEVEL_SECONDARY && (queue_flags & sec_cmd_buf_queue_flags) == 0) {
        skip |= LogWarning(device, kVUID_BestPractices_AllocateCommandBuffers_UnusableSecondary,
                           "vkAllocateCommandBuffer(): Allocating secondary level command buffer from command pool "
                           "created against queue family #%u (queue flags: %s), but vkCmdExecuteCommands() is only "
                           "supported on queue families supporting VK_QUEUE_GRAPHICS_BIT, VK_QUEUE_COMPUTE_BIT, or "
                           "VK_QUEUE_TRANSFER_BIT. The allocated command buffer will not be submittable.",
                           cp_state->queueFamilyIndex, string_VkQueueFlags(queue_flags).c_str());
    }

    return skip;
}

void BestPractices::PreCallRecordBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo) {
    StateTracker::PreCallRecordBeginCommandBuffer(commandBuffer, pBeginInfo);

    auto cb = GetWrite<bp_state::CommandBuffer>(commandBuffer);
    if (!cb) return;

    cb->num_submits = 0;
    cb->is_one_time_submit = (pBeginInfo->flags & VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT) != 0;
}

bool BestPractices::PreCallValidateBeginCommandBuffer(VkCommandBuffer commandBuffer,
                                                      const VkCommandBufferBeginInfo* pBeginInfo) const {
    bool skip = false;

    if (pBeginInfo->flags & VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT) {
        skip |= LogPerformanceWarning(device, kVUID_BestPractices_BeginCommandBuffer_SimultaneousUse,
                                      "vkBeginCommandBuffer(): VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT is set.");
    }

    if (VendorCheckEnabled(kBPVendorArm)) {
        if (!(pBeginInfo->flags & VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT)) {
            skip |= LogPerformanceWarning(device, kVUID_BestPractices_BeginCommandBuffer_OneTimeSubmit,
                                          "%s vkBeginCommandBuffer(): VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT is not set. "
                                          "For best performance on Mali GPUs, consider setting ONE_TIME_SUBMIT by default.",
                                          VendorSpecificTag(kBPVendorArm));
        }
    }
    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        auto cb = GetRead<bp_state::CommandBuffer>(commandBuffer);
        if (cb->num_submits == 1 && !cb->is_one_time_submit) {
            skip |= LogPerformanceWarning(device, kVUID_BestPractices_BeginCommandBuffer_OneTimeSubmit,
                                          "%s vkBeginCommandBuffer(): VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT was not set "
                                          "and the command buffer has only been submitted once. "
                                          "For best performance on NVIDIA GPUs, use ONE_TIME_SUBMIT.",
                                          VendorSpecificTag(kBPVendorNVIDIA));
        }
    }

    return skip;
}

bool BestPractices::PreCallValidateCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) const {
    bool skip = false;

    skip |= CheckPipelineStageFlags("vkCmdSetEvent", stageMask);

    return skip;
}

bool BestPractices::PreCallValidateCmdSetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event,
                                                   const VkDependencyInfoKHR* pDependencyInfo) const {
    return CheckDependencyInfo("vkCmdSetEvent2KHR", *pDependencyInfo);
}

bool BestPractices::PreCallValidateCmdSetEvent2(VkCommandBuffer commandBuffer, VkEvent event,
                                                const VkDependencyInfo* pDependencyInfo) const {
    return CheckDependencyInfo("vkCmdSetEvent2", *pDependencyInfo);
}

bool BestPractices::PreCallValidateCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event,
                                                 VkPipelineStageFlags stageMask) const {
    bool skip = false;

    skip |= CheckPipelineStageFlags("vkCmdResetEvent", stageMask);

    return skip;
}

bool BestPractices::PreCallValidateCmdResetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event,
                                                     VkPipelineStageFlags2KHR stageMask) const {
    bool skip = false;

    skip |= CheckPipelineStageFlags("vkCmdResetEvent2KHR", stageMask);

    return skip;
}

bool BestPractices::PreCallValidateCmdResetEvent2(VkCommandBuffer commandBuffer, VkEvent event,
                                                  VkPipelineStageFlags2 stageMask) const {
    bool skip = false;

    skip |= CheckPipelineStageFlags("vkCmdResetEvent2", stageMask);

    return skip;
}

bool BestPractices::PreCallValidateCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                                 VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                                                 uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                                 uint32_t bufferMemoryBarrierCount,
                                                 const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                                 uint32_t imageMemoryBarrierCount,
                                                 const VkImageMemoryBarrier* pImageMemoryBarriers) const {
    bool skip = false;

    skip |= CheckPipelineStageFlags("vkCmdWaitEvents", srcStageMask);
    skip |= CheckPipelineStageFlags("vkCmdWaitEvents", dstStageMask);

    return skip;
}

bool BestPractices::PreCallValidateCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                                     const VkDependencyInfoKHR* pDependencyInfos) const {
    bool skip = false;
    for (uint32_t i = 0; i < eventCount; i++) {
        skip = CheckDependencyInfo("vkCmdWaitEvents2KHR", pDependencyInfos[i]);
    }

    return skip;
}

bool BestPractices::PreCallValidateCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                                  const VkDependencyInfo* pDependencyInfos) const {
    bool skip = false;
    for (uint32_t i = 0; i < eventCount; i++) {
        skip = CheckDependencyInfo("vkCmdWaitEvents2", pDependencyInfos[i]);
    }

    return skip;
}

bool BestPractices::ValidateAccessLayoutCombination(const std::string& api_name, VkAccessFlags2 access, VkImageLayout layout,
                                                    VkImageAspectFlags aspect) const {
    bool skip = false;

    const VkAccessFlags2 all = vvl::kU64Max;  // core validation is responsible for detecting undefined flags.
    VkAccessFlags2 allowed = 0;

    // Combinations taken from https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/2918
    switch (layout) {
        case VK_IMAGE_LAYOUT_UNDEFINED:
            allowed = all;
            break;
        case VK_IMAGE_LAYOUT_GENERAL:
            allowed = all;
            break;
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            allowed = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                      VK_ACCESS_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT;
            break;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            allowed = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
            allowed = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
            break;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            allowed = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_SHADER_READ_BIT;
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            allowed = VK_ACCESS_TRANSFER_READ_BIT;
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            allowed = VK_ACCESS_TRANSFER_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_PREINITIALIZED:
            allowed = VK_ACCESS_HOST_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
            if (aspect & VK_IMAGE_ASPECT_DEPTH_BIT) {
                allowed |= VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
            }
            if (aspect & VK_IMAGE_ASPECT_STENCIL_BIT) {
                allowed |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            }
            break;
        case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
            if (aspect & VK_IMAGE_ASPECT_DEPTH_BIT) {
                allowed |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            }
            if (aspect & VK_IMAGE_ASPECT_STENCIL_BIT) {
                allowed |= VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
            }
            break;
        case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
            allowed = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL:
            allowed = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
            break;
        case VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL:
            allowed = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL:
            allowed = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
            break;
        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
            allowed = VK_ACCESS_NONE;  // PR table says "Must be 0"
            break;
        case VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR:
            allowed = all;
            break;
        // alias VK_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV
        case VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR:
            // alias VK_ACCESS_SHADING_RATE_IMAGE_READ_BIT_NV
            allowed = VK_ACCESS_FRAGMENT_SHADING_RATE_ATTACHMENT_READ_BIT_KHR;
            break;
        case VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT:
            allowed = VK_ACCESS_FRAGMENT_DENSITY_MAP_READ_BIT_EXT;
            break;
        default:
            // If a new layout is added, will need to manually add it
            return false;
    }

    if ((allowed | access) != allowed) {
        skip |=
            LogWarning(device, kVUID_BestPractices_ImageBarrierAccessLayout,
                       "%s: accessMask is %s, but for layout %s expected accessMask are %s.", api_name.c_str(),
                       string_VkAccessFlags2(access).c_str(), string_VkImageLayout(layout), string_VkAccessFlags2(allowed).c_str());
    }

    return skip;
}

bool BestPractices::ValidateImageMemoryBarrier(const std::string& api_name, VkImageLayout oldLayout, VkImageLayout newLayout,
                                               VkAccessFlags2 srcAccessMask, VkAccessFlags2 dstAccessMask,
                                               VkImageAspectFlags aspectMask) const {
    bool skip = false;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && IsImageLayoutReadOnly(newLayout)) {
        skip |= LogWarning(device, kVUID_BestPractices_TransitionUndefinedToReadOnly,
                           "VkImageMemoryBarrier is being submitted with oldLayout VK_IMAGE_LAYOUT_UNDEFINED and the contents "
                           "may be discarded, but the newLayout is %s, which is read only.",
                           string_VkImageLayout(newLayout));
    }

    skip |= ValidateAccessLayoutCombination(api_name, srcAccessMask, oldLayout, aspectMask);
    skip |= ValidateAccessLayoutCombination(api_name, dstAccessMask, newLayout, aspectMask);

    return skip;
}

bool BestPractices::PreCallValidateCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                                      VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                                      uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                                      uint32_t bufferMemoryBarrierCount,
                                                      const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                                      uint32_t imageMemoryBarrierCount,
                                                      const VkImageMemoryBarrier* pImageMemoryBarriers) const {
    bool skip = false;

    skip |= CheckPipelineStageFlags("vkCmdPipelineBarrier", srcStageMask);
    skip |= CheckPipelineStageFlags("vkCmdPipelineBarrier", dstStageMask);

    for (uint32_t i = 0; i < imageMemoryBarrierCount; ++i) {
        skip |=
            ValidateImageMemoryBarrier("vkCmdPipelineBarrier", pImageMemoryBarriers[i].oldLayout, pImageMemoryBarriers[i].newLayout,
                                       pImageMemoryBarriers[i].srcAccessMask, pImageMemoryBarriers[i].dstAccessMask,
                                       pImageMemoryBarriers[i].subresourceRange.aspectMask);
    }

    if (VendorCheckEnabled(kBPVendorAMD)) {
        auto num = num_barriers_objects_.load();
        if (num + imageMemoryBarrierCount + bufferMemoryBarrierCount > kMaxRecommendedBarriersSizeAMD) {
            skip |= LogPerformanceWarning(device, kVUID_BestPractices_CmdBuffer_highBarrierCount,
                                          "%s Performance warning: In this frame, %" PRIu32
                                          " barriers were already submitted. Barriers have a high cost and can "
                                          "stall the GPU. "
                                          "Consider consolidating and re-organizing the frame to use fewer barriers.",
                                          VendorSpecificTag(kBPVendorAMD), num);
        }
    }
    if (VendorCheckEnabled(kBPVendorAMD) || VendorCheckEnabled(kBPVendorNVIDIA)) {
        static constexpr std::array<VkImageLayout, 3> read_layouts = {
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        };

        for (uint32_t i = 0; i < imageMemoryBarrierCount; i++) {
            // read to read barriers
            const auto& image_barrier = pImageMemoryBarriers[i];
            const bool old_is_read_layout =
                std::find(read_layouts.begin(), read_layouts.end(), image_barrier.oldLayout) != read_layouts.end();
            const bool new_is_read_layout =
                std::find(read_layouts.begin(), read_layouts.end(), image_barrier.newLayout) != read_layouts.end();

            if (old_is_read_layout && new_is_read_layout) {
                skip |= LogPerformanceWarning(device, kVUID_BestPractices_PipelineBarrier_readToReadBarrier,
                                              "%s %s Performance warning: Don't issue read-to-read barriers. "
                                              "Get the resource in the right state the first time you use it.",
                                              VendorSpecificTag(kBPVendorAMD), VendorSpecificTag(kBPVendorNVIDIA));
            }

            // general with no storage
            if (VendorCheckEnabled(kBPVendorAMD) && image_barrier.newLayout == VK_IMAGE_LAYOUT_GENERAL) {
                auto image_state = Get<IMAGE_STATE>(pImageMemoryBarriers[i].image);
                if (!(image_state->createInfo.usage & VK_IMAGE_USAGE_STORAGE_BIT)) {
                    skip |= LogPerformanceWarning(device, kVUID_BestPractices_vkImage_AvoidGeneral,
                                                  "%s Performance warning: VK_IMAGE_LAYOUT_GENERAL should only be used with "
                                                  "VK_IMAGE_USAGE_STORAGE_BIT images.",
                                                  VendorSpecificTag(kBPVendorAMD));
                }
            }
        }
    }

    for (uint32_t i = 0; i < imageMemoryBarrierCount; ++i) {
        skip |= ValidateCmdPipelineBarrierImageBarrier(commandBuffer, pImageMemoryBarriers[i]);
    }

    return skip;
}

bool BestPractices::PreCallValidateCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer,
                                                          const VkDependencyInfoKHR* pDependencyInfo) const {
    bool skip = false;

    skip |= CheckDependencyInfo("vkCmdPipelineBarrier2KHR", *pDependencyInfo);

    for (uint32_t i = 0; i < pDependencyInfo->imageMemoryBarrierCount; ++i) {
        skip |= ValidateCmdPipelineBarrierImageBarrier(commandBuffer, pDependencyInfo->pImageMemoryBarriers[i]);
    }

    return skip;
}

bool BestPractices::PreCallValidateCmdPipelineBarrier2(VkCommandBuffer commandBuffer,
                                                       const VkDependencyInfo* pDependencyInfo) const {
    bool skip = false;

    skip |= CheckDependencyInfo("vkCmdPipelineBarrier2", *pDependencyInfo);

    for (uint32_t i = 0; i < pDependencyInfo->imageMemoryBarrierCount; ++i) {
        skip |= ValidateCmdPipelineBarrierImageBarrier(commandBuffer, pDependencyInfo->pImageMemoryBarriers[i]);
    }

    return skip;
}

template <typename ImageMemoryBarrier>
bool BestPractices::ValidateCmdPipelineBarrierImageBarrier(VkCommandBuffer commandBuffer, const ImageMemoryBarrier& barrier) const {
    bool skip = false;

    const auto cmd_state = GetRead<bp_state::CommandBuffer>(commandBuffer);
    assert(cmd_state);

    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        if (barrier.oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && barrier.newLayout != VK_IMAGE_LAYOUT_UNDEFINED) {
            skip |= ValidateZcull(*cmd_state, barrier.image, barrier.subresourceRange);
        }
    }

    return skip;
}

bool BestPractices::PreCallValidateCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                                     VkQueryPool queryPool, uint32_t query) const {
    bool skip = false;

    skip |= CheckPipelineStageFlags("vkCmdWriteTimestamp", static_cast<VkPipelineStageFlags>(pipelineStage));

    return skip;
}

bool BestPractices::PreCallValidateCmdWriteTimestamp2KHR(VkCommandBuffer commandBuffer, VkPipelineStageFlags2KHR pipelineStage,
                                                         VkQueryPool queryPool, uint32_t query) const {
    bool skip = false;

    skip |= CheckPipelineStageFlags("vkCmdWriteTimestamp2KHR", pipelineStage);

    return skip;
}

bool BestPractices::PreCallValidateCmdWriteTimestamp2(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 pipelineStage,
                                                      VkQueryPool queryPool, uint32_t query) const {
    bool skip = false;

    skip |= CheckPipelineStageFlags("vkCmdWriteTimestamp2", pipelineStage);

    return skip;
}

void BestPractices::PreCallRecordCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                 VkPipeline pipeline) {
    StateTracker::PreCallRecordCmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline);

    auto pipeline_info = Get<PIPELINE_STATE>(pipeline);
    auto cb = GetWrite<bp_state::CommandBuffer>(commandBuffer);

    assert(pipeline_info);
    assert(cb);

    if (pipelineBindPoint == VK_PIPELINE_BIND_POINT_GRAPHICS && VendorCheckEnabled(kBPVendorNVIDIA)) {
        using TessGeometryMeshState = bp_state::CommandBufferStateNV::TessGeometryMesh::State;
        auto& tgm = cb->nv.tess_geometry_mesh;

        // Make sure the message is only signaled once per command buffer
        tgm.threshold_signaled = tgm.num_switches >= kNumBindPipelineTessGeometryMeshSwitchesThresholdNVIDIA;

        // Track pipeline switches with tessellation, geometry, and/or mesh shaders enabled, and disabled
        auto tgm_stages = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT |
                          VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_TASK_BIT_NV | VK_SHADER_STAGE_MESH_BIT_NV;
        auto new_tgm_state =
            (pipeline_info->active_shaders & tgm_stages) != 0 ? TessGeometryMeshState::Enabled : TessGeometryMeshState::Disabled;
        if (tgm.state != new_tgm_state && tgm.state != TessGeometryMeshState::Unknown) {
            tgm.num_switches++;
        }
        tgm.state = new_tgm_state;

        // Track depthTestEnable and depthCompareOp
        auto& pipeline_create_info = pipeline_info->GetCreateInfo<VkGraphicsPipelineCreateInfo>();
        auto depth_stencil_state = pipeline_create_info.pDepthStencilState;
        auto dynamic_state = pipeline_create_info.pDynamicState;
        if (depth_stencil_state && dynamic_state) {
            auto dynamic_state_begin = dynamic_state->pDynamicStates;
            auto dynamic_state_end = dynamic_state->pDynamicStates + dynamic_state->dynamicStateCount;

            const bool dynamic_depth_test_enable =
                std::find(dynamic_state_begin, dynamic_state_end, VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE) != dynamic_state_end;
            const bool dynamic_depth_func =
                std::find(dynamic_state_begin, dynamic_state_end, VK_DYNAMIC_STATE_DEPTH_COMPARE_OP) != dynamic_state_end;

            if (!dynamic_depth_test_enable) {
                RecordSetDepthTestState(*cb, cb->nv.depth_compare_op, depth_stencil_state->depthTestEnable != VK_FALSE);
            }
            if (!dynamic_depth_func) {
                RecordSetDepthTestState(*cb, depth_stencil_state->depthCompareOp, cb->nv.depth_test_enable);
            }
        }
    }
}

void BestPractices::PostCallRecordCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                  VkPipeline pipeline) {
    StateTracker::PostCallRecordCmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline);

    // AMD best practice
    PipelineUsedInFrame(pipeline);

    if (pipelineBindPoint == VK_PIPELINE_BIND_POINT_GRAPHICS) {
        auto pipeline_state = Get<bp_state::Pipeline>(pipeline);
        // check for depth/blend state tracking
        if (pipeline_state) {
            auto cb_node = GetWrite<bp_state::CommandBuffer>(commandBuffer);
            assert(cb_node);
            auto& render_pass_state = cb_node->render_pass_state;

            render_pass_state.nextDrawTouchesAttachments = pipeline_state->access_framebuffer_attachments;
            render_pass_state.drawTouchAttachments = true;

            const auto* blend_state = pipeline_state->ColorBlendState();
            const auto* stencil_state = pipeline_state->DepthStencilState();

            if (blend_state) {
                // assume the pipeline is depth-only unless any of the attachments have color writes enabled
                render_pass_state.depthOnly = true;
                for (size_t i = 0; i < blend_state->attachmentCount; i++) {
                    if (blend_state->pAttachments[i].colorWriteMask != 0) {
                        render_pass_state.depthOnly = false;
                    }
                }
            }

            // check for depth value usage
            render_pass_state.depthEqualComparison = false;

            if (stencil_state && stencil_state->depthTestEnable) {
                switch (stencil_state->depthCompareOp) {
                    case VK_COMPARE_OP_EQUAL:
                    case VK_COMPARE_OP_GREATER_OR_EQUAL:
                    case VK_COMPARE_OP_LESS_OR_EQUAL:
                        render_pass_state.depthEqualComparison = true;
                        break;
                    default:
                        break;
                }
            }
        }
    }
}

void BestPractices::PreCallRecordCmdSetDepthCompareOp(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp) {
    StateTracker::PreCallRecordCmdSetDepthCompareOp(commandBuffer, depthCompareOp);

    auto cb = GetWrite<bp_state::CommandBuffer>(commandBuffer);
    assert(cb);

    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        RecordSetDepthTestState(*cb, depthCompareOp, cb->nv.depth_test_enable);
    }
}

void BestPractices::PreCallRecordCmdSetDepthCompareOpEXT(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp) {
    StateTracker::PreCallRecordCmdSetDepthCompareOpEXT(commandBuffer, depthCompareOp);

    auto cb = GetWrite<bp_state::CommandBuffer>(commandBuffer);
    assert(cb);

    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        RecordSetDepthTestState(*cb, depthCompareOp, cb->nv.depth_test_enable);
    }
}

void BestPractices::PreCallRecordCmdSetDepthTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable) {
    StateTracker::PreCallRecordCmdSetDepthTestEnable(commandBuffer, depthTestEnable);

    auto cb = GetWrite<bp_state::CommandBuffer>(commandBuffer);
    assert(cb);

    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        RecordSetDepthTestState(*cb, cb->nv.depth_compare_op, depthTestEnable != VK_FALSE);
    }
}

void BestPractices::PreCallRecordCmdSetDepthTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable) {
    StateTracker::PreCallRecordCmdSetDepthTestEnableEXT(commandBuffer, depthTestEnable);

    auto cb = GetWrite<bp_state::CommandBuffer>(commandBuffer);
    assert(cb);

    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        RecordSetDepthTestState(*cb, cb->nv.depth_compare_op, depthTestEnable != VK_FALSE);
    }
}

void BestPractices::RecordSetDepthTestState(bp_state::CommandBuffer& cmd_state, VkCompareOp new_depth_compare_op,
                                            bool new_depth_test_enable) {
    assert(VendorCheckEnabled(kBPVendorNVIDIA));

    if (cmd_state.nv.depth_compare_op != new_depth_compare_op) {
        switch (new_depth_compare_op) {
            case VK_COMPARE_OP_LESS:
            case VK_COMPARE_OP_LESS_OR_EQUAL:
                cmd_state.nv.zcull_direction = bp_state::CommandBufferStateNV::ZcullDirection::Less;
                break;
            case VK_COMPARE_OP_GREATER:
            case VK_COMPARE_OP_GREATER_OR_EQUAL:
                cmd_state.nv.zcull_direction = bp_state::CommandBufferStateNV::ZcullDirection::Greater;
                break;
            default:
                // The other ops carry over the previous state.
                break;
        }
    }
    cmd_state.nv.depth_compare_op = new_depth_compare_op;
    cmd_state.nv.depth_test_enable = new_depth_test_enable;
}

void BestPractices::RecordCmdBeginRenderingCommon(VkCommandBuffer commandBuffer) {
    auto cmd_state = GetWrite<bp_state::CommandBuffer>(commandBuffer);
    assert(cmd_state);

    auto rp = cmd_state->activeRenderPass.get();
    assert(rp);

    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        std::shared_ptr<IMAGE_VIEW_STATE> depth_image_view_shared_ptr;
        IMAGE_VIEW_STATE* depth_image_view = nullptr;
        std::optional<VkAttachmentLoadOp> load_op;

        if (rp->use_dynamic_rendering || rp->use_dynamic_rendering_inherited) {
            const auto depth_attachment = rp->dynamic_rendering_begin_rendering_info.pDepthAttachment;
            if (depth_attachment) {
                load_op.emplace(depth_attachment->loadOp);
                depth_image_view_shared_ptr = Get<IMAGE_VIEW_STATE>(depth_attachment->imageView);
                depth_image_view = depth_image_view_shared_ptr.get();
            }

            for (uint32_t i = 0; i < rp->dynamic_rendering_begin_rendering_info.colorAttachmentCount; ++i) {
                const auto& color_attachment = rp->dynamic_rendering_begin_rendering_info.pColorAttachments[i];
                if (color_attachment.loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR) {
                    const VkFormat format = Get<IMAGE_VIEW_STATE>(color_attachment.imageView)->create_info.format;
                    RecordClearColor(format, color_attachment.clearValue.color);
                }
            }

        } else {
            if (rp->createInfo.pAttachments) {
                if (rp->createInfo.subpassCount > 0) {
                    const auto depth_attachment = rp->createInfo.pSubpasses[0].pDepthStencilAttachment;
                    if (depth_attachment) {
                        const uint32_t attachment_index = depth_attachment->attachment;
                        if (attachment_index != VK_ATTACHMENT_UNUSED) {
                            load_op.emplace(rp->createInfo.pAttachments[attachment_index].loadOp);
                            depth_image_view = (*cmd_state->active_attachments)[attachment_index];
                        }
                    }
                }
                for (uint32_t i = 0; i < cmd_state->activeRenderPassBeginInfo.clearValueCount; ++i) {
                    const auto& attachment = rp->createInfo.pAttachments[i];
                    if (attachment.loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR) {
                        const auto& clear_color = cmd_state->activeRenderPassBeginInfo.pClearValues[i].color;
                        RecordClearColor(attachment.format, clear_color);
                    }
                }
            }
        }
        if (depth_image_view && (depth_image_view->create_info.subresourceRange.aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT) != 0U) {
            const VkImage depth_image = depth_image_view->image_state->image();
            const VkImageSubresourceRange& subresource_range = depth_image_view->create_info.subresourceRange;
            RecordBindZcullScope(*cmd_state, depth_image, subresource_range);
        } else {
            RecordUnbindZcullScope(*cmd_state);
        }
        if (load_op) {
            if (*load_op == VK_ATTACHMENT_LOAD_OP_CLEAR || *load_op == VK_ATTACHMENT_LOAD_OP_DONT_CARE) {
                RecordResetScopeZcullDirection(*cmd_state);
            }
        }
    }
}

void BestPractices::RecordCmdEndRenderingCommon(VkCommandBuffer commandBuffer) {
    auto cmd_state = GetWrite<bp_state::CommandBuffer>(commandBuffer);
    assert(cmd_state);

    auto rp = cmd_state->activeRenderPass.get();
    assert(rp);

    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        std::optional<VkAttachmentStoreOp> store_op;

        if (rp->use_dynamic_rendering || rp->use_dynamic_rendering_inherited) {
            const auto depth_attachment = rp->dynamic_rendering_begin_rendering_info.pDepthAttachment;
            if (depth_attachment) {
                store_op.emplace(depth_attachment->storeOp);
            }
        } else {
            if (rp->createInfo.subpassCount > 0) {
                const uint32_t last_subpass = rp->createInfo.subpassCount - 1;
                const auto depth_attachment = rp->createInfo.pSubpasses[last_subpass].pDepthStencilAttachment;
                if (depth_attachment) {
                    const uint32_t attachment = depth_attachment->attachment;
                    if (attachment != VK_ATTACHMENT_UNUSED) {
                        store_op.emplace(rp->createInfo.pAttachments[attachment].storeOp);
                    }
                }
            }
        }

        if (store_op) {
            if (*store_op == VK_ATTACHMENT_STORE_OP_DONT_CARE || *store_op == VK_ATTACHMENT_STORE_OP_NONE) {
                RecordResetScopeZcullDirection(*cmd_state);
            }
        }

        RecordUnbindZcullScope(*cmd_state);
    }
}

void BestPractices::RecordBindZcullScope(bp_state::CommandBuffer& cmd_state, VkImage depth_attachment,
                                         const VkImageSubresourceRange& subresource_range) {
    assert(VendorCheckEnabled(kBPVendorNVIDIA));

    if (depth_attachment == VK_NULL_HANDLE) {
        cmd_state.nv.zcull_scope = {};
        return;
    }

    assert((subresource_range.aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT) != 0U);

    auto image_state = Get<IMAGE_STATE>(depth_attachment);
    assert(image_state);

    const uint32_t mip_levels = image_state->createInfo.mipLevels;
    const uint32_t array_layers = image_state->createInfo.arrayLayers;

    auto& tree = cmd_state.nv.zcull_per_image[depth_attachment];
    if (tree.states.empty()) {
        tree.mip_levels = mip_levels;
        tree.array_layers = array_layers;
        tree.states.resize(array_layers * mip_levels);
    }

    cmd_state.nv.zcull_scope.image = depth_attachment;
    cmd_state.nv.zcull_scope.range = subresource_range;
    cmd_state.nv.zcull_scope.tree = &tree;
}

void BestPractices::RecordUnbindZcullScope(bp_state::CommandBuffer& cmd_state) {
    assert(VendorCheckEnabled(kBPVendorNVIDIA));

    RecordBindZcullScope(cmd_state, VK_NULL_HANDLE, VkImageSubresourceRange{});
}

void BestPractices::RecordResetScopeZcullDirection(bp_state::CommandBuffer& cmd_state) {
    assert(VendorCheckEnabled(kBPVendorNVIDIA));

    auto& scope = cmd_state.nv.zcull_scope;
    RecordResetZcullDirection(cmd_state, scope.image, scope.range);
}

template <typename Func>
static void ForEachSubresource(const IMAGE_STATE& image, const VkImageSubresourceRange& range, Func&& func) {
    const uint32_t layerCount =
        (range.layerCount == VK_REMAINING_ARRAY_LAYERS) ? (image.full_range.layerCount - range.baseArrayLayer) : range.layerCount;
    const uint32_t levelCount =
        (range.levelCount == VK_REMAINING_MIP_LEVELS) ? (image.full_range.levelCount - range.baseMipLevel) : range.levelCount;

    for (uint32_t i = 0; i < layerCount; ++i) {
        const uint32_t layer = range.baseArrayLayer + i;
        for (uint32_t j = 0; j < levelCount; ++j) {
            const uint32_t level = range.baseMipLevel + j;
            func(layer, level);
        }
    }
}

void BestPractices::RecordResetZcullDirection(bp_state::CommandBuffer& cmd_state, VkImage depth_image,
                                              const VkImageSubresourceRange& subresource_range) {
    assert(VendorCheckEnabled(kBPVendorNVIDIA));

    RecordSetZcullDirection(cmd_state, depth_image, subresource_range, bp_state::CommandBufferStateNV::ZcullDirection::Unknown);

    const auto image_it = cmd_state.nv.zcull_per_image.find(depth_image);
    if (image_it == cmd_state.nv.zcull_per_image.end()) {
        return;
    }
    auto& tree = image_it->second;

    auto image = Get<IMAGE_STATE>(depth_image);
    if (!image) return;

    ForEachSubresource(*image, subresource_range, [&tree](uint32_t layer, uint32_t level) {
        auto& subresource = tree.GetState(layer, level);
        subresource.num_less_draws = 0;
        subresource.num_greater_draws = 0;
    });
}

void BestPractices::RecordSetScopeZcullDirection(bp_state::CommandBuffer& cmd_state,
                                                 bp_state::CommandBufferStateNV::ZcullDirection mode) {
    assert(VendorCheckEnabled(kBPVendorNVIDIA));

    auto& scope = cmd_state.nv.zcull_scope;
    RecordSetZcullDirection(cmd_state, scope.image, scope.range, mode);
}

void BestPractices::RecordSetZcullDirection(bp_state::CommandBuffer& cmd_state, VkImage depth_image,
                                            const VkImageSubresourceRange& subresource_range,
                                            bp_state::CommandBufferStateNV::ZcullDirection mode) {
    assert(VendorCheckEnabled(kBPVendorNVIDIA));

    const auto image_it = cmd_state.nv.zcull_per_image.find(depth_image);
    if (image_it == cmd_state.nv.zcull_per_image.end()) {
        return;
    }
    auto& tree = image_it->second;

    auto image = Get<IMAGE_STATE>(depth_image);
    if (!image) return;

    ForEachSubresource(*image, subresource_range, [&tree, &cmd_state](uint32_t layer, uint32_t level) {
        tree.GetState(layer, level).direction = cmd_state.nv.zcull_direction;
    });
}

void BestPractices::RecordZcullDraw(bp_state::CommandBuffer& cmd_state) {
    assert(VendorCheckEnabled(kBPVendorNVIDIA));

    // Add one draw to each subresource depending on the current Z-cull direction
    auto& scope = cmd_state.nv.zcull_scope;

    auto image = Get<IMAGE_STATE>(scope.image);
    if (!image) return;

    ForEachSubresource(*image, scope.range, [&scope](uint32_t layer, uint32_t level) {
        auto& subresource = scope.tree->GetState(layer, level);

        switch (subresource.direction) {
            case bp_state::CommandBufferStateNV::ZcullDirection::Unknown:
                // Unreachable
                assert(0);
                break;
            case bp_state::CommandBufferStateNV::ZcullDirection::Less:
                ++subresource.num_less_draws;
                break;
            case bp_state::CommandBufferStateNV::ZcullDirection::Greater:
                ++subresource.num_greater_draws;
                break;
        }
    });
}

bool BestPractices::ValidateZcullScope(const bp_state::CommandBuffer& cmd_state) const {
    assert(VendorCheckEnabled(kBPVendorNVIDIA));

    bool skip = false;

    if (cmd_state.nv.depth_test_enable) {
        auto& scope = cmd_state.nv.zcull_scope;
        skip |= ValidateZcull(cmd_state, scope.image, scope.range);
    }

    return skip;
}

bool BestPractices::ValidateZcull(const bp_state::CommandBuffer& cmd_state, VkImage image,
                                  const VkImageSubresourceRange& subresource_range) const {
    bool skip = false;

    const char* good_mode = nullptr;
    const char* bad_mode = nullptr;
    bool is_balanced = false;

    const auto image_it = cmd_state.nv.zcull_per_image.find(image);
    if (image_it == cmd_state.nv.zcull_per_image.end()) {
        return skip;
    }
    const auto& tree = image_it->second;

    auto image_state = Get<IMAGE_STATE>(image);
    if (!image_state) {
        return skip;
    }

    ForEachSubresource(*image_state, subresource_range, [&](uint32_t layer, uint32_t level) {
        if (is_balanced) {
            return;
        }
        const auto& resource = tree.GetState(layer, level);
        const uint64_t num_draws = resource.num_less_draws + resource.num_greater_draws;

        if (num_draws == 0) {
            return;
        }
        const uint64_t less_ratio = (resource.num_less_draws * 100) / num_draws;
        const uint64_t greater_ratio = (resource.num_greater_draws * 100) / num_draws;

        if ((less_ratio > kZcullDirectionBalanceRatioNVIDIA) && (greater_ratio > kZcullDirectionBalanceRatioNVIDIA)) {
            is_balanced = true;

            if (greater_ratio > less_ratio) {
                good_mode = "GREATER";
                bad_mode = "LESS";
            } else {
                good_mode = "LESS";
                bad_mode = "GREATER";
            }
        }
    });

    if (is_balanced) {
        skip |= LogPerformanceWarning(
            cmd_state.commandBuffer(), kVUID_BestPractices_Zcull_LessGreaterRatio,
            "%s Depth attachment %s is primarily rendered with depth compare op %s, but some draws use %s. "
            "Z-cull is disabled for the least used direction, which harms depth testing performance. "
            "The Z-cull direction can be reset by clearing the depth attachment, transitioning from VK_IMAGE_LAYOUT_UNDEFINED, "
            "using VK_ATTACHMENT_LOAD_OP_DONT_CARE, or using VK_ATTACHMENT_STORE_OP_DONT_CARE.",
            VendorSpecificTag(kBPVendorNVIDIA), report_data->FormatHandle(cmd_state.nv.zcull_scope.image).c_str(), good_mode,
            bad_mode);
    }

    return skip;
}

static std::array<uint32_t, 4> GetRawClearColor(VkFormat format, const VkClearColorValue& clear_value) {
    std::array<uint32_t, 4> raw_color{};
    std::copy_n(clear_value.uint32, raw_color.size(), raw_color.data());

    // Zero out unused components to avoid polluting the cache with garbage
    if (!FormatHasRed(format)) raw_color[0] = 0;
    if (!FormatHasGreen(format)) raw_color[1] = 0;
    if (!FormatHasBlue(format)) raw_color[2] = 0;
    if (!FormatHasAlpha(format)) raw_color[3] = 0;

    return raw_color;
}

static bool IsClearColorZeroOrOne(VkFormat format, const std::array<uint32_t, 4> clear_color) {
    static_assert(sizeof(float) == sizeof(uint32_t), "Mismatching float <-> uint32 sizes");
    const float one = 1.0f;
    const float zero = 0.0f;
    uint32_t raw_one{};
    uint32_t raw_zero{};
    memcpy(&raw_one, &one, sizeof(one));
    memcpy(&raw_zero, &zero, sizeof(zero));

    const bool is_one =
        (!FormatHasRed(format) || (clear_color[0] == raw_one)) && (!FormatHasGreen(format) || (clear_color[1] == raw_one)) &&
        (!FormatHasBlue(format) || (clear_color[2] == raw_one)) && (!FormatHasAlpha(format) || (clear_color[3] == raw_one));
    const bool is_zero =
        (!FormatHasRed(format) || (clear_color[0] == raw_zero)) && (!FormatHasGreen(format) || (clear_color[1] == raw_zero)) &&
        (!FormatHasBlue(format) || (clear_color[2] == raw_zero)) && (!FormatHasAlpha(format) || (clear_color[3] == raw_zero));
    return is_one || is_zero;
}

static std::string MakeCompressedFormatListNVIDIA() {
    std::string format_list;
    for (VkFormat compressed_format : kCustomClearColorCompressedFormatsNVIDIA) {
        if (compressed_format == kCustomClearColorCompressedFormatsNVIDIA.back()) {
            format_list += "or ";
        }
        format_list += string_VkFormat(compressed_format);
        if (compressed_format != kCustomClearColorCompressedFormatsNVIDIA.back()) {
            format_list += ", ";
        }
    }
    return format_list;
}

void BestPractices::RecordClearColor(VkFormat format, const VkClearColorValue& clear_value) {
    assert(VendorCheckEnabled(kBPVendorNVIDIA));

    const std::array<uint32_t, 4> raw_color = GetRawClearColor(format, clear_value);
    if (IsClearColorZeroOrOne(format, raw_color)) {
        // These colors are always compressed
        return;
    }

    const auto it =
        std::find(kCustomClearColorCompressedFormatsNVIDIA.begin(), kCustomClearColorCompressedFormatsNVIDIA.end(), format);
    if (it == kCustomClearColorCompressedFormatsNVIDIA.end()) {
        // The format cannot be compressed with a custom color
        return;
    }

    // Record custom clear color
    WriteLockGuard guard{clear_colors_lock_};
    if (clear_colors_.size() < kMaxRecommendedNumberOfClearColorsNVIDIA) {
        clear_colors_.insert(raw_color);
    }
}

bool BestPractices::ValidateClearColor(VkCommandBuffer commandBuffer, VkFormat format, const VkClearColorValue& clear_value) const {
    assert(VendorCheckEnabled(kBPVendorNVIDIA));

    bool skip = false;

    const std::array<uint32_t, 4> raw_color = GetRawClearColor(format, clear_value);
    if (IsClearColorZeroOrOne(format, raw_color)) {
        return skip;
    }

    const auto it =
        std::find(kCustomClearColorCompressedFormatsNVIDIA.begin(), kCustomClearColorCompressedFormatsNVIDIA.end(), format);
    if (it == kCustomClearColorCompressedFormatsNVIDIA.end()) {
        // The format is not compressible
        static const std::string format_list = MakeCompressedFormatListNVIDIA();

        skip |= LogPerformanceWarning(commandBuffer, kVUID_BestPractices_ClearColor_NotCompressed,
                                      "%s Clearing image with format %s without a 1.0f or 0.0f clear color. "
                                      "The clear will not get compressed in the GPU, harming performance. "
                                      "This can be fixed using a clear color of VkClearColorValue{0.0f, 0.0f, 0.0f, 0.0f}, or "
                                      "VkClearColorValue{1.0f, 1.0f, 1.0f, 1.0f}. Alternatively, use %s.",
                                      VendorSpecificTag(kBPVendorNVIDIA), string_VkFormat(format), format_list.c_str());
    } else {
        // The format is compressible
        bool registered = false;
        {
            ReadLockGuard guard{clear_colors_lock_};
            registered = clear_colors_.find(raw_color) != clear_colors_.end();

            if (!registered) {
                // If it's not in the list, it might be new. Check if there's still space for new entries.
                registered = clear_colors_.size() < kMaxRecommendedNumberOfClearColorsNVIDIA;
            }
        }
        if (!registered) {
            std::string clear_color_str;

            if (FormatIsUINT(format)) {
                clear_color_str = std::to_string(clear_value.uint32[0]) + ", " + std::to_string(clear_value.uint32[1]) + ", " +
                                  std::to_string(clear_value.uint32[2]) + ", " + std::to_string(clear_value.uint32[3]);
            } else if (FormatIsSINT(format)) {
                clear_color_str = std::to_string(clear_value.int32[0]) + ", " + std::to_string(clear_value.int32[1]) + ", " +
                                  std::to_string(clear_value.int32[2]) + ", " + std::to_string(clear_value.int32[3]);
            } else {
                clear_color_str = std::to_string(clear_value.float32[0]) + ", " + std::to_string(clear_value.float32[1]) + ", " +
                                  std::to_string(clear_value.float32[2]) + ", " + std::to_string(clear_value.float32[3]);
            }

            skip |= LogPerformanceWarning(
                commandBuffer, kVUID_BestPractices_ClearColor_NotCompressed,
                "%s Clearing image with unregistered VkClearColorValue{%s}. "
                "This clear will not get compressed in the GPU, harming performance. "
                "The clear color is not registered because too many unique colors have been used. "
                "Select a discrete set of clear colors and stick to those. "
                "VkClearColorValue{0, 0, 0, 0} and VkClearColorValue{1.0f, 1.0f, 1.0f, 1.0f} are always registered.",
                VendorSpecificTag(kBPVendorNVIDIA), clear_color_str.c_str());
        }
    }

    return skip;
}

static inline bool RenderPassUsesAttachmentAsResolve(const safe_VkRenderPassCreateInfo2& createInfo, uint32_t attachment) {
    for (uint32_t subpass = 0; subpass < createInfo.subpassCount; subpass++) {
        const auto& subpass_info = createInfo.pSubpasses[subpass];
        if (subpass_info.pResolveAttachments) {
            for (uint32_t i = 0; i < subpass_info.colorAttachmentCount; i++) {
                if (subpass_info.pResolveAttachments[i].attachment == attachment) return true;
            }
        }
    }

    return false;
}

static inline bool RenderPassUsesAttachmentOnTile(const safe_VkRenderPassCreateInfo2& createInfo, uint32_t attachment) {
    for (uint32_t subpass = 0; subpass < createInfo.subpassCount; subpass++) {
        const auto& subpass_info = createInfo.pSubpasses[subpass];

        // If an attachment is ever used as a color attachment,
        // resolve attachment or depth stencil attachment,
        // it needs to exist on tile at some point.

        for (uint32_t i = 0; i < subpass_info.colorAttachmentCount; i++) {
            if (subpass_info.pColorAttachments[i].attachment == attachment) return true;
        }

        if (subpass_info.pResolveAttachments) {
            for (uint32_t i = 0; i < subpass_info.colorAttachmentCount; i++) {
                if (subpass_info.pResolveAttachments[i].attachment == attachment) return true;
            }
        }

        if (subpass_info.pDepthStencilAttachment && subpass_info.pDepthStencilAttachment->attachment == attachment) return true;
    }

    return false;
}

static inline bool RenderPassUsesAttachmentAsImageOnly(const safe_VkRenderPassCreateInfo2& createInfo, uint32_t attachment) {
    if (RenderPassUsesAttachmentOnTile(createInfo, attachment)) {
        return false;
    }

    for (uint32_t subpass = 0; subpass < createInfo.subpassCount; subpass++) {
        const auto& subpassInfo = createInfo.pSubpasses[subpass];

        for (uint32_t i = 0; i < subpassInfo.inputAttachmentCount; i++) {
            if (subpassInfo.pInputAttachments[i].attachment == attachment) {
                return true;
            }
        }
    }

    return false;
}

bool BestPractices::ValidateCmdBeginRenderPass(VkCommandBuffer commandBuffer, RenderPassCreateVersion rp_version,
                                               const VkRenderPassBeginInfo* pRenderPassBegin) const {
    bool skip = false;

    if (!pRenderPassBegin) {
        return skip;
    }

    if (pRenderPassBegin->renderArea.extent.width == 0 || pRenderPassBegin->renderArea.extent.height == 0) {
        skip |= LogWarning(device, kVUID_BestPractices_BeginRenderPass_ZeroSizeRenderArea,
                           "This render pass has a zero-size render area. It cannot write to any attachments, "
                           "and can only be used for side effects such as layout transitions.");
    }

    auto rp_state = Get<RENDER_PASS_STATE>(pRenderPassBegin->renderPass);
    if (rp_state) {
        if (rp_state->createInfo.flags & VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT) {
            const VkRenderPassAttachmentBeginInfo* rpabi = LvlFindInChain<VkRenderPassAttachmentBeginInfo>(pRenderPassBegin->pNext);
            if (rpabi) {
                skip = ValidateAttachments(rp_state->createInfo.ptr(), rpabi->attachmentCount, rpabi->pAttachments);
            }
        }
        // Check if any attachments have LOAD operation on them
        for (uint32_t att = 0; att < rp_state->createInfo.attachmentCount; att++) {
            const auto& attachment = rp_state->createInfo.pAttachments[att];

            bool attachment_has_readback = false;
            if (!FormatIsStencilOnly(attachment.format) && attachment.loadOp == VK_ATTACHMENT_LOAD_OP_LOAD) {
                attachment_has_readback = true;
            }

            if (FormatHasStencil(attachment.format) && attachment.stencilLoadOp == VK_ATTACHMENT_LOAD_OP_LOAD) {
                attachment_has_readback = true;
            }

            bool attachment_needs_readback = false;

            // Check if the attachment is actually used in any subpass on-tile
            if (attachment_has_readback && RenderPassUsesAttachmentOnTile(rp_state->createInfo, att)) {
                attachment_needs_readback = true;
            }

            // Using LOAD_OP_LOAD is expensive on tiled GPUs, so flag it as a potential improvement
            if (attachment_needs_readback && (VendorCheckEnabled(kBPVendorArm) || VendorCheckEnabled(kBPVendorIMG))) {
                skip |=
                    LogPerformanceWarning(device, kVUID_BestPractices_BeginRenderPass_AttachmentNeedsReadback,
                                          "%s %s: Attachment #%u in render pass has begun with VK_ATTACHMENT_LOAD_OP_LOAD.\n"
                                          "Submitting this renderpass will cause the driver to inject a readback of the attachment "
                                          "which will copy in total %u pixels (renderArea = "
                                          "{ %" PRId32 ", %" PRId32 ", %" PRIu32 ", %" PRIu32 " }) to the tile buffer.",
                                          VendorSpecificTag(kBPVendorArm), VendorSpecificTag(kBPVendorIMG), att,
                                          pRenderPassBegin->renderArea.extent.width * pRenderPassBegin->renderArea.extent.height,
                                          pRenderPassBegin->renderArea.offset.x, pRenderPassBegin->renderArea.offset.y,
                                          pRenderPassBegin->renderArea.extent.width, pRenderPassBegin->renderArea.extent.height);
            }
        }

        // Check if renderpass has at least one VK_ATTACHMENT_LOAD_OP_CLEAR

        bool clearing = false;

        for (uint32_t att = 0; att < rp_state->createInfo.attachmentCount; att++) {
            const auto& attachment = rp_state->createInfo.pAttachments[att];

            if (attachment.loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR) {
                clearing = true;
                break;
            }
        }

        // Check if there are ClearValues passed to BeginRenderPass even though no attachments will be cleared
        if (!clearing && pRenderPassBegin->clearValueCount > 0) {
            // Flag as warning because nothing will happen per spec, and pClearValues will be ignored
            skip |= LogWarning(
                device, kVUID_BestPractices_ClearValueWithoutLoadOpClear,
                "This render pass does not have VkRenderPassCreateInfo.pAttachments->loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR "
                "but VkRenderPassBeginInfo.clearValueCount > 0. VkRenderPassBeginInfo.pClearValues will be ignored and no "
                "attachments will be cleared.");
        }

        // Check if there are more clearValues than attachments
        if (pRenderPassBegin->clearValueCount > rp_state->createInfo.attachmentCount) {
            // Flag as warning because the overflowing clearValues will be ignored and could even be undefined on certain platforms.
            // This could signal a bug and there seems to be no reason for this to happen on purpose.
            skip |=
                LogWarning(device, kVUID_BestPractices_ClearValueCountHigherThanAttachmentCount,
                           "This render pass has VkRenderPassBeginInfo.clearValueCount > VkRenderPassCreateInfo.attachmentCount "
                           "(%" PRIu32 " > %" PRIu32
                           ") and as such the clearValues that do not have a corresponding attachment will be ignored.",
                           pRenderPassBegin->clearValueCount, rp_state->createInfo.attachmentCount);
        }

        if (VendorCheckEnabled(kBPVendorNVIDIA) && rp_state->createInfo.pAttachments) {
            for (uint32_t i = 0; i < pRenderPassBegin->clearValueCount; ++i) {
                const auto& attachment = rp_state->createInfo.pAttachments[i];
                if (attachment.loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR) {
                    const auto& clear_color = pRenderPassBegin->pClearValues[i].color;
                    skip |= ValidateClearColor(commandBuffer, attachment.format, clear_color);
                }
            }
        }
    }

    return skip;
}

bool BestPractices::ValidateCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo) const {
    bool skip = false;

    auto cmd_state = Get<bp_state::CommandBuffer>(commandBuffer);
    assert(cmd_state);

    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        for (uint32_t i = 0; i < pRenderingInfo->colorAttachmentCount; ++i) {
            const auto& color_attachment = pRenderingInfo->pColorAttachments[i];
            if (color_attachment.loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR) {
                const VkFormat format = Get<IMAGE_VIEW_STATE>(color_attachment.imageView)->create_info.format;
                skip |= ValidateClearColor(commandBuffer, format, color_attachment.clearValue.color);
            }
        }
    }

    return skip;
}

void BestPractices::QueueValidateImageView(QueueCallbacks& funcs, const char* function_name, IMAGE_VIEW_STATE* view,
                                           IMAGE_SUBRESOURCE_USAGE_BP usage) {
    if (view) {
        auto image_state = std::static_pointer_cast<bp_state::Image>(view->image_state);
        QueueValidateImage(funcs, function_name, image_state, usage, view->normalized_subresource_range);
    }
}

void BestPractices::QueueValidateImage(QueueCallbacks& funcs, const char* function_name, std::shared_ptr<bp_state::Image>& state,
                                       IMAGE_SUBRESOURCE_USAGE_BP usage, const VkImageSubresourceRange& subresource_range) {
    // If we're viewing a 3D slice, ignore base array layer.
    // The entire 3D subresource is accessed as one atomic unit.
    const uint32_t base_array_layer = state->createInfo.imageType == VK_IMAGE_TYPE_3D ? 0 : subresource_range.baseArrayLayer;

    const uint32_t max_layers = state->createInfo.arrayLayers - base_array_layer;
    const uint32_t array_layers = std::min(subresource_range.layerCount, max_layers);
    const uint32_t max_levels = state->createInfo.mipLevels - subresource_range.baseMipLevel;
    const uint32_t mip_levels = std::min(state->createInfo.mipLevels, max_levels);

    for (uint32_t layer = 0; layer < array_layers; layer++) {
        for (uint32_t level = 0; level < mip_levels; level++) {
            QueueValidateImage(funcs, function_name, state, usage, layer + base_array_layer,
                               level + subresource_range.baseMipLevel);
        }
    }
}

void BestPractices::QueueValidateImage(QueueCallbacks& funcs, const char* function_name, std::shared_ptr<bp_state::Image>& state,
                                       IMAGE_SUBRESOURCE_USAGE_BP usage, const VkImageSubresourceLayers& subresource_layers) {
    const uint32_t max_layers = state->createInfo.arrayLayers - subresource_layers.baseArrayLayer;
    const uint32_t array_layers = std::min(subresource_layers.layerCount, max_layers);

    for (uint32_t layer = 0; layer < array_layers; layer++) {
        QueueValidateImage(funcs, function_name, state, usage, layer + subresource_layers.baseArrayLayer,
                           subresource_layers.mipLevel);
    }
}

void BestPractices::QueueValidateImage(QueueCallbacks& funcs, const char* function_name, std::shared_ptr<bp_state::Image>& state,
                                       IMAGE_SUBRESOURCE_USAGE_BP usage, uint32_t array_layer, uint32_t mip_level) {
    funcs.push_back([this, function_name, state, usage, array_layer, mip_level](
                        const ValidationStateTracker& vst, const QUEUE_STATE& qs, const CMD_BUFFER_STATE& cbs) -> bool {
        ValidateImageInQueue(qs, cbs, function_name, *state, usage, array_layer, mip_level);
        return false;
    });
}

void BestPractices::ValidateImageInQueueArmImg(const char* function_name, const bp_state::Image& image,
                                               IMAGE_SUBRESOURCE_USAGE_BP last_usage, IMAGE_SUBRESOURCE_USAGE_BP usage,
                                               uint32_t array_layer, uint32_t mip_level) {
    // Swapchain images are implicitly read so clear after store is expected.
    if (usage == IMAGE_SUBRESOURCE_USAGE_BP::RENDER_PASS_CLEARED && last_usage == IMAGE_SUBRESOURCE_USAGE_BP::RENDER_PASS_STORED &&
        !image.IsSwapchainImage()) {
        LogPerformanceWarning(
            device, kVUID_BestPractices_RenderPass_RedundantStore,
            "%s %s: %s Subresource (arrayLayer: %u, mipLevel: %u) of image was cleared as part of LOAD_OP_CLEAR, but last time "
            "image was used, it was written to with STORE_OP_STORE. "
            "Storing to the image is probably redundant in this case, and wastes bandwidth on tile-based "
            "architectures.",
            function_name, VendorSpecificTag(kBPVendorArm), VendorSpecificTag(kBPVendorIMG), array_layer, mip_level);
    } else if (usage == IMAGE_SUBRESOURCE_USAGE_BP::RENDER_PASS_CLEARED && last_usage == IMAGE_SUBRESOURCE_USAGE_BP::CLEARED) {
        LogPerformanceWarning(
            device, kVUID_BestPractices_RenderPass_RedundantClear,
            "%s %s: %s Subresource (arrayLayer: %u, mipLevel: %u) of image was cleared as part of LOAD_OP_CLEAR, but last time "
            "image was used, it was written to with vkCmdClear*Image(). "
            "Clearing the image with vkCmdClear*Image() is probably redundant in this case, and wastes bandwidth on "
            "tile-based architectures.",
            function_name, VendorSpecificTag(kBPVendorArm), VendorSpecificTag(kBPVendorIMG), array_layer, mip_level);
    } else if (usage == IMAGE_SUBRESOURCE_USAGE_BP::RENDER_PASS_READ_TO_TILE &&
               (last_usage == IMAGE_SUBRESOURCE_USAGE_BP::BLIT_WRITE || last_usage == IMAGE_SUBRESOURCE_USAGE_BP::CLEARED ||
                last_usage == IMAGE_SUBRESOURCE_USAGE_BP::COPY_WRITE || last_usage == IMAGE_SUBRESOURCE_USAGE_BP::RESOLVE_WRITE)) {
        const char* last_cmd = nullptr;
        const char* vuid = nullptr;
        const char* suggestion = nullptr;

        switch (last_usage) {
            case IMAGE_SUBRESOURCE_USAGE_BP::BLIT_WRITE:
                vuid = kVUID_BestPractices_RenderPass_BlitImage_LoadOpLoad;
                last_cmd = "vkCmdBlitImage";
                suggestion =
                    "The blit is probably redundant in this case, and wastes bandwidth on tile-based architectures. "
                    "Rather than blitting, just render the source image in a fragment shader in this render pass, "
                    "which avoids the memory roundtrip.";
                break;
            case IMAGE_SUBRESOURCE_USAGE_BP::CLEARED:
                vuid = kVUID_BestPractices_RenderPass_InefficientClear;
                last_cmd = "vkCmdClear*Image";
                suggestion =
                    "Clearing the image with vkCmdClear*Image() is probably redundant in this case, and wastes bandwidth on "
                    "tile-based architectures. "
                    "Use LOAD_OP_CLEAR instead to clear the image for free.";
                break;
            case IMAGE_SUBRESOURCE_USAGE_BP::COPY_WRITE:
                vuid = kVUID_BestPractices_RenderPass_CopyImage_LoadOpLoad;
                last_cmd = "vkCmdCopy*Image";
                suggestion =
                    "The copy is probably redundant in this case, and wastes bandwidth on tile-based architectures. "
                    "Rather than copying, just render the source image in a fragment shader in this render pass, "
                    "which avoids the memory roundtrip.";
                break;
            case IMAGE_SUBRESOURCE_USAGE_BP::RESOLVE_WRITE:
                vuid = kVUID_BestPractices_RenderPass_ResolveImage_LoadOpLoad;
                last_cmd = "vkCmdResolveImage";
                suggestion =
                    "The resolve is probably redundant in this case, and wastes a lot of bandwidth on tile-based architectures. "
                    "Rather than resolving, and then loading, try to keep rendering in the same render pass, "
                    "which avoids the memory roundtrip.";
                break;
            default:
                break;
        }

        LogPerformanceWarning(
            device, vuid,
            "%s %s: %s Subresource (arrayLayer: %u, mipLevel: %u) of image was loaded to tile as part of LOAD_OP_LOAD, but last "
            "time image was used, it was written to with %s. %s",
            function_name, VendorSpecificTag(kBPVendorArm), VendorSpecificTag(kBPVendorIMG), array_layer, mip_level, last_cmd,
            suggestion);
    }
}

void BestPractices::ValidateImageInQueue(const QUEUE_STATE& qs, const CMD_BUFFER_STATE& cbs, const char* function_name,
                                         bp_state::Image& state, IMAGE_SUBRESOURCE_USAGE_BP usage, uint32_t array_layer,
                                         uint32_t mip_level) {
    auto queue_family = qs.queueFamilyIndex;
    auto last_usage = state.UpdateUsage(array_layer, mip_level, usage, queue_family);

    // Concurrent sharing usage of image with exclusive sharing mode
    if (state.createInfo.sharingMode == VK_SHARING_MODE_EXCLUSIVE && last_usage.queue_family_index != queue_family) {
        // if UNDEFINED then first use/acquisition of subresource
        if (last_usage.type != IMAGE_SUBRESOURCE_USAGE_BP::UNDEFINED) {
            // If usage might read from the subresource, as contents are undefined
            // so write only is fine
            if (usage == IMAGE_SUBRESOURCE_USAGE_BP::RENDER_PASS_READ_TO_TILE || usage == IMAGE_SUBRESOURCE_USAGE_BP::BLIT_READ ||
                usage == IMAGE_SUBRESOURCE_USAGE_BP::COPY_READ || usage == IMAGE_SUBRESOURCE_USAGE_BP::DESCRIPTOR_ACCESS ||
                usage == IMAGE_SUBRESOURCE_USAGE_BP::RESOLVE_READ) {
                LogWarning(
                    state.image(), kVUID_BestPractices_ConcurrentUsageOfExclusiveImage,
                    "%s : Subresource (arrayLayer: %" PRIu32 ", mipLevel: %" PRIu32
                    ") of image is used on queue family index %" PRIu32
                    " after being used on "
                    "queue family index %" PRIu32
                    ", "
                    "but has VK_SHARING_MODE_EXCLUSIVE, and has not been acquired and released with a ownership transfer operation",
                    function_name, array_layer, mip_level, queue_family, last_usage.queue_family_index);
            }
        }
    }

    // When image was discarded with StoreOpDontCare but is now being read with LoadOpLoad
    if (last_usage.type == IMAGE_SUBRESOURCE_USAGE_BP::RENDER_PASS_DISCARDED &&
        usage == IMAGE_SUBRESOURCE_USAGE_BP::RENDER_PASS_READ_TO_TILE) {
        LogWarning(device, kVUID_BestPractices_StoreOpDontCareThenLoadOpLoad,
                   "Trying to load an attachment with LOAD_OP_LOAD that was previously stored with STORE_OP_DONT_CARE. This may "
                   "result in undefined behaviour.");
    }

    if (VendorCheckEnabled(kBPVendorArm) || VendorCheckEnabled(kBPVendorIMG)) {
        ValidateImageInQueueArmImg(function_name, state, last_usage.type, usage, array_layer, mip_level);
    }
}

void BestPractices::AddDeferredQueueOperations(bp_state::CommandBuffer& cb) {
    cb.queue_submit_functions.insert(cb.queue_submit_functions.end(), cb.queue_submit_functions_after_render_pass.begin(),
                                     cb.queue_submit_functions_after_render_pass.end());
    cb.queue_submit_functions_after_render_pass.clear();
}

void BestPractices::PreCallRecordCmdEndRenderPass(VkCommandBuffer commandBuffer) {
    RecordCmdEndRenderingCommon(commandBuffer);

    ValidationStateTracker::PreCallRecordCmdEndRenderPass(commandBuffer);
    auto cb_node = GetWrite<bp_state::CommandBuffer>(commandBuffer);
    if (cb_node) {
        AddDeferredQueueOperations(*cb_node);
    }
}

void BestPractices::PreCallRecordCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassInfo) {
    RecordCmdEndRenderingCommon(commandBuffer);

    ValidationStateTracker::PreCallRecordCmdEndRenderPass2(commandBuffer, pSubpassInfo);
    auto cb_node = GetWrite<bp_state::CommandBuffer>(commandBuffer);
    if (cb_node) {
        AddDeferredQueueOperations(*cb_node);
    }
}

void BestPractices::PreCallRecordCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfoKHR* pSubpassInfo) {
    RecordCmdEndRenderingCommon(commandBuffer);

    ValidationStateTracker::PreCallRecordCmdEndRenderPass2KHR(commandBuffer, pSubpassInfo);
    auto cb_node = GetWrite<bp_state::CommandBuffer>(commandBuffer);
    if (cb_node) {
        AddDeferredQueueOperations(*cb_node);
    }
}

void BestPractices::PreCallRecordCmdEndRendering(VkCommandBuffer commandBuffer) {
    RecordCmdEndRenderingCommon(commandBuffer);

    ValidationStateTracker::PreCallRecordCmdEndRendering(commandBuffer);
}

void BestPractices::PreCallRecordCmdEndRenderingKHR(VkCommandBuffer commandBuffer) {
    RecordCmdEndRenderingCommon(commandBuffer);

    ValidationStateTracker::PreCallRecordCmdEndRenderingKHR(commandBuffer);
}

void BestPractices::PreCallRecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                                    VkSubpassContents contents) {
    ValidationStateTracker::PreCallRecordCmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);
    RecordCmdBeginRenderingCommon(commandBuffer);
    RecordCmdBeginRenderPass(commandBuffer, pRenderPassBegin);
}

void BestPractices::PreCallRecordCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                                     const VkSubpassBeginInfo* pSubpassBeginInfo) {
    ValidationStateTracker::PreCallRecordCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
    RecordCmdBeginRenderingCommon(commandBuffer);
    RecordCmdBeginRenderPass(commandBuffer, pRenderPassBegin);
}

void BestPractices::PreCallRecordCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer,
                                                        const VkRenderPassBeginInfo* pRenderPassBegin,
                                                        const VkSubpassBeginInfo* pSubpassBeginInfo) {
    ValidationStateTracker::PreCallRecordCmdBeginRenderPass2KHR(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
    RecordCmdBeginRenderingCommon(commandBuffer);
    RecordCmdBeginRenderPass(commandBuffer, pRenderPassBegin);
}

void BestPractices::PreCallRecordCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo) {
    ValidationStateTracker::PreCallRecordCmdBeginRendering(commandBuffer, pRenderingInfo);
    RecordCmdBeginRenderingCommon(commandBuffer);
}

void BestPractices::PreCallRecordCmdBeginRenderingKHR(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo) {
    ValidationStateTracker::PreCallRecordCmdBeginRenderingKHR(commandBuffer, pRenderingInfo);
    RecordCmdBeginRenderingCommon(commandBuffer);
}

void BestPractices::PostCallRecordCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents) {
    ValidationStateTracker::PostCallRecordCmdNextSubpass(commandBuffer, contents);

    auto cmd_state = GetWrite<bp_state::CommandBuffer>(commandBuffer);
    auto rp = cmd_state->activeRenderPass.get();
    assert(rp);

    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        IMAGE_VIEW_STATE* depth_image_view = nullptr;

        const auto depth_attachment = rp->createInfo.pSubpasses[cmd_state->activeSubpass].pDepthStencilAttachment;
        if (depth_attachment) {
            const uint32_t attachment_index = depth_attachment->attachment;
            if (attachment_index != VK_ATTACHMENT_UNUSED) {
                depth_image_view = (*cmd_state->active_attachments)[attachment_index];
            }
        }
        if (depth_image_view && (depth_image_view->create_info.subresourceRange.aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT) != 0U) {
            const VkImage depth_image = depth_image_view->image_state->image();
            const VkImageSubresourceRange& subresource_range = depth_image_view->create_info.subresourceRange;
            RecordBindZcullScope(*cmd_state, depth_image, subresource_range);
        } else {
            RecordUnbindZcullScope(*cmd_state);
        }
    }
}

void BestPractices::RecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin) {
    if (!pRenderPassBegin) {
        return;
    }

    auto cb = GetWrite<bp_state::CommandBuffer>(commandBuffer);

    auto rp_state = Get<RENDER_PASS_STATE>(pRenderPassBegin->renderPass);
    if (rp_state) {
        // Check load ops
        for (uint32_t att = 0; att < rp_state->createInfo.attachmentCount; att++) {
            const auto& attachment = rp_state->createInfo.pAttachments[att];

            if (!RenderPassUsesAttachmentAsImageOnly(rp_state->createInfo, att) &&
                !RenderPassUsesAttachmentOnTile(rp_state->createInfo, att)) {
                continue;
            }

            // If renderpass doesn't load attachment, no need to validate image in queue
            if ((!FormatIsStencilOnly(attachment.format) && attachment.loadOp == VK_ATTACHMENT_LOAD_OP_NONE_EXT) ||
                (FormatHasStencil(attachment.format) && attachment.stencilLoadOp == VK_ATTACHMENT_LOAD_OP_NONE_EXT)) {
                continue;
            }

            IMAGE_SUBRESOURCE_USAGE_BP usage = IMAGE_SUBRESOURCE_USAGE_BP::UNDEFINED;

            if ((!FormatIsStencilOnly(attachment.format) && attachment.loadOp == VK_ATTACHMENT_LOAD_OP_LOAD) ||
                (FormatHasStencil(attachment.format) && attachment.stencilLoadOp == VK_ATTACHMENT_LOAD_OP_LOAD)) {
                usage = IMAGE_SUBRESOURCE_USAGE_BP::RENDER_PASS_READ_TO_TILE;
            } else if ((!FormatIsStencilOnly(attachment.format) && attachment.loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR) ||
                       (FormatHasStencil(attachment.format) && attachment.stencilLoadOp == VK_ATTACHMENT_LOAD_OP_CLEAR)) {
                usage = IMAGE_SUBRESOURCE_USAGE_BP::RENDER_PASS_CLEARED;
            } else if (RenderPassUsesAttachmentAsImageOnly(rp_state->createInfo, att)) {
                usage = IMAGE_SUBRESOURCE_USAGE_BP::DESCRIPTOR_ACCESS;
            }

            auto framebuffer = Get<FRAMEBUFFER_STATE>(pRenderPassBegin->framebuffer);
            std::shared_ptr<IMAGE_VIEW_STATE> image_view = nullptr;

            if (framebuffer->createInfo.flags & VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT) {
                const VkRenderPassAttachmentBeginInfo* rpabi =
                    LvlFindInChain<VkRenderPassAttachmentBeginInfo>(pRenderPassBegin->pNext);
                if (rpabi) {
                    image_view = Get<IMAGE_VIEW_STATE>(rpabi->pAttachments[att]);
                }
            } else {
                image_view = Get<IMAGE_VIEW_STATE>(framebuffer->createInfo.pAttachments[att]);
            }

            QueueValidateImageView(cb->queue_submit_functions, "vkCmdBeginRenderPass()", image_view.get(), usage);
        }

        // Check store ops
        for (uint32_t att = 0; att < rp_state->createInfo.attachmentCount; att++) {
            const auto& attachment = rp_state->createInfo.pAttachments[att];

            if (!RenderPassUsesAttachmentOnTile(rp_state->createInfo, att)) {
                continue;
            }

            // If renderpass doesn't store attachment, no need to validate image in queue
            if ((!FormatIsStencilOnly(attachment.format) && attachment.storeOp == VK_ATTACHMENT_STORE_OP_NONE) ||
                (FormatHasStencil(attachment.format) && attachment.stencilStoreOp == VK_ATTACHMENT_STORE_OP_NONE)) {
                continue;
            }

            IMAGE_SUBRESOURCE_USAGE_BP usage = IMAGE_SUBRESOURCE_USAGE_BP::RENDER_PASS_DISCARDED;

            if ((!FormatIsStencilOnly(attachment.format) && attachment.storeOp == VK_ATTACHMENT_STORE_OP_STORE) ||
                (FormatHasStencil(attachment.format) && attachment.stencilStoreOp == VK_ATTACHMENT_STORE_OP_STORE)) {
                usage = IMAGE_SUBRESOURCE_USAGE_BP::RENDER_PASS_STORED;
            }

            auto framebuffer = Get<FRAMEBUFFER_STATE>(pRenderPassBegin->framebuffer);

            std::shared_ptr<IMAGE_VIEW_STATE> image_view;
            if (framebuffer->createInfo.flags & VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT) {
                const VkRenderPassAttachmentBeginInfo* rpabi =
                    LvlFindInChain<VkRenderPassAttachmentBeginInfo>(pRenderPassBegin->pNext);
                if (rpabi) {
                    image_view = Get<IMAGE_VIEW_STATE>(rpabi->pAttachments[att]);
                }
            } else {
                image_view = Get<IMAGE_VIEW_STATE>(framebuffer->createInfo.pAttachments[att]);
            }

            QueueValidateImageView(cb->queue_submit_functions_after_render_pass, "vkCmdEndRenderPass()", image_view.get(), usage);
        }
    }
}

bool BestPractices::PreCallValidateCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                                      VkSubpassContents contents) const {
    bool skip = StateTracker::PreCallValidateCmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);
    skip |= ValidateCmdBeginRenderPass(commandBuffer, RENDER_PASS_VERSION_1, pRenderPassBegin);
    return skip;
}

bool BestPractices::PreCallValidateCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer,
                                                          const VkRenderPassBeginInfo* pRenderPassBegin,
                                                          const VkSubpassBeginInfo* pSubpassBeginInfo) const {
    bool skip = StateTracker::PreCallValidateCmdBeginRenderPass2KHR(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
    skip |= ValidateCmdBeginRenderPass(commandBuffer, RENDER_PASS_VERSION_2, pRenderPassBegin);
    return skip;
}

bool BestPractices::PreCallValidateCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                                       const VkSubpassBeginInfo* pSubpassBeginInfo) const {
    bool skip = StateTracker::PreCallValidateCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
    skip |= ValidateCmdBeginRenderPass(commandBuffer, RENDER_PASS_VERSION_2, pRenderPassBegin);
    return skip;
}

bool BestPractices::PreCallValidateCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo) const {
    bool skip = StateTracker::PreCallValidateCmdBeginRendering(commandBuffer, pRenderingInfo);
    skip |= ValidateCmdBeginRendering(commandBuffer, pRenderingInfo);
    return skip;
}

bool BestPractices::PreCallValidateCmdBeginRenderingKHR(VkCommandBuffer commandBuffer,
                                                        const VkRenderingInfo* pRenderingInfo) const {
    bool skip = StateTracker::PreCallValidateCmdBeginRenderingKHR(commandBuffer, pRenderingInfo);
    skip |= ValidateCmdBeginRendering(commandBuffer, pRenderingInfo);
    return skip;
}

void BestPractices::RecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, RenderPassCreateVersion rp_version,
                                             const VkRenderPassBeginInfo* pRenderPassBegin) {
    // Reset the renderpass state
    auto cb = GetWrite<bp_state::CommandBuffer>(commandBuffer);
    // TODO - move this logic to the Render Pass state as cb->has_draw_cmd should stay true for lifetime of command buffer
    cb->has_draw_cmd = false;
    assert(cb);
    auto& render_pass_state = cb->render_pass_state;
    render_pass_state.touchesAttachments.clear();
    render_pass_state.earlyClearAttachments.clear();
    render_pass_state.numDrawCallsDepthOnly = 0;
    render_pass_state.numDrawCallsDepthEqualCompare = 0;
    render_pass_state.colorAttachment = false;
    render_pass_state.depthAttachment = false;
    render_pass_state.drawTouchAttachments = true;
    // Don't reset state related to pipeline state.

    // Reset NV state
    cb->nv = {};

    auto rp_state = Get<RENDER_PASS_STATE>(pRenderPassBegin->renderPass);

    // track depth / color attachment usage within the renderpass
    for (size_t i = 0; i < rp_state->createInfo.subpassCount; i++) {
        // record if depth/color attachments are in use for this renderpass
        if (rp_state->createInfo.pSubpasses[i].pDepthStencilAttachment != nullptr) render_pass_state.depthAttachment = true;

        if (rp_state->createInfo.pSubpasses[i].colorAttachmentCount > 0) render_pass_state.colorAttachment = true;
    }
}

void BestPractices::PostCallRecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                                     VkSubpassContents contents) {
    StateTracker::PostCallRecordCmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);
    RecordCmdBeginRenderPass(commandBuffer, RENDER_PASS_VERSION_1, pRenderPassBegin);
}

void BestPractices::PostCallRecordCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                                      const VkSubpassBeginInfo* pSubpassBeginInfo) {
    StateTracker::PostCallRecordCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
    RecordCmdBeginRenderPass(commandBuffer, RENDER_PASS_VERSION_2, pRenderPassBegin);
}

void BestPractices::PostCallRecordCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer,
                                                         const VkRenderPassBeginInfo* pRenderPassBegin,
                                                         const VkSubpassBeginInfo* pSubpassBeginInfo) {
    StateTracker::PostCallRecordCmdBeginRenderPass2KHR(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
    RecordCmdBeginRenderPass(commandBuffer, RENDER_PASS_VERSION_2, pRenderPassBegin);
}

// Generic function to handle validation for all CmdDraw* type functions
bool BestPractices::ValidateCmdDrawType(VkCommandBuffer cmd_buffer, const char* caller) const {
    bool skip = false;
    const auto cb_state = GetRead<bp_state::CommandBuffer>(cmd_buffer);
    if (cb_state) {
        const auto lv_bind_point = ConvertToLvlBindPoint(VK_PIPELINE_BIND_POINT_GRAPHICS);
        const auto* pipeline_state = cb_state->lastBound[lv_bind_point].pipeline_state;
        const auto& current_vtx_bfr_binding_info = cb_state->current_vertex_buffer_binding_info.vertex_buffer_bindings;

        // Verify vertex binding
        if (pipeline_state && pipeline_state->vertex_input_state &&
            pipeline_state->vertex_input_state->binding_descriptions.size() <= 0) {
            if ((!current_vtx_bfr_binding_info.empty()) && (!cb_state->vertex_buffer_used)) {
                skip |= LogPerformanceWarning(cb_state->commandBuffer(), kVUID_BestPractices_DrawState_VtxIndexOutOfBounds,
                                              "Vertex buffers are bound to %s but no vertex buffers are attached to %s.",
                                              report_data->FormatHandle(cb_state->commandBuffer()).c_str(),
                                              report_data->FormatHandle(pipeline_state->pipeline()).c_str());
            }
        }

        const auto* pipe = cb_state->GetCurrentPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS);
        if (pipe) {
            const auto& rp_state = pipe->RenderPassState();
            if (rp_state) {
                for (uint32_t i = 0; i < rp_state->createInfo.subpassCount; ++i) {
                    const auto& subpass = rp_state->createInfo.pSubpasses[i];
                    const auto* ds_state = pipe->DepthStencilState();
                    const uint32_t depth_stencil_attachment =
                        GetSubpassDepthStencilAttachmentIndex(ds_state, subpass.pDepthStencilAttachment);
                    const auto* raster_state = pipe->RasterizationState();
                    if ((depth_stencil_attachment == VK_ATTACHMENT_UNUSED) && raster_state &&
                        raster_state->depthBiasEnable == VK_TRUE) {
                        skip |= LogWarning(cb_state->commandBuffer(), kVUID_BestPractices_DepthBiasNoAttachment,
                                           "%s: depthBiasEnable == VK_TRUE without a depth-stencil attachment.", caller);
                    }
                }
            }
        }
    }
    return skip;
}

void BestPractices::RecordCmdDrawType(VkCommandBuffer cmd_buffer, uint32_t draw_count, const char* caller) {
    auto cb_node = GetWrite<bp_state::CommandBuffer>(cmd_buffer);
    assert(cb_node);
    if (VendorCheckEnabled(kBPVendorArm)) {
        RecordCmdDrawTypeArm(*cb_node, draw_count, caller);
    }
    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        RecordCmdDrawTypeNVIDIA(*cb_node);
    }

    if (cb_node->render_pass_state.drawTouchAttachments) {
        for (auto& touch : cb_node->render_pass_state.nextDrawTouchesAttachments) {
            RecordAttachmentAccess(*cb_node, touch.framebufferAttachment, touch.aspects);
        }
        // No need to touch the same attachments over and over.
        cb_node->render_pass_state.drawTouchAttachments = false;
    }
}

void BestPractices::RecordCmdDrawTypeArm(bp_state::CommandBuffer& cb_node, uint32_t draw_count, const char* caller) {
    auto& render_pass_state = cb_node.render_pass_state;
    // Each TBDR vendor requires a depth pre-pass draw call to have a minimum number of vertices/indices before it counts towards
    // depth prepass warnings First find the lowest enabled draw count
    uint32_t lowestEnabledMinDrawCount = 0;
    lowestEnabledMinDrawCount = VendorCheckEnabled(kBPVendorArm) * kDepthPrePassMinDrawCountArm;
    if (VendorCheckEnabled(kBPVendorIMG) && kDepthPrePassMinDrawCountIMG < lowestEnabledMinDrawCount)
        lowestEnabledMinDrawCount = kDepthPrePassMinDrawCountIMG;

    if (draw_count >= lowestEnabledMinDrawCount) {
        if (render_pass_state.depthOnly) render_pass_state.numDrawCallsDepthOnly++;
        if (render_pass_state.depthEqualComparison) render_pass_state.numDrawCallsDepthEqualCompare++;
    }
}

void BestPractices::RecordCmdDrawTypeNVIDIA(bp_state::CommandBuffer& cmd_state) {
    assert(VendorCheckEnabled(kBPVendorNVIDIA));

    if (cmd_state.nv.depth_test_enable && cmd_state.nv.zcull_direction != bp_state::CommandBufferStateNV::ZcullDirection::Unknown) {
        RecordSetScopeZcullDirection(cmd_state, cmd_state.nv.zcull_direction);
        RecordZcullDraw(cmd_state);
    }
}

bool BestPractices::PreCallValidateCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount,
                                           uint32_t firstVertex, uint32_t firstInstance) const {
    bool skip = false;

    if (instanceCount == 0) {
        skip |= LogWarning(device, kVUID_BestPractices_CmdDraw_InstanceCountZero,
                           "Warning: You are calling vkCmdDraw() with an instanceCount of Zero.");
    }
    skip |= ValidateCmdDrawType(commandBuffer, "vkCmdDraw()");

    return skip;
}

void BestPractices::PostCallRecordCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount,
                                          uint32_t firstVertex, uint32_t firstInstance) {
    StateTracker::PostCallRecordCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
    RecordCmdDrawType(commandBuffer, vertexCount * instanceCount, "vkCmdDraw()");
}

bool BestPractices::PreCallValidateCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                                  uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) const {
    bool skip = false;

    if (instanceCount == 0) {
        skip |= LogWarning(device, kVUID_BestPractices_CmdDraw_InstanceCountZero,
                           "Warning: You are calling vkCmdDrawIndexed() with an instanceCount of Zero.");
    }
    skip |= ValidateCmdDrawType(commandBuffer, "vkCmdDrawIndexed()");

    // Check if we reached the limit for small indexed draw calls.
    // Note that we cannot update the draw call count here, so we do it in PreCallRecordCmdDrawIndexed.
    const auto cmd_state = GetRead<bp_state::CommandBuffer>(commandBuffer);
    if ((indexCount * instanceCount) <= kSmallIndexedDrawcallIndices &&
        (cmd_state->small_indexed_draw_call_count == kMaxSmallIndexedDrawcalls - 1) &&
        (VendorCheckEnabled(kBPVendorArm) || VendorCheckEnabled(kBPVendorIMG))) {
        skip |= LogPerformanceWarning(device, kVUID_BestPractices_CmdDrawIndexed_ManySmallIndexedDrawcalls,
                                      "%s %s: The command buffer contains many small indexed drawcalls "
                                      "(at least %u drawcalls with less than %u indices each). This may cause pipeline bubbles. "
                                      "You can try batching drawcalls or instancing when applicable.",
                                      VendorSpecificTag(kBPVendorArm), VendorSpecificTag(kBPVendorIMG), kMaxSmallIndexedDrawcalls,
                                      kSmallIndexedDrawcallIndices);
    }

    if (VendorCheckEnabled(kBPVendorArm)) {
        ValidateIndexBufferArm(*cmd_state, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    }

    return skip;
}

bool BestPractices::ValidateIndexBufferArm(const bp_state::CommandBuffer& cmd_state, uint32_t indexCount, uint32_t instanceCount,
                                           uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) const {
    bool skip = false;

    // check for sparse/underutilised index buffer, and post-transform cache thrashing

    const auto* ib_state = cmd_state.index_buffer_binding.buffer_state.get();
    if (ib_state == nullptr || cmd_state.index_buffer_binding.buffer_state->Destroyed()) return skip;

    const VkIndexType ib_type = cmd_state.index_buffer_binding.index_type;
    const auto& ib_mem_state = *ib_state->MemState();
    const VkDeviceSize ib_mem_offset = ib_mem_state.mapped_range.offset;
    const void* ib_mem = ib_mem_state.p_driver_data;
    bool primitive_restart_enable = false;

    const auto lv_bind_point = ConvertToLvlBindPoint(VK_PIPELINE_BIND_POINT_GRAPHICS);
    const auto& last_bound = cmd_state.lastBound[lv_bind_point];
    const auto* pipeline_state = last_bound.pipeline_state;

    const auto* ia_state = pipeline_state ? pipeline_state->InputAssemblyState() : nullptr;
    if (ia_state) {
        primitive_restart_enable = ia_state->primitiveRestartEnable == VK_TRUE;
    }

    // no point checking index buffer if the memory is nonexistant/unmapped, or if there is no graphics pipeline bound to this CB
    if (ib_mem && last_bound.IsUsing()) {
        const uint32_t scan_stride = GetIndexAlignment(ib_type);
        const uint8_t* scan_begin = static_cast<const uint8_t*>(ib_mem) + ib_mem_offset + firstIndex * scan_stride;
        const uint8_t* scan_end = scan_begin + indexCount * scan_stride;

        // Min and max are important to track for some Mali architectures. In older Mali devices without IDVS, all
        // vertices corresponding to indices between the minimum and maximum may be loaded, and possibly shaded,
        // irrespective of whether or not they're part of the draw call.

        // start with minimum as 0xFFFFFFFF and adjust to indices in the buffer
        uint32_t min_index = ~0u;
        // start with maximum as 0 and adjust to indices in the buffer
        uint32_t max_index = 0u;

        // first scan-through, we're looking to simulate a model LRU post-transform cache, estimating the number of vertices shaded
        // for the given index buffer
        uint32_t vertex_shade_count = 0;

        PostTransformLRUCacheModel post_transform_cache;

        // The size of the cache being modelled positively correlates with how much behaviour it can capture about
        // arbitrary ground-truth hardware/architecture cache behaviour. I.e. it's a good solution when we don't know the
        // target architecture.
        // However, modelling a post-transform cache with more than 32 elements gives diminishing returns in practice.
        // http://eelpi.gotdns.org/papers/fast_vert_cache_opt.html
        post_transform_cache.resize(32);

        for (const uint8_t* scan_ptr = scan_begin; scan_ptr < scan_end; scan_ptr += scan_stride) {
            uint32_t scan_index;
            uint32_t primitive_restart_value;
            if (ib_type == VK_INDEX_TYPE_UINT8_EXT) {
                scan_index = *reinterpret_cast<const uint8_t*>(scan_ptr);
                primitive_restart_value = 0xFF;
            } else if (ib_type == VK_INDEX_TYPE_UINT16) {
                scan_index = *reinterpret_cast<const uint16_t*>(scan_ptr);
                primitive_restart_value = 0xFFFF;
            } else {
                scan_index = *reinterpret_cast<const uint32_t*>(scan_ptr);
                primitive_restart_value = 0xFFFFFFFF;
            }

            max_index = std::max(max_index, scan_index);
            min_index = std::min(min_index, scan_index);

            if (!primitive_restart_enable || scan_index != primitive_restart_value) {
                const bool in_cache = post_transform_cache.query_cache(scan_index);
                // if the shaded vertex corresponding to the index is not in the PT-cache, we need to shade again
                if (!in_cache) vertex_shade_count++;
            }
        }

        // if the max and min values were not set, then we either have no indices, or all primitive restarts, exit...
        // if the max and min are the same, then it implies all the indices are the same, then we don't need to do anything
        if (max_index < min_index || max_index == min_index) return skip;

        if (max_index - min_index >= indexCount) {
            skip |=
                LogPerformanceWarning(device, kVUID_BestPractices_CmdDrawIndexed_SparseIndexBuffer,
                                      "%s The indices which were specified for the draw call only utilise approximately %.02f%% of "
                                      "index buffer value range. Arm Mali architectures before G71 do not have IDVS (Index-Driven "
                                      "Vertex Shading), meaning all vertices corresponding to indices between the minimum and "
                                      "maximum would be loaded, and possibly shaded, whether or not they are used.",
                                      VendorSpecificTag(kBPVendorArm),
                                      (static_cast<float>(indexCount) / static_cast<float>(max_index - min_index)) * 100.0f);
            return skip;
        }

        // use a dynamic vector of bitsets as a memory-compact representation of which indices are included in the draw call
        // each bit of the n-th bucket contains the inclusion information for indices (n*n_buckets) to ((n+1)*n_buckets)
        const size_t refs_per_bucket = 64;
        std::vector<std::bitset<refs_per_bucket>> vertex_reference_buckets;

        const uint32_t n_indices = max_index - min_index + 1;
        const uint32_t n_buckets = (n_indices / static_cast<uint32_t>(refs_per_bucket)) +
                                   ((n_indices % static_cast<uint32_t>(refs_per_bucket)) != 0 ? 1 : 0);

        // there needs to be at least one bitset to store a set of indices smaller than n_buckets
        vertex_reference_buckets.resize(std::max(1u, n_buckets));

        // To avoid using too much memory, we run over the indices again.
        // Knowing the size from the last scan allows us to record index usage with bitsets
        for (const uint8_t* scan_ptr = scan_begin; scan_ptr < scan_end; scan_ptr += scan_stride) {
            uint32_t scan_index;
            if (ib_type == VK_INDEX_TYPE_UINT8_EXT) {
                scan_index = *reinterpret_cast<const uint8_t*>(scan_ptr);
            } else if (ib_type == VK_INDEX_TYPE_UINT16) {
                scan_index = *reinterpret_cast<const uint16_t*>(scan_ptr);
            } else {
                scan_index = *reinterpret_cast<const uint32_t*>(scan_ptr);
            }
            // keep track of the set of all indices used to reference vertices in the draw call
            size_t index_offset = scan_index - min_index;
            size_t bitset_bucket_index = index_offset / refs_per_bucket;
            uint64_t used_indices = 1ull << ((index_offset % refs_per_bucket) & 0xFFFFFFFFu);
            vertex_reference_buckets[bitset_bucket_index] |= used_indices;
        }

        uint32_t vertex_reference_count = 0;
        for (const auto& bitset : vertex_reference_buckets) {
            vertex_reference_count += static_cast<uint32_t>(bitset.count());
        }

        // low index buffer utilization implies that: of the vertices available to the draw call, not all are utilized
        float utilization = static_cast<float>(vertex_reference_count) / static_cast<float>(max_index - min_index + 1);
        // low hit rate (high miss rate) implies the order of indices in the draw call may be possible to improve
        float cache_hit_rate = static_cast<float>(vertex_reference_count) / static_cast<float>(vertex_shade_count);

        if (utilization < 0.5f) {
            skip |= LogPerformanceWarning(device, kVUID_BestPractices_CmdDrawIndexed_SparseIndexBuffer,
                                          "%s The indices which were specified for the draw call only utilise approximately "
                                          "%.02f%% of the bound vertex buffer.",
                                          VendorSpecificTag(kBPVendorArm), utilization);
        }

        if (cache_hit_rate <= 0.5f) {
            skip |=
                LogPerformanceWarning(device, kVUID_BestPractices_CmdDrawIndexed_PostTransformCacheThrashing,
                                      "%s The indices which were specified for the draw call are estimated to cause thrashing of "
                                      "the post-transform vertex cache, with a hit-rate of %.02f%%. "
                                      "I.e. the ordering of the index buffer may not make optimal use of indices associated with "
                                      "recently shaded vertices.",
                                      VendorSpecificTag(kBPVendorArm), cache_hit_rate * 100.0f);
        }
    }

    return skip;
}

bool BestPractices::PreCallValidateCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount,
                                                      const VkCommandBuffer* pCommandBuffers) const {
    bool skip = false;
    const auto primary = GetRead<bp_state::CommandBuffer>(commandBuffer);
    for (uint32_t i = 0; i < commandBufferCount; i++) {
        const auto secondary_cb = GetRead<bp_state::CommandBuffer>(pCommandBuffers[i]);
        if (secondary_cb == nullptr) {
            continue;
        }
        const auto& secondary = secondary_cb->render_pass_state;
        for (auto& clear : secondary.earlyClearAttachments) {
            if (ClearAttachmentsIsFullClear(*primary, uint32_t(clear.rects.size()), clear.rects.data())) {
                skip |= ValidateClearAttachment(*primary, clear.framebufferAttachment, clear.colorAttachment, clear.aspects, true);
            }
        }
    }

    if (VendorCheckEnabled(kBPVendorAMD)) {
        if (commandBufferCount > 0) {
            skip |= LogPerformanceWarning(device, kVUID_BestPractices_CmdBuffer_AvoidSecondaryCmdBuffers,
                                          "%s Performance warning: Use of secondary command buffers is not recommended. ",
                                          VendorSpecificTag(kBPVendorAMD));
        }
    }
    return skip;
}

void BestPractices::PreCallRecordCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount,
                                                    const VkCommandBuffer* pCommandBuffers) {
    ValidationStateTracker::PreCallRecordCmdExecuteCommands(commandBuffer, commandBufferCount, pCommandBuffers);

    auto primary = GetWrite<bp_state::CommandBuffer>(commandBuffer);
    if (!primary) {
        return;
    }

    for (uint32_t i = 0; i < commandBufferCount; i++) {
        auto secondary = GetWrite<bp_state::CommandBuffer>(pCommandBuffers[i]);
        if (!secondary) {
            continue;
        }

        for (auto& early_clear : secondary->render_pass_state.earlyClearAttachments) {
            if (ClearAttachmentsIsFullClear(*primary, uint32_t(early_clear.rects.size()), early_clear.rects.data())) {
                RecordAttachmentClearAttachments(*primary, early_clear.framebufferAttachment, early_clear.colorAttachment,
                                                 early_clear.aspects, uint32_t(early_clear.rects.size()), early_clear.rects.data());
            } else {
                RecordAttachmentAccess(*primary, early_clear.framebufferAttachment, early_clear.aspects);
            }
        }

        for (auto& touch : secondary->render_pass_state.touchesAttachments) {
            RecordAttachmentAccess(*primary, touch.framebufferAttachment, touch.aspects);
        }

        primary->render_pass_state.numDrawCallsDepthEqualCompare += secondary->render_pass_state.numDrawCallsDepthEqualCompare;
        primary->render_pass_state.numDrawCallsDepthOnly += secondary->render_pass_state.numDrawCallsDepthOnly;
    }
}

bool BestPractices::PreCallValidateCmdBuildAccelerationStructureNV(VkCommandBuffer commandBuffer,
                                                                   const VkAccelerationStructureInfoNV* pInfo,
                                                                   VkBuffer instanceData, VkDeviceSize instanceOffset,
                                                                   VkBool32 update, VkAccelerationStructureNV dst,
                                                                   VkAccelerationStructureNV src, VkBuffer scratch,
                                                                   VkDeviceSize scratchOffset) const {
    return ValidateBuildAccelerationStructure(commandBuffer);
}

bool BestPractices::PreCallValidateCmdBuildAccelerationStructuresIndirectKHR(
    VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkDeviceAddress* pIndirectDeviceAddresses, const uint32_t* pIndirectStrides,
    const uint32_t* const* ppMaxPrimitiveCounts) const {
    return ValidateBuildAccelerationStructure(commandBuffer);
}

bool BestPractices::PreCallValidateCmdBuildAccelerationStructuresKHR(
    VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos) const {
    return ValidateBuildAccelerationStructure(commandBuffer);
}

bool BestPractices::ValidateBuildAccelerationStructure(VkCommandBuffer commandBuffer) const {
    bool skip = false;
    auto cb_node = GetRead<bp_state::CommandBuffer>(commandBuffer);
    assert(cb_node);

    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        if ((cb_node->GetQueueFlags() & VK_QUEUE_GRAPHICS_BIT) != 0) {
            skip |= LogPerformanceWarning(commandBuffer, kVUID_BestPractices_AccelerationStructure_NotAsync,
                                          "%s Performance warning: Prefer building acceleration structures on an asynchronous "
                                          "compute queue, instead of using the universal graphics queue.",
                                          VendorSpecificTag(kBPVendorNVIDIA));
        }
    }

    return skip;
}

bool BestPractices::ValidateBindMemory(VkDevice device, VkDeviceMemory memory) const {
    bool skip = false;

    if (VendorCheckEnabled(kBPVendorNVIDIA) && IsExtEnabled(device_extensions.vk_ext_pageable_device_local_memory)) {
        auto mem_info = std::static_pointer_cast<const bp_state::DeviceMemory>(Get<DEVICE_MEMORY_STATE>(memory));
        if (!mem_info->dynamic_priority) {
            skip |=
                LogPerformanceWarning(device, kVUID_BestPractices_BindMemory_NoPriority,
                                      "%s Use vkSetDeviceMemoryPriorityEXT to provide the OS with information on which allocations "
                                      "should stay in memory and which should be demoted first when video memory is limited. The "
                                      "highest priority should be given to GPU-written resources like color attachments, depth "
                                      "attachments, storage images, and buffers written from the GPU.",
                                      VendorSpecificTag(kBPVendorNVIDIA));
        }
    }

    return skip;
}

void BestPractices::RecordAttachmentAccess(bp_state::CommandBuffer& cb_state, uint32_t fb_attachment, VkImageAspectFlags aspects) {
    auto& state = cb_state.render_pass_state;
    // Called when we have a partial clear attachment, or a normal draw call which accesses an attachment.
    auto itr =
        std::find_if(state.touchesAttachments.begin(), state.touchesAttachments.end(),
                     [fb_attachment](const bp_state::AttachmentInfo& info) { return info.framebufferAttachment == fb_attachment; });

    if (itr != state.touchesAttachments.end()) {
        itr->aspects |= aspects;
    } else {
        state.touchesAttachments.push_back({fb_attachment, aspects});
    }
}

void BestPractices::RecordAttachmentClearAttachments(bp_state::CommandBuffer& cmd_state, uint32_t fb_attachment,
                                                     uint32_t color_attachment, VkImageAspectFlags aspects, uint32_t rectCount,
                                                     const VkClearRect* pRects) {
    auto& state = cmd_state.render_pass_state;
    // If we observe a full clear before any other access to a frame buffer attachment,
    // we have candidate for redundant clear attachments.
    auto itr =
        std::find_if(state.touchesAttachments.begin(), state.touchesAttachments.end(),
                     [fb_attachment](const bp_state::AttachmentInfo& info) { return info.framebufferAttachment == fb_attachment; });

    uint32_t new_aspects = aspects;
    if (itr != state.touchesAttachments.end()) {
        new_aspects = aspects & ~itr->aspects;
        itr->aspects |= aspects;
    } else {
        state.touchesAttachments.push_back({fb_attachment, aspects});
    }

    if (new_aspects == 0) {
        return;
    }

    if (cmd_state.createInfo.level == VK_COMMAND_BUFFER_LEVEL_SECONDARY) {
        // The first command might be a clear, but might not be the first in the render pass, defer any checks until
        // CmdExecuteCommands.
        state.earlyClearAttachments.push_back(
            {fb_attachment, color_attachment, new_aspects, std::vector<VkClearRect>{pRects, pRects + rectCount}});
    }
}

void BestPractices::PreCallRecordCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                                     const VkClearAttachment* pClearAttachments, uint32_t rectCount,
                                                     const VkClearRect* pRects) {
    ValidationStateTracker::PreCallRecordCmdClearAttachments(commandBuffer, attachmentCount, pClearAttachments, rectCount, pRects);

    auto cmd_state = GetWrite<bp_state::CommandBuffer>(commandBuffer);
    auto* rp_state = cmd_state->activeRenderPass.get();
    auto* fb_state = cmd_state->activeFramebuffer.get();
    const bool is_secondary = cmd_state->createInfo.level == VK_COMMAND_BUFFER_LEVEL_SECONDARY;

    if (rectCount == 0 || !rp_state) {
        return;
    }

    if (!is_secondary && !fb_state && !rp_state->use_dynamic_rendering && !rp_state->use_dynamic_rendering_inherited) {
        return;
    }

    // If we have a rect which covers the entire frame buffer, we have a LOAD_OP_CLEAR-like command.
    const bool full_clear = ClearAttachmentsIsFullClear(*cmd_state, rectCount, pRects);

    if (rp_state->UsesDynamicRendering()) {
        if (VendorCheckEnabled(kBPVendorNVIDIA)) {
            auto pColorAttachments = rp_state->dynamic_rendering_begin_rendering_info.pColorAttachments;

            for (uint32_t i = 0; i < attachmentCount; i++) {
                auto& clear_attachment = pClearAttachments[i];

                if (clear_attachment.aspectMask & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) {
                    RecordResetScopeZcullDirection(*cmd_state);
                }
                if ((clear_attachment.aspectMask & VK_IMAGE_ASPECT_COLOR_BIT) &&
                    clear_attachment.colorAttachment != VK_ATTACHMENT_UNUSED && pColorAttachments) {
                    const auto& attachment = pColorAttachments[clear_attachment.colorAttachment];
                    if (attachment.imageView) {
                        auto image_view_state = Get<IMAGE_VIEW_STATE>(attachment.imageView);
                        const VkFormat format = image_view_state->create_info.format;
                        RecordClearColor(format, clear_attachment.clearValue.color);
                    }
                }
            }
        }

        // TODO: Implement other best practices for dynamic rendering

    } else {
        auto& subpass = rp_state->createInfo.pSubpasses[cmd_state->activeSubpass];
        for (uint32_t i = 0; i < attachmentCount; i++) {
            auto& attachment = pClearAttachments[i];
            uint32_t fb_attachment = VK_ATTACHMENT_UNUSED;
            VkImageAspectFlags aspects = attachment.aspectMask;

            if (aspects & VK_IMAGE_ASPECT_DEPTH_BIT) {
                if (VendorCheckEnabled(kBPVendorNVIDIA)) {
                    RecordResetScopeZcullDirection(*cmd_state);
                }
            }
            if (aspects & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) {
                if (subpass.pDepthStencilAttachment) {
                    fb_attachment = subpass.pDepthStencilAttachment->attachment;
                }
            } else if (aspects & VK_IMAGE_ASPECT_COLOR_BIT) {
                fb_attachment = subpass.pColorAttachments[attachment.colorAttachment].attachment;
            }
            if (fb_attachment != VK_ATTACHMENT_UNUSED) {
                if (full_clear) {
                    RecordAttachmentClearAttachments(*cmd_state, fb_attachment, attachment.colorAttachment, aspects, rectCount,
                                                     pRects);
                } else {
                    RecordAttachmentAccess(*cmd_state, fb_attachment, aspects);
                }
                if (VendorCheckEnabled(kBPVendorNVIDIA)) {
                    const VkFormat format = rp_state->createInfo.pAttachments[fb_attachment].format;
                    RecordClearColor(format, attachment.clearValue.color);
                }
            }
        }
    }
}

void BestPractices::PreCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                                uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) {
    ValidationStateTracker::PreCallRecordCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset,
                                                        firstInstance);

    auto cmd_state = GetWrite<bp_state::CommandBuffer>(commandBuffer);
    if ((indexCount * instanceCount) <= kSmallIndexedDrawcallIndices) {
        cmd_state->small_indexed_draw_call_count++;
    }

    ValidateBoundDescriptorSets(*cmd_state, VK_PIPELINE_BIND_POINT_GRAPHICS, "vkCmdDrawIndexed()");
}

void BestPractices::PostCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                                 uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) {
    StateTracker::PostCallRecordCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    RecordCmdDrawType(commandBuffer, indexCount * instanceCount, "vkCmdDrawIndexed()");
}

bool BestPractices::PreCallValidateCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                   uint32_t drawCount, uint32_t stride) const {
    bool skip = false;

    if (drawCount == 0) {
        skip |= LogWarning(device, kVUID_BestPractices_CmdDraw_DrawCountZero,
                           "Warning: You are calling vkCmdDrawIndirect() with a drawCount of Zero.");
    }

    skip |= ValidateCmdDrawType(commandBuffer, "vkCmdDrawIndirect()");

    return skip;
}

void BestPractices::PostCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                  uint32_t count, uint32_t stride) {
    StateTracker::PostCallRecordCmdDrawIndirect(commandBuffer, buffer, offset, count, stride);
    RecordCmdDrawType(commandBuffer, count, "vkCmdDrawIndirect()");
}

bool BestPractices::PreCallValidateCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                          uint32_t drawCount, uint32_t stride) const {
    bool skip = false;

    if (drawCount == 0) {
        skip |= LogWarning(device, kVUID_BestPractices_CmdDraw_DrawCountZero,
                           "Warning: You are calling vkCmdDrawIndexedIndirect() with a drawCount of Zero.");
    }

    skip |= ValidateCmdDrawType(commandBuffer, "vkCmdDrawIndexedIndirect()");

    return skip;
}

void BestPractices::PostCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                         uint32_t count, uint32_t stride) {
    StateTracker::PostCallRecordCmdDrawIndexedIndirect(commandBuffer, buffer, offset, count, stride);
    RecordCmdDrawType(commandBuffer, count, "vkCmdDrawIndexedIndirect()");
}

bool BestPractices::PreCallValidateCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                               VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                               uint32_t maxDrawCount, uint32_t stride) const {
    bool skip = ValidateCmdDrawType(commandBuffer, "vkCmdDrawIndexedIndirectCount()");

    return skip;
}

void BestPractices::PostCallRecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                              VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                              uint32_t maxDrawCount, uint32_t stride) {
    StateTracker::PostCallRecordCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset,
                                                            maxDrawCount, stride);
    RecordCmdDrawType(commandBuffer, 0, "vkCmdDrawIndexedIndirectCount()");
}

bool BestPractices::PreCallValidateCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                  VkDeviceSize offset, VkBuffer countBuffer,
                                                                  VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                  uint32_t stride) const {
    bool skip = ValidateCmdDrawType(commandBuffer, "vkCmdDrawIndexedIndirectCountAMD");

    return skip;
}

void BestPractices::PostCallRecordCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                 VkDeviceSize offset, VkBuffer countBuffer,
                                                                 VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                 uint32_t stride) {
    StateTracker::PostCallRecordCmdDrawIndexedIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset,
                                                               maxDrawCount, stride);
    RecordCmdDrawType(commandBuffer, 0, "vkCmdDrawIndexedIndirectCountAMD()");
}

bool BestPractices::PreCallValidateCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                  VkDeviceSize offset, VkBuffer countBuffer,
                                                                  VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                  uint32_t stride) const {
    bool skip = ValidateCmdDrawType(commandBuffer, "vkCmdDrawIndexedIndirectCountKHR");

    return skip;
}

void BestPractices::PostCallRecordCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                 VkDeviceSize offset, VkBuffer countBuffer,
                                                                 VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                 uint32_t stride) {
    StateTracker::PostCallRecordCmdDrawIndexedIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset,
                                                               maxDrawCount, stride);
    RecordCmdDrawType(commandBuffer, 0, "vkCmdDrawIndexedIndirectCountKHR()");
}

bool BestPractices::PreCallValidateCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount,
                                                               uint32_t firstInstance, VkBuffer counterBuffer,
                                                               VkDeviceSize counterBufferOffset, uint32_t counterOffset,
                                                               uint32_t vertexStride) const {
    bool skip = ValidateCmdDrawType(commandBuffer, "vkCmdDrawIndirectByteCountEXT");

    return skip;
}

void BestPractices::PostCallRecordCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount,
                                                              uint32_t firstInstance, VkBuffer counterBuffer,
                                                              VkDeviceSize counterBufferOffset, uint32_t counterOffset,
                                                              uint32_t vertexStride) {
    StateTracker::PostCallRecordCmdDrawIndirectByteCountEXT(commandBuffer, instanceCount, firstInstance, counterBuffer,
                                                            counterBufferOffset, counterOffset, vertexStride);
    RecordCmdDrawType(commandBuffer, 0, "vkCmdDrawIndirectByteCountEXT()");
}

bool BestPractices::PreCallValidateCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                        VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                        uint32_t stride) const {
    bool skip = ValidateCmdDrawType(commandBuffer, "vkCmdDrawIndirectCount");

    return skip;
}

void BestPractices::PostCallRecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                       uint32_t stride) {
    StateTracker::PostCallRecordCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount,
                                                     stride);
    RecordCmdDrawType(commandBuffer, 0, "vkCmdDrawIndirectCount()");
}

bool BestPractices::PreCallValidateCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                           VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                           uint32_t maxDrawCount, uint32_t stride) const {
    bool skip = ValidateCmdDrawType(commandBuffer, "vkCmdDrawIndirectCountAMD");

    return skip;
}

void BestPractices::PostCallRecordCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                          VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                          uint32_t maxDrawCount, uint32_t stride) {
    StateTracker::PostCallRecordCmdDrawIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount,
                                                        stride);
    RecordCmdDrawType(commandBuffer, 0, "vkCmdDrawIndirectCountAMD()");
}

bool BestPractices::PreCallValidateCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                           VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                           uint32_t maxDrawCount, uint32_t stride) const {
    bool skip = ValidateCmdDrawType(commandBuffer, "vkCmdDrawIndirectCountKHR");

    return skip;
}

void BestPractices::PostCallRecordCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                          VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                          uint32_t maxDrawCount, uint32_t stride) {
    StateTracker::PostCallRecordCmdDrawIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount,
                                                        stride);
    RecordCmdDrawType(commandBuffer, 0, "vkCmdDrawIndirectCountKHR()");
}

bool BestPractices::PreCallValidateCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                   VkDeviceSize offset, VkBuffer countBuffer,
                                                                   VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                   uint32_t stride) const {
    bool skip = ValidateCmdDrawType(commandBuffer, "vkCmdDrawMeshTasksIndirectCountNV");

    return skip;
}

void BestPractices::PostCallRecordCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                  VkDeviceSize offset, VkBuffer countBuffer,
                                                                  VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                  uint32_t stride) {
    StateTracker::PostCallRecordCmdDrawMeshTasksIndirectCountNV(commandBuffer, buffer, offset, countBuffer, countBufferOffset,
                                                                maxDrawCount, stride);
    RecordCmdDrawType(commandBuffer, 0, "vkCmdDrawMeshTasksIndirectCountNV()");
}

bool BestPractices::PreCallValidateCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                              uint32_t drawCount, uint32_t stride) const {
    bool skip = ValidateCmdDrawType(commandBuffer, "vkCmdDrawMeshTasksIndirectNV");

    return skip;
}

void BestPractices::PostCallRecordCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                             uint32_t drawCount, uint32_t stride) {
    StateTracker::PostCallRecordCmdDrawMeshTasksIndirectNV(commandBuffer, buffer, offset, drawCount, stride);
    RecordCmdDrawType(commandBuffer, 0, "vkCmdDrawMeshTasksIndirectNV()");
}

bool BestPractices::PreCallValidateCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask) const {
    bool skip = ValidateCmdDrawType(commandBuffer, "vkCmdDrawMeshTasksNV");

    return skip;
}

void BestPractices::PostCallRecordCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask) {
    StateTracker::PostCallRecordCmdDrawMeshTasksNV(commandBuffer, taskCount, firstTask);
    RecordCmdDrawType(commandBuffer, 0, "vkCmdDrawMeshTasksNV()");
}

bool BestPractices::PreCallValidateCmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                                          const VkMultiDrawIndexedInfoEXT* pIndexInfo, uint32_t instanceCount,
                                                          uint32_t firstInstance, uint32_t stride,
                                                          const int32_t* pVertexOffset) const {
    bool skip = ValidateCmdDrawType(commandBuffer, "vkCmdDrawMultiIndexedEXT");

    return skip;
}

void BestPractices::PostCallRecordCmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                                         const VkMultiDrawIndexedInfoEXT* pIndexInfo, uint32_t instanceCount,
                                                         uint32_t firstInstance, uint32_t stride, const int32_t* pVertexOffset) {
    StateTracker::PostCallRecordCmdDrawMultiIndexedEXT(commandBuffer, drawCount, pIndexInfo, instanceCount, firstInstance, stride,
                                                       pVertexOffset);
    uint32_t count = 0;
    for (uint32_t i = 0; i < drawCount; ++i) {
        count += pIndexInfo[i].indexCount;
    }
    RecordCmdDrawType(commandBuffer, count, "vkCmdDrawMultiIndexedEXT()");
}

bool BestPractices::PreCallValidateCmdDrawMultiEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                                   const VkMultiDrawInfoEXT* pVertexInfo, uint32_t instanceCount,
                                                   uint32_t firstInstance, uint32_t stride) const {
    bool skip = ValidateCmdDrawType(commandBuffer, "vkCmdDrawMultiEXT");

    return skip;
}

void BestPractices::PostCallRecordCmdDrawMultiEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                                  const VkMultiDrawInfoEXT* pVertexInfo, uint32_t instanceCount,
                                                  uint32_t firstInstance, uint32_t stride) {
    StateTracker::PostCallRecordCmdDrawMultiEXT(commandBuffer, drawCount, pVertexInfo, instanceCount, firstInstance, stride);
    uint32_t count = 0;
    for (uint32_t i = 0; i < drawCount; ++i) {
        count += pVertexInfo[i].vertexCount;
    }
    RecordCmdDrawType(commandBuffer, count, "vkCmdDrawMultiEXT()");
}

void BestPractices::ValidateBoundDescriptorSets(bp_state::CommandBuffer& cb_state, VkPipelineBindPoint bind_point,
                                                const char* function_name) {
    auto lvl_bind_point = ConvertToLvlBindPoint(bind_point);
    auto& last_bound = cb_state.lastBound[lvl_bind_point];

    for (const auto& descriptor_set : last_bound.per_set) {
        if (!descriptor_set.bound_descriptor_set) continue;
        for (const auto& binding : *descriptor_set.bound_descriptor_set) {
            // For bindless scenarios, we should not attempt to track descriptor set state.
            // It is highly uncertain which resources are actually bound.
            // Resources which are written to such a descriptor should be marked as indeterminate w.r.t. state.
            if (binding->binding_flags & (VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT |
                                          VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT)) {
                continue;
            }

            for (uint32_t i = 0; i < binding->count; ++i) {
                VkImageView image_view{VK_NULL_HANDLE};

                auto descriptor = binding->GetDescriptor(i);
                if (!descriptor) {
                    continue;
                }
                switch (descriptor->GetClass()) {
                    case cvdescriptorset::DescriptorClass::Image: {
                        if (const auto image_descriptor = static_cast<const cvdescriptorset::ImageDescriptor*>(descriptor)) {
                            image_view = image_descriptor->GetImageView();
                        }
                        break;
                    }
                    case cvdescriptorset::DescriptorClass::ImageSampler: {
                        if (const auto image_sampler_descriptor =
                                static_cast<const cvdescriptorset::ImageSamplerDescriptor*>(descriptor)) {
                            image_view = image_sampler_descriptor->GetImageView();
                        }
                        break;
                    }
                    default:
                        break;
                }

                if (image_view) {
                    auto image_view_state = Get<IMAGE_VIEW_STATE>(image_view);
                    QueueValidateImageView(cb_state.queue_submit_functions, function_name, image_view_state.get(),
                                           IMAGE_SUBRESOURCE_USAGE_BP::DESCRIPTOR_ACCESS);
                }
            }
        }
    }
}

void BestPractices::PreCallRecordCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount,
                                         uint32_t firstVertex, uint32_t firstInstance) {
    const auto cb_node = GetWrite<bp_state::CommandBuffer>(commandBuffer);
    ValidateBoundDescriptorSets(*cb_node, VK_PIPELINE_BIND_POINT_GRAPHICS, "vkCmdDraw()");
}

void BestPractices::PreCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                 uint32_t drawCount, uint32_t stride) {
    const auto cb_node = GetWrite<bp_state::CommandBuffer>(commandBuffer);
    ValidateBoundDescriptorSets(*cb_node, VK_PIPELINE_BIND_POINT_GRAPHICS, "vkCmdDrawIndirect()");
}

void BestPractices::PreCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                        uint32_t drawCount, uint32_t stride) {
    const auto cb_node = GetWrite<bp_state::CommandBuffer>(commandBuffer);
    ValidateBoundDescriptorSets(*cb_node, VK_PIPELINE_BIND_POINT_GRAPHICS, "vkCmdDrawIndexedIndirect()");
}

bool BestPractices::PreCallValidateCmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                               uint32_t groupCountZ) const {
    bool skip = false;

    if ((groupCountX == 0) || (groupCountY == 0) || (groupCountZ == 0)) {
        skip |= LogWarning(device, kVUID_BestPractices_CmdDispatch_GroupCountZero,
                           "Warning: You are calling vkCmdDispatch() while one or more groupCounts are zero (groupCountX = %" PRIu32
                           ", groupCountY = %" PRIu32 ", groupCountZ = %" PRIu32 ").",
                           groupCountX, groupCountY, groupCountZ);
    }

    return skip;
}

bool BestPractices::PreCallValidateCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo) const {
    bool skip = false;
    skip |= StateTracker::PreCallValidateCmdEndRenderPass2(commandBuffer, pSubpassEndInfo);
    skip |= ValidateCmdEndRenderPass(commandBuffer);
    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        const auto cmd_state = GetRead<bp_state::CommandBuffer>(commandBuffer);
        assert(cmd_state);
        skip |= ValidateZcullScope(*cmd_state);
    }
    return skip;
}

bool BestPractices::PreCallValidateCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer,
                                                        const VkSubpassEndInfo* pSubpassEndInfo) const {
    bool skip = false;
    skip |= StateTracker::PreCallValidateCmdEndRenderPass2KHR(commandBuffer, pSubpassEndInfo);
    skip |= ValidateCmdEndRenderPass(commandBuffer);
    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        const auto cmd_state = GetRead<bp_state::CommandBuffer>(commandBuffer);
        assert(cmd_state);
        skip |= ValidateZcullScope(*cmd_state);
    }
    return skip;
}

bool BestPractices::PreCallValidateCmdEndRenderPass(VkCommandBuffer commandBuffer) const {
    bool skip = false;
    skip |= StateTracker::PreCallValidateCmdEndRenderPass(commandBuffer);
    skip |= ValidateCmdEndRenderPass(commandBuffer);
    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        const auto cmd_state = GetRead<bp_state::CommandBuffer>(commandBuffer);
        assert(cmd_state);
        skip |= ValidateZcullScope(*cmd_state);
    }
    return skip;
}

bool BestPractices::PreCallValidateCmdEndRendering(VkCommandBuffer commandBuffer) const {
    bool skip = false;
    skip |= StateTracker::PreCallValidateCmdEndRendering(commandBuffer);
    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        const auto cmd_state = GetRead<bp_state::CommandBuffer>(commandBuffer);
        assert(cmd_state);
        skip |= ValidateZcullScope(*cmd_state);
    }
    return skip;
}

bool BestPractices::PreCallValidateCmdEndRenderingKHR(VkCommandBuffer commandBuffer) const {
    bool skip = false;
    skip |= StateTracker::PreCallValidateCmdEndRenderingKHR(commandBuffer);
    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        const auto cmd_state = GetRead<bp_state::CommandBuffer>(commandBuffer);
        assert(cmd_state);
        skip |= ValidateZcullScope(*cmd_state);
    }
    return skip;
}

bool BestPractices::ValidateCmdEndRenderPass(VkCommandBuffer commandBuffer) const {
    bool skip = false;
    const auto cmd = GetRead<bp_state::CommandBuffer>(commandBuffer);

    if (cmd == nullptr) return skip;
    auto& render_pass_state = cmd->render_pass_state;

    // Does the number of draw calls classified as depth only surpass the vendor limit for a specified vendor
    const bool depth_only_arm = render_pass_state.numDrawCallsDepthEqualCompare >= kDepthPrePassNumDrawCallsArm &&
                                render_pass_state.numDrawCallsDepthOnly >= kDepthPrePassNumDrawCallsArm;
    const bool depth_only_img = render_pass_state.numDrawCallsDepthEqualCompare >= kDepthPrePassNumDrawCallsIMG &&
                                render_pass_state.numDrawCallsDepthOnly >= kDepthPrePassNumDrawCallsIMG;

    // Only send the warning when the vendor is enabled and a depth prepass is detected
    bool uses_depth =
        (render_pass_state.depthAttachment || render_pass_state.colorAttachment) &&
        ((depth_only_arm && VendorCheckEnabled(kBPVendorArm)) || (depth_only_img && VendorCheckEnabled(kBPVendorIMG)));

    if (uses_depth) {
        skip |= LogPerformanceWarning(
            device, kVUID_BestPractices_EndRenderPass_DepthPrePassUsage,
            "%s %s: Depth pre-passes may be in use. In general, this is not recommended in tile-based deferred "
            "renderering architectures; such as those in Arm Mali or PowerVR GPUs. Since they can remove geometry "
            "hidden by other opaque geometry. Mali has Forward Pixel Killing (FPK), PowerVR has Hiden Surface "
            "Remover (HSR) in which case, using depth pre-passes for hidden surface removal may worsen performance.",
            VendorSpecificTag(kBPVendorArm), VendorSpecificTag(kBPVendorIMG));
    }

    RENDER_PASS_STATE* rp = cmd->activeRenderPass.get();

    if ((VendorCheckEnabled(kBPVendorArm) || VendorCheckEnabled(kBPVendorIMG)) && rp) {
        // If we use an attachment on-tile, we should access it in some way. Otherwise,
        // it is redundant to have it be part of the render pass.
        // Only consider it redundant if it will actually consume bandwidth, i.e.
        // LOAD_OP_LOAD is used or STORE_OP_STORE. CLEAR -> DONT_CARE is benign,
        // as is using pure input attachments.
        // CLEAR -> STORE might be considered a "useful" thing to do, but
        // the optimal thing to do is to defer the clear until you're actually
        // going to render to the image.

        uint32_t num_attachments = rp->createInfo.attachmentCount;
        for (uint32_t i = 0; i < num_attachments; i++) {
            if (!RenderPassUsesAttachmentOnTile(rp->createInfo, i) || RenderPassUsesAttachmentAsResolve(rp->createInfo, i)) {
                continue;
            }

            auto& attachment = rp->createInfo.pAttachments[i];

            VkImageAspectFlags bandwidth_aspects = 0;

            if (!FormatIsStencilOnly(attachment.format) &&
                (attachment.loadOp == VK_ATTACHMENT_LOAD_OP_LOAD || attachment.storeOp == VK_ATTACHMENT_STORE_OP_STORE)) {
                if (FormatHasDepth(attachment.format)) {
                    bandwidth_aspects |= VK_IMAGE_ASPECT_DEPTH_BIT;
                } else {
                    bandwidth_aspects |= VK_IMAGE_ASPECT_COLOR_BIT;
                }
            }

            if (FormatHasStencil(attachment.format) && (attachment.stencilLoadOp == VK_ATTACHMENT_LOAD_OP_LOAD ||
                                                        attachment.stencilStoreOp == VK_ATTACHMENT_STORE_OP_STORE)) {
                bandwidth_aspects |= VK_IMAGE_ASPECT_STENCIL_BIT;
            }

            if (!bandwidth_aspects) {
                continue;
            }

            auto itr = std::find_if(render_pass_state.touchesAttachments.begin(), render_pass_state.touchesAttachments.end(),
                                    [i](const bp_state::AttachmentInfo& info) { return info.framebufferAttachment == i; });
            uint32_t untouched_aspects = bandwidth_aspects;
            if (itr != render_pass_state.touchesAttachments.end()) {
                untouched_aspects &= ~itr->aspects;
            }

            if (untouched_aspects) {
                skip |= LogPerformanceWarning(
                    device, kVUID_BestPractices_EndRenderPass_RedundantAttachmentOnTile,
                    "%s %s: Render pass was ended, but attachment #%u (format: %u, untouched aspects 0x%x) "
                    "was never accessed by a pipeline or clear command. "
                    "On tile-based architectures, LOAD_OP_LOAD and STORE_OP_STORE consume bandwidth and should not be part of the "
                    "render pass if the attachments are not intended to be accessed.",
                    VendorSpecificTag(kBPVendorArm), VendorSpecificTag(kBPVendorIMG), i, attachment.format, untouched_aspects);
            }
        }
    }

    return skip;
}

void BestPractices::PreCallRecordCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z) {
    const auto cb_node = GetWrite<bp_state::CommandBuffer>(commandBuffer);
    ValidateBoundDescriptorSets(*cb_node, VK_PIPELINE_BIND_POINT_COMPUTE, "vkCmdDispatch()");
}

void BestPractices::PreCallRecordCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset) {
    const auto cb_node = GetWrite<bp_state::CommandBuffer>(commandBuffer);
    ValidateBoundDescriptorSets(*cb_node, VK_PIPELINE_BIND_POINT_COMPUTE, "vkCmdDispatchIndirect()");
}

bool BestPractices::ValidateGetPhysicalDeviceDisplayPlanePropertiesKHRQuery(VkPhysicalDevice physicalDevice,
                                                                            const char* api_name) const {
    bool skip = false;
    const auto bp_pd_state = Get<bp_state::PhysicalDevice>(physicalDevice);

    if (bp_pd_state) {
        if (bp_pd_state->vkGetPhysicalDeviceDisplayPlanePropertiesKHRState == UNCALLED) {
            skip |= LogWarning(physicalDevice, kVUID_BestPractices_DisplayPlane_PropertiesNotCalled,
                               "Potential problem with calling %s() without first retrieving properties from "
                               "vkGetPhysicalDeviceDisplayPlanePropertiesKHR or vkGetPhysicalDeviceDisplayPlaneProperties2KHR.",
                               api_name);
        }
    }

    return skip;
}

bool BestPractices::PreCallValidateGetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice physicalDevice, uint32_t planeIndex,
                                                                       uint32_t* pDisplayCount, VkDisplayKHR* pDisplays) const {
    bool skip = false;

    skip |= ValidateGetPhysicalDeviceDisplayPlanePropertiesKHRQuery(physicalDevice, "vkGetDisplayPlaneSupportedDisplaysKHR");

    return skip;
}

bool BestPractices::PreCallValidateGetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkDisplayModeKHR mode,
                                                                  uint32_t planeIndex,
                                                                  VkDisplayPlaneCapabilitiesKHR* pCapabilities) const {
    bool skip = false;

    skip |= ValidateGetPhysicalDeviceDisplayPlanePropertiesKHRQuery(physicalDevice, "vkGetDisplayPlaneCapabilitiesKHR");

    return skip;
}

bool BestPractices::PreCallValidateGetDisplayPlaneCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                                   const VkDisplayPlaneInfo2KHR* pDisplayPlaneInfo,
                                                                   VkDisplayPlaneCapabilities2KHR* pCapabilities) const {
    bool skip = false;

    skip |= ValidateGetPhysicalDeviceDisplayPlanePropertiesKHRQuery(physicalDevice, "vkGetDisplayPlaneCapabilities2KHR");

    return skip;
}

bool BestPractices::PreCallValidateGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount,
                                                         VkImage* pSwapchainImages) const {
    bool skip = false;

    auto swapchain_state = Get<bp_state::Swapchain>(swapchain);

    if (swapchain_state && pSwapchainImages) {
        // Compare the preliminary value of *pSwapchainImageCount with the value this time:
        if (swapchain_state->vkGetSwapchainImagesKHRState == UNCALLED) {
            skip |=
                LogWarning(device, kVUID_Core_Swapchain_PriorCount,
                           "vkGetSwapchainImagesKHR() called with non-NULL pSwapchainImageCount; but no prior positive value has "
                           "been seen for pSwapchainImages.");
        }

        if (*pSwapchainImageCount > swapchain_state->get_swapchain_image_count) {
            skip |= LogWarning(
                device, kVUID_BestPractices_Swapchain_InvalidCount,
                "vkGetSwapchainImagesKHR() called with non-NULL pSwapchainImages, and with pSwapchainImageCount set to a "
                "value (%" PRId32 ") that is greater than the value (%" PRId32
                ") that was returned when pSwapchainImages was NULL.",
                *pSwapchainImageCount, swapchain_state->get_swapchain_image_count);
        }
    }

    return skip;
}

// Common function to handle validation for GetPhysicalDeviceQueueFamilyProperties & 2KHR version
bool BestPractices::ValidateCommonGetPhysicalDeviceQueueFamilyProperties(const PHYSICAL_DEVICE_STATE* bp_pd_state,
                                                                         uint32_t requested_queue_family_property_count,
                                                                         const CALL_STATE call_state,
                                                                         const char* caller_name) const {
    bool skip = false;
    // Verify that for each physical device, this command is called first with NULL pQueueFamilyProperties in order to get count
    if (UNCALLED == call_state) {
        skip |= LogWarning(
            bp_pd_state->Handle(), kVUID_BestPractices_DevLimit_MissingQueryCount,
            "%s is called with non-NULL pQueueFamilyProperties before obtaining pQueueFamilyPropertyCount. It is "
            "recommended "
            "to first call %s with NULL pQueueFamilyProperties in order to obtain the maximal pQueueFamilyPropertyCount.",
            caller_name, caller_name);
        // Then verify that pCount that is passed in on second call matches what was returned
    } else if (bp_pd_state->queue_family_known_count != requested_queue_family_property_count) {
        skip |= LogWarning(bp_pd_state->Handle(), kVUID_BestPractices_DevLimit_CountMismatch,
                           "%s is called with non-NULL pQueueFamilyProperties and pQueueFamilyPropertyCount value %" PRIu32
                           ", but the largest previously returned pQueueFamilyPropertyCount for this physicalDevice is %" PRIu32
                           ". It is recommended to instead receive all the properties by calling %s with "
                           "pQueueFamilyPropertyCount that was "
                           "previously obtained by calling %s with NULL pQueueFamilyProperties.",
                           caller_name, requested_queue_family_property_count, bp_pd_state->queue_family_known_count, caller_name,
                           caller_name);
    }

    return skip;
}

bool BestPractices::PreCallValidateBindAccelerationStructureMemoryNV(
    VkDevice device, uint32_t bindInfoCount, const VkBindAccelerationStructureMemoryInfoNV* pBindInfos) const {
    bool skip = false;

    for (uint32_t i = 0; i < bindInfoCount; i++) {
        auto as_state = Get<ACCELERATION_STRUCTURE_STATE>(pBindInfos[i].accelerationStructure);
        if (!as_state->memory_requirements_checked) {
            // There's not an explicit requirement in the spec to call vkGetImageMemoryRequirements() prior to calling
            // BindAccelerationStructureMemoryNV but it's implied in that memory being bound must conform with
            // VkAccelerationStructureMemoryRequirementsInfoNV from vkGetAccelerationStructureMemoryRequirementsNV
            skip |= LogWarning(
                device, kVUID_BestPractices_BindAccelNV_NoMemReqQuery,
                "vkBindAccelerationStructureMemoryNV(): "
                "Binding memory to %s but vkGetAccelerationStructureMemoryRequirementsNV() has not been called on that structure.",
                report_data->FormatHandle(pBindInfos[i].accelerationStructure).c_str());
        }
    }

    return skip;
}

bool BestPractices::PreCallValidateGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice,
                                                                          uint32_t* pQueueFamilyPropertyCount,
                                                                          VkQueueFamilyProperties* pQueueFamilyProperties) const {
    const auto bp_pd_state = Get<bp_state::PhysicalDevice>(physicalDevice);
    if (pQueueFamilyProperties && bp_pd_state) {
        return ValidateCommonGetPhysicalDeviceQueueFamilyProperties(bp_pd_state.get(), *pQueueFamilyPropertyCount,
                                                                    bp_pd_state->vkGetPhysicalDeviceQueueFamilyPropertiesState,
                                                                    "vkGetPhysicalDeviceQueueFamilyProperties()");
    }
    return false;
}

bool BestPractices::PreCallValidateGetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice,
                                                                           uint32_t* pQueueFamilyPropertyCount,
                                                                           VkQueueFamilyProperties2* pQueueFamilyProperties) const {
    const auto bp_pd_state = Get<bp_state::PhysicalDevice>(physicalDevice);
    if (pQueueFamilyProperties && bp_pd_state) {
        return ValidateCommonGetPhysicalDeviceQueueFamilyProperties(bp_pd_state.get(), *pQueueFamilyPropertyCount,
                                                                    bp_pd_state->vkGetPhysicalDeviceQueueFamilyProperties2State,
                                                                    "vkGetPhysicalDeviceQueueFamilyProperties2()");
    }
    return false;
}

bool BestPractices::PreCallValidateGetPhysicalDeviceQueueFamilyProperties2KHR(
    VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties2* pQueueFamilyProperties) const {
    const auto bp_pd_state = Get<bp_state::PhysicalDevice>(physicalDevice);
    if (pQueueFamilyProperties && bp_pd_state) {
        return ValidateCommonGetPhysicalDeviceQueueFamilyProperties(bp_pd_state.get(), *pQueueFamilyPropertyCount,
                                                                    bp_pd_state->vkGetPhysicalDeviceQueueFamilyProperties2KHRState,
                                                                    "vkGetPhysicalDeviceQueueFamilyProperties2KHR()");
    }
    return false;
}

bool BestPractices::PreCallValidateGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                      uint32_t* pSurfaceFormatCount,
                                                                      VkSurfaceFormatKHR* pSurfaceFormats) const {
    if (!pSurfaceFormats) return false;
    const auto bp_pd_state = Get<bp_state::PhysicalDevice>(physicalDevice);
    const auto& call_state = bp_pd_state->vkGetPhysicalDeviceSurfaceFormatsKHRState;
    bool skip = false;
    if (call_state == UNCALLED) {
        // Since we haven't recorded a preliminary value of *pSurfaceFormatCount, that likely means that the application didn't
        // previously call this function with a NULL value of pSurfaceFormats:
        skip |= LogWarning(physicalDevice, kVUID_BestPractices_DevLimit_MustQueryCount,
                           "vkGetPhysicalDeviceSurfaceFormatsKHR() called with non-NULL pSurfaceFormatCount; but no prior "
                           "positive value has been seen for pSurfaceFormats.");
    } else {
        if (*pSurfaceFormatCount > bp_pd_state->surface_formats_count) {
            skip |= LogWarning(physicalDevice, kVUID_BestPractices_DevLimit_CountMismatch,
                               "vkGetPhysicalDeviceSurfaceFormatsKHR() called with non-NULL pSurfaceFormatCount, and with "
                               "pSurfaceFormats set to a value (%u) that is greater than the value (%u) that was returned "
                               "when pSurfaceFormatCount was NULL.",
                               *pSurfaceFormatCount, bp_pd_state->surface_formats_count);
        }
    }
    return skip;
}

bool BestPractices::PreCallValidateQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo,
                                                   VkFence fence) const {
    bool skip = false;

    for (uint32_t bind_idx = 0; bind_idx < bindInfoCount; bind_idx++) {
        const VkBindSparseInfo& bind_info = pBindInfo[bind_idx];
        // Store sparse binding image_state and after binding is complete make sure that any requiring metadata have it bound
        vvl::unordered_set<const IMAGE_STATE*> sparse_images;
        // Track images getting metadata bound by this call in a set, it'll be recorded into the image_state
        // in RecordQueueBindSparse.
        vvl::unordered_set<const IMAGE_STATE*> sparse_images_with_metadata;
        // If we're binding sparse image memory make sure reqs were queried and note if metadata is required and bound
        for (uint32_t i = 0; i < bind_info.imageBindCount; ++i) {
            const auto& image_bind = bind_info.pImageBinds[i];
            auto image_state = Get<IMAGE_STATE>(image_bind.image);
            if (!image_state) {
                continue;  // Param/Object validation should report image_bind.image handles being invalid, so just skip here.
            }
            sparse_images.insert(image_state.get());
            if (image_state->createInfo.flags & VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT) {
                if (!image_state->get_sparse_reqs_called || image_state->sparse_requirements.empty()) {
                    // For now just warning if sparse image binding occurs without calling to get reqs first
                    skip |= LogWarning(image_state->image(), kVUID_BestPractices_MemTrack_InvalidState,
                                       "vkQueueBindSparse(): Binding sparse memory to %s without first calling "
                                       "vkGetImageSparseMemoryRequirements[2KHR]() to retrieve requirements.",
                                       report_data->FormatHandle(image_state->image()).c_str());
                }
            }
            if (!image_state->memory_requirements_checked[0]) {
                // For now just warning if sparse image binding occurs without calling to get reqs first
                skip |= LogWarning(image_state->image(), kVUID_BestPractices_MemTrack_InvalidState,
                                   "vkQueueBindSparse(): Binding sparse memory to %s without first calling "
                                   "vkGetImageMemoryRequirements() to retrieve requirements.",
                                   report_data->FormatHandle(image_state->image()).c_str());
            }
        }
        for (uint32_t i = 0; i < bind_info.imageOpaqueBindCount; ++i) {
            const auto& image_opaque_bind = bind_info.pImageOpaqueBinds[i];
            auto image_state = Get<IMAGE_STATE>(bind_info.pImageOpaqueBinds[i].image);
            if (!image_state) {
                continue;  // Param/Object validation should report image_bind.image handles being invalid, so just skip here.
            }
            sparse_images.insert(image_state.get());
            if (image_state->createInfo.flags & VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT) {
                if (!image_state->get_sparse_reqs_called || image_state->sparse_requirements.empty()) {
                    // For now just warning if sparse image binding occurs without calling to get reqs first
                    skip |= LogWarning(image_state->image(), kVUID_BestPractices_MemTrack_InvalidState,
                                       "vkQueueBindSparse(): Binding opaque sparse memory to %s without first calling "
                                       "vkGetImageSparseMemoryRequirements[2KHR]() to retrieve requirements.",
                                       report_data->FormatHandle(image_state->image()).c_str());
                }
            }
            if (!image_state->memory_requirements_checked[0]) {
                // For now just warning if sparse image binding occurs without calling to get reqs first
                skip |= LogWarning(image_state->image(), kVUID_BestPractices_MemTrack_InvalidState,
                                   "vkQueueBindSparse(): Binding opaque sparse memory to %s without first calling "
                                   "vkGetImageMemoryRequirements() to retrieve requirements.",
                                   report_data->FormatHandle(image_state->image()).c_str());
            }
            for (uint32_t j = 0; j < image_opaque_bind.bindCount; ++j) {
                if (image_opaque_bind.pBinds[j].flags & VK_SPARSE_MEMORY_BIND_METADATA_BIT) {
                    sparse_images_with_metadata.insert(image_state.get());
                }
            }
        }
        for (const auto& sparse_image_state : sparse_images) {
            if (sparse_image_state->sparse_metadata_required && !sparse_image_state->sparse_metadata_bound &&
                sparse_images_with_metadata.find(sparse_image_state) == sparse_images_with_metadata.end()) {
                // Warn if sparse image binding metadata required for image with sparse binding, but metadata not bound
                skip |= LogWarning(sparse_image_state->image(), kVUID_BestPractices_MemTrack_InvalidState,
                                   "vkQueueBindSparse(): Binding sparse memory to %s which requires a metadata aspect but no "
                                   "binding with VK_SPARSE_MEMORY_BIND_METADATA_BIT set was made.",
                                   report_data->FormatHandle(sparse_image_state->image()).c_str());
            }
        }
    }

    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        auto queue_state = Get<QUEUE_STATE>(queue);
        if (queue_state && queue_state->queueFamilyProperties.queueFlags != (VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT)) {
            skip |= LogPerformanceWarning(queue, kVUID_BestPractices_QueueBindSparse_NotAsync,
                                          "vkQueueBindSparse() issued on queue %s. All binds should happen on an asynchronous copy "
                                          "queue to hide the OS scheduling and submit costs.",
                                          report_data->FormatHandle(queue).c_str());
        }
    }

    return skip;
}

void BestPractices::ManualPostCallRecordQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo,
                                                        VkFence fence, VkResult result) {
    if (result != VK_SUCCESS) {
        return;
    }

    for (uint32_t bind_idx = 0; bind_idx < bindInfoCount; bind_idx++) {
        const VkBindSparseInfo& bind_info = pBindInfo[bind_idx];
        for (uint32_t i = 0; i < bind_info.imageOpaqueBindCount; ++i) {
            const auto& image_opaque_bind = bind_info.pImageOpaqueBinds[i];
            auto image_state = Get<IMAGE_STATE>(bind_info.pImageOpaqueBinds[i].image);
            if (!image_state) {
                continue;  // Param/Object validation should report image_bind.image handles being invalid, so just skip here.
            }
            for (uint32_t j = 0; j < image_opaque_bind.bindCount; ++j) {
                if (image_opaque_bind.pBinds[j].flags & VK_SPARSE_MEMORY_BIND_METADATA_BIT) {
                    image_state->sparse_metadata_bound = true;
                }
            }
        }
    }
}

bool BestPractices::ClearAttachmentsIsFullClear(const bp_state::CommandBuffer& cmd, uint32_t rectCount,
                                                const VkClearRect* pRects) const {
    if (cmd.createInfo.level == VK_COMMAND_BUFFER_LEVEL_SECONDARY) {
        // We don't know the accurate render area in a secondary,
        // so assume we clear the entire frame buffer.
        // This is resolved in CmdExecuteCommands where we can check if the clear is a full clear.
        return true;
    }

    // If we have a rect which covers the entire frame buffer, we have a LOAD_OP_CLEAR-like command.
    for (uint32_t i = 0; i < rectCount; i++) {
        auto& rect = pRects[i];
        auto& render_area = cmd.activeRenderPassBeginInfo.renderArea;
        if (rect.rect.extent.width == render_area.extent.width && rect.rect.extent.height == render_area.extent.height) {
            return true;
        }
    }

    return false;
}

bool BestPractices::ValidateClearAttachment(const bp_state::CommandBuffer& cmd, uint32_t fb_attachment, uint32_t color_attachment,
                                            VkImageAspectFlags aspects, bool secondary) const {
    const RENDER_PASS_STATE* rp = cmd.activeRenderPass.get();
    bool skip = false;

    if (!rp || fb_attachment == VK_ATTACHMENT_UNUSED) {
        return skip;
    }

    const auto& rp_state = cmd.render_pass_state;

    auto attachment_itr =
        std::find_if(rp_state.touchesAttachments.begin(), rp_state.touchesAttachments.end(),
                     [fb_attachment](const bp_state::AttachmentInfo& info) { return info.framebufferAttachment == fb_attachment; });

    // Only report aspects which haven't been touched yet.
    VkImageAspectFlags new_aspects = aspects;
    if (attachment_itr != rp_state.touchesAttachments.end()) {
        new_aspects &= ~attachment_itr->aspects;
    }

    // Warn if this is issued prior to Draw Cmd and clearing the entire attachment
    if (!cmd.has_draw_cmd) {
        skip |= LogPerformanceWarning(
            cmd.Handle(), kVUID_BestPractices_DrawState_ClearCmdBeforeDraw,
            "vkCmdClearAttachments() issued on %s prior to any Draw Cmds in current render pass. It is recommended you "
            "use RenderPass LOAD_OP_CLEAR on attachments instead.",
            report_data->FormatHandle(cmd.Handle()).c_str());
    }

    if ((new_aspects & VK_IMAGE_ASPECT_COLOR_BIT) &&
        rp->createInfo.pAttachments[fb_attachment].loadOp == VK_ATTACHMENT_LOAD_OP_LOAD) {
        skip |= LogPerformanceWarning(
            device, kVUID_BestPractices_ClearAttachments_ClearAfterLoad,
            "%svkCmdClearAttachments() issued on %s for color attachment #%u in this subpass, "
            "but LOAD_OP_LOAD was used. If you need to clear the framebuffer, always use LOAD_OP_CLEAR as "
            "it is more efficient.",
            secondary ? "vkCmdExecuteCommands(): " : "", report_data->FormatHandle(cmd.Handle()).c_str(), color_attachment);
    }

    if ((new_aspects & VK_IMAGE_ASPECT_DEPTH_BIT) &&
        rp->createInfo.pAttachments[fb_attachment].loadOp == VK_ATTACHMENT_LOAD_OP_LOAD) {
        skip |=
            LogPerformanceWarning(device, kVUID_BestPractices_ClearAttachments_ClearAfterLoad,
                                  "%svkCmdClearAttachments() issued on %s for the depth attachment in this subpass, "
                                  "but LOAD_OP_LOAD was used. If you need to clear the framebuffer, always use LOAD_OP_CLEAR as "
                                  "it is more efficient.",
                                  secondary ? "vkCmdExecuteCommands(): " : "", report_data->FormatHandle(cmd.Handle()).c_str());

        if (VendorCheckEnabled(kBPVendorNVIDIA)) {
            const auto cmd_state = GetRead<bp_state::CommandBuffer>(cmd.commandBuffer());
            assert(cmd_state);
            skip |= ValidateZcullScope(*cmd_state);
        }
    }

    if ((new_aspects & VK_IMAGE_ASPECT_STENCIL_BIT) &&
        rp->createInfo.pAttachments[fb_attachment].stencilLoadOp == VK_ATTACHMENT_LOAD_OP_LOAD) {
        skip |=
            LogPerformanceWarning(device, kVUID_BestPractices_ClearAttachments_ClearAfterLoad,
                                  "%svkCmdClearAttachments() issued on %s for the stencil attachment in this subpass, "
                                  "but LOAD_OP_LOAD was used. If you need to clear the framebuffer, always use LOAD_OP_CLEAR as "
                                  "it is more efficient.",
                                  secondary ? "vkCmdExecuteCommands(): " : "", report_data->FormatHandle(cmd.Handle()).c_str());
    }

    return skip;
}

bool BestPractices::PreCallValidateCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                                       const VkClearAttachment* pAttachments, uint32_t rectCount,
                                                       const VkClearRect* pRects) const {
    bool skip = false;
    const auto cb_node = GetRead<bp_state::CommandBuffer>(commandBuffer);
    if (!cb_node) return skip;

    if (cb_node->createInfo.level == VK_COMMAND_BUFFER_LEVEL_SECONDARY) {
        // Defer checks to ExecuteCommands.
        return skip;
    }

    // Only care about full clears, partial clears might have legitimate uses.
    const bool is_full_clear = ClearAttachmentsIsFullClear(*cb_node, rectCount, pRects);

    // Check for uses of ClearAttachments along with LOAD_OP_LOAD,
    // as it can be more efficient to just use LOAD_OP_CLEAR
    const RENDER_PASS_STATE* rp = cb_node->activeRenderPass.get();
    if (rp) {
        if (rp->use_dynamic_rendering || rp->use_dynamic_rendering_inherited) {
            const auto pColorAttachments = rp->dynamic_rendering_begin_rendering_info.pColorAttachments;

            if (VendorCheckEnabled(kBPVendorNVIDIA)) {
                for (uint32_t i = 0; i < attachmentCount; i++) {
                    const auto& attachment = pAttachments[i];
                    if (attachment.aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT) {
                        skip |= ValidateZcullScope(*cb_node);
                    }
                    if ((attachment.aspectMask & VK_IMAGE_ASPECT_COLOR_BIT) && attachment.colorAttachment != VK_ATTACHMENT_UNUSED) {
                        const auto& color_attachment = pColorAttachments[attachment.colorAttachment];
                        if (color_attachment.imageView) {
                            auto image_view_state = Get<IMAGE_VIEW_STATE>(color_attachment.imageView);
                            const VkFormat format = image_view_state->create_info.format;
                            skip |= ValidateClearColor(commandBuffer, format, attachment.clearValue.color);
                        }
                    }
                }
            }

            if (is_full_clear) {
                // TODO: Implement ValidateClearAttachment for dynamic rendering
            }

        } else {
            const auto& subpass = rp->createInfo.pSubpasses[cb_node->activeSubpass];

            if (is_full_clear) {
                for (uint32_t i = 0; i < attachmentCount; i++) {
                    const auto& attachment = pAttachments[i];

                    if (attachment.aspectMask & VK_IMAGE_ASPECT_COLOR_BIT) {
                        uint32_t color_attachment = attachment.colorAttachment;
                        uint32_t fb_attachment = subpass.pColorAttachments[color_attachment].attachment;
                        skip |= ValidateClearAttachment(*cb_node, fb_attachment, color_attachment, attachment.aspectMask, false);
                    }

                    if (subpass.pDepthStencilAttachment &&
                        (attachment.aspectMask & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT))) {
                        uint32_t fb_attachment = subpass.pDepthStencilAttachment->attachment;
                        skip |=
                            ValidateClearAttachment(*cb_node, fb_attachment, VK_ATTACHMENT_UNUSED, attachment.aspectMask, false);
                    }
                }
            }
            if (VendorCheckEnabled(kBPVendorNVIDIA) && rp->createInfo.pAttachments) {
                for (uint32_t attachment_idx = 0; attachment_idx < attachmentCount; ++attachment_idx) {
                    const auto& attachment = pAttachments[attachment_idx];

                    if (attachment.aspectMask & VK_IMAGE_ASPECT_COLOR_BIT) {
                        const uint32_t fb_attachment = subpass.pColorAttachments[attachment.colorAttachment].attachment;
                        if (fb_attachment != VK_ATTACHMENT_UNUSED) {
                            const VkFormat format = rp->createInfo.pAttachments[fb_attachment].format;
                            skip |= ValidateClearColor(commandBuffer, format, attachment.clearValue.color);
                        }
                    }
                }
            }
        }
    }

    if (VendorCheckEnabled(kBPVendorAMD)) {
        for (uint32_t attachment_idx = 0; attachment_idx < attachmentCount; attachment_idx++) {
            if (pAttachments[attachment_idx].aspectMask == VK_IMAGE_ASPECT_COLOR_BIT) {
                bool black_check = false;
                black_check |= pAttachments[attachment_idx].clearValue.color.float32[0] != 0.0f;
                black_check |= pAttachments[attachment_idx].clearValue.color.float32[1] != 0.0f;
                black_check |= pAttachments[attachment_idx].clearValue.color.float32[2] != 0.0f;
                black_check |= pAttachments[attachment_idx].clearValue.color.float32[3] != 0.0f &&
                               pAttachments[attachment_idx].clearValue.color.float32[3] != 1.0f;

                bool white_check = false;
                white_check |= pAttachments[attachment_idx].clearValue.color.float32[0] != 1.0f;
                white_check |= pAttachments[attachment_idx].clearValue.color.float32[1] != 1.0f;
                white_check |= pAttachments[attachment_idx].clearValue.color.float32[2] != 1.0f;
                white_check |= pAttachments[attachment_idx].clearValue.color.float32[3] != 0.0f &&
                               pAttachments[attachment_idx].clearValue.color.float32[3] != 1.0f;

                if (black_check && white_check) {
                    skip |= LogPerformanceWarning(
                        device, kVUID_BestPractices_ClearAttachment_FastClearValues,
                        "%s Performance warning: vkCmdClearAttachments() clear value for color attachment %" PRId32
                        " is not a fast clear value."
                        "Consider changing to one of the following:"
                        "RGBA(0, 0, 0, 0) "
                        "RGBA(0, 0, 0, 1) "
                        "RGBA(1, 1, 1, 0) "
                        "RGBA(1, 1, 1, 1)",
                        VendorSpecificTag(kBPVendorAMD), attachment_idx);
                }
            } else {
                if ((pAttachments[attachment_idx].clearValue.depthStencil.depth != 0 &&
                     pAttachments[attachment_idx].clearValue.depthStencil.depth != 1) &&
                    pAttachments[attachment_idx].clearValue.depthStencil.stencil != 0) {
                    skip |= LogPerformanceWarning(device, kVUID_BestPractices_ClearAttachment_FastClearValues,
                                                  "%s Performance warning: vkCmdClearAttachments() clear value for depth/stencil "
                                                  "attachment %" PRId32
                                                  " is not a fast clear value."
                                                  "Consider changing to one of the following:"
                                                  "D=0.0f, S=0"
                                                  "D=1.0f, S=0",
                                                  VendorSpecificTag(kBPVendorAMD), attachment_idx);
                }
            }
        }
    }

    return skip;
}

bool BestPractices::ValidateCmdResolveImage(VkCommandBuffer command_buffer, VkImage src_image, VkImage dst_image,
                                            CMD_TYPE cmd_type) const {
    bool skip = false;
    const char* func_name = CommandTypeString(cmd_type);
    auto src_image_type = Get<IMAGE_STATE>(src_image)->createInfo.imageType;
    auto dst_image_type = Get<IMAGE_STATE>(dst_image)->createInfo.imageType;

    if (src_image_type != dst_image_type) {
        skip |= LogPerformanceWarning(command_buffer, kVUID_BestPractices_DrawState_MismatchedImageType,
                                      "%s: srcImage type (%s) and dstImage type (%s) are not the same.", func_name,
                                      string_VkImageType(src_image_type), string_VkImageType(dst_image_type));
    }

    skip |= VendorCheckEnabled(kBPVendorArm) &&
            LogPerformanceWarning(command_buffer, kVUID_BestPractices_CmdResolveImage_ResolvingImage,
                                  "%s Attempting to use %s to resolve a multisampled image. "
                                  "This is a very slow and extremely bandwidth intensive path. "
                                  "You should always resolve multisampled images on-tile with pResolveAttachments in VkRenderPass.",
                                  VendorSpecificTag(kBPVendorArm), func_name);
    return skip;
}

bool BestPractices::PreCallValidateCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                   VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                                   const VkImageResolve* pRegions) const {
    bool skip = false;
    skip |= ValidateCmdResolveImage(commandBuffer, srcImage, dstImage, CMD_RESOLVEIMAGE);
    return skip;
}

bool BestPractices::PreCallValidateCmdResolveImage2KHR(VkCommandBuffer commandBuffer,
                                                       const VkResolveImageInfo2KHR* pResolveImageInfo) const {
    bool skip = false;
    skip |= ValidateCmdResolveImage(commandBuffer, pResolveImageInfo->srcImage, pResolveImageInfo->dstImage, CMD_RESOLVEIMAGE2KHR);
    return skip;
}

bool BestPractices::PreCallValidateCmdResolveImage2(VkCommandBuffer commandBuffer,
                                                    const VkResolveImageInfo2* pResolveImageInfo) const {
    bool skip = false;
    skip |= ValidateCmdResolveImage(commandBuffer, pResolveImageInfo->srcImage, pResolveImageInfo->dstImage, CMD_RESOLVEIMAGE2);
    return skip;
}

void BestPractices::PreCallRecordCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                 VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                                 const VkImageResolve* pRegions) {
    auto cb = GetWrite<bp_state::CommandBuffer>(commandBuffer);
    auto& funcs = cb->queue_submit_functions;
    auto src = Get<bp_state::Image>(srcImage);
    auto dst = Get<bp_state::Image>(dstImage);

    for (uint32_t i = 0; i < regionCount; i++) {
        QueueValidateImage(funcs, "vkCmdResolveImage()", src, IMAGE_SUBRESOURCE_USAGE_BP::RESOLVE_READ, pRegions[i].srcSubresource);
        QueueValidateImage(funcs, "vkCmdResolveImage()", dst, IMAGE_SUBRESOURCE_USAGE_BP::RESOLVE_WRITE,
                           pRegions[i].dstSubresource);
    }
}

void BestPractices::PreCallRecordCmdResolveImage2KHR(VkCommandBuffer commandBuffer,
                                                     const VkResolveImageInfo2KHR* pResolveImageInfo) {
    auto cb = GetWrite<bp_state::CommandBuffer>(commandBuffer);
    auto& funcs = cb->queue_submit_functions;
    auto src = Get<bp_state::Image>(pResolveImageInfo->srcImage);
    auto dst = Get<bp_state::Image>(pResolveImageInfo->dstImage);
    uint32_t regionCount = pResolveImageInfo->regionCount;

    for (uint32_t i = 0; i < regionCount; i++) {
        QueueValidateImage(funcs, "vkCmdResolveImage2KHR()", src, IMAGE_SUBRESOURCE_USAGE_BP::RESOLVE_READ,
                           pResolveImageInfo->pRegions[i].srcSubresource);
        QueueValidateImage(funcs, "vkCmdResolveImage2KHR()", dst, IMAGE_SUBRESOURCE_USAGE_BP::RESOLVE_WRITE,
                           pResolveImageInfo->pRegions[i].dstSubresource);
    }
}

void BestPractices::PreCallRecordCmdResolveImage2(VkCommandBuffer commandBuffer, const VkResolveImageInfo2* pResolveImageInfo) {
    auto cb = GetWrite<bp_state::CommandBuffer>(commandBuffer);
    auto& funcs = cb->queue_submit_functions;
    auto src = Get<bp_state::Image>(pResolveImageInfo->srcImage);
    auto dst = Get<bp_state::Image>(pResolveImageInfo->dstImage);
    uint32_t regionCount = pResolveImageInfo->regionCount;

    for (uint32_t i = 0; i < regionCount; i++) {
        QueueValidateImage(funcs, "vkCmdResolveImage2()", src, IMAGE_SUBRESOURCE_USAGE_BP::RESOLVE_READ,
                           pResolveImageInfo->pRegions[i].srcSubresource);
        QueueValidateImage(funcs, "vkCmdResolveImage2()", dst, IMAGE_SUBRESOURCE_USAGE_BP::RESOLVE_WRITE,
                           pResolveImageInfo->pRegions[i].dstSubresource);
    }
}

void BestPractices::PreCallRecordCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                    const VkClearColorValue* pColor, uint32_t rangeCount,
                                                    const VkImageSubresourceRange* pRanges) {
    auto cb = GetWrite<bp_state::CommandBuffer>(commandBuffer);
    auto& funcs = cb->queue_submit_functions;
    auto dst = Get<bp_state::Image>(image);

    for (uint32_t i = 0; i < rangeCount; i++) {
        QueueValidateImage(funcs, "vkCmdClearColorImage()", dst, IMAGE_SUBRESOURCE_USAGE_BP::CLEARED, pRanges[i]);
    }

    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        RecordClearColor(dst->createInfo.format, *pColor);
    }
}

void BestPractices::PreCallRecordCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                           const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount,
                                                           const VkImageSubresourceRange* pRanges) {
    ValidationStateTracker::PreCallRecordCmdClearDepthStencilImage(commandBuffer, image, imageLayout, pDepthStencil, rangeCount,
                                                                   pRanges);

    auto cb = GetWrite<bp_state::CommandBuffer>(commandBuffer);
    auto& funcs = cb->queue_submit_functions;
    auto dst = Get<bp_state::Image>(image);

    for (uint32_t i = 0; i < rangeCount; i++) {
        QueueValidateImage(funcs, "vkCmdClearDepthStencilImage()", dst, IMAGE_SUBRESOURCE_USAGE_BP::CLEARED, pRanges[i]);
    }
    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        for (uint32_t i = 0; i < rangeCount; i++) {
            RecordResetZcullDirection(*cb, image, pRanges[i]);
        }
    }
}

void BestPractices::PreCallRecordCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                              VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                              const VkImageCopy* pRegions) {
    ValidationStateTracker::PreCallRecordCmdCopyImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout,
                                                      regionCount, pRegions);

    auto cb = GetWrite<bp_state::CommandBuffer>(commandBuffer);
    auto& funcs = cb->queue_submit_functions;
    auto src = Get<bp_state::Image>(srcImage);
    auto dst = Get<bp_state::Image>(dstImage);

    for (uint32_t i = 0; i < regionCount; i++) {
        QueueValidateImage(funcs, "vkCmdCopyImage()", src, IMAGE_SUBRESOURCE_USAGE_BP::COPY_READ, pRegions[i].srcSubresource);
        QueueValidateImage(funcs, "vkCmdCopyImage()", dst, IMAGE_SUBRESOURCE_USAGE_BP::COPY_WRITE, pRegions[i].dstSubresource);
    }
}

void BestPractices::PreCallRecordCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                                      VkImageLayout dstImageLayout, uint32_t regionCount,
                                                      const VkBufferImageCopy* pRegions) {
    auto cb = GetWrite<bp_state::CommandBuffer>(commandBuffer);
    auto& funcs = cb->queue_submit_functions;
    auto dst = Get<bp_state::Image>(dstImage);

    for (uint32_t i = 0; i < regionCount; i++) {
        QueueValidateImage(funcs, "vkCmdCopyBufferToImage()", dst, IMAGE_SUBRESOURCE_USAGE_BP::COPY_WRITE,
                           pRegions[i].imageSubresource);
    }
}

void BestPractices::PreCallRecordCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                      VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions) {
    auto cb = GetWrite<bp_state::CommandBuffer>(commandBuffer);
    auto& funcs = cb->queue_submit_functions;
    auto src = Get<bp_state::Image>(srcImage);

    for (uint32_t i = 0; i < regionCount; i++) {
        QueueValidateImage(funcs, "vkCmdCopyImageToBuffer()", src, IMAGE_SUBRESOURCE_USAGE_BP::COPY_READ,
                           pRegions[i].imageSubresource);
    }
}

void BestPractices::PreCallRecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                              VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                              const VkImageBlit* pRegions, VkFilter filter) {
    auto cb = GetWrite<bp_state::CommandBuffer>(commandBuffer);
    auto& funcs = cb->queue_submit_functions;
    auto src = Get<bp_state::Image>(srcImage);
    auto dst = Get<bp_state::Image>(dstImage);

    for (uint32_t i = 0; i < regionCount; i++) {
        QueueValidateImage(funcs, "vkCmdBlitImage()", src, IMAGE_SUBRESOURCE_USAGE_BP::BLIT_READ, pRegions[i].srcSubresource);
        QueueValidateImage(funcs, "vkCmdBlitImage()", dst, IMAGE_SUBRESOURCE_USAGE_BP::BLIT_WRITE, pRegions[i].dstSubresource);
    }
}

template <typename RegionType>
bool BestPractices::ValidateCmdBlitImage(VkCommandBuffer command_buffer, uint32_t region_count, const RegionType* regions,
                                         CMD_TYPE cmd_type) const {
    bool skip = false;
    const char* func_name = CommandTypeString(cmd_type);
    for (uint32_t i = 0; i < region_count; i++) {
        const RegionType region = regions[i];
        if ((region.srcOffsets[0].x == region.srcOffsets[1].x) || (region.srcOffsets[0].y == region.srcOffsets[1].y) ||
            (region.srcOffsets[0].z == region.srcOffsets[1].z)) {
            skip |= LogWarning(command_buffer, kVUID_BestPractices_DrawState_InvalidExtents,
                               "%s: pRegions[%" PRIu32 "].srcOffsets specify a zero-volume area", func_name, i);
        }
        if ((region.dstOffsets[0].x == region.dstOffsets[1].x) || (region.dstOffsets[0].y == region.dstOffsets[1].y) ||
            (region.dstOffsets[0].z == region.dstOffsets[1].z)) {
            skip |= LogWarning(command_buffer, kVUID_BestPractices_DrawState_InvalidExtents,
                               "%s: pRegions[%" PRIu32 "].dstOffsets specify a zero-volume area", func_name, i);
        }
    }
    return skip;
}

bool BestPractices::PreCallValidateCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                                const VkImageBlit* pRegions, VkFilter filter) const {
    return ValidateCmdBlitImage(commandBuffer, regionCount, pRegions, CMD_BLITIMAGE);
}

bool BestPractices::PreCallValidateCmdBlitImage2KHR(VkCommandBuffer commandBuffer,
                                                    const VkBlitImageInfo2KHR* pBlitImageInfo) const {
    return ValidateCmdBlitImage(commandBuffer, pBlitImageInfo->regionCount, pBlitImageInfo->pRegions, CMD_BLITIMAGE2KHR);
}

bool BestPractices::PreCallValidateCmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2* pBlitImageInfo) const {
    return ValidateCmdBlitImage(commandBuffer, pBlitImageInfo->regionCount, pBlitImageInfo->pRegions, CMD_BLITIMAGE2);
}

bool BestPractices::PreCallValidateCreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkSampler* pSampler) const {
    bool skip = false;

    if (VendorCheckEnabled(kBPVendorArm)) {
        if ((pCreateInfo->addressModeU != pCreateInfo->addressModeV) || (pCreateInfo->addressModeV != pCreateInfo->addressModeW)) {
            skip |= LogPerformanceWarning(
                device, kVUID_BestPractices_CreateSampler_DifferentWrappingModes,
                "%s Creating a sampler object with wrapping modes which do not match (U = %u, V = %u, W = %u). "
                "This may cause reduced performance even if only U (1D image) or U/V wrapping modes (2D "
                "image) are actually used. If you need different wrapping modes, disregard this warning.",
                VendorSpecificTag(kBPVendorArm), pCreateInfo->addressModeU, pCreateInfo->addressModeV, pCreateInfo->addressModeW);
        }

        if ((pCreateInfo->minLod != 0.0f) || (pCreateInfo->maxLod < VK_LOD_CLAMP_NONE)) {
            skip |= LogPerformanceWarning(
                device, kVUID_BestPractices_CreateSampler_LodClamping,
                "%s Creating a sampler object with LOD clamping (minLod = %f, maxLod = %f). This may cause reduced performance. "
                "Instead of clamping LOD in the sampler, consider using an VkImageView which restricts the mip-levels, set minLod "
                "to 0.0, and maxLod to VK_LOD_CLAMP_NONE.",
                VendorSpecificTag(kBPVendorArm), pCreateInfo->minLod, pCreateInfo->maxLod);
        }

        if (pCreateInfo->mipLodBias != 0.0f) {
            skip |=
                LogPerformanceWarning(device, kVUID_BestPractices_CreateSampler_LodBias,
                                      "%s Creating a sampler object with LOD bias != 0.0 (%f). This will lead to less efficient "
                                      "descriptors being created and may cause reduced performance.",
                                      VendorSpecificTag(kBPVendorArm), pCreateInfo->mipLodBias);
        }

        if ((pCreateInfo->addressModeU == VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER ||
             pCreateInfo->addressModeV == VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER ||
             pCreateInfo->addressModeW == VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER) &&
            (pCreateInfo->borderColor != VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK)) {
            skip |= LogPerformanceWarning(
                device, kVUID_BestPractices_CreateSampler_BorderClampColor,
                "%s Creating a sampler object with border clamping and borderColor != VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK. "
                "This will lead to less efficient descriptors being created and may cause reduced performance. "
                "If possible, use VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK as the border color.",
                VendorSpecificTag(kBPVendorArm));
        }

        if (pCreateInfo->unnormalizedCoordinates) {
            skip |= LogPerformanceWarning(
                device, kVUID_BestPractices_CreateSampler_UnnormalizedCoordinates,
                "%s Creating a sampler object with unnormalized coordinates. This will lead to less efficient "
                "descriptors being created and may cause reduced performance.",
                VendorSpecificTag(kBPVendorArm));
        }

        if (pCreateInfo->anisotropyEnable) {
            skip |= LogPerformanceWarning(
                device, kVUID_BestPractices_CreateSampler_Anisotropy,
                "%s Creating a sampler object with anisotropy. This will lead to less efficient descriptors being created "
                "and may cause reduced performance.",
                VendorSpecificTag(kBPVendorArm));
        }
    }

    return skip;
}

void BestPractices::PreCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                         const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                                         const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                         void* cgpl_state) {
    ValidationStateTracker::PreCallRecordCreateGraphicsPipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator,
                                                                 pPipelines);
    // AMD best practice
    num_pso_ += createInfoCount;
}

bool BestPractices::PreCallValidateUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount,
                                                        const VkWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount,
                                                        const VkCopyDescriptorSet* pDescriptorCopies) const {
    bool skip = false;
    if (VendorCheckEnabled(kBPVendorAMD)) {
        if (descriptorCopyCount > 0) {
            skip |= LogPerformanceWarning(device, kVUID_BestPractices_UpdateDescriptors_AvoidCopyingDescriptors,
                                          "%s Performance warning: copying descriptor sets is not recommended",
                                          VendorSpecificTag(kBPVendorAMD));
        }
    }

    return skip;
}

bool BestPractices::PreCallValidateCreateDescriptorUpdateTemplate(VkDevice device,
                                                                  const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
                                                                  const VkAllocationCallbacks* pAllocator,
                                                                  VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate) const {
    bool skip = false;
    if (VendorCheckEnabled(kBPVendorAMD)) {
        skip |= LogPerformanceWarning(device, kVUID_BestPractices_UpdateDescriptors_PreferNonTemplate,
                                      "%s Performance warning: using DescriptorSetWithTemplate is not recommended. Prefer using "
                                      "vkUpdateDescriptorSet instead",
                                      VendorSpecificTag(kBPVendorAMD));
    }

    return skip;
}

bool BestPractices::PreCallValidateCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                      const VkClearColorValue* pColor, uint32_t rangeCount,
                                                      const VkImageSubresourceRange* pRanges) const {
    bool skip = false;

    auto dst = Get<bp_state::Image>(image);

    if (VendorCheckEnabled(kBPVendorAMD)) {
        skip |= LogPerformanceWarning(
            device, kVUID_BestPractices_ClearAttachment_ClearImage,
            "%s Performance warning: using vkCmdClearColorImage is not recommended. Prefer using LOAD_OP_CLEAR or "
            "vkCmdClearAttachments instead",
            VendorSpecificTag(kBPVendorAMD));
    }
    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        skip |= ValidateClearColor(commandBuffer, dst->createInfo.format, *pColor);
    }

    return skip;
}

bool BestPractices::PreCallValidateCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image,
                                                             VkImageLayout imageLayout,
                                                             const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount,
                                                             const VkImageSubresourceRange* pRanges) const {
    bool skip = false;
    if (VendorCheckEnabled(kBPVendorAMD)) {
        skip |= LogPerformanceWarning(
            device, kVUID_BestPractices_ClearAttachment_ClearImage,
            "%s Performance warning: using vkCmdClearDepthStencilImage is not recommended. Prefer using LOAD_OP_CLEAR or "
            "vkCmdClearAttachments instead",
            VendorSpecificTag(kBPVendorAMD));
    }
    const auto cmd_state = GetRead<bp_state::CommandBuffer>(commandBuffer);
    assert(cmd_state);
    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        for (uint32_t i = 0; i < rangeCount; i++) {
            skip |= ValidateZcull(*cmd_state, image, pRanges[i]);
        }
    }

    return skip;
}

bool BestPractices::PreCallValidateCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo,
                                                        const VkAllocationCallbacks* pAllocator,
                                                        VkPipelineLayout* pPipelineLayout) const {
    bool skip = false;
    if (VendorCheckEnabled(kBPVendorAMD)) {
        uint32_t descriptor_size = enabled_features.core.robustBufferAccess ? 4 : 2;
        // Descriptor sets cost 1 DWORD each.
        // Dynamic buffers cost 2 DWORDs each when robust buffer access is OFF.
        // Dynamic buffers cost 4 DWORDs each when robust buffer access is ON.
        // Push constants cost 1 DWORD per 4 bytes in the Push constant range.
        uint32_t pipeline_size = pCreateInfo->setLayoutCount;  // in DWORDS
        for (uint32_t i = 0; i < pCreateInfo->setLayoutCount; i++) {
            auto descriptor_set_layout_state = Get<cvdescriptorset::DescriptorSetLayout>(pCreateInfo->pSetLayouts[i]);
            pipeline_size += descriptor_set_layout_state->GetDynamicDescriptorCount() * descriptor_size;
        }

        for (uint32_t i = 0; i < pCreateInfo->pushConstantRangeCount; i++) {
            pipeline_size += pCreateInfo->pPushConstantRanges[i].size / 4;
        }

        if (pipeline_size > kPipelineLayoutSizeWarningLimitAMD) {
            skip |=
                LogPerformanceWarning(device, kVUID_BestPractices_CreatePipelinesLayout_KeepLayoutSmall,
                                      "%s Performance warning: pipeline layout size is too large. Prefer smaller pipeline layouts."
                                      "Descriptor sets cost 1 DWORD each. "
                                      "Dynamic buffers cost 2 DWORDs each when robust buffer access is OFF. "
                                      "Dynamic buffers cost 4 DWORDs each when robust buffer access is ON. "
                                      "Push constants cost 1 DWORD per 4 bytes in the Push constant range. ",
                                      VendorSpecificTag(kBPVendorAMD));
        }
    }

    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        bool has_separate_sampler = false;
        size_t fast_space_usage = 0;

        for (uint32_t i = 0; i < pCreateInfo->setLayoutCount; ++i) {
            auto descriptor_set_layout_state = Get<cvdescriptorset::DescriptorSetLayout>(pCreateInfo->pSetLayouts[i]);
            for (const auto& binding : descriptor_set_layout_state->GetBindings()) {
                if (binding.descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER) {
                    has_separate_sampler = true;
                }

                if ((descriptor_set_layout_state->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT) ==
                    0U) {
                    size_t descriptor_type_size = 0;

                    switch (binding.descriptorType) {
                        case VK_DESCRIPTOR_TYPE_SAMPLER:
                        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
                        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
                        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
                            descriptor_type_size = 4;
                            break;
                        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
                        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
                        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV:
                            descriptor_type_size = 8;
                            break;
                        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
                        case VK_DESCRIPTOR_TYPE_MUTABLE_EXT:
                            descriptor_type_size = 16;
                            break;
                        case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK:
                            descriptor_type_size = 1;
                            break;
                        default:
                            // Unknown type.
                            break;
                    }

                    size_t descriptor_size = descriptor_type_size * binding.descriptorCount;
                    fast_space_usage += descriptor_size;
                }
            }
        }

        if (has_separate_sampler) {
            skip |= LogPerformanceWarning(
                device, kVUID_BestPractices_CreatePipelineLayout_SeparateSampler,
                "%s Consider using combined image samplers instead of separate samplers for marginally better performance.",
                VendorSpecificTag(kBPVendorNVIDIA));
        }

        if (fast_space_usage > kPipelineLayoutFastDescriptorSpaceNVIDIA) {
            skip |= LogPerformanceWarning(
                device, kVUID_BestPractices_CreatePipelinesLayout_LargePipelineLayout,
                "%s Pipeline layout size is too large, prefer using pipeline-specific descriptor set layouts. "
                "Aim for consuming less than %" PRIu32
                " bytes to allow fast reads for all non-bindless descriptors. "
                "Samplers, textures, texel buffers, and combined image samplers consume 4 bytes each. "
                "Uniform buffers and acceleration structures consume 8 bytes. "
                "Storage buffers consume 16 bytes. "
                "Push constants do not consume space.",
                VendorSpecificTag(kBPVendorNVIDIA), kPipelineLayoutFastDescriptorSpaceNVIDIA);
        }
    }

    return skip;
}

bool BestPractices::PreCallValidateCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                                const VkImageCopy* pRegions) const {
    bool skip = false;
    std::stringstream src_image_hex;
    std::stringstream dst_image_hex;
    src_image_hex << "0x" << std::hex << HandleToUint64(srcImage);
    dst_image_hex << "0x" << std::hex << HandleToUint64(dstImage);

    if (VendorCheckEnabled(kBPVendorAMD)) {
        auto src_state = Get<IMAGE_STATE>(srcImage);
        auto dst_state = Get<IMAGE_STATE>(dstImage);

        if (src_state && dst_state) {
            VkImageTiling src_Tiling = src_state->createInfo.tiling;
            VkImageTiling dst_Tiling = dst_state->createInfo.tiling;
            if (src_Tiling != dst_Tiling && (src_Tiling == VK_IMAGE_TILING_LINEAR || dst_Tiling == VK_IMAGE_TILING_LINEAR)) {
                skip |= LogPerformanceWarning(device, kVUID_BestPractices_vkImage_AvoidImageToImageCopy,
                                              "%s Performance warning: image %s and image %s have differing tilings. Use buffer to "
                                              "image (vkCmdCopyImageToBuffer) "
                                              "and image to buffer (vkCmdCopyBufferToImage) copies instead of image to image "
                                              "copies when converting between linear and optimal images",
                                              VendorSpecificTag(kBPVendorAMD), src_image_hex.str().c_str(),
                                              dst_image_hex.str().c_str());
            }
        }
    }

    return skip;
}

bool BestPractices::PreCallValidateCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                   VkPipeline pipeline) const {
    bool skip = false;

    auto cb = Get<bp_state::CommandBuffer>(commandBuffer);

    if (VendorCheckEnabled(kBPVendorAMD) || VendorCheckEnabled(kBPVendorNVIDIA)) {
        if (IsPipelineUsedInFrame(pipeline)) {
            skip |= LogPerformanceWarning(
                device, kVUID_BestPractices_Pipeline_SortAndBind,
                "%s %s Performance warning: Pipeline %s was bound twice in the frame. "
                "Keep pipeline state changes to a minimum, for example, by sorting draw calls by pipeline.",
                VendorSpecificTag(kBPVendorAMD), VendorSpecificTag(kBPVendorNVIDIA), report_data->FormatHandle(pipeline).c_str());
        }
    }
    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        const auto& tgm = cb->nv.tess_geometry_mesh;
        if (tgm.num_switches >= kNumBindPipelineTessGeometryMeshSwitchesThresholdNVIDIA && !tgm.threshold_signaled) {
            LogPerformanceWarning(commandBuffer, kVUID_BestPractices_BindPipeline_SwitchTessGeometryMesh,
                                  "%s Avoid switching between pipelines with and without tessellation, geometry, task, "
                                  "and/or mesh shaders. Group draw calls using these shader stages together.",
                                  VendorSpecificTag(kBPVendorNVIDIA));
            // Do not set 'skip' so the number of switches gets properly counted after the message.
        }
    }

    return skip;
}

void BestPractices::ManualPostCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits,
                                                    VkFence fence, VkResult result) {
    // AMD best practice
    num_queue_submissions_ += submitCount;
}

bool BestPractices::PreCallValidateQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo) const {
    bool skip = false;

    if (VendorCheckEnabled(kBPVendorAMD) || VendorCheckEnabled(kBPVendorNVIDIA)) {
        auto num = num_queue_submissions_.load();
        if (num > kNumberOfSubmissionWarningLimitAMD) {
            skip |= LogPerformanceWarning(device, kVUID_BestPractices_Submission_ReduceNumberOfSubmissions,
                                          "%s %s Performance warning: command buffers submitted %" PRId32
                                          " times this frame. Submitting command buffers has a CPU "
                                          "and GPU overhead. Submit fewer times to incur less overhead.",
                                          VendorSpecificTag(kBPVendorAMD), VendorSpecificTag(kBPVendorNVIDIA), num);
        }
    }

    return skip;
}

void BestPractices::PostCallRecordCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                                     VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                                     uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                                     uint32_t bufferMemoryBarrierCount,
                                                     const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                                     uint32_t imageMemoryBarrierCount,
                                                     const VkImageMemoryBarrier* pImageMemoryBarriers) {
    ValidationStateTracker::PostCallRecordCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, dependencyFlags,
                                                             memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount,
                                                             pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);

    num_barriers_objects_ += (memoryBarrierCount + imageMemoryBarrierCount + bufferMemoryBarrierCount);

    for (uint32_t i = 0; i < imageMemoryBarrierCount; ++i) {
        RecordCmdPipelineBarrierImageBarrier(commandBuffer, pImageMemoryBarriers[i]);
    }
}

void BestPractices::PostCallRecordCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo) {
    ValidationStateTracker::PostCallRecordCmdPipelineBarrier2(commandBuffer, pDependencyInfo);

    for (uint32_t i = 0; i < pDependencyInfo->imageMemoryBarrierCount; ++i) {
        RecordCmdPipelineBarrierImageBarrier(commandBuffer, pDependencyInfo->pImageMemoryBarriers[i]);
    }
}

void BestPractices::PostCallRecordCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo) {
    ValidationStateTracker::PostCallRecordCmdPipelineBarrier2KHR(commandBuffer, pDependencyInfo);

    for (uint32_t i = 0; i < pDependencyInfo->imageMemoryBarrierCount; ++i) {
        RecordCmdPipelineBarrierImageBarrier(commandBuffer, pDependencyInfo->pImageMemoryBarriers[i]);
    }
}

template <typename ImageMemoryBarrier>
void BestPractices::RecordCmdPipelineBarrierImageBarrier(VkCommandBuffer commandBuffer, const ImageMemoryBarrier& barrier) {
    auto cb = Get<bp_state::CommandBuffer>(commandBuffer);
    assert(cb);

    // Is a queue ownership acquisition barrier
    if (barrier.srcQueueFamilyIndex != barrier.dstQueueFamilyIndex &&
        barrier.dstQueueFamilyIndex == cb->command_pool->queueFamilyIndex) {
        auto image = Get<bp_state::Image>(barrier.image);
        auto subresource_range = barrier.subresourceRange;
        cb->queue_submit_functions.push_back([image, subresource_range](const ValidationStateTracker& vst, const QUEUE_STATE& qs,
                                                                        const CMD_BUFFER_STATE& cbs) -> bool {
            ForEachSubresource(*image, subresource_range, [&](uint32_t layer, uint32_t level) {
                // Update queue family index without changing usage, signifying a correct queue family transfer
                image->UpdateUsage(layer, level, image->GetUsageType(layer, level), qs.queueFamilyIndex);
            });
            return false;
        });
    }

    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        RecordResetZcullDirection(*cb, barrier.image, barrier.subresourceRange);
    }
}

bool BestPractices::PreCallValidateCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore) const {
    bool skip = false;
    if (VendorCheckEnabled(kBPVendorAMD) || VendorCheckEnabled(kBPVendorNVIDIA)) {
        if (Count<SEMAPHORE_STATE>() > kMaxRecommendedSemaphoreObjectsSizeAMD) {
            skip |= LogPerformanceWarning(device, kVUID_BestPractices_SyncObjects_HighNumberOfSemaphores,
                                          "%s %s Performance warning: High number of vkSemaphore objects created. "
                                          "Minimize the amount of queue synchronization that is used. "
                                          "Semaphores and fences have overhead. Each fence has a CPU and GPU cost with it.",
                                          VendorSpecificTag(kBPVendorAMD), VendorSpecificTag(kBPVendorNVIDIA));
        }
    }

    return skip;
}

bool BestPractices::PreCallValidateCreateFence(VkDevice device, const VkFenceCreateInfo* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkFence* pFence) const {
    bool skip = false;
    if (VendorCheckEnabled(kBPVendorAMD) || VendorCheckEnabled(kBPVendorNVIDIA)) {
        if (Count<FENCE_STATE>() > kMaxRecommendedFenceObjectsSizeAMD) {
            skip |= LogPerformanceWarning(device, kVUID_BestPractices_SyncObjects_HighNumberOfFences,
                                          "%s %s Performance warning: High number of VkFence objects created."
                                          "Minimize the amount of CPU-GPU synchronization that is used. "
                                          "Semaphores and fences have overhead. Each fence has a CPU and GPU cost with it.",
                                          VendorSpecificTag(kBPVendorAMD), VendorSpecificTag(kBPVendorNVIDIA));
        }
    }

    return skip;
}

void BestPractices::PostTransformLRUCacheModel::resize(size_t size) { _entries.resize(size); }

bool BestPractices::PostTransformLRUCacheModel::query_cache(uint32_t value) {
    // look for a cache hit
    auto hit = std::find_if(_entries.begin(), _entries.end(), [value](const CacheEntry& entry) { return entry.value == value; });
    if (hit != _entries.end()) {
        // mark the cache hit as being most recently used
        hit->age = iteration++;
        return true;
    }

    // if there's no cache hit, we need to model the entry being inserted into the cache
    CacheEntry new_entry = {value, iteration};
    if (iteration < static_cast<uint32_t>(std::distance(_entries.begin(), _entries.end()))) {
        // if there is still space left in the cache, use the next available slot
        *(_entries.begin() + iteration) = new_entry;
    } else {
        // otherwise replace the least recently used cache entry
        auto lru = std::min_element(_entries.begin(), hit, [](const CacheEntry& a, const CacheEntry& b) { return a.age < b.age; });
        *lru = new_entry;
    }
    iteration++;
    return false;
}

bool BestPractices::PreCallValidateAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout,
                                                       VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex) const {
    auto swapchain_data = Get<SWAPCHAIN_NODE>(swapchain);
    bool skip = false;
    if (swapchain_data && swapchain_data->images.size() == 0) {
        skip |= LogWarning(swapchain, kVUID_BestPractices_DrawState_SwapchainImagesNotFound,
                           "vkAcquireNextImageKHR: No images found to acquire from. Application probably did not call "
                           "vkGetSwapchainImagesKHR after swapchain creation.");
    }
    return skip;
}

void BestPractices::CommonPostCallRecordGetPhysicalDeviceQueueFamilyProperties(CALL_STATE& call_state, bool no_pointer) {
    if (no_pointer) {
        if (UNCALLED == call_state) {
            call_state = QUERY_COUNT;
        }
    } else {  // Save queue family properties
        call_state = QUERY_DETAILS;
    }
}

void BestPractices::PostCallRecordGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice,
                                                                         uint32_t* pQueueFamilyPropertyCount,
                                                                         VkQueueFamilyProperties* pQueueFamilyProperties) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceQueueFamilyProperties(physicalDevice, pQueueFamilyPropertyCount,
                                                                                 pQueueFamilyProperties);
    auto bp_pd_state = Get<bp_state::PhysicalDevice>(physicalDevice);
    if (bp_pd_state) {
        CommonPostCallRecordGetPhysicalDeviceQueueFamilyProperties(bp_pd_state->vkGetPhysicalDeviceQueueFamilyPropertiesState,
                                                                   nullptr == pQueueFamilyProperties);
    }
}

void BestPractices::PostCallRecordGetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice,
                                                                          uint32_t* pQueueFamilyPropertyCount,
                                                                          VkQueueFamilyProperties2* pQueueFamilyProperties) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, pQueueFamilyPropertyCount,
                                                                                  pQueueFamilyProperties);
    auto bp_pd_state = Get<bp_state::PhysicalDevice>(physicalDevice);
    if (bp_pd_state) {
        CommonPostCallRecordGetPhysicalDeviceQueueFamilyProperties(bp_pd_state->vkGetPhysicalDeviceQueueFamilyProperties2State,
                                                                   nullptr == pQueueFamilyProperties);
    }
}

void BestPractices::PostCallRecordGetPhysicalDeviceQueueFamilyProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                             uint32_t* pQueueFamilyPropertyCount,
                                                                             VkQueueFamilyProperties2* pQueueFamilyProperties) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceQueueFamilyProperties2KHR(physicalDevice, pQueueFamilyPropertyCount,
                                                                                     pQueueFamilyProperties);
    auto bp_pd_state = Get<bp_state::PhysicalDevice>(physicalDevice);
    if (bp_pd_state) {
        CommonPostCallRecordGetPhysicalDeviceQueueFamilyProperties(bp_pd_state->vkGetPhysicalDeviceQueueFamilyProperties2KHRState,
                                                                   nullptr == pQueueFamilyProperties);
    }
}

void BestPractices::PostCallRecordGetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures* pFeatures) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceFeatures(physicalDevice, pFeatures);
    auto bp_pd_state = Get<bp_state::PhysicalDevice>(physicalDevice);
    if (bp_pd_state) {
        bp_pd_state->vkGetPhysicalDeviceFeaturesState = QUERY_DETAILS;
    }
}

void BestPractices::PostCallRecordGetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice,
                                                             VkPhysicalDeviceFeatures2* pFeatures) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceFeatures2(physicalDevice, pFeatures);
    auto bp_pd_state = Get<bp_state::PhysicalDevice>(physicalDevice);
    if (bp_pd_state) {
        bp_pd_state->vkGetPhysicalDeviceFeaturesState = QUERY_DETAILS;
    }
}

void BestPractices::PostCallRecordGetPhysicalDeviceFeatures2KHR(VkPhysicalDevice physicalDevice,
                                                                VkPhysicalDeviceFeatures2* pFeatures) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceFeatures2KHR(physicalDevice, pFeatures);
    auto bp_pd_state = Get<bp_state::PhysicalDevice>(physicalDevice);
    if (bp_pd_state) {
        bp_pd_state->vkGetPhysicalDeviceFeaturesState = QUERY_DETAILS;
    }
}

void BestPractices::ManualPostCallRecordGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice,
                                                                                VkSurfaceKHR surface,
                                                                                VkSurfaceCapabilitiesKHR* pSurfaceCapabilities,
                                                                                VkResult result) {
    auto bp_pd_state = Get<bp_state::PhysicalDevice>(physicalDevice);
    if (bp_pd_state) {
        bp_pd_state->vkGetPhysicalDeviceSurfaceCapabilitiesKHRState = QUERY_DETAILS;
    }
}

void BestPractices::ManualPostCallRecordGetPhysicalDeviceSurfaceCapabilities2KHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
    VkSurfaceCapabilities2KHR* pSurfaceCapabilities, VkResult result) {
    auto bp_pd_state = Get<bp_state::PhysicalDevice>(physicalDevice);
    if (bp_pd_state) {
        bp_pd_state->vkGetPhysicalDeviceSurfaceCapabilitiesKHRState = QUERY_DETAILS;
    }
}

void BestPractices::ManualPostCallRecordGetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice,
                                                                                 VkSurfaceKHR surface,
                                                                                 VkSurfaceCapabilities2EXT* pSurfaceCapabilities,
                                                                                 VkResult result) {
    auto bp_pd_state = Get<bp_state::PhysicalDevice>(physicalDevice);
    if (bp_pd_state) {
        bp_pd_state->vkGetPhysicalDeviceSurfaceCapabilitiesKHRState = QUERY_DETAILS;
    }
}

void BestPractices::ManualPostCallRecordGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice,
                                                                                VkSurfaceKHR surface, uint32_t* pPresentModeCount,
                                                                                VkPresentModeKHR* pPresentModes, VkResult result) {
    auto bp_pd_data = Get<bp_state::PhysicalDevice>(physicalDevice);
    if (bp_pd_data) {
        auto& call_state = bp_pd_data->vkGetPhysicalDeviceSurfacePresentModesKHRState;

        if (*pPresentModeCount) {
            if (call_state < QUERY_COUNT) {
                call_state = QUERY_COUNT;
            }
        }
        if (pPresentModes) {
            if (call_state < QUERY_DETAILS) {
                call_state = QUERY_DETAILS;
            }
        }
    }
}

void BestPractices::ManualPostCallRecordGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                           uint32_t* pSurfaceFormatCount,
                                                                           VkSurfaceFormatKHR* pSurfaceFormats, VkResult result) {
    auto bp_pd_data = Get<bp_state::PhysicalDevice>(physicalDevice);
    if (bp_pd_data) {
        auto& call_state = bp_pd_data->vkGetPhysicalDeviceSurfaceFormatsKHRState;

        if (*pSurfaceFormatCount) {
            if (call_state < QUERY_COUNT) {
                call_state = QUERY_COUNT;
            }
            bp_pd_data->surface_formats_count = *pSurfaceFormatCount;
        }
        if (pSurfaceFormats) {
            if (call_state < QUERY_DETAILS) {
                call_state = QUERY_DETAILS;
            }
        }
    }
}

void BestPractices::ManualPostCallRecordGetPhysicalDeviceSurfaceFormats2KHR(VkPhysicalDevice physicalDevice,
                                                                            const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                                            uint32_t* pSurfaceFormatCount,
                                                                            VkSurfaceFormat2KHR* pSurfaceFormats, VkResult result) {
    auto bp_pd_data = Get<bp_state::PhysicalDevice>(physicalDevice);
    if (bp_pd_data) {
        if (*pSurfaceFormatCount) {
            if (bp_pd_data->vkGetPhysicalDeviceSurfaceFormatsKHRState < QUERY_COUNT) {
                bp_pd_data->vkGetPhysicalDeviceSurfaceFormatsKHRState = QUERY_COUNT;
            }
            bp_pd_data->surface_formats_count = *pSurfaceFormatCount;
        }
        if (pSurfaceFormats) {
            if (bp_pd_data->vkGetPhysicalDeviceSurfaceFormatsKHRState < QUERY_DETAILS) {
                bp_pd_data->vkGetPhysicalDeviceSurfaceFormatsKHRState = QUERY_DETAILS;
            }
        }
    }
}

void BestPractices::ManualPostCallRecordGetPhysicalDeviceDisplayPlanePropertiesKHR(VkPhysicalDevice physicalDevice,
                                                                                   uint32_t* pPropertyCount,
                                                                                   VkDisplayPlanePropertiesKHR* pProperties,
                                                                                   VkResult result) {
    auto bp_pd_data = Get<bp_state::PhysicalDevice>(physicalDevice);
    if (bp_pd_data) {
        if (*pPropertyCount) {
            if (bp_pd_data->vkGetPhysicalDeviceDisplayPlanePropertiesKHRState < QUERY_COUNT) {
                bp_pd_data->vkGetPhysicalDeviceDisplayPlanePropertiesKHRState = QUERY_COUNT;
            }
        }
        if (pProperties) {
            if (bp_pd_data->vkGetPhysicalDeviceDisplayPlanePropertiesKHRState < QUERY_DETAILS) {
                bp_pd_data->vkGetPhysicalDeviceDisplayPlanePropertiesKHRState = QUERY_DETAILS;
            }
        }
    }
}

void BestPractices::ManualPostCallRecordGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain,
                                                              uint32_t* pSwapchainImageCount, VkImage* pSwapchainImages,
                                                              VkResult result) {
    auto swapchain_state = Get<bp_state::Swapchain>(swapchain);
    if (swapchain_state && (pSwapchainImages || *pSwapchainImageCount)) {
        if (swapchain_state->vkGetSwapchainImagesKHRState < QUERY_DETAILS) {
            swapchain_state->vkGetSwapchainImagesKHRState = QUERY_DETAILS;
        }
    }
}

void BestPractices::PreCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence) {
    ValidationStateTracker::PreCallRecordQueueSubmit(queue, submitCount, pSubmits, fence);

    auto queue_state = Get<QUEUE_STATE>(queue);
    for (uint32_t submit = 0; submit < submitCount; submit++) {
        const auto& submit_info = pSubmits[submit];
        for (uint32_t cb_index = 0; cb_index < submit_info.commandBufferCount; cb_index++) {
            auto cb = GetWrite<bp_state::CommandBuffer>(submit_info.pCommandBuffers[cb_index]);
            for (auto& func : cb->queue_submit_functions) {
                func(*this, *queue_state, *cb);
            }
            cb->num_submits++;
        }
    }
}
