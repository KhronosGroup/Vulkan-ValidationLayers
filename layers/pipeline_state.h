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
 * Author: John Zulauf <jzulauf@lunarg.com>
 * Author: Tobias Hector <tobias.hector@amd.com>
 * Author: Jeremy Gebben <jeremyg@lunarg.com>
 */
#pragma once
#include "hash_vk_types.h"
#include "base_node.h"
#include "sampler_state.h"
#include "ray_tracing_state.h"
#include "render_pass_state.h"
#include "shader_module.h"

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
class PIPELINE_STATE;

// Flags describing requirements imposed by the pipeline on a descriptor. These
// can't be checked at pipeline creation time as they depend on the Image or
// ImageView bound.
enum DescriptorReqBits {
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
    DESCRIPTOR_REQ_IMAGE_READ_WITHOUT_FORMAT = DESCRIPTOR_REQ_SAMPLER_BIAS_OFFSET << 1,
    DESCRIPTOR_REQ_IMAGE_WRITE_WITHOUT_FORMAT = DESCRIPTOR_REQ_IMAGE_READ_WITHOUT_FORMAT << 1,
    DESCRIPTOR_REQ_IMAGE_DREF = DESCRIPTOR_REQ_IMAGE_WRITE_WITHOUT_FORMAT << 1,

};
typedef uint32_t DescriptorReqFlags;

extern DescriptorReqFlags DescriptorRequirementsBitsFromFormat(VkFormat fmt);

struct DescriptorRequirement {
    DescriptorReqFlags reqs;
    bool is_writable;
    // Copy from StageState.interface_var. It combines from plural shader stages. The index of array is index of image.
    std::vector<layer_data::unordered_set<SamplerUsedByImage>> samplers_used_by_image;
    DescriptorRequirement() : reqs(0), is_writable(false) {}
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
    using SetLayoutVector = std::vector<std::shared_ptr<cvdescriptorset::DescriptorSetLayout const>>;
    const SetLayoutVector set_layouts;
    // canonical form IDs for the "compatible for set" contents
    const PushConstantRangesId push_constant_ranges;
    // table of "compatible for set N" cannonical forms for trivial accept validation
    const std::vector<PipelineLayoutCompatId> compat_for_set;

    PIPELINE_LAYOUT_STATE(ValidationStateTracker *dev_data, VkPipelineLayout l, const VkPipelineLayoutCreateInfo *pCreateInfo);

    VkPipelineLayout layout() const { return handle_.Cast<VkPipelineLayout>(); }

    std::shared_ptr<cvdescriptorset::DescriptorSetLayout const> GetDsl(uint32_t set) const {
        std::shared_ptr<cvdescriptorset::DescriptorSetLayout const> dsl = nullptr;
        if (set < set_layouts.size()) {
            dsl = set_layouts[set];
        }
        return dsl;
    }
};

struct PipelineStageState {
    std::shared_ptr<const SHADER_MODULE_STATE> module_state;
    const VkPipelineShaderStageCreateInfo *create_info;
    VkShaderStageFlagBits stage_flag;
    spirv_inst_iter entrypoint;
    layer_data::unordered_set<uint32_t> accessible_ids;
    using DescriptorUse = std::pair<DescriptorSlot, interface_var>;
    std::vector<DescriptorUse> descriptor_uses;
    bool has_writable_descriptor;
    bool has_atomic_descriptor;
    bool wrote_primitive_shading_rate;

    PipelineStageState(const VkPipelineShaderStageCreateInfo *stage, std::shared_ptr<const SHADER_MODULE_STATE> &module_state);
};

class PIPELINE_STATE : public BASE_NODE {
  public:
    union CreateInfo {
        CreateInfo(const VkGraphicsPipelineCreateInfo *ci, std::shared_ptr<const RENDER_PASS_STATE> rpstate) : graphics() {
            bool use_color = false;
            bool use_depth_stencil = false;

            if (ci->renderPass == VK_NULL_HANDLE) {
                auto dynamic_rendering = LvlFindInChain<VkPipelineRenderingCreateInfo>(ci->pNext);
                if (dynamic_rendering) {
                    use_color = (dynamic_rendering->colorAttachmentCount > 0);
                    use_depth_stencil = (dynamic_rendering->depthAttachmentFormat != VK_FORMAT_UNDEFINED) ||
                                        (dynamic_rendering->stencilAttachmentFormat != VK_FORMAT_UNDEFINED);
                }
            } else {
                use_color = rpstate->UsesColorAttachment(ci->subpass);
                use_depth_stencil = rpstate->UsesDepthStencilAttachment(ci->subpass);
            }

            graphics.initialize(ci, use_color, use_depth_stencil);
        }
        CreateInfo(const VkComputePipelineCreateInfo *ci) : compute(ci) {}
        CreateInfo(const VkRayTracingPipelineCreateInfoKHR *ci) : raytracing(ci) {}
        CreateInfo(const VkRayTracingPipelineCreateInfoNV *ci) : raytracing(ci) {}

        ~CreateInfo() {
            switch (graphics.sType) {
                case VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO:
                    graphics.~safe_VkGraphicsPipelineCreateInfo();
                    break;
                case VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO:
                    compute.~safe_VkComputePipelineCreateInfo();
                    break;
                case VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV:
                case VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR:
                    raytracing.~safe_VkRayTracingPipelineCreateInfoCommon();
                    break;
                default:
                    assert(false);
                    break;
            }
        }

