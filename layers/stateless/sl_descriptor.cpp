/* Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (C) 2015-2023 Google Inc.
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

#include "stateless/stateless_validation.h"
#include "generated/enum_flag_bits.h"

bool StatelessValidation::ValidateCoarseSampleOrderCustomNV(const VkCoarseSampleOrderCustomNV *order,
                                                            const Location &order_loc) const {
    bool skip = false;

    struct SampleOrderInfo {
        VkShadingRatePaletteEntryNV shadingRate;
        uint32_t width;
        uint32_t height;
    };

    // All palette entries with more than one pixel per fragment
    constexpr std::array sample_order_infos = {
        SampleOrderInfo{VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_1X2_PIXELS_NV, 1, 2},
        SampleOrderInfo{VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_2X1_PIXELS_NV, 2, 1},
        SampleOrderInfo{VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_2X2_PIXELS_NV, 2, 2},
        SampleOrderInfo{VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_4X2_PIXELS_NV, 4, 2},
        SampleOrderInfo{VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_2X4_PIXELS_NV, 2, 4},
        SampleOrderInfo{VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_4X4_PIXELS_NV, 4, 4},
    };

    const SampleOrderInfo *sample_order_info;
    uint32_t info_idx = 0;
    for (sample_order_info = nullptr; info_idx < sample_order_infos.size(); ++info_idx) {
        if (sample_order_infos[info_idx].shadingRate == order->shadingRate) {
            sample_order_info = &sample_order_infos[info_idx];
            break;
        }
    }

    if (sample_order_info == nullptr) {
        skip |= LogError("VUID-VkCoarseSampleOrderCustomNV-shadingRate-02073", device, order_loc,
                         "shadingRate must be a shading rate "
                         "that generates fragments with more than one pixel.");
        return skip;
    }

    if (order->sampleCount == 0 || (order->sampleCount & (order->sampleCount - 1)) ||
        !(order->sampleCount & device_limits.framebufferNoAttachmentsSampleCounts)) {
        skip |= LogError("VUID-VkCoarseSampleOrderCustomNV-sampleCount-02074", device, order_loc,
                         "sampleCount (=%" PRIu32
                         ") must "
                         "correspond to a sample count enumerated in VkSampleCountFlags whose corresponding bit "
                         "is set in framebufferNoAttachmentsSampleCounts.",
                         order->sampleCount);
    }

    if (order->sampleLocationCount != order->sampleCount * sample_order_info->width * sample_order_info->height) {
        skip |= LogError("VUID-VkCoarseSampleOrderCustomNV-sampleLocationCount-02075", device, order_loc,
                         "sampleLocationCount (=%" PRIu32
                         ") must "
                         "be equal to the product of sampleCount (=%" PRIu32
                         "), the fragment width for shadingRate "
                         "(=%" PRIu32 "), and the fragment height for shadingRate (=%" PRIu32 ").",
                         order->sampleLocationCount, order->sampleCount, sample_order_info->width, sample_order_info->height);
    }

    if (order->sampleLocationCount > phys_dev_ext_props.shading_rate_image_props.shadingRateMaxCoarseSamples) {
        skip |= LogError(
            "VUID-VkCoarseSampleOrderCustomNV-sampleLocationCount-02076", device, order_loc,
            "sampleLocationCount (=%" PRIu32
            ") must "
            "be less than or equal to VkPhysicalDeviceShadingRateImagePropertiesNV shadingRateMaxCoarseSamples (=%" PRIu32 ").",
            order->sampleLocationCount, phys_dev_ext_props.shading_rate_image_props.shadingRateMaxCoarseSamples);
    }

    // Accumulate a bitmask tracking which (x,y,sample) tuples are seen. Expect
    // the first width*height*sampleCount bits to all be set. Note: There is no
    // guarantee that 64 bits is enough, but practically it's unlikely for an
    // implementation to support more than 32 bits for samplemask.
    assert(phys_dev_ext_props.shading_rate_image_props.shadingRateMaxCoarseSamples <= 64);
    uint64_t sample_locations_mask = 0;
    for (uint32_t i = 0; i < order->sampleLocationCount; ++i) {
        const VkCoarseSampleLocationNV *sample_loc = &order->pSampleLocations[i];
        if (sample_loc->pixelX >= sample_order_info->width) {
            skip |= LogError("VUID-VkCoarseSampleLocationNV-pixelX-02078", device, order_loc,
                             "pixelX must be less than the width (in pixels) of the fragment.");
        }
        if (sample_loc->pixelY >= sample_order_info->height) {
            skip |= LogError("VUID-VkCoarseSampleLocationNV-pixelY-02079", device, order_loc,
                             "pixelY must be less than the height (in pixels) of the fragment.");
        }
        if (sample_loc->sample >= order->sampleCount) {
            skip |= LogError("VUID-VkCoarseSampleLocationNV-sample-02080", device, order_loc,
                             "sample must be less than the number of coverage samples in each pixel belonging to the fragment.");
        }
        uint32_t idx =
            sample_loc->sample + order->sampleCount * (sample_loc->pixelX + sample_order_info->width * sample_loc->pixelY);
        sample_locations_mask |= 1ULL << idx;
    }

    uint64_t expected_mask = (order->sampleLocationCount == 64) ? ~0ULL : ((1ULL << order->sampleLocationCount) - 1);
    if (sample_locations_mask != expected_mask) {
        skip |= LogError(
            "VUID-VkCoarseSampleOrderCustomNV-pSampleLocations-02077", device, order_loc,
            "The array pSampleLocations must contain exactly one entry for "
            "every combination of valid values for pixelX, pixelY, and sample in the structure VkCoarseSampleOrderCustomNV.");
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCreateSampler(VkDevice device, const VkSamplerCreateInfo *pCreateInfo,
                                                              const VkAllocationCallbacks *pAllocator, VkSampler *pSampler,
                                                              const ErrorObject &error_obj) const {
    bool skip = false;

    if (pCreateInfo == nullptr) {
        return skip;
    }
    const Location create_info_loc = error_obj.location.dot(Field::pCreateInfo);
    const auto &features = physical_device_features;
    const auto &limits = device_limits;

    if (pCreateInfo->anisotropyEnable == VK_TRUE) {
        if (!IsBetweenInclusive(pCreateInfo->maxAnisotropy, 1.0F, limits.maxSamplerAnisotropy)) {
            skip |= LogError("VUID-VkSamplerCreateInfo-anisotropyEnable-01071", device, create_info_loc.dot(Field::maxAnisotropy),
                             "is %f but must be in the range of [1.0, %f] (maxSamplerAnistropy).", pCreateInfo->maxAnisotropy,
                             limits.maxSamplerAnisotropy);
        }

        // Anistropy cannot be enabled in sampler unless enabled as a feature
        if (features.samplerAnisotropy == VK_FALSE) {
            skip |=
                LogError("VUID-VkSamplerCreateInfo-anisotropyEnable-01070", device, create_info_loc.dot(Field::anisotropyEnable),
                         "is VK_TRUE but the samplerAnisotropy feature was not enabled.");
        }
    }

    if (pCreateInfo->unnormalizedCoordinates == VK_TRUE) {
        if (pCreateInfo->minFilter != pCreateInfo->magFilter) {
            skip |= LogError("VUID-VkSamplerCreateInfo-unnormalizedCoordinates-01072", device,
                             create_info_loc.dot(Field::unnormalizedCoordinates),
                             "is VK_TRUE, but minFilter (%s) is different then magFilter (%s).",
                             string_VkFilter(pCreateInfo->minFilter), string_VkFilter(pCreateInfo->magFilter));
        }
        if (pCreateInfo->mipmapMode != VK_SAMPLER_MIPMAP_MODE_NEAREST) {
            skip |= LogError("VUID-VkSamplerCreateInfo-unnormalizedCoordinates-01073", device,
                             create_info_loc.dot(Field::unnormalizedCoordinates),
                             "is VK_TRUE, but mipmapMode (%s) must be VK_SAMPLER_MIPMAP_MODE_NEAREST.",
                             string_VkSamplerMipmapMode(pCreateInfo->mipmapMode));
        }
        if (pCreateInfo->minLod != 0.0f || pCreateInfo->maxLod != 0.0f) {
            skip |= LogError("VUID-VkSamplerCreateInfo-unnormalizedCoordinates-01074", device,
                             create_info_loc.dot(Field::unnormalizedCoordinates),
                             "is VK_TRUE, but minLod (%f) and maxLod (%f) must both be zero.", pCreateInfo->minLod,
                             pCreateInfo->maxLod);
        }
        if ((pCreateInfo->addressModeU != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE &&
             pCreateInfo->addressModeU != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER) ||
            (pCreateInfo->addressModeV != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE &&
             pCreateInfo->addressModeV != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER)) {
            skip |= LogError("VUID-VkSamplerCreateInfo-unnormalizedCoordinates-01075", device,
                             create_info_loc.dot(Field::unnormalizedCoordinates),
                             "is VK_TRUE, but addressModeU (%s) and addressModeV (%s) must both be "
                             "CLAMP_TO_EDGE or CLAMP_TO_BORDER.",
                             string_VkSamplerAddressMode(pCreateInfo->addressModeU),
                             string_VkSamplerAddressMode(pCreateInfo->addressModeV));
        }
        if (pCreateInfo->anisotropyEnable == VK_TRUE) {
            skip |= LogError("VUID-VkSamplerCreateInfo-unnormalizedCoordinates-01076", device, create_info_loc,
                             "anisotropyEnable and unnormalizedCoordinates are both VK_TRUE.");
        }
        if (pCreateInfo->compareEnable == VK_TRUE) {
            skip |= LogError("VUID-VkSamplerCreateInfo-unnormalizedCoordinates-01077", device, create_info_loc,
                             "compareEnable and unnormalizedCoordinates are both VK_TRUE.");
        }
    }

    // If compareEnable is VK_TRUE, compareOp must be a valid VkCompareOp value
    const auto *sampler_reduction = vku::FindStructInPNextChain<VkSamplerReductionModeCreateInfo>(pCreateInfo->pNext);
    if (pCreateInfo->compareEnable == VK_TRUE) {
        skip |= ValidateRangedEnum(create_info_loc.dot(Field::compareOp), "VkCompareOp", pCreateInfo->compareOp,
                                   "VUID-VkSamplerCreateInfo-compareEnable-01080");
        if (sampler_reduction != nullptr) {
            if (sampler_reduction->reductionMode != VK_SAMPLER_REDUCTION_MODE_WEIGHTED_AVERAGE) {
                skip |= LogError("VUID-VkSamplerCreateInfo-compareEnable-01423", device,
                                 create_info_loc.pNext(Struct::VkSamplerReductionModeCreateInfo, Field::reductionMode),
                                 "is %s but compareEnable is VK_TRUE.",
                                 string_VkSamplerReductionMode(sampler_reduction->reductionMode));
            }
        }
    }
    if (sampler_reduction && sampler_reduction->reductionMode != VK_SAMPLER_REDUCTION_MODE_WEIGHTED_AVERAGE) {
        if (!IsExtEnabled(device_extensions.vk_ext_filter_cubic)) {
            if (pCreateInfo->magFilter == VK_FILTER_CUBIC_EXT || pCreateInfo->minFilter == VK_FILTER_CUBIC_EXT) {
                skip |= LogError("VUID-VkSamplerCreateInfo-magFilter-07911", device,
                                 create_info_loc.pNext(Struct::VkSamplerReductionModeCreateInfo, Field::reductionMode),
                                 "is %s, magFilter is %s and minFilter is %s, but "
                                 "extension %s is not enabled.",
                                 string_VkSamplerReductionMode(sampler_reduction->reductionMode),
                                 string_VkFilter(pCreateInfo->magFilter), string_VkFilter(pCreateInfo->minFilter),
                                 VK_EXT_FILTER_CUBIC_EXTENSION_NAME);
            }
        }
    }

    // If any of addressModeU, addressModeV or addressModeW are VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, borderColor must be a
    // valid VkBorderColor value
    if ((pCreateInfo->addressModeU == VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER) ||
        (pCreateInfo->addressModeV == VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER) ||
        (pCreateInfo->addressModeW == VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER)) {
        skip |= ValidateRangedEnum(create_info_loc.dot(Field::borderColor), "VkBorderColor", pCreateInfo->borderColor,
                                   "VUID-VkSamplerCreateInfo-addressModeU-01078");
    }

    // Checks for the IMG cubic filtering extension
    if (IsExtEnabled(device_extensions.vk_img_filter_cubic)) {
        if ((pCreateInfo->anisotropyEnable == VK_TRUE) &&
            ((pCreateInfo->minFilter == VK_FILTER_CUBIC_IMG) || (pCreateInfo->magFilter == VK_FILTER_CUBIC_IMG))) {
            skip |= LogError("VUID-VkSamplerCreateInfo-magFilter-01081", device, create_info_loc,
                             "anisotropyEnable is VK_TRUE, but minFilter = %s and magFilter = %s",
                             string_VkFilter(pCreateInfo->minFilter), string_VkFilter(pCreateInfo->magFilter));
        }
    }

    // Check for valid Lod range
    if (pCreateInfo->minLod > pCreateInfo->maxLod) {
        skip |= LogError("VUID-VkSamplerCreateInfo-maxLod-01973", device, create_info_loc,
                         "minLod (%f) is greater than maxLod (%f)", pCreateInfo->minLod, pCreateInfo->maxLod);
    }

    // Check mipLodBias to device limit
    if (pCreateInfo->mipLodBias > limits.maxSamplerLodBias) {
        skip |= LogError("VUID-VkSamplerCreateInfo-mipLodBias-01069", device, create_info_loc.dot(Field::mipLodBias),
                         "(%f) is greater than maxSamplerLodBias (%f)", pCreateInfo->mipLodBias, limits.maxSamplerLodBias);
    }

    const auto *sampler_conversion = vku::FindStructInPNextChain<VkSamplerYcbcrConversionInfo>(pCreateInfo->pNext);
    if (sampler_conversion != nullptr) {
        if ((pCreateInfo->addressModeU != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE) ||
            (pCreateInfo->addressModeV != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE) ||
            (pCreateInfo->addressModeW != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE) || (pCreateInfo->anisotropyEnable != VK_FALSE) ||
            (pCreateInfo->unnormalizedCoordinates != VK_FALSE)) {
            skip |= LogError(
                "VUID-VkSamplerCreateInfo-addressModeU-01646", device, create_info_loc,
                "vkCreateSampler():  SamplerYCbCrConversion is enabled: "
                "addressModeU (%s), addressModeV (%s), addressModeW (%s) must be CLAMP_TO_EDGE, and anisotropyEnable (%s) "
                "and unnormalizedCoordinates (%s) must be VK_FALSE.",
                string_VkSamplerAddressMode(pCreateInfo->addressModeU), string_VkSamplerAddressMode(pCreateInfo->addressModeV),
                string_VkSamplerAddressMode(pCreateInfo->addressModeW), pCreateInfo->anisotropyEnable ? "VK_TRUE" : "VK_FALSE",
                pCreateInfo->unnormalizedCoordinates ? "VK_TRUE" : "VK_FALSE");
        }
    }

    if (pCreateInfo->flags & VK_SAMPLER_CREATE_SUBSAMPLED_BIT_EXT) {
        if (pCreateInfo->minFilter != pCreateInfo->magFilter) {
            skip |= LogError("VUID-VkSamplerCreateInfo-flags-02574", device, create_info_loc.dot(Field::flags),
                             "includes VK_SAMPLER_CREATE_SUBSAMPLED_BIT_EXT, but "
                             "minFilter (%s) and magFilter (%s) must be equal.",
                             string_VkFilter(pCreateInfo->minFilter), string_VkFilter(pCreateInfo->magFilter));
        }
        if (pCreateInfo->mipmapMode != VK_SAMPLER_MIPMAP_MODE_NEAREST) {
            skip |= LogError("VUID-VkSamplerCreateInfo-flags-02575", device, create_info_loc.dot(Field::flags),
                             "includes VK_SAMPLER_CREATE_SUBSAMPLED_BIT_EXT, but "
                             "mipmapMode (%s) must be VK_SAMPLER_MIPMAP_MODE_NEAREST.",
                             string_VkSamplerMipmapMode(pCreateInfo->mipmapMode));
        }
        if (pCreateInfo->minLod != 0.0 || pCreateInfo->maxLod != 0.0) {
            skip |= LogError("VUID-VkSamplerCreateInfo-flags-02576", device, create_info_loc.dot(Field::flags),
                             "includes VK_SAMPLER_CREATE_SUBSAMPLED_BIT_EXT, but "
                             "minLod (%f) and maxLod (%f) must be zero.",
                             pCreateInfo->minLod, pCreateInfo->maxLod);
        }
        if (((pCreateInfo->addressModeU != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE) &&
             (pCreateInfo->addressModeU != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER)) ||
            ((pCreateInfo->addressModeV != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE) &&
             (pCreateInfo->addressModeV != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER))) {
            skip |= LogError("VUID-VkSamplerCreateInfo-flags-02577", device, create_info_loc.dot(Field::flags),
                             "includes VK_SAMPLER_CREATE_SUBSAMPLED_BIT_EXT, "
                             "addressModeU (%s) and addressModeV (%s) must be "
                             "CLAMP_TO_EDGE or CLAMP_TO_BORDER",
                             string_VkSamplerAddressMode(pCreateInfo->addressModeU),
                             string_VkSamplerAddressMode(pCreateInfo->addressModeV));
        }
        if (pCreateInfo->anisotropyEnable) {
            skip |= LogError("VUID-VkSamplerCreateInfo-flags-02578", device, create_info_loc.dot(Field::flags),
                             "includes VK_SAMPLER_CREATE_SUBSAMPLED_BIT_EXT, "
                             "but anisotropyEnable is VK_TRUE.");
        }
        if (pCreateInfo->compareEnable) {
            skip |= LogError("VUID-VkSamplerCreateInfo-flags-02579", device, create_info_loc.dot(Field::flags),
                             "includes VK_SAMPLER_CREATE_SUBSAMPLED_BIT_EXT, "
                             "but compareEnable is VK_TRUE.");
        }
        if (pCreateInfo->unnormalizedCoordinates) {
            skip |= LogError("VUID-VkSamplerCreateInfo-flags-02580", device, create_info_loc.dot(Field::flags),
                             "includes VK_SAMPLER_CREATE_SUBSAMPLED_BIT_EXT, "
                             "but unnormalizedCoordinates is VK_TRUE.");
        }
    }

    if (pCreateInfo->borderColor == VK_BORDER_COLOR_INT_CUSTOM_EXT ||
        pCreateInfo->borderColor == VK_BORDER_COLOR_FLOAT_CUSTOM_EXT) {
        if (!IsExtEnabled(device_extensions.vk_ext_custom_border_color)) {
            skip |= LogError(kVUID_PVError_ExtensionNotEnabled, device, create_info_loc.dot(Field::borderColor),
                             "is %s but %s is not enabled.", string_VkBorderColor(pCreateInfo->borderColor),
                             VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME);
        }
        auto custom_create_info = vku::FindStructInPNextChain<VkSamplerCustomBorderColorCreateInfoEXT>(pCreateInfo->pNext);
        if (!custom_create_info) {
            skip |= LogError("VUID-VkSamplerCreateInfo-borderColor-04011", device, create_info_loc.dot(Field::borderColor),
                             "is %s but there is no VkSamplerCustomBorderColorCreateInfoEXT "
                             "struct in pNext chain.",
                             string_VkBorderColor(pCreateInfo->borderColor));
        } else {
            if ((custom_create_info->format != VK_FORMAT_UNDEFINED) && !vkuFormatIsDepthAndStencil(custom_create_info->format) &&
                ((pCreateInfo->borderColor == VK_BORDER_COLOR_INT_CUSTOM_EXT && !vkuFormatIsSampledInt(custom_create_info->format)) ||
                 (pCreateInfo->borderColor == VK_BORDER_COLOR_FLOAT_CUSTOM_EXT &&
                  !vkuFormatIsSampledFloat(custom_create_info->format)))) {
                skip |= LogError("VUID-VkSamplerCustomBorderColorCreateInfoEXT-format-07605", device,
                                 create_info_loc.pNext(Struct::VkSamplerCustomBorderColorCreateInfoEXT, Field::format),
                                 "%s does not match borderColor (%s).", string_VkFormat(custom_create_info->format),
                                 string_VkBorderColor(pCreateInfo->borderColor));
            }
        }
    }

    const auto *border_color_component_mapping =
        vku::FindStructInPNextChain<VkSamplerBorderColorComponentMappingCreateInfoEXT>(pCreateInfo->pNext);
    if (border_color_component_mapping) {
        const auto *border_color_swizzle_features =
            vku::FindStructInPNextChain<VkPhysicalDeviceBorderColorSwizzleFeaturesEXT>(device_createinfo_pnext);
        bool border_color_swizzle_features_enabled =
            border_color_swizzle_features && border_color_swizzle_features->borderColorSwizzle;
        if (!border_color_swizzle_features_enabled) {
            skip |=
                LogError("VUID-VkSamplerBorderColorComponentMappingCreateInfoEXT-borderColorSwizzle-06437", device, create_info_loc,
                         "The borderColorSwizzle feature must be enabled to use "
                         "VkPhysicalDeviceBorderColorSwizzleFeaturesEXT");
        }
    }

    // VK_QCOM_image_processing
    if ((pCreateInfo->flags & VK_SAMPLER_CREATE_IMAGE_PROCESSING_BIT_QCOM) != 0) {
        if ((pCreateInfo->minFilter != VK_FILTER_NEAREST) || (pCreateInfo->magFilter != VK_FILTER_NEAREST)) {
            skip |= LogError("VUID-VkSamplerCreateInfo-flags-06964", device, create_info_loc.dot(Field::flags),
                             "includes VK_SAMPLER_CREATE_IMAGE_PROCESSING_BIT_QCOM, "
                             "minFilter (%s) must be VK_FILTER_NEAREST and "
                             "magFilter (%s) must be VK_FILTER_NEAREST.",
                             string_VkFilter(pCreateInfo->minFilter), string_VkFilter(pCreateInfo->magFilter));
        }
        if (pCreateInfo->mipmapMode != VK_SAMPLER_MIPMAP_MODE_NEAREST) {
            skip |= LogError("VUID-VkSamplerCreateInfo-flags-06965", device, create_info_loc.dot(Field::flags),
                             "includes VK_SAMPLER_CREATE_IMAGE_PROCESSING_BIT_QCOM, "
                             "mipmapMode (%s) must be VK_SAMPLER_MIPMAP_MODE_NEAREST.",
                             string_VkSamplerMipmapMode(pCreateInfo->mipmapMode));
        }
        if ((pCreateInfo->minLod != 0) || (pCreateInfo->maxLod != 0)) {
            skip |= LogError("VUID-VkSamplerCreateInfo-flags-06966", device, create_info_loc.dot(Field::flags),
                             "includes VK_SAMPLER_CREATE_IMAGE_PROCESSING_BIT_QCOM, "
                             "minLod (%f) and maxLod (%f) must be 0.",
                             pCreateInfo->minLod, pCreateInfo->maxLod);
        }
        if (((pCreateInfo->addressModeU != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE) &&
             (pCreateInfo->addressModeU != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER)) ||
            ((pCreateInfo->addressModeV != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE) &&
             (pCreateInfo->addressModeV != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER))) {
            skip |= LogError("VUID-VkSamplerCreateInfo-flags-06967", device, create_info_loc.dot(Field::flags),
                             "includes VK_SAMPLER_CREATE_IMAGE_PROCESSING_BIT_QCOM, "
                             "addressModeU (%s) and addressModeV (%s) must be either "
                             "VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE or VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER.",
                             string_VkSamplerAddressMode(pCreateInfo->addressModeU),
                             string_VkSamplerAddressMode(pCreateInfo->addressModeV));
        }
        if (((pCreateInfo->addressModeU == VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER) ||
             (pCreateInfo->addressModeV == VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER)) &&
            (pCreateInfo->borderColor != VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK)) {
            skip |=
                LogError("VUID-VkSamplerCreateInfo-flags-06968", device, create_info_loc.dot(Field::flags),
                         "includes VK_SAMPLER_CREATE_IMAGE_PROCESSING_BIT_QCOM, "
                         "and if addressModeU (%s) or addressModeV (%s) are "
                         "VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, then"
                         "borderColor (%s) must be VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK.",
                         string_VkSamplerAddressMode(pCreateInfo->addressModeU),
                         string_VkSamplerAddressMode(pCreateInfo->addressModeV), string_VkBorderColor(pCreateInfo->borderColor));
        }
        if (pCreateInfo->anisotropyEnable) {
            skip |= LogError("VUID-VkSamplerCreateInfo-flags-06969", device, create_info_loc.dot(Field::flags),
                             "includes VK_SAMPLER_CREATE_IMAGE_PROCESSING_BIT_QCOM, "
                             "but anisotropyEnable is VK_TRUE.");
        }
        if (pCreateInfo->compareEnable) {
            skip |= LogError("VUID-VkSamplerCreateInfo-flags-06970", device, create_info_loc.dot(Field::flags),
                             "includes VK_SAMPLER_CREATE_IMAGE_PROCESSING_BIT_QCOM, "
                             "but compareEnable is VK_TRUE.");
        }
    }

    return skip;
}

bool StatelessValidation::ValidateMutableDescriptorTypeCreateInfo(const VkDescriptorSetLayoutCreateInfo &create_info,
                                                                  const VkMutableDescriptorTypeCreateInfoEXT &mutable_create_info,
                                                                  const Location &loc) const {
    bool skip = false;

    for (uint32_t i = 0; i < create_info.bindingCount; ++i) {
        const Location binding_loc = loc.dot(Field::pBindings, i);
        uint32_t mutable_type_count = 0;
        if (mutable_create_info.mutableDescriptorTypeListCount > i) {
            mutable_type_count = mutable_create_info.pMutableDescriptorTypeLists[i].descriptorTypeCount;
        }
        if (create_info.pBindings[i].descriptorType == VK_DESCRIPTOR_TYPE_MUTABLE_EXT) {
            if (mutable_type_count == 0) {
                skip |= LogError(
                    "VUID-VkMutableDescriptorTypeListEXT-descriptorTypeCount-04597", device, binding_loc.dot(Field::descriptorType),
                    "is VK_DESCRIPTOR_TYPE_MUTABLE_EXT, but "
                    "VkMutableDescriptorTypeCreateInfoEXT::pMutableDescriptorTypeLists[%" PRIu32 "].descriptorTypeCount is 0.",
                    i);
            }
        } else {
            if (mutable_type_count > 0) {
                skip |= LogError(
                    "VUID-VkMutableDescriptorTypeListEXT-descriptorTypeCount-04599", device, binding_loc.dot(Field::descriptorType),
                    "is %s, but "
                    "VkMutableDescriptorTypeCreateInfoEXT::pMutableDescriptorTypeLists[%" PRIu32 "].descriptorTypeCount is not 0.",
                    string_VkDescriptorType(create_info.pBindings[i].descriptorType), i);
            }
        }
    }

    for (uint32_t j = 0; j < mutable_create_info.mutableDescriptorTypeListCount; ++j) {
        const Location mutable_loc = loc.pNext(Struct::VkMutableDescriptorTypeCreateInfoEXT, Field::pMutableDescriptorTypeLists, j);
        for (uint32_t k = 0; k < mutable_create_info.pMutableDescriptorTypeLists[j].descriptorTypeCount; ++k) {
            const Location type_loc = mutable_loc.dot(Field::pDescriptorTypes, k);
            switch (mutable_create_info.pMutableDescriptorTypeLists[j].pDescriptorTypes[k]) {
                case VK_DESCRIPTOR_TYPE_MUTABLE_EXT:
                    skip |= LogError("VUID-VkMutableDescriptorTypeListEXT-pDescriptorTypes-04600", device, type_loc,
                                     "is VK_DESCRIPTOR_TYPE_MUTABLE_EXT.");
                    break;
                case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
                    skip |= LogError("VUID-VkMutableDescriptorTypeListEXT-pDescriptorTypes-04601", device, type_loc,
                                     "is VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC.");
                    break;
                case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
                    skip |= LogError("VUID-VkMutableDescriptorTypeListEXT-pDescriptorTypes-04602", device, type_loc,
                                     "is VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC.");
                    break;
                case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT:
                    skip |= LogError("VUID-VkMutableDescriptorTypeListEXT-pDescriptorTypes-04603", device, type_loc,
                                     "is VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT.");
                    break;
                default:
                    break;
            }
            for (uint32_t l = k + 1; l < mutable_create_info.pMutableDescriptorTypeLists[j].descriptorTypeCount; ++l) {
                if (mutable_create_info.pMutableDescriptorTypeLists[j].pDescriptorTypes[k] ==
                    mutable_create_info.pMutableDescriptorTypeLists[j].pDescriptorTypes[l]) {
                    skip |=
                        LogError("VUID-VkMutableDescriptorTypeListEXT-pDescriptorTypes-04598", device, type_loc,
                                 "and pDescriptorTypes[%" PRIu32 "] are both %s.", l,
                                 string_VkDescriptorType(mutable_create_info.pMutableDescriptorTypeLists[j].pDescriptorTypes[k]));
                }
            }
        }
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCreateDescriptorSetLayout(VkDevice device,
                                                                          const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
                                                                          const VkAllocationCallbacks *pAllocator,
                                                                          VkDescriptorSetLayout *pSetLayout,
                                                                          const ErrorObject &error_obj) const {
    bool skip = false;

    const auto *mutable_descriptor_type = vku::FindStructInPNextChain<VkMutableDescriptorTypeCreateInfoEXT>(pCreateInfo->pNext);
    const auto *mutable_descriptor_type_features =
        vku::FindStructInPNextChain<VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT>(device_createinfo_pnext);
    bool mutable_descriptor_type_features_enabled =
        mutable_descriptor_type_features && mutable_descriptor_type_features->mutableDescriptorType == VK_TRUE;
    const Location create_info_loc = error_obj.location.dot(Field::pCreateInfo);

    // Validation for parameters excluded from the generated validation code due to a 'noautovalidity' tag in vk.xml
    if (pCreateInfo->pBindings != nullptr) {
        for (uint32_t i = 0; i < pCreateInfo->bindingCount; ++i) {
            const Location binding_loc = create_info_loc.dot(Field::pBindings, i);
            if (pCreateInfo->pBindings[i].descriptorCount != 0) {
                // If descriptorCount is not 0, stageFlags must be a valid combination of VkShaderStageFlagBits values
                if ((pCreateInfo->pBindings[i].stageFlags != 0) &&
                    ((pCreateInfo->pBindings[i].stageFlags & (~AllVkShaderStageFlagBits)) != 0)) {
                    skip |= LogError("VUID-VkDescriptorSetLayoutBinding-descriptorCount-00283", device,
                                     binding_loc.dot(Field::descriptorCount),
                                     "is %" PRIu32 " but stageFlags is invalid (0x%" PRIx32 ").",
                                     pCreateInfo->pBindings[i].descriptorCount, pCreateInfo->pBindings[i].stageFlags);
                }

                if ((pCreateInfo->pBindings[i].descriptorType == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT) &&
                    (pCreateInfo->pBindings[i].stageFlags != 0) &&
                    (pCreateInfo->pBindings[i].stageFlags != VK_SHADER_STAGE_FRAGMENT_BIT)) {
                    skip |= LogError("VUID-VkDescriptorSetLayoutBinding-descriptorType-01510", device,
                                     binding_loc.dot(Field::stageFlags),
                                     "is %s but descriptorType is VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT.",
                                     string_VkShaderStageFlags(pCreateInfo->pBindings[i].stageFlags).c_str());
                }

                if (pCreateInfo->pBindings[i].descriptorType == VK_DESCRIPTOR_TYPE_MUTABLE_EXT) {
                    if (mutable_descriptor_type) {
                        if (i >= mutable_descriptor_type->mutableDescriptorTypeListCount) {
                            skip |= LogError("VUID-VkDescriptorSetLayoutCreateInfo-pBindings-07303", device,
                                             binding_loc.pNext(Struct::VkMutableDescriptorTypeCreateInfoEXT,
                                                               Field::mutableDescriptorTypeListCount),
                                             "is %" PRIu32 " but descriptorType is VK_DESCRIPTOR_TYPE_MUTABLE_EXT but ",
                                             mutable_descriptor_type->mutableDescriptorTypeListCount);
                        }
                    } else {
                        skip |= LogError("VUID-VkDescriptorSetLayoutCreateInfo-pBindings-07303", device,
                                         binding_loc.dot(Field::descriptorType),
                                         "is VK_DESCRIPTOR_TYPE_MUTABLE_EXT but VkMutableDescriptorTypeCreateInfoEXT is not "
                                         "included in the pNext chain.");
                    }
                    if (pCreateInfo->pBindings[i].pImmutableSamplers) {
                        skip |= LogError("VUID-VkDescriptorSetLayoutCreateInfo-descriptorType-04594", device,
                                         binding_loc.dot(Field::descriptorType),
                                         "is VK_DESCRIPTOR_TYPE_MUTABLE_EXT but pImmutableSamplers is not NULL.");
                    }
                    if (!mutable_descriptor_type_features_enabled) {
                        skip |= LogError("VUID-VkDescriptorSetLayoutCreateInfo-mutableDescriptorType-04595", device,
                                         binding_loc.dot(Field::descriptorType),
                                         "is VK_DESCRIPTOR_TYPE_MUTABLE_EXT but "
                                         "mutableDescriptorType feature was not enabled.");
                    }
                }

                if (pCreateInfo->flags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR &&
                    pCreateInfo->pBindings[i].descriptorType == VK_DESCRIPTOR_TYPE_MUTABLE_EXT) {
                    skip |=
                        LogError("VUID-VkDescriptorSetLayoutCreateInfo-flags-04591", device, binding_loc.dot(Field::descriptorType),
                                 "is VK_DESCRIPTOR_TYPE_MUTABLE_EXT, but flags includes "
                                 "VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR.");
                }

                if (pCreateInfo->flags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT &&
                    ((pCreateInfo->pBindings[i].descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) ||
                     (pCreateInfo->pBindings[i].descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC))) {
                    skip |=
                        LogError("VUID-VkDescriptorSetLayoutCreateInfo-flags-08000", device, binding_loc.dot(Field::descriptorType),
                                 "is %s, but flags includes VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR.",
                                 string_VkDescriptorType(pCreateInfo->pBindings[i].descriptorType));
                }
            }
        }

        if (mutable_descriptor_type) {
            skip |= ValidateMutableDescriptorTypeCreateInfo(*pCreateInfo, *mutable_descriptor_type, create_info_loc);
        }
    }

    // TODO - Remove these 2 extension checks once the enum-to-extensions logic is generated
    // mostly likely will fail test trying to hit these
    if (pCreateInfo->flags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR &&
        !IsExtEnabled(device_extensions.vk_khr_push_descriptor)) {
        skip |= LogError(
            device, kVUID_Core_DrawState_ExtensionNotEnabled,
            "vkCreateDescriptorSetLayout(): Attempted to use %s in %s but its required extension %s has not been enabled.\n",
            "VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR", "VkDescriptorSetLayoutCreateInfo::flags",
            VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    }
    if (pCreateInfo->flags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT &&
        !IsExtEnabled(device_extensions.vk_ext_descriptor_indexing)) {
        skip |= LogError(
            device, kVUID_Core_DrawState_ExtensionNotEnabled,
            "vkCreateDescriptorSetLayout(): Attemped to use %s in %s but its required extension %s has not been enabled.\n",
            "VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT", "VkDescriptorSetLayoutCreateInfo::flags",
            VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    }

    if ((pCreateInfo->flags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR) &&
        (pCreateInfo->flags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_HOST_ONLY_POOL_BIT_EXT)) {
        skip |= LogError("VUID-VkDescriptorSetLayoutCreateInfo-flags-04590", device, create_info_loc.dot(Field::flags), "is %s.",
                         string_VkDescriptorSetLayoutCreateFlags(pCreateInfo->flags).c_str());
    }
    if ((pCreateInfo->flags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT) &&
        (pCreateInfo->flags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_HOST_ONLY_POOL_BIT_EXT)) {
        skip |= LogError("VUID-VkDescriptorSetLayoutCreateInfo-flags-04592", device, create_info_loc.dot(Field::flags), "is %s.",
                         string_VkDescriptorSetLayoutCreateFlags(pCreateInfo->flags).c_str());
    }
    if (pCreateInfo->flags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_HOST_ONLY_POOL_BIT_EXT && !mutable_descriptor_type_features_enabled) {
        skip |= LogError("VUID-VkDescriptorSetLayoutCreateInfo-flags-04596", device, create_info_loc.dot(Field::flags),
                         "is %s, but mutableDescriptorType feature was not enabled.",
                         string_VkDescriptorSetLayoutCreateFlags(pCreateInfo->flags).c_str());
    }

    if ((pCreateInfo->flags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_EMBEDDED_IMMUTABLE_SAMPLERS_BIT_EXT) &&
        !(pCreateInfo->flags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT)) {
        skip |= LogError("VUID-VkDescriptorSetLayoutCreateInfo-flags-08001", device, create_info_loc.dot(Field::flags), "is %s.",
                         string_VkDescriptorSetLayoutCreateFlags(pCreateInfo->flags).c_str());
    }

    if ((pCreateInfo->flags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT) &&
        (pCreateInfo->flags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT)) {
        skip |= LogError("VUID-VkDescriptorSetLayoutCreateInfo-flags-08002", device, create_info_loc.dot(Field::flags), "is %s.",
                         string_VkDescriptorSetLayoutCreateFlags(pCreateInfo->flags).c_str());
    }

    if ((pCreateInfo->flags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT) &&
        (pCreateInfo->flags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_HOST_ONLY_POOL_BIT_VALVE)) {
        skip |= LogError("VUID-VkDescriptorSetLayoutCreateInfo-flags-08003", device, create_info_loc.dot(Field::flags), "is %s.",
                         string_VkDescriptorSetLayoutCreateFlags(pCreateInfo->flags).c_str());
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool,
                                                                   uint32_t descriptorSetCount,
                                                                   const VkDescriptorSet *pDescriptorSets,
                                                                   const ErrorObject &error_obj) const {
    // Validation for parameters excluded from the generated validation code due to a 'noautovalidity' tag in vk.xml
    // This is an array of handles, where the elements are allowed to be VK_NULL_HANDLE, and does not require any validation beyond
    // ValidateArray()
    return ValidateArray(error_obj.location.dot(Field::descriptorSetCount), error_obj.location.dot(Field::pDescriptorSets),
                         descriptorSetCount, &pDescriptorSets, true, true, kVUIDUndefined,
                         "VUID-vkFreeDescriptorSets-pDescriptorSets-00310");
}

bool StatelessValidation::ValidateWriteDescriptorSet(const Location &loc, const uint32_t descriptorWriteCount,
                                                     const VkWriteDescriptorSet *pDescriptorWrites,
                                                     const bool isPushDescriptor) const {
    bool skip = false;
    if (!pDescriptorWrites) {
        return skip;
    }
    const char *vkCallingFunction = loc.StringFunc();
    for (uint32_t i = 0; i < descriptorWriteCount; ++i) {
        // descriptorCount must be greater than 0
        if (pDescriptorWrites[i].descriptorCount == 0) {
            skip |= LogError(device, "VUID-VkWriteDescriptorSet-descriptorCount-arraylength",
                             "%s(): parameter pDescriptorWrites[%" PRIu32 "].descriptorCount must be greater than 0.",
                             vkCallingFunction, i);
        }

        // If called from vkCmdPushDescriptorSetKHR, the dstSet member is ignored.
        if (!isPushDescriptor) {
            // dstSet must be a valid VkDescriptorSet handle
            skip |= ValidateRequiredHandle(loc.dot(Field::pDescriptorWrites, i).dot(Field::dstSet), pDescriptorWrites[i].dstSet);
        }

        if ((pDescriptorWrites[i].descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER) ||
            (pDescriptorWrites[i].descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) ||
            (pDescriptorWrites[i].descriptorType == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE) ||
            (pDescriptorWrites[i].descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE) ||
            (pDescriptorWrites[i].descriptorType == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT)) {
            if (pDescriptorWrites[i].pImageInfo == nullptr) {
                if (!isPushDescriptor) {
                    // If descriptorType is VK_DESCRIPTOR_TYPE_SAMPLER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    // VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE or
                    // VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, pImageInfo must be a pointer to an array of descriptorCount valid
                    // VkDescriptorImageInfo structures. Valid imageView handles are checked in
                    // ObjectLifetimes::ValidateDescriptorWrite.
                    skip |=
                        LogError(device, "VUID-vkUpdateDescriptorSets-pDescriptorWrites-06493",
                                 "%s(): if pDescriptorWrites[%" PRIu32
                                 "].descriptorType is VK_DESCRIPTOR_TYPE_SAMPLER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, "
                                 "VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE or "
                                 "VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, pDescriptorWrites[%" PRIu32 "].pImageInfo must not be NULL.",
                                 vkCallingFunction, i, i);
                } else if ((pDescriptorWrites[i].descriptorType == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE) ||
                           (pDescriptorWrites[i].descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE) ||
                           (pDescriptorWrites[i].descriptorType == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT)) {
                    // If called from vkCmdPushDescriptorSetKHR, pImageInfo is only requred for descriptor types
                    // VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, and
                    // VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT
                    skip |= LogError(device, "VUID-vkCmdPushDescriptorSetKHR-pDescriptorWrites-06494",
                                     "%s(): if pDescriptorWrites[%" PRIu32
                                     "].descriptorType is VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE "
                                     "or VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, pDescriptorWrites[%" PRIu32
                                     "].pImageInfo must not be NULL.",
                                     vkCallingFunction, i, i);
                }
            } else if (pDescriptorWrites[i].descriptorType != VK_DESCRIPTOR_TYPE_SAMPLER) {
                // If descriptorType is VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                // VK_DESCRIPTOR_TYPE_STORAGE_IMAGE or VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, the imageLayout
                // member of any given element of pImageInfo must be a valid VkImageLayout
                for (uint32_t descriptor_index = 0; descriptor_index < pDescriptorWrites[i].descriptorCount; ++descriptor_index) {
                    skip |= ValidateRangedEnum(
                        loc.dot(Field::pDescriptorWrites, i).dot(Field::pImageInfo, descriptor_index).dot(Field::imageLayout),
                        "VkImageLayout", pDescriptorWrites[i].pImageInfo[descriptor_index].imageLayout, kVUIDUndefined);
                }
            }
        } else if ((pDescriptorWrites[i].descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) ||
                   (pDescriptorWrites[i].descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) ||
                   (pDescriptorWrites[i].descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) ||
                   (pDescriptorWrites[i].descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)) {
            // If descriptorType is VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            // VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC or VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, pBufferInfo must be a
            // pointer to an array of descriptorCount valid VkDescriptorBufferInfo structures
            // Valid buffer handles are checked in ObjectLifetimes::ValidateDescriptorWrite.
            if (pDescriptorWrites[i].pBufferInfo == nullptr) {
                skip |= LogError(device, "VUID-VkWriteDescriptorSet-descriptorType-00324",
                                 "%s(): if pDescriptorWrites[%" PRIu32
                                 "].descriptorType is "
                                 "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, "
                                 "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC or VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, "
                                 "pDescriptorWrites[%" PRIu32 "].pBufferInfo must not be NULL.",
                                 vkCallingFunction, i, i);
            } else {
                const auto *robustness2_features = vku::FindStructInPNextChain<VkPhysicalDeviceRobustness2FeaturesEXT>(device_createinfo_pnext);
                if (robustness2_features && robustness2_features->nullDescriptor) {
                    for (uint32_t descriptor_index = 0; descriptor_index < pDescriptorWrites[i].descriptorCount;
                         ++descriptor_index) {
                        if (pDescriptorWrites[i].pBufferInfo[descriptor_index].buffer == VK_NULL_HANDLE &&
                            (pDescriptorWrites[i].pBufferInfo[descriptor_index].offset != 0 ||
                             pDescriptorWrites[i].pBufferInfo[descriptor_index].range != VK_WHOLE_SIZE)) {
                            skip |= LogError(device, "VUID-VkDescriptorBufferInfo-buffer-02999",
                                             "%s(): if pDescriptorWrites[%" PRIu32
                                             "].buffer is VK_NULL_HANDLE, "
                                             "offset (%" PRIu64 ") must be zero and range (%" PRIu64 ") must be VK_WHOLE_SIZE.",
                                             vkCallingFunction, i, pDescriptorWrites[i].pBufferInfo[descriptor_index].offset,
                                             pDescriptorWrites[i].pBufferInfo[descriptor_index].range);
                        }
                    }
                }
            }
        } else if ((pDescriptorWrites[i].descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER) ||
                   (pDescriptorWrites[i].descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER)) {
            // Valid bufferView handles are checked in ObjectLifetimes::ValidateDescriptorWrite.
        }

        if ((pDescriptorWrites[i].descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) ||
            (pDescriptorWrites[i].descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC)) {
            VkDeviceSize uniform_alignment = device_limits.minUniformBufferOffsetAlignment;
            for (uint32_t j = 0; j < pDescriptorWrites[i].descriptorCount; j++) {
                if (pDescriptorWrites[i].pBufferInfo != NULL) {
                    if (SafeModulo(pDescriptorWrites[i].pBufferInfo[j].offset, uniform_alignment) != 0) {
                        skip |= LogError(device, "VUID-VkWriteDescriptorSet-descriptorType-00327",
                                         "%s(): pDescriptorWrites[%" PRIu32 "].pBufferInfo[%" PRIu32 "].offset (%" PRIu64
                                         ") must be a multiple of device limit minUniformBufferOffsetAlignment %" PRIu64 ".",
                                         vkCallingFunction, i, j, pDescriptorWrites[i].pBufferInfo[j].offset, uniform_alignment);
                    }
                }
            }
        } else if ((pDescriptorWrites[i].descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) ||
                   (pDescriptorWrites[i].descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)) {
            VkDeviceSize storage_alignment = device_limits.minStorageBufferOffsetAlignment;
            for (uint32_t j = 0; j < pDescriptorWrites[i].descriptorCount; j++) {
                if (pDescriptorWrites[i].pBufferInfo != NULL) {
                    if (SafeModulo(pDescriptorWrites[i].pBufferInfo[j].offset, storage_alignment) != 0) {
                        skip |= LogError(device, "VUID-VkWriteDescriptorSet-descriptorType-00328",
                                         "%s(): pDescriptorWrites[%" PRIu32 "].pBufferInfo[%" PRIu32 "].offset (%" PRIu64
                                         ") must be a multiple of device limit minStorageBufferOffsetAlignment %" PRIu64 ".",
                                         vkCallingFunction, i, j, pDescriptorWrites[i].pBufferInfo[j].offset, storage_alignment);
                    }
                }
            }
        }
        // pNext chain must be either NULL or a pointer to a valid instance of VkWriteDescriptorSetAccelerationStructureKHR
        // or VkWriteDescriptorSetInlineUniformBlockEX
        if (pDescriptorWrites[i].descriptorType == VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR) {
            const auto *pnext_struct = vku::FindStructInPNextChain<VkWriteDescriptorSetAccelerationStructureKHR>(pDescriptorWrites[i].pNext);
            if (!pnext_struct || (pnext_struct->accelerationStructureCount != pDescriptorWrites[i].descriptorCount)) {
                skip |= LogError(device, "VUID-VkWriteDescriptorSet-descriptorType-02382",
                                 "%s(): If descriptorType is VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, the pNext"
                                 "chain must include a VkWriteDescriptorSetAccelerationStructureKHR structure whose "
                                 "accelerationStructureCount %" PRIu32 " member equals descriptorCount %" PRIu32 ".",
                                 vkCallingFunction, pnext_struct ? pnext_struct->accelerationStructureCount : -1,
                                 pDescriptorWrites[i].descriptorCount);
            }
            // further checks only if we have right structtype
            if (pnext_struct) {
                if (pnext_struct->accelerationStructureCount != pDescriptorWrites[i].descriptorCount) {
                    skip |=
                        LogError(device, "VUID-VkWriteDescriptorSetAccelerationStructureKHR-accelerationStructureCount-02236",
                                 "%s(): accelerationStructureCount %" PRIu32 " must be equal to descriptorCount %" PRIu32
                                 " in the extended structure "
                                 ".",
                                 vkCallingFunction, pnext_struct->accelerationStructureCount, pDescriptorWrites[i].descriptorCount);
                }
                if (pnext_struct->accelerationStructureCount == 0) {
                    skip |=
                        LogError(device, "VUID-VkWriteDescriptorSetAccelerationStructureKHR-accelerationStructureCount-arraylength",
                                 "%s(): accelerationStructureCount must be greater than 0 .", vkCallingFunction);
                }
                const auto *robustness2_features = vku::FindStructInPNextChain<VkPhysicalDeviceRobustness2FeaturesEXT>(device_createinfo_pnext);
                if (robustness2_features && robustness2_features->nullDescriptor == VK_FALSE) {
                    for (uint32_t j = 0; j < pnext_struct->accelerationStructureCount; ++j) {
                        if (pnext_struct->pAccelerationStructures[j] == VK_NULL_HANDLE) {
                            skip |=
                                LogError(device, "VUID-VkWriteDescriptorSetAccelerationStructureKHR-pAccelerationStructures-03580",
                                         "%s(): If the nullDescriptor feature is not enabled, each member of "
                                         "pAccelerationStructures must not be VK_NULL_HANDLE.",
                                         vkCallingFunction);
                        }
                    }
                }
            }
        } else if (pDescriptorWrites[i].descriptorType == VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV) {
            const auto *pnext_struct = vku::FindStructInPNextChain<VkWriteDescriptorSetAccelerationStructureNV>(pDescriptorWrites[i].pNext);
            if (!pnext_struct || (pnext_struct->accelerationStructureCount != pDescriptorWrites[i].descriptorCount)) {
                skip |= LogError(device, "VUID-VkWriteDescriptorSet-descriptorType-03817",
                                 "%s(): If descriptorType is VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV, the pNext"
                                 "chain must include a VkWriteDescriptorSetAccelerationStructureNV structure whose "
                                 "accelerationStructureCount %" PRIu32 " member equals descriptorCount %" PRIu32 ".",
                                 vkCallingFunction, pnext_struct ? pnext_struct->accelerationStructureCount : -1,
                                 pDescriptorWrites[i].descriptorCount);
            }
            // further checks only if we have right structtype
            if (pnext_struct) {
                if (pnext_struct->accelerationStructureCount != pDescriptorWrites[i].descriptorCount) {
                    skip |=
                        LogError(device, "VUID-VkWriteDescriptorSetAccelerationStructureNV-accelerationStructureCount-03747",
                                 "%s(): accelerationStructureCount %" PRIu32 " must be equal to descriptorCount %" PRIu32
                                 " in the extended structure "
                                 ".",
                                 vkCallingFunction, pnext_struct->accelerationStructureCount, pDescriptorWrites[i].descriptorCount);
                }
                if (pnext_struct->accelerationStructureCount == 0) {
                    skip |=
                        LogError(device, "VUID-VkWriteDescriptorSetAccelerationStructureNV-accelerationStructureCount-arraylength",
                                 "%s(): accelerationStructureCount must be greater than 0 .", vkCallingFunction);
                }
                const auto *robustness2_features = vku::FindStructInPNextChain<VkPhysicalDeviceRobustness2FeaturesEXT>(device_createinfo_pnext);
                if (robustness2_features && robustness2_features->nullDescriptor == VK_FALSE) {
                    for (uint32_t j = 0; j < pnext_struct->accelerationStructureCount; ++j) {
                        if (pnext_struct->pAccelerationStructures[j] == VK_NULL_HANDLE) {
                            skip |=
                                LogError(device, "VUID-VkWriteDescriptorSetAccelerationStructureNV-pAccelerationStructures-03749",
                                         "%s(): If the nullDescriptor feature is not enabled, each member of "
                                         "pAccelerationStructures must not be VK_NULL_HANDLE.",
                                         vkCallingFunction);
                        }
                    }
                }
            }
        }
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount,
                                                                     const VkWriteDescriptorSet *pDescriptorWrites,
                                                                     uint32_t descriptorCopyCount,
                                                                     const VkCopyDescriptorSet *pDescriptorCopies,
                                                                     const ErrorObject &error_obj) const {
    return ValidateWriteDescriptorSet(error_obj.location, descriptorWriteCount, pDescriptorWrites, false);
}

static bool MutableDescriptorTypePartialOverlap(const VkDescriptorPoolCreateInfo *pCreateInfo, uint32_t i, uint32_t j) {
    bool partial_overlap = false;

    constexpr std::array all_descriptor_types = {
        VK_DESCRIPTOR_TYPE_SAMPLER,
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
        VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
        VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
        VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT,
        VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
        VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV,
    };

    const auto *mutable_descriptor_type = vku::FindStructInPNextChain<VkMutableDescriptorTypeCreateInfoEXT>(pCreateInfo->pNext);
    if (mutable_descriptor_type) {
        vvl::span<const VkDescriptorType> first_types, second_types;

        if (mutable_descriptor_type->mutableDescriptorTypeListCount > i) {
            const uint32_t descriptorTypeCount = mutable_descriptor_type->pMutableDescriptorTypeLists[i].descriptorTypeCount;
            auto *pDescriptorTypes = mutable_descriptor_type->pMutableDescriptorTypeLists[i].pDescriptorTypes;
            first_types = vvl::make_span(pDescriptorTypes, descriptorTypeCount);
        } else {
            first_types = vvl::make_span(all_descriptor_types.data(), all_descriptor_types.size());
        }

        if (mutable_descriptor_type->mutableDescriptorTypeListCount > j) {
            const uint32_t descriptorTypeCount = mutable_descriptor_type->pMutableDescriptorTypeLists[j].descriptorTypeCount;
            auto *pDescriptorTypes = mutable_descriptor_type->pMutableDescriptorTypeLists[j].pDescriptorTypes;
            second_types = vvl::make_span(pDescriptorTypes, descriptorTypeCount);
        } else {
            second_types = vvl::make_span(all_descriptor_types.data(), all_descriptor_types.size());
        }

        bool complete_overlap = first_types.size() == second_types.size();
        bool disjoint = true;
        for (const auto first_type : first_types) {
            bool found = false;
            for (const auto second_type : second_types) {
                if (first_type == second_type) {
                    found = true;
                    break;
                }
            }
            if (found) {
                disjoint = false;
            } else {
                complete_overlap = false;
            }
            if (!disjoint && !complete_overlap) {
                partial_overlap = true;
                break;
            }
        }
    }

    return partial_overlap;
}

bool StatelessValidation::manual_PreCallValidateCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo *pCreateInfo,
                                                                     const VkAllocationCallbacks *pAllocator,
                                                                     VkDescriptorPool *pDescriptorPool,
                                                                     const ErrorObject &error_obj) const {
    bool skip = false;

    if (!pCreateInfo) {
        return skip;
    }
    const Location create_info_loc = error_obj.location.dot(Field::pCreateInfo);
    if (pCreateInfo->maxSets == 0 && ((pCreateInfo->flags & VK_DESCRIPTOR_POOL_CREATE_ALLOW_OVERALLOCATION_SETS_BIT_NV) == 0)) {
        skip |= LogError("VUID-VkDescriptorPoolCreateInfo-descriptorPoolOverallocation-09227", device,
                         create_info_loc.dot(Field::maxSets), "is zero.");
    }

    const auto *mutable_descriptor_type_features =
        vku::FindStructInPNextChain<VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT>(device_createinfo_pnext);
    bool mutable_descriptor_type_enabled =
        mutable_descriptor_type_features && mutable_descriptor_type_features->mutableDescriptorType == VK_TRUE;

    if (pCreateInfo->pPoolSizes) {
        for (uint32_t i = 0; i < pCreateInfo->poolSizeCount; ++i) {
            const Location pool_loc = create_info_loc.dot(Field::pPoolSizes, i);
            if (pCreateInfo->pPoolSizes[i].descriptorCount <= 0) {
                skip |= LogError("VUID-VkDescriptorPoolSize-descriptorCount-00302", device, pool_loc.dot(Field::descriptorCount),
                                 "is zero.");
            }
            if (pCreateInfo->pPoolSizes[i].type == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT &&
                (pCreateInfo->pPoolSizes[i].descriptorCount % 4) != 0) {
                skip |= LogError("VUID-VkDescriptorPoolSize-type-02218", device, pool_loc.dot(Field::descriptorCount),
                                 "is %" PRIu32 " (not a multiple of 4), but type is VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT.",
                                 pCreateInfo->pPoolSizes[i].descriptorCount);
            }
            if (pCreateInfo->pPoolSizes[i].type == VK_DESCRIPTOR_TYPE_MUTABLE_EXT && !mutable_descriptor_type_enabled) {
                skip |= LogError("VUID-VkDescriptorPoolCreateInfo-mutableDescriptorType-04608", device, pool_loc.dot(Field::type),
                                 "is VK_DESCRIPTOR_TYPE_MUTABLE_EXT "
                                 ", but mutableDescriptorType feature was not enabled.");
            }
            if (pCreateInfo->pPoolSizes[i].type == VK_DESCRIPTOR_TYPE_MUTABLE_EXT) {
                for (uint32_t j = i + 1; j < pCreateInfo->poolSizeCount; ++j) {
                    if (pCreateInfo->pPoolSizes[j].type == VK_DESCRIPTOR_TYPE_MUTABLE_EXT) {
                        if (MutableDescriptorTypePartialOverlap(pCreateInfo, i, j)) {
                            skip |= LogError("VUID-VkDescriptorPoolCreateInfo-pPoolSizes-04787", device, pool_loc.dot(Field::type),
                                             "and pCreateInfo->pPoolSizes[%" PRIu32
                                             "].type are both VK_DESCRIPTOR_TYPE_MUTABLE_EXT "
                                             " and have sets which partially overlap.",
                                             j);
                        }
                    }
                }
            }
        }
    }

    if (pCreateInfo->flags & VK_DESCRIPTOR_POOL_CREATE_HOST_ONLY_BIT_EXT && (!mutable_descriptor_type_enabled)) {
        skip |= LogError("VUID-VkDescriptorPoolCreateInfo-flags-04609", device, create_info_loc.dot(Field::flags),
                         "includes VK_DESCRIPTOR_POOL_CREATE_HOST_ONLY_BIT_EXT, "
                         "but mutableDescriptorType feature was not enabled.");
    }
    if ((pCreateInfo->flags & VK_DESCRIPTOR_POOL_CREATE_HOST_ONLY_BIT_EXT) &&
        (pCreateInfo->flags & VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT)) {
        skip |= LogError("VUID-VkDescriptorPoolCreateInfo-flags-04607", device, create_info_loc.dot(Field::flags),
                         "includes both "
                         "VK_DESCRIPTOR_POOL_CREATE_HOST_ONLY_BIT_EXT and VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT");
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo *pCreateInfo,
                                                                const VkAllocationCallbacks *pAllocator, VkQueryPool *pQueryPool,
                                                                const ErrorObject &error_obj) const {
    bool skip = false;

    // Validation for parameters excluded from the generated validation code due to a 'noautovalidity' tag in vk.xml
    if (!pCreateInfo) {
        return skip;
    }
    const Location create_info_loc = error_obj.location.dot(Field::pCreateInfo);
    // If queryType is VK_QUERY_TYPE_PIPELINE_STATISTICS, pipelineStatistics must be a valid combination of
    // VkQueryPipelineStatisticFlagBits values
    if ((pCreateInfo->queryType == VK_QUERY_TYPE_PIPELINE_STATISTICS) && (pCreateInfo->pipelineStatistics != 0) &&
        ((pCreateInfo->pipelineStatistics & (~AllVkQueryPipelineStatisticFlagBits)) != 0)) {
        skip |= LogError("VUID-VkQueryPoolCreateInfo-queryType-00792", device, create_info_loc.dot(Field::queryType),
                         "is VK_QUERY_TYPE_PIPELINE_STATISTICS, but "
                         "pCreateInfo->pipelineStatistics must be a valid combination of VkQueryPipelineStatisticFlagBits "
                         "values.");
    }
    if (pCreateInfo->queryCount == 0) {
        skip |= LogError("VUID-VkQueryPoolCreateInfo-queryCount-02763", device, create_info_loc.dot(Field::queryCount), "is zero.");
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCreateSamplerYcbcrConversion(VkDevice device,
                                                                             const VkSamplerYcbcrConversionCreateInfo *pCreateInfo,
                                                                             const VkAllocationCallbacks *pAllocator,
                                                                             VkSamplerYcbcrConversion *pYcbcrConversion,
                                                                             const ErrorObject &error_obj) const {
    bool skip = false;

    // Check samplerYcbcrConversion feature is set
    const auto *ycbcr_features = vku::FindStructInPNextChain<VkPhysicalDeviceSamplerYcbcrConversionFeatures>(device_createinfo_pnext);
    if ((ycbcr_features == nullptr) || (ycbcr_features->samplerYcbcrConversion == VK_FALSE)) {
        const auto *vulkan_11_features = vku::FindStructInPNextChain<VkPhysicalDeviceVulkan11Features>(device_createinfo_pnext);
        if ((vulkan_11_features == nullptr) || (vulkan_11_features->samplerYcbcrConversion == VK_FALSE)) {
            skip |= LogError("VUID-vkCreateSamplerYcbcrConversion-None-01648", device, error_obj.location,
                             "samplerYcbcrConversion feature must be enabled.");
        }
    }

    const VkFormat format = pCreateInfo->format;
    const Location create_info_loc = error_obj.location.dot(Field::pCreateInfo);

    // If there is a VkExternalFormatANDROID with externalFormat != 0, the value of components is ignored.
    if (GetExternalFormat(pCreateInfo->pNext) != 0) {
        return skip;
    }
    const VkComponentMapping components = pCreateInfo->components;
    // XChroma Subsampled is same as "the format has a _422 or _420 suffix" from spec
    if (vkuFormatIsXChromaSubsampled(format) == true) {
        if ((components.g != VK_COMPONENT_SWIZZLE_G) && (components.g != VK_COMPONENT_SWIZZLE_IDENTITY)) {
            skip |= LogError("VUID-VkSamplerYcbcrConversionCreateInfo-components-02581", device, create_info_loc,
                             "When using a XChroma subsampled format (%s) the components.g needs to be VK_COMPONENT_SWIZZLE_G "
                             "or VK_COMPONENT_SWIZZLE_IDENTITY, but is %s.",
                             string_VkFormat(format), string_VkComponentSwizzle(components.g));
        }

        if ((components.a != VK_COMPONENT_SWIZZLE_A) && (components.a != VK_COMPONENT_SWIZZLE_IDENTITY) &&
            (components.a != VK_COMPONENT_SWIZZLE_ONE) && (components.a != VK_COMPONENT_SWIZZLE_ZERO)) {
            skip |= LogError("VUID-VkSamplerYcbcrConversionCreateInfo-components-02582", device, create_info_loc,
                             " When using a XChroma subsampled format (%s) the components.a needs to be VK_COMPONENT_SWIZZLE_A or "
                             "VK_COMPONENT_SWIZZLE_IDENTITY or VK_COMPONENT_SWIZZLE_ONE or VK_COMPONENT_SWIZZLE_ZERO, but is %s.",
                             string_VkFormat(format), string_VkComponentSwizzle(components.a));
        }

        if ((components.r != VK_COMPONENT_SWIZZLE_R) && (components.r != VK_COMPONENT_SWIZZLE_IDENTITY) &&
            (components.r != VK_COMPONENT_SWIZZLE_B)) {
            skip |= LogError("VUID-VkSamplerYcbcrConversionCreateInfo-components-02583", device, create_info_loc,
                             "When using a XChroma subsampled format (%s) the components.r needs to be VK_COMPONENT_SWIZZLE_R "
                             "or VK_COMPONENT_SWIZZLE_IDENTITY or VK_COMPONENT_SWIZZLE_B, but is %s.",
                             string_VkFormat(format), string_VkComponentSwizzle(components.r));
        }

        if ((components.b != VK_COMPONENT_SWIZZLE_B) && (components.b != VK_COMPONENT_SWIZZLE_IDENTITY) &&
            (components.b != VK_COMPONENT_SWIZZLE_R)) {
            skip |= LogError("VUID-VkSamplerYcbcrConversionCreateInfo-components-02584", device, create_info_loc,
                             "When using a XChroma subsampled format (%s) the components.b needs to be VK_COMPONENT_SWIZZLE_B "
                             "or VK_COMPONENT_SWIZZLE_IDENTITY or VK_COMPONENT_SWIZZLE_R, but is %s.",
                             string_VkFormat(format), string_VkComponentSwizzle(components.b));
        }

        // If one is identity, both need to be
        const bool r_identity = ((components.r == VK_COMPONENT_SWIZZLE_R) || (components.r == VK_COMPONENT_SWIZZLE_IDENTITY));
        const bool b_identity = ((components.b == VK_COMPONENT_SWIZZLE_B) || (components.b == VK_COMPONENT_SWIZZLE_IDENTITY));
        if ((r_identity != b_identity) && ((r_identity == true) || (b_identity == true))) {
            skip |=
                LogError("VUID-VkSamplerYcbcrConversionCreateInfo-components-02585", device, create_info_loc,
                         "When using a XChroma subsampled format (%s) if either the components.r (%s) or components.b (%s) "
                         "are an identity swizzle, then both need to be an identity swizzle.",
                         string_VkFormat(format), string_VkComponentSwizzle(components.r), string_VkComponentSwizzle(components.b));
        }
    }

    if (pCreateInfo->ycbcrModel != VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY) {
        // Checks same VU multiple ways in order to give a more useful error message
        const char *vuid = "VUID-VkSamplerYcbcrConversionCreateInfo-ycbcrModel-01655";
        if ((components.r == VK_COMPONENT_SWIZZLE_ONE) || (components.r == VK_COMPONENT_SWIZZLE_ZERO) ||
            (components.g == VK_COMPONENT_SWIZZLE_ONE) || (components.g == VK_COMPONENT_SWIZZLE_ZERO) ||
            (components.b == VK_COMPONENT_SWIZZLE_ONE) || (components.b == VK_COMPONENT_SWIZZLE_ZERO)) {
            skip |=
                LogError(vuid, device, create_info_loc,
                         "The ycbcrModel (%s) is not VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY so components.r (%s), "
                         "components.g (%s), nor components.b (%s) can't be VK_COMPONENT_SWIZZLE_ZERO or VK_COMPONENT_SWIZZLE_ONE.",
                         string_VkSamplerYcbcrModelConversion(pCreateInfo->ycbcrModel), string_VkComponentSwizzle(components.r),
                         string_VkComponentSwizzle(components.g), string_VkComponentSwizzle(components.b));
        }

        // "must not correspond to a component which contains zero or one as a consequence of conversion to RGBA"
        // 4 component format = no issue
        // 3 = no [a]
        // 2 = no [b,a]
        // 1 = no [g,b,a]
        // depth/stencil = no [g,b,a] (shouldn't ever occur, but no VU preventing it)
        const uint32_t component_count = (vkuFormatIsDepthOrStencil(format) == true) ? 1 : vkuFormatComponentCount(format);

        if ((component_count < 4) && ((components.r == VK_COMPONENT_SWIZZLE_A) || (components.g == VK_COMPONENT_SWIZZLE_A) ||
                                      (components.b == VK_COMPONENT_SWIZZLE_A))) {
            skip |= LogError(vuid, device, create_info_loc,
                             "The ycbcrModel (%s) is not VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY so components.r (%s), "
                             "components.g (%s), or components.b (%s) can't be VK_COMPONENT_SWIZZLE_A.",
                             string_VkSamplerYcbcrModelConversion(pCreateInfo->ycbcrModel), string_VkComponentSwizzle(components.r),
                             string_VkComponentSwizzle(components.g), string_VkComponentSwizzle(components.b));
        } else if ((component_count < 3) &&
                   ((components.r == VK_COMPONENT_SWIZZLE_B) || (components.g == VK_COMPONENT_SWIZZLE_B) ||
                    (components.b == VK_COMPONENT_SWIZZLE_B) || (components.b == VK_COMPONENT_SWIZZLE_IDENTITY))) {
            skip |= LogError(vuid, device, create_info_loc,
                             "The ycbcrModel (%s) is not VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY so components.r (%s), "
                             "components.g (%s), or components.b (%s) can't be VK_COMPONENT_SWIZZLE_B "
                             "(components.b also can't be VK_COMPONENT_SWIZZLE_IDENTITY).",
                             string_VkSamplerYcbcrModelConversion(pCreateInfo->ycbcrModel), string_VkComponentSwizzle(components.r),
                             string_VkComponentSwizzle(components.g), string_VkComponentSwizzle(components.b));
        } else if ((component_count < 2) &&
                   ((components.r == VK_COMPONENT_SWIZZLE_G) || (components.g == VK_COMPONENT_SWIZZLE_G) ||
                    (components.g == VK_COMPONENT_SWIZZLE_IDENTITY) || (components.b == VK_COMPONENT_SWIZZLE_G))) {
            skip |= LogError(vuid, device, create_info_loc,
                             "The ycbcrModel (%s) is not VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY so components.r (%s), "
                             "components.g (%s), or components.b (%s) can't be VK_COMPONENT_SWIZZLE_G "
                             "(components.g also can't be VK_COMPONENT_SWIZZLE_IDENTITY).",
                             string_VkSamplerYcbcrModelConversion(pCreateInfo->ycbcrModel), string_VkComponentSwizzle(components.r),
                             string_VkComponentSwizzle(components.g), string_VkComponentSwizzle(components.b));
        }
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCreateSamplerYcbcrConversionKHR(
    VkDevice device, const VkSamplerYcbcrConversionCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator,
    VkSamplerYcbcrConversion *pYcbcrConversion, const ErrorObject &error_obj) const {
    return manual_PreCallValidateCreateSamplerYcbcrConversion(device, pCreateInfo, pAllocator, pYcbcrConversion, error_obj);
}
