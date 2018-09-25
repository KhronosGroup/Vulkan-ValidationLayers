# ~~~
# Copyright (c) 2014-2018 Valve Corporation
# Copyright (c) 2014-2018 LunarG, Inc.
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

cmake_minimum_required(VERSION 2.8.11)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_definitions(-DVK_USE_PLATFORM_WIN32_KHR -DVK_USE_PLATFORM_WIN32_KHX -DWIN32_LEAN_AND_MEAN)
    add_custom_target(mk_layer_config_dir ALL
                      COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>)
    set_target_properties(mk_layer_config_dir PROPERTIES FOLDER ${LAYERS_HELPER_FOLDER})
elseif(CMAKE_SYSTEM_NAME STREQUAL "Android")
    add_definitions(-DVK_USE_PLATFORM_ANDROID_KHR -DVK_USE_PLATFORM_ANDROID_KHX)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    if(BUILD_WSI_XCB_SUPPORT)
        add_definitions(-DVK_USE_PLATFORM_XCB_KHR -DVK_USE_PLATFORM_XCB_KHX)
    endif()

    if(BUILD_WSI_XLIB_SUPPORT)
        add_definitions(-DVK_USE_PLATFORM_XLIB_KHR -DVK_USE_PLATFORM_XLIB_KHX -DVK_USE_PLATFORM_XLIB_XRANDR_EXT)
    endif()

    if(BUILD_WSI_WAYLAND_SUPPORT)
        add_definitions(-DVK_USE_PLATFORM_WAYLAND_KHR -DVK_USE_PLATFORM_WAYLAND_KHX)
    endif()

    if(BUILD_WSI_MIR_SUPPORT)
        add_definitions(-DVK_USE_PLATFORM_MIR_KHR -DVK_USE_PLATFORM_MIR_KHX)
        include_directories(${MIR_INCLUDE_DIR})
    endif()
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    add_definitions(-DVK_USE_PLATFORM_MACOS_MVK)
    if(CMAKE_GENERATOR MATCHES "^Xcode.*")
        add_custom_target(mk_layer_config_dir ALL
                          COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>)
    endif()
else()
    message(FATAL_ERROR "Unsupported Platform!")
endif()

# Define macro used for generating header files containing commit IDs for external dependencies
macro(run_external_revision_generate symbol_name output)
    add_custom_command(OUTPUT ${output}
                       COMMAND ${PYTHON_EXECUTABLE} ${SCRIPTS_DIR}/external_revision_generator.py
                               --from_uuid -s ${symbol_name} -o ${output}
                       DEPENDS ${SCRIPTS_DIR}/external_revision_generator.py)
endmacro()

# Custom targets for generated validation layer helper file dependencies
add_custom_target(spirv_tools_revision_file DEPENDS spirv_tools_commit_id.h)

set_target_properties(spirv_tools_revision_file PROPERTIES FOLDER ${LAYERS_HELPER_FOLDER})

if(BUILD_LAYERS)
    run_external_revision_generate(SPIRV_TOOLS_COMMIT_ID spirv_tools_commit_id.h)
endif()

# Configure installation of source files that are dependencies of other repos.
set(LAYER_UTIL_FILES
    hash_util.h
    hash_vk_types.h
    vk_format_utils.h
    vk_format_utils.cpp
    vk_layer_config.h
    vk_layer_config.cpp
    vk_layer_data.h
    vk_layer_extension_utils.h
    vk_layer_extension_utils.cpp
    vk_layer_logging.h
    vk_layer_utils.h
    vk_layer_utils.cpp
    vk_loader_layer.h
    vk_loader_platform.h
    vk_validation_error_messages.h
    ${PROJECT_BINARY_DIR}/vk_layer_dispatch_table.h
    ${PROJECT_BINARY_DIR}/vk_dispatch_table_helper.h
    ${PROJECT_BINARY_DIR}/vk_safe_struct.h
    ${PROJECT_BINARY_DIR}/vk_safe_struct.cpp
    ${PROJECT_BINARY_DIR}/vk_enum_string_helper.h
    ${PROJECT_BINARY_DIR}/vk_object_types.h
    ${PROJECT_BINARY_DIR}/vk_extension_helper.h
    ${PROJECT_BINARY_DIR}/vk_typemap_helper.h)
