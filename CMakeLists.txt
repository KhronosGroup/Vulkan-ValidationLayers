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

project(VVL LANGUAGES CXX C)

add_subdirectory(scripts)

set(VVL_CPP_STANDARD 17 CACHE STRING "Set the C++ standard to build against.")
set(CMAKE_CXX_STANDARD ${VVL_CPP_STANDARD})
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_VISIBILITY_PRESET "hidden")
set(CMAKE_C_VISIBILITY_PRESET "hidden")
set(CMAKE_VISIBILITY_INLINES_HIDDEN "YES")

include(GNUInstallDirs)

if(WIN32 AND CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    # Windows: if install locations not set by user, set install prefix to "<build_dir>\install".
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "default install path" FORCE)
endif()

# Enable GUI folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
# "Helper" targets that don't have interesting source code should set their FOLDER property to this
set(LAYERS_HELPER_FOLDER "Helper Targets")

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
        target_compile_definitions(Vulkan::Headers INTERFACE VK_USE_PLATFORM_WAYLAND_KHR)
    endif()
endif()

if (ANNOTATED_SPEC_LINK)
    message("ANNOTATED_SPEC_LINK is ${ANNOTATED_SPEC_LINK}")
    add_compile_definitions(ANNOTATED_SPEC_LINK=${ANNOTATED_SPEC_LINK})
endif()

# NOTE: The idiom for open source projects is to not enable warnings as errors.
# This reduces problems for users who simply want to build the repository.
option(BUILD_WERROR "Treat compiler warnings as errors" OFF)
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

if(${CMAKE_C_COMPILER_ID} MATCHES "Clang")
    add_compile_options(-Wconversion
                        -Wimplicit-fallthrough

                        # TODO These warnings also get turned on with -Wconversion in some versions of clang.
                        #      Leave off until further investigation.
                        -Wno-sign-conversion
                        -Wno-shorten-64-to-32
                        -Wno-implicit-int-conversion
                        -Wno-enum-enum-conversion
                        -Wstring-conversion)

endif()
if(${CMAKE_C_COMPILER_ID} MATCHES "(GNU|Clang)")
    add_compile_options(-Wall
                        -Wextra
                        -Wno-unused-parameter
                        -Wno-missing-field-initializers
                        -fno-strict-aliasing
                        -fno-builtin-memcmp)

    set(CMAKE_C_STANDARD 99)
elseif(MSVC)
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

option(BUILD_LAYERS "Build layers" ON)
option(BUILD_LAYER_SUPPORT_FILES "Generate layer files" OFF) # For generating files when not building layers

# uninstall target ---------------------------------------------------------------------------------------------------------------
if(NOT TARGET uninstall)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
                   "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
                   IMMEDIATE
                   @ONLY)
    add_custom_target(uninstall COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
    set_target_properties(uninstall PROPERTIES FOLDER ${LAYERS_HELPER_FOLDER})
endif()

# Add subprojects ----------------------------------------------------------------------------------------------------------------
if(BUILD_LAYERS OR BUILD_LAYER_SUPPORT_FILES)
    add_subdirectory(layers)
endif()

option(BUILD_TESTS "Build the tests")
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
