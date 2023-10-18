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

#include "utils/vk_layer_utils.h"
#include <vulkan/vk_enum_string_helper.h>
#include "core_validation.h"

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
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
bool CoreChecks::PreCallValidateGetAndroidHardwareBufferPropertiesANDROID(VkDevice device, const struct AHardwareBuffer *buffer,
                                                                          VkAndroidHardwareBufferPropertiesANDROID *pProperties,
                                                                          const ErrorObject &error_obj) const {
    bool skip = false;
    //  buffer must be a valid Android hardware buffer object with at least one of the AHARDWAREBUFFER_USAGE_GPU_* usage flags.
    AHardwareBuffer_Desc ahb_desc;
    AHardwareBuffer_describe(buffer, &ahb_desc);
    uint32_t required_flags = AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE | AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER |
                              AHARDWAREBUFFER_USAGE_GPU_CUBE_MAP | AHARDWAREBUFFER_USAGE_GPU_MIPMAP_COMPLETE |
                              AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER;
    if (0 == (ahb_desc.usage & required_flags)) {
        skip |= LogError(
            "VUID-vkGetAndroidHardwareBufferPropertiesANDROID-buffer-01884", device, error_obj.location.dot(Field::buffer),
            "AHardwareBuffer_Desc.usage (0x%" PRIx64 ") does not have any AHARDWAREBUFFER_USAGE_GPU_* flags set. (AHB = %p).",
            ahb_desc.usage, buffer);
    }
    return skip;
}

bool CoreChecks::PreCallValidateGetMemoryAndroidHardwareBufferANDROID(VkDevice device,
                                                                      const VkMemoryGetAndroidHardwareBufferInfoANDROID *pInfo,
                                                                      struct AHardwareBuffer **pBuffer,
                                                                      const ErrorObject &error_obj) const {
    bool skip = false;
    auto mem_info = Get<DEVICE_MEMORY_STATE>(pInfo->memory);

    // VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID must have been included in
    // VkExportMemoryAllocateInfo::handleTypes when memory was created.
    if (!mem_info->IsExport() ||
        (0 == (mem_info->export_handle_types & VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID))) {
        skip |= LogError("VUID-VkMemoryGetAndroidHardwareBufferInfoANDROID-handleTypes-01882", device,
                         error_obj.location.dot(Field::pInfo).dot(Field::memory),
                         "(%s) was not allocated for export, or the "
                         "export handleTypes (0x%" PRIx32
                         ") did not contain VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID.",
                         FormatHandle(pInfo->memory).c_str(), mem_info->export_handle_types);
    }

    // If the pNext chain of the VkMemoryAllocateInfo used to allocate memory included a VkMemoryDedicatedAllocateInfo
    // with non-NULL image member, then that image must already be bound to memory.
    if (mem_info->IsDedicatedImage()) {
        auto image_state = Get<IMAGE_STATE>(mem_info->dedicated->handle.Cast<VkImage>());
        if ((nullptr == image_state) || (0 == (image_state->CountDeviceMemory(mem_info->deviceMemory())))) {
            const LogObjectList objlist(device, pInfo->memory, mem_info->dedicated->handle);
            skip |= LogError("VUID-VkMemoryGetAndroidHardwareBufferInfoANDROID-pNext-01883", objlist,
                             error_obj.location.dot(Field::pInfo).dot(Field::memory),
                             "(%s) was allocated using a dedicated "
                             "%s, but that image is not bound to the VkDeviceMemory object.",
                             FormatHandle(pInfo->memory).c_str(), FormatHandle(mem_info->dedicated->handle).c_str());
        }
    }

    return skip;
}

