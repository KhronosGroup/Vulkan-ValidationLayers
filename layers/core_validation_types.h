/* Copyright (c) 2015-2021 The Khronos Group Inc.
 * Copyright (c) 2015-2021 Valve Corporation
 * Copyright (c) 2015-2021 LunarG, Inc.
 * Copyright (C) 2015-2021 Google Inc.
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
 * Author: John Zulauf <jzulauf@lunarg.com>
 * Author: Tobias Hector <tobias.hector@amd.com>
 */
#ifndef CORE_VALIDATION_TYPES_H_
#define CORE_VALIDATION_TYPES_H_

#include "cast_utils.h"
#include "hash_vk_types.h"
#include "sparse_containers.h"
#include "vk_safe_struct.h"
#include "vulkan/vulkan.h"
#include "vk_layer_logging.h"
#include "vk_object_types.h"
#include "vk_extension_helper.h"
#include "vk_typemap_helper.h"
#include "convert_to_renderpass2.h"
#include "layer_chassis_dispatch.h"
#include "image_layout_map.h"

#include <array>
#include <atomic>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <string.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <memory>
#include <list>

#include "android_ndk_types.h"

// Fwd declarations -- including descriptor_set.h creates an ugly include loop
namespace cvdescriptorset {
class DescriptorSetLayoutDef;
class DescriptorSetLayout;
class DescriptorSet;
class Descriptor;

}  // namespace cvdescriptorset

// Only CoreChecks uses this, but the state tracker stores it.
constexpr static auto kInvalidLayout = image_layout_map::kInvalidLayout;
using ImageSubresourceLayoutMap = image_layout_map::ImageSubresourceLayoutMap;

struct CMD_BUFFER_STATE;
class CoreChecks;
class ValidationStateTracker;

class BASE_NODE {
  public:
    // Track when object is being used by an in-flight command buffer
    std::atomic_int in_use;
    // Track command buffers that this object is bound to
    //  binding initialized when cmd referencing object is bound to command buffer
    //  binding removed when command buffer is reset or destroyed
    // When an object is destroyed, any bound cbs are set to INVALID.
    // "int" value is an index into object_bindings where the corresponding
    // backpointer to this node is stored.
    small_unordered_map<CMD_BUFFER_STATE *, int, 8> cb_bindings;
    // Set to true when the API-level object is destroyed, but this object may
    // hang around until its shared_ptr refcount goes to zero.
    bool destroyed;

    BASE_NODE() {
        in_use.store(0);
        destroyed = false;
    };
};

// Track command pools and their command buffers
struct COMMAND_POOL_STATE : public BASE_NODE {
    VkCommandPool commandPool;
    VkCommandPoolCreateFlags createFlags;
    uint32_t queueFamilyIndex;
    bool unprotected;  // can't be used for protected memory
    // Cmd buffers allocated from this pool
    std::unordered_set<VkCommandBuffer> commandBuffers;
};

// Utilities for barriers and the commmand pool
template <typename Barrier>
static bool IsTransferOp(const Barrier &barrier) {
    return barrier.srcQueueFamilyIndex != barrier.dstQueueFamilyIndex;
}

template <typename Barrier, bool assume_transfer = false>
static bool TempIsReleaseOp(const COMMAND_POOL_STATE *pool, const Barrier &barrier) {
    return (assume_transfer || IsTransferOp(barrier)) && (pool->queueFamilyIndex == barrier.srcQueueFamilyIndex);
}

template <typename Barrier, bool assume_transfer = false>
static bool IsAcquireOp(const COMMAND_POOL_STATE *pool, const Barrier &barrier) {
    return (assume_transfer || IsTransferOp(barrier)) && (pool->queueFamilyIndex == barrier.dstQueueFamilyIndex);
}

static inline bool QueueFamilyIsExternal(const uint32_t queue_family_index) {
    return (queue_family_index == VK_QUEUE_FAMILY_EXTERNAL) || (queue_family_index == VK_QUEUE_FAMILY_FOREIGN_EXT);
}

static inline bool QueueFamilyIsIgnored(uint32_t queue_family_index) { return queue_family_index == VK_QUEUE_FAMILY_IGNORED; }

// Intentionally ignore VulkanTypedHandle::node, it is optional
inline bool operator==(const VulkanTypedHandle &a, const VulkanTypedHandle &b) NOEXCEPT {
    return a.handle == b.handle && a.type == b.type;
}
namespace std {
template <>
struct hash<VulkanTypedHandle> {
    size_t operator()(VulkanTypedHandle obj) const NOEXCEPT { return hash<uint64_t>()(obj.handle) ^ hash<uint32_t>()(obj.type); }
};
}  // namespace std

// Flags describing requirements imposed by the pipeline on a descriptor. These
// can't be checked at pipeline creation time as they depend on the Image or
// ImageView bound.
enum descriptor_req {
    DESCRIPTOR_REQ_VIEW_TYPE_1D = 1 << VK_IMAGE_VIEW_TYPE_1D,
    DESCRIPTOR_REQ_VIEW_TYPE_1D_ARRAY = 1 << VK_IMAGE_VIEW_TYPE_1D_ARRAY,
    DESCRIPTOR_REQ_VIEW_TYPE_2D = 1 << VK_IMAGE_VIEW_TYPE_2D,
    DESCRIPTOR_REQ_VIEW_TYPE_2D_ARRAY = 1 << VK_IMAGE_VIEW_TYPE_2D_ARRAY,
    DESCRIPTOR_REQ_VIEW_TYPE_3D = 1 << VK_IMAGE_VIEW_TYPE_3D,
    DESCRIPTOR_REQ_VIEW_TYPE_CUBE = 1 << VK_IMAGE_VIEW_TYPE_CUBE,
    DESCRIPTOR_REQ_VIEW_TYPE_CUBE_ARRAY = 1 << VK_IMAGE_VIEW_TYPE_CUBE_ARRAY,

    DESCRIPTOR_REQ_ALL_VIEW_TYPE_BITS = (1 << (VK_IMAGE_VIEW_TYPE_CUBE_ARRAY + 1)) - 1,

    DESCRIPTOR_REQ_SINGLE_SAMPLE = 2 << VK_IMAGE_VIEW_TYPE_CUBE_ARRAY,
    DESCRIPTOR_REQ_MULTI_SAMPLE = DESCRIPTOR_REQ_SINGLE_SAMPLE << 1,

    DESCRIPTOR_REQ_COMPONENT_TYPE_FLOAT = DESCRIPTOR_REQ_MULTI_SAMPLE << 1,
    DESCRIPTOR_REQ_COMPONENT_TYPE_SINT = DESCRIPTOR_REQ_COMPONENT_TYPE_FLOAT << 1,
    DESCRIPTOR_REQ_COMPONENT_TYPE_UINT = DESCRIPTOR_REQ_COMPONENT_TYPE_SINT << 1,

    DESCRIPTOR_REQ_VIEW_ATOMIC_OPERATION = DESCRIPTOR_REQ_COMPONENT_TYPE_UINT << 1,
    DESCRIPTOR_REQ_SAMPLER_IMPLICITLOD_DREF_PROJ = DESCRIPTOR_REQ_VIEW_ATOMIC_OPERATION << 1,
    DESCRIPTOR_REQ_SAMPLER_BIAS_OFFSET = DESCRIPTOR_REQ_SAMPLER_IMPLICITLOD_DREF_PROJ << 1,

};

extern unsigned DescriptorRequirementsBitsFromFormat(VkFormat fmt);

typedef std::pair<unsigned, unsigned> descriptor_slot_t;

struct SamplerUsedByImage {
    descriptor_slot_t sampler_slot;
    uint32_t sampler_index;
};

namespace std {
template <>
struct less<SamplerUsedByImage> {
    bool operator()(const SamplerUsedByImage &left, const SamplerUsedByImage &right) const { return false; }
};
}  // namespace std

struct SAMPLER_STATE;
struct DescriptorRequirement {
    descriptor_req reqs;
    bool is_writable;
    std::vector<std::map<SamplerUsedByImage, const cvdescriptorset::Descriptor *>>
        samplers_used_by_image;  // Copy from StageState.interface_var. It combines from plural shader stages.
                                 // The index of array is index of image.

    DescriptorRequirement() : reqs(descriptor_req(0)), is_writable(false) {}
};

inline bool operator==(const DescriptorRequirement &a, const DescriptorRequirement &b) NOEXCEPT { return a.reqs == b.reqs; }

inline bool operator<(const DescriptorRequirement &a, const DescriptorRequirement &b) NOEXCEPT { return a.reqs < b.reqs; }

typedef std::map<uint32_t, DescriptorRequirement> BindingReqMap;

struct DESCRIPTOR_POOL_STATE : BASE_NODE {
    VkDescriptorPool pool;
    uint32_t maxSets;        // Max descriptor sets allowed in this pool
    uint32_t availableSets;  // Available descriptor sets in this pool

    safe_VkDescriptorPoolCreateInfo createInfo;
    std::unordered_set<cvdescriptorset::DescriptorSet *> sets;  // Collection of all sets in this pool
    std::map<uint32_t, uint32_t> maxDescriptorTypeCount;        // Max # of descriptors of each type in this pool
    std::map<uint32_t, uint32_t> availableDescriptorTypeCount;  // Available # of descriptors of each type in this pool

