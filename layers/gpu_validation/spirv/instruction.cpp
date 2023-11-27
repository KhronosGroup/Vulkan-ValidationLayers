/* Copyright (c) 2023 LunarG, Inc.
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

#include "instruction.h"
#include "generated/spirv_grammar_helper.h"
#include "module.h"

namespace gpuav {
namespace spirv {

void Instruction::SetResultTypeIndex() {
    const bool has_result = OpcodeHasResult(Opcode());
    if (OpcodeHasType(Opcode())) {
        type_id_index_ = 1;
        if (has_result) {
            result_id_index_ = 2;
        }
    } else if (has_result) {
        result_id_index_ = 1;
    }
}

Instruction::Instruction(std::vector<uint32_t>::const_iterator it, uint32_t position) : position_index_(position) {
    words_.emplace_back(*it++);
    words_.reserve(Length());
    for (uint32_t i = 1; i < Length(); i++) {
        words_.emplace_back(*it++);
    }

    SetResultTypeIndex();
}

Instruction::Instruction(const uint32_t* words) {
    words_.emplace_back(words[0]);
    words_.reserve(Length());
    for (uint32_t i = 1; i < Length(); i++) {
        words_.emplace_back(words[i]);
    }

    SetResultTypeIndex();
}

Instruction::Instruction(uint32_t length, spv::Op opcode) {
    words_.reserve(length);
    uint32_t first_word = (length << 16) | opcode;
    words_.emplace_back(first_word);

    SetResultTypeIndex();
}

void Instruction::AppendWord(uint32_t word) {
    words_.emplace_back(word);
    const uint32_t new_length = Length() + 1;
    uint32_t first_word = (new_length << 16) | Opcode();
    words_[0] = first_word;
}

void Instruction::ToBinary(std::vector<uint32_t>& out) {
    for (auto word : words_) {
        out.push_back(word);
    }
}

// The main challenge with linking to functions from 2 modules is the IDs overlap.
// Taking advantage that we create and control all the incoming linked functions, we know the limited set of instructions that will
// ever be used. With this, instead of trying to over-engineer something generated to find all reference IDs in each instructions,
// we can just add support for each instruction that we need to swap IDs for.
void Instruction::ReplaceLinkedId(vvl::unordered_map<uint32_t, uint32_t>& id_swap_map) {
    auto swap = [this, &id_swap_map](uint32_t index) {
        uint32_t old_id = words_[index];
        uint32_t new_id = id_swap_map[old_id];
        assert(new_id != 0);
        words_[index] = new_id;
    };

    auto swap_to_end = [this, swap](uint32_t start_index) {
        for (uint32_t i = start_index; i < Length(); i++) {
            swap(i);
        }
    };

    // Swap all Reference IDs (ignores Result ID)
    switch (Opcode()) {
        case spv::OpCompositeExtract:
        case spv::OpLoad:
        case spv::OpArrayLength:
        case spv::OpBitcast:
            swap(1);
            swap(3);
            return;
        case spv::OpFAdd:
        case spv::OpIAdd:
        case spv::OpISub:
        case spv::OpFSub:
        case spv::OpIMul:
        case spv::OpFMul:
        case spv::OpUDiv:
        case spv::OpSDiv:
        case spv::OpFDiv:
        case spv::OpUMod:
        case spv::OpSRem:
        case spv::OpSMod:
        case spv::OpFRem:
        case spv::OpFMod:
        case spv::OpIEqual:
        case spv::OpINotEqual:
        case spv::OpUGreaterThan:
        case spv::OpSGreaterThan:
        case spv::OpUGreaterThanEqual:
        case spv::OpSGreaterThanEqual:
        case spv::OpULessThan:
        case spv::OpSLessThan:
        case spv::OpULessThanEqual:
        case spv::OpSLessThanEqual:
        case spv::OpFOrdEqual:
        case spv::OpFUnordEqual:
        case spv::OpFOrdNotEqual:
        case spv::OpFUnordNotEqual:
        case spv::OpFOrdLessThan:
        case spv::OpFUnordLessThan:
        case spv::OpFOrdGreaterThan:
        case spv::OpFUnordGreaterThan:
        case spv::OpFOrdLessThanEqual:
        case spv::OpFUnordLessThanEqual:
        case spv::OpFOrdGreaterThanEqual:
        case spv::OpFUnordGreaterThanEqual:
        case spv::OpLogicalEqual:
        case spv::OpLogicalNotEqual:
        case spv::OpLogicalOr:
        case spv::OpLogicalAnd:
        case spv::OpLogicalNot:
        case spv::OpShiftRightLogical:
        case spv::OpShiftRightArithmetic:
        case spv::OpShiftLeftLogical:
        case spv::OpBitwiseOr:
        case spv::OpBitwiseXor:
        case spv::OpBitwiseAnd:
            swap(1);
            swap(3);
            swap(4);
            return;
        case spv::OpStore:
        case spv::OpLoopMerge:
            swap(1);
            swap(2);
            return;
        case spv::OpReturnValue:
        case spv::OpFunctionParameter:
        case spv::OpVariable:  // never use optional initializer
        case spv::OpConstantTrue:
        case spv::OpSpecConstantTrue:
        case spv::OpConstantFalse:
        case spv::OpSpecConstantFalse:
        case spv::OpConstant:
        case spv::OpSpecConstant:
        case spv::OpConstantNull:
        case spv::OpSelectionMerge:
        case spv::OpBranch:
        case spv::OpDecorate:
        case spv::OpMemberDecorate:
            swap(1);
            return;
        case spv::OpAtomicStore:
        case spv::OpBranchConditional:
            swap_to_end(1);
            return;
        case spv::OpAtomicLoad:
        case spv::OpAtomicExchange:
        case spv::OpAtomicCompareExchange:
        case spv::OpAtomicCompareExchangeWeak:
        case spv::OpAtomicIIncrement:
        case spv::OpAtomicIDecrement:
        case spv::OpAtomicIAdd:
        case spv::OpAtomicISub:
        case spv::OpAtomicSMin:
        case spv::OpAtomicUMin:
        case spv::OpAtomicSMax:
        case spv::OpAtomicUMax:
        case spv::OpAtomicAnd:
        case spv::OpAtomicOr:
        case spv::OpAtomicXor:
        case spv::OpPhi:
        case spv::OpAccessChain:
        case spv::OpConstantComposite:
        case spv::OpSpecConstantComposite:
            swap(1);
            swap_to_end(3);
            return;
        case spv::OpTypeStruct:
        case spv::OpTypeFunction:
            swap_to_end(2);
            return;

        case spv::OpReturn:
        case spv::OpLabel:
        case spv::OpFunctionEnd:
            return;  // Instructions aware of, but nothing to swap
        default:
            assert(false && "Need to add support for new instruction");
    }
}

}  // namespace spirv
}  // namespace gpuav