# ~~~
# Copyright (c) 2014-2018 Valve Corporation
# Copyright (c) 2014-2018 LunarG, Inc.
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

set(GTEST_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/../external/googletest)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_definitions(-DVK_USE_PLATFORM_WIN32_KHR -DWIN32_LEAN_AND_MEAN)
    # Workaround for TR1 deprecation in Visual Studio 15.5 until Google Test is updated
    add_definitions(-D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)
    set(DisplayServer Win32)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Android")
    add_definitions(-DVK_USE_PLATFORM_ANDROID_KHR)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    if(BUILD_WSI_XCB_SUPPORT)
        add_definitions(-DVK_USE_PLATFORM_XCB_KHR)
    endif()

    if(BUILD_WSI_XLIB_SUPPORT)
        add_definitions(-DVK_USE_PLATFORM_XLIB_KHR)
    endif()

    if(BUILD_WSI_WAYLAND_SUPPORT)
        add_definitions(-DVK_USE_PLATFORM_WAYLAND_KHR)
    endif()

    if(BUILD_WSI_MIR_SUPPORT)
        add_definitions(-DVK_USE_PLATFORM_MIR_KHR)
        include_directories(${MIR_INCLUDE_DIR})
    endif()
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    add_definitions(-DVK_USE_PLATFORM_MACOS_MVK)
else()
    message(FATAL_ERROR "Unsupported Platform!")
endif()

if(WIN32)
    file(COPY vk_layer_validation_tests.vcxproj.user DESTINATION ${CMAKE_BINARY_DIR}/tests)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

if(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_CRT_SECURE_NO_WARNINGS -D_USE_MATH_DEFINES")

    # If MSVC, disable some signed/unsigned mismatch warnings.
    if(MSVC)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4267")
    endif()

else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

set(LIBGLM_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/libs)

set(COMMON_CPP vkrenderframework.cpp vktestbinding.cpp vktestframework.cpp test_environment.cpp)

if(NOT WIN32)
    # extra setup for out-of-tree builds
    if(NOT (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR))
        add_custom_target(binary-dir-symlinks ALL COMMAND ln -sf ${CMAKE_CURRENT_SOURCE_DIR}/run_all_tests.sh VERBATIM)
    endif()
else()
    if(NOT (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR))
        file(TO_NATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/_run_all_tests.ps1 RUN_ALL)
        add_custom_target(binary-dir-symlinks ALL
                          COMMAND ${CMAKE_COMMAND} -E copy_if_different ${RUN_ALL} run_all_tests.ps1
                          VERBATIM)
        set_target_properties(binary-dir-symlinks PROPERTIES FOLDER ${LAYERS_HELPER_FOLDER})
    endif()
endif()

# ~~~
# The vulkan loader search is:
#     User-supplied setting of CMAKE_PREFIX_PATH
#     VULKAN_LOADER_INSTALL_DIR defined via cmake option
#     VULKAN_LOADER_INSTALL_DIR defined via environment variable
#     Default findVulkan operation if the VULKAN_SDK environment variable is defined
# ~~~
set(VULKAN_LOADER_INSTALL_DIR "LOADER-NOTFOUND" CACHE PATH "Absolute path to a Vulkan-Loader install directory")

if(VULKAN_LOADER_INSTALL_DIR)
    message(STATUS "VULKAN_LOADER_INSTALL_DIR specified, using find_package to locate Vulkan")
elseif(ENV{VULKAN_LOADER_INSTALL_DIR})
    message(STATUS "VULKAN_LOADER_INSTALL_DIR environment variable specified, using find_package to locate Vulkan")
endif()
set(
    CMAKE_PREFIX_PATH
    ${CMAKE_PREFIX_PATH};${VULKAN_LOADER_INSTALL_DIR};${VULKAN_HEADERS_INSTALL_DIR};$ENV{VULKAN_LOADER_INSTALL_DIR};$ENV{VULKAN_HEADERS_INSTALL_DIR}
    )
find_package(Vulkan)
set(LIBVK "Vulkan::Vulkan")

add_executable(vk_layer_validation_tests layer_validation_tests.cpp ../layers/vk_format_utils.cpp ${COMMON_CPP})
set_target_properties(vk_layer_validation_tests PROPERTIES COMPILE_DEFINITIONS "GTEST_LINKED_AS_SHARED_LIBRARY=1")
target_include_directories(vk_layer_validation_tests
                           PUBLIC
                           ${VulkanHeaders_INCLUDE_DIR}
                           ${CMAKE_CURRENT_SOURCE_DIR}
                           ${GTEST_LOCATION}/googletest/include
                           ${PROJECT_SOURCE_DIR}/layers
                           ${GLSLANG_SPIRV_INCLUDE_DIR}
                           ${CMAKE_CURRENT_BINARY_DIR}
                           ${CMAKE_BINARY_DIR}
                           ${PROJECT_BINARY_DIR}
                           ${PROJECT_BINARY_DIR}/layers)
add_dependencies(vk_layer_validation_tests
                 VkLayer_utils
                 VkLayer_core_validation-json
                 VkLayer_device_profile_api-json
                 VkLayer_object_tracker-json
                 VkLayer_parameter_validation-json
                 VkLayer_standard_validation-json
                 VkLayer_threading-json
                 VkLayer_unique_objects-json)

if(NOT WIN32)
    set_target_properties(vk_layer_validation_tests PROPERTIES COMPILE_FLAGS "-Wno-sign-compare")
    if(BUILD_WSI_XCB_SUPPORT OR BUILD_WSI_XLIB_SUPPORT)
        target_link_libraries(
            vk_layer_validation_tests ${LIBVK} ${XCB_LIBRARIES} ${X11_LIBRARIES} gtest gtest_main ${GLSLANG_LIBRARIES})
    else()
        target_link_libraries(vk_layer_validation_tests ${LIBVK} gtest gtest_main ${GLSLANG_LIBRARIES})
    endif()
endif()
if(WIN32)
    target_link_libraries(vk_layer_validation_tests ${LIBVK} gtest gtest_main ${GLSLANG_LIBRARIES})
endif()

if(WIN32)
    # For Windows, copy necessary gtest DLLs to the right spot for the vk_layer_tests...
    file(TO_NATIVE_PATH ${PROJECT_BINARY_DIR}/external/googletest/googletest/$<CONFIGURATION>/*.dll SRC_GTEST_DLLS)
    file(TO_NATIVE_PATH ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION> DST_GTEST_DLLS)
    add_custom_command(TARGET vk_layer_validation_tests POST_BUILD COMMAND xcopy /Y /I ${SRC_GTEST_DLLS} ${DST_GTEST_DLLS})
    # Copy the loader shared lib (if supplied) to the test application directory so the test app finds it.
    if(VULKAN_LOADER_INSTALL_DIR)
        add_custom_command(TARGET vk_layer_validation_tests POST_BUILD
                           COMMAND ${CMAKE_COMMAND} -E copy ${VULKAN_LOADER_INSTALL_DIR}/bin/vulkan-1.dll
                                   $<TARGET_FILE_DIR:vk_layer_validation_tests>)
    endif()
endif()

if(INSTALL_TESTS)
    install(TARGETS vk_layer_validation_tests DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

add_subdirectory(layers)
