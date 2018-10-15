# ~~~
# Copyright (c) 2016-2018 Valve Corporation
# Copyright (c) 2016-2018 LunarG, Inc.
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

set(TEST_LAYER_NAME VkLayer_device_profile_api)

set(VK_LAYER_RPATH /usr/lib/x86_64-linux-gnu/vulkan/layer:/usr/lib/i386-linux-gnu/vulkan/layer)
set(CMAKE_INSTALL_RPATH ${VK_LAYER_RPATH})

if(WIN32)
    if(NOT (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR))
        file(TO_NATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/windows/${TEST_LAYER_NAME}.json src_json)
        if(CMAKE_GENERATOR MATCHES "^Visual Studio.*")
            file(TO_NATIVE_PATH ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/${TEST_LAYER_NAME}.json dst_json)
        else()
            file(TO_NATIVE_PATH ${CMAKE_CURRENT_BINARY_DIR}/${TEST_LAYER_NAME}.json dst_json)
        endif()
        add_custom_target(${TEST_LAYER_NAME}-json ALL COMMAND copy ${src_json} ${dst_json} VERBATIM)
        add_dependencies(${TEST_LAYER_NAME}-json ${TEST_LAYER_NAME})
        set_target_properties(${TEST_LAYER_NAME}-json PROPERTIES FOLDER ${LAYERS_HELPER_FOLDER})
    endif()
elseif(APPLE)
    # extra setup for out-of-tree builds
    if(NOT (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR))
        if(CMAKE_GENERATOR MATCHES "^Xcode.*")
            add_custom_target(mk_test_layer_config_dir ALL
                              COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>)
            add_custom_target(${TEST_LAYER_NAME}-json ALL
                              DEPENDS mk_test_layer_config_dir
                              COMMAND ln -sf ${CMAKE_CURRENT_SOURCE_DIR}/macos/${TEST_LAYER_NAME}.json $<CONFIG>
                              VERBATIM)
        else()
            add_custom_target(${TEST_LAYER_NAME}-json ALL
                              COMMAND ln -sf ${CMAKE_CURRENT_SOURCE_DIR}/macos/${TEST_LAYER_NAME}.json
                              VERBATIM)
        endif()
    endif()
else()
    # extra setup for out-of-tree builds
    if(NOT (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR))
        add_custom_target(${TEST_LAYER_NAME}-json ALL
                          COMMAND ln -sf ${CMAKE_CURRENT_SOURCE_DIR}/linux/${TEST_LAYER_NAME}.json
                          VERBATIM)
    endif()
endif()

# --------------------------------------------------------------------------------------------------------------------------------

# System-specific macros to create a library target.
if(WIN32)
    macro(AddVkLayer target)
        file(TO_NATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/VkLayer_${target}.def DEF_FILE)
        add_custom_target(copy-${target}-def-file ALL
                          COMMAND ${CMAKE_COMMAND} -E copy_if_different ${DEF_FILE} VkLayer_${target}.def
                          VERBATIM)
        set_target_properties(copy-${target}-def-file PROPERTIES FOLDER ${LAYERS_HELPER_FOLDER})

        add_library(VkLayer_${target} SHARED ${ARGN} VkLayer_${target}.def)
        add_dependencies(VkLayer_${target} VulkanVL_generate_helper_files VkLayer_utils)
        target_link_libraries(VkLayer_${target} PRIVATE VkLayer_utils)
        target_compile_definitions(VkLayer_${target} PRIVATE "_CRT_SECURE_NO_WARNINGS")
        target_compile_options(VkLayer_${target} PRIVATE $<$<CONFIG:Debug>:/bigobj>)
    endmacro()
elseif(APPLE)
    macro(AddVkLayer target)
        add_library(VkLayer_${target} SHARED ${ARGN})
        add_dependencies(VkLayer_${target} VulkanVL_generate_helper_files VkLayer_utils)
        set_target_properties(VkLayer_${target} PROPERTIES LINK_FLAGS "-Wl")
        target_link_libraries(VkLayer_${target} PRIVATE VkLayer_utils)
        target_compile_options(VkLayer_${target} PRIVATE "-Wpointer-arith" "-Wno-unused-function")
    endmacro()
