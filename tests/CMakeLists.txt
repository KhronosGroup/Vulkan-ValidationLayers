# ~~~
# Copyright (c) 2014-2025 Valve Corporation
# Copyright (c) 2014-2025 LunarG, Inc.
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
    framework/shader_helper.h
    framework/shader_helper.cpp
    framework/shader_object_helper.h
    framework/shader_object_helper.cpp
    framework/test_common.h
    framework/shader_templates.h
    framework/error_monitor.cpp
    framework/error_monitor.h
    framework/video_objects.h
    framework/render.cpp
    framework/render.h
    framework/binding.h
    framework/binding.cpp
    framework/buffer_helper.h
    framework/test_framework.cpp
    framework/ray_tracing_objects.h
    framework/ray_tracing_objects.cpp
    framework/ray_tracing_helper_nv.h
    framework/ray_tracing_helper_nv.cpp
    framework/external_memory_sync.h
    framework/external_memory_sync.cpp
    framework/sync_helper.h
    framework/sync_helper.cpp
    framework/descriptor_helper.h
    framework/descriptor_helper.cpp
    framework/thread_helper.h
    framework/thread_helper.cpp
    framework/gpu_av_helper.h
    framework/render_pass_helper.h
    framework/render_pass_helper.cpp
    framework/feature_requirements.h
    framework/feature_requirements.cpp
    framework/queue_submit_context.h
    framework/queue_submit_context.cpp
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
    unit/cooperative_vector.cpp
    unit/cooperative_vector_positive.cpp
    unit/copy_buffer_image.cpp
    unit/copy_buffer_image_positive.cpp
    unit/debug_extensions.cpp
    unit/debug_extensions_positive.cpp
    unit/debug_printf.cpp
    unit/debug_printf_shader_debug_info.cpp
    unit/debug_printf_ray_tracing.cpp
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
    unit/gpu_av_image_layout.cpp
    unit/gpu_av_image_layout_positive.cpp
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
    unit/shader_compute.cpp
    unit/shader_compute_positive.cpp
    unit/shader_cooperative_matrix.cpp
    unit/shader_cooperative_matrix_positive.cpp
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
    unit/video_encode.cpp
    unit/video_encode_positive.cpp
    unit/video_encode_av1.cpp
    unit/video_encode_av1_positive.cpp
    unit/video_encode_h264.cpp
    unit/video_encode_h264_positive.cpp
    unit/video_encode_h265.cpp
    unit/video_encode_h265_positive.cpp
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
if (APPLE)
    target_sources(vk_layer_validation_tests PRIVATE
        framework/apple_wsi.h
        framework/apple_wsi.mm
    )
    # QuartzCore framework is needed for minimal Metal interaction
    target_link_libraries(vk_layer_validation_tests PRIVATE "-framework QuartzCore")
endif()

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

    if (CMAKE_SIZEOF_VOID_P EQUAL 4)
        # Due to IHV driver issues, we need the extra 2GB of virtual address space for 32 bit testing
        target_link_options(vk_layer_validation_tests PRIVATE /LARGEADDRESSAWARE)
    endif()
endif()

find_package(GTest CONFIG)
find_package(glslang CONFIG)
find_package(SPIRV-Tools CONFIG)

include(ExternalProject)

set(SLANG_BINARY_DIR "${CMAKE_BINARY_DIR}/slang")
set(SLANG_DOWNLOAD_DIR "${SLANG_BINARY_DIR}/download")
set(SLANG_EXTRACT_DIR "${SLANG_BINARY_DIR}/extract")

# Function to determine the correct binary URL based on platform
function(get_slang_binary_url RESULT_VAR VERSION_VAR)
    set(SLANG_VERSION "2025.8.1" CACHE STRING "Slang version to use")
    set(${VERSION_VAR} ${SLANG_VERSION} PARENT_SCOPE)

    if(WIN32)
        set(${RESULT_VAR} "https://github.com/shader-slang/slang/releases/download/v${SLANG_VERSION}/slang-${SLANG_VERSION}-windows-x86_64.zip" PARENT_SCOPE)
    elseif(UNIX AND NOT APPLE)
        set(${RESULT_VAR} "https://github.com/shader-slang/slang/releases/download/v${SLANG_VERSION}/slang-${SLANG_VERSION}-linux-x86_64.zip" PARENT_SCOPE)
    elseif(APPLE)
        set(${RESULT_VAR} "https://github.com/shader-slang/slang/releases/download/v${SLANG_VERSION}/slang-${SLANG_VERSION}-macos-aarch64.zip" PARENT_SCOPE)
    else()
        message(FATAL_ERROR "Unsupported platform for Slang prebuilt binaries")
    endif()
endfunction()

# Get the URL and version
get_slang_binary_url(SLANG_URL SLANG_VERSION)
message(STATUS "Using Slang version: ${SLANG_VERSION}")
message(STATUS "Downloading from: ${SLANG_URL}")

# Download the ZIP file directly using file(DOWNLOAD)
file(DOWNLOAD "${SLANG_URL}" "${SLANG_DOWNLOAD_DIR}/slang-${SLANG_VERSION}.zip"
     SHOW_PROGRESS
     STATUS DOWNLOAD_STATUS
)
list(GET DOWNLOAD_STATUS 0 STATUS_CODE)
if(NOT STATUS_CODE EQUAL 0)
    list(GET DOWNLOAD_STATUS 1 ERROR_MESSAGE)
    message(FATAL_ERROR "Failed to download Slang: ${ERROR_MESSAGE}")
endif()

