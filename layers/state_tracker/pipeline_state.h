/* Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (C) 2015-2023 Google Inc.
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
 */
#pragma once
#include "hash_vk_types.h"
#include "state_tracker/base_node.h"
#include "state_tracker/sampler_state.h"
#include "state_tracker/ray_tracing_state.h"
#include "state_tracker/render_pass_state.h"
#include "state_tracker/shader_module.h"
#include "state_tracker/pipeline_layout_state.h"
#include "state_tracker/pipeline_sub_state.h"

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
    DESCRIPTOR_REQ_SAMPLER_SAMPLED = DESCRIPTOR_REQ_VIEW_ATOMIC_OPERATION << 1,
    DESCRIPTOR_REQ_SAMPLER_IMPLICITLOD_DREF_PROJ = DESCRIPTOR_REQ_SAMPLER_SAMPLED << 1,
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
    // Copy from StageState.ResourceInterfaceVariable. It combines from plural shader stages. The index of array is index of image.
    std::vector<vvl::unordered_set<SamplerUsedByImage>> samplers_used_by_image;
    // For storage images - list of < OpImageWrite : Texel component length >
    std::vector<std::pair<Instruction, uint32_t>> write_without_formats_component_count_list;
    // OpTypeImage the variable points to, null if doesn't use the image
    uint32_t image_sampled_type_width = 0;
    DescriptorRequirement() : reqs(0), is_writable(false) {}
};

inline bool operator==(const DescriptorRequirement &a, const DescriptorRequirement &b) noexcept { return a.reqs == b.reqs; }

inline bool operator<(const DescriptorRequirement &a, const DescriptorRequirement &b) noexcept { return a.reqs < b.reqs; }

// < binding index (of descriptor set) : meta data >
typedef std::map<uint32_t, DescriptorRequirement> BindingReqMap;

struct PipelineStageState {
    std::shared_ptr<const SHADER_MODULE_STATE> module_state;
    const safe_VkPipelineShaderStageCreateInfo *create_info;
    VkShaderStageFlagBits stage_flag;
    std::optional<Instruction> entrypoint;
    const std::vector<ResourceInterfaceVariable> *descriptor_variables = {};
    bool has_writable_descriptor;
    bool has_atomic_descriptor;
    bool wrote_primitive_shading_rate;
    bool writes_to_gl_layer;
    bool has_input_attachment_capability;

    PipelineStageState(const safe_VkPipelineShaderStageCreateInfo *stage, std::shared_ptr<const SHADER_MODULE_STATE> &module_state);
};

