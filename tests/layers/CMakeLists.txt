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

cmake_minimum_required(VERSION 2.8.11)

set(LAYER_JSON_FILES VkLayer_device_profile_api)

set(VK_LAYER_RPATH /usr/lib/x86_64-linux-gnu/vulkan/layer:/usr/lib/i386-linux-gnu/vulkan/layer)
set(CMAKE_INSTALL_RPATH ${VK_LAYER_RPATH})

if(WIN32)
    if(NOT (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR))
        foreach(config_file ${LAYER_JSON_FILES})
            file(TO_NATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/windows/${config_file}.json src_json)
            if(CMAKE_GENERATOR MATCHES "^Visual Studio.*")
                file(TO_NATIVE_PATH ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>/${config_file}.json dst_json)
            else()
                file(TO_NATIVE_PATH ${CMAKE_CURRENT_BINARY_DIR}/${config_file}.json dst_json)
            endif()
            add_custom_target(${config_file}-json ALL COMMAND copy ${src_json} ${dst_json} VERBATIM)
            add_dependencies(${config_file}-json ${config_file})
            set_target_properties(${config_file}-json PROPERTIES FOLDER ${LAYERS_HELPER_FOLDER})
        endforeach(config_file)
    endif()
elseif(APPLE)
    # extra setup for out-of-tree builds
    if(NOT (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR))
        if(CMAKE_GENERATOR MATCHES "^Xcode.*")
            add_custom_target(mk_test_layer_config_dir ALL
                              COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>)
            foreach(config_file ${LAYER_JSON_FILES})
                add_custom_target(${config_file}-json ALL
                                  DEPENDS mk_test_layer_config_dir
                                  COMMAND ln -sf ${CMAKE_CURRENT_SOURCE_DIR}/macos/${config_file}.json $<CONFIG>
                                  VERBATIM)
            endforeach(config_file)
        else()
            foreach(config_file ${LAYER_JSON_FILES})
                add_custom_target(${config_file}-json ALL
                                  COMMAND ln -sf ${CMAKE_CURRENT_SOURCE_DIR}/macos/${config_file}.json
                                  VERBATIM)
            endforeach(config_file)
        endif()
    endif()
else()
    # extra setup for out-of-tree builds
    if(NOT (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR))
        foreach(config_file ${LAYER_JSON_FILES})
            add_custom_target(${config_file}-json ALL COMMAND ln -sf ${CMAKE_CURRENT_SOURCE_DIR}/linux/${config_file}.json VERBATIM)
        endforeach(config_file)
    endif()
endif()

if(WIN32)
    macro(add_test_layer target)
        file(TO_NATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/VkLayer_${target}.def DEF_FILE)
        add_custom_target(copy-${target}-def-file ALL
                          COMMAND ${CMAKE_COMMAND} -E copy_if_different ${DEF_FILE} VkLayer_${target}.def
                          VERBATIM)
        set_target_properties(copy-${target}-def-file PROPERTIES FOLDER ${LAYERS_HELPER_FOLDER})
        add_library(VkLayer_${target} SHARED ${ARGN} VkLayer_${target}.def)
        add_dependencies(VkLayer_${target} generate_helper_files VkLayer_utils)
        target_link_libraries(VkLayer_${target} VkLayer_utils)
    endmacro()
elseif(APPLE)
    macro(add_test_layer target)
        add_library(VkLayer_${target} SHARED ${ARGN})
        add_dependencies(VkLayer_${target} generate_helper_files VkLayer_utils)
        set_target_properties(VkLayer_${target} PROPERTIES LINK_FLAGS "-Wl")
        target_link_libraries(VkLayer_${target} VkLayer_utils)
    endmacro()
else()
    macro(add_test_layer target)
        add_library(VkLayer_${target} SHARED ${ARGN})
        add_dependencies(VkLayer_${target} generate_helper_files VkLayer_utils)
        set_target_properties(VkLayer_${target} PROPERTIES LINK_FLAGS "-Wl,-Bsymbolic")
        target_link_libraries(VkLayer_${target} VkLayer_utils)
    endmacro()
endif()

include_directories(${VulkanHeaders_INCLUDE_DIR} # TODO: Clear out this list
                    ${CMAKE_CURRENT_SOURCE_DIR}
                    ${PROJECT_SOURCE_DIR}/layers
                    ${CMAKE_CURRENT_BINARY_DIR}
                    ${PROJECT_BINARY_DIR}
                    ${PROJECT_BINARY_DIR}/layers
                    ${CMAKE_BINARY_DIR})

if(WIN32)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -D_CRT_SECURE_NO_WARNINGS")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -D_CRT_SECURE_NO_WARNINGS")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_CRT_SECURE_NO_WARNINGS /bigobj")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -D_CRT_SECURE_NO_WARNINGS /bigobj")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wpointer-arith -Wno-unused-function")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wpointer-arith -Wno-unused-function")
endif()

set(DEVICE_PROFILE_API_SRCS device_profile_api.cpp ${PROJECT_SOURCE_DIR}/layers/vk_layer_extension_utils.cpp)

add_test_layer(device_profile_api ${DEVICE_PROFILE_API_SRCS})

if(WIN32)
    # For Windows, copy necessary gtest DLLs to the right spot for the vk_layer_tests...
    file(TO_NATIVE_PATH ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>/*device_profile_api.* SRC_LAYER)
    file(TO_NATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/windows/VkLayer_device_profile_api.json SRC_JSON)
    file(TO_NATIVE_PATH ${PROJECT_BINARY_DIR}/layers/$<CONFIGURATION> DST_LAYER)
    add_custom_command(TARGET VkLayer_device_profile_api POST_BUILD
                       COMMAND xcopy /Y /I ${SRC_LAYER} ${DST_LAYER}
                       COMMAND xcopy /Y /I ${SRC_JSON} ${DST_LAYER})
elseif(APPLE)
    if(CMAKE_GENERATOR MATCHES "^Xcode.*")
        add_custom_command(TARGET VkLayer_device_profile_api POST_BUILD
                           COMMAND ln -sf ${CMAKE_CURRENT_BINARY_DIR}/VkLayer_device_profile_api.json
                                   ${CMAKE_BINARY_DIR}/layers/$<CONFIG>
                           COMMAND ln -sf ${CMAKE_CURRENT_BINARY_DIR}/libVkLayer_device_profile_api.dylib
                                   ${CMAKE_BINARY_DIR}/layers/$<CONFIG>
                           VERBATIM)
    else()
        add_custom_command(TARGET VkLayer_device_profile_api POST_BUILD
                           COMMAND ln -sf ${CMAKE_CURRENT_BINARY_DIR}/VkLayer_device_profile_api.json ${CMAKE_BINARY_DIR}/layers
                           COMMAND ln -sf ${CMAKE_CURRENT_BINARY_DIR}/libVkLayer_device_profile_api.dylib ${CMAKE_BINARY_DIR}/layers
                           VERBATIM)
    endif()
else()
    add_custom_command(TARGET VkLayer_device_profile_api POST_BUILD
                       COMMAND ln -sf ${CMAKE_CURRENT_BINARY_DIR}/VkLayer_device_profile_api.json ${PROJECT_BINARY_DIR}/layers
                       COMMAND ln -sf ${CMAKE_CURRENT_BINARY_DIR}/libVkLayer_device_profile_api.so ${PROJECT_BINARY_DIR}/layers
                       VERBATIM)
endif()
