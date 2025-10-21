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

add_library(vk_test_framework STATIC
    android_hardware_buffer.h
    layer_validation_tests.h
    layer_validation_tests.cpp
    pipeline_helper.h
    pipeline_helper.cpp
    shader_helper.h
    shader_helper.cpp
    shader_object_helper.h
    shader_object_helper.cpp
    test_common.h
    shader_templates.h
    error_monitor.cpp
    error_monitor.h
    video_objects.h
    render.cpp
    render.h
    binding.h
    binding.cpp
    buffer_helper.h
    test_framework.cpp
    ray_tracing_objects.h
    ray_tracing_objects.cpp
    data_graph_objects.h
    data_graph_objects.cpp
    ray_tracing_helper_nv.h
    ray_tracing_helper_nv.cpp
    external_memory_sync.h
    external_memory_sync.cpp
    sync_helper.h
    sync_helper.cpp
    sync_val_tests.h
    descriptor_helper.h
    descriptor_helper.cpp
    thread_helper.h
    thread_helper.cpp
    gpu_av_helper.h
    render_pass_helper.h
    render_pass_helper.cpp
    feature_requirements.h
    feature_requirements.cpp
    queue_submit_context.h
    queue_submit_context.cpp
)
if (APPLE)
    target_sources(vk_test_framework PRIVATE
        apple_wsi.h
        apple_wsi.mm
    )
    # QuartzCore framework is needed for minimal Metal interaction
    target_link_libraries(vk_test_framework PRIVATE "-framework QuartzCore")
endif()

target_include_directories(vk_test_framework PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

find_package(GTest CONFIG)
find_package(glslang CONFIG)
find_package(SPIRV-Tools CONFIG)

if(SLANG_INSTALL_DIR)
    find_package(slang CONFIG QUIET)
endif()

if(${CMAKE_CXX_COMPILER_ID} MATCHES "(GNU|Clang)")
    target_compile_options(vk_test_framework PUBLIC
        -Wno-sign-compare
        -Wno-shorten-64-to-32
        -Wno-missing-field-initializers
    )
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(vk_test_framework PUBLIC
            -Wno-sign-conversion
            -Wno-implicit-int-conversion
        )
    endif()
elseif(MSVC)
    target_compile_options(vk_test_framework PUBLIC
        /wd4389 # signed/unsigned mismatch
        /wd4267 # Disable some signed/unsigned mismatch warnings.
    )

    if (CMAKE_SIZEOF_VOID_P EQUAL 4)
        # Due to IHV driver issues, we need the extra 2GB of virtual address space for 32 bit testing
        target_link_options(vk_test_framework PUBLIC /LARGEADDRESSAWARE)
    endif()
endif()

target_link_libraries(vk_test_framework PUBLIC
    VkLayer_utils
    glslang::SPIRV
    SPIRV-Tools-static
    SPIRV-Headers::SPIRV-Headers
    GTest::gtest
    GTest::gtest_main
    $<TARGET_NAME_IF_EXISTS:PkgConfig::XCB>
    $<TARGET_NAME_IF_EXISTS:PkgConfig::X11>
    $<TARGET_NAME_IF_EXISTS:PkgConfig::WAYlAND_CLIENT>
)

# setup framework/config.h using framework/config.h.in as a source
file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/config_$<CONFIG>.h" INPUT "config.h.in")

# Since config_$<CONFIG>.h differs per build, set a compiler definition that files can use to include it
target_compile_definitions(vk_test_framework PRIVATE CONFIG_HEADER_FILE="config_$<CONFIG>.h")

target_sources(vk_test_framework PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/config_$<CONFIG>.h)

target_include_directories(vk_test_framework PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}
    ${VVL_SOURCE_DIR}/layers/external
)

if(SLANG_INSTALL_DIR)
    target_link_libraries(vk_test_framework PUBLIC slang::slang)
    target_compile_definitions(vk_test_framework PUBLIC VVL_USE_SLANG)
endif()