    DESCRIPTOR_POOL_STATE(const VkDescriptorPool pool, const VkDescriptorPoolCreateInfo *pCreateInfo)
        : pool(pool),
          maxSets(pCreateInfo->maxSets),
          availableSets(pCreateInfo->maxSets),
          createInfo(pCreateInfo),
          maxDescriptorTypeCount(),
          availableDescriptorTypeCount() {
        // Collect maximums per descriptor type.
        for (uint32_t i = 0; i < createInfo.poolSizeCount; ++i) {
            uint32_t typeIndex = static_cast<uint32_t>(createInfo.pPoolSizes[i].type);
            // Same descriptor types can appear several times
            maxDescriptorTypeCount[typeIndex] += createInfo.pPoolSizes[i].descriptorCount;
            availableDescriptorTypeCount[typeIndex] = maxDescriptorTypeCount[typeIndex];
        }
    }
};

struct MemRange {
    VkDeviceSize offset = 0;
    VkDeviceSize size = 0;
};

// Data struct for tracking memory object
struct DEVICE_MEMORY_STATE : public BASE_NODE {
    void *object;  // Dispatchable object used to create this memory (device of swapchain)
    VkDeviceMemory mem;
    safe_VkMemoryAllocateInfo alloc_info;
    bool is_dedicated;
    VkBuffer dedicated_buffer;
    VkImage dedicated_image;
    bool is_export;
    bool is_import;
    bool is_import_ahb;   // The VUID check depends on if the imported memory is for AHB
    bool unprotected;     // can't be used for protected memory
    bool multi_instance;  // Allocated from MULTI_INSTANCE heap or having more than one deviceMask bit set
    VkExternalMemoryHandleTypeFlags export_handle_type_flags;
    VkExternalMemoryHandleTypeFlags import_handle_type_flags;
    std::unordered_set<VulkanTypedHandle> obj_bindings;  // objects bound to this memory
    // Convenience vectors of handles to speed up iterating over objects independently
    std::unordered_set<VkImage> bound_images;
    std::unordered_set<VkBuffer> bound_buffers;
    std::unordered_set<VkAccelerationStructureNV> bound_acceleration_structures;

    MemRange mapped_range;
    void *shadow_copy_base;          // Base of layer's allocation for guard band, data, and alignment space
    void *shadow_copy;               // Pointer to start of guard-band data before mapped region
    uint64_t shadow_pad_size;        // Size of the guard-band data before and after actual data. It MUST be a
                                     // multiple of limits.minMemoryMapAlignment
    void *p_driver_data;             // Pointer to application's actual memory
    VkDeviceSize fake_base_address;  // To allow a unified view of allocations, useful to Synchronization Validation

    DEVICE_MEMORY_STATE(void *disp_object, const VkDeviceMemory in_mem, const VkMemoryAllocateInfo *p_alloc_info,
                        uint64_t fake_address)
        : object(disp_object),
          mem(in_mem),
          alloc_info(p_alloc_info),
          is_dedicated(false),
          dedicated_buffer(VK_NULL_HANDLE),
          dedicated_image(VK_NULL_HANDLE),
          is_export(false),
          is_import(false),
          is_import_ahb(false),
          unprotected(true),
          multi_instance(false),
          export_handle_type_flags(0),
          import_handle_type_flags(0),
          mapped_range{},
          shadow_copy_base(0),
          shadow_copy(0),
          shadow_pad_size(0),
          p_driver_data(0),
          fake_base_address(fake_address){};
};

// Generic memory binding struct to track objects bound to objects
struct MEM_BINDING {
    std::shared_ptr<DEVICE_MEMORY_STATE> mem_state;
    VkDeviceSize offset;
    VkDeviceSize size;
};

class BUFFER_STATE;
struct BufferBinding {
    std::shared_ptr<BUFFER_STATE> buffer_state;
    VkDeviceSize size;
    VkDeviceSize offset;
    VkDeviceSize stride;

    BufferBinding() : buffer_state(), size(0), offset(0), stride(0) {}
    virtual ~BufferBinding() {}

    virtual void reset() { *this = BufferBinding(); }
};

struct IndexBufferBinding : BufferBinding {
    VkIndexType index_type;

    IndexBufferBinding() : BufferBinding(), index_type(static_cast<VkIndexType>(0)) {}
    virtual ~IndexBufferBinding() {}

    virtual void reset() override { *this = IndexBufferBinding(); }
};

inline bool operator==(MEM_BINDING a, MEM_BINDING b) NOEXCEPT {
    return a.mem_state == b.mem_state && a.offset == b.offset && a.size == b.size;
}

namespace std {
template <>
struct hash<MEM_BINDING> {
    size_t operator()(MEM_BINDING mb) const NOEXCEPT {
        auto intermediate = hash<uint64_t>()(reinterpret_cast<uint64_t &>(mb.mem_state)) ^ hash<uint64_t>()(mb.offset);
        return intermediate ^ hash<uint64_t>()(mb.size);
    }
};
}  // namespace std

// Superclass for bindable object state (currently images and buffers)
class BINDABLE : public BASE_NODE {
  public:
    bool sparse;  // Is this object being bound with sparse memory or not?
    // Non-sparse binding data
    MEM_BINDING binding;
    // Memory requirements for this BINDABLE
    VkMemoryRequirements requirements;
    // bool to track if memory requirements were checked
    bool memory_requirements_checked;
    // Tracks external memory types creating resource
    VkExternalMemoryHandleTypeFlags external_memory_handle;
    // Sparse binding data, initially just tracking MEM_BINDING per mem object
    //  There's more data for sparse bindings so need better long-term solution
    // TODO : Need to update solution to track all sparse binding data
    std::unordered_set<MEM_BINDING> sparse_bindings;
    // True if memory will be imported/exported from/to an Android Hardware Buffer
    bool external_ahb;
    bool unprotected;  // can't be used for protected memory

    small_unordered_set<DEVICE_MEMORY_STATE *, 1> bound_memory_set_;

    BINDABLE()
        : sparse(false),
          binding{},
          requirements{},
          memory_requirements_checked(false),
          external_memory_handle(0),
          sparse_bindings{},
          external_ahb(false),
          unprotected(true),
          bound_memory_set_{} {};

    // Update the cached set of memory bindings.
    // Code that changes binding.mem or sparse_bindings must call UpdateBoundMemorySet()
    void UpdateBoundMemorySet() {
        bound_memory_set_.clear();
        if (!sparse) {
            if (binding.mem_state) bound_memory_set_.insert(binding.mem_state.get());
        } else {
            for (auto sb : sparse_bindings) {
                bound_memory_set_.insert(sb.mem_state.get());
            }
        }
    }

    // Return unordered set of memory objects that are bound
    // Instead of creating a set from scratch each query, return the cached one
    const small_unordered_set<DEVICE_MEMORY_STATE *, 1> &GetBoundMemory() const { return bound_memory_set_; }
};

class BUFFER_STATE : public BINDABLE {
  public:
    VkBuffer buffer;
    VkBufferCreateInfo createInfo;
    VkDeviceAddress deviceAddress;
    BUFFER_STATE(VkBuffer buff, const VkBufferCreateInfo *pCreateInfo) : buffer(buff), createInfo(*pCreateInfo) {
        if ((createInfo.sharingMode == VK_SHARING_MODE_CONCURRENT) && (createInfo.queueFamilyIndexCount > 0)) {
            uint32_t *pQueueFamilyIndices = new uint32_t[createInfo.queueFamilyIndexCount];
            for (uint32_t i = 0; i < createInfo.queueFamilyIndexCount; i++) {
                pQueueFamilyIndices[i] = pCreateInfo->pQueueFamilyIndices[i];
            }
            createInfo.pQueueFamilyIndices = pQueueFamilyIndices;
        }

        if (createInfo.flags & VK_BUFFER_CREATE_SPARSE_BINDING_BIT) {
            sparse = true;
        }

        auto *externalMemoryInfo = LvlFindInChain<VkExternalMemoryBufferCreateInfo>(pCreateInfo->pNext);
        if (externalMemoryInfo) {
            external_memory_handle = externalMemoryInfo->handleTypes;
        }
    };

    BUFFER_STATE(BUFFER_STATE const &rh_obj) = delete;

    ~BUFFER_STATE() {
        if ((createInfo.sharingMode == VK_SHARING_MODE_CONCURRENT) && (createInfo.queueFamilyIndexCount > 0)) {
            delete[] createInfo.pQueueFamilyIndices;
            createInfo.pQueueFamilyIndices = nullptr;
        }
    };
};

class BUFFER_VIEW_STATE : public BASE_NODE {
  public:
    VkBufferView buffer_view;
    VkBufferViewCreateInfo create_info;
    std::shared_ptr<BUFFER_STATE> buffer_state;
    VkFormatFeatureFlags format_features;
    BUFFER_VIEW_STATE(const std::shared_ptr<BUFFER_STATE> &bf, VkBufferView bv, const VkBufferViewCreateInfo *ci)
        : buffer_view(bv), create_info(*ci), buffer_state(bf){};
    BUFFER_VIEW_STATE(const BUFFER_VIEW_STATE &rh_obj) = delete;
};

struct SAMPLER_STATE : public BASE_NODE {
    VkSampler sampler;
    VkSamplerCreateInfo createInfo;
    VkSamplerYcbcrConversion samplerConversion = VK_NULL_HANDLE;
    VkSamplerCustomBorderColorCreateInfoEXT customCreateInfo = {};

    SAMPLER_STATE(const VkSampler *ps, const VkSamplerCreateInfo *pci) : sampler(*ps), createInfo(*pci) {
        auto *conversionInfo = LvlFindInChain<VkSamplerYcbcrConversionInfo>(pci->pNext);
        if (conversionInfo) samplerConversion = conversionInfo->conversion;
        auto cbci = LvlFindInChain<VkSamplerCustomBorderColorCreateInfoEXT>(pci->pNext);
        if (cbci) customCreateInfo = *cbci;
    }
};

