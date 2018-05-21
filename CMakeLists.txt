# The name of our project is "VULKAN". CMakeLists files in this project can
# refer to the root source directory of the project as ${VULKAN_SOURCE_DIR} and
# to the root binary directory of the project as ${VULKAN_BINARY_DIR}.
cmake_minimum_required(VERSION 2.8.11)

# This must come before the project command.
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.12" CACHE STRING "Minimum OS X deployment version")

project (Vulkan-ValidationLayers)
# set (CMAKE_VERBOSE_MAKEFILE 1)

# The API_NAME allows renaming builds to avoid conflicts with installed SDKs
# The MAJOR number of the version we're building, used in naming
# <api-name>-<major>.dll (and other files).
set(API_NAME "Vulkan" CACHE STRING "API name to use when building")
set(MAJOR "1")
string(TOLOWER ${API_NAME} API_LOWERCASE)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(PythonInterp 3 REQUIRED)

# TODO: Add option to override this with env var, or even to use SDK
set(VULKAN_HEADERS_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/Vulkan-Headers")

# Output warning if vulkan headers submodule contents are not present
if (NOT EXISTS "${VULKAN_HEADERS_LOCATION}/include/vulkan/vulkan_core.h")
    message(FATAL_ERROR "Please run 'git submodule update --init' before running cmake")
endif()

option(USE_CCACHE "Use ccache" OFF)
if (USE_CCACHE)
    find_program(CCACHE_FOUND ccache)
    if(CCACHE_FOUND)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
    endif(CCACHE_FOUND)
endif()

if(APPLE)
    # CMake versions 3 or later need CMAKE_MACOSX_RPATH defined.
    # This avoids the CMP0042 policy message.
    set(CMAKE_MACOSX_RPATH 1)
    # The "install" target for MacOS fixes up bundles in place.
    set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR})
endif()

# Enable cmake folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(VVL_TARGET_FOLDER vl_cmake_targets)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(FALLBACK_CONFIG_DIRS "/etc/xdg" CACHE STRING
        "Search path to use when XDG_CONFIG_DIRS is unset or empty or the current process is SUID/SGID. Default is freedesktop compliant.")
    set(FALLBACK_DATA_DIRS "/usr/local/share:/usr/share" CACHE STRING
        "Search path to use when XDG_DATA_DIRS is unset or empty or the current process is SUID/SGID. Default is freedesktop compliant.")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    include(FindPkgConfig)
    option(BUILD_WSI_XCB_SUPPORT "Build XCB WSI support" ON)
    option(BUILD_WSI_XLIB_SUPPORT "Build Xlib WSI support" ON)
    option(BUILD_WSI_WAYLAND_SUPPORT "Build Wayland WSI support" ON)
    option(BUILD_WSI_MIR_SUPPORT "Build Mir WSI support" OFF)
    set(DEMOS_WSI_SELECTION "XCB" CACHE STRING "Select WSI target for demos (XCB, XLIB, WAYLAND, MIR, DISPLAY)")

    if (BUILD_WSI_XCB_SUPPORT)
        find_package(XCB REQUIRED)
    endif()

    if (BUILD_WSI_XLIB_SUPPORT)
        find_package(X11 REQUIRED)
    endif()

    if (BUILD_WSI_WAYLAND_SUPPORT)
        find_package(Wayland REQUIRED)
        include_directories(${WAYLAND_CLIENT_INCLUDE_DIR})
    endif()

    if (BUILD_WSI_MIR_SUPPORT)
        find_package(Mir REQUIRED)
    endif()
endif()

if (CMAKE_COMPILER_IS_GNUCC OR CMAKE_C_COMPILER_ID MATCHES "Clang")
    set(COMMON_COMPILE_FLAGS "-Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers")
    set(COMMON_COMPILE_FLAGS "${COMMON_COMPILE_FLAGS} -fno-strict-aliasing -fno-builtin-memcmp")

    # For GCC version 7.1 or greater, we need to disable the implicit fallthrough warning since
    # there's no consistent way to satisfy all compilers until they all accept the C++17 standard
    if (CMAKE_COMPILER_IS_GNUCC AND NOT (CMAKE_CXX_COMPILER_VERSION LESS 7.1))
        set(COMMON_COMPILE_FLAGS "${COMMON_COMPILE_FLAGS} -Wimplicit-fallthrough=0")
    endif()

    if (APPLE)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMMON_COMPILE_FLAGS}")
    else()
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 ${COMMON_COMPILE_FLAGS}")
    endif()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_COMPILE_FLAGS} -std=c++11 -fno-rtti")
    if (UNIX)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
    endif()
