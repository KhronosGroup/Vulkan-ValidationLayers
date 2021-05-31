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
#include "command_validation.h"
#include "base_node.h"
#include "image_state.h"
#include "buffer_state.h"
#include "ray_tracing_state.h"

#include <array>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <string.h>
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

// Track command pools and their command buffers
struct COMMAND_POOL_STATE : public BASE_NODE {
    VkCommandPoolCreateFlags createFlags;
    uint32_t queueFamilyIndex;
    bool unprotected;  // can't be used for protected memory
    // Cmd buffers allocated from this pool
    layer_data::unordered_set<VkCommandBuffer> commandBuffers;

    COMMAND_POOL_STATE(VkCommandPool cp, const VkCommandPoolCreateInfo *pCreateInfo)
        : BASE_NODE(cp, kVulkanObjectTypeCommandPool),
          createFlags(pCreateInfo->flags),
          queueFamilyIndex(pCreateInfo->queueFamilyIndex),
          unprotected((pCreateInfo->flags & VK_COMMAND_POOL_CREATE_PROTECTED_BIT) == 0) {}


    VkCommandPool commandPool() const { return handle_.Cast<VkCommandPool>(); }

    virtual ~COMMAND_POOL_STATE() { Destroy(); }

    void Destroy() override {
        commandBuffers.clear();
        BASE_NODE::Destroy();
    }
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

// Caution: Section 7.7.4 states that "If the values of srcQueueFamilyIndex and dstQueueFamilyIndex are equal, no ownership transfer
// is performed, and the barrier operates as if they were both set to VK_QUEUE_FAMILY_IGNORED."; this does not handle that case.
static inline bool QueueFamilyIsIgnored(uint32_t queue_family_index) { return queue_family_index == VK_QUEUE_FAMILY_IGNORED; }

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

inline bool operator==(const SamplerUsedByImage &a, const SamplerUsedByImage &b) NOEXCEPT {
    return a.sampler_slot == b.sampler_slot && a.sampler_index == b.sampler_index;
}

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
    layer_data::unordered_set<cvdescriptorset::DescriptorSet *> sets;  // Collection of all sets in this pool
    std::map<uint32_t, uint32_t> maxDescriptorTypeCount;        // Max # of descriptors of each type in this pool
    std::map<uint32_t, uint32_t> availableDescriptorTypeCount;  // Available # of descriptors of each type in this pool

    DESCRIPTOR_POOL_STATE(const VkDescriptorPool pool, const VkDescriptorPoolCreateInfo *pCreateInfo)
        : BASE_NODE(pool, kVulkanObjectTypeDescriptorPool),
          pool(pool),
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

struct SAMPLER_STATE : public BASE_NODE {
    VkSamplerCreateInfo createInfo;
    VkSamplerYcbcrConversion samplerConversion = VK_NULL_HANDLE;
    VkSamplerCustomBorderColorCreateInfoEXT customCreateInfo = {};

    SAMPLER_STATE(const VkSampler *ps, const VkSamplerCreateInfo *pci)
        : BASE_NODE(*ps, kVulkanObjectTypeSampler), createInfo(*pci) {
        auto *conversionInfo = LvlFindInChain<VkSamplerYcbcrConversionInfo>(pci->pNext);
        if (conversionInfo) samplerConversion = conversionInfo->conversion;
        auto cbci = LvlFindInChain<VkSamplerCustomBorderColorCreateInfoEXT>(pci->pNext);
        if (cbci) customCreateInfo = *cbci;
    }

    VkSampler sampler() const { return handle_.Cast<VkSampler>(); }
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

    safe_VkRenderPassCreateInfo2 createInfo;
    std::vector<std::vector<uint32_t>> self_dependencies;
    std::vector<DAGNode> subpassToNode;
    layer_data::unordered_map<uint32_t, bool> attachment_first_read;
    std::vector<uint32_t> attachment_first_subpass;
    std::vector<uint32_t> attachment_last_subpass;
    std::vector<bool> attachment_first_is_transition;
    std::vector<SubpassDependencyGraphNode> subpass_dependencies;
    std::vector<std::vector<AttachmentTransition>> subpass_transitions;

