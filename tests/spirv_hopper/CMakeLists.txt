# ~~~
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
add_subdirectory(external)
add_executable(spirv-hopper)

target_sources(spirv-hopper PRIVATE
    main.cpp
    spirv_hopper.h
    spirv_hopper.cpp
    pass_through_shaders.cpp
    vulkan_object.h
    vulkan_object.cpp
)

target_link_libraries(spirv-hopper PRIVATE
    Vulkan::Headers
    glslang::SPIRV
    VkLayer_utils
    spirv-reflect
)
