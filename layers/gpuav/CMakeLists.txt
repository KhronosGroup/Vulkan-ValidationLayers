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

add_subdirectory(spirv)

add_library(vvl_gpu_av STATIC)
target_sources(vvl_gpu_av PRIVATE
    core/gpuav.h
    core/gpuav_constants.h
    core/gpuav_features.cpp
    core/gpuav_record.cpp
    core/gpuav_setup.cpp
    core/gpuav_settings.h
    core/gpuav_settings.cpp
    core/gpuav_validation_pipeline.h
    core/gpuav_validation_pipeline.cpp
    validation_cmd/gpuav_validation_cmd_common.h
    validation_cmd/gpuav_validation_cmd_common.cpp
    validation_cmd/gpuav_draw.h
    validation_cmd/gpuav_draw.cpp
    validation_cmd/gpuav_dispatch.h
    validation_cmd/gpuav_dispatch.cpp
    validation_cmd/gpuav_ray_tracing.h
    validation_cmd/gpuav_ray_tracing.cpp
    validation_cmd/gpuav_copy_buffer_to_image.h
    validation_cmd/gpuav_copy_buffer_to_image.cpp
    validation_cmd/gpuav_copy_memory_indirect.h
    validation_cmd/gpuav_copy_memory_indirect.cpp
    descriptor_validation/gpuav_descriptor_validation.h
    descriptor_validation/gpuav_descriptor_validation.cpp
    descriptor_validation/gpuav_descriptor_set.cpp
    descriptor_validation/gpuav_descriptor_set.h
    debug_printf/debug_printf.cpp
    debug_printf/debug_printf.h
    error_message/gpuav_vuids.cpp
    error_message/gpuav_vuids.h
    instrumentation/gpuav_shader_instrumentor.cpp
    instrumentation/gpuav_shader_instrumentor.h
    instrumentation/gpuav_instrumentation.h
    instrumentation/gpuav_instrumentation.cpp
    instrumentation/buffer_device_address.cpp
    instrumentation/descriptor_checks.h
    instrumentation/descriptor_checks.cpp
    instrumentation/post_process_descriptor_indexing.cpp
    instrumentation/ray_query.cpp
    instrumentation/register_validation.h
    instrumentation/sanitizer.cpp
    instrumentation/vertex_attribute_fetch_oob.cpp
    resources/gpuav_state_trackers.cpp
    resources/gpuav_state_trackers.h
    resources/gpuav_shader_resources.h
    resources/gpuav_vulkan_objects.h
    resources/gpuav_vulkan_objects.cpp
    shaders/gpuav_error_codes.h
    shaders/gpuav_error_header.h
    shaders/gpuav_shaders_constants.h
    shaders/validation_cmd/push_data.h
    ../${API_TYPE}/generated/gpuav_offline_spirv.h
    ../${API_TYPE}/generated/gpuav_offline_spirv.cpp
)

target_link_libraries(vvl_gpu_av PRIVATE
    SPIRV-Headers::SPIRV-Headers
    SPIRV-Tools-opt # Need spirv-tools to call spirv-val on instrumented shaders
    VkLayer_utils # goes last for mimalloc
)

if(MSVC)
    set(GPUAV_SHADERS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
    file(GLOB_RECURSE GPUAV_SHADERS
        "${GPUAV_SHADERS_DIR}/*.vert"
        "${GPUAV_SHADERS_DIR}/*.comp"
        "${GPUAV_SHADERS_DIR}/*.h"
    )
    add_custom_target(Shaders SOURCES ${GPUAV_SHADERS})
    source_group(TREE ${GPUAV_SHADERS_DIR} PREFIX "GPU-AV Shaders" FILES ${GPUAV_SHADERS})
endif()