# ~~~
# Copyright (c) 2014-2023 Valve Corporation
# Copyright (c) 2014-2023 LunarG, Inc.
# Copyright (c) 2023-2023 RasterGrid Kft.
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

add_library(VkLayer_utils STATIC)
target_sources(VkLayer_utils PRIVATE
    containers/custom_containers.h
    error_message/logging.h
    error_message/logging.cpp
    error_message/error_location.cpp
    error_message/error_location.h
    error_message/record_object.h
    external/xxhash.h
    external/xxhash.cpp
    ${API_TYPE}/generated/error_location_helper.cpp
    ${API_TYPE}/generated/error_location_helper.h
    ${API_TYPE}/generated/pnext_chain_extraction.cpp
    ${API_TYPE}/generated/pnext_chain_extraction.h
    ${API_TYPE}/generated/vk_function_pointers.cpp
    ${API_TYPE}/generated/vk_function_pointers.h
    ${API_TYPE}/generated/vk_validation_error_messages.h
    ${API_TYPE}/generated/vk_layer_dispatch_table.h
    ${API_TYPE}/generated/vk_dispatch_table_helper.h
    ${API_TYPE}/generated/vk_safe_struct.h
    ${API_TYPE}/generated/vk_safe_struct_utils.cpp
    ${API_TYPE}/generated/vk_safe_struct_core.cpp
    ${API_TYPE}/generated/vk_safe_struct_khr.cpp
    ${API_TYPE}/generated/vk_safe_struct_ext.cpp
    ${API_TYPE}/generated/vk_safe_struct_vendor.cpp
    ${API_TYPE}/generated/vk_object_types.h
    ${API_TYPE}/generated/vk_api_version.h
    ${API_TYPE}/generated/vk_extension_helper.h
    utils/cast_utils.h
    utils/convert_utils.cpp
    utils/convert_utils.h
    utils/hash_util.h
    utils/hash_vk_types.h
    utils/vk_layer_extension_utils.cpp
    utils/vk_layer_extension_utils.h
    utils/ray_tracing_utils.cpp
    utils/ray_tracing_utils.h
    utils/vk_layer_utils.cpp
    utils/vk_layer_utils.h
    vk_layer_config.h
    vk_layer_config.cpp
)

# XXH_NO_LONG_LONG: removes compilation of algorithms relying on 64-bit types (XXH3 and XXH64). Only XXH32 will be compiled.
# We only need XXH32 due to restrictions requiring a 32 bit hash. This also reduces binary size.
#
# v0.8.1 also has compilation issues that are removed by setting this define.
# https://github.com/KhronosGroup/Vulkan-ValidationLayers/pull/4639
# https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/4640
target_compile_definitions(VkLayer_utils PUBLIC XXH_NO_LONG_LONG)

target_link_libraries(VkLayer_utils PUBLIC
    Vulkan::Headers
    Vulkan::LayerSettings
    Vulkan::UtilityHeaders
    ${CMAKE_DL_LIBS}
)
target_include_directories(VkLayer_utils SYSTEM PRIVATE external)
target_include_directories(VkLayer_utils PUBLIC . ${API_TYPE})

find_package(robin_hood CONFIG)
option(USE_ROBIN_HOOD_HASHING "robin_hood provides faster versions of std::unordered_map and std::unordered_set" ${robin_hood_FOUND})
if (USE_ROBIN_HOOD_HASHING)
    target_link_libraries(VkLayer_utils PUBLIC robin_hood::robin_hood)
    target_compile_definitions(VkLayer_utils PUBLIC USE_ROBIN_HOOD_HASHING)
endif()

