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

# "CMAKE_ANDROID_STL_TYPE specifies the C++ STL implementation to be used. Earlier NDK releases
# supported a range of different options, but projects should now use either c++_shared or c++_static.
# Only use the latter if your application consists of a single shared library."- Professional CMake
#
# Here is the problem we encountered when building tests with c++_static:
#
# https://developer.android.com/ndk/guides/cpp-support recommends using
# c++_shared for applications that use more than one shared library.
# If multiple libraries using c++_static are loaded several copies of
# the globals will be present in the C++ runtime. This also happens
# if the same library is dlopen/dlclosed several times, as when running
# the Layer Validation Tests. Some of the c++ runtime globals are
# thread_local, so each copy consumes a TLS key. There are only 128 TLS
# keys allowed on android, and the unit tests can hit this because of
# repeatedly loading and unloading VVL.
#
# The drawback to using c++_shared is that the layer library can no longer
# be installed manually, but must be installed in an APK. It is still
# common practice to load layer libraries manually.
if ("${CMAKE_ANDROID_STL_TYPE}" MATCHES "static")
    message(FATAL_ERROR "Cannot build tests with ${CMAKE_ANDROID_STL_TYPE}!")
endif()

set(native_app_glue_dir "${ANDROID_NDK}/sources/android/native_app_glue")

if (NOT EXISTS ${native_app_glue_dir})
    message(FATAL_ERROR "Couldn't find Android Native Glue directory!")
endif()

# While AndroidNdkModules.cmake exists it cannot be used reliably since it's not part of upstream CMake.
target_sources(vk_layer_validation_tests PRIVATE
    ${native_app_glue_dir}/android_native_app_glue.c
    ${native_app_glue_dir}/android_native_app_glue.h
)

target_include_directories(vk_layer_validation_tests PRIVATE ${native_app_glue_dir})

# TODO, add testing to ensure vkEnumerateDeviceExtensionProperties and vkEnumerateDeviceLayerProperties are exported.
# This is needed for older versions of the Android loader.

# TODO, ensure the binary name is VulkanLayerValidationTests instead of vk_layer_validation_tests

# TODO, also based on https://github.com/KhronosGroup/Vulkan-ValidationLayers/pull/3088/files testing should be a SHARED library instead of an executable.
# Needs investigation.

# TODO, look into CMAKE_CROSSCOMPILING_EMULATOR.

# TODO, look into linker option "-u ANativeActivity_onCreate". Do we need to set this linker option?

# TODO, look into Android testing on github actions. https://github.com/microsoft/GSL/blob/main/.github/workflows/android.yml

# TODO, we've been requested by Google to test with an NDK that's no older than 6 months or so.

# Look into 
# https://github.com/KhronosGroup/Vulkan-ValidationLayers/pull/3088/files
#
#    # Necessary for vulkan_wrapper.h
#    target_include_directories(${layer_test_target_} PRIVATE ${ANDROID_NDK_HOME}/sources/third_party/vulkan/src/common)
#
#    set(_app_glue_dir ${ANDROID_NDK_HOME}/sources/android/native_app_glue)
#    target_sources(${layer_test_target_} PRIVATE ${_app_glue_dir}/android_native_app_glue.c)
#    target_include_directories(${layer_test_target_} PRIVATE ${_app_glue_dir})
#    target_compile_definitions(${layer_test_target_} PRIVATE VALIDATION_APK VK_PROTOTYPES)
#
#    # TODO use target_link_options if using cmake >= 3.13
#    target_link_libraries(${layer_test_target_} PRIVATE "-u ANativeActivity_onCreate")
#
#    target_link_libraries(${layer_test_target_} PRIVATE SPIRV-Tools log android)
#
#    set(_android_jar ${ANDROID_SDK_HOME}/platforms/android-${ANDROID_PLATFORM}/android.jar)
#    set(_aapt ${ANDROID_SDK_HOME}/build-tools/${ANDROID_BUILD_TOOLS}/aapt)
#    set(_zipalign ${ANDROID_SDK_HOME}/build-tools/${ANDROID_BUILD_TOOLS}/zipalign)
#    add_custom_command(TARGET ${layer_test_target_} POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E rm -rf ${CMAKE_BINARY_DIR}/apk
#        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/apk/out
#        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/platforms/android/AndroidManifest.xml ${CMAKE_BINARY_DIR}/apk/AndroidManifest.xml
#        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${layer_test_target_}> ${CMAKE_BINARY_DIR}/apk/out/lib/${ANDROID_ABI}/$<TARGET_FILE_NAME:${layer_test_target_}>
#        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:VkLayer_khronos_validation> ${CMAKE_BINARY_DIR}/apk/out/lib/${ANDROID_ABI}/$<TARGET_FILE_NAME:VkLayer_khronos_validation>
#        COMMAND ${_aapt} package -f -M ${CMAKE_BINARY_DIR}/apk/AndroidManifest.xml -I ${_android_jar} -S ${CMAKE_CURRENT_SOURCE_DIR}/platforms/android/res -F ${CMAKE_BINARY_DIR}/apk/out/${layer_test_target_}-unaligned.apk ${CMAKE_BINARY_DIR}/apk/out
#        COMMAND jarsigner -verbose -keystore $ENV{HOME}/.android/debug.keystore -storepass android -keypass android ${CMAKE_BINARY_DIR}/apk/out/${layer_test_target_}-unaligned.apk androiddebugkey
#        COMMAND ${_zipalign} -f 4 ${CMAKE_BINARY_DIR}/apk/out/${layer_test_target_}-unaligned.apk ${CMAKE_BINARY_DIR}/apk/out/${layer_test_target_}.apk
#        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/..)
