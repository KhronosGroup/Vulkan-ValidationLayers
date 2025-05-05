/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (C) 2015-2024 Google Inc.
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

#include "state_tracker/wsi_state.h"
#include "state_tracker/image_state.h"
#include "state_tracker/state_tracker.h"
#include "state_tracker/fence_state.h"
#include "state_tracker/semaphore_state.h"
#include "generated/dispatch_functions.h"

static vku::safe_VkImageCreateInfo GetImageCreateInfo(const VkSwapchainCreateInfoKHR *pCreateInfo) {
    VkImageCreateInfo image_ci = vku::InitStructHelper();
    // Pull out the format list only. This stack variable will get copied onto the heap
    // by the 'safe' constructor used to build the return value below.
    VkImageFormatListCreateInfo fmt_info;
    auto chain_fmt_info = vku::FindStructInPNextChain<VkImageFormatListCreateInfo>(pCreateInfo->pNext);
    if (chain_fmt_info) {
        fmt_info = *chain_fmt_info;
        fmt_info.pNext = nullptr;
        image_ci.pNext = &fmt_info;
    } else {
        image_ci.pNext = nullptr;
    }
    image_ci.flags = 0;  // to be updated below
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = pCreateInfo->imageFormat;
    image_ci.extent.width = pCreateInfo->imageExtent.width;
    image_ci.extent.height = pCreateInfo->imageExtent.height;
    image_ci.extent.depth = 1;
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = pCreateInfo->imageArrayLayers;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = pCreateInfo->imageUsage;
    image_ci.sharingMode = pCreateInfo->imageSharingMode;
    image_ci.queueFamilyIndexCount = pCreateInfo->queueFamilyIndexCount;
    image_ci.pQueueFamilyIndices = pCreateInfo->pQueueFamilyIndices;
    image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    if (pCreateInfo->flags & VK_SWAPCHAIN_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT_KHR) {
        image_ci.flags |= VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT;
    }
    if (pCreateInfo->flags & VK_SWAPCHAIN_CREATE_PROTECTED_BIT_KHR) {
        image_ci.flags |= VK_IMAGE_CREATE_PROTECTED_BIT;
    }
    if (pCreateInfo->flags & VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR) {
        image_ci.flags |= (VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT | VK_IMAGE_CREATE_EXTENDED_USAGE_BIT);
    }
    return vku::safe_VkImageCreateInfo(&image_ci);
}

