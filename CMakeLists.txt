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

# CMake project initialization ---------------------------------------------------------------------------------------------------
# This section contains pre-project() initialization, and ends with the project() command.

cmake_minimum_required(VERSION 3.10.2)

# Apple: Must be set before enable_language() or project() as it may influence configuration of the toolchain and flags.
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.12" CACHE STRING "Minimum OS X deployment version")

project(Vulkan-ValidationLayers)

enable_testing()

# User-interface declarations ----------------------------------------------------------------------------------------------------
# This section contains variables that affect development GUIs (e.g. CMake GUI and IDEs), such as option(), folders, and variables
# with the CACHE property.

# API_NAME allows renaming builds to avoid conflicts with installed SDKs.  It is referenced by layers/vk_loader_platform.h
set(API_NAME "Vulkan" CACHE STRING "API name to use when building")
string(TOLOWER ${API_NAME} API_LOWERCASE)
add_definitions(-DAPI_NAME="${API_NAME}")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(PythonInterp 3 QUIET)

if (TARGET Vulkan::Headers)
    message(STATUS "Using Vulkan headers from Vulkan::Headers target")
    get_target_property(VulkanHeaders_INCLUDE_DIRS Vulkan::Headers INTERFACE_INCLUDE_DIRECTORIES)
    get_target_property(VulkanRegistry_DIR Vulkan::Registry INTERFACE_INCLUDE_DIRECTORIES)
else()
    find_package(VulkanHeaders REQUIRED)

    # xxxnsubtil: this should eventually be replaced by exported targets
    add_library(Vulkan-Headers INTERFACE)
    target_include_directories(Vulkan-Headers INTERFACE ${VulkanHeaders_INCLUDE_DIRS})
    add_library(Vulkan::Headers ALIAS Vulkan-Headers)
endif()

option(USE_CCACHE "Use ccache" OFF)
if(USE_CCACHE)
    find_program(CCACHE_FOUND ccache)
    if(CCACHE_FOUND)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    endif()
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(GNUInstallDirs)

if(WIN32 AND CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    # Windows: if install locations not set by user, set install prefix to "<build_dir>\install".
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "default install path" FORCE)
endif()

if(APPLE)
    # CMake versions 3 or later need CMAKE_MACOSX_RPATH defined. This avoids the CMP0042 policy message.
    set(CMAKE_MACOSX_RPATH 1)
endif()

# Enable IDE GUI folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
# "Helper" targets that don't have interesting source code should set their FOLDER property to this
set(LAYERS_HELPER_FOLDER "Helper Targets")

# Options for Linux only
if(UNIX AND NOT APPLE) # i.e. Linux
    include(FindPkgConfig)
    option(BUILD_WSI_XCB_SUPPORT "Build XCB WSI support" ON)
    option(BUILD_WSI_XLIB_SUPPORT "Build Xlib WSI support" ON)
    option(BUILD_WSI_WAYLAND_SUPPORT "Build Wayland WSI support" ON)
    set(DEMOS_WSI_SELECTION "XCB" CACHE STRING "Select WSI target for demos (XCB, XLIB, WAYLAND, DISPLAY)")

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
endif()

# Platform-specific compiler switches
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_C_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wall
                        -Wextra
                        -Wno-unused-parameter
                        -Wno-missing-field-initializers
                        -fno-strict-aliasing
                        -fno-builtin-memcmp
                        -fvisibility=hidden)

    # Treat warnings as errors for versions of GCC and Clang that are shipped on Ubuntu 18.04 or older.
    if((CMAKE_COMPILER_IS_GNUCXX AND NOT (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 7.3.0)) OR
       (("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang") AND NOT (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 3.8.0)))
        add_compile_options(-Werror)
    endif()

    set(CMAKE_C_STANDARD 99)
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")

    # For GCC version 7.1 or greater, we need to disable the implicit fallthrough warning since there's no consistent way to satisfy
    # all compilers until they all accept the C++17 standard.
    if(CMAKE_COMPILER_IS_GNUCC AND NOT (CMAKE_CXX_COMPILER_VERSION LESS 7.1))
        add_compile_options(-Wimplicit-fallthrough=0)
    endif()
