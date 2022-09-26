# ~~~
# Copyright (c) 2014-2021 Valve Corporation
# Copyright (c) 2014-2021 LunarG, Inc.
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

if (VULKANSC)
    set(LAYERS_NAME "VkSCLayer_khronos_validation")
    set(TARGET_NAME "vksc_layer_validation_tests")
    set(GENERATED_DIR "generated-vksc")
    add_definitions(-DVULKANSC)
else()
    set(LAYERS_NAME "VkLayer_khronos_validation")
    set(TARGET_NAME "vk_layer_validation_tests")
    set(GENERATED_DIR "generated")
endif()

# Needed to make structure definitions match with glslang libraries
add_definitions(-DNV_EXTENSIONS -DAMD_EXTENSIONS)

if(ANNOTATED_SPEC_LINK)
    add_definitions(-DANNOTATED_SPEC_LINK=${ANNOTATED_SPEC_LINK})
endif()

if(WIN32)
    add_definitions(-DVK_USE_PLATFORM_WIN32_KHR -DWIN32_LEAN_AND_MEAN -DNOMINMAX)
    # Workaround for TR1 deprecation in Visual Studio 15.5 until Google Test is updated
    add_definitions(-D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)
    # If MSVC, allow Windows to use multiprocessor compilation
    if(MSVC)
        add_compile_options(/MP)
    endif()
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

# Test sources require a lot of edits and possible rewrites for VulkanSC.
# Future changes will make these edits and reenable the sources commented
# out here.
if (VULKANSC)
set(COMMON_CPP
    vklayertests_instanceless.cpp
    #vklayertests_pipeline_shader.cpp
    #vklayertests_buffer_image_memory_sampler.cpp
    #vklayertests_others.cpp
    #vklayertests_best_practices.cpp
    #vklayertests_arm_best_practices.cpp
    #vklayertests_amd_best_practices.cpp
    #vklayertests_gpu.cpp
    #vklayertests_descriptor_renderpass_framebuffer.cpp
    #vklayertests_command.cpp
    #vklayertests_imageless_framebuffer.cpp
    #vklayertests_portability_subset.cpp
    ##vklayertests_viewport_inheritance.cpp
    ##vklayertests_wsi.cpp
    ##positive/command.cpp
    ##positive/descriptors.cpp
    ##positive/image_buffer.cpp
    positive/instance.cpp
    ##positive/other.cpp
    #positive/pipeline.cpp
    #positive/render_pass.cpp
    ##positive/shaderval.cpp
    ##positive/sync.cpp
    ###positive/tooling.cpp
    ##vksyncvaltests.cpp
    vkrenderframework.cpp
    vktestbinding.cpp
    vktestframework.cpp
    test_environment.cpp)
else()
set(COMMON_CPP
    vklayertests_instanceless.cpp
    vklayertests_pipeline_shader.cpp
    vklayertests_buffer_image_memory_sampler.cpp
    vklayertests_others.cpp
    vklayertests_best_practices.cpp
    vklayertests_arm_best_practices.cpp
    vklayertests_amd_best_practices.cpp
    vklayertests_gpu.cpp
    vklayertests_descriptor_renderpass_framebuffer.cpp
    vklayertests_command.cpp
    vklayertests_imageless_framebuffer.cpp
    vklayertests_portability_subset.cpp
    vklayertests_viewport_inheritance.cpp
    vklayertests_wsi.cpp
    positive/command.cpp
    positive/descriptors.cpp
    positive/image_buffer.cpp
    positive/instance.cpp
    positive/other.cpp
    positive/pipeline.cpp
    positive/render_pass.cpp
    positive/shaderval.cpp
    positive/sync.cpp
    positive/tooling.cpp
    vksyncvaltests.cpp
    vkrenderframework.cpp
    vktestbinding.cpp
    vktestframework.cpp
    test_environment.cpp)
endif()

if(NOT TARGET vulkan)
    set(
        CMAKE_PREFIX_PATH
        ${CMAKE_PREFIX_PATH};${VULKAN_HEADERS_INSTALL_DIR};$ENV{VULKAN_HEADERS_INSTALL_DIR}
        )
endif()

