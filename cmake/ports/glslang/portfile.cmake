vcpkg_check_linkage(ONLY_STATIC_LIBRARY)

vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO KhronosGroup/glslang
  REF 28b53119bdfbc43eae532641337a7adbb315b273
  SHA512 52039c54c75286499294bc03fcfa4ae1377e24924ba6fe31160baa3bbfdb3389437b0b68a7519e6d4df47b8bcecf553f07b8686c9f36efd280e5bd7a74100c8a
  HEAD_REF master
)

vcpkg_find_acquire_program(PYTHON3)
get_filename_component(PYTHON_PATH ${PYTHON3} DIRECTORY)
vcpkg_add_to_path("${PYTHON_PATH}")

if(VCPKG_TARGET_IS_IOS)
  # this case will report error since all executable will require BUNDLE DESTINATION
  set(BUILD_BINARIES OFF)
else()
  set(BUILD_BINARIES ON)  
endif()

vcpkg_cmake_configure(
  SOURCE_PATH "${SOURCE_PATH}"
  OPTIONS
    -DSKIP_GLSLANG_INSTALL=OFF
    -DBUILD_EXTERNAL=OFF
    -DENABLE_GLSLANG_BINARIES=${BUILD_BINARIES}
)

vcpkg_cmake_install()
vcpkg_cmake_config_fixup(CONFIG_PATH lib/cmake)

vcpkg_copy_pdbs()

if(NOT BUILD_BINARIES)
  file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/bin")
else()
  vcpkg_copy_tools(TOOL_NAMES glslangValidator spirv-remap AUTO_CLEAN)
endif()
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include"
                    "${CURRENT_PACKAGES_DIR}/debug/bin")

# Install custom usage
configure_file("${CMAKE_CURRENT_LIST_DIR}/usage" "${CURRENT_PACKAGES_DIR}/share/${PORT}/usage" @ONLY)

file(INSTALL "${SOURCE_PATH}/LICENSE.txt" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)
