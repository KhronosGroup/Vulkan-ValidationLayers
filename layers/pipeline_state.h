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
#pragma once
#include "hash_vk_types.h"
#include "base_node.h"
#include "sampler_state.h"
#include "ray_tracing_state.h"

// Fwd declarations -- including descriptor_set.h creates an ugly include loop
namespace cvdescriptorset {
class DescriptorSetLayoutDef;
class DescriptorSetLayout;
class DescriptorSet;
class Descriptor;

}  // namespace cvdescriptorset

class ValidationStateTracker;
class CMD_BUFFER_STATE;
class RENDER_PASS_STATE;
struct SHADER_MODULE_STATE;

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

// Canonical dictionary for the pipeline layout's layout of descriptorsetlayouts
using DescriptorSetLayoutDef = cvdescriptorset::DescriptorSetLayoutDef;
using DescriptorSetLayoutId = std::shared_ptr<const DescriptorSetLayoutDef>;
using PipelineLayoutSetLayoutsDef = std::vector<DescriptorSetLayoutId>;
using PipelineLayoutSetLayoutsDict =
    hash_util::Dictionary<PipelineLayoutSetLayoutsDef, hash_util::IsOrderedContainer<PipelineLayoutSetLayoutsDef>>;
using PipelineLayoutSetLayoutsId = PipelineLayoutSetLayoutsDict::Id;

// Canonical dictionary for PushConstantRanges
using PushConstantRangesDict = hash_util::Dictionary<PushConstantRanges>;
using PushConstantRangesId = PushConstantRangesDict::Id;

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
class PIPELINE_LAYOUT_STATE : public BASE_NODE {
  public:
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

struct PipelineStageState {
    layer_data::unordered_set<uint32_t> accessible_ids;
    std::vector<std::pair<descriptor_slot_t, interface_var>> descriptor_uses;
    bool has_writable_descriptor;
    bool has_atomic_descriptor;
    VkShaderStageFlagBits stage_flag;
    std::string entry_point_name;
    std::shared_ptr<const SHADER_MODULE_STATE> shader_state;
};

class PIPELINE_STATE : public BASE_NODE {
  public:
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
    std::vector<PipelineStageState> stage_state;
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

    void SetHandle(VkPipeline p) { handle_.handle = CastToUint64(p); }

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
    return BindPoint_Count;
}