endif()

if(WIN32)
    # Treat warnings as errors
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/WX>")
    # Disable RTTI
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/GR->")
    # Warn about nested declarations
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/w34456>")
    # Warn about potentially uninitialized variables
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/w34701>")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/w34703>")
    # Warn about different indirection types.
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/w34057>")
    # Warn about signed/unsigned mismatch.
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/w34245>")
endif()

if(NOT WIN32)
    set (BUILDTGT_DIR build)
    set (BINDATA_DIR Bin)
    set (LIBSOURCE_DIR Lib)
else()
    # For Windows, since 32-bit and 64-bit items can co-exist, we build each in its own build directory.
    # 32-bit target data goes in build32, and 64-bit target data goes into build.  So, include/link the
    # appropriate data at build time.
    if (DISABLE_BUILDTGT_DIR_DECORATION)
        set (BUILDTGT_DIR "")
        set (BINDATA_DIR "")
        set (LIBSOURCE_DIR "")
    elseif (CMAKE_CL_64)
        set (BUILDTGT_DIR build)
        set (BINDATA_DIR Bin)
        set (LIBSOURCE_DIR Lib)
    else()
        set (BUILDTGT_DIR build32)
        set (BINDATA_DIR Bin32)
        set (LIBSOURCE_DIR Lib32)
    endif()
endif()

option(BUILD_TESTS "Build tests" ON)
option(BUILD_LAYERS "Build layers" ON)

set(VULKAN_SDK $ENV{VULKAN_SDK})

# set GLSLANG_INSTALL_DIR to define locations of glslang includes, libs, and binaries.
# This is the preferred method for building this repository.
set(GLSLANG_INSTALL_DIR "GLSLANG-NOTFOUND" CACHE PATH "Absolute path to a glslang install directory")

# Set GLSLANG_REPO_ROOT to locate glslang repository
# This should be deprecated in lieu of GLSANG_INSTALL_DIR, remove once users have switched to preferred solution
set(GLSLANG_REPO_ROOT "GLSLANG-NOTFOUND" CACHE PATH "Absolute path to glslang repository")

# Repo requires glslang and that the update_glslang_sources.py script was run in the indicated glslang repo
if (NOT GLSLANG_INSTALL_DIR AND NOT GLSLANG_REPO_ROOT)
    message(FATAL_ERROR "Must define location of glslang binaries -- see BUILD.md")
endif()

# Is user specifying a glslang install directory?
if (GLSLANG_INSTALL_DIR)
    message(STATUS "Using glslang install located at ${GLSLANG_INSTALL_DIR}")
    set(SPIRV_TOOLS_BINARY_ROOT "${GLSLANG_INSTALL_DIR}/lib" CACHE PATH "User defined path to the SPIRV-Tools binaries for this project")
    set(SPIRV_TOOLS_OPT_BINARY_ROOT "${GLSLANG_INSTALL_DIR}/lib" CACHE PATH "User defined path to the SPIRV-Tools-opt binaries for this project")
    set(GLSLANG_SPIRV_INCLUDE_DIR "${GLSLANG_INSTALL_DIR}/include" CACHE PATH "Path to glslang spirv headers")
    set(SPIRV_TOOLS_INCLUDE_DIR "${GLSLANG_INSTALL_DIR}/include" CACHE PATH "Path to spirv tools headers")

    set(GLSLANG_SEARCH_PATH "${GLSLANG_INSTALL_DIR}/lib")
    set(GLSLANG_DEBUG_SEARCH_PATH "${GLSLANG_INSTALL_DIR}/lib")
    set (SPIRV_TOOLS_SEARCH_PATH "${GLSLANG_INSTALL_DIR}/lib")
    set (SPIRV_TOOLS_DEBUG_SEARCH_PATH "${GLSLANG_INSTALL_DIR}/lib")
    set (SPIRV_TOOLS_OPT_SEARCH_PATH "${GLSLANG_INSTALL_DIR}/lib")
    set (SPIRV_TOOLS_OPT_DEBUG_SEARCH_PATH "${GLSLANG_INSTALL_DIR}/lib")
