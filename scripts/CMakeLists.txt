# ~~~
# Copyright (c) 2023 LunarG, Inc.
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

option(UPDATE_DEPS "Run update_deps.py for user")
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

    set(optional_args)
    if (NOT BUILD_TESTS)
        set(optional_args "--optional=tests")
    endif()

    if (UPDATE_DEPS_SKIP_EXISTING_INSTALL)
        set(optional_args ${optional_args} "--skip-existing-install")
    endif()

    list(APPEND cmake_vars "CMAKE_TOOLCHAIN_FILE")
    if (ANDROID)
        list(APPEND cmake_vars "ANDROID_PLATFORM" "CMAKE_ANDROID_ARCH_ABI" "CMAKE_ANDROID_STL_TYPE" "CMAKE_ANDROID_RTTI" "CMAKE_ANDROID_EXCEPTIONS" "ANDROID_USE_LEGACY_TOOLCHAIN_FILE")
    endif()

    set(cmake_var)
    foreach(var IN LISTS cmake_vars)
        if (DEFINED ${var})
            set(optional_args ${optional_args} "--cmake_var")
            set(optional_args ${optional_args} "${var}=${${var}}")
        endif()
    endforeach()

    if (cmake_var)
        set(optional_args "${optional_args} ${cmake_var}")
    endif()

    set(update_dep_py "${CMAKE_CURRENT_LIST_DIR}/update_deps.py")
    set(known_good_json "${CMAKE_CURRENT_LIST_DIR}/known_good.json")

    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${update_dep_py} ${known_good_json})

    file(TIMESTAMP ${update_dep_py} timestamp_1)
    file(TIMESTAMP ${known_good_json} timestamp_2)
    set(timestamp "${timestamp_1}-${timestamp_2}")

    set(UPDATE_DEPS_TIMESTAMP "0" CACHE STRING "Default value until we run update_deps.py")

    if (timestamp STREQUAL $CACHE{UPDATE_DEPS_TIMESTAMP})
        message(DEBUG "update_deps.py: no work to do.")
    else()
        set(UPDATE_DEPS_DIR_SUFFIX "${_build_type}")
        if (ANDROID)
            set(UPDATE_DEPS_DIR_SUFFIX "android/${UPDATE_DEPS_DIR_SUFFIX}")
        endif()
        set(UPDATE_DEPS_DIR "${PROJECT_SOURCE_DIR}/external/${UPDATE_DEPS_DIR_SUFFIX}" CACHE PATH "Location where update_deps.py installs packages")
        set(helper_file "${UPDATE_DEPS_DIR}/helper.cmake")

        execute_process(
            COMMAND ${PYTHON_EXECUTABLE} ${update_dep_py}
                --dir ${UPDATE_DEPS_DIR} --arch ${_target_arch} --config ${_build_type} --generator "${CMAKE_GENERATOR}" ${optional_args}
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
            RESULT_VARIABLE _update_deps_result
        )
        if (NOT (${_update_deps_result} EQUAL 0))
            message(FATAL_ERROR "Could not run update_deps.py which is necessary to download dependencies.")
        endif()
        set(UPDATE_DEPS_TIMESTAMP ${timestamp} CACHE STRING "Ensure we only run update_deps.py when we need to by caching time stamps" FORCE)
        include(${helper_file})
    endif()
endif()
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
if (VULKAN_HEADERS_INSTALL_DIR)
    list(APPEND CMAKE_PREFIX_PATH ${VULKAN_HEADERS_INSTALL_DIR})
endif()
if (MIMALLOC_INSTALL_DIR)
    list(APPEND CMAKE_PREFIX_PATH ${MIMALLOC_INSTALL_DIR})
endif()

if (CMAKE_CROSSCOMPILING)
    set(CMAKE_FIND_ROOT_PATH ${CMAKE_PREFIX_PATH} PARENT_SCOPE)
else()
    set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} PARENT_SCOPE)
endif()
