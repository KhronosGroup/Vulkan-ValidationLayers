/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (C) 2015-2025 Google Inc.
 * Copyright (c) 2025 Arm Limited.
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

#include <cassert>
#include <cinttypes>
#include <cstdint>
#include <spirv/unified1/spirv.hpp>
#include <sstream>
#include <string>
#include <vector>

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
#include "utils/shader_utils.h"
#include "utils/hash_util.h"
#include "chassis/chassis_modification_state.h"
#include "state_tracker/descriptor_sets.h"
#include "state_tracker/descriptor_set_layouts.h"
#include "state_tracker/render_pass_state.h"
#include "spirv-tools/optimizer.hpp"
#include "containers/limits.h"
#include "utils/math_utils.h"

// Validate use of input attachments against subpass structure
bool CoreChecks::ValidateShaderInputAttachment(const spirv::Module &module_state, const vvl::Pipeline &pipeline,
                                               const spirv::ResourceInterfaceVariable &variable, const Location &loc) const {
    bool skip = false;
    assert(variable.is_input_attachment);

    const auto &rp_state = pipeline.RenderPassState();
    // Dynamic Rendering guards this with VUID 06061
    if (!rp_state || rp_state->UsesDynamicRendering()) {
        return skip;
    }

    for (uint32_t i = 0; i < variable.input_attachment_index_read.size(); i++) {
        // If the attachment is not read from, nothing to validate
        if (!variable.input_attachment_index_read[i]) {
            continue;
        }
        const auto rpci = rp_state->create_info.ptr();
        const uint32_t subpass = pipeline.Subpass();
        const auto subpass_description = rpci->pSubpasses[subpass];
        const auto input_attachments = subpass_description.pInputAttachments;
        // offsets by the InputAttachmentIndex decoration
        const uint32_t input_attachment_index = variable.decorations.input_attachment_index_start + i;

        // Same error, but provide more useful message 'how' VK_ATTACHMENT_UNUSED is derived
        if (!input_attachments) {
            const LogObjectList objlist(module_state.handle(), rp_state->Handle());
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-renderPass-06038", objlist, loc,
                             "SPIR-V consumes input attachment index %" PRIu32 " but pSubpasses[%" PRIu32
                             "].pInputAttachments is NULL.",
                             input_attachment_index, subpass);
        } else if (input_attachment_index >= subpass_description.inputAttachmentCount) {
            const LogObjectList objlist(module_state.handle(), rp_state->Handle());
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-renderPass-06038", objlist, loc,
                             "SPIR-V consumes input attachment index %" PRIu32 " but that is not less than the pSubpasses[%" PRIu32
                             "].inputAttachmentCount (%" PRIu32 ").",
                             input_attachment_index, subpass, subpass_description.inputAttachmentCount);
        } else if (input_attachments[input_attachment_index].attachment == VK_ATTACHMENT_UNUSED) {
            const LogObjectList objlist(module_state.handle(), rp_state->Handle());
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-renderPass-06038", objlist, loc,
                             "SPIR-V consumes input attachment index %" PRIu32 " but pSubpasses[%" PRIu32
                             "].pInputAttachments[%" PRIu32 "].attachment is VK_ATTACHMENT_UNUSED.",
                             input_attachment_index, subpass, input_attachment_index);
        }
    }

    return skip;
}

bool CoreChecks::ValidatePushConstantUsage(const spirv::Module &module_state, const spirv::EntryPoint &entrypoint,
                                           const vvl::Pipeline *pipeline, const ShaderStageState &stage_state,
                                           const Location &loc) const {
    bool skip = false;

    // TODO - Workaround for https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5911
    if (module_state.static_data_.has_specialization_constants) {
        return skip;
    }

    const VkShaderStageFlagBits stage = entrypoint.stage;
    const auto push_constant_variable = entrypoint.push_constant_variable;
    if (!push_constant_variable) {
        return skip;
    }

    PushConstantRangesId shader_object_push_constant_ranges_id;
    std::vector<VkPushConstantRange> const *push_constant_ranges;
    std::string stage_vuid;
    std::string range_vuid;
    if (pipeline) {
        push_constant_ranges = pipeline->PipelineLayoutState()->push_constant_ranges_layout.get();

        switch (pipeline->GetCreateInfoSType()) {
            case VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO:
                stage_vuid = "VUID-VkGraphicsPipelineCreateInfo-layout-07987";
                range_vuid = "VUID-VkGraphicsPipelineCreateInfo-layout-10069";
                break;
            case VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO:
                stage_vuid = "VUID-VkComputePipelineCreateInfo-layout-07987";
                range_vuid = "VUID-VkComputePipelineCreateInfo-layout-10069";
                break;
            case VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR:
                stage_vuid = "VUID-VkRayTracingPipelineCreateInfoKHR-layout-07987";
                range_vuid = "VUID-VkRayTracingPipelineCreateInfoKHR-layout-10069";
                break;
            case VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV:
                stage_vuid = "VUID-VkRayTracingPipelineCreateInfoNV-layout-07987";
                range_vuid = "VUID-VkRayTracingPipelineCreateInfoNV-layout-10069";
                break;
            default:
                assert(false);
                break;
        }
    } else {
        shader_object_push_constant_ranges_id = GetCanonicalId(stage_state.shader_object_create_info->pushConstantRangeCount,
                                                               stage_state.shader_object_create_info->pPushConstantRanges);
        push_constant_ranges = shader_object_push_constant_ranges_id.get();
        stage_vuid = "VUID-VkShaderCreateInfoEXT-codeType-10064";
        range_vuid = "VUID-VkShaderCreateInfoEXT-codeType-10065";
    }

    bool found_stage = false;
    for (auto const &range : *push_constant_ranges) {
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
                skip |= LogError(range_vuid, objlist, loc,
                                 "SPIR-V (%s) has a push constant buffer Block with range [%" PRIu32 ", %" PRIu32
                                 "] which outside the VkPushConstantRange of [%" PRIu32 ", %" PRIu32 "].",
                                 string_VkShaderStageFlags(stage).c_str(), push_constant_variable->offset, push_constant_end,
                                 range.offset, range_end);
                break;
            }
        }
    }

    if (!found_stage) {
        LogObjectList objlist(module_state.handle());
        std::string msg = "";
        if (pipeline) {
            objlist.add(pipeline->PipelineLayoutState()->Handle());
            msg = FormatHandle(pipeline->PipelineLayoutState()->Handle());
        } else {
            msg = "VkShaderCreateInfoEXT::pPushConstantRanges";
        }
        skip |= LogError(stage_vuid, objlist, loc, "SPIR-V (%s) Push constant are used, but %s doesn't set %s.",
                         string_VkShaderStageFlags(stage).c_str(), msg.c_str(), string_VkShaderStageFlags(stage).c_str());
    }
    return skip;
}