else()
    message(STATUS "Using glslang built in repository located at ${GLSLANG_REPO_ROOT}")

    # These paths assume that the update_glslang_sources.py script was run in the indicated glslang repo and the repo was built
    set(SPIRV_TOOLS_BINARY_ROOT "${GLSLANG_REPO_ROOT}/${BUILDTGT_DIR}/External/spirv-tools/source" CACHE PATH "User defined path to the SPIRV-Tools binaries for this project")
    set(SPIRV_TOOLS_OPT_BINARY_ROOT "${GLSLANG_REPO_ROOT}/${BUILDTGT_DIR}/External/spirv-tools/source/opt" CACHE PATH "User defined path to the SPIRV-Tools-opt binaries for this project")

    if(WIN32)
        set(GSLANG_FINAL_BINARY_PATH ${GLSLANG_REPO_ROOT}/${BUILDTGT_DIR})

        if(DISABLE_BUILD_PATH_DECORATION)
            set (DEBUG_DECORATION "")
            set (RELEASE_DECORATION "")
        else()
            set (DEBUG_DECORATION "Debug")
            set (RELEASE_DECORATION "Release")
        endif()

        # Take some steps to set up a variable pointing to the final glslang binaries given the variety of input options
        set (GLSLANG_SEARCH_PATH "${GSLANG_FINAL_BINARY_PATH}/glslang/${RELEASE_DECORATION}"
                                 "${GSLANG_FINAL_BINARY_PATH}/glslang/OSDependent/Windows/${RELEASE_DECORATION}"
                                 "${GSLANG_FINAL_BINARY_PATH}/hlsl/${RELEASE_DECORATION}"
                                 "${GSLANG_FINAL_BINARY_PATH}/OGLCompilersDLL/${RELEASE_DECORATION}"
                                 "${GSLANG_FINAL_BINARY_PATH}/SPIRV/${RELEASE_DECORATION}" )

        set (GLSLANG_DEBUG_SEARCH_PATH "${GSLANG_FINAL_BINARY_PATH}/glslang/${DEBUG_DECORATION}"
                                       "${GSLANG_FINAL_BINARY_PATH}/glslang/OSDependent/Windows/${DEBUG_DECORATION}"
                                       "${GSLANG_FINAL_BINARY_PATH}/hlsl/${DEBUG_DECORATION}"
                                       "${GSLANG_FINAL_BINARY_PATH}/OGLCompilersDLL/${DEBUG_DECORATION}"
                                       "${GSLANG_FINAL_BINARY_PATH}/SPIRV/${DEBUG_DECORATION}")

        set (SPIRV_TOOLS_SEARCH_PATH "${SPIRV_TOOLS_BINARY_ROOT}/${RELEASE_DECORATION}")
        set (SPIRV_TOOLS_DEBUG_SEARCH_PATH "${SPIRV_TOOLS_BINARY_ROOT}/${DEBUG_DECORATION}")
        set (SPIRV_TOOLS_OPT_SEARCH_PATH "${SPIRV_TOOLS_OPT_BINARY_ROOT}/${RELEASE_DECORATION}")
        set (SPIRV_TOOLS_OPT_DEBUG_SEARCH_PATH "${SPIRV_TOOLS_OPT_BINARY_ROOT}/${DEBUG_DECORATION}")
    else()
        # not WIN32
        set (GLSLANG_SEARCH_PATH "${GLSLANG_REPO_ROOT}/${BUILDTGT_DIR}/install/lib"
                                 "${GLSLANG_REPO_ROOT}/${BUILDTGT_DIR}/glslang"
                                 "${GLSLANG_REPO_ROOT}/${BUILDTGT_DIR}/glslang/OSDependent/Unix"
                                 "${GLSLANG_REPO_ROOT}/${BUILDTGT_DIR}/OGLCompilersDLL"
                                 "${GLSLANG_REPO_ROOT}/${BUILDTGT_DIR}/SPIRV"
                                 "${GLSLANG_REPO_ROOT}/${BUILDTGT_DIR}/hlsl"
                                 "${GLSLANG_REPO_ROOT}/${BUILDTGT_DIR}/StandAlone")

        set (SPIRV_TOOLS_SEARCH_PATH "${SPIRV_TOOLS_BINARY_ROOT}" )
        set (SPIRV_TOOLS_OPT_SEARCH_PATH "${SPIRV_TOOLS_OPT_BINARY_ROOT}" )
    endif()
    find_path(GLSLANG_SPIRV_INCLUDE_DIR SPIRV/spirv.hpp HINTS
             "${GLSLANG_REPO_ROOT}"
             "${CMAKE_SOURCE_DIR}/../glslang"
             DOC "Path to SPIRV/spirv.hpp")

    find_path(SPIRV_TOOLS_INCLUDE_DIR spirv-tools/libspirv.h HINTS
              "${GLSLANG_REPO_ROOT}/External/spirv-tools/include"
              "${CMAKE_SOURCE_DIR}/../glslang/External/spirv-tools/include"
              DOC "Path to spirv-tools/libspirv.h")
