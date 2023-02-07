/* Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (C) 2015-2023 Google Inc.
 * Modifications Copyright (C) 2020-2022 Advanced Micro Devices, Inc. All rights reserved.
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

#include <vector>

#include "vk_layer_utils.h"
#include "vk_enum_string_helper.h"
#include "chassis.h"
#include "core_checks/core_validation.h"

#ifdef AHB_VALIDATION_SUPPORT
// Android-specific validation that uses types defined only on Android and only for NDK versions
// that support the VK_ANDROID_external_memory_android_hardware_buffer extension.
// This chunk could move into a seperate core_validation_android.cpp file... ?

// clang-format off

// Map external format and usage flags to/from equivalent Vulkan flags
// (Tables as of v1.1.92)

// AHardwareBuffer Format                       Vulkan Format
// ======================                       =============
// AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM        VK_FORMAT_R8G8B8A8_UNORM
// AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM        VK_FORMAT_R8G8B8A8_UNORM
// AHARDWAREBUFFER_FORMAT_R8G8B8_UNORM          VK_FORMAT_R8G8B8_UNORM
// AHARDWAREBUFFER_FORMAT_R5G6B5_UNORM          VK_FORMAT_R5G6B5_UNORM_PACK16
// AHARDWAREBUFFER_FORMAT_R16G16B16A16_FLOAT    VK_FORMAT_R16G16B16A16_SFLOAT
// AHARDWAREBUFFER_FORMAT_R10G10B10A2_UNORM     VK_FORMAT_A2B10G10R10_UNORM_PACK32
// AHARDWAREBUFFER_FORMAT_D16_UNORM             VK_FORMAT_D16_UNORM
// AHARDWAREBUFFER_FORMAT_D24_UNORM             VK_FORMAT_X8_D24_UNORM_PACK32
// AHARDWAREBUFFER_FORMAT_D24_UNORM_S8_UINT     VK_FORMAT_D24_UNORM_S8_UINT
// AHARDWAREBUFFER_FORMAT_D32_FLOAT             VK_FORMAT_D32_SFLOAT
// AHARDWAREBUFFER_FORMAT_D32_FLOAT_S8_UINT     VK_FORMAT_D32_SFLOAT_S8_UINT
// AHARDWAREBUFFER_FORMAT_S8_UINT               VK_FORMAT_S8_UINT

// The AHARDWAREBUFFER_FORMAT_* are an enum in the NDK headers, but get passed in to Vulkan
// as uint32_t. Casting the enums here avoids scattering casts around in the code.
std::map<uint32_t, VkFormat> ahb_format_map_a2v = {
    { (uint32_t)AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,        VK_FORMAT_R8G8B8A8_UNORM },
    { (uint32_t)AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM,        VK_FORMAT_R8G8B8A8_UNORM },
    { (uint32_t)AHARDWAREBUFFER_FORMAT_R8G8B8_UNORM,          VK_FORMAT_R8G8B8_UNORM },
    { (uint32_t)AHARDWAREBUFFER_FORMAT_R5G6B5_UNORM,          VK_FORMAT_R5G6B5_UNORM_PACK16 },
    { (uint32_t)AHARDWAREBUFFER_FORMAT_R16G16B16A16_FLOAT,    VK_FORMAT_R16G16B16A16_SFLOAT },
    { (uint32_t)AHARDWAREBUFFER_FORMAT_R10G10B10A2_UNORM,     VK_FORMAT_A2B10G10R10_UNORM_PACK32 },
    { (uint32_t)AHARDWAREBUFFER_FORMAT_D16_UNORM,             VK_FORMAT_D16_UNORM },
    { (uint32_t)AHARDWAREBUFFER_FORMAT_D24_UNORM,             VK_FORMAT_X8_D24_UNORM_PACK32 },
    { (uint32_t)AHARDWAREBUFFER_FORMAT_D24_UNORM_S8_UINT,     VK_FORMAT_D24_UNORM_S8_UINT },
    { (uint32_t)AHARDWAREBUFFER_FORMAT_D32_FLOAT,             VK_FORMAT_D32_SFLOAT },
    { (uint32_t)AHARDWAREBUFFER_FORMAT_D32_FLOAT_S8_UINT,     VK_FORMAT_D32_SFLOAT_S8_UINT },
    { (uint32_t)AHARDWAREBUFFER_FORMAT_S8_UINT,               VK_FORMAT_S8_UINT }
};

// AHardwareBuffer Usage                        Vulkan Usage or Creation Flag (Intermixed - Aargh!)
// =====================                        ===================================================
// None                                         VK_IMAGE_USAGE_TRANSFER_SRC_BIT
// None                                         VK_IMAGE_USAGE_TRANSFER_DST_BIT
// AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE      VK_IMAGE_USAGE_SAMPLED_BIT
// AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE      VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT
// AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
// AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
// AHARDWAREBUFFER_USAGE_GPU_CUBE_MAP           VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT
// AHARDWAREBUFFER_USAGE_GPU_MIPMAP_COMPLETE    None
// AHARDWAREBUFFER_USAGE_PROTECTED_CONTENT      VK_IMAGE_CREATE_PROTECTED_BIT
// None                                         VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT
// None                                         VK_IMAGE_CREATE_EXTENDED_USAGE_BIT
// AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER        VK_IMAGE_USAGE_STORAGE_BIT

// Same casting rationale. De-mixing the table to prevent type confusion and aliasing
std::map<uint64_t, VkImageUsageFlags> ahb_usage_map_a2v = {
    { (uint64_t)AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE,    (VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT) },
    { (uint64_t)AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER,     (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) },
    { (uint64_t)AHARDWAREBUFFER_USAGE_GPU_MIPMAP_COMPLETE,  0 },   // No equivalent
    { (uint64_t)AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER,  VK_IMAGE_USAGE_STORAGE_BIT },
};

std::map<uint64_t, VkImageCreateFlags> ahb_create_map_a2v = {
    { (uint64_t)AHARDWAREBUFFER_USAGE_GPU_CUBE_MAP,         VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT },
    { (uint64_t)AHARDWAREBUFFER_USAGE_PROTECTED_CONTENT,    VK_IMAGE_CREATE_PROTECTED_BIT },
    { (uint64_t)AHARDWAREBUFFER_USAGE_GPU_MIPMAP_COMPLETE,  0 },   // No equivalent
};

std::map<VkImageUsageFlags, uint64_t> ahb_usage_map_v2a = {
    { VK_IMAGE_USAGE_SAMPLED_BIT,           (uint64_t)AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE },
    { VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,  (uint64_t)AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE },
    { VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,  (uint64_t)AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER  },
    { VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,  (uint64_t)AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER  },
    { VK_IMAGE_USAGE_STORAGE_BIT,  (uint64_t)AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER  },
};

std::map<VkImageCreateFlags, uint64_t> ahb_create_map_v2a = {
    { VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,  (uint64_t)AHARDWAREBUFFER_USAGE_GPU_CUBE_MAP },
    { VK_IMAGE_CREATE_PROTECTED_BIT,        (uint64_t)AHARDWAREBUFFER_USAGE_PROTECTED_CONTENT },
};

// clang-format on

//
// AHB-extension new APIs
//
bool CoreChecks::PreCallValidateGetAndroidHardwareBufferPropertiesANDROID(
    VkDevice device, const struct AHardwareBuffer *buffer, VkAndroidHardwareBufferPropertiesANDROID *pProperties) const {
    bool skip = false;
    //  buffer must be a valid Android hardware buffer object with at least one of the AHARDWAREBUFFER_USAGE_GPU_* usage flags.
    AHardwareBuffer_Desc ahb_desc;
    AHardwareBuffer_describe(buffer, &ahb_desc);
    uint32_t required_flags = AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE | AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER |
                              AHARDWAREBUFFER_USAGE_GPU_CUBE_MAP | AHARDWAREBUFFER_USAGE_GPU_MIPMAP_COMPLETE |
                              AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER;
    if (0 == (ahb_desc.usage & required_flags)) {
        skip |= LogError(device, "VUID-vkGetAndroidHardwareBufferPropertiesANDROID-buffer-01884",
                         "vkGetAndroidHardwareBufferPropertiesANDROID: The AHardwareBuffer's AHardwareBuffer_Desc.usage (0x%" PRIx64
                         ") does not have any AHARDWAREBUFFER_USAGE_GPU_* flags set.",
                         ahb_desc.usage);
    }
    return skip;
}

bool CoreChecks::PreCallValidateGetMemoryAndroidHardwareBufferANDROID(VkDevice device,
                                                                      const VkMemoryGetAndroidHardwareBufferInfoANDROID *pInfo,
                                                                      struct AHardwareBuffer **pBuffer) const {
    bool skip = false;
    auto mem_info = Get<DEVICE_MEMORY_STATE>(pInfo->memory);

    // VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID must have been included in
    // VkExportMemoryAllocateInfo::handleTypes when memory was created.
    if (!mem_info->IsExport() ||
        (0 == (mem_info->export_handle_type_flags & VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID))) {
        skip |= LogError(device, "VUID-VkMemoryGetAndroidHardwareBufferInfoANDROID-handleTypes-01882",
                         "vkGetMemoryAndroidHardwareBufferANDROID: %s was not allocated for export, or the "
                         "export handleTypes (0x%" PRIx32
                         ") did not contain VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID.",
                         report_data->FormatHandle(pInfo->memory).c_str(), mem_info->export_handle_type_flags);
    }

    // If the pNext chain of the VkMemoryAllocateInfo used to allocate memory included a VkMemoryDedicatedAllocateInfo
    // with non-NULL image member, then that image must already be bound to memory.
    if (mem_info->IsDedicatedImage()) {
        auto image_state = Get<IMAGE_STATE>(mem_info->dedicated->handle.Cast<VkImage>());
        if ((nullptr == image_state) || (0 == (image_state->CountDeviceMemory(mem_info->mem())))) {
            const LogObjectList objlist(device, pInfo->memory, mem_info->dedicated->handle);
            skip |= LogError(objlist, "VUID-VkMemoryGetAndroidHardwareBufferInfoANDROID-pNext-01883",
                             "vkGetMemoryAndroidHardwareBufferANDROID: %s was allocated using a dedicated "
                             "%s, but that image is not bound to the VkDeviceMemory object.",
                             report_data->FormatHandle(pInfo->memory).c_str(),
                             report_data->FormatHandle(mem_info->dedicated->handle).c_str());
        }
    }

    return skip;
}

//
// AHB-specific validation within non-AHB APIs
//
bool CoreChecks::ValidateAllocateMemoryANDROID(const VkMemoryAllocateInfo *alloc_info) const {
    bool skip = false;
    auto import_ahb_info = LvlFindInChain<VkImportAndroidHardwareBufferInfoANDROID>(alloc_info->pNext);
    auto exp_mem_alloc_info = LvlFindInChain<VkExportMemoryAllocateInfo>(alloc_info->pNext);
    auto mem_ded_alloc_info = LvlFindInChain<VkMemoryDedicatedAllocateInfo>(alloc_info->pNext);

    if ((import_ahb_info) && (NULL != import_ahb_info->buffer)) {
        // This is an import with handleType of VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID
        AHardwareBuffer_Desc ahb_desc = {};
        AHardwareBuffer_describe(import_ahb_info->buffer, &ahb_desc);

        //  Validate AHardwareBuffer_Desc::usage is a valid usage for imported AHB
        //
        //  BLOB & GPU_DATA_BUFFER combo specifically allowed
        if ((AHARDWAREBUFFER_FORMAT_BLOB != ahb_desc.format) || (0 == (ahb_desc.usage & AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER))) {
            // Otherwise, must be a combination from the AHardwareBuffer Format and Usage Equivalence tables
            // Usage must have at least one bit from the table. It may have additional bits not in the table
            uint64_t ahb_equiv_usage_bits = AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE | AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER |
                                            AHARDWAREBUFFER_USAGE_GPU_CUBE_MAP | AHARDWAREBUFFER_USAGE_GPU_MIPMAP_COMPLETE |
                                            AHARDWAREBUFFER_USAGE_PROTECTED_CONTENT;
            if (0 == (ahb_desc.usage & ahb_equiv_usage_bits)) {
                skip |=
                    LogError(device, "VUID-VkImportAndroidHardwareBufferInfoANDROID-buffer-01881",
                             "vkAllocateMemory: The AHardwareBuffer_Desc's usage (0x%" PRIx64 ") is not compatible with Vulkan.",
                             ahb_desc.usage);
            }
        }

        // Collect external buffer info
        auto pdebi = LvlInitStruct<VkPhysicalDeviceExternalBufferInfo>();
        pdebi.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;
        if (AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE & ahb_desc.usage) {
            pdebi.usage |= ahb_usage_map_a2v[AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE];
        }
        if (AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER & ahb_desc.usage) {
            pdebi.usage |= ahb_usage_map_a2v[AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER];
        }
        auto ext_buf_props = LvlInitStruct<VkExternalBufferProperties>();
        DispatchGetPhysicalDeviceExternalBufferProperties(physical_device, &pdebi, &ext_buf_props);

        //  If buffer is not NULL, Android hardware buffers must be supported for import, as reported by
        //  VkExternalImageFormatProperties or VkExternalBufferProperties.
        if (0 == (ext_buf_props.externalMemoryProperties.externalMemoryFeatures & VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT)) {
            // Collect external format info
            auto pdeifi = LvlInitStruct<VkPhysicalDeviceExternalImageFormatInfo>();
            pdeifi.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;
            auto pdifi2 = LvlInitStruct<VkPhysicalDeviceImageFormatInfo2>(&pdeifi);
            if (0 < ahb_format_map_a2v.count(ahb_desc.format)) pdifi2.format = ahb_format_map_a2v[ahb_desc.format];
            pdifi2.type = VK_IMAGE_TYPE_2D;           // Seems likely
            pdifi2.tiling = VK_IMAGE_TILING_OPTIMAL;  // Ditto
            if (AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE & ahb_desc.usage) {
                pdifi2.usage |= ahb_usage_map_a2v[AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE];
            }
            if (AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER & ahb_desc.usage) {
                pdifi2.usage |= ahb_usage_map_a2v[AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER];
            }
            if (AHARDWAREBUFFER_USAGE_GPU_CUBE_MAP & ahb_desc.usage) {
                pdifi2.flags |= ahb_create_map_a2v[AHARDWAREBUFFER_USAGE_GPU_CUBE_MAP];
            }
            if (AHARDWAREBUFFER_USAGE_PROTECTED_CONTENT & ahb_desc.usage) {
                pdifi2.flags |= ahb_create_map_a2v[AHARDWAREBUFFER_USAGE_PROTECTED_CONTENT];
            }

            auto ext_img_fmt_props = LvlInitStruct<VkExternalImageFormatProperties>();
            auto ifp2 = LvlInitStruct<VkImageFormatProperties2>(&ext_img_fmt_props);

            VkResult fmt_lookup_result = DispatchGetPhysicalDeviceImageFormatProperties2(physical_device, &pdifi2, &ifp2);

            if ((VK_SUCCESS != fmt_lookup_result) || (0 == (ext_img_fmt_props.externalMemoryProperties.externalMemoryFeatures &
                                                            VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT))) {
                skip |= LogError(device, "VUID-VkImportAndroidHardwareBufferInfoANDROID-buffer-01880",
                                 "vkAllocateMemory: Neither the VkExternalImageFormatProperties nor the VkExternalBufferProperties "
                                 "structs for the AHardwareBuffer include the VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT flag.");
            }
        }

        // Retrieve buffer and format properties of the provided AHardwareBuffer
        auto ahb_props = LvlInitStruct<VkAndroidHardwareBufferPropertiesANDROID>();
        DispatchGetAndroidHardwareBufferPropertiesANDROID(device, import_ahb_info->buffer, &ahb_props);

        // allocationSize must be the size returned by vkGetAndroidHardwareBufferPropertiesANDROID for the Android hardware buffer
        if (alloc_info->allocationSize != ahb_props.allocationSize) {
            skip |= LogError(device, "VUID-VkMemoryAllocateInfo-allocationSize-02383",
                             "vkAllocateMemory: VkMemoryAllocateInfo struct with chained VkImportAndroidHardwareBufferInfoANDROID "
                             "struct, allocationSize (%" PRId64
                             ") does not match the AHardwareBuffer's reported allocationSize (%" PRId64 ").",
                             alloc_info->allocationSize, ahb_props.allocationSize);
        }

        // memoryTypeIndex must be one of those returned by vkGetAndroidHardwareBufferPropertiesANDROID for the AHardwareBuffer
        // Note: memoryTypeIndex is an index, memoryTypeBits is a bitmask
        uint32_t mem_type_bitmask = 1 << alloc_info->memoryTypeIndex;
        if (0 == (mem_type_bitmask & ahb_props.memoryTypeBits)) {
            skip |= LogError(device, "VUID-VkMemoryAllocateInfo-memoryTypeIndex-02385",
                             "vkAllocateMemory: VkMemoryAllocateInfo struct with chained VkImportAndroidHardwareBufferInfoANDROID "
                             "struct, memoryTypeIndex (%" PRId32
                             ") does not correspond to a bit set in AHardwareBuffer's reported "
                             "memoryTypeBits bitmask (0x%" PRIx32 ").",
                             alloc_info->memoryTypeIndex, ahb_props.memoryTypeBits);
        }

        // Checks for allocations without a dedicated allocation requirement
        if ((nullptr == mem_ded_alloc_info) || (VK_NULL_HANDLE == mem_ded_alloc_info->image)) {
            // the Android hardware buffer must have a format of AHARDWAREBUFFER_FORMAT_BLOB and a usage that includes
            // AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER
            if (((uint64_t)AHARDWAREBUFFER_FORMAT_BLOB != ahb_desc.format) ||
                (0 == (ahb_desc.usage & AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER))) {
                skip |= LogError(
                    device, "VUID-VkMemoryAllocateInfo-pNext-02384",
                    "vkAllocateMemory: VkMemoryAllocateInfo struct with chained VkImportAndroidHardwareBufferInfoANDROID "
                    "struct without a dedicated allocation requirement, while the AHardwareBuffer_Desc's format ( %u ) is not "
                    "AHARDWAREBUFFER_FORMAT_BLOB or usage (0x%" PRIx64 ") does not include AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER.",
                    ahb_desc.format, ahb_desc.usage);
            }
        } else {  // Checks specific to import with a dedicated allocation requirement

            // Dedicated allocation have limit usage flags supported
            if (0 == (ahb_desc.usage & (AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER | AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE |
                                        AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER))) {
                skip |= LogError(
                    device, "VUID-VkMemoryAllocateInfo-pNext-02386",
                    "vkAllocateMemory: VkMemoryAllocateInfo struct with chained VkImportAndroidHardwareBufferInfoANDROID and a "
                    "dedicated allocation requirement, while the AHardwareBuffer's usage (0x%" PRIx64
                    ") contains neither AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER, AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER, nor "
                    "AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE.",
                    ahb_desc.usage);
            }

            auto image_state = Get<IMAGE_STATE>(mem_ded_alloc_info->image);
            const auto *ici = &image_state->createInfo;

            //  the format of image must be VK_FORMAT_UNDEFINED or the format returned by
            //  vkGetAndroidHardwareBufferPropertiesANDROID
            if (VK_FORMAT_UNDEFINED != ici->format) {
                // Mali drivers will not return a valid VkAndroidHardwareBufferPropertiesANDROID::allocationSize if the
                // FormatPropertiesANDROID is passed in as well so need to query again for the format
                auto ahb_format_props = LvlInitStruct<VkAndroidHardwareBufferFormatPropertiesANDROID>();
                auto dummy_ahb_props = LvlInitStruct<VkAndroidHardwareBufferPropertiesANDROID>(&ahb_format_props);
                DispatchGetAndroidHardwareBufferPropertiesANDROID(device, import_ahb_info->buffer, &dummy_ahb_props);
                if (ici->format != ahb_format_props.format) {
                    skip |= LogError(device, "VUID-VkMemoryAllocateInfo-pNext-02387",
                                     "vkAllocateMemory: VkMemoryAllocateInfo struct with chained "
                                     "VkImportAndroidHardwareBufferInfoANDROID, the dedicated allocation image's "
                                     "format (%s) is not VK_FORMAT_UNDEFINED and does not match the AHardwareBuffer's format (%s).",
                                     string_VkFormat(ici->format), string_VkFormat(ahb_format_props.format));
                }
            }

            // The width, height, and array layer dimensions of image and the Android hardwarebuffer must be identical
            if ((ici->extent.width != ahb_desc.width) || (ici->extent.height != ahb_desc.height) ||
                (ici->arrayLayers != ahb_desc.layers)) {
                skip |= LogError(device, "VUID-VkMemoryAllocateInfo-pNext-02388",
                                 "vkAllocateMemory: VkMemoryAllocateInfo struct with chained "
                                 "VkImportAndroidHardwareBufferInfoANDROID, the dedicated allocation image's "
                                 "width, height, and arrayLayers (%" PRId32 " %" PRId32 " %" PRId32
                                 ") do not match those of the AHardwareBuffer (%" PRId32 " %" PRId32 " %" PRId32 ").",
                                 ici->extent.width, ici->extent.height, ici->arrayLayers, ahb_desc.width, ahb_desc.height,
                                 ahb_desc.layers);
            }

            // If the Android hardware buffer's usage includes AHARDWAREBUFFER_USAGE_GPU_MIPMAP_COMPLETE, the image must
            // have either a full mipmap chain or exactly 1 mip level.
            //
            // NOTE! The language of this VUID contradicts the language in the spec (1.1.93), which says "The
            // AHARDWAREBUFFER_USAGE_GPU_MIPMAP_COMPLETE flag does not correspond to a Vulkan image usage or creation flag. Instead,
            // its presence indicates that the Android hardware buffer contains a complete mipmap chain, and its absence indicates
            // that the Android hardware buffer contains only a single mip level."
            //
            // TODO: This code implements the VUID's meaning, but it seems likely that the spec text is actually correct.
            // Clarification requested.
            if ((ahb_desc.usage & AHARDWAREBUFFER_USAGE_GPU_MIPMAP_COMPLETE) && (ici->mipLevels != 1) &&
                (ici->mipLevels != FullMipChainLevels(ici->extent))) {
                skip |=
                    LogError(device, "VUID-VkMemoryAllocateInfo-pNext-02389",
                             "vkAllocateMemory: VkMemoryAllocateInfo struct with chained VkImportAndroidHardwareBufferInfoANDROID, "
                             "usage includes AHARDWAREBUFFER_USAGE_GPU_MIPMAP_COMPLETE but mipLevels (%" PRId32
                             ") is neither 1 nor full mip "
                             "chain levels (%" PRId32 ").",
                             ici->mipLevels, FullMipChainLevels(ici->extent));
            }

            // each bit set in the usage of image must be listed in AHardwareBuffer Usage Equivalence, and if there is a
            // corresponding AHARDWAREBUFFER_USAGE bit listed that bit must be included in the Android hardware buffer's
            // AHardwareBuffer_Desc::usage
            if (ici->usage & ~(VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT |
                               VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                               VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
                skip |=
                    LogError(device, "VUID-VkMemoryAllocateInfo-pNext-02390",
                             "vkAllocateMemory: VkMemoryAllocateInfo struct with chained VkImportAndroidHardwareBufferInfoANDROID, "
                             "dedicated image usage bits (0x%" PRIx32
                             ") include an issue not listed in the AHardwareBuffer Usage Equivalence table.",
                             ici->usage);
            }

            std::vector<VkImageUsageFlags> usages = {VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
                                                     VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                                     VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT};
            for (VkImageUsageFlags ubit : usages) {
                if (ici->usage & ubit) {
                    uint64_t ahb_usage = ahb_usage_map_v2a[ubit];
                    if (0 == (ahb_usage & ahb_desc.usage)) {
                        skip |= LogError(
                            device, "VUID-VkMemoryAllocateInfo-pNext-02390",
                            "vkAllocateMemory: VkMemoryAllocateInfo struct with chained VkImportAndroidHardwareBufferInfoANDROID, "
                            "The dedicated image usage bit %s equivalent is not in AHardwareBuffer_Desc.usage (0x%" PRIx64 ") ",
                            string_VkImageUsageFlags(ubit).c_str(), ahb_desc.usage);
                    }
                }
            }
        }
    } else {  // Not an import
        if ((exp_mem_alloc_info) && (mem_ded_alloc_info) &&
            (0 != (VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID & exp_mem_alloc_info->handleTypes)) &&
            (VK_NULL_HANDLE != mem_ded_alloc_info->image)) {
            // This is an Android HW Buffer export
            if (0 != alloc_info->allocationSize) {
                skip |= LogError(device, "VUID-VkMemoryAllocateInfo-pNext-01874",
                                 "vkAllocateMemory: pNext chain indicates a dedicated Android Hardware Buffer export allocation, "
                                 "but allocationSize is non-zero.");
            }
        } else {
            if (0 == alloc_info->allocationSize) {
                skip |= LogError(
                    device, "VUID-VkMemoryAllocateInfo-pNext-01874",
                    "vkAllocateMemory: pNext chain does not indicate a dedicated export allocation, but allocationSize is 0.");
            };
        }
    }
    return skip;
}

bool CoreChecks::ValidateGetImageMemoryRequirementsANDROID(const VkImage image, const char *func_name) const {
    bool skip = false;

    auto image_state = Get<IMAGE_STATE>(image);
    if (image_state != nullptr) {
        if (image_state->IsExternalAHB() && (0 == image_state->GetBoundMemoryStates().size())) {
            const char *vuid = strcmp(func_name, "vkGetImageMemoryRequirements()") == 0
                                   ? "VUID-vkGetImageMemoryRequirements-image-04004"
                                   : "VUID-VkImageMemoryRequirementsInfo2-image-01897";
            skip |=
                LogError(image, vuid,
                         "%s: Attempt get image memory requirements for an image created with a "
                         "VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID handleType, which has not yet been "
                         "bound to memory.",
                         func_name);
        }
    }
    return skip;
}

bool CoreChecks::ValidateGetPhysicalDeviceImageFormatProperties2ANDROID(
    const VkPhysicalDeviceImageFormatInfo2 *pImageFormatInfo, const VkImageFormatProperties2 *pImageFormatProperties) const {
    bool skip = false;
    const VkAndroidHardwareBufferUsageANDROID *ahb_usage =
        LvlFindInChain<VkAndroidHardwareBufferUsageANDROID>(pImageFormatProperties->pNext);
    if (nullptr != ahb_usage) {
        const VkPhysicalDeviceExternalImageFormatInfo *pdeifi =
            LvlFindInChain<VkPhysicalDeviceExternalImageFormatInfo>(pImageFormatInfo->pNext);
        if ((nullptr == pdeifi) || (VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID != pdeifi->handleType)) {
            skip |= LogError(device, "VUID-vkGetPhysicalDeviceImageFormatProperties2-pNext-01868",
                             "vkGetPhysicalDeviceImageFormatProperties2: pImageFormatProperties includes a chained "
                             "VkAndroidHardwareBufferUsageANDROID struct, but pImageFormatInfo does not include a chained "
                             "VkPhysicalDeviceExternalImageFormatInfo struct with handleType "
                             "VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID.");
        }
    }
    return skip;
}

bool CoreChecks::ValidateBufferImportedHandleANDROID(const char *func_name, VkExternalMemoryHandleTypeFlags handleType,
                                                     VkDeviceMemory memory, VkBuffer buffer) const {
    bool skip = false;
    if ((handleType & VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID) == 0) {
        const char *vuid = (strcmp(func_name, "vkBindBufferMemory()") == 0) ? "VUID-vkBindBufferMemory-memory-02986"
                                                                            : "VUID-VkBindBufferMemoryInfo-memory-02986";
        const LogObjectList objlist(buffer, memory);
        skip |= LogError(objlist, vuid,
                         "%s: The VkDeviceMemory (%s) was created with an AHB import operation which is not set "
                         "VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID in the VkBuffer (%s) "
                         "VkExternalMemoryBufferreateInfo::handleType (%s)",
                         func_name, report_data->FormatHandle(memory).c_str(), report_data->FormatHandle(buffer).c_str(),
                         string_VkExternalMemoryHandleTypeFlags(handleType).c_str());
    }
    return skip;
}

bool CoreChecks::ValidateImageImportedHandleANDROID(const char *func_name, VkExternalMemoryHandleTypeFlags handleType,
                                                    VkDeviceMemory memory, VkImage image) const {
    bool skip = false;
    if ((handleType & VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID) == 0) {
        const char *vuid = (strcmp(func_name, "vkBindImageMemory()") == 0) ? "VUID-vkBindImageMemory-memory-02990"
                                                                           : "VUID-VkBindImageMemoryInfo-memory-02990";
        const LogObjectList objlist(image, memory);
        skip |= LogError(objlist, vuid,
                         "%s: The VkDeviceMemory (%s) was created with an AHB import operation which is not set "
                         "VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID in the VkImage (%s) "
                         "VkExternalMemoryImageCreateInfo::handleType (%s)",
                         func_name, report_data->FormatHandle(memory).c_str(), report_data->FormatHandle(image).c_str(),
                         string_VkExternalMemoryHandleTypeFlags(handleType).c_str());
    }
    return skip;
}

// Validate creating an image with an external format
// This could be wrapped with VK_USE_PLATFORM_ANDROID_KHR instead of AHB_VALIDATION_SUPPORT, but this check is only for AHB
bool CoreChecks::ValidateCreateImageANDROID(const debug_report_data *report_data, const VkImageCreateInfo *create_info) const {
    bool skip = false;

    const VkExternalFormatANDROID *ext_fmt_android = LvlFindInChain<VkExternalFormatANDROID>(create_info->pNext);
    if (ext_fmt_android) {
        if (0 != ext_fmt_android->externalFormat) {
            if (VK_FORMAT_UNDEFINED != create_info->format) {
                skip |=
                    LogError(device, "VUID-VkImageCreateInfo-pNext-01974",
                             "vkCreateImage(): VkImageCreateInfo struct has a chained VkExternalFormatANDROID struct with non-zero "
                             "externalFormat, but the VkImageCreateInfo's format is not VK_FORMAT_UNDEFINED.");
            }

            if (0 != (VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT & create_info->flags)) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-pNext-02396",
                                 "vkCreateImage(): VkImageCreateInfo struct has a chained VkExternalFormatANDROID struct with "
                                 "non-zero externalFormat, but flags include VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT.");
            }

            if (0 != (~VK_IMAGE_USAGE_SAMPLED_BIT & create_info->usage)) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-pNext-02397",
                                 "vkCreateImage(): VkImageCreateInfo struct has a chained VkExternalFormatANDROID struct with "
                                 "non-zero externalFormat, but usage includes bits (0x%" PRIx32
                                 ") other than VK_IMAGE_USAGE_SAMPLED_BIT.",
                                 create_info->usage);
            }

            if (VK_IMAGE_TILING_OPTIMAL != create_info->tiling) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-pNext-02398",
                                 "vkCreateImage(): VkImageCreateInfo struct has a chained VkExternalFormatANDROID struct with "
                                 "non-zero externalFormat, but layout is not VK_IMAGE_TILING_OPTIMAL.");
            }
        }

        if ((0 != ext_fmt_android->externalFormat) &&
            (ahb_ext_formats_map.find(ext_fmt_android->externalFormat) == ahb_ext_formats_map.end())) {
            skip |= LogError(device, "VUID-VkExternalFormatANDROID-externalFormat-01894",
                             "vkCreateImage(): Chained VkExternalFormatANDROID struct contains a non-zero externalFormat (%" PRIu64
                             ") which has "
                             "not been previously retrieved by vkGetAndroidHardwareBufferPropertiesANDROID().",
                             ext_fmt_android->externalFormat);
        }
    }

    if ((nullptr == ext_fmt_android) || (0 == ext_fmt_android->externalFormat)) {
        if (VK_FORMAT_UNDEFINED == create_info->format) {
            skip |=
                LogError(device, "VUID-VkImageCreateInfo-pNext-01975",
                         "vkCreateImage(): VkImageCreateInfo struct's format is VK_FORMAT_UNDEFINED, but either does not have a "
                         "chained VkExternalFormatANDROID struct or the struct exists but has an externalFormat of 0.");
        }
    }

    const VkExternalMemoryImageCreateInfo *emici = LvlFindInChain<VkExternalMemoryImageCreateInfo>(create_info->pNext);
    if (emici && (emici->handleTypes & VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID)) {
        if (create_info->imageType != VK_IMAGE_TYPE_2D) {
            skip |=
                LogError(device, "VUID-VkImageCreateInfo-pNext-02393",
                         "vkCreateImage(): VkImageCreateInfo struct with imageType %s has chained VkExternalMemoryImageCreateInfo "
                         "struct with handleType VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID.",
                         string_VkImageType(create_info->imageType));
        }

        if ((create_info->mipLevels != 1) && (create_info->mipLevels != FullMipChainLevels(create_info->extent))) {
            skip |= LogError(device, "VUID-VkImageCreateInfo-pNext-02394",
                             "vkCreateImage(): VkImageCreateInfo struct with chained VkExternalMemoryImageCreateInfo struct of "
                             "handleType VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID "
                             "specifies mipLevels = %" PRId32 " (full chain mipLevels are %" PRId32 ").",
                             create_info->mipLevels, FullMipChainLevels(create_info->extent));
        }
    }

    return skip;
}

// Validate creating an image view with an AHB format
// This could be wrapped with VK_USE_PLATFORM_ANDROID_KHR instead of AHB_VALIDATION_SUPPORT, but this check is only for AHB
bool CoreChecks::ValidateCreateImageViewANDROID(const VkImageViewCreateInfo *create_info) const {
    bool skip = false;
    auto image_state = Get<IMAGE_STATE>(create_info->image);

    if (image_state->HasAHBFormat()) {
        if (VK_FORMAT_UNDEFINED != create_info->format) {
            skip |= LogError(create_info->image, "VUID-VkImageViewCreateInfo-image-02399",
                             "vkCreateImageView(): VkImageViewCreateInfo struct has a chained VkExternalFormatANDROID struct, but "
                             "format member is %s and must be VK_FORMAT_UNDEFINED.",
                             string_VkFormat(create_info->format));
        }

        // Chain must include a compatible ycbcr conversion
        bool conv_found = false;
        uint64_t external_format = 0;
        const VkSamplerYcbcrConversionInfo *ycbcr_conv_info = LvlFindInChain<VkSamplerYcbcrConversionInfo>(create_info->pNext);
        if (ycbcr_conv_info != nullptr) {
            auto ycbcr_state = Get<SAMPLER_YCBCR_CONVERSION_STATE>(ycbcr_conv_info->conversion);
            if (ycbcr_state) {
                conv_found = true;
                external_format = ycbcr_state->external_format;
            }
        }
        if ((!conv_found) || (external_format != image_state->ahb_format)) {
            skip |= LogError(create_info->image, "VUID-VkImageViewCreateInfo-image-02400",
                             "vkCreateImageView(): VkImageViewCreateInfo struct has a chained VkExternalFormatANDROID struct with "
                             "an externalFormat (%" PRIu64
                             ") but needs a chained VkSamplerYcbcrConversionInfo struct with a VkSamplerYcbcrConversion created "
                             "with the same external format.",
                             image_state->ahb_format);
        }

        // Errors in create_info swizzles
        if (IsIdentitySwizzle(create_info->components) == false) {
            skip |= LogError(
                create_info->image, "VUID-VkImageViewCreateInfo-image-02401",
                "vkCreateImageView(): VkImageViewCreateInfo struct has a chained VkExternalFormatANDROID struct, but "
                "includes one or more non-identity component swizzles, r swizzle = %s, g swizzle = %s, b swizzle = %s, a swizzle "
                "= %s.",
                string_VkComponentSwizzle(create_info->components.r), string_VkComponentSwizzle(create_info->components.g),
                string_VkComponentSwizzle(create_info->components.b), string_VkComponentSwizzle(create_info->components.a));
        }
    }

    return skip;
}

#else  // !AHB_VALIDATION_SUPPORT

// Case building for Android without AHB Validation
#ifdef VK_USE_PLATFORM_ANDROID_KHR
bool CoreChecks::PreCallValidateGetAndroidHardwareBufferPropertiesANDROID(
    VkDevice device, const struct AHardwareBuffer *buffer, VkAndroidHardwareBufferPropertiesANDROID *pProperties) const {
    return false;
}
bool CoreChecks::PreCallValidateGetMemoryAndroidHardwareBufferANDROID(VkDevice device,
                                                                      const VkMemoryGetAndroidHardwareBufferInfoANDROID *pInfo,
                                                                      struct AHardwareBuffer **pBuffer) const {
    return false;
}
#endif  // VK_USE_PLATFORM_ANDROID_KHR

bool CoreChecks::ValidateAllocateMemoryANDROID(const VkMemoryAllocateInfo *alloc_info) const { return false; }

bool CoreChecks::ValidateGetPhysicalDeviceImageFormatProperties2ANDROID(
    const VkPhysicalDeviceImageFormatInfo2 *pImageFormatInfo, const VkImageFormatProperties2 *pImageFormatProperties) const {
    return false;
}

bool CoreChecks::ValidateGetImageMemoryRequirementsANDROID(const VkImage image, const char *func_name) const { return false; }

bool CoreChecks::ValidateBufferImportedHandleANDROID(const char *func_name, VkExternalMemoryHandleTypeFlags handleType,
                                                     VkDeviceMemory memory, VkBuffer buffer) const {
    return false;
}

bool CoreChecks::ValidateImageImportedHandleANDROID(const char *func_name, VkExternalMemoryHandleTypeFlags handleType,
                                                    VkDeviceMemory memory, VkImage image) const {
    return false;
}

bool CoreChecks::ValidateCreateImageANDROID(const debug_report_data *report_data, const VkImageCreateInfo *create_info) const {
    return false;
}

bool CoreChecks::ValidateCreateImageViewANDROID(const VkImageViewCreateInfo *create_info) const { return false; }

#endif  // AHB_VALIDATION_SUPPORT
