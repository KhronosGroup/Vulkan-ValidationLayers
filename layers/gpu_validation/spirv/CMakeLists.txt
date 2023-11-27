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

add_library(gpu_av_spirv STATIC)
target_sources(gpu_av_spirv PRIVATE
    instruction.h
    instruction.cpp
    function_basic_block.h
    function_basic_block.cpp
    link.h
    module.h
    module.cpp
    type_manager.h
    type_manager.cpp
    pass.h
    pass.cpp
    ray_tracing_runtime_pass.h
    ray_tracing_runtime_pass.cpp
    ${CMAKE_SOURCE_DIR}/layers/${API_TYPE}/generated/spirv_grammar_helper.cpp
)

target_include_directories(gpu_av_spirv PRIVATE .
    ${CMAKE_SOURCE_DIR}/layers
    ${CMAKE_SOURCE_DIR}/layers/${API_TYPE}
)

target_link_libraries(gpu_av_spirv PRIVATE
    SPIRV-Headers::SPIRV-Headers
)

# Khronos validation additional dependencies
if (USE_ROBIN_HOOD_HASHING)
    target_link_libraries(gpu_av_spirv PRIVATE robin_hood::robin_hood)
endif()

# Using mimalloc on non-Windows OSes currently results in unit test instability with some
# OS version / driver combinations. On 32-bit systems, using mimalloc cause an increase in
# the amount of virtual address space needed, which can also cause stability problems.
if (MSVC AND CMAKE_SIZEOF_VOID_P EQUAL 8)
   find_package(mimalloc CONFIG)
   option(USE_MIMALLOC "Use mimalloc, a fast malloc/free replacement library" ${mimalloc_FOUND})
   if (USE_MIMALLOC)
      target_compile_definitions(gpu_av_spirv PRIVATE USE_MIMALLOC)
      target_link_libraries(gpu_av_spirv PRIVATE mimalloc-static)
   endif()
endif()