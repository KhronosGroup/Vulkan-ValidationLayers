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
if (ANDROID_USE_LEGACY_TOOLCHAIN_FILE)
    # https://github.com/android/ndk/issues/1693
    message(WARNING "Using legacy android toolchain!")
endif()

if (ANDROID_PLATFORM LESS "24")
    message(FATAL_ERROR "Vulkan not supported on Android 23 and below!")
endif()

if (ANDROID_PLATFORM LESS "26")
    message(WARNING "No Android HardwareBuffer support!\n"
                    "While Vulkan is supported in Android 24 and 25, there is no AHardwareBuffer support.\n"
                    "To build a version of the Validation Layers for use with Android that will not require AHB support,\n"
                    "simply change `ANDROID_PLATFORM` parameter value in the CMake generate command to 24 or 25.")
endif()

# Required for __android_log_print. Marking as PUBLIC since the tests use __android_log_print as well.
target_link_libraries(VkLayer_utils PUBLIC log)

# For now just install the .so
# TODO: This seems valid only if CMAKE_ANDROID_STL_TYPE is c++_static.
install(TARGETS VkLayer_khronos_validation DESTINATION ${CMAKE_INSTALL_LIBDIR})