    RENDER_PASS_STATE(VkRenderPass rp, VkRenderPassCreateInfo2 const *pCreateInfo)
        : BASE_NODE(rp, kVulkanObjectTypeRenderPass), createInfo(pCreateInfo) {}
    RENDER_PASS_STATE(VkRenderPass rp, VkRenderPassCreateInfo const *pCreateInfo)
        : BASE_NODE(rp, kVulkanObjectTypeRenderPass) {
        ConvertVkRenderPassCreateInfoToV2KHR(*pCreateInfo, &createInfo);
    }

    VkRenderPass renderPass() const { return handle_.Cast<VkRenderPass>(); }
};

// Autogenerated as part of the command_validation.h codegen
const char *CommandTypeString(CMD_TYPE type);

enum CB_STATE {
    CB_NEW,                 // Newly created CB w/o any cmds
    CB_RECORDING,           // BeginCB has been called on this CB
    CB_RECORDED,            // EndCB has been called on this CB
    CB_INVALID_COMPLETE,    // had a complete recording, but was since invalidated
    CB_INVALID_INCOMPLETE,  // fouled before recording was completed
};

// CB Status -- used to track status of various bindings on cmd buffer objects
typedef uint64_t CBStatusFlags;
enum CBStatusFlagBits : uint64_t {
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
    CBSTATUS_PATCH_CONTROL_POINTS_SET        = 0x20000000,
    CBSTATUS_RASTERIZER_DISCARD_ENABLE_SET   = 0x40000000,
    CBSTATUS_DEPTH_BIAS_ENABLE_SET           = 0x80000000,
    CBSTATUS_LOGIC_OP_SET                    = 0x100000000,
    CBSTATUS_PRIMITIVE_RESTART_ENABLE_SET    = 0x200000000,
    CBSTATUS_VERTEX_INPUT_SET                = 0x400000000,
    CBSTATUS_ALL_STATE_SET                   = 0x7FFFFFDFF,   // All state set (intentionally exclude index buffer)
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
    std::vector<std::shared_ptr<cvdescriptorset::DescriptorSetLayout const>> set_layouts;
    PushConstantRangesId push_constant_ranges;
    std::vector<PipelineLayoutCompatId> compat_for_set;

    PIPELINE_LAYOUT_STATE(VkPipelineLayout l)
        : BASE_NODE(l, kVulkanObjectTypePipelineLayout), set_layouts{}, push_constant_ranges{}, compat_for_set{} {}

    VkPipelineLayout layout() const { return handle_.Cast<VkPipelineLayout>(); }