class IMAGE_STATE : public BINDABLE {
  public:
    VkImage image;
    safe_VkImageCreateInfo safe_create_info;
    VkImageCreateInfo &createInfo;
    bool valid;               // If this is a swapchain image backing memory track valid here as it doesn't have DEVICE_MEMORY_STATE
    bool acquired;            // If this is a swapchain image, has it been acquired by the app.
    bool shared_presentable;  // True for a front-buffered swapchain image
    bool layout_locked;       // A front-buffered image that has been presented can never have layout transitioned
    bool get_sparse_reqs_called;         // Track if GetImageSparseMemoryRequirements() has been called for this image
    bool sparse_metadata_required;       // Track if sparse metadata aspect is required for this image
    bool sparse_metadata_bound;          // Track if sparse metadata aspect is bound to this image
    bool has_ahb_format;                 // True if image was created with an external Android format
    bool is_swapchain_image;             // True if image is a swapchain image
    uint64_t ahb_format;                 // External Android format, if provided
    VkImageSubresourceRange full_range;  // The normalized ISR for all levels, layers (slices), and aspects
    VkSwapchainKHR create_from_swapchain;
    VkSwapchainKHR bind_swapchain;
    uint32_t bind_swapchain_imageIndex;
    image_layout_map::Encoder range_encoder;
    VkFormatFeatureFlags format_features = 0;
    // Need to memory requirments for each plane if image is disjoint
    bool disjoint;  // True if image was created with VK_IMAGE_CREATE_DISJOINT_BIT
    VkMemoryRequirements plane0_requirements;
    bool plane0_memory_requirements_checked;
    VkMemoryRequirements plane1_requirements;
    bool plane1_memory_requirements_checked;
    VkMemoryRequirements plane2_requirements;
    bool plane2_memory_requirements_checked;

    const image_layout_map::Encoder subresource_encoder;                             // Subresource resolution encoder
    std::unique_ptr<const subresource_adapter::ImageRangeEncoder> fragment_encoder;  // Fragment resolution encoder
    const VkDevice store_device_as_workaround;                                       // TODO REMOVE WHEN encoder can be const

    std::vector<VkSparseImageMemoryRequirements> sparse_requirements;
    IMAGE_STATE(VkDevice dev, VkImage img, const VkImageCreateInfo *pCreateInfo);
    IMAGE_STATE(IMAGE_STATE const &rh_obj) = delete;

    std::unordered_set<VkImage> aliasing_images;
    bool IsCompatibleAliasing(IMAGE_STATE *other_image_state);

    bool IsCreateInfoEqual(const VkImageCreateInfo &other_createInfo) const;
    bool IsCreateInfoDedicatedAllocationImageAliasingCompatible(const VkImageCreateInfo &other_createInfo) const;

    inline bool IsImageTypeEqual(const VkImageCreateInfo &other_createInfo) const {
        return createInfo.imageType == other_createInfo.imageType;
    }
    inline bool IsFormatEqual(const VkImageCreateInfo &other_createInfo) const {
        return createInfo.format == other_createInfo.format;
    }
    inline bool IsMipLevelsEqual(const VkImageCreateInfo &other_createInfo) const {
        return createInfo.mipLevels == other_createInfo.mipLevels;
    }
    inline bool IsUsageEqual(const VkImageCreateInfo &other_createInfo) const { return createInfo.usage == other_createInfo.usage; }
    inline bool IsSamplesEqual(const VkImageCreateInfo &other_createInfo) const {
        return createInfo.samples == other_createInfo.samples;
    }
    inline bool IsTilingEqual(const VkImageCreateInfo &other_createInfo) const {
        return createInfo.tiling == other_createInfo.tiling;
    }
    inline bool IsArrayLayersEqual(const VkImageCreateInfo &other_createInfo) const {
        return createInfo.arrayLayers == other_createInfo.arrayLayers;
    }
    inline bool IsInitialLayoutEqual(const VkImageCreateInfo &other_createInfo) const {
        return createInfo.initialLayout == other_createInfo.initialLayout;
    }
    inline bool IsSharingModeEqual(const VkImageCreateInfo &other_createInfo) const {
        return createInfo.sharingMode == other_createInfo.sharingMode;
    }
    inline bool IsExtentEqual(const VkImageCreateInfo &other_createInfo) const {
        return (createInfo.extent.width == other_createInfo.extent.width) &&
               (createInfo.extent.height == other_createInfo.extent.height) &&
               (createInfo.extent.depth == other_createInfo.extent.depth);
    }
    inline bool IsQueueFamilyIndicesEqual(const VkImageCreateInfo &other_createInfo) const {
        return (createInfo.queueFamilyIndexCount == other_createInfo.queueFamilyIndexCount) &&
               (createInfo.queueFamilyIndexCount == 0 ||
                memcmp(createInfo.pQueueFamilyIndices, other_createInfo.pQueueFamilyIndices,
                       createInfo.queueFamilyIndexCount * sizeof(createInfo.pQueueFamilyIndices[0])) == 0);
    }

    ~IMAGE_STATE() {
        if ((createInfo.sharingMode == VK_SHARING_MODE_CONCURRENT) && (createInfo.queueFamilyIndexCount > 0)) {
            delete[] createInfo.pQueueFamilyIndices;
            createInfo.pQueueFamilyIndices = nullptr;
        }
    };
};

class IMAGE_VIEW_STATE : public BASE_NODE {
  public:
    VkImageView image_view;
    VkImageViewCreateInfo create_info;
    const VkImageSubresourceRange normalized_subresource_range;
    const image_layout_map::RangeGenerator range_generator;
    VkSampleCountFlagBits samples;
    unsigned descriptor_format_bits;
    VkSamplerYcbcrConversion samplerConversion;  // Handle of the ycbcr sampler conversion the image was created with, if any
    VkFilterCubicImageViewImageFormatPropertiesEXT filter_cubic_props;
    VkFormatFeatureFlags format_features;
    VkImageUsageFlags inherited_usage;  // from spec #resources-image-inherited-usage
    std::shared_ptr<IMAGE_STATE> image_state;
    IMAGE_VIEW_STATE(const std::shared_ptr<IMAGE_STATE> &image_state, VkImageView iv, const VkImageViewCreateInfo *ci);
    IMAGE_VIEW_STATE(const IMAGE_VIEW_STATE &rh_obj) = delete;

    bool OverlapSubresource(const IMAGE_VIEW_STATE &compare_view) const;
};

class ACCELERATION_STRUCTURE_STATE : public BINDABLE {
  public:
    VkAccelerationStructureNV acceleration_structure;
    safe_VkAccelerationStructureCreateInfoNV create_infoNV = {};
    safe_VkAccelerationStructureInfoNV build_info;
    bool memory_requirements_checked = false;
    VkMemoryRequirements2 memory_requirements;
    bool build_scratch_memory_requirements_checked = false;
    VkMemoryRequirements2 build_scratch_memory_requirements;
    bool update_scratch_memory_requirements_checked = false;
    VkMemoryRequirements2 update_scratch_memory_requirements;
    bool built = false;
    uint64_t opaque_handle = 0;
    const VkAllocationCallbacks *allocator = NULL;
    ACCELERATION_STRUCTURE_STATE(VkAccelerationStructureNV as, const VkAccelerationStructureCreateInfoNV *ci)
        : acceleration_structure(as),
          create_infoNV(ci),
          memory_requirements{},
          build_scratch_memory_requirements_checked{},
          build_scratch_memory_requirements{},
          update_scratch_memory_requirements_checked{},
          update_scratch_memory_requirements{} {}
    ACCELERATION_STRUCTURE_STATE(const ACCELERATION_STRUCTURE_STATE &rh_obj) = delete;
};

class ACCELERATION_STRUCTURE_STATE_KHR : public BINDABLE {
  public:
    VkAccelerationStructureKHR acceleration_structure;
    safe_VkAccelerationStructureCreateInfoKHR create_infoKHR = {};
    safe_VkAccelerationStructureBuildGeometryInfoKHR build_info_khr;
    bool memory_requirements_checked = false;
    VkMemoryRequirements2 memory_requirements;
    bool build_scratch_memory_requirements_checked = false;
    VkMemoryRequirements2 build_scratch_memory_requirements;
    bool update_scratch_memory_requirements_checked = false;
    VkMemoryRequirements2 update_scratch_memory_requirements;
    bool built = false;
    uint64_t opaque_handle = 0;
    const VkAllocationCallbacks *allocator = NULL;
    ACCELERATION_STRUCTURE_STATE_KHR(VkAccelerationStructureKHR as, const VkAccelerationStructureCreateInfoKHR *ci)
        : acceleration_structure(as),
          create_infoKHR(ci),
          memory_requirements{},
          build_scratch_memory_requirements_checked{},
          build_scratch_memory_requirements{},
          update_scratch_memory_requirements_checked{},
          update_scratch_memory_requirements{} {}
    ACCELERATION_STRUCTURE_STATE_KHR(const ACCELERATION_STRUCTURE_STATE_KHR &rh_obj) = delete;
};

struct SWAPCHAIN_IMAGE {
    VkImage image;
    std::unordered_set<VkImage> bound_images;
};

class SWAPCHAIN_NODE : public BASE_NODE {
  public:
    safe_VkSwapchainCreateInfoKHR createInfo;
    VkSwapchainKHR swapchain;
    std::vector<SWAPCHAIN_IMAGE> images;
    bool retired = false;
    bool shared_presentable = false;
    uint32_t get_swapchain_image_count = 0;
    SWAPCHAIN_NODE(const VkSwapchainCreateInfoKHR *pCreateInfo, VkSwapchainKHR swapchain)
        : createInfo(pCreateInfo), swapchain(swapchain) {}
};