//
// AHB-specific validation within non-AHB APIs
//
bool CoreChecks::ValidateAllocateMemoryANDROID(const VkMemoryAllocateInfo *allocate_info, const Location &allocate_info_loc) const {
    bool skip = false;
    auto import_ahb_info = vku::FindStructInPNextChain<VkImportAndroidHardwareBufferInfoANDROID>(allocate_info->pNext);
    auto exp_mem_alloc_info = vku::FindStructInPNextChain<VkExportMemoryAllocateInfo>(allocate_info->pNext);
    auto mem_ded_alloc_info = vku::FindStructInPNextChain<VkMemoryDedicatedAllocateInfo>(allocate_info->pNext);

    if ((import_ahb_info) && (NULL != import_ahb_info->buffer)) {
        const Location ahb_loc = allocate_info_loc.dot(Struct::VkImportAndroidHardwareBufferInfoANDROID, Field::buffer);

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
                skip |= LogError("VUID-VkImportAndroidHardwareBufferInfoANDROID-buffer-01881", device, ahb_loc,
                                 "AHardwareBuffer_Desc's usage (0x%" PRIx64 ") is not compatible with Vulkan. (AHB = %p).",
                                 ahb_desc.usage, import_ahb_info->buffer);
            }
        }

        // Collect external buffer info
        VkPhysicalDeviceExternalBufferInfo pdebi = vku::InitStructHelper();
        pdebi.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;
        if (AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE & ahb_desc.usage) {
            pdebi.usage |= ahb_usage_map_a2v[AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE];
        }
        if (AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER & ahb_desc.usage) {
            pdebi.usage |= ahb_usage_map_a2v[AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER];
        }
        VkExternalBufferProperties ext_buf_props = vku::InitStructHelper();
        DispatchGetPhysicalDeviceExternalBufferProperties(physical_device, &pdebi, &ext_buf_props);

        //  If buffer is not NULL, Android hardware buffers must be supported for import, as reported by
        //  VkExternalImageFormatProperties or VkExternalBufferProperties.
        if (0 == (ext_buf_props.externalMemoryProperties.externalMemoryFeatures & VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT)) {
            // Collect external format info
            VkPhysicalDeviceExternalImageFormatInfo pdeifi = vku::InitStructHelper();
            pdeifi.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;
            VkPhysicalDeviceImageFormatInfo2 pdifi2 = vku::InitStructHelper(&pdeifi);
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

            VkExternalImageFormatProperties ext_img_fmt_props = vku::InitStructHelper();
            VkImageFormatProperties2 ifp2 = vku::InitStructHelper(&ext_img_fmt_props);

            VkResult fmt_lookup_result = DispatchGetPhysicalDeviceImageFormatProperties2(physical_device, &pdifi2, &ifp2);

            if ((VK_SUCCESS != fmt_lookup_result) || (0 == (ext_img_fmt_props.externalMemoryProperties.externalMemoryFeatures &
                                                            VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT))) {
                skip |= LogError(
                    "VUID-VkImportAndroidHardwareBufferInfoANDROID-buffer-01880", device, allocate_info_loc,
                    "Neither the VkExternalImageFormatProperties nor the VkExternalBufferProperties "
                    "structs for the AHardwareBuffer include the VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT flag. (AHB = %p).",
                    import_ahb_info->buffer);
            }
        }

        // Retrieve buffer and format properties of the provided AHardwareBuffer
        VkAndroidHardwareBufferPropertiesANDROID ahb_props = vku::InitStructHelper();
        DispatchGetAndroidHardwareBufferPropertiesANDROID(device, import_ahb_info->buffer, &ahb_props);

        // allocationSize must be the size returned by vkGetAndroidHardwareBufferPropertiesANDROID for the Android hardware buffer
        if (allocate_info->allocationSize != ahb_props.allocationSize) {
            skip |= LogError("VUID-VkMemoryAllocateInfo-allocationSize-02383", device, allocate_info_loc.dot(Field::allocationSize),
                             "(%" PRIu64 ") does not match the %s AHardwareBuffer's allocationSize (%" PRIu64 "). (AHB = %p).",
                             allocate_info->allocationSize, ahb_loc.Fields().c_str(), ahb_props.allocationSize,
                             import_ahb_info->buffer);
        }

        // memoryTypeIndex must be one of those returned by vkGetAndroidHardwareBufferPropertiesANDROID for the AHardwareBuffer
        // Note: memoryTypeIndex is an index, memoryTypeBits is a bitmask
        uint32_t mem_type_bitmask = 1 << allocate_info->memoryTypeIndex;
        if (0 == (mem_type_bitmask & ahb_props.memoryTypeBits)) {
            skip |= LogError(
                "VUID-VkMemoryAllocateInfo-memoryTypeIndex-02385", device, allocate_info_loc.dot(Field::memoryTypeIndex),
                "(%" PRIu32
                ") does not correspond to a bit set in %s "
                "AHardwareBuffer's memoryTypeBits bitmask (0x%" PRIx32 "). (AHB = %p).",
                allocate_info->memoryTypeIndex, ahb_loc.Fields().c_str(), ahb_props.memoryTypeBits, import_ahb_info->buffer);
        }

        // Checks for allocations without a dedicated allocation requirement
        if ((nullptr == mem_ded_alloc_info) || (VK_NULL_HANDLE == mem_ded_alloc_info->image)) {
            // the Android hardware buffer must have a format of AHARDWAREBUFFER_FORMAT_BLOB and a usage that includes
            // AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER
            if (((uint64_t)AHARDWAREBUFFER_FORMAT_BLOB != ahb_desc.format) ||
                (0 == (ahb_desc.usage & AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER))) {
                skip |= LogError("VUID-VkMemoryAllocateInfo-pNext-02384", device, ahb_loc,
                                 "AHardwareBuffer_Desc's format ( %u ) is not "
                                 "AHARDWAREBUFFER_FORMAT_BLOB or usage (0x%" PRIx64
                                 ") does not include AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER. (AHB = %p).",
                                 ahb_desc.format, ahb_desc.usage, import_ahb_info->buffer);
            }
        } else {  // Checks specific to import with a dedicated allocation requirement

            // Dedicated allocation have limit usage flags supported
            if (0 == (ahb_desc.usage & (AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER | AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE |
                                        AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER))) {
                skip |= LogError("VUID-VkMemoryAllocateInfo-pNext-02386", device, ahb_loc,
                                 "AHardwareBuffer's usage is 0x%" PRIx64 ". (AHB = %p).", ahb_desc.usage, import_ahb_info->buffer);
            }

            auto image_state = Get<IMAGE_STATE>(mem_ded_alloc_info->image);
            const auto *ici = &image_state->createInfo;
            const Location &dedicated_image_loc = allocate_info_loc.dot(Struct::VkMemoryDedicatedAllocateInfo, Field::image);

            //  the format of image must be VK_FORMAT_UNDEFINED or the format returned by
            //  vkGetAndroidHardwareBufferPropertiesANDROID
            if (VK_FORMAT_UNDEFINED != ici->format) {
                // Mali drivers will not return a valid VkAndroidHardwareBufferPropertiesANDROID::allocationSize if the
                // FormatPropertiesANDROID is passed in as well so need to query again for the format
                VkAndroidHardwareBufferFormatPropertiesANDROID ahb_format_props = vku::InitStructHelper();
                VkAndroidHardwareBufferPropertiesANDROID dummy_ahb_props = vku::InitStructHelper(&ahb_format_props);
                DispatchGetAndroidHardwareBufferPropertiesANDROID(device, import_ahb_info->buffer, &dummy_ahb_props);
                if (ici->format != ahb_format_props.format) {
                    skip |= LogError(
                        "VUID-VkMemoryAllocateInfo-pNext-02387", mem_ded_alloc_info->image, dedicated_image_loc,
                        "was created with format (%s) which does not match the %s AHardwareBuffer's format (%s). (AHB = %p).",
                        string_VkFormat(ici->format), ahb_loc.Fields().c_str(), string_VkFormat(ahb_format_props.format),
                        import_ahb_info->buffer);
                }
            }

            // The width, height, and array layer dimensions of image and the Android hardwarebuffer must be identical
            if ((ici->extent.width != ahb_desc.width) || (ici->extent.height != ahb_desc.height) ||
                (ici->arrayLayers != ahb_desc.layers)) {
                skip |=
                    LogError("VUID-VkMemoryAllocateInfo-pNext-02388", mem_ded_alloc_info->image, dedicated_image_loc,
                             "was created with width (%" PRId32 "),  height (%" PRId32 "), and arrayLayers (%" PRId32
                             ") which not match those of the %s AHardwareBuffer (%" PRId32 " %" PRId32 " %" PRId32 "). (AHB = %p).",
                             ici->extent.width, ici->extent.height, ici->arrayLayers, ahb_loc.Fields().c_str(), ahb_desc.width,
                             ahb_desc.height, ahb_desc.layers, import_ahb_info->buffer);
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
            if ((ahb_desc.usage & AHARDWAREBUFFER_USAGE_GPU_MIPMAP_COMPLETE) != 0) {
                if ((ici->mipLevels != 1) && (ici->mipLevels != FullMipChainLevels(ici->extent))) {
                    skip |= LogError(
                        "VUID-VkMemoryAllocateInfo-pNext-02389", mem_ded_alloc_info->image, ahb_loc,
                        "AHardwareBuffer_Desc's usage includes AHARDWAREBUFFER_USAGE_GPU_MIPMAP_COMPLETE but %s mipLevels (%" PRIu32
                        ") is neither 1 nor full mip "
                        "chain levels (%" PRIu32 "). (AHB = %p).",
                        dedicated_image_loc.Fields().c_str(), ici->mipLevels, FullMipChainLevels(ici->extent),
                        import_ahb_info->buffer);
                }
            } else {
                if (ici->mipLevels != 1) {
                    skip |= LogError("VUID-VkMemoryAllocateInfo-pNext-02586", mem_ded_alloc_info->image, ahb_loc,
                                     "AHardwareBuffer_Desc's  usage is 0x%" PRIx64 " but %s mipLevels is %" PRIu32 ". (AHB = %p).",
                                     ahb_desc.usage, dedicated_image_loc.Fields().c_str(), ici->mipLevels, import_ahb_info->buffer);
                }
            }

            // each bit set in the usage of image must be listed in AHardwareBuffer Usage Equivalence, and if there is a
            // corresponding AHARDWAREBUFFER_USAGE bit listed that bit must be included in the Android hardware buffer's
            // AHardwareBuffer_Desc::usage
            if (ici->usage & ~(VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT |
                               VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                               VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
                skip |= LogError(
                    "VUID-VkMemoryAllocateInfo-pNext-02390", mem_ded_alloc_info->image, dedicated_image_loc,
                    "usage bits (%s) include an issue not listed in the AHardwareBuffer Usage Equivalence table. (AHB = %p).",
                    string_VkImageUsageFlags(ici->usage).c_str(), import_ahb_info->buffer);
            }

            std::vector<VkImageUsageFlags> usages = {VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
                                                     VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                                     VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT};
            for (VkImageUsageFlags ubit : usages) {
                if (ici->usage & ubit) {
                    uint64_t ahb_usage = ahb_usage_map_v2a[ubit];
                    if (0 == (ahb_usage & ahb_desc.usage)) {
                        skip |=
                            LogError("VUID-VkMemoryAllocateInfo-pNext-02390", mem_ded_alloc_info->image, dedicated_image_loc,
                                     " usage bit %s equivalent is not in AHardwareBuffer_Desc.usage (0x%" PRIx64 "). (AHB = %p).",
                                     string_VkImageUsageFlags(ubit).c_str(), ahb_desc.usage, import_ahb_info->buffer);
                    }
                }
            }
        }
    } else {  // Not an import
              // auto exp_mem_alloc_info = vku::FindStructInPNextChain<VkExportMemoryAllocateInfo>(allocate_info->pNext);
              // auto mem_ded_alloc_info = vku::FindStructInPNextChain<VkMemoryDedicatedAllocateInfo>(allocate_info->pNext);

        if (exp_mem_alloc_info &&
            (VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID & exp_mem_alloc_info->handleTypes)) {
            if (mem_ded_alloc_info) {
                if (mem_ded_alloc_info->image != VK_NULL_HANDLE && allocate_info->allocationSize != 0) {
                    skip |= LogError("VUID-VkMemoryAllocateInfo-pNext-01874", mem_ded_alloc_info->image,
                                     allocate_info_loc.pNext(Struct::VkMemoryDedicatedAllocateInfo, Field::image),
                                     "is %s but allocationSize is %" PRIu64 ".", FormatHandle(mem_ded_alloc_info->image).c_str(),
                                     allocate_info->allocationSize);
                }
                if (mem_ded_alloc_info->buffer != VK_NULL_HANDLE && allocate_info->allocationSize == 0) {
                    skip |= LogError("VUID-VkMemoryAllocateInfo-pNext-07901", mem_ded_alloc_info->buffer,
                                     allocate_info_loc.pNext(Struct::VkMemoryDedicatedAllocateInfo, Field::buffer),
                                     "is %s but allocationSize is 0.", FormatHandle(mem_ded_alloc_info->buffer).c_str());
                }
            } else if (0 == allocate_info->allocationSize) {
                skip |= LogError("VUID-VkMemoryAllocateInfo-pNext-07900", device, allocate_info_loc,
                                 "pNext chain does not include VkMemoryDedicatedAllocateInfo, but allocationSize is 0.");
            }
        }
    }
    return skip;
}

