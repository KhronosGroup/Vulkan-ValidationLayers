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

# This must come before the project command.
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.12" CACHE STRING "Minimum OS X deployment version")

project(Vulkan-ValidationLayers)
# set (CMAKE_VERBOSE_MAKEFILE 1)

# The API_NAME allows renaming builds to avoid conflicts with installed SDKs. The MAJOR number of the version we're building, used
# in naming <api-name>-<major>.dll (and other files).
set(API_NAME "Vulkan" CACHE STRING "API name to use when building")
set(MAJOR "1")
string(TOLOWER ${API_NAME} API_LOWERCASE)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(PythonInterp 3 REQUIRED)

find_package(VulkanHeaders)

option(USE_CCACHE "Use ccache" OFF)
if(USE_CCACHE)
    find_program(CCACHE_FOUND ccache)
    if(CCACHE_FOUND)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    endif(CCACHE_FOUND)
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(GNUInstallDirs)
# Set a better default install location for Windows only if the user did not provide one.
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT AND WIN32)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "default install path" FORCE)
endif()

if(APPLE)
    # CMake versions 3 or later need CMAKE_MACOSX_RPATH defined. This avoids the CMP0042 policy message.
    set(CMAKE_MACOSX_RPATH 1)
    # The "install" target for MacOS fixes up bundles in place.
    set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR})
endif()

# Enable IDE GUI folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
# "Helper" targets that don't have interesting source code should set their FOLDER property to this
set(LAYERS_HELPER_FOLDER "Helper Targets")

if(CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(
        FALLBACK_CONFIG_DIRS "/etc/xdg"
        CACHE
            STRING
            "Search path to use when XDG_CONFIG_DIRS is unset or empty or the current process is SUID/SGID. Default is freedesktop compliant."
        )
    set(
        FALLBACK_DATA_DIRS "/usr/local/share:/usr/share"
        CACHE
            STRING
            "Search path to use when XDG_DATA_DIRS is unset or empty or the current process is SUID/SGID. Default is freedesktop compliant."
        )
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    include(FindPkgConfig)
    option(BUILD_WSI_XCB_SUPPORT "Build XCB WSI support" ON)
    option(BUILD_WSI_XLIB_SUPPORT "Build Xlib WSI support" ON)
    option(BUILD_WSI_WAYLAND_SUPPORT "Build Wayland WSI support" ON)
    option(BUILD_WSI_MIR_SUPPORT "Build Mir WSI support" OFF)
    set(DEMOS_WSI_SELECTION "XCB" CACHE STRING "Select WSI target for demos (XCB, XLIB, WAYLAND, MIR, DISPLAY)")

    if(BUILD_WSI_XCB_SUPPORT)
        find_package(XCB REQUIRED)
    endif()

    if(BUILD_WSI_XLIB_SUPPORT)
        find_package(X11 REQUIRED)
    endif()

    if(BUILD_WSI_WAYLAND_SUPPORT)
        find_package(Wayland REQUIRED)
        include_directories(${WAYLAND_CLIENT_INCLUDE_DIR})
    endif()

    if(BUILD_WSI_MIR_SUPPORT)
        find_package(Mir REQUIRED)
    endif()
endif()

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_C_COMPILER_ID MATCHES "Clang")
    set(COMMON_COMPILE_FLAGS "-Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers")
    set(COMMON_COMPILE_FLAGS "${COMMON_COMPILE_FLAGS} -fno-strict-aliasing -fno-builtin-memcmp")

    # For GCC version 7.1 or greater, we need to disable the implicit fallthrough warning since there's no consistent way to satisfy
    # all compilers until they all accept the C++17 standard.
    if(CMAKE_COMPILER_IS_GNUCC AND NOT (CMAKE_CXX_COMPILER_VERSION LESS 7.1))
        set(COMMON_COMPILE_FLAGS "${COMMON_COMPILE_FLAGS} -Wimplicit-fallthrough=0")
    endif()

    if(APPLE)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMMON_COMPILE_FLAGS}")
    else()
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 ${COMMON_COMPILE_FLAGS}")
    endif()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_COMPILE_FLAGS} -std=c++11 -fno-rtti")
    if(UNIX)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
    endif()
endif()

