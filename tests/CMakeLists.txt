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
    framework/sync_val_tests.h
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
    unit/copy_buffer_image.cpp
    unit/copy_buffer_image_positive.cpp
    unit/debug_extensions.cpp
    unit/debug_extensions_positive.cpp
    unit/debug_printf.cpp
    unit/debug_printf_shader_debug_info.cpp
    unit/debug_printf_ray_tracing.cpp
    unit/deprecation.cpp
    unit/deprecation_positive.cpp
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
    unit/ray_tracing_spheres.cpp
    unit/ray_tracing_spheres_positive.cpp
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

# Slang
# ---
if(DEFINED SLANG_INSTALL_DIR)
    # Define paths based on actual structure
    if(WIN32)
        if(EXISTS "${SLANG_INSTALL_DIR}/bin")
            set(SLANG_BIN_DIR "${SLANG_INSTALL_DIR}/bin")
        endif()

        if(EXISTS "${SLANG_INSTALL_DIR}/lib")
            set(SLANG_LIB_DIR "${SLANG_INSTALL_DIR}/lib")
        else()
            set(SLANG_LIB_DIR "${SLANG_INSTALL_DIR}/bin")
        endif()

        set(SLANG_INCLUDE_DIR "${SLANG_INSTALL_DIR}/include")

        # Find slang.lib
        if(EXISTS "${SLANG_LIB_DIR}/slang.lib")
            set(SLANG_LIBRARY "${SLANG_LIB_DIR}/slang.lib")
        else()
            file(GLOB SLANG_LIB_FILES "${SLANG_INSTALL_DIR}/**/slang.lib")
            if(SLANG_LIB_FILES)
                list(GET SLANG_LIB_FILES 0 SLANG_LIBRARY)
            else()
                message(FATAL_ERROR "Could not locate slang.lib in extracted directory")
            endif()
        endif()

        # Find slang.dll
        file(GLOB SLANG_DLL_FILES "${SLANG_INSTALL_DIR}/**/slang.dll")
        if(SLANG_DLL_FILES)
            list(GET SLANG_DLL_FILES 0 SLANG_DLL)
        elseif(EXISTS "${SLANG_BIN_DIR}/slang.dll")
            set(SLANG_DLL "${SLANG_BIN_DIR}/slang.dll")
        else()
            message(FATAL_ERROR "Could not locate slang.dll in extracted directory")
        endif()

        # On Windows, also check for any other DLLs in the same directory
        # as slang.dll and add them to a list for copying
        get_filename_component(SLANG_DLL_DIR "${SLANG_DLL}" DIRECTORY)
        file(GLOB ADDITIONAL_DLLS "${SLANG_DLL_DIR}/*.dll")
        foreach(DLL ${ADDITIONAL_DLLS})
            if(NOT DLL STREQUAL SLANG_DLL)
                list(APPEND SLANG_DEPENDENCY_DLLS ${DLL})
            endif()
        endforeach()

    else()
        set(SLANG_INCLUDE_DIR "${SLANG_INSTALL_DIR}/include")

        if(EXISTS "${SLANG_INSTALL_DIR}/lib")
            set(SLANG_LIB_DIR "${SLANG_INSTALL_DIR}/lib")
        else()
            # Some releases might put libraries in a different location
            file(GLOB LIB_DIRS "${SLANG_INSTALL_DIR}/**/lib")
            if(LIB_DIRS)
                list(GET LIB_DIRS 0 SLANG_LIB_DIR)
            else()
                set(SLANG_LIB_DIR "${SLANG_INSTALL_DIR}")
            endif()
        endif()

        # Find libslang.so
        if(EXISTS "${SLANG_LIB_DIR}/libslang.so")
            set(SLANG_LIBRARY "${SLANG_LIB_DIR}/libslang.so")
            message(STATUS "Found libslang.so at: ${SLANG_LIBRARY}")
        else()
            file(GLOB SLANG_LIB_FILES "${SLANG_INSTALL_DIR}/**/libslang.so")
            if(SLANG_LIB_FILES)
                list(GET SLANG_LIB_FILES 0 SLANG_LIBRARY)
                message(STATUS "Found libslang.so via glob at: ${SLANG_LIBRARY}")
            else()
                message(FATAL_ERROR "Could not locate libslang.so in extracted directory")
            endif()
        endif()

        # Check if we have other shared libraries that might be dependencies
        get_filename_component(SLANG_LIB_DIR_REAL "${SLANG_LIBRARY}" DIRECTORY)
        file(GLOB ADDITIONAL_LIBS "${SLANG_LIB_DIR_REAL}/*.so*")
        foreach(LIB ${ADDITIONAL_LIBS})
            if(NOT LIB STREQUAL SLANG_LIBRARY)
                list(APPEND SLANG_DEPENDENCY_LIBS ${LIB})
            endif()
        endforeach()
    endif()

    # Create an imported target for easier usage
    add_library(slang SHARED IMPORTED GLOBAL)

    set_target_properties(slang PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES ${SLANG_INCLUDE_DIR}
    )

    if(WIN32)
        set_target_properties(slang PROPERTIES
            IMPORTED_IMPLIB ${SLANG_LIBRARY}
            IMPORTED_LOCATION ${SLANG_DLL}
        )
    else()
        set_target_properties(slang PROPERTIES
            IMPORTED_LOCATION ${SLANG_LIBRARY}
        )
    endif()

    # For specific target directories (adds to the common destinations above)
    function(configure_slang_for_target TARGET_NAME)
        if(WIN32)
            add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${SLANG_DLL}
                    $<TARGET_FILE_DIR:${TARGET_NAME}>
                COMMENT "Copying Slang DLL to output directory for ${TARGET_NAME}"
            )

            get_target_property(TARGET_OUTPUT_DIR ${TARGET_NAME} RUNTIME_OUTPUT_DIRECTORY)
            if(NOT TARGET_OUTPUT_DIR)
                set(TARGET_OUTPUT_DIR "$<TARGET_FILE_DIR:${TARGET_NAME}>")
            endif()

            foreach(DEP_DLL ${SLANG_DEPENDENCY_DLLS})
                get_filename_component(DEP_DLL_NAME ${DEP_DLL} NAME)
                add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        ${DEP_DLL}
                        $<TARGET_FILE_DIR:${TARGET_NAME}>
                    COMMENT "Copying dependency DLL ${DEP_DLL_NAME} to output directory for ${TARGET_NAME}"
                )
            endforeach()
        else()
            get_filename_component(SLANG_SONAME ${SLANG_LIBRARY} NAME)
            add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${SLANG_LIBRARY}
                    $<TARGET_FILE_DIR:${TARGET_NAME}>/${SLANG_SONAME}
                COMMENT "Copying Slang shared library to output directory for ${TARGET_NAME}"
            )

            get_target_property(TARGET_OUTPUT_DIR ${TARGET_NAME} RUNTIME_OUTPUT_DIRECTORY)
            if(NOT TARGET_OUTPUT_DIR)
                set(TARGET_OUTPUT_DIR "$<TARGET_FILE_DIR:${TARGET_NAME}>")
            endif()

            # Copy any dependency libraries
            foreach(DEP_LIB ${SLANG_DEPENDENCY_LIBS})
                get_filename_component(DEP_LIB_NAME ${DEP_LIB} NAME)
                add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        ${DEP_LIB}
                        $<TARGET_FILE_DIR:${TARGET_NAME}>/${DEP_LIB_NAME}
                    COMMENT "Copying dependency library ${DEP_LIB_NAME} to output directory for ${TARGET_NAME}"
                )
            endforeach()

            # Set RPATH to look in the same directory as the executable
            set_target_properties(${TARGET_NAME} PROPERTIES
                INSTALL_RPATH "$ORIGIN"
                BUILD_WITH_INSTALL_RPATH TRUE
            )

            # Add post-build step to verify the library was copied and is executable
            add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E echo "Verifying shared libraries in $<TARGET_FILE_DIR:${TARGET_NAME}>"
                COMMAND ${CMAKE_COMMAND} -E echo "Slang library path: $<TARGET_FILE_DIR:${TARGET_NAME}>/${SLANG_SONAME}"
                COMMAND test -f "$<TARGET_FILE_DIR:${TARGET_NAME}>/${SLANG_SONAME}" || echo "Library not copied correctly"
                COMMAND chmod +x "$<TARGET_FILE_DIR:${TARGET_NAME}>/${SLANG_SONAME}"
                COMMAND echo "Library permissions:" && ls -la "$<TARGET_FILE_DIR:${TARGET_NAME}>/${SLANG_SONAME}"
            )
        endif()
    endfunction()

    function(install_slang_with_target TARGET_NAME DESTINATION)
        if(NOT WIN32)
            get_filename_component(SLANG_SONAME ${SLANG_LIBRARY} NAME)
            install(FILES ${SLANG_LIBRARY} DESTINATION ${DESTINATION})

            foreach(DEP_LIB ${SLANG_DEPENDENCY_LIBS})
                get_filename_component(DEP_LIB_NAME ${DEP_LIB} NAME)
                install(FILES ${DEP_LIB} DESTINATION ${DESTINATION})
            endforeach()

            set_target_properties(${TARGET_NAME} PROPERTIES INSTALL_RPATH "$ORIGIN")

            # Add install-time command to set executable permissions
            install(CODE "message(STATUS \"Setting executable permissions on ${CMAKE_INSTALL_PREFIX}/${DESTINATION}/${SLANG_SONAME}\")
                         execute_process(COMMAND chmod +x \"${CMAKE_INSTALL_PREFIX}/${DESTINATION}/${SLANG_SONAME}\")")
         endif()
    endfunction()

    set(USE_SLANG TRUE)
    configure_slang_for_target(vk_layer_validation_tests)
else()
    message(STATUS "Skipping Slang setup for 32-bit architecture")
    set(USE_SLANG FALSE)
endif()

target_link_libraries(vk_layer_validation_tests PRIVATE
    VkLayer_utils
    $<$<BOOL:${USE_SLANG}>:slang>
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
if(USE_SLANG)
    target_compile_definitions(vk_layer_validation_tests PRIVATE VVL_USE_SLANG)
    install_slang_with_target(vk_layer_validation_tests lib)
endif()

include(GoogleTest)
# If cross-compiling, discovery will use an executable with the wrong architecture and stop execution here.
# Set env. variable VVL_SKIP_GTEST_DISCOVERY to skip discovery.
if(NOT VVL_SKIP_GTEST_DISCOVERY)
    gtest_discover_tests(vk_layer_validation_tests DISCOVERY_TIMEOUT 100)
endif()

add_subdirectory(layers)
add_subdirectory(icd)
