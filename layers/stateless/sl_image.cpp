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

bool StatelessValidation::manual_PreCallValidateCreateImage(VkDevice device, const VkImageCreateInfo *pCreateInfo,
                                                            const VkAllocationCallbacks *pAllocator, VkImage *pImage) const {
    bool skip = false;

    if (pCreateInfo != nullptr) {
        const VkFormat image_format = pCreateInfo->format;
        const VkImageCreateFlags image_flags = pCreateInfo->flags;
        // Validation for parameters excluded from the generated validation code due to a 'noautovalidity' tag in vk.xml
        if (pCreateInfo->sharingMode == VK_SHARING_MODE_CONCURRENT) {
            // If sharingMode is VK_SHARING_MODE_CONCURRENT, queueFamilyIndexCount must be greater than 1
            auto const queue_family_index_count = pCreateInfo->queueFamilyIndexCount;
            if (queue_family_index_count <= 1) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-sharingMode-00942",
                                 "vkCreateImage(): queueFamilyIndexCount is %" PRIu32 "!", queue_family_index_count);
            }

            // If sharingMode is VK_SHARING_MODE_CONCURRENT, pQueueFamilyIndices must be a pointer to an array of
            // queueFamilyIndexCount uint32_t values
            if (pCreateInfo->pQueueFamilyIndices == nullptr) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-sharingMode-00941",
                                 "vkCreateImage(): pQueueFamilyIndices is nullptr!");
            }
        }

        skip |= ValidateGreaterThanZero(pCreateInfo->extent.width, "pCreateInfo->extent.width",
                                        "VUID-VkImageCreateInfo-extent-00944", "vkCreateImage");
        skip |= ValidateGreaterThanZero(pCreateInfo->extent.height, "pCreateInfo->extent.height",
                                        "VUID-VkImageCreateInfo-extent-00945", "vkCreateImage");
        skip |= ValidateGreaterThanZero(pCreateInfo->extent.depth, "pCreateInfo->extent.depth",
                                        "VUID-VkImageCreateInfo-extent-00946", "vkCreateImage");

        skip |= ValidateGreaterThanZero(pCreateInfo->mipLevels, "pCreateInfo->mipLevels", "VUID-VkImageCreateInfo-mipLevels-00947",
                                        "vkCreateImage");
        skip |= ValidateGreaterThanZero(pCreateInfo->arrayLayers, "pCreateInfo->arrayLayers",
                                        "VUID-VkImageCreateInfo-arrayLayers-00948", "vkCreateImage");

        // InitialLayout must be PREINITIALIZED or UNDEFINED
        if ((pCreateInfo->initialLayout != VK_IMAGE_LAYOUT_UNDEFINED) &&
            (pCreateInfo->initialLayout != VK_IMAGE_LAYOUT_PREINITIALIZED)) {
            skip |= LogError(
                device, "VUID-VkImageCreateInfo-initialLayout-00993",
                "vkCreateImage(): initialLayout is %s, must be VK_IMAGE_LAYOUT_UNDEFINED or VK_IMAGE_LAYOUT_PREINITIALIZED.",
                string_VkImageLayout(pCreateInfo->initialLayout));
        }

        // If imageType is VK_IMAGE_TYPE_1D, both extent.height and extent.depth must be 1
        if ((pCreateInfo->imageType == VK_IMAGE_TYPE_1D) &&
            ((pCreateInfo->extent.height != 1) || (pCreateInfo->extent.depth != 1))) {
            skip |= LogError(device, "VUID-VkImageCreateInfo-imageType-00956",
                             "vkCreateImage(): if pCreateInfo->imageType is VK_IMAGE_TYPE_1D, both pCreateInfo->extent.height and "
                             "pCreateInfo->extent.depth must be 1.");
        }

        if (pCreateInfo->flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) {
            const VkImageType type = pCreateInfo->imageType;
            const auto width = pCreateInfo->extent.width;
            const auto height = pCreateInfo->extent.height;
            if (type != VK_IMAGE_TYPE_2D) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-flags-00949",
                                 "vkCreateImage(): Image type %s is incompatible with VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT.",
                                 string_VkImageType(type));
            }

            if (width != height) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-flags-08865",
                                 "vkCreateImage(): extent.width (=%" PRIu32 ") not equal to extent.height (=%" PRIu32 ").",
                                 pCreateInfo->extent.width, pCreateInfo->extent.height);
            }

            if (pCreateInfo->arrayLayers < 6) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-flags-08866",
                                 "vkCreateImage(): arrayLayers (=%" PRIu32 ") is less than 6.", pCreateInfo->arrayLayers);
            }
        }

        if (pCreateInfo->imageType == VK_IMAGE_TYPE_2D) {
            if (pCreateInfo->extent.depth != 1) {
                skip |= LogError(
                    device, "VUID-VkImageCreateInfo-imageType-00957",
                    "vkCreateImage(): if pCreateInfo->imageType is VK_IMAGE_TYPE_2D, pCreateInfo->extent.depth must be 1.");
            }
        }

        // 3D image may have only 1 layer
        if ((pCreateInfo->imageType == VK_IMAGE_TYPE_3D) && (pCreateInfo->arrayLayers != 1)) {
            skip |= LogError(device, "VUID-VkImageCreateInfo-imageType-00961",
                             "vkCreateImage(): if pCreateInfo->imageType is VK_IMAGE_TYPE_3D, pCreateInfo->arrayLayers must be 1.");
        }

        if (0 != (pCreateInfo->usage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT)) {
            VkImageUsageFlags legal_flags = (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
                                             VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);
            // At least one of the legal attachment bits must be set
            if (0 == (pCreateInfo->usage & legal_flags)) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-usage-00966",
                                 "vkCreateImage(): Transient attachment image without a compatible attachment flag set.");
            }
            // No flags other than the legal attachment bits may be set
            legal_flags |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
            if (0 != (pCreateInfo->usage & ~legal_flags)) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-usage-00963",
                                 "vkCreateImage(): Transient attachment image with incompatible usage flags set.");
            }
        }

        // mipLevels must be less than or equal to the number of levels in the complete mipmap chain
        uint32_t max_dim = std::max(std::max(pCreateInfo->extent.width, pCreateInfo->extent.height), pCreateInfo->extent.depth);
        // Max mip levels is different for corner-sampled images vs normal images.
        uint32_t max_mip_levels = (image_flags & VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV)
                                      ? static_cast<uint32_t>(ceil(log2(max_dim)))
                                      : static_cast<uint32_t>(floor(log2(max_dim)) + 1);
        if (max_dim > 0 && pCreateInfo->mipLevels > max_mip_levels) {
            skip |=
                LogError(device, "VUID-VkImageCreateInfo-mipLevels-00958",
                         "vkCreateImage(): pCreateInfo->mipLevels must be less than or equal to "
                         "floor(log2(max(pCreateInfo->extent.width, pCreateInfo->extent.height, pCreateInfo->extent.depth)))+1.");
        }

        if ((image_flags & VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT) && (pCreateInfo->imageType != VK_IMAGE_TYPE_3D)) {
            skip |= LogError(device, "VUID-VkImageCreateInfo-flags-00950",
                             "vkCreateImage(): pCreateInfo->flags contains VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT but "
                             "pCreateInfo->imageType is not VK_IMAGE_TYPE_3D.");
        }

        if ((image_flags & VK_IMAGE_CREATE_2D_VIEW_COMPATIBLE_BIT_EXT) && (pCreateInfo->imageType != VK_IMAGE_TYPE_3D)) {
            skip |= LogError(device, "VUID-VkImageCreateInfo-flags-07755",
                             "vkCreateImage(): pCreateInfo->flags contains VK_IMAGE_CREATE_2D_VIEW_COMPATIBLE_BIT_EXT but "
                             "pCreateInfo->imageType is not VK_IMAGE_TYPE_3D.");
        }

        if ((image_flags & VK_IMAGE_CREATE_SPARSE_BINDING_BIT) && (!physical_device_features.sparseBinding)) {
            skip |= LogError(device, "VUID-VkImageCreateInfo-flags-00969",
                             "vkCreateImage(): pCreateInfo->flags contains VK_IMAGE_CREATE_SPARSE_BINDING_BIT, but the "
                             "VkPhysicalDeviceFeatures::sparseBinding feature is disabled.");
        }

        if ((image_flags & VK_IMAGE_CREATE_SPARSE_ALIASED_BIT) && (!physical_device_features.sparseResidencyAliased)) {
            skip |= LogError(
                device, "VUID-VkImageCreateInfo-flags-01924",
                "vkCreateImage(): the sparseResidencyAliased device feature is disabled: Images cannot be created with the "
                "VK_IMAGE_CREATE_SPARSE_ALIASED_BIT set.");
        }

        // If flags contains VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT or VK_IMAGE_CREATE_SPARSE_ALIASED_BIT, it must also contain
        // VK_IMAGE_CREATE_SPARSE_BINDING_BIT
        if (((image_flags & (VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT | VK_IMAGE_CREATE_SPARSE_ALIASED_BIT)) != 0) &&
            ((image_flags & VK_IMAGE_CREATE_SPARSE_BINDING_BIT) != VK_IMAGE_CREATE_SPARSE_BINDING_BIT)) {
            skip |= LogError(device, "VUID-VkImageCreateInfo-flags-00987",
                             "vkCreateImage: if pCreateInfo->flags contains VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT or "
                             "VK_IMAGE_CREATE_SPARSE_ALIASED_BIT, it must also contain VK_IMAGE_CREATE_SPARSE_BINDING_BIT.");
        }

        // Check for combinations of attributes that are incompatible with having VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT set
        if ((image_flags & VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT) != 0) {
            // Linear tiling is unsupported
            if (VK_IMAGE_TILING_LINEAR == pCreateInfo->tiling) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-tiling-04121",
                                 "vkCreateImage: if pCreateInfo->flags contains VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT then image "
                                 "tiling of VK_IMAGE_TILING_LINEAR is not supported");
            }

            // Sparse 1D image isn't valid
            if (VK_IMAGE_TYPE_1D == pCreateInfo->imageType) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-imageType-00970",
                                 "vkCreateImage: cannot specify VK_IMAGE_CREATE_SPARSE_BINDING_BIT for 1D image.");
            }

            // Sparse 2D image when device doesn't support it
            if ((VK_FALSE == physical_device_features.sparseResidencyImage2D) && (VK_IMAGE_TYPE_2D == pCreateInfo->imageType)) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-imageType-00971",
                                 "vkCreateImage: cannot specify VK_IMAGE_CREATE_SPARSE_BINDING_BIT for 2D image if corresponding "
                                 "feature is not enabled on the device.");
            }

            // Sparse 3D image when device doesn't support it
            if ((VK_FALSE == physical_device_features.sparseResidencyImage3D) && (VK_IMAGE_TYPE_3D == pCreateInfo->imageType)) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-imageType-00972",
                                 "vkCreateImage: cannot specify VK_IMAGE_CREATE_SPARSE_BINDING_BIT for 3D image if corresponding "
                                 "feature is not enabled on the device.");
            }

            // Multi-sample 2D image when device doesn't support it
            if (VK_IMAGE_TYPE_2D == pCreateInfo->imageType) {
                if ((VK_FALSE == physical_device_features.sparseResidency2Samples) &&
                    (VK_SAMPLE_COUNT_2_BIT == pCreateInfo->samples)) {
                    skip |= LogError(device, "VUID-VkImageCreateInfo-imageType-00973",
                                     "vkCreateImage: cannot specify VK_IMAGE_CREATE_SPARSE_BINDING_BIT for 2-sample image if "
                                     "corresponding feature is not enabled on the device.");
                } else if ((VK_FALSE == physical_device_features.sparseResidency4Samples) &&
                           (VK_SAMPLE_COUNT_4_BIT == pCreateInfo->samples)) {
                    skip |= LogError(device, "VUID-VkImageCreateInfo-imageType-00974",
                                     "vkCreateImage: cannot specify VK_IMAGE_CREATE_SPARSE_BINDING_BIT for 4-sample image if "
                                     "corresponding feature is not enabled on the device.");
                } else if ((VK_FALSE == physical_device_features.sparseResidency8Samples) &&
                           (VK_SAMPLE_COUNT_8_BIT == pCreateInfo->samples)) {
                    skip |= LogError(device, "VUID-VkImageCreateInfo-imageType-00975",
                                     "vkCreateImage: cannot specify VK_IMAGE_CREATE_SPARSE_BINDING_BIT for 8-sample image if "
                                     "corresponding feature is not enabled on the device.");
                } else if ((VK_FALSE == physical_device_features.sparseResidency16Samples) &&
                           (VK_SAMPLE_COUNT_16_BIT == pCreateInfo->samples)) {
                    skip |= LogError(device, "VUID-VkImageCreateInfo-imageType-00976",
                                     "vkCreateImage: cannot specify VK_IMAGE_CREATE_SPARSE_BINDING_BIT for 16-sample image if "
                                     "corresponding feature is not enabled on the device.");
                }
            }
        }

        // alias VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV
        if (pCreateInfo->usage & VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR) {
            if (pCreateInfo->imageType != VK_IMAGE_TYPE_2D) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-imageType-02082",
                                 "vkCreateImage: if usage includes VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR (or the "
                                 "alias VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV), imageType must be VK_IMAGE_TYPE_2D.");
            }
            if (pCreateInfo->samples != VK_SAMPLE_COUNT_1_BIT) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-samples-02083",
                                 "vkCreateImage: if usage includes VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR (or the "
                                 "alias VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV), samples must be VK_SAMPLE_COUNT_1_BIT.");
            }
            const auto *shading_rate_image_features =
                LvlFindInChain<VkPhysicalDeviceShadingRateImageFeaturesNV>(device_createinfo_pnext);
            if (shading_rate_image_features && shading_rate_image_features->shadingRateImage &&
                pCreateInfo->tiling != VK_IMAGE_TILING_OPTIMAL) {
                // KHR flag can be non-optimal
                skip |= LogError(device, "VUID-VkImageCreateInfo-shadingRateImage-07727",
                                 "vkCreateImage: if usage includes VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV, tiling must be "
                                 "VK_IMAGE_TILING_OPTIMAL.");
            }
        }

        if (image_flags & VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV) {
            if (pCreateInfo->imageType != VK_IMAGE_TYPE_2D && pCreateInfo->imageType != VK_IMAGE_TYPE_3D) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-flags-02050",
                                 "vkCreateImage: If flags contains VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV, "
                                 "imageType must be VK_IMAGE_TYPE_2D or VK_IMAGE_TYPE_3D.");
            }

            if ((image_flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) || FormatIsDepthOrStencil(image_format)) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-flags-02051",
                                 "vkCreateImage: If flags contains VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV, "
                                 "it must not also contain VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT and format (%s) must not be a "
                                 "depth/stencil format.",
                                 string_VkFormat(image_format));
            }

            if (pCreateInfo->imageType == VK_IMAGE_TYPE_2D && (pCreateInfo->extent.width == 1 || pCreateInfo->extent.height == 1)) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-flags-02052",
                                 "vkCreateImage: If flags contains VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV and "
                                 "imageType is VK_IMAGE_TYPE_2D, extent.width and extent.height must be "
                                 "greater than 1.");
            } else if (pCreateInfo->imageType == VK_IMAGE_TYPE_3D &&
                       (pCreateInfo->extent.width == 1 || pCreateInfo->extent.height == 1 || pCreateInfo->extent.depth == 1)) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-flags-02053",
                                 "vkCreateImage: If flags contains VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV and "
                                 "imageType is VK_IMAGE_TYPE_3D, extent.width, extent.height, and extent.depth "
                                 "must be greater than 1.");
            }
        }

        if (((image_flags & VK_IMAGE_CREATE_SAMPLE_LOCATIONS_COMPATIBLE_DEPTH_BIT_EXT) != 0) &&
            (FormatHasDepth(image_format) == false)) {
            skip |= LogError(device, "VUID-VkImageCreateInfo-flags-01533",
                             "vkCreateImage(): if flags contain VK_IMAGE_CREATE_SAMPLE_LOCATIONS_COMPATIBLE_DEPTH_BIT_EXT the "
                             "format (%s) must be a depth or depth/stencil format.",
                             string_VkFormat(image_format));
        }

        const auto image_stencil_struct = LvlFindInChain<VkImageStencilUsageCreateInfo>(pCreateInfo->pNext);
        if (image_stencil_struct != nullptr) {
            if ((image_stencil_struct->stencilUsage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT) != 0) {
                VkImageUsageFlags legal_flags = (VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);
                // No flags other than the legal attachment bits may be set
                legal_flags |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
                if ((image_stencil_struct->stencilUsage & ~legal_flags) != 0) {
                    skip |= LogError(device, "VUID-VkImageStencilUsageCreateInfo-stencilUsage-02539",
                                     "vkCreateImage(): in pNext chain, VkImageStencilUsageCreateInfo::stencilUsage includes "
                                     "VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT, it must not include bits other than "
                                     "VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT or VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT");
                }
            }

            if (FormatIsDepthOrStencil(image_format)) {
                if ((image_stencil_struct->stencilUsage & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT) != 0) {
                    if (pCreateInfo->extent.width > device_limits.maxFramebufferWidth) {
                        skip |=
                            LogError(device, "VUID-VkImageCreateInfo-Format-02536",
                                     "vkCreateImage(): Depth-stencil image contains VkImageStencilUsageCreateInfo structure with "
                                     "stencilUsage including VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT and image width (%" PRIu32
                                     ") exceeds device "
                                     "maxFramebufferWidth (%" PRIu32 ")",
                                     pCreateInfo->extent.width, device_limits.maxFramebufferWidth);
                    }

                    if (pCreateInfo->extent.height > device_limits.maxFramebufferHeight) {
                        skip |=
                            LogError(device, "VUID-VkImageCreateInfo-format-02537",
                                     "vkCreateImage(): Depth-stencil image contains VkImageStencilUsageCreateInfo structure with "
                                     "stencilUsage including VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT and image height (%" PRIu32
                                     ") exceeds device "
                                     "maxFramebufferHeight (%" PRIu32 ")",
                                     pCreateInfo->extent.height, device_limits.maxFramebufferHeight);
                    }
                }

                if (!physical_device_features.shaderStorageImageMultisample &&
                    ((image_stencil_struct->stencilUsage & VK_IMAGE_USAGE_STORAGE_BIT) != 0) &&
                    (pCreateInfo->samples != VK_SAMPLE_COUNT_1_BIT)) {
                    skip |=
                        LogError(device, "VUID-VkImageCreateInfo-format-02538",
                                 "vkCreateImage(): Depth-stencil image contains VkImageStencilUsageCreateInfo structure with "
                                 "stencilUsage including VK_IMAGE_USAGE_STORAGE_BIT and the multisampled storage images feature is "
                                 "not enabled, image samples must be VK_SAMPLE_COUNT_1_BIT");
                }

                if (((pCreateInfo->usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0) &&
                    ((image_stencil_struct->stencilUsage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) == 0)) {
                    skip |= LogError(
                        device, "VUID-VkImageCreateInfo-format-02795",
                        "vkCreateImage(): Depth-stencil image in which usage includes VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT "
                        "contains VkImageStencilUsageCreateInfo structure, VkImageStencilUsageCreateInfo::stencilUsage must  "
                        "also include VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT");
                } else if (((pCreateInfo->usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) == 0) &&
                           ((image_stencil_struct->stencilUsage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0)) {
                    skip |= LogError(
                        device, "VUID-VkImageCreateInfo-format-02796",
                        "vkCreateImage(): Depth-stencil image in which usage does not include "
                        "VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT "
                        "contains VkImageStencilUsageCreateInfo structure, VkImageStencilUsageCreateInfo::stencilUsage must  "
                        "also not include VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT");
                }

                if (((pCreateInfo->usage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT) != 0) &&
                    ((image_stencil_struct->stencilUsage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT) == 0)) {
                    skip |= LogError(
                        device, "VUID-VkImageCreateInfo-format-02797",
                        "vkCreateImage(): Depth-stencil image in which usage includes VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT "
                        "contains VkImageStencilUsageCreateInfo structure, VkImageStencilUsageCreateInfo::stencilUsage must  "
                        "also include VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT");
                } else if (((pCreateInfo->usage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT) == 0) &&
                           ((image_stencil_struct->stencilUsage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT) != 0)) {
                    skip |= LogError(
                        device, "VUID-VkImageCreateInfo-format-02798",
                        "vkCreateImage(): Depth-stencil image in which usage does not include "
                        "VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT "
                        "contains VkImageStencilUsageCreateInfo structure, VkImageStencilUsageCreateInfo::stencilUsage must  "
                        "also not include VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT");
                }
            }
        }

        if ((!physical_device_features.shaderStorageImageMultisample) && ((pCreateInfo->usage & VK_IMAGE_USAGE_STORAGE_BIT) != 0) &&
            (pCreateInfo->samples != VK_SAMPLE_COUNT_1_BIT)) {
            skip |= LogError(device, "VUID-VkImageCreateInfo-usage-00968",
                             "vkCreateImage(): usage contains VK_IMAGE_USAGE_STORAGE_BIT and the multisampled storage images "
                             "feature is not enabled, image samples must be VK_SAMPLE_COUNT_1_BIT");
        }

        std::vector<uint64_t> image_create_drm_format_modifiers;
        if (IsExtEnabled(device_extensions.vk_ext_image_drm_format_modifier)) {
            const auto drm_format_mod_list = LvlFindInChain<VkImageDrmFormatModifierListCreateInfoEXT>(pCreateInfo->pNext);
            const auto drm_format_mod_explict = LvlFindInChain<VkImageDrmFormatModifierExplicitCreateInfoEXT>(pCreateInfo->pNext);
            if (pCreateInfo->tiling == VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT) {
                if (((drm_format_mod_list != nullptr) && (drm_format_mod_explict != nullptr)) ||
                    ((drm_format_mod_list == nullptr) && (drm_format_mod_explict == nullptr))) {
                    skip |= LogError(device, "VUID-VkImageCreateInfo-tiling-02261",
                                     "vkCreateImage(): Tiling is VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT but pNext must have "
                                     "either VkImageDrmFormatModifierListCreateInfoEXT or "
                                     "VkImageDrmFormatModifierExplicitCreateInfoEXT in the pNext chain");
                } else if (drm_format_mod_explict != nullptr) {
                    image_create_drm_format_modifiers.push_back(drm_format_mod_explict->drmFormatModifier);
                } else if (drm_format_mod_list != nullptr) {
                    for (uint32_t i = 0; i < drm_format_mod_list->drmFormatModifierCount; i++) {
                        image_create_drm_format_modifiers.push_back(*drm_format_mod_list->pDrmFormatModifiers);
                    }
                }
            } else if ((drm_format_mod_list != nullptr) || (drm_format_mod_explict != nullptr)) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-pNext-02262",
                                 "vkCreateImage(): Tiling is not VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT but there is a "
                                 "VkImageDrmFormatModifierListCreateInfoEXT or VkImageDrmFormatModifierExplicitCreateInfoEXT "
                                 "in the pNext chain");
            }

            if (drm_format_mod_explict != nullptr && drm_format_mod_explict->pPlaneLayouts != nullptr) {
                for (uint32_t i = 0; i < drm_format_mod_explict->drmFormatModifierPlaneCount; ++i) {
                    if (drm_format_mod_explict->pPlaneLayouts[i].size != 0) {
                        skip |= LogError(device, "VUID-VkImageDrmFormatModifierExplicitCreateInfoEXT-size-02267",
                                         "vkCreateImage(): size is nonzero (%" PRIu64 ") in element %" PRIu32
                                         " of  VkImageDrmFormatModifierListCreateInfoEXT->pPlanedLayouts.",
                                         drm_format_mod_explict->pPlaneLayouts[i].size, i);
                    }
                    if (pCreateInfo->arrayLayers == 1 && drm_format_mod_explict->pPlaneLayouts[i].arrayPitch != 0) {
                        skip |= LogError(device, "VUID-VkImageDrmFormatModifierExplicitCreateInfoEXT-arrayPitch-02268",
                                         "vkCreateImage(): arrayPitch is nonzero (%" PRIu64 ") in element %" PRIu32
                                         " of VkImageDrmFormatModifierListCreateInfoEXT->pPlanedLayouts "
                                         "with pCreateInfo->arrayLayers being 1.",
                                         drm_format_mod_explict->pPlaneLayouts[i].arrayPitch, i);
                    }
                    if (pCreateInfo->extent.depth == 1 && drm_format_mod_explict->pPlaneLayouts[i].depthPitch != 0) {
                        skip |= LogError(device, "VUID-VkImageDrmFormatModifierExplicitCreateInfoEXT-depthPitch-02269",
                                         "vkCreateImage(): depthPitch is nonzero (%" PRIu64 ") in element %" PRIu32
                                         " of VkImageDrmFormatModifierListCreateInfoEXT->pPlanedLayouts "
                                         "with pCreateInfo->extext.depth being 1.",
                                         drm_format_mod_explict->pPlaneLayouts[i].depthPitch, i);
                    }
                }
            }
        }

        static const uint64_t drm_format_mod_linear = 0;
        bool image_create_maybe_linear = false;
        if (pCreateInfo->tiling == VK_IMAGE_TILING_LINEAR) {
            image_create_maybe_linear = true;
        } else if (pCreateInfo->tiling == VK_IMAGE_TILING_OPTIMAL) {
            image_create_maybe_linear = false;
        } else if (pCreateInfo->tiling == VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT) {
            image_create_maybe_linear =
                (std::find(image_create_drm_format_modifiers.begin(), image_create_drm_format_modifiers.end(),
                           drm_format_mod_linear) != image_create_drm_format_modifiers.end());
        }

        // If multi-sample, validate type, usage, tiling and mip levels.
        if ((pCreateInfo->samples != VK_SAMPLE_COUNT_1_BIT) &&
            ((pCreateInfo->imageType != VK_IMAGE_TYPE_2D) || (image_flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) ||
             (pCreateInfo->mipLevels != 1) || image_create_maybe_linear)) {
            skip |= LogError(device, "VUID-VkImageCreateInfo-samples-02257",
                             "vkCreateImage(): Multi-sample image with incompatible type, usage, tiling, or mips.");
        }

        if ((image_flags & VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT) &&
            ((pCreateInfo->mipLevels != 1) || (pCreateInfo->arrayLayers != 1) || (pCreateInfo->imageType != VK_IMAGE_TYPE_2D) ||
             image_create_maybe_linear)) {
            skip |= LogError(device, "VUID-VkImageCreateInfo-flags-02259",
                             "vkCreateImage(): Multi-device image with incompatible type, usage, tiling, or mips.");
        }

        if (pCreateInfo->usage & VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT) {
            if (pCreateInfo->imageType != VK_IMAGE_TYPE_2D) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-flags-02557",
                                 "vkCreateImage: if usage includes VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT, "
                                 "imageType must be VK_IMAGE_TYPE_2D.");
            }
            if (pCreateInfo->samples != VK_SAMPLE_COUNT_1_BIT) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-samples-02558",
                                 "vkCreateImage: if usage includes VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT, "
                                 "samples must be VK_SAMPLE_COUNT_1_BIT.");
            }
        }
        if (image_flags & VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT) {
            if (pCreateInfo->tiling != VK_IMAGE_TILING_OPTIMAL) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-flags-02565",
                                 "vkCreateImage: if usage includes VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT, "
                                 "tiling must be VK_IMAGE_TILING_OPTIMAL.");
            }
            if (pCreateInfo->imageType != VK_IMAGE_TYPE_2D) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-flags-02566",
                                 "vkCreateImage: if flags includes VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT, "
                                 "imageType must be VK_IMAGE_TYPE_2D.");
            }
            if (image_flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-flags-02567",
                                 "vkCreateImage: if flags includes VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT, "
                                 "flags must not include VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT.");
            }
            if (pCreateInfo->mipLevels != 1) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-flags-02568",
                                 "vkCreateImage: if flags includes VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT, mipLevels (%" PRIu32
                                 ") must be 1.",
                                 pCreateInfo->mipLevels);
            }
        }

        const auto swapchain_create_info = LvlFindInChain<VkImageSwapchainCreateInfoKHR>(pCreateInfo->pNext);
        if (swapchain_create_info != nullptr) {
            if (swapchain_create_info->swapchain != VK_NULL_HANDLE) {
                // All the following fall under the same VU that checks that the swapchain image uses parameters limited by the
                // table in #swapchain-wsi-image-create-info. Breaking up into multiple checks allows for more useful information
                // returned why this error occured. Check for matching Swapchain flags is done later in state tracking validation
                const char *vuid = "VUID-VkImageSwapchainCreateInfoKHR-swapchain-00995";
                const char *base_message = "vkCreateImage(): The image used for creating a presentable swapchain image";

                if (pCreateInfo->imageType != VK_IMAGE_TYPE_2D) {
                    // also implicitly forces the check above that extent.depth is 1
                    skip |= LogError(device, vuid, "%s must have a imageType value VK_IMAGE_TYPE_2D instead of %s.", base_message,
                                     string_VkImageType(pCreateInfo->imageType));
                }
                if (pCreateInfo->mipLevels != 1) {
                    skip |= LogError(device, vuid, "%s must have a mipLevels value of 1 instead of %" PRIu32 ".", base_message,
                                     pCreateInfo->mipLevels);
                }
                if (pCreateInfo->samples != VK_SAMPLE_COUNT_1_BIT) {
                    skip |= LogError(device, vuid, "%s must have a samples value of VK_SAMPLE_COUNT_1_BIT instead of %s.",
                                     base_message, string_VkSampleCountFlagBits(pCreateInfo->samples));
                }
                if (pCreateInfo->tiling != VK_IMAGE_TILING_OPTIMAL) {
                    skip |= LogError(device, vuid, "%s must have a tiling value of VK_IMAGE_TILING_OPTIMAL instead of %s.",
                                     base_message, string_VkImageTiling(pCreateInfo->tiling));
                }
                if (pCreateInfo->initialLayout != VK_IMAGE_LAYOUT_UNDEFINED) {
                    skip |= LogError(device, vuid, "%s must have a initialLayout value of VK_IMAGE_LAYOUT_UNDEFINED instead of %s.",
                                     base_message, string_VkImageLayout(pCreateInfo->initialLayout));
                }
                const VkImageCreateFlags valid_flags =
                    (VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT | VK_IMAGE_CREATE_PROTECTED_BIT |
                     VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT | VK_IMAGE_CREATE_EXTENDED_USAGE_BIT);
                if ((image_flags & ~valid_flags) != 0) {
                    skip |= LogError(device, vuid, "%s flags are %" PRIu32 "and must only have valid flags set.", base_message,
                                     image_flags);
                }
            }
        }

        // If Chroma subsampled format ( _420_ or _422_ )
        if (FormatIsXChromaSubsampled(image_format) && (SafeModulo(pCreateInfo->extent.width, 2) != 0)) {
            skip |=
                LogError(device, "VUID-VkImageCreateInfo-format-04712",
                         "vkCreateImage(): The format (%s) is X Chroma Subsampled (has _422 or _420 suffix) so the width (=%" PRIu32
                         ") must be a multiple of 2.",
                         string_VkFormat(image_format), pCreateInfo->extent.width);
        }
        if (FormatIsYChromaSubsampled(image_format) && (SafeModulo(pCreateInfo->extent.height, 2) != 0)) {
            skip |= LogError(device, "VUID-VkImageCreateInfo-format-04713",
                             "vkCreateImage(): The format (%s) is Y Chroma Subsampled (has _420 suffix) so the height (=%" PRIu32
                             ") must be a multiple of 2.",
                             string_VkFormat(image_format), pCreateInfo->extent.height);
        }

        const auto format_list_info = LvlFindInChain<VkImageFormatListCreateInfo>(pCreateInfo->pNext);
        if (format_list_info) {
            const uint32_t viewFormatCount = format_list_info->viewFormatCount;
            if (((image_flags & VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT) == 0) && (viewFormatCount > 1)) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-flags-04738",
                                 "vkCreateImage(): If the VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT is not set, then "
                                 "VkImageFormatListCreateInfo::viewFormatCount (%" PRIu32 ") must be 0 or 1.",
                                 viewFormatCount);
            }
            // Check if viewFormatCount is not zero that it is all compatible
            for (uint32_t i = 0; i < viewFormatCount; i++) {
                const bool class_compatible =
                    FormatCompatibilityClass(format_list_info->pViewFormats[i]) == FormatCompatibilityClass(image_format);
                if (!class_compatible) {
                    if (image_flags & VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT) {
                        const bool size_compatible =
                            !FormatIsCompressed(format_list_info->pViewFormats[i]) &&
                            FormatElementSize(format_list_info->pViewFormats[i]) == FormatElementSize(image_format);
                        if (!size_compatible) {
                            skip |= LogError(device, "VUID-VkImageCreateInfo-pNext-06722",
                                             "vkCreateImage(): VkImageFormatListCreateInfo::pViewFormats[%" PRIu32
                                             "] (%s) and VkImageCreateInfo::format (%s) are not compatible or size-compatible.",
                                             i, string_VkFormat(format_list_info->pViewFormats[i]), string_VkFormat(image_format));
                        }
                    } else {
                        skip |= LogError(device, "VUID-VkImageCreateInfo-pNext-06722",
                                         "vkCreateImage(): VkImageFormatListCreateInfo::pViewFormats[%" PRIu32
                                         "] (%s) and VkImageCreateInfo::format (%s) are not compatible.",
                                         i, string_VkFormat(format_list_info->pViewFormats[i]), string_VkFormat(image_format));
                    }
                }
            }
        }

        const auto image_compression_control = LvlFindInChain<VkImageCompressionControlEXT>(pCreateInfo->pNext);
        if (image_compression_control) {
            constexpr VkImageCompressionFlagsEXT AllVkImageCompressionFlagBitsEXT =
                (VK_IMAGE_COMPRESSION_DEFAULT_EXT | VK_IMAGE_COMPRESSION_FIXED_RATE_DEFAULT_EXT |
                 VK_IMAGE_COMPRESSION_FIXED_RATE_EXPLICIT_EXT | VK_IMAGE_COMPRESSION_DISABLED_EXT);
            skip |= ValidateFlags("vkCreateImage", "VkImageCompressionControlEXT::flags", "VkImageCompressionFlagsEXT",
                                  AllVkImageCompressionFlagBitsEXT, image_compression_control->flags, kOptionalSingleBit,
                                  "VUID-VkImageCompressionControlEXT-flags-06747");

            if (image_compression_control->flags == VK_IMAGE_COMPRESSION_FIXED_RATE_EXPLICIT_EXT &&
                !image_compression_control->pFixedRateFlags) {
                skip |= LogError(device, "VUID-VkImageCompressionControlEXT-flags-06748",
                                 "VkImageCompressionControlEXT::pFixedRateFlags is nullptr even though "
                                 "VkImageCompressionControlEXT::flags are %s",
                                 string_VkImageCompressionFlagsEXT(image_compression_control->flags).c_str());
            }
        }
