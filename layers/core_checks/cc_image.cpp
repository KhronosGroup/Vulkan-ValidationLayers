/* Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (C) 2015-2023 Google Inc.
 * Modifications Copyright (C) 2020-2022 Advanced Micro Devices, Inc. All rights reserved.
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
 *
 */

#include <string>
#include <vulkan/vk_enum_string_helper.h>

#include "core_validation.h"
#include "generated/chassis.h"
#include "generated/pnext_chain_extraction.h"

bool CoreChecks::ValidateImageFormatFeatures(const VkImageCreateInfo *pCreateInfo, const Location &loc) const {
    bool skip = false;

    // validates based on imageCreateFormatFeatures from vkspec.html#resources-image-creation-limits
    VkFormatFeatureFlags2KHR tiling_features = 0;
    const VkImageTiling image_tiling = pCreateInfo->tiling;
    const VkFormat image_format = pCreateInfo->format;

    if (image_format == VK_FORMAT_UNDEFINED) {
        // VU 01975 states format can't be undefined unless an android externalFormat
        const uint64_t external_format = GetExternalFormat(pCreateInfo->pNext);
        if ((image_tiling == VK_IMAGE_TILING_OPTIMAL) && (0 != external_format)) {
            auto it = ahb_ext_formats_map.find(external_format);
            if (it != ahb_ext_formats_map.end()) {
                tiling_features = it->second;
            }
        }
    } else if (image_tiling == VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT) {
        vvl::unordered_set<uint64_t> drm_format_modifiers;
        const VkImageDrmFormatModifierExplicitCreateInfoEXT *drm_explicit =
            vku::FindStructInPNextChain<VkImageDrmFormatModifierExplicitCreateInfoEXT>(pCreateInfo->pNext);
        const VkImageDrmFormatModifierListCreateInfoEXT *drm_implicit =
            vku::FindStructInPNextChain<VkImageDrmFormatModifierListCreateInfoEXT>(pCreateInfo->pNext);

        if (drm_explicit != nullptr) {
            drm_format_modifiers.insert(drm_explicit->drmFormatModifier);
        } else {
            // VUID 02261 makes sure its only explict or implict in parameter checking
            assert(drm_implicit != nullptr);
            for (uint32_t i = 0; i < drm_implicit->drmFormatModifierCount; i++) {
                drm_format_modifiers.insert(drm_implicit->pDrmFormatModifiers[i]);
            }
        }

        VkDrmFormatModifierPropertiesListEXT fmt_drm_props = vku::InitStructHelper();
        VkFormatProperties2 fmt_props_2 = vku::InitStructHelper(&fmt_drm_props);
        DispatchGetPhysicalDeviceFormatProperties2(physical_device, image_format, &fmt_props_2);
        std::vector<VkDrmFormatModifierPropertiesEXT> drm_properties;
        drm_properties.resize(fmt_drm_props.drmFormatModifierCount);
        fmt_drm_props.pDrmFormatModifierProperties = drm_properties.data();
        DispatchGetPhysicalDeviceFormatProperties2(physical_device, image_format, &fmt_props_2);

        for (uint32_t i = 0; i < fmt_drm_props.drmFormatModifierCount; i++) {
            if (drm_format_modifiers.find(fmt_drm_props.pDrmFormatModifierProperties[i].drmFormatModifier) !=
                drm_format_modifiers.end()) {
                tiling_features |= fmt_drm_props.pDrmFormatModifierProperties[i].drmFormatModifierTilingFeatures;
            }
        }
    } else {
        VkFormatProperties3KHR format_properties = GetPDFormatProperties(image_format);
        tiling_features = (image_tiling == VK_IMAGE_TILING_LINEAR) ? format_properties.linearTilingFeatures
                                                                   : format_properties.optimalTilingFeatures;
    }

    // Lack of disjoint format feature support while using the flag
    if (vkuFormatIsMultiplane(image_format) && ((pCreateInfo->flags & VK_IMAGE_CREATE_DISJOINT_BIT) != 0) &&
        ((tiling_features & VK_FORMAT_FEATURE_2_DISJOINT_BIT_KHR) == 0)) {
        skip |= LogError("VUID-VkImageCreateInfo-imageCreateFormatFeatures-02260", device, loc.dot(Field::usage),
                         "includes VK_IMAGE_CREATE_DISJOINT_BIT, but %s doesn't support "
                         "VK_FORMAT_FEATURE_DISJOINT_BIT (supported features: %s).",
                         string_VkFormat(pCreateInfo->format), string_VkFormatFeatureFlags2(tiling_features).c_str());
    }

    if (((tiling_features & VK_FORMAT_FEATURE_2_HOST_IMAGE_TRANSFER_BIT_EXT) == 0) &&
        (pCreateInfo->usage & VK_IMAGE_USAGE_HOST_TRANSFER_BIT_EXT)) {
        skip |= LogError("VUID-VkImageCreateInfo-imageCreateFormatFeatures-09048", device, loc.dot(Field::usage),
                         "includes VK_IMAGE_USAGE_HOST_TRANSFER_BIT_EXT, but %s doesn't support "
                         "VK_FORMAT_FEATURE_2_HOST_IMAGE_TRANSFER_BIT_EXT (supported features: %s).",
                         string_VkFormat(pCreateInfo->format), string_VkFormatFeatureFlags2(tiling_features).c_str());
    }

    return skip;
}

