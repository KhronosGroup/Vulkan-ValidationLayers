/* Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
 * Modifications Copyright (C) 2022 RasterGrid Kft.
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

#include "best_practices/best_practices_validation.h"
#include "best_practices/best_practices_error_enums.h"

bool BestPractices::ValidateGetPhysicalDeviceDisplayPlanePropertiesKHRQuery(VkPhysicalDevice physicalDevice,
                                                                            const char* api_name) const {
    bool skip = false;
    const auto bp_pd_state = Get<bp_state::PhysicalDevice>(physicalDevice);

    if (bp_pd_state) {
        if (bp_pd_state->vkGetPhysicalDeviceDisplayPlanePropertiesKHRState == UNCALLED) {
            skip |= LogWarning(physicalDevice, kVUID_BestPractices_DisplayPlane_PropertiesNotCalled,
                               "Potential problem with calling %s() without first retrieving properties from "
                               "vkGetPhysicalDeviceDisplayPlanePropertiesKHR or vkGetPhysicalDeviceDisplayPlaneProperties2KHR.",
                               api_name);
        }
    }

    return skip;
}

bool BestPractices::PreCallValidateGetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice physicalDevice, uint32_t planeIndex,
                                                                       uint32_t* pDisplayCount, VkDisplayKHR* pDisplays) const {
    bool skip = false;

    skip |= ValidateGetPhysicalDeviceDisplayPlanePropertiesKHRQuery(physicalDevice, "vkGetDisplayPlaneSupportedDisplaysKHR");

    return skip;
}

bool BestPractices::PreCallValidateGetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkDisplayModeKHR mode,
                                                                  uint32_t planeIndex,
                                                                  VkDisplayPlaneCapabilitiesKHR* pCapabilities) const {
    bool skip = false;

    skip |= ValidateGetPhysicalDeviceDisplayPlanePropertiesKHRQuery(physicalDevice, "vkGetDisplayPlaneCapabilitiesKHR");

    return skip;
}

bool BestPractices::PreCallValidateGetDisplayPlaneCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                                   const VkDisplayPlaneInfo2KHR* pDisplayPlaneInfo,
                                                                   VkDisplayPlaneCapabilities2KHR* pCapabilities) const {
    bool skip = false;

    skip |= ValidateGetPhysicalDeviceDisplayPlanePropertiesKHRQuery(physicalDevice, "vkGetDisplayPlaneCapabilities2KHR");

    return skip;
}

bool BestPractices::PreCallValidateGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount,
                                                         VkImage* pSwapchainImages) const {
    bool skip = false;

    auto swapchain_state = Get<bp_state::Swapchain>(swapchain);

    if (swapchain_state && pSwapchainImages) {
        // Compare the preliminary value of *pSwapchainImageCount with the value this time:
        if (swapchain_state->vkGetSwapchainImagesKHRState == UNCALLED) {
            skip |=
                LogWarning(device, kVUID_Core_Swapchain_PriorCount,
                           "vkGetSwapchainImagesKHR() called with non-NULL pSwapchainImageCount; but no prior positive value has "
                           "been seen for pSwapchainImages.");
        }

        if (*pSwapchainImageCount > swapchain_state->get_swapchain_image_count) {
            skip |= LogWarning(
                device, kVUID_BestPractices_Swapchain_InvalidCount,
                "vkGetSwapchainImagesKHR() called with non-NULL pSwapchainImages, and with pSwapchainImageCount set to a "
                "value (%" PRId32 ") that is greater than the value (%" PRId32
                ") that was returned when pSwapchainImages was NULL.",
                *pSwapchainImageCount, swapchain_state->get_swapchain_image_count);
        }
    }

    return skip;
}

bool BestPractices::PreCallValidateCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain) const {
    bool skip = false;

    const auto* bp_pd_state = GetPhysicalDeviceState();
    if (bp_pd_state) {
        if (bp_pd_state->vkGetPhysicalDeviceSurfaceCapabilitiesKHRState == UNCALLED) {
            skip |= LogWarning(device, kVUID_BestPractices_Swapchain_GetSurfaceNotCalled,
                               "vkCreateSwapchainKHR() called before getting surface capabilities from "
                               "vkGetPhysicalDeviceSurfaceCapabilitiesKHR().");
        }

        if ((pCreateInfo->presentMode != VK_PRESENT_MODE_FIFO_KHR) &&
            (bp_pd_state->vkGetPhysicalDeviceSurfacePresentModesKHRState != QUERY_DETAILS)) {
            skip |= LogWarning(device, kVUID_BestPractices_Swapchain_GetSurfaceNotCalled,
                               "vkCreateSwapchainKHR() called before getting surface present mode(s) from "
                               "vkGetPhysicalDeviceSurfacePresentModesKHR().");
        }

        if (bp_pd_state->vkGetPhysicalDeviceSurfaceFormatsKHRState != QUERY_DETAILS) {
            skip |= LogWarning(
                device, kVUID_BestPractices_Swapchain_GetSurfaceNotCalled,
                "vkCreateSwapchainKHR() called before getting surface format(s) from vkGetPhysicalDeviceSurfaceFormatsKHR().");
        }
    }

    if ((pCreateInfo->queueFamilyIndexCount > 1) && (pCreateInfo->imageSharingMode == VK_SHARING_MODE_EXCLUSIVE)) {
        skip |=
            LogWarning(device, kVUID_BestPractices_SharingModeExclusive,
                       "Warning: A Swapchain is being created which specifies a sharing mode of VK_SHARING_MODE_EXCLUSIVE while "
                       "specifying multiple queues (queueFamilyIndexCount of %" PRIu32 ").",
                       pCreateInfo->queueFamilyIndexCount);
    }

    const auto present_mode = pCreateInfo->presentMode;
    if (((present_mode == VK_PRESENT_MODE_MAILBOX_KHR) || (present_mode == VK_PRESENT_MODE_FIFO_KHR)) &&
        (pCreateInfo->minImageCount == 2)) {
        skip |= LogPerformanceWarning(
            device, kVUID_BestPractices_SuboptimalSwapchainImageCount,
            "Warning: A Swapchain is being created with minImageCount set to %" PRIu32
            ", which means double buffering is going "
            "to be used. Using double buffering and vsync locks rendering to an integer fraction of the vsync rate. In turn, "
            "reducing the performance of the application if rendering is slower than vsync. Consider setting minImageCount to "
            "3 to use triple buffering to maximize performance in such cases.",
            pCreateInfo->minImageCount);
    }

    if (VendorCheckEnabled(kBPVendorArm) && (pCreateInfo->presentMode != VK_PRESENT_MODE_FIFO_KHR)) {
        skip |= LogWarning(device, kVUID_BestPractices_CreateSwapchain_PresentMode,
                           "%s Warning: Swapchain is not being created with presentation mode \"VK_PRESENT_MODE_FIFO_KHR\". "
                           "Prefer using \"VK_PRESENT_MODE_FIFO_KHR\" to avoid unnecessary CPU and GPU load and save power. "
                           "Presentation modes which are not FIFO will present the latest available frame and discard other "
                           "frame(s) if any.",
                           VendorSpecificTag(kBPVendorArm));
    }

    return skip;
}

bool BestPractices::PreCallValidateCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount,
                                                             const VkSwapchainCreateInfoKHR* pCreateInfos,
                                                             const VkAllocationCallbacks* pAllocator,
                                                             VkSwapchainKHR* pSwapchains) const {
    bool skip = false;

    for (uint32_t i = 0; i < swapchainCount; i++) {
        if ((pCreateInfos[i].queueFamilyIndexCount > 1) && (pCreateInfos[i].imageSharingMode == VK_SHARING_MODE_EXCLUSIVE)) {
            skip |= LogWarning(
                device, kVUID_BestPractices_SharingModeExclusive,
                "Warning: A shared swapchain (index %" PRIu32
                ") is being created which specifies a sharing mode of VK_SHARING_MODE_EXCLUSIVE while specifying multiple "
                "queues (queueFamilyIndexCount of %" PRIu32 ").",
                i, pCreateInfos[i].queueFamilyIndexCount);
        }
    }

    return skip;
}

void BestPractices::ManualPostCallRecordQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo, VkResult result) {
    for (uint32_t i = 0; i < pPresentInfo->swapchainCount; ++i) {
        auto swapchains_result = pPresentInfo->pResults ? pPresentInfo->pResults[i] : result;
        if (swapchains_result == VK_SUBOPTIMAL_KHR) {
            LogPerformanceWarning(
                pPresentInfo->pSwapchains[i], kVUID_BestPractices_SuboptimalSwapchain,
                "vkQueuePresentKHR: %s :VK_SUBOPTIMAL_KHR was returned. VK_SUBOPTIMAL_KHR - Presentation will still succeed, "
                "subject to the window resize behavior, but the swapchain is no longer configured optimally for the surface it "
                "targets. Applications should query updated surface information and recreate their swapchain at the next "
                "convenient opportunity.",
                report_data->FormatHandle(pPresentInfo->pSwapchains[i]).c_str());
        }
    }

    // AMD best practice
    // end-of-frame cleanup
    num_queue_submissions_ = 0;
    num_barriers_objects_ = 0;
    ClearPipelinesUsedInFrame();
}

bool BestPractices::PreCallValidateGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                      uint32_t* pSurfaceFormatCount,
                                                                      VkSurfaceFormatKHR* pSurfaceFormats) const {
    if (!pSurfaceFormats) return false;
    const auto bp_pd_state = Get<bp_state::PhysicalDevice>(physicalDevice);
    const auto& call_state = bp_pd_state->vkGetPhysicalDeviceSurfaceFormatsKHRState;
    bool skip = false;
    if (call_state == UNCALLED) {
        // Since we haven't recorded a preliminary value of *pSurfaceFormatCount, that likely means that the application didn't
        // previously call this function with a NULL value of pSurfaceFormats:
        skip |= LogWarning(physicalDevice, kVUID_BestPractices_DevLimit_MustQueryCount,
                           "vkGetPhysicalDeviceSurfaceFormatsKHR() called with non-NULL pSurfaceFormatCount; but no prior "
                           "positive value has been seen for pSurfaceFormats.");
    } else {
        if (*pSurfaceFormatCount > bp_pd_state->surface_formats_count) {
            skip |= LogWarning(physicalDevice, kVUID_BestPractices_DevLimit_CountMismatch,
                               "vkGetPhysicalDeviceSurfaceFormatsKHR() called with non-NULL pSurfaceFormatCount, and with "
                               "pSurfaceFormats set to a value (%u) that is greater than the value (%u) that was returned "
                               "when pSurfaceFormatCount was NULL.",
                               *pSurfaceFormatCount, bp_pd_state->surface_formats_count);
        }
    }
    return skip;
}

bool BestPractices::PreCallValidateQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo) const {
    bool skip = false;

    if (VendorCheckEnabled(kBPVendorAMD) || VendorCheckEnabled(kBPVendorNVIDIA)) {
        auto num = num_queue_submissions_.load();
        if (num > kNumberOfSubmissionWarningLimitAMD) {
            skip |= LogPerformanceWarning(device, kVUID_BestPractices_Submission_ReduceNumberOfSubmissions,
                                          "%s %s Performance warning: command buffers submitted %" PRId32
                                          " times this frame. Submitting command buffers has a CPU "
                                          "and GPU overhead. Submit fewer times to incur less overhead.",
                                          VendorSpecificTag(kBPVendorAMD), VendorSpecificTag(kBPVendorNVIDIA), num);
        }
    }

    return skip;
}

bool BestPractices::PreCallValidateAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout,
                                                       VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex) const {
    auto swapchain_data = Get<SWAPCHAIN_NODE>(swapchain);
    bool skip = false;
    if (swapchain_data && swapchain_data->images.size() == 0) {
        skip |= LogWarning(swapchain, kVUID_BestPractices_DrawState_SwapchainImagesNotFound,
                           "vkAcquireNextImageKHR: No images found to acquire from. Application probably did not call "
                           "vkGetSwapchainImagesKHR after swapchain creation.");
    }
    return skip;
}

void BestPractices::ManualPostCallRecordGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice,
                                                                                VkSurfaceKHR surface,
                                                                                VkSurfaceCapabilitiesKHR* pSurfaceCapabilities,
                                                                                VkResult result) {
    auto bp_pd_state = Get<bp_state::PhysicalDevice>(physicalDevice);
    if (bp_pd_state) {
        bp_pd_state->vkGetPhysicalDeviceSurfaceCapabilitiesKHRState = QUERY_DETAILS;
    }
}

void BestPractices::ManualPostCallRecordGetPhysicalDeviceSurfaceCapabilities2KHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
    VkSurfaceCapabilities2KHR* pSurfaceCapabilities, VkResult result) {
    auto bp_pd_state = Get<bp_state::PhysicalDevice>(physicalDevice);
    if (bp_pd_state) {
        bp_pd_state->vkGetPhysicalDeviceSurfaceCapabilitiesKHRState = QUERY_DETAILS;
    }
}

void BestPractices::ManualPostCallRecordGetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice,
                                                                                 VkSurfaceKHR surface,
                                                                                 VkSurfaceCapabilities2EXT* pSurfaceCapabilities,
                                                                                 VkResult result) {
    auto bp_pd_state = Get<bp_state::PhysicalDevice>(physicalDevice);
    if (bp_pd_state) {
        bp_pd_state->vkGetPhysicalDeviceSurfaceCapabilitiesKHRState = QUERY_DETAILS;
    }
}

void BestPractices::ManualPostCallRecordGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice,
                                                                                VkSurfaceKHR surface, uint32_t* pPresentModeCount,
                                                                                VkPresentModeKHR* pPresentModes, VkResult result) {
    auto bp_pd_data = Get<bp_state::PhysicalDevice>(physicalDevice);
    if (bp_pd_data) {
        auto& call_state = bp_pd_data->vkGetPhysicalDeviceSurfacePresentModesKHRState;

        if (*pPresentModeCount) {
            if (call_state < QUERY_COUNT) {
                call_state = QUERY_COUNT;
            }
        }
        if (pPresentModes) {
            if (call_state < QUERY_DETAILS) {
                call_state = QUERY_DETAILS;
            }
        }
    }
}

void BestPractices::ManualPostCallRecordGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                           uint32_t* pSurfaceFormatCount,
                                                                           VkSurfaceFormatKHR* pSurfaceFormats, VkResult result) {
    auto bp_pd_data = Get<bp_state::PhysicalDevice>(physicalDevice);
    if (bp_pd_data) {
        auto& call_state = bp_pd_data->vkGetPhysicalDeviceSurfaceFormatsKHRState;

        if (*pSurfaceFormatCount) {
            if (call_state < QUERY_COUNT) {
                call_state = QUERY_COUNT;
            }
            bp_pd_data->surface_formats_count = *pSurfaceFormatCount;
        }
        if (pSurfaceFormats) {
            if (call_state < QUERY_DETAILS) {
                call_state = QUERY_DETAILS;
            }
        }
    }
}

void BestPractices::ManualPostCallRecordGetPhysicalDeviceSurfaceFormats2KHR(VkPhysicalDevice physicalDevice,
                                                                            const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                                            uint32_t* pSurfaceFormatCount,
                                                                            VkSurfaceFormat2KHR* pSurfaceFormats, VkResult result) {
    auto bp_pd_data = Get<bp_state::PhysicalDevice>(physicalDevice);
    if (bp_pd_data) {
        if (*pSurfaceFormatCount) {
            if (bp_pd_data->vkGetPhysicalDeviceSurfaceFormatsKHRState < QUERY_COUNT) {
                bp_pd_data->vkGetPhysicalDeviceSurfaceFormatsKHRState = QUERY_COUNT;
            }
            bp_pd_data->surface_formats_count = *pSurfaceFormatCount;
        }
        if (pSurfaceFormats) {
            if (bp_pd_data->vkGetPhysicalDeviceSurfaceFormatsKHRState < QUERY_DETAILS) {
                bp_pd_data->vkGetPhysicalDeviceSurfaceFormatsKHRState = QUERY_DETAILS;
            }
        }
    }
}

void BestPractices::ManualPostCallRecordGetPhysicalDeviceDisplayPlanePropertiesKHR(VkPhysicalDevice physicalDevice,
                                                                                   uint32_t* pPropertyCount,
                                                                                   VkDisplayPlanePropertiesKHR* pProperties,
                                                                                   VkResult result) {
    auto bp_pd_data = Get<bp_state::PhysicalDevice>(physicalDevice);
    if (bp_pd_data) {
        if (*pPropertyCount) {
            if (bp_pd_data->vkGetPhysicalDeviceDisplayPlanePropertiesKHRState < QUERY_COUNT) {
                bp_pd_data->vkGetPhysicalDeviceDisplayPlanePropertiesKHRState = QUERY_COUNT;
            }
        }
        if (pProperties) {
            if (bp_pd_data->vkGetPhysicalDeviceDisplayPlanePropertiesKHRState < QUERY_DETAILS) {
                bp_pd_data->vkGetPhysicalDeviceDisplayPlanePropertiesKHRState = QUERY_DETAILS;
            }
        }
    }
}

void BestPractices::ManualPostCallRecordGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain,
                                                              uint32_t* pSwapchainImageCount, VkImage* pSwapchainImages,
                                                              VkResult result) {
    auto swapchain_state = Get<bp_state::Swapchain>(swapchain);
    if (swapchain_state && (pSwapchainImages || *pSwapchainImageCount)) {
        if (swapchain_state->vkGetSwapchainImagesKHRState < QUERY_DETAILS) {
            swapchain_state->vkGetSwapchainImagesKHRState = QUERY_DETAILS;
        }
    }
}