// Store the DAG.
struct DAGNode {
    uint32_t pass;
    std::vector<uint32_t> prev;
    std::vector<uint32_t> next;
};

struct SubpassDependencyGraphNode {
    uint32_t pass;
    struct Dependency {
        const VkSubpassDependency2 *dependency;
        const SubpassDependencyGraphNode *node;
        Dependency() = default;
        Dependency(const VkSubpassDependency2 *dependency_, const SubpassDependencyGraphNode *node_)
            : dependency(dependency_), node(node_) {}
    };
    std::map<const SubpassDependencyGraphNode *, std::vector<const VkSubpassDependency2 *>> prev;
    std::map<const SubpassDependencyGraphNode *, std::vector<const VkSubpassDependency2 *>> next;
    std::vector<uint32_t> async;  // asynchronous subpasses with a lower subpass index

    std::vector<const VkSubpassDependency2 *> barrier_from_external;
    std::vector<const VkSubpassDependency2 *> barrier_to_external;
    std::unique_ptr<VkSubpassDependency2> implicit_barrier_from_external;
    std::unique_ptr<VkSubpassDependency2> implicit_barrier_to_external;
};

struct RENDER_PASS_STATE : public BASE_NODE {
    struct AttachmentTransition {
        uint32_t prev_pass;
        uint32_t attachment;
        VkImageLayout old_layout;
        VkImageLayout new_layout;
        AttachmentTransition(uint32_t prev_pass_, uint32_t attachment_, VkImageLayout old_layout_, VkImageLayout new_layout_)
            : prev_pass(prev_pass_), attachment(attachment_), old_layout(old_layout_), new_layout(new_layout_) {}
    };

    VkRenderPass renderPass;
    safe_VkRenderPassCreateInfo2 createInfo;
    std::vector<std::vector<uint32_t>> self_dependencies;
    std::vector<DAGNode> subpassToNode;
    std::unordered_map<uint32_t, bool> attachment_first_read;
    std::vector<uint32_t> attachment_first_subpass;
    std::vector<uint32_t> attachment_last_subpass;
    std::vector<bool> attachment_first_is_transition;
    std::vector<SubpassDependencyGraphNode> subpass_dependencies;
    std::vector<std::vector<AttachmentTransition>> subpass_transitions;

    RENDER_PASS_STATE(VkRenderPassCreateInfo2 const *pCreateInfo) : createInfo(pCreateInfo) {}
    RENDER_PASS_STATE(VkRenderPassCreateInfo const *pCreateInfo) {
        ConvertVkRenderPassCreateInfoToV2KHR(*pCreateInfo, &createInfo);
    }
};

// Autogenerated as part of the vk_validation_error_message.h codegen
const char *CommandTypeString(CMD_TYPE type);

enum CB_STATE {
    CB_NEW,                 // Newly created CB w/o any cmds
    CB_RECORDING,           // BeginCB has been called on this CB
    CB_RECORDED,            // EndCB has been called on this CB
    CB_INVALID_COMPLETE,    // had a complete recording, but was since invalidated
    CB_INVALID_INCOMPLETE,  // fouled before recording was completed
};

// CB Status -- used to track status of various bindings on cmd buffer objects
typedef VkFlags CBStatusFlags;
enum CBStatusFlagBits {
    // clang-format off
    CBSTATUS_NONE                            = 0x00000000,   // No status is set
    CBSTATUS_LINE_WIDTH_SET                  = 0x00000001,   // Line width has been set
    CBSTATUS_DEPTH_BIAS_SET                  = 0x00000002,   // Depth bias has been set
    CBSTATUS_BLEND_CONSTANTS_SET             = 0x00000004,   // Blend constants state has been set
    CBSTATUS_DEPTH_BOUNDS_SET                = 0x00000008,   // Depth bounds state object has been set
    CBSTATUS_STENCIL_READ_MASK_SET           = 0x00000010,   // Stencil read mask has been set
    CBSTATUS_STENCIL_WRITE_MASK_SET          = 0x00000020,   // Stencil write mask has been set
    CBSTATUS_STENCIL_REFERENCE_SET           = 0x00000040,   // Stencil reference has been set
    CBSTATUS_VIEWPORT_SET                    = 0x00000080,
    CBSTATUS_SCISSOR_SET                     = 0x00000100,
    CBSTATUS_INDEX_BUFFER_BOUND              = 0x00000200,   // Index buffer has been set
    CBSTATUS_EXCLUSIVE_SCISSOR_SET           = 0x00000400,
    CBSTATUS_SHADING_RATE_PALETTE_SET        = 0x00000800,
    CBSTATUS_LINE_STIPPLE_SET                = 0x00001000,
    CBSTATUS_VIEWPORT_W_SCALING_SET          = 0x00002000,
    CBSTATUS_CULL_MODE_SET                   = 0x00004000,
    CBSTATUS_FRONT_FACE_SET                  = 0x00008000,
    CBSTATUS_PRIMITIVE_TOPOLOGY_SET          = 0x00010000,
    CBSTATUS_VIEWPORT_WITH_COUNT_SET         = 0x00020000,
    CBSTATUS_SCISSOR_WITH_COUNT_SET          = 0x00040000,
    CBSTATUS_VERTEX_INPUT_BINDING_STRIDE_SET = 0x00080000,
    CBSTATUS_DEPTH_TEST_ENABLE_SET           = 0x00100000,
    CBSTATUS_DEPTH_WRITE_ENABLE_SET          = 0x00200000,
    CBSTATUS_DEPTH_COMPARE_OP_SET            = 0x00400000,
    CBSTATUS_DEPTH_BOUNDS_TEST_ENABLE_SET    = 0x00800000,
    CBSTATUS_STENCIL_TEST_ENABLE_SET         = 0x01000000,
    CBSTATUS_STENCIL_OP_SET                  = 0x02000000,
    CBSTATUS_DISCARD_RECTANGLE_SET           = 0x04000000,
    CBSTATUS_SAMPLE_LOCATIONS_SET            = 0x08000000,
    CBSTATUS_COARSE_SAMPLE_ORDER_SET         = 0x10000000,
    CBSTATUS_ALL_STATE_SET                   = 0x1FFFFDFF,   // All state set (intentionally exclude index buffer)
    // clang-format on
};

VkDynamicState ConvertToDynamicState(CBStatusFlagBits flag);
CBStatusFlagBits ConvertToCBStatusFlagBits(VkDynamicState state);
std::string DynamicStateString(CBStatusFlags input_value);

struct QueryObject {
    VkQueryPool pool;
    uint32_t query;
    // These next two fields are *not* used in hash or comparison, they are effectively a data payload
    uint32_t index;  // must be zero if !indexed
    uint32_t perf_pass;
    bool indexed;
    // Command index in the command buffer where the end of the query was
    // recorded (equal to the number of commands in the command buffer before
    // the end of the query).
    uint64_t endCommandIndex;

    QueryObject(VkQueryPool pool_, uint32_t query_)
        : pool(pool_), query(query_), index(0), perf_pass(0), indexed(false), endCommandIndex(0) {}
    QueryObject(VkQueryPool pool_, uint32_t query_, uint32_t index_)
        : pool(pool_), query(query_), index(index_), perf_pass(0), indexed(true), endCommandIndex(0) {}
    QueryObject(const QueryObject &obj)
        : pool(obj.pool),
          query(obj.query),
          index(obj.index),
          perf_pass(obj.perf_pass),
          indexed(obj.indexed),
          endCommandIndex(obj.endCommandIndex) {}
    QueryObject(const QueryObject &obj, uint32_t perf_pass_)
        : pool(obj.pool),
          query(obj.query),
          index(obj.index),
          perf_pass(perf_pass_),
          indexed(obj.indexed),
          endCommandIndex(obj.endCommandIndex) {}
    bool operator<(const QueryObject &rhs) const {
        return (pool == rhs.pool) ? ((query == rhs.query) ? (perf_pass < rhs.perf_pass) : (query < rhs.query)) : pool < rhs.pool;
    }
};

inline bool operator==(const QueryObject &query1, const QueryObject &query2) {
    return ((query1.pool == query2.pool) && (query1.query == query2.query) && (query1.perf_pass == query2.perf_pass));
}

enum QueryState {
    QUERYSTATE_UNKNOWN,    // Initial state.
    QUERYSTATE_RESET,      // After resetting.
    QUERYSTATE_RUNNING,    // Query running.
    QUERYSTATE_ENDED,      // Query ended but results may not be available.
    QUERYSTATE_AVAILABLE,  // Results available.
};

enum QueryResultType {
    QUERYRESULT_UNKNOWN,
    QUERYRESULT_NO_DATA,
    QUERYRESULT_SOME_DATA,
    QUERYRESULT_WAIT_ON_RESET,
    QUERYRESULT_WAIT_ON_RUNNING,
};

inline const char *string_QueryResultType(QueryResultType result_type) {
    switch (result_type) {
        case QUERYRESULT_UNKNOWN:
            return "query may be in an unknown state";
        case QUERYRESULT_NO_DATA:
            return "query may return no data";
        case QUERYRESULT_SOME_DATA:
            return "query will return some data or availability bit";
        case QUERYRESULT_WAIT_ON_RESET:
            return "waiting on a query that has been reset and not issued yet";
        case QUERYRESULT_WAIT_ON_RUNNING:
            return "waiting on a query that has not ended yet";
    }
    assert(false);
    return "UNKNOWN QUERY STATE";  // Unreachable.
}