elseif(MSVC)
    # Treat warnings as errors
    add_compile_options("/WX")
    # Disable RTTI
    add_compile_options("/GR-")
    # Warn about nested declarations
    add_compile_options("/w34456")
    # Warn about potentially uninitialized variables
    add_compile_options("/w34701")
    add_compile_options("/w34703")
    # Warn about different indirection types.
    add_compile_options("/w34057")
    # Warn about signed/unsigned mismatch.
    add_compile_options("/w34245")
endif()

if(TARGET gtest OR IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/external/googletest)
    option(BUILD_TESTS "Build tests" ON)
else()
    option(BUILD_TESTS "Build tests" OFF)
endif()

option(INSTALL_TESTS "Install tests" OFF)
option(BUILD_LAYERS "Build layers" ON)
option(BUILD_LAYER_SUPPORT_FILES "Generate layer files" OFF) # For generating files when not building layers

if(BUILD_TESTS OR BUILD_LAYERS)

    set(GLSLANG_INSTALL_DIR "GLSLANG-NOTFOUND" CACHE PATH "Absolute path to a glslang install directory")
    if(NOT GLSLANG_INSTALL_DIR AND NOT DEFINED ENV{GLSLANG_INSTALL_DIR} AND NOT TARGET glslang)
        message(FATAL_ERROR "Must define location of glslang binaries -- see BUILD.md")
    endif()

    # GLSLANG_INSTALL_DIR is used as the path to all dependent projects' install dirs
    # CMake command line option overrides environment variable
    if(NOT GLSLANG_INSTALL_DIR)
        set(GLSLANG_INSTALL_DIR $ENV{GLSLANG_INSTALL_DIR})
    endif()

    if (NOT TARGET glslang)
        message(STATUS "Using glslang install located at ${GLSLANG_INSTALL_DIR}")
        set(GLSLANG_SEARCH_PATH "${GLSLANG_INSTALL_DIR}/lib")
        set(GLSLANG_DEBUG_SEARCH_PATH "${GLSLANG_INSTALL_DIR}/lib")
        set(GLSLANG_SPIRV_INCLUDE_DIR "${GLSLANG_INSTALL_DIR}/include" CACHE PATH "Path to glslang spirv headers")

        find_library(GLSLANG_LIB NAMES glslang HINTS ${GLSLANG_SEARCH_PATH})
        find_library(OGLCompiler_LIB NAMES OGLCompiler HINTS ${GLSLANG_SEARCH_PATH})
        find_library(OSDependent_LIB NAMES OSDependent HINTS ${GLSLANG_SEARCH_PATH})
        find_library(HLSL_LIB NAMES HLSL HINTS ${GLSLANG_SEARCH_PATH})
        find_library(SPIRV_LIB NAMES SPIRV HINTS ${GLSLANG_SEARCH_PATH})
        find_library(SPIRV_REMAPPER_LIB NAMES SPVRemapper HINTS ${GLSLANG_SEARCH_PATH})

        if(WIN32)
            add_library(glslang STATIC IMPORTED)
            add_library(OGLCompiler STATIC IMPORTED)
            add_library(OSDependent STATIC IMPORTED)
            add_library(HLSL STATIC IMPORTED)
            add_library(SPIRV STATIC IMPORTED)
            add_library(SPVRemapper STATIC IMPORTED)
            add_library(Loader STATIC IMPORTED)

            find_library(GLSLANG_DLIB NAMES glslangd HINTS ${GLSLANG_DEBUG_SEARCH_PATH})
            find_library(OGLCompiler_DLIB NAMES OGLCompilerd HINTS ${GLSLANG_DEBUG_SEARCH_PATH})
            find_library(OSDependent_DLIB NAMES OSDependentd HINTS ${GLSLANG_DEBUG_SEARCH_PATH})
            find_library(HLSL_DLIB NAMES HLSLd HINTS ${GLSLANG_DEBUG_SEARCH_PATH})
            find_library(SPIRV_DLIB NAMES SPIRVd HINTS ${GLSLANG_DEBUG_SEARCH_PATH})
            find_library(SPIRV_REMAPPER_DLIB NAMES SPVRemapperd HINTS ${GLSLANG_DEBUG_SEARCH_PATH})

            set_target_properties(glslang
                                  PROPERTIES IMPORTED_LOCATION
                                             "${GLSLANG_LIB}"
                                             IMPORTED_LOCATION_DEBUG
                                             "${GLSLANG_DLIB}")
            set_target_properties(OGLCompiler
                                  PROPERTIES IMPORTED_LOCATION
                                             "${OGLCompiler_LIB}"
                                             IMPORTED_LOCATION_DEBUG
                                             "${OGLCompiler_DLIB}")
            set_target_properties(OSDependent
                                  PROPERTIES IMPORTED_LOCATION
                                             "${OSDependent_LIB}"
                                             IMPORTED_LOCATION_DEBUG
                                             "${OSDependent_DLIB}")
            set_target_properties(HLSL
                                  PROPERTIES IMPORTED_LOCATION
                                             "${HLSL_LIB}"
                                             IMPORTED_LOCATION_DEBUG
                                             "${HLSL_DLIB}")
            set_target_properties(SPIRV
                                  PROPERTIES IMPORTED_LOCATION
                                             "${SPIRV_LIB}"
                                             IMPORTED_LOCATION_DEBUG
                                             "${SPIRV_DLIB}")
            set_target_properties(SPVRemapper
                                  PROPERTIES IMPORTED_LOCATION
                                             "${SPIRV_REMAPPER_LIB}"
                                             IMPORTED_LOCATION_DEBUG
                                             "${SPIRV_REMAPPER_DLIB}")

            set(GLSLANG_LIBRARIES glslang OGLCompiler OSDependent HLSL SPIRV SPVRemapper ${SPIRV_TOOLS_LIBRARIES})
        else()
            set(GLSLANG_LIBRARIES
                ${GLSLANG_LIB}
                ${OGLCompiler_LIB}
                ${OSDependent_LIB}
                ${HLSL_LIB}
                ${SPIRV_LIB}
                ${SPIRV_REMAPPER_LIB}
                ${SPIRV_TOOLS_LIBRARIES})
        endif()
    else()
        set(GLSLANG_SPIRV_INCLUDE_DIR "${glslang_SOURCE_DIR}" CACHE PATH "Path to glslang spirv headers")
        set(GLSLANG_LIBRARIES glslang SPIRV SPVRemapper)
    endif()

    # spirv-tools
    if (NOT TARGET SPIRV-Tools)
        set(SPIRV_TOOLS_BINARY_ROOT "${GLSLANG_INSTALL_DIR}/lib"
            CACHE PATH "User defined path to the SPIRV-Tools binaries for this project")
        set(SPIRV_TOOLS_OPT_BINARY_ROOT "${GLSLANG_INSTALL_DIR}/lib"
            CACHE PATH "User defined path to the SPIRV-Tools-opt binaries for this project")
        set(SPIRV_TOOLS_INCLUDE_DIR "${GLSLANG_INSTALL_DIR}/include" CACHE PATH "Path to spirv tools headers")
        set(SPIRV_TOOLS_SEARCH_PATH "${GLSLANG_INSTALL_DIR}/lib")
        set(SPIRV_TOOLS_DEBUG_SEARCH_PATH "${GLSLANG_INSTALL_DIR}/lib")
        set(SPIRV_TOOLS_OPT_SEARCH_PATH "${GLSLANG_INSTALL_DIR}/lib")
        set(SPIRV_TOOLS_OPT_DEBUG_SEARCH_PATH "${GLSLANG_INSTALL_DIR}/lib")

        find_library(SPIRV_TOOLS_LIB NAMES SPIRV-Tools HINTS ${SPIRV_TOOLS_SEARCH_PATH})
        find_library(SPIRV_TOOLS_OPT_LIB NAMES SPIRV-Tools-opt HINTS ${SPIRV_TOOLS_OPT_SEARCH_PATH})

        if(WIN32)
            add_library(SPIRV-Tools-opt STATIC IMPORTED)
            add_library(SPIRV-Tools STATIC IMPORTED)

            find_library(SPIRV_TOOLS_DLIB NAMES SPIRV-Toolsd HINTS ${SPIRV_TOOLS_DEBUG_SEARCH_PATH})
            find_library(SPIRV_TOOLS_OPT_DLIB NAMES SPIRV-Tools-optd HINTS ${SPIRV_TOOLS_OPT_DEBUG_SEARCH_PATH})

            set_target_properties(SPIRV-Tools
                                  PROPERTIES IMPORTED_LOCATION
                                             "${SPIRV_TOOLS_LIB}"
                                             IMPORTED_LOCATION_DEBUG
                                             "${SPIRV_TOOLS_DLIB}")
            set_target_properties(SPIRV-Tools-opt
                                  PROPERTIES IMPORTED_LOCATION
                                             "${SPIRV_TOOLS_OPT_LIB}"
                                             IMPORTED_LOCATION_DEBUG
                                             "${SPIRV_TOOLS_OPT_DLIB}")

            set(SPIRV_TOOLS_LIBRARIES SPIRV-Tools-opt SPIRV-Tools)
        else()
            set(SPIRV_TOOLS_LIBRARIES ${SPIRV_TOOLS_OPT_LIB} ${SPIRV_TOOLS_LIB})
        endif()
    else()
        set(SPIRV_TOOLS_LIBRARIES SPIRV-Tools SPIRV-Tools-opt)
        set(SPIRV_TOOLS_INCLUDE_DIR "${spirv-tools_SOURCE_DIR}/include" CACHE PATH "Path to spirv tools headers")
    endif()

    set(GLSLANG_LIBRARIES ${GLSLANG_LIBRARIES} ${SPIRV_TOOLS_LIBRARIES})
