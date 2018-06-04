cmake_minimum_required(VERSION 2.8.11)

if (TARGET gtest_main)
    message(STATUS "Vulkan-ValidationLayers/tests: googletests found - building tests")

    set(GTEST_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/../external/googletest)

    if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        add_definitions(-DVK_USE_PLATFORM_WIN32_KHR -DWIN32_LEAN_AND_MEAN)
        # Workaround for TR1 deprecation in Visual Studio 15.5 until Google Test is updated
        add_definitions(-D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)
        set(DisplayServer Win32)
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Android")
        add_definitions(-DVK_USE_PLATFORM_ANDROID_KHR)
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        if (BUILD_WSI_XCB_SUPPORT)
            add_definitions(-DVK_USE_PLATFORM_XCB_KHR)
        endif()

        if (BUILD_WSI_XLIB_SUPPORT)
           add_definitions(-DVK_USE_PLATFORM_XLIB_KHR)
        endif()

        if (BUILD_WSI_WAYLAND_SUPPORT)
           add_definitions(-DVK_USE_PLATFORM_WAYLAND_KHR)
        endif()

        if (BUILD_WSI_MIR_SUPPORT)
            add_definitions(-DVK_USE_PLATFORM_MIR_KHR)
            include_directories(${MIR_INCLUDE_DIR})
        endif()
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        add_definitions(-DVK_USE_PLATFORM_MACOS_MVK)
    else()
        message(FATAL_ERROR "Unsupported Platform!")
    endif()

    if (WIN32)
       file(COPY vk_layer_validation_tests.vcxproj.user DESTINATION ${CMAKE_BINARY_DIR}/tests)
    endif()

    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

    if(WIN32)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_CRT_SECURE_NO_WARNINGS -D_USE_MATH_DEFINES")

        # If MSVC, disable some signed/unsigned mismatch warnings.
        if (MSVC)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4267")
        endif()

    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    endif()

    set (LIBGLM_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/libs)

    set(COMMON_CPP
        vkrenderframework.cpp
        vktestbinding.cpp
        vktestframework.cpp
        test_environment.cpp
       )

    if (NOT WIN32)
        # extra setup for out-of-tree builds
        if (NOT (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR))
            add_custom_target(binary-dir-symlinks ALL
                COMMAND ln -sf ${CMAKE_CURRENT_SOURCE_DIR}/run_all_tests.sh
                COMMAND ln -sf ${CMAKE_CURRENT_SOURCE_DIR}/vkvalidatelayerdoc.sh
                VERBATIM
                )
        endif()
    else()
        if (NOT (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR))
            FILE(TO_NATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/_run_all_tests.ps1 RUN_ALL)
            FILE(TO_NATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/_vkvalidatelayerdoc.ps1 VALIDATE_DOC)
            add_custom_target(binary-dir-symlinks ALL
                COMMAND ${CMAKE_COMMAND} -E copy_if_different ${RUN_ALL} run_all_tests.ps1
                COMMAND ${CMAKE_COMMAND} -E copy_if_different ${VALIDATE_DOC} vkvalidatelayerdoc.ps1
                VERBATIM
                )
            set_target_properties(binary-dir-symlinks PROPERTIES FOLDER ${VVL_TARGET_FOLDER})
        endif()
    endif()

    # The vulkan loader search is:
    #     User-supplied setting of CMAKE_PREFIX_PATH
    #     VULKAN_LOADER_INSTALL_DIR defined via cmake option
    #     VULKAN_LOADER_INSTALL_DIR defined via environment variable
    #     Default findVulkan operation if the VULKAN_SDK environment variable is defined
    set(VULKAN_LOADER_INSTALL_DIR "LOADER-NOTFOUND" CACHE PATH "Absolute path to a Vulkan-Loader install directory")

    # TODO: Remove this option after all repos converted
    set(LOADER_REPO_ROOT "LOADER-NOTFOUND" CACHE PATH "Absolute path to the root of the loader repository")

    if(NOT LOADER_REPO_ROOT)
        if ({VULKAN_LOADER_INSTALL_DIR)
            message(STATUS "VULKAN_LOADER_INSTALL_DIR specified, using find_package to locate Vulkan")
        elseif(ENV{VULKAN_LOADER_INSTALL_DIR})
            message(STATUS "VULKAN_LOADER_INSTALL_DIR environment variable specified, using find_package to locate Vulkan")
        endif()
        set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH};${VULKAN_LOADER_INSTALL_DIR};$ENV{VULKAN_LOADER_INSTALL_DIR})
        find_package(Vulkan)
        set (LIBVK "Vulkan::Vulkan")
    else()
        # TODO: Delete this code block when we're done with REPO_ROOT
        message(STATUS "Using user-supplied LOADER_REPO_ROOT to locate Vulkan")
        message(STATUS "DO NOT USE LOADER_REPO_ROOT!  See BUILD.md for information on using VULKAN_LOADER_INSTALL_DIR.")
        if(WIN32)
            set (LOADER_SEARCH_PATHS
                "${LOADER_REPO_ROOT}/${BUILDTGT_DIR}/loader/${DEBUG_DECORATION}"
                "${LOADER_REPO_ROOT}/${BUILDTGT_DIR}/loader/${RELEASE_DECORATION}"
                "${LOADER_REPO_ROOT}/Lib"
                )
        elseif(UNIX)
            set (LOADER_SEARCH_PATHS
                "${LOADER_REPO_ROOT}/${BUILDTGT_DIR}/loader"
                "${LOADER_REPO_ROOT}/x86_64/lib"
                )
        endif()
        find_library(LIBVK NAMES vulkan vulkan-1
            HINTS ${LOADER_SEARCH_PATHS}
            )
        if (LIBVK)
            message(STATUS "Found Vulkan: ${LIBVK}")
        endif()
    endif()

    add_executable(vk_layer_validation_tests layer_validation_tests.cpp ../layers/vk_format_utils.cpp ${COMMON_CPP})
    set_target_properties(vk_layer_validation_tests
       PROPERTIES
       COMPILE_DEFINITIONS "GTEST_LINKED_AS_SHARED_LIBRARY=1")
    target_include_directories(vk_layer_validation_tests PUBLIC
        ${VulkanHeaders_INCLUDE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${GTEST_LOCATION}/googletest/include
        ${PROJECT_SOURCE_DIR}/layers
        ${GLSLANG_SPIRV_INCLUDE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_BINARY_DIR}
        ${PROJECT_BINARY_DIR}
        ${PROJECT_BINARY_DIR}/layers
        )
    add_dependencies(vk_layer_validation_tests VkLayer_utils)

    if(NOT WIN32)
        set_target_properties(vk_layer_validation_tests
            PROPERTIES
            COMPILE_FLAGS "-Wno-sign-compare")
        if (BUILD_WSI_XCB_SUPPORT OR BUILD_WSI_XLIB_SUPPORT)
            target_link_libraries(vk_layer_validation_tests ${LIBVK} ${XCB_LIBRARIES} ${X11_LIBRARIES} gtest gtest_main ${GLSLANG_LIBRARIES})
        else()
            target_link_libraries(vk_layer_validation_tests ${LIBVK} gtest gtest_main ${GLSLANG_LIBRARIES})
        endif()
    endif()
    if(WIN32)
        target_link_libraries(vk_layer_validation_tests ${LIBVK} gtest gtest_main ${GLSLANG_LIBRARIES})
    endif()

    if (WIN32)
        # For Windows, copy necessary gtest DLLs to the right spot for the vk_layer_tests...
        FILE(TO_NATIVE_PATH ${PROJECT_BINARY_DIR}/external/googletest/googletest/$<CONFIGURATION>/*.dll SRC_GTEST_DLLS)
        FILE(TO_NATIVE_PATH ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION> DST_GTEST_DLLS)
        add_custom_command(TARGET vk_layer_validation_tests POST_BUILD
            COMMAND xcopy /Y /I ${SRC_GTEST_DLLS} ${DST_GTEST_DLLS})
    endif()

    add_subdirectory(layers)
endif()