bool CoreChecks::ValidateGetImageMemoryRequirementsANDROID(const VkImage image, const Location &loc) const {
    bool skip = false;

    auto image_state = Get<IMAGE_STATE>(image);
    if (image_state != nullptr) {
        if (image_state->IsExternalBuffer() && (0 == image_state->GetBoundMemoryStates().size())) {
            const char *vuid = loc.function == Func::vkGetImageMemoryRequirements
                                   ? "VUID-vkGetImageMemoryRequirements-image-04004"
                                   : "VUID-VkImageMemoryRequirementsInfo2-image-01897";
            skip |= LogError(vuid, image, loc,
                             "was created with a VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID handleType, "
                             "which has not yet been "
                             "bound to memory, so image memory requirements can't yet be queried.");
        }
    }
    return skip;
}

bool CoreChecks::ValidateGetPhysicalDeviceImageFormatProperties2ANDROID(const VkPhysicalDeviceImageFormatInfo2 *pImageFormatInfo,
                                                                        const VkImageFormatProperties2 *pImageFormatProperties,
                                                                        const ErrorObject &error_obj) const {
    bool skip = false;
    const auto *ahb_usage = vku::FindStructInPNextChain<VkAndroidHardwareBufferUsageANDROID>(pImageFormatProperties->pNext);
    if (ahb_usage) {
        const auto *pdeifi = vku::FindStructInPNextChain<VkPhysicalDeviceExternalImageFormatInfo>(pImageFormatInfo->pNext);
        if ((!pdeifi) || (VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID != pdeifi->handleType)) {
            skip |= LogError("VUID-vkGetPhysicalDeviceImageFormatProperties2-pNext-01868", physical_device,
                             error_obj.location.dot(Field::pImageFormatProperties),
                             "includes a chained "
                             "VkAndroidHardwareBufferUsageANDROID struct, but pImageFormatInfo does not include a chained "
                             "VkPhysicalDeviceExternalImageFormatInfo struct with handleType "
                             "VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID.");
        }
    }
    return skip;
}

