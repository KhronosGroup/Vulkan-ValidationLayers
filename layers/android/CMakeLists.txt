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

message(STATUS "Targeting Android API Level ${CMAKE_SYSTEM_VERSION}")

message(STATUS "Building with Android NDK Version ${ANDROID_NDK_REVISION}")

# Required for __android_log_print. Marking as PUBLIC since the tests use __android_log_print as well.
target_link_libraries(VkLayer_utils PUBLIC log)

install(TARGETS vvl DESTINATION ${CMAKE_INSTALL_LIBDIR})