#ifdef VK_USE_PLATFORM_METAL_EXT
        auto export_metal_object_info = LvlFindInChain<VkExportMetalObjectCreateInfoEXT>(pCreateInfo->pNext);
        while (export_metal_object_info) {
            if ((export_metal_object_info->exportObjectType != VK_EXPORT_METAL_OBJECT_TYPE_METAL_TEXTURE_BIT_EXT) &&
                (export_metal_object_info->exportObjectType != VK_EXPORT_METAL_OBJECT_TYPE_METAL_IOSURFACE_BIT_EXT)) {
                skip |=
                    LogError(device, "VUID-VkImageCreateInfo-pNext-06783",
                             "vkCreateImage(): The pNext chain contains a VkExportMetalObjectCreateInfoEXT whose "
                             "exportObjectType = %s, but only VkExportMetalObjectCreateInfoEXT structs with exportObjectType of "
                             "VK_EXPORT_METAL_OBJECT_TYPE_METAL_TEXTURE_BIT_EXT or "
                             "VK_EXPORT_METAL_OBJECT_TYPE_METAL_IOSURFACE_BIT_EXT are allowed",
                             string_VkExportMetalObjectTypeFlagBitsEXT(export_metal_object_info->exportObjectType));
            }
            export_metal_object_info = LvlFindInChain<VkExportMetalObjectCreateInfoEXT>(export_metal_object_info->pNext);
        }
        auto import_metal_texture_info = LvlFindInChain<VkImportMetalTextureInfoEXT>(pCreateInfo->pNext);
        while (import_metal_texture_info) {
            if ((import_metal_texture_info->plane != VK_IMAGE_ASPECT_PLANE_0_BIT) &&
                (import_metal_texture_info->plane != VK_IMAGE_ASPECT_PLANE_1_BIT) &&
                (import_metal_texture_info->plane != VK_IMAGE_ASPECT_PLANE_2_BIT)) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-pNext-06784",
                                 "vkCreateImage(): The pNext chain contains a VkImportMetalTextureInfoEXT whose "
                                 "plane = %s, but only VK_IMAGE_ASPECT_PLANE_0_BIT, VK_IMAGE_ASPECT_PLANE_1_BIT, or "
                                 "VK_IMAGE_ASPECT_PLANE_2_BIT are allowed",
                                 string_VkImageAspectFlags(import_metal_texture_info->plane).c_str());
            }
            auto format_plane_count = FormatPlaneCount(pCreateInfo->format);
            if ((format_plane_count <= 1) && (import_metal_texture_info->plane != VK_IMAGE_ASPECT_PLANE_0_BIT)) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-pNext-06785",
                                 "vkCreateImage(): The pNext chain contains a VkImportMetalTextureInfoEXT whose "
                                 "plane = %s, but only VK_IMAGE_ASPECT_PLANE_0_BIT is allowed for an image created with format %s, "
                                 "which is not multiplaner",
                                 string_VkImageAspectFlags(import_metal_texture_info->plane).c_str(),
                                 string_VkFormat(pCreateInfo->format));
            }
            if ((format_plane_count == 2) && (import_metal_texture_info->plane == VK_IMAGE_ASPECT_PLANE_2_BIT)) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-pNext-06786",
                                 "vkCreateImage(): The pNext chain contains a VkImportMetalTextureInfoEXT whose "
                                 "plane == VK_IMAGE_ASPECT_PLANE_2_BIT, which is not allowed for an image created with format %s, "
                                 "which has only 2 planes",
                                 string_VkFormat(pCreateInfo->format));
            }
            import_metal_texture_info = LvlFindInChain<VkImportMetalTextureInfoEXT>(import_metal_texture_info->pNext);
        }