# Extract the ZIP file using CMake's capabilities
file(MAKE_DIRECTORY "${SLANG_EXTRACT_DIR}")
file(ARCHIVE_EXTRACT 
     INPUT "${SLANG_DOWNLOAD_DIR}/slang-${SLANG_VERSION}.zip"
     DESTINATION "${SLANG_EXTRACT_DIR}"
)

# Print the directory structure for debugging
message(STATUS "Listing contents of extracted directory:")
file(GLOB_RECURSE EXTRACTED_FILES "${SLANG_EXTRACT_DIR}/*")
foreach(FILE ${EXTRACTED_FILES})
    message(STATUS "  ${FILE}")
endforeach()

# Define paths based on actual structure - these might need adjustment
if(WIN32)
    # Most Windows releases put files in bin/ directory
    set(SLANG_BIN_DIR "${SLANG_EXTRACT_DIR}/bin")
    set(SLANG_LIB_DIR "${SLANG_EXTRACT_DIR}/bin")
    set(SLANG_INCLUDE_DIR "${SLANG_EXTRACT_DIR}/include")

    # Double-check if lib directory exists separately
    if(EXISTS "${SLANG_EXTRACT_DIR}/lib")
        set(SLANG_LIB_DIR "${SLANG_EXTRACT_DIR}/lib")
    endif()

    # Check for slang.lib in both potential locations
    if(EXISTS "${SLANG_BIN_DIR}/slang.lib")
        set(SLANG_LIBRARY "${SLANG_BIN_DIR}/slang.lib")
    elseif(EXISTS "${SLANG_LIB_DIR}/slang.lib")
        set(SLANG_LIBRARY "${SLANG_LIB_DIR}/slang.lib")
    else()
        # Try to find the file with glob
        file(GLOB SLANG_LIB_FILES "${SLANG_EXTRACT_DIR}/**/slang.lib")
        if(SLANG_LIB_FILES)
            list(GET SLANG_LIB_FILES 0 SLANG_LIBRARY)
            message(STATUS "Found slang.lib at: ${SLANG_LIBRARY}")
        else()
            message(FATAL_ERROR "Could not locate slang.lib in extracted directory")
        endif()
    endif()
else()
    # Unix systems typically have lib/ and include/ directories
    set(SLANG_INCLUDE_DIR "${SLANG_EXTRACT_DIR}/include")
    set(SLANG_LIB_DIR "${SLANG_EXTRACT_DIR}/lib")

    if(EXISTS "${SLANG_LIB_DIR}/libslang.so")
        set(SLANG_LIBRARY "${SLANG_LIB_DIR}/libslang.so")
    else()
        file(GLOB SLANG_LIB_FILES "${SLANG_EXTRACT_DIR}/**/libslang.so")
        if(SLANG_LIB_FILES)
            list(GET SLANG_LIB_FILES 0 SLANG_LIBRARY)
            message(STATUS "Found libslang.so at: ${SLANG_LIBRARY}")
        else()
            message(FATAL_ERROR "Could not locate libslang.so in extracted directory")
        endif()
    endif()
endif()

# Create an imported target for easier usage
add_library(slang SHARED IMPORTED GLOBAL)

# Set properties
file(MAKE_DIRECTORY ${SLANG_INCLUDE_DIR})
message(STATUS "Setting Slang include dir: ${SLANG_INCLUDE_DIR}")
message(STATUS "Setting Slang library: ${SLANG_LIBRARY}")

set_target_properties(slang PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${SLANG_INCLUDE_DIR}
)

if(WIN32)
    message(STATUS "Setting Slang .lib: ${SLANG_LIBRARY}")
    set_target_properties(slang PROPERTIES
        IMPORTED_IMPLIB ${SLANG_LIBRARY}
    )
else()
    set_target_properties(slang PROPERTIES
        IMPORTED_LOCATION ${SLANG_LIBRARY}
    )
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${SLANG_LIBRARY}
            $<TARGET_FILE_DIR:${TARGET_NAME}>
        COMMENT "Copying Slang .so to output directory for ${TARGET_NAME}"
    )
endif()

# Function to copy the DLL to the target's output directory (for Windows)
function(copy_slang_dll TARGET_NAME)
    if(WIN32)
        add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${SLANG_DLL}
                $<TARGET_FILE_DIR:${TARGET_NAME}>
            COMMENT "Copying Slang DLL to output directory for ${TARGET_NAME}"
        )
    endif()
endfunction()

target_link_libraries(vk_layer_validation_tests PRIVATE
    VkLayer_utils
    slang
    glslang::SPIRV
    glslang::SPVRemapper
    SPIRV-Tools-static
    SPIRV-Headers::SPIRV-Headers
    GTest::gtest
    GTest::gtest_main
    $<TARGET_NAME_IF_EXISTS:PkgConfig::XCB>
    $<TARGET_NAME_IF_EXISTS:PkgConfig::X11>
    $<TARGET_NAME_IF_EXISTS:PkgConfig::WAYlAND_CLIENT>
)

# setup framework/config.h using framework/config.h.in as a source
file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/config_$<CONFIG>.h" INPUT "${CMAKE_CURRENT_SOURCE_DIR}/framework/config.h.in")

# Since config_$<CONFIG>.h differs per build, set a compiler definition that files can use to include it
target_compile_definitions(vk_layer_validation_tests PRIVATE CONFIG_HEADER_FILE="config_$<CONFIG>.h")

target_sources(vk_layer_validation_tests PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/config_$<CONFIG>.h)
target_include_directories(vk_layer_validation_tests PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${VVL_SOURCE_DIR}/layers/external
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

install(TARGETS vk_layer_validation_tests)

include(GoogleTest)
gtest_discover_tests(vk_layer_validation_tests DISCOVERY_TIMEOUT 100)

add_subdirectory(layers)
add_subdirectory(icd)
