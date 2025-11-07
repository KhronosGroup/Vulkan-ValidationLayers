// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See extension_helper_generator.py for modifications

/***************************************************************************
 *
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (c) 2015-2025 Google Inc.
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

#pragma once

#include <string>
#include <utility>
#include <vector>
#include <cassert>

#include <vulkan/vulkan.h>
#include "containers/custom_containers.h"
#include "generated/vk_api_version.h"
#include "generated/error_location_helper.h"

// Extensions (unlike functions, struct, etc) are passed in as strings.
// The goal is to turn the string to a enum and pass that around the layers.
// Only when we need to print an error do we try and turn it back into a string
vvl::Extension GetExtension(std::string extension);

enum ExtEnabled : unsigned char {
    kNotSupported,
    kNotEnabled,            // Extension is supported, but not enabled
    kEnabledByCreateinfo,   // Extension is passed at vkCreateDevice/vkCreateInstance time
    kEnabledByApiLevel,     // the API version implicitly enabled it
    kEnabledByInteraction,  // is implicity enabled by another extension
};

// Map of promoted extension information per version (a separate map exists for instance and device extensions).
// The map is keyed by the version number (e.g. VK_API_VERSION_1_1) and each value is a pair consisting of the
// version string (e.g. "VK_VERSION_1_1") and the set of name of the promoted extensions.
typedef vvl::unordered_map<uint32_t, std::pair<const char *, vvl::unordered_set<vvl::Extension>>> PromotedExtensionInfoMap;
const PromotedExtensionInfoMap &GetInstancePromotionInfoMap();
const PromotedExtensionInfoMap &GetDevicePromotionInfoMap();

/*
This function is a helper to know if the extension is enabled.

Times to use it
- To determine the VUID
- The VU mentions the use of the extension
- Extension exposes property limits being validated
- Checking not enabled
    - if (!IsExtEnabled(...)) { }
- Special extensions that being EXPOSED alters the VUs
    - IsExtEnabled(extensions.vk_khr_portability_subset)
- Special extensions that alter behaviour of enabled
    - IsExtEnabled(extensions.vk_khr_maintenance*)

Times to NOT use it
    - If checking if a struct or enum is being used. There are a stateless checks
      to make sure the new Structs/Enums are not being used without this enabled.
    - If checking if the extension's feature enable status, because if the feature
      is enabled, then we already validated that extension is enabled.
    - Some variables (ex. viewMask) require the extension to be used if non-zero
*/
[[maybe_unused]] static bool IsExtEnabled(ExtEnabled extension) {
    return (extension == kEnabledByCreateinfo || extension == kEnabledByApiLevel || extension == kEnabledByInteraction);
}

[[maybe_unused]] static bool IsExtSupported(ExtEnabled extension) { return (extension != kNotSupported); }

[[maybe_unused]] static bool IsExtEnabledByCreateinfo(ExtEnabled extension) { return (extension == kEnabledByCreateinfo); }

struct InstanceExtensions {
    APIVersion api_version{};
    ExtEnabled vk_feature_version_1_1{kNotSupported};
    ExtEnabled vk_feature_version_1_2{kNotSupported};
    ExtEnabled vk_feature_version_1_3{kNotSupported};
    ExtEnabled vk_feature_version_1_4{kNotSupported};
    ExtEnabled vk_khr_surface{kNotSupported};
    ExtEnabled vk_khr_display{kNotSupported};
    ExtEnabled vk_khr_xlib_surface{kNotSupported};
    ExtEnabled vk_khr_xcb_surface{kNotSupported};
    ExtEnabled vk_khr_wayland_surface{kNotSupported};
    ExtEnabled vk_khr_android_surface{kNotSupported};
    ExtEnabled vk_khr_win32_surface{kNotSupported};
    ExtEnabled vk_khr_get_physical_device_properties2{kNotSupported};
    ExtEnabled vk_khr_device_group_creation{kNotSupported};
    ExtEnabled vk_khr_external_memory_capabilities{kNotSupported};
    ExtEnabled vk_khr_external_semaphore_capabilities{kNotSupported};
    ExtEnabled vk_khr_external_fence_capabilities{kNotSupported};
    ExtEnabled vk_khr_get_surface_capabilities2{kNotSupported};
    ExtEnabled vk_khr_get_display_properties2{kNotSupported};
    ExtEnabled vk_khr_surface_protected_capabilities{kNotSupported};
    ExtEnabled vk_khr_portability_enumeration{kNotSupported};
    ExtEnabled vk_khr_surface_maintenance1{kNotSupported};
    ExtEnabled vk_ext_debug_report{kNotSupported};
    ExtEnabled vk_ggp_stream_descriptor_surface{kNotSupported};
    ExtEnabled vk_nv_external_memory_capabilities{kNotSupported};
    ExtEnabled vk_ext_validation_flags{kNotSupported};
    ExtEnabled vk_nn_vi_surface{kNotSupported};
    ExtEnabled vk_ext_direct_mode_display{kNotSupported};
    ExtEnabled vk_ext_acquire_xlib_display{kNotSupported};
    ExtEnabled vk_ext_display_surface_counter{kNotSupported};
    ExtEnabled vk_ext_swapchain_colorspace{kNotSupported};
    ExtEnabled vk_mvk_ios_surface{kNotSupported};
    ExtEnabled vk_mvk_macos_surface{kNotSupported};
    ExtEnabled vk_ext_debug_utils{kNotSupported};
    ExtEnabled vk_fuchsia_imagepipe_surface{kNotSupported};
    ExtEnabled vk_ext_metal_surface{kNotSupported};
    ExtEnabled vk_ext_validation_features{kNotSupported};
    ExtEnabled vk_ext_headless_surface{kNotSupported};
    ExtEnabled vk_ext_surface_maintenance1{kNotSupported};
    ExtEnabled vk_ext_acquire_drm_display{kNotSupported};
    ExtEnabled vk_ext_directfb_surface{kNotSupported};
    ExtEnabled vk_qnx_screen_surface{kNotSupported};
    ExtEnabled vk_google_surfaceless_query{kNotSupported};
    ExtEnabled vk_lunarg_direct_driver_loading{kNotSupported};
    ExtEnabled vk_ext_layer_settings{kNotSupported};
    ExtEnabled vk_nv_display_stereo{kNotSupported};
    ExtEnabled vk_ohos_surface{kNotSupported};

    struct Requirement {
        const ExtEnabled InstanceExtensions::*enabled;
        const char *name;
    };
    typedef std::vector<Requirement> RequirementVec;
    struct Info {
        Info(ExtEnabled InstanceExtensions::*state_, const RequirementVec requirements_)
            : state(state_), requirements(requirements_) {}
        ExtEnabled InstanceExtensions::*state;
        RequirementVec requirements;
    };

    const Info &GetInfo(vvl::Extension extension_name) const;

    InstanceExtensions() = default;
    InstanceExtensions(APIVersion requested_api_version, const VkInstanceCreateInfo *pCreateInfo);
};