#endif  // VK_USE_PLATFORM_METAL_EXT
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCreateImageView(VkDevice device, const VkImageViewCreateInfo *pCreateInfo,
                                                                const VkAllocationCallbacks *pAllocator, VkImageView *pView) const {
    bool skip = false;

    if (pCreateInfo != nullptr) {
        // Validate feature set if using CUBE_ARRAY
        if ((pCreateInfo->viewType == VK_IMAGE_VIEW_TYPE_CUBE_ARRAY) && (physical_device_features.imageCubeArray == false)) {
            skip |= LogError(pCreateInfo->image, "VUID-VkImageViewCreateInfo-viewType-01004",
                             "vkCreateImageView(): pCreateInfo->viewType can't be VK_IMAGE_VIEW_TYPE_CUBE_ARRAY without "
                             "enabling the imageCubeArray feature.");
        }

        if (pCreateInfo->subresourceRange.layerCount != VK_REMAINING_ARRAY_LAYERS) {
            if (pCreateInfo->viewType == VK_IMAGE_VIEW_TYPE_CUBE && pCreateInfo->subresourceRange.layerCount != 6) {
                skip |= LogError(device, "VUID-VkImageViewCreateInfo-viewType-02960",
                                 "vkCreateImageView(): subresourceRange.layerCount (%" PRIu32
                                 ") must be 6 or VK_REMAINING_ARRAY_LAYERS.",
                                 pCreateInfo->subresourceRange.layerCount);
            }
            if (pCreateInfo->viewType == VK_IMAGE_VIEW_TYPE_CUBE_ARRAY && (pCreateInfo->subresourceRange.layerCount % 6) != 0) {
                skip |= LogError(device, "VUID-VkImageViewCreateInfo-viewType-02961",
                                 "vkCreateImageView(): subresourceRange.layerCount (%" PRIu32
                                 ") must be a multiple of 6 or VK_REMAINING_ARRAY_LAYERS.",
                                 pCreateInfo->subresourceRange.layerCount);
            }
        }

        auto astc_decode_mode = LvlFindInChain<VkImageViewASTCDecodeModeEXT>(pCreateInfo->pNext);
        if (astc_decode_mode != nullptr) {
            if ((astc_decode_mode->decodeMode != VK_FORMAT_R16G16B16A16_SFLOAT) &&
                (astc_decode_mode->decodeMode != VK_FORMAT_R8G8B8A8_UNORM) &&
                (astc_decode_mode->decodeMode != VK_FORMAT_E5B9G9R9_UFLOAT_PACK32)) {
                skip |= LogError(device, "VUID-VkImageViewASTCDecodeModeEXT-decodeMode-02230",
                                 "vkCreateImageView(): VkImageViewASTCDecodeModeEXT::decodeMode must be "
                                 "VK_FORMAT_R16G16B16A16_SFLOAT, VK_FORMAT_R8G8B8A8_UNORM, or VK_FORMAT_E5B9G9R9_UFLOAT_PACK32.");
            }
            if ((FormatIsCompressed_ASTC_LDR(pCreateInfo->format) == false) &&
                (FormatIsCompressed_ASTC_HDR(pCreateInfo->format) == false)) {
                skip |= LogError(device, "VUID-VkImageViewASTCDecodeModeEXT-format-04084",
                                 "vkCreateImageView(): is using a VkImageViewASTCDecodeModeEXT but the image view format is %s and "
                                 "not an ASTC format.",
                                 string_VkFormat(pCreateInfo->format));
            }
        }

        auto ycbcr_conversion = LvlFindInChain<VkSamplerYcbcrConversionInfo>(pCreateInfo->pNext);
        if (ycbcr_conversion != nullptr) {
            if (ycbcr_conversion->conversion != VK_NULL_HANDLE) {
                if (IsIdentitySwizzle(pCreateInfo->components) == false) {
                    skip |= LogError(
                        device, "VUID-VkImageViewCreateInfo-pNext-01970",
                        "vkCreateImageView(): If there is a VkSamplerYcbcrConversion, the imageView must "
                        "be created with the identity swizzle. Here are the actual swizzle values:\n"
                        "r swizzle = %s\n"
                        "g swizzle = %s\n"
                        "b swizzle = %s\n"
                        "a swizzle = %s\n",
                        string_VkComponentSwizzle(pCreateInfo->components.r), string_VkComponentSwizzle(pCreateInfo->components.g),
                        string_VkComponentSwizzle(pCreateInfo->components.b), string_VkComponentSwizzle(pCreateInfo->components.a));
                }
            }
        }
#ifdef VK_USE_PLATFORM_METAL_EXT
        skip |= ExportMetalObjectsPNextUtil(
            VK_EXPORT_METAL_OBJECT_TYPE_METAL_TEXTURE_BIT_EXT, "VUID-VkImageViewCreateInfo-pNext-06787",
            "vkCreateImageView():", "VK_EXPORT_METAL_OBJECT_TYPE_METAL_TEXTURE_BIT_EXT", pCreateInfo->pNext);
#endif  // VK_USE_PLATFORM_METAL_EXT
    }
    return skip;
}
