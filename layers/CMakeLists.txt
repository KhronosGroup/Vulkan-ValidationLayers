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

# XXH_NO_LONG_LONG: removes compilation of algorithms relying on 64-bit types (XXH3 and XXH64). Only XXH32 will be compiled.
# We only need XXH32 due to restrictions requiring a 32 bit hash. This also reduces binary size.
#
# v0.8.1 also has compilation issues that are removed by setting this define.
# https://github.com/KhronosGroup/Vulkan-ValidationLayers/pull/4639
# https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/4640
add_compile_definitions(XXH_NO_LONG_LONG)

add_library(VkLayer_utils STATIC)
target_sources(VkLayer_utils PRIVATE
    generated/vk_format_utils.h
    generated/vk_format_utils.cpp
    generated/vk_validation_error_messages.h
    generated/vk_layer_dispatch_table.h
    generated/vk_dispatch_table_helper.h
    generated/vk_safe_struct.h
    generated/vk_safe_struct.cpp
    # NOTE: This header file is installed by the LunarG SDK and treated as part of VulkanHeaders
    # IE: This header file is also a deliverable of VVL.
    generated/vk_enum_string_helper.h
    generated/vk_object_types.h
    generated/vk_extension_helper.h
    generated/vk_typemap_helper.h
    cast_utils.h
    hash_util.h
    hash_vk_types.h
    vk_layer_config.h
    vk_layer_config.cpp
    vk_layer_data.h
    vk_layer_extension_utils.h
    vk_layer_extension_utils.cpp
    vk_layer_logging.h
    vk_layer_logging.cpp
    vk_layer_utils.h
    vk_layer_utils.cpp
    xxhash.h
    xxhash.c
)
target_link_libraries(VkLayer_utils PUBLIC Vulkan::Headers)
set_target_properties(VkLayer_utils PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(VkLayer_utils PUBLIC
    .
    generated
)

find_package(robin_hood CONFIG)
option(USE_ROBIN_HOOD_HASHING "robin_hood provides faster versions of std::unordered_map and std::unordered_set" ${robin_hood_FOUND})
if (USE_ROBIN_HOOD_HASHING)
    target_link_libraries(VkLayer_utils PRIVATE robin_hood::robin_hood)
    target_compile_definitions(robin_hood::robin_hood INTERFACE USE_ROBIN_HOOD_HASHING)
endif()

# Using mimalloc on non-Windows OSes currently results in unit test instability with some
# OS version / driver combinations. On 32-bit systems, using mimalloc cause an increase in
# the amount of virtual address space needed, which can also cause stability problems.
if (MSVC AND CMAKE_SIZEOF_VOID_P EQUAL 8)
   find_package(mimalloc CONFIG)
   option(USE_MIMALLOC "Use mimalloc, a fast malloc/free replacement library" ${mimalloc_FOUND})
   if (USE_MIMALLOC)
      target_compile_definitions(mimalloc-static INTERFACE USE_MIMALLOC)
      target_link_libraries(VkLayer_utils PRIVATE mimalloc-static)
   endif()
endif()

# Currently repos like LunarG/VulkanTools require source code from VVL
# This results in VkLayer_utils needing to be installed. Ideally this is
# cleaned up in the future since this is a result of history involving the mono-repo.
if(BUILD_LAYER_SUPPORT_FILES)
    # NOTE: We need to install header/source files for Android
    get_target_property(LAYER_UTIL_FILES VkLayer_utils SOURCES)
    install(FILES ${LAYER_UTIL_FILES} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/vulkan)
    install(TARGETS VkLayer_utils)
endif()

if(MSVC)
    # Avoid: fatal error C1128: number of sections exceeded object file format limit: compile with /bigobj
    add_compile_options("/bigobj")
    add_compile_options("/we4189")
    add_compile_options("/we5038")
else()
    add_compile_options(-Wpointer-arith -Wno-unused-function -Wno-sign-compare)
endif()

# Clang warns about unused const variables. Generated files may purposely contain unused consts, so silence this warning in Clang
if(CMAKE_C_COMPILER_ID MATCHES "Clang")
    set_source_files_properties(generated/parameter_validation.cpp PROPERTIES COMPILE_FLAGS "-Wno-unused-const-variable")
endif()

# NOTE: LunarG/VulkanTools disables BUILD_LAYERS and BUILD_TESTS to minimize dependencies.
if (NOT BUILD_LAYERS)
    return()
endif()

# Represents all SPIRV libraries we need
add_library(VVL-SPIRV-LIBS INTERFACE)

find_package(SPIRV-Headers REQUIRED CONFIG QUIET)
target_link_libraries(VVL-SPIRV-LIBS INTERFACE SPIRV-Headers::SPIRV-Headers)

find_package(SPIRV-Tools-opt REQUIRED CONFIG QUIET)
target_link_libraries(VVL-SPIRV-LIBS INTERFACE SPIRV-Tools-opt)

find_package(SPIRV-Tools REQUIRED CONFIG QUIET)

# See https://github.com/KhronosGroup/SPIRV-Tools/issues/3909 for background on this.
# The targets available from SPIRV-Tools change depending on how SPIRV_TOOLS_BUILD_STATIC is set.
# Try to handle all possible combinations so that we work with externally built packages.
if (TARGET SPIRV-Tools)
    target_link_libraries(VVL-SPIRV-LIBS INTERFACE SPIRV-Tools)
elseif(TARGET SPIRV-Tools-static)
    target_link_libraries(VVL-SPIRV-LIBS INTERFACE SPIRV-Tools-static)
elseif(TARGET SPIRV-Tools-shared)
    target_link_libraries(VVL-SPIRV-LIBS INTERFACE SPIRV-Tools-shared)
else()
    message(FATAL_ERROR "Cannot determine SPIRV-Tools target name")
endif()

find_package(PythonInterp 3 QUIET)

if (PYTHONINTERP_FOUND)
    # Get the include directory of the SPIRV-Headers
    get_target_property(SPIRV_HEADERS_INCLUDE_DIR SPIRV-Headers::SPIRV-Headers INTERFACE_INCLUDE_DIRECTORIES)

    set(spirv_unified_include_dir "${SPIRV_HEADERS_INCLUDE_DIR}/spirv/unified1/")
    if (NOT IS_DIRECTORY ${spirv_unified_include_dir})
        message(FATAL_ERROR "Unable to find spirv/unified1")
    endif()

    set(generate_source_py "${VVL_SOURCE_DIR}/scripts/generate_source.py")
    if (NOT EXISTS ${generate_source_py})
        message(FATAL_ERROR "Unable to find generate_source.py!")
    endif()

    add_custom_target(VulkanVL_generated_source
        COMMAND ${PYTHON_EXECUTABLE} ${generate_source_py} ${VULKAN_HEADERS_REGISTRY_DIRECTORY} ${spirv_unified_include_dir} --incremental
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/generated
    )

    set_target_properties(VulkanVL_generated_source PROPERTIES FOLDER ${LAYERS_HELPER_FOLDER})
else()
    message(NOTICE "VulkanVL_generated_source target requires python 3")
endif()

add_library(VkLayer_khronos_validation MODULE)

target_sources(VkLayer_khronos_validation PRIVATE
    generated/best_practices.cpp
    generated/best_practices.h
    generated/chassis.cpp
    generated/command_validation.cpp
    generated/enum_flag_bits.h
    generated/layer_chassis_dispatch.cpp
    generated/object_tracker.cpp
    generated/object_tracker.h
    generated/parameter_validation.cpp
    generated/parameter_validation.h
    generated/spirv_grammar_helper.cpp
    generated/spirv_validation_helper.cpp
    generated/synchronization_validation_types.cpp
    generated/thread_safety.cpp
    generated/thread_safety.h
    generated/vk_safe_struct.cpp
    generated/vk_safe_struct.h
    android_validation.cpp
    base_node.cpp
    base_node.h
    best_practices_error_enums.h
    best_practices_utils.cpp
    best_practices_validation.h
    buffer_state.cpp
    buffer_state.h
    buffer_validation.cpp
    cmd_buffer_dynamic_validation.cpp
    cmd_buffer_state.cpp
    cmd_buffer_state.h
    convert_to_renderpass2.cpp
    core_error_location.cpp
    core_error_location.h
    core_validation.cpp
    core_validation_error_enums.h
    core_validation.h
    debug_printf.cpp
    debug_printf.h
    descriptor_sets.cpp
    descriptor_sets.h
    descriptor_validation.cpp
    device_validation.cpp
    device_memory_state.cpp
    device_memory_state.h
    device_memory_validation.cpp
    drawdispatch.cpp
    external_object_validation.cpp
    gpu_utils.cpp
    gpu_utils.h
    gpu_validation.cpp
    gpu_validation.cpp
    gpu_validation.h
    image_layout_map.cpp
    image_layout_map.h
    image_state.cpp
    image_state.h
    layer_options.cpp
    object_lifetime_validation.h
    object_tracker_utils.cpp
    parameter_validation_utils.cpp
    pipeline_layout_state.cpp
    pipeline_state.cpp
    pipeline_state.h
    pipeline_sub_state.cpp
    pipeline_validation.cpp
    query_state.h
    query_validation.cpp
    queue_state.cpp
    queue_state.h
    range_vector.h
    ray_tracing_state.h
    ray_tracing_validation.cpp
    render_pass_state.cpp
    render_pass_state.h
    render_pass_validation.cpp
    sampler_state.h
    shader_instruction.cpp
    shader_instruction.h
    shader_module.cpp
    shader_module.h
    shader_validation.cpp
    shader_validation.h
    stateless_validation.h
    state_tracker.cpp
    state_tracker.h
    subresource_adapter.cpp
    subresource_adapter.h
    synchronization_validation.cpp
    synchronization_validation.h
    sync_utils.cpp
    sync_utils.h
    sync_vuid_maps.cpp
    sync_vuid_maps.h
    video_session_state.cpp
    video_session_state.h
    video_validation.cpp
    vk_layer_settings_ext.h
    wsi_validation.cpp
    xxhash.c
    xxhash.h
)

target_compile_definitions(VkLayer_khronos_validation PUBLIC ${KHRONOS_LAYER_COMPILE_DEFINITIONS})

if(WIN32)
    target_link_options(VkLayer_khronos_validation PRIVATE /DEF:${CMAKE_CURRENT_SOURCE_DIR}/VkLayer_khronos_validation.def)
elseif(APPLE)
    set_target_properties(VkLayer_khronos_validation PROPERTIES SUFFIX ".dylib")
else()
    target_link_options(VkLayer_khronos_validation PRIVATE LINKER:--version-script=${CMAKE_CURRENT_SOURCE_DIR}/libVkLayer_khronos_validation.map,-Bsymbolic,--exclude-libs,ALL)
endif()

# Khronos validation additional dependencies
if (USE_ROBIN_HOOD_HASHING)
    target_link_libraries(VkLayer_khronos_validation PRIVATE robin_hood::robin_hood)
endif()
# Order matters here. VkLayer_utils should be the last link library to ensure mimalloc overrides are picked up correctly.
# Otherwise, libraries after VkLayer_utils will not benefit from this performance improvement.
target_link_libraries(VkLayer_khronos_validation PRIVATE VVL-SPIRV-LIBS VkLayer_utils)

# There are 2 primary deliverables for the validation layers.
# - The actual library VkLayer_khronos_validation.(dll|so|dylib)
# - The respective json file, VkLayer_khronos_validation.json
# This code generates the appropriate json for both local testing and the installation.
# NOTE: For WIN32 the JSON and dll MUST be placed in the same location, due to Win32 using a relative path for installation.
set(INPUT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/json/VkLayer_khronos_validation.json.in")
set(INTERMEDIATE_FILE "${CMAKE_CURRENT_BINARY_DIR}/json/validation.json")
set(OUTPUT_FILE_FINAL_NAME "VkLayer_khronos_validation.json")
set(LAYER_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR})
if (WIN32)
    set(LAYER_INSTALL_DIR ${CMAKE_INSTALL_BINDIR}) # WIN32/MINGW expect the dll in the `bin` dir, this matches our WIN32 SDK process
endif()

if (WIN32)
    set(JSON_LIBRARY_PATH ".\\\\VkLayer_khronos_validation.dll")
elseif(APPLE)
    set(JSON_LIBRARY_PATH "./libVkLayer_khronos_validation.dylib")
else()
    set(JSON_LIBRARY_PATH "./libVkLayer_khronos_validation.so")
endif()

set(JSON_API_VERSION ${VulkanHeaders_VERSION})

configure_file(${INPUT_FILE} ${INTERMEDIATE_FILE} @ONLY)

# To support both multi/single configuration generators just copy the json to the correct directory
add_custom_command(TARGET VkLayer_khronos_validation POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${INTERMEDIATE_FILE} $<TARGET_FILE_DIR:VkLayer_khronos_validation>/${OUTPUT_FILE_FINAL_NAME}
)

# For UNIX-based systems, `library_path` should not contain a relative path (indicated by "./") before installing to system directories
# This json isn't used for regular local development, it's used for installation
if (UNIX)
    set(UNIX_INTERMEDIATE_FILE "${CMAKE_CURRENT_BINARY_DIR}/json/unix_install_validation.json")

    if(APPLE)
        set(JSON_LIBRARY_PATH "libVkLayer_khronos_validation.dylib")
    else()
        set(JSON_LIBRARY_PATH "libVkLayer_khronos_validation.so")
    endif()

    configure_file(${INPUT_FILE} ${UNIX_INTERMEDIATE_FILE} @ONLY)

    install(FILES ${UNIX_INTERMEDIATE_FILE} DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/vulkan/explicit_layer.d RENAME ${OUTPUT_FILE_FINAL_NAME})
endif()

if (WIN32)
    install(FILES ${INTERMEDIATE_FILE} DESTINATION ${LAYER_INSTALL_DIR} RENAME ${OUTPUT_FILE_FINAL_NAME})
    install(FILES $<TARGET_PDB_FILE:VkLayer_khronos_validation> DESTINATION ${LAYER_INSTALL_DIR})
endif()

install(TARGETS VkLayer_khronos_validation DESTINATION ${LAYER_INSTALL_DIR})
