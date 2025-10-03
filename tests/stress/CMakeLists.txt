# ~~~
# Copyright (c) 2014-2025 Valve Corporation
# Copyright (c) 2014-2025 LunarG, Inc.
# Copyright (C) 2025 Arm Limited.
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
    # currenlty only load vk_layer_validation_tests in for APK
    return()
else()
    add_executable(vk_layer_validation_stress)
endif()
target_sources(vk_layer_validation_stress PRIVATE
    core_stress.cpp
    gpu_av_stress.cpp
    sync_val_stress.cpp
)

# TODO - This slang stuff is a mess and currently need in both spots
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
        else()
            file(GLOB SLANG_LIB_FILES "${SLANG_INSTALL_DIR}/**/libslang.so")
            if(SLANG_LIB_FILES)
                list(GET SLANG_LIB_FILES 0 SLANG_LIBRARY)
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
    configure_slang_for_target(vk_layer_validation_stress)
else()
    message(STATUS "Skipping Slang setup for 32-bit architecture")
    set(USE_SLANG FALSE)
endif()

get_target_property(TEST_SOURCES vk_layer_validation_stress SOURCES)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${TEST_SOURCES})

add_dependencies(vk_layer_validation_stress vvl)

target_link_libraries(vk_layer_validation_stress PRIVATE
    vk_test_framework
)

# Want next to other executable in build folder
get_filename_component(TEST_MAIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.." REALPATH)
set_target_properties(vk_layer_validation_stress
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/.."
)

install(TARGETS vk_layer_validation_stress)

if(USE_SLANG)
    install_slang_with_target(vk_layer_validation_stress lib)
endif()

include(GoogleTest)
# If cross-compiling, discovery will use an executable with the wrong architecture and stop execution here.
# Set env. variable VVL_SKIP_GTEST_DISCOVERY to skip discovery.
if(NOT VVL_SKIP_GTEST_DISCOVERY)
    gtest_discover_tests(vk_layer_validation_stress DISCOVERY_TIMEOUT 100)
endif()