if(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
    target_compile_options(VkLayer_utils PRIVATE
        -Wno-sign-conversion
        -Wno-implicit-int-conversion
    )
elseif(MSVC)
    target_compile_options(VkLayer_utils PRIVATE
        /wd4324 # padding
        /wd4458 # hiding class member
        /wd4457 # hiding function parameter
        /wd4702 # unreachable code
        /wd4389 # signed/unsigned mismatch
    )
endif()

add_library(vvl MODULE)

target_sources(vvl PRIVATE
    best_practices/best_practices_error_enums.h
    best_practices/best_practices_utils.cpp
    best_practices/bp_buffer.cpp
    best_practices/bp_cmd_buffer.cpp
    best_practices/bp_copy_blit_resolve.cpp
    best_practices/bp_descriptor.cpp
    best_practices/bp_device_memory.cpp
    best_practices/bp_drawdispatch.cpp
    best_practices/bp_framebuffer.cpp
    best_practices/bp_image.cpp
    best_practices/bp_instance_device.cpp
    best_practices/bp_pipeline.cpp
    best_practices/bp_ray_tracing.cpp
    best_practices/bp_render_pass.cpp
    best_practices/bp_synchronization.cpp
    best_practices/bp_video.cpp
    best_practices/bp_wsi.cpp
    best_practices/best_practices_validation.h
    containers/qfo_transfer.h
    containers/range_vector.h
    containers/sparse_containers.h
    containers/subresource_adapter.cpp
    containers/subresource_adapter.h
    core_checks/cc_android.cpp
    core_checks/cc_buffer.cpp
    core_checks/cc_buffer_address.h
    core_checks/cc_cmd_buffer_dynamic.cpp
    core_checks/cc_cmd_buffer.cpp
    core_checks/cc_copy_blit_resolve.cpp
    core_checks/core_validation.h
    core_checks/cc_descriptor.cpp
    core_checks/cc_device.cpp
    core_checks/cc_device_memory.cpp
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
    core_checks/cc_synchronization.cpp
    core_checks/cc_video.cpp
    core_checks/cc_wsi.cpp
    core_checks/cc_ycbcr.cpp
    error_message/validation_error_enums.h
    external/vma/vma.h
    external/vma/vma.cpp
    ${API_TYPE}/generated/best_practices.cpp
    ${API_TYPE}/generated/best_practices.h
    ${API_TYPE}/generated/chassis.cpp
    ${API_TYPE}/generated/valid_enum_values.cpp
    ${API_TYPE}/generated/valid_enum_values.h
    ${API_TYPE}/generated/command_validation.cpp
    ${API_TYPE}/generated/dynamic_state_helper.cpp
    ${API_TYPE}/generated/dynamic_state_helper.h
    ${API_TYPE}/generated/enum_flag_bits.h
    ${API_TYPE}/generated/layer_chassis_dispatch.cpp
    layer_chassis_dispatch_manual.cpp
    ${API_TYPE}/generated/object_tracker.cpp
    ${API_TYPE}/generated/object_tracker.h
    ${API_TYPE}/generated/spirv_grammar_helper.cpp
    ${API_TYPE}/generated/spirv_validation_helper.cpp
    ${API_TYPE}/generated/state_tracker_helper.cpp
    ${API_TYPE}/generated/state_tracker_helper.h
    ${API_TYPE}/generated/stateless_validation_helper.cpp
    ${API_TYPE}/generated/stateless_validation_helper.h
    ${API_TYPE}/generated/sync_validation_types.cpp
    ${API_TYPE}/generated/thread_safety.cpp
    ${API_TYPE}/generated/thread_safety_commands.h
    ${API_TYPE}/generated/thread_safety_counter_bodies.h
    ${API_TYPE}/generated/thread_safety_counter_definitions.h
    ${API_TYPE}/generated/thread_safety_counter_instances.h
    ${API_TYPE}/generated/vk_safe_struct_utils.cpp
    ${API_TYPE}/generated/vk_safe_struct_core.cpp
    ${API_TYPE}/generated/vk_safe_struct_khr.cpp
    ${API_TYPE}/generated/vk_safe_struct_ext.cpp
    ${API_TYPE}/generated/vk_safe_struct_vendor.cpp
    ${API_TYPE}/generated/vk_safe_struct.h
    gpu_validation/debug_printf.cpp
    gpu_validation/debug_printf.h
    gpu_validation/gpu_descriptor_set.cpp
    gpu_validation/gpu_descriptor_set.h
    gpu_validation/gpu_error_message.cpp
    gpu_validation/gpu_error_message.h
    gpu_validation/gpu_state_tracker.cpp
    gpu_validation/gpu_state_tracker.h
    gpu_validation/gpu_subclasses.cpp
    gpu_validation/gpu_subclasses.h
    gpu_validation/gpu_validation.cpp
    gpu_validation/gpu_validation.cpp
    gpu_validation/gpu_validation.h
    object_tracker/object_lifetime_validation.h
    object_tracker/object_tracker_utils.cpp
    state_tracker/base_node.cpp
    state_tracker/base_node.h
    state_tracker/buffer_state.cpp
    state_tracker/buffer_state.h
    state_tracker/cmd_buffer_state.cpp
    state_tracker/cmd_buffer_state.h
    state_tracker/descriptor_sets.cpp
    state_tracker/descriptor_sets.h
    state_tracker/device_memory_state.cpp
    state_tracker/device_memory_state.h
    state_tracker/device_state.h
    state_tracker/image_layout_map.cpp
    state_tracker/image_layout_map.h
    state_tracker/image_state.cpp
    state_tracker/image_state.h
    state_tracker/pipeline_layout_state.cpp
    state_tracker/pipeline_layout_state.h
    state_tracker/pipeline_state.cpp
    state_tracker/pipeline_state.h
    state_tracker/pipeline_sub_state.cpp
    state_tracker/pipeline_sub_state.h
    state_tracker/query_state.h
    state_tracker/queue_state.cpp
    state_tracker/queue_state.h
    state_tracker/ray_tracing_state.h
    state_tracker/render_pass_state.cpp
    state_tracker/render_pass_state.h
    state_tracker/sampler_state.h
    state_tracker/shader_instruction.cpp
    state_tracker/shader_instruction.h
    state_tracker/shader_module.cpp
    state_tracker/shader_module.h
    state_tracker/shader_object_state.cpp
    state_tracker/shader_object_state.h
    state_tracker/state_tracker.cpp
    state_tracker/state_tracker.h
    state_tracker/video_session_state.cpp
    state_tracker/video_session_state.h
    stateless/sl_buffer.cpp
    stateless/sl_cmd_buffer_dynamic.cpp
    stateless/sl_cmd_buffer.cpp
    stateless/sl_descriptor.cpp
    stateless/sl_device_memory.cpp
    stateless/sl_external_object.cpp
    stateless/sl_framebuffer.cpp
    stateless/sl_image.cpp
    stateless/sl_instance_device.cpp
    stateless/sl_pipeline.cpp
    stateless/sl_ray_tracing.cpp
    stateless/sl_render_pass.cpp
    stateless/sl_shader_object.cpp
    stateless/sl_synchronization.cpp
    stateless/sl_utils.cpp
    stateless/sl_wsi.cpp
    stateless/stateless_validation.h
    sync/sync_validation.cpp
    sync/sync_validation.h
    sync/sync_utils.cpp
    sync/sync_utils.h
    sync/sync_vuid_maps.cpp
    sync/sync_vuid_maps.h
    thread_tracker/thread_safety_validation.cpp
    thread_tracker/thread_safety_validation.h
    utils/shader_utils.cpp
    utils/shader_utils.h
    layer_options.cpp
    layer_options.h
)
get_target_property(LAYER_SOURCES vvl SOURCES)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${LAYER_SOURCES})

