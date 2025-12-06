# ~~~
# Copyright (c) 2025 Valve Corporation
# Copyright (c) 2025 LunarG, Inc.
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

add_library(vvl_external STATIC)
target_sources(vvl_external PRIVATE
    vma/vma.h
    vma/vma.cpp
    xxhash.h
    inplace_function.h
)

target_include_directories(vvl_external PUBLIC .)

# For external libs, they might not follow our error style
if(${CMAKE_CXX_COMPILER_ID} MATCHES "(GNU|Clang)")
    target_compile_options(vvl_external PRIVATE -w)
elseif(MSVC)
    target_compile_options(vvl_external PRIVATE /W0)
endif()

target_link_libraries(vvl_external PRIVATE
    Vulkan::Headers
)