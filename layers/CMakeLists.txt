# ~~~
# Copyright (c) 2014-2025 Valve Corporation
# Copyright (c) 2014-2025 LunarG, Inc.
# Copyright (c) 2023-2025 RasterGrid Kft.
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

# This variable enables downstream users to customize the CMake targets
# based on the target API variant (e.g. Vulkan SC)
set(LAYER_NAME "VkLayer_khronos_validation")

add_subdirectory(external)

add_library(VkLayer_utils STATIC)
target_sources(VkLayer_utils PRIVATE
    containers/container_utils.h
    containers/custom_containers.h
    containers/limits.h
    containers/small_container.h
    containers/small_range_map.h
    containers/small_vector.h
    containers/span.h
    containers/tls_guard.h
    error_message/logging.h
    error_message/logging.cpp
    error_message/error_location.cpp
    error_message/error_location.h
    error_message/error_strings.h
    error_message/record_object.h
    error_message/log_message_type.h
    ${API_TYPE}/generated/error_location_helper.cpp
    ${API_TYPE}/generated/error_location_helper.h
    ${API_TYPE}/generated/feature_requirements_helper.cpp
    ${API_TYPE}/generated/feature_requirements_helper.h
    ${API_TYPE}/generated/pnext_chain_extraction.cpp
    ${API_TYPE}/generated/pnext_chain_extraction.h
    ${API_TYPE}/generated/vk_function_pointers.cpp
    ${API_TYPE}/generated/vk_function_pointers.h
    ${API_TYPE}/generated/vk_validation_error_messages.h
    ${API_TYPE}/generated/vk_layer_dispatch_table.h
    ${API_TYPE}/generated/vk_dispatch_table_helper.h
    ${API_TYPE}/generated/vk_dispatch_table_helper.cpp
    ${API_TYPE}/generated/vk_object_types.h
    ${API_TYPE}/generated/vk_object_types.cpp
    ${API_TYPE}/generated/vk_api_version.h
    ${API_TYPE}/generated/vk_extension_helper.h
    ${API_TYPE}/generated/vk_extension_helper.cpp
    utils/action_command_utils.h
    utils/assert_utils.h
    utils/cast_utils.h
    utils/convert_utils.cpp
    utils/convert_utils.h
    utils/dispatch_utils.cpp
    utils/dispatch_utils.h
    utils/file_system_utils.cpp
    utils/file_system_utils.h
    utils/hash_util.h
    utils/hash_util.cpp
    utils/hash_vk_types.h
    utils/image_utils.h
    utils/image_utils.cpp
    utils/image_layout_utils.h
    utils/image_layout_utils.cpp
    utils/lock_utils.h
    utils/math_utils.h
    utils/vk_layer_extension_utils.cpp
    utils/vk_layer_extension_utils.h
    utils/keyboard.cpp
    utils/keyboard.h
    utils/ray_tracing_utils.cpp
    utils/ray_tracing_utils.h
    utils/sync_utils.cpp
    utils/sync_utils.h
    utils/text_utils.cpp
    utils/text_utils.h
    utils/vk_struct_compare.cpp
    utils/vk_struct_compare.h
    utils/vk_api_utils.h
    vk_layer_config.h
    vk_layer_config.cpp
)

target_link_libraries(VkLayer_utils PUBLIC
    Vulkan::Headers
    Vulkan::LayerSettings
    Vulkan::SafeStruct
    Vulkan::UtilityHeaders
    ${CMAKE_DL_LIBS}
    vvl_external
)
target_include_directories(VkLayer_utils PUBLIC . ${API_TYPE})

