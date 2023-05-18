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

set_directory_properties(PROPERTIES "COMPILE_OPTIONS" "") # Disable compiler warnings

enable_language(C) # NOTE: We need to enable the C language for android_native_app_glue.c

set(native_app_glue_dir "${CMAKE_ANDROID_NDK}/sources/android/native_app_glue")

if (NOT EXISTS ${native_app_glue_dir})
    message(FATAL_ERROR "Couldn't find Android Native Glue directory!")
endif()

add_library(android_glue STATIC)

target_include_directories(android_glue PUBLIC ${native_app_glue_dir})
target_sources(android_glue PRIVATE
    ${native_app_glue_dir}/android_native_app_glue.c
    ${native_app_glue_dir}/android_native_app_glue.h
)

# Despite the fact this issue has been fixed "https://github.com/android-ndk/ndk/issues/381".
# We still need to force export ANativeActivity_onCreate.
target_link_options(android_glue PUBLIC -u ANativeActivity_onCreate)

target_link_libraries(vk_layer_validation_tests PRIVATE android_glue)

# 'VulkanLayerValidationTests.so' needed for Android.mk backcompat
set_target_properties(vk_layer_validation_tests PROPERTIES
    OUTPUT_NAME "VulkanLayerValidationTests"
    PREFIX ""
)

install(TARGETS vk_layer_validation_tests DESTINATION ${CMAKE_INSTALL_LIBDIR})

find_program(GNU_NM NAMES nm)
if (GNU_NM)
    # Ensure ANativeActivity_onCreate is being exported
    add_test(NAME ANativeActivity_onCreate COMMAND ${GNU_NM} --dynamic $<TARGET_FILE:vk_layer_validation_tests>)
    set_tests_properties(ANativeActivity_onCreate
        PROPERTIES PASS_REGULAR_EXPRESSION "T ANativeActivity_onCreate"
    )

    # Ensure compatibility with older Android loaders
    add_test(NAME vkEnumerateDeviceExtensionProperties COMMAND ${GNU_NM} --dynamic $<TARGET_FILE:vvl>)
    set_tests_properties(vkEnumerateDeviceExtensionProperties
        PROPERTIES PASS_REGULAR_EXPRESSION "T vkEnumerateDeviceExtensionProperties"
    )

    # Ensure compatibility with older Android loaders
    add_test(NAME vkEnumerateDeviceLayerProperties COMMAND ${GNU_NM} --dynamic $<TARGET_FILE:vvl>)
    set_tests_properties(vkEnumerateDeviceLayerProperties
        PROPERTIES PASS_REGULAR_EXPRESSION "T vkEnumerateDeviceLayerProperties"
    )
endif()