namespace std {
template <>
struct hash<QueryObject> {
    size_t operator()(QueryObject query) const throw() {
        return hash<uint64_t>()((uint64_t)(query.pool)) ^
               hash<uint64_t>()(static_cast<uint64_t>(query.query) | (static_cast<uint64_t>(query.perf_pass) << 32));
    }
};

}  // namespace std

struct CBVertexBufferBindingInfo {
    std::vector<BufferBinding> vertex_buffer_bindings;
};

static inline bool operator==(const VkImageSubresource &lhs, const VkImageSubresource &rhs) {
    bool is_equal = (lhs.aspectMask == rhs.aspectMask) && (lhs.mipLevel == rhs.mipLevel) && (lhs.arrayLayer == rhs.arrayLayer);
    return is_equal;
}

// Canonical dictionary for PushConstantRanges
using PushConstantRangesDict = hash_util::Dictionary<PushConstantRanges>;
using PushConstantRangesId = PushConstantRangesDict::Id;

// Canonical dictionary for the pipeline layout's layout of descriptorsetlayouts
using DescriptorSetLayoutDef = cvdescriptorset::DescriptorSetLayoutDef;
using DescriptorSetLayoutId = std::shared_ptr<const DescriptorSetLayoutDef>;
using PipelineLayoutSetLayoutsDef = std::vector<DescriptorSetLayoutId>;
using PipelineLayoutSetLayoutsDict =
    hash_util::Dictionary<PipelineLayoutSetLayoutsDef, hash_util::IsOrderedContainer<PipelineLayoutSetLayoutsDef>>;
using PipelineLayoutSetLayoutsId = PipelineLayoutSetLayoutsDict::Id;

// Defines/stores a compatibility defintion for set N
// The "layout layout" must store at least set+1 entries, but only the first set+1 are considered for hash and equality testing
// Note: the "cannonical" data are referenced by Id, not including handle or device specific state
// Note: hash and equality only consider layout_id entries [0, set] for determining uniqueness
struct PipelineLayoutCompatDef {
    uint32_t set;
    PushConstantRangesId push_constant_ranges;
    PipelineLayoutSetLayoutsId set_layouts_id;
    PipelineLayoutCompatDef(const uint32_t set_index, const PushConstantRangesId pcr_id, const PipelineLayoutSetLayoutsId sl_id)
        : set(set_index), push_constant_ranges(pcr_id), set_layouts_id(sl_id) {}
    size_t hash() const;
    bool operator==(const PipelineLayoutCompatDef &other) const;
};

// Canonical dictionary for PipelineLayoutCompat records
using PipelineLayoutCompatDict = hash_util::Dictionary<PipelineLayoutCompatDef, hash_util::HasHashMember<PipelineLayoutCompatDef>>;
using PipelineLayoutCompatId = PipelineLayoutCompatDict::Id;

// Store layouts and pushconstants for PipelineLayout
struct PIPELINE_LAYOUT_STATE : public BASE_NODE {
    VkPipelineLayout layout;
    std::vector<std::shared_ptr<cvdescriptorset::DescriptorSetLayout const>> set_layouts;
    PushConstantRangesId push_constant_ranges;
    std::vector<PipelineLayoutCompatId> compat_for_set;

    PIPELINE_LAYOUT_STATE() : layout(VK_NULL_HANDLE), set_layouts{}, push_constant_ranges{}, compat_for_set{} {}

    void reset() {
        layout = VK_NULL_HANDLE;
        set_layouts.clear();
        push_constant_ranges.reset();
        compat_for_set.clear();
    }
};

// Shader typedefs needed to store StageStage below
struct interface_var {
    uint32_t id;
    uint32_t type_id;
    uint32_t offset;

    std::vector<std::set<SamplerUsedByImage>> samplers_used_by_image;  // List of samplers that sample a given image.
                                                                       // The index of array is index of image.

    bool is_patch;
    bool is_block_member;
    bool is_relaxed_precision;
    bool is_writable;
    bool is_atomic_operation;
    bool is_sampler_implicitLod_dref_proj;
    bool is_sampler_bias_offset;
    // TODO: collect the name, too? Isn't required to be present.

    interface_var()
        : id(0),
          type_id(0),
          offset(0),
          is_patch(false),
          is_block_member(false),
          is_relaxed_precision(false),
          is_writable(false),
          is_atomic_operation(false),
          is_sampler_implicitLod_dref_proj(false),
          is_sampler_bias_offset(false) {}
};

// Safe struct that spans NV and KHR VkRayTracingPipelineCreateInfo structures.
// It is a safe_VkRayTracingPipelineCreateInfoKHR and supports construction from
// a VkRayTracingPipelineCreateInfoNV.
class safe_VkRayTracingPipelineCreateInfoCommon : public safe_VkRayTracingPipelineCreateInfoKHR {
  public:
    safe_VkRayTracingPipelineCreateInfoCommon() : safe_VkRayTracingPipelineCreateInfoKHR() {}
    safe_VkRayTracingPipelineCreateInfoCommon(const VkRayTracingPipelineCreateInfoNV *pCreateInfo)
        : safe_VkRayTracingPipelineCreateInfoKHR() {
        initialize(pCreateInfo);
    }
    void initialize(const VkRayTracingPipelineCreateInfoNV *pCreateInfo) {
        safe_VkRayTracingPipelineCreateInfoNV nvStruct;
        nvStruct.initialize(pCreateInfo);

        sType = nvStruct.sType;

        // Take ownership of the pointer and null it out in nvStruct
        pNext = nvStruct.pNext;
        nvStruct.pNext = nullptr;

        flags = nvStruct.flags;
        stageCount = nvStruct.stageCount;

        pStages = nvStruct.pStages;
        nvStruct.pStages = nullptr;

        groupCount = nvStruct.groupCount;
        maxRecursionDepth = nvStruct.maxRecursionDepth;
        layout = nvStruct.layout;
        basePipelineHandle = nvStruct.basePipelineHandle;
        basePipelineIndex = nvStruct.basePipelineIndex;

        assert(pGroups == nullptr);
        if (nvStruct.groupCount && nvStruct.pGroups) {
            pGroups = new safe_VkRayTracingShaderGroupCreateInfoKHR[groupCount];
            for (uint32_t i = 0; i < groupCount; ++i) {
                pGroups[i].sType = nvStruct.pGroups[i].sType;
                pGroups[i].pNext = nvStruct.pGroups[i].pNext;
                pGroups[i].type = nvStruct.pGroups[i].type;
                pGroups[i].generalShader = nvStruct.pGroups[i].generalShader;
                pGroups[i].closestHitShader = nvStruct.pGroups[i].closestHitShader;
                pGroups[i].anyHitShader = nvStruct.pGroups[i].anyHitShader;
                pGroups[i].intersectionShader = nvStruct.pGroups[i].intersectionShader;
                pGroups[i].intersectionShader = nvStruct.pGroups[i].intersectionShader;
                pGroups[i].pShaderGroupCaptureReplayHandle = nullptr;
            }
        }
    }
    void initialize(const VkRayTracingPipelineCreateInfoKHR *pCreateInfo) {
        safe_VkRayTracingPipelineCreateInfoKHR::initialize(pCreateInfo);
    }
    uint32_t maxRecursionDepth;  // NV specific
};

struct SHADER_MODULE_STATE;
class PIPELINE_STATE : public BASE_NODE {
  public:
    struct StageState {
        std::unordered_set<uint32_t> accessible_ids;
        std::vector<std::pair<descriptor_slot_t, interface_var>> descriptor_uses;
        bool has_writable_descriptor;
        bool has_atomic_descriptor;
        VkShaderStageFlagBits stage_flag;
        std::string entry_point_name;
        std::shared_ptr<const SHADER_MODULE_STATE> shader_state;
    };

    VkPipeline pipeline;
    safe_VkGraphicsPipelineCreateInfo graphicsPipelineCI;
    safe_VkComputePipelineCreateInfo computePipelineCI;
    safe_VkRayTracingPipelineCreateInfoCommon raytracingPipelineCI;
    // Hold shared ptr to RP in case RP itself is destroyed
    std::shared_ptr<const RENDER_PASS_STATE> rp_state;
    // Flag of which shader stages are active for this pipeline
    uint32_t active_shaders;
    uint32_t duplicate_shaders;
    // Capture which slots (set#->bindings) are actually used by the shaders of this pipeline
    std::unordered_map<uint32_t, BindingReqMap> active_slots;
    uint32_t max_active_slot;  // the highest set number in active_slots for pipeline layout compatibility checks
    // Additional metadata needed by pipeline_state initialization and validation
    std::vector<StageState> stage_state;
    std::unordered_set<uint32_t> fragmentShader_writable_output_location_list;
    // Vtx input info (if any)
    std::vector<VkVertexInputBindingDescription> vertex_binding_descriptions_;
    std::vector<VkVertexInputAttributeDescription> vertex_attribute_descriptions_;
    std::vector<VkDeviceSize> vertex_attribute_alignments_;
    std::unordered_map<uint32_t, uint32_t> vertex_binding_to_index_map_;
    std::vector<VkPipelineColorBlendAttachmentState> attachments;
    std::unordered_set<VkShaderStageFlagBits, hash_util::HashCombiner::WrappedHash<VkShaderStageFlagBits>>
        wrote_primitive_shading_rate;
    bool blendConstantsEnabled;  // Blend constants enabled for any attachments
    std::shared_ptr<const PIPELINE_LAYOUT_STATE> pipeline_layout;
    VkPrimitiveTopology topology_at_rasterizer;
    VkBool32 sample_location_enabled;

