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
                                                                    VkSemaphore semaphore, VkFence fence, uint32_t *pImageIndex,
                                                                    const ErrorObject &error_obj) const {
    bool skip = false;

    if (semaphore == VK_NULL_HANDLE && fence == VK_NULL_HANDLE) {
        skip |= LogError("VUID-vkAcquireNextImageKHR-semaphore-01780", swapchain, error_obj.location,
                         "semaphore and fence are both VK_NULL_HANDLE.");
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR *pAcquireInfo,
                                                                     uint32_t *pImageIndex, const ErrorObject &error_obj) const {
    bool skip = false;

    if (pAcquireInfo->semaphore == VK_NULL_HANDLE && pAcquireInfo->fence == VK_NULL_HANDLE) {
        skip |= LogError("VUID-VkAcquireNextImageInfoKHR-semaphore-01782", pAcquireInfo->swapchain,
                         error_obj.location.dot(Field::pAcquireInfo), "semaphore and fence are both VK_NULL_HANDLE.");
    }

    return skip;
}

bool StatelessValidation::ValidateSwapchainCreateInfo(VkSwapchainCreateInfoKHR const *pCreateInfo, const Location &loc) const {
    bool skip = false;

    if (pCreateInfo != nullptr) {
        // Validation for parameters excluded from the generated validation code due to a 'noautovalidity' tag in vk.xml
        if (pCreateInfo->imageSharingMode == VK_SHARING_MODE_CONCURRENT) {
            // If imageSharingMode is VK_SHARING_MODE_CONCURRENT, queueFamilyIndexCount must be greater than 1
            if (pCreateInfo->queueFamilyIndexCount <= 1) {
                skip |= LogError("VUID-VkSwapchainCreateInfoKHR-imageSharingMode-01278", device, loc.dot(Field::imageSharingMode),
                                 "is VK_SHARING_MODE_CONCURRENT, but queueFamilyIndexCount is %" PRIu32 ".",
                                 pCreateInfo->queueFamilyIndexCount);
            }

            // If imageSharingMode is VK_SHARING_MODE_CONCURRENT, pQueueFamilyIndices must be a pointer to an array of
            // queueFamilyIndexCount uint32_t values
            if (pCreateInfo->pQueueFamilyIndices == nullptr) {
                skip |= LogError("VUID-VkSwapchainCreateInfoKHR-imageSharingMode-01277", device, loc.dot(Field::imageSharingMode),
                                 "is VK_SHARING_MODE_CONCURRENT, but pQueueFamilyIndices is NULL.");
            }
        }

        skip |= ValidateNotZero(pCreateInfo->imageArrayLayers == 0, "VUID-VkSwapchainCreateInfoKHR-imageArrayLayers-01275",
                                loc.dot(Field::imageArrayLayers));

        // Validate VK_KHR_image_format_list VkImageFormatListCreateInfo
        const auto format_list_info = vku::FindStructInPNextChain<VkImageFormatListCreateInfo>(pCreateInfo->pNext);
        if (format_list_info) {
            const uint32_t viewFormatCount = format_list_info->viewFormatCount;
            if (((pCreateInfo->flags & VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR) == 0) && (viewFormatCount > 1)) {
                skip |= LogError("VUID-VkSwapchainCreateInfoKHR-flags-04100", device,
                                 loc.pNext(Struct::VkImageFormatListCreateInfo, Field::viewFormatCount),
                                 "is %" PRIu32 " but flag (%s) does not includes VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR.",
                                 viewFormatCount, string_VkImageCreateFlags(pCreateInfo->flags).c_str());
            }

            // Using the first format, compare the rest of the formats against it that they are compatible
            for (uint32_t i = 1; i < viewFormatCount; i++) {
                if (vkuFormatCompatibilityClass(format_list_info->pViewFormats[0]) !=
                    vkuFormatCompatibilityClass(format_list_info->pViewFormats[i])) {
                    skip |= LogError("VUID-VkSwapchainCreateInfoKHR-pNext-04099", device,
                                     loc.pNext(Struct::VkImageFormatListCreateInfo, Field::pViewFormats, i),
                                     "(%s) and pViewFormats[0] (%s) are not compatible in the pNext chain.",
                                     string_VkFormat(format_list_info->pViewFormats[i]),
                                     string_VkFormat(format_list_info->pViewFormats[0]));
                }
            }
        }

        // Validate VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR
        if ((pCreateInfo->flags & VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR) != 0) {
            if (!IsExtEnabled(device_extensions.vk_khr_swapchain_mutable_format)) {
                skip |= LogError(kVUID_PVError_ExtensionNotEnabled, device, loc.dot(Field::flags),
                                 "includes VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR which requires the "
                                 "VK_KHR_swapchain_mutable_format extension, which has not been enabled.");
            } else {
                if (format_list_info == nullptr) {
                    skip |= LogError("VUID-VkSwapchainCreateInfoKHR-flags-03168", device, loc.dot(Field::flags),
                                     "includes VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR but the pNext does not contain "
                                     "VkImageFormatListCreateInfo.");
                } else if (format_list_info->viewFormatCount == 0) {
                    skip |= LogError("VUID-VkSwapchainCreateInfoKHR-flags-03168", device, loc.dot(Field::flags),
                                     "includes VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR but %s is zero.",
                                     loc.pNext(Struct::VkImageFormatListCreateInfo, Field::viewFormatCount).Fields().c_str());
                } else {
                    bool found_base_format = false;
                    for (uint32_t i = 0; i < format_list_info->viewFormatCount; ++i) {
                        if (format_list_info->pViewFormats[i] == pCreateInfo->imageFormat) {
                            found_base_format = true;
                            break;
                        }
                    }
                    if (!found_base_format) {
                        skip |= LogError("VUID-VkSwapchainCreateInfoKHR-flags-03168", device, loc.dot(Field::flags),
                                         "includes VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR but none of the "
                                         "elements of the pViewFormats member of VkImageFormatListCreateInfo match "
                                         "imageFormat (%s).",
                                         string_VkFormat(pCreateInfo->imageFormat));
                    }
                }
            }
        }
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR *pCreateInfo,
                                                                   const VkAllocationCallbacks *pAllocator,
                                                                   VkSwapchainKHR *pSwapchain, const ErrorObject &error_obj) const {
    bool skip = false;
    skip |= ValidateSwapchainCreateInfo(pCreateInfo, error_obj.location.dot(Field::pCreateInfo));
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount,
                                                                          const VkSwapchainCreateInfoKHR *pCreateInfos,
                                                                          const VkAllocationCallbacks *pAllocator,
                                                                          VkSwapchainKHR *pSwapchains,
                                                                          const ErrorObject &error_obj) const {
    bool skip = false;
    if (pCreateInfos) {
        for (uint32_t i = 0; i < swapchainCount; i++) {
            skip |= ValidateSwapchainCreateInfo(&pCreateInfos[i], error_obj.location.dot(Field::pCreateInfos, i));
        }
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR *pPresentInfo,
                                                                const ErrorObject &error_obj) const {
    bool skip = false;

    if (pPresentInfo && pPresentInfo->pNext) {
        const auto *present_regions = vku::FindStructInPNextChain<VkPresentRegionsKHR>(pPresentInfo->pNext);
        if (present_regions) {
            // TODO: This and all other pNext extension dependencies should be added to code-generation
            if (!IsExtEnabled(device_extensions.vk_khr_incremental_present)) {
                skip |= LogError(kVUID_PVError_ExtensionNotEnabled, device, error_obj.location, "%s extension was not enabled.",
                                 VK_KHR_INCREMENTAL_PRESENT_EXTENSION_NAME);
            }

            if (present_regions->swapchainCount != pPresentInfo->swapchainCount) {
                skip |= LogError("VUID-VkPresentRegionsKHR-swapchainCount-01260", device,
                                 error_obj.location.pNext(Struct::VkPresentRegionsKHR, Field::swapchainCount),
                                 "(%" PRIu32 ") is not equal to %s (%" PRIu32 ").", present_regions->swapchainCount,
                                 error_obj.location.dot(Field::pPresentInfo).dot(Field::swapchainCount).Fields().c_str(),
                                 pPresentInfo->swapchainCount);
            }
            skip |= ValidateStructPnext(error_obj.location.pNext(Struct::VkPresentRegionsKHR), present_regions->pNext, 0, nullptr,
                                        GeneratedVulkanHeaderVersion, "VUID-VkPresentInfoKHR-pNext-pNext",
                                        "VUID-VkPresentInfoKHR-sType-unique");
        }
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCreateDisplayModeKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                                     const VkDisplayModeCreateInfoKHR *pCreateInfo,
                                                                     const VkAllocationCallbacks *pAllocator,
                                                                     VkDisplayModeKHR *pMode, const ErrorObject &error_obj) const {
    bool skip = false;

    const VkDisplayModeParametersKHR display_mode_parameters = pCreateInfo->parameters;
    const Location create_info_loc = error_obj.location.dot(Field::pCreateInfo);
    const Location param_loc = create_info_loc.dot(Field::parameters);

    skip |= ValidateNotZero(display_mode_parameters.visibleRegion.width == 0, "VUID-VkDisplayModeParametersKHR-width-01990",
                            param_loc.dot(Field::visibleRegion).dot(Field::width));
    skip |= ValidateNotZero(display_mode_parameters.visibleRegion.height == 0, "VUID-VkDisplayModeParametersKHR-height-01991",
                            param_loc.dot(Field::visibleRegion).dot(Field::width));
    skip |= ValidateNotZero(display_mode_parameters.refreshRate == 0, "VUID-VkDisplayModeParametersKHR-refreshRate-01992",
                            param_loc.dot(Field::refreshRate));

    return skip;
}

bool StatelessValidation::manual_PreCallValidateGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice,
                                                                                   VkSurfaceKHR surface,
                                                                                   uint32_t *pSurfaceFormatCount,
                                                                                   VkSurfaceFormatKHR *pSurfaceFormats,
                                                                                   const ErrorObject &error_obj) const {
    bool skip = false;
    if (surface == VK_NULL_HANDLE && !instance_extensions.vk_google_surfaceless_query) {
        skip |=
            LogError("VUID-vkGetPhysicalDeviceSurfaceFormatsKHR-surface-06524", physicalDevice,
                     error_obj.location.dot(Field::surface), "is VK_NULL_HANDLE and VK_GOOGLE_surfaceless_query is not enabled.");
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice,
                                                                                        VkSurfaceKHR surface,
                                                                                        uint32_t *pPresentModeCount,
                                                                                        VkPresentModeKHR *pPresentModes,
                                                                                        const ErrorObject &error_obj) const {
    bool skip = false;
    if (surface == VK_NULL_HANDLE && !instance_extensions.vk_google_surfaceless_query) {
        skip |=
            LogError("VUID-vkGetPhysicalDeviceSurfacePresentModesKHR-surface-06524", physicalDevice,
                     error_obj.location.dot(Field::surface), "is VK_NULL_HANDLE and VK_GOOGLE_surfaceless_query is not enabled.");
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateGetPhysicalDeviceSurfaceCapabilities2KHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo,
    VkSurfaceCapabilities2KHR *pSurfaceCapabilities, const ErrorObject &error_obj) const {
    bool skip = false;
    if (pSurfaceInfo && pSurfaceInfo->surface == VK_NULL_HANDLE && !instance_extensions.vk_google_surfaceless_query) {
        skip |= LogError("VUID-vkGetPhysicalDeviceSurfaceCapabilities2KHR-pSurfaceInfo-06521", physicalDevice,
                         error_obj.location.dot(Field::pSurfaceInfo).dot(Field::surface),
                         "is VK_NULL_HANDLE and VK_GOOGLE_surfaceless_query is not enabled.");
    }
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    const auto *capabilities_full_screen_exclusive =
        vku::FindStructInPNextChain<VkSurfaceCapabilitiesFullScreenExclusiveEXT>(pSurfaceCapabilities->pNext);
    if (capabilities_full_screen_exclusive) {
        const auto *full_screen_exclusive_win32_info =
            vku::FindStructInPNextChain<VkSurfaceFullScreenExclusiveWin32InfoEXT>(pSurfaceInfo->pNext);
        if (!full_screen_exclusive_win32_info) {
            skip |= LogError("VUID-vkGetPhysicalDeviceSurfaceCapabilities2KHR-pNext-02671", physicalDevice, error_obj.location,
                             "pSurfaceCapabilities->pNext contains "
                             "VkSurfaceCapabilitiesFullScreenExclusiveEXT, but pSurfaceInfo->pNext does not contain "
                             "VkSurfaceFullScreenExclusiveWin32InfoEXT");
        }
    }
#endif

    const auto *surface_present_mode_compatibility =
        vku::FindStructInPNextChain<VkSurfacePresentModeCompatibilityEXT>(pSurfaceCapabilities->pNext);
    const auto *surface_present_scaling_compatibilities =
        vku::FindStructInPNextChain<VkSurfacePresentScalingCapabilitiesEXT>(pSurfaceCapabilities->pNext);

    if (!(vku::FindStructInPNextChain<VkSurfacePresentModeEXT>(pSurfaceInfo->pNext))) {
        if (surface_present_mode_compatibility) {
            skip |= LogError("VUID-vkGetPhysicalDeviceSurfaceCapabilities2KHR-pNext-07776", physicalDevice, error_obj.location,
                             "pSurfaceCapabilities->pNext contains VkSurfacePresentModeCompatibilityEXT, but "
                             "pSurfaceInfo->pNext does not contain a VkSurfacePresentModeEXT structure.");
        }

        if (surface_present_scaling_compatibilities) {
            skip |= LogError("VUID-vkGetPhysicalDeviceSurfaceCapabilities2KHR-pNext-07777", physicalDevice, error_obj.location,
                             "pSurfaceCapabilities->pNext contains VkSurfacePresentScalingCapabilitiesEXT, but "
                             "pSurfaceInfo->pNext does not contain a VkSurfacePresentModeEXT structure.");
        }
    }

    if (pSurfaceInfo->surface == VK_NULL_HANDLE) {
        if (surface_present_mode_compatibility) {
            skip |= LogError("VUID-vkGetPhysicalDeviceSurfaceCapabilities2KHR-pNext-07778", physicalDevice, error_obj.location,
                             "pSurfaceCapabilities->pNext contains a "
                             "VkSurfacePresentModeCompatibilityEXT structure, but pSurfaceInfo->surface is VK_NULL_HANDLE.");
        }

        if (surface_present_scaling_compatibilities) {
            skip |= LogError("VUID-vkGetPhysicalDeviceSurfaceCapabilities2KHR-pNext-07779", physicalDevice, error_obj.location,
                             "pSurfaceCapabilities->pNext contains a "
                             "VkSurfacePresentScalingCapabilitiesEXT structure, but pSurfaceInfo->surface is VK_NULL_HANDLE.");
        }
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateGetPhysicalDeviceSurfaceFormats2KHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo, uint32_t *pSurfaceFormatCount,
    VkSurfaceFormat2KHR *pSurfaceFormats, const ErrorObject &error_obj) const {
    bool skip = false;
    if (pSurfaceInfo && pSurfaceInfo->surface == VK_NULL_HANDLE && !instance_extensions.vk_google_surfaceless_query) {
        skip |= LogError("VUID-vkGetPhysicalDeviceSurfaceFormats2KHR-pSurfaceInfo-06521", physicalDevice,
                         error_obj.location.dot(Field::pSurfaceInfo).dot(Field::surface),
                         "is VK_NULL_HANDLE and VK_GOOGLE_surfaceless_query is not enabled.");
    }
    return skip;
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
bool StatelessValidation::manual_PreCallValidateGetPhysicalDeviceSurfacePresentModes2EXT(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo, uint32_t *pPresentModeCount,
    VkPresentModeKHR *pPresentModes, const ErrorObject &error_obj) const {
    bool skip = false;
    if (pSurfaceInfo && pSurfaceInfo->surface == VK_NULL_HANDLE && !instance_extensions.vk_google_surfaceless_query) {
        skip |= LogError("VUID-vkGetPhysicalDeviceSurfacePresentModes2EXT-pSurfaceInfo-06521", physicalDevice,
                         error_obj.location.dot(Field::pSurfaceInfo).dot(Field::surface),
                         "is VK_NULL_HANDLE and VK_GOOGLE_surfaceless_query is not enabled.");
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCreateWin32SurfaceKHR(VkInstance instance,
                                                                      const VkWin32SurfaceCreateInfoKHR *pCreateInfo,
                                                                      const VkAllocationCallbacks *pAllocator,
                                                                      VkSurfaceKHR *pSurface, const ErrorObject &error_obj) const {
    bool skip = false;

    if (pCreateInfo->hwnd == nullptr) {
        skip |= LogError("VUID-VkWin32SurfaceCreateInfoKHR-hwnd-01308", instance, error_obj.location, "pCreateInfo->hwnd is NULL.");
    }

    return skip;
}

bool StatelessValidation::PreCallValidateGetDeviceGroupSurfacePresentModes2EXT(VkDevice device,
                                                                               const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo,
                                                                               VkDeviceGroupPresentModeFlagsKHR *pModes,
                                                                               const ErrorObject &error_obj) const {
    bool skip = false;
    if (!IsExtEnabled(device_extensions.vk_khr_swapchain))
        skip |= OutputExtensionError(error_obj.location, VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    if (!IsExtEnabled(device_extensions.vk_khr_get_surface_capabilities2))
        skip |= OutputExtensionError(error_obj.location, VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);
    if (!IsExtEnabled(device_extensions.vk_khr_surface))
        skip |= OutputExtensionError(error_obj.location, VK_KHR_SURFACE_EXTENSION_NAME);
    if (!IsExtEnabled(device_extensions.vk_khr_get_physical_device_properties2))
        skip |= OutputExtensionError(error_obj.location, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    if (!IsExtEnabled(device_extensions.vk_ext_full_screen_exclusive))
        skip |= OutputExtensionError(error_obj.location, VK_EXT_FULL_SCREEN_EXCLUSIVE_EXTENSION_NAME);
    skip |= ValidateStructType(error_obj.location.dot(Field::pSurfaceInfo), "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR",
                               pSurfaceInfo, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR, true,
                               "VUID-vkGetDeviceGroupSurfacePresentModes2EXT-pSurfaceInfo-parameter",
                               "VUID-VkPhysicalDeviceSurfaceInfo2KHR-sType-sType");
    if (pSurfaceInfo != NULL) {
        constexpr std::array allowed_structs = {VK_STRUCTURE_TYPE_SURFACE_FULL_SCREEN_EXCLUSIVE_INFO_EXT,
                                                VK_STRUCTURE_TYPE_SURFACE_FULL_SCREEN_EXCLUSIVE_WIN32_INFO_EXT};

        skip |= ValidateStructPnext(error_obj.location.dot(Field::pSurfaceInfo), pSurfaceInfo->pNext, allowed_structs.size(),
                                    allowed_structs.data(), GeneratedVulkanHeaderVersion,
                                    "VUID-VkPhysicalDeviceSurfaceInfo2KHR-pNext-pNext",
                                    "VUID-VkPhysicalDeviceSurfaceInfo2KHR-sType-unique");

        if (pSurfaceInfo->surface == VK_NULL_HANDLE && !instance_extensions.vk_google_surfaceless_query) {
            skip |= LogError("VUID-vkGetPhysicalDeviceSurfacePresentModes2EXT-pSurfaceInfo-06521", device,
                             error_obj.location.dot(Field::pSurfaceInfo).dot(Field::surface),
                             "is VK_NULL_HANDLE and VK_GOOGLE_surfaceless_query is not enabled.");
        }

        skip |= ValidateRequiredHandle(error_obj.location.dot(Field::pSurfaceInfo).dot(Field::surface), pSurfaceInfo->surface);
    }
    return skip;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
bool StatelessValidation::manual_PreCallValidateCreateWaylandSurfaceKHR(VkInstance instance,
                                                                        const VkWaylandSurfaceCreateInfoKHR *pCreateInfo,
                                                                        const VkAllocationCallbacks *pAllocator,
                                                                        VkSurfaceKHR *pSurface,
                                                                        const ErrorObject &error_obj) const {
    bool skip = false;

    const auto display = pCreateInfo->display;
    const auto surface = pCreateInfo->surface;

    if (display == nullptr) {
        skip |= LogError("VUID-VkWaylandSurfaceCreateInfoKHR-display-01304", instance,
                         error_obj.location.dot(Field::pCreateInfo).dot(Field::display), "is NULL!");
    }

    if (surface == nullptr) {
        skip |= LogError("VUID-VkWaylandSurfaceCreateInfoKHR-surface-01305", instance,
                         error_obj.location.dot(Field::pCreateInfo).dot(Field::surface), "is NULL!");
    }

    return skip;
}
#endif  // VK_USE_PLATFORM_WAYLAND_KHR

#ifdef VK_USE_PLATFORM_XCB_KHR
bool StatelessValidation::manual_PreCallValidateCreateXcbSurfaceKHR(VkInstance instance,
                                                                    const VkXcbSurfaceCreateInfoKHR *pCreateInfo,
                                                                    const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface,
                                                                    const ErrorObject &error_obj) const {
    bool skip = false;

    const auto connection = pCreateInfo->connection;
    const auto window = pCreateInfo->window;

    if (connection == nullptr) {
        skip |= LogError("VUID-VkXcbSurfaceCreateInfoKHR-connection-01310", instance,
                         error_obj.location.dot(Field::pCreateInfo).dot(Field::connection), "is NULL!");
    }

    skip |= ValidateNotZero(window == 0, "VUID-VkXcbSurfaceCreateInfoKHR-window-01311",
                            error_obj.location.dot(Field::pCreateInfo).dot(Field::window));

    return skip;
}
#endif  // VK_USE_PLATFORM_XCB_KHR

#ifdef VK_USE_PLATFORM_XLIB_KHR
bool StatelessValidation::manual_PreCallValidateCreateXlibSurfaceKHR(VkInstance instance,
                                                                     const VkXlibSurfaceCreateInfoKHR *pCreateInfo,
                                                                     const VkAllocationCallbacks *pAllocator,
                                                                     VkSurfaceKHR *pSurface, const ErrorObject &error_obj) const {
    bool skip = false;

    const auto display = pCreateInfo->dpy;
    const auto window = pCreateInfo->window;

    if (display == nullptr) {
        skip |= LogError("VUID-VkXlibSurfaceCreateInfoKHR-dpy-01313", instance,
                         error_obj.location.dot(Field::pCreateInfo).dot(Field::dpy), "is NULL!");
    }

    skip |= ValidateNotZero(window == 0, "VUID-VkXlibSurfaceCreateInfoKHR-window-01314",
                            error_obj.location.dot(Field::pCreateInfo).dot(Field::window));

    return skip;
}
#endif  // VK_USE_PLATFORM_XLIB_KHR