# These are warnings we want to ignore everywhere
if(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
    # Not needed for GCC
    target_compile_options(VkLayer_utils PUBLIC
        -Wno-sign-conversion
        -Wno-implicit-int-conversion
    )
elseif(MSVC)
    target_compile_options(VkLayer_utils PUBLIC
        /wd4324 # padding
        /wd4458 # hiding class member
        /wd4457 # hiding function parameter
        /wd4702 # unreachable code
        /wd4389 # signed/unsigned mismatch
    )
endif()
# Every generated function will hit this
target_compile_options(VkLayer_utils PUBLIC "$<IF:$<CXX_COMPILER_ID:MSVC>,/wd4100,-Wno-unused-parameter>")

option(USE_CUSTOM_HASH_MAP "Use a custom hashing library that is faster than std::unordered_map and std::unordered_set (default: ON)" ON)

if(DEFINED USE_ROBIN_HOOD_HASHING)
    message(DEPRECATION "USE_ROBIN_HOOD_HASHING has been deprecated. Please use USE_CUSTOM_HASH_MAP option instead.")
    set(USE_CUSTOM_HASH_MAP ${USE_ROBIN_HOOD_HASHING})
endif()

if (USE_CUSTOM_HASH_MAP)
    message(STATUS "Using parallel_hashmap as custom hash maps")
    target_compile_definitions(VkLayer_utils PUBLIC USE_CUSTOM_HASH_MAP)
    # Prefer using a locally installed parallel-hashmap package before using the in-tree sources.
    # This approach supports installation with package managers such as conan
    find_package(phmap CONFIG)
    if (TARGET phmap)
        target_link_libraries(VkLayer_utils PRIVATE phmap)
    else()
        add_subdirectory(external/parallel_hashmap)
    endif()
else()
    message(STATUS "Using STL maps instead of custom hash maps")
endif()

# Using mimalloc on non-Windows OSes currently results in unit test instability with some
# OS version / driver combinations. On 32-bit systems, using mimalloc cause an increase in
# the amount of virtual address space needed, which can also cause stability problems.
if (WIN32 AND CMAKE_SIZEOF_VOID_P EQUAL 8)
   find_package(mimalloc CONFIG)
   option(USE_MIMALLOC "Use mimalloc, a fast malloc/free replacement library" ${mimalloc_FOUND})
   if (USE_MIMALLOC)
      target_compile_definitions(VkLayer_utils PUBLIC USE_MIMALLOC)
      target_link_libraries(VkLayer_utils PUBLIC mimalloc-static)
   endif()
endif()

option(DEBUG_CAPTURE_KEYBOARD "Every QueueSubmit if F1 is pressed, a DebugCapture function will be called and a custom action can be done (default: OFF)" OFF)
if (DEBUG_CAPTURE_KEYBOARD)
    target_compile_definitions(VkLayer_utils PUBLIC DEBUG_CAPTURE_KEYBOARD)
    if(WIN32)
    else()
        # needed for XQueryKeymap
        find_package(X11 REQUIRED)
        target_link_libraries(VkLayer_utils PRIVATE ${X11_LIBRARIES})
        target_include_directories(VkLayer_utils PRIVATE ${X11_INCLUDE_DIR})

        # needed for xcb_key_symbols_alloc
        find_library(XCB_LIB xcb REQUIRED)
        find_path(XCB_INCLUDE_DIR xcb/xcb.h REQUIRED)
        find_library(XCB_KEYSYMS_LIB xcb-keysyms REQUIRED)
        find_path(XCB_KEYSYMS_INCLUDE_DIR xcb/xcb_keysyms.h REQUIRED)
        target_link_libraries(VkLayer_utils PRIVATE ${XCB_LIB} ${XCB_KEYSYMS_LIB})
        target_include_directories(VkLayer_utils PRIVATE ${XCB_INCLUDE_DIR} ${XCB_KEYSYMS_INCLUDE_DIR})
    endif()
endif()

add_subdirectory(gpuav)
add_subdirectory(best_practices)
add_subdirectory(sync)
add_subdirectory(thread_tracker)
add_subdirectory(stateless)
add_subdirectory(legacy)

if(IOS)
    add_library(vvl SHARED)
else()
    add_library(vvl MODULE)
endif()

target_sources(vvl PRIVATE
    chassis/chassis_modification_state.h
    chassis/chassis_manual.cpp
    chassis/dispatch_object_manual.cpp
    containers/range.h
    containers/range_map.h
    core_checks/cc_android.cpp
    core_checks/cc_buffer.cpp
    core_checks/cc_buffer_address.h
    core_checks/cc_cmd_buffer_dynamic.cpp
    core_checks/cc_cmd_buffer.cpp
    core_checks/cc_copy_blit_resolve.cpp
    core_checks/core_validation.h
    core_checks/cc_data_graph.cpp
    core_checks/cc_descriptor.cpp
    core_checks/cc_device.cpp
    core_checks/cc_device_memory.cpp
    core_checks/cc_device_generated_commands.cpp
    core_checks/cc_drawdispatch.cpp
    core_checks/cc_external_object.cpp
    core_checks/cc_image.cpp
    core_checks/cc_image_layout.cpp
    core_checks/cc_pipeline_compute.cpp
    core_checks/cc_pipeline_graphics.cpp
    core_checks/cc_pipeline_ray_tracing.cpp
    core_checks/cc_pipeline.cpp
    core_checks/cc_query.cpp
    core_checks/cc_queue.cpp
    core_checks/cc_ray_tracing.cpp
    core_checks/cc_render_pass.cpp
    core_checks/cc_spirv.cpp
    core_checks/cc_shader_interface.cpp
    core_checks/cc_shader_object.cpp
    core_checks/cc_state_tracker.h
    core_checks/cc_state_tracker.cpp
    core_checks/cc_submit.h
    core_checks/cc_submit.cpp
    core_checks/cc_sync_vuid_maps.h
    core_checks/cc_sync_vuid_maps.cpp
    core_checks/cc_synchronization.h
    core_checks/cc_synchronization.cpp
    core_checks/cc_tensor.cpp
    core_checks/cc_video.cpp
    core_checks/cc_vuid_maps.cpp
    core_checks/cc_vuid_maps.h
    core_checks/cc_wsi.cpp
    core_checks/cc_ycbcr.cpp
    drawdispatch/descriptor_validator.cpp
    drawdispatch/descriptor_validator.h
    drawdispatch/drawdispatch_vuids.cpp
    drawdispatch/drawdispatch_vuids.h
    error_message/spirv_logging.h
    error_message/spirv_logging.cpp
    ${API_TYPE}/generated/chassis.cpp
    ${API_TYPE}/generated/valid_enum_values.cpp
    ${API_TYPE}/generated/valid_enum_values.h
    ${API_TYPE}/generated/valid_flag_values.cpp
    ${API_TYPE}/generated/command_validation.cpp
    ${API_TYPE}/generated/command_validation.h
    ${API_TYPE}/generated/device_features.cpp
    ${API_TYPE}/generated/device_features.h
    ${API_TYPE}/generated/dynamic_state_helper.cpp
    ${API_TYPE}/generated/dynamic_state_helper.h
    ${API_TYPE}/generated/enum_flag_bits.h
    ${API_TYPE}/generated/feature_not_present.cpp
    ${API_TYPE}/generated/dispatch_vector.cpp
    ${API_TYPE}/generated/dispatch_object.cpp
    ${API_TYPE}/generated/validation_object_device_methods.h
    ${API_TYPE}/generated/validation_object_instance_methods.h
    ${API_TYPE}/generated/validation_object.cpp
    ${API_TYPE}/generated/object_tracker.cpp
    ${API_TYPE}/generated/object_tracker_device_methods.h
    ${API_TYPE}/generated/object_tracker_instance_methods.h
    ${API_TYPE}/generated/spirv_grammar_helper.cpp
    ${API_TYPE}/generated/spirv_validation_helper.cpp
    ${API_TYPE}/generated/spirv_validation_helper.h
    ${API_TYPE}/generated/sync_validation_types.cpp
    object_tracker/object_lifetime_validation.cpp
    object_tracker/object_lifetime_validation.h
    state_tracker/bind_point.h
    state_tracker/buffer_state.cpp
    state_tracker/buffer_state.h
    state_tracker/cmd_buffer_state.cpp
    state_tracker/cmd_buffer_state.h
    state_tracker/data_graph_pipeline_session_state.cpp
    state_tracker/data_graph_pipeline_session_state.h
    state_tracker/descriptor_mode.h
    state_tracker/descriptor_set_layouts.cpp
    state_tracker/descriptor_set_layouts.h
    state_tracker/descriptor_sets.cpp
    state_tracker/descriptor_sets.h
    state_tracker/device_generated_commands_state.cpp
    state_tracker/device_generated_commands_state.h
    state_tracker/device_memory_state.cpp
    state_tracker/device_memory_state.h
    state_tracker/device_state.cpp
    state_tracker/device_state.h
    state_tracker/event_map.h
    state_tracker/fence_state.cpp
    state_tracker/fence_state.h
    state_tracker/image_layout_map.cpp
    state_tracker/image_layout_map.h
    state_tracker/image_state.cpp
    state_tracker/image_state.h
    state_tracker/last_bound_state.cpp
    state_tracker/last_bound_state.h
    state_tracker/pipeline_layout_state.cpp
    state_tracker/pipeline_layout_state.h
    state_tracker/pipeline_state.cpp
    state_tracker/pipeline_state.h
    state_tracker/pipeline_library_state.cpp
    state_tracker/pipeline_library_state.h
    state_tracker/push_constant_data.h
    state_tracker/semaphore_state.cpp
    state_tracker/semaphore_state.h
    state_tracker/special_supported.h
    state_tracker/state_object.cpp
    state_tracker/state_object.h
    state_tracker/subresource_adapter.cpp
    state_tracker/subresource_adapter.h
    state_tracker/query_state.cpp
    state_tracker/query_state.h
    state_tracker/tensor_state.cpp
    state_tracker/tensor_state.h
    state_tracker/queue_state.cpp
    state_tracker/queue_state.h
    state_tracker/ray_tracing_state.h
    state_tracker/render_pass_state.cpp
    state_tracker/render_pass_state.h
    state_tracker/sampler_state.cpp
    state_tracker/sampler_state.h
    state_tracker/shader_instruction.cpp
    state_tracker/shader_instruction.h
    state_tracker/shader_module.cpp
    state_tracker/shader_module.h
    state_tracker/shader_object_state.cpp
    state_tracker/shader_object_state.h
    state_tracker/shader_stage_state.cpp
    state_tracker/shader_stage_state.h
    state_tracker/state_tracker.cpp
    state_tracker/state_tracker.h
    state_tracker/submission_reference.h
    state_tracker/vertex_index_buffer_state.h
    state_tracker/video_session_state.cpp
    state_tracker/video_session_state.h
    state_tracker/wsi_state.cpp
    state_tracker/wsi_state.h
    utils/shader_utils.cpp
    utils/shader_utils.h
    layer_options.cpp
    layer_options.h
    layer_options_validation.h
    profiling/profiling.h
)
get_target_property(LAYER_SOURCES vvl SOURCES)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${LAYER_SOURCES})