set_target_properties(vvl PROPERTIES OUTPUT_NAME ${LAYER_NAME})

if(MSVC)
    target_link_options(vvl PRIVATE /DEF:${CMAKE_CURRENT_SOURCE_DIR}/${LAYER_NAME}.def)
    target_compile_options(vvl PRIVATE /bigobj)
elseif(MINGW)
    target_sources(vvl PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/${LAYER_NAME}.def)
    target_compile_options(vvl PRIVATE -Wa,-mbig-obj)
elseif(APPLE)
    set_target_properties(vvl PROPERTIES SUFFIX ".dylib")
    target_link_options(vvl PRIVATE -exported_symbols_list ${CMAKE_CURRENT_SOURCE_DIR}/${LAYER_NAME}.exp)
elseif(ANDROID)
    target_link_options(vvl PRIVATE LINKER:--version-script=${CMAKE_CURRENT_SOURCE_DIR}/${LAYER_NAME}-android.map)
else()
    target_link_options(vvl PRIVATE LINKER:--version-script=${CMAKE_CURRENT_SOURCE_DIR}/${LAYER_NAME}.map,-Bsymbolic,--exclude-libs,ALL)
endif()

target_compile_options(vvl PRIVATE "$<IF:$<CXX_COMPILER_ID:MSVC>,/wd4100,-Wno-unused-parameter>")

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(vvl PRIVATE
        -Wno-sign-conversion
        -Wno-implicit-int-conversion
    )
elseif(MSVC)
    target_compile_options(vvl PRIVATE
        /wd4324 # padding
        /wd4458 # hiding class member
        /wd4457 # hiding function parameter
        /wd4702 # unreachable code
        /wd4389 # signed/unsigned mismatch
    )
endif()

# Khronos validation additional dependencies
if (USE_ROBIN_HOOD_HASHING)
    target_link_libraries(vvl PRIVATE robin_hood::robin_hood)
endif()

# Order matters here. VkLayer_utils should be the last link library to ensure mimalloc overrides are picked up correctly.
# Otherwise, libraries after VkLayer_utils will not benefit from this performance improvement.
target_link_libraries(vvl PRIVATE
    SPIRV-Headers::SPIRV-Headers
    SPIRV-Tools-opt
    SPIRV-Tools-link
    VkLayer_utils
)

# Using mimalloc on non-Windows OSes currently results in unit test instability with some
# OS version / driver combinations. On 32-bit systems, using mimalloc cause an increase in
# the amount of virtual address space needed, which can also cause stability problems.
if (MSVC AND CMAKE_SIZEOF_VOID_P EQUAL 8)
   find_package(mimalloc CONFIG)
   option(USE_MIMALLOC "Use mimalloc, a fast malloc/free replacement library" ${mimalloc_FOUND})
   if (USE_MIMALLOC)
      target_compile_definitions(vvl PRIVATE USE_MIMALLOC)
      target_link_libraries(vvl PRIVATE mimalloc-static)
   endif()
endif()

target_include_directories(vvl SYSTEM PRIVATE external)

if (ANDROID)
    # https://gitlab.kitware.com/cmake/cmake/issues/18787
    # https://github.com/android-ndk/ndk/issues/463
    # "Users should be able to reliably use the toolchain provided by the NDK r23 or later when using CMake 3.21 or later" - Professional CMake
    if (CMAKE_VERSION VERSION_LESS "3.21")
        message(FATAL_ERROR "Android build requires at least CMake 3.21!")
    endif()

    # Required for __android_log_print. Marking as PUBLIC since the tests use __android_log_print as well.
    target_link_libraries(VkLayer_utils PUBLIC log)

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
set(OUTPUT_FILE_FINAL_NAME "${LAYER_NAME}.json")
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
    install(FILES $<TARGET_PDB_FILE:vvl> DESTINATION ${LAYER_INSTALL_DIR})
endif()

install(TARGETS vvl DESTINATION ${LAYER_INSTALL_DIR})
