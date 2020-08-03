/* Copyright (c) 2015-2020 The Khronos Group Inc.
 * Copyright (c) 2015-2020 Valve Corporation
 * Copyright (c) 2015-2020 LunarG, Inc.
 * Copyright (C) 2015-2020 Google Inc.
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
 */

#pragma once
#include "chassis.h"
#include "core_validation_error_enums.h"
#include "core_validation_types.h"
#include "descriptor_sets.h"
#include "vk_layer_logging.h"
#include "vulkan/vk_layer.h"
#include "vk_typemap_helper.h"
#include "vk_layer_data.h"
#include <atomic>
#include <functional>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <list>
#include <deque>
#include <map>

uint32_t ResolveRemainingLevels(const VkImageSubresourceRange* range, uint32_t mip_levels);
uint32_t ResolveRemainingLayers(const VkImageSubresourceRange* range, uint32_t layers);
VkImageSubresourceRange NormalizeSubresourceRange(const VkImageCreateInfo& image_create_info, const VkImageSubresourceRange& range);
std::pair<uint32_t, const VkImageView*> GetFramebufferAttachments(const VkRenderPassBeginInfo& rp_begin,
                                                                  const FRAMEBUFFER_STATE& fb_state);
VkImageSubresourceRange NormalizeSubresourceRange(const IMAGE_STATE& image_state, const VkImageSubresourceRange& range);
PIPELINE_STATE* GetCurrentPipelineFromCommandBuffer(const CMD_BUFFER_STATE& cmd, VkPipelineBindPoint pipelineBindPoint);
void GetCurrentPipelineAndDesriptorSetsFromCommandBuffer(const CMD_BUFFER_STATE& cmd, VkPipelineBindPoint pipelineBindPoint,
                                                         const PIPELINE_STATE** rtn_pipe,
                                                         const std::vector<LAST_BOUND_STATE::PER_SET>** rtn_sets);

enum SyncScope {
    kSyncScopeInternal,
    kSyncScopeExternalTemporary,
    kSyncScopeExternalPermanent,
};

enum FENCE_STATUS { FENCE_UNSIGNALED, FENCE_INFLIGHT, FENCE_RETIRED };

class FENCE_STATE : public BASE_NODE {
  public:
    VkFence fence;
    VkFenceCreateInfo createInfo;
    std::pair<VkQueue, uint64_t> signaler;
    FENCE_STATUS state;
    SyncScope scope;

    // Default constructor
    FENCE_STATE() : state(FENCE_UNSIGNALED), scope(kSyncScopeInternal) {}
};

class SEMAPHORE_STATE : public BASE_NODE {
  public:
    std::pair<VkQueue, uint64_t> signaler;
    bool signaled;
    SyncScope scope;
    VkSemaphoreTypeKHR type;
    uint64_t payload;
};

class EVENT_STATE : public BASE_NODE {
  public:
    int write_in_use;
    VkPipelineStageFlags stageMask;
};

class QUEUE_STATE {
  public:
    VkQueue queue;
    uint32_t queueFamilyIndex;

    uint64_t seq;
    std::deque<CB_SUBMISSION> submissions;
};

class QUERY_POOL_STATE : public BASE_NODE {
  public:
    VkQueryPoolCreateInfo createInfo;
    VkQueryPool pool;

    bool has_perf_scope_command_buffer = false;
    bool has_perf_scope_render_pass = false;
    uint32_t n_performance_passes = 0;
};

class SAMPLER_YCBCR_CONVERSION_STATE : public BASE_NODE {
  public:
    VkFormatFeatureFlags format_features;
    VkFormat format;
    VkFilter chromaFilter;
};

class QUEUE_FAMILY_PERF_COUNTERS {
  public:
    std::vector<VkPerformanceCounterKHR> counters;
};

struct PHYSICAL_DEVICE_STATE {
    // Track the call state and array sizes for various query functions
    CALL_STATE vkGetPhysicalDeviceQueueFamilyPropertiesState = UNCALLED;
    CALL_STATE vkGetPhysicalDeviceLayerPropertiesState = UNCALLED;
    CALL_STATE vkGetPhysicalDeviceExtensionPropertiesState = UNCALLED;
    CALL_STATE vkGetPhysicalDeviceFeaturesState = UNCALLED;
    CALL_STATE vkGetPhysicalDeviceSurfaceCapabilitiesKHRState = UNCALLED;
    bool vkGetPhysicalDeviceSurfaceCapabilitiesKHR_called = false;
    CALL_STATE vkGetPhysicalDeviceSurfacePresentModesKHRState = UNCALLED;
    CALL_STATE vkGetPhysicalDeviceSurfaceFormatsKHRState = UNCALLED;
    CALL_STATE vkGetPhysicalDeviceDisplayPlanePropertiesKHRState = UNCALLED;
    bool vkGetPhysicalDeviceDisplayPlanePropertiesKHR_called = false;
    safe_VkPhysicalDeviceFeatures2 features2 = {};
    VkPhysicalDevice phys_device = VK_NULL_HANDLE;
    uint32_t queue_family_known_count = 1;  // spec implies one QF must always be supported
    std::vector<VkQueueFamilyProperties> queue_family_properties;
    VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
    std::vector<VkPresentModeKHR> present_modes;
    std::vector<VkSurfaceFormatKHR> surface_formats;
    uint32_t display_plane_property_count = 0;

    // Map of queue family index to QUEUE_FAMILY_PERF_COUNTERS
    std::unordered_map<uint32_t, std::unique_ptr<QUEUE_FAMILY_PERF_COUNTERS>> perf_counters;
};

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
    std::vector<unsigned int> instrumented_pgm;
};

struct GpuQueue {
    VkPhysicalDevice gpu;
    uint32_t queue_family_index;
};

struct SubresourceRangeErrorCodes {
    const char *base_mip_err, *mip_count_err, *base_layer_err, *layer_count_err;
};