    // Default constructor
    PIPELINE_STATE()
        : pipeline{},
          graphicsPipelineCI{},
          computePipelineCI{},
          raytracingPipelineCI{},
          rp_state(nullptr),
          active_shaders(0),
          duplicate_shaders(0),
          active_slots(),
          max_active_slot(0),
          vertex_binding_descriptions_(),
          vertex_attribute_descriptions_(),
          vertex_binding_to_index_map_(),
          attachments(),
          blendConstantsEnabled(false),
          pipeline_layout(),
          topology_at_rasterizer{},
          sample_location_enabled(VK_FALSE) {}

    void reset() {
        VkGraphicsPipelineCreateInfo emptyGraphicsCI = {};
        graphicsPipelineCI.initialize(&emptyGraphicsCI, false, false);
        VkComputePipelineCreateInfo emptyComputeCI = {};
        computePipelineCI.initialize(&emptyComputeCI);
        VkRayTracingPipelineCreateInfoKHR emptyRayTracingCI = {};
        raytracingPipelineCI.initialize(&emptyRayTracingCI);
        stage_state.clear();
        fragmentShader_writable_output_location_list.clear();
    }

    void initGraphicsPipeline(const ValidationStateTracker *state_data, const VkGraphicsPipelineCreateInfo *pCreateInfo,
                              std::shared_ptr<const RENDER_PASS_STATE> &&rpstate);
    void initComputePipeline(const ValidationStateTracker *state_data, const VkComputePipelineCreateInfo *pCreateInfo);

    template <typename CreateInfo>
    void initRayTracingPipeline(const ValidationStateTracker *state_data, const CreateInfo *pCreateInfo);

    inline VkPipelineBindPoint getPipelineType() const {
        if (graphicsPipelineCI.sType == VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO)
            return VK_PIPELINE_BIND_POINT_GRAPHICS;
        else if (computePipelineCI.sType == VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO)
            return VK_PIPELINE_BIND_POINT_COMPUTE;
        else if (raytracingPipelineCI.sType == VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV)
            return VK_PIPELINE_BIND_POINT_RAY_TRACING_NV;
        else if (raytracingPipelineCI.sType == VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR)
            return VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR;
        else
            return VK_PIPELINE_BIND_POINT_MAX_ENUM;
    }

    inline VkPipelineCreateFlags getPipelineCreateFlags() const {
        if (graphicsPipelineCI.sType == VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO)
            return graphicsPipelineCI.flags;
        else if (computePipelineCI.sType == VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO)
            return computePipelineCI.flags;
        else if ((raytracingPipelineCI.sType == VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV) ||
                 (raytracingPipelineCI.sType == VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR))
            return raytracingPipelineCI.flags;
        else
            return 0;
    }
};

// Track last states that are bound per pipeline bind point (Gfx & Compute)
struct LAST_BOUND_STATE {
    LAST_BOUND_STATE() { reset(); }  // must define default constructor for portability reasons
    PIPELINE_STATE *pipeline_state;
    VkPipelineLayout pipeline_layout;
    std::unique_ptr<cvdescriptorset::DescriptorSet> push_descriptor_set;

    // Ordered bound set tracking where index is set# that given set is bound to
    struct PER_SET {
        PER_SET()
            : bound_descriptor_set(nullptr),
              compat_id_for_set(0),
              validated_set(nullptr),
              validated_set_change_count(~0ULL),
              validated_set_image_layout_change_count(~0ULL),
              validated_set_binding_req_map() {}

        cvdescriptorset::DescriptorSet *bound_descriptor_set;
        // one dynamic offset per dynamic descriptor bound to this CB
        std::vector<uint32_t> dynamicOffsets;
        PipelineLayoutCompatId compat_id_for_set;

        // Cache most recently validated descriptor state for ValidateCmdBufDrawState/UpdateDrawState
        const cvdescriptorset::DescriptorSet *validated_set;
        uint64_t validated_set_change_count;
        uint64_t validated_set_image_layout_change_count;
        BindingReqMap validated_set_binding_req_map;
    };

    std::vector<PER_SET> per_set;

    void reset() {
        pipeline_state = nullptr;
        pipeline_layout = VK_NULL_HANDLE;
        push_descriptor_set = nullptr;
        per_set.clear();
    }

    void UnbindAndResetPushDescriptorSet(cvdescriptorset::DescriptorSet *ds) {
        if (push_descriptor_set) {
            for (std::size_t i = 0; i < per_set.size(); i++) {
                if (per_set[i].bound_descriptor_set == push_descriptor_set.get()) {
                    per_set[i].bound_descriptor_set = nullptr;
                }
            }
        }
        push_descriptor_set.reset(ds);
    }

    inline bool IsUsing() const { return pipeline_state ? true : false; }
};

static inline bool CompatForSet(uint32_t set, const LAST_BOUND_STATE &a, const std::vector<PipelineLayoutCompatId> &b) {
    bool result = (set < a.per_set.size()) && (set < b.size()) && (a.per_set[set].compat_id_for_set == b[set]);
    return result;
}

static inline bool CompatForSet(uint32_t set, const PIPELINE_LAYOUT_STATE *a, const PIPELINE_LAYOUT_STATE *b) {
    // Intentionally have a result variable to simplify debugging
    bool result = a && b && (set < a->compat_for_set.size()) && (set < b->compat_for_set.size()) &&
                  (a->compat_for_set[set] == b->compat_for_set[set]);
    return result;
}

// Types to store queue family ownership (QFO) Transfers

// Common to image and buffer memory barriers
template <typename Handle>
struct QFOTransferBarrierBase {
    using HandleType = Handle;
    Handle handle = VK_NULL_HANDLE;
    uint32_t srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    uint32_t dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    QFOTransferBarrierBase() = default;
    QFOTransferBarrierBase(const Handle &resource_handle, uint32_t src, uint32_t dst)
        : handle(resource_handle), srcQueueFamilyIndex(src), dstQueueFamilyIndex(dst) {}

    hash_util::HashCombiner base_hash_combiner() const {
        hash_util::HashCombiner hc;
        hc << srcQueueFamilyIndex << dstQueueFamilyIndex << handle;
        return hc;
    }

    bool operator==(const QFOTransferBarrierBase<Handle> &rhs) const {
        return (srcQueueFamilyIndex == rhs.srcQueueFamilyIndex) && (dstQueueFamilyIndex == rhs.dstQueueFamilyIndex) &&
               (handle == rhs.handle);
    }
};

// Image barrier specific implementation
struct QFOImageTransferBarrier : public QFOTransferBarrierBase<VkImage> {
    using BaseType = QFOTransferBarrierBase<VkImage>;
    VkImageLayout oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkImageLayout newLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkImageSubresourceRange subresourceRange;

    QFOImageTransferBarrier() = default;
    QFOImageTransferBarrier(const VkImageMemoryBarrier &barrier)
        : BaseType(barrier.image, barrier.srcQueueFamilyIndex, barrier.dstQueueFamilyIndex),
          oldLayout(barrier.oldLayout),
          newLayout(barrier.newLayout),
          subresourceRange(barrier.subresourceRange) {}
    QFOImageTransferBarrier(const VkImageMemoryBarrier2KHR &barrier)
        : BaseType(barrier.image, barrier.srcQueueFamilyIndex, barrier.dstQueueFamilyIndex),
          oldLayout(barrier.oldLayout),
          newLayout(barrier.newLayout),
          subresourceRange(barrier.subresourceRange) {}
    size_t hash() const {
        // Ignoring the layout information for the purpose of the hash, as we're interested in QFO release/acquisition w.r.t.
        // the subresource affected, an layout transitions are current validated on another path
        auto hc = base_hash_combiner() << subresourceRange;
        return hc.Value();
    }
    bool operator==(const QFOImageTransferBarrier &rhs) const {
        // Ignoring layout w.r.t. equality. See comment in hash above.
        return (static_cast<BaseType>(*this) == static_cast<BaseType>(rhs)) && (subresourceRange == rhs.subresourceRange);
    }
    // TODO: codegen a comprehensive complie time type -> string (and or other traits) template family
    static const char *BarrierName() { return "VkImageMemoryBarrier"; }
    static const char *HandleName() { return "VkImage"; }
    // UNASSIGNED-VkImageMemoryBarrier-image-00001 QFO transfer image barrier must not duplicate QFO recorded in command buffer
    static const char *ErrMsgDuplicateQFOInCB() { return "UNASSIGNED-VkImageMemoryBarrier-image-00001"; }
    // UNASSIGNED-VkImageMemoryBarrier-image-00002 QFO transfer image barrier must not duplicate QFO submitted in batch
    static const char *ErrMsgDuplicateQFOInSubmit() { return "UNASSIGNED-VkImageMemoryBarrier-image-00002"; }
    // UNASSIGNED-VkImageMemoryBarrier-image-00003 QFO transfer image barrier must not duplicate QFO submitted previously
    static const char *ErrMsgDuplicateQFOSubmitted() { return "UNASSIGNED-VkImageMemoryBarrier-image-00003"; }
    // UNASSIGNED-VkImageMemoryBarrier-image-00004 QFO acquire image barrier must have matching QFO release submitted previously
    static const char *ErrMsgMissingQFOReleaseInSubmit() { return "UNASSIGNED-VkImageMemoryBarrier-image-00004"; }
};