option(BUILD_SELF_VVL "Setup a VVL build that will be used for self validation" FALSE)
if (BUILD_SELF_VVL)
    target_compile_definitions(VkLayer_utils PUBLIC BUILD_SELF_VVL)
    target_compile_definitions(vvl PUBLIC BUILD_SELF_VVL)
endif()

set_target_properties(vvl PROPERTIES OUTPUT_NAME ${LAYER_NAME})

if(MSVC)
    target_link_options(vvl PRIVATE /DEF:${CMAKE_CURRENT_SOURCE_DIR}/${LAYER_NAME}.def)
    target_compile_options(vvl PRIVATE /bigobj)

    set(NATVIS_FILES "${CMAKE_CURRENT_SOURCE_DIR}/types.natvis" "${CMAKE_CURRENT_SOURCE_DIR}/external/parallel_hashmap/natvis/phmap.natvis")
    target_sources(vvl PRIVATE ${NATVIS_FILES})
elseif(MINGW)
    target_sources(vvl PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/${LAYER_NAME}.def)
    target_compile_options(vvl PRIVATE -Wa,-mbig-obj)
    set_target_properties(vvl PROPERTIES PREFIX "") # remove the prefix "lib" so the manifest json "library_path" matches
elseif(APPLE)

# IOS and APPLE can both be true (FYI)
if(IOS)
    set_target_properties(vvl PROPERTIES
        FRAMEWORK TRUE
        MACOSX_FRAMEWORK_BUNDLE_VERSION "${VulkanHeaders_VERSION}"
        MACOSX_FRAMEWORK_SHORT_VERSION_STRING "${VulkanHeaders_VERSION}"
        MACOSX_FRAMEWORK_IDENTIFIER com.khronos.validation
    )