inline bool operator==(GpuQueue const& lhs, GpuQueue const& rhs) {
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

struct SURFACE_STATE : public BASE_NODE {
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    SWAPCHAIN_NODE* swapchain = nullptr;
    std::unordered_map<GpuQueue, bool> gpu_queue_support;

    SURFACE_STATE() {}
    SURFACE_STATE(VkSurfaceKHR surface) : surface(surface) {}
};

struct SubpassLayout {
    uint32_t index;
    VkImageLayout layout;
};

using std::unordered_map;

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

extern std::shared_ptr<cvdescriptorset::DescriptorSetLayout const> GetDslFromPipelineLayout(
    PIPELINE_LAYOUT_STATE const* layout_data, uint32_t set);

// Returns the effective extent of an image subresource, adjusted for mip level and array depth.
static inline VkExtent3D GetImageSubresourceExtent(const IMAGE_STATE* img, const VkImageSubresourceLayers* subresource) {
    const uint32_t mip = subresource->mipLevel;

    // Return zero extent if mip level doesn't exist
    if (mip >= img->createInfo.mipLevels) {
        return VkExtent3D{0, 0, 0};
    }

    // Don't allow mip adjustment to create 0 dim, but pass along a 0 if that's what subresource specified
    VkExtent3D extent = img->createInfo.extent;

    // If multi-plane, adjust per-plane extent
    if (FormatIsMultiplane(img->createInfo.format)) {
        VkExtent2D divisors = FindMultiplaneExtentDivisors(img->createInfo.format, subresource->aspectMask);
        extent.width /= divisors.width;
        extent.height /= divisors.height;
    }

    if (img->createInfo.flags & VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV) {
        extent.width = (0 == extent.width ? 0 : std::max(2U, 1 + ((extent.width - 1) >> mip)));
        extent.height = (0 == extent.height ? 0 : std::max(2U, 1 + ((extent.height - 1) >> mip)));
        extent.depth = (0 == extent.depth ? 0 : std::max(2U, 1 + ((extent.depth - 1) >> mip)));
    } else {
        extent.width = (0 == extent.width ? 0 : std::max(1U, extent.width >> mip));
        extent.height = (0 == extent.height ? 0 : std::max(1U, extent.height >> mip));
        extent.depth = (0 == extent.depth ? 0 : std::max(1U, extent.depth >> mip));
    }

    // Image arrays have an effective z extent that isn't diminished by mip level
    if (VK_IMAGE_TYPE_3D != img->createInfo.imageType) {
        extent.depth = img->createInfo.arrayLayers;
    }

    return extent;
}

// For image copies between compressed/uncompressed formats, the extent is provided in source image texels
// Destination image texel extents must be adjusted by block size for the dest validation checks
static inline VkExtent3D GetAdjustedDestImageExtent(VkFormat src_format, VkFormat dst_format, VkExtent3D extent) {
    VkExtent3D adjusted_extent = extent;
    if ((FormatIsCompressed(src_format) || FormatIsSinglePlane_422(src_format)) &&
        !(FormatIsCompressed(dst_format) || FormatIsSinglePlane_422(dst_format))) {
        VkExtent3D block_size = FormatTexelBlockExtent(src_format);
        adjusted_extent.width /= block_size.width;
        adjusted_extent.height /= block_size.height;
        adjusted_extent.depth /= block_size.depth;
    } else if (!(FormatIsCompressed(src_format) || FormatIsSinglePlane_422(src_format)) &&
               (FormatIsCompressed(dst_format) || FormatIsSinglePlane_422(dst_format))) {
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

static inline VkDeviceSize GetBufferSizeFromCopyImage(const VkBufferImageCopy& region, VkFormat image_format) {
    VkDeviceSize buffer_size = 0;
    VkExtent3D copy_extent = region.imageExtent;
    VkDeviceSize buffer_width = (0 == region.bufferRowLength ? copy_extent.width : region.bufferRowLength);
    VkDeviceSize buffer_height = (0 == region.bufferImageHeight ? copy_extent.height : region.bufferImageHeight);
    VkDeviceSize unit_size = FormatElementSize(image_format,
                                               region.imageSubresource.aspectMask);  // size (bytes) of texel or block

    if (FormatIsCompressed(image_format) || FormatIsSinglePlane_422(image_format)) {
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

struct SHADER_MODULE_STATE;

class ValidationStateTracker : public ValidationObject {
  public:
    //  TODO -- move to private
    //  TODO -- make consistent with traits approach below.
    unordered_map<VkQueue, QUEUE_STATE> queueMap;
    unordered_map<VkEvent, EVENT_STATE> eventMap;

    std::unordered_set<VkQueue> queues;  // All queues under given device
    QueryMap queryToStateMap;
    unordered_map<VkSamplerYcbcrConversion, uint64_t> ycbcr_conversion_ahb_fmt_map;
    unordered_map<uint64_t, VkFormatFeatureFlags> ahb_ext_formats_map;

    // Traits for State function resolution.  Specializations defined in the macro.
    // NOTE: The Dummy argument allows for *partial* specialization at class scope, as full specialization at class scope
    //       isn't supported until C++17.  Since the Dummy has a default all instantiations of the template can ignore it, but all
    //       specializations of the template must list it (and not give it a default).
    template <typename StateType, typename Dummy = int>
    struct AccessorStateHandle {};
    template <typename StateType, typename Dummy = int>
    struct AccessorTraits {};
    template <typename StateType_>
    struct AccessorTraitsTypes {
        using StateType = StateType_;
        using HandleType = typename AccessorStateHandle<StateType>::HandleType;
        using ReturnType = StateType*;
        using SharedType = std::shared_ptr<StateType>;
        using ConstSharedType = std::shared_ptr<const StateType>;
        using MappedType = std::shared_ptr<StateType>;
        using MapType = unordered_map<HandleType, MappedType>;
    };

    // Override base class, we have some extra work to do here
    void InitDeviceValidationObject(bool add_obj, ValidationObject* inst_obj, ValidationObject* dev_obj);

    VALSTATETRACK_MAP_AND_TRAITS(VkRenderPass, RENDER_PASS_STATE, renderPassMap)
    VALSTATETRACK_MAP_AND_TRAITS(VkDescriptorSetLayout, cvdescriptorset::DescriptorSetLayout, descriptorSetLayoutMap)
    VALSTATETRACK_MAP_AND_TRAITS(VkSampler, SAMPLER_STATE, samplerMap)
    VALSTATETRACK_MAP_AND_TRAITS(VkImageView, IMAGE_VIEW_STATE, imageViewMap)
    VALSTATETRACK_MAP_AND_TRAITS(VkImage, IMAGE_STATE, imageMap)
    VALSTATETRACK_MAP_AND_TRAITS(VkBufferView, BUFFER_VIEW_STATE, bufferViewMap)
    VALSTATETRACK_MAP_AND_TRAITS(VkBuffer, BUFFER_STATE, bufferMap)
    VALSTATETRACK_MAP_AND_TRAITS(VkPipeline, PIPELINE_STATE, pipelineMap)
    VALSTATETRACK_MAP_AND_TRAITS(VkDeviceMemory, DEVICE_MEMORY_STATE, memObjMap)
    VALSTATETRACK_MAP_AND_TRAITS(VkFramebuffer, FRAMEBUFFER_STATE, frameBufferMap)
    VALSTATETRACK_MAP_AND_TRAITS(VkShaderModule, SHADER_MODULE_STATE, shaderModuleMap)
    VALSTATETRACK_MAP_AND_TRAITS(VkDescriptorUpdateTemplateKHR, TEMPLATE_STATE, desc_template_map)
    VALSTATETRACK_MAP_AND_TRAITS(VkSwapchainKHR, SWAPCHAIN_NODE, swapchainMap)
    VALSTATETRACK_MAP_AND_TRAITS(VkDescriptorPool, DESCRIPTOR_POOL_STATE, descriptorPoolMap)
    VALSTATETRACK_MAP_AND_TRAITS(VkDescriptorSet, cvdescriptorset::DescriptorSet, setMap)
    VALSTATETRACK_MAP_AND_TRAITS(VkCommandBuffer, CMD_BUFFER_STATE, commandBufferMap)
    VALSTATETRACK_MAP_AND_TRAITS(VkCommandPool, COMMAND_POOL_STATE, commandPoolMap)
    VALSTATETRACK_MAP_AND_TRAITS(VkPipelineLayout, PIPELINE_LAYOUT_STATE, pipelineLayoutMap)
    VALSTATETRACK_MAP_AND_TRAITS(VkFence, FENCE_STATE, fenceMap)
    VALSTATETRACK_MAP_AND_TRAITS(VkQueryPool, QUERY_POOL_STATE, queryPoolMap)
    VALSTATETRACK_MAP_AND_TRAITS(VkSemaphore, SEMAPHORE_STATE, semaphoreMap)
    VALSTATETRACK_MAP_AND_TRAITS(VkSamplerYcbcrConversion, SAMPLER_YCBCR_CONVERSION_STATE, samplerYcbcrConversionMap)
    VALSTATETRACK_MAP_AND_TRAITS(VkAccelerationStructureNV, ACCELERATION_STRUCTURE_STATE, accelerationStructureMap)
    VALSTATETRACK_MAP_AND_TRAITS_INSTANCE_SCOPE(VkSurfaceKHR, SURFACE_STATE, surface_map)

    void AddAliasingImage(IMAGE_STATE* image_state);
    void RemoveAliasingImage(IMAGE_STATE* image_state);
    void RemoveAliasingImages(const std::unordered_set<VkImage>& bound_images);

  public:
    template <typename State>
    typename AccessorTraits<State>::ReturnType Get(typename AccessorTraits<State>::HandleType handle) {
        using Traits = AccessorTraits<State>;
        auto map_member = Traits::Map();
        const typename Traits::MapType& map =
            (Traits::kInstanceScope && (this->*map_member).size() == 0) ? instance_state->*map_member : this->*map_member;

        const auto found_it = map.find(handle);
        if (found_it == map.end()) {
            return nullptr;
        }
        return found_it->second.get();
    };

    template <typename State>
    const typename AccessorTraits<State>::ReturnType Get(typename AccessorTraits<State>::HandleType handle) const {
        using Traits = AccessorTraits<State>;
        auto map_member = Traits::Map();
        const typename Traits::MapType& map =
            (Traits::kInstanceScope && (this->*map_member).size() == 0) ? instance_state->*map_member : this->*map_member;

        const auto found_it = map.find(handle);
        if (found_it == map.cend()) {
            return nullptr;
        }
        return found_it->second.get();
    };

    template <typename State>
    typename AccessorTraits<State>::SharedType GetShared(typename AccessorTraits<State>::HandleType handle) {
        using Traits = AccessorTraits<State>;
        auto map_member = Traits::Map();
        const typename Traits::MapType& map =
            (Traits::kInstanceScope && (this->*map_member).size() == 0) ? instance_state->*map_member : this->*map_member;

        const auto found_it = map.find(handle);
        if (found_it == map.end()) {
            return nullptr;
        }
        return found_it->second;
    };

    template <typename State>
    typename AccessorTraits<State>::ConstSharedType GetShared(typename AccessorTraits<State>::HandleType handle) const {
        using Traits = AccessorTraits<State>;
        auto map_member = Traits::Map();
        const typename Traits::MapType& map =
            (Traits::kInstanceScope && (this->*map_member).size() == 0) ? instance_state->*map_member : this->*map_member;

        const auto found_it = map.find(handle);
        if (found_it == map.cend()) {
            return nullptr;
        }
        return found_it->second;
    };

    // When needing to share ownership, control over constness of access with another object (i.e. adding references while
    // not modifying the contents of the ValidationStateTracker)
    template <typename State>
    typename AccessorTraits<State>::SharedType GetConstCastShared(typename AccessorTraits<State>::HandleType handle) const {
        using Traits = AccessorTraits<State>;
        auto map_member = Traits::Map();
        const typename Traits::MapType& map =
            (Traits::kInstanceScope && (this->*map_member).size() == 0) ? instance_state->*map_member : this->*map_member;

        const auto found_it = map.find(handle);
        if (found_it == map.cend()) {
            return nullptr;
        }
        return found_it->second;
    };
    // Accessors for the VALSTATE... maps
    std::shared_ptr<const cvdescriptorset::DescriptorSetLayout> GetDescriptorSetLayoutShared(VkDescriptorSetLayout dsLayout) const {
        return GetShared<cvdescriptorset::DescriptorSetLayout>(dsLayout);
    }
    std::shared_ptr<cvdescriptorset::DescriptorSetLayout> GetDescriptorSetLayoutShared(VkDescriptorSetLayout dsLayout) {
        return GetShared<cvdescriptorset::DescriptorSetLayout>(dsLayout);
    }

    std::shared_ptr<const RENDER_PASS_STATE> GetRenderPassShared(VkRenderPass renderpass) const {
        return GetShared<RENDER_PASS_STATE>(renderpass);
    }
    std::shared_ptr<RENDER_PASS_STATE> GetRenderPassShared(VkRenderPass renderpass) {
        return GetShared<RENDER_PASS_STATE>(renderpass);
    }
    const RENDER_PASS_STATE* GetRenderPassState(VkRenderPass renderpass) const { return Get<RENDER_PASS_STATE>(renderpass); }
    RENDER_PASS_STATE* GetRenderPassState(VkRenderPass renderpass) { return Get<RENDER_PASS_STATE>(renderpass); }

    std::shared_ptr<const SAMPLER_STATE> GetSamplerShared(VkSampler sampler) const { return GetShared<SAMPLER_STATE>(sampler); }
    std::shared_ptr<SAMPLER_STATE> GetSamplerShared(VkSampler sampler) { return GetShared<SAMPLER_STATE>(sampler); }
    const SAMPLER_STATE* GetSamplerState(VkSampler sampler) const { return Get<SAMPLER_STATE>(sampler); }
    SAMPLER_STATE* GetSamplerState(VkSampler sampler) { return Get<SAMPLER_STATE>(sampler); }

    std::shared_ptr<const IMAGE_VIEW_STATE> GetImageViewShared(VkImageView image_view) const {
        return GetShared<IMAGE_VIEW_STATE>(image_view);
    }
    std::shared_ptr<IMAGE_VIEW_STATE> GetImageViewShared(VkImageView image_view) { return GetShared<IMAGE_VIEW_STATE>(image_view); }
    const IMAGE_VIEW_STATE* GetImageViewState(VkImageView image_view) const { return Get<IMAGE_VIEW_STATE>(image_view); }
    IMAGE_VIEW_STATE* GetImageViewState(VkImageView image_view) { return Get<IMAGE_VIEW_STATE>(image_view); }

    std::shared_ptr<const IMAGE_STATE> GetImageShared(VkImage image) const { return GetShared<IMAGE_STATE>(image); }
    std::shared_ptr<IMAGE_STATE> GetImageShared(VkImage image) { return GetShared<IMAGE_STATE>(image); }
    const IMAGE_STATE* GetImageState(VkImage image) const { return Get<IMAGE_STATE>(image); }
    IMAGE_STATE* GetImageState(VkImage image) { return Get<IMAGE_STATE>(image); }

    std::shared_ptr<const BUFFER_VIEW_STATE> GetBufferViewShared(VkBufferView buffer_view) const {
        return GetShared<BUFFER_VIEW_STATE>(buffer_view);
    }
    std::shared_ptr<BUFFER_VIEW_STATE> GetBufferViewShared(VkBufferView buffer_view) {
        return GetShared<BUFFER_VIEW_STATE>(buffer_view);
    }
    const BUFFER_VIEW_STATE* GetBufferViewState(VkBufferView buffer_view) const { return Get<BUFFER_VIEW_STATE>(buffer_view); }
    BUFFER_VIEW_STATE* GetBufferViewState(VkBufferView buffer_view) { return Get<BUFFER_VIEW_STATE>(buffer_view); }

    std::shared_ptr<const BUFFER_STATE> GetBufferShared(VkBuffer buffer) const { return GetShared<BUFFER_STATE>(buffer); }
    std::shared_ptr<BUFFER_STATE> GetBufferShared(VkBuffer buffer) { return GetShared<BUFFER_STATE>(buffer); }
    const BUFFER_STATE* GetBufferState(VkBuffer buffer) const { return Get<BUFFER_STATE>(buffer); }
    BUFFER_STATE* GetBufferState(VkBuffer buffer) { return Get<BUFFER_STATE>(buffer); }

    const PIPELINE_STATE* GetPipelineState(VkPipeline pipeline) const { return Get<PIPELINE_STATE>(pipeline); }
    PIPELINE_STATE* GetPipelineState(VkPipeline pipeline) { return Get<PIPELINE_STATE>(pipeline); }
    const DEVICE_MEMORY_STATE* GetDevMemState(VkDeviceMemory mem) const { return Get<DEVICE_MEMORY_STATE>(mem); }
    DEVICE_MEMORY_STATE* GetDevMemState(VkDeviceMemory mem) { return Get<DEVICE_MEMORY_STATE>(mem); }
    const FRAMEBUFFER_STATE* GetFramebufferState(VkFramebuffer framebuffer) const { return Get<FRAMEBUFFER_STATE>(framebuffer); }
    FRAMEBUFFER_STATE* GetFramebufferState(VkFramebuffer framebuffer) { return Get<FRAMEBUFFER_STATE>(framebuffer); }
    const SHADER_MODULE_STATE* GetShaderModuleState(VkShaderModule module) const { return Get<SHADER_MODULE_STATE>(module); }
    SHADER_MODULE_STATE* GetShaderModuleState(VkShaderModule module) { return Get<SHADER_MODULE_STATE>(module); }
    const TEMPLATE_STATE* GetDescriptorTemplateState(VkDescriptorUpdateTemplateKHR descriptor_update_template) const {
        return Get<TEMPLATE_STATE>(descriptor_update_template);
    }
    TEMPLATE_STATE* GetDescriptorTemplateState(VkDescriptorUpdateTemplateKHR descriptor_update_template) {
        return Get<TEMPLATE_STATE>(descriptor_update_template);
    }
    const SWAPCHAIN_NODE* GetSwapchainState(VkSwapchainKHR swapchain) const { return Get<SWAPCHAIN_NODE>(swapchain); }
    SWAPCHAIN_NODE* GetSwapchainState(VkSwapchainKHR swapchain) { return Get<SWAPCHAIN_NODE>(swapchain); }
    const DESCRIPTOR_POOL_STATE* GetDescriptorPoolState(const VkDescriptorPool pool) const {
        return Get<DESCRIPTOR_POOL_STATE>(pool);
    }
    DESCRIPTOR_POOL_STATE* GetDescriptorPoolState(const VkDescriptorPool pool) { return Get<DESCRIPTOR_POOL_STATE>(pool); }
    const cvdescriptorset::DescriptorSet* GetSetNode(VkDescriptorSet set) const { return Get<cvdescriptorset::DescriptorSet>(set); }
    cvdescriptorset::DescriptorSet* GetSetNode(VkDescriptorSet set) { return Get<cvdescriptorset::DescriptorSet>(set); }
    const CMD_BUFFER_STATE* GetCBState(const VkCommandBuffer cb) const { return Get<CMD_BUFFER_STATE>(cb); }
    CMD_BUFFER_STATE* GetCBState(const VkCommandBuffer cb) { return Get<CMD_BUFFER_STATE>(cb); }

    std::shared_ptr<const COMMAND_POOL_STATE> GetCommandPoolShared(VkCommandPool pool) const {
        return GetShared<COMMAND_POOL_STATE>(pool);
    }
    std::shared_ptr<COMMAND_POOL_STATE> GetCommandPoolShared(VkCommandPool pool) { return GetShared<COMMAND_POOL_STATE>(pool); }
    const COMMAND_POOL_STATE* GetCommandPoolState(VkCommandPool pool) const { return Get<COMMAND_POOL_STATE>(pool); }
    COMMAND_POOL_STATE* GetCommandPoolState(VkCommandPool pool) { return Get<COMMAND_POOL_STATE>(pool); }

    std::shared_ptr<const PIPELINE_LAYOUT_STATE> GetPipelineLayoutShared(VkPipelineLayout pipeLayout) const {
        return GetShared<PIPELINE_LAYOUT_STATE>(pipeLayout);
    }
    std::shared_ptr<PIPELINE_LAYOUT_STATE> GetPipelineLayoutShared(VkPipelineLayout pipeLayout) {
        return GetShared<PIPELINE_LAYOUT_STATE>(pipeLayout);
    }
    const PIPELINE_LAYOUT_STATE* GetPipelineLayout(VkPipelineLayout pipeLayout) const {
        return Get<PIPELINE_LAYOUT_STATE>(pipeLayout);
    }
    PIPELINE_LAYOUT_STATE* GetPipelineLayout(VkPipelineLayout pipeLayout) { return Get<PIPELINE_LAYOUT_STATE>(pipeLayout); }

    const FENCE_STATE* GetFenceState(VkFence fence) const { return Get<FENCE_STATE>(fence); }
    FENCE_STATE* GetFenceState(VkFence fence) { return Get<FENCE_STATE>(fence); }
    const QUERY_POOL_STATE* GetQueryPoolState(VkQueryPool query_pool) const { return Get<QUERY_POOL_STATE>(query_pool); }
    QUERY_POOL_STATE* GetQueryPoolState(VkQueryPool query_pool) { return Get<QUERY_POOL_STATE>(query_pool); }
    const SEMAPHORE_STATE* GetSemaphoreState(VkSemaphore semaphore) const { return Get<SEMAPHORE_STATE>(semaphore); }
    SEMAPHORE_STATE* GetSemaphoreState(VkSemaphore semaphore) { return Get<SEMAPHORE_STATE>(semaphore); }
    const SAMPLER_YCBCR_CONVERSION_STATE* GetSamplerYcbcrConversionState(VkSamplerYcbcrConversion samplerYcbcrConversion) const {
        return Get<SAMPLER_YCBCR_CONVERSION_STATE>(samplerYcbcrConversion);
    }
    SAMPLER_YCBCR_CONVERSION_STATE* GetSamplerYcbcrConversionState(VkSamplerYcbcrConversion samplerYcbcrConversion) {
        return Get<SAMPLER_YCBCR_CONVERSION_STATE>(samplerYcbcrConversion);
    }
    const ACCELERATION_STRUCTURE_STATE* GetAccelerationStructureState(VkAccelerationStructureNV as) const {
        return Get<ACCELERATION_STRUCTURE_STATE>(as);
    }
    ACCELERATION_STRUCTURE_STATE* GetAccelerationStructureState(VkAccelerationStructureNV as) {
        return Get<ACCELERATION_STRUCTURE_STATE>(as);
    }
    const SURFACE_STATE* GetSurfaceState(VkSurfaceKHR surface) const { return Get<SURFACE_STATE>(surface); }
    SURFACE_STATE* GetSurfaceState(VkSurfaceKHR surface) { return Get<SURFACE_STATE>(surface); }

    // Class Declarations for helper functions
    IMAGE_VIEW_STATE* GetAttachmentImageViewState(CMD_BUFFER_STATE* cb, FRAMEBUFFER_STATE* framebuffer, uint32_t index);
    const IMAGE_VIEW_STATE* GetAttachmentImageViewState(const CMD_BUFFER_STATE* cb, const FRAMEBUFFER_STATE* framebuffer,
                                                        uint32_t index) const;
    const EVENT_STATE* GetEventState(VkEvent event) const;
    EVENT_STATE* GetEventState(VkEvent event);
    const QUEUE_STATE* GetQueueState(VkQueue queue) const;
    QUEUE_STATE* GetQueueState(VkQueue queue);
    const BINDABLE* GetObjectMemBinding(const VulkanTypedHandle& typed_handle) const;
    BINDABLE* GetObjectMemBinding(const VulkanTypedHandle& typed_handle);

    // Used for instance versions of this object
    unordered_map<VkPhysicalDevice, PHYSICAL_DEVICE_STATE> physical_device_map;
    // Link to the device's physical-device data
    PHYSICAL_DEVICE_STATE* physical_device_state;

    // Link for derived device objects back to their parent instance object
    ValidationStateTracker* instance_state;

    const PHYSICAL_DEVICE_STATE* GetPhysicalDeviceState(VkPhysicalDevice phys) const;
    PHYSICAL_DEVICE_STATE* GetPhysicalDeviceState(VkPhysicalDevice phys);
    PHYSICAL_DEVICE_STATE* GetPhysicalDeviceState();
    const PHYSICAL_DEVICE_STATE* GetPhysicalDeviceState() const;

    VkQueueFlags GetQueueFlags(const COMMAND_POOL_STATE& cp_state) const {
        return GetPhysicalDeviceState()->queue_family_properties[cp_state.queueFamilyIndex].queueFlags;
    }

    VkQueueFlags GetQueueFlags(const CMD_BUFFER_STATE& cb_state) const {
        VkQueueFlags queue_flags = 0;
        auto pool = cb_state.command_pool.get();
        if (pool) {
            queue_flags = GetQueueFlags(*pool);
        }
        return queue_flags;
    }

    using CommandBufferResetCallback = std::function<void(VkCommandBuffer)>;
    std::unique_ptr<CommandBufferResetCallback> command_buffer_reset_callback;
    template <typename Fn>
    void SetCommandBufferResetCallback(Fn&& fn) {
        command_buffer_reset_callback.reset(new CommandBufferResetCallback(std::forward<Fn>(fn)));
    }

    using CommandBufferFreeCallback = std::function<void(VkCommandBuffer)>;
    std::unique_ptr<CommandBufferFreeCallback> command_buffer_free_callback;
    template <typename Fn>
    void SetCommandBufferFreeCallback(Fn&& fn) {
        command_buffer_free_callback.reset(new CommandBufferFreeCallback(std::forward<Fn>(fn)));
    }

    using SetImageViewInitialLayoutCallback = std::function<void(CMD_BUFFER_STATE*, const IMAGE_VIEW_STATE&, VkImageLayout)>;
    std::unique_ptr<SetImageViewInitialLayoutCallback> set_image_view_initial_layout_callback;
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
    void PostCallRecordEnumeratePhysicalDeviceGroups(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount,
                                                     VkPhysicalDeviceGroupPropertiesKHR* pPhysicalDeviceGroupProperties,
                                                     VkResult result);
    void PostCallRecordEnumeratePhysicalDeviceGroupsKHR(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount,
                                                        VkPhysicalDeviceGroupPropertiesKHR* pPhysicalDeviceGroupProperties,
                                                        VkResult result);
    void PostCallRecordEnumeratePhysicalDevices(VkInstance instance, uint32_t* pPhysicalDeviceCount,
                                                VkPhysicalDevice* pPhysicalDevices, VkResult result);
    void PostCallRecordEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(
        VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, uint32_t* pCounterCount, VkPerformanceCounterKHR* pCounters,
        VkPerformanceCounterDescriptionKHR* pCounterDescriptions, VkResult result);
    void PostCallRecordGetAccelerationStructureMemoryRequirementsNV(VkDevice device,
                                                                    const VkAccelerationStructureMemoryRequirementsInfoNV* pInfo,
                                                                    VkMemoryRequirements2KHR* pMemoryRequirements);
    void PostCallRecordGetBufferMemoryRequirements(VkDevice device, VkBuffer buffer, VkMemoryRequirements* pMemoryRequirements);
    void PostCallRecordGetBufferMemoryRequirements2(VkDevice device, const VkBufferMemoryRequirementsInfo2KHR* pInfo,
                                                    VkMemoryRequirements2KHR* pMemoryRequirements);
    void PostCallRecordGetBufferMemoryRequirements2KHR(VkDevice device, const VkBufferMemoryRequirementsInfo2KHR* pInfo,
                                                       VkMemoryRequirements2KHR* pMemoryRequirements);
    void PostCallRecordGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue);
    void PostCallRecordGetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2* pQueueInfo, VkQueue* pQueue);
    void PostCallRecordGetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR* pGetFdInfo, int* pFd, VkResult result);
    void PostCallRecordGetFenceStatus(VkDevice device, VkFence fence, VkResult result);
#ifdef VK_USE_PLATFORM_WIN32_KHR
    void PostCallRecordGetFenceWin32HandleKHR(VkDevice device, const VkFenceGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                              HANDLE* pHandle, VkResult result);
#endif  // VK_USE_PLATFORM_WIN32_KHR
    void PostCallRecordGetImageMemoryRequirements(VkDevice device, VkImage image, VkMemoryRequirements* pMemoryRequirements);
    void PostCallRecordGetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
                                                   VkMemoryRequirements2* pMemoryRequirements);
    void PostCallRecordGetImageMemoryRequirements2KHR(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
                                                      VkMemoryRequirements2* pMemoryRequirements);
    void PostCallRecordGetImageSparseMemoryRequirements(VkDevice device, VkImage image, uint32_t* pSparseMemoryRequirementCount,
                                                        VkSparseImageMemoryRequirements* pSparseMemoryRequirements);
    void PostCallRecordGetImageSparseMemoryRequirements2(VkDevice device, const VkImageSparseMemoryRequirementsInfo2KHR* pInfo,
                                                         uint32_t* pSparseMemoryRequirementCount,
                                                         VkSparseImageMemoryRequirements2KHR* pSparseMemoryRequirements);
    void PostCallRecordGetImageSparseMemoryRequirements2KHR(VkDevice device, const VkImageSparseMemoryRequirementsInfo2KHR* pInfo,
                                                            uint32_t* pSparseMemoryRequirementCount,
                                                            VkSparseImageMemoryRequirements2KHR* pSparseMemoryRequirements);
    void PostCallRecordGetPhysicalDeviceDisplayPlanePropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                                  VkDisplayPlanePropertiesKHR* pProperties, VkResult result);
    void PostCallRecordGetPhysicalDeviceDisplayPlaneProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                                   VkDisplayPlaneProperties2KHR* pProperties, VkResult result);
    void PostCallRecordGetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures* pFeatures);
    void PostCallRecordGetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures);
    void PostCallRecordGetPhysicalDeviceFeatures2KHR(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures);
    void PostCallRecordGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount,
                                                              VkQueueFamilyProperties* pQueueFamilyProperties);
    void PostCallRecordGetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount,
                                                               VkQueueFamilyProperties2KHR* pQueueFamilyProperties);
    void PostCallRecordGetPhysicalDeviceQueueFamilyProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                  uint32_t* pQueueFamilyPropertyCount,
                                                                  VkQueueFamilyProperties2KHR* pQueueFamilyProperties);
    void PostCallRecordGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                               VkSurfaceCapabilitiesKHR* pSurfaceCapabilities, VkResult result);
    void PostCallRecordGetPhysicalDeviceSurfaceCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                                const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                                VkSurfaceCapabilities2KHR* pSurfaceCapabilities, VkResult result);
    void PostCallRecordGetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                VkSurfaceCapabilities2EXT* pSurfaceCapabilities, VkResult result);
    void PostCallRecordGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                          uint32_t* pSurfaceFormatCount, VkSurfaceFormatKHR* pSurfaceFormats,
                                                          VkResult result);
    void PostCallRecordGetPhysicalDeviceSurfaceFormats2KHR(VkPhysicalDevice physicalDevice,
                                                           const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                           uint32_t* pSurfaceFormatCount, VkSurfaceFormat2KHR* pSurfaceFormats,
                                                           VkResult result);
    void PostCallRecordGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                               uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes,
                                                               VkResult result);
    void PostCallRecordGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                          VkSurfaceKHR surface, VkBool32* pSupported, VkResult result);
    void PostCallRecordGetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR* pGetFdInfo, int* pFd, VkResult result);
