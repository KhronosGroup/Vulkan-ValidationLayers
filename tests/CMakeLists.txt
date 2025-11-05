# ~~~
# Copyright (c) 2014-2025 Valve Corporation
# Copyright (c) 2014-2025 LunarG, Inc.
# Copyright (C) 2025 Arm Limited.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ~~~

if(SLANG_INSTALL_DIR)
    find_package(slang CONFIG QUIET)
endif()

add_subdirectory(framework)

if (ANDROID)
    add_library(vk_layer_validation_tests MODULE)
else()
    add_executable(vk_layer_validation_tests)
endif()
target_sources(vk_layer_validation_tests PRIVATE
    unit/amd_best_practices.cpp
    unit/android_hardware_buffer.cpp
    unit/android_hardware_buffer_positive.cpp
    unit/android_external_resolve.cpp
    unit/android_external_resolve_positive.cpp
    unit/arm_best_practices.cpp
    unit/atomics.cpp
    unit/atomics_positive.cpp
    unit/best_practices.cpp
    unit/best_practices_positive.cpp
    unit/buffer.cpp
    unit/buffer_positive.cpp
    unit/command.cpp
    unit/command_positive.cpp
    unit/copy_buffer_image.cpp
    unit/copy_buffer_image_positive.cpp
    unit/data_graph.cpp
    unit/data_graph_positive.cpp
    unit/debug_extensions.cpp
    unit/debug_extensions_positive.cpp
    unit/debug_printf.cpp
    unit/debug_printf_shader_debug_info.cpp
    unit/debug_printf_ray_tracing.cpp
    unit/legacy.cpp
    unit/legacy_positive.cpp
    unit/descriptor_buffer.cpp
    unit/descriptor_buffer_positive.cpp
    unit/descriptor_indexing.cpp
    unit/descriptor_indexing_positive.cpp
    unit/descriptors.cpp
    unit/descriptors_positive.cpp
    unit/device_feature_property.cpp
    unit/device_feature_property_positive.cpp
    unit/device_generated_commands.cpp
    unit/device_generated_commands_positive.cpp
    unit/device_queue.cpp
    unit/device_queue_positive.cpp
    unit/dynamic_rendering.cpp
    unit/dynamic_rendering_positive.cpp
    unit/dynamic_rendering_local_read.cpp
    unit/dynamic_rendering_local_read_positive.cpp
    unit/dynamic_state.cpp
    unit/dynamic_state_positive.cpp
    unit/external_memory_metal.cpp
    unit/external_memory_sync.cpp
    unit/external_memory_sync_positive.cpp
    unit/fragment_shading_rate.cpp
    unit/fragment_shading_rate_positive.cpp
    unit/geometry_tessellation.cpp
    unit/geometry_tessellation_positive.cpp
    unit/gpu_av.cpp
    unit/gpu_av_buffer_device_address.cpp
    unit/gpu_av_buffer_device_address_positive.cpp
    unit/gpu_av_cooperative_vector.cpp
    unit/gpu_av_cooperative_vector_positive.cpp
    unit/gpu_av_copies.cpp
    unit/gpu_av_copies_positive.cpp
    unit/gpu_av_debug_printf.cpp
    unit/gpu_av_descriptor_buffer_positive.cpp
    unit/gpu_av_descriptor_class_general_buffer.cpp
    unit/gpu_av_descriptor_class_general_buffer_positive.cpp
    unit/gpu_av_descriptor_class_texel_buffer.cpp
    unit/gpu_av_descriptor_class_texel_buffer_positive.cpp
    unit/gpu_av_descriptor_indexing.cpp
    unit/gpu_av_descriptor_indexing_positive.cpp
    unit/gpu_av_descriptor_post_process.cpp
    unit/gpu_av_descriptor_post_process_positive.cpp
    unit/gpu_av_indirect_buffer.cpp
    unit/gpu_av_indirect_buffer_positive.cpp
    unit/gpu_av_index_buffer.cpp
    unit/gpu_av_index_buffer_positive.cpp
    unit/gpu_av_positive.cpp
    unit/gpu_av_shader_object_positive.cpp
    unit/gpu_av_shader_debug_info.cpp
    unit/gpu_av_spirv.cpp
    unit/gpu_av_spirv_positive.cpp
    unit/gpu_av_ray_query.cpp
    unit/gpu_av_ray_query_positive.cpp
    unit/gpu_av_ray_tracing.cpp
    unit/gpu_av_ray_tracing_positive.cpp
    unit/gpu_av_vertex_attribute_fetch.cpp
    unit/gpu_av_vertex_attribute_fetch_positive.cpp
    unit/graphics_library.cpp
    unit/graphics_library_positive.cpp
    unit/host_image_copy.cpp
    unit/host_image_copy_positive.cpp
    unit/image_drm.cpp
    unit/image_drm_positive.cpp
    unit/image_layout.cpp
    unit/image_layout_positive.cpp
    unit/image.cpp
    unit/image_positive.cpp
    unit/imageless_framebuffer.cpp
    unit/imageless_framebuffer_positive.cpp
    unit/instance_positive.cpp
    unit/instanceless.cpp
    unit/layer_utils_positive.cpp
    unit/layer_settings.cpp
    unit/layer_settings_positive.cpp
    unit/memory.cpp
    unit/memory_positive.cpp
    unit/mesh.cpp
    unit/mesh_positive.cpp
    unit/multiview.cpp
    unit/multiview_positive.cpp
    unit/nvidia_best_practices.cpp
    unit/object_lifetime.cpp
    unit/object_lifetime_positive.cpp
    unit/other_positive.cpp
    unit/others.cpp
    unit/parent.cpp
    unit/parent_positive.cpp
    unit/pipeline.cpp
    unit/pipeline_advanced_blend.cpp
    unit/pipeline_binary.cpp
    unit/pipeline_binary_positive.cpp
    unit/pipeline_layout.cpp
    unit/pipeline_layout_positive.cpp
    unit/pipeline_positive.cpp
    unit/pipeline_topology.cpp
    unit/pipeline_topology_positive.cpp
    unit/portability_subset.cpp
    unit/protected_memory.cpp
    unit/protected_memory_positive.cpp
    unit/query.cpp
    unit/push_descriptor.cpp
    unit/push_descriptor_positive.cpp
    unit/query_positive.cpp
    unit/ray_tracing.cpp
    unit/ray_tracing_nv.cpp
    unit/ray_tracing_pipeline.cpp
    unit/ray_tracing_pipeline_positive.cpp
    unit/ray_tracing_pipeline_nv.cpp
    unit/ray_tracing_pipeline_positive_nv.cpp
    unit/ray_tracing_positive.cpp
    unit/render_pass_positive.cpp
    unit/render_pass.cpp
    unit/robustness.cpp
    unit/robustness_positive.cpp
    unit/sampler.cpp
    unit/sampler_positive.cpp
    unit/secondary_command_buffer.cpp
    unit/secondary_command_buffer_positive.cpp
    unit/shader_64bit_indexing.cpp
    unit/shader_64bit_indexing_positive.cpp
    unit/shader_compute.cpp
    unit/shader_compute_positive.cpp
    unit/shader_cooperative_matrix.cpp
    unit/shader_cooperative_matrix_positive.cpp
    unit/shader_cooperative_vector.cpp
    unit/shader_cooperative_vector_positive.cpp
    unit/shader_debug_info.cpp
    unit/shader_image_access.cpp
    unit/shader_image_access_positive.cpp
    unit/shader_interface.cpp
    unit/shader_interface_positive.cpp
    unit/shader_limits.cpp
    unit/shader_limits_positive.cpp
    unit/shader_mesh.cpp
    unit/shader_mesh_positive.cpp
    unit/shader_object.cpp
    unit/shader_object_positive.cpp
    unit/shader_push_constants.cpp
    unit/shader_push_constants_positive.cpp
    unit/shader_spirv.cpp
    unit/shader_spirv_positive.cpp
    unit/shader_storage_image.cpp
    unit/shader_storage_image_positive.cpp
    unit/shader_storage_texel.cpp
    unit/shader_storage_texel_positive.cpp
    unit/shader_untyped.cpp
    unit/shader_untyped_positive.cpp
    unit/sparse_buffer.cpp
    unit/sparse_buffer_positive.cpp
    unit/sparse_image.cpp
    unit/sparse_image_positive.cpp
    unit/subgroups.cpp
    unit/subpass.cpp
    unit/subpass_positive.cpp
    unit/sync_object.cpp
    unit/sync_object_positive.cpp
    unit/sync_val.cpp
    unit/sync_val_positive.cpp
    unit/sync_val_reporting.cpp
    unit/sync_val_ray_tracing.cpp
    unit/sync_val_ray_tracing_positive.cpp
    unit/sync_val_semaphore.cpp
    unit/sync_val_semaphore_positive.cpp
    unit/sync_val_video.cpp
    unit/sync_val_video_positive.cpp
    unit/sync_val_wsi.cpp
    unit/sync_val_wsi_positive.cpp
    unit/tensor.cpp
    unit/tensor_positive.cpp
    unit/threading.cpp
    unit/threading_positive.cpp
    unit/tooling.cpp
    unit/tooling_positive.cpp
    unit/transform_feedback.cpp
    unit/vertex_input.cpp
    unit/vertex_input_positive.cpp
    unit/video.cpp
    unit/video_positive.cpp
    unit/video_decode.cpp
    unit/video_decode_positive.cpp
    unit/video_decode_av1.cpp
    unit/video_decode_av1_positive.cpp
    unit/video_decode_h264.cpp
    unit/video_decode_h264_positive.cpp
    unit/video_decode_h265.cpp
    unit/video_decode_h265_positive.cpp
    unit/video_decode_vp9.cpp
    unit/video_decode_vp9_positive.cpp
    unit/video_encode.cpp
    unit/video_encode_positive.cpp
    unit/video_encode_av1.cpp
    unit/video_encode_av1_positive.cpp
    unit/video_encode_h264.cpp
    unit/video_encode_h264_positive.cpp
    unit/video_encode_h265.cpp
    unit/video_encode_h265_positive.cpp
    unit/video_encode_intra_refresh.cpp
    unit/video_encode_intra_refresh_positive.cpp
    unit/video_encode_quantization_map.cpp
    unit/video_encode_quantization_map_positive.cpp
    unit/viewport_inheritance.cpp
    unit/wsi.cpp
    unit/wsi_positive.cpp
    unit/ycbcr.cpp
    unit/ycbcr_positive.cpp
    vvl_utils/small_vector.cpp
    vvl_utils/pnext_chain_extraction.cpp
)

