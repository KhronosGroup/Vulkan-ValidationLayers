// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See valid_flag_values_generator.py for modifications

/***************************************************************************
 *
 * Copyright (c) 2024 The Khronos Group Inc.
 * Copyright (c) 2024 Valve Corporation
 * Copyright (c) 2024 LunarG, Inc.
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

// For flags, we can't use the VkFlag as it can't be templated (since it all resolves to a int).
// It is simpler for the caller to already check for both
//    - if zero is valid value or not
//    - if the value is even found in the API
// so the this file is only focused on checking for extensions being supported

vvl::Extensions StatelessValidation::IsValidFlagValue(vvl::FlagBitmask flag_bitmask, VkFlags value,
                                                      const DeviceExtensions& device_extensions) const {
    switch (flag_bitmask) {
        case vvl::FlagBitmask::VkAccessFlagBits:
            if (value & (VK_ACCESS_NONE)) {
                if (!IsExtEnabled(device_extensions.vk_khr_synchronization2)) {
                    return {vvl::Extension::_VK_KHR_synchronization2};
                }
            }
            if (value & (VK_ACCESS_TRANSFORM_FEEDBACK_WRITE_BIT_EXT | VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_READ_BIT_EXT |
                         VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT)) {
                if (!IsExtEnabled(device_extensions.vk_ext_transform_feedback)) {
                    return {vvl::Extension::_VK_EXT_transform_feedback};
                }
            }
            if (value & (VK_ACCESS_CONDITIONAL_RENDERING_READ_BIT_EXT)) {
                if (!IsExtEnabled(device_extensions.vk_ext_conditional_rendering)) {
                    return {vvl::Extension::_VK_EXT_conditional_rendering};
                }
            }
            if (value & (VK_ACCESS_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT)) {
                if (!IsExtEnabled(device_extensions.vk_ext_blend_operation_advanced)) {
                    return {vvl::Extension::_VK_EXT_blend_operation_advanced};
                }
            }
            if (value & (VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR | VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR)) {
                if (!IsExtEnabled(device_extensions.vk_nv_ray_tracing) &&
                    !IsExtEnabled(device_extensions.vk_khr_acceleration_structure)) {
                    return {vvl::Extension::_VK_NV_ray_tracing, vvl::Extension::_VK_KHR_acceleration_structure};
                }
            }
            if (value & (VK_ACCESS_FRAGMENT_DENSITY_MAP_READ_BIT_EXT)) {
                if (!IsExtEnabled(device_extensions.vk_ext_fragment_density_map)) {
                    return {vvl::Extension::_VK_EXT_fragment_density_map};
                }
            }
            if (value & (VK_ACCESS_FRAGMENT_SHADING_RATE_ATTACHMENT_READ_BIT_KHR)) {
                if (!IsExtEnabled(device_extensions.vk_khr_fragment_shading_rate) &&
                    !IsExtEnabled(device_extensions.vk_nv_shading_rate_image)) {
                    return {vvl::Extension::_VK_KHR_fragment_shading_rate, vvl::Extension::_VK_NV_shading_rate_image};
                }
            }
            if (value & (VK_ACCESS_COMMAND_PREPROCESS_READ_BIT_NV | VK_ACCESS_COMMAND_PREPROCESS_WRITE_BIT_NV)) {
                if (!IsExtEnabled(device_extensions.vk_nv_device_generated_commands)) {
                    return {vvl::Extension::_VK_NV_device_generated_commands};
                }
            }
            return {};
        case vvl::FlagBitmask::VkImageAspectFlagBits:
            if (value & (VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT | VK_IMAGE_ASPECT_PLANE_2_BIT)) {
                if (!IsExtEnabled(device_extensions.vk_khr_sampler_ycbcr_conversion)) {
                    return {vvl::Extension::_VK_KHR_sampler_ycbcr_conversion};
                }
            }
            if (value & (VK_IMAGE_ASPECT_NONE)) {
                if (!IsExtEnabled(device_extensions.vk_khr_maintenance4)) {
                    return {vvl::Extension::_VK_KHR_maintenance4};
                }
            }
            if (value & (VK_IMAGE_ASPECT_MEMORY_PLANE_0_BIT_EXT | VK_IMAGE_ASPECT_MEMORY_PLANE_1_BIT_EXT |
                         VK_IMAGE_ASPECT_MEMORY_PLANE_2_BIT_EXT | VK_IMAGE_ASPECT_MEMORY_PLANE_3_BIT_EXT)) {
                if (!IsExtEnabled(device_extensions.vk_ext_image_drm_format_modifier)) {
                    return {vvl::Extension::_VK_EXT_image_drm_format_modifier};
                }
            }
            return {};
        case vvl::FlagBitmask::VkFormatFeatureFlagBits:
            if (value & (VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT)) {
                if (!IsExtEnabled(device_extensions.vk_khr_maintenance1)) {
                    return {vvl::Extension::_VK_KHR_maintenance1};
                }
            }
            if (value & (VK_FORMAT_FEATURE_MIDPOINT_CHROMA_SAMPLES_BIT |
                         VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT |
                         VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT |
                         VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_BIT |
                         VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_FORCEABLE_BIT |
                         VK_FORMAT_FEATURE_DISJOINT_BIT | VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT)) {
                if (!IsExtEnabled(device_extensions.vk_khr_sampler_ycbcr_conversion)) {
                    return {vvl::Extension::_VK_KHR_sampler_ycbcr_conversion};
                }
            }
            if (value & (VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_MINMAX_BIT)) {
                if (!IsExtEnabled(device_extensions.vk_ext_sampler_filter_minmax)) {
                    return {vvl::Extension::_VK_EXT_sampler_filter_minmax};
                }
            }
            if (value & (VK_FORMAT_FEATURE_VIDEO_DECODE_OUTPUT_BIT_KHR | VK_FORMAT_FEATURE_VIDEO_DECODE_DPB_BIT_KHR)) {
                if (!IsExtEnabled(device_extensions.vk_khr_video_decode_queue)) {
                    return {vvl::Extension::_VK_KHR_video_decode_queue};
                }
            }
            if (value & (VK_FORMAT_FEATURE_ACCELERATION_STRUCTURE_VERTEX_BUFFER_BIT_KHR)) {
                if (!IsExtEnabled(device_extensions.vk_khr_acceleration_structure)) {
                    return {vvl::Extension::_VK_KHR_acceleration_structure};
                }
            }
            if (value & (VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_CUBIC_BIT_EXT)) {
                if (!IsExtEnabled(device_extensions.vk_img_filter_cubic) && !IsExtEnabled(device_extensions.vk_ext_filter_cubic)) {
                    return {vvl::Extension::_VK_IMG_filter_cubic, vvl::Extension::_VK_EXT_filter_cubic};
                }
            }
            if (value & (VK_FORMAT_FEATURE_FRAGMENT_DENSITY_MAP_BIT_EXT)) {
                if (!IsExtEnabled(device_extensions.vk_ext_fragment_density_map)) {
                    return {vvl::Extension::_VK_EXT_fragment_density_map};
                }
            }
            if (value & (VK_FORMAT_FEATURE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR)) {
                if (!IsExtEnabled(device_extensions.vk_khr_fragment_shading_rate)) {
                    return {vvl::Extension::_VK_KHR_fragment_shading_rate};
                }
            }
            if (value & (VK_FORMAT_FEATURE_VIDEO_ENCODE_INPUT_BIT_KHR | VK_FORMAT_FEATURE_VIDEO_ENCODE_DPB_BIT_KHR)) {
                if (!IsExtEnabled(device_extensions.vk_khr_video_encode_queue)) {
                    return {vvl::Extension::_VK_KHR_video_encode_queue};
                }
            }
            return {};
        case vvl::FlagBitmask::VkImageCreateFlagBits:
            if (value & (VK_IMAGE_CREATE_ALIAS_BIT)) {
                if (!IsExtEnabled(device_extensions.vk_khr_bind_memory2)) {
                    return {vvl::Extension::_VK_KHR_bind_memory2};
                }
            }
            if (value & (VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT)) {
                if (!IsExtEnabled(device_extensions.vk_khr_device_group)) {
                    return {vvl::Extension::_VK_KHR_device_group};
                }
            }
            if (value & (VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT)) {
                if (!IsExtEnabled(device_extensions.vk_khr_maintenance1)) {
                    return {vvl::Extension::_VK_KHR_maintenance1};
                }
            }
            if (value & (VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT | VK_IMAGE_CREATE_EXTENDED_USAGE_BIT)) {
                if (!IsExtEnabled(device_extensions.vk_khr_maintenance2)) {
                    return {vvl::Extension::_VK_KHR_maintenance2};
                }
            }
            if (value & (VK_IMAGE_CREATE_DISJOINT_BIT)) {
                if (!IsExtEnabled(device_extensions.vk_khr_sampler_ycbcr_conversion)) {
                    return {vvl::Extension::_VK_KHR_sampler_ycbcr_conversion};
                }
            }
            if (value & (VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV)) {
                if (!IsExtEnabled(device_extensions.vk_nv_corner_sampled_image)) {
                    return {vvl::Extension::_VK_NV_corner_sampled_image};
                }
            }
            if (value & (VK_IMAGE_CREATE_SAMPLE_LOCATIONS_COMPATIBLE_DEPTH_BIT_EXT)) {
                if (!IsExtEnabled(device_extensions.vk_ext_sample_locations)) {
                    return {vvl::Extension::_VK_EXT_sample_locations};
                }
            }
            if (value & (VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT)) {
                if (!IsExtEnabled(device_extensions.vk_ext_fragment_density_map)) {
                    return {vvl::Extension::_VK_EXT_fragment_density_map};
                }
            }
            if (value & (VK_IMAGE_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT)) {
                if (!IsExtEnabled(device_extensions.vk_ext_descriptor_buffer)) {
                    return {vvl::Extension::_VK_EXT_descriptor_buffer};
                }
            }
            if (value & (VK_IMAGE_CREATE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_BIT_EXT)) {
                if (!IsExtEnabled(device_extensions.vk_ext_multisampled_render_to_single_sampled)) {
                    return {vvl::Extension::_VK_EXT_multisampled_render_to_single_sampled};
                }
            }
            if (value & (VK_IMAGE_CREATE_2D_VIEW_COMPATIBLE_BIT_EXT)) {
                if (!IsExtEnabled(device_extensions.vk_ext_image_2d_view_of_3d)) {
                    return {vvl::Extension::_VK_EXT_image_2d_view_of_3d};
                }
            }
            if (value & (VK_IMAGE_CREATE_FRAGMENT_DENSITY_MAP_OFFSET_BIT_QCOM)) {
                if (!IsExtEnabled(device_extensions.vk_qcom_fragment_density_map_offset)) {
                    return {vvl::Extension::_VK_QCOM_fragment_density_map_offset};
                }
            }
            if (value & (VK_IMAGE_CREATE_VIDEO_PROFILE_INDEPENDENT_BIT_KHR)) {
                if (!IsExtEnabled(device_extensions.vk_khr_video_maintenance1)) {
                    return {vvl::Extension::_VK_KHR_video_maintenance1};
                }
            }
            return {};
        case vvl::FlagBitmask::VkImageUsageFlagBits:
            if (value & (VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR | VK_IMAGE_USAGE_VIDEO_DECODE_SRC_BIT_KHR |
                         VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR)) {
                if (!IsExtEnabled(device_extensions.vk_khr_video_decode_queue)) {
                    return {vvl::Extension::_VK_KHR_video_decode_queue};
                }
            }
            if (value & (VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT)) {
                if (!IsExtEnabled(device_extensions.vk_ext_fragment_density_map)) {
                    return {vvl::Extension::_VK_EXT_fragment_density_map};
                }
            }
            if (value & (VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR)) {
                if (!IsExtEnabled(device_extensions.vk_khr_fragment_shading_rate) &&
                    !IsExtEnabled(device_extensions.vk_nv_shading_rate_image)) {
                    return {vvl::Extension::_VK_KHR_fragment_shading_rate, vvl::Extension::_VK_NV_shading_rate_image};
                }
            }
            if (value & (VK_IMAGE_USAGE_HOST_TRANSFER_BIT_EXT)) {
                if (!IsExtEnabled(device_extensions.vk_ext_host_image_copy)) {
                    return {vvl::Extension::_VK_EXT_host_image_copy};
                }
            }
            if (value & (VK_IMAGE_USAGE_VIDEO_ENCODE_DST_BIT_KHR | VK_IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT_KHR |
                         VK_IMAGE_USAGE_VIDEO_ENCODE_DPB_BIT_KHR)) {
                if (!IsExtEnabled(device_extensions.vk_khr_video_encode_queue)) {
                    return {vvl::Extension::_VK_KHR_video_encode_queue};
                }
            }
            if (value & (VK_IMAGE_USAGE_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT)) {
                if (!IsExtEnabled(device_extensions.vk_ext_attachment_feedback_loop_layout)) {
                    return {vvl::Extension::_VK_EXT_attachment_feedback_loop_layout};
                }
            }
            if (value & (VK_IMAGE_USAGE_INVOCATION_MASK_BIT_HUAWEI)) {
                if (!IsExtEnabled(device_extensions.vk_huawei_invocation_mask)) {
                    return {vvl::Extension::_VK_HUAWEI_invocation_mask};
                }
            }
            if (value & (VK_IMAGE_USAGE_SAMPLE_WEIGHT_BIT_QCOM | VK_IMAGE_USAGE_SAMPLE_BLOCK_MATCH_BIT_QCOM)) {
                if (!IsExtEnabled(device_extensions.vk_qcom_image_processing)) {
                    return {vvl::Extension::_VK_QCOM_image_processing};
                }
            }
            return {};
        case vvl::FlagBitmask::VkPipelineStageFlagBits:
            if (value & (VK_PIPELINE_STAGE_NONE)) {
                if (!IsExtEnabled(device_extensions.vk_khr_synchronization2)) {
                    return {vvl::Extension::_VK_KHR_synchronization2};
                }
            }
            if (value & (VK_PIPELINE_STAGE_TRANSFORM_FEEDBACK_BIT_EXT)) {
                if (!IsExtEnabled(device_extensions.vk_ext_transform_feedback)) {
                    return {vvl::Extension::_VK_EXT_transform_feedback};
                }
            }
            if (value & (VK_PIPELINE_STAGE_CONDITIONAL_RENDERING_BIT_EXT)) {
                if (!IsExtEnabled(device_extensions.vk_ext_conditional_rendering)) {
                    return {vvl::Extension::_VK_EXT_conditional_rendering};
                }
            }
            if (value & (VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR)) {
                if (!IsExtEnabled(device_extensions.vk_nv_ray_tracing) &&
                    !IsExtEnabled(device_extensions.vk_khr_acceleration_structure)) {
                    return {vvl::Extension::_VK_NV_ray_tracing, vvl::Extension::_VK_KHR_acceleration_structure};
                }
            }
            if (value & (VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR)) {
                if (!IsExtEnabled(device_extensions.vk_nv_ray_tracing) &&
                    !IsExtEnabled(device_extensions.vk_khr_ray_tracing_pipeline)) {
                    return {vvl::Extension::_VK_NV_ray_tracing, vvl::Extension::_VK_KHR_ray_tracing_pipeline};
                }
            }
            if (value & (VK_PIPELINE_STAGE_FRAGMENT_DENSITY_PROCESS_BIT_EXT)) {
                if (!IsExtEnabled(device_extensions.vk_ext_fragment_density_map)) {
                    return {vvl::Extension::_VK_EXT_fragment_density_map};
                }
            }
            if (value & (VK_PIPELINE_STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR)) {
                if (!IsExtEnabled(device_extensions.vk_khr_fragment_shading_rate) &&
                    !IsExtEnabled(device_extensions.vk_nv_shading_rate_image)) {
                    return {vvl::Extension::_VK_KHR_fragment_shading_rate, vvl::Extension::_VK_NV_shading_rate_image};
                }
            }
            if (value & (VK_PIPELINE_STAGE_COMMAND_PREPROCESS_BIT_NV)) {
                if (!IsExtEnabled(device_extensions.vk_nv_device_generated_commands)) {
                    return {vvl::Extension::_VK_NV_device_generated_commands};
                }
            }
            if (value & (VK_PIPELINE_STAGE_TASK_SHADER_BIT_EXT | VK_PIPELINE_STAGE_MESH_SHADER_BIT_EXT)) {
                if (!IsExtEnabled(device_extensions.vk_nv_mesh_shader) && !IsExtEnabled(device_extensions.vk_ext_mesh_shader)) {
                    return {vvl::Extension::_VK_NV_mesh_shader, vvl::Extension::_VK_EXT_mesh_shader};
                }
            }
            return {};
        case vvl::FlagBitmask::VkMemoryMapFlagBits:
            if (value & (VK_MEMORY_MAP_PLACED_BIT_EXT)) {
                if (!IsExtEnabled(device_extensions.vk_ext_map_memory_placed)) {
                    return {vvl::Extension::_VK_EXT_map_memory_placed};
                }
            }
            return {};
        case vvl::FlagBitmask::VkEventCreateFlagBits:
            if (value & (VK_EVENT_CREATE_DEVICE_ONLY_BIT)) {
                if (!IsExtEnabled(device_extensions.vk_khr_synchronization2)) {
                    return {vvl::Extension::_VK_KHR_synchronization2};
                }
            }
            return {};
        case vvl::FlagBitmask::VkQueryPipelineStatisticFlagBits:
            if (value & (VK_QUERY_PIPELINE_STATISTIC_TASK_SHADER_INVOCATIONS_BIT_EXT |
                         VK_QUERY_PIPELINE_STATISTIC_MESH_SHADER_INVOCATIONS_BIT_EXT)) {
                if (!IsExtEnabled(device_extensions.vk_ext_mesh_shader)) {
                    return {vvl::Extension::_VK_EXT_mesh_shader};
                }
            }
            if (value & (VK_QUERY_PIPELINE_STATISTIC_CLUSTER_CULLING_SHADER_INVOCATIONS_BIT_HUAWEI)) {
                if (!IsExtEnabled(device_extensions.vk_huawei_cluster_culling_shader)) {
                    return {vvl::Extension::_VK_HUAWEI_cluster_culling_shader};
                }
            }
            return {};
        case vvl::FlagBitmask::VkQueryResultFlagBits:
            if (value & (VK_QUERY_RESULT_WITH_STATUS_BIT_KHR)) {
                if (!IsExtEnabled(device_extensions.vk_khr_video_queue)) {
                    return {vvl::Extension::_VK_KHR_video_queue};
                }
            }
            return {};
        case vvl::FlagBitmask::VkBufferCreateFlagBits:
            if (value & (VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT)) {
                if (!IsExtEnabled(device_extensions.vk_khr_buffer_device_address) &&
                    !IsExtEnabled(device_extensions.vk_ext_buffer_device_address)) {
                    return {vvl::Extension::_VK_KHR_buffer_device_address, vvl::Extension::_VK_EXT_buffer_device_address};
                }
            }
            if (value & (VK_BUFFER_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT)) {
                if (!IsExtEnabled(device_extensions.vk_ext_descriptor_buffer)) {
                    return {vvl::Extension::_VK_EXT_descriptor_buffer};
                }
            }
            if (value & (VK_BUFFER_CREATE_VIDEO_PROFILE_INDEPENDENT_BIT_KHR)) {
                if (!IsExtEnabled(device_extensions.vk_khr_video_maintenance1)) {
                    return {vvl::Extension::_VK_KHR_video_maintenance1};
                }
            }
            return {};
        case vvl::FlagBitmask::VkBufferUsageFlagBits:
            if (value & (VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)) {
                if (!IsExtEnabled(device_extensions.vk_khr_buffer_device_address) &&
                    !IsExtEnabled(device_extensions.vk_ext_buffer_device_address)) {
                    return {vvl::Extension::_VK_KHR_buffer_device_address, vvl::Extension::_VK_EXT_buffer_device_address};
                }
            }
            if (value & (VK_BUFFER_USAGE_VIDEO_DECODE_SRC_BIT_KHR | VK_BUFFER_USAGE_VIDEO_DECODE_DST_BIT_KHR)) {
                if (!IsExtEnabled(device_extensions.vk_khr_video_decode_queue)) {
                    return {vvl::Extension::_VK_KHR_video_decode_queue};
                }
            }
            if (value &
                (VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT | VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT)) {
                if (!IsExtEnabled(device_extensions.vk_ext_transform_feedback)) {
                    return {vvl::Extension::_VK_EXT_transform_feedback};
                }
            }
            if (value & (VK_BUFFER_USAGE_CONDITIONAL_RENDERING_BIT_EXT)) {
                if (!IsExtEnabled(device_extensions.vk_ext_conditional_rendering)) {
                    return {vvl::Extension::_VK_EXT_conditional_rendering};
                }
            }
            if (value & (VK_BUFFER_USAGE_EXECUTION_GRAPH_SCRATCH_BIT_AMDX)) {
                if (!IsExtEnabled(device_extensions.vk_amdx_shader_enqueue)) {
                    return {vvl::Extension::_VK_AMDX_shader_enqueue};
                }
            }
            if (value & (VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                         VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR)) {
                if (!IsExtEnabled(device_extensions.vk_khr_acceleration_structure)) {
                    return {vvl::Extension::_VK_KHR_acceleration_structure};
                }
            }
            if (value & (VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR)) {
                if (!IsExtEnabled(device_extensions.vk_nv_ray_tracing) &&
                    !IsExtEnabled(device_extensions.vk_khr_ray_tracing_pipeline)) {
                    return {vvl::Extension::_VK_NV_ray_tracing, vvl::Extension::_VK_KHR_ray_tracing_pipeline};
                }
            }
            if (value & (VK_BUFFER_USAGE_VIDEO_ENCODE_DST_BIT_KHR | VK_BUFFER_USAGE_VIDEO_ENCODE_SRC_BIT_KHR)) {
                if (!IsExtEnabled(device_extensions.vk_khr_video_encode_queue)) {
                    return {vvl::Extension::_VK_KHR_video_encode_queue};
                }
            }
            if (value & (VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT |
                         VK_BUFFER_USAGE_PUSH_DESCRIPTORS_DESCRIPTOR_BUFFER_BIT_EXT)) {
                if (!IsExtEnabled(device_extensions.vk_ext_descriptor_buffer)) {
                    return {vvl::Extension::_VK_EXT_descriptor_buffer};
                }
            }
            if (value & (VK_BUFFER_USAGE_MICROMAP_BUILD_INPUT_READ_ONLY_BIT_EXT | VK_BUFFER_USAGE_MICROMAP_STORAGE_BIT_EXT)) {
                if (!IsExtEnabled(device_extensions.vk_ext_opacity_micromap)) {
                    return {vvl::Extension::_VK_EXT_opacity_micromap};
                }
            }
            return {};
        case vvl::FlagBitmask::VkImageViewCreateFlagBits:
            if (value & (VK_IMAGE_VIEW_CREATE_FRAGMENT_DENSITY_MAP_DYNAMIC_BIT_EXT)) {
                if (!IsExtEnabled(device_extensions.vk_ext_fragment_density_map)) {
                    return {vvl::Extension::_VK_EXT_fragment_density_map};
                }
            }
            if (value & (VK_IMAGE_VIEW_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT)) {
                if (!IsExtEnabled(device_extensions.vk_ext_descriptor_buffer)) {
                    return {vvl::Extension::_VK_EXT_descriptor_buffer};
                }
            }
            if (value & (VK_IMAGE_VIEW_CREATE_FRAGMENT_DENSITY_MAP_DEFERRED_BIT_EXT)) {
                if (!IsExtEnabled(device_extensions.vk_ext_fragment_density_map2)) {
                    return {vvl::Extension::_VK_EXT_fragment_density_map2};
                }
            }
            return {};
        case vvl::FlagBitmask::VkPipelineCacheCreateFlagBits:
            if (value & (VK_PIPELINE_CACHE_CREATE_EXTERNALLY_SYNCHRONIZED_BIT)) {
                if (!IsExtEnabled(device_extensions.vk_ext_pipeline_creation_cache_control)) {
                    return {vvl::Extension::_VK_EXT_pipeline_creation_cache_control};
                }
            }
            return {};
        case vvl::FlagBitmask::VkPipelineCreateFlagBits:
            if (value & (VK_PIPELINE_CREATE_VIEW_INDEX_FROM_DEVICE_INDEX_BIT)) {
                if (!IsExtEnabled(device_extensions.vk_khr_device_group)) {
                    return {vvl::Extension::_VK_KHR_device_group};
                }
            }
            if (value &
                (VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT | VK_PIPELINE_CREATE_EARLY_RETURN_ON_FAILURE_BIT)) {
                if (!IsExtEnabled(device_extensions.vk_ext_pipeline_creation_cache_control)) {
                    return {vvl::Extension::_VK_EXT_pipeline_creation_cache_control};
                }
            }
            if (value & (VK_PIPELINE_CREATE_RENDERING_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR |
                         VK_PIPELINE_CREATE_RENDERING_FRAGMENT_DENSITY_MAP_ATTACHMENT_BIT_EXT)) {
                if (!IsExtEnabled(device_extensions.vk_khr_dynamic_rendering)) {
                    return {vvl::Extension::_VK_KHR_dynamic_rendering};
                }
            }
            if (value & (VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_ANY_HIT_SHADERS_BIT_KHR |
                         VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_CLOSEST_HIT_SHADERS_BIT_KHR |
                         VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_MISS_SHADERS_BIT_KHR |
                         VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_INTERSECTION_SHADERS_BIT_KHR |
                         VK_PIPELINE_CREATE_RAY_TRACING_SKIP_TRIANGLES_BIT_KHR | VK_PIPELINE_CREATE_RAY_TRACING_SKIP_AABBS_BIT_KHR |
                         VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR)) {
                if (!IsExtEnabled(device_extensions.vk_khr_ray_tracing_pipeline)) {
                    return {vvl::Extension::_VK_KHR_ray_tracing_pipeline};
                }
            }
            if (value & (VK_PIPELINE_CREATE_DEFER_COMPILE_BIT_NV)) {
                if (!IsExtEnabled(device_extensions.vk_nv_ray_tracing)) {
                    return {vvl::Extension::_VK_NV_ray_tracing};
                }
            }
            if (value &
                (VK_PIPELINE_CREATE_CAPTURE_STATISTICS_BIT_KHR | VK_PIPELINE_CREATE_CAPTURE_INTERNAL_REPRESENTATIONS_BIT_KHR)) {
                if (!IsExtEnabled(device_extensions.vk_khr_pipeline_executable_properties)) {
                    return {vvl::Extension::_VK_KHR_pipeline_executable_properties};
                }
            }
            if (value & (VK_PIPELINE_CREATE_INDIRECT_BINDABLE_BIT_NV)) {
                if (!IsExtEnabled(device_extensions.vk_nv_device_generated_commands)) {
                    return {vvl::Extension::_VK_NV_device_generated_commands};
                }
            }
            if (value & (VK_PIPELINE_CREATE_LIBRARY_BIT_KHR)) {
                if (!IsExtEnabled(device_extensions.vk_khr_pipeline_library)) {
                    return {vvl::Extension::_VK_KHR_pipeline_library};
                }
            }
            if (value & (VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT)) {
                if (!IsExtEnabled(device_extensions.vk_ext_descriptor_buffer)) {
                    return {vvl::Extension::_VK_EXT_descriptor_buffer};
                }
            }
            if (value & (VK_PIPELINE_CREATE_RETAIN_LINK_TIME_OPTIMIZATION_INFO_BIT_EXT |
                         VK_PIPELINE_CREATE_LINK_TIME_OPTIMIZATION_BIT_EXT)) {
                if (!IsExtEnabled(device_extensions.vk_ext_graphics_pipeline_library)) {
                    return {vvl::Extension::_VK_EXT_graphics_pipeline_library};
                }
            }
            if (value & (VK_PIPELINE_CREATE_RAY_TRACING_ALLOW_MOTION_BIT_NV)) {
                if (!IsExtEnabled(device_extensions.vk_nv_ray_tracing_motion_blur)) {
                    return {vvl::Extension::_VK_NV_ray_tracing_motion_blur};
                }
            }
            if (value & (VK_PIPELINE_CREATE_COLOR_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT |
                         VK_PIPELINE_CREATE_DEPTH_STENCIL_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT)) {
                if (!IsExtEnabled(device_extensions.vk_ext_attachment_feedback_loop_layout)) {
                    return {vvl::Extension::_VK_EXT_attachment_feedback_loop_layout};
                }
            }
            if (value & (VK_PIPELINE_CREATE_RAY_TRACING_OPACITY_MICROMAP_BIT_EXT)) {
                if (!IsExtEnabled(device_extensions.vk_ext_opacity_micromap)) {
                    return {vvl::Extension::_VK_EXT_opacity_micromap};
                }
            }
            if (value & (VK_PIPELINE_CREATE_RAY_TRACING_DISPLACEMENT_MICROMAP_BIT_NV)) {
                if (!IsExtEnabled(device_extensions.vk_nv_displacement_micromap)) {
                    return {vvl::Extension::_VK_NV_displacement_micromap};
                }
            }
            if (value & (VK_PIPELINE_CREATE_NO_PROTECTED_ACCESS_BIT_EXT | VK_PIPELINE_CREATE_PROTECTED_ACCESS_ONLY_BIT_EXT)) {
                if (!IsExtEnabled(device_extensions.vk_ext_pipeline_protected_access)) {
                    return {vvl::Extension::_VK_EXT_pipeline_protected_access};
                }
            }
            return {};
        case vvl::FlagBitmask::VkPipelineShaderStageCreateFlagBits:
            if (value & (VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT |
                         VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT)) {
                if (!IsExtEnabled(device_extensions.vk_ext_subgroup_size_control)) {
                    return {vvl::Extension::_VK_EXT_subgroup_size_control};
                }
            }
            return {};
        case vvl::FlagBitmask::VkShaderStageFlagBits:
            if (value == VK_SHADER_STAGE_ALL_GRAPHICS) {
                return {};
            }
            if (value == VK_SHADER_STAGE_ALL) {
                return {};
            }
            if (value & (VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_ANY_HIT_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR |
                         VK_SHADER_STAGE_MISS_BIT_KHR | VK_SHADER_STAGE_INTERSECTION_BIT_KHR | VK_SHADER_STAGE_CALLABLE_BIT_KHR)) {
                if (!IsExtEnabled(device_extensions.vk_nv_ray_tracing) &&
                    !IsExtEnabled(device_extensions.vk_khr_ray_tracing_pipeline)) {
                    return {vvl::Extension::_VK_NV_ray_tracing, vvl::Extension::_VK_KHR_ray_tracing_pipeline};
                }
            }
            if (value & (VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT)) {
                if (!IsExtEnabled(device_extensions.vk_nv_mesh_shader) && !IsExtEnabled(device_extensions.vk_ext_mesh_shader)) {
                    return {vvl::Extension::_VK_NV_mesh_shader, vvl::Extension::_VK_EXT_mesh_shader};
                }
            }
            if (value & (VK_SHADER_STAGE_SUBPASS_SHADING_BIT_HUAWEI)) {
                if (!IsExtEnabled(device_extensions.vk_huawei_subpass_shading)) {
                    return {vvl::Extension::_VK_HUAWEI_subpass_shading};
                }
            }
            if (value & (VK_SHADER_STAGE_CLUSTER_CULLING_BIT_HUAWEI)) {
                if (!IsExtEnabled(device_extensions.vk_huawei_cluster_culling_shader)) {
                    return {vvl::Extension::_VK_HUAWEI_cluster_culling_shader};
                }
            }
            return {};
        case vvl::FlagBitmask::VkPipelineDepthStencilStateCreateFlagBits:
            if (value & (VK_PIPELINE_DEPTH_STENCIL_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_DEPTH_ACCESS_BIT_EXT |
                         VK_PIPELINE_DEPTH_STENCIL_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_STENCIL_ACCESS_BIT_EXT)) {
                if (!IsExtEnabled(device_extensions.vk_arm_rasterization_order_attachment_access) &&
                    !IsExtEnabled(device_extensions.vk_ext_rasterization_order_attachment_access)) {
                    return {vvl::Extension::_VK_ARM_rasterization_order_attachment_access,
                            vvl::Extension::_VK_EXT_rasterization_order_attachment_access};
                }
            }
            return {};
        case vvl::FlagBitmask::VkPipelineColorBlendStateCreateFlagBits:
            if (value & (VK_PIPELINE_COLOR_BLEND_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_BIT_EXT)) {
                if (!IsExtEnabled(device_extensions.vk_arm_rasterization_order_attachment_access) &&
                    !IsExtEnabled(device_extensions.vk_ext_rasterization_order_attachment_access)) {
                    return {vvl::Extension::_VK_ARM_rasterization_order_attachment_access,
                            vvl::Extension::_VK_EXT_rasterization_order_attachment_access};
                }
            }
            return {};
        case vvl::FlagBitmask::VkPipelineLayoutCreateFlagBits:
            if (value & (VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT)) {
                if (!IsExtEnabled(device_extensions.vk_ext_graphics_pipeline_library)) {
                    return {vvl::Extension::_VK_EXT_graphics_pipeline_library};
                }
            }
            return {};
        case vvl::FlagBitmask::VkSamplerCreateFlagBits:
            if (value & (VK_SAMPLER_CREATE_SUBSAMPLED_BIT_EXT | VK_SAMPLER_CREATE_SUBSAMPLED_COARSE_RECONSTRUCTION_BIT_EXT)) {
                if (!IsExtEnabled(device_extensions.vk_ext_fragment_density_map)) {
                    return {vvl::Extension::_VK_EXT_fragment_density_map};
                }
            }
            if (value & (VK_SAMPLER_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT)) {
                if (!IsExtEnabled(device_extensions.vk_ext_descriptor_buffer)) {
                    return {vvl::Extension::_VK_EXT_descriptor_buffer};
                }
            }
            if (value & (VK_SAMPLER_CREATE_NON_SEAMLESS_CUBE_MAP_BIT_EXT)) {
                if (!IsExtEnabled(device_extensions.vk_ext_non_seamless_cube_map)) {
                    return {vvl::Extension::_VK_EXT_non_seamless_cube_map};
                }
            }
            if (value & (VK_SAMPLER_CREATE_IMAGE_PROCESSING_BIT_QCOM)) {
                if (!IsExtEnabled(device_extensions.vk_qcom_image_processing)) {
                    return {vvl::Extension::_VK_QCOM_image_processing};
                }
            }
            return {};
        case vvl::FlagBitmask::VkDescriptorPoolCreateFlagBits:
            if (value & (VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT)) {
                if (!IsExtEnabled(device_extensions.vk_ext_descriptor_indexing)) {
                    return {vvl::Extension::_VK_EXT_descriptor_indexing};
                }
            }
            if (value & (VK_DESCRIPTOR_POOL_CREATE_HOST_ONLY_BIT_EXT)) {
                if (!IsExtEnabled(device_extensions.vk_valve_mutable_descriptor_type) &&
                    !IsExtEnabled(device_extensions.vk_ext_mutable_descriptor_type)) {
                    return {vvl::Extension::_VK_VALVE_mutable_descriptor_type, vvl::Extension::_VK_EXT_mutable_descriptor_type};
                }
            }
            if (value & (VK_DESCRIPTOR_POOL_CREATE_ALLOW_OVERALLOCATION_SETS_BIT_NV |
                         VK_DESCRIPTOR_POOL_CREATE_ALLOW_OVERALLOCATION_POOLS_BIT_NV)) {
                if (!IsExtEnabled(device_extensions.vk_nv_descriptor_pool_overallocation)) {
                    return {vvl::Extension::_VK_NV_descriptor_pool_overallocation};
                }
            }
            return {};
        case vvl::FlagBitmask::VkDescriptorSetLayoutCreateFlagBits:
            if (value & (VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT)) {
                if (!IsExtEnabled(device_extensions.vk_ext_descriptor_indexing)) {
                    return {vvl::Extension::_VK_EXT_descriptor_indexing};
                }
            }
            if (value & (VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR)) {
                if (!IsExtEnabled(device_extensions.vk_khr_push_descriptor)) {
                    return {vvl::Extension::_VK_KHR_push_descriptor};
                }
            }
            if (value & (VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT |
                         VK_DESCRIPTOR_SET_LAYOUT_CREATE_EMBEDDED_IMMUTABLE_SAMPLERS_BIT_EXT)) {
                if (!IsExtEnabled(device_extensions.vk_ext_descriptor_buffer)) {
                    return {vvl::Extension::_VK_EXT_descriptor_buffer};
                }
            }
            if (value & (VK_DESCRIPTOR_SET_LAYOUT_CREATE_INDIRECT_BINDABLE_BIT_NV)) {
                if (!IsExtEnabled(device_extensions.vk_nv_device_generated_commands_compute)) {
                    return {vvl::Extension::_VK_NV_device_generated_commands_compute};
                }
            }
            if (value & (VK_DESCRIPTOR_SET_LAYOUT_CREATE_HOST_ONLY_POOL_BIT_EXT)) {
                if (!IsExtEnabled(device_extensions.vk_valve_mutable_descriptor_type) &&
                    !IsExtEnabled(device_extensions.vk_ext_mutable_descriptor_type)) {
                    return {vvl::Extension::_VK_VALVE_mutable_descriptor_type, vvl::Extension::_VK_EXT_mutable_descriptor_type};
                }
            }
            if (value & (VK_DESCRIPTOR_SET_LAYOUT_CREATE_PER_STAGE_BIT_NV)) {
                if (!IsExtEnabled(device_extensions.vk_nv_per_stage_descriptor_set)) {
                    return {vvl::Extension::_VK_NV_per_stage_descriptor_set};
                }
            }
            return {};
        case vvl::FlagBitmask::VkDependencyFlagBits:
            if (value & (VK_DEPENDENCY_DEVICE_GROUP_BIT)) {
                if (!IsExtEnabled(device_extensions.vk_khr_device_group)) {
                    return {vvl::Extension::_VK_KHR_device_group};
                }
            }
            if (value & (VK_DEPENDENCY_VIEW_LOCAL_BIT)) {
                if (!IsExtEnabled(device_extensions.vk_khr_multiview)) {
                    return {vvl::Extension::_VK_KHR_multiview};
                }
            }
            if (value & (VK_DEPENDENCY_FEEDBACK_LOOP_BIT_EXT)) {
                if (!IsExtEnabled(device_extensions.vk_ext_attachment_feedback_loop_layout)) {
                    return {vvl::Extension::_VK_EXT_attachment_feedback_loop_layout};
                }
            }
            return {};
        case vvl::FlagBitmask::VkFramebufferCreateFlagBits:
            if (value & (VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT)) {
                if (!IsExtEnabled(device_extensions.vk_khr_imageless_framebuffer)) {
                    return {vvl::Extension::_VK_KHR_imageless_framebuffer};
                }
            }
            return {};
        case vvl::FlagBitmask::VkRenderPassCreateFlagBits:
            if (value & (VK_RENDER_PASS_CREATE_TRANSFORM_BIT_QCOM)) {
                if (!IsExtEnabled(device_extensions.vk_qcom_render_pass_transform)) {
                    return {vvl::Extension::_VK_QCOM_render_pass_transform};
                }
            }
            return {};
        case vvl::FlagBitmask::VkSubpassDescriptionFlagBits:
            if (value &
                (VK_SUBPASS_DESCRIPTION_PER_VIEW_ATTRIBUTES_BIT_NVX | VK_SUBPASS_DESCRIPTION_PER_VIEW_POSITION_X_ONLY_BIT_NVX)) {
                if (!IsExtEnabled(device_extensions.vk_nvx_multiview_per_view_attributes)) {
                    return {vvl::Extension::_VK_NVX_multiview_per_view_attributes};
                }
            }
            if (value & (VK_SUBPASS_DESCRIPTION_FRAGMENT_REGION_BIT_QCOM | VK_SUBPASS_DESCRIPTION_SHADER_RESOLVE_BIT_QCOM)) {
                if (!IsExtEnabled(device_extensions.vk_qcom_render_pass_shader_resolve)) {
                    return {vvl::Extension::_VK_QCOM_render_pass_shader_resolve};
                }
            }
            if (value & (VK_SUBPASS_DESCRIPTION_RASTERIZATION_ORDER_ATTACHMENT_COLOR_ACCESS_BIT_EXT |
                         VK_SUBPASS_DESCRIPTION_RASTERIZATION_ORDER_ATTACHMENT_DEPTH_ACCESS_BIT_EXT |
                         VK_SUBPASS_DESCRIPTION_RASTERIZATION_ORDER_ATTACHMENT_STENCIL_ACCESS_BIT_EXT)) {
                if (!IsExtEnabled(device_extensions.vk_arm_rasterization_order_attachment_access) &&
                    !IsExtEnabled(device_extensions.vk_ext_rasterization_order_attachment_access)) {
                    return {vvl::Extension::_VK_ARM_rasterization_order_attachment_access,
                            vvl::Extension::_VK_EXT_rasterization_order_attachment_access};
                }
            }
            if (value & (VK_SUBPASS_DESCRIPTION_ENABLE_LEGACY_DITHERING_BIT_EXT)) {
                if (!IsExtEnabled(device_extensions.vk_ext_legacy_dithering)) {
                    return {vvl::Extension::_VK_EXT_legacy_dithering};
                }
            }
            return {};
        case vvl::FlagBitmask::VkMemoryAllocateFlagBits:
            if (value & (VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT | VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT)) {
                if (!IsExtEnabled(device_extensions.vk_khr_buffer_device_address)) {
                    return {vvl::Extension::_VK_KHR_buffer_device_address};
                }
            }
            return {};
        case vvl::FlagBitmask::VkExternalMemoryHandleTypeFlagBits:
            if (value & (VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT)) {
                if (!IsExtEnabled(device_extensions.vk_ext_external_memory_dma_buf)) {
                    return {vvl::Extension::_VK_EXT_external_memory_dma_buf};
                }
            }
            if (value & (VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID)) {
                if (!IsExtEnabled(device_extensions.vk_android_external_memory_android_hardware_buffer)) {
                    return {vvl::Extension::_VK_ANDROID_external_memory_android_hardware_buffer};
                }
            }
            if (value & (VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_ALLOCATION_BIT_EXT |
                         VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_MAPPED_FOREIGN_MEMORY_BIT_EXT)) {
                if (!IsExtEnabled(device_extensions.vk_ext_external_memory_host)) {
                    return {vvl::Extension::_VK_EXT_external_memory_host};
                }
            }
            if (value & (VK_EXTERNAL_MEMORY_HANDLE_TYPE_ZIRCON_VMO_BIT_FUCHSIA)) {
                if (!IsExtEnabled(device_extensions.vk_fuchsia_external_memory)) {
                    return {vvl::Extension::_VK_FUCHSIA_external_memory};
                }
            }
            if (value & (VK_EXTERNAL_MEMORY_HANDLE_TYPE_RDMA_ADDRESS_BIT_NV)) {
                if (!IsExtEnabled(device_extensions.vk_nv_external_memory_rdma)) {
                    return {vvl::Extension::_VK_NV_external_memory_rdma};
                }
            }
            if (value & (VK_EXTERNAL_MEMORY_HANDLE_TYPE_SCREEN_BUFFER_BIT_QNX)) {
                if (!IsExtEnabled(device_extensions.vk_qnx_external_memory_screen_buffer)) {
                    return {vvl::Extension::_VK_QNX_external_memory_screen_buffer};
                }
            }
            return {};
        case vvl::FlagBitmask::VkExternalSemaphoreHandleTypeFlagBits:
            if (value & (VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_ZIRCON_EVENT_BIT_FUCHSIA)) {
                if (!IsExtEnabled(device_extensions.vk_fuchsia_external_semaphore)) {
                    return {vvl::Extension::_VK_FUCHSIA_external_semaphore};
                }
            }
            return {};
        case vvl::FlagBitmask::VkResolveModeFlagBits:
            if (value & (VK_RESOLVE_MODE_EXTERNAL_FORMAT_DOWNSAMPLE_ANDROID)) {
                if (!IsExtEnabled(device_extensions.vk_android_external_format_resolve)) {
                    return {vvl::Extension::_VK_ANDROID_external_format_resolve};
                }
            }
            return {};
        case vvl::FlagBitmask::VkRenderingFlagBits:
            if (value & (VK_RENDERING_CONTENTS_INLINE_BIT_EXT)) {
                if (!IsExtEnabled(device_extensions.vk_ext_nested_command_buffer)) {
                    return {vvl::Extension::_VK_EXT_nested_command_buffer};
                }
            }
            if (value & (VK_RENDERING_ENABLE_LEGACY_DITHERING_BIT_EXT)) {
                if (!IsExtEnabled(device_extensions.vk_ext_legacy_dithering)) {
                    return {vvl::Extension::_VK_EXT_legacy_dithering};
                }
            }
            return {};
        case vvl::FlagBitmask::VkSwapchainCreateFlagBitsKHR:
            if (value & (VK_SWAPCHAIN_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT_KHR)) {
                if (!IsExtEnabled(device_extensions.vk_khr_device_group)) {
                    return {vvl::Extension::_VK_KHR_device_group};
                }
            }
            if (value & (VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR)) {
                if (!IsExtEnabled(device_extensions.vk_khr_swapchain_mutable_format)) {
                    return {vvl::Extension::_VK_KHR_swapchain_mutable_format};
                }
            }
            if (value & (VK_SWAPCHAIN_CREATE_DEFERRED_MEMORY_ALLOCATION_BIT_EXT)) {
                if (!IsExtEnabled(device_extensions.vk_ext_swapchain_maintenance1)) {
                    return {vvl::Extension::_VK_EXT_swapchain_maintenance1};
                }
            }
            return {};
        case vvl::FlagBitmask::VkVideoSessionCreateFlagBitsKHR:
            if (value & (VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_PARAMETER_OPTIMIZATIONS_BIT_KHR)) {
                if (!IsExtEnabled(device_extensions.vk_khr_video_encode_queue)) {
                    return {vvl::Extension::_VK_KHR_video_encode_queue};
                }
            }
            if (value & (VK_VIDEO_SESSION_CREATE_INLINE_QUERIES_BIT_KHR)) {
                if (!IsExtEnabled(device_extensions.vk_khr_video_maintenance1)) {
                    return {vvl::Extension::_VK_KHR_video_maintenance1};
                }
            }
            return {};
        case vvl::FlagBitmask::VkVideoCodingControlFlagBitsKHR:
            if (value &
                (VK_VIDEO_CODING_CONTROL_ENCODE_RATE_CONTROL_BIT_KHR | VK_VIDEO_CODING_CONTROL_ENCODE_QUALITY_LEVEL_BIT_KHR)) {
                if (!IsExtEnabled(device_extensions.vk_khr_video_encode_queue)) {
                    return {vvl::Extension::_VK_KHR_video_encode_queue};
                }
            }
            return {};
        case vvl::FlagBitmask::VkMemoryUnmapFlagBitsKHR:
            if (value & (VK_MEMORY_UNMAP_RESERVE_BIT_EXT)) {
                if (!IsExtEnabled(device_extensions.vk_ext_map_memory_placed)) {
                    return {vvl::Extension::_VK_EXT_map_memory_placed};
                }
            }
            return {};
        case vvl::FlagBitmask::VkDebugUtilsMessageTypeFlagBitsEXT:
            if (value & (VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT)) {
                if (!IsExtEnabled(device_extensions.vk_ext_device_address_binding_report)) {
                    return {vvl::Extension::_VK_EXT_device_address_binding_report};
                }
            }
            return {};
        case vvl::FlagBitmask::VkGeometryInstanceFlagBitsKHR:
            if (value &
                (VK_GEOMETRY_INSTANCE_FORCE_OPACITY_MICROMAP_2_STATE_EXT | VK_GEOMETRY_INSTANCE_DISABLE_OPACITY_MICROMAPS_EXT)) {
                if (!IsExtEnabled(device_extensions.vk_ext_opacity_micromap)) {
                    return {vvl::Extension::_VK_EXT_opacity_micromap};
                }
            }
            return {};
        case vvl::FlagBitmask::VkBuildAccelerationStructureFlagBitsKHR:
            if (value & (VK_BUILD_ACCELERATION_STRUCTURE_MOTION_BIT_NV)) {
                if (!IsExtEnabled(device_extensions.vk_nv_ray_tracing_motion_blur)) {
                    return {vvl::Extension::_VK_NV_ray_tracing_motion_blur};
                }
            }
            if (value & (VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_OPACITY_MICROMAP_UPDATE_EXT |
                         VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_DISABLE_OPACITY_MICROMAPS_EXT |
                         VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_OPACITY_MICROMAP_DATA_UPDATE_EXT)) {
                if (!IsExtEnabled(device_extensions.vk_ext_opacity_micromap)) {
                    return {vvl::Extension::_VK_EXT_opacity_micromap};
                }
            }
            if (value & (VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_DISPLACEMENT_MICROMAP_UPDATE_NV)) {
                if (!IsExtEnabled(device_extensions.vk_nv_displacement_micromap)) {
                    return {vvl::Extension::_VK_NV_displacement_micromap};
                }
            }
            if (value & (VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_DATA_ACCESS_KHR)) {
                if (!IsExtEnabled(device_extensions.vk_khr_ray_tracing_position_fetch)) {
                    return {vvl::Extension::_VK_KHR_ray_tracing_position_fetch};
                }
            }
            return {};
        case vvl::FlagBitmask::VkAccelerationStructureCreateFlagBitsKHR:
            if (value & (VK_ACCELERATION_STRUCTURE_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT)) {
                if (!IsExtEnabled(device_extensions.vk_ext_descriptor_buffer)) {
                    return {vvl::Extension::_VK_EXT_descriptor_buffer};
                }
            }
            if (value & (VK_ACCELERATION_STRUCTURE_CREATE_MOTION_BIT_NV)) {
                if (!IsExtEnabled(device_extensions.vk_nv_ray_tracing_motion_blur)) {
                    return {vvl::Extension::_VK_NV_ray_tracing_motion_blur};
                }
            }
            return {};
        default:
            return {};
    }
}

vvl::Extensions StatelessValidation::IsValidFlag64Value(vvl::FlagBitmask flag_bitmask, VkFlags64 value,
                                                        const DeviceExtensions& device_extensions) const {
    switch (flag_bitmask) {
        case vvl::FlagBitmask::VkPipelineStageFlagBits2:
            if (value & (VK_PIPELINE_STAGE_2_VIDEO_DECODE_BIT_KHR)) {
                if (!IsExtEnabled(device_extensions.vk_khr_video_decode_queue)) {
                    return {vvl::Extension::_VK_KHR_video_decode_queue};
                }
            }
            if (value & (VK_PIPELINE_STAGE_2_VIDEO_ENCODE_BIT_KHR)) {
                if (!IsExtEnabled(device_extensions.vk_khr_video_encode_queue)) {
                    return {vvl::Extension::_VK_KHR_video_encode_queue};
                }
            }
            if (value & (VK_PIPELINE_STAGE_2_SUBPASS_SHADER_BIT_HUAWEI)) {
                if (!IsExtEnabled(device_extensions.vk_huawei_subpass_shading)) {
                    return {vvl::Extension::_VK_HUAWEI_subpass_shading};
                }
            }
            if (value & (VK_PIPELINE_STAGE_2_INVOCATION_MASK_BIT_HUAWEI)) {
                if (!IsExtEnabled(device_extensions.vk_huawei_invocation_mask)) {
                    return {vvl::Extension::_VK_HUAWEI_invocation_mask};
                }
            }
            if (value & (VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_COPY_BIT_KHR)) {
                if (!IsExtEnabled(device_extensions.vk_khr_ray_tracing_maintenance1)) {
                    return {vvl::Extension::_VK_KHR_ray_tracing_maintenance1};
                }
            }
            if (value & (VK_PIPELINE_STAGE_2_MICROMAP_BUILD_BIT_EXT)) {
                if (!IsExtEnabled(device_extensions.vk_ext_opacity_micromap)) {
                    return {vvl::Extension::_VK_EXT_opacity_micromap};
                }
            }
            if (value & (VK_PIPELINE_STAGE_2_CLUSTER_CULLING_SHADER_BIT_HUAWEI)) {
                if (!IsExtEnabled(device_extensions.vk_huawei_cluster_culling_shader)) {
                    return {vvl::Extension::_VK_HUAWEI_cluster_culling_shader};
                }
            }
            if (value & (VK_PIPELINE_STAGE_2_OPTICAL_FLOW_BIT_NV)) {
                if (!IsExtEnabled(device_extensions.vk_nv_optical_flow)) {
                    return {vvl::Extension::_VK_NV_optical_flow};
                }
            }
            return {};
        case vvl::FlagBitmask::VkAccessFlagBits2:
            if (value & (VK_ACCESS_2_VIDEO_DECODE_READ_BIT_KHR | VK_ACCESS_2_VIDEO_DECODE_WRITE_BIT_KHR)) {
                if (!IsExtEnabled(device_extensions.vk_khr_video_decode_queue)) {
                    return {vvl::Extension::_VK_KHR_video_decode_queue};
                }
            }
            if (value & (VK_ACCESS_2_VIDEO_ENCODE_READ_BIT_KHR | VK_ACCESS_2_VIDEO_ENCODE_WRITE_BIT_KHR)) {
                if (!IsExtEnabled(device_extensions.vk_khr_video_encode_queue)) {
                    return {vvl::Extension::_VK_KHR_video_encode_queue};
                }
            }
            if (value & (VK_ACCESS_2_DESCRIPTOR_BUFFER_READ_BIT_EXT)) {
                if (!IsExtEnabled(device_extensions.vk_ext_descriptor_buffer)) {
                    return {vvl::Extension::_VK_EXT_descriptor_buffer};
                }
            }
            if (value & (VK_ACCESS_2_INVOCATION_MASK_READ_BIT_HUAWEI)) {
                if (!IsExtEnabled(device_extensions.vk_huawei_invocation_mask)) {
                    return {vvl::Extension::_VK_HUAWEI_invocation_mask};
                }
            }
            if (value & (VK_ACCESS_2_SHADER_BINDING_TABLE_READ_BIT_KHR)) {
                if (!IsExtEnabled(device_extensions.vk_khr_ray_tracing_maintenance1)) {
                    return {vvl::Extension::_VK_KHR_ray_tracing_maintenance1};
                }
            }
            if (value & (VK_ACCESS_2_MICROMAP_READ_BIT_EXT | VK_ACCESS_2_MICROMAP_WRITE_BIT_EXT)) {
                if (!IsExtEnabled(device_extensions.vk_ext_opacity_micromap)) {
                    return {vvl::Extension::_VK_EXT_opacity_micromap};
                }
            }
            if (value & (VK_ACCESS_2_OPTICAL_FLOW_READ_BIT_NV | VK_ACCESS_2_OPTICAL_FLOW_WRITE_BIT_NV)) {
                if (!IsExtEnabled(device_extensions.vk_nv_optical_flow)) {
                    return {vvl::Extension::_VK_NV_optical_flow};
                }
            }
            return {};
        case vvl::FlagBitmask::VkBufferUsageFlagBits2KHR:
            if (value & (VK_BUFFER_USAGE_2_EXECUTION_GRAPH_SCRATCH_BIT_AMDX)) {
                if (!IsExtEnabled(device_extensions.vk_amdx_shader_enqueue)) {
                    return {vvl::Extension::_VK_AMDX_shader_enqueue};
                }
            }
            return {};
        default:
            return {};
    }
}

// NOLINTEND