bool CoreChecks::ValidateBufferImportedHandleANDROID(VkExternalMemoryHandleTypeFlags handle_types, VkDeviceMemory memory,
                                                     VkBuffer buffer, const Location &loc) const {
    bool skip = false;
    if ((handle_types & VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID) == 0) {
        const char *vuid = loc.function == Func::vkBindBufferMemory ? "VUID-vkBindBufferMemory-memory-02986"
                                                                    : "VUID-VkBindBufferMemoryInfo-memory-02986";
        const LogObjectList objlist(buffer, memory);
        skip |= LogError(vuid, objlist, loc.dot(Field::memory),
                         "(%s) was created with an AHB import operation which is not set "
                         "VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID in the VkBuffer (%s) "
                         "VkExternalMemoryBufferCreateInfo::handleTypes (%s)",
                         FormatHandle(memory).c_str(), FormatHandle(buffer).c_str(),
                         string_VkExternalMemoryHandleTypeFlags(handle_types).c_str());
    }
    return skip;
}

bool CoreChecks::ValidateImageImportedHandleANDROID(VkExternalMemoryHandleTypeFlags handle_types, VkDeviceMemory memory,
                                                    VkImage image, const Location &loc) const {
    bool skip = false;
    if ((handle_types & VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID) == 0) {
        const char *vuid = loc.function == Func::vkBindImageMemory ? "VUID-vkBindImageMemory-memory-02990"
                                                                   : "VUID-VkBindImageMemoryInfo-memory-02990";
        const LogObjectList objlist(image, memory);
        skip |= LogError(vuid, objlist, loc.dot(Field::memory),
                         "(%s) was created with an AHB import operation which is not set "
                         "VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID in the VkImage (%s) "
                         "VkExternalMemoryImageCreateInfo::handleTypes (%s)",
                         FormatHandle(memory).c_str(), FormatHandle(image).c_str(),
                         string_VkExternalMemoryHandleTypeFlags(handle_types).c_str());
    }
    return skip;
}

