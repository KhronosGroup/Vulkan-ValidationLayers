#.rst:
# FindVulkanHeaders
# -----------------
#
# Try to find Vulkan Headers and Registry.
#
# This module is intended to be used by projects that build Vulkan
# "system" components such as the loader and layers.
# Vulkan applications should instead use the FindVulkan (or similar)
# find module that locates the headers and the loader library.
#
# When using this find module to locate the headers and registry
# in a Vulkan-Headers repository, the Vulkan-Headers repository
# should be built with 'install' target and the following environment
# or CMake variable set to the location of the install directory.
#
#    VULKAN_HEADERS_INSTALL_DIR
#
# IMPORTED Targets
# ^^^^^^^^^^^^^^^^
#
# This module defines no IMPORTED targets
#
# Result Variables
# ^^^^^^^^^^^^^^^^
#
# This module defines the following variables::
#
#   VulkanHeaders_FOUND          - True if VulkanHeaders was found
#   VulkanHeaders_INCLUDE_DIRS   - include directories for VulkanHeaders
#
#   VulkanRegistry_FOUND         - True if VulkanRegistry was found
#   VulkanRegistry_DIRS          - directories for VulkanRegistry
#
# The module will also define two cache variables::
#
#   VulkanHeaders_INCLUDE_DIR    - the VulkanHeaders include directory
#   VulkanRegistry_DIR           - the VulkanRegistry directory
#

# Probe command-line arguments and the environment to see if they specify the
# Vulkan headers installation path.
if(NOT DEFINED VULKAN_HEADERS_INSTALL_DIR)
  if (DEFINED ENV{VULKAN_HEADERS_INSTALL_DIR})
    set(VULKAN_HEADERS_INSTALL_DIR "$ENV{VULKAN_HEADERS_INSTALL_DIR}")
  elseif(DEFINED ENV{VULKAN_SDK})
    set(VULKAN_HEADERS_INSTALL_DIR "$ENV{VULKAN_SDK}")
  endif()
endif()

if(DEFINED VULKAN_HEADERS_INSTALL_DIR)
  # When CMAKE_FIND_ROOT_PATH_INCLUDE is set to ONLY, the HINTS in find_path()
  # are re-rooted, which prevents VULKAN_HEADERS_INSTALL_DIR to work as
  # expected. So use NO_CMAKE_FIND_ROOT_PATH to avoid it.

  # Use HINTS instead of PATH to search these locations before
  # searching system environment variables like $PATH that may
  # contain SDK directories.
  find_path(VulkanHeaders_INCLUDE_DIR
      NAMES vulkan/vulkan.h
      HINTS ${VULKAN_HEADERS_INSTALL_DIR}/include
      NO_CMAKE_FIND_ROOT_PATH)
  find_path(VulkanRegistry_DIR
      NAMES vk.xml
      HINTS ${VULKAN_HEADERS_INSTALL_DIR}/share/vulkan/registry
      NO_CMAKE_FIND_ROOT_PATH)
else()
  # If VULKAN_HEADERS_INSTALL_DIR, or one of its variants was not specified,
  # do a normal search without hints.
  find_path(VulkanHeaders_INCLUDE_DIR NAMES vulkan/vulkan.h)
  get_filename_component(VULKAN_REGISTRY_PATH_HINT ${VulkanHeaders_INCLUDE_DIR} DIRECTORY)
  find_path(VulkanRegistry_DIR NAMES vk.xml HINTS ${VULKAN_REGISTRY_PATH_HINT})
endif()

set(VulkanHeaders_INCLUDE_DIRS ${VulkanHeaders_INCLUDE_DIR})
set(VulkanRegistry_DIRS ${VulkanRegistry_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(VulkanHeaders
    DEFAULT_MSG
    VulkanHeaders_INCLUDE_DIR)
set(FPHSA_NAME_MISMATCHED TRUE)
find_package_handle_standard_args(VulkanRegistry
    DEFAULT_MSG
    VulkanRegistry_DIR)
unset(FPHSA_NAME_MISMATCHED)

mark_as_advanced(VulkanHeaders_INCLUDE_DIR VulkanRegistry_DIR)
