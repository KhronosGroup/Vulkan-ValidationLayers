# Generate a CMake file with all information that may be 
# needed by external projects we build with update_deps.py

set(vvl_cmake_info "")

# Ensure the same generator is used
list(APPEND vvl_cmake_info "CMAKE_GENERATOR")

# Ensure the same toolchain file is used
list(APPEND vvl_cmake_info "CMAKE_TOOLCHAIN_FILE")

foreach(lang C CXX ASM)
    # Ensure flags passed via command line are used
    list(APPEND vvl_cmake_info "CMAKE_${lang}_FLAGS")
    # Ensure the same compilers are used
    list(APPEND vvl_cmake_info "CMAKE_${lang}_COMPILER")
endforeach()

if (APPLE)
    # Ensure the deployment target is consistent to avoid linker warnings
    list(APPEND vvl_cmake_info "CMAKE_OSX_DEPLOYMENT_TARGET")
    # Ensure the same system root is used
    list(APPEND vvl_cmake_info "CMAKE_OSX_SYSROOT")
endif()

if (MSVC_IDE)
    # Ensure the same platform is targeted
    list(APPEND vvl_cmake_info "CMAKE_GENERATOR_PLATFORM")
    # Ensure the same toolset is used
    list(APPEND vvl_cmake_info "CMAKE_GENERATOR_TOOLSET")
endif()

# NOTE: This files timestamp will be updated anytime CMake is run,
# making it problematic to pass to update_deps, this is addressed by configure_file,
# as suggessted by the CMake docs.
set(_initial_cache "${CMAKE_CURRENT_BINARY_DIR}/.vvl/_env.cmake")
file(WRITE ${_initial_cache} "message(STATUS \"VVL: Setting cache variables\")\n")

foreach(var IN LISTS vvl_cmake_info)
    # Ignore variables that were never defined
    # EX: Often CMAKE_TOOLCHAIN_FILE isn't defined
    if (NOT DEFINED ${var})
        continue()
    endif()

    # EX: If var is CMAKE_GENERATOR, value is Ninja
    set(value ${${var}})

    # Ignore values that were never defined
    # EX: Often CMAKE_CXX_FLAGS has no value
    if ("${value}" STREQUAL "")
        continue()
    endif()

    file(APPEND ${_initial_cache} "set(${var} ${value} CACHE INTERNAL \"\" FORCE) \n")
endforeach()

set(VVL_INITIAL_CACHE "${CMAKE_CURRENT_BINARY_DIR}/.vvl/g_inital_cache.cmake")

# Use the configure_file() command to update the file only when its content changes.
configure_file(${_initial_cache} ${VVL_INITIAL_CACHE} COPYONLY)
