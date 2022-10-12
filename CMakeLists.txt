# ~~~
# Copyright (c) 2014-2022 Valve Corporation
# Copyright (c) 2014-2022 LunarG, Inc.
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
cmake_minimum_required(VERSION 3.10.2)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

project(Vulkan-ValidationLayers LANGUAGES CXX C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(VVL_CPP_STANDARD 11 CACHE STRING "Set the C++ standard to build against. The value will be passed to cmake's CXX_STANDARD property.")

option(VVL_ENABLE_ASAN "Use address sanitization (specifically -fsanitize=address)" OFF)

option(BUILD_TESTS "Build the tests" OFF)

add_definitions(-DVK_ENABLE_BETA_EXTENSIONS) # Enable beta Vulkan extensions

if (UPDATE_DEPS)
    find_package(PythonInterp 3 REQUIRED)

    if (CMAKE_GENERATOR_PLATFORM)
        set(_target_arch ${CMAKE_GENERATOR_PLATFORM})
    else()
        if (MSVC_IDE)
            message(WARNING "CMAKE_GENERATOR_PLATFORM not set. Using x64 as target architecture.")
        endif()
        set(_target_arch x64)
    endif()

    if (NOT CMAKE_BUILD_TYPE)
        message(WARNING "CMAKE_BUILD_TYPE not set. Using Debug for dependency build type")
        set(_build_type Debug)
    else()
        set(_build_type ${CMAKE_BUILD_TYPE})
    endif()

    message("********************************************************************************")
    message("* NOTE: Adding target vvl_update_deps to run as needed for updating            *")
    message("*       dependencies.                                                          *")
    message("********************************************************************************")

    set(_update_deps_arg "")
    if (NOT BUILD_TESTS)
        set(_update_deps_arg "--optional=tests")
    endif()

    if (UPDATE_DEPS_SKIP_EXISTING_INSTALL)
        set(_update_deps_arg ${_update_deps_arg} "--skip-existing-install")
    endif()

    # Add a target so that update_deps.py will run when necessary
    # NOTE: This is triggered off of the timestamps of known_good.json and helper.cmake
    add_custom_command(OUTPUT ${CMAKE_CURRENT_LIST_DIR}/external/helper.cmake
                       COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_LIST_DIR}/scripts/update_deps.py --dir ${CMAKE_CURRENT_LIST_DIR}/external --arch ${_target_arch} --config ${_build_type} --generator "${CMAKE_GENERATOR}" ${_update_deps_arg}
                       DEPENDS ${CMAKE_CURRENT_LIST_DIR}/scripts/known_good.json)

    add_custom_target(vvl_update_deps ALL DEPENDS ${CMAKE_CURRENT_LIST_DIR}/external/helper.cmake)

    # Check if update_deps.py needs to be run on first cmake run
    if (${CMAKE_CURRENT_LIST_DIR}/scripts/known_good.json IS_NEWER_THAN ${CMAKE_CURRENT_LIST_DIR}/external/helper.cmake)
        execute_process(
            COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_LIST_DIR}/scripts/update_deps.py --dir ${CMAKE_CURRENT_LIST_DIR}/external --arch ${_target_arch} --config ${_build_type} --generator "${CMAKE_GENERATOR}" ${_update_deps_arg}
            WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
            RESULT_VARIABLE _update_deps_result
        )
        if (NOT (${_update_deps_result} EQUAL 0))
            message(FATAL_ERROR "Could not run update_deps.py which is necessary to download dependencies.")
        endif()
    endif()
    include(${CMAKE_CURRENT_LIST_DIR}/external/helper.cmake)
else()
    message("********************************************************************************")
    message("* NOTE: Not adding target to run update_deps.py automatically.                 *")
    message("********************************************************************************")
    find_package(PythonInterp 3 QUIET)
endif()
# Dependencies can be installed in arbitrary locations. This is done by update_deps.py / users.
if (ROBIN_HOOD_HASHING_INSTALL_DIR)
    list(APPEND CMAKE_PREFIX_PATH ${ROBIN_HOOD_HASHING_INSTALL_DIR})
endif()
if (GLSLANG_INSTALL_DIR)
    list(APPEND CMAKE_PREFIX_PATH ${GLSLANG_INSTALL_DIR})
endif()
if (SPIRV_HEADERS_INSTALL_DIR)
    list(APPEND CMAKE_PREFIX_PATH ${SPIRV_HEADERS_INSTALL_DIR})
endif()
if (SPIRV_TOOLS_INSTALL_DIR)
    list(APPEND CMAKE_PREFIX_PATH ${SPIRV_TOOLS_INSTALL_DIR})
endif()
if (GOOGLETEST_INSTALL_DIR)
    list(APPEND CMAKE_PREFIX_PATH ${GOOGLETEST_INSTALL_DIR})
endif()

find_package(VulkanHeaders REQUIRED)
add_library(Vulkan-Headers INTERFACE)
target_include_directories(Vulkan-Headers INTERFACE ${VulkanHeaders_INCLUDE_DIRS})
add_library(Vulkan::Headers ALIAS Vulkan-Headers)

option(USE_CCACHE "Use ccache" OFF)
if(USE_CCACHE)
    find_program(CCACHE_FOUND ccache)
    if(CCACHE_FOUND)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    endif()
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_VISIBILITY_PRESET "hidden")
set(CMAKE_C_VISIBILITY_PRESET "hidden")
set(CMAKE_VISIBILITY_INLINES_HIDDEN "YES")

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
        include_directories(${XCB_INCLUDE_DIR})
    endif()

    if(BUILD_WSI_XLIB_SUPPORT)
        find_package(X11 REQUIRED)
    endif()
