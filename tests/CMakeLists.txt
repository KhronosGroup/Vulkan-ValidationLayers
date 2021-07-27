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
    vkpositivelayertests.cpp
    vksyncvaltests.cpp
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
if (NOT ANDROID)
    set(layer_test_target_ vk_layer_validation_tests)
    add_executable(${layer_test_target_}
                   layer_validation_tests.cpp
                   ../layers/vk_format_utils.cpp
                   ../layers/convert_to_renderpass2.cpp
                   ../layers/generated/vk_safe_struct.cpp
                   ../layers/generated/lvt_function_pointers.cpp
                   ${COMMON_CPP})
else()
    set(layer_test_target_ VulkanLayerValidationTests)
    add_library(${layer_test_target_} SHARED
                   layer_validation_tests.cpp
                   ../layers/vk_format_utils.cpp
                   ../layers/convert_to_renderpass2.cpp
                   ../layers/generated/vk_safe_struct.cpp
                   ../layers/generated/lvt_function_pointers.cpp
                   ${COMMON_CPP})
endif()
list(APPEND VVL_COMPILED_TARGETS ${layer_test_target_})

add_test(NAME ${layer_test_target_} COMMAND vk_layer_validation_tests)
add_dependencies(${layer_test_target_} VkLayer_khronos_validation VkLayer_khronos_validation-json)
if ((NOT GTEST_IS_STATIC_LIB) AND (NOT ANDROID))
    set_target_properties(${layer_test_target_} PROPERTIES COMPILE_DEFINITIONS "GTEST_LINKED_AS_SHARED_LIBRARY=1")
endif()
# Note that there is no need to add GTEST directories here due to googletest exporting them via the gtest target.
target_include_directories(${layer_test_target_}
                           PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
                                  ${PROJECT_SOURCE_DIR}/layers
                                  ${PROJECT_SOURCE_DIR}/layers/generated
                                  ${GLSLANG_INCLUDE_DIR}
                                  ${SPIRV_TOOLS_INCLUDE_DIR}
                                  ${SPIRV_HEADERS_INCLUDE_DIR}
                                  ${CMAKE_CURRENT_BINARY_DIR}
                                  ${CMAKE_BINARY_DIR}
                                  ${PROJECT_BINARY_DIR}
                                  ${VulkanHeaders_INCLUDE_DIR}
                                  ${PROJECT_BINARY_DIR}/layers)
if (USE_ROBIN_HOOD_HASHING)
    target_include_directories(${layer_test_target_} PUBLIC ${ROBIN_HOOD_HASHING_INCLUDE_DIR})
endif()
add_dependencies(${layer_test_target_}
                 VkLayer_utils)

option(PORTABILITY_TESTS_USE_DEVSIM "Automatically add the devsim layer for portability tests" OFF)
if (PORTABILITY_TESTS_USE_DEVSIM)
    target_compile_definitions(${layer_test_target_} PRIVATE PORTABILITY_TESTS_USE_DEVSIM=1)
endif()

if (VVL_ENABLE_ASAN)
    target_compile_options(${layer_test_target_} PRIVATE -fsanitize=address)
    # NOTE: Use target_link_options when cmake 3.13 is available on CI
    target_link_libraries(${layer_test_target_} PRIVATE "-fsanitize=address")
endif()

if (NOT MSVC)
    target_compile_options(${layer_test_target_} PRIVATE "-Wno-sign-compare")
endif()

# Specify target_link_libraries
target_link_libraries(${layer_test_target_}
                      PRIVATE VkLayer_utils)

if(WIN32)
    set_target_properties(gtest PROPERTIES FOLDER ${LAYERS_HELPER_FOLDER})
    set_target_properties(gtest_main PROPERTIES FOLDER ${LAYERS_HELPER_FOLDER})
    target_link_libraries(${layer_test_target_}
                          PRIVATE gtest
                                  gtest_main
                                  ${GLSLANG_LIBRARIES})
else()
    target_compile_options(${layer_test_target_} PRIVATE "-Wno-sign-compare")
    if(BUILD_WSI_XCB_SUPPORT OR BUILD_WSI_XLIB_SUPPORT)
        target_link_libraries(${layer_test_target_}
                              PRIVATE ${XCB_LIBRARIES}
                                      ${X11_LIBRARIES}
                                      gtest
                                      gtest_main
                                      dl
                                      ${GLSLANG_LIBRARIES})
    else()
        target_link_libraries(${layer_test_target_}
                              PRIVATE gtest
                                      gtest_main
                                      dl
                                      ${GLSLANG_LIBRARIES})
    endif()
endif()

