// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See valid_enum_values_generator.py for modifications

/***************************************************************************
 *
 * Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
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
 ****************************************************************************/

// NOLINTBEGIN

#include "stateless/stateless_validation.h"

//  Checking for values is a 2 part process
//    1. Check if is valid at all
//    2. If invalid, spend more time to figure out how and what info to report to the user
//
//  While this might not seem ideal to compute the enabled extensions every time this function is called, the
//  other solution would be to build a list at vkCreateDevice time of all the valid values. This adds much higher
//  memory overhead.
//
//  Another key point to consider is being able to tell the user a value is invalid because it "doesn't exist" vs
//  "forgot to enable an extension" is VERY important

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkPipelineCacheHeaderVersion value) const {
    switch (value) {
        case VK_PIPELINE_CACHE_HEADER_VERSION_ONE:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkImageLayout value) const {
    switch (value) {
        case VK_IMAGE_LAYOUT_UNDEFINED:
        case VK_IMAGE_LAYOUT_GENERAL:
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        case VK_IMAGE_LAYOUT_PREINITIALIZED:
            return ValidValue::Valid;
        case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
        case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
            return IsExtEnabled(device_extensions.vk_khr_maintenance2) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
        case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL:
        case VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL:
        case VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL:
            return IsExtEnabled(device_extensions.vk_khr_separate_depth_stencil_layouts) ? ValidValue::Valid
                                                                                         : ValidValue::NoExtension;
        case VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL:
        case VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL:
            return IsExtEnabled(device_extensions.vk_khr_synchronization2) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
            return IsExtEnabled(device_extensions.vk_khr_swapchain) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR:
        case VK_IMAGE_LAYOUT_VIDEO_DECODE_SRC_KHR:
        case VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR:
            return IsExtEnabled(device_extensions.vk_khr_video_decode_queue) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR:
            return IsExtEnabled(device_extensions.vk_khr_shared_presentable_image) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT:
            return IsExtEnabled(device_extensions.vk_ext_fragment_density_map) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR:
            return IsExtEnabled(device_extensions.vk_khr_fragment_shading_rate) ||
                           IsExtEnabled(device_extensions.vk_nv_shading_rate_image)
                       ? ValidValue::Valid
                       : ValidValue::NoExtension;
        case VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ_KHR:
            return IsExtEnabled(device_extensions.vk_khr_dynamic_rendering_local_read) ? ValidValue::Valid
                                                                                       : ValidValue::NoExtension;
        case VK_IMAGE_LAYOUT_VIDEO_ENCODE_DST_KHR:
        case VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR:
        case VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR:
            return IsExtEnabled(device_extensions.vk_khr_video_encode_queue) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT:
            return IsExtEnabled(device_extensions.vk_ext_attachment_feedback_loop_layout) ? ValidValue::Valid
                                                                                          : ValidValue::NoExtension;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkObjectType value) const {
    switch (value) {
        case VK_OBJECT_TYPE_UNKNOWN:
        case VK_OBJECT_TYPE_INSTANCE:
        case VK_OBJECT_TYPE_PHYSICAL_DEVICE:
        case VK_OBJECT_TYPE_DEVICE:
        case VK_OBJECT_TYPE_QUEUE:
        case VK_OBJECT_TYPE_SEMAPHORE:
        case VK_OBJECT_TYPE_COMMAND_BUFFER:
        case VK_OBJECT_TYPE_FENCE:
        case VK_OBJECT_TYPE_DEVICE_MEMORY:
        case VK_OBJECT_TYPE_BUFFER:
        case VK_OBJECT_TYPE_IMAGE:
        case VK_OBJECT_TYPE_EVENT:
        case VK_OBJECT_TYPE_QUERY_POOL:
        case VK_OBJECT_TYPE_BUFFER_VIEW:
        case VK_OBJECT_TYPE_IMAGE_VIEW:
        case VK_OBJECT_TYPE_SHADER_MODULE:
        case VK_OBJECT_TYPE_PIPELINE_CACHE:
        case VK_OBJECT_TYPE_PIPELINE_LAYOUT:
        case VK_OBJECT_TYPE_RENDER_PASS:
        case VK_OBJECT_TYPE_PIPELINE:
        case VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT:
        case VK_OBJECT_TYPE_SAMPLER:
        case VK_OBJECT_TYPE_DESCRIPTOR_POOL:
        case VK_OBJECT_TYPE_DESCRIPTOR_SET:
        case VK_OBJECT_TYPE_FRAMEBUFFER:
        case VK_OBJECT_TYPE_COMMAND_POOL:
            return ValidValue::Valid;
        case VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION:
            return IsExtEnabled(device_extensions.vk_khr_sampler_ycbcr_conversion) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE:
            return IsExtEnabled(device_extensions.vk_khr_descriptor_update_template) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_OBJECT_TYPE_PRIVATE_DATA_SLOT:
            return IsExtEnabled(device_extensions.vk_ext_private_data) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_OBJECT_TYPE_SURFACE_KHR:
            return IsExtEnabled(device_extensions.vk_khr_surface) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_OBJECT_TYPE_SWAPCHAIN_KHR:
            return IsExtEnabled(device_extensions.vk_khr_swapchain) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_OBJECT_TYPE_DISPLAY_KHR:
        case VK_OBJECT_TYPE_DISPLAY_MODE_KHR:
            return IsExtEnabled(device_extensions.vk_khr_display) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT:
            return IsExtEnabled(device_extensions.vk_ext_debug_report) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_OBJECT_TYPE_VIDEO_SESSION_KHR:
        case VK_OBJECT_TYPE_VIDEO_SESSION_PARAMETERS_KHR:
            return IsExtEnabled(device_extensions.vk_khr_video_queue) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_OBJECT_TYPE_CU_MODULE_NVX:
        case VK_OBJECT_TYPE_CU_FUNCTION_NVX:
            return IsExtEnabled(device_extensions.vk_nvx_binary_import) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT:
            return IsExtEnabled(device_extensions.vk_ext_debug_utils) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR:
            return IsExtEnabled(device_extensions.vk_khr_acceleration_structure) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_OBJECT_TYPE_VALIDATION_CACHE_EXT:
            return IsExtEnabled(device_extensions.vk_ext_validation_cache) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV:
            return IsExtEnabled(device_extensions.vk_nv_ray_tracing) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_OBJECT_TYPE_PERFORMANCE_CONFIGURATION_INTEL:
            return IsExtEnabled(device_extensions.vk_intel_performance_query) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_OBJECT_TYPE_DEFERRED_OPERATION_KHR:
            return IsExtEnabled(device_extensions.vk_khr_deferred_host_operations) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NV:
            return IsExtEnabled(device_extensions.vk_nv_device_generated_commands) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_OBJECT_TYPE_CUDA_MODULE_NV:
        case VK_OBJECT_TYPE_CUDA_FUNCTION_NV:
            return IsExtEnabled(device_extensions.vk_nv_cuda_kernel_launch) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_OBJECT_TYPE_BUFFER_COLLECTION_FUCHSIA:
            return IsExtEnabled(device_extensions.vk_fuchsia_buffer_collection) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_OBJECT_TYPE_MICROMAP_EXT:
            return IsExtEnabled(device_extensions.vk_ext_opacity_micromap) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_OBJECT_TYPE_OPTICAL_FLOW_SESSION_NV:
            return IsExtEnabled(device_extensions.vk_nv_optical_flow) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_OBJECT_TYPE_SHADER_EXT:
            return IsExtEnabled(device_extensions.vk_ext_shader_object) ? ValidValue::Valid : ValidValue::NoExtension;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkFormat value) const {
    switch (value) {
        case VK_FORMAT_UNDEFINED:
        case VK_FORMAT_R4G4_UNORM_PACK8:
        case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
        case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
        case VK_FORMAT_R5G6B5_UNORM_PACK16:
        case VK_FORMAT_B5G6R5_UNORM_PACK16:
        case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
        case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
        case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
        case VK_FORMAT_R8_UNORM:
        case VK_FORMAT_R8_SNORM:
        case VK_FORMAT_R8_USCALED:
        case VK_FORMAT_R8_SSCALED:
        case VK_FORMAT_R8_UINT:
        case VK_FORMAT_R8_SINT:
        case VK_FORMAT_R8_SRGB:
        case VK_FORMAT_R8G8_UNORM:
        case VK_FORMAT_R8G8_SNORM:
        case VK_FORMAT_R8G8_USCALED:
        case VK_FORMAT_R8G8_SSCALED:
        case VK_FORMAT_R8G8_UINT:
        case VK_FORMAT_R8G8_SINT:
        case VK_FORMAT_R8G8_SRGB:
        case VK_FORMAT_R8G8B8_UNORM:
        case VK_FORMAT_R8G8B8_SNORM:
        case VK_FORMAT_R8G8B8_USCALED:
        case VK_FORMAT_R8G8B8_SSCALED:
        case VK_FORMAT_R8G8B8_UINT:
        case VK_FORMAT_R8G8B8_SINT:
        case VK_FORMAT_R8G8B8_SRGB:
        case VK_FORMAT_B8G8R8_UNORM:
        case VK_FORMAT_B8G8R8_SNORM:
        case VK_FORMAT_B8G8R8_USCALED:
        case VK_FORMAT_B8G8R8_SSCALED:
        case VK_FORMAT_B8G8R8_UINT:
        case VK_FORMAT_B8G8R8_SINT:
        case VK_FORMAT_B8G8R8_SRGB:
        case VK_FORMAT_R8G8B8A8_UNORM:
        case VK_FORMAT_R8G8B8A8_SNORM:
        case VK_FORMAT_R8G8B8A8_USCALED:
        case VK_FORMAT_R8G8B8A8_SSCALED:
        case VK_FORMAT_R8G8B8A8_UINT:
        case VK_FORMAT_R8G8B8A8_SINT:
        case VK_FORMAT_R8G8B8A8_SRGB:
        case VK_FORMAT_B8G8R8A8_UNORM:
        case VK_FORMAT_B8G8R8A8_SNORM:
        case VK_FORMAT_B8G8R8A8_USCALED:
        case VK_FORMAT_B8G8R8A8_SSCALED:
        case VK_FORMAT_B8G8R8A8_UINT:
        case VK_FORMAT_B8G8R8A8_SINT:
        case VK_FORMAT_B8G8R8A8_SRGB:
        case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
        case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
        case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
        case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
        case VK_FORMAT_A8B8G8R8_UINT_PACK32:
        case VK_FORMAT_A8B8G8R8_SINT_PACK32:
        case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
        case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
        case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
        case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
        case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
        case VK_FORMAT_A2R10G10B10_UINT_PACK32:
        case VK_FORMAT_A2R10G10B10_SINT_PACK32:
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
        case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
        case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
        case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
        case VK_FORMAT_A2B10G10R10_UINT_PACK32:
        case VK_FORMAT_A2B10G10R10_SINT_PACK32:
        case VK_FORMAT_R16_UNORM:
        case VK_FORMAT_R16_SNORM:
        case VK_FORMAT_R16_USCALED:
        case VK_FORMAT_R16_SSCALED:
        case VK_FORMAT_R16_UINT:
        case VK_FORMAT_R16_SINT:
        case VK_FORMAT_R16_SFLOAT:
        case VK_FORMAT_R16G16_UNORM:
        case VK_FORMAT_R16G16_SNORM:
        case VK_FORMAT_R16G16_USCALED:
        case VK_FORMAT_R16G16_SSCALED:
        case VK_FORMAT_R16G16_UINT:
        case VK_FORMAT_R16G16_SINT:
        case VK_FORMAT_R16G16_SFLOAT:
        case VK_FORMAT_R16G16B16_UNORM:
        case VK_FORMAT_R16G16B16_SNORM:
        case VK_FORMAT_R16G16B16_USCALED:
        case VK_FORMAT_R16G16B16_SSCALED:
        case VK_FORMAT_R16G16B16_UINT:
        case VK_FORMAT_R16G16B16_SINT:
        case VK_FORMAT_R16G16B16_SFLOAT:
        case VK_FORMAT_R16G16B16A16_UNORM:
        case VK_FORMAT_R16G16B16A16_SNORM:
        case VK_FORMAT_R16G16B16A16_USCALED:
        case VK_FORMAT_R16G16B16A16_SSCALED:
        case VK_FORMAT_R16G16B16A16_UINT:
        case VK_FORMAT_R16G16B16A16_SINT:
        case VK_FORMAT_R16G16B16A16_SFLOAT:
        case VK_FORMAT_R32_UINT:
        case VK_FORMAT_R32_SINT:
        case VK_FORMAT_R32_SFLOAT:
        case VK_FORMAT_R32G32_UINT:
        case VK_FORMAT_R32G32_SINT:
        case VK_FORMAT_R32G32_SFLOAT:
        case VK_FORMAT_R32G32B32_UINT:
        case VK_FORMAT_R32G32B32_SINT:
        case VK_FORMAT_R32G32B32_SFLOAT:
        case VK_FORMAT_R32G32B32A32_UINT:
        case VK_FORMAT_R32G32B32A32_SINT:
        case VK_FORMAT_R32G32B32A32_SFLOAT:
        case VK_FORMAT_R64_UINT:
        case VK_FORMAT_R64_SINT:
        case VK_FORMAT_R64_SFLOAT:
        case VK_FORMAT_R64G64_UINT:
        case VK_FORMAT_R64G64_SINT:
        case VK_FORMAT_R64G64_SFLOAT:
        case VK_FORMAT_R64G64B64_UINT:
        case VK_FORMAT_R64G64B64_SINT:
        case VK_FORMAT_R64G64B64_SFLOAT:
        case VK_FORMAT_R64G64B64A64_UINT:
        case VK_FORMAT_R64G64B64A64_SINT:
        case VK_FORMAT_R64G64B64A64_SFLOAT:
        case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
        case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:
        case VK_FORMAT_D16_UNORM:
        case VK_FORMAT_X8_D24_UNORM_PACK32:
        case VK_FORMAT_D32_SFLOAT:
        case VK_FORMAT_S8_UINT:
        case VK_FORMAT_D16_UNORM_S8_UINT:
        case VK_FORMAT_D24_UNORM_S8_UINT:
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
        case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
        case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
        case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
        case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
        case VK_FORMAT_BC2_UNORM_BLOCK:
        case VK_FORMAT_BC2_SRGB_BLOCK:
        case VK_FORMAT_BC3_UNORM_BLOCK:
        case VK_FORMAT_BC3_SRGB_BLOCK:
        case VK_FORMAT_BC4_UNORM_BLOCK:
        case VK_FORMAT_BC4_SNORM_BLOCK:
        case VK_FORMAT_BC5_UNORM_BLOCK:
        case VK_FORMAT_BC5_SNORM_BLOCK:
        case VK_FORMAT_BC6H_UFLOAT_BLOCK:
        case VK_FORMAT_BC6H_SFLOAT_BLOCK:
        case VK_FORMAT_BC7_UNORM_BLOCK:
        case VK_FORMAT_BC7_SRGB_BLOCK:
        case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
        case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK:
        case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK:
        case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK:
        case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK:
        case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK:
        case VK_FORMAT_EAC_R11_UNORM_BLOCK:
        case VK_FORMAT_EAC_R11_SNORM_BLOCK:
        case VK_FORMAT_EAC_R11G11_UNORM_BLOCK:
        case VK_FORMAT_EAC_R11G11_SNORM_BLOCK:
        case VK_FORMAT_ASTC_4x4_UNORM_BLOCK:
        case VK_FORMAT_ASTC_4x4_SRGB_BLOCK:
        case VK_FORMAT_ASTC_5x4_UNORM_BLOCK:
        case VK_FORMAT_ASTC_5x4_SRGB_BLOCK:
        case VK_FORMAT_ASTC_5x5_UNORM_BLOCK:
        case VK_FORMAT_ASTC_5x5_SRGB_BLOCK:
        case VK_FORMAT_ASTC_6x5_UNORM_BLOCK:
        case VK_FORMAT_ASTC_6x5_SRGB_BLOCK:
        case VK_FORMAT_ASTC_6x6_UNORM_BLOCK:
        case VK_FORMAT_ASTC_6x6_SRGB_BLOCK:
        case VK_FORMAT_ASTC_8x5_UNORM_BLOCK:
        case VK_FORMAT_ASTC_8x5_SRGB_BLOCK:
        case VK_FORMAT_ASTC_8x6_UNORM_BLOCK:
        case VK_FORMAT_ASTC_8x6_SRGB_BLOCK:
        case VK_FORMAT_ASTC_8x8_UNORM_BLOCK:
        case VK_FORMAT_ASTC_8x8_SRGB_BLOCK:
        case VK_FORMAT_ASTC_10x5_UNORM_BLOCK:
        case VK_FORMAT_ASTC_10x5_SRGB_BLOCK:
        case VK_FORMAT_ASTC_10x6_UNORM_BLOCK:
        case VK_FORMAT_ASTC_10x6_SRGB_BLOCK:
        case VK_FORMAT_ASTC_10x8_UNORM_BLOCK:
        case VK_FORMAT_ASTC_10x8_SRGB_BLOCK:
        case VK_FORMAT_ASTC_10x10_UNORM_BLOCK:
        case VK_FORMAT_ASTC_10x10_SRGB_BLOCK:
        case VK_FORMAT_ASTC_12x10_UNORM_BLOCK:
        case VK_FORMAT_ASTC_12x10_SRGB_BLOCK:
        case VK_FORMAT_ASTC_12x12_UNORM_BLOCK:
        case VK_FORMAT_ASTC_12x12_SRGB_BLOCK:
            return ValidValue::Valid;
        case VK_FORMAT_G8B8G8R8_422_UNORM:
        case VK_FORMAT_B8G8R8G8_422_UNORM:
        case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM:
        case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM:
        case VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM:
        case VK_FORMAT_G8_B8R8_2PLANE_422_UNORM:
        case VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM:
        case VK_FORMAT_R10X6_UNORM_PACK16:
        case VK_FORMAT_R10X6G10X6_UNORM_2PACK16:
        case VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16:
        case VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16:
        case VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16:
        case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16:
        case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16:
        case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16:
        case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16:
        case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16:
        case VK_FORMAT_R12X4_UNORM_PACK16:
        case VK_FORMAT_R12X4G12X4_UNORM_2PACK16:
        case VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16:
        case VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16:
        case VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16:
        case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16:
        case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16:
        case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16:
        case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16:
        case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16:
        case VK_FORMAT_G16B16G16R16_422_UNORM:
        case VK_FORMAT_B16G16R16G16_422_UNORM:
        case VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM:
        case VK_FORMAT_G16_B16R16_2PLANE_420_UNORM:
        case VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM:
        case VK_FORMAT_G16_B16R16_2PLANE_422_UNORM:
        case VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM:
            return IsExtEnabled(device_extensions.vk_khr_sampler_ycbcr_conversion) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_FORMAT_G8_B8R8_2PLANE_444_UNORM:
        case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16:
        case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16:
        case VK_FORMAT_G16_B16R16_2PLANE_444_UNORM:
            return IsExtEnabled(device_extensions.vk_ext_ycbcr_2plane_444_formats) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_FORMAT_A4R4G4B4_UNORM_PACK16:
        case VK_FORMAT_A4B4G4R4_UNORM_PACK16:
            return IsExtEnabled(device_extensions.vk_ext_4444_formats) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK:
            return IsExtEnabled(device_extensions.vk_ext_texture_compression_astc_hdr) ? ValidValue::Valid
                                                                                       : ValidValue::NoExtension;
        case VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG:
        case VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG:
        case VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG:
        case VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG:
        case VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG:
        case VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG:
        case VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG:
        case VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG:
            return IsExtEnabled(device_extensions.vk_img_format_pvrtc) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_FORMAT_R16G16_S10_5_NV:
            return IsExtEnabled(device_extensions.vk_nv_optical_flow) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_FORMAT_A1B5G5R5_UNORM_PACK16_KHR:
        case VK_FORMAT_A8_UNORM_KHR:
            return IsExtEnabled(device_extensions.vk_khr_maintenance5) ? ValidValue::Valid : ValidValue::NoExtension;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkImageTiling value) const {
    switch (value) {
        case VK_IMAGE_TILING_OPTIMAL:
        case VK_IMAGE_TILING_LINEAR:
            return ValidValue::Valid;
        case VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT:
            return IsExtEnabled(device_extensions.vk_ext_image_drm_format_modifier) ? ValidValue::Valid : ValidValue::NoExtension;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkImageType value) const {
    switch (value) {
        case VK_IMAGE_TYPE_1D:
        case VK_IMAGE_TYPE_2D:
        case VK_IMAGE_TYPE_3D:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkQueryType value) const {
    switch (value) {
        case VK_QUERY_TYPE_OCCLUSION:
        case VK_QUERY_TYPE_PIPELINE_STATISTICS:
        case VK_QUERY_TYPE_TIMESTAMP:
            return ValidValue::Valid;
        case VK_QUERY_TYPE_RESULT_STATUS_ONLY_KHR:
            return IsExtEnabled(device_extensions.vk_khr_video_queue) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT:
            return IsExtEnabled(device_extensions.vk_ext_transform_feedback) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR:
            return IsExtEnabled(device_extensions.vk_khr_performance_query) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR:
        case VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR:
            return IsExtEnabled(device_extensions.vk_khr_acceleration_structure) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_NV:
            return IsExtEnabled(device_extensions.vk_nv_ray_tracing) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_QUERY_TYPE_PERFORMANCE_QUERY_INTEL:
            return IsExtEnabled(device_extensions.vk_intel_performance_query) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_QUERY_TYPE_VIDEO_ENCODE_FEEDBACK_KHR:
            return IsExtEnabled(device_extensions.vk_khr_video_encode_queue) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_QUERY_TYPE_MESH_PRIMITIVES_GENERATED_EXT:
            return IsExtEnabled(device_extensions.vk_ext_mesh_shader) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT:
            return IsExtEnabled(device_extensions.vk_ext_primitives_generated_query) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_BOTTOM_LEVEL_POINTERS_KHR:
        case VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SIZE_KHR:
            return IsExtEnabled(device_extensions.vk_khr_ray_tracing_maintenance1) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_QUERY_TYPE_MICROMAP_SERIALIZATION_SIZE_EXT:
        case VK_QUERY_TYPE_MICROMAP_COMPACTED_SIZE_EXT:
            return IsExtEnabled(device_extensions.vk_ext_opacity_micromap) ? ValidValue::Valid : ValidValue::NoExtension;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkSharingMode value) const {
    switch (value) {
        case VK_SHARING_MODE_EXCLUSIVE:
        case VK_SHARING_MODE_CONCURRENT:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkComponentSwizzle value) const {
    switch (value) {
        case VK_COMPONENT_SWIZZLE_IDENTITY:
        case VK_COMPONENT_SWIZZLE_ZERO:
        case VK_COMPONENT_SWIZZLE_ONE:
        case VK_COMPONENT_SWIZZLE_R:
        case VK_COMPONENT_SWIZZLE_G:
        case VK_COMPONENT_SWIZZLE_B:
        case VK_COMPONENT_SWIZZLE_A:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkImageViewType value) const {
    switch (value) {
        case VK_IMAGE_VIEW_TYPE_1D:
        case VK_IMAGE_VIEW_TYPE_2D:
        case VK_IMAGE_VIEW_TYPE_3D:
        case VK_IMAGE_VIEW_TYPE_CUBE:
        case VK_IMAGE_VIEW_TYPE_1D_ARRAY:
        case VK_IMAGE_VIEW_TYPE_2D_ARRAY:
        case VK_IMAGE_VIEW_TYPE_CUBE_ARRAY:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkBlendFactor value) const {
    switch (value) {
        case VK_BLEND_FACTOR_ZERO:
        case VK_BLEND_FACTOR_ONE:
        case VK_BLEND_FACTOR_SRC_COLOR:
        case VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR:
        case VK_BLEND_FACTOR_DST_COLOR:
        case VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR:
        case VK_BLEND_FACTOR_SRC_ALPHA:
        case VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA:
        case VK_BLEND_FACTOR_DST_ALPHA:
        case VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA:
        case VK_BLEND_FACTOR_CONSTANT_COLOR:
        case VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR:
        case VK_BLEND_FACTOR_CONSTANT_ALPHA:
        case VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA:
        case VK_BLEND_FACTOR_SRC_ALPHA_SATURATE:
        case VK_BLEND_FACTOR_SRC1_COLOR:
        case VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR:
        case VK_BLEND_FACTOR_SRC1_ALPHA:
        case VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkBlendOp value) const {
    switch (value) {
        case VK_BLEND_OP_ADD:
        case VK_BLEND_OP_SUBTRACT:
        case VK_BLEND_OP_REVERSE_SUBTRACT:
        case VK_BLEND_OP_MIN:
        case VK_BLEND_OP_MAX:
            return ValidValue::Valid;
        case VK_BLEND_OP_ZERO_EXT:
        case VK_BLEND_OP_SRC_EXT:
        case VK_BLEND_OP_DST_EXT:
        case VK_BLEND_OP_SRC_OVER_EXT:
        case VK_BLEND_OP_DST_OVER_EXT:
        case VK_BLEND_OP_SRC_IN_EXT:
        case VK_BLEND_OP_DST_IN_EXT:
        case VK_BLEND_OP_SRC_OUT_EXT:
        case VK_BLEND_OP_DST_OUT_EXT:
        case VK_BLEND_OP_SRC_ATOP_EXT:
        case VK_BLEND_OP_DST_ATOP_EXT:
        case VK_BLEND_OP_XOR_EXT:
        case VK_BLEND_OP_MULTIPLY_EXT:
        case VK_BLEND_OP_SCREEN_EXT:
        case VK_BLEND_OP_OVERLAY_EXT:
        case VK_BLEND_OP_DARKEN_EXT:
        case VK_BLEND_OP_LIGHTEN_EXT:
        case VK_BLEND_OP_COLORDODGE_EXT:
        case VK_BLEND_OP_COLORBURN_EXT:
        case VK_BLEND_OP_HARDLIGHT_EXT:
        case VK_BLEND_OP_SOFTLIGHT_EXT:
        case VK_BLEND_OP_DIFFERENCE_EXT:
        case VK_BLEND_OP_EXCLUSION_EXT:
        case VK_BLEND_OP_INVERT_EXT:
        case VK_BLEND_OP_INVERT_RGB_EXT:
        case VK_BLEND_OP_LINEARDODGE_EXT:
        case VK_BLEND_OP_LINEARBURN_EXT:
        case VK_BLEND_OP_VIVIDLIGHT_EXT:
        case VK_BLEND_OP_LINEARLIGHT_EXT:
        case VK_BLEND_OP_PINLIGHT_EXT:
        case VK_BLEND_OP_HARDMIX_EXT:
        case VK_BLEND_OP_HSL_HUE_EXT:
        case VK_BLEND_OP_HSL_SATURATION_EXT:
        case VK_BLEND_OP_HSL_COLOR_EXT:
        case VK_BLEND_OP_HSL_LUMINOSITY_EXT:
        case VK_BLEND_OP_PLUS_EXT:
        case VK_BLEND_OP_PLUS_CLAMPED_EXT:
        case VK_BLEND_OP_PLUS_CLAMPED_ALPHA_EXT:
        case VK_BLEND_OP_PLUS_DARKER_EXT:
        case VK_BLEND_OP_MINUS_EXT:
        case VK_BLEND_OP_MINUS_CLAMPED_EXT:
        case VK_BLEND_OP_CONTRAST_EXT:
        case VK_BLEND_OP_INVERT_OVG_EXT:
        case VK_BLEND_OP_RED_EXT:
        case VK_BLEND_OP_GREEN_EXT:
        case VK_BLEND_OP_BLUE_EXT:
            return IsExtEnabled(device_extensions.vk_ext_blend_operation_advanced) ? ValidValue::Valid : ValidValue::NoExtension;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkCompareOp value) const {
    switch (value) {
        case VK_COMPARE_OP_NEVER:
        case VK_COMPARE_OP_LESS:
        case VK_COMPARE_OP_EQUAL:
        case VK_COMPARE_OP_LESS_OR_EQUAL:
        case VK_COMPARE_OP_GREATER:
        case VK_COMPARE_OP_NOT_EQUAL:
        case VK_COMPARE_OP_GREATER_OR_EQUAL:
        case VK_COMPARE_OP_ALWAYS:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkDynamicState value) const {
    switch (value) {
        case VK_DYNAMIC_STATE_VIEWPORT:
        case VK_DYNAMIC_STATE_SCISSOR:
        case VK_DYNAMIC_STATE_LINE_WIDTH:
        case VK_DYNAMIC_STATE_DEPTH_BIAS:
        case VK_DYNAMIC_STATE_BLEND_CONSTANTS:
        case VK_DYNAMIC_STATE_DEPTH_BOUNDS:
        case VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK:
        case VK_DYNAMIC_STATE_STENCIL_WRITE_MASK:
        case VK_DYNAMIC_STATE_STENCIL_REFERENCE:
            return ValidValue::Valid;
        case VK_DYNAMIC_STATE_CULL_MODE:
        case VK_DYNAMIC_STATE_FRONT_FACE:
        case VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY:
        case VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT:
        case VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT:
        case VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE:
        case VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE:
        case VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE:
        case VK_DYNAMIC_STATE_DEPTH_COMPARE_OP:
        case VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE:
        case VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE:
        case VK_DYNAMIC_STATE_STENCIL_OP:
            return IsExtEnabled(device_extensions.vk_ext_extended_dynamic_state) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE:
        case VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE:
        case VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE:
        case VK_DYNAMIC_STATE_PATCH_CONTROL_POINTS_EXT:
        case VK_DYNAMIC_STATE_LOGIC_OP_EXT:
            return IsExtEnabled(device_extensions.vk_ext_extended_dynamic_state2) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_NV:
            return IsExtEnabled(device_extensions.vk_nv_clip_space_w_scaling) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_DYNAMIC_STATE_DISCARD_RECTANGLE_EXT:
        case VK_DYNAMIC_STATE_DISCARD_RECTANGLE_ENABLE_EXT:
        case VK_DYNAMIC_STATE_DISCARD_RECTANGLE_MODE_EXT:
            return IsExtEnabled(device_extensions.vk_ext_discard_rectangles) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT:
            return IsExtEnabled(device_extensions.vk_ext_sample_locations) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_DYNAMIC_STATE_RAY_TRACING_PIPELINE_STACK_SIZE_KHR:
            return IsExtEnabled(device_extensions.vk_khr_ray_tracing_pipeline) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_DYNAMIC_STATE_VIEWPORT_SHADING_RATE_PALETTE_NV:
        case VK_DYNAMIC_STATE_VIEWPORT_COARSE_SAMPLE_ORDER_NV:
            return IsExtEnabled(device_extensions.vk_nv_shading_rate_image) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_ENABLE_NV:
        case VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_NV:
            return IsExtEnabled(device_extensions.vk_nv_scissor_exclusive) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_DYNAMIC_STATE_FRAGMENT_SHADING_RATE_KHR:
            return IsExtEnabled(device_extensions.vk_khr_fragment_shading_rate) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_DYNAMIC_STATE_VERTEX_INPUT_EXT:
            return IsExtEnabled(device_extensions.vk_ext_vertex_input_dynamic_state) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_DYNAMIC_STATE_COLOR_WRITE_ENABLE_EXT:
            return IsExtEnabled(device_extensions.vk_ext_color_write_enable) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_DYNAMIC_STATE_DEPTH_CLAMP_ENABLE_EXT:
        case VK_DYNAMIC_STATE_POLYGON_MODE_EXT:
        case VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT:
        case VK_DYNAMIC_STATE_SAMPLE_MASK_EXT:
        case VK_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT:
        case VK_DYNAMIC_STATE_ALPHA_TO_ONE_ENABLE_EXT:
        case VK_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT:
        case VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT:
        case VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT:
        case VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT:
        case VK_DYNAMIC_STATE_TESSELLATION_DOMAIN_ORIGIN_EXT:
        case VK_DYNAMIC_STATE_RASTERIZATION_STREAM_EXT:
        case VK_DYNAMIC_STATE_CONSERVATIVE_RASTERIZATION_MODE_EXT:
        case VK_DYNAMIC_STATE_EXTRA_PRIMITIVE_OVERESTIMATION_SIZE_EXT:
        case VK_DYNAMIC_STATE_DEPTH_CLIP_ENABLE_EXT:
        case VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_ENABLE_EXT:
        case VK_DYNAMIC_STATE_COLOR_BLEND_ADVANCED_EXT:
        case VK_DYNAMIC_STATE_PROVOKING_VERTEX_MODE_EXT:
        case VK_DYNAMIC_STATE_LINE_RASTERIZATION_MODE_EXT:
        case VK_DYNAMIC_STATE_LINE_STIPPLE_ENABLE_EXT:
        case VK_DYNAMIC_STATE_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE_EXT:
        case VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_ENABLE_NV:
        case VK_DYNAMIC_STATE_VIEWPORT_SWIZZLE_NV:
        case VK_DYNAMIC_STATE_COVERAGE_TO_COLOR_ENABLE_NV:
        case VK_DYNAMIC_STATE_COVERAGE_TO_COLOR_LOCATION_NV:
        case VK_DYNAMIC_STATE_COVERAGE_MODULATION_MODE_NV:
        case VK_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_ENABLE_NV:
        case VK_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_NV:
        case VK_DYNAMIC_STATE_SHADING_RATE_IMAGE_ENABLE_NV:
        case VK_DYNAMIC_STATE_REPRESENTATIVE_FRAGMENT_TEST_ENABLE_NV:
        case VK_DYNAMIC_STATE_COVERAGE_REDUCTION_MODE_NV:
            return IsExtEnabled(device_extensions.vk_ext_extended_dynamic_state3) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_DYNAMIC_STATE_ATTACHMENT_FEEDBACK_LOOP_ENABLE_EXT:
            return IsExtEnabled(device_extensions.vk_ext_attachment_feedback_loop_dynamic_state) ? ValidValue::Valid
                                                                                                 : ValidValue::NoExtension;
        case VK_DYNAMIC_STATE_LINE_STIPPLE_KHR:
            return IsExtEnabled(device_extensions.vk_khr_line_rasterization) ||
                           IsExtEnabled(device_extensions.vk_ext_line_rasterization)
                       ? ValidValue::Valid
                       : ValidValue::NoExtension;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkFrontFace value) const {
    switch (value) {
        case VK_FRONT_FACE_COUNTER_CLOCKWISE:
        case VK_FRONT_FACE_CLOCKWISE:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkVertexInputRate value) const {
    switch (value) {
        case VK_VERTEX_INPUT_RATE_VERTEX:
        case VK_VERTEX_INPUT_RATE_INSTANCE:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkPrimitiveTopology value) const {
    switch (value) {
        case VK_PRIMITIVE_TOPOLOGY_POINT_LIST:
        case VK_PRIMITIVE_TOPOLOGY_LINE_LIST:
        case VK_PRIMITIVE_TOPOLOGY_LINE_STRIP:
        case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST:
        case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP:
        case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN:
        case VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY:
        case VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY:
        case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY:
        case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY:
        case VK_PRIMITIVE_TOPOLOGY_PATCH_LIST:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkPolygonMode value) const {
    switch (value) {
        case VK_POLYGON_MODE_FILL:
        case VK_POLYGON_MODE_LINE:
        case VK_POLYGON_MODE_POINT:
            return ValidValue::Valid;
        case VK_POLYGON_MODE_FILL_RECTANGLE_NV:
            return IsExtEnabled(device_extensions.vk_nv_fill_rectangle) ? ValidValue::Valid : ValidValue::NoExtension;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkStencilOp value) const {
    switch (value) {
        case VK_STENCIL_OP_KEEP:
        case VK_STENCIL_OP_ZERO:
        case VK_STENCIL_OP_REPLACE:
        case VK_STENCIL_OP_INCREMENT_AND_CLAMP:
        case VK_STENCIL_OP_DECREMENT_AND_CLAMP:
        case VK_STENCIL_OP_INVERT:
        case VK_STENCIL_OP_INCREMENT_AND_WRAP:
        case VK_STENCIL_OP_DECREMENT_AND_WRAP:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkLogicOp value) const {
    switch (value) {
        case VK_LOGIC_OP_CLEAR:
        case VK_LOGIC_OP_AND:
        case VK_LOGIC_OP_AND_REVERSE:
        case VK_LOGIC_OP_COPY:
        case VK_LOGIC_OP_AND_INVERTED:
        case VK_LOGIC_OP_NO_OP:
        case VK_LOGIC_OP_XOR:
        case VK_LOGIC_OP_OR:
        case VK_LOGIC_OP_NOR:
        case VK_LOGIC_OP_EQUIVALENT:
        case VK_LOGIC_OP_INVERT:
        case VK_LOGIC_OP_OR_REVERSE:
        case VK_LOGIC_OP_COPY_INVERTED:
        case VK_LOGIC_OP_OR_INVERTED:
        case VK_LOGIC_OP_NAND:
        case VK_LOGIC_OP_SET:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkBorderColor value) const {
    switch (value) {
        case VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK:
        case VK_BORDER_COLOR_INT_TRANSPARENT_BLACK:
        case VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK:
        case VK_BORDER_COLOR_INT_OPAQUE_BLACK:
        case VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE:
        case VK_BORDER_COLOR_INT_OPAQUE_WHITE:
            return ValidValue::Valid;
        case VK_BORDER_COLOR_FLOAT_CUSTOM_EXT:
        case VK_BORDER_COLOR_INT_CUSTOM_EXT:
            return IsExtEnabled(device_extensions.vk_ext_custom_border_color) ? ValidValue::Valid : ValidValue::NoExtension;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkFilter value) const {
    switch (value) {
        case VK_FILTER_NEAREST:
        case VK_FILTER_LINEAR:
            return ValidValue::Valid;
        case VK_FILTER_CUBIC_EXT:
            return IsExtEnabled(device_extensions.vk_img_filter_cubic) || IsExtEnabled(device_extensions.vk_ext_filter_cubic)
                       ? ValidValue::Valid
                       : ValidValue::NoExtension;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkSamplerAddressMode value) const {
    switch (value) {
        case VK_SAMPLER_ADDRESS_MODE_REPEAT:
        case VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT:
        case VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE:
        case VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER:
            return ValidValue::Valid;
        case VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE:
            return IsExtEnabled(device_extensions.vk_khr_sampler_mirror_clamp_to_edge) ? ValidValue::Valid
                                                                                       : ValidValue::NoExtension;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkSamplerMipmapMode value) const {
    switch (value) {
        case VK_SAMPLER_MIPMAP_MODE_NEAREST:
        case VK_SAMPLER_MIPMAP_MODE_LINEAR:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkDescriptorType value) const {
    switch (value) {
        case VK_DESCRIPTOR_TYPE_SAMPLER:
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
            return ValidValue::Valid;
        case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK:
            return IsExtEnabled(device_extensions.vk_ext_inline_uniform_block) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
            return IsExtEnabled(device_extensions.vk_khr_acceleration_structure) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV:
            return IsExtEnabled(device_extensions.vk_nv_ray_tracing) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_DESCRIPTOR_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM:
        case VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM:
            return IsExtEnabled(device_extensions.vk_qcom_image_processing) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_DESCRIPTOR_TYPE_MUTABLE_EXT:
            return IsExtEnabled(device_extensions.vk_valve_mutable_descriptor_type) ||
                           IsExtEnabled(device_extensions.vk_ext_mutable_descriptor_type)
                       ? ValidValue::Valid
                       : ValidValue::NoExtension;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkAttachmentLoadOp value) const {
    switch (value) {
        case VK_ATTACHMENT_LOAD_OP_LOAD:
        case VK_ATTACHMENT_LOAD_OP_CLEAR:
        case VK_ATTACHMENT_LOAD_OP_DONT_CARE:
            return ValidValue::Valid;
        case VK_ATTACHMENT_LOAD_OP_NONE_KHR:
            return IsExtEnabled(device_extensions.vk_khr_load_store_op_none) ||
                           IsExtEnabled(device_extensions.vk_ext_load_store_op_none)
                       ? ValidValue::Valid
                       : ValidValue::NoExtension;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkAttachmentStoreOp value) const {
    switch (value) {
        case VK_ATTACHMENT_STORE_OP_STORE:
        case VK_ATTACHMENT_STORE_OP_DONT_CARE:
            return ValidValue::Valid;
        case VK_ATTACHMENT_STORE_OP_NONE:
            return IsExtEnabled(device_extensions.vk_khr_dynamic_rendering) ||
                           IsExtEnabled(device_extensions.vk_khr_load_store_op_none) ||
                           IsExtEnabled(device_extensions.vk_qcom_render_pass_store_ops) ||
                           IsExtEnabled(device_extensions.vk_ext_load_store_op_none)
                       ? ValidValue::Valid
                       : ValidValue::NoExtension;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkPipelineBindPoint value) const {
    switch (value) {
        case VK_PIPELINE_BIND_POINT_GRAPHICS:
        case VK_PIPELINE_BIND_POINT_COMPUTE:
            return ValidValue::Valid;
        case VK_PIPELINE_BIND_POINT_EXECUTION_GRAPH_AMDX:
            return IsExtEnabled(device_extensions.vk_amdx_shader_enqueue) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR:
            return IsExtEnabled(device_extensions.vk_nv_ray_tracing) || IsExtEnabled(device_extensions.vk_khr_ray_tracing_pipeline)
                       ? ValidValue::Valid
                       : ValidValue::NoExtension;
        case VK_PIPELINE_BIND_POINT_SUBPASS_SHADING_HUAWEI:
            return IsExtEnabled(device_extensions.vk_huawei_subpass_shading) ? ValidValue::Valid : ValidValue::NoExtension;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkCommandBufferLevel value) const {
    switch (value) {
        case VK_COMMAND_BUFFER_LEVEL_PRIMARY:
        case VK_COMMAND_BUFFER_LEVEL_SECONDARY:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkIndexType value) const {
    switch (value) {
        case VK_INDEX_TYPE_UINT16:
        case VK_INDEX_TYPE_UINT32:
            return ValidValue::Valid;
        case VK_INDEX_TYPE_NONE_KHR:
            return IsExtEnabled(device_extensions.vk_nv_ray_tracing) ||
                           IsExtEnabled(device_extensions.vk_khr_acceleration_structure)
                       ? ValidValue::Valid
                       : ValidValue::NoExtension;
        case VK_INDEX_TYPE_UINT8_KHR:
            return IsExtEnabled(device_extensions.vk_khr_index_type_uint8) ||
                           IsExtEnabled(device_extensions.vk_ext_index_type_uint8)
                       ? ValidValue::Valid
                       : ValidValue::NoExtension;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkSubpassContents value) const {
    switch (value) {
        case VK_SUBPASS_CONTENTS_INLINE:
        case VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS:
            return ValidValue::Valid;
        case VK_SUBPASS_CONTENTS_INLINE_AND_SECONDARY_COMMAND_BUFFERS_EXT:
            return IsExtEnabled(device_extensions.vk_ext_nested_command_buffer) ? ValidValue::Valid : ValidValue::NoExtension;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkTessellationDomainOrigin value) const {
    switch (value) {
        case VK_TESSELLATION_DOMAIN_ORIGIN_UPPER_LEFT:
        case VK_TESSELLATION_DOMAIN_ORIGIN_LOWER_LEFT:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkSamplerYcbcrModelConversion value) const {
    switch (value) {
        case VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY:
        case VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_IDENTITY:
        case VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_709:
        case VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_601:
        case VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_2020:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkSamplerYcbcrRange value) const {
    switch (value) {
        case VK_SAMPLER_YCBCR_RANGE_ITU_FULL:
        case VK_SAMPLER_YCBCR_RANGE_ITU_NARROW:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkChromaLocation value) const {
    switch (value) {
        case VK_CHROMA_LOCATION_COSITED_EVEN:
        case VK_CHROMA_LOCATION_MIDPOINT:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkDescriptorUpdateTemplateType value) const {
    switch (value) {
        case VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET:
            return ValidValue::Valid;
        case VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_PUSH_DESCRIPTORS_KHR:
            return IsExtEnabled(device_extensions.vk_khr_push_descriptor) ? ValidValue::Valid : ValidValue::NoExtension;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkSamplerReductionMode value) const {
    switch (value) {
        case VK_SAMPLER_REDUCTION_MODE_WEIGHTED_AVERAGE:
        case VK_SAMPLER_REDUCTION_MODE_MIN:
        case VK_SAMPLER_REDUCTION_MODE_MAX:
            return ValidValue::Valid;
        case VK_SAMPLER_REDUCTION_MODE_WEIGHTED_AVERAGE_RANGECLAMP_QCOM:
            return IsExtEnabled(device_extensions.vk_qcom_filter_cubic_clamp) ? ValidValue::Valid : ValidValue::NoExtension;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkSemaphoreType value) const {
    switch (value) {
        case VK_SEMAPHORE_TYPE_BINARY:
        case VK_SEMAPHORE_TYPE_TIMELINE:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkPresentModeKHR value) const {
    switch (value) {
        case VK_PRESENT_MODE_IMMEDIATE_KHR:
        case VK_PRESENT_MODE_MAILBOX_KHR:
        case VK_PRESENT_MODE_FIFO_KHR:
        case VK_PRESENT_MODE_FIFO_RELAXED_KHR:
            return ValidValue::Valid;
        case VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR:
        case VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR:
            return IsExtEnabled(device_extensions.vk_khr_shared_presentable_image) ? ValidValue::Valid : ValidValue::NoExtension;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkColorSpaceKHR value) const {
    switch (value) {
        case VK_COLOR_SPACE_SRGB_NONLINEAR_KHR:
            return ValidValue::Valid;
        case VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT:
        case VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT:
        case VK_COLOR_SPACE_DISPLAY_P3_LINEAR_EXT:
        case VK_COLOR_SPACE_DCI_P3_NONLINEAR_EXT:
        case VK_COLOR_SPACE_BT709_LINEAR_EXT:
        case VK_COLOR_SPACE_BT709_NONLINEAR_EXT:
        case VK_COLOR_SPACE_BT2020_LINEAR_EXT:
        case VK_COLOR_SPACE_HDR10_ST2084_EXT:
        case VK_COLOR_SPACE_DOLBYVISION_EXT:
        case VK_COLOR_SPACE_HDR10_HLG_EXT:
        case VK_COLOR_SPACE_ADOBERGB_LINEAR_EXT:
        case VK_COLOR_SPACE_ADOBERGB_NONLINEAR_EXT:
        case VK_COLOR_SPACE_PASS_THROUGH_EXT:
        case VK_COLOR_SPACE_EXTENDED_SRGB_NONLINEAR_EXT:
            return IsExtEnabled(device_extensions.vk_ext_swapchain_colorspace) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_COLOR_SPACE_DISPLAY_NATIVE_AMD:
            return IsExtEnabled(device_extensions.vk_amd_display_native_hdr) ? ValidValue::Valid : ValidValue::NoExtension;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkQueueGlobalPriorityKHR value) const {
    switch (value) {
        case VK_QUEUE_GLOBAL_PRIORITY_LOW_KHR:
        case VK_QUEUE_GLOBAL_PRIORITY_MEDIUM_KHR:
        case VK_QUEUE_GLOBAL_PRIORITY_HIGH_KHR:
        case VK_QUEUE_GLOBAL_PRIORITY_REALTIME_KHR:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkFragmentShadingRateCombinerOpKHR value) const {
    switch (value) {
        case VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR:
        case VK_FRAGMENT_SHADING_RATE_COMBINER_OP_REPLACE_KHR:
        case VK_FRAGMENT_SHADING_RATE_COMBINER_OP_MIN_KHR:
        case VK_FRAGMENT_SHADING_RATE_COMBINER_OP_MAX_KHR:
        case VK_FRAGMENT_SHADING_RATE_COMBINER_OP_MUL_KHR:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkVideoEncodeTuningModeKHR value) const {
    switch (value) {
        case VK_VIDEO_ENCODE_TUNING_MODE_DEFAULT_KHR:
        case VK_VIDEO_ENCODE_TUNING_MODE_HIGH_QUALITY_KHR:
        case VK_VIDEO_ENCODE_TUNING_MODE_LOW_LATENCY_KHR:
        case VK_VIDEO_ENCODE_TUNING_MODE_ULTRA_LOW_LATENCY_KHR:
        case VK_VIDEO_ENCODE_TUNING_MODE_LOSSLESS_KHR:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkLineRasterizationModeKHR value) const {
    switch (value) {
        case VK_LINE_RASTERIZATION_MODE_DEFAULT_KHR:
        case VK_LINE_RASTERIZATION_MODE_RECTANGULAR_KHR:
        case VK_LINE_RASTERIZATION_MODE_BRESENHAM_KHR:
        case VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH_KHR:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkTimeDomainKHR value) const {
    switch (value) {
        case VK_TIME_DOMAIN_DEVICE_KHR:
        case VK_TIME_DOMAIN_CLOCK_MONOTONIC_KHR:
        case VK_TIME_DOMAIN_CLOCK_MONOTONIC_RAW_KHR:
        case VK_TIME_DOMAIN_QUERY_PERFORMANCE_COUNTER_KHR:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkDebugReportObjectTypeEXT value) const {
    switch (value) {
        case VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_EVENT_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_QUERY_POOL_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_VIEW_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_CACHE_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_POOL_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_SURFACE_KHR_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_DISPLAY_KHR_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_DISPLAY_MODE_KHR_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_VALIDATION_CACHE_EXT_EXT:
            return ValidValue::Valid;
        case VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION_EXT:
            return IsExtEnabled(device_extensions.vk_khr_sampler_ycbcr_conversion) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_EXT:
            return IsExtEnabled(device_extensions.vk_khr_descriptor_update_template) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_DEBUG_REPORT_OBJECT_TYPE_CU_MODULE_NVX_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_CU_FUNCTION_NVX_EXT:
            return IsExtEnabled(device_extensions.vk_nvx_binary_import) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_DEBUG_REPORT_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR_EXT:
            return IsExtEnabled(device_extensions.vk_khr_acceleration_structure) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_DEBUG_REPORT_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV_EXT:
            return IsExtEnabled(device_extensions.vk_nv_ray_tracing) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_DEBUG_REPORT_OBJECT_TYPE_CUDA_MODULE_NV_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_CUDA_FUNCTION_NV_EXT:
            return IsExtEnabled(device_extensions.vk_nv_cuda_kernel_launch) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_COLLECTION_FUCHSIA_EXT:
            return IsExtEnabled(device_extensions.vk_fuchsia_buffer_collection) ? ValidValue::Valid : ValidValue::NoExtension;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkRasterizationOrderAMD value) const {
    switch (value) {
        case VK_RASTERIZATION_ORDER_STRICT_AMD:
        case VK_RASTERIZATION_ORDER_RELAXED_AMD:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkShaderInfoTypeAMD value) const {
    switch (value) {
        case VK_SHADER_INFO_TYPE_STATISTICS_AMD:
        case VK_SHADER_INFO_TYPE_BINARY_AMD:
        case VK_SHADER_INFO_TYPE_DISASSEMBLY_AMD:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkValidationCheckEXT value) const {
    switch (value) {
        case VK_VALIDATION_CHECK_ALL_EXT:
        case VK_VALIDATION_CHECK_SHADERS_EXT:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkPipelineRobustnessBufferBehaviorEXT value) const {
    switch (value) {
        case VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_DEVICE_DEFAULT_EXT:
        case VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_DISABLED_EXT:
        case VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS_EXT:
        case VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS_2_EXT:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkPipelineRobustnessImageBehaviorEXT value) const {
    switch (value) {
        case VK_PIPELINE_ROBUSTNESS_IMAGE_BEHAVIOR_DEVICE_DEFAULT_EXT:
        case VK_PIPELINE_ROBUSTNESS_IMAGE_BEHAVIOR_DISABLED_EXT:
        case VK_PIPELINE_ROBUSTNESS_IMAGE_BEHAVIOR_ROBUST_IMAGE_ACCESS_EXT:
        case VK_PIPELINE_ROBUSTNESS_IMAGE_BEHAVIOR_ROBUST_IMAGE_ACCESS_2_EXT:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkDisplayPowerStateEXT value) const {
    switch (value) {
        case VK_DISPLAY_POWER_STATE_OFF_EXT:
        case VK_DISPLAY_POWER_STATE_SUSPEND_EXT:
        case VK_DISPLAY_POWER_STATE_ON_EXT:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkDeviceEventTypeEXT value) const {
    switch (value) {
        case VK_DEVICE_EVENT_TYPE_DISPLAY_HOTPLUG_EXT:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkDisplayEventTypeEXT value) const {
    switch (value) {
        case VK_DISPLAY_EVENT_TYPE_FIRST_PIXEL_OUT_EXT:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkViewportCoordinateSwizzleNV value) const {
    switch (value) {
        case VK_VIEWPORT_COORDINATE_SWIZZLE_POSITIVE_X_NV:
        case VK_VIEWPORT_COORDINATE_SWIZZLE_NEGATIVE_X_NV:
        case VK_VIEWPORT_COORDINATE_SWIZZLE_POSITIVE_Y_NV:
        case VK_VIEWPORT_COORDINATE_SWIZZLE_NEGATIVE_Y_NV:
        case VK_VIEWPORT_COORDINATE_SWIZZLE_POSITIVE_Z_NV:
        case VK_VIEWPORT_COORDINATE_SWIZZLE_NEGATIVE_Z_NV:
        case VK_VIEWPORT_COORDINATE_SWIZZLE_POSITIVE_W_NV:
        case VK_VIEWPORT_COORDINATE_SWIZZLE_NEGATIVE_W_NV:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkDiscardRectangleModeEXT value) const {
    switch (value) {
        case VK_DISCARD_RECTANGLE_MODE_INCLUSIVE_EXT:
        case VK_DISCARD_RECTANGLE_MODE_EXCLUSIVE_EXT:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkConservativeRasterizationModeEXT value) const {
    switch (value) {
        case VK_CONSERVATIVE_RASTERIZATION_MODE_DISABLED_EXT:
        case VK_CONSERVATIVE_RASTERIZATION_MODE_OVERESTIMATE_EXT:
        case VK_CONSERVATIVE_RASTERIZATION_MODE_UNDERESTIMATE_EXT:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkBlendOverlapEXT value) const {
    switch (value) {
        case VK_BLEND_OVERLAP_UNCORRELATED_EXT:
        case VK_BLEND_OVERLAP_DISJOINT_EXT:
        case VK_BLEND_OVERLAP_CONJOINT_EXT:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkCoverageModulationModeNV value) const {
    switch (value) {
        case VK_COVERAGE_MODULATION_MODE_NONE_NV:
        case VK_COVERAGE_MODULATION_MODE_RGB_NV:
        case VK_COVERAGE_MODULATION_MODE_ALPHA_NV:
        case VK_COVERAGE_MODULATION_MODE_RGBA_NV:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkShadingRatePaletteEntryNV value) const {
    switch (value) {
        case VK_SHADING_RATE_PALETTE_ENTRY_NO_INVOCATIONS_NV:
        case VK_SHADING_RATE_PALETTE_ENTRY_16_INVOCATIONS_PER_PIXEL_NV:
        case VK_SHADING_RATE_PALETTE_ENTRY_8_INVOCATIONS_PER_PIXEL_NV:
        case VK_SHADING_RATE_PALETTE_ENTRY_4_INVOCATIONS_PER_PIXEL_NV:
        case VK_SHADING_RATE_PALETTE_ENTRY_2_INVOCATIONS_PER_PIXEL_NV:
        case VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_PIXEL_NV:
        case VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_2X1_PIXELS_NV:
        case VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_1X2_PIXELS_NV:
        case VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_2X2_PIXELS_NV:
        case VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_4X2_PIXELS_NV:
        case VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_2X4_PIXELS_NV:
        case VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_4X4_PIXELS_NV:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkCoarseSampleOrderTypeNV value) const {
    switch (value) {
        case VK_COARSE_SAMPLE_ORDER_TYPE_DEFAULT_NV:
        case VK_COARSE_SAMPLE_ORDER_TYPE_CUSTOM_NV:
        case VK_COARSE_SAMPLE_ORDER_TYPE_PIXEL_MAJOR_NV:
        case VK_COARSE_SAMPLE_ORDER_TYPE_SAMPLE_MAJOR_NV:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkRayTracingShaderGroupTypeKHR value) const {
    switch (value) {
        case VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR:
        case VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR:
        case VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkGeometryTypeKHR value) const {
    switch (value) {
        case VK_GEOMETRY_TYPE_TRIANGLES_KHR:
        case VK_GEOMETRY_TYPE_AABBS_KHR:
        case VK_GEOMETRY_TYPE_INSTANCES_KHR:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkAccelerationStructureTypeKHR value) const {
    switch (value) {
        case VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR:
        case VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR:
        case VK_ACCELERATION_STRUCTURE_TYPE_GENERIC_KHR:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkCopyAccelerationStructureModeKHR value) const {
    switch (value) {
        case VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_KHR:
        case VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_KHR:
        case VK_COPY_ACCELERATION_STRUCTURE_MODE_SERIALIZE_KHR:
        case VK_COPY_ACCELERATION_STRUCTURE_MODE_DESERIALIZE_KHR:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkAccelerationStructureMemoryRequirementsTypeNV value) const {
    switch (value) {
        case VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV:
        case VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_NV:
        case VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_UPDATE_SCRATCH_NV:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkMemoryOverallocationBehaviorAMD value) const {
    switch (value) {
        case VK_MEMORY_OVERALLOCATION_BEHAVIOR_DEFAULT_AMD:
        case VK_MEMORY_OVERALLOCATION_BEHAVIOR_ALLOWED_AMD:
        case VK_MEMORY_OVERALLOCATION_BEHAVIOR_DISALLOWED_AMD:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkPerformanceConfigurationTypeINTEL value) const {
    switch (value) {
        case VK_PERFORMANCE_CONFIGURATION_TYPE_COMMAND_QUEUE_METRICS_DISCOVERY_ACTIVATED_INTEL:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkQueryPoolSamplingModeINTEL value) const {
    switch (value) {
        case VK_QUERY_POOL_SAMPLING_MODE_MANUAL_INTEL:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkPerformanceOverrideTypeINTEL value) const {
    switch (value) {
        case VK_PERFORMANCE_OVERRIDE_TYPE_NULL_HARDWARE_INTEL:
        case VK_PERFORMANCE_OVERRIDE_TYPE_FLUSH_GPU_CACHES_INTEL:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkPerformanceParameterTypeINTEL value) const {
    switch (value) {
        case VK_PERFORMANCE_PARAMETER_TYPE_HW_COUNTERS_SUPPORTED_INTEL:
        case VK_PERFORMANCE_PARAMETER_TYPE_STREAM_MARKER_VALID_BITS_INTEL:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkValidationFeatureEnableEXT value) const {
    switch (value) {
        case VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT:
        case VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT:
        case VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT:
        case VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT:
        case VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkValidationFeatureDisableEXT value) const {
    switch (value) {
        case VK_VALIDATION_FEATURE_DISABLE_ALL_EXT:
        case VK_VALIDATION_FEATURE_DISABLE_SHADERS_EXT:
        case VK_VALIDATION_FEATURE_DISABLE_THREAD_SAFETY_EXT:
        case VK_VALIDATION_FEATURE_DISABLE_API_PARAMETERS_EXT:
        case VK_VALIDATION_FEATURE_DISABLE_OBJECT_LIFETIMES_EXT:
        case VK_VALIDATION_FEATURE_DISABLE_CORE_CHECKS_EXT:
        case VK_VALIDATION_FEATURE_DISABLE_UNIQUE_HANDLES_EXT:
        case VK_VALIDATION_FEATURE_DISABLE_SHADER_VALIDATION_CACHE_EXT:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkCoverageReductionModeNV value) const {
    switch (value) {
        case VK_COVERAGE_REDUCTION_MODE_MERGE_NV:
        case VK_COVERAGE_REDUCTION_MODE_TRUNCATE_NV:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkProvokingVertexModeEXT value) const {
    switch (value) {
        case VK_PROVOKING_VERTEX_MODE_FIRST_VERTEX_EXT:
        case VK_PROVOKING_VERTEX_MODE_LAST_VERTEX_EXT:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
template <>
ValidValue StatelessValidation::IsValidEnumValue(VkFullScreenExclusiveEXT value) const {
    switch (value) {
        case VK_FULL_SCREEN_EXCLUSIVE_DEFAULT_EXT:
        case VK_FULL_SCREEN_EXCLUSIVE_ALLOWED_EXT:
        case VK_FULL_SCREEN_EXCLUSIVE_DISALLOWED_EXT:
        case VK_FULL_SCREEN_EXCLUSIVE_APPLICATION_CONTROLLED_EXT:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkIndirectCommandsTokenTypeNV value) const {
    switch (value) {
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_SHADER_GROUP_NV:
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_STATE_FLAGS_NV:
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_INDEX_BUFFER_NV:
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_VERTEX_BUFFER_NV:
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_PUSH_CONSTANT_NV:
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_INDEXED_NV:
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_NV:
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_TASKS_NV:
            return ValidValue::Valid;
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_MESH_TASKS_NV:
            return IsExtEnabled(device_extensions.vk_ext_mesh_shader) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_PIPELINE_NV:
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_DISPATCH_NV:
            return IsExtEnabled(device_extensions.vk_nv_device_generated_commands_compute) ? ValidValue::Valid
                                                                                           : ValidValue::NoExtension;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkDepthBiasRepresentationEXT value) const {
    switch (value) {
        case VK_DEPTH_BIAS_REPRESENTATION_LEAST_REPRESENTABLE_VALUE_FORMAT_EXT:
        case VK_DEPTH_BIAS_REPRESENTATION_LEAST_REPRESENTABLE_VALUE_FORCE_UNORM_EXT:
        case VK_DEPTH_BIAS_REPRESENTATION_FLOAT_EXT:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkFragmentShadingRateTypeNV value) const {
    switch (value) {
        case VK_FRAGMENT_SHADING_RATE_TYPE_FRAGMENT_SIZE_NV:
        case VK_FRAGMENT_SHADING_RATE_TYPE_ENUMS_NV:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkFragmentShadingRateNV value) const {
    switch (value) {
        case VK_FRAGMENT_SHADING_RATE_1_INVOCATION_PER_PIXEL_NV:
        case VK_FRAGMENT_SHADING_RATE_1_INVOCATION_PER_1X2_PIXELS_NV:
        case VK_FRAGMENT_SHADING_RATE_1_INVOCATION_PER_2X1_PIXELS_NV:
        case VK_FRAGMENT_SHADING_RATE_1_INVOCATION_PER_2X2_PIXELS_NV:
        case VK_FRAGMENT_SHADING_RATE_1_INVOCATION_PER_2X4_PIXELS_NV:
        case VK_FRAGMENT_SHADING_RATE_1_INVOCATION_PER_4X2_PIXELS_NV:
        case VK_FRAGMENT_SHADING_RATE_1_INVOCATION_PER_4X4_PIXELS_NV:
        case VK_FRAGMENT_SHADING_RATE_2_INVOCATIONS_PER_PIXEL_NV:
        case VK_FRAGMENT_SHADING_RATE_4_INVOCATIONS_PER_PIXEL_NV:
        case VK_FRAGMENT_SHADING_RATE_8_INVOCATIONS_PER_PIXEL_NV:
        case VK_FRAGMENT_SHADING_RATE_16_INVOCATIONS_PER_PIXEL_NV:
        case VK_FRAGMENT_SHADING_RATE_NO_INVOCATIONS_NV:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkAccelerationStructureMotionInstanceTypeNV value) const {
    switch (value) {
        case VK_ACCELERATION_STRUCTURE_MOTION_INSTANCE_TYPE_STATIC_NV:
        case VK_ACCELERATION_STRUCTURE_MOTION_INSTANCE_TYPE_MATRIX_MOTION_NV:
        case VK_ACCELERATION_STRUCTURE_MOTION_INSTANCE_TYPE_SRT_MOTION_NV:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkDeviceFaultAddressTypeEXT value) const {
    switch (value) {
        case VK_DEVICE_FAULT_ADDRESS_TYPE_NONE_EXT:
        case VK_DEVICE_FAULT_ADDRESS_TYPE_READ_INVALID_EXT:
        case VK_DEVICE_FAULT_ADDRESS_TYPE_WRITE_INVALID_EXT:
        case VK_DEVICE_FAULT_ADDRESS_TYPE_EXECUTE_INVALID_EXT:
        case VK_DEVICE_FAULT_ADDRESS_TYPE_INSTRUCTION_POINTER_UNKNOWN_EXT:
        case VK_DEVICE_FAULT_ADDRESS_TYPE_INSTRUCTION_POINTER_INVALID_EXT:
        case VK_DEVICE_FAULT_ADDRESS_TYPE_INSTRUCTION_POINTER_FAULT_EXT:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkDeviceFaultVendorBinaryHeaderVersionEXT value) const {
    switch (value) {
        case VK_DEVICE_FAULT_VENDOR_BINARY_HEADER_VERSION_ONE_EXT:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkDeviceAddressBindingTypeEXT value) const {
    switch (value) {
        case VK_DEVICE_ADDRESS_BINDING_TYPE_BIND_EXT:
        case VK_DEVICE_ADDRESS_BINDING_TYPE_UNBIND_EXT:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkMicromapTypeEXT value) const {
    switch (value) {
        case VK_MICROMAP_TYPE_OPACITY_MICROMAP_EXT:
            return ValidValue::Valid;
        case VK_MICROMAP_TYPE_DISPLACEMENT_MICROMAP_NV:
            return IsExtEnabled(device_extensions.vk_nv_displacement_micromap) ? ValidValue::Valid : ValidValue::NoExtension;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkBuildMicromapModeEXT value) const {
    switch (value) {
        case VK_BUILD_MICROMAP_MODE_BUILD_EXT:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkCopyMicromapModeEXT value) const {
    switch (value) {
        case VK_COPY_MICROMAP_MODE_CLONE_EXT:
        case VK_COPY_MICROMAP_MODE_SERIALIZE_EXT:
        case VK_COPY_MICROMAP_MODE_DESERIALIZE_EXT:
        case VK_COPY_MICROMAP_MODE_COMPACT_EXT:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkAccelerationStructureCompatibilityKHR value) const {
    switch (value) {
        case VK_ACCELERATION_STRUCTURE_COMPATIBILITY_COMPATIBLE_KHR:
        case VK_ACCELERATION_STRUCTURE_COMPATIBILITY_INCOMPATIBLE_KHR:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkAccelerationStructureBuildTypeKHR value) const {
    switch (value) {
        case VK_ACCELERATION_STRUCTURE_BUILD_TYPE_HOST_KHR:
        case VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR:
        case VK_ACCELERATION_STRUCTURE_BUILD_TYPE_HOST_OR_DEVICE_KHR:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkDirectDriverLoadingModeLUNARG value) const {
    switch (value) {
        case VK_DIRECT_DRIVER_LOADING_MODE_EXCLUSIVE_LUNARG:
        case VK_DIRECT_DRIVER_LOADING_MODE_INCLUSIVE_LUNARG:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkOpticalFlowPerformanceLevelNV value) const {
    switch (value) {
        case VK_OPTICAL_FLOW_PERFORMANCE_LEVEL_UNKNOWN_NV:
        case VK_OPTICAL_FLOW_PERFORMANCE_LEVEL_SLOW_NV:
        case VK_OPTICAL_FLOW_PERFORMANCE_LEVEL_MEDIUM_NV:
        case VK_OPTICAL_FLOW_PERFORMANCE_LEVEL_FAST_NV:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkOpticalFlowSessionBindingPointNV value) const {
    switch (value) {
        case VK_OPTICAL_FLOW_SESSION_BINDING_POINT_UNKNOWN_NV:
        case VK_OPTICAL_FLOW_SESSION_BINDING_POINT_INPUT_NV:
        case VK_OPTICAL_FLOW_SESSION_BINDING_POINT_REFERENCE_NV:
        case VK_OPTICAL_FLOW_SESSION_BINDING_POINT_HINT_NV:
        case VK_OPTICAL_FLOW_SESSION_BINDING_POINT_FLOW_VECTOR_NV:
        case VK_OPTICAL_FLOW_SESSION_BINDING_POINT_BACKWARD_FLOW_VECTOR_NV:
        case VK_OPTICAL_FLOW_SESSION_BINDING_POINT_COST_NV:
        case VK_OPTICAL_FLOW_SESSION_BINDING_POINT_BACKWARD_COST_NV:
        case VK_OPTICAL_FLOW_SESSION_BINDING_POINT_GLOBAL_FLOW_NV:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkShaderCodeTypeEXT value) const {
    switch (value) {
        case VK_SHADER_CODE_TYPE_BINARY_EXT:
        case VK_SHADER_CODE_TYPE_SPIRV_EXT:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkLayerSettingTypeEXT value) const {
    switch (value) {
        case VK_LAYER_SETTING_TYPE_BOOL32_EXT:
        case VK_LAYER_SETTING_TYPE_INT32_EXT:
        case VK_LAYER_SETTING_TYPE_INT64_EXT:
        case VK_LAYER_SETTING_TYPE_UINT32_EXT:
        case VK_LAYER_SETTING_TYPE_UINT64_EXT:
        case VK_LAYER_SETTING_TYPE_FLOAT32_EXT:
        case VK_LAYER_SETTING_TYPE_FLOAT64_EXT:
        case VK_LAYER_SETTING_TYPE_STRING_EXT:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkLatencyMarkerNV value) const {
    switch (value) {
        case VK_LATENCY_MARKER_SIMULATION_START_NV:
        case VK_LATENCY_MARKER_SIMULATION_END_NV:
        case VK_LATENCY_MARKER_RENDERSUBMIT_START_NV:
        case VK_LATENCY_MARKER_RENDERSUBMIT_END_NV:
        case VK_LATENCY_MARKER_PRESENT_START_NV:
        case VK_LATENCY_MARKER_PRESENT_END_NV:
        case VK_LATENCY_MARKER_INPUT_SAMPLE_NV:
        case VK_LATENCY_MARKER_TRIGGER_FLASH_NV:
        case VK_LATENCY_MARKER_OUT_OF_BAND_RENDERSUBMIT_START_NV:
        case VK_LATENCY_MARKER_OUT_OF_BAND_RENDERSUBMIT_END_NV:
        case VK_LATENCY_MARKER_OUT_OF_BAND_PRESENT_START_NV:
        case VK_LATENCY_MARKER_OUT_OF_BAND_PRESENT_END_NV:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkOutOfBandQueueTypeNV value) const {
    switch (value) {
        case VK_OUT_OF_BAND_QUEUE_TYPE_RENDER_NV:
        case VK_OUT_OF_BAND_QUEUE_TYPE_PRESENT_NV:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkBlockMatchWindowCompareModeQCOM value) const {
    switch (value) {
        case VK_BLOCK_MATCH_WINDOW_COMPARE_MODE_MIN_QCOM:
        case VK_BLOCK_MATCH_WINDOW_COMPARE_MODE_MAX_QCOM:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkCubicFilterWeightsQCOM value) const {
    switch (value) {
        case VK_CUBIC_FILTER_WEIGHTS_CATMULL_ROM_QCOM:
        case VK_CUBIC_FILTER_WEIGHTS_ZERO_TANGENT_CARDINAL_QCOM:
        case VK_CUBIC_FILTER_WEIGHTS_B_SPLINE_QCOM:
        case VK_CUBIC_FILTER_WEIGHTS_MITCHELL_NETRAVALI_QCOM:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkBuildAccelerationStructureModeKHR value) const {
    switch (value) {
        case VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR:
        case VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue StatelessValidation::IsValidEnumValue(VkShaderGroupShaderKHR value) const {
    switch (value) {
        case VK_SHADER_GROUP_SHADER_GENERAL_KHR:
        case VK_SHADER_GROUP_SHADER_CLOSEST_HIT_KHR:
        case VK_SHADER_GROUP_SHADER_ANY_HIT_KHR:
        case VK_SHADER_GROUP_SHADER_INTERSECTION_KHR:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkPipelineCacheHeaderVersion value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkImageLayout value) const {
    switch (value) {
        case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
        case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
            return {vvl::Extension::_VK_KHR_maintenance2};
        case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
        case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL:
        case VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL:
        case VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL:
            return {vvl::Extension::_VK_KHR_separate_depth_stencil_layouts};
        case VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL:
        case VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL:
            return {vvl::Extension::_VK_KHR_synchronization2};
        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
            return {vvl::Extension::_VK_KHR_swapchain};
        case VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR:
        case VK_IMAGE_LAYOUT_VIDEO_DECODE_SRC_KHR:
        case VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR:
            return {vvl::Extension::_VK_KHR_video_decode_queue};
        case VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR:
            return {vvl::Extension::_VK_KHR_shared_presentable_image};
        case VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT:
            return {vvl::Extension::_VK_EXT_fragment_density_map};
        case VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR:
            return {vvl::Extension::_VK_KHR_fragment_shading_rate, vvl::Extension::_VK_NV_shading_rate_image};
        case VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ_KHR:
            return {vvl::Extension::_VK_KHR_dynamic_rendering_local_read};
        case VK_IMAGE_LAYOUT_VIDEO_ENCODE_DST_KHR:
        case VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR:
        case VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR:
            return {vvl::Extension::_VK_KHR_video_encode_queue};
        case VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT:
            return {vvl::Extension::_VK_EXT_attachment_feedback_loop_layout};
        default:
            return {};
    };
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkObjectType value) const {
    switch (value) {
        case VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION:
            return {vvl::Extension::_VK_KHR_sampler_ycbcr_conversion};
        case VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE:
            return {vvl::Extension::_VK_KHR_descriptor_update_template};
        case VK_OBJECT_TYPE_PRIVATE_DATA_SLOT:
            return {vvl::Extension::_VK_EXT_private_data};
        case VK_OBJECT_TYPE_SURFACE_KHR:
            return {vvl::Extension::_VK_KHR_surface};
        case VK_OBJECT_TYPE_SWAPCHAIN_KHR:
            return {vvl::Extension::_VK_KHR_swapchain};
        case VK_OBJECT_TYPE_DISPLAY_KHR:
        case VK_OBJECT_TYPE_DISPLAY_MODE_KHR:
            return {vvl::Extension::_VK_KHR_display};
        case VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT:
            return {vvl::Extension::_VK_EXT_debug_report};
        case VK_OBJECT_TYPE_VIDEO_SESSION_KHR:
        case VK_OBJECT_TYPE_VIDEO_SESSION_PARAMETERS_KHR:
            return {vvl::Extension::_VK_KHR_video_queue};
        case VK_OBJECT_TYPE_CU_MODULE_NVX:
        case VK_OBJECT_TYPE_CU_FUNCTION_NVX:
            return {vvl::Extension::_VK_NVX_binary_import};
        case VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT:
            return {vvl::Extension::_VK_EXT_debug_utils};
        case VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR:
            return {vvl::Extension::_VK_KHR_acceleration_structure};
        case VK_OBJECT_TYPE_VALIDATION_CACHE_EXT:
            return {vvl::Extension::_VK_EXT_validation_cache};
        case VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV:
            return {vvl::Extension::_VK_NV_ray_tracing};
        case VK_OBJECT_TYPE_PERFORMANCE_CONFIGURATION_INTEL:
            return {vvl::Extension::_VK_INTEL_performance_query};
        case VK_OBJECT_TYPE_DEFERRED_OPERATION_KHR:
            return {vvl::Extension::_VK_KHR_deferred_host_operations};
        case VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NV:
            return {vvl::Extension::_VK_NV_device_generated_commands};
        case VK_OBJECT_TYPE_CUDA_MODULE_NV:
        case VK_OBJECT_TYPE_CUDA_FUNCTION_NV:
            return {vvl::Extension::_VK_NV_cuda_kernel_launch};
        case VK_OBJECT_TYPE_BUFFER_COLLECTION_FUCHSIA:
            return {vvl::Extension::_VK_FUCHSIA_buffer_collection};
        case VK_OBJECT_TYPE_MICROMAP_EXT:
            return {vvl::Extension::_VK_EXT_opacity_micromap};
        case VK_OBJECT_TYPE_OPTICAL_FLOW_SESSION_NV:
            return {vvl::Extension::_VK_NV_optical_flow};
        case VK_OBJECT_TYPE_SHADER_EXT:
            return {vvl::Extension::_VK_EXT_shader_object};
        default:
            return {};
    };
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkFormat value) const {
    switch (value) {
        case VK_FORMAT_G8B8G8R8_422_UNORM:
        case VK_FORMAT_B8G8R8G8_422_UNORM:
        case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM:
        case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM:
        case VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM:
        case VK_FORMAT_G8_B8R8_2PLANE_422_UNORM:
        case VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM:
        case VK_FORMAT_R10X6_UNORM_PACK16:
        case VK_FORMAT_R10X6G10X6_UNORM_2PACK16:
        case VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16:
        case VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16:
        case VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16:
        case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16:
        case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16:
        case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16:
        case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16:
        case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16:
        case VK_FORMAT_R12X4_UNORM_PACK16:
        case VK_FORMAT_R12X4G12X4_UNORM_2PACK16:
        case VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16:
        case VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16:
        case VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16:
        case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16:
        case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16:
        case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16:
        case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16:
        case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16:
        case VK_FORMAT_G16B16G16R16_422_UNORM:
        case VK_FORMAT_B16G16R16G16_422_UNORM:
        case VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM:
        case VK_FORMAT_G16_B16R16_2PLANE_420_UNORM:
        case VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM:
        case VK_FORMAT_G16_B16R16_2PLANE_422_UNORM:
        case VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM:
            return {vvl::Extension::_VK_KHR_sampler_ycbcr_conversion};
        case VK_FORMAT_G8_B8R8_2PLANE_444_UNORM:
        case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16:
        case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16:
        case VK_FORMAT_G16_B16R16_2PLANE_444_UNORM:
            return {vvl::Extension::_VK_EXT_ycbcr_2plane_444_formats};
        case VK_FORMAT_A4R4G4B4_UNORM_PACK16:
        case VK_FORMAT_A4B4G4R4_UNORM_PACK16:
            return {vvl::Extension::_VK_EXT_4444_formats};
        case VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK:
            return {vvl::Extension::_VK_EXT_texture_compression_astc_hdr};
        case VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG:
        case VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG:
        case VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG:
        case VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG:
        case VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG:
        case VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG:
        case VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG:
        case VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG:
            return {vvl::Extension::_VK_IMG_format_pvrtc};
        case VK_FORMAT_R16G16_S10_5_NV:
            return {vvl::Extension::_VK_NV_optical_flow};
        case VK_FORMAT_A1B5G5R5_UNORM_PACK16_KHR:
        case VK_FORMAT_A8_UNORM_KHR:
            return {vvl::Extension::_VK_KHR_maintenance5};
        default:
            return {};
    };
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkImageTiling value) const {
    switch (value) {
        case VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT:
            return {vvl::Extension::_VK_EXT_image_drm_format_modifier};
        default:
            return {};
    };
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkImageType value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkQueryType value) const {
    switch (value) {
        case VK_QUERY_TYPE_RESULT_STATUS_ONLY_KHR:
            return {vvl::Extension::_VK_KHR_video_queue};
        case VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT:
            return {vvl::Extension::_VK_EXT_transform_feedback};
        case VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR:
            return {vvl::Extension::_VK_KHR_performance_query};
        case VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR:
        case VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR:
            return {vvl::Extension::_VK_KHR_acceleration_structure};
        case VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_NV:
            return {vvl::Extension::_VK_NV_ray_tracing};
        case VK_QUERY_TYPE_PERFORMANCE_QUERY_INTEL:
            return {vvl::Extension::_VK_INTEL_performance_query};
        case VK_QUERY_TYPE_VIDEO_ENCODE_FEEDBACK_KHR:
            return {vvl::Extension::_VK_KHR_video_encode_queue};
        case VK_QUERY_TYPE_MESH_PRIMITIVES_GENERATED_EXT:
            return {vvl::Extension::_VK_EXT_mesh_shader};
        case VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT:
            return {vvl::Extension::_VK_EXT_primitives_generated_query};
        case VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_BOTTOM_LEVEL_POINTERS_KHR:
        case VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SIZE_KHR:
            return {vvl::Extension::_VK_KHR_ray_tracing_maintenance1};
        case VK_QUERY_TYPE_MICROMAP_SERIALIZATION_SIZE_EXT:
        case VK_QUERY_TYPE_MICROMAP_COMPACTED_SIZE_EXT:
            return {vvl::Extension::_VK_EXT_opacity_micromap};
        default:
            return {};
    };
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkSharingMode value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkComponentSwizzle value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkImageViewType value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkBlendFactor value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkBlendOp value) const {
    switch (value) {
        case VK_BLEND_OP_ZERO_EXT:
        case VK_BLEND_OP_SRC_EXT:
        case VK_BLEND_OP_DST_EXT:
        case VK_BLEND_OP_SRC_OVER_EXT:
        case VK_BLEND_OP_DST_OVER_EXT:
        case VK_BLEND_OP_SRC_IN_EXT:
        case VK_BLEND_OP_DST_IN_EXT:
        case VK_BLEND_OP_SRC_OUT_EXT:
        case VK_BLEND_OP_DST_OUT_EXT:
        case VK_BLEND_OP_SRC_ATOP_EXT:
        case VK_BLEND_OP_DST_ATOP_EXT:
        case VK_BLEND_OP_XOR_EXT:
        case VK_BLEND_OP_MULTIPLY_EXT:
        case VK_BLEND_OP_SCREEN_EXT:
        case VK_BLEND_OP_OVERLAY_EXT:
        case VK_BLEND_OP_DARKEN_EXT:
        case VK_BLEND_OP_LIGHTEN_EXT:
        case VK_BLEND_OP_COLORDODGE_EXT:
        case VK_BLEND_OP_COLORBURN_EXT:
        case VK_BLEND_OP_HARDLIGHT_EXT:
        case VK_BLEND_OP_SOFTLIGHT_EXT:
        case VK_BLEND_OP_DIFFERENCE_EXT:
        case VK_BLEND_OP_EXCLUSION_EXT:
        case VK_BLEND_OP_INVERT_EXT:
        case VK_BLEND_OP_INVERT_RGB_EXT:
        case VK_BLEND_OP_LINEARDODGE_EXT:
        case VK_BLEND_OP_LINEARBURN_EXT:
        case VK_BLEND_OP_VIVIDLIGHT_EXT:
        case VK_BLEND_OP_LINEARLIGHT_EXT:
        case VK_BLEND_OP_PINLIGHT_EXT:
        case VK_BLEND_OP_HARDMIX_EXT:
        case VK_BLEND_OP_HSL_HUE_EXT:
        case VK_BLEND_OP_HSL_SATURATION_EXT:
        case VK_BLEND_OP_HSL_COLOR_EXT:
        case VK_BLEND_OP_HSL_LUMINOSITY_EXT:
        case VK_BLEND_OP_PLUS_EXT:
        case VK_BLEND_OP_PLUS_CLAMPED_EXT:
        case VK_BLEND_OP_PLUS_CLAMPED_ALPHA_EXT:
        case VK_BLEND_OP_PLUS_DARKER_EXT:
        case VK_BLEND_OP_MINUS_EXT:
        case VK_BLEND_OP_MINUS_CLAMPED_EXT:
        case VK_BLEND_OP_CONTRAST_EXT:
        case VK_BLEND_OP_INVERT_OVG_EXT:
        case VK_BLEND_OP_RED_EXT:
        case VK_BLEND_OP_GREEN_EXT:
        case VK_BLEND_OP_BLUE_EXT:
            return {vvl::Extension::_VK_EXT_blend_operation_advanced};
        default:
            return {};
    };
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkCompareOp value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkDynamicState value) const {
    switch (value) {
        case VK_DYNAMIC_STATE_CULL_MODE:
        case VK_DYNAMIC_STATE_FRONT_FACE:
        case VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY:
        case VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT:
        case VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT:
        case VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE:
        case VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE:
        case VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE:
        case VK_DYNAMIC_STATE_DEPTH_COMPARE_OP:
        case VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE:
        case VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE:
        case VK_DYNAMIC_STATE_STENCIL_OP:
            return {vvl::Extension::_VK_EXT_extended_dynamic_state};
        case VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE:
        case VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE:
        case VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE:
        case VK_DYNAMIC_STATE_PATCH_CONTROL_POINTS_EXT:
        case VK_DYNAMIC_STATE_LOGIC_OP_EXT:
            return {vvl::Extension::_VK_EXT_extended_dynamic_state2};
        case VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_NV:
            return {vvl::Extension::_VK_NV_clip_space_w_scaling};
        case VK_DYNAMIC_STATE_DISCARD_RECTANGLE_EXT:
        case VK_DYNAMIC_STATE_DISCARD_RECTANGLE_ENABLE_EXT:
        case VK_DYNAMIC_STATE_DISCARD_RECTANGLE_MODE_EXT:
            return {vvl::Extension::_VK_EXT_discard_rectangles};
        case VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT:
            return {vvl::Extension::_VK_EXT_sample_locations};
        case VK_DYNAMIC_STATE_RAY_TRACING_PIPELINE_STACK_SIZE_KHR:
            return {vvl::Extension::_VK_KHR_ray_tracing_pipeline};
        case VK_DYNAMIC_STATE_VIEWPORT_SHADING_RATE_PALETTE_NV:
        case VK_DYNAMIC_STATE_VIEWPORT_COARSE_SAMPLE_ORDER_NV:
            return {vvl::Extension::_VK_NV_shading_rate_image};
        case VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_ENABLE_NV:
        case VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_NV:
            return {vvl::Extension::_VK_NV_scissor_exclusive};
        case VK_DYNAMIC_STATE_FRAGMENT_SHADING_RATE_KHR:
            return {vvl::Extension::_VK_KHR_fragment_shading_rate};
        case VK_DYNAMIC_STATE_VERTEX_INPUT_EXT:
            return {vvl::Extension::_VK_EXT_vertex_input_dynamic_state};
        case VK_DYNAMIC_STATE_COLOR_WRITE_ENABLE_EXT:
            return {vvl::Extension::_VK_EXT_color_write_enable};
        case VK_DYNAMIC_STATE_DEPTH_CLAMP_ENABLE_EXT:
        case VK_DYNAMIC_STATE_POLYGON_MODE_EXT:
        case VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT:
        case VK_DYNAMIC_STATE_SAMPLE_MASK_EXT:
        case VK_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT:
        case VK_DYNAMIC_STATE_ALPHA_TO_ONE_ENABLE_EXT:
        case VK_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT:
        case VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT:
        case VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT:
        case VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT:
        case VK_DYNAMIC_STATE_TESSELLATION_DOMAIN_ORIGIN_EXT:
        case VK_DYNAMIC_STATE_RASTERIZATION_STREAM_EXT:
        case VK_DYNAMIC_STATE_CONSERVATIVE_RASTERIZATION_MODE_EXT:
        case VK_DYNAMIC_STATE_EXTRA_PRIMITIVE_OVERESTIMATION_SIZE_EXT:
        case VK_DYNAMIC_STATE_DEPTH_CLIP_ENABLE_EXT:
        case VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_ENABLE_EXT:
        case VK_DYNAMIC_STATE_COLOR_BLEND_ADVANCED_EXT:
        case VK_DYNAMIC_STATE_PROVOKING_VERTEX_MODE_EXT:
        case VK_DYNAMIC_STATE_LINE_RASTERIZATION_MODE_EXT:
        case VK_DYNAMIC_STATE_LINE_STIPPLE_ENABLE_EXT:
        case VK_DYNAMIC_STATE_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE_EXT:
        case VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_ENABLE_NV:
        case VK_DYNAMIC_STATE_VIEWPORT_SWIZZLE_NV:
        case VK_DYNAMIC_STATE_COVERAGE_TO_COLOR_ENABLE_NV:
        case VK_DYNAMIC_STATE_COVERAGE_TO_COLOR_LOCATION_NV:
        case VK_DYNAMIC_STATE_COVERAGE_MODULATION_MODE_NV:
        case VK_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_ENABLE_NV:
        case VK_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_NV:
        case VK_DYNAMIC_STATE_SHADING_RATE_IMAGE_ENABLE_NV:
        case VK_DYNAMIC_STATE_REPRESENTATIVE_FRAGMENT_TEST_ENABLE_NV:
        case VK_DYNAMIC_STATE_COVERAGE_REDUCTION_MODE_NV:
            return {vvl::Extension::_VK_EXT_extended_dynamic_state3};
        case VK_DYNAMIC_STATE_ATTACHMENT_FEEDBACK_LOOP_ENABLE_EXT:
            return {vvl::Extension::_VK_EXT_attachment_feedback_loop_dynamic_state};
        case VK_DYNAMIC_STATE_LINE_STIPPLE_KHR:
            return {vvl::Extension::_VK_KHR_line_rasterization, vvl::Extension::_VK_EXT_line_rasterization};
        default:
            return {};
    };
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkFrontFace value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkVertexInputRate value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkPrimitiveTopology value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkPolygonMode value) const {
    switch (value) {
        case VK_POLYGON_MODE_FILL_RECTANGLE_NV:
            return {vvl::Extension::_VK_NV_fill_rectangle};
        default:
            return {};
    };
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkStencilOp value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkLogicOp value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkBorderColor value) const {
    switch (value) {
        case VK_BORDER_COLOR_FLOAT_CUSTOM_EXT:
        case VK_BORDER_COLOR_INT_CUSTOM_EXT:
            return {vvl::Extension::_VK_EXT_custom_border_color};
        default:
            return {};
    };
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkFilter value) const {
    switch (value) {
        case VK_FILTER_CUBIC_EXT:
            return {vvl::Extension::_VK_IMG_filter_cubic, vvl::Extension::_VK_EXT_filter_cubic};
        default:
            return {};
    };
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkSamplerAddressMode value) const {
    switch (value) {
        case VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE:
            return {vvl::Extension::_VK_KHR_sampler_mirror_clamp_to_edge};
        default:
            return {};
    };
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkSamplerMipmapMode value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkDescriptorType value) const {
    switch (value) {
        case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK:
            return {vvl::Extension::_VK_EXT_inline_uniform_block};
        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
            return {vvl::Extension::_VK_KHR_acceleration_structure};
        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV:
            return {vvl::Extension::_VK_NV_ray_tracing};
        case VK_DESCRIPTOR_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM:
        case VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM:
            return {vvl::Extension::_VK_QCOM_image_processing};
        case VK_DESCRIPTOR_TYPE_MUTABLE_EXT:
            return {vvl::Extension::_VK_VALVE_mutable_descriptor_type, vvl::Extension::_VK_EXT_mutable_descriptor_type};
        default:
            return {};
    };
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkAttachmentLoadOp value) const {
    switch (value) {
        case VK_ATTACHMENT_LOAD_OP_NONE_KHR:
            return {vvl::Extension::_VK_KHR_load_store_op_none, vvl::Extension::_VK_EXT_load_store_op_none};
        default:
            return {};
    };
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkAttachmentStoreOp value) const {
    switch (value) {
        case VK_ATTACHMENT_STORE_OP_NONE:
            return {vvl::Extension::_VK_KHR_dynamic_rendering, vvl::Extension::_VK_KHR_load_store_op_none,
                    vvl::Extension::_VK_QCOM_render_pass_store_ops, vvl::Extension::_VK_EXT_load_store_op_none};
        default:
            return {};
    };
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkPipelineBindPoint value) const {
    switch (value) {
        case VK_PIPELINE_BIND_POINT_EXECUTION_GRAPH_AMDX:
            return {vvl::Extension::_VK_AMDX_shader_enqueue};
        case VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR:
            return {vvl::Extension::_VK_NV_ray_tracing, vvl::Extension::_VK_KHR_ray_tracing_pipeline};
        case VK_PIPELINE_BIND_POINT_SUBPASS_SHADING_HUAWEI:
            return {vvl::Extension::_VK_HUAWEI_subpass_shading};
        default:
            return {};
    };
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkCommandBufferLevel value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkIndexType value) const {
    switch (value) {
        case VK_INDEX_TYPE_NONE_KHR:
            return {vvl::Extension::_VK_NV_ray_tracing, vvl::Extension::_VK_KHR_acceleration_structure};
        case VK_INDEX_TYPE_UINT8_KHR:
            return {vvl::Extension::_VK_KHR_index_type_uint8, vvl::Extension::_VK_EXT_index_type_uint8};
        default:
            return {};
    };
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkSubpassContents value) const {
    switch (value) {
        case VK_SUBPASS_CONTENTS_INLINE_AND_SECONDARY_COMMAND_BUFFERS_EXT:
            return {vvl::Extension::_VK_EXT_nested_command_buffer};
        default:
            return {};
    };
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkTessellationDomainOrigin value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkSamplerYcbcrModelConversion value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkSamplerYcbcrRange value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkChromaLocation value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkDescriptorUpdateTemplateType value) const {
    switch (value) {
        case VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_PUSH_DESCRIPTORS_KHR:
            return {vvl::Extension::_VK_KHR_push_descriptor};
        default:
            return {};
    };
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkSamplerReductionMode value) const {
    switch (value) {
        case VK_SAMPLER_REDUCTION_MODE_WEIGHTED_AVERAGE_RANGECLAMP_QCOM:
            return {vvl::Extension::_VK_QCOM_filter_cubic_clamp};
        default:
            return {};
    };
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkSemaphoreType value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkPresentModeKHR value) const {
    switch (value) {
        case VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR:
        case VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR:
            return {vvl::Extension::_VK_KHR_shared_presentable_image};
        default:
            return {};
    };
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkColorSpaceKHR value) const {
    switch (value) {
        case VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT:
        case VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT:
        case VK_COLOR_SPACE_DISPLAY_P3_LINEAR_EXT:
        case VK_COLOR_SPACE_DCI_P3_NONLINEAR_EXT:
        case VK_COLOR_SPACE_BT709_LINEAR_EXT:
        case VK_COLOR_SPACE_BT709_NONLINEAR_EXT:
        case VK_COLOR_SPACE_BT2020_LINEAR_EXT:
        case VK_COLOR_SPACE_HDR10_ST2084_EXT:
        case VK_COLOR_SPACE_DOLBYVISION_EXT:
        case VK_COLOR_SPACE_HDR10_HLG_EXT:
        case VK_COLOR_SPACE_ADOBERGB_LINEAR_EXT:
        case VK_COLOR_SPACE_ADOBERGB_NONLINEAR_EXT:
        case VK_COLOR_SPACE_PASS_THROUGH_EXT:
        case VK_COLOR_SPACE_EXTENDED_SRGB_NONLINEAR_EXT:
            return {vvl::Extension::_VK_EXT_swapchain_colorspace};
        case VK_COLOR_SPACE_DISPLAY_NATIVE_AMD:
            return {vvl::Extension::_VK_AMD_display_native_hdr};
        default:
            return {};
    };
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkQueueGlobalPriorityKHR value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkFragmentShadingRateCombinerOpKHR value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkVideoEncodeTuningModeKHR value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkLineRasterizationModeKHR value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkTimeDomainKHR value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkDebugReportObjectTypeEXT value) const {
    switch (value) {
        case VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION_EXT:
            return {vvl::Extension::_VK_KHR_sampler_ycbcr_conversion};
        case VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_EXT:
            return {vvl::Extension::_VK_KHR_descriptor_update_template};
        case VK_DEBUG_REPORT_OBJECT_TYPE_CU_MODULE_NVX_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_CU_FUNCTION_NVX_EXT:
            return {vvl::Extension::_VK_NVX_binary_import};
        case VK_DEBUG_REPORT_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR_EXT:
            return {vvl::Extension::_VK_KHR_acceleration_structure};
        case VK_DEBUG_REPORT_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV_EXT:
            return {vvl::Extension::_VK_NV_ray_tracing};
        case VK_DEBUG_REPORT_OBJECT_TYPE_CUDA_MODULE_NV_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_CUDA_FUNCTION_NV_EXT:
            return {vvl::Extension::_VK_NV_cuda_kernel_launch};
        case VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_COLLECTION_FUCHSIA_EXT:
            return {vvl::Extension::_VK_FUCHSIA_buffer_collection};
        default:
            return {};
    };
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkRasterizationOrderAMD value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkShaderInfoTypeAMD value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkValidationCheckEXT value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkPipelineRobustnessBufferBehaviorEXT value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkPipelineRobustnessImageBehaviorEXT value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkDisplayPowerStateEXT value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkDeviceEventTypeEXT value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkDisplayEventTypeEXT value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkViewportCoordinateSwizzleNV value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkDiscardRectangleModeEXT value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkConservativeRasterizationModeEXT value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkBlendOverlapEXT value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkCoverageModulationModeNV value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkShadingRatePaletteEntryNV value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkCoarseSampleOrderTypeNV value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkRayTracingShaderGroupTypeKHR value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkGeometryTypeKHR value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkAccelerationStructureTypeKHR value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkCopyAccelerationStructureModeKHR value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkAccelerationStructureMemoryRequirementsTypeNV value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkMemoryOverallocationBehaviorAMD value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkPerformanceConfigurationTypeINTEL value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkQueryPoolSamplingModeINTEL value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkPerformanceOverrideTypeINTEL value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkPerformanceParameterTypeINTEL value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkValidationFeatureEnableEXT value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkValidationFeatureDisableEXT value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkCoverageReductionModeNV value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkProvokingVertexModeEXT value) const {
    return {};
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkFullScreenExclusiveEXT value) const {
    return {};
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkIndirectCommandsTokenTypeNV value) const {
    switch (value) {
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_MESH_TASKS_NV:
            return {vvl::Extension::_VK_EXT_mesh_shader};
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_PIPELINE_NV:
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_DISPATCH_NV:
            return {vvl::Extension::_VK_NV_device_generated_commands_compute};
        default:
            return {};
    };
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkDepthBiasRepresentationEXT value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkFragmentShadingRateTypeNV value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkFragmentShadingRateNV value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkAccelerationStructureMotionInstanceTypeNV value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkDeviceFaultAddressTypeEXT value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkDeviceFaultVendorBinaryHeaderVersionEXT value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkDeviceAddressBindingTypeEXT value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkMicromapTypeEXT value) const {
    switch (value) {
        case VK_MICROMAP_TYPE_DISPLACEMENT_MICROMAP_NV:
            return {vvl::Extension::_VK_NV_displacement_micromap};
        default:
            return {};
    };
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkBuildMicromapModeEXT value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkCopyMicromapModeEXT value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkAccelerationStructureCompatibilityKHR value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkAccelerationStructureBuildTypeKHR value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkDirectDriverLoadingModeLUNARG value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkOpticalFlowPerformanceLevelNV value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkOpticalFlowSessionBindingPointNV value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkShaderCodeTypeEXT value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkLayerSettingTypeEXT value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkLatencyMarkerNV value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkOutOfBandQueueTypeNV value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkBlockMatchWindowCompareModeQCOM value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkCubicFilterWeightsQCOM value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkBuildAccelerationStructureModeKHR value) const {
    return {};
}

template <>
vvl::Extensions StatelessValidation::GetEnumExtensions(VkShaderGroupShaderKHR value) const {
    return {};
}

// NOLINTEND
