/* Copyright (c) 2024-2026 LunarG, Inc.
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

#include "containers/limits.h"
#include "containers/small_vector.h"
#include "generated/spirv_grammar_helper.h"
#include "module.h"
#include <cassert>
#include <cstdint>
#include <cstring>
#include <spirv/unified1/spirv.hpp>
#include "containers/custom_containers.h"
#include "function_basic_block.h"

namespace gpuav {
namespace spirv {

void Module::SetSpecConstantValue(Instruction* inst, const Type& type, vvl::unordered_map<uint32_t, uint32_t>& id_to_spec_id) {
    const uint32_t opcode = inst->Opcode();

    if (opcode == spv::OpSpecConstantComposite) {
        return inst->FreezeSpecConstant();
    }

    const bool has_spec_info = interface_.specialization_info && interface_.specialization_info->mapEntryCount > 0;
    if (!has_spec_info) {
        return inst->FreezeSpecConstant();
    }

    const uint32_t result_id = inst->ResultId();
    const auto it = id_to_spec_id.find(result_id);
    if (it == id_to_spec_id.end()) {
        // OpDecorate SpecId was not set, so using default
        return inst->FreezeSpecConstant();
    }
    const uint32_t spec_id = it->second;

    VkSpecializationMapEntry map_entry = {0, 0, 0};
    bool found = false;
    for (uint32_t i = 0; i < interface_.specialization_info->mapEntryCount; i++) {
        if (interface_.specialization_info->pMapEntries[i].constantID == spec_id) {
            map_entry = interface_.specialization_info->pMapEntries[i];
            found = true;
            break;
        }
    }

    if (!found) {
        return inst->FreezeSpecConstant();
    }

    if ((map_entry.offset + map_entry.size) <= interface_.specialization_info->dataSize) {
        // Spec constants at most can be a int64/float64
        assert(map_entry.size <= 8);
        const uint8_t* out_p = static_cast<const uint8_t*>(interface_.specialization_info->pData);
        const uint8_t* target_addr = out_p + map_entry.offset;

        if (opcode == spv::OpSpecConstantTrue || opcode == spv::OpSpecConstantFalse) {
            // For Boolean, just swap from True <-> False spec constant, then will be frozen
            VkBool32 raw_value = 0;
            std::memcpy(&raw_value, target_addr, std::min(map_entry.size, sizeof(VkBool32)));
            inst->SetNewOpcode(raw_value ? spv::OpSpecConstantTrue : spv::OpSpecConstantFalse);
        } else {
            assert(opcode == spv::OpSpecConstant);
            const bool is_signed = type.IsSignedInt();

            uint64_t raw_value = 0;
            std::memcpy(&raw_value, target_addr, map_entry.size);

            // Sign-extend 8-bit and 16-bit negative numbers up to 32 bits
            if (is_signed && map_entry.size < 4) {
                const uint32_t bit_width = static_cast<uint32_t>(map_entry.size * 8);
                const uint64_t sign_bit = 1ULL << (bit_width - 1);

                // If the sign bit is 1, fill the upper bits with 1s
                if (raw_value & sign_bit) {
                    uint64_t mask = ~0ULL << bit_width;
                    raw_value |= mask;
                }
            }

            const uint32_t word_0 = static_cast<uint32_t>(raw_value & 0xFFFFFFFF);
            inst->UpdateWord(3, word_0);
            if (map_entry.size == 8) {
                const uint32_t word_1 = static_cast<uint32_t>(raw_value >> 32);
                inst->UpdateWord(4, word_1);
            }
        }
    }

    return inst->FreezeSpecConstant();
}

static uint64_t EvaluateArithmetic(spv::Op opcode, uint64_t arg0, uint64_t arg1, uint32_t bit_width) {
    uint64_t mask = (bit_width == 64) ? ~0ULL : (1ULL << bit_width) - 1;

    uint64_t u_a = arg0;
    uint64_t u_b = arg1;
    int64_t s_a = static_cast<int64_t>(arg0);
    int64_t s_b = static_cast<int64_t>(arg1);

    uint64_t res = 0;

    switch (opcode) {
        case spv::OpSNegate:
            res = static_cast<uint64_t>(-s_a);
            break;
        case spv::OpSDiv:
            if (s_b != 0) {
                if (bit_width == 64 && s_a == vvl::kI64Min && s_b == -1) {
                    res = static_cast<uint64_t>(vvl::kI64Min);
                } else {
                    res = static_cast<uint64_t>(s_a / s_b);
                }
            }
            break;
        case spv::OpSRem:
            if (s_b != 0) {
                if (bit_width == 64 && s_a == vvl::kI64Min && s_b == -1) {
                    res = 0;
                } else {
                    res = static_cast<uint64_t>(s_a % s_b);
                }
            }
            break;
        case spv::OpSMod:
            if (s_b != 0) {
                if (bit_width == 64 && s_a == vvl::kI64Min && s_b == -1) {
                    res = 0;
                } else {
                    res = static_cast<uint64_t>(s_a % s_b);
                    if ((res > 0 && s_b < 0) || s_b > 0) {
                        res += u_b;
                    }
                }
            }
            break;
        case spv::OpShiftRightArithmetic:
            if (u_b >= 64) {
                res = (s_a < 0) ? -1 : 0;
            } else {
                res = static_cast<uint64_t>(s_a >> u_b);
            }
            break;
        case spv::OpSLessThan:
            res = (s_a < s_b) ? 1 : 0;
            break;
        case spv::OpSGreaterThan:
            res = (s_a > s_b) ? 1 : 0;
            break;
        case spv::OpSLessThanEqual:
            res = (s_a <= s_b) ? 1 : 0;
            break;
        case spv::OpSGreaterThanEqual:
            res = (s_a >= s_b) ? 1 : 0;
            break;

        case spv::OpIAdd:
            res = u_a + u_b;
            break;
        case spv::OpISub:
            res = u_a - u_b;
            break;
        case spv::OpIMul:
            res = u_a * u_b;
            break;
        case spv::OpUDiv:
            if (u_b != 0) res = u_a / u_b;
            break;
        case spv::OpUMod:
            if (u_b != 0) res = u_a % u_b;
            break;
        case spv::OpShiftRightLogical:
            res = (u_b >= 64) ? 0 : (u_a >> u_b);
            break;
        case spv::OpShiftLeftLogical:
            res = (u_b >= 64) ? 0 : (u_a << u_b);
            break;
        case spv::OpBitwiseOr:
            res = u_a | u_b;
            break;
        case spv::OpBitwiseXor:
            res = u_a ^ u_b;
            break;
        case spv::OpBitwiseAnd:
            res = u_a & u_b;
            break;
        case spv::OpNot:
            res = ~u_a;
            break;

        case spv::OpLogicalOr:
            res = (u_a || u_b) ? 1 : 0;
            break;
        case spv::OpLogicalAnd:
            res = (u_a && u_b) ? 1 : 0;
            break;
        case spv::OpLogicalNot:
            res = (!u_a) ? 1 : 0;
            break;
        case spv::OpLogicalEqual:
        case spv::OpIEqual:
            res = (u_a == u_b) ? 1 : 0;
            break;
        case spv::OpLogicalNotEqual:
        case spv::OpINotEqual:
            res = (u_a != u_b) ? 1 : 0;
            break;
        case spv::OpULessThan:
            res = (u_a < u_b) ? 1 : 0;
            break;
        case spv::OpUGreaterThan:
            res = (u_a > u_b) ? 1 : 0;
            break;
        case spv::OpULessThanEqual:
            res = (u_a <= u_b) ? 1 : 0;
            break;
        case spv::OpUGreaterThanEqual:
            res = (u_a >= u_b) ? 1 : 0;
            break;
        default:
            assert(false);  // Only a limited set of instructions allowed
            return 0;
    }

    return res & mask;
}

bool Module::ConstantFold(Instruction* inst, const Type& result_type) {
    assert(inst->Opcode() == spv::OpSpecConstantOp);
    if (result_type.spv_type_ == SpvType::kStruct) {
        return false;  // TODO - Add support
    }

    spv::Op target_opcode = (spv::Op)inst->Word(3);

    const bool is_vector = result_type.spv_type_ == SpvType::kVector;
    uint32_t vector_length = is_vector ? result_type.inst_.Word(3) : 1;
    const Type& scalar_type = is_vector ? *type_manager_.FindTypeById(result_type.inst_.Word(2)) : result_type;

    small_vector<uint32_t, 4> new_composite_components;

    // Scalar will have a single lane
    for (uint32_t lane = 0; lane < vector_length; lane++) {
        // might be up to 3 for OpSelect
        small_vector<uint64_t, 3> args;

        for (uint32_t i = 4; i < inst->Length(); i++) {
            const uint32_t operand_id = inst->Word(i);
            const Constant* constant = type_manager_.FindConstantById(operand_id);
            if (!constant) {
                assert(false);  // TODO
                return false;
            }

            const Constant* lane_constant = constant;
            if (is_vector && constant->inst_.Opcode() == spv::OpConstantComposite) {
                uint32_t comp_id = constant->inst_.Word(3 + lane);
                lane_constant = type_manager_.FindConstantById(comp_id);
                if (!lane_constant) {
                    assert(false);
                    return false;
                }
            }

            if (lane_constant->inst_.Opcode() == spv::OpConstantNull) {
                args.emplace_back(0ul);
            } else if (lane_constant->type_.spv_type_ == SpvType::kBool) {
                if (lane_constant->inst_.Opcode() == spv::OpConstantTrue) {
                    args.emplace_back(1ul);
                } else {
                    assert(lane_constant->inst_.Opcode() == spv::OpConstantFalse);
                    args.emplace_back(0ul);
                }
            } else {
                bool op_is_signed = lane_constant->type_.inst_.Word(3) != 0;

                if (target_opcode == spv::OpSConvert) {
                    // OpSConvert implies the source is treated as signed and must sign-extend
                    op_is_signed = true;
                } else if (target_opcode == spv::OpUConvert) {
                    // OpUConvert implies the source is treated as unsigned (zero-extends)
                    op_is_signed = false;
                }

                const uint64_t value64 = lane_constant->GetValueUint64(op_is_signed);
                args.emplace_back(value64);
            }
        }

        uint64_t lane_result = 0;
        const uint32_t result_bit_width = scalar_type.inst_.GetBitWidth();
        if (target_opcode == spv::OpSConvert || target_opcode == spv::OpUConvert) {
            uint64_t mask = (result_bit_width == 64) ? ~0ULL : (1ULL << result_bit_width) - 1;
            lane_result = args[0] & mask;
        } else if (target_opcode == spv::OpSelect) {
            lane_result = (args[0] != 0) ? args[1] : args[2];
        } else {
            const uint64_t arg1 = args.size() > 1 ? args[1] : 0;
            lane_result = EvaluateArithmetic(target_opcode, args[0], arg1, result_bit_width);
        }

        if (is_vector) {
            // If we have
            //   %12 = OpSpecConstantComposite %v2short %short_4 %short_4
            //   %13 = OpSpecConstantOp %v2short IMul %12 %12
            // we will want
            //   %12 = OpConstantComposite %v2short %short_4 %short_4
            //   %short_16 = OpConstant %short 16
            //   %13 = OpConstantComposite %v2short %short_16 %short_16
            // Which means we need to need to "create" the new OpConstant here
            uint32_t scalar_id = type_manager_.CreateConstantScalar(lane_result, scalar_type).Id();
            new_composite_components.emplace_back(scalar_id);
        } else if (scalar_type.spv_type_ == SpvType::kBool) {
            const spv::Op new_opcode = (lane_result == 0) ? spv::OpConstantFalse : spv::OpConstantTrue;
            auto new_inst = std::make_unique<Instruction>(3, new_opcode);
            new_inst->Fill({scalar_type.Id(), inst->ResultId()});
            type_manager_.AddConstant(std::move(new_inst), scalar_type);
            return true;
        } else {
            type_manager_.CreateConstantScalar(lane_result, scalar_type, inst->ResultId());
            return true;
        }
    }

    assert(is_vector);
    auto new_inst = std::make_unique<Instruction>(3 + new_composite_components.size(), spv::OpConstantComposite);
    std::vector<uint32_t> words = {result_type.Id(), inst->ResultId()};
    words.insert(words.end(), new_composite_components.begin(), new_composite_components.end());
    new_inst->Fill(words);
    type_manager_.AddConstant(std::move(new_inst), result_type);
    return true;
}

}  // namespace spirv
}  // namespace gpuav