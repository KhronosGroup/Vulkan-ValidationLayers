/* Copyright (c) 2015-2022 The Khronos Group Inc.
 * Copyright (c) 2015-2022 Valve Corporation
 * Copyright (c) 2015-2022 LunarG, Inc.
 * Copyright (C) 2015-2022 Google Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
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
 *
 * Author: Courtney Goeltzenleuchter <courtneygo@google.com>
 * Author: Tobin Ehlis <tobine@google.com>
 * Author: Chris Forbes <chrisf@ijw.co.nz>
 * Author: Mark Lobodzinski <mark@lunarg.com>
 * Author: Dave Houlton <daveh@lunarg.com>
 * Author: Tobias Hector <tobias.hector@amd.com>
 */

#pragma once
#include "chassis.h"
#include "core_validation_error_enums.h"
#include "device_state.h"
#include "queue_state.h"
#include "query_state.h"
#include "ray_tracing_state.h"
#include "command_validation.h"
#include "layer_chassis_dispatch.h"
#include "vk_layer_logging.h"
#include "vulkan/vk_layer.h"
#include "vk_typemap_helper.h"
#include "vk_layer_data.h"
#include "android_ndk_types.h"
#include "range_vector.h"
#include <atomic>
#include <functional>
#include <memory>
#include <vector>

namespace cvdescriptorset {
class DescriptorSet;
class DescriptorSetLayout;
struct AllocateDescriptorSetsData;
}  // namespace cvdescriptorset

class CMD_BUFFER_STATE;
class DESCRIPTOR_POOL_STATE;
class FRAMEBUFFER_STATE;
class PIPELINE_STATE;
struct PipelineStageState;
class PIPELINE_LAYOUT_STATE;
class QUEUE_STATE;
class BUFFER_STATE;
class BUFFER_VIEW_STATE;
class IMAGE_STATE;
class IMAGE_VIEW_STATE;
class COMMAND_POOL_STATE;
class DISPLAY_MODE_STATE;
class RENDER_PASS_STATE;
class SAMPLER_STATE;
class SAMPLER_YCBCR_CONVERSION_STATE;
class EVENT_STATE;
class SWAPCHAIN_NODE;
class SURFACE_STATE;
class UPDATE_TEMPLATE_STATE;

// These versions allow functions that are the same to share the same logic but can use different VUs
// The common case are functions that were missing the pNext in Vulkan 1.0 and added via extension
//
// Added from VK_KHR_create_renderpass2
enum RenderPassCreateVersion { RENDER_PASS_VERSION_1 = 0, RENDER_PASS_VERSION_2 = 1 };
// Added from VK_KHR_device_group but added to VK_KHR_swapchain with Vulkan 1.1
enum AcquireVersion { ACQUIRE_VERSION_1 = 0, ACQUIRE_VERSION_2 = 1 };

// This structure is used to save data across the CreateGraphicsPipelines down-chain API call
struct create_graphics_pipeline_api_state {
    std::vector<safe_VkGraphicsPipelineCreateInfo> gpu_create_infos;
    std::vector<safe_VkGraphicsPipelineCreateInfo> printf_create_infos;
    std::vector<std::shared_ptr<PIPELINE_STATE>> pipe_state;
    const VkGraphicsPipelineCreateInfo* pCreateInfos;
};

// This structure is used to save data across the CreateComputePipelines down-chain API call
struct create_compute_pipeline_api_state {
    std::vector<safe_VkComputePipelineCreateInfo> gpu_create_infos;
    std::vector<safe_VkComputePipelineCreateInfo> printf_create_infos;
    std::vector<std::shared_ptr<PIPELINE_STATE>> pipe_state;
    const VkComputePipelineCreateInfo* pCreateInfos;
};

// This structure is used to save data across the CreateRayTracingPipelinesNV down-chain API call.
struct create_ray_tracing_pipeline_api_state {
    std::vector<safe_VkRayTracingPipelineCreateInfoCommon> gpu_create_infos;
    std::vector<safe_VkRayTracingPipelineCreateInfoCommon> printf_create_infos;
    std::vector<std::shared_ptr<PIPELINE_STATE>> pipe_state;
    const VkRayTracingPipelineCreateInfoNV* pCreateInfos;
};

// This structure is used to save data across the CreateRayTracingPipelinesKHR down-chain API call.
struct create_ray_tracing_pipeline_khr_api_state {
    std::vector<safe_VkRayTracingPipelineCreateInfoCommon> gpu_create_infos;
    std::vector<safe_VkRayTracingPipelineCreateInfoCommon> printf_create_infos;
    std::vector<std::shared_ptr<PIPELINE_STATE>> pipe_state;
    const VkRayTracingPipelineCreateInfoKHR* pCreateInfos;
};

// This structure is used modify parameters for the CreatePipelineLayout down-chain API call
struct create_pipeline_layout_api_state {
    std::vector<VkDescriptorSetLayout> new_layouts;
    VkPipelineLayoutCreateInfo modified_create_info;
};

// This structure is used modify parameters for the CreateBuffer down-chain API call
struct create_buffer_api_state {
    VkBufferCreateInfo modified_create_info;
};

// This structure is used modify and pass parameters for the CreateShaderModule down-chain API call
struct create_shader_module_api_state {
    uint32_t unique_shader_id;
    VkShaderModuleCreateInfo instrumented_create_info;
    std::vector<uint32_t> instrumented_pgm;
};

#define VALSTATETRACK_MAP_AND_TRAITS_IMPL(handle_type, state_type, map_member, instance_scope)        \
    template <typename Dummy>                                                                         \
    struct AccessorStateHandle<state_type, Dummy> {                                                   \
        using StateType = state_type;                                                                 \
        using HandleType = handle_type;                                                               \
    };                                                                                                \
    AccessorTraitsTypes<state_type>::MapType map_member;                                              \
    template <typename Dummy>                                                                         \
    struct AccessorTraits<state_type, Dummy> : AccessorTraitsTypes<state_type> {                      \
        static const bool kInstanceScope = instance_scope;                                            \
        static MapType ValidationStateTracker::*Map() { return &ValidationStateTracker::map_member; } \
    };

#define VALSTATETRACK_MAP_AND_TRAITS(handle_type, state_type, map_member) \
    VALSTATETRACK_MAP_AND_TRAITS_IMPL(handle_type, state_type, map_member, false)
#define VALSTATETRACK_MAP_AND_TRAITS_INSTANCE_SCOPE(handle_type, state_type, map_member) \
    VALSTATETRACK_MAP_AND_TRAITS_IMPL(handle_type, state_type, map_member, true)

// For image copies between compressed/uncompressed formats, the extent is provided in source image texels
// Destination image texel extents must be adjusted by block size for the dest validation checks
static inline VkExtent3D GetAdjustedDestImageExtent(VkFormat src_format, VkFormat dst_format, VkExtent3D extent) {
    VkExtent3D adjusted_extent = extent;
    if (FormatIsBlockedImage(src_format) && !FormatIsBlockedImage(dst_format)) {
        VkExtent3D block_size = FormatTexelBlockExtent(src_format);
        adjusted_extent.width /= block_size.width;
        adjusted_extent.height /= block_size.height;
        adjusted_extent.depth /= block_size.depth;
    } else if (!FormatIsBlockedImage(src_format) && FormatIsBlockedImage(dst_format)) {
        VkExtent3D block_size = FormatTexelBlockExtent(dst_format);
        adjusted_extent.width *= block_size.width;
        adjusted_extent.height *= block_size.height;
        adjusted_extent.depth *= block_size.depth;
    }
    return adjusted_extent;
}

// Test if the extent argument has any dimensions set to 0.
static inline bool IsExtentSizeZero(const VkExtent3D* extent) {
    return ((extent->width == 0) || (extent->height == 0) || (extent->depth == 0));
}