struct DeviceExtensions : public InstanceExtensions {
    ExtEnabled vk_feature_version_1_1{kNotSupported};
    ExtEnabled vk_feature_version_1_2{kNotSupported};
    ExtEnabled vk_feature_version_1_3{kNotSupported};
    ExtEnabled vk_feature_version_1_4{kNotSupported};
    ExtEnabled vk_khr_swapchain{kNotSupported};
    ExtEnabled vk_khr_display_swapchain{kNotSupported};
    ExtEnabled vk_khr_sampler_mirror_clamp_to_edge{kNotSupported};
    ExtEnabled vk_khr_video_queue{kNotSupported};
    ExtEnabled vk_khr_video_decode_queue{kNotSupported};
    ExtEnabled vk_khr_video_encode_h264{kNotSupported};
    ExtEnabled vk_khr_video_encode_h265{kNotSupported};
    ExtEnabled vk_khr_video_decode_h264{kNotSupported};
    ExtEnabled vk_khr_dynamic_rendering{kNotSupported};
    ExtEnabled vk_khr_multiview{kNotSupported};
    ExtEnabled vk_khr_device_group{kNotSupported};
    ExtEnabled vk_khr_shader_draw_parameters{kNotSupported};
    ExtEnabled vk_khr_maintenance1{kNotSupported};
    ExtEnabled vk_khr_external_memory{kNotSupported};
    ExtEnabled vk_khr_external_memory_win32{kNotSupported};
    ExtEnabled vk_khr_external_memory_fd{kNotSupported};
    ExtEnabled vk_khr_win32_keyed_mutex{kNotSupported};
    ExtEnabled vk_khr_external_semaphore{kNotSupported};
    ExtEnabled vk_khr_external_semaphore_win32{kNotSupported};
    ExtEnabled vk_khr_external_semaphore_fd{kNotSupported};
    ExtEnabled vk_khr_push_descriptor{kNotSupported};
    ExtEnabled vk_khr_shader_float16_int8{kNotSupported};
    ExtEnabled vk_khr_16bit_storage{kNotSupported};
    ExtEnabled vk_khr_incremental_present{kNotSupported};
    ExtEnabled vk_khr_descriptor_update_template{kNotSupported};
    ExtEnabled vk_khr_imageless_framebuffer{kNotSupported};
    ExtEnabled vk_khr_create_renderpass2{kNotSupported};
    ExtEnabled vk_khr_shared_presentable_image{kNotSupported};
    ExtEnabled vk_khr_external_fence{kNotSupported};
    ExtEnabled vk_khr_external_fence_win32{kNotSupported};
    ExtEnabled vk_khr_external_fence_fd{kNotSupported};
    ExtEnabled vk_khr_performance_query{kNotSupported};
    ExtEnabled vk_khr_maintenance2{kNotSupported};
    ExtEnabled vk_khr_variable_pointers{kNotSupported};
    ExtEnabled vk_khr_dedicated_allocation{kNotSupported};
    ExtEnabled vk_khr_storage_buffer_storage_class{kNotSupported};
    ExtEnabled vk_khr_shader_bfloat16{kNotSupported};
    ExtEnabled vk_khr_relaxed_block_layout{kNotSupported};
    ExtEnabled vk_khr_get_memory_requirements2{kNotSupported};
    ExtEnabled vk_khr_image_format_list{kNotSupported};
    ExtEnabled vk_khr_sampler_ycbcr_conversion{kNotSupported};
    ExtEnabled vk_khr_bind_memory2{kNotSupported};
    ExtEnabled vk_khr_portability_subset{kNotSupported};
    ExtEnabled vk_khr_maintenance3{kNotSupported};
    ExtEnabled vk_khr_draw_indirect_count{kNotSupported};
    ExtEnabled vk_khr_shader_subgroup_extended_types{kNotSupported};
    ExtEnabled vk_khr_8bit_storage{kNotSupported};
    ExtEnabled vk_khr_shader_atomic_int64{kNotSupported};
    ExtEnabled vk_khr_shader_clock{kNotSupported};
    ExtEnabled vk_khr_video_decode_h265{kNotSupported};
    ExtEnabled vk_khr_global_priority{kNotSupported};
    ExtEnabled vk_khr_driver_properties{kNotSupported};
    ExtEnabled vk_khr_shader_float_controls{kNotSupported};
    ExtEnabled vk_khr_depth_stencil_resolve{kNotSupported};
    ExtEnabled vk_khr_swapchain_mutable_format{kNotSupported};
    ExtEnabled vk_khr_timeline_semaphore{kNotSupported};
    ExtEnabled vk_khr_vulkan_memory_model{kNotSupported};
    ExtEnabled vk_khr_shader_terminate_invocation{kNotSupported};
    ExtEnabled vk_khr_fragment_shading_rate{kNotSupported};
    ExtEnabled vk_khr_dynamic_rendering_local_read{kNotSupported};
    ExtEnabled vk_khr_shader_quad_control{kNotSupported};
    ExtEnabled vk_khr_spirv_1_4{kNotSupported};
    ExtEnabled vk_khr_separate_depth_stencil_layouts{kNotSupported};
    ExtEnabled vk_khr_present_wait{kNotSupported};
    ExtEnabled vk_khr_uniform_buffer_standard_layout{kNotSupported};
    ExtEnabled vk_khr_buffer_device_address{kNotSupported};
    ExtEnabled vk_khr_deferred_host_operations{kNotSupported};
    ExtEnabled vk_khr_pipeline_executable_properties{kNotSupported};
    ExtEnabled vk_khr_map_memory2{kNotSupported};
    ExtEnabled vk_khr_shader_integer_dot_product{kNotSupported};
    ExtEnabled vk_khr_pipeline_library{kNotSupported};
    ExtEnabled vk_khr_shader_non_semantic_info{kNotSupported};
    ExtEnabled vk_khr_present_id{kNotSupported};
    ExtEnabled vk_khr_video_encode_queue{kNotSupported};
    ExtEnabled vk_khr_synchronization2{kNotSupported};
    ExtEnabled vk_khr_fragment_shader_barycentric{kNotSupported};
    ExtEnabled vk_khr_shader_subgroup_uniform_control_flow{kNotSupported};
    ExtEnabled vk_khr_zero_initialize_workgroup_memory{kNotSupported};
    ExtEnabled vk_khr_workgroup_memory_explicit_layout{kNotSupported};
    ExtEnabled vk_khr_copy_commands2{kNotSupported};
    ExtEnabled vk_khr_format_feature_flags2{kNotSupported};
    ExtEnabled vk_khr_ray_tracing_maintenance1{kNotSupported};
    ExtEnabled vk_khr_shader_untyped_pointers{kNotSupported};
    ExtEnabled vk_khr_maintenance4{kNotSupported};
    ExtEnabled vk_khr_shader_subgroup_rotate{kNotSupported};
    ExtEnabled vk_khr_shader_maximal_reconvergence{kNotSupported};
    ExtEnabled vk_khr_maintenance5{kNotSupported};
    ExtEnabled vk_khr_present_id2{kNotSupported};
    ExtEnabled vk_khr_present_wait2{kNotSupported};
    ExtEnabled vk_khr_ray_tracing_position_fetch{kNotSupported};
    ExtEnabled vk_khr_pipeline_binary{kNotSupported};
    ExtEnabled vk_khr_swapchain_maintenance1{kNotSupported};
    ExtEnabled vk_khr_cooperative_matrix{kNotSupported};
    ExtEnabled vk_khr_compute_shader_derivatives{kNotSupported};
    ExtEnabled vk_khr_video_decode_av1{kNotSupported};
    ExtEnabled vk_khr_video_encode_av1{kNotSupported};
    ExtEnabled vk_khr_video_decode_vp9{kNotSupported};
    ExtEnabled vk_khr_video_maintenance1{kNotSupported};
    ExtEnabled vk_khr_vertex_attribute_divisor{kNotSupported};
    ExtEnabled vk_khr_load_store_op_none{kNotSupported};
    ExtEnabled vk_khr_unified_image_layouts{kNotSupported};
    ExtEnabled vk_khr_shader_float_controls2{kNotSupported};
    ExtEnabled vk_khr_index_type_uint8{kNotSupported};
    ExtEnabled vk_khr_line_rasterization{kNotSupported};
    ExtEnabled vk_khr_calibrated_timestamps{kNotSupported};
    ExtEnabled vk_khr_shader_expect_assume{kNotSupported};
    ExtEnabled vk_khr_maintenance6{kNotSupported};
    ExtEnabled vk_khr_copy_memory_indirect{kNotSupported};
    ExtEnabled vk_khr_video_encode_intra_refresh{kNotSupported};
    ExtEnabled vk_khr_video_encode_quantization_map{kNotSupported};
    ExtEnabled vk_khr_shader_relaxed_extended_instruction{kNotSupported};
    ExtEnabled vk_khr_maintenance7{kNotSupported};
    ExtEnabled vk_khr_maintenance8{kNotSupported};
    ExtEnabled vk_khr_shader_fma{kNotSupported};
    ExtEnabled vk_khr_maintenance9{kNotSupported};
    ExtEnabled vk_khr_video_maintenance2{kNotSupported};
    ExtEnabled vk_khr_depth_clamp_zero_one{kNotSupported};
    ExtEnabled vk_khr_robustness2{kNotSupported};
    ExtEnabled vk_khr_present_mode_fifo_latest_ready{kNotSupported};
    ExtEnabled vk_khr_maintenance10{kNotSupported};
    ExtEnabled vk_nv_glsl_shader{kNotSupported};
    ExtEnabled vk_ext_depth_range_unrestricted{kNotSupported};
    ExtEnabled vk_img_filter_cubic{kNotSupported};
    ExtEnabled vk_amd_rasterization_order{kNotSupported};
    ExtEnabled vk_amd_shader_trinary_minmax{kNotSupported};
    ExtEnabled vk_amd_shader_explicit_vertex_parameter{kNotSupported};
    ExtEnabled vk_ext_debug_marker{kNotSupported};
    ExtEnabled vk_amd_gcn_shader{kNotSupported};
    ExtEnabled vk_nv_dedicated_allocation{kNotSupported};
    ExtEnabled vk_ext_transform_feedback{kNotSupported};
    ExtEnabled vk_nvx_binary_import{kNotSupported};
    ExtEnabled vk_nvx_image_view_handle{kNotSupported};
    ExtEnabled vk_amd_draw_indirect_count{kNotSupported};
    ExtEnabled vk_amd_negative_viewport_height{kNotSupported};
    ExtEnabled vk_amd_gpu_shader_half_float{kNotSupported};
    ExtEnabled vk_amd_shader_ballot{kNotSupported};
    ExtEnabled vk_amd_texture_gather_bias_lod{kNotSupported};
    ExtEnabled vk_amd_shader_info{kNotSupported};
    ExtEnabled vk_amd_shader_image_load_store_lod{kNotSupported};
    ExtEnabled vk_nv_corner_sampled_image{kNotSupported};
    ExtEnabled vk_img_format_pvrtc{kNotSupported};
    ExtEnabled vk_nv_external_memory{kNotSupported};
    ExtEnabled vk_nv_external_memory_win32{kNotSupported};
    ExtEnabled vk_nv_win32_keyed_mutex{kNotSupported};
    ExtEnabled vk_ext_shader_subgroup_ballot{kNotSupported};
    ExtEnabled vk_ext_shader_subgroup_vote{kNotSupported};
    ExtEnabled vk_ext_texture_compression_astc_hdr{kNotSupported};
    ExtEnabled vk_ext_astc_decode_mode{kNotSupported};
    ExtEnabled vk_ext_pipeline_robustness{kNotSupported};
    ExtEnabled vk_ext_conditional_rendering{kNotSupported};
    ExtEnabled vk_nv_clip_space_w_scaling{kNotSupported};
    ExtEnabled vk_ext_display_control{kNotSupported};
    ExtEnabled vk_google_display_timing{kNotSupported};
    ExtEnabled vk_nv_sample_mask_override_coverage{kNotSupported};
    ExtEnabled vk_nv_geometry_shader_passthrough{kNotSupported};
    ExtEnabled vk_nv_viewport_array2{kNotSupported};
    ExtEnabled vk_nvx_multiview_per_view_attributes{kNotSupported};
    ExtEnabled vk_nv_viewport_swizzle{kNotSupported};
    ExtEnabled vk_ext_discard_rectangles{kNotSupported};
    ExtEnabled vk_ext_conservative_rasterization{kNotSupported};
    ExtEnabled vk_ext_depth_clip_enable{kNotSupported};
    ExtEnabled vk_ext_hdr_metadata{kNotSupported};
    ExtEnabled vk_img_relaxed_line_rasterization{kNotSupported};
    ExtEnabled vk_ext_external_memory_dma_buf{kNotSupported};
    ExtEnabled vk_ext_queue_family_foreign{kNotSupported};
    ExtEnabled vk_android_external_memory_android_hardware_buffer{kNotSupported};
    ExtEnabled vk_ext_sampler_filter_minmax{kNotSupported};
    ExtEnabled vk_amd_gpu_shader_int16{kNotSupported};
    ExtEnabled vk_amdx_shader_enqueue{kNotSupported};
    ExtEnabled vk_amd_mixed_attachment_samples{kNotSupported};
    ExtEnabled vk_amd_shader_fragment_mask{kNotSupported};
    ExtEnabled vk_ext_inline_uniform_block{kNotSupported};
    ExtEnabled vk_ext_shader_stencil_export{kNotSupported};
    ExtEnabled vk_ext_sample_locations{kNotSupported};
    ExtEnabled vk_ext_blend_operation_advanced{kNotSupported};
    ExtEnabled vk_nv_fragment_coverage_to_color{kNotSupported};
    ExtEnabled vk_nv_framebuffer_mixed_samples{kNotSupported};
    ExtEnabled vk_nv_fill_rectangle{kNotSupported};
    ExtEnabled vk_nv_shader_sm_builtins{kNotSupported};
    ExtEnabled vk_ext_post_depth_coverage{kNotSupported};
    ExtEnabled vk_ext_image_drm_format_modifier{kNotSupported};
    ExtEnabled vk_ext_validation_cache{kNotSupported};
    ExtEnabled vk_ext_descriptor_indexing{kNotSupported};
    ExtEnabled vk_ext_shader_viewport_index_layer{kNotSupported};
    ExtEnabled vk_nv_shading_rate_image{kNotSupported};
    ExtEnabled vk_nv_ray_tracing{kNotSupported};
    ExtEnabled vk_nv_representative_fragment_test{kNotSupported};
    ExtEnabled vk_ext_filter_cubic{kNotSupported};
    ExtEnabled vk_qcom_render_pass_shader_resolve{kNotSupported};
    ExtEnabled vk_ext_global_priority{kNotSupported};
    ExtEnabled vk_ext_external_memory_host{kNotSupported};
    ExtEnabled vk_amd_buffer_marker{kNotSupported};
    ExtEnabled vk_amd_pipeline_compiler_control{kNotSupported};
    ExtEnabled vk_ext_calibrated_timestamps{kNotSupported};
    ExtEnabled vk_amd_shader_core_properties{kNotSupported};
    ExtEnabled vk_amd_memory_overallocation_behavior{kNotSupported};
    ExtEnabled vk_ext_vertex_attribute_divisor{kNotSupported};
    ExtEnabled vk_ggp_frame_token{kNotSupported};
    ExtEnabled vk_ext_pipeline_creation_feedback{kNotSupported};
    ExtEnabled vk_nv_shader_subgroup_partitioned{kNotSupported};
    ExtEnabled vk_nv_compute_shader_derivatives{kNotSupported};
    ExtEnabled vk_nv_mesh_shader{kNotSupported};
    ExtEnabled vk_nv_fragment_shader_barycentric{kNotSupported};
    ExtEnabled vk_nv_shader_image_footprint{kNotSupported};
    ExtEnabled vk_nv_scissor_exclusive{kNotSupported};
    ExtEnabled vk_nv_device_diagnostic_checkpoints{kNotSupported};
    ExtEnabled vk_intel_shader_integer_functions2{kNotSupported};
    ExtEnabled vk_intel_performance_query{kNotSupported};
    ExtEnabled vk_ext_pci_bus_info{kNotSupported};
    ExtEnabled vk_amd_display_native_hdr{kNotSupported};
    ExtEnabled vk_ext_fragment_density_map{kNotSupported};
    ExtEnabled vk_ext_scalar_block_layout{kNotSupported};
    ExtEnabled vk_google_hlsl_functionality1{kNotSupported};
    ExtEnabled vk_google_decorate_string{kNotSupported};
    ExtEnabled vk_ext_subgroup_size_control{kNotSupported};
    ExtEnabled vk_amd_shader_core_properties2{kNotSupported};
    ExtEnabled vk_amd_device_coherent_memory{kNotSupported};
    ExtEnabled vk_ext_shader_image_atomic_int64{kNotSupported};
    ExtEnabled vk_ext_memory_budget{kNotSupported};
    ExtEnabled vk_ext_memory_priority{kNotSupported};
    ExtEnabled vk_nv_dedicated_allocation_image_aliasing{kNotSupported};
    ExtEnabled vk_ext_buffer_device_address{kNotSupported};
    ExtEnabled vk_ext_tooling_info{kNotSupported};
    ExtEnabled vk_ext_separate_stencil_usage{kNotSupported};
    ExtEnabled vk_nv_cooperative_matrix{kNotSupported};
    ExtEnabled vk_nv_coverage_reduction_mode{kNotSupported};
    ExtEnabled vk_ext_fragment_shader_interlock{kNotSupported};
    ExtEnabled vk_ext_ycbcr_image_arrays{kNotSupported};
    ExtEnabled vk_ext_provoking_vertex{kNotSupported};
    ExtEnabled vk_ext_full_screen_exclusive{kNotSupported};
    ExtEnabled vk_ext_line_rasterization{kNotSupported};
    ExtEnabled vk_ext_shader_atomic_float{kNotSupported};
    ExtEnabled vk_ext_host_query_reset{kNotSupported};
    ExtEnabled vk_ext_index_type_uint8{kNotSupported};
    ExtEnabled vk_ext_extended_dynamic_state{kNotSupported};
    ExtEnabled vk_ext_host_image_copy{kNotSupported};
    ExtEnabled vk_ext_map_memory_placed{kNotSupported};
    ExtEnabled vk_ext_shader_atomic_float2{kNotSupported};
    ExtEnabled vk_ext_swapchain_maintenance1{kNotSupported};
    ExtEnabled vk_ext_shader_demote_to_helper_invocation{kNotSupported};
    ExtEnabled vk_nv_device_generated_commands{kNotSupported};
    ExtEnabled vk_nv_inherited_viewport_scissor{kNotSupported};
    ExtEnabled vk_ext_texel_buffer_alignment{kNotSupported};
    ExtEnabled vk_qcom_render_pass_transform{kNotSupported};
    ExtEnabled vk_ext_depth_bias_control{kNotSupported};
    ExtEnabled vk_ext_device_memory_report{kNotSupported};
    ExtEnabled vk_ext_robustness2{kNotSupported};
    ExtEnabled vk_ext_custom_border_color{kNotSupported};
    ExtEnabled vk_google_user_type{kNotSupported};
    ExtEnabled vk_nv_present_barrier{kNotSupported};
    ExtEnabled vk_ext_private_data{kNotSupported};
    ExtEnabled vk_ext_pipeline_creation_cache_control{kNotSupported};
    ExtEnabled vk_nv_device_diagnostics_config{kNotSupported};
    ExtEnabled vk_qcom_render_pass_store_ops{kNotSupported};
    ExtEnabled vk_nv_cuda_kernel_launch{kNotSupported};
    ExtEnabled vk_qcom_tile_shading{kNotSupported};
    ExtEnabled vk_nv_low_latency{kNotSupported};
    ExtEnabled vk_ext_metal_objects{kNotSupported};
    ExtEnabled vk_ext_descriptor_buffer{kNotSupported};
    ExtEnabled vk_ext_graphics_pipeline_library{kNotSupported};
    ExtEnabled vk_amd_shader_early_and_late_fragment_tests{kNotSupported};
    ExtEnabled vk_nv_fragment_shading_rate_enums{kNotSupported};
    ExtEnabled vk_nv_ray_tracing_motion_blur{kNotSupported};
    ExtEnabled vk_ext_ycbcr_2plane_444_formats{kNotSupported};
    ExtEnabled vk_ext_fragment_density_map2{kNotSupported};
    ExtEnabled vk_qcom_rotated_copy_commands{kNotSupported};
    ExtEnabled vk_ext_image_robustness{kNotSupported};
    ExtEnabled vk_ext_image_compression_control{kNotSupported};
    ExtEnabled vk_ext_attachment_feedback_loop_layout{kNotSupported};
    ExtEnabled vk_ext_4444_formats{kNotSupported};
    ExtEnabled vk_ext_device_fault{kNotSupported};
    ExtEnabled vk_arm_rasterization_order_attachment_access{kNotSupported};
    ExtEnabled vk_ext_rgba10x6_formats{kNotSupported};
    ExtEnabled vk_nv_acquire_winrt_display{kNotSupported};
    ExtEnabled vk_valve_mutable_descriptor_type{kNotSupported};
    ExtEnabled vk_ext_vertex_input_dynamic_state{kNotSupported};
    ExtEnabled vk_ext_physical_device_drm{kNotSupported};
    ExtEnabled vk_ext_device_address_binding_report{kNotSupported};
    ExtEnabled vk_ext_depth_clip_control{kNotSupported};
    ExtEnabled vk_ext_primitive_topology_list_restart{kNotSupported};
    ExtEnabled vk_ext_present_mode_fifo_latest_ready{kNotSupported};
    ExtEnabled vk_fuchsia_external_memory{kNotSupported};
    ExtEnabled vk_fuchsia_external_semaphore{kNotSupported};
    ExtEnabled vk_fuchsia_buffer_collection{kNotSupported};
    ExtEnabled vk_huawei_subpass_shading{kNotSupported};
    ExtEnabled vk_huawei_invocation_mask{kNotSupported};
    ExtEnabled vk_nv_external_memory_rdma{kNotSupported};
    ExtEnabled vk_ext_pipeline_properties{kNotSupported};
    ExtEnabled vk_ext_frame_boundary{kNotSupported};
    ExtEnabled vk_ext_multisampled_render_to_single_sampled{kNotSupported};
    ExtEnabled vk_ext_extended_dynamic_state2{kNotSupported};
    ExtEnabled vk_ext_color_write_enable{kNotSupported};
    ExtEnabled vk_ext_primitives_generated_query{kNotSupported};
    ExtEnabled vk_ext_global_priority_query{kNotSupported};
    ExtEnabled vk_valve_video_encode_rgb_conversion{kNotSupported};
    ExtEnabled vk_ext_image_view_min_lod{kNotSupported};
    ExtEnabled vk_ext_multi_draw{kNotSupported};
    ExtEnabled vk_ext_image_2d_view_of_3d{kNotSupported};
    ExtEnabled vk_ext_shader_tile_image{kNotSupported};
    ExtEnabled vk_ext_opacity_micromap{kNotSupported};
    ExtEnabled vk_nv_displacement_micromap{kNotSupported};
    ExtEnabled vk_ext_load_store_op_none{kNotSupported};
    ExtEnabled vk_huawei_cluster_culling_shader{kNotSupported};
    ExtEnabled vk_ext_border_color_swizzle{kNotSupported};
    ExtEnabled vk_ext_pageable_device_local_memory{kNotSupported};
    ExtEnabled vk_arm_shader_core_properties{kNotSupported};
    ExtEnabled vk_arm_scheduling_controls{kNotSupported};
    ExtEnabled vk_ext_image_sliced_view_of_3d{kNotSupported};
    ExtEnabled vk_valve_descriptor_set_host_mapping{kNotSupported};
    ExtEnabled vk_ext_depth_clamp_zero_one{kNotSupported};
    ExtEnabled vk_ext_non_seamless_cube_map{kNotSupported};
    ExtEnabled vk_arm_render_pass_striped{kNotSupported};
    ExtEnabled vk_qcom_fragment_density_map_offset{kNotSupported};
    ExtEnabled vk_nv_copy_memory_indirect{kNotSupported};
    ExtEnabled vk_nv_memory_decompression{kNotSupported};
    ExtEnabled vk_nv_device_generated_commands_compute{kNotSupported};
    ExtEnabled vk_nv_ray_tracing_linear_swept_spheres{kNotSupported};
    ExtEnabled vk_nv_linear_color_attachment{kNotSupported};
    ExtEnabled vk_ext_image_compression_control_swapchain{kNotSupported};
    ExtEnabled vk_qcom_image_processing{kNotSupported};
    ExtEnabled vk_ext_nested_command_buffer{kNotSupported};
    ExtEnabled vk_ohos_external_memory{kNotSupported};
    ExtEnabled vk_ext_external_memory_acquire_unmodified{kNotSupported};
    ExtEnabled vk_ext_extended_dynamic_state3{kNotSupported};
    ExtEnabled vk_ext_subpass_merge_feedback{kNotSupported};
    ExtEnabled vk_arm_tensors{kNotSupported};
    ExtEnabled vk_ext_shader_module_identifier{kNotSupported};
    ExtEnabled vk_ext_rasterization_order_attachment_access{kNotSupported};
    ExtEnabled vk_nv_optical_flow{kNotSupported};
    ExtEnabled vk_ext_legacy_dithering{kNotSupported};
    ExtEnabled vk_ext_pipeline_protected_access{kNotSupported};
    ExtEnabled vk_android_external_format_resolve{kNotSupported};
    ExtEnabled vk_amd_anti_lag{kNotSupported};
    ExtEnabled vk_amdx_dense_geometry_format{kNotSupported};
    ExtEnabled vk_ext_shader_object{kNotSupported};
    ExtEnabled vk_qcom_tile_properties{kNotSupported};
    ExtEnabled vk_sec_amigo_profiling{kNotSupported};
    ExtEnabled vk_qcom_multiview_per_view_viewports{kNotSupported};
    ExtEnabled vk_nv_ray_tracing_invocation_reorder{kNotSupported};
    ExtEnabled vk_nv_cooperative_vector{kNotSupported};
    ExtEnabled vk_nv_extended_sparse_address_space{kNotSupported};
    ExtEnabled vk_ext_mutable_descriptor_type{kNotSupported};
    ExtEnabled vk_ext_legacy_vertex_attributes{kNotSupported};
    ExtEnabled vk_arm_shader_core_builtins{kNotSupported};
    ExtEnabled vk_ext_pipeline_library_group_handles{kNotSupported};
    ExtEnabled vk_ext_dynamic_rendering_unused_attachments{kNotSupported};
    ExtEnabled vk_nv_low_latency2{kNotSupported};
    ExtEnabled vk_arm_data_graph{kNotSupported};
    ExtEnabled vk_qcom_multiview_per_view_render_areas{kNotSupported};
    ExtEnabled vk_nv_per_stage_descriptor_set{kNotSupported};
    ExtEnabled vk_qcom_image_processing2{kNotSupported};
    ExtEnabled vk_qcom_filter_cubic_weights{kNotSupported};
    ExtEnabled vk_qcom_ycbcr_degamma{kNotSupported};
    ExtEnabled vk_qcom_filter_cubic_clamp{kNotSupported};
    ExtEnabled vk_ext_attachment_feedback_loop_dynamic_state{kNotSupported};
    ExtEnabled vk_qnx_external_memory_screen_buffer{kNotSupported};
    ExtEnabled vk_msft_layered_driver{kNotSupported};
    ExtEnabled vk_nv_descriptor_pool_overallocation{kNotSupported};
    ExtEnabled vk_qcom_tile_memory_heap{kNotSupported};
    ExtEnabled vk_ext_memory_decompression{kNotSupported};
    ExtEnabled vk_nv_raw_access_chains{kNotSupported};
    ExtEnabled vk_nv_external_compute_queue{kNotSupported};
    ExtEnabled vk_nv_command_buffer_inheritance{kNotSupported};
    ExtEnabled vk_nv_shader_atomic_float16_vector{kNotSupported};
    ExtEnabled vk_ext_shader_replicated_composites{kNotSupported};
    ExtEnabled vk_ext_shader_float8{kNotSupported};
    ExtEnabled vk_nv_ray_tracing_validation{kNotSupported};
    ExtEnabled vk_nv_cluster_acceleration_structure{kNotSupported};
    ExtEnabled vk_nv_partitioned_acceleration_structure{kNotSupported};
    ExtEnabled vk_ext_device_generated_commands{kNotSupported};
    ExtEnabled vk_mesa_image_alignment_control{kNotSupported};
    ExtEnabled vk_ext_depth_clamp_control{kNotSupported};
    ExtEnabled vk_ohos_native_buffer{kNotSupported};
    ExtEnabled vk_huawei_hdr_vivid{kNotSupported};
    ExtEnabled vk_nv_cooperative_matrix2{kNotSupported};
    ExtEnabled vk_arm_pipeline_opacity_micromap{kNotSupported};
    ExtEnabled vk_ext_external_memory_metal{kNotSupported};
    ExtEnabled vk_arm_performance_counters_by_region{kNotSupported};
    ExtEnabled vk_ext_vertex_attribute_robustness{kNotSupported};
    ExtEnabled vk_arm_format_pack{kNotSupported};
    ExtEnabled vk_valve_fragment_density_map_layered{kNotSupported};
    ExtEnabled vk_nv_present_metering{kNotSupported};
    ExtEnabled vk_ext_fragment_density_map_offset{kNotSupported};
    ExtEnabled vk_ext_zero_initialize_device_memory{kNotSupported};
    ExtEnabled vk_ext_shader_64bit_indexing{kNotSupported};
    ExtEnabled vk_qcom_data_graph_model{kNotSupported};
    ExtEnabled vk_sec_pipeline_cache_incremental_mode{kNotSupported};
    ExtEnabled vk_ext_shader_uniform_buffer_unsized_array{kNotSupported};
    ExtEnabled vk_khr_acceleration_structure{kNotSupported};
    ExtEnabled vk_khr_ray_tracing_pipeline{kNotSupported};
    ExtEnabled vk_khr_ray_query{kNotSupported};
    ExtEnabled vk_ext_mesh_shader{kNotSupported};

