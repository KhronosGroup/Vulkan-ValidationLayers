/* Copyright (c) 2015-2026 The Khronos Group Inc.
 * Copyright (c) 2015-2026 Valve Corporation
 * Copyright (c) 2015-2026 LunarG, Inc.
 * Copyright (C) 2015-2026 Google Inc.
 * Copyright (c) 2025 Arm Limited.
 * Modifications Copyright (C) 2020,2025-2026 Advanced Micro Devices, Inc. All rights reserved.
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

#include <cassert>
#include <cinttypes>
#include <cstdint>
#include <memory>
#include <spirv/unified1/spirv.hpp>
#include <sstream>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "containers/custom_containers.h"
#include "error_message/error_location.h"
#include "error_message/error_strings.h"
#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/utility/vk_format_utils.h>
#include <vulkan/vulkan_core.h>
#include "core_checks/cc_vuid_maps.h"
#include "core_validation.h"
#include "generated/spirv_grammar_helper.h"
#include "generated/spirv_validation_helper.h"
#include "state_tracker/shader_instruction.h"
#include "state_tracker/shader_module.h"
#include "state_tracker/shader_stage_state.h"
#include "state_tracker/pipeline_state.h"
#include "utils/assert_utils.h"
#include "utils/shader_utils.h"
#include "utils/hash_util.h"
#include "utils/descriptor_utils.h"
#include "chassis/chassis_modification_state.h"
#include "state_tracker/descriptor_sets.h"
#include "state_tracker/descriptor_set_layouts.h"
#include "state_tracker/render_pass_state.h"
#include "spirv-tools/optimizer.hpp"
#include "containers/limits.h"
#include "containers/container_utils.h"
#include "utils/math_utils.h"

bool CoreChecks::ValidatePushConstantUsage(const spirv::Module& module_state, const spirv::EntryPoint& entrypoint,
                                           const vvl::Pipeline* pipeline, const ShaderStageState& stage_state,
                                           const Location& loc) const {
    bool skip = false;
    const auto push_constant_variable = entrypoint.push_constant_variable;
    if (!push_constant_variable) {
        return skip;
    }

    if (stage_state.heap.descriptor_heap_mode) {
        // In DescriptorModeClassic, this is normally caught binding pipeline layouts (with ranges in them)
        const VkDeviceSize max_size = phys_dev_ext_props.descriptor_heap_props.maxPushDataSize;
        if (push_constant_variable->size > max_size) {
            skip |= LogError("VUID-RuntimeSpirv-maxPushDataSize-12455", module_state.handle(), loc,
                             "shader %s defines a push constant statically (\"%s\") that block size %" PRIu32
                             " is larger than maxPushDataSize (%" PRIu64 ").\nEven if only bytes [0:%" PRIu64
                             "] are accessed, compilers may need to allocate the amount of memory declared statically.",
                             stage_state.entrypoint->Describe().c_str(), push_constant_variable->debug_name.c_str(),
                             push_constant_variable->size, max_size, max_size - 1);
        } else if ((push_constant_variable->offset + push_constant_variable->size) > max_size) {
            skip |= LogError("VUID-RuntimeSpirv-maxPushDataSize-12455", module_state.handle(), loc,
                             "shader %s defines a push constant statically (\"%s\") that block offset (%" PRIu32
                             ") + size (%" PRIu32 ") is larger than maxPushDataSize (%" PRIu64 ").\nEven if only bytes [%" PRIu32
                             ":%" PRIu64 "] are accessed, compilers may need to allocate the amount of memory declared statically.",
                             stage_state.entrypoint->Describe().c_str(), push_constant_variable->debug_name.c_str(),
                             push_constant_variable->offset, push_constant_variable->size, max_size, push_constant_variable->offset,
                             max_size - 1);
        }
        return skip;  // rest is testing against the pipeline layout
    } else if (module_state.static_data_.has_specialization_constants) {
        // TODO - Workaround for https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5911
        return skip;
    }

    const VkShaderStageFlagBits stage = entrypoint.stage;

    PushConstantRangesId shader_object_push_constant_ranges_id;
    std::vector<VkPushConstantRange> const* push_constant_ranges;
    if (pipeline) {
        push_constant_ranges = pipeline->PipelineLayoutState()->push_constant_ranges_layout.get();
    } else {
        shader_object_push_constant_ranges_id = GetCanonicalId(stage_state.shader_object_create_info->pushConstantRangeCount,
                                                               stage_state.shader_object_create_info->pPushConstantRanges);
        push_constant_ranges = shader_object_push_constant_ranges_id.get();
    }

    if (!push_constant_ranges || push_constant_ranges->empty()) {
        LogObjectList objlist(module_state.handle());
        std::string msg = "";
        if (pipeline) {
            objlist.add(pipeline->PipelineLayoutState()->Handle());
            msg = FormatHandle(pipeline->PipelineLayoutState()->Handle());
        } else {
            msg = "VkShaderCreateInfoEXT::pPushConstantRanges";
        }
        skip |= LogError(GetSpirvInterfaceVariableVUID(loc, vvl::SpirvInterfaceVariableError::PushConstantStage_07987), objlist,
                         loc, "shader %s is using push constants, but no VkPushConstantRange were found in %s.",
                         entrypoint.Describe().c_str(), msg.c_str());
        return skip;
    }

    bool found_stage = false;
    for (auto const& range : *push_constant_ranges) {
        if (range.stageFlags & stage) {
            found_stage = true;
            const uint32_t range_end = range.offset + range.size;
            const uint32_t push_constant_end = push_constant_variable->offset + push_constant_variable->size;
            // spec: "If a push constant block is declared in a shader"
            // Is checked regardless if element in Block is not statically used
            if ((push_constant_variable->offset < range.offset) | (push_constant_end > range_end)) {
                LogObjectList objlist(module_state.handle());
                if (pipeline) {
                    objlist.add(pipeline->PipelineLayoutState()->Handle());
                }
                skip |= LogError(
                    GetSpirvInterfaceVariableVUID(loc, vvl::SpirvInterfaceVariableError::PushConstantRange_10069), objlist, loc,
                    "shader %s has a push constant buffer Block with range [%" PRIu32 ", %" PRIu32
                    "] which is outside the VkPushConstantRange of [%" PRIu32 ", %" PRIu32 "].",
                    entrypoint.Describe().c_str(), push_constant_variable->offset, push_constant_end, range.offset, range_end);
                break;
            }
        }
    }

    if (!found_stage) {
        LogObjectList objlist(module_state.handle());
        std::stringstream ss;
        ss << "shader " << entrypoint.Describe() << " is using push constants, but ";
        if (pipeline) {
            objlist.add(pipeline->PipelineLayoutState()->Handle());
            ss << FormatHandle(pipeline->PipelineLayoutState()->Handle());
        } else {
            ss << "VkShaderCreateInfoEXT::pPushConstantRanges";
        }
        ss << " doesn't set any with " << string_VkShaderStageFlags(stage) << "\nCurrent VkPushConstantRange:";
        for (auto const& range : *push_constant_ranges) {
            ss << "\n - " << string_VkPushConstantRange(range);
        }
        skip |= LogError(GetSpirvInterfaceVariableVUID(loc, vvl::SpirvInterfaceVariableError::PushConstantStage_07987), objlist,
                         loc, "%s", ss.str().c_str());
    }
    return skip;
}

bool CoreChecks::ValidateShader64BitIndexing(const spirv::Module& module_state, const spirv::EntryPoint& entrypoint,
                                             const ShaderStageState& stage_state, const vvl::Pipeline* pipeline,
                                             const Location& loc) const {
    bool skip = false;

    if (pipeline && (pipeline->create_flags & VK_PIPELINE_CREATE_2_64_BIT_INDEXING_BIT_EXT)) {
        return skip;
    }
    if (stage_state.shader_object_create_info &&
        (stage_state.shader_object_create_info->flags & VK_SHADER_CREATE_64_BIT_INDEXING_BIT_EXT)) {
        return skip;
    }
    if (entrypoint.execution_mode.Has(spirv::ExecutionModeSet::shader_64bit_indexing)) {
        return skip;
    }

    auto const& check = [&](uint32_t value_id) -> bool {
        auto value_insn = module_state.FindDef(value_id);
        auto type_insn = module_state.FindDef(value_insn->Word(1));
        return type_insn->Word(2) != 32;
    };

    for (const spirv::Instruction* cooperative_vector_inst : module_state.static_data_.cooperative_vector_inst) {
        const spirv::Instruction& insn = *cooperative_vector_inst;
        switch (insn.Opcode()) {
            case spv::OpCooperativeVectorMatrixMulNV:
            case spv::OpCooperativeVectorMatrixMulAddNV: {
                uint32_t matrix_offset_id = insn.Word(6);
                if (check(matrix_offset_id)) {
                    skip |= LogError("VUID-RuntimeSpirv-OpCooperativeVectorMatrixMulAddNV-11808", module_state.handle(), loc,
                                     "shader %s contains 64-bit matrix offset\n%s\n", entrypoint.Describe().c_str(),
                                     module_state.DescribeInstruction(insn).c_str());
                }

                if (insn.Opcode() == spv::OpCooperativeVectorMatrixMulAddNV) {
                    uint32_t bias_offset_id = insn.Word(9);
                    if (check(bias_offset_id)) {
                        skip |= LogError("VUID-RuntimeSpirv-OpCooperativeVectorMatrixMulAddNV-11808", module_state.handle(), loc,
                                         "shader %s contains 64-bit bias offset\n%s\n", entrypoint.Describe().c_str(),
                                         module_state.DescribeInstruction(insn).c_str());
                    }
                }
                break;
            }
            case spv::OpCooperativeVectorLoadNV: {
                if (check(insn.Word(4))) {
                    skip |= LogError("VUID-RuntimeSpirv-OpCooperativeVectorLoadNV-11809", module_state.handle(), loc,
                                     "shader %s contains 64-bit load offset\n%s\n", entrypoint.Describe().c_str(),
                                     module_state.DescribeInstruction(insn).c_str());
                }
                break;
            }
            case spv::OpCooperativeVectorStoreNV: {
                if (check(insn.Word(2))) {
                    skip |= LogError("VUID-RuntimeSpirv-OpCooperativeVectorLoadNV-11809", module_state.handle(), loc,
                                     "shader %s contains 64-bit store offset\n%s\n", entrypoint.Describe().c_str(),
                                     module_state.DescribeInstruction(insn).c_str());
                }
                break;
            }
            case spv::OpCooperativeVectorReduceSumAccumulateNV: {
                if (check(insn.Word(2))) {
                    skip |= LogError("VUID-RuntimeSpirv-OpCooperativeVectorLoadNV-11809", module_state.handle(), loc,
                                     "shader %s contains 64-bit reducesum offset\n%s\n", entrypoint.Describe().c_str(),
                                     module_state.DescribeInstruction(insn).c_str());
                }
                break;
            }
            case spv::OpCooperativeVectorOuterProductAccumulateNV: {
                if (check(insn.Word(2))) {
                    skip |= LogError("VUID-RuntimeSpirv-OpCooperativeVectorLoadNV-11809", module_state.handle(), loc,
                                     "shader %s contains 64-bit outerproduct offset\n%s\n", entrypoint.Describe().c_str(),
                                     module_state.DescribeInstruction(insn).c_str());
                }
                break;
            }
            default:
                break;
        }
    }
    for (const spirv::Instruction* array_length_inst : module_state.static_data_.array_length_inst) {
        const spirv::Instruction& insn = *array_length_inst;
        if (check(insn.TypeId())) {
            skip |= LogError("VUID-RuntimeSpirv-OpArrayLength-11807", module_state.handle(), loc,
                             "shader %s contains 64-bit array length return type\n%s\n", entrypoint.Describe().c_str(),
                             module_state.DescribeInstruction(insn).c_str());
        }
    }
    for (const spirv::Instruction* constant_size_of_inst : module_state.static_data_.constant_size_of_inst) {
        const spirv::Instruction& insn = *constant_size_of_inst;
        if (check(insn.TypeId())) {
            skip |= LogError("VUID-RuntimeSpirv-OpConstantSizeOfEXT-11475", module_state.handle(), loc,
                             "shader %s contains 64-bit OpConstantSizeOfEXT return type\n%s\n", entrypoint.Describe().c_str(),
                             module_state.DescribeInstruction(insn).c_str());
        }
    }
    return skip;
}

// Done here instead of stateless because we need deal with spec constants
bool CoreChecks::ValidateVectorTypes(const spirv::Module& module_state, const spirv::EntryPoint& entrypoint,
                                     const Location& loc) const {
    bool skip = false;

    if (!enabled_features.longVector) {
        return skip;
    }

    const uint32_t max_vector_components = phys_dev_ext_props.shader_long_vector_props.maxVectorComponents;
    for (const spirv::Instruction* insn : module_state.static_data_.vector_type_inst) {
        const uint32_t components = module_state.GetNumComponentsInBaseType(insn);
        if (components > max_vector_components) {
            skip |= LogError("VUID-RuntimeSpirv-longVector-12296", module_state.handle(), loc,
                             "shader %s has a %s with a Component Count (%" PRIu32 ") which exceeds maxVectorComponents (%" PRIu32
                             ").\n%s\n",
                             entrypoint.Describe().c_str(), string_SpvOpcode(insn->Opcode()), components, max_vector_components,
                             module_state.DescribeInstruction(*insn).c_str());
        }
    }
    return skip;
}

bool CoreChecks::ValidateSubpassCustomeResolve(const spirv::Module& module_state, const spirv::EntryPoint& entrypoint,
                                               VkShaderStageFlagBits stage, const vvl::Pipeline& pipeline,
                                               const Location& loc) const {
    bool skip = false;

    // If the pipeline's subpass description contains flag VK_SUBPASS_DESCRIPTION_FRAGMENT_REGION_BIT_EXT,
    // then the fragment shader must not enable the SPIRV SampleRateShading capability.
    if (stage == VK_SHADER_STAGE_FRAGMENT_BIT && module_state.HasCapability(spv::CapabilitySampleRateShading)) {
        const auto& rp_state = pipeline.RenderPassState();
        if (!rp_state || rp_state->UsesDynamicRendering()) {
            return skip;
        }
        const VkSubpassDescriptionFlags subpass_flags = rp_state->create_info.pSubpasses[pipeline.Subpass()].flags;
        if ((subpass_flags & VK_SUBPASS_DESCRIPTION_FRAGMENT_REGION_BIT_EXT) != 0) {
            const LogObjectList objlist(module_state.handle(), rp_state->Handle());
            skip |= LogError("VUID-RuntimeSpirv-SampleRateShading-06378", objlist, loc,
                             "shader %s enables SampleRateShading capability "
                             "and the subpass flags includes VK_SUBPASS_DESCRIPTION_FRAGMENT_REGION_BIT_EXT.",
                             entrypoint.Describe().c_str());
        }
    }

    return skip;
}

bool CoreChecks::ValidateShaderExecutionModes(const spirv::Module& module_state, const spirv::EntryPoint& entrypoint,
                                              VkShaderStageFlagBits stage, const vvl::Pipeline* pipeline,
                                              const Location& loc) const {
    bool skip = false;

    if (entrypoint.stage == VK_SHADER_STAGE_GEOMETRY_BIT) {
        const uint32_t vertices_out = entrypoint.execution_mode.output_vertices;
        const uint32_t invocations = entrypoint.execution_mode.invocations;
        if (vertices_out != spirv::kInvalidValue &&
            (vertices_out == 0 || vertices_out > phys_dev_props.limits.maxGeometryOutputVertices)) {
            const char* vuid =
                pipeline ? "VUID-VkPipelineShaderStageCreateInfo-stage-00714" : "VUID-VkShaderCreateInfoEXT-pCode-08454";
            skip |= LogError(vuid, module_state.handle(), loc,
                             "shader %s entry point must have an OpExecutionMode instruction that "
                             "specifies a maximum output vertex count that is greater than 0 and less "
                             "than or equal to maxGeometryOutputVertices.\n"
                             "OutputVertices = %" PRIu32 "\nmaxGeometryOutputVertices = %" PRIu32 "\n",
                             entrypoint.Describe().c_str(), vertices_out, phys_dev_props.limits.maxGeometryOutputVertices);
        }

        if (invocations == 0 || invocations > phys_dev_props.limits.maxGeometryShaderInvocations) {
            const char* vuid =
                pipeline ? "VUID-VkPipelineShaderStageCreateInfo-stage-00715" : "VUID-VkShaderCreateInfoEXT-pCode-08455";
            skip |= LogError(vuid, module_state.handle(), loc,
                             "shader %s entry point must have an OpExecutionMode instruction that "
                             "specifies an invocation count that is greater than 0 and less "
                             "than or equal to maxGeometryShaderInvocations.\n"
                             "Invocations = %" PRIu32 "\nmaxGeometryShaderInvocations = %" PRIu32 "\n",
                             entrypoint.Describe().c_str(), invocations, phys_dev_props.limits.maxGeometryShaderInvocations);
        }
    } else if (entrypoint.stage == VK_SHADER_STAGE_FRAGMENT_BIT &&
               entrypoint.execution_mode.Has(spirv::ExecutionModeSet::early_fragment_test_bit)) {
        if (pipeline) {
            const auto* ds_state = pipeline->DepthStencilState();
            if ((ds_state &&
                 (ds_state->flags &
                  (VK_PIPELINE_DEPTH_STENCIL_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_DEPTH_ACCESS_BIT_EXT |
                   VK_PIPELINE_DEPTH_STENCIL_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_STENCIL_ACCESS_BIT_EXT)) != 0)) {
                skip |=
                    LogError("VUID-VkGraphicsPipelineCreateInfo-flags-06591", module_state.handle(), loc,
                             "shader %s enables early fragment tests, but VkPipelineDepthStencilStateCreateInfo::flags == "
                             "%s.",
                             entrypoint.Describe().c_str(), string_VkPipelineDepthStencilStateCreateFlags(ds_state->flags).c_str());
            }
        }
    }

    return skip;
}

bool CoreChecks::ValidatePointSizeShaderState(const spirv::Module& module_state, const spirv::EntryPoint& entrypoint,
                                              const vvl::Pipeline& pipeline, VkShaderStageFlagBits stage,
                                              const Location& loc) const {
    bool skip = false;
    // vkspec.html#primsrast-points describes which is the final stage that needs to check for points
    //
    // Vertex - Need to read input topology in pipeline
    // Geo/Tess - Need to know the feature bit is on
    // Mesh - are checked in spirv-val as they don't require any runtime information
    if (!IsValueIn(stage,
                   {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, VK_SHADER_STAGE_GEOMETRY_BIT})) {
        return skip;
    }

    const bool output_points = entrypoint.execution_mode.Has(spirv::ExecutionModeSet::output_points_bit);
    const bool point_mode = entrypoint.execution_mode.Has(spirv::ExecutionModeSet::point_mode_bit);
    const bool maintenance5 = enabled_features.maintenance5;

    if (stage == VK_SHADER_STAGE_GEOMETRY_BIT && output_points) {
        if (enabled_features.shaderTessellationAndGeometryPointSize && !entrypoint.written_built_in_point_size &&
            entrypoint.emit_vertex_geometry && !maintenance5) {
            skip |=
                LogError("VUID-VkGraphicsPipelineCreateInfo-shaderTessellationAndGeometryPointSize-08776", module_state.handle(),
                         loc, "shader %s PointSize is not written, but shaderTessellationAndGeometryPointSize was enabled.",
                         entrypoint.Describe().c_str());
        } else if (!enabled_features.shaderTessellationAndGeometryPointSize && entrypoint.written_built_in_point_size) {
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-Geometry-07726", module_state.handle(), loc,
                             "shader %s PointSize is written to, but shaderTessellationAndGeometryPointSize was not "
                             "enabled (gl_PointSize must NOT be written and a default of 1.0 is assumed).",
                             entrypoint.Describe().c_str());
        }
    } else if (stage == VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT &&
               ((pipeline.create_info_shaders & VK_SHADER_STAGE_GEOMETRY_BIT) == 0) && point_mode) {
        if (enabled_features.shaderTessellationAndGeometryPointSize && !entrypoint.written_built_in_point_size && !maintenance5) {
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-TessellationEvaluation-07723", module_state.handle(), loc,
                             "shader %s PointSize is not written, but "
                             "shaderTessellationAndGeometryPointSize was enabled.",
                             entrypoint.Describe().c_str());
        } else if (!enabled_features.shaderTessellationAndGeometryPointSize && entrypoint.written_built_in_point_size) {
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-TessellationEvaluation-07724", module_state.handle(), loc,
                             "shader %s PointSize is written to, shaderTessellationAndGeometryPointSize "
                             "was not enabled (gl_PointSize must NOT be written and a default of 1.0 is assumed).",
                             entrypoint.Describe().c_str());
        }
    } else if (stage == VK_SHADER_STAGE_VERTEX_BIT &&
               ((pipeline.create_info_shaders & (VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT | VK_SHADER_STAGE_GEOMETRY_BIT)) ==
                0)) {
        if (!entrypoint.written_built_in_point_size && IsPointTopology(pipeline.topology_at_rasterizer) && !maintenance5) {
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-topology-08773", module_state.handle(), loc,
                             "shader %s PointSize is not written to, but Pipeline topology is set to "
                             "VK_PRIMITIVE_TOPOLOGY_POINT_LIST.",
                             entrypoint.Describe().c_str());
        }
    }

    return skip;
}

bool CoreChecks::ValidatePrimitiveRateShaderState(const spirv::Module& module_state, const spirv::EntryPoint& entrypoint,
                                                  const vvl::Pipeline& pipeline, VkShaderStageFlagBits stage,
                                                  const Location& loc) const {
    bool skip = false;

    const auto viewport_state = pipeline.ViewportState();
    if (!phys_dev_ext_props.fragment_shading_rate_props.primitiveFragmentShadingRateWithMultipleViewports &&
        (pipeline.pipeline_type == VK_PIPELINE_BIND_POINT_GRAPHICS) && viewport_state) {
        if (!pipeline.IsDynamic(CB_DYNAMIC_STATE_VIEWPORT_WITH_COUNT) && viewport_state->viewportCount > 1 &&
            entrypoint.written_built_in_primitive_shading_rate_khr) {
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-primitiveFragmentShadingRateWithMultipleViewports-04503",
                             module_state.handle(), loc,
                             "shader %s statically writes to PrimitiveShadingRateKHR built-in, but "
                             "multiple viewports "
                             "are used and the primitiveFragmentShadingRateWithMultipleViewports limit is not supported.",
                             entrypoint.Describe().c_str());
        }

        if (entrypoint.written_built_in_primitive_shading_rate_khr && entrypoint.written_built_in_viewport_index) {
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-primitiveFragmentShadingRateWithMultipleViewports-04504",
                             module_state.handle(), loc,
                             "shader %s statically writes to both PrimitiveShadingRateKHR and "
                             "ViewportIndex built-ins, "
                             "but the primitiveFragmentShadingRateWithMultipleViewports limit is not supported.",
                             entrypoint.Describe().c_str());
        }

        if (entrypoint.written_built_in_primitive_shading_rate_khr && entrypoint.written_built_in_viewport_mask_nv) {
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-primitiveFragmentShadingRateWithMultipleViewports-04505",
                             module_state.handle(), loc,
                             "shader %s statically writes to both PrimitiveShadingRateKHR and "
                             "ViewportMaskNV built-ins, "
                             "but the primitiveFragmentShadingRateWithMultipleViewports limit is not supported.",
                             entrypoint.Describe().c_str());
        }
    }
    return skip;
}

bool CoreChecks::ValidateWorkgroupSharedMemory(const spirv::Module& module_state, const spirv::EntryPoint& entrypoint,
                                               VkShaderStageFlagBits stage, const Location& loc) const {
    bool skip = false;

    const uint32_t total_workgroup_shared_memory = module_state.CalculateWorkgroupSharedMemory();

    if (stage == VK_SHADER_STAGE_COMPUTE_BIT) {
        if (total_workgroup_shared_memory > phys_dev_props.limits.maxComputeSharedMemorySize) {
            skip |= LogError(
                "VUID-RuntimeSpirv-Workgroup-06530", module_state.handle(), loc,
                "shader %s uses %" PRIu32 " bytes of shared memory, which is more than maxComputeSharedMemorySize (%" PRIu32 ").",
                entrypoint.Describe().c_str(), total_workgroup_shared_memory, phys_dev_props.limits.maxComputeSharedMemorySize);
        }

        if (enabled_features.cooperativeMatrixWorkgroupScopeNV) {
            for (auto& cooperative_matrix_inst : module_state.static_data_.cooperative_matrix_inst) {
                if (cooperative_matrix_inst->Opcode() != spv::OpTypeCooperativeMatrixKHR) {
                    continue;
                }
                auto scope = module_state.GetAnyConstantDef(cooperative_matrix_inst->Word(3));
                if (!scope || scope->GetConstantValue() != VK_SCOPE_WORKGROUP_KHR) {
                    continue;
                }
                if (total_workgroup_shared_memory >
                    phys_dev_props.limits.maxComputeSharedMemorySize -
                        phys_dev_ext_props.cooperative_matrix_props2_nv.cooperativeMatrixWorkgroupScopeReservedSharedMemory) {
                    skip |= LogError(
                        "VUID-RuntimeSpirv-maxComputeSharedMemorySize-10168", module_state.handle(), loc,
                        "shader %s uses %" PRIu32 " bytes of shared memory, which is more than maxComputeSharedMemorySize (%" PRIu32
                        ") minus "
                        "cooperativeMatrixWorkgroupScopeReservedSharedMemory (%" PRIu32 ").",
                        entrypoint.Describe().c_str(), total_workgroup_shared_memory,
                        phys_dev_props.limits.maxComputeSharedMemorySize,
                        phys_dev_ext_props.cooperative_matrix_props2_nv.cooperativeMatrixWorkgroupScopeReservedSharedMemory);
                    break;
                }
            }
        }
    } else if (stage == VK_SHADER_STAGE_TASK_BIT_EXT) {
        skip |= ValidateTaskShaderLimits(module_state, entrypoint, total_workgroup_shared_memory, loc);
    } else if (stage == VK_SHADER_STAGE_MESH_BIT_EXT) {
        skip |= ValidateMeshShaderLimits(module_state, entrypoint, total_workgroup_shared_memory, loc);
    } else {
        assert(false);  // other stages should not have called this function
    }

    return skip;
}

bool CoreChecks::ValidateShaderInterfaceVariable(const spirv::Module& module_state, const spirv::EntryPoint& entrypoint,
                                                 const ShaderStageState& stage_state,
                                                 const spirv::ResourceInterfaceVariable& variable, const Location& loc) const {
    bool skip = false;

    if (stage_state.descriptor_set_layouts) {
        skip |= ValidateShaderInterfaceVariableDSL(module_state, entrypoint, stage_state, variable, loc);
    }

    // We just check the currently known writable descriptor types, spec doesn't provide a list for feature bit
    if (((variable.stage & VK_SHADER_STAGE_ALL_GRAPHICS) != 0) &&
        (variable.is_storage_image || variable.is_storage_texel_buffer || variable.is_storage_buffer ||
         variable.is_storage_tensor) &&
        !variable.decorations.Has(spirv::DecorationSet::nonwritable_bit)) {
        // If the variable is a struct, all members must contain NonWritable
        if (!variable.type_struct_info ||
            !variable.type_struct_info->decorations.AllMemberHave(spirv::DecorationSet::nonwritable_bit)) {
            switch (variable.stage) {
                case VK_SHADER_STAGE_FRAGMENT_BIT:
                    if (!enabled_features.fragmentStoresAndAtomics) {
                        skip |= LogError("VUID-RuntimeSpirv-NonWritable-06340", module_state.handle(), loc,
                                         "shader %s uses descriptor %s (type %s) which is not "
                                         "marked with NonWritable, but fragmentStoresAndAtomics was not enabled.",
                                         entrypoint.Describe().c_str(), variable.DescribeDescriptor().c_str(),
                                         string_VkDescriptorType(variable.GetPotentialDescriptorType()));
                    }
                    break;
                case VK_SHADER_STAGE_VERTEX_BIT:
                case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
                case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
                case VK_SHADER_STAGE_GEOMETRY_BIT:
                    if (!enabled_features.vertexPipelineStoresAndAtomics) {
                        skip |= LogError("VUID-RuntimeSpirv-NonWritable-06341", module_state.handle(), loc,
                                         "shader %s uses descriptor %s (type %s) which is not marked with NonWritable, but "
                                         "vertexPipelineStoresAndAtomics was not enabled.",
                                         entrypoint.Describe().c_str(), variable.DescribeDescriptor().c_str(),
                                         string_VkDescriptorType(variable.GetPotentialDescriptorType()));
                    }
                    break;
                default:
                    // No feature requirements for writes and atomics for other stages
                    break;
            }
        }
    }

    if (!variable.decorations.Has(spirv::DecorationSet::input_attachment_bit) && variable.info.image_dim == spv::DimSubpassData) {
        if (variable.IsArray()) {
            skip |=
                LogError("VUID-RuntimeSpirv-OpTypeImage-09644", module_state.handle(), loc,
                         "shader %s has a variable that is an array of OpTypeImage with Dim::SubpassData, but it is missing the "
                         "InputAttachmentIndex decoration.\n%s\n",
                         entrypoint.Describe().c_str(), variable.base_type.Describe().c_str());
        } else if (!enabled_features.dynamicRenderingLocalRead) {
            skip |= LogError("VUID-RuntimeSpirv-None-09558", module_state.handle(), loc,
                             "shader %s has a variable that is a OpTypeImage with Dim::SubpassData, but it is missing the "
                             "InputAttachmentIndex decoration (dynamicRenderingLocalRead was not enabled).\n%s\n",
                             entrypoint.Describe().c_str(), variable.base_type.Describe().c_str());
        }
    }

    if (variable.is_uniform_buffer && variable.type_struct_info && variable.type_struct_info->has_runtime_array &&
        !enabled_features.shaderUniformBufferUnsizedArray) {
        skip |= LogError("VUID-RuntimeSpirv-shaderUniformBufferUnsizedArray-11806", module_state.handle(), loc,
                         "shader %s uses descriptor %s which is an uniform buffer with a runtime array, but "
                         "shaderUniformBufferUnsizedArray was not enabled.",
                         entrypoint.Describe().c_str(), variable.DescribeDescriptor().c_str());
    }

    return skip;
}

struct ShaderResourceType {
    const spirv::ResourceInterfaceVariable& resource_variable;
    const vvl::unordered_set<VkDescriptorType> descriptor_type_set;

    explicit ShaderResourceType(const spirv::ResourceInterfaceVariable& variable)
        : resource_variable(variable), descriptor_type_set(variable.GetAllDescriptorTypes()) {}

    bool HasType(VkDescriptorType type) { return descriptor_type_set.find(type) != descriptor_type_set.end(); }

    std::string Describe(bool hints) {
        std::ostringstream ss;
        for (auto it = descriptor_type_set.begin(); it != descriptor_type_set.end(); ++it) {
            if (ss.tellp()) ss << " or ";
            ss << string_VkDescriptorType(VkDescriptorType(*it));
        }

        // Currently this is used for 2 checks
        // - When there is no binding found at all
        // - When it is found, but the mismatch, here we want to help give hints
        if (hints) {
            ss << "\nInfo on SPIR-V mapping for each type:";
            if (descriptor_type_set.count(VK_DESCRIPTOR_TYPE_SAMPLER)) {
                ss << "\n - VK_DESCRIPTOR_TYPE_SAMPLER is an OpTypeSampler with UniformConstant";
            }
            if (descriptor_type_set.count(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)) {
                ss << "\n - VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER is an OpTypeSampledImage that consumes both a OpTypeSampler "
                      "and OpTypeImage in UniformConstant";
            }
            if (descriptor_type_set.count(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)) {
                ss << "\n - VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE is an OpTypeImage, with Sampled = 1, in UniformConstant";
            }
            if (descriptor_type_set.count(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)) {
                ss << "\n - VK_DESCRIPTOR_TYPE_STORAGE_IMAGE is an OpTypeImage, with Sampled = 2, in UniformConstant";
            }
            if (descriptor_type_set.count(VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER)) {
                ss << "\n - VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER is an OpTypeImage, with Sampled = 1 and Dim = Buffer, in "
                      "UniformConstant";
            }
            if (descriptor_type_set.count(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER)) {
                ss << "\n - VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER is an OpTypeImage, with Sampled = 2 and Dim = Buffer, in "
                      "UniformConstant";
            }
            if (descriptor_type_set.count(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)) {
                ss << "\n - VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER/VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK is an OpTypeStruct as "
                      "Uniform";
            }
            if (descriptor_type_set.count(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)) {
                ss << "\n - VK_DESCRIPTOR_TYPE_STORAGE_BUFFER is an OpTypeStruct as ";
                if (resource_variable.is_buffer_block) {
                    ss << "Uniform, with BufferBlock (Vulkan 1.0 didn't have a dedicated StorageBuffer storage class, more info at "
                          "https://docs.vulkan.org/guide/latest/extensions/"
                          "shader_features.html#VK_KHR_storage_buffer_storage_class)";
                } else {
                    ss << "StorageBuffer";
                }
            }
            if (descriptor_type_set.count(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT)) {
                ss << "\n - VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT is an OpTypeImage, with Sampled = 2 and Dim = SubpassData, in "
                      "UniformConstant";
            }
            if (descriptor_type_set.count(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR)) {
                ss << "\n - VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR is an OpTypeAccelerationStructureKHR in UniformConstant";
            }
            if (descriptor_type_set.count(VK_DESCRIPTOR_TYPE_PARTITIONED_ACCELERATION_STRUCTURE_NV)) {
                ss << "\n - VK_DESCRIPTOR_TYPE_PARTITIONED_ACCELERATION_STRUCTURE_NV is an OpTypeAccelerationStructureKHR in "
                      "UniformConstant";
            }
            if (descriptor_type_set.count(VK_DESCRIPTOR_TYPE_TENSOR_ARM)) {
                ss << "\n - VK_DESCRIPTOR_TYPE_TENSOR_ARM is an OpTypeTensorARM in UniformConstant";
            }
            if (descriptor_type_set.count(VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM)) {
                ss << "\n - VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM is an OpTypeImage, with Sampled = 1, in UniformConstant";
            }
            if (descriptor_type_set.count(VK_DESCRIPTOR_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM)) {
                ss << "\n - VK_DESCRIPTOR_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM is an OpTypeImage, with Sampled = 1 and Arrayed = 1, in "
                      "UniformConstant";
            }
            ss << "\nFull list of mappings can be found at "
                  "https://docs.vulkan.org/spec/latest/chapters/interfaces.html#interfaces-resources-storage-class-correspondence";
        }
        return ss.str();
    }
};

bool CoreChecks::ValidateShaderInterfaceVariableDSL(const spirv::Module& module_state, const spirv::EntryPoint& entrypoint,
                                                    const ShaderStageState& stage_state,
                                                    const spirv::ResourceInterfaceVariable& variable, const Location& loc) const {
    bool skip = false;
    if (!stage_state.descriptor_set_layouts) {
        return skip;
    } else if (stage_state.heap.descriptor_heap_mode) {
        return skip;
    }

    LogObjectList objlist(module_state.handle());

    const VkDescriptorSetLayoutBinding* binding = nullptr;

    const vvl::DescriptorSetLayout* descriptor_set_layout = stage_state.descriptor_set_layouts->FindFromVariable(variable);
    if (descriptor_set_layout) {
        objlist.add(descriptor_set_layout->Handle());
        binding = descriptor_set_layout->GetDescriptorSetLayoutBindingPtrFromBinding(variable.decorations.binding);
    }

    auto print_dsl_info = [&stage_state, &variable]() {
        std::ostringstream ss;
        if (stage_state.HasPipeline()) {
            ss << "VkPipelineLayoutCreateInfo::pSetLayouts[" << variable.decorations.set << "]";
        } else {
            ss << "VkShaderCreateInfoEXT::pSetLayouts[" << variable.decorations.set << "]";
        }
        return ss.str();
    };

    ShaderResourceType resource_type(variable);

    // If no binding nothing left to validate
    if (!binding) {
        if (variable.IsHeap()) {
            skip |= LogError(GetSpirvInterfaceVariableVUID(loc, vvl::SpirvInterfaceVariableError::ShaderStage_07988), objlist, loc,
                             "SPIR-V (%s) is trying to use descriptor heaps (%s) but is also trying to use a %s, either set the "
                             "layout to NULL or remove the heaps from the shader.",
                             entrypoint.Describe().c_str(), variable.DescribeDescriptor().c_str(),
                             stage_state.HasPipeline() ? "VkPipelineLayout" : "VkDescriptorSetLayout");
        } else {
            skip |=
                LogError(GetSpirvInterfaceVariableVUID(loc, vvl::SpirvInterfaceVariableError::ShaderStage_07988), objlist, loc,
                         "SPIR-V (%s) uses descriptor %s but the binding was not declared in the %s.\nPossible VkDescriptorType "
                         "that could be used are: %s",
                         entrypoint.Describe().c_str(), variable.DescribeDescriptor().c_str(), print_dsl_info().c_str(),
                         resource_type.Describe(false).c_str());
        }
        return skip;
    }

    if (~binding->stageFlags & variable.stage) {
        skip |= LogError(GetSpirvInterfaceVariableVUID(loc, vvl::SpirvInterfaceVariableError::ShaderStage_07988), objlist, loc,
                         "shader %s uses descriptor %s (%s) but the VkDescriptorSetLayoutBinding::stageFlags was "
                         "%s.\n(VkDescriptorSetLayout from %s)",
                         entrypoint.Describe().c_str(), variable.DescribeDescriptor().c_str(),
                         string_VkDescriptorType(binding->descriptorType), string_VkShaderStageFlags(binding->stageFlags).c_str(),
                         print_dsl_info().c_str());
    } else if (binding->descriptorType != VK_DESCRIPTOR_TYPE_MUTABLE_EXT && !resource_type.HasType(binding->descriptorType)) {
        skip |= LogError(
            GetSpirvInterfaceVariableVUID(loc, vvl::SpirvInterfaceVariableError::Mutable_07990), objlist, loc,
            "shader %s uses descriptor %s which has a VkDescriptorType mismatch.\n  VkDescriptorSetLayoutBinding::descriptorType "
            "is %s\n  Possible VkDescriptorType for the SPIR-V variable are: %s\n(VkDescriptorSetLayout from %s)",
            entrypoint.Describe().c_str(), variable.DescribeDescriptor().c_str(), string_VkDescriptorType(binding->descriptorType),
            resource_type.Describe(true).c_str(), print_dsl_info().c_str());
    } else if (binding->descriptorType == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK && variable.IsArray()) {
        skip |=
            LogError(GetSpirvInterfaceVariableVUID(loc, vvl::SpirvInterfaceVariableError::Inline_10391), objlist, loc,
                     "shader %s uses descriptor %s as VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK, but it is an array of descriptor."
                     "\n(VkDescriptorSetLayout from %s)",
                     entrypoint.Describe().c_str(), variable.DescribeDescriptor().c_str(), print_dsl_info().c_str());

    } else if (variable.IsRuntimeArray() && binding->descriptorCount == 0) {
        skip |= LogError(GetSpirvInterfaceVariableVUID(loc, vvl::SpirvInterfaceVariableError::DescriptorCount_07991), objlist, loc,
                         "shader %s uses a runtime descriptor array %s with a VkDescriptorSetLayoutBinding::descriptorCount of 0 "
                         "but requires at least 1 descriptor.\n(VkDescriptorSetLayout from %s)",
                         entrypoint.Describe().c_str(), variable.DescribeDescriptor().c_str(), print_dsl_info().c_str());
    } else if (!variable.IsRuntimeArray() && binding->descriptorCount < variable.array_length) {
        skip |= LogError(GetSpirvInterfaceVariableVUID(loc, vvl::SpirvInterfaceVariableError::DescriptorCount_07991), objlist, loc,
                         "shader %s uses descriptor %s which has an array size of %" PRIu32
                         " in the SPIR-V but VkDescriptorSetLayoutBinding::descriptorCount of %" PRIu32
                         " which doesn't initialize all the elements (can be done with the runtimeDescriptorArray "
                         "feature).\n(VkDescriptorSetLayout from %s)",
                         entrypoint.Describe().c_str(), variable.DescribeDescriptor().c_str(), variable.array_length,
                         binding->descriptorCount, print_dsl_info().c_str());
    }

    skip |= ValidateShaderYcbcrSampler(module_state, entrypoint, *descriptor_set_layout, *binding, variable, objlist, loc);

    return skip;
}

// "friends don't let friends validate YCbCr in SPIR-V" ~Spencer
bool CoreChecks::ValidateShaderYcbcrSampler(const spirv::Module& module_state, const spirv::EntryPoint& entrypoint,
                                            const vvl::DescriptorSetLayout& descriptor_set_layout,
                                            const VkDescriptorSetLayoutBinding& binding,
                                            const spirv::ResourceInterfaceVariable& variable, const LogObjectList& objlist,
                                            const Location& loc) const {
    bool skip = false;

    // pImmutableSamplers can have non-YCbCr samplers (but can't mix between YCbCr/Non-YCbCr)
    //
    // IsAccessed() will prevent things like textureSize() from be marked as a false positive.
    // Note that for YCbCr, OpImageQueryLod will query the sampler, but OpImageQuerySize only queries
    // the image and therefor can still be used with YCbCr.
    const bool possible_ycbcr = binding.pImmutableSamplers && descriptor_set_layout.HasYcbcrSamplers() &&
                                (variable.IsImage() && variable.IsImageAccessed());
    if (!possible_ycbcr) {
        return skip;
    }

    // YCbCr is only allowed for Combined Image Samplers (error is caught before)
    if (!variable.is_combined_image_sampler) {
        return skip;
    }

    // The sampler state might have been destroyed, we need to get the safe struct we saved
    const uint32_t index = descriptor_set_layout.GetIndexFromBinding(binding.binding);
    const std::vector<vku::safe_VkSamplerCreateInfo>& sampler_create_infos =
        descriptor_set_layout.GetImmutableSamplerCreateInfosFromIndex(index);
    for (uint32_t i = 0; i < sampler_create_infos.size(); i++) {
        auto* conversion_info = vku::FindStructInPNextChain<VkSamplerYcbcrConversionInfo>(sampler_create_infos[i].pNext);
        if (!conversion_info || conversion_info->conversion == VK_NULL_HANDLE) {
            continue;
        }

        if (!variable.info.image_insn.is_sampler_sampled) {
            skip |= LogError("VUID-RuntimeSpirv-OpTypeSampledImage-12206", objlist, loc,
                             "shader %s has %s which points to pImmutableSamplers[%" PRIu32
                             "] (%s) that was created with a VkSamplerYcbcrConversion, but was accessed in the SPIR-V "
                             "with a non OpImage*Sample* instruction.\nNon-sampled operations (like texelFetch) can't be used "
                             "because it doesn't contain the sampler YCbCr conversion information for the driver.",
                             entrypoint.Describe().c_str(), variable.DescribeDescriptor().c_str(), i,
                             FormatHandle(conversion_info->conversion).c_str());
            break;  // only need to report a single descriptor
        } else if (variable.info.image_insn.is_sampler_offset) {
            skip |= LogError("VUID-RuntimeSpirv-ConstOffset-10718", objlist, loc,
                             "shader %s has %s which points to pImmutableSamplers[%" PRIu32
                             "] (%s) that was created with a VkSamplerYcbcrConversion, but was accessed in the SPIR-V "
                             "with ConstOffset/Offset image operands.",
                             entrypoint.Describe().c_str(), variable.DescribeDescriptor().c_str(), i,
                             FormatHandle(conversion_info->conversion).c_str());
            break;  // only need to report a single descriptor
        }

        if (!variable.all_constant_integral_expressions) {
            std::ostringstream ss;
            ss << "shader " << entrypoint.Describe() << " has " << variable.DescribeDescriptor()
               << " which is an COMBINED_SAMPLED_IMAGE tied to an array of YCbCr samplers and it is trying to be accessed with a "
                  "non-constant index value.\nRegardless if it is uniform or not, you can't dynamically index into an array of "
                  "YCbCr samplers "
                  "in your shader and you need a constant value.\nThis is because the driver's compiler needs to know the exact "
                  "YCbCr sampler/image being used in order to inject special instructions into the final shader.\nOne possible "
                  "workaround is to use Specialization Constant to decide the index at pipeline/shaderObject creation time.";
            skip |= LogError("VUID-RuntimeSpirv-None-12205", objlist, loc, "%s", ss.str().c_str());

            // only need to report a single descriptor
            // If we hook up to print the ShaderDebugInfo, might be worth it to print all of them
            break;
        }
    }

    return skip;
}

bool CoreChecks::ValidateTransformFeedbackPipeline(const spirv::Module& module_state, const spirv::EntryPoint& entrypoint,
                                                   const vvl::Pipeline& pipeline, const Location& loc) const {
    bool skip = false;

    const bool is_xfb_execution_mode = entrypoint.execution_mode.Has(spirv::ExecutionModeSet::xfb_bit);
    if (is_xfb_execution_mode) {
        if ((pipeline.create_info_shaders & (VK_SHADER_STAGE_MESH_BIT_EXT | VK_SHADER_STAGE_TASK_BIT_EXT)) != 0) {
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-None-02322", module_state.handle(), loc,
                             "shader %s has OpExecutionMode of Xfb and the pipeline contains mesh shaders (%s).",
                             entrypoint.Describe().c_str(), string_VkShaderStageFlags(pipeline.create_info_shaders).c_str());
        }

        if (pipeline.pre_raster_state) {
            if (entrypoint.stage != pipeline.pre_raster_state->last_stage) {
                skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-pStages-02318", module_state.handle(), loc,
                                 "shader %s has OpExecutionMode of Xfb , but %s is the last pre-rasterization shader stage "
                                 "(and must be %s).",
                                 entrypoint.Describe().c_str(), string_VkShaderStageFlagBits(pipeline.pre_raster_state->last_stage),
                                 string_VkShaderStageFlagBits(entrypoint.stage));
            }
            if ((pipeline.create_flags & VK_PIPELINE_CREATE_2_INDIRECT_BINDABLE_BIT_EXT) != 0) {
                skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-flags-11001", module_state.handle(), loc,
                                 "shader %s has OpExecutionMode of Xfb but this pipeline is being created with "
                                 "VK_PIPELINE_CREATE_2_INDIRECT_BINDABLE_BIT_EXT.",
                                 entrypoint.Describe().c_str());
            }
        }
    }

    if (pipeline.pre_raster_state && (pipeline.create_info_shaders & VK_SHADER_STAGE_GEOMETRY_BIT) != 0 &&
        module_state.HasCapability(spv::CapabilityGeometryStreams) && !enabled_features.geometryStreams) {
        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-geometryStreams-02321", module_state.handle(), loc,
                         "shader %s uses GeometryStreams capability, but "
                         "VkPhysicalDeviceTransformFeedbackFeaturesEXT::geometryStreams is not enabled.",
                         entrypoint.Describe().c_str());
    }
    return skip;
}

bool CoreChecks::ValidateTransformFeedbackShaderObject(const spirv::Module& module_state, const spirv::EntryPoint& entrypoint,
                                                       const vku::safe_VkShaderCreateInfoEXT& create_info,
                                                       const Location& loc) const {
    bool skip = false;

    if ((create_info.flags & VK_SHADER_CREATE_INDIRECT_BINDABLE_BIT_EXT) != 0 &&
        entrypoint.execution_mode.Has(spirv::ExecutionModeSet::xfb_bit)) {
        skip |= LogError("VUID-VkShaderCreateInfoEXT-flags-11006", module_state.handle(), loc.dot(Field::flags),
                         "contains VK_SHADER_CREATE_INDIRECT_BINDABLE_BIT_EXT, but shader %s specifies xfb execution mode.",
                         entrypoint.Describe().c_str());
    }

    return skip;
}

bool CoreChecks::ValidateImageWrite(const spirv::Module& module_state, const spirv::EntryPoint& entrypoint,
                                    const Location& loc) const {
    bool skip = false;
    for (const auto& [insn, load_id] : module_state.static_data_.image_write_load_id_map) {
        // guaranteed by spirv-val to be an OpTypeImage
        const uint32_t image = module_state.GetTypeId(load_id);
        const spirv::Instruction* image_def = module_state.FindDef(image);
        const uint32_t image_format = image_def->Word(8);
        // If format is 'Unknown' then need to wait until a descriptor is bound to it
        if (image_format != spv::ImageFormatUnknown) {
            const VkFormat compatible_format = CompatibleSpirvImageFormat(image_format);
            if (compatible_format != VK_FORMAT_UNDEFINED) {
                const uint32_t format_component_count = vkuFormatComponentCount(compatible_format);
                const uint32_t texel_component_count = module_state.GetTexelComponentCount(*insn);
                if (texel_component_count < format_component_count) {
                    skip |= LogError("VUID-RuntimeSpirv-OpImageWrite-07112", module_state.handle(), loc,
                                     "shader %s has an OpImageWrite where the Texel operand only contains %" PRIu32
                                     " components, but the OpImage format mapping to %s has %" PRIu32 " components.\n%s\n%s\n",
                                     entrypoint.Describe().c_str(), texel_component_count, string_VkFormat(compatible_format),
                                     format_component_count, module_state.DescribeInstruction(*insn).c_str(),
                                     module_state.DescribeInstruction(*image_def).c_str());
                }
            }
        }
    }
    return skip;
}

static const std::string GetShaderTileImageCapabilitiesString(const spirv::Module& module_state) {
    struct SpvCapabilityWithString {
        const spv::Capability cap;
        const std::string cap_string;
    };

    // Shader tile image capabilities
    static const std::array<SpvCapabilityWithString, 3> shader_tile_image_capabilities = {
        {{spv::CapabilityTileImageColorReadAccessEXT, "TileImageColorReadAccessEXT"},
         {spv::CapabilityTileImageDepthReadAccessEXT, "TileImageDepthReadAccessEXT"},
         {spv::CapabilityTileImageStencilReadAccessEXT, "TileImageStencilReadAccessEXT"}}};

    std::ostringstream ss_capabilities;
    for (auto spv_capability : shader_tile_image_capabilities) {
        if (module_state.HasCapability(spv_capability.cap)) {
            if (ss_capabilities.tellp()) ss_capabilities << ", ";
            ss_capabilities << spv_capability.cap_string;
        }
    }

    return ss_capabilities.str();
}

bool CoreChecks::ValidateShaderTileImage(const spirv::Module& module_state, const spirv::EntryPoint& entrypoint,
                                         const vvl::Pipeline& pipeline, const Location& loc) const {
    bool skip = false;

    const bool using_tile_image_capability = module_state.HasCapability(spv::CapabilityTileImageColorReadAccessEXT) ||
                                             module_state.HasCapability(spv::CapabilityTileImageDepthReadAccessEXT) ||
                                             module_state.HasCapability(spv::CapabilityTileImageStencilReadAccessEXT);

    if (!using_tile_image_capability) {
        // None of the capabilities exist.
        return skip;
    }

    auto rp = pipeline.GraphicsCreateInfo().renderPass;
    if (rp != VK_NULL_HANDLE) {
        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-renderPass-08710", module_state.handle(), loc,
                         "shader %s is using capabilities (%s), but renderpass (%s) is not VK_NULL_HANDLE.",
                         entrypoint.Describe().c_str(), GetShaderTileImageCapabilitiesString(module_state).c_str(),
                         FormatHandle(rp).c_str());
    }

    const bool mode_early_fragment_test = entrypoint.execution_mode.Has(spirv::ExecutionModeSet::early_fragment_test_bit);
    if (module_state.static_data_.has_shader_tile_image_depth_read) {
        const auto* ds_state = pipeline.DepthStencilState();
        const bool write_enabled =
            !pipeline.IsDynamic(CB_DYNAMIC_STATE_DEPTH_WRITE_ENABLE) && (ds_state && ds_state->depthWriteEnable);
        if (mode_early_fragment_test && write_enabled) {
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-pStages-08711", module_state.handle(), loc,
                             "shader %s contains OpDepthAttachmentReadEXT, and depthWriteEnable is not false.",
                             entrypoint.Describe().c_str());
        }
    }

    if (module_state.static_data_.has_shader_tile_image_stencil_read) {
        const auto* ds_state = pipeline.DepthStencilState();
        const bool is_write_mask_set = !pipeline.IsDynamic(CB_DYNAMIC_STATE_STENCIL_WRITE_MASK) &&
                                       (ds_state && (ds_state->front.writeMask != 0 || ds_state->back.writeMask != 0));
        if (mode_early_fragment_test && is_write_mask_set) {
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-pStages-08712", module_state.handle(), loc,
                             "shader %s contains OpStencilAttachmentReadEXT, and stencil write mask is not equal to 0 for "
                             "both front(%" PRIu32 ") and back (%" PRIu32 ").",
                             entrypoint.Describe().c_str(), ds_state->front.writeMask, ds_state->back.writeMask);
        }
    }

    bool using_tile_image_op = module_state.static_data_.has_shader_tile_image_depth_read ||
                               module_state.static_data_.has_shader_tile_image_stencil_read ||
                               module_state.static_data_.has_shader_tile_image_color_read;
    const auto* ms_state = pipeline.MultisampleState();
    if (using_tile_image_op && ms_state && ms_state->sampleShadingEnable && (ms_state->minSampleShading != 1.0)) {
        skip |= LogError("VUID-RuntimeSpirv-minSampleShading-08732", module_state.handle(), loc,
                         "shader %s is using tile attachment reads, but minSampleShading (%f) is not equal to 1.0.",
                         entrypoint.Describe().c_str(), ms_state->minSampleShading);
    }

    return skip;
}

struct SpirvValDiagInfo {
    std::string vuid;
    std::string error_msg;
};

// We want to search inside the spirv-val error message to see if there is VUID in it as it allows people to silence just that VUID
// and not the whole spirv-val check
static SpirvValDiagInfo ExtractSpirvValDiagnostic(spv_diagnostic diag, const char* fallback_vuid) {
    SpirvValDiagInfo info;
    info.vuid = fallback_vuid;
    info.error_msg = (diag && diag->error) ? diag->error : "(no error text)";

    // Note: Will always start with "[VUID-xxx-00000]" if there is one
    if (diag && diag->error && std::strncmp(info.error_msg.c_str(), "[VUID", 5) == 0) {
        const char* start = info.error_msg.c_str();
        const char* bracket_end = std::strchr(start, ']');
        const char* space_pos = std::strchr(start, ' ');

        if (bracket_end && (!space_pos || bracket_end < space_pos)) {
            const size_t vuid_len = bracket_end - start - 1;
            info.vuid.assign(start + 1, vuid_len);

            // Strip "] " from the beginning of the error message
            size_t strip_pos = (bracket_end - start) + 1;
            if (strip_pos < info.error_msg.length() && info.error_msg[strip_pos] == ' ') {
                strip_pos++;
            }
            info.error_msg = info.error_msg.substr(strip_pos);
        }
    }
    return info;
}

// Validate the VkPipelineShaderStageCreateInfo from the various pipeline types or a Shader Object
bool CoreChecks::ValidateShaderStage(const ShaderStageState& stage_state, const vvl::Pipeline* pipeline,
                                     const Location& loc) const {
    bool skip = false;
    const VkShaderStageFlagBits stage = stage_state.GetStage();

    // First validate all things that don't require valid SPIR-V
    // this is found when using VK_EXT_shader_module_identifier
    skip |= ValidateSpecializations(stage_state.GetSpecializationInfo(), loc.dot(Field::pSpecializationInfo));
    if (pipeline) {
        skip |= ValidateShaderStageMaxResources(stage, *pipeline, loc);
        if (const auto* pipeline_robustness_info =
                vku::FindStructInPNextChain<VkPipelineRobustnessCreateInfo>(stage_state.GetPNext())) {
            skip |= ValidatePipelineRobustnessCreateInfo(*pipeline, *pipeline_robustness_info, loc);
        }
    }

    // Skip if VK_VALIDATION_FEATURE_DISABLE_SHADERS_EXT is set
    // Both the validation and running spirv-opt on the spec constants really makes this function slow
    // See https://github.com/KhronosGroup/Vulkan-ValidationLayers/pull/10566 for more info
    if (disabled[shader_validation]) {
        return skip;
    }

    if ((pipeline && pipeline->uses_shader_module_id) || !stage_state.spirv_state) {
        return skip;  // these edge cases should be validated already
    }

    if (!stage_state.spirv_state->valid_spirv) {
        return skip;  // checked elsewhere
    }

    if (!stage_state.entrypoint) {
        const char* vuid = pipeline ? "VUID-VkPipelineShaderStageCreateInfo-pName-00707" : "VUID-VkShaderCreateInfoEXT-pName-08440";
        std::ostringstream err;
        err << "\"" << stage_state.GetPName() << "\" entry point not found for stage " << string_VkShaderStageFlagBits(stage)
            << ".";
        if (stage_state.spirv_state->static_data_.entry_points.size() == 1) {
            auto entry_point = stage_state.spirv_state->static_data_.entry_points[0];
            if (entry_point) {
                if (entry_point->stage != stage) {
                    err << " (Seems like you accidentally created your SPIR-V with "
                        << string_VkShaderStageFlagBits(entry_point->stage) << " so the entry point is not matching up)";
                } else {
                    err << " (The only entry point found was \"" << entry_point->name << "\" for "
                        << string_VkShaderStageFlagBits(entry_point->stage) << ")";
                    if (entry_point->name == "main") {
                        err << "\nSome shading languages will let you name the main function something else, but when "
                               "compiled to SPIR-V, it will keep it as 'main' to match defaults found in other shading languages "
                               "such "
                               "as GLSL. It is also valid in a single SPIR-V binary to have 'main' for two different stages.";
                    }
                }
            }
        } else {
            err << " The following entry points were found in the SPIR-V module:\n";
            for (const auto& entry_point : stage_state.spirv_state->static_data_.entry_points) {
                if (!entry_point) continue;
                err << "\"" << entry_point->name << "\"\t(" << string_VkShaderStageFlagBits(entry_point->stage) << ")\n";
            }
        }
        return LogError(vuid, device, loc.dot(Field::pName), "%s", err.str().c_str());
    }

    std::shared_ptr<const spirv::Module> module_state_ptr = stage_state.spirv_state;
    std::shared_ptr<const spirv::EntryPoint> entrypoint_ptr = stage_state.entrypoint;

    // If specialization-constant instructions are present in the shader, the specializations should be applied.
    // If spirv_const_fold is turned off, the default spec constants values are used
    if (module_state_ptr->static_data_.has_specialization_constants && global_settings.spirv_const_fold) {
        // setup the call back if the optimizer fails
        spvtools::Optimizer optimizer(spirv_environment);
        spvtools::MessageConsumer consumer = [&skip, &module_state_ptr, &stage, loc, this](
                                                 spv_message_level_t level, const char* source, const spv_position_t& position,
                                                 const char* message) {
            skip |= LogError("VUID-VkPipelineShaderStageCreateInfo-module-parameter", device, loc,
                             "%s failed in spirv-opt because it does not contain valid spirv for stage %s. %s",
                             FormatHandle(module_state_ptr->handle()).c_str(), string_VkShaderStageFlagBits(stage), message);
        };
        optimizer.SetMessageConsumer(consumer);

        // The app might be using the default spec constant values, but if they pass values at runtime to the pipeline then need to
        // use those values to apply to the spec constants
        auto const& specialization_info = stage_state.GetSpecializationInfo();
        if (specialization_info != nullptr && specialization_info->mapEntryCount > 0 &&
            specialization_info->pMapEntries != nullptr) {
            // Gather the specialization-constant values.
            auto const& specialization_data = reinterpret_cast<uint8_t const*>(specialization_info->pData);
            // This must be std:: (instead of vvl::) to work with spvtools
            // The value here is a vector because SPIR-V is a list of 32-bit words, so for 64-bit constants, it takes 2 words
            // Also there is now OpSpecConstantData that can take any number of values
            std::unordered_map<uint32_t, std::vector<uint32_t>> id_value_map;
            id_value_map.reserve(specialization_info->mapEntryCount);

            // < spec_id, map_entry_index >
            vvl::unordered_map<uint32_t, uint32_t> spec_constant_data;

            // spirv-val makes sure every OpSpecConstant has a OpDecoration.
            for (const auto& [result_id, spec_id] : module_state_ptr->static_data_.id_to_spec_id) {
                VkSpecializationMapEntry map_entry = {spirv::kInvalidValue, 0, 0};
                uint32_t map_entry_index = 0;
                for (uint32_t i = 0; i < specialization_info->mapEntryCount; i++) {
                    if (specialization_info->pMapEntries[i].constantID == spec_id) {
                        map_entry = specialization_info->pMapEntries[i];
                        map_entry_index = i;
                        break;
                    }
                }

                // "If a constantID value is not a specialization constant ID used in the shader, that map entry does not affect the
                // behavior of the pipeline."
                if (map_entry.constantID == spirv::kInvalidValue) {
                    continue;
                }

                uint32_t spec_const_size = spirv::kInvalidValue;
                const spirv::Instruction* def_insn = module_state_ptr->FindDef(result_id);
                const spirv::Instruction* type_insn = module_state_ptr->FindDef(def_insn->Word(1));

                // Specialization constants can only be scalar (or array of scalar)
                const uint32_t type_opcode = type_insn->Opcode();
                switch (type_opcode) {
                    case spv::OpTypeBool:
                        // "If the specialization constant is of type boolean, size must be the byte size of VkBool32"
                        spec_const_size = sizeof(VkBool32);
                        break;
                    case spv::OpTypeInt:
                    case spv::OpTypeFloat:
                        spec_const_size = type_insn->Word(2) / 8;
                        break;
                    case spv::OpTypeArray:
                        // Array is not allowed for other spec constants
                        assert(def_insn->Opcode() == spv::OpSpecConstantDataKHR);
                        break;
                    default:
                        break;  // spirv-val should catch this
                }

                if (def_insn->Opcode() == spv::OpSpecConstantDataKHR) {
                    spec_constant_data.emplace(spec_id, map_entry_index);
                } else if (map_entry.size != spec_const_size) {
                    std::ostringstream ss;
                    ss << "specialization constant (OpDecorate %" << result_id << " SpecId " << spec_id << ") ";
                    if (module_state_ptr->handle() != NullVulkanTypedHandle) {
                        // if inlined or shader object, not handle to print
                        ss << "in " << FormatHandle(module_state_ptr->handle());
                    }
                    ss << " is mapped to pMapEntries[" << map_entry_index << "].size of " << map_entry.size
                       << ", but the shader references a ";
                    if (type_opcode == spv::OpTypeBool) {
                        ss << "OpTypeBool is defined as sizeof(VkBool32) (" << sizeof(VkBool32) << " bytes";
                    } else if (type_opcode == spv::OpTypeInt) {
                        ss << type_insn->Word(2) << "-bit OpTypeInt which requires " << spec_const_size << " bytes";
                    } else if (type_opcode == spv::OpTypeFloat) {
                        ss << type_insn->Word(2) << "-bit OpTypeFloat which requires " << spec_const_size << " bytes";
                    }
                    skip |= LogError("VUID-VkSpecializationMapEntry-constantID-00776", device, loc, "%s", ss.str().c_str());
                }

                if ((map_entry.offset + map_entry.size) <= specialization_info->dataSize) {
                    // Allocate enough room for ceil(map_entry.size / 4) to store entries
                    std::vector<uint32_t> entry_data((map_entry.size + 4 - 1) / 4, 0);
                    uint8_t* out_p = reinterpret_cast<uint8_t*>(entry_data.data());
                    const uint8_t* const start_in_p = specialization_data + map_entry.offset;
                    const uint8_t* const end_in_p = start_in_p + map_entry.size;

                    std::copy(start_in_p, end_in_p, out_p);
                    id_value_map.emplace(map_entry.constantID, std::move(entry_data));
                }
            }

            // We need to validate the size of OpSpecConstantData afterwards because the lenght is likely an OpSpecConstant and we
            // need to get the value
            for (auto& [spec_id, map_entry_index] : spec_constant_data) {
                VkSpecializationMapEntry map_entry = specialization_info->pMapEntries[map_entry_index];
                uint32_t result_id = 0;
                for (const auto& [search_result_id, search_spec_id] : module_state_ptr->static_data_.id_to_spec_id) {
                    if (search_spec_id == spec_id) {
                        result_id = search_result_id;
                        break;
                    }
                }
                assert(result_id != 0);  // we already found above, so can find again here

                const spirv::Instruction* def_insn = module_state_ptr->FindDef(result_id);
                const spirv::Instruction* type_insn = module_state_ptr->FindDef(def_insn->Word(1));
                // Result Type must be an array of scalar integer type elements
                assert(type_insn->Opcode() == spv::OpTypeArray);
                const spirv::Instruction* element_inst = module_state_ptr->FindDef(type_insn->Word(2));
                assert(element_inst->Opcode() == spv::OpTypeInt);
                const uint32_t byte_width = element_inst->Word(2) / 8;
                const spirv::Instruction* length_inst = module_state_ptr->FindDef(type_insn->Word(3));

                uint32_t length = 0;
                if (length_inst->Opcode() == spv::OpConstant) {
                    length = length_inst->Word(3);
                } else if (length_inst->Opcode() == spv::OpSpecConstant) {
                    // Need to do yet-another-reverse lookup to get the length SpecId
                    uint32_t length_spec_id = spirv::kInvalidValue;
                    for (const auto& [search_result_id, search_spec_id] : module_state_ptr->static_data_.id_to_spec_id) {
                        if (search_result_id == length_inst->ResultId()) {
                            length_spec_id = search_spec_id;
                            break;
                        }
                    }

                    auto it = id_value_map.find(length_spec_id);
                    if (it != id_value_map.end()) {
                        // Hard assumption this is not a 64-bit spec constant
                        length = it->second[0];
                    } else {
                        length = length_inst->Word(3);  // use the default
                    }
                } else {
                    assert(false);  // spirv-val should catch
                }

                if ((length * byte_width) != map_entry.size) {
                    std::ostringstream ss;
                    ss << "specialization constant (OpDecorate %" << result_id << " SpecId " << spec_id << ") ";
                    if (module_state_ptr->handle() != NullVulkanTypedHandle) {
                        ss << "in " << FormatHandle(module_state_ptr->handle());
                    }
                    ss << " is mapped to pMapEntries[" << map_entry_index << "].size of " << map_entry.size << ", but should be "
                       << (length * byte_width) << " since the shader references an OpSpecConstantDataKHR where each element is "
                       << byte_width << " bytes and the OpTypeArray has a length of " << length;
                    skip |= LogError("VUID-VkSpecializationMapEntry-constantID-00776", device, loc, "%s", ss.str().c_str());
                }
            }

            // This pass takes the runtime spec const values and applies it into the SPIR-V
            // will turn a spec constant like
            //     OpSpecConstant %uint 1
            // to a use the value passed in instead (for example if the value is 32) so now it looks like
            //     OpSpecConstant %uint 32
            optimizer.RegisterPass(spvtools::CreateSetSpecConstantDefaultValuePass(id_value_map));
        }

        // This pass will turn OpSpecConstant into a OpConstant (also OpSpecConstantTrue/OpSpecConstantFalse)
        optimizer.RegisterPass(spvtools::CreateFreezeSpecConstantValuePass());
        // Using the new frozen OpConstant all OpSpecConstantComposite can be resolved turning them into OpConstantComposite
        // This is need incase a shdaer looks like:
        //
        //     layout(constant_id = 0) const uint x = 64;
        //     shared uint arr[x > 64 ? 64 : x];
        //
        // this will generate branch/switch statements that we want to leverage spirv-opt to apply to make parsing easier
        optimizer.RegisterPass(spvtools::CreateFoldSpecConstantOpAndCompositePass());

        // Apply the specialization-constant values and revalidate the shader module is valid.
        // Example of the SPIR-V Optimization occuring (https://godbolt.org/z/Y7WYczEq4)
        std::vector<uint32_t> specialized_spirv;
        auto const optimized = optimizer.Run(module_state_ptr->words_.data(), module_state_ptr->words_.size(), &specialized_spirv,
                                             spirv_val_options, true);
        if (optimized) {
            spv_context ctx = spvContextCreate(spirv_environment);
            spv_const_binary_t binary{specialized_spirv.data(), specialized_spirv.size()};
            spv_diagnostic diag = nullptr;
            auto const spv_valid = spvValidateWithOptions(ctx, spirv_val_options, &binary, &diag);
            if (spv_valid != SPV_SUCCESS) {
                const char* fallback_vuid = pipeline ? "VUID-VkPipelineShaderStageCreateInfo-pSpecializationInfo-06849"
                                                     : "VUID-VkShaderCreateInfoEXT-pCode-08460";
                const auto diag_info = ExtractSpirvValDiagnostic(diag, fallback_vuid);
                std::string name = pipeline ? FormatHandle(module_state_ptr->handle()) : "shader object";
                skip |= LogError(diag_info.vuid.c_str(), device, loc,
                                 "after specialization was applied, %s produces a spirv-val error (stage %s):\n%s\nCommand to "
                                 "reproduce:\n\t%s\n",
                                 name.c_str(), string_VkShaderStageFlagBits(stage), diag_info.error_msg.c_str(),
                                 spirv_val_command.c_str());
            }

            // There is only 3 real ways to handle spec constants
            // 1. Store a new copy of spirv::Module in the pipeline/shaderObject and do nothing during vkCreateShaderModule.
            //    For things using spec const, this will save parsing twice,
            //    but for things not using spec constant, it is wasted memory to store and wasted time to parse twice.
            // 2. Do the 3 spirv-opt passes internally, the first 2 are easy, but CreateFoldSpecConstantOpAndCompositePass is harder
            //    There are a lot of cases to get correct, but if we have this, then we just have the mapping internally.
            // 3. [what we do] Realize most things don't really care about the spec constants, but those that do, have all been
            //    (hopefully) funneled into the this ValidateShaderStage function below.
            //    So now all the checks below can assume things are OpConstant. The 2 main drawbacks are:
            //      one, this is not obvious what is going on here at first
            //      two, the new optimized spirv::Module IDs will not match the old one, for error messages that is fine, because we
            //           should just be using ShaderDebugInfo anyway.
            //
            // Side note, according to https://github.com/KhronosGroup/Vulkan-Docs/issues/1671 anything labeled as "static use"
            // (such as if an input is used or not) don't have to be checked post spec constants freezing since the device compiler
            // is not guaranteed to run things such as dead-code elimination.
            module_state_ptr =
                std::make_shared<spirv::Module>(vvl::make_span<const uint32_t>(specialized_spirv.data(), specialized_spirv.size()));
            entrypoint_ptr = module_state_ptr->FindEntrypoint(entrypoint_ptr->name.c_str(), entrypoint_ptr->stage);
            assert(entrypoint_ptr);  // spirv-opt won't change Entrypoint Name/stage

            spvDiagnosticDestroy(diag);
            spvContextDestroy(ctx);
        } else {
            // Should never get here, but better then asserting
            const char* vuid = pipeline ? "VUID-VkPipelineShaderStageCreateInfo-pSpecializationInfo-06849"
                                        : "VUID-VkShaderCreateInfoEXT-pCode-08460";
            skip |= LogError(vuid, device, loc,
                             "%s shader (stage %s) attempted to apply specialization constants with spirv-opt but failed.",
                             FormatHandle(module_state_ptr->handle()).c_str(), string_VkShaderStageFlagBits(stage));
            return skip;
        }

        if (skip) {
            return skip;  // if spec constants have errors, can produce false positives later
        }
    }

    const spirv::Module& module_state = *module_state_ptr;
    const spirv::EntryPoint& entrypoint = *entrypoint_ptr;

    spirv::LocalSize local_size = module_state.FindLocalSize(entrypoint);

    skip |= ValidateImageWrite(module_state, entrypoint, loc);
    skip |= ValidateShaderExecutionModes(module_state, entrypoint, stage, pipeline, loc);
    skip |= ValidateBuiltInLimits(module_state, entrypoint, pipeline, loc);
    skip |= ValidatePushConstantUsage(module_state, entrypoint, pipeline, stage_state, loc);
    if (enabled_features.cooperativeMatrix) {
        skip |= ValidateCooperativeMatrix(module_state, entrypoint, stage_state, local_size, loc);
    }
    if (enabled_features.cooperativeVector) {
        skip |= ValidateCooperativeVector(module_state, entrypoint, loc);
    }
    skip |= ValidateShader64BitIndexing(module_state, entrypoint, stage_state, pipeline, loc);
    skip |= ValidateVectorTypes(module_state, entrypoint, loc);
    if (enabled_features.descriptorHeap) {
        skip |= ValidateShaderDescriptorSetAndBindingMappingInfo(module_state, entrypoint, pipeline, stage_state, loc);
        skip |= ValidateDescriptorHeapStructs(module_state, entrypoint, loc);
    }

    if (pipeline) {
        if (enabled_features.transformFeedback) {
            skip |= ValidateTransformFeedbackPipeline(module_state, entrypoint, *pipeline, loc);
        }
        if (enabled_features.primitiveFragmentShadingRate) {
            skip |= ValidatePrimitiveRateShaderState(module_state, entrypoint, *pipeline, stage, loc);
        }
        if (enabled_features.customResolve || IsExtEnabled(extensions.vk_qcom_render_pass_shader_resolve)) {
            skip |= ValidateSubpassCustomeResolve(module_state, entrypoint, stage, *pipeline, loc);
        }
        skip |= ValidatePointSizeShaderState(module_state, entrypoint, *pipeline, stage, loc);
        skip |= ValidatePrimitiveTopology(module_state, entrypoint, *pipeline, loc);

        if (stage == VK_SHADER_STAGE_FRAGMENT_BIT) {
            if (IsExtEnabled(extensions.vk_ext_shader_tile_image)) {
                skip |= ValidateShaderTileImage(module_state, entrypoint, *pipeline, loc);
            }

            if (pipeline->GraphicsCreateInfo().renderPass == VK_NULL_HANDLE &&
                module_state.HasCapability(spv::CapabilityInputAttachment) && !enabled_features.dynamicRenderingLocalRead) {
                skip |=
                    LogError("VUID-VkGraphicsPipelineCreateInfo-renderPass-06061", device, loc,
                             "shader %s is being created with fragment shader with InputAttachment capability, but renderPass is "
                             "VK_NULL_HANDLE.\nHint: It is only possible to use input attachments with dynamic rendering if the "
                             "dynamicRenderingLocalRead feature is enabled",
                             entrypoint.Describe().c_str());
            }
        }
    } else {
        if (enabled_features.transformFeedback) {
            skip |= ValidateTransformFeedbackShaderObject(module_state, entrypoint, *stage_state.shader_object_create_info, loc);
        }
    }

    // Only stages this matters to calculate workgroup shared memory size
    if (stage == VK_SHADER_STAGE_COMPUTE_BIT || stage == VK_SHADER_STAGE_TASK_BIT_EXT || stage == VK_SHADER_STAGE_MESH_BIT_EXT) {
        bool fail = false;
        const uint32_t limit = phys_dev_props.limits.maxComputeWorkGroupInvocations;
        uint64_t invocations = static_cast<uint64_t>(local_size.x) * static_cast<uint64_t>(local_size.y);
        // Prevent overflow.
        if (invocations > limit) {
            fail = true;
        }
        invocations *= local_size.z;
        if (invocations > limit) {
            fail = true;
        }

        if (fail && stage == VK_SHADER_STAGE_COMPUTE_BIT) {
            skip |= LogError("VUID-RuntimeSpirv-x-06432", module_state.handle(), loc,
                             "shader %s LocalSize (%s) exceeds device limit maxComputeWorkGroupInvocations (%" PRIu32 ").",
                             entrypoint.Describe().c_str(), local_size.ToString().c_str(),
                             phys_dev_props.limits.maxComputeWorkGroupInvocations);
        }

        skip |= ValidateRequiredSubgroupSize(module_state, entrypoint, stage_state, invocations, local_size, loc);
        skip |= ValidateWorkgroupSharedMemory(module_state, entrypoint, stage, loc);

        if (stage == VK_SHADER_STAGE_COMPUTE_BIT) {
            skip |= ValidateComputeWorkGroupSizes(module_state, entrypoint, stage_state, local_size, loc);
        } else if (stage == VK_SHADER_STAGE_TASK_BIT_EXT || stage == VK_SHADER_STAGE_MESH_BIT_EXT) {
            skip |= ValidateTaskMeshWorkGroupSizes(module_state, entrypoint, local_size, loc);
        }
    }

    for (const auto& variable : entrypoint.resource_interface_variables) {
        skip |= ValidateShaderInterfaceVariable(module_state, entrypoint, stage_state, variable, loc);

        // We need to do the InputAttachment stuff here (instead of in ValidateInterfaceFragmentOutput) is because we want to handle
        // the indexing of SpecConstants
        if (entrypoint.has_input_attachment && variable.decorations.Has(spirv::DecorationSet::input_attachment_bit) && pipeline) {
            if (const auto& rp_state = pipeline->RenderPassState()) {
                if (rp_state->UsesDynamicRendering()) {
                    skip |= ValidateShaderInputAttachmentDynamicRendering(module_state, entrypoint, variable, *pipeline, *rp_state,
                                                                          loc);
                } else {
                    skip |= ValidateShaderInputAttachmentRenderPass(module_state, entrypoint, variable, *pipeline, *rp_state, loc);
                }
            }
        }
    }

    return skip;
}

uint32_t CoreChecks::CalcShaderStageCount(const vvl::Pipeline& pipeline, VkShaderStageFlagBits stageBit) const {
    uint32_t total = 0;
    for (const auto& stage_ci : pipeline.shader_stages_ci) {
        if (stage_ci.stage == stageBit) {
            total++;
        }
    }

    if (pipeline.ray_tracing_library_ci) {
        for (uint32_t i = 0; i < pipeline.ray_tracing_library_ci->libraryCount; ++i) {
            auto library_pipeline = Get<vvl::Pipeline>(pipeline.ray_tracing_library_ci->pLibraries[i]);
            if (!library_pipeline) continue;
            total += CalcShaderStageCount(*library_pipeline, stageBit);
        }
    }

    return total;
}

// This is done in PreCallRecord to help with the interaction with GPU-AV
// See diagram on https://github.com/KhronosGroup/Vulkan-ValidationLayers/pull/6230
void CoreChecks::PreCallRecordCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule,
                                                 const RecordObject& record_obj, chassis::CreateShaderModule& chassis_state) {
    // Normally would validate in PreCallValidate, but need a non-const function to update chassis_state
    // This is on the stack, we don't have to worry about threading hazards and this could be moved and used const_cast
    chassis_state.skip |=
        stateless_spirv_validator.Validate(*chassis_state.module_state, chassis_state.stateless_data, record_obj.location);
}

void CoreChecks::PreCallRecordCreateShadersEXT(VkDevice device, uint32_t createInfoCount, const VkShaderCreateInfoEXT* pCreateInfos,
                                               const VkAllocationCallbacks* pAllocator, VkShaderEXT* pShaders,
                                               const RecordObject& record_obj, chassis::ShaderObject& chassis_state) {
    // For ShaderObjects, to validate most things we need to first parse the SPIR-V.
    // We use to parse both at PreCallValidate and PreCallRecord which was wasteful.
    // We now parse it at PreCallRecord (so we can store it) and then just do the validation here
    chassis_state.skip = ValidateCreateShadersSpirv(createInfoCount, pCreateInfos, record_obj.location, chassis_state);
}

bool CoreChecks::RunSpirvValidation(spv_const_binary_t& binary, const Location& loc, ValidationCache* cache) const {
    bool skip = false;

    if (global_settings.debug_disable_spirv_val) {
        return skip;
    }

    uint32_t hash = 0;
    if (cache) {
        hash = hash_util::Hash32((void*)binary.code, binary.wordCount * sizeof(uint32_t));
        if (cache->Contains(hash)) {
            return skip;
        }
    }

    // Use SPIRV-Tools validator to try and catch any issues with the module itself. If specialization constants are present,
    // the default values will be used during validation.
    spv_context ctx = spvContextCreate(spirv_environment);
    spv_diagnostic diag = nullptr;
    const spv_result_t spv_valid = spvValidateWithOptions(ctx, spirv_val_options, &binary, &diag);
    if (spv_valid != SPV_SUCCESS) {
        // Umbrella VUID if we can't find one in spirv-val
        const char* fallback_vuid = loc.function == Func::vkCreateShadersEXT ? "VUID-VkShaderCreateInfoEXT-pCode-08737"
                                                                             : "VUID-VkShaderModuleCreateInfo-pCode-08737";

        const auto diag_info = ExtractSpirvValDiagnostic(diag, fallback_vuid);

        if (spv_valid == SPV_WARNING) {
            skip |= LogWarning(diag_info.vuid.c_str(), device, loc.dot(Field::pCode),
                               "(spirv-val produced a warning):\n%s\nCommand to reproduce:\n\t%s\n", diag_info.error_msg.c_str(),
                               spirv_val_command.c_str());
        } else {
            skip |= LogError(diag_info.vuid.c_str(), device, loc.dot(Field::pCode),
                             "(spirv-val produced an error):\n%s\nCommand to reproduce:\n\t%s\n", diag_info.error_msg.c_str(),
                             spirv_val_command.c_str());
        }
    } else if (cache) {
        // No point to cache anything that is not valid, or it will get suppressed on the next run
        cache->Insert(hash);
    }

    spvDiagnosticDestroy(diag);
    spvContextDestroy(ctx);

    return skip;
}

bool CoreChecks::ValidateShaderModuleCreateInfo(const VkShaderModuleCreateInfo& create_info,
                                                const Location& create_info_loc) const {
    bool skip = false;

    if (disabled[shader_validation]) {
        return skip;  // VK_VALIDATION_FEATURE_DISABLE_SHADERS_EXT
    } else if (!create_info.pCode) {
        return skip;  // will be caught elsewhere
    }

    // This extension is meant for tooling, but still valid to be used, if used, we need to detect if GLSL
    if (IsExtEnabled(extensions.vk_nv_glsl_shader)) {
        if (strncmp((char*)create_info.pCode, "#version", 8) == 0) {
            return skip;  // incoming GLSL
        }
    }

    const uint32_t first_dword = create_info.pCode[0];
    if (!IsIntegerMultipleOf(create_info.codeSize, 4)) {
        skip |=
            LogError("VUID-VkShaderModuleCreateInfo-codeSize-08735", device, create_info_loc.dot(Field::codeSize),
                     "(%zu) must be a multiple of 4. You might have forgot to multiply by sizeof(uint32_t).", create_info.codeSize);
    } else if (first_dword != spv::MagicNumber) {
        skip |= LogError("VUID-VkShaderModuleCreateInfo-pCode-08738", device, create_info_loc.dot(Field::pCode),
                         "doesn't point to a SPIR-V module. The first dword (0x%" PRIx32
                         ") is not the SPIR-V MagicNumber (0x07230203).",
                         first_dword);
    } else {
        // if pCode is garbage, don't pass along to spirv-val

        const auto validation_cache_ci = vku::FindStructInPNextChain<VkShaderModuleValidationCacheCreateInfoEXT>(create_info.pNext);
        ValidationCache* cache =
            validation_cache_ci ? CastFromHandle<ValidationCache*>(validation_cache_ci->validationCache) : nullptr;
        // If app isn't using a shader validation cache, use the default one from CoreChecks
        if (!cache) {
            cache = CastFromHandle<ValidationCache*>(core_validation_cache);
        }

        spv_const_binary_t binary{create_info.pCode, create_info.codeSize / sizeof(uint32_t)};
        skip |= RunSpirvValidation(binary, create_info_loc, cache);
    }

    return skip;
}

bool CoreChecks::PreCallValidateCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule,
                                                   const ErrorObject& error_obj) const {
    return ValidateShaderModuleCreateInfo(*pCreateInfo, error_obj.location.dot(Field::pCreateInfo));
}

bool CoreChecks::PreCallValidateGetShaderModuleIdentifierEXT(VkDevice device, VkShaderModule shaderModule,
                                                             VkShaderModuleIdentifierEXT* pIdentifier,
                                                             const ErrorObject& error_obj) const {
    bool skip = false;
    if (!(enabled_features.shaderModuleIdentifier)) {
        skip |= LogError("VUID-vkGetShaderModuleIdentifierEXT-shaderModuleIdentifier-06884", shaderModule, error_obj.location,
                         "the shaderModuleIdentifier feature was not enabled.");
    }
    return skip;
}

bool CoreChecks::PreCallValidateGetShaderModuleCreateInfoIdentifierEXT(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                                                       VkShaderModuleIdentifierEXT* pIdentifier,
                                                                       const ErrorObject& error_obj) const {
    bool skip = false;
    if (!(enabled_features.shaderModuleIdentifier)) {
        skip |= LogError("VUID-vkGetShaderModuleCreateInfoIdentifierEXT-shaderModuleIdentifier-06885", device, error_obj.location,
                         "the shaderModuleIdentifier feature was not enabled.");
    }
    return skip;
}

bool CoreChecks::ValidateRequiredSubgroupSize(const spirv::Module& module_state, const spirv::EntryPoint& entrypoint,
                                              const ShaderStageState& stage_state, uint64_t invocations,
                                              const spirv::LocalSize& local_size, const Location& loc) const {
    bool skip = false;

    const auto* required_subgroup_size_ci =
        vku::FindStructInPNextChain<VkPipelineShaderStageRequiredSubgroupSizeCreateInfo>(stage_state.GetPNext());
    if (!required_subgroup_size_ci) return skip;

    const Location pNext_loc = loc.pNext(Struct::VkPipelineShaderStageRequiredSubgroupSizeCreateInfo);

    const uint32_t required_subgroup_size = required_subgroup_size_ci->requiredSubgroupSize;
    if (!enabled_features.subgroupSizeControl) {
        skip |= LogError("VUID-VkPipelineShaderStageCreateInfo-pNext-02755", module_state.handle(), pNext_loc,
                         "the subgroupSizeControl feature was not enabled\n%s",
                         PrintPNextChain(Struct::Empty, stage_state.GetPNext()).c_str());
    }
    if ((phys_dev_props_core13.requiredSubgroupSizeStages & stage_state.GetStage()) == 0) {
        skip |= LogError("VUID-VkPipelineShaderStageCreateInfo-pNext-02755", module_state.handle(), loc,
                         "%s is not in requiredSubgroupSizeStages (%s).", string_VkShaderStageFlagBits(stage_state.GetStage()),
                         string_VkShaderStageFlags(phys_dev_props_core13.requiredSubgroupSizeStages).c_str());
    }
    if ((invocations > required_subgroup_size * phys_dev_props_core13.maxComputeWorkgroupSubgroups)) {
        skip |= LogError("VUID-VkPipelineShaderStageCreateInfo-pNext-02756", module_state.handle(), loc,
                         "shader %s local workgroup size (%s) is greater than requiredSubgroupSize (%" PRIu32
                         ") * maxComputeWorkgroupSubgroups (%" PRIu32 ").",
                         entrypoint.Describe().c_str(), local_size.ToString().c_str(), required_subgroup_size,
                         phys_dev_props_core13.maxComputeWorkgroupSubgroups);
    }
    if (stage_state.pipeline_create_info &&
        (stage_state.pipeline_create_info->flags & VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT) > 0) {
        if (!IsIntegerMultipleOf(local_size.x, required_subgroup_size)) {
            skip |= LogError("VUID-VkPipelineShaderStageCreateInfo-pNext-02757", module_state.handle(), loc,
                             "shader %s local workgroup size x (%" PRIu32
                             ") is not a multiple of "
                             "requiredSubgroupSize (%" PRIu32 ").",
                             entrypoint.Describe().c_str(), local_size.x, required_subgroup_size);
        }
    }
    if (!IsPowerOfTwo(required_subgroup_size)) {
        skip |= LogError("VUID-VkPipelineShaderStageRequiredSubgroupSizeCreateInfo-requiredSubgroupSize-02760",
                         module_state.handle(), pNext_loc.dot(Field::requiredSubgroupSizeStages),
                         "(%" PRIu32 ") is not a power of 2.", required_subgroup_size);
    }
    if (required_subgroup_size < phys_dev_props_core13.minSubgroupSize) {
        skip |=
            LogError("VUID-VkPipelineShaderStageRequiredSubgroupSizeCreateInfo-requiredSubgroupSize-02761", module_state.handle(),
                     pNext_loc.dot(Field::requiredSubgroupSizeStages), "(%" PRIu32 ") is less than minSubgroupSize (%" PRIu32 ").",
                     required_subgroup_size, phys_dev_props_core13.minSubgroupSize);
    }
    if (required_subgroup_size > phys_dev_props_core13.maxSubgroupSize) {
        skip |= LogError("VUID-VkPipelineShaderStageRequiredSubgroupSizeCreateInfo-requiredSubgroupSize-02762",
                         module_state.handle(), pNext_loc.dot(Field::requiredSubgroupSizeStages),
                         "(%" PRIu32 ") is greater than maxSubgroupSize (%" PRIu32 ").", required_subgroup_size,
                         phys_dev_props_core13.maxSubgroupSize);
    }

    return skip;
}

bool CoreChecks::ValidateComputeWorkGroupSizes(const spirv::Module& module_state, const spirv::EntryPoint& entrypoint,
                                               const ShaderStageState& stage_state, const spirv::LocalSize& local_size,
                                               const Location& loc) const {
    bool skip = false;

    if (local_size.x == 0) {
        return skip;
    }

    if (local_size.x > phys_dev_props.limits.maxComputeWorkGroupSize[0]) {
        skip |= LogError("VUID-RuntimeSpirv-x-06429", module_state.handle(), loc,
                         "shader %s LocalSize X (%" PRIu32 ") exceeds device limit maxComputeWorkGroupSize[0] (%" PRIu32 ").",
                         entrypoint.Describe().c_str(), local_size.x, phys_dev_props.limits.maxComputeWorkGroupSize[0]);
    }
    if (local_size.y > phys_dev_props.limits.maxComputeWorkGroupSize[1]) {
        skip |= LogError("VUID-RuntimeSpirv-y-06430", module_state.handle(), loc,
                         "shader %s LocalSize Y (%" PRIu32 ") exceeds device limit maxComputeWorkGroupSize[1] (%" PRIu32 ").",
                         entrypoint.Describe().c_str(), local_size.y, phys_dev_props.limits.maxComputeWorkGroupSize[1]);
    }
    if (local_size.z > phys_dev_props.limits.maxComputeWorkGroupSize[2]) {
        skip |= LogError("VUID-RuntimeSpirv-z-06431", module_state.handle(), loc,
                         "shader %s LocalSize Z (%" PRIu32 ") exceeds device limit maxComputeWorkGroupSize[2] (%" PRIu32 ").",
                         entrypoint.Describe().c_str(), local_size.z, phys_dev_props.limits.maxComputeWorkGroupSize[2]);
    }

    if (stage_state.pipeline_create_info) {
        const auto subgroup_flags = VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT |
                                    VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT;
        if ((stage_state.pipeline_create_info->flags & subgroup_flags) == subgroup_flags) {
            if (!IsIntegerMultipleOf(local_size.x, phys_dev_props_core13.maxSubgroupSize)) {
                skip |= LogError(
                    "VUID-VkPipelineShaderStageCreateInfo-flags-02758", module_state.handle(), loc.dot(Field::flags),
                    "(%s), but local workgroup size X dimension (%" PRIu32
                    ") is not a multiple of VkPhysicalDeviceSubgroupSizeControlPropertiesEXT::maxSubgroupSize (%" PRIu32 ").",
                    string_VkPipelineShaderStageCreateFlags(stage_state.pipeline_create_info->flags).c_str(), local_size.x,
                    phys_dev_props_core13.maxSubgroupSize);
            }
        } else if ((stage_state.pipeline_create_info->flags & VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT) &&
                   (stage_state.pipeline_create_info->flags & VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT) ==
                       0) {
            if (!vku::FindStructInPNextChain<VkPipelineShaderStageRequiredSubgroupSizeCreateInfo>(stage_state.GetPNext())) {
                if (!IsIntegerMultipleOf(local_size.x, phys_dev_props_core11.subgroupSize)) {
                    skip |=
                        LogError("VUID-VkPipelineShaderStageCreateInfo-flags-02759", module_state.handle(), loc.dot(Field::flags),
                                 "(%s), but local workgroup size X dimension (%" PRIu32
                                 ") is not a multiple of VkPhysicalDeviceVulkan11Properties::subgroupSize (%" PRIu32 ").",
                                 string_VkPipelineShaderStageCreateFlags(stage_state.pipeline_create_info->flags).c_str(),
                                 local_size.x, phys_dev_props_core11.subgroupSize);
                }
            }
        }
    } else {
        const bool varying = stage_state.shader_object_create_info->flags & VK_SHADER_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT_EXT;
        const bool full = stage_state.shader_object_create_info->flags & VK_SHADER_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT;
        const auto* required_subgroup_size =
            vku::FindStructInPNextChain<VkShaderRequiredSubgroupSizeCreateInfoEXT>(stage_state.GetPNext());
        if (varying && full) {
            if (!IsIntegerMultipleOf(local_size.x, phys_dev_props_core13.maxSubgroupSize)) {
                skip |= LogError(
                    "VUID-VkShaderCreateInfoEXT-flags-08416", module_state.handle(), loc.dot(Field::flags),
                    "(%s) but local workgroup size X dimension (%" PRIu32
                    ") is not a multiple of VkPhysicalDeviceSubgroupSizeControlPropertiesEXT::maxSubgroupSize (%" PRIu32 ").",
                    string_VkPipelineShaderStageCreateFlags(stage_state.shader_object_create_info->flags).c_str(), local_size.x,
                    phys_dev_props_core13.maxSubgroupSize);
            }
        } else if (full && !varying) {
            if (!required_subgroup_size && !IsIntegerMultipleOf(local_size.x, phys_dev_props_core11.subgroupSize)) {
                skip |= LogError("VUID-VkShaderCreateInfoEXT-flags-08417", module_state.handle(), loc.dot(Field::flags),
                                 "(%s), but local workgroup size X dimension (%" PRIu32
                                 ") is not a multiple of VkPhysicalDeviceVulkan11Properties::subgroupSize (%" PRIu32 ").",
                                 string_VkPipelineShaderStageCreateFlags(stage_state.shader_object_create_info->flags).c_str(),
                                 local_size.x, phys_dev_props_core11.subgroupSize);
            }
        }
    }
    return skip;
}

bool CoreChecks::ValidateTaskMeshWorkGroupSizes(const spirv::Module& module_state, const spirv::EntryPoint& entrypoint,
                                                const spirv::LocalSize& local_size, const Location& loc) const {
    bool skip = false;

    if (local_size.x == 0) {
        return skip;
    } else if (entrypoint.execution_model != spv::ExecutionModelTaskEXT &&
               entrypoint.execution_model != spv::ExecutionModelMeshEXT) {
        return skip;  // NV version not supported currently
    }

    bool is_task = entrypoint.execution_model == spv::ExecutionModelTaskEXT;
    spirv::LocalSize max_local_size;
    if (is_task) {
        max_local_size.x = phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupSize[0];
        max_local_size.y = phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupSize[1];
        max_local_size.z = phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupSize[2];
    } else {
        max_local_size.x = phys_dev_ext_props.mesh_shader_props_ext.maxMeshWorkGroupSize[0];
        max_local_size.y = phys_dev_ext_props.mesh_shader_props_ext.maxMeshWorkGroupSize[1];
        max_local_size.z = phys_dev_ext_props.mesh_shader_props_ext.maxMeshWorkGroupSize[2];
    }

    if (local_size.x > max_local_size.x) {
        const char* vuid = is_task ? "VUID-RuntimeSpirv-TaskEXT-07291" : "VUID-RuntimeSpirv-MeshEXT-07295";
        skip |= LogError(vuid, module_state.handle(), loc,
                         "shader %s local workgroup size X dimension (%" PRIu32
                         ") must be less than or equal to the max workgroup size (%" PRIu32 ").",
                         entrypoint.Describe().c_str(), local_size.x, max_local_size.x);
    }

    if (local_size.y > max_local_size.y) {
        const char* vuid = is_task ? "VUID-RuntimeSpirv-TaskEXT-07292" : "VUID-RuntimeSpirv-MeshEXT-07296";
        skip |= LogError(vuid, module_state.handle(), loc,
                         "shader %s local workgroup size Y dimension (%" PRIu32
                         ") must be less than or equal to the max workgroup size (%" PRIu32 ").",
                         entrypoint.Describe().c_str(), local_size.y, max_local_size.y);
    }

    if (local_size.z > max_local_size.z) {
        const char* vuid = is_task ? "VUID-RuntimeSpirv-TaskEXT-07293" : "VUID-RuntimeSpirv-MeshEXT-07297";
        skip |= LogError(vuid, module_state.handle(), loc,
                         "shader %s local workgroup size Z dimension (%" PRIu32
                         ") must be less than or equal to the max workgroup size (%" PRIu32 ").",
                         entrypoint.Describe().c_str(), local_size.z, max_local_size.z);
    }

    uint64_t invocations = static_cast<uint64_t>(local_size.x) * static_cast<uint64_t>(local_size.y);
    // Prevent overflow.
    bool fail = false;
    const uint32_t max_workgroup_size = is_task ? phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupInvocations
                                                : phys_dev_ext_props.mesh_shader_props_ext.maxMeshWorkGroupInvocations;
    if (invocations > vvl::kU32Max || invocations > max_workgroup_size) {
        fail = true;
    }
    if (!fail) {
        invocations *= local_size.z;
        if (invocations > vvl::kU32Max || invocations > max_workgroup_size) {
            fail = true;
        }
    }
    if (fail) {
        const char* vuid = is_task ? "VUID-RuntimeSpirv-TaskEXT-07294" : "VUID-RuntimeSpirv-MeshEXT-07298";
        skip |= LogError(vuid, module_state.handle(), loc,
                         "shader %s total invocation size of %" PRIu64
                         " (%s) must be less than or equal to max workgroup invocations (%" PRIu32 ").",
                         entrypoint.Describe().c_str(), invocations, local_size.ToString().c_str(), max_workgroup_size);
    }
    return skip;
}

bool CoreChecks::ValidateTaskShaderLimits(const spirv::Module& module_state, const spirv::EntryPoint& entrypoint,
                                          uint32_t total_workgroup_shared_memory, const Location& loc) const {
    bool skip = false;

    for (const spirv::Instruction* insn : module_state.static_data_.emit_mesh_tasks_inst) {
        uint32_t x, y, z;
        bool found_x = module_state.GetInt32IfConstant(*module_state.FindDef(insn->Word(1)), &x);
        bool found_y = module_state.GetInt32IfConstant(*module_state.FindDef(insn->Word(2)), &y);
        bool found_z = module_state.GetInt32IfConstant(*module_state.FindDef(insn->Word(3)), &z);
        if (found_x && x > phys_dev_ext_props.mesh_shader_props_ext.maxMeshWorkGroupCount[0]) {
            skip |= LogError("VUID-RuntimeSpirv-TaskEXT-07299", module_state.handle(), loc,
                             "shader %s is emitting %" PRIu32
                             " mesh work groups in X dimension, which is greater than max mesh "
                             "workgroup count (%" PRIu32 ").",
                             entrypoint.Describe().c_str(), x, phys_dev_ext_props.mesh_shader_props_ext.maxMeshWorkGroupCount[0]);
        }
        if (found_y && y > phys_dev_ext_props.mesh_shader_props_ext.maxMeshWorkGroupCount[1]) {
            skip |= LogError("VUID-RuntimeSpirv-TaskEXT-07300", module_state.handle(), loc,
                             "shader %s is emitting %" PRIu32
                             " mesh work groups in Y dimension, which is greater than max mesh "
                             "workgroup count (%" PRIu32 ").",
                             entrypoint.Describe().c_str(), y, phys_dev_ext_props.mesh_shader_props_ext.maxMeshWorkGroupCount[1]);
        }
        if (found_z && z > phys_dev_ext_props.mesh_shader_props_ext.maxMeshWorkGroupCount[2]) {
            skip |= LogError("VUID-RuntimeSpirv-TaskEXT-07301", module_state.handle(), loc,
                             "shader %s is emitting %" PRIu32
                             " mesh work groups in Z dimension, which is greater than max mesh "
                             "workgroup count (%" PRIu32 ").",
                             entrypoint.Describe().c_str(), z, phys_dev_ext_props.mesh_shader_props_ext.maxMeshWorkGroupCount[2]);
        }
        if (found_x && found_y && found_z) {
            uint64_t invocations = static_cast<uint64_t>(x) * static_cast<uint64_t>(y);
            // Prevent overflow.
            bool fail = false;
            if (invocations > phys_dev_ext_props.mesh_shader_props_ext.maxMeshWorkGroupTotalCount) {
                fail = true;
            }
            if (!fail) {
                invocations *= z;
                if (invocations > vvl::kU32Max ||
                    invocations > phys_dev_ext_props.mesh_shader_props_ext.maxMeshWorkGroupTotalCount) {
                    fail = true;
                }
            }
            if (fail) {
                skip |= LogError("VUID-RuntimeSpirv-TaskEXT-07302", module_state.handle(), loc,
                                 "shader %s is emitting %" PRIu32 " x %" PRIu32 " x %" PRIu32 " mesh work groups (total %" PRIu32
                                 "), which is greater than max mesh "
                                 "workgroup total count (%" PRIu32 ").",
                                 entrypoint.Describe().c_str(), x, y, z, x * y * z,
                                 phys_dev_ext_props.mesh_shader_props_ext.maxMeshWorkGroupTotalCount);
            }
        }
    }

    if (total_workgroup_shared_memory > phys_dev_ext_props.mesh_shader_props_ext.maxTaskSharedMemorySize) {
        skip |=
            LogError("VUID-RuntimeSpirv-maxTaskSharedMemorySize-08759", module_state.handle(), loc,
                     "shader %s uses %" PRIu32 " bytes of shared memory, which is more than maxTaskSharedMemorySize (%" PRIu32 ").",
                     entrypoint.Describe().c_str(), total_workgroup_shared_memory,
                     phys_dev_ext_props.mesh_shader_props_ext.maxTaskSharedMemorySize);
    }

    const uint32_t total_task_payload_memory = entrypoint.task_payload_variable ? entrypoint.task_payload_variable->size : 0;
    if (total_task_payload_memory + total_workgroup_shared_memory >
        phys_dev_ext_props.mesh_shader_props_ext.maxTaskPayloadAndSharedMemorySize) {
        skip |= LogError("VUID-RuntimeSpirv-maxTaskPayloadAndSharedMemorySize-08760", module_state.handle(), loc,
                         "shader %s uses %" PRIu32 " bytes of task payload memory and %" PRIu32
                         " bytes of shared memory (combined %" PRIu32
                         " bytes), which is more than maxTaskPayloadAndSharedMemorySize (%" PRIu32 ").",
                         entrypoint.Describe().c_str(), total_task_payload_memory, total_workgroup_shared_memory,
                         total_task_payload_memory + total_workgroup_shared_memory,
                         phys_dev_ext_props.mesh_shader_props_ext.maxTaskPayloadAndSharedMemorySize);
    }
    if (total_task_payload_memory > phys_dev_ext_props.mesh_shader_props_ext.maxTaskPayloadSize) {
        skip |= LogError("VUID-RuntimeSpirv-maxTaskPayloadSize-08758", module_state.handle(), loc,
                         "shader %s uses %" PRIu32 " bytes of task payload memory, which is more than maxTaskPayloadSize (%" PRIu32
                         ").",
                         entrypoint.Describe().c_str(), total_workgroup_shared_memory,
                         phys_dev_ext_props.mesh_shader_props_ext.maxTaskPayloadSize);
    }

    return skip;
}

bool CoreChecks::ValidateMeshShaderLimits(const spirv::Module& module_state, const spirv::EntryPoint& entrypoint,
                                          uint32_t total_workgroup_shared_memory, const Location& loc) const {
    bool skip = false;

    if (total_workgroup_shared_memory > phys_dev_ext_props.mesh_shader_props_ext.maxMeshSharedMemorySize) {
        skip |=
            LogError("VUID-RuntimeSpirv-maxMeshSharedMemorySize-08754", module_state.handle(), loc,
                     "shader %s uses %" PRIu32 " bytes of shared memory, which is more than maxMeshSharedMemorySize (%" PRIu32 ").",
                     entrypoint.Describe().c_str(), total_workgroup_shared_memory,
                     phys_dev_ext_props.mesh_shader_props_ext.maxMeshSharedMemorySize);
    }

    const uint32_t total_task_payload_memory = entrypoint.task_payload_variable ? entrypoint.task_payload_variable->size : 0;
    if (total_task_payload_memory + total_workgroup_shared_memory >
        phys_dev_ext_props.mesh_shader_props_ext.maxMeshPayloadAndSharedMemorySize) {
        // If task payload memory size is 0 and only shared memory is already over the limit then the more appropriate VUID 08754
        // was already reported
        if (total_task_payload_memory > 0) {
            skip |= LogError("VUID-RuntimeSpirv-maxMeshPayloadAndSharedMemorySize-08755", module_state.handle(), loc,
                             "shader %s uses %" PRIu32 " bytes of task payload memory and %" PRIu32
                             " bytes of shared memory (combined %" PRIu32
                             " bytes), which is more than maxMeshPayloadAndSharedMemorySize (%" PRIu32 ").",
                             entrypoint.Describe().c_str(), total_task_payload_memory, total_workgroup_shared_memory,
                             total_task_payload_memory + total_workgroup_shared_memory,
                             phys_dev_ext_props.mesh_shader_props_ext.maxMeshPayloadAndSharedMemorySize);
        }
    }

    return skip;
}

static const ShaderStageState* GetDataGraphShaderStage(const vvl::Pipeline& pipeline) {
    if (pipeline.stage_states.empty()) {
        // no ShaderModule defined
        return nullptr;
    }
    // the one and only stage for a datagraph
    return &pipeline.stage_states[0];
}

bool CoreChecks::ValidateDataGraphPipelineShaderModuleSpirv(VkDevice device, const VkDataGraphPipelineCreateInfoARM& create_info,
                                                            const Location& create_info_loc,
                                                            const VkDataGraphPipelineShaderModuleCreateInfoARM& dg_shader_ci,
                                                            const vvl::Pipeline& pipeline) const {
    bool skip = false;

    const ShaderStageState* stage_state_ptr = GetDataGraphShaderStage(pipeline);
    if (!stage_state_ptr) {
        return skip;
    }
    const ShaderStageState& stage_state = *stage_state_ptr;
    const spirv::Module* module_spirv_ptr = stage_state.spirv_state.get();
    if (!module_spirv_ptr) {
        return skip;
    }
    const spirv::Module& module_spirv = *module_spirv_ptr;

    const Location dg_shader_ci_loc = create_info_loc.pNext(Struct::VkDataGraphPipelineShaderModuleCreateInfoARM);

    // location where the VkShaderModule was defined: VkDataGraphPipelineShaderModuleCreateInfoARM or VkShaderModuleCreateInfo?
    const Location module_loc =
        (dg_shader_ci.module) ? dg_shader_ci_loc.dot(Field::module) : create_info_loc.pNext(Struct::VkShaderModuleCreateInfo);

    if (!enabled_features.dataGraphSpecializationConstants && module_spirv.static_data_.has_specialization_constants) {
        skip |=
            LogError("VUID-VkDataGraphPipelineShaderModuleCreateInfoARM-dataGraphSpecializationConstants-09849", device, module_loc,
                     "contains OpSpec* instruction(s), but the dataGraphSpecializationConstants feature is not enabled.");
    }

    std::shared_ptr<spirv::EntryPoint> entry_point = nullptr;
    for (auto& ep : module_spirv.static_data_.entry_points) {
        if (!ep->is_data_graph) {
            continue;
        }

        if (ep->name.compare(dg_shader_ci.pName) == 0) {
            entry_point = ep;
            break;
        }
    }

    if (!entry_point) {
        std::ostringstream wrong_names;
        for (const auto& ep : module_spirv.static_data_.entry_points) {
            if (!wrong_names.str().empty()) {
                wrong_names << ", ";
            }
            wrong_names << ep->name;
        }
        skip |= LogError("VUID-VkDataGraphPipelineShaderModuleCreateInfoARM-pName-09872", device,
                         dg_shader_ci_loc.dot(Field::pName), " is '%s' but names in OpGraphEntryPointARM instructions are: '%s'",
                         dg_shader_ci.pName, wrong_names.str().c_str());

        // from here on we must have the correct entrypoint
        return skip;
    }

    skip |= ValidateDataGraphResourceVariables(module_spirv, *entry_point, stage_state, create_info, create_info_loc, module_loc);
    skip |= ValidateDataGraphConstants(module_spirv, *entry_point, dg_shader_ci, dg_shader_ci_loc, module_loc);

    return skip;
}

// Check consistency of datagraph variables between spirv and vulkan. First we match spirv -> vulkan, then vulkan -> spirv
bool CoreChecks::ValidateDataGraphResourceVariables(const spirv::Module& module_spirv, const spirv::EntryPoint& entry_point,
                                                    const ShaderStageState& stage_state,
                                                    const VkDataGraphPipelineCreateInfoARM& create_info,
                                                    const Location& create_info_loc, const Location& module_loc) const {
    bool skip = false;

    // loop over spirv resource definitions
    std::vector<bool> pResource_matched(create_info.resourceInfoCount, false);
    for (const spirv::ResourceInterfaceVariable& variable : entry_point.resource_interface_variables) {
        // layout checks are the same as for shader resources
        skip |= ValidateShaderInterfaceVariableDSL(module_spirv, entry_point, stage_state, variable, module_loc);

        if (!variable.is_storage_tensor) {
            continue;
        }

        const spirv::Instruction& tensor_type_instr = variable.base_type;

        // input/output tensors must have rank and shape, i.e. exactly 5 words
        if (tensor_type_instr.Length() < 5) {
            skip |= LogError("VUID-RuntimeSpirv-pNext-09919", module_spirv.handle(), module_loc,
                             "'%s' defines a tensor without a shape.", tensor_type_instr.Describe().c_str());
        }

        // MUST match 1 and only 1 element of pResourceInfos in the pipeline create_info
        std::vector<uint32_t> vk_indexes;
        for (uint32_t j = 0; j < create_info.resourceInfoCount; j++) {
            const VkDataGraphPipelineResourceInfoARM& resource = create_info.pResourceInfos[j];
            if ((resource.descriptorSet == variable.decorations.set) && (resource.binding == variable.decorations.binding)) {
                vk_indexes.push_back(j);
            }
        }
        if (vk_indexes.empty()) {
            skip |= LogError("VUID-RuntimeSpirv-pNext-09923", device, create_info_loc.dot(Field::pResourceInfos),
                             "no element found matching spirv descriptor %s.", variable.DescribeDescriptor().c_str());
        } else if (vk_indexes.size() > 1) {
            std::stringstream matches;
            for (auto i : vk_indexes) {
                matches << (matches.str().empty() ? "" : ", ") << i;
            }
            skip |= LogError("VUID-RuntimeSpirv-pNext-09923", device, create_info_loc.dot(Field::pResourceInfos),
                             "contains %zu elements (at indexes [%s]) that match the spirv descriptor %s, only 1 is allowed.",
                             vk_indexes.size(), matches.str().c_str(), variable.DescribeDescriptor().c_str());
        }

        // NOTE: VU 9923 specifies a 1-to-1 match between spirv and vulkan, because tensor arrays are not allowed. With arrays we
        // have multiple resources with the same (descriptorSet, binding) as the spirv OpVariable, and different arrayElement. At
        // some point we will probably allow arrays, and we already have a couple of tests using them, so here we process all the
        // vk_indexes
        for (auto vk_index : vk_indexes) {
            pResource_matched[vk_index] = true;
            const VkDataGraphPipelineResourceInfoARM& resource = create_info.pResourceInfos[vk_index];
            const Location resource_loc = create_info_loc.dot(Field::pResourceInfos, vk_index);

            // part of VU 9923, this is the specific text in the specs:
            // "its arrayElement member must be zero if OpVariable is not a OpTypeArray or if OpVariable is a OpTypeArray of
            // OpTypeTensorARM with Shape present"
            if (resource.arrayElement != 0) {
                skip |= LogError("VUID-RuntimeSpirv-pNext-09923", device, resource_loc.dot(Field::arrayElement),
                                 "(%" PRIu32 ") is not zero.\n%s", resource.arrayElement, variable.DescribeDescriptor().c_str());
            }

            if (auto* tensor_desc = vku::FindStructInPNextChain<VkTensorDescriptionARM>(resource.pNext)) {
                const spirv::Instruction& element_type_instr = *module_spirv.FindDef(tensor_type_instr.Word(2));
                if (!module_spirv.IsTensorFormatCompatible(tensor_desc->format, element_type_instr)) {
                    skip |= LogError("VUID-RuntimeSpirv-pNext-09923", device,
                                     resource_loc.pNext(Struct::VkTensorDescriptionARM).dot(Field::format),
                                     "(%s) is incompatible with the element type (%s) of the tensor type definition (%s) for spirv "
                                     "descriptor %s.",
                                     string_VkFormat(tensor_desc->format),
                                     module_spirv.DescribeTypeInstruction(element_type_instr).c_str(),
                                     element_type_instr.Describe().c_str(), variable.DescribeDescriptor().c_str());
                }

                const uint32_t spirv_rank = module_spirv.GetConstantValueById(tensor_type_instr.Word(3));
                if (tensor_desc->dimensionCount != spirv_rank) {
                    skip |= LogError("VUID-RuntimeSpirv-pNext-09923", device,
                                     resource_loc.pNext(Struct::VkTensorDescriptionARM).dot(Field::dimensionCount),
                                     "(%" PRIu32 ") doesn't match the rank (%" PRIu32
                                     ") of the tensor type definition (%s) for spirv descriptor %s.",
                                     tensor_desc->dimensionCount, spirv_rank, tensor_type_instr.Describe().c_str(),
                                     variable.DescribeDescriptor().c_str());
                    continue;
                }

                // nothing to check here if the tensor has no shape
                if (tensor_type_instr.Length() > 4) {
                    const spirv::Instruction& shape_instr = *module_spirv.FindDef(tensor_type_instr.Word(4));
                    const uint32_t max_dim = std::min(tensor_desc->dimensionCount, spirv_rank);
                    for (uint32_t i = 0; i < max_dim; i++) {
                        const uint32_t spirv_dim_i = module_spirv.GetConstantValueById(shape_instr.Word(3 + i));
                        if (tensor_desc->pDimensions[i] != spirv_dim_i) {
                            skip |= LogError("VUID-RuntimeSpirv-pNext-09923", device,
                                             resource_loc.pNext(Struct::VkTensorDescriptionARM).dot(Field::pDimensions, i),
                                             "(%" PRIi64 ") doesn't match the value (%" PRIu32
                                             ") of the corresponding element in the spirv definition (%s)",
                                             tensor_desc->pDimensions[i], spirv_dim_i, tensor_type_instr.Describe().c_str());
                        }
                    }
                }
            } else {  // missing VkTensorDescriptionARM
                skip |= LogError("VUID-RuntimeSpirv-pNext-09923", device, resource_loc,
                                 "does not include a VkTensorDescriptionARM in its pNext chain.\n%s",
                                 PrintPNextChain(Struct::VkDataGraphPipelineResourceInfoARM, resource.pNext).c_str());
            }
        }
    }

    // loop over Vulkan resource declarations
    for (uint32_t j = 0; j < create_info.resourceInfoCount; j++) {
        const VkDataGraphPipelineResourceInfoARM& resource = create_info.pResourceInfos[j];
        const Location resource_loc = create_info_loc.dot(Field::pResourceInfos, j);

        if (!pResource_matched[j]) {
            skip |=
                LogError("VUID-RuntimeSpirv-pNext-09923", device, resource_loc, "%s has no matching OpVariable in the module spirv",
                         string_VkDataGraphPipelineResourceInfoARM(resource).c_str());
        } else {
            if (auto* tensor_desc = vku::FindStructInPNextChain<VkTensorDescriptionARM>(resource.pNext)) {
                if ((tensor_desc->usage & VK_TENSOR_USAGE_DATA_GRAPH_BIT_ARM) == 0) {
                    skip |=
                        LogError("VUID-VkDataGraphPipelineResourceInfoARM-descriptorSet-09851", device,
                                 resource_loc.pNext(Struct::VkTensorDescriptionARM).dot(Field::usage),
                                 "(%s) invalid for tensor resource %s", string_VkTensorUsageFlagsARM(tensor_desc->usage).c_str(),
                                 string_VkDataGraphPipelineResourceInfoARM(resource).c_str());
                }
            }
        }
    }

    return skip;
}

// Check consistency of datagraph constants between spirv and vulkan. First we match spirv -> vulkan, then vulkan -> spirv
bool CoreChecks::ValidateDataGraphConstants(const spirv::Module& module_spirv, const spirv::EntryPoint& entry_point,
                                            const VkDataGraphPipelineShaderModuleCreateInfoARM& dg_shader_ci,
                                            const Location& dg_shader_ci_loc, const Location& module_loc) const {
    bool skip = false;

    // loop over spirv constant definitions
    std::vector<bool> pConstant_matched(dg_shader_ci.constantCount, false);
    for (auto constant_instr : entry_point.accessible.graph_constant) {
        const spirv::Instruction& tensor_type_instr = *module_spirv.FindDef(constant_instr->TypeId());

        // the following checks are only for tensors. Any type other than tensor will throw an error executing spirv-val, which
        // results in VU 8737 in the VVL
        if (!tensor_type_instr.IsTensor()) {
            continue;
        }

        // if the constant type is a tensor, it MUST have a shape, i.e. exactly 5 words long
        if (tensor_type_instr.Length() < 5) {
            skip |= LogError("VUID-RuntimeSpirv-pNext-09920", module_spirv.handle(), module_loc,
                             "the type of '%s' is a tensor without a shape: '%s'.", constant_instr->Describe().c_str(),
                             tensor_type_instr.Describe().c_str());
        }

        // MUST match 1 and only 1 element of pConstants in the shader module create info
        const uint32_t graph_constant_id = constant_instr->Word(3);
        std::vector<uint32_t> vk_indexes;
        for (uint32_t j = 0; j < dg_shader_ci.constantCount; j++) {
            const VkDataGraphPipelineConstantARM& vk_constant = dg_shader_ci.pConstants[j];
            if (vk_constant.id == graph_constant_id) {
                vk_indexes.push_back(j);
            }
        }
        if (vk_indexes.empty()) {
            skip |= LogError("VUID-RuntimeSpirv-pNext-09921", device, dg_shader_ci_loc.dot(Field::pConstants),
                             "none of the elements has the same id (%" PRIu32 ") of the spirv definition (%s)", graph_constant_id,
                             constant_instr->Describe().c_str());
        } else if (vk_indexes.size() > 1) {
            std::stringstream matches;
            for (auto i : vk_indexes) {
                matches << (matches.str().empty() ? "" : ", ") << i;
            }
            skip |= LogError("VUID-RuntimeSpirv-pNext-09921", device, dg_shader_ci_loc.dot(Field::pConstants),
                             "%zu elements at indexes [%s] found with the same id (%" PRIu32 ") of the spirv definition (%s).",
                             vk_indexes.size(), matches.str().c_str(), graph_constant_id, constant_instr->Describe().c_str());
        } else {
            // found the one and only match
            uint32_t vk_index = vk_indexes[0];
            pConstant_matched[vk_index] = true;
            const VkDataGraphPipelineConstantARM& vk_constant = dg_shader_ci.pConstants[vk_index];
            const Location vk_constant_loc = dg_shader_ci_loc.dot(Field::pConstants, vk_index);
            if (auto* tensor_desc = vku::FindStructInPNextChain<VkTensorDescriptionARM>(vk_constant.pNext)) {
                const spirv::Instruction& element_type_instr = *module_spirv.FindDef(tensor_type_instr.Word(2));
                if (!module_spirv.IsTensorFormatCompatible(tensor_desc->format, element_type_instr)) {
                    skip |= LogError("VUID-RuntimeSpirv-pNext-09921", device,
                                     vk_constant_loc.pNext(Struct::VkTensorDescriptionARM).dot(Field::format),
                                     "(%s) is incompatible with the element type (%s) of the spirv definition (%s)",
                                     string_VkFormat(tensor_desc->format),
                                     module_spirv.DescribeTypeInstruction(element_type_instr).c_str(),
                                     element_type_instr.Describe().c_str());
                }

                const uint32_t spirv_rank = module_spirv.GetConstantValueById(tensor_type_instr.Word(3));
                if (tensor_desc->dimensionCount != spirv_rank) {
                    skip |= LogError("VUID-RuntimeSpirv-pNext-09921", device,
                                     vk_constant_loc.pNext(Struct::VkTensorDescriptionARM).dot(Field::dimensionCount),
                                     "(%" PRIu32 ") doesn't match the rank (%" PRIu32 ") of the spirv definition (%s)",
                                     tensor_desc->dimensionCount, spirv_rank, tensor_type_instr.Describe().c_str());
                    continue;
                }

                // nothing to check here if the tensor has no shape, and we have already failed VU 9920 anyway.
                if (tensor_type_instr.Length() > 4) {
                    const spirv::Instruction* shape_instr = module_spirv.FindDef(tensor_type_instr.Word(4));
                    const uint32_t max_dim = std::min(tensor_desc->dimensionCount, spirv_rank);
                    for (uint32_t i = 0; i < max_dim; i++) {
                        const uint32_t spirv_dim_i = module_spirv.GetConstantValueById(shape_instr->Word(3 + i));
                        if (tensor_desc->pDimensions[i] != spirv_dim_i) {
                            skip |= LogError("VUID-RuntimeSpirv-pNext-09921", device,
                                             vk_constant_loc.pNext(Struct::VkTensorDescriptionARM).dot(Field::pDimensions, i),
                                             "(%" PRIi64 ") doesn't match the value (%" PRIu32
                                             ") of the corresponding element in the spirv definition (%s)",
                                             tensor_desc->pDimensions[i], spirv_dim_i, tensor_type_instr.Describe().c_str());
                        }
                    }
                }
            } else {  // missing VkTensorDescriptionARM
                skip |= LogError("VUID-RuntimeSpirv-pNext-09921", device, vk_constant_loc,
                                 "does not include a VkTensorDescriptionARM in its pNext chain.\n%s",
                                 PrintPNextChain(Struct::VkDataGraphPipelineConstantARM, vk_constant.pNext).c_str());
            }
        }
    }

    // loop over Vulkan constant declarations
    for (uint32_t i = 0; i < dg_shader_ci.constantCount; i++) {
        const VkDataGraphPipelineConstantARM& constant = dg_shader_ci.pConstants[i];
        const Location constant_loc = dg_shader_ci_loc.dot(Field::pConstants, i);
        if (!pConstant_matched[i]) {
            skip |= LogError("VUID-RuntimeSpirv-pNext-09921", device, constant_loc.dot(Field::id),
                             "(%" PRIu32 ") does not match the id of any of the OpGraphConstantARM instructions in module",
                             constant.id);
        } else {
            if (auto* tensor_desc = vku::FindStructInPNextChain<VkTensorDescriptionARM>(constant.pNext)) {
                if ((tensor_desc->usage & VK_TENSOR_USAGE_DATA_GRAPH_BIT_ARM) == 0) {
                    skip |= LogError(
                        "VUID-VkDataGraphPipelineConstantARM-id-09850", device, constant_loc.dot(Field::id),
                        "(%" PRIu32
                        ") is a graph constant of tensor type but its matching VkTensorDescriptionARM has an invalid usage (%s)",
                        constant.id, string_VkTensorUsageFlagsARM(tensor_desc->usage).c_str());
                }
                if (tensor_desc->tiling != VK_TENSOR_TILING_LINEAR_ARM) {
                    skip |= LogError(
                        "VUID-VkDataGraphPipelineConstantARM-pNext-09917", device, constant_loc.dot(Field::tiling),
                        "(%" PRIu32
                        ") is a graph constant of tensor type but its matching VkTensorDescriptionARM has an invalid tiling (%s)",
                        constant.id, string_VkTensorTilingARM(tensor_desc->tiling));
                }
            }
        }
    }

    return skip;
}

// It is not listed in the spec, but each vendor decides which engine/operations are required.
// TOSA 1.0 is the current valid instruction set for the ARM engine
const VkQueueFamilyDataGraphPropertiesARM tosa_1_0_property{
    VK_STRUCTURE_TYPE_QUEUE_FAMILY_DATA_GRAPH_PROPERTIES_ARM,
    nullptr,
    {VK_PHYSICAL_DEVICE_DATA_GRAPH_PROCESSING_ENGINE_TYPE_DEFAULT_ARM, false},
    {VK_PHYSICAL_DEVICE_DATA_GRAPH_OPERATION_TYPE_SPIRV_EXTENDED_INSTRUCTION_SET_ARM, "TOSA.001000.1", 0}};

bool CoreChecks::ValidateDataGraphOperations(const vvl::Pipeline& pipeline, uint32_t queueFamilyIndex, const Location& loc) const {
    bool skip = false;

    const ShaderStageState* stage_state_ptr = GetDataGraphShaderStage(pipeline);
    if (!stage_state_ptr) {
        return skip;
    }
    const spirv::EntryPoint* entry_point = stage_state_ptr->entrypoint.get();
    if (!entry_point) {
        return skip;
    }

    // this checks only the ARM operation type, which requires TOSA 1.0, not any of the other operation types (QCOM).

    if (!entry_point->uses_tosa_1_0) {
        return skip;
    }

    bool queue_uses_tosa_1_0 = false;
    // no properties for the specified family index means NO support for TOSA 1.0, i.e. VUID 9941
    // this also covers completely empty properties, meaning VK_ARM_data_graph extension not supported
    const auto properties_it = physical_device_state->data_graph.queue_family_properties.find(queueFamilyIndex);
    if (properties_it != physical_device_state->data_graph.queue_family_properties.end()) {
        for (const auto& p : properties_it->second) {
            if (CompareVkQueueFamilyDataGraphPropertiesARM(tosa_1_0_property, p)) {
                queue_uses_tosa_1_0 = true;
                break;
            }
        }
    }

    if (!queue_uses_tosa_1_0) {
        std::ostringstream ss;
        ss << "Entrypoint " << entry_point->name
           << "includes \"TOSA.001000.1\" instructions but the queue associated with the command buffer does not include the "
              "required property:\n"
           << string_VkQueueFamilyDataGraphPropertiesARM(tosa_1_0_property)
           << "vkGetPhysicalDeviceQueueFamilyDataGraphPropertiesARM for queueFamilyIndex " << queueFamilyIndex << "returned:\n";
        if (properties_it != physical_device_state->data_graph.queue_family_properties.end() && !properties_it->second.empty()) {
            for (const auto& p : properties_it->second) {
                ss << string_VkQueueFamilyDataGraphPropertiesARM(p) << '\n';
            }
        } else {
            ss << "EMPTY\n";
        }
        skip |= LogError("VUID-vkCmdDispatchDataGraphARM-commandBuffer-09941", device, loc, "%s", ss.str().c_str());
    }

    return skip;
}

// VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_*
bool CoreChecks::ValidateDescriptorMappingSourceHeap(const spirv::Module& module_state, const spirv::EntryPoint& entrypoint,
                                                     const spirv::ResourceInterfaceVariable& resource_variable,
                                                     const VkDescriptorSetAndBindingMappingEXT& mapping,
                                                     const Location& mapping_loc) const {
    bool skip = false;
    const uint32_t base_opcode = resource_variable.base_type.Opcode();

    const bool has_embedded_sampler = GetEmbeddedSampler(mapping) != nullptr;

    struct MappingSourceInfo {
        uint32_t offset = 0;
        uint32_t array_stride = 0;
        const char* vuid = nullptr;
        VkDeviceSize align = 0;
        Field align_field = Field::Empty;
    } r_info;  // resource info

    if (base_opcode == spv::OpTypeSampledImage) {
        r_info.vuid = "VUID-VkDescriptorSetAndBindingMappingEXT-source-12406";
        r_info.align = phys_dev_ext_props.descriptor_heap_props.imageDescriptorAlignment;
        r_info.align_field = Field::imageDescriptorAlignment;
    } else if (base_opcode == spv::OpTypeImage) {
        r_info.vuid = "VUID-VkDescriptorSetAndBindingMappingEXT-source-11251";
        r_info.align = phys_dev_ext_props.descriptor_heap_props.imageDescriptorAlignment;
        r_info.align_field = Field::imageDescriptorAlignment;
    } else if (base_opcode == spv::OpTypeStruct) {
        r_info.vuid = "VUID-VkDescriptorSetAndBindingMappingEXT-source-11252";
        r_info.align = phys_dev_ext_props.descriptor_heap_props.bufferDescriptorAlignment;
        r_info.align_field = Field::bufferDescriptorAlignment;
    } else if (base_opcode == spv::OpTypeSampler) {
        if (has_embedded_sampler) {
            return skip;
        }
        r_info.vuid = "VUID-VkDescriptorSetAndBindingMappingEXT-source-11253";
        r_info.align = phys_dev_ext_props.descriptor_heap_props.samplerDescriptorAlignment;
        r_info.align_field = Field::samplerDescriptorAlignment;
    } else if (base_opcode == spv::OpTypeTensorARM) {
        r_info.vuid = "VUID-VkDescriptorSetAndBindingMappingEXT-source-11390";
        r_info.align = phys_dev_ext_props.descriptor_heap_tensor_props.tensorDescriptorAlignment;
        r_info.align_field = Field::tensorDescriptorAlignment;
    } else {
        return skip;
    }

    if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT) {
        const auto& source_data = mapping.sourceData.constantOffset;
        r_info.offset = source_data.heapOffset;
        r_info.array_stride = source_data.heapArrayStride;
    } else if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_PUSH_INDEX_EXT) {
        const auto& source_data = mapping.sourceData.pushIndex;
        r_info.offset = source_data.heapOffset;
        r_info.array_stride = source_data.heapArrayStride;
    } else if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_EXT) {
        const auto& source_data = mapping.sourceData.indirectIndex;
        r_info.offset = source_data.heapOffset;
        r_info.array_stride = source_data.heapArrayStride;
    } else if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_ARRAY_EXT) {
        const auto& source_data = mapping.sourceData.indirectIndexArray;
        r_info.offset = source_data.heapOffset;
    } else if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_SHADER_RECORD_INDEX_EXT) {
        const auto& source_data = mapping.sourceData.shaderRecordIndex;
        r_info.offset = source_data.heapOffset;
        r_info.array_stride = source_data.heapArrayStride;
    }

    if (mapping.source != VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_ARRAY_EXT && resource_variable.IsArray() &&
        r_info.array_stride == 0) {
        // Tried to ban in https://gitlab.khronos.org/vulkan/vulkan/-/issues/4815
        // but we wrote tests, so this is allowed and will work, just high chance not what people intend to do.
        skip |= LogWarning("WARNING-VkDescriptorSetAndBindingMappingEXT-heapArrayStride-zero", module_state.handle(),
                           mapping_loc.dot(Field::source),
                           "(%s) is used to map descriptor %s in %s which is a descriptor array, but heapArrayStride is zero. This "
                           "mean every index of the descriptor array will be the same descriptor, which is likely not desired.",
                           string_VkDescriptorMappingSourceEXT(mapping.source), resource_variable.DescribeDescriptor().c_str(),
                           entrypoint.Describe().c_str());
    }

    if (!IsIntegerMultipleOf(r_info.offset, r_info.align) || !IsIntegerMultipleOf(r_info.array_stride, r_info.align)) {
        const Field source_field = vvl::Field_VkDescriptorMappingSourceDataEXT(mapping.source);

        std::stringstream ss;
        ss << "(" << string_VkDescriptorMappingSourceEXT(mapping.source) << ") is used to map descriptor "
           << resource_variable.DescribeDescriptor() << " in " << entrypoint.Describe() << " but it is unaligned.\n"
           << String(source_field) << ".heapOffset (" << r_info.offset << ") ";
        if (mapping.source != VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_ARRAY_EXT) {
            ss << "and " << String(source_field) << ".heapArrayStride (" << r_info.array_stride << ") both ";
        }
        ss << "must be zero or aligned with " << String(r_info.align_field) << " (" << r_info.align << ")";

        skip |= LogError(r_info.vuid, module_state.handle(), mapping_loc.dot(Field::source), "%s", ss.str().c_str());
    }

    // Combined Image Sampler is only spot we need to check twice
    if (base_opcode == spv::OpTypeSampledImage) {
        struct SamplerMappingSourceInfo {
            uint32_t offset = 0;
            uint32_t array_stride = 0;
            uint32_t push_offset = 0;
            uint32_t address_offset = 0;
        } s_info;

        if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT) {
            const auto& source_data = mapping.sourceData.constantOffset;
            s_info.offset = source_data.samplerHeapOffset;
            s_info.array_stride = source_data.samplerHeapArrayStride;
        } else if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_PUSH_INDEX_EXT) {
            const auto& source_data = mapping.sourceData.pushIndex;
            s_info.offset = source_data.samplerHeapOffset;
            s_info.array_stride = source_data.samplerHeapArrayStride;
            // We don't use the pushOffset here as that is already validated in 11258 (and related VUs)
            // and we don't want to spam the user with a duplicate error message
            s_info.push_offset = source_data.useCombinedImageSamplerIndex ? 0 : source_data.samplerPushOffset;
        } else if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_EXT) {
            const auto& source_data = mapping.sourceData.indirectIndex;
            s_info.offset = source_data.samplerHeapOffset;
            s_info.array_stride = source_data.samplerHeapArrayStride;
            if (!source_data.useCombinedImageSamplerIndex) {
                s_info.push_offset = source_data.samplerPushOffset;
                s_info.address_offset = source_data.samplerAddressOffset;
            }
        } else if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_ARRAY_EXT) {
            const auto& source_data = mapping.sourceData.indirectIndexArray;
            s_info.offset = source_data.samplerHeapOffset;
            if (!source_data.useCombinedImageSamplerIndex) {
                s_info.push_offset = source_data.samplerPushOffset;
                s_info.address_offset = source_data.samplerAddressOffset;
            }
        } else if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_SHADER_RECORD_INDEX_EXT) {
            const auto& source_data = mapping.sourceData.shaderRecordIndex;
            s_info.offset = source_data.samplerHeapOffset;
            s_info.array_stride = source_data.samplerHeapArrayStride;
        }

        if (!has_embedded_sampler) {
            const VkDeviceSize sampler_align = phys_dev_ext_props.descriptor_heap_props.samplerDescriptorAlignment;
            if (!IsIntegerMultipleOf(s_info.offset, sampler_align) || !IsIntegerMultipleOf(s_info.array_stride, sampler_align)) {
                const Field source_field = vvl::Field_VkDescriptorMappingSourceDataEXT(mapping.source);

                std::stringstream ss;
                ss << "(" << string_VkDescriptorMappingSourceEXT(mapping.source) << ") is used to map descriptor "
                   << resource_variable.DescribeDescriptor() << " in " << entrypoint.Describe() << " but it is unaligned.\n"
                   << String(source_field) << ".samplerHeapOffset (" << s_info.offset << ") ";
                if (mapping.source != VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_ARRAY_EXT) {
                    ss << "and " << String(source_field) << ".samplerHeapArrayStride (" << s_info.array_stride << ") both ";
                }
                ss << "must be zero or aligned with samplerDescriptorAlignment (" << sampler_align << ")";

                skip |= LogError("VUID-VkDescriptorSetAndBindingMappingEXT-source-11254", module_state.handle(),
                                 mapping_loc.dot(Field::source), "%s", ss.str().c_str());
            }

            if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_PUSH_INDEX_EXT) {
                if (!IsIntegerMultipleOf(s_info.push_offset, 4)) {
                    skip |= LogError("VUID-VkDescriptorSetAndBindingMappingEXT-source-12456", device,
                                     mapping_loc.dot(Field::sourceData).dot(Field::pushIndex).dot(Field::samplerPushOffset),
                                     "(%" PRIu32
                                     ") is not a multiple of 4\nVkDescriptorSetAndBindingMappingEXT::source = "
                                     "VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_PUSH_INDEX_EXT",
                                     s_info.push_offset);
                }
                if (s_info.push_offset > phys_dev_ext_props.descriptor_heap_props.maxPushDataSize - 4) {
                    skip |= LogError("VUID-VkDescriptorSetAndBindingMappingEXT-source-12457", device,
                                     mapping_loc.dot(Field::sourceData).dot(Field::pushIndex).dot(Field::samplerPushOffset),
                                     "(%" PRIu32 ") is greater than maxPushDataSize (%" PRIu64
                                     ") - 4\nVkDescriptorSetAndBindingMappingEXT::source = "
                                     "VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_PUSH_INDEX_EXT\nHint - samplerPushOffset points to an "
                                     "uint32_t (4 bytes) "
                                     "inside the push data, this is currently going to access OOB.",
                                     s_info.push_offset, phys_dev_ext_props.descriptor_heap_props.maxPushDataSize);
                }
            } else if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_EXT ||
                       mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_ARRAY_EXT) {
                if (!IsIntegerMultipleOf(s_info.push_offset, 8)) {
                    skip |= LogError("VUID-VkDescriptorSetAndBindingMappingEXT-source-12458", device,
                                     mapping_loc.dot(Field::sourceData)
                                         .dot(vvl::Field_VkDescriptorMappingSourceDataEXT(mapping.source))
                                         .dot(Field::samplerPushOffset),
                                     "(%" PRIu32 ") is not a multiple of 8\nVkDescriptorSetAndBindingMappingEXT::source = %s",
                                     s_info.push_offset, string_VkDescriptorMappingSourceEXT(mapping.source));
                }
                if (s_info.push_offset > phys_dev_ext_props.descriptor_heap_props.maxPushDataSize - 8) {
                    skip |= LogError("VUID-VkDescriptorSetAndBindingMappingEXT-source-12459", device,
                                     mapping_loc.dot(Field::sourceData)
                                         .dot(vvl::Field_VkDescriptorMappingSourceDataEXT(mapping.source))
                                         .dot(Field::samplerPushOffset),
                                     "(%" PRIu32 ") is greater than maxPushDataSize (%" PRIu64
                                     ") - 8\nVkDescriptorSetAndBindingMappingEXT::source = %s\nHint - samplerPushOffset points to "
                                     "an address (8 bytes) "
                                     "inside the push data, this is currently going to access OOB.",
                                     s_info.push_offset, phys_dev_ext_props.descriptor_heap_props.maxPushDataSize,
                                     string_VkDescriptorMappingSourceEXT(mapping.source));
                }
                if (!IsIntegerMultipleOf(s_info.address_offset, 4)) {
                    skip |= LogError("VUID-VkDescriptorSetAndBindingMappingEXT-source-12460", device,
                                     mapping_loc.dot(Field::sourceData)
                                         .dot(vvl::Field_VkDescriptorMappingSourceDataEXT(mapping.source))
                                         .dot(Field::samplerAddressOffset),
                                     "(%" PRIu32 ") is not a multiple of 4\nVkDescriptorSetAndBindingMappingEXT::source = %s",
                                     s_info.address_offset, string_VkDescriptorMappingSourceEXT(mapping.source));
                }
            }
        }
    } else if (base_opcode == spv::OpTypeSampler) {
        // It is VERY easy to think samplerHeapOffset is for mapping to the sampler heap, but that is wrong, it is only
        // for combined image samplers... so try and guide people as likely a mistake.
        if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT) {
            const auto& source_data = mapping.sourceData.constantOffset;
            if (source_data.heapOffset == 0 && source_data.heapArrayStride == 0 &&
                (source_data.samplerHeapOffset != 0 || source_data.samplerHeapArrayStride != 0)) {
                skip |= LogError("WARNING-VkDescriptorSetAndBindingMappingEXT-constantOffset-sampler", module_state.handle(),
                                 mapping_loc.dot(Field::source),
                                 "(%s) is used to map descriptor %s in %s which is a sampler, but seems like you are "
                                 "setting samplerHeapOffset/samplerHeapArrayStride instead of "
                                 "heapOffset/heapArrayStride.\nThe samplerHeapOffset field is there to map the sampler "
                                 "portion of a VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, but for "
                                 "VK_DESCRIPTOR_TYPE_SAMPLER, you just use heapOffset to offset into the sampler heap.",
                                 string_VkDescriptorMappingSourceEXT(mapping.source),
                                 resource_variable.DescribeDescriptor().c_str(), entrypoint.Describe().c_str());
            }
        } else if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_PUSH_INDEX_EXT) {
            const auto& source_data = mapping.sourceData.pushIndex;
            const bool zero_heap = source_data.heapOffset == 0 && source_data.pushOffset == 0 && source_data.heapArrayStride == 0 &&
                                   source_data.heapIndexStride == 0;
            const bool non_zero_sampler = !source_data.pEmbeddedSampler &&
                                          (source_data.samplerHeapOffset != 0 || source_data.samplerPushOffset != 0 ||
                                           source_data.samplerHeapArrayStride != 0 || source_data.samplerHeapIndexStride != 0);
            if (zero_heap && non_zero_sampler) {
                skip |= LogError("WARNING-VkDescriptorSetAndBindingMappingEXT-pushIndex-sampler", module_state.handle(),
                                 mapping_loc.dot(Field::source),
                                 "(%s) is used to map descriptor %s in %s which is a sampler, but seems like you are setting "
                                 "samplerHeapOffset/samplerPushOffset/samplerHeapArrayStride/samplerHeapIndexStride instead of "
                                 "heapOffset/pushOffset/heapArrayStride/heapIndexStride.\nThe samplerHeapOffset field is there to "
                                 "map the sampler portion of a VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, but for "
                                 "VK_DESCRIPTOR_TYPE_SAMPLER, you just use heapOffset to offset into the sampler heap.",
                                 string_VkDescriptorMappingSourceEXT(mapping.source),
                                 resource_variable.DescribeDescriptor().c_str(), entrypoint.Describe().c_str());
            }
        } else if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_EXT) {
            const auto& source_data = mapping.sourceData.indirectIndex;
            const bool zero_heap = source_data.heapOffset == 0 && source_data.pushOffset == 0 && source_data.addressOffset == 0 &&
                                   source_data.heapArrayStride == 0 && source_data.heapIndexStride == 0;
            const bool non_zero_heap =
                !source_data.pEmbeddedSampler & (source_data.samplerHeapOffset != 0 || source_data.samplerPushOffset != 0 ||
                                                 source_data.samplerAddressOffset != 0 || source_data.samplerHeapArrayStride != 0 ||
                                                 source_data.samplerHeapIndexStride != 0);
            if (zero_heap && non_zero_heap) {
                skip |=
                    LogError("WARNING-VkDescriptorSetAndBindingMappingEXT-indirectIndex-sampler", module_state.handle(),
                             mapping_loc.dot(Field::source),
                             "(%s) is used to map descriptor %s in %s which is a sampler, but seems like you are setting "
                             "samplerHeapOffset/samplerPushOffset/samplerAddressOffset/samplerHeapArrayStride/"
                             "samplerHeapIndexStride instead of "
                             "heapOffset/pushOffset/addressOffset/heapArrayStride/heapIndexStride.\nThe samplerHeapOffset field "
                             "is there to map the sampler portion of a VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, but for "
                             "VK_DESCRIPTOR_TYPE_SAMPLER, you just use heapOffset to offset into the sampler heap.",
                             string_VkDescriptorMappingSourceEXT(mapping.source), resource_variable.DescribeDescriptor().c_str(),
                             entrypoint.Describe().c_str());
            }
        } else if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_ARRAY_EXT) {
            const auto& source_data = mapping.sourceData.indirectIndexArray;
            const bool zero_heap = source_data.heapOffset == 0 && source_data.pushOffset == 0 && source_data.addressOffset == 0 &&
                                   source_data.heapIndexStride == 0;
            const bool non_zero_heap =
                !source_data.pEmbeddedSampler && (source_data.samplerHeapOffset != 0 || source_data.samplerPushOffset != 0 ||
                                                  source_data.samplerAddressOffset != 0 || source_data.samplerHeapIndexStride != 0);
            if (zero_heap && non_zero_heap) {
                skip |=
                    LogError("WARNING-VkDescriptorSetAndBindingMappingEXT-indirectIndexArray-sampler", module_state.handle(),
                             mapping_loc.dot(Field::source),
                             "(%s) is used to map descriptor %s in %s which is a sampler, but seems like you are setting "
                             "samplerHeapOffset/samplerPushOffset/samplerAddressOffset/samplerHeapIndexStride instead of "
                             "heapOffset/pushOffset/addressOffset/heapIndexStride.\nThe samplerHeapOffset field is there to map "
                             "the sampler portion of a VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, but for "
                             "VK_DESCRIPTOR_TYPE_SAMPLER, you just use heapOffset to offset into the sampler heap.",
                             string_VkDescriptorMappingSourceEXT(mapping.source), resource_variable.DescribeDescriptor().c_str(),
                             entrypoint.Describe().c_str());
            }
        }
    }

    return skip;
}

bool CoreChecks::ValidateShaderDescriptorSetAndBindingMappingInfo(const spirv::Module& module_state,
                                                                  const spirv::EntryPoint& entrypoint,
                                                                  const vvl::Pipeline* pipeline,
                                                                  const ShaderStageState& stage_state, const Location& loc) const {
    bool skip = false;
    const auto* mapping_info = vku::FindStructInPNextChain<VkShaderDescriptorSetAndBindingMappingInfoEXT>(stage_state.GetPNext());

    // If there is no VkShaderDescriptorSetAndBindingMappingInfoEXT, but the heap flags is used, we need to still ensure all the
    // resource variables are untyped (not using set/binding)
    if (!mapping_info) {
        // If not flag, this is just a "normal" vulkan 1.0 situtation
        if (stage_state.heap.descriptor_heap_mode) {
            for (const spirv::ResourceInterfaceVariable& resource_variable : entrypoint.resource_interface_variables) {
                if (!resource_variable.IsHeap()) {
                    skip |= LogError(
                        vvl::GetSpirvInterfaceVariableVUID(loc, vvl::SpirvInterfaceVariableError::DescriptorHeapMapping_11312),
                        module_state.handle(), loc,
                        "does not have a pNext to VkShaderDescriptorSetAndBindingMappingInfoEXT, but %s is set and %s needs a "
                        "mapping for shader %s.\n%s\nHint: Either pass in a valid VkShaderDescriptorSetAndBindingMappingInfoEXT or "
                        "use "
                        "SPV_EXT_descriptor_heap to replace the traditional Set/Binding SPIR-V mapping. (SPV_EXT_descriptor_heap "
                        "requires re-writing your shader and each shading language, if it is supported, will have a dedicated way "
                        "to set it)",
                        pipeline ? "VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT" : "VK_SHADER_CREATE_DESCRIPTOR_HEAP_BIT_EXT",
                        resource_variable.DescribeDescriptor().c_str(), entrypoint.Describe().c_str(),
                        PrintPNextChain(Struct::Empty, stage_state.GetPNext()).c_str());
                    break;  // only need to report once
                }
            }
        }
        return skip;  // rest of checks require actual mapping
    } else if (!stage_state.heap.descriptor_heap_mode && mapping_info->mappingCount > 0) {
        // If they are here, the pipeline layout would also have to be non-null
        // Provide a warning here incase people are trying to go from normal descriptor to heap and don't realize their mappings are
        // ignored
        skip |= LogWarning("WARNING-VkShaderDescriptorSetAndBindingMappingInfoEXT-ignored", module_state.handle(),
                           loc.dot(Field::pNext),
                           "contains a VkShaderDescriptorSetAndBindingMappingInfoEXT with mappings, but %s is not set and the "
                           "VkDescriptorSetLayout will be read instead.",
                           pipeline ? "VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT" : "VK_SHADER_CREATE_DESCRIPTOR_HEAP_BIT_EXT");
        return skip;
    }

    std::vector<bool> used_mapping_set(mapping_info->mappingCount, false);
    std::unordered_set<const spirv::ResourceInterfaceVariable*> unmapped_variables;

    for (const spirv::ResourceInterfaceVariable& resource_variable : entrypoint.resource_interface_variables) {
        if (resource_variable.IsHeap()) {
            continue;
        }

        bool found_mapping = false;
        for (uint32_t i = 0; i < mapping_info->mappingCount; i++) {
            const VkDescriptorSetAndBindingMappingEXT& mapping = mapping_info->pMappings[i];
            if (!IsResourceVaribleInMapping(mapping, resource_variable)) {
                continue;
            }
            used_mapping_set[i] = true;
            found_mapping = true;

            const Location mapping_loc = loc.pNext(Struct::VkShaderDescriptorSetAndBindingMappingInfoEXT, Field::pMappings, i);

            const std::shared_ptr<const spirv::TypeStructInfo>& type_struct_info = resource_variable.type_struct_info;
            if (IsValueIn(mapping.source, {VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT,
                                           VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_PUSH_INDEX_EXT,
                                           VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_EXT,
                                           VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_ARRAY_EXT,
                                           VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_SHADER_RECORD_INDEX_EXT})) {
                skip |= ValidateDescriptorMappingSourceHeap(module_state, entrypoint, resource_variable, mapping, mapping_loc);
            } else if (IsValueIn(mapping.source,
                                 {VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_DATA_EXT, VK_DESCRIPTOR_MAPPING_SOURCE_SHADER_RECORD_DATA_EXT,
                                  VK_DESCRIPTOR_MAPPING_SOURCE_RESOURCE_HEAP_DATA_EXT})) {
                if (!resource_variable.is_uniform_buffer) {
                    const char* vuid =
                        pipeline ? "VUID-VkPipelineShaderStageCreateInfo-pNext-11315" : "VUID-VkShaderCreateInfoEXT-pNext-11315";
                    const VkDescriptorType potential_descriptor_type = resource_variable.GetPotentialDescriptorType();
                    std::ostringstream ss;
                    ss << "(" << string_VkDescriptorMappingSourceEXT(mapping.source) << ") is used to map descriptor "
                       << resource_variable.DescribeDescriptor() << " in " << entrypoint.Describe() << " with StorageClass "
                       << string_SpvStorageClass(resource_variable.storage_class);
                    // Prevent more confusion with the cases
                    if (potential_descriptor_type != VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER && !resource_variable.is_buffer_block) {
                        ss << " (likely " << string_VkDescriptorType(potential_descriptor_type) << ")";
                    }
                    if (resource_variable.is_buffer_block) {
                        ss << ", but it is decorated with BufferBlock (which is the Vulkan 1.0 way to turn the Uniform "
                              "StorageClass into a Storage Buffer)";
                    } else {
                        ss << ", but it must be a Uniform Buffer (StorageClass Uniform) when using this mapping source as the "
                              "descriptor is read only.\nHint: Did "
                              "you mean to use VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_PUSH_INDEX_EXT instead?";
                    }
                    skip |= LogError(vuid, module_state.handle(), mapping_loc.dot(Field::source), "%s", ss.str().c_str());
                } else if (resource_variable.IsArray()) {
                    // Additional message for descriptor array case
                    const char* vuid =
                        pipeline ? "VUID-VkPipelineShaderStageCreateInfo-pNext-11315" : "VUID-VkShaderCreateInfoEXT-pNext-11315";
                    skip |= LogError(
                        vuid, module_state.handle(), mapping_loc.dot(Field::source),
                        "(%s) is used to map descriptor %s in %s with storage class Uniform, but it is an array.\n"
                        "Descriptor arrays are not allowed for this mapping as it is not defined where each index would get the "
                        "descriptor from.%s",
                        string_VkDescriptorMappingSourceEXT(mapping.source), resource_variable.DescribeDescriptor().c_str(),
                        entrypoint.Describe().c_str(),
                        resource_variable.array_length == 1 ? "(array of length 1 is also not allowed)" : "");
                }
            }

            // If there is a runtime array, we can't detect statically, but should be handled in some GPU-AV check
            if (type_struct_info && !type_struct_info->has_runtime_array) {
                if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_DATA_EXT) {
                    const uint64_t struct_size = (uint64_t)type_struct_info->GetSize(module_state).size;
                    if (struct_size >
                        (uint64_t)(phys_dev_ext_props.descriptor_heap_props.maxPushDataSize - mapping.sourceData.pushDataOffset)) {
                        const char* vuid = pipeline ? "VUID-VkPipelineShaderStageCreateInfo-pNext-11316"
                                                    : "VUID-VkShaderCreateInfoEXT-pNext-11316";
                        skip |=
                            LogError(vuid, module_state.handle(), mapping_loc.dot(Field::source),
                                     "(VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_DATA_EXT) is used to map descriptor %s in %s which has a "
                                     "structure size of %" PRIu64 ", which when the pushDataOffset (%" PRIu32
                                     ") is applied, will be larger than the maxPushDataSize (%" PRIu64 ").",
                                     resource_variable.DescribeDescriptor().c_str(), entrypoint.Describe().c_str(), struct_size,
                                     mapping.sourceData.pushDataOffset, phys_dev_ext_props.descriptor_heap_props.maxPushDataSize);
                    }
                } else if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_SHADER_RECORD_DATA_EXT) {
                    const uint32_t struct_size = type_struct_info ? type_struct_info->GetSize(module_state).size : 0;
                    if (mapping.sourceData.shaderRecordDataOffset + struct_size >
                        phys_dev_ext_props.ray_tracing_props_khr.maxShaderGroupStride) {
                        const char* vuid = pipeline ? "VUID-VkPipelineShaderStageCreateInfo-pNext-11317"
                                                    : "VUID-VkShaderCreateInfoEXT-pNext-11317";
                        skip |= LogError(
                            vuid, module_state.handle(), mapping_loc.dot(Field::source),
                            "(VK_DESCRIPTOR_MAPPING_SOURCE_SHADER_RECORD_DATA_EXT) is used to map descriptor %s in %s which has a "
                            "structure "
                            "size of %" PRIu32 ", which summed with shaderRecordDataOffset (%" PRIu32
                            ") is larger than maxShaderGroupStride (%" PRIu32 ").",
                            resource_variable.DescribeDescriptor().c_str(), entrypoint.Describe().c_str(), struct_size,
                            mapping.sourceData.shaderRecordDataOffset,
                            phys_dev_ext_props.ray_tracing_props_khr.maxShaderGroupStride);
                    }
                }
            }

            if (IsValueIn(mapping.source,
                          {VK_DESCRIPTOR_MAPPING_SOURCE_SHADER_RECORD_ADDRESS_EXT, VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_ADDRESS_EXT,
                           VK_DESCRIPTOR_MAPPING_SOURCE_INDIRECT_ADDRESS_EXT})) {
                if (!resource_variable.is_uniform_buffer && !resource_variable.is_storage_buffer &&
                    !resource_variable.is_acceleration_structure && !resource_variable.is_acceleration_structure_nv) {
                    const char* vuid =
                        pipeline ? "VUID-VkPipelineShaderStageCreateInfo-pNext-11318" : "VUID-VkShaderCreateInfoEXT-pNext-11318";

                    std::ostringstream ss;
                    ss << "(" << string_VkDescriptorMappingSourceEXT(mapping.source) << ") is used to map descriptor "
                       << resource_variable.DescribeDescriptor() << " in " << entrypoint.Describe() << " with StorageClass "
                       << string_SpvStorageClass(resource_variable.storage_class) << " (likely "
                       << string_VkDescriptorType(resource_variable.GetPotentialDescriptorType())
                       << ") but it must be a Uniform Buffer, Storage Buffer, or Acceleration Structure when using this mapping "
                          "source.\nHint: Did you mean to use VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_PUSH_INDEX_EXT instead?";
                    skip |= LogError(vuid, module_state.handle(), mapping_loc.dot(Field::source), "%s", ss.str().c_str());
                } else if (resource_variable.IsArray()) {
                    // Additional message for descriptor array case
                    const char* vuid =
                        pipeline ? "VUID-VkPipelineShaderStageCreateInfo-pNext-11318" : "VUID-VkShaderCreateInfoEXT-pNext-11318";

                    std::ostringstream ss;
                    ss << "(" << string_VkDescriptorMappingSourceEXT(mapping.source) << ") is used to map descriptor "
                       << resource_variable.DescribeDescriptor() << " in " << entrypoint.Describe() << " with StorageClass "
                       << string_SpvStorageClass(resource_variable.storage_class) << " (likely "
                       << string_VkDescriptorType(resource_variable.GetPotentialDescriptorType())
                       << ") but it is an array.\nDescriptor arrays are not allowed for this mapping as it is not defined where "
                          "each index would get the descriptor from.";
                    if (resource_variable.array_length == 1) {
                        ss << " (array of length 1 is also not allowed)";
                    }
                    ss << "\nHint: Did you mean to use VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_PUSH_INDEX_EXT instead?";
                    skip |= LogError(vuid, module_state.handle(), mapping_loc.dot(Field::source), "%s", ss.str().c_str());
                }
            }

            if (IsValueIn(mapping.source, {VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT,
                                           VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_PUSH_INDEX_EXT,
                                           VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_SHADER_RECORD_INDEX_EXT,
                                           VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_EXT,
                                           VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_ARRAY_EXT})) {
                const VkSamplerCreateInfo* embedded_sampler = GetEmbeddedSampler(mapping);
                if (resource_variable.IsArray() && embedded_sampler != nullptr) {
                    skip |= LogError(
                        pipeline ? "VUID-VkPipelineShaderStageCreateInfo-pNext-11399" : "VUID-VkShaderCreateInfoEXT-pNext-11399",
                        module_state.handle(), mapping_loc.dot(Field::source),
                        "(%s) is used to map to an array of descriptors %s in %s, but %s.pEmbeddedSampler is %p (not null)",
                        string_VkDescriptorMappingSourceEXT(mapping.source), resource_variable.DescribeDescriptor().c_str(),
                        entrypoint.Describe().c_str(), String(vvl::Field_VkDescriptorMappingSourceDataEXT(mapping.source)),
                        embedded_sampler);
                }
            }

            if (IsValueIn(mapping.source,
                          {VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_ADDRESS_EXT, VK_DESCRIPTOR_MAPPING_SOURCE_SHADER_RECORD_ADDRESS_EXT,
                           VK_DESCRIPTOR_MAPPING_SOURCE_INDIRECT_ADDRESS_EXT})) {
                const spirv::Instruction* found_inst = nullptr;
                const uint32_t desc_type = module_state.FindDef(resource_variable.type_id)->ResultId();
                for (const spirv::Instruction* array_length_inst : module_state.static_data_.array_length_inst) {
                    const spirv::Instruction* type = module_state.FindDef(array_length_inst->Word(3));
                    if (type->Opcode() == spv::OpVariable && type->Word(1) == desc_type) {
                        found_inst = array_length_inst;
                        break;
                    }
                }
                if (found_inst) {
                    const char* vuid =
                        pipeline ? "VUID-VkPipelineShaderStageCreateInfo-pNext-11378" : "VUID-VkShaderCreateInfoEXT-pNext-11378";
                    skip |= LogError(vuid, module_state.handle(), mapping_loc.dot(Field::source),
                                     "(%s) is used to map to descriptor %s in %s but the %s was used to access the length\n%s",
                                     string_VkDescriptorMappingSourceEXT(mapping.source),
                                     resource_variable.DescribeDescriptor().c_str(), entrypoint.Describe().c_str(),
                                     string_SpvOpcode(found_inst->Opcode()), found_inst->Describe().c_str());
                }
            }

            if (IsValueIn(
                    mapping.source,
                    {VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_DATA_EXT, VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_ADDRESS_EXT,
                     VK_DESCRIPTOR_MAPPING_SOURCE_SHADER_RECORD_DATA_EXT, VK_DESCRIPTOR_MAPPING_SOURCE_SHADER_RECORD_ADDRESS_EXT,
                     VK_DESCRIPTOR_MAPPING_SOURCE_INDIRECT_ADDRESS_EXT, VK_DESCRIPTOR_MAPPING_SOURCE_RESOURCE_HEAP_DATA_EXT})) {
                if (!resource_variable.all_constant_integral_expressions) {
                    std::stringstream msg;
                    if (resource_variable.non_constant_id != 0) {
                        // We know this is a OpAccessChain, because of how all_constant_integral_expressions is determined
                        const spirv::Instruction* pointer = module_state.FindDef(resource_variable.non_constant_id);
                        const spirv::Instruction* base = module_state.FindDef(pointer->Word(3));
                        for (uint32_t j = 4; j < pointer->Length(); ++j) {
                            const spirv::Instruction* access_op = module_state.FindDef(pointer->Word(j));
                            if (!IsValueIn((spv::Op)access_op->Opcode(),
                                           {spv::OpConstant, spv::OpSpecConstant, spv::OpConstantComposite})) {
                                // TODO - Currently a bit aimed towards GLSL and need a general util to help with this
                                msg << "\nback trace of instructions:\n";
                                msg << "  " << module_state.DescribeInstruction(*base) << "\n";
                                msg << "  " << module_state.DescribeInstruction(*access_op) << "\n";
                                msg << "  " << module_state.DescribeInstruction(*pointer) << "\n";
                                break;
                            }
                        }
                    }
                    skip |= LogError(
                        "VUID-RuntimeSpirv-DescriptorSet-11385", module_state.handle(), mapping_loc.dot(Field::source),
                        "(%s) is used to map to descriptor %s in %s which is accessed with a non-constant expression (it isn't "
                        "allowed "
                        "to dynamically index)%s",
                        string_VkDescriptorMappingSourceEXT(mapping.source), resource_variable.DescribeDescriptor().c_str(),
                        entrypoint.Describe().c_str(), msg.str().c_str());
                }
            }

            if (IsValueIn(mapping.source, {VK_DESCRIPTOR_MAPPING_SOURCE_SHADER_RECORD_DATA_EXT,
                                           VK_DESCRIPTOR_MAPPING_SOURCE_SHADER_RECORD_ADDRESS_EXT,
                                           VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_SHADER_RECORD_INDEX_EXT})) {
                if ((stage_state.GetStage() & kShaderStageAllRayTracing) == 0) {
                    const char* vuid =
                        pipeline ? "VUID-VkPipelineShaderStageCreateInfo-pNext-12454" : "VUID-VkShaderCreateInfoEXT-pNext-12454";
                    skip |= LogError(vuid, module_state.handle(), mapping_loc.dot(Field::source),
                                     "(%s) is used to map to descriptor %s in %s, but the stage is not a ray tracing stage. "
                                     "(Shader record are only for ray tracing workflows)",
                                     string_VkDescriptorMappingSourceEXT(mapping.source),
                                     resource_variable.DescribeDescriptor().c_str(), entrypoint.Describe().c_str());
                }
            }
        }

        if (!found_mapping) {
            unmapped_variables.insert(&resource_variable);
        }
    }

    // This error message logic is complex, but this check is complex as we want to provide the user with the most helpful message
    // to why their mappings are invalid
    if (!unmapped_variables.empty()) {
        // Currently only report the first variable, likely will be spam if trying to print them all
        const spirv::ResourceInterfaceVariable& resource_variable = **unmapped_variables.begin();
        std::stringstream ss;
        ss << "has no mapping for " << resource_variable.DescribeDescriptor() << " in " << entrypoint.Describe() << " but "
           << (pipeline ? "VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT" : "VK_SHADER_CREATE_DESCRIPTOR_HEAP_BIT_EXT")
           << " is set.\n";

        const size_t unused_count = mapping_info->mappingCount - std::count(used_mapping_set.begin(), used_mapping_set.end(), true);
        if (unused_count == 0) {
            ss << "All " << mapping_info->mappingCount
               << " pMappings[] were used for another variable already, likely the mapping for this variable is just missing";
        } else if (unused_count > 6) {
            // Real apps can easily have hundreds of mappings, if we have more than a few to print, just provide the indexes instead
            ss << "The following indexes into pMappings[] were not used: ";
            bool first = true;
            for (uint32_t i = 0; i < mapping_info->mappingCount; i++) {
                if (!used_mapping_set[i]) {
                    if (!first) {
                        ss << ", ";
                    }
                    ss << i;
                    first = false;
                }
            }
        } else {
            // Hopefuly people just have a few mixed up, provide the whole error detail here
            ss << "The following mappings where not used:\n";
            for (uint32_t i = 0; i < mapping_info->mappingCount; i++) {
                if (used_mapping_set[i]) {
                    continue;
                }
                const auto& mapping = mapping_info->pMappings[i];
                ss << " - pMappings[" << i << "]: descriptorSet (" << mapping.descriptorSet << "), firstBinding ("
                   << mapping.firstBinding << "), bindingCount (" << mapping.bindingCount << "), resourceMask ("
                   << string_VkSpirvResourceTypeFlagsEXT(mapping.resourceMask) << ")\n\t| not valid because ";
                if (mapping.descriptorSet != resource_variable.decorations.set) {
                    ss << "descriptorSet doesn't match the SPIR-V set (" << resource_variable.decorations.set << ")\n";
                } else if (resource_variable.decorations.binding < mapping.firstBinding) {
                    ss << "firstBinding is greater than the SPIR-V binding (" << resource_variable.decorations.binding << ")\n";
                } else if (resource_variable.decorations.binding >= mapping.firstBinding + uint64_t(mapping.bindingCount)) {
                    ss << "firstBinding + bindingCount does not include SPIR-V binding (" << resource_variable.decorations.binding
                       << ")\n";
                } else if (!ResourceTypeMatchesBinding(mapping.resourceMask, resource_variable)) {
                    ss << "resourceMask doesn't match: " << DescribeResourceTypeMismatch(mapping.resourceMask, resource_variable)
                       << "\n";
                } else {
                    ss << "[UNKNOWN]\n";
                }
            }
        }
        skip |= LogError(vvl::GetSpirvInterfaceVariableVUID(loc, vvl::SpirvInterfaceVariableError::DescriptorHeapMapping_11312),
                         module_state.handle(), loc.pNext(Struct::VkShaderDescriptorSetAndBindingMappingInfoEXT, Field::pMappings),
                         "%s", ss.str().c_str());
    }

    return skip;
}

// Done here instead of stateless because we need deal with spec constants
bool CoreChecks::ValidateDescriptorHeapStructs(const spirv::Module& module_state, const spirv::EntryPoint& entrypoint,
                                               const Location& loc) const {
    bool skip = false;
    // These checks are only things done with the untyped pointer workflow of Descriptor Heap,
    // so skip if they are using the mapping API instead
    if (!module_state.HasCapability(spv::CapabilityUntypedPointersKHR)) {
        return skip;
    }

    const VkPhysicalDeviceDescriptorHeapPropertiesEXT& props = phys_dev_ext_props.descriptor_heap_props;

    // There are to ways to set the offset with decorations
    //  - classic Offset
    //  - using new OffsetIdEXT, which is designed to be used with a spec constant
    for (const auto& type_struct : module_state.static_data_.type_structs) {
        if (!type_struct->has_descriptor_type) {
            continue;  // way to skip skip majority of structs
        }
        // Even if there is a struct inside member, we don't need to go into it, it will be it's own iteration inside
        // |static_data_.type_structs|
        for (uint32_t i = 0; i < type_struct->members.size(); i++) {
            const spirv::TypeStructInfo::Member& member = type_struct->members[i];
            if (!member.insn->IsDescriptorType()) {
                continue;
            }

            // If using the old, hardcoded Offset
            uint32_t offset_value = member.decorations->GetOffset(module_state, props);
            ASSERT_AND_CONTINUE(offset_value != spirv::kInvalidValue);

            const spv::Op opcode = (spv::Op)member.insn->Opcode();
            if (opcode == spv::OpTypeSampler) {
                if (!IsIntegerMultipleOf(offset_value, phys_dev_ext_props.descriptor_heap_props.samplerDescriptorAlignment)) {
                    skip |= LogError("VUID-RuntimeSpirv-samplerDescriptorAlignment-11476", module_state.handle(), loc,
                                     "shader %s has a struct (ID %" PRIu32 ") where member %" PRIu32
                                     " is an OpTypeSampler with an offset of %" PRIu32
                                     " which is not aligned with samplerDescriptorAlignment (%" PRIu64 ")",
                                     entrypoint.Describe().c_str(), type_struct->id, i, offset_value,
                                     phys_dev_ext_props.descriptor_heap_props.samplerDescriptorAlignment);
                }
            } else if (opcode == spv::OpTypeImage) {
                if (!IsIntegerMultipleOf(offset_value, phys_dev_ext_props.descriptor_heap_props.imageDescriptorAlignment)) {
                    skip |= LogError("VUID-RuntimeSpirv-imageDescriptorAlignment-11477", module_state.handle(), loc,
                                     "shader %s has a struct (ID %" PRIu32 ") where member %" PRIu32
                                     " is an OpTypeImage with an offset of %" PRIu32
                                     " which is not aligned with imageDescriptorAlignment (%" PRIu64 ")",
                                     entrypoint.Describe().c_str(), type_struct->id, i, offset_value,
                                     phys_dev_ext_props.descriptor_heap_props.imageDescriptorAlignment);
                }
            } else if (opcode == spv::OpTypeBufferEXT) {
                if (!IsIntegerMultipleOf(offset_value, phys_dev_ext_props.descriptor_heap_props.bufferDescriptorAlignment)) {
                    skip |= LogError("VUID-RuntimeSpirv-bufferDescriptorAlignment-11478", module_state.handle(), loc,
                                     "shader %s has a struct (ID %" PRIu32 ") where member %" PRIu32
                                     " is an OpTypeBufferEXT with an offset of %" PRIu32
                                     " which is not aligned with bufferDescriptorAlignment (%" PRIu64 ")",
                                     entrypoint.Describe().c_str(), type_struct->id, i, offset_value,
                                     phys_dev_ext_props.descriptor_heap_props.bufferDescriptorAlignment);
                }
            } else if (opcode == spv::OpTypeAccelerationStructureKHR) {
                if (!IsIntegerMultipleOf(offset_value, phys_dev_ext_props.descriptor_heap_props.bufferDescriptorAlignment)) {
                    skip |= LogError("VUID-RuntimeSpirv-bufferDescriptorAlignment-11479", module_state.handle(), loc,
                                     "shader %s has a struct (ID %" PRIu32 ") where member %" PRIu32
                                     " is an OpTypeAccelerationStructureKHR with an offset of %" PRIu32
                                     " which is not aligned with bufferDescriptorAlignment (%" PRIu64 ")",
                                     entrypoint.Describe().c_str(), type_struct->id, i, offset_value,
                                     phys_dev_ext_props.descriptor_heap_props.bufferDescriptorAlignment);
                }
            } else if (opcode == spv::OpTypeTensorARM) {
                if (!IsIntegerMultipleOf(offset_value, phys_dev_ext_props.descriptor_heap_tensor_props.tensorDescriptorAlignment)) {
                    skip |= LogError("VUID-RuntimeSpirv-tensorDescriptorAlignment-11480", module_state.handle(), loc,
                                     "shader %s has a struct (ID %" PRIu32 ") where member %" PRIu32
                                     " is an OpTypeTensorARM with an offset of %" PRIu32
                                     " which is not aligned with tensorDescriptorAlignment (%" PRIu64 ")",
                                     entrypoint.Describe().c_str(), type_struct->id, i, offset_value,
                                     phys_dev_ext_props.descriptor_heap_tensor_props.tensorDescriptorAlignment);
                }
            }
        }
    }

    return skip;
}
