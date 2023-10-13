# ~~~
# Copyright (c) 2014-2023 Valve Corporation
# Copyright (c) 2014-2023 LunarG, Inc.
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
if (ANDROID)
    add_library(vk_layer_validation_tests MODULE)
else()
    add_executable(vk_layer_validation_tests)
endif()
target_sources(vk_layer_validation_tests PRIVATE
    framework/android_hardware_buffer.h
    framework/layer_validation_tests.h
    framework/layer_validation_tests.cpp
    framework/pipeline_helper.h
    framework/pipeline_helper.cpp
    framework/test_common.h
    framework/shader_templates.h
    framework/error_monitor.cpp
    framework/error_monitor.h
    framework/video_objects.h
    framework/render.cpp
    framework/render.h
    framework/binding.h
    framework/binding.cpp
    framework/test_framework.cpp
    framework/ray_tracing_objects.h
    framework/ray_tracing_objects.cpp
    framework/ray_tracing_nv.h
    framework/ray_tracing_nv.cpp
    framework/external_memory_sync.h
    framework/external_memory_sync.cpp
    framework/barrier_queue_family.h
    framework/barrier_queue_family.cpp
    framework/descriptor_helper.h
    framework/descriptor_helper.cpp
    framework/thread_timeout_helper.h
    framework/thread_timeout_helper.cpp
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
    unit/debug_printf.cpp
    unit/descriptor_buffer.cpp
    unit/descriptor_buffer_positive.cpp
    unit/descriptor_indexing.cpp
    unit/descriptor_indexing_positive.cpp
    unit/descriptors.cpp
    unit/descriptors_positive.cpp
    unit/device_queue.cpp
    unit/dynamic_rendering.cpp
    unit/dynamic_rendering_positive.cpp
    unit/dynamic_state.cpp
    unit/dynamic_state_positive.cpp
    unit/external_memory_sync.cpp
    unit/external_memory_sync_positive.cpp
    unit/fragment_shading_rate.cpp
    unit/fragment_shading_rate_positive.cpp
    unit/geometry_tessellation.cpp
    unit/geometry_tessellation_positive.cpp
    unit/gpu_av.cpp
    unit/gpu_av_positive.cpp
    unit/graphics_library.cpp
    unit/graphics_library_positive.cpp
    unit/host_image_copy.cpp
    unit/host_image_copy_positive.cpp
    unit/image_drm.cpp
    unit/image_drm_positive.cpp
    unit/image.cpp
    unit/image_positive.cpp
    unit/imageless_framebuffer.cpp
    unit/imageless_framebuffer_positive.cpp
    unit/instance_positive.cpp
    unit/instanceless.cpp
    unit/layer_utils_positive.cpp
    unit/memory.cpp
    unit/memory_positive.cpp
    unit/mesh.cpp
    unit/mesh_positive.cpp
    unit/multiview.cpp
    unit/nvidia_best_practices.cpp
    unit/object_lifetime.cpp
    unit/other_positive.cpp
    unit/others.cpp
    unit/parent.cpp
    unit/pipeline.cpp
    unit/pipeline_advanced_blend.cpp
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
    unit/ray_tracing_gpu.cpp
    unit/ray_tracing_pipeline.cpp
    unit/ray_tracing_pipeline_positive.cpp
    unit/ray_tracing_positive.cpp
    unit/render_pass_positive.cpp
    unit/renderpass.cpp
    unit/robustness.cpp
    unit/robustness_positive.cpp
    unit/sampler.cpp
    unit/sampler_positive.cpp
    unit/shader_compute.cpp
    unit/shader_compute_positive.cpp
    unit/shader_cooperative_matrix.cpp
    unit/shader_cooperative_matrix_positive.cpp
    unit/shader_image_access_positive.cpp
    unit/shader_interface.cpp
    unit/shader_interface_positive.cpp
    unit/shader_limits.cpp
    unit/shader_limits_positive.cpp
    unit/shader_mesh.cpp
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
    unit/threading.cpp
    unit/threading_positive.cpp
    unit/tooling_positive.cpp
    unit/transform_feedback.cpp
    unit/vertex_input.cpp
    unit/vertex_input_positive.cpp
    unit/video.cpp
    unit/video_positive.cpp
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

target_compile_options(vk_layer_validation_tests PRIVATE "$<IF:$<CXX_COMPILER_ID:MSVC>,/wd4100,-Wno-unused-parameter>")

if(${CMAKE_CXX_COMPILER_ID} MATCHES "(GNU|Clang)")
    target_compile_options(vk_layer_validation_tests PRIVATE
        -Wno-sign-compare
        -Wno-shorten-64-to-32
        -Wno-missing-field-initializers
    )
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(vk_layer_validation_tests PRIVATE
            -Wno-sign-conversion
            -Wno-implicit-int-conversion
        )
    endif()
elseif(MSVC)
    target_compile_options(vk_layer_validation_tests PRIVATE
        /wd4389 # signed/unsigned mismatch
        /wd4267 # Disable some signed/unsigned mismatch warnings.
    )

    set_target_properties(vk_layer_validation_tests PROPERTIES VS_DEBUGGER_ENVIRONMENT "VK_LAYER_PATH=$<TARGET_FILE_DIR:vvl>")

    if (CMAKE_SIZEOF_VOID_P EQUAL 4)
        # Due to IHV driver issues, we need the extra 2GB of virtual address space for 32 bit testing
        target_link_options(vk_layer_validation_tests PRIVATE /LARGEADDRESSAWARE)
    endif()
endif()

find_package(GTest CONFIG)
find_package(glslang CONFIG)

target_link_libraries(vk_layer_validation_tests PRIVATE
    VkLayer_utils
    glslang::glslang
    glslang::OGLCompiler
    glslang::OSDependent
    glslang::MachineIndependent
    glslang::GenericCodeGen
    glslang::HLSL
    glslang::SPIRV
    glslang::SPVRemapper
    SPIRV-Headers::SPIRV-Headers
    GTest::gtest
    GTest::gtest_main
    $<TARGET_NAME_IF_EXISTS:PkgConfig::XCB>
    $<TARGET_NAME_IF_EXISTS:PkgConfig::X11>
    $<TARGET_NAME_IF_EXISTS:PkgConfig::WAYlAND_CLIENT>
)

find_package(SPIRV-Tools CONFIG QUIET)

# See https://github.com/KhronosGroup/SPIRV-Tools/issues/3909 for background on this.
# The targets available from SPIRV-Tools change depending on how SPIRV_TOOLS_BUILD_STATIC is set.
# Try to handle all possible combinations so that we work with externally built packages.
if (TARGET SPIRV-Tools)
    target_link_libraries(vk_layer_validation_tests PRIVATE SPIRV-Tools)
elseif(TARGET SPIRV-Tools-static)
    target_link_libraries(vk_layer_validation_tests PRIVATE SPIRV-Tools-static)
elseif(TARGET SPIRV-Tools-shared)
    target_link_libraries(vk_layer_validation_tests PRIVATE SPIRV-Tools-shared)
else()
    message(FATAL_ERROR "Cannot determine SPIRV-Tools target name")
endif()

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

install(TARGETS vk_layer_validation_tests)

include(GoogleTest)
gtest_discover_tests(vk_layer_validation_tests DISCOVERY_TIMEOUT 100)

add_subdirectory(layers)
