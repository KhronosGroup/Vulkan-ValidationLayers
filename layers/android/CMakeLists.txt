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

if (ANDROID_PLATFORM LESS "26")
    message(FATAL_ERROR "Vulkan-ValidationLayers is not supported on Android 25 and below!")
endif()

# Required for __android_log_print. Marking as PUBLIC since the tests use __android_log_print as well.
target_link_libraries(vvl_utils PUBLIC log)

# For now just install the .so
# TODO: This seems valid only if CMAKE_ANDROID_STL_TYPE is c++_static.
install(TARGETS vvl DESTINATION ${CMAKE_INSTALL_LIBDIR})