    struct Requirement {
        const ExtEnabled DeviceExtensions::*enabled;
        const char *name;
    };
    typedef std::vector<Requirement> RequirementVec;
    struct Info {
        Info(ExtEnabled DeviceExtensions::*state_, const RequirementVec requirements_)
            : state(state_), requirements(requirements_) {}
        ExtEnabled DeviceExtensions::*state;
        RequirementVec requirements;
    };

    const Info &GetInfo(vvl::Extension extension_name) const;

    DeviceExtensions() = default;
    DeviceExtensions(const InstanceExtensions &instance_ext) : InstanceExtensions(instance_ext) {}

    DeviceExtensions(const InstanceExtensions &instance_extensions, APIVersion requested_api_version,
                     const VkDeviceCreateInfo *pCreateInfo = nullptr);
    DeviceExtensions(const InstanceExtensions &instance_ext, APIVersion requested_api_version,
                     const std::vector<VkExtensionProperties> &props);
};

const InstanceExtensions::Info &GetInstanceVersionMap(const char *version);
const DeviceExtensions::Info &GetDeviceVersionMap(const char *version);

constexpr bool IsInstanceExtension(vvl::Extension extension) {
    switch (extension) {
        case vvl::Extension::_VK_KHR_surface:
        case vvl::Extension::_VK_KHR_display:
        case vvl::Extension::_VK_KHR_xlib_surface:
        case vvl::Extension::_VK_KHR_xcb_surface:
        case vvl::Extension::_VK_KHR_wayland_surface:
        case vvl::Extension::_VK_KHR_android_surface:
        case vvl::Extension::_VK_KHR_win32_surface:
        case vvl::Extension::_VK_KHR_get_physical_device_properties2:
        case vvl::Extension::_VK_KHR_device_group_creation:
        case vvl::Extension::_VK_KHR_external_memory_capabilities:
        case vvl::Extension::_VK_KHR_external_semaphore_capabilities:
        case vvl::Extension::_VK_KHR_external_fence_capabilities:
        case vvl::Extension::_VK_KHR_get_surface_capabilities2:
        case vvl::Extension::_VK_KHR_get_display_properties2:
        case vvl::Extension::_VK_KHR_surface_protected_capabilities:
        case vvl::Extension::_VK_KHR_portability_enumeration:
        case vvl::Extension::_VK_KHR_surface_maintenance1:
        case vvl::Extension::_VK_EXT_debug_report:
        case vvl::Extension::_VK_GGP_stream_descriptor_surface:
        case vvl::Extension::_VK_NV_external_memory_capabilities:
        case vvl::Extension::_VK_EXT_validation_flags:
        case vvl::Extension::_VK_NN_vi_surface:
        case vvl::Extension::_VK_EXT_direct_mode_display:
        case vvl::Extension::_VK_EXT_acquire_xlib_display:
        case vvl::Extension::_VK_EXT_display_surface_counter:
        case vvl::Extension::_VK_EXT_swapchain_colorspace:
        case vvl::Extension::_VK_MVK_ios_surface:
        case vvl::Extension::_VK_MVK_macos_surface:
        case vvl::Extension::_VK_EXT_debug_utils:
        case vvl::Extension::_VK_FUCHSIA_imagepipe_surface:
        case vvl::Extension::_VK_EXT_metal_surface:
        case vvl::Extension::_VK_EXT_validation_features:
        case vvl::Extension::_VK_EXT_headless_surface:
        case vvl::Extension::_VK_EXT_surface_maintenance1:
        case vvl::Extension::_VK_EXT_acquire_drm_display:
        case vvl::Extension::_VK_EXT_directfb_surface:
        case vvl::Extension::_VK_QNX_screen_surface:
        case vvl::Extension::_VK_GOOGLE_surfaceless_query:
        case vvl::Extension::_VK_LUNARG_direct_driver_loading:
        case vvl::Extension::_VK_EXT_layer_settings:
        case vvl::Extension::_VK_NV_display_stereo:
        case vvl::Extension::_VK_OHOS_surface:
            return true;
        default:
            return false;
    }
}

