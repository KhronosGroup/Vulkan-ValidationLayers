# ~~~
# Copyright (c) 2023 Valve Corporation
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
cmake_minimum_required(VERSION 3.17.2)

project(VVL_INSTALL LANGUAGES CXX)

set(VVL_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../../")
set(VVL_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/vvl/build")
set(VVL_INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/vvl/install")
set(VVL_UPDATE_DEPS_DIR "${CMAKE_CURRENT_BINARY_DIR}/vvl/external")

execute_process(COMMAND ${CMAKE_COMMAND} -E rm -rf ${VVL_BINARY_DIR})
execute_process(COMMAND ${CMAKE_COMMAND} -E rm -rf ${VVL_INSTALL_DIR})

execute_process(
    # NOTE: LunarG/VulkanTools disables BUILD_LAYERS and enables BUILD_LAYER_SUPPORT_FILES
    # since they are only interested in VkLayer_utils
    COMMAND ${CMAKE_COMMAND} -S ${VVL_SOURCE_DIR} -B ${VVL_BINARY_DIR}
        -D BUILD_LAYERS=OFF -D BUILD_LAYER_SUPPORT_FILES=ON -G "Ninja"
        -D CMAKE_BUILD_TYPE=Debug -D UPDATE_DEPS=ON -D UPDATE_DEPS_DIR=${VVL_UPDATE_DEPS_DIR}
)
execute_process(COMMAND ${CMAKE_COMMAND} --build ${VVL_BINARY_DIR} --clean-first)
execute_process(COMMAND ${CMAKE_COMMAND} --install ${VVL_BINARY_DIR} --prefix ${VVL_INSTALL_DIR})

list(APPEND CMAKE_PREFIX_PATH ${VVL_INSTALL_DIR})
list(APPEND CMAKE_PREFIX_PATH "${VVL_UPDATE_DEPS_DIR}/Vulkan-Headers/build/install")

find_package(VulkanHeaders REQUIRED CONFIG)

# This will ensure the static library is being installed
find_library(vklayer_utils VkLayer_utils REQUIRED)

message(STATUS "vklayer_utils = ${vklayer_utils}")

add_library(foobar)

target_link_libraries(foobar PRIVATE Vulkan::Headers)

target_include_directories(foobar PRIVATE
    ${VVL_INSTALL_DIR}/include/
    ${VVL_INSTALL_DIR}/include/vulkan/
)

target_sources(foobar PRIVATE main.cpp)
