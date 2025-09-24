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

# Slang
# ---
if(DEFINED SLANG_INSTALL_DIR)
    # Define paths based on actual structure
    if(WIN32)
        if(EXISTS "${SLANG_INSTALL_DIR}/bin")
            set(SLANG_BIN_DIR "${SLANG_INSTALL_DIR}/bin")
        endif()

        if(EXISTS "${SLANG_INSTALL_DIR}/lib")
            set(SLANG_LIB_DIR "${SLANG_INSTALL_DIR}/lib")
        else()
            set(SLANG_LIB_DIR "${SLANG_INSTALL_DIR}/bin")
        endif()

        set(SLANG_INCLUDE_DIR "${SLANG_INSTALL_DIR}/include")

        # Find slang.lib
        if(EXISTS "${SLANG_LIB_DIR}/slang.lib")
            set(SLANG_LIBRARY "${SLANG_LIB_DIR}/slang.lib")
        else()
            file(GLOB SLANG_LIB_FILES "${SLANG_INSTALL_DIR}/**/slang.lib")
            if(SLANG_LIB_FILES)
                list(GET SLANG_LIB_FILES 0 SLANG_LIBRARY)
            else()
                message(FATAL_ERROR "Could not locate slang.lib in extracted directory")
            endif()
        endif()

        # Find slang.dll
        file(GLOB SLANG_DLL_FILES "${SLANG_INSTALL_DIR}/**/slang.dll")
        if(SLANG_DLL_FILES)
            list(GET SLANG_DLL_FILES 0 SLANG_DLL)
        elseif(EXISTS "${SLANG_BIN_DIR}/slang.dll")
            set(SLANG_DLL "${SLANG_BIN_DIR}/slang.dll")
        else()
            message(FATAL_ERROR "Could not locate slang.dll in extracted directory")
        endif()

        # On Windows, also check for any other DLLs in the same directory
        # as slang.dll and add them to a list for copying
        get_filename_component(SLANG_DLL_DIR "${SLANG_DLL}" DIRECTORY)
        file(GLOB ADDITIONAL_DLLS "${SLANG_DLL_DIR}/*.dll")
        foreach(DLL ${ADDITIONAL_DLLS})
            if(NOT DLL STREQUAL SLANG_DLL)
                list(APPEND SLANG_DEPENDENCY_DLLS ${DLL})
            endif()
        endforeach()

    else()
        set(SLANG_INCLUDE_DIR "${SLANG_INSTALL_DIR}/include")

        if(EXISTS "${SLANG_INSTALL_DIR}/lib")
            set(SLANG_LIB_DIR "${SLANG_INSTALL_DIR}/lib")
        else()
            # Some releases might put libraries in a different location
            file(GLOB LIB_DIRS "${SLANG_INSTALL_DIR}/**/lib")
            if(LIB_DIRS)
                list(GET LIB_DIRS 0 SLANG_LIB_DIR)
            else()
                set(SLANG_LIB_DIR "${SLANG_INSTALL_DIR}")
            endif()
        endif()

        # Find libslang.so
        if(EXISTS "${SLANG_LIB_DIR}/libslang.so")
            set(SLANG_LIBRARY "${SLANG_LIB_DIR}/libslang.so")
            message(STATUS "Found libslang.so at: ${SLANG_LIBRARY}")
        else()
            file(GLOB SLANG_LIB_FILES "${SLANG_INSTALL_DIR}/**/libslang.so")
            if(SLANG_LIB_FILES)
                list(GET SLANG_LIB_FILES 0 SLANG_LIBRARY)
                message(STATUS "Found libslang.so via glob at: ${SLANG_LIBRARY}")
            else()
                message(FATAL_ERROR "Could not locate libslang.so in extracted directory")
            endif()
        endif()

        # Check if we have other shared libraries that might be dependencies
        get_filename_component(SLANG_LIB_DIR_REAL "${SLANG_LIBRARY}" DIRECTORY)
        file(GLOB ADDITIONAL_LIBS "${SLANG_LIB_DIR_REAL}/*.so*")
        foreach(LIB ${ADDITIONAL_LIBS})
            if(NOT LIB STREQUAL SLANG_LIBRARY)
                list(APPEND SLANG_DEPENDENCY_LIBS ${LIB})
            endif()
        endforeach()
    endif()

    # Create an imported target for easier usage
    add_library(slang SHARED IMPORTED GLOBAL)

    set_target_properties(slang PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES ${SLANG_INCLUDE_DIR}
    )

    if(WIN32)
        set_target_properties(slang PROPERTIES
            IMPORTED_IMPLIB ${SLANG_LIBRARY}
            IMPORTED_LOCATION ${SLANG_DLL}
        )
    else()
        set_target_properties(slang PROPERTIES
            IMPORTED_LOCATION ${SLANG_LIBRARY}
        )
    endif()

    # For specific target directories (adds to the common destinations above)
    function(configure_slang_for_target TARGET_NAME)
        if(WIN32)
            add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${SLANG_DLL}
                    $<TARGET_FILE_DIR:${TARGET_NAME}>
                COMMENT "Copying Slang DLL to output directory for ${TARGET_NAME}"
            )

            get_target_property(TARGET_OUTPUT_DIR ${TARGET_NAME} RUNTIME_OUTPUT_DIRECTORY)
            if(NOT TARGET_OUTPUT_DIR)
                set(TARGET_OUTPUT_DIR "$<TARGET_FILE_DIR:${TARGET_NAME}>")
            endif()

            foreach(DEP_DLL ${SLANG_DEPENDENCY_DLLS})
                get_filename_component(DEP_DLL_NAME ${DEP_DLL} NAME)
                add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        ${DEP_DLL}
                        $<TARGET_FILE_DIR:${TARGET_NAME}>
                    COMMENT "Copying dependency DLL ${DEP_DLL_NAME} to output directory for ${TARGET_NAME}"
                )
            endforeach()
        else()
            get_filename_component(SLANG_SONAME ${SLANG_LIBRARY} NAME)
            add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${SLANG_LIBRARY}
                    $<TARGET_FILE_DIR:${TARGET_NAME}>/${SLANG_SONAME}
                COMMENT "Copying Slang shared library to output directory for ${TARGET_NAME}"
            )

            get_target_property(TARGET_OUTPUT_DIR ${TARGET_NAME} RUNTIME_OUTPUT_DIRECTORY)
            if(NOT TARGET_OUTPUT_DIR)
                set(TARGET_OUTPUT_DIR "$<TARGET_FILE_DIR:${TARGET_NAME}>")
            endif()

            # Copy any dependency libraries
            foreach(DEP_LIB ${SLANG_DEPENDENCY_LIBS})
                get_filename_component(DEP_LIB_NAME ${DEP_LIB} NAME)
                add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        ${DEP_LIB}
                        $<TARGET_FILE_DIR:${TARGET_NAME}>/${DEP_LIB_NAME}
                    COMMENT "Copying dependency library ${DEP_LIB_NAME} to output directory for ${TARGET_NAME}"
                )
            endforeach()

            # Set RPATH to look in the same directory as the executable
            set_target_properties(${TARGET_NAME} PROPERTIES
                INSTALL_RPATH "$ORIGIN"
                BUILD_WITH_INSTALL_RPATH TRUE
            )

            # Add post-build step to verify the library was copied and is executable
            add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E echo "Verifying slang library path: $<TARGET_FILE_DIR:${TARGET_NAME}>/${SLANG_SONAME}"
                COMMAND test -f "$<TARGET_FILE_DIR:${TARGET_NAME}>/${SLANG_SONAME}" || echo "Library not copied correctly"
            )
        endif()
    endfunction()

    function(install_slang_with_target TARGET_NAME DESTINATION)
        if(NOT WIN32)
            get_filename_component(SLANG_SONAME ${SLANG_LIBRARY} NAME)
            install(FILES ${SLANG_LIBRARY} DESTINATION ${DESTINATION})

            foreach(DEP_LIB ${SLANG_DEPENDENCY_LIBS})
                get_filename_component(DEP_LIB_NAME ${DEP_LIB} NAME)
                install(FILES ${DEP_LIB} DESTINATION ${DESTINATION})
            endforeach()

            set_target_properties(${TARGET_NAME} PROPERTIES INSTALL_RPATH "$ORIGIN")
         endif()
    endfunction()

    set(USE_SLANG TRUE)
    configure_slang_for_target(vk_test_framework)
else()
    message(STATUS "Skipping Slang setup for 32-bit architecture")
    set(USE_SLANG FALSE)
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
    $<$<BOOL:${USE_SLANG}>:slang>
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

if(USE_SLANG)
    target_compile_definitions(vk_test_framework PUBLIC VVL_USE_SLANG)
    install_slang_with_target(vk_test_framework lib)
endif()