constexpr bool IsDeviceExtension(vvl::Extension extension) {
    switch (extension) {
        case vvl::Extension::_VK_KHR_swapchain:
        case vvl::Extension::_VK_KHR_display_swapchain:
        case vvl::Extension::_VK_KHR_sampler_mirror_clamp_to_edge:
        case vvl::Extension::_VK_KHR_video_queue:
        case vvl::Extension::_VK_KHR_video_decode_queue:
        case vvl::Extension::_VK_KHR_video_encode_h264:
        case vvl::Extension::_VK_KHR_video_encode_h265:
        case vvl::Extension::_VK_KHR_video_decode_h264:
        case vvl::Extension::_VK_KHR_dynamic_rendering:
        case vvl::Extension::_VK_KHR_multiview:
        case vvl::Extension::_VK_KHR_device_group:
        case vvl::Extension::_VK_KHR_shader_draw_parameters:
        case vvl::Extension::_VK_KHR_maintenance1:
        case vvl::Extension::_VK_KHR_external_memory:
        case vvl::Extension::_VK_KHR_external_memory_win32:
        case vvl::Extension::_VK_KHR_external_memory_fd:
        case vvl::Extension::_VK_KHR_win32_keyed_mutex:
        case vvl::Extension::_VK_KHR_external_semaphore:
        case vvl::Extension::_VK_KHR_external_semaphore_win32:
        case vvl::Extension::_VK_KHR_external_semaphore_fd:
        case vvl::Extension::_VK_KHR_push_descriptor:
        case vvl::Extension::_VK_KHR_shader_float16_int8:
        case vvl::Extension::_VK_KHR_16bit_storage:
        case vvl::Extension::_VK_KHR_incremental_present:
        case vvl::Extension::_VK_KHR_descriptor_update_template:
        case vvl::Extension::_VK_KHR_imageless_framebuffer:
        case vvl::Extension::_VK_KHR_create_renderpass2:
        case vvl::Extension::_VK_KHR_shared_presentable_image:
        case vvl::Extension::_VK_KHR_external_fence:
        case vvl::Extension::_VK_KHR_external_fence_win32:
        case vvl::Extension::_VK_KHR_external_fence_fd:
        case vvl::Extension::_VK_KHR_performance_query:
        case vvl::Extension::_VK_KHR_maintenance2:
        case vvl::Extension::_VK_KHR_variable_pointers:
        case vvl::Extension::_VK_KHR_dedicated_allocation:
        case vvl::Extension::_VK_KHR_storage_buffer_storage_class:
        case vvl::Extension::_VK_KHR_shader_bfloat16:
        case vvl::Extension::_VK_KHR_relaxed_block_layout:
        case vvl::Extension::_VK_KHR_get_memory_requirements2:
        case vvl::Extension::_VK_KHR_image_format_list:
        case vvl::Extension::_VK_KHR_sampler_ycbcr_conversion:
        case vvl::Extension::_VK_KHR_bind_memory2:
        case vvl::Extension::_VK_KHR_portability_subset:
        case vvl::Extension::_VK_KHR_maintenance3:
        case vvl::Extension::_VK_KHR_draw_indirect_count:
        case vvl::Extension::_VK_KHR_shader_subgroup_extended_types:
        case vvl::Extension::_VK_KHR_8bit_storage:
        case vvl::Extension::_VK_KHR_shader_atomic_int64:
        case vvl::Extension::_VK_KHR_shader_clock:
        case vvl::Extension::_VK_KHR_video_decode_h265:
        case vvl::Extension::_VK_KHR_global_priority:
        case vvl::Extension::_VK_KHR_driver_properties:
        case vvl::Extension::_VK_KHR_shader_float_controls:
        case vvl::Extension::_VK_KHR_depth_stencil_resolve:
        case vvl::Extension::_VK_KHR_swapchain_mutable_format:
        case vvl::Extension::_VK_KHR_timeline_semaphore:
        case vvl::Extension::_VK_KHR_vulkan_memory_model:
        case vvl::Extension::_VK_KHR_shader_terminate_invocation:
        case vvl::Extension::_VK_KHR_fragment_shading_rate:
        case vvl::Extension::_VK_KHR_dynamic_rendering_local_read:
        case vvl::Extension::_VK_KHR_shader_quad_control:
        case vvl::Extension::_VK_KHR_spirv_1_4:
        case vvl::Extension::_VK_KHR_separate_depth_stencil_layouts:
        case vvl::Extension::_VK_KHR_present_wait:
        case vvl::Extension::_VK_KHR_uniform_buffer_standard_layout:
        case vvl::Extension::_VK_KHR_buffer_device_address:
        case vvl::Extension::_VK_KHR_deferred_host_operations:
        case vvl::Extension::_VK_KHR_pipeline_executable_properties:
        case vvl::Extension::_VK_KHR_map_memory2:
        case vvl::Extension::_VK_KHR_shader_integer_dot_product:
        case vvl::Extension::_VK_KHR_pipeline_library:
        case vvl::Extension::_VK_KHR_shader_non_semantic_info:
        case vvl::Extension::_VK_KHR_present_id:
        case vvl::Extension::_VK_KHR_video_encode_queue:
        case vvl::Extension::_VK_KHR_synchronization2:
        case vvl::Extension::_VK_KHR_fragment_shader_barycentric:
        case vvl::Extension::_VK_KHR_shader_subgroup_uniform_control_flow:
        case vvl::Extension::_VK_KHR_zero_initialize_workgroup_memory:
        case vvl::Extension::_VK_KHR_workgroup_memory_explicit_layout:
        case vvl::Extension::_VK_KHR_copy_commands2:
        case vvl::Extension::_VK_KHR_format_feature_flags2:
        case vvl::Extension::_VK_KHR_ray_tracing_maintenance1:
        case vvl::Extension::_VK_KHR_shader_untyped_pointers:
        case vvl::Extension::_VK_KHR_maintenance4:
        case vvl::Extension::_VK_KHR_shader_subgroup_rotate:
        case vvl::Extension::_VK_KHR_shader_maximal_reconvergence:
        case vvl::Extension::_VK_KHR_maintenance5:
        case vvl::Extension::_VK_KHR_present_id2:
        case vvl::Extension::_VK_KHR_present_wait2:
        case vvl::Extension::_VK_KHR_ray_tracing_position_fetch:
        case vvl::Extension::_VK_KHR_pipeline_binary:
        case vvl::Extension::_VK_KHR_swapchain_maintenance1:
        case vvl::Extension::_VK_KHR_cooperative_matrix:
        case vvl::Extension::_VK_KHR_compute_shader_derivatives:
        case vvl::Extension::_VK_KHR_video_decode_av1:
        case vvl::Extension::_VK_KHR_video_encode_av1:
        case vvl::Extension::_VK_KHR_video_decode_vp9:
        case vvl::Extension::_VK_KHR_video_maintenance1:
        case vvl::Extension::_VK_KHR_vertex_attribute_divisor:
        case vvl::Extension::_VK_KHR_load_store_op_none:
        case vvl::Extension::_VK_KHR_unified_image_layouts:
        case vvl::Extension::_VK_KHR_shader_float_controls2:
        case vvl::Extension::_VK_KHR_index_type_uint8:
        case vvl::Extension::_VK_KHR_line_rasterization:
        case vvl::Extension::_VK_KHR_calibrated_timestamps:
        case vvl::Extension::_VK_KHR_shader_expect_assume:
        case vvl::Extension::_VK_KHR_maintenance6:
        case vvl::Extension::_VK_KHR_copy_memory_indirect:
        case vvl::Extension::_VK_KHR_video_encode_intra_refresh:
        case vvl::Extension::_VK_KHR_video_encode_quantization_map:
        case vvl::Extension::_VK_KHR_shader_relaxed_extended_instruction:
        case vvl::Extension::_VK_KHR_maintenance7:
        case vvl::Extension::_VK_KHR_maintenance8:
        case vvl::Extension::_VK_KHR_shader_fma:
        case vvl::Extension::_VK_KHR_maintenance9:
        case vvl::Extension::_VK_KHR_video_maintenance2:
        case vvl::Extension::_VK_KHR_depth_clamp_zero_one:
        case vvl::Extension::_VK_KHR_robustness2:
        case vvl::Extension::_VK_KHR_present_mode_fifo_latest_ready:
        case vvl::Extension::_VK_KHR_maintenance10:
        case vvl::Extension::_VK_NV_glsl_shader:
        case vvl::Extension::_VK_EXT_depth_range_unrestricted:
        case vvl::Extension::_VK_IMG_filter_cubic:
        case vvl::Extension::_VK_AMD_rasterization_order:
        case vvl::Extension::_VK_AMD_shader_trinary_minmax:
        case vvl::Extension::_VK_AMD_shader_explicit_vertex_parameter:
        case vvl::Extension::_VK_EXT_debug_marker:
        case vvl::Extension::_VK_AMD_gcn_shader:
        case vvl::Extension::_VK_NV_dedicated_allocation:
        case vvl::Extension::_VK_EXT_transform_feedback:
        case vvl::Extension::_VK_NVX_binary_import:
        case vvl::Extension::_VK_NVX_image_view_handle:
        case vvl::Extension::_VK_AMD_draw_indirect_count:
        case vvl::Extension::_VK_AMD_negative_viewport_height:
        case vvl::Extension::_VK_AMD_gpu_shader_half_float:
        case vvl::Extension::_VK_AMD_shader_ballot:
        case vvl::Extension::_VK_AMD_texture_gather_bias_lod:
        case vvl::Extension::_VK_AMD_shader_info:
        case vvl::Extension::_VK_AMD_shader_image_load_store_lod:
        case vvl::Extension::_VK_NV_corner_sampled_image:
        case vvl::Extension::_VK_IMG_format_pvrtc:
        case vvl::Extension::_VK_NV_external_memory:
        case vvl::Extension::_VK_NV_external_memory_win32:
        case vvl::Extension::_VK_NV_win32_keyed_mutex:
        case vvl::Extension::_VK_EXT_shader_subgroup_ballot:
        case vvl::Extension::_VK_EXT_shader_subgroup_vote:
        case vvl::Extension::_VK_EXT_texture_compression_astc_hdr:
        case vvl::Extension::_VK_EXT_astc_decode_mode:
        case vvl::Extension::_VK_EXT_pipeline_robustness:
        case vvl::Extension::_VK_EXT_conditional_rendering:
        case vvl::Extension::_VK_NV_clip_space_w_scaling:
        case vvl::Extension::_VK_EXT_display_control:
        case vvl::Extension::_VK_GOOGLE_display_timing:
        case vvl::Extension::_VK_NV_sample_mask_override_coverage:
        case vvl::Extension::_VK_NV_geometry_shader_passthrough:
        case vvl::Extension::_VK_NV_viewport_array2:
        case vvl::Extension::_VK_NVX_multiview_per_view_attributes:
        case vvl::Extension::_VK_NV_viewport_swizzle:
        case vvl::Extension::_VK_EXT_discard_rectangles:
        case vvl::Extension::_VK_EXT_conservative_rasterization:
        case vvl::Extension::_VK_EXT_depth_clip_enable:
        case vvl::Extension::_VK_EXT_hdr_metadata:
        case vvl::Extension::_VK_IMG_relaxed_line_rasterization:
        case vvl::Extension::_VK_EXT_external_memory_dma_buf:
        case vvl::Extension::_VK_EXT_queue_family_foreign:
        case vvl::Extension::_VK_ANDROID_external_memory_android_hardware_buffer:
        case vvl::Extension::_VK_EXT_sampler_filter_minmax:
        case vvl::Extension::_VK_AMD_gpu_shader_int16:
        case vvl::Extension::_VK_AMDX_shader_enqueue:
        case vvl::Extension::_VK_AMD_mixed_attachment_samples:
        case vvl::Extension::_VK_AMD_shader_fragment_mask:
        case vvl::Extension::_VK_EXT_inline_uniform_block:
        case vvl::Extension::_VK_EXT_shader_stencil_export:
        case vvl::Extension::_VK_EXT_sample_locations:
        case vvl::Extension::_VK_EXT_blend_operation_advanced:
        case vvl::Extension::_VK_NV_fragment_coverage_to_color:
        case vvl::Extension::_VK_NV_framebuffer_mixed_samples:
        case vvl::Extension::_VK_NV_fill_rectangle:
        case vvl::Extension::_VK_NV_shader_sm_builtins:
        case vvl::Extension::_VK_EXT_post_depth_coverage:
        case vvl::Extension::_VK_EXT_image_drm_format_modifier:
        case vvl::Extension::_VK_EXT_validation_cache:
        case vvl::Extension::_VK_EXT_descriptor_indexing:
        case vvl::Extension::_VK_EXT_shader_viewport_index_layer:
        case vvl::Extension::_VK_NV_shading_rate_image:
        case vvl::Extension::_VK_NV_ray_tracing:
        case vvl::Extension::_VK_NV_representative_fragment_test:
        case vvl::Extension::_VK_EXT_filter_cubic:
        case vvl::Extension::_VK_QCOM_render_pass_shader_resolve:
        case vvl::Extension::_VK_EXT_global_priority:
        case vvl::Extension::_VK_EXT_external_memory_host:
        case vvl::Extension::_VK_AMD_buffer_marker:
        case vvl::Extension::_VK_AMD_pipeline_compiler_control:
        case vvl::Extension::_VK_EXT_calibrated_timestamps:
        case vvl::Extension::_VK_AMD_shader_core_properties:
        case vvl::Extension::_VK_AMD_memory_overallocation_behavior:
        case vvl::Extension::_VK_EXT_vertex_attribute_divisor:
        case vvl::Extension::_VK_GGP_frame_token:
        case vvl::Extension::_VK_EXT_pipeline_creation_feedback:
        case vvl::Extension::_VK_NV_shader_subgroup_partitioned:
        case vvl::Extension::_VK_NV_compute_shader_derivatives:
        case vvl::Extension::_VK_NV_mesh_shader:
        case vvl::Extension::_VK_NV_fragment_shader_barycentric:
        case vvl::Extension::_VK_NV_shader_image_footprint:
        case vvl::Extension::_VK_NV_scissor_exclusive:
        case vvl::Extension::_VK_NV_device_diagnostic_checkpoints:
        case vvl::Extension::_VK_INTEL_shader_integer_functions2:
        case vvl::Extension::_VK_INTEL_performance_query:
        case vvl::Extension::_VK_EXT_pci_bus_info:
        case vvl::Extension::_VK_AMD_display_native_hdr:
        case vvl::Extension::_VK_EXT_fragment_density_map:
        case vvl::Extension::_VK_EXT_scalar_block_layout:
        case vvl::Extension::_VK_GOOGLE_hlsl_functionality1:
        case vvl::Extension::_VK_GOOGLE_decorate_string:
        case vvl::Extension::_VK_EXT_subgroup_size_control:
        case vvl::Extension::_VK_AMD_shader_core_properties2:
        case vvl::Extension::_VK_AMD_device_coherent_memory:
        case vvl::Extension::_VK_EXT_shader_image_atomic_int64:
        case vvl::Extension::_VK_EXT_memory_budget:
        case vvl::Extension::_VK_EXT_memory_priority:
        case vvl::Extension::_VK_NV_dedicated_allocation_image_aliasing:
        case vvl::Extension::_VK_EXT_buffer_device_address:
        case vvl::Extension::_VK_EXT_tooling_info:
        case vvl::Extension::_VK_EXT_separate_stencil_usage:
        case vvl::Extension::_VK_NV_cooperative_matrix:
        case vvl::Extension::_VK_NV_coverage_reduction_mode:
        case vvl::Extension::_VK_EXT_fragment_shader_interlock:
        case vvl::Extension::_VK_EXT_ycbcr_image_arrays:
        case vvl::Extension::_VK_EXT_provoking_vertex:
        case vvl::Extension::_VK_EXT_full_screen_exclusive:
        case vvl::Extension::_VK_EXT_line_rasterization:
        case vvl::Extension::_VK_EXT_shader_atomic_float:
        case vvl::Extension::_VK_EXT_host_query_reset:
        case vvl::Extension::_VK_EXT_index_type_uint8:
        case vvl::Extension::_VK_EXT_extended_dynamic_state:
        case vvl::Extension::_VK_EXT_host_image_copy:
        case vvl::Extension::_VK_EXT_map_memory_placed:
        case vvl::Extension::_VK_EXT_shader_atomic_float2:
        case vvl::Extension::_VK_EXT_swapchain_maintenance1:
        case vvl::Extension::_VK_EXT_shader_demote_to_helper_invocation:
        case vvl::Extension::_VK_NV_device_generated_commands:
        case vvl::Extension::_VK_NV_inherited_viewport_scissor:
        case vvl::Extension::_VK_EXT_texel_buffer_alignment:
        case vvl::Extension::_VK_QCOM_render_pass_transform:
        case vvl::Extension::_VK_EXT_depth_bias_control:
        case vvl::Extension::_VK_EXT_device_memory_report:
        case vvl::Extension::_VK_EXT_robustness2:
        case vvl::Extension::_VK_EXT_custom_border_color:
        case vvl::Extension::_VK_GOOGLE_user_type:
        case vvl::Extension::_VK_NV_present_barrier:
        case vvl::Extension::_VK_EXT_private_data:
        case vvl::Extension::_VK_EXT_pipeline_creation_cache_control:
        case vvl::Extension::_VK_NV_device_diagnostics_config:
        case vvl::Extension::_VK_QCOM_render_pass_store_ops:
        case vvl::Extension::_VK_NV_cuda_kernel_launch:
        case vvl::Extension::_VK_QCOM_tile_shading:
        case vvl::Extension::_VK_NV_low_latency:
        case vvl::Extension::_VK_EXT_metal_objects:
        case vvl::Extension::_VK_EXT_descriptor_buffer:
        case vvl::Extension::_VK_EXT_graphics_pipeline_library:
        case vvl::Extension::_VK_AMD_shader_early_and_late_fragment_tests:
        case vvl::Extension::_VK_NV_fragment_shading_rate_enums:
        case vvl::Extension::_VK_NV_ray_tracing_motion_blur:
        case vvl::Extension::_VK_EXT_ycbcr_2plane_444_formats:
        case vvl::Extension::_VK_EXT_fragment_density_map2:
        case vvl::Extension::_VK_QCOM_rotated_copy_commands:
        case vvl::Extension::_VK_EXT_image_robustness:
        case vvl::Extension::_VK_EXT_image_compression_control:
        case vvl::Extension::_VK_EXT_attachment_feedback_loop_layout:
        case vvl::Extension::_VK_EXT_4444_formats:
        case vvl::Extension::_VK_EXT_device_fault:
        case vvl::Extension::_VK_ARM_rasterization_order_attachment_access:
        case vvl::Extension::_VK_EXT_rgba10x6_formats:
        case vvl::Extension::_VK_NV_acquire_winrt_display:
        case vvl::Extension::_VK_VALVE_mutable_descriptor_type:
        case vvl::Extension::_VK_EXT_vertex_input_dynamic_state:
        case vvl::Extension::_VK_EXT_physical_device_drm:
        case vvl::Extension::_VK_EXT_device_address_binding_report:
        case vvl::Extension::_VK_EXT_depth_clip_control:
        case vvl::Extension::_VK_EXT_primitive_topology_list_restart:
        case vvl::Extension::_VK_EXT_present_mode_fifo_latest_ready:
        case vvl::Extension::_VK_FUCHSIA_external_memory:
        case vvl::Extension::_VK_FUCHSIA_external_semaphore:
        case vvl::Extension::_VK_FUCHSIA_buffer_collection:
        case vvl::Extension::_VK_HUAWEI_subpass_shading:
        case vvl::Extension::_VK_HUAWEI_invocation_mask:
        case vvl::Extension::_VK_NV_external_memory_rdma:
        case vvl::Extension::_VK_EXT_pipeline_properties:
        case vvl::Extension::_VK_EXT_frame_boundary:
        case vvl::Extension::_VK_EXT_multisampled_render_to_single_sampled:
        case vvl::Extension::_VK_EXT_extended_dynamic_state2:
        case vvl::Extension::_VK_EXT_color_write_enable:
        case vvl::Extension::_VK_EXT_primitives_generated_query:
        case vvl::Extension::_VK_EXT_global_priority_query:
        case vvl::Extension::_VK_VALVE_video_encode_rgb_conversion:
        case vvl::Extension::_VK_EXT_image_view_min_lod:
        case vvl::Extension::_VK_EXT_multi_draw:
        case vvl::Extension::_VK_EXT_image_2d_view_of_3d:
        case vvl::Extension::_VK_EXT_shader_tile_image:
        case vvl::Extension::_VK_EXT_opacity_micromap:
        case vvl::Extension::_VK_NV_displacement_micromap:
        case vvl::Extension::_VK_EXT_load_store_op_none:
        case vvl::Extension::_VK_HUAWEI_cluster_culling_shader:
        case vvl::Extension::_VK_EXT_border_color_swizzle:
        case vvl::Extension::_VK_EXT_pageable_device_local_memory:
        case vvl::Extension::_VK_ARM_shader_core_properties:
        case vvl::Extension::_VK_ARM_scheduling_controls:
        case vvl::Extension::_VK_EXT_image_sliced_view_of_3d:
        case vvl::Extension::_VK_VALVE_descriptor_set_host_mapping:
        case vvl::Extension::_VK_EXT_depth_clamp_zero_one:
        case vvl::Extension::_VK_EXT_non_seamless_cube_map:
        case vvl::Extension::_VK_ARM_render_pass_striped:
        case vvl::Extension::_VK_QCOM_fragment_density_map_offset:
        case vvl::Extension::_VK_NV_copy_memory_indirect:
        case vvl::Extension::_VK_NV_memory_decompression:
        case vvl::Extension::_VK_NV_device_generated_commands_compute:
        case vvl::Extension::_VK_NV_ray_tracing_linear_swept_spheres:
        case vvl::Extension::_VK_NV_linear_color_attachment:
        case vvl::Extension::_VK_EXT_image_compression_control_swapchain:
        case vvl::Extension::_VK_QCOM_image_processing:
        case vvl::Extension::_VK_EXT_nested_command_buffer:
        case vvl::Extension::_VK_OHOS_external_memory:
        case vvl::Extension::_VK_EXT_external_memory_acquire_unmodified:
        case vvl::Extension::_VK_EXT_extended_dynamic_state3:
        case vvl::Extension::_VK_EXT_subpass_merge_feedback:
        case vvl::Extension::_VK_ARM_tensors:
        case vvl::Extension::_VK_EXT_shader_module_identifier:
        case vvl::Extension::_VK_EXT_rasterization_order_attachment_access:
        case vvl::Extension::_VK_NV_optical_flow:
        case vvl::Extension::_VK_EXT_legacy_dithering:
        case vvl::Extension::_VK_EXT_pipeline_protected_access:
        case vvl::Extension::_VK_ANDROID_external_format_resolve:
        case vvl::Extension::_VK_AMD_anti_lag:
        case vvl::Extension::_VK_AMDX_dense_geometry_format:
        case vvl::Extension::_VK_EXT_shader_object:
        case vvl::Extension::_VK_QCOM_tile_properties:
        case vvl::Extension::_VK_SEC_amigo_profiling:
        case vvl::Extension::_VK_QCOM_multiview_per_view_viewports:
        case vvl::Extension::_VK_NV_ray_tracing_invocation_reorder:
        case vvl::Extension::_VK_NV_cooperative_vector:
        case vvl::Extension::_VK_NV_extended_sparse_address_space:
        case vvl::Extension::_VK_EXT_mutable_descriptor_type:
        case vvl::Extension::_VK_EXT_legacy_vertex_attributes:
        case vvl::Extension::_VK_ARM_shader_core_builtins:
        case vvl::Extension::_VK_EXT_pipeline_library_group_handles:
        case vvl::Extension::_VK_EXT_dynamic_rendering_unused_attachments:
        case vvl::Extension::_VK_NV_low_latency2:
        case vvl::Extension::_VK_ARM_data_graph:
        case vvl::Extension::_VK_QCOM_multiview_per_view_render_areas:
        case vvl::Extension::_VK_NV_per_stage_descriptor_set:
        case vvl::Extension::_VK_QCOM_image_processing2:
        case vvl::Extension::_VK_QCOM_filter_cubic_weights:
        case vvl::Extension::_VK_QCOM_ycbcr_degamma:
        case vvl::Extension::_VK_QCOM_filter_cubic_clamp:
        case vvl::Extension::_VK_EXT_attachment_feedback_loop_dynamic_state:
        case vvl::Extension::_VK_QNX_external_memory_screen_buffer:
        case vvl::Extension::_VK_MSFT_layered_driver:
        case vvl::Extension::_VK_NV_descriptor_pool_overallocation:
        case vvl::Extension::_VK_QCOM_tile_memory_heap:
        case vvl::Extension::_VK_EXT_memory_decompression:
        case vvl::Extension::_VK_NV_raw_access_chains:
        case vvl::Extension::_VK_NV_external_compute_queue:
        case vvl::Extension::_VK_NV_command_buffer_inheritance:
        case vvl::Extension::_VK_NV_shader_atomic_float16_vector:
        case vvl::Extension::_VK_EXT_shader_replicated_composites:
        case vvl::Extension::_VK_EXT_shader_float8:
        case vvl::Extension::_VK_NV_ray_tracing_validation:
        case vvl::Extension::_VK_NV_cluster_acceleration_structure:
        case vvl::Extension::_VK_NV_partitioned_acceleration_structure:
        case vvl::Extension::_VK_EXT_device_generated_commands:
        case vvl::Extension::_VK_MESA_image_alignment_control:
        case vvl::Extension::_VK_EXT_depth_clamp_control:
        case vvl::Extension::_VK_OHOS_native_buffer:
        case vvl::Extension::_VK_HUAWEI_hdr_vivid:
        case vvl::Extension::_VK_NV_cooperative_matrix2:
        case vvl::Extension::_VK_ARM_pipeline_opacity_micromap:
        case vvl::Extension::_VK_EXT_external_memory_metal:
        case vvl::Extension::_VK_ARM_performance_counters_by_region:
        case vvl::Extension::_VK_EXT_vertex_attribute_robustness:
        case vvl::Extension::_VK_ARM_format_pack:
        case vvl::Extension::_VK_VALVE_fragment_density_map_layered:
        case vvl::Extension::_VK_NV_present_metering:
        case vvl::Extension::_VK_EXT_fragment_density_map_offset:
        case vvl::Extension::_VK_EXT_zero_initialize_device_memory:
        case vvl::Extension::_VK_EXT_shader_64bit_indexing:
        case vvl::Extension::_VK_QCOM_data_graph_model:
        case vvl::Extension::_VK_SEC_pipeline_cache_incremental_mode:
        case vvl::Extension::_VK_EXT_shader_uniform_buffer_unsized_array:
        case vvl::Extension::_VK_KHR_acceleration_structure:
        case vvl::Extension::_VK_KHR_ray_tracing_pipeline:
        case vvl::Extension::_VK_KHR_ray_query:
        case vvl::Extension::_VK_EXT_mesh_shader:
            return true;
        default:
            return false;
    }
}

// NOLINTEND
