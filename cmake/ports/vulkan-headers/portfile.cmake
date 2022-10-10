# header-only library
vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO KhronosGroup/Vulkan-Headers
    REF v1.3.230
    SHA512 1d94c220e6e0a274c57ffac09885d7154ed5b79aebd6018a20117d0f7907a5af875e697908b4cd4acdb9c4f6b0ff96f4c70b8d68dd2c1c2add90f41d85feaa2e
    HEAD_REF master
)

# This must be vulkan as other vulkan packages expect it there.
file(COPY "${SOURCE_PATH}/include/vulkan/" DESTINATION "${CURRENT_PACKAGES_DIR}/include/vulkan")
file(COPY "${SOURCE_PATH}/include/vk_video/" DESTINATION "${CURRENT_PACKAGES_DIR}/include/vk_video")
file(COPY "${SOURCE_PATH}/registry/" DESTINATION "${CURRENT_PACKAGES_DIR}/registry")

# Handle copyright
file(INSTALL "${SOURCE_PATH}/LICENSE.txt" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)