endif()

# Platform-specific compiler switches
option(BUILD_WERROR "Treat compiler warnings as errors" ON)
if(${CMAKE_C_COMPILER_ID} STREQUAL "Clang")
    add_compile_options(-Wconversion

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

    # Treat warnings as errors for versions of GCC and c++11-compliant Clang versions that are shipped on Ubuntu 18.04 or older.
    if(BUILD_WERROR OR
      (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS_EQUAL 7.3.0) OR
      (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 6.0.0))
        add_compile_options(-Werror)
    endif()


    set(CMAKE_C_STANDARD 99)

    # For GCC version 7.1 or greater, we need to disable the implicit fallthrough warning since there's no consistent way to satisfy
    # all compilers until they all accept the C++17 standard.
    if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 7.1)
        add_compile_options(-Wimplicit-fallthrough=0)
    endif()
elseif(MSVC)
    if(BUILD_WERROR)
        # Treat warnings as errors
        add_compile_options("/WX")
    endif()
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

option(INSTALL_TESTS "Install tests" OFF)
option(BUILD_LAYERS "Build layers" ON)
option(BUILD_LAYER_SUPPORT_FILES "Generate layer files" OFF) # For generating files when not building layers
option(USE_ROBIN_HOOD_HASHING "Use robin-hood-hashing" ON)
if (USE_ROBIN_HOOD_HASHING)
    find_package(robin_hood REQUIRED CONFIG)
endif()

if(BUILD_LAYERS OR BUILD_TESTS)
    find_package(SPIRV-Headers CONFIG QUIET)
    if(SPIRV-Headers_FOUND)
	# pefer the package if found. Note that if SPIRV_HEADERS_INSTALL_DIR points at an 'installed'
	# version of SPIRV-Headers, the package will be found.
	get_target_property(SPIRV_HEADERS_INCLUDE_DIR SPIRV-Headers::SPIRV-Headers INTERFACE_INCLUDE_DIRECTORIES)
    elseif(SPIRV_HEADERS_INCLUDE_DIR)
	# This is set by SPIRV-Tools (in parent scope!) and also some packages that include VVL with add_subdirectory
	if (NOT EXISTS "${SPIRV_HEADERS_INCLUDE_DIR}/spirv/unified1/spirv.h")
	    message(FATAL_ERROR "Cannot find SPIRV-Headers from SPIRV_HEADERS_INCLUDE_DIR: ${SPIRV_HEADERS_INCLUDE_DIR}")
        endif()
    elseif(SPIRV_HEADERS_INSTALL_DIR)
        # This is our official variable for setting SPIRV-Headers location, but pointing at the raw source of SPIRV-Headers
	if (NOT EXISTS "${SPIRV_HEADERS_INSTALL_DIR}/include/spirv/unified1/spirv.h")
	    message(FATAL_ERROR "Cannot find SPIRV-Headers from SPIRV_HEADERS_INSTALL_DIR: ${SPIRV_HEADERS_INSTALL_DIR}")
        endif()
	set(SPIRV_HEADERS_INCLUDE_DIR "${SPIRV_HEADERS_INSTALL_DIR}/include")
    endif()
    if (NOT TARGET SPIRV-Tools)
        find_package(SPIRV-Tools REQUIRED CONFIG)
	# See https://github.com/KhronosGroup/SPIRV-Tools/issues/3909 for background on this.
	# The targets available from SPIRV-Tools change depending on how SPIRV_TOOLS_BUILD_STATIC is set.
	# Try to handle all possible combinations so that we work with externally built packages.
	if (TARGET SPIRV-Tools)
	    set(SPIRV_TOOLS_TARGET "SPIRV-Tools")
	elseif(TARGET SPIRV-Tools-static)
	    set(SPIRV_TOOLS_TARGET "SPIRV-Tools-static")
	elseif(TARGET SPIRV-Tools-shared)
	    set(SPIRV_TOOLS_TARGET "SPIRV-Tools-shared")
	else()
	    message(FATAL_ERROR "Cannot determine SPIRV-Tools target name")
	endif()
    endif()
    # Thankfully SPIRV-Tools-opt has only one target name.
    if (NOT TARGET SPIRV-Tools-opt)
        find_package(SPIRV-Tools-opt REQUIRED CONFIG)
    endif()
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
            layers/generated/vk_format_utils.cpp)
target_link_libraries(VkLayer_utils PUBLIC Vulkan::Headers)
set_target_properties(VkLayer_utils PROPERTIES CXX_STANDARD ${VVL_CPP_STANDARD})
if (VVL_ENABLE_ASAN)
    target_compile_options(VkLayer_utils PRIVATE -fsanitize=address)
    # NOTE: Use target_link_options when cmake 3.13 is available on CI
    target_link_libraries(VkLayer_utils PRIVATE "-fsanitize=address")
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

include(VVLGenerateSourceCode)

# Add subprojects ----------------------------------------------------------------------------------------------------------------
if(BUILD_LAYERS OR BUILD_LAYER_SUPPORT_FILES)
    add_subdirectory(layers)
endif()

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests ${CMAKE_BINARY_DIR}/tests)
endif()
