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
add_definitions(-DXXH_NO_LONG_LONG)

add_library(VkLayer_utils STATIC)
target_sources(VkLayer_utils PRIVATE
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
    # Turn off transitional "changed behavior" warning message for Visual Studio versions prior to 2015. The changed behavior is
    # that constructor initializers are now fixed to clear the struct members.
    add_compile_options("$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,19>>:/wd4351>")
else()
    add_compile_options(-Wpointer-arith -Wno-unused-function -Wno-sign-compare)
endif()

# Clang warns about unused const variables. Generated files may purposely contain unused consts, so silence this warning in Clang
if(CMAKE_C_COMPILER_ID MATCHES "Clang")
    set_source_files_properties(generated/parameter_validation.cpp PROPERTIES COMPILE_FLAGS "-Wno-unused-const-variable")
endif()

set(CHASSIS_LIBRARY_FILES
    generated/chassis.cpp
    generated/layer_chassis_dispatch.cpp
    generated/vk_safe_struct.cpp
    generated/vk_safe_struct.h
    layer_options.cpp
    state_tracker.cpp
    state_tracker.h
    image_layout_map.cpp
    image_layout_map.h
    range_vector.h
    vk_layer_settings_ext.h
    subresource_adapter.cpp
    subresource_adapter.h
    sync_utils.cpp
    sync_utils.h)

set(CORE_VALIDATION_LIBRARY_FILES
    android_validation.cpp
    core_validation.cpp
    core_validation.h
    core_validation_error_enums.h
    core_error_location.h
    core_error_location.cpp
    base_node.h
    base_node.cpp
    device_memory_state.h
    device_memory_state.cpp
    device_memory_validation.cpp
    buffer_state.h
    buffer_state.cpp
    cmd_buffer_state.h
    cmd_buffer_state.cpp
    image_state.h
    image_state.cpp
    pipeline_state.h
    pipeline_state.cpp
    pipeline_layout_state.cpp
    pipeline_sub_state.cpp
    pipeline_validation.cpp
    queue_state.h
    queue_state.cpp
    query_state.h
    query_validation.cpp
    ray_tracing_state.h
    ray_tracing_validation.cpp
    render_pass_state.h
    render_pass_state.cpp
    video_session_state.h
    video_session_state.cpp
    video_validation.cpp
    sampler_state.h
    drawdispatch.cpp
    convert_to_renderpass2.cpp
    descriptor_sets.cpp
    descriptor_sets.h
    descriptor_validation.cpp
    buffer_validation.cpp
    shader_module.cpp
    shader_module.h
    shader_instruction.cpp
    shader_instruction.h
    shader_validation.cpp
    shader_validation.h
    sync_vuid_maps.cpp
    sync_vuid_maps.h
    wsi_validation.cpp
    generated/spirv_validation_helper.cpp
    generated/spirv_grammar_helper.cpp
    generated/command_validation.cpp
    generated/synchronization_validation_types.cpp
    gpu_validation.cpp
    generated/corechecks_optick_instrumentation.cpp
    xxhash.c
    xxhash.h)

set(OBJECT_LIFETIMES_LIBRARY_FILES
    generated/object_tracker.cpp
    generated/object_tracker.h
    object_tracker_utils.cpp
    object_lifetime_validation.h)

set(THREAD_SAFETY_LIBRARY_FILES
    generated/thread_safety.cpp
    generated/thread_safety.h)

set(STATELESS_VALIDATION_LIBRARY_FILES
    generated/parameter_validation.cpp
    generated/parameter_validation.h
    generated/enum_flag_bits.h
    parameter_validation_utils.cpp
    stateless_validation.h)

set(BEST_PRACTICES_LIBRARY_FILES
    best_practices_utils.cpp
    generated/best_practices.cpp
    generated/best_practices.h
    best_practices_validation.h
    best_practices_error_enums.h)

set(GPU_ASSISTED_LIBRARY_FILES
    gpu_validation.cpp
    gpu_validation.h)

set(DEBUG_PRINTF_LIBRARY_FILES
    debug_printf.cpp
    debug_printf.h)

set(GPU_UTILITY_LIBRARY_FILES
    gpu_utils.cpp
    gpu_utils.h)

set(SYNC_VALIDATION_LIBRARY_FILES
    synchronization_validation.cpp
    synchronization_validation.h)

# Validation Layer performance instrumentation support using Optick.
# https://optick.dev/ https://github.com/bombomby/optick
# To include Optick instrumentation:
# 1) Download the Optick 1.3.1 release: https://github.com/bombomby/optick/releases/tag/1.3.1.0
# 2) Copy the files from the src directory to external/optick.
# 3) Run CMake configure with -DINSTRUMENT_OPTICK.
set(OPTICK_SOURCE_DIR "${PROJECT_SOURCE_DIR}/external/optick")
if(INSTRUMENT_OPTICK)
    # Optick Instrumentation currently supported only on Microsoft toolchain.
    if(MSVC)
        if(IS_DIRECTORY "${OPTICK_SOURCE_DIR}")
            file(GLOB OPTICK_SOURCE_FILES "${OPTICK_SOURCE_DIR}/*.*")
            source_group("Optick Lib" FILES ${OPTICK_SOURCE_FILES})
            # Disable GPU performance measurements (timestamp queries).
            # Not needed for CPU Validation Layer measurements, but would be interesting to try someday.
            # We'll need to map Vulkan API calls into the layer stack API entry points in order to build.
            set(KHRONOS_LAYER_COMPILE_DEFINITIONS
                -DINSTRUMENT_OPTICK
                -DOPTICK_ENABLE_GPU_D3D12=0
                -DOPTICK_ENABLE_GPU_VULKAN=0)
        else()
            message(WARNING "Optick sources not found at ${OPTICK_SOURCE_DIR} - continuing without instrumentation.")
            set(INSTRUMENT_OPTICK OFF)
        endif()
    else()
        message(WARNING "Optick instrumentation not supported on this platform - continuing without instrumentation.")
        set(INSTRUMENT_OPTICK OFF)
    endif()
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
    ${CHASSIS_LIBRARY_FILES}
    ${CORE_VALIDATION_LIBRARY_FILES}
    ${OBJECT_LIFETIMES_LIBRARY_FILES}
    ${THREAD_SAFETY_LIBRARY_FILES}
    ${STATELESS_VALIDATION_LIBRARY_FILES}
    ${BEST_PRACTICES_LIBRARY_FILES}
    ${GPU_UTILITY_LIBRARY_FILES}
    ${GPU_ASSISTED_LIBRARY_FILES}
    ${DEBUG_PRINTF_LIBRARY_FILES}
    ${SYNC_VALIDATION_LIBRARY_FILES}
    ${OPTICK_SOURCE_FILES}
)

