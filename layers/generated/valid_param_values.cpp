/* *** THIS FILE IS GENERATED - DO NOT EDIT! ***
 * See parameter_validation_generator.py for modifications
 *
 * Copyright (c) 2023 The Khronos Group Inc.
 * Copyright (c) 2023 LunarG, Inc.
 * Copyright (C) 2023 Google Inc.
 * Copyright (c) 2023 Valve Corporation
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


#include "chassis.h"
#include "hash_vk_types.h"


template<>
std::vector<VkPipelineCacheHeaderVersion> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkPipelineCacheHeaderVersionEnums = { VK_PIPELINE_CACHE_HEADER_VERSION_ONE,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkPipelineCacheHeaderVersion>> ExtendedVkPipelineCacheHeaderVersionEnums = {
    };
    std::vector<VkPipelineCacheHeaderVersion> values(CoreVkPipelineCacheHeaderVersionEnums.cbegin(), CoreVkPipelineCacheHeaderVersionEnums.cend());
    std::set<VkPipelineCacheHeaderVersion> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkPipelineCacheHeaderVersionEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkImageLayout> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkImageLayoutEnums = { VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PREINITIALIZED,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkImageLayout>> ExtendedVkImageLayoutEnums = {
        { &DeviceExtensions::vk_khr_maintenance2, { VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL,  } },
        { &DeviceExtensions::vk_khr_separate_depth_stencil_layouts, { VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL,  } },
        { &DeviceExtensions::vk_khr_synchronization2, { VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,  } },
        { &DeviceExtensions::vk_khr_swapchain, { VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,  } },
        { &DeviceExtensions::vk_khr_video_decode_queue, { VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR, VK_IMAGE_LAYOUT_VIDEO_DECODE_SRC_KHR, VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR,  } },
        { &DeviceExtensions::vk_khr_shared_presentable_image, { VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR,  } },
        { &DeviceExtensions::vk_ext_fragment_density_map, { VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT,  } },
        { &DeviceExtensions::vk_khr_fragment_shading_rate, { VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR,  } },
        { &DeviceExtensions::vk_nv_shading_rate_image, { VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR,  } },
        { &DeviceExtensions::vk_khr_video_encode_queue, { VK_IMAGE_LAYOUT_VIDEO_ENCODE_DST_KHR, VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR, VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR,  } },
        { &DeviceExtensions::vk_ext_attachment_feedback_loop_layout, { VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT,  } },
    };
    std::vector<VkImageLayout> values(CoreVkImageLayoutEnums.cbegin(), CoreVkImageLayoutEnums.cend());
    std::set<VkImageLayout> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkImageLayoutEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkObjectType> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkObjectTypeEnums = { VK_OBJECT_TYPE_UNKNOWN, VK_OBJECT_TYPE_INSTANCE, VK_OBJECT_TYPE_PHYSICAL_DEVICE, VK_OBJECT_TYPE_DEVICE, VK_OBJECT_TYPE_QUEUE, VK_OBJECT_TYPE_SEMAPHORE, VK_OBJECT_TYPE_COMMAND_BUFFER, VK_OBJECT_TYPE_FENCE, VK_OBJECT_TYPE_DEVICE_MEMORY, VK_OBJECT_TYPE_BUFFER, VK_OBJECT_TYPE_IMAGE, VK_OBJECT_TYPE_EVENT, VK_OBJECT_TYPE_QUERY_POOL, VK_OBJECT_TYPE_BUFFER_VIEW, VK_OBJECT_TYPE_IMAGE_VIEW, VK_OBJECT_TYPE_SHADER_MODULE, VK_OBJECT_TYPE_PIPELINE_CACHE, VK_OBJECT_TYPE_PIPELINE_LAYOUT, VK_OBJECT_TYPE_RENDER_PASS, VK_OBJECT_TYPE_PIPELINE, VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, VK_OBJECT_TYPE_SAMPLER, VK_OBJECT_TYPE_DESCRIPTOR_POOL, VK_OBJECT_TYPE_DESCRIPTOR_SET, VK_OBJECT_TYPE_FRAMEBUFFER, VK_OBJECT_TYPE_COMMAND_POOL,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkObjectType>> ExtendedVkObjectTypeEnums = {
        { &DeviceExtensions::vk_khr_sampler_ycbcr_conversion, { VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION,  } },
        { &DeviceExtensions::vk_khr_descriptor_update_template, { VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE,  } },
        { &DeviceExtensions::vk_ext_private_data, { VK_OBJECT_TYPE_PRIVATE_DATA_SLOT,  } },
        { &DeviceExtensions::vk_khr_surface, { VK_OBJECT_TYPE_SURFACE_KHR,  } },
        { &DeviceExtensions::vk_khr_swapchain, { VK_OBJECT_TYPE_SWAPCHAIN_KHR,  } },
        { &DeviceExtensions::vk_khr_display, { VK_OBJECT_TYPE_DISPLAY_KHR, VK_OBJECT_TYPE_DISPLAY_MODE_KHR,  } },
        { &DeviceExtensions::vk_ext_debug_report, { VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT,  } },
        { &DeviceExtensions::vk_khr_video_queue, { VK_OBJECT_TYPE_VIDEO_SESSION_KHR, VK_OBJECT_TYPE_VIDEO_SESSION_PARAMETERS_KHR,  } },
        { &DeviceExtensions::vk_nvx_binary_import, { VK_OBJECT_TYPE_CU_MODULE_NVX, VK_OBJECT_TYPE_CU_FUNCTION_NVX,  } },
        { &DeviceExtensions::vk_ext_debug_utils, { VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT,  } },
        { &DeviceExtensions::vk_khr_acceleration_structure, { VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR,  } },
        { &DeviceExtensions::vk_ext_validation_cache, { VK_OBJECT_TYPE_VALIDATION_CACHE_EXT,  } },
        { &DeviceExtensions::vk_nv_ray_tracing, { VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV,  } },
        { &DeviceExtensions::vk_intel_performance_query, { VK_OBJECT_TYPE_PERFORMANCE_CONFIGURATION_INTEL,  } },
        { &DeviceExtensions::vk_khr_deferred_host_operations, { VK_OBJECT_TYPE_DEFERRED_OPERATION_KHR,  } },
        { &DeviceExtensions::vk_nv_device_generated_commands, { VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NV,  } },
        { &DeviceExtensions::vk_fuchsia_buffer_collection, { VK_OBJECT_TYPE_BUFFER_COLLECTION_FUCHSIA,  } },
        { &DeviceExtensions::vk_ext_opacity_micromap, { VK_OBJECT_TYPE_MICROMAP_EXT,  } },
        { &DeviceExtensions::vk_nv_optical_flow, { VK_OBJECT_TYPE_OPTICAL_FLOW_SESSION_NV,  } },
    };
    std::vector<VkObjectType> values(CoreVkObjectTypeEnums.cbegin(), CoreVkObjectTypeEnums.cend());
    std::set<VkObjectType> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkObjectTypeEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkFormat> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkFormatEnums = { VK_FORMAT_UNDEFINED, VK_FORMAT_R4G4_UNORM_PACK8, VK_FORMAT_R4G4B4A4_UNORM_PACK16, VK_FORMAT_B4G4R4A4_UNORM_PACK16, VK_FORMAT_R5G6B5_UNORM_PACK16, VK_FORMAT_B5G6R5_UNORM_PACK16, VK_FORMAT_R5G5B5A1_UNORM_PACK16, VK_FORMAT_B5G5R5A1_UNORM_PACK16, VK_FORMAT_A1R5G5B5_UNORM_PACK16, VK_FORMAT_R8_UNORM, VK_FORMAT_R8_SNORM, VK_FORMAT_R8_USCALED, VK_FORMAT_R8_SSCALED, VK_FORMAT_R8_UINT, VK_FORMAT_R8_SINT, VK_FORMAT_R8_SRGB, VK_FORMAT_R8G8_UNORM, VK_FORMAT_R8G8_SNORM, VK_FORMAT_R8G8_USCALED, VK_FORMAT_R8G8_SSCALED, VK_FORMAT_R8G8_UINT, VK_FORMAT_R8G8_SINT, VK_FORMAT_R8G8_SRGB, VK_FORMAT_R8G8B8_UNORM, VK_FORMAT_R8G8B8_SNORM, VK_FORMAT_R8G8B8_USCALED, VK_FORMAT_R8G8B8_SSCALED, VK_FORMAT_R8G8B8_UINT, VK_FORMAT_R8G8B8_SINT, VK_FORMAT_R8G8B8_SRGB, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_B8G8R8_SNORM, VK_FORMAT_B8G8R8_USCALED, VK_FORMAT_B8G8R8_SSCALED, VK_FORMAT_B8G8R8_UINT, VK_FORMAT_B8G8R8_SINT, VK_FORMAT_B8G8R8_SRGB, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R8G8B8A8_SNORM, VK_FORMAT_R8G8B8A8_USCALED, VK_FORMAT_R8G8B8A8_SSCALED, VK_FORMAT_R8G8B8A8_UINT, VK_FORMAT_R8G8B8A8_SINT, VK_FORMAT_R8G8B8A8_SRGB, VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_B8G8R8A8_SNORM, VK_FORMAT_B8G8R8A8_USCALED, VK_FORMAT_B8G8R8A8_SSCALED, VK_FORMAT_B8G8R8A8_UINT, VK_FORMAT_B8G8R8A8_SINT, VK_FORMAT_B8G8R8A8_SRGB, VK_FORMAT_A8B8G8R8_UNORM_PACK32, VK_FORMAT_A8B8G8R8_SNORM_PACK32, VK_FORMAT_A8B8G8R8_USCALED_PACK32, VK_FORMAT_A8B8G8R8_SSCALED_PACK32, VK_FORMAT_A8B8G8R8_UINT_PACK32, VK_FORMAT_A8B8G8R8_SINT_PACK32, VK_FORMAT_A8B8G8R8_SRGB_PACK32, VK_FORMAT_A2R10G10B10_UNORM_PACK32, VK_FORMAT_A2R10G10B10_SNORM_PACK32, VK_FORMAT_A2R10G10B10_USCALED_PACK32, VK_FORMAT_A2R10G10B10_SSCALED_PACK32, VK_FORMAT_A2R10G10B10_UINT_PACK32, VK_FORMAT_A2R10G10B10_SINT_PACK32, VK_FORMAT_A2B10G10R10_UNORM_PACK32, VK_FORMAT_A2B10G10R10_SNORM_PACK32, VK_FORMAT_A2B10G10R10_USCALED_PACK32, VK_FORMAT_A2B10G10R10_SSCALED_PACK32, VK_FORMAT_A2B10G10R10_UINT_PACK32, VK_FORMAT_A2B10G10R10_SINT_PACK32, VK_FORMAT_R16_UNORM, VK_FORMAT_R16_SNORM, VK_FORMAT_R16_USCALED, VK_FORMAT_R16_SSCALED, VK_FORMAT_R16_UINT, VK_FORMAT_R16_SINT, VK_FORMAT_R16_SFLOAT, VK_FORMAT_R16G16_UNORM, VK_FORMAT_R16G16_SNORM, VK_FORMAT_R16G16_USCALED, VK_FORMAT_R16G16_SSCALED, VK_FORMAT_R16G16_UINT, VK_FORMAT_R16G16_SINT, VK_FORMAT_R16G16_SFLOAT, VK_FORMAT_R16G16B16_UNORM, VK_FORMAT_R16G16B16_SNORM, VK_FORMAT_R16G16B16_USCALED, VK_FORMAT_R16G16B16_SSCALED, VK_FORMAT_R16G16B16_UINT, VK_FORMAT_R16G16B16_SINT, VK_FORMAT_R16G16B16_SFLOAT, VK_FORMAT_R16G16B16A16_UNORM, VK_FORMAT_R16G16B16A16_SNORM, VK_FORMAT_R16G16B16A16_USCALED, VK_FORMAT_R16G16B16A16_SSCALED, VK_FORMAT_R16G16B16A16_UINT, VK_FORMAT_R16G16B16A16_SINT, VK_FORMAT_R16G16B16A16_SFLOAT, VK_FORMAT_R32_UINT, VK_FORMAT_R32_SINT, VK_FORMAT_R32_SFLOAT, VK_FORMAT_R32G32_UINT, VK_FORMAT_R32G32_SINT, VK_FORMAT_R32G32_SFLOAT, VK_FORMAT_R32G32B32_UINT, VK_FORMAT_R32G32B32_SINT, VK_FORMAT_R32G32B32_SFLOAT, VK_FORMAT_R32G32B32A32_UINT, VK_FORMAT_R32G32B32A32_SINT, VK_FORMAT_R32G32B32A32_SFLOAT, VK_FORMAT_R64_UINT, VK_FORMAT_R64_SINT, VK_FORMAT_R64_SFLOAT, VK_FORMAT_R64G64_UINT, VK_FORMAT_R64G64_SINT, VK_FORMAT_R64G64_SFLOAT, VK_FORMAT_R64G64B64_UINT, VK_FORMAT_R64G64B64_SINT, VK_FORMAT_R64G64B64_SFLOAT, VK_FORMAT_R64G64B64A64_UINT, VK_FORMAT_R64G64B64A64_SINT, VK_FORMAT_R64G64B64A64_SFLOAT, VK_FORMAT_B10G11R11_UFLOAT_PACK32, VK_FORMAT_E5B9G9R9_UFLOAT_PACK32, VK_FORMAT_D16_UNORM, VK_FORMAT_X8_D24_UNORM_PACK32, VK_FORMAT_D32_SFLOAT, VK_FORMAT_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_BC1_RGB_UNORM_BLOCK, VK_FORMAT_BC1_RGB_SRGB_BLOCK, VK_FORMAT_BC1_RGBA_UNORM_BLOCK, VK_FORMAT_BC1_RGBA_SRGB_BLOCK, VK_FORMAT_BC2_UNORM_BLOCK, VK_FORMAT_BC2_SRGB_BLOCK, VK_FORMAT_BC3_UNORM_BLOCK, VK_FORMAT_BC3_SRGB_BLOCK, VK_FORMAT_BC4_UNORM_BLOCK, VK_FORMAT_BC4_SNORM_BLOCK, VK_FORMAT_BC5_UNORM_BLOCK, VK_FORMAT_BC5_SNORM_BLOCK, VK_FORMAT_BC6H_UFLOAT_BLOCK, VK_FORMAT_BC6H_SFLOAT_BLOCK, VK_FORMAT_BC7_UNORM_BLOCK, VK_FORMAT_BC7_SRGB_BLOCK, VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK, VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK, VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK, VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK, VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK, VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK, VK_FORMAT_EAC_R11_UNORM_BLOCK, VK_FORMAT_EAC_R11_SNORM_BLOCK, VK_FORMAT_EAC_R11G11_UNORM_BLOCK, VK_FORMAT_EAC_R11G11_SNORM_BLOCK, VK_FORMAT_ASTC_4x4_UNORM_BLOCK, VK_FORMAT_ASTC_4x4_SRGB_BLOCK, VK_FORMAT_ASTC_5x4_UNORM_BLOCK, VK_FORMAT_ASTC_5x4_SRGB_BLOCK, VK_FORMAT_ASTC_5x5_UNORM_BLOCK, VK_FORMAT_ASTC_5x5_SRGB_BLOCK, VK_FORMAT_ASTC_6x5_UNORM_BLOCK, VK_FORMAT_ASTC_6x5_SRGB_BLOCK, VK_FORMAT_ASTC_6x6_UNORM_BLOCK, VK_FORMAT_ASTC_6x6_SRGB_BLOCK, VK_FORMAT_ASTC_8x5_UNORM_BLOCK, VK_FORMAT_ASTC_8x5_SRGB_BLOCK, VK_FORMAT_ASTC_8x6_UNORM_BLOCK, VK_FORMAT_ASTC_8x6_SRGB_BLOCK, VK_FORMAT_ASTC_8x8_UNORM_BLOCK, VK_FORMAT_ASTC_8x8_SRGB_BLOCK, VK_FORMAT_ASTC_10x5_UNORM_BLOCK, VK_FORMAT_ASTC_10x5_SRGB_BLOCK, VK_FORMAT_ASTC_10x6_UNORM_BLOCK, VK_FORMAT_ASTC_10x6_SRGB_BLOCK, VK_FORMAT_ASTC_10x8_UNORM_BLOCK, VK_FORMAT_ASTC_10x8_SRGB_BLOCK, VK_FORMAT_ASTC_10x10_UNORM_BLOCK, VK_FORMAT_ASTC_10x10_SRGB_BLOCK, VK_FORMAT_ASTC_12x10_UNORM_BLOCK, VK_FORMAT_ASTC_12x10_SRGB_BLOCK, VK_FORMAT_ASTC_12x12_UNORM_BLOCK, VK_FORMAT_ASTC_12x12_SRGB_BLOCK,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkFormat>> ExtendedVkFormatEnums = {
        { &DeviceExtensions::vk_khr_sampler_ycbcr_conversion, { VK_FORMAT_G8B8G8R8_422_UNORM, VK_FORMAT_B8G8R8G8_422_UNORM, VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM, VK_FORMAT_G8_B8R8_2PLANE_420_UNORM, VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM, VK_FORMAT_G8_B8R8_2PLANE_422_UNORM, VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM, VK_FORMAT_R10X6_UNORM_PACK16, VK_FORMAT_R10X6G10X6_UNORM_2PACK16, VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16, VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16, VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16, VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16, VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16, VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16, VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16, VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16, VK_FORMAT_R12X4_UNORM_PACK16, VK_FORMAT_R12X4G12X4_UNORM_2PACK16, VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16, VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16, VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16, VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16, VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16, VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16, VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16, VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16, VK_FORMAT_G16B16G16R16_422_UNORM, VK_FORMAT_B16G16R16G16_422_UNORM, VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM, VK_FORMAT_G16_B16R16_2PLANE_420_UNORM, VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM, VK_FORMAT_G16_B16R16_2PLANE_422_UNORM, VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM,  } },
        { &DeviceExtensions::vk_ext_ycbcr_2plane_444_formats, { VK_FORMAT_G8_B8R8_2PLANE_444_UNORM, VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16, VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16, VK_FORMAT_G16_B16R16_2PLANE_444_UNORM,  } },
        { &DeviceExtensions::vk_ext_4444_formats, { VK_FORMAT_A4R4G4B4_UNORM_PACK16, VK_FORMAT_A4B4G4R4_UNORM_PACK16,  } },
        { &DeviceExtensions::vk_ext_texture_compression_astc_hdr, { VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK, VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK, VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK, VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK, VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK, VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK, VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK, VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK, VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK, VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK, VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK, VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK, VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK, VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK,  } },
        { &DeviceExtensions::vk_img_format_pvrtc, { VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG, VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG, VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG, VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG, VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG, VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG, VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG, VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG,  } },
        { &DeviceExtensions::vk_nv_optical_flow, { VK_FORMAT_R16G16_S10_5_NV,  } },
    };
    std::vector<VkFormat> values(CoreVkFormatEnums.cbegin(), CoreVkFormatEnums.cend());
    std::set<VkFormat> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkFormatEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkImageTiling> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkImageTilingEnums = { VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_TILING_LINEAR,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkImageTiling>> ExtendedVkImageTilingEnums = {
        { &DeviceExtensions::vk_ext_image_drm_format_modifier, { VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT,  } },
    };
    std::vector<VkImageTiling> values(CoreVkImageTilingEnums.cbegin(), CoreVkImageTilingEnums.cend());
    std::set<VkImageTiling> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkImageTilingEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkImageType> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkImageTypeEnums = { VK_IMAGE_TYPE_1D, VK_IMAGE_TYPE_2D, VK_IMAGE_TYPE_3D,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkImageType>> ExtendedVkImageTypeEnums = {
    };
    std::vector<VkImageType> values(CoreVkImageTypeEnums.cbegin(), CoreVkImageTypeEnums.cend());
    std::set<VkImageType> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkImageTypeEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkQueryType> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkQueryTypeEnums = { VK_QUERY_TYPE_OCCLUSION, VK_QUERY_TYPE_PIPELINE_STATISTICS, VK_QUERY_TYPE_TIMESTAMP,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkQueryType>> ExtendedVkQueryTypeEnums = {
        { &DeviceExtensions::vk_khr_video_queue, { VK_QUERY_TYPE_RESULT_STATUS_ONLY_KHR,  } },
        { &DeviceExtensions::vk_ext_transform_feedback, { VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT,  } },
        { &DeviceExtensions::vk_khr_performance_query, { VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR,  } },
        { &DeviceExtensions::vk_khr_acceleration_structure, { VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR, VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR,  } },
        { &DeviceExtensions::vk_nv_ray_tracing, { VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_NV,  } },
        { &DeviceExtensions::vk_intel_performance_query, { VK_QUERY_TYPE_PERFORMANCE_QUERY_INTEL,  } },
        { &DeviceExtensions::vk_khr_video_encode_queue, { VK_QUERY_TYPE_VIDEO_ENCODE_BITSTREAM_BUFFER_RANGE_KHR,  } },
        { &DeviceExtensions::vk_ext_mesh_shader, { VK_QUERY_TYPE_MESH_PRIMITIVES_GENERATED_EXT,  } },
        { &DeviceExtensions::vk_ext_primitives_generated_query, { VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT,  } },
        { &DeviceExtensions::vk_khr_ray_tracing_maintenance1, { VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_BOTTOM_LEVEL_POINTERS_KHR, VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SIZE_KHR,  } },
        { &DeviceExtensions::vk_ext_opacity_micromap, { VK_QUERY_TYPE_MICROMAP_SERIALIZATION_SIZE_EXT, VK_QUERY_TYPE_MICROMAP_COMPACTED_SIZE_EXT,  } },
    };
    std::vector<VkQueryType> values(CoreVkQueryTypeEnums.cbegin(), CoreVkQueryTypeEnums.cend());
    std::set<VkQueryType> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkQueryTypeEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkSharingMode> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkSharingModeEnums = { VK_SHARING_MODE_EXCLUSIVE, VK_SHARING_MODE_CONCURRENT,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkSharingMode>> ExtendedVkSharingModeEnums = {
    };
    std::vector<VkSharingMode> values(CoreVkSharingModeEnums.cbegin(), CoreVkSharingModeEnums.cend());
    std::set<VkSharingMode> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkSharingModeEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkComponentSwizzle> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkComponentSwizzleEnums = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ONE, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkComponentSwizzle>> ExtendedVkComponentSwizzleEnums = {
    };
    std::vector<VkComponentSwizzle> values(CoreVkComponentSwizzleEnums.cbegin(), CoreVkComponentSwizzleEnums.cend());
    std::set<VkComponentSwizzle> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkComponentSwizzleEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkImageViewType> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkImageViewTypeEnums = { VK_IMAGE_VIEW_TYPE_1D, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_VIEW_TYPE_3D, VK_IMAGE_VIEW_TYPE_CUBE, VK_IMAGE_VIEW_TYPE_1D_ARRAY, VK_IMAGE_VIEW_TYPE_2D_ARRAY, VK_IMAGE_VIEW_TYPE_CUBE_ARRAY,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkImageViewType>> ExtendedVkImageViewTypeEnums = {
    };
    std::vector<VkImageViewType> values(CoreVkImageViewTypeEnums.cbegin(), CoreVkImageViewTypeEnums.cend());
    std::set<VkImageViewType> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkImageViewTypeEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkBlendFactor> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkBlendFactorEnums = { VK_BLEND_FACTOR_ZERO, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_SRC_COLOR, VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR, VK_BLEND_FACTOR_DST_COLOR, VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR, VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_FACTOR_DST_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA, VK_BLEND_FACTOR_CONSTANT_COLOR, VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR, VK_BLEND_FACTOR_CONSTANT_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA, VK_BLEND_FACTOR_SRC_ALPHA_SATURATE, VK_BLEND_FACTOR_SRC1_COLOR, VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR, VK_BLEND_FACTOR_SRC1_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkBlendFactor>> ExtendedVkBlendFactorEnums = {
    };
    std::vector<VkBlendFactor> values(CoreVkBlendFactorEnums.cbegin(), CoreVkBlendFactorEnums.cend());
    std::set<VkBlendFactor> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkBlendFactorEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkBlendOp> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkBlendOpEnums = { VK_BLEND_OP_ADD, VK_BLEND_OP_SUBTRACT, VK_BLEND_OP_REVERSE_SUBTRACT, VK_BLEND_OP_MIN, VK_BLEND_OP_MAX,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkBlendOp>> ExtendedVkBlendOpEnums = {
        { &DeviceExtensions::vk_ext_blend_operation_advanced, { VK_BLEND_OP_ZERO_EXT, VK_BLEND_OP_SRC_EXT, VK_BLEND_OP_DST_EXT, VK_BLEND_OP_SRC_OVER_EXT, VK_BLEND_OP_DST_OVER_EXT, VK_BLEND_OP_SRC_IN_EXT, VK_BLEND_OP_DST_IN_EXT, VK_BLEND_OP_SRC_OUT_EXT, VK_BLEND_OP_DST_OUT_EXT, VK_BLEND_OP_SRC_ATOP_EXT, VK_BLEND_OP_DST_ATOP_EXT, VK_BLEND_OP_XOR_EXT, VK_BLEND_OP_MULTIPLY_EXT, VK_BLEND_OP_SCREEN_EXT, VK_BLEND_OP_OVERLAY_EXT, VK_BLEND_OP_DARKEN_EXT, VK_BLEND_OP_LIGHTEN_EXT, VK_BLEND_OP_COLORDODGE_EXT, VK_BLEND_OP_COLORBURN_EXT, VK_BLEND_OP_HARDLIGHT_EXT, VK_BLEND_OP_SOFTLIGHT_EXT, VK_BLEND_OP_DIFFERENCE_EXT, VK_BLEND_OP_EXCLUSION_EXT, VK_BLEND_OP_INVERT_EXT, VK_BLEND_OP_INVERT_RGB_EXT, VK_BLEND_OP_LINEARDODGE_EXT, VK_BLEND_OP_LINEARBURN_EXT, VK_BLEND_OP_VIVIDLIGHT_EXT, VK_BLEND_OP_LINEARLIGHT_EXT, VK_BLEND_OP_PINLIGHT_EXT, VK_BLEND_OP_HARDMIX_EXT, VK_BLEND_OP_HSL_HUE_EXT, VK_BLEND_OP_HSL_SATURATION_EXT, VK_BLEND_OP_HSL_COLOR_EXT, VK_BLEND_OP_HSL_LUMINOSITY_EXT, VK_BLEND_OP_PLUS_EXT, VK_BLEND_OP_PLUS_CLAMPED_EXT, VK_BLEND_OP_PLUS_CLAMPED_ALPHA_EXT, VK_BLEND_OP_PLUS_DARKER_EXT, VK_BLEND_OP_MINUS_EXT, VK_BLEND_OP_MINUS_CLAMPED_EXT, VK_BLEND_OP_CONTRAST_EXT, VK_BLEND_OP_INVERT_OVG_EXT, VK_BLEND_OP_RED_EXT, VK_BLEND_OP_GREEN_EXT, VK_BLEND_OP_BLUE_EXT,  } },
    };
    std::vector<VkBlendOp> values(CoreVkBlendOpEnums.cbegin(), CoreVkBlendOpEnums.cend());
    std::set<VkBlendOp> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkBlendOpEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkCompareOp> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkCompareOpEnums = { VK_COMPARE_OP_NEVER, VK_COMPARE_OP_LESS, VK_COMPARE_OP_EQUAL, VK_COMPARE_OP_LESS_OR_EQUAL, VK_COMPARE_OP_GREATER, VK_COMPARE_OP_NOT_EQUAL, VK_COMPARE_OP_GREATER_OR_EQUAL, VK_COMPARE_OP_ALWAYS,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkCompareOp>> ExtendedVkCompareOpEnums = {
    };
    std::vector<VkCompareOp> values(CoreVkCompareOpEnums.cbegin(), CoreVkCompareOpEnums.cend());
    std::set<VkCompareOp> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkCompareOpEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkDynamicState> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkDynamicStateEnums = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_LINE_WIDTH, VK_DYNAMIC_STATE_DEPTH_BIAS, VK_DYNAMIC_STATE_BLEND_CONSTANTS, VK_DYNAMIC_STATE_DEPTH_BOUNDS, VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK, VK_DYNAMIC_STATE_STENCIL_WRITE_MASK, VK_DYNAMIC_STATE_STENCIL_REFERENCE,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkDynamicState>> ExtendedVkDynamicStateEnums = {
        { &DeviceExtensions::vk_ext_extended_dynamic_state, { VK_DYNAMIC_STATE_CULL_MODE, VK_DYNAMIC_STATE_FRONT_FACE, VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY, VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT, VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT, VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE, VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE, VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE, VK_DYNAMIC_STATE_DEPTH_COMPARE_OP, VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE, VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE, VK_DYNAMIC_STATE_STENCIL_OP,  } },
        { &DeviceExtensions::vk_ext_extended_dynamic_state2, { VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE, VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE, VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE, VK_DYNAMIC_STATE_PATCH_CONTROL_POINTS_EXT, VK_DYNAMIC_STATE_LOGIC_OP_EXT,  } },
        { &DeviceExtensions::vk_nv_clip_space_w_scaling, { VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_NV,  } },
        { &DeviceExtensions::vk_ext_discard_rectangles, { VK_DYNAMIC_STATE_DISCARD_RECTANGLE_EXT,  } },
        { &DeviceExtensions::vk_ext_sample_locations, { VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT,  } },
        { &DeviceExtensions::vk_khr_ray_tracing_pipeline, { VK_DYNAMIC_STATE_RAY_TRACING_PIPELINE_STACK_SIZE_KHR,  } },
        { &DeviceExtensions::vk_nv_shading_rate_image, { VK_DYNAMIC_STATE_VIEWPORT_SHADING_RATE_PALETTE_NV, VK_DYNAMIC_STATE_VIEWPORT_COARSE_SAMPLE_ORDER_NV,  } },
        { &DeviceExtensions::vk_nv_scissor_exclusive, { VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_NV,  } },
        { &DeviceExtensions::vk_khr_fragment_shading_rate, { VK_DYNAMIC_STATE_FRAGMENT_SHADING_RATE_KHR,  } },
        { &DeviceExtensions::vk_ext_line_rasterization, { VK_DYNAMIC_STATE_LINE_STIPPLE_EXT,  } },
        { &DeviceExtensions::vk_ext_vertex_input_dynamic_state, { VK_DYNAMIC_STATE_VERTEX_INPUT_EXT,  } },
        { &DeviceExtensions::vk_ext_color_write_enable, { VK_DYNAMIC_STATE_COLOR_WRITE_ENABLE_EXT,  } },
        { &DeviceExtensions::vk_ext_extended_dynamic_state3, { VK_DYNAMIC_STATE_TESSELLATION_DOMAIN_ORIGIN_EXT, VK_DYNAMIC_STATE_DEPTH_CLAMP_ENABLE_EXT, VK_DYNAMIC_STATE_POLYGON_MODE_EXT, VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT, VK_DYNAMIC_STATE_SAMPLE_MASK_EXT, VK_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT, VK_DYNAMIC_STATE_ALPHA_TO_ONE_ENABLE_EXT, VK_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT, VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT, VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT, VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT, VK_DYNAMIC_STATE_RASTERIZATION_STREAM_EXT, VK_DYNAMIC_STATE_CONSERVATIVE_RASTERIZATION_MODE_EXT, VK_DYNAMIC_STATE_EXTRA_PRIMITIVE_OVERESTIMATION_SIZE_EXT, VK_DYNAMIC_STATE_DEPTH_CLIP_ENABLE_EXT, VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_ENABLE_EXT, VK_DYNAMIC_STATE_COLOR_BLEND_ADVANCED_EXT, VK_DYNAMIC_STATE_PROVOKING_VERTEX_MODE_EXT, VK_DYNAMIC_STATE_LINE_RASTERIZATION_MODE_EXT, VK_DYNAMIC_STATE_LINE_STIPPLE_ENABLE_EXT, VK_DYNAMIC_STATE_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE_EXT, VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_ENABLE_NV, VK_DYNAMIC_STATE_VIEWPORT_SWIZZLE_NV, VK_DYNAMIC_STATE_COVERAGE_TO_COLOR_ENABLE_NV, VK_DYNAMIC_STATE_COVERAGE_TO_COLOR_LOCATION_NV, VK_DYNAMIC_STATE_COVERAGE_MODULATION_MODE_NV, VK_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_ENABLE_NV, VK_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_NV, VK_DYNAMIC_STATE_SHADING_RATE_IMAGE_ENABLE_NV, VK_DYNAMIC_STATE_REPRESENTATIVE_FRAGMENT_TEST_ENABLE_NV, VK_DYNAMIC_STATE_COVERAGE_REDUCTION_MODE_NV,  } },
    };
    std::vector<VkDynamicState> values(CoreVkDynamicStateEnums.cbegin(), CoreVkDynamicStateEnums.cend());
    std::set<VkDynamicState> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkDynamicStateEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkFrontFace> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkFrontFaceEnums = { VK_FRONT_FACE_COUNTER_CLOCKWISE, VK_FRONT_FACE_CLOCKWISE,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkFrontFace>> ExtendedVkFrontFaceEnums = {
    };
    std::vector<VkFrontFace> values(CoreVkFrontFaceEnums.cbegin(), CoreVkFrontFaceEnums.cend());
    std::set<VkFrontFace> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkFrontFaceEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkVertexInputRate> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkVertexInputRateEnums = { VK_VERTEX_INPUT_RATE_VERTEX, VK_VERTEX_INPUT_RATE_INSTANCE,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkVertexInputRate>> ExtendedVkVertexInputRateEnums = {
    };
    std::vector<VkVertexInputRate> values(CoreVkVertexInputRateEnums.cbegin(), CoreVkVertexInputRateEnums.cend());
    std::set<VkVertexInputRate> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkVertexInputRateEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkPrimitiveTopology> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkPrimitiveTopologyEnums = { VK_PRIMITIVE_TOPOLOGY_POINT_LIST, VK_PRIMITIVE_TOPOLOGY_LINE_LIST, VK_PRIMITIVE_TOPOLOGY_LINE_STRIP, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN, VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY, VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY, VK_PRIMITIVE_TOPOLOGY_PATCH_LIST,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkPrimitiveTopology>> ExtendedVkPrimitiveTopologyEnums = {
    };
    std::vector<VkPrimitiveTopology> values(CoreVkPrimitiveTopologyEnums.cbegin(), CoreVkPrimitiveTopologyEnums.cend());
    std::set<VkPrimitiveTopology> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkPrimitiveTopologyEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkPolygonMode> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkPolygonModeEnums = { VK_POLYGON_MODE_FILL, VK_POLYGON_MODE_LINE, VK_POLYGON_MODE_POINT,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkPolygonMode>> ExtendedVkPolygonModeEnums = {
        { &DeviceExtensions::vk_nv_fill_rectangle, { VK_POLYGON_MODE_FILL_RECTANGLE_NV,  } },
    };
    std::vector<VkPolygonMode> values(CoreVkPolygonModeEnums.cbegin(), CoreVkPolygonModeEnums.cend());
    std::set<VkPolygonMode> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkPolygonModeEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkStencilOp> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkStencilOpEnums = { VK_STENCIL_OP_KEEP, VK_STENCIL_OP_ZERO, VK_STENCIL_OP_REPLACE, VK_STENCIL_OP_INCREMENT_AND_CLAMP, VK_STENCIL_OP_DECREMENT_AND_CLAMP, VK_STENCIL_OP_INVERT, VK_STENCIL_OP_INCREMENT_AND_WRAP, VK_STENCIL_OP_DECREMENT_AND_WRAP,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkStencilOp>> ExtendedVkStencilOpEnums = {
    };
    std::vector<VkStencilOp> values(CoreVkStencilOpEnums.cbegin(), CoreVkStencilOpEnums.cend());
    std::set<VkStencilOp> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkStencilOpEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkLogicOp> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkLogicOpEnums = { VK_LOGIC_OP_CLEAR, VK_LOGIC_OP_AND, VK_LOGIC_OP_AND_REVERSE, VK_LOGIC_OP_COPY, VK_LOGIC_OP_AND_INVERTED, VK_LOGIC_OP_NO_OP, VK_LOGIC_OP_XOR, VK_LOGIC_OP_OR, VK_LOGIC_OP_NOR, VK_LOGIC_OP_EQUIVALENT, VK_LOGIC_OP_INVERT, VK_LOGIC_OP_OR_REVERSE, VK_LOGIC_OP_COPY_INVERTED, VK_LOGIC_OP_OR_INVERTED, VK_LOGIC_OP_NAND, VK_LOGIC_OP_SET,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkLogicOp>> ExtendedVkLogicOpEnums = {
    };
    std::vector<VkLogicOp> values(CoreVkLogicOpEnums.cbegin(), CoreVkLogicOpEnums.cend());
    std::set<VkLogicOp> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkLogicOpEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkBorderColor> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkBorderColorEnums = { VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK, VK_BORDER_COLOR_INT_TRANSPARENT_BLACK, VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK, VK_BORDER_COLOR_INT_OPAQUE_BLACK, VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE, VK_BORDER_COLOR_INT_OPAQUE_WHITE,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkBorderColor>> ExtendedVkBorderColorEnums = {
        { &DeviceExtensions::vk_ext_custom_border_color, { VK_BORDER_COLOR_FLOAT_CUSTOM_EXT, VK_BORDER_COLOR_INT_CUSTOM_EXT,  } },
    };
    std::vector<VkBorderColor> values(CoreVkBorderColorEnums.cbegin(), CoreVkBorderColorEnums.cend());
    std::set<VkBorderColor> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkBorderColorEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkFilter> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkFilterEnums = { VK_FILTER_NEAREST, VK_FILTER_LINEAR,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkFilter>> ExtendedVkFilterEnums = {
        { &DeviceExtensions::vk_ext_filter_cubic, { VK_FILTER_CUBIC_EXT,  } },
        { &DeviceExtensions::vk_img_filter_cubic, { VK_FILTER_CUBIC_EXT,  } },
    };
    std::vector<VkFilter> values(CoreVkFilterEnums.cbegin(), CoreVkFilterEnums.cend());
    std::set<VkFilter> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkFilterEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkSamplerAddressMode> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkSamplerAddressModeEnums = { VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkSamplerAddressMode>> ExtendedVkSamplerAddressModeEnums = {
        { &DeviceExtensions::vk_khr_sampler_mirror_clamp_to_edge, { VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE,  } },
    };
    std::vector<VkSamplerAddressMode> values(CoreVkSamplerAddressModeEnums.cbegin(), CoreVkSamplerAddressModeEnums.cend());
    std::set<VkSamplerAddressMode> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkSamplerAddressModeEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkSamplerMipmapMode> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkSamplerMipmapModeEnums = { VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_MIPMAP_MODE_LINEAR,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkSamplerMipmapMode>> ExtendedVkSamplerMipmapModeEnums = {
    };
    std::vector<VkSamplerMipmapMode> values(CoreVkSamplerMipmapModeEnums.cbegin(), CoreVkSamplerMipmapModeEnums.cend());
    std::set<VkSamplerMipmapMode> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkSamplerMipmapModeEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkDescriptorType> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkDescriptorTypeEnums = { VK_DESCRIPTOR_TYPE_SAMPLER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkDescriptorType>> ExtendedVkDescriptorTypeEnums = {
        { &DeviceExtensions::vk_ext_inline_uniform_block, { VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK,  } },
        { &DeviceExtensions::vk_khr_acceleration_structure, { VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,  } },
        { &DeviceExtensions::vk_nv_ray_tracing, { VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV,  } },
        { &DeviceExtensions::vk_qcom_image_processing, { VK_DESCRIPTOR_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM, VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM,  } },
        { &DeviceExtensions::vk_ext_mutable_descriptor_type, { VK_DESCRIPTOR_TYPE_MUTABLE_EXT,  } },
        { &DeviceExtensions::vk_valve_mutable_descriptor_type, { VK_DESCRIPTOR_TYPE_MUTABLE_EXT,  } },
    };
    std::vector<VkDescriptorType> values(CoreVkDescriptorTypeEnums.cbegin(), CoreVkDescriptorTypeEnums.cend());
    std::set<VkDescriptorType> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkDescriptorTypeEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkAttachmentLoadOp> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkAttachmentLoadOpEnums = { VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_LOAD_OP_DONT_CARE,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkAttachmentLoadOp>> ExtendedVkAttachmentLoadOpEnums = {
        { &DeviceExtensions::vk_ext_load_store_op_none, { VK_ATTACHMENT_LOAD_OP_NONE_EXT,  } },
    };
    std::vector<VkAttachmentLoadOp> values(CoreVkAttachmentLoadOpEnums.cbegin(), CoreVkAttachmentLoadOpEnums.cend());
    std::set<VkAttachmentLoadOp> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkAttachmentLoadOpEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkAttachmentStoreOp> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkAttachmentStoreOpEnums = { VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_STORE_OP_DONT_CARE,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkAttachmentStoreOp>> ExtendedVkAttachmentStoreOpEnums = {
        { &DeviceExtensions::vk_ext_load_store_op_none, { VK_ATTACHMENT_STORE_OP_NONE,  } },
        { &DeviceExtensions::vk_khr_dynamic_rendering, { VK_ATTACHMENT_STORE_OP_NONE,  } },
        { &DeviceExtensions::vk_qcom_render_pass_store_ops, { VK_ATTACHMENT_STORE_OP_NONE,  } },
    };
    std::vector<VkAttachmentStoreOp> values(CoreVkAttachmentStoreOpEnums.cbegin(), CoreVkAttachmentStoreOpEnums.cend());
    std::set<VkAttachmentStoreOp> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkAttachmentStoreOpEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkPipelineBindPoint> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkPipelineBindPointEnums = { VK_PIPELINE_BIND_POINT_GRAPHICS, VK_PIPELINE_BIND_POINT_COMPUTE,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkPipelineBindPoint>> ExtendedVkPipelineBindPointEnums = {
        { &DeviceExtensions::vk_khr_ray_tracing_pipeline, { VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,  } },
        { &DeviceExtensions::vk_nv_ray_tracing, { VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,  } },
        { &DeviceExtensions::vk_huawei_subpass_shading, { VK_PIPELINE_BIND_POINT_SUBPASS_SHADING_HUAWEI,  } },
    };
    std::vector<VkPipelineBindPoint> values(CoreVkPipelineBindPointEnums.cbegin(), CoreVkPipelineBindPointEnums.cend());
    std::set<VkPipelineBindPoint> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkPipelineBindPointEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkCommandBufferLevel> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkCommandBufferLevelEnums = { VK_COMMAND_BUFFER_LEVEL_PRIMARY, VK_COMMAND_BUFFER_LEVEL_SECONDARY,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkCommandBufferLevel>> ExtendedVkCommandBufferLevelEnums = {
    };
    std::vector<VkCommandBufferLevel> values(CoreVkCommandBufferLevelEnums.cbegin(), CoreVkCommandBufferLevelEnums.cend());
    std::set<VkCommandBufferLevel> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkCommandBufferLevelEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkIndexType> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkIndexTypeEnums = { VK_INDEX_TYPE_UINT16, VK_INDEX_TYPE_UINT32,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkIndexType>> ExtendedVkIndexTypeEnums = {
        { &DeviceExtensions::vk_khr_acceleration_structure, { VK_INDEX_TYPE_NONE_KHR,  } },
        { &DeviceExtensions::vk_nv_ray_tracing, { VK_INDEX_TYPE_NONE_KHR,  } },
        { &DeviceExtensions::vk_ext_index_type_uint8, { VK_INDEX_TYPE_UINT8_EXT,  } },
    };
    std::vector<VkIndexType> values(CoreVkIndexTypeEnums.cbegin(), CoreVkIndexTypeEnums.cend());
    std::set<VkIndexType> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkIndexTypeEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkSubpassContents> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkSubpassContentsEnums = { VK_SUBPASS_CONTENTS_INLINE, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkSubpassContents>> ExtendedVkSubpassContentsEnums = {
    };
    std::vector<VkSubpassContents> values(CoreVkSubpassContentsEnums.cbegin(), CoreVkSubpassContentsEnums.cend());
    std::set<VkSubpassContents> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkSubpassContentsEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkTessellationDomainOrigin> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkTessellationDomainOriginEnums = { VK_TESSELLATION_DOMAIN_ORIGIN_UPPER_LEFT, VK_TESSELLATION_DOMAIN_ORIGIN_LOWER_LEFT,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkTessellationDomainOrigin>> ExtendedVkTessellationDomainOriginEnums = {
    };
    std::vector<VkTessellationDomainOrigin> values(CoreVkTessellationDomainOriginEnums.cbegin(), CoreVkTessellationDomainOriginEnums.cend());
    std::set<VkTessellationDomainOrigin> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkTessellationDomainOriginEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkSamplerYcbcrModelConversion> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkSamplerYcbcrModelConversionEnums = { VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY, VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_IDENTITY, VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_709, VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_601, VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_2020,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkSamplerYcbcrModelConversion>> ExtendedVkSamplerYcbcrModelConversionEnums = {
    };
    std::vector<VkSamplerYcbcrModelConversion> values(CoreVkSamplerYcbcrModelConversionEnums.cbegin(), CoreVkSamplerYcbcrModelConversionEnums.cend());
    std::set<VkSamplerYcbcrModelConversion> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkSamplerYcbcrModelConversionEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkSamplerYcbcrRange> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkSamplerYcbcrRangeEnums = { VK_SAMPLER_YCBCR_RANGE_ITU_FULL, VK_SAMPLER_YCBCR_RANGE_ITU_NARROW,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkSamplerYcbcrRange>> ExtendedVkSamplerYcbcrRangeEnums = {
    };
    std::vector<VkSamplerYcbcrRange> values(CoreVkSamplerYcbcrRangeEnums.cbegin(), CoreVkSamplerYcbcrRangeEnums.cend());
    std::set<VkSamplerYcbcrRange> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkSamplerYcbcrRangeEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkChromaLocation> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkChromaLocationEnums = { VK_CHROMA_LOCATION_COSITED_EVEN, VK_CHROMA_LOCATION_MIDPOINT,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkChromaLocation>> ExtendedVkChromaLocationEnums = {
    };
    std::vector<VkChromaLocation> values(CoreVkChromaLocationEnums.cbegin(), CoreVkChromaLocationEnums.cend());
    std::set<VkChromaLocation> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkChromaLocationEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkDescriptorUpdateTemplateType> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkDescriptorUpdateTemplateTypeEnums = { VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkDescriptorUpdateTemplateType>> ExtendedVkDescriptorUpdateTemplateTypeEnums = {
        { &DeviceExtensions::vk_khr_descriptor_update_template, { VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_PUSH_DESCRIPTORS_KHR, VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_PUSH_DESCRIPTORS_KHR, VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_PUSH_DESCRIPTORS_KHR,  } },
        { &DeviceExtensions::vk_khr_push_descriptor, { VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_PUSH_DESCRIPTORS_KHR, VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_PUSH_DESCRIPTORS_KHR, VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_PUSH_DESCRIPTORS_KHR,  } },
    };
    std::vector<VkDescriptorUpdateTemplateType> values(CoreVkDescriptorUpdateTemplateTypeEnums.cbegin(), CoreVkDescriptorUpdateTemplateTypeEnums.cend());
    std::set<VkDescriptorUpdateTemplateType> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkDescriptorUpdateTemplateTypeEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkSamplerReductionMode> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkSamplerReductionModeEnums = { VK_SAMPLER_REDUCTION_MODE_WEIGHTED_AVERAGE, VK_SAMPLER_REDUCTION_MODE_MIN, VK_SAMPLER_REDUCTION_MODE_MAX,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkSamplerReductionMode>> ExtendedVkSamplerReductionModeEnums = {
    };
    std::vector<VkSamplerReductionMode> values(CoreVkSamplerReductionModeEnums.cbegin(), CoreVkSamplerReductionModeEnums.cend());
    std::set<VkSamplerReductionMode> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkSamplerReductionModeEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkSemaphoreType> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkSemaphoreTypeEnums = { VK_SEMAPHORE_TYPE_BINARY, VK_SEMAPHORE_TYPE_TIMELINE,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkSemaphoreType>> ExtendedVkSemaphoreTypeEnums = {
    };
    std::vector<VkSemaphoreType> values(CoreVkSemaphoreTypeEnums.cbegin(), CoreVkSemaphoreTypeEnums.cend());
    std::set<VkSemaphoreType> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkSemaphoreTypeEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkPresentModeKHR> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkPresentModeKHREnums = { VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_FIFO_KHR, VK_PRESENT_MODE_FIFO_RELAXED_KHR,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkPresentModeKHR>> ExtendedVkPresentModeKHREnums = {
        { &DeviceExtensions::vk_khr_shared_presentable_image, { VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR, VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR,  } },
    };
    std::vector<VkPresentModeKHR> values(CoreVkPresentModeKHREnums.cbegin(), CoreVkPresentModeKHREnums.cend());
    std::set<VkPresentModeKHR> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkPresentModeKHREnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkColorSpaceKHR> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkColorSpaceKHREnums = { VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkColorSpaceKHR>> ExtendedVkColorSpaceKHREnums = {
        { &DeviceExtensions::vk_ext_swapchain_colorspace, { VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT, VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT, VK_COLOR_SPACE_DISPLAY_P3_LINEAR_EXT, VK_COLOR_SPACE_DCI_P3_NONLINEAR_EXT, VK_COLOR_SPACE_BT709_LINEAR_EXT, VK_COLOR_SPACE_BT709_NONLINEAR_EXT, VK_COLOR_SPACE_BT2020_LINEAR_EXT, VK_COLOR_SPACE_HDR10_ST2084_EXT, VK_COLOR_SPACE_DOLBYVISION_EXT, VK_COLOR_SPACE_HDR10_HLG_EXT, VK_COLOR_SPACE_ADOBERGB_LINEAR_EXT, VK_COLOR_SPACE_ADOBERGB_NONLINEAR_EXT, VK_COLOR_SPACE_PASS_THROUGH_EXT, VK_COLOR_SPACE_EXTENDED_SRGB_NONLINEAR_EXT,  } },
        { &DeviceExtensions::vk_amd_display_native_hdr, { VK_COLOR_SPACE_DISPLAY_NATIVE_AMD,  } },
    };
    std::vector<VkColorSpaceKHR> values(CoreVkColorSpaceKHREnums.cbegin(), CoreVkColorSpaceKHREnums.cend());
    std::set<VkColorSpaceKHR> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkColorSpaceKHREnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkQueueGlobalPriorityKHR> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkQueueGlobalPriorityKHREnums = { VK_QUEUE_GLOBAL_PRIORITY_LOW_KHR, VK_QUEUE_GLOBAL_PRIORITY_MEDIUM_KHR, VK_QUEUE_GLOBAL_PRIORITY_HIGH_KHR, VK_QUEUE_GLOBAL_PRIORITY_REALTIME_KHR,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkQueueGlobalPriorityKHR>> ExtendedVkQueueGlobalPriorityKHREnums = {
    };
    std::vector<VkQueueGlobalPriorityKHR> values(CoreVkQueueGlobalPriorityKHREnums.cbegin(), CoreVkQueueGlobalPriorityKHREnums.cend());
    std::set<VkQueueGlobalPriorityKHR> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkQueueGlobalPriorityKHREnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkFragmentShadingRateCombinerOpKHR> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkFragmentShadingRateCombinerOpKHREnums = { VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR, VK_FRAGMENT_SHADING_RATE_COMBINER_OP_REPLACE_KHR, VK_FRAGMENT_SHADING_RATE_COMBINER_OP_MIN_KHR, VK_FRAGMENT_SHADING_RATE_COMBINER_OP_MAX_KHR, VK_FRAGMENT_SHADING_RATE_COMBINER_OP_MUL_KHR,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkFragmentShadingRateCombinerOpKHR>> ExtendedVkFragmentShadingRateCombinerOpKHREnums = {
    };
    std::vector<VkFragmentShadingRateCombinerOpKHR> values(CoreVkFragmentShadingRateCombinerOpKHREnums.cbegin(), CoreVkFragmentShadingRateCombinerOpKHREnums.cend());
    std::set<VkFragmentShadingRateCombinerOpKHR> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkFragmentShadingRateCombinerOpKHREnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}

#ifdef VK_ENABLE_BETA_EXTENSIONS

template<>
std::vector<VkVideoEncodeTuningModeKHR> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkVideoEncodeTuningModeKHREnums = { VK_VIDEO_ENCODE_TUNING_MODE_DEFAULT_KHR, VK_VIDEO_ENCODE_TUNING_MODE_HIGH_QUALITY_KHR, VK_VIDEO_ENCODE_TUNING_MODE_LOW_LATENCY_KHR, VK_VIDEO_ENCODE_TUNING_MODE_ULTRA_LOW_LATENCY_KHR, VK_VIDEO_ENCODE_TUNING_MODE_LOSSLESS_KHR,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkVideoEncodeTuningModeKHR>> ExtendedVkVideoEncodeTuningModeKHREnums = {
    };
    std::vector<VkVideoEncodeTuningModeKHR> values(CoreVkVideoEncodeTuningModeKHREnums.cbegin(), CoreVkVideoEncodeTuningModeKHREnums.cend());
    std::set<VkVideoEncodeTuningModeKHR> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkVideoEncodeTuningModeKHREnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}

#endif // VK_ENABLE_BETA_EXTENSIONS

template<>
std::vector<VkDebugReportObjectTypeEXT> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkDebugReportObjectTypeEXTEnums = { VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_EVENT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_QUERY_POOL_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_VIEW_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_CACHE_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_POOL_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_SURFACE_KHR_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DISPLAY_KHR_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DISPLAY_MODE_KHR_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_VALIDATION_CACHE_EXT_EXT,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkDebugReportObjectTypeEXT>> ExtendedVkDebugReportObjectTypeEXTEnums = {
        { &DeviceExtensions::vk_ext_debug_report, { VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION_EXT,  } },
        { &DeviceExtensions::vk_khr_sampler_ycbcr_conversion, { VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION_EXT,  } },
        { &DeviceExtensions::vk_khr_descriptor_update_template, { VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_EXT,  } },
        { &DeviceExtensions::vk_nvx_binary_import, { VK_DEBUG_REPORT_OBJECT_TYPE_CU_MODULE_NVX_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_CU_FUNCTION_NVX_EXT,  } },
        { &DeviceExtensions::vk_khr_acceleration_structure, { VK_DEBUG_REPORT_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR_EXT,  } },
        { &DeviceExtensions::vk_nv_ray_tracing, { VK_DEBUG_REPORT_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV_EXT,  } },
        { &DeviceExtensions::vk_fuchsia_buffer_collection, { VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_COLLECTION_FUCHSIA_EXT,  } },
    };
    std::vector<VkDebugReportObjectTypeEXT> values(CoreVkDebugReportObjectTypeEXTEnums.cbegin(), CoreVkDebugReportObjectTypeEXTEnums.cend());
    std::set<VkDebugReportObjectTypeEXT> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkDebugReportObjectTypeEXTEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkRasterizationOrderAMD> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkRasterizationOrderAMDEnums = { VK_RASTERIZATION_ORDER_STRICT_AMD, VK_RASTERIZATION_ORDER_RELAXED_AMD,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkRasterizationOrderAMD>> ExtendedVkRasterizationOrderAMDEnums = {
    };
    std::vector<VkRasterizationOrderAMD> values(CoreVkRasterizationOrderAMDEnums.cbegin(), CoreVkRasterizationOrderAMDEnums.cend());
    std::set<VkRasterizationOrderAMD> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkRasterizationOrderAMDEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}

#ifdef VK_ENABLE_BETA_EXTENSIONS

template<>
std::vector<VkVideoEncodeH264RateControlStructureEXT> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkVideoEncodeH264RateControlStructureEXTEnums = { VK_VIDEO_ENCODE_H264_RATE_CONTROL_STRUCTURE_UNKNOWN_EXT, VK_VIDEO_ENCODE_H264_RATE_CONTROL_STRUCTURE_FLAT_EXT, VK_VIDEO_ENCODE_H264_RATE_CONTROL_STRUCTURE_DYADIC_EXT,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkVideoEncodeH264RateControlStructureEXT>> ExtendedVkVideoEncodeH264RateControlStructureEXTEnums = {
    };
    std::vector<VkVideoEncodeH264RateControlStructureEXT> values(CoreVkVideoEncodeH264RateControlStructureEXTEnums.cbegin(), CoreVkVideoEncodeH264RateControlStructureEXTEnums.cend());
    std::set<VkVideoEncodeH264RateControlStructureEXT> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkVideoEncodeH264RateControlStructureEXTEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}

#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS

template<>
std::vector<VkVideoEncodeH265RateControlStructureEXT> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkVideoEncodeH265RateControlStructureEXTEnums = { VK_VIDEO_ENCODE_H265_RATE_CONTROL_STRUCTURE_UNKNOWN_EXT, VK_VIDEO_ENCODE_H265_RATE_CONTROL_STRUCTURE_FLAT_EXT, VK_VIDEO_ENCODE_H265_RATE_CONTROL_STRUCTURE_DYADIC_EXT,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkVideoEncodeH265RateControlStructureEXT>> ExtendedVkVideoEncodeH265RateControlStructureEXTEnums = {
    };
    std::vector<VkVideoEncodeH265RateControlStructureEXT> values(CoreVkVideoEncodeH265RateControlStructureEXTEnums.cbegin(), CoreVkVideoEncodeH265RateControlStructureEXTEnums.cend());
    std::set<VkVideoEncodeH265RateControlStructureEXT> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkVideoEncodeH265RateControlStructureEXTEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}

#endif // VK_ENABLE_BETA_EXTENSIONS

template<>
std::vector<VkShaderInfoTypeAMD> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkShaderInfoTypeAMDEnums = { VK_SHADER_INFO_TYPE_STATISTICS_AMD, VK_SHADER_INFO_TYPE_BINARY_AMD, VK_SHADER_INFO_TYPE_DISASSEMBLY_AMD,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkShaderInfoTypeAMD>> ExtendedVkShaderInfoTypeAMDEnums = {
    };
    std::vector<VkShaderInfoTypeAMD> values(CoreVkShaderInfoTypeAMDEnums.cbegin(), CoreVkShaderInfoTypeAMDEnums.cend());
    std::set<VkShaderInfoTypeAMD> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkShaderInfoTypeAMDEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkValidationCheckEXT> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkValidationCheckEXTEnums = { VK_VALIDATION_CHECK_ALL_EXT, VK_VALIDATION_CHECK_SHADERS_EXT,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkValidationCheckEXT>> ExtendedVkValidationCheckEXTEnums = {
    };
    std::vector<VkValidationCheckEXT> values(CoreVkValidationCheckEXTEnums.cbegin(), CoreVkValidationCheckEXTEnums.cend());
    std::set<VkValidationCheckEXT> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkValidationCheckEXTEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkPipelineRobustnessBufferBehaviorEXT> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkPipelineRobustnessBufferBehaviorEXTEnums = { VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_DEVICE_DEFAULT_EXT, VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_DISABLED_EXT, VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS_EXT, VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS_2_EXT,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkPipelineRobustnessBufferBehaviorEXT>> ExtendedVkPipelineRobustnessBufferBehaviorEXTEnums = {
    };
    std::vector<VkPipelineRobustnessBufferBehaviorEXT> values(CoreVkPipelineRobustnessBufferBehaviorEXTEnums.cbegin(), CoreVkPipelineRobustnessBufferBehaviorEXTEnums.cend());
    std::set<VkPipelineRobustnessBufferBehaviorEXT> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkPipelineRobustnessBufferBehaviorEXTEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkPipelineRobustnessImageBehaviorEXT> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkPipelineRobustnessImageBehaviorEXTEnums = { VK_PIPELINE_ROBUSTNESS_IMAGE_BEHAVIOR_DEVICE_DEFAULT_EXT, VK_PIPELINE_ROBUSTNESS_IMAGE_BEHAVIOR_DISABLED_EXT, VK_PIPELINE_ROBUSTNESS_IMAGE_BEHAVIOR_ROBUST_IMAGE_ACCESS_EXT, VK_PIPELINE_ROBUSTNESS_IMAGE_BEHAVIOR_ROBUST_IMAGE_ACCESS_2_EXT,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkPipelineRobustnessImageBehaviorEXT>> ExtendedVkPipelineRobustnessImageBehaviorEXTEnums = {
    };
    std::vector<VkPipelineRobustnessImageBehaviorEXT> values(CoreVkPipelineRobustnessImageBehaviorEXTEnums.cbegin(), CoreVkPipelineRobustnessImageBehaviorEXTEnums.cend());
    std::set<VkPipelineRobustnessImageBehaviorEXT> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkPipelineRobustnessImageBehaviorEXTEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkDisplayPowerStateEXT> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkDisplayPowerStateEXTEnums = { VK_DISPLAY_POWER_STATE_OFF_EXT, VK_DISPLAY_POWER_STATE_SUSPEND_EXT, VK_DISPLAY_POWER_STATE_ON_EXT,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkDisplayPowerStateEXT>> ExtendedVkDisplayPowerStateEXTEnums = {
    };
    std::vector<VkDisplayPowerStateEXT> values(CoreVkDisplayPowerStateEXTEnums.cbegin(), CoreVkDisplayPowerStateEXTEnums.cend());
    std::set<VkDisplayPowerStateEXT> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkDisplayPowerStateEXTEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkDeviceEventTypeEXT> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkDeviceEventTypeEXTEnums = { VK_DEVICE_EVENT_TYPE_DISPLAY_HOTPLUG_EXT,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkDeviceEventTypeEXT>> ExtendedVkDeviceEventTypeEXTEnums = {
    };
    std::vector<VkDeviceEventTypeEXT> values(CoreVkDeviceEventTypeEXTEnums.cbegin(), CoreVkDeviceEventTypeEXTEnums.cend());
    std::set<VkDeviceEventTypeEXT> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkDeviceEventTypeEXTEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkDisplayEventTypeEXT> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkDisplayEventTypeEXTEnums = { VK_DISPLAY_EVENT_TYPE_FIRST_PIXEL_OUT_EXT,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkDisplayEventTypeEXT>> ExtendedVkDisplayEventTypeEXTEnums = {
    };
    std::vector<VkDisplayEventTypeEXT> values(CoreVkDisplayEventTypeEXTEnums.cbegin(), CoreVkDisplayEventTypeEXTEnums.cend());
    std::set<VkDisplayEventTypeEXT> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkDisplayEventTypeEXTEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkViewportCoordinateSwizzleNV> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkViewportCoordinateSwizzleNVEnums = { VK_VIEWPORT_COORDINATE_SWIZZLE_POSITIVE_X_NV, VK_VIEWPORT_COORDINATE_SWIZZLE_NEGATIVE_X_NV, VK_VIEWPORT_COORDINATE_SWIZZLE_POSITIVE_Y_NV, VK_VIEWPORT_COORDINATE_SWIZZLE_NEGATIVE_Y_NV, VK_VIEWPORT_COORDINATE_SWIZZLE_POSITIVE_Z_NV, VK_VIEWPORT_COORDINATE_SWIZZLE_NEGATIVE_Z_NV, VK_VIEWPORT_COORDINATE_SWIZZLE_POSITIVE_W_NV, VK_VIEWPORT_COORDINATE_SWIZZLE_NEGATIVE_W_NV,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkViewportCoordinateSwizzleNV>> ExtendedVkViewportCoordinateSwizzleNVEnums = {
    };
    std::vector<VkViewportCoordinateSwizzleNV> values(CoreVkViewportCoordinateSwizzleNVEnums.cbegin(), CoreVkViewportCoordinateSwizzleNVEnums.cend());
    std::set<VkViewportCoordinateSwizzleNV> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkViewportCoordinateSwizzleNVEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkDiscardRectangleModeEXT> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkDiscardRectangleModeEXTEnums = { VK_DISCARD_RECTANGLE_MODE_INCLUSIVE_EXT, VK_DISCARD_RECTANGLE_MODE_EXCLUSIVE_EXT,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkDiscardRectangleModeEXT>> ExtendedVkDiscardRectangleModeEXTEnums = {
    };
    std::vector<VkDiscardRectangleModeEXT> values(CoreVkDiscardRectangleModeEXTEnums.cbegin(), CoreVkDiscardRectangleModeEXTEnums.cend());
    std::set<VkDiscardRectangleModeEXT> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkDiscardRectangleModeEXTEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkConservativeRasterizationModeEXT> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkConservativeRasterizationModeEXTEnums = { VK_CONSERVATIVE_RASTERIZATION_MODE_DISABLED_EXT, VK_CONSERVATIVE_RASTERIZATION_MODE_OVERESTIMATE_EXT, VK_CONSERVATIVE_RASTERIZATION_MODE_UNDERESTIMATE_EXT,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkConservativeRasterizationModeEXT>> ExtendedVkConservativeRasterizationModeEXTEnums = {
    };
    std::vector<VkConservativeRasterizationModeEXT> values(CoreVkConservativeRasterizationModeEXTEnums.cbegin(), CoreVkConservativeRasterizationModeEXTEnums.cend());
    std::set<VkConservativeRasterizationModeEXT> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkConservativeRasterizationModeEXTEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkBlendOverlapEXT> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkBlendOverlapEXTEnums = { VK_BLEND_OVERLAP_UNCORRELATED_EXT, VK_BLEND_OVERLAP_DISJOINT_EXT, VK_BLEND_OVERLAP_CONJOINT_EXT,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkBlendOverlapEXT>> ExtendedVkBlendOverlapEXTEnums = {
    };
    std::vector<VkBlendOverlapEXT> values(CoreVkBlendOverlapEXTEnums.cbegin(), CoreVkBlendOverlapEXTEnums.cend());
    std::set<VkBlendOverlapEXT> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkBlendOverlapEXTEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkCoverageModulationModeNV> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkCoverageModulationModeNVEnums = { VK_COVERAGE_MODULATION_MODE_NONE_NV, VK_COVERAGE_MODULATION_MODE_RGB_NV, VK_COVERAGE_MODULATION_MODE_ALPHA_NV, VK_COVERAGE_MODULATION_MODE_RGBA_NV,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkCoverageModulationModeNV>> ExtendedVkCoverageModulationModeNVEnums = {
    };
    std::vector<VkCoverageModulationModeNV> values(CoreVkCoverageModulationModeNVEnums.cbegin(), CoreVkCoverageModulationModeNVEnums.cend());
    std::set<VkCoverageModulationModeNV> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkCoverageModulationModeNVEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkShadingRatePaletteEntryNV> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkShadingRatePaletteEntryNVEnums = { VK_SHADING_RATE_PALETTE_ENTRY_NO_INVOCATIONS_NV, VK_SHADING_RATE_PALETTE_ENTRY_16_INVOCATIONS_PER_PIXEL_NV, VK_SHADING_RATE_PALETTE_ENTRY_8_INVOCATIONS_PER_PIXEL_NV, VK_SHADING_RATE_PALETTE_ENTRY_4_INVOCATIONS_PER_PIXEL_NV, VK_SHADING_RATE_PALETTE_ENTRY_2_INVOCATIONS_PER_PIXEL_NV, VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_PIXEL_NV, VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_2X1_PIXELS_NV, VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_1X2_PIXELS_NV, VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_2X2_PIXELS_NV, VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_4X2_PIXELS_NV, VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_2X4_PIXELS_NV, VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_4X4_PIXELS_NV,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkShadingRatePaletteEntryNV>> ExtendedVkShadingRatePaletteEntryNVEnums = {
    };
    std::vector<VkShadingRatePaletteEntryNV> values(CoreVkShadingRatePaletteEntryNVEnums.cbegin(), CoreVkShadingRatePaletteEntryNVEnums.cend());
    std::set<VkShadingRatePaletteEntryNV> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkShadingRatePaletteEntryNVEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkCoarseSampleOrderTypeNV> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkCoarseSampleOrderTypeNVEnums = { VK_COARSE_SAMPLE_ORDER_TYPE_DEFAULT_NV, VK_COARSE_SAMPLE_ORDER_TYPE_CUSTOM_NV, VK_COARSE_SAMPLE_ORDER_TYPE_PIXEL_MAJOR_NV, VK_COARSE_SAMPLE_ORDER_TYPE_SAMPLE_MAJOR_NV,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkCoarseSampleOrderTypeNV>> ExtendedVkCoarseSampleOrderTypeNVEnums = {
    };
    std::vector<VkCoarseSampleOrderTypeNV> values(CoreVkCoarseSampleOrderTypeNVEnums.cbegin(), CoreVkCoarseSampleOrderTypeNVEnums.cend());
    std::set<VkCoarseSampleOrderTypeNV> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkCoarseSampleOrderTypeNVEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkRayTracingShaderGroupTypeKHR> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkRayTracingShaderGroupTypeKHREnums = { VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR, VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR, VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkRayTracingShaderGroupTypeKHR>> ExtendedVkRayTracingShaderGroupTypeKHREnums = {
    };
    std::vector<VkRayTracingShaderGroupTypeKHR> values(CoreVkRayTracingShaderGroupTypeKHREnums.cbegin(), CoreVkRayTracingShaderGroupTypeKHREnums.cend());
    std::set<VkRayTracingShaderGroupTypeKHR> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkRayTracingShaderGroupTypeKHREnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkGeometryTypeKHR> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkGeometryTypeKHREnums = { VK_GEOMETRY_TYPE_TRIANGLES_KHR, VK_GEOMETRY_TYPE_AABBS_KHR, VK_GEOMETRY_TYPE_INSTANCES_KHR,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkGeometryTypeKHR>> ExtendedVkGeometryTypeKHREnums = {
    };
    std::vector<VkGeometryTypeKHR> values(CoreVkGeometryTypeKHREnums.cbegin(), CoreVkGeometryTypeKHREnums.cend());
    std::set<VkGeometryTypeKHR> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkGeometryTypeKHREnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkAccelerationStructureTypeKHR> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkAccelerationStructureTypeKHREnums = { VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR, VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR, VK_ACCELERATION_STRUCTURE_TYPE_GENERIC_KHR,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkAccelerationStructureTypeKHR>> ExtendedVkAccelerationStructureTypeKHREnums = {
    };
    std::vector<VkAccelerationStructureTypeKHR> values(CoreVkAccelerationStructureTypeKHREnums.cbegin(), CoreVkAccelerationStructureTypeKHREnums.cend());
    std::set<VkAccelerationStructureTypeKHR> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkAccelerationStructureTypeKHREnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}



template<>
std::vector<VkCopyAccelerationStructureModeKHR> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkCopyAccelerationStructureModeKHREnums = { VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_KHR, VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_KHR, VK_COPY_ACCELERATION_STRUCTURE_MODE_SERIALIZE_KHR, VK_COPY_ACCELERATION_STRUCTURE_MODE_DESERIALIZE_KHR,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkCopyAccelerationStructureModeKHR>> ExtendedVkCopyAccelerationStructureModeKHREnums = {
    };
    std::vector<VkCopyAccelerationStructureModeKHR> values(CoreVkCopyAccelerationStructureModeKHREnums.cbegin(), CoreVkCopyAccelerationStructureModeKHREnums.cend());
    std::set<VkCopyAccelerationStructureModeKHR> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkCopyAccelerationStructureModeKHREnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkAccelerationStructureMemoryRequirementsTypeNV> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkAccelerationStructureMemoryRequirementsTypeNVEnums = { VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV, VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_NV, VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_UPDATE_SCRATCH_NV,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkAccelerationStructureMemoryRequirementsTypeNV>> ExtendedVkAccelerationStructureMemoryRequirementsTypeNVEnums = {
    };
    std::vector<VkAccelerationStructureMemoryRequirementsTypeNV> values(CoreVkAccelerationStructureMemoryRequirementsTypeNVEnums.cbegin(), CoreVkAccelerationStructureMemoryRequirementsTypeNVEnums.cend());
    std::set<VkAccelerationStructureMemoryRequirementsTypeNV> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkAccelerationStructureMemoryRequirementsTypeNVEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkTimeDomainEXT> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkTimeDomainEXTEnums = { VK_TIME_DOMAIN_DEVICE_EXT, VK_TIME_DOMAIN_CLOCK_MONOTONIC_EXT, VK_TIME_DOMAIN_CLOCK_MONOTONIC_RAW_EXT, VK_TIME_DOMAIN_QUERY_PERFORMANCE_COUNTER_EXT,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkTimeDomainEXT>> ExtendedVkTimeDomainEXTEnums = {
    };
    std::vector<VkTimeDomainEXT> values(CoreVkTimeDomainEXTEnums.cbegin(), CoreVkTimeDomainEXTEnums.cend());
    std::set<VkTimeDomainEXT> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkTimeDomainEXTEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkMemoryOverallocationBehaviorAMD> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkMemoryOverallocationBehaviorAMDEnums = { VK_MEMORY_OVERALLOCATION_BEHAVIOR_DEFAULT_AMD, VK_MEMORY_OVERALLOCATION_BEHAVIOR_ALLOWED_AMD, VK_MEMORY_OVERALLOCATION_BEHAVIOR_DISALLOWED_AMD,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkMemoryOverallocationBehaviorAMD>> ExtendedVkMemoryOverallocationBehaviorAMDEnums = {
    };
    std::vector<VkMemoryOverallocationBehaviorAMD> values(CoreVkMemoryOverallocationBehaviorAMDEnums.cbegin(), CoreVkMemoryOverallocationBehaviorAMDEnums.cend());
    std::set<VkMemoryOverallocationBehaviorAMD> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkMemoryOverallocationBehaviorAMDEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkPerformanceConfigurationTypeINTEL> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkPerformanceConfigurationTypeINTELEnums = { VK_PERFORMANCE_CONFIGURATION_TYPE_COMMAND_QUEUE_METRICS_DISCOVERY_ACTIVATED_INTEL,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkPerformanceConfigurationTypeINTEL>> ExtendedVkPerformanceConfigurationTypeINTELEnums = {
    };
    std::vector<VkPerformanceConfigurationTypeINTEL> values(CoreVkPerformanceConfigurationTypeINTELEnums.cbegin(), CoreVkPerformanceConfigurationTypeINTELEnums.cend());
    std::set<VkPerformanceConfigurationTypeINTEL> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkPerformanceConfigurationTypeINTELEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkQueryPoolSamplingModeINTEL> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkQueryPoolSamplingModeINTELEnums = { VK_QUERY_POOL_SAMPLING_MODE_MANUAL_INTEL,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkQueryPoolSamplingModeINTEL>> ExtendedVkQueryPoolSamplingModeINTELEnums = {
    };
    std::vector<VkQueryPoolSamplingModeINTEL> values(CoreVkQueryPoolSamplingModeINTELEnums.cbegin(), CoreVkQueryPoolSamplingModeINTELEnums.cend());
    std::set<VkQueryPoolSamplingModeINTEL> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkQueryPoolSamplingModeINTELEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkPerformanceOverrideTypeINTEL> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkPerformanceOverrideTypeINTELEnums = { VK_PERFORMANCE_OVERRIDE_TYPE_NULL_HARDWARE_INTEL, VK_PERFORMANCE_OVERRIDE_TYPE_FLUSH_GPU_CACHES_INTEL,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkPerformanceOverrideTypeINTEL>> ExtendedVkPerformanceOverrideTypeINTELEnums = {
    };
    std::vector<VkPerformanceOverrideTypeINTEL> values(CoreVkPerformanceOverrideTypeINTELEnums.cbegin(), CoreVkPerformanceOverrideTypeINTELEnums.cend());
    std::set<VkPerformanceOverrideTypeINTEL> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkPerformanceOverrideTypeINTELEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkPerformanceParameterTypeINTEL> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkPerformanceParameterTypeINTELEnums = { VK_PERFORMANCE_PARAMETER_TYPE_HW_COUNTERS_SUPPORTED_INTEL, VK_PERFORMANCE_PARAMETER_TYPE_STREAM_MARKER_VALID_BITS_INTEL,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkPerformanceParameterTypeINTEL>> ExtendedVkPerformanceParameterTypeINTELEnums = {
    };
    std::vector<VkPerformanceParameterTypeINTEL> values(CoreVkPerformanceParameterTypeINTELEnums.cbegin(), CoreVkPerformanceParameterTypeINTELEnums.cend());
    std::set<VkPerformanceParameterTypeINTEL> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkPerformanceParameterTypeINTELEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkPerformanceValueTypeINTEL> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkPerformanceValueTypeINTELEnums = { VK_PERFORMANCE_VALUE_TYPE_UINT32_INTEL, VK_PERFORMANCE_VALUE_TYPE_UINT64_INTEL, VK_PERFORMANCE_VALUE_TYPE_FLOAT_INTEL, VK_PERFORMANCE_VALUE_TYPE_BOOL_INTEL, VK_PERFORMANCE_VALUE_TYPE_STRING_INTEL,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkPerformanceValueTypeINTEL>> ExtendedVkPerformanceValueTypeINTELEnums = {
    };
    std::vector<VkPerformanceValueTypeINTEL> values(CoreVkPerformanceValueTypeINTELEnums.cbegin(), CoreVkPerformanceValueTypeINTELEnums.cend());
    std::set<VkPerformanceValueTypeINTEL> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkPerformanceValueTypeINTELEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkValidationFeatureEnableEXT> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkValidationFeatureEnableEXTEnums = { VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT, VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT, VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT, VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT, VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkValidationFeatureEnableEXT>> ExtendedVkValidationFeatureEnableEXTEnums = {
    };
    std::vector<VkValidationFeatureEnableEXT> values(CoreVkValidationFeatureEnableEXTEnums.cbegin(), CoreVkValidationFeatureEnableEXTEnums.cend());
    std::set<VkValidationFeatureEnableEXT> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkValidationFeatureEnableEXTEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkValidationFeatureDisableEXT> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkValidationFeatureDisableEXTEnums = { VK_VALIDATION_FEATURE_DISABLE_ALL_EXT, VK_VALIDATION_FEATURE_DISABLE_SHADERS_EXT, VK_VALIDATION_FEATURE_DISABLE_THREAD_SAFETY_EXT, VK_VALIDATION_FEATURE_DISABLE_API_PARAMETERS_EXT, VK_VALIDATION_FEATURE_DISABLE_OBJECT_LIFETIMES_EXT, VK_VALIDATION_FEATURE_DISABLE_CORE_CHECKS_EXT, VK_VALIDATION_FEATURE_DISABLE_UNIQUE_HANDLES_EXT, VK_VALIDATION_FEATURE_DISABLE_SHADER_VALIDATION_CACHE_EXT,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkValidationFeatureDisableEXT>> ExtendedVkValidationFeatureDisableEXTEnums = {
    };
    std::vector<VkValidationFeatureDisableEXT> values(CoreVkValidationFeatureDisableEXTEnums.cbegin(), CoreVkValidationFeatureDisableEXTEnums.cend());
    std::set<VkValidationFeatureDisableEXT> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkValidationFeatureDisableEXTEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkComponentTypeNV> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkComponentTypeNVEnums = { VK_COMPONENT_TYPE_FLOAT16_NV, VK_COMPONENT_TYPE_FLOAT32_NV, VK_COMPONENT_TYPE_FLOAT64_NV, VK_COMPONENT_TYPE_SINT8_NV, VK_COMPONENT_TYPE_SINT16_NV, VK_COMPONENT_TYPE_SINT32_NV, VK_COMPONENT_TYPE_SINT64_NV, VK_COMPONENT_TYPE_UINT8_NV, VK_COMPONENT_TYPE_UINT16_NV, VK_COMPONENT_TYPE_UINT32_NV, VK_COMPONENT_TYPE_UINT64_NV,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkComponentTypeNV>> ExtendedVkComponentTypeNVEnums = {
    };
    std::vector<VkComponentTypeNV> values(CoreVkComponentTypeNVEnums.cbegin(), CoreVkComponentTypeNVEnums.cend());
    std::set<VkComponentTypeNV> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkComponentTypeNVEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkScopeNV> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkScopeNVEnums = { VK_SCOPE_DEVICE_NV, VK_SCOPE_WORKGROUP_NV, VK_SCOPE_SUBGROUP_NV, VK_SCOPE_QUEUE_FAMILY_NV,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkScopeNV>> ExtendedVkScopeNVEnums = {
    };
    std::vector<VkScopeNV> values(CoreVkScopeNVEnums.cbegin(), CoreVkScopeNVEnums.cend());
    std::set<VkScopeNV> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkScopeNVEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkCoverageReductionModeNV> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkCoverageReductionModeNVEnums = { VK_COVERAGE_REDUCTION_MODE_MERGE_NV, VK_COVERAGE_REDUCTION_MODE_TRUNCATE_NV,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkCoverageReductionModeNV>> ExtendedVkCoverageReductionModeNVEnums = {
    };
    std::vector<VkCoverageReductionModeNV> values(CoreVkCoverageReductionModeNVEnums.cbegin(), CoreVkCoverageReductionModeNVEnums.cend());
    std::set<VkCoverageReductionModeNV> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkCoverageReductionModeNVEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkProvokingVertexModeEXT> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkProvokingVertexModeEXTEnums = { VK_PROVOKING_VERTEX_MODE_FIRST_VERTEX_EXT, VK_PROVOKING_VERTEX_MODE_LAST_VERTEX_EXT,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkProvokingVertexModeEXT>> ExtendedVkProvokingVertexModeEXTEnums = {
    };
    std::vector<VkProvokingVertexModeEXT> values(CoreVkProvokingVertexModeEXTEnums.cbegin(), CoreVkProvokingVertexModeEXTEnums.cend());
    std::set<VkProvokingVertexModeEXT> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkProvokingVertexModeEXTEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}

#ifdef VK_USE_PLATFORM_WIN32_KHR

template<>
std::vector<VkFullScreenExclusiveEXT> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkFullScreenExclusiveEXTEnums = { VK_FULL_SCREEN_EXCLUSIVE_DEFAULT_EXT, VK_FULL_SCREEN_EXCLUSIVE_ALLOWED_EXT, VK_FULL_SCREEN_EXCLUSIVE_DISALLOWED_EXT, VK_FULL_SCREEN_EXCLUSIVE_APPLICATION_CONTROLLED_EXT,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkFullScreenExclusiveEXT>> ExtendedVkFullScreenExclusiveEXTEnums = {
    };
    std::vector<VkFullScreenExclusiveEXT> values(CoreVkFullScreenExclusiveEXTEnums.cbegin(), CoreVkFullScreenExclusiveEXTEnums.cend());
    std::set<VkFullScreenExclusiveEXT> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkFullScreenExclusiveEXTEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}

#endif // VK_USE_PLATFORM_WIN32_KHR

template<>
std::vector<VkLineRasterizationModeEXT> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkLineRasterizationModeEXTEnums = { VK_LINE_RASTERIZATION_MODE_DEFAULT_EXT, VK_LINE_RASTERIZATION_MODE_RECTANGULAR_EXT, VK_LINE_RASTERIZATION_MODE_BRESENHAM_EXT, VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH_EXT,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkLineRasterizationModeEXT>> ExtendedVkLineRasterizationModeEXTEnums = {
    };
    std::vector<VkLineRasterizationModeEXT> values(CoreVkLineRasterizationModeEXTEnums.cbegin(), CoreVkLineRasterizationModeEXTEnums.cend());
    std::set<VkLineRasterizationModeEXT> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkLineRasterizationModeEXTEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkIndirectCommandsTokenTypeNV> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkIndirectCommandsTokenTypeNVEnums = { VK_INDIRECT_COMMANDS_TOKEN_TYPE_SHADER_GROUP_NV, VK_INDIRECT_COMMANDS_TOKEN_TYPE_STATE_FLAGS_NV, VK_INDIRECT_COMMANDS_TOKEN_TYPE_INDEX_BUFFER_NV, VK_INDIRECT_COMMANDS_TOKEN_TYPE_VERTEX_BUFFER_NV, VK_INDIRECT_COMMANDS_TOKEN_TYPE_PUSH_CONSTANT_NV, VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_INDEXED_NV, VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_NV, VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_TASKS_NV,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkIndirectCommandsTokenTypeNV>> ExtendedVkIndirectCommandsTokenTypeNVEnums = {
        { &DeviceExtensions::vk_ext_mesh_shader, { VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_MESH_TASKS_NV,  } },
    };
    std::vector<VkIndirectCommandsTokenTypeNV> values(CoreVkIndirectCommandsTokenTypeNVEnums.cbegin(), CoreVkIndirectCommandsTokenTypeNVEnums.cend());
    std::set<VkIndirectCommandsTokenTypeNV> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkIndirectCommandsTokenTypeNVEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkFragmentShadingRateNV> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkFragmentShadingRateNVEnums = { VK_FRAGMENT_SHADING_RATE_1_INVOCATION_PER_PIXEL_NV, VK_FRAGMENT_SHADING_RATE_1_INVOCATION_PER_1X2_PIXELS_NV, VK_FRAGMENT_SHADING_RATE_1_INVOCATION_PER_2X1_PIXELS_NV, VK_FRAGMENT_SHADING_RATE_1_INVOCATION_PER_2X2_PIXELS_NV, VK_FRAGMENT_SHADING_RATE_1_INVOCATION_PER_2X4_PIXELS_NV, VK_FRAGMENT_SHADING_RATE_1_INVOCATION_PER_4X2_PIXELS_NV, VK_FRAGMENT_SHADING_RATE_1_INVOCATION_PER_4X4_PIXELS_NV, VK_FRAGMENT_SHADING_RATE_2_INVOCATIONS_PER_PIXEL_NV, VK_FRAGMENT_SHADING_RATE_4_INVOCATIONS_PER_PIXEL_NV, VK_FRAGMENT_SHADING_RATE_8_INVOCATIONS_PER_PIXEL_NV, VK_FRAGMENT_SHADING_RATE_16_INVOCATIONS_PER_PIXEL_NV, VK_FRAGMENT_SHADING_RATE_NO_INVOCATIONS_NV,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkFragmentShadingRateNV>> ExtendedVkFragmentShadingRateNVEnums = {
    };
    std::vector<VkFragmentShadingRateNV> values(CoreVkFragmentShadingRateNVEnums.cbegin(), CoreVkFragmentShadingRateNVEnums.cend());
    std::set<VkFragmentShadingRateNV> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkFragmentShadingRateNVEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkAccelerationStructureMotionInstanceTypeNV> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkAccelerationStructureMotionInstanceTypeNVEnums = { VK_ACCELERATION_STRUCTURE_MOTION_INSTANCE_TYPE_STATIC_NV, VK_ACCELERATION_STRUCTURE_MOTION_INSTANCE_TYPE_MATRIX_MOTION_NV, VK_ACCELERATION_STRUCTURE_MOTION_INSTANCE_TYPE_SRT_MOTION_NV,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkAccelerationStructureMotionInstanceTypeNV>> ExtendedVkAccelerationStructureMotionInstanceTypeNVEnums = {
    };
    std::vector<VkAccelerationStructureMotionInstanceTypeNV> values(CoreVkAccelerationStructureMotionInstanceTypeNVEnums.cbegin(), CoreVkAccelerationStructureMotionInstanceTypeNVEnums.cend());
    std::set<VkAccelerationStructureMotionInstanceTypeNV> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkAccelerationStructureMotionInstanceTypeNVEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkDeviceFaultAddressTypeEXT> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkDeviceFaultAddressTypeEXTEnums = { VK_DEVICE_FAULT_ADDRESS_TYPE_NONE_EXT, VK_DEVICE_FAULT_ADDRESS_TYPE_READ_INVALID_EXT, VK_DEVICE_FAULT_ADDRESS_TYPE_WRITE_INVALID_EXT, VK_DEVICE_FAULT_ADDRESS_TYPE_EXECUTE_INVALID_EXT, VK_DEVICE_FAULT_ADDRESS_TYPE_INSTRUCTION_POINTER_UNKNOWN_EXT, VK_DEVICE_FAULT_ADDRESS_TYPE_INSTRUCTION_POINTER_INVALID_EXT, VK_DEVICE_FAULT_ADDRESS_TYPE_INSTRUCTION_POINTER_FAULT_EXT,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkDeviceFaultAddressTypeEXT>> ExtendedVkDeviceFaultAddressTypeEXTEnums = {
    };
    std::vector<VkDeviceFaultAddressTypeEXT> values(CoreVkDeviceFaultAddressTypeEXTEnums.cbegin(), CoreVkDeviceFaultAddressTypeEXTEnums.cend());
    std::set<VkDeviceFaultAddressTypeEXT> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkDeviceFaultAddressTypeEXTEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkDeviceFaultVendorBinaryHeaderVersionEXT> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkDeviceFaultVendorBinaryHeaderVersionEXTEnums = { VK_DEVICE_FAULT_VENDOR_BINARY_HEADER_VERSION_ONE_EXT,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkDeviceFaultVendorBinaryHeaderVersionEXT>> ExtendedVkDeviceFaultVendorBinaryHeaderVersionEXTEnums = {
    };
    std::vector<VkDeviceFaultVendorBinaryHeaderVersionEXT> values(CoreVkDeviceFaultVendorBinaryHeaderVersionEXTEnums.cbegin(), CoreVkDeviceFaultVendorBinaryHeaderVersionEXTEnums.cend());
    std::set<VkDeviceFaultVendorBinaryHeaderVersionEXT> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkDeviceFaultVendorBinaryHeaderVersionEXTEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkDeviceAddressBindingTypeEXT> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkDeviceAddressBindingTypeEXTEnums = { VK_DEVICE_ADDRESS_BINDING_TYPE_BIND_EXT, VK_DEVICE_ADDRESS_BINDING_TYPE_UNBIND_EXT,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkDeviceAddressBindingTypeEXT>> ExtendedVkDeviceAddressBindingTypeEXTEnums = {
    };
    std::vector<VkDeviceAddressBindingTypeEXT> values(CoreVkDeviceAddressBindingTypeEXTEnums.cbegin(), CoreVkDeviceAddressBindingTypeEXTEnums.cend());
    std::set<VkDeviceAddressBindingTypeEXT> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkDeviceAddressBindingTypeEXTEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkMicromapTypeEXT> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkMicromapTypeEXTEnums = { VK_MICROMAP_TYPE_OPACITY_MICROMAP_EXT,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkMicromapTypeEXT>> ExtendedVkMicromapTypeEXTEnums = {
    };
    std::vector<VkMicromapTypeEXT> values(CoreVkMicromapTypeEXTEnums.cbegin(), CoreVkMicromapTypeEXTEnums.cend());
    std::set<VkMicromapTypeEXT> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkMicromapTypeEXTEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkCopyMicromapModeEXT> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkCopyMicromapModeEXTEnums = { VK_COPY_MICROMAP_MODE_CLONE_EXT, VK_COPY_MICROMAP_MODE_SERIALIZE_EXT, VK_COPY_MICROMAP_MODE_DESERIALIZE_EXT, VK_COPY_MICROMAP_MODE_COMPACT_EXT,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkCopyMicromapModeEXT>> ExtendedVkCopyMicromapModeEXTEnums = {
    };
    std::vector<VkCopyMicromapModeEXT> values(CoreVkCopyMicromapModeEXTEnums.cbegin(), CoreVkCopyMicromapModeEXTEnums.cend());
    std::set<VkCopyMicromapModeEXT> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkCopyMicromapModeEXTEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkAccelerationStructureBuildTypeKHR> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkAccelerationStructureBuildTypeKHREnums = { VK_ACCELERATION_STRUCTURE_BUILD_TYPE_HOST_KHR, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_HOST_OR_DEVICE_KHR,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkAccelerationStructureBuildTypeKHR>> ExtendedVkAccelerationStructureBuildTypeKHREnums = {
    };
    std::vector<VkAccelerationStructureBuildTypeKHR> values(CoreVkAccelerationStructureBuildTypeKHREnums.cbegin(), CoreVkAccelerationStructureBuildTypeKHREnums.cend());
    std::set<VkAccelerationStructureBuildTypeKHR> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkAccelerationStructureBuildTypeKHREnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkDirectDriverLoadingModeLUNARG> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkDirectDriverLoadingModeLUNARGEnums = { VK_DIRECT_DRIVER_LOADING_MODE_EXCLUSIVE_LUNARG, VK_DIRECT_DRIVER_LOADING_MODE_INCLUSIVE_LUNARG,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkDirectDriverLoadingModeLUNARG>> ExtendedVkDirectDriverLoadingModeLUNARGEnums = {
    };
    std::vector<VkDirectDriverLoadingModeLUNARG> values(CoreVkDirectDriverLoadingModeLUNARGEnums.cbegin(), CoreVkDirectDriverLoadingModeLUNARGEnums.cend());
    std::set<VkDirectDriverLoadingModeLUNARG> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkDirectDriverLoadingModeLUNARGEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkOpticalFlowPerformanceLevelNV> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkOpticalFlowPerformanceLevelNVEnums = { VK_OPTICAL_FLOW_PERFORMANCE_LEVEL_UNKNOWN_NV, VK_OPTICAL_FLOW_PERFORMANCE_LEVEL_SLOW_NV, VK_OPTICAL_FLOW_PERFORMANCE_LEVEL_MEDIUM_NV, VK_OPTICAL_FLOW_PERFORMANCE_LEVEL_FAST_NV,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkOpticalFlowPerformanceLevelNV>> ExtendedVkOpticalFlowPerformanceLevelNVEnums = {
    };
    std::vector<VkOpticalFlowPerformanceLevelNV> values(CoreVkOpticalFlowPerformanceLevelNVEnums.cbegin(), CoreVkOpticalFlowPerformanceLevelNVEnums.cend());
    std::set<VkOpticalFlowPerformanceLevelNV> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkOpticalFlowPerformanceLevelNVEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkOpticalFlowSessionBindingPointNV> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkOpticalFlowSessionBindingPointNVEnums = { VK_OPTICAL_FLOW_SESSION_BINDING_POINT_UNKNOWN_NV, VK_OPTICAL_FLOW_SESSION_BINDING_POINT_INPUT_NV, VK_OPTICAL_FLOW_SESSION_BINDING_POINT_REFERENCE_NV, VK_OPTICAL_FLOW_SESSION_BINDING_POINT_HINT_NV, VK_OPTICAL_FLOW_SESSION_BINDING_POINT_FLOW_VECTOR_NV, VK_OPTICAL_FLOW_SESSION_BINDING_POINT_BACKWARD_FLOW_VECTOR_NV, VK_OPTICAL_FLOW_SESSION_BINDING_POINT_COST_NV, VK_OPTICAL_FLOW_SESSION_BINDING_POINT_BACKWARD_COST_NV, VK_OPTICAL_FLOW_SESSION_BINDING_POINT_GLOBAL_FLOW_NV,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkOpticalFlowSessionBindingPointNV>> ExtendedVkOpticalFlowSessionBindingPointNVEnums = {
    };
    std::vector<VkOpticalFlowSessionBindingPointNV> values(CoreVkOpticalFlowSessionBindingPointNVEnums.cbegin(), CoreVkOpticalFlowSessionBindingPointNVEnums.cend());
    std::set<VkOpticalFlowSessionBindingPointNV> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkOpticalFlowSessionBindingPointNVEnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}


template<>
std::vector<VkShaderGroupShaderKHR> ValidationObject::ValidParamValues() const {
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array CoreVkShaderGroupShaderKHREnums = { VK_SHADER_GROUP_SHADER_GENERAL_KHR, VK_SHADER_GROUP_SHADER_CLOSEST_HIT_KHR, VK_SHADER_GROUP_SHADER_ANY_HIT_KHR, VK_SHADER_GROUP_SHADER_INTERSECTION_KHR,  };
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<VkShaderGroupShaderKHR>> ExtendedVkShaderGroupShaderKHREnums = {
    };
    std::vector<VkShaderGroupShaderKHR> values(CoreVkShaderGroupShaderKHREnums.cbegin(), CoreVkShaderGroupShaderKHREnums.cend());
    std::set<VkShaderGroupShaderKHR> unique_exts;
    for (const auto& [extension, enums]: ExtendedVkShaderGroupShaderKHREnums) {
        if (IsExtEnabled(device_extensions.*extension)) {
            unique_exts.insert(enums.cbegin(), enums.cend());
        }
    }
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}

