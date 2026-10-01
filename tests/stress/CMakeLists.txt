# ~~~
# Copyright (c) 2014-2026 Valve Corporation
# Copyright (c) 2014-2026 LunarG, Inc.
# Copyright (C) 2026 Arm Limited.
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

# To compile sources while VVL builds, add them to an object library
# to be linked into the final exe
add_library(vk_layer_validation_stress_objects OBJECT)
target_sources(vk_layer_validation_stress_objects PRIVATE
    core_stress.cpp
    gpu_av_stress.cpp
    sync_val_stress.cpp
    core_performance.cpp
)

get_target_property(TEST_SOURCES vk_layer_validation_stress_objects SOURCES)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${TEST_SOURCES})

add_dependencies(vk_layer_validation_stress vvl)

# 2 target_link_libraries calls:
# First so that the object library get vk_test_framework include dirs and definitions
# Second to actually link objects library and vk_test_framework into exe
target_link_libraries(vk_layer_validation_stress_objects PRIVATE
    vk_test_framework
)
target_link_libraries(vk_layer_validation_stress PRIVATE
    vk_layer_validation_stress_objects
)

# Want next to other executable in build folder
set_target_properties(vk_layer_validation_stress
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "$<TARGET_FILE_DIR:vk_layer_validation_tests>"
)

if(SLANG_INSTALL_DIR)
    configure_slang_for_target(vk_layer_validation_stress)
endif()

install(TARGETS vk_layer_validation_stress)
if (WIN32)
    install(FILES $<TARGET_RUNTIME_DLLS:vk_layer_validation_stress> DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()
if(SLANG_INSTALL_DIR)
    install_slang_with_target(vk_layer_validation_stress ${CMAKE_INSTALL_LIBDIR})
endif()

include(GoogleTest)
# Discover the tests when running ctest (PRE_TEST), not after every link (POST_BUILD), to keep incremental builds fast.
# If cross-compiling, discovery will use an executable with the wrong architecture and fail.
# Set env. variable VVL_SKIP_GTEST_DISCOVERY to skip discovery.
if(NOT VVL_SKIP_GTEST_DISCOVERY)
    gtest_discover_tests(vk_layer_validation_stress DISCOVERY_TIMEOUT 100 DISCOVERY_MODE PRE_TEST)
endif()