endif()

find_library(GLSLANG_LIB NAMES glslang
             HINTS ${GLSLANG_SEARCH_PATH} )

find_library(OGLCompiler_LIB NAMES OGLCompiler
             HINTS ${GLSLANG_SEARCH_PATH} )

find_library(OSDependent_LIB NAMES OSDependent
             HINTS ${GLSLANG_SEARCH_PATH} )

find_library(HLSL_LIB NAMES HLSL
             HINTS ${GLSLANG_SEARCH_PATH} )

find_library(SPIRV_LIB NAMES SPIRV
             HINTS ${GLSLANG_SEARCH_PATH} )

find_library(SPIRV_REMAPPER_LIB NAMES SPVRemapper
             HINTS ${GLSLANG_SEARCH_PATH} )

find_library(SPIRV_TOOLS_LIB NAMES SPIRV-Tools
             HINTS ${SPIRV_TOOLS_SEARCH_PATH} )

find_library(SPIRV_TOOLS_OPT_LIB NAMES SPIRV-Tools-opt
             HINTS ${SPIRV_TOOLS_OPT_SEARCH_PATH} )

if (WIN32)
    add_library(glslang         STATIC IMPORTED)
    add_library(OGLCompiler     STATIC IMPORTED)
    add_library(OSDependent     STATIC IMPORTED)
    add_library(HLSL            STATIC IMPORTED)
    add_library(SPIRV           STATIC IMPORTED)
    add_library(SPVRemapper     STATIC IMPORTED)
    add_library(Loader          STATIC IMPORTED)
    add_library(SPIRV-Tools-opt STATIC IMPORTED)
    add_library(SPIRV-Tools     STATIC IMPORTED)

    find_library(GLSLANG_DLIB NAMES glslangd
                 HINTS ${GLSLANG_DEBUG_SEARCH_PATH} )
    find_library(OGLCompiler_DLIB NAMES OGLCompilerd
                 HINTS ${GLSLANG_DEBUG_SEARCH_PATH} )
    find_library(OSDependent_DLIB NAMES OSDependentd
                 HINTS ${GLSLANG_DEBUG_SEARCH_PATH} )
    find_library(HLSL_DLIB NAMES HLSLd
                 HINTS ${GLSLANG_DEBUG_SEARCH_PATH} )
    find_library(SPIRV_DLIB NAMES SPIRVd
                 HINTS ${GLSLANG_DEBUG_SEARCH_PATH} )
    find_library(SPIRV_REMAPPER_DLIB NAMES SPVRemapperd
                 HINTS ${GLSLANG_DEBUG_SEARCH_PATH} )
    find_library(SPIRV_TOOLS_DLIB NAMES SPIRV-Toolsd
                 HINTS ${SPIRV_TOOLS_DEBUG_SEARCH_PATH} )
    find_library(SPIRV_TOOLS_OPT_DLIB NAMES SPIRV-Tools-optd
                 HINTS ${SPIRV_TOOLS_OPT_DEBUG_SEARCH_PATH} )
    set_target_properties(glslang PROPERTIES
                         IMPORTED_LOCATION       "${GLSLANG_LIB}"
                         IMPORTED_LOCATION_DEBUG "${GLSLANG_DLIB}")
    set_target_properties(OGLCompiler PROPERTIES
                         IMPORTED_LOCATION       "${OGLCompiler_LIB}"
                         IMPORTED_LOCATION_DEBUG "${OGLCompiler_DLIB}")
    set_target_properties(OSDependent PROPERTIES
                         IMPORTED_LOCATION       "${OSDependent_LIB}"
                         IMPORTED_LOCATION_DEBUG "${OSDependent_DLIB}")
    set_target_properties(HLSL PROPERTIES
                         IMPORTED_LOCATION       "${HLSL_LIB}"
                         IMPORTED_LOCATION_DEBUG "${HLSL_DLIB}")
    set_target_properties(SPIRV PROPERTIES
                         IMPORTED_LOCATION       "${SPIRV_LIB}"
                         IMPORTED_LOCATION_DEBUG "${SPIRV_DLIB}")
    set_target_properties(SPVRemapper PROPERTIES
                         IMPORTED_LOCATION       "${SPIRV_REMAPPER_LIB}"
                         IMPORTED_LOCATION_DEBUG "${SPIRV_REMAPPER_DLIB}")
    set_target_properties(SPIRV-Tools PROPERTIES
                         IMPORTED_LOCATION       "${SPIRV_TOOLS_LIB}"
                         IMPORTED_LOCATION_DEBUG "${SPIRV_TOOLS_DLIB}")
    set_target_properties(SPIRV-Tools-opt PROPERTIES
                         IMPORTED_LOCATION       "${SPIRV_TOOLS_OPT_LIB}"
                         IMPORTED_LOCATION_DEBUG "${SPIRV_TOOLS_OPT_DLIB}")