install(FILES ${LAYER_UTIL_FILES} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

set(TARGET_NAMES
    VkLayer_core_validation
    VkLayer_object_tracker
    VkLayer_unique_objects
    VkLayer_parameter_validation
    VkLayer_standard_validation
    VkLayer_threading)

# Install the layer json files
if(WIN32)
    if(CMAKE_GENERATOR MATCHES "^Visual Studio.*")
        foreach(TARGET_NAME ${TARGET_NAMES})
            install(FILES ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/${TARGET_NAME}.json DESTINATION ${CMAKE_INSTALL_LIBDIR})
        endforeach()
    else()
        foreach(TARGET_NAME ${TARGET_NAMES})
            install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}.json DESTINATION ${CMAKE_INSTALL_LIBDIR})
        endforeach()
    endif()
elseif(UNIX) # UNIX includes APPLE
    foreach(TARGET_NAME ${TARGET_NAMES})
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/staging-json/${TARGET_NAME}.json
                DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/vulkan/explicit_layer.d)
    endforeach()
endif()

if(WIN32)
    macro(add_vk_layer target)
        file(TO_NATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/VkLayer_${target}.def DEF_FILE)
        add_custom_target(copy-${target}-def-file ALL
                          COMMAND ${CMAKE_COMMAND} -E copy_if_different ${DEF_FILE} VkLayer_${target}.def
                          VERBATIM)
        set_target_properties(copy-${target}-def-file PROPERTIES FOLDER ${LAYERS_HELPER_FOLDER})
        add_library(VkLayer_${target} SHARED ${ARGN} VkLayer_${target}.def)
        add_dependencies(VkLayer_${target} generate_helper_files)
        target_link_libraries(VkLayer_${target} VkLayer_utils)
        add_dependencies(VkLayer_${target} generate_helper_files VkLayer_utils)
        install(TARGETS VkLayer_${target} DESTINATION ${CMAKE_INSTALL_LIBDIR})
    endmacro()
elseif(APPLE)
    macro(add_vk_layer target)
        add_library(VkLayer_${target} SHARED ${ARGN})
        target_link_libraries(VkLayer_${target} VkLayer_utils)
        add_dependencies(VkLayer_${target} generate_helper_files VkLayer_utils)
        set_target_properties(VkLayer_${target}
                              PROPERTIES LINK_FLAGS
                                         "-Wl"
                                         INSTALL_RPATH
                                         "@loader_path/")
        install(TARGETS VkLayer_${target} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    endmacro()
else()
    macro(add_vk_layer target)
        add_library(VkLayer_${target} SHARED ${ARGN})
        target_link_libraries(VkLayer_${target} VkLayer_utils)
        add_dependencies(VkLayer_${target} generate_helper_files VkLayer_utils)
        set_target_properties(VkLayer_${target} PROPERTIES LINK_FLAGS "-Wl,-Bsymbolic,--exclude-libs,ALL")
        install(TARGETS VkLayer_${target} DESTINATION ${CMAKE_INSTALL_LIBDIR})
    endmacro()
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${VulkanHeaders_INCLUDE_DIR})

if(WIN32)
    # Applies to all configurations
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    # Avoid: fatal error C1128: number of sections exceeded object file format limit: compile with /bigobj
    set_source_files_properties(core_validation.cpp threading.cpp parameter_validation_utils.cpp unique_objects.cpp PROPERTIES COMPILE_FLAGS "/bigobj")
    # Turn off transitional "changed behavior" warning message for Visual Studio versions prior to 2015. The changed behavior is
    # that constructor initializers are now fixed to clear the struct members.
    add_compile_options("$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,19>>:/wd4351>")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wpointer-arith -Wno-unused-function -Wno-sign-compare")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wpointer-arith -Wno-unused-function -Wno-sign-compare")
endif()

# Clang warns about unused const variables. Generated files may purposely contain unused consts, so silence this warning in Clang
if(CMAKE_C_COMPILER_ID MATCHES "Clang")
    set_source_files_properties(parameter_validation.cpp PROPERTIES COMPILE_FLAGS "-Wno-unused-const-variable")
endif()

run_vk_xml_generate(threading_generator.py thread_check.h)
run_vk_xml_generate(parameter_validation_generator.py parameter_validation.cpp)
run_vk_xml_generate(unique_objects_generator.py unique_objects_wrappers.h)
run_vk_xml_generate(dispatch_table_helper_generator.py vk_dispatch_table_helper.h)
run_vk_xml_generate(object_tracker_generator.py object_tracker.cpp)