if(WIN32)
    # Treat warnings as errors
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/WX>")
    # Disable RTTI
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/GR->")
    # Warn about nested declarations
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/w34456>")
    # Warn about potentially uninitialized variables
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/w34701>")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/w34703>")
    # Warn about different indirection types.
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/w34057>")
    # Warn about signed/unsigned mismatch.
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/w34245>")
endif()

if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/external/googletest)
    option(BUILD_TESTS "Build tests" ON)
else()
    option(BUILD_TESTS "Build tests" OFF)
endif()
option(INSTALL_TESTS "Install tests" OFF)
option(BUILD_LAYERS "Build layers" ON)
option(BUILD_LAYER_SUPPORT_FILES "Generate layer files" OFF) # For generating files when not building layers

set(GLSLANG_INSTALL_DIR "GLSLANG-NOTFOUND" CACHE PATH "Absolute path to a glslang install directory")
if(NOT GLSLANG_INSTALL_DIR AND NOT DEFINED ENV{GLSLANG_INSTALL_DIR})
    message(FATAL_ERROR "Must define location of glslang binaries -- see BUILD.md")
endif()

# Cmake command line option overrides environment variable
if(NOT GLSLANG_INSTALL_DIR)
    set(GLSLANG_INSTALL_DIR $ENV{GLSLANG_INSTALL_DIR})
endif()
message(STATUS "Using glslang install located at ${GLSLANG_INSTALL_DIR}")
set(SPIRV_TOOLS_BINARY_ROOT "${GLSLANG_INSTALL_DIR}/lib"
    CACHE PATH "User defined path to the SPIRV-Tools binaries for this project")
set(SPIRV_TOOLS_OPT_BINARY_ROOT "${GLSLANG_INSTALL_DIR}/lib"
    CACHE PATH "User defined path to the SPIRV-Tools-opt binaries for this project")
set(GLSLANG_SPIRV_INCLUDE_DIR "${GLSLANG_INSTALL_DIR}/include" CACHE PATH "Path to glslang spirv headers")
set(SPIRV_TOOLS_INCLUDE_DIR "${GLSLANG_INSTALL_DIR}/include" CACHE PATH "Path to spirv tools headers")
set(GLSLANG_SEARCH_PATH "${GLSLANG_INSTALL_DIR}/lib")
set(GLSLANG_DEBUG_SEARCH_PATH "${GLSLANG_INSTALL_DIR}/lib")
set(SPIRV_TOOLS_SEARCH_PATH "${GLSLANG_INSTALL_DIR}/lib")
set(SPIRV_TOOLS_DEBUG_SEARCH_PATH "${GLSLANG_INSTALL_DIR}/lib")
set(SPIRV_TOOLS_OPT_SEARCH_PATH "${GLSLANG_INSTALL_DIR}/lib")
set(SPIRV_TOOLS_OPT_DEBUG_SEARCH_PATH "${GLSLANG_INSTALL_DIR}/lib")

find_library(GLSLANG_LIB NAMES glslang HINTS ${GLSLANG_SEARCH_PATH})

find_library(OGLCompiler_LIB NAMES OGLCompiler HINTS ${GLSLANG_SEARCH_PATH})

find_library(OSDependent_LIB NAMES OSDependent HINTS ${GLSLANG_SEARCH_PATH})

find_library(HLSL_LIB NAMES HLSL HINTS ${GLSLANG_SEARCH_PATH})

find_library(SPIRV_LIB NAMES SPIRV HINTS ${GLSLANG_SEARCH_PATH})

find_library(SPIRV_REMAPPER_LIB NAMES SPVRemapper HINTS ${GLSLANG_SEARCH_PATH})

find_library(SPIRV_TOOLS_LIB NAMES SPIRV-Tools HINTS ${SPIRV_TOOLS_SEARCH_PATH})

find_library(SPIRV_TOOLS_OPT_LIB NAMES SPIRV-Tools-opt HINTS ${SPIRV_TOOLS_OPT_SEARCH_PATH})

