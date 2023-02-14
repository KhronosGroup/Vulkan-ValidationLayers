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
cmake_minimum_required(VERSION 3.17.2)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

project(VVL LANGUAGES CXX)

add_subdirectory(scripts)

set(VVL_CPP_STANDARD 17 CACHE STRING "Set the C++ standard to build against.")
set(CMAKE_CXX_STANDARD ${VVL_CPP_STANDARD})
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_VISIBILITY_PRESET "hidden")
set(CMAKE_VISIBILITY_INLINES_HIDDEN "YES")

include(GNUInstallDirs)

set_property(GLOBAL PROPERTY USE_FOLDERS ON) # Remove when min is 3.26, see CMP0143

find_package(VulkanHeaders REQUIRED CONFIG QUIET)

target_compile_definitions(Vulkan::Headers INTERFACE VK_ENABLE_BETA_EXTENSIONS) # Enable beta Vulkan extensions
if(WIN32)
    target_compile_definitions(Vulkan::Headers INTERFACE VK_USE_PLATFORM_WIN32_KHR)
elseif(ANDROID)
    target_compile_definitions(Vulkan::Headers INTERFACE VK_USE_PLATFORM_ANDROID_KHR)
elseif(APPLE)
    target_compile_definitions(Vulkan::Headers INTERFACE VK_USE_PLATFORM_MACOS_MVK VK_USE_PLATFORM_METAL_EXT)
else()
    option(BUILD_WSI_XCB_SUPPORT "Build XCB WSI support" ON)
    option(BUILD_WSI_XLIB_SUPPORT "Build Xlib WSI support" ON)
    option(BUILD_WSI_WAYLAND_SUPPORT "Build Wayland WSI support" ON)

    find_package(PkgConfig REQUIRED QUIET) # Use PkgConfig to find Linux system libraries

    if(BUILD_WSI_XCB_SUPPORT)
        pkg_check_modules(XCB REQUIRED QUIET IMPORTED_TARGET xcb)
        target_compile_definitions(Vulkan::Headers INTERFACE VK_USE_PLATFORM_XCB_KHR)
    endif()

    if(BUILD_WSI_XLIB_SUPPORT)
        pkg_check_modules(X11 REQUIRED QUIET IMPORTED_TARGET x11)
        target_compile_definitions(Vulkan::Headers INTERFACE VK_USE_PLATFORM_XLIB_KHR VK_USE_PLATFORM_XLIB_XRANDR_EXT)
    endif()

    if(BUILD_WSI_WAYLAND_SUPPORT)
        pkg_check_modules(WAYlAND_CLIENT QUIET REQUIRED IMPORTED_TARGET wayland-client)
        target_compile_definitions(Vulkan::Headers INTERFACE VK_USE_PLATFORM_WAYLAND_KHR)
    endif()
endif()

if (ANNOTATED_SPEC_LINK)
    message("ANNOTATED_SPEC_LINK is ${ANNOTATED_SPEC_LINK}")
    add_compile_definitions(ANNOTATED_SPEC_LINK=${ANNOTATED_SPEC_LINK})
endif()

option(BUILD_WERROR "Treat compiler warnings as errors")
if (BUILD_WERROR)
    add_compile_options("$<IF:$<CXX_COMPILER_ID:MSVC>,/WX,-Werror>")
endif()

option(VVL_ENABLE_ASAN "Use address sanitization")
option(VVL_ENABLE_TSAN "Use thread sanitizer")
if (VVL_ENABLE_ASAN)
    add_compile_options(-fsanitize=address)
    add_compile_options(-fno-omit-frame-pointer) # Get nicer stack traces in error messages
    add_link_options(-fsanitize=address)
elseif (VVL_ENABLE_TSAN)
    # Thread sanitizer cannot be used in combination with address sanitizer
    add_compile_options(-fsanitize=thread)
    add_compile_options(-fno-omit-frame-pointer) # Get nicer stack traces in error messages
    add_link_options(-fsanitize=thread)
endif()

option(VVL_CLANG_TIDY "Use clang-tidy")
if (VVL_CLANG_TIDY)
    find_program(CLANG_TIDY NAMES clang-tidy)
    if(NOT CLANG_TIDY)
        message(FATAL_ERROR "clang-tidy not found!")
    endif()
    set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY}")
endif()

if(${CMAKE_CXX_COMPILER_ID} MATCHES "(GNU|Clang)")
    add_compile_options(
        -Wall
        -Wextra
        -Wpointer-arith
    )
    if(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
        add_compile_options(
            -Wconversion
            -Wimplicit-fallthrough
            -Wstring-conversion
        )
    endif()
elseif(MSVC)
    # TODO: Update to /W4
    add_compile_options("/W3")
    # Warn about nested declarations
    add_compile_options("/w34456")
    # Warn about potentially uninitialized variables
    add_compile_options("/w34701")
    add_compile_options("/w34703")
    # Warn about different indirection types.
    add_compile_options("/w34057")
    # Warn about signed/unsigned mismatch.
    add_compile_options("/w34245")

    # PDBs aren't generated on Release builds by default.
    add_compile_options("$<$<CONFIG:Release>:/Zi>")
    add_link_options("$<$<CONFIG:Release>:/DEBUG:FULL>")

    # Enable /LTCG (Link-time code generation)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
    # Remove unreferenced code/data.
    add_link_options("$<$<CONFIG:Release>:/OPT:REF>")
    # Merge duplicate data/functions
    add_link_options("$<$<CONFIG:Release>:/OPT:ICF>")

    # Allow usage of unsafe CRT functions and minimize what Windows.h leaks
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS NOMINMAX WIN32_LEAN_AND_MEAN)

    add_compile_options($<$<BOOL:${MSVC_IDE}>:/MP>) # Speed up Visual Studio builds
endif()

if(MINGW)
    add_compile_definitions(_WIN32_WINNT=0x0600) # Must be declared before including Windows.h
    add_compile_options("-Wa,-mbig-obj")
endif()

option(BUILD_LAYERS "Build VkLayer_khronos_validation" ON)
option(BUILD_LAYER_SUPPORT_FILES "Install VkLayer_utils")

if(BUILD_LAYERS OR BUILD_LAYER_SUPPORT_FILES)
    add_subdirectory(layers)
endif()

option(BUILD_TESTS "Build the tests")
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
