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

    message("********************************************************************************")
    message("* NOTE: Adding target vvl_update_deps to run as needed for updating            *")
    message("*       dependencies.                                                          *")
    message("********************************************************************************")

    set(optional_args)
    if (NOT BUILD_TESTS)
        set(optional_args "--optional=tests")
    endif()

    if (UPDATE_DEPS_SKIP_EXISTING_INSTALL)
        set(optional_args ${optional_args} "--skip-existing-install")
    endif()

    if (DEFINED CMAKE_TOOLCHAIN_FILE)
        set(optional_args ${optional_args} "--cmake_var")
        set(optional_args ${optional_args} "CMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}")
    endif()

    set(_update_deps_dir ${VVL_SOURCE_DIR}/external/${_build_type})
    if (UPDATE_DEPS_DIR AND EXISTS "${UPDATE_DEPS_DIR}")
        set(_update_deps_dir ${UPDATE_DEPS_DIR})
    endif()
    set(_helper_file "${_update_deps_dir}/helper.cmake")

    set(update_dep_py "${CMAKE_CURRENT_LIST_DIR}/update_deps.py")
    set(known_good_json "${CMAKE_CURRENT_LIST_DIR}/known_good.json")

    # Add a target so that update_deps.py will run when necessary
    # NOTE: This is triggered off of the timestamps of known_good.json and helper.cmake
    add_custom_command(
        OUTPUT ${_helper_file}
        COMMAND ${PYTHON_EXECUTABLE} ${update_dep_py}
            --dir ${_update_deps_dir} --arch ${_target_arch} --config ${_build_type} --generator "${CMAKE_GENERATOR}" ${optional_args}
        DEPENDS ${known_good_json}
    )

    add_custom_target(vvl_update_deps ALL DEPENDS ${_helper_file})

    # Check if update_deps.py needs to be run on first cmake run
    if (${known_good_json} IS_NEWER_THAN ${_helper_file})
        execute_process(
            COMMAND ${PYTHON_EXECUTABLE} ${update_dep_py}
                --dir ${_update_deps_dir} --arch ${_target_arch} --config ${_build_type} --generator "${CMAKE_GENERATOR}" ${optional_args}
            WORKING_DIRECTORY ${VVL_SOURCE_DIR}
            RESULT_VARIABLE _update_deps_result
        )
        if (NOT (${_update_deps_result} EQUAL 0))
            message(FATAL_ERROR "Could not run update_deps.py which is necessary to download dependencies.")
        endif()
    endif()
    include(${_helper_file})
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
if (VULKAN_HEADERS_INSTALL_DIR)
    list(APPEND CMAKE_PREFIX_PATH ${VULKAN_HEADERS_INSTALL_DIR})
endif()
if (MIMALLOC_INSTALL_DIR)
    list(APPEND CMAKE_PREFIX_PATH ${MIMALLOC_INSTALL_DIR})
endif()

set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} PARENT_SCOPE)
