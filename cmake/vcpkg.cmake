include(FetchContent)
FetchContent_Declare(vcpkg 
    GIT_REPOSITORY https://github.com/microsoft/vcpkg.git
    GIT_TAG 2022.09.27
)
FetchContent_MakeAvailable(vcpkg)

# https://github.com/microsoft/vcpkg/blob/master/docs/users/buildsystems/cmake-integration.md

set(VCPKG_OVERLAY_PORTS "")
list(APPEND VCPKG_OVERLAY_PORTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/ports/glslang")
list(APPEND VCPKG_OVERLAY_PORTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/ports/spirv-headers")
list(APPEND VCPKG_OVERLAY_PORTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/ports/spirv-tools")
list(APPEND VCPKG_OVERLAY_PORTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/ports/vulkan-headers")

# Sets the minimum macOS version for compiled binaries. This also changes what versions of
# the macOS platform SDK that CMake will search for. See the CMake documentation for CMAKE_OSX_DEPLOYMENT_TARGET for more information.
set(VCPKG_OSX_DEPLOYMENT_TARGET "10.12" CACHE STRING "Minimum OS X deployment version")

set(CMAKE_TOOLCHAIN_FILE "${FETCHCONTENT_BASE_DIR}/vcpkg-src/scripts/buildsystems/vcpkg.cmake" CACHE FILEPATH "")
