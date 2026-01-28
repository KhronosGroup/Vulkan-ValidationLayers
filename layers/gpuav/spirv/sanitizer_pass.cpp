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

#include "sanitizer_pass.h"
#include "containers/container_utils.h"
#include "function_basic_block.h"
#include "gpuav/shaders/gpuav_error_codes.h"
#include "module.h"
#include <spirv/unified1/GLSL.std.450.h>
#include <cstdint>
#include <spirv/unified1/spirv.hpp>
#include <iostream>

#include "generated/gpuav_offline_spirv.h"

namespace gpuav {
namespace spirv {

const static OfflineModule kOfflineModule = {instrumentation_sanitizer_comp, instrumentation_sanitizer_comp_size,
                                             UseErrorPayloadVariable};

const static OfflineFunction kOfflineFunctions[glsl::kErrorSubCode_Sanitizer_Count] = {
    {"empty", 0},
    {"inst_sanitizer_divide_by_zero", instrumentation_sanitizer_comp_function_0_offset},
    {"inst_sanitizer_image_gather", instrumentation_sanitizer_comp_function_1_offset},
    {"inst_sanitizer_pow", instrumentation_sanitizer_comp_function_2_offset},
    {"inst_sanitizer_atan2", instrumentation_sanitizer_comp_function_3_offset},
    {"inst_sanitizer_fminmax", instrumentation_sanitizer_comp_function_4_offset},
};

SanitizerPass::SanitizerPass(Module& module) : Pass(module, kOfflineModule) {
    for (uint32_t i = 0; i < glsl::kErrorSubCode_Sanitizer_Count; i++) {
        link_function_ids_[i] = 0;
    }
}

// By appending the LinkInfo, it will attempt at linking stage to add the function.
uint32_t SanitizerPass::GetLinkFunctionId(uint32_t sub_code) {
    return GetLinkFunction(link_function_ids_[sub_code], kOfflineFunctions[sub_code]);
}

// We do the check for zero in C++ to make it easier to handle the various cases of signed/unsigned/64bit/etc
// Returns an ID of type OpTypeBool
uint32_t SanitizerPass::DivideByZeroCheck(BasicBlock& block, InstructionIt* inst_it, const InstructionMeta& meta) {
    const Type& bool_type = type_manager_.GetTypeBool();
    const uint32_t vector_size = meta.result_type->VectorSize();
    const uint32_t divisor_id = meta.target_instruction->Word(4);

    const bool is_float = meta.target_instruction->Opcode() == spv::OpFMod || meta.target_instruction->Opcode() == spv::OpFRem;
    const spv::Op compare_op = is_float ? spv::OpFOrdEqual : spv::OpIEqual;

    if (vector_size == 0) {
        const uint32_t null_type_id = type_manager_.GetConstantNull(*meta.result_type).Id();
        const uint32_t compare_id = module_.TakeNextId();
        block.CreateInstruction(compare_op, {bool_type.Id(), compare_id, null_type_id, divisor_id}, inst_it);
        return compare_id;
    } else {
        const uint32_t bool_vector_type_id = type_manager_.GetTypeVector(bool_type, vector_size).Id();
        const uint32_t zero_id = type_manager_.GetConstantZeroVector(*meta.result_type).Id();

        const uint32_t compare_id = module_.TakeNextId();
        block.CreateInstruction(compare_op, {bool_vector_type_id, compare_id, zero_id, divisor_id}, inst_it);

        const uint32_t any_id = module_.TakeNextId();
        block.CreateInstruction(spv::OpAny, {bool_type.Id(), any_id, compare_id}, inst_it);
        return any_id;
    }
}

// Based off https://godbolt.org/z/dbToMGKTd - but found can hand-roll the spirv much better
// bool is_invalid = (x < 0.0 || (x == 0.0 && y <= 0.0));
// Returns an ID of type OpTypeBool
uint32_t SanitizerPass::PowCheck(BasicBlock& block, InstructionIt* inst_it, const InstructionMeta& meta) {
    const Type& bool_type = type_manager_.GetTypeBool();
    const uint32_t vector_size = meta.result_type->VectorSize();

    uint32_t bool_compare_type_id = 0;
    uint32_t null_type_id = 0;
    if (vector_size == 0) {
        bool_compare_type_id = bool_type.Id();
        null_type_id = type_manager_.GetConstantNull(*meta.result_type).Id();
    } else {
        bool_compare_type_id = type_manager_.GetTypeVector(bool_type, vector_size).Id();
        null_type_id = type_manager_.GetConstantZeroVector(*meta.result_type).Id();
    }

    const uint32_t x_value_id = meta.target_instruction->Word(5);
    const uint32_t y_value_id = meta.target_instruction->Word(6);

    const uint32_t compare_1_id = module_.TakeNextId();
    const uint32_t compare_2_id = module_.TakeNextId();
    const uint32_t compare_3_id = module_.TakeNextId();
    block.CreateInstruction(spv::OpFOrdLessThan, {bool_compare_type_id, compare_1_id, x_value_id, null_type_id}, inst_it);
    block.CreateInstruction(spv::OpFOrdEqual, {bool_compare_type_id, compare_2_id, x_value_id, null_type_id}, inst_it);
    block.CreateInstruction(spv::OpFOrdLessThanEqual, {bool_compare_type_id, compare_3_id, y_value_id, null_type_id}, inst_it);

    const uint32_t compare_and_id = module_.TakeNextId();
    const uint32_t compare_or_id = module_.TakeNextId();
    block.CreateInstruction(spv::OpLogicalAnd, {bool_compare_type_id, compare_and_id, compare_2_id, compare_3_id}, inst_it);
    block.CreateInstruction(spv::OpLogicalOr, {bool_compare_type_id, compare_or_id, compare_1_id, compare_and_id}, inst_it);

    uint32_t result_bool_id = 0;
    if (vector_size == 0) {
        result_bool_id = compare_or_id;
    } else {
        result_bool_id = module_.TakeNextId();
        block.CreateInstruction(spv::OpAny, {bool_type.Id(), result_bool_id, compare_or_id}, inst_it);
    }

    return result_bool_id;
}

// Returns an ID of type OpTypeBool
uint32_t SanitizerPass::Atan2Check(BasicBlock& block, InstructionIt* inst_it, const InstructionMeta& meta) {
    const Type& bool_type = type_manager_.GetTypeBool();
    const uint32_t vector_size = meta.result_type->VectorSize();

    uint32_t bool_compare_type_id = 0;
    uint32_t null_type_id = 0;
    if (vector_size == 0) {
        bool_compare_type_id = bool_type.Id();
        null_type_id = type_manager_.GetConstantNull(*meta.result_type).Id();
    } else {
        bool_compare_type_id = type_manager_.GetTypeVector(bool_type, vector_size).Id();
        null_type_id = type_manager_.GetConstantZeroVector(*meta.result_type).Id();
    }

    // Seems Atan flips the x/y order from other functions, doesn't really matter, but noting here
    const uint32_t y_value_id = meta.target_instruction->Word(5);
    const uint32_t x_value_id = meta.target_instruction->Word(6);

    const uint32_t compare_y_id = module_.TakeNextId();
    const uint32_t compare_x_id = module_.TakeNextId();
    block.CreateInstruction(spv::OpFOrdEqual, {bool_compare_type_id, compare_y_id, y_value_id, null_type_id}, inst_it);
    block.CreateInstruction(spv::OpFOrdEqual, {bool_compare_type_id, compare_x_id, x_value_id, null_type_id}, inst_it);
    const uint32_t compare_and_id = module_.TakeNextId();
    block.CreateInstruction(spv::OpLogicalAnd, {bool_compare_type_id, compare_and_id, compare_y_id, compare_x_id}, inst_it);

    uint32_t result_bool_id = 0;
    if (vector_size == 0) {
        result_bool_id = compare_and_id;
    } else {
        result_bool_id = module_.TakeNextId();
        block.CreateInstruction(spv::OpAny, {bool_type.Id(), result_bool_id, compare_and_id}, inst_it);
    }

    return result_bool_id;
}

// Returns an ID of type OpTypeBool for X and Y
BoolResultXY SanitizerPass::FminmaxCheck(BasicBlock& block, InstructionIt* inst_it, const InstructionMeta& meta) {
    BoolResultXY result_bool_id;
    const Type& bool_type = type_manager_.GetTypeBool();
    const uint32_t vector_size = meta.result_type->VectorSize();

    const uint32_t bool_result_type_id =
        (vector_size == 0) ? bool_type.Id() : type_manager_.GetTypeVector(bool_type, vector_size).Id();

    const uint32_t x_value_id = meta.target_instruction->Word(5);
    const uint32_t y_value_id = meta.target_instruction->Word(6);

    const uint32_t nan_x_id = module_.TakeNextId();
    const uint32_t nan_y_id = module_.TakeNextId();
    block.CreateInstruction(spv::OpIsNan, {bool_result_type_id, nan_x_id, x_value_id}, inst_it);
    block.CreateInstruction(spv::OpIsNan, {bool_result_type_id, nan_y_id, y_value_id}, inst_it);

    if (vector_size == 0) {
        result_bool_id.x = nan_x_id;
        result_bool_id.y = nan_y_id;
    } else {
        result_bool_id.x = module_.TakeNextId();
        result_bool_id.y = module_.TakeNextId();
        block.CreateInstruction(spv::OpAny, {bool_type.Id(), result_bool_id.x, nan_x_id}, inst_it);
        block.CreateInstruction(spv::OpAny, {bool_type.Id(), result_bool_id.y, nan_y_id}, inst_it);
    }

    return result_bool_id;
}

uint32_t SanitizerPass::CreateFunctionCall(BasicBlock& block, InstructionIt* inst_it, const InstructionMeta& meta) {
    const uint32_t function_result = module_.TakeNextId();
    const uint32_t function_def = GetLinkFunctionId(meta.sub_code);

    const uint32_t inst_position = meta.target_instruction->GetPositionOffset();
    const uint32_t inst_position_id = type_manager_.CreateConstantUInt32(inst_position).Id();

    if (meta.sub_code == glsl::kErrorSubCode_Sanitizer_DivideZero) {
        const uint32_t is_invalid_id = DivideByZeroCheck(block, inst_it, meta);

        const uint32_t bool_type = type_manager_.GetTypeBool().Id();
        const uint32_t opcode_id = type_manager_.CreateConstantUInt32(meta.target_instruction->Opcode()).Id();
        const uint32_t vector_size_id = type_manager_.CreateConstantUInt32(meta.result_type->VectorSize()).Id();

        block.CreateInstruction(
            spv::OpFunctionCall,
            {bool_type, function_result, function_def, is_invalid_id, inst_position_id, opcode_id, vector_size_id}, inst_it);
    } else if (meta.sub_code == glsl::kErrorSubCode_Sanitizer_ImageGather) {
        const uint32_t void_type = type_manager_.GetTypeVoid().Id();
        // If the OpConstant was a signed int, this will "cast" it by making a new OpConstant with the same value
        const uint32_t component_value_id = type_manager_.CreateConstantUInt32(meta.constant_value).Id();
        // Because we know this is 100% is an error if executed, just replace with a safe value
        const uint32_t safe_value_id = type_manager_.GetConstantZeroUint32().Id();
        const_cast<Instruction*>(meta.target_instruction)->UpdateWord(5, safe_value_id);
        block.CreateInstruction(spv::OpFunctionCall,
                                {void_type, function_result, function_def, inst_position_id, component_value_id}, inst_it);
    } else if (meta.sub_code == glsl::kErrorSubCode_Sanitizer_Pow) {
        const uint32_t is_invalid_id = PowCheck(block, inst_it, meta);

        const uint32_t bool_type = type_manager_.GetTypeBool().Id();
        const uint32_t vector_size = meta.result_type->VectorSize();
        const uint32_t vector_size_id = type_manager_.CreateConstantUInt32(vector_size).Id();

        uint32_t x_value_id = 0;
        uint32_t y_value_id = 0;
        if (vector_size == 0) {
            // cast as uint as that is how we are encoding the payload currently
            const Type& uint32_type = type_manager_.GetTypeInt(32, false);
            const uint32_t x_value_float = meta.target_instruction->Word(5);
            const uint32_t y_value_float = meta.target_instruction->Word(6);
            x_value_id = module_.TakeNextId();
            y_value_id = module_.TakeNextId();
            block.CreateInstruction(spv::OpBitcast, {uint32_type.Id(), x_value_id, x_value_float}, inst_it);
            block.CreateInstruction(spv::OpBitcast, {uint32_type.Id(), y_value_id, y_value_float}, inst_it);
        } else {
            // Put something valid, these are ignored on when printing error
            x_value_id = type_manager_.GetConstantZeroUint32().Id();
            y_value_id = type_manager_.GetConstantZeroUint32().Id();
        }

        block.CreateInstruction(
            spv::OpFunctionCall,
            {bool_type, function_result, function_def, is_invalid_id, inst_position_id, vector_size_id, x_value_id, y_value_id},
            inst_it);
    } else if (meta.sub_code == glsl::kErrorSubCode_Sanitizer_Atan2) {
        const uint32_t is_invalid_id = Atan2Check(block, inst_it, meta);
        const uint32_t bool_type = type_manager_.GetTypeBool().Id();
        block.CreateInstruction(spv::OpFunctionCall, {bool_type, function_result, function_def, is_invalid_id, inst_position_id},
                                inst_it);
    } else if (meta.sub_code == glsl::kErrorSubCode_Sanitizer_Fminmax) {
        const BoolResultXY is_invalid_id = FminmaxCheck(block, inst_it, meta);
        const uint32_t bool_type = type_manager_.GetTypeBool().Id();
        const uint32_t vector_size = meta.result_type->VectorSize();
        const uint32_t vector_size_id = type_manager_.CreateConstantUInt32(vector_size).Id();
        const uint32_t glsl_opcode_id = type_manager_.CreateConstantUInt32(meta.glsl_opcode).Id();
        block.CreateInstruction(spv::OpFunctionCall,
                                {bool_type, function_result, function_def, is_invalid_id.x, is_invalid_id.y, inst_position_id,
                                 vector_size_id, glsl_opcode_id},
                                inst_it);
    } else {
        assert(false);
    }

    module_.need_log_error_ = true;
    return function_result;
}

bool SanitizerPass::IsConstantZero(const Constant& constant) const {
    if (constant.is_spec_constant_) {
        // TODO - We have the spec constants information, we just need to pipe it into the passes
        return false;
    }
    const spv::Op opcode = (spv::Op)constant.inst_.Opcode();
    if (opcode == spv::OpConstantNull || opcode == spv::OpConstantFalse) {
        return true;
    } else if (opcode == spv::OpConstant) {
        // This works for signed ints and floats because zero is same for all
        if (constant.type_.Is64Bit()) {
            return constant.inst_.Word(3) == 0 && constant.inst_.Word(4) == 0;
        } else {
            return constant.inst_.Word(3) == 0;
        }
    } else if (opcode == spv::OpConstantComposite) {
        const size_t constituent_count = constant.inst_.Length() - 3;
        for (uint32_t i = 0; i < constituent_count; i++) {
            const Constant* component_constant = type_manager_.FindConstantById(constant.inst_.Operand(i));
            if (component_constant && IsConstantZero(*component_constant)) {
                return true;
            }
        }
    }
    return false;
}

bool SanitizerPass::RequiresInstrumentation(const Instruction& inst, InstructionMeta& meta) {
    const spv::Op opcode = (spv::Op)inst.Opcode();
    meta.target_instruction = &inst;

    if (IsValueIn(opcode, {spv::OpUDiv, spv::OpSDiv, spv::OpUMod, spv::OpSMod, spv::OpSRem, spv::OpFMod, spv::OpFRem})) {
        // Note - It is valid to divide by zero for a float (you get NaN), but invalid for an int.
        if (const Constant* constant = type_manager_.FindConstantById(inst.Word(4))) {
            // If its a constant, no reason to instrument, unless its a constant value of zero,
            // then it is only invalid if executed on the GPU.
            // (Tried to add to spirv-val, decided it should be a warning at most in VVL)
            if (!IsConstantZero(*constant)) {
                return false;
            }
        }

        // FMod/FRem are exceptions for float, they have undefined value if zero
        if (opcode == spv::OpFMod || opcode == spv::OpFRem) {
            meta.skip_safe_mode = true;
        }

        meta.result_type = type_manager_.FindTypeById(inst.TypeId());
        meta.sub_code = glsl::kErrorSubCode_Sanitizer_DivideZero;
        return true;
    } else if (opcode == spv::OpImageGather) {
        // 04664 requires this to be a constant
        if (const Constant* constant = type_manager_.FindConstantById(inst.Word(5))) {
            const uint32_t constant_value = constant->GetValueUint32();
            // TODO - Support spec constants
            if (!constant->is_spec_constant_ && constant_value > 3) {
                meta.sub_code = glsl::kErrorSubCode_Sanitizer_ImageGather;
                meta.skip_safe_mode = true;
                meta.constant_value = constant_value;
                return true;
            }
        }
    } else if (opcode == spv::OpExtInst && inst.Word(3) == glsl_std450_id_) {
        uint32_t glsl_opcode = inst.Word(4);
        if (glsl_opcode == GLSLstd450Pow) {
            meta.sub_code = glsl::kErrorSubCode_Sanitizer_Pow;
        } else if (glsl_opcode == GLSLstd450Atan2) {
            meta.sub_code = glsl::kErrorSubCode_Sanitizer_Atan2;
        } else if (glsl_opcode == GLSLstd450FMin || glsl_opcode == GLSLstd450FMax) {
            meta.sub_code = glsl::kErrorSubCode_Sanitizer_Fminmax;
            meta.glsl_opcode = glsl_opcode;
        } else {
            return false;
        }

        // all of these only have results that are undefined
        meta.skip_safe_mode = true;
        meta.result_type = type_manager_.FindTypeById(inst.TypeId());
        return true;
    }

    return false;
}

bool SanitizerPass::Instrument() {
    for (const auto& inst : module_.ext_inst_imports_) {
        const char* import_string = inst->GetAsString(2);
        if (strcmp(import_string, "GLSL.std.450") == 0) {
            glsl_std450_id_ = inst->ResultId();
            break;
        }
    }

    // Can safely loop function list as there is no injecting of new Functions until linking time
    for (const auto& function : module_.functions_) {
        if (function->instrumentation_added_) {
            continue;
        }
        for (auto block_it = function->blocks_.begin(); block_it != function->blocks_.end(); ++block_it) {
            BasicBlock& current_block = **block_it;

            cf_.Update(current_block);
            if (debug_disable_loops_ && cf_.in_loop) {
                continue;
            }

            if (current_block.IsLoopHeader()) {
                continue;  // Currently can't properly handle injecting CFG logic into a loop header block
            }
            auto& block_instructions = current_block.instructions_;

            for (auto inst_it = block_instructions.begin(); inst_it != block_instructions.end(); ++inst_it) {
                InstructionMeta meta;
                // Every instruction is analyzed by the specific pass and lets us know if we need to inject a function or not
                if (!RequiresInstrumentation(*(inst_it->get()), meta)) {
                    continue;
                }

                if (IsMaxInstrumentationsCount()) {
                    continue;
                }
                instrumentations_count_++;

                if (!module_.settings_.safe_mode || meta.skip_safe_mode) {
                    CreateFunctionCall(current_block, &inst_it, meta);
                } else {
                    InjectConditionalData ic_data = InjectFunctionPre(*function.get(), block_it, inst_it);
                    ic_data.function_result_id = CreateFunctionCall(current_block, nullptr, meta);
                    InjectFunctionPost(current_block, ic_data);
                    // Skip the newly added valid and invalid block. Start searching again from newly split merge block
                    block_it++;
                    block_it++;
                    break;
                }
            }
        }
    }

    return instrumentations_count_ != 0;
}

void SanitizerPass::PrintDebugInfo() const {
    std::cout << "SanitizerPass instrumentation count: " << instrumentations_count_ << '\n';
}

}  // namespace spirv
}  // namespace gpuav