// Validate creating an image with an external format
bool CoreChecks::ValidateCreateImageANDROID(const VkImageCreateInfo *create_info, const Location &create_info_loc) const {
    bool skip = false;

    const VkExternalFormatANDROID *ext_fmt_android = vku::FindStructInPNextChain<VkExternalFormatANDROID>(create_info->pNext);
    if (ext_fmt_android && (0 != ext_fmt_android->externalFormat)) {
        if (VK_FORMAT_UNDEFINED != create_info->format) {
            skip |= LogError("VUID-VkImageCreateInfo-pNext-01974", device,
                             create_info_loc.pNext(Struct::VkExternalFormatANDROID, Field::externalFormat),
                             "(%" PRIu64 ") is non-zero, format is %s.", ext_fmt_android->externalFormat,
                             string_VkFormat(create_info->format));
        }

        if (0 != (VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT & create_info->flags)) {
            skip |= LogError("VUID-VkImageCreateInfo-pNext-02396", device,
                             create_info_loc.pNext(Struct::VkExternalFormatANDROID, Field::externalFormat),
                             "(%" PRIu64 ") is non-zero, but flags is %s.", ext_fmt_android->externalFormat,
                             string_VkImageCreateFlags(create_info->flags).c_str());
        }

        // only SAMPLED is allowed, but format_resolve allowed INPUT as well
        if (0 != (~VK_IMAGE_USAGE_SAMPLED_BIT & create_info->usage)) {
            if (((VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT & create_info->usage) == VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT) ||
                !enabled_features.externalFormatResolve) {
                skip |= LogError("VUID-VkImageCreateInfo-pNext-02397", device,
                                 create_info_loc.pNext(Struct::VkExternalFormatANDROID, Field::externalFormat),
                                 "(%" PRIu64 ") is non-zero, but usage is %s.", ext_fmt_android->externalFormat,
                                 string_VkImageUsageFlags(create_info->usage).c_str());
            }
        }

        if (VK_IMAGE_TILING_OPTIMAL != create_info->tiling) {
            skip |= LogError("VUID-VkImageCreateInfo-pNext-02398", device,
                             create_info_loc.pNext(Struct::VkExternalFormatANDROID, Field::externalFormat),
                             "(%" PRIu64 ") is non-zero, but layout is %s.", ext_fmt_android->externalFormat,
                             string_VkImageTiling(create_info->tiling));
        }
        if (ahb_ext_formats_map.find(ext_fmt_android->externalFormat) == ahb_ext_formats_map.end()) {
            skip |= LogError("VUID-VkExternalFormatANDROID-externalFormat-01894", device,
                             create_info_loc.pNext(Struct::VkExternalFormatANDROID, Field::externalFormat),
                             "(%" PRIu64 ") has not been previously retrieved by vkGetAndroidHardwareBufferPropertiesANDROID().",
                             ext_fmt_android->externalFormat);
        }
    }

    if ((nullptr == ext_fmt_android) || (0 == ext_fmt_android->externalFormat)) {
        if (VK_FORMAT_UNDEFINED == create_info->format) {
            if (ext_fmt_android) {
                skip |= LogError("VUID-VkImageCreateInfo-pNext-01975", device, create_info_loc.dot(Field::format),
                                 "is VK_FORMAT_UNDEFINED, but the chained VkExternalFormatANDROID has an externalFormat of 0.");
            } else {
                skip |= LogError("VUID-VkImageCreateInfo-pNext-01975", device, create_info_loc.dot(Field::format),
                                 "is VK_FORMAT_UNDEFINED, but does not have a chained VkExternalFormatANDROID struct.");
            }
        }
    }

    const VkExternalMemoryImageCreateInfo *emici = vku::FindStructInPNextChain<VkExternalMemoryImageCreateInfo>(create_info->pNext);
    if (emici && (emici->handleTypes & VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID)) {
        if (create_info->imageType != VK_IMAGE_TYPE_2D) {
            skip |= LogError("VUID-VkImageCreateInfo-pNext-02393", device,
                             create_info_loc.pNext(Struct::VkExternalMemoryImageCreateInfo, Field::handleTypes),
                             "includes VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID, but imageType is %s.",
                             string_VkImageType(create_info->imageType));
        }

        if ((create_info->mipLevels != 1) && (create_info->mipLevels != FullMipChainLevels(create_info->extent))) {
            skip |= LogError("VUID-VkImageCreateInfo-pNext-02394", device,
                             create_info_loc.pNext(Struct::VkExternalMemoryImageCreateInfo, Field::handleTypes),
                             "includes VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID, "
                             "but mipLevels is %" PRIu32 " (full chain mipLevels are %" PRIu32 ").",
                             create_info->mipLevels, FullMipChainLevels(create_info->extent));
        }
    }

    return skip;
}