else()
    set_target_properties(vvl PROPERTIES SUFFIX ".dylib")
endif()
    # Both Apple and IOS
    target_link_options(vvl PRIVATE -exported_symbols_list ${CMAKE_CURRENT_SOURCE_DIR}/${LAYER_NAME}.exp)
elseif(ANDROID)
    # Need 16k pages in Android 15
    # When we bump our NDK requirement to r28 we can remove
    # https://developer.android.com/guide/practices/page-sizes#compile-r28
    # https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/9703
    # https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/9170
    target_link_options(vvl PRIVATE "-Wl,-z,max-page-size=16384")

    target_link_options(vvl PRIVATE LINKER:--version-script=${CMAKE_CURRENT_SOURCE_DIR}/${LAYER_NAME}-android.map)
else()
    target_link_options(vvl PRIVATE LINKER:--version-script=${CMAKE_CURRENT_SOURCE_DIR}/${LAYER_NAME}.map)
endif()

# Order matters here. VkLayer_utils should be the last link library to ensure mimalloc overrides are picked up correctly.
# Otherwise, libraries after VkLayer_utils will not benefit from this performance improvement.
target_link_libraries(vvl PRIVATE
    SPIRV-Headers::SPIRV-Headers
    SPIRV-Tools-opt
    vvl_external
    vvl_best_practices
    vvl_sync
    vvl_stateless
    vvl_thread_tracker
    vvl_legacy
    vvl_gpu_av
    gpu_av_spirv
    VkLayer_utils
)

