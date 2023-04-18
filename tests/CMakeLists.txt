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
add_executable(vk_layer_validation_tests)
target_sources(vk_layer_validation_tests PRIVATE
    ${VVL_SOURCE_DIR}/layers/utils/convert_to_renderpass2.cpp
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
    positive/atomics.cpp
    positive/best_practices.cpp
    positive/command.cpp
    positive/descriptors.cpp
    positive/dynamic_rendering.cpp
    positive/dynamic_state.cpp
    positive/external_memory_sync.cpp
    positive/gpu_av.cpp
    positive/graphics_library.cpp
    positive/image_buffer.cpp
    positive/instance.cpp
    positive/layer_utils.cpp
    positive/memory.cpp
    positive/mesh.cpp
    positive/other.cpp
    positive/pipeline.cpp
    positive/ray_tracing.cpp
    positive/ray_tracing_pipeline.cpp
    positive/render_pass.cpp
    positive/robustness.cpp
    positive/shaderval.cpp
    positive/sync.cpp
    positive/sync_val.cpp
    positive/tooling.cpp
    positive/video.cpp
    positive/wsi.cpp
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
    negative/pipeline_shader.cpp
    negative/portability_subset.cpp
    negative/protected_memory.cpp
    negative/query.cpp
    negative/ray_tracing.cpp
    negative/ray_tracing_gpu.cpp
    negative/ray_tracing_pipeline.cpp
    negative/renderpass.cpp
    negative/robustness.cpp
    negative/sampler.cpp
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

if (VVL_ENABLE_ASAN)
    target_compile_definitions(vk_layer_validation_tests PRIVATE VVL_ENABLE_ASAN=1)
endif()

add_dependencies(vk_layer_validation_tests VkLayer_khronos_validation)

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

    if(MSVC_IDE)
        set_target_properties(vk_layer_validation_tests PROPERTIES VS_DEBUGGER_ENVIRONMENT "VK_LAYER_PATH=$<TARGET_FILE_DIR:VkLayer_khronos_validation>")
    endif()

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

include(GoogleTest)

if (CMAKE_CROSSCOMPILING)
    gtest_add_tests(TARGET vk_layer_validation_tests)
else()
    gtest_discover_tests(vk_layer_validation_tests DISCOVERY_TIMEOUT 100)
endif()

if (ANDROID)
    # "CMAKE_ANDROID_STL_TYPE specifies the C++ STL implementation to be used. Earlier NDK releases
    # supported a range of different options, but projects should now use either c++_shared or c++_static.
    # Only use the latter if your application consists of a single shared library." - Professional CMake
    #
    # Here is the problem we encountered when building tests with c++_static:
    #
    # https://developer.android.com/ndk/guides/cpp-support recommends using
    # c++_shared for applications that use more than one shared library.
    # If multiple libraries using c++_static are loaded several copies of
    # the globals will be present in the C++ runtime. This also happens
    # if the same library is dlopen/dlclosed several times, as when running
    # the Layer Validation Tests. Some of the c++ runtime globals are
    # thread_local, so each copy consumes a TLS key. There are only 128 TLS
    # keys allowed on android, and the unit tests can hit this because of
    # repeatedly loading and unloading VVL.
    #
    # The drawback to using c++_shared is that the layer library can no longer
    # be installed manually, but must be installed in an APK. It is still
    # common practice to load layer libraries manually.
    if ("${CMAKE_ANDROID_STL_TYPE}" MATCHES "static")
        message(FATAL_ERROR "Cannot build tests with ${CMAKE_ANDROID_STL_TYPE}!")
    endif()

    # AndroidNdkModules.cmake cannot be used reliably since it's not part of upstream CMake.
    target_sources(vk_layer_validation_tests PRIVATE
        ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c
        ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.h
    )
    target_include_directories(vk_layer_validation_tests PUBLIC ${ANDROID_NDK}/sources/android/native_app_glue)
    target_link_libraries(vk_layer_validation_tests PUBLIC log)
endif()

option(INSTALL_TESTS "Install tests")
if(INSTALL_TESTS)
    install(TARGETS vk_layer_validation_tests)
endif()

add_subdirectory(layers)