if(WIN32)
    add_library(glslang STATIC IMPORTED)
    add_library(OGLCompiler STATIC IMPORTED)
    add_library(OSDependent STATIC IMPORTED)
    add_library(HLSL STATIC IMPORTED)
    add_library(SPIRV STATIC IMPORTED)
    add_library(SPVRemapper STATIC IMPORTED)
    add_library(Loader STATIC IMPORTED)
    add_library(SPIRV-Tools-opt STATIC IMPORTED)
    add_library(SPIRV-Tools STATIC IMPORTED)

    find_library(GLSLANG_DLIB NAMES glslangd HINTS ${GLSLANG_DEBUG_SEARCH_PATH})
    find_library(OGLCompiler_DLIB NAMES OGLCompilerd HINTS ${GLSLANG_DEBUG_SEARCH_PATH})
    find_library(OSDependent_DLIB NAMES OSDependentd HINTS ${GLSLANG_DEBUG_SEARCH_PATH})
    find_library(HLSL_DLIB NAMES HLSLd HINTS ${GLSLANG_DEBUG_SEARCH_PATH})
    find_library(SPIRV_DLIB NAMES SPIRVd HINTS ${GLSLANG_DEBUG_SEARCH_PATH})
    find_library(SPIRV_REMAPPER_DLIB NAMES SPVRemapperd HINTS ${GLSLANG_DEBUG_SEARCH_PATH})
    find_library(SPIRV_TOOLS_DLIB NAMES SPIRV-Toolsd HINTS ${SPIRV_TOOLS_DEBUG_SEARCH_PATH})
    find_library(SPIRV_TOOLS_OPT_DLIB NAMES SPIRV-Tools-optd HINTS ${SPIRV_TOOLS_OPT_DEBUG_SEARCH_PATH})
    set_target_properties(glslang PROPERTIES IMPORTED_LOCATION "${GLSLANG_LIB}" IMPORTED_LOCATION_DEBUG "${GLSLANG_DLIB}")
    set_target_properties(
        OGLCompiler PROPERTIES IMPORTED_LOCATION "${OGLCompiler_LIB}" IMPORTED_LOCATION_DEBUG "${OGLCompiler_DLIB}")
    set_target_properties(
        OSDependent PROPERTIES IMPORTED_LOCATION "${OSDependent_LIB}" IMPORTED_LOCATION_DEBUG "${OSDependent_DLIB}")
    set_target_properties(HLSL PROPERTIES IMPORTED_LOCATION "${HLSL_LIB}" IMPORTED_LOCATION_DEBUG "${HLSL_DLIB}")
    set_target_properties(SPIRV PROPERTIES IMPORTED_LOCATION "${SPIRV_LIB}" IMPORTED_LOCATION_DEBUG "${SPIRV_DLIB}")
    set_target_properties(
        SPVRemapper PROPERTIES IMPORTED_LOCATION "${SPIRV_REMAPPER_LIB}" IMPORTED_LOCATION_DEBUG "${SPIRV_REMAPPER_DLIB}")
    set_target_properties(
        SPIRV-Tools PROPERTIES IMPORTED_LOCATION "${SPIRV_TOOLS_LIB}" IMPORTED_LOCATION_DEBUG "${SPIRV_TOOLS_DLIB}")
    set_target_properties(
        SPIRV-Tools-opt PROPERTIES IMPORTED_LOCATION "${SPIRV_TOOLS_OPT_LIB}" IMPORTED_LOCATION_DEBUG "${SPIRV_TOOLS_OPT_DLIB}")
endif()

if(WIN32)
    set(SPIRV_TOOLS_LIBRARIES SPIRV-Tools-opt SPIRV-Tools)
    set(GLSLANG_LIBRARIES glslang OGLCompiler OSDependent HLSL SPIRV SPVRemapper ${SPIRV_TOOLS_LIBRARIES})
else()
    set(SPIRV_TOOLS_LIBRARIES ${SPIRV_TOOLS_OPT_LIB} ${SPIRV_TOOLS_LIB})
    set(GLSLANG_LIBRARIES
        ${GLSLANG_LIB}
        ${OGLCompiler_LIB}
        ${OSDependent_LIB}
        ${HLSL_LIB}
        ${SPIRV_LIB}
        ${SPIRV_REMAPPER_LIB}
        ${SPIRV_TOOLS_LIBRARIES})
endif()

set(PYTHON_CMD ${PYTHON_EXECUTABLE})

if(NOT WIN32)
    add_definitions(-DFALLBACK_CONFIG_DIRS="${FALLBACK_CONFIG_DIRS}")
    add_definitions(-DFALLBACK_DATA_DIRS="${FALLBACK_DATA_DIRS}")
    add_definitions(-DSYSCONFDIR="${CMAKE_INSTALL_FULL_SYSCONFDIR}")

    # Make sure /etc is searched by the loader
    if(NOT (CMAKE_INSTALL_FULL_SYSCONFDIR STREQUAL "/etc"))
        add_definitions(-DEXTRASYSCONFDIR="/etc")
    endif()
