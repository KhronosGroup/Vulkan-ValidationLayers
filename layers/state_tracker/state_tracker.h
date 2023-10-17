/* Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (C) 2015-2023 Google Inc.
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
#include "generated/chassis.h"
#include "error_message/validation_error_enums.h"
#include "state_tracker/device_state.h"
#include "state_tracker/queue_state.h"
#include "state_tracker/query_state.h"
#include "state_tracker/ray_tracing_state.h"
#include "state_tracker/video_session_state.h"
#include "generated/layer_chassis_dispatch.h"
#include "generated/state_tracker_helper.h"
#include "error_message/logging.h"
#include "vulkan/vk_layer.h"
#include "containers/custom_containers.h"
#include "utils/android_ndk_types.h"
#include "containers/range_vector.h"
#include <vulkan/utility/vk_struct_helper.hpp>
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
class PIPELINE_CACHE_STATE;
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
struct SHADER_MODULE_STATE;
struct SHADER_OBJECT_STATE;
struct SPIRV_MODULE_STATE;

// This structure is used modify and pass parameters for the CreateShaderModule down-chain API call
struct create_shader_module_api_state {
    // We build a SPIRV_MODULE_STATE at PreCallRecord time were we can do basic validation of the SPIR-V (which can crash drivers
    // if passed in the Dispatch). It is then passed to PostCallRecord to save in state tracking so it can be used at Pipeline
    // creation time where the rest of the information is needed to do the remaining SPIR-V validation.
    std::shared_ptr<SPIRV_MODULE_STATE> module_state;  // contains SPIR-V to validate
    uint32_t unique_shader_id = 0;
    bool valid_spirv = true;

    // Pass the instrumented SPIR-V info from PreCallRecord to Dispatch (so GPU-AV logic can run with it)
    VkShaderModuleCreateInfo instrumented_create_info;
    std::vector<uint32_t> instrumented_spirv;
};

// same idea as create_shader_module_api_state but for VkShaderEXT (VK_EXT_shader_object)
struct create_shader_object_api_state {
    std::vector<std::shared_ptr<SPIRV_MODULE_STATE>> module_states;  // contains SPIR-V to validate
    std::vector<uint32_t> unique_shader_ids;
    bool valid_spirv = true;

    // Pass the instrumented SPIR-V info from PreCallRecord to Dispatch (so GPU-AV logic can run with it)
    VkShaderCreateInfoEXT* instrumented_create_info;
    std::vector<std::vector<uint32_t>> instrumented_spirv;

    std::vector<VkDescriptorSetLayout> new_layouts;

    create_shader_object_api_state(uint32_t createInfoCount, const VkShaderCreateInfoEXT* pCreateInfos) {
        instrumented_create_info = const_cast<VkShaderCreateInfoEXT*>(pCreateInfos);
        module_states.resize(createInfoCount);
        unique_shader_ids.resize(createInfoCount);
        instrumented_spirv.resize(createInfoCount);
    }
};

// This structure is used to save data across the CreateGraphicsPipelines down-chain API call
// CreateShaderModuleStates[i] = ith shader state
using CreateShaderModuleStates = std::array<create_shader_module_api_state, 32>;
struct create_graphics_pipeline_api_state {
    std::vector<safe_VkGraphicsPipelineCreateInfo> modified_create_infos;
    std::vector<std::shared_ptr<PIPELINE_STATE>> pipe_state;
    std::vector<CreateShaderModuleStates> shader_states;
    const VkGraphicsPipelineCreateInfo* pCreateInfos;
};

// This structure is used to save data across the CreateComputePipelines down-chain API call
struct create_compute_pipeline_api_state {
    std::vector<safe_VkComputePipelineCreateInfo> modified_create_infos;
    std::vector<std::shared_ptr<PIPELINE_STATE>> pipe_state;
    std::vector<CreateShaderModuleStates> shader_states;
    const VkComputePipelineCreateInfo* pCreateInfos;
};

// This structure is used to save data across the CreateRayTracingPipelinesNV down-chain API call.
struct create_ray_tracing_pipeline_api_state {
    std::vector<safe_VkRayTracingPipelineCreateInfoCommon> modified_create_infos;
    std::vector<std::shared_ptr<PIPELINE_STATE>> pipe_state;
    std::vector<CreateShaderModuleStates> shader_states;
    const VkRayTracingPipelineCreateInfoNV* pCreateInfos;
};

// This structure is used to save data across the CreateRayTracingPipelinesKHR down-chain API call.
struct create_ray_tracing_pipeline_khr_api_state {
    std::vector<safe_VkRayTracingPipelineCreateInfoCommon> modified_create_infos;
    std::vector<std::shared_ptr<PIPELINE_STATE>> pipe_state;
    std::vector<CreateShaderModuleStates> shader_states;
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

#define VALSTATETRACK_MAP_AND_TRAITS_IMPL(handle_type, state_type, map_member, instance_scope)        \
    vl_concurrent_unordered_map<handle_type, std::shared_ptr<state_type>> map_member;                 \
    template <typename Dummy>                                                                         \
    struct MapTraits<state_type, Dummy> {                                                             \
        static constexpr bool kInstanceScope = instance_scope;                                        \
        using MapType = decltype(map_member);                                                         \
        static MapType ValidationStateTracker::*Map() { return &ValidationStateTracker::map_member; } \
    };

#define VALSTATETRACK_MAP_AND_TRAITS(handle_type, state_type, map_member) \
    VALSTATETRACK_MAP_AND_TRAITS_IMPL(handle_type, state_type, map_member, false)
#define VALSTATETRACK_MAP_AND_TRAITS_INSTANCE_SCOPE(handle_type, state_type, map_member) \
    VALSTATETRACK_MAP_AND_TRAITS_IMPL(handle_type, state_type, map_member, true)

namespace state_object {
// Traits for State function resolution.  Specializations defined in the macros below.
template <typename StateType>
struct Traits {};

// Helper object to make the macros simpler
// HandleType_ is a vulkan handle type
// StateType_ is the type of the corresponding state object, which may be a derived type
// BaseType_ is the type of object stored in the ValidationStateTracker, there
//            *must* be a corresponding map using this type
template <typename HandleType_, typename StateType_, typename BaseType_ = StateType_>
struct TraitsBase {
    using StateType = StateType_;
    using BaseType = BaseType_;
    using HandleType = HandleType_;
    using SharedType = std::shared_ptr<StateType>;
    using ConstSharedType = std::shared_ptr<const StateType>;
    using ReadLockedType = LockedSharedPtr<const StateType, ReadLockGuard>;
    using WriteLockedType = LockedSharedPtr<StateType, WriteLockGuard>;
};
}  // namespace state_object

#define VALSTATETRACK_STATE_OBJECT(handle_type, state_type)                    \
    namespace state_object {                                                   \
    template <>                                                                \
    struct Traits<state_type> : public TraitsBase<handle_type, state_type> {}; \
    }

#define VALSTATETRACK_DERIVED_STATE_OBJECT(handle_type, state_type, base_type)            \
    namespace state_object {                                                              \
    template <>                                                                           \
    struct Traits<state_type> : public TraitsBase<handle_type, state_type, base_type> {}; \
    }

// For image copies between compressed/uncompressed formats, the extent is provided in source image texels
// Destination image texel extents must be adjusted by block size for the dest validation checks
static inline VkExtent3D GetAdjustedDestImageExtent(VkFormat src_format, VkFormat dst_format, VkExtent3D extent) {
    VkExtent3D adjusted_extent = extent;
    if (vkuFormatIsBlockedImage(src_format) && !vkuFormatIsBlockedImage(dst_format)) {
        VkExtent3D block_size = vkuFormatTexelBlockExtent(src_format);
        adjusted_extent.width /= block_size.width;
        adjusted_extent.height /= block_size.height;
        adjusted_extent.depth /= block_size.depth;
    } else if (!vkuFormatIsBlockedImage(src_format) && vkuFormatIsBlockedImage(dst_format)) {
        VkExtent3D block_size = vkuFormatTexelBlockExtent(dst_format);
        adjusted_extent.width *= block_size.width;
        adjusted_extent.height *= block_size.height;
        adjusted_extent.depth *= block_size.depth;
    }
    return adjusted_extent;
}

// Get buffer size from VkBufferImageCopy / VkBufferImageCopy2KHR structure, for a given format
template <typename RegionType>
static inline VkDeviceSize GetBufferSizeFromCopyImage(const RegionType& region, VkFormat image_format, uint32_t image_layout_count) {
    VkDeviceSize buffer_size = 0;
    VkExtent3D copy_extent = region.imageExtent;
    VkDeviceSize buffer_width = (0 == region.bufferRowLength ? copy_extent.width : region.bufferRowLength);
    VkDeviceSize buffer_height = (0 == region.bufferImageHeight ? copy_extent.height : region.bufferImageHeight);
    uint32_t layer_count = region.imageSubresource.layerCount != VK_REMAINING_ARRAY_LAYERS
                               ? region.imageSubresource.layerCount
                               : image_layout_count - region.imageSubresource.baseArrayLayer;
    // VUID-VkImageCreateInfo-imageType-00961 prevents having both depth and layerCount ever both be greater than 1 together. Take
    // max to logic simple. This is the number of 'slices' to copy.
    const uint32_t z_copies = std::max(copy_extent.depth, layer_count);

    // Invalid if copy size is 0 and other validation checks will catch it. Returns zero as the caller should have fallback already
    // to ignore.
    if (copy_extent.width == 0 || copy_extent.height == 0 || copy_extent.depth == 0 || z_copies == 0) {
        return 0;
    }

    VkDeviceSize unit_size = 0;
    if (region.imageSubresource.aspectMask & (VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_DEPTH_BIT)) {
        // Spec in VkBufferImageCopy section list special cases for each format
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
        unit_size = vkuFormatElementSizeWithAspect(image_format, static_cast<VkImageAspectFlagBits>(region.imageSubresource.aspectMask));
    }

    if (vkuFormatIsBlockedImage(image_format)) {
        // Switch to texel block units, rounding up for any partially-used blocks
        auto block_dim = vkuFormatTexelBlockExtent(image_format);
        buffer_width = (buffer_width + block_dim.width - 1) / block_dim.width;
        buffer_height = (buffer_height + block_dim.height - 1) / block_dim.height;

        copy_extent.width = (copy_extent.width + block_dim.width - 1) / block_dim.width;
        copy_extent.height = (copy_extent.height + block_dim.height - 1) / block_dim.height;
        copy_extent.depth = (copy_extent.depth + block_dim.depth - 1) / block_dim.depth;
    }

    // Calculate buffer offset of final copied byte, + 1.
    buffer_size = (z_copies - 1) * buffer_height * buffer_width;                   // offset to slice
    buffer_size += ((copy_extent.height - 1) * buffer_width) + copy_extent.width;  // add row,col
    buffer_size *= unit_size;                                                      // convert to bytes
    return buffer_size;
}

VALSTATETRACK_STATE_OBJECT(VkQueue, QUEUE_STATE)
VALSTATETRACK_STATE_OBJECT(VkAccelerationStructureNV, ACCELERATION_STRUCTURE_STATE_NV)
VALSTATETRACK_STATE_OBJECT(VkRenderPass, RENDER_PASS_STATE)
VALSTATETRACK_STATE_OBJECT(VkDescriptorSetLayout, cvdescriptorset::DescriptorSetLayout)
VALSTATETRACK_STATE_OBJECT(VkSampler, SAMPLER_STATE)
VALSTATETRACK_STATE_OBJECT(VkImageView, IMAGE_VIEW_STATE)
VALSTATETRACK_STATE_OBJECT(VkImage, IMAGE_STATE)
VALSTATETRACK_STATE_OBJECT(VkBufferView, BUFFER_VIEW_STATE)
VALSTATETRACK_STATE_OBJECT(VkBuffer, BUFFER_STATE)
VALSTATETRACK_STATE_OBJECT(VkPipelineCache, PIPELINE_CACHE_STATE)
VALSTATETRACK_STATE_OBJECT(VkPipeline, PIPELINE_STATE)
VALSTATETRACK_STATE_OBJECT(VkShaderEXT, SHADER_OBJECT_STATE)
VALSTATETRACK_STATE_OBJECT(VkDeviceMemory, DEVICE_MEMORY_STATE)
VALSTATETRACK_STATE_OBJECT(VkFramebuffer, FRAMEBUFFER_STATE)
VALSTATETRACK_STATE_OBJECT(VkShaderModule, SHADER_MODULE_STATE)
VALSTATETRACK_STATE_OBJECT(VkDescriptorUpdateTemplate, UPDATE_TEMPLATE_STATE)
VALSTATETRACK_STATE_OBJECT(VkSwapchainKHR, SWAPCHAIN_NODE)
VALSTATETRACK_STATE_OBJECT(VkDescriptorPool, DESCRIPTOR_POOL_STATE)
VALSTATETRACK_STATE_OBJECT(VkDescriptorSet, cvdescriptorset::DescriptorSet)
VALSTATETRACK_STATE_OBJECT(VkCommandBuffer, CMD_BUFFER_STATE)
VALSTATETRACK_STATE_OBJECT(VkCommandPool, COMMAND_POOL_STATE)
VALSTATETRACK_STATE_OBJECT(VkPipelineLayout, PIPELINE_LAYOUT_STATE)
VALSTATETRACK_STATE_OBJECT(VkFence, FENCE_STATE)
VALSTATETRACK_STATE_OBJECT(VkQueryPool, QUERY_POOL_STATE)
VALSTATETRACK_STATE_OBJECT(VkSemaphore, SEMAPHORE_STATE)
VALSTATETRACK_STATE_OBJECT(VkEvent, EVENT_STATE)
VALSTATETRACK_STATE_OBJECT(VkSamplerYcbcrConversion, SAMPLER_YCBCR_CONVERSION_STATE)
VALSTATETRACK_STATE_OBJECT(VkVideoSessionKHR, VIDEO_SESSION_STATE)
VALSTATETRACK_STATE_OBJECT(VkVideoSessionParametersKHR, VIDEO_SESSION_PARAMETERS_STATE)
VALSTATETRACK_STATE_OBJECT(VkAccelerationStructureKHR, ACCELERATION_STRUCTURE_STATE_KHR)
VALSTATETRACK_STATE_OBJECT(VkSurfaceKHR, SURFACE_STATE)
VALSTATETRACK_STATE_OBJECT(VkDisplayModeKHR, DISPLAY_MODE_STATE)
VALSTATETRACK_STATE_OBJECT(VkPhysicalDevice, PHYSICAL_DEVICE_STATE)

class ValidationStateTracker : public ValidationObject {
    using Func = vvl::Func;

  private:
    // NOTE: The Dummy argument allows for *partial* specialization at class scope, as full specialization at class scope
    //       isn't supported until C++17.  Since the Dummy has a default all instantiations of the template can ignore it, but all
    //       specializations of the template must list it (and not give it a default).
    // These must be declared at the same access level as the map declarations (below).
    template <typename State, typename Dummy = int>
    struct MapTraits {};

    template <typename State, typename BaseType = typename state_object::Traits<State>::BaseType,
              typename MapTraits = MapTraits<BaseType>>
    typename MapTraits::MapType& GetStateMap() {
        auto map_member = MapTraits::Map();
        return (MapTraits::kInstanceScope && (this->*map_member).size() == 0) ? instance_state->*map_member : this->*map_member;
    }
    template <typename State, typename BaseType = typename state_object::Traits<State>::BaseType,
              typename MapTraits = MapTraits<BaseType>>
    const typename MapTraits::MapType& GetStateMap() const {
        auto map_member = MapTraits::Map();
        return (MapTraits::kInstanceScope && (this->*map_member).size() == 0) ? instance_state->*map_member : this->*map_member;
    }

  public:
    static VkBindImageMemoryInfo ConvertImageMemoryInfo(VkDevice device, VkImage image, VkDeviceMemory mem,
                                                        VkDeviceSize memoryOffset);

    template <typename State, typename HandleType = typename state_object::Traits<State>::HandleType>
    void Add(std::shared_ptr<State>&& state_object) {
        auto& map = GetStateMap<State>();
        auto handle = state_object->Handle().template Cast<HandleType>();
        // Finish setting up the object node tree, which cannot be done from the state object contructors
        // due to use of shared_from_this()
        state_object->LinkChildNodes();
        map.insert_or_assign(handle, std::move(state_object));
    }

    template <typename State, typename Traits = typename state_object::Traits<State>>
    void Destroy(typename Traits::HandleType handle) {
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

    template <typename State, typename Fn>
    void ForEachShared(Fn&& fn) const {
        const auto& map = GetStateMap<State>();
        for (const auto& entry : map.snapshot()) {
            fn(entry.second);
        }
    }

    template <typename State, typename Fn>
    void ForEachShared(Fn&& fn) {
        auto& map = GetStateMap<State>();
        for (const auto& entry : map.snapshot()) {
            fn(entry.second);
        }
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

    template <typename State, typename Traits = typename state_object::Traits<State>>
    typename Traits::SharedType Get(typename Traits::HandleType handle) {
        const auto& map = GetStateMap<State>();
        const auto found_it = map.find(handle);
        if (found_it == map.end()) {
            return nullptr;
        }
        // NOTE: vl_concurrent_unordered_map::find() makes a copy of the value, so it is safe to move out.
        // But this will break everything, when switching to a different map type.
        return std::static_pointer_cast<State>(std::move(found_it->second));
    }

    template <typename State, typename Traits = typename state_object::Traits<State>>
    typename Traits::ConstSharedType Get(typename Traits::HandleType handle) const {
        const auto& map = GetStateMap<State>();
        const auto found_it = map.find(handle);
        if (found_it == map.end()) {
            return nullptr;
        }
        return std::static_pointer_cast<State>(std::move(found_it->second));
    }

    // GetRead() and GetWrite() return an already locked state object. Currently this is only supported by
    // CMD_BUFFER_STATE, because it has public ReadLock() and WriteLock() methods.
    // NOTE: Calling base class hook methods with a CMD_BUFFER_STATE lock held will lead to deadlock. Instead,
    // call the base class hook method before getting/locking the command buffer state for processing in the
    // derived class method.
    template <typename State, typename Traits = typename state_object::Traits<State>,
              typename ReadLockedType = typename Traits::ReadLockedType>
    ReadLockedType GetRead(typename Traits::HandleType handle) const {
        auto ptr = Get<State>(handle);
        if (ptr) {
            auto guard = ptr->ReadLock();
            return ReadLockedType(std::move(ptr), std::move(guard));
        } else {
            return ReadLockedType();
        }
    }

    template <typename State, typename Traits = state_object::Traits<State>,
              typename WriteLockedType = typename Traits::WriteLockedType>
    WriteLockedType GetWrite(typename Traits::HandleType handle) {
        auto ptr = Get<State>(handle);
        if (ptr) {
            auto guard = ptr->WriteLock();
            return WriteLockedType(std::move(ptr), std::move(guard));
        } else {
            return WriteLockedType();
        }
    }

    // When needing to share ownership, control over constness of access with another object (i.e. adding references while
    // not modifying the contents of the ValidationStateTracker)
    template <typename State, typename Traits = state_object::Traits<State>>
    typename Traits::SharedType GetConstCastShared(typename Traits::HandleType handle) const {
        const auto& map = GetStateMap<State>();
        const auto found_it = map.find(handle);
        if (found_it == map.end()) {
            return nullptr;
        }
        return found_it->second;
    }

    // From the spec:
    // If multiple VkBuffer objects are bound to overlapping ranges of VkDeviceMemory, implementations may return
    // address ranges which overlap. In this case, it is ambiguous which VkBuffer is associated with any given
    // device address. For purposes of valid usage, if multiple VkBuffer objects can be attributed to
    // a device address, a VkBuffer is selected such that valid usage passes, if it exists.
    // Regarding using raw pointers instead of shared: The reason is performance, because arrays of BUFFER_STATE_PTR are used, it is
    // more efficient to store them using raw pointers. It is safe to do so (at time of writing) because those raw pointers come
    // from shared ones created when the buffer is first recorded, and they are removed from buffer_address_map_ at BufferDestroy
    // time
    using BUFFER_STATE_PTR = BUFFER_STATE*;
    vvl::span<BUFFER_STATE_PTR> GetBuffersByAddress(VkDeviceAddress address) {
        ReadLockGuard guard(buffer_address_lock_);
        auto found_it = buffer_address_map_.find(address);
        if (found_it == buffer_address_map_.end()) {
            return vvl::make_span<BUFFER_STATE_PTR>(nullptr, static_cast<size_t>(0));
        }
        return found_it->second;
    }

    vvl::span<const BUFFER_STATE_PTR> GetBuffersByAddress(VkDeviceAddress address) const {
        ReadLockGuard guard(buffer_address_lock_);
        auto found_it = buffer_address_map_.find(address);
        if (found_it == buffer_address_map_.end()) {
            return vvl::make_span<const BUFFER_STATE_PTR>(nullptr, static_cast<size_t>(0));
        }
        return found_it->second;
    }

    using BufferAddressRange = sparse_container::range<VkDeviceAddress>;
    std::vector<BufferAddressRange> GetBufferAddressRanges() const {
        ReadLockGuard guard(buffer_address_lock_);
        std::vector<BufferAddressRange> result;
        result.reserve(buffer_address_map_.size());
        for (const auto& entry : buffer_address_map_) {
            result.push_back(entry.first);
        }
        return result;
    }

    using SetImageViewInitialLayoutCallback = std::function<void(CMD_BUFFER_STATE*, const IMAGE_VIEW_STATE&, VkImageLayout)>;
    template <typename Fn>
    void SetSetImageViewInitialLayoutCallback(Fn&& fn) {
        set_image_view_initial_layout_callback.reset(new SetImageViewInitialLayoutCallback(std::forward<Fn>(fn)));
    }

    void CallSetImageViewInitialLayoutCallback(CMD_BUFFER_STATE* cb_state, const IMAGE_VIEW_STATE& iv_state, VkImageLayout layout) {
        if (set_image_view_initial_layout_callback) {
            (*set_image_view_initial_layout_callback)(cb_state, iv_state, layout);
        }
    }

    VkDeviceSize AllocFakeMemory(VkDeviceSize size) { return fake_memory.Alloc(size); }
    void FreeFakeMemory(VkDeviceSize address) { fake_memory.Free(address); }

    // State update functions
    // Gets/Enumerations
    virtual std::shared_ptr<PHYSICAL_DEVICE_STATE> CreatePhysicalDeviceState(VkPhysicalDevice phys_dev);
    void PostCallRecordCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                      VkInstance* pInstance, const RecordObject& record_obj) override;
    void PostCallRecordEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(
        VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, uint32_t* pCounterCount, VkPerformanceCounterKHR* pCounters,
        VkPerformanceCounterDescriptionKHR* pCounterDescriptions, const RecordObject& record_obj) override;
    void PostCallRecordGetAccelerationStructureMemoryRequirementsNV(VkDevice device,
                                                                    const VkAccelerationStructureMemoryRequirementsInfoNV* pInfo,
                                                                    VkMemoryRequirements2* pMemoryRequirements,
                                                                    const RecordObject& record_obj) override;

    void PostCallRecordGetVideoSessionMemoryRequirementsKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                            uint32_t* pMemoryRequirementsCount,
                                                            VkVideoSessionMemoryRequirementsKHR* pMemoryRequirements,
                                                            const RecordObject& record_obj) override;

    virtual std::shared_ptr<QUEUE_STATE> CreateQueue(VkQueue queue, uint32_t queue_family_index, VkDeviceQueueCreateFlags flags,
                                                     const VkQueueFamilyProperties& queueFamilyProperties);

    void PostCallRecordGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue,
                                      const RecordObject& record_obj) override;
    void PostCallRecordGetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2* pQueueInfo, VkQueue* pQueue,
                                       const RecordObject& record_obj) override;
    void PostCallRecordGetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR* pGetFdInfo, int* pFd,
                                     const RecordObject& record_obj) override;
    void PostCallRecordGetFenceStatus(VkDevice device, VkFence fence, const RecordObject& record_obj) override;
#ifdef VK_USE_PLATFORM_WIN32_KHR
    void PostCallRecordGetFenceWin32HandleKHR(VkDevice device, const VkFenceGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                              HANDLE* pHandle, const RecordObject& record_obj) override;
#endif  // VK_USE_PLATFORM_WIN32_KHR
    void PostCallRecordGetImageMemoryRequirements(VkDevice device, VkImage image, VkMemoryRequirements* pMemoryRequirements,
                                                  const RecordObject& record_obj) override;
    void PostCallRecordGetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
                                                   VkMemoryRequirements2* pMemoryRequirements,
                                                   const RecordObject& record_obj) override;
    void PostCallRecordGetImageMemoryRequirements2KHR(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
                                                      VkMemoryRequirements2* pMemoryRequirements,
                                                      const RecordObject& record_obj) override;
    void PostCallRecordGetImageSparseMemoryRequirements(VkDevice device, VkImage image, uint32_t* pSparseMemoryRequirementCount,
                                                        VkSparseImageMemoryRequirements* pSparseMemoryRequirements,
                                                        const RecordObject& record_obj) override;
    void PostCallRecordGetImageSparseMemoryRequirements2(VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo,
                                                         uint32_t* pSparseMemoryRequirementCount,
                                                         VkSparseImageMemoryRequirements2* pSparseMemoryRequirements,
                                                         const RecordObject& record_obj) override;
    void PostCallRecordGetImageSparseMemoryRequirements2KHR(VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo,
                                                            uint32_t* pSparseMemoryRequirementCount,
                                                            VkSparseImageMemoryRequirements2* pSparseMemoryRequirements,
                                                            const RecordObject& record_obj) override;
    void PostCallRecordGetPhysicalDeviceDisplayPlanePropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                                  VkDisplayPlanePropertiesKHR* pProperties,
                                                                  const RecordObject& record_obj) override;
    void PostCallRecordGetPhysicalDeviceDisplayPlaneProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                                   VkDisplayPlaneProperties2KHR* pProperties,
                                                                   const RecordObject& record_obj) override;
    void PostCallRecordGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount,
                                                              VkQueueFamilyProperties* pQueueFamilyProperties,
                                                              const RecordObject& record_obj) override;
    void PostCallRecordGetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount,
                                                               VkQueueFamilyProperties2* pQueueFamilyProperties,
                                                               const RecordObject& record_obj) override;
    void PostCallRecordGetPhysicalDeviceQueueFamilyProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                  uint32_t* pQueueFamilyPropertyCount,
                                                                  VkQueueFamilyProperties2* pQueueFamilyProperties,
                                                                  const RecordObject& record_obj) override;
    void PostCallRecordGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                               VkSurfaceCapabilitiesKHR* pSurfaceCapabilities,
                                                               const RecordObject& record_obj) override;
    void PostCallRecordGetPhysicalDeviceSurfaceCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                                const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                                VkSurfaceCapabilities2KHR* pSurfaceCapabilities,
                                                                const RecordObject& record_obj) override;
    void PostCallRecordGetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                VkSurfaceCapabilities2EXT* pSurfaceCapabilities,
                                                                const RecordObject& record_obj) override;
    void PostCallRecordGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                          uint32_t* pSurfaceFormatCount, VkSurfaceFormatKHR* pSurfaceFormats,
                                                          const RecordObject& record_obj) override;
    void PostCallRecordGetPhysicalDeviceSurfaceFormats2KHR(VkPhysicalDevice physicalDevice,
                                                           const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                           uint32_t* pSurfaceFormatCount, VkSurfaceFormat2KHR* pSurfaceFormats,
                                                           const RecordObject& record_obj) override;
    void PostCallRecordGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                               uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes,
                                                               const RecordObject& record_obj) override;
    void PostCallRecordGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                          VkSurfaceKHR surface, VkBool32* pSupported,
                                                          const RecordObject& record_obj) override;
    void PostCallRecordGetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR* pGetFdInfo, int* pFd,
                                         const RecordObject& record_obj) override;
#ifdef VK_USE_PLATFORM_WIN32_KHR
    void PostCallRecordGetSemaphoreWin32HandleKHR(VkDevice device, const VkSemaphoreGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                                  HANDLE* pHandle, const RecordObject& record_obj) override;
#endif  // VK_USE_PLATFORM_WIN32_KHR
    void PostCallRecordGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount,
                                             VkImage* pSwapchainImages, const RecordObject& record_obj) override;
#ifdef VK_USE_PLATFORM_WIN32_KHR
    void PostCallRecordGetMemoryWin32HandleKHR(VkDevice device, const VkMemoryGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                               HANDLE* pHandle, const RecordObject& record_obj) override;
#endif  // VK_USE_PLATFORM_WIN32_KHR
    void PostCallRecordGetMemoryFdKHR(VkDevice device, const VkMemoryGetFdInfoKHR* pGetFdInfo, int* pFd,
                                      const RecordObject& record_obj) override;
    void PostCallRecordImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR* pImportFenceFdInfo,
                                        const RecordObject& record_obj) override;
#ifdef VK_USE_PLATFORM_WIN32_KHR
    void PostCallRecordImportFenceWin32HandleKHR(VkDevice device,
                                                 const VkImportFenceWin32HandleInfoKHR* pImportFenceWin32HandleInfo,
                                                 const RecordObject& record_obj) override;
#endif  // VK_USE_PLATFORM_WIN32_KHR
    void PostCallRecordImportSemaphoreFdKHR(VkDevice device, const VkImportSemaphoreFdInfoKHR* pImportSemaphoreFdInfo,
                                            const RecordObject& record_obj) override;
#ifdef VK_USE_PLATFORM_WIN32_KHR
    void PostCallRecordImportSemaphoreWin32HandleKHR(VkDevice device,
                                                     const VkImportSemaphoreWin32HandleInfoKHR* pImportSemaphoreWin32HandleInfo,
                                                     const RecordObject& record_obj) override;
#endif  // VK_USE_PLATFORM_WIN32_KHR
    void PreCallRecordSignalSemaphoreKHR(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo) override;
    void PreCallRecordSignalSemaphore(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo) override;
    void PostCallRecordSignalSemaphoreKHR(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo,
                                          const RecordObject& record_obj) override;
    void PostCallRecordSignalSemaphore(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo,
                                       const RecordObject& record_obj) override;

    // Create/Destroy/Bind
    void PostCallRecordBindAccelerationStructureMemoryNV(VkDevice device, uint32_t bindInfoCount,
                                                         const VkBindAccelerationStructureMemoryInfoNV* pBindInfos,
                                                         const RecordObject& record_obj) override;
    void PostCallRecordBindVideoSessionMemoryKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                 uint32_t bindSessionMemoryInfoCount,
                                                 const VkBindVideoSessionMemoryInfoKHR* pBindSessionMemoryInfos,
                                                 const RecordObject& record_obj) override;
    void PostCallRecordBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory mem, VkDeviceSize memoryOffset,
                                        const RecordObject& record_obj) override;
    void PostCallRecordBindBufferMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos,
                                         const RecordObject& record_obj) override;
    void PostCallRecordBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos,
                                            const RecordObject& record_obj) override;
    void PostCallRecordBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory mem, VkDeviceSize memoryOffset,
                                       const RecordObject& record_obj) override;
    void PostCallRecordBindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos,
                                        const RecordObject& record_obj) override;
    void PostCallRecordBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos,
                                           const RecordObject& record_obj) override;

    void PostCallRecordCreateDevice(VkPhysicalDevice gpu, const VkDeviceCreateInfo* pCreateInfo,
                                    const VkAllocationCallbacks* pAllocator, VkDevice* pDevice,
                                    const RecordObject& record_obj) override;
    virtual void CreateDevice(const VkDeviceCreateInfo* pCreateInfo);

    void PreCallRecordDestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator) override;

    virtual std::shared_ptr<ACCELERATION_STRUCTURE_STATE_NV> CreateAccelerationStructureState(
        VkAccelerationStructureNV as, const VkAccelerationStructureCreateInfoNV* pCreateInfo);
    void PostCallRecordCreateAccelerationStructureNV(VkDevice device, const VkAccelerationStructureCreateInfoNV* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator,
                                                     VkAccelerationStructureNV* pAccelerationStructure,
                                                     const RecordObject& record_obj) override;
    void PreCallRecordDestroyAccelerationStructureNV(VkDevice device, VkAccelerationStructureNV accelerationStructure,
                                                     const VkAllocationCallbacks* pAllocator) override;

    virtual std::shared_ptr<ACCELERATION_STRUCTURE_STATE_KHR> CreateAccelerationStructureState(VkAccelerationStructureKHR as, const VkAccelerationStructureCreateInfoKHR * pCreateInfo,
                                                                                               std::shared_ptr<BUFFER_STATE> &&buf_state, VkDeviceAddress address);
    void PostCallRecordCreateAccelerationStructureKHR(VkDevice device, const VkAccelerationStructureCreateInfoKHR* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator,
                                                      VkAccelerationStructureKHR* pAccelerationStructure,
                                                      const RecordObject& record_obj) override;
    void PostCallRecordBuildAccelerationStructuresKHR(VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount,
                                                      const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
                                                      const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos,
                                                      const RecordObject& record_obj) override;
    void RecordDeviceAccelerationStructureBuildInfo(CMD_BUFFER_STATE& cb_state,
                                                    const VkAccelerationStructureBuildGeometryInfoKHR& info);
    void PostCallRecordCmdBuildAccelerationStructuresKHR(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                                         const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
                                                         const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos,
                                                         const RecordObject& record_obj) override;

    void PostCallRecordCmdBuildAccelerationStructuresIndirectKHR(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                                                 const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
                                                                 const VkDeviceAddress* pIndirectDeviceAddresses,
                                                                 const uint32_t* pIndirectStrides,
                                                                 const uint32_t* const* ppMaxPrimitiveCounts,
                                                                 const RecordObject& record_obj) override;
    void PreCallRecordDestroyAccelerationStructureKHR(VkDevice device, VkAccelerationStructureKHR accelerationStructure,
                                                      const VkAllocationCallbacks* pAllocator) override;

    virtual std::shared_ptr<BUFFER_STATE> CreateBufferState(VkBuffer buf, const VkBufferCreateInfo* pCreateInfo);
    void PostCallRecordCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                    VkBuffer* pBuffer, const RecordObject& record_obj) override;
    void PreCallRecordDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator) override;

    virtual std::shared_ptr<BUFFER_VIEW_STATE> CreateBufferViewState(const std::shared_ptr<BUFFER_STATE> &bf, VkBufferView bv, const VkBufferViewCreateInfo *ci, VkFormatFeatureFlags2KHR buf_ff);
    void PostCallRecordCreateBufferView(VkDevice device, const VkBufferViewCreateInfo* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkBufferView* pView,
                                        const RecordObject& record_obj) override;
    void PreCallRecordDestroyBufferView(VkDevice device, VkBufferView bufferView, const VkAllocationCallbacks* pAllocator) override;
    virtual std::shared_ptr<COMMAND_POOL_STATE> CreateCommandPoolState(VkCommandPool command_pool,
                                                                       const VkCommandPoolCreateInfo* pCreateInfo);
    void PostCallRecordCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool,
                                         const RecordObject& record_obj) override;
    void PreCallRecordDestroyCommandPool(VkDevice device, VkCommandPool commandPool,
                                         const VkAllocationCallbacks* pAllocator) override;
    void PostCallRecordCreateDisplayPlaneSurfaceKHR(VkInstance instance, const VkDisplaySurfaceCreateInfoKHR* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                    const RecordObject& record_obj) override;
    void PostCallRecordCreateEvent(VkDevice device, const VkEventCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                   VkEvent* pEvent, const RecordObject& record_obj) override;
    void PreCallRecordDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks* pAllocator) override;

    virtual std::shared_ptr<DESCRIPTOR_POOL_STATE> CreateDescriptorPoolState(VkDescriptorPool pool,
                                                                             const VkDescriptorPoolCreateInfo* pCreateInfo);
    virtual std::shared_ptr<cvdescriptorset::DescriptorSet> CreateDescriptorSet(
        VkDescriptorSet, DESCRIPTOR_POOL_STATE*, const std::shared_ptr<cvdescriptorset::DescriptorSetLayout const>& layout,
        uint32_t variable_count);

    void PostCallRecordCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkDescriptorPool* pDescriptorPool,
                                            const RecordObject& record_obj) override;
    void PreCallRecordDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                            const VkAllocationCallbacks* pAllocator) override;
    void PostCallRecordCreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkDescriptorSetLayout* pSetLayout,
                                                 const RecordObject& record_obj) override;
    void PostCallRecordResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags,
                                          const RecordObject& record_obj) override;
    void PostCallRecordResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags,
                                        const RecordObject& record_obj) override;

    virtual std::shared_ptr<PIPELINE_STATE> CreateComputePipelineState(const VkComputePipelineCreateInfo* pCreateInfo,
                                                                       std::shared_ptr<const PIPELINE_LAYOUT_STATE>&& layout) const;
    bool PreCallValidateCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                               const VkComputePipelineCreateInfo* pCreateInfos,
                                               const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                               const ErrorObject& error_obj, void* pipe_state) const override;
    void PostCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                              const VkComputePipelineCreateInfo* pCreateInfos,
                                              const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                              const RecordObject& record_obj, void* pipe_state) override;
    void PostCallRecordResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorPoolResetFlags flags,
                                           const RecordObject& record_obj) override;
    bool PreCallValidateAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo,
                                               VkDescriptorSet* pDescriptorSets, const ErrorObject& error_obj,
                                               void* ads_state_data) const override;
    void PreCallRecordDestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout,
                                                 const VkAllocationCallbacks* pAllocator) override;
    void PostCallRecordCreateDescriptorUpdateTemplate(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator,
                                                      VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate,
                                                      const RecordObject& record_obj) override;
    void PostCallRecordCreateDescriptorUpdateTemplateKHR(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
                                                         const VkAllocationCallbacks* pAllocator,
                                                         VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate,
                                                         const RecordObject& record_obj) override;
    void PreCallRecordDestroyDescriptorUpdateTemplate(VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                      const VkAllocationCallbacks* pAllocator) override;
    void PreCallRecordDestroyDescriptorUpdateTemplateKHR(VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                         const VkAllocationCallbacks* pAllocator) override;
    void PostCallRecordCreateFence(VkDevice device, const VkFenceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                   VkFence* pFence, const RecordObject& record_obj) override;
    void PreCallRecordDestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks* pAllocator) override;
    void PostCallRecordResetFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences,
                                   const RecordObject& record_obj) override;
    void PostCallRecordCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer,
                                         const RecordObject& record_obj) override;
    void PreCallRecordDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer,
                                         const VkAllocationCallbacks* pAllocator) override;

    virtual std::shared_ptr<PIPELINE_CACHE_STATE> CreatePipelineCacheState(VkPipelineCache pipeline_cache,
                                                                           const VkPipelineCacheCreateInfo* pCreateInfo) const;
    void PostCallRecordCreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkPipelineCache* pPipelineCache,
                                           const RecordObject& record_obj) override;
    void PreCallRecordDestroyPipelineCache(VkDevice device, VkPipelineCache pipelineCache,
                                           const VkAllocationCallbacks* pAllocator) override;

    virtual std::shared_ptr<PIPELINE_STATE> CreateGraphicsPipelineState(const VkGraphicsPipelineCreateInfo* pCreateInfo,
                                                                        std::shared_ptr<const RENDER_PASS_STATE>&& render_pass,
                                                                        std::shared_ptr<const PIPELINE_LAYOUT_STATE>&& layout,
                                                                        CreateShaderModuleStates* csm_states) const;
    bool PreCallValidateCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                                const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                const ErrorObject& error_obj, void* cgpl_state) const override;
    void PostCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                               const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                               const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                               const RecordObject& record_obj, void* cgpl_state) override;

    virtual std::shared_ptr<IMAGE_STATE> CreateImageState(VkImage img, const VkImageCreateInfo* pCreateInfo,
                                                          VkFormatFeatureFlags2KHR features);
    virtual std::shared_ptr<IMAGE_STATE> CreateImageState(VkImage img, const VkImageCreateInfo* pCreateInfo,
                                                          VkSwapchainKHR swapchain, uint32_t swapchain_index,
                                                          VkFormatFeatureFlags2KHR features);
    void PostCallRecordCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                   VkImage* pImage, const RecordObject& record_obj) override;
    void PreCallRecordDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator) override;


    virtual std::shared_ptr<IMAGE_VIEW_STATE> CreateImageViewState(const std::shared_ptr<IMAGE_STATE> &image_state, VkImageView iv, const VkImageViewCreateInfo *ci,
                                                                   VkFormatFeatureFlags2KHR ff, const VkFilterCubicImageViewImageFormatPropertiesEXT &cubic_props);
    void PostCallRecordCreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkImageView* pView,
                                       const RecordObject& record_obj) override;
    void PreCallRecordDestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks* pAllocator) override;

    void PreCallRecordDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks* pAllocator) override;
    void PostCallRecordCreateShadersEXT(VkDevice device, uint32_t createInfoCount, const VkShaderCreateInfoEXT* pCreateInfos,
                                        const VkAllocationCallbacks* pAllocator, VkShaderEXT* pShaders,
                                        const RecordObject& record_obj, void* csm_state_data) override;
    void PreCallRecordDestroyShaderEXT(VkDevice device, VkShaderEXT shader, const VkAllocationCallbacks* pAllocator) override;
    void PostCallRecordCmdBindShadersEXT(VkCommandBuffer commandBuffer, uint32_t stageCount, const VkShaderStageFlagBits* pStages,
                                         const VkShaderEXT* pShaders, const RecordObject& record_obj) override;
    void PostCallRecordCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout,
                                            const RecordObject& record_obj) override;
    void PreCallRecordDestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout,
                                            const VkAllocationCallbacks* pAllocator) override;
    void PostCallRecordCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo* pCreateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkQueryPool* pQueryPool,
                                       const RecordObject& record_obj) override;
    void PreCallRecordDestroyQueryPool(VkDevice device, VkQueryPool queryPool, const VkAllocationCallbacks* pAllocator) override;
    void RecordResetQueryPool(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount);
    void PostCallRecordResetQueryPoolEXT(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                         const RecordObject& record_obj) override;
    void PostCallRecordResetQueryPool(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                      const RecordObject& record_obj) override;

    virtual std::shared_ptr<PIPELINE_STATE> CreateRayTracingPipelineState(
        const VkRayTracingPipelineCreateInfoNV* pCreateInfo, std::shared_ptr<const PIPELINE_LAYOUT_STATE>&& layout) const;
    bool PreCallValidateCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                    const VkRayTracingPipelineCreateInfoNV* pCreateInfos,
                                                    const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                    const ErrorObject& error_obj, void* pipe_state) const override;
    void PostCallRecordCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                   const VkRayTracingPipelineCreateInfoNV* pCreateInfos,
                                                   const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                   const RecordObject& record_obj, void* pipe_state) override;
    virtual std::shared_ptr<PIPELINE_STATE> CreateRayTracingPipelineState(
        const VkRayTracingPipelineCreateInfoKHR* pCreateInfo, std::shared_ptr<const PIPELINE_LAYOUT_STATE>&& layout) const;
    bool PreCallValidateCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                     VkPipelineCache pipelineCache, uint32_t count,
                                                     const VkRayTracingPipelineCreateInfoKHR* pCreateInfos,
                                                     const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                     const ErrorObject& error_obj, void* pipe_state) const override;
    void PostCallRecordCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                    VkPipelineCache pipelineCache, uint32_t count,
                                                    const VkRayTracingPipelineCreateInfoKHR* pCreateInfos,
                                                    const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                    const RecordObject& record_obj, void* pipe_state) override;
    void PostCallRecordCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass,
                                        const RecordObject& record_obj) override;
    void PostCallRecordCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass,
                                            const RecordObject& record_obj) override;
    void PostCallRecordCreateRenderPass2(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass,
                                         const RecordObject& record_obj) override;
    void PreCallRecordDestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks* pAllocator) override;
    void PostCallRecordCreateVideoSessionKHR(VkDevice device, const VkVideoSessionCreateInfoKHR* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkVideoSessionKHR* pVideoSession,
                                             const RecordObject& record_obj) override;
    void PreCallRecordDestroyVideoSessionKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                             const VkAllocationCallbacks* pAllocator) override;
    void PostCallRecordCreateVideoSessionParametersKHR(VkDevice device, const VkVideoSessionParametersCreateInfoKHR* pCreateInfo,
                                                       const VkAllocationCallbacks* pAllocator,
                                                       VkVideoSessionParametersKHR* pVideoSessionParameters,
                                                       const RecordObject& record_obj) override;
    void PostCallRecordUpdateVideoSessionParametersKHR(VkDevice device, VkVideoSessionParametersKHR videoSessionParameters,
                                                       const VkVideoSessionParametersUpdateInfoKHR* pUpdateInfo,
                                                       const RecordObject& record_obj) override;
    void PreCallRecordDestroyVideoSessionParametersKHR(VkDevice device, VkVideoSessionParametersKHR videoSessionParameters,
                                                       const VkAllocationCallbacks* pAllocator) override;

    virtual std::shared_ptr<SAMPLER_STATE> CreateSamplerState(VkSampler s, const VkSamplerCreateInfo *ci);
    void PostCallRecordCreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo,
                                     const VkAllocationCallbacks* pAllocator, VkSampler* pSampler,
                                     const RecordObject& record_obj) override;
    void PreCallRecordDestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks* pAllocator) override;
    void PostCallRecordCreateSamplerYcbcrConversion(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator,
                                                    VkSamplerYcbcrConversion* pYcbcrConversion,
                                                    const RecordObject& record_obj) override;
    void PostCallRecordDestroySamplerYcbcrConversion(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                                     const VkAllocationCallbacks* pAllocator,
                                                     const RecordObject& record_obj) override;
    void PostCallRecordCreateSamplerYcbcrConversionKHR(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                       const VkAllocationCallbacks* pAllocator,
                                                       VkSamplerYcbcrConversion* pYcbcrConversion,
                                                       const RecordObject& record_obj) override;
    void PostCallRecordDestroySamplerYcbcrConversionKHR(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                                        const VkAllocationCallbacks* pAllocator,
                                                        const RecordObject& record_obj) override;
    void PostCallRecordCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore,
                                       const RecordObject& record_obj) override;
    void PreCallRecordDestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks* pAllocator) override;

    void PreCallRecordCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule,
                                         void* csm_state_data) override;
    void PreCallRecordCreateShadersEXT(VkDevice device, uint32_t createInfoCount, const VkShaderCreateInfoEXT* pCreateInfos,
                                       const VkAllocationCallbacks* pAllocator, VkShaderEXT* pShaders,
                                       void* csm_state_data) override;

    void PostCallRecordCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule,
                                          const RecordObject& record_obj, void* csm_state_data) override;
    void PreCallRecordDestroyShaderModule(VkDevice device, VkShaderModule shaderModule,
                                          const VkAllocationCallbacks* pAllocator) override;
    void PreCallRecordDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface,
                                        const VkAllocationCallbacks* pAllocator) override;
    void PostCallRecordCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount,
                                                 const VkSwapchainCreateInfoKHR* pCreateInfos,
                                                 const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchains,
                                                 const RecordObject& record_obj) override;
    void PostCallRecordCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain,
                                          const RecordObject& record_obj) override;
    void PreCallRecordDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain,
                                          const VkAllocationCallbacks* pAllocator) override;
    void PostCallRecordCreateDisplayModeKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                            const VkDisplayModeCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                            VkDisplayModeKHR* pMode, const RecordObject& record_obj) override;

    // CommandBuffer/Queue Control
    void PreCallRecordBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo) override;
    void PostCallRecordDeviceWaitIdle(VkDevice device, const RecordObject& record_obj) override;
    void PostCallRecordEndCommandBuffer(VkCommandBuffer commandBuffer, const RecordObject& record_obj) override;
    void PreCallRecordQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo,
                                      VkFence fence) override;
    void PostCallRecordQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo,
                                       const RecordObject& record_obj) override;
    void PreCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence) override;
    void PostCallRecordQueueWaitIdle(VkQueue queue, const RecordObject& record_obj) override;
    void PreCallRecordSetEvent(VkDevice device, VkEvent event) override;
    void PostCallRecordWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll,
                                     uint64_t timeout, const RecordObject& record_obj) override;
    void PreCallRecordWaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout) override;
    void PreCallRecordWaitSemaphoresKHR(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout) override;
    void PostCallRecordWaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout,
                                      const RecordObject& record_obj) override;
    void PostCallRecordWaitSemaphoresKHR(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout,
                                         const RecordObject& record_obj) override;
    void PostCallRecordGetSemaphoreCounterValue(VkDevice device, VkSemaphore semaphore, uint64_t* pValue,
                                                const RecordObject& record_obj) override;
    void PostCallRecordGetSemaphoreCounterValueKHR(VkDevice device, VkSemaphore semaphore, uint64_t* pValue,
                                                   const RecordObject& record_obj) override;
    void PostCallRecordAcquireProfilingLockKHR(VkDevice device, const VkAcquireProfilingLockInfoKHR* pInfo,
                                               const RecordObject& record_obj) override;
    void PostCallRecordReleaseProfilingLockKHR(VkDevice device, const RecordObject& record_obj) override;

    virtual std::shared_ptr<CMD_BUFFER_STATE> CreateCmdBufferState(VkCommandBuffer cb,
                                                                   const VkCommandBufferAllocateInfo* create_info,
                                                                   const COMMAND_POOL_STATE* pool);
    // Allocate/Free
    void PostCallRecordAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* pCreateInfo,
                                              VkCommandBuffer* pCommandBuffer, const RecordObject& record_obj) override;
    void PostCallRecordAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo,
                                              VkDescriptorSet* pDescriptorSets, const RecordObject& record_obj,
                                              void* ads_state) override;
    void PostCallRecordAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo,
                                      const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory,
                                      const RecordObject& record_obj) override;
    void PreCallRecordFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount,
                                         const VkCommandBuffer* pCommandBuffers) override;
    void PreCallRecordFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t count,
                                         const VkDescriptorSet* pDescriptorSets) override;
    void PreCallRecordFreeMemory(VkDevice device, VkDeviceMemory mem, const VkAllocationCallbacks* pAllocator) override;

    void PerformUpdateDescriptorSets(uint32_t, const VkWriteDescriptorSet*, uint32_t, const VkCopyDescriptorSet*);

    void PreCallRecordUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount,
                                           const VkWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount,
                                           const VkCopyDescriptorSet* pDescriptorCopies) override;
    void PreCallRecordUpdateDescriptorSetWithTemplate(VkDevice device, VkDescriptorSet descriptorSet,
                                                      VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                      const void* pData) override;
    void PreCallRecordUpdateDescriptorSetWithTemplateKHR(VkDevice device, VkDescriptorSet descriptorSet,
                                                         VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                         const void* pData) override;

    virtual std::shared_ptr<DEVICE_MEMORY_STATE> CreateDeviceMemoryState(
        VkDeviceMemory mem, const VkMemoryAllocateInfo* p_alloc_info, uint64_t fake_address, const VkMemoryType& memory_type,
        const VkMemoryHeap& memory_heap, std::optional<DedicatedBinding>&& dedicated_binding, uint32_t physical_device_count);

    // Memory mapping
    void PostCallRecordMapMemory(VkDevice device, VkDeviceMemory mem, VkDeviceSize offset, VkDeviceSize size, VkFlags flags,
                                 void** ppData, const RecordObject& record_obj) override;
    void PostCallRecordMapMemory2KHR(VkDevice device, const VkMemoryMapInfoKHR* pMemoryMapInfo, void** ppData,
                                     const RecordObject& record_obj) override;
    void PreCallRecordUnmapMemory(VkDevice device, VkDeviceMemory mem) override;
    void PreCallRecordUnmapMemory2KHR(VkDevice device, const VkMemoryUnmapInfoKHR* pMemoryUnmapInfo) override;

    // Recorded Commands
    void PreCallRecordCmdBeginDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo) override;
    void PostCallRecordCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot, VkQueryControlFlags flags,
                                     const RecordObject& record_obj) override;
    void PostCallRecordCmdBeginQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                               VkQueryControlFlags flags, uint32_t index, const RecordObject& record_obj) override;
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
    void PreCallRecordCmdBeginVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoBeginCodingInfoKHR* pBeginInfo) override;
    void PostCallRecordCmdBeginTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                                    uint32_t counterBufferCount, const VkBuffer* pCounterBuffers,
                                                    const VkDeviceSize* pCounterBufferOffsets,
                                                    const RecordObject& record_obj) override;
    void PostCallRecordCmdEndTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                                  uint32_t counterBufferCount, const VkBuffer* pCounterBuffers,
                                                  const VkDeviceSize* pCounterBufferOffsets,
                                                  const RecordObject& record_obj) override;
    void PostCallRecordCmdBeginConditionalRenderingEXT(VkCommandBuffer commandBuffer,
                                                       const VkConditionalRenderingBeginInfoEXT* pConditionalRenderingBegin,
                                                       const RecordObject& record_obj) override;
    void PostCallRecordCmdEndConditionalRenderingEXT(VkCommandBuffer commandBuffer, const RecordObject& record_obj) override;
    void PreCallRecordCmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                            VkPipelineLayout layout, uint32_t firstSet, uint32_t setCount,
                                            const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount,
                                            const uint32_t* pDynamicOffsets) override;
    void PreCallRecordCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                         VkIndexType indexType) override;
    void PreCallRecordCmdBindIndexBuffer2KHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize size,
                                             VkIndexType indexType) override;
    void PreCallRecordCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                      VkPipeline pipeline) override;
    void PostCallRecordCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline,
                                       const RecordObject& record_obj) override;
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
                                                       VkBuffer scratch, VkDeviceSize scratchOffset,
                                                       const RecordObject& record_obj) override;
    void PreCallRecordCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                         const VkClearColorValue* pColor, uint32_t rangeCount,
                                         const VkImageSubresourceRange* pRanges) override;
    void PreCallRecordCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount,
                                                const VkImageSubresourceRange* pRanges) override;
    void PostCallRecordCmdControlVideoCodingKHR(VkCommandBuffer commandBuffer,
                                                const VkVideoCodingControlInfoKHR* pCodingControlInfo,
                                                const RecordObject& record_obj) override;
    void PostCallRecordCmdCopyAccelerationStructureNV(VkCommandBuffer commandBuffer, VkAccelerationStructureNV dst,
                                                      VkAccelerationStructureNV src, VkCopyAccelerationStructureModeNV mode,
                                                      const RecordObject& record_obj) override;
    void PreCallRecordCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount,
                                    const VkBufferCopy* pRegions) override;
    void PreCallRecordCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2KHR* pCopyBufferInfo) override;
    void PreCallRecordCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2* pCopyBufferInfo) override;
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
                                               VkQueryResultFlags flags, const RecordObject& record_obj) override;
    void PostCallRecordCmdDecodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoDecodeInfoKHR* pDecodeInfo,
                                         const RecordObject& record_obj) override;
    void PostCallRecordCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z,
                                   const RecordObject& record_obj) override;
    void PostCallRecordCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                           const RecordObject& record_obj) override;
    void PostCallRecordCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t base_x, uint32_t base_y, uint32_t base_z,
                                          uint32_t x, uint32_t y, uint32_t z, const RecordObject& record_obj) override;
    void PostCallRecordCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t base_x, uint32_t base_y, uint32_t base_z, uint32_t x,
                                       uint32_t y, uint32_t z, const RecordObject& record_obj) override;
    void PostCallRecordCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                               uint32_t firstInstance, const RecordObject& record_obj) override;
    void PostCallRecordCmdDrawMultiEXT(VkCommandBuffer commandBuffer, uint32_t drawCount, const VkMultiDrawInfoEXT* pVertexInfo,
                                       uint32_t instanceCount, uint32_t firstInstance, uint32_t stride,
                                       const RecordObject& record_obj) override;
    void PostCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                      uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance,
                                      const RecordObject& record_obj) override;
    void PostCallRecordCmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                              const VkMultiDrawIndexedInfoEXT* pIndexInfo, uint32_t instanceCount,
                                              uint32_t firstInstance, uint32_t stride, const int32_t* pVertexOffset,
                                              const RecordObject& record_obj) override;
    void PostCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count,
                                              uint32_t stride, const RecordObject& record_obj) override;
    void PostCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count,
                                       uint32_t stride, const RecordObject& record_obj) override;
    void RecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                           VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                           uint32_t stride, Func command);
    void PreCallRecordCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                     VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                     uint32_t stride) override;
    void PreCallRecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                  VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                  uint32_t stride) override;
    void RecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer,
                                    VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride, Func command);
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
    void PreCallRecordCmdDrawMeshTasksIndirectCountEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                       uint32_t stride) override;
    void PreCallRecordCmdDrawMeshTasksIndirectEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                  uint32_t drawCount, uint32_t stride) override;
    void PreCallRecordCmdDrawMeshTasksEXT(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                          uint32_t groupCountZ) override;
    void PostCallRecordCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer,
                                      VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer,
                                      VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride,
                                      VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset,
                                      VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer,
                                      VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride,
                                      uint32_t width, uint32_t height, uint32_t depth, const RecordObject& record_obj) override;
    void PostCallRecordCmdTraceRaysKHR(VkCommandBuffer commandBuffer,
                                       const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                                       const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                                       const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                                       const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable, uint32_t width,
                                       uint32_t height, uint32_t depth, const RecordObject& record_obj) override;
    void PostCallRecordCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer,
                                               const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                                               const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                                               const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                                               const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable,
                                               VkDeviceAddress indirectDeviceAddress, const RecordObject& record_obj) override;
    void PostCallRecordCmdTraceRaysIndirect2KHR(VkCommandBuffer commandBuffer, VkDeviceAddress indirectDeviceAddress,
                                                const RecordObject& record_obj) override;
    void PostCallRecordCmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const RecordObject& record_obj) override;
    void PostCallRecordCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot,
                                   const RecordObject& record_obj) override;
    void PostCallRecordCmdEndQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, uint32_t index,
                                             const RecordObject& record_obj) override;
    void PostCallRecordCmdEndRenderPass(VkCommandBuffer commandBuffer, const RecordObject& record_obj) override;
    void PostCallRecordCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo,
                                            const RecordObject& record_obj) override;
    void PostCallRecordCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo,
                                         const RecordObject& record_obj) override;
    void PostCallRecordCmdEndVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoEndCodingInfoKHR* pEndCodingInfo,
                                            const RecordObject& record_obj) override;
    void PreCallRecordCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBuffersCount,
                                         const VkCommandBuffer* pCommandBuffers) override;
    void PreCallRecordCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size,
                                    uint32_t data) override;
    void PreCallRecordCmdInsertDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo) override;
    void PostCallRecordCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents,
                                      const RecordObject& record_obj) override;
    void PostCallRecordCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo* pSubpassBeginInfo,
                                          const VkSubpassEndInfo* pSubpassEndInfo, const RecordObject& record_obj) override;
    void PostCallRecordCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo* pSubpassBeginInfo,
                                       const VkSubpassEndInfo* pSubpassEndInfo, const RecordObject& record_obj) override;
    void PreCallRecordCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                              VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                              const VkWriteDescriptorSet* pDescriptorWrites) override;
    void PreCallRecordCmdPushDescriptorSetWithTemplateKHR(VkCommandBuffer commandBuffer,
                                                          VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                          VkPipelineLayout layout, uint32_t set, const void* pData) override;
    void PostCallRecordCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlags stageFlags,
                                        uint32_t offset, uint32_t size, const void* pValues,
                                        const RecordObject& record_obj) override;
    void PreCallRecordCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) override;
    void PostCallRecordCmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                         uint32_t queryCount, const RecordObject& record_obj) override;
    void PreCallRecordCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                      VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                      const VkImageResolve* pRegions) override;
    void PreCallRecordCmdResolveImage2KHR(VkCommandBuffer commandBuffer, const VkResolveImageInfo2KHR* pResolveImageInfo) override;
    void PreCallRecordCmdResolveImage2(VkCommandBuffer commandBuffer, const VkResolveImageInfo2* pResolveImageInfo) override;
    void PostCallRecordCmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4],
                                            const RecordObject& record_obj) override;
    void PostCallRecordCmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp,
                                       float depthBiasSlopeFactor, const RecordObject& record_obj) override;
    void PostCallRecordCmdSetDepthBias2EXT(VkCommandBuffer commandBuffer, const VkDepthBiasInfoEXT* pDepthBiasInfo,
                                           const RecordObject& record_obj) override;
    void PostCallRecordCmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds,
                                         const RecordObject& record_obj) override;
    void PreCallRecordCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) override;
    void PostCallRecordCmdSetExclusiveScissorNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                                uint32_t exclusiveScissorCount, const VkRect2D* pExclusiveScissors,
                                                const RecordObject& record_obj) override;
    void PostCallRecordCmdSetExclusiveScissorEnableNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                                      uint32_t exclusiveScissorCount, const VkBool32* pExclusiveScissorEnables,
                                                      const RecordObject& record_obj) override;
    void PostCallRecordCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth, const RecordObject& record_obj) override;
    void PostCallRecordCmdSetLineStippleEXT(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor, uint16_t lineStipplePattern,
                                            const RecordObject& record_obj) override;
    void PostCallRecordCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount,
                                     const VkRect2D* pScissors, const RecordObject& record_obj) override;
    void PostCallRecordCmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t compareMask,
                                                const RecordObject& record_obj) override;
    void PostCallRecordCmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t reference,
                                              const RecordObject& record_obj) override;
    void PostCallRecordCmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t writeMask,
                                              const RecordObject& record_obj) override;
    void PostCallRecordCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                      const VkViewport* pViewports, const RecordObject& record_obj) override;
    void PostCallRecordCmdSetViewportShadingRatePaletteNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                          uint32_t viewportCount,
                                                          const VkShadingRatePaletteNV* pShadingRatePalettes,
                                                          const RecordObject& record_obj) override;
    void PostCallRecordCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                       VkDeviceSize dataSize, const void* pData, const RecordObject& record_obj) override;
    void PreCallRecordCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                    VkPipelineStageFlags sourceStageMask, VkPipelineStageFlags dstStageMask,
                                    uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                    uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                    uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers) override;
    void PostCallRecordCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                         VkQueryPool queryPool, uint32_t slot, const RecordObject& record_obj) override;
    void PostCallRecordCmdWriteAccelerationStructuresPropertiesKHR(VkCommandBuffer commandBuffer,
                                                                   uint32_t accelerationStructureCount,
                                                                   const VkAccelerationStructureKHR* pAccelerationStructures,
                                                                   VkQueryType queryType, VkQueryPool queryPool,
                                                                   uint32_t firstQuery, const RecordObject& record_obj) override;
    void PostCallRecordCmdSetViewportWScalingNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                                const VkViewportWScalingNV* pViewportWScalings,
                                                const RecordObject& record_obj) override;
    void PostCallRecordCmdSetVertexInputEXT(VkCommandBuffer commandBuffer, uint32_t vertexBindingDescriptionCount,
                                            const VkVertexInputBindingDescription2EXT* pVertexBindingDescriptions,
                                            uint32_t vertexAttributeDescriptionCount,
                                            const VkVertexInputAttributeDescription2EXT* pVertexAttributeDescriptions,
                                            const RecordObject& record_obj) override;
    void PostCallRecordCmdSetColorWriteEnableEXT(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                                 const VkBool32* pColorWriteEnables, const RecordObject& record_obj) override;
    void PostCallRecordCmdSetAttachmentFeedbackLoopEnableEXT(VkCommandBuffer commandBuffer, VkImageAspectFlags aspectMask,
                                                             const RecordObject& record_obj) override;
#ifdef VK_USE_PLATFORM_WIN32_KHR
    void PostCallRecordAcquireFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain,
                                                         const RecordObject& record_obj) override;
    void PostCallRecordReleaseFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain,
                                                         const RecordObject& record_obj) override;
#endif
    void PostCallRecordCmdSetTessellationDomainOriginEXT(VkCommandBuffer commandBuffer, VkTessellationDomainOrigin domainOrigin,
                                                         const RecordObject& record_obj) override;
    void PostCallRecordCmdSetDepthClampEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClampEnable,
                                                 const RecordObject& record_obj) override;
    void PostCallRecordCmdSetPolygonModeEXT(VkCommandBuffer commandBuffer, VkPolygonMode polygonMode,
                                            const RecordObject& record_obj) override;
    void PostCallRecordCmdSetRasterizationSamplesEXT(VkCommandBuffer commandBuffer, VkSampleCountFlagBits rasterizationSamples,
                                                     const RecordObject& record_obj) override;
    void PostCallRecordCmdSetSampleMaskEXT(VkCommandBuffer commandBuffer, VkSampleCountFlagBits samples,
                                           const VkSampleMask* pSampleMask, const RecordObject& record_obj) override;
    void PostCallRecordCmdSetAlphaToCoverageEnableEXT(VkCommandBuffer commandBuffer, VkBool32 alphaToCoverageEnable,
                                                      const RecordObject& record_obj) override;
    void PostCallRecordCmdSetAlphaToOneEnableEXT(VkCommandBuffer commandBuffer, VkBool32 alphaToOneEnable,
                                                 const RecordObject& record_obj) override;
    void PostCallRecordCmdSetLogicOpEnableEXT(VkCommandBuffer commandBuffer, VkBool32 logicOpEnable,
                                              const RecordObject& record_obj) override;
    void PostCallRecordCmdSetColorBlendEnableEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment, uint32_t attachmentCount,
                                                 const VkBool32* pColorBlendEnables, const RecordObject& record_obj) override;
    void PostCallRecordCmdSetColorBlendEquationEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                   uint32_t attachmentCount, const VkColorBlendEquationEXT* pColorBlendEquations,
                                                   const RecordObject& record_obj) override;
    void PostCallRecordCmdSetColorWriteMaskEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment, uint32_t attachmentCount,
                                               const VkColorComponentFlags* pColorWriteMasks,
                                               const RecordObject& record_obj) override;
    void PostCallRecordCmdSetRasterizationStreamEXT(VkCommandBuffer commandBuffer, uint32_t rasterizationStream,
                                                    const RecordObject& record_obj) override;
    void PostCallRecordCmdSetConservativeRasterizationModeEXT(VkCommandBuffer commandBuffer,
                                                              VkConservativeRasterizationModeEXT conservativeRasterizationMode,
                                                              const RecordObject& record_obj) override;
    void PostCallRecordCmdSetExtraPrimitiveOverestimationSizeEXT(VkCommandBuffer commandBuffer,
                                                                 float extraPrimitiveOverestimationSize,
                                                                 const RecordObject& record_obj) override;
    void PostCallRecordCmdSetDepthClipEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClipEnable,
                                                const RecordObject& record_obj) override;
    void PostCallRecordCmdSetSampleLocationsEnableEXT(VkCommandBuffer commandBuffer, VkBool32 sampleLocationsEnable,
                                                      const RecordObject& record_obj) override;
    void PostCallRecordCmdSetColorBlendAdvancedEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                   uint32_t attachmentCount, const VkColorBlendAdvancedEXT* pColorBlendAdvanced,
                                                   const RecordObject& record_obj) override;
    void PostCallRecordCmdSetProvokingVertexModeEXT(VkCommandBuffer commandBuffer, VkProvokingVertexModeEXT provokingVertexMode,
                                                    const RecordObject& record_obj) override;
    void PostCallRecordCmdSetLineRasterizationModeEXT(VkCommandBuffer commandBuffer,
                                                      VkLineRasterizationModeEXT lineRasterizationMode,
                                                      const RecordObject& record_obj) override;
    void PostCallRecordCmdSetLineStippleEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stippledLineEnable,
                                                  const RecordObject& record_obj) override;
    void PostCallRecordCmdSetDepthClipNegativeOneToOneEXT(VkCommandBuffer commandBuffer, VkBool32 negativeOneToOne,
                                                          const RecordObject& record_obj) override;
    void PostCallRecordCmdSetViewportWScalingEnableNV(VkCommandBuffer commandBuffer, VkBool32 viewportWScalingEnable,
                                                      const RecordObject& record_obj) override;
    void PostCallRecordCmdSetViewportSwizzleNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                               const VkViewportSwizzleNV* pViewportSwizzles,
                                               const RecordObject& record_obj) override;
    void PostCallRecordCmdSetCoverageToColorEnableNV(VkCommandBuffer commandBuffer, VkBool32 coverageToColorEnable,
                                                     const RecordObject& record_obj) override;
    void PostCallRecordCmdSetCoverageToColorLocationNV(VkCommandBuffer commandBuffer, uint32_t coverageToColorLocation,
                                                       const RecordObject& record_obj) override;
    void PostCallRecordCmdSetCoverageModulationModeNV(VkCommandBuffer commandBuffer,
                                                      VkCoverageModulationModeNV coverageModulationMode,
                                                      const RecordObject& record_obj) override;
    void PostCallRecordCmdSetCoverageModulationTableEnableNV(VkCommandBuffer commandBuffer, VkBool32 coverageModulationTableEnable,
                                                             const RecordObject& record_obj) override;
    void PostCallRecordCmdSetCoverageModulationTableNV(VkCommandBuffer commandBuffer, uint32_t coverageModulationTableCount,
                                                       const float* pCoverageModulationTable,
                                                       const RecordObject& record_obj) override;
    void PostCallRecordCmdSetShadingRateImageEnableNV(VkCommandBuffer commandBuffer, VkBool32 shadingRateImageEnable,
                                                      const RecordObject& record_obj) override;
    void PostCallRecordCmdSetRepresentativeFragmentTestEnableNV(VkCommandBuffer commandBuffer,
                                                                VkBool32 representativeFragmentTestEnable,
                                                                const RecordObject& record_obj) override;
    void PostCallRecordCmdSetCoverageReductionModeNV(VkCommandBuffer commandBuffer, VkCoverageReductionModeNV coverageReductionMode,
                                                     const RecordObject& record_obj) override;

    template <typename CreateInfo>
    VkFormatFeatureFlags2KHR GetExternalFormatFeaturesANDROID(const CreateInfo* create_info) const;
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    void PostCallRecordGetAndroidHardwareBufferPropertiesANDROID(VkDevice device, const struct AHardwareBuffer* buffer,
                                                                 VkAndroidHardwareBufferPropertiesANDROID* pProperties,
                                                                 const RecordObject& record_obj) override;
#endif  // VK_USE_PLATFORM_ANDROID_KHR

    // WSI
    void PostCallRecordAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore,
                                           VkFence fence, uint32_t* pImageIndex, const RecordObject& record_obj) override;
    void PostCallRecordAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo, uint32_t* pImageIndex,
                                            const RecordObject& record_obj) override;
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    void PostCallRecordCreateAndroidSurfaceKHR(VkInstance instance, const VkAndroidSurfaceCreateInfoKHR* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                               const RecordObject& record_obj) override;
#endif  // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_FUCHSIA
    void PostCallRecordCreateImagePipeSurfaceFUCHSIA(VkInstance instance, const VkImagePipeSurfaceCreateInfoFUCHSIA* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                     const RecordObject& record_obj) override;
#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_IOS_MVK
    void PostCallRecordCreateIOSSurfaceMVK(VkInstance instance, const VkIOSSurfaceCreateInfoMVK* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                           const RecordObject& record_obj) override;
#endif  // VK_USE_PLATFORM_IOS_MVK
#ifdef VK_USE_PLATFORM_MACOS_MVK
    void PostCallRecordCreateMacOSSurfaceMVK(VkInstance instance, const VkMacOSSurfaceCreateInfoMVK* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                             const RecordObject& record_obj) override;
#endif  // VK_USE_PLATFORM_MACOS_MVK
#ifdef VK_USE_PLATFORM_METAL_EXT
    void PostCallRecordCreateMetalSurfaceEXT(VkInstance instance, const VkMetalSurfaceCreateInfoEXT* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                             const RecordObject& record_obj) override;
#endif  // VK_USE_PLATFORM_METAL_EXT
#ifdef VK_USE_PLATFORM_WIN32_KHR
    void PostCallRecordCreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                             const RecordObject& record_obj) override;
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    void PostCallRecordCreateWaylandSurfaceKHR(VkInstance instance, const VkWaylandSurfaceCreateInfoKHR* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                               const RecordObject& record_obj) override;
#endif  // VK_USE_PLATFORM_WAYLAND_KHR
#ifdef VK_USE_PLATFORM_XCB_KHR
    void PostCallRecordCreateXcbSurfaceKHR(VkInstance instance, const VkXcbSurfaceCreateInfoKHR* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                           const RecordObject& record_obj) override;
#endif  // VK_USE_PLATFORM_XCB_KHR
#ifdef VK_USE_PLATFORM_XLIB_KHR
    void PostCallRecordCreateXlibSurfaceKHR(VkInstance instance, const VkXlibSurfaceCreateInfoKHR* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                            const RecordObject& record_obj) override;
#endif  // VK_USE_PLATFORM_XLIB_KHR
#ifdef VK_USE_PLATFORM_SCREEN_QNX
    void PostCallRecordCreateScreenSurfaceQNX(VkInstance instance, const VkScreenSurfaceCreateInfoQNX* pCreateInfo,
                                              const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                              const RecordObject& record_obj) override;
#endif  // VK_USE_PLATFORM_SCREEN_QNX
    void PostCallRecordCreateHeadlessSurfaceEXT(VkInstance instance, const VkHeadlessSurfaceCreateInfoEXT* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                const RecordObject& record_obj) override;

    // State Utilty functions
    std::vector<std::shared_ptr<const IMAGE_VIEW_STATE>> GetAttachmentViews(const VkRenderPassBeginInfo& rp_begin,
                                                                            const FRAMEBUFFER_STATE& fb_state) const;

    VkFormatFeatureFlags2KHR GetPotentialFormatFeatures(VkFormat format) const;
    void PerformUpdateDescriptorSetsWithTemplateKHR(VkDescriptorSet descriptorSet, const UPDATE_TEMPLATE_STATE* template_state,
                                                    const void* pData);
    void RecordAcquireNextImageState(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore,
                                     VkFence fence, uint32_t* pImageIndex, vvl::Func command);
    void RecordCreateSamplerYcbcrConversionState(const VkSamplerYcbcrConversionCreateInfo* create_info,
                                                 VkSamplerYcbcrConversion ycbcr_conversion);
    virtual std::shared_ptr<SWAPCHAIN_NODE> CreateSwapchainState(const VkSwapchainCreateInfoKHR* create_info,
                                                                 VkSwapchainKHR swapchain);
    void RecordCreateSwapchainState(VkResult result, const VkSwapchainCreateInfoKHR* pCreateInfo, VkSwapchainKHR* pSwapchain,
                                    std::shared_ptr<SURFACE_STATE>&& surface_state, SWAPCHAIN_NODE* old_swapchain_state);
    void RecordEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCounters(VkPhysicalDevice physicalDevice,
                                                                          uint32_t queueFamilyIndex, uint32_t* pCounterCount,
                                                                          VkPerformanceCounterKHR* pCounters);
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
    void RecordVulkanSurface(VkSurfaceKHR* pSurface);
    void PreRecordWaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout);
    void PostRecordWaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout,
                                  const RecordObject& record_obj);
    void RecordGetSemaphoreCounterValue(VkDevice device, VkSemaphore semaphore, uint64_t* pValue, const RecordObject& record_obj);
    void UpdateBindBufferMemoryState(VkBuffer buffer, VkDeviceMemory mem, VkDeviceSize memoryOffset);
    void UpdateBindImageMemoryState(const VkBindImageMemoryInfo& bindInfo);
    void UpdateAllocateDescriptorSetsData(const VkDescriptorSetAllocateInfo*, cvdescriptorset::AllocateDescriptorSetsData*) const;

    void PostCallRecordCopyAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                    const VkCopyAccelerationStructureInfoKHR* pInfo,
                                                    const RecordObject& record_obj) override;
    void PostCallRecordCmdCopyAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                                       const VkCopyAccelerationStructureInfoKHR* pInfo,
                                                       const RecordObject& record_obj) override;
    void PostCallRecordCmdCopyAccelerationStructureToMemoryKHR(VkCommandBuffer commandBuffer,
                                                               const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo,
                                                               const RecordObject& record_obj) override;
    void PostCallRecordCmdCopyMemoryToAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                                               const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo,
                                                               const RecordObject& record_obj) override;
    void RecordCmdSetCullMode(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode, Func command);
    void PostCallRecordCmdSetCullMode(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode,
                                      const RecordObject& record_obj) override;
    void PostCallRecordCmdSetCullModeEXT(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode,
                                         const RecordObject& record_obj) override;
    void RecordCmdSetFrontFace(VkCommandBuffer commandBuffer, VkFrontFace frontFace, Func command);
    void PostCallRecordCmdSetFrontFace(VkCommandBuffer commandBuffer, VkFrontFace frontFace,
                                       const RecordObject& record_obj) override;
    void PostCallRecordCmdSetFrontFaceEXT(VkCommandBuffer commandBuffer, VkFrontFace frontFace,
                                          const RecordObject& record_obj) override;
    void RecordCmdSetPrimitiveTopology(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology, Func command);
    void PostCallRecordCmdSetPrimitiveTopologyEXT(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology,
                                                  const RecordObject& record_obj) override;
    void PostCallRecordCmdSetPrimitiveTopology(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology,
                                               const RecordObject& record_obj) override;
    void RecordCmdSetViewportWithCount(VkCommandBuffer commandBuffer, uint32_t viewportCount, const VkViewport* pViewports,
                                       Func command);
    void PostCallRecordCmdSetViewportWithCountEXT(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                                  const VkViewport* pViewports, const RecordObject& record_obj) override;
    void PostCallRecordCmdSetViewportWithCount(VkCommandBuffer commandBuffer, uint32_t viewportCount, const VkViewport* pViewports,
                                               const RecordObject& record_obj) override;
    void RecordCmdSetScissorWithCount(VkCommandBuffer commandBuffer, uint32_t scissorCount, const VkRect2D* pScissors,
                                      Func command);
    void PostCallRecordCmdSetScissorWithCountEXT(VkCommandBuffer commandBuffer, uint32_t scissorCount, const VkRect2D* pScissors,
                                                 const RecordObject& record_obj) override;
    void PostCallRecordCmdSetScissorWithCount(VkCommandBuffer commandBuffer, uint32_t scissorCount, const VkRect2D* pScissors,
                                              const RecordObject& record_obj) override;
    void RecordCmdBindVertexBuffers2(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                     const VkBuffer* pBuffers, const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes,
                                     const VkDeviceSize* pStrides, Func command);
    void PostCallRecordCmdBindVertexBuffers2EXT(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                                const VkBuffer* pBuffers, const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes,
                                                const VkDeviceSize* pStrides, const RecordObject& record_obj) override;
    void PostCallRecordCmdBindVertexBuffers2(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                             const VkBuffer* pBuffers, const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes,
                                             const VkDeviceSize* pStrides, const RecordObject& record_obj) override;
    void RecordCmdSetDepthTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable, Func command);
    void PostCallRecordCmdSetDepthTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable,
                                                const RecordObject& record_obj) override;
    void PostCallRecordCmdSetDepthTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable,
                                             const RecordObject& record_obj) override;
    void RecordCmdSetDepthWriteEnable(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable, Func command);
    void PostCallRecordCmdSetDepthWriteEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable,
                                                 const RecordObject& record_obj) override;
    void PostCallRecordCmdSetDepthWriteEnable(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable,
                                              const RecordObject& record_obj) override;
    void RecordCmdSetDepthCompareOp(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp, Func command);
    void PostCallRecordCmdSetDepthCompareOpEXT(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp,
                                               const RecordObject& record_obj) override;
    void PostCallRecordCmdSetDepthCompareOp(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp,
                                            const RecordObject& record_obj) override;
    void RecordCmdSetDepthBoundsTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable, Func command);
    void PostCallRecordCmdSetDepthBoundsTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable,
                                                      const RecordObject& record_obj) override;
    void PostCallRecordCmdSetDepthBoundsTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable,
                                                   const RecordObject& record_obj) override;
    void RecordCmdSetStencilTestEnable(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable, Func command);
    void PostCallRecordCmdSetStencilTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable,
                                                  const RecordObject& record_obj) override;
    void PostCallRecordCmdSetStencilTestEnable(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable,
                                               const RecordObject& record_obj) override;
    void RecordCmdSetStencilOp(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp, VkStencilOp passOp,
                               VkStencilOp depthFailOp, VkCompareOp compareOp, Func command);
    void PostCallRecordCmdSetStencilOpEXT(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp,
                                          VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp,
                                          const RecordObject& record_obj) override;
    void PostCallRecordCmdSetStencilOp(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp,
                                       VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp,
                                       const RecordObject& record_obj) override;
    void PostCallRecordCmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer, uint32_t firstDiscardRectangle,
                                                 uint32_t discardRectangleCount, const VkRect2D* pDiscardRectangles,
                                                 const RecordObject& record_obj) override;
    void PostCallRecordCmdSetDiscardRectangleEnableEXT(VkCommandBuffer commandBuffer, VkBool32 discardRectangleEnable,
                                                       const RecordObject& record_obj) override;
    void PostCallRecordCmdSetDiscardRectangleModeEXT(VkCommandBuffer commandBuffer, VkDiscardRectangleModeEXT discardRectangleMode,
                                                     const RecordObject& record_obj) override;
    void PostCallRecordCmdSetSampleLocationsEXT(VkCommandBuffer commandBuffer, const VkSampleLocationsInfoEXT* pSampleLocationsInfo,
                                                const RecordObject& record_obj) override;
    void PostCallRecordCmdSetCoarseSampleOrderNV(VkCommandBuffer commandBuffer, VkCoarseSampleOrderTypeNV sampleOrderType,
                                                 uint32_t customSampleOrderCount,
                                                 const VkCoarseSampleOrderCustomNV* pCustomSampleOrders,
                                                 const RecordObject& record_obj) override;

    void PostCallRecordCmdSetPatchControlPointsEXT(VkCommandBuffer commandBuffer, uint32_t patchControlPoints,
                                                   const RecordObject& record_obj) override;
    void PostCallRecordCmdSetLogicOpEXT(VkCommandBuffer commandBuffer, VkLogicOp logicOp, const RecordObject& record_obj) override;
    void RecordCmdSetRasterizerDiscardEnable(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable, Func command);
    void PostCallRecordCmdSetRasterizerDiscardEnableEXT(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable,
                                                        const RecordObject& record_obj) override;
    void PostCallRecordCmdSetRasterizerDiscardEnable(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable,
                                                     const RecordObject& record_obj) override;
    void RecordCmdSetDepthBiasEnable(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable, Func command);
    void PostCallRecordCmdSetDepthBiasEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable,
                                                const RecordObject& record_obj) override;
    void PostCallRecordCmdSetDepthBiasEnable(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable,
                                             const RecordObject& record_obj) override;
    void RecordCmdSetPrimitiveRestartEnable(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable, Func command);
    void PostCallRecordCmdSetPrimitiveRestartEnableEXT(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable,
                                                       const RecordObject& record_obj) override;
    void PostCallRecordCmdSetPrimitiveRestartEnable(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable,
                                                    const RecordObject& record_obj) override;
    void PostCallRecordCmdSetFragmentShadingRateKHR(VkCommandBuffer commandBuffer, const VkExtent2D* pFragmentSize,
                                                    const VkFragmentShadingRateCombinerOpKHR combinerOps[2],
                                                    const RecordObject& record_obj) override;

    void PostCallRecordCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                          VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                          uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                          uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                          uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers,
                                          const RecordObject& record_obj) override;

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
                                             uint32_t query, const RecordObject& record_obj) override;
    void PostCallRecordCmdWriteTimestamp2(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage, VkQueryPool queryPool,
                                          uint32_t query, const RecordObject& record_obj) override;
    void RecordQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR* pSubmits, VkFence fence);
    void PreCallRecordQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR* pSubmits,
                                      VkFence fence) override;
    void PreCallRecordQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence) override;

    void PostCallRecordGetDescriptorSetLayoutSizeEXT(VkDevice device, VkDescriptorSetLayout layout,
                                                     VkDeviceSize* pLayoutSizeInBytes, const RecordObject& record_obj) override;
    void PreCallRecordCmdSetDescriptorBufferOffsetsEXT(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                       VkPipelineLayout layout, uint32_t firstSet, uint32_t setCount,
                                                       const uint32_t* pBufferIndices, const VkDeviceSize* pOffsets) override;
    void PreCallRecordCmdBindDescriptorBuffersEXT(VkCommandBuffer commandBuffer, uint32_t bufferCount,
                                                  const VkDescriptorBufferBindingInfoEXT* pBindingInfos) override;

    void RecordGetBufferDeviceAddress(const VkBufferDeviceAddressInfo* pInfo, const RecordObject& record_obj);
    void PostCallRecordGetBufferDeviceAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                              const RecordObject& record_obj) override;
    void PostCallRecordGetBufferDeviceAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                                 const RecordObject& record_obj) override;
    void PostCallRecordGetBufferDeviceAddressEXT(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                                 const RecordObject& record_obj) override;

    void PostCallRecordGetShaderModuleIdentifierEXT(VkDevice device, VkShaderModule shaderModule,
                                                    VkShaderModuleIdentifierEXT* pIdentifier,
                                                    const RecordObject& record_obj) override;
    void PostCallRecordGetShaderModuleCreateInfoIdentifierEXT(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                                              VkShaderModuleIdentifierEXT* pIdentifier,
                                                              const RecordObject& record_obj) override;
    void PreCallRecordCmdBindShadersEXT(VkCommandBuffer commandBuffer, uint32_t stageCount, const VkShaderStageFlagBits* pStages,
                                        const VkShaderEXT* pShaders) override;

    template <bool init = true, typename ExtProp>
    void GetPhysicalDeviceExtProperties(VkPhysicalDevice gpu, ExtEnabled enabled, ExtProp* ext_prop) {
        assert(ext_prop);
        if (IsExtEnabled(enabled)) {
            // Extensions that use two calls to get properties don't want to init on the second call
            if constexpr (init) {
                *ext_prop = vku::InitStructHelper();
            }
            if (api_version < VK_API_VERSION_1_1) {
                VkPhysicalDeviceProperties2 prop2 = vku::InitStructHelper(ext_prop);
                DispatchGetPhysicalDeviceProperties2KHR(gpu, &prop2);
            } else {
                VkPhysicalDeviceProperties2 prop2 = vku::InitStructHelper(ext_prop);
                DispatchGetPhysicalDeviceProperties2(gpu, &prop2);
            }
        }
    }

    template <typename ExtProp>
    void GetPhysicalDeviceExtProperties(VkPhysicalDevice gpu, ExtProp* ext_prop) {
        assert(ext_prop);
        *ext_prop = vku::InitStructHelper();
        if (api_version < VK_API_VERSION_1_1) {
            VkPhysicalDeviceProperties2 prop2 = vku::InitStructHelper(ext_prop);
            DispatchGetPhysicalDeviceProperties2KHR(gpu, &prop2);
        } else {
            VkPhysicalDeviceProperties2 prop2 = vku::InitStructHelper(ext_prop);
            DispatchGetPhysicalDeviceProperties2(gpu, &prop2);
        }
    }

    inline std::shared_ptr<SHADER_MODULE_STATE> GetShaderModuleStateFromIdentifier(const VkShaderModuleIdentifierEXT& ident) {
        ReadLockGuard guard(shader_identifier_map_lock_);
        if (const auto itr = shader_identifier_map_.find(ident); itr != shader_identifier_map_.cend()) {
            return itr->second;
        }
        return {};
    }

    inline std::shared_ptr<SHADER_MODULE_STATE> GetShaderModuleStateFromIdentifier(
        const VkPipelineShaderStageModuleIdentifierCreateInfoEXT& shader_stage_id) const {
        if (shader_stage_id.pIdentifier) {
            VkShaderModuleIdentifierEXT shader_id = vku::InitStructHelper();
            shader_id.identifierSize = shader_stage_id.identifierSize;
            const uint32_t copy_size = std::min(VK_MAX_SHADER_MODULE_IDENTIFIER_SIZE_EXT, shader_stage_id.identifierSize);
            std::copy(shader_stage_id.pIdentifier, shader_stage_id.pIdentifier + copy_size, shader_id.identifier);
            ReadLockGuard guard(shader_identifier_map_lock_);
            if (const auto itr = shader_identifier_map_.find(shader_id); itr != shader_identifier_map_.cend()) {
                return itr->second;
            }
        }
        return {};
    }

    inline std::optional<VkMemoryAllocateInfo> GetAllocateInfoFromFdHandle(int fd) const {
        ReadLockGuard guard(fd_handle_map_lock_);
        if (const auto itr = fd_handle_map_.find(fd); itr != fd_handle_map_.cend()) {
            return itr->second;
        }
        return {};
    }

#ifdef VK_USE_PLATFORM_WIN32_KHR
    inline std::optional<VkMemoryAllocateInfo> GetAllocateInfoFromWin32Handle(HANDLE handle) const {
        ReadLockGuard guard(win32_handle_map_lock_);
        if (const auto itr = win32_handle_map_.find(handle); itr != win32_handle_map_.cend()) {
            return itr->second;
        }
        return {};
    }
#endif

    // Link to the device's physical-device data
    PHYSICAL_DEVICE_STATE* physical_device_state;

    // Link for derived device objects back to their parent instance object
    ValidationStateTracker* instance_state;

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
#ifdef VK_USE_PLATFORM_METAL_EXT
    std::vector<VkExportMetalObjectTypeFlagBitsEXT> export_metal_flags;
#endif  // VK_USE_PLATFORM_METAL_EXT

    // Some extensions/features changes the behavior of the app/layers/spec if present.
    // So it needs its own special boolean unlike the enabled_fatures.
    bool has_format_feature2;  // VK_KHR_format_feature_flags2
    // VK_EXT_pipeline_robustness was designed to be a subset of robustness extensions
    // Enabling the other robustness features can reduce performance on GPU, so just the
    // support is needed to check
    bool has_robust_image_access;  // VK_EXT_image_robustness
    // TODO - Issue 5657
    // bool has_robust_image_access2;  // VK_EXT_robustness2
    // bool has_robust_buffer_access2; // VK_EXT_robustness2

    // Device extension properties -- storing properties gathered from VkPhysicalDeviceProperties2::pNext chain
    struct DeviceExtensionProperties {
        VkPhysicalDevicePushDescriptorPropertiesKHR push_descriptor_props;
        VkPhysicalDeviceShadingRateImagePropertiesNV shading_rate_image_props;
        VkPhysicalDeviceMeshShaderPropertiesNV mesh_shader_props_nv;
        VkPhysicalDeviceMeshShaderPropertiesEXT mesh_shader_props_ext;
        VkPhysicalDeviceInlineUniformBlockPropertiesEXT inline_uniform_block_props;
        VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT vtx_attrib_divisor_props;
        VkPhysicalDeviceCooperativeMatrixPropertiesNV cooperative_matrix_props;
        VkPhysicalDeviceCooperativeMatrixPropertiesKHR cooperative_matrix_props_khr;
        VkPhysicalDeviceTransformFeedbackPropertiesEXT transform_feedback_props;
        VkPhysicalDeviceRayTracingPropertiesNV ray_tracing_props_nv;
        VkPhysicalDeviceRayTracingPipelinePropertiesKHR ray_tracing_props_khr;
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
        VkPhysicalDeviceSubgroupProperties subgroup_props;
        VkPhysicalDeviceExtendedDynamicState3PropertiesEXT extended_dynamic_state3_props;
        VkPhysicalDeviceImageProcessingPropertiesQCOM image_processing_props;
        VkPhysicalDeviceDescriptorBufferPropertiesEXT descriptor_buffer_props;
        VkPhysicalDeviceDescriptorBufferDensityMapPropertiesEXT descriptor_buffer_density_props;
        VkPhysicalDeviceHostImageCopyPropertiesEXT host_image_copy_properties;
    };
    DeviceExtensionProperties phys_dev_ext_props = {};
    std::vector<VkCooperativeMatrixPropertiesNV> cooperative_matrix_properties;
    std::vector<VkCooperativeMatrixPropertiesKHR> cooperative_matrix_properties_khr;
    std::vector<VkImageLayout> host_image_copy_src_layouts;
    std::vector<VkImageLayout> host_image_copy_dst_layouts;

    // Features and properties that depend on platforms being defined
    // They will be false if platform is not defined
    bool android_external_format_resolve_null_color_attachment_prop = false;  // VK_ANDROID_external_format_resolve

    // Queue family extension properties -- storing queue family properties gathered from
    // VkQueueFamilyProperties2::pNext chain
    struct QueueFamilyExtensionProperties {
        VkQueueFamilyVideoPropertiesKHR video_props;
        VkQueueFamilyQueryResultStatusPropertiesKHR query_result_status_props;
    };
    std::vector<QueueFamilyExtensionProperties> queue_family_ext_props;

    bool performance_lock_acquired = false;
    uint32_t buffer_device_address_ranges_version = 0;

    mutable VideoProfileDesc::Cache video_profile_cache_;

    using BufferAddressMapStore = small_vector<BUFFER_STATE_PTR, 1, size_t>;
    using BufferAddressRangeMap = sparse_container::range_map<VkDeviceAddress, BufferAddressMapStore>;

  protected:
    // tracks which queue family index were used when creating the device for quick lookup
    vvl::unordered_set<uint32_t> queue_family_index_set;
    // The queue count can different for the same queueFamilyIndex if the create flag are different
    struct DeviceQueueInfo {
        uint32_t index;  // from VkDeviceCreateInfo
        uint32_t queue_family_index;
        VkDeviceQueueCreateFlags flags;
        uint32_t queue_count;
    };
    std::vector<DeviceQueueInfo> device_queue_info_list;
    // If vkGetBufferDeviceAddress is called, keep track of buffer <-> address mapping.
    BufferAddressRangeMap buffer_address_map_;
    mutable std::shared_mutex buffer_address_lock_;

    vl_concurrent_unordered_map<uint64_t, VkFormatFeatureFlags2KHR> ahb_ext_formats_map;
    std::atomic<VkDeviceSize> descriptorBufferAddressSpaceSize = {0u};
    std::atomic<VkDeviceSize> resourceDescriptorBufferAddressSpaceSize = {0u};
    std::atomic<VkDeviceSize> samplerDescriptorBufferAddressSpaceSize = {0u};

    // Keep track of identifier -> state
    vvl::unordered_map<VkShaderModuleIdentifierEXT, std::shared_ptr<SHADER_MODULE_STATE>> shader_identifier_map_;
    mutable std::shared_mutex shader_identifier_map_lock_;

    // If vkGetMemoryFdKHR is called, keep track of fd handle -> allocation info
    vvl::unordered_map<int, VkMemoryAllocateInfo> fd_handle_map_;
    mutable std::shared_mutex fd_handle_map_lock_;

#ifdef VK_USE_PLATFORM_WIN32_KHR
    // If vkGetMemoryWin32HandleKHR is called, keep track of HANDLE -> allocation info
    vvl::unordered_map<HANDLE, VkMemoryAllocateInfo> win32_handle_map_;
    mutable std::shared_mutex win32_handle_map_lock_;
#endif

  private:
    VALSTATETRACK_MAP_AND_TRAITS(VkQueue, QUEUE_STATE, queue_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkAccelerationStructureNV, ACCELERATION_STRUCTURE_STATE_NV, acceleration_structure_nv_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkRenderPass, RENDER_PASS_STATE, render_pass_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkDescriptorSetLayout, cvdescriptorset::DescriptorSetLayout, descriptor_set_layout_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkSampler, SAMPLER_STATE, sampler_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkImageView, IMAGE_VIEW_STATE, image_view_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkImage, IMAGE_STATE, image_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkBufferView, BUFFER_VIEW_STATE, buffer_view_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkBuffer, BUFFER_STATE, buffer_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkPipelineCache, PIPELINE_CACHE_STATE, pipeline_cache_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkPipeline, PIPELINE_STATE, pipeline_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkShaderEXT, SHADER_OBJECT_STATE, shader_object_map_)
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
    VALSTATETRACK_MAP_AND_TRAITS(VkVideoSessionKHR, VIDEO_SESSION_STATE, video_session_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkVideoSessionParametersKHR, VIDEO_SESSION_PARAMETERS_STATE, video_session_parameters_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkAccelerationStructureKHR, ACCELERATION_STRUCTURE_STATE_KHR, acceleration_structure_khr_map_)
    VALSTATETRACK_MAP_AND_TRAITS_INSTANCE_SCOPE(VkSurfaceKHR, SURFACE_STATE, surface_map_)
    VALSTATETRACK_MAP_AND_TRAITS_INSTANCE_SCOPE(VkDisplayModeKHR, DISPLAY_MODE_STATE, display_mode_map_)
    VALSTATETRACK_MAP_AND_TRAITS_INSTANCE_SCOPE(VkPhysicalDevice, PHYSICAL_DEVICE_STATE, physical_device_map_)

    // Simple base address allocator allow allow VkDeviceMemory allocations to appear to exist in a common address space.
    // At 256GB allocated/sec  ( > 8GB at 30Hz), will overflow in just over 2 years
    class FakeAllocator {
      public:
        void Free(VkDeviceSize fake_address){};  // Define the interface just in case we ever need to be cleverer.
        VkDeviceSize Alloc(VkDeviceSize size) {
            const auto alloc = free_.fetch_add(size);
            assert(std::numeric_limits<VkDeviceSize>::max() - size >= alloc);  //  776.722963 days later...
            return alloc;
        }

      private:
        std::atomic<VkDeviceSize> free_{1U << 20};  // start at 1mb to leave room for a NULL address
    };
    FakeAllocator fake_memory;
};

