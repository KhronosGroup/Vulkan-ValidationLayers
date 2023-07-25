# ~~~
# Copyright (c) 2023-2023 The Khronos Group Inc.
# Copyright (c) 2023-2023 RasterGrid Kft.
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
project(VVL_CUSTOM_FRAMEWORK LANGUAGES CXX)

cmake_minimum_required(VERSION 3.17.2)

set(CMAKE_CXX_STANDARD 17)

set(VVL_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../../")
set(VVL_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/vvl/build")
set(VVL_UPDATE_DEPS_DIR "${CMAKE_CURRENT_BINARY_DIR}/vvl/external")

execute_process(COMMAND ${CMAKE_COMMAND} -E rm -rf ${VVL_BINARY_DIR})

execute_process(
    COMMAND ${CMAKE_COMMAND} -S ${VVL_SOURCE_DIR} -B ${VVL_BINARY_DIR}
        -D BUILD_TESTS=ON -G "Ninja" -D CMAKE_BUILD_TYPE=Debug -D UPDATE_DEPS=ON -D UPDATE_DEPS_DIR=${VVL_UPDATE_DEPS_DIR}
)
execute_process(COMMAND ${CMAKE_COMMAND} --build ${VVL_BINARY_DIR} --clean-first --target VkLayer_utils)

list(APPEND CMAKE_PREFIX_PATH "${VVL_UPDATE_DEPS_DIR}/glslang/build/install")
list(APPEND CMAKE_PREFIX_PATH "${VVL_UPDATE_DEPS_DIR}/Vulkan-Headers/build/install")
list(APPEND CMAKE_PREFIX_PATH "${VVL_UPDATE_DEPS_DIR}/SPIRV-Headers/build/install")
list(APPEND CMAKE_PREFIX_PATH "${VVL_UPDATE_DEPS_DIR}/SPIRV-Tools/build/install")
list(APPEND CMAKE_PREFIX_PATH "${VVL_UPDATE_DEPS_DIR}/googletest/build/install")

find_package(VulkanHeaders REQUIRED CONFIG)

find_package(Python3 REQUIRED QUIET)

add_custom_target(test_api_specific_generator_interfaces ALL
    COMMAND Python3::Interpreter ${CMAKE_CURRENT_SOURCE_DIR}/test_api_specific_generator_interfaces.py)

add_executable(custom_test_framework)

target_compile_definitions(custom_test_framework PRIVATE VK_ENABLE_BETA_EXTENSIONS)
target_compile_definitions(custom_test_framework PRIVATE VVL_TESTS_USE_CMAKE)
target_compile_definitions(custom_test_framework PRIVATE VVL_TESTS_USE_CUSTOM_TEST_FRAMEWORK)

target_include_directories(custom_test_framework PRIVATE
    .
    ../../layers
    ../../layers/vulkan
    ../framework)

find_package(GTest REQUIRED CONFIG QUIET)
find_package(glslang REQUIRED CONFIG QUIET)

target_sources(custom_test_framework PRIVATE
    framework/custom_test_framework.h
    framework/custom_test_framework.cpp
    ../framework/layer_validation_tests.h
    ../framework/layer_validation_tests.cpp
    ../framework/test_common.h
    ../framework/error_monitor.cpp
    ../framework/error_monitor.h
    ../framework/render.cpp
    ../framework/render.h
    ../framework/binding.h
    ../framework/binding.cpp
    ../framework/test_framework.cpp)

target_link_directories(custom_test_framework PRIVATE ${VVL_BINARY_DIR}/layers)

# Represents all SPIRV libraries we need
add_library(VVL-SPIRV-LIBS INTERFACE)

find_package(SPIRV-Headers REQUIRED CONFIG QUIET)
target_link_libraries(VVL-SPIRV-LIBS INTERFACE SPIRV-Headers::SPIRV-Headers)

find_package(SPIRV-Tools-opt REQUIRED CONFIG QUIET)
target_link_libraries(VVL-SPIRV-LIBS INTERFACE SPIRV-Tools-opt)

find_package(SPIRV-Tools REQUIRED CONFIG QUIET)

# See https://github.com/KhronosGroup/SPIRV-Tools/issues/3909 for background on this.
# The targets available from SPIRV-Tools change depending on how SPIRV_TOOLS_BUILD_STATIC is set.
# Try to handle all possible combinations so that we work with externally built packages.
if (TARGET SPIRV-Tools)
    target_link_libraries(VVL-SPIRV-LIBS INTERFACE SPIRV-Tools)
elseif(TARGET SPIRV-Tools-static)
    target_link_libraries(VVL-SPIRV-LIBS INTERFACE SPIRV-Tools-static)
elseif(TARGET SPIRV-Tools-shared)
    target_link_libraries(VVL-SPIRV-LIBS INTERFACE SPIRV-Tools-shared)
else()
    message(FATAL_ERROR "Cannot determine SPIRV-Tools target name")
endif()

target_link_libraries(custom_test_framework
    VkLayer_utils
    Vulkan::Headers
    glslang::glslang
    glslang::OGLCompiler
    glslang::OSDependent
    glslang::MachineIndependent
    glslang::GenericCodeGen
    glslang::HLSL
    glslang::SPIRV
    glslang::SPVRemapper
    VVL-SPIRV-LIBS
    GTest::gtest
    GTest::gtest_main)