// Buffer barrier specific implementation
struct QFOBufferTransferBarrier : public QFOTransferBarrierBase<VkBuffer> {
    using BaseType = QFOTransferBarrierBase<VkBuffer>;
    VkDeviceSize offset = 0;
    VkDeviceSize size = 0;
    QFOBufferTransferBarrier() = default;
    QFOBufferTransferBarrier(const VkBufferMemoryBarrier &barrier)
        : BaseType(barrier.buffer, barrier.srcQueueFamilyIndex, barrier.dstQueueFamilyIndex),
          offset(barrier.offset),
          size(barrier.size) {}
    QFOBufferTransferBarrier(const VkBufferMemoryBarrier2KHR &barrier)
        : BaseType(barrier.buffer, barrier.srcQueueFamilyIndex, barrier.dstQueueFamilyIndex),
          offset(barrier.offset),
          size(barrier.size) {}
    size_t hash() const {
        auto hc = base_hash_combiner() << offset << size;
        return hc.Value();
    }
    bool operator==(const QFOBufferTransferBarrier &rhs) const {
        return (static_cast<BaseType>(*this) == static_cast<BaseType>(rhs)) && (offset == rhs.offset) && (size == rhs.size);
    }
    static const char *BarrierName() { return "VkBufferMemoryBarrier"; }
    static const char *HandleName() { return "VkBuffer"; }
    // UNASSIGNED-VkImageMemoryBarrier-buffer-00001 QFO transfer buffer barrier must not duplicate QFO recorded in command buffer
    static const char *ErrMsgDuplicateQFOInCB() { return "UNASSIGNED-VkBufferMemoryBarrier-buffer-00001"; }
    // UNASSIGNED-VkBufferMemoryBarrier-buffer-00002 QFO transfer buffer barrier must not duplicate QFO submitted in batch
    static const char *ErrMsgDuplicateQFOInSubmit() { return "UNASSIGNED-VkBufferMemoryBarrier-buffer-00002"; }
    // UNASSIGNED-VkBufferMemoryBarrier-buffer-00003 QFO transfer buffer barrier must not duplicate QFO submitted previously
    static const char *ErrMsgDuplicateQFOSubmitted() { return "UNASSIGNED-VkBufferMemoryBarrier-buffer-00003"; }
    // UNASSIGNED-VkBufferMemoryBarrier-buffer-00004 QFO acquire buffer barrier must have matching QFO release submitted previously
    static const char *ErrMsgMissingQFOReleaseInSubmit() { return "UNASSIGNED-VkBufferMemoryBarrier-buffer-00004"; }
};

template <typename TransferBarrier>
using QFOTransferBarrierHash = hash_util::HasHashMember<TransferBarrier>;

// Command buffers store the set of barriers recorded
template <typename TransferBarrier>
using QFOTransferBarrierSet = std::unordered_set<TransferBarrier, QFOTransferBarrierHash<TransferBarrier>>;

template <typename TransferBarrier>
struct QFOTransferBarrierSets {
    QFOTransferBarrierSet<TransferBarrier> release;
    QFOTransferBarrierSet<TransferBarrier> acquire;
    void Reset() {
        acquire.clear();
        release.clear();
    }
};

// The layer_data stores the map of pending release barriers
template <typename TransferBarrier>
using GlobalQFOTransferBarrierMap =
    std::unordered_map<typename TransferBarrier::HandleType, QFOTransferBarrierSet<TransferBarrier>>;

// Submit queue uses the Scoreboard to track all release/acquire operations in a batch.
template <typename TransferBarrier>
using QFOTransferCBScoreboard =
    std::unordered_map<TransferBarrier, const CMD_BUFFER_STATE *, QFOTransferBarrierHash<TransferBarrier>>;

template <typename TransferBarrier>
struct QFOTransferCBScoreboards {
    QFOTransferCBScoreboard<TransferBarrier> acquire;
    QFOTransferCBScoreboard<TransferBarrier> release;
};

typedef std::map<QueryObject, QueryState> QueryMap;
typedef std::unordered_map<VkEvent, VkPipelineStageFlags2KHR> EventToStageMap;
typedef ImageSubresourceLayoutMap::LayoutMap GlobalImageLayoutRangeMap;
typedef std::unordered_map<VkImage, Optional<GlobalImageLayoutRangeMap>> GlobalImageLayoutMap;

typedef std::unordered_map<VkImage, Optional<ImageSubresourceLayoutMap>> CommandBufferImageLayoutMap;

enum LvlBindPoint {
    BindPoint_Graphics = VK_PIPELINE_BIND_POINT_GRAPHICS,
    BindPoint_Compute = VK_PIPELINE_BIND_POINT_COMPUTE,
    BindPoint_Ray_Tracing = 2,
    BindPoint_Count = 3,
};

static VkPipelineBindPoint inline ConvertToPipelineBindPoint(LvlBindPoint bind_point) {
    switch (bind_point) {
        case BindPoint_Ray_Tracing:
            return VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR;
        default:
            return static_cast<VkPipelineBindPoint>(bind_point);
    }
    return VK_PIPELINE_BIND_POINT_MAX_ENUM;
}

static LvlBindPoint inline ConvertToLvlBindPoint(VkPipelineBindPoint bind_point) {
    switch (bind_point) {
        case VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR:
            return BindPoint_Ray_Tracing;
        default:
            return static_cast<LvlBindPoint>(bind_point);
    }
    return LvlBindPoint(~0U);
}

struct SUBPASS_INFO;
class FRAMEBUFFER_STATE;
// Cmd Buffer Wrapper Struct - TODO : This desperately needs its own class
struct CMD_BUFFER_STATE : public BASE_NODE {
    VkCommandBuffer commandBuffer;
    VkCommandBufferAllocateInfo createInfo = {};
    VkCommandBufferBeginInfo beginInfo;
    VkCommandBufferInheritanceInfo inheritanceInfo;
    std::shared_ptr<const COMMAND_POOL_STATE> command_pool;
    bool hasDrawCmd;
    bool hasTraceRaysCmd;
    bool hasBuildAccelerationStructureCmd;
    bool hasDispatchCmd;
    bool unprotected;  // can't be used for protected memory

    CB_STATE state;         // Track cmd buffer update state
    uint64_t commandCount;  // Number of commands recorded
    uint64_t submitCount;   // Number of times CB has been submitted
    typedef uint64_t ImageLayoutUpdateCount;
    ImageLayoutUpdateCount image_layout_change_count;  // The sequence number for changes to image layout (for cached validation)
    CBStatusFlags status;                              // Track status of various bindings on cmd buffer
    CBStatusFlags static_status;                       // All state bits provided by current graphics pipeline
                                                       // rather than dynamic state
    CBStatusFlags dynamic_status;                      // dynamic state set up in pipeline
    // Currently storing "lastBound" objects on per-CB basis
    //  long-term may want to create caches of "lastBound" states and could have
    //  each individual CMD_NODE referencing its own "lastBound" state
    // Store last bound state for Gfx & Compute pipeline bind points
    std::array<LAST_BOUND_STATE, BindPoint_Count> lastBound;  // index is LvlBindPoint.

    struct CmdDrawDispatchInfo {
        CMD_TYPE cmd_type;
        std::string function;
        std::vector<std::pair<const uint32_t, DescriptorRequirement>> binding_infos;
        VkFramebuffer framebuffer;
        std::shared_ptr<std::vector<SUBPASS_INFO>> subpasses;
        std::shared_ptr<std::vector<IMAGE_VIEW_STATE *>> attachments;
    };
    std::unordered_map<VkDescriptorSet, std::vector<CmdDrawDispatchInfo>> validate_descriptorsets_in_queuesubmit;

    uint32_t viewportMask;
    uint32_t viewportWithCountMask;
    uint32_t viewportWithCountCount;
    uint32_t scissorMask;
    uint32_t scissorWithCountMask;
    uint32_t initial_device_mask;
    VkPrimitiveTopology primitiveTopology;

    safe_VkRenderPassBeginInfo activeRenderPassBeginInfo;
    std::shared_ptr<RENDER_PASS_STATE> activeRenderPass;
    std::shared_ptr<std::vector<SUBPASS_INFO>> active_subpasses;
    std::shared_ptr<std::vector<IMAGE_VIEW_STATE *>> active_attachments;
    std::set<std::shared_ptr<IMAGE_VIEW_STATE>> attachments_view_states;

    VkSubpassContents activeSubpassContents;
    uint32_t active_render_pass_device_mask;
    uint32_t activeSubpass;
    std::shared_ptr<FRAMEBUFFER_STATE> activeFramebuffer;
    std::unordered_set<std::shared_ptr<FRAMEBUFFER_STATE>> framebuffers;
    // Unified data structs to track objects bound to this command buffer as well as object
    //  dependencies that have been broken : either destroyed objects, or updated descriptor sets
    std::vector<VulkanTypedHandle> object_bindings;
    std::vector<VulkanTypedHandle> broken_bindings;

    QFOTransferBarrierSets<QFOBufferTransferBarrier> qfo_transfer_buffer_barriers;
    QFOTransferBarrierSets<QFOImageTransferBarrier> qfo_transfer_image_barriers;

