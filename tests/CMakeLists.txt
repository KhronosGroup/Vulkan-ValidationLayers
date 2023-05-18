# ~~~
# Copyright (c) 2014-2023 Valve Corporation
# Copyright (c) 2014-2023 LunarG, Inc.
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
if (ANDROID)
    add_library(vk_layer_validation_tests MODULE)
else()
    add_executable(vk_layer_validation_tests)
endif()
target_sources(vk_layer_validation_tests PRIVATE
    framework/android_hardware_buffer.h
    framework/layer_validation_tests.h
    framework/layer_validation_tests.cpp
    framework/test_common.h
    framework/error_monitor.cpp
    framework/error_monitor.h
    framework/video_objects.h
    framework/render.cpp
    framework/render.h
    framework/binding.h
    framework/binding.cpp
    framework/test_framework.cpp
    framework/ray_tracing_objects.h
    framework/ray_tracing_objects.cpp
    positive/android_hardware_buffer.cpp
    positive/atomics.cpp
    positive/best_practices.cpp
    positive/buffer.cpp
    positive/command.cpp
    positive/descriptor_buffer.cpp
    positive/descriptors.cpp
    positive/dynamic_rendering.cpp
    positive/dynamic_state.cpp
    positive/external_memory_sync.cpp
    positive/fragment_shading_rate.cpp
    positive/format_utils.cpp
    positive/gpu_av.cpp
    positive/graphics_library.cpp
    positive/image.cpp
    positive/instance.cpp
    positive/layer_utils.cpp
    positive/memory.cpp
    positive/mesh.cpp
    positive/other.cpp
    positive/pipeline_topology.cpp
    positive/pipeline.cpp
    positive/protected_memory.cpp
    positive/query.cpp
    positive/ray_tracing.cpp
    positive/ray_tracing_pipeline.cpp
    positive/render_pass.cpp
    positive/robustness.cpp
    positive/sampler.cpp
    positive/shader_compute.cpp
    positive/shader_image_access.cpp
    positive/shader_interface.cpp
    positive/shader_limits.cpp
    positive/shader_push_constants.cpp
    positive/shader_spirv.cpp
    positive/shader_storage_image.cpp
    positive/shader_storage_texel.cpp
    positive/sparse.cpp
    positive/sync_object.cpp
    positive/sync_val.cpp
    positive/tooling.cpp
    positive/vertex_input.cpp
    positive/video.cpp
    positive/wsi.cpp
    positive/ycbcr.cpp
    negative/amd_best_practices.cpp
    negative/android_hardware_buffer.cpp
    negative/arm_best_practices.cpp
    negative/atomics.cpp
    negative/best_practices.cpp
    negative/buffer.cpp
    negative/command.cpp
    negative/debug_printf.cpp
    negative/descriptor_buffer.cpp
    negative/descriptors.cpp
    negative/dynamic_rendering.cpp
    negative/dynamic_state.cpp
    negative/external_memory_sync.cpp
    negative/fragment_shading_rate.cpp
    negative/geometry_tessellation.cpp
    negative/gpu_av.cpp
    negative/graphics_library.cpp
    negative/image.cpp
    negative/imageless_framebuffer.cpp
    negative/instanceless.cpp
    negative/memory.cpp
    negative/mesh.cpp
    negative/multiview.cpp
    negative/nvidia_best_practices.cpp
    negative/object_lifetime.cpp
    negative/others.cpp
    negative/pipeline_advanced_blend.cpp
    negative/pipeline_layout.cpp
    negative/pipeline_topology.cpp
    negative/pipeline.cpp
    negative/portability_subset.cpp
    negative/protected_memory.cpp
    negative/query.cpp
    negative/ray_tracing.cpp
    negative/ray_tracing_gpu.cpp
    negative/ray_tracing_pipeline.cpp
    negative/renderpass.cpp
    negative/robustness.cpp
    negative/sampler.cpp
    negative/shader_compute.cpp
    negative/shader_interface.cpp
    negative/shader_limits.cpp
    negative/shader_push_constants.cpp
    negative/shader_mesh.cpp
    negative/shader_spirv.cpp
    negative/shader_storage_image.cpp
    negative/shader_storage_texel.cpp
    negative/sparse.cpp
    negative/subgroups.cpp
    negative/subpass.cpp
    negative/sync_object.cpp
    negative/sync_val.cpp
    negative/transform_feedback.cpp
    negative/vertex_input.cpp
    negative/video.cpp
    negative/viewport_inheritance.cpp
    negative/wsi.cpp
    negative/ycbcr.cpp
    containers/small_vector.cpp
)
get_target_property(TEST_SOURCES vk_layer_validation_tests SOURCES)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${TEST_SOURCES})

if (VVL_ENABLE_ASAN)
    target_compile_definitions(vk_layer_validation_tests PRIVATE VVL_ENABLE_ASAN=1)
endif()

# TODO: Remove once Android.mk is gone.
target_compile_definitions(vk_layer_validation_tests PRIVATE VVL_TESTS_USE_CMAKE)

add_dependencies(vk_layer_validation_tests vvl)

target_include_directories(vk_layer_validation_tests PRIVATE .)

if(${CMAKE_CXX_COMPILER_ID} MATCHES "(GNU|Clang)")
    target_compile_options(vk_layer_validation_tests PRIVATE
        -Wno-sign-compare
        -Wno-shorten-64-to-32
        -Wno-unused-parameter
        -Wno-missing-field-initializers
    )
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(vk_layer_validation_tests PRIVATE
            -Wno-sign-conversion
            -Wno-implicit-int-conversion
        )
    endif()
elseif(MSVC)
    # Disable some signed/unsigned mismatch warnings.
    target_compile_options(vk_layer_validation_tests PRIVATE /wd4267)

    set_target_properties(vk_layer_validation_tests PROPERTIES VS_DEBUGGER_ENVIRONMENT "VK_LAYER_PATH=$<TARGET_FILE_DIR:vvl>")

    if (CMAKE_SIZEOF_VOID_P EQUAL 4)
        # Due to IHV driver issues, we need the extra 2GB of virtual address space for 32 bit testing
        target_link_options(vk_layer_validation_tests PRIVATE /LARGEADDRESSAWARE)
    endif()
endif()

find_package(GTest REQUIRED CONFIG QUIET)
find_package(glslang REQUIRED CONFIG QUIET)

target_link_libraries(vk_layer_validation_tests PRIVATE
    VkLayer_utils
    glslang::glslang
    glslang::OGLCompiler
    glslang::OSDependent
    glslang::MachineIndependent
    glslang::GenericCodeGen
    glslang::HLSL
    glslang::SPIRV
    glslang::SPVRemapper
    VVL-SPIRV-LIBS
    GTest::gtest
    GTest::gtest_main
    ${CMAKE_DL_LIBS}
    $<TARGET_NAME_IF_EXISTS:PkgConfig::XCB>
    $<TARGET_NAME_IF_EXISTS:PkgConfig::X11>
    $<TARGET_NAME_IF_EXISTS:PkgConfig::WAYlAND_CLIENT>
)

if (ANDROID)
    add_subdirectory(android)
    return()
endif()

install(TARGETS vk_layer_validation_tests)

include(GoogleTest)
gtest_discover_tests(vk_layer_validation_tests DISCOVERY_TIMEOUT 100)

add_subdirectory(layers)
