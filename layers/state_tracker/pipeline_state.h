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
#include "utils/hash_vk_types.h"
#include "state_tracker/base_node.h"
#include "state_tracker/sampler_state.h"
#include "state_tracker/ray_tracing_state.h"
#include "state_tracker/render_pass_state.h"
#include "state_tracker/shader_module.h"
#include "state_tracker/pipeline_layout_state.h"
#include "state_tracker/pipeline_sub_state.h"
#include "generated/dynamic_state_helper.h"
#include "utils/shader_utils.h"

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

class PIPELINE_CACHE_STATE : public BASE_NODE {
  public:
    PIPELINE_CACHE_STATE(VkPipelineCache pipeline_cache, const VkPipelineCacheCreateInfo *pCreateInfo)
        : BASE_NODE(pipeline_cache, kVulkanObjectTypePipelineCache), create_info(pCreateInfo) {}

    VkPipelineCache pipelineCache() const { return handle_.Cast<VkPipelineCache>(); }

    const safe_VkPipelineCacheCreateInfo create_info;
};

struct StageCreateInfo {
    const PIPELINE_STATE *pipeline;

    const PushConstantRangesId shader_object_const_ranges;

    std::vector<VkPushConstantRange> const *GetPushConstantRanges() const;

    StageCreateInfo(const PIPELINE_STATE *pipeline);
    StageCreateInfo(const VkShaderCreateInfoEXT &create_info);
};

class PIPELINE_STATE : public BASE_NODE {
  public:
    union CreateInfo {
        template <typename CI>
        struct Traits {};

