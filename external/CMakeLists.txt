# googletest is an external dependency for the tests in the ValidationLayers
# repo. Check for its presence before trying to build tests.

# Suppress all warnings from external projects.
set_property(DIRECTORY APPEND PROPERTY COMPILE_OPTIONS -w)

if(TARGET gtest_main)
    message(STATUS "Google Test (googletest) already configured - use it")
elseif(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/googletest)
    SET(BUILD_GTEST ON CACHE BOOL "Builds the googletest subproject")
    SET(BUILD_GMOCK OFF CACHE BOOL "Builds the googlemock subproject")
    SET(gtest_force_shared_crt ON CACHE BOOL "Link gtest runtimes dynamically")
    SET(BUILD_SHARED_LIBS ON CACHE BOOL "Build shared libraries")
    # EXCLUDE_FROM_ALL keeps the install target from installing GTEST files.
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/googletest EXCLUDE_FROM_ALL)
else()
    message(STATUS
        "Google Test was not found - tests based on that will not build")
endif()