// Get buffer size from vkBufferImageCopy / vkBufferImageCopy2KHR structure, for a given format
template <typename BufferImageCopyRegionType>
static inline VkDeviceSize GetBufferSizeFromCopyImage(const BufferImageCopyRegionType& region, VkFormat image_format) {
    VkDeviceSize buffer_size = 0;
    VkExtent3D copy_extent = region.imageExtent;
    VkDeviceSize buffer_width = (0 == region.bufferRowLength ? copy_extent.width : region.bufferRowLength);
    VkDeviceSize buffer_height = (0 == region.bufferImageHeight ? copy_extent.height : region.bufferImageHeight);

    VkDeviceSize unit_size = 0;
    if (region.imageSubresource.aspectMask & (VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_DEPTH_BIT)) {
        // Spec in vkBufferImageCopy section list special cases for each format
        if (region.imageSubresource.aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT) {
            unit_size = 1;
        } else {
            // VK_IMAGE_ASPECT_DEPTH_BIT
            switch (image_format) {
                case VK_FORMAT_D16_UNORM:
                case VK_FORMAT_D16_UNORM_S8_UINT:
                    unit_size = 2;
                    break;
                case VK_FORMAT_D32_SFLOAT:
                case VK_FORMAT_D32_SFLOAT_S8_UINT:
                // packed with the D24 value in the LSBs of the word, and undefined values in the eight MSBs
                case VK_FORMAT_X8_D24_UNORM_PACK32:
                case VK_FORMAT_D24_UNORM_S8_UINT:
                    unit_size = 4;
                    break;
                default:
                    // Any misuse of formats vs aspect mask should be caught before here
                    return 0;
            }
        }
    } else {
        // size (bytes) of texel or block
        unit_size = FormatElementSize(image_format, region.imageSubresource.aspectMask);
    }

    if (FormatIsBlockedImage(image_format)) {
        // Switch to texel block units, rounding up for any partially-used blocks
        auto block_dim = FormatTexelBlockExtent(image_format);
        buffer_width = (buffer_width + block_dim.width - 1) / block_dim.width;
        buffer_height = (buffer_height + block_dim.height - 1) / block_dim.height;

        copy_extent.width = (copy_extent.width + block_dim.width - 1) / block_dim.width;
        copy_extent.height = (copy_extent.height + block_dim.height - 1) / block_dim.height;
        copy_extent.depth = (copy_extent.depth + block_dim.depth - 1) / block_dim.depth;
    }

    // Either depth or layerCount may be greater than 1 (not both). This is the number of 'slices' to copy
    uint32_t z_copies = std::max(copy_extent.depth, region.imageSubresource.layerCount);
    if (IsExtentSizeZero(&copy_extent) || (0 == z_copies)) {
        // TODO: Issue warning here? Already warned in ValidateImageBounds()...
    } else {
        // Calculate buffer offset of final copied byte, + 1.
        buffer_size = (z_copies - 1) * buffer_height * buffer_width;                   // offset to slice
        buffer_size += ((copy_extent.height - 1) * buffer_width) + copy_extent.width;  // add row,col
        buffer_size *= unit_size;                                                      // convert to bytes
    }
    return buffer_size;
}

enum PushConstantByteState {
    PC_Byte_Updated = 0,
    PC_Byte_Not_Set = 1,
    PC_Byte_Not_Updated = 2,
};

struct SHADER_MODULE_STATE;

class ValidationStateTracker : public ValidationObject {
  private:
    // Traits for State function resolution.  Specializations defined in the macro.
    // NOTE: The Dummy argument allows for *partial* specialization at class scope, as full specialization at class scope
    //       isn't supported until C++17.  Since the Dummy has a default all instantiations of the template can ignore it, but all
    //       specializations of the template must list it (and not give it a default).
    // These must be declared at the same access level as the map declarations (below).
    template <typename StateType, typename Dummy = int>
    struct AccessorStateHandle {};
    template <typename StateType, typename Dummy = int>
    struct AccessorTraits {};
    template <typename StateType_>
    struct AccessorTraitsTypes {
        using StateType = StateType_;
        using HandleType = typename AccessorStateHandle<StateType>::HandleType;
        using SharedType = std::shared_ptr<StateType>;
        using ConstSharedType = std::shared_ptr<const StateType>;
        using ReadLockedType = LockedSharedPtr<const StateType, ReadLockGuard>;
        using WriteLockedType = LockedSharedPtr<StateType, WriteLockGuard>;
        using MappedType = std::shared_ptr<StateType>;
        using MapType = vl_concurrent_unordered_map<HandleType, MappedType>;
    };

    template <typename State, typename Traits = AccessorTraits<State>>
    typename Traits::MapType& GetStateMap() {
        auto map_member = Traits::Map();
        return (Traits::kInstanceScope && (this->*map_member).size() == 0) ? instance_state->*map_member : this->*map_member;
    }
    template <typename State, typename Traits = AccessorTraits<State>>
    const typename Traits::MapType& GetStateMap() const {
        auto map_member = Traits::Map();
        return (Traits::kInstanceScope && (this->*map_member).size() == 0) ? instance_state->*map_member : this->*map_member;
    }

  public:
    // Override base class, we have some extra work to do here
    void InitDeviceValidationObject(bool add_obj, ValidationObject* inst_obj, ValidationObject* dev_obj) override;

    template <typename State>
    void Add(std::shared_ptr<State>&& state_object) {
        auto& map = GetStateMap<State>();
        auto handle = state_object->Handle().template Cast<typename AccessorTraits<State>::HandleType>();
        // Finish setting up the object node tree, which cannot be done from the state object contructors
        // due to use of shared_from_this()
        state_object->LinkChildNodes();
        map.insert_or_assign(handle, std::move(state_object));
    }

    template <typename State>
    void Destroy(typename AccessorTraits<State>::HandleType handle) {
        auto& map = GetStateMap<State>();
        auto iter = map.pop(handle);
        if (iter != map.end()) {
            iter->second->Destroy();
        }
    }

    template <typename State>
    size_t Count() const {
        return GetStateMap<State>().size();
    }

    template <typename State>
    void ForEach(std::function<void(const State& s)> fn) const {
        const auto& map = GetStateMap<State>();
        for (const auto& entry : map.snapshot()) {
            fn(*entry.second);
        }
    }

    template <typename State>
    bool AnyOf(std::function<bool(const State& s)> fn) const {
        const auto& map = GetStateMap<State>();
        for (const auto& entry : map.snapshot()) {
            if (fn(*entry.second)) {
                return true;
            }
        }
        return false;
    }

    template <typename State>
    typename AccessorTraits<State>::SharedType Get(typename AccessorTraits<State>::HandleType handle) {
        const auto& map = GetStateMap<State>();
        const auto found_it = map.find(handle);
        if (found_it == map.end()) {
            return nullptr;
        }
        // NOTE: vl_concurrent_unordered_map::find() makes a copy of the value, so it is safe to move out.
        // But this will break everything, when switching to a different map type.
        return std::move(found_it->second);
    };

    template <typename State>
    typename AccessorTraits<State>::ConstSharedType Get(typename AccessorTraits<State>::HandleType handle) const {
        const auto& map = GetStateMap<State>();
        const auto found_it = map.find(handle);
        if (found_it == map.end()) {
            return nullptr;
        }
        return std::move(found_it->second);
    };

    // GetRead() and GetWrite() return an already locked state object. Currently this is only supported by
    // CMD_BUFFER_STATE, because it has public ReadLock() and WriteLock() methods.
    // NOTE: Calling base class hook methods with a CMD_BUFFER_STATE lock held will lead to deadlock. Instead,
    // call the base class hook method before getting/locking the command buffer state for processing in the
    // derived class method.
    template <typename State>
    typename AccessorTraits<State>::ReadLockedType GetRead(typename AccessorTraits<State>::HandleType handle) const {
        using LockedPtrType = typename AccessorTraits<State>::ReadLockedType;
        auto ptr = Get<State>(handle);
        if (ptr) {
            auto guard = ptr->ReadLock();
            return LockedPtrType(std::move(ptr), std::move(guard));
        } else {
            return LockedPtrType();
        }
    };

    template <typename State>
    typename AccessorTraits<State>::WriteLockedType GetWrite(typename AccessorTraits<State>::HandleType handle) {
        using LockedPtrType = typename AccessorTraits<State>::WriteLockedType;
        auto ptr = Get<State>(handle);
        if (ptr) {
            auto guard = ptr->WriteLock();
            return LockedPtrType(std::move(ptr), std::move(guard));
        } else {
            return LockedPtrType();
        }
    };

    // When needing to share ownership, control over constness of access with another object (i.e. adding references while
    // not modifying the contents of the ValidationStateTracker)
    template <typename State>
    typename AccessorTraits<State>::SharedType GetConstCastShared(typename AccessorTraits<State>::HandleType handle) const {
        const auto& map = GetStateMap<State>();
        const auto found_it = map.find(handle);
        if (found_it == map.end()) {
            return nullptr;
        }
        return std::move(found_it->second);
    };

    std::shared_ptr<BUFFER_STATE> GetBufferByAddress(VkDeviceAddress address) {
        ReadLockGuard guard(buffer_address_lock_);
        auto found_it = buffer_address_map_.find(address);
        if (found_it == buffer_address_map_.end()) {
            return nullptr;
        }
        // NOTE: for the address map found_it is the actual map entry rather than a copy so we cannot std::move
        return found_it->second;
    }

    std::shared_ptr<const BUFFER_STATE> GetBufferByAddress(VkDeviceAddress address) const {
        ReadLockGuard guard(buffer_address_lock_);
        auto found_it = buffer_address_map_.find(address);
        if (found_it == buffer_address_map_.end()) {
            return nullptr;
        }
        // NOTE: for the address map found_it is the actual map entry rather than a copy so we cannot std::move
        return found_it->second;
    }

    using CommandBufferResetCallback = std::function<void(VkCommandBuffer)>;
    template <typename Fn>
    void SetCommandBufferResetCallback(Fn&& fn) {
        command_buffer_reset_callback.reset(new CommandBufferResetCallback(std::forward<Fn>(fn)));
    }