endif()

# Generate dependent helper files
set(SCRIPTS_DIR "${PROJECT_SOURCE_DIR}/scripts")
# Define macro used for building vkxml generated files
macro(run_vk_xml_generate dependency output)
    add_custom_command(OUTPUT ${output}
                       COMMAND ${PYTHON_CMD} ${SCRIPTS_DIR}/lvl_genvk.py -registry ${VulkanRegistry_DIR}/vk.xml -scripts
                               ${VulkanRegistry_DIR} ${output}
                       DEPENDS ${VulkanRegistry_DIR}/vk.xml
                               ${VulkanRegistry_DIR}/generator.py
                               ${SCRIPTS_DIR}/${dependency}
                               ${SCRIPTS_DIR}/lvl_genvk.py
                               ${VulkanRegistry_DIR}/reg.py)
endmacro()
add_custom_target(generate_helper_files
                  DEPENDS vk_enum_string_helper.h
                          vk_safe_struct.h
                          vk_safe_struct.cpp
                          vk_object_types.h
                          vk_layer_dispatch_table.h
                          vk_dispatch_table_helper.h
                          vk_extension_helper.h
                          vk_typemap_helper.h)
set_target_properties(generate_helper_files PROPERTIES FOLDER ${LAYERS_HELPER_FOLDER})
# Rules to build generated helper files
run_vk_xml_generate(loader_extension_generator.py vk_layer_dispatch_table.h)
run_vk_xml_generate(dispatch_table_helper_generator.py vk_dispatch_table_helper.h)
run_vk_xml_generate(helper_file_generator.py vk_safe_struct.h)
run_vk_xml_generate(helper_file_generator.py vk_safe_struct.cpp)
run_vk_xml_generate(helper_file_generator.py vk_enum_string_helper.h)
run_vk_xml_generate(helper_file_generator.py vk_object_types.h)
run_vk_xml_generate(helper_file_generator.py vk_extension_helper.h)
run_vk_xml_generate(helper_file_generator.py vk_typemap_helper.h)

# ~~~
# Layer Utils Library.
# For Windows, we use a static lib because the Windows loader has a fairly restrictive loader search
# path that can't be easily modified to point it to the same directory that contains the layers.
# TODO: This should not be a library -- in future, include files directly in layers.
# ~~~
set(VALIDATION_LAYER_UTILS_SOURCES
    layers/vk_layer_config.cpp
    layers/vk_layer_extension_utils.cpp
    layers/vk_layer_utils.cpp
    layers/vk_format_utils.cpp)
if(WIN32)
    add_library(VkLayer_utils STATIC ${VALIDATION_LAYER_UTILS_SOURCES})
    target_compile_definitions(VkLayer_utils PUBLIC _CRT_SECURE_NO_WARNINGS)
else()
    add_library(VkLayer_utils STATIC ${VALIDATION_LAYER_UTILS_SOURCES})
endif()
install(TARGETS VkLayer_utils DESTINATION ${CMAKE_INSTALL_LIBDIR})
set_target_properties(VkLayer_utils PROPERTIES LINKER_LANGUAGE CXX)
add_dependencies(VkLayer_utils generate_helper_files)
target_include_directories(VkLayer_utils
                           PUBLIC
                           ${VulkanHeaders_INCLUDE_DIR}
                           ${CMAKE_CURRENT_BINARY_DIR}
                           ${CMAKE_CURRENT_BINARY_DIR}/layers
                           ${PROJECT_BINARY_DIR})

# uninstall target
if(NOT TARGET uninstall)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
                   IMMEDIATE
                   @ONLY)
    add_custom_target(uninstall COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
    set_target_properties(uninstall PROPERTIES FOLDER ${LAYERS_HELPER_FOLDER})
endif()

add_definitions(-DAPI_NAME="${API_NAME}")

add_subdirectory(external)
if(BUILD_TESTS)
    add_subdirectory(tests)
endif()

if(BUILD_LAYERS OR BUILD_LAYER_SUPPORT_FILES)
    add_subdirectory(layers)
endif()
