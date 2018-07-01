# ~~~
# Copyright (c) 2018 Valve Corporation
# Copyright (c) 2018 LunarG, Inc.
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

if(BUILD_TESTS)
    # googletest is an external dependency for the tests in the ValidationLayers repo. Add googletest to the project if present and
    # not already defined.

    # Suppress all warnings from external projects.
    set_property(DIRECTORY APPEND PROPERTY COMPILE_OPTIONS -w)

    if(TARGET gtest_main)
        # It is possible that a project enclosing this one has defined the gtest target
        message(STATUS "Vulkan-ValidationLayers/external: " "Google Test (googletest) already configured - use it")
    elseif(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/googletest)
        message(STATUS "Vulkan-ValidationLayers/external: " "googletests found - configuring it for tests")
        set(BUILD_GTEST ON CACHE BOOL "Builds the googletest subproject")
        set(BUILD_GMOCK OFF CACHE BOOL "Builds the googlemock subproject")
        set(gtest_force_shared_crt ON CACHE BOOL "Link gtest runtimes dynamically")
        set(BUILD_SHARED_LIBS ON CACHE BOOL "Build shared libraries")
        # EXCLUDE_FROM_ALL keeps the install target from installing GTEST files.
        add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/googletest EXCLUDE_FROM_ALL)
    else()
        message(SEND_ERROR "Vulkan-ValidationLayers/external: Google Test was not found.  "
                           "Provide Google Test in external/googletest or set BUILD_TESTS=OFF")
    endif()
endif()