    using CommandBufferFreeCallback = std::function<void(VkCommandBuffer)>;
    template <typename Fn>
    void SetCommandBufferFreeCallback(Fn&& fn) {
        command_buffer_free_callback.reset(new CommandBufferFreeCallback(std::forward<Fn>(fn)));
    }

    using SetImageViewInitialLayoutCallback = std::function<void(CMD_BUFFER_STATE*, const IMAGE_VIEW_STATE&, VkImageLayout)>;
    template <typename Fn>
    void SetSetImageViewInitialLayoutCallback(Fn&& fn) {
        set_image_view_initial_layout_callback.reset(new SetImageViewInitialLayoutCallback(std::forward<Fn>(fn)));
    }

    void CallSetImageViewInitialLayoutCallback(CMD_BUFFER_STATE* cb_node, const IMAGE_VIEW_STATE& iv_state, VkImageLayout layout) {
        if (set_image_view_initial_layout_callback) {
            (*set_image_view_initial_layout_callback)(cb_node, iv_state, layout);
        }
    }

    // State update functions
    // Gets/Enumerations
    virtual std::shared_ptr<PHYSICAL_DEVICE_STATE> CreatePhysicalDeviceState(VkPhysicalDevice phys_dev);
    void PostCallRecordCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                      VkInstance* pInstance, VkResult result) override;
    void PostCallRecordEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(
        VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, uint32_t* pCounterCount, VkPerformanceCounterKHR* pCounters,
        VkPerformanceCounterDescriptionKHR* pCounterDescriptions, VkResult result) override;
    void PostCallRecordGetAccelerationStructureMemoryRequirementsNV(VkDevice device,
                                                                    const VkAccelerationStructureMemoryRequirementsInfoNV* pInfo,
                                                                    VkMemoryRequirements2* pMemoryRequirements) override;
    void PostCallRecordGetBufferMemoryRequirements(VkDevice device, VkBuffer buffer,
                                                   VkMemoryRequirements* pMemoryRequirements) override;
    void PostCallRecordGetBufferMemoryRequirements2(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo,
                                                    VkMemoryRequirements2* pMemoryRequirements) override;
    void PostCallRecordGetBufferMemoryRequirements2KHR(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo,
                                                       VkMemoryRequirements2* pMemoryRequirements) override;
    void PostCallRecordGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue) override;
    void PostCallRecordGetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2* pQueueInfo, VkQueue* pQueue) override;
    void PostCallRecordGetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR* pGetFdInfo, int* pFd, VkResult result) override;
    void PostCallRecordGetFenceStatus(VkDevice device, VkFence fence, VkResult result) override;
#ifdef VK_USE_PLATFORM_WIN32_KHR
    void PostCallRecordGetFenceWin32HandleKHR(VkDevice device, const VkFenceGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                              HANDLE* pHandle, VkResult result) override;
#endif  // VK_USE_PLATFORM_WIN32_KHR
    void PostCallRecordGetImageMemoryRequirements(VkDevice device, VkImage image,
                                                  VkMemoryRequirements* pMemoryRequirements) override;
    void PostCallRecordGetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
                                                   VkMemoryRequirements2* pMemoryRequirements) override;
    void PostCallRecordGetImageMemoryRequirements2KHR(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
                                                      VkMemoryRequirements2* pMemoryRequirements) override;
    void PostCallRecordGetImageSparseMemoryRequirements(VkDevice device, VkImage image, uint32_t* pSparseMemoryRequirementCount,
                                                        VkSparseImageMemoryRequirements* pSparseMemoryRequirements) override;
    void PostCallRecordGetImageSparseMemoryRequirements2(VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo,
                                                         uint32_t* pSparseMemoryRequirementCount,
                                                         VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) override;
    void PostCallRecordGetImageSparseMemoryRequirements2KHR(VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo,
                                                            uint32_t* pSparseMemoryRequirementCount,
                                                            VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) override;
    void PostCallRecordGetPhysicalDeviceDisplayPlanePropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                                  VkDisplayPlanePropertiesKHR* pProperties,
                                                                  VkResult result) override;
    void PostCallRecordGetPhysicalDeviceDisplayPlaneProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                                   VkDisplayPlaneProperties2KHR* pProperties,
                                                                   VkResult result) override;
    void PostCallRecordGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount,
                                                              VkQueueFamilyProperties* pQueueFamilyProperties) override;
    void PostCallRecordGetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount,
                                                               VkQueueFamilyProperties2* pQueueFamilyProperties) override;
    void PostCallRecordGetPhysicalDeviceQueueFamilyProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                  uint32_t* pQueueFamilyPropertyCount,
                                                                  VkQueueFamilyProperties2* pQueueFamilyProperties) override;
    void PostCallRecordGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                               VkSurfaceCapabilitiesKHR* pSurfaceCapabilities,
                                                               VkResult result) override;
    void PostCallRecordGetPhysicalDeviceSurfaceCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                                const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                                VkSurfaceCapabilities2KHR* pSurfaceCapabilities,
                                                                VkResult result) override;
    void PostCallRecordGetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                VkSurfaceCapabilities2EXT* pSurfaceCapabilities,
                                                                VkResult result) override;
    void PostCallRecordGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                          uint32_t* pSurfaceFormatCount, VkSurfaceFormatKHR* pSurfaceFormats,
                                                          VkResult result) override;
    void PostCallRecordGetPhysicalDeviceSurfaceFormats2KHR(VkPhysicalDevice physicalDevice,
                                                           const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                           uint32_t* pSurfaceFormatCount, VkSurfaceFormat2KHR* pSurfaceFormats,
                                                           VkResult result) override;
    void PostCallRecordGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                               uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes,
                                                               VkResult result) override;
    void PostCallRecordGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                          VkSurfaceKHR surface, VkBool32* pSupported, VkResult result) override;
    void PostCallRecordGetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR* pGetFdInfo, int* pFd,
                                         VkResult result) override;
#ifdef VK_USE_PLATFORM_WIN32_KHR
    void PostCallRecordGetSemaphoreWin32HandleKHR(VkDevice device, const VkSemaphoreGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                                  HANDLE* pHandle, VkResult result) override;
#endif  // VK_USE_PLATFORM_WIN32_KHR
    void PostCallRecordGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount,
                                             VkImage* pSwapchainImages, VkResult result) override;
    void PostCallRecordImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR* pImportFenceFdInfo,
                                        VkResult result) override;
#ifdef VK_USE_PLATFORM_WIN32_KHR
    void PostCallRecordImportFenceWin32HandleKHR(VkDevice device,
                                                 const VkImportFenceWin32HandleInfoKHR* pImportFenceWin32HandleInfo,
                                                 VkResult result) override;
#endif  // VK_USE_PLATFORM_WIN32_KHR
    void PostCallRecordImportSemaphoreFdKHR(VkDevice device, const VkImportSemaphoreFdInfoKHR* pImportSemaphoreFdInfo,
                                            VkResult result) override;
#ifdef VK_USE_PLATFORM_WIN32_KHR
    void PostCallRecordImportSemaphoreWin32HandleKHR(VkDevice device,
                                                     const VkImportSemaphoreWin32HandleInfoKHR* pImportSemaphoreWin32HandleInfo,
                                                     VkResult result) override;