endif()

# Generate dependent helper files ------------------------------------------------------------------------------------------------

set(SCRIPTS_DIR "${PROJECT_SOURCE_DIR}/scripts")

# VkLayer_utils library ----------------------------------------------------------------------------------------------------------
# For Windows, we use a static lib because the Windows loader has a fairly restrictive loader search path that can't be easily
# modified to point it to the same directory that contains the layers. TODO: This should not be a library -- in future, include
# files directly in layers.

add_library(VkLayer_utils
            STATIC
            layers/vk_layer_config.cpp
            layers/vk_layer_extension_utils.cpp
            layers/vk_layer_utils.cpp
            layers/vk_format_utils.cpp)
target_link_libraries(VkLayer_utils PUBLIC Vulkan::Headers)
if(WIN32)
    target_compile_definitions(VkLayer_utils PUBLIC _CRT_SECURE_NO_WARNINGS)
endif()
install(TARGETS VkLayer_utils DESTINATION ${CMAKE_INSTALL_LIBDIR})
set_target_properties(VkLayer_utils PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(VkLayer_utils
                           PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/layers
                                  ${CMAKE_CURRENT_SOURCE_DIR}/layers/generated
                                  ${CMAKE_CURRENT_BINARY_DIR}
                                  ${CMAKE_CURRENT_BINARY_DIR}/layers
                                  ${PROJECT_BINARY_DIR}
                                  ${VulkanHeaders_INCLUDE_DIR})

# uninstall target ---------------------------------------------------------------------------------------------------------------
if(NOT TARGET uninstall)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
                   "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
                   IMMEDIATE
                   @ONLY)
    add_custom_target(uninstall COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
    set_target_properties(uninstall PROPERTIES FOLDER ${LAYERS_HELPER_FOLDER})
endif()

# Fetch header version from vulkan_core.h ----------------------------------------------------------------------------------------
file(STRINGS "${VulkanHeaders_INCLUDE_DIRS}/vulkan/vulkan_core.h" lines REGEX "^#define VK_HEADER_VERSION [0-9]+")
list(LENGTH lines len)
if(${len} EQUAL 1)
    string(REGEX MATCHALL
                 "[0-9]+"
                 vk_header_version
                 ${lines})
else()
    message(FATAL_ERROR "Unable to fetch version from vulkan_core.h")
endif()

# Optional codegen target --------------------------------------------------------------------------------------------------------
if(PYTHONINTERP_FOUND)
    add_custom_target(VulkanVL_generated_source
                      COMMAND ${PYTHON_EXECUTABLE} ${PROJECT_SOURCE_DIR}/scripts/generate_source.py
                              ${VulkanRegistry_DIR} --incremental
                      WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/layers/generated
                      )
else()
    message("WARNING: VulkanVL_generated_source target requires python 3")
endif()

# Add subprojects ----------------------------------------------------------------------------------------------------------------

add_subdirectory(external)
if(BUILD_TESTS)
    add_subdirectory(tests)
endif()

if(BUILD_LAYERS OR BUILD_LAYER_SUPPORT_FILES)
    add_subdirectory(layers)
endif()
