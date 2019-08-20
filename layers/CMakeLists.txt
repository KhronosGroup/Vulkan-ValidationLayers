# ~~~
# Copyright (c) 2014-2019 Valve Corporation
# Copyright (c) 2014-2019 LunarG, Inc.
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

if(WIN32)
    add_definitions(-DVK_USE_PLATFORM_WIN32_KHR -DVK_USE_PLATFORM_WIN32_KHX -DWIN32_LEAN_AND_MEAN)
    add_custom_target(mk_layer_config_dir ALL
                      COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>)
    set_target_properties(mk_layer_config_dir PROPERTIES FOLDER ${LAYERS_HELPER_FOLDER})
elseif(ANDROID)
    add_definitions(-DVK_USE_PLATFORM_ANDROID_KHR -DVK_USE_PLATFORM_ANDROID_KHX)
elseif(APPLE)
    add_definitions(-DVK_USE_PLATFORM_MACOS_MVK)
    if(CMAKE_GENERATOR MATCHES "^Xcode.*")
        add_custom_target(mk_layer_config_dir ALL
                          COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>)
    endif()
elseif(UNIX AND NOT APPLE) # i.e. Linux
    if(BUILD_WSI_XCB_SUPPORT)
        add_definitions(-DVK_USE_PLATFORM_XCB_KHR -DVK_USE_PLATFORM_XCB_KHX)
    endif()

    if(BUILD_WSI_XLIB_SUPPORT)
        add_definitions(-DVK_USE_PLATFORM_XLIB_KHR -DVK_USE_PLATFORM_XLIB_KHX -DVK_USE_PLATFORM_XLIB_XRANDR_EXT)
    endif()

    if(BUILD_WSI_WAYLAND_SUPPORT)
        add_definitions(-DVK_USE_PLATFORM_WAYLAND_KHR -DVK_USE_PLATFORM_WAYLAND_KHX)
    endif()
else()
    message(FATAL_ERROR "Unsupported Platform!")
endif()