static void TypeToDescriptorTypeSet(const spirv::Module &module_state, uint32_t type_id, uint32_t data_type_id, vvl::unordered_set<uint32_t> &descriptor_type_set) {
    const spirv::Instruction *type = module_state.FindDef(type_id);
    assert(type->Opcode() == spv::OpTypePointer || type->Opcode() == spv::OpTypeUntypedPointerKHR);
    bool is_storage_buffer = type->StorageClass() == spv::StorageClassStorageBuffer;

    if (data_type_id != 0) type = module_state.FindDef(data_type_id);

    // Strip off any array or ptrs. Where we remove array levels, adjust the  descriptor count for each dimension.
    while (type->IsArray() || type->Opcode() == spv::OpTypePointer) {
        if (type->Opcode() == spv::OpTypeRuntimeArray) {
            type = module_state.FindDef(type->Word(2));
        } else if (type->Opcode() == spv::OpTypeArray) {
            type = module_state.FindDef(type->Word(2));
        } else {
            if (type->StorageClass() == spv::StorageClassStorageBuffer) {
                is_storage_buffer = true;
            }
            type = module_state.FindDef(type->Word(3));
        }
    }

    switch (type->Opcode()) {
        case spv::OpTypeStruct: {
            for (const spirv::Instruction *insn : module_state.static_data_.decoration_inst) {
                if (insn->Word(1) == type->ResultId()) {
                    if (insn->Word(2) == spv::DecorationBlock) {
                        if (is_storage_buffer) {
                            descriptor_type_set.insert(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
                            descriptor_type_set.insert(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC);
                        } else {
                            descriptor_type_set.insert(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
                            descriptor_type_set.insert(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC);
                            descriptor_type_set.insert(VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK);
                        }
                    } else if (insn->Word(2) == spv::DecorationBufferBlock) {
                        descriptor_type_set.insert(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
                        descriptor_type_set.insert(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC);
                    }
                    break;
                }
            }
            return;
        }

        case spv::OpTypeSampler:
            descriptor_type_set.insert(VK_DESCRIPTOR_TYPE_SAMPLER);
            descriptor_type_set.insert(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
            return;

        case spv::OpTypeSampledImage: {
            // Slight relaxation for some GLSL historical madness: samplerBuffer doesn't really have a sampler, and a texel
            // buffer descriptor doesn't really provide one. Allow this slight mismatch.
            const spirv::Instruction *image_type = module_state.FindDef(type->Word(2));
            auto dim = image_type->Word(3);
            auto sampled = image_type->Word(7);
            if (dim == spv::DimBuffer && sampled == 1) {
                descriptor_type_set.insert(VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER);
            } else {
                descriptor_type_set.insert(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
            }
            return;
        }
        case spv::OpTypeImage: {
            // Many descriptor types backing image types-- depends on dimension and whether the image will be used with a sampler.
            // SPIRV for Vulkan requires that sampled be 1 or 2 -- leaving the decision to runtime is unacceptable.
            auto dim = type->Word(3);
            auto sampled = type->Word(7);

            if (dim == spv::DimSubpassData) {
                descriptor_type_set.insert(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT);
            } else if (dim == spv::DimBuffer) {
                if (sampled == 1) {
                    descriptor_type_set.insert(VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER);
                } else {
                    descriptor_type_set.insert(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER);
                }
            } else if (sampled == 1) {
                descriptor_type_set.insert(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
                descriptor_type_set.insert(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
            } else {
                descriptor_type_set.insert(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
            }
            return;
        }

        // The OpType are alias, but the Descriptor Types are different
        case spv::OpTypeAccelerationStructureKHR:
            if (module_state.HasCapability(spv::CapabilityRayTracingNV)) {
                descriptor_type_set.insert(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV);
            } else {
                descriptor_type_set.insert(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR);
            }
            return;

        case spv::OpTypeTensorARM: {
            descriptor_type_set.insert(VK_DESCRIPTOR_TYPE_TENSOR_ARM);
            return;
        }

        default:
            // We shouldn't really see any other junk types -- but if we do, they're a mismatch.
            return;  // Matches nothing
    }
}

static std::string string_DescriptorTypeSet(const vvl::unordered_set<uint32_t> &descriptor_type_set) {
    std::stringstream ss;
    for (auto it = descriptor_type_set.begin(); it != descriptor_type_set.end(); ++it) {
        if (ss.tellp()) ss << " or ";
        ss << string_VkDescriptorType(VkDescriptorType(*it));
    }
    return ss.str();
}

// Map SPIR-V type to VK_COMPONENT_TYPE enum
VkComponentTypeKHR GetComponentType(const spirv::Instruction *insn, bool is_signed_int) {
    if (insn->Opcode() == spv::OpTypeInt) {
        switch (insn->Word(2)) {
            case 8:
                return is_signed_int ? VK_COMPONENT_TYPE_SINT8_KHR : VK_COMPONENT_TYPE_UINT8_KHR;
            case 16:
                return is_signed_int ? VK_COMPONENT_TYPE_SINT16_KHR : VK_COMPONENT_TYPE_UINT16_KHR;
            case 32:
                return is_signed_int ? VK_COMPONENT_TYPE_SINT32_KHR : VK_COMPONENT_TYPE_UINT32_KHR;
            case 64:
                return is_signed_int ? VK_COMPONENT_TYPE_SINT64_KHR : VK_COMPONENT_TYPE_UINT64_KHR;
            default:
                return VK_COMPONENT_TYPE_MAX_ENUM_KHR;
        }
    } else if (insn->Opcode() == spv::OpTypeFloat) {
        switch (insn->Word(2)) {
            case 8: {
                assert(insn->Length() > 3);  // all float8 have an encoding
                const uint32_t encoding = insn->Word(3);
                if (encoding == spv::FPEncodingFloat8E4M3EXT) {
                    return VK_COMPONENT_TYPE_FLOAT8_E4M3_EXT;
                } else if (encoding == spv::FPEncodingFloat8E5M2EXT) {
                    return VK_COMPONENT_TYPE_FLOAT8_E5M2_EXT;
                } else {
                    assert(false);  // New float8 encoding
                }
            } break;
            case 16: {
                if (insn->Length() > 3) {
                    const uint32_t encoding = insn->Word(3);
                    if (encoding == spv::FPEncodingBFloat16KHR) {
                        return VK_COMPONENT_TYPE_BFLOAT16_KHR;
                    } else {
                        assert(false);  // New float16 encoding
                    }
                } else {
                    return VK_COMPONENT_TYPE_FLOAT16_KHR;
                }
            } break;
            case 32:
                return VK_COMPONENT_TYPE_FLOAT32_KHR;
            case 64:
                return VK_COMPONENT_TYPE_FLOAT64_KHR;
            default:
                return VK_COMPONENT_TYPE_MAX_ENUM_KHR;
        }
    }
    return VK_COMPONENT_TYPE_MAX_ENUM_KHR;
}

static bool IsSignedIntEnum(const VkComponentTypeKHR component_type) {
    switch (component_type) {
        case VK_COMPONENT_TYPE_SINT8_KHR:
        case VK_COMPONENT_TYPE_SINT16_KHR:
        case VK_COMPONENT_TYPE_SINT32_KHR:
        case VK_COMPONENT_TYPE_SINT64_KHR:
            return true;
        default:
            return false;
    }
}

// Validate SPV_KHR_cooperative_matrix (and SPV_NV_cooperative_matrix) behavior that can't be statically validated in SPIRV-Tools
// (e.g. due to specialization constant usage).
bool CoreChecks::ValidateCooperativeMatrix(const spirv::Module &module_state, const spirv::EntryPoint &entrypoint,
                                           const ShaderStageState &stage_state, const spirv::LocalSize &local_size,
                                           const Location &loc) const {
    bool skip = false;
    const uint64_t workgroup_size = local_size.x * local_size.y * local_size.z;

    uint32_t effective_subgroup_size = phys_dev_props_core11.subgroupSize;
    if (const auto *required_subgroup_size_ci =
            vku::FindStructInPNextChain<VkPipelineShaderStageRequiredSubgroupSizeCreateInfo>(stage_state.GetPNext())) {
        effective_subgroup_size = required_subgroup_size_ci->requiredSubgroupSize;
    }

    const auto &IsSignedIntType = [&module_state](const uint32_t type_id) {
        const spirv::Instruction *type = module_state.FindDef(type_id);
        if (type->Opcode() == spv::OpTypeCooperativeMatrixKHR || type->Opcode() == spv::OpTypeCooperativeMatrixNV) {
            type = module_state.FindDef(type->Word(2));
        }
        return type->Opcode() == spv::OpTypeInt && type->Word(3) != 0;
    };

    struct CoopMatType {
        VkScopeKHR scope;
        uint32_t rows;
        uint32_t cols;
        VkComponentTypeKHR component_type;
        uint32_t use;
        bool all_constant;

        CoopMatType(uint32_t id, const spirv::Module &module_state, const ShaderStageState &stage_state, bool is_signed_int) {
            const spirv::Instruction *insn = module_state.FindDef(id);
            const spirv::Instruction *component_type_insn = module_state.FindDef(insn->Word(2));
            const spirv::Instruction *scope_insn = module_state.FindDef(insn->Word(3));
            const spirv::Instruction *rows_insn = module_state.FindDef(insn->Word(4));
            const spirv::Instruction *cols_insn = module_state.FindDef(insn->Word(5));

            all_constant = true;
            uint32_t tmp_scope = 0;  // TODO - Remove GetIntConstantValue
            if (!stage_state.GetInt32ConstantValue(*scope_insn, &tmp_scope)) {
                all_constant = false;
            }
            scope = VkScopeKHR(tmp_scope);
            if (!stage_state.GetInt32ConstantValue(*rows_insn, &rows)) {
                all_constant = false;
            }
            if (!stage_state.GetInt32ConstantValue(*cols_insn, &cols)) {
                all_constant = false;
            }
            component_type = GetComponentType(component_type_insn, is_signed_int);

            if (insn->Opcode() == spv::OpTypeCooperativeMatrixKHR) {
                const spirv::Instruction *use_insn = module_state.FindDef(insn->Word(6));
                if (!stage_state.GetInt32ConstantValue(*use_insn, &use)) {
                    all_constant = false;
                }
            }
        }

        std::string Describe() {
            std::ostringstream ss;
            ss << "rows: " << rows << ", cols: " << cols << ", scope: " << string_VkScopeKHR(scope)
               << ", type: " << string_VkComponentTypeKHR(component_type) << ", use: " << use;
            return ss.str();
        }
    };

    if (module_state.HasCapability(spv::CapabilityCooperativeMatrixKHR)) {
        if (!(entrypoint.stage & phys_dev_ext_props.cooperative_matrix_props_khr.cooperativeMatrixSupportedStages)) {
            skip |=
                LogError("VUID-RuntimeSpirv-cooperativeMatrixSupportedStages-08985", module_state.handle(), loc,
                         "SPIR-V contains OpTypeCooperativeMatrixKHR used in shader stage %s but is not in "
                         "cooperativeMatrixSupportedStages (%s)",
                         string_VkShaderStageFlagBits(entrypoint.stage),
                         string_VkShaderStageFlags(phys_dev_ext_props.cooperative_matrix_props_khr.cooperativeMatrixSupportedStages)
                             .c_str());
        }
    } else if (module_state.HasCapability(spv::CapabilityCooperativeMatrixNV)) {
        if (!(entrypoint.stage & phys_dev_ext_props.cooperative_matrix_props.cooperativeMatrixSupportedStages)) {
            skip |= LogError(
                "VUID-RuntimeSpirv-OpTypeCooperativeMatrixNV-06322", module_state.handle(), loc,
                "SPIR-V contains OpTypeCooperativeMatrixNV used in shader stage %s but is not in cooperativeMatrixSupportedStages "
                "(%s)",
                string_VkShaderStageFlagBits(entrypoint.stage),
                string_VkShaderStageFlags(phys_dev_ext_props.cooperative_matrix_props.cooperativeMatrixSupportedStages).c_str());
        }
    } else {
        return skip;  // If the capability isn't enabled, don't bother with the rest of this function.
    }

    if (!module_state.static_data_.cooperative_matrix_inst.empty() && api_version < VK_API_VERSION_1_3) {
        bool has_full_subgroups = false;
        if (stage_state.pipeline_create_info) {
            has_full_subgroups =
                stage_state.pipeline_create_info->flags & VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT;
        } else {
            has_full_subgroups = stage_state.shader_object_create_info->flags & VK_SHADER_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT;
        }

        if (!has_full_subgroups) {
            const char *vuid = stage_state.HasPipeline() ? "VUID-RuntimeSpirv-OpTypeCooperativeMatrixKHR-10770"
                                                         : "VUID-RuntimeSpirv-OpTypeCooperativeMatrixKHR-10771";
            skip |= LogError(vuid, module_state.handle(), loc,
                             "SPIR-V (%s) contains SPV_KHR_cooperative_matrix which requires SPIR-V 1.6 (Vulkan 1.3). In order to "
                             "use it with older versions, you need to use %s (which requires VK_EXT_subgroup_size_control).",
                             string_VkShaderStageFlagBits(entrypoint.stage),
                             stage_state.HasPipeline() ? "VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT"
                                                       : "VK_SHADER_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT");
        }
    }

    // Map SPIR-V result ID to the ID of its type.
    // TODO - Should have more robust way in ModuleState to find the type
    vvl::unordered_map<uint32_t, uint32_t> id_to_type_id;
    for (const spirv::Instruction &insn : module_state.GetInstructions()) {
        if (OpcodeHasType(insn.Opcode()) && OpcodeHasResult(insn.Opcode())) {
            id_to_type_id[insn.Word(2)] = insn.Word(1);
        }
    }

    auto print_properties = [this]() {
        std::ostringstream ss;
        for (uint32_t i = 0; i < device_state->cooperative_matrix_properties_khr.size(); ++i) {
            const auto &prop = device_state->cooperative_matrix_properties_khr[i];
            ss << "[" << i << "] MSize = " << prop.MSize << " | NSize = " << prop.NSize << " | KSize = " << prop.KSize
               << " | AType = " << string_VkComponentTypeKHR(prop.AType) << " | BType = " << string_VkComponentTypeKHR(prop.BType)
               << " | CType = " << string_VkComponentTypeKHR(prop.CType)
               << " | ResultType = " << string_VkComponentTypeKHR(prop.ResultType) << " | scope = " << string_VkScopeKHR(prop.scope)
               << '\n';
        }
        return ss.str();
    };

    auto print_flexible_properties = [this]() {
        std::ostringstream ss;
        for (uint32_t i = 0; i < device_state->cooperative_matrix_flexible_dimensions_properties.size(); ++i) {
            const auto &prop = device_state->cooperative_matrix_flexible_dimensions_properties[i];
            ss << "[" << i << "] MGranularity = " << prop.MGranularity << " | NGranularity = " << prop.NGranularity
               << " | KGranularity = " << prop.KGranularity << " | AType = " << string_VkComponentTypeKHR(prop.AType)
               << " | BType = " << string_VkComponentTypeKHR(prop.BType) << " | CType = " << string_VkComponentTypeKHR(prop.CType)
               << " | ResultType = " << string_VkComponentTypeKHR(prop.ResultType) << " | scope = " << string_VkScopeKHR(prop.scope)
               << " | workgroupInvocations = " << prop.workgroupInvocations << '\n';
        }
        return ss.str();
    };

    for (const spirv::Instruction *cooperative_matrix_inst : module_state.static_data_.cooperative_matrix_inst) {
        const spirv::Instruction &insn = *cooperative_matrix_inst;
        switch (insn.Opcode()) {
            case spv::OpTypeCooperativeMatrixKHR: {
                CoopMatType m(insn.ResultId(), module_state, stage_state, IsSignedIntType(insn.Word(2)));

                if ((entrypoint.stage & VK_SHADER_STAGE_COMPUTE_BIT) != 0) {
                    if (SafeModulo(local_size.x, effective_subgroup_size) != 0) {
                        const auto vuid_string = m.scope == VK_SCOPE_SUBGROUP_KHR
                                                     ? "VUID-VkPipelineShaderStageCreateInfo-module-08987"
                                                     : "VUID-VkPipelineShaderStageCreateInfo-module-10169";
                        skip |= LogError(vuid_string, module_state.handle(), loc,
                                         "SPIR-V (compute stage) Local workgroup size in the X dimension (%" PRIu32
                                         ") is not a multiple of subgroupSize (%" PRIu32 ").",
                                         local_size.x, effective_subgroup_size);
                    }
                    if (m.scope == VK_SCOPE_WORKGROUP_KHR) {
                        if (workgroup_size >
                            phys_dev_ext_props.cooperative_matrix_props2_nv.cooperativeMatrixWorkgroupScopeMaxWorkgroupSize) {
                            skip |= LogError(
                                "VUID-VkPipelineShaderStageCreateInfo-module-10169", module_state.handle(), loc,
                                "SPIR-V (compute stage) Total local workgroup size (%" PRIu64
                                ") is larger than cooperativeMatrixWorkgroupScopeMaxWorkgroupSize (%" PRIu32 ").",
                                workgroup_size,
                                phys_dev_ext_props.cooperative_matrix_props2_nv.cooperativeMatrixWorkgroupScopeMaxWorkgroupSize);
                        }
                    }
                }

                if (!m.all_constant) {
                    break;
                }

                if (m.scope == VK_SCOPE_WORKGROUP_KHR && !enabled_features.cooperativeMatrixWorkgroupScope) {
                    skip |= LogError("VUID-RuntimeSpirv-cooperativeMatrixWorkgroupScope-10164", module_state.handle(), loc,
                                     "SPIR-V (compute stage) Cooperative matrix uses workgroup scope but "
                                     "cooperativeMatrixWorkgroupScope is not enabled.");
                }

                // Validate that the type parameters are all supported for one of the
                // operands of a cooperative matrix khr property.
                bool valid = false;
                for (uint32_t i = 0; i < device_state->cooperative_matrix_properties_khr.size(); ++i) {
                    const auto &property = device_state->cooperative_matrix_properties_khr[i];
                    if (property.AType == m.component_type && property.MSize == m.rows && property.KSize == m.cols &&
                        property.scope == m.scope && m.use == spv::CooperativeMatrixUseMatrixAKHR) {
                        valid = true;
                        break;
                    }
                    if (property.BType == m.component_type && property.KSize == m.rows && property.NSize == m.cols &&
                        property.scope == m.scope && m.use == spv::CooperativeMatrixUseMatrixBKHR) {
                        valid = true;
                        break;
                    }
                    if (property.CType == m.component_type && property.MSize == m.rows && property.NSize == m.cols &&
                        property.scope == m.scope && m.use == spv::CooperativeMatrixUseMatrixAccumulatorKHR) {
                        valid = true;
                        break;
                    }
                    if (property.ResultType == m.component_type && property.MSize == m.rows && property.NSize == m.cols &&
                        property.scope == m.scope && m.use == spv::CooperativeMatrixUseMatrixAccumulatorKHR) {
                        valid = true;
                        break;
                    }
                }
                if (enabled_features.cooperativeMatrixFlexibleDimensions) {
                    for (uint32_t i = 0; i < device_state->cooperative_matrix_flexible_dimensions_properties.size(); ++i) {
                        const auto &property = device_state->cooperative_matrix_flexible_dimensions_properties[i];

                        if (property.scope == VK_SCOPE_WORKGROUP_KHR && workgroup_size != property.workgroupInvocations) {
                            continue;
                        }

                        if (property.AType == m.component_type && SafeModulo(m.rows, property.MGranularity) == 0 &&
                            SafeModulo(m.cols, property.KGranularity) == 0 && property.scope == m.scope &&
                            m.use == spv::CooperativeMatrixUseMatrixAKHR) {
                            valid = true;
                            break;
                        }
                        if (property.BType == m.component_type && SafeModulo(m.rows, property.KGranularity) == 0 &&
                            SafeModulo(m.cols, property.NGranularity) == 0 && property.scope == m.scope &&
                            m.use == spv::CooperativeMatrixUseMatrixBKHR) {
                            valid = true;
                            break;
                        }
                        if (property.CType == m.component_type && SafeModulo(m.rows, property.MGranularity) == 0 &&
                            SafeModulo(m.cols, property.NGranularity) == 0 && property.scope == m.scope &&
                            m.use == spv::CooperativeMatrixUseMatrixAccumulatorKHR) {
                            valid = true;
                            break;
                        }
                        if (property.ResultType == m.component_type && SafeModulo(m.rows, property.MGranularity) == 0 &&
                            SafeModulo(m.cols, property.NGranularity) == 0 && property.scope == m.scope &&
                            m.use == spv::CooperativeMatrixUseMatrixAccumulatorKHR) {
                            valid = true;
                            break;
                        }
                    }
                }
                if (!valid) {
                    if (!enabled_features.cooperativeMatrixFlexibleDimensions) {
                        skip |= LogError("VUID-RuntimeSpirv-OpTypeCooperativeMatrixKHR-10163", module_state.handle(), loc,
                                         "SPIR-V (%s) has\n%s (%s)\nbut doesn't match any VkCooperativeMatrixPropertiesKHR\n%s.",
                                         string_VkShaderStageFlagBits(entrypoint.stage), insn.Describe().c_str(),
                                         m.Describe().c_str(), print_properties().c_str());
                    } else {
                        skip |= LogError("VUID-RuntimeSpirv-cooperativeMatrixFlexibleDimensions-10165", module_state.handle(), loc,
                                         "SPIR-V (%s) has\n%s (%s)\nbut doesn't match any VkCooperativeMatrixPropertiesKHR or "
                                         "VkCooperativeMatrixFlexibleDimensionsPropertiesNV\n%s\n%s.",
                                         string_VkShaderStageFlagBits(entrypoint.stage), insn.Describe().c_str(),
                                         m.Describe().c_str(), print_properties().c_str(), print_flexible_properties().c_str());
                    }
                }
                if (IsExtEnabled(extensions.vk_nv_cooperative_matrix2)) {
                    if (m.rows > phys_dev_ext_props.cooperative_matrix_props2_nv.cooperativeMatrixFlexibleDimensionsMaxDimension ||
                        m.cols > phys_dev_ext_props.cooperative_matrix_props2_nv.cooperativeMatrixFlexibleDimensionsMaxDimension) {
                        skip |= LogError(
                            "VUID-RuntimeSpirv-cooperativeMatrixFlexibleDimensionsMaxDimension-10167", module_state.handle(), loc,
                            "SPIR-V (%s) has\n%s (%s)\nbut number of rows or columns is greater than "
                            "cooperativeMatrixFlexibleDimensionsMaxDimension (%" PRIu32 ").",
                            string_VkShaderStageFlagBits(entrypoint.stage), insn.Describe().c_str(), m.Describe().c_str(),
                            phys_dev_ext_props.cooperative_matrix_props2_nv.cooperativeMatrixFlexibleDimensionsMaxDimension);
                    }
                }

                break;
            }
            case spv::OpCooperativeMatrixMulAddKHR: {
                const uint32_t flags = insn.Length() > 6 ? insn.Word(6) : 0u;
                CoopMatType r(id_to_type_id[insn.Word(2)], module_state, stage_state,
                              (flags & spv::CooperativeMatrixOperandsMatrixResultSignedComponentsKHRMask));
                CoopMatType a(id_to_type_id[insn.Word(3)], module_state, stage_state,
                              (flags & spv::CooperativeMatrixOperandsMatrixASignedComponentsKHRMask));
                CoopMatType b(id_to_type_id[insn.Word(4)], module_state, stage_state,
                              (flags & spv::CooperativeMatrixOperandsMatrixBSignedComponentsKHRMask));
                CoopMatType c(id_to_type_id[insn.Word(5)], module_state, stage_state,
                              (flags & spv::CooperativeMatrixOperandsMatrixCSignedComponentsKHRMask));
                if (a.all_constant && b.all_constant && c.all_constant && r.all_constant) {
                    // Validate that the type parameters are all supported for the same
                    // cooperative matrix property.
                    bool found_matching_prop = false;
                    for (uint32_t i = 0; i < device_state->cooperative_matrix_properties_khr.size(); ++i) {
                        const auto &property = device_state->cooperative_matrix_properties_khr[i];

                        bool valid = true;
                        valid &= property.AType == a.component_type && property.MSize == a.rows && property.KSize == a.cols &&
                                 property.scope == a.scope && a.use == spv::CooperativeMatrixUseMatrixAKHR;
                        valid &= property.BType == b.component_type && property.KSize == b.rows && property.NSize == b.cols &&
                                 property.scope == b.scope && b.use == spv::CooperativeMatrixUseMatrixBKHR;
                        valid &= property.CType == c.component_type && property.MSize == c.rows && property.NSize == c.cols &&
                                 property.scope == c.scope && c.use == spv::CooperativeMatrixUseMatrixAccumulatorKHR;
                        valid &= property.ResultType == r.component_type && property.MSize == r.rows && property.NSize == r.cols &&
                                 property.scope == r.scope && r.use == spv::CooperativeMatrixUseMatrixAccumulatorKHR;

                        valid &= !IsSignedIntEnum(property.AType) ||
                                 (flags & spv::CooperativeMatrixOperandsMatrixASignedComponentsKHRMask);
                        valid &= !IsSignedIntEnum(property.BType) ||
                                 (flags & spv::CooperativeMatrixOperandsMatrixBSignedComponentsKHRMask);
                        valid &= !IsSignedIntEnum(property.CType) ||
                                 (flags & spv::CooperativeMatrixOperandsMatrixCSignedComponentsKHRMask);
                        valid &= !IsSignedIntEnum(property.ResultType) ||
                                 (flags & spv::CooperativeMatrixOperandsMatrixResultSignedComponentsKHRMask);

                        valid &= property.saturatingAccumulation ==
                                 !!(flags & spv::CooperativeMatrixOperandsSaturatingAccumulationKHRMask);

                        if (valid) {
                            found_matching_prop = true;
                            break;
                        }
                    }
                    bool found_matching_flexible_prop = false;
                    if (enabled_features.cooperativeMatrixFlexibleDimensions) {
                        for (uint32_t i = 0; i < device_state->cooperative_matrix_flexible_dimensions_properties.size(); ++i) {
                            const auto &property = device_state->cooperative_matrix_flexible_dimensions_properties[i];

                            bool valid = true;
                            valid &= property.AType == a.component_type && SafeModulo(a.rows, property.MGranularity) == 0 &&
                                     SafeModulo(a.cols, property.KGranularity) == 0 && property.scope == a.scope &&
                                     a.use == spv::CooperativeMatrixUseMatrixAKHR;
                            valid &= property.BType == b.component_type && SafeModulo(b.rows, property.KGranularity) == 0 &&
                                     SafeModulo(b.cols, property.NGranularity) == 0 && property.scope == b.scope &&
                                     b.use == spv::CooperativeMatrixUseMatrixBKHR;
                            valid &= property.CType == c.component_type && SafeModulo(c.rows, property.MGranularity) == 0 &&
                                     SafeModulo(c.cols, property.NGranularity) == 0 && property.scope == c.scope &&
                                     c.use == spv::CooperativeMatrixUseMatrixAccumulatorKHR;
                            valid &= property.ResultType == r.component_type && SafeModulo(r.rows, property.MGranularity) == 0 &&
                                     SafeModulo(r.cols, property.NGranularity) == 0 && property.scope == r.scope &&
                                     r.use == spv::CooperativeMatrixUseMatrixAccumulatorKHR;

                            valid &= !IsSignedIntEnum(property.AType) ||
                                     (flags & spv::CooperativeMatrixOperandsMatrixASignedComponentsKHRMask);
                            valid &= !IsSignedIntEnum(property.BType) ||
                                     (flags & spv::CooperativeMatrixOperandsMatrixBSignedComponentsKHRMask);
                            valid &= !IsSignedIntEnum(property.CType) ||
                                     (flags & spv::CooperativeMatrixOperandsMatrixCSignedComponentsKHRMask);
                            valid &= !IsSignedIntEnum(property.ResultType) ||
                                     (flags & spv::CooperativeMatrixOperandsMatrixResultSignedComponentsKHRMask);

                            valid &= property.saturatingAccumulation ==
                                     !!(flags & spv::CooperativeMatrixOperandsSaturatingAccumulationKHRMask);

                            valid &= property.scope != VK_SCOPE_WORKGROUP_KHR || workgroup_size == property.workgroupInvocations;

                            if (valid) {
                                found_matching_flexible_prop = true;
                                break;
                            }
                        }
                    }
                    if (!found_matching_prop && !found_matching_flexible_prop) {
                        if (!enabled_features.cooperativeMatrixFlexibleDimensions) {
                            skip |= LogError("VUID-RuntimeSpirv-OpCooperativeMatrixMulAddKHR-10060", module_state.handle(), loc,
                                             "SPIR-V (%s) instruction\n%s\ndoesn't match a supported matrix "
                                             "VkCooperativeMatrixPropertiesKHR\n%s\n%s\n%s\n%s\n%s\n",
                                             string_VkShaderStageFlagBits(entrypoint.stage), insn.Describe().c_str(),
                                             a.Describe().c_str(), b.Describe().c_str(), c.Describe().c_str(), r.Describe().c_str(),
                                             print_properties().c_str());
                        } else {
                            skip |=
                                LogError("VUID-RuntimeSpirv-cooperativeMatrixFlexibleDimensions-10166", module_state.handle(), loc,
                                         "SPIR-V (%s) instruction\n%s\ndoesn't match a supported matrix "
                                         "VkCooperativeMatrixPropertiesKHR or "
                                         "VkPhysicalDeviceCooperativeMatrix2PropertiesNV\n%s\n%s\n%s\n%s\n%s\n%s\n",
                                         string_VkShaderStageFlagBits(entrypoint.stage), insn.Describe().c_str(),
                                         a.Describe().c_str(), b.Describe().c_str(), c.Describe().c_str(), r.Describe().c_str(),
                                         print_properties().c_str(), print_flexible_properties().c_str());
                        }
                    }
                }
                break;
            }
            case spv::OpTypeCooperativeMatrixNV: {
                CoopMatType m(insn.ResultId(), module_state, stage_state, IsSignedIntType(insn.Word(2)));

                if (!m.all_constant) {
                    break;
                }
                // Validate that the type parameters are all supported for one of the
                // operands of a cooperative matrix property.
                bool valid = false;
                for (uint32_t i = 0; i < device_state->cooperative_matrix_properties_nv.size(); ++i) {
                    const auto &property = device_state->cooperative_matrix_properties_nv[i];
                    if (property.AType == m.component_type && property.MSize == m.rows && property.KSize == m.cols &&
                        property.scope == m.scope) {
                        valid = true;
                        break;
                    }
                    if (property.BType == m.component_type && property.KSize == m.rows && property.NSize == m.cols &&
                        property.scope == m.scope) {
                        valid = true;
                        break;
                    }
                    if (property.CType == m.component_type && property.MSize == m.rows && property.NSize == m.cols &&
                        property.scope == m.scope) {
                        valid = true;
                        break;
                    }
                    if (property.DType == m.component_type && property.MSize == m.rows && property.NSize == m.cols &&
                        property.scope == m.scope) {
                        valid = true;
                        break;
                    }
                }
                if (!valid) {
                    skip |= LogError("VUID-RuntimeSpirv-OpTypeCooperativeMatrixNV-06316", module_state.handle(), loc,
                                     "SPIR-V (%s) has an OpTypeCooperativeMatrixNV (result id = %" PRIu32
                                     ") operand that don't match a supported matrix type (%s).",
                                     string_VkShaderStageFlagBits(entrypoint.stage), insn.Word(1), m.Describe().c_str());
                }
                break;
            }
            case spv::OpCooperativeMatrixMulAddNV: {
                CoopMatType d(id_to_type_id[insn.Word(2)], module_state, stage_state, IsSignedIntType(id_to_type_id[insn.Word(2)]));
                CoopMatType a(id_to_type_id[insn.Word(3)], module_state, stage_state, IsSignedIntType(id_to_type_id[insn.Word(3)]));
                CoopMatType b(id_to_type_id[insn.Word(4)], module_state, stage_state, IsSignedIntType(id_to_type_id[insn.Word(4)]));
                CoopMatType c(id_to_type_id[insn.Word(5)], module_state, stage_state, IsSignedIntType(id_to_type_id[insn.Word(5)]));

                if (a.all_constant && b.all_constant && c.all_constant && d.all_constant) {
                    // Validate that the type parameters are all supported for the same
                    // cooperative matrix property.
                    bool valid_a = false;
                    bool valid_b = false;
                    bool valid_c = false;
                    bool valid_d = false;
                    for (uint32_t i = 0; i < device_state->cooperative_matrix_properties_nv.size(); ++i) {
                        const auto &property = device_state->cooperative_matrix_properties_nv[i];
                        valid_a |= property.AType == a.component_type && property.MSize == a.rows && property.KSize == a.cols &&
                                   property.scope == a.scope;
                        valid_b |= property.BType == b.component_type && property.KSize == b.rows && property.NSize == b.cols &&
                                   property.scope == b.scope;
                        valid_c |= property.CType == c.component_type && property.MSize == c.rows && property.NSize == c.cols &&
                                   property.scope == c.scope;
                        valid_d |= property.DType == d.component_type && property.MSize == d.rows && property.NSize == d.cols &&
                                   property.scope == d.scope;
                        if (valid_a && valid_b && valid_c && valid_d) {
                            break;
                        }
                    }
                    if (!valid_a) {
                        skip |= LogError("VUID-RuntimeSpirv-OpTypeCooperativeMatrixMulAddNV-10059", module_state.handle(), loc,
                                         "SPIR-V (%s) OpCooperativeMatrixMulAddNV (result id = %" PRIu32
                                         ") operands don't match a supported matrix "
                                         "VkCooperativeMatrixPropertiesNV for A type (%s).",
                                         string_VkShaderStageFlagBits(entrypoint.stage), insn.Word(2), a.Describe().c_str());
                    } else if (!valid_b) {
                        skip |= LogError("VUID-RuntimeSpirv-OpTypeCooperativeMatrixMulAddNV-10059", module_state.handle(), loc,
                                         "SPIR-V (%s) OpCooperativeMatrixMulAddNV (result id = %" PRIu32
                                         ") operands don't match a supported matrix "
                                         "VkCooperativeMatrixPropertiesNV for B type (%s).",
                                         string_VkShaderStageFlagBits(entrypoint.stage), insn.Word(2), b.Describe().c_str());
                    } else if (!valid_c) {
                        skip |= LogError("VUID-RuntimeSpirv-OpTypeCooperativeMatrixMulAddNV-10059", module_state.handle(), loc,
                                         "SPIR-V (%s) OpCooperativeMatrixMulAddNV (result id = %" PRIu32
                                         ") operands don't match a supported matrix "
                                         "VkCooperativeMatrixPropertiesNV for C type (%s).",
                                         string_VkShaderStageFlagBits(entrypoint.stage), insn.Word(2), c.Describe().c_str());
                    } else if (!valid_d) {
                        skip |= LogError("VUID-RuntimeSpirv-OpTypeCooperativeMatrixMulAddNV-10059", module_state.handle(), loc,
                                         "SPIR-V (%s) OpCooperativeMatrixMulAddNV (result id = %" PRIu32
                                         ") operands don't match a supported matrix "
                                         "VkCooperativeMatrixPropertiesNV for D type (%s).",
                                         string_VkShaderStageFlagBits(entrypoint.stage), insn.Word(2), d.Describe().c_str());
                    }
                }
                break;
            }
            default:
                break;
        }
    }

    return skip;
}

bool CoreChecks::ValidateCooperativeVector(const spirv::Module &module_state, const spirv::EntryPoint &entrypoint,
                                           const ShaderStageState &stage_state, const Location &loc) const {
    bool skip = false;

    struct CoopVecType {
        VkComponentTypeKHR component_type;
        uint32_t component_count;
        bool all_constant;

        CoopVecType(uint32_t id, const spirv::Module &module_state, const ShaderStageState &stage_state, bool is_signed) {
            const spirv::Instruction *insn = module_state.FindDef(id);
            const spirv::Instruction *component_type_insn = module_state.FindDef(insn->Word(2));
            const spirv::Instruction *component_count_insn = module_state.FindDef(insn->Word(3));

            all_constant = true;
            if (!stage_state.GetInt32ConstantValue(*component_count_insn, &component_count)) {
                all_constant = false;
            }
            component_type = GetComponentType(component_type_insn, is_signed);
        }

        std::string Describe() {
            std::ostringstream ss;
            ss << "component count: " << component_count << ", type: " << string_VkComponentTypeKHR(component_type);
            return ss.str();
        }
    };

    if (module_state.HasCapability(spv::CapabilityCooperativeVectorNV) ||
        module_state.HasCapability(spv::CapabilityCooperativeVectorTrainingNV)) {
        if (!(entrypoint.stage & phys_dev_ext_props.cooperative_vector_props_nv.cooperativeVectorSupportedStages)) {
            skip |= LogError(
                "VUID-RuntimeSpirv-cooperativeVectorSupportedStages-10091", module_state.handle(), loc,
                "SPIR-V contains cooperative vector capability used in shader stage %s but is not in "
                "cooperativeVectorSupportedStages (%s)",
                string_VkShaderStageFlagBits(entrypoint.stage),
                string_VkShaderStageFlags(phys_dev_ext_props.cooperative_vector_props_nv.cooperativeVectorSupportedStages).c_str());
        }
    } else {
        return skip;
    }

    vvl::unordered_map<uint32_t, uint32_t> id_to_type_id;
    for (const spirv::Instruction &insn : module_state.GetInstructions()) {
        if (OpcodeHasType(insn.Opcode()) && OpcodeHasResult(insn.Opcode())) {
            id_to_type_id[insn.Word(2)] = insn.Word(1);
        }
    }
    for (const spirv::Instruction *cooperative_vector_inst : module_state.static_data_.cooperative_vector_inst) {
        const spirv::Instruction &insn = *cooperative_vector_inst;
        switch (insn.Opcode()) {
            case spv::OpTypeCooperativeVectorNV: {
                // SPIR-V integer types are not strictly signed or unsigned. Allow this type to
                // match against either signed or unsigned types in the device properties.
                CoopVecType m_signed(insn.Word(1), module_state, stage_state, true);
                CoopVecType m_unsigned(insn.Word(1), module_state, stage_state, false);

                if (!m_signed.all_constant) {
                    break;
                }

                if (m_signed.component_count > phys_dev_ext_props.cooperative_vector_props_nv.maxCooperativeVectorComponents) {
                    skip |= LogError("VUID-RuntimeSpirv-maxCooperativeVectorComponents-10094", module_state.handle(), loc,
                                     "SPIR-V (%s) component count (%d) is greater than maxCooperativeVectorComponents (%d)",
                                     string_VkShaderStageFlagBits(entrypoint.stage), m_signed.component_count,
                                     phys_dev_ext_props.cooperative_vector_props_nv.maxCooperativeVectorComponents);
                }

                bool found = false;
                for (uint32_t i = 0; i < device_state->cooperative_vector_properties_nv.size(); ++i) {
                    const auto &property = device_state->cooperative_vector_properties_nv[i];
                    if (m_signed.component_type == property.inputType || m_signed.component_type == property.resultType ||
                        m_unsigned.component_type == property.inputType || m_unsigned.component_type == property.resultType) {
                        found = true;
                        break;
                    }
                }

                if (!found) {
                    skip |= LogError("VUID-RuntimeSpirv-OpTypeCooperativeVector-10095", module_state.handle(), loc,
                                     "SPIR-V (%s) contains unsupported cooperative vector component type (%s)",
                                     string_VkShaderStageFlagBits(entrypoint.stage),
                                     string_VkComponentTypeKHR((VkComponentTypeKHR)m_signed.component_type));
                }

                break;
            }
            case spv::OpCooperativeVectorLoadNV:
            case spv::OpCooperativeVectorStoreNV: {
                // Nothing we can validate outside of GPUAV
                break;
            }
            case spv::OpCooperativeVectorMatrixMulNV:
            case spv::OpCooperativeVectorMatrixMulAddNV: {
                uint32_t matrix_operands = 0;
                if (insn.Opcode() == spv::OpCooperativeVectorMatrixMulAddNV) {
                    if (insn.Length() > 16) {
                        matrix_operands = insn.Word(16);
                    }
                } else {
                    if (insn.Length() > 13) {
                        matrix_operands = insn.Word(13);
                    }
                }
                bool result_is_signed = matrix_operands & spv::CooperativeMatrixOperandsMatrixResultSignedComponentsKHRMask;
                bool input_is_signed = matrix_operands & spv::CooperativeMatrixOperandsMatrixBSignedComponentsKHRMask;

                CoopVecType result(id_to_type_id[insn.Word(2)], module_state, stage_state, result_is_signed);
                CoopVecType input(id_to_type_id[insn.Word(3)], module_state, stage_state, input_is_signed);

                uint32_t result_type = result.component_type;
                uint32_t input_type = input.component_type;

                uint32_t biasOffset = insn.Opcode() == spv::OpCooperativeVectorMatrixMulAddNV ? 3 : 0;

                bool all_constant = true;
                uint32_t input_interpretation{};
                uint32_t matrix_interpretation{};
                uint32_t bias_interpretation{};
                bool transpose{};
                if (!stage_state.GetInt32ConstantValue(*module_state.FindDef(insn.Word(4)), &input_interpretation)) {
                    all_constant = false;
                }
                if (!stage_state.GetInt32ConstantValue(*module_state.FindDef(insn.Word(7)), &matrix_interpretation)) {
                    all_constant = false;
                }
                if (insn.Opcode() == spv::OpCooperativeVectorMatrixMulAddNV) {
                    if (!stage_state.GetInt32ConstantValue(*module_state.FindDef(insn.Word(10)), &bias_interpretation)) {
                        all_constant = false;
                    }
                }
                if (!stage_state.GetBooleanConstantValue(*module_state.FindDef(insn.Word(11 + biasOffset)), &transpose)) {
                    all_constant = false;
                }

                if (!all_constant) {
                    break;
                }

                bool found = false;
                for (uint32_t i = 0; i < device_state->cooperative_vector_properties_nv.size(); ++i) {
                    const auto &property = device_state->cooperative_vector_properties_nv[i];
                    if (property.inputType == input_type && property.inputInterpretation == input_interpretation &&
                        property.matrixInterpretation == matrix_interpretation &&
                        (insn.Opcode() == spv::OpCooperativeVectorMatrixMulNV ||
                         property.biasInterpretation == bias_interpretation) &&
                        property.resultType == result_type && (!transpose || property.transpose)) {
                        found = true;
                        break;
                    }
                }

                if (!found) {
                    skip |= LogError("VUID-RuntimeSpirv-OpCooperativeVectorMatrixMulNV-10089", module_state.handle(), loc,
                                     "SPIR-V (%s) contains unsupported cooperative vector matrix mul with "
                                     "result component type (%s), input component type (%s), input interpretation (%s), "
                                     "matrix interpretation (%s), bias interpretation (%s), transpose (%d)",
                                     string_VkShaderStageFlagBits(entrypoint.stage),
                                     string_VkComponentTypeKHR((VkComponentTypeKHR)result_type),
                                     string_VkComponentTypeKHR((VkComponentTypeKHR)input_type),
                                     string_VkComponentTypeKHR((VkComponentTypeKHR)input_interpretation),
                                     string_VkComponentTypeKHR((VkComponentTypeKHR)matrix_interpretation),
                                     (insn.Opcode() == spv::OpCooperativeVectorMatrixMulNV
                                          ? "None"
                                          : string_VkComponentTypeKHR((VkComponentTypeKHR)bias_interpretation)),
                                     transpose);
                }

                uint32_t memory_layout{};
                if (stage_state.GetInt32ConstantValue(*module_state.FindDef(insn.Word(10 + biasOffset)), &memory_layout)) {
                    if ((matrix_interpretation == VK_COMPONENT_TYPE_FLOAT_E4M3_NV ||
                         matrix_interpretation == VK_COMPONENT_TYPE_FLOAT_E5M2_NV) &&
                        !(memory_layout == VK_COOPERATIVE_VECTOR_MATRIX_LAYOUT_INFERENCING_OPTIMAL_NV ||
                          memory_layout == VK_COOPERATIVE_VECTOR_MATRIX_LAYOUT_TRAINING_OPTIMAL_NV)) {
                        skip |=
                            LogError("VUID-RuntimeSpirv-OpCooperativeVectorMatrixMulNV-10090", module_state.handle(), loc,
                                     "SPIR-V (%s) contains unsupported cooperative vector matrix mul with "
                                     "matrix_interpretation (%s) and memory layout (%s)",
                                     string_VkShaderStageFlagBits(entrypoint.stage),
                                     string_VkComponentTypeKHR((VkComponentTypeKHR)matrix_interpretation),
                                     string_VkCooperativeVectorMatrixLayoutNV((VkCooperativeVectorMatrixLayoutNV)memory_layout));
                    }
                }
                break;
            }
            case spv::OpCooperativeVectorReduceSumAccumulateNV: {
                CoopVecType v(id_to_type_id[insn.Word(3)], module_state, stage_state, false);

                switch (v.component_type) {
                    case VK_COMPONENT_TYPE_FLOAT16_KHR:
                        if (!phys_dev_ext_props.cooperative_vector_props_nv.cooperativeVectorTrainingFloat16Accumulation) {
                            skip |= LogError("VUID-RuntimeSpirv-OpCooperativeVectorReduceSumAccumulateNV-10092",
                                             module_state.handle(), loc,
                                             "SPIR-V (%s) Component type is FLOAT16 but "
                                             "cooperativeVectorTrainingFloat16Accumulation not supported",
                                             string_VkShaderStageFlagBits(entrypoint.stage));
                        }
                        break;
                    case VK_COMPONENT_TYPE_FLOAT32_KHR:
                        if (!phys_dev_ext_props.cooperative_vector_props_nv.cooperativeVectorTrainingFloat32Accumulation) {
                            skip |= LogError("VUID-RuntimeSpirv-OpCooperativeVectorReduceSumAccumulateNV-10092",
                                             module_state.handle(), loc,
                                             "SPIR-V (%s) Component type is FLOAT32 but "
                                             "cooperativeVectorTrainingFloat32Accumulation not supported",
                                             string_VkShaderStageFlagBits(entrypoint.stage));
                        }
                        break;
                    default:
                        skip |=
                            LogError("VUID-RuntimeSpirv-OpCooperativeVectorReduceSumAccumulateNV-10092", module_state.handle(), loc,
                                     "SPIR-V (%s) Unsupported component type (%s)", string_VkShaderStageFlagBits(entrypoint.stage),
                                     string_VkComponentTypeKHR((VkComponentTypeKHR)v.component_type));
                        break;
                }

                const spirv::Instruction *ptr_type = module_state.FindDef(id_to_type_id[insn.Word(1)]);
                if (ptr_type->StorageClass() != spv::StorageClassStorageBuffer &&
                    ptr_type->StorageClass() != spv::StorageClassPhysicalStorageBuffer) {
                    skip |=
                        LogError("VUID-RuntimeSpirv-OpCooperativeVectorReduceSumAccumulateNV-10092", module_state.handle(), loc,
                                 "SPIR-V (%s) Unsupported pointer storage class (%s)",
                                 string_VkShaderStageFlagBits(entrypoint.stage), string_SpvStorageClass(ptr_type->StorageClass()));
                }

                break;
            }

            case spv::OpCooperativeVectorOuterProductAccumulateNV: {
                uint32_t matrix_interpretation{};
                if (stage_state.GetInt32ConstantValue(*module_state.FindDef(insn.Word(6)), &matrix_interpretation)) {
                    switch (matrix_interpretation) {
                        case VK_COMPONENT_TYPE_FLOAT16_KHR:
                            if (!phys_dev_ext_props.cooperative_vector_props_nv.cooperativeVectorTrainingFloat16Accumulation) {
                                skip |= LogError("VUID-RuntimeSpirv-OpCooperativeVectorOuterProductAccumulateNV-10093",
                                                 module_state.handle(), loc,
                                                 "SPIR-V (%s) Matrix interpretation is FLOAT16 but "
                                                 "cooperativeVectorTrainingFloat16Accumulation not supported",
                                                 string_VkShaderStageFlagBits(entrypoint.stage));
                            }
                            break;
                        case VK_COMPONENT_TYPE_FLOAT32_KHR:
                            if (!phys_dev_ext_props.cooperative_vector_props_nv.cooperativeVectorTrainingFloat32Accumulation) {
                                skip |= LogError("VUID-RuntimeSpirv-OpCooperativeVectorOuterProductAccumulateNV-10093",
                                                 module_state.handle(), loc,
                                                 "SPIR-V (%s) Matrix interpretation is FLOAT32 but "
                                                 "cooperativeVectorTrainingFloat32Accumulation not supported",
                                                 string_VkShaderStageFlagBits(entrypoint.stage));
                            }
                            break;
                        default:
                            skip |= LogError("VUID-RuntimeSpirv-OpCooperativeVectorOuterProductAccumulateNV-10093",
                                             module_state.handle(), loc, "SPIR-V (%s) Unsupported Matrix interpretation (%s)",
                                             string_VkShaderStageFlagBits(entrypoint.stage),
                                             string_VkComponentTypeKHR((VkComponentTypeKHR)matrix_interpretation));
                            break;
                    }
                }

                CoopVecType a(id_to_type_id[insn.Word(3)], module_state, stage_state, false);
                CoopVecType b(id_to_type_id[insn.Word(4)], module_state, stage_state, false);

                if (a.component_type != VK_COMPONENT_TYPE_FLOAT16_KHR) {
                    skip |= LogError("VUID-RuntimeSpirv-OpCooperativeVectorOuterProductAccumulateNV-10093", module_state.handle(),
                                     loc, "SPIR-V (%s) Component type of A (%s) must be FLOAT16",
                                     string_VkShaderStageFlagBits(entrypoint.stage),
                                     string_VkComponentTypeKHR((VkComponentTypeKHR)a.component_type));
                }
                if (b.component_type != VK_COMPONENT_TYPE_FLOAT16_KHR) {
                    skip |= LogError("VUID-RuntimeSpirv-OpCooperativeVectorOuterProductAccumulateNV-10093", module_state.handle(),
                                     loc, "SPIR-V (%s) Component type of B (%s) must be FLOAT16",
                                     string_VkShaderStageFlagBits(entrypoint.stage),
                                     string_VkComponentTypeKHR((VkComponentTypeKHR)b.component_type));
                }

                uint32_t memory_layout{};
                if (stage_state.GetInt32ConstantValue(*module_state.FindDef(insn.Word(5)), &memory_layout)) {
                    if (memory_layout != VK_COOPERATIVE_VECTOR_MATRIX_LAYOUT_TRAINING_OPTIMAL_NV) {
                        skip |=
                            LogError("VUID-RuntimeSpirv-OpCooperativeVectorOuterProductAccumulateNV-10093", module_state.handle(),
                                     loc, "SPIR-V (%s) Memory layout (%s) must be TRAINING_OPTIMAL",
                                     string_VkShaderStageFlagBits(entrypoint.stage),
                                     string_VkCooperativeVectorMatrixLayoutNV((VkCooperativeVectorMatrixLayoutNV)memory_layout));
                    }
                }

                const spirv::Instruction *ptr_type = module_state.FindDef(id_to_type_id[insn.Word(1)]);
                if (ptr_type->StorageClass() != spv::StorageClassStorageBuffer &&
                    ptr_type->StorageClass() != spv::StorageClassPhysicalStorageBuffer) {
                    skip |=
                        LogError("VUID-RuntimeSpirv-OpCooperativeVectorOuterProductAccumulateNV-10093", module_state.handle(), loc,
                                 "SPIR-V (%s) Unsupported pointer storage class (%s)",
                                 string_VkShaderStageFlagBits(entrypoint.stage), string_SpvStorageClass(ptr_type->StorageClass()));
                }

                break;
            }

            default:
                assert(false);  // unexpected instruction
                break;
        }
    }
    return skip;
}

bool CoreChecks::ValidateShader64BitIndexing(const spirv::Module &module_state, const spirv::EntryPoint &entrypoint,
                                             const ShaderStageState &stage_state, const vvl::Pipeline *pipeline,
                                             const Location &loc) const {
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

    auto const &check = [&](uint32_t value_id) -> bool {
        auto value_insn = module_state.FindDef(value_id);
        auto type_insn = module_state.FindDef(value_insn->Word(1));
        return type_insn->Word(2) != 32;
    };

    for (const spirv::Instruction *cooperative_vector_inst : module_state.static_data_.cooperative_vector_inst) {
        const spirv::Instruction &insn = *cooperative_vector_inst;
        switch (insn.Opcode()) {
            case spv::OpCooperativeVectorMatrixMulNV:
            case spv::OpCooperativeVectorMatrixMulAddNV: {
                uint32_t matrix_offset_id = insn.Word(6);
                if (check(matrix_offset_id)) {
                    skip |=
                        LogError("VUID-RuntimeSpirv-OpCooperativeVectorMatrixMulAddNV-11808", module_state.handle(), loc,
                                 "SPIR-V (%s) contains 64-bit matrix offset\n%s\n", string_VkShaderStageFlagBits(entrypoint.stage),
                                 module_state.DescribeInstruction(insn).c_str());
                }

                if (insn.Opcode() == spv::OpCooperativeVectorMatrixMulAddNV) {
                    uint32_t bias_offset_id = insn.Word(9);
                    if (check(bias_offset_id)) {
                        skip |= LogError("VUID-RuntimeSpirv-OpCooperativeVectorMatrixMulAddNV-11808", module_state.handle(), loc,
                                         "SPIR-V (%s) contains 64-bit bias offset\n%s\n",
                                         string_VkShaderStageFlagBits(entrypoint.stage),
                                         module_state.DescribeInstruction(insn).c_str());
                    }
                }
                break;
            }
            case spv::OpCooperativeVectorLoadNV: {
                if (check(insn.Word(4))) {
                    skip |=
                        LogError("VUID-RuntimeSpirv-OpCooperativeVectorLoadNV-11809", module_state.handle(), loc,
                                 "SPIR-V (%s) contains 64-bit load offset\n%s\n", string_VkShaderStageFlagBits(entrypoint.stage),
                                 module_state.DescribeInstruction(insn).c_str());
                }
                break;
            }
            case spv::OpCooperativeVectorStoreNV: {
                if (check(insn.Word(2))) {
                    skip |=
                        LogError("VUID-RuntimeSpirv-OpCooperativeVectorLoadNV-11809", module_state.handle(), loc,
                                 "SPIR-V (%s) contains 64-bit store offset\n%s\n", string_VkShaderStageFlagBits(entrypoint.stage),
                                 module_state.DescribeInstruction(insn).c_str());
                }
                break;
            }
            case spv::OpCooperativeVectorReduceSumAccumulateNV: {
                if (check(insn.Word(2))) {
                    skip |=
                        LogError("VUID-RuntimeSpirv-OpCooperativeVectorLoadNV-11809", module_state.handle(), loc,
                                 "SPIR-V (%s) contains 64-bit reducesum offset\n%s\n",
                                 string_VkShaderStageFlagBits(entrypoint.stage), module_state.DescribeInstruction(insn).c_str());
                }
                break;
            }
            case spv::OpCooperativeVectorOuterProductAccumulateNV: {
                if (check(insn.Word(2))) {
                    skip |=
                        LogError("VUID-RuntimeSpirv-OpCooperativeVectorLoadNV-11809", module_state.handle(), loc,
                                 "SPIR-V (%s) contains 64-bit outerproduct offset\n%s\n",
                                 string_VkShaderStageFlagBits(entrypoint.stage), module_state.DescribeInstruction(insn).c_str());
                }
                break;
            }
            default:
                break;
        }
    }
    for (const spirv::Instruction *array_length_inst : module_state.static_data_.array_length_inst) {
        const spirv::Instruction &insn = *array_length_inst;
        if (check(insn.Word(1))) {
            skip |= LogError("VUID-RuntimeSpirv-OpArrayLength-11807", module_state.handle(), loc,
                             "SPIR-V (%s) contains 64-bit array length return type\n%s\n",
                             string_VkShaderStageFlagBits(entrypoint.stage), module_state.DescribeInstruction(insn).c_str());
        }
    }
    return skip;
}

bool CoreChecks::ValidateShaderResolveQCOM(const spirv::Module &module_state, VkShaderStageFlagBits stage,
                                           const vvl::Pipeline &pipeline, const Location &loc) const {
    bool skip = false;

    // If the pipeline's subpass description contains flag VK_SUBPASS_DESCRIPTION_FRAGMENT_REGION_BIT_QCOM,
    // then the fragment shader must not enable the SPIRV SampleRateShading capability.
    if (stage == VK_SHADER_STAGE_FRAGMENT_BIT && module_state.HasCapability(spv::CapabilitySampleRateShading)) {
        const auto &rp_state = pipeline.RenderPassState();
        auto subpass_flags = (!rp_state) ? 0 : rp_state->create_info.pSubpasses[pipeline.Subpass()].flags;
        if ((subpass_flags & VK_SUBPASS_DESCRIPTION_FRAGMENT_REGION_BIT_QCOM) != 0) {
            const LogObjectList objlist(module_state.handle(), rp_state->Handle());
            skip |= LogError("VUID-RuntimeSpirv-SampleRateShading-06378", objlist, loc,
                             "SPIR-V (Fragment stage) enables SampleRateShading capability "
                             "and the subpass flags includes VK_SUBPASS_DESCRIPTION_FRAGMENT_REGION_BIT_QCOM.");
        }
    }

    return skip;
}

bool CoreChecks::ValidateShaderExecutionModes(const spirv::Module &module_state, const spirv::EntryPoint &entrypoint,
                                              VkShaderStageFlagBits stage, const vvl::Pipeline *pipeline,
                                              const Location &loc) const {
    bool skip = false;

    if (entrypoint.stage == VK_SHADER_STAGE_GEOMETRY_BIT) {
        const uint32_t vertices_out = entrypoint.execution_mode.output_vertices;
        const uint32_t invocations = entrypoint.execution_mode.invocations;
        if (vertices_out != spirv::kInvalidValue &&
            (vertices_out == 0 || vertices_out > phys_dev_props.limits.maxGeometryOutputVertices)) {
            const char *vuid =
                pipeline ? "VUID-VkPipelineShaderStageCreateInfo-stage-00714" : "VUID-VkShaderCreateInfoEXT-pCode-08454";
            skip |= LogError(vuid, module_state.handle(), loc,
                             "SPIR-V (Geometry stage) entry point must have an OpExecutionMode instruction that "
                             "specifies a maximum output vertex count that is greater than 0 and less "
                             "than or equal to maxGeometryOutputVertices.\n"
                             "OutputVertices = %" PRIu32 "\nmaxGeometryOutputVertices = %" PRIu32 "\n",
                             vertices_out, phys_dev_props.limits.maxGeometryOutputVertices);
        }

        if (invocations == 0 || invocations > phys_dev_props.limits.maxGeometryShaderInvocations) {
            const char *vuid =
                pipeline ? "VUID-VkPipelineShaderStageCreateInfo-stage-00715" : "VUID-VkShaderCreateInfoEXT-pCode-08455";
            skip |= LogError(vuid, module_state.handle(), loc,
                             "SPIR-V (Geometry stage) entry point must have an OpExecutionMode instruction that "
                             "specifies an invocation count that is greater than 0 and less "
                             "than or equal to maxGeometryShaderInvocations.\n"
                             "Invocations = %" PRIu32 "\nmaxGeometryShaderInvocations = %" PRIu32 "\n",
                             invocations, phys_dev_props.limits.maxGeometryShaderInvocations);
        }
    } else if (entrypoint.stage == VK_SHADER_STAGE_FRAGMENT_BIT &&
               entrypoint.execution_mode.Has(spirv::ExecutionModeSet::early_fragment_test_bit)) {
        if (pipeline) {
            const auto *ds_state = pipeline->DepthStencilState();
            if ((ds_state &&
                 (ds_state->flags &
                  (VK_PIPELINE_DEPTH_STENCIL_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_DEPTH_ACCESS_BIT_EXT |
                   VK_PIPELINE_DEPTH_STENCIL_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_STENCIL_ACCESS_BIT_EXT)) != 0)) {
                skip |= LogError(
                    "VUID-VkGraphicsPipelineCreateInfo-flags-06591", module_state.handle(), loc,
                    "SPIR-V (Fragment stage) enables early fragment tests, but VkPipelineDepthStencilStateCreateInfo::flags == "
                    "%s.",
                    string_VkPipelineDepthStencilStateCreateFlags(ds_state->flags).c_str());
            }
        }
    }

    return skip;
}

bool CoreChecks::ValidatePointSizeShaderState(const spirv::Module &module_state, const spirv::EntryPoint &entrypoint,
                                              const vvl::Pipeline &pipeline, VkShaderStageFlagBits stage,
                                              const Location &loc) const {
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
        if (enabled_features.shaderTessellationAndGeometryPointSize && !entrypoint.written_builtin_point_size &&
            entrypoint.emit_vertex_geometry && !maintenance5) {
            skip |= LogError(
                "VUID-VkGraphicsPipelineCreateInfo-shaderTessellationAndGeometryPointSize-08776", module_state.handle(), loc,
                "SPIR-V (Geometry stage) PointSize is not written, but shaderTessellationAndGeometryPointSize was enabled.");
        } else if (!enabled_features.shaderTessellationAndGeometryPointSize && entrypoint.written_builtin_point_size) {
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-Geometry-07726", module_state.handle(), loc,
                             "SPIR-V (Geometry stage) PointSize is written to, but shaderTessellationAndGeometryPointSize was not "
                             "enabled (gl_PointSize must NOT be written and a default of 1.0 is assumed).");
        }
    } else if (stage == VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT &&
               ((pipeline.create_info_shaders & VK_SHADER_STAGE_GEOMETRY_BIT) == 0) && point_mode) {
        if (enabled_features.shaderTessellationAndGeometryPointSize && !entrypoint.written_builtin_point_size && !maintenance5) {
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-TessellationEvaluation-07723", module_state.handle(), loc,
                             "SPIR-V (Tessellation Evaluation stage) PointSize is not written, but "
                             "shaderTessellationAndGeometryPointSize was enabled.");
        } else if (!enabled_features.shaderTessellationAndGeometryPointSize && entrypoint.written_builtin_point_size) {
            skip |=
                LogError("VUID-VkGraphicsPipelineCreateInfo-TessellationEvaluation-07724", module_state.handle(), loc,
                         "SPIR-V (Tessellation Evaluation stage) PointSize is written to, shaderTessellationAndGeometryPointSize "
                         "was not enabled (gl_PointSize must NOT be written and a default of 1.0 is assumed).");
        }
    } else if (stage == VK_SHADER_STAGE_VERTEX_BIT &&
               ((pipeline.create_info_shaders & (VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT | VK_SHADER_STAGE_GEOMETRY_BIT)) ==
                0)) {
        if (!entrypoint.written_builtin_point_size && IsPointTopology(pipeline.topology_at_rasterizer) && !maintenance5) {
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-topology-08773", module_state.handle(), loc,
                             "SPIR-V (Vertex) PointSize is not written to, but Pipeline topology is set to "
                             "VK_PRIMITIVE_TOPOLOGY_POINT_LIST.");
        }
    }

    return skip;
}

bool CoreChecks::ValidatePrimitiveRateShaderState(const spirv::Module &module_state, const spirv::EntryPoint &entrypoint,
                                                  const vvl::Pipeline &pipeline, VkShaderStageFlagBits stage,
                                                  const Location &loc) const {
    bool skip = false;

    const auto viewport_state = pipeline.ViewportState();
    if (!phys_dev_ext_props.fragment_shading_rate_props.primitiveFragmentShadingRateWithMultipleViewports &&
        (pipeline.pipeline_type == VK_PIPELINE_BIND_POINT_GRAPHICS) && viewport_state) {
        if (!pipeline.IsDynamic(CB_DYNAMIC_STATE_VIEWPORT_WITH_COUNT) && viewport_state->viewportCount > 1 &&
            entrypoint.written_builtin_primitive_shading_rate_khr) {
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-primitiveFragmentShadingRateWithMultipleViewports-04503",
                             module_state.handle(), loc,
                             "SPIR-V (%s) statically writes to PrimitiveShadingRateKHR built-in, but "
                             "multiple viewports "
                             "are used and the primitiveFragmentShadingRateWithMultipleViewports limit is not supported.",
                             string_VkShaderStageFlagBits(stage));
        }

        if (entrypoint.written_builtin_primitive_shading_rate_khr && entrypoint.written_builtin_viewport_index) {
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-primitiveFragmentShadingRateWithMultipleViewports-04504",
                             module_state.handle(), loc,
                             "SPIR-V (%s) statically writes to both PrimitiveShadingRateKHR and "
                             "ViewportIndex built-ins, "
                             "but the primitiveFragmentShadingRateWithMultipleViewports limit is not supported.",
                             string_VkShaderStageFlagBits(stage));
        }

        if (entrypoint.written_builtin_primitive_shading_rate_khr && entrypoint.written_builtin_viewport_mask_nv) {
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-primitiveFragmentShadingRateWithMultipleViewports-04505",
                             module_state.handle(), loc,
                             "SPIR-V (%s) statically writes to both PrimitiveShadingRateKHR and "
                             "ViewportMaskNV built-ins, "
                             "but the primitiveFragmentShadingRateWithMultipleViewports limit is not supported.",
                             string_VkShaderStageFlagBits(stage));
        }
    }
    return skip;
}

bool CoreChecks::ValidateWorkgroupSharedMemory(const spirv::Module &module_state, VkShaderStageFlagBits stage,
                                               uint32_t total_workgroup_shared_memory, const Location &loc) const {
    bool skip = false;

    switch (stage) {
        case VK_SHADER_STAGE_COMPUTE_BIT: {
            if (total_workgroup_shared_memory > phys_dev_props.limits.maxComputeSharedMemorySize) {
                skip |= LogError("VUID-RuntimeSpirv-Workgroup-06530", module_state.handle(), loc,
                                 "SPIR-V uses %" PRIu32
                                 " bytes of shared memory, which is more than maxComputeSharedMemorySize (%" PRIu32 ").",
                                 total_workgroup_shared_memory, phys_dev_props.limits.maxComputeSharedMemorySize);
            }

            if (enabled_features.cooperativeMatrixWorkgroupScope) {
                for (auto &cooperative_matrix_inst : module_state.static_data_.cooperative_matrix_inst) {
                    if (cooperative_matrix_inst->Opcode() != spv::OpTypeCooperativeMatrixKHR) {
                        continue;
                    }
                    auto scope = module_state.GetConstantDef(cooperative_matrix_inst->Word(3));
                    if (!scope || scope->GetConstantValue() != VK_SCOPE_WORKGROUP_KHR) {
                        continue;
                    }
                    if (total_workgroup_shared_memory >
                        phys_dev_props.limits.maxComputeSharedMemorySize -
                            phys_dev_ext_props.cooperative_matrix_props2_nv.cooperativeMatrixWorkgroupScopeReservedSharedMemory) {
                        skip |= LogError(
                            "VUID-RuntimeSpirv-maxComputeSharedMemorySize-10168", module_state.handle(), loc,
                            "SPIR-V uses %" PRIu32
                            " bytes of shared memory, which is more than maxComputeSharedMemorySize (%" PRIu32
                            ") minus "
                            "cooperativeMatrixWorkgroupScopeReservedSharedMemory (%" PRIu32 ").",
                            total_workgroup_shared_memory, phys_dev_props.limits.maxComputeSharedMemorySize,
                            phys_dev_ext_props.cooperative_matrix_props2_nv.cooperativeMatrixWorkgroupScopeReservedSharedMemory);
                        break;
                    }
                }
            }

            break;
        }
        case VK_SHADER_STAGE_MESH_BIT_EXT: {
            if (total_workgroup_shared_memory > phys_dev_ext_props.mesh_shader_props_ext.maxMeshSharedMemorySize) {
                skip |= LogError("VUID-RuntimeSpirv-maxMeshSharedMemorySize-08754", module_state.handle(), loc,
                                 "SPIR-V uses %" PRIu32
                                 " bytes of shared memory, which is more than maxMeshSharedMemorySize (%" PRIu32 ").",
                                 total_workgroup_shared_memory, phys_dev_ext_props.mesh_shader_props_ext.maxMeshSharedMemorySize);
            }
            break;
        }
        case VK_SHADER_STAGE_TASK_BIT_EXT: {
            if (total_workgroup_shared_memory > phys_dev_ext_props.mesh_shader_props_ext.maxTaskSharedMemorySize) {
                skip |= LogError("VUID-RuntimeSpirv-maxTaskSharedMemorySize-08759", module_state.handle(), loc,
                                 "SPIR-V uses %" PRIu32
                                 " bytes of shared memory, which is more than maxTaskSharedMemorySize (%" PRIu32 ").",
                                 total_workgroup_shared_memory, phys_dev_ext_props.mesh_shader_props_ext.maxTaskSharedMemorySize);
            }
            break;
        }
        default:
            assert(false);  // other stages should not have called this function
            break;
    }

    return skip;
}

bool CoreChecks::ValidateShaderInterfaceVariable(const spirv::Module &module_state,
                                                 const spirv::ResourceInterfaceVariable &variable,
                                                 vvl::unordered_set<uint32_t> &descriptor_type_set, const Location &loc) const {
    bool skip = false;

    if ((variable.is_storage_image || variable.is_storage_texel_buffer || variable.is_storage_buffer) &&
        !variable.decorations.Has(spirv::DecorationSet::nonwritable_bit)) {
        // If the variable is a struct, all members must contain NonWritable
        if (!variable.type_struct_info ||
            !variable.type_struct_info->decorations.AllMemberHave(spirv::DecorationSet::nonwritable_bit)) {
            switch (variable.stage) {
                case VK_SHADER_STAGE_FRAGMENT_BIT:
                    if (!enabled_features.fragmentStoresAndAtomics) {
                        skip |=
                            LogError("VUID-RuntimeSpirv-NonWritable-06340", module_state.handle(), loc,
                                     "SPIR-V (VK_SHADER_STAGE_FRAGMENT_BIT) uses descriptor %s (type %s) which is not "
                                     "marked with NonWritable, but fragmentStoresAndAtomics was not enabled.",
                                     variable.DescribeDescriptor().c_str(), string_DescriptorTypeSet(descriptor_type_set).c_str());
                    }
                    break;
                case VK_SHADER_STAGE_VERTEX_BIT:
                case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
                case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
                case VK_SHADER_STAGE_GEOMETRY_BIT:
                    if (!enabled_features.vertexPipelineStoresAndAtomics) {
                        skip |= LogError("VUID-RuntimeSpirv-NonWritable-06341", module_state.handle(), loc,
                                         "SPIR-V (%s) uses descriptor %s (type %s) which is not marked with NonWritable, but "
                                         "vertexPipelineStoresAndAtomics was not enabled.",
                                         string_VkShaderStageFlagBits(variable.stage), variable.DescribeDescriptor().c_str(),
                                         string_DescriptorTypeSet(descriptor_type_set).c_str());
                    }
                    break;
                default:
                    // No feature requirements for writes and atomics for other stages
                    break;
            }
        }
    }

    if (!variable.decorations.Has(spirv::DecorationSet::input_attachment_bit) && variable.info.image_dim == spv::DimSubpassData) {
        if (variable.array_length != 0) {
            skip |= LogError("VUID-RuntimeSpirv-OpTypeImage-09644", module_state.handle(), loc,
                             "the variable is an array of OpTypeImage with Dim::SubpassData, but it is missing the "
                             "InputAttachmentIndex decoration.\n%s\n",
                             variable.base_type.Describe().c_str());
        } else if (!enabled_features.dynamicRenderingLocalRead) {
            skip |= LogError("VUID-RuntimeSpirv-None-09558", module_state.handle(), loc,
                             "the variable is a OpTypeImage with Dim::SubpassData, but it is missing the "
                             "InputAttachmentIndex decoration (dynamicRenderingLocalRead was not enabled).\n%s\n",
                             variable.base_type.Describe().c_str());
        }
    }

    if (variable.is_uniform_buffer && variable.type_struct_info && variable.type_struct_info->has_runtime_array &&
        !enabled_features.shaderUniformBufferUnsizedArray) {
        skip |= LogError("VUID-RuntimeSpirv-shaderUniformBufferUnsizedArray-11806", module_state.handle(), loc,
                         "SPIR-V (%s) uses descriptor %s which is an uniform buffer with a runtime array, but "
                         "shaderUniformBufferUnsizedArray was not enabled.",
                         string_VkShaderStageFlagBits(variable.stage), variable.DescribeDescriptor().c_str());
    }

    return skip;
}

bool CoreChecks::ValidateShaderInterfaceVariableDSL(const spirv::Module &module_state, const spirv::EntryPoint &entrypoint,
                                                    const ShaderStageState &stage_state,
                                                    const spirv::ResourceInterfaceVariable &variable,
                                                    vvl::unordered_set<uint32_t> &descriptor_type_set, const Location &loc) const {
    bool skip = false;
    if (!stage_state.descriptor_set_layouts) {
        return skip;
    }

    LogObjectList objlist(module_state.handle());

    const VkDescriptorSetLayoutBinding *binding = nullptr;

    const vvl::DescriptorSetLayout *descriptor_set_layout = stage_state.descriptor_set_layouts->FindFromVariable(variable);
    if (descriptor_set_layout) {
        objlist.add(descriptor_set_layout->Handle());
        binding = descriptor_set_layout->GetDescriptorSetLayoutBindingPtrFromBinding(variable.decorations.binding);
    }

    if (binding) {
        skip |= ValidateShaderYcbcrSampler(module_state, entrypoint, *stage_state.descriptor_set_layouts, *descriptor_set_layout,
                                           *binding, variable, objlist, loc);
    }

    auto print_dsl_info = [&stage_state, &variable]() {
        std::stringstream ss;
        const bool has_pipeline = stage_state.shader_object_create_info == nullptr;
        if (has_pipeline) {
            ss << "VkPipelineLayoutCreateInfo::pSetLayouts[" << variable.decorations.set << "]";
        } else {
            ss << "VkShaderCreateInfoEXT::pSetLayouts[" << variable.decorations.set << "]";
        }
        return ss.str();
    };

    if (!binding) {
        skip |= LogError(GetSpirvInterfaceVariableVUID(loc, vvl::SpirvInterfaceVariableError::ShaderStage_07988), objlist, loc,
                         "SPIR-V (%s) uses descriptor %s (type %s) but was not declared in the %s.",
                         string_VkShaderStageFlagBits(variable.stage), variable.DescribeDescriptor().c_str(),
                         string_DescriptorTypeSet(descriptor_type_set).c_str(), print_dsl_info().c_str());
    } else if (~binding->stageFlags & variable.stage) {
        skip |=
            LogError(GetSpirvInterfaceVariableVUID(loc, vvl::SpirvInterfaceVariableError::ShaderStage_07988), objlist, loc,
                     "SPIR-V (%s) uses descriptor %s (type %s) but the VkDescriptorSetLayoutBinding::stageFlags was %s. (from %s)",
                     string_VkShaderStageFlagBits(variable.stage), variable.DescribeDescriptor().c_str(),
                     string_DescriptorTypeSet(descriptor_type_set).c_str(), string_VkShaderStageFlags(binding->stageFlags).c_str(),
                     print_dsl_info().c_str());
    } else if ((binding->descriptorType != VK_DESCRIPTOR_TYPE_MUTABLE_EXT) &&
               (descriptor_type_set.find(binding->descriptorType) == descriptor_type_set.end())) {
        skip |= LogError(GetSpirvInterfaceVariableVUID(loc, vvl::SpirvInterfaceVariableError::Mutable_07990), objlist, loc,
                         "SPIR-V (%s) uses descriptor %s of type %s but expected %s. (from %s)",
                         string_VkShaderStageFlagBits(variable.stage), variable.DescribeDescriptor().c_str(),
                         string_VkDescriptorType(binding->descriptorType), string_DescriptorTypeSet(descriptor_type_set).c_str(),
                         print_dsl_info().c_str());
    } else if (binding->descriptorType == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK && variable.array_length) {
        skip |=
            LogError(GetSpirvInterfaceVariableVUID(loc, vvl::SpirvInterfaceVariableError::Inline_10391), objlist, loc,
                     "SPIR-V (%s) uses descriptor %s as VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK, but it is an array of descriptor. "
                     "(from %s)",
                     string_VkShaderStageFlagBits(variable.stage), variable.DescribeDescriptor().c_str(), print_dsl_info().c_str());

    } else if (binding->descriptorCount < variable.array_length && variable.array_length != spirv::kRuntimeArray) {
        skip |= LogError(GetSpirvInterfaceVariableVUID(loc, vvl::SpirvInterfaceVariableError::DescriptorCount_07991), objlist, loc,
                         "SPIR-V (%s) uses descriptor %s with a VkDescriptorSetLayoutBinding::descriptorCount of %" PRIu32
                         ", but requires at least %" PRIu32 " in the SPIR-V. (from %s)",
                         string_VkShaderStageFlagBits(variable.stage), variable.DescribeDescriptor().c_str(),
                         binding->descriptorCount, variable.array_length, print_dsl_info().c_str());
    } else if (binding->descriptorCount == 0 && variable.array_length == spirv::kRuntimeArray) {
        skip |=
            LogError(GetSpirvInterfaceVariableVUID(loc, vvl::SpirvInterfaceVariableError::DescriptorCount_07991), objlist, loc,
                     "SPIR-V (%s) uses a runtime descriptor array %s with a VkDescriptorSetLayoutBinding::descriptorCount of 0 "
                     "but requires at least 1 descriptor. (from %s)",
                     string_VkShaderStageFlagBits(variable.stage), variable.DescribeDescriptor().c_str(), print_dsl_info().c_str());
    }

    return skip;
}

// "friends don't let friends validate YCbCr in SPIR-V" ~Spencer
bool CoreChecks::ValidateShaderYcbcrSampler(const spirv::Module &module_state, const spirv::EntryPoint &entrypoint,
                                            const vvl::DescriptorSetLayoutList &all_dsl,
                                            const vvl::DescriptorSetLayout &descriptor_set_layout,
                                            const VkDescriptorSetLayoutBinding &binding,
                                            const spirv::ResourceInterfaceVariable &variable, const LogObjectList &objlist,
                                            const Location &loc) const {
    bool skip = false;

    // Need to check every DescriptorSetLayout as you can have 2 sets for the SAMPLED_IMAGE and SAMPLER
    //
    // IsAccessed() will prevent things like textureSize() from be marked as a false positive.
    // Note that for YCbCr, OpImageQueryLod will query the sampler, but OpImageQuerySize only queries
    // the image and therefor can still be used with YCbCr.
    const bool possible_ycbcr = all_dsl.HasYcbcrSamplers() && (variable.IsImage() && variable.IsImageAccessed());
    if (!possible_ycbcr) {
        return skip;
    }

    if (variable.is_type_sampled_image) {
        // simple case if using combined image sampler
        if (binding.pImmutableSamplers) {
            skip |= ValidateShaderYcbcrSamplerAccess(descriptor_set_layout, binding, variable, nullptr, nullptr, objlist, loc);
        }
    } else {
        // otherwise we need to search for each sampler variable used with this image access
        for (const YcbcrSamplerUsedByImage &sampler_info : variable.ycbcr_samplers_used_by_image) {
            const spirv::ResourceInterfaceVariable *sampler_variable =
                entrypoint.resource_interface_variable_map.at(sampler_info.variable_id);
            ASSERT_AND_CONTINUE(sampler_variable);

            const vvl::DescriptorSetLayout *sampler_dsl = all_dsl.FindFromVariable(*sampler_variable);
            ASSERT_AND_CONTINUE(sampler_dsl);
            const VkDescriptorSetLayoutBinding *sampler_binding =
                sampler_dsl->GetDescriptorSetLayoutBindingPtrFromBinding(sampler_variable->decorations.binding);
            ASSERT_AND_CONTINUE(sampler_binding);
            if (sampler_binding->pImmutableSamplers) {
                skip |= ValidateShaderYcbcrSamplerAccess(*sampler_dsl, *sampler_binding, variable, sampler_variable, &sampler_info,
                                                         objlist, loc);
            }
        }
    }

    return skip;
}

bool CoreChecks::ValidateShaderYcbcrSamplerAccess(const vvl::DescriptorSetLayout &descriptor_set_layout,
                                                  const VkDescriptorSetLayoutBinding &binding,
                                                  const spirv::ResourceInterfaceVariable &image_variable,
                                                  const spirv::ResourceInterfaceVariable *sampler_variable,
                                                  const YcbcrSamplerUsedByImage *sampler_info, const LogObjectList &objlist,
                                                  const Location &loc) const {
    bool skip = false;

    auto print_ycbcr_sampler = [&image_variable, &sampler_variable]() {
        std::stringstream ss;
        ss << image_variable.DescribeDescriptor();
        if (sampler_variable) {
            ss << " (sampled with " << sampler_variable->DescribeDescriptor() << ")";
        } else {
            ss << " (a combined image sampler)";
        }
        return ss.str();
    };

    // The sampler state might have been destroyed, we need to get the safe struct we saved
    const uint32_t index = descriptor_set_layout.GetIndexFromBinding(binding.binding);
    const std::vector<vku::safe_VkSamplerCreateInfo> &sampler_create_infos =
        descriptor_set_layout.GetImmutableSamplerCreateInfosFromIndex(index);
    for (uint32_t i = 0; i < sampler_create_infos.size(); i++) {
        auto *conversion_info = vku::FindStructInPNextChain<VkSamplerYcbcrConversionInfo>(sampler_create_infos[i].pNext);
        if (!conversion_info || conversion_info->conversion == VK_NULL_HANDLE) {
            continue;
        }

        if (!image_variable.info.image_insn.is_sampler_sampled) {
            skip |= LogError("VUID-RuntimeSpirv-None-10716", objlist, loc,
                             "%s points to pImmutableSamplers[%" PRIu32
                             "] (%s) that was created with a VkSamplerYcbcrConversion, but was accessed in the SPIR-V "
                             "with a non OpImage*Sample* instruction.\nNon-sampled operations (like texelFetch) can't be used "
                             "because it doesn't contain the sampler YCbCr conversion information for the driver.",
                             print_ycbcr_sampler().c_str(), i, FormatHandle(conversion_info->conversion).c_str());
            break;  // only need to report a single descriptor
        } else if (image_variable.info.image_insn.is_sampler_offset) {
            skip |= LogError("VUID-RuntimeSpirv-ConstOffset-10718", objlist, loc,
                             "%s points to pImmutableSamplers[%" PRIu32
                             "] (%s) that was created with a VkSamplerYcbcrConversion, but was accessed in the SPIR-V "
                             "with ConstOffset/Offset image operands.",
                             print_ycbcr_sampler().c_str(), i, FormatHandle(conversion_info->conversion).c_str());
            break;  // only need to report a single descriptor
        }

        // When dealing with COMBINED_IMAGE_SAMPLER it is easy because you just check how the array is accessed at anytime
        const bool dynamic_access_combined = (!sampler_variable && !image_variable.all_constant_integral_expressions);
        // For SAMPLER/SAMPLED_IMAGE if the sampler is an array, we must always checkc
        // https://gitlab.khronos.org/vulkan/vulkan/-/issues/4521
        const bool dynamic_access_sampler = (sampler_variable && sampler_variable->array_length != 0 &&
                                             sampler_info->sampler_access_chain_index == spirv::kInvalidValue);
        // indexed, but only when not tied to a YCbCr sampler see test PositiveYcbcr.ArrayOfImagesSometimesUsedForYcbcr
        const bool dynamic_access_image = (sampler_variable && !image_variable.all_constant_integral_expressions &&
                                           sampler_info->image_access_chain_index == spirv::kInvalidValue);

        if (dynamic_access_combined || dynamic_access_sampler || dynamic_access_image) {
            std::stringstream ss;
            if (dynamic_access_combined) {
                ss << image_variable.DescribeDescriptor()
                   << " is an COMBINED_SAMPLED_IMAGE tied to an array of YCbCr samplers and it trying to be accessed with a "
                      "non-constant index value.";
            } else {
                ss << sampler_variable->DescribeDescriptor() << " is a YCbCr sampler ";
                if (dynamic_access_image) {
                    ss << "being accessed with " << image_variable.DescribeDescriptor()
                       << " but this SAMPLED_IMAGE is an array accessed with an non-constant index value.";
                } else {
                    ss << "array that is being accessed with an non-constant index value (with image "
                       << image_variable.DescribeDescriptor() << ")";
                }
            }
            ss << "\nRegardless if it is uniform or not, you can't dynamically index into an array of YCbCr samplers (or images) "
                  "in your shader and you need a constant value.\nThis is because the driver's compiler needs to know the exact "
                  "YCbCr sampler/image being used in order to inject special instructions into the final shader.\nOne possible "
                  "workaround is to use Specialization Constant to decide the index at pipeline/shaderObject creation time.";
            skip |= LogError("VUID-RuntimeSpirv-None-10715", objlist, loc, "%s", ss.str().c_str());

            // only need to report a single descriptor
            // If we hook up to print the ShaderDebugInfo, might be worth it to print all of them
            break;
        }
    }
    return skip;
}

bool CoreChecks::ValidateTransformFeedbackPipeline(const spirv::Module &module_state, const spirv::EntryPoint &entrypoint,
                                                   const vvl::Pipeline &pipeline, const Location &loc) const {
    bool skip = false;

    const bool is_xfb_execution_mode = entrypoint.execution_mode.Has(spirv::ExecutionModeSet::xfb_bit);
    if (is_xfb_execution_mode) {
        if ((pipeline.create_info_shaders & (VK_SHADER_STAGE_MESH_BIT_EXT | VK_SHADER_STAGE_TASK_BIT_EXT)) != 0) {
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-None-02322", module_state.handle(), loc,
                             "SPIR-V has OpExecutionMode of Xfb and using mesh shaders (%s).",
                             string_VkShaderStageFlags(pipeline.create_info_shaders).c_str());
        }

        if (pipeline.pre_raster_state) {
            if (entrypoint.stage != pipeline.pre_raster_state->last_stage) {
                skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-pStages-02318", module_state.handle(), loc,
                                 "SPIR-V has OpExecutionMode of Xfb in %s, but %s is the last last pre-rasterization shader stage.",
                                 string_VkShaderStageFlagBits(entrypoint.stage),
                                 string_VkShaderStageFlagBits(pipeline.pre_raster_state->last_stage));
            }
            if ((pipeline.create_flags & VK_PIPELINE_CREATE_2_INDIRECT_BINDABLE_BIT_EXT) != 0) {
                skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-flags-11001", module_state.handle(), loc,
                                 "SPIR-V has OpExecutionMode of Xfb but this pipeline is being created with "
                                 "VK_PIPELINE_CREATE_2_INDIRECT_BINDABLE_BIT_EXT.");
            }
        }
    }

    if (pipeline.pre_raster_state && (pipeline.create_info_shaders & VK_SHADER_STAGE_GEOMETRY_BIT) != 0 &&
        module_state.HasCapability(spv::CapabilityGeometryStreams) && !enabled_features.geometryStreams) {
        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-geometryStreams-02321", module_state.handle(), loc,
                         "SPIR-V uses GeometryStreams capability, but "
                         "VkPhysicalDeviceTransformFeedbackFeaturesEXT::geometryStreams is not enabled.");
    }
    return skip;
}