#endif  // VK_USE_PLATFORM_WIN32_KHR
    void PostCallRecordSignalSemaphoreKHR(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo, VkResult result) override;

    // Create/Destroy/Bind
    void PostCallRecordBindAccelerationStructureMemoryNV(VkDevice device, uint32_t bindInfoCount,
                                                         const VkBindAccelerationStructureMemoryInfoNV* pBindInfos,
                                                         VkResult result) override;
    void PostCallRecordBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory mem, VkDeviceSize memoryOffset,
                                        VkResult result) override;
    void PostCallRecordBindBufferMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos,
                                         VkResult result) override;
    void PostCallRecordBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos,
                                            VkResult result) override;
    void PostCallRecordBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory mem, VkDeviceSize memoryOffset,
                                       VkResult result) override;
    void PostCallRecordBindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos,
                                        VkResult result) override;
    void PostCallRecordBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos,
                                           VkResult result) override;

    void PostCallRecordCreateDevice(VkPhysicalDevice gpu, const VkDeviceCreateInfo* pCreateInfo,
                                    const VkAllocationCallbacks* pAllocator, VkDevice* pDevice, VkResult result) override;
    void PreCallRecordDestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator) override;

    void PostCallRecordCreateAccelerationStructureNV(VkDevice device, const VkAccelerationStructureCreateInfoNV* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator,
                                                     VkAccelerationStructureNV* pAccelerationStructure, VkResult result) override;
    void PreCallRecordDestroyAccelerationStructureNV(VkDevice device, VkAccelerationStructureNV accelerationStructure,
                                                     const VkAllocationCallbacks* pAllocator) override;

    void PostCallRecordCreateAccelerationStructureKHR(VkDevice device, const VkAccelerationStructureCreateInfoKHR* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator,
                                                      VkAccelerationStructureKHR* pAccelerationStructure, VkResult result) override;
    void PostCallRecordBuildAccelerationStructuresKHR(VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount,
                                                      const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
                                                      const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos,
                                                      VkResult result) override;
    void RecordDeviceAccelerationStructureBuildInfo(CMD_BUFFER_STATE& cb_state,
                                                    const VkAccelerationStructureBuildGeometryInfoKHR& info);
    void PostCallRecordCmdBuildAccelerationStructuresKHR(
        VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
        const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos) override;

    void PostCallRecordCmdBuildAccelerationStructuresIndirectKHR(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                                                 const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
                                                                 const VkDeviceAddress* pIndirectDeviceAddresses,
                                                                 const uint32_t* pIndirectStrides,
                                                                 const uint32_t* const* ppMaxPrimitiveCounts) override;
    void PreCallRecordDestroyAccelerationStructureKHR(VkDevice device, VkAccelerationStructureKHR accelerationStructure,
                                                      const VkAllocationCallbacks* pAllocator) override;

    void PostCallRecordCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                    VkBuffer* pBuffer, VkResult result) override;
    void PreCallRecordDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator) override;
    void PostCallRecordCreateBufferView(VkDevice device, const VkBufferViewCreateInfo* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkBufferView* pView, VkResult result) override;
    void PreCallRecordDestroyBufferView(VkDevice device, VkBufferView bufferView, const VkAllocationCallbacks* pAllocator) override;
    void PostCallRecordCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool,
                                         VkResult result) override;
    void PreCallRecordDestroyCommandPool(VkDevice device, VkCommandPool commandPool,
                                         const VkAllocationCallbacks* pAllocator) override;
    void PostCallRecordCreateDisplayPlaneSurfaceKHR(VkInstance instance, const VkDisplaySurfaceCreateInfoKHR* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                    VkResult result) override;
    void PostCallRecordCreateEvent(VkDevice device, const VkEventCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                   VkEvent* pEvent, VkResult result) override;
    void PreCallRecordDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks* pAllocator) override;
    void PostCallRecordCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkDescriptorPool* pDescriptorPool,
                                            VkResult result) override;
    void PreCallRecordDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                            const VkAllocationCallbacks* pAllocator) override;
    void PostCallRecordCreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkDescriptorSetLayout* pSetLayout,
                                                 VkResult result) override;
    void PostCallRecordResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags, VkResult result) override;
    void PostCallRecordResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags,
                                        VkResult result) override;
    bool PreCallValidateCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                               const VkComputePipelineCreateInfo* pCreateInfos,
                                               const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                               void* pipe_state) const override;
    void PostCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                              const VkComputePipelineCreateInfo* pCreateInfos,
                                              const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, VkResult result,
                                              void* pipe_state) override;
    void PostCallRecordResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorPoolResetFlags flags,
                                           VkResult result) override;
    bool PreCallValidateAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo,
                                               VkDescriptorSet* pDescriptorSets, void* ads_state_data) const override;
    void PreCallRecordDestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout,
                                                 const VkAllocationCallbacks* pAllocator) override;
    void PostCallRecordCreateDescriptorUpdateTemplate(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator,
                                                      VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate,
                                                      VkResult result) override;
    void PostCallRecordCreateDescriptorUpdateTemplateKHR(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
                                                         const VkAllocationCallbacks* pAllocator,
                                                         VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate,
                                                         VkResult result) override;
    void PreCallRecordDestroyDescriptorUpdateTemplate(VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                      const VkAllocationCallbacks* pAllocator) override;
    void PreCallRecordDestroyDescriptorUpdateTemplateKHR(VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                         const VkAllocationCallbacks* pAllocator) override;
    void PostCallRecordCreateFence(VkDevice device, const VkFenceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                   VkFence* pFence, VkResult result) override;
    void PreCallRecordDestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks* pAllocator) override;
    void PostCallRecordResetFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkResult result) override;
    void PostCallRecordCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer,
                                         VkResult result) override;
    void PreCallRecordDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer,
                                         const VkAllocationCallbacks* pAllocator) override;
    bool PreCallValidateCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                                const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                void* cgpl_state) const override;
    void PostCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                               const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                               const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, VkResult result,
                                               void* cgpl_state) override;
    void PostCallRecordCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                   VkImage* pImage, VkResult result) override;
    void PreCallRecordDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator) override;
    void PostCallRecordCreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkImageView* pView, VkResult result) override;
    void PreCallRecordDestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks* pAllocator) override;

    void PreCallRecordDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks* pAllocator) override;
    void PostCallRecordCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout,
                                            VkResult result) override;
    void PreCallRecordDestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout,
                                            const VkAllocationCallbacks* pAllocator) override;
    void PostCallRecordCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo* pCreateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkQueryPool* pQueryPool, VkResult result) override;
    void PreCallRecordDestroyQueryPool(VkDevice device, VkQueryPool queryPool, const VkAllocationCallbacks* pAllocator) override;
    void RecordResetQueryPool(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount);
    void PostCallRecordResetQueryPoolEXT(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount) override;
    void PostCallRecordResetQueryPool(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount) override;
    bool PreCallValidateCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                    const VkRayTracingPipelineCreateInfoNV* pCreateInfos,
                                                    const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                    void* pipe_state) const override;
    void PostCallRecordCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                   const VkRayTracingPipelineCreateInfoNV* pCreateInfos,
                                                   const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, VkResult result,
                                                   void* pipe_state) override;
    bool PreCallValidateCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                     VkPipelineCache pipelineCache, uint32_t count,
                                                     const VkRayTracingPipelineCreateInfoKHR* pCreateInfos,
                                                     const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                     void* pipe_state) const override;
    void PostCallRecordCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                    VkPipelineCache pipelineCache, uint32_t count,
                                                    const VkRayTracingPipelineCreateInfoKHR* pCreateInfos,
                                                    const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                    VkResult result, void* pipe_state) override;
    void PostCallRecordCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass,
                                        VkResult result) override;
    void PostCallRecordCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass,
                                            VkResult result) override;
    void PostCallRecordCreateRenderPass2(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass,
                                         VkResult result) override;
    void PreCallRecordDestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks* pAllocator) override;
    void PostCallRecordCreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo,
                                     const VkAllocationCallbacks* pAllocator, VkSampler* pSampler, VkResult result) override;
    void PreCallRecordDestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks* pAllocator) override;
    void PostCallRecordCreateSamplerYcbcrConversion(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator,
                                                    VkSamplerYcbcrConversion* pYcbcrConversion, VkResult result) override;
    void PostCallRecordDestroySamplerYcbcrConversion(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                                     const VkAllocationCallbacks* pAllocator) override;
    void PostCallRecordCreateSamplerYcbcrConversionKHR(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                       const VkAllocationCallbacks* pAllocator,
                                                       VkSamplerYcbcrConversion* pYcbcrConversion, VkResult result) override;
    void PostCallRecordDestroySamplerYcbcrConversionKHR(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                                        const VkAllocationCallbacks* pAllocator) override;
    void PostCallRecordCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore, VkResult result) override;
    void PreCallRecordDestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks* pAllocator) override;
    void PostCallRecordCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule, VkResult result,
                                          void* csm_state) override;
    void PreCallRecordDestroyShaderModule(VkDevice device, VkShaderModule shaderModule,
                                          const VkAllocationCallbacks* pAllocator) override;
    void PreCallRecordDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface,
                                        const VkAllocationCallbacks* pAllocator) override;
    void PostCallRecordCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount,
                                                 const VkSwapchainCreateInfoKHR* pCreateInfos,
                                                 const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchains,
                                                 VkResult result) override;
    void PostCallRecordCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain,
                                          VkResult result) override;
    void PreCallRecordDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain,
                                          const VkAllocationCallbacks* pAllocator) override;
    void PostCallRecordCreateDisplayModeKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                            const VkDisplayModeCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                            VkDisplayModeKHR* pMode, VkResult result) override;

    // CommandBuffer/Queue Control
    void PreCallRecordBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo) override;
    void PostCallRecordDeviceWaitIdle(VkDevice device, VkResult result) override;
    void PostCallRecordEndCommandBuffer(VkCommandBuffer commandBuffer, VkResult result) override;
    void PostCallRecordQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo, VkFence fence,
                                       VkResult result) override;
    void PostCallRecordQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo, VkResult result) override;
    void PostCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence,
                                   VkResult result) override;
    void PostCallRecordQueueWaitIdle(VkQueue queue, VkResult result) override;
    void PreCallRecordSetEvent(VkDevice device, VkEvent event) override;
    void PostCallRecordWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll,
                                     uint64_t timeout, VkResult result) override;
    void PostCallRecordWaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout,
                                      VkResult result) override;
    void PostCallRecordWaitSemaphoresKHR(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout,
                                         VkResult result) override;
    void PostCallRecordGetSemaphoreCounterValue(VkDevice device, VkSemaphore semaphore, uint64_t* pValue, VkResult result) override;
    void PostCallRecordGetSemaphoreCounterValueKHR(VkDevice device, VkSemaphore semaphore, uint64_t* pValue,
                                                   VkResult result) override;
    void PostCallRecordAcquireProfilingLockKHR(VkDevice device, const VkAcquireProfilingLockInfoKHR* pInfo,
                                               VkResult result) override;
    void PostCallRecordReleaseProfilingLockKHR(VkDevice device) override;

    virtual std::shared_ptr<CMD_BUFFER_STATE> CreateCmdBufferState(VkCommandBuffer cb,
                                                                   const VkCommandBufferAllocateInfo* create_info,
                                                                   const COMMAND_POOL_STATE* pool);
    // Allocate/Free
    void PostCallRecordAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* pCreateInfo,
                                              VkCommandBuffer* pCommandBuffer, VkResult result) override;
    void PostCallRecordAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo,
                                              VkDescriptorSet* pDescriptorSets, VkResult result, void* ads_state) override;
    void PostCallRecordAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo,
                                      const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory, VkResult result) override;
    void PreCallRecordFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount,
                                         const VkCommandBuffer* pCommandBuffers) override;
    void PreCallRecordFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t count,
                                         const VkDescriptorSet* pDescriptorSets) override;
    void PreCallRecordFreeMemory(VkDevice device, VkDeviceMemory mem, const VkAllocationCallbacks* pAllocator) override;
    void PreCallRecordUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount,
                                           const VkWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount,
                                           const VkCopyDescriptorSet* pDescriptorCopies) override;
    void PreCallRecordUpdateDescriptorSetWithTemplate(VkDevice device, VkDescriptorSet descriptorSet,
                                                      VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                      const void* pData) override;
    void PreCallRecordUpdateDescriptorSetWithTemplateKHR(VkDevice device, VkDescriptorSet descriptorSet,
                                                         VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                         const void* pData) override;

    // Memory mapping
    void PostCallRecordMapMemory(VkDevice device, VkDeviceMemory mem, VkDeviceSize offset, VkDeviceSize size, VkFlags flags,
                                 void** ppData, VkResult result) override;
    void PreCallRecordUnmapMemory(VkDevice device, VkDeviceMemory mem) override;

    // Recorded Commands
    void PreCallRecordCmdBeginDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo) override;
    void PostCallRecordCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot, VkFlags flags) override;
    void PostCallRecordCmdBeginQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                               VkQueryControlFlags flags, uint32_t index) override;
    void PreCallRecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                         VkSubpassContents contents) override;
    void PreCallRecordCmdBeginRenderingKHR(VkCommandBuffer commandBuffer, const VkRenderingInfoKHR* pRenderingInfo) override;
    void PreCallRecordCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo) override;
    void PreCallRecordCmdEndRenderingKHR(VkCommandBuffer commandBuffer) override;
    void PreCallRecordCmdEndRendering(VkCommandBuffer commandBuffer) override;
    void PreCallRecordCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                          const VkSubpassBeginInfo* pSubpassBeginInfo) override;
    void PreCallRecordCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                             const VkSubpassBeginInfo* pSubpassBeginInfo) override;
    void PostCallRecordCmdBeginTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                                    uint32_t counterBufferCount, const VkBuffer* pCounterBuffers,
                                                    const VkDeviceSize* pCounterBufferOffsets) override;
    void PostCallRecordCmdEndTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                                  uint32_t counterBufferCount, const VkBuffer* pCounterBuffers,
                                                  const VkDeviceSize* pCounterBufferOffsets) override;
    void PostCallRecordCmdBeginConditionalRenderingEXT(
        VkCommandBuffer commandBuffer, const VkConditionalRenderingBeginInfoEXT* pConditionalRenderingBegin) override;
    void PostCallRecordCmdEndConditionalRenderingEXT(VkCommandBuffer commandBuffer) override;
    void PreCallRecordCmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                            VkPipelineLayout layout, uint32_t firstSet, uint32_t setCount,
                                            const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount,
                                            const uint32_t* pDynamicOffsets) override;
    void PreCallRecordCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                         VkIndexType indexType) override;
    void PreCallRecordCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                      VkPipeline pipeline) override;
    void PreCallRecordCmdBindShadingRateImageNV(VkCommandBuffer commandBuffer, VkImageView imageView,
                                                VkImageLayout imageLayout) override;
    void PreCallRecordCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                           const VkBuffer* pBuffers, const VkDeviceSize* pOffsets) override;
    void PreCallRecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                   VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit* pRegions,
                                   VkFilter filter) override;
    void PreCallRecordCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2KHR* pBlitImageInfo) override;
    void PreCallRecordCmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2* pBlitImageInfo) override;
    void PostCallRecordCmdBuildAccelerationStructureNV(VkCommandBuffer commandBuffer, const VkAccelerationStructureInfoNV* pInfo,
                                                       VkBuffer instanceData, VkDeviceSize instanceOffset, VkBool32 update,
                                                       VkAccelerationStructureNV dst, VkAccelerationStructureNV src,
                                                       VkBuffer scratch, VkDeviceSize scratchOffset) override;
    void PreCallRecordCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                         const VkClearColorValue* pColor, uint32_t rangeCount,
                                         const VkImageSubresourceRange* pRanges) override;
    void PreCallRecordCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount,
                                                const VkImageSubresourceRange* pRanges) override;
    void PostCallRecordCmdCopyAccelerationStructureNV(VkCommandBuffer commandBuffer, VkAccelerationStructureNV dst,
                                                      VkAccelerationStructureNV src,
                                                      VkCopyAccelerationStructureModeNV mode) override;
    void PreCallRecordCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount,
                                    const VkBufferCopy* pRegions) override;
    void PreCallRecordCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2KHR* pCopyBufferInfos) override;
    void PreCallRecordCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2* pCopyBufferInfos) override;
    void PreCallRecordCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                           VkImageLayout dstImageLayout, uint32_t regionCount,
                                           const VkBufferImageCopy* pRegions) override;
    void PreCallRecordCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer,
                                               const VkCopyBufferToImageInfo2KHR* pCopyBufferToImageInfo) override;
    void PreCallRecordCmdCopyBufferToImage2(VkCommandBuffer commandBuffer,
                                            const VkCopyBufferToImageInfo2* pCopyBufferToImageInfo) override;
    void PreCallRecordCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                   VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy* pRegions) override;
    void PreCallRecordCmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2KHR* pCopyImageInfo) override;
    void PreCallRecordCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2* pCopyImageInfo) override;
    void PreCallRecordCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                           VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions) override;
    void PreCallRecordCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer,
                                               const VkCopyImageToBufferInfo2KHR* pCopyImageToBufferInfo) override;
    void PreCallRecordCmdCopyImageToBuffer2(VkCommandBuffer commandBuffer,
                                            const VkCopyImageToBufferInfo2* pCopyImageToBufferInfo) override;
    void PostCallRecordCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                               uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride,
                                               VkQueryResultFlags flags) override;
    void PostCallRecordCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z) override;
    void PostCallRecordCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset) override;
    void PostCallRecordCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t base_x, uint32_t base_y, uint32_t base_z,
                                          uint32_t x, uint32_t y, uint32_t z) override;
    void PostCallRecordCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t base_x, uint32_t base_y, uint32_t base_z, uint32_t x,
                                       uint32_t y, uint32_t z) override;
    void PostCallRecordCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                               uint32_t firstInstance) override;
    void PostCallRecordCmdDrawMultiEXT(VkCommandBuffer commandBuffer, uint32_t drawCount, const VkMultiDrawInfoEXT* pVertexInfo,
                                       uint32_t instanceCount, uint32_t firstInstance, uint32_t stride) override;
    void PostCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                      uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) override;
    void PostCallRecordCmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                              const VkMultiDrawIndexedInfoEXT* pIndexInfo, uint32_t instanceCount,
                                              uint32_t firstInstance, uint32_t stride, const int32_t* pVertexOffset) override;
    void PostCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count,
                                              uint32_t stride) override;
    void PostCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count,
                                       uint32_t stride) override;
    void RecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                           VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                           uint32_t stride, CMD_TYPE cmd_type);
    void PreCallRecordCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                     VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                     uint32_t stride) override;
    void PreCallRecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                  VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                  uint32_t stride) override;
    void RecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer,
                                    VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride, CMD_TYPE cmd_type);
    void PreCallRecordCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                              VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                              uint32_t stride) override;
    void PreCallRecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                           VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                           uint32_t stride) override;
    void PreCallRecordCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                      VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                      uint32_t stride) override;
    void PreCallRecordCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                 uint32_t drawCount, uint32_t stride) override;
    void PreCallRecordCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask) override;
    void PostCallRecordCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer,
                                      VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer,
                                      VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride,
                                      VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset,
                                      VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer,
                                      VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride,
                                      uint32_t width, uint32_t height, uint32_t depth) override;
    void PostCallRecordCmdTraceRaysKHR(VkCommandBuffer commandBuffer,
                                       const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable,
                                       const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable,
                                       const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable,
                                       const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable, uint32_t width,
                                       uint32_t height, uint32_t depth) override;
    void PostCallRecordCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer,
                                               const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable,
                                               const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable,
                                               const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable,
                                               const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable,
                                               VkDeviceAddress indirectDeviceAddress) override;
    void PostCallRecordCmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer) override;
    void PostCallRecordCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot) override;
    void PostCallRecordCmdEndQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                             uint32_t index) override;
    void PostCallRecordCmdEndRenderPass(VkCommandBuffer commandBuffer) override;
    void PostCallRecordCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo) override;
    void PostCallRecordCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo) override;
    void PreCallRecordCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBuffersCount,
                                         const VkCommandBuffer* pCommandBuffers) override;
    void PreCallRecordCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size,
                                    uint32_t data) override;
    void PreCallRecordCmdInsertDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo) override;
    void PostCallRecordCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents) override;
    void PostCallRecordCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo* pSubpassBeginInfo,
                                          const VkSubpassEndInfo* pSubpassEndInfo) override;
    void PostCallRecordCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo* pSubpassBeginInfo,
                                       const VkSubpassEndInfo* pSubpassEndInfo) override;
    void PreCallRecordCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                              VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                              const VkWriteDescriptorSet* pDescriptorWrites) override;
    void PreCallRecordCmdPushDescriptorSetWithTemplateKHR(VkCommandBuffer commandBuffer,
                                                          VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                          VkPipelineLayout layout, uint32_t set, const void* pData) override;
    void PostCallRecordCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlags stageFlags,
                                        uint32_t offset, uint32_t size, const void* pValues) override;
    void PreCallRecordCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) override;
    void PostCallRecordCmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                         uint32_t queryCount) override;
    void PreCallRecordCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                      VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                      const VkImageResolve* pRegions) override;
    void PreCallRecordCmdResolveImage2KHR(VkCommandBuffer commandBuffer, const VkResolveImageInfo2KHR* pResolveImageInfo) override;
    void PreCallRecordCmdResolveImage2(VkCommandBuffer commandBuffer, const VkResolveImageInfo2* pResolveImageInfo) override;
    void PreCallRecordCmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4]) override;
    void PreCallRecordCmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp,
                                      float depthBiasSlopeFactor) override;
    void PreCallRecordCmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds) override;
    void PreCallRecordCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) override;
    void PreCallRecordCmdSetExclusiveScissorNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                               uint32_t exclusiveScissorCount, const VkRect2D* pExclusiveScissors) override;
    void PreCallRecordCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth) override;
    void PreCallRecordCmdSetLineStippleEXT(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor,
                                           uint16_t lineStipplePattern) override;
    void PreCallRecordCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount,
                                    const VkRect2D* pScissors) override;
    void PreCallRecordCmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                               uint32_t compareMask) override;
    void PreCallRecordCmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                             uint32_t reference) override;
    void PreCallRecordCmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                             uint32_t writeMask) override;
    void PreCallRecordCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                     const VkViewport* pViewports) override;
    void PreCallRecordCmdSetViewportShadingRatePaletteNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                         uint32_t viewportCount,
                                                         const VkShadingRatePaletteNV* pShadingRatePalettes) override;
    void PostCallRecordCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                       VkDeviceSize dataSize, const void* pData) override;
    void PreCallRecordCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                    VkPipelineStageFlags sourceStageMask, VkPipelineStageFlags dstStageMask,
                                    uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                    uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                    uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers) override;
    void PostCallRecordCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                         VkQueryPool queryPool, uint32_t slot) override;
    void PostCallRecordCmdWriteAccelerationStructuresPropertiesKHR(VkCommandBuffer commandBuffer,
                                                                   uint32_t accelerationStructureCount,
                                                                   const VkAccelerationStructureKHR* pAccelerationStructures,
                                                                   VkQueryType queryType, VkQueryPool queryPool,
                                                                   uint32_t firstQuery) override;
    void PreCallRecordCmdSetViewportWScalingNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                               const VkViewportWScalingNV* pViewportWScalings) override;
    void PreCallRecordCmdSetVertexInputEXT(VkCommandBuffer commandBuffer, uint32_t vertexBindingDescriptionCount,
                                           const VkVertexInputBindingDescription2EXT* pVertexBindingDescriptions,
                                           uint32_t vertexAttributeDescriptionCount,
                                           const VkVertexInputAttributeDescription2EXT* pVertexAttributeDescriptions) override;
    template <typename CreateInfo>
    VkFormatFeatureFlags2KHR GetExternalFormatFeaturesANDROID(const CreateInfo* create_info) const;
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    void PostCallRecordGetAndroidHardwareBufferPropertiesANDROID(VkDevice device, const struct AHardwareBuffer* buffer,
                                                                 VkAndroidHardwareBufferPropertiesANDROID* pProperties,
                                                                 VkResult result) override;
