// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See best_practices_generator.py for modifications

/***************************************************************************
 *
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
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

#include "best_practices/best_practices_validation.h"

DeprecationData GetDeprecatedData(vvl::Extension extension_name) {
    static const DeprecationData empty_deprecated_data{DeprecationReason::Empty, vvl::Extension::Empty};
    static const vvl::unordered_map<vvl::Extension, DeprecationData> deprecated_extensions = {
        {vvl::Extension::_VK_KHR_sampler_mirror_clamp_to_edge, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_2}}},
        {vvl::Extension::_VK_KHR_dynamic_rendering, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_3}}},
        {vvl::Extension::_VK_KHR_multiview, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_1}}},
        {vvl::Extension::_VK_KHR_get_physical_device_properties2, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_1}}},
        {vvl::Extension::_VK_KHR_device_group, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_1}}},
        {vvl::Extension::_VK_KHR_shader_draw_parameters, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_1}}},
        {vvl::Extension::_VK_KHR_maintenance1, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_1}}},
        {vvl::Extension::_VK_KHR_device_group_creation, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_1}}},
        {vvl::Extension::_VK_KHR_external_memory_capabilities, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_1}}},
        {vvl::Extension::_VK_KHR_external_memory, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_1}}},
        {vvl::Extension::_VK_KHR_external_semaphore_capabilities, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_1}}},
        {vvl::Extension::_VK_KHR_external_semaphore, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_1}}},
        {vvl::Extension::_VK_KHR_push_descriptor, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_4}}},
        {vvl::Extension::_VK_KHR_shader_float16_int8, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_2}}},
        {vvl::Extension::_VK_KHR_16bit_storage, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_1}}},
        {vvl::Extension::_VK_KHR_descriptor_update_template, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_1}}},
        {vvl::Extension::_VK_KHR_imageless_framebuffer, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_2}}},
        {vvl::Extension::_VK_KHR_create_renderpass2, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_2}}},
        {vvl::Extension::_VK_KHR_external_fence_capabilities, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_1}}},
        {vvl::Extension::_VK_KHR_external_fence, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_1}}},
        {vvl::Extension::_VK_KHR_maintenance2, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_1}}},
        {vvl::Extension::_VK_KHR_variable_pointers, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_1}}},
        {vvl::Extension::_VK_KHR_dedicated_allocation, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_1}}},
        {vvl::Extension::_VK_KHR_storage_buffer_storage_class, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_1}}},
        {vvl::Extension::_VK_KHR_relaxed_block_layout, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_1}}},
        {vvl::Extension::_VK_KHR_get_memory_requirements2, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_1}}},
        {vvl::Extension::_VK_KHR_image_format_list, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_2}}},
        {vvl::Extension::_VK_KHR_sampler_ycbcr_conversion, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_1}}},
        {vvl::Extension::_VK_KHR_bind_memory2, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_1}}},
        {vvl::Extension::_VK_KHR_maintenance3, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_1}}},
        {vvl::Extension::_VK_KHR_draw_indirect_count, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_2}}},
        {vvl::Extension::_VK_KHR_shader_subgroup_extended_types, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_2}}},
        {vvl::Extension::_VK_KHR_8bit_storage, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_2}}},
        {vvl::Extension::_VK_KHR_shader_atomic_int64, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_2}}},
        {vvl::Extension::_VK_KHR_global_priority, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_4}}},
        {vvl::Extension::_VK_KHR_driver_properties, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_2}}},
        {vvl::Extension::_VK_KHR_shader_float_controls, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_2}}},
        {vvl::Extension::_VK_KHR_depth_stencil_resolve, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_2}}},
        {vvl::Extension::_VK_KHR_timeline_semaphore, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_2}}},
        {vvl::Extension::_VK_KHR_vulkan_memory_model, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_2}}},
        {vvl::Extension::_VK_KHR_shader_terminate_invocation, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_3}}},
        {vvl::Extension::_VK_KHR_dynamic_rendering_local_read, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_4}}},
        {vvl::Extension::_VK_KHR_spirv_1_4, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_2}}},
        {vvl::Extension::_VK_KHR_separate_depth_stencil_layouts, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_2}}},
        {vvl::Extension::_VK_KHR_uniform_buffer_standard_layout, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_2}}},
        {vvl::Extension::_VK_KHR_buffer_device_address, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_2}}},
        {vvl::Extension::_VK_KHR_map_memory2, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_4}}},
        {vvl::Extension::_VK_KHR_shader_integer_dot_product, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_3}}},
        {vvl::Extension::_VK_KHR_shader_non_semantic_info, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_3}}},
        {vvl::Extension::_VK_KHR_synchronization2, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_3}}},
        {vvl::Extension::_VK_KHR_zero_initialize_workgroup_memory, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_3}}},
        {vvl::Extension::_VK_KHR_copy_commands2, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_3}}},
        {vvl::Extension::_VK_KHR_format_feature_flags2, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_3}}},
        {vvl::Extension::_VK_KHR_maintenance4, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_3}}},
        {vvl::Extension::_VK_KHR_shader_subgroup_rotate, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_4}}},
        {vvl::Extension::_VK_KHR_maintenance5, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_4}}},
        {vvl::Extension::_VK_KHR_vertex_attribute_divisor, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_4}}},
        {vvl::Extension::_VK_KHR_load_store_op_none, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_4}}},
        {vvl::Extension::_VK_KHR_shader_float_controls2, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_4}}},
        {vvl::Extension::_VK_KHR_index_type_uint8, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_4}}},
        {vvl::Extension::_VK_KHR_line_rasterization, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_4}}},
        {vvl::Extension::_VK_KHR_shader_expect_assume, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_4}}},
        {vvl::Extension::_VK_KHR_maintenance6, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_4}}},
        {vvl::Extension::_VK_EXT_debug_report, {DeprecationReason::Deprecated, {vvl::Extension::_VK_EXT_debug_utils}}},
        {vvl::Extension::_VK_NV_glsl_shader, {DeprecationReason::Deprecated, {vvl::Extension::Empty}}},
        {vvl::Extension::_VK_EXT_debug_marker, {DeprecationReason::Promoted, {vvl::Extension::_VK_EXT_debug_utils}}},
        {vvl::Extension::_VK_NV_dedicated_allocation,
         {DeprecationReason::Deprecated, {vvl::Extension::_VK_KHR_dedicated_allocation}}},
        {vvl::Extension::_VK_AMD_draw_indirect_count, {DeprecationReason::Promoted, {vvl::Extension::_VK_KHR_draw_indirect_count}}},
        {vvl::Extension::_VK_AMD_negative_viewport_height, {DeprecationReason::Obsoleted, {vvl::Extension::_VK_KHR_maintenance1}}},
        {vvl::Extension::_VK_AMD_gpu_shader_half_float,
         {DeprecationReason::Deprecated, {vvl::Extension::_VK_KHR_shader_float16_int8}}},
        {vvl::Extension::_VK_IMG_format_pvrtc, {DeprecationReason::Deprecated, {vvl::Extension::Empty}}},
        {vvl::Extension::_VK_NV_external_memory_capabilities,
         {DeprecationReason::Deprecated, {vvl::Extension::_VK_KHR_external_memory_capabilities}}},
        {vvl::Extension::_VK_NV_external_memory, {DeprecationReason::Deprecated, {vvl::Extension::_VK_KHR_external_memory}}},
        {vvl::Extension::_VK_NV_external_memory_win32,
         {DeprecationReason::Deprecated, {vvl::Extension::_VK_KHR_external_memory_win32}}},
        {vvl::Extension::_VK_NV_win32_keyed_mutex, {DeprecationReason::Promoted, {vvl::Extension::_VK_KHR_win32_keyed_mutex}}},
        {vvl::Extension::_VK_EXT_validation_flags, {DeprecationReason::Deprecated, {vvl::Extension::_VK_EXT_layer_settings}}},
        {vvl::Extension::_VK_EXT_shader_subgroup_ballot, {DeprecationReason::Deprecated, {vvl::Version::_VK_VERSION_1_2}}},
        {vvl::Extension::_VK_EXT_shader_subgroup_vote, {DeprecationReason::Deprecated, {vvl::Version::_VK_VERSION_1_1}}},
        {vvl::Extension::_VK_EXT_texture_compression_astc_hdr, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_3}}},
        {vvl::Extension::_VK_EXT_pipeline_robustness, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_4}}},
        {vvl::Extension::_VK_MVK_ios_surface, {DeprecationReason::Deprecated, {vvl::Extension::_VK_EXT_metal_surface}}},
        {vvl::Extension::_VK_MVK_macos_surface, {DeprecationReason::Deprecated, {vvl::Extension::_VK_EXT_metal_surface}}},
        {vvl::Extension::_VK_EXT_sampler_filter_minmax, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_2}}},
        {vvl::Extension::_VK_AMD_gpu_shader_int16, {DeprecationReason::Deprecated, {vvl::Extension::_VK_KHR_shader_float16_int8}}},
        {vvl::Extension::_VK_EXT_inline_uniform_block, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_3}}},
        {vvl::Extension::_VK_EXT_descriptor_indexing, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_2}}},
        {vvl::Extension::_VK_EXT_shader_viewport_index_layer, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_2}}},
        {vvl::Extension::_VK_NV_ray_tracing, {DeprecationReason::Deprecated, {vvl::Extension::_VK_KHR_ray_tracing_pipeline}}},
        {vvl::Extension::_VK_EXT_global_priority, {DeprecationReason::Promoted, {vvl::Extension::_VK_KHR_global_priority}}},
        {vvl::Extension::_VK_EXT_calibrated_timestamps,
         {DeprecationReason::Promoted, {vvl::Extension::_VK_KHR_calibrated_timestamps}}},
        {vvl::Extension::_VK_EXT_vertex_attribute_divisor,
         {DeprecationReason::Promoted, {vvl::Extension::_VK_KHR_vertex_attribute_divisor}}},
        {vvl::Extension::_VK_EXT_pipeline_creation_feedback, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_3}}},
        {vvl::Extension::_VK_NV_compute_shader_derivatives,
         {DeprecationReason::Promoted, {vvl::Extension::_VK_KHR_compute_shader_derivatives}}},
        {vvl::Extension::_VK_NV_fragment_shader_barycentric,
         {DeprecationReason::Promoted, {vvl::Extension::_VK_KHR_fragment_shader_barycentric}}},
        {vvl::Extension::_VK_EXT_scalar_block_layout, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_2}}},
        {vvl::Extension::_VK_EXT_subgroup_size_control, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_3}}},
        {vvl::Extension::_VK_EXT_buffer_device_address,
         {DeprecationReason::Deprecated, {vvl::Extension::_VK_KHR_buffer_device_address}}},
        {vvl::Extension::_VK_EXT_tooling_info, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_3}}},
        {vvl::Extension::_VK_EXT_separate_stencil_usage, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_2}}},
        {vvl::Extension::_VK_EXT_validation_features, {DeprecationReason::Deprecated, {vvl::Extension::_VK_EXT_layer_settings}}},
        {vvl::Extension::_VK_EXT_line_rasterization, {DeprecationReason::Promoted, {vvl::Extension::_VK_KHR_line_rasterization}}},
        {vvl::Extension::_VK_EXT_host_query_reset, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_2}}},
        {vvl::Extension::_VK_EXT_index_type_uint8, {DeprecationReason::Promoted, {vvl::Extension::_VK_KHR_index_type_uint8}}},
        {vvl::Extension::_VK_EXT_extended_dynamic_state, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_3}}},
        {vvl::Extension::_VK_EXT_host_image_copy, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_4}}},
        {vvl::Extension::_VK_EXT_shader_demote_to_helper_invocation,
         {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_3}}},
        {vvl::Extension::_VK_EXT_texel_buffer_alignment, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_3}}},
        {vvl::Extension::_VK_EXT_robustness2, {DeprecationReason::Promoted, {vvl::Extension::_VK_KHR_robustness2}}},
        {vvl::Extension::_VK_EXT_private_data, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_3}}},
        {vvl::Extension::_VK_EXT_pipeline_creation_cache_control, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_3}}},
        {vvl::Extension::_VK_EXT_ycbcr_2plane_444_formats, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_3}}},
        {vvl::Extension::_VK_EXT_image_robustness, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_3}}},
        {vvl::Extension::_VK_EXT_4444_formats, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_3}}},
        {vvl::Extension::_VK_ARM_rasterization_order_attachment_access,
         {DeprecationReason::Promoted, {vvl::Extension::_VK_EXT_rasterization_order_attachment_access}}},
        {vvl::Extension::_VK_VALVE_mutable_descriptor_type,
         {DeprecationReason::Promoted, {vvl::Extension::_VK_EXT_mutable_descriptor_type}}},
        {vvl::Extension::_VK_EXT_extended_dynamic_state2, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_3}}},
        {vvl::Extension::_VK_EXT_global_priority_query, {DeprecationReason::Promoted, {vvl::Extension::_VK_KHR_global_priority}}},
        {vvl::Extension::_VK_NV_displacement_micromap,
         {DeprecationReason::Deprecated, {vvl::Extension::_VK_NV_cluster_acceleration_structure}}},
        {vvl::Extension::_VK_EXT_load_store_op_none, {DeprecationReason::Promoted, {vvl::Extension::_VK_KHR_load_store_op_none}}},
        {vvl::Extension::_VK_EXT_depth_clamp_zero_one,
         {DeprecationReason::Promoted, {vvl::Extension::_VK_KHR_depth_clamp_zero_one}}},
        {vvl::Extension::_VK_QCOM_fragment_density_map_offset,
         {DeprecationReason::Promoted, {vvl::Extension::_VK_EXT_fragment_density_map_offset}}},
        {vvl::Extension::_VK_EXT_pipeline_protected_access, {DeprecationReason::Promoted, {vvl::Version::_VK_VERSION_1_4}}},
    };

    auto it = deprecated_extensions.find(extension_name);
    return (it == deprecated_extensions.end()) ? empty_deprecated_data : it->second;
}

std::string GetSpecialUse(vvl::Extension extension_name) {
    const vvl::unordered_map<vvl::Extension, std::string> special_use_extensions = {
        {vvl::Extension::_VK_KHR_performance_query, "devtools"},
        {vvl::Extension::_VK_KHR_pipeline_executable_properties, "devtools"},
        {vvl::Extension::_VK_EXT_debug_report, "debugging"},
        {vvl::Extension::_VK_EXT_debug_marker, "debugging"},
        {vvl::Extension::_VK_EXT_transform_feedback, "glemulation, d3demulation, devtools"},
        {vvl::Extension::_VK_AMD_shader_info, "devtools"},
        {vvl::Extension::_VK_EXT_validation_flags, "debugging"},
        {vvl::Extension::_VK_EXT_depth_clip_enable, "d3demulation"},
        {vvl::Extension::_VK_IMG_relaxed_line_rasterization, "glemulation"},
        {vvl::Extension::_VK_EXT_debug_utils, "debugging"},
        {vvl::Extension::_VK_AMD_buffer_marker, "devtools"},
        {vvl::Extension::_VK_EXT_pipeline_creation_feedback, "devtools"},
        {vvl::Extension::_VK_INTEL_performance_query, "devtools"},
        {vvl::Extension::_VK_EXT_validation_features, "debugging"},
        {vvl::Extension::_VK_EXT_provoking_vertex, "glemulation"},
        {vvl::Extension::_VK_EXT_line_rasterization, "cadsupport"},
        {vvl::Extension::_VK_EXT_depth_bias_control, "d3demulation"},
        {vvl::Extension::_VK_EXT_device_memory_report, "devtools"},
        {vvl::Extension::_VK_EXT_custom_border_color, "glemulation, d3demulation"},
        {vvl::Extension::_VK_EXT_attachment_feedback_loop_layout, "glemulation, d3demulation"},
        {vvl::Extension::_VK_VALVE_mutable_descriptor_type, "d3demulation"},
        {vvl::Extension::_VK_EXT_device_address_binding_report, "debugging, devtools"},
        {vvl::Extension::_VK_EXT_depth_clip_control, "glemulation"},
        {vvl::Extension::_VK_EXT_primitive_topology_list_restart, "glemulation"},
        {vvl::Extension::_VK_EXT_primitives_generated_query, "glemulation"},
        {vvl::Extension::_VK_EXT_image_2d_view_of_3d, "glemulation"},
        {vvl::Extension::_VK_EXT_border_color_swizzle, "glemulation, d3demulation"},
        {vvl::Extension::_VK_EXT_image_sliced_view_of_3d, "d3demulation"},
        {vvl::Extension::_VK_VALVE_descriptor_set_host_mapping, "d3demulation"},
        {vvl::Extension::_VK_EXT_non_seamless_cube_map, "d3demulation, glemulation"},
        {vvl::Extension::_VK_GOOGLE_surfaceless_query, "glemulation"},
        {vvl::Extension::_VK_EXT_legacy_dithering, "glemulation"},
        {vvl::Extension::_VK_ANDROID_external_format_resolve, "glemulation"},
        {vvl::Extension::_VK_EXT_mutable_descriptor_type, "d3demulation"},
        {vvl::Extension::_VK_EXT_legacy_vertex_attributes, "glemulation"},
        {vvl::Extension::_VK_EXT_attachment_feedback_loop_dynamic_state, "glemulation, d3demulation"},
        {vvl::Extension::_VK_MESA_image_alignment_control, "d3demulation"},
    };

    auto it = special_use_extensions.find(extension_name);
    return (it == special_use_extensions.end()) ? "" : it->second;
}

void bp_state::Instance::PostCallRecordCreateInstance(const VkInstanceCreateInfo* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator, VkInstance* pInstance,
                                                      const RecordObject& record_obj) {
    bp_state::LogResult(*this, *pInstance, record_obj);
}

void bp_state::Instance::PostCallRecordEnumeratePhysicalDevices(VkInstance instance, uint32_t* pPhysicalDeviceCount,
                                                                VkPhysicalDevice* pPhysicalDevices,
                                                                const RecordObject& record_obj) {
    bp_state::LogResult(*this, instance, record_obj);
}

void bp_state::Instance::PostCallRecordGetPhysicalDeviceImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format,
                                                                              VkImageType type, VkImageTiling tiling,
                                                                              VkImageUsageFlags usage, VkImageCreateFlags flags,
                                                                              VkImageFormatProperties* pImageFormatProperties,
                                                                              const RecordObject& record_obj) {
    bp_state::LogResult(*this, physicalDevice, record_obj);
}

void bp_state::Instance::PostCallRecordCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkDevice* pDevice,
                                                    const RecordObject& record_obj) {
    bp_state::LogResult(*this, physicalDevice, record_obj);
}

void bp_state::Instance::PostCallRecordEnumerateInstanceExtensionProperties(const char* pLayerName, uint32_t* pPropertyCount,
                                                                            VkExtensionProperties* pProperties,
                                                                            const RecordObject& record_obj) {
    bp_state::LogResult(*this, (VkInstance)VK_NULL_HANDLE, record_obj);
}

void bp_state::Instance::PostCallRecordEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char* pLayerName,
                                                                          uint32_t* pPropertyCount,
                                                                          VkExtensionProperties* pProperties,
                                                                          const RecordObject& record_obj) {
    bp_state::LogResult(*this, physicalDevice, record_obj);
}

void bp_state::Instance::PostCallRecordEnumerateInstanceLayerProperties(uint32_t* pPropertyCount, VkLayerProperties* pProperties,
                                                                        const RecordObject& record_obj) {
    bp_state::LogResult(*this, (VkInstance)VK_NULL_HANDLE, record_obj);
}

void bp_state::Instance::PostCallRecordEnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                                      VkLayerProperties* pProperties,
                                                                      const RecordObject& record_obj) {
    bp_state::LogResult(*this, physicalDevice, record_obj);
}

void BestPractices::PostCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence,
                                              const RecordObject& record_obj) {
    ManualPostCallRecordQueueSubmit(queue, submitCount, pSubmits, fence, record_obj);
    bp_state::LogResult(*this, queue, record_obj);
}

void BestPractices::PostCallRecordQueueWaitIdle(VkQueue queue, const RecordObject& record_obj) {
    bp_state::LogResult(*this, queue, record_obj);
}

void BestPractices::PostCallRecordDeviceWaitIdle(VkDevice device, const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory,
                                                 const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordMapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size,
                                            VkMemoryMapFlags flags, void** ppData, const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordFlushMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
                                                          const VkMappedMemoryRange* pMemoryRanges,
                                                          const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordInvalidateMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
                                                               const VkMappedMemoryRange* pMemoryRanges,
                                                               const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory,
                                                   VkDeviceSize memoryOffset, const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset,
                                                  const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo,
                                                  VkFence fence, const RecordObject& record_obj) {
    ManualPostCallRecordQueueBindSparse(queue, bindInfoCount, pBindInfo, fence, record_obj);
    bp_state::LogResult(*this, queue, record_obj);
}

void BestPractices::PostCallRecordCreateFence(VkDevice device, const VkFenceCreateInfo* pCreateInfo,
                                              const VkAllocationCallbacks* pAllocator, VkFence* pFence,
                                              const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordResetFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences,
                                              const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordGetFenceStatus(VkDevice device, VkFence fence, const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll,
                                                uint64_t timeout, const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore,
                                                  const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordCreateEvent(VkDevice device, const VkEventCreateInfo* pCreateInfo,
                                              const VkAllocationCallbacks* pAllocator, VkEvent* pEvent,
                                              const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordGetEventStatus(VkDevice device, VkEvent event, const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordSetEvent(VkDevice device, VkEvent event, const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordResetEvent(VkDevice device, VkEvent event, const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkQueryPool* pQueryPool,
                                                  const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery,
                                                      uint32_t queryCount, size_t dataSize, void* pData, VkDeviceSize stride,
                                                      VkQueryResultFlags flags, const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer,
                                               const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordCreateBufferView(VkDevice device, const VkBufferViewCreateInfo* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator, VkBufferView* pView,
                                                   const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo,
                                              const VkAllocationCallbacks* pAllocator, VkImage* pImage,
                                              const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordCreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkImageView* pView,
                                                  const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule,
                                                     const RecordObject& record_obj, chassis::CreateShaderModule& chassis_state) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordCreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator, VkPipelineCache* pPipelineCache,
                                                      const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordGetPipelineCacheData(VkDevice device, VkPipelineCache pipelineCache, size_t* pDataSize,
                                                       void* pData, const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordMergePipelineCaches(VkDevice device, VkPipelineCache dstCache, uint32_t srcCacheCount,
                                                      const VkPipelineCache* pSrcCaches, const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                          const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                                          const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                          const RecordObject& record_obj, PipelineStates& pipeline_states,
                                                          chassis::CreateGraphicsPipelines& chassis_state) {
    ManualPostCallRecordCreateGraphicsPipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines,
                                                record_obj, pipeline_states, chassis_state);
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                         const VkComputePipelineCreateInfo* pCreateInfos,
                                                         const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                         const RecordObject& record_obj, PipelineStates& pipeline_states,
                                                         chassis::CreateComputePipelines& chassis_state) {
    ManualPostCallRecordCreateComputePipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines,
                                               record_obj, pipeline_states, chassis_state);
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo,
                                                       const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout,
                                                       const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordCreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkSampler* pSampler,
                                                const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordCreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                                            const VkAllocationCallbacks* pAllocator,
                                                            VkDescriptorSetLayout* pSetLayout, const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo* pCreateInfo,
                                                       const VkAllocationCallbacks* pAllocator, VkDescriptorPool* pDescriptorPool,
                                                       const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo,
                                                         VkDescriptorSet* pDescriptorSets, const RecordObject& record_obj,
                                                         vvl::AllocateDescriptorSetsData& chassis_state) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer,
                                                    const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass,
                                                   const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool,
                                                    const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags,
                                                   const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* pAllocateInfo,
                                                         VkCommandBuffer* pCommandBuffers, const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo,
                                                     const RecordObject& record_obj) {
    bp_state::LogResult(*this, commandBuffer, record_obj);
}

void BestPractices::PostCallRecordEndCommandBuffer(VkCommandBuffer commandBuffer, const RecordObject& record_obj) {
    bp_state::LogResult(*this, commandBuffer, record_obj);
}

void BestPractices::PostCallRecordResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags,
                                                     const RecordObject& record_obj) {
    bp_state::LogResult(*this, commandBuffer, record_obj);
}

void BestPractices::PostCallRecordBindBufferMemory2(VkDevice device, uint32_t bindInfoCount,
                                                    const VkBindBufferMemoryInfo* pBindInfos, const RecordObject& record_obj) {
    ManualPostCallRecordBindBufferMemory2(device, bindInfoCount, pBindInfos, record_obj);
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordBindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos,
                                                   const RecordObject& record_obj) {
    ManualPostCallRecordBindImageMemory2(device, bindInfoCount, pBindInfos, record_obj);
    bp_state::LogResult(*this, device, record_obj);
}

void bp_state::Instance::PostCallRecordEnumeratePhysicalDeviceGroups(
    VkInstance instance, uint32_t* pPhysicalDeviceGroupCount, VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties,
    const RecordObject& record_obj) {
    bp_state::LogResult(*this, instance, record_obj);
}

void bp_state::Instance::PostCallRecordGetPhysicalDeviceImageFormatProperties2(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo,
    VkImageFormatProperties2* pImageFormatProperties, const RecordObject& record_obj) {
    bp_state::LogResult(*this, physicalDevice, record_obj);
}

void BestPractices::PostCallRecordCreateSamplerYcbcrConversion(VkDevice device,
                                                               const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                               const VkAllocationCallbacks* pAllocator,
                                                               VkSamplerYcbcrConversion* pYcbcrConversion,
                                                               const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordCreateDescriptorUpdateTemplate(VkDevice device,
                                                                 const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
                                                                 const VkAllocationCallbacks* pAllocator,
                                                                 VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate,
                                                                 const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordCreateRenderPass2(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass,
                                                    const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordGetSemaphoreCounterValue(VkDevice device, VkSemaphore semaphore, uint64_t* pValue,
                                                           const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordWaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout,
                                                 const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordSignalSemaphore(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo,
                                                  const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void bp_state::Instance::PostCallRecordGetPhysicalDeviceToolProperties(VkPhysicalDevice physicalDevice, uint32_t* pToolCount,
                                                                       VkPhysicalDeviceToolProperties* pToolProperties,
                                                                       const RecordObject& record_obj) {
    bp_state::LogResult(*this, physicalDevice, record_obj);
}

void BestPractices::PostCallRecordCreatePrivateDataSlot(VkDevice device, const VkPrivateDataSlotCreateInfo* pCreateInfo,
                                                        const VkAllocationCallbacks* pAllocator,
                                                        VkPrivateDataSlot* pPrivateDataSlot, const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordSetPrivateData(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                                 VkPrivateDataSlot privateDataSlot, uint64_t data, const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence,
                                               const RecordObject& record_obj) {
    bp_state::LogResult(*this, queue, record_obj);
}

void BestPractices::PostCallRecordMapMemory2(VkDevice device, const VkMemoryMapInfo* pMemoryMapInfo, void** ppData,
                                             const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordUnmapMemory2(VkDevice device, const VkMemoryUnmapInfo* pMemoryUnmapInfo,
                                               const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordCopyMemoryToImage(VkDevice device, const VkCopyMemoryToImageInfo* pCopyMemoryToImageInfo,
                                                    const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordCopyImageToMemory(VkDevice device, const VkCopyImageToMemoryInfo* pCopyImageToMemoryInfo,
                                                    const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordCopyImageToImage(VkDevice device, const VkCopyImageToImageInfo* pCopyImageToImageInfo,
                                                   const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordTransitionImageLayout(VkDevice device, uint32_t transitionCount,
                                                        const VkHostImageLayoutTransitionInfo* pTransitions,
                                                        const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void bp_state::Instance::PostCallRecordGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice,
                                                                          uint32_t queueFamilyIndex, VkSurfaceKHR surface,
                                                                          VkBool32* pSupported, const RecordObject& record_obj) {
    bp_state::LogResult(*this, physicalDevice, record_obj);
}

void bp_state::Instance::PostCallRecordGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice,
                                                                               VkSurfaceKHR surface,
                                                                               VkSurfaceCapabilitiesKHR* pSurfaceCapabilities,
                                                                               const RecordObject& record_obj) {
    bp_state::LogResult(*this, physicalDevice, record_obj);
}

void bp_state::Instance::PostCallRecordGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                          uint32_t* pSurfaceFormatCount,
                                                                          VkSurfaceFormatKHR* pSurfaceFormats,
                                                                          const RecordObject& record_obj) {
    bp_state::LogResult(*this, physicalDevice, record_obj);
}

void bp_state::Instance::PostCallRecordGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice,
                                                                               VkSurfaceKHR surface, uint32_t* pPresentModeCount,
                                                                               VkPresentModeKHR* pPresentModes,
                                                                               const RecordObject& record_obj) {
    bp_state::LogResult(*this, physicalDevice, record_obj);
}

void BestPractices::PostCallRecordCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain,
                                                     const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount,
                                                        VkImage* pSwapchainImages, const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout,
                                                      VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex,
                                                      const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo,
                                                  const RecordObject& record_obj) {
    ManualPostCallRecordQueuePresentKHR(queue, pPresentInfo, record_obj);
    bp_state::LogResult(*this, queue, record_obj);
}

void BestPractices::PostCallRecordGetDeviceGroupPresentCapabilitiesKHR(
    VkDevice device, VkDeviceGroupPresentCapabilitiesKHR* pDeviceGroupPresentCapabilities, const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordGetDeviceGroupSurfacePresentModesKHR(VkDevice device, VkSurfaceKHR surface,
                                                                       VkDeviceGroupPresentModeFlagsKHR* pModes,
                                                                       const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void bp_state::Instance::PostCallRecordGetPhysicalDevicePresentRectanglesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                             uint32_t* pRectCount, VkRect2D* pRects,
                                                                             const RecordObject& record_obj) {
    bp_state::LogResult(*this, physicalDevice, record_obj);
}

void BestPractices::PostCallRecordAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo,
                                                       uint32_t* pImageIndex, const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void bp_state::Instance::PostCallRecordGetPhysicalDeviceDisplayPropertiesKHR(VkPhysicalDevice physicalDevice,
                                                                             uint32_t* pPropertyCount,
                                                                             VkDisplayPropertiesKHR* pProperties,
                                                                             const RecordObject& record_obj) {
    bp_state::LogResult(*this, physicalDevice, record_obj);
}

void bp_state::Instance::PostCallRecordGetPhysicalDeviceDisplayPlanePropertiesKHR(VkPhysicalDevice physicalDevice,
                                                                                  uint32_t* pPropertyCount,
                                                                                  VkDisplayPlanePropertiesKHR* pProperties,
                                                                                  const RecordObject& record_obj) {
    bp_state::LogResult(*this, physicalDevice, record_obj);
}

void bp_state::Instance::PostCallRecordGetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice physicalDevice, uint32_t planeIndex,
                                                                           uint32_t* pDisplayCount, VkDisplayKHR* pDisplays,
                                                                           const RecordObject& record_obj) {
    bp_state::LogResult(*this, physicalDevice, record_obj);
}

void bp_state::Instance::PostCallRecordGetDisplayModePropertiesKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                                   uint32_t* pPropertyCount,
                                                                   VkDisplayModePropertiesKHR* pProperties,
                                                                   const RecordObject& record_obj) {
    bp_state::LogResult(*this, physicalDevice, record_obj);
}

void bp_state::Instance::PostCallRecordCreateDisplayModeKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                            const VkDisplayModeCreateInfoKHR* pCreateInfo,
                                                            const VkAllocationCallbacks* pAllocator, VkDisplayModeKHR* pMode,
                                                            const RecordObject& record_obj) {
    bp_state::LogResult(*this, physicalDevice, record_obj);
}

void bp_state::Instance::PostCallRecordGetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkDisplayModeKHR mode,
                                                                      uint32_t planeIndex,
                                                                      VkDisplayPlaneCapabilitiesKHR* pCapabilities,
                                                                      const RecordObject& record_obj) {
    bp_state::LogResult(*this, physicalDevice, record_obj);
}

void bp_state::Instance::PostCallRecordCreateDisplayPlaneSurfaceKHR(VkInstance instance,
                                                                    const VkDisplaySurfaceCreateInfoKHR* pCreateInfo,
                                                                    const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                                    const RecordObject& record_obj) {
    bp_state::LogResult(*this, instance, record_obj);
}

void BestPractices::PostCallRecordCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount,
                                                            const VkSwapchainCreateInfoKHR* pCreateInfos,
                                                            const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchains,
                                                            const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

#ifdef VK_USE_PLATFORM_XLIB_KHR
void bp_state::Instance::PostCallRecordCreateXlibSurfaceKHR(VkInstance instance, const VkXlibSurfaceCreateInfoKHR* pCreateInfo,
                                                            const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                            const RecordObject& record_obj) {
    bp_state::LogResult(*this, instance, record_obj);
}
#endif  // VK_USE_PLATFORM_XLIB_KHR

#ifdef VK_USE_PLATFORM_XCB_KHR
void bp_state::Instance::PostCallRecordCreateXcbSurfaceKHR(VkInstance instance, const VkXcbSurfaceCreateInfoKHR* pCreateInfo,
                                                           const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                           const RecordObject& record_obj) {
    bp_state::LogResult(*this, instance, record_obj);
}
#endif  // VK_USE_PLATFORM_XCB_KHR

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
void bp_state::Instance::PostCallRecordCreateWaylandSurfaceKHR(VkInstance instance,
                                                               const VkWaylandSurfaceCreateInfoKHR* pCreateInfo,
                                                               const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                               const RecordObject& record_obj) {
    bp_state::LogResult(*this, instance, record_obj);
}
#endif  // VK_USE_PLATFORM_WAYLAND_KHR

#ifdef VK_USE_PLATFORM_ANDROID_KHR
void bp_state::Instance::PostCallRecordCreateAndroidSurfaceKHR(VkInstance instance,
                                                               const VkAndroidSurfaceCreateInfoKHR* pCreateInfo,
                                                               const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                               const RecordObject& record_obj) {
    bp_state::LogResult(*this, instance, record_obj);
}
#endif  // VK_USE_PLATFORM_ANDROID_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR
void bp_state::Instance::PostCallRecordCreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR* pCreateInfo,
                                                             const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                             const RecordObject& record_obj) {
    bp_state::LogResult(*this, instance, record_obj);
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

void bp_state::Instance::PostCallRecordGetPhysicalDeviceVideoCapabilitiesKHR(VkPhysicalDevice physicalDevice,
                                                                             const VkVideoProfileInfoKHR* pVideoProfile,
                                                                             VkVideoCapabilitiesKHR* pCapabilities,
                                                                             const RecordObject& record_obj) {
    bp_state::LogResult(*this, physicalDevice, record_obj);
}

void bp_state::Instance::PostCallRecordGetPhysicalDeviceVideoFormatPropertiesKHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceVideoFormatInfoKHR* pVideoFormatInfo,
    uint32_t* pVideoFormatPropertyCount, VkVideoFormatPropertiesKHR* pVideoFormatProperties, const RecordObject& record_obj) {
    bp_state::LogResult(*this, physicalDevice, record_obj);
}

void BestPractices::PostCallRecordCreateVideoSessionKHR(VkDevice device, const VkVideoSessionCreateInfoKHR* pCreateInfo,
                                                        const VkAllocationCallbacks* pAllocator, VkVideoSessionKHR* pVideoSession,
                                                        const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordGetVideoSessionMemoryRequirementsKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                                       uint32_t* pMemoryRequirementsCount,
                                                                       VkVideoSessionMemoryRequirementsKHR* pMemoryRequirements,
                                                                       const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordBindVideoSessionMemoryKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                            uint32_t bindSessionMemoryInfoCount,
                                                            const VkBindVideoSessionMemoryInfoKHR* pBindSessionMemoryInfos,
                                                            const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordCreateVideoSessionParametersKHR(VkDevice device,
                                                                  const VkVideoSessionParametersCreateInfoKHR* pCreateInfo,
                                                                  const VkAllocationCallbacks* pAllocator,
                                                                  VkVideoSessionParametersKHR* pVideoSessionParameters,
                                                                  const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordUpdateVideoSessionParametersKHR(VkDevice device,
                                                                  VkVideoSessionParametersKHR videoSessionParameters,
                                                                  const VkVideoSessionParametersUpdateInfoKHR* pUpdateInfo,
                                                                  const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void bp_state::Instance::PostCallRecordGetPhysicalDeviceImageFormatProperties2KHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo,
    VkImageFormatProperties2* pImageFormatProperties, const RecordObject& record_obj) {
    PostCallRecordGetPhysicalDeviceImageFormatProperties2(physicalDevice, pImageFormatInfo, pImageFormatProperties, record_obj);
}

void bp_state::Instance::PostCallRecordEnumeratePhysicalDeviceGroupsKHR(
    VkInstance instance, uint32_t* pPhysicalDeviceGroupCount, VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties,
    const RecordObject& record_obj) {
    PostCallRecordEnumeratePhysicalDeviceGroups(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties, record_obj);
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
void BestPractices::PostCallRecordGetMemoryWin32HandleKHR(VkDevice device, const VkMemoryGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                                          HANDLE* pHandle, const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordGetMemoryWin32HandlePropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType,
                                                                    HANDLE handle,
                                                                    VkMemoryWin32HandlePropertiesKHR* pMemoryWin32HandleProperties,
                                                                    const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

void BestPractices::PostCallRecordGetMemoryFdKHR(VkDevice device, const VkMemoryGetFdInfoKHR* pGetFdInfo, int* pFd,
                                                 const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordGetMemoryFdPropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, int fd,
                                                           VkMemoryFdPropertiesKHR* pMemoryFdProperties,
                                                           const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
void BestPractices::PostCallRecordImportSemaphoreWin32HandleKHR(
    VkDevice device, const VkImportSemaphoreWin32HandleInfoKHR* pImportSemaphoreWin32HandleInfo, const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordGetSemaphoreWin32HandleKHR(VkDevice device,
                                                             const VkSemaphoreGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                                             HANDLE* pHandle, const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

void BestPractices::PostCallRecordImportSemaphoreFdKHR(VkDevice device, const VkImportSemaphoreFdInfoKHR* pImportSemaphoreFdInfo,
                                                       const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordGetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR* pGetFdInfo, int* pFd,
                                                    const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordCreateDescriptorUpdateTemplateKHR(VkDevice device,
                                                                    const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
                                                                    const VkAllocationCallbacks* pAllocator,
                                                                    VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate,
                                                                    const RecordObject& record_obj) {
    PostCallRecordCreateDescriptorUpdateTemplate(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate, record_obj);
}

void BestPractices::PostCallRecordCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo,
                                                       const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass,
                                                       const RecordObject& record_obj) {
    PostCallRecordCreateRenderPass2(device, pCreateInfo, pAllocator, pRenderPass, record_obj);
}

void BestPractices::PostCallRecordGetSwapchainStatusKHR(VkDevice device, VkSwapchainKHR swapchain, const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
void BestPractices::PostCallRecordImportFenceWin32HandleKHR(VkDevice device,
                                                            const VkImportFenceWin32HandleInfoKHR* pImportFenceWin32HandleInfo,
                                                            const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordGetFenceWin32HandleKHR(VkDevice device, const VkFenceGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                                         HANDLE* pHandle, const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

void BestPractices::PostCallRecordImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR* pImportFenceFdInfo,
                                                   const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordGetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR* pGetFdInfo, int* pFd,
                                                const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void bp_state::Instance::PostCallRecordEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(
    VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, uint32_t* pCounterCount, VkPerformanceCounterKHR* pCounters,
    VkPerformanceCounterDescriptionKHR* pCounterDescriptions, const RecordObject& record_obj) {
    bp_state::LogResult(*this, physicalDevice, record_obj);
}

void BestPractices::PostCallRecordAcquireProfilingLockKHR(VkDevice device, const VkAcquireProfilingLockInfoKHR* pInfo,
                                                          const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void bp_state::Instance::PostCallRecordGetPhysicalDeviceSurfaceCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                                                const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                                                VkSurfaceCapabilities2KHR* pSurfaceCapabilities,
                                                                                const RecordObject& record_obj) {
    bp_state::LogResult(*this, physicalDevice, record_obj);
}

void bp_state::Instance::PostCallRecordGetPhysicalDeviceSurfaceFormats2KHR(VkPhysicalDevice physicalDevice,
                                                                           const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                                           uint32_t* pSurfaceFormatCount,
                                                                           VkSurfaceFormat2KHR* pSurfaceFormats,
                                                                           const RecordObject& record_obj) {
    bp_state::LogResult(*this, physicalDevice, record_obj);
}

void bp_state::Instance::PostCallRecordGetPhysicalDeviceDisplayProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                              uint32_t* pPropertyCount,
                                                                              VkDisplayProperties2KHR* pProperties,
                                                                              const RecordObject& record_obj) {
    bp_state::LogResult(*this, physicalDevice, record_obj);
}

void bp_state::Instance::PostCallRecordGetPhysicalDeviceDisplayPlaneProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                                   uint32_t* pPropertyCount,
                                                                                   VkDisplayPlaneProperties2KHR* pProperties,
                                                                                   const RecordObject& record_obj) {
    bp_state::LogResult(*this, physicalDevice, record_obj);
}

void bp_state::Instance::PostCallRecordGetDisplayModeProperties2KHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                                    uint32_t* pPropertyCount,
                                                                    VkDisplayModeProperties2KHR* pProperties,
                                                                    const RecordObject& record_obj) {
    bp_state::LogResult(*this, physicalDevice, record_obj);
}

void bp_state::Instance::PostCallRecordGetDisplayPlaneCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                                       const VkDisplayPlaneInfo2KHR* pDisplayPlaneInfo,
                                                                       VkDisplayPlaneCapabilities2KHR* pCapabilities,
                                                                       const RecordObject& record_obj) {
    bp_state::LogResult(*this, physicalDevice, record_obj);
}

void BestPractices::PostCallRecordCreateSamplerYcbcrConversionKHR(VkDevice device,
                                                                  const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                                  const VkAllocationCallbacks* pAllocator,
                                                                  VkSamplerYcbcrConversion* pYcbcrConversion,
                                                                  const RecordObject& record_obj) {
    PostCallRecordCreateSamplerYcbcrConversion(device, pCreateInfo, pAllocator, pYcbcrConversion, record_obj);
}

void BestPractices::PostCallRecordBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                                       const VkBindBufferMemoryInfo* pBindInfos, const RecordObject& record_obj) {
    PostCallRecordBindBufferMemory2(device, bindInfoCount, pBindInfos, record_obj);
}

void BestPractices::PostCallRecordBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                                      const VkBindImageMemoryInfo* pBindInfos, const RecordObject& record_obj) {
    PostCallRecordBindImageMemory2(device, bindInfoCount, pBindInfos, record_obj);
}

void BestPractices::PostCallRecordGetSemaphoreCounterValueKHR(VkDevice device, VkSemaphore semaphore, uint64_t* pValue,
                                                              const RecordObject& record_obj) {
    PostCallRecordGetSemaphoreCounterValue(device, semaphore, pValue, record_obj);
}

void BestPractices::PostCallRecordWaitSemaphoresKHR(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout,
                                                    const RecordObject& record_obj) {
    PostCallRecordWaitSemaphores(device, pWaitInfo, timeout, record_obj);
}

void BestPractices::PostCallRecordSignalSemaphoreKHR(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo,
                                                     const RecordObject& record_obj) {
    PostCallRecordSignalSemaphore(device, pSignalInfo, record_obj);
}

void bp_state::Instance::PostCallRecordGetPhysicalDeviceFragmentShadingRatesKHR(
    VkPhysicalDevice physicalDevice, uint32_t* pFragmentShadingRateCount,
    VkPhysicalDeviceFragmentShadingRateKHR* pFragmentShadingRates, const RecordObject& record_obj) {
    bp_state::LogResult(*this, physicalDevice, record_obj);
}

void BestPractices::PostCallRecordWaitForPresentKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t presentId, uint64_t timeout,
                                                    const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordCreateDeferredOperationKHR(VkDevice device, const VkAllocationCallbacks* pAllocator,
                                                             VkDeferredOperationKHR* pDeferredOperation,
                                                             const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordGetDeferredOperationResultKHR(VkDevice device, VkDeferredOperationKHR operation,
                                                                const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordDeferredOperationJoinKHR(VkDevice device, VkDeferredOperationKHR operation,
                                                           const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordGetPipelineExecutablePropertiesKHR(VkDevice device, const VkPipelineInfoKHR* pPipelineInfo,
                                                                     uint32_t* pExecutableCount,
                                                                     VkPipelineExecutablePropertiesKHR* pProperties,
                                                                     const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordGetPipelineExecutableStatisticsKHR(VkDevice device,
                                                                     const VkPipelineExecutableInfoKHR* pExecutableInfo,
                                                                     uint32_t* pStatisticCount,
                                                                     VkPipelineExecutableStatisticKHR* pStatistics,
                                                                     const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordGetPipelineExecutableInternalRepresentationsKHR(
    VkDevice device, const VkPipelineExecutableInfoKHR* pExecutableInfo, uint32_t* pInternalRepresentationCount,
    VkPipelineExecutableInternalRepresentationKHR* pInternalRepresentations, const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordMapMemory2KHR(VkDevice device, const VkMemoryMapInfo* pMemoryMapInfo, void** ppData,
                                                const RecordObject& record_obj) {
    PostCallRecordMapMemory2(device, pMemoryMapInfo, ppData, record_obj);
}

void BestPractices::PostCallRecordUnmapMemory2KHR(VkDevice device, const VkMemoryUnmapInfo* pMemoryUnmapInfo,
                                                  const RecordObject& record_obj) {
    PostCallRecordUnmapMemory2(device, pMemoryUnmapInfo, record_obj);
}

void bp_state::Instance::PostCallRecordGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR* pQualityLevelInfo,
    VkVideoEncodeQualityLevelPropertiesKHR* pQualityLevelProperties, const RecordObject& record_obj) {
    bp_state::LogResult(*this, physicalDevice, record_obj);
}

void BestPractices::PostCallRecordGetEncodedVideoSessionParametersKHR(
    VkDevice device, const VkVideoEncodeSessionParametersGetInfoKHR* pVideoSessionParametersInfo,
    VkVideoEncodeSessionParametersFeedbackInfoKHR* pFeedbackInfo, size_t* pDataSize, void* pData, const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence,
                                                  const RecordObject& record_obj) {
    PostCallRecordQueueSubmit2(queue, submitCount, pSubmits, fence, record_obj);
}

void BestPractices::PostCallRecordCreatePipelineBinariesKHR(VkDevice device, const VkPipelineBinaryCreateInfoKHR* pCreateInfo,
                                                            const VkAllocationCallbacks* pAllocator,
                                                            VkPipelineBinaryHandlesInfoKHR* pBinaries,
                                                            const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordGetPipelineKeyKHR(VkDevice device, const VkPipelineCreateInfoKHR* pPipelineCreateInfo,
                                                    VkPipelineBinaryKeyKHR* pPipelineKey, const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordGetPipelineBinaryDataKHR(VkDevice device, const VkPipelineBinaryDataInfoKHR* pInfo,
                                                           VkPipelineBinaryKeyKHR* pPipelineBinaryKey,
                                                           size_t* pPipelineBinaryDataSize, void* pPipelineBinaryData,
                                                           const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void bp_state::Instance::PostCallRecordGetPhysicalDeviceCooperativeMatrixPropertiesKHR(
    VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkCooperativeMatrixPropertiesKHR* pProperties,
    const RecordObject& record_obj) {
    bp_state::LogResult(*this, physicalDevice, record_obj);
}

void bp_state::Instance::PostCallRecordGetPhysicalDeviceCalibrateableTimeDomainsKHR(VkPhysicalDevice physicalDevice,
                                                                                    uint32_t* pTimeDomainCount,
                                                                                    VkTimeDomainKHR* pTimeDomains,
                                                                                    const RecordObject& record_obj) {
    bp_state::LogResult(*this, physicalDevice, record_obj);
}

void BestPractices::PostCallRecordGetCalibratedTimestampsKHR(VkDevice device, uint32_t timestampCount,
                                                             const VkCalibratedTimestampInfoKHR* pTimestampInfos,
                                                             uint64_t* pTimestamps, uint64_t* pMaxDeviation,
                                                             const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void bp_state::Instance::PostCallRecordCreateDebugReportCallbackEXT(VkInstance instance,
                                                                    const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
                                                                    const VkAllocationCallbacks* pAllocator,
                                                                    VkDebugReportCallbackEXT* pCallback,
                                                                    const RecordObject& record_obj) {
    bp_state::LogResult(*this, instance, record_obj);
}

void BestPractices::PostCallRecordDebugMarkerSetObjectTagEXT(VkDevice device, const VkDebugMarkerObjectTagInfoEXT* pTagInfo,
                                                             const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordDebugMarkerSetObjectNameEXT(VkDevice device, const VkDebugMarkerObjectNameInfoEXT* pNameInfo,
                                                              const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordCreateCuModuleNVX(VkDevice device, const VkCuModuleCreateInfoNVX* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkCuModuleNVX* pModule,
                                                    const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordCreateCuFunctionNVX(VkDevice device, const VkCuFunctionCreateInfoNVX* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator, VkCuFunctionNVX* pFunction,
                                                      const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordGetImageViewAddressNVX(VkDevice device, VkImageView imageView,
                                                         VkImageViewAddressPropertiesNVX* pProperties,
                                                         const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordGetShaderInfoAMD(VkDevice device, VkPipeline pipeline, VkShaderStageFlagBits shaderStage,
                                                   VkShaderInfoTypeAMD infoType, size_t* pInfoSize, void* pInfo,
                                                   const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

#ifdef VK_USE_PLATFORM_GGP
void bp_state::Instance::PostCallRecordCreateStreamDescriptorSurfaceGGP(VkInstance instance,
                                                                        const VkStreamDescriptorSurfaceCreateInfoGGP* pCreateInfo,
                                                                        const VkAllocationCallbacks* pAllocator,
                                                                        VkSurfaceKHR* pSurface, const RecordObject& record_obj) {
    bp_state::LogResult(*this, instance, record_obj);
}
#endif  // VK_USE_PLATFORM_GGP

void bp_state::Instance::PostCallRecordGetPhysicalDeviceExternalImageFormatPropertiesNV(
    VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage,
    VkImageCreateFlags flags, VkExternalMemoryHandleTypeFlagsNV externalHandleType,
    VkExternalImageFormatPropertiesNV* pExternalImageFormatProperties, const RecordObject& record_obj) {
    bp_state::LogResult(*this, physicalDevice, record_obj);
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
void BestPractices::PostCallRecordGetMemoryWin32HandleNV(VkDevice device, VkDeviceMemory memory,
                                                         VkExternalMemoryHandleTypeFlagsNV handleType, HANDLE* pHandle,
                                                         const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_VI_NN
void bp_state::Instance::PostCallRecordCreateViSurfaceNN(VkInstance instance, const VkViSurfaceCreateInfoNN* pCreateInfo,
                                                         const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                         const RecordObject& record_obj) {
    bp_state::LogResult(*this, instance, record_obj);
}
#endif  // VK_USE_PLATFORM_VI_NN

#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT
void bp_state::Instance::PostCallRecordAcquireXlibDisplayEXT(VkPhysicalDevice physicalDevice, Display* dpy, VkDisplayKHR display,
                                                             const RecordObject& record_obj) {
    bp_state::LogResult(*this, physicalDevice, record_obj);
}

void bp_state::Instance::PostCallRecordGetRandROutputDisplayEXT(VkPhysicalDevice physicalDevice, Display* dpy, RROutput rrOutput,
                                                                VkDisplayKHR* pDisplay, const RecordObject& record_obj) {
    bp_state::LogResult(*this, physicalDevice, record_obj);
}
#endif  // VK_USE_PLATFORM_XLIB_XRANDR_EXT

void bp_state::Instance::PostCallRecordGetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice,
                                                                                VkSurfaceKHR surface,
                                                                                VkSurfaceCapabilities2EXT* pSurfaceCapabilities,
                                                                                const RecordObject& record_obj) {
    bp_state::LogResult(*this, physicalDevice, record_obj);
}

void BestPractices::PostCallRecordDisplayPowerControlEXT(VkDevice device, VkDisplayKHR display,
                                                         const VkDisplayPowerInfoEXT* pDisplayPowerInfo,
                                                         const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordRegisterDeviceEventEXT(VkDevice device, const VkDeviceEventInfoEXT* pDeviceEventInfo,
                                                         const VkAllocationCallbacks* pAllocator, VkFence* pFence,
                                                         const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordRegisterDisplayEventEXT(VkDevice device, VkDisplayKHR display,
                                                          const VkDisplayEventInfoEXT* pDisplayEventInfo,
                                                          const VkAllocationCallbacks* pAllocator, VkFence* pFence,
                                                          const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordGetSwapchainCounterEXT(VkDevice device, VkSwapchainKHR swapchain,
                                                         VkSurfaceCounterFlagBitsEXT counter, uint64_t* pCounterValue,
                                                         const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordGetRefreshCycleDurationGOOGLE(VkDevice device, VkSwapchainKHR swapchain,
                                                                VkRefreshCycleDurationGOOGLE* pDisplayTimingProperties,
                                                                const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordGetPastPresentationTimingGOOGLE(VkDevice device, VkSwapchainKHR swapchain,
                                                                  uint32_t* pPresentationTimingCount,
                                                                  VkPastPresentationTimingGOOGLE* pPresentationTimings,
                                                                  const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

#ifdef VK_USE_PLATFORM_IOS_MVK
void bp_state::Instance::PostCallRecordCreateIOSSurfaceMVK(VkInstance instance, const VkIOSSurfaceCreateInfoMVK* pCreateInfo,
                                                           const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                           const RecordObject& record_obj) {
    bp_state::LogResult(*this, instance, record_obj);
}
#endif  // VK_USE_PLATFORM_IOS_MVK

#ifdef VK_USE_PLATFORM_MACOS_MVK
void bp_state::Instance::PostCallRecordCreateMacOSSurfaceMVK(VkInstance instance, const VkMacOSSurfaceCreateInfoMVK* pCreateInfo,
                                                             const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                             const RecordObject& record_obj) {
    bp_state::LogResult(*this, instance, record_obj);
}
#endif  // VK_USE_PLATFORM_MACOS_MVK

void BestPractices::PostCallRecordSetDebugUtilsObjectNameEXT(VkDevice device, const VkDebugUtilsObjectNameInfoEXT* pNameInfo,
                                                             const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordSetDebugUtilsObjectTagEXT(VkDevice device, const VkDebugUtilsObjectTagInfoEXT* pTagInfo,
                                                            const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void bp_state::Instance::PostCallRecordCreateDebugUtilsMessengerEXT(VkInstance instance,
                                                                    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                                                    const VkAllocationCallbacks* pAllocator,
                                                                    VkDebugUtilsMessengerEXT* pMessenger,
                                                                    const RecordObject& record_obj) {
    bp_state::LogResult(*this, instance, record_obj);
}

#ifdef VK_USE_PLATFORM_ANDROID_KHR
void BestPractices::PostCallRecordGetAndroidHardwareBufferPropertiesANDROID(VkDevice device, const struct AHardwareBuffer* buffer,
                                                                            VkAndroidHardwareBufferPropertiesANDROID* pProperties,
                                                                            const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordGetMemoryAndroidHardwareBufferANDROID(VkDevice device,
                                                                        const VkMemoryGetAndroidHardwareBufferInfoANDROID* pInfo,
                                                                        struct AHardwareBuffer** pBuffer,
                                                                        const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}
#endif  // VK_USE_PLATFORM_ANDROID_KHR

#ifdef VK_ENABLE_BETA_EXTENSIONS
void BestPractices::PostCallRecordCreateExecutionGraphPipelinesAMDX(VkDevice device, VkPipelineCache pipelineCache,
                                                                    uint32_t createInfoCount,
                                                                    const VkExecutionGraphPipelineCreateInfoAMDX* pCreateInfos,
                                                                    const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                                    const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordGetExecutionGraphPipelineScratchSizeAMDX(VkDevice device, VkPipeline executionGraph,
                                                                           VkExecutionGraphPipelineScratchSizeAMDX* pSizeInfo,
                                                                           const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordGetExecutionGraphPipelineNodeIndexAMDX(VkDevice device, VkPipeline executionGraph,
                                                                         const VkPipelineShaderStageNodeCreateInfoAMDX* pNodeInfo,
                                                                         uint32_t* pNodeIndex, const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}
#endif  // VK_ENABLE_BETA_EXTENSIONS

void BestPractices::PostCallRecordGetImageDrmFormatModifierPropertiesEXT(VkDevice device, VkImage image,
                                                                         VkImageDrmFormatModifierPropertiesEXT* pProperties,
                                                                         const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordCreateAccelerationStructureNV(VkDevice device,
                                                                const VkAccelerationStructureCreateInfoNV* pCreateInfo,
                                                                const VkAllocationCallbacks* pAllocator,
                                                                VkAccelerationStructureNV* pAccelerationStructure,
                                                                const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordBindAccelerationStructureMemoryNV(VkDevice device, uint32_t bindInfoCount,
                                                                    const VkBindAccelerationStructureMemoryInfoNV* pBindInfos,
                                                                    const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache,
                                                              uint32_t createInfoCount,
                                                              const VkRayTracingPipelineCreateInfoNV* pCreateInfos,
                                                              const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                              const RecordObject& record_obj, PipelineStates& pipeline_states) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordGetRayTracingShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline, uint32_t firstGroup,
                                                                     uint32_t groupCount, size_t dataSize, void* pData,
                                                                     const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordGetRayTracingShaderGroupHandlesNV(VkDevice device, VkPipeline pipeline, uint32_t firstGroup,
                                                                    uint32_t groupCount, size_t dataSize, void* pData,
                                                                    const RecordObject& record_obj) {
    PostCallRecordGetRayTracingShaderGroupHandlesKHR(device, pipeline, firstGroup, groupCount, dataSize, pData, record_obj);
}

void BestPractices::PostCallRecordGetAccelerationStructureHandleNV(VkDevice device, VkAccelerationStructureNV accelerationStructure,
                                                                   size_t dataSize, void* pData, const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordCompileDeferredNV(VkDevice device, VkPipeline pipeline, uint32_t shader,
                                                    const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordGetMemoryHostPointerPropertiesEXT(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType,
                                                                    const void* pHostPointer,
                                                                    VkMemoryHostPointerPropertiesEXT* pMemoryHostPointerProperties,
                                                                    const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void bp_state::Instance::PostCallRecordGetPhysicalDeviceCalibrateableTimeDomainsEXT(VkPhysicalDevice physicalDevice,
                                                                                    uint32_t* pTimeDomainCount,
                                                                                    VkTimeDomainKHR* pTimeDomains,
                                                                                    const RecordObject& record_obj) {
    PostCallRecordGetPhysicalDeviceCalibrateableTimeDomainsKHR(physicalDevice, pTimeDomainCount, pTimeDomains, record_obj);
}

void BestPractices::PostCallRecordGetCalibratedTimestampsEXT(VkDevice device, uint32_t timestampCount,
                                                             const VkCalibratedTimestampInfoKHR* pTimestampInfos,
                                                             uint64_t* pTimestamps, uint64_t* pMaxDeviation,
                                                             const RecordObject& record_obj) {
    PostCallRecordGetCalibratedTimestampsKHR(device, timestampCount, pTimestampInfos, pTimestamps, pMaxDeviation, record_obj);
}

void BestPractices::PostCallRecordInitializePerformanceApiINTEL(VkDevice device,
                                                                const VkInitializePerformanceApiInfoINTEL* pInitializeInfo,
                                                                const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordCmdSetPerformanceMarkerINTEL(VkCommandBuffer commandBuffer,
                                                               const VkPerformanceMarkerInfoINTEL* pMarkerInfo,
                                                               const RecordObject& record_obj) {
    bp_state::LogResult(*this, commandBuffer, record_obj);
}

void BestPractices::PostCallRecordCmdSetPerformanceStreamMarkerINTEL(VkCommandBuffer commandBuffer,
                                                                     const VkPerformanceStreamMarkerInfoINTEL* pMarkerInfo,
                                                                     const RecordObject& record_obj) {
    bp_state::LogResult(*this, commandBuffer, record_obj);
}

void BestPractices::PostCallRecordCmdSetPerformanceOverrideINTEL(VkCommandBuffer commandBuffer,
                                                                 const VkPerformanceOverrideInfoINTEL* pOverrideInfo,
                                                                 const RecordObject& record_obj) {
    bp_state::LogResult(*this, commandBuffer, record_obj);
}

void BestPractices::PostCallRecordAcquirePerformanceConfigurationINTEL(
    VkDevice device, const VkPerformanceConfigurationAcquireInfoINTEL* pAcquireInfo,
    VkPerformanceConfigurationINTEL* pConfiguration, const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordReleasePerformanceConfigurationINTEL(VkDevice device,
                                                                       VkPerformanceConfigurationINTEL configuration,
                                                                       const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordQueueSetPerformanceConfigurationINTEL(VkQueue queue,
                                                                        VkPerformanceConfigurationINTEL configuration,
                                                                        const RecordObject& record_obj) {
    bp_state::LogResult(*this, queue, record_obj);
}

void BestPractices::PostCallRecordGetPerformanceParameterINTEL(VkDevice device, VkPerformanceParameterTypeINTEL parameter,
                                                               VkPerformanceValueINTEL* pValue, const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

#ifdef VK_USE_PLATFORM_FUCHSIA
void bp_state::Instance::PostCallRecordCreateImagePipeSurfaceFUCHSIA(VkInstance instance,
                                                                     const VkImagePipeSurfaceCreateInfoFUCHSIA* pCreateInfo,
                                                                     const VkAllocationCallbacks* pAllocator,
                                                                     VkSurfaceKHR* pSurface, const RecordObject& record_obj) {
    bp_state::LogResult(*this, instance, record_obj);
}
#endif  // VK_USE_PLATFORM_FUCHSIA

#ifdef VK_USE_PLATFORM_METAL_EXT
void bp_state::Instance::PostCallRecordCreateMetalSurfaceEXT(VkInstance instance, const VkMetalSurfaceCreateInfoEXT* pCreateInfo,
                                                             const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                             const RecordObject& record_obj) {
    bp_state::LogResult(*this, instance, record_obj);
}
#endif  // VK_USE_PLATFORM_METAL_EXT

void bp_state::Instance::PostCallRecordGetPhysicalDeviceToolPropertiesEXT(VkPhysicalDevice physicalDevice, uint32_t* pToolCount,
                                                                          VkPhysicalDeviceToolProperties* pToolProperties,
                                                                          const RecordObject& record_obj) {
    PostCallRecordGetPhysicalDeviceToolProperties(physicalDevice, pToolCount, pToolProperties, record_obj);
}

void bp_state::Instance::PostCallRecordGetPhysicalDeviceCooperativeMatrixPropertiesNV(VkPhysicalDevice physicalDevice,
                                                                                      uint32_t* pPropertyCount,
                                                                                      VkCooperativeMatrixPropertiesNV* pProperties,
                                                                                      const RecordObject& record_obj) {
    bp_state::LogResult(*this, physicalDevice, record_obj);
}

void bp_state::Instance::PostCallRecordGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(
    VkPhysicalDevice physicalDevice, uint32_t* pCombinationCount, VkFramebufferMixedSamplesCombinationNV* pCombinations,
    const RecordObject& record_obj) {
    bp_state::LogResult(*this, physicalDevice, record_obj);
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
void bp_state::Instance::PostCallRecordGetPhysicalDeviceSurfacePresentModes2EXT(VkPhysicalDevice physicalDevice,
                                                                                const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                                                uint32_t* pPresentModeCount,
                                                                                VkPresentModeKHR* pPresentModes,
                                                                                const RecordObject& record_obj) {
    bp_state::LogResult(*this, physicalDevice, record_obj);
}

void BestPractices::PostCallRecordAcquireFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain,
                                                                    const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordReleaseFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain,
                                                                    const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordGetDeviceGroupSurfacePresentModes2EXT(VkDevice device,
                                                                        const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                                        VkDeviceGroupPresentModeFlagsKHR* pModes,
                                                                        const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

void bp_state::Instance::PostCallRecordCreateHeadlessSurfaceEXT(VkInstance instance,
                                                                const VkHeadlessSurfaceCreateInfoEXT* pCreateInfo,
                                                                const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                                const RecordObject& record_obj) {
    bp_state::LogResult(*this, instance, record_obj);
}

void BestPractices::PostCallRecordCopyMemoryToImageEXT(VkDevice device, const VkCopyMemoryToImageInfo* pCopyMemoryToImageInfo,
                                                       const RecordObject& record_obj) {
    PostCallRecordCopyMemoryToImage(device, pCopyMemoryToImageInfo, record_obj);
}

void BestPractices::PostCallRecordCopyImageToMemoryEXT(VkDevice device, const VkCopyImageToMemoryInfo* pCopyImageToMemoryInfo,
                                                       const RecordObject& record_obj) {
    PostCallRecordCopyImageToMemory(device, pCopyImageToMemoryInfo, record_obj);
}

void BestPractices::PostCallRecordCopyImageToImageEXT(VkDevice device, const VkCopyImageToImageInfo* pCopyImageToImageInfo,
                                                      const RecordObject& record_obj) {
    PostCallRecordCopyImageToImage(device, pCopyImageToImageInfo, record_obj);
}

void BestPractices::PostCallRecordTransitionImageLayoutEXT(VkDevice device, uint32_t transitionCount,
                                                           const VkHostImageLayoutTransitionInfo* pTransitions,
                                                           const RecordObject& record_obj) {
    PostCallRecordTransitionImageLayout(device, transitionCount, pTransitions, record_obj);
}

void BestPractices::PostCallRecordReleaseSwapchainImagesEXT(VkDevice device, const VkReleaseSwapchainImagesInfoEXT* pReleaseInfo,
                                                            const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordCreateIndirectCommandsLayoutNV(VkDevice device,
                                                                 const VkIndirectCommandsLayoutCreateInfoNV* pCreateInfo,
                                                                 const VkAllocationCallbacks* pAllocator,
                                                                 VkIndirectCommandsLayoutNV* pIndirectCommandsLayout,
                                                                 const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void bp_state::Instance::PostCallRecordAcquireDrmDisplayEXT(VkPhysicalDevice physicalDevice, int32_t drmFd, VkDisplayKHR display,
                                                            const RecordObject& record_obj) {
    bp_state::LogResult(*this, physicalDevice, record_obj);
}

void bp_state::Instance::PostCallRecordGetDrmDisplayEXT(VkPhysicalDevice physicalDevice, int32_t drmFd, uint32_t connectorId,
                                                        VkDisplayKHR* display, const RecordObject& record_obj) {
    bp_state::LogResult(*this, physicalDevice, record_obj);
}

void BestPractices::PostCallRecordCreatePrivateDataSlotEXT(VkDevice device, const VkPrivateDataSlotCreateInfo* pCreateInfo,
                                                           const VkAllocationCallbacks* pAllocator,
                                                           VkPrivateDataSlot* pPrivateDataSlot, const RecordObject& record_obj) {
    PostCallRecordCreatePrivateDataSlot(device, pCreateInfo, pAllocator, pPrivateDataSlot, record_obj);
}

void BestPractices::PostCallRecordSetPrivateDataEXT(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                                    VkPrivateDataSlot privateDataSlot, uint64_t data,
                                                    const RecordObject& record_obj) {
    PostCallRecordSetPrivateData(device, objectType, objectHandle, privateDataSlot, data, record_obj);
}

#ifdef VK_ENABLE_BETA_EXTENSIONS
void BestPractices::PostCallRecordCreateCudaModuleNV(VkDevice device, const VkCudaModuleCreateInfoNV* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator, VkCudaModuleNV* pModule,
                                                     const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordGetCudaModuleCacheNV(VkDevice device, VkCudaModuleNV module, size_t* pCacheSize, void* pCacheData,
                                                       const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordCreateCudaFunctionNV(VkDevice device, const VkCudaFunctionCreateInfoNV* pCreateInfo,
                                                       const VkAllocationCallbacks* pAllocator, VkCudaFunctionNV* pFunction,
                                                       const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}
#endif  // VK_ENABLE_BETA_EXTENSIONS

void BestPractices::PostCallRecordGetBufferOpaqueCaptureDescriptorDataEXT(VkDevice device,
                                                                          const VkBufferCaptureDescriptorDataInfoEXT* pInfo,
                                                                          void* pData, const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordGetImageOpaqueCaptureDescriptorDataEXT(VkDevice device,
                                                                         const VkImageCaptureDescriptorDataInfoEXT* pInfo,
                                                                         void* pData, const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordGetImageViewOpaqueCaptureDescriptorDataEXT(VkDevice device,
                                                                             const VkImageViewCaptureDescriptorDataInfoEXT* pInfo,
                                                                             void* pData, const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordGetSamplerOpaqueCaptureDescriptorDataEXT(VkDevice device,
                                                                           const VkSamplerCaptureDescriptorDataInfoEXT* pInfo,
                                                                           void* pData, const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordGetAccelerationStructureOpaqueCaptureDescriptorDataEXT(
    VkDevice device, const VkAccelerationStructureCaptureDescriptorDataInfoEXT* pInfo, void* pData,
    const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordGetDeviceFaultInfoEXT(VkDevice device, VkDeviceFaultCountsEXT* pFaultCounts,
                                                        VkDeviceFaultInfoEXT* pFaultInfo, const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
void bp_state::Instance::PostCallRecordAcquireWinrtDisplayNV(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                             const RecordObject& record_obj) {
    bp_state::LogResult(*this, physicalDevice, record_obj);
}

void bp_state::Instance::PostCallRecordGetWinrtDisplayNV(VkPhysicalDevice physicalDevice, uint32_t deviceRelativeId,
                                                         VkDisplayKHR* pDisplay, const RecordObject& record_obj) {
    bp_state::LogResult(*this, physicalDevice, record_obj);
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_DIRECTFB_EXT
void bp_state::Instance::PostCallRecordCreateDirectFBSurfaceEXT(VkInstance instance,
                                                                const VkDirectFBSurfaceCreateInfoEXT* pCreateInfo,
                                                                const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                                const RecordObject& record_obj) {
    bp_state::LogResult(*this, instance, record_obj);
}
#endif  // VK_USE_PLATFORM_DIRECTFB_EXT

#ifdef VK_USE_PLATFORM_FUCHSIA
void BestPractices::PostCallRecordGetMemoryZirconHandleFUCHSIA(VkDevice device,
                                                               const VkMemoryGetZirconHandleInfoFUCHSIA* pGetZirconHandleInfo,
                                                               zx_handle_t* pZirconHandle, const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordGetMemoryZirconHandlePropertiesFUCHSIA(
    VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, zx_handle_t zirconHandle,
    VkMemoryZirconHandlePropertiesFUCHSIA* pMemoryZirconHandleProperties, const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordImportSemaphoreZirconHandleFUCHSIA(
    VkDevice device, const VkImportSemaphoreZirconHandleInfoFUCHSIA* pImportSemaphoreZirconHandleInfo,
    const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordGetSemaphoreZirconHandleFUCHSIA(VkDevice device,
                                                                  const VkSemaphoreGetZirconHandleInfoFUCHSIA* pGetZirconHandleInfo,
                                                                  zx_handle_t* pZirconHandle, const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordCreateBufferCollectionFUCHSIA(VkDevice device,
                                                                const VkBufferCollectionCreateInfoFUCHSIA* pCreateInfo,
                                                                const VkAllocationCallbacks* pAllocator,
                                                                VkBufferCollectionFUCHSIA* pCollection,
                                                                const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordSetBufferCollectionImageConstraintsFUCHSIA(
    VkDevice device, VkBufferCollectionFUCHSIA collection, const VkImageConstraintsInfoFUCHSIA* pImageConstraintsInfo,
    const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordSetBufferCollectionBufferConstraintsFUCHSIA(
    VkDevice device, VkBufferCollectionFUCHSIA collection, const VkBufferConstraintsInfoFUCHSIA* pBufferConstraintsInfo,
    const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordGetBufferCollectionPropertiesFUCHSIA(VkDevice device, VkBufferCollectionFUCHSIA collection,
                                                                       VkBufferCollectionPropertiesFUCHSIA* pProperties,
                                                                       const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}
#endif  // VK_USE_PLATFORM_FUCHSIA

void BestPractices::PostCallRecordGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI(VkDevice device, VkRenderPass renderpass,
                                                                                VkExtent2D* pMaxWorkgroupSize,
                                                                                const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordGetMemoryRemoteAddressNV(VkDevice device,
                                                           const VkMemoryGetRemoteAddressInfoNV* pMemoryGetRemoteAddressInfo,
                                                           VkRemoteAddressNV* pAddress, const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordGetPipelinePropertiesEXT(VkDevice device, const VkPipelineInfoEXT* pPipelineInfo,
                                                           VkBaseOutStructure* pPipelineProperties,
                                                           const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

#ifdef VK_USE_PLATFORM_SCREEN_QNX
void bp_state::Instance::PostCallRecordCreateScreenSurfaceQNX(VkInstance instance, const VkScreenSurfaceCreateInfoQNX* pCreateInfo,
                                                              const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                              const RecordObject& record_obj) {
    bp_state::LogResult(*this, instance, record_obj);
}
#endif  // VK_USE_PLATFORM_SCREEN_QNX

void BestPractices::PostCallRecordCreateMicromapEXT(VkDevice device, const VkMicromapCreateInfoEXT* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkMicromapEXT* pMicromap,
                                                    const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordBuildMicromapsEXT(VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount,
                                                    const VkMicromapBuildInfoEXT* pInfos, const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordCopyMicromapEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                  const VkCopyMicromapInfoEXT* pInfo, const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordCopyMicromapToMemoryEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                          const VkCopyMicromapToMemoryInfoEXT* pInfo,
                                                          const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordCopyMemoryToMicromapEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                          const VkCopyMemoryToMicromapInfoEXT* pInfo,
                                                          const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordWriteMicromapsPropertiesEXT(VkDevice device, uint32_t micromapCount,
                                                              const VkMicromapEXT* pMicromaps, VkQueryType queryType,
                                                              size_t dataSize, void* pData, size_t stride,
                                                              const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void bp_state::Instance::PostCallRecordGetPhysicalDeviceOpticalFlowImageFormatsNV(
    VkPhysicalDevice physicalDevice, const VkOpticalFlowImageFormatInfoNV* pOpticalFlowImageFormatInfo, uint32_t* pFormatCount,
    VkOpticalFlowImageFormatPropertiesNV* pImageFormatProperties, const RecordObject& record_obj) {
    bp_state::LogResult(*this, physicalDevice, record_obj);
}

void BestPractices::PostCallRecordCreateOpticalFlowSessionNV(VkDevice device, const VkOpticalFlowSessionCreateInfoNV* pCreateInfo,
                                                             const VkAllocationCallbacks* pAllocator,
                                                             VkOpticalFlowSessionNV* pSession, const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordBindOpticalFlowSessionImageNV(VkDevice device, VkOpticalFlowSessionNV session,
                                                                VkOpticalFlowSessionBindingPointNV bindingPoint, VkImageView view,
                                                                VkImageLayout layout, const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordCreateShadersEXT(VkDevice device, uint32_t createInfoCount,
                                                   const VkShaderCreateInfoEXT* pCreateInfos,
                                                   const VkAllocationCallbacks* pAllocator, VkShaderEXT* pShaders,
                                                   const RecordObject& record_obj, chassis::ShaderObject& chassis_state) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordGetShaderBinaryDataEXT(VkDevice device, VkShaderEXT shader, size_t* pDataSize, void* pData,
                                                         const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordGetFramebufferTilePropertiesQCOM(VkDevice device, VkFramebuffer framebuffer,
                                                                   uint32_t* pPropertiesCount, VkTilePropertiesQCOM* pProperties,
                                                                   const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void bp_state::Instance::PostCallRecordGetPhysicalDeviceCooperativeVectorPropertiesNV(VkPhysicalDevice physicalDevice,
                                                                                      uint32_t* pPropertyCount,
                                                                                      VkCooperativeVectorPropertiesNV* pProperties,
                                                                                      const RecordObject& record_obj) {
    bp_state::LogResult(*this, physicalDevice, record_obj);
}

void BestPractices::PostCallRecordConvertCooperativeVectorMatrixNV(VkDevice device,
                                                                   const VkConvertCooperativeVectorMatrixInfoNV* pInfo,
                                                                   const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordSetLatencySleepModeNV(VkDevice device, VkSwapchainKHR swapchain,
                                                        const VkLatencySleepModeInfoNV* pSleepModeInfo,
                                                        const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

#ifdef VK_USE_PLATFORM_SCREEN_QNX
void BestPractices::PostCallRecordGetScreenBufferPropertiesQNX(VkDevice device, const struct _screen_buffer* buffer,
                                                               VkScreenBufferPropertiesQNX* pProperties,
                                                               const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}
#endif  // VK_USE_PLATFORM_SCREEN_QNX

void BestPractices::PostCallRecordCreateExternalComputeQueueNV(VkDevice device,
                                                               const VkExternalComputeQueueCreateInfoNV* pCreateInfo,
                                                               const VkAllocationCallbacks* pAllocator,
                                                               VkExternalComputeQueueNV* pExternalQueue,
                                                               const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordCreateIndirectCommandsLayoutEXT(VkDevice device,
                                                                  const VkIndirectCommandsLayoutCreateInfoEXT* pCreateInfo,
                                                                  const VkAllocationCallbacks* pAllocator,
                                                                  VkIndirectCommandsLayoutEXT* pIndirectCommandsLayout,
                                                                  const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordCreateIndirectExecutionSetEXT(VkDevice device,
                                                                const VkIndirectExecutionSetCreateInfoEXT* pCreateInfo,
                                                                const VkAllocationCallbacks* pAllocator,
                                                                VkIndirectExecutionSetEXT* pIndirectExecutionSet,
                                                                const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void bp_state::Instance::PostCallRecordGetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV(
    VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkCooperativeMatrixFlexibleDimensionsPropertiesNV* pProperties,
    const RecordObject& record_obj) {
    bp_state::LogResult(*this, physicalDevice, record_obj);
}

#ifdef VK_USE_PLATFORM_METAL_EXT
void BestPractices::PostCallRecordGetMemoryMetalHandleEXT(VkDevice device, const VkMemoryGetMetalHandleInfoEXT* pGetMetalHandleInfo,
                                                          void** pHandle, const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordGetMemoryMetalHandlePropertiesEXT(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType,
                                                                    const void* pHandle,
                                                                    VkMemoryMetalHandlePropertiesEXT* pMemoryMetalHandleProperties,
                                                                    const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}
#endif  // VK_USE_PLATFORM_METAL_EXT

void BestPractices::PostCallRecordCreateAccelerationStructureKHR(VkDevice device,
                                                                 const VkAccelerationStructureCreateInfoKHR* pCreateInfo,
                                                                 const VkAllocationCallbacks* pAllocator,
                                                                 VkAccelerationStructureKHR* pAccelerationStructure,
                                                                 const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordBuildAccelerationStructuresKHR(
    VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount,
    const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos, const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordCopyAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                               const VkCopyAccelerationStructureInfoKHR* pInfo,
                                                               const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordCopyAccelerationStructureToMemoryKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                                       const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo,
                                                                       const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordCopyMemoryToAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                                       const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo,
                                                                       const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordWriteAccelerationStructuresPropertiesKHR(
    VkDevice device, uint32_t accelerationStructureCount, const VkAccelerationStructureKHR* pAccelerationStructures,
    VkQueryType queryType, size_t dataSize, void* pData, size_t stride, const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordCreateRayTracingPipelinesKHR(
    VkDevice device, VkDeferredOperationKHR deferredOperation, VkPipelineCache pipelineCache, uint32_t createInfoCount,
    const VkRayTracingPipelineCreateInfoKHR* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
    const RecordObject& record_obj, PipelineStates& pipeline_states,
    std::shared_ptr<chassis::CreateRayTracingPipelinesKHR> chassis_state) {
    bp_state::LogResult(*this, device, record_obj);
}

void BestPractices::PostCallRecordGetRayTracingCaptureReplayShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline,
                                                                                  uint32_t firstGroup, uint32_t groupCount,
                                                                                  size_t dataSize, void* pData,
                                                                                  const RecordObject& record_obj) {
    bp_state::LogResult(*this, device, record_obj);
}

// NOLINTEND