if (VVL_ENABLE_TRACY)
    # Rely on FetchContent to get Tracy, it is simple
    # update_deps.py is too chaotic. Quite ok since Tracy is intended as a dev tool
    include(FetchContent)
    FetchContent_Declare(
        tracy
        GIT_REPOSITORY https://github.com/arno-lunarg/tracy.git
        GIT_TAG v0.12.1-arno
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE)

    FetchContent_MakeAvailable(tracy)

    target_sources(vvl PRIVATE
        profiling/profiling.h
        profiling/profiling.cpp
    )
    target_link_libraries(vvl PRIVATE TracyClient)
endif()

# TODO - Figure out why IOS builds have duplicated spirv-tools libs
if (IOS)
    target_link_options(vvl PRIVATE "LINKER:-no_warn_duplicate_libraries")
endif()

if (ANDROID)
    # Required for __android_log_print. Marking as PUBLIC since the tests use __android_log_print as well.
    target_link_libraries(VkLayer_utils PUBLIC log)

    # Required for AHardwareBuffer_describe. Marking as PUBLIC since the tests use AHardwareBuffer_describe as well.
    target_link_libraries(VkLayer_utils PUBLIC android)

    install(TARGETS vvl DESTINATION ${CMAKE_INSTALL_LIBDIR})

    return()
endif()