// Validate creating an image view with an AHB format
bool CoreChecks::ValidateCreateImageViewANDROID(const VkImageViewCreateInfo *create_info, const Location &create_info_loc) const {
    bool skip = false;
    auto image_state = Get<IMAGE_STATE>(create_info->image);

    if (image_state->HasAHBFormat()) {
        if (VK_FORMAT_UNDEFINED != create_info->format) {
            skip |= LogError("VUID-VkImageViewCreateInfo-image-02399", create_info->image, create_info_loc.dot(Field::format),
                             "is %s (not VK_FORMAT_UNDEFINED) but the VkImageViewCreateInfo struct has a chained "
                             "VkExternalFormatANDROID struct.",
                             string_VkFormat(create_info->format));
        }

        // Chain must include a compatible ycbcr conversion
        bool conv_found = false;
        uint64_t external_format = 0;
        const VkSamplerYcbcrConversionInfo *ycbcr_conv_info = vku::FindStructInPNextChain<VkSamplerYcbcrConversionInfo>(create_info->pNext);
        if (ycbcr_conv_info != nullptr) {
            auto ycbcr_state = Get<SAMPLER_YCBCR_CONVERSION_STATE>(ycbcr_conv_info->conversion);
            if (ycbcr_state) {
                conv_found = true;
                external_format = ycbcr_state->external_format;
            }
        }
        if (!conv_found) {
            const LogObjectList objlist(create_info->image);
            skip |= LogError("VUID-VkImageViewCreateInfo-image-02400", objlist,
                             create_info_loc.pNext(Struct::VkSamplerYcbcrConversionInfo, Field::conversion),
                             "is not valid (or forgot to add VkSamplerYcbcrConversionInfo).");
        } else if ((external_format != image_state->ahb_format)) {
            const LogObjectList objlist(create_info->image, ycbcr_conv_info->conversion);
            skip |= LogError("VUID-VkImageViewCreateInfo-image-02400", objlist,
                             create_info_loc.pNext(Struct::VkSamplerYcbcrConversionInfo, Field::conversion),
                             "(%s) was created with externalFormat (%" PRIu64
                             ") which is different then image chain VkExternalFormatANDROID::externalFormat (%" PRIu64 ").",
                             FormatHandle(ycbcr_conv_info->conversion).c_str(), external_format, image_state->ahb_format);
        }

        // Errors in create_info swizzles
        if (IsIdentitySwizzle(create_info->components) == false) {
            skip |= LogError(
                "VUID-VkImageViewCreateInfo-image-02401", create_info->image, create_info_loc.dot(Field::image),
                "was chained with a VkExternalFormatANDROID struct, but "
                "includes one or more non-identity component swizzles, r swizzle = %s, g swizzle = %s, b swizzle = %s, a swizzle "
                "= %s.",
                string_VkComponentSwizzle(create_info->components.r), string_VkComponentSwizzle(create_info->components.g),
                string_VkComponentSwizzle(create_info->components.b), string_VkComponentSwizzle(create_info->components.a));
        }
    }

    return skip;
}