bool CoreChecks::PreCallValidateCreateImage(VkDevice device, const VkImageCreateInfo *pCreateInfo,
                                            const VkAllocationCallbacks *pAllocator, VkImage *pImage,
                                            const ErrorObject &error_obj) const {
    bool skip = false;
    const Location create_info_loc = error_obj.location.dot(Field::pCreateInfo);
    if (IsExtEnabled(device_extensions.vk_android_external_memory_android_hardware_buffer)) {
        skip |= ValidateCreateImageANDROID(pCreateInfo, create_info_loc);
    } else {  // These checks are omitted or replaced when Android HW Buffer extension is active
        if (pCreateInfo->format == VK_FORMAT_UNDEFINED) {
            return LogError("VUID-VkImageCreateInfo-pNext-01975", device, create_info_loc.dot(Field::format),
                            "must not be VK_FORMAT_UNDEFINED.");
        }
    }

    const VkPhysicalDeviceLimits *device_limits = &phys_dev_props.limits;
    const VkImageUsageFlags attach_flags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
                                           VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    if (pCreateInfo->usage & attach_flags) {
        if (pCreateInfo->extent.width > device_limits->maxFramebufferWidth) {
            skip |= LogError("VUID-VkImageCreateInfo-usage-00964", device, create_info_loc.dot(Field::usage),
                             "(%s) include a frame buffer attachment bit and image width (%" PRIu32
                             ") exceeds "
                             "device maxFramebufferWidth (%" PRIu32 ").",
                             string_VkImageUsageFlags(pCreateInfo->usage).c_str(), pCreateInfo->extent.width,
                             device_limits->maxFramebufferWidth);
        }
        if (pCreateInfo->extent.height > device_limits->maxFramebufferHeight) {
            skip |= LogError("VUID-VkImageCreateInfo-usage-00965", device, create_info_loc.dot(Field::usage),
                             "(%s) include a frame buffer attachment bit and image height (%" PRIu32
                             ") exceeds "
                             "device maxFramebufferHeight (%" PRIu32 ").",
                             string_VkImageUsageFlags(pCreateInfo->usage).c_str(), pCreateInfo->extent.height,
                             device_limits->maxFramebufferHeight);
        }
    }

    VkImageCreateFlags sparseFlags =
        VK_IMAGE_CREATE_SPARSE_BINDING_BIT | VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT | VK_IMAGE_CREATE_SPARSE_ALIASED_BIT;
    if ((pCreateInfo->flags & sparseFlags) && (pCreateInfo->usage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT)) {
        skip |= LogError("VUID-VkImageCreateInfo-None-01925", device, create_info_loc,
                         "images using sparse memory cannot have VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT set.");
    }

    if (!enabled_features.fragmentDensityMapOffset && (pCreateInfo->usage & VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT)) {
        uint32_t ceiling_width = static_cast<uint32_t>(ceilf(
            static_cast<float>(device_limits->maxFramebufferWidth) /
            std::max(static_cast<float>(phys_dev_ext_props.fragment_density_map_props.minFragmentDensityTexelSize.width), 1.0f)));
        if (pCreateInfo->extent.width > ceiling_width) {
            skip |= LogError("VUID-VkImageCreateInfo-fragmentDensityMapOffset-06514", device, create_info_loc.dot(Field::usage),
                             "includes VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT and image width (%" PRIu32
                             ") exceeds the "
                             "ceiling of device "
                             "maxFramebufferWidth (%" PRIu32 ") / minFragmentDensityTexelSize.width (%" PRIu32
                             "). The ceiling value: %" PRIu32 ".",
                             pCreateInfo->extent.width, device_limits->maxFramebufferWidth,
                             phys_dev_ext_props.fragment_density_map_props.minFragmentDensityTexelSize.width, ceiling_width);
        }

        uint32_t ceiling_height = static_cast<uint32_t>(ceilf(
            static_cast<float>(device_limits->maxFramebufferHeight) /
            std::max(static_cast<float>(phys_dev_ext_props.fragment_density_map_props.minFragmentDensityTexelSize.height), 1.0f)));
        if (pCreateInfo->extent.height > ceiling_height) {
            skip |= LogError("VUID-VkImageCreateInfo-fragmentDensityMapOffset-06515", device, create_info_loc.dot(Field::usage),
                             "includes VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT and image height (%" PRIu32
                             ") exceeds the "
                             "ceiling of device "
                             "maxFramebufferHeight (%" PRIu32 ") / minFragmentDensityTexelSize.height (%" PRIu32
                             "). The ceiling value: %" PRIu32 ".",
                             pCreateInfo->extent.height, device_limits->maxFramebufferHeight,
                             phys_dev_ext_props.fragment_density_map_props.minFragmentDensityTexelSize.height, ceiling_height);
        }
    }

    VkImageFormatProperties2 image_format_properties = vku::InitStructHelper();
    VkPhysicalDeviceImageFormatInfo2 image_format_info = vku::InitStructHelper();
    image_format_info.type = pCreateInfo->imageType;
    image_format_info.format = pCreateInfo->format;
    image_format_info.tiling = pCreateInfo->tiling;
    image_format_info.usage = pCreateInfo->usage;
    image_format_info.flags = pCreateInfo->flags;

    vvl::PnextChainVkPhysicalDeviceImageFormatInfo2 image_format_info_pnext_chain{};
    image_format_info.pNext = vvl::PnextChainExtract(pCreateInfo->pNext, image_format_info_pnext_chain);

    // Exit early if any thing is not succesful
    VkResult result = VK_SUCCESS;
    if (pCreateInfo->tiling != VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT) {
        if (IsExtEnabled(device_extensions.vk_khr_get_physical_device_properties2)) {
            result = DispatchGetPhysicalDeviceImageFormatProperties2(physical_device, &image_format_info, &image_format_properties);
        } else {
            result = DispatchGetPhysicalDeviceImageFormatProperties(physical_device, pCreateInfo->format, pCreateInfo->imageType,
                                                                    pCreateInfo->tiling, pCreateInfo->usage, pCreateInfo->flags,
                                                                    &image_format_properties.imageFormatProperties);
        }

        // 1. vkGetPhysicalDeviceImageFormatProperties[2] only success code is VK_SUCCESS
        // 2. If call returns an error, then "imageCreateImageFormatPropertiesList" is defined to be the empty list
        // 3. All values in 02251 are undefined if "imageCreateImageFormatPropertiesList" is empty.
        if (result != VK_SUCCESS) {
            // External memory will always have a "imageCreateImageFormatPropertiesList" so skip
#ifdef VK_USE_PLATFORM_ANDROID_KHR
            if (!vku::FindStructInPNextChain<VkExternalFormatANDROID>(pCreateInfo->pNext)) {
#endif  // VK_USE_PLATFORM_ANDROID_KHR
                Func command = IsExtEnabled(device_extensions.vk_khr_get_physical_device_properties2)
                                   ? Func::vkGetPhysicalDeviceImageFormatProperties2
                                   : Func::vkGetPhysicalDeviceImageFormatProperties;
                skip |= LogError("VUID-VkImageCreateInfo-imageCreateMaxMipLevels-02251", device, create_info_loc,
                                 "The following parameters -\n"
                                 "format (%s)\n"
                                 "type (%s)\n"
                                 "tiling (%s)\n"
                                 "usage (%s)\n"
                                 "flags (%s)\n"
                                 "returned (%s) when calling %s.",
                                 string_VkFormat(pCreateInfo->format), string_VkImageType(pCreateInfo->imageType),
                                 string_VkImageTiling(pCreateInfo->tiling), string_VkImageUsageFlags(pCreateInfo->usage).c_str(),
                                 string_VkImageCreateFlags(pCreateInfo->flags).c_str(), string_VkResult(result), String(command));
#ifdef VK_USE_PLATFORM_ANDROID_KHR
            }
#endif  // VK_USE_PLATFORM_ANDROID_KHR
        }
    } else {
        auto *modifier_list = vku::FindStructInPNextChain<VkImageDrmFormatModifierListCreateInfoEXT>(pCreateInfo->pNext);
        auto *explicit_modifier = vku::FindStructInPNextChain<VkImageDrmFormatModifierExplicitCreateInfoEXT>(pCreateInfo->pNext);
        VkPhysicalDeviceImageDrmFormatModifierInfoEXT drm_format_modifier = vku::InitStructHelper();
        drm_format_modifier.sharingMode = pCreateInfo->sharingMode;
        drm_format_modifier.queueFamilyIndexCount = pCreateInfo->queueFamilyIndexCount;
        drm_format_modifier.pQueueFamilyIndices = pCreateInfo->pQueueFamilyIndices;
        vvl::PnextChainScopedAdd scoped_add_drm_fmt_mod(&image_format_info, &drm_format_modifier);

        if (modifier_list) {
            for (uint32_t i = 0; i < modifier_list->drmFormatModifierCount; i++) {
                drm_format_modifier.drmFormatModifier = modifier_list->pDrmFormatModifiers[i];
                result =
                    DispatchGetPhysicalDeviceImageFormatProperties2(physical_device, &image_format_info, &image_format_properties);

                // The application gives a list of modifier and the driver selects one. If one is valid, stop there.
                if (result == VK_SUCCESS) {
                    break;
                }
            }
        } else if (explicit_modifier) {
            drm_format_modifier.drmFormatModifier = explicit_modifier->drmFormatModifier;
            result = DispatchGetPhysicalDeviceImageFormatProperties2(physical_device, &image_format_info, &image_format_properties);
        }

        if (result != VK_SUCCESS) {
            // Will not have to worry about VkExternalFormatANDROID if using DRM format modifier
            std::string drm_source = modifier_list ? "pDrmFormatModifiers[]" : "VkImageDrmFormatModifierExplicitCreateInfoEXT";
            skip |= LogError("VUID-VkImageCreateInfo-imageCreateMaxMipLevels-02251", device, create_info_loc,
                             "The following parameters -\n"
                             "format (%s)\n"
                             "type (%s)\n"
                             "tiling (%s)\n"
                             "usage (%s)\n"
                             "flags (%s)\n"
                             "drmFormatModifier (%" PRIu64
                             ") from %s\n"
                             "returned (%s) when calling VkGetPhysicalDeviceImageFormatProperties2.",
                             string_VkFormat(pCreateInfo->format), string_VkImageType(pCreateInfo->imageType),
                             string_VkImageTiling(pCreateInfo->tiling), string_VkImageUsageFlags(pCreateInfo->usage).c_str(),
                             string_VkImageCreateFlags(pCreateInfo->flags).c_str(), drm_format_modifier.drmFormatModifier,
                             drm_source.c_str(), string_VkResult(result));
        }
    }

    // only check if we got valid image format info back
    if (result == VK_SUCCESS) {
        const auto format_limits = image_format_properties.imageFormatProperties;
        if (pCreateInfo->mipLevels > format_limits.maxMipLevels) {
            skip |= LogError("VUID-VkImageCreateInfo-mipLevels-02255", device, create_info_loc.dot(Field::mipLevels),
                             "(%d) exceed image format maxMipLevels (%d) for format %s.", pCreateInfo->mipLevels,
                             format_limits.maxMipLevels, string_VkFormat(pCreateInfo->format));
        }

        uint64_t texel_count = static_cast<uint64_t>(pCreateInfo->extent.width) *
                               static_cast<uint64_t>(pCreateInfo->extent.height) *
                               static_cast<uint64_t>(pCreateInfo->extent.depth) * static_cast<uint64_t>(pCreateInfo->arrayLayers) *
                               static_cast<uint64_t>(pCreateInfo->samples);

        // Depth/Stencil formats size can't be accurately calculated
        if (!vkuFormatIsDepthAndStencil(pCreateInfo->format)) {
            uint64_t total_size =
                static_cast<uint64_t>(std::ceil(vkuFormatTexelSize(pCreateInfo->format) * static_cast<double>(texel_count)));

            // Round up to imageGranularity boundary
            VkDeviceSize image_granularity = phys_dev_props.limits.bufferImageGranularity;
            uint64_t ig_mask = image_granularity - 1;
            total_size = (total_size + ig_mask) & ~ig_mask;

            if (total_size > format_limits.maxResourceSize) {
                skip |= LogWarning(kVUID_Core_Image_InvalidFormatLimitsViolation, device, error_obj.location,
                                   "resource size exceeds allowable maximum Image resource size = %" PRIu64
                                   ", maximum resource size = %" PRIu64 " for format %s.",
                                   total_size, format_limits.maxResourceSize, string_VkFormat(pCreateInfo->format));
            }
        }

        if (pCreateInfo->arrayLayers > format_limits.maxArrayLayers) {
            skip |= LogError("VUID-VkImageCreateInfo-arrayLayers-02256", device, create_info_loc.dot(Field::arrayLayers),
                             "(%d) exceeds allowable maximum supported by format %s (format maxArrayLayers: %" PRIu32 ").",
                             pCreateInfo->arrayLayers, string_VkFormat(pCreateInfo->format), format_limits.maxArrayLayers);
        }

        if ((pCreateInfo->samples & format_limits.sampleCounts) == 0) {
            skip |= LogError("VUID-VkImageCreateInfo-samples-02258", device, create_info_loc.dot(Field::samples),
                             "(%s) is not supported by format %s (format sampleCounts: 0x%.8X).",
                             string_VkSampleCountFlagBits(pCreateInfo->samples), string_VkFormat(pCreateInfo->format),
                             format_limits.sampleCounts);
        }

        if (pCreateInfo->extent.width > format_limits.maxExtent.width) {
            skip |= LogError("VUID-VkImageCreateInfo-extent-02252", device, create_info_loc.dot(Field::extent).dot(Field::width),
                             "(%" PRIu32 ") exceeds allowable maximum image extent width %" PRIu32 " for format %s.",
                             pCreateInfo->extent.width, format_limits.maxExtent.width, string_VkFormat(pCreateInfo->format));
        }

        if (pCreateInfo->extent.height > format_limits.maxExtent.height) {
            skip |= LogError("VUID-VkImageCreateInfo-extent-02253", device, create_info_loc.dot(Field::extent).dot(Field::height),
                             "(%" PRIu32 ") exceeds allowable maximum image extent height %" PRIu32 " for format %s.",
                             pCreateInfo->extent.height, format_limits.maxExtent.height, string_VkFormat(pCreateInfo->format));
        }

        if (pCreateInfo->extent.depth > format_limits.maxExtent.depth) {
            skip |= LogError("VUID-VkImageCreateInfo-extent-02254", device, create_info_loc.dot(Field::extent).dot(Field::depth),
                             "(%" PRIu32 ") exceeds allowable maximum image extent depth %" PRIu32 " for format %s.",
                             pCreateInfo->extent.depth, format_limits.maxExtent.depth, string_VkFormat(pCreateInfo->format));
        }
    }

    // Tests for "Formats requiring sampler YCBCR conversion for VK_IMAGE_ASPECT_COLOR_BIT image views"
    if (FormatRequiresYcbcrConversionExplicitly(pCreateInfo->format)) {
        if (pCreateInfo->mipLevels != 1) {
            skip |= LogError("VUID-VkImageCreateInfo-format-06410", device, create_info_loc.dot(Field::mipLevels),
                             "(%d), but when using a YCbCr Conversion format (%s), mipLevels must be 1.", pCreateInfo->mipLevels,
                             string_VkFormat(pCreateInfo->format));
        }

        if (pCreateInfo->samples != VK_SAMPLE_COUNT_1_BIT) {
            skip |= LogError("VUID-VkImageCreateInfo-format-06411", device, create_info_loc.dot(Field::samples),
                             "(%s), but when using a YCbCr Conversion format (%s), samples must be "
                             "VK_SAMPLE_COUNT_1_BIT.",
                             string_VkSampleCountFlagBits(pCreateInfo->samples), string_VkFormat(pCreateInfo->format));
        }

        if (pCreateInfo->imageType != VK_IMAGE_TYPE_2D) {
            skip |= LogError("VUID-VkImageCreateInfo-format-06412", device, create_info_loc.dot(Field::imageType),
                             "(%s), but when using a YCbCr Conversion format (%s), imageType must be "
                             "VK_IMAGE_TYPE_2D.",
                             string_VkImageType(pCreateInfo->imageType), string_VkFormat(pCreateInfo->format));
        }
    }

    if (IsExtEnabled(device_extensions.vk_khr_maintenance2)) {
        if (pCreateInfo->flags & VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT) {
            if (!vkuFormatIsCompressed(pCreateInfo->format)) {
                skip |= LogError(
                    "VUID-VkImageCreateInfo-flags-01572", device, create_info_loc.dot(Field::flags),
                    "contains VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT, but format (%s) must be a compressed image format.",
                    string_VkFormat(pCreateInfo->format));
            }
            if (!(pCreateInfo->flags & VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT)) {
                skip |= LogError("VUID-VkImageCreateInfo-flags-01573", device, create_info_loc.dot(Field::flags),
                                 "contains VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT, "
                                 "flags must also contain VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT.");
            }
        }
    }

    if (pCreateInfo->sharingMode == VK_SHARING_MODE_CONCURRENT && pCreateInfo->pQueueFamilyIndices) {
        skip |= ValidatePhysicalDeviceQueueFamilies(pCreateInfo->queueFamilyIndexCount, pCreateInfo->pQueueFamilyIndices,
                                                    create_info_loc, "VUID-VkImageCreateInfo-sharingMode-01420");
    }

    if (!vkuFormatIsMultiplane(pCreateInfo->format) && !(pCreateInfo->flags & VK_IMAGE_CREATE_ALIAS_BIT) &&
        (pCreateInfo->flags & VK_IMAGE_CREATE_DISJOINT_BIT)) {
        skip |= LogError("VUID-VkImageCreateInfo-format-01577", device, create_info_loc,
                         "format is %s and flags are %s. The flags should not include VK_IMAGE_CREATE_DISJOINT_BIT.",
                         string_VkFormat(pCreateInfo->format), string_VkImageCreateFlags(pCreateInfo->flags).c_str());
    }

    const auto swapchain_create_info = vku::FindStructInPNextChain<VkImageSwapchainCreateInfoKHR>(pCreateInfo->pNext);
    if (swapchain_create_info != nullptr) {
        if (swapchain_create_info->swapchain != VK_NULL_HANDLE) {
            auto swapchain_state = Get<SWAPCHAIN_NODE>(swapchain_create_info->swapchain);
            const VkSwapchainCreateFlagsKHR swapchain_flags = swapchain_state->createInfo.flags;

            // Validate rest of Swapchain Image create check that require swapchain state
            const char *vuid = "VUID-VkImageSwapchainCreateInfoKHR-swapchain-00995";
            if (((swapchain_flags & VK_SWAPCHAIN_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT_KHR) != 0) &&
                ((pCreateInfo->flags & VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT) == 0)) {
                skip |= LogError(vuid, device, create_info_loc.pNext(Struct::VkImageSwapchainCreateInfoKHR, Field::swapchain),
                                 "was created with VK_SWAPCHAIN_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT_KHR flag so "
                                 "all swapchain images must have the VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT flag set.");
            }
            if (((swapchain_flags & VK_SWAPCHAIN_CREATE_PROTECTED_BIT_KHR) != 0) &&
                ((pCreateInfo->flags & VK_IMAGE_CREATE_PROTECTED_BIT) == 0)) {
                skip |= LogError(vuid, device, create_info_loc.pNext(Struct::VkImageSwapchainCreateInfoKHR, Field::swapchain),
                                 "was created with VK_SWAPCHAIN_CREATE_PROTECTED_BIT_KHR flag so all "
                                 "swapchain images must have the VK_IMAGE_CREATE_PROTECTED_BIT flag set.");
            }
            const VkImageCreateFlags mutable_flags = (VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT | VK_IMAGE_CREATE_EXTENDED_USAGE_BIT);
            if (((swapchain_flags & VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR) != 0) &&
                ((pCreateInfo->flags & mutable_flags) != mutable_flags)) {
                skip |= LogError(vuid, device, create_info_loc.pNext(Struct::VkImageSwapchainCreateInfoKHR, Field::swapchain),
                                 "was created with VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR flag so "
                                 "all swapchain images must have the VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT and "
                                 "VK_IMAGE_CREATE_EXTENDED_USAGE_BIT flags both set.");
            }
        }
    }

    if ((pCreateInfo->flags & VK_IMAGE_CREATE_PROTECTED_BIT) != 0) {
        if (enabled_features.protectedMemory == VK_FALSE) {
            skip |= LogError("VUID-VkImageCreateInfo-flags-01890", device, create_info_loc.dot(Field::flags),
                             "has VK_IMAGE_CREATE_PROTECTED_BIT set, but the protectedMemory device feature is not enabled.");
        }
        const VkImageCreateFlags invalid_flags =
            VK_IMAGE_CREATE_SPARSE_BINDING_BIT | VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT | VK_IMAGE_CREATE_SPARSE_ALIASED_BIT;
        if ((pCreateInfo->flags & invalid_flags) != 0) {
            skip |= LogError("VUID-VkImageCreateInfo-None-01891", device, create_info_loc.dot(Field::flags),
                             "can't have both protected and sparse flags set.");
        }
    }

    if ((pCreateInfo->flags & VK_IMAGE_CREATE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_BIT_EXT) != 0) {
        if (!(enabled_features.multisampledRenderToSingleSampled)) {
            skip |= LogError("VUID-VkImageCreateInfo-multisampledRenderToSingleSampled-06882", device,
                             create_info_loc.dot(Field::flags),
                             "contains VK_IMAGE_CREATE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_BIT_EXT but the "
                             "multisampledRenderToSingleSampled feature is not enabled.");
        }
        if (pCreateInfo->samples != VK_SAMPLE_COUNT_1_BIT) {
            skip |= LogError("VUID-VkImageCreateInfo-flags-06883", device, create_info_loc.dot(Field::flags),
                             "contains VK_IMAGE_CREATE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_BIT_EXT but samples (%s) is not equal "
                             "to VK_SAMPLE_COUNT_1_BIT.",
                             string_VkSampleCountFlagBits(pCreateInfo->samples));
        }
    }

    skip |= ValidateImageFormatFeatures(pCreateInfo, create_info_loc);

    // Check compatibility with VK_KHR_portability_subset
    if (IsExtEnabled(device_extensions.vk_khr_portability_subset)) {
        if (VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT & pCreateInfo->flags && VK_FALSE == enabled_features.imageView2DOn3DImage) {
            skip |= LogError("VUID-VkImageCreateInfo-imageView2DOn3DImage-04459", device, create_info_loc,
                             "(portability error) VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT is not supported.");
        }
        if ((VK_SAMPLE_COUNT_1_BIT != pCreateInfo->samples) && (1 != pCreateInfo->arrayLayers) &&
            (VK_FALSE == enabled_features.multisampleArrayImage)) {
            skip |= LogError("VUID-VkImageCreateInfo-multisampleArrayImage-04460", device, create_info_loc,
                             "(portability error) Cannot create an image with samples/texel > 1 && arrayLayers != 1");
        }
    }

    const auto external_memory_create_info_nv = vku::FindStructInPNextChain<VkExternalMemoryImageCreateInfoNV>(pCreateInfo->pNext);
    const auto external_memory_create_info = vku::FindStructInPNextChain<VkExternalMemoryImageCreateInfo>(pCreateInfo->pNext);
    if (external_memory_create_info_nv != nullptr && external_memory_create_info != nullptr) {
        skip |= LogError("VUID-VkImageCreateInfo-pNext-00988", device, create_info_loc,
                         "has both VkExternalMemoryImageCreateInfoNV and "
                         "VkExternalMemoryImageCreateInfo chained structs.");
    }
    if (external_memory_create_info && external_memory_create_info->handleTypes != 0) {
        if (pCreateInfo->initialLayout != VK_IMAGE_LAYOUT_UNDEFINED) {
            skip |= LogError("VUID-VkImageCreateInfo-pNext-01443", device,
                             create_info_loc.pNext(Struct::VkExternalMemoryImageCreateInfo, Field::handleTypes),
                             "is %" PRIu32 " but pCreateInfo->initialLayout is %s.", external_memory_create_info->handleTypes,
                             string_VkImageLayout(pCreateInfo->initialLayout));
        }
        // Check external memory handle types compatibility
        const uint32_t any_type = 1u << MostSignificantBit(external_memory_create_info->handleTypes);
        VkPhysicalDeviceExternalImageFormatInfo external_image_info = vku::InitStructHelper();
        external_image_info.handleType = static_cast<VkExternalMemoryHandleTypeFlagBits>(any_type);
        vvl::PnextChainScopedAdd scoped_add_ext_img_info(&image_format_info, &external_image_info);

        VkExternalImageFormatProperties external_image_properties = vku::InitStructHelper();
        VkImageFormatProperties2 image_properties = vku::InitStructHelper(&external_image_properties);
        VkExternalMemoryHandleTypeFlags compatible_types = 0;
        if (pCreateInfo->tiling != VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT) {
            result = DispatchGetPhysicalDeviceImageFormatProperties2(physical_device, &image_format_info, &image_properties);
            compatible_types = external_image_properties.externalMemoryProperties.compatibleHandleTypes;
        } else {
            auto modifier_list = vku::FindStructInPNextChain<VkImageDrmFormatModifierListCreateInfoEXT>(pCreateInfo->pNext);
            auto explicit_modifier = vku::FindStructInPNextChain<VkImageDrmFormatModifierExplicitCreateInfoEXT>(pCreateInfo->pNext);
            VkPhysicalDeviceImageDrmFormatModifierInfoEXT drm_format_modifier = vku::InitStructHelper();
            drm_format_modifier.sharingMode = pCreateInfo->sharingMode;
            drm_format_modifier.queueFamilyIndexCount = pCreateInfo->queueFamilyIndexCount;
            drm_format_modifier.pQueueFamilyIndices = pCreateInfo->pQueueFamilyIndices;
            vvl::PnextChainScopedAdd scoped_add_drm_fmt_mod(&image_format_info, &drm_format_modifier);

            if (modifier_list) {
                for (uint32_t i = 0; i < modifier_list->drmFormatModifierCount; i++) {
                    drm_format_modifier.drmFormatModifier = modifier_list->pDrmFormatModifiers[i];
                    result =
                        DispatchGetPhysicalDeviceImageFormatProperties2(physical_device, &image_format_info, &image_properties);
                    if (result == VK_SUCCESS) {
                        compatible_types = external_image_properties.externalMemoryProperties.compatibleHandleTypes;
                        if ((external_memory_create_info->handleTypes & compatible_types) ==
                            external_memory_create_info->handleTypes)
                            break;
                    }
                }
                if (compatible_types != 0) result = VK_SUCCESS;
            } else if (explicit_modifier) {
                drm_format_modifier.drmFormatModifier = explicit_modifier->drmFormatModifier;
                result = DispatchGetPhysicalDeviceImageFormatProperties2(physical_device, &image_format_info, &image_properties);
                compatible_types = external_image_properties.externalMemoryProperties.compatibleHandleTypes;
            }
        }

        if (result != VK_SUCCESS) {
            skip |= LogError(
                "VUID-VkImageCreateInfo-pNext-00990", device, create_info_loc,
                "The handle type (%s), format (%s), type (%s), tiling (%s), usage (%s), flags (%s) "
                "is not supported combination of parameters and vkGetPhysicalDeviceImageFormatProperties2 returned back %s.",
                string_VkExternalMemoryHandleTypeFlagBits(external_image_info.handleType),
                string_VkFormat(image_format_info.format), string_VkImageType(image_format_info.type),
                string_VkImageTiling(image_format_info.tiling), string_VkImageUsageFlags(image_format_info.usage).c_str(),
                string_VkImageCreateFlags(image_format_info.flags).c_str(), string_VkResult(result));
        } else if ((external_memory_create_info->handleTypes & compatible_types) != external_memory_create_info->handleTypes) {
            skip |= LogError(
                "VUID-VkImageCreateInfo-pNext-00990", device,
                create_info_loc.pNext(Struct::VkExternalMemoryImageCreateInfo, Field::handleTypes),
                "(%s) is not reported as compatible by vkGetPhysicalDeviceImageFormatProperties2. Compatible types are %s.",
                string_VkExternalMemoryHandleTypeFlags(external_memory_create_info->handleTypes).c_str(),
                string_VkExternalMemoryHandleTypeFlags(compatible_types).c_str());
        }
    } else if (external_memory_create_info_nv && external_memory_create_info_nv->handleTypes != 0) {
        if (pCreateInfo->initialLayout != VK_IMAGE_LAYOUT_UNDEFINED) {
            skip |= LogError("VUID-VkImageCreateInfo-pNext-01443", device,
                             create_info_loc.pNext(Struct::VkExternalMemoryImageCreateInfoNV, Field::handleTypes),
                             "is %" PRIu32 " but pCreateInfo->initialLayout is %s.", external_memory_create_info_nv->handleTypes,
                             string_VkImageLayout(pCreateInfo->initialLayout));
        }
        // Check external memory handle types compatibility
        const uint32_t any_type = 1u << MostSignificantBit(external_memory_create_info_nv->handleTypes);
        auto handle_type = static_cast<VkExternalMemoryHandleTypeFlagBitsNV>(any_type);
        VkExternalImageFormatPropertiesNV external_image_properties = {};
        result = DispatchGetPhysicalDeviceExternalImageFormatPropertiesNV(
            physical_device, pCreateInfo->format, pCreateInfo->imageType, pCreateInfo->tiling, pCreateInfo->usage,
            pCreateInfo->flags, handle_type, &external_image_properties);
        const auto compatible_types = external_image_properties.compatibleHandleTypes;

        if (result != VK_SUCCESS) {
            skip |= LogError("VUID-VkImageCreateInfo-pNext-00991", device, create_info_loc,
                             "The handle type (%s), format (%s), type (%s), tiling (%s), usage (%s), flags (%s) "
                             "is not supported combination of parameters and vkGetPhysicalDeviceExternalImageFormatPropertiesNV "
                             "returned back %s.",
                             string_VkExternalMemoryHandleTypeFlagBitsNV(handle_type), string_VkFormat(pCreateInfo->format),
                             string_VkImageType(pCreateInfo->imageType), string_VkImageTiling(pCreateInfo->tiling),
                             string_VkImageUsageFlags(pCreateInfo->usage).c_str(),
                             string_VkImageCreateFlags(pCreateInfo->flags).c_str(), string_VkResult(result));
        } else if ((external_memory_create_info_nv->handleTypes & compatible_types) !=
                   external_memory_create_info_nv->handleTypes) {
            skip |= LogError("VUID-VkImageCreateInfo-pNext-00991", device,
                             create_info_loc.pNext(Struct::VkExternalMemoryImageCreateInfoNV, Field::handleTypes),
                             "(%s) is not reported as compatible by vkGetPhysicalDeviceExternalImageFormatPropertiesNV.",
                             string_VkExternalMemoryHandleTypeFlagsNV(external_memory_create_info_nv->handleTypes).c_str());
        }
    }

    if (device_group_create_info.physicalDeviceCount == 1) {
        if (pCreateInfo->flags & VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT) {
            skip |= LogError("VUID-VkImageCreateInfo-physicalDeviceCount-01421", device, create_info_loc.dot(Field::flags),
                             "contains VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT, but the device was created with "
                             "VkDeviceGroupDeviceCreateInfo::physicalDeviceCount equal to 1. Device creation with "
                             "VkDeviceGroupDeviceCreateInfo::physicalDeviceCount equal to 1 may have been implicit.");
        }
    }

    if ((pCreateInfo->flags & VK_IMAGE_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT) &&
        !enabled_features.descriptorBufferCaptureReplay) {
        skip |= LogError("VUID-VkImageCreateInfo-flags-08104", device, create_info_loc.dot(Field::flags),
                         "contains VK_IMAGE_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT but the descriptorBufferCaptureReplay "
                         "feature is not enabled.");
    }

    auto opaque_capture_descriptor_buffer = vku::FindStructInPNextChain<VkOpaqueCaptureDescriptorDataCreateInfoEXT>(pCreateInfo->pNext);
    if (opaque_capture_descriptor_buffer && !(pCreateInfo->flags & VK_IMAGE_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT)) {
        skip |= LogError("VUID-VkImageCreateInfo-pNext-08105", device, create_info_loc.dot(Field::flags),
                         "(%s) does not have VK_IMAGE_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT, but "
                         "VkOpaqueCaptureDescriptorDataCreateInfoEXT is in pNext chain.",
                         string_VkImageCreateFlags(pCreateInfo->flags).c_str());
    }

    bool has_decode_usage =
        pCreateInfo->usage & (VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR | VK_IMAGE_USAGE_VIDEO_DECODE_SRC_BIT_KHR |
                              VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR);
    bool has_encode_usage =
        pCreateInfo->usage & (VK_IMAGE_USAGE_VIDEO_ENCODE_DST_BIT_KHR | VK_IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT_KHR |
                              VK_IMAGE_USAGE_VIDEO_ENCODE_DPB_BIT_KHR);
    if (has_decode_usage || has_encode_usage) {
        const auto *video_profiles = vku::FindStructInPNextChain<VkVideoProfileListInfoKHR>(pCreateInfo->pNext);
        skip |= ValidateVideoProfileListInfo(video_profiles, device, "vkCreateImage", has_decode_usage,
                                             "VUID-VkImageCreateInfo-usage-04815", has_encode_usage,
                                             "VUID-VkImageCreateInfo-usage-04816");

        if (video_profiles && video_profiles->profileCount > 0) {
            auto format_props_list = GetVideoFormatProperties(pCreateInfo->usage, video_profiles);

            bool supported_video_format = false;
            for (auto &format_props : format_props_list) {
                if (pCreateInfo->format == format_props.format &&
                    (pCreateInfo->flags & format_props.imageCreateFlags) == pCreateInfo->flags &&
                    pCreateInfo->imageType == format_props.imageType && pCreateInfo->tiling == format_props.imageTiling &&
                    (pCreateInfo->usage & format_props.imageUsageFlags) == pCreateInfo->usage) {
                    supported_video_format = true;
                }
            }

            if (!supported_video_format) {
                skip |=
                    LogError("VUID-VkImageCreateInfo-pNext-06811", device, create_info_loc,
                             "image creation parameters (flags: 0x%08x, format: %s, imageType: %s, "
                             "tiling: %s) are not supported by any of the supported video format properties for "
                             "the video profiles specified in the VkVideoProfileListInfoKHR structure included in "
                             "the pCreateInfo->pNext chain, as reported by "
                             "vkGetPhysicalDeviceVideoFormatPropertiesKHR for the same video profiles "
                             "and the image usage flags specified in pCreateInfo->usage (0x%08x)",
                             pCreateInfo->flags, string_VkFormat(pCreateInfo->format), string_VkImageType(pCreateInfo->imageType),
                             string_VkImageTiling(pCreateInfo->tiling), pCreateInfo->usage);
            }
        }
    }

    return skip;
}

void CoreChecks::PostCallRecordCreateImage(VkDevice device, const VkImageCreateInfo *pCreateInfo,
                                           const VkAllocationCallbacks *pAllocator, VkImage *pImage,
                                           const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;

    StateTracker::PostCallRecordCreateImage(device, pCreateInfo, pAllocator, pImage, record_obj);
    if ((pCreateInfo->flags & VK_IMAGE_CREATE_SPARSE_BINDING_BIT) != 0) {
        // non-sparse images set up their layout maps when memory is bound
        auto image_state = Get<IMAGE_STATE>(*pImage);
        image_state->SetInitialLayoutMap();
    }
}

bool CoreChecks::PreCallValidateDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks *pAllocator,
                                             const ErrorObject &error_obj) const {
    auto image_state = Get<IMAGE_STATE>(image);
    bool skip = false;
    if (image_state) {
        if (image_state->IsSwapchainImage() && image_state->owned_by_swapchain) {
            skip |= LogError("VUID-vkDestroyImage-image-04882", image, error_obj.location.dot(Field::image),
                             "%s is a presentable image controlled by the implementation and must be destroyed "
                             "with vkDestroySwapchainKHR.",
                             FormatHandle(image_state->image()).c_str());
        }
        skip |= ValidateObjectNotInUse(image_state.get(), error_obj.location, "VUID-vkDestroyImage-image-01000");
    }
    return skip;
}

void CoreChecks::PreCallRecordDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks *pAllocator) {
    // Clean up validation specific data
    auto image_state = Get<IMAGE_STATE>(image);
    qfo_release_image_barrier_map.erase(image);
    // Clean up generic image state
    StateTracker::PreCallRecordDestroyImage(device, image, pAllocator);
}

