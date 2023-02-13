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

#include "vk_enum_string_helper.h"
#include "chassis.h"
#include "core_checks/core_validation.h"
#include "core_error_location.h"
#include "sync/sync_vuid_maps.h"

// There is a table in the Vulkan spec to list all formats that implicitly require YCbCr conversion,
// but some features/extensions can explicitly turn that restriction off
// The implicit check is done in format utils, while feature checks are done here in CoreChecks
bool CoreChecks::FormatRequiresYcbcrConversionExplicitly(const VkFormat format) const {
    if (format == VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16 &&
        enabled_features.rgba10x6_formats_features.formatRgba10x6WithoutYCbCrSampler) {
        return false;
    }
    return FormatRequiresYcbcrConversion(format);
}

bool CoreChecks::ValidateImageFormatFeatures(const VkImageCreateInfo *pCreateInfo) const {
    bool skip = false;

    // validates based on imageCreateFormatFeatures from vkspec.html#resources-image-creation-limits
    VkFormatFeatureFlags2KHR tiling_features = 0;
    const VkImageTiling image_tiling = pCreateInfo->tiling;
    const VkFormat image_format = pCreateInfo->format;

    if (image_format == VK_FORMAT_UNDEFINED) {
        // VU 01975 states format can't be undefined unless an android externalFormat
#ifdef VK_USE_PLATFORM_ANDROID_KHR
        const VkExternalFormatANDROID *ext_fmt_android = LvlFindInChain<VkExternalFormatANDROID>(pCreateInfo->pNext);
        if ((image_tiling == VK_IMAGE_TILING_OPTIMAL) && (ext_fmt_android != nullptr) && (0 != ext_fmt_android->externalFormat)) {
            auto it = ahb_ext_formats_map.find(ext_fmt_android->externalFormat);
            if (it != ahb_ext_formats_map.end()) {
                tiling_features = it->second;
            }
        }
#endif
    } else if (image_tiling == VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT) {
        vvl::unordered_set<uint64_t> drm_format_modifiers;
        const VkImageDrmFormatModifierExplicitCreateInfoEXT *drm_explicit =
            LvlFindInChain<VkImageDrmFormatModifierExplicitCreateInfoEXT>(pCreateInfo->pNext);
        const VkImageDrmFormatModifierListCreateInfoEXT *drm_implicit =
            LvlFindInChain<VkImageDrmFormatModifierListCreateInfoEXT>(pCreateInfo->pNext);

        if (drm_explicit != nullptr) {
            drm_format_modifiers.insert(drm_explicit->drmFormatModifier);
        } else {
            // VUID 02261 makes sure its only explict or implict in parameter checking
            assert(drm_implicit != nullptr);
            for (uint32_t i = 0; i < drm_implicit->drmFormatModifierCount; i++) {
                drm_format_modifiers.insert(drm_implicit->pDrmFormatModifiers[i]);
            }
        }

        auto fmt_drm_props = LvlInitStruct<VkDrmFormatModifierPropertiesListEXT>();
        auto fmt_props_2 = LvlInitStruct<VkFormatProperties2>(&fmt_drm_props);
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
    if (FormatIsMultiplane(image_format) && ((pCreateInfo->flags & VK_IMAGE_CREATE_DISJOINT_BIT) != 0) &&
        ((tiling_features & VK_FORMAT_FEATURE_2_DISJOINT_BIT_KHR) == 0)) {
        skip |= LogError(device, "VUID-VkImageCreateInfo-imageCreateFormatFeatures-02260",
                         "vkCreateImage(): can't use VK_IMAGE_CREATE_DISJOINT_BIT because %s doesn't support "
                         "VK_FORMAT_FEATURE_DISJOINT_BIT based on imageCreateFormatFeatures.",
                         string_VkFormat(pCreateInfo->format));
    }

    return skip;
}

bool CoreChecks::PreCallValidateCreateImage(VkDevice device, const VkImageCreateInfo *pCreateInfo,
                                            const VkAllocationCallbacks *pAllocator, VkImage *pImage) const {
    bool skip = false;

    if (IsExtEnabled(device_extensions.vk_android_external_memory_android_hardware_buffer)) {
        skip |= ValidateCreateImageANDROID(report_data, pCreateInfo);
    } else {  // These checks are omitted or replaced when Android HW Buffer extension is active
        if (pCreateInfo->format == VK_FORMAT_UNDEFINED) {
            return LogError(device, "VUID-VkImageCreateInfo-format-00943",
                            "vkCreateImage(): VkFormat for image must not be VK_FORMAT_UNDEFINED.");
        }
    }

    if (pCreateInfo->flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) {
        if (VK_IMAGE_TYPE_2D != pCreateInfo->imageType) {
            skip |= LogError(device, "VUID-VkImageCreateInfo-flags-00949",
                             "vkCreateImage(): Image type must be VK_IMAGE_TYPE_2D when VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT "
                             "flag bit is set");
        }
    }

    const VkPhysicalDeviceLimits *device_limits = &phys_dev_props.limits;
    VkImageUsageFlags attach_flags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
                                     VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    if ((pCreateInfo->usage & attach_flags) && (pCreateInfo->extent.width > device_limits->maxFramebufferWidth)) {
        skip |= LogError(device, "VUID-VkImageCreateInfo-usage-00964",
                         "vkCreateImage(): Image usage flags include a frame buffer attachment bit and image width (%" PRIu32
                         ") exceeds "
                         "device maxFramebufferWidth (%" PRIu32 ").",
                         pCreateInfo->extent.width, device_limits->maxFramebufferWidth);
    }

    if ((pCreateInfo->usage & attach_flags) && (pCreateInfo->extent.height > device_limits->maxFramebufferHeight)) {
        skip |= LogError(device, "VUID-VkImageCreateInfo-usage-00965",
                         "vkCreateImage(): Image usage flags include a frame buffer attachment bit and image height (%" PRIu32
                         ") exceeds "
                         "device maxFramebufferHeight (%" PRIu32 ").",
                         pCreateInfo->extent.height, device_limits->maxFramebufferHeight);
    }

    VkImageCreateFlags sparseFlags =
        VK_IMAGE_CREATE_SPARSE_BINDING_BIT | VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT | VK_IMAGE_CREATE_SPARSE_ALIASED_BIT;
    if ((pCreateInfo->flags & sparseFlags) && (pCreateInfo->usage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT)) {
        skip |= LogError(device, "VUID-VkImageCreateInfo-None-01925",
                         "vkCreateImage(): images using sparse memory cannot have VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT set");
    }

    if (!enabled_features.fragment_density_map_offset_features.fragmentDensityMapOffset) {
        uint32_t ceiling_width = static_cast<uint32_t>(ceilf(
            static_cast<float>(device_limits->maxFramebufferWidth) /
            std::max(static_cast<float>(phys_dev_ext_props.fragment_density_map_props.minFragmentDensityTexelSize.width), 1.0f)));
        if ((pCreateInfo->usage & VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT) && (pCreateInfo->extent.width > ceiling_width)) {
            skip |= LogError(device, "VUID-VkImageCreateInfo-fragmentDensityMapOffset-06514",
                             "vkCreateImage(): Image usage flags include a fragment density map bit and image width (%" PRIu32
                             ") exceeds the "
                             "ceiling of device "
                             "maxFramebufferWidth (%" PRIu32 ") / minFragmentDensityTexelSize.width (%" PRIu32
                             "). The ceiling value: %" PRIu32 "",
                             pCreateInfo->extent.width, device_limits->maxFramebufferWidth,
                             phys_dev_ext_props.fragment_density_map_props.minFragmentDensityTexelSize.width, ceiling_width);
        }

        uint32_t ceiling_height = static_cast<uint32_t>(ceilf(
            static_cast<float>(device_limits->maxFramebufferHeight) /
            std::max(static_cast<float>(phys_dev_ext_props.fragment_density_map_props.minFragmentDensityTexelSize.height), 1.0f)));
        if ((pCreateInfo->usage & VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT) && (pCreateInfo->extent.height > ceiling_height)) {
            skip |= LogError(device, "VUID-VkImageCreateInfo-fragmentDensityMapOffset-06515",
                             "vkCreateImage(): Image usage flags include a fragment density map bit and image height (%" PRIu32
                             ") exceeds the "
                             "ceiling of device "
                             "maxFramebufferHeight (%" PRIu32 ") / minFragmentDensityTexelSize.height (%" PRIu32
                             "). The ceiling value: %" PRIu32 "",
                             pCreateInfo->extent.height, device_limits->maxFramebufferHeight,
                             phys_dev_ext_props.fragment_density_map_props.minFragmentDensityTexelSize.height, ceiling_height);
        }
    }

    VkImageFormatProperties format_limits = {};
    VkResult result = VK_SUCCESS;
    if (pCreateInfo->tiling != VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT) {
        result = DispatchGetPhysicalDeviceImageFormatProperties(physical_device, pCreateInfo->format, pCreateInfo->imageType,
                                                                pCreateInfo->tiling, pCreateInfo->usage, pCreateInfo->flags,
                                                                &format_limits);
    } else {
        auto modifier_list = LvlFindInChain<VkImageDrmFormatModifierListCreateInfoEXT>(pCreateInfo->pNext);
        auto explicit_modifier = LvlFindInChain<VkImageDrmFormatModifierExplicitCreateInfoEXT>(pCreateInfo->pNext);
        if (modifier_list) {
            for (uint32_t i = 0; i < modifier_list->drmFormatModifierCount; i++) {
                auto drm_format_modifier = LvlInitStruct<VkPhysicalDeviceImageDrmFormatModifierInfoEXT>();
                drm_format_modifier.drmFormatModifier = modifier_list->pDrmFormatModifiers[i];
                auto image_format_info = LvlInitStruct<VkPhysicalDeviceImageFormatInfo2>(&drm_format_modifier);
                image_format_info.type = pCreateInfo->imageType;
                image_format_info.format = pCreateInfo->format;
                image_format_info.tiling = pCreateInfo->tiling;
                image_format_info.usage = pCreateInfo->usage;
                image_format_info.flags = pCreateInfo->flags;
                auto image_format_properties = LvlInitStruct<VkImageFormatProperties2>();

                result =
                    DispatchGetPhysicalDeviceImageFormatProperties2(physical_device, &image_format_info, &image_format_properties);
                format_limits = image_format_properties.imageFormatProperties;

                /* The application gives a list of modifier and the driver
                 * selects one. If one is wrong, stop there.
                 */
                if (result != VK_SUCCESS) break;
            }
        } else if (explicit_modifier) {
            auto drm_format_modifier = LvlInitStruct<VkPhysicalDeviceImageDrmFormatModifierInfoEXT>();
            drm_format_modifier.drmFormatModifier = explicit_modifier->drmFormatModifier;
            auto image_format_info = LvlInitStruct<VkPhysicalDeviceImageFormatInfo2>(&drm_format_modifier);
            image_format_info.type = pCreateInfo->imageType;
            image_format_info.format = pCreateInfo->format;
            image_format_info.tiling = pCreateInfo->tiling;
            image_format_info.usage = pCreateInfo->usage;
            image_format_info.flags = pCreateInfo->flags;
            auto image_format_properties = LvlInitStruct<VkImageFormatProperties2>();

            result = DispatchGetPhysicalDeviceImageFormatProperties2(physical_device, &image_format_info, &image_format_properties);
            format_limits = image_format_properties.imageFormatProperties;
        }
    }

    // 1. vkGetPhysicalDeviceImageFormatProperties[2] only success code is VK_SUCCESS
    // 2. If call returns an error, then "imageCreateImageFormatPropertiesList" is defined to be the empty list
    // 3. All values in 02251 are undefined if "imageCreateImageFormatPropertiesList" is empty.
    if (result != VK_SUCCESS) {
        // External memory will always have a "imageCreateImageFormatPropertiesList" so skip
#ifdef VK_USE_PLATFORM_ANDROID_KHR
        if (!LvlFindInChain<VkExternalFormatANDROID>(pCreateInfo->pNext)) {
#endif  // VK_USE_PLATFORM_ANDROID_KHR
            skip |= LogError(device, "VUID-VkImageCreateInfo-imageCreateMaxMipLevels-02251",
                             "vkCreateImage(): Format %s is not supported for this combination of parameters and "
                             "VkGetPhysicalDeviceImageFormatProperties returned back %s.",
                             string_VkFormat(pCreateInfo->format), string_VkResult(result));
#ifdef VK_USE_PLATFORM_ANDROID_KHR
        }
#endif  // VK_USE_PLATFORM_ANDROID_KHR
    } else {
        if (pCreateInfo->mipLevels > format_limits.maxMipLevels) {
            const char *format_string = string_VkFormat(pCreateInfo->format);
            skip |= LogError(device, "VUID-VkImageCreateInfo-mipLevels-02255",
                             "vkCreateImage(): Image mip levels=%d exceed image format maxMipLevels=%d for format %s.",
                             pCreateInfo->mipLevels, format_limits.maxMipLevels, format_string);
        }

        uint64_t texel_count = static_cast<uint64_t>(pCreateInfo->extent.width) *
                               static_cast<uint64_t>(pCreateInfo->extent.height) *
                               static_cast<uint64_t>(pCreateInfo->extent.depth) * static_cast<uint64_t>(pCreateInfo->arrayLayers) *
                               static_cast<uint64_t>(pCreateInfo->samples);

        // Depth/Stencil formats size can't be accurately calculated
        if (!FormatIsDepthAndStencil(pCreateInfo->format)) {
            uint64_t total_size =
                static_cast<uint64_t>(std::ceil(FormatTexelSize(pCreateInfo->format) * static_cast<double>(texel_count)));

            // Round up to imageGranularity boundary
            VkDeviceSize image_granularity = phys_dev_props.limits.bufferImageGranularity;
            uint64_t ig_mask = image_granularity - 1;
            total_size = (total_size + ig_mask) & ~ig_mask;

            if (total_size > format_limits.maxResourceSize) {
                skip |= LogWarning(device, kVUID_Core_Image_InvalidFormatLimitsViolation,
                                   "vkCreateImage(): resource size exceeds allowable maximum Image resource size = 0x%" PRIxLEAST64
                                   ", maximum resource size = 0x%" PRIxLEAST64 " ",
                                   total_size, format_limits.maxResourceSize);
            }
        }

        if (pCreateInfo->arrayLayers > format_limits.maxArrayLayers) {
            skip |= LogError(device, "VUID-VkImageCreateInfo-arrayLayers-02256",
                             "vkCreateImage(): arrayLayers=%d exceeds allowable maximum supported by format of %d.",
                             pCreateInfo->arrayLayers, format_limits.maxArrayLayers);
        }

        if ((pCreateInfo->samples & format_limits.sampleCounts) == 0) {
            skip |= LogError(device, "VUID-VkImageCreateInfo-samples-02258",
                             "vkCreateImage(): samples %s is not supported by format 0x%.8X.",
                             string_VkSampleCountFlagBits(pCreateInfo->samples), format_limits.sampleCounts);
        }

        if (pCreateInfo->extent.width > format_limits.maxExtent.width) {
            skip |= LogError(device, "VUID-VkImageCreateInfo-extent-02252",
                             "vkCreateImage(): extent.width %" PRIu32 " exceeds allowable maximum image extent width %" PRIu32 ".",
                             pCreateInfo->extent.width, format_limits.maxExtent.width);
        }

        if (pCreateInfo->extent.height > format_limits.maxExtent.height) {
            skip |=
                LogError(device, "VUID-VkImageCreateInfo-extent-02253",
                         "vkCreateImage(): extent.height %" PRIu32 " exceeds allowable maximum image extent height %" PRIu32 ".",
                         pCreateInfo->extent.height, format_limits.maxExtent.height);
        }

        if (pCreateInfo->extent.depth > format_limits.maxExtent.depth) {
            skip |= LogError(device, "VUID-VkImageCreateInfo-extent-02254",
                             "vkCreateImage(): extent.depth %" PRIu32 " exceeds allowable maximum image extent depth %" PRIu32 ".",
                             pCreateInfo->extent.depth, format_limits.maxExtent.depth);
        }
    }

    // Tests for "Formats requiring sampler YCBCR conversion for VK_IMAGE_ASPECT_COLOR_BIT image views"
    if (FormatRequiresYcbcrConversionExplicitly(pCreateInfo->format)) {
        if (pCreateInfo->mipLevels != 1) {
            skip |= LogError(device, "VUID-VkImageCreateInfo-format-06410",
                             "vkCreateImage(): mipLevels = %d, but when using a YCbCr Conversion format, mipLevels must be 1",
                             pCreateInfo->mipLevels);
        }

        if (pCreateInfo->samples != VK_SAMPLE_COUNT_1_BIT) {
            skip |= LogError(
                device, "VUID-VkImageCreateInfo-format-06411",
                "vkCreateImage(): samples = %s, but when using a YCbCr Conversion format, samples must be VK_SAMPLE_COUNT_1_BIT",
                string_VkSampleCountFlagBits(pCreateInfo->samples));
        }

        if (pCreateInfo->imageType != VK_IMAGE_TYPE_2D) {
            skip |= LogError(
                device, "VUID-VkImageCreateInfo-format-06412",
                "vkCreateImage(): imageType = %s, but when using a YCbCr Conversion format, imageType must be VK_IMAGE_TYPE_2D ",
                string_VkImageType(pCreateInfo->imageType));
        }
    }

    if (IsExtEnabled(device_extensions.vk_khr_maintenance2)) {
        if (pCreateInfo->flags & VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT) {
            if (!FormatIsCompressed(pCreateInfo->format)) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-flags-01572",
                                 "vkCreateImage(): If pCreateInfo->flags contains VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT, "
                                 "format must be a compressed image format, but is %s",
                                 string_VkFormat(pCreateInfo->format));
            }
            if (!(pCreateInfo->flags & VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT)) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-flags-01573",
                                 "vkCreateImage(): If pCreateInfo->flags contains VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT, "
                                 "flags must also contain VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT.");
            }
        }
    }

    if (pCreateInfo->sharingMode == VK_SHARING_MODE_CONCURRENT && pCreateInfo->pQueueFamilyIndices) {
        const char *vuid = IsExtEnabled(device_extensions.vk_khr_get_physical_device_properties2)
                               ? "VUID-VkImageCreateInfo-sharingMode-01420"
                               : "VUID-VkImageCreateInfo-sharingMode-01392";
        skip |= ValidatePhysicalDeviceQueueFamilies(pCreateInfo->queueFamilyIndexCount, pCreateInfo->pQueueFamilyIndices,
                                                    "vkCreateImage", "pCreateInfo->pQueueFamilyIndices", vuid);
    }

    if (!FormatIsMultiplane(pCreateInfo->format) && !(pCreateInfo->flags & VK_IMAGE_CREATE_ALIAS_BIT) &&
        (pCreateInfo->flags & VK_IMAGE_CREATE_DISJOINT_BIT)) {
        skip |=
            LogError(device, "VUID-VkImageCreateInfo-format-01577",
                     "vkCreateImage(): format is %s and flags are %s. The flags should not include VK_IMAGE_CREATE_DISJOINT_BIT.",
                     string_VkFormat(pCreateInfo->format), string_VkImageCreateFlags(pCreateInfo->flags).c_str());
    }

    const auto swapchain_create_info = LvlFindInChain<VkImageSwapchainCreateInfoKHR>(pCreateInfo->pNext);
    if (swapchain_create_info != nullptr) {
        if (swapchain_create_info->swapchain != VK_NULL_HANDLE) {
            auto swapchain_state = Get<SWAPCHAIN_NODE>(swapchain_create_info->swapchain);
            const VkSwapchainCreateFlagsKHR swapchain_flags = swapchain_state->createInfo.flags;

            // Validate rest of Swapchain Image create check that require swapchain state
            const char *vuid = "VUID-VkImageSwapchainCreateInfoKHR-swapchain-00995";
            if (((swapchain_flags & VK_SWAPCHAIN_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT_KHR) != 0) &&
                ((pCreateInfo->flags & VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT) == 0)) {
                skip |= LogError(
                    device, vuid,
                    "vkCreateImage(): Swapchain was created with VK_SWAPCHAIN_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT_KHR flag so "
                    "all swapchain images must have the VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT flag set.");
            }
            if (((swapchain_flags & VK_SWAPCHAIN_CREATE_PROTECTED_BIT_KHR) != 0) &&
                ((pCreateInfo->flags & VK_IMAGE_CREATE_PROTECTED_BIT) == 0)) {
                skip |= LogError(device, vuid,
                                 "vkCreateImage(): Swapchain was created with VK_SWAPCHAIN_CREATE_PROTECTED_BIT_KHR flag so all "
                                 "swapchain images must have the VK_IMAGE_CREATE_PROTECTED_BIT flag set.");
            }
            const VkImageCreateFlags mutable_flags = (VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT | VK_IMAGE_CREATE_EXTENDED_USAGE_BIT);
            if (((swapchain_flags & VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR) != 0) &&
                ((pCreateInfo->flags & mutable_flags) != mutable_flags)) {
                skip |= LogError(device, vuid,
                                 "vkCreateImage(): Swapchain was created with VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR flag so "
                                 "all swapchain images must have the VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT and "
                                 "VK_IMAGE_CREATE_EXTENDED_USAGE_BIT flags both set.");
            }
        }
    }

    if ((pCreateInfo->flags & VK_IMAGE_CREATE_PROTECTED_BIT) != 0) {
        if (enabled_features.core11.protectedMemory == VK_FALSE) {
            skip |= LogError(device, "VUID-VkImageCreateInfo-flags-01890",
                             "vkCreateImage(): the protectedMemory device feature is disabled: Images cannot be created with the "
                             "VK_IMAGE_CREATE_PROTECTED_BIT set.");
        }
        const VkImageCreateFlags invalid_flags =
            VK_IMAGE_CREATE_SPARSE_BINDING_BIT | VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT | VK_IMAGE_CREATE_SPARSE_ALIASED_BIT;
        if ((pCreateInfo->flags & invalid_flags) != 0) {
            skip |= LogError(device, "VUID-VkImageCreateInfo-None-01891",
                             "vkCreateImage(): VK_IMAGE_CREATE_PROTECTED_BIT is set so no sparse create flags can be used at same "
                             "time (VK_IMAGE_CREATE_SPARSE_BINDING_BIT | VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT | "
                             "VK_IMAGE_CREATE_SPARSE_ALIASED_BIT).");
        }
    }

    if ((pCreateInfo->flags & VK_IMAGE_CREATE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_BIT_EXT) != 0) {
        if (!(enabled_features.multisampled_render_to_single_sampled_features.multisampledRenderToSingleSampled)) {
            skip |= LogError(
                device, "VUID-VkImageCreateInfo-multisampledRenderToSingleSampled-06882",
                "vkCreateImage(): pCreateInfo.flags contains VK_IMAGE_CREATE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_BIT_EXT "
                "but the multisampledRenderToSingleSampled feature is not enabled");
        }
        if (pCreateInfo->samples != VK_SAMPLE_COUNT_1_BIT) {
            skip |= LogError(
                device, "VUID-VkImageCreateInfo-flags-06883",
                "vkCreateImage(): pCreateInfo.flags contains VK_IMAGE_CREATE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_BIT_EXT "
                "but samples (%s) is not equal to VK_SAMPLE_COUNT_1_BIT",
                string_VkSampleCountFlagBits(pCreateInfo->samples));
        }
    }

    skip |= ValidateImageFormatFeatures(pCreateInfo);

    // Check compatibility with VK_KHR_portability_subset
    if (IsExtEnabled(device_extensions.vk_khr_portability_subset)) {
        if (VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT & pCreateInfo->flags &&
            VK_FALSE == enabled_features.portability_subset_features.imageView2DOn3DImage) {
            skip |= LogError(device, "VUID-VkImageCreateInfo-imageView2DOn3DImage-04459",
                             "vkCreateImage (portability error): VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT is not supported.");
        }
        if ((VK_SAMPLE_COUNT_1_BIT != pCreateInfo->samples) && (1 != pCreateInfo->arrayLayers) &&
            (VK_FALSE == enabled_features.portability_subset_features.multisampleArrayImage)) {
            skip |=
                LogError(device, "VUID-VkImageCreateInfo-multisampleArrayImage-04460",
                         "vkCreateImage (portability error): Cannot create an image with samples/texel > 1 && arrayLayers != 1");
        }
    }

    const auto external_memory_create_info_nv = LvlFindInChain<VkExternalMemoryImageCreateInfoNV>(pCreateInfo->pNext);
    const auto external_memory_create_info = LvlFindInChain<VkExternalMemoryImageCreateInfo>(pCreateInfo->pNext);
    if (external_memory_create_info_nv != nullptr && external_memory_create_info != nullptr) {
        skip |= LogError(device, "VUID-VkImageCreateInfo-pNext-00988",
                         "vkCreateImage(): VkImageCreateInfo struct has both VkExternalMemoryImageCreateInfoNV and "
                         "VkExternalMemoryImageCreateInfo chained structs.");
    }
    if (external_memory_create_info && external_memory_create_info->handleTypes != 0) {
        if (pCreateInfo->initialLayout != VK_IMAGE_LAYOUT_UNDEFINED) {
            skip |= LogError(
                device, "VUID-VkImageCreateInfo-pNext-01443",
                "vkCreateImage: VkImageCreateInfo pNext chain includes VkExternalMemoryImageCreateInfo with handleTypes %" PRIu32
                " but pCreateInfo->initialLayout is %s.",
                external_memory_create_info->handleTypes, string_VkImageLayout(pCreateInfo->initialLayout));
        }
        // Check external memory handle types compatibility
        const uint32_t any_type = 1u << MostSignificantBit(external_memory_create_info->handleTypes);
        auto external_image_info = LvlInitStruct<VkPhysicalDeviceExternalImageFormatInfo>();
        external_image_info.handleType = static_cast<VkExternalMemoryHandleTypeFlagBits>(any_type);
        auto image_info = LvlInitStruct<VkPhysicalDeviceImageFormatInfo2>(&external_image_info);
        image_info.format = pCreateInfo->format;
        image_info.type = pCreateInfo->imageType;
        image_info.tiling = pCreateInfo->tiling;
        image_info.usage = pCreateInfo->usage;
        image_info.flags = pCreateInfo->flags;

        auto external_image_properties = LvlInitStruct<VkExternalImageFormatProperties>();
        auto image_properties = LvlInitStruct<VkImageFormatProperties2>(&external_image_properties);
        result = DispatchGetPhysicalDeviceImageFormatProperties2(physical_device, &image_info, &image_properties);
        const auto compatible_types = external_image_properties.externalMemoryProperties.compatibleHandleTypes;

        if (result != VK_SUCCESS) {
            skip |= LogError(
                device, "VUID-VkImageCreateInfo-pNext-00990",
                "vkCreateImage(): The handle type (%s), format (%s), type (%s), tiling (%s), usage (%s), flags (%s) "
                "is not supported combination of parameters and vkGetPhysicalDeviceImageFormatProperties2 returned back %s.",
                string_VkExternalMemoryHandleTypeFlagBits(external_image_info.handleType), string_VkFormat(image_info.format),
                string_VkImageType(image_info.type), string_VkImageTiling(image_info.tiling),
                string_VkImageUsageFlags(image_info.usage).c_str(), string_VkImageCreateFlags(image_info.flags).c_str(),
                string_VkResult(result));
        } else if ((external_memory_create_info->handleTypes & compatible_types) != external_memory_create_info->handleTypes) {
            skip |=
                LogError(device, "VUID-VkImageCreateInfo-pNext-00990",
                         "vkCreateImage(): VkImageCreateInfo pNext chain contains VkExternalMemoryImageCreateInfo with handleTypes "
                         "flags (%s)  that are not reported as compatible by vkGetPhysicalDeviceImageFormatProperties2.",
                         string_VkExternalMemoryHandleTypeFlags(external_memory_create_info->handleTypes).c_str());
        }
    } else if (external_memory_create_info_nv && external_memory_create_info_nv->handleTypes != 0) {
        if (pCreateInfo->initialLayout != VK_IMAGE_LAYOUT_UNDEFINED) {
            skip |= LogError(
                device, "VUID-VkImageCreateInfo-pNext-01443",
                "vkCreateImage: VkImageCreateInfo pNext chain includes VkExternalMemoryImageCreateInfoNV with handleTypes %" PRIu32
                " but pCreateInfo->initialLayout is %s.",
                external_memory_create_info_nv->handleTypes, string_VkImageLayout(pCreateInfo->initialLayout));
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
            skip |= LogError(device, "VUID-VkImageCreateInfo-pNext-00991",
                             "vkCreateImage(): The handle type (%s), format (%s), type (%s), tiling (%s), usage (%s), flags (%s) "
                             "is not supported combination of parameters and vkGetPhysicalDeviceExternalImageFormatPropertiesNV "
                             "returned back %s.",
                             string_VkExternalMemoryHandleTypeFlagBitsNV(handle_type), string_VkFormat(pCreateInfo->format),
                             string_VkImageType(pCreateInfo->imageType), string_VkImageTiling(pCreateInfo->tiling),
                             string_VkImageUsageFlags(pCreateInfo->usage).c_str(),
                             string_VkImageCreateFlags(pCreateInfo->flags).c_str(), string_VkResult(result));
        } else if ((external_memory_create_info_nv->handleTypes & compatible_types) !=
                   external_memory_create_info_nv->handleTypes) {
            skip |= LogError(
                device, "VUID-VkImageCreateInfo-pNext-00991",
                "vkCreateImage(): VkImageCreateInfo pNext chain contains VkExternalMemoryImageCreateInfoNV with handleTypes flags "
                "(%s) that are not reported as compatible by vkGetPhysicalDeviceExternalImageFormatPropertiesNV.",
                string_VkExternalMemoryHandleTypeFlagsNV(external_memory_create_info_nv->handleTypes).c_str());
        }
    }

    if (device_group_create_info.physicalDeviceCount == 1) {
        if (pCreateInfo->flags & VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT) {
            skip |= LogError(
                device, "VUID-VkImageCreateInfo-physicalDeviceCount-01421",
                "vkCreateImage: Device was created with VkDeviceGroupDeviceCreateInfo::physicalDeviceCount equal to 1, but "
                "flags contain VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT bit. Device creation with "
                "VkDeviceGroupDeviceCreateInfo::physicalDeviceCount equal to 1 may have been implicit.");
        }
    }

    if ((pCreateInfo->flags & VK_IMAGE_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT) &&
        !enabled_features.descriptor_buffer_features.descriptorBufferCaptureReplay) {
        skip |=
            LogError(device, "VUID-VkImageCreateInfo-flags-08104",
                     "vkCreateImage(): the descriptorBufferCaptureReplay device feature is disabled: Images cannot be created with "
                     "the VK_IMAGE_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT.");
    }

    auto opaque_capture_descriptor_buffer = LvlFindInChain<VkOpaqueCaptureDescriptorDataCreateInfoEXT>(pCreateInfo->pNext);
    if (opaque_capture_descriptor_buffer && !(pCreateInfo->flags & VK_IMAGE_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT)) {
        skip |= LogError(device, "VUID-VkImageCreateInfo-pNext-08105",
                         "vkCreateImage(): VkOpaqueCaptureDescriptorDataCreateInfoEXT is in pNext chain, but "
                         "VK_IMAGE_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT is not set.");
    }

    bool has_decode_usage =
        pCreateInfo->usage & (VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR | VK_IMAGE_USAGE_VIDEO_DECODE_SRC_BIT_KHR |
                              VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR);
    bool has_encode_usage =
        pCreateInfo->usage & (VK_IMAGE_USAGE_VIDEO_ENCODE_DST_BIT_KHR | VK_IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT_KHR |
                              VK_IMAGE_USAGE_VIDEO_ENCODE_DPB_BIT_KHR);
    if (has_decode_usage || has_encode_usage) {
        const auto *video_profiles = LvlFindInChain<VkVideoProfileListInfoKHR>(pCreateInfo->pNext);
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
                    LogError(device, "VUID-VkImageCreateInfo-pNext-06811",
                             "vkCreateImage: image creation parameters (flags: 0x%08x, format: %s, imageType: %s, "
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
                                           const VkAllocationCallbacks *pAllocator, VkImage *pImage, VkResult result) {
    if (VK_SUCCESS != result) return;

    StateTracker::PostCallRecordCreateImage(device, pCreateInfo, pAllocator, pImage, result);
    if ((pCreateInfo->flags & VK_IMAGE_CREATE_SPARSE_BINDING_BIT) != 0) {
        // non-sparse images set up their layout maps when memory is bound
        auto image_state = Get<IMAGE_STATE>(*pImage);
        image_state->SetInitialLayoutMap();
    }
}

bool CoreChecks::PreCallValidateDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks *pAllocator) const {
    auto image_state = Get<IMAGE_STATE>(image);
    bool skip = false;
    if (image_state) {
        if (image_state->IsSwapchainImage() && image_state->owned_by_swapchain) {
            skip |= LogError(device, "VUID-vkDestroyImage-image-04882",
                             "vkDestroyImage(): %s is a presentable image and it is controlled by the implementation and is "
                             "destroyed with vkDestroySwapchainKHR.",
                             report_data->FormatHandle(image_state->image()).c_str());
        }
        skip |= ValidateObjectNotInUse(image_state.get(), "vkDestroyImage", "VUID-vkDestroyImage-image-01000");
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

bool CoreChecks::ValidateImageAttributes(const IMAGE_STATE *image_state, const VkImageSubresourceRange &range,
                                         const char *param_name) const {
    bool skip = false;
    const VkImage image = image_state->image();
    const VkFormat format = image_state->createInfo.format;

    if (range.aspectMask != VK_IMAGE_ASPECT_COLOR_BIT) {
        skip |= LogError(image, "VUID-vkCmdClearColorImage-aspectMask-02498",
                         "vkCmdClearColorImage(): %s.aspectMasks must only be set to VK_IMAGE_ASPECT_COLOR_BIT.", param_name);
    }

    if (FormatIsDepthOrStencil(format)) {
        skip |= LogError(image, "VUID-vkCmdClearColorImage-image-00007",
                         "vkCmdClearColorImage(): %s called with image %s which has a depth/stencil format (%s).", param_name,
                         report_data->FormatHandle(image).c_str(), string_VkFormat(format));
    } else if (FormatIsCompressed(format)) {
        skip |= LogError(image, "VUID-vkCmdClearColorImage-image-00007",
                         "vkCmdClearColorImage(): %s called with image %s which has a compressed format (%s).", param_name,
                         report_data->FormatHandle(image).c_str(), string_VkFormat(format));
    }

    if (!(image_state->createInfo.usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT)) {
        skip |=
            LogError(image, "VUID-vkCmdClearColorImage-image-00002",
                     "vkCmdClearColorImage() %s called with image %s which was created without VK_IMAGE_USAGE_TRANSFER_DST_BIT.",
                     param_name, report_data->FormatHandle(image).c_str());
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                   const VkClearColorValue *pColor, uint32_t rangeCount,
                                                   const VkImageSubresourceRange *pRanges) const {
    bool skip = false;
    // TODO : Verify memory is in VK_IMAGE_STATE_CLEAR state
    auto cb_state_ptr = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    auto image_state = Get<IMAGE_STATE>(image);
    if (cb_state_ptr && image_state) {
        const CMD_BUFFER_STATE &cb_state = *cb_state_ptr;
        skip |= ValidateMemoryIsBoundToImage(commandBuffer, *image_state, "vkCmdClearColorImage()",
                                             "VUID-vkCmdClearColorImage-image-00003");
        skip |= ValidateCmd(cb_state, CMD_CLEARCOLORIMAGE);
        if (IsExtEnabled(device_extensions.vk_khr_maintenance1)) {
            skip |= ValidateImageFormatFeatureFlags(commandBuffer, *image_state, VK_FORMAT_FEATURE_2_TRANSFER_DST_BIT_KHR,
                                                    "vkCmdClearColorImage", "VUID-vkCmdClearColorImage-image-01993");
        }
        skip |= ValidateProtectedImage(cb_state, *image_state, "vkCmdClearColorImage()",
                                       "VUID-vkCmdClearColorImage-commandBuffer-01805");
        skip |= ValidateUnprotectedImage(cb_state, *image_state, "vkCmdClearColorImage()",
                                         "VUID-vkCmdClearColorImage-commandBuffer-01806");
        for (uint32_t i = 0; i < rangeCount; ++i) {
            std::string param_name = "pRanges[" + std::to_string(i) + "]";
            skip |= ValidateCmdClearColorSubresourceRange(image_state.get(), pRanges[i], param_name.c_str());
            skip |= ValidateImageAttributes(image_state.get(), pRanges[i], param_name.c_str());
            skip |= VerifyClearImageLayout(cb_state, image_state.get(), pRanges[i], imageLayout, "vkCmdClearColorImage()");
        }
        // Tests for "Formats requiring sampler Y’CBCR conversion for VK_IMAGE_ASPECT_COLOR_BIT image views"
        if (FormatRequiresYcbcrConversionExplicitly(image_state->createInfo.format)) {
            skip |= LogError(device, "VUID-vkCmdClearColorImage-image-01545",
                             "vkCmdClearColorImage(): format (%s) must not be one of the formats requiring sampler YCBCR "
                             "conversion for VK_IMAGE_ASPECT_COLOR_BIT image views",
                             string_VkFormat(image_state->createInfo.format));
        }
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
                                                const char *apiName) const {
    bool skip = false;

    // The extension was not created with a feature bit whichs prevents displaying the 2 variations of the VUIDs
    if (!IsExtEnabled(device_extensions.vk_ext_depth_range_unrestricted)) {
        if (!(clearValue.depth >= 0.0) || !(clearValue.depth <= 1.0)) {
            // Also VUID-VkClearDepthStencilValue-depth-00022
            skip |= LogError(commandBuffer, "VUID-VkClearDepthStencilValue-depth-02506",
                             "%s: VK_EXT_depth_range_unrestricted extension is not enabled and VkClearDepthStencilValue::depth "
                             "(=%f) is not within the [0.0, 1.0] range.",
                             apiName, clearValue.depth);
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                          const VkClearDepthStencilValue *pDepthStencil, uint32_t rangeCount,
                                                          const VkImageSubresourceRange *pRanges) const {
    bool skip = false;

    // TODO : Verify memory is in VK_IMAGE_STATE_CLEAR state
    auto cb_state_ptr = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    auto image_state = Get<IMAGE_STATE>(image);
    if (cb_state_ptr && image_state) {
        const CMD_BUFFER_STATE &cb_state = *cb_state_ptr;
        const VkFormat image_format = image_state->createInfo.format;
        skip |= ValidateMemoryIsBoundToImage(commandBuffer, *image_state, "vkCmdClearDepthStencilImage()",
                                             "VUID-vkCmdClearDepthStencilImage-image-00010");
        skip |= ValidateCmd(cb_state, CMD_CLEARDEPTHSTENCILIMAGE);
        if (IsExtEnabled(device_extensions.vk_khr_maintenance1)) {
            skip |= ValidateImageFormatFeatureFlags(commandBuffer, *image_state, VK_FORMAT_FEATURE_2_TRANSFER_DST_BIT_KHR,
                                                    "vkCmdClearDepthStencilImage", "VUID-vkCmdClearDepthStencilImage-image-01994");
        }
        skip |= ValidateClearDepthStencilValue(commandBuffer, *pDepthStencil, "vkCmdClearDepthStencilImage()");
        skip |= ValidateProtectedImage(cb_state, *image_state, "vkCmdClearDepthStencilImage()",
                                       "VUID-vkCmdClearDepthStencilImage-commandBuffer-01807");
        skip |= ValidateUnprotectedImage(cb_state, *image_state, "vkCmdClearDepthStencilImage()",
                                         "VUID-vkCmdClearDepthStencilImage-commandBuffer-01808");

        bool any_include_aspect_depth_bit = false;
        bool any_include_aspect_stencil_bit = false;

        for (uint32_t i = 0; i < rangeCount; ++i) {
            std::string param_name = "pRanges[" + std::to_string(i) + "]";
            skip |= ValidateCmdClearDepthSubresourceRange(image_state.get(), pRanges[i], param_name.c_str());
            skip |= VerifyClearImageLayout(cb_state, image_state.get(), pRanges[i], imageLayout, "vkCmdClearDepthStencilImage()");
            // Image aspect must be depth or stencil or both
            VkImageAspectFlags valid_aspects = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
            if (((pRanges[i].aspectMask & valid_aspects) == 0) || ((pRanges[i].aspectMask & ~valid_aspects) != 0)) {
                skip |= LogError(commandBuffer, "VUID-vkCmdClearDepthStencilImage-aspectMask-02824",
                                 "vkCmdClearDepthStencilImage(): pRanges[%" PRIu32
                                 "].aspectMask can only be VK_IMAGE_ASPECT_DEPTH_BIT "
                                 "and/or VK_IMAGE_ASPECT_STENCIL_BIT.",
                                 i);
            }
            if ((pRanges[i].aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT) != 0) {
                any_include_aspect_depth_bit = true;
                if (FormatHasDepth(image_format) == false) {
                    skip |= LogError(commandBuffer, "VUID-vkCmdClearDepthStencilImage-image-02826",
                                     "vkCmdClearDepthStencilImage(): pRanges[%" PRIu32
                                     "].aspectMask has a VK_IMAGE_ASPECT_DEPTH_BIT but %s "
                                     "doesn't have a depth component.",
                                     i, string_VkFormat(image_format));
                }
            }
            if ((pRanges[i].aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT) != 0) {
                any_include_aspect_stencil_bit = true;
                if (FormatHasStencil(image_format) == false) {
                    skip |= LogError(commandBuffer, "VUID-vkCmdClearDepthStencilImage-image-02825",
                                     "vkCmdClearDepthStencilImage(): pRanges[%" PRIu32
                                     "].aspectMask has a VK_IMAGE_ASPECT_STENCIL_BIT but "
                                     "%s doesn't have a stencil component.",
                                     i, string_VkFormat(image_format));
                }
            }
        }
        if (any_include_aspect_stencil_bit) {
            const auto image_stencil_struct = LvlFindInChain<VkImageStencilUsageCreateInfo>(image_state->createInfo.pNext);
            if (image_stencil_struct != nullptr) {
                if ((image_stencil_struct->stencilUsage & VK_IMAGE_USAGE_TRANSFER_DST_BIT) == 0) {
                    skip |=
                        LogError(device, "VUID-vkCmdClearDepthStencilImage-pRanges-02658",
                                 "vkCmdClearDepthStencilImage(): an element of pRanges.aspect includes VK_IMAGE_ASPECT_STENCIL_BIT "
                                 "and image was created with separate stencil usage, VK_IMAGE_USAGE_TRANSFER_DST_BIT must be "
                                 "included in VkImageStencilUsageCreateInfo::stencilUsage used to create image");
                }
            } else {
                if ((image_state->createInfo.usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT) == 0) {
                    skip |= LogError(
                        device, "VUID-vkCmdClearDepthStencilImage-pRanges-02659",
                        "vkCmdClearDepthStencilImage(): an element of pRanges.aspect includes VK_IMAGE_ASPECT_STENCIL_BIT and "
                        "image was not created with separate stencil usage, VK_IMAGE_USAGE_TRANSFER_DST_BIT must be included "
                        "in VkImageCreateInfo::usage used to create image");
                }
            }
        }
        if (any_include_aspect_depth_bit && (image_state->createInfo.usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT) == 0) {
            skip |= LogError(device, "VUID-vkCmdClearDepthStencilImage-pRanges-02660",
                             "vkCmdClearDepthStencilImage(): an element of pRanges.aspect includes VK_IMAGE_ASPECT_DEPTH_BIT, "
                             "VK_IMAGE_USAGE_TRANSFER_DST_BIT must be included in VkImageCreateInfo::usage used to create image");
        }
        if (image_state && !FormatIsDepthOrStencil(image_format)) {
            skip |= LogError(image, "VUID-vkCmdClearDepthStencilImage-image-00014",
                             "vkCmdClearDepthStencilImage(): called with image %s which doesn't have a depth/stencil format (%s).",
                             report_data->FormatHandle(image).c_str(), string_VkFormat(image_format));
        }
        if (VK_IMAGE_USAGE_TRANSFER_DST_BIT != (VK_IMAGE_USAGE_TRANSFER_DST_BIT & image_state->createInfo.usage)) {
            skip |= LogError(image, "VUID-vkCmdClearDepthStencilImage-image-00009",
                             "vkCmdClearDepthStencilImage(): called with image %s which was not created with the "
                             "VK_IMAGE_USAGE_TRANSFER_DST_BIT set.",
                             report_data->FormatHandle(image).c_str());
        }
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

bool CoreChecks::ValidateClearAttachmentExtent(const CMD_BUFFER_STATE &cb_state, uint32_t attachment_index,
                                               const IMAGE_VIEW_STATE *image_view_state, const VkRect2D &render_area,
                                               uint32_t rect_count, const VkClearRect *clear_rects) const {
    bool skip = false;

    for (uint32_t j = 0; j < rect_count; j++) {
        if (!ContainsRect(render_area, clear_rects[j].rect)) {
            skip |= LogError(cb_state.Handle(), "VUID-vkCmdClearAttachments-pRects-00016",
                             "vkCmdClearAttachments(): The area defined by pRects[%d] is not contained in the area of "
                             "the current render pass instance.",
                             j);
        }

        if (image_view_state) {
            // The layers specified by a given element of pRects must be contained within every attachment that
            // pAttachments refers to
            const uint32_t attachment_layer_count = image_view_state->GetAttachmentLayerCount();
            if ((clear_rects[j].baseArrayLayer >= attachment_layer_count) ||
                (clear_rects[j].baseArrayLayer + clear_rects[j].layerCount > attachment_layer_count)) {
                skip |= LogError(cb_state.Handle(), "VUID-vkCmdClearAttachments-pRects-06937",
                                 "vkCmdClearAttachments(): The layers defined in pRects[%d] are not contained in the layers "
                                 "of pAttachment[%d].",
                                 j, attachment_index);
            }
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                                    const VkClearAttachment *pAttachments, uint32_t rectCount,
                                                    const VkClearRect *pRects) const {
    bool skip = false;
    auto cb_state_ptr = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    if (!cb_state_ptr) {
        return skip;
    }
    const CMD_BUFFER_STATE &cb_state = *cb_state_ptr;

    skip |= ValidateCmd(cb_state, CMD_CLEARATTACHMENTS);

    // Validate that attachments are in reference list of active subpass
    if (cb_state.activeRenderPass) {
        const auto &render_area = (cb_state.activeRenderPass->use_dynamic_rendering)
                                      ? cb_state.activeRenderPass->dynamic_rendering_begin_rendering_info.renderArea
                                      : cb_state.activeRenderPassBeginInfo.renderArea;

        for (uint32_t attachment_index = 0; attachment_index < attachmentCount; attachment_index++) {
            auto clear_desc = &pAttachments[attachment_index];

            const VkImageAspectFlags aspect_mask = clear_desc->aspectMask;

            bool is_valid_color_attachment_index = false;
            const IMAGE_VIEW_STATE *color_view_state = nullptr;
            uint32_t color_attachment_count = 0;

            bool has_valid_depth_attachment = false;
            const IMAGE_VIEW_STATE *depth_view_state = nullptr;

            bool has_valid_stencil_attachment = false;
            const IMAGE_VIEW_STATE *stencil_view_state = nullptr;

            uint32_t view_mask = 0;

            std::string renderpass_info;

            if (cb_state.activeRenderPass->UsesDynamicRendering()) {
                is_valid_color_attachment_index = cb_state.IsValidDynamicColorAttachmentImageIndex(clear_desc->colorAttachment);
                color_view_state = cb_state.GetActiveAttachmentImageViewState(
                    cb_state.GetDynamicColorAttachmentImageIndex(clear_desc->colorAttachment));
                color_attachment_count = cb_state.GetDynamicColorAttachmentCount();

                has_valid_depth_attachment = cb_state.HasValidDynamicDepthAttachment();
                depth_view_state = cb_state.GetActiveAttachmentImageViewState(cb_state.GetDynamicDepthAttachmentImageIndex());

                has_valid_stencil_attachment = cb_state.HasValidDynamicStencilAttachment();
                stencil_view_state = cb_state.GetActiveAttachmentImageViewState(cb_state.GetDynamicStencilAttachmentImageIndex());

                view_mask = cb_state.activeRenderPass->dynamic_rendering_begin_rendering_info.viewMask;
            } else {
                const auto *renderpass_create_info = cb_state.activeRenderPass->createInfo.ptr();
                const auto *subpass_desc = &renderpass_create_info->pSubpasses[cb_state.activeSubpass];
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
                        }
                    }

                    color_attachment_count = subpass_desc->colorAttachmentCount;

                    has_valid_depth_attachment =
                        (subpass_desc->pDepthStencilAttachment != nullptr) &&
                        ((subpass_desc->pDepthStencilAttachment->attachment == VK_ATTACHMENT_UNUSED) ||
                         (subpass_desc->pDepthStencilAttachment->attachment < renderpass_create_info->attachmentCount));
                    has_valid_stencil_attachment = has_valid_depth_attachment;
                    if (subpass_desc->pDepthStencilAttachment) {
                        depth_view_state =
                            cb_state.GetActiveAttachmentImageViewState(subpass_desc->pDepthStencilAttachment->attachment);
                    }

                    stencil_view_state = depth_view_state;

                    view_mask = subpass_desc->viewMask;
                }

                renderpass_info = " for " + report_data->FormatHandle(cb_state.activeRenderPass->renderPass()) + " subpass " +
                                  std::to_string(cb_state.activeSubpass);
            }

            if (aspect_mask & VK_IMAGE_ASPECT_METADATA_BIT) {
                skip |= LogError(commandBuffer, "VUID-VkClearAttachment-aspectMask-00020",
                                 "vkCmdClearAttachments() pAttachments[%" PRIu32 "] mask contains VK_IMAGE_ASPECT_METADATA_BIT.",
                                 attachment_index);
            } else if (aspect_mask & (VK_IMAGE_ASPECT_MEMORY_PLANE_0_BIT_EXT | VK_IMAGE_ASPECT_MEMORY_PLANE_1_BIT_EXT |
                                      VK_IMAGE_ASPECT_MEMORY_PLANE_2_BIT_EXT | VK_IMAGE_ASPECT_MEMORY_PLANE_3_BIT_EXT)) {
                skip |= LogError(commandBuffer, "VUID-VkClearAttachment-aspectMask-02246",
                                 "vkCmdClearAttachments() pAttachments[%" PRIu32
                                 "] mask contains a VK_IMAGE_ASPECT_MEMORY_PLANE_*_BIT_EXT bit.",
                                 attachment_index);
            } else if (aspect_mask & VK_IMAGE_ASPECT_COLOR_BIT) {
                if (is_valid_color_attachment_index) {
                    if (color_view_state &&
                        !(color_view_state->safe_create_info.subresourceRange.aspectMask & VK_IMAGE_ASPECT_COLOR_BIT)) {
                        skip |= LogError(commandBuffer, "VUID-vkCmdClearAttachments-pAttachments-07270",
                                         "vkCmdClearAttachments() pAttachments[%" PRIu32 "].colorAttachment=%" PRIu32
                                         " is backed by an "
                                         "image view that does not have aspect VK_IMAGE_ASPECT_COLOR_BIT%s.",
                                         attachment_index, clear_desc->colorAttachment, renderpass_info.c_str());
                    }
                } else {
                    skip |= LogError(commandBuffer, "VUID-vkCmdClearAttachments-aspectMask-07271",
                                     "vkCmdClearAttachments() pAttachments[%" PRIu32 "].colorAttachment=%" PRIu32
                                     " out of range%s."
                                     " colorAttachmentCount=%" PRIu32 ".",
                                     attachment_index, clear_desc->colorAttachment,

                                     renderpass_info.c_str(), color_attachment_count);
                }

                if ((aspect_mask & VK_IMAGE_ASPECT_DEPTH_BIT) || (aspect_mask & VK_IMAGE_ASPECT_STENCIL_BIT)) {
                    skip |= LogError(commandBuffer, "VUID-VkClearAttachment-aspectMask-00019",
                                     "vkCmdClearAttachments() pAttachments[%" PRIu32
                                     "] aspectMask must set only VK_IMAGE_ASPECT_COLOR_BIT "
                                     "of a color attachment.",
                                     attachment_index);
                }

            } else {  // Must be depth and/or stencil
                if (aspect_mask & VK_IMAGE_ASPECT_DEPTH_BIT) {
                    if (has_valid_depth_attachment) {
                        if (depth_view_state &&
                            !(depth_view_state->safe_create_info.subresourceRange.aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT)) {
                            skip |= LogError(commandBuffer, "VUID-vkCmdClearAttachments-pAttachments-07270",
                                             "vkCmdClearAttachments() pAttachments[%" PRIu32
                                             "] is backed by an "
                                             "image view that does not have aspect VK_IMAGE_ASPECT_DEPTH_BIT%s.",
                                             attachment_index, renderpass_info.c_str());
                        }
                    } else {
                        skip |= LogError(commandBuffer, "VUID-vkCmdClearAttachments-pAttachments-07270",
                                         "vkCmdClearAttachments() pAttachments[%" PRIu32
                                         "] does not refer to a valid depth attachment%s.",
                                         attachment_index, renderpass_info.c_str());
                    }
                    skip |= ValidateClearDepthStencilValue(commandBuffer, clear_desc->clearValue.depthStencil,
                                                           "vkCmdClearAttachments()");
                }

                if (aspect_mask & VK_IMAGE_ASPECT_STENCIL_BIT) {
                    if (has_valid_stencil_attachment) {
                        if (stencil_view_state &&
                            !(stencil_view_state->safe_create_info.subresourceRange.aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT)) {
                            skip |= LogError(commandBuffer, "VUID-vkCmdClearAttachments-pAttachments-07270",
                                             "vkCmdClearAttachments() pAttachments[%" PRIu32
                                             "] is backed by an "
                                             "image view that does not have aspect VK_IMAGE_ASPECT_STENCIL_BIT%s.",
                                             attachment_index, renderpass_info.c_str());
                        }
                    } else {
                        skip |= LogError(commandBuffer, "VUID-vkCmdClearAttachments-pAttachments-07270",
                                         "vkCmdClearAttachments() pAttachments[%" PRIu32
                                         "] does not refer to a valid stencil attachment%s.",
                                         attachment_index, renderpass_info.c_str());
                    }
                }
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

            if (cb_state.createInfo.level == VK_COMMAND_BUFFER_LEVEL_PRIMARY) {
                for (auto image_view : image_views) {
                    if (image_view) {
                        skip |=
                            ValidateClearAttachmentExtent(cb_state, attachment_index, image_view, render_area, rectCount, pRects);
                    }
                }
            }

            for (auto image_view : image_views) {
                if (image_view) {
                    skip |= ValidateProtectedImage(cb_state, *image_view->image_state, "vkCmdClearAttachments()",
                                                   "VUID-vkCmdClearAttachments-commandBuffer-02504");
                    skip |= ValidateUnprotectedImage(cb_state, *image_view->image_state, "vkCmdClearAttachments()",
                                                     "VUID-vkCmdClearAttachments-commandBuffer-02505");
                }
            }

            // With a non-zero view mask, multiview functionality is considered to be enabled
            if (view_mask > 0) {
                for (uint32_t i = 0; i < rectCount; ++i) {
                    if (pRects[i].baseArrayLayer != 0 || pRects[i].layerCount != 1) {
                        skip |= LogError(commandBuffer, "VUID-vkCmdClearAttachments-baseArrayLayer-00018",
                                         "vkCmdClearAttachments(): pRects[%" PRIu32 "] baseArrayLayer is %" PRIu32
                                         " and layerCount is %" PRIu32 ", but the render pass instance uses multiview.",
                                         i, pRects[i].baseArrayLayer, pRects[i].layerCount);
                    }
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
    if (cb_state.activeRenderPass && (cb_state.createInfo.level == VK_COMMAND_BUFFER_LEVEL_SECONDARY)) {
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
                    auto val_fn = [this, attachment_index, image_index, rectCount, clear_rect_copy](
                                      const CMD_BUFFER_STATE &secondary, const CMD_BUFFER_STATE *prim_cb,
                                      const FRAMEBUFFER_STATE *fb) {
                        assert(rectCount == clear_rect_copy->size());
                        bool skip = false;
                        const IMAGE_VIEW_STATE *image_view_state = nullptr;
                        if (image_index != -1) {
                            image_view_state = (*prim_cb->active_attachments)[image_index];
                        }
                        skip = ValidateClearAttachmentExtent(
                            secondary, attachment_index, image_view_state,
                            prim_cb->activeRenderPass->dynamic_rendering_begin_rendering_info.renderArea, rectCount,
                            clear_rect_copy->data());
                        return skip;
                    };
                    cb_state_ptr->cmd_execute_commands_functions.emplace_back(val_fn);
                }
            }
        } else if (cb_state.activeRenderPass->use_dynamic_rendering == false) {
            const VkRenderPassCreateInfo2 *renderpass_create_info = cb_state.activeRenderPass->createInfo.ptr();
            const VkSubpassDescription2 *subpass_desc = &renderpass_create_info->pSubpasses[cb_state.activeSubpass];

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
                    auto val_fn = [this, attachment_index, fb_attachment, rectCount, clear_rect_copy](
                                      const CMD_BUFFER_STATE &secondary, const CMD_BUFFER_STATE *prim_cb,
                                      const FRAMEBUFFER_STATE *fb) {
                        assert(rectCount == clear_rect_copy->size());
                        const auto &render_area = prim_cb->activeRenderPassBeginInfo.renderArea;
                        bool skip = false;
                        const IMAGE_VIEW_STATE *image_view_state = nullptr;
                        if (fb && (fb_attachment != VK_ATTACHMENT_UNUSED) && (fb_attachment < fb->createInfo.attachmentCount)) {
                            image_view_state = prim_cb->GetActiveAttachmentImageViewState(fb_attachment);
                        }
                        skip = ValidateClearAttachmentExtent(secondary, attachment_index, image_view_state, render_area, rectCount,
                                                             clear_rect_copy->data());
                        return skip;
                    };
                    cb_state_ptr->cmd_execute_commands_functions.emplace_back(val_fn);
                }
            }
        }
    }
}

// Helper function to validate correct usage bits set for buffers or images. Verify that (actual & desired) flags != 0 or, if strict
// is true, verify that (actual & desired) flags == desired
bool CoreChecks::ValidateUsageFlags(VkFlags actual, VkFlags desired, VkBool32 strict, const LogObjectList &objlist,
                                    const VulkanTypedHandle &typed_handle, const char *msgCode, char const *func_name,
                                    char const *usage_str) const {
    bool correct_usage = false;
    bool skip = false;
    const char *type_str = object_string[typed_handle.type];
    if (strict) {
        correct_usage = ((actual & desired) == desired);
    } else {
        correct_usage = ((actual & desired) != 0);
    }

    if (!correct_usage) {
        // All callers should have a valid VUID
        assert(msgCode != kVUIDUndefined);
        skip =
            LogError(objlist, msgCode, "Invalid usage flag for %s used by %s. In this case, %s should have %s set during creation.",
                     report_data->FormatHandle(typed_handle).c_str(), func_name, type_str, usage_str);
    }
    return skip;
}

// Helper function to validate usage flags for buffers. For given buffer_state send actual vs. desired usage off to helper above
// where an error will be flagged if usage is not correct
bool CoreChecks::ValidateImageUsageFlags(VkCommandBuffer cb, IMAGE_STATE const &image_state, VkImageUsageFlags desired, bool strict,
                                         const char *msgCode, char const *func_name) const {
    LogObjectList objlist(cb, image_state.Handle());
    return ValidateUsageFlags(image_state.createInfo.usage, desired, strict, objlist, image_state.Handle(), msgCode, func_name,
                              string_VkImageUsageFlags(desired).c_str());
}

bool CoreChecks::ValidateImageFormatFeatureFlags(VkCommandBuffer cb, IMAGE_STATE const &image_state,
                                                 VkFormatFeatureFlags2KHR desired, char const *func_name, const char *vuid) const {
    bool skip = false;
    const VkFormatFeatureFlags2KHR image_format_features = image_state.format_features;
    if ((image_format_features & desired) != desired) {
        const LogObjectList objlist(cb, image_state.Handle());
        // Same error, but more details if it was an AHB external format
        if (image_state.HasAHBFormat()) {
            skip |= LogError(objlist, vuid,
                             "In %s, VkFormatFeatureFlags (0x%" PRIxLEAST64
                             ") does not support required feature %s for the external format "
                             "found in VkAndroidHardwareBufferFormatPropertiesANDROID::formatFeatures used by %s.",
                             func_name, image_format_features, string_VkFormatFeatureFlags2KHR(desired).c_str(),
                             report_data->FormatHandle(image_state.image()).c_str());
        } else {
            skip |= LogError(
                objlist, vuid,
                "In %s, VkFormatFeatureFlags (0x%" PRIxLEAST64 ") does not support required feature %s for format %" PRIu32
                " used by %s "
                "with tiling %s.",
                func_name, image_format_features, string_VkFormatFeatureFlags2KHR(desired).c_str(), image_state.createInfo.format,
                report_data->FormatHandle(image_state.image()).c_str(), string_VkImageTiling(image_state.createInfo.tiling));
        }
    }
    return skip;
}

bool CoreChecks::ValidateImageSubresourceLayers(const CMD_BUFFER_STATE &cb_state,
                                                const VkImageSubresourceLayers *subresource_layers, char const *func_name,
                                                char const *member, uint32_t i) const {
    bool skip = false;
    const VkImageAspectFlags apsect_mask = subresource_layers->aspectMask;
    // layerCount must not be zero
    if (subresource_layers->layerCount == 0) {
        skip |= LogError(cb_state.commandBuffer(), "VUID-VkImageSubresourceLayers-layerCount-01700",
                         "In %s, pRegions[%" PRIu32 "].%s.layerCount must not be zero.", func_name, i, member);
    }
    // aspectMask must not contain VK_IMAGE_ASPECT_METADATA_BIT
    if (apsect_mask & VK_IMAGE_ASPECT_METADATA_BIT) {
        skip |= LogError(cb_state.commandBuffer(), "VUID-VkImageSubresourceLayers-aspectMask-00168",
                         "In %s, pRegions[%" PRIu32 "].%s.aspectMask has VK_IMAGE_ASPECT_METADATA_BIT set.", func_name, i, member);
    }
    // if aspectMask contains COLOR, it must not contain either DEPTH or STENCIL
    if ((apsect_mask & VK_IMAGE_ASPECT_COLOR_BIT) && (apsect_mask & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT))) {
        skip |= LogError(cb_state.commandBuffer(), "VUID-VkImageSubresourceLayers-aspectMask-00167",
                         "In %s, pRegions[%" PRIu32
                         "].%s.aspectMask has VK_IMAGE_ASPECT_COLOR_BIT and either VK_IMAGE_ASPECT_DEPTH_BIT or "
                         "VK_IMAGE_ASPECT_STENCIL_BIT set.",
                         func_name, i, member);
    }
    // aspectMask must not contain VK_IMAGE_ASPECT_MEMORY_PLANE_i_BIT_EXT
    if (apsect_mask & (VK_IMAGE_ASPECT_MEMORY_PLANE_0_BIT_EXT | VK_IMAGE_ASPECT_MEMORY_PLANE_1_BIT_EXT |
                       VK_IMAGE_ASPECT_MEMORY_PLANE_2_BIT_EXT | VK_IMAGE_ASPECT_MEMORY_PLANE_3_BIT_EXT)) {
        skip |= LogError(cb_state.commandBuffer(), "VUID-VkImageSubresourceLayers-aspectMask-02247",
                         "In %s, pRegions[%" PRIu32 "].%s.aspectMask has a VK_IMAGE_ASPECT_MEMORY_PLANE_*_BIT_EXT bit set.",
                         func_name, i, member);
    }
    return skip;
}

// For the given format verify that the aspect masks make sense
bool CoreChecks::ValidateImageAspectMask(VkImage image, VkFormat format, VkImageAspectFlags aspect_mask, bool is_image_disjoint,
                                         const char *func_name, const char *vuid) const {
    bool skip = false;
    // checks color format and (single-plane or non-disjoint)
    // if ycbcr extension is not supported then single-plane and non-disjoint are always both true
    if ((FormatIsColor(format)) && ((FormatIsMultiplane(format) == false) || (is_image_disjoint == false))) {
        if ((aspect_mask & VK_IMAGE_ASPECT_COLOR_BIT) != VK_IMAGE_ASPECT_COLOR_BIT) {
            skip |= LogError(
                image, vuid,
                "%s: Using format (%s) with aspect flags (%s) but color image formats must have the VK_IMAGE_ASPECT_COLOR_BIT set.",
                func_name, string_VkFormat(format), string_VkImageAspectFlags(aspect_mask).c_str());
        } else if ((aspect_mask & VK_IMAGE_ASPECT_COLOR_BIT) != aspect_mask) {
            skip |= LogError(image, vuid,
                             "%s: Using format (%s) with aspect flags (%s) but color image formats must have ONLY the "
                             "VK_IMAGE_ASPECT_COLOR_BIT set.",
                             func_name, string_VkFormat(format), string_VkImageAspectFlags(aspect_mask).c_str());
        }
    } else if (FormatIsDepthAndStencil(format)) {
        if ((aspect_mask & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) == 0) {
            skip |= LogError(image, vuid,
                             "%s: Using format (%s) with aspect flags (%s) but depth/stencil image formats must have at least one "
                             "of VK_IMAGE_ASPECT_DEPTH_BIT and VK_IMAGE_ASPECT_STENCIL_BIT set.",
                             func_name, string_VkFormat(format), string_VkImageAspectFlags(aspect_mask).c_str());
        } else if ((aspect_mask & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) != aspect_mask) {
            skip |= LogError(image, vuid,
                             "%s: Using format (%s) with aspect flags (%s) but combination depth/stencil image formats can have "
                             "only the VK_IMAGE_ASPECT_DEPTH_BIT and VK_IMAGE_ASPECT_STENCIL_BIT set.",
                             func_name, string_VkFormat(format), string_VkImageAspectFlags(aspect_mask).c_str());
        }
    } else if (FormatIsDepthOnly(format)) {
        if ((aspect_mask & VK_IMAGE_ASPECT_DEPTH_BIT) != VK_IMAGE_ASPECT_DEPTH_BIT) {
            skip |= LogError(image, vuid,
                             "%s: Using format (%s) with aspect flags (%s) but depth-only image formats must have the "
                             "VK_IMAGE_ASPECT_DEPTH_BIT set.",
                             func_name, string_VkFormat(format), string_VkImageAspectFlags(aspect_mask).c_str());
        } else if ((aspect_mask & VK_IMAGE_ASPECT_DEPTH_BIT) != aspect_mask) {
            skip |= LogError(image, vuid,
                             "%s: Using format (%s) with aspect flags (%s) but depth-only image formats can have only the "
                             "VK_IMAGE_ASPECT_DEPTH_BIT set.",
                             func_name, string_VkFormat(format), string_VkImageAspectFlags(aspect_mask).c_str());
        }
    } else if (FormatIsStencilOnly(format)) {
        if ((aspect_mask & VK_IMAGE_ASPECT_STENCIL_BIT) != VK_IMAGE_ASPECT_STENCIL_BIT) {
            skip |= LogError(image, vuid,
                             "%s: Using format (%s) with aspect flags (%s) but stencil-only image formats must have the "
                             "VK_IMAGE_ASPECT_STENCIL_BIT set.",
                             func_name, string_VkFormat(format), string_VkImageAspectFlags(aspect_mask).c_str());
        } else if ((aspect_mask & VK_IMAGE_ASPECT_STENCIL_BIT) != aspect_mask) {
            skip |= LogError(image, vuid,
                             "%s: Using format (%s) with aspect flags (%s) but stencil-only image formats can have only the "
                             "VK_IMAGE_ASPECT_STENCIL_BIT set.",
                             func_name, string_VkFormat(format), string_VkImageAspectFlags(aspect_mask).c_str());
        }
    } else if (FormatIsMultiplane(format)) {
        VkImageAspectFlags valid_flags = VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT;
        if (3 == FormatPlaneCount(format)) {
            valid_flags = valid_flags | VK_IMAGE_ASPECT_PLANE_2_BIT;
        }
        if ((aspect_mask & valid_flags) != aspect_mask) {
            skip |= LogError(image, vuid,
                             "%s: Using format (%s) with aspect flags (%s) but multi-plane image formats may have only "
                             "VK_IMAGE_ASPECT_COLOR_BIT or VK_IMAGE_ASPECT_PLANE_n_BITs set, where n = [0, 1, 2].",
                             func_name, string_VkFormat(format), string_VkImageAspectFlags(aspect_mask).c_str());
        }
    }
    return skip;
}

bool CoreChecks::ValidateImageSubresourceRange(const uint32_t image_mip_count, const uint32_t image_layer_count,
                                               const VkImageSubresourceRange &subresourceRange, const char *cmd_name,
                                               const char *param_name, const char *image_layer_count_var_name, const VkImage image,
                                               const SubresourceRangeErrorCodes &errorCodes) const {
    bool skip = false;

    // Validate mip levels
    if (subresourceRange.baseMipLevel >= image_mip_count) {
        skip |= LogError(image, errorCodes.base_mip_err,
                         "%s: %s.baseMipLevel (= %" PRIu32
                         ") is greater or equal to the mip level count of the image (i.e. greater or equal to %" PRIu32 ").",
                         cmd_name, param_name, subresourceRange.baseMipLevel, image_mip_count);
    }

    if (subresourceRange.levelCount != VK_REMAINING_MIP_LEVELS) {
        if (subresourceRange.levelCount == 0) {
            skip |=
                LogError(image, "VUID-VkImageSubresourceRange-levelCount-01720", "%s: %s.levelCount is 0.", cmd_name, param_name);
        } else {
            const uint64_t necessary_mip_count = uint64_t{subresourceRange.baseMipLevel} + uint64_t{subresourceRange.levelCount};

            if (necessary_mip_count > image_mip_count) {
                skip |= LogError(image, errorCodes.mip_count_err,
                                 "%s: %s.baseMipLevel + .levelCount (= %" PRIu32 " + %" PRIu32 " = %" PRIu64
                                 ") is greater than the mip level count of the image (i.e. greater than %" PRIu32 ").",
                                 cmd_name, param_name, subresourceRange.baseMipLevel, subresourceRange.levelCount,
                                 necessary_mip_count, image_mip_count);
            }
        }
    }

    // Validate array layers
    if (subresourceRange.baseArrayLayer >= image_layer_count) {
        skip |= LogError(image, errorCodes.base_layer_err,
                         "%s: %s.baseArrayLayer (= %" PRIu32
                         ") is greater or equal to the %s of the image when it was created (i.e. greater or equal to %" PRIu32 ").",
                         cmd_name, param_name, subresourceRange.baseArrayLayer, image_layer_count_var_name, image_layer_count);
    }

    if (subresourceRange.layerCount != VK_REMAINING_ARRAY_LAYERS) {
        if (subresourceRange.layerCount == 0) {
            skip |=
                LogError(image, "VUID-VkImageSubresourceRange-layerCount-01721", "%s: %s.layerCount is 0.", cmd_name, param_name);
        } else {
            const uint64_t necessary_layer_count =
                uint64_t{subresourceRange.baseArrayLayer} + uint64_t{subresourceRange.layerCount};

            if (necessary_layer_count > image_layer_count) {
                skip |= LogError(image, errorCodes.layer_count_err,
                                 "%s: %s.baseArrayLayer + .layerCount (= %" PRIu32 " + %" PRIu32 " = %" PRIu64
                                 ") is greater than the %s of the image when it was created (i.e. greater than %" PRIu32 ").",
                                 cmd_name, param_name, subresourceRange.baseArrayLayer, subresourceRange.layerCount,
                                 necessary_layer_count, image_layer_count_var_name, image_layer_count);
            }
        }
    }

    if (subresourceRange.aspectMask & VK_IMAGE_ASPECT_COLOR_BIT) {
        if (subresourceRange.aspectMask &
            (VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT | VK_IMAGE_ASPECT_PLANE_2_BIT)) {
            skip |= LogError(image, "VUID-VkImageSubresourceRange-aspectMask-01670",
                             "%s: aspectMask includes both VK_IMAGE_ASPECT_COLOR_BIT and one of VK_IMAGE_ASPECT_PLANE_0_BIT, "
                             "VK_IMAGE_ASPECT_PLANE_1_BIT, or VK_IMAGE_ASPECT_PLANE_2_BIT.",
                             cmd_name);
        }
    }

    return skip;
}

bool CoreChecks::ValidateCreateImageViewSubresourceRange(const IMAGE_STATE *image_state, bool is_imageview_2d_type,
                                                         const VkImageSubresourceRange &subresourceRange) const {
    const bool is_khr_maintenance1 = IsExtEnabled(device_extensions.vk_khr_maintenance1);
    const bool is_2d_compatible =
        image_state->createInfo.flags & (VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT | VK_IMAGE_CREATE_2D_VIEW_COMPATIBLE_BIT_EXT);
    const bool is_image_slicable = (image_state->createInfo.imageType == VK_IMAGE_TYPE_3D) && is_2d_compatible;
    const bool is_3_d_to_2_d_map = is_khr_maintenance1 && is_image_slicable && is_imageview_2d_type;

    uint32_t image_layer_count;

    if (is_3_d_to_2_d_map) {
        const auto layers = LayersFromRange(subresourceRange);
        const auto extent = image_state->GetSubresourceExtent(layers);
        image_layer_count = extent.depth;
    } else {
        image_layer_count = image_state->createInfo.arrayLayers;
    }

    const auto image_layer_count_var_name = is_3_d_to_2_d_map ? "extent.depth" : "arrayLayers";

    SubresourceRangeErrorCodes subresource_range_error_codes = {};
    subresource_range_error_codes.base_mip_err = "VUID-VkImageViewCreateInfo-subresourceRange-01478";
    subresource_range_error_codes.mip_count_err = "VUID-VkImageViewCreateInfo-subresourceRange-01718";
    subresource_range_error_codes.base_layer_err =
        is_khr_maintenance1 ? (is_3_d_to_2_d_map ? "VUID-VkImageViewCreateInfo-image-02724"
                                                 : (IsExtEnabled(device_extensions.vk_ext_image_2d_view_of_3d)
                                                        ? "VUID-VkImageViewCreateInfo-image-06724"
                                                        : "VUID-VkImageViewCreateInfo-image-01482"))
                            : "VUID-VkImageViewCreateInfo-subresourceRange-01480";
    subresource_range_error_codes.layer_count_err =
        is_khr_maintenance1 ? (is_3_d_to_2_d_map ? "VUID-VkImageViewCreateInfo-subresourceRange-02725"
                                                 : (IsExtEnabled(device_extensions.vk_ext_image_2d_view_of_3d)
                                                        ? "VUID-VkImageViewCreateInfo-subresourceRange-06725"
                                                        : "VUID-VkImageViewCreateInfo-subresourceRange-01483"))
                            : "VUID-VkImageViewCreateInfo-subresourceRange-01719";

    return ValidateImageSubresourceRange(image_state->createInfo.mipLevels, image_layer_count, subresourceRange,
                                         "vkCreateImageView", "pCreateInfo->subresourceRange", image_layer_count_var_name,
                                         image_state->image(), subresource_range_error_codes);
}

bool CoreChecks::ValidateCmdClearColorSubresourceRange(const IMAGE_STATE *image_state,
                                                       const VkImageSubresourceRange &subresourceRange,
                                                       const char *param_name) const {
    SubresourceRangeErrorCodes subresource_range_error_codes = {};
    subresource_range_error_codes.base_mip_err = "VUID-vkCmdClearColorImage-baseMipLevel-01470";
    subresource_range_error_codes.mip_count_err = "VUID-vkCmdClearColorImage-pRanges-01692";
    subresource_range_error_codes.base_layer_err = "VUID-vkCmdClearColorImage-baseArrayLayer-01472";
    subresource_range_error_codes.layer_count_err = "VUID-vkCmdClearColorImage-pRanges-01693";

    return ValidateImageSubresourceRange(image_state->createInfo.mipLevels, image_state->createInfo.arrayLayers, subresourceRange,
                                         "vkCmdClearColorImage", param_name, "arrayLayers", image_state->image(),
                                         subresource_range_error_codes);
}

bool CoreChecks::ValidateCmdClearDepthSubresourceRange(const IMAGE_STATE *image_state,
                                                       const VkImageSubresourceRange &subresourceRange,
                                                       const char *param_name) const {
    SubresourceRangeErrorCodes subresource_range_error_codes = {};
    subresource_range_error_codes.base_mip_err = "VUID-vkCmdClearDepthStencilImage-baseMipLevel-01474";
    subresource_range_error_codes.mip_count_err = "VUID-vkCmdClearDepthStencilImage-pRanges-01694";
    subresource_range_error_codes.base_layer_err = "VUID-vkCmdClearDepthStencilImage-baseArrayLayer-01476";
    subresource_range_error_codes.layer_count_err = "VUID-vkCmdClearDepthStencilImage-pRanges-01695";

    return ValidateImageSubresourceRange(image_state->createInfo.mipLevels, image_state->createInfo.arrayLayers, subresourceRange,
                                         "vkCmdClearDepthStencilImage", param_name, "arrayLayers", image_state->image(),
                                         subresource_range_error_codes);
}

bool CoreChecks::ValidateImageBarrierSubresourceRange(const Location &loc, const IMAGE_STATE *image_state,
                                                      const VkImageSubresourceRange &subresourceRange) const {
    return ValidateImageSubresourceRange(image_state->createInfo.mipLevels, image_state->createInfo.arrayLayers, subresourceRange,
                                         loc.StringFunc().c_str(), loc.StringField().c_str(), "arrayLayers", image_state->image(),
                                         sync_vuid_maps::GetSubResourceVUIDs(loc));
}

bool CoreChecks::ValidateImageViewFormatFeatures(const IMAGE_STATE *image_state, const VkFormat view_format,
                                                 const VkImageUsageFlags image_usage) const {
    // Pass in image_usage here instead of extracting it from image_state in case there's a chained VkImageViewUsageCreateInfo
    bool skip = false;

    VkFormatFeatureFlags2KHR tiling_features = 0;
    const VkImageTiling image_tiling = image_state->createInfo.tiling;

    if (image_state->HasAHBFormat()) {
        // AHB image view and image share same feature sets
        tiling_features = image_state->format_features;
    } else if (image_tiling == VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT) {
        // Parameter validation should catch if this is used without VK_EXT_image_drm_format_modifier
        assert(IsExtEnabled(device_extensions.vk_ext_image_drm_format_modifier));
        VkImageDrmFormatModifierPropertiesEXT drm_format_properties = LvlInitStruct<VkImageDrmFormatModifierPropertiesEXT>();
        DispatchGetImageDrmFormatModifierPropertiesEXT(device, image_state->image(), &drm_format_properties);

        auto fmt_drm_props = LvlInitStruct<VkDrmFormatModifierPropertiesListEXT>();
        auto fmt_props_2 = LvlInitStruct<VkFormatProperties2>(&fmt_drm_props);
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
        skip |= LogError(image_state->image(), "VUID-VkImageViewCreateInfo-None-02273",
                         "vkCreateImageView(): pCreateInfo->format %s with tiling %s has no supported format features on this "
                         "physical device.",
                         string_VkFormat(view_format), string_VkImageTiling(image_tiling));
    } else if ((image_usage & VK_IMAGE_USAGE_SAMPLED_BIT) && !(tiling_features & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)) {
        skip |= LogError(image_state->image(), "VUID-VkImageViewCreateInfo-usage-02274",
                         "vkCreateImageView(): pCreateInfo->format %s with tiling %s does not support usage that includes "
                         "VK_IMAGE_USAGE_SAMPLED_BIT.",
                         string_VkFormat(view_format), string_VkImageTiling(image_tiling));
    } else if ((image_usage & VK_IMAGE_USAGE_STORAGE_BIT) && !(tiling_features & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT)) {
        skip |= LogError(image_state->image(), "VUID-VkImageViewCreateInfo-usage-02275",
                         "vkCreateImageView(): pCreateInfo->format %s with tiling %s does not support usage that includes "
                         "VK_IMAGE_USAGE_STORAGE_BIT.",
                         string_VkFormat(view_format), string_VkImageTiling(image_tiling));
    } else if ((image_usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) && !(tiling_features & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT)) {
        skip |= LogError(image_state->image(), "VUID-VkImageViewCreateInfo-usage-02276",
                         "vkCreateImageView(): pCreateInfo->format %s with tiling %s does not support usage that includes "
                         "VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT.",
                         string_VkFormat(view_format), string_VkImageTiling(image_tiling));
    } else if ((image_usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) &&
               !(tiling_features & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
        skip |= LogError(image_state->image(), "VUID-VkImageViewCreateInfo-usage-02277",
                         "vkCreateImageView(): pCreateInfo->format %s with tiling %s does not support usage that includes "
                         "VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT.",
                         string_VkFormat(view_format), string_VkImageTiling(image_tiling));
    } else if ((image_usage & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT) &&
               !(tiling_features & (VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT))) {
        skip |= LogError(image_state->image(), "VUID-VkImageViewCreateInfo-usage-02652",
                         "vkCreateImageView(): pCreateInfo->format %s with tiling %s does not support usage that includes "
                         "VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT or VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT.",
                         string_VkFormat(view_format), string_VkImageTiling(image_tiling));
    } else if ((image_usage & VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR) &&
               !(tiling_features & VK_FORMAT_FEATURE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR)) {
        if (enabled_features.fragment_shading_rate_features.attachmentFragmentShadingRate) {
            skip |= LogError(image_state->image(), "VUID-VkImageViewCreateInfo-usage-04550",
                             "vkCreateImageView(): pCreateInfo->format %s with tiling %s does not support usage that includes "
                             "VK_FORMAT_FEATURE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR.",
                             string_VkFormat(view_format), string_VkImageTiling(image_tiling));
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateCreateImageView(VkDevice device, const VkImageViewCreateInfo *pCreateInfo,
                                                const VkAllocationCallbacks *pAllocator, VkImageView *pView) const {
    bool skip = false;
    auto image_state = Get<IMAGE_STATE>(pCreateInfo->image);
    if (image_state) {
        const VkImageUsageFlags valid_usage_flags =
            VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT |
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
            VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR |
            VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT | VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR |
            VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR | VK_IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT_KHR |
            VK_IMAGE_USAGE_VIDEO_ENCODE_DPB_BIT_KHR | VK_IMAGE_USAGE_SAMPLE_WEIGHT_BIT_QCOM |
            VK_IMAGE_USAGE_SAMPLE_BLOCK_MATCH_BIT_QCOM;
        skip |= ValidateImageUsageFlags(VK_NULL_HANDLE, *image_state, valid_usage_flags, false,
                                        "VUID-VkImageViewCreateInfo-image-04441", "vkCreateImageView()");
        // If this isn't a sparse image, it needs to have memory backing it at CreateImageView time
        skip |= ValidateMemoryIsBoundToImage(device, *image_state, "vkCreateImageView()", "VUID-VkImageViewCreateInfo-image-01020");
        // Checks imported from image layer
        skip |= ValidateCreateImageViewSubresourceRange(
            image_state.get(),
            pCreateInfo->viewType == VK_IMAGE_VIEW_TYPE_2D || pCreateInfo->viewType == VK_IMAGE_VIEW_TYPE_2D_ARRAY,
            pCreateInfo->subresourceRange);

        const auto normalized_subresource_range = image_state->NormalizeSubresourceRange(pCreateInfo->subresourceRange);
        VkImageCreateFlags image_flags = image_state->createInfo.flags;
        VkFormat image_format = image_state->createInfo.format;
        VkImageUsageFlags image_usage = image_state->createInfo.usage;
        VkFormat view_format = pCreateInfo->format;
        VkImageAspectFlags aspect_mask = pCreateInfo->subresourceRange.aspectMask;
        VkImageType image_type = image_state->createInfo.imageType;
        VkImageViewType view_type = pCreateInfo->viewType;
        uint32_t layer_count = pCreateInfo->subresourceRange.layerCount;

        // If there's a chained VkImageViewUsageCreateInfo struct, modify image_usage to match
        auto chained_ivuci_struct = LvlFindInChain<VkImageViewUsageCreateInfo>(pCreateInfo->pNext);
        if (chained_ivuci_struct) {
            if (IsExtEnabled(device_extensions.vk_khr_maintenance2)) {
                if (!IsExtEnabled(device_extensions.vk_ext_separate_stencil_usage)) {
                    if ((image_usage | chained_ivuci_struct->usage) != image_usage) {
                        skip |= LogError(pCreateInfo->image, "VUID-VkImageViewCreateInfo-pNext-02661",
                                         "vkCreateImageView(): pNext chain includes VkImageViewUsageCreateInfo, usage must not "
                                         "include any bits that were not set in VkImageCreateInfo::usage used to create image");
                    }
                } else {
                    const auto image_stencil_struct = LvlFindInChain<VkImageStencilUsageCreateInfo>(image_state->createInfo.pNext);
                    if (image_stencil_struct == nullptr) {
                        if ((image_usage | chained_ivuci_struct->usage) != image_usage) {
                            skip |= LogError(
                                pCreateInfo->image, "VUID-VkImageViewCreateInfo-pNext-02662",
                                "vkCreateImageView(): pNext chain includes VkImageViewUsageCreateInfo and image was not created "
                                "with a VkImageStencilUsageCreateInfo in pNext of vkImageCreateInfo, usage must not include "
                                "any bits that were not set in VkImageCreateInfo::usage used to create image");
                        }
                    } else {
                        if ((aspect_mask & VK_IMAGE_ASPECT_STENCIL_BIT) == VK_IMAGE_ASPECT_STENCIL_BIT &&
                            (image_stencil_struct->stencilUsage | chained_ivuci_struct->usage) !=
                                image_stencil_struct->stencilUsage) {
                            skip |= LogError(
                                pCreateInfo->image, "VUID-VkImageViewCreateInfo-pNext-02663",
                                "vkCreateImageView(): pNext chain includes VkImageViewUsageCreateInfo, image was created with a "
                                "VkImageStencilUsageCreateInfo in pNext of vkImageCreateInfo, and subResourceRange.aspectMask "
                                "includes VK_IMAGE_ASPECT_STENCIL_BIT, VkImageViewUsageCreateInfo::usage must not include any "
                                "bits that were not set in VkImageStencilUsageCreateInfo::stencilUsage used to create image");
                        }
                        if ((aspect_mask & ~VK_IMAGE_ASPECT_STENCIL_BIT) != 0 &&
                            (image_usage | chained_ivuci_struct->usage) != image_usage) {
                            skip |= LogError(
                                pCreateInfo->image, "VUID-VkImageViewCreateInfo-pNext-02664",
                                "vkCreateImageView(): pNext chain includes VkImageViewUsageCreateInfo, image was created with a "
                                "VkImageStencilUsageCreateInfo in pNext of vkImageCreateInfo, and subResourceRange.aspectMask "
                                "includes bits other than VK_IMAGE_ASPECT_STENCIL_BIT, VkImageViewUsageCreateInfo::usage must not "
                                "include any bits that were not set in VkImageCreateInfo::usage used to create image");
                        }
                    }
                }
            }

            image_usage = chained_ivuci_struct->usage;
        }

        // If image used VkImageFormatListCreateInfo need to make sure a format from list is used
        const auto format_list_info = LvlFindInChain<VkImageFormatListCreateInfo>(image_state->createInfo.pNext);
        if (format_list_info && (format_list_info->viewFormatCount > 0)) {
            bool foundFormat = false;
            for (uint32_t i = 0; i < format_list_info->viewFormatCount; i++) {
                if (format_list_info->pViewFormats[i] == view_format) {
                    foundFormat = true;
                    break;
                }
            }
            if (foundFormat == false) {
                skip |= LogError(pCreateInfo->image, "VUID-VkImageViewCreateInfo-pNext-01585",
                                 "vkCreateImageView(): image was created with a VkImageFormatListCreateInfo in pNext of "
                                 "vkImageCreateInfo, but none of the formats match the VkImageViewCreateInfo::format (%s).",
                                 string_VkFormat(view_format));
            }
        }

        // Validate VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT state, if view/image formats differ
        if ((image_flags & VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT) && (image_format != view_format)) {
            if (FormatIsMultiplane(image_format)) {
                const VkFormat compat_format = FindMultiplaneCompatibleFormat(image_format, aspect_mask);
                auto image_class = FormatCompatibilityClass(compat_format);
                auto view_class = FormatCompatibilityClass(view_format);
                // Need to only check if one is NONE to handle edge case both are NONE
                if ((image_class != view_class) || (image_class == FORMAT_COMPATIBILITY_CLASS::NONE)) {
                    // View format must match the multiplane compatible format
                    std::stringstream ss;
                    ss << "vkCreateImageView(): ImageView format " << string_VkFormat(view_format)
                       << " is not compatible with plane " << GetPlaneIndex(aspect_mask) << " of underlying image format "
                       << string_VkFormat(image_format) << ", must be compatible with " << string_VkFormat(compat_format) << ".";
                    skip |= LogError(pCreateInfo->image, "VUID-VkImageViewCreateInfo-image-01586", "%s", ss.str().c_str());
                }
            } else if (!(image_flags & VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT)) {
                // Format MUST be compatible (in the same format compatibility class) as the format the image was created with
                auto image_class = FormatCompatibilityClass(image_format);
                auto view_class = FormatCompatibilityClass(view_format);
                // Need to only check if one is NONE to handle edge case both are NONE
                if ((image_class != view_class) || (image_class == FORMAT_COMPATIBILITY_CLASS::NONE)) {
                    const char *error_vuid;
                    if ((!IsExtEnabled(device_extensions.vk_khr_maintenance2)) &&
                        (!IsExtEnabled(device_extensions.vk_khr_sampler_ycbcr_conversion))) {
                        error_vuid = "VUID-VkImageViewCreateInfo-image-01018";
                    } else if ((IsExtEnabled(device_extensions.vk_khr_maintenance2)) &&
                               (!IsExtEnabled(device_extensions.vk_khr_sampler_ycbcr_conversion))) {
                        error_vuid = "VUID-VkImageViewCreateInfo-image-01759";
                    } else if ((!IsExtEnabled(device_extensions.vk_khr_maintenance2)) &&
                               (IsExtEnabled(device_extensions.vk_khr_sampler_ycbcr_conversion))) {
                        error_vuid = "VUID-VkImageViewCreateInfo-image-01760";
                    } else {
                        // both enabled
                        error_vuid = "VUID-VkImageViewCreateInfo-image-01761";
                    }
                    std::stringstream ss;
                    ss << "vkCreateImageView(): ImageView format " << string_VkFormat(view_format)
                       << " is not in the same format compatibility class as "
                       << report_data->FormatHandle(pCreateInfo->image).c_str() << "  format " << string_VkFormat(image_format)
                       << ".  Images created with the VK_IMAGE_CREATE_MUTABLE_FORMAT BIT "
                       << "can support ImageViews with differing formats but they must be in the same compatibility class.";
                    skip |= LogError(pCreateInfo->image, error_vuid, "%s", ss.str().c_str());
                }
            }
        } else {
            // Format MUST be IDENTICAL to the format the image was created with
            // Unless it is a multi-planar color bit aspect
            if ((image_format != view_format) &&
                ((FormatIsMultiplane(image_format) == false) || (aspect_mask != VK_IMAGE_ASPECT_COLOR_BIT))) {
                const char *vuid = IsExtEnabled(device_extensions.vk_khr_sampler_ycbcr_conversion)
                                       ? "VUID-VkImageViewCreateInfo-image-01762"
                                       : "VUID-VkImageViewCreateInfo-image-01019";
                std::stringstream ss;
                ss << "vkCreateImageView() format " << string_VkFormat(view_format) << " differs from "
                   << report_data->FormatHandle(pCreateInfo->image).c_str() << " format " << string_VkFormat(image_format)
                   << ".  Formats MUST be IDENTICAL unless VK_IMAGE_CREATE_MUTABLE_FORMAT BIT was set on image creation.";
                skip |= LogError(pCreateInfo->image, vuid, "%s", ss.str().c_str());
            }
        }

        if (image_state->createInfo.samples != VK_SAMPLE_COUNT_1_BIT && view_type != VK_IMAGE_VIEW_TYPE_2D &&
            view_type != VK_IMAGE_VIEW_TYPE_2D_ARRAY) {
            skip |= LogError(pCreateInfo->image, "VUID-VkImageViewCreateInfo-image-04972",
                             "vkCreateImageView(): image was created with sample count %s, but pCreateInfo->viewType is %s.",
                             string_VkSampleCountFlagBits(image_state->createInfo.samples), string_VkImageViewType(view_type));
        }

        if (image_state->createInfo.usage & (VK_IMAGE_USAGE_VIDEO_ENCODE_DST_BIT_KHR | VK_IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT_KHR |
                                             VK_IMAGE_USAGE_VIDEO_ENCODE_DPB_BIT_KHR)) {
            if (view_type != VK_IMAGE_VIEW_TYPE_2D && view_type != VK_IMAGE_VIEW_TYPE_2D_ARRAY) {
                skip |= LogError(pCreateInfo->image, "VUID-VkImageViewCreateInfo-image-04818",
                                 "vkCreateImageView(): Image created with video encode usage flags, but pCreateInfo->viewType (%s) "
                                 "is not VK_IMAGE_VIEW_TYPE_2D or VK_IMAGE_VIEW_TYPE_2D_ARRAY.",
                                 string_VkImageViewType(view_type));
            }
            if (!IsIdentitySwizzle(pCreateInfo->components)) {
                skip |= LogError(
                    pCreateInfo->image, "VUID-VkImageViewCreateInfo-image-04818",
                    "vkCreateImageView(): Image created with video encode usage flags, but not all members of "
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
        skip |=
            ValidateImageAspectMask(image_state->image(), image_format, aspect_mask, image_state->disjoint, "vkCreateImageView()");

        // Valdiate Image/ImageView type compatibility #resources-image-views-compatibility
        switch (image_type) {
            case VK_IMAGE_TYPE_1D:
                if (view_type != VK_IMAGE_VIEW_TYPE_1D && view_type != VK_IMAGE_VIEW_TYPE_1D_ARRAY) {
                    skip |= LogError(pCreateInfo->image, "VUID-VkImageViewCreateInfo-subResourceRange-01021",
                                     "vkCreateImageView(): pCreateInfo->viewType %s is not compatible with image type %s.",
                                     string_VkImageViewType(view_type), string_VkImageType(image_type));
                }
                break;
            case VK_IMAGE_TYPE_2D:
                if (view_type != VK_IMAGE_VIEW_TYPE_2D && view_type != VK_IMAGE_VIEW_TYPE_2D_ARRAY) {
                    if ((view_type == VK_IMAGE_VIEW_TYPE_CUBE || view_type == VK_IMAGE_VIEW_TYPE_CUBE_ARRAY) &&
                        !(image_flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT)) {
                        skip |= LogError(pCreateInfo->image, "VUID-VkImageViewCreateInfo-image-01003",
                                         "vkCreateImageView(): pCreateInfo->viewType %s is not compatible with image type %s.",
                                         string_VkImageViewType(view_type), string_VkImageType(image_type));
                    } else if (view_type != VK_IMAGE_VIEW_TYPE_CUBE && view_type != VK_IMAGE_VIEW_TYPE_CUBE_ARRAY) {
                        skip |= LogError(pCreateInfo->image, "VUID-VkImageViewCreateInfo-subResourceRange-01021",
                                         "vkCreateImageView(): pCreateInfo->viewType %s is not compatible with image type %s.",
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
                                            pCreateInfo->image, "VUID-VkImageViewCreateInfo-image-06723",
                                            "vkCreateImageView(): pCreateInfo->viewType %s is not compatible with image type "
                                            "%s since the image doesn't have VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT flag set.",
                                            string_VkImageViewType(view_type), string_VkImageType(image_type));
                                    } else if (view_type == VK_IMAGE_VIEW_TYPE_2D &&
                                               !(image_flags & VK_IMAGE_CREATE_2D_VIEW_COMPATIBLE_BIT_EXT)) {
                                        skip |= LogError(
                                            pCreateInfo->image, "VUID-VkImageViewCreateInfo-image-06728",
                                            "vkCreateImageView(): pCreateInfo->viewType %s is not compatible with image type "
                                            "%s since the image doesn't have VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT or "
                                            "VK_IMAGE_CREATE_2D_VIEW_COMPATIBLE_BIT_EXT flag set.",
                                            string_VkImageViewType(view_type), string_VkImageType(image_type));
                                    }
                                }
                            } else if (!(image_flags & VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT) &&
                                       (view_type == VK_IMAGE_VIEW_TYPE_2D)) {
                                skip |=
                                    LogError(pCreateInfo->image, "VUID-VkImageViewCreateInfo-image-06727",
                                             "vkCreateImageView(): pCreateInfo->viewType VK_IMAGE_VIEW_TYPE_2D is not compatible "
                                             "with image type "
                                             "%s since the image doesn't have VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT flag set.",
                                             string_VkImageType(image_type));
                            }
                            if ((image_flags & (VK_IMAGE_CREATE_SPARSE_BINDING_BIT | VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT |
                                                VK_IMAGE_CREATE_SPARSE_ALIASED_BIT))) {
                                skip |= LogError(
                                    pCreateInfo->image, "VUID-VkImageViewCreateInfo-image-04971",
                                    "vkCreateImageView(): pCreateInfo->viewType %s is not compatible with image type %s "
                                    "when the VK_IMAGE_CREATE_SPARSE_BINDING_BIT, VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT, or "
                                    "VK_IMAGE_CREATE_SPARSE_ALIASED_BIT flags are enabled.",
                                    string_VkImageViewType(view_type), string_VkImageType(image_type));
                            } else if (pCreateInfo->subresourceRange.levelCount != 1) {
                                skip |= LogError(pCreateInfo->image, "VUID-VkImageViewCreateInfo-image-04970",
                                                 "vkCreateImageView(): pCreateInfo->viewType %s is with image type %s must have a "
                                                 "levelCount of 1 but it is %" PRIu32 ".",
                                                 string_VkImageViewType(view_type), string_VkImageType(image_type),
                                                 pCreateInfo->subresourceRange.levelCount);
                            }
                        } else {
                            skip |= LogError(pCreateInfo->image, "VUID-VkImageViewCreateInfo-subResourceRange-01021",
                                             "vkCreateImageView(): pCreateInfo->viewType %s is not compatible with image type %s.",
                                             string_VkImageViewType(view_type), string_VkImageType(image_type));
                        }
                    }
                } else {
                    if (view_type != VK_IMAGE_VIEW_TYPE_3D) {
                        // Help point to VK_KHR_maintenance1
                        if ((view_type == VK_IMAGE_VIEW_TYPE_2D || view_type == VK_IMAGE_VIEW_TYPE_2D_ARRAY)) {
                            skip |= LogError(pCreateInfo->image, "VUID-VkImageViewCreateInfo-subResourceRange-01021",
                                             "vkCreateImageView(): pCreateInfo->viewType %s is not compatible with image type %s "
                                             "without VK_KHR_maintenance1 enabled which was promoted in Vulkan 1.0.",
                                             string_VkImageViewType(view_type), string_VkImageType(image_type));
                        } else {
                            skip |= LogError(pCreateInfo->image, "VUID-VkImageViewCreateInfo-subResourceRange-01021",
                                             "vkCreateImageView(): pCreateInfo->viewType %s is not compatible with image type %s.",
                                             string_VkImageViewType(view_type), string_VkImageType(image_type));
                        }
                    }
                }
                break;
            default:
                break;
        }

        // External format checks needed when VK_ANDROID_external_memory_android_hardware_buffer enabled
        if (IsExtEnabled(device_extensions.vk_android_external_memory_android_hardware_buffer)) {
            skip |= ValidateCreateImageViewANDROID(pCreateInfo);
        }

        skip |= ValidateImageViewFormatFeatures(image_state.get(), view_format, image_usage);

        if (enabled_features.shading_rate_image_features.shadingRateImage) {
            if (image_usage & VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV) {
                if (view_format != VK_FORMAT_R8_UINT) {
                    skip |= LogError(pCreateInfo->image, "VUID-VkImageViewCreateInfo-image-02087",
                                     "vkCreateImageView() If image was created with usage containing "
                                     "VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV, format must be VK_FORMAT_R8_UINT.");
                }
            }
        }

        if (enabled_features.shading_rate_image_features.shadingRateImage ||
            enabled_features.fragment_shading_rate_features.attachmentFragmentShadingRate) {
            if (image_usage & VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR) {
                if (view_type != VK_IMAGE_VIEW_TYPE_2D && view_type != VK_IMAGE_VIEW_TYPE_2D_ARRAY) {
                    skip |= LogError(pCreateInfo->image, "VUID-VkImageViewCreateInfo-image-02086",
                                     "vkCreateImageView() If image was created with usage containing "
                                     "VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR, viewType must be "
                                     "VK_IMAGE_VIEW_TYPE_2D or VK_IMAGE_VIEW_TYPE_2D_ARRAY.");
                }
            }
        }

        if (enabled_features.fragment_shading_rate_features.attachmentFragmentShadingRate &&
            !phys_dev_ext_props.fragment_shading_rate_props.layeredShadingRateAttachments &&
            image_usage & VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR && normalized_subresource_range.layerCount != 1) {
            skip |= LogError(device, "VUID-VkImageViewCreateInfo-usage-04551",
                             "vkCreateImageView(): subresourceRange.layerCount is %" PRIu32
                             " for a shading rate attachment image view.",
                             normalized_subresource_range.layerCount);
        }

        if (layer_count == VK_REMAINING_ARRAY_LAYERS) {
            const uint32_t remaining_layers = image_state->createInfo.arrayLayers - pCreateInfo->subresourceRange.baseArrayLayer;
            if (view_type == VK_IMAGE_VIEW_TYPE_CUBE && remaining_layers != 6) {
                skip |= LogError(device, "VUID-VkImageViewCreateInfo-viewType-02962",
                                 "vkCreateImageView(): subresourceRange.layerCount VK_REMAINING_ARRAY_LAYERS=(%d) must be 6",
                                 remaining_layers);
            }
            if (view_type == VK_IMAGE_VIEW_TYPE_CUBE_ARRAY && ((remaining_layers) % 6) != 0) {
                skip |= LogError(
                    device, "VUID-VkImageViewCreateInfo-viewType-02963",
                    "vkCreateImageView(): subresourceRange.layerCount VK_REMAINING_ARRAY_LAYERS=(%d) must be a multiple of 6",
                    remaining_layers);
            }
            if ((remaining_layers != 1) && ((view_type == VK_IMAGE_VIEW_TYPE_1D) || (view_type == VK_IMAGE_VIEW_TYPE_2D) ||
                                            (view_type == VK_IMAGE_VIEW_TYPE_3D))) {
                skip |= LogError(pCreateInfo->image, "VUID-VkImageViewCreateInfo-imageViewType-04974",
                                 "vkCreateImageView(): Using pCreateInfo->viewType %s and the subresourceRange.layerCount "
                                 "VK_REMAINING_ARRAY_LAYERS=(%d) and must 1 (try looking into VK_IMAGE_VIEW_TYPE_*_ARRAY).",
                                 string_VkImageViewType(view_type), remaining_layers);
            }
        } else {
            if ((layer_count != 1) && ((view_type == VK_IMAGE_VIEW_TYPE_1D) || (view_type == VK_IMAGE_VIEW_TYPE_2D) ||
                                       (view_type == VK_IMAGE_VIEW_TYPE_3D))) {
                skip |= LogError(pCreateInfo->image, "VUID-VkImageViewCreateInfo-imageViewType-04973",
                                 "vkCreateImageView(): subresourceRange.layerCount (%" PRIu32
                                 ") must be 1 when using viewType %s (try looking into VK_IMAGE_VIEW_TYPE_*_ARRAY).",
                                 layer_count, string_VkImageViewType(view_type));
            }
        }

        if (image_usage & VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT) {
            if (normalized_subresource_range.levelCount != 1) {
                skip |= LogError(pCreateInfo->image, "VUID-VkImageViewCreateInfo-image-02571",
                                 "vkCreateImageView(): If image was created with usage containing "
                                 "VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT, subresourceRange.levelCount (%d) must: be 1",
                                 pCreateInfo->subresourceRange.levelCount);
            }
        }
        if (pCreateInfo->flags & VK_IMAGE_VIEW_CREATE_FRAGMENT_DENSITY_MAP_DYNAMIC_BIT_EXT) {
            if (!enabled_features.fragment_density_map_features.fragmentDensityMapDynamic) {
                skip |= LogError(pCreateInfo->image, "VUID-VkImageViewCreateInfo-flags-02572",
                                 "vkCreateImageView(): If the fragmentDensityMapDynamic feature is not enabled, "
                                 "flags must not contain VK_IMAGE_VIEW_CREATE_FRAGMENT_DENSITY_MAP_DYNAMIC_BIT_EXT");
            }
        } else {
            if (image_usage & VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT) {
                if (image_flags & (VK_IMAGE_CREATE_PROTECTED_BIT | VK_IMAGE_CREATE_SPARSE_BINDING_BIT |
                                   VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT | VK_IMAGE_CREATE_SPARSE_ALIASED_BIT)) {
                    skip |= LogError(pCreateInfo->image, "VUID-VkImageViewCreateInfo-flags-04116",
                                     "vkCreateImageView(): If image was created with usage containing "
                                     "VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT flags must not contain any of "
                                     "VK_IMAGE_CREATE_PROTECTED_BIT, VK_IMAGE_CREATE_SPARSE_BINDING_BIT, "
                                     "VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT, or VK_IMAGE_CREATE_SPARSE_ALIASED_BIT");
                }
            }
        }

        if (image_flags & VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT) {
            if (!FormatIsCompressed(view_format)) {
                if (pCreateInfo->subresourceRange.levelCount != 1) {
                    skip |=
                        LogError(pCreateInfo->image, "VUID-VkImageViewCreateInfo-image-07072",
                                 "vkCreateImageView(): Image was created with VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT bit, "
                                 "and format is not compressed, but subresourcesRange.levelCount (%" PRIu32 ") is not 1.",
                                 pCreateInfo->subresourceRange.levelCount);
                }
                if (pCreateInfo->subresourceRange.layerCount != 1) {
                    skip |=
                        LogError(pCreateInfo->image, "VUID-VkImageViewCreateInfo-image-07072",
                                 "vkCreateImageView(): Image was created with VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT bit, "
                                 "and format is not compressed, but subresourcesRange.layerCount (%" PRIu32 ") is not 1.",
                                 pCreateInfo->subresourceRange.layerCount);
                }
            }

            const bool class_compatible = FormatCompatibilityClass(view_format) == FormatCompatibilityClass(image_format);
            // "uncompressed format that is size-compatible" so if compressed, same as not being compatible
            const bool size_compatible =
                FormatIsCompressed(view_format) ? false : FormatElementSize(view_format) == FormatElementSize(image_format);
            if (!class_compatible && !size_compatible) {
                skip |=
                    LogError(pCreateInfo->image, "VUID-VkImageViewCreateInfo-image-01583",
                             "vkCreateImageView(): Image was created with VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT bit and "
                             "format (%s), but pCreateInfo->format (%s) are not compatible.",
                             string_VkFormat(image_format), string_VkFormat(view_format));
            }
        }

        if (pCreateInfo->flags & VK_IMAGE_VIEW_CREATE_FRAGMENT_DENSITY_MAP_DEFERRED_BIT_EXT) {
            if (!enabled_features.fragment_density_map2_features.fragmentDensityMapDeferred) {
                skip |= LogError(pCreateInfo->image, "VUID-VkImageViewCreateInfo-flags-03567",
                                 "vkCreateImageView(): If the fragmentDensityMapDeferred feature is not enabled, "
                                 "flags must not contain VK_IMAGE_VIEW_CREATE_FRAGMENT_DENSITY_MAP_DEFERRED_BIT_EXT");
            }
            if (pCreateInfo->flags & VK_IMAGE_VIEW_CREATE_FRAGMENT_DENSITY_MAP_DYNAMIC_BIT_EXT) {
                skip |=
                    LogError(pCreateInfo->image, "VUID-VkImageViewCreateInfo-flags-03568",
                             "vkCreateImageView(): If flags contains VK_IMAGE_VIEW_CREATE_FRAGMENT_DENSITY_MAP_DEFERRED_BIT_EXT, "
                             "flags must not contain VK_IMAGE_VIEW_CREATE_FRAGMENT_DENSITY_MAP_DYNAMIC_BIT_EXT");
            }
        }
        if (IsExtEnabled(device_extensions.vk_ext_fragment_density_map2)) {
            if ((image_flags & VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT) && (image_usage & VK_IMAGE_USAGE_SAMPLED_BIT) &&
                (layer_count > phys_dev_ext_props.fragment_density_map2_props.maxSubsampledArrayLayers)) {
                skip |= LogError(pCreateInfo->image, "VUID-VkImageViewCreateInfo-image-03569",
                                 "vkCreateImageView(): If image was created with flags containing "
                                 "VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT and usage containing VK_IMAGE_USAGE_SAMPLED_BIT "
                                 "subresourceRange.layerCount (%d) must: be less than or equal to maxSubsampledArrayLayers (%d)",
                                 layer_count, phys_dev_ext_props.fragment_density_map2_props.maxSubsampledArrayLayers);
            }
        }

        auto astc_decode_mode = LvlFindInChain<VkImageViewASTCDecodeModeEXT>(pCreateInfo->pNext);
        if (IsExtEnabled(device_extensions.vk_ext_astc_decode_mode) && (astc_decode_mode != nullptr)) {
            if ((enabled_features.astc_decode_features.decodeModeSharedExponent == VK_FALSE) &&
                (astc_decode_mode->decodeMode == VK_FORMAT_E5B9G9R9_UFLOAT_PACK32)) {
                skip |= LogError(device, "VUID-VkImageViewASTCDecodeModeEXT-decodeMode-02231",
                                 "vkCreateImageView(): decodeModeSharedExponent is not enabled but "
                                 "VkImageViewASTCDecodeModeEXT::decodeMode is VK_FORMAT_E5B9G9R9_UFLOAT_PACK32.");
            }
        }

        if (IsExtEnabled(device_extensions.vk_khr_portability_subset)) {
            // If swizzling is disabled, make sure it isn't used
            // NOTE: as of spec version 1.2.183, VUID 04465 states: "all elements of components _must_ be
            // VK_COMPONENT_SWIZZLE_IDENTITY."
            //       However, issue https://github.com/KhronosGroup/Vulkan-Portability/issues/27 points out that the identity can
            //       also be defined via R, G, B, A enums in the correct order.
            //       Spec change is at https://gitlab.khronos.org/vulkan/vulkan/-/merge_requests/4600
            if ((VK_FALSE == enabled_features.portability_subset_features.imageViewFormatSwizzle) &&
                !IsIdentitySwizzle(pCreateInfo->components)) {
                skip |= LogError(device, "VUID-VkImageViewCreateInfo-imageViewFormatSwizzle-04465",
                                 "vkCreateImageView (portability error): swizzle is disabled for this device.");
            }

            // Ensure ImageView's format has the same number of bits and components as Image's format if format reinterpretation is
            // disabled
            // TODO (ncesario): This is not correct for some cases (e.g., VK_FORMAT_B10G11R11_UFLOAT_PACK32 and
            // VK_FORMAT_E5B9G9R9_UFLOAT_PACK32), but requires additional information that should probably be generated from the
            // spec. See Github issue #2361.
            if ((VK_FALSE == enabled_features.portability_subset_features.imageViewFormatReinterpretation) &&
                ((FormatElementSize(pCreateInfo->format, VK_IMAGE_ASPECT_COLOR_BIT) !=
                  FormatElementSize(image_state->createInfo.format, VK_IMAGE_ASPECT_COLOR_BIT)) ||
                 (FormatComponentCount(pCreateInfo->format) != FormatComponentCount(image_state->createInfo.format)))) {
                skip |= LogError(device, "VUID-VkImageViewCreateInfo-imageViewFormatReinterpretation-04466",
                                 "vkCreateImageView (portability error): ImageView format must have"
                                 " the same number of components and bits per component as the Image's format");
            }
        }

        auto image_view_min_lod = LvlFindInChain<VkImageViewMinLodCreateInfoEXT>(pCreateInfo->pNext);
        if (image_view_min_lod) {
            if ((!enabled_features.image_view_min_lod_features.minLod) && (image_view_min_lod->minLod != 0)) {
                skip |= LogError(device, "VUID-VkImageViewMinLodCreateInfoEXT-minLod-06455",
                                 "vkCreateImageView(): VkImageViewMinLodCreateInfoEXT::minLod = %f, but the minLod feature is not "
                                 "enabled.  If the minLod feature is not enabled, minLod must be 0.0",
                                 image_view_min_lod->minLod);
            }
            auto max_level =
                static_cast<float>(pCreateInfo->subresourceRange.baseMipLevel + (pCreateInfo->subresourceRange.levelCount - 1));
            if (image_view_min_lod->minLod > max_level) {
                skip |= LogError(device, "VUID-VkImageViewMinLodCreateInfoEXT-minLod-06456",
                                 "vkCreateImageView(): minLod (%f) must be less or equal to the index of the last mipmap level "
                                 "accessible to the view (%f)",
                                 image_view_min_lod->minLod, max_level);
            }
        }

        if (image_usage & VK_IMAGE_USAGE_SAMPLED_BIT && FormatRequiresYcbcrConversionExplicitly(view_format)) {
            const auto ycbcr_conversion = LvlFindInChain<VkSamplerYcbcrConversionInfo>(pCreateInfo->pNext);
            if ((!ycbcr_conversion || ycbcr_conversion->conversion == VK_NULL_HANDLE) &&
                (image_usage & VK_IMAGE_USAGE_SAMPLED_BIT)) {
                skip |= LogError(device, "VUID-VkImageViewCreateInfo-format-06415",
                                 "vkCreateImageView(): When using VK_IMAGE_USAGE_SAMPLED_BIT, YCbCr Format %s requires a "
                                 "VkSamplerYcbcrConversion but one was not passed in the pNext chain.",
                                 string_VkFormat(view_format));
            }
        }

        if (pCreateInfo->viewType != VK_IMAGE_VIEW_TYPE_2D && pCreateInfo->viewType != VK_IMAGE_VIEW_TYPE_2D_ARRAY) {
            VkImageUsageFlags decode_usage = VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR | VK_IMAGE_USAGE_VIDEO_DECODE_SRC_BIT_KHR |
                                             VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR;
            if (image_usage & decode_usage) {
                skip |= LogError(device, "VUID-VkImageViewCreateInfo-image-04817",
                                 "vkCreateImageView(): View type %s is incompatible with decode usage.",
                                 string_VkImageViewType(pCreateInfo->viewType));
            }
        }

        if ((pCreateInfo->flags & VK_IMAGE_VIEW_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT) &&
            !enabled_features.descriptor_buffer_features.descriptorBufferCaptureReplay) {
            skip |= LogError(device, "VUID-VkImageViewCreateInfo-flags-08106",
                             "vkCreateImageView(): the descriptorBufferCaptureReplay device feature is disabled: Image views "
                             "cannot be created with "
                             "the VK_IMAGE_VIEW_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT.");
        }

        auto opaque_capture_descriptor_buffer = LvlFindInChain<VkOpaqueCaptureDescriptorDataCreateInfoEXT>(pCreateInfo->pNext);
        if (opaque_capture_descriptor_buffer &&
            !(pCreateInfo->flags & VK_IMAGE_VIEW_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT)) {
            skip |= LogError(device, "VUID-VkImageViewCreateInfo-pNext-08107",
                             "vkCreateImageView(): VkOpaqueCaptureDescriptorDataCreateInfoEXT is in pNext chain, but "
                             "VK_IMAGE_VIEW_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT is not set.");
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateDestroyImageView(VkDevice device, VkImageView imageView,
                                                 const VkAllocationCallbacks *pAllocator) const {
    auto image_view_state = Get<IMAGE_VIEW_STATE>(imageView);

    bool skip = false;
    if (image_view_state) {
        skip |= ValidateObjectNotInUse(image_view_state.get(), "vkDestroyImageView", "VUID-vkDestroyImageView-imageView-01026");
    }
    return skip;
}
bool CoreChecks::ValidateGetImageSubresourceLayout(VkDevice device, const IMAGE_STATE &image_state,
                                                   const VkImageSubresource &subresource, VkSubresourceLayout &layout,
                                                   bool is_ext) const {
    bool skip = false;
    const char *caller = is_ext ? "vkGetImageSubresourceLayout2EXT()" : "vkGetImageSubresourceLayout()";
    const VkImageAspectFlags aspect_mask = subresource.aspectMask;

    // The aspectMask member of pSubresource must only have a single bit set
    if (GetBitSetCount(aspect_mask) != 1) {
        const char *vuid =
            is_ext ? "VUID-vkGetImageSubresourceLayout2EXT-aspectMask-00997" : "VUID-vkGetImageSubresourceLayout-aspectMask-00997";
        skip |= LogError(image_state.image(), vuid, "%s: VkImageSubresource.aspectMask must have exactly 1 bit set (currently %s).",
                         caller, string_VkImageAspectFlags(aspect_mask).c_str());
    }

    // mipLevel must be less than the mipLevels specified in VkImageCreateInfo when the image was created
    if (subresource.mipLevel >= image_state.createInfo.mipLevels) {
        const char *vuid =
            is_ext ? "VUID-vkGetImageSubresourceLayout2EXT-mipLevel-01716" : "VUID-vkGetImageSubresourceLayout-mipLevel-01716";
        skip |= LogError(image_state.image(), vuid, "%s: pSubresource.mipLevel (%d) must be less than %d.", caller,
                         subresource.mipLevel, image_state.createInfo.mipLevels);
    }

    // arrayLayer must be less than the arrayLayers specified in VkImageCreateInfo when the image was created
    if (subresource.arrayLayer >= image_state.createInfo.arrayLayers) {
        const char *vuid =
            is_ext ? "VUID-vkGetImageSubresourceLayout2EXT-arrayLayer-01717" : "VUID-vkGetImageSubresourceLayout-arrayLayer-01717";
        skip |= LogError(image_state.image(), vuid, "%s: pSubresource.arrayLayer (%d) must be less than %d.", caller,
                         subresource.arrayLayer, image_state.createInfo.arrayLayers);
    }

    const VkFormat image_format = image_state.createInfo.format;
    if (FormatIsColor(image_format) && aspect_mask != VK_IMAGE_ASPECT_COLOR_BIT) {
        const char *vuid =
            is_ext ? "VUID-vkGetImageSubresourceLayout2EXT-format-04461" : "VUID-vkGetImageSubresourceLayout-format-04461";
        skip |= LogError(image_state.image(), vuid,
                         "%s: format of image is %s which is a color format but VkImageSubresource.aspectMask is %s", caller,
                         string_VkFormat(image_format), string_VkImageAspectFlags(aspect_mask).c_str());
    }

    if (FormatHasDepth(image_format) && ((aspect_mask & VK_IMAGE_ASPECT_DEPTH_BIT) == 0)) {
        const char *vuid =
            is_ext ? "VUID-vkGetImageSubresourceLayout2EXT-format-04462" : "VUID-vkGetImageSubresourceLayout-format-04462";
        skip |= LogError(image_state.image(), vuid,
                         "%s: format of image is %s which has depth component "
                         "but VkImageSubresource.aspectMask is %s",
                         caller, string_VkFormat(image_format), string_VkImageAspectFlags(aspect_mask).c_str());
    }

    if (FormatHasStencil(image_format) && ((aspect_mask & VK_IMAGE_ASPECT_STENCIL_BIT) == 0)) {
        const char *vuid =
            is_ext ? "VUID-vkGetImageSubresourceLayout2EXT-format-04463" : "VUID-vkGetImageSubresourceLayout-format-04463";
        skip |= LogError(image_state.image(), vuid,
                         "%s: format of image is %s which which has stencil "
                         "component but VkImageSubresource.aspectMask is %s",
                         caller, string_VkFormat(image_format), string_VkImageAspectFlags(aspect_mask).c_str());
    }

    if (!FormatHasDepth(image_format) && !FormatHasStencil(image_format)) {
        if ((aspect_mask & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) != 0) {
            const char *vuid =
                is_ext ? "VUID-vkGetImageSubresourceLayout2EXT-format-04464" : "VUID-vkGetImageSubresourceLayout-format-04464";
            skip |= LogError(image_state.image(), vuid,
                             "%s: format of image is %s which which does not have depth or stencil"
                             "component but VkImageSubresource.aspectMask is %s",
                             caller, string_VkFormat(image_format), string_VkImageAspectFlags(aspect_mask).c_str());
        }
    }

    // subresource's aspect must be compatible with image's format.
    if (image_state.createInfo.tiling == VK_IMAGE_TILING_LINEAR) {
        if (FormatPlaneCount(image_format) == 2) {
            if ((aspect_mask != VK_IMAGE_ASPECT_PLANE_0_BIT) && (aspect_mask != VK_IMAGE_ASPECT_PLANE_1_BIT)) {
                const char *vuid =
                    is_ext ? "VUID-vkGetImageSubresourceLayout2EXT-format-01581" : "VUID-vkGetImageSubresourceLayout-format-01581";
                skip |= LogError(image_state.image(), vuid,
                                 "%s: plane count of image format (%s) is 2 but VkImageSubresource.aspectMask is %s", caller,
                                 string_VkFormat(image_format), string_VkImageAspectFlags(aspect_mask).c_str());
            }
        }

        if (FormatPlaneCount(image_format) == 3) {
            if ((aspect_mask != VK_IMAGE_ASPECT_PLANE_0_BIT) && (aspect_mask != VK_IMAGE_ASPECT_PLANE_1_BIT) &&
                (aspect_mask != VK_IMAGE_ASPECT_PLANE_2_BIT)) {
                const char *vuid =
                    is_ext ? "VUID-vkGetImageSubresourceLayout2EXT-format-01582" : "VUID-vkGetImageSubresourceLayout-format-01582";
                skip |= LogError(image_state.image(), vuid,
                                 "%s: plane count of image format (%s) is 3 but VkImageSubresource.aspectMask is %s", caller,
                                 string_VkFormat(image_format), string_VkImageAspectFlags(aspect_mask).c_str());
            }
        }
    } else if (image_state.createInfo.tiling == VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT) {
        if ((aspect_mask != VK_IMAGE_ASPECT_MEMORY_PLANE_0_BIT_EXT) && (aspect_mask != VK_IMAGE_ASPECT_MEMORY_PLANE_1_BIT_EXT) &&
            (aspect_mask != VK_IMAGE_ASPECT_MEMORY_PLANE_2_BIT_EXT) && (aspect_mask != VK_IMAGE_ASPECT_MEMORY_PLANE_3_BIT_EXT)) {
            const char *vuid =
                is_ext ? "VUID-vkGetImageSubresourceLayout2EXT-tiling-02271" : "VUID-vkGetImageSubresourceLayout-tiling-02271";
            skip |= LogError(image_state.image(), vuid,
                             "%s: VkImageSubresource.aspectMask (%s) must be VK_IMAGE_ASPECT_MEMORY_PLANE_i_BIT_EXT.", caller,
                             string_VkImageAspectFlags(aspect_mask).c_str());
        } else {
            // Parameter validation should catch if this is used without VK_EXT_image_drm_format_modifier
            assert(IsExtEnabled(device_extensions.vk_ext_image_drm_format_modifier));
            VkImageDrmFormatModifierPropertiesEXT drm_format_properties = LvlInitStruct<VkImageDrmFormatModifierPropertiesEXT>();
            DispatchGetImageDrmFormatModifierPropertiesEXT(device, image_state.image(), &drm_format_properties);

            auto fmt_drm_props = LvlInitStruct<VkDrmFormatModifierPropertiesListEXT>();
            auto fmt_props_2 = LvlInitStruct<VkFormatProperties2>(&fmt_drm_props);
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
                    is_ext ? "VUID-vkGetImageSubresourceLayout2EXT-tiling-02271" : "VUID-vkGetImageSubresourceLayout-tiling-02271";
                skip |= LogError(image_state.image(), vuid,
                                 "%s: VkImageSubresource.aspectMask (%s) must be "
                                 "VK_IMAGE_ASPECT_MEMORY_PLANE_i_BIT_EXT, with i being less than the "
                                 "VkDrmFormatModifierPropertiesEXT::drmFormatModifierPlaneCount (%" PRIu32
                                 ") associated with the image's format (%s) and "
                                 "VkImageDrmFormatModifierPropertiesEXT::drmFormatModifier (%" PRIu64 ").",
                                 caller, string_VkImageAspectFlags(aspect_mask).c_str(), max_plane_count,
                                 string_VkFormat(image_state.createInfo.format), drm_format_properties.drmFormatModifier);
            }
        }
    }

    if (image_state.IsExternalAHB() && image_state.GetBoundMemoryStates().empty()) {
        const char *vuid =
            is_ext ? "VUID-vkGetImageSubresourceLayout2EXT-image-01895" : "VUID-vkGetImageSubresourceLayout-image-01895";
        skip |= LogError(image_state.image(), vuid,
                         "vkGetImageSubresourceLayout(): Attempt to query layout from an image created with "
                         "VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID handleType which has not yet been "
                         "bound to memory.");
    }

    return skip;
}

bool CoreChecks::PreCallValidateGetImageSubresourceLayout(VkDevice device, VkImage image, const VkImageSubresource *pSubresource,
                                                          VkSubresourceLayout *pLayout) const {
    bool skip = false;
    auto image_state = Get<IMAGE_STATE>(image);
    if (pSubresource && pLayout && image_state) {
        skip = ValidateGetImageSubresourceLayout(device, *image_state, *pSubresource, *pLayout, false);

        // Image must have been created with tiling equal to VK_IMAGE_TILING_LINEAR
        if (IsExtEnabled(device_extensions.vk_ext_image_drm_format_modifier)) {
            if ((image_state->createInfo.tiling != VK_IMAGE_TILING_LINEAR) &&
                (image_state->createInfo.tiling != VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT)) {
                skip |= LogError(image, "VUID-vkGetImageSubresourceLayout-image-07790",
                                 "vkGetImageSubresourceLayout(): Image must have tiling of VK_IMAGE_TILING_LINEAR or "
                                 "VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT.");
            }
        } else {
            if (image_state->createInfo.tiling != VK_IMAGE_TILING_LINEAR) {
                skip |= LogError(image, "VUID-vkGetImageSubresourceLayout-image-07789",
                                 "vkGetImageSubresourceLayout(): Image must have tiling of VK_IMAGE_TILING_LINEAR.");
            }
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateGetImageSubresourceLayout2EXT(VkDevice device, VkImage image,
                                                              const VkImageSubresource2EXT *pSubresource,
                                                              VkSubresourceLayout2EXT *pLayout) const

{
    bool skip = false;
    auto image_state = Get<IMAGE_STATE>(image);
    if (pSubresource && pLayout && image_state) {
        skip = ValidateGetImageSubresourceLayout(device, *image_state, pSubresource->imageSubresource, pLayout->subresourceLayout,
                                                 true);
    }
    return skip;
}

// Validates the image is allowed to be protected
bool CoreChecks::ValidateProtectedImage(const CMD_BUFFER_STATE &cb_state, const IMAGE_STATE &image_state, const char *cmd_name,
                                        const char *vuid, const char *more_message) const {
    bool skip = false;

    // if driver supports protectedNoFault the operation is valid, just has undefined values
    if ((!phys_dev_props_core11.protectedNoFault) && (cb_state.unprotected == true) && (image_state.unprotected == false)) {
        const LogObjectList objlist(cb_state.Handle(), image_state.Handle());
        skip |= LogError(objlist, vuid, "%s: command buffer %s is unprotected while image %s is a protected image.%s", cmd_name,
                         report_data->FormatHandle(cb_state.Handle()).c_str(),
                         report_data->FormatHandle(image_state.Handle()).c_str(), more_message);
    }
    return skip;
}

// Validates the image is allowed to be unprotected
bool CoreChecks::ValidateUnprotectedImage(const CMD_BUFFER_STATE &cb_state, const IMAGE_STATE &image_state, const char *cmd_name,
                                          const char *vuid, const char *more_message) const {
    bool skip = false;

    // if driver supports protectedNoFault the operation is valid, just has undefined values
    if ((!phys_dev_props_core11.protectedNoFault) && (cb_state.unprotected == false) && (image_state.unprotected == true)) {
        const LogObjectList objlist(cb_state.Handle(), image_state.Handle());
        skip |= LogError(objlist, vuid, "%s: command buffer %s is protected while image %s is an unprotected image.%s", cmd_name,
                         report_data->FormatHandle(cb_state.Handle()).c_str(),
                         report_data->FormatHandle(image_state.Handle()).c_str(), more_message);
    }
    return skip;
}