    std::unordered_set<VkEvent> waitedEvents;
    std::vector<VkEvent> writeEventsBeforeWait;
    std::vector<VkEvent> events;
    std::unordered_set<QueryObject> activeQueries;
    std::unordered_set<QueryObject> startedQueries;
    std::unordered_set<QueryObject> resetQueries;
    CommandBufferImageLayoutMap image_layout_map;
    CBVertexBufferBindingInfo current_vertex_buffer_binding_info;
    bool vertex_buffer_used;  // Track for perf warning to make sure any bound vtx buffer used
    VkCommandBuffer primaryCommandBuffer;
    // If primary, the secondary command buffers we will call.
    // If secondary, the primary command buffers we will be called by.
    std::unordered_set<CMD_BUFFER_STATE *> linkedCommandBuffers;
    // Validation functions run at primary CB queue submit time
    std::vector<std::function<bool(const ValidationStateTracker *device_data, const class QUEUE_STATE *queue_state)>>
        queue_submit_functions;
    // Validation functions run when secondary CB is executed in primary
    std::vector<std::function<bool(const CMD_BUFFER_STATE *, const FRAMEBUFFER_STATE *)>> cmd_execute_commands_functions;
    std::vector<
        std::function<bool(const ValidationStateTracker *device_data, bool do_validate, EventToStageMap *localEventToStageMap)>>
        eventUpdates;
    std::vector<std::function<bool(const ValidationStateTracker *device_data, bool do_validate, VkQueryPool &firstPerfQueryPool,
                                   uint32_t perfQueryPass, QueryMap *localQueryToStateMap)>>
        queryUpdates;
    std::unordered_set<cvdescriptorset::DescriptorSet *> validated_descriptor_sets;
    // Contents valid only after an index buffer is bound (CBSTATUS_INDEX_BUFFER_BOUND set)
    IndexBufferBinding index_buffer_binding;
    bool performance_lock_acquired = false;
    bool performance_lock_released = false;

    // Cache of current insert label...
    LoggingLabel debug_label;

    std::vector<uint8_t> push_constant_data;
    PushConstantRangesId push_constant_data_ranges;

    std::map<VkShaderStageFlagBits, std::vector<uint8_t>>
        push_constant_data_update;  // vector's value is enum PushConstantByteState.
    VkPipelineLayout push_constant_pipeline_layout_set;

    // Used for Best Practices tracking
    uint32_t small_indexed_draw_call_count;

    bool transform_feedback_active{false};
};

static inline const QFOTransferBarrierSets<QFOImageTransferBarrier> &GetQFOBarrierSets(const CMD_BUFFER_STATE *cb,
                                                                                       const QFOImageTransferBarrier &type_tag) {
    return cb->qfo_transfer_image_barriers;
}
static inline const QFOTransferBarrierSets<QFOBufferTransferBarrier> &GetQFOBarrierSets(const CMD_BUFFER_STATE *cb,
                                                                                        const QFOBufferTransferBarrier &type_tag) {
    return cb->qfo_transfer_buffer_barriers;
}
static inline QFOTransferBarrierSets<QFOImageTransferBarrier> &GetQFOBarrierSets(CMD_BUFFER_STATE *cb,
                                                                                 const QFOImageTransferBarrier &type_tag) {
    return cb->qfo_transfer_image_barriers;
}
static inline QFOTransferBarrierSets<QFOBufferTransferBarrier> &GetQFOBarrierSets(CMD_BUFFER_STATE *cb,
                                                                                  const QFOBufferTransferBarrier &type_tag) {
    return cb->qfo_transfer_buffer_barriers;
}

struct SEMAPHORE_WAIT {
    VkSemaphore semaphore;
    VkSemaphoreType type;
    VkQueue queue;
    uint64_t payload;
    uint64_t seq;
};

struct SEMAPHORE_SIGNAL {
    VkSemaphore semaphore;
    uint64_t payload;
    uint64_t seq;
};

struct CB_SUBMISSION {
    CB_SUBMISSION()
        : cbs(), waitSemaphores(), signalSemaphores(), externalSemaphores(), fence(VK_NULL_HANDLE), perf_submit_pass(0) {}

    std::vector<VkCommandBuffer> cbs;
    std::vector<SEMAPHORE_WAIT> waitSemaphores;
    std::vector<SEMAPHORE_SIGNAL> signalSemaphores;
    std::vector<VkSemaphore> externalSemaphores;
    VkFence fence;
    uint32_t perf_submit_pass;
};

struct MT_FB_ATTACHMENT_INFO {
    IMAGE_VIEW_STATE *view_state;
    VkImage image;
};

struct SUBPASS_INFO {
    bool used;
    VkImageUsageFlagBits usage;
    VkImageLayout layout;

    SUBPASS_INFO() : used(false), usage(VkImageUsageFlagBits(0)), layout(VK_IMAGE_LAYOUT_UNDEFINED) {}
};

class FRAMEBUFFER_STATE : public BASE_NODE {
  public:
    VkFramebuffer framebuffer;
    safe_VkFramebufferCreateInfo createInfo;
    std::shared_ptr<const RENDER_PASS_STATE> rp_state;
    std::vector<std::shared_ptr<IMAGE_VIEW_STATE>> attachments_view_state;
    FRAMEBUFFER_STATE(VkFramebuffer fb, const VkFramebufferCreateInfo *pCreateInfo, std::shared_ptr<RENDER_PASS_STATE> &&rpstate)
        : framebuffer(fb), createInfo(pCreateInfo), rp_state(rpstate){};
};

struct SHADER_MODULE_STATE;
struct DeviceExtensions;

struct DeviceFeatures {
    VkPhysicalDeviceFeatures core;
    VkPhysicalDeviceVulkan11Features core11;
    VkPhysicalDeviceVulkan12Features core12;

    VkPhysicalDeviceExclusiveScissorFeaturesNV exclusive_scissor;
    VkPhysicalDeviceShadingRateImageFeaturesNV shading_rate_image;
    VkPhysicalDeviceMeshShaderFeaturesNV mesh_shader;
    VkPhysicalDeviceInlineUniformBlockFeaturesEXT inline_uniform_block;
    VkPhysicalDeviceTransformFeedbackFeaturesEXT transform_feedback_features;
    VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT vtx_attrib_divisor_features;
    VkPhysicalDeviceBufferDeviceAddressFeaturesEXT buffer_device_address_ext;
    VkPhysicalDeviceCooperativeMatrixFeaturesNV cooperative_matrix_features;
    VkPhysicalDeviceComputeShaderDerivativesFeaturesNV compute_shader_derivatives_features;
    VkPhysicalDeviceFragmentShaderBarycentricFeaturesNV fragment_shader_barycentric_features;
    VkPhysicalDeviceShaderImageFootprintFeaturesNV shader_image_footprint_features;
    VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT fragment_shader_interlock_features;
    VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT demote_to_helper_invocation_features;
    VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT texel_buffer_alignment_features;
    VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR pipeline_exe_props_features;
    VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV dedicated_allocation_image_aliasing_features;
    VkPhysicalDevicePerformanceQueryFeaturesKHR performance_query_features;
    VkPhysicalDeviceCoherentMemoryFeaturesAMD device_coherent_memory_features;
    VkPhysicalDeviceYcbcrImageArraysFeaturesEXT ycbcr_image_array_features;
    VkPhysicalDeviceRayQueryFeaturesKHR ray_query_features;
    VkPhysicalDeviceRayTracingPipelineFeaturesKHR ray_tracing_pipeline_features;
    VkPhysicalDeviceAccelerationStructureFeaturesKHR ray_tracing_acceleration_structure_features;
    VkPhysicalDeviceRobustness2FeaturesEXT robustness2_features;
    VkPhysicalDeviceFragmentDensityMapFeaturesEXT fragment_density_map_features;
    VkPhysicalDeviceFragmentDensityMap2FeaturesEXT fragment_density_map2_features;
    VkPhysicalDeviceASTCDecodeFeaturesEXT astc_decode_features;
    VkPhysicalDeviceCustomBorderColorFeaturesEXT custom_border_color_features;
    VkPhysicalDevicePipelineCreationCacheControlFeaturesEXT pipeline_creation_cache_control_features;
    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extended_dynamic_state_features;
    VkPhysicalDeviceMultiviewFeatures multiview_features;
    VkPhysicalDevicePortabilitySubsetFeaturesKHR portability_subset_features;
    VkPhysicalDeviceFragmentShadingRateFeaturesKHR fragment_shading_rate_features;
    VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL shader_integer_functions2_features;
    VkPhysicalDeviceShaderSMBuiltinsFeaturesNV shader_sm_builtins_feature;
    VkPhysicalDeviceShaderAtomicFloatFeaturesEXT shader_atomic_float_feature;
    VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT shader_image_atomic_int64_feature;
    VkPhysicalDeviceShaderClockFeaturesKHR shader_clock_feature;
    VkPhysicalDeviceConditionalRenderingFeaturesEXT conditional_rendering;
    VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR workgroup_memory_explicit_layout_features;
    VkPhysicalDeviceSynchronization2FeaturesKHR synchronization2_features;
    // If a new feature is added here that involves a SPIR-V capability add also in spirv_validation_generator.py
    // This is known by checking the table in the spec or if the struct is in a <spirvcapability> in vk.xml
};

enum RenderPassCreateVersion { RENDER_PASS_VERSION_1 = 0, RENDER_PASS_VERSION_2 = 1 };
enum CopyCommandVersion { COPY_COMMAND_VERSION_1 = 0, COPY_COMMAND_VERSION_2 = 1 };
enum CommandVersion { CMD_VERSION_1 = 0, CMD_VERSION_2 = 1 };

enum BarrierOperationsType {
    kAllAcquire,  // All Barrier operations are "ownership acquire" operations
    kAllRelease,  // All Barrier operations are "ownership release" operations
    kGeneral,     // Either no ownership operations or a mix of ownership operation types and/or non-ownership operations
};

ImageSubresourceLayoutMap *GetImageSubresourceLayoutMap(CMD_BUFFER_STATE *cb_state, const IMAGE_STATE &image_state);
const ImageSubresourceLayoutMap *GetImageSubresourceLayoutMap(const CMD_BUFFER_STATE *cb_state, VkImage image);
void AddInitialLayoutintoImageLayoutMap(const IMAGE_STATE &image_state, GlobalImageLayoutMap &image_layout_map);

#endif  // CORE_VALIDATION_TYPES_H_
