# ~~~
# Copyright (c) 2014-2025 Valve Corporation
# Copyright (c) 2014-2025 LunarG, Inc.
# Copyright (C) 2025 Arm Limited.
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

if (ANDROID)
    # currenlty only load vk_layer_validation_tests in for APK
    return()
else()
    add_executable(vk_layer_validation_stress)
endif()
target_sources(vk_layer_validation_stress PRIVATE
    core_stress.cpp
    gpu_av_stress.cpp
    sync_val_stress.cpp
)

get_target_property(TEST_SOURCES vk_layer_validation_stress SOURCES)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${TEST_SOURCES})

add_dependencies(vk_layer_validation_stress vvl)

target_link_libraries(vk_layer_validation_stress PRIVATE
    vk_test_framework
)

# Want next to other executable in build folder
set_target_properties(vk_layer_validation_stress
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "$<TARGET_FILE_DIR:vk_layer_validation_tests>"
)

# Copy the dependent DLL's into the build directory of vk_layer_validation_stress
if(SLANG_INSTALL_DIR AND WIN32)
    add_custom_command(TARGET vk_layer_validation_stress POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:vk_layer_validation_stress> $<TARGET_RUNTIME_DLLS:vk_layer_validation_stress>
                       COMMAND_EXPAND_LISTS)
endif()

install(TARGETS vk_layer_validation_stress)
if (WIN32)
    install(FILES $<TARGET_RUNTIME_DLLS:vk_layer_validation_stress> DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()

include(GoogleTest)
# If cross-compiling, discovery will use an executable with the wrong architecture and stop execution here.
# Set env. variable VVL_SKIP_GTEST_DISCOVERY to skip discovery.
if(NOT VVL_SKIP_GTEST_DISCOVERY)
    gtest_discover_tests(vk_layer_validation_stress DISCOVERY_TIMEOUT 100)
endif()

