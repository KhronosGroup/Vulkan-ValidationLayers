# ~~~
# Copyright (c) 2024 LunarG, Inc.
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

add_executable(instrumentation)

target_sources(instrumentation PRIVATE
    instrumentation.cpp
)

target_include_directories(instrumentation PRIVATE
    ${CMAKE_SOURCE_DIR}/layers
    ${CMAKE_SOURCE_DIR}/layers/${API_TYPE}
    ${CMAKE_SOURCE_DIR}/layers/gpu/spirv)

target_link_libraries(instrumentation PRIVATE
    SPIRV-Headers::SPIRV-Headers
    gpu_av_spirv
    VkLayer_utils
)
