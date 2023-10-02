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

project(VVL LANGUAGES CXX)

# This variable enables downstream users to customize the target API
# variant (e.g. Vulkan SC)
set(API_TYPE "vulkan")

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

add_compile_definitions(VK_ENABLE_BETA_EXTENSIONS)
if(WIN32)
    add_compile_definitions(VK_USE_PLATFORM_WIN32_KHR)

    # Allow usage of unsafe CRT functions and minimize what Windows.h leaks
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS NOMINMAX WIN32_LEAN_AND_MEAN)
elseif(ANDROID)
    add_compile_definitions(VK_USE_PLATFORM_ANDROID_KHR)
elseif(APPLE)
    add_compile_definitions(VK_USE_PLATFORM_MACOS_MVK VK_USE_PLATFORM_METAL_EXT)
else()
    option(BUILD_WSI_XCB_SUPPORT "Build XCB WSI support" ON)
    option(BUILD_WSI_XLIB_SUPPORT "Build Xlib WSI support" ON)
    option(BUILD_WSI_WAYLAND_SUPPORT "Build Wayland WSI support" ON)

    find_package(PkgConfig REQUIRED QUIET)

    if(BUILD_WSI_XCB_SUPPORT)
        pkg_check_modules(XCB REQUIRED QUIET IMPORTED_TARGET xcb)
        add_compile_definitions(VK_USE_PLATFORM_XCB_KHR)
    endif()

    if(BUILD_WSI_XLIB_SUPPORT)
        pkg_check_modules(X11 REQUIRED QUIET IMPORTED_TARGET x11)
        add_compile_definitions(VK_USE_PLATFORM_XLIB_KHR VK_USE_PLATFORM_XLIB_XRANDR_EXT)
    endif()

    if(BUILD_WSI_WAYLAND_SUPPORT)
        pkg_check_modules(WAYlAND_CLIENT QUIET REQUIRED IMPORTED_TARGET wayland-client)
        add_compile_definitions(VK_USE_PLATFORM_WAYLAND_KHR)
    endif()
endif()

find_package(VulkanHeaders CONFIG QUIET)

find_package(VulkanUtilityLibraries CONFIG QUIET)

find_package(SPIRV-Headers CONFIG QUIET)

find_package(SPIRV-Tools-opt CONFIG QUIET)

find_package(SPIRV-Tools-link CONFIG QUIET)

# NOTE: Our custom code generation target isn't desirable for system package managers or add_subdirectory users.
# So this target needs to be off by default to avoid obtuse build errors or patches.
option(VVL_CODEGEN "Enable vulkan validation layer code generation")
if (VVL_CODEGEN)
    find_package(Python3 REQUIRED)
    add_custom_target(vvl_codegen
        COMMAND Python3::Interpreter "${VVL_SOURCE_DIR}/scripts/generate_source.py"
            "${VULKAN_HEADERS_INSTALL_DIR}/${CMAKE_INSTALL_DATADIR}/vulkan/registry"
            "${SPIRV_HEADERS_INSTALL_DIR}/include/spirv/unified1"
            --incremental --generated-version ${VulkanHeaders_VERSION} --api ${API_TYPE}
        WORKING_DIRECTORY ${VVL_SOURCE_DIR}/layers/${API_TYPE}/generated
    )
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
if (VVL_ENABLE_ASAN)
    if (MSVC)
        add_compile_options(/fsanitize=address)
    else()
        add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address)
    endif()
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
    add_compile_options(
        /W4
        /we5038 # Enable warning about MIL ordering in constructors
    )

    # Enforce stricter ISO C++
    add_compile_options(/permissive-)

    # PDBs aren't generated on Release builds by default.
    add_compile_options("$<$<CONFIG:Release>:/Zi>")
    add_link_options("$<$<CONFIG:Release>:/DEBUG:FULL>")

    # Enable /LTCG (Link-time code generation)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
    # Remove unreferenced code/data.
    add_link_options("$<$<CONFIG:Release>:/OPT:REF>")
    # Merge duplicate data/functions
    add_link_options("$<$<CONFIG:Release>:/OPT:ICF>")

    add_compile_options($<$<BOOL:${MSVC_IDE}>:/MP>) # Speed up Visual Studio builds
endif()

if (BUILD_LAYER_SUPPORT_FILES)
    message(WARNING "BUILD_LAYER_SUPPORT_FILES is deprecated! See https://github.com/KhronosGroup/Vulkan-Utility-Libraries/issues/18")
endif()

add_subdirectory(layers)

option(BUILD_TESTS "Build the tests")
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