#else  // !defined(VK_USE_PLATFORM_ANDROID_KHR)

bool CoreChecks::ValidateAllocateMemoryANDROID(const VkMemoryAllocateInfo *allocate_info, const Location &allocate_info_loc) const {
    return false;
}

bool CoreChecks::ValidateGetPhysicalDeviceImageFormatProperties2ANDROID(const VkPhysicalDeviceImageFormatInfo2 *pImageFormatInfo,
                                                                        const VkImageFormatProperties2 *pImageFormatProperties,
                                                                        const ErrorObject &error_obj) const {
    return false;
}

bool CoreChecks::ValidateGetImageMemoryRequirementsANDROID(const VkImage image, const Location &loc) const { return false; }

bool CoreChecks::ValidateBufferImportedHandleANDROID(VkExternalMemoryHandleTypeFlags handle_types, VkDeviceMemory memory,
                                                     VkBuffer buffer, const Location &loc) const {
    return false;
}

bool CoreChecks::ValidateImageImportedHandleANDROID(VkExternalMemoryHandleTypeFlags handle_types, VkDeviceMemory memory,
                                                    VkImage image, const Location &loc) const {
    return false;
}

bool CoreChecks::ValidateCreateImageANDROID(const VkImageCreateInfo *create_info, const Location &create_info_loc) const {
    return false;
}

bool CoreChecks::ValidateCreateImageViewANDROID(const VkImageViewCreateInfo *create_info, const Location &create_info_loc) const {
    return false;
}

#endif