#ifdef VK_USE_PLATFORM_WIN32_KHR
    void PostCallRecordGetSemaphoreWin32HandleKHR(VkDevice device, const VkSemaphoreGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                                  HANDLE* pHandle, VkResult result);
#endif  // VK_USE_PLATFORM_WIN32_KHR
    void PostCallRecordGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount,
                                             VkImage* pSwapchainImages, VkResult result);
    void PostCallRecordImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR* pImportFenceFdInfo, VkResult result);
#ifdef VK_USE_PLATFORM_WIN32_KHR
    void PostCallRecordImportFenceWin32HandleKHR(VkDevice device,
                                                 const VkImportFenceWin32HandleInfoKHR* pImportFenceWin32HandleInfo,
                                                 VkResult result);
#endif  // VK_USE_PLATFORM_WIN32_KHR
    void PostCallRecordImportSemaphoreFdKHR(VkDevice device, const VkImportSemaphoreFdInfoKHR* pImportSemaphoreFdInfo,
                                            VkResult result);
#ifdef VK_USE_PLATFORM_WIN32_KHR
    void PostCallRecordImportSemaphoreWin32HandleKHR(VkDevice device,
                                                     const VkImportSemaphoreWin32HandleInfoKHR* pImportSemaphoreWin32HandleInfo,
                                                     VkResult result);
