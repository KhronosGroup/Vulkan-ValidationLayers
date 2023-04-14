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

#include "generated/chassis.h"
#include "core_validation.h"

// There is a table in the Vulkan spec to list all formats that implicitly require YCbCr conversion,
// but some features/extensions can explicitly turn that restriction off
// The implicit check is done in format utils, while feature checks are done here in CoreChecks
bool CoreChecks::FormatRequiresYcbcrConversionExplicitly(const VkFormat format) const {
    // VK_EXT_rgba10x6_formats
    if (format == VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16 &&
        enabled_features.rgba10x6_formats_features.formatRgba10x6WithoutYCbCrSampler) {
        return false;
    }
    return FormatRequiresYcbcrConversion(format);
}

bool CoreChecks::ValidateCreateSamplerYcbcrConversion(const char *func_name,
                                                      const VkSamplerYcbcrConversionCreateInfo *create_info) const {
    bool skip = false;
    const VkFormat conversion_format = create_info->format;

    // Need to check for external format conversion first as it allows for non-UNORM format
    bool external_format = false;
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    const VkExternalFormatANDROID *ext_format_android = LvlFindInChain<VkExternalFormatANDROID>(create_info->pNext);
    if ((nullptr != ext_format_android) && (0 != ext_format_android->externalFormat)) {
        external_format = true;
        if (VK_FORMAT_UNDEFINED != create_info->format) {
            return LogError(device, "VUID-VkSamplerYcbcrConversionCreateInfo-format-01904",
                            "%s: CreateInfo format is not VK_FORMAT_UNDEFINED while "
                            "there is a chained VkExternalFormatANDROID struct with a non-zero externalFormat.",
                            func_name);
        }
    }
#endif  // VK_USE_PLATFORM_ANDROID_KHR

    if ((external_format == false) && (FormatIsUNORM(conversion_format) == false)) {
        const char *vuid = IsExtEnabled(device_extensions.vk_android_external_memory_android_hardware_buffer)
                               ? "VUID-VkSamplerYcbcrConversionCreateInfo-format-04061"
                               : "VUID-VkSamplerYcbcrConversionCreateInfo-format-04060";
        skip |=
            LogError(device, vuid,
                     "%s: CreateInfo format (%s) is not an UNORM format and there is no external format conversion being created.",
                     func_name, string_VkFormat(conversion_format));
    }

    // Gets VkFormatFeatureFlags according to Sampler Ycbcr Conversion Format Features
    // (vkspec.html#potential-format-features)
    VkFormatFeatureFlags2KHR format_features = ~0ULL;
    if (conversion_format == VK_FORMAT_UNDEFINED) {
#ifdef VK_USE_PLATFORM_ANDROID_KHR
        // only check for external format inside VK_FORMAT_UNDEFINED check to prevent unnecessary extra errors from no format
        // features being supported
        if (external_format == true) {
            auto it = ahb_ext_formats_map.find(ext_format_android->externalFormat);
            if (it != ahb_ext_formats_map.end()) {
                format_features = it->second;
            }
        }
#endif  // VK_USE_PLATFORM_ANDROID_KHR
    } else {
        format_features = GetPotentialFormatFeatures(conversion_format);
    }

    // Check all VUID that are based off of VkFormatFeatureFlags
    // These can't be in StatelessValidation due to needing possible External AHB state for feature support
    if (((format_features & VK_FORMAT_FEATURE_2_MIDPOINT_CHROMA_SAMPLES_BIT_KHR) == 0) &&
        ((format_features & VK_FORMAT_FEATURE_2_COSITED_CHROMA_SAMPLES_BIT_KHR) == 0)) {
        skip |= LogError(device, "VUID-VkSamplerYcbcrConversionCreateInfo-format-01650",
                         "%s: Format %s does not support either VK_FORMAT_FEATURE_MIDPOINT_CHROMA_SAMPLES_BIT or "
                         "VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT",
                         func_name, string_VkFormat(conversion_format));
    }
    if ((format_features & VK_FORMAT_FEATURE_2_COSITED_CHROMA_SAMPLES_BIT_KHR) == 0) {
        if (FormatIsXChromaSubsampled(conversion_format) && create_info->xChromaOffset == VK_CHROMA_LOCATION_COSITED_EVEN) {
            skip |= LogError(device, "VUID-VkSamplerYcbcrConversionCreateInfo-xChromaOffset-01651",
                             "%s: Format %s does not support VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT so xChromaOffset can't "
                             "be VK_CHROMA_LOCATION_COSITED_EVEN",
                             func_name, string_VkFormat(conversion_format));
        }
        if (FormatIsYChromaSubsampled(conversion_format) && create_info->yChromaOffset == VK_CHROMA_LOCATION_COSITED_EVEN) {
            skip |= LogError(device, "VUID-VkSamplerYcbcrConversionCreateInfo-xChromaOffset-01651",
                             "%s: Format %s does not support VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT so yChromaOffset can't "
                             "be VK_CHROMA_LOCATION_COSITED_EVEN",
                             func_name, string_VkFormat(conversion_format));
        }
    }
    if ((format_features & VK_FORMAT_FEATURE_2_MIDPOINT_CHROMA_SAMPLES_BIT_KHR) == 0) {
        if (FormatIsXChromaSubsampled(conversion_format) && create_info->xChromaOffset == VK_CHROMA_LOCATION_MIDPOINT) {
            skip |= LogError(device, "VUID-VkSamplerYcbcrConversionCreateInfo-xChromaOffset-01652",
                             "%s: Format %s does not support VK_FORMAT_FEATURE_MIDPOINT_CHROMA_SAMPLES_BIT so xChromaOffset can't "
                             "be VK_CHROMA_LOCATION_MIDPOINT",
                             func_name, string_VkFormat(conversion_format));
        }
        if (FormatIsYChromaSubsampled(conversion_format) && create_info->yChromaOffset == VK_CHROMA_LOCATION_MIDPOINT) {
            skip |= LogError(device, "VUID-VkSamplerYcbcrConversionCreateInfo-xChromaOffset-01652",
                             "%s: Format %s does not support VK_FORMAT_FEATURE_MIDPOINT_CHROMA_SAMPLES_BIT so yChromaOffset can't "
                             "be VK_CHROMA_LOCATION_MIDPOINT",
                             func_name, string_VkFormat(conversion_format));
        }
    }
    if (((format_features & VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_FORCEABLE_BIT_KHR) ==
         0) &&
        (create_info->forceExplicitReconstruction == VK_TRUE)) {
        skip |= LogError(device, "VUID-VkSamplerYcbcrConversionCreateInfo-forceExplicitReconstruction-01656",
                         "%s: Format %s does not support "
                         "VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_FORCEABLE_BIT so "
                         "forceExplicitReconstruction must be VK_FALSE",
                         func_name, string_VkFormat(conversion_format));
    }
    if (((format_features & VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT_KHR) == 0) &&
        (create_info->chromaFilter == VK_FILTER_LINEAR)) {
        skip |= LogError(device, "VUID-VkSamplerYcbcrConversionCreateInfo-chromaFilter-01657",
                         "%s: Format %s does not support VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT so "
                         "chromaFilter must not be VK_FILTER_LINEAR",
                         func_name, string_VkFormat(conversion_format));
    }

    return skip;
}

bool CoreChecks::PreCallValidateCreateSamplerYcbcrConversion(VkDevice device, const VkSamplerYcbcrConversionCreateInfo *pCreateInfo,
                                                             const VkAllocationCallbacks *pAllocator,
                                                             VkSamplerYcbcrConversion *pYcbcrConversion) const {
    return ValidateCreateSamplerYcbcrConversion("vkCreateSamplerYcbcrConversion()", pCreateInfo);
}

bool CoreChecks::PreCallValidateCreateSamplerYcbcrConversionKHR(VkDevice device,
                                                                const VkSamplerYcbcrConversionCreateInfo *pCreateInfo,
                                                                const VkAllocationCallbacks *pAllocator,
                                                                VkSamplerYcbcrConversion *pYcbcrConversion) const {
    return ValidateCreateSamplerYcbcrConversion("vkCreateSamplerYcbcrConversionKHR()", pCreateInfo);
}
