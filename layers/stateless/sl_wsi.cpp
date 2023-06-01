/* Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (C) 2015-2023 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "stateless/stateless_validation.h"
#include "generated/enum_flag_bits.h"

bool StatelessValidation::manual_PreCallValidateAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout,
                                                                    VkSemaphore semaphore, VkFence fence,
                                                                    uint32_t *pImageIndex) const {
    bool skip = false;

    if (semaphore == VK_NULL_HANDLE && fence == VK_NULL_HANDLE) {
        skip |= LogError(swapchain, "VUID-vkAcquireNextImageKHR-semaphore-01780",
                         "vkAcquireNextImageKHR: semaphore and fence are both VK_NULL_HANDLE.");
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR *pAcquireInfo,
                                                                     uint32_t *pImageIndex) const {
    bool skip = false;

    if (pAcquireInfo->semaphore == VK_NULL_HANDLE && pAcquireInfo->fence == VK_NULL_HANDLE) {
        skip |= LogError(pAcquireInfo->swapchain, "VUID-VkAcquireNextImageInfoKHR-semaphore-01782",
                         "vkAcquireNextImage2KHR: pAcquireInfo->semaphore and pAcquireInfo->fence are both VK_NULL_HANDLE.");
    }

    return skip;
}

bool StatelessValidation::ValidateSwapchainCreateInfo(const char *func_name, VkSwapchainCreateInfoKHR const *pCreateInfo) const {
    bool skip = false;

    if (pCreateInfo != nullptr) {
        // Validation for parameters excluded from the generated validation code due to a 'noautovalidity' tag in vk.xml
        if (pCreateInfo->imageSharingMode == VK_SHARING_MODE_CONCURRENT) {
            // If imageSharingMode is VK_SHARING_MODE_CONCURRENT, queueFamilyIndexCount must be greater than 1
            if (pCreateInfo->queueFamilyIndexCount <= 1) {
                skip |= LogError(device, "VUID-VkSwapchainCreateInfoKHR-imageSharingMode-01278",
                                 "%s: if pCreateInfo->imageSharingMode is VK_SHARING_MODE_CONCURRENT, "
                                 "pCreateInfo->queueFamilyIndexCount must be greater than 1.",
                                 func_name);
            }

            // If imageSharingMode is VK_SHARING_MODE_CONCURRENT, pQueueFamilyIndices must be a pointer to an array of
            // queueFamilyIndexCount uint32_t values
            if (pCreateInfo->pQueueFamilyIndices == nullptr) {
                skip |= LogError(device, "VUID-VkSwapchainCreateInfoKHR-imageSharingMode-01277",
                                 "%s: if pCreateInfo->imageSharingMode is VK_SHARING_MODE_CONCURRENT, "
                                 "pCreateInfo->pQueueFamilyIndices must be a pointer to an array of "
                                 "pCreateInfo->queueFamilyIndexCount uint32_t values.",
                                 func_name);
            }
        }

        skip |= ValidateGreaterThanZero(pCreateInfo->imageArrayLayers, "pCreateInfo->imageArrayLayers",
                                        "VUID-VkSwapchainCreateInfoKHR-imageArrayLayers-01275", func_name);

        // Validate VK_KHR_image_format_list VkImageFormatListCreateInfo
        const auto format_list_info = LvlFindInChain<VkImageFormatListCreateInfo>(pCreateInfo->pNext);
        if (format_list_info) {
            const uint32_t viewFormatCount = format_list_info->viewFormatCount;
            if (((pCreateInfo->flags & VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR) == 0) && (viewFormatCount > 1)) {
                skip |= LogError(device, "VUID-VkSwapchainCreateInfoKHR-flags-04100",
                                 "%s: If the VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR is not set, then "
                                 "VkImageFormatListCreateInfo::viewFormatCount (%" PRIu32
                                 ") must be 0 or 1 if it is in the pNext chain.",
                                 func_name, viewFormatCount);
            }

            // Using the first format, compare the rest of the formats against it that they are compatible
            for (uint32_t i = 1; i < viewFormatCount; i++) {
                if (FormatCompatibilityClass(format_list_info->pViewFormats[0]) !=
                    FormatCompatibilityClass(format_list_info->pViewFormats[i])) {
                    skip |= LogError(device, "VUID-VkSwapchainCreateInfoKHR-pNext-04099",
                                     "%s: VkImageFormatListCreateInfo::pViewFormats[0] (%s) and "
                                     "VkImageFormatListCreateInfo::pViewFormats[%" PRIu32
                                     "] (%s) are not compatible in the pNext chain.",
                                     func_name, string_VkFormat(format_list_info->pViewFormats[0]), i,
                                     string_VkFormat(format_list_info->pViewFormats[i]));
                }
            }
        }

        // Validate VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR
        if ((pCreateInfo->flags & VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR) != 0) {
            if (!IsExtEnabled(device_extensions.vk_khr_swapchain_mutable_format)) {
                skip |= LogError(device, kVUID_PVError_ExtensionNotEnabled,
                                 "%s: pCreateInfo->flags contains VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR which requires the "
                                 "VK_KHR_swapchain_mutable_format extension, which has not been enabled.",
                                 func_name);
            } else {
                if (format_list_info == nullptr) {
                    skip |= LogError(
                        device, "VUID-VkSwapchainCreateInfoKHR-flags-03168",
                        "%s: pCreateInfo->flags contains VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR but the pNext chain of "
                        "pCreateInfo does not contain an instance of VkImageFormatListCreateInfo.",
                        func_name);
                } else if (format_list_info->viewFormatCount == 0) {
                    skip |= LogError(
                        device, "VUID-VkSwapchainCreateInfoKHR-flags-03168",
                        "%s: pCreateInfo->flags contains VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR but the viewFormatCount "
                        "member of VkImageFormatListCreateInfo in the pNext chain is zero.",
                        func_name);
                } else {
                    bool found_base_format = false;
                    for (uint32_t i = 0; i < format_list_info->viewFormatCount; ++i) {
                        if (format_list_info->pViewFormats[i] == pCreateInfo->imageFormat) {
                            found_base_format = true;
                            break;
                        }
                    }
                    if (!found_base_format) {
                        skip |=
                            LogError(device, "VUID-VkSwapchainCreateInfoKHR-flags-03168",
                                     "%s: pCreateInfo->flags contains VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR but none of the "
                                     "elements of the pViewFormats member of VkImageFormatListCreateInfo match "
                                     "pCreateInfo->imageFormat.",
                                     func_name);
                    }
                }
            }
        }
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR *pCreateInfo,
                                                                   const VkAllocationCallbacks *pAllocator,
                                                                   VkSwapchainKHR *pSwapchain) const {
    bool skip = false;
    skip |= ValidateSwapchainCreateInfo("vkCreateSwapchainKHR()", pCreateInfo);
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount,
                                                                          const VkSwapchainCreateInfoKHR *pCreateInfos,
                                                                          const VkAllocationCallbacks *pAllocator,
                                                                          VkSwapchainKHR *pSwapchains) const {
    bool skip = false;
    if (pCreateInfos) {
        for (uint32_t i = 0; i < swapchainCount; i++) {
            std::stringstream func_name;
            func_name << "vkCreateSharedSwapchainsKHR[" << swapchainCount << "]()";
            skip |= ValidateSwapchainCreateInfo(func_name.str().c_str(), &pCreateInfos[i]);
        }
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR *pPresentInfo) const {
    bool skip = false;

    if (pPresentInfo && pPresentInfo->pNext) {
        const auto *present_regions = LvlFindInChain<VkPresentRegionsKHR>(pPresentInfo->pNext);
        if (present_regions) {
            // TODO: This and all other pNext extension dependencies should be added to code-generation
            if (!IsExtEnabled(device_extensions.vk_khr_incremental_present)) {
                skip |= LogError(device, kVUID_PVError_ExtensionNotEnabled,
                                 "vkQueuePresentKHR() called even though the %s extension was not enabled for this VkDevice.",
                                 VK_KHR_INCREMENTAL_PRESENT_EXTENSION_NAME);
            }

            if (present_regions->swapchainCount != pPresentInfo->swapchainCount) {
                skip |= LogError(device, "VUID-VkPresentRegionsKHR-swapchainCount-01260",
                                 "QueuePresentKHR(): pPresentInfo->swapchainCount has a value of %i but VkPresentRegionsKHR "
                                 "extension swapchainCount is %i. These values must be equal.",
                                 pPresentInfo->swapchainCount, present_regions->swapchainCount);
            }
            skip |= ValidateStructPnext("QueuePresentKHR", "pCreateInfo->pNext->pNext", NULL, present_regions->pNext, 0, NULL,
                                        GeneratedVulkanHeaderVersion, "VUID-VkPresentInfoKHR-pNext-pNext",
                                        "VUID-VkPresentInfoKHR-sType-unique");
        }
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCreateDisplayModeKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                                     const VkDisplayModeCreateInfoKHR *pCreateInfo,
                                                                     const VkAllocationCallbacks *pAllocator,
                                                                     VkDisplayModeKHR *pMode) const {
    bool skip = false;

    const VkDisplayModeParametersKHR display_mode_parameters = pCreateInfo->parameters;
    if (display_mode_parameters.visibleRegion.width == 0) {
        skip |= LogError(device, "VUID-VkDisplayModeParametersKHR-width-01990",
                         "vkCreateDisplayModeKHR(): pCreateInfo->parameters.visibleRegion.width must be greater than 0.");
    }
    if (display_mode_parameters.visibleRegion.height == 0) {
        skip |= LogError(device, "VUID-VkDisplayModeParametersKHR-height-01991",
                         "vkCreateDisplayModeKHR(): pCreateInfo->parameters.visibleRegion.height must be greater than 0.");
    }
    if (display_mode_parameters.refreshRate == 0) {
        skip |= LogError(device, "VUID-VkDisplayModeParametersKHR-refreshRate-01992",
                         "vkCreateDisplayModeKHR(): pCreateInfo->parameters.refreshRate must be greater than 0.");
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice,
                                                                                   VkSurfaceKHR surface,
                                                                                   uint32_t *pSurfaceFormatCount,
                                                                                   VkSurfaceFormatKHR *pSurfaceFormats) const {
    bool skip = false;
    if (surface == VK_NULL_HANDLE && !instance_extensions.vk_google_surfaceless_query) {
        skip |= LogError(
            physicalDevice, "VUID-vkGetPhysicalDeviceSurfaceFormatsKHR-surface-06524",
            "vkGetPhysicalDeviceSurfaceFormatsKHR(): surface is VK_NULL_HANDLE and VK_GOOGLE_surfaceless_query is not enabled.");
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice,
                                                                                        VkSurfaceKHR surface,
                                                                                        uint32_t *pPresentModeCount,
                                                                                        VkPresentModeKHR *pPresentModes) const {
    bool skip = false;
    if (surface == VK_NULL_HANDLE && !instance_extensions.vk_google_surfaceless_query) {
        skip |= LogError(
            physicalDevice, "VUID-vkGetPhysicalDeviceSurfacePresentModesKHR-surface-06524",
            "vkGetPhysicalDeviceSurfacePresentModesKHR: surface is VK_NULL_HANDLE and VK_GOOGLE_surfaceless_query is not enabled.");
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateGetPhysicalDeviceSurfaceCapabilities2KHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo,
    VkSurfaceCapabilities2KHR *pSurfaceCapabilities) const {
    bool skip = false;
    if (pSurfaceInfo && pSurfaceInfo->surface == VK_NULL_HANDLE && !instance_extensions.vk_google_surfaceless_query) {
        skip |= LogError(physicalDevice, "VUID-vkGetPhysicalDeviceSurfaceCapabilities2KHR-pSurfaceInfo-06520",
                         "vkGetPhysicalDeviceSurfaceCapabilities2KHR: pSurfaceInfo->surface is VK_NULL_HANDLE and "
                         "VK_GOOGLE_surfaceless_query is not enabled.");
    }
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    const auto *capabilities_full_screen_exclusive =
        LvlFindInChain<VkSurfaceCapabilitiesFullScreenExclusiveEXT>(pSurfaceCapabilities->pNext);
    if (capabilities_full_screen_exclusive) {
        const auto *full_screen_exclusive_win32_info =
            LvlFindInChain<VkSurfaceFullScreenExclusiveWin32InfoEXT>(pSurfaceInfo->pNext);
        if (!full_screen_exclusive_win32_info) {
            skip |= LogError(device, "VUID-vkGetPhysicalDeviceSurfaceCapabilities2KHR-pNext-02671",
                             "vkGetPhysicalDeviceSurfaceCapabilities2KHR(): pSurfaceCapabilities->pNext contains "
                             "VkSurfaceCapabilitiesFullScreenExclusiveEXT, but pSurfaceInfo->pNext does not contain "
                             "VkSurfaceFullScreenExclusiveWin32InfoEXT");
        }
    }
#endif

    if (IsExtEnabled(device_extensions.vk_ext_surface_maintenance1)) {
        const auto *surface_present_mode_compatibility =
            LvlFindInChain<VkSurfacePresentModeCompatibilityEXT>(pSurfaceCapabilities->pNext);
        const auto *surface_present_scaling_compatibilities =
            LvlFindInChain<VkSurfacePresentScalingCapabilitiesEXT>(pSurfaceCapabilities->pNext);

        if (!(LvlFindInChain<VkSurfacePresentModeEXT>(pSurfaceInfo->pNext))) {
            if (surface_present_mode_compatibility) {
                skip |= LogError(device, "VUID-vkGetPhysicalDeviceSurfaceCapabilities2KHR-pNext-07776",
                                 "vkGetPhysicalDeviceSurfaceCapabilities2KHR(): VK_EXT_surface_maintenance1 is enabled and "
                                 "pSurfaceCapabilities->pNext contains VkSurfacePresentModeCompatibilityEXT, but "
                                 "pSurfaceInfo->pNext does not contain a VkSurfacePresentModeEXT structure.");
            }

            if (surface_present_scaling_compatibilities) {
                skip |= LogError(device, "VUID-vkGetPhysicalDeviceSurfaceCapabilities2KHR-pNext-07777",
                                 "vkGetPhysicalDeviceSurfaceCapabilities2KHR(): VK_EXT_surface_maintenance1 is enabled and "
                                 "pSurfaceCapabilities->pNext contains VkSurfacePresentScalingCapabilitiesEXT, but "
                                 "pSurfaceInfo->pNext does not contain a VkSurfacePresentModeEXT structure.");
            }
        }

        if (IsExtEnabled(instance_extensions.vk_google_surfaceless_query)) {
            if (pSurfaceInfo->surface == VK_NULL_HANDLE) {
                if (surface_present_mode_compatibility) {
                    skip |=
                        LogError(physicalDevice, "VUID-vkGetPhysicalDeviceSurfaceCapabilities2KHR-pNext-07778",
                                 "vkGetPhysicalDeviceSurfaceCapabilities2KHR: VK_EXT_surface_maintenance1 and "
                                 "VK_GOOGLE_surfaceless_query are enabled and pSurfaceCapabilities->pNext contains a "
                                 "VkSurfacePresentModeCompatibilityEXT structure, but pSurfaceInfo->surface is VK_NULL_HANDLE.");
                }

                if (surface_present_scaling_compatibilities) {
                    skip |=
                        LogError(physicalDevice, "VUID-vkGetPhysicalDeviceSurfaceCapabilities2KHR-pNext-07779",
                                 "vkGetPhysicalDeviceSurfaceCapabilities2KHR: VK_EXT_surface_maintenance1 and "
                                 "VK_GOOGLE_surfaceless_query are enabled and pSurfaceCapabilities->pNext contains a "
                                 "VkSurfacePresentScalingCapabilitiesEXT structure, but pSurfaceInfo->surface is VK_NULL_HANDLE.");
                }
            }
        }
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateGetPhysicalDeviceSurfaceFormats2KHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo, uint32_t *pSurfaceFormatCount,
    VkSurfaceFormat2KHR *pSurfaceFormats) const {
    bool skip = false;
    if (pSurfaceInfo && pSurfaceInfo->surface == VK_NULL_HANDLE && !instance_extensions.vk_google_surfaceless_query) {
        skip |= LogError(physicalDevice, "VUID-vkGetPhysicalDeviceSurfaceFormats2KHR-pSurfaceInfo-06521",
                         "vkGetPhysicalDeviceSurfaceFormats2KHR: pSurfaceInfo->surface is VK_NULL_HANDLE and "
                         "VK_GOOGLE_surfaceless_query is not enabled.");
    }
    return skip;
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
bool StatelessValidation::manual_PreCallValidateGetPhysicalDeviceSurfacePresentModes2EXT(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo, uint32_t *pPresentModeCount,
    VkPresentModeKHR *pPresentModes) const {
    bool skip = false;
    if (pSurfaceInfo && pSurfaceInfo->surface == VK_NULL_HANDLE && !instance_extensions.vk_google_surfaceless_query) {
        skip |= LogError(physicalDevice, "VUID-vkGetPhysicalDeviceSurfacePresentModes2EXT-pSurfaceInfo-06521",
                         "vkGetPhysicalDeviceSurfacePresentModes2EXT: pSurfaceInfo->surface is VK_NULL_HANDLE and "
                         "VK_GOOGLE_surfaceless_query is not enabled.");
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCreateWin32SurfaceKHR(VkInstance instance,
                                                                      const VkWin32SurfaceCreateInfoKHR *pCreateInfo,
                                                                      const VkAllocationCallbacks *pAllocator,
                                                                      VkSurfaceKHR *pSurface) const {
    bool skip = false;

    if (pCreateInfo->hwnd == nullptr) {
        skip |= LogError(device, "VUID-VkWin32SurfaceCreateInfoKHR-hwnd-01308", "vkCreateWin32SurfaceKHR(): hwnd is NULL.");
    }

    return skip;
}

bool StatelessValidation::PreCallValidateGetDeviceGroupSurfacePresentModes2EXT(VkDevice device,
                                                                               const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo,
                                                                               VkDeviceGroupPresentModeFlagsKHR *pModes) const {
    bool skip = false;
    if (!IsExtEnabled(device_extensions.vk_khr_swapchain))
        skip |= OutputExtensionError("vkGetDeviceGroupSurfacePresentModes2EXT", VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    if (!IsExtEnabled(device_extensions.vk_khr_get_surface_capabilities2))
        skip |= OutputExtensionError("vkGetDeviceGroupSurfacePresentModes2EXT", VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);
    if (!IsExtEnabled(device_extensions.vk_khr_surface))
        skip |= OutputExtensionError("vkGetDeviceGroupSurfacePresentModes2EXT", VK_KHR_SURFACE_EXTENSION_NAME);
    if (!IsExtEnabled(device_extensions.vk_khr_get_physical_device_properties2))
        skip |=
            OutputExtensionError("vkGetDeviceGroupSurfacePresentModes2EXT", VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    if (!IsExtEnabled(device_extensions.vk_ext_full_screen_exclusive))
        skip |= OutputExtensionError("vkGetDeviceGroupSurfacePresentModes2EXT", VK_EXT_FULL_SCREEN_EXCLUSIVE_EXTENSION_NAME);
    skip |= ValidateStructType(
        "vkGetDeviceGroupSurfacePresentModes2EXT", "pSurfaceInfo", "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR",
        pSurfaceInfo, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR, true,
        "VUID-vkGetDeviceGroupSurfacePresentModes2EXT-pSurfaceInfo-parameter", "VUID-VkPhysicalDeviceSurfaceInfo2KHR-sType-sType");
    if (pSurfaceInfo != NULL) {
        constexpr std::array allowed_structs = {VK_STRUCTURE_TYPE_SURFACE_FULL_SCREEN_EXCLUSIVE_INFO_EXT,
                                                VK_STRUCTURE_TYPE_SURFACE_FULL_SCREEN_EXCLUSIVE_WIN32_INFO_EXT};

        skip |= ValidateStructPnext("vkGetDeviceGroupSurfacePresentModes2EXT", "pSurfaceInfo->pNext",
                                    "VkSurfaceFullScreenExclusiveInfoEXT, VkSurfaceFullScreenExclusiveWin32InfoEXT",
                                    pSurfaceInfo->pNext, allowed_structs.size(), allowed_structs.data(),
                                    GeneratedVulkanHeaderVersion, "VUID-VkPhysicalDeviceSurfaceInfo2KHR-pNext-pNext",
                                    "VUID-VkPhysicalDeviceSurfaceInfo2KHR-sType-unique");

        if (pSurfaceInfo->surface == VK_NULL_HANDLE && !instance_extensions.vk_google_surfaceless_query) {
            skip |= LogError(device, "VUID-vkGetPhysicalDeviceSurfacePresentModes2EXT-pSurfaceInfo-06521",
                             "vkGetPhysicalDeviceSurfacePresentModes2EXT: pSurfaceInfo->surface is VK_NULL_HANDLE and "
                             "VK_GOOGLE_surfaceless_query is not enabled.");
        }

        skip |= ValidateRequiredHandle("vkGetDeviceGroupSurfacePresentModes2EXT", "pSurfaceInfo->surface", pSurfaceInfo->surface);
    }
    return skip;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
bool StatelessValidation::manual_PreCallValidateCreateWaylandSurfaceKHR(VkInstance instance,
                                                                        const VkWaylandSurfaceCreateInfoKHR *pCreateInfo,
                                                                        const VkAllocationCallbacks *pAllocator,
                                                                        VkSurfaceKHR *pSurface) const {
    bool skip = false;

    const auto display = pCreateInfo->display;
    const auto surface = pCreateInfo->surface;

    if (display == nullptr) {
        skip |= LogError(device, "VUID-VkWaylandSurfaceCreateInfoKHR-display-01304", "vkCreateWaylandSurfaceKHR: display is NULL!");
    }

    if (surface == nullptr) {
        skip |= LogError(device, "VUID-VkWaylandSurfaceCreateInfoKHR-surface-01305", "vkCreateWaylandSurfaceKHR: surface is NULL!");
    }

    return skip;
}
#endif  // VK_USE_PLATFORM_WAYLAND_KHR

#ifdef VK_USE_PLATFORM_XCB_KHR
bool StatelessValidation::manual_PreCallValidateCreateXcbSurfaceKHR(VkInstance instance,
                                                                    const VkXcbSurfaceCreateInfoKHR *pCreateInfo,
                                                                    const VkAllocationCallbacks *pAllocator,
                                                                    VkSurfaceKHR *pSurface) const {
    bool skip = false;

    const auto connection = pCreateInfo->connection;
    const auto window = pCreateInfo->window;

    if (connection == nullptr) {
        skip |= LogError(device, "VUID-VkXcbSurfaceCreateInfoKHR-connection-01310", "vkCreateXcbSurfaceKHR: connection is NULL!");
    }

    if (window == 0) {
        skip |= LogError(device, "VUID-VkXcbSurfaceCreateInfoKHR-window-01311", "vkCreateXcbSurfaceKHR: window is 0!");
    }

    return skip;
}
#endif  // VK_USE_PLATFORM_XCB_KHR

#ifdef VK_USE_PLATFORM_XLIB_KHR
bool StatelessValidation::manual_PreCallValidateCreateXlibSurfaceKHR(VkInstance instance,
                                                                     const VkXlibSurfaceCreateInfoKHR *pCreateInfo,
                                                                     const VkAllocationCallbacks *pAllocator,
                                                                     VkSurfaceKHR *pSurface) const {
    bool skip = false;

    const auto display = pCreateInfo->dpy;
    const auto window = pCreateInfo->window;

    if (display == nullptr) {
        skip |= LogError(device, "VUID-VkXlibSurfaceCreateInfoKHR-dpy-01313", "vkCreateXlibSurfaceKHR: dpy is NULL!");
    }

    if (window == 0) {
        skip |= LogError(device, "VUID-VkXlibSurfaceCreateInfoKHR-window-01314", "vkCreateXlibSurfaceKHR: window is 0!");
    }

    return skip;
}
#endif  // VK_USE_PLATFORM_XLIB_KHR