target_compile_definitions(VkLayer_khronos_validation PUBLIC ${KHRONOS_LAYER_COMPILE_DEFINITIONS})
target_link_libraries(VkLayer_khronos_validation PRIVATE VkLayer_utils)

if(WIN32)
    target_link_options(VkLayer_khronos_validation PRIVATE /DEF:${CMAKE_CURRENT_SOURCE_DIR}/VkLayer_khronos_validation.def)
elseif(APPLE)
    set_target_properties(VkLayer_khronos_validation PROPERTIES SUFFIX ".dylib")
else()
    target_link_options(VkLayer_khronos_validation PRIVATE LINKER:--version-script=${CMAKE_CURRENT_SOURCE_DIR}/libVkLayer_khronos_validation.map,-Bsymbolic,--exclude-libs,ALL)
endif()

# Khronos validation additional dependencies
if(INSTRUMENT_OPTICK)
    target_include_directories(VkLayer_khronos_validation PRIVATE ${OPTICK_SOURCE_DIR})
endif()
if (USE_ROBIN_HOOD_HASHING)
    target_link_libraries(VkLayer_khronos_validation PRIVATE robin_hood::robin_hood)
    # This warning produces what look like false positives in robin_hood.h with Visual Studio 2015
    target_compile_options(VkLayer_khronos_validation PRIVATE "$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,19.1>>:/wd4996>")
endif()
target_link_libraries(VkLayer_khronos_validation PRIVATE VVL-SPIRV-LIBS)

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