bool CoreChecks::ValidateClearImageSubresourceRange(const CMD_BUFFER_STATE &cb_state, const IMAGE_STATE &image_state,
                                                    const VkImageSubresourceRange &range, const Location &loc) const {
    bool skip = false;

    if (range.aspectMask != VK_IMAGE_ASPECT_COLOR_BIT) {
        LogObjectList objlist(cb_state.commandBuffer(), image_state.image());
        skip |= LogError("VUID-vkCmdClearColorImage-aspectMask-02498", objlist, loc.dot(Field::aspectMask),
                         "is %s (must only include COLOR_BIT).", string_VkImageAspectFlags(range.aspectMask).c_str());
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                   const VkClearColorValue *pColor, uint32_t rangeCount,
                                                   const VkImageSubresourceRange *pRanges, const ErrorObject &error_obj) const {
    bool skip = false;
    // TODO : Verify memory is in VK_IMAGE_STATE_CLEAR state
    auto cb_state_ptr = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    auto image_state_ptr = Get<IMAGE_STATE>(image);
    if (!cb_state_ptr || !image_state_ptr) {
        return skip;
    }
    const auto &cb_state = *cb_state_ptr;
    const auto &image_state = *image_state_ptr;
    const Location image_loc = error_obj.location.dot(Field::image);
    skip |= ValidateMemoryIsBoundToImage(LogObjectList(commandBuffer, image), image_state, image_loc,
                                         "VUID-vkCmdClearColorImage-image-00003");
    skip |= ValidateCmd(cb_state, error_obj.location);
    if (IsExtEnabled(device_extensions.vk_khr_maintenance1)) {
        skip |= ValidateImageFormatFeatureFlags(commandBuffer, image_state, VK_FORMAT_FEATURE_2_TRANSFER_DST_BIT_KHR, image_loc,
                                                "VUID-vkCmdClearColorImage-image-01993");
    }
    skip |= ValidateProtectedImage(cb_state, image_state, image_loc, "VUID-vkCmdClearColorImage-commandBuffer-01805");
    skip |= ValidateUnprotectedImage(cb_state, image_state, image_loc, "VUID-vkCmdClearColorImage-commandBuffer-01806");
    for (uint32_t i = 0; i < rangeCount; ++i) {
        const Location range_loc = error_obj.location.dot(Field::pRanges, i);
        skip |= ValidateCmdClearColorSubresourceRange(image_state, pRanges[i], range_loc);
        skip |= ValidateClearImageSubresourceRange(cb_state, image_state, pRanges[i], range_loc);
        skip |= VerifyClearImageLayout(cb_state, image_state, pRanges[i], imageLayout, range_loc);
    }

    const VkFormat format = image_state.createInfo.format;
    if (vkuFormatIsDepthOrStencil(format)) {
        LogObjectList objlist(commandBuffer, image);
        skip |=
            LogError("VUID-vkCmdClearColorImage-image-00007", objlist, image_loc,
                     "(%s) was created with a depth/stencil format (%s).", FormatHandle(image).c_str(), string_VkFormat(format));
    } else if (vkuFormatIsCompressed(format)) {
        LogObjectList objlist(commandBuffer, image);
        skip |= LogError("VUID-vkCmdClearColorImage-image-00007", objlist, image_loc,
                         "(%s) was created with a compressed format (%s).", FormatHandle(image).c_str(), string_VkFormat(format));
    }

    if (!(image_state.createInfo.usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT)) {
        LogObjectList objlist(commandBuffer, image);
        skip |= LogError("VUID-vkCmdClearColorImage-image-00002", objlist, image_loc,
                         "(%s) was created with usage %s (missing VK_IMAGE_USAGE_TRANSFER_DST_BIT).", FormatHandle(image).c_str(),
                         string_VkImageUsageFlags(image_state.createInfo.usage).c_str());
    }

    // Tests for "Formats requiring sampler Y’CBCR conversion for VK_IMAGE_ASPECT_COLOR_BIT image views"
    if (FormatRequiresYcbcrConversionExplicitly(format)) {
        LogObjectList objlist(commandBuffer, image);
        skip |= LogError("VUID-vkCmdClearColorImage-image-01545", objlist, image_loc, "(%s) was created with format %s.",
                         FormatHandle(image).c_str(), string_VkFormat(format));
    }
    return skip;
}

void CoreChecks::PreCallRecordCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                 const VkClearColorValue *pColor, uint32_t rangeCount,
                                                 const VkImageSubresourceRange *pRanges) {
    StateTracker::PreCallRecordCmdClearColorImage(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);

    auto cb_state_ptr = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    auto image_state = Get<IMAGE_STATE>(image);
    if (cb_state_ptr && image_state) {
        for (uint32_t i = 0; i < rangeCount; ++i) {
            cb_state_ptr->SetImageInitialLayout(image, pRanges[i], imageLayout);
        }
    }
}