#endif  // VK_USE_PLATFORM_WIN32_KHR
    void PostCallRecordSignalSemaphoreKHR(VkDevice device, const VkSemaphoreSignalInfoKHR* pSignalInfo, VkResult result);

    // Create/Destroy/Bind
    void PostCallRecordBindAccelerationStructureMemoryCommon(VkDevice device, uint32_t bindInfoCount,
                                                             const VkBindAccelerationStructureMemoryInfoKHR* pBindInfos,
                                                             VkResult result, bool isNV);
    void PostCallRecordBindAccelerationStructureMemoryNV(VkDevice device, uint32_t bindInfoCount,
                                                         const VkBindAccelerationStructureMemoryInfoNV* pBindInfos,
                                                         VkResult result);
    void PostCallRecordBindAccelerationStructureMemoryKHR(VkDevice device, uint32_t bindInfoCount,
                                                          const VkBindAccelerationStructureMemoryInfoKHR* pBindInfos,
                                                          VkResult result);
    void PostCallRecordBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory mem, VkDeviceSize memoryOffset,
                                        VkResult result);
    void PostCallRecordBindBufferMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfoKHR* pBindInfos,
                                         VkResult result);
    void PostCallRecordBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfoKHR* pBindInfos,
                                            VkResult result);
    void PostCallRecordBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory mem, VkDeviceSize memoryOffset,
                                       VkResult result);
    void PostCallRecordBindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfoKHR* pBindInfos,
                                        VkResult result);
    void PostCallRecordBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfoKHR* pBindInfos,
                                           VkResult result);

    void PostCallRecordCreateDevice(VkPhysicalDevice gpu, const VkDeviceCreateInfo* pCreateInfo,
                                    const VkAllocationCallbacks* pAllocator, VkDevice* pDevice, VkResult result);
    void PreCallRecordDestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator);

    void PostCallRecordCreateAccelerationStructureNV(VkDevice device, const VkAccelerationStructureCreateInfoNV* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator,
                                                     VkAccelerationStructureNV* pAccelerationStructure, VkResult result);
    void PreCallRecordDestroyAccelerationStructureNV(VkDevice device, VkAccelerationStructureNV accelerationStructure,
                                                     const VkAllocationCallbacks* pAllocator);

    void PostCallRecordCreateAccelerationStructureKHR(VkDevice device, const VkAccelerationStructureCreateInfoKHR* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator,
                                                      VkAccelerationStructureKHR* pAccelerationStructure, VkResult result);
    void PreCallRecordDestroyAccelerationStructureKHR(VkDevice device, VkAccelerationStructureKHR accelerationStructure,
                                                      const VkAllocationCallbacks* pAllocator);

    void PostCallRecordCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                    VkBuffer* pBuffer, VkResult result);
    void PreCallRecordDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator);
    void PostCallRecordCreateBufferView(VkDevice device, const VkBufferViewCreateInfo* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkBufferView* pView, VkResult result);
    void PreCallRecordDestroyBufferView(VkDevice device, VkBufferView bufferView, const VkAllocationCallbacks* pAllocator);
    void PostCallRecordCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool, VkResult result);
    void PreCallRecordDestroyCommandPool(VkDevice device, VkCommandPool commandPool, const VkAllocationCallbacks* pAllocator);
    void PostCallRecordCreateDisplayPlaneSurfaceKHR(VkInstance instance, const VkDisplaySurfaceCreateInfoKHR* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                    VkResult result);
    void PostCallRecordCreateEvent(VkDevice device, const VkEventCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                   VkEvent* pEvent, VkResult result);
    void PreCallRecordDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks* pAllocator);
    void PostCallRecordCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkDescriptorPool* pDescriptorPool,
                                            VkResult result);
    void PreCallRecordDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                            const VkAllocationCallbacks* pAllocator);
    void PostCallRecordCreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkDescriptorSetLayout* pSetLayout,
                                                 VkResult result);
    void PostCallRecordResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags, VkResult result);
    void PostCallRecordResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags, VkResult result);
    bool PreCallValidateCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                               const VkComputePipelineCreateInfo* pCreateInfos,
                                               const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                               void* pipe_state) const;
    void PostCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                              const VkComputePipelineCreateInfo* pCreateInfos,
                                              const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, VkResult result,
                                              void* pipe_state);
    void PostCallRecordResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorPoolResetFlags flags,
                                           VkResult result);
    bool PreCallValidateAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo,
                                               VkDescriptorSet* pDescriptorSets, void* ads_state_data) const;
    void PreCallRecordDestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout,
                                                 const VkAllocationCallbacks* pAllocator);
    void PostCallRecordCreateDescriptorUpdateTemplate(VkDevice device, const VkDescriptorUpdateTemplateCreateInfoKHR* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator,
                                                      VkDescriptorUpdateTemplateKHR* pDescriptorUpdateTemplate, VkResult result);
    void PostCallRecordCreateDescriptorUpdateTemplateKHR(VkDevice device,
                                                         const VkDescriptorUpdateTemplateCreateInfoKHR* pCreateInfo,
                                                         const VkAllocationCallbacks* pAllocator,
                                                         VkDescriptorUpdateTemplateKHR* pDescriptorUpdateTemplate, VkResult result);
    void PreCallRecordDestroyDescriptorUpdateTemplate(VkDevice device, VkDescriptorUpdateTemplateKHR descriptorUpdateTemplate,
                                                      const VkAllocationCallbacks* pAllocator);
    void PreCallRecordDestroyDescriptorUpdateTemplateKHR(VkDevice device, VkDescriptorUpdateTemplateKHR descriptorUpdateTemplate,
                                                         const VkAllocationCallbacks* pAllocator);
    void PostCallRecordCreateFence(VkDevice device, const VkFenceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                   VkFence* pFence, VkResult result);
    void PreCallRecordDestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks* pAllocator);
    void PostCallRecordResetFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkResult result);
    void PostCallRecordCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer, VkResult result);
    void PreCallRecordDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer, const VkAllocationCallbacks* pAllocator);
    bool PreCallValidateCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                                const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                void* cgpl_state) const;
    void PostCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                               const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                               const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, VkResult result,
                                               void* cgpl_state);
    void PostCallRecordCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                   VkImage* pImage, VkResult result);
    void PreCallRecordDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator);
    void PostCallRecordCreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkImageView* pView, VkResult result);
    void PreCallRecordDestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks* pAllocator);

    void PreCallRecordDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks* pAllocator);
    void PostCallRecordCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout,
                                            VkResult result);
    void PreCallRecordDestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout,
                                            const VkAllocationCallbacks* pAllocator);
    void PostCallRecordCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo* pCreateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkQueryPool* pQueryPool, VkResult result);
    void PreCallRecordDestroyQueryPool(VkDevice device, VkQueryPool queryPool, const VkAllocationCallbacks* pAllocator);
    void RecordResetQueryPool(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount);
    void PostCallRecordResetQueryPoolEXT(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount);
    void PostCallRecordResetQueryPool(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount);
    bool PreCallValidateCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                    const VkRayTracingPipelineCreateInfoNV* pCreateInfos,
                                                    const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                    void* pipe_state) const;
    void PostCallRecordCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                   const VkRayTracingPipelineCreateInfoNV* pCreateInfos,
                                                   const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, VkResult result,
                                                   void* pipe_state);
    bool PreCallValidateCreateRayTracingPipelinesKHR(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                     const VkRayTracingPipelineCreateInfoKHR* pCreateInfos,
                                                     const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                     void* pipe_state) const;
    void PostCallRecordCreateRayTracingPipelinesKHR(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                    const VkRayTracingPipelineCreateInfoKHR* pCreateInfos,
                                                    const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                    VkResult result, void* pipe_state);
    void PostCallRecordCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass, VkResult result);
    void RecordCreateRenderPass2(VkDevice device, const VkRenderPassCreateInfo2KHR* pCreateInfo,
                                 const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass, VkResult result);
    void PostCallRecordCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2KHR* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass, VkResult result);
    void PostCallRecordCreateRenderPass2(VkDevice device, const VkRenderPassCreateInfo2KHR* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass, VkResult result);
    void PreCallRecordDestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks* pAllocator);
    void PostCallRecordCreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo,
                                     const VkAllocationCallbacks* pAllocator, VkSampler* pSampler, VkResult result);
    void PreCallRecordDestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks* pAllocator);
    void PostCallRecordCreateSamplerYcbcrConversion(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator,
                                                    VkSamplerYcbcrConversion* pYcbcrConversion, VkResult result);
    void PostCallRecordDestroySamplerYcbcrConversion(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                                     const VkAllocationCallbacks* pAllocator);
    void PostCallRecordCreateSamplerYcbcrConversionKHR(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                       const VkAllocationCallbacks* pAllocator,
                                                       VkSamplerYcbcrConversion* pYcbcrConversion, VkResult result);
    void PostCallRecordDestroySamplerYcbcrConversionKHR(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                                        const VkAllocationCallbacks* pAllocator);
    void PostCallRecordCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore, VkResult result);
    void PreCallRecordDestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks* pAllocator);
    void PostCallRecordCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule, VkResult result,
                                          void* csm_state);
    void PreCallRecordDestroyShaderModule(VkDevice device, VkShaderModule shaderModule, const VkAllocationCallbacks* pAllocator);
    void PreCallRecordDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks* pAllocator);
    void PostCallRecordCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount,
                                                 const VkSwapchainCreateInfoKHR* pCreateInfos,
                                                 const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchains,
                                                 VkResult result);
    void PostCallRecordCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain, VkResult result);
    void PreCallRecordDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks* pAllocator);

    // CommandBuffer/Queue Control
    void PreCallRecordBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo);
    void PostCallRecordDeviceWaitIdle(VkDevice device, VkResult result);
    void PostCallRecordEndCommandBuffer(VkCommandBuffer commandBuffer, VkResult result);
    void PostCallRecordQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo, VkFence fence,
                                       VkResult result);
    void PostCallRecordQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo, VkResult result);
    void PostCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence,
                                   VkResult result);
    void PostCallRecordQueueWaitIdle(VkQueue queue, VkResult result);
    void PreCallRecordSetEvent(VkDevice device, VkEvent event);
    void PostCallRecordWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll,
                                     uint64_t timeout, VkResult result);
    void PostCallRecordWaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout, VkResult result);
    void PostCallRecordWaitSemaphoresKHR(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout, VkResult result);
    void PostCallRecordAcquireProfilingLockKHR(VkDevice device, const VkAcquireProfilingLockInfoKHR* pInfo, VkResult result);
    void PostCallRecordReleaseProfilingLockKHR(VkDevice device);

    // Allocate/Free
    void PostCallRecordAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* pCreateInfo,
                                              VkCommandBuffer* pCommandBuffer, VkResult result);
    void PostCallRecordAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo,
                                              VkDescriptorSet* pDescriptorSets, VkResult result, void* ads_state);
    void PostCallRecordAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo,
                                      const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory, VkResult result);
    void PreCallRecordFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount,
                                         const VkCommandBuffer* pCommandBuffers);
    void PreCallRecordFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t count,
                                         const VkDescriptorSet* pDescriptorSets);
    void PreCallRecordFreeMemory(VkDevice device, VkDeviceMemory mem, const VkAllocationCallbacks* pAllocator);
    void PreCallRecordUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount,
                                           const VkWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount,
                                           const VkCopyDescriptorSet* pDescriptorCopies);
    void PreCallRecordUpdateDescriptorSetWithTemplate(VkDevice device, VkDescriptorSet descriptorSet,
                                                      VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void* pData);
    void PreCallRecordUpdateDescriptorSetWithTemplateKHR(VkDevice device, VkDescriptorSet descriptorSet,
                                                         VkDescriptorUpdateTemplateKHR descriptorUpdateTemplate, const void* pData);

    // Memory mapping
    void PostCallRecordMapMemory(VkDevice device, VkDeviceMemory mem, VkDeviceSize offset, VkDeviceSize size, VkFlags flags,
                                 void** ppData, VkResult result);
    void PreCallRecordUnmapMemory(VkDevice device, VkDeviceMemory mem);

    // Recorded Commands
    void PreCallRecordCmdBeginDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo);
    void PostCallRecordCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot, VkFlags flags);
    void PostCallRecordCmdBeginQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                               VkQueryControlFlags flags, uint32_t index);
    void PreCallRecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                         VkSubpassContents contents);
    void PreCallRecordCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                          const VkSubpassBeginInfoKHR* pSubpassBeginInfo);
    void PreCallRecordCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                             const VkSubpassBeginInfoKHR* pSubpassBeginInfo);
    void PostCallRecordCmdBeginTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                                    uint32_t counterBufferCount, const VkBuffer* pCounterBuffers,
                                                    const VkDeviceSize* pCounterBufferOffsets);
    void PostCallRecordCmdEndTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                                  uint32_t counterBufferCount, const VkBuffer* pCounterBuffers,
                                                  const VkDeviceSize* pCounterBufferOffsets);
    void PreCallRecordCmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                            VkPipelineLayout layout, uint32_t firstSet, uint32_t setCount,
                                            const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount,
                                            const uint32_t* pDynamicOffsets);
    void PreCallRecordCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                         VkIndexType indexType);
    void PreCallRecordCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline);
    void PreCallRecordCmdBindShadingRateImageNV(VkCommandBuffer commandBuffer, VkImageView imageView, VkImageLayout imageLayout);
    void PreCallRecordCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                           const VkBuffer* pBuffers, const VkDeviceSize* pOffsets);
    void PreCallRecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                   VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit* pRegions,
                                   VkFilter filter);
    void PostCallRecordCmdBuildAccelerationStructureNV(VkCommandBuffer commandBuffer, const VkAccelerationStructureInfoNV* pInfo,
                                                       VkBuffer instanceData, VkDeviceSize instanceOffset, VkBool32 update,
                                                       VkAccelerationStructureNV dst, VkAccelerationStructureNV src,
                                                       VkBuffer scratch, VkDeviceSize scratchOffset);
    void PreCallRecordCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                         const VkClearColorValue* pColor, uint32_t rangeCount,
                                         const VkImageSubresourceRange* pRanges);
    void PreCallRecordCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount,
                                                const VkImageSubresourceRange* pRanges);
    void PostCallRecordCmdCopyAccelerationStructureNV(VkCommandBuffer commandBuffer, VkAccelerationStructureNV dst,
                                                      VkAccelerationStructureNV src, VkCopyAccelerationStructureModeNV mode);
    void PreCallRecordCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount,
                                    const VkBufferCopy* pRegions);
    void PreCallRecordCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                           VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy* pRegions);
    void PreCallRecordCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                   VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy* pRegions);
    void PreCallRecordCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                           VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions);
    void PostCallRecordCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                               uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride,
                                               VkQueryResultFlags flags);
    void PostCallRecordCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z);
    void PostCallRecordCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset);
    void PostCallRecordCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                               uint32_t firstInstance);
    void PostCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                      uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance);
    void PostCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count,
                                              uint32_t stride);
    void PostCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count,
                                       uint32_t stride);
    void RecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                           VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                           uint32_t stride, const char* function);
    void PreCallRecordCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                     VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                     uint32_t stride);
    void PreCallRecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                  VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                  uint32_t stride);
    void RecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer,
                                    VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride, const char* function);
    void PreCallRecordCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                              VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                              uint32_t stride);
    void PreCallRecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                           VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                           uint32_t stride);
    void PreCallRecordCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                      VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                      uint32_t stride);
    void PreCallRecordCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                 uint32_t drawCount, uint32_t stride);
    void PreCallRecordCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask);
    void PostCallRecordCmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer);
    void PostCallRecordCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot);
    void PostCallRecordCmdEndQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, uint32_t index);
    void PostCallRecordCmdEndRenderPass(VkCommandBuffer commandBuffer);
    void PostCallRecordCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfoKHR* pSubpassEndInfo);
    void PostCallRecordCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfoKHR* pSubpassEndInfo);
    void PreCallRecordCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBuffersCount,
                                         const VkCommandBuffer* pCommandBuffers);
    void PreCallRecordCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size,
                                    uint32_t data);
    void PreCallRecordCmdInsertDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo);
    void PostCallRecordCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents);
    void PostCallRecordCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfoKHR* pSubpassBeginInfo,
                                          const VkSubpassEndInfoKHR* pSubpassEndInfo);
    void PostCallRecordCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfoKHR* pSubpassBeginInfo,
                                       const VkSubpassEndInfoKHR* pSubpassEndInfo);
    void PreCallRecordCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                              VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                              const VkWriteDescriptorSet* pDescriptorWrites);
    void PreCallRecordCmdPushDescriptorSetWithTemplateKHR(VkCommandBuffer commandBuffer,
                                                          VkDescriptorUpdateTemplateKHR descriptorUpdateTemplate,
                                                          VkPipelineLayout layout, uint32_t set, const void* pData);
    void PostCallRecordCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlags stageFlags,
                                        uint32_t offset, uint32_t size, const void* pValues);
    void PreCallRecordCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask);
    void PostCallRecordCmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                         uint32_t queryCount);
    void PreCallRecordCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                      VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                      const VkImageResolve* pRegions);
    void PreCallRecordCmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4]);
    void PreCallRecordCmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp,
                                      float depthBiasSlopeFactor);
    void PreCallRecordCmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds);
    void PreCallRecordCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask);
    void PreCallRecordCmdSetExclusiveScissorNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                               uint32_t exclusiveScissorCount, const VkRect2D* pExclusiveScissors);
    void PreCallRecordCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth);
    void PreCallRecordCmdSetLineStippleEXT(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor, uint16_t lineStipplePattern);
    void PreCallRecordCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount,
                                    const VkRect2D* pScissors);
    void PreCallRecordCmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t compareMask);
    void PreCallRecordCmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t reference);
    void PreCallRecordCmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t writeMask);
    void PreCallRecordCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                     const VkViewport* pViewports);
    void PreCallRecordCmdSetViewportShadingRatePaletteNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                         uint32_t viewportCount,
                                                         const VkShadingRatePaletteNV* pShadingRatePalettes);
    void PostCallRecordCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                       VkDeviceSize dataSize, const void* pData);
    void PreCallRecordCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                    VkPipelineStageFlags sourceStageMask, VkPipelineStageFlags dstStageMask,
                                    uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                    uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                    uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers);
    void PostCallRecordCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                         VkQueryPool queryPool, uint32_t slot);
    void PreCallRecordCmdSetViewportWScalingNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                               const VkViewportWScalingNV* pViewportWScalings);
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    void PostCallRecordGetAndroidHardwareBufferPropertiesANDROID(VkDevice device, const struct AHardwareBuffer* buffer,
                                                                 VkAndroidHardwareBufferPropertiesANDROID* pProperties,
                                                                 VkResult result);
