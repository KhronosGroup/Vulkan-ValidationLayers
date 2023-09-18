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
                                                            const VkAllocationCallbacks *pAllocator, VkImage *pImage,
                                                            const ErrorObject &error_obj) const {
    bool skip = false;

    if (pCreateInfo == nullptr) {
        return skip;
    }
    const Location create_info_loc = error_obj.location.dot(Field::pCreateInfo);
    const VkFormat image_format = pCreateInfo->format;
    const VkImageCreateFlags image_flags = pCreateInfo->flags;
    // Validation for parameters excluded from the generated validation code due to a 'noautovalidity' tag in vk.xml
    if (pCreateInfo->sharingMode == VK_SHARING_MODE_CONCURRENT) {
        // If sharingMode is VK_SHARING_MODE_CONCURRENT, queueFamilyIndexCount must be greater than 1
        auto const queue_family_index_count = pCreateInfo->queueFamilyIndexCount;
        if (queue_family_index_count <= 1) {
            skip |= LogError("VUID-VkImageCreateInfo-sharingMode-00942", device, create_info_loc.dot(Field::queueFamilyIndexCount),
                             "is %" PRIu32 ".", queue_family_index_count);
        }

        // If sharingMode is VK_SHARING_MODE_CONCURRENT, pQueueFamilyIndices must be a pointer to an array of
        // queueFamilyIndexCount uint32_t values
        if (pCreateInfo->pQueueFamilyIndices == nullptr) {
            skip |= LogError("VUID-VkImageCreateInfo-sharingMode-00941", device, create_info_loc.dot(Field::pQueueFamilyIndices),
                             "is NULL.");
        }
    }

    skip |= ValidateNotZero(pCreateInfo->extent.width == 0, "VUID-VkImageCreateInfo-extent-00944",
                            create_info_loc.dot(Field::extent).dot(Field::width));
    skip |= ValidateNotZero(pCreateInfo->extent.height == 0, "VUID-VkImageCreateInfo-extent-00945",
                            create_info_loc.dot(Field::extent).dot(Field::height));
    skip |= ValidateNotZero(pCreateInfo->extent.depth == 0, "VUID-VkImageCreateInfo-extent-00946",
                            create_info_loc.dot(Field::extent).dot(Field::depth));

    skip |= ValidateNotZero(pCreateInfo->mipLevels == 0, "VUID-VkImageCreateInfo-mipLevels-00947",
                            create_info_loc.dot(Field::mipLevels));
    skip |= ValidateNotZero(pCreateInfo->arrayLayers == 0, "VUID-VkImageCreateInfo-arrayLayers-00948",
                            create_info_loc.dot(Field::arrayLayers));

    // InitialLayout must be PREINITIALIZED or UNDEFINED
    if ((pCreateInfo->initialLayout != VK_IMAGE_LAYOUT_UNDEFINED) &&
        (pCreateInfo->initialLayout != VK_IMAGE_LAYOUT_PREINITIALIZED)) {
        skip |= LogError("VUID-VkImageCreateInfo-initialLayout-00993", device, create_info_loc.dot(Field::initialLayout),
                         "is %s, but must be UNDEFINED or PREINITIALIZED.", string_VkImageLayout(pCreateInfo->initialLayout));
    }

    // If imageType is VK_IMAGE_TYPE_1D, both extent.height and extent.depth must be 1
    if ((pCreateInfo->imageType == VK_IMAGE_TYPE_1D) && ((pCreateInfo->extent.height != 1) || (pCreateInfo->extent.depth != 1))) {
        skip |= LogError("VUID-VkImageCreateInfo-imageType-00956", device, create_info_loc,
                         "if pCreateInfo->imageType is VK_IMAGE_TYPE_1D, both pCreateInfo->extent.height and "
                         "pCreateInfo->extent.depth must be 1.");
    }

    if (pCreateInfo->flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) {
        const VkImageType type = pCreateInfo->imageType;
        const auto width = pCreateInfo->extent.width;
        const auto height = pCreateInfo->extent.height;
        if (type != VK_IMAGE_TYPE_2D) {
            skip |= LogError("VUID-VkImageCreateInfo-flags-00949", device, create_info_loc,
                             "Image type %s is incompatible with VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT.", string_VkImageType(type));
        }

        if (width != height) {
            skip |= LogError("VUID-VkImageCreateInfo-flags-08865", device, create_info_loc,
                             "extent.width (=%" PRIu32 ") not equal to extent.height (=%" PRIu32 ").", pCreateInfo->extent.width,
                             pCreateInfo->extent.height);
        }

        if (pCreateInfo->arrayLayers < 6) {
            skip |= LogError("VUID-VkImageCreateInfo-flags-08866", device, create_info_loc,
                             "arrayLayers (=%" PRIu32 ") is less than 6.", pCreateInfo->arrayLayers);
        }
    }

    if (pCreateInfo->imageType == VK_IMAGE_TYPE_2D) {
        if (pCreateInfo->extent.depth != 1) {
            skip |= LogError("VUID-VkImageCreateInfo-imageType-00957", device, create_info_loc,
                             "if pCreateInfo->imageType is VK_IMAGE_TYPE_2D, pCreateInfo->extent.depth must be 1.");
        }
    }

    // 3D image may have only 1 layer
    if ((pCreateInfo->imageType == VK_IMAGE_TYPE_3D) && (pCreateInfo->arrayLayers != 1)) {
        skip |= LogError("VUID-VkImageCreateInfo-imageType-00961", device, create_info_loc,
                         "if pCreateInfo->imageType is VK_IMAGE_TYPE_3D, pCreateInfo->arrayLayers must be 1.");
    }

    if (0 != (pCreateInfo->usage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT)) {
        VkImageUsageFlags legal_flags = (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
                                         VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);
        // At least one of the legal attachment bits must be set
        if (0 == (pCreateInfo->usage & legal_flags)) {
            skip |= LogError("VUID-VkImageCreateInfo-usage-00966", device, create_info_loc,
                             "Transient attachment image without a compatible attachment flag set.");
        }
        // No flags other than the legal attachment bits may be set
        legal_flags |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
        if (0 != (pCreateInfo->usage & ~legal_flags)) {
            skip |= LogError("VUID-VkImageCreateInfo-usage-00963", device, create_info_loc,
                             "Transient attachment image with incompatible usage flags set.");
        }
    }

    // mipLevels must be less than or equal to the number of levels in the complete mipmap chain
    uint32_t max_dim = std::max(std::max(pCreateInfo->extent.width, pCreateInfo->extent.height), pCreateInfo->extent.depth);
    // Max mip levels is different for corner-sampled images vs normal images.
    uint32_t max_mip_levels = (image_flags & VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV)
                                  ? static_cast<uint32_t>(ceil(log2(max_dim)))
                                  : static_cast<uint32_t>(floor(log2(max_dim)) + 1);
    if (max_dim > 0 && pCreateInfo->mipLevels > max_mip_levels) {
        skip |= LogError("VUID-VkImageCreateInfo-mipLevels-00958", device, create_info_loc,
                         "pCreateInfo->mipLevels must be less than or equal to "
                         "floor(log2(max(pCreateInfo->extent.width, pCreateInfo->extent.height, pCreateInfo->extent.depth)))+1.");
    }

    if ((image_flags & VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT) && (pCreateInfo->imageType != VK_IMAGE_TYPE_3D)) {
        skip |= LogError("VUID-VkImageCreateInfo-flags-00950", device, create_info_loc.dot(Field::flags),
                         "includes VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT but "
                         "imageType is %s.",
                         string_VkImageType(pCreateInfo->imageType));
    }

    if ((image_flags & VK_IMAGE_CREATE_2D_VIEW_COMPATIBLE_BIT_EXT) && (pCreateInfo->imageType != VK_IMAGE_TYPE_3D)) {
        skip |= LogError("VUID-VkImageCreateInfo-flags-07755", device, create_info_loc.dot(Field::flags),
                         "includes VK_IMAGE_CREATE_2D_VIEW_COMPATIBLE_BIT_EXT but "
                         "imageType is %s.",
                         string_VkImageType(pCreateInfo->imageType));
    }

    if ((image_flags & VK_IMAGE_CREATE_SPARSE_BINDING_BIT) && (!physical_device_features.sparseBinding)) {
        skip |= LogError("VUID-VkImageCreateInfo-flags-00969", device, create_info_loc.dot(Field::flags),
                         "includes VK_IMAGE_CREATE_SPARSE_BINDING_BIT, but the "
                         "sparseBinding feature was not enabled.");
    }

    if ((image_flags & VK_IMAGE_CREATE_SPARSE_ALIASED_BIT) && (!physical_device_features.sparseResidencyAliased)) {
        skip |= LogError("VUID-VkImageCreateInfo-flags-01924", device, create_info_loc.dot(Field::flags),
                         "includes VK_IMAGE_CREATE_SPARSE_ALIASED_BIT but the sparseResidencyAliased feature was not enabled.");
    }

    // If flags contains VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT or VK_IMAGE_CREATE_SPARSE_ALIASED_BIT, it must also contain
    // VK_IMAGE_CREATE_SPARSE_BINDING_BIT
    if (((image_flags & (VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT | VK_IMAGE_CREATE_SPARSE_ALIASED_BIT)) != 0) &&
        ((image_flags & VK_IMAGE_CREATE_SPARSE_BINDING_BIT) != VK_IMAGE_CREATE_SPARSE_BINDING_BIT)) {
        skip |= LogError("VUID-VkImageCreateInfo-flags-00987", device, create_info_loc.dot(Field::flags), "is %s.",
                         string_VkImageCreateFlags(image_flags).c_str());
    }

    // Check for combinations of attributes that are incompatible with having VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT set
    if ((image_flags & VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT) != 0) {
        // Linear tiling is unsupported
        if (VK_IMAGE_TILING_LINEAR == pCreateInfo->tiling) {
            skip |= LogError("VUID-VkImageCreateInfo-tiling-04121", device, create_info_loc.dot(Field::flags),
                             "includes VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT but tiling is VK_IMAGE_TILING_LINEAR.");
        }

        // Sparse 1D image isn't valid
        if (VK_IMAGE_TYPE_1D == pCreateInfo->imageType) {
            skip |= LogError("VUID-VkImageCreateInfo-imageType-00970", device, create_info_loc.dot(Field::flags),
                             "includes VK_IMAGE_CREATE_SPARSE_BINDING_BIT but imageType is VK_IMAGE_TYPE_1D.");
        }

        // Sparse 2D image when device doesn't support it
        if ((!physical_device_features.sparseResidencyImage2D) && (VK_IMAGE_TYPE_2D == pCreateInfo->imageType)) {
            skip |= LogError("VUID-VkImageCreateInfo-imageType-00971", device, create_info_loc.dot(Field::flags),
                             "includes VK_IMAGE_CREATE_SPARSE_BINDING_BIT and imageType is VK_IMAGE_TYPE_2D, but "
                             "sparseResidencyImage2D feature was not enabled.");
        }

        // Sparse 3D image when device doesn't support it
        if ((!physical_device_features.sparseResidencyImage3D) && (VK_IMAGE_TYPE_3D == pCreateInfo->imageType)) {
            skip |= LogError("VUID-VkImageCreateInfo-imageType-00972", device, create_info_loc.dot(Field::flags),
                             "includes VK_IMAGE_CREATE_SPARSE_BINDING_BIT and imageType is VK_IMAGE_TYPE_3D, but "
                             "sparseResidencyImage3D feature was not enabled.");
        }

        // Multi-sample 2D image when device doesn't support it
        if (VK_IMAGE_TYPE_2D == pCreateInfo->imageType) {
            if ((!physical_device_features.sparseResidency2Samples) && (VK_SAMPLE_COUNT_2_BIT == pCreateInfo->samples)) {
                skip |= LogError("VUID-VkImageCreateInfo-imageType-00973", device, create_info_loc.dot(Field::flags),
                                 "includes VK_IMAGE_CREATE_SPARSE_BINDING_BIT and imageType is VK_IMAGE_TYPE_2D and samples is "
                                 "VK_SAMPLE_COUNT_2_BIT, but sparseResidency2Samples feature was not enabled.");
            } else if ((!physical_device_features.sparseResidency4Samples) && (VK_SAMPLE_COUNT_4_BIT == pCreateInfo->samples)) {
                skip |= LogError("VUID-VkImageCreateInfo-imageType-00974", device, create_info_loc.dot(Field::flags),
                                 "includes VK_IMAGE_CREATE_SPARSE_BINDING_BIT and imageType is VK_IMAGE_TYPE_2D and samples is "
                                 "VK_SAMPLE_COUNT_4_BIT, but sparseResidency4Samples feature was not enabled.");
            } else if ((!physical_device_features.sparseResidency8Samples) && (VK_SAMPLE_COUNT_8_BIT == pCreateInfo->samples)) {
                skip |= LogError("VUID-VkImageCreateInfo-imageType-00975", device, create_info_loc.dot(Field::flags),
                                 "includes VK_IMAGE_CREATE_SPARSE_BINDING_BIT and imageType is VK_IMAGE_TYPE_2D and samples is "
                                 "VK_SAMPLE_COUNT_8_BIT, but sparseResidency8Samples feature was not enabled.");
            } else if ((!physical_device_features.sparseResidency16Samples) && (VK_SAMPLE_COUNT_16_BIT == pCreateInfo->samples)) {
                skip |= LogError("VUID-VkImageCreateInfo-imageType-00976", device, create_info_loc.dot(Field::flags),
                                 "includes VK_IMAGE_CREATE_SPARSE_BINDING_BIT and imageType is VK_IMAGE_TYPE_2D and samples is "
                                 "VK_SAMPLE_COUNT_16_BIT, but sparseResidency16Samples feature was not enabled.");
            }
        }
    }

    // alias VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV
    if (pCreateInfo->usage & VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR) {
        if (pCreateInfo->imageType != VK_IMAGE_TYPE_2D) {
            skip |= LogError("VUID-VkImageCreateInfo-imageType-02082", device, create_info_loc.dot(Field::usage),
                             "includes VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR (or the "
                             "alias VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV), but imageType is %s.",
                             string_VkImageType(pCreateInfo->imageType));
        }
        if (pCreateInfo->samples != VK_SAMPLE_COUNT_1_BIT) {
            skip |= LogError("VUID-VkImageCreateInfo-samples-02083", device, create_info_loc.dot(Field::usage),
                             "includes VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR (or the "
                             "alias VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV), but samples is %s.",
                             string_VkSampleCountFlagBits(pCreateInfo->samples));
        }
        const auto *shading_rate_image_features =
            vku::FindStructInPNextChain<VkPhysicalDeviceShadingRateImageFeaturesNV>(device_createinfo_pnext);
        if (shading_rate_image_features && shading_rate_image_features->shadingRateImage &&
            pCreateInfo->tiling != VK_IMAGE_TILING_OPTIMAL) {
            // KHR flag can be non-optimal
            skip |= LogError("VUID-VkImageCreateInfo-shadingRateImage-07727", device, create_info_loc.dot(Field::usage),
                             "includes VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV, tiling must be "
                             "VK_IMAGE_TILING_OPTIMAL.");
        }
    }

    if (image_flags & VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV) {
        if (pCreateInfo->imageType != VK_IMAGE_TYPE_2D && pCreateInfo->imageType != VK_IMAGE_TYPE_3D) {
            skip |= LogError("VUID-VkImageCreateInfo-flags-02050", device, create_info_loc.dot(Field::flags),
                             "includes VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV, "
                             "but imageType is %s.",
                             string_VkImageType(pCreateInfo->imageType));
        }

        if ((image_flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) || vkuFormatIsDepthOrStencil(image_format)) {
            skip |= LogError("VUID-VkImageCreateInfo-flags-02051", device, create_info_loc.dot(Field::flags),
                             "includes VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV, "
                             "it must not also contain VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT and format (%s) must not be a "
                             "depth/stencil format.",
                             string_VkFormat(image_format));
        }

        if (pCreateInfo->imageType == VK_IMAGE_TYPE_2D && (pCreateInfo->extent.width == 1 || pCreateInfo->extent.height == 1)) {
            skip |= LogError("VUID-VkImageCreateInfo-flags-02052", device, create_info_loc.dot(Field::flags),
                             "includes VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV and "
                             "imageType is VK_IMAGE_TYPE_2D, extent.width and extent.height must be "
                             "greater than 1.");
        } else if (pCreateInfo->imageType == VK_IMAGE_TYPE_3D &&
                   (pCreateInfo->extent.width == 1 || pCreateInfo->extent.height == 1 || pCreateInfo->extent.depth == 1)) {
            skip |= LogError("VUID-VkImageCreateInfo-flags-02053", device, create_info_loc.dot(Field::flags),
                             "includes VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV and "
                             "imageType is VK_IMAGE_TYPE_3D, extent.width, extent.height, and extent.depth "
                             "must be greater than 1.");
        }
    }

    if (((image_flags & VK_IMAGE_CREATE_SAMPLE_LOCATIONS_COMPATIBLE_DEPTH_BIT_EXT) != 0) &&
        (vkuFormatHasDepth(image_format) == false)) {
        skip |= LogError("VUID-VkImageCreateInfo-flags-01533", device, create_info_loc.dot(Field::flags),
                         "includes VK_IMAGE_CREATE_SAMPLE_LOCATIONS_COMPATIBLE_DEPTH_BIT_EXT the "
                         "format (%s) must be a depth or depth/stencil format.",
                         string_VkFormat(image_format));
    }

    const auto image_stencil_struct = vku::FindStructInPNextChain<VkImageStencilUsageCreateInfo>(pCreateInfo->pNext);
    if (image_stencil_struct != nullptr) {
        if ((image_stencil_struct->stencilUsage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT) != 0) {
            VkImageUsageFlags legal_flags = (VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);
            // No flags other than the legal attachment bits may be set
            legal_flags |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
            if ((image_stencil_struct->stencilUsage & ~legal_flags) != 0) {
                skip |= LogError("VUID-VkImageStencilUsageCreateInfo-stencilUsage-02539", device,
                                 create_info_loc.pNext(Struct::VkImageStencilUsageCreateInfo, Field::stencilUsage), "is %s.",
                                 string_VkImageUsageFlags(image_stencil_struct->stencilUsage).c_str());
            }
        }

        if (vkuFormatIsDepthOrStencil(image_format)) {
            if ((image_stencil_struct->stencilUsage & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT) != 0) {
                if (pCreateInfo->extent.width > device_limits.maxFramebufferWidth) {
                    skip |= LogError("VUID-VkImageCreateInfo-Format-02536", device,
                                     create_info_loc.pNext(Struct::VkImageStencilUsageCreateInfo, Field::stencilUsage),
                                     "includes VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT and image width (%" PRIu32
                                     ") exceeds device "
                                     "maxFramebufferWidth (%" PRIu32 ")",
                                     pCreateInfo->extent.width, device_limits.maxFramebufferWidth);
                }

                if (pCreateInfo->extent.height > device_limits.maxFramebufferHeight) {
                    skip |= LogError("VUID-VkImageCreateInfo-format-02537", device,
                                     create_info_loc.pNext(Struct::VkImageStencilUsageCreateInfo, Field::stencilUsage),
                                     "includes VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT and image height (%" PRIu32
                                     ") exceeds device "
                                     "maxFramebufferHeight (%" PRIu32 ")",
                                     pCreateInfo->extent.height, device_limits.maxFramebufferHeight);
                }
            }

            if (!physical_device_features.shaderStorageImageMultisample &&
                ((image_stencil_struct->stencilUsage & VK_IMAGE_USAGE_STORAGE_BIT) != 0) &&
                (pCreateInfo->samples != VK_SAMPLE_COUNT_1_BIT)) {
                skip |= LogError("VUID-VkImageCreateInfo-format-02538", device,
                                 create_info_loc.pNext(Struct::VkImageStencilUsageCreateInfo, Field::stencilUsage),
                                 "includes VK_IMAGE_USAGE_STORAGE_BIT and format is %s and samples is %s, but "
                                 "shaderStorageImageMultisample feature was not enabled.",
                                 string_VkFormat(image_format), string_VkSampleCountFlagBits(pCreateInfo->samples));
            }

            if (((pCreateInfo->usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0) &&
                ((image_stencil_struct->stencilUsage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) == 0)) {
                skip |= LogError("VUID-VkImageCreateInfo-format-02795", device, create_info_loc.dot(Field::usage),
                                 "is (%s), format is %s, and %s is %s", string_VkImageUsageFlags(pCreateInfo->usage).c_str(),
                                 string_VkFormat(image_format),
                                 create_info_loc.pNext(Struct::VkImageStencilUsageCreateInfo, Field::stencilUsage).Fields().c_str(),
                                 string_VkImageUsageFlags(image_stencil_struct->stencilUsage).c_str());
            } else if (((pCreateInfo->usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) == 0) &&
                       ((image_stencil_struct->stencilUsage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0)) {
                skip |= LogError("VUID-VkImageCreateInfo-format-02796", device, create_info_loc.dot(Field::usage),
                                 "is (%s), format is %s, and %s is %s", string_VkImageUsageFlags(pCreateInfo->usage).c_str(),
                                 string_VkFormat(image_format),
                                 create_info_loc.pNext(Struct::VkImageStencilUsageCreateInfo, Field::stencilUsage).Fields().c_str(),
                                 string_VkImageUsageFlags(image_stencil_struct->stencilUsage).c_str());
            }

            if (((pCreateInfo->usage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT) != 0) &&
                ((image_stencil_struct->stencilUsage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT) == 0)) {
                skip |= LogError("VUID-VkImageCreateInfo-format-02797", device, create_info_loc.dot(Field::usage),
                                 "is (%s), format is %s, and %s is %s", string_VkImageUsageFlags(pCreateInfo->usage).c_str(),
                                 string_VkFormat(image_format),
                                 create_info_loc.pNext(Struct::VkImageStencilUsageCreateInfo, Field::stencilUsage).Fields().c_str(),
                                 string_VkImageUsageFlags(image_stencil_struct->stencilUsage).c_str());
            } else if (((pCreateInfo->usage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT) == 0) &&
                       ((image_stencil_struct->stencilUsage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT) != 0)) {
                skip |= LogError("VUID-VkImageCreateInfo-format-02798", device, create_info_loc.dot(Field::usage),
                                 "is (%s), format is %s, and %s is %s", string_VkImageUsageFlags(pCreateInfo->usage).c_str(),
                                 string_VkFormat(image_format),
                                 create_info_loc.pNext(Struct::VkImageStencilUsageCreateInfo, Field::stencilUsage).Fields().c_str(),
                                 string_VkImageUsageFlags(image_stencil_struct->stencilUsage).c_str());
            }
        }
    }

    if ((!physical_device_features.shaderStorageImageMultisample) && ((pCreateInfo->usage & VK_IMAGE_USAGE_STORAGE_BIT) != 0) &&
        (pCreateInfo->samples != VK_SAMPLE_COUNT_1_BIT)) {
        skip |= LogError("VUID-VkImageCreateInfo-usage-00968", device, create_info_loc.dot(Field::usage),
                         "includes VK_IMAGE_USAGE_STORAGE_BIT and imageType is %s, but shaderStorageImageMultisample feature "
                         "was not enabled.",
                         string_VkSampleCountFlagBits(pCreateInfo->samples));
    }

    const auto format_list_info = vku::FindStructInPNextChain<VkImageFormatListCreateInfo>(pCreateInfo->pNext);

    std::vector<uint64_t> image_create_drm_format_modifiers;
    if (IsExtEnabled(device_extensions.vk_ext_image_drm_format_modifier)) {
        const auto drm_format_mod_list = vku::FindStructInPNextChain<VkImageDrmFormatModifierListCreateInfoEXT>(pCreateInfo->pNext);
        const auto drm_format_mod_explict = vku::FindStructInPNextChain<VkImageDrmFormatModifierExplicitCreateInfoEXT>(pCreateInfo->pNext);
        if (pCreateInfo->tiling == VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT) {
            if ((!drm_format_mod_list) && (!drm_format_mod_explict)) {
                skip |= LogError("VUID-VkImageCreateInfo-tiling-02261", device, create_info_loc.dot(Field::tiling),
                                 "is VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT but pNext is missing "
                                 "VkImageDrmFormatModifierListCreateInfoEXT or "
                                 "VkImageDrmFormatModifierExplicitCreateInfoEXT.");
            } else if ((drm_format_mod_list) && (drm_format_mod_explict)) {
                skip |= LogError("VUID-VkImageCreateInfo-tiling-02261", device, create_info_loc.dot(Field::tiling),
                                 "is VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT but pNext has both "
                                 "VkImageDrmFormatModifierListCreateInfoEXT and "
                                 "VkImageDrmFormatModifierExplicitCreateInfoEXT.");
            } else if (drm_format_mod_explict) {
                image_create_drm_format_modifiers.push_back(drm_format_mod_explict->drmFormatModifier);
            } else if (drm_format_mod_list) {
                for (uint32_t i = 0; i < drm_format_mod_list->drmFormatModifierCount; i++) {
                    image_create_drm_format_modifiers.push_back(*drm_format_mod_list->pDrmFormatModifiers);
                }
            }

            if (pCreateInfo->flags & VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT) {
                if (!format_list_info) {
                    skip |= LogError("VUID-VkImageCreateInfo-tiling-02353", device, create_info_loc.dot(Field::tiling),
                                     "is VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT, flags includes "
                                     "VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT, but pNext is missing VkImageFormatListCreateInfo.");
                } else if (format_list_info->viewFormatCount == 0) {
                    skip |=
                        LogError("VUID-VkImageCreateInfo-tiling-02353", device, create_info_loc.dot(Field::tiling),
                                 "is VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT, flags includes VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT, "
                                 "but pNext<VkImageFormatListCreateInfo>.viewFormatCount is zero.");
                }
            }
        } else if (drm_format_mod_list) {
            skip |= LogError("VUID-VkImageCreateInfo-pNext-02262", device, create_info_loc.dot(Field::tiling),
                             "is %s, but there is a "
                             "VkImageDrmFormatModifierListCreateInfoEXT in the pNext chain",
                             string_VkImageTiling(pCreateInfo->tiling));
        } else if (drm_format_mod_explict) {
            skip |= LogError("VUID-VkImageCreateInfo-pNext-02262", device, create_info_loc.dot(Field::tiling),
                             "is %s, but there is a VkImageDrmFormatModifierExplicitCreateInfoEXT "
                             "in the pNext chain",
                             string_VkImageTiling(pCreateInfo->tiling));
        }

        if (drm_format_mod_explict && drm_format_mod_explict->pPlaneLayouts) {
            for (uint32_t i = 0; i < drm_format_mod_explict->drmFormatModifierPlaneCount; ++i) {
                const Location drm_loc =
                    create_info_loc.pNext(Struct::VkImageDrmFormatModifierExplicitCreateInfoEXT, Field::pPlaneLayouts, i);
                if (drm_format_mod_explict->pPlaneLayouts[i].size != 0) {
                    skip |= LogError("VUID-VkImageDrmFormatModifierExplicitCreateInfoEXT-size-02267", device,
                                     drm_loc.dot(Field::size), "is %" PRIu64 ".", drm_format_mod_explict->pPlaneLayouts[i].size);
                }
                if (pCreateInfo->arrayLayers == 1 && drm_format_mod_explict->pPlaneLayouts[i].arrayPitch != 0) {
                    skip |= LogError("VUID-VkImageDrmFormatModifierExplicitCreateInfoEXT-arrayPitch-02268", device,
                                     drm_loc.dot(Field::arrayPitch), "is %" PRIu64 " and arrayLayers is 1.",
                                     drm_format_mod_explict->pPlaneLayouts[i].arrayPitch);
                }
                if (pCreateInfo->extent.depth == 1 && drm_format_mod_explict->pPlaneLayouts[i].depthPitch != 0) {
                    skip |= LogError("VUID-VkImageDrmFormatModifierExplicitCreateInfoEXT-depthPitch-02269", device,
                                     drm_loc.dot(Field::depthPitch), "is %" PRIu64 " and extext.depth is 1.",
                                     drm_format_mod_explict->pPlaneLayouts[i].depthPitch);
                }
            }
        }

        const auto compression_control = vku::FindStructInPNextChain<VkImageCompressionControlEXT>(pCreateInfo->pNext);
        if (drm_format_mod_explict && compression_control) {
            skip |= LogError("VUID-VkImageCreateInfo-pNext-06746", device, create_info_loc.dot(Field::pNext),
                             "has both VkImageCompressionControlEXT and VkImageDrmFormatModifierExplicitCreateInfoEXT.");
        }
    }

    static const uint64_t drm_format_mod_linear = 0;
    bool image_create_maybe_linear = false;
    if (pCreateInfo->tiling == VK_IMAGE_TILING_LINEAR) {
        image_create_maybe_linear = true;
    } else if (pCreateInfo->tiling == VK_IMAGE_TILING_OPTIMAL) {
        image_create_maybe_linear = false;
    } else if (pCreateInfo->tiling == VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT) {
        image_create_maybe_linear = (std::find(image_create_drm_format_modifiers.begin(), image_create_drm_format_modifiers.end(),
                                               drm_format_mod_linear) != image_create_drm_format_modifiers.end());
    }

    // If multi-sample, validate type, usage, tiling and mip levels.
    if ((pCreateInfo->samples != VK_SAMPLE_COUNT_1_BIT) &&
        ((pCreateInfo->imageType != VK_IMAGE_TYPE_2D) || (image_flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) ||
         (pCreateInfo->mipLevels != 1) || image_create_maybe_linear)) {
        skip |= LogError("VUID-VkImageCreateInfo-samples-02257", device, create_info_loc,
                         "image created with\n"
                         "samples = %s\n"
                         "imageType = %s\n"
                         "flags = %s (contains VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT)\n"
                         "mipLevels = %" PRIu32
                         "\n"
                         "which is not valid.",
                         string_VkSampleCountFlagBits(pCreateInfo->samples), string_VkImageType(pCreateInfo->imageType),
                         string_VkImageCreateFlags(image_flags).c_str(), pCreateInfo->mipLevels);
    }

    if ((image_flags & VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT) &&
        ((pCreateInfo->mipLevels != 1) || (pCreateInfo->arrayLayers != 1) || (pCreateInfo->imageType != VK_IMAGE_TYPE_2D) ||
         image_create_maybe_linear)) {
        skip |= LogError("VUID-VkImageCreateInfo-flags-02259", device, create_info_loc,
                         "image created with\n"
                         "imageType = %s\n"
                         "flags = %s (contains VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT)\n"
                         "arrayLayers = %" PRIu32
                         "\n"
                         "mipLevels = %" PRIu32
                         "\n"
                         "which is not valid.",
                         string_VkImageType(pCreateInfo->imageType), string_VkImageCreateFlags(image_flags).c_str(),
                         pCreateInfo->arrayLayers, pCreateInfo->mipLevels);
    }

    if (pCreateInfo->usage & VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT) {
        if (pCreateInfo->imageType != VK_IMAGE_TYPE_2D) {
            skip |= LogError("VUID-VkImageCreateInfo-flags-02557", device, create_info_loc.dot(Field::usage),
                             "includes VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT, but imageType is %s.",
                             string_VkImageType(pCreateInfo->imageType));
        }
        if (pCreateInfo->samples != VK_SAMPLE_COUNT_1_BIT) {
            skip |= LogError("VUID-VkImageCreateInfo-samples-02558", device, create_info_loc.dot(Field::usage),
                             "includes VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT, but samples is %s.",
                             string_VkSampleCountFlagBits(pCreateInfo->samples));
        }
    }
    if (image_flags & VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT) {
        if (pCreateInfo->tiling != VK_IMAGE_TILING_OPTIMAL) {
            skip |= LogError("VUID-VkImageCreateInfo-flags-02565", device, create_info_loc.dot(Field::flags),
                             "includes VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT, but tiling is %s.",
                             string_VkImageTiling(pCreateInfo->tiling));
        }
        if (pCreateInfo->imageType != VK_IMAGE_TYPE_2D) {
            skip |= LogError("VUID-VkImageCreateInfo-flags-02566", device, create_info_loc.dot(Field::flags),
                             "includes VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT, but imageType is %s.",
                             string_VkImageType(pCreateInfo->imageType));
        }
        if (image_flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) {
            skip |= LogError("VUID-VkImageCreateInfo-flags-02567", device, create_info_loc.dot(Field::flags),
                             "is %s, which contains SUBSAMPLED_BIT and CUBE_COMPATIBLE.",
                             string_VkImageCreateFlags(image_flags).c_str());
        }
        if (pCreateInfo->mipLevels != 1) {
            skip |= LogError("VUID-VkImageCreateInfo-flags-02568", device, create_info_loc.dot(Field::flags),
                             "includes VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT, but mipLevels is %" PRIu32 ".", pCreateInfo->mipLevels);
        }
    }

    const auto swapchain_create_info = vku::FindStructInPNextChain<VkImageSwapchainCreateInfoKHR>(pCreateInfo->pNext);
    if (swapchain_create_info != nullptr) {
        if (swapchain_create_info->swapchain != VK_NULL_HANDLE) {
            // All the following fall under the same VU that checks that the swapchain image uses parameters limited by the
            // table in #swapchain-wsi-image-create-info. Breaking up into multiple checks allows for more useful information
            // returned why this error occured. Check for matching Swapchain flags is done later in state tracking validation
            const char *vuid = "VUID-VkImageSwapchainCreateInfoKHR-swapchain-00995";
            const Location swapchain_loc = create_info_loc.pNext(Struct::VkImageSwapchainCreateInfoKHR, Field::swapchain);

            if (pCreateInfo->imageType != VK_IMAGE_TYPE_2D) {
                // also implicitly forces the check above that extent.depth is 1
                skip |= LogError(vuid, swapchain_create_info->swapchain, swapchain_loc,
                                 "must have a imageType value VK_IMAGE_TYPE_2D instead of %s.",
                                 string_VkImageType(pCreateInfo->imageType));
            }
            if (pCreateInfo->mipLevels != 1) {
                skip |= LogError(vuid, swapchain_create_info->swapchain, swapchain_loc,
                                 "must have a mipLevels value of 1 instead of %" PRIu32 ".", pCreateInfo->mipLevels);
            }
            if (pCreateInfo->samples != VK_SAMPLE_COUNT_1_BIT) {
                skip |= LogError(vuid, swapchain_create_info->swapchain, swapchain_loc,
                                 "must have a samples value of VK_SAMPLE_COUNT_1_BIT instead of %s.",
                                 string_VkSampleCountFlagBits(pCreateInfo->samples));
            }
            if (pCreateInfo->tiling != VK_IMAGE_TILING_OPTIMAL) {
                skip |= LogError(vuid, swapchain_create_info->swapchain, swapchain_loc,
                                 "must have a tiling value of VK_IMAGE_TILING_OPTIMAL instead of %s.",
                                 string_VkImageTiling(pCreateInfo->tiling));
            }
            if (pCreateInfo->initialLayout != VK_IMAGE_LAYOUT_UNDEFINED) {
                skip |= LogError(vuid, swapchain_create_info->swapchain, swapchain_loc,
                                 "must have a initialLayout value of VK_IMAGE_LAYOUT_UNDEFINED instead of %s.",
                                 string_VkImageLayout(pCreateInfo->initialLayout));
            }
            const VkImageCreateFlags valid_flags =
                (VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT | VK_IMAGE_CREATE_PROTECTED_BIT |
                 VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT | VK_IMAGE_CREATE_EXTENDED_USAGE_BIT);
            if ((image_flags & ~valid_flags) != 0) {
                skip |= LogError(vuid, swapchain_create_info->swapchain, swapchain_loc,
                                 "flags are %" PRIu32 "and must only have valid flags set.", image_flags);
            }
        }
    }

    // If Chroma subsampled format ( _420_ or _422_ )
    if (vkuFormatIsXChromaSubsampled(image_format) && (SafeModulo(pCreateInfo->extent.width, 2) != 0)) {
        skip |=
            LogError("VUID-VkImageCreateInfo-format-04712", device, create_info_loc.dot(Field::format),
                     "(%s) is X Chroma Subsampled (has _422 or _420 suffix) so the width (%" PRIu32 ") must be a multiple of 2.",
                     string_VkFormat(image_format), pCreateInfo->extent.width);
    }
    if (vkuFormatIsYChromaSubsampled(image_format) && (SafeModulo(pCreateInfo->extent.height, 2) != 0)) {
        skip |= LogError("VUID-VkImageCreateInfo-format-04713", device, create_info_loc.dot(Field::format),
                         "(%s) is Y Chroma Subsampled (has _420 suffix) so the height (%" PRIu32 ") must be a multiple of 2.",
                         string_VkFormat(image_format), pCreateInfo->extent.height);
    }

    if (format_list_info) {
        const uint32_t viewFormatCount = format_list_info->viewFormatCount;
        if (((image_flags & VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT) == 0) && (viewFormatCount > 1)) {
            skip |= LogError("VUID-VkImageCreateInfo-flags-04738", device,
                             create_info_loc.pNext(Struct::VkImageFormatListCreateInfo, Field::viewFormatCount),
                             "is %" PRIu32 " but flag (%s) does not include VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT.", viewFormatCount,
                             string_VkImageCreateFlags(image_flags).c_str());
        }
        // Check if viewFormatCount is not zero that it is all compatible
        for (uint32_t i = 0; i < viewFormatCount; i++) {
            const bool class_compatible =
                vkuFormatCompatibilityClass(format_list_info->pViewFormats[i]) == vkuFormatCompatibilityClass(image_format);
            if (!class_compatible) {
                if (image_flags & VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT) {
                    const bool size_compatible =
                        !vkuFormatIsCompressed(format_list_info->pViewFormats[i]) &&
                        vkuFormatElementSize(format_list_info->pViewFormats[i]) == vkuFormatElementSize(image_format);
                    if (!size_compatible) {
                        skip |= LogError("VUID-VkImageCreateInfo-pNext-06722", device,
                                         create_info_loc.pNext(Struct::VkImageFormatListCreateInfo, Field::pViewFormats, i),
                                         "(%s) and VkImageCreateInfo::format (%s) are not compatible or size-compatible.",
                                         string_VkFormat(format_list_info->pViewFormats[i]), string_VkFormat(image_format));
                    }
                } else {
                    skip |= LogError("VUID-VkImageCreateInfo-pNext-06722", device,
                                     create_info_loc.pNext(Struct::VkImageFormatListCreateInfo, Field::pViewFormats, i),
                                     "(%s) and VkImageCreateInfo::format (%s) are not compatible.",
                                     string_VkFormat(format_list_info->pViewFormats[i]), string_VkFormat(image_format));
                }
            }
        }
    }

    const auto image_compression_control = vku::FindStructInPNextChain<VkImageCompressionControlEXT>(pCreateInfo->pNext);
    if (image_compression_control) {
        skip |= ValidateFlags(create_info_loc.pNext(Struct::VkImageCompressionControlEXT, Field::flags),
                              "VkImageCompressionFlagsEXT", AllVkImageCompressionFlagBitsEXT, image_compression_control->flags,
                              kOptionalSingleBit, "VUID-VkImageCompressionControlEXT-flags-06747");

        if (image_compression_control->flags == VK_IMAGE_COMPRESSION_FIXED_RATE_EXPLICIT_EXT &&
            !image_compression_control->pFixedRateFlags) {
            skip |= LogError("VUID-VkImageCompressionControlEXT-flags-06748", device,
                             create_info_loc.pNext(Struct::VkImageCompressionControlEXT, Field::flags),
                             "is %s, but pFixedRateFlags is NULL.",
                             string_VkImageCompressionFlagsEXT(image_compression_control->flags).c_str());
        }
    }
#ifdef VK_USE_PLATFORM_METAL_EXT
    auto export_metal_object_info = vku::FindStructInPNextChain<VkExportMetalObjectCreateInfoEXT>(pCreateInfo->pNext);
    while (export_metal_object_info) {
        if ((export_metal_object_info->exportObjectType != VK_EXPORT_METAL_OBJECT_TYPE_METAL_TEXTURE_BIT_EXT) &&
            (export_metal_object_info->exportObjectType != VK_EXPORT_METAL_OBJECT_TYPE_METAL_IOSURFACE_BIT_EXT)) {
            skip |= LogError("VUID-VkImageCreateInfo-pNext-06783", device,
                             create_info_loc.pNext(Struct::VkExportMetalObjectCreateInfoEXT, Field::exportObjectType),
                             "is %s, but only VkExportMetalObjectCreateInfoEXT structs with exportObjectType of "
                             "VK_EXPORT_METAL_OBJECT_TYPE_METAL_TEXTURE_BIT_EXT or "
                             "VK_EXPORT_METAL_OBJECT_TYPE_METAL_IOSURFACE_BIT_EXT are allowed",
                             string_VkExportMetalObjectTypeFlagBitsEXT(export_metal_object_info->exportObjectType));
        }
        export_metal_object_info = vku::FindStructInPNextChain<VkExportMetalObjectCreateInfoEXT>(export_metal_object_info->pNext);
    }
    auto import_metal_texture_info = vku::FindStructInPNextChain<VkImportMetalTextureInfoEXT>(pCreateInfo->pNext);
    while (import_metal_texture_info) {
        const Location texture_info_loc = create_info_loc.pNext(Struct::VkImportMetalTextureInfoEXT, Field::plane);
        if ((import_metal_texture_info->plane != VK_IMAGE_ASPECT_PLANE_0_BIT) &&
            (import_metal_texture_info->plane != VK_IMAGE_ASPECT_PLANE_1_BIT) &&
            (import_metal_texture_info->plane != VK_IMAGE_ASPECT_PLANE_2_BIT)) {
            skip |= LogError("VUID-VkImageCreateInfo-pNext-06784", device, texture_info_loc,
                             "is %s, but only VK_IMAGE_ASPECT_PLANE_0_BIT, VK_IMAGE_ASPECT_PLANE_1_BIT, or "
                             "VK_IMAGE_ASPECT_PLANE_2_BIT are allowed",
                             string_VkImageAspectFlags(import_metal_texture_info->plane).c_str());
        }
        auto format_plane_count = vkuFormatPlaneCount(pCreateInfo->format);
        if ((format_plane_count <= 1) && (import_metal_texture_info->plane != VK_IMAGE_ASPECT_PLANE_0_BIT)) {
            skip |=
                LogError("VUID-VkImageCreateInfo-pNext-06785", device, texture_info_loc,
                         "is %s, but only VK_IMAGE_ASPECT_PLANE_0_BIT is allowed for an image created with format %s, "
                         "which is not multiplaner",
                         string_VkImageAspectFlags(import_metal_texture_info->plane).c_str(), string_VkFormat(pCreateInfo->format));
        }
        if ((format_plane_count == 2) && (import_metal_texture_info->plane == VK_IMAGE_ASPECT_PLANE_2_BIT)) {
            skip |= LogError("VUID-VkImageCreateInfo-pNext-06786", device, texture_info_loc,
                             "is VK_IMAGE_ASPECT_PLANE_2_BIT, which is not allowed for an image created with format %s, "
                             "which has only 2 planes",
                             string_VkFormat(pCreateInfo->format));
        }
        import_metal_texture_info = vku::FindStructInPNextChain<VkImportMetalTextureInfoEXT>(import_metal_texture_info->pNext);
    }
#endif  // VK_USE_PLATFORM_METAL_EXT

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCreateImageView(VkDevice device, const VkImageViewCreateInfo *pCreateInfo,
                                                                const VkAllocationCallbacks *pAllocator, VkImageView *pView,
                                                                const ErrorObject &error_obj) const {
    bool skip = false;

    if (pCreateInfo == nullptr) {
        return skip;
    }
    const Location create_info_loc = error_obj.location.dot(Field::pCreateInfo);
    // Validate feature set if using CUBE_ARRAY
    if ((pCreateInfo->viewType == VK_IMAGE_VIEW_TYPE_CUBE_ARRAY) && (physical_device_features.imageCubeArray == false)) {
        skip |= LogError("VUID-VkImageViewCreateInfo-viewType-01004", pCreateInfo->image, create_info_loc.dot(Field::viewType),
                         "is VK_IMAGE_VIEW_TYPE_CUBE_ARRAY but the imageCubeArray feature is not enabled.");
    }

    if (pCreateInfo->subresourceRange.layerCount != VK_REMAINING_ARRAY_LAYERS) {
        if (pCreateInfo->viewType == VK_IMAGE_VIEW_TYPE_CUBE && pCreateInfo->subresourceRange.layerCount != 6) {
            skip |= LogError("VUID-VkImageViewCreateInfo-viewType-02960", pCreateInfo->image,
                             create_info_loc.dot(Field::subresourceRange).dot(Field::layerCount),
                             " (%" PRIu32 ") must be 6 or VK_REMAINING_ARRAY_LAYERS.", pCreateInfo->subresourceRange.layerCount);
        }
        if (pCreateInfo->viewType == VK_IMAGE_VIEW_TYPE_CUBE_ARRAY && (pCreateInfo->subresourceRange.layerCount % 6) != 0) {
            skip |= LogError("VUID-VkImageViewCreateInfo-viewType-02961", pCreateInfo->image,
                             create_info_loc.dot(Field::subresourceRange).dot(Field::layerCount),
                             "(%" PRIu32 ") must be a multiple of 6 or VK_REMAINING_ARRAY_LAYERS.",
                             pCreateInfo->subresourceRange.layerCount);
        }
    }

    auto astc_decode_mode = vku::FindStructInPNextChain<VkImageViewASTCDecodeModeEXT>(pCreateInfo->pNext);
    if (astc_decode_mode != nullptr) {
        if ((astc_decode_mode->decodeMode != VK_FORMAT_R16G16B16A16_SFLOAT) &&
            (astc_decode_mode->decodeMode != VK_FORMAT_R8G8B8A8_UNORM) &&
            (astc_decode_mode->decodeMode != VK_FORMAT_E5B9G9R9_UFLOAT_PACK32)) {
            skip |= LogError("VUID-VkImageViewASTCDecodeModeEXT-decodeMode-02230", pCreateInfo->image,
                             create_info_loc.pNext(Struct::VkImageViewASTCDecodeModeEXT, Field::decodeMode), "is %s.",
                             string_VkFormat(astc_decode_mode->decodeMode));
        }
        if ((vkuFormatIsCompressed_ASTC_LDR(pCreateInfo->format) == false) &&
            (vkuFormatIsCompressed_ASTC_HDR(pCreateInfo->format) == false)) {
            skip |=
                LogError("VUID-VkImageViewASTCDecodeModeEXT-format-04084", pCreateInfo->image, create_info_loc.dot(Field::format),
                         "%s is  not an ASTC format (because VkImageViewASTCDecodeModeEXT was passed in the pNext chain).",
                         string_VkFormat(pCreateInfo->format));
        }
    }

    auto ycbcr_conversion = vku::FindStructInPNextChain<VkSamplerYcbcrConversionInfo>(pCreateInfo->pNext);
    if (ycbcr_conversion != nullptr) {
        if (ycbcr_conversion->conversion != VK_NULL_HANDLE) {
            if (IsIdentitySwizzle(pCreateInfo->components) == false) {
                skip |= LogError(
                    "VUID-VkImageViewCreateInfo-pNext-01970", pCreateInfo->image, create_info_loc,
                    "If there is a VkSamplerYcbcrConversion, the imageView must "
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
    skip |=
        ExportMetalObjectsPNextUtil(VK_EXPORT_METAL_OBJECT_TYPE_METAL_TEXTURE_BIT_EXT, "VUID-VkImageViewCreateInfo-pNext-06787",
                                    error_obj.location, "VK_EXPORT_METAL_OBJECT_TYPE_METAL_TEXTURE_BIT_EXT", pCreateInfo->pNext);
#endif  // VK_USE_PLATFORM_METAL_EXT
    return skip;
}