        safe_VkGraphicsPipelineCreateInfo graphics;
        safe_VkComputePipelineCreateInfo compute;
        safe_VkRayTracingPipelineCreateInfoCommon raytracing;
    };
    const CreateInfo create_info;
    std::shared_ptr<const PIPELINE_LAYOUT_STATE> pipeline_layout;
    std::shared_ptr<const RENDER_PASS_STATE> rp_state;
    // Additional metadata needed by pipeline_state initialization and validation
    using StageStateVec = std::vector<PipelineStageState>;
    const StageStateVec stage_state;

    // Capture which slots (set#->bindings) are actually used by the shaders of this pipeline
    using ActiveSlotMap = layer_data::unordered_map<uint32_t, BindingReqMap>;
    // NOTE: this map is 'almost' const and used in performance critical code paths.
    // The values of existing entries in the DescriptorRequirement::samplers_used_by_image map
    // are updated at various times. Locking requirements are TBD.
    const ActiveSlotMap active_slots;
    const uint32_t max_active_slot = 0;  // the highest set number in active_slots for pipeline layout compatibility checks

    const layer_data::unordered_set<uint32_t> fragmentShader_writable_output_location_list;
    // Vtx input info (if any)
    using VertexBindingVector = std::vector<VkVertexInputBindingDescription>;
    const VertexBindingVector vertex_binding_descriptions_;

    using VertexAttrVector = std::vector<VkVertexInputAttributeDescription>;
    const VertexAttrVector vertex_attribute_descriptions_;

    using VertexAttrAlignmentVector = std::vector<VkDeviceSize>;
    const VertexAttrAlignmentVector vertex_attribute_alignments_;

    using VertexBindingIndexMap = layer_data::unordered_map<uint32_t, uint32_t>;
    const VertexBindingIndexMap vertex_binding_to_index_map_;

    using AttachmentVector = std::vector<VkPipelineColorBlendAttachmentState>;
    const AttachmentVector attachments;

    const bool blend_constants_enabled;  // Blend constants enabled for any attachments
    const bool sample_location_enabled;
    // Flag of which shader stages are active for this pipeline
    const uint32_t active_shaders = 0;
    const VkPrimitiveTopology topology_at_rasterizer;
    const bool dual_source_blending_;

    PIPELINE_STATE(const ValidationStateTracker *state_data, const VkGraphicsPipelineCreateInfo *pCreateInfo,
                   std::shared_ptr<const RENDER_PASS_STATE> &&rpstate, std::shared_ptr<const PIPELINE_LAYOUT_STATE> &&layout);

    PIPELINE_STATE(const ValidationStateTracker *state_data, const VkComputePipelineCreateInfo *pCreateInfo,
                   std::shared_ptr<const PIPELINE_LAYOUT_STATE> &&layout);

    PIPELINE_STATE(const ValidationStateTracker *state_data, const VkRayTracingPipelineCreateInfoKHR *pCreateInfo,
                   std::shared_ptr<const PIPELINE_LAYOUT_STATE> &&layout);

    PIPELINE_STATE(const ValidationStateTracker *state_data, const VkRayTracingPipelineCreateInfoNV *pCreateInfo,
                   std::shared_ptr<const PIPELINE_LAYOUT_STATE> &&layout);

    VkPipeline pipeline() const { return handle_.Cast<VkPipeline>(); }

    void SetHandle(VkPipeline p) { handle_.handle = CastToUint64(p); }

    inline VkPipelineBindPoint GetPipelineType() const {
        switch (create_info.graphics.sType) {
            case VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO:
                return VK_PIPELINE_BIND_POINT_GRAPHICS;
            case VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO:
                return VK_PIPELINE_BIND_POINT_COMPUTE;
            case VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV:
                return VK_PIPELINE_BIND_POINT_RAY_TRACING_NV;
            case VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR:
                return VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR;
            default:
                assert(false);
                return VK_PIPELINE_BIND_POINT_MAX_ENUM;
        }
    }

    inline VkPipelineCreateFlags GetPipelineCreateFlags() const {
        switch (create_info.graphics.sType) {
            case VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO:
                return create_info.graphics.flags;
            case VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO:
                return create_info.compute.flags;
            case VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV:
            case VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR:
                return create_info.raytracing.flags;
            default:
                assert(false);
                return 0;
        }
    }
};

// Track last states that are bound per pipeline bind point (Gfx & Compute)
struct LAST_BOUND_STATE {
    LAST_BOUND_STATE() { Reset(); }  // must define default constructor for portability reasons
    PIPELINE_STATE *pipeline_state;
    VkPipelineLayout pipeline_layout;
    std::shared_ptr<cvdescriptorset::DescriptorSet> push_descriptor_set;

    // Ordered bound set tracking where index is set# that given set is bound to
    struct PER_SET {
        std::shared_ptr<cvdescriptorset::DescriptorSet> bound_descriptor_set;
        // one dynamic offset per dynamic descriptor bound to this CB
        std::vector<uint32_t> dynamicOffsets;
        PipelineLayoutCompatId compat_id_for_set{0};

        // Cache most recently validated descriptor state for ValidateCmdBufDrawState/UpdateDrawState
        const cvdescriptorset::DescriptorSet *validated_set{nullptr};
        uint64_t validated_set_change_count{~0ULL};
        uint64_t validated_set_image_layout_change_count{~0ULL};
        BindingReqMap validated_set_binding_req_map;
    };

    std::vector<PER_SET> per_set;

    void Reset();

    void UnbindAndResetPushDescriptorSet(CMD_BUFFER_STATE *cb_state, std::shared_ptr<cvdescriptorset::DescriptorSet> &&ds);

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