#endif  // VK_USE_PLATFORM_ANDROID_KHR

    // WSI
    void PostCallRecordAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore,
                                           VkFence fence, uint32_t* pImageIndex, VkResult result) override;
    void PostCallRecordAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo, uint32_t* pImageIndex,
                                            VkResult result) override;
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    void PostCallRecordCreateAndroidSurfaceKHR(VkInstance instance, const VkAndroidSurfaceCreateInfoKHR* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                               VkResult result) override;
#endif  // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_IOS_MVK
    void PostCallRecordCreateIOSSurfaceMVK(VkInstance instance, const VkIOSSurfaceCreateInfoMVK* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                           VkResult result) override;
#endif  // VK_USE_PLATFORM_IOS_MVK
#ifdef VK_USE_PLATFORM_MACOS_MVK
    void PostCallRecordCreateMacOSSurfaceMVK(VkInstance instance, const VkMacOSSurfaceCreateInfoMVK* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                             VkResult result) override;
#endif  // VK_USE_PLATFORM_MACOS_MVK
#ifdef VK_USE_PLATFORM_METAL_EXT
    void PostCallRecordCreateMetalSurfaceEXT(VkInstance instance, const VkMetalSurfaceCreateInfoEXT* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                             VkResult result) override;
#endif  // VK_USE_PLATFORM_METAL_EXT
#ifdef VK_USE_PLATFORM_WIN32_KHR
    void PostCallRecordCreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                             VkResult result) override;
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    void PostCallRecordCreateWaylandSurfaceKHR(VkInstance instance, const VkWaylandSurfaceCreateInfoKHR* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                               VkResult result) override;
#endif  // VK_USE_PLATFORM_WAYLAND_KHR
#ifdef VK_USE_PLATFORM_XCB_KHR
    void PostCallRecordCreateXcbSurfaceKHR(VkInstance instance, const VkXcbSurfaceCreateInfoKHR* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                           VkResult result) override;