class PIPELINE_STATE : public BASE_NODE {
  public:
    union CreateInfo {
        template <typename CI>
        struct Traits {};

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
            } else if (rpstate) {
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

  protected:
    // NOTE: The style guide suggests private data appear at the end, but we need this populated first, so placing it here

    // Render pass state for dynamic rendering, etc.
    std::shared_ptr<const RENDER_PASS_STATE> rp_state;

    const CreateInfo create_info;

  public:
    const uint32_t create_index;  // which index in pCreateInfos, used for error messages
    VkGraphicsPipelineLibraryFlagsEXT graphics_lib_type = static_cast<VkGraphicsPipelineLibraryFlagsEXT>(0);
    // State split up based on library types
    const std::shared_ptr<VertexInputState> vertex_input_state;  // VK_GRAPHICS_PIPELINE_LIBRARY_VERTEX_INPUT_INTERFACE_BIT_EXT
    const std::shared_ptr<PreRasterState> pre_raster_state;      // VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT
    const std::shared_ptr<FragmentShaderState> fragment_shader_state;  // VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT
    const std::shared_ptr<FragmentOutputState>
        fragment_output_state;  // VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT

    const VkPipelineRenderingCreateInfo *rendering_create_info = nullptr;

    // Additional metadata needed by pipeline_state initialization and validation
    using StageStateVec = std::vector<PipelineStageState>;
    const StageStateVec stage_state;
    bool HasShaderStage(VkShaderStageFlagBits stage_flag) const {
        for (const auto &state : stage_state) {
            if (state.stage_flag == stage_flag) {
                return true;
            }
        }
        return false;
    }

    const vvl::unordered_set<uint32_t> fragmentShader_writable_output_location_list;

    // Capture which slots (set#->bindings) are actually used by the shaders of this pipeline
    using ActiveSlotMap = vvl::unordered_map<uint32_t, BindingReqMap>;
    // NOTE: this map is 'almost' const and used in performance critical code paths.
    // The values of existing entries in the DescriptorRequirement::samplers_used_by_image map
    // are updated at various times. Locking requirements are TBD.
    const ActiveSlotMap active_slots;
    const uint32_t max_active_slot = 0;  // the highest set number in active_slots for pipeline layout compatibility checks

    // Flag of which shader stages are active for this pipeline
    const uint32_t active_shaders = 0;
    const VkPrimitiveTopology topology_at_rasterizer = VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
    const bool uses_shader_module_id;
    const bool descriptor_buffer_mode = false;
    const bool uses_pipeline_robustness;

    CreateShaderModuleStates *csm_states = nullptr;

    // Executable or legacy pipeline
    PIPELINE_STATE(const ValidationStateTracker *state_data, const VkGraphicsPipelineCreateInfo *pCreateInfo, uint32_t create_index,
                   std::shared_ptr<const RENDER_PASS_STATE> &&rpstate, std::shared_ptr<const PIPELINE_LAYOUT_STATE> &&layout,
                   CreateShaderModuleStates *csm_states = nullptr);

    // Compute pipeline
    PIPELINE_STATE(const ValidationStateTracker *state_data, const VkComputePipelineCreateInfo *pCreateInfo, uint32_t create_index,
                   std::shared_ptr<const PIPELINE_LAYOUT_STATE> &&layout, CreateShaderModuleStates *csm_states = nullptr);

    PIPELINE_STATE(const ValidationStateTracker *state_data, const VkRayTracingPipelineCreateInfoKHR *pCreateInfo,
                   uint32_t create_index, std::shared_ptr<const PIPELINE_LAYOUT_STATE> &&layout,
                   CreateShaderModuleStates *csm_states = nullptr);

    PIPELINE_STATE(const ValidationStateTracker *state_data, const VkRayTracingPipelineCreateInfoNV *pCreateInfo,
                   uint32_t create_index, std::shared_ptr<const PIPELINE_LAYOUT_STATE> &&layout,
                   CreateShaderModuleStates *csm_states = nullptr);

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

    inline const char *GetCreateFunctionName() const {
        switch (create_info.graphics.sType) {
            case VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO:
                return "vkCreateGraphicsPipelines";
            case VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO:
                return "vkCreateComputePipelines";
            case VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV:
                return "vkCreateRayTracingPipelinesNV";
            case VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR:
                return "vkCreateRayTracingPipelinesKHR";
            default:
                assert(false);
                return "";
        }
    }

    bool IsGraphicsLibrary() const { return !HasFullState(); }
    bool HasFullState() const { return vertex_input_state && pre_raster_state && fragment_shader_state && fragment_output_state; }

    template <VkGraphicsPipelineLibraryFlagBitsEXT type_flag>
    struct SubStateTraits {};

    template <VkGraphicsPipelineLibraryFlagBitsEXT type_flag>
    static inline typename SubStateTraits<type_flag>::type GetSubState(const PIPELINE_STATE &) {
        return {};
    }

    std::shared_ptr<const SHADER_MODULE_STATE> GetSubStateShader(VkShaderStageFlagBits state) const;

    template <VkGraphicsPipelineLibraryFlagBitsEXT type_flag>
    static inline typename SubStateTraits<type_flag>::type GetLibSubState(const ValidationStateTracker &state,
                                                                          const VkPipelineLibraryCreateInfoKHR &link_info) {
        for (uint32_t i = 0; i < link_info.libraryCount; ++i) {
            const auto lib_state = state.Get<PIPELINE_STATE>(link_info.pLibraries[i]);
            if (lib_state && ((lib_state->graphics_lib_type & type_flag) != 0)) {
                return GetSubState<type_flag>(*lib_state);
            }
        }
        return {};
    }

    const std::shared_ptr<const RENDER_PASS_STATE> RenderPassState() const {
        // TODO A render pass object is required for all of these sub-states. Which one should be used for an "executable pipeline"?
        if (fragment_output_state && fragment_output_state->rp_state) {
            return fragment_output_state->rp_state;
        } else if (fragment_shader_state && fragment_shader_state->rp_state) {
            return fragment_shader_state->rp_state;
        } else if (pre_raster_state && pre_raster_state->rp_state) {
            return pre_raster_state->rp_state;
        }
        return rp_state;
    }

    bool IsRenderPassStateRequired() const { return pre_raster_state || fragment_shader_state || fragment_output_state; }

    const std::shared_ptr<const PIPELINE_LAYOUT_STATE> PipelineLayoutState() const {
        // TODO A render pass object is required for all of these sub-states. Which one should be used for an "executable pipeline"?
        if (merged_graphics_layout) {
            return merged_graphics_layout;
        } else if (pre_raster_state) {
            return pre_raster_state->pipeline_layout;
        } else if (fragment_shader_state) {
            return fragment_shader_state->pipeline_layout;
        }
        return merged_graphics_layout;
    }

    std::vector<std::shared_ptr<const PIPELINE_LAYOUT_STATE>> PipelineLayoutStateUnion() const;

    const std::shared_ptr<const PIPELINE_LAYOUT_STATE> PreRasterPipelineLayoutState() const {
        if (pre_raster_state) {
            return pre_raster_state->pipeline_layout;
        }
        return merged_graphics_layout;
    }

    const std::shared_ptr<const PIPELINE_LAYOUT_STATE> FragmentShaderPipelineLayoutState() const {
        if (fragment_shader_state) {
            return fragment_shader_state->pipeline_layout;
        }
        return merged_graphics_layout;
    }

    const safe_VkPipelineMultisampleStateCreateInfo *MultisampleState() const {
        // TODO A render pass object is required for all of these sub-states. Which one should be used for an "executable pipeline"?
        if (fragment_shader_state && fragment_shader_state->ms_state &&
            (fragment_shader_state->ms_state->rasterizationSamples >= VK_SAMPLE_COUNT_1_BIT) &&
            (fragment_shader_state->ms_state->rasterizationSamples < VK_SAMPLE_COUNT_FLAG_BITS_MAX_ENUM)) {
            return fragment_shader_state->ms_state.get();
        } else if (fragment_output_state && fragment_output_state->ms_state &&
                   (fragment_output_state->ms_state->rasterizationSamples >= VK_SAMPLE_COUNT_1_BIT) &&
                   (fragment_output_state->ms_state->rasterizationSamples < VK_SAMPLE_COUNT_FLAG_BITS_MAX_ENUM)) {
            return fragment_output_state->ms_state.get();
        }
        return nullptr;
    }

    const safe_VkPipelineRasterizationStateCreateInfo *RasterizationState() const {
        // TODO A render pass object is required for all of these sub-states. Which one should be used for an "executable pipeline"?
        if (pre_raster_state) {
            return pre_raster_state->raster_state;
        }
        return nullptr;
    }

    bool RasterizationDisabled() const {
        if (pre_raster_state && pre_raster_state->raster_state) {
            return pre_raster_state->raster_state->rasterizerDiscardEnable == VK_TRUE;
        }
        return false;
    }

    const safe_VkPipelineViewportStateCreateInfo *ViewportState() const {
        // TODO A render pass object is required for all of these sub-states. Which one should be used for an "executable pipeline"?
        if (pre_raster_state) {
            return pre_raster_state->viewport_state;
        }
        return nullptr;
    }

    const safe_VkPipelineColorBlendStateCreateInfo *ColorBlendState() const {
        if (fragment_output_state) {
            return fragment_output_state->color_blend_state.get();
        }
        return nullptr;
    }

    const safe_VkPipelineVertexInputStateCreateInfo *InputState() const {
        if (vertex_input_state) {
            return vertex_input_state->input_state;
        }
        return nullptr;
    }

    const safe_VkPipelineInputAssemblyStateCreateInfo *InputAssemblyState() const {
        if (vertex_input_state) {
            return vertex_input_state->input_assembly_state;
        }
        return nullptr;
    }

    const VertexInputState::VertexBindingVector *BindingDescriptions() const {
        if (vertex_input_state) {
            return &vertex_input_state->binding_descriptions;
        }
        return nullptr;
    }

    uint32_t Subpass() const {
        // TODO A render pass object is required for all of these sub-states. Which one should be used for an "executable pipeline"?
        if (pre_raster_state) {
            return pre_raster_state->subpass;
        } else if (fragment_shader_state) {
            return fragment_shader_state->subpass;
        } else if (fragment_output_state) {
            return fragment_output_state->subpass;
        }
        return create_info.graphics.subpass;
    }

    const FragmentOutputState::AttachmentVector &Attachments() const {
        if (fragment_output_state) {
            return fragment_output_state->attachments;
        }
        static FragmentOutputState::AttachmentVector empty_vec = {};
        return empty_vec;
    }

    const safe_VkPipelineDepthStencilStateCreateInfo *DepthStencilState() const {
        if (fragment_shader_state) {
            return fragment_shader_state->ds_state.get();
        }
        return nullptr;
    }

    bool BlendConstantsEnabled() const { return fragment_output_state ? fragment_output_state->blend_constants_enabled : false; }

    bool SampleLocationEnabled() const { return fragment_output_state ? fragment_output_state->sample_location_enabled : false; }

    template <typename CI>
    VkPipeline BasePipeline() const {
        // TODO this _should_ be a static_assert, but that only works on MSVC currently
        assert(false && "Not implemented");
        return {};
    }

    template <typename CI>
    int32_t BasePipelineIndex() const {
        // TODO this _should_ be a static_assert, but that only works on MSVC currently
        assert(false && "Not implemented");
        return {};
    }

    // Return the ith shader module where i indexes into CreateInfo::pStages
    template <typename CI>
    VkShaderModule GetShaderModuleByCIIndex(uint32_t i) {
        // TODO this _should_ be a static_assert, but that only works on MSVC currently
        assert(false && "Not implemented");
        return {};
    }

    std::shared_ptr<const SHADER_MODULE_STATE> GetShaderModuleState(VkShaderStageFlagBits stage) {
        for (auto &s : stage_state) {
            if (s.stage_flag == stage) {
                return s.module_state;
            }
        }
        return {};
    }

    const safe_VkPipelineDynamicStateCreateInfo *DynamicState() const {
        // TODO Each library can contain its own dynamic state (apparently?). Which one should be returned here? Union?
        return create_info.graphics.pDynamicState;
    }

    vvl::span<const safe_VkPipelineShaderStageCreateInfo> GetShaderStages() const {
        switch (create_info.graphics.sType) {
            case VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO:
                return vvl::span<const safe_VkPipelineShaderStageCreateInfo>{create_info.graphics.pStages,
                                                                                    create_info.graphics.stageCount};
            case VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO:
                return vvl::span<const safe_VkPipelineShaderStageCreateInfo>{&create_info.compute.stage, 1};
            case VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV:
                return vvl::span<const safe_VkPipelineShaderStageCreateInfo>{create_info.raytracing.pStages,
                                                                                    create_info.raytracing.stageCount};
            case VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR:
                return vvl::span<const safe_VkPipelineShaderStageCreateInfo>{create_info.raytracing.pStages,
                                                                                    create_info.raytracing.stageCount};
            default:
                assert(false);
                return {};
        }
    }

    const safe_VkPipelineLibraryCreateInfoKHR *GetRayTracingLibraryCreateInfo() const {
        if ((create_info.graphics.sType == VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV) ||
            (create_info.graphics.sType == VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR)) {
            return create_info.raytracing.pLibraryInfo;
        }
        return nullptr;
    }

    template <typename CI>
    const typename CreateInfo::Traits<CI>::SafeCreateInfo &GetCreateInfo() const {
        return CreateInfo::Traits<CI>::GetSafeCreateInfo(create_info);
    }

    VkStructureType GetCreateInfoSType() const { return create_info.graphics.sType; }
    const VkPipelineRenderingCreateInfo *GetPipelineRenderingCreateInfo() const { return rendering_create_info; }

    const void *PNext() const { return create_info.graphics.pNext; }

    static ActiveSlotMap GetActiveSlots(const StageStateVec &stage_states);
    static StageStateVec GetStageStates(const ValidationStateTracker &state_data, const PIPELINE_STATE &pipe_state,
                                        CreateShaderModuleStates *csm_states);

    // Return true if for a given PSO, the given state enum is dynamic, else return false
    bool IsDynamic(const VkDynamicState state) const {
        const auto *dynamic_state = DynamicState();
        if ((GetPipelineType() == VK_PIPELINE_BIND_POINT_GRAPHICS) && dynamic_state) {
            for (uint32_t i = 0; i < dynamic_state->dynamicStateCount; i++) {
                if (state == dynamic_state->pDynamicStates[i]) return true;
            }
        }
        return false;
    }

  protected:
    static std::shared_ptr<VertexInputState> CreateVertexInputState(const PIPELINE_STATE &p, const ValidationStateTracker &state,
                                                                    const safe_VkGraphicsPipelineCreateInfo &create_info);
    static std::shared_ptr<PreRasterState> CreatePreRasterState(const PIPELINE_STATE &p, const ValidationStateTracker &state,
                                                                const safe_VkGraphicsPipelineCreateInfo &create_info,
                                                                const std::shared_ptr<const RENDER_PASS_STATE> &rp);
    static std::shared_ptr<FragmentShaderState> CreateFragmentShaderState(const PIPELINE_STATE &p,
                                                                          const ValidationStateTracker &state,
                                                                          const VkGraphicsPipelineCreateInfo &create_info,
                                                                          const safe_VkGraphicsPipelineCreateInfo &safe_create_info,
                                                                          const std::shared_ptr<const RENDER_PASS_STATE> &rp);
    static std::shared_ptr<FragmentOutputState> CreateFragmentOutputState(const PIPELINE_STATE &p,
                                                                          const ValidationStateTracker &state,
                                                                          const VkGraphicsPipelineCreateInfo &create_info,
                                                                          const safe_VkGraphicsPipelineCreateInfo &safe_create_info,
                                                                          const std::shared_ptr<const RENDER_PASS_STATE> &rp);

    // Merged layouts
    std::shared_ptr<const PIPELINE_LAYOUT_STATE> merged_graphics_layout;
};

template <>
struct PIPELINE_STATE::SubStateTraits<VK_GRAPHICS_PIPELINE_LIBRARY_VERTEX_INPUT_INTERFACE_BIT_EXT> {
    using type = std::shared_ptr<VertexInputState>;
};

// static
template <>
inline PIPELINE_STATE::SubStateTraits<VK_GRAPHICS_PIPELINE_LIBRARY_VERTEX_INPUT_INTERFACE_BIT_EXT>::type
PIPELINE_STATE::GetSubState<VK_GRAPHICS_PIPELINE_LIBRARY_VERTEX_INPUT_INTERFACE_BIT_EXT>(const PIPELINE_STATE &pipe_state) {
    return pipe_state.vertex_input_state;
}

template <>
struct PIPELINE_STATE::SubStateTraits<VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT> {
    using type = std::shared_ptr<PreRasterState>;
};

// static
template <>
inline PIPELINE_STATE::SubStateTraits<VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT>::type
PIPELINE_STATE::GetSubState<VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT>(const PIPELINE_STATE &pipe_state) {
    return pipe_state.pre_raster_state;
}

template <>
struct PIPELINE_STATE::SubStateTraits<VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT> {
    using type = std::shared_ptr<FragmentShaderState>;
};

// static
template <>
inline PIPELINE_STATE::SubStateTraits<VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT>::type
PIPELINE_STATE::GetSubState<VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT>(const PIPELINE_STATE &pipe_state) {
    return pipe_state.fragment_shader_state;
}

template <>
struct PIPELINE_STATE::SubStateTraits<VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT> {
    using type = std::shared_ptr<FragmentOutputState>;
};

// static
template <>
inline PIPELINE_STATE::SubStateTraits<VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT>::type
PIPELINE_STATE::GetSubState<VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT>(const PIPELINE_STATE &pipe_state) {
    return pipe_state.fragment_output_state;
}

template <>
struct PIPELINE_STATE::CreateInfo::Traits<VkGraphicsPipelineCreateInfo> {
    using SafeCreateInfo = safe_VkGraphicsPipelineCreateInfo;
    static const SafeCreateInfo &GetSafeCreateInfo(const CreateInfo &ci) {
        assert(ci.graphics.sType == VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO);
        return ci.graphics;
    }
};

template <>
struct PIPELINE_STATE::CreateInfo::Traits<VkComputePipelineCreateInfo> {
    using SafeCreateInfo = safe_VkComputePipelineCreateInfo;
    static const SafeCreateInfo &GetSafeCreateInfo(const CreateInfo &ci) {
        assert(ci.compute.sType == VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO);
        return ci.compute;
    }
};

template <>
struct PIPELINE_STATE::CreateInfo::Traits<VkRayTracingPipelineCreateInfoKHR> {
    using SafeCreateInfo = safe_VkRayTracingPipelineCreateInfoCommon;
    static const SafeCreateInfo &GetSafeCreateInfo(const CreateInfo &ci) {
        assert(ci.raytracing.sType == VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR);
        return ci.raytracing;
    }
};

template <>
struct PIPELINE_STATE::CreateInfo::Traits<VkRayTracingPipelineCreateInfoNV> {
    using SafeCreateInfo = safe_VkRayTracingPipelineCreateInfoCommon;
    static const SafeCreateInfo &GetSafeCreateInfo(const CreateInfo &ci) {
        assert(ci.raytracing.sType == VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV);
        return ci.raytracing;
    }
};

template <>
VkPipeline PIPELINE_STATE::BasePipeline<VkGraphicsPipelineCreateInfo>() const;
template <>
VkPipeline PIPELINE_STATE::BasePipeline<VkComputePipelineCreateInfo>() const;
template <>
VkPipeline PIPELINE_STATE::BasePipeline<VkRayTracingPipelineCreateInfoKHR>() const;
template <>
VkPipeline PIPELINE_STATE::BasePipeline<VkRayTracingPipelineCreateInfoNV>() const;

template <>
int32_t PIPELINE_STATE::BasePipelineIndex<VkGraphicsPipelineCreateInfo>() const;
template <>
int32_t PIPELINE_STATE::BasePipelineIndex<VkComputePipelineCreateInfo>() const;
template <>
int32_t PIPELINE_STATE::BasePipelineIndex<VkRayTracingPipelineCreateInfoKHR>() const;
template <>
int32_t PIPELINE_STATE::BasePipelineIndex<VkRayTracingPipelineCreateInfoNV>() const;

template <>
VkShaderModule PIPELINE_STATE::PIPELINE_STATE::GetShaderModuleByCIIndex<VkGraphicsPipelineCreateInfo>(uint32_t i);
template <>
VkShaderModule PIPELINE_STATE::GetShaderModuleByCIIndex<VkComputePipelineCreateInfo>(uint32_t);
template <>
VkShaderModule PIPELINE_STATE::GetShaderModuleByCIIndex<VkRayTracingPipelineCreateInfoKHR>(uint32_t i);
template <>
VkShaderModule PIPELINE_STATE::GetShaderModuleByCIIndex<VkRayTracingPipelineCreateInfoNV>(uint32_t i);

// Track last states that are bound per pipeline bind point (Gfx & Compute)
struct LAST_BOUND_STATE {
    LAST_BOUND_STATE(CMD_BUFFER_STATE &cb) : cb_state(cb) {}