bool CoreChecks::ValidateImageWrite(const spirv::Module &module_state, const Location &loc) const {
    bool skip = false;
    for (const auto &[insn, load_id] : module_state.static_data_.image_write_load_id_map) {
        // guaranteed by spirv-val to be an OpTypeImage
        const uint32_t image = module_state.GetTypeId(load_id);
        const spirv::Instruction *image_def = module_state.FindDef(image);
        const uint32_t image_format = image_def->Word(8);
        // If format is 'Unknown' then need to wait until a descriptor is bound to it
        if (image_format != spv::ImageFormatUnknown) {
            const VkFormat compatible_format = CompatibleSpirvImageFormat(image_format);
            if (compatible_format != VK_FORMAT_UNDEFINED) {
                const uint32_t format_component_count = vkuFormatComponentCount(compatible_format);
                const uint32_t texel_component_count = module_state.GetTexelComponentCount(*insn);
                if (texel_component_count < format_component_count) {
                    skip |= LogError("VUID-RuntimeSpirv-OpImageWrite-07112", module_state.handle(), loc,
                                     "SPIR-V OpImageWrite Texel operand only contains %" PRIu32
                                     " components, but the OpImage format mapping to %s has %" PRIu32 " components.\n%s\n%s\n",
                                     texel_component_count, string_VkFormat(compatible_format), format_component_count,
                                     module_state.DescribeInstruction(*insn).c_str(),
                                     module_state.DescribeInstruction(*image_def).c_str());
                }
            }
        }
    }
    return skip;
}