        CreateInfo(const VkGraphicsPipelineCreateInfo &ci, std::shared_ptr<const RENDER_PASS_STATE> rpstate,
                   const ValidationStateTracker *state_data)
            : graphics() {
            bool use_color = false;
            bool use_depth_stencil = false;

            if (ci.renderPass == VK_NULL_HANDLE) {
                auto dynamic_rendering = vku::FindStructInPNextChain<VkPipelineRenderingCreateInfo>(ci.pNext);
                if (dynamic_rendering) {
                    use_color = (dynamic_rendering->colorAttachmentCount > 0);
                    use_depth_stencil = (dynamic_rendering->depthAttachmentFormat != VK_FORMAT_UNDEFINED) ||
                                        (dynamic_rendering->stencilAttachmentFormat != VK_FORMAT_UNDEFINED);
                }
            } else if (rpstate) {
                use_color = rpstate->UsesColorAttachment(ci.subpass);
                use_depth_stencil = rpstate->UsesDepthStencilAttachment(ci.subpass);
            }

            PNextCopyState copy_state = {
                [state_data, &ci](VkBaseOutStructure *safe_struct, const VkBaseOutStructure *in_struct) -> bool {
                    return PIPELINE_STATE::PnextRenderingInfoCustomCopy(state_data, ci, safe_struct, in_struct);
                }};
            graphics.initialize(&ci, use_color, use_depth_stencil, &copy_state);
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
    // Create Info values saved for fast access later
    const VkPipelineRenderingCreateInfo *rendering_create_info = nullptr;
    const VkPipelineLibraryCreateInfoKHR *library_create_info = nullptr;
    VkGraphicsPipelineLibraryFlagsEXT graphics_lib_type = static_cast<VkGraphicsPipelineLibraryFlagsEXT>(0);
    VkPipelineBindPoint pipeline_type;
    VkPipelineCreateFlags create_flags;
    vvl::span<const safe_VkPipelineShaderStageCreateInfo> shader_stages_ci;
    const safe_VkPipelineLibraryCreateInfoKHR *ray_tracing_library_ci = nullptr;
    // If using a shader module identifier, the module itself is not validated, but the shader stage is still known
    const bool uses_shader_module_id;

    // State split up based on library types
    const std::shared_ptr<VertexInputState> vertex_input_state;  // VK_GRAPHICS_PIPELINE_LIBRARY_VERTEX_INPUT_INTERFACE_BIT_EXT
    const std::shared_ptr<PreRasterState> pre_raster_state;      // VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT
    const std::shared_ptr<FragmentShaderState> fragment_shader_state;  // VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT
    const std::shared_ptr<FragmentOutputState>
        fragment_output_state;  // VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT

    // Additional metadata needed by pipeline_state initialization and validation
    const StageStateVec stage_states;

    // Shaders from the pipeline create info
    // Normally used for validating pipeline creation, if stages are linked, they will already have been validated
    const VkShaderStageFlags create_info_shaders = 0;
    // Shaders being linked in, don't need to be re-validated
    const VkShaderStageFlags linking_shaders = 0;
    // Flag of which shader stages are active for this pipeline
    // create_info_shaders + linking_shaders
    const VkShaderStageFlags active_shaders = 0;

    const vvl::unordered_set<uint32_t> fragmentShader_writable_output_location_list;

    // NOTE: this map is 'almost' const and used in performance critical code paths.
    // The values of existing entries in the samplers_used_by_image map
    // are updated at various times. Locking requirements are TBD.
    const ActiveSlotMap active_slots;
    const uint32_t max_active_slot = 0;  // the highest set number in active_slots for pipeline layout compatibility checks

    // Which state is dynamic from pipeline creation
    CBDynamicFlags dynamic_state;

    const VkPrimitiveTopology topology_at_rasterizer = VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
    const bool descriptor_buffer_mode = false;
    const bool uses_pipeline_robustness;
    bool ignore_color_attachments;

    CreateShaderModuleStates *csm_states = nullptr;

    // Executable or legacy pipeline
    PIPELINE_STATE(const ValidationStateTracker *state_data, const VkGraphicsPipelineCreateInfo *pCreateInfo,
                   std::shared_ptr<const RENDER_PASS_STATE> &&rpstate, std::shared_ptr<const PIPELINE_LAYOUT_STATE> &&layout,
                   CreateShaderModuleStates *csm_states = nullptr);

    // Compute pipeline
    PIPELINE_STATE(const ValidationStateTracker *state_data, const VkComputePipelineCreateInfo *pCreateInfo,
                   std::shared_ptr<const PIPELINE_LAYOUT_STATE> &&layout, CreateShaderModuleStates *csm_states = nullptr);

    PIPELINE_STATE(const ValidationStateTracker *state_data, const VkRayTracingPipelineCreateInfoKHR *pCreateInfo,
                   std::shared_ptr<const PIPELINE_LAYOUT_STATE> &&layout, CreateShaderModuleStates *csm_states = nullptr);

    PIPELINE_STATE(const ValidationStateTracker *state_data, const VkRayTracingPipelineCreateInfoNV *pCreateInfo,
                   std::shared_ptr<const PIPELINE_LAYOUT_STATE> &&layout, CreateShaderModuleStates *csm_states = nullptr);

    VkPipeline pipeline() const { return handle_.Cast<VkPipeline>(); }

    void SetHandle(VkPipeline p) { handle_.handle = CastToUint64(p); }

    bool IsGraphicsLibrary() const { return !HasFullState(); }
    bool HasFullState() const {
        // First make sure that this pipeline is a "classic" pipeline, or is linked together with the appropriate sub-state
        // libraries
        if (graphics_lib_type && (graphics_lib_type != (VK_GRAPHICS_PIPELINE_LIBRARY_VERTEX_INPUT_INTERFACE_BIT_EXT |
                                                        VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT |
                                                        VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT |
                                                        VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT))) {
            return false;
        }

        // If Pre-raster state does contain a vertex shader, vertex input state is not required
        const bool vi_satisfied = [this]() -> bool {
            if (pre_raster_state && pre_raster_state->vertex_shader) {
                // Vertex shader present, so vertex input state is necessary for complete state
                return static_cast<bool>(vertex_input_state);
            } else {
                // there is no vertex shader, so no vertex input state is required
                return true;
            }
        }();

        // Fragment output/shader state is not required if rasterization is disabled.
        const bool rasterization_disabled = RasterizationDisabled();
        const bool frag_shader_satisfied = rasterization_disabled || static_cast<bool>(fragment_shader_state);
        const bool frag_out_satisfied = rasterization_disabled || static_cast<bool>(fragment_output_state);

        return vi_satisfied && pre_raster_state && frag_shader_satisfied && frag_out_satisfied;
    }

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

    // Used to know if the pipeline substate is being created (as opposed to being linked)
    // Important as some pipeline checks need pipeline state that won't be there if the substate is from linking
    bool OwnsSubState(const std::shared_ptr<PipelineSubState> sub_state) const { return sub_state && (&sub_state->parent == this); }

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

    bool DualSourceBlending() const {
        if (fragment_output_state) {
            return fragment_output_state->dual_source_blending == VK_TRUE;
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

    bool BlendConstantsEnabled() const { return fragment_output_state && fragment_output_state->blend_constants_enabled; }

    bool SampleLocationEnabled() const { return fragment_output_state && fragment_output_state->sample_location_enabled; }

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

    const safe_VkPipelineDynamicStateCreateInfo *DynamicState() const {
        // TODO Each library can contain its own dynamic state (apparently?). Which one should be returned here? Union?
        return create_info.graphics.pDynamicState;
    }

    template <typename CI>
    const typename CreateInfo::Traits<CI>::SafeCreateInfo &GetCreateInfo() const {
        return CreateInfo::Traits<CI>::GetSafeCreateInfo(create_info);
    }

    VkStructureType GetCreateInfoSType() const { return create_info.graphics.sType; }
    const VkPipelineRenderingCreateInfo *GetPipelineRenderingCreateInfo() const { return rendering_create_info; }

    const void *PNext() const { return create_info.graphics.pNext; }

    static StageStateVec GetStageStates(const ValidationStateTracker &state_data, const PIPELINE_STATE &pipe_state,
                                        CreateShaderModuleStates *csm_states);

    // Return true if for a given PSO, the given state enum is dynamic, else return false
    bool IsDynamic(const VkDynamicState state) const { return dynamic_state.test(ConvertToCBDynamicState(state)); }

    // From https://gitlab.khronos.org/vulkan/vulkan/-/issues/3263
    // None of these require VK_EXT_extended_dynamic_state3
    inline bool IsDepthStencilStateDynamic() const {
        return IsDynamic(VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE) && IsDynamic(VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE) &&
               IsDynamic(VK_DYNAMIC_STATE_DEPTH_COMPARE_OP) && IsDynamic(VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE) &&
               IsDynamic(VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE) && IsDynamic(VK_DYNAMIC_STATE_STENCIL_OP) &&
               IsDynamic(VK_DYNAMIC_STATE_DEPTH_BOUNDS);
    }

    // If true, VK_EXT_extended_dynamic_state3 must also have been enabled
    inline bool IsColorBlendStateDynamic() const {
        return IsDynamic(VK_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT) && IsDynamic(VK_DYNAMIC_STATE_LOGIC_OP_EXT) &&
               IsDynamic(VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT) && IsDynamic(VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT) &&
               IsDynamic(VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT) && IsDynamic(VK_DYNAMIC_STATE_BLEND_CONSTANTS);
    }

    template <typename ValidationObject, typename CreateInfo>
    static bool EnablesRasterizationStates(const ValidationObject &vo, const CreateInfo &create_info) {
        // If this is an executable pipeline created from linking graphics libraries, we need to find the pre-raster library to
        // check if rasterization is enabled
        auto link_info = vku::FindStructInPNextChain<VkPipelineLibraryCreateInfoKHR>(create_info.pNext);
        if (link_info) {
            const auto libs = vvl::make_span(link_info->pLibraries, link_info->libraryCount);
            for (const auto handle : libs) {
                auto lib = vo.template Get<PIPELINE_STATE>(handle);
                if (lib && lib->pre_raster_state) {
                    return EnablesRasterizationStates(lib->pre_raster_state);
                }
            }

            // Getting here indicates this is a set of linked libraries, but does not link to a valid pre-raster library. Assume
            // rasterization is enabled in this case
            return true;
        }

        // Check if rasterization is enabled if this is a graphics library (only known in pre-raster libraries)
        auto lib_info = vku::FindStructInPNextChain<VkGraphicsPipelineLibraryCreateInfoEXT>(create_info.pNext);
        if (lib_info) {
            if (lib_info && (lib_info->flags & VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT)) {
                return EnablesRasterizationStates(create_info);
            }
            // Assume rasterization is enabled for non-pre-raster state libraries
            return true;
        }

        // This is a "legacy pipeline"
        return EnablesRasterizationStates(create_info);
    }

    template <typename CreateInfo>
    static bool ContainsSubState(const ValidationObject *vo, const CreateInfo &create_info,
                                 VkGraphicsPipelineLibraryFlagsEXT sub_state) {
        constexpr VkGraphicsPipelineLibraryFlagsEXT null_lib = static_cast<VkGraphicsPipelineLibraryFlagsEXT>(0);
        VkGraphicsPipelineLibraryFlagsEXT current_state = null_lib;

        // Check linked libraries
        auto link_info = vku::FindStructInPNextChain<VkPipelineLibraryCreateInfoKHR>(create_info.pNext);
        if (link_info) {
            auto state_tracker = dynamic_cast<const ValidationStateTracker *>(vo);
            if (state_tracker) {
                const auto libs = vvl::make_span(link_info->pLibraries, link_info->libraryCount);
                for (const auto handle : libs) {
                    auto lib = state_tracker->Get<PIPELINE_STATE>(handle);
                    current_state |= lib->graphics_lib_type;
                }
            }
        }

        // Check if this is a graphics library
        auto lib_info = vku::FindStructInPNextChain<VkGraphicsPipelineLibraryCreateInfoEXT>(create_info.pNext);
        if (lib_info) {
            current_state |= lib_info->flags;
        }

        if (!link_info && !lib_info) {
            // This is not a graphics pipeline library, and therefore contains all necessary state
            return true;
        }

        return (current_state & sub_state) != null_lib;
    }

    // Version used at dispatch time for stateless VOs
    template <typename CreateInfo>
    static bool ContainsSubState(const CreateInfo &create_info, VkGraphicsPipelineLibraryFlagsEXT sub_state) {
        constexpr VkGraphicsPipelineLibraryFlagsEXT null_lib = static_cast<VkGraphicsPipelineLibraryFlagsEXT>(0);
        VkGraphicsPipelineLibraryFlagsEXT current_state = null_lib;

        auto link_info = vku::FindStructInPNextChain<VkPipelineLibraryCreateInfoKHR>(create_info.pNext);
        // Cannot check linked library state in stateless VO

        // Check if this is a graphics library
        auto lib_info = vku::FindStructInPNextChain<VkGraphicsPipelineLibraryCreateInfoEXT>(create_info.pNext);
        if (lib_info) {
            current_state |= lib_info->flags;
        }

        if (!link_info && !lib_info) {
            // This is not a graphics pipeline library, and therefore (should) contains all necessary state
            return true;
        }

        return (current_state & sub_state) != null_lib;
    }

    // This is a helper that is meant to be used during safe_VkPipelineRenderingCreateInfo construction to determine whether or not
    // certain fields should be ignored based on graphics pipeline state
    static bool PnextRenderingInfoCustomCopy(const ValidationStateTracker *state_data,
                                             const VkGraphicsPipelineCreateInfo &graphics_info, VkBaseOutStructure *safe_struct,
                                             const VkBaseOutStructure *in_struct) {
        // "safe_struct" is assumed to be non-null as it should be the "this" member of calling class instance
        assert(safe_struct);
        if (safe_struct->sType == VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO) {
            const bool has_fo_state = PIPELINE_STATE::ContainsSubState(
                state_data, graphics_info, VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT);
            if (!has_fo_state) {
                // Clear out all pointers except for viewMask. Since viewMask is a scalar, it has already been copied at this point
                // in safe_VkPipelineRenderingCreateInfo construction.
                auto pri = reinterpret_cast<safe_VkPipelineRenderingCreateInfo *>(safe_struct);
                pri->colorAttachmentCount = 0u;
                pri->depthAttachmentFormat = VK_FORMAT_UNDEFINED;
                pri->stencilAttachmentFormat = VK_FORMAT_UNDEFINED;

                // Signal that we do not want the "normal" safe struct initialization to run
                return true;
            }
        }
        // Signal that the custom initialization was not used
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

    template <typename CreateInfo>
    static bool EnablesRasterizationStates(const CreateInfo &create_info) {
        if (create_info.pDynamicState && create_info.pDynamicState->pDynamicStates) {
            for (uint32_t i = 0; i < create_info.pDynamicState->dynamicStateCount; ++i) {
                if (create_info.pDynamicState->pDynamicStates[i] == VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE_EXT) {
                    // If RASTERIZER_DISCARD_ENABLE is dynamic, then we must return true (i.e., rasterization is enabled)
                    // NOTE: create_info must contain pre-raster state, otherwise it is an invalid pipeline and will trigger
                    //       an error outside of this function.
                    return true;
                }
            }
        }

        // Return rasterization state from create info if it will not be set dynamically
        if (create_info.pRasterizationState) {
            return create_info.pRasterizationState->rasterizerDiscardEnable == VK_FALSE;
        }

        // Getting here indicates create_info represents a pipeline that does not contain pre-raster state
        // Return true, though the return value _shouldn't_ matter in such cases
        return true;
    }

    static bool EnablesRasterizationStates(const std::shared_ptr<PreRasterState> pre_raster_state) {
        if (!pre_raster_state) {
            // Assume rasterization is enabled if we don't know for sure that it is disabled
            return true;
        }
        return EnablesRasterizationStates(pre_raster_state->parent.create_info.graphics);
    }

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
    // All shader stages for a used pipeline bind point must be bound to with a valid shader or VK_NULL_HANDLE
    // We have to track shader_object_bound, because shader_object_states will be nullptr when VK_NULL_HANDLE is used
    bool shader_object_bound[SHADER_OBJECT_STAGE_COUNT]{false};
    SHADER_OBJECT_STATE *shader_object_states[SHADER_OBJECT_STAGE_COUNT]{nullptr};
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

        // Cache most recently validated descriptor state for ValidateActionState/UpdateDrawState
        const cvdescriptorset::DescriptorSet *validated_set{nullptr};
        uint64_t validated_set_change_count{~0ULL};
        uint64_t validated_set_image_layout_change_count{~0ULL};
        BindingVariableMap validated_set_binding_req_map;

        void Reset() {
            bound_descriptor_set.reset();
            bound_descriptor_buffer.reset();
            dynamicOffsets.clear();
        }
    };

    std::vector<PER_SET> per_set;

    void Reset();

    void UnbindAndResetPushDescriptorSet(std::shared_ptr<cvdescriptorset::DescriptorSet> &&ds);

    inline bool IsUsing() const { return pipeline_state != nullptr; }

    // Dynamic State helpers that require both the Pipeline and CommandBuffer state are here
    bool IsDepthTestEnable() const;
    bool IsDepthBoundTestEnable() const;
    bool IsDepthWriteEnable() const;
    bool IsStencilTestEnable() const;
    VkStencilOpState GetStencilOpStateFront() const;
    VkStencilOpState GetStencilOpStateBack() const;
    VkSampleCountFlagBits GetRasterizationSamples() const;
    bool IsRasterizationDisabled() const;
    VkColorComponentFlags GetColorWriteMask(uint32_t i) const;
    bool IsColorWriteEnabled(uint32_t i) const;

    bool ValidShaderObjectCombination(const VkPipelineBindPoint bind_point, const DeviceFeatures &device_features) const;
    VkShaderEXT GetShader(ShaderObjectStage stage) const;
    SHADER_OBJECT_STATE *GetShaderState(ShaderObjectStage stage) const;
    bool HasShaderObjects() const;
    bool IsValidShaderBound(ShaderObjectStage stage) const;
    bool IsValidShaderOrNullBound(ShaderObjectStage stage) const;
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

static VkPipelineBindPoint inline ConvertToPipelineBindPoint(VkShaderStageFlagBits stage) {
    switch (stage) {
        case VK_SHADER_STAGE_VERTEX_BIT:
        case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
        case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
        case VK_SHADER_STAGE_GEOMETRY_BIT:
        case VK_SHADER_STAGE_FRAGMENT_BIT:
        case VK_SHADER_STAGE_TASK_BIT_EXT:
        case VK_SHADER_STAGE_MESH_BIT_EXT:
            return VK_PIPELINE_BIND_POINT_GRAPHICS;
        case VK_SHADER_STAGE_COMPUTE_BIT:
            return VK_PIPELINE_BIND_POINT_COMPUTE;
        case VK_SHADER_STAGE_RAYGEN_BIT_KHR:
        case VK_SHADER_STAGE_ANY_HIT_BIT_KHR:
        case VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR:
        case VK_SHADER_STAGE_MISS_BIT_KHR:
        case VK_SHADER_STAGE_INTERSECTION_BIT_KHR:
        case VK_SHADER_STAGE_CALLABLE_BIT_KHR:
            return VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR;
        default:
            return static_cast<VkPipelineBindPoint>(stage);
    }
    return VK_PIPELINE_BIND_POINT_MAX_ENUM;
}

static LvlBindPoint inline ConvertToLvlBindPoint(VkShaderStageFlagBits stage) {
    switch (stage) {
        case VK_SHADER_STAGE_VERTEX_BIT:
        case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
        case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
        case VK_SHADER_STAGE_GEOMETRY_BIT:
        case VK_SHADER_STAGE_FRAGMENT_BIT:
        case VK_SHADER_STAGE_TASK_BIT_EXT:
        case VK_SHADER_STAGE_MESH_BIT_EXT:
            return BindPoint_Graphics;
        case VK_SHADER_STAGE_COMPUTE_BIT:
            return BindPoint_Compute;
        case VK_SHADER_STAGE_RAYGEN_BIT_KHR:
        case VK_SHADER_STAGE_ANY_HIT_BIT_KHR:
        case VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR:
        case VK_SHADER_STAGE_MISS_BIT_KHR:
        case VK_SHADER_STAGE_INTERSECTION_BIT_KHR:
        case VK_SHADER_STAGE_CALLABLE_BIT_KHR:
            return BindPoint_Ray_Tracing;
        default:
            return static_cast<LvlBindPoint>(stage);
    }
    return BindPoint_Count;
}

static ShaderObjectStage inline ConvertToShaderObjectStage(VkShaderStageFlagBits stage) {
    if (stage == VK_SHADER_STAGE_VERTEX_BIT) return ShaderObjectStage::VERTEX;
    if (stage == VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT) return ShaderObjectStage::TESSELLATION_CONTROL;
    if (stage == VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT) return ShaderObjectStage::TESSELLATION_EVALUATION;
    if (stage == VK_SHADER_STAGE_GEOMETRY_BIT) return ShaderObjectStage::GEOMETRY;
    if (stage == VK_SHADER_STAGE_FRAGMENT_BIT) return ShaderObjectStage::FRAGMENT;
    if (stage == VK_SHADER_STAGE_COMPUTE_BIT) return ShaderObjectStage::COMPUTE;
    if (stage == VK_SHADER_STAGE_TASK_BIT_EXT) return ShaderObjectStage::TASK;
    if (stage == VK_SHADER_STAGE_MESH_BIT_EXT) return ShaderObjectStage::MESH;

    assert(false);

    return ShaderObjectStage::LAST;
}