get_target_property(TEST_SOURCES vk_layer_validation_tests SOURCES)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${TEST_SOURCES})

add_dependencies(vk_layer_validation_tests vvl)

target_link_libraries(vk_layer_validation_tests PRIVATE
    vk_test_framework
)

# More details in tests/android/mock/README.md
option(VVL_MOCK_ANDROID "Enable building for Android on desktop for testing with MockICD setup")
if(VVL_MOCK_ANDROID)
    # We don't want to build the android APK, so we just set the minimum Android settings
    target_sources(VkLayer_utils PRIVATE android/mock/android/hardware_buffer.cpp)
    target_compile_definitions(VkLayer_utils PUBLIC VK_USE_PLATFORM_ANDROID_KHR VVL_MOCK_ANDROID __ANDROID__)
    target_include_directories(VkLayer_utils SYSTEM PUBLIC android/mock)
elseif (ANDROID)
    add_subdirectory(android)
    return()
endif()

if(SLANG_INSTALL_DIR)
    configure_slang_for_target(vk_layer_validation_tests)
endif()

install(TARGETS vk_layer_validation_tests)
if (WIN32)
    install(FILES $<TARGET_RUNTIME_DLLS:vk_layer_validation_tests> DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()
if(SLANG_INSTALL_DIR)
    install_slang_with_target(vk_layer_validation_tests ${CMAKE_INSTALL_LIBDIR})
endif()

include(GoogleTest)
# If cross-compiling, discovery will use an executable with the wrong architecture and stop execution here.
# Set env. variable VVL_SKIP_GTEST_DISCOVERY to skip discovery.
if(NOT VVL_SKIP_GTEST_DISCOVERY)
    gtest_discover_tests(vk_layer_validation_tests DISCOVERY_TIMEOUT 100)
endif()

add_subdirectory(stress)
add_subdirectory(undefined)
add_subdirectory(layers)
add_subdirectory(icd)