static const std::string GetShaderTileImageCapabilitiesString(const spirv::Module &module_state) {
    struct SpvCapabilityWithString {
        const spv::Capability cap;
        const std::string cap_string;
    };

    // Shader tile image capabilities
    static const std::array<SpvCapabilityWithString, 3> shader_tile_image_capabilities = {
        {{spv::CapabilityTileImageColorReadAccessEXT, "TileImageColorReadAccessEXT"},
         {spv::CapabilityTileImageDepthReadAccessEXT, "TileImageDepthReadAccessEXT"},
         {spv::CapabilityTileImageStencilReadAccessEXT, "TileImageStencilReadAccessEXT"}}};

    std::stringstream ss_capabilities;
    for (auto spv_capability : shader_tile_image_capabilities) {
        if (module_state.HasCapability(spv_capability.cap)) {
            if (ss_capabilities.tellp()) ss_capabilities << ", ";
            ss_capabilities << spv_capability.cap_string;
        }
    }

    return ss_capabilities.str();
}

bool CoreChecks::ValidateShaderTileImage(const spirv::Module &module_state, const spirv::EntryPoint &entrypoint,
                                         const vvl::Pipeline &pipeline, const Location &loc) const {
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
                         "SPIR-V (Fragment stage) is using capabilities (%s), but renderpass (%s) is not VK_NULL_HANDLE.",
                         GetShaderTileImageCapabilitiesString(module_state).c_str(), FormatHandle(rp).c_str());
    }

    const bool mode_early_fragment_test = entrypoint.execution_mode.Has(spirv::ExecutionModeSet::early_fragment_test_bit);
    if (module_state.static_data_.has_shader_tile_image_depth_read) {
        const auto *ds_state = pipeline.DepthStencilState();
        const bool write_enabled =
            !pipeline.IsDynamic(CB_DYNAMIC_STATE_DEPTH_WRITE_ENABLE) && (ds_state && ds_state->depthWriteEnable);
        if (mode_early_fragment_test && write_enabled) {
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-pStages-08711", module_state.handle(), loc,
                             "SPIR-V (Fragment stage) contains OpDepthAttachmentReadEXT, and depthWriteEnable is not false.");
        }
    }

    if (module_state.static_data_.has_shader_tile_image_stencil_read) {
        const auto *ds_state = pipeline.DepthStencilState();
        const bool is_write_mask_set = !pipeline.IsDynamic(CB_DYNAMIC_STATE_STENCIL_WRITE_MASK) &&
                                       (ds_state && (ds_state->front.writeMask != 0 || ds_state->back.writeMask != 0));
        if (mode_early_fragment_test && is_write_mask_set) {
            skip |= LogError(
                "VUID-VkGraphicsPipelineCreateInfo-pStages-08712", module_state.handle(), loc,
                "SPIR-V (Fragment stage) contains OpStencilAttachmentReadEXT, and stencil write mask is not equal to 0 for "
                "both front(%" PRIu32 ") and back (%" PRIu32 ").",
                ds_state->front.writeMask, ds_state->back.writeMask);
        }
    }

    bool using_tile_image_op = module_state.static_data_.has_shader_tile_image_depth_read ||
                               module_state.static_data_.has_shader_tile_image_stencil_read ||
                               module_state.static_data_.has_shader_tile_image_color_read;
    const auto *ms_state = pipeline.MultisampleState();
    if (using_tile_image_op && ms_state && ms_state->sampleShadingEnable && (ms_state->minSampleShading != 1.0)) {
        skip |= LogError("VUID-RuntimeSpirv-minSampleShading-08732", module_state.handle(), loc,
                         "minSampleShading (%f) is not equal to 1.0.", ms_state->minSampleShading);
    }

    return skip;
}