bool CoreChecks::ValidateClearDepthStencilValue(VkCommandBuffer commandBuffer, VkClearDepthStencilValue clearValue,
                                                const Location &loc) const {
    bool skip = false;

    if (!IsExtEnabled(device_extensions.vk_ext_depth_range_unrestricted)) {
        if (!(clearValue.depth >= 0.0) || !(clearValue.depth <= 1.0)) {
            skip |=
                LogError("VUID-VkClearDepthStencilValue-depth-00022", commandBuffer, loc.dot(Field::depth),
                         "is %f (not within the [0.0, 1.0] range) but VK_EXT_depth_range_unrestricted extension is not enabled.",
                         clearValue.depth);
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                          const VkClearDepthStencilValue *pDepthStencil, uint32_t rangeCount,
                                                          const VkImageSubresourceRange *pRanges,
                                                          const ErrorObject &error_obj) const {
    bool skip = false;

    // TODO : Verify memory is in VK_IMAGE_STATE_CLEAR state
    auto cb_state_ptr = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    auto image_state_ptr = Get<IMAGE_STATE>(image);
    if (!cb_state_ptr || !image_state_ptr) {
        return skip;
    }
    const auto &cb_state = *cb_state_ptr;
    const auto &image_state = *image_state_ptr;
    const Location image_loc = error_obj.location.dot(Field::image);

    const VkFormat image_format = image_state.createInfo.format;
    skip |= ValidateMemoryIsBoundToImage(LogObjectList(commandBuffer, image), image_state, image_loc,
                                         "VUID-vkCmdClearDepthStencilImage-image-00010");
    skip |= ValidateCmd(cb_state, error_obj.location);
    if (IsExtEnabled(device_extensions.vk_khr_maintenance1)) {
        skip |= ValidateImageFormatFeatureFlags(commandBuffer, image_state, VK_FORMAT_FEATURE_2_TRANSFER_DST_BIT_KHR, image_loc,
                                                "VUID-vkCmdClearDepthStencilImage-image-01994");
    }
    skip |= ValidateClearDepthStencilValue(commandBuffer, *pDepthStencil, error_obj.location.dot(Field::pDepthStencil));
    skip |= ValidateProtectedImage(cb_state, image_state, image_loc, "VUID-vkCmdClearDepthStencilImage-commandBuffer-01807");
    skip |= ValidateUnprotectedImage(cb_state, image_state, image_loc, "VUID-vkCmdClearDepthStencilImage-commandBuffer-01808");

    const auto image_stencil_struct = vku::FindStructInPNextChain<VkImageStencilUsageCreateInfo>(image_state.createInfo.pNext);
    for (uint32_t i = 0; i < rangeCount; ++i) {
        const Location range_loc = error_obj.location.dot(Field::pRanges, i);
        skip |= ValidateCmdClearDepthSubresourceRange(image_state, pRanges[i], range_loc);
        skip |= VerifyClearImageLayout(cb_state, image_state, pRanges[i], imageLayout, range_loc);
        // Image aspect must be depth or stencil or both
        VkImageAspectFlags valid_aspects = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        if (((pRanges[i].aspectMask & valid_aspects) == 0) || ((pRanges[i].aspectMask & ~valid_aspects) != 0)) {
            LogObjectList objlist(cb_state.commandBuffer(), image);
            skip |=
                LogError("VUID-vkCmdClearDepthStencilImage-aspectMask-02824", objlist, range_loc.dot(Field::aspectMask),
                         "is %s (can only be DEPTH_BIT or STENCIL_BIT).", string_VkImageAspectFlags(pRanges[i].aspectMask).c_str());
        }
        if ((pRanges[i].aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT) != 0) {
            if (vkuFormatHasDepth(image_format) == false) {
                LogObjectList objlist(cb_state.commandBuffer(), image);
                skip |= LogError("VUID-vkCmdClearDepthStencilImage-image-02826", objlist, range_loc.dot(Field::aspectMask),
                                 "has a VK_IMAGE_ASPECT_DEPTH_BIT but %s "
                                 "doesn't have a depth component.",
                                 string_VkFormat(image_format));
            }
            if ((image_state.createInfo.usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT) == 0) {
                LogObjectList objlist(cb_state.commandBuffer(), image);
                skip |= LogError(
                    "VUID-vkCmdClearDepthStencilImage-pRanges-02660", objlist, range_loc.dot(Field::aspectMask),
                    "includes VK_IMAGE_ASPECT_DEPTH_BIT, but the image was not created with VK_IMAGE_USAGE_TRANSFER_DST_BIT.");
            }
        }
        if ((pRanges[i].aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT) != 0) {
            if (vkuFormatHasStencil(image_format) == false) {
                LogObjectList objlist(cb_state.commandBuffer(), image);
                skip |= LogError("VUID-vkCmdClearDepthStencilImage-image-02825", objlist, range_loc.dot(Field::aspectMask),
                                 "has a VK_IMAGE_ASPECT_STENCIL_BIT but "
                                 "%s doesn't have a stencil component.",
                                 string_VkFormat(image_format));
            }

            if (image_stencil_struct != nullptr) {
                if ((image_stencil_struct->stencilUsage & VK_IMAGE_USAGE_TRANSFER_DST_BIT) == 0) {
                    LogObjectList objlist(cb_state.commandBuffer(), image);
                    skip |= LogError("VUID-vkCmdClearDepthStencilImage-pRanges-02658", objlist, range_loc.dot(Field::aspectMask),
                                     "includes VK_IMAGE_ASPECT_STENCIL_BIT and "
                                     "image was created with VkImageStencilUsageCreateInfo::stencilUsage = %s.",
                                     string_VkImageUsageFlags(image_stencil_struct->stencilUsage).c_str());
                }
            } else if ((image_state.createInfo.usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT) == 0) {
                LogObjectList objlist(cb_state.commandBuffer(), image);
                skip |= LogError("VUID-vkCmdClearDepthStencilImage-pRanges-02659", objlist, range_loc.dot(Field::aspectMask),
                                 "includes VK_IMAGE_ASPECT_STENCIL_BIT and "
                                 "image was not created with VkImageStencilUsageCreateInfo, but was created with "
                                 "VK_IMAGE_USAGE_TRANSFER_DST_BIT.");
            }
        }
    }

    if (!vkuFormatIsDepthOrStencil(image_format)) {
        LogObjectList objlist(cb_state.commandBuffer(), image);
        skip |=
            LogError("VUID-vkCmdClearDepthStencilImage-image-00014", objlist, image_loc,
                     "(%s) doesn't have a depth/stencil format (%s).", FormatHandle(image).c_str(), string_VkFormat(image_format));
    }
    if (VK_IMAGE_USAGE_TRANSFER_DST_BIT != (VK_IMAGE_USAGE_TRANSFER_DST_BIT & image_state.createInfo.usage)) {
        LogObjectList objlist(cb_state.commandBuffer(), image);
        skip |= LogError("VUID-vkCmdClearDepthStencilImage-pRanges-02659", objlist, image_loc,
                         "(%s) was not created with the "
                         "VK_IMAGE_USAGE_TRANSFER_DST_BIT set.",
                         FormatHandle(image).c_str());
    }
    return skip;
}

void CoreChecks::PreCallRecordCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                        const VkClearDepthStencilValue *pDepthStencil, uint32_t rangeCount,
                                                        const VkImageSubresourceRange *pRanges) {
    StateTracker::PreCallRecordCmdClearDepthStencilImage(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges);

    auto cb_state_ptr = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    auto image_state = Get<IMAGE_STATE>(image);
    if (cb_state_ptr && image_state) {
        for (uint32_t i = 0; i < rangeCount; ++i) {
            cb_state_ptr->SetImageInitialLayout(image, pRanges[i], imageLayout);
        }
    }
}

// Returns true if sub_rect is entirely contained within rect
static inline bool ContainsRect(VkRect2D rect, VkRect2D sub_rect) {
    if ((sub_rect.offset.x < rect.offset.x) || (sub_rect.offset.x + sub_rect.extent.width > rect.offset.x + rect.extent.width) ||
        (sub_rect.offset.y < rect.offset.y) || (sub_rect.offset.y + sub_rect.extent.height > rect.offset.y + rect.extent.height)) {
        return false;
    }
    return true;
}

static std::string string_VkRect2D(VkRect2D rect) {
    std::stringstream ss;
    ss << "offset.x: " << rect.offset.x << ", offset.y: " << rect.offset.y << ", extent.width: " << rect.extent.width
       << ", extent.height: " << rect.extent.height;
    return ss.str();
}

bool CoreChecks::ValidateClearAttachmentExtent(const CMD_BUFFER_STATE &cb_state, const VkRect2D &render_area,
                                               uint32_t render_pass_layer_count, uint32_t rect_count,
                                               const VkClearRect *clear_rects, const Location &loc) const {
    bool skip = false;

    for (uint32_t i = 0; i < rect_count; i++) {
        if (!ContainsRect(render_area, clear_rects[i].rect)) {
            skip |=
                LogError("VUID-vkCmdClearAttachments-pRects-00016", cb_state.Handle(), loc.dot(Field::pRects, i).dot(Field::rect),
                         "(%s) is not contained in the area of "
                         "the current render pass instance (%s).",
                         string_VkRect2D(clear_rects[i].rect).c_str(), string_VkRect2D(render_area).c_str());
        }

        const uint32_t rect_base_layer = clear_rects[i].baseArrayLayer;
        const uint32_t rect_layer_count = clear_rects[i].layerCount;
        // The layer indices specified by elements of pRects must be inferior to render pass layer count
        if (rect_base_layer + rect_layer_count > render_pass_layer_count) {
            skip |= LogError(
                "VUID-vkCmdClearAttachments-pRects-06937", cb_state.Handle(), loc.dot(Field::pRects, i).dot(Field::baseArrayLayer),
                "(%" PRIu32 ") + layerCount (%" PRIu32 ") (sum: %" PRIu32
                "), is larger then the number of layers rendered to in the current render pass instance (%" PRIu32 ").",
                rect_base_layer, rect_layer_count, rect_base_layer + rect_layer_count, render_pass_layer_count);
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                                    const VkClearAttachment *pAttachments, uint32_t rectCount,
                                                    const VkClearRect *pRects, const ErrorObject &error_obj) const {
    bool skip = false;
    auto cb_state_ptr = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    if (!cb_state_ptr) {
        return skip;
    }
    const CMD_BUFFER_STATE &cb_state = *cb_state_ptr;

    skip |= ValidateCmd(cb_state, error_obj.location);
    if (skip) return skip;  // basic validation failed, might have null pointers

    // Validate that attachments are in reference list of active subpass
    const auto &render_area = (cb_state.activeRenderPass->use_dynamic_rendering)
                                  ? cb_state.activeRenderPass->dynamic_rendering_begin_rendering_info.renderArea
                                  : cb_state.active_render_pass_begin_info.renderArea;

    if (cb_state.createInfo.level == VK_COMMAND_BUFFER_LEVEL_PRIMARY) {
        uint32_t layer_count = 0;
        if (cb_state.activeRenderPass->UsesDynamicRendering()) {
            layer_count = cb_state.activeRenderPass->dynamic_rendering_begin_rendering_info.layerCount;
        } else {
            layer_count = cb_state.activeFramebuffer.get()->createInfo.layers;
        }
        skip |= ValidateClearAttachmentExtent(cb_state, render_area, layer_count, rectCount, pRects, error_obj.location);
    }

    for (uint32_t attachment_index = 0; attachment_index < attachmentCount; attachment_index++) {
        const Location &attachment_loc = error_obj.location.dot(Field::pAttachments, attachment_index);
        auto clear_desc = &pAttachments[attachment_index];

        const VkImageAspectFlags aspect_mask = clear_desc->aspectMask;

        bool is_valid_color_attachment_index = false;
        const IMAGE_VIEW_STATE *color_view_state = nullptr;
        uint32_t color_attachment_count = 0;

        const IMAGE_VIEW_STATE *depth_view_state = nullptr;
        const IMAGE_VIEW_STATE *stencil_view_state = nullptr;

        uint32_t view_mask = 0;
        bool external_format_resolve = false;

        if (cb_state.activeRenderPass->UsesDynamicRendering()) {
            is_valid_color_attachment_index = cb_state.IsValidDynamicColorAttachmentImageIndex(clear_desc->colorAttachment);
            color_view_state = cb_state.GetActiveAttachmentImageViewState(
                cb_state.GetDynamicColorAttachmentImageIndex(clear_desc->colorAttachment));
            color_attachment_count = cb_state.GetDynamicColorAttachmentCount();

            depth_view_state = cb_state.GetActiveAttachmentImageViewState(cb_state.GetDynamicDepthAttachmentImageIndex());
            stencil_view_state = cb_state.GetActiveAttachmentImageViewState(cb_state.GetDynamicStencilAttachmentImageIndex());

            view_mask = cb_state.activeRenderPass->dynamic_rendering_begin_rendering_info.viewMask;
            external_format_resolve = cb_state.HasExternalFormatResolveAttachment();
        } else {
            const auto *renderpass_create_info = cb_state.activeRenderPass->createInfo.ptr();
            const auto *subpass_desc = &renderpass_create_info->pSubpasses[cb_state.GetActiveSubpass()];
            const auto *framebuffer = cb_state.activeFramebuffer.get();

            is_valid_color_attachment_index = (clear_desc->colorAttachment == VK_ATTACHMENT_UNUSED);
            if (subpass_desc) {
                is_valid_color_attachment_index |= clear_desc->colorAttachment < subpass_desc->colorAttachmentCount;

                if (framebuffer && (clear_desc->colorAttachment != VK_ATTACHMENT_UNUSED) &&
                    (clear_desc->colorAttachment < subpass_desc->colorAttachmentCount)) {
                    if (subpass_desc->pColorAttachments[clear_desc->colorAttachment].attachment <
                        framebuffer->createInfo.attachmentCount) {
                        color_view_state = cb_state.GetActiveAttachmentImageViewState(
                            subpass_desc->pColorAttachments[clear_desc->colorAttachment].attachment);

                        if (subpass_desc->pResolveAttachments) {
                            const uint32_t resolve_attachment =
                                subpass_desc->pResolveAttachments[clear_desc->colorAttachment].attachment;
                            external_format_resolve =
                                GetExternalFormat(renderpass_create_info->pAttachments[resolve_attachment].pNext) != 0;
                        }
                    }
                }

                color_attachment_count = subpass_desc->colorAttachmentCount;

                if (framebuffer && subpass_desc->pDepthStencilAttachment &&
                    (subpass_desc->pDepthStencilAttachment->attachment != VK_ATTACHMENT_UNUSED)) {
                    depth_view_state =
                        cb_state.GetActiveAttachmentImageViewState(subpass_desc->pDepthStencilAttachment->attachment);
                    stencil_view_state = depth_view_state;

                    const VkFormat image_view_format = depth_view_state->safe_create_info.format;
                    if ((aspect_mask & VK_IMAGE_ASPECT_DEPTH_BIT) && !vkuFormatHasDepth(image_view_format)) {
                        const LogObjectList objlist(commandBuffer, cb_state.activeRenderPass->renderPass(),
                                                    depth_view_state->image_view());
                        skip |= LogError("VUID-vkCmdClearAttachments-aspectMask-07884", objlist, attachment_loc,
                                         "in pSubpasses[%" PRIu32
                                         "] has VK_IMAGE_ASPECT_DEPTH_BIT and is backed by an image view with format (%s).",
                                         cb_state.GetActiveSubpass(), string_VkFormat(image_view_format));
                    }

                    if ((aspect_mask & VK_IMAGE_ASPECT_STENCIL_BIT) && !vkuFormatHasStencil(image_view_format)) {
                        const LogObjectList objlist(commandBuffer, cb_state.activeRenderPass->renderPass(),
                                                    stencil_view_state->image_view());
                        skip |= LogError("VUID-vkCmdClearAttachments-aspectMask-07885", objlist, attachment_loc,
                                         "in pSubpasses[%" PRIu32
                                         "] has VK_IMAGE_ASPECT_STENCIL_BIT and is backed by an image view with format (%s).",
                                         cb_state.GetActiveSubpass(), string_VkFormat(image_view_format));
                    }
                }

                view_mask = subpass_desc->viewMask;
            }
        }

        if (aspect_mask & VK_IMAGE_ASPECT_METADATA_BIT) {
            const LogObjectList objlist(commandBuffer, cb_state.activeRenderPass->renderPass());
            skip |= LogError("VUID-VkClearAttachment-aspectMask-00020", objlist, attachment_loc.dot(Field::aspectMask), "is %s.",
                             string_VkImageAspectFlags(aspect_mask).c_str());
        } else if (aspect_mask & (VK_IMAGE_ASPECT_MEMORY_PLANE_0_BIT_EXT | VK_IMAGE_ASPECT_MEMORY_PLANE_1_BIT_EXT |
                                  VK_IMAGE_ASPECT_MEMORY_PLANE_2_BIT_EXT | VK_IMAGE_ASPECT_MEMORY_PLANE_3_BIT_EXT)) {
            const LogObjectList objlist(commandBuffer, cb_state.activeRenderPass->renderPass());
            skip |= LogError("VUID-VkClearAttachment-aspectMask-02246", objlist, attachment_loc.dot(Field::aspectMask), "is %s.",
                             string_VkImageAspectFlags(aspect_mask).c_str());
        } else if (aspect_mask & VK_IMAGE_ASPECT_COLOR_BIT) {
            if (!is_valid_color_attachment_index) {
                const LogObjectList objlist(commandBuffer, cb_state.activeRenderPass->renderPass());
                skip |= LogError("VUID-vkCmdClearAttachments-aspectMask-07271", objlist, attachment_loc.dot(Field::colorAttachment),
                                 "(%" PRIu32 " is larger than colorAttachmentCount (%" PRIu32 ").", clear_desc->colorAttachment,
                                 color_attachment_count);
            }

            if ((aspect_mask & VK_IMAGE_ASPECT_DEPTH_BIT) || (aspect_mask & VK_IMAGE_ASPECT_STENCIL_BIT)) {
                const LogObjectList objlist(commandBuffer, cb_state.activeRenderPass->renderPass());
                skip |= LogError("VUID-VkClearAttachment-aspectMask-00019", objlist, attachment_loc.dot(Field::aspectMask),
                                 "is %s.", string_VkImageAspectFlags(aspect_mask).c_str());
            }

        } else if (aspect_mask & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) {
            skip |= ValidateClearDepthStencilValue(commandBuffer, clear_desc->clearValue.depthStencil,
                                                   attachment_loc.dot(Field::clearValue).dot(Field::depthStencil));
        } else if (external_format_resolve && IsAnyPlaneAspect(aspect_mask)) {
            const LogObjectList objlist(commandBuffer, cb_state.activeRenderPass->renderPass());
            skip |= LogError("VUID-vkCmdClearAttachments-aspectMask-09298", objlist, attachment_loc.dot(Field::aspectMask),
                             "is %s.", string_VkImageAspectFlags(aspect_mask).c_str());
        }

        std::array<const IMAGE_VIEW_STATE *, 3> image_views = {nullptr, nullptr, nullptr};
        if (aspect_mask & VK_IMAGE_ASPECT_COLOR_BIT) {
            image_views[0] = color_view_state;
        }
        if (aspect_mask & VK_IMAGE_ASPECT_DEPTH_BIT) {
            image_views[1] = depth_view_state;
        }
        if (aspect_mask & VK_IMAGE_ASPECT_STENCIL_BIT) {
            image_views[2] = stencil_view_state;
        }
        if (image_views[1] == image_views[2]) {
            image_views[2] = nullptr;
        }

        for (auto image_view : image_views) {
            if (image_view) {
                skip |= ValidateProtectedImage(cb_state, *image_view->image_state, error_obj.location,
                                               "VUID-vkCmdClearAttachments-commandBuffer-02504");
                skip |= ValidateUnprotectedImage(cb_state, *image_view->image_state, error_obj.location,
                                                 "VUID-vkCmdClearAttachments-commandBuffer-02505");
            }
        }

        // With a non-zero view mask, multiview functionality is considered to be enabled
        if (view_mask > 0) {
            for (uint32_t i = 0; i < rectCount; ++i) {
                if (pRects[i].baseArrayLayer != 0 || pRects[i].layerCount != 1) {
                    const LogObjectList objlist(commandBuffer, cb_state.activeRenderPass->renderPass());
                    skip |= LogError("VUID-vkCmdClearAttachments-baseArrayLayer-00018", objlist,
                                     error_obj.location.dot(Field::pRects, i).dot(Field::baseArrayLayer),
                                     "is %" PRIu32 " and layerCount is %" PRIu32 ", but the render pass instance uses multiview.",
                                     pRects[i].baseArrayLayer, pRects[i].layerCount);
                }
            }
        }
    }
    return skip;
}

void CoreChecks::PreCallRecordCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                                  const VkClearAttachment *pAttachments, uint32_t rectCount,
                                                  const VkClearRect *pRects) {
    auto cb_state_ptr = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    if (!cb_state_ptr) {
        return;
    }
    const CMD_BUFFER_STATE &cb_state = *cb_state_ptr;
    if (!cb_state.activeRenderPass || (cb_state.createInfo.level != VK_COMMAND_BUFFER_LEVEL_SECONDARY)) {
        return;
    }
    std::shared_ptr<std::vector<VkClearRect>> clear_rect_copy;
    if (cb_state.activeRenderPass->use_dynamic_rendering_inherited) {
        for (uint32_t attachment_index = 0; attachment_index < attachmentCount; attachment_index++) {
            const auto clear_desc = &pAttachments[attachment_index];
            auto colorAttachmentCount = cb_state.activeRenderPass->inheritance_rendering_info.colorAttachmentCount;
            int image_index = -1;
            if ((clear_desc->aspectMask & VK_IMAGE_ASPECT_COLOR_BIT) && (clear_desc->colorAttachment < colorAttachmentCount)) {
                image_index = cb_state.GetDynamicColorAttachmentImageIndex(clear_desc->colorAttachment);
            } else if (clear_desc->aspectMask & (VK_IMAGE_ASPECT_DEPTH_BIT)) {
                image_index = cb_state.GetDynamicDepthAttachmentImageIndex();
            } else if (clear_desc->aspectMask & (VK_IMAGE_ASPECT_STENCIL_BIT)) {
                image_index = cb_state.GetDynamicStencilAttachmentImageIndex();
            }

            if (image_index != -1) {
                if (!clear_rect_copy) {
                    // We need a copy of the clear rectangles that will persist until the last lambda executes
                    // but we want to create it as lazily as possible
                    clear_rect_copy.reset(new std::vector<VkClearRect>(pRects, pRects + rectCount));
                }
                // if a secondary level command buffer inherits the framebuffer from the primary command buffer
                // (see VkCommandBufferInheritanceInfo), this validation must be deferred until queue submit time
                auto val_fn = [this, rectCount, clear_rect_copy](const CMD_BUFFER_STATE &secondary, const CMD_BUFFER_STATE *prim_cb,
                                                                 const FRAMEBUFFER_STATE *) {
                    assert(rectCount == clear_rect_copy->size());
                    bool skip = false;
                    const Location loc(Func::vkCmdClearAttachments);
                    skip = ValidateClearAttachmentExtent(
                        secondary, prim_cb->activeRenderPass->dynamic_rendering_begin_rendering_info.renderArea,
                        prim_cb->activeRenderPass->dynamic_rendering_begin_rendering_info.layerCount, rectCount,
                        clear_rect_copy->data(), loc);
                    return skip;
                };
                cb_state_ptr->cmd_execute_commands_functions.emplace_back(val_fn);
            }
        }
    } else if (cb_state.activeRenderPass->use_dynamic_rendering == false) {
        const VkRenderPassCreateInfo2 *renderpass_create_info = cb_state.activeRenderPass->createInfo.ptr();
        const VkSubpassDescription2 *subpass_desc = &renderpass_create_info->pSubpasses[cb_state.GetActiveSubpass()];

        for (uint32_t attachment_index = 0; attachment_index < attachmentCount; attachment_index++) {
            const auto clear_desc = &pAttachments[attachment_index];
            uint32_t fb_attachment = VK_ATTACHMENT_UNUSED;
            if ((clear_desc->aspectMask & VK_IMAGE_ASPECT_COLOR_BIT) &&
                (clear_desc->colorAttachment < subpass_desc->colorAttachmentCount)) {
                fb_attachment = subpass_desc->pColorAttachments[clear_desc->colorAttachment].attachment;
            } else if ((clear_desc->aspectMask & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) &&
                       subpass_desc->pDepthStencilAttachment) {
                fb_attachment = subpass_desc->pDepthStencilAttachment->attachment;
            }
            if (fb_attachment != VK_ATTACHMENT_UNUSED) {
                if (!clear_rect_copy) {
                    // We need a copy of the clear rectangles that will persist until the last lambda executes
                    // but we want to create it as lazily as possible
                    clear_rect_copy.reset(new std::vector<VkClearRect>(pRects, pRects + rectCount));
                }
                // if a secondary level command buffer inherits the framebuffer from the primary command buffer
                // (see VkCommandBufferInheritanceInfo), this validation must be deferred until queue submit time
                auto val_fn = [this, rectCount, clear_rect_copy](const CMD_BUFFER_STATE &secondary, const CMD_BUFFER_STATE *prim_cb,
                                                                 const FRAMEBUFFER_STATE *fb) {
                    assert(rectCount == clear_rect_copy->size());
                    const auto &render_area = prim_cb->active_render_pass_begin_info.renderArea;
                    bool skip = false;

                    if (fb && prim_cb->IsPrimary()) {
                        const Location loc(Func::vkCmdClearAttachments);
                        skip = ValidateClearAttachmentExtent(secondary, render_area, fb->createInfo.layers, rectCount,
                                                             clear_rect_copy->data(), loc);
                    }
                    return skip;
                };
                cb_state_ptr->cmd_execute_commands_functions.emplace_back(val_fn);
            }
        }
    }
}

// Helper function to validate usage flags for images
bool CoreChecks::ValidateImageUsageFlags(VkCommandBuffer cb, IMAGE_STATE const &image_state, VkImageUsageFlags desired, bool strict,
                                         const char *vuid, const Location &image_loc) const {
    bool skip = false;
    LogObjectList objlist(cb, image_state.Handle());
    bool correct_usage = false;
    if (strict) {
        correct_usage = ((image_state.createInfo.usage & desired) == desired);
    } else {
        correct_usage = ((image_state.createInfo.usage & desired) != 0);
    }

    if (!correct_usage) {
        skip = LogError(vuid, objlist, image_loc, "(%s) was created with %s but requires %s.",
                        FormatHandle(image_state.Handle()).c_str(), string_VkImageUsageFlags(image_state.createInfo.usage).c_str(),
                        string_VkImageUsageFlags(desired).c_str());
    }
    return skip;
}

bool CoreChecks::ValidateImageFormatFeatureFlags(VkCommandBuffer cb, IMAGE_STATE const &image_state,
                                                 VkFormatFeatureFlags2KHR desired, const Location &image_loc,
                                                 const char *vuid) const {
    bool skip = false;
    const VkFormatFeatureFlags2KHR image_format_features = image_state.format_features;
    if ((image_format_features & desired) != desired) {
        const LogObjectList objlist(cb, image_state.Handle());
        // Same error, but more details if it was an AHB external format
        if (image_state.HasAHBFormat()) {
            skip |= LogError(
                vuid, objlist, image_loc,
                "(%s) was created with an external format having VkFormatFeatureFlags2 (%s) which is missing the required feature "
                "%s (Features found in VkAndroidHardwareBufferFormatPropertiesANDROID::formatFeatures).",
                FormatHandle(image_state).c_str(), string_VkFormatFeatureFlags2(image_format_features).c_str(),
                string_VkFormatFeatureFlags2(desired).c_str());
        } else {
            skip |= LogError(vuid, objlist, image_loc,
                             "(%s) was created with format %s and tiling %s which have VkFormatFeatureFlags2 (%s) which in turn is "
                             "missing the required feature %s.",
                             FormatHandle(image_state).c_str(), string_VkFormat(image_state.createInfo.format),
                             string_VkImageTiling(image_state.createInfo.tiling),
                             string_VkFormatFeatureFlags2(image_format_features).c_str(),
                             string_VkFormatFeatureFlags2(desired).c_str());
        }
    }
    return skip;
}

// For the given format verify that the aspect masks make sense
bool CoreChecks::ValidateImageAspectMask(VkImage image, VkFormat format, VkImageAspectFlags aspect_mask, bool is_image_disjoint,
                                         const Location &loc, const char *vuid) const {
    bool skip = false;
    // checks color format and (single-plane or non-disjoint)
    // if ycbcr extension is not supported then single-plane and non-disjoint are always both true
    if ((vkuFormatIsColor(format)) && ((vkuFormatIsMultiplane(format) == false) || (is_image_disjoint == false))) {
        if ((aspect_mask & VK_IMAGE_ASPECT_COLOR_BIT) != VK_IMAGE_ASPECT_COLOR_BIT) {
            skip |= LogError(
                vuid, image, loc,
                "Using format (%s) with aspect flags (%s) but color image formats must have the VK_IMAGE_ASPECT_COLOR_BIT set.",
                string_VkFormat(format), string_VkImageAspectFlags(aspect_mask).c_str());
        } else if ((aspect_mask & VK_IMAGE_ASPECT_COLOR_BIT) != aspect_mask) {
            skip |= LogError(vuid, image, loc,
                             "Using format (%s) with aspect flags (%s) but color image formats must have ONLY the "
                             "VK_IMAGE_ASPECT_COLOR_BIT set.",
                             string_VkFormat(format), string_VkImageAspectFlags(aspect_mask).c_str());
        }
    } else if (vkuFormatIsDepthAndStencil(format)) {
        if ((aspect_mask & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) == 0) {
            skip |= LogError(vuid, image, loc,
                             "Using format (%s) with aspect flags (%s) but depth/stencil image formats must have at least one "
                             "of VK_IMAGE_ASPECT_DEPTH_BIT and VK_IMAGE_ASPECT_STENCIL_BIT set.",
                             string_VkFormat(format), string_VkImageAspectFlags(aspect_mask).c_str());
        } else if ((aspect_mask & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) != aspect_mask) {
            skip |= LogError(vuid, image, loc,
                             "Using format (%s) with aspect flags (%s) but combination depth/stencil image formats can have "
                             "only the VK_IMAGE_ASPECT_DEPTH_BIT and VK_IMAGE_ASPECT_STENCIL_BIT set.",
                             string_VkFormat(format), string_VkImageAspectFlags(aspect_mask).c_str());
        }
    } else if (vkuFormatIsDepthOnly(format)) {
        if ((aspect_mask & VK_IMAGE_ASPECT_DEPTH_BIT) != VK_IMAGE_ASPECT_DEPTH_BIT) {
            skip |= LogError(vuid, image, loc,
                             "Using format (%s) with aspect flags (%s) but depth-only image formats must have the "
                             "VK_IMAGE_ASPECT_DEPTH_BIT set.",
                             string_VkFormat(format), string_VkImageAspectFlags(aspect_mask).c_str());
        } else if ((aspect_mask & VK_IMAGE_ASPECT_DEPTH_BIT) != aspect_mask) {
            skip |= LogError(vuid, image, loc,
                             "Using format (%s) with aspect flags (%s) but depth-only image formats can have only the "
                             "VK_IMAGE_ASPECT_DEPTH_BIT set.",
                             string_VkFormat(format), string_VkImageAspectFlags(aspect_mask).c_str());
        }
    } else if (vkuFormatIsStencilOnly(format)) {
        if ((aspect_mask & VK_IMAGE_ASPECT_STENCIL_BIT) != VK_IMAGE_ASPECT_STENCIL_BIT) {
            skip |= LogError(vuid, image, loc,
                             "Using format (%s) with aspect flags (%s) but stencil-only image formats must have the "
                             "VK_IMAGE_ASPECT_STENCIL_BIT set.",
                             string_VkFormat(format), string_VkImageAspectFlags(aspect_mask).c_str());
        } else if ((aspect_mask & VK_IMAGE_ASPECT_STENCIL_BIT) != aspect_mask) {
            skip |= LogError(vuid, image, loc,
                             "Using format (%s) with aspect flags (%s) but stencil-only image formats can have only the "
                             "VK_IMAGE_ASPECT_STENCIL_BIT set.",
                             string_VkFormat(format), string_VkImageAspectFlags(aspect_mask).c_str());
        }
    } else if (vkuFormatIsMultiplane(format)) {
        VkImageAspectFlags valid_flags = VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT;
        if (3 == vkuFormatPlaneCount(format)) {
            valid_flags = valid_flags | VK_IMAGE_ASPECT_PLANE_2_BIT;
        }
        if ((aspect_mask & valid_flags) != aspect_mask) {
            skip |= LogError(vuid, image, loc,
                             "Using format (%s) with aspect flags (%s) but multi-plane image formats may have only "
                             "VK_IMAGE_ASPECT_COLOR_BIT or VK_IMAGE_ASPECT_PLANE_n_BITs set, where n = [0, 1, 2].",
                             string_VkFormat(format), string_VkImageAspectFlags(aspect_mask).c_str());
        }
    }
    return skip;
}

bool CoreChecks::ValidateImageSubresourceRange(const uint32_t image_mip_count, const uint32_t image_layer_count,
                                               const VkImageSubresourceRange &subresourceRange,
                                               const char *image_layer_count_var_name, const VkImage image,
                                               const SubresourceRangeErrorCodes &errorCodes,
                                               const Location &subresource_loc) const {
    bool skip = false;

    // Validate mip levels
    if (subresourceRange.baseMipLevel >= image_mip_count) {
        skip |= LogError(errorCodes.base_mip_err, image, subresource_loc.dot(Field::baseMipLevel),
                         "(%" PRIu32 ") is greater or equal to the mip level count of the image (%" PRIu32 ").",
                         subresourceRange.baseMipLevel, image_mip_count);
    }

    if (subresourceRange.levelCount != VK_REMAINING_MIP_LEVELS) {
        if (subresourceRange.levelCount == 0) {
            skip |= LogError("VUID-VkImageSubresourceRange-levelCount-01720", image, subresource_loc.dot(Field::levelCount),
                             "is zero.");
        } else {
            const uint64_t necessary_mip_count = uint64_t{subresourceRange.baseMipLevel} + uint64_t{subresourceRange.levelCount};

            if (necessary_mip_count > image_mip_count) {
                skip |= LogError(errorCodes.mip_count_err, image, subresource_loc.dot(Field::baseMipLevel),
                                 "(%" PRIu32 ") + levelCount (%" PRIu32 ") is (%" PRIu64
                                 ") which is greater than the mip level count of the image (i.e. greater than %" PRIu32 ").",
                                 subresourceRange.baseMipLevel, subresourceRange.levelCount, necessary_mip_count, image_mip_count);
            }
        }
    }

    // Validate array layers
    if (subresourceRange.baseArrayLayer >= image_layer_count) {
        skip |= LogError(errorCodes.base_layer_err, image, subresource_loc.dot(Field::baseArrayLayer),
                         "(%" PRIu32 ") is greater or equal to the %s of the image when it was created (%" PRIu32 ").",
                         subresourceRange.baseArrayLayer, image_layer_count_var_name, image_layer_count);
    }

    if (subresourceRange.layerCount != VK_REMAINING_ARRAY_LAYERS) {
        if (subresourceRange.layerCount == 0) {
            skip |= LogError("VUID-VkImageSubresourceRange-layerCount-01721", image, subresource_loc.dot(Field::layerCount),
                             "is zero.");
        } else {
            const uint64_t necessary_layer_count =
                uint64_t{subresourceRange.baseArrayLayer} + uint64_t{subresourceRange.layerCount};

            if (necessary_layer_count > image_layer_count) {
                skip |= LogError(errorCodes.layer_count_err, image, subresource_loc.dot(Field::baseArrayLayer),
                                 "(%" PRIu32 ") + layerCount (%" PRIu32 ") is (%" PRIu64
                                 ") which is greater than the %s of the image when it was created (%" PRIu32 ").",
                                 subresourceRange.baseArrayLayer, subresourceRange.layerCount, necessary_layer_count,
                                 image_layer_count_var_name, image_layer_count);
            }
        }
    }

    if (subresourceRange.aspectMask & VK_IMAGE_ASPECT_COLOR_BIT) {
        if (subresourceRange.aspectMask &
            (VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT | VK_IMAGE_ASPECT_PLANE_2_BIT)) {
            skip |= LogError("VUID-VkImageSubresourceRange-aspectMask-01670", image, subresource_loc.dot(Field::aspectMask),
                             "is %s.", string_VkImageAspectFlags(subresourceRange.aspectMask).c_str());
        }
    }

    // aspectMask must not contain VK_IMAGE_ASPECT_MEMORY_PLANE_i_BIT_EXT
    if (subresourceRange.aspectMask & (VK_IMAGE_ASPECT_MEMORY_PLANE_0_BIT_EXT | VK_IMAGE_ASPECT_MEMORY_PLANE_1_BIT_EXT |
                                       VK_IMAGE_ASPECT_MEMORY_PLANE_2_BIT_EXT | VK_IMAGE_ASPECT_MEMORY_PLANE_3_BIT_EXT)) {
        skip |= LogError("VUID-VkImageSubresourceRange-aspectMask-02278", image, subresource_loc.dot(Field::aspectMask), "is %s.",
                         string_VkImageAspectFlags(subresourceRange.aspectMask).c_str());
    }

    return skip;
}

bool CoreChecks::ValidateCreateImageViewSubresourceRange(const IMAGE_STATE &image_state, bool is_imageview_2d_type,
                                                         const VkImageSubresourceRange &subresourceRange,
                                                         const Location &loc) const {
    const bool is_khr_maintenance1 = IsExtEnabled(device_extensions.vk_khr_maintenance1);
    const bool is_2d_compatible =
        image_state.createInfo.flags & (VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT | VK_IMAGE_CREATE_2D_VIEW_COMPATIBLE_BIT_EXT);
    const bool is_image_slicable = (image_state.createInfo.imageType == VK_IMAGE_TYPE_3D) && is_2d_compatible;
    const bool is_3_d_to_2_d_map = is_khr_maintenance1 && is_image_slicable && is_imageview_2d_type;

    uint32_t image_layer_count;

    if (is_3_d_to_2_d_map) {
        const auto layers = LayersFromRange(subresourceRange);
        const auto extent = image_state.GetEffectiveSubresourceExtent(layers);
        image_layer_count = extent.depth;
    } else {
        image_layer_count = image_state.createInfo.arrayLayers;
    }

    const auto image_layer_count_var_name = is_3_d_to_2_d_map ? "extent.depth" : "arrayLayers";

    SubresourceRangeErrorCodes subresource_range_error_codes = {};
    subresource_range_error_codes.base_mip_err = "VUID-VkImageViewCreateInfo-subresourceRange-01478";
    subresource_range_error_codes.mip_count_err = "VUID-VkImageViewCreateInfo-subresourceRange-01718";
    subresource_range_error_codes.base_layer_err =
        is_3_d_to_2_d_map ? "VUID-VkImageViewCreateInfo-image-02724" : "VUID-VkImageViewCreateInfo-image-06724";
    subresource_range_error_codes.layer_count_err = is_3_d_to_2_d_map ? "VUID-VkImageViewCreateInfo-subresourceRange-02725"
                                                                      : "VUID-VkImageViewCreateInfo-subresourceRange-06725";

    return ValidateImageSubresourceRange(image_state.createInfo.mipLevels, image_layer_count, subresourceRange,
                                         image_layer_count_var_name, image_state.image(), subresource_range_error_codes,
                                         loc.dot(Field::subresourceRange));
}

bool CoreChecks::ValidateCmdClearColorSubresourceRange(const IMAGE_STATE &image_state,
                                                       const VkImageSubresourceRange &subresourceRange, const Location &loc) const {
    SubresourceRangeErrorCodes subresource_range_error_codes = {};
    subresource_range_error_codes.base_mip_err = "VUID-vkCmdClearColorImage-baseMipLevel-01470";
    subresource_range_error_codes.mip_count_err = "VUID-vkCmdClearColorImage-pRanges-01692";
    subresource_range_error_codes.base_layer_err = "VUID-vkCmdClearColorImage-baseArrayLayer-01472";
    subresource_range_error_codes.layer_count_err = "VUID-vkCmdClearColorImage-pRanges-01693";

    return ValidateImageSubresourceRange(image_state.createInfo.mipLevels, image_state.createInfo.arrayLayers, subresourceRange,
                                         "arrayLayers", image_state.image(), subresource_range_error_codes,
                                         loc.dot(Field::subresourceRange));
}

bool CoreChecks::ValidateCmdClearDepthSubresourceRange(const IMAGE_STATE &image_state,
                                                       const VkImageSubresourceRange &subresourceRange, const Location &loc) const {
    SubresourceRangeErrorCodes subresource_range_error_codes = {};
    subresource_range_error_codes.base_mip_err = "VUID-vkCmdClearDepthStencilImage-baseMipLevel-01474";
    subresource_range_error_codes.mip_count_err = "VUID-vkCmdClearDepthStencilImage-pRanges-01694";
    subresource_range_error_codes.base_layer_err = "VUID-vkCmdClearDepthStencilImage-baseArrayLayer-01476";
    subresource_range_error_codes.layer_count_err = "VUID-vkCmdClearDepthStencilImage-pRanges-01695";

    return ValidateImageSubresourceRange(image_state.createInfo.mipLevels, image_state.createInfo.arrayLayers, subresourceRange,
                                         "arrayLayers", image_state.image(), subresource_range_error_codes,
                                         loc.dot(Field::subresourceRange));
}

bool CoreChecks::ValidateImageBarrierSubresourceRange(const Location &loc, const IMAGE_STATE &image_state,
                                                      const VkImageSubresourceRange &subresourceRange) const {
    return ValidateImageSubresourceRange(image_state.createInfo.mipLevels, image_state.createInfo.arrayLayers, subresourceRange,
                                         "arrayLayers", image_state.image(), sync_vuid_maps::GetSubResourceVUIDs(loc),
                                         loc.dot(Field::subresourceRange));
}

bool CoreChecks::ValidateImageViewFormatFeatures(const IMAGE_STATE &image_state, const VkFormat view_format,
                                                 const VkImageUsageFlags image_usage, const Location &create_info_loc) const {
    // Pass in image_usage here instead of extracting it from image_state in case there's a chained VkImageViewUsageCreateInfo
    bool skip = false;

    VkFormatFeatureFlags2KHR tiling_features = 0;
    const VkImageTiling image_tiling = image_state.createInfo.tiling;

    if (image_state.HasAHBFormat()) {
        // AHB image view and image share same feature sets
        tiling_features = image_state.format_features;
    } else if (image_tiling == VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT) {
        // Parameter validation should catch if this is used without VK_EXT_image_drm_format_modifier
        assert(IsExtEnabled(device_extensions.vk_ext_image_drm_format_modifier));
        VkImageDrmFormatModifierPropertiesEXT drm_format_properties = vku::InitStructHelper();
        DispatchGetImageDrmFormatModifierPropertiesEXT(device, image_state.image(), &drm_format_properties);

        VkDrmFormatModifierPropertiesListEXT fmt_drm_props = vku::InitStructHelper();
        VkFormatProperties2 fmt_props_2 = vku::InitStructHelper(&fmt_drm_props);
        DispatchGetPhysicalDeviceFormatProperties2(physical_device, view_format, &fmt_props_2);

        std::vector<VkDrmFormatModifierPropertiesEXT> drm_properties;
        drm_properties.resize(fmt_drm_props.drmFormatModifierCount);
        fmt_drm_props.pDrmFormatModifierProperties = drm_properties.data();

        DispatchGetPhysicalDeviceFormatProperties2(physical_device, view_format, &fmt_props_2);

        for (uint32_t i = 0; i < fmt_drm_props.drmFormatModifierCount; i++) {
            if (fmt_drm_props.pDrmFormatModifierProperties[i].drmFormatModifier == drm_format_properties.drmFormatModifier) {
                tiling_features = fmt_drm_props.pDrmFormatModifierProperties[i].drmFormatModifierTilingFeatures;
                break;
            }
        }
    } else {
        VkFormatProperties3KHR format_properties = GetPDFormatProperties(view_format);
        tiling_features = (image_tiling == VK_IMAGE_TILING_LINEAR) ? format_properties.linearTilingFeatures
                                                                   : format_properties.optimalTilingFeatures;
    }

    if (tiling_features == 0) {
        skip |= LogError("VUID-VkImageViewCreateInfo-None-02273", image_state.image(), create_info_loc.dot(Field::format),
                         "%s with tiling %s has no supported format features on this "
                         "physical device.",
                         string_VkFormat(view_format), string_VkImageTiling(image_tiling));
    } else if ((image_usage & VK_IMAGE_USAGE_SAMPLED_BIT) && !(tiling_features & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)) {
        skip |= LogError("VUID-VkImageViewCreateInfo-usage-02274", image_state.image(), create_info_loc.dot(Field::format),
                         "%s with tiling %s only supports %s.", string_VkFormat(view_format), string_VkImageTiling(image_tiling),
                         string_VkFormatFeatureFlags2(tiling_features).c_str());
    } else if ((image_usage & VK_IMAGE_USAGE_STORAGE_BIT) && !(tiling_features & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT)) {
        skip |= LogError("VUID-VkImageViewCreateInfo-usage-02275", image_state.image(), create_info_loc.dot(Field::format),
                         "%s with tiling %s only supports %s.", string_VkFormat(view_format), string_VkImageTiling(image_tiling),
                         string_VkFormatFeatureFlags2(tiling_features).c_str());
    } else if ((image_usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) &&
               !(tiling_features & (VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_2_LINEAR_COLOR_ATTACHMENT_BIT_NV))) {
        skip |= LogError("VUID-VkImageViewCreateInfo-usage-08931", image_state.image(), create_info_loc.dot(Field::format),
                         "%s with tiling %s only supports %s.", string_VkFormat(view_format), string_VkImageTiling(image_tiling),
                         string_VkFormatFeatureFlags2(tiling_features).c_str());
    } else if ((image_usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) &&
               !(tiling_features & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
        skip |= LogError("VUID-VkImageViewCreateInfo-usage-02277", image_state.image(), create_info_loc.dot(Field::format),
                         "%s with tiling %s only supports %s.", string_VkFormat(view_format), string_VkImageTiling(image_tiling),
                         string_VkFormatFeatureFlags2(tiling_features).c_str());
    } else if ((image_usage & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT) &&
               !(tiling_features & (VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT |
                                    VK_FORMAT_FEATURE_2_LINEAR_COLOR_ATTACHMENT_BIT_NV))) {
        if (!enabled_features.externalFormatResolve && !android_external_format_resolve_null_color_attachment_prop &&
            !image_state.HasAHBFormat()) {
            skip |= LogError("VUID-VkImageViewCreateInfo-usage-08932", image_state.image(), create_info_loc.dot(Field::format),
                             "%s with tiling %s only supports %s.", string_VkFormat(view_format),
                             string_VkImageTiling(image_tiling), string_VkFormatFeatureFlags2(tiling_features).c_str());
        }
    } else if ((image_usage & VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR) &&
               !(tiling_features & VK_FORMAT_FEATURE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR)) {
        if (enabled_features.attachmentFragmentShadingRate) {
            skip |= LogError("VUID-VkImageViewCreateInfo-usage-04550", image_state.image(), create_info_loc.dot(Field::format),
                             "%s with tiling %s only supports %s.", string_VkFormat(view_format),
                             string_VkImageTiling(image_tiling), string_VkFormatFeatureFlags2(tiling_features).c_str());
        }
    }

    return skip;
}
// Returns whether two formats have identical components (compares the size and type of each component)
// EX. (R8G8B8A8, B8G8R8A8) -> true
bool FormatsEqualComponentBits(VkFormat format_a, VkFormat format_b) {
    const VKU_FORMAT_INFO format_info_a = vkuGetFormatInfo(format_a);
    const VKU_FORMAT_INFO format_info_b = vkuGetFormatInfo(format_b);
    if (format_info_a.compatibility == VKU_FORMAT_COMPATIBILITY_CLASS_NONE || format_info_b.compatibility == VKU_FORMAT_COMPATIBILITY_CLASS_NONE) {
        return false;
    } else if (format_info_a.component_count != format_info_b.component_count) {
        return false;
    }
    // Need to loop match each component type is found in both formats
    // formats are maxed at 4 components, so the double loop is not going to scale
    for (uint32_t i = 0; i < format_info_a.component_count; i++) {
        const VKU_FORMAT_COMPONENT_INFO component_a = format_info_a.components[i];
        bool component_match = false;
        for (uint32_t j = 0; j < format_info_b.component_count; j++) {
            const VKU_FORMAT_COMPONENT_INFO component_b = format_info_b.components[j];
            if ((component_a.type == component_b.type) && (component_a.size == component_b.size)) {
                component_match = true;
                break;
            }
        }
        if (!component_match) {
            return false;
        }
    }
    return true;
}

bool CoreChecks::PreCallValidateCreateImageView(VkDevice device, const VkImageViewCreateInfo *pCreateInfo,
                                                [[maybe_unused]] const VkAllocationCallbacks *pAllocator,
                                                [[maybe_unused]] VkImageView *pView, const ErrorObject &error_obj) const {
    bool skip = false;
    auto image_state_ptr = Get<IMAGE_STATE>(pCreateInfo->image);
    if (!image_state_ptr) {
        return skip;
    }
    const Location create_info_loc = error_obj.location.dot(Field::pCreateInfo);
    const auto &image_state = *image_state_ptr;

    const VkImageUsageFlags valid_usage_flags = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT |
                                                VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
                                                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |
                                                VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR |
                                                VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT |
                                                VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR | VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR |
                                                VK_IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT_KHR | VK_IMAGE_USAGE_VIDEO_ENCODE_DPB_BIT_KHR |
                                                VK_IMAGE_USAGE_SAMPLE_WEIGHT_BIT_QCOM | VK_IMAGE_USAGE_SAMPLE_BLOCK_MATCH_BIT_QCOM;
    skip |= ValidateImageUsageFlags(VK_NULL_HANDLE, image_state, valid_usage_flags, false, "VUID-VkImageViewCreateInfo-image-04441",
                                    create_info_loc.dot(Field::image));
    // If this isn't a sparse image, it needs to have memory backing it at CreateImageView time
    skip |= ValidateMemoryIsBoundToImage(LogObjectList(device, pCreateInfo->image), image_state, create_info_loc.dot(Field::image),
                                         "VUID-VkImageViewCreateInfo-image-01020");
    // Checks imported from image layer
    skip |= ValidateCreateImageViewSubresourceRange(
        image_state, pCreateInfo->viewType == VK_IMAGE_VIEW_TYPE_2D || pCreateInfo->viewType == VK_IMAGE_VIEW_TYPE_2D_ARRAY,
        pCreateInfo->subresourceRange, create_info_loc);

    const auto normalized_subresource_range = image_state.NormalizeSubresourceRange(pCreateInfo->subresourceRange);
    const VkImageCreateFlags image_flags = image_state.createInfo.flags;
    const VkFormat image_format = image_state.createInfo.format;
    const VkFormat view_format = pCreateInfo->format;
    const VkImageAspectFlags aspect_mask = pCreateInfo->subresourceRange.aspectMask;
    const VkImageType image_type = image_state.createInfo.imageType;
    const VkImageViewType view_type = pCreateInfo->viewType;
    const uint32_t layer_count = pCreateInfo->subresourceRange.layerCount;

    // If there's a chained VkImageViewUsageCreateInfo struct, modify image_usage to match
    VkImageUsageFlags image_usage = image_state.createInfo.usage;
    if (const auto chained_ivuci_struct = vku::FindStructInPNextChain<VkImageViewUsageCreateInfo>(pCreateInfo->pNext); chained_ivuci_struct) {
        if (IsExtEnabled(device_extensions.vk_khr_maintenance2)) {
            const auto image_stencil_struct = vku::FindStructInPNextChain<VkImageStencilUsageCreateInfo>(image_state.createInfo.pNext);
            if (image_stencil_struct == nullptr) {
                if ((image_usage | chained_ivuci_struct->usage) != image_usage) {
                    skip |=
                        LogError("VUID-VkImageViewCreateInfo-pNext-02662", pCreateInfo->image,
                                 create_info_loc.pNext(Struct::VkImageViewUsageCreateInfo, Field::usage),
                                 "(%s) must not include any bits that were not set in VkImageCreateInfo::usage (%s) of the image.",
                                 string_VkImageUsageFlags(chained_ivuci_struct->usage).c_str(),
                                 string_VkImageUsageFlags(image_usage).c_str());
                }
            } else {
                if ((aspect_mask & VK_IMAGE_ASPECT_STENCIL_BIT) == VK_IMAGE_ASPECT_STENCIL_BIT &&
                    (image_stencil_struct->stencilUsage | chained_ivuci_struct->usage) != image_stencil_struct->stencilUsage) {
                    skip |=
                        LogError("VUID-VkImageViewCreateInfo-pNext-02663", pCreateInfo->image,
                                 create_info_loc.pNext(Struct::VkImageViewUsageCreateInfo, Field::usage),
                                 "(%s) must not include any bits that were not set in VkImageStencilUsageCreateInfo::stencilUsage "
                                 "(%s) if subResourceRange.aspectMask includes VK_IMAGE_ASPECT_STENCIL_BIT.",
                                 string_VkImageUsageFlags(chained_ivuci_struct->usage).c_str(),
                                 string_VkImageUsageFlags(image_stencil_struct->stencilUsage).c_str());
                }
                if ((aspect_mask & ~VK_IMAGE_ASPECT_STENCIL_BIT) != 0 &&
                    (image_usage | chained_ivuci_struct->usage) != image_usage) {
                    skip |=
                        LogError("VUID-VkImageViewCreateInfo-pNext-02664", pCreateInfo->image,
                                 create_info_loc.pNext(Struct::VkImageViewUsageCreateInfo, Field::usage),
                                 "(%s) must not include any bits that were not set in VkImageCreateInfo::usage (%s) of the image "
                                 "if subResourceRange.aspectMask (%s) includes bits other than VK_IMAGE_ASPECT_STENCIL_BIT.",
                                 string_VkImageUsageFlags(chained_ivuci_struct->usage).c_str(),
                                 string_VkImageUsageFlags(image_usage).c_str(), string_VkImageAspectFlags(aspect_mask).c_str());
                }
            }
        }

        image_usage = chained_ivuci_struct->usage;
    }

    if (const auto sliced_create_info_ext = vku::FindStructInPNextChain<VkImageViewSlicedCreateInfoEXT>(pCreateInfo->pNext);
        sliced_create_info_ext) {
        const bool feature_disabled = (enabled_features.imageSlicedViewOf3D == VK_FALSE);
        if (feature_disabled) {
            skip |= LogError("VUID-VkImageViewSlicedCreateInfoEXT-None-07871", pCreateInfo->image, create_info_loc,
                             "imageSlicedViewOf3D is not enabled.");
        }

        if (image_type != VK_IMAGE_TYPE_3D) {
            skip |= LogError("VUID-VkImageViewSlicedCreateInfoEXT-image-07869", pCreateInfo->image,
                             create_info_loc.dot(Field::image), "was created with imageType (%s).", string_VkImageType(image_type));
        }

        if (view_type != VK_IMAGE_VIEW_TYPE_3D) {
            skip |= LogError("VUID-VkImageViewSlicedCreateInfoEXT-viewType-07909", pCreateInfo->image,
                             create_info_loc.dot(Field::viewType), "is (%s).", string_VkImageViewType(view_type));
        }

        const uint32_t effective_mip_levels = ResolveRemainingLevels(image_state.createInfo, pCreateInfo->subresourceRange);
        if (effective_mip_levels != 1) {
            skip |= LogError("VUID-VkImageViewSlicedCreateInfoEXT-None-07870", pCreateInfo->image, create_info_loc,
                             "Image view references %" PRIu32 " mip levels.", effective_mip_levels);
        }

        const uint32_t effective_view_depth = image_state.GetEffectiveSubresourceExtent(pCreateInfo->subresourceRange).depth;

        const uint32_t slice_offset = sliced_create_info_ext->sliceOffset;
        const uint32_t slice_count = sliced_create_info_ext->sliceCount;

        if (slice_offset >= effective_view_depth) {
            skip |= LogError("VUID-VkImageViewSlicedCreateInfoEXT-sliceOffset-07867", pCreateInfo->image,
                             create_info_loc.pNext(Struct::VkImageViewSlicedCreateInfoEXT, Field::sliceOffset),
                             "(%" PRIu32 ") must be less than the effective view depth (%" PRIu32 ").", slice_offset,
                             effective_view_depth);
        }

        if (slice_count != VK_REMAINING_3D_SLICES_EXT) {
            if (slice_count == 0) {
                skip |= LogError("VUID-VkImageViewSlicedCreateInfoEXT-sliceCount-07868", pCreateInfo->image,
                                 create_info_loc.pNext(Struct::VkImageViewSlicedCreateInfoEXT, Field::sliceCount), "is 0.");
            }

            if ((slice_offset + slice_count) > effective_view_depth) {
                skip |= LogError("VUID-VkImageViewSlicedCreateInfoEXT-sliceCount-07868", pCreateInfo->image,
                                 create_info_loc.pNext(Struct::VkImageViewSlicedCreateInfoEXT, Field::sliceOffset),
                                 "(%" PRIu32 ") + sliceCount (%" PRIu32 ") greater than effective view depth (%" PRIu32 ").",
                                 slice_offset, slice_count, effective_view_depth);
            }
        }
    }

    // If image used VkImageFormatListCreateInfo need to make sure a format from list is used
    if (const auto format_list_info = vku::FindStructInPNextChain<VkImageFormatListCreateInfo>(image_state.createInfo.pNext);
        format_list_info && (format_list_info->viewFormatCount > 0)) {
        bool found_format = false;
        for (uint32_t i = 0; i < format_list_info->viewFormatCount; i++) {
            if (format_list_info->pViewFormats[i] == view_format) {
                found_format = true;
                break;
            }
        }
        if (found_format == false) {
            skip |= LogError("VUID-VkImageViewCreateInfo-pNext-01585", pCreateInfo->image,
                             create_info_loc.pNext(Struct::VkImageFormatListCreateInfo, Field::pViewFormats),
                             "has no formats that match the VkImageViewCreateInfo::format (%s).", string_VkFormat(view_format));
        }
    }

    const bool multiplane_image = vkuFormatIsMultiplane(image_format);
    if (multiplane_image && IsMultiplePlaneAspect(aspect_mask)) {
        skip |= LogError("VUID-VkImageViewCreateInfo-subresourceRange-07818", pCreateInfo->image,
                         create_info_loc.dot(Field::subresourceRange).dot(Field::aspectMask), "(%s) is invalid for %s.",
                         string_VkImageAspectFlags(aspect_mask).c_str(), string_VkFormat(image_format));
    }

    // Validate VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT state, if view/image formats differ
    if ((image_flags & VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT) && (image_format != view_format)) {
        const auto view_class = vkuFormatCompatibilityClass(view_format);
        if (multiplane_image) {
            const VkFormat compat_format = vkuFindMultiplaneCompatibleFormat(image_format, static_cast<VkImageAspectFlagBits>(aspect_mask));
            const auto image_class = vkuFormatCompatibilityClass(compat_format);
            // Need valid aspect mask otherwise will throw extra error when getting compatible format
            // Also this can be VK_IMAGE_ASPECT_COLOR_BIT
            const bool has_valid_aspect = IsOnlyOneValidPlaneAspect(image_format, aspect_mask);
            if (has_valid_aspect && ((image_class != view_class) || (image_class == VKU_FORMAT_COMPATIBILITY_CLASS_NONE))) {
                // Need to only check if one is NONE to handle edge case both are NONE
                // View format must match the multiplane compatible format
                skip |= LogError("VUID-VkImageViewCreateInfo-image-01586", pCreateInfo->image, create_info_loc.dot(Field::format),
                                 "(%s) is not compatible with plane %" PRIu32 " of the %s format %s, must be compatible with %s.",
                                 string_VkFormat(view_format), vkuGetPlaneIndex(static_cast<VkImageAspectFlagBits>(aspect_mask)), FormatHandle(pCreateInfo->image).c_str(),
                                 string_VkFormat(image_format), string_VkFormat(compat_format));
            }
        } else if (!(image_flags & VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT)) {
            // Format MUST be compatible (in the same format compatibility class) as the format the image was created with
            const auto image_class = vkuFormatCompatibilityClass(image_format);
            // Need to only check if one is NONE to handle edge case both are NONE
            if ((image_class != view_class) || (image_class == VKU_FORMAT_COMPATIBILITY_CLASS_NONE)) {
                skip |=
                    LogError("VUID-VkImageViewCreateInfo-image-01761", pCreateInfo->image, create_info_loc.dot(Field::format),
                             "%s is not in the same format compatibility class as %s format %s. Images created with the "
                             "VK_IMAGE_CREATE_MUTABLE_FORMAT BIT can support ImageViews with differing formats but they must be in "
                             "the same compatibility class.",
                             string_VkFormat(view_format), FormatHandle(pCreateInfo->image).c_str(), string_VkFormat(image_format));
            }
        }
    } else {
        // Format MUST be IDENTICAL to the format the image was created with
        // Unless it is a multi-planar color bit aspect
        if ((image_format != view_format) && (!multiplane_image || (aspect_mask != VK_IMAGE_ASPECT_COLOR_BIT))) {
            skip |= LogError("VUID-VkImageViewCreateInfo-image-01762", pCreateInfo->image, create_info_loc.dot(Field::format),
                             "%s is different from %s format (%s). Formats MUST be IDENTICAL unless VK_IMAGE_CREATE_MUTABLE_FORMAT "
                             "BIT was set on image creation.",
                             string_VkFormat(view_format), FormatHandle(pCreateInfo->image).c_str(), string_VkFormat(image_format));
        }
    }

    if (image_state.createInfo.samples != VK_SAMPLE_COUNT_1_BIT && view_type != VK_IMAGE_VIEW_TYPE_2D &&
        view_type != VK_IMAGE_VIEW_TYPE_2D_ARRAY) {
        skip |= LogError("VUID-VkImageViewCreateInfo-image-04972", pCreateInfo->image, create_info_loc.dot(Field::image),
                         "was created with sample count %s, but pCreateInfo->viewType is %s.",
                         string_VkSampleCountFlagBits(image_state.createInfo.samples), string_VkImageViewType(view_type));
    }

    if (image_usage & (VK_IMAGE_USAGE_VIDEO_ENCODE_DST_BIT_KHR | VK_IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT_KHR |
                       VK_IMAGE_USAGE_VIDEO_ENCODE_DPB_BIT_KHR)) {
        if (view_type != VK_IMAGE_VIEW_TYPE_2D && view_type != VK_IMAGE_VIEW_TYPE_2D_ARRAY) {
            skip |= LogError("VUID-VkImageViewCreateInfo-image-04818", pCreateInfo->image, create_info_loc.dot(Field::image),
                             "was created with video encode usage flags, but pCreateInfo->viewType (%s) "
                             "is not VK_IMAGE_VIEW_TYPE_2D or VK_IMAGE_VIEW_TYPE_2D_ARRAY.",
                             string_VkImageViewType(view_type));
        }
        if (!IsIdentitySwizzle(pCreateInfo->components)) {
            skip |= LogError(
                "VUID-VkImageViewCreateInfo-image-04818", pCreateInfo->image, create_info_loc.dot(Field::image),
                "was created with video encode usage flags, but not all members of "
                "pCreateInfo->components have identity swizzle. Here are the actual swizzle values:\n"
                "r swizzle = %s\n"
                "g swizzle = %s\n"
                "b swizzle = %s\n"
                "a swizzle = %s\n",
                string_VkComponentSwizzle(pCreateInfo->components.r), string_VkComponentSwizzle(pCreateInfo->components.g),
                string_VkComponentSwizzle(pCreateInfo->components.b), string_VkComponentSwizzle(pCreateInfo->components.a));
        }
    }

    // Validate correct image aspect bits for desired formats and format consistency
    skip |= ValidateImageAspectMask(image_state.image(), image_format, aspect_mask, image_state.disjoint, error_obj.location);

    // Valdiate Image/ImageView type compatibility #resources-image-views-compatibility
    switch (image_type) {
        case VK_IMAGE_TYPE_1D:
            if (view_type != VK_IMAGE_VIEW_TYPE_1D && view_type != VK_IMAGE_VIEW_TYPE_1D_ARRAY) {
                skip |= LogError("VUID-VkImageViewCreateInfo-subResourceRange-01021", pCreateInfo->image,
                                 create_info_loc.dot(Field::viewType), "(%s) is not compatible with image type %s.",
                                 string_VkImageViewType(view_type), string_VkImageType(image_type));
            }
            break;
        case VK_IMAGE_TYPE_2D:
            if (view_type != VK_IMAGE_VIEW_TYPE_2D && view_type != VK_IMAGE_VIEW_TYPE_2D_ARRAY) {
                if ((view_type == VK_IMAGE_VIEW_TYPE_CUBE || view_type == VK_IMAGE_VIEW_TYPE_CUBE_ARRAY) &&
                    !(image_flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT)) {
                    skip |= LogError("VUID-VkImageViewCreateInfo-image-01003", pCreateInfo->image,
                                     create_info_loc.dot(Field::viewType), "(%s) is not compatible with image type %s.",
                                     string_VkImageViewType(view_type), string_VkImageType(image_type));
                } else if (view_type != VK_IMAGE_VIEW_TYPE_CUBE && view_type != VK_IMAGE_VIEW_TYPE_CUBE_ARRAY) {
                    skip |= LogError("VUID-VkImageViewCreateInfo-subResourceRange-01021", pCreateInfo->image,
                                     create_info_loc.dot(Field::viewType), "(%s) is not compatible with image type %s.",
                                     string_VkImageViewType(view_type), string_VkImageType(image_type));
                }
            }
            break;
        case VK_IMAGE_TYPE_3D:
            if (IsExtEnabled(device_extensions.vk_khr_maintenance1)) {
                if (view_type != VK_IMAGE_VIEW_TYPE_3D) {
                    if ((view_type == VK_IMAGE_VIEW_TYPE_2D || view_type == VK_IMAGE_VIEW_TYPE_2D_ARRAY)) {
                        if (IsExtEnabled(device_extensions.vk_ext_image_2d_view_of_3d)) {
                            if (!(image_flags & VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT)) {
                                if (view_type == VK_IMAGE_VIEW_TYPE_2D_ARRAY) {
                                    skip |= LogError(
                                        "VUID-VkImageViewCreateInfo-image-06723", pCreateInfo->image,
                                        create_info_loc.dot(Field::viewType),
                                        "(%s) is not compatible with image type "
                                        "%s since the image doesn't have VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT flag set.",
                                        string_VkImageViewType(view_type), string_VkImageType(image_type));
                                } else if (view_type == VK_IMAGE_VIEW_TYPE_2D &&
                                           !(image_flags & VK_IMAGE_CREATE_2D_VIEW_COMPATIBLE_BIT_EXT)) {
                                    skip |= LogError("VUID-VkImageViewCreateInfo-image-06728", pCreateInfo->image,
                                                     create_info_loc.dot(Field::viewType),
                                                     "(%s) is not compatible with image type "
                                                     "%s since the image doesn't have VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT or "
                                                     "VK_IMAGE_CREATE_2D_VIEW_COMPATIBLE_BIT_EXT flag set.",
                                                     string_VkImageViewType(view_type), string_VkImageType(image_type));
                                }
                            }
                        } else if (!(image_flags & VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT) &&
                                   (view_type == VK_IMAGE_VIEW_TYPE_2D)) {
                            // TODO - combine this logic
                            skip |= LogError("VUID-VkImageViewCreateInfo-image-06728", pCreateInfo->image,
                                             create_info_loc.dot(Field::viewType),
                                             "VK_IMAGE_VIEW_TYPE_2D is not compatible "
                                             "with image type "
                                             "%s since the image doesn't have VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT flag set.",
                                             string_VkImageType(image_type));
                        }
                        if ((image_flags & (VK_IMAGE_CREATE_SPARSE_BINDING_BIT | VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT |
                                            VK_IMAGE_CREATE_SPARSE_ALIASED_BIT))) {
                            skip |= LogError(
                                "VUID-VkImageViewCreateInfo-image-04971", pCreateInfo->image, create_info_loc.dot(Field::viewType),
                                "(%s) is not compatible with image type %s "
                                "when the VK_IMAGE_CREATE_SPARSE_BINDING_BIT, VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT, or "
                                "VK_IMAGE_CREATE_SPARSE_ALIASED_BIT flags are enabled.",
                                string_VkImageViewType(view_type), string_VkImageType(image_type));
                        } else if (pCreateInfo->subresourceRange.levelCount != 1) {
                            skip |= LogError("VUID-VkImageViewCreateInfo-image-04970", pCreateInfo->image,
                                             create_info_loc.dot(Field::viewType),
                                             "(%s) is with image type %s must have a "
                                             "levelCount of 1 but it is %" PRIu32 ".",
                                             string_VkImageViewType(view_type), string_VkImageType(image_type),
                                             pCreateInfo->subresourceRange.levelCount);
                        }
                    } else {
                        skip |= LogError("VUID-VkImageViewCreateInfo-subResourceRange-01021", pCreateInfo->image,
                                         create_info_loc.dot(Field::viewType), "(%s) is not compatible with image type %s.",
                                         string_VkImageViewType(view_type), string_VkImageType(image_type));
                    }
                }
            } else {
                if (view_type != VK_IMAGE_VIEW_TYPE_3D) {
                    // Help point to VK_KHR_maintenance1
                    if ((view_type == VK_IMAGE_VIEW_TYPE_2D || view_type == VK_IMAGE_VIEW_TYPE_2D_ARRAY)) {
                        skip |= LogError("VUID-VkImageViewCreateInfo-subResourceRange-01021", pCreateInfo->image,
                                         create_info_loc.dot(Field::viewType),
                                         "(%s) is not compatible with image type %s "
                                         "without VK_KHR_maintenance1 enabled which was promoted in Vulkan 1.0.",
                                         string_VkImageViewType(view_type), string_VkImageType(image_type));
                    } else {
                        skip |= LogError("VUID-VkImageViewCreateInfo-subResourceRange-01021", pCreateInfo->image,
                                         create_info_loc.dot(Field::viewType), "(%s) is not compatible with image type %s.",
                                         string_VkImageViewType(view_type), string_VkImageType(image_type));
                    }
                }
            }
            break;
        default:
            break;
    }

    skip |= ValidateCreateImageViewANDROID(pCreateInfo, create_info_loc);
    skip |= ValidateImageViewFormatFeatures(image_state, view_format, image_usage, create_info_loc);

    if (enabled_features.shadingRateImage) {
        if (image_usage & VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV) {
            if (view_format != VK_FORMAT_R8_UINT) {
                skip |= LogError("VUID-VkImageViewCreateInfo-image-02087", pCreateInfo->image, create_info_loc.dot(Field::image),
                                 "was created with usage containing "
                                 "VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV, format must be VK_FORMAT_R8_UINT.");
            }
        }
    }

    if (enabled_features.shadingRateImage || enabled_features.attachmentFragmentShadingRate) {
        if (image_usage & VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR) {
            if (view_type != VK_IMAGE_VIEW_TYPE_2D && view_type != VK_IMAGE_VIEW_TYPE_2D_ARRAY) {
                skip |= LogError("VUID-VkImageViewCreateInfo-image-02086", pCreateInfo->image, create_info_loc.dot(Field::image),
                                 "was created with usage containing "
                                 "VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR, viewType must be "
                                 "VK_IMAGE_VIEW_TYPE_2D or VK_IMAGE_VIEW_TYPE_2D_ARRAY.");
            }
        }
    }

    if (enabled_features.attachmentFragmentShadingRate &&
        !phys_dev_ext_props.fragment_shading_rate_props.layeredShadingRateAttachments &&
        image_usage & VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR && normalized_subresource_range.layerCount != 1) {
        skip |= LogError("VUID-VkImageViewCreateInfo-usage-04551", pCreateInfo->image,
                         create_info_loc.dot(Field::subresourceRange).dot(Field::layerCount),
                         "is %" PRIu32 " for a shading rate attachment image view.", normalized_subresource_range.layerCount);
    }

    if (layer_count == VK_REMAINING_ARRAY_LAYERS) {
        const uint32_t remaining_layers = image_state.createInfo.arrayLayers - pCreateInfo->subresourceRange.baseArrayLayer;
        if (view_type == VK_IMAGE_VIEW_TYPE_CUBE && remaining_layers != 6) {
            skip |= LogError("VUID-VkImageViewCreateInfo-viewType-02962", pCreateInfo->image,
                             create_info_loc.dot(Field::subresourceRange).dot(Field::layerCount),
                             "VK_REMAINING_ARRAY_LAYERS=(%d) must be 6.", remaining_layers);
        }
        if (view_type == VK_IMAGE_VIEW_TYPE_CUBE_ARRAY && ((remaining_layers) % 6) != 0) {
            skip |= LogError("VUID-VkImageViewCreateInfo-viewType-02963", pCreateInfo->image,
                             create_info_loc.dot(Field::subresourceRange).dot(Field::layerCount),
                             "VK_REMAINING_ARRAY_LAYERS=(%d) must be a multiple of 6.", remaining_layers);
        }
        if ((remaining_layers != 1) && ((view_type == VK_IMAGE_VIEW_TYPE_1D) || (view_type == VK_IMAGE_VIEW_TYPE_2D) ||
                                        (view_type == VK_IMAGE_VIEW_TYPE_3D))) {
            skip |=
                LogError("VUID-VkImageViewCreateInfo-imageViewType-04974", pCreateInfo->image, create_info_loc.dot(Field::viewType),
                         "%s and the subresourceRange.layerCount "
                         "VK_REMAINING_ARRAY_LAYERS=(%d) and must 1 (try looking into VK_IMAGE_VIEW_TYPE_*_ARRAY).",
                         string_VkImageViewType(view_type), remaining_layers);
        }
    } else {
        if ((layer_count != 1) && ((view_type == VK_IMAGE_VIEW_TYPE_1D) || (view_type == VK_IMAGE_VIEW_TYPE_2D) ||
                                   (view_type == VK_IMAGE_VIEW_TYPE_3D))) {
            skip |= LogError("VUID-VkImageViewCreateInfo-imageViewType-04973", pCreateInfo->image,
                             create_info_loc.dot(Field::subresourceRange).dot(Field::layerCount),
                             "(%" PRIu32 ") must be 1 when using viewType %s (try looking into VK_IMAGE_VIEW_TYPE_*_ARRAY).",
                             layer_count, string_VkImageViewType(view_type));
        }
    }

    if (image_usage & VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT) {
        if (normalized_subresource_range.levelCount != 1) {
            skip |= LogError("VUID-VkImageViewCreateInfo-image-02571", pCreateInfo->image, create_info_loc.dot(Field::image),
                             "was created with usage containing "
                             "VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT, subresourceRange.levelCount (%d) must be 1",
                             pCreateInfo->subresourceRange.levelCount);
        }
    }
    if (pCreateInfo->flags & VK_IMAGE_VIEW_CREATE_FRAGMENT_DENSITY_MAP_DYNAMIC_BIT_EXT) {
        if (!enabled_features.fragmentDensityMapDynamic) {
            skip |= LogError("VUID-VkImageViewCreateInfo-flags-02572", pCreateInfo->image, create_info_loc.dot(Field::flags),
                             "contains VK_IMAGE_VIEW_CREATE_FRAGMENT_DENSITY_MAP_DYNAMIC_BIT_EXT but the fragmentDensityMapDynamic "
                             "feature is not enabled.");
        }
    } else {
        if (image_usage & VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT) {
            if (image_flags & (VK_IMAGE_CREATE_PROTECTED_BIT | VK_IMAGE_CREATE_SPARSE_BINDING_BIT |
                               VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT | VK_IMAGE_CREATE_SPARSE_ALIASED_BIT)) {
                skip |= LogError("VUID-VkImageViewCreateInfo-flags-04116", pCreateInfo->image, create_info_loc.dot(Field::image),
                                 "was created with usage containing "
                                 "VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT flags must not contain any of "
                                 "VK_IMAGE_CREATE_PROTECTED_BIT, VK_IMAGE_CREATE_SPARSE_BINDING_BIT, "
                                 "VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT, or VK_IMAGE_CREATE_SPARSE_ALIASED_BIT");
            }
        }
    }

    if (image_flags & VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT) {
        if (!vkuFormatIsCompressed(view_format)) {
            if (pCreateInfo->subresourceRange.levelCount != 1) {
                skip |= LogError("VUID-VkImageViewCreateInfo-image-07072", pCreateInfo->image, create_info_loc.dot(Field::image),
                                 "was created with VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT bit, "
                                 "and format (%s) is not compressed, but subresourcesRange.levelCount (%" PRIu32 ") is not 1.",
                                 string_VkFormat(view_format), pCreateInfo->subresourceRange.levelCount);
            }
            if (pCreateInfo->subresourceRange.layerCount != 1) {
                skip |= LogError("VUID-VkImageViewCreateInfo-image-07072", pCreateInfo->image, create_info_loc.dot(Field::image),
                                 "was created with VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT bit, "
                                 "and format (%s) is not compressed, but subresourcesRange.layerCount (%" PRIu32 ") is not 1.",
                                 string_VkFormat(view_format), pCreateInfo->subresourceRange.layerCount);
            }
        }

        const bool class_compatible = vkuFormatCompatibilityClass(view_format) == vkuFormatCompatibilityClass(image_format);
        // "uncompressed format that is size-compatible" so if compressed, same as not being compatible
        const bool size_compatible =
            vkuFormatIsCompressed(view_format) ? false : vkuFormatElementSize(view_format) == vkuFormatElementSize(image_format);
        if (!class_compatible && !size_compatible) {
            skip |= LogError("VUID-VkImageViewCreateInfo-image-01583", pCreateInfo->image, create_info_loc.dot(Field::image),
                             "was created with VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT bit and "
                             "format (%s), but pCreateInfo->format (%s) are not compatible.",
                             string_VkFormat(image_format), string_VkFormat(view_format));
        }
    }

    if (pCreateInfo->flags & VK_IMAGE_VIEW_CREATE_FRAGMENT_DENSITY_MAP_DEFERRED_BIT_EXT) {
        if (!enabled_features.fragmentDensityMapDeferred) {
            skip |= LogError("VUID-VkImageViewCreateInfo-flags-03567", pCreateInfo->image, create_info_loc.dot(Field::flags),
                             "includes VK_IMAGE_VIEW_CREATE_FRAGMENT_DENSITY_MAP_DEFERRED_BIT_EXT but the "
                             "fragmentDensityMapDeferred feature is not enabled.");
        }
        if (pCreateInfo->flags & VK_IMAGE_VIEW_CREATE_FRAGMENT_DENSITY_MAP_DYNAMIC_BIT_EXT) {
            skip |= LogError("VUID-VkImageViewCreateInfo-flags-03568", pCreateInfo->image, create_info_loc.dot(Field::flags),
                             "(%s) includes both DEFERRED_BIT and DYNAMIC_BIT.",
                             string_VkImageViewCreateFlags(pCreateInfo->flags).c_str());
        }
    }
    if (IsExtEnabled(device_extensions.vk_ext_fragment_density_map2)) {
        if ((image_flags & VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT) && (image_usage & VK_IMAGE_USAGE_SAMPLED_BIT) &&
            (layer_count > phys_dev_ext_props.fragment_density_map2_props.maxSubsampledArrayLayers)) {
            skip |= LogError("VUID-VkImageViewCreateInfo-image-03569", pCreateInfo->image, create_info_loc.dot(Field::image),
                             "was created with flags containing "
                             "VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT and usage containing VK_IMAGE_USAGE_SAMPLED_BIT "
                             "subresourceRange.layerCount (%d) must: be less than or equal to maxSubsampledArrayLayers (%d)",
                             layer_count, phys_dev_ext_props.fragment_density_map2_props.maxSubsampledArrayLayers);
        }
    }

    if (const auto astc_decode_mode = vku::FindStructInPNextChain<VkImageViewASTCDecodeModeEXT>(pCreateInfo->pNext);
        (astc_decode_mode != nullptr)) {
        if ((enabled_features.decodeModeSharedExponent == VK_FALSE) &&
            (astc_decode_mode->decodeMode == VK_FORMAT_E5B9G9R9_UFLOAT_PACK32)) {
            skip |= LogError("VUID-VkImageViewASTCDecodeModeEXT-decodeMode-02231", pCreateInfo->image,
                             create_info_loc.pNext(Struct::VkImageViewASTCDecodeModeEXT, Field::decodeMode),
                             "is VK_FORMAT_E5B9G9R9_UFLOAT_PACK32 but decodeModeSharedExponent feature is not enabled.");
        }
    }

    if (IsExtEnabled(device_extensions.vk_khr_portability_subset)) {
        // If swizzling is disabled, make sure it isn't used
        // NOTE: as of spec version 1.2.183, VUID 04465 states: "all elements of components _must_ be
        // VK_COMPONENT_SWIZZLE_IDENTITY."
        //       However, issue https://github.com/KhronosGroup/Vulkan-Portability/issues/27 points out that the identity can
        //       also be defined via R, G, B, A enums in the correct order.
        //       Spec change is at https://gitlab.khronos.org/vulkan/vulkan/-/merge_requests/4600
        if ((VK_FALSE == enabled_features.imageViewFormatSwizzle) && !IsIdentitySwizzle(pCreateInfo->components)) {
            skip |= LogError("VUID-VkImageViewCreateInfo-imageViewFormatSwizzle-04465", pCreateInfo->image, create_info_loc,
                             "(portability error): swizzle is disabled for this device.");
        }

        // Ensure ImageView's format has the same number of bits and components as Image's format if format reinterpretation is
        // disabled
        if (!enabled_features.imageViewFormatReinterpretation &&
            !FormatsEqualComponentBits(pCreateInfo->format, image_state.createInfo.format)) {
            skip |=
                LogError("VUID-VkImageViewCreateInfo-imageViewFormatReinterpretation-04466", pCreateInfo->image, create_info_loc,
                         "(portability error): ImageView format must have"
                         " the same number of components and bits per component as the Image's format");
        }
    }

    if (const auto image_view_min_lod = vku::FindStructInPNextChain<VkImageViewMinLodCreateInfoEXT>(pCreateInfo->pNext); image_view_min_lod) {
        if ((!enabled_features.minLod) && (image_view_min_lod->minLod != 0)) {
            skip |= LogError("VUID-VkImageViewMinLodCreateInfoEXT-minLod-06455", pCreateInfo->image,
                             create_info_loc.pNext(Struct::VkImageViewMinLodCreateInfoEXT, Field::minLod),
                             "%f, but the minLod feature is not enabled.", image_view_min_lod->minLod);
        }
        const auto max_level =
            static_cast<float>(pCreateInfo->subresourceRange.baseMipLevel + (pCreateInfo->subresourceRange.levelCount - 1));
        if (image_view_min_lod->minLod > max_level) {
            skip |= LogError("VUID-VkImageViewMinLodCreateInfoEXT-minLod-06456", pCreateInfo->image,
                             create_info_loc.pNext(Struct::VkImageViewMinLodCreateInfoEXT, Field::minLod),
                             "(%f) must be less or equal to the index of the last mipmap level "
                             "accessible to the view (%f).",
                             image_view_min_lod->minLod, max_level);
        }
    }

    const auto ycbcr_conversion = vku::FindStructInPNextChain<VkSamplerYcbcrConversionInfo>(pCreateInfo->pNext);
    if (ycbcr_conversion && ycbcr_conversion->conversion != VK_NULL_HANDLE) {
        auto ycbcr_state = Get<SAMPLER_YCBCR_CONVERSION_STATE>(ycbcr_conversion->conversion);
        if (pCreateInfo->format != ycbcr_state->format) {
            skip |= LogError("VUID-VkImageViewCreateInfo-pNext-06658", pCreateInfo->image,
                             create_info_loc.pNext(Struct::VkSamplerYcbcrConversionInfo, Field::conversion),
                             "was created with format %s which is different than pCreateInfo->format %s.",
                             string_VkFormat(ycbcr_state->format), string_VkFormat(view_format));
        }
    } else if ((image_usage & VK_IMAGE_USAGE_SAMPLED_BIT) && FormatRequiresYcbcrConversionExplicitly(view_format)) {
        skip |= LogError("VUID-VkImageViewCreateInfo-format-06415", pCreateInfo->image, create_info_loc.dot(Field::image),
                         "was created with VK_IMAGE_USAGE_SAMPLED_BIT, but pCreateInfo->format %s requires a "
                         "VkSamplerYcbcrConversion but one was not passed in the pNext chain.",
                         string_VkFormat(view_format));
    }

    if (pCreateInfo->viewType != VK_IMAGE_VIEW_TYPE_2D && pCreateInfo->viewType != VK_IMAGE_VIEW_TYPE_2D_ARRAY) {
        VkImageUsageFlags decode_usage = VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR | VK_IMAGE_USAGE_VIDEO_DECODE_SRC_BIT_KHR |
                                         VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR;
        if (image_usage & decode_usage) {
            skip |= LogError("VUID-VkImageViewCreateInfo-image-04817", pCreateInfo->image, create_info_loc.dot(Field::viewType),
                             "%s is incompatible with decode usage.", string_VkImageViewType(pCreateInfo->viewType));
        }
    }

    if ((pCreateInfo->flags & VK_IMAGE_VIEW_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT) &&
        !enabled_features.descriptorBufferCaptureReplay) {
        skip |= LogError("VUID-VkImageViewCreateInfo-flags-08106", pCreateInfo->image, create_info_loc.dot(Field::flags),
                         "includes VK_IMAGE_VIEW_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT but the "
                         "descriptorBufferCaptureReplay feature is not enabled.");
    }

    if (const auto opaque_capture_descriptor_buffer =
            vku::FindStructInPNextChain<VkOpaqueCaptureDescriptorDataCreateInfoEXT>(pCreateInfo->pNext);
        opaque_capture_descriptor_buffer && !(pCreateInfo->flags & VK_IMAGE_VIEW_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT)) {
        skip |= LogError("VUID-VkImageViewCreateInfo-pNext-08107", pCreateInfo->image, create_info_loc.dot(Field::flags),
                         "(%s) is missing VK_IMAGE_VIEW_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT but "
                         "VkOpaqueCaptureDescriptorDataCreateInfoEXT is in pNext chain.",
                         string_VkImageViewCreateFlags(pCreateInfo->flags).c_str());
    }

    skip |= ValidateImageViewSampleWeightQCOM(pCreateInfo, image_state, create_info_loc);

    return skip;
}

bool CoreChecks::PreCallValidateDestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks *pAllocator,
                                                 const ErrorObject &error_obj) const {
    auto image_view_state = Get<IMAGE_VIEW_STATE>(imageView);

    bool skip = false;
    if (image_view_state) {
        skip |= ValidateObjectNotInUse(image_view_state.get(), error_obj.location, "VUID-vkDestroyImageView-imageView-01026");
    }
    return skip;
}

bool CoreChecks::ValidateGetImageSubresourceLayout(const IMAGE_STATE &image_state, const VkImageSubresource &subresource,
                                                   const Location &subresource_loc) const {
    bool skip = false;
    const bool is_2 = subresource_loc.function != Func::vkGetImageSubresourceLayout;
    const VkImageAspectFlags aspect_mask = subresource.aspectMask;

    // The aspectMask member of pSubresource must only have a single bit set
    if (GetBitSetCount(aspect_mask) != 1) {
        const char *vuid =
            is_2 ? "VUID-vkGetImageSubresourceLayout2KHR-aspectMask-00997" : "VUID-vkGetImageSubresourceLayout-aspectMask-00997";
        skip |= LogError(vuid, image_state.image(), subresource_loc.dot(Field::aspectMask), "(%s) must have exactly 1 bit set.",
                         string_VkImageAspectFlags(aspect_mask).c_str());
    }

    // mipLevel must be less than the mipLevels specified in VkImageCreateInfo when the image was created
    if (subresource.mipLevel >= image_state.createInfo.mipLevels) {
        const char *vuid =
            is_2 ? "VUID-vkGetImageSubresourceLayout2KHR-mipLevel-01716" : "VUID-vkGetImageSubresourceLayout-mipLevel-01716";
        skip |= LogError(vuid, image_state.image(), subresource_loc.dot(Field::mipLevel),
                         "(%" PRIu32 ") must be less than %" PRIu32 ".", subresource.mipLevel, image_state.createInfo.mipLevels);
    }

    // arrayLayer must be less than the arrayLayers specified in VkImageCreateInfo when the image was created
    if (subresource.arrayLayer >= image_state.createInfo.arrayLayers) {
        const char *vuid =
            is_2 ? "VUID-vkGetImageSubresourceLayout2KHR-arrayLayer-01717" : "VUID-vkGetImageSubresourceLayout-arrayLayer-01717";
        skip |=
            LogError(vuid, image_state.image(), subresource_loc.dot(Field::arrayLayer),
                     "(%" PRIu32 ") must be less than %" PRIu32 ".", subresource.arrayLayer, image_state.createInfo.arrayLayers);
    }

    const VkFormat image_format = image_state.createInfo.format;
    const bool tiling_linear_optimal =
        image_state.createInfo.tiling == VK_IMAGE_TILING_LINEAR || image_state.createInfo.tiling == VK_IMAGE_TILING_OPTIMAL;
    if (vkuFormatIsColor(image_format) && !vkuFormatIsMultiplane(image_format) && (aspect_mask != VK_IMAGE_ASPECT_COLOR_BIT) &&
        tiling_linear_optimal) {
        const char *vuid =
            is_2 ? "VUID-vkGetImageSubresourceLayout2KHR-format-08886" : "VUID-vkGetImageSubresourceLayout-format-08886";
        skip |= LogError(vuid, image_state.image(), subresource_loc.dot(Field::aspectMask),
                         "is %s but image was created with color format %s.", string_VkImageAspectFlags(aspect_mask).c_str(),
                         string_VkFormat(image_format));
    }

    if (vkuFormatHasDepth(image_format) && ((aspect_mask & VK_IMAGE_ASPECT_DEPTH_BIT) == 0)) {
        const char *vuid =
            is_2 ? "VUID-vkGetImageSubresourceLayout2KHR-format-04462" : "VUID-vkGetImageSubresourceLayout-format-04462";
        skip |= LogError(vuid, image_state.image(), subresource_loc.dot(Field::aspectMask),
                         "is %s but image was created with depth format %s.", string_VkImageAspectFlags(aspect_mask).c_str(),
                         string_VkFormat(image_format));
    }

    if (vkuFormatHasStencil(image_format) && ((aspect_mask & VK_IMAGE_ASPECT_STENCIL_BIT) == 0)) {
        const char *vuid =
            is_2 ? "VUID-vkGetImageSubresourceLayout2KHR-format-04463" : "VUID-vkGetImageSubresourceLayout-format-04463";
        skip |= LogError(vuid, image_state.image(), subresource_loc.dot(Field::aspectMask),
                         "is %s but image was created with stencil format %s.", string_VkImageAspectFlags(aspect_mask).c_str(),
                         string_VkFormat(image_format));
    }

    if (!vkuFormatHasDepth(image_format) && !vkuFormatHasStencil(image_format)) {
        if ((aspect_mask & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) != 0) {
            const char *vuid =
                is_2 ? "VUID-vkGetImageSubresourceLayout2KHR-format-04464" : "VUID-vkGetImageSubresourceLayout-format-04464";
            skip |= LogError(vuid, image_state.image(), subresource_loc.dot(Field::aspectMask),
                             "is %s but image was created with format %s.", string_VkImageAspectFlags(aspect_mask).c_str(),
                             string_VkFormat(image_format));
        }
    }

    // subresource's aspect must be compatible with image's format.
    if (image_state.createInfo.tiling == VK_IMAGE_TILING_LINEAR) {
        if (vkuFormatIsMultiplane(image_format) && !IsOnlyOneValidPlaneAspect(image_format, aspect_mask)) {
            const char *vuid =
                is_2 ? "VUID-vkGetImageSubresourceLayout2KHR-tiling-08717" : "VUID-vkGetImageSubresourceLayout-tiling-08717";
            skip |= LogError(vuid, image_state.image(), subresource_loc.dot(Field::aspectMask), "(%s) is invalid for format %s.",
                             string_VkImageAspectFlags(aspect_mask).c_str(), string_VkFormat(image_format));
        }
    } else if (image_state.createInfo.tiling == VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT) {
        if ((aspect_mask != VK_IMAGE_ASPECT_MEMORY_PLANE_0_BIT_EXT) && (aspect_mask != VK_IMAGE_ASPECT_MEMORY_PLANE_1_BIT_EXT) &&
            (aspect_mask != VK_IMAGE_ASPECT_MEMORY_PLANE_2_BIT_EXT) && (aspect_mask != VK_IMAGE_ASPECT_MEMORY_PLANE_3_BIT_EXT)) {
            const char *vuid =
                is_2 ? "VUID-vkGetImageSubresourceLayout2KHR-tiling-02271" : "VUID-vkGetImageSubresourceLayout-tiling-02271";
            skip |=
                LogError(vuid, image_state.image(), subresource_loc.dot(Field::aspectMask),
                         "(%s) must be VK_IMAGE_ASPECT_MEMORY_PLANE_i_BIT_EXT.", string_VkImageAspectFlags(aspect_mask).c_str());
        } else {
            // Parameter validation should catch if this is used without VK_EXT_image_drm_format_modifier
            assert(IsExtEnabled(device_extensions.vk_ext_image_drm_format_modifier));
            VkImageDrmFormatModifierPropertiesEXT drm_format_properties = vku::InitStructHelper();
            DispatchGetImageDrmFormatModifierPropertiesEXT(device, image_state.image(), &drm_format_properties);

            VkDrmFormatModifierPropertiesListEXT fmt_drm_props = vku::InitStructHelper();
            VkFormatProperties2 fmt_props_2 = vku::InitStructHelper(&fmt_drm_props);
            DispatchGetPhysicalDeviceFormatProperties2(physical_device, image_state.createInfo.format, &fmt_props_2);
            std::vector<VkDrmFormatModifierPropertiesEXT> drm_properties{fmt_drm_props.drmFormatModifierCount};
            fmt_drm_props.pDrmFormatModifierProperties = drm_properties.data();
            DispatchGetPhysicalDeviceFormatProperties2(physical_device, image_state.createInfo.format, &fmt_props_2);

            uint32_t max_plane_count = 0u;

            for (auto const &drm_property : drm_properties) {
                if (drm_format_properties.drmFormatModifier == drm_property.drmFormatModifier) {
                    max_plane_count = drm_property.drmFormatModifierPlaneCount;
                    break;
                }
            }

            VkImageAspectFlagBits allowed_plane_indices[] = {
                VK_IMAGE_ASPECT_MEMORY_PLANE_0_BIT_EXT, VK_IMAGE_ASPECT_MEMORY_PLANE_1_BIT_EXT,
                VK_IMAGE_ASPECT_MEMORY_PLANE_2_BIT_EXT, VK_IMAGE_ASPECT_MEMORY_PLANE_3_BIT_EXT};

            bool is_valid = false;

            for (uint32_t i = 0u; i < max_plane_count; ++i) {
                if (aspect_mask == allowed_plane_indices[i]) {
                    is_valid = true;
                    break;
                }
            }

            if (!is_valid) {
                const char *vuid =
                    is_2 ? "VUID-vkGetImageSubresourceLayout2KHR-tiling-02271" : "VUID-vkGetImageSubresourceLayout-tiling-02271";
                skip |= LogError(vuid, image_state.image(), subresource_loc.dot(Field::aspectMask),
                                 "is %s for image format %s, but drmFormatModifierPlaneCount is %" PRIu32 " (drmFormatModifier = %" PRIu64 ").",
                                 string_VkImageAspectFlags(aspect_mask).c_str(), string_VkFormat(image_state.createInfo.format),
                                 max_plane_count, drm_format_properties.drmFormatModifier);
            }
        }
    }

    if (image_state.IsExternalBuffer() && image_state.GetBoundMemoryStates().empty()) {
        const char *vuid =
            is_2 ? "VUID-vkGetImageSubresourceLayout2KHR-image-01895" : "VUID-vkGetImageSubresourceLayout-image-01895";
        skip |= LogError(vuid, image_state.image(), subresource_loc,
                         "Attempt to query layout from an image created with "
                         "VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID handleType which has not yet been "
                         "bound to memory.");
    }

    return skip;
}

bool CoreChecks::PreCallValidateGetImageSubresourceLayout(VkDevice device, VkImage image, const VkImageSubresource *pSubresource,
                                                          VkSubresourceLayout *pLayout, const ErrorObject &error_obj) const {
    bool skip = false;
    auto image_state = Get<IMAGE_STATE>(image);
    if (pSubresource && pLayout && image_state) {
        skip = ValidateGetImageSubresourceLayout(*image_state, *pSubresource, error_obj.location.dot(Field::pSubresource));
        if ((image_state->createInfo.tiling != VK_IMAGE_TILING_LINEAR) &&
            (image_state->createInfo.tiling != VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT)) {
            skip |= LogError("VUID-vkGetImageSubresourceLayout-image-07790", image, error_obj.location,
                             "image was created with tiling %s.", string_VkImageTiling(image_state->createInfo.tiling));
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateGetImageSubresourceLayout2KHR(VkDevice device, VkImage image,
                                                              const VkImageSubresource2KHR *pSubresource,
                                                              VkSubresourceLayout2KHR *pLayout,
                                                              const ErrorObject &error_obj) const {
    bool skip = false;
    auto image_state = Get<IMAGE_STATE>(image);
    if (pSubresource && pLayout && image_state) {
        skip = ValidateGetImageSubresourceLayout(*image_state, pSubresource->imageSubresource,
                                                 error_obj.location.dot(Field::pSubresource).dot(Field::imageSubresource));
    }
    return skip;
}

bool CoreChecks::PreCallValidateGetImageSubresourceLayout2EXT(VkDevice device, VkImage image,
                                                              const VkImageSubresource2EXT *pSubresource,
                                                              VkSubresourceLayout2EXT *pLayout,
                                                              const ErrorObject &error_obj) const {
    return PreCallValidateGetImageSubresourceLayout2KHR(device, image, pSubresource, pLayout, error_obj);
}

bool CoreChecks::PreCallValidateGetImageDrmFormatModifierPropertiesEXT(VkDevice device, VkImage image,
                                                                       VkImageDrmFormatModifierPropertiesEXT *pProperties,
                                                                       const ErrorObject &error_obj) const {
    bool skip = false;
    auto image_state = Get<IMAGE_STATE>(image);
    if (image_state) {
        if (image_state->createInfo.tiling != VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT) {
            skip |=
                LogError("VUID-vkGetImageDrmFormatModifierPropertiesEXT-image-02272", image, error_obj.location.dot(Field::image),
                         "was created with tiling %s.", string_VkImageTiling(image_state->createInfo.tiling));
        }
    }
    return skip;
}

static const SubresourceRangeErrorCodes TransitionImageLayoutVUIDs{
    "VUID-VkHostImageLayoutTransitionInfoEXT-subresourceRange-01486",
    "VUID-VkHostImageLayoutTransitionInfoEXT-subresourceRange-01724",
    "VUID-VkHostImageLayoutTransitionInfoEXT-subresourceRange-01488",
    "VUID-VkHostImageLayoutTransitionInfoEXT-subresourceRange-01725",
};

bool CoreChecks::PreCallValidateTransitionImageLayoutEXT(VkDevice device, uint32_t transitionCount,
                                                         const VkHostImageLayoutTransitionInfoEXT *pTransitions,
                                                         const ErrorObject &error_obj) const {
    bool skip = false;

    for (uint32_t i = 0; i < transitionCount; ++i) {
        const Location transition_loc = error_obj.location.dot(Field::pTransitions, i);
        const auto &transition = pTransitions[i];
        const auto image_state = Get<IMAGE_STATE>(transition.image);
        const auto image_format = image_state->createInfo.format;
        const auto aspect_mask = transition.subresourceRange.aspectMask;
        const bool has_depth_mask = (aspect_mask & VK_IMAGE_ASPECT_DEPTH_BIT) != 0;
        const bool has_stencil_mask = (aspect_mask & VK_IMAGE_ASPECT_STENCIL_BIT) != 0;

        if ((image_state->createInfo.usage & VK_IMAGE_USAGE_HOST_TRANSFER_BIT_EXT) == 0) {
            const LogObjectList objlist(device, image_state->Handle());
            skip |= LogError("VUID-VkHostImageLayoutTransitionInfoEXT-image-09055", objlist, transition_loc.dot(Field::image),
                             "was created with usage (%s) which does not contain "
                             "VK_IMAGE_USAGE_HOST_TRANSFER_BIT_EXT.",
                             string_VkImageUsageFlags(image_state->createInfo.usage).c_str());
        }

        skip |= ValidateImageSubresourceRange(image_state->createInfo.mipLevels, image_state->createInfo.arrayLayers,
                                              transition.subresourceRange, "arrayLayers", image_state->image(),
                                              TransitionImageLayoutVUIDs, transition_loc.dot(Field::subresourceRange));
        skip |=
            ValidateMemoryIsBoundToImage(LogObjectList(device, transition.image), *image_state, transition_loc.dot(Field::image),
                                         "VUID-VkHostImageLayoutTransitionInfoEXT-image-01932");

        if (vkuFormatIsColor(image_format) && (aspect_mask != VK_IMAGE_ASPECT_COLOR_BIT)) {
            if (!vkuFormatIsMultiplane(image_format)) {
                const LogObjectList objlist(device, image_state->Handle());
                skip |= LogError("VUID-VkHostImageLayoutTransitionInfoEXT-image-09241", objlist,
                                 transition_loc.dot(Field::subresourceRange).dot(Field::aspectMask),
                                 "is %s (not VK_IMAGE_ASPECT_COLOR_BIT) and image was created with format %s.",
                                 string_VkImageAspectFlags(aspect_mask).c_str(), string_VkFormat(image_format));
            } else if (!image_state->disjoint) {
                const LogObjectList objlist(device, image_state->Handle());
                skip |= LogError("VUID-VkHostImageLayoutTransitionInfoEXT-image-09242", objlist,
                                 transition_loc.dot(Field::subresourceRange).dot(Field::aspectMask),
                                 "is %s (not VK_IMAGE_ASPECT_COLOR_BIT) and image was created with format %s.",
                                 string_VkImageAspectFlags(aspect_mask).c_str(), string_VkFormat(image_format));
            }
        }
        if ((vkuFormatIsMultiplane(image_format)) && (image_state->disjoint)) {
            if (!IsValidPlaneAspect(image_format, aspect_mask) && ((aspect_mask & VK_IMAGE_ASPECT_COLOR_BIT) == 0)) {
                const LogObjectList objlist(device, image_state->Handle());
                skip |= LogError("VUID-VkHostImageLayoutTransitionInfoEXT-image-01672", objlist,
                                 transition_loc.dot(Field::subresourceRange).dot(Field::aspectMask),
                                 "is %s and image was created with format %s.", string_VkImageAspectFlags(aspect_mask).c_str(),
                                 string_VkFormat(image_format));
            }
        }
        if (vkuFormatIsDepthAndStencil(image_format)) {
            if (enabled_features.separateDepthStencilLayouts) {
                if (!has_depth_mask && !has_stencil_mask) {
                    const LogObjectList objlist(device, image_state->Handle());
                    skip |= LogError("VUID-VkHostImageLayoutTransitionInfoEXT-image-03319", objlist,
                                     transition_loc.dot(Field::subresourceRange).dot(Field::aspectMask),
                                     "is %s and image was created with format %s.", string_VkImageAspectFlags(aspect_mask).c_str(),
                                     string_VkFormat(image_format));
                }
            } else {
                if (!has_depth_mask || !has_stencil_mask) {
                    const LogObjectList objlist(device, image_state->Handle());
                    skip |= LogError("VUID-VkHostImageLayoutTransitionInfoEXT-image-03320", objlist,
                                     transition_loc.dot(Field::subresourceRange).dot(Field::aspectMask),
                                     "is %s and image was created with format %s.", string_VkImageAspectFlags(aspect_mask).c_str(),
                                     string_VkFormat(image_format));
                }
            }
        }
        if (aspect_mask & VK_IMAGE_ASPECT_DEPTH_BIT) {
            if ((transition.oldLayout == VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL) ||
                (transition.oldLayout == VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL) ||
                (transition.newLayout == VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL) ||
                (transition.newLayout == VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL)) {
                const LogObjectList objlist(device, image_state->Handle());
                skip |= LogError("VUID-VkHostImageLayoutTransitionInfoEXT-aspectMask-08702", objlist,
                                 transition_loc.dot(Field::subresourceRange).dot(Field::aspectMask),
                                 "is %s meaning that neither oldLayout nor newLayout can be "
                                 "VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL or VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL"
                                 " (oldLayout = %s and newLayout = %s).",
                                 string_VkImageAspectFlags(aspect_mask).c_str(), string_VkImageLayout(transition.oldLayout),
                                 string_VkImageLayout(transition.oldLayout));
            }
        }
        if (aspect_mask & VK_IMAGE_ASPECT_STENCIL_BIT) {
            if ((transition.oldLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) ||
                (transition.oldLayout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL) ||
                (transition.newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) ||
                (transition.newLayout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL)) {
                const LogObjectList objlist(device, image_state->Handle());
                skip |= LogError("VUID-VkHostImageLayoutTransitionInfoEXT-aspectMask-08703", objlist,
                                 transition_loc.dot(Field::subresourceRange).dot(Field::aspectMask),
                                 "is %s meaning that neither oldLayout nor newLayout can be "
                                 "VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL or VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL"
                                 " (oldLayout = %s and newLayout = %s).",
                                 string_VkImageAspectFlags(aspect_mask).c_str(), string_VkImageLayout(transition.oldLayout),
                                 string_VkImageLayout(transition.oldLayout));
            }
        }
        if ((transition.oldLayout != VK_IMAGE_LAYOUT_UNDEFINED) && (transition.oldLayout != VK_IMAGE_LAYOUT_PREINITIALIZED)) {
            auto *props = &phys_dev_ext_props.host_image_copy_properties;
            skip |= ValidateHostCopyImageLayout(device, transition.image, props->copySrcLayoutCount, props->pCopySrcLayouts,
                                                transition.oldLayout, transition_loc.dot(Field::oldLayout), "pCopySrcLayouts",
                                                "VUID-VkHostImageLayoutTransitionInfoEXT-oldLayout-09230");
        }

        const auto *props = &phys_dev_ext_props.host_image_copy_properties;
        skip |= ValidateHostCopyImageLayout(device, transition.image, props->copyDstLayoutCount, props->pCopyDstLayouts,
                                            transition.newLayout, transition_loc.dot(Field::newLayout), "pCopyDstLayouts",
                                            "VUID-VkHostImageLayoutTransitionInfoEXT-newLayout-09057");
        if (transition.oldLayout != VK_IMAGE_LAYOUT_UNDEFINED) {
            skip |= ValidateHostCopyCurrentLayout(device, transition.oldLayout, transition.subresourceRange, i, *image_state,
                                                  transition_loc.dot(Field::oldLayout), "transition",
                                                  "VUID-VkHostImageLayoutTransitionInfoEXT-oldLayout-09229");
        }
    }
    return skip;
}

void CoreChecks::PostCallRecordTransitionImageLayoutEXT(VkDevice device, uint32_t transitionCount,
                                                        const VkHostImageLayoutTransitionInfoEXT *pTransitions,
                                                        const RecordObject &record_obj) {
    ValidationStateTracker::PostCallRecordTransitionImageLayoutEXT(device, transitionCount, pTransitions, record_obj);

    if (VK_SUCCESS != record_obj.result) return;

    for (uint32_t i = 0; i < transitionCount; ++i) {
        auto &transition = pTransitions[i];
        auto image_state = Get<IMAGE_STATE>(transition.image);
        if (!image_state) continue;
        image_state->SetImageLayout(transition.subresourceRange, transition.newLayout);
    }
}

// Validates the image is allowed to be protected
bool CoreChecks::ValidateProtectedImage(const CMD_BUFFER_STATE &cb_state, const IMAGE_STATE &image_state, const Location &loc,
                                        const char *vuid, const char *more_message) const {
    bool skip = false;

    // if driver supports protectedNoFault the operation is valid, just has undefined values
    if ((!phys_dev_props_core11.protectedNoFault) && (cb_state.unprotected == true) && (image_state.unprotected == false)) {
        const LogObjectList objlist(cb_state.Handle(), image_state.Handle());
        skip |= LogError(vuid, objlist, loc, "(%s) is a protected image, but command buffer (%s) is unprotected.%s",
                         FormatHandle(image_state).c_str(), FormatHandle(cb_state).c_str(), more_message);
    }
    return skip;
}

// Validates the image is allowed to be unprotected
bool CoreChecks::ValidateUnprotectedImage(const CMD_BUFFER_STATE &cb_state, const IMAGE_STATE &image_state, const Location &loc,
                                          const char *vuid, const char *more_message) const {
    bool skip = false;

    // if driver supports protectedNoFault the operation is valid, just has undefined values
    if ((!phys_dev_props_core11.protectedNoFault) && (cb_state.unprotected == false) && (image_state.unprotected == true)) {
        const LogObjectList objlist(cb_state.Handle(), image_state.Handle());
        skip |= LogError(vuid, objlist, loc, "(%s) is an unprotected image, but command buffer (%s) is protected.%s",
                         FormatHandle(image_state).c_str(), FormatHandle(cb_state).c_str(), more_message);
    }
    return skip;
}

bool CoreChecks::ValidateImageViewSampleWeightQCOM(const VkImageViewCreateInfo *pCreateInfo, const IMAGE_STATE &image_state,
                                                   const Location &loc) const {
    bool skip = false;

    auto sample_weight_info = vku::FindStructInPNextChain<VkImageViewSampleWeightCreateInfoQCOM>(pCreateInfo->pNext);
    if (!sample_weight_info) {
        return skip;
    }

    const VkImageAspectFlags aspect_mask = pCreateInfo->subresourceRange.aspectMask;
    const VkImageType image_type = image_state.createInfo.imageType;
    const VkImageUsageFlags image_usage = image_state.createInfo.usage;
    const VkExtent3D image_extent = image_state.createInfo.extent;
    const uint32_t layer_count = pCreateInfo->subresourceRange.layerCount;

    if ((enabled_features.textureSampleWeighted == VK_FALSE)) {
        skip |= LogError("VUID-VkImageViewCreateInfo-pNext-06944", pCreateInfo->image, loc,
                         "pNext chain includes VkImageViewSampleWeightCreateInfoQCOM "
                         "but textureSampleWeighted feature is not enabled.");
    }

    // Validate VkImage and VkImageView compatibility with  VkImageViewSampleWeightCreateInfoQCOM
    if ((image_usage & VK_IMAGE_USAGE_SAMPLE_WEIGHT_BIT_QCOM) == 0) {
        skip |= LogError("VUID-VkImageViewCreateInfo-pNext-06945", pCreateInfo->image, loc,
                         "pNext chain includes VkImageViewSampleWeightCreateInfoQCOM "
                         "but image not created with VK_IMAGE_USAGE_SAMPLE_WEIGHT_BIT_QCOM.");
    }
    if (!IsIdentitySwizzle(pCreateInfo->components)) {
        skip |=
            LogError("VUID-VkImageViewCreateInfo-pNext-06946", pCreateInfo->image, loc,
                     "pNext chain includes VkImageViewSampleWeightCreateInfoQCOM "
                     "but not all members of pCreateInfo->components have identity swizzle. Here are the actual swizzle values:\n"
                     "r swizzle = %s\n"
                     "g swizzle = %s\n"
                     "b swizzle = %s\n"
                     "a swizzle = %s\n",
                     string_VkComponentSwizzle(pCreateInfo->components.r), string_VkComponentSwizzle(pCreateInfo->components.g),
                     string_VkComponentSwizzle(pCreateInfo->components.b), string_VkComponentSwizzle(pCreateInfo->components.a));
    }

    if (aspect_mask != VK_IMAGE_ASPECT_COLOR_BIT) {
        skip |= LogError("VUID-VkImageViewCreateInfo-pNext-06947", pCreateInfo->image, loc,
                         "pNext chain includes VkImageViewSampleWeightCreateInfoQCOM "
                         "but the subresource range aspect mask (%s) is not VK_IMAGE_ASPECT_COLOR_BIT.",
                         string_VkImageAspectFlags(aspect_mask).c_str());
    }
    if (pCreateInfo->subresourceRange.levelCount != 1) {
        skip |= LogError("VUID-VkImageViewCreateInfo-pNext-06948", pCreateInfo->image, loc,
                         "pNext chain includes VkImageViewSampleWeightCreateInfoQCOM "
                         "but the subresource level count (%" PRIu32 ") is not equal to 1.",
                         pCreateInfo->subresourceRange.levelCount);
    }
    if ((pCreateInfo->viewType != VK_IMAGE_VIEW_TYPE_1D_ARRAY) && (pCreateInfo->viewType != VK_IMAGE_VIEW_TYPE_2D_ARRAY)) {
        skip |= LogError("VUID-VkImageViewCreateInfo-pNext-06949", pCreateInfo->image, loc,
                         "pNext chain includes VkImageViewSampleWeightCreateInfoQCOM "
                         "but the view type (%s) is not VK_IMAGE_VIEW_TYPE_1D_ARRAY or VK_IMAGE_VIEW_TYPE_2D_ARRAY.",
                         string_VkImageViewType(pCreateInfo->viewType));
    }
    if ((pCreateInfo->viewType == VK_IMAGE_VIEW_TYPE_1D_ARRAY) && (image_type != VK_IMAGE_TYPE_1D)) {
        skip |= LogError("VUID-VkImageViewCreateInfo-pNext-06950", pCreateInfo->image, loc,
                         "pNext chain includes VkImageViewSampleWeightCreateInfoQCOM "
                         "and the viewType is VK_IMAGE_VIEW_TYPE_1D_ARRAY but the image type (%s) is not VK_IMAGE_TYPE_1D.",
                         string_VkImageType(image_type));
    }
    if ((pCreateInfo->viewType == VK_IMAGE_VIEW_TYPE_1D_ARRAY) && (layer_count != 2)) {
        skip |=
            LogError("VUID-VkImageViewCreateInfo-pNext-06951", pCreateInfo->image, loc,
                     "pNext chain includes VkImageViewSampleWeightCreateInfoQCOM "
                     "and the view type is VK_IMAGE_VIEW_TYPE_1D_ARRAY but the subresourceRange layerCount (%" PRIu32 ") is not 2.",
                     layer_count);
    }

    const uint32_t filter_width_aligned_to_four = (sample_weight_info->filterSize.width + 3) & ~(3);
    const uint32_t min_image_width =
        sample_weight_info->numPhases * (std::max(filter_width_aligned_to_four, sample_weight_info->filterSize.height));
    if ((pCreateInfo->viewType == VK_IMAGE_VIEW_TYPE_1D_ARRAY) && (image_extent.width < min_image_width)) {
        skip |= LogError("VUID-VkImageViewCreateInfo-pNext-06952", pCreateInfo->image, loc,
                         "pNext chain includes VkImageViewSampleWeightCreateInfoQCOM "
                         "for a VK_IMAGE_VIEW_TYPE_1D_ARRAY with an image created with extent.width (%" PRIu32
                         "), but numPhases (%" PRIu32 "), filterSize.width (%" PRIu32 "), and filterSize.height (%" PRIu32
                         ") requires an image extent.width greater than or equal to (%" PRIu32 ").",
                         image_extent.width, sample_weight_info->numPhases, sample_weight_info->filterSize.width,
                         sample_weight_info->filterSize.height, min_image_width);
    }
    if ((pCreateInfo->viewType == VK_IMAGE_VIEW_TYPE_2D_ARRAY) && (image_type != VK_IMAGE_TYPE_2D)) {
        skip |= LogError("VUID-VkImageViewCreateInfo-pNext-06953", pCreateInfo->image, loc,
                         "pNext chain includes VkImageViewSampleWeightCreateInfoQCOM "
                         "and the view type is VK_IMAGE_VIEW_TYPE_2D_ARRAY but the image created type (%s) is "
                         "not VK_IMAGE_TYPE_2D.",
                         string_VkImageType(image_type));
    }
    if ((pCreateInfo->viewType == VK_IMAGE_VIEW_TYPE_2D_ARRAY) && (layer_count < (sample_weight_info->numPhases))) {
        skip |= LogError("VUID-VkImageViewCreateInfo-pNext-06954", pCreateInfo->image, loc,
                         "pNext chain includes VkImageViewSampleWeightCreateInfoQCOM "
                         "and the view type is VK_IMAGE_VIEW_TYPE_1D_ARRAY but the subresourceRange "
                         "layerCount (%" PRIu32
                         ") is not greater than or equal to "
                         "VkImageViewSampleWeightCreateInfoQCOM::numPhases (%" PRIu32 ").",
                         layer_count, sample_weight_info->numPhases);
    }
    if ((pCreateInfo->viewType == VK_IMAGE_VIEW_TYPE_2D_ARRAY) && (image_extent.width < sample_weight_info->filterSize.width)) {
        skip |= LogError("VUID-VkImageViewCreateInfo-pNext-06955", pCreateInfo->image, loc,
                         "pNext chain includes VkImageViewSampleWeightCreateInfoQCOM "
                         "for a VK_IMAGE_VIEW_TYPE_2D_ARRAY but the created image extent.width (%" PRIu32
                         ") is not greater than or equal to filerSize.width (%" PRIu32 ").",
                         image_extent.width, sample_weight_info->filterSize.width);
    }
    if ((pCreateInfo->viewType == VK_IMAGE_VIEW_TYPE_2D_ARRAY) && (image_extent.width < sample_weight_info->filterSize.height)) {
        skip |= LogError("VUID-VkImageViewCreateInfo-pNext-06956", pCreateInfo->image, loc,
                         "pNext chain includes VkImageViewSampleWeightCreateInfoQCOM "
                         "for a VK_IMAGE_VIEW_TYPE_2D_ARRAY but the created image extent.height (%" PRIu32
                         ") is not greater than or equal to filerSize.height (%" PRIu32 ").",
                         image_extent.height, sample_weight_info->filterSize.height);
    }
    if ((sample_weight_info->filterSize.height > phys_dev_ext_props.image_processing_props.maxWeightFilterDimension.height)) {
        skip |= LogError("VUID-VkImageViewCreateInfo-pNext-06957", pCreateInfo->image, loc,
                         "VkImageViewSampleWeightCreateInfoQCOM::filterSize.height (%" PRIu32
                         ") is not less than or equal to "
                         "VkPhysicalDeviceImageProcessingPropertiesQCOM::maxWeightFilterDimension.height (%" PRIu32 ").",
                         sample_weight_info->filterSize.height,
                         phys_dev_ext_props.image_processing_props.maxWeightFilterDimension.height);
    }

    // Valiate VkImageViewSampleWeightCreateInfoQCOM
    if ((sample_weight_info->filterSize.width > phys_dev_ext_props.image_processing_props.maxWeightFilterDimension.width)) {
        skip |= LogError("VUID-VkImageViewSampleWeightCreateInfoQCOM-filterSize-06958", pCreateInfo->image, loc,
                         "VkImageViewSampleWeightCreateInfoQCOM::filterSize.width (%" PRIu32
                         ") is not less than or equal to "
                         "VkPhysicalDeviceImageProcessingPropertiesQCOM::maxWeightFilterDimension.width (%" PRIu32 ").",
                         sample_weight_info->filterSize.width,
                         phys_dev_ext_props.image_processing_props.maxWeightFilterDimension.width);
    }
    if ((sample_weight_info->filterSize.height > phys_dev_ext_props.image_processing_props.maxWeightFilterDimension.height)) {
        skip |= LogError("VUID-VkImageViewSampleWeightCreateInfoQCOM-filterSize-06959", pCreateInfo->image, loc,
                         "VkImageViewSampleWeightCreateInfoQCOM::filterSize.height (%" PRIu32
                         ") is not less than or equal to "
                         "VkPhysicalDeviceImageProcessingPropertiesQCOM::maxWeightFilterDimension.height (%" PRIu32 ").",
                         sample_weight_info->filterSize.width,
                         phys_dev_ext_props.image_processing_props.maxWeightFilterDimension.width);
    }
    if ((static_cast<uint32_t>(sample_weight_info->filterCenter.x) >= sample_weight_info->filterSize.width)) {
        skip |= LogError("VUID-VkImageViewSampleWeightCreateInfoQCOM-filterCenter-06960", pCreateInfo->image, loc,
                         "VkImageViewSampleWeightCreateInfoQCOM::filterCenter.x (%" PRId32
                         ") is not less than "
                         "VkImageViewSampleWeightCreateInfoQCOM::filterSize.width (%" PRIu32 ").",
                         sample_weight_info->filterCenter.x, sample_weight_info->filterSize.width);
    }
    if ((static_cast<uint32_t>(sample_weight_info->filterCenter.y) >= sample_weight_info->filterSize.height)) {
        skip |= LogError("VUID-VkImageViewSampleWeightCreateInfoQCOM-filterCenter-06961", pCreateInfo->image, loc,
                         "VkImageViewSampleWeightCreateInfoQCOM::filterCenter.y (%" PRId32
                         ") is not less than "
                         "VkImageViewSampleWeightCreateInfoQCOM::filterSize.height (%" PRIu32 ").",
                         sample_weight_info->filterCenter.y, sample_weight_info->filterSize.height);
    }
    if (!IsPowerOfTwo(sample_weight_info->numPhases) || (MostSignificantBit(sample_weight_info->numPhases) % 2 != 0)) {
        skip |= LogError("VUID-VkImageViewSampleWeightCreateInfoQCOM-numPhases-06962", pCreateInfo->image, loc,
                         "VkImageViewSampleWeightCreateInfoQCOM::numPhasaes (%" PRIu32
                         ") is not a power of two squared "
                         "value (i.e., 1, 4, 16, 64, 256, etc.)",
                         sample_weight_info->numPhases);
    }
    if ((sample_weight_info->numPhases > phys_dev_ext_props.image_processing_props.maxWeightFilterPhases)) {
        skip |= LogError("VUID-VkImageViewSampleWeightCreateInfoQCOM-numPhases-06963", pCreateInfo->image, loc,
                         "VkImageViewSampleWeightCreateInfoQCOM::numPhases (%" PRIu32
                         ") is not less than or equal to "
                         "VkPhysicalDeviceImageProcessingPropertiesQCOM::maxWeightFilterPhases (%" PRIu32 ")",
                         sample_weight_info->numPhases, phys_dev_ext_props.image_processing_props.maxWeightFilterPhases);
    }
    return skip;
}