if(WIN32)
    # For Windows, copy necessary gtest DLLs to the right spot for the vk_layer_tests...
    if(NOT GTEST_IS_STATIC_LIB)
        file(TO_NATIVE_PATH ${CMAKE_BINARY_DIR}/googletest/googletest/$<CONFIG>/*.dll SRC_GTEST_DLLS)
        file(TO_NATIVE_PATH ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG> DST_GTEST_DLLS)
        add_custom_command(TARGET ${layer_test_target_} POST_BUILD
                           COMMAND xcopy /Y /I ${SRC_GTEST_DLLS} ${DST_GTEST_DLLS})
    endif()
endif()

if(INSTALL_TESTS)
    install(TARGETS ${layer_test_target_} DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

if (ANDROID)
    if (NOT ANDROID_SDK_HOME)
        set(ANDROID_SDK_HOME $ENV{ANDROID_SDK_HOME})
    endif()
    if (NOT ANDROID_SDK_HOME)
        message(FATAL_ERROR "Please specify ANDROID_SDK_HOME")
    endif()

    if (NOT ANDROID_NDK_HOME)
        set(ANDROID_NDK_HOME $ENV{ANDROID_NDK_HOME})
    endif()
    if (NOT ANDROID_NDK_HOME)
        message(FATAL_ERROR "Please specify ANDROID_NDK_HOME. This is commonly $ANDROID_SDK_HOME/ndk-bundle.")
    endif()

    if (NOT ANDROID_BUILD_TOOLS)
        set(ANDROID_BUILD_TOOLS $ENV{ANDROID_BUILD_TOOLS})
    endif()
    if (NOT ANDROID_BUILD_TOOLS)
        message(FATAL_ERROR "Please specify ANDROID_BUILD_TOOLS version ($ANDROID_SDK_HOME/build-tools/<version>)")
    endif()

    if (NOT ANDROID_STL)
        set(ANDROID_STL $ENV{ANDROID_STL})
    endif()
    if (NOT ANDROID_STL)
        set(ANDROID_STL c++_static)
    endif()

    # Necessary for vulkan_wrapper.h
    target_include_directories(${layer_test_target_} PRIVATE ${ANDROID_NDK_HOME}/sources/third_party/vulkan/src/common)

    set(_app_glue_dir ${ANDROID_NDK_HOME}/sources/android/native_app_glue)
    target_sources(${layer_test_target_} PRIVATE ${_app_glue_dir}/android_native_app_glue.c)
    target_include_directories(${layer_test_target_} PRIVATE ${_app_glue_dir})
    target_compile_definitions(${layer_test_target_} PRIVATE VALIDATION_APK VK_PROTOTYPES)

    # TODO use target_link_options if using cmake >= 3.13
    target_link_libraries(${layer_test_target_} PRIVATE "-u ANativeActivity_onCreate")

    target_link_libraries(${layer_test_target_} PRIVATE SPIRV-Tools log android)

    set(_android_jar ${ANDROID_SDK_HOME}/platforms/android-${ANDROID_PLATFORM}/android.jar)
    set(_aapt ${ANDROID_SDK_HOME}/build-tools/${ANDROID_BUILD_TOOLS}/aapt)
    set(_zipalign ${ANDROID_SDK_HOME}/build-tools/${ANDROID_BUILD_TOOLS}/zipalign)
    add_custom_command(TARGET ${layer_test_target_} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E rm -rf ${CMAKE_BINARY_DIR}/apk
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/apk/out
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/platforms/android/AndroidManifest.xml ${CMAKE_BINARY_DIR}/apk/AndroidManifest.xml
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${layer_test_target_}> ${CMAKE_BINARY_DIR}/apk/out/lib/${ANDROID_ABI}/$<TARGET_FILE_NAME:${layer_test_target_}>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:VkLayer_khronos_validation> ${CMAKE_BINARY_DIR}/apk/out/lib/${ANDROID_ABI}/$<TARGET_FILE_NAME:VkLayer_khronos_validation>
        COMMAND ${_aapt} package -f -M ${CMAKE_BINARY_DIR}/apk/AndroidManifest.xml -I ${_android_jar} -S ${CMAKE_CURRENT_SOURCE_DIR}/platforms/android/res -F ${CMAKE_BINARY_DIR}/apk/out/${layer_test_target_}-unaligned.apk ${CMAKE_BINARY_DIR}/apk/out
        COMMAND jarsigner -verbose -keystore $ENV{HOME}/.android/debug.keystore -storepass android -keypass android ${CMAKE_BINARY_DIR}/apk/out/${layer_test_target_}-unaligned.apk androiddebugkey
        COMMAND ${_zipalign} -f 4 ${CMAKE_BINARY_DIR}/apk/out/${layer_test_target_}-unaligned.apk ${CMAKE_BINARY_DIR}/apk/out/${layer_test_target_}.apk
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/..)
endif()

if (NOT ANDROID)
    add_subdirectory(layers)
endif()