    CMD_BUFFER_STATE &cb_state;
    PIPELINE_STATE *pipeline_state{nullptr};
    VkPipelineLayout pipeline_layout{VK_NULL_HANDLE};
    std::shared_ptr<cvdescriptorset::DescriptorSet> push_descriptor_set;

    struct DescriptorBufferBinding {
        uint32_t index{0};
        VkDeviceSize offset{0};
    };
    // Ordered bound set tracking where index is set# that given set is bound to
    struct PER_SET {
        std::shared_ptr<cvdescriptorset::DescriptorSet> bound_descriptor_set;
        std::optional<DescriptorBufferBinding> bound_descriptor_buffer;

        // one dynamic offset per dynamic descriptor bound to this CB
        std::vector<uint32_t> dynamicOffsets;
        PipelineLayoutCompatId compat_id_for_set{0};

        // Cache most recently validated descriptor state for ValidateCmdBufDrawState/UpdateDrawState
        const cvdescriptorset::DescriptorSet *validated_set{nullptr};
        uint64_t validated_set_change_count{~0ULL};
        uint64_t validated_set_image_layout_change_count{~0ULL};
        BindingReqMap validated_set_binding_req_map;

        void Reset() {
            bound_descriptor_set.reset();
            bound_descriptor_buffer.reset();
            dynamicOffsets.clear();
        }
    };

    std::vector<PER_SET> per_set;

    void Reset();

    void UnbindAndResetPushDescriptorSet(std::shared_ptr<cvdescriptorset::DescriptorSet> &&ds);

    inline bool IsUsing() const { return pipeline_state ? true : false; }
};

static inline bool IsBoundSetCompat(uint32_t set, const LAST_BOUND_STATE &last_bound,
                                    const PIPELINE_LAYOUT_STATE &pipeline_layout) {
    if ((set >= last_bound.per_set.size()) || (set >= pipeline_layout.set_compat_ids.size())) {
        return false;
    }
    return (*(last_bound.per_set[set].compat_id_for_set) == *(pipeline_layout.set_compat_ids[set]));
}

static inline bool IsPipelineLayoutSetCompat(uint32_t set, const PIPELINE_LAYOUT_STATE *a, const PIPELINE_LAYOUT_STATE *b) {
    if (!a || !b) {
        return false;
    }
    if ((set >= a->set_compat_ids.size()) || (set >= b->set_compat_ids.size())) {
        return false;
    }
    return a->set_compat_ids[set] == b->set_compat_ids[set];
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
