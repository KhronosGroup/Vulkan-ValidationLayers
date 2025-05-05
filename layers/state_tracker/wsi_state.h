/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (C) 2015-2025 Google Inc.
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
#pragma once

#include "state_tracker/state_object.h"
#include "state_tracker/submission_reference.h"
#include "state_tracker/image_layout_map.h"
#include "containers/span.h"
#include <vulkan/utility/vk_safe_struct.hpp>

namespace vvl {
class DeviceState;
class Fence;
class Semaphore;
class Surface;
class Swapchain;
class SwapchainSubState;
}  // namespace vvl

struct GpuQueue {
    VkPhysicalDevice gpu;
    uint32_t queue_family_index;
};

inline bool operator==(GpuQueue const &lhs, GpuQueue const &rhs) {
    return (lhs.gpu == rhs.gpu && lhs.queue_family_index == rhs.queue_family_index);
}

namespace std {
template <>
struct hash<GpuQueue> {
    size_t operator()(GpuQueue gq) const throw() {
        return hash<uint64_t>()((uint64_t)(gq.gpu)) ^ hash<uint32_t>()(gq.queue_family_index);
    }
};
}  // namespace std

namespace vvl {

struct SwapchainImage {
    vvl::Image *image_state = nullptr;

    // Acquire state
    bool acquired = false;
    std::shared_ptr<vvl::Semaphore> acquire_semaphore;
    std::shared_ptr<vvl::Fence> acquire_fence;

    // Queue location (seq) for present operation that presented this image.
    // When this image is reacquired, the acquire fence can synchronize with this location.
    std::optional<SubmissionReference> present_submission_ref;

    // Wait semaphores from the presentation request
    small_vector<std::shared_ptr<vvl::Semaphore>, 1> present_wait_semaphores;
    void ResetPresentWaitSemaphores();
};

// State for VkSwapchainKHR objects.
// Parent -> child relationships in the object usage tree:
//    vvl::Swapchain [N] -> [1] vvl::Surface
//    However, only 1 swapchain for each surface can be !retired.
class Swapchain : public StateObject, public SubStateManager<SwapchainSubState> {
  public:
    const vku::safe_VkSwapchainCreateInfoKHR safe_create_info;
    const VkSwapchainCreateInfoKHR &create_info;

    std::vector<VkPresentModeKHR> present_modes;
    std::vector<SwapchainImage> images;
    bool retired = false;
    bool exclusive_full_screen_access;
    const bool shared_presentable;
    uint64_t max_present_id = 0;
    const vku::safe_VkImageCreateInfo image_create_info;

    std::shared_ptr<vvl::Surface> surface;
    DeviceState &dev_data;
    uint32_t acquired_images = 0;

    // Image acquire history
    static constexpr uint32_t acquire_history_max_length = 16;
    std::array<uint32_t, acquire_history_max_length> acquire_history;  // ring buffer contanis the last acquired images
    uint32_t acquire_count = 0;                                        // total number of image acquire requests

    Swapchain(DeviceState &dev_data, const VkSwapchainCreateInfoKHR *pCreateInfo, VkSwapchainKHR handle);

    ~Swapchain() {
        if (!Destroyed()) {
            Destroy();
        }
    }

    VkSwapchainKHR VkHandle() const { return handle_.Cast<VkSwapchainKHR>(); }

    void PresentImage(uint32_t image_index, uint64_t present_id, const SubmissionReference &present_submission_ref,
                      vvl::span<std::shared_ptr<vvl::Semaphore>> present_wait_semaphores);

    void ReleaseImage(uint32_t image_index);

    void AcquireImage(uint32_t image_index, const std::shared_ptr<vvl::Semaphore> &semaphore_state,
                      const std::shared_ptr<vvl::Fence> &fence_state);

    void Destroy() override;

    SwapchainImage GetSwapChainImage(uint32_t index) const;

    std::shared_ptr<const vvl::Image> GetSwapChainImageShared(uint32_t index) const;

    uint32_t GetAcquireHistoryLength() const;
    uint32_t GetAcquiredImageIndexFromHistory(uint32_t acquire_history_index) const;

    std::shared_ptr<const Swapchain> shared_from_this() const { return SharedFromThisImpl(this); }
    std::shared_ptr<Swapchain> shared_from_this() { return SharedFromThisImpl(this); }

  protected:
    void NotifyInvalidate(const StateObject::NodeList &invalid_nodes, bool unlink) override;
};

class SwapchainSubState {
  public:
    explicit SwapchainSubState(Swapchain &sc) : base(sc) {}
    SwapchainSubState(const SwapchainSubState &) = delete;
    SwapchainSubState &operator=(const SwapchainSubState &) = delete;
    virtual ~SwapchainSubState() {}
    virtual void Destroy() {}

