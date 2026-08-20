/* Copyright (c) 2015-2026 The Khronos Group Inc.
 * Copyright (c) 2015-2026 Valve Corporation
 * Copyright (c) 2015-2026 LunarG, Inc.
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

#include "containers/custom_containers.h"
#include "error_message/error_location.h"
#include "error_message/error_strings.h"
#include <vulkan/vk_enum_string_helper.h>
#include "core_validation.h"
#include "generated/spirv_grammar_helper.h"
#include "generated/dispatch_functions.h"
#include "state_tracker/shader_instruction.h"
#include "state_tracker/shader_module.h"
#include "state_tracker/shader_stage_state.h"
#include "state_tracker/pipeline_state.h"
#include "utils/math_utils.h"

// Map SPIR-V type to VK_COMPONENT_TYPE enum
VkComponentTypeKHR GetComponentType(const spirv::Instruction* insn, bool is_signed_int) {
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
bool CoreChecks::ValidateCooperativeMatrix(const spirv::Module& module_state, const spirv::EntryPoint& entrypoint,
                                           const ShaderStageState& stage_state, const spirv::LocalSize& local_size,
                                           const Location& loc) const {
    bool skip = false;
    const uint64_t workgroup_size = static_cast<uint64_t>(local_size.x) * local_size.y * local_size.z;

    const auto* pipeline_required_subgroup_size_ci =
        vku::FindStructInPNextChain<VkPipelineShaderStageRequiredSubgroupSizeCreateInfo>(stage_state.GetPNext());
    const auto* shader_required_subgroup_size_ci =
        vku::FindStructInPNextChain<VkShaderRequiredSubgroupSizeCreateInfoEXT>(stage_state.GetPNext());
    uint32_t effective_subgroup_size = phys_dev_props_core11.subgroupSize;
    if (pipeline_required_subgroup_size_ci) {
        effective_subgroup_size = pipeline_required_subgroup_size_ci->requiredSubgroupSize;
    } else if (shader_required_subgroup_size_ci) {
        effective_subgroup_size = shader_required_subgroup_size_ci->requiredSubgroupSize;
    }
    const bool has_required_subgroup_size = pipeline_required_subgroup_size_ci || shader_required_subgroup_size_ci;

    const bool allows_varying_subgroup_size =
        stage_state.HasPipeline()
            ? (stage_state.pipeline_create_info->flags & VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT) != 0
            : (stage_state.shader_object_create_info->flags & VK_SHADER_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT_EXT) != 0;
    constexpr uint32_t spirv_version_1_6 = 0x00010600;
    const bool spirv_1_6_or_later = module_state.words_.size() > 1 && module_state.words_[1] >= spirv_version_1_6;
    // A zero selector requests properties suitable for an effective subgroup size that is not known at creation time.
    const uint32_t properties2_subgroup_size =
        (allows_varying_subgroup_size || (spirv_1_6_or_later && !has_required_subgroup_size)) ? 0 : effective_subgroup_size;

    const auto& IsSignedIntType = [&module_state](const uint32_t type_id) {
        const spirv::Instruction* type = module_state.FindDef(type_id);
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

        CoopMatType(uint32_t id, const spirv::Module& module_state, bool is_signed_int) {
            const spirv::Instruction* insn = module_state.FindDef(id);
            const spirv::Instruction* component_type_insn = module_state.FindDef(insn->Word(2));
            const spirv::Instruction* scope_insn = module_state.FindDef(insn->Word(3));
            const spirv::Instruction* rows_insn = module_state.FindDef(insn->Word(4));
            const spirv::Instruction* cols_insn = module_state.FindDef(insn->Word(5));

            all_constant = true;
            uint32_t tmp_scope = 0;
            if (!module_state.GetInt32IfConstant(*scope_insn, &tmp_scope)) {
                all_constant = false;
            }
            scope = VkScopeKHR(tmp_scope);
            if (!module_state.GetInt32IfConstant(*rows_insn, &rows)) {
                all_constant = false;
            }
            if (!module_state.GetInt32IfConstant(*cols_insn, &cols)) {
                all_constant = false;
            }
            component_type = GetComponentType(component_type_insn, is_signed_int);

            if (insn->Opcode() == spv::OpTypeCooperativeMatrixKHR) {
                const spirv::Instruction* use_insn = module_state.FindDef(insn->Word(6));
                if (!module_state.GetInt32IfConstant(*use_insn, &use)) {
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
                         "shader %s contains OpTypeCooperativeMatrixKHR but %s is not in "
                         "cooperativeMatrixSupportedStages (%s)",
                         entrypoint.Describe().c_str(), string_VkShaderStageFlagBits(entrypoint.stage),
                         string_VkShaderStageFlags(phys_dev_ext_props.cooperative_matrix_props_khr.cooperativeMatrixSupportedStages)
                             .c_str());
        }
    } else if (module_state.HasCapability(spv::CapabilityCooperativeMatrixNV)) {
        if (!(entrypoint.stage & phys_dev_ext_props.cooperative_matrix_props.cooperativeMatrixSupportedStages)) {
            skip |= LogError(
                "VUID-RuntimeSpirv-OpTypeCooperativeMatrixNV-06322", module_state.handle(), loc,
                "shader %s contains OpTypeCooperativeMatrixNV but %s is not in cooperativeMatrixSupportedStages "
                "(%s)",
                entrypoint.Describe().c_str(), string_VkShaderStageFlagBits(entrypoint.stage),
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
            const char* vuid = stage_state.HasPipeline() ? "VUID-RuntimeSpirv-OpTypeCooperativeMatrixKHR-10770"
                                                         : "VUID-RuntimeSpirv-OpTypeCooperativeMatrixKHR-10771";
            skip |= LogError(vuid, module_state.handle(), loc,
                             "shader %s contains SPV_KHR_cooperative_matrix which requires SPIR-V 1.6 (Vulkan 1.3). In order to "
                             "use it with older versions, you need to use %s (which requires VK_EXT_subgroup_size_control).",
                             entrypoint.Describe().c_str(),
                             stage_state.HasPipeline() ? "VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT"
                                                       : "VK_SHADER_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT");
        }
    }

    // Map SPIR-V result ID to the ID of its type.
    // TODO - Should have more robust way in ModuleState to find the type
    vvl::unordered_map<uint32_t, uint32_t> id_to_type_id;
    for (const spirv::Instruction& insn : module_state.GetInstructions()) {
        if (OpcodeHasType(insn.Opcode()) && OpcodeHasResult(insn.Opcode())) {
            id_to_type_id[insn.Word(2)] = insn.Word(1);
        }
    }

    auto print_properties = [this]() {
        std::ostringstream ss;
        for (uint32_t i = 0; i < device_state->cooperative_matrix_properties_khr.size(); ++i) {
            const auto& prop = device_state->cooperative_matrix_properties_khr[i];
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
            const auto& prop = device_state->cooperative_matrix_flexible_dimensions_properties[i];
            ss << "[" << i << "] MGranularity = " << prop.MGranularity << " | NGranularity = " << prop.NGranularity
               << " | KGranularity = " << prop.KGranularity << " | AType = " << string_VkComponentTypeKHR(prop.AType)
               << " | BType = " << string_VkComponentTypeKHR(prop.BType) << " | CType = " << string_VkComponentTypeKHR(prop.CType)
               << " | ResultType = " << string_VkComponentTypeKHR(prop.ResultType) << " | scope = " << string_VkScopeKHR(prop.scope)
               << " | workgroupInvocations = " << prop.workgroupInvocations << '\n';
        }
        return ss.str();
    };

    // Shader-derived values can already be invalid. Do not dispatch an internal query that violates its own VUs.
    const auto can_query_properties2 = [this](VkScopeKHR scope, uint64_t invocations, uint32_t subgroup_size) {
        if (scope != VK_SCOPE_SUBGROUP_KHR && scope != VK_SCOPE_WORKGROUP_KHR) {
            return false;
        }
        if (subgroup_size != 0 && !IsPowerOfTwo(subgroup_size)) {
            return false;
        }
        if (scope == VK_SCOPE_SUBGROUP_KHR) {
            if (invocations != 0) {
                return false;
            }
            if (subgroup_size != 0) {
                if (enabled_features.subgroupSizeControl) {
                    if (subgroup_size < phys_dev_props_core13.minSubgroupSize ||
                        subgroup_size > phys_dev_props_core13.maxSubgroupSize) {
                        return false;
                    }
                } else if (subgroup_size != phys_dev_props_core11.subgroupSize) {
                    return false;
                }
            }
        } else {
            if (!enabled_features.cooperativeMatrixWorkgroupScopeNV || invocations > UINT32_MAX || !IsPowerOfTwo(invocations)) {
                return false;
            }
            if (subgroup_size != 0 && (invocations % subgroup_size) != 0) {
                return false;
            }
            if (invocations > phys_dev_ext_props.cooperative_matrix_props2_nv.cooperativeMatrixWorkgroupScopeMaxWorkgroupSize) {
                return false;
            }
        }
        return true;
    };

    const auto query_properties2 =
        [this, &can_query_properties2](
            VkScopeKHR scope, uint64_t invocations, uint32_t subgroup_size,
            VkCooperativeMatrixFlagsEXT flags) -> std::optional<std::vector<VkCooperativeMatrixProperties2EXT>> {
        // nullopt means no complete query was available; an engaged empty vector is a successful empty query.
        if (!can_query_properties2(scope, invocations, subgroup_size)) {
            return std::nullopt;
        }

        VkPhysicalDeviceCooperativeMatrixInfo2EXT info = vku::InitStructHelper();
        info.scope = scope;
        info.invocations = static_cast<uint32_t>(invocations);
        info.subgroupSize = subgroup_size;
        info.flags = flags;

        uint32_t count = 0;
        if (DispatchGetPhysicalDeviceCooperativeMatrixProperties2EXT(physical_device_state->VkHandle(), &info, &count, nullptr) !=
            VK_SUCCESS) {
            return std::nullopt;
        }

        std::vector<VkCooperativeMatrixProperties2EXT> properties(count, vku::InitStruct<VkCooperativeMatrixProperties2EXT>());
        if (count == 0) {
            return properties;
        }

        if (DispatchGetPhysicalDeviceCooperativeMatrixProperties2EXT(physical_device_state->VkHandle(), &info, &count,
                                                                     properties.data()) != VK_SUCCESS) {
            return std::nullopt;
        }
        properties.resize(count);
        return properties;
    };

    const auto properties2_dimension_matches = [this](uint32_t dimension, uint32_t granularity) {
        return enabled_features.cooperativeMatrixFlexibleDimensionsNV ? IsIntegerMultipleOf(dimension, granularity)
                                                                      : dimension == granularity;
    };

    // Only want to report single error, otherwise easy to spam users with 10 messages that are all the same
    bool found_error = false;

    for (const spirv::Instruction* cooperative_matrix_inst : module_state.static_data_.cooperative_matrix_inst) {
        if (found_error) {
            break;
        }
        const spirv::Instruction& insn = *cooperative_matrix_inst;
        switch (insn.Opcode()) {
            case spv::OpTypeCooperativeMatrixKHR: {
                CoopMatType m(insn.ResultId(), module_state, IsSignedIntType(insn.Word(2)));

                if ((entrypoint.stage & VK_SHADER_STAGE_COMPUTE_BIT) != 0) {
                    if (!IsIntegerMultipleOf(local_size.x, effective_subgroup_size)) {
                        const auto vuid_string = m.scope == VK_SCOPE_SUBGROUP_KHR
                                                     ? "VUID-VkPipelineShaderStageCreateInfo-module-08987"
                                                     : "VUID-VkPipelineShaderStageCreateInfo-module-10169";
                        skip |= LogError(vuid_string, module_state.handle(), loc,
                                         "shader %s has a local workgroup size in the X dimension (%" PRIu32
                                         ") is not a multiple of subgroupSize (%" PRIu32 ").",
                                         entrypoint.Describe().c_str(), local_size.x, effective_subgroup_size);
                        found_error = true;
                    }
                    if (m.scope == VK_SCOPE_WORKGROUP_KHR) {
                        if (workgroup_size >
                            phys_dev_ext_props.cooperative_matrix_props2_nv.cooperativeMatrixWorkgroupScopeMaxWorkgroupSize) {
                            skip |= LogError(
                                "VUID-VkPipelineShaderStageCreateInfo-module-10169", module_state.handle(), loc,
                                "shader %s has a total local workgroup size (%" PRIu64
                                ") is larger than cooperativeMatrixWorkgroupScopeMaxWorkgroupSize (%" PRIu32 ").",
                                entrypoint.Describe().c_str(), workgroup_size,
                                phys_dev_ext_props.cooperative_matrix_props2_nv.cooperativeMatrixWorkgroupScopeMaxWorkgroupSize);
                            found_error = true;
                        }
                    }
                }

                if (!m.all_constant) {
                    break;
                }

                if (m.scope == VK_SCOPE_WORKGROUP_KHR && !enabled_features.cooperativeMatrixWorkgroupScopeNV) {
                    skip |= LogError("VUID-RuntimeSpirv-cooperativeMatrixWorkgroupScope-10164", module_state.handle(), loc,
                                     "shader %s has a cooperative matrix that uses workgroup scope but "
                                     "cooperativeMatrixWorkgroupScope is not enabled.",
                                     entrypoint.Describe().c_str());
                    found_error = true;
                }

                // Validate that the type parameters are all supported for one of the
                // operands of a cooperative matrix khr property.
                bool valid = false;
                for (uint32_t i = 0; i < device_state->cooperative_matrix_properties_khr.size(); ++i) {
                    const auto& property = device_state->cooperative_matrix_properties_khr[i];
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
                if (enabled_features.cooperativeMatrixFlexibleDimensionsNV) {
                    for (uint32_t i = 0; i < device_state->cooperative_matrix_flexible_dimensions_properties.size(); ++i) {
                        const auto& property = device_state->cooperative_matrix_flexible_dimensions_properties[i];

                        if (property.scope == VK_SCOPE_WORKGROUP_KHR && workgroup_size != property.workgroupInvocations) {
                            continue;
                        }

                        if (property.AType == m.component_type && IsIntegerMultipleOf(m.rows, property.MGranularity) &&
                            IsIntegerMultipleOf(m.cols, property.KGranularity) && property.scope == m.scope &&
                            m.use == spv::CooperativeMatrixUseMatrixAKHR) {
                            valid = true;
                            break;
                        }
                        if (property.BType == m.component_type && IsIntegerMultipleOf(m.rows, property.KGranularity) &&
                            IsIntegerMultipleOf(m.cols, property.NGranularity) && property.scope == m.scope &&
                            m.use == spv::CooperativeMatrixUseMatrixBKHR) {
                            valid = true;
                            break;
                        }
                        if (property.CType == m.component_type && IsIntegerMultipleOf(m.rows, property.MGranularity) &&
                            IsIntegerMultipleOf(m.cols, property.NGranularity) && property.scope == m.scope &&
                            m.use == spv::CooperativeMatrixUseMatrixAccumulatorKHR) {
                            valid = true;
                            break;
                        }
                        if (property.ResultType == m.component_type && IsIntegerMultipleOf(m.rows, property.MGranularity) &&
                            IsIntegerMultipleOf(m.cols, property.NGranularity) && property.scope == m.scope &&
                            m.use == spv::CooperativeMatrixUseMatrixAccumulatorKHR) {
                            valid = true;
                            break;
                        }
                    }
                }
                if (!valid && enabled_features.cooperativeMatrixProperties2) {
                    const uint64_t query_invocations = m.scope == VK_SCOPE_WORKGROUP_KHR ? workgroup_size : 0;
                    bool query_succeeded = true;

                    // OpTypeCooperativeMatrixKHR has no saturating operand, so either property class can support its type.
                    for (const VkCooperativeMatrixFlagsEXT flags :
                         {VkCooperativeMatrixFlagsEXT{0},
                          VkCooperativeMatrixFlagsEXT{VK_COOPERATIVE_MATRIX_SATURATING_ACCUMULATION_BIT_EXT}}) {
                        auto properties2 = query_properties2(m.scope, query_invocations, properties2_subgroup_size, flags);
                        if (!properties2) {
                            query_succeeded = false;
                            break;
                        }
                        for (const auto& property : *properties2) {
                            if (property.AType == m.component_type &&
                                properties2_dimension_matches(m.rows, property.MGranularity) &&
                                properties2_dimension_matches(m.cols, property.KGranularity) &&
                                m.use == spv::CooperativeMatrixUseMatrixAKHR) {
                                valid = true;
                                break;
                            }
                            if (property.BType == m.component_type &&
                                properties2_dimension_matches(m.rows, property.KGranularity) &&
                                properties2_dimension_matches(m.cols, property.NGranularity) &&
                                m.use == spv::CooperativeMatrixUseMatrixBKHR) {
                                valid = true;
                                break;
                            }
                            if ((property.CType == m.component_type || property.ResultType == m.component_type) &&
                                properties2_dimension_matches(m.rows, property.MGranularity) &&
                                properties2_dimension_matches(m.cols, property.NGranularity) &&
                                m.use == spv::CooperativeMatrixUseMatrixAccumulatorKHR) {
                                valid = true;
                                break;
                            }
                        }
                        if (valid) {
                            break;
                        }
                    }
                    if (!query_succeeded) {
                        // The invalid shader-derived selector is diagnosed by another VU.
                        break;
                    }
                }
                if (!valid) {
                    if (enabled_features.cooperativeMatrixProperties2) {
                        found_error = true;
                        skip |= LogError("VUID-RuntimeSpirv-cooperativeMatrixProperties2-13382", module_state.handle(), loc,
                                         "shader %s has\n%s (%s)\nbut doesn't match any supported "
                                         "VkCooperativeMatrixPropertiesKHR or VkCooperativeMatrixProperties2EXT.",
                                         entrypoint.Describe().c_str(), insn.Describe().c_str(), m.Describe().c_str());
                    } else if (!enabled_features.cooperativeMatrixFlexibleDimensionsNV) {
                        found_error = true;
                        skip |= LogError("VUID-RuntimeSpirv-OpTypeCooperativeMatrixKHR-10163", module_state.handle(), loc,
                                         "shader %s has\n%s (%s)\nbut doesn't match any VkCooperativeMatrixPropertiesKHR\n%s.",
                                         entrypoint.Describe().c_str(), insn.Describe().c_str(), m.Describe().c_str(),
                                         print_properties().c_str());
                    } else {
                        skip |= LogError("VUID-RuntimeSpirv-cooperativeMatrixFlexibleDimensions-10165", module_state.handle(), loc,
                                         "shader %s has\n%s (%s)\nbut doesn't match any VkCooperativeMatrixPropertiesKHR or "
                                         "VkCooperativeMatrixFlexibleDimensionsPropertiesNV\n%s\n%s.",
                                         entrypoint.Describe().c_str(), insn.Describe().c_str(), m.Describe().c_str(),
                                         print_properties().c_str(), print_flexible_properties().c_str());
                    }
                }
                if (IsExtEnabled(extensions.vk_nv_cooperative_matrix2)) {
                    if (m.rows > phys_dev_ext_props.cooperative_matrix_props2_nv.cooperativeMatrixFlexibleDimensionsMaxDimension ||
                        m.cols > phys_dev_ext_props.cooperative_matrix_props2_nv.cooperativeMatrixFlexibleDimensionsMaxDimension) {
                        skip |= LogError(
                            "VUID-RuntimeSpirv-cooperativeMatrixFlexibleDimensionsMaxDimension-10167", module_state.handle(), loc,
                            "shader %s has\n%s (%s)\nbut number of rows or columns is greater than "
                            "cooperativeMatrixFlexibleDimensionsMaxDimension (%" PRIu32 ").",
                            entrypoint.Describe().c_str(), insn.Describe().c_str(), m.Describe().c_str(),
                            phys_dev_ext_props.cooperative_matrix_props2_nv.cooperativeMatrixFlexibleDimensionsMaxDimension);
                        found_error = true;
                    }
                }

                break;
            }
            case spv::OpCooperativeMatrixMulAddKHR: {
                const uint32_t flags = insn.Length() > 6 ? insn.Word(6) : 0u;
                CoopMatType r(id_to_type_id[insn.Word(2)], module_state,
                              (flags & spv::CooperativeMatrixOperandsMatrixResultSignedComponentsKHRMask));
                CoopMatType a(id_to_type_id[insn.Word(3)], module_state,
                              (flags & spv::CooperativeMatrixOperandsMatrixASignedComponentsKHRMask));
                CoopMatType b(id_to_type_id[insn.Word(4)], module_state,
                              (flags & spv::CooperativeMatrixOperandsMatrixBSignedComponentsKHRMask));
                CoopMatType c(id_to_type_id[insn.Word(5)], module_state,
                              (flags & spv::CooperativeMatrixOperandsMatrixCSignedComponentsKHRMask));
                if (a.all_constant && b.all_constant && c.all_constant && r.all_constant) {
                    const auto signed_components_match = [flags](const auto& property) {
                        return IsSignedIntEnum(property.AType) ==
                                   !!(flags & spv::CooperativeMatrixOperandsMatrixASignedComponentsKHRMask) &&
                               IsSignedIntEnum(property.BType) ==
                                   !!(flags & spv::CooperativeMatrixOperandsMatrixBSignedComponentsKHRMask) &&
                               IsSignedIntEnum(property.CType) ==
                                   !!(flags & spv::CooperativeMatrixOperandsMatrixCSignedComponentsKHRMask) &&
                               IsSignedIntEnum(property.ResultType) ==
                                   !!(flags & spv::CooperativeMatrixOperandsMatrixResultSignedComponentsKHRMask);
                    };
                    // Validate that the type parameters are all supported for the same
                    // cooperative matrix property.
                    bool found_matching_prop = false;
                    for (uint32_t i = 0; i < device_state->cooperative_matrix_properties_khr.size(); ++i) {
                        const auto& property = device_state->cooperative_matrix_properties_khr[i];

                        bool valid = true;
                        valid &= property.AType == a.component_type && property.MSize == a.rows && property.KSize == a.cols &&
                                 property.scope == a.scope && a.use == spv::CooperativeMatrixUseMatrixAKHR;
                        valid &= property.BType == b.component_type && property.KSize == b.rows && property.NSize == b.cols &&
                                 property.scope == b.scope && b.use == spv::CooperativeMatrixUseMatrixBKHR;
                        valid &= property.CType == c.component_type && property.MSize == c.rows && property.NSize == c.cols &&
                                 property.scope == c.scope && c.use == spv::CooperativeMatrixUseMatrixAccumulatorKHR;
                        valid &= property.ResultType == r.component_type && property.MSize == r.rows && property.NSize == r.cols &&
                                 property.scope == r.scope && r.use == spv::CooperativeMatrixUseMatrixAccumulatorKHR;

                        valid &= signed_components_match(property);

                        valid &= property.saturatingAccumulation ==
                                 !!(flags & spv::CooperativeMatrixOperandsSaturatingAccumulationKHRMask);

                        if (valid) {
                            found_matching_prop = true;
                            break;
                        }
                    }
                    bool found_matching_flexible_prop = false;
                    if (enabled_features.cooperativeMatrixFlexibleDimensionsNV) {
                        for (uint32_t i = 0; i < device_state->cooperative_matrix_flexible_dimensions_properties.size(); ++i) {
                            const auto& property = device_state->cooperative_matrix_flexible_dimensions_properties[i];

                            bool valid = true;
                            valid &= property.AType == a.component_type && IsIntegerMultipleOf(a.rows, property.MGranularity) &&
                                     IsIntegerMultipleOf(a.cols, property.KGranularity) && property.scope == a.scope &&
                                     a.use == spv::CooperativeMatrixUseMatrixAKHR;
                            valid &= property.BType == b.component_type && IsIntegerMultipleOf(b.rows, property.KGranularity) &&
                                     IsIntegerMultipleOf(b.cols, property.NGranularity) && property.scope == b.scope &&
                                     b.use == spv::CooperativeMatrixUseMatrixBKHR;
                            valid &= property.CType == c.component_type && IsIntegerMultipleOf(c.rows, property.MGranularity) &&
                                     IsIntegerMultipleOf(c.cols, property.NGranularity) && property.scope == c.scope &&
                                     c.use == spv::CooperativeMatrixUseMatrixAccumulatorKHR;
                            valid &= property.ResultType == r.component_type &&
                                     IsIntegerMultipleOf(r.rows, property.MGranularity) &&
                                     IsIntegerMultipleOf(r.cols, property.NGranularity) && property.scope == r.scope &&
                                     r.use == spv::CooperativeMatrixUseMatrixAccumulatorKHR;

                            valid &= signed_components_match(property);

                            valid &= property.saturatingAccumulation ==
                                     !!(flags & spv::CooperativeMatrixOperandsSaturatingAccumulationKHRMask);

                            valid &= property.scope != VK_SCOPE_WORKGROUP_KHR || workgroup_size == property.workgroupInvocations;

                            if (valid) {
                                found_matching_flexible_prop = true;
                                break;
                            }
                        }
                    }
                    bool found_matching_properties2 = false;
                    if (!found_matching_prop && !found_matching_flexible_prop && enabled_features.cooperativeMatrixProperties2) {
                        const VkCooperativeMatrixFlagsEXT query_flags =
                            (flags & spv::CooperativeMatrixOperandsSaturatingAccumulationKHRMask)
                                ? VK_COOPERATIVE_MATRIX_SATURATING_ACCUMULATION_BIT_EXT
                                : 0;
                        const uint64_t query_invocations = a.scope == VK_SCOPE_WORKGROUP_KHR ? workgroup_size : 0;
                        auto properties2 = query_properties2(a.scope, query_invocations, properties2_subgroup_size, query_flags);
                        if (!properties2) {
                            // The invalid shader-derived selector is diagnosed by another VU.
                            break;
                        }
                        for (const auto& property : *properties2) {
                            bool valid = true;
                            valid &= a.scope == b.scope && a.scope == c.scope && a.scope == r.scope;
                            valid &= property.AType == a.component_type &&
                                     properties2_dimension_matches(a.rows, property.MGranularity) &&
                                     properties2_dimension_matches(a.cols, property.KGranularity) &&
                                     a.use == spv::CooperativeMatrixUseMatrixAKHR;
                            valid &= property.BType == b.component_type &&
                                     properties2_dimension_matches(b.rows, property.KGranularity) &&
                                     properties2_dimension_matches(b.cols, property.NGranularity) &&
                                     b.use == spv::CooperativeMatrixUseMatrixBKHR;
                            valid &= property.CType == c.component_type &&
                                     properties2_dimension_matches(c.rows, property.MGranularity) &&
                                     properties2_dimension_matches(c.cols, property.NGranularity) &&
                                     c.use == spv::CooperativeMatrixUseMatrixAccumulatorKHR;
                            valid &= property.ResultType == r.component_type &&
                                     properties2_dimension_matches(r.rows, property.MGranularity) &&
                                     properties2_dimension_matches(r.cols, property.NGranularity) &&
                                     r.use == spv::CooperativeMatrixUseMatrixAccumulatorKHR;

                            valid &= signed_components_match(property);

                            if (valid) {
                                found_matching_properties2 = true;
                                break;
                            }
                        }
                    }
                    if (!found_matching_prop && !found_matching_flexible_prop && !found_matching_properties2) {
                        found_error = true;
                        if (enabled_features.cooperativeMatrixProperties2) {
                            skip |= LogError("VUID-RuntimeSpirv-cooperativeMatrixProperties2-13383", module_state.handle(), loc,
                                             "shader %s instruction\n%s\ndoesn't match any supported "
                                             "VkCooperativeMatrixPropertiesKHR or VkCooperativeMatrixProperties2EXT\n"
                                             "%s\n%s\n%s\n%s\n",
                                             entrypoint.Describe().c_str(), insn.Describe().c_str(), a.Describe().c_str(),
                                             b.Describe().c_str(), c.Describe().c_str(), r.Describe().c_str());
                        } else if (!enabled_features.cooperativeMatrixFlexibleDimensionsNV) {
                            skip |= LogError("VUID-RuntimeSpirv-OpCooperativeMatrixMulAddKHR-10060", module_state.handle(), loc,
                                             "shader %s instruction\n%s\ndoesn't match a supported matrix "
                                             "VkCooperativeMatrixPropertiesKHR\n%s\n%s\n%s\n%s\n%s\n",
                                             entrypoint.Describe().c_str(), insn.Describe().c_str(), a.Describe().c_str(),
                                             b.Describe().c_str(), c.Describe().c_str(), r.Describe().c_str(),
                                             print_properties().c_str());
                        } else {
                            skip |=
                                LogError("VUID-RuntimeSpirv-cooperativeMatrixFlexibleDimensions-10166", module_state.handle(), loc,
                                         "shader %s instruction\n%s\ndoesn't match a supported matrix "
                                         "VkCooperativeMatrixPropertiesKHR or "
                                         "VkPhysicalDeviceCooperativeMatrix2PropertiesNV\n%s\n%s\n%s\n%s\n%s\n%s\n",
                                         entrypoint.Describe().c_str(), insn.Describe().c_str(), a.Describe().c_str(),
                                         b.Describe().c_str(), c.Describe().c_str(), r.Describe().c_str(),
                                         print_properties().c_str(), print_flexible_properties().c_str());
                        }
                    }
                }
                break;
            }
            case spv::OpTypeCooperativeMatrixNV: {
                CoopMatType m(insn.ResultId(), module_state, IsSignedIntType(insn.Word(2)));

                if (!m.all_constant) {
                    break;
                }
                // Validate that the type parameters are all supported for one of the
                // operands of a cooperative matrix property.
                bool valid = false;
                for (uint32_t i = 0; i < device_state->cooperative_matrix_properties_nv.size(); ++i) {
                    const auto& property = device_state->cooperative_matrix_properties_nv[i];
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
                    found_error = true;
                    skip |= LogError("VUID-RuntimeSpirv-OpTypeCooperativeMatrixNV-06316", module_state.handle(), loc,
                                     "shader %s has an OpTypeCooperativeMatrixNV (result id = %" PRIu32
                                     ") operand that don't match a supported matrix type (%s).",
                                     entrypoint.Describe().c_str(), insn.Word(1), m.Describe().c_str());
                }
                break;
            }
            case spv::OpCooperativeMatrixMulAddNV: {
                CoopMatType d(id_to_type_id[insn.Word(2)], module_state, IsSignedIntType(id_to_type_id[insn.Word(2)]));
                CoopMatType a(id_to_type_id[insn.Word(3)], module_state, IsSignedIntType(id_to_type_id[insn.Word(3)]));
                CoopMatType b(id_to_type_id[insn.Word(4)], module_state, IsSignedIntType(id_to_type_id[insn.Word(4)]));
                CoopMatType c(id_to_type_id[insn.Word(5)], module_state, IsSignedIntType(id_to_type_id[insn.Word(5)]));

                if (a.all_constant && b.all_constant && c.all_constant && d.all_constant) {
                    // Validate that the type parameters are all supported for the same
                    // cooperative matrix property.
                    bool valid_a = false;
                    bool valid_b = false;
                    bool valid_c = false;
                    bool valid_d = false;
                    for (uint32_t i = 0; i < device_state->cooperative_matrix_properties_nv.size(); ++i) {
                        const auto& property = device_state->cooperative_matrix_properties_nv[i];
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
                                         "shader %s OpCooperativeMatrixMulAddNV (result id = %" PRIu32
                                         ") operands don't match a supported matrix "
                                         "VkCooperativeMatrixPropertiesNV for A type (%s).",
                                         entrypoint.Describe().c_str(), insn.Word(2), a.Describe().c_str());
                        found_error = true;
                    } else if (!valid_b) {
                        skip |= LogError("VUID-RuntimeSpirv-OpTypeCooperativeMatrixMulAddNV-10059", module_state.handle(), loc,
                                         "shader %s OpCooperativeMatrixMulAddNV (result id = %" PRIu32
                                         ") operands don't match a supported matrix "
                                         "VkCooperativeMatrixPropertiesNV for B type (%s).",
                                         entrypoint.Describe().c_str(), insn.Word(2), b.Describe().c_str());
                        found_error = true;
                    } else if (!valid_c) {
                        skip |= LogError("VUID-RuntimeSpirv-OpTypeCooperativeMatrixMulAddNV-10059", module_state.handle(), loc,
                                         "shader %s OpCooperativeMatrixMulAddNV (result id = %" PRIu32
                                         ") operands don't match a supported matrix "
                                         "VkCooperativeMatrixPropertiesNV for C type (%s).",
                                         entrypoint.Describe().c_str(), insn.Word(2), c.Describe().c_str());
                        found_error = true;
                    } else if (!valid_d) {
                        skip |= LogError("VUID-RuntimeSpirv-OpTypeCooperativeMatrixMulAddNV-10059", module_state.handle(), loc,
                                         "shader %s OpCooperativeMatrixMulAddNV (result id = %" PRIu32
                                         ") operands don't match a supported matrix "
                                         "VkCooperativeMatrixPropertiesNV for D type (%s).",
                                         entrypoint.Describe().c_str(), insn.Word(2), d.Describe().c_str());
                        found_error = true;
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

bool CoreChecks::ValidateCooperativeVector(const spirv::Module& module_state, const spirv::EntryPoint& entrypoint,
                                           const Location& loc) const {
    bool skip = false;

    struct CoopVecType {
        VkComponentTypeKHR component_type;
        uint32_t component_count;
        bool all_constant;

        CoopVecType(uint32_t id, const spirv::Module& module_state, bool is_signed) {
            const spirv::Instruction* insn = module_state.FindDef(id);
            const spirv::Instruction* component_type_insn = module_state.FindDef(insn->Word(2));
            const spirv::Instruction* component_count_insn = module_state.FindDef(insn->Word(3));

            all_constant = true;
            if (!module_state.GetInt32IfConstant(*component_count_insn, &component_count)) {
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
                "shader %s contains cooperative vector capability but %s is not in "
                "cooperativeVectorSupportedStages (%s)",
                entrypoint.Describe().c_str(), string_VkShaderStageFlagBits(entrypoint.stage),
                string_VkShaderStageFlags(phys_dev_ext_props.cooperative_vector_props_nv.cooperativeVectorSupportedStages).c_str());
        }
    } else {
        return skip;
    }

    vvl::unordered_map<uint32_t, uint32_t> id_to_type_id;
    for (const spirv::Instruction& insn : module_state.GetInstructions()) {
        if (OpcodeHasType(insn.Opcode()) && OpcodeHasResult(insn.Opcode())) {
            id_to_type_id[insn.Word(2)] = insn.Word(1);
        }
    }
    for (const spirv::Instruction* cooperative_vector_inst : module_state.static_data_.cooperative_vector_inst) {
        const spirv::Instruction& insn = *cooperative_vector_inst;
        switch (insn.Opcode()) {
            case spv::OpTypeCooperativeVectorNV: {
                // SPIR-V integer types are not strictly signed or unsigned. Allow this type to
                // match against either signed or unsigned types in the device properties.
                CoopVecType m_signed(insn.Word(1), module_state, true);
                CoopVecType m_unsigned(insn.Word(1), module_state, false);

                if (!m_signed.all_constant) {
                    break;
                }

                if (m_signed.component_count > phys_dev_ext_props.cooperative_vector_props_nv.maxCooperativeVectorComponents) {
                    skip |= LogError("VUID-RuntimeSpirv-maxCooperativeVectorComponents-10094", module_state.handle(), loc,
                                     "shader %s has a cooperative vector component count (%" PRIu32
                                     ") which is greater than maxCooperativeVectorComponents (%" PRIu32 ")",
                                     entrypoint.Describe().c_str(), m_signed.component_count,
                                     phys_dev_ext_props.cooperative_vector_props_nv.maxCooperativeVectorComponents);
                }

                bool found = false;
                for (uint32_t i = 0; i < device_state->cooperative_vector_properties_nv.size(); ++i) {
                    const auto& property = device_state->cooperative_vector_properties_nv[i];
                    if (m_signed.component_type == property.inputType || m_signed.component_type == property.resultType ||
                        m_unsigned.component_type == property.inputType || m_unsigned.component_type == property.resultType) {
                        found = true;
                        break;
                    }
                }

                if (!found) {
                    skip |= LogError("VUID-RuntimeSpirv-OpTypeCooperativeVector-10095", module_state.handle(), loc,
                                     "shader %s contains unsupported cooperative vector component type (%s)",
                                     entrypoint.Describe().c_str(),
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

                CoopVecType result(id_to_type_id[insn.Word(2)], module_state, result_is_signed);
                CoopVecType input(id_to_type_id[insn.Word(3)], module_state, input_is_signed);

                uint32_t result_type = result.component_type;
                uint32_t input_type = input.component_type;

                uint32_t biasOffset = insn.Opcode() == spv::OpCooperativeVectorMatrixMulAddNV ? 3 : 0;

                bool all_constant = true;
                uint32_t input_interpretation{};
                uint32_t matrix_interpretation{};
                uint32_t bias_interpretation{};
                bool transpose{};
                if (!module_state.GetInt32IfConstant(*module_state.FindDef(insn.Word(4)), &input_interpretation)) {
                    all_constant = false;
                }
                if (!module_state.GetInt32IfConstant(*module_state.FindDef(insn.Word(7)), &matrix_interpretation)) {
                    all_constant = false;
                }
                if (insn.Opcode() == spv::OpCooperativeVectorMatrixMulAddNV) {
                    if (!module_state.GetInt32IfConstant(*module_state.FindDef(insn.Word(10)), &bias_interpretation)) {
                        all_constant = false;
                    }
                }
                if (!module_state.GetBoolIfConstant(*module_state.FindDef(insn.Word(11 + biasOffset)), &transpose)) {
                    all_constant = false;
                }

                if (!all_constant) {
                    break;
                }

                bool found = false;
                for (uint32_t i = 0; i < device_state->cooperative_vector_properties_nv.size(); ++i) {
                    const auto& property = device_state->cooperative_vector_properties_nv[i];
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
                                     "shader %s contains unsupported cooperative vector matrix mul with "
                                     "result component type (%s), input component type (%s), input interpretation (%s), "
                                     "matrix interpretation (%s), bias interpretation (%s), transpose (%d)",
                                     entrypoint.Describe().c_str(), string_VkComponentTypeKHR((VkComponentTypeKHR)result_type),
                                     string_VkComponentTypeKHR((VkComponentTypeKHR)input_type),
                                     string_VkComponentTypeKHR((VkComponentTypeKHR)input_interpretation),
                                     string_VkComponentTypeKHR((VkComponentTypeKHR)matrix_interpretation),
                                     (insn.Opcode() == spv::OpCooperativeVectorMatrixMulNV
                                          ? "None"
                                          : string_VkComponentTypeKHR((VkComponentTypeKHR)bias_interpretation)),
                                     transpose);
                }

                uint32_t memory_layout{};
                if (module_state.GetInt32IfConstant(*module_state.FindDef(insn.Word(10 + biasOffset)), &memory_layout)) {
                    if ((matrix_interpretation == VK_COMPONENT_TYPE_FLOAT_E4M3_NV ||
                         matrix_interpretation == VK_COMPONENT_TYPE_FLOAT_E5M2_NV) &&
                        !(memory_layout == VK_COOPERATIVE_VECTOR_MATRIX_LAYOUT_INFERENCING_OPTIMAL_NV ||
                          memory_layout == VK_COOPERATIVE_VECTOR_MATRIX_LAYOUT_TRAINING_OPTIMAL_NV)) {
                        skip |= LogError(
                            "VUID-RuntimeSpirv-OpCooperativeVectorMatrixMulNV-10090", module_state.handle(), loc,
                            "shader %s contains unsupported cooperative vector matrix mul with "
                            "matrix_interpretation (%s) and memory layout (%s)",
                            entrypoint.Describe().c_str(), string_VkComponentTypeKHR((VkComponentTypeKHR)matrix_interpretation),
                            string_VkCooperativeVectorMatrixLayoutNV((VkCooperativeVectorMatrixLayoutNV)memory_layout));
                    }
                }
                break;
            }
            case spv::OpCooperativeVectorReduceSumAccumulateNV: {
                CoopVecType v(id_to_type_id[insn.Word(3)], module_state, false);

                switch (v.component_type) {
                    case VK_COMPONENT_TYPE_FLOAT16_KHR:
                        if (!phys_dev_ext_props.cooperative_vector_props_nv.cooperativeVectorTrainingFloat16Accumulation) {
                            skip |= LogError("VUID-RuntimeSpirv-OpCooperativeVectorReduceSumAccumulateNV-10092",
                                             module_state.handle(), loc,
                                             "shader %s cooperative vector component type is FLOAT16 but "
                                             "cooperativeVectorTrainingFloat16Accumulation not supported",
                                             entrypoint.Describe().c_str());
                        }
                        break;
                    case VK_COMPONENT_TYPE_FLOAT32_KHR:
                        if (!phys_dev_ext_props.cooperative_vector_props_nv.cooperativeVectorTrainingFloat32Accumulation) {
                            skip |= LogError("VUID-RuntimeSpirv-OpCooperativeVectorReduceSumAccumulateNV-10092",
                                             module_state.handle(), loc,
                                             "shader %s cooperative vector component type is FLOAT32 but "
                                             "cooperativeVectorTrainingFloat32Accumulation not supported",
                                             entrypoint.Describe().c_str());
                        }
                        break;
                    default:
                        skip |= LogError("VUID-RuntimeSpirv-OpCooperativeVectorReduceSumAccumulateNV-10092", module_state.handle(),
                                         loc, "shader %s has an unsupported component type (%s)", entrypoint.Describe().c_str(),
                                         string_VkComponentTypeKHR((VkComponentTypeKHR)v.component_type));
                        break;
                }

                const spirv::Instruction* ptr_type = module_state.FindDef(id_to_type_id[insn.Word(1)]);
                if (ptr_type->StorageClass() != spv::StorageClassStorageBuffer &&
                    ptr_type->StorageClass() != spv::StorageClassPhysicalStorageBuffer) {
                    skip |= LogError("VUID-RuntimeSpirv-OpCooperativeVectorReduceSumAccumulateNV-10092", module_state.handle(), loc,
                                     "shader %s has an unsupported pointer storage class (%s)", entrypoint.Describe().c_str(),
                                     string_SpvStorageClass(ptr_type->StorageClass()));
                }

                break;
            }

            case spv::OpCooperativeVectorOuterProductAccumulateNV: {
                uint32_t matrix_interpretation{};
                if (module_state.GetInt32IfConstant(*module_state.FindDef(insn.Word(6)), &matrix_interpretation)) {
                    switch (matrix_interpretation) {
                        case VK_COMPONENT_TYPE_FLOAT16_KHR:
                            if (!phys_dev_ext_props.cooperative_vector_props_nv.cooperativeVectorTrainingFloat16Accumulation) {
                                skip |= LogError("VUID-RuntimeSpirv-OpCooperativeVectorOuterProductAccumulateNV-10093",
                                                 module_state.handle(), loc,
                                                 "shader %s matrix interpretation is FLOAT16 but "
                                                 "cooperativeVectorTrainingFloat16Accumulation not supported",
                                                 entrypoint.Describe().c_str());
                            }
                            break;
                        case VK_COMPONENT_TYPE_FLOAT32_KHR:
                            if (!phys_dev_ext_props.cooperative_vector_props_nv.cooperativeVectorTrainingFloat32Accumulation) {
                                skip |= LogError("VUID-RuntimeSpirv-OpCooperativeVectorOuterProductAccumulateNV-10093",
                                                 module_state.handle(), loc,
                                                 "shader %s matrix interpretation is FLOAT32 but "
                                                 "cooperativeVectorTrainingFloat32Accumulation not supported",
                                                 entrypoint.Describe().c_str());
                            }
                            break;
                        default:
                            skip |= LogError("VUID-RuntimeSpirv-OpCooperativeVectorOuterProductAccumulateNV-10093",
                                             module_state.handle(), loc, "shader %s has unsupported Matrix interpretation (%s)",
                                             entrypoint.Describe().c_str(),
                                             string_VkComponentTypeKHR((VkComponentTypeKHR)matrix_interpretation));
                            break;
                    }
                }

                CoopVecType a(id_to_type_id[insn.Word(3)], module_state, false);
                CoopVecType b(id_to_type_id[insn.Word(4)], module_state, false);

                if (a.component_type != VK_COMPONENT_TYPE_FLOAT16_KHR) {
                    skip |=
                        LogError("VUID-RuntimeSpirv-OpCooperativeVectorOuterProductAccumulateNV-10093", module_state.handle(), loc,
                                 "shader %s has a component type of A (%s) but it must be FLOAT16", entrypoint.Describe().c_str(),
                                 string_VkComponentTypeKHR((VkComponentTypeKHR)a.component_type));
                }
                if (b.component_type != VK_COMPONENT_TYPE_FLOAT16_KHR) {
                    skip |=
                        LogError("VUID-RuntimeSpirv-OpCooperativeVectorOuterProductAccumulateNV-10093", module_state.handle(), loc,
                                 "shader %s has a component type of B (%s) but it must be FLOAT16", entrypoint.Describe().c_str(),
                                 string_VkComponentTypeKHR((VkComponentTypeKHR)b.component_type));
                }

                uint32_t memory_layout{};
                if (module_state.GetInt32IfConstant(*module_state.FindDef(insn.Word(5)), &memory_layout)) {
                    if (memory_layout != VK_COOPERATIVE_VECTOR_MATRIX_LAYOUT_TRAINING_OPTIMAL_NV) {
                        skip |= LogError(
                            "VUID-RuntimeSpirv-OpCooperativeVectorOuterProductAccumulateNV-10093", module_state.handle(), loc,
                            "shader %s has memory layout (%s) but it must be TRAINING_OPTIMAL", entrypoint.Describe().c_str(),
                            string_VkCooperativeVectorMatrixLayoutNV((VkCooperativeVectorMatrixLayoutNV)memory_layout));
                    }
                }

                const spirv::Instruction* ptr_type = module_state.FindDef(id_to_type_id[insn.Word(1)]);
                if (ptr_type->StorageClass() != spv::StorageClassStorageBuffer &&
                    ptr_type->StorageClass() != spv::StorageClassPhysicalStorageBuffer) {
                    skip |= LogError("VUID-RuntimeSpirv-OpCooperativeVectorOuterProductAccumulateNV-10093", module_state.handle(),
                                     loc, "shader %s has unsupported pointer storage class (%s)", entrypoint.Describe().c_str(),
                                     string_SpvStorageClass(ptr_type->StorageClass()));
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