#endif  // VK_USE_PLATFORM_ANDROID_KHR

    // WSI
    void PostCallRecordAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore,
                                           VkFence fence, uint32_t* pImageIndex, VkResult result);
    void PostCallRecordAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo, uint32_t* pImageIndex,
                                            VkResult result);
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    void PostCallRecordCreateAndroidSurfaceKHR(VkInstance instance, const VkAndroidSurfaceCreateInfoKHR* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface, VkResult result);
#endif  // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_IOS_MVK
    void PostCallRecordCreateIOSSurfaceMVK(VkInstance instance, const VkIOSSurfaceCreateInfoMVK* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface, VkResult result);
#endif  // VK_USE_PLATFORM_IOS_MVK
#ifdef VK_USE_PLATFORM_MACOS_MVK
    void PostCallRecordCreateMacOSSurfaceMVK(VkInstance instance, const VkMacOSSurfaceCreateInfoMVK* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface, VkResult result);
#endif  // VK_USE_PLATFORM_MACOS_MVK
#ifdef VK_USE_PLATFORM_METAL_EXT
    void PostCallRecordCreateMetalSurfaceEXT(VkInstance instance, const VkMetalSurfaceCreateInfoEXT* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface, VkResult result);
#endif  // VK_USE_PLATFORM_METAL_EXT
#ifdef VK_USE_PLATFORM_WIN32_KHR
    void PostCallRecordCreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface, VkResult result);
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    void PostCallRecordCreateWaylandSurfaceKHR(VkInstance instance, const VkWaylandSurfaceCreateInfoKHR* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface, VkResult result);
#endif  // VK_USE_PLATFORM_WAYLAND_KHR
#ifdef VK_USE_PLATFORM_XCB_KHR
    void PostCallRecordCreateXcbSurfaceKHR(VkInstance instance, const VkXcbSurfaceCreateInfoKHR* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface, VkResult result);
