# ~~~
# Copyright (c) 2014-2021 Valve Corporation
# Copyright (c) 2014-2021 LunarG, Inc.
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

cmake_minimum_required(VERSION 3.11.0)

# Apple: Must be set before enable_language() or project() as it may influence configuration of the toolchain and flags.
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.12" CACHE STRING "Minimum OS X deployment version")

project(Vulkan-ValidationLayers)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

enable_testing()

# User-interface declarations ----------------------------------------------------------------------------------------------------
# This section contains variables that affect development GUIs (e.g. CMake GUI and IDEs), such as option(), folders, and variables
# with the CACHE property.

set(VVL_CPP_STANDARD 11 CACHE STRING "Set the C++ standard to build against. The value will be passed to cmake's CXX_STANDARD property.")

option(VVL_ENABLE_ASAN "Use address sanitization (specifically -fsanitize=address)" OFF)

option(BUILD_TESTS "Build the tests" OFF)

# API_NAME allows renaming builds to avoid conflicts with installed SDKs.  It is referenced by layers/vk_loader_platform.h
set(API_NAME "Vulkan" CACHE STRING "API name to use when building")
string(TOLOWER ${API_NAME} API_LOWERCASE)
add_definitions(-DAPI_NAME="${API_NAME}")

# Enable beta Vulkan extensions
add_definitions(-DVK_ENABLE_BETA_EXTENSIONS)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(cmake/known_good.cmake)
include(cmake/fetch_content.cmake)

FetchRepo(Vulkan-Headers ${VVL_VULKAN_HEADERS_URL} ${VVL_VULKAN_HEADERS_REF})

set(SPIRV_HEADERS_SKIP_EXAMPLES OFF CACHE BOOL INTERNAL FORCE)
FetchRepo(SPIRV-Headers ${VVL_SPIRV_HEADERS_URL} ${VVL_SPIRV_HEADERS_REF})

FetchRepo(SPIRV-Tools ${VVL_SPIRV_TOOLS_URL} ${VVL_SPIRV_TOOLS_REF})

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

# Accumulate built targets for adding common options
set(VVL_COMPILED_TARGETS)

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

option(INSTALL_TESTS "Install tests" OFF)
option(BUILD_LAYERS "Build layers" ON)
option(BUILD_LAYER_SUPPORT_FILES "Generate layer files" OFF) # For generating files when not building layers
option(USE_ROBIN_HOOD_HASHING "Use robin-hood-hashing" ON)
if (USE_ROBIN_HOOD_HASHING)
    FetchRepo(robin-hood-hashing ${VVL_ROBIN_HOOD_HASHING_URL} ${VVL_ROBIN_HOOD_HASHING_REF})
endif()

if(BUILD_TESTS)
    set(BUILD_GTEST ON CACHE BOOL "Builds the googletest subproject")
    set(BUILD_GMOCK OFF CACHE BOOL "Builds the googlemock subproject")
    set(gtest_force_shared_crt ON CACHE BOOL "Link gtest runtimes dynamically")

    set(build_shared_libs_tmp_ ${BUILD_SHARED_LIBS})
    if (NOT ${build_shared_libs_tmp_})
        set(build_shared_libs_tmp_ OFF)
    endif()
    set(BUILD_SHARED_LIBS ON CACHE INTERNAL BOOL FORCE)
    FetchRepo(googletest ${VVL_GOOGLETEST_URL} ${VVL_GOOGLETEST_REF})
    set(BUILD_SHARED_LIBS ${build_shared_libs_tmp_} CACHE INTERNAL BOOL FORCE)

    set(ENABLE_CTEST OFF CACHE BOOL INTERNAL FORCE)
    FetchRepo(glslang ${VVL_GLSLANG_URL} ${VVL_GLSLANG_REF})
endif()

if(BUILD_TESTS OR BUILD_LAYERS)
    set(SPIRV_TOOLS_LIBRARIES SPIRV-Tools SPIRV-Tools-opt)
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
list(APPEND VVL_COMPILED_TARGETS VkLayer_utils)
target_link_libraries(VkLayer_utils PUBLIC Vulkan::Headers)
set_target_properties(VkLayer_utils PROPERTIES CXX_STANDARD ${VVL_CPP_STANDARD})
if (VVL_ENABLE_ASAN)
    target_compile_options(VkLayer_utils PRIVATE -fsanitize=address)
    # NOTE: Use target_link_options when cmake 3.13 is available on CI
    target_link_libraries(VkLayer_utils PRIVATE "-fsanitize=address")
endif()
if (UPDATE_DEPS)
    add_dependencies(VkLayer_utils vvl_update_deps)
