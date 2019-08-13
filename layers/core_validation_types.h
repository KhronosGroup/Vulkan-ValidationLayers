/* Copyright (c) 2015-2019 The Khronos Group Inc.
 * Copyright (c) 2015-2019 Valve Corporation
 * Copyright (c) 2015-2019 LunarG, Inc.
 * Copyright (C) 2015-2019 Google Inc.
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

#ifdef VK_USE_PLATFORM_ANDROID_KHR
#include "android_ndk_types.h"
#endif  // VK_USE_PLATFORM_ANDROID_KHR

// Fwd declarations -- including descriptor_set.h creates an ugly include loop
namespace cvdescriptorset {
class DescriptorSetLayoutDef;
class DescriptorSetLayout;
class DescriptorSet;
}  // namespace cvdescriptorset

struct CMD_BUFFER_STATE;
class CoreChecks;
class ValidationStateTracker;

enum CALL_STATE {
    UNCALLED,       // Function has not been called
    QUERY_COUNT,    // Function called once to query a count
    QUERY_DETAILS,  // Function called w/ a count to query details
};

class BASE_NODE {
   public:
    // Track when object is being used by an in-flight command buffer
    std::atomic_int in_use;
    // Track command buffers that this object is bound to
    //  binding initialized when cmd referencing object is bound to command buffer
    //  binding removed when command buffer is reset or destroyed
    // When an object is destroyed, any bound cbs are set to INVALID
    std::unordered_set<CMD_BUFFER_STATE *> cb_bindings;

    BASE_NODE() { in_use.store(0); };
};

// Track command pools and their command buffers
struct COMMAND_POOL_STATE : public BASE_NODE {
    VkCommandPoolCreateFlags createFlags;
    uint32_t queueFamilyIndex;
    // Cmd buffers allocated from this pool
    std::unordered_set<VkCommandBuffer> commandBuffers;
};

// Utilities for barriers and the commmand pool
template <typename Barrier>
static bool IsTransferOp(const Barrier *barrier) {
    return barrier->srcQueueFamilyIndex != barrier->dstQueueFamilyIndex;
}

template <typename Barrier, bool assume_transfer = false>
static bool TempIsReleaseOp(const COMMAND_POOL_STATE *pool, const Barrier *barrier) {
    return (assume_transfer || IsTransferOp(barrier)) && (pool->queueFamilyIndex == barrier->srcQueueFamilyIndex);
}

template <typename Barrier, bool assume_transfer = false>
static bool IsAcquireOp(const COMMAND_POOL_STATE *pool, const Barrier *barrier) {
    return (assume_transfer || IsTransferOp(barrier)) && (pool->queueFamilyIndex == barrier->dstQueueFamilyIndex);
}

inline bool IsSpecial(const uint32_t queue_family_index) {
    return (queue_family_index == VK_QUEUE_FAMILY_EXTERNAL_KHR) || (queue_family_index == VK_QUEUE_FAMILY_FOREIGN_EXT);
}

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

    DESCRIPTOR_REQ_ALL_VIEW_TYPE_BITS = (1 << (VK_IMAGE_VIEW_TYPE_END_RANGE + 1)) - 1,

    DESCRIPTOR_REQ_SINGLE_SAMPLE = 2 << VK_IMAGE_VIEW_TYPE_END_RANGE,
    DESCRIPTOR_REQ_MULTI_SAMPLE = DESCRIPTOR_REQ_SINGLE_SAMPLE << 1,

    DESCRIPTOR_REQ_COMPONENT_TYPE_FLOAT = DESCRIPTOR_REQ_MULTI_SAMPLE << 1,
    DESCRIPTOR_REQ_COMPONENT_TYPE_SINT = DESCRIPTOR_REQ_COMPONENT_TYPE_FLOAT << 1,
    DESCRIPTOR_REQ_COMPONENT_TYPE_UINT = DESCRIPTOR_REQ_COMPONENT_TYPE_SINT << 1,
};

extern unsigned DescriptorRequirementsBitsFromFormat(VkFormat fmt);

typedef std::map<uint32_t, descriptor_req> BindingReqMap;

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

// Generic memory binding struct to track objects bound to objects
struct MEM_BINDING {
    VkDeviceMemory mem;
    VkDeviceSize offset;
    VkDeviceSize size;
};

struct BufferBinding {
    VkBuffer buffer;
    VkDeviceSize size;
    VkDeviceSize offset;
};

struct IndexBufferBinding : BufferBinding {
    VkIndexType index_type;
};

inline bool operator==(MEM_BINDING a, MEM_BINDING b) NOEXCEPT { return a.mem == b.mem && a.offset == b.offset && a.size == b.size; }

namespace std {
template <>
struct hash<MEM_BINDING> {
    size_t operator()(MEM_BINDING mb) const NOEXCEPT {
        auto intermediate = hash<uint64_t>()(reinterpret_cast<uint64_t &>(mb.mem)) ^ hash<uint64_t>()(mb.offset);
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
    // Sparse binding data, initially just tracking MEM_BINDING per mem object
    //  There's more data for sparse bindings so need better long-term solution
    // TODO : Need to update solution to track all sparse binding data
    std::unordered_set<MEM_BINDING> sparse_bindings;

    std::unordered_set<VkDeviceMemory> bound_memory_set_;

    BINDABLE()
        : sparse(false), binding{}, requirements{}, memory_requirements_checked(false), sparse_bindings{}, bound_memory_set_{} {};

    // Update the cached set of memory bindings.
    // Code that changes binding.mem or sparse_bindings must call UpdateBoundMemorySet()
    void UpdateBoundMemorySet() {
        bound_memory_set_.clear();
        if (!sparse) {
            bound_memory_set_.insert(binding.mem);
        } else {
            for (auto sb : sparse_bindings) {
                bound_memory_set_.insert(sb.mem);
            }
        }
    }

    // Return unordered set of memory objects that are bound
    // Instead of creating a set from scratch each query, return the cached one
    const std::unordered_set<VkDeviceMemory> &GetBoundMemory() const { return bound_memory_set_; }
};

class BUFFER_STATE : public BINDABLE {
   public:
    VkBuffer buffer;
    VkBufferCreateInfo createInfo;
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
    BUFFER_VIEW_STATE(VkBufferView bv, const VkBufferViewCreateInfo *ci) : buffer_view(bv), create_info(*ci){};
    BUFFER_VIEW_STATE(const BUFFER_VIEW_STATE &rh_obj) = delete;
};

struct SAMPLER_STATE : public BASE_NODE {
    VkSampler sampler;
    VkSamplerCreateInfo createInfo;
    VkSamplerYcbcrConversion samplerConversion = VK_NULL_HANDLE;

    SAMPLER_STATE(const VkSampler *ps, const VkSamplerCreateInfo *pci) : sampler(*ps), createInfo(*pci) {
        auto *conversionInfo = lvl_find_in_chain<VkSamplerYcbcrConversionInfo>(pci->pNext);
        if (conversionInfo) samplerConversion = conversionInfo->conversion;
    }
};

class IMAGE_STATE : public BINDABLE {
   public:
    VkImage image;
    VkImageCreateInfo createInfo;
    bool valid;               // If this is a swapchain image backing memory track valid here as it doesn't have DEVICE_MEMORY_STATE
    bool acquired;            // If this is a swapchain image, has it been acquired by the app.
    bool shared_presentable;  // True for a front-buffered swapchain image
    bool layout_locked;       // A front-buffered image that has been presented can never have layout transitioned
    bool get_sparse_reqs_called;         // Track if GetImageSparseMemoryRequirements() has been called for this image
    bool sparse_metadata_required;       // Track if sparse metadata aspect is required for this image
    bool sparse_metadata_bound;          // Track if sparse metadata aspect is bound to this image
    bool imported_ahb;                   // True if image was imported from an Android Hardware Buffer
    bool has_ahb_format;                 // True if image was created with an external Android format
    uint64_t ahb_format;                 // External Android format, if provided
    VkImageSubresourceRange full_range;  // The normalized ISR for all levels, layers (slices), and aspects
    VkSwapchainKHR create_from_swapchain;
    VkSwapchainKHR bind_swapchain;
    uint32_t bind_swapchain_imageIndex;

#ifdef VK_USE_PLATFORM_ANDROID_KHR
    uint64_t external_format_android;
#endif  // VK_USE_PLATFORM_ANDROID_KHR

    std::vector<VkSparseImageMemoryRequirements> sparse_requirements;
    IMAGE_STATE(VkImage img, const VkImageCreateInfo *pCreateInfo);
    IMAGE_STATE(IMAGE_STATE const &rh_obj) = delete;

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
    VkImageSubresourceRange normalized_subresource_range;
    VkSampleCountFlagBits samples;
    unsigned descriptor_format_bits;
    VkSamplerYcbcrConversion samplerConversion;  // Handle of the ycbcr sampler conversion the image was created with, if any
    IMAGE_VIEW_STATE(const IMAGE_STATE *image_state, VkImageView iv, const VkImageViewCreateInfo *ci);
    IMAGE_VIEW_STATE(const IMAGE_VIEW_STATE &rh_obj) = delete;
};

class ACCELERATION_STRUCTURE_STATE : public BINDABLE {
   public:
    VkAccelerationStructureNV acceleration_structure;
    safe_VkAccelerationStructureCreateInfoNV create_info;
    bool memory_requirements_checked = false;
    VkMemoryRequirements2KHR memory_requirements;
    bool build_scratch_memory_requirements_checked = false;
    VkMemoryRequirements2KHR build_scratch_memory_requirements;
    bool update_scratch_memory_requirements_checked = false;
    VkMemoryRequirements2KHR update_scratch_memory_requirements;
    bool built = false;
    safe_VkAccelerationStructureInfoNV build_info;
    ACCELERATION_STRUCTURE_STATE(VkAccelerationStructureNV as, const VkAccelerationStructureCreateInfoNV *ci)
        : acceleration_structure(as),
          create_info(ci),
          memory_requirements{},
          build_scratch_memory_requirements_checked{},
          update_scratch_memory_requirements_checked{} {}
    ACCELERATION_STRUCTURE_STATE(const ACCELERATION_STRUCTURE_STATE &rh_obj) = delete;
};

struct MemRange {
    VkDeviceSize offset;
    VkDeviceSize size;
};

// Data struct for tracking memory object
struct DEVICE_MEMORY_STATE : public BASE_NODE {
    void *object;  // Dispatchable object used to create this memory (device of swapchain)
    VkDeviceMemory mem;
    VkMemoryAllocateInfo alloc_info;
    bool is_dedicated;
    VkBuffer dedicated_buffer;
    VkImage dedicated_image;
    bool is_export;
    VkExternalMemoryHandleTypeFlags export_handle_type_flags;
    std::unordered_set<VulkanTypedHandle> obj_bindings;  // objects bound to this memory
    // Convenience vectors of handles to speed up iterating over objects independently
    std::unordered_set<uint64_t> bound_images;
    std::unordered_set<uint64_t> bound_buffers;
    std::unordered_set<uint64_t> bound_acceleration_structures;

    MemRange mem_range;
    void *shadow_copy_base;    // Base of layer's allocation for guard band, data, and alignment space
    void *shadow_copy;         // Pointer to start of guard-band data before mapped region
    uint64_t shadow_pad_size;  // Size of the guard-band data before and after actual data. It MUST be a
                               // multiple of limits.minMemoryMapAlignment
    void *p_driver_data;       // Pointer to application's actual memory

    DEVICE_MEMORY_STATE(void *disp_object, const VkDeviceMemory in_mem, const VkMemoryAllocateInfo *p_alloc_info)
        : object(disp_object),
          mem(in_mem),
          alloc_info(*p_alloc_info),
          is_dedicated(false),
          dedicated_buffer(VK_NULL_HANDLE),
          dedicated_image(VK_NULL_HANDLE),
          is_export(false),
          export_handle_type_flags(0),
          mem_range{},
          shadow_copy_base(0),
          shadow_copy(0),
          shadow_pad_size(0),
          p_driver_data(0){};
};

class SWAPCHAIN_NODE {
   public:
    safe_VkSwapchainCreateInfoKHR createInfo;
    VkSwapchainKHR swapchain;
    std::vector<VkImage> images;
    bool retired = false;
    bool shared_presentable = false;
    CALL_STATE vkGetSwapchainImagesKHRState = UNCALLED;
    uint32_t get_swapchain_image_count = 0;
    SWAPCHAIN_NODE(const VkSwapchainCreateInfoKHR *pCreateInfo, VkSwapchainKHR swapchain)
        : createInfo(pCreateInfo), swapchain(swapchain) {}
};

struct ColorAspectTraits {
    static const uint32_t kAspectCount = 1;
    static int Index(VkImageAspectFlags mask) { return 0; };
    static VkImageAspectFlags AspectMask() { return VK_IMAGE_ASPECT_COLOR_BIT; }
    static const std::array<VkImageAspectFlagBits, kAspectCount> &AspectBits() {
        static std::array<VkImageAspectFlagBits, kAspectCount> kAspectBits{{VK_IMAGE_ASPECT_COLOR_BIT}};
        return kAspectBits;
    }
};

struct DepthAspectTraits {
    static const uint32_t kAspectCount = 1;
    static int Index(VkImageAspectFlags mask) { return 0; };
    static VkImageAspectFlags AspectMask() { return VK_IMAGE_ASPECT_DEPTH_BIT; }
    static const std::array<VkImageAspectFlagBits, kAspectCount> &AspectBits() {
        static std::array<VkImageAspectFlagBits, kAspectCount> kAspectBits{{VK_IMAGE_ASPECT_DEPTH_BIT}};
        return kAspectBits;
    }
};

struct StencilAspectTraits {
    static const uint32_t kAspectCount = 1;
    static int Index(VkImageAspectFlags mask) { return 0; };
    static VkImageAspectFlags AspectMask() { return VK_IMAGE_ASPECT_STENCIL_BIT; }
    static const std::array<VkImageAspectFlagBits, kAspectCount> &AspectBits() {
        static std::array<VkImageAspectFlagBits, kAspectCount> kAspectBits{{VK_IMAGE_ASPECT_STENCIL_BIT}};
        return kAspectBits;
    }
};

struct DepthStencilAspectTraits {
    // VK_IMAGE_ASPECT_DEPTH_BIT = 0x00000002,  >> 1 -> 1 -1 -> 0
    // VK_IMAGE_ASPECT_STENCIL_BIT = 0x00000004, >> 1 -> 2 -1 = 1
    static const uint32_t kAspectCount = 2;
    static uint32_t Index(VkImageAspectFlags mask) {
        uint32_t index = (mask >> 1) - 1;
        assert((index == 0) || (index == 1));
        return index;
    };
    static VkImageAspectFlags AspectMask() { return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT; }
    static const std::array<VkImageAspectFlagBits, kAspectCount> &AspectBits() {
        static std::array<VkImageAspectFlagBits, kAspectCount> kAspectBits{
            {VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_ASPECT_STENCIL_BIT}};
        return kAspectBits;
    }
};

struct Multiplane2AspectTraits {
    // VK_IMAGE_ASPECT_PLANE_0_BIT = 0x00000010, >> 4 - 1 -> 0
    // VK_IMAGE_ASPECT_PLANE_1_BIT = 0x00000020, >> 4 - 1 -> 1
    static const uint32_t kAspectCount = 2;
    static uint32_t Index(VkImageAspectFlags mask) {
        uint32_t index = (mask >> 4) - 1;
        assert((index == 0) || (index == 1));
        return index;
    };
    static VkImageAspectFlags AspectMask() { return VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT; }
    static const std::array<VkImageAspectFlagBits, kAspectCount> &AspectBits() {
        static std::array<VkImageAspectFlagBits, kAspectCount> kAspectBits{
            {VK_IMAGE_ASPECT_PLANE_0_BIT, VK_IMAGE_ASPECT_PLANE_1_BIT}};
        return kAspectBits;
    }
};

struct Multiplane3AspectTraits {
    // VK_IMAGE_ASPECT_PLANE_0_BIT = 0x00000010, >> 4 - 1 -> 0
    // VK_IMAGE_ASPECT_PLANE_1_BIT = 0x00000020, >> 4 - 1 -> 1
    // VK_IMAGE_ASPECT_PLANE_2_BIT = 0x00000040, >> 4 - 1 -> 3
    static const uint32_t kAspectCount = 3;
    static uint32_t Index(VkImageAspectFlags mask) {
        uint32_t index = (mask >> 4) - 1;
        index = index > 2 ? 2 : index;
        assert((index == 0) || (index == 1) || (index == 2));
        return index;
    };
    static VkImageAspectFlags AspectMask() {
        return VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT | VK_IMAGE_ASPECT_PLANE_2_BIT;
    }
    static const std::array<VkImageAspectFlagBits, kAspectCount> &AspectBits() {
        static std::array<VkImageAspectFlagBits, kAspectCount> kAspectBits{
            {VK_IMAGE_ASPECT_PLANE_0_BIT, VK_IMAGE_ASPECT_PLANE_1_BIT, VK_IMAGE_ASPECT_PLANE_2_BIT}};
        return kAspectBits;
    }
};

std::string FormatDebugLabel(const char *prefix, const LoggingLabel &label);

const static VkImageLayout kInvalidLayout = VK_IMAGE_LAYOUT_MAX_ENUM;
// Interface class.
class ImageSubresourceLayoutMap {
   public:
    typedef std::function<bool(const VkImageSubresource &, VkImageLayout, VkImageLayout)> Callback;
    struct InitialLayoutState {
        VkImageView image_view;          // For relaxed matching rule evaluation, else VK_NULL_HANDLE
        VkImageAspectFlags aspect_mask;  // For relaxed matching rules... else 0
        LoggingLabel label;
        InitialLayoutState(const CMD_BUFFER_STATE &cb_state_, const IMAGE_VIEW_STATE *view_state);
        InitialLayoutState() : image_view(VK_NULL_HANDLE), aspect_mask(0), label() {}
    };

    struct SubresourceLayout {
        VkImageSubresource subresource;
        VkImageLayout layout;
    };

    struct SubresourceRangeLayout {
        VkImageSubresourceRange range;
        VkImageLayout layout;
    };

    class ConstIteratorInterface {
       public:
        // Make the value accessor non virtual
        const SubresourceLayout &operator*() const { return value_; }

        virtual ConstIteratorInterface &operator++() = 0;
        virtual bool AtEnd() const = 0;
        virtual ~ConstIteratorInterface(){};

       protected:
        SubresourceLayout value_;
    };

    class ConstIterator {
       public:
        ConstIterator &operator++() {
            ++(*it_);
            return *this;
        }
        const SubresourceLayout &operator*() const { return *(*it_); }
        ConstIterator(ConstIteratorInterface *it) : it_(it){};
        bool AtEnd() const { return it_->AtEnd(); }

       protected:
        std::unique_ptr<ConstIteratorInterface> it_;
    };

    virtual ConstIterator BeginInitialUse() const = 0;
    virtual ConstIterator BeginSetLayout() const = 0;

    virtual bool SetSubresourceRangeLayout(const CMD_BUFFER_STATE &cb_state, const VkImageSubresourceRange &range,
                                           VkImageLayout layout, VkImageLayout expected_layout = kInvalidLayout) = 0;
    virtual bool SetSubresourceRangeInitialLayout(const CMD_BUFFER_STATE &cb_state, const VkImageSubresourceRange &range,
                                                  VkImageLayout layout, const IMAGE_VIEW_STATE *view_state = nullptr) = 0;
    virtual bool ForRange(const VkImageSubresourceRange &range, const Callback &callback, bool skip_invalid = true,
                          bool always_get_initial = false) const = 0;
    virtual VkImageLayout GetSubresourceLayout(const VkImageSubresource subresource) const = 0;
    virtual VkImageLayout GetSubresourceInitialLayout(const VkImageSubresource subresource) const = 0;
    virtual const InitialLayoutState *GetSubresourceInitialLayoutState(const VkImageSubresource subresource) const = 0;
    virtual bool UpdateFrom(const ImageSubresourceLayoutMap &from) = 0;
    virtual uintptr_t CompatibilityKey() const = 0;
    ImageSubresourceLayoutMap() {}
    virtual ~ImageSubresourceLayoutMap() {}
};

template <typename AspectTraits_, size_t kSparseThreshold = 64U>
class ImageSubresourceLayoutMapImpl : public ImageSubresourceLayoutMap {
   public:
    typedef ImageSubresourceLayoutMap Base;
    typedef AspectTraits_ AspectTraits;
    typedef Base::SubresourceLayout SubresourceLayout;
    typedef sparse_container::SparseVector<size_t, VkImageLayout, true, kInvalidLayout, kSparseThreshold> LayoutMap;
    typedef sparse_container::SparseVector<size_t, VkImageLayout, false, kInvalidLayout, kSparseThreshold> InitialLayoutMap;

    struct Layouts {
        LayoutMap current;
        InitialLayoutMap initial;
        Layouts(size_t size) : current(0, size), initial(0, size) {}
    };

    template <typename Container>
    class ConstIteratorImpl : public Base::ConstIteratorInterface {
       public:
        ConstIteratorImpl &operator++() override {
            ++it_;
            UpdateValue();
            return *this;
        }
        // Just good enough for cend checks
        ConstIteratorImpl(const ImageSubresourceLayoutMapImpl &map, const Container &container)
            : map_(&map), container_(&container), the_end_(false) {
            it_ = container_->cbegin();
            UpdateValue();
        }
        ~ConstIteratorImpl() override {}
        virtual bool AtEnd() const override { return the_end_; }

       protected:
        void UpdateValue() {
            if (it_ != container_->cend()) {
                value_.subresource = map_->Decode((*it_).first);
                value_.layout = (*it_).second;
            } else {
                the_end_ = true;
                value_.layout = kInvalidLayout;
            }
        }

        typedef typename Container::const_iterator ContainerIterator;
        const ImageSubresourceLayoutMapImpl *map_;
        const Container *container_;
        bool the_end_;
        ContainerIterator it_;
    };

    Base::ConstIterator BeginInitialUse() const override {
        return Base::ConstIterator(new ConstIteratorImpl<InitialLayoutMap>(*this, layouts_.initial));
    }

    Base::ConstIterator BeginSetLayout() const override {
        return Base::ConstIterator(new ConstIteratorImpl<LayoutMap>(*this, layouts_.current));
    }

    bool SetSubresourceRangeLayout(const CMD_BUFFER_STATE &cb_state, const VkImageSubresourceRange &range, VkImageLayout layout,
                                   VkImageLayout expected_layout = kInvalidLayout) override {
        bool updated = false;
        if (expected_layout == kInvalidLayout) {
            // Set the initial layout to the set layout as we had no other layout to reference
            expected_layout = layout;
        }
        if (!InRange(range)) return false;  // Don't even try to track bogus subreources

        InitialLayoutState *initial_state = nullptr;
        const uint32_t end_mip = range.baseMipLevel + range.levelCount;
        const auto &aspects = AspectTraits::AspectBits();
        for (uint32_t aspect_index = 0; aspect_index < AspectTraits::kAspectCount; aspect_index++) {
            if (0 == (range.aspectMask & aspects[aspect_index])) continue;
            size_t array_offset = Encode(aspect_index, range.baseMipLevel);
            for (uint32_t mip_level = range.baseMipLevel; mip_level < end_mip; ++mip_level, array_offset += mip_size_) {
                size_t start = array_offset + range.baseArrayLayer;
                size_t end = start + range.layerCount;
                bool updated_level = layouts_.current.SetRange(start, end, layout);
                if (updated_level) {
                    // We only need to try setting the initial layout, if we changed any of the layout values above
                    updated = true;
                    if (layouts_.initial.SetRange(start, end, expected_layout)) {
                        // We only need to try setting the initial layout *state* if the initial layout was updated
                        initial_state = UpdateInitialLayoutState(start, end, initial_state, cb_state, nullptr);
                    }
                }
            }
        }
        if (updated) version_++;
        return updated;
    }

    bool SetSubresourceRangeInitialLayout(const CMD_BUFFER_STATE &cb_state, const VkImageSubresourceRange &range,
                                          VkImageLayout layout, const IMAGE_VIEW_STATE *view_state = nullptr) override {
        bool updated = false;
        if (!InRange(range)) return false;  // Don't even try to track bogus subreources

        InitialLayoutState *initial_state = nullptr;
        const uint32_t end_mip = range.baseMipLevel + range.levelCount;
        const auto &aspects = AspectTraits::AspectBits();
        for (uint32_t aspect_index = 0; aspect_index < AspectTraits::kAspectCount; aspect_index++) {
            if (0 == (range.aspectMask & aspects[aspect_index])) continue;
            size_t array_offset = Encode(aspect_index, range.baseMipLevel);
            for (uint32_t mip_level = range.baseMipLevel; mip_level < end_mip; ++mip_level, array_offset += mip_size_) {
                size_t start = array_offset + range.baseArrayLayer;
                size_t end = start + range.layerCount;
                bool updated_level = layouts_.initial.SetRange(start, end, layout);
                if (updated_level) {
                    updated = true;
                    // We only need to try setting the initial layout *state* if the initial layout was updated
                    initial_state = UpdateInitialLayoutState(start, end, initial_state, cb_state, view_state);
                }
            }
        }
        if (updated) version_++;
        return updated;
    }

    // Loop over the given range calling the callback, primarily for
    // validation checks.  By default the initial_value is only looked
    // up if the set value isn't found.
    bool ForRange(const VkImageSubresourceRange &range, const Callback &callback, bool skip_invalid = true,
                  bool always_get_initial = false) const override {
        if (!InRange(range)) return false;  // Don't even try to process bogus subreources

        VkImageSubresource subres;
        auto &level = subres.mipLevel;
        auto &layer = subres.arrayLayer;
        auto &aspect = subres.aspectMask;
        const auto &aspects = AspectTraits::AspectBits();
        bool keep_on = true;
        const uint32_t end_mip = range.baseMipLevel + range.levelCount;
        const uint32_t end_layer = range.baseArrayLayer + range.layerCount;
        for (uint32_t aspect_index = 0; aspect_index < AspectTraits::kAspectCount; aspect_index++) {
            if (0 == (range.aspectMask & aspects[aspect_index])) continue;
            aspect = aspects[aspect_index];  // noting that this and the following loop indices are references
            size_t array_offset = Encode(aspect_index, range.baseMipLevel);
            for (level = range.baseMipLevel; level < end_mip; ++level, array_offset += mip_size_) {
                for (layer = range.baseArrayLayer; layer < end_layer; layer++) {
                    // TODO -- would an interator with range check be faster?
                    size_t index = array_offset + layer;
                    VkImageLayout layout = layouts_.current.Get(index);
                    VkImageLayout initial_layout = kInvalidLayout;
                    if (always_get_initial || (layout == kInvalidLayout)) {
                        initial_layout = layouts_.initial.Get(index);
                    }

                    if (!skip_invalid || (layout != kInvalidLayout) || (initial_layout != kInvalidLayout)) {
                        keep_on = callback(subres, layout, initial_layout);
                        if (!keep_on) return keep_on;  // False value from the callback aborts the range traversal
                    }
                }
            }
        }
        return keep_on;
    }
    VkImageLayout GetSubresourceInitialLayout(const VkImageSubresource subresource) const override {
        if (!InRange(subresource)) return kInvalidLayout;
        uint32_t aspect_index = AspectTraits::Index(subresource.aspectMask);
        size_t index = Encode(aspect_index, subresource.mipLevel, subresource.arrayLayer);
        return layouts_.initial.Get(index);
    }

    const InitialLayoutState *GetSubresourceInitialLayoutState(const VkImageSubresource subresource) const override {
        if (!InRange(subresource)) return nullptr;
        uint32_t aspect_index = AspectTraits::Index(subresource.aspectMask);
        size_t index = Encode(aspect_index, subresource.mipLevel, subresource.arrayLayer);
        return initial_layout_state_map_.Get(index);
    }

    VkImageLayout GetSubresourceLayout(const VkImageSubresource subresource) const override {
        if (!InRange(subresource)) return kInvalidLayout;
        uint32_t aspect_index = AspectTraits::Index(subresource.aspectMask);
        size_t index = Encode(aspect_index, subresource.mipLevel, subresource.arrayLayer);
        return layouts_.current.Get(index);
    }

    // TODO: make sure this paranoia check is sufficient and not too much.
    uintptr_t CompatibilityKey() const override {
        return (reinterpret_cast<const uintptr_t>(&image_state_) ^ AspectTraits::AspectMask() ^ kSparseThreshold);
    }

    bool UpdateFrom(const ImageSubresourceLayoutMap &other) override {
        // Must be from matching images for the reinterpret cast to be valid
        assert(CompatibilityKey() == other.CompatibilityKey());
        if (CompatibilityKey() != other.CompatibilityKey()) return false;

        const auto &from = reinterpret_cast<const ImageSubresourceLayoutMapImpl &>(other);
        bool updated = false;
        updated |= layouts_.initial.Merge(from.layouts_.initial);
        updated |= layouts_.current.Merge(from.layouts_.current);
        initial_layout_state_map_.Merge(from.initial_layout_state_map_);

        return updated;
    }

    ImageSubresourceLayoutMapImpl() : Base() {}
    ImageSubresourceLayoutMapImpl(const IMAGE_STATE &image_state)
        : Base(),
          image_state_(image_state),
          mip_size_(image_state.full_range.layerCount),
          aspect_size_(mip_size_ * image_state.full_range.levelCount),
          version_(0),
          layouts_(aspect_size_ * AspectTraits::kAspectCount),
          initial_layout_states_(),
          initial_layout_state_map_(0, aspect_size_ * AspectTraits::kAspectCount) {
        // Setup the row <-> aspect/mip_level base Encode/Decode LUT...
        aspect_offsets_[0] = 0;
        for (size_t i = 1; i < aspect_offsets_.size(); ++i) {  // Size is a compile time constant
            aspect_offsets_[i] = aspect_offsets_[i - 1] + aspect_size_;
        }
    }
    ~ImageSubresourceLayoutMapImpl() override {}

   protected:
    // This looks a bit ponderous but kAspectCount is a compile time constant
    VkImageSubresource Decode(size_t index) const {
        VkImageSubresource subres;
        // find aspect index
        uint32_t aspect_index = 0;
        if (AspectTraits::kAspectCount == 2) {
            if (index >= aspect_offsets_[1]) {
                aspect_index = 1;
                index = index - aspect_offsets_[aspect_index];
            }
        } else if (AspectTraits::kAspectCount == 3) {
            if (index >= aspect_offsets_[2]) {
                aspect_index = 2;
            } else if (index >= aspect_offsets_[1]) {
                aspect_index = 1;
            }
            index = index - aspect_offsets_[aspect_index];
        } else {
            assert(AspectTraits::kAspectCount == 1);  // Only aspect counts of 1, 2, and 3 supported
        }

        subres.aspectMask = AspectTraits::AspectBits()[aspect_index];
        subres.mipLevel =
            static_cast<uint32_t>(index / mip_size_);  // One hopes the compiler with optimize this pair of divisions...
        subres.arrayLayer = static_cast<uint32_t>(index % mip_size_);

        return subres;
    }

    uint32_t LevelLimit(uint32_t level) const { return (std::min)(image_state_.full_range.levelCount, level); }
    uint32_t LayerLimit(uint32_t layer) const { return (std::min)(image_state_.full_range.layerCount, layer); }

    bool InRange(const VkImageSubresource &subres) const {
        bool in_range = (subres.mipLevel < image_state_.full_range.levelCount) &&
                        (subres.arrayLayer < image_state_.full_range.layerCount) &&
                        (subres.aspectMask & AspectTraits::AspectMask());
        return in_range;
    }

    bool InRange(const VkImageSubresourceRange &range) const {
        bool in_range = (range.baseMipLevel < image_state_.full_range.levelCount) &&
                        ((range.baseMipLevel + range.levelCount) <= image_state_.full_range.levelCount) &&
                        (range.baseArrayLayer < image_state_.full_range.layerCount) &&
                        ((range.baseArrayLayer + range.layerCount) <= image_state_.full_range.layerCount) &&
                        (range.aspectMask & AspectTraits::AspectMask());
        return in_range;
    }

    inline size_t Encode(uint32_t aspect_index) const {
        return (AspectTraits::kAspectCount == 1) ? 0 : aspect_offsets_[aspect_index];
    }
    inline size_t Encode(uint32_t aspect_index, uint32_t mip_level) const { return Encode(aspect_index) + mip_level * mip_size_; }
    inline size_t Encode(uint32_t aspect_index, uint32_t mip_level, uint32_t array_layer) const {
        return Encode(aspect_index, mip_level) + array_layer;
    }

    InitialLayoutState *UpdateInitialLayoutState(size_t start, size_t end, InitialLayoutState *initial_state,
                                                 const CMD_BUFFER_STATE &cb_state, const IMAGE_VIEW_STATE *view_state) {
        if (!initial_state) {
            // Allocate on demand...  initial_layout_states_ holds ownership as a unique_ptr, while
            // each subresource has a non-owning copy of the plain pointer.
            initial_state = new InitialLayoutState(cb_state, view_state);
            initial_layout_states_.emplace_back(initial_state);
        }
        assert(initial_state);
        initial_layout_state_map_.SetRange(start, end, initial_state);
        return initial_state;
    }

    typedef std::vector<std::unique_ptr<InitialLayoutState>> InitialLayoutStates;
    // This map *also* needs "write once" semantics
    typedef sparse_container::SparseVector<size_t, InitialLayoutState *, false, nullptr, kSparseThreshold> InitialLayoutStateMap;

    const IMAGE_STATE &image_state_;
    const size_t mip_size_;
    const size_t aspect_size_;
    uint64_t version_ = 0;
    Layouts layouts_;
    InitialLayoutStates initial_layout_states_;
    InitialLayoutStateMap initial_layout_state_map_;
    std::array<size_t, AspectTraits::kAspectCount> aspect_offsets_;
};

static VkImageLayout NormalizeImageLayout(VkImageLayout layout, VkImageLayout non_normal, VkImageLayout normal) {
    return (layout == non_normal) ? normal : layout;
}

static VkImageLayout NormalizeDepthImageLayout(VkImageLayout layout) {
    return NormalizeImageLayout(layout, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
                                VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL);
}

static VkImageLayout NormalizeStencilImageLayout(VkImageLayout layout) {
    return NormalizeImageLayout(layout, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
                                VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL);
}

static bool ImageLayoutMatches(const VkImageAspectFlags aspect_mask, VkImageLayout a, VkImageLayout b) {
    bool matches = (a == b);
    if (!matches) {
        // Relaxed rules when referencing *only* the depth or stencil aspects
        if (aspect_mask == VK_IMAGE_ASPECT_DEPTH_BIT) {
            matches = NormalizeDepthImageLayout(a) == NormalizeDepthImageLayout(b);
        } else if (aspect_mask == VK_IMAGE_ASPECT_STENCIL_BIT) {
            matches = NormalizeStencilImageLayout(a) == NormalizeStencilImageLayout(b);
        }
    }
    return matches;
}

// Utility type for ForRange callbacks
struct LayoutUseCheckAndMessage {
    const static VkImageAspectFlags kDepthOrStencil = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    const ImageSubresourceLayoutMap *layout_map;
    const VkImageAspectFlags aspect_mask;
    const char *message;
    VkImageLayout layout;

    LayoutUseCheckAndMessage() = delete;
    LayoutUseCheckAndMessage(const ImageSubresourceLayoutMap *layout_map_, const VkImageAspectFlags aspect_mask_ = 0)
        : layout_map(layout_map_), aspect_mask{aspect_mask_}, message(nullptr), layout(kInvalidLayout) {}
    bool Check(const VkImageSubresource &subres, VkImageLayout check, VkImageLayout current_layout, VkImageLayout initial_layout) {
        message = nullptr;
        layout = kInvalidLayout;  // Success status
        if (current_layout != kInvalidLayout && !ImageLayoutMatches(aspect_mask, check, current_layout)) {
            message = "previous known";
            layout = current_layout;
        } else if ((initial_layout != kInvalidLayout) && !ImageLayoutMatches(aspect_mask, check, initial_layout)) {
            // To check the relaxed rule matching we need to see how the initial use was used
            const auto initial_layout_state = layout_map->GetSubresourceInitialLayoutState(subres);
            assert(initial_layout_state);  // If we have an initial layout, we better have a state for it
            if (!((initial_layout_state->aspect_mask & kDepthOrStencil) &&
                  ImageLayoutMatches(initial_layout_state->aspect_mask, check, initial_layout))) {
                message = "previously used";
                layout = initial_layout;
            }
        }
        return layout == kInvalidLayout;
    }
};

// Store the DAG.
struct DAGNode {
    uint32_t pass;
    std::vector<uint32_t> prev;
    std::vector<uint32_t> next;
};

struct RENDER_PASS_STATE : public BASE_NODE {
    VkRenderPass renderPass;
    safe_VkRenderPassCreateInfo2KHR createInfo;
    std::vector<std::vector<uint32_t>> self_dependencies;
    std::vector<DAGNode> subpassToNode;
    std::unordered_map<uint32_t, bool> attachment_first_read;

    RENDER_PASS_STATE(VkRenderPassCreateInfo2KHR const *pCreateInfo) : createInfo(pCreateInfo) {}
    RENDER_PASS_STATE(VkRenderPassCreateInfo const *pCreateInfo) { ConvertVkRenderPassCreateInfoToV2KHR(pCreateInfo, &createInfo); }
};

// Autogenerated as part of the vk_validation_error_message.h codegen
enum CMD_TYPE { VUID_CMD_ENUM_LIST(CMD_) };

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
    CBSTATUS_NONE                   = 0x00000000,   // No status is set
    CBSTATUS_LINE_WIDTH_SET         = 0x00000001,   // Line width has been set
    CBSTATUS_DEPTH_BIAS_SET         = 0x00000002,   // Depth bias has been set
    CBSTATUS_BLEND_CONSTANTS_SET    = 0x00000004,   // Blend constants state has been set
    CBSTATUS_DEPTH_BOUNDS_SET       = 0x00000008,   // Depth bounds state object has been set
    CBSTATUS_STENCIL_READ_MASK_SET  = 0x00000010,   // Stencil read mask has been set
    CBSTATUS_STENCIL_WRITE_MASK_SET = 0x00000020,   // Stencil write mask has been set
    CBSTATUS_STENCIL_REFERENCE_SET  = 0x00000040,   // Stencil reference has been set
    CBSTATUS_VIEWPORT_SET           = 0x00000080,
    CBSTATUS_SCISSOR_SET            = 0x00000100,
    CBSTATUS_INDEX_BUFFER_BOUND     = 0x00000200,   // Index buffer has been set
    CBSTATUS_EXCLUSIVE_SCISSOR_SET  = 0x00000400,
    CBSTATUS_SHADING_RATE_PALETTE_SET = 0x00000800,
    CBSTATUS_LINE_STIPPLE_SET       = 0x00001000,
    CBSTATUS_ALL_STATE_SET          = 0x00001DFF,   // All state set (intentionally exclude index buffer)
    // clang-format on
};

struct QueryObject {
    VkQueryPool pool;
    uint32_t query;
    // These next two fields are *not* used in hash or comparison, they are effectively a data payload
    uint32_t index;  // must be zero if !indexed
    bool indexed;
    QueryObject(VkQueryPool pool_, uint32_t query_) : pool(pool_), query(query_), index(0), indexed(false) {}
    QueryObject(VkQueryPool pool_, uint32_t query_, uint32_t index_) : pool(pool_), query(query_), index(index_), indexed(true) {}
    bool operator<(const QueryObject &rhs) const { return (pool == rhs.pool) ? query < rhs.query : pool < rhs.pool; }
};

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
    QUERYRESULT_MAYBE_NO_DATA,
    QUERYRESULT_SOME_DATA,
    QUERYRESULT_WAIT_ON_RESET,
    QUERYRESULT_WAIT_ON_RUNNING,
};

inline const char *string_QueryResultType(QueryResultType result_type) {
    switch (result_type) {
        case QUERYRESULT_UNKNOWN:
            return "query may be in an unknown state";
        case QUERYRESULT_NO_DATA:
        case QUERYRESULT_MAYBE_NO_DATA:
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

inline bool operator==(const QueryObject &query1, const QueryObject &query2) {
    return ((query1.pool == query2.pool) && (query1.query == query2.query));
}

namespace std {
template <>
struct hash<QueryObject> {
    size_t operator()(QueryObject query) const throw() {
        return hash<uint64_t>()((uint64_t)(query.pool)) ^ hash<uint32_t>()(query.query);
    }
};
}  // namespace std

struct CBVertexBufferBindingInfo {
    std::vector<BufferBinding> vertex_buffer_bindings;
};

struct ImageSubresourcePair {
    VkImage image;
    bool hasSubresource;
    VkImageSubresource subresource;
};

inline bool operator==(const ImageSubresourcePair &img1, const ImageSubresourcePair &img2) {
    if (img1.image != img2.image || img1.hasSubresource != img2.hasSubresource) return false;
    return !img1.hasSubresource ||
           (img1.subresource.aspectMask == img2.subresource.aspectMask && img1.subresource.mipLevel == img2.subresource.mipLevel &&
            img1.subresource.arrayLayer == img2.subresource.arrayLayer);
}

namespace std {
template <>
struct hash<ImageSubresourcePair> {
    size_t operator()(ImageSubresourcePair img) const throw() {
        size_t hashVal = hash<uint64_t>()(reinterpret_cast<uint64_t &>(img.image));
        hashVal ^= hash<bool>()(img.hasSubresource);
        if (img.hasSubresource) {
            hashVal ^= hash<uint32_t>()(reinterpret_cast<uint32_t &>(img.subresource.aspectMask));
            hashVal ^= hash<uint32_t>()(img.subresource.mipLevel);
            hashVal ^= hash<uint32_t>()(img.subresource.arrayLayer);
        }
        return hashVal;
    }
};
}  // namespace std

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
struct PIPELINE_LAYOUT_STATE {
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
    bool is_patch;
    bool is_block_member;
    bool is_relaxed_precision;
    // TODO: collect the name, too? Isn't required to be present.
};
typedef std::pair<unsigned, unsigned> descriptor_slot_t;

class PIPELINE_STATE : public BASE_NODE {
   public:
    struct StageState {
        std::unordered_set<uint32_t> accessible_ids;
        std::vector<std::pair<descriptor_slot_t, interface_var>> descriptor_uses;
        bool has_writable_descriptor;
    };

    VkPipeline pipeline;
    safe_VkGraphicsPipelineCreateInfo graphicsPipelineCI;
    safe_VkComputePipelineCreateInfo computePipelineCI;
    safe_VkRayTracingPipelineCreateInfoNV raytracingPipelineCI;
    // Hold shared ptr to RP in case RP itself is destroyed
    std::shared_ptr<RENDER_PASS_STATE> rp_state;
    // Flag of which shader stages are active for this pipeline
    uint32_t active_shaders;
    uint32_t duplicate_shaders;
    // Capture which slots (set#->bindings) are actually used by the shaders of this pipeline
    std::unordered_map<uint32_t, BindingReqMap> active_slots;
    // Additional metadata needed by pipeline_state initialization and validation
    std::vector<StageState> stage_state;
    // Vtx input info (if any)
    std::vector<VkVertexInputBindingDescription> vertex_binding_descriptions_;
    std::vector<VkVertexInputAttributeDescription> vertex_attribute_descriptions_;
    std::unordered_map<uint32_t, uint32_t> vertex_binding_to_index_map_;
    std::vector<VkPipelineColorBlendAttachmentState> attachments;
    bool blendConstantsEnabled;  // Blend constants enabled for any attachments
    PIPELINE_LAYOUT_STATE pipeline_layout;
    VkPrimitiveTopology topology_at_rasterizer;

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
          vertex_binding_descriptions_(),
          vertex_attribute_descriptions_(),
          vertex_binding_to_index_map_(),
          attachments(),
          blendConstantsEnabled(false),
          pipeline_layout(),
          topology_at_rasterizer{} {}

    void reset() {
        VkGraphicsPipelineCreateInfo emptyGraphicsCI = {};
        graphicsPipelineCI.initialize(&emptyGraphicsCI, false, false);
        VkComputePipelineCreateInfo emptyComputeCI = {};
        computePipelineCI.initialize(&emptyComputeCI);
        VkRayTracingPipelineCreateInfoNV emptyRayTracingCI = {};
        raytracingPipelineCI.initialize(&emptyRayTracingCI);
        stage_state.clear();
    }

    void initGraphicsPipeline(ValidationStateTracker *state_data, const VkGraphicsPipelineCreateInfo *pCreateInfo,
                              std::shared_ptr<RENDER_PASS_STATE> &&rpstate);
    void initComputePipeline(ValidationStateTracker *state_data, const VkComputePipelineCreateInfo *pCreateInfo);
    void initRayTracingPipelineNV(ValidationStateTracker *state_data, const VkRayTracingPipelineCreateInfoNV *pCreateInfo);

    inline VkPipelineBindPoint getPipelineType() const {
        if (graphicsPipelineCI.sType == VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO)
            return VK_PIPELINE_BIND_POINT_GRAPHICS;
        else if (computePipelineCI.sType == VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO)
            return VK_PIPELINE_BIND_POINT_COMPUTE;
        else if (raytracingPipelineCI.sType == VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV)
            return VK_PIPELINE_BIND_POINT_RAY_TRACING_NV;
        else
            return VK_PIPELINE_BIND_POINT_MAX_ENUM;
    }

    inline VkPipelineCreateFlags getPipelineCreateFlags() const {
        if (graphicsPipelineCI.sType == VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO)
            return graphicsPipelineCI.flags;
        else if (computePipelineCI.sType == VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO)
            return computePipelineCI.flags;
        else if (raytracingPipelineCI.sType == VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV)
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
template <typename Handle, typename Barrier>
struct QFOTransferBarrierBase {
    using HandleType = Handle;
    using BarrierType = Barrier;
    struct Tag {};
    HandleType handle = VK_NULL_HANDLE;
    uint32_t srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    uint32_t dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    QFOTransferBarrierBase() = default;
    QFOTransferBarrierBase(const BarrierType &barrier, const HandleType &resource_handle)
        : handle(resource_handle),
          srcQueueFamilyIndex(barrier.srcQueueFamilyIndex),
          dstQueueFamilyIndex(barrier.dstQueueFamilyIndex) {}

    hash_util::HashCombiner base_hash_combiner() const {
        hash_util::HashCombiner hc;
        hc << srcQueueFamilyIndex << dstQueueFamilyIndex << handle;
        return hc;
    }

    bool operator==(const QFOTransferBarrierBase &rhs) const {
        return (srcQueueFamilyIndex == rhs.srcQueueFamilyIndex) && (dstQueueFamilyIndex == rhs.dstQueueFamilyIndex) &&
               (handle == rhs.handle);
    }
};

template <typename Barrier>
struct QFOTransferBarrier {};

// Image barrier specific implementation
template <>
struct QFOTransferBarrier<VkImageMemoryBarrier> : public QFOTransferBarrierBase<VkImage, VkImageMemoryBarrier> {
    using BaseType = QFOTransferBarrierBase<VkImage, VkImageMemoryBarrier>;
    VkImageLayout oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkImageLayout newLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkImageSubresourceRange subresourceRange;

    QFOTransferBarrier() = default;
    QFOTransferBarrier(const BarrierType &barrier)
        : BaseType(barrier, barrier.image),
          oldLayout(barrier.oldLayout),
          newLayout(barrier.newLayout),
          subresourceRange(barrier.subresourceRange) {}
    size_t hash() const {
        // Ignoring the layout information for the purpose of the hash, as we're interested in QFO release/acquisition w.r.t.
        // the subresource affected, an layout transitions are current validated on another path
        auto hc = base_hash_combiner() << subresourceRange;
        return hc.Value();
    }
    bool operator==(const QFOTransferBarrier<BarrierType> &rhs) const {
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
template <>
struct QFOTransferBarrier<VkBufferMemoryBarrier> : public QFOTransferBarrierBase<VkBuffer, VkBufferMemoryBarrier> {
    using BaseType = QFOTransferBarrierBase<VkBuffer, VkBufferMemoryBarrier>;
    VkDeviceSize offset = 0;
    VkDeviceSize size = 0;
    QFOTransferBarrier(const VkBufferMemoryBarrier &barrier)
        : BaseType(barrier, barrier.buffer), offset(barrier.offset), size(barrier.size) {}
    size_t hash() const {
        auto hc = base_hash_combiner() << offset << size;
        return hc.Value();
    }
    bool operator==(const QFOTransferBarrier<BarrierType> &rhs) const {
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

template <typename Barrier>
using QFOTransferBarrierHash = hash_util::HasHashMember<QFOTransferBarrier<Barrier>>;

// Command buffers store the set of barriers recorded
template <typename Barrier>
using QFOTransferBarrierSet = std::unordered_set<QFOTransferBarrier<Barrier>, QFOTransferBarrierHash<Barrier>>;
template <typename Barrier>
struct QFOTransferBarrierSets {
    QFOTransferBarrierSet<Barrier> release;
    QFOTransferBarrierSet<Barrier> acquire;
    void Reset() {
        acquire.clear();
        release.clear();
    }
};

// The layer_data stores the map of pending release barriers
template <typename Barrier>
using GlobalQFOTransferBarrierMap =
    std::unordered_map<typename QFOTransferBarrier<Barrier>::HandleType, QFOTransferBarrierSet<Barrier>>;

// Submit queue uses the Scoreboard to track all release/acquire operations in a batch.
template <typename Barrier>
using QFOTransferCBScoreboard =
    std::unordered_map<QFOTransferBarrier<Barrier>, const CMD_BUFFER_STATE *, QFOTransferBarrierHash<Barrier>>;
template <typename Barrier>
struct QFOTransferCBScoreboards {
    QFOTransferCBScoreboard<Barrier> acquire;
    QFOTransferCBScoreboard<Barrier> release;
};

// Cmd Buffer Wrapper Struct - TODO : This desperately needs its own class
struct CMD_BUFFER_STATE : public BASE_NODE {
    VkCommandBuffer commandBuffer;
    VkCommandBufferAllocateInfo createInfo = {};
    VkCommandBufferBeginInfo beginInfo;
    VkCommandBufferInheritanceInfo inheritanceInfo;
    VkDevice device;  // device this CB belongs to
    bool hasDrawCmd;
    bool hasTraceRaysCmd;
    bool hasDispatchCmd;
    CB_STATE state;        // Track cmd buffer update state
    uint64_t submitCount;  // Number of times CB has been submitted
    typedef uint64_t ImageLayoutUpdateCount;
    ImageLayoutUpdateCount image_layout_change_count;  // The sequence number for changes to image layout (for cached validation)
    CBStatusFlags status;                              // Track status of various bindings on cmd buffer
    CBStatusFlags static_status;                       // All state bits provided by current graphics pipeline
                                                       // rather than dynamic state
    // Currently storing "lastBound" objects on per-CB basis
    //  long-term may want to create caches of "lastBound" states and could have
    //  each individual CMD_NODE referencing its own "lastBound" state
    // Store last bound state for Gfx & Compute pipeline bind points
    std::map<uint32_t, LAST_BOUND_STATE> lastBound;

    uint32_t viewportMask;
    uint32_t scissorMask;
    uint32_t initial_device_mask;

    VkRenderPassBeginInfo activeRenderPassBeginInfo;
    RENDER_PASS_STATE *activeRenderPass;
    VkSubpassContents activeSubpassContents;
    uint32_t active_render_pass_device_mask;
    uint32_t activeSubpass;
    VkFramebuffer activeFramebuffer;
    std::unordered_set<VkFramebuffer> framebuffers;
    // Unified data structs to track objects bound to this command buffer as well as object
    //  dependencies that have been broken : either destroyed objects, or updated descriptor sets
    std::unordered_set<VulkanTypedHandle> object_bindings;
    std::vector<VulkanTypedHandle> broken_bindings;

    QFOTransferBarrierSets<VkBufferMemoryBarrier> qfo_transfer_buffer_barriers;
    QFOTransferBarrierSets<VkImageMemoryBarrier> qfo_transfer_image_barriers;

    std::unordered_set<VkEvent> waitedEvents;
    std::vector<VkEvent> writeEventsBeforeWait;
    std::vector<VkEvent> events;
    std::map<QueryObject, QueryState> queryToStateMap;
    std::unordered_set<QueryObject> activeQueries;
    std::unordered_set<QueryObject> startedQueries;
    typedef std::unordered_map<VkImage, std::unique_ptr<ImageSubresourceLayoutMap>> ImageLayoutMap;
    ImageLayoutMap image_layout_map;
    std::unordered_map<VkEvent, VkPipelineStageFlags> eventToStageMap;
    std::vector<CBVertexBufferBindingInfo> cb_vertex_buffer_binding_info;
    CBVertexBufferBindingInfo current_vertex_buffer_binding_info;
    bool vertex_buffer_used;  // Track for perf warning to make sure any bound vtx buffer used
    VkCommandBuffer primaryCommandBuffer;
    // If primary, the secondary command buffers we will call.
    // If secondary, the primary command buffers we will be called by.
    std::unordered_set<CMD_BUFFER_STATE *> linkedCommandBuffers;
    // Validation functions run at primary CB queue submit time
    std::vector<std::function<bool()>> queue_submit_functions;
    // Validation functions run when secondary CB is executed in primary
    std::vector<std::function<bool(const CMD_BUFFER_STATE *, VkFramebuffer)>> cmd_execute_commands_functions;
    std::unordered_set<VkDeviceMemory> memObjs;
    std::vector<std::function<bool(VkQueue)>> eventUpdates;
    std::vector<std::function<bool(VkQueue)>> queryUpdates;
    std::unordered_set<cvdescriptorset::DescriptorSet *> validated_descriptor_sets;
    // Contents valid only after an index buffer is bound (CBSTATUS_INDEX_BUFFER_BOUND set)
    IndexBufferBinding index_buffer_binding;

    // Cache of current insert label...
    LoggingLabel debug_label;
};

static inline const QFOTransferBarrierSets<VkImageMemoryBarrier> &GetQFOBarrierSets(
    const CMD_BUFFER_STATE *cb, const QFOTransferBarrier<VkImageMemoryBarrier>::Tag &type_tag) {
    return cb->qfo_transfer_image_barriers;
}
static inline const QFOTransferBarrierSets<VkBufferMemoryBarrier> &GetQFOBarrierSets(
    const CMD_BUFFER_STATE *cb, const QFOTransferBarrier<VkBufferMemoryBarrier>::Tag &type_tag) {
    return cb->qfo_transfer_buffer_barriers;
}
static inline QFOTransferBarrierSets<VkImageMemoryBarrier> &GetQFOBarrierSets(
    CMD_BUFFER_STATE *cb, const QFOTransferBarrier<VkImageMemoryBarrier>::Tag &type_tag) {
    return cb->qfo_transfer_image_barriers;
}
static inline QFOTransferBarrierSets<VkBufferMemoryBarrier> &GetQFOBarrierSets(
    CMD_BUFFER_STATE *cb, const QFOTransferBarrier<VkBufferMemoryBarrier>::Tag &type_tag) {
    return cb->qfo_transfer_buffer_barriers;
}

struct SEMAPHORE_WAIT {
    VkSemaphore semaphore;
    VkQueue queue;
    uint64_t seq;
};

struct CB_SUBMISSION {
    CB_SUBMISSION(std::vector<VkCommandBuffer> const &cbs, std::vector<SEMAPHORE_WAIT> const &waitSemaphores,
                  std::vector<VkSemaphore> const &signalSemaphores, std::vector<VkSemaphore> const &externalSemaphores,
                  VkFence fence)
        : cbs(cbs),
          waitSemaphores(waitSemaphores),
          signalSemaphores(signalSemaphores),
          externalSemaphores(externalSemaphores),
          fence(fence) {}

    std::vector<VkCommandBuffer> cbs;
    std::vector<SEMAPHORE_WAIT> waitSemaphores;
    std::vector<VkSemaphore> signalSemaphores;
    std::vector<VkSemaphore> externalSemaphores;
    VkFence fence;
};

struct IMAGE_LAYOUT_STATE {
    VkImageLayout layout;
    VkFormat format;
};

struct MT_FB_ATTACHMENT_INFO {
    IMAGE_VIEW_STATE *view_state;
    VkImage image;
};

class FRAMEBUFFER_STATE : public BASE_NODE {
   public:
    VkFramebuffer framebuffer;
    safe_VkFramebufferCreateInfo createInfo;
    std::shared_ptr<RENDER_PASS_STATE> rp_state;
    FRAMEBUFFER_STATE(VkFramebuffer fb, const VkFramebufferCreateInfo *pCreateInfo, std::shared_ptr<RENDER_PASS_STATE> &&rpstate)
        : framebuffer(fb), createInfo(pCreateInfo), rp_state(rpstate){};
};

struct SHADER_MODULE_STATE;
struct DeviceExtensions;

struct DeviceFeatures {
    VkPhysicalDeviceFeatures core;
    VkPhysicalDeviceDescriptorIndexingFeaturesEXT descriptor_indexing;
    VkPhysicalDevice8BitStorageFeaturesKHR eight_bit_storage;
    VkPhysicalDeviceExclusiveScissorFeaturesNV exclusive_scissor;
    VkPhysicalDeviceShadingRateImageFeaturesNV shading_rate_image;
    VkPhysicalDeviceMeshShaderFeaturesNV mesh_shader;
    VkPhysicalDeviceInlineUniformBlockFeaturesEXT inline_uniform_block;
    VkPhysicalDeviceTransformFeedbackFeaturesEXT transform_feedback_features;
    VkPhysicalDeviceFloat16Int8FeaturesKHR float16_int8;
    VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT vtx_attrib_divisor_features;
    VkPhysicalDeviceUniformBufferStandardLayoutFeaturesKHR uniform_buffer_standard_layout;
    VkPhysicalDeviceScalarBlockLayoutFeaturesEXT scalar_block_layout_features;
    VkPhysicalDeviceBufferAddressFeaturesEXT buffer_address;
    VkPhysicalDeviceCooperativeMatrixFeaturesNV cooperative_matrix_features;
    VkPhysicalDeviceFloatControlsPropertiesKHR float_controls;
    VkPhysicalDeviceHostQueryResetFeaturesEXT host_query_reset_features;
    VkPhysicalDeviceComputeShaderDerivativesFeaturesNV compute_shader_derivatives_features;
    VkPhysicalDeviceFragmentShaderBarycentricFeaturesNV fragment_shader_barycentric_features;
    VkPhysicalDeviceShaderImageFootprintFeaturesNV shader_image_footprint_features;
    VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT fragment_shader_interlock_features;
    VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT demote_to_helper_invocation_features;
    VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT texel_buffer_alignment_features;
    VkPhysicalDeviceImagelessFramebufferFeaturesKHR imageless_framebuffer_features;
    VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR pipeline_exe_props_features;
};

enum RenderPassCreateVersion { RENDER_PASS_VERSION_1 = 0, RENDER_PASS_VERSION_2 = 1 };

struct ShaderTracker {
    VkPipeline pipeline;
    VkShaderModule shader_module;
    std::vector<unsigned int> pgm;
};

enum BarrierOperationsType {
    kAllAcquire,  // All Barrier operations are "ownership acquire" operations
    kAllRelease,  // All Barrier operations are "ownership release" operations
    kGeneral,     // Either no ownership operations or a mix of ownership operation types and/or non-ownership operations
};

std::shared_ptr<cvdescriptorset::DescriptorSetLayout const> const GetDescriptorSetLayout(const ValidationStateTracker *,
                                                                                         VkDescriptorSetLayout);

ImageSubresourceLayoutMap *GetImageSubresourceLayoutMap(CMD_BUFFER_STATE *cb_state, const IMAGE_STATE &image_state);
const ImageSubresourceLayoutMap *GetImageSubresourceLayoutMap(const CMD_BUFFER_STATE *cb_state, VkImage image);

#endif  // CORE_VALIDATION_TYPES_H_