if(BUILD_LAYERS)
    add_vk_layer(core_validation core_validation.cpp descriptor_sets.cpp buffer_validation.cpp shader_validation.cpp xxhash.c)
    add_vk_layer(object_tracker object_tracker.cpp object_tracker_utils.cpp)
    add_vk_layer(threading threading.cpp thread_check.h)
    add_vk_layer(unique_objects unique_objects.cpp unique_objects_wrappers.h)
    add_vk_layer(parameter_validation
                 parameter_validation.cpp
                 parameter_validation_utils.cpp
                 parameter_validation.h
                 vk_validation_error_messages.h)

    # Core validation has additional dependencies
    target_include_directories(VkLayer_core_validation PRIVATE ${GLSLANG_SPIRV_INCLUDE_DIR})
    target_include_directories(VkLayer_core_validation PRIVATE ${SPIRV_TOOLS_INCLUDE_DIR})
    target_link_libraries(VkLayer_core_validation ${SPIRV_TOOLS_LIBRARIES})
    add_dependencies(VkLayer_core_validation spirv_tools_revision_file)
endif()

# The output file needs Unix "/" separators or Windows "\" separators On top of that, Windows separators actually need to be doubled
# because the json format uses backslash escapes
file(TO_NATIVE_PATH "./" RELATIVE_PATH_PREFIX)
string(REPLACE "\\"
               "\\\\"
               RELATIVE_PATH_PREFIX
               "${RELATIVE_PATH_PREFIX}")

# Run each .json.in file through the generator We need to create the generator.cmake script so that the generator can be run at
# compile time, instead of configure time Running at compile time lets us use cmake generator expressions (TARGET_FILE_NAME and
# TARGET_FILE_DIR, specifically)
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/generator.cmake" "configure_file(\"\${INPUT_FILE}\" \"\${OUTPUT_FILE}\")")
foreach(TARGET_NAME ${TARGET_NAMES})
    set(CONFIG_DEFINES -DINPUT_FILE="${CMAKE_CURRENT_SOURCE_DIR}/json/${TARGET_NAME}.json.in" -DVK_VERSION=1.1.${vk_header_version})
    # If this json file is not a metalayer, get the needed properties from that target
    if(TARGET ${TARGET_NAME})
        set(CONFIG_DEFINES
            ${CONFIG_DEFINES}
            -DOUTPUT_FILE="$<TARGET_FILE_DIR:${TARGET_NAME}>/${TARGET_NAME}.json"
            -DRELATIVE_LAYER_BINARY="${RELATIVE_PATH_PREFIX}$<TARGET_FILE_NAME:${TARGET_NAME}>")
        # If this json file is a metalayer, make the output path match core validation, and there is no layer binary file
    else()
        set(CONFIG_DEFINES ${CONFIG_DEFINES} -DOUTPUT_FILE="$<TARGET_FILE_DIR:VkLayer_core_validation>/${TARGET_NAME}.json")
    endif()
    add_custom_target(${TARGET_NAME}-json ALL
                      COMMAND ${CMAKE_COMMAND} ${CONFIG_DEFINES} -P "${CMAKE_CURRENT_BINARY_DIR}/generator.cmake")
endforeach()

# For UNIX-based systems, `library_path` should not contain a relative path (indicated by "./") before installing to system
# directories, so we do not include it in the staging-json files which are used for installation
if(UNIX)
    foreach(TARGET_NAME ${TARGET_NAMES})
        set(INSTALL_DEFINES
            -DINPUT_FILE="${CMAKE_CURRENT_SOURCE_DIR}/json/${TARGET_NAME}.json.in"
            -DOUTPUT_FILE="${CMAKE_CURRENT_BINARY_DIR}/staging-json/${TARGET_NAME}.json"
            -DVK_VERSION=1.1.${vk_header_version})
        # If this json file is not a metalayer, get the needed properties from that target
        if(TARGET ${TARGET_NAME})
            set(INSTALL_DEFINES ${INSTALL_DEFINES} -DRELATIVE_LAYER_BINARY="$<TARGET_FILE_NAME:${TARGET_NAME}>")
        endif()
        add_custom_target(${TARGET_NAME}-staging-json ALL
                          COMMAND ${CMAKE_COMMAND} ${INSTALL_DEFINES} -P "${CMAKE_CURRENT_BINARY_DIR}/generator.cmake")
    endforeach()
endif()
