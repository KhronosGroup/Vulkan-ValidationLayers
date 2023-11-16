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

add_library(vvl_spirv STATIC)
target_sources(vvl_spirv PRIVATE
    spv_instruction.cpp
    spv_instruction.h
)
target_include_directories(vvl_spirv PRIVATE . ../ ../${API_TYPE})
target_link_libraries(vvl_spirv PRIVATE
    SPIRV-Headers::SPIRV-Headers
)

# Khronos validation additional dependencies
if (USE_ROBIN_HOOD_HASHING)
    target_link_libraries(vvl_spirv PRIVATE robin_hood::robin_hood)
endif()

# Using mimalloc on non-Windows OSes currently results in unit test instability with some
# OS version / driver combinations. On 32-bit systems, using mimalloc cause an increase in
# the amount of virtual address space needed, which can also cause stability problems.
if (MSVC AND CMAKE_SIZEOF_VOID_P EQUAL 8)
   find_package(mimalloc CONFIG)
   option(USE_MIMALLOC "Use mimalloc, a fast malloc/free replacement library" ${mimalloc_FOUND})
   if (USE_MIMALLOC)
      target_compile_definitions(vvl_spirv PRIVATE USE_MIMALLOC)
      target_link_libraries(vvl_spirv PRIVATE mimalloc-static)
   endif()
endif()