# Configure installation of source files that are dependencies of other repos.
if(BUILD_LAYER_SUPPORT_FILES)
    set(LAYER_UTIL_FILES
        cast_utils.h
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
        generated/vk_validation_error_messages.h
        generated/vk_layer_dispatch_table.h
        generated/vk_dispatch_table_helper.h
        generated/vk_safe_struct.h
        generated/vk_safe_struct.cpp
        generated/vk_enum_string_helper.h
        generated/vk_object_types.h
        generated/vk_extension_helper.h
        generated/vk_typemap_helper.h)
    install(FILES ${LAYER_UTIL_FILES} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
endif()

set(TARGET_NAMES
    VkLayer_khronos_validation
    VkLayer_core_validation
    VkLayer_object_lifetimes
    VkLayer_unique_objects
    VkLayer_stateless_validation
    VkLayer_standard_validation
    VkLayer_thread_safety)

if(BUILD_LAYERS)
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
endif()

# System-specific macros to create a library target.
if(WIN32)
    macro(AddVkLayer target LAYER_COMPILE_DEFINITIONS)
        file(TO_NATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/VkLayer_${target}.def DEF_FILE)
        add_custom_target(copy-${target}-def-file ALL
                          COMMAND ${CMAKE_COMMAND} -E copy_if_different ${DEF_FILE} VkLayer_${target}.def
                          VERBATIM)
        set_target_properties(copy-${target}-def-file PROPERTIES FOLDER ${LAYERS_HELPER_FOLDER})

        add_library(VkLayer_${target} SHARED ${ARGN} VkLayer_${target}.def)
        target_compile_definitions(VkLayer_${target} PUBLIC ${LAYER_COMPILE_DEFINITIONS})
        target_link_libraries(VkLayer_${target} PRIVATE VkLayer_utils)
        add_dependencies(VkLayer_${target} VkLayer_utils)
        install(TARGETS VkLayer_${target} DESTINATION ${CMAKE_INSTALL_LIBDIR})
    endmacro()
elseif(APPLE)
    macro(AddVkLayer target LAYER_COMPILE_DEFINITIONS)
        add_library(VkLayer_${target} SHARED ${ARGN})
        target_compile_definitions(VkLayer_${target} PUBLIC ${LAYER_COMPILE_DEFINITIONS})
        target_link_libraries(VkLayer_${target} PRIVATE VkLayer_utils)
        add_dependencies(VkLayer_${target} VkLayer_utils)
        set_target_properties(VkLayer_${target}
                              PROPERTIES LINK_FLAGS
                                         "-Wl"
                                         INSTALL_RPATH
                                         "@loader_path/")
        install(TARGETS VkLayer_${target} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    endmacro()
else(UNIX AND NOT APPLE) # i.e.: Linux
    macro(AddVkLayer target LAYER_COMPILE_DEFINITIONS)
        add_library(VkLayer_${target} SHARED ${ARGN})
        target_compile_definitions(VkLayer_${target} PUBLIC ${LAYER_COMPILE_DEFINITIONS})
        target_link_libraries(VkLayer_${target} PRIVATE VkLayer_utils)
        add_dependencies(VkLayer_${target} VkLayer_utils)
        set_target_properties(VkLayer_${target} PROPERTIES LINK_FLAGS "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/libVkLayer_${target}.map,-Bsymbolic,--exclude-libs,ALL")
        install(TARGETS VkLayer_${target} DESTINATION ${CMAKE_INSTALL_LIBDIR})
    endmacro()
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/generated ${VulkanHeaders_INCLUDE_DIR})

if(WIN32)
    # Applies to all configurations
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    # Avoid: fatal error C1128: number of sections exceeded object file format limit: compile with /bigobj
    add_compile_options("/bigobj")
    # Allow Windows to use multiprocessor compilation
    add_compile_options(/MP)
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

set(CHASSIS_LIBRARY_FILES
    generated/chassis.cpp
    generated/layer_chassis_dispatch.cpp)

set(CORE_VALIDATION_LIBRARY_FILES
    core_validation.cpp
    drawdispatch.cpp
    convert_to_renderpass2.cpp
    descriptor_sets.cpp
    buffer_validation.cpp
    shader_validation.cpp
    gpu_validation.cpp
    xxhash.c)

set(OBJECT_LIFETIMES_LIBRARY_FILES
    generated/object_tracker.cpp
    generated/object_tracker.h
    object_tracker_utils.cpp)

set(THREAD_SAFETY_LIBRARY_FILES
    generated/thread_safety.cpp
    generated/thread_safety.h)

set(STATELESS_VALIDATION_LIBRARY_FILES
    generated/parameter_validation.cpp
    generated/parameter_validation.h
    parameter_validation_utils.cpp)

set(BEST_PRACTICES_LIBRARY_FILES
    best_practices.cpp
    best_practices.h)

if(BUILD_LAYERS)
    AddVkLayer(core_validation "BUILD_CORE_VALIDATION"
        ${CHASSIS_LIBRARY_FILES}
        ${CORE_VALIDATION_LIBRARY_FILES})
    AddVkLayer(object_lifetimes "BUILD_OBJECT_TRACKER" ${CHASSIS_LIBRARY_FILES} ${OBJECT_LIFETIMES_LIBRARY_FILES})
    AddVkLayer(thread_safety "BUILD_THREAD_SAFETY" ${CHASSIS_LIBRARY_FILES} ${THREAD_SAFETY_LIBRARY_FILES})
    AddVkLayer(stateless_validation "BUILD_PARAMETER_VALIDATION" ${CHASSIS_LIBRARY_FILES} ${STATELESS_VALIDATION_LIBRARY_FILES})
    AddVkLayer(unique_objects "LAYER_CHASSIS_CAN_WRAP_HANDLES" ${CHASSIS_LIBRARY_FILES})
    AddVkLayer(khronos_validation "BUILD_KHRONOS_VALIDATION;BUILD_CORE_VALIDATION;BUILD_OBJECT_TRACKER;BUILD_THREAD_SAFETY;BUILD_PARAMETER_VALIDATION;LAYER_CHASSIS_CAN_WRAP_HANDLES;BUILD_BEST_PRACTICES"
        ${CHASSIS_LIBRARY_FILES}
        ${CORE_VALIDATION_LIBRARY_FILES}
        ${OBJECT_LIFETIMES_LIBRARY_FILES}
        ${THREAD_SAFETY_LIBRARY_FILES}
        ${STATELESS_VALIDATION_LIBRARY_FILES}
        ${BEST_PRACTICES_LIBRARY_FILES})

    # Core validation and Khronos validation have additional dependencies
    target_include_directories(VkLayer_core_validation PRIVATE ${GLSLANG_SPIRV_INCLUDE_DIR})
    target_include_directories(VkLayer_core_validation PRIVATE ${SPIRV_TOOLS_INCLUDE_DIR})
    target_link_libraries(VkLayer_core_validation PRIVATE ${SPIRV_TOOLS_LIBRARIES})
    target_include_directories(VkLayer_khronos_validation PRIVATE ${GLSLANG_SPIRV_INCLUDE_DIR})
    target_include_directories(VkLayer_khronos_validation PRIVATE ${SPIRV_TOOLS_INCLUDE_DIR})
    target_link_libraries(VkLayer_khronos_validation PRIVATE ${SPIRV_TOOLS_LIBRARIES})

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
        if(CMAKE_GENERATOR MATCHES "^Visual Studio.*")
            set_target_properties(${TARGET_NAME}-json PROPERTIES FOLDER ${LAYERS_HELPER_FOLDER})
        endif()
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
endif()