endif()

if(WIN32)
    set (SPIRV_TOOLS_LIBRARIES SPIRV-Tools-opt SPIRV-Tools)
    set (GLSLANG_LIBRARIES glslang OGLCompiler OSDependent HLSL SPIRV SPVRemapper ${SPIRV_TOOLS_LIBRARIES})
else ()
    set (SPIRV_TOOLS_LIBRARIES ${SPIRV_TOOLS_OPT_LIB} ${SPIRV_TOOLS_LIB})
    set (GLSLANG_LIBRARIES ${GLSLANG_LIB} ${OGLCompiler_LIB} ${OSDependent_LIB} ${HLSL_LIB} ${SPIRV_LIB} ${SPIRV_REMAPPER_LIB} ${SPIRV_TOOLS_LIBRARIES})
endif()

set (PYTHON_CMD ${PYTHON_EXECUTABLE})

if(NOT WIN32)
    include(GNUInstallDirs)

    add_definitions(-DFALLBACK_CONFIG_DIRS="${FALLBACK_CONFIG_DIRS}")
    add_definitions(-DFALLBACK_DATA_DIRS="${FALLBACK_DATA_DIRS}")
    add_definitions(-DSYSCONFDIR="${CMAKE_INSTALL_FULL_SYSCONFDIR}")

    # Make sure /etc is searched by the loader
    if(NOT (CMAKE_INSTALL_FULL_SYSCONFDIR STREQUAL "/etc"))
        add_definitions(-DEXTRASYSCONFDIR="/etc")
    endif()
endif()

if(UNIX)
    # uninstall target
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
        IMMEDIATE @ONLY)

    add_custom_target(uninstall
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()

add_definitions(-DAPI_NAME="${API_NAME}")

if(BUILD_TESTS)
    # Output warning if googletest submodule contents are not present
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/googletest/googletest/include/gtest/gtest.h")
        message(FATAL_ERROR "Please run 'git submodule update --init' before running cmake")
    endif()
    add_subdirectory(tests)
endif()

if(BUILD_LAYERS)
    add_subdirectory(layers)
endif()