#endif  // VK_USE_PLATFORM_XCB_KHR
#ifdef VK_USE_PLATFORM_XLIB_KHR
    void PostCallRecordCreateXlibSurfaceKHR(VkInstance instance, const VkXlibSurfaceCreateInfoKHR* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface, VkResult result);
#endif  // VK_USE_PLATFORM_XLIB_KHR
    void PostCallRecordCreateHeadlessSurfaceEXT(VkInstance instance, const VkHeadlessSurfaceCreateInfoEXT* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface, VkResult result);

    // State Utilty functions
    bool AddCommandBufferMem(small_unordered_map<CMD_BUFFER_STATE*, int, 8>& cb_bindings, VkDeviceMemory obj,
                             CMD_BUFFER_STATE* cb_node);
    bool AddCommandBufferBinding(small_unordered_map<CMD_BUFFER_STATE*, int, 8>& cb_bindings, const VulkanTypedHandle& obj,
                                 CMD_BUFFER_STATE* cb_node);
    void AddCommandBufferBindingAccelerationStructure(CMD_BUFFER_STATE*, ACCELERATION_STRUCTURE_STATE*);
    void AddCommandBufferBindingBuffer(CMD_BUFFER_STATE*, BUFFER_STATE*);
    void AddCommandBufferBindingBufferView(CMD_BUFFER_STATE*, BUFFER_VIEW_STATE*);
    void AddCommandBufferBindingImage(CMD_BUFFER_STATE*, IMAGE_STATE*);
    void AddCommandBufferBindingImageView(CMD_BUFFER_STATE*, IMAGE_VIEW_STATE*);
    void AddCommandBufferBindingSampler(CMD_BUFFER_STATE*, SAMPLER_STATE*);
    void AddMemObjInfo(void* object, const VkDeviceMemory mem, const VkMemoryAllocateInfo* pAllocateInfo);
    void AddFramebufferBinding(CMD_BUFFER_STATE* cb_state, FRAMEBUFFER_STATE* fb_state);
    void ClearMemoryObjectBindings(const VulkanTypedHandle& typed_handle);
    void ClearMemoryObjectBinding(const VulkanTypedHandle& typed_handle, DEVICE_MEMORY_STATE* mem_info);
    void DecrementBoundResources(CMD_BUFFER_STATE const* cb_node);
    void DeleteDescriptorSetPools();
    void FreeCommandBufferStates(COMMAND_POOL_STATE* pool_state, const uint32_t command_buffer_count,
                                 const VkCommandBuffer* command_buffers);
    void FreeDescriptorSet(cvdescriptorset::DescriptorSet* descriptor_set);
    std::vector<const IMAGE_VIEW_STATE*> GetAttachmentViews(const VkRenderPassBeginInfo& rp_begin,
                                                            const FRAMEBUFFER_STATE& fb_state) const;
    std::vector<const IMAGE_VIEW_STATE*> GetCurrentAttachmentViews(const CMD_BUFFER_STATE& cb_state) const;
    BASE_NODE* GetStateStructPtrFromObject(const VulkanTypedHandle& object_struct);
    VkFormatFeatureFlags GetPotentialFormatFeatures(VkFormat format) const;
    void IncrementBoundObjects(CMD_BUFFER_STATE const* cb_node);
    void IncrementResources(CMD_BUFFER_STATE* cb_node);
    void InsertAccelerationStructureMemoryRange(VkAccelerationStructureNV as, DEVICE_MEMORY_STATE* mem_info,
                                                VkDeviceSize mem_offset);
    void InsertBufferMemoryRange(VkBuffer buffer, DEVICE_MEMORY_STATE* mem_info, VkDeviceSize mem_offset);
    void InsertImageMemoryRange(VkImage image, DEVICE_MEMORY_STATE* mem_info, VkDeviceSize mem_offset);
    void InsertMemoryRange(const VulkanTypedHandle& typed_handle, DEVICE_MEMORY_STATE* mem_info, VkDeviceSize memoryOffset);
    void InvalidateCommandBuffers(small_unordered_map<CMD_BUFFER_STATE*, int, 8>& cb_nodes, const VulkanTypedHandle& obj,
                                  bool unlink = true);
    void InvalidateLinkedCommandBuffers(std::unordered_set<CMD_BUFFER_STATE*>& cb_nodes, const VulkanTypedHandle& obj);
    void PerformAllocateDescriptorSets(const VkDescriptorSetAllocateInfo*, const VkDescriptorSet*,
                                       const cvdescriptorset::AllocateDescriptorSetsData*);
    void PerformUpdateDescriptorSetsWithTemplateKHR(VkDescriptorSet descriptorSet, const TEMPLATE_STATE* template_state,
                                                    const void* pData);
    void RecordAcquireNextImageState(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore,
                                     VkFence fence, uint32_t* pImageIndex);
    void RecordCmdBeginQuery(CMD_BUFFER_STATE* cb_state, const QueryObject& query_obj);
    void RecordCmdEndQuery(CMD_BUFFER_STATE* cb_state, const QueryObject& query_obj);
    void RecordCmdEndRenderPassState(VkCommandBuffer commandBuffer);
    void RecordCmdBeginRenderPassState(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                       const VkSubpassContents contents);
    void RecordCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents);
    void RecordCmdPushDescriptorSetState(CMD_BUFFER_STATE* cb_state, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout,
                                         uint32_t set, uint32_t descriptorWriteCount,
                                         const VkWriteDescriptorSet* pDescriptorWrites);
    void RecordCreateImageANDROID(const VkImageCreateInfo* create_info, IMAGE_STATE* is_node);
    void RecordCreateBufferANDROID(const VkBufferCreateInfo* create_info, BUFFER_STATE* bs_node);
    void RecordCreateRenderPassState(RenderPassCreateVersion rp_version, std::shared_ptr<RENDER_PASS_STATE>& render_pass,
                                     VkRenderPass* pRenderPass);
    void RecordCreateSamplerYcbcrConversionState(const VkSamplerYcbcrConversionCreateInfo* create_info,
                                                 VkSamplerYcbcrConversion ycbcr_conversion);
    void RecordCreateSamplerYcbcrConversionANDROID(const VkSamplerYcbcrConversionCreateInfo* create_info,
                                                   VkSamplerYcbcrConversion ycbcr_conversion,
                                                   SAMPLER_YCBCR_CONVERSION_STATE* ycbcr_state);
    void RecordCreateSwapchainState(VkResult result, const VkSwapchainCreateInfoKHR* pCreateInfo, VkSwapchainKHR* pSwapchain,
                                    SURFACE_STATE* surface_state, SWAPCHAIN_NODE* old_swapchain_state);
    void RecordDestroySamplerYcbcrConversionState(VkSamplerYcbcrConversion ycbcr_conversion);
    void RecordDestroySamplerYcbcrConversionANDROID(VkSamplerYcbcrConversion ycbcr_conversion);
    void RecordEnumeratePhysicalDeviceGroupsState(uint32_t* pPhysicalDeviceGroupCount,
                                                  VkPhysicalDeviceGroupPropertiesKHR* pPhysicalDeviceGroupProperties);
    void RecordEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCounters(VkPhysicalDevice physicalDevice,
                                                                          uint32_t queueFamilyIndex, uint32_t* pCounterCount,
                                                                          VkPerformanceCounterKHR* pCounters);
    void RecordGetBufferMemoryRequirementsState(VkBuffer buffer);
    void RecordGetDeviceQueueState(uint32_t queue_family_index, VkQueue queue);
    void RecordGetExternalFenceState(VkFence fence, VkExternalFenceHandleTypeFlagBitsKHR handle_type);
    void RecordGetImageMemoryRequirementsState(VkImage image, const VkImageMemoryRequirementsInfo2* pInfo);
    void RecordImportSemaphoreState(VkSemaphore semaphore, VkExternalSemaphoreHandleTypeFlagBitsKHR handle_type,
                                    VkSemaphoreImportFlagsKHR flags);
    void RecordGetPhysicalDeviceDisplayPlanePropertiesState(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                            void* pProperties);
    void RecordGetExternalSemaphoreState(VkSemaphore semaphore, VkExternalSemaphoreHandleTypeFlagBitsKHR handle_type);
    void RecordImportFenceState(VkFence fence, VkExternalFenceHandleTypeFlagBitsKHR handle_type, VkFenceImportFlagsKHR flags);
    void RecordUpdateDescriptorSetWithTemplateState(VkDescriptorSet descriptorSet,
                                                    VkDescriptorUpdateTemplateKHR descriptorUpdateTemplate, const void* pData);
    void RecordCreateDescriptorUpdateTemplateState(const VkDescriptorUpdateTemplateCreateInfoKHR* pCreateInfo,
                                                   VkDescriptorUpdateTemplateKHR* pDescriptorUpdateTemplate);
    void RecordMappedMemory(VkDeviceMemory mem, VkDeviceSize offset, VkDeviceSize size, void** ppData);
    void RecordPipelineShaderStage(const VkPipelineShaderStageCreateInfo* pStage, PIPELINE_STATE* pipeline,
                                   PIPELINE_STATE::StageState* stage_state) const;
    void RecordRenderPassDAG(RenderPassCreateVersion rp_version, const VkRenderPassCreateInfo2KHR* pCreateInfo,
                             RENDER_PASS_STATE* render_pass);
    void RecordVulkanSurface(VkSurfaceKHR* pSurface);
    void RemoveAccelerationStructureMemoryRange(VkAccelerationStructureNV as, DEVICE_MEMORY_STATE* mem_info);
    void RemoveCommandBufferBinding(const VulkanTypedHandle& object, CMD_BUFFER_STATE* cb_node);
    void RemoveBufferMemoryRange(VkBuffer buffer, DEVICE_MEMORY_STATE* mem_info);
    void RemoveImageMemoryRange(VkImage image, DEVICE_MEMORY_STATE* mem_info);
    void ResetCommandBufferState(const VkCommandBuffer cb);
    void RetireFence(VkFence fence);
    void RetireTimelineSemaphore(VkSemaphore semaphore, uint64_t until_payload);
    void RecordWaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout, VkResult result);
    void RetireWorkOnQueue(QUEUE_STATE* pQueue, uint64_t seq);
    static bool SetEventStageMask(VkEvent event, VkPipelineStageFlags stageMask, EventToStageMap* localEventToStageMap);
    void ResetCommandBufferPushConstantDataIfIncompatible(CMD_BUFFER_STATE* cb_state, VkPipelineLayout layout);
    void SetMemBinding(VkDeviceMemory mem, BINDABLE* mem_binding, VkDeviceSize memory_offset,
                       const VulkanTypedHandle& typed_handle);
    static bool SetQueryState(QueryObject object, QueryState value, QueryMap* localQueryToStateMap);
    static bool SetQueryStateMulti(VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, uint32_t perfPass,
                                   QueryState value, QueryMap* localQueryToStateMap);
    QueryState GetQueryState(const QueryMap* localQueryToStateMap, VkQueryPool queryPool, uint32_t queryIndex,
                             uint32_t perfPass) const;
    bool SetSparseMemBinding(const VkDeviceMemory mem, const VkDeviceSize mem_offset, const VkDeviceSize mem_size,
                             const VulkanTypedHandle& typed_handle);
    void UpdateBindBufferMemoryState(VkBuffer buffer, VkDeviceMemory mem, VkDeviceSize memoryOffset);
    void UpdateBindImageMemoryState(const VkBindImageMemoryInfo& bindInfo);
    void UpdateLastBoundDescriptorSets(CMD_BUFFER_STATE* cb_state, VkPipelineBindPoint pipeline_bind_point,
                                       const PIPELINE_LAYOUT_STATE* pipeline_layout, uint32_t first_set, uint32_t set_count,
                                       const VkDescriptorSet* pDescriptorSets, cvdescriptorset::DescriptorSet* push_descriptor_set,
                                       uint32_t dynamic_offset_count, const uint32_t* p_dynamic_offsets);
    void UpdateStateCmdDrawDispatchType(CMD_BUFFER_STATE* cb_state, CMD_TYPE cmd_type, VkPipelineBindPoint bind_point,
                                        const char* function);
    void UpdateStateCmdDrawType(CMD_BUFFER_STATE* cb_state, CMD_TYPE cmd_type, VkPipelineBindPoint bind_point,
                                const char* function);
    void UpdateDrawState(CMD_BUFFER_STATE* cb_state, CMD_TYPE cmd_type, const VkPipelineBindPoint bind_point, const char* function);
    void UpdateAllocateDescriptorSetsData(const VkDescriptorSetAllocateInfo*, cvdescriptorset::AllocateDescriptorSetsData*) const;
    void PostCallRecordCmdBuildAccelerationStructureKHR(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                                        const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
                                                        const VkAccelerationStructureBuildOffsetInfoKHR* const* ppOffsetInfos);

    void PostCallRecordCmdCopyAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                                       const VkCopyAccelerationStructureInfoKHR* pInfo);

    void PreCallRecordCmdSetCullModeEXT(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode);
    void PreCallRecordCmdSetFrontFaceEXT(VkCommandBuffer commandBuffer, VkFrontFace frontFace);
    void PreCallRecordCmdSetPrimitiveTopologyEXT(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology);
    void PreCallRecordCmdSetViewportWithCountEXT(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                                 const VkViewport* pViewports);
    void PreCallRecordCmdSetScissorWithCountEXT(VkCommandBuffer commandBuffer, uint32_t scissorCount, const VkRect2D* pScissors);
    void PreCallRecordCmdBindVertexBuffers2EXT(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                               const VkBuffer* pBuffers, const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes,
                                               const VkDeviceSize* pStrides);
    void PreCallRecordCmdSetDepthTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable);
    void PreCallRecordCmdSetDepthWriteEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable);
    void PreCallRecordCmdSetDepthCompareOpEXT(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp);
    void PreCallRecordCmdSetDepthBoundsTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable);
    void PreCallRecordCmdSetStencilTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable);
    void PreCallRecordCmdSetStencilOpEXT(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp,
                                         VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp);

    DeviceFeatures enabled_features = {};
    // Device specific data
    VkPhysicalDeviceMemoryProperties phys_dev_mem_props = {};
    VkPhysicalDeviceProperties phys_dev_props = {};
    VkPhysicalDeviceVulkan11Properties phys_dev_props_core11 = {};
    VkPhysicalDeviceVulkan12Properties phys_dev_props_core12 = {};
    VkDeviceGroupDeviceCreateInfo device_group_create_info = {};
    uint32_t physical_device_count;
    uint32_t custom_border_color_sampler_count = 0;

    // Device extension properties -- storing properties gathered from VkPhysicalDeviceProperties2KHR::pNext chain
    struct DeviceExtensionProperties {
        uint32_t max_push_descriptors;  // from VkPhysicalDevicePushDescriptorPropertiesKHR::maxPushDescriptors
        VkPhysicalDeviceShadingRateImagePropertiesNV shading_rate_image_props;
        VkPhysicalDeviceMeshShaderPropertiesNV mesh_shader_props;
        VkPhysicalDeviceInlineUniformBlockPropertiesEXT inline_uniform_block_props;
        VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT vtx_attrib_divisor_props;
        VkPhysicalDeviceCooperativeMatrixPropertiesNV cooperative_matrix_props;
        VkPhysicalDeviceTransformFeedbackPropertiesEXT transform_feedback_props;
        VkPhysicalDeviceRayTracingPropertiesNV ray_tracing_propsNV;
        VkPhysicalDeviceRayTracingPropertiesKHR ray_tracing_propsKHR;
        VkPhysicalDeviceTexelBufferAlignmentPropertiesEXT texel_buffer_alignment_props;
        VkPhysicalDeviceFragmentDensityMapPropertiesEXT fragment_density_map_props;
        VkPhysicalDeviceFragmentDensityMap2PropertiesEXT fragment_density_map2_props;
        VkPhysicalDevicePerformanceQueryPropertiesKHR performance_query_props;
        VkPhysicalDeviceSampleLocationsPropertiesEXT sample_locations_props;
        VkPhysicalDeviceCustomBorderColorPropertiesEXT custom_border_color_props;
    };
    DeviceExtensionProperties phys_dev_ext_props = {};
    std::vector<VkCooperativeMatrixPropertiesNV> cooperative_matrix_properties;

    // Map for queue family index to queue count
    unordered_map<uint32_t, uint32_t> queue_family_index_map;
    unordered_map<uint32_t, VkDeviceQueueCreateFlags> queue_family_create_flags_map;
    bool performance_lock_acquired = false;

    template <typename ExtProp>
    void GetPhysicalDeviceExtProperties(VkPhysicalDevice gpu, ExtEnabled enabled, ExtProp* ext_prop) {
        assert(ext_prop);
        if (enabled) {
            *ext_prop = lvl_init_struct<ExtProp>();
            auto prop2 = lvl_init_struct<VkPhysicalDeviceProperties2KHR>(ext_prop);
            DispatchGetPhysicalDeviceProperties2KHR(gpu, &prop2);
        }
    }

  private:
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
        VkDeviceSize free_ = 0;
    };
    FakeAllocator fake_memory;
};