set_source_files_properties(${PROJECT_BINARY_DIR}/vk_safe_struct.cpp PROPERTIES GENERATED TRUE)
add_executable(${TARGET_NAME}
               layer_validation_tests.cpp
               ../layers/vk_format_utils.cpp
               ../layers/convert_to_renderpass2.cpp
               ../layers/${GENERATED_DIR}/vk_safe_struct.cpp
               ../layers/${GENERATED_DIR}/lvt_function_pointers.cpp
               ${COMMON_CPP})
add_test(NAME ${TARGET_NAME} COMMAND ${TARGET_NAME})
add_dependencies(${TARGET_NAME} ${LAYERS_NAME} ${LAYERS_NAME}-json)
if(NOT GTEST_IS_STATIC_LIB)
    set_target_properties(${TARGET_NAME} PROPERTIES COMPILE_DEFINITIONS "GTEST_LINKED_AS_SHARED_LIBRARY=1")
endif()
# Note that there is no need to add GTEST directories here due to googletest exporting them via the gtest target.
target_include_directories(${TARGET_NAME}
                           PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
                                  ${PROJECT_SOURCE_DIR}/layers
                                  ${PROJECT_SOURCE_DIR}/layers/${GENERATED_DIR}
                                  ${GLSLANG_INCLUDE_DIR}
                                  ${SPIRV_TOOLS_INCLUDE_DIR}
                                  ${SPIRV_HEADERS_INCLUDE_DIR}
                                  ${CMAKE_CURRENT_BINARY_DIR}
                                  ${CMAKE_BINARY_DIR}
                                  ${PROJECT_BINARY_DIR}
                                  ${VulkanHeaders_INCLUDE_DIR}
                                  ${PROJECT_BINARY_DIR}/layers)
if (USE_ROBIN_HOOD_HASHING)
    target_include_directories(${TARGET_NAME} PUBLIC ${ROBIN_HOOD_HASHING_INCLUDE_DIR})
endif()
add_dependencies(${TARGET_NAME}
                 VkLayer_utils)

option(PORTABILITY_TESTS_USE_DEVSIM "Automatically add the devsim layer for portability tests" OFF)
if (PORTABILITY_TESTS_USE_DEVSIM)
    target_compile_definitions(${TARGET_NAME} PRIVATE PORTABILITY_TESTS_USE_DEVSIM=1)
endif()

if (VVL_ENABLE_ASAN)
    target_compile_options(${TARGET_NAME}  PRIVATE -fsanitize=address)
    # NOTE: Use target_link_options when cmake 3.13 is available on CI
    target_link_libraries(${TARGET_NAME}  PRIVATE "-fsanitize=address")
endif()

if (NOT MSVC)
    target_compile_options(${TARGET_NAME} PRIVATE "-Wno-sign-compare")
endif()

# Specify target_link_libraries
target_link_libraries(${TARGET_NAME}
                      PRIVATE VkLayer_utils
                              gtest
                              gtest_main
                              ${GLSLANG_LIBRARIES}
                              ${SPIRV_TOOLS_LIBRARIES})

if(NOT WIN32)
    target_link_libraries(${TARGET_NAME} PRIVATE dl)
    if(BUILD_WSI_XCB_SUPPORT OR BUILD_WSI_XLIB_SUPPORT)
        target_link_libraries(${TARGET_NAME}
                              PRIVATE ${XCB_LIBRARIES}
                                      ${X11_LIBRARIES})
    endif()
endif()

if(WIN32)
    # For Windows, copy necessary gtest DLLs to the right spot for the vk_layer_tests...
    if(NOT GTEST_IS_STATIC_LIB)
        file(TO_NATIVE_PATH ${CMAKE_BINARY_DIR}/googletest/googletest/$<CONFIG>/*.dll SRC_GTEST_DLLS)
        file(TO_NATIVE_PATH ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG> DST_GTEST_DLLS)
        add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
                           COMMAND xcopy /Y /I ${SRC_GTEST_DLLS} ${DST_GTEST_DLLS})
    endif()
endif()

if(INSTALL_TESTS)
    install(TARGETS ${TARGET_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

add_subdirectory(layers)