# There are 2 primary deliverables for the validation layers.
# - The actual library VkLayer_khronos_validation.(dll|so|dylib)
# - The respective json file, VkLayer_khronos_validation.json
# This code generates the appropriate json for both local testing and the installation.
# NOTE: For WIN32 the JSON and dll MUST be placed in the same location, due to Win32 using a relative path for installation.
set(INPUT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/${LAYER_NAME}.json.in")
set(INTERMEDIATE_FILE "${CMAKE_CURRENT_BINARY_DIR}/json/validation.json")
if (BUILD_SELF_VVL)
    set(OUTPUT_FILE_FINAL_NAME "VkLayer_dev_self_validation.json")
    set(JSON_LAYER_NAME "VK_LAYER_DEV_self_validation")
else()
    set(OUTPUT_FILE_FINAL_NAME "${LAYER_NAME}.json")
    set(JSON_LAYER_NAME "VK_LAYER_KHRONOS_validation")
endif()
set(LAYER_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR})
if (WIN32)
    set(LAYER_INSTALL_DIR ${CMAKE_INSTALL_BINDIR}) # WIN32/MINGW expect the dll in the `bin` dir, this matches our WIN32 SDK process
endif()

if (WIN32)
    set(JSON_LIBRARY_PATH ".\\\\${LAYER_NAME}.dll")
elseif(APPLE)
    set(JSON_LIBRARY_PATH "./lib${LAYER_NAME}.dylib")
else()
    set(JSON_LIBRARY_PATH "./lib${LAYER_NAME}.so")
endif()

configure_file(${INPUT_FILE} ${INTERMEDIATE_FILE} @ONLY)

# To support both multi/single configuration generators just copy the json to the correct directory
add_custom_command(TARGET vvl POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${INTERMEDIATE_FILE} $<TARGET_FILE_DIR:vvl>/${OUTPUT_FILE_FINAL_NAME}
)

# Auto generate vk_layer_settings.txt upon setting changes
find_package(Python3 REQUIRED COMPONENTS Interpreter)
add_custom_command(
    OUTPUT "${VVL_SOURCE_DIR}/layers/vk_layer_settings.txt" "${VVL_SOURCE_DIR}/layers/layer_options_validation.h"
    COMMAND Python3::Interpreter "scripts/generate_settings.py"
    DEPENDS "${VVL_SOURCE_DIR}/layers/VkLayer_khronos_validation.json.in"
    WORKING_DIRECTORY ${VVL_SOURCE_DIR}
)
add_custom_target(vvl_generated_setting_files
    DEPENDS "${VVL_SOURCE_DIR}/layers/vk_layer_settings.txt" "${VVL_SOURCE_DIR}/layers/layer_options_validation.h"
)
add_dependencies(vvl vvl_generated_setting_files)

# For UNIX-based systems, `library_path` should not contain a relative path (indicated by "./") before installing to system directories
# This json isn't used for regular local development, it's used for installation
if (UNIX)
    set(UNIX_INTERMEDIATE_FILE "${CMAKE_CURRENT_BINARY_DIR}/json/unix_install_validation.json")

    if(APPLE)
        set(JSON_LIBRARY_PATH "lib${LAYER_NAME}.dylib")
    else()
        set(JSON_LIBRARY_PATH "lib${LAYER_NAME}.so")
    endif()

    configure_file(${INPUT_FILE} ${UNIX_INTERMEDIATE_FILE} @ONLY)

    install(FILES ${UNIX_INTERMEDIATE_FILE} DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/vulkan/explicit_layer.d RENAME ${OUTPUT_FILE_FINAL_NAME})
endif()

if (WIN32)
    install(FILES ${INTERMEDIATE_FILE} DESTINATION ${LAYER_INSTALL_DIR} RENAME ${OUTPUT_FILE_FINAL_NAME})
endif()
if (MSVC)
    install(FILES $<TARGET_PDB_FILE:vvl> DESTINATION ${LAYER_INSTALL_DIR} OPTIONAL)
endif()

install(TARGETS vvl DESTINATION ${LAYER_INSTALL_DIR})