    void reset() {
        handle_.handle = 0;
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

struct SHADER_MODULE_STATE;
class PIPELINE_STATE : public BASE_NODE {
  public:
    struct StageState {
        layer_data::unordered_set<uint32_t> accessible_ids;
        std::vector<std::pair<descriptor_slot_t, interface_var>> descriptor_uses;
        bool has_writable_descriptor;
        bool has_atomic_descriptor;
        VkShaderStageFlagBits stage_flag;
        std::string entry_point_name;
        std::shared_ptr<const SHADER_MODULE_STATE> shader_state;
    };

    safe_VkGraphicsPipelineCreateInfo graphicsPipelineCI;
    safe_VkComputePipelineCreateInfo computePipelineCI;
    safe_VkRayTracingPipelineCreateInfoCommon raytracingPipelineCI;
    // Hold shared ptr to RP in case RP itself is destroyed
    std::shared_ptr<const RENDER_PASS_STATE> rp_state;
    // Flag of which shader stages are active for this pipeline
    uint32_t active_shaders;
    uint32_t duplicate_shaders;
    // Capture which slots (set#->bindings) are actually used by the shaders of this pipeline
    layer_data::unordered_map<uint32_t, BindingReqMap> active_slots;
    uint32_t max_active_slot;  // the highest set number in active_slots for pipeline layout compatibility checks
    // Additional metadata needed by pipeline_state initialization and validation
    std::vector<StageState> stage_state;
    layer_data::unordered_set<uint32_t> fragmentShader_writable_output_location_list;
    // Vtx input info (if any)
    std::vector<VkVertexInputBindingDescription> vertex_binding_descriptions_;
    std::vector<VkVertexInputAttributeDescription> vertex_attribute_descriptions_;
    std::vector<VkDeviceSize> vertex_attribute_alignments_;
    layer_data::unordered_map<uint32_t, uint32_t> vertex_binding_to_index_map_;
    std::vector<VkPipelineColorBlendAttachmentState> attachments;
    layer_data::unordered_set<VkShaderStageFlagBits, hash_util::HashCombiner::WrappedHash<VkShaderStageFlagBits>>
        wrote_primitive_shading_rate;
    bool blendConstantsEnabled;  // Blend constants enabled for any attachments
    std::shared_ptr<const PIPELINE_LAYOUT_STATE> pipeline_layout;
    VkPrimitiveTopology topology_at_rasterizer;
    VkBool32 sample_location_enabled;
    // Default constructor
    PIPELINE_STATE()
        : BASE_NODE(static_cast<VkPipeline>(VK_NULL_HANDLE), kVulkanObjectTypePipeline),
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

    VkPipeline pipeline() const { return handle_.Cast<VkPipeline>(); }

    void SetHandle(VkPipeline p) {
        handle_.handle = CastToUint64(p);
    }

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
    LAST_BOUND_STATE() { Reset(); }  // must define default constructor for portability reasons
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

    void Reset();

    void UnbindAndResetPushDescriptorSet(CMD_BUFFER_STATE *cb_state, cvdescriptorset::DescriptorSet *ds);

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
using QFOTransferBarrierSet = layer_data::unordered_set<TransferBarrier, QFOTransferBarrierHash<TransferBarrier>>;

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
    layer_data::unordered_map<typename TransferBarrier::HandleType, QFOTransferBarrierSet<TransferBarrier>>;

// Submit queue uses the Scoreboard to track all release/acquire operations in a batch.
template <typename TransferBarrier>
using QFOTransferCBScoreboard =
    layer_data::unordered_map<TransferBarrier, const CMD_BUFFER_STATE *, QFOTransferBarrierHash<TransferBarrier>>;

template <typename TransferBarrier>
struct QFOTransferCBScoreboards {
    QFOTransferCBScoreboard<TransferBarrier> acquire;
    QFOTransferCBScoreboard<TransferBarrier> release;
};

typedef std::map<QueryObject, QueryState> QueryMap;
typedef layer_data::unordered_map<VkEvent, VkPipelineStageFlags2KHR> EventToStageMap;
typedef subresource_adapter::BothRangeMap<VkImageLayout, 16> GlobalImageLayoutRangeMap;
typedef layer_data::unordered_map<VkImage, layer_data::optional<GlobalImageLayoutRangeMap>> GlobalImageLayoutMap;

typedef layer_data::unordered_map<VkImage, layer_data::optional<ImageSubresourceLayoutMap>> CommandBufferImageLayoutMap;

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
struct CMD_BUFFER_STATE : public REFCOUNTED_NODE {
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
    uint64_t commandCount;  // Number of commands recorded. Currently only used with VK_KHR_performance_query
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
    layer_data::unordered_map<VkDescriptorSet, std::vector<CmdDrawDispatchInfo>> validate_descriptorsets_in_queuesubmit;

    // If VK_NV_inherited_viewport_scissor is enabled and VkCommandBufferInheritanceViewportScissorInfoNV::viewportScissor2D is
    // true, then is the nonempty list of viewports passed in pViewportDepths. Otherwise, this is empty.
    std::vector<VkViewport> inheritedViewportDepths;

    // For each draw command D recorded to this command buffer, let
    //  * g_D be the graphics pipeline used
    //  * v_G be the viewportCount of g_D (0 if g_D disables rasterization or enables VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT_EXT)
    //  * s_G be the scissorCount  of g_D (0 if g_D disables rasterization or enables VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT_EXT)
    // Then this value is max(0, max(v_G for all D in cb), max(s_G for all D in cb))
    uint32_t usedViewportScissorCount;
    uint32_t pipelineStaticViewportCount; // v_G for currently-bound graphics pipeline.
    uint32_t pipelineStaticScissorCount;  // s_G for currently-bound graphics pipeline.

    uint32_t viewportMask;
    uint32_t viewportWithCountMask;
    uint32_t viewportWithCountCount;
    uint32_t scissorMask;
    uint32_t scissorWithCountMask;
    uint32_t scissorWithCountCount;

    // Dynamic viewports set in this command buffer; if bit j of viewportMask is set then dynamicViewports[j] is valid, but the
    // converse need not be true.
    std::vector<VkViewport> dynamicViewports;

    // Bits set when binding graphics pipeline defining corresponding static state, or executing any secondary command buffer.
    // Bits unset by calling a corresponding vkCmdSet[State] cmd.
    uint32_t trashedViewportMask;
    uint32_t trashedScissorMask;
    bool     trashedViewportCount;
    bool     trashedScissorCount;

    // True iff any draw command recorded to this command buffer consumes dynamic viewport/scissor with count state.
    bool usedDynamicViewportCount;
    bool usedDynamicScissorCount;

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
    layer_data::unordered_set<std::shared_ptr<FRAMEBUFFER_STATE>> framebuffers;
    // Unified data structs to track objects bound to this command buffer as well as object
    //  dependencies that have been broken : either destroyed objects, or updated descriptor sets
    layer_data::unordered_set<VulkanTypedHandle> object_bindings;
    layer_data::unordered_map<VulkanTypedHandle, LogObjectList> broken_bindings;

    QFOTransferBarrierSets<QFOBufferTransferBarrier> qfo_transfer_buffer_barriers;
    QFOTransferBarrierSets<QFOImageTransferBarrier> qfo_transfer_image_barriers;

    layer_data::unordered_set<VkEvent> waitedEvents;
    std::vector<VkEvent> writeEventsBeforeWait;
    std::vector<VkEvent> events;
    layer_data::unordered_set<QueryObject> activeQueries;
    layer_data::unordered_set<QueryObject> startedQueries;
    layer_data::unordered_set<QueryObject> resetQueries;
    CommandBufferImageLayoutMap image_layout_map;
    CBVertexBufferBindingInfo current_vertex_buffer_binding_info;
    bool vertex_buffer_used;  // Track for perf warning to make sure any bound vtx buffer used
    VkCommandBuffer primaryCommandBuffer;
    // If primary, the secondary command buffers we will call.
    // If secondary, the primary command buffers we will be called by.
    layer_data::unordered_set<CMD_BUFFER_STATE *> linkedCommandBuffers;
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
    layer_data::unordered_set<cvdescriptorset::DescriptorSet *> validated_descriptor_sets;
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

    CMD_BUFFER_STATE(VkCommandBuffer cb, const VkCommandBufferAllocateInfo* pCreateInfo)
        : REFCOUNTED_NODE(cb, kVulkanObjectTypeCommandBuffer), createInfo(*pCreateInfo) {}

    ~CMD_BUFFER_STATE() { Destroy(); }

    void Destroy() override;

    VkCommandBuffer commandBuffer() const { return handle_.Cast<VkCommandBuffer>(); }

    int AddReverseBinding(const VulkanTypedHandle &obj);

    IMAGE_VIEW_STATE* GetActiveAttachmentImageViewState(uint32_t index);
    const IMAGE_VIEW_STATE* GetActiveAttachmentImageViewState(uint32_t index) const;

    void AddChild(BASE_NODE *child_node);

    void RemoveChild(BASE_NODE *child_node);

    void Reset();

  protected:
    void NotifyInvalidate(const LogObjectList& invalid_handles, bool unlink) override;
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
    safe_VkFramebufferCreateInfo createInfo;
    std::shared_ptr<const RENDER_PASS_STATE> rp_state;
    std::vector<std::shared_ptr<IMAGE_VIEW_STATE>> attachments_view_state;
    FRAMEBUFFER_STATE(VkFramebuffer fb, const VkFramebufferCreateInfo *pCreateInfo, std::shared_ptr<RENDER_PASS_STATE> &&rpstate)
        : BASE_NODE(fb, kVulkanObjectTypeFramebuffer), createInfo(pCreateInfo), rp_state(rpstate){};

    VkFramebuffer framebuffer() const { return handle_.Cast<VkFramebuffer>(); }

    virtual ~FRAMEBUFFER_STATE() { Destroy(); }

    void Destroy() override;
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
    VkPhysicalDeviceExtendedDynamicState2FeaturesEXT extended_dynamic_state2_features;
    VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT vertex_input_dynamic_state_features;
    VkPhysicalDeviceInheritedViewportScissorFeaturesNV inherited_viewport_scissor_features;
    VkPhysicalDeviceProvokingVertexFeaturesEXT provoking_vertex_features;
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

uint32_t GetSubpassDepthStencilAttachmentIndex(const safe_VkPipelineDepthStencilStateCreateInfo *pipe_ds_ci,
                                               const safe_VkAttachmentReference2 *depth_stencil_ref);

#endif  // CORE_VALIDATION_TYPES_H_
