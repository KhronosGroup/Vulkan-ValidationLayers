/* Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (C) 2015-2023 Google Inc.
 * Modifications Copyright (C) 2020-2022 Advanced Micro Devices, Inc. All rights reserved.
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

#include <algorithm>
#include <assert.h>
#include <sstream>
#include <vector>

#include <vulkan/vk_enum_string_helper.h>
#include "generated/chassis.h"
#include "core_validation.h"

static bool IsExtentInsideBounds(VkExtent2D extent, VkExtent2D min, VkExtent2D max) {
    if ((extent.width < min.width) || (extent.width > max.width) || (extent.height < min.height) || (extent.height > max.height)) {
        return false;
    }
    return true;
}

static VkImageCreateInfo GetSwapchainImpliedImageCreateInfo(VkSwapchainCreateInfoKHR const *pCreateInfo) {
    VkImageCreateInfo result = vku::InitStructHelper();

    if (pCreateInfo->flags & VK_SWAPCHAIN_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT_KHR) {
        result.flags |= VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT;
    }
    if (pCreateInfo->flags & VK_SWAPCHAIN_CREATE_PROTECTED_BIT_KHR) result.flags |= VK_IMAGE_CREATE_PROTECTED_BIT;
    if (pCreateInfo->flags & VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR) {
        result.flags |= VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT | VK_IMAGE_CREATE_EXTENDED_USAGE_BIT;
    }

    result.imageType = VK_IMAGE_TYPE_2D;
    result.format = pCreateInfo->imageFormat;
    result.extent.width = pCreateInfo->imageExtent.width;
    result.extent.height = pCreateInfo->imageExtent.height;
    result.extent.depth = 1;
    result.mipLevels = 1;
    result.arrayLayers = pCreateInfo->imageArrayLayers;
    result.samples = VK_SAMPLE_COUNT_1_BIT;
    result.tiling = VK_IMAGE_TILING_OPTIMAL;
    result.usage = pCreateInfo->imageUsage;
    result.sharingMode = pCreateInfo->imageSharingMode;
    result.queueFamilyIndexCount = pCreateInfo->queueFamilyIndexCount;
    result.pQueueFamilyIndices = pCreateInfo->pQueueFamilyIndices;
    result.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    return result;
}

bool CoreChecks::ValidateSwapchainPresentModesCreateInfo(VkPresentModeKHR present_mode, const Location &create_info_loc,
                                                         VkSwapchainCreateInfoKHR const *create_info,
                                                         const SURFACE_STATE *surface_state) const {
    bool skip = false;
    std::vector<VkPresentModeKHR> present_modes{};
    if (surface_state) {
        present_modes = surface_state->GetPresentModes(physical_device, this);
    } else if (IsExtEnabled(instance_extensions.vk_google_surfaceless_query)) {
        present_modes = physical_device_state->surfaceless_query_state.present_modes;
    }

    if (std::find(present_modes.begin(), present_modes.end(), present_mode) == present_modes.end()) {
        skip |= LogError("VUID-VkSwapchainCreateInfoKHR-presentMode-01281", device, create_info_loc,
                         "called with a non-supported presentMode (%s).", string_VkPresentModeKHR(present_mode));
    }

    // Validate VkSwapchainPresentModesCreateInfoEXT data
    auto swapchain_present_modes_ci = vku::FindStructInPNextChain<VkSwapchainPresentModesCreateInfoEXT>(create_info->pNext);
    if (swapchain_present_modes_ci) {
        // Ensure surface state is non-null if not using surfaceless_query
        assert(surface_state);
        bool found_swapchain_modes_ci_present_mode = false;
        const std::vector<VkPresentModeKHR> compatible_present_modes =
            surface_state->GetCompatibleModes(physical_device, present_mode);
        for (uint32_t i = 0; i < swapchain_present_modes_ci->presentModeCount; i++) {
            VkPresentModeKHR swapchain_present_mode = swapchain_present_modes_ci->pPresentModes[i];

            if (std::find(present_modes.begin(), present_modes.end(), swapchain_present_mode) == present_modes.end()) {
                if (LogError("VUID-VkSwapchainPresentModesCreateInfoEXT-None-07762", device,
                             create_info_loc.pNext(Struct::VkSwapchainPresentModesCreateInfoEXT, Field::pPresentModes, i),
                             "%s is a non-supported presentMode.", string_VkPresentModeKHR(swapchain_present_mode))) {
                    skip |= true;
                }
            }

            if (std::find(compatible_present_modes.begin(), compatible_present_modes.end(), swapchain_present_mode) ==
                compatible_present_modes.end()) {
                if (LogError("VUID-VkSwapchainPresentModesCreateInfoEXT-pPresentModes-07763", device,
                             create_info_loc.pNext(Struct::VkSwapchainPresentModesCreateInfoEXT, Field::pPresentModes, i),
                             "%s is a non-compatible presentMode.", string_VkPresentModeKHR(swapchain_present_mode))) {
                    skip |= true;
                }
            }

            const bool has_present_mode = (swapchain_present_modes_ci->pPresentModes[i] == present_mode);
            found_swapchain_modes_ci_present_mode |= has_present_mode;
        }
        if (!found_swapchain_modes_ci_present_mode) {
            if (LogError("VUID-VkSwapchainPresentModesCreateInfoEXT-presentMode-07764", device, create_info_loc,
                         "was called with a present mode (%s) that was not included in the set of present modes specified in "
                         "the vkSwapchainPresentModesCreateInfoEXT structure included in its pNext chain.",
                         string_VkPresentModeKHR(present_mode))) {
                skip |= true;
            }
        }
    }
    return skip;
}

bool CoreChecks::ValidateSwapchainPresentScalingCreateInfo(VkPresentModeKHR present_mode, const Location &create_info_loc,
                                                           const VkSurfaceCapabilitiesKHR *capabilities,
                                                           VkSwapchainCreateInfoKHR const *create_info,
                                                           const SURFACE_STATE *surface_state) const {
    bool skip = false;
    auto pres_scale_ci = vku::FindStructInPNextChain<VkSwapchainPresentScalingCreateInfoEXT>(create_info->pNext);
    if ((!pres_scale_ci) || (pres_scale_ci && (pres_scale_ci->scalingBehavior == 0))) {
        if (!IsExtentInsideBounds(create_info->imageExtent, capabilities->minImageExtent, capabilities->maxImageExtent)) {
            skip |= LogError("VUID-VkSwapchainCreateInfoKHR-pNext-07781", device, create_info_loc.dot(Field::imageExtent),
                             "(%" PRIu32 ",%" PRIu32
                             "), which is outside the bounds returned by "
                             "vkGetPhysicalDeviceSurfaceCapabilitiesKHR(): currentExtent = (%" PRIu32 ",%" PRIu32
                             "), minImageExtent = (%" PRIu32 ",%" PRIu32 "), maxImageExtent = (%" PRIu32 ",%" PRIu32 ").",
                             create_info->imageExtent.width, create_info->imageExtent.height, capabilities->currentExtent.width,
                             capabilities->currentExtent.height, capabilities->minImageExtent.width,
                             capabilities->minImageExtent.height, capabilities->maxImageExtent.width,
                             capabilities->maxImageExtent.height);
        }
    }

    if (pres_scale_ci) {
        if ((pres_scale_ci->presentGravityX == 0) && (pres_scale_ci->presentGravityY != 0)) {
            if (LogError("VUID-VkSwapchainPresentScalingCreateInfoEXT-presentGravityX-07765", device,
                         create_info_loc.pNext(Struct::VkSwapchainPresentScalingCreateInfoEXT, Field::presentGravityX),
                         "is zero but presentGravityY (%" PRIu32 ") is not zero.", pres_scale_ci->presentGravityY)) {
                skip |= true;
            }
        }

        if ((pres_scale_ci->presentGravityX != 0) && (pres_scale_ci->presentGravityY == 0)) {
            if (LogError("VUID-VkSwapchainPresentScalingCreateInfoEXT-presentGravityX-07766", device,
                         create_info_loc.pNext(Struct::VkSwapchainPresentScalingCreateInfoEXT, Field::presentGravityY),
                         "is zero but presentGravityX (%" PRIu32 ") is not zero.", pres_scale_ci->presentGravityX)) {
                skip |= true;
            }
        }

        if (GetBitSetCount(pres_scale_ci->scalingBehavior) > 1) {
            if (LogError("VUID-VkSwapchainPresentScalingCreateInfoEXT-scalingBehavior-07767", device,
                         create_info_loc.pNext(Struct::VkSwapchainPresentScalingCreateInfoEXT, Field::scalingBehavior),
                         "(%s) must not have more than one bit set.",
                         string_VkPresentScalingFlagsEXT(pres_scale_ci->scalingBehavior).c_str())) {
                skip |= true;
            }
        }

        if (GetBitSetCount(pres_scale_ci->presentGravityX) > 1) {
            if (LogError("VUID-VkSwapchainPresentScalingCreateInfoEXT-presentGravityX-07768", device,
                         create_info_loc.pNext(Struct::VkSwapchainPresentScalingCreateInfoEXT, Field::presentGravityX),
                         "(%s) must not have more than one bit set.",
                         string_VkPresentGravityFlagsEXT(pres_scale_ci->presentGravityX).c_str())) {
                skip |= true;
            }
        }

        if (GetBitSetCount(pres_scale_ci->presentGravityY) > 1) {
            if (LogError("VUID-VkSwapchainPresentScalingCreateInfoEXT-presentGravityY-07769", device,
                         create_info_loc.pNext(Struct::VkSwapchainPresentScalingCreateInfoEXT, Field::presentGravityY),
                         "(%s) must not have more than one bit set.",
                         string_VkPresentGravityFlagsEXT(pres_scale_ci->presentGravityY).c_str())) {
                skip |= true;
            }
        }

        assert(surface_state);
        VkSurfacePresentScalingCapabilitiesEXT scaling_caps =
            surface_state->GetPresentModeScalingCapabilities(physical_device, present_mode);

        if ((scaling_caps.supportedPresentScaling != 0) &&
            (scaling_caps.supportedPresentScaling & pres_scale_ci->scalingBehavior) == 0) {
            if (LogError("VUID-VkSwapchainPresentScalingCreateInfoEXT-scalingBehavior-07770", device,
                         create_info_loc.pNext(Struct::VkSwapchainPresentScalingCreateInfoEXT, Field::scalingBehavior),
                         "(%s) is not among the scaling methods for the surface as returned in "
                         "VkSurfacePresentScalingCapabilitiesEXT::supportedPresentScaling for the specified presentMode: (%s).",
                         string_VkPresentScalingFlagsEXT(pres_scale_ci->scalingBehavior).c_str(),
                         string_VkPresentGravityFlagsEXT(scaling_caps.supportedPresentScaling).c_str())) {
                skip |= true;
            }
        }

        if ((scaling_caps.supportedPresentGravityX != 0) &&
            (scaling_caps.supportedPresentGravityX & pres_scale_ci->presentGravityX) == 0) {
            if (LogError("VUID-VkSwapchainPresentScalingCreateInfoEXT-presentGravityX-07772", device,
                         create_info_loc.pNext(Struct::VkSwapchainPresentScalingCreateInfoEXT, Field::presentGravityX),
                         "(%s) must "
                         "be a valid present gravity for the surface as returned in "
                         "VkSurfacePresentScalingCapabilitiesEXT::supportedPresentGravityX for the given presentMode (%s).",
                         string_VkPresentGravityFlagsEXT(pres_scale_ci->presentGravityX).c_str(),
                         string_VkPresentGravityFlagsEXT(scaling_caps.supportedPresentGravityX).c_str())) {
                skip |= true;
            }
        }

        if ((scaling_caps.supportedPresentGravityY != 0) &&
            (scaling_caps.supportedPresentGravityY & pres_scale_ci->presentGravityY) == 0) {
            if (LogError("VUID-VkSwapchainPresentScalingCreateInfoEXT-presentGravityY-07774", device,
                         create_info_loc.pNext(Struct::VkSwapchainPresentScalingCreateInfoEXT, Field::presentGravityY),
                         "(%s) must "
                         "be a valid present gravity for the surface as returned in "
                         "VkSurfacePresentScalingCapabilitiesEXT::supportedPresentGravityY for the given presentMode (%s).",
                         string_VkPresentGravityFlagsEXT(pres_scale_ci->presentGravityY).c_str(),
                         string_VkPresentGravityFlagsEXT(scaling_caps.supportedPresentGravityY).c_str())) {
                skip |= true;
            }
        }

        if ((pres_scale_ci->scalingBehavior != 0) &&
            (!IsExtentInsideBounds(create_info->imageExtent, scaling_caps.minScaledImageExtent,
                                   scaling_caps.maxScaledImageExtent))) {
            if (LogError("VUID-VkSwapchainCreateInfoKHR-pNext-07782", device, create_info_loc.dot(Field::imageExtent),
                         "(%" PRIu32 ",%" PRIu32 "), which is outside the bounds returned in "
                         "VkSurfacePresentScalingCapabilitiesEXT minScaledImageExtent = (%" PRIu32 ",%" PRIu32 "), "
                         "maxScaledImageExtent = (%" PRIu32 ",%" PRIu32 ").",
                         create_info->imageExtent.width, create_info->imageExtent.height, scaling_caps.minScaledImageExtent.width,
                         scaling_caps.minScaledImageExtent.height, scaling_caps.maxScaledImageExtent.width,
                         scaling_caps.maxScaledImageExtent.height)) {
                skip |= true;
            }
        }

        // Further validation for when a VkSwapchainPresentModesCreateInfoEXT struct is *also* in the pNext chain
        const auto *present_modes_ci = vku::FindStructInPNextChain<VkSwapchainPresentModesCreateInfoEXT>(create_info->pNext);
        if (present_modes_ci) {
            for (uint32_t i = 0; i < present_modes_ci->presentModeCount; i++) {
                const Location present_mode_loc =
                    create_info_loc.pNext(Struct::VkSwapchainPresentModesCreateInfoEXT, Field::pPresentModes, i);
                scaling_caps =
                    surface_state->GetPresentModeScalingCapabilities(physical_device, present_modes_ci->pPresentModes[i]);

                if ((scaling_caps.supportedPresentScaling != 0) &&
                    (scaling_caps.supportedPresentScaling & pres_scale_ci->scalingBehavior) == 0) {
                    if (LogError("VUID-VkSwapchainPresentScalingCreateInfoEXT-scalingBehavior-07771", device,
                                 create_info_loc.pNext(Struct::VkSwapchainPresentScalingCreateInfoEXT, Field::scalingBehavior),
                                 "(%s) is not a valid present scaling benavior as returned in "
                                 "VkSurfacePresentScalingCapabilitiesEXT::supportedPresentScaling for %s (%s).",
                                 string_VkPresentScalingFlagsEXT(pres_scale_ci->scalingBehavior).c_str(),
                                 present_mode_loc.Fields().c_str(),
                                 string_VkPresentScalingFlagsEXT(scaling_caps.supportedPresentScaling).c_str())) {
                        skip |= true;
                    }
                }

                if ((scaling_caps.supportedPresentGravityX != 0) &&
                    (scaling_caps.supportedPresentGravityX & pres_scale_ci->presentGravityX) == 0) {
                    if (LogError("VUID-VkSwapchainPresentScalingCreateInfoEXT-presentGravityX-07773", device,
                                 create_info_loc.pNext(Struct::VkSwapchainPresentScalingCreateInfoEXT, Field::presentGravityX),
                                 "(%s) is not a valid x-axis present gravity as returned in "
                                 "VkSurfacePresentScalingCapabilitiesEXT::supportedPresentGravityX for %s (%s).",
                                 string_VkPresentGravityFlagsEXT(pres_scale_ci->presentGravityX).c_str(),
                                 present_mode_loc.Fields().c_str(),
                                 string_VkPresentGravityFlagsEXT(scaling_caps.supportedPresentGravityX).c_str())) {
                        skip |= true;
                    }
                }

                if ((scaling_caps.supportedPresentGravityY != 0) &&
                    (scaling_caps.supportedPresentGravityY & pres_scale_ci->presentGravityY) == 0) {
                    if (LogError("VUID-VkSwapchainPresentScalingCreateInfoEXT-presentGravityY-07775", device,
                                 create_info_loc.pNext(Struct::VkSwapchainPresentScalingCreateInfoEXT, Field::presentGravityY),
                                 "(%s) is not a valid y-axis present gravity as returned in "
                                 "VkSurfacePresentScalingCapabilitiesEXT::supportedPresentGravityY for %s (%s).",
                                 string_VkPresentGravityFlagsEXT(pres_scale_ci->presentGravityY).c_str(),
                                 present_mode_loc.Fields().c_str(),
                                 string_VkPresentGravityFlagsEXT(scaling_caps.supportedPresentGravityY).c_str())) {
                        skip |= true;
                    }
                }
            }
        }
    }
    return skip;
}

bool CoreChecks::ValidateCreateSwapchain(VkSwapchainCreateInfoKHR const *pCreateInfo, const SURFACE_STATE *surface_state,
                                         const SWAPCHAIN_NODE *old_swapchain_state, const Location &create_info_loc) const {
    // All physical devices and queue families are required to be able to present to any native window on Android; require the
    // application to have established support on any other platform.
    if (!instance_extensions.vk_khr_android_surface) {
        // restrict search only to queue families of VkDeviceQueueCreateInfos, not the whole physical device
        const bool is_supported = AnyOf<QUEUE_STATE>([this, surface_state](const QUEUE_STATE &queue_state) {
            return surface_state->GetQueueSupport(physical_device, queue_state.queueFamilyIndex);
        });

        if (!is_supported) {
            const LogObjectList objlist(device, surface_state->Handle());
            if (LogError("VUID-VkSwapchainCreateInfoKHR-surface-01270", objlist, create_info_loc.dot(Field::surface),
                         "is not supported for presentation by this device.")) {
                return true;
            }
        }
    }

    if (old_swapchain_state) {
        if (old_swapchain_state->createInfo.surface != pCreateInfo->surface) {
            if (LogError("VUID-VkSwapchainCreateInfoKHR-oldSwapchain-01933", pCreateInfo->oldSwapchain,
                         create_info_loc.dot(Field::oldSwapchain), "surface is not pCreateInfo->surface")) {
                return true;
            }
        }
        if (old_swapchain_state->retired) {
            if (LogError("VUID-VkSwapchainCreateInfoKHR-oldSwapchain-01933", pCreateInfo->oldSwapchain,
                         create_info_loc.dot(Field::oldSwapchain), "is retired")) {
                return true;
            }
        }
    }

    if ((pCreateInfo->imageExtent.width == 0) || (pCreateInfo->imageExtent.height == 0)) {
        if (LogError("VUID-VkSwapchainCreateInfoKHR-imageExtent-01689", device, create_info_loc.dot(Field::imageExtent),
                     "width (%d) and height (%d) is invalid.", pCreateInfo->imageExtent.width, pCreateInfo->imageExtent.height)) {
            return true;
        }
    }

    void *surface_caps_query_pnext = nullptr;
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    VkSurfaceFullScreenExclusiveInfoEXT full_screen_info_copy = vku::InitStructHelper();
    VkSurfaceFullScreenExclusiveWin32InfoEXT win32_full_screen_info_copy = vku::InitStructHelper();
    const auto *full_screen_info = vku::FindStructInPNextChain<VkSurfaceFullScreenExclusiveInfoEXT>(pCreateInfo->pNext);
    if (full_screen_info && full_screen_info->fullScreenExclusive == VK_FULL_SCREEN_EXCLUSIVE_APPLICATION_CONTROLLED_EXT) {
        full_screen_info_copy = *full_screen_info;
        full_screen_info_copy.pNext = surface_caps_query_pnext;
        surface_caps_query_pnext = &full_screen_info_copy;

        if (IsExtEnabled(device_extensions.vk_khr_win32_surface)) {
            const auto *win32_full_screen_info = vku::FindStructInPNextChain<VkSurfaceFullScreenExclusiveWin32InfoEXT>(pCreateInfo->pNext);
            if (!win32_full_screen_info) {
                const LogObjectList objlist(device, pCreateInfo->surface);
                if (LogError("VUID-VkSwapchainCreateInfoKHR-pNext-02679", objlist, create_info_loc,
                             "pNext chain contains "
                             "VkSurfaceFullScreenExclusiveInfoEXT, but does not contain "
                             "VkSurfaceFullScreenExclusiveWin32InfoEXT.")) {
                    return true;
                }
            } else {
                win32_full_screen_info_copy = *win32_full_screen_info;
                win32_full_screen_info_copy.pNext = surface_caps_query_pnext;
                surface_caps_query_pnext = &win32_full_screen_info_copy;
            }
        }
    }
#endif
    VkSurfacePresentModeEXT present_mode_info = vku::InitStructHelper();
    if (IsExtEnabled(device_extensions.vk_ext_surface_maintenance1)) {
        present_mode_info.presentMode = pCreateInfo->presentMode;
        present_mode_info.pNext = surface_caps_query_pnext;
        surface_caps_query_pnext = &present_mode_info;
    }

    const auto surface_caps2 = surface_state->GetCapabilities(IsExtEnabled(instance_extensions.vk_khr_get_surface_capabilities2),
                                                              physical_device_state->PhysDev(), surface_caps_query_pnext, this);

    bool skip = false;
    VkSurfaceTransformFlagBitsKHR current_transform = surface_caps2.surfaceCapabilities.currentTransform;
    if ((pCreateInfo->preTransform & current_transform) != pCreateInfo->preTransform) {
        skip |= LogPerformanceWarning(kVUID_Core_Swapchain_PreTransform, physical_device, create_info_loc.dot(Field::preTransform),
                                      "(%s) doesn't match the currentTransform (%s) returned by "
                                      "vkGetPhysicalDeviceSurfaceCapabilitiesKHR, the presentation engine will transform the image "
                                      "content as part of the presentation operation.",
                                      string_VkSurfaceTransformFlagBitsKHR(pCreateInfo->preTransform),
                                      string_VkSurfaceTransformFlagBitsKHR(current_transform));
    }

    const VkPresentModeKHR present_mode = pCreateInfo->presentMode;
    const bool shared_present_mode = (VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR == present_mode ||
                                      VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR == present_mode);

    // Validate pCreateInfo->minImageCount against VkSurfaceCapabilitiesKHR::{min|max}ImageCount:
    // Shared Present Mode must have a minImageCount of 1
    if ((pCreateInfo->minImageCount < surface_caps2.surfaceCapabilities.minImageCount) && !shared_present_mode) {
        if (LogError("VUID-VkSwapchainCreateInfoKHR-presentMode-02839", device, create_info_loc.dot(Field::minImageCount),
                     "%" PRIu32 ", which is outside the bounds returned by "
                     "vkGetPhysicalDeviceSurfaceCapabilitiesKHR() (i.e. minImageCount = %d, maxImageCount = %d).",
                     pCreateInfo->minImageCount, surface_caps2.surfaceCapabilities.minImageCount,
                     surface_caps2.surfaceCapabilities.maxImageCount)) {
            return true;
        }
    }

    if ((surface_caps2.surfaceCapabilities.maxImageCount > 0) &&
        (pCreateInfo->minImageCount > surface_caps2.surfaceCapabilities.maxImageCount)) {
        if (LogError("VUID-VkSwapchainCreateInfoKHR-minImageCount-01272", device, create_info_loc.dot(Field::minImageCount),
                     "%" PRIu32 ", which is outside the bounds returned by "
                     "vkGetPhysicalDeviceSurfaceCapabilitiesKHR() (i.e. minImageCount = %d, maxImageCount = %d).",
                     pCreateInfo->minImageCount, surface_caps2.surfaceCapabilities.minImageCount,
                     surface_caps2.surfaceCapabilities.maxImageCount)) {
            return true;
        }
    }

    // pCreateInfo->preTransform should have exactly one bit set, and that bit must also be set in
    // VkSurfaceCapabilitiesKHR::supportedTransforms.
    if (!pCreateInfo->preTransform || (pCreateInfo->preTransform & (pCreateInfo->preTransform - 1)) ||
        !(pCreateInfo->preTransform & surface_caps2.surfaceCapabilities.supportedTransforms)) {
        std::stringstream ss;
        for (int i = 0; i < 32; i++) {
            if ((1 << i) & surface_caps2.surfaceCapabilities.supportedTransforms) {
                ss << "  " << string_VkSurfaceTransformFlagBitsKHR(static_cast<VkSurfaceTransformFlagBitsKHR>(1 << i)) << "%s\n";
            }
        }
        return LogError("VUID-VkSwapchainCreateInfoKHR-preTransform-01279", device, create_info_loc.dot(Field::preTransform),
                        "is not supported, support values are:\n%s.", ss.str().c_str());
    }

    // pCreateInfo->compositeAlpha should have exactly one bit set, and that bit must also be set in
    // VkSurfaceCapabilitiesKHR::supportedCompositeAlpha
    if (!pCreateInfo->compositeAlpha || (pCreateInfo->compositeAlpha & (pCreateInfo->compositeAlpha - 1)) ||
        !((pCreateInfo->compositeAlpha) & surface_caps2.surfaceCapabilities.supportedCompositeAlpha)) {
        std::stringstream ss;
        for (int i = 0; i < 32; i++) {
            if ((1 << i) & surface_caps2.surfaceCapabilities.supportedCompositeAlpha) {
                ss << "  " << string_VkCompositeAlphaFlagBitsKHR(static_cast<VkCompositeAlphaFlagBitsKHR>(1 << i)) << "%s\n";
            }
        }
        return LogError("VUID-VkSwapchainCreateInfoKHR-compositeAlpha-01280", device, create_info_loc.dot(Field::compositeAlpha),
                        "is not supported, support values are:\n%s.", ss.str().c_str());
    }
    // Validate pCreateInfo->imageArrayLayers against VkSurfaceCapabilitiesKHR::maxImageArrayLayers:
    if (pCreateInfo->imageArrayLayers > surface_caps2.surfaceCapabilities.maxImageArrayLayers) {
        if (LogError("VUID-VkSwapchainCreateInfoKHR-imageArrayLayers-01275", device, create_info_loc.dot(Field::imageArrayLayers),
                     "%" PRIu32 " is more than maxImageArrayLayers %" PRIu32 ".", pCreateInfo->imageArrayLayers,
                     surface_caps2.surfaceCapabilities.maxImageArrayLayers)) {
            return true;
        }
    }
    const VkImageUsageFlags image_usage = pCreateInfo->imageUsage;
    // Validate pCreateInfo->imageUsage against VkSurfaceCapabilitiesKHR::supportedUsageFlags:
    // Shared Present Mode uses different set of capabilities to check imageUsage support
    if ((image_usage != (image_usage & surface_caps2.surfaceCapabilities.supportedUsageFlags)) && !shared_present_mode) {
        if (LogError("VUID-VkSwapchainCreateInfoKHR-presentMode-01427", device, create_info_loc.dot(Field::imageUsage),
                     "(0x%08x) are not in supportedUsageFlags (0x%08x).", image_usage,
                     surface_caps2.surfaceCapabilities.supportedUsageFlags)) {
            return true;
        }
    }

    if (pCreateInfo->flags & VK_SWAPCHAIN_CREATE_PROTECTED_BIT_KHR) {
        const bool is_required_ext_enabled = IsExtEnabled(instance_extensions.vk_khr_surface_protected_capabilities);

        // Assume that the "protected" flag is not supported if VK_KHR_surface_protected_capabilities is not enabled
        bool log_error = !is_required_ext_enabled;

        if (is_required_ext_enabled) {
            VkPhysicalDeviceSurfaceInfo2KHR surface_info = vku::InitStructHelper();
            surface_info.surface = pCreateInfo->surface;
            VkSurfaceProtectedCapabilitiesKHR surface_protected_capabilities = vku::InitStructHelper();
            VkSurfaceCapabilities2KHR surface_capabilities =
                vku::InitStructHelper(&surface_protected_capabilities);
            const VkResult result = DispatchGetPhysicalDeviceSurfaceCapabilities2KHR(physical_device_state->PhysDev(),
                                                                                     &surface_info, &surface_capabilities);

            log_error = (result == VK_SUCCESS) && !surface_protected_capabilities.supportsProtected;
        }

        if (log_error) {
            if (LogError("VUID-VkSwapchainCreateInfoKHR-flags-03187", device, create_info_loc.dot(Field::flags),
                         "contains VK_SWAPCHAIN_CREATE_PROTECTED_BIT_KHR but the surface "
                         "capabilities does not have VkSurfaceProtectedCapabilitiesKHR.supportsProtected set to VK_TRUE.")) {
                return true;
            }
        }
    }

    // Validate pCreateInfo values with the results of vkGetPhysicalDeviceSurfaceFormats2KHR():
    {
        // Validate pCreateInfo->imageFormat against VkSurfaceFormatKHR::format:
        bool found_format = false;
        bool found_color_space = false;
        bool found_match = false;

        vvl::span<const safe_VkSurfaceFormat2KHR> formats{};
        if (surface_state) {
            formats = surface_state->GetFormats(IsExtEnabled(instance_extensions.vk_khr_get_surface_capabilities2),
                                                physical_device_state->PhysDev(), surface_caps_query_pnext, this);
        } else if (IsExtEnabled(instance_extensions.vk_google_surfaceless_query)) {
            formats = physical_device_state->surfaceless_query_state.formats;
        }
        for (const auto &format : formats) {
            if (pCreateInfo->imageFormat == format.surfaceFormat.format) {
                // Validate pCreateInfo->imageColorSpace against VkSurfaceFormatKHR::colorSpace:
                found_format = true;
                if (pCreateInfo->imageColorSpace == format.surfaceFormat.colorSpace) {
                    found_match = true;
                    break;
                }
            } else {
                if (pCreateInfo->imageColorSpace == format.surfaceFormat.colorSpace) {
                    found_color_space = true;
                }
            }
        }
        if (!found_match) {
            if (!found_format) {
                if (LogError("VUID-VkSwapchainCreateInfoKHR-imageFormat-01273", device, create_info_loc.dot(Field::imageFormat),
                             "is %s.", string_VkFormat(pCreateInfo->imageFormat))) {
                    return true;
                }
            }
            if (!found_color_space) {
                if (LogError("VUID-VkSwapchainCreateInfoKHR-imageFormat-01273", device, create_info_loc.dot(Field::imageColorSpace),
                             "is %s.", string_VkColorSpaceKHR(pCreateInfo->imageColorSpace))) {
                    return true;
                }
            }
        }
    }

    if (!IsExtEnabled(device_extensions.vk_ext_swapchain_maintenance1)) {
        // Validate pCreateInfo->imageExtent against VkSurfaceCapabilitiesKHR::{current|min|max}ImageExtent:
        if (!IsExtentInsideBounds(pCreateInfo->imageExtent, surface_caps2.surfaceCapabilities.minImageExtent,
                                  surface_caps2.surfaceCapabilities.maxImageExtent)) {
            safe_VkSurfaceCapabilities2KHR cached_capabilities{};
            if (surface_state) {
                cached_capabilities =
                    surface_state->GetCapabilities(IsExtEnabled(instance_extensions.vk_khr_get_surface_capabilities2),
                                                   physical_device_state->PhysDev(), surface_caps_query_pnext, this);
            } else if (IsExtEnabled(instance_extensions.vk_google_surfaceless_query)) {
                cached_capabilities = physical_device_state->surfaceless_query_state.capabilities;
            }
            if (!IsExtentInsideBounds(pCreateInfo->imageExtent, cached_capabilities.surfaceCapabilities.minImageExtent,
                                      cached_capabilities.surfaceCapabilities.maxImageExtent)) {
                // TODO - Combine VUs with other same VUID
                skip |= LogError(
                    "VUID-VkSwapchainCreateInfoKHR-pNext-07781", device, create_info_loc.dot(Field::imageExtent),
                    "(%" PRIu32 ", %" PRIu32
                    "), which is outside the bounds returned by "
                    "vkGetPhysicalDeviceSurfaceCapabilitiesKHR(): currentExtent = (%" PRIu32 ",%" PRIu32
                    "), minImageExtent = (%" PRIu32 ",%" PRIu32 "), maxImageExtent = (%" PRIu32 ",%" PRIu32 ").",
                    pCreateInfo->imageExtent.width, pCreateInfo->imageExtent.height,
                    surface_caps2.surfaceCapabilities.currentExtent.width, surface_caps2.surfaceCapabilities.currentExtent.height,
                    surface_caps2.surfaceCapabilities.minImageExtent.width, surface_caps2.surfaceCapabilities.minImageExtent.height,
                    surface_caps2.surfaceCapabilities.maxImageExtent.width,
                    surface_caps2.surfaceCapabilities.maxImageExtent.height);
            }
        }
    } else {
        skip |= ValidateSwapchainPresentModesCreateInfo(present_mode, create_info_loc, pCreateInfo, surface_state);
        skip |= ValidateSwapchainPresentScalingCreateInfo(present_mode, create_info_loc, &surface_caps2.surfaceCapabilities,
                                                          pCreateInfo, surface_state);
    }
    // Validate state for shared presentable case
    if (shared_present_mode) {
        if (!IsExtEnabled(device_extensions.vk_khr_shared_presentable_image)) {
            if (LogError(kVUID_Core_DrawState_ExtensionNotEnabled, device, create_info_loc,
                         "called with presentMode %s which requires the VK_KHR_shared_presentable_image extension, which has not "
                         "been enabled.",
                         string_VkPresentModeKHR(present_mode))) {
                return true;
            }
        } else if (pCreateInfo->minImageCount != 1) {
            if (LogError("VUID-VkSwapchainCreateInfoKHR-minImageCount-01383", device, create_info_loc,
                         "called with presentMode %s, but minImageCount value is %d. For shared presentable image, minImageCount "
                         "must be 1.",
                         string_VkPresentModeKHR(present_mode), pCreateInfo->minImageCount)) {
                return true;
            }
        }

        VkSharedPresentSurfaceCapabilitiesKHR shared_present_capabilities = vku::InitStructHelper();
        VkSurfaceCapabilities2KHR capabilities2 = vku::InitStructHelper(&shared_present_capabilities);
        VkPhysicalDeviceSurfaceInfo2KHR surface_info = vku::InitStructHelper();
        surface_info.surface = pCreateInfo->surface;
        DispatchGetPhysicalDeviceSurfaceCapabilities2KHR(physical_device_state->PhysDev(), &surface_info, &capabilities2);

        if (image_usage != (image_usage & shared_present_capabilities.sharedPresentSupportedUsageFlags)) {
            if (LogError("VUID-VkSwapchainCreateInfoKHR-imageUsage-01384", device, create_info_loc.dot(Field::imageUsage),
                         "(0x%08x), but the supported flag bits for %s "
                         "present mode are 0x%08x.",
                         image_usage, string_VkPresentModeKHR(pCreateInfo->presentMode),
                         shared_present_capabilities.sharedPresentSupportedUsageFlags)) {
                return true;
            }
        }
    }

    if ((pCreateInfo->imageSharingMode == VK_SHARING_MODE_CONCURRENT) && pCreateInfo->pQueueFamilyIndices) {
        bool skip1 = ValidatePhysicalDeviceQueueFamilies(pCreateInfo->queueFamilyIndexCount, pCreateInfo->pQueueFamilyIndices,
                                                         create_info_loc, "VUID-VkSwapchainCreateInfoKHR-imageSharingMode-01428");
        if (skip1) return true;
    }

    // Validate pCreateInfo->imageUsage against GetPhysicalDeviceFormatProperties
    const VkFormatProperties3KHR format_properties = GetPDFormatProperties(pCreateInfo->imageFormat);
    const VkFormatFeatureFlags2KHR tiling_features = format_properties.optimalTilingFeatures;

    if (tiling_features == 0) {
        if (LogError("VUID-VkSwapchainCreateInfoKHR-imageFormat-01778", device, create_info_loc.dot(Field::imageFormat),
                     "%s with tiling VK_IMAGE_TILING_OPTIMAL has no supported format features on this "
                     "physical device.",
                     string_VkFormat(pCreateInfo->imageFormat))) {
            return true;
        }
    } else if ((image_usage & VK_IMAGE_USAGE_SAMPLED_BIT) && !(tiling_features & VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_BIT_KHR)) {
        if (LogError("VUID-VkSwapchainCreateInfoKHR-imageFormat-01778", device, create_info_loc.dot(Field::imageFormat),
                     "%s with tiling VK_IMAGE_TILING_OPTIMAL does not support usage that includes "
                     "VK_IMAGE_USAGE_SAMPLED_BIT.",
                     string_VkFormat(pCreateInfo->imageFormat))) {
            return true;
        }
    } else if ((image_usage & VK_IMAGE_USAGE_STORAGE_BIT) && !(tiling_features & VK_FORMAT_FEATURE_2_STORAGE_IMAGE_BIT_KHR)) {
        if (LogError("VUID-VkSwapchainCreateInfoKHR-imageFormat-01778", device, create_info_loc.dot(Field::imageFormat),
                     "%s with tiling VK_IMAGE_TILING_OPTIMAL does not support usage that includes "
                     "VK_IMAGE_USAGE_STORAGE_BIT.",
                     string_VkFormat(pCreateInfo->imageFormat))) {
            return true;
        }
    } else if ((image_usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) &&
               !(tiling_features & VK_FORMAT_FEATURE_2_COLOR_ATTACHMENT_BIT_KHR)) {
        if (LogError("VUID-VkSwapchainCreateInfoKHR-imageFormat-01778", device, create_info_loc.dot(Field::imageFormat),
                     "%s with tiling VK_IMAGE_TILING_OPTIMAL does not support usage that includes "
                     "VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT.",
                     string_VkFormat(pCreateInfo->imageFormat))) {
            return true;
        }
    } else if ((image_usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) &&
               !(tiling_features & VK_FORMAT_FEATURE_2_DEPTH_STENCIL_ATTACHMENT_BIT_KHR)) {
        if (LogError("VUID-VkSwapchainCreateInfoKHR-imageFormat-01778", device, create_info_loc.dot(Field::imageFormat),
                     "%s with tiling VK_IMAGE_TILING_OPTIMAL does not support usage that includes "
                     "VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT.",
                     string_VkFormat(pCreateInfo->imageFormat))) {
            return true;
        }
    } else if ((image_usage & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT) &&
               !(tiling_features &
                 (VK_FORMAT_FEATURE_2_COLOR_ATTACHMENT_BIT_KHR | VK_FORMAT_FEATURE_2_DEPTH_STENCIL_ATTACHMENT_BIT_KHR))) {
        if (LogError("VUID-VkSwapchainCreateInfoKHR-imageFormat-01778", device, create_info_loc.dot(Field::imageFormat),
                     "%s with tiling VK_IMAGE_TILING_OPTIMAL does not support usage that includes "
                     "VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT or VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT.",
                     string_VkFormat(pCreateInfo->imageFormat))) {
            return true;
        }
    }

    const VkImageCreateInfo image_create_info = GetSwapchainImpliedImageCreateInfo(pCreateInfo);
    VkImageFormatProperties image_properties = {};
    const VkResult image_properties_result = DispatchGetPhysicalDeviceImageFormatProperties(
        physical_device, image_create_info.format, image_create_info.imageType, image_create_info.tiling, image_create_info.usage,
        image_create_info.flags, &image_properties);

    if (image_properties_result != VK_SUCCESS) {
        if (LogError("VUID-VkSwapchainCreateInfoKHR-imageFormat-01778", device, create_info_loc,
                     "vkGetPhysicalDeviceImageFormatProperties() unexpectedly failed, "
                     "with following params: "
                     "format: %s, imageType: %s, "
                     "tiling: %s, usage: %s, "
                     "flags: %s.",
                     string_VkFormat(image_create_info.format), string_VkImageType(image_create_info.imageType),
                     string_VkImageTiling(image_create_info.tiling), string_VkImageUsageFlags(image_create_info.usage).c_str(),
                     string_VkImageCreateFlags(image_create_info.flags).c_str())) {
            return true;
        }
    }

    // Validate pCreateInfo->imageArrayLayers against VkImageFormatProperties::maxArrayLayers
    if (pCreateInfo->imageArrayLayers > image_properties.maxArrayLayers) {
        if (LogError("VUID-VkSwapchainCreateInfoKHR-imageFormat-01778", device, create_info_loc.dot(Field::imageArrayLayers),
                     "%" PRIu32 ", but Maximum value returned by vkGetPhysicalDeviceImageFormatProperties() is %d "
                     "for imageFormat %s with tiling VK_IMAGE_TILING_OPTIMAL.",
                     pCreateInfo->imageArrayLayers, image_properties.maxArrayLayers, string_VkFormat(pCreateInfo->imageFormat))) {
            return true;
        }
    }

    // Validate pCreateInfo->imageExtent against VkImageFormatProperties::maxExtent
    if ((pCreateInfo->imageExtent.width > image_properties.maxExtent.width) ||
        (pCreateInfo->imageExtent.height > image_properties.maxExtent.height)) {
        if (LogError("VUID-VkSwapchainCreateInfoKHR-imageFormat-01778", device, create_info_loc.dot(Field::imageExtent),
                     "(%d,%d), which is bigger than max extent (%d,%d)"
                     "returned by vkGetPhysicalDeviceImageFormatProperties(): "
                     "for imageFormat %s with tiling VK_IMAGE_TILING_OPTIMAL.",
                     pCreateInfo->imageExtent.width, pCreateInfo->imageExtent.height, image_properties.maxExtent.width,
                     image_properties.maxExtent.height, string_VkFormat(pCreateInfo->imageFormat))) {
            return true;
        }
    }

    if ((pCreateInfo->flags & VK_SWAPCHAIN_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT_KHR) && physical_device_count == 1) {
        if (LogError("VUID-VkSwapchainCreateInfoKHR-physicalDeviceCount-01429", device, create_info_loc.dot(Field::flags),
                     "containing VK_SWAPCHAIN_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT_KHR"
                     "but logical device was created with VkDeviceGroupDeviceCreateInfo::physicalDeviceCount equal to 1."
                     "The logical device may have been created without explicitly using VkDeviceGroupDeviceCreateInfo, or with"
                     "VkDeviceGroupDeviceCreateInfo::physicalDeviceCount equal to zero. "
                     "It is equivalent to using VkDeviceGroupDeviceCreateInfo with "
                     "VkDeviceGroupDeviceCreateInfo::physicalDeviceCount equal to 1.")) {
            return true;
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR *pCreateInfo,
                                                   const VkAllocationCallbacks *pAllocator, VkSwapchainKHR *pSwapchain,
                                                   const ErrorObject &error_obj) const {
    auto surface_state = Get<SURFACE_STATE>(pCreateInfo->surface);
    auto old_swapchain_state = Get<SWAPCHAIN_NODE>(pCreateInfo->oldSwapchain);
    return ValidateCreateSwapchain(pCreateInfo, surface_state.get(), old_swapchain_state.get(),
                                   error_obj.location.dot(Field::pCreateInfo));
}

void CoreChecks::PreCallRecordDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain,
                                                  const VkAllocationCallbacks *pAllocator) {
    if (swapchain) {
        auto swapchain_data = Get<SWAPCHAIN_NODE>(swapchain);
        if (swapchain_data) {
            for (const auto &swapchain_image : swapchain_data->images) {
                if (!swapchain_image.image_state) continue;
                qfo_release_image_barrier_map.erase(swapchain_image.image_state->image());
            }
        }
    }
    StateTracker::PreCallRecordDestroySwapchainKHR(device, swapchain, pAllocator);
}

void CoreChecks::PostCallRecordGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t *pSwapchainImageCount,
                                                     VkImage *pSwapchainImages, const RecordObject &record_obj) {
    // This function will run twice. The first is to get pSwapchainImageCount. The second is to get pSwapchainImages.
    // The first time in StateTracker::PostCallRecordGetSwapchainImagesKHR only generates the container's size.
    // The second time in StateTracker::PostCallRecordGetSwapchainImagesKHR will create VKImage and IMAGE_STATE.

    // So GlobalImageLayoutMap saving new IMAGE_STATEs has to run in the second time.
    // pSwapchainImages is not nullptr and it needs to wait until StateTracker::PostCallRecordGetSwapchainImagesKHR.

    uint32_t new_swapchain_image_index = 0;
    if (((record_obj.result == VK_SUCCESS) || (record_obj.result == VK_INCOMPLETE)) && pSwapchainImages) {
        auto swapchain_state = Get<SWAPCHAIN_NODE>(swapchain);
        const auto image_vector_size = swapchain_state->images.size();

        for (; new_swapchain_image_index < *pSwapchainImageCount; ++new_swapchain_image_index) {
            if ((new_swapchain_image_index >= image_vector_size) ||
                !swapchain_state->images[new_swapchain_image_index].image_state) {
                break;
            }
        }
    }
    StateTracker::PostCallRecordGetSwapchainImagesKHR(device, swapchain, pSwapchainImageCount, pSwapchainImages, record_obj);

    if (((record_obj.result == VK_SUCCESS) || (record_obj.result == VK_INCOMPLETE)) && pSwapchainImages) {
        for (; new_swapchain_image_index < *pSwapchainImageCount; ++new_swapchain_image_index) {
            auto image_state = Get<IMAGE_STATE>(pSwapchainImages[new_swapchain_image_index]);
            image_state->SetInitialLayoutMap();
        }
    }
}

bool CoreChecks::PreCallValidateQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR *pPresentInfo,
                                                const ErrorObject &error_obj) const {
    bool skip = false;
    auto queue_state = Get<QUEUE_STATE>(queue);

    SemaphoreSubmitState sem_submit_state(this, queue,
                                          physical_device_state->queue_family_properties[queue_state->queueFamilyIndex].queueFlags);

    const Location present_info_loc = error_obj.location.dot(Struct::VkPresentInfoKHR, Field::pPresentInfo);
    for (uint32_t i = 0; i < pPresentInfo->waitSemaphoreCount; ++i) {
        auto semaphore_state = Get<SEMAPHORE_STATE>(pPresentInfo->pWaitSemaphores[i]);
        if (semaphore_state && semaphore_state->type != VK_SEMAPHORE_TYPE_BINARY) {
            skip |= LogError("VUID-vkQueuePresentKHR-pWaitSemaphores-03267", pPresentInfo->pWaitSemaphores[i],
                             present_info_loc.dot(Field::pWaitSemaphores, i), "(%s) is not a VK_SEMAPHORE_TYPE_BINARY",
                             FormatHandle(pPresentInfo->pWaitSemaphores[i]).c_str());
            continue;
        }
        skip |= sem_submit_state.ValidateWaitSemaphore(present_info_loc.dot(Field::pWaitSemaphores, i),
                                                       pPresentInfo->pWaitSemaphores[i], 0);
    }

    for (uint32_t i = 0; i < pPresentInfo->swapchainCount; ++i) {
        auto swapchain_data = Get<SWAPCHAIN_NODE>(pPresentInfo->pSwapchains[i]);
        if (swapchain_data) {
            const Location swapchain_loc = present_info_loc.dot(Field::pSwapchains, i);
            // Check if index is even possible to be acquired to give better error message
            if (pPresentInfo->pImageIndices[i] >= swapchain_data->images.size()) {
                skip |= LogError("VUID-VkPresentInfoKHR-pImageIndices-01430", pPresentInfo->pSwapchains[i], swapchain_loc,
                                 "image index is too large (%" PRIu32 "), There are only %" PRIu32 " images in this swapchain.",
                                 pPresentInfo->pImageIndices[i], static_cast<uint32_t>(swapchain_data->images.size()));
            } else if (!swapchain_data->images[pPresentInfo->pImageIndices[i]].image_state ||
                       !swapchain_data->images[pPresentInfo->pImageIndices[i]].acquired) {
                skip |= LogError("VUID-VkPresentInfoKHR-pImageIndices-01430", pPresentInfo->pSwapchains[i], swapchain_loc,
                                 "image at index %" PRIu32 " was not acquired from the swapchain.", pPresentInfo->pImageIndices[i]);
            } else {
                const auto *image_state = swapchain_data->images[pPresentInfo->pImageIndices[i]].image_state;
                assert(image_state);

                std::vector<VkImageLayout> layouts;
                if (FindLayouts(*image_state, layouts)) {
                    for (auto layout : layouts) {
                        if ((layout != VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) &&
                            (!IsExtEnabled(device_extensions.vk_khr_shared_presentable_image) ||
                             (layout != VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR))) {
                            skip |= LogError("VUID-VkPresentInfoKHR-pImageIndices-01430", queue, swapchain_loc,
                                             "images passed to present must be in layout "
                                             "VK_IMAGE_LAYOUT_PRESENT_SRC_KHR or "
                                             "VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR but is in %s.",
                                             string_VkImageLayout(layout));
                        }
                    }
                }
                const auto *display_present_info = vku::FindStructInPNextChain<VkDisplayPresentInfoKHR>(pPresentInfo->pNext);
                if (display_present_info) {
                    if (display_present_info->srcRect.offset.x < 0 || display_present_info->srcRect.offset.y < 0 ||
                        display_present_info->srcRect.offset.x + display_present_info->srcRect.extent.width >
                            image_state->createInfo.extent.width ||
                        display_present_info->srcRect.offset.y + display_present_info->srcRect.extent.height >
                            image_state->createInfo.extent.height) {
                        skip |= LogError("VUID-VkDisplayPresentInfoKHR-srcRect-01257", queue, swapchain_loc,
                                         "vkQueuePresentKHR(): VkDisplayPresentInfoKHR::srcRect (offset (%" PRIu32 ", %" PRIu32
                                         "), extent (%" PRIu32 ", %" PRIu32
                                         ")) in the pNext chain of VkPresentInfoKHR is not a subset of the image begin presented "
                                         "(extent (%" PRIu32 ", %" PRIu32 ")).",
                                         display_present_info->srcRect.offset.x, display_present_info->srcRect.offset.y,
                                         display_present_info->srcRect.extent.width, display_present_info->srcRect.extent.height,
                                         image_state->createInfo.extent.width, image_state->createInfo.extent.height);
                    }
                }
            }

            // All physical devices and queue families are required to be able to present to any native window on Android
            if (!instance_extensions.vk_khr_android_surface) {
                auto surface_state = Get<SURFACE_STATE>(swapchain_data->createInfo.surface);
                if (!surface_state->GetQueueSupport(physical_device, queue_state->queueFamilyIndex)) {
                    skip |= LogError("VUID-vkQueuePresentKHR-pSwapchains-01292", pPresentInfo->pSwapchains[i], swapchain_loc,
                                     "image on queue that cannot present to this surface.");
                }
            }
        }
    }
    if (pPresentInfo->pNext) {
        // Verify ext struct
        const auto *present_regions = vku::FindStructInPNextChain<VkPresentRegionsKHR>(pPresentInfo->pNext);
        if (present_regions) {
            for (uint32_t i = 0; i < present_regions->swapchainCount; ++i) {
                auto swapchain_data = Get<SWAPCHAIN_NODE>(pPresentInfo->pSwapchains[i]);
                assert(swapchain_data);
                VkPresentRegionKHR region = present_regions->pRegions[i];
                const Location region_loc = present_info_loc.pNext(Struct::VkPresentRegionsKHR, Field::pRegions, i);
                for (uint32_t j = 0; j < region.rectangleCount; ++j) {
                    const Location rect_loc = region_loc.dot(Field::pRectangles, j);
                    VkRectLayerKHR rect = region.pRectangles[j];
                    // Swap offsets and extents for 90 or 270 degree preTransform rotation
                    if (swapchain_data->createInfo.preTransform &
                        (VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR | VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR |
                         VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90_BIT_KHR |
                         VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270_BIT_KHR)) {
                        std::swap(rect.offset.x, rect.offset.y);
                        std::swap(rect.extent.width, rect.extent.height);
                    }
                    if ((rect.offset.x + rect.extent.width) > swapchain_data->createInfo.imageExtent.width) {
                        skip |= LogError("VUID-VkRectLayerKHR-offset-04864", pPresentInfo->pSwapchains[i], rect_loc,
                                         "sum of offset.x (%" PRId32 ") and extent.width (%" PRIu32
                                         ") after applying preTransform (%s) is greater "
                                         "than the corresponding swapchain's imageExtent.width (%" PRIu32 ").",
                                         rect.offset.x, rect.extent.width,
                                         string_VkSurfaceTransformFlagBitsKHR(swapchain_data->createInfo.preTransform),
                                         swapchain_data->createInfo.imageExtent.width);
                    }
                    if ((rect.offset.y + rect.extent.height) > swapchain_data->createInfo.imageExtent.height) {
                        skip |= LogError("VUID-VkRectLayerKHR-offset-04864", pPresentInfo->pSwapchains[i], rect_loc,
                                         "sum of offset.y (%" PRId32 ") and extent.height (%" PRIu32
                                         ") after applying preTransform (%s) is greater "
                                         "than the corresponding swapchain's imageExtent.height (%" PRIu32 ").",
                                         rect.offset.y, rect.extent.height,
                                         string_VkSurfaceTransformFlagBitsKHR(swapchain_data->createInfo.preTransform),
                                         swapchain_data->createInfo.imageExtent.height);
                    }
                    if (rect.layer > swapchain_data->createInfo.imageArrayLayers) {
                        skip |= LogError(
                            "VUID-VkRectLayerKHR-layer-01262", pPresentInfo->pSwapchains[i], rect_loc.dot(Field::layer),
                            "layer (%" PRIu32 ") is greater than the corresponding swapchain's imageArrayLayers (%" PRIu32 ").",
                            rect.layer, swapchain_data->createInfo.imageArrayLayers);
                    }
                }
            }
        }

        const auto *present_times_info = vku::FindStructInPNextChain<VkPresentTimesInfoGOOGLE>(pPresentInfo->pNext);
        if (present_times_info) {
            if (pPresentInfo->swapchainCount != present_times_info->swapchainCount) {
                skip |= LogError("VUID-VkPresentTimesInfoGOOGLE-swapchainCount-01247", pPresentInfo->pSwapchains[0],
                                 present_info_loc.pNext(Struct::VkPresentTimesInfoGOOGLE, Field::swapchainCount),
                                 "(%" PRIu32 ") is not equal to pPresentInfo->swapchainCount (%" PRIu32 ").",
                                 present_times_info->swapchainCount, pPresentInfo->swapchainCount);
            }
        }

        const auto *present_id_info = vku::FindStructInPNextChain<VkPresentIdKHR>(pPresentInfo->pNext);
        if (present_id_info) {
            if (!enabled_features.presentId) {
                for (uint32_t i = 0; i < present_id_info->swapchainCount; i++) {
                    if (present_id_info->pPresentIds[i] != 0) {
                        skip |= LogError("VUID-VkPresentInfoKHR-pNext-06235", pPresentInfo->pSwapchains[0],
                                         present_info_loc.pNext(Struct::VkPresentIdKHR, Field::pPresentIds, i),
                                         "%" PRIu64 " is not NULL but presentId feature is not enabled.",
                                         present_id_info->pPresentIds[i]);
                    }
                }
            }
            if (pPresentInfo->swapchainCount != present_id_info->swapchainCount) {
                skip |= LogError("VUID-VkPresentIdKHR-swapchainCount-04998", pPresentInfo->pSwapchains[0],
                                 present_info_loc.pNext(Struct::VkPresentIdKHR, Field::swapchainCount),
                                 "(%" PRIu32 ") is not equal to pPresentInfo->swapchainCount (%" PRIu32 ").",
                                 present_id_info->swapchainCount, pPresentInfo->swapchainCount);
            }
            for (uint32_t i = 0; i < present_id_info->swapchainCount; i++) {
                auto swapchain_state = Get<SWAPCHAIN_NODE>(pPresentInfo->pSwapchains[i]);
                if ((present_id_info->pPresentIds[i] != 0) &&
                    (present_id_info->pPresentIds[i] <= swapchain_state->max_present_id)) {
                    skip |= LogError("VUID-VkPresentIdKHR-presentIds-04999", pPresentInfo->pSwapchains[i],
                                     present_info_loc.pNext(Struct::VkPresentIdKHR, Field::pPresentIds, i),
                                     "%" PRIu64 " and the largest presentId sent for this swapchain is %" PRIu64
                                     ". Each presentIds entry must be greater than any previous presentIds entry passed for the "
                                     "associated pSwapchains entry",
                                     present_id_info->pPresentIds[i], swapchain_state->max_present_id);
                }
            }
        }

        const auto *swapchain_present_fence_info = vku::FindStructInPNextChain<VkSwapchainPresentFenceInfoEXT>(pPresentInfo->pNext);
        if (swapchain_present_fence_info) {
            if (pPresentInfo->swapchainCount != swapchain_present_fence_info->swapchainCount) {
                skip |= LogError("VUID-VkSwapchainPresentFenceInfoEXT-swapchainCount-07757", pPresentInfo->pSwapchains[0],
                                 present_info_loc.pNext(Struct::VkSwapchainPresentFenceInfoEXT, Field::swapchainCount),
                                 "(%" PRIu32 ") is not equal to pPresentInfo->swapchainCount (%" PRIu32 ").",
                                 swapchain_present_fence_info->swapchainCount, pPresentInfo->swapchainCount);
            }

            for (uint32_t i = 0; i < swapchain_present_fence_info->swapchainCount; i++) {
                if (swapchain_present_fence_info->pFences[i]) {
                    const auto fence_state = Get<FENCE_STATE>(swapchain_present_fence_info->pFences[i]);
                    const LogObjectList objlist(queue, swapchain_present_fence_info->pFences[i]);
                    skip |=
                        ValidateFenceForSubmit(fence_state.get(), "VUID-VkSwapchainPresentFenceInfoEXT-pFences-07759",
                                               "VUID-VkSwapchainPresentFenceInfoEXT-pFences-07758", objlist,
                                               present_info_loc.pNext(Struct::VkSwapchainPresentFenceInfoEXT, Field::pFences, i));
                }
            }
        }

        const auto *swapchain_present_mode_info = vku::FindStructInPNextChain<VkSwapchainPresentModeInfoEXT>(pPresentInfo->pNext);
        if (swapchain_present_mode_info) {
            if (pPresentInfo->swapchainCount != swapchain_present_mode_info->swapchainCount) {
                skip |= LogError("VUID-VkSwapchainPresentModeInfoEXT-swapchainCount-07760", pPresentInfo->pSwapchains[0],
                                 present_info_loc.pNext(Struct::VkSwapchainPresentModeInfoEXT, Field::swapchainCount),
                                 "(%" PRIu32 ") is not equal to pPresentInfo->swapchainCount (%" PRIu32 ").",
                                 swapchain_present_mode_info->swapchainCount, pPresentInfo->swapchainCount);
            }

            for (uint32_t i = 0; i < swapchain_present_mode_info->swapchainCount; i++) {
                const VkPresentModeKHR present_mode = swapchain_present_mode_info->pPresentModes[i];
                const auto swapchain_state = Get<SWAPCHAIN_NODE>(pPresentInfo->pSwapchains[i]);
                if (!swapchain_state->present_modes.empty()) {
                    bool found_match = std::find(swapchain_state->present_modes.begin(), swapchain_state->present_modes.end(),
                                                 present_mode) != swapchain_state->present_modes.end();
                    if (!found_match) {
                        skip |= LogError("VUID-VkSwapchainPresentModeInfoEXT-pPresentModes-07761", pPresentInfo->pSwapchains[i],
                                         present_info_loc.pNext(Struct::VkSwapchainPresentModeInfoEXT, Field::presentMode),
                                         "(%s) that was not specified in a VkSwapchainPresentModesCreateInfoEXT "
                                         "structure extending VkCreateSwapchainsKHR.",
                                         string_VkPresentModeKHR(present_mode));
                    }
                } else {
                    skip |= LogError("VUID-VkSwapchainPresentModeInfoEXT-pPresentModes-07761", pPresentInfo->pSwapchains[i],
                                     present_info_loc.pNext(Struct::VkSwapchainPresentModeInfoEXT, Field::presentMode),
                                     "(%s), but a VkSwapchainPresentModesCreateInfoEXT structure was not included in the "
                                     "pNext chain of VkCreateSwapchainsKHR.",
                                     string_VkPresentModeKHR(present_mode));
                }
            }
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateReleaseSwapchainImagesEXT(VkDevice device, const VkReleaseSwapchainImagesInfoEXT *pReleaseInfo,
                                                          const ErrorObject &error_obj) const {
    bool skip = false;
    bool image_in_use = false;
    auto swapchain_state = Get<SWAPCHAIN_NODE>(pReleaseInfo->swapchain);
    if (swapchain_state) {
        const Location release_info_loc = error_obj.location.dot(Field::pReleaseInfo);
        for (uint32_t i = 0; i < pReleaseInfo->imageIndexCount; i++) {
            if (pReleaseInfo->pImageIndices[i] >= swapchain_state->images.size()) {
                skip |= LogError("VUID-VkReleaseSwapchainImagesInfoEXT-pImageIndices-07785", pReleaseInfo->swapchain,
                                 release_info_loc.dot(Field::pImageIndices, i),
                                 "%" PRIu32 " is too large, there are only %" PRIu32 " images in this swapchain.",
                                 pReleaseInfo->pImageIndices[i], static_cast<uint32_t>(swapchain_state->images.size()));
            } else if (!swapchain_state->images[pReleaseInfo->pImageIndices[i]].image_state ||
                       !swapchain_state->images[pReleaseInfo->pImageIndices[i]].acquired) {
                skip |= LogError("VUID-VkReleaseSwapchainImagesInfoEXT-pImageIndices-07785", pReleaseInfo->swapchain,
                                 release_info_loc.dot(Field::pImageIndices, i), "%" PRIu32 " was not acquired from the swapchain.",
                                 pReleaseInfo->pImageIndices[i]);
            }

            if (swapchain_state->images[i].image_state->InUse()) {
                image_in_use = true;
            }
        }

        if (image_in_use) {
            skip |= LogError("VUID-VkReleaseSwapchainImagesInfoEXT-pImageIndices-07786", pReleaseInfo->swapchain, release_info_loc,
                             "One or more of the images in this swapchain is still in use.");
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount,
                                                          const VkSwapchainCreateInfoKHR *pCreateInfos,
                                                          const VkAllocationCallbacks *pAllocator, VkSwapchainKHR *pSwapchains,
                                                          const ErrorObject &error_obj) const {
    bool skip = false;
    if (pCreateInfos) {
        for (uint32_t i = 0; i < swapchainCount; i++) {
            auto surface_state = Get<SURFACE_STATE>(pCreateInfos[i].surface);
            auto old_swapchain_state = Get<SWAPCHAIN_NODE>(pCreateInfos[i].oldSwapchain);
            skip |= ValidateCreateSwapchain(&pCreateInfos[i], surface_state.get(), old_swapchain_state.get(),
                                            error_obj.location.dot(Field::pCreateInfos, i));
        }
    }
    return skip;
}

bool CoreChecks::ValidateAcquireNextImage(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore,
                                          VkFence fence, uint32_t *pImageIndex, const Location &loc,
                                          const char *semaphore_type_vuid) const {
    bool skip = false;
    const bool version_2 = loc.function != Func::vkAcquireNextImageKHR;
    auto semaphore_state = Get<SEMAPHORE_STATE>(semaphore);
    if (semaphore_state) {
        if (semaphore_state->type != VK_SEMAPHORE_TYPE_BINARY) {
            skip |= LogError(semaphore_type_vuid, semaphore, loc, "%s is not a VK_SEMAPHORE_TYPE_BINARY.",
                             FormatHandle(semaphore).c_str());
        } else if (semaphore_state->Scope() == kSyncScopeInternal) {
            // TODO: VUIDs 01779 and 01781 cover the case where there are pending wait or signal operations on the
            // semaphore. But we don't currently have a good enough way to track when acquire & present operations
            // are completed. So it is possible to get in a condition where the semaphore is doing
            // acquire / wait / acquire and the first acquire (and thus the wait) have completed, but our state
            // isn't aware of it yet. This results in MANY false positives.
            if (!semaphore_state->CanBeSignaled()) {
                const char *vuid =
                    version_2 ? "VUID-VkAcquireNextImageInfoKHR-semaphore-01288" : "VUID-vkAcquireNextImageKHR-semaphore-01286";
                skip |= LogError(vuid, semaphore, loc, "Semaphore must not be currently signaled.");
            }
        }
    }

    auto fence_state = Get<FENCE_STATE>(fence);
    if (fence_state) {
        const LogObjectList objlist(device, fence);
        skip |= ValidateFenceForSubmit(fence_state.get(), "VUID-vkAcquireNextImageKHR-fence-01287",
                                       "VUID-vkAcquireNextImageKHR-fence-01287", objlist, loc);
    }

    auto swapchain_data = Get<SWAPCHAIN_NODE>(swapchain);
    if (swapchain_data) {
        if (swapchain_data->retired) {
            const char *vuid =
                version_2 ? "VUID-VkAcquireNextImageInfoKHR-swapchain-01675" : "VUID-vkAcquireNextImageKHR-swapchain-01285";
            skip |= LogError(vuid, swapchain, loc,
                             "This swapchain has been retired. The application can still present any images it "
                             "has acquired, but cannot acquire any more.");
        }

        const uint32_t acquired_images = swapchain_data->acquired_images;
        const uint32_t swapchain_image_count = static_cast<uint32_t>(swapchain_data->images.size());

        safe_VkSurfaceCapabilities2KHR surface_caps2{};
        if (swapchain_data->surface) {
            surface_caps2 = swapchain_data->surface->GetCapabilities(
                IsExtEnabled(device_extensions.vk_khr_get_surface_capabilities2), physical_device, nullptr, this);
        } else if (IsExtEnabled(instance_extensions.vk_google_surfaceless_query)) {
            surface_caps2 = physical_device_state->surfaceless_query_state.capabilities;
        }
        auto min_image_count = surface_caps2.surfaceCapabilities.minImageCount;
        const VkSwapchainPresentModesCreateInfoEXT *present_modes_ci =
            vku::FindStructInPNextChain<VkSwapchainPresentModesCreateInfoEXT>(swapchain_data->createInfo.pNext);
        if (present_modes_ci) {
            auto surface_state = Get<SURFACE_STATE>(swapchain_data->createInfo.surface);
            // If a SwapchainPresentModesCreateInfo struct was included, min_image_count becomes the max of the
            // minImageCount values returned via VkSurfaceCapabilitiesKHR for each of the present modes in
            // SwapchainPresentModesCreateInfo
            VkSurfaceCapabilitiesKHR surface_capabilities{};
            min_image_count = 0;
            for (uint32_t i = 0; i < present_modes_ci->presentModeCount; i++) {
                surface_capabilities =
                    surface_state->GetPresentModeSurfaceCapabilities(physical_device, present_modes_ci->pPresentModes[i]);
                if (surface_capabilities.minImageCount > min_image_count) {
                    min_image_count = surface_capabilities.minImageCount;
                }
            }
        }
        const bool too_many_already_acquired = acquired_images > swapchain_image_count - min_image_count;
        if (timeout == vvl::kU64Max && too_many_already_acquired) {
            const char *vuid = version_2 ? "VUID-vkAcquireNextImage2KHR-surface-07784" : "VUID-vkAcquireNextImageKHR-surface-07783";
            const uint32_t acquirable = swapchain_image_count - min_image_count + 1;
            skip |= LogError(vuid, swapchain, loc,
                             "Application has already previously acquired %" PRIu32 " image%s from swapchain. Only %" PRIu32
                             " %s available to be acquired using a timeout of UINT64_MAX (given the swapchain has %" PRIu32
                             ", and VkSurfaceCapabilitiesKHR::minImageCount is %" PRIu32 ").",
                             acquired_images, acquired_images > 1 ? "s" : "", acquirable, acquirable > 1 ? "are" : "is",
                             swapchain_image_count, min_image_count);
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout,
                                                    VkSemaphore semaphore, VkFence fence, uint32_t *pImageIndex,
                                                    const ErrorObject &error_obj) const {
    return ValidateAcquireNextImage(device, swapchain, timeout, semaphore, fence, pImageIndex, error_obj.location,
                                    "VUID-vkAcquireNextImageKHR-semaphore-03265");
}

bool CoreChecks::PreCallValidateAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR *pAcquireInfo,
                                                     uint32_t *pImageIndex, const ErrorObject &error_obj) const {
    bool skip = false;
    const LogObjectList objlist(pAcquireInfo->swapchain);
    const Location acquire_info_loc = error_obj.location.dot(Field::pAcquireInfo);
    skip |= ValidateDeviceMaskToPhysicalDeviceCount(pAcquireInfo->deviceMask, objlist, acquire_info_loc.dot(Field::deviceMask),
                                                    "VUID-VkAcquireNextImageInfoKHR-deviceMask-01290");
    skip |= ValidateDeviceMaskToZero(pAcquireInfo->deviceMask, objlist, acquire_info_loc.dot(Field::deviceMask),
                                     "VUID-VkAcquireNextImageInfoKHR-deviceMask-01291");
    skip |= ValidateAcquireNextImage(device, pAcquireInfo->swapchain, pAcquireInfo->timeout, pAcquireInfo->semaphore,
                                     pAcquireInfo->fence, pImageIndex, error_obj.location,
                                     "VUID-VkAcquireNextImageInfoKHR-semaphore-03266");
    return skip;
}

bool CoreChecks::PreCallValidateWaitForPresentKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t presentId, uint64_t timeout,
                                                  const ErrorObject &error_obj) const {
    bool skip = false;
    if (!enabled_features.presentWait) {
        skip |= LogError("VUID-vkWaitForPresentKHR-presentWait-06234", swapchain, error_obj.location,
                         "presentWait feature is not enabled.");
    }
    auto swapchain_state = Get<SWAPCHAIN_NODE>(swapchain);
    if (swapchain_state) {
        if (swapchain_state->retired) {
            skip |= LogError("VUID-vkWaitForPresentKHR-swapchain-04997", swapchain, error_obj.location,
                             "called with a retired swapchain.");
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface,
                                                  const VkAllocationCallbacks *pAllocator, const ErrorObject &error_obj) const {
    auto surface_state = Get<SURFACE_STATE>(surface);
    bool skip = false;
    if ((surface_state) && (surface_state->swapchain)) {
        skip |= LogError("VUID-vkDestroySurfaceKHR-surface-01266", instance, error_obj.location,
                         "called before its associated VkSwapchainKHR was destroyed.");
    }
    return skip;
}

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
bool CoreChecks::PreCallValidateGetPhysicalDeviceWaylandPresentationSupportKHR(VkPhysicalDevice physicalDevice,
                                                                               uint32_t queueFamilyIndex,
                                                                               struct wl_display *display,
                                                                               const ErrorObject &error_obj) const {
    auto pd_state = Get<PHYSICAL_DEVICE_STATE>(physicalDevice);
    return ValidateQueueFamilyIndex(pd_state.get(), queueFamilyIndex,
                                    "VUID-vkGetPhysicalDeviceWaylandPresentationSupportKHR-queueFamilyIndex-01306",
                                    error_obj.location.dot(Field::queueFamilyIndex));
}
#endif  // VK_USE_PLATFORM_WAYLAND_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR
bool CoreChecks::PreCallValidateGetPhysicalDeviceWin32PresentationSupportKHR(VkPhysicalDevice physicalDevice,
                                                                             uint32_t queueFamilyIndex,
                                                                             const ErrorObject &error_obj) const {
    auto pd_state = Get<PHYSICAL_DEVICE_STATE>(physicalDevice);
    return ValidateQueueFamilyIndex(pd_state.get(), queueFamilyIndex,
                                    "VUID-vkGetPhysicalDeviceWin32PresentationSupportKHR-queueFamilyIndex-01309",
                                    error_obj.location.dot(Field::queueFamilyIndex));
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_XCB_KHR
bool CoreChecks::PreCallValidateGetPhysicalDeviceXcbPresentationSupportKHR(VkPhysicalDevice physicalDevice,
                                                                           uint32_t queueFamilyIndex, xcb_connection_t *connection,
                                                                           xcb_visualid_t visual_id,
                                                                           const ErrorObject &error_obj) const {
    auto pd_state = Get<PHYSICAL_DEVICE_STATE>(physicalDevice);
    return ValidateQueueFamilyIndex(pd_state.get(), queueFamilyIndex,
                                    "VUID-vkGetPhysicalDeviceXcbPresentationSupportKHR-queueFamilyIndex-01312",
                                    error_obj.location.dot(Field::queueFamilyIndex));
}
#endif  // VK_USE_PLATFORM_XCB_KHR

#ifdef VK_USE_PLATFORM_XLIB_KHR
bool CoreChecks::PreCallValidateGetPhysicalDeviceXlibPresentationSupportKHR(VkPhysicalDevice physicalDevice,
                                                                            uint32_t queueFamilyIndex, Display *dpy,
                                                                            VisualID visualID, const ErrorObject &error_obj) const {
    auto pd_state = Get<PHYSICAL_DEVICE_STATE>(physicalDevice);
    return ValidateQueueFamilyIndex(pd_state.get(), queueFamilyIndex,
                                    "VUID-vkGetPhysicalDeviceXlibPresentationSupportKHR-queueFamilyIndex-01315",
                                    error_obj.location.dot(Field::queueFamilyIndex));
}
#endif  // VK_USE_PLATFORM_XLIB_KHR

#ifdef VK_USE_PLATFORM_SCREEN_QNX
bool CoreChecks::PreCallValidateGetPhysicalDeviceScreenPresentationSupportQNX(VkPhysicalDevice physicalDevice,
                                                                              uint32_t queueFamilyIndex,
                                                                              struct _screen_window *window,
                                                                              const ErrorObject &error_obj) const {
    auto pd_state = Get<PHYSICAL_DEVICE_STATE>(physicalDevice);
    return ValidateQueueFamilyIndex(pd_state.get(), queueFamilyIndex,
                                    "VUID-vkGetPhysicalDeviceScreenPresentationSupportQNX-queueFamilyIndex-04743",
                                    error_obj.location.dot(Field::queueFamilyIndex));
}
#endif  // VK_USE_PLATFORM_SCREEN_QNX

bool CoreChecks::PreCallValidateGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                   VkSurfaceKHR surface, VkBool32 *pSupported,
                                                                   const ErrorObject &error_obj) const {
    auto pd_state = Get<PHYSICAL_DEVICE_STATE>(physicalDevice);
    return ValidateQueueFamilyIndex(pd_state.get(), queueFamilyIndex,
                                    "VUID-vkGetPhysicalDeviceSurfaceSupportKHR-queueFamilyIndex-01269",
                                    error_obj.location.dot(Field::queueFamilyIndex));
}

bool CoreChecks::PreCallValidateGetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice physicalDevice, uint32_t planeIndex,
                                                                    uint32_t *pDisplayCount, VkDisplayKHR *pDisplays,
                                                                    const ErrorObject &error_obj) const {
    bool skip = false;
    skip |= ValidateGetPhysicalDeviceDisplayPlanePropertiesKHRQuery(physicalDevice, planeIndex,
                                                                    error_obj.location.dot(Field::planeIndex));
    return skip;
}

bool CoreChecks::PreCallValidateGetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkDisplayModeKHR mode,
                                                               uint32_t planeIndex, VkDisplayPlaneCapabilitiesKHR *pCapabilities,
                                                               const ErrorObject &error_obj) const {
    bool skip = false;
    skip |= ValidateGetPhysicalDeviceDisplayPlanePropertiesKHRQuery(physicalDevice, planeIndex,
                                                                    error_obj.location.dot(Field::planeIndex));
    return skip;
}

bool CoreChecks::PreCallValidateGetDisplayPlaneCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                                const VkDisplayPlaneInfo2KHR *pDisplayPlaneInfo,
                                                                VkDisplayPlaneCapabilities2KHR *pCapabilities,
                                                                const ErrorObject &error_obj) const {
    bool skip = false;
    skip |= ValidateGetPhysicalDeviceDisplayPlanePropertiesKHRQuery(
        physicalDevice, pDisplayPlaneInfo->planeIndex, error_obj.location.dot(Field::pDisplayPlaneInfo).dot(Field::planeIndex));
    return skip;
}

bool CoreChecks::PreCallValidateCreateDisplayPlaneSurfaceKHR(VkInstance instance, const VkDisplaySurfaceCreateInfoKHR *pCreateInfo,
                                                             const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface,
                                                             const ErrorObject &error_obj) const {
    bool skip = false;
    const VkDisplayModeKHR display_mode = pCreateInfo->displayMode;
    const uint32_t plane_index = pCreateInfo->planeIndex;
    const Location create_info_loc = error_obj.location.dot(Field::pCreateInfo);
    if (pCreateInfo->alphaMode == VK_DISPLAY_PLANE_ALPHA_GLOBAL_BIT_KHR) {
        const float global_alpha = pCreateInfo->globalAlpha;
        if ((global_alpha > 1.0f) || (global_alpha < 0.0f)) {
            skip |= LogError("VUID-VkDisplaySurfaceCreateInfoKHR-alphaMode-01254", display_mode,
                             create_info_loc.dot(Field::globalAlpha),
                             "is %f, but alphaMode is VK_DISPLAY_PLANE_ALPHA_GLOBAL_BIT_KHR.", global_alpha);
        }
    }

    auto dm_state = Get<DISPLAY_MODE_STATE>(display_mode);
    if (dm_state != nullptr) {
        // Get physical device from VkDisplayModeKHR state tracking
        const VkPhysicalDevice physical_device = dm_state->physical_device;
        auto pd_state = Get<PHYSICAL_DEVICE_STATE>(physical_device);
        VkPhysicalDeviceProperties device_properties = {};
        DispatchGetPhysicalDeviceProperties(physical_device, &device_properties);

        const uint32_t width = pCreateInfo->imageExtent.width;
        const uint32_t height = pCreateInfo->imageExtent.height;
        if (width >= device_properties.limits.maxImageDimension2D) {
            skip |= LogError("VUID-VkDisplaySurfaceCreateInfoKHR-width-01256", display_mode,
                             create_info_loc.dot(Field::imageExtent).dot(Field::width),
                             "(%" PRIu32 ") exceeds device limit maxImageDimension2D (%" PRIu32 ").", width,
                             device_properties.limits.maxImageDimension2D);
        }
        if (height >= device_properties.limits.maxImageDimension2D) {
            skip |= LogError("VUID-VkDisplaySurfaceCreateInfoKHR-width-01256", display_mode,
                             create_info_loc.dot(Field::imageExtent).dot(Field::height),
                             "(%" PRIu32 ") exceeds device limit maxImageDimension2D (%" PRIu32 ").", height,
                             device_properties.limits.maxImageDimension2D);
        }

        if (pd_state->vkGetPhysicalDeviceDisplayPlanePropertiesKHR_called) {
            if (plane_index >= pd_state->display_plane_property_count) {
                skip |= LogError("VUID-VkDisplaySurfaceCreateInfoKHR-planeIndex-01252", display_mode,
                                 create_info_loc.dot(Field::planeIndex),
                                 "(%" PRIu32 ") must be in the range [0, %" PRIu32
                                 "] that was returned by "
                                 "vkGetPhysicalDeviceDisplayPlanePropertiesKHR "
                                 "or vkGetPhysicalDeviceDisplayPlaneProperties2KHR. Do you have the plane index hardcoded?",
                                 plane_index, pd_state->display_plane_property_count - 1);
            } else {
                // call here once we know the plane index used is a valid plane index
                VkDisplayPlaneCapabilitiesKHR plane_capabilities;
                DispatchGetDisplayPlaneCapabilitiesKHR(physical_device, display_mode, plane_index, &plane_capabilities);

                if ((pCreateInfo->alphaMode & plane_capabilities.supportedAlpha) == 0) {
                    skip |= LogError("VUID-VkDisplaySurfaceCreateInfoKHR-alphaMode-01255", display_mode, create_info_loc,
                                     "alphaMode is %s but planeIndex %" PRIu32
                                     " supportedAlpha (0x%x) "
                                     "does not support the mode.",
                                     string_VkDisplayPlaneAlphaFlagBitsKHR(pCreateInfo->alphaMode), plane_index,
                                     plane_capabilities.supportedAlpha);
                }
            }
        }
    }

    return skip;
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
bool CoreChecks::PreCallValidateAcquireFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain,
                                                                  const ErrorObject &error_obj) const {
    bool skip = false;

    auto swapchain_state = Get<SWAPCHAIN_NODE>(swapchain);
    if (swapchain_state) {
        if (swapchain_state->retired) {
            skip |= LogError("VUID-vkAcquireFullScreenExclusiveModeEXT-swapchain-02674", device, error_obj.location,
                             "swapchain %s is retired.", FormatHandle(swapchain).c_str());
        }
        const auto *surface_full_screen_exclusive_info =
            vku::FindStructInPNextChain<VkSurfaceFullScreenExclusiveInfoEXT>(swapchain_state->createInfo.pNext);
        if (!surface_full_screen_exclusive_info ||
            surface_full_screen_exclusive_info->fullScreenExclusive != VK_FULL_SCREEN_EXCLUSIVE_APPLICATION_CONTROLLED_EXT) {
            skip |=
                LogError("VUID-vkAcquireFullScreenExclusiveModeEXT-swapchain-02675", device, error_obj.location,
                         "swapchain %s was not created with VkSurfaceFullScreenExclusiveInfoEXT in "
                         "the pNext chain with fullScreenExclusive equal to VK_FULL_SCREEN_EXCLUSIVE_APPLICATION_CONTROLLED_EXT.",
                         FormatHandle(swapchain).c_str());
        }
        if (swapchain_state->exclusive_full_screen_access) {
            skip |= LogError("VUID-vkAcquireFullScreenExclusiveModeEXT-swapchain-02676", device, error_obj.location,
                             "swapchain %s already has exclusive full-screen access.", FormatHandle(swapchain).c_str());
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateReleaseFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain,
                                                                  const ErrorObject &error_obj) const {
    bool skip = false;

    const auto swapchain_state = Get<SWAPCHAIN_NODE>(swapchain);
    if (swapchain_state) {
        if (swapchain_state->retired) {
            skip |= LogError("VUID-vkReleaseFullScreenExclusiveModeEXT-swapchain-02677", device, error_obj.location,
                             "swapchain %s is retired.", FormatHandle(swapchain).c_str());
        }
        const auto *surface_full_screen_exclusive_info =
            vku::FindStructInPNextChain<VkSurfaceFullScreenExclusiveInfoEXT>(swapchain_state->createInfo.pNext);
        if (!surface_full_screen_exclusive_info ||
            surface_full_screen_exclusive_info->fullScreenExclusive != VK_FULL_SCREEN_EXCLUSIVE_APPLICATION_CONTROLLED_EXT) {
            skip |=
                LogError("VUID-vkReleaseFullScreenExclusiveModeEXT-swapchain-02678", device, error_obj.location,
                         "swapchain %s was not created with VkSurfaceFullScreenExclusiveInfoEXT in "
                         "the pNext chain with fullScreenExclusive equal to VK_FULL_SCREEN_EXCLUSIVE_APPLICATION_CONTROLLED_EXT.",
                         FormatHandle(swapchain).c_str());
        }
    }

    return skip;
}
#endif

bool CoreChecks::ValidatePhysicalDeviceSurfaceSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, const char *vuid,
                                                      const Location &loc) const {
    bool skip = false;

    auto pd_state = Get<PHYSICAL_DEVICE_STATE>(physicalDevice);
    auto surface_state = Get<SURFACE_STATE>(surface);
    if (pd_state && surface_state) {
        bool is_supported = false;
        for (uint32_t i = 0; i < pd_state->queue_family_properties.size(); i++) {
            if (surface_state->GetQueueSupport(physicalDevice, i)) {
                is_supported = true;
                break;
            }
        }
        if (!is_supported) {
            skip |= LogError(vuid, physicalDevice, loc, "surface is not supported by the physicalDevice.");
        }
    }

    return skip;
}

#ifdef VK_USE_PLATFORM_WIN32_KHR

bool CoreChecks::PreCallValidateGetDeviceGroupSurfacePresentModes2EXT(VkDevice device,
                                                                      const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo,
                                                                      VkDeviceGroupPresentModeFlagsKHR *pModes,
                                                                      const ErrorObject &error_obj) const {
    bool skip = false;

    if (physical_device_count == 1) {
        ValidationObject *device_object = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
        skip |= ValidatePhysicalDeviceSurfaceSupport(device_object->physical_device, pSurfaceInfo->surface,
                                                     "VUID-vkGetDeviceGroupSurfacePresentModes2EXT-pSurfaceInfo-06213",
                                                     error_obj.location);
    } else {
        for (uint32_t i = 0; i < physical_device_count; ++i) {
            skip |= ValidatePhysicalDeviceSurfaceSupport(device_group_create_info.pPhysicalDevices[i], pSurfaceInfo->surface,
                                                         "VUID-vkGetDeviceGroupSurfacePresentModes2EXT-pSurfaceInfo-06213",
                                                         error_obj.location);
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateGetPhysicalDeviceSurfacePresentModes2EXT(VkPhysicalDevice physicalDevice,
                                                                         const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo,
                                                                         uint32_t *pPresentModeCount,
                                                                         VkPresentModeKHR *pPresentModes,
                                                                         const ErrorObject &error_obj) const {
    bool skip = false;

    skip |= ValidatePhysicalDeviceSurfaceSupport(physicalDevice, pSurfaceInfo->surface,
                                                 "VUID-vkGetPhysicalDeviceSurfacePresentModes2EXT-pSurfaceInfo-06522",
                                                 error_obj.location);

    return skip;
}

#endif

bool CoreChecks::PreCallValidateGetDeviceGroupSurfacePresentModesKHR(VkDevice device, VkSurfaceKHR surface,
                                                                     VkDeviceGroupPresentModeFlagsKHR *pModes,
                                                                     const ErrorObject &error_obj) const {
    bool skip = false;

    if (physical_device_count == 1) {
        ValidationObject *device_object = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
        skip |=
            ValidatePhysicalDeviceSurfaceSupport(device_object->physical_device, surface,
                                                 "VUID-vkGetDeviceGroupSurfacePresentModesKHR-surface-06212", error_obj.location);
    } else {
        for (uint32_t i = 0; i < physical_device_count; ++i) {
            skip |= ValidatePhysicalDeviceSurfaceSupport(device_group_create_info.pPhysicalDevices[i], surface,
                                                         "VUID-vkGetDeviceGroupSurfacePresentModesKHR-surface-06212",
                                                         error_obj.location);
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateGetPhysicalDevicePresentRectanglesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                      uint32_t *pRectCount, VkRect2D *pRects,
                                                                      const ErrorObject &error_obj) const {
    bool skip = false;

    skip |= ValidatePhysicalDeviceSurfaceSupport(physicalDevice, surface,
                                                 "VUID-vkGetPhysicalDevicePresentRectanglesKHR-surface-06211", error_obj.location);

    return skip;
}

bool CoreChecks::PreCallValidateGetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                         VkSurfaceCapabilities2EXT *pSurfaceCapabilities,
                                                                         const ErrorObject &error_obj) const {
    bool skip = false;

    skip |= ValidatePhysicalDeviceSurfaceSupport(
        physicalDevice, surface, "VUID-vkGetPhysicalDeviceSurfaceCapabilities2EXT-surface-06211", error_obj.location);

    return skip;
}

bool CoreChecks::PreCallValidateGetPhysicalDeviceSurfaceCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                                         const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo,
                                                                         VkSurfaceCapabilities2KHR *pSurfaceCapabilities,
                                                                         const ErrorObject &error_obj) const {
    bool skip = false;

    skip |= ValidatePhysicalDeviceSurfaceSupport(physicalDevice, pSurfaceInfo->surface,
                                                 "VUID-vkGetPhysicalDeviceSurfaceCapabilities2KHR-pSurfaceInfo-06522",
                                                 error_obj.location);

    const auto surface_state = Get<SURFACE_STATE>(pSurfaceInfo->surface);

    if (IsExtEnabled(device_extensions.vk_ext_surface_maintenance1)) {
        const auto *surface_present_mode = vku::FindStructInPNextChain<VkSurfacePresentModeEXT>(pSurfaceInfo->pNext);
        if (surface_present_mode) {
            VkPresentModeKHR present_mode = surface_present_mode->presentMode;
            std::vector<VkPresentModeKHR> present_modes{};
            if (surface_state) {
                present_modes = surface_state->GetPresentModes(physicalDevice, this);
            }
            bool found_match = std::find(present_modes.begin(), present_modes.end(), present_mode) != present_modes.end();
            if (!found_match) {
                skip |=
                    LogError("VUID-VkSurfacePresentModeEXT-presentMode-07780", device, error_obj.location,
                             "is called with VK_EXT_surface_maintenance1 enabled and "
                             "a VkSurfacePresentModeEXT structure included in "
                             "the pNext chain of VkPhysicalDeviceSurfaceInfo2KHR, but the specified presentMode (%s) is not among "
                             "those returned by vkGetPhysicalDevicePresentModesKHR().",
                             string_VkPresentModeKHR(present_mode));
            }
        }
    }

#if defined(VK_USE_PLATFORM_WIN32_KHR)
    if (IsExtEnabled(device_extensions.vk_khr_win32_surface) && IsExtEnabled(device_extensions.vk_ext_full_screen_exclusive)) {
        if (surface_state) {
            if (const auto *full_screen_info = vku::FindStructInPNextChain<VkSurfaceFullScreenExclusiveInfoEXT>(pSurfaceInfo->pNext);
                full_screen_info && full_screen_info->fullScreenExclusive == VK_FULL_SCREEN_EXCLUSIVE_APPLICATION_CONTROLLED_EXT) {
                if (const auto *win32_full_screen_info =
                        vku::FindStructInPNextChain<VkSurfaceFullScreenExclusiveWin32InfoEXT>(pSurfaceInfo->pNext);
                    !win32_full_screen_info) {
                    const LogObjectList objlist(device, pSurfaceInfo->surface);
                    skip |= LogError("VUID-VkPhysicalDeviceSurfaceInfo2KHR-pNext-02672", objlist,
                                     error_obj.location.dot(Field::pSurfaceInfo)
                                         .pNext(Struct::VkSurfaceFullScreenExclusiveInfoEXT, Field::fullScreenExclusive),
                                     "is VK_FULL_SCREEN_EXCLUSIVE_APPLICATION_CONTROLLED_EXT, but does not contain "
                                     "a VkSurfaceFullScreenExclusiveWin32InfoEXT structure.");
                }
            }
        }
    }
#endif

    return skip;
}

bool CoreChecks::PreCallValidateGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                        VkSurfaceCapabilitiesKHR *pSurfaceCapabilities,
                                                                        const ErrorObject &error_obj) const {
    bool skip = false;

    skip |= ValidatePhysicalDeviceSurfaceSupport(
        physicalDevice, surface, "VUID-vkGetPhysicalDeviceSurfaceCapabilitiesKHR-surface-06211", error_obj.location);

    return skip;
}

bool CoreChecks::PreCallValidateGetPhysicalDeviceSurfaceFormats2KHR(VkPhysicalDevice physicalDevice,
                                                                    const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo,
                                                                    uint32_t *pSurfaceFormatCount,
                                                                    VkSurfaceFormat2KHR *pSurfaceFormats,
                                                                    const ErrorObject &error_obj) const {
    bool skip = false;

    skip |= ValidatePhysicalDeviceSurfaceSupport(
        physicalDevice, pSurfaceInfo->surface, "VUID-vkGetPhysicalDeviceSurfaceFormats2KHR-pSurfaceInfo-06522", error_obj.location);

    return skip;
}

bool CoreChecks::PreCallValidateGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                   uint32_t *pSurfaceFormatCount,
                                                                   VkSurfaceFormatKHR *pSurfaceFormats,
                                                                   const ErrorObject &error_obj) const {
    bool skip = false;

    skip |= ValidatePhysicalDeviceSurfaceSupport(physicalDevice, surface, "VUID-vkGetPhysicalDeviceSurfaceFormatsKHR-surface-06525",
                                                 error_obj.location);

    return skip;
}

bool CoreChecks::PreCallValidateGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                        uint32_t *pPresentModeCount,
                                                                        VkPresentModeKHR *pPresentModes,
                                                                        const ErrorObject &error_obj) const {
    bool skip = false;

    skip |= ValidatePhysicalDeviceSurfaceSupport(
        physicalDevice, surface, "VUID-vkGetPhysicalDeviceSurfacePresentModesKHR-surface-06525", error_obj.location);

    return skip;
}

bool CoreChecks::ValidateGetPhysicalDeviceDisplayPlanePropertiesKHRQuery(VkPhysicalDevice physicalDevice, uint32_t planeIndex,
                                                                         const Location &loc) const {
    bool skip = false;
    auto pd_state = Get<PHYSICAL_DEVICE_STATE>(physicalDevice);
    if (pd_state->vkGetPhysicalDeviceDisplayPlanePropertiesKHR_called) {
        if (planeIndex >= pd_state->display_plane_property_count) {
            skip |= LogError("VUID-vkGetDisplayPlaneSupportedDisplaysKHR-planeIndex-01249", physicalDevice, loc,
                             "is %" PRIu32 ", but vkGetPhysicalDeviceDisplayPlaneProperties(2)KHR returned %" PRIu32
                             ". (Do you have the plane index hardcoded?).",
                             planeIndex, pd_state->display_plane_property_count);
        }
    }

    return skip;
}
