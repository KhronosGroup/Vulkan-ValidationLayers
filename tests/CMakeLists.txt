# ~~~
# Copyright (c) 2014-2020 Valve Corporation
# Copyright (c) 2014-2020 LunarG, Inc.
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

# Needed to make structure definitions match with glslang libraries
add_definitions(-DNV_EXTENSIONS -DAMD_EXTENSIONS)

if(ANNOTATED_SPEC_LINK)
    add_definitions(-DANNOTATED_SPEC_LINK=${ANNOTATED_SPEC_LINK})
endif()

if(WIN32)
    add_definitions(-DVK_USE_PLATFORM_WIN32_KHR -DWIN32_LEAN_AND_MEAN -DNOMINMAX)
    # Workaround for TR1 deprecation in Visual Studio 15.5 until Google Test is updated
    add_definitions(-D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)
    # Allow Windows to use multiprocessor compilation
    add_compile_options(/MP)
elseif(ANDROID)
    add_definitions(-DVK_USE_PLATFORM_ANDROID_KHR)
elseif(APPLE)
    add_definitions(-DVK_USE_PLATFORM_MACOS_MVK)
elseif(UNIX AND NOT APPLE) # i.e. Linux
    if(BUILD_WSI_XCB_SUPPORT)
        add_definitions(-DVK_USE_PLATFORM_XCB_KHR)
    endif()

    if(BUILD_WSI_XLIB_SUPPORT)
        add_definitions(-DVK_USE_PLATFORM_XLIB_KHR)
    endif()

    if(BUILD_WSI_WAYLAND_SUPPORT)
        add_definitions(-DVK_USE_PLATFORM_WAYLAND_KHR)
    endif()
else()
    message(FATAL_ERROR "Unsupported Platform!")
endif()

if(WIN32)
    file(COPY vk_layer_validation_tests.vcxproj.user DESTINATION ${CMAKE_BINARY_DIR}/tests)
endif()

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

set(COMMON_CPP
    vklayertests_instanceless.cpp
    vklayertests_pipeline_shader.cpp
    vklayertests_buffer_image_memory_sampler.cpp
    vklayertests_others.cpp
    vklayertests_best_practices.cpp
    vklayertests_arm_best_practices.cpp
    vklayertests_gpu.cpp
    vklayertests_descriptor_renderpass_framebuffer.cpp
    vklayertests_command.cpp
    vklayertests_imageless_framebuffer.cpp
    vkpositivelayertests.cpp
    vkrenderframework.cpp
    vktestbinding.cpp
    vktestframework.cpp
    test_environment.cpp)

if(NOT TARGET vulkan)
    set(
        CMAKE_PREFIX_PATH
        ${CMAKE_PREFIX_PATH};${VULKAN_HEADERS_INSTALL_DIR};$ENV{VULKAN_HEADERS_INSTALL_DIR}
        )
endif()

set_source_files_properties(${PROJECT_BINARY_DIR}/vk_safe_struct.cpp PROPERTIES GENERATED TRUE)
add_executable(vk_layer_validation_tests
               layer_validation_tests.cpp
               ../layers/vk_format_utils.cpp
               ../layers/convert_to_renderpass2.cpp
               ../layers/generated/vk_safe_struct.cpp
               ../layers/generated/lvt_function_pointers.cpp
               ${COMMON_CPP})
add_test(NAME vk_layer_validation_tests COMMAND vk_layer_validation_tests)
add_dependencies(vk_layer_validation_tests VkLayer_khronos_validation VkLayer_khronos_validation-json)
if(NOT GTEST_IS_STATIC_LIB)
    set_target_properties(vk_layer_validation_tests PROPERTIES COMPILE_DEFINITIONS "GTEST_LINKED_AS_SHARED_LIBRARY=1")
endif()
# Note that there is no need to add GTEST directories here due to googletest exporting them via the gtest target.
target_include_directories(vk_layer_validation_tests
                           PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
                                  ${PROJECT_SOURCE_DIR}/layers
                                  ${PROJECT_SOURCE_DIR}/layers/generated
                                  ${GLSLANG_SPIRV_INCLUDE_DIR}
                                  ${SPIRV_TOOLS_INCLUDE_DIR}
                                  ${SPIRV_HEADERS_INCLUDE_DIR}
                                  ${CMAKE_CURRENT_BINARY_DIR}
                                  ${CMAKE_BINARY_DIR}
                                  ${PROJECT_BINARY_DIR}
                                  ${VulkanHeaders_INCLUDE_DIR}
                                  ${PROJECT_BINARY_DIR}/layers)
add_dependencies(vk_layer_validation_tests
                 VkLayer_utils)

# Specify target_link_libraries
if(WIN32)
    set_target_properties(gtest PROPERTIES FOLDER ${LAYERS_HELPER_FOLDER})
    set_target_properties(gtest_main PROPERTIES FOLDER ${LAYERS_HELPER_FOLDER})
    target_link_libraries(vk_layer_validation_tests
                          PRIVATE gtest
                                  gtest_main
                                  ${GLSLANG_LIBRARIES})
else()
    target_compile_options(vk_layer_validation_tests PRIVATE "-Wno-sign-compare")
    if(BUILD_WSI_XCB_SUPPORT OR BUILD_WSI_XLIB_SUPPORT)
        target_link_libraries(vk_layer_validation_tests
                              PRIVATE ${XCB_LIBRARIES}
                                      ${X11_LIBRARIES}
                                      gtest
                                      gtest_main
                                      dl
                                      ${GLSLANG_LIBRARIES})
    else()
        target_link_libraries(vk_layer_validation_tests
                              PRIVATE gtest
                                      gtest_main
                                      dl
                                      ${GLSLANG_LIBRARIES})
    endif()
endif()

if(WIN32)
    # For Windows, copy necessary gtest DLLs to the right spot for the vk_layer_tests...
    if(NOT GTEST_IS_STATIC_LIB)
        file(TO_NATIVE_PATH ${PROJECT_BINARY_DIR}/external/googletest/googletest/$<CONFIG>/*.dll SRC_GTEST_DLLS)
        file(TO_NATIVE_PATH ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG> DST_GTEST_DLLS)
        add_custom_command(TARGET vk_layer_validation_tests POST_BUILD
                           COMMAND xcopy /Y /I ${SRC_GTEST_DLLS} ${DST_GTEST_DLLS})
    endif()
endif()

if(INSTALL_TESTS)
    install(TARGETS vk_layer_validation_tests DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

add_subdirectory(layers)