else(UNIX AND NOT APPLE) # i.e.: Linux
    macro(AddVkLayer target)
        add_library(VkLayer_${target} SHARED ${ARGN})
        add_dependencies(VkLayer_${target} VulkanVL_generate_helper_files VkLayer_utils)
        set_target_properties(VkLayer_${target} PROPERTIES LINK_FLAGS "-Wl,-Bsymbolic")
        target_link_libraries(VkLayer_${target} PRIVATE VkLayer_utils)
        target_compile_options(VkLayer_${target} PRIVATE "-Wpointer-arith" "-Wno-unused-function")
    endmacro()
endif()

AddVkLayer(device_profile_api device_profile_api.cpp ${PROJECT_SOURCE_DIR}/layers/vk_layer_extension_utils.cpp)

# --------------------------------------------------------------------------------------------------------------------------------

# TODO: Clear out this list
target_include_directories(${TEST_LAYER_NAME}
                           PRIVATE ${VulkanHeaders_INCLUDE_DIR}
                                   ${CMAKE_CURRENT_SOURCE_DIR}
                                   ${PROJECT_SOURCE_DIR}/layers
                                   ${CMAKE_CURRENT_BINARY_DIR}
                                   ${PROJECT_BINARY_DIR}
                                   ${PROJECT_BINARY_DIR}/layers
                                   ${CMAKE_BINARY_DIR})

if(WIN32)
    # For Windows, copy necessary gtest DLLs to the right spot for the vk_layer_tests...
    if (MSVC_IDE)
        file(TO_NATIVE_PATH ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/*device_profile_api.* SRC_LAYER)
    else()
        file(TO_NATIVE_PATH ${CMAKE_CURRENT_BINARY_DIR}/*device_profile_api.* SRC_LAYER)
    endif()

    file(TO_NATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/windows/${TEST_LAYER_NAME}.json SRC_JSON)
    file(TO_NATIVE_PATH ${PROJECT_BINARY_DIR}/layers/$<CONFIG> DST_LAYER)
    add_custom_command(TARGET ${TEST_LAYER_NAME} POST_BUILD
                       COMMAND xcopy /Y /I ${SRC_LAYER} ${DST_LAYER}
                       COMMAND xcopy /Y /I ${SRC_JSON} ${DST_LAYER})
elseif(APPLE)
    if(CMAKE_GENERATOR MATCHES "^Xcode.*")
        add_custom_command(TARGET ${TEST_LAYER_NAME} POST_BUILD
                           COMMAND ln -sf ${CMAKE_CURRENT_BINARY_DIR}/${TEST_LAYER_NAME}.json ${CMAKE_BINARY_DIR}/layers/$<CONFIG>
                           COMMAND ln -sf ${CMAKE_CURRENT_BINARY_DIR}/lib${TEST_LAYER_NAME}.dylib
                                   ${CMAKE_BINARY_DIR}/layers/$<CONFIG>
                           VERBATIM)
    else()
        add_custom_command(TARGET ${TEST_LAYER_NAME} POST_BUILD
                           COMMAND ln -sf ${CMAKE_CURRENT_BINARY_DIR}/${TEST_LAYER_NAME}.json ${CMAKE_BINARY_DIR}/layers
                           COMMAND ln -sf ${CMAKE_CURRENT_BINARY_DIR}/lib${TEST_LAYER_NAME}.dylib ${CMAKE_BINARY_DIR}/layers
                           VERBATIM)
    endif()
else(UNIX AND NOT APPLE) # i.e.: Linux
    add_custom_command(TARGET ${TEST_LAYER_NAME} POST_BUILD
                       COMMAND ln -sf ${CMAKE_CURRENT_BINARY_DIR}/${TEST_LAYER_NAME}.json ${PROJECT_BINARY_DIR}/layers
                       COMMAND ln -sf ${CMAKE_CURRENT_BINARY_DIR}/lib${TEST_LAYER_NAME}.so ${PROJECT_BINARY_DIR}/layers
                       VERBATIM)
endif()