#endif  // VK_USE_PLATFORM_XCB_KHR
#ifdef VK_USE_PLATFORM_XLIB_KHR
    void PostCallRecordCreateXlibSurfaceKHR(VkInstance instance, const VkXlibSurfaceCreateInfoKHR* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                            VkResult result) override;
#endif  // VK_USE_PLATFORM_XLIB_KHR
    void PostCallRecordCreateHeadlessSurfaceEXT(VkInstance instance, const VkHeadlessSurfaceCreateInfoEXT* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                VkResult result) override;

    // State Utilty functions
    std::vector<std::shared_ptr<const IMAGE_VIEW_STATE>> GetAttachmentViews(const VkRenderPassBeginInfo& rp_begin,
                                                                            const FRAMEBUFFER_STATE& fb_state) const;

    VkFormatFeatureFlags2KHR GetPotentialFormatFeatures(VkFormat format) const;
    void PerformUpdateDescriptorSetsWithTemplateKHR(VkDescriptorSet descriptorSet, const UPDATE_TEMPLATE_STATE* template_state,
                                                    const void* pData);
    void RecordAcquireNextImageState(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore,
                                     VkFence fence, uint32_t* pImageIndex);
    void RecordCreateSamplerYcbcrConversionState(const VkSamplerYcbcrConversionCreateInfo* create_info,
                                                 VkSamplerYcbcrConversion ycbcr_conversion);
    virtual std::shared_ptr<SWAPCHAIN_NODE> CreateSwapchainState(const VkSwapchainCreateInfoKHR* create_info,
                                                                 VkSwapchainKHR swapchain);
    void RecordCreateSwapchainState(VkResult result, const VkSwapchainCreateInfoKHR* pCreateInfo, VkSwapchainKHR* pSwapchain,
                                    std::shared_ptr<SURFACE_STATE>&& surface_state, SWAPCHAIN_NODE* old_swapchain_state);
    void RecordDestroySamplerYcbcrConversionState(VkSamplerYcbcrConversion ycbcr_conversion);
    void RecordEnumeratePhysicalDeviceGroupsState(uint32_t* pPhysicalDeviceGroupCount,
                                                  VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties);
    void RecordEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCounters(VkPhysicalDevice physicalDevice,
                                                                          uint32_t queueFamilyIndex, uint32_t* pCounterCount,
                                                                          VkPerformanceCounterKHR* pCounters);
    void RecordGetBufferMemoryRequirementsState(VkBuffer buffer);
    void RecordGetDeviceQueueState(uint32_t queue_family_index, VkDeviceQueueCreateFlags flags, VkQueue queue);
    void RecordGetExternalFenceState(VkFence fence, VkExternalFenceHandleTypeFlagBits handle_type);
    void RecordGetImageMemoryRequirementsState(VkImage image, const VkImageMemoryRequirementsInfo2* pInfo);
    void RecordImportSemaphoreState(VkSemaphore semaphore, VkExternalSemaphoreHandleTypeFlagBits handle_type,
                                    VkSemaphoreImportFlags flags);
    void RecordGetPhysicalDeviceDisplayPlanePropertiesState(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                            void* pProperties);
    void RecordGetExternalSemaphoreState(VkSemaphore semaphore, VkExternalSemaphoreHandleTypeFlagBits handle_type);
    void RecordImportFenceState(VkFence fence, VkExternalFenceHandleTypeFlagBits handle_type, VkFenceImportFlags flags);
    void RecordUpdateDescriptorSetWithTemplateState(VkDescriptorSet descriptorSet,
                                                    VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void* pData);
    void RecordCreateDescriptorUpdateTemplateState(const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
                                                   VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate);
    void RecordMappedMemory(VkDeviceMemory mem, VkDeviceSize offset, VkDeviceSize size, void** ppData);
    void RecordCmdEndRenderingRenderPassState(VkCommandBuffer commandBuffer);
    void RecordVulkanSurface(VkSurfaceKHR* pSurface);
    void RecordWaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout, VkResult result);
    void RecordGetSemaphoreCounterValue(VkDevice device, VkSemaphore semaphore, uint64_t* pValue, VkResult result);
    void UpdateBindBufferMemoryState(VkBuffer buffer, VkDeviceMemory mem, VkDeviceSize memoryOffset);
    void UpdateBindImageMemoryState(const VkBindImageMemoryInfo& bindInfo);
    void UpdateAllocateDescriptorSetsData(const VkDescriptorSetAllocateInfo*, cvdescriptorset::AllocateDescriptorSetsData*) const;

    void PostCallRecordCopyAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                    const VkCopyAccelerationStructureInfoKHR* pInfo, VkResult result) override;
    void PostCallRecordCmdCopyAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                                       const VkCopyAccelerationStructureInfoKHR* pInfo) override;
    void PostCallRecordCmdCopyAccelerationStructureToMemoryKHR(VkCommandBuffer commandBuffer,
                                                               const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo) override;
    void PostCallRecordCmdCopyMemoryToAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                                               const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo) override;
    void PreCallRecordCmdSetCullMode(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode) override;
    void PreCallRecordCmdSetCullModeEXT(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode) override;
    void PreCallRecordCmdSetFrontFace(VkCommandBuffer commandBuffer, VkFrontFace frontFace) override;
    void PreCallRecordCmdSetFrontFaceEXT(VkCommandBuffer commandBuffer, VkFrontFace frontFace) override;
    void PreCallRecordCmdSetPrimitiveTopologyEXT(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology) override;
    void PreCallRecordCmdSetPrimitiveTopology(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology) override;
    void RecordCmdSetViewportWithCount(VkCommandBuffer commandBuffer, uint32_t viewportCount, const VkViewport* pViewports,
                                       CMD_TYPE cmdType);
    void PreCallRecordCmdSetViewportWithCountEXT(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                                 const VkViewport* pViewports) override;
    void PreCallRecordCmdSetViewportWithCount(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                              const VkViewport* pViewports) override;
    void RecordCmdSetScissorWithCount(VkCommandBuffer commandBuffer, uint32_t scissorCount, const VkRect2D* pScissors,
                                      CMD_TYPE cmdType);
    void PreCallRecordCmdSetScissorWithCountEXT(VkCommandBuffer commandBuffer, uint32_t scissorCount,
                                                const VkRect2D* pScissors) override;
    void PreCallRecordCmdSetScissorWithCount(VkCommandBuffer commandBuffer, uint32_t scissorCount,
                                             const VkRect2D* pScissors) override;
    void RecordCmdBindVertexBuffers2(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                     const VkBuffer* pBuffers, const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes,
                                     const VkDeviceSize* pStrides, CMD_TYPE cmd_type);
    void PreCallRecordCmdBindVertexBuffers2EXT(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                               const VkBuffer* pBuffers, const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes,
                                               const VkDeviceSize* pStrides) override;
    void PreCallRecordCmdBindVertexBuffers2(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                            const VkBuffer* pBuffers, const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes,
                                            const VkDeviceSize* pStrides) override;
    void PreCallRecordCmdSetDepthTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable) override;
    void PreCallRecordCmdSetDepthTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable) override;
    void PreCallRecordCmdSetDepthWriteEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable) override;
    void PreCallRecordCmdSetDepthWriteEnable(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable) override;
    void PreCallRecordCmdSetDepthCompareOpEXT(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp) override;
    void PreCallRecordCmdSetDepthCompareOp(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp) override;
    void PreCallRecordCmdSetDepthBoundsTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable) override;
    void PreCallRecordCmdSetDepthBoundsTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable) override;
    void PreCallRecordCmdSetStencilTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable) override;
    void PreCallRecordCmdSetStencilTestEnable(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable) override;
    void PreCallRecordCmdSetStencilOpEXT(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp,
                                         VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp) override;
    void PreCallRecordCmdSetStencilOp(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp,
                                      VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp) override;
    void PreCallRecordCmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer, uint32_t firstDiscardRectangle,
                                                uint32_t discardRectangleCount, const VkRect2D* pDiscardRectangles) override;
    void PreCallRecordCmdSetSampleLocationsEXT(VkCommandBuffer commandBuffer,
                                               const VkSampleLocationsInfoEXT* pSampleLocationsInfo) override;
    void PreCallRecordCmdSetCoarseSampleOrderNV(VkCommandBuffer commandBuffer, VkCoarseSampleOrderTypeNV sampleOrderType,
                                                uint32_t customSampleOrderCount,
                                                const VkCoarseSampleOrderCustomNV* pCustomSampleOrders) override;

    void PreCallRecordCmdSetPatchControlPointsEXT(VkCommandBuffer commandBuffer, uint32_t patchControlPoints) override;
    void PreCallRecordCmdSetLogicOpEXT(VkCommandBuffer commandBuffer, VkLogicOp logicOp) override;
    void PreCallRecordCmdSetRasterizerDiscardEnableEXT(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable) override;
    void PreCallRecordCmdSetRasterizerDiscardEnable(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable) override;
    void PreCallRecordCmdSetDepthBiasEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable) override;
    void PreCallRecordCmdSetDepthBiasEnable(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable) override;
    void PreCallRecordCmdSetPrimitiveRestartEnableEXT(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable) override;
    void PreCallRecordCmdSetPrimitiveRestartEnable(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable) override;

    void PostCallRecordCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                          VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                          uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                          uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                          uint32_t imageMemoryBarrierCount,
                                          const VkImageMemoryBarrier* pImageMemoryBarriers) override;

    void PreCallRecordCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer, const VkDependencyInfoKHR* pDependencyInfo) override;
    void PreCallRecordCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo) override;

    void PreCallRecordCmdSetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event,
                                      const VkDependencyInfoKHR* pDependencyInfo) override;
    void PreCallRecordCmdSetEvent2(VkCommandBuffer commandBuffer, VkEvent event, const VkDependencyInfo* pDependencyInfo) override;
    void PreCallRecordCmdResetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2KHR stageMask) override;
    void PreCallRecordCmdResetEvent2(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask) override;
    void PreCallRecordCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                        const VkDependencyInfoKHR* pDependencyInfos) override;
    void PreCallRecordCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                     const VkDependencyInfo* pDependencyInfos) override;
    void PostCallRecordCmdWriteTimestamp2KHR(VkCommandBuffer commandBuffer, VkPipelineStageFlags2KHR stage, VkQueryPool queryPool,
                                             uint32_t query) override;
    void PostCallRecordCmdWriteTimestamp2(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage, VkQueryPool queryPool,
                                          uint32_t query) override;
    void RecordQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR* pSubmits, VkFence fence, VkResult result);
    void PostCallRecordQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR* pSubmits, VkFence fence,
                                       VkResult result) override;
    void PostCallRecordQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence,
                                    VkResult result) override;

    void RecordGetBufferDeviceAddress(const VkBufferDeviceAddressInfo* pInfo, VkDeviceAddress address);
    void PostCallRecordGetBufferDeviceAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                              VkDeviceAddress address) override;
    void PostCallRecordGetBufferDeviceAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                                 VkDeviceAddress address) override;
    void PostCallRecordGetBufferDeviceAddressEXT(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                                 VkDeviceAddress address) override;
    template <typename ExtProp>
    void GetPhysicalDeviceExtProperties(VkPhysicalDevice gpu, ExtEnabled enabled, ExtProp* ext_prop) {
        assert(ext_prop);
        if (IsExtEnabled(enabled)) {
            *ext_prop = LvlInitStruct<ExtProp>();
            if (api_version < VK_API_VERSION_1_1) {
                auto prop2 = LvlInitStruct<VkPhysicalDeviceProperties2>(ext_prop);
                DispatchGetPhysicalDeviceProperties2KHR(gpu, &prop2);
            } else {
                auto prop2 = LvlInitStruct<VkPhysicalDeviceProperties2>(ext_prop);
                DispatchGetPhysicalDeviceProperties2(gpu, &prop2);
            }
        }
    }

    // Link to the device's physical-device data
    PHYSICAL_DEVICE_STATE* physical_device_state;

    // Link for derived device objects back to their parent instance object
    ValidationStateTracker* instance_state;

    std::unique_ptr<CommandBufferResetCallback> command_buffer_reset_callback;
    std::unique_ptr<CommandBufferFreeCallback> command_buffer_free_callback;
    std::unique_ptr<SetImageViewInitialLayoutCallback> set_image_view_initial_layout_callback;

    DeviceFeatures enabled_features = {};
    // Device specific data
    std::set<std::string> phys_dev_extensions;
    VkPhysicalDeviceMemoryProperties phys_dev_mem_props = {};
    VkPhysicalDeviceProperties phys_dev_props = {};
    VkPhysicalDeviceVulkan11Properties phys_dev_props_core11 = {};
    VkPhysicalDeviceVulkan12Properties phys_dev_props_core12 = {};
    VkPhysicalDeviceVulkan13Properties phys_dev_props_core13 = {};
    VkDeviceGroupDeviceCreateInfo device_group_create_info = {};
    uint32_t physical_device_count;
    uint32_t custom_border_color_sampler_count = 0;

    // VK_KHR_format_feature_flags2 changes the behavior of the
    // app/layers/spec if present. So it needs its own special boolean unlike
    // the enabled_fatures.
    bool has_format_feature2;

    // Device extension properties -- storing properties gathered from VkPhysicalDeviceProperties2::pNext chain
    struct DeviceExtensionProperties {
        VkPhysicalDevicePushDescriptorPropertiesKHR push_descriptor_props;
        VkPhysicalDeviceShadingRateImagePropertiesNV shading_rate_image_props;
        VkPhysicalDeviceMeshShaderPropertiesNV mesh_shader_props;
        VkPhysicalDeviceInlineUniformBlockPropertiesEXT inline_uniform_block_props;
        VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT vtx_attrib_divisor_props;
        VkPhysicalDeviceCooperativeMatrixPropertiesNV cooperative_matrix_props;
        VkPhysicalDeviceTransformFeedbackPropertiesEXT transform_feedback_props;
        VkPhysicalDeviceRayTracingPropertiesNV ray_tracing_propsNV;
        VkPhysicalDeviceRayTracingPipelinePropertiesKHR ray_tracing_propsKHR;
        VkPhysicalDeviceAccelerationStructurePropertiesKHR acc_structure_props;
        VkPhysicalDeviceTexelBufferAlignmentPropertiesEXT texel_buffer_alignment_props;
        VkPhysicalDeviceFragmentDensityMapPropertiesEXT fragment_density_map_props;
        VkPhysicalDeviceFragmentDensityMap2PropertiesEXT fragment_density_map2_props;
        VkPhysicalDeviceFragmentDensityMapOffsetPropertiesQCOM fragment_density_map_offset_props;
        VkPhysicalDevicePerformanceQueryPropertiesKHR performance_query_props;
        VkPhysicalDeviceSampleLocationsPropertiesEXT sample_locations_props;
        VkPhysicalDeviceCustomBorderColorPropertiesEXT custom_border_color_props;
        VkPhysicalDeviceMultiviewProperties multiview_props;
        VkPhysicalDevicePortabilitySubsetPropertiesKHR portability_props;
        VkPhysicalDeviceFragmentShadingRatePropertiesKHR fragment_shading_rate_props;
        VkPhysicalDeviceProvokingVertexPropertiesEXT provoking_vertex_props;
        VkPhysicalDeviceMultiDrawPropertiesEXT multi_draw_props;
        VkPhysicalDeviceDiscardRectanglePropertiesEXT discard_rectangle_props;
        VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT blend_operation_advanced_props;
        VkPhysicalDeviceConservativeRasterizationPropertiesEXT conservative_rasterization_props;
        VkPhysicalDeviceSubgroupSizeControlPropertiesEXT subgroup_size_control_props;
    };
    DeviceExtensionProperties phys_dev_ext_props = {};
    std::vector<VkCooperativeMatrixPropertiesNV> cooperative_matrix_properties;

    bool performance_lock_acquired = false;

  protected:
    // tracks which queue family index were used when creating the device for quick lookup
    layer_data::unordered_set<uint32_t> queue_family_index_set;
    // The queue count can different for the same queueFamilyIndex if the create flag are different
    struct DeviceQueueInfo {
        uint32_t index;  // from VkDeviceCreateInfo
        uint32_t queue_family_index;
        VkDeviceQueueCreateFlags flags;
        uint32_t queue_count;
    };
    std::vector<DeviceQueueInfo> device_queue_info_list;
    // If vkGetBufferDeviceAddress is called, keep track of buffer <-> address mapping.
    sparse_container::range_map<VkDeviceAddress, std::shared_ptr<BUFFER_STATE>> buffer_address_map_;
    mutable ReadWriteLock buffer_address_lock_;

    vl_concurrent_unordered_map<uint64_t, VkFormatFeatureFlags2KHR> ahb_ext_formats_map;

  private:
    VALSTATETRACK_MAP_AND_TRAITS(VkQueue, QUEUE_STATE, queue_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkAccelerationStructureNV, ACCELERATION_STRUCTURE_STATE, acceleration_structure_nv_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkRenderPass, RENDER_PASS_STATE, render_pass_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkDescriptorSetLayout, cvdescriptorset::DescriptorSetLayout, descriptor_set_layout_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkSampler, SAMPLER_STATE, sampler_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkImageView, IMAGE_VIEW_STATE, image_view_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkImage, IMAGE_STATE, image_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkBufferView, BUFFER_VIEW_STATE, buffer_view_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkBuffer, BUFFER_STATE, buffer_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkPipeline, PIPELINE_STATE, pipeline_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkDeviceMemory, DEVICE_MEMORY_STATE, mem_obj_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkFramebuffer, FRAMEBUFFER_STATE, frame_buffer_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkShaderModule, SHADER_MODULE_STATE, shader_module_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkDescriptorUpdateTemplate, UPDATE_TEMPLATE_STATE, desc_template_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkSwapchainKHR, SWAPCHAIN_NODE, swapchain_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkDescriptorPool, DESCRIPTOR_POOL_STATE, descriptor_pool_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkDescriptorSet, cvdescriptorset::DescriptorSet, descriptor_set_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkCommandBuffer, CMD_BUFFER_STATE, command_buffer_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkCommandPool, COMMAND_POOL_STATE, command_pool_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkPipelineLayout, PIPELINE_LAYOUT_STATE, pipeline_layout_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkFence, FENCE_STATE, fence_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkQueryPool, QUERY_POOL_STATE, query_pool_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkSemaphore, SEMAPHORE_STATE, semaphore_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkEvent, EVENT_STATE, event_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkSamplerYcbcrConversion, SAMPLER_YCBCR_CONVERSION_STATE, sampler_ycbcr_conversion_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkAccelerationStructureKHR, ACCELERATION_STRUCTURE_STATE_KHR, acceleration_structure_khr_map_)
    VALSTATETRACK_MAP_AND_TRAITS_INSTANCE_SCOPE(VkSurfaceKHR, SURFACE_STATE, surface_map_)
    VALSTATETRACK_MAP_AND_TRAITS_INSTANCE_SCOPE(VkDisplayModeKHR, DISPLAY_MODE_STATE, display_mode_map_)
    VALSTATETRACK_MAP_AND_TRAITS_INSTANCE_SCOPE(VkPhysicalDevice, PHYSICAL_DEVICE_STATE, physical_device_map_);

    // Simple base address allocator allow allow VkDeviceMemory allocations to appear to exist in a common address space.
    // At 256GB allocated/sec  ( > 8GB at 30Hz), will overflow in just over 2 years
    class FakeAllocator {
      public:
        void Free(VkDeviceSize fake_address){};  // Define the interface just in case we ever need to be cleverer.
        VkDeviceSize Alloc(VkDeviceSize size) {
            const auto alloc = free_;
            assert(std::numeric_limits<VkDeviceSize>::max() - size >= free_);  //  776.722963 days later...
            free_ = free_ + size;
            return alloc;
        }

      private:
        VkDeviceSize free_ = 1U << 20; // start at 1mb to leave room for a NULL address
    };
    FakeAllocator fake_memory;
};
