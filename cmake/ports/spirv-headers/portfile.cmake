
vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO KhronosGroup/SPIRV-Headers
    REF 87d5b782bec60822aa878941e6b13c0a9a954c9b
    SHA512 d6ce02e53c259e508d1d72d81cc6aa6b3019e7ecd6a8878d81d8681d9734756f66c762ebd4b8b1d0f9fbb7a8f1f18d72aeb27c56822d810aca5a3e53c51c1ef6
    HEAD_REF master
)

# This must be spirv as other spirv packages expect it there.
file(COPY "${SOURCE_PATH}/include/spirv/" DESTINATION "${CURRENT_PACKAGES_DIR}/include/spirv")

# Handle copyright
file(INSTALL "${SOURCE_PATH}/LICENSE" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)