endif()
if(WIN32)
    target_compile_definitions(VkLayer_utils PUBLIC _CRT_SECURE_NO_WARNINGS NOMINMAX)
    if(MINGW)
        target_compile_definitions(VkLayer_utils PUBLIC "_WIN32_WINNT=0x0600")
    endif()
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

if (USE_ROBIN_HOOD_HASHING)
    target_link_libraries(VkLayer_utils PUBLIC robin_hood::robin_hood)
    target_compile_definitions(VkLayer_utils PUBLIC USE_ROBIN_HOOD_HASHING)
endif()

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
file(STRINGS "${vulkan-headers_SOURCE_DIR}/include/vulkan/vulkan_core.h" lines REGEX "^#define VK_HEADER_VERSION [0-9]+")
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

if(BUILD_TESTS)
    # Attempt to enable googletest if available.

    if(NOT (TARGET gtest_main))
        message(SEND_ERROR "Could not find googletest directory. Be sure to run update_deps.py with the --tests option to download the appropriate version of googletest")
        set(BUILD_TESTS OFF)
    endif()

    if (WIN32)
        set_target_properties(gtest PROPERTIES FOLDER ${LAYERS_HELPER_FOLDER})
        set_target_properties(gtest_main PROPERTIES FOLDER ${LAYERS_HELPER_FOLDER})
    endif()

    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set_target_properties(gtest PROPERTIES COMPILE_OPTIONS "-w")
        set_target_properties(gtest_main PROPERTIES COMPILE_OPTIONS "-w")
    endif()

    if (BUILD_TESTS)
        add_subdirectory(tests ${CMAKE_BINARY_DIR}/tests)
    endif()
endif()

if(BUILD_LAYERS OR BUILD_LAYER_SUPPORT_FILES)
    add_subdirectory(layers)
endif()

foreach (_target ${VVL_COMPILED_TARGETS})
    # Platform-specific compiler switches
    option(BUILD_WERROR "Treat compiler warnings as errors" ON)
    if(MAKE_C_COMPILER_ID MATCHES "Clang")
        target_compile_options(${_target} PRIVATE -Wconversion

                            # TODO These warnings also get turned on with -Wconversion in some versions of clang.
                            #      Leave off until further investigation.
                            -Wno-sign-conversion
                            -Wno-shorten-64-to-32
                            -Wno-string-conversion
                            -Wno-implicit-int-conversion
                            -Wno-enum-enum-conversion)

    endif()
    if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_C_COMPILER_ID MATCHES "Clang")
        target_compile_options(${_target} PRIVATE -Wall
                            -Wextra
                            -Wno-unused-parameter
                            -Wno-missing-field-initializers
                            -fno-strict-aliasing
                            -fno-builtin-memcmp
                            -fvisibility=hidden)

        # Treat warnings as errors for versions of GCC and c++11-compliant Clang versions that are shipped on Ubuntu 18.04 or older.
        if(BUILD_WERROR)
            if ((CMAKE_COMPILER_IS_GNUCXX AND NOT (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 7.3.0)) OR
               (("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang") AND (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 6.0.0)))
               target_compile_options(${_target} PRIVATE -Werror)
            endif()
        endif()

        set(CMAKE_C_STANDARD 99)

        # For GCC version 7.1 or greater, we need to disable the implicit fallthrough warning since there's no consistent way to satisfy
        # all compilers until they all accept the C++17 standard.
        if(CMAKE_COMPILER_IS_GNUCC AND NOT (CMAKE_CXX_COMPILER_VERSION LESS 7.1))
            target_compile_options(${_target} PRIVATE -Wimplicit-fallthrough=0)
        endif()
    elseif(MSVC)
        if(BUILD_WERROR)
            # Treat warnings as errors
            target_compile_options(${_target} PRIVATE "/WX")
        endif()
        # Warn about nested declarations
        target_compile_options(${_target} PRIVATE "/w34456")
        # Warn about potentially uninitialized variables
        target_compile_options(${_target} PRIVATE "/w34701")
        target_compile_options(${_target} PRIVATE "/w34703")
        # Warn about different indirection types.
        target_compile_options(${_target} PRIVATE "/w34057")
        # Warn about signed/unsigned mismatch.
        target_compile_options(${_target} PRIVATE "/w34245")
    endif()

    if(MAKE_C_COMPILER_ID MATCHES "Clang")
        target_compile_options(${_target} PRIVATE -Wconversion

                            # TODO These warnings also get turned on with -Wconversion in some versions of clang.
                            #      Leave off until further investigation.
                            -Wno-sign-conversion
                            -Wno-shorten-64-to-32
                            -Wno-string-conversion
                            -Wno-implicit-int-conversion
                            -Wno-enum-enum-conversion)

    endif()
endforeach()