// Validate the VkPipelineShaderStageCreateInfo from the various pipeline types or a Shader Object
bool CoreChecks::ValidateShaderStage(const ShaderStageState &stage_state, const vvl::Pipeline *pipeline,
                                     const Location &loc) const {
    bool skip = false;
    const VkShaderStageFlagBits stage = stage_state.GetStage();

    // First validate all things that don't require valid SPIR-V
    // this is found when using VK_EXT_shader_module_identifier
    skip |= ValidateSpecializations(stage_state.GetSpecializationInfo(), loc.dot(Field::pSpecializationInfo));
    if (pipeline) {
        skip |= ValidateShaderStageMaxResources(stage, *pipeline, loc);
        if (const auto *pipeline_robustness_info =
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

    const spirv::Module &module_state = *stage_state.spirv_state.get();
    if (!module_state.valid_spirv) {
        return skip;  // checked elsewhere
    }

    if (!stage_state.entrypoint) {
        const char *vuid = pipeline ? "VUID-VkPipelineShaderStageCreateInfo-pName-00707" : "VUID-VkShaderCreateInfoEXT-pName-08440";
        std::stringstream err;
        err << "\"" << stage_state.GetPName() << "\" entry point not found for stage " << string_VkShaderStageFlagBits(stage)
            << ".";
        if (stage_state.spirv_state->static_data_.entry_points.size() == 1) {
            auto entry_point = stage_state.spirv_state->static_data_.entry_points[0];
            if (entry_point) {
                if (entry_point->stage != stage) {
                    err << " (Seems like you accidently created your SPIR-V with "
                        << string_VkShaderStageFlagBits(entry_point->stage) << " so the entry point is not matching up)";
                } else {
                    err << " (The only entry point found was \"" << entry_point->name << "\" for "
                        << string_VkShaderStageFlagBits(entry_point->stage) << ")";
                    if (entry_point->name == "main") {
                        err << "\nSome shading languages will let you name the main function something else, but when "
                               "compiled to SPIR-V, it will keep it as 'main' to match defaults found in other shading langauges "
                               "such "
                               "as GLSL. It is also valid in a single SPIR-V binary to have 'main' for two different stages.";
                    }
                }
            }
        } else {
            err << " The following entry points were found in the SPIR-V module:\n";
            for (const auto &entry_point : stage_state.spirv_state->static_data_.entry_points) {
                if (!entry_point) continue;
                err << "\"" << entry_point->name << "\"\t(" << string_VkShaderStageFlagBits(entry_point->stage) << ")\n";
            }
        }
        return LogError(vuid, device, loc.dot(Field::pName), "%s", err.str().c_str());
    }
    const spirv::EntryPoint &entrypoint = *stage_state.entrypoint;

    // to prevent const_cast on pipeline object, just store here as not needed outside function anyway
    spirv::LocalSize local_size;
    uint32_t total_workgroup_shared_memory = 0;
    uint32_t total_task_payload_memory = 0;

    // If specialization-constant instructions are present in the shader, the specializations should be applied.
    if (module_state.static_data_.has_specialization_constants && global_settings.spirv_const_fold) {
        // setup the call back if the optimizer fails
        spvtools::Optimizer optimizer(spirv_environment);
        spvtools::MessageConsumer consumer = [&skip, &module_state, &stage, loc, this](
                                                 spv_message_level_t level, const char *source, const spv_position_t &position,
                                                 const char *message) {
            skip |= LogError("VUID-VkPipelineShaderStageCreateInfo-module-parameter", device, loc,
                             "%s failed in spirv-opt because it does not contain valid spirv for stage %s. %s",
                             FormatHandle(module_state.handle()).c_str(), string_VkShaderStageFlagBits(stage), message);
        };
        optimizer.SetMessageConsumer(consumer);

        // The app might be using the default spec constant values, but if they pass values at runtime to the pipeline then need to
        // use those values to apply to the spec constants
        auto const &specialization_info = stage_state.GetSpecializationInfo();
        if (specialization_info != nullptr && specialization_info->mapEntryCount > 0 &&
            specialization_info->pMapEntries != nullptr) {
            // Gather the specialization-constant values.
            auto const &specialization_data = reinterpret_cast<uint8_t const *>(specialization_info->pData);
            std::unordered_map<uint32_t, std::vector<uint32_t>> id_value_map;  // note: this must be std:: to work with spvtools
            id_value_map.reserve(specialization_info->mapEntryCount);

            // spirv-val makes sure every OpSpecConstant has a OpDecoration.
            for (const auto &[result_id, spec_id] : module_state.static_data_.id_to_spec_id) {
                VkSpecializationMapEntry map_entry = {spirv::kInvalidValue, 0, 0};
                for (uint32_t i = 0; i < specialization_info->mapEntryCount; i++) {
                    if (specialization_info->pMapEntries[i].constantID == spec_id) {
                        map_entry = specialization_info->pMapEntries[i];
                        break;
                    }
                }

                // "If a constantID value is not a specialization constant ID used in the shader, that map entry does not affect the
                // behavior of the pipeline."
                if (map_entry.constantID == spirv::kInvalidValue) {
                    continue;
                }

                uint32_t spec_const_size = spirv::kInvalidValue;
                const spirv::Instruction *def_insn = module_state.FindDef(result_id);
                const spirv::Instruction *type_insn = module_state.FindDef(def_insn->Word(1));

                // Specialization constants can only be of type bool, scalar integer, or scalar floating point
                switch (type_insn->Opcode()) {
                    case spv::OpTypeBool:
                        // "If the specialization constant is of type boolean, size must be the byte size of VkBool32"
                        spec_const_size = sizeof(VkBool32);
                        break;
                    case spv::OpTypeInt:
                    case spv::OpTypeFloat:
                        spec_const_size = type_insn->Word(2) / 8;
                        break;
                    default:
                        // spirv-val should catch if SpecId is not used on a
                        // OpSpecConstantTrue/OpSpecConstantFalse/OpSpecConstant and OpSpecConstant is validated to be a
                        // OpTypeInt or OpTypeFloat
                        break;
                }

                if (map_entry.size != spec_const_size) {
                    std::stringstream name;
                    if (module_state.handle()) {
                        name << "shader module " << module_state.handle();
                    } else {
                        name << "shader object";
                    }
                    skip |= LogError("VUID-VkSpecializationMapEntry-constantID-00776", device, loc,
                                     "specialization constant (ID = %" PRIu32 ", entry = %" PRIu32
                                     ") has invalid size %zu in %s. Expected size is %" PRIu32 " from shader definition.",
                                     map_entry.constantID, spec_id, map_entry.size, FormatHandle(module_state.handle()).c_str(),
                                     spec_const_size);
                }

                if ((map_entry.offset + map_entry.size) <= specialization_info->dataSize) {
                    // Allocate enough room for ceil(map_entry.size / 4) to store entries
                    std::vector<uint32_t> entry_data((map_entry.size + 4 - 1) / 4, 0);
                    uint8_t *out_p = reinterpret_cast<uint8_t *>(entry_data.data());
                    const uint8_t *const start_in_p = specialization_data + map_entry.offset;
                    const uint8_t *const end_in_p = start_in_p + map_entry.size;

                    std::copy(start_in_p, end_in_p, out_p);
                    id_value_map.emplace(map_entry.constantID, std::move(entry_data));
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
        std::vector<uint32_t> specialized_spirv;
        auto const optimized =
            optimizer.Run(module_state.words_.data(), module_state.words_.size(), &specialized_spirv, spirv_val_options, true);
        if (optimized) {
            spv_context ctx = spvContextCreate(spirv_environment);
            spv_const_binary_t binary{specialized_spirv.data(), specialized_spirv.size()};
            spv_diagnostic diag = nullptr;
            auto const spv_valid = spvValidateWithOptions(ctx, spirv_val_options, &binary, &diag);
            if (spv_valid != SPV_SUCCESS) {
                const char *vuid = pipeline ? "VUID-VkPipelineShaderStageCreateInfo-pSpecializationInfo-06849"
                                            : "VUID-VkShaderCreateInfoEXT-pCode-08460";
                std::string name = pipeline ? FormatHandle(module_state.handle()) : "shader object";
                skip |= LogError(vuid, device, loc,
                                 "After specialization was applied, %s produces a spirv-val error (stage %s):\n%s\nCommand to "
                                 "reproduce:\n\t%s\n",
                                 name.c_str(), string_VkShaderStageFlagBits(stage),
                                 diag && diag->error ? diag->error : "(no error text)", spirv_val_command.c_str());
            }

            // The new optimized SPIR-V will NOT match the original spirv::Module object parsing, so a new spirv::Module
            // object is needed. This an issue due to each pipeline being able to reuse the same shader module but with different
            // spec constant values.
            spirv::Module spec_mod(vvl::make_span<const uint32_t>(specialized_spirv.data(), specialized_spirv.size()));

            // According to https://github.com/KhronosGroup/Vulkan-Docs/issues/1671 anything labeled as "static use" (such as if an
            // input is used or not) don't have to be checked post spec constants freezing since the device compiler is not
            // guaranteed to run things such as dead-code elimination. The following checks are things that don't follow under
            // "static use" rules and need to be validated still.

            const auto spec_entrypoint = spec_mod.FindEntrypoint(entrypoint.name.c_str(), entrypoint.stage);
            assert(spec_entrypoint);  // spirv-opt won't change Entrypoint Name/stage

            local_size = spec_mod.FindLocalSize(*spec_entrypoint);

            total_workgroup_shared_memory = spec_mod.CalculateWorkgroupSharedMemory();

            if (spec_entrypoint->task_payload_variable) {
                total_task_payload_memory = spec_entrypoint->task_payload_variable->size;
            }

            spvDiagnosticDestroy(diag);
            spvContextDestroy(ctx);
        } else {
            // Should never get here, but better then asserting
            const char *vuid = pipeline ? "VUID-VkPipelineShaderStageCreateInfo-pSpecializationInfo-06849"
                                        : "VUID-VkShaderCreateInfoEXT-pCode-08460";
            skip |= LogError(vuid, device, loc,
                             "%s shader (stage %s) attempted to apply specialization constants with spirv-opt but failed.",
                             FormatHandle(module_state.handle()).c_str(), string_VkShaderStageFlagBits(stage));
        }

        if (skip) {
            return skip;  // if spec constants have errors, can produce false positives later
        }
    } else {
        local_size = module_state.FindLocalSize(entrypoint);

        total_workgroup_shared_memory = module_state.CalculateWorkgroupSharedMemory();

        if (entrypoint.task_payload_variable) {
            total_task_payload_memory = entrypoint.task_payload_variable->size;
        }
    }

    skip |= ValidateImageWrite(module_state, loc);
    skip |= ValidateShaderExecutionModes(module_state, entrypoint, stage, pipeline, loc);
    skip |= ValidateBuiltinLimits(module_state, entrypoint, pipeline, loc);
    skip |= ValidatePushConstantUsage(module_state, entrypoint, pipeline, stage_state, loc);
    if (enabled_features.cooperativeMatrix) {
        skip |= ValidateCooperativeMatrix(module_state, entrypoint, stage_state, local_size, loc);
    }
    if (enabled_features.cooperativeVector) {
        skip |= ValidateCooperativeVector(module_state, entrypoint, stage_state, loc);
    }
    skip |= ValidateShader64BitIndexing(module_state, entrypoint, stage_state, pipeline, loc);

    if (pipeline) {
        if (enabled_features.transformFeedback) {
            skip |= ValidateTransformFeedbackPipeline(module_state, entrypoint, *pipeline, loc);
        }
        if (enabled_features.primitiveFragmentShadingRate) {
            skip |= ValidatePrimitiveRateShaderState(module_state, entrypoint, *pipeline, stage, loc);
        }
        if (IsExtEnabled(extensions.vk_qcom_render_pass_shader_resolve)) {
            skip |= ValidateShaderResolveQCOM(module_state, stage, *pipeline, loc);
        }
        skip |= ValidatePointSizeShaderState(module_state, entrypoint, *pipeline, stage, loc);
        skip |= ValidatePrimitiveTopology(module_state, entrypoint, *pipeline, loc);

        if (stage == VK_SHADER_STAGE_FRAGMENT_BIT) {
            if (IsExtEnabled(extensions.vk_ext_shader_tile_image)) {
                skip |= ValidateShaderTileImage(module_state, entrypoint, *pipeline, loc);
            }

            if (pipeline->GraphicsCreateInfo().renderPass == VK_NULL_HANDLE &&
                module_state.HasCapability(spv::CapabilityInputAttachment) && !enabled_features.dynamicRenderingLocalRead) {
                skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-renderPass-06061", device, loc,
                                 "is being created with fragment shader with InputAttachment capability, but renderPass is "
                                 "VK_NULL_HANDLE. (It is only possbile to use input attachments with dynamic rendering if the "
                                 "dynamicRenderingLocalRead feature is enabled)");
            }
        }
    }

    if (stage == VK_SHADER_STAGE_COMPUTE_BIT || stage == VK_SHADER_STAGE_TASK_BIT_EXT || stage == VK_SHADER_STAGE_MESH_BIT_EXT) {
        // If spec constants were used then the local size are already found if possible
        if (local_size.x == 0) {
            local_size = module_state.FindLocalSize(entrypoint);
        }

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
                             "SPIR-V LocalSize (%s) exceeds device limit maxComputeWorkGroupInvocations (%" PRIu32 ").",
                             local_size.ToString().c_str(), phys_dev_props.limits.maxComputeWorkGroupInvocations);
        }

        skip |= ValidateRequiredSubgroupSize(module_state, stage_state, invocations, local_size, loc);
        skip |= ValidateWorkgroupSharedMemory(module_state, stage, total_workgroup_shared_memory, loc);
    }

    for (const auto &variable : entrypoint.resource_interface_variables) {
        vvl::unordered_set<uint32_t> descriptor_type_set;
        TypeToDescriptorTypeSet(module_state, variable.type_id, variable.data_type_id, descriptor_type_set);
        skip |= ValidateShaderInterfaceVariable(module_state, variable, descriptor_type_set, loc);
        if (stage_state.descriptor_set_layouts) {
            skip |= ValidateShaderInterfaceVariableDSL(module_state, entrypoint, stage_state, variable, descriptor_type_set, loc);
        }
        if (pipeline) {
            if (variable.decorations.Has(spirv::DecorationSet::input_attachment_bit)) {
                skip |= ValidateShaderInputAttachment(module_state, *pipeline, variable, loc);
            }
        }
    }

    if (stage == VK_SHADER_STAGE_COMPUTE_BIT) {
        skip |= ValidateComputeWorkGroupSizes(module_state, entrypoint, stage_state, local_size, loc);
    } else if (stage == VK_SHADER_STAGE_TASK_BIT_EXT || stage == VK_SHADER_STAGE_MESH_BIT_EXT) {
        skip |= ValidateTaskMeshWorkGroupSizes(module_state, entrypoint, local_size, loc);
        if (stage == VK_SHADER_STAGE_TASK_BIT_EXT) {
            skip |= ValidateEmitMeshTasksSize(module_state, entrypoint, stage_state, loc);
        } else if (stage == VK_SHADER_STAGE_MESH_BIT_EXT) {
            skip |= ValidateMeshMemorySize(module_state, total_workgroup_shared_memory, total_task_payload_memory, loc);
        }
    }

    return skip;
}

uint32_t CoreChecks::CalcShaderStageCount(const vvl::Pipeline &pipeline, VkShaderStageFlagBits stageBit) const {
    uint32_t total = 0;
    for (const auto &stage_ci : pipeline.shader_stages_ci) {
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

bool CoreChecks::GroupHasValidIndex(const vvl::Pipeline &pipeline, uint32_t group, uint32_t stage) const {
    if (group == VK_SHADER_UNUSED_KHR) {
        return true;
    }

    const auto num_stages = static_cast<uint32_t>(pipeline.shader_stages_ci.size());
    if (group < num_stages) {
        return (pipeline.shader_stages_ci[group].stage & stage) != 0;
    }
    group -= num_stages;

    // Search libraries
    if (pipeline.ray_tracing_library_ci) {
        for (uint32_t i = 0; i < pipeline.ray_tracing_library_ci->libraryCount; ++i) {
            auto library_pipeline = Get<vvl::Pipeline>(pipeline.ray_tracing_library_ci->pLibraries[i]);
            if (!library_pipeline) {
                continue;
            }
            const uint32_t stage_count = static_cast<uint32_t>(library_pipeline->shader_stages_ci.size());
            if (group < stage_count) {
                return (library_pipeline->shader_stages_ci[group].stage & stage) != 0;
            }
            group -= stage_count;
        }
    }

    // group index too large
    return false;
}

// This is done in PreCallRecord to help with the interaction with GPU-AV
// See diagram on https://github.com/KhronosGroup/Vulkan-ValidationLayers/pull/6230
void CoreChecks::PreCallRecordCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo *pCreateInfo,
                                                 const VkAllocationCallbacks *pAllocator, VkShaderModule *pShaderModule,
                                                 const RecordObject &record_obj, chassis::CreateShaderModule &chassis_state) {
    // Normally would validate in PreCallValidate, but need a non-const function to update chassis_state
    // This is on the stack, we don't have to worry about threading hazards and this could be moved and used const_cast
    chassis_state.skip |=
        stateless_spirv_validator.Validate(*chassis_state.module_state, chassis_state.stateless_data, record_obj.location);
}

void CoreChecks::PreCallRecordCreateShadersEXT(VkDevice device, uint32_t createInfoCount, const VkShaderCreateInfoEXT *pCreateInfos,
                                               const VkAllocationCallbacks *pAllocator, VkShaderEXT *pShaders,
                                               const RecordObject &record_obj, chassis::ShaderObject &chassis_state) {
    // For ShaderObjects, to validate most things we need to first parse the SPIR-V.
    // We use to parse both at PreCallValidate and PreCallRecord which was wasteful.
    // We now parse it at PreCallRecord (so we can store it) and then just do the validation here
    chassis_state.skip = ValidateCreateShadersSpirv(createInfoCount, pCreateInfos, record_obj.location, chassis_state);
}

bool CoreChecks::RunSpirvValidation(spv_const_binary_t &binary, const Location &loc, ValidationCache *cache) const {
    bool skip = false;

    if (global_settings.debug_disable_spirv_val) {
        return skip;
    }

    uint32_t hash = 0;
    if (cache) {
        hash = hash_util::Hash32((void *)binary.code, binary.wordCount * sizeof(uint32_t));
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
        const char *error_message = diag && diag->error ? diag->error : "(no error text)";

        // Umbrella VUID if we can't find one in spirv-val
        const char *vuid = loc.function == Func::vkCreateShadersEXT ? "VUID-VkShaderCreateInfoEXT-pCode-08737"
                                                                    : "VUID-VkShaderModuleCreateInfo-pCode-08737";

        // We want to search inside the spirv-val error message to see if there is VUID in it as it allows people to silence just
        // that VUID and not the whole spirv-val check
        char *spirv_val_vuid = nullptr;
        if (diag && diag->error) {
            // Note: Will always start with "[VUID-xxx-00000]" if there is one
            if (std::strncmp(error_message, "[VUID", 5) == 0) {
                const char *bracket_end = std::strchr(error_message, ']');
                if (bracket_end) {
                    const size_t vuid_len = bracket_end - error_message - 1;
                    spirv_val_vuid = new char[vuid_len + 1];  // +1 for null-terminator
                    std::strncpy(spirv_val_vuid, error_message + 1, vuid_len);
                    spirv_val_vuid[vuid_len] = '\0';

                    // Remove VUID from error message now
                    error_message = bracket_end + 2;
                }
                vuid = spirv_val_vuid;
            }
        }

        if (spv_valid == SPV_WARNING) {
            skip |= LogWarning(vuid, device, loc.dot(Field::pCode),
                               "(spirv-val produced a warning):\n%s\nCommand to reproduce:\n\t%s\n", error_message,
                               spirv_val_command.c_str());
        } else {
            skip |=
                LogError(vuid, device, loc.dot(Field::pCode), "(spirv-val produced an error):\n%s\nCommand to reproduce:\n\t%s\n",
                         error_message, spirv_val_command.c_str());
        }

        if (spirv_val_vuid) {
            delete[] spirv_val_vuid;
        }
    } else if (cache) {
        // No point to cache anything that is not valid, or it will get suppressed on the next run
        cache->Insert(hash);
    }

    spvDiagnosticDestroy(diag);
    spvContextDestroy(ctx);

    return skip;
}

bool CoreChecks::ValidateShaderModuleCreateInfo(const VkShaderModuleCreateInfo &create_info,
                                                const Location &create_info_loc) const {
    bool skip = false;

    if (disabled[shader_validation]) {
        return skip;  // VK_VALIDATION_FEATURE_DISABLE_SHADERS_EXT
    } else if (!create_info.pCode) {
        return skip;  // will be caught elsewhere
    }

    // This extension is meant for tooling, but still valid to be used, if used, we need to detect if GLSL
    if (IsExtEnabled(extensions.vk_nv_glsl_shader)) {
        if (strncmp((char *)create_info.pCode, "#version", 8) == 0) {
            return skip;  // incoming GLSL
        }
    }

    const uint32_t first_dword = create_info.pCode[0];
    if (SafeModulo(create_info.codeSize, 4) != 0) {
        skip |=
            LogError("VUID-VkShaderModuleCreateInfo-codeSize-08735", device, create_info_loc.dot(Field::codeSize),
                     "(%zu) must be a multiple of 4. You might have forget to multiply by sizeof(uint32_t).", create_info.codeSize);
    } else if (first_dword != spv::MagicNumber) {
        skip |= LogError("VUID-VkShaderModuleCreateInfo-pCode-08738", device, create_info_loc.dot(Field::pCode),
                         "doesn't point to a SPIR-V module. The first dword (0x%" PRIx32
                         ") is not the SPIR-V MagicNumber (0x07230203).",
                         first_dword);
    } else {
        // if pCode is garbage, don't pass along to spirv-val

        const auto validation_cache_ci = vku::FindStructInPNextChain<VkShaderModuleValidationCacheCreateInfoEXT>(create_info.pNext);
        ValidationCache *cache =
            validation_cache_ci ? CastFromHandle<ValidationCache *>(validation_cache_ci->validationCache) : nullptr;
        // If app isn't using a shader validation cache, use the default one from CoreChecks
        if (!cache) {
            cache = CastFromHandle<ValidationCache *>(core_validation_cache);
        }

        spv_const_binary_t binary{create_info.pCode, create_info.codeSize / sizeof(uint32_t)};
        skip |= RunSpirvValidation(binary, create_info_loc, cache);
    }

    return skip;
}

bool CoreChecks::PreCallValidateCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo *pCreateInfo,
                                                   const VkAllocationCallbacks *pAllocator, VkShaderModule *pShaderModule,
                                                   const ErrorObject &error_obj) const {
    return ValidateShaderModuleCreateInfo(*pCreateInfo, error_obj.location.dot(Field::pCreateInfo));
}

bool CoreChecks::PreCallValidateGetShaderModuleIdentifierEXT(VkDevice device, VkShaderModule shaderModule,
                                                             VkShaderModuleIdentifierEXT *pIdentifier,
                                                             const ErrorObject &error_obj) const {
    bool skip = false;
    if (!(enabled_features.shaderModuleIdentifier)) {
        skip |= LogError("VUID-vkGetShaderModuleIdentifierEXT-shaderModuleIdentifier-06884", shaderModule, error_obj.location,
                         "the shaderModuleIdentifier feature was not enabled.");
    }
    return skip;
}

bool CoreChecks::PreCallValidateGetShaderModuleCreateInfoIdentifierEXT(VkDevice device, const VkShaderModuleCreateInfo *pCreateInfo,
                                                                       VkShaderModuleIdentifierEXT *pIdentifier,
                                                                       const ErrorObject &error_obj) const {
    bool skip = false;
    if (!(enabled_features.shaderModuleIdentifier)) {
        skip |= LogError("VUID-vkGetShaderModuleCreateInfoIdentifierEXT-shaderModuleIdentifier-06885", device, error_obj.location,
                         "the shaderModuleIdentifier feature was not enabled.");
    }
    return skip;
}

bool CoreChecks::ValidateRequiredSubgroupSize(const spirv::Module &module_state, const ShaderStageState &stage_state,
                                              uint64_t invocations, const spirv::LocalSize &local_size, const Location &loc) const {
    bool skip = false;

    const auto *required_subgroup_size_ci =
        vku::FindStructInPNextChain<VkPipelineShaderStageRequiredSubgroupSizeCreateInfo>(stage_state.GetPNext());
    if (!required_subgroup_size_ci) return skip;

    const Location pNext_loc = loc.pNext(Struct::VkPipelineShaderStageRequiredSubgroupSizeCreateInfo);

    const uint32_t required_subgroup_size = required_subgroup_size_ci->requiredSubgroupSize;
    if (!enabled_features.subgroupSizeControl) {
        skip |= LogError("VUID-VkPipelineShaderStageCreateInfo-pNext-02755", module_state.handle(), pNext_loc,
                         "the subgroupSizeControl feature was not enabled");
    }
    if ((phys_dev_props_core13.requiredSubgroupSizeStages & stage_state.GetStage()) == 0) {
        skip |=
            LogError("VUID-VkPipelineShaderStageCreateInfo-pNext-02755", module_state.handle(), loc,
                     "SPIR-V (%s) is not in requiredSubgroupSizeStages (%s).", string_VkShaderStageFlagBits(stage_state.GetStage()),
                     string_VkShaderStageFlags(phys_dev_props_core13.requiredSubgroupSizeStages).c_str());
    }
    if ((invocations > required_subgroup_size * phys_dev_props_core13.maxComputeWorkgroupSubgroups)) {
        skip |= LogError("VUID-VkPipelineShaderStageCreateInfo-pNext-02756", module_state.handle(), loc,
                         "SPIR-V Local workgroup size (%s) is greater than requiredSubgroupSize (%" PRIu32
                         ") * maxComputeWorkgroupSubgroups (%" PRIu32 ").",
                         local_size.ToString().c_str(), required_subgroup_size, phys_dev_props_core13.maxComputeWorkgroupSubgroups);
    }
    if (stage_state.pipeline_create_info &&
        (stage_state.pipeline_create_info->flags & VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT) > 0) {
        if (SafeModulo(local_size.x, required_subgroup_size) != 0) {
            skip |= LogError("VUID-VkPipelineShaderStageCreateInfo-pNext-02757", module_state.handle(), loc,
                             "SPIR-V Local workgroup size x (%" PRIu32
                             ") is not a multiple of "
                             "requiredSubgroupSize (%" PRIu32 ").",
                             local_size.x, required_subgroup_size);
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

bool CoreChecks::ValidateComputeWorkGroupSizes(const spirv::Module &module_state, const spirv::EntryPoint &entrypoint,
                                               const ShaderStageState &stage_state, const spirv::LocalSize &local_size,
                                               const Location &loc) const {
    bool skip = false;

    if (local_size.x == 0) {
        return skip;
    }

    if (local_size.x > phys_dev_props.limits.maxComputeWorkGroupSize[0]) {
        skip |= LogError("VUID-RuntimeSpirv-x-06429", module_state.handle(), loc,
                         "SPIR-V LocalSize X (%" PRIu32 ") exceeds device limit maxComputeWorkGroupSize[0] (%" PRIu32 ").",
                         local_size.x, phys_dev_props.limits.maxComputeWorkGroupSize[0]);
    }
    if (local_size.y > phys_dev_props.limits.maxComputeWorkGroupSize[1]) {
        skip |= LogError("VUID-RuntimeSpirv-y-06430", module_state.handle(), loc,
                         "SPIR-V LocalSize Y (%" PRIu32 ") exceeds device limit maxComputeWorkGroupSize[1] (%" PRIu32 ").",
                         local_size.y, phys_dev_props.limits.maxComputeWorkGroupSize[1]);
    }
    if (local_size.z > phys_dev_props.limits.maxComputeWorkGroupSize[2]) {
        skip |= LogError("VUID-RuntimeSpirv-z-06431", module_state.handle(), loc,
                         "SPIR-V LocalSize Z (%" PRIu32 ") exceeds device limit maxComputeWorkGroupSize[2] (%" PRIu32 ").",
                         local_size.z, phys_dev_props.limits.maxComputeWorkGroupSize[2]);
    }

    if (stage_state.pipeline_create_info) {
        const auto subgroup_flags = VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT |
                                    VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT;
        if ((stage_state.pipeline_create_info->flags & subgroup_flags) == subgroup_flags) {
            if (SafeModulo(local_size.x, phys_dev_props_core13.maxSubgroupSize) != 0) {
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
                if (SafeModulo(local_size.x, phys_dev_props_core11.subgroupSize) != 0) {
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
        const auto *required_subgroup_size =
            vku::FindStructInPNextChain<VkShaderRequiredSubgroupSizeCreateInfoEXT>(stage_state.GetPNext());
        if (varying && full) {
            if (SafeModulo(local_size.x, phys_dev_props_core13.maxSubgroupSize) != 0) {
                skip |= LogError(
                    "VUID-VkShaderCreateInfoEXT-flags-08416", module_state.handle(), loc.dot(Field::flags),
                    "(%s) but local workgroup size X dimension (%" PRIu32
                    ") is not a multiple of VkPhysicalDeviceSubgroupSizeControlPropertiesEXT::maxSubgroupSize (%" PRIu32 ").",
                    string_VkPipelineShaderStageCreateFlags(stage_state.shader_object_create_info->flags).c_str(), local_size.x,
                    phys_dev_props_core13.maxSubgroupSize);
            }
        } else if (full && !varying) {
            if (!required_subgroup_size && SafeModulo(local_size.x, phys_dev_props_core11.subgroupSize) != 0) {
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

bool CoreChecks::ValidateTaskMeshWorkGroupSizes(const spirv::Module &module_state, const spirv::EntryPoint &entrypoint,
                                                const spirv::LocalSize &local_size, const Location &loc) const {
    bool skip = false;

    if (local_size.x == 0) {
        return skip;
    }

    spirv::LocalSize max_local_size;
    uint32_t max_workgroup_size = 0;
    const char *x_vuid;
    const char *y_vuid;
    const char *z_vuid;
    const char *workgroup_size_vuid;

    switch (entrypoint.execution_model) {
        case spv::ExecutionModelTaskEXT: {
            x_vuid = "VUID-RuntimeSpirv-TaskEXT-07291";
            y_vuid = "VUID-RuntimeSpirv-TaskEXT-07292";
            z_vuid = "VUID-RuntimeSpirv-TaskEXT-07293";
            workgroup_size_vuid = "VUID-RuntimeSpirv-TaskEXT-07294";
            max_local_size.x = phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupSize[0];
            max_local_size.y = phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupSize[1];
            max_local_size.z = phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupSize[2];
            max_workgroup_size = phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupInvocations;
            break;
        }

        case spv::ExecutionModelMeshEXT: {
            x_vuid = "VUID-RuntimeSpirv-MeshEXT-07295";
            y_vuid = "VUID-RuntimeSpirv-MeshEXT-07296";
            z_vuid = "VUID-RuntimeSpirv-MeshEXT-07297";
            workgroup_size_vuid = "VUID-RuntimeSpirv-MeshEXT-07298";
            max_local_size.x = phys_dev_ext_props.mesh_shader_props_ext.maxMeshWorkGroupSize[0];
            max_local_size.y = phys_dev_ext_props.mesh_shader_props_ext.maxMeshWorkGroupSize[1];
            max_local_size.z = phys_dev_ext_props.mesh_shader_props_ext.maxMeshWorkGroupSize[2];
            max_workgroup_size = phys_dev_ext_props.mesh_shader_props_ext.maxMeshWorkGroupInvocations;
            break;
        }

        // skip for spv::ExecutionModelTaskNV and spv::ExecutionModelMeshNV case
        default: {
            // must match one of the above case
            return skip;
        }
    }

    if (local_size.x > max_local_size.x) {
        skip |= LogError(x_vuid, module_state.handle(), loc,
                         "SPIR-V (%s) local workgroup size X dimension (%" PRIu32
                         ") must be less than or equal to the max workgroup size (%" PRIu32 ").",
                         string_SpvExecutionModel(entrypoint.execution_model), local_size.x, max_local_size.x);
    }

    if (local_size.y > max_local_size.y) {
        skip |= LogError(y_vuid, module_state.handle(), loc,
                         "SPIR-V (%s) local workgroup size Y dimension (%" PRIu32
                         ") must be less than or equal to the max workgroup size (%" PRIu32 ").",
                         string_SpvExecutionModel(entrypoint.execution_model), local_size.y, max_local_size.y);
    }

    if (local_size.z > max_local_size.z) {
        skip |= LogError(z_vuid, module_state.handle(), loc,
                         "SPIR-V (%s) local workgroup size Z dimension (%" PRIu32
                         ") must be less than or equal to the max workgroup size (%" PRIu32 ").",
                         string_SpvExecutionModel(entrypoint.execution_model), local_size.z, max_local_size.z);
    }

    uint64_t invocations = static_cast<uint64_t>(local_size.x) * static_cast<uint64_t>(local_size.y);
    // Prevent overflow.
    bool fail = false;
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
        skip |= LogError(workgroup_size_vuid, module_state.handle(), loc,
                         "SPIR-V (%s) total invocation size of %" PRIu64
                         " (%s) must be less than or equal to max workgroup invocations (%" PRIu32 ").",
                         string_SpvExecutionModel(entrypoint.execution_model), invocations, local_size.ToString().c_str(),
                         max_workgroup_size);
    }
    return skip;
}

bool CoreChecks::ValidateEmitMeshTasksSize(const spirv::Module &module_state, const spirv::EntryPoint &entrypoint,
                                           const ShaderStageState &stage_state, const Location &loc) const {
    bool skip = false;

    for (const spirv::Instruction &insn : module_state.static_data_.instructions) {
        if (insn.Opcode() == spv::OpEmitMeshTasksEXT) {
            uint32_t x, y, z;
            bool found_x = stage_state.GetInt32ConstantValue(*module_state.FindDef(insn.Word(1)), &x);
            bool found_y = stage_state.GetInt32ConstantValue(*module_state.FindDef(insn.Word(2)), &y);
            bool found_z = stage_state.GetInt32ConstantValue(*module_state.FindDef(insn.Word(3)), &z);
            if (found_x && x > phys_dev_ext_props.mesh_shader_props_ext.maxMeshWorkGroupCount[0]) {
                skip |= LogError("VUID-RuntimeSpirv-TaskEXT-07299", module_state.handle(), loc,
                                 "SPIR-V (%s) is emitting %" PRIu32
                                 " mesh work groups in X dimension, which is greater than max mesh "
                                 "workgroup count (%" PRIu32 ").",
                                 string_SpvExecutionModel(entrypoint.execution_model), x,
                                 phys_dev_ext_props.mesh_shader_props_ext.maxMeshWorkGroupCount[0]);
            }
            if (found_y && y > phys_dev_ext_props.mesh_shader_props_ext.maxMeshWorkGroupCount[1]) {
                skip |= LogError("VUID-RuntimeSpirv-TaskEXT-07300", module_state.handle(), loc,
                                 "SPIR-V (%s) is emitting %" PRIu32
                                 " mesh work groups in Y dimension, which is greater than max mesh "
                                 "workgroup count (%" PRIu32 ").",
                                 string_SpvExecutionModel(entrypoint.execution_model), y,
                                 phys_dev_ext_props.mesh_shader_props_ext.maxMeshWorkGroupCount[1]);
            }
            if (found_z && z > phys_dev_ext_props.mesh_shader_props_ext.maxMeshWorkGroupCount[2]) {
                skip |= LogError("VUID-RuntimeSpirv-TaskEXT-07301", module_state.handle(), loc,
                                 "SPIR-V (%s) is emitting %" PRIu32
                                 " mesh work groups in Z dimension, which is greater than max mesh "
                                 "workgroup count (%" PRIu32 ").",
                                 string_SpvExecutionModel(entrypoint.execution_model), z,
                                 phys_dev_ext_props.mesh_shader_props_ext.maxMeshWorkGroupCount[2]);
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
                    skip |=
                        LogError("VUID-RuntimeSpirv-TaskEXT-07302", module_state.handle(), loc,
                                 "SPIR-V (%s) is emitting %" PRIu32 " x %" PRIu32 " x %" PRIu32 " mesh work groups (total %" PRIu32
                                 "), which is greater than max mesh "
                                 "workgroup total count (%" PRIu32 ").",
                                 string_SpvExecutionModel(entrypoint.execution_model), x, y, z, x * y * z,
                                 phys_dev_ext_props.mesh_shader_props_ext.maxMeshWorkGroupTotalCount);
                }
            }
        }
    }

    return skip;
}

bool CoreChecks::ValidateMeshMemorySize(const spirv::Module &module_state, uint32_t total_workgroup_shared_memory,
                                        uint32_t total_task_payload_memory, const Location &loc) const {
    bool skip = false;

    if (total_task_payload_memory + total_workgroup_shared_memory >
        phys_dev_ext_props.mesh_shader_props_ext.maxMeshPayloadAndSharedMemorySize) {
        // If task payload memory size is 0 and only shared memory is already over the limit then the more appropriate VUID 08754
        // was already reported
        if (total_task_payload_memory > 0) {
            skip |= LogError(
                "VUID-RuntimeSpirv-maxMeshPayloadAndSharedMemorySize-08755", module_state.handle(), loc,
                "SPIR-V uses %" PRIu32 " bytes of task payload memory and %" PRIu32 " bytes of shared memory (combined %" PRIu32
                " bytes), which is more than maxMeshPayloadAndSharedMemorySize (%" PRIu32 ").",
                total_task_payload_memory, total_workgroup_shared_memory, total_task_payload_memory + total_workgroup_shared_memory,
                phys_dev_ext_props.mesh_shader_props_ext.maxMeshPayloadAndSharedMemorySize);
        }
    }

    return skip;
}

bool CoreChecks::ValidateTaskPayload(const spirv::Module &task_state, const spirv::EntryPoint &mesh_entrypoint,
                                     const Location &loc) const {
    bool skip = false;
    uint32_t task_payload_size = 0;
    uint32_t mesh_payload_size = 0;

    if (task_state.static_data_.emit_mesh_tasks_inst.size() > 1) {
        // If there are multiple OpEmitMeshTasksEXT we will need GPU-AV to know which was actually called
        return skip;
    }

    if (!task_state.static_data_.emit_mesh_tasks_inst.empty()) {
        const auto emit_mesh_task = task_state.static_data_.emit_mesh_tasks_inst.front();
        // Payload is optional
        if (emit_mesh_task->Length() == 5) {
            if (task_state.static_data_.has_specialization_constants) {
                // There is a chance this is not resolvable here to match exact size
                return skip;
            }
            const auto *payload_variable = task_state.FindDef(emit_mesh_task->Word(4));
            const spirv::Instruction *type = task_state.GetVariablePointerType(*payload_variable);
            task_payload_size = task_state.GetTypeBytesSize(type);
        }
    }

    if (mesh_entrypoint.task_payload_variable) {
        mesh_payload_size = mesh_entrypoint.task_payload_variable->size;
    }

    // If task and mesh are both set and just matter of trying to resolve spec constant values, skip possibly reporting false
    // positives
    if (mesh_payload_size == spirv::kInvalidValue && task_payload_size != 0) {
        return skip;
    }

    // It is valid to have the Task use the payload, but the mesh to ignore it
    if (mesh_payload_size != 0 && task_payload_size != mesh_payload_size) {
        std::stringstream ss;
        ss << "The Mesh Shader has a TaskPayloadWorkgroupEXT variable, but the Task Shader ";
        if (task_payload_size == 0) {
            ss << "never sets a TaskPayloadWorkgroupEXT variable in the call to OpEmitMeshTasksEXT";
        } else {
            ss << "sets a TaskPayloadWorkgroupEXT variable that is " << task_payload_size << " bytes but needs to match the "
               << mesh_payload_size << " bytes payload in the Mesh Shader.";
        }
        skip |= LogError("VUID-RuntimeSpirv-MeshEXT-10883", device, loc, "%s", ss.str().c_str());
    }

    return skip;
}

bool CoreChecks::ValidateDataGraphPipelineShaderModuleSpirv(VkDevice device, const VkDataGraphPipelineCreateInfoARM& create_info, const Location& create_info_loc, const vvl::Pipeline& pipeline) const {
    bool skip = false;
    auto dg_pipeline_shader_module_ci = vku::FindStructInPNextChain<VkDataGraphPipelineShaderModuleCreateInfoARM>(create_info.pNext);
    if (!dg_pipeline_shader_module_ci) {
        return skip;
    }

    auto module_state = Get<vvl::ShaderModule>(dg_pipeline_shader_module_ci->module);
    if (!module_state) {
        return skip;
    }
    ASSERT_AND_RETURN_SKIP(module_state->spirv);
    auto &module_spirv = *(module_state->spirv);

    const Location pipeline_shader_module_ci_loc = create_info_loc.pNext(Struct::VkDataGraphPipelineShaderModuleCreateInfoARM);
    const Location module_loc = pipeline_shader_module_ci_loc.dot(Field::module);

    std::vector<std::pair<uint32_t, uint32_t>> tensor_bindings;
    bool name_found = false;
    const auto &stage_state = pipeline.stage_states[0];
    for (auto &entry_point : module_spirv.static_data_.entry_points) {
        if (!entry_point->is_data_graph)
            continue;

        if (!name_found && entry_point->name.compare(dg_pipeline_shader_module_ci->pName) == 0) {
            name_found = true;
            for (const auto &variable : entry_point->resource_interface_variables) {
                vvl::unordered_set<uint32_t> descriptor_type_set;
                TypeToDescriptorTypeSet(module_spirv, variable.type_id, variable.data_type_id, descriptor_type_set);
                skip |= ValidateShaderInterfaceVariable(module_spirv, variable, descriptor_type_set, module_loc);
                skip |= ValidateShaderInterfaceVariableDSL(module_spirv, *entry_point, stage_state, variable, descriptor_type_set,
                                                           module_loc);
                if (variable.is_storage_tensor) {
                    tensor_bindings.push_back({variable.decorations.set, variable.decorations.binding});
                }
            }
            break;
        }
    }

    if (!name_found) {
        std::stringstream wrong_names;
        for (const auto& entry_point : module_spirv.static_data_.entry_points) {
            if (!wrong_names.str().empty()) {
                wrong_names << ", ";
            }
            wrong_names << entry_point->name;
        }
        skip |= LogError("VUID-VkDataGraphPipelineShaderModuleCreateInfoARM-pName-09872", device,
                         pipeline_shader_module_ci_loc.dot(Field::pName),
                         " is '%s' but names in OpGraphEntryPointARM instructions are: '%s'", dg_pipeline_shader_module_ci->pName,
                         wrong_names.str().c_str());
    }

    std::unordered_set<uint32_t> graph_tensor_ids;
    std::unordered_map<uint32_t, uint32_t> graph_constant_map;
    for (auto &instruction : module_spirv.GetInstructions()) {
        if (instruction.Opcode() == spv::OpTypeTensorARM) {
            graph_tensor_ids.insert(instruction.Word(1));
        }
        if (instruction.Opcode() == spv::OpGraphConstantARM) {
            graph_constant_map[instruction.Word(3)] = instruction.Word(1);
        }
    }
    if (!enabled_features.dataGraphSpecializationConstants) {
        if (module_spirv.static_data_.has_specialization_constants) {
            skip |= LogError("VUID-VkDataGraphPipelineShaderModuleCreateInfoARM-dataGraphSpecializationConstants-09849", device,
                             module_loc,
                             "contains OpSpec* instruction(s), but the dataGraphSpecializationConstants feature is not enabled.");
        }
    }
    for (uint32_t j = 0; j < dg_pipeline_shader_module_ci->constantCount; j++) {
        auto& constant = dg_pipeline_shader_module_ci->pConstants[j];
        const Location constant_loc = pipeline_shader_module_ci_loc.dot(Field::pConstants, j);
        if (graph_constant_map.find(constant.id) == graph_constant_map.end()) {
            std::stringstream const_ids;
            for (auto &c : graph_constant_map) {
                if (!const_ids.str().empty()) {
                    const_ids << ", ";
                }
                const_ids << c.first;
            }
            skip |= LogError(
                "VUID-VkDataGraphPipelineShaderModuleCreateInfoARM-id-09774", device, constant_loc.dot(Field::id),
                "(%" PRIu32 ") does not match any of the GraphConstantIDs ([%s]) used by OpGraphConstantARM instructions in module",
                constant.id, const_ids.str().c_str());
        } else {
            if (std::find(graph_tensor_ids.begin(), graph_tensor_ids.end(), graph_constant_map[constant.id]) !=
                graph_tensor_ids.end()) {
                auto *tensor_desc = vku::FindStructInPNextChain<VkTensorDescriptionARM>(constant.pNext);
                if (!tensor_desc) {
                    skip |=
                        LogError("VUID-VkDataGraphPipelineConstantARM-id-09850", device, constant_loc,
                                 "(%" PRIu32
                                 ") is a graph constant of tensor type, but there is no VkTensorDescriptionARM in the pNext chain",
                                 constant.id);
                } else if ((tensor_desc->usage & VK_TENSOR_USAGE_DATA_GRAPH_BIT_ARM) == 0) {
                    skip |= LogError(
                        "VUID-VkDataGraphPipelineConstantARM-id-09850", device, constant_loc.dot(Field::id),
                        "(%" PRIu32
                        ") is a graph constant of tensor type but its matching VkTensorDescriptionARM has an invalid usage (%s)",
                        constant.id, string_VkTensorUsageFlagsARM(tensor_desc->usage).c_str());
                }
            }
        }
    }

    for (uint32_t j = 0; j < create_info.resourceInfoCount; j++) {
        auto resource = create_info.pResourceInfos[j];
        auto resource_loc = create_info_loc.dot(Field::pResourceInfos, j);
        std::pair<uint32_t, uint32_t> resource_binding = {resource.descriptorSet, resource.binding};
        auto tensor_binding = std::find(tensor_bindings.begin(), tensor_bindings.end(), resource_binding);
        if (tensor_binding != tensor_bindings.end()) {
            auto *tensor_desc = vku::FindStructInPNextChain<VkTensorDescriptionARM>(resource.pNext);
            if (!tensor_desc) {
                skip |= LogError("VUID-VkDataGraphPipelineResourceInfoARM-descriptorSet-09851", device, resource_loc,
                                 "(descriptorSet %" PRIu32 ", binding %" PRIu32
                                 ") identifies a tensor or array of tensor resources, but the pNext chain doesn't include a "
                                 "VkTensorDescriptionARM structure",
                                 resource.descriptorSet, resource.binding);
            } else if ((tensor_desc->usage & VK_TENSOR_USAGE_DATA_GRAPH_BIT_ARM) == 0) {
                skip |=
                    LogError("VUID-VkDataGraphPipelineResourceInfoARM-descriptorSet-09851", device,
                             resource_loc.pNext(Struct::VkTensorDescriptionARM).dot(Field::usage),
                             "(%s) invalid for tensor resource with (descriptorSet %" PRIu32 ", binding %" PRIu32 ")",
                             string_VkTensorUsageFlagsARM(tensor_desc->usage).c_str(), resource.descriptorSet, resource.binding);
            }
        }
    }

    return skip;
}