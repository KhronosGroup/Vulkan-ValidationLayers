# ~~~
# Copyright (c) 2024-2026 LunarG, Inc.
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
    # Passes
    descriptor_indexing_oob_pass.h
    descriptor_indexing_oob_pass.cpp
    descriptor_indexing_non_uniform_pass.h
    descriptor_indexing_non_uniform_pass.cpp
    descriptor_class_general_buffer_pass.h
    descriptor_class_general_buffer_pass.cpp
    descriptor_class_texel_buffer_pass.h
    descriptor_class_texel_buffer_pass.cpp
    descriptor_buffer_pass.h
    descriptor_buffer_pass.cpp
    descriptor_heap_pass.h
    descriptor_heap_pass.cpp
    buffer_device_address_pass.h
    buffer_device_address_pass.cpp
    sanitizer_pass.h
    sanitizer_pass.cpp
    shared_memory_data_race_pass.h
    shared_memory_data_race_pass.cpp
    trace_ray_pass.h
    trace_ray_pass.cpp
    debug_printf_pass.h
    debug_printf_pass.cpp
    post_process_descriptor_indexing_pass.h
    post_process_descriptor_indexing_pass.cpp
    vertex_attribute_fetch_oob_pass.h
    vertex_attribute_fetch_oob_pass.cpp
    mesh_shading_pass.h
    mesh_shading_pass.cpp
    log_error_pass.h
    log_error_pass.cpp

    # Framework
    access_path.h
    access_path.cpp
    cooperative_matrix.h
    function_basic_block.h
    function_basic_block.cpp
    interface.h
    instrumentation_status.h
    link.h
    module.h
    module.cpp
    type_manager.h
    type_manager.cpp
    pass.h
    pass.cpp
    spec_constant.cpp
    ${VVL_SOURCE_DIR}/layers/${API_TYPE}/generated/spirv_grammar_helper.cpp

    ${VVL_SOURCE_DIR}/layers/${API_TYPE}/generated/gpuav_offline_spirv.h
    ${VVL_SOURCE_DIR}/layers/${API_TYPE}/generated/gpuav_offline_spirv_glsl.cpp
    ${VVL_SOURCE_DIR}/layers/${API_TYPE}/generated/gpuav_offline_spirv_slang.cpp
)

target_include_directories(gpu_av_spirv PRIVATE .
    ${VVL_SOURCE_DIR}/layers
    ${VVL_SOURCE_DIR}/layers/external
    ${VVL_SOURCE_DIR}/layers/${API_TYPE}
)

target_link_libraries(gpu_av_spirv PRIVATE
    SPIRV-Headers::SPIRV-Headers
    VkLayer_utils # To pull in compile_definitions
)

if (VVL_ENABLE_TRACY)
    target_link_libraries(gpu_av_spirv PRIVATE TracyClient)
endif()

if(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
    target_compile_options(gpu_av_spirv PRIVATE
        -Wno-sign-conversion
        -Wno-implicit-int-conversion
    )
elseif(MSVC)
    target_compile_options(gpu_av_spirv PRIVATE
        /wd4324 # padding
        /wd4458 # hiding class member
        /wd4457 # hiding function parameter
        /wd4702 # unreachable code
        /wd4389 # signed/unsigned mismatch
    )
endif()

# Found to significantly cut VVL build time
#
# Note: If using ccache, see the CCACHE_SLOPPINESS note in tests/CMakeLists.txt
target_precompile_headers(gpu_av_spirv PRIVATE
    <algorithm>
    <functional>
    <list>
    <memory>
    <mutex>
    <string>
    <vector>
    <vulkan/vulkan.h>
    <spirv/unified1/spirv.hpp>
    # Included by almost all TUs in library
    module.h
)

# Auto generate Shaders' SPIR-V code upon GLSL changes
set(GPUAV_SHADERS_DIR "${VVL_SOURCE_DIR}/layers/gpuav/shaders")
file(GLOB_RECURSE GPUAV_SHADERS CONFIGURE_DEPENDS
    "${GPUAV_SHADERS_DIR}/*.vert"
    "${GPUAV_SHADERS_DIR}/*.comp"
    "${GPUAV_SHADERS_DIR}/*.slang"
    "${GPUAV_SHADERS_DIR}/*.h"
)
# The shader compilers used by generate_spirv.py come from update_deps.py, which builds them for the target.
# When cross compiling they cannot run on the host, so rely on the checked in generated files instead.
if (CMAKE_CROSSCOMPILING)
    set(_compile_gpuav_shaders_default OFF)
else()
    set(_compile_gpuav_shaders_default ON)
endif()
option(VVL_COMPILE_GPUAV_SHADERS "Recompile GPU-AV shaders to SPIR-V when they change" ${_compile_gpuav_shaders_default})
if (VVL_COMPILE_GPUAV_SHADERS)
    find_package(Python3 REQUIRED COMPONENTS Interpreter)
    set(GPUAV_SHADERS_STAMP "${CMAKE_CURRENT_BINARY_DIR}/gpuav_offline_spirv.stamp")
    add_custom_command(
        OUTPUT "${GPUAV_SHADERS_STAMP}"
        BYPRODUCTS
          "${VVL_SOURCE_DIR}/layers/${API_TYPE}/generated/gpuav_offline_spirv.h"
          "${VVL_SOURCE_DIR}/layers/${API_TYPE}/generated/gpuav_offline_spirv_glsl.cpp"
          "${VVL_SOURCE_DIR}/layers/${API_TYPE}/generated/gpuav_offline_spirv_slang.cpp"
        COMMAND Python3::Interpreter "scripts/generate_spirv.py"
        COMMAND ${CMAKE_COMMAND} -E touch "${GPUAV_SHADERS_STAMP}"
        DEPENDS ${GPUAV_SHADERS}
        WORKING_DIRECTORY ${VVL_SOURCE_DIR}
        VERBATIM
    )
    add_custom_target(vvl_gpu_av_shaders_spirv DEPENDS "${GPUAV_SHADERS_STAMP}")
    add_dependencies(gpu_av_spirv vvl_gpu_av_shaders_spirv)
else()
    # The checked in generated files would not match edited shaders, so fail instead of using them.
    # Editing a shader re-runs CMake so this is checked again.
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${GPUAV_SHADERS})
    find_package(Git QUIET)
    if (GIT_FOUND)
        execute_process(
            COMMAND ${GIT_EXECUTABLE} status --porcelain -- ${GPUAV_SHADERS}
            WORKING_DIRECTORY ${VVL_SOURCE_DIR}
            RESULT_VARIABLE _git_status_result
            OUTPUT_VARIABLE _edited_gpuav_shaders
            ERROR_QUIET
        )
        if (_git_status_result EQUAL 0 AND _edited_gpuav_shaders)
            message(FATAL_ERROR "GPU-AV shaders were edited, but they are not compiled to SPIR-V as VVL_COMPILE_GPUAV_SHADERS is OFF (default when cross compiling):\n${_edited_gpuav_shaders}")
        endif()
    endif()
endif()
