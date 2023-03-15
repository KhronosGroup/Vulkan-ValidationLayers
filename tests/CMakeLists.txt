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
include(GoogleTest) # gtest_discover_tests

# GoogleTest is required for testing framework
find_package(GTest REQUIRED CONFIG)

# glslang is required for testing framework
find_package(glslang CONFIG REQUIRED)

add_executable(vk_layer_validation_tests)

target_sources(vk_layer_validation_tests PRIVATE
    ${VVL_SOURCE_DIR}/layers/convert_to_renderpass2.cpp
    ${VVL_SOURCE_DIR}/layers/generated/lvt_function_pointers.cpp
    ${VVL_SOURCE_DIR}/layers/generated/vk_format_utils.cpp
    ${VVL_SOURCE_DIR}/layers/generated/vk_safe_struct.cpp
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
    positive/best_practices.cpp
    positive/command.cpp
    positive/descriptors.cpp
    positive/dynamic_rendering.cpp
    positive/gpu_av.cpp
    positive/graphics_library.cpp
    positive/image_buffer.cpp
    positive/instance.cpp
    positive/other.cpp
    positive/pipeline.cpp
    positive/ray_tracing.cpp
    positive/ray_tracing_pipeline.cpp
    positive/render_pass.cpp
    positive/shaderval.cpp
    positive/sync.cpp
    positive/tooling.cpp
    positive/video.cpp
    positive/wsi.cpp
    negative/amd_best_practices.cpp
    negative/android_hardware_buffer.cpp
    negative/arm_best_practices.cpp
    negative/best_practices.cpp
    negative/buffer.cpp
    negative/command.cpp
    negative/debug_printf.cpp
    negative/descriptor_renderpass_framebuffer.cpp
    negative/dynamic_rendering.cpp
    negative/external_memory_sync.cpp
    negative/gpu_av.cpp
    negative/graphics_library.cpp
    negative/image.cpp
    negative/imageless_framebuffer.cpp
    negative/instanceless.cpp
    negative/memory.cpp
    negative/nvidia_best_practices.cpp
    negative/object_lifetime.cpp
    negative/others.cpp
    negative/pipeline_shader.cpp
    negative/portability_subset.cpp
    negative/query.cpp
    negative/ray_tracing.cpp
    negative/ray_tracing_gpu.cpp
    negative/ray_tracing_pipeline.cpp
    negative/sampler.cpp
    negative/sparse.cpp
    negative/sync_object.cpp
    negative/sync_val.cpp
    negative/video.cpp
    negative/viewport_inheritance.cpp
    negative/wsi.cpp
    negative/ycbcr.cpp
)

if (VVL_ENABLE_ASAN)
    target_compile_definitions(vk_layer_validation_tests PRIVATE VVL_ENABLE_ASAN=1)
endif()

# gtest_discover_tests has problem with cross-compiling, but it is faster and more robust
if (CMAKE_CROSSCOMPILING)
    gtest_add_tests(TARGET vk_layer_validation_tests)
else()
    gtest_discover_tests(vk_layer_validation_tests DISCOVERY_TIMEOUT 100)
endif()

add_dependencies(vk_layer_validation_tests VkLayer_khronos_validation)
target_include_directories(vk_layer_validation_tests PRIVATE
    ${VVL_SOURCE_DIR}/layers
    .
)

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
            -Wno-enum-enum-conversion
        )
    endif()
elseif(MSVC)
    # Disable some signed/unsigned mismatch warnings.
    target_compile_options(vk_layer_validation_tests PRIVATE /wd4267)

    if(MSVC_IDE)
        set_target_properties(vk_layer_validation_tests PROPERTIES VS_DEBUGGER_ENVIRONMENT "VK_LAYER_PATH=$<TARGET_FILE_DIR:VkLayer_khronos_validation>")
    endif()

    if (CMAKE_SIZEOF_VOID_P EQUAL 4)
        # Due to IHV driver issues, we need the extra 2GB of virtual address space for 32 bit testing
        target_link_options(vk_layer_validation_tests PRIVATE /LARGEADDRESSAWARE)
    endif()
endif()

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

option(INSTALL_TESTS "Install tests")
if(INSTALL_TESTS)
    install(TARGETS vk_layer_validation_tests)
endif()

add_subdirectory(layers)