namespace vvl {

void SwapchainImage::ResetPresentWaitSemaphores() {
    for (auto &semaphore : present_wait_semaphores) {
        semaphore->ClearSwapchainWaitInfo();
    }
    present_wait_semaphores.clear();
}

Swapchain::Swapchain(vvl::DeviceState &dev_data_, const VkSwapchainCreateInfoKHR *pCreateInfo, VkSwapchainKHR handle)
    : StateObject(handle, kVulkanObjectTypeSwapchainKHR),
      safe_create_info(pCreateInfo),
      create_info(*safe_create_info.ptr()),
      images(),
      exclusive_full_screen_access(false),
      shared_presentable(VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR == pCreateInfo->presentMode ||
                         VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR == pCreateInfo->presentMode),
      image_create_info(GetImageCreateInfo(pCreateInfo)),
      dev_data(dev_data_) {
    // Initialize with visible values for debugging purposes.
    // This helps to show used slots during the first few frames.
    acquire_history.fill(vvl::kU32Max);
}

void Swapchain::PresentImage(uint32_t image_index, uint64_t present_id, const SubmissionReference &present_submission_ref,
                             vvl::span<std::shared_ptr<vvl::Semaphore>> present_wait_semaphores) {
    if (image_index >= images.size()) return;
    assert(acquired_images > 0);
    if (!shared_presentable) {
        acquired_images--;
        images[image_index].acquired = false;
        images[image_index].acquire_semaphore.reset();
        images[image_index].acquire_fence.reset();
    } else {
        images[image_index].image_state->layout_locked = true;
    }
    images[image_index].present_submission_ref = present_submission_ref;

    images[image_index].present_wait_semaphores.clear();
    for (const auto &semaphore : present_wait_semaphores) {
        images[image_index].present_wait_semaphores.emplace_back(semaphore);
    }

    if (present_id > max_present_id) {
        max_present_id = present_id;
    }
}

void Swapchain::ReleaseImage(uint32_t image_index) {
    if (image_index >= images.size()) return;
    assert(acquired_images > 0);
    acquired_images--;
    images[image_index].acquired = false;
    images[image_index].acquire_semaphore.reset();
    images[image_index].acquire_fence.reset();
    images[image_index].ResetPresentWaitSemaphores();
}

void Swapchain::AcquireImage(uint32_t image_index, const std::shared_ptr<vvl::Semaphore> &semaphore_state,
                             const std::shared_ptr<vvl::Fence> &fence_state) {
    acquired_images++;
    images[image_index].acquired = true;
    images[image_index].acquire_semaphore = semaphore_state;
    images[image_index].acquire_fence = fence_state;
    if (fence_state && images[image_index].present_submission_ref.has_value()) {
        fence_state->SetPresentSubmissionRef(*images[image_index].present_submission_ref);
        images[image_index].present_submission_ref.reset();
    }
    if (shared_presentable) {
        images[image_index].image_state->shared_presentable = shared_presentable;
    }
    images[image_index].ResetPresentWaitSemaphores();

    acquire_history[acquire_count % acquire_history_max_length] = image_index;
    acquire_count++;
}

void Swapchain::Destroy() {
    for (auto &swapchain_image : images) {
        swapchain_image.ResetPresentWaitSemaphores();
        RemoveParent(swapchain_image.image_state);
        dev_data.Destroy<vvl::Image>(swapchain_image.image_state->VkHandle());
        // NOTE: We don't have access to dev_data.fake_memory.Free() here, but it is currently a no-op
    }
    images.clear();
    if (surface) {
        surface->RemoveParent(this);
        surface = nullptr;
    }
    StateObject::Destroy();
}

void Swapchain::NotifyInvalidate(const StateObject::NodeList &invalid_nodes, bool unlink) {
    StateObject::NotifyInvalidate(invalid_nodes, unlink);
    if (unlink) {
        surface = nullptr;
    }
}

SwapchainImage Swapchain::GetSwapChainImage(uint32_t index) const {
    if (index < images.size()) {
        return images[index];
    }
    return SwapchainImage();
}

std::shared_ptr<const vvl::Image> Swapchain::GetSwapChainImageShared(uint32_t index) const {
    const SwapchainImage swapchain_image(GetSwapChainImage(index));
    if (swapchain_image.image_state) {
        return swapchain_image.image_state->shared_from_this();
    }
    return std::shared_ptr<const vvl::Image>();
}

uint32_t Swapchain::GetAcquireHistoryLength() const {
    return (acquire_count >= acquire_history_max_length) ? acquire_history_max_length : acquire_count;
}

uint32_t Swapchain::GetAcquiredImageIndexFromHistory(uint32_t acquire_history_index) const {
    const uint32_t history_length = GetAcquireHistoryLength();
    assert(acquire_history_index < history_length);

    const uint32_t global_start_index = acquire_count - history_length;
    const uint32_t global_index = global_start_index + acquire_history_index;
    const uint32_t ring_buffer_index = global_index % acquire_history_max_length;

    const uint32_t acquire_image_index = acquire_history[ring_buffer_index];
    assert(acquire_image_index != vvl::kU32Max);
    return acquire_image_index;
}

void Surface::Destroy() {
    if (swapchain) {
        swapchain = nullptr;
    }
    StateObject::Destroy();
}

void Surface::RemoveParent(StateObject *parent_node) {
    if (swapchain == parent_node) {
        swapchain = nullptr;
    }
    StateObject::RemoveParent(parent_node);
}

void Surface::SetQueueSupport(VkPhysicalDevice phys_dev, uint32_t qfi, bool supported) {
    auto guard = Lock();
    assert(phys_dev);
    GpuQueue key{phys_dev, qfi};
    gpu_queue_support_[key] = supported;
}

bool Surface::GetQueueSupport(VkPhysicalDevice phys_dev, uint32_t qfi) const {
    auto guard = Lock();
    assert(phys_dev);
    GpuQueue key{phys_dev, qfi};
    auto iter = gpu_queue_support_.find(key);
    if (iter != gpu_queue_support_.end()) {
        return iter->second;
    }
    VkBool32 supported = VK_FALSE;
    DispatchGetPhysicalDeviceSurfaceSupportKHR(phys_dev, qfi, VkHandle(), &supported);
    gpu_queue_support_[key] = (supported == VK_TRUE);
    return supported == VK_TRUE;
}

// Save data from vkGetPhysicalDeviceSurfacePresentModes
void Surface::SetPresentModes(VkPhysicalDevice phys_dev, vvl::span<const VkPresentModeKHR> modes) {
    auto guard = Lock();
    cache_[phys_dev].present_modes.emplace(modes.begin(), modes.end());
}

// Helper for data obtained from vkGetPhysicalDeviceSurfacePresentModesKHR
std::vector<VkPresentModeKHR> Surface::GetPresentModes(VkPhysicalDevice phys_dev) const {
    if (auto guard = Lock(); auto cache = GetPhysDevCache(phys_dev)) {
        if (cache->present_modes.has_value()) {
            return cache->present_modes.value();
        }
    }
    uint32_t count = 0;
    if (DispatchGetPhysicalDeviceSurfacePresentModesKHR(phys_dev, VkHandle(), &count, nullptr) != VK_SUCCESS) {
        return {};
    }
    std::vector<VkPresentModeKHR> present_modes(count);
    if (DispatchGetPhysicalDeviceSurfacePresentModesKHR(phys_dev, VkHandle(), &count, present_modes.data()) != VK_SUCCESS) {
        return {};
    }
    return present_modes;
}

void Surface::SetFormats(VkPhysicalDevice phys_dev, std::vector<vku::safe_VkSurfaceFormat2KHR> &&fmts) {
    auto guard = Lock();
    assert(phys_dev);
    formats_[phys_dev] = std::move(fmts);
}

vvl::span<const vku::safe_VkSurfaceFormat2KHR> Surface::GetFormats(bool get_surface_capabilities2, VkPhysicalDevice phys_dev,
                                                                   const void *surface_info2_pnext) const {
    auto guard = Lock();

    // TODO: BUG: format also depends on pNext. Rework this function similar to GetSurfaceCapabilities
    if (const auto search = formats_.find(phys_dev); search != formats_.end()) {
        vvl::span<const vku::safe_VkSurfaceFormat2KHR>(search->second);
    }

    std::vector<vku::safe_VkSurfaceFormat2KHR> result;
    if (get_surface_capabilities2) {
        VkPhysicalDeviceSurfaceInfo2KHR surface_info2 = vku::InitStructHelper();
        surface_info2.pNext = surface_info2_pnext;
        surface_info2.surface = VkHandle();
        uint32_t count = 0;
        if (DispatchGetPhysicalDeviceSurfaceFormats2KHR(phys_dev, &surface_info2, &count, nullptr) != VK_SUCCESS) {
            return {};
        }
        std::vector<VkSurfaceFormat2KHR> formats2(count, vku::InitStruct<VkSurfaceFormat2KHR>());

        if (DispatchGetPhysicalDeviceSurfaceFormats2KHR(phys_dev, &surface_info2, &count, formats2.data()) != VK_SUCCESS) {
            result.clear();
        } else {
            result.resize(count);
            for (uint32_t surface_format_index = 0; surface_format_index < count; ++surface_format_index) {
                result.emplace_back(&formats2[surface_format_index]);
            }
        }
    } else {
        std::vector<VkSurfaceFormatKHR> formats;
        uint32_t count = 0;
        if (DispatchGetPhysicalDeviceSurfaceFormatsKHR(phys_dev, VkHandle(), &count, nullptr) != VK_SUCCESS) {
            return {};
        }
        formats.resize(count);

        if (DispatchGetPhysicalDeviceSurfaceFormatsKHR(phys_dev, VkHandle(), &count, formats.data()) != VK_SUCCESS) {
            result.clear();
        } else {
            result.reserve(count);
            VkSurfaceFormat2KHR format2 = vku::InitStructHelper();
            for (const auto &format : formats) {
                format2.surfaceFormat = format;
                result.emplace_back(&format2);
            }
        }
    }
    formats_[phys_dev] = std::move(result);
    return vvl::span<const vku::safe_VkSurfaceFormat2KHR>(formats_[phys_dev]);
}

const Surface::PresentModeInfo *Surface::PhysDevCache::GetPresentModeInfo(VkPresentModeKHR present_mode) const {
    for (auto &info : present_mode_infos) {
        if (info.present_mode == present_mode) {
            return &info;
        }
    }
    return nullptr;
}

const Surface::PhysDevCache *Surface::GetPhysDevCache(VkPhysicalDevice phys_dev) const {
    auto it = cache_.find(phys_dev);
    return (it == cache_.end()) ? nullptr : &it->second;
}

void Surface::UpdateCapabilitiesCache(VkPhysicalDevice phys_dev, const VkSurfaceCapabilitiesKHR &surface_caps) {
    auto guard = Lock();
    PhysDevCache &cache = cache_[phys_dev];
    cache.capabilities = surface_caps;
    cache.last_capability_query_used_present_mode = false;
}

void Surface::UpdateCapabilitiesCache(VkPhysicalDevice phys_dev, const VkSurfaceCapabilities2KHR &surface_caps,
                                      VkPresentModeKHR present_mode) {
    auto guard = Lock();
    auto &cache = cache_[phys_dev];

    // Get entry for the given presentation mode
    PresentModeInfo *info = nullptr;
    for (auto &cur_info : cache.present_mode_infos) {
        if (cur_info.present_mode == present_mode) {
            info = &cur_info;
            break;
        }
    }
    if (!info) {
        cache.present_mode_infos.emplace_back(PresentModeInfo{});
        info = &cache.present_mode_infos.back();
        info->present_mode = present_mode;
    }

    // Update entry
    info->surface_capabilities = surface_caps.surfaceCapabilities;
    const auto *present_scaling_caps = vku::FindStructInPNextChain<VkSurfacePresentScalingCapabilitiesEXT>(surface_caps.pNext);
    if (present_scaling_caps) {
        info->scaling_capabilities = *present_scaling_caps;
    }
    const auto *compat_modes = vku::FindStructInPNextChain<VkSurfacePresentModeCompatibilityEXT>(surface_caps.pNext);
    if (compat_modes && compat_modes->pPresentModes) {
        info->compatible_present_modes.emplace(compat_modes->pPresentModes,
                                               compat_modes->pPresentModes + compat_modes->presentModeCount);
    }
    cache.last_capability_query_used_present_mode = true;
}

bool Surface::IsLastCapabilityQueryUsedPresentMode(VkPhysicalDevice phys_dev) const {
    if (auto guard = Lock(); auto cache = GetPhysDevCache(phys_dev)) {
        return cache->last_capability_query_used_present_mode;
    }
    return false;
}

VkSurfaceCapabilitiesKHR Surface::GetSurfaceCapabilities(VkPhysicalDevice phys_dev, const void *surface_info_pnext) const {
    if (!surface_info_pnext) {
        if (auto guard = Lock(); auto cache = GetPhysDevCache(phys_dev)) {
            if (cache->capabilities.has_value()) {
                return cache->capabilities.value();
            }
        }
        VkSurfaceCapabilitiesKHR surface_caps{};
        DispatchGetPhysicalDeviceSurfaceCapabilitiesKHR(phys_dev, VkHandle(), &surface_caps);
        return surface_caps;
    }

    // Per present mode caching is supported for a common case when pNext chain is a single VkSurfacePresentModeEXT structure.
    const auto *surface_present_mode = vku::FindStructInPNextChain<VkSurfacePresentModeEXT>(surface_info_pnext);
    const bool single_pnext_element = static_cast<const VkBaseInStructure *>(surface_info_pnext)->pNext == nullptr;
    if (surface_present_mode && single_pnext_element) {
        if (auto guard = Lock(); auto cache = GetPhysDevCache(phys_dev)) {
            const PresentModeInfo *info = cache->GetPresentModeInfo(surface_present_mode->presentMode);
            if (info) {
                return info->surface_capabilities;
            }
        }
    }
    VkPhysicalDeviceSurfaceInfo2KHR surface_info = vku::InitStructHelper();
    surface_info.pNext = surface_info_pnext;
    surface_info.surface = VkHandle();
    VkSurfaceCapabilities2KHR surface_caps = vku::InitStructHelper();
    DispatchGetPhysicalDeviceSurfaceCapabilities2KHR(phys_dev, &surface_info, &surface_caps);
    return surface_caps.surfaceCapabilities;
}

VkSurfaceCapabilitiesKHR Surface::GetPresentModeSurfaceCapabilities(VkPhysicalDevice phys_dev,
                                                                    VkPresentModeKHR present_mode) const {
    VkSurfacePresentModeEXT surface_present_mode = vku::InitStructHelper();
    surface_present_mode.presentMode = present_mode;
    return GetSurfaceCapabilities(phys_dev, &surface_present_mode);
}

VkSurfacePresentScalingCapabilitiesEXT Surface::GetPresentModeScalingCapabilities(VkPhysicalDevice phys_dev,
                                                                                  VkPresentModeKHR present_mode) const {
    if (auto guard = Lock(); auto cache = GetPhysDevCache(phys_dev)) {
        const PresentModeInfo *info = cache->GetPresentModeInfo(present_mode);
        if (info && info->scaling_capabilities.has_value()) {
            return info->scaling_capabilities.value();
        }
    }
    VkSurfacePresentModeEXT surface_present_mode = vku::InitStructHelper();
    surface_present_mode.presentMode = present_mode;
    VkPhysicalDeviceSurfaceInfo2KHR surface_info = vku::InitStructHelper(&surface_present_mode);
    surface_info.surface = VkHandle();
    VkSurfacePresentScalingCapabilitiesEXT scaling_caps = vku::InitStructHelper();
    VkSurfaceCapabilities2KHR surface_caps = vku::InitStructHelper(&scaling_caps);
    DispatchGetPhysicalDeviceSurfaceCapabilities2KHR(phys_dev, &surface_info, &surface_caps);
    return scaling_caps;
}

std::vector<VkPresentModeKHR> Surface::GetCompatibleModes(VkPhysicalDevice phys_dev, VkPresentModeKHR present_mode) const {
    if (auto guard = Lock(); auto cache = GetPhysDevCache(phys_dev)) {
        const PresentModeInfo *info = cache->GetPresentModeInfo(present_mode);
        if (info && info->compatible_present_modes.has_value()) {
            return info->compatible_present_modes.value();
        }
    }
    VkSurfacePresentModeEXT surface_present_mode = vku::InitStructHelper();
    surface_present_mode.presentMode = present_mode;
    VkPhysicalDeviceSurfaceInfo2KHR surface_info = vku::InitStructHelper(&surface_present_mode);
    surface_info.surface = VkHandle();
    VkSurfacePresentModeCompatibilityEXT present_mode_compat = vku::InitStructHelper();
    VkSurfaceCapabilities2KHR surface_caps = vku::InitStructHelper(&present_mode_compat);
    DispatchGetPhysicalDeviceSurfaceCapabilities2KHR(phys_dev, &surface_info, &surface_caps);
    std::vector<VkPresentModeKHR> present_modes(present_mode_compat.presentModeCount);
    present_mode_compat.pPresentModes = present_modes.data();
    DispatchGetPhysicalDeviceSurfaceCapabilities2KHR(phys_dev, &surface_info, &surface_caps);
    return present_modes;
}

}  // namespace vvl