    Swapchain &base;
};

// Parent -> child relationships in the object usage tree:
//    vvl::Surface -> nothing
class Surface : public StateObject {
  public:
    Surface(VkSurfaceKHR handle) : StateObject(handle, kVulkanObjectTypeSurfaceKHR) {}

    ~Surface() {
        if (!Destroyed()) {
            Destroy();
        }
    }

    VkSurfaceKHR VkHandle() const { return handle_.Cast<VkSurfaceKHR>(); }

    void Destroy() override;

    void RemoveParent(StateObject *parent_node) override;

    void SetQueueSupport(VkPhysicalDevice phys_dev, uint32_t qfi, bool supported);
    bool GetQueueSupport(VkPhysicalDevice phys_dev, uint32_t qfi) const;

    void SetPresentModes(VkPhysicalDevice phys_dev, vvl::span<const VkPresentModeKHR> modes);
    std::vector<VkPresentModeKHR> GetPresentModes(VkPhysicalDevice phys_dev) const;

    void SetFormats(VkPhysicalDevice phys_dev, std::vector<vku::safe_VkSurfaceFormat2KHR> &&fmts);
    vvl::span<const vku::safe_VkSurfaceFormat2KHR> GetFormats(bool get_surface_capabilities2, VkPhysicalDevice phys_dev,
                                                              const void *surface_info2_pnext) const;

    // Cache capabilities that do not depend on the present mode
    void UpdateCapabilitiesCache(VkPhysicalDevice phys_dev, const VkSurfaceCapabilitiesKHR &surface_caps);
    // Cache per present mode capabilities
    void UpdateCapabilitiesCache(VkPhysicalDevice phys_dev, const VkSurfaceCapabilities2KHR &surface_caps,
                                 VkPresentModeKHR present_mode);

    bool IsLastCapabilityQueryUsedPresentMode(VkPhysicalDevice phys_dev) const;
    VkSurfaceCapabilitiesKHR GetSurfaceCapabilities(VkPhysicalDevice phys_dev, const void *surface_info_pnext) const;
    VkSurfaceCapabilitiesKHR GetPresentModeSurfaceCapabilities(VkPhysicalDevice phys_dev, VkPresentModeKHR present_mode) const;
    VkSurfacePresentScalingCapabilitiesEXT GetPresentModeScalingCapabilities(VkPhysicalDevice phys_dev,
                                                                             VkPresentModeKHR present_mode) const;
    std::vector<VkPresentModeKHR> GetCompatibleModes(VkPhysicalDevice phys_dev, VkPresentModeKHR present_mode) const;

    vvl::Swapchain *swapchain{nullptr};

  private:
    // Contains per present mode capabilities
    struct PresentModeInfo {
        VkPresentModeKHR present_mode;
        VkSurfaceCapabilitiesKHR surface_capabilities;
        std::optional<VkSurfacePresentScalingCapabilitiesEXT> scaling_capabilities;
        std::optional<std::vector<VkPresentModeKHR>> compatible_present_modes;
    };
    // Cached information per physical device. Optional indicates if element is in the cache.
    //
    // NOTE: One of the reasons to cache surface caps is to prevent a false-positive
    // when the surface change happens (e.g. resize) after the surface caps are queried
    // and before the swapchain is created. The assumption is that with the current API,
    // the app can't do better than this (no atomicity between query and swapchain creation).
    // The caching ensures that validation sees the same surface state as the application.
    //
    // The priority is to avoid false-positives for correctly written application.
    // When the application behaves incorrectly (e.g. forgets to query surface caps after
    // it processed the resize event), then the caching can hide a problem, since validation
    // will think that application respects surface caps values.
    struct PhysDevCache {
        std::optional<std::vector<VkPresentModeKHR>> present_modes;
        std::optional<VkSurfaceCapabilitiesKHR> capabilities;
        std::vector<PresentModeInfo> present_mode_infos;
        bool last_capability_query_used_present_mode = false;

        const PresentModeInfo *GetPresentModeInfo(VkPresentModeKHR present_mode) const;
    };
    const PhysDevCache *GetPhysDevCache(VkPhysicalDevice phys_dev) const;

  private:
    std::unique_lock<std::mutex> Lock() const { return std::unique_lock<std::mutex>(lock_); }
    // TODO: make mutex shared, so multiple Validate can read simultaneously. Remove remaining mutations in Validate first
    mutable std::mutex lock_;
    mutable vvl::unordered_map<GpuQueue, bool> gpu_queue_support_;
    mutable vvl::unordered_map<VkPhysicalDevice, std::vector<vku::safe_VkSurfaceFormat2KHR>> formats_;

    vvl::unordered_map<VkPhysicalDevice, PhysDevCache> cache_;
};

}  // namespace vvl