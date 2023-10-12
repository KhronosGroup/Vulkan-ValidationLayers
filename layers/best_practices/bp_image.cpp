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
#include "best_practices/best_practices_error_enums.h"

bool BestPractices::PreCallValidateCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkImage* pImage,
                                               const ErrorObject& error_obj) const {
    bool skip = false;

    if ((pCreateInfo->queueFamilyIndexCount > 1) && (pCreateInfo->sharingMode == VK_SHARING_MODE_EXCLUSIVE)) {
        std::stringstream image_hex;
        image_hex << "0x" << std::hex << HandleToUint64(pImage);

        skip |=
            LogWarning(kVUID_BestPractices_SharingModeExclusive, device, error_obj.location,
                       "Warning: Image (%s) specifies a sharing mode of VK_SHARING_MODE_EXCLUSIVE while specifying multiple queues "
                       "(queueFamilyIndexCount of %" PRIu32 ").",
                       image_hex.str().c_str(), pCreateInfo->queueFamilyIndexCount);
    }

    if ((pCreateInfo->flags & VK_IMAGE_CREATE_EXTENDED_USAGE_BIT) && !(pCreateInfo->flags & VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT)) {
        skip |= LogWarning(kVUID_BestPractices_ImageCreateFlags, device, error_obj.location,
                           "pCreateInfo->flags has VK_IMAGE_CREATE_EXTENDED_USAGE_BIT set, but not "
                           "VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT, therefore image views created from this image will have to use the "
                           "same format and VK_IMAGE_CREATE_EXTENDED_USAGE_BIT will not have any effect.");
    }

    if (VendorCheckEnabled(kBPVendorArm) || VendorCheckEnabled(kBPVendorIMG)) {
        if (pCreateInfo->samples > VK_SAMPLE_COUNT_1_BIT && !(pCreateInfo->usage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT)) {
            skip |= LogPerformanceWarning(
                kVUID_BestPractices_CreateImage_NonTransientMSImage, device, error_obj.location,
                "%s %s Trying to create a multisampled image, but createInfo.usage did not have "
                "VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT set. Multisampled images may be resolved on-chip, "
                "and do not need to be backed by physical storage. "
                "TRANSIENT_ATTACHMENT allows tiled GPUs to not back the multisampled image with physical memory.",
                VendorSpecificTag(kBPVendorArm), VendorSpecificTag(kBPVendorIMG));
        }
    }

    if (VendorCheckEnabled(kBPVendorArm) && pCreateInfo->samples > kMaxEfficientSamplesArm) {
        skip |= LogPerformanceWarning(
            kVUID_BestPractices_CreateImage_TooLargeSampleCount, device, error_obj.location,
            "%s Trying to create an image with %u samples. "
            "The hardware revision may not have full throughput for framebuffers with more than %u samples.",
            VendorSpecificTag(kBPVendorArm), static_cast<uint32_t>(pCreateInfo->samples), kMaxEfficientSamplesArm);
    }

    if (VendorCheckEnabled(kBPVendorIMG) && pCreateInfo->samples > kMaxEfficientSamplesImg) {
        skip |= LogPerformanceWarning(
            kVUID_BestPractices_CreateImage_TooLargeSampleCount, device, error_obj.location,
            "%s Trying to create an image with %u samples. "
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
        skip |= LogPerformanceWarning(kVUID_BestPractices_Texture_Format_PVRTC_Outdated, device, error_obj.location,
                                      "%s Trying to create an image with a PVRTC format. Both PVRTC1 and PVRTC2 "
                                      "are slower than standard image formats on PowerVR GPUs, prefer ETC, BC, ASTC, etc.",
                                      VendorSpecificTag(kBPVendorIMG));
    }

    if (VendorCheckEnabled(kBPVendorAMD)) {
        std::stringstream image_hex;
        image_hex << "0x" << std::hex << HandleToUint64(pImage);

        if ((pCreateInfo->usage & (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) &&
            (pCreateInfo->sharingMode == VK_SHARING_MODE_CONCURRENT)) {
            skip |= LogPerformanceWarning(
                kVUID_BestPractices_vkImage_AvoidConcurrentRenderTargets, device, error_obj.location,
                "%s Performance warning: image (%s) is created as a render target with VK_SHARING_MODE_CONCURRENT. "
                "Using a SHARING_MODE_CONCURRENT "
                "is not recommended with color and depth targets",
                VendorSpecificTag(kBPVendorAMD), image_hex.str().c_str());
        }

        if ((pCreateInfo->usage &
             (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) &&
            (pCreateInfo->flags & VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT)) {
            skip |= LogPerformanceWarning(
                kVUID_BestPractices_vkImage_DontUseMutableRenderTargets, device, error_obj.location,
                "%s Performance warning: image (%s) is created as a render target with VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT. "
                "Using a MUTABLE_FORMAT is not recommended with color, depth, and storage targets",
                VendorSpecificTag(kBPVendorAMD), image_hex.str().c_str());
        }

        if ((pCreateInfo->usage & (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) &&
            (pCreateInfo->usage & VK_IMAGE_USAGE_STORAGE_BIT)) {
            skip |= LogPerformanceWarning(
                kVUID_BestPractices_vkImage_DontUseStorageRenderTargets, device, error_obj.location,
                "%s Performance warning: image (%s) is created as a render target with VK_IMAGE_USAGE_STORAGE_BIT. Using a "
                "VK_IMAGE_USAGE_STORAGE_BIT is not recommended with color and depth targets",
                VendorSpecificTag(kBPVendorAMD), image_hex.str().c_str());
        }
    }

    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        std::stringstream image_hex;
        image_hex << "0x" << std::hex << HandleToUint64(pImage);

        if (pCreateInfo->tiling == VK_IMAGE_TILING_LINEAR) {
            skip |= LogPerformanceWarning(kVUID_BestPractices_CreateImage_TilingLinear, device, error_obj.location,
                                          "%s Performance warning: image (%s) is created with tiling VK_IMAGE_TILING_LINEAR. "
                                          "Use VK_IMAGE_TILING_OPTIMAL instead.",
                                          VendorSpecificTag(kBPVendorNVIDIA), image_hex.str().c_str());
        }

        if (pCreateInfo->format == VK_FORMAT_D32_SFLOAT || pCreateInfo->format == VK_FORMAT_D32_SFLOAT_S8_UINT) {
            skip |= LogPerformanceWarning(
                kVUID_BestPractices_CreateImage_Depth32Format, device, error_obj.location,
                "%s Performance warning: image (%s) is created with a 32-bit depth format. Use VK_FORMAT_D24_UNORM_S8_UINT or "
                "VK_FORMAT_D16_UNORM instead, unless the extra precision is needed.",
                VendorSpecificTag(kBPVendorNVIDIA), image_hex.str().c_str());
        }
    }

    return skip;
}
