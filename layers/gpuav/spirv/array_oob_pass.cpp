/* Copyright (c) 2026 LunarG, Inc.
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

#include "array_oob_pass.h"
#include "containers/container_utils.h"
#include "module.h"
#include <cassert>
#include <iostream>
#include <spirv/unified1/spirv.hpp>
#include <vector>

#include "generated/gpuav_offline_spirv.h"
#include "type_manager.h"

namespace gpuav {
namespace spirv {

const static OfflineModule kOfflineModule = {instrumentation_array_oob_comp, instrumentation_array_oob_comp_size,
                                             UseErrorPayloadVariable};

const static OfflineFunction kOfflineFunction = {"inst_array_oob", instrumentation_array_oob_comp_function_0_offset};

static bool IsTrackedStorageClass(spv::StorageClass sc) {
    return sc == spv::StorageClassWorkgroup || sc == spv::StorageClassPrivate || sc == spv::StorageClassFunction;
}

bool ArrayOobPass::IsTrackedPointerStorageClass(const Instruction* inst) const {
    if (!inst) return false;
    spv::StorageClass sc = inst->StorageClass();  // OpVariable
    if (sc == spv::StorageClassMax) {
        const Type* ptr_type = type_manager_.FindTypeById(inst->TypeId());
        if (ptr_type && ptr_type->spv_type_ == SpvType::kPointer) {
            sc = ptr_type->inst_.StorageClass();
        }
    }
    return IsTrackedStorageClass(sc);
}

ArrayOobPass::ArrayOobPass(Module& module) : Pass(module, kOfflineModule) {}

void ArrayOobPass::EmitBoundsChecks(BasicBlock& block, InstructionIt* inst_it, Instruction& target_inst,
                                    const InstructionMeta& meta) {
    const uint32_t function_def = GetLinkFunction(link_function_id_, kOfflineFunction);
    const uint32_t bool_type = type_manager_.GetTypeBool().Id();
    const uint32_t inst_position_id = type_manager_.GetConstantUInt32(meta.inst_position_offset).Id();
    const uint32_t variable_id_const = type_manager_.GetConstantUInt32(meta.variable_id).Id();

    // Operand ids still on the access chain / vector op (before any UpdateWord). `check.index_id` may be a CastToUint32
    // rewriting for inst_array_oob; OpSelect must use the original integer type the consumer instruction expects.
    std::vector<uint32_t> orig_index_ids;
    orig_index_ids.reserve(meta.checks.size());
    for (const auto& check : meta.checks) {
        orig_index_ids.push_back(target_inst.Word(check.index_spirv_word));
    }

    for (size_t ci = 0; ci < meta.checks.size(); ++ci) {
        const auto& check = meta.checks[ci];
        const uint32_t orig_index_id = orig_index_ids[ci];
        const uint32_t encoded_bound = check.bound | (check.access_type << 24) | (check.dim_index << 26);
        const uint32_t bound_id = type_manager_.GetConstantUInt32(encoded_bound).Id();
        const uint32_t ok_id = module_.TakeNextId();
        block.CreateInstruction(spv::OpFunctionCall,
                                {bool_type, ok_id, function_def, check.index_id, bound_id, inst_position_id, variable_id_const},
                                inst_it);
        module_.need_log_error_ = true;

        if (module_.settings_.safe_mode) {
            const Instruction* orig_def = block.function_->FindInstruction(orig_index_id);
            const Constant* orig_const = type_manager_.FindConstantById(orig_index_id);
            const Type* select_type = nullptr;
            if (orig_def) {
                select_type = type_manager_.FindTypeById(orig_def->TypeId());
            } else if (orig_const) {
                select_type = &orig_const->type_;
            }
            assert(select_type);
            const uint32_t select_type_id = select_type->Id();
            const uint32_t zero_id = type_manager_.CreateConstantScalar(0, *select_type).Id();
            // inst_array_oob returns true iff index is in bounds. Force index to 0 when OOB (no CFG split).
            const uint32_t safe_id = module_.TakeNextId();
            block.CreateInstruction(spv::OpSelect, {select_type_id, safe_id, ok_id, orig_index_id, zero_id}, inst_it);
            target_inst.UpdateWord(check.index_spirv_word, safe_id);
        }
    }
}

bool ArrayOobPass::RequiresInstrumentation(const Function& function, BasicBlock& block, InstructionIt& inst_it,
                                           const Instruction& inst, InstructionMeta& meta) {
    const spv::Op opcode = (spv::Op)inst.Opcode();

    if (opcode == spv::OpVectorExtractDynamic || opcode == spv::OpVectorInsertDynamic) {
        const uint32_t vector_id = inst.Operand(0);
        const Instruction* vector_inst = function.FindInstruction(vector_id);
        // OpVectorExtract/InsertDynamic operate on values, not pointers, so the vector operand
        // will be an OpLoad from the variable. We trace through the OpLoad to find the source
        // pointer. Even though the OOB occurs on the loaded value, we still report it because
        // the vector's bounds are determined by the variable's declaration and the index is
        // checked before the access.
        if (!vector_inst || vector_inst->Opcode() != spv::OpLoad) {
            return false;
        }

        const uint32_t ptr_id = vector_inst->Operand(0);

        const Variable* variable = type_manager_.FindVariableById(ptr_id);
        const Instruction* base_inst = function.FindInstruction(ptr_id);
        while (base_inst && base_inst->IsNonPtrAccessChain()) {
            const uint32_t base_id = base_inst->Operand(0);
            variable = type_manager_.FindVariableById(base_id);
            if (variable) {
                break;
            }
            base_inst = function.FindInstruction(base_id);
        }
        const Instruction* leaf_inst = variable ? &variable->inst_ : base_inst;
        if (!leaf_inst || !IsTrackedPointerStorageClass(leaf_inst)) {
            return false;
        }

        const Type* vector_type = type_manager_.FindTypeById(vector_inst->TypeId());
        if (!vector_type || !vector_type->inst_.IsVector()) {
            return false;
        }

        uint32_t index_id = (opcode == spv::OpVectorExtractDynamic) ? inst.Operand(1) : inst.Operand(2);
        index_id = CastToUint32(index_id, block, &inst_it);

        meta.inst_position_offset = inst.GetPositionOffset();
        meta.variable_id = leaf_inst->ResultId();
        // OpVectorExtractDynamic: Index is word 4. OpVectorInsertDynamic: Index is word 5 (Component is word 4).
        const uint32_t index_word = (opcode == spv::OpVectorExtractDynamic) ? 4u : 5u;
        meta.checks.push_back({index_id, vector_type->meta_.vector.component_count, 1, 0, index_word});
        return true;
    } else if (!inst.IsNonPtrAccessChain()) {
        return false;
    }

    const uint32_t base_id = inst.Operand(0);
    const Variable* variable = type_manager_.FindVariableById(base_id);
    const Instruction* chain_inst = function.FindInstruction(base_id);
    while (chain_inst && chain_inst->IsNonPtrAccessChain()) {
        const uint32_t chain_base_id = chain_inst->Operand(0);
        variable = type_manager_.FindVariableById(chain_base_id);
        if (variable) {
            break;
        }
        chain_inst = function.FindInstruction(chain_base_id);
    }
    const Instruction* leaf_inst = variable ? &variable->inst_ : chain_inst;
    if (!leaf_inst || !IsTrackedPointerStorageClass(leaf_inst)) {
        return false;
    }
    meta.variable_id = leaf_inst->ResultId();
    meta.inst_position_offset = inst.GetPositionOffset();

    const Instruction* base_inst = function.FindInstruction(base_id);
    const Type* base_ptr_type = base_inst ? type_manager_.FindTypeById(base_inst->TypeId()) : nullptr;
    if (!base_ptr_type) {
        assert(leaf_inst->ResultId() == base_id);
        base_ptr_type = type_manager_.FindTypeById(leaf_inst->TypeId());
    }
    assert(base_ptr_type);
    const Type* pointee_type = type_manager_.FindChildType(*base_ptr_type, 0);

    uint32_t dim_index = 0;
    for (uint32_t i = 4; i < inst.Length(); ++i) {
        uint32_t idx_id = inst.Word(i);

        switch (pointee_type->spv_type_) {
            case SpvType::kArray:
            case SpvType::kVector:
            case SpvType::kVectorIdEXT: {
                idx_id = CastToUint32(idx_id, block, &inst_it);
                const bool is_array = (pointee_type->spv_type_ == SpvType::kArray);
                const uint32_t bound = is_array ? pointee_type->meta_.array.length : pointee_type->meta_.vector.component_count;
                const uint32_t access_type = is_array ? 0 : 1;
                meta.checks.push_back({idx_id, bound, access_type, dim_index, i});
                dim_index++;
                pointee_type = type_manager_.FindChildType(*pointee_type, 0);
            } break;
            case SpvType::kStruct: {
                auto idx_c = type_manager_.FindConstantById(idx_id);
                uint32_t member_idx = idx_c->GetValueUint32();
                pointee_type = type_manager_.FindChildType(*pointee_type, member_idx);
            } break;
            case SpvType::kMatrix: {
                // TODO check matrix column indices
                pointee_type = type_manager_.FindChildType(*pointee_type, 0);
            } break;
            default:
                pointee_type = type_manager_.FindChildType(*pointee_type, 0);
                break;
        }
    }

    return !meta.checks.empty();
}

bool ArrayOobPass::Instrument() {
    for (Function& function : module_.functions_) {
        if (!function.called_from_target_) {
            continue;
        }

        for (auto block_it = function.blocks_.begin(); block_it != function.blocks_.end(); ++block_it) {
            BasicBlock& current_block = **block_it;

            cf_.Update(current_block);
            if (debug_disable_loops_ && cf_.in_loop) {
                continue;
            }

            if (current_block.IsLoopHeader()) {
                continue;
            }

            auto& block_instructions = current_block.instructions_;
            for (auto inst_it = block_instructions.begin(); inst_it != block_instructions.end(); ++inst_it) {
                Instruction& inst = *inst_it->get();

                InstructionMeta meta;
                if (!RequiresInstrumentation(function, current_block, inst_it, inst, meta)) {
                    continue;
                }

                if (MaxInstrumentationsCountReached()) {
                    continue;
                }
                instrumentations_count_++;

                EmitBoundsChecks(current_block, &inst_it, inst, meta);
            }
        }
    }

    return instrumentations_count_ != 0;
}

void ArrayOobPass::PrintDebugInfo() const {
    std::cout << "ArrayOobPass instrumentation count: " << instrumentations_count_ << '\n';
}

}  // namespace spirv
}  // namespace gpuav
