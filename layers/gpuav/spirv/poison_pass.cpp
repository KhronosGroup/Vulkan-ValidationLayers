/* Copyright (c) 2026 LunarG, Inc.
 * Copyright (c) 2026 The Khronos Group Inc.
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

// =============================================================================
// Poison Pass - Poison Value Propagation and UB Detection
// =============================================================================
//
// This pass instruments SPIR-V shaders to model the propagation of "poison"
// values and detect undefined behavior caused by their use. In SPIR-V, a
// poison value is a value that the spec says has no defined meaning - using
// it in certain ways is UB.
//
// The pass is a general poison-tracking framework. Currently, the only seed
// source is uninitialized variables (Function/Private storage class without
// an initializer), but the architecture supports adding other poison sources
// in the future (see "Future Poison Sources" below).
//
// OVERVIEW
// --------
// The pass processes each function independently in four phases:
//
//   1. Seed identification          - find initial poison sources
//   2. ComputePotentiallyPoisonSet  - static dataflow analysis
//   3. CreateShadowVariables        - allocate runtime shadow state
//   4. InstrumentFunction           - insert shadow tracking and error checks
//
// Each function's analysis is self-contained. Cross-function poison flow is
// handled conservatively at call boundaries (see "Function Calls" below).
//
// SHADOW VARIABLES
// ----------------
// For each variable that may contain poison, a parallel "shadow" variable is
// created. The shadow has the same composite structure as the original but
// with all leaf types replaced by bool:
//
//   uint        -> bool
//   vec4        -> bvec4
//   float[3]    -> bool[3]
//   struct{uint a; vec2 b;} -> struct{bool a; bvec2 b;}
//   mat3        -> bvec3[3]  (array of column shadows)
//
// A shadow value of `true` means "clean" and `false` means "poison". Shadow
// variables for uninitialized sources are initialized to OpConstantNull (all
// false / all poison). When a store writes to a tracked variable, the
// corresponding shadow is updated; when a load reads from it, the shadow
// is loaded and propagated through dependent instructions.
//
// INSTRUCTION CATEGORIES
// ----------------------
// Instructions are classified into three categories based on how they
// interact with poison:
//
//   Pass-through (data movement):
//     OpLoad, OpStore (to local), OpSelect, OpPhi, OpCopyObject, OpCopyLogical,
//     OpCompositeExtract, OpCompositeInsert, OpCompositeConstruct,
//     OpCompositeConstructReplicateEXT,
//     OpVectorShuffle, OpVectorExtractDynamic, OpVectorInsertDynamic.
//     Shadow follows the data naturally, preserving per-component tracking.
//
//   Propagation (computation):
//     Arithmetic, bitwise, comparison, conversion, dot product, etc.
//     If any input operand is poison, the output is poison (logical AND
//     of all operand shadows). When operand and result types differ
//     (e.g. dot(vec4, vec4)->float), each operand shadow is reduced to
//     scalar bool and broadcast to match the result shadow type.
//     No error is reported - poison silently propagates.
//
//   UB triggers (error reporting):
//     These instructions cause undefined behavior if given poison input:
//       - OpBranchConditional / OpSwitch with poison condition/selector
//       - OpAccessChain / OpPtrAccessChain with poison index
//     An error is reported and the poison condition is replaced with a safe
//     constant to prevent the compiler from optimizing away the error
//     reporting code.
//
//   Likely-bug warnings (error reporting, not spec UB):
//     These are not undefined behavior per the SPIR-V spec, but are almost
//     certainly application bugs:
//       - OpStore to non-local (externally-visible) storage
//       - OpStore to a function parameter (out/inout)
//       - OpReturnValue of a poison value
//
// STATIC ANALYSIS (ComputePotentiallyPoisonSet)
// ---------------------------------------------
// To avoid instrumenting every instruction, a worklist-based dataflow analysis
// determines which SSA values *could possibly* be poison. Only these values
// receive shadow tracking at runtime.
//
// The analysis seeds the worklist with the result IDs of all OpLoad
// instructions that read from poison-source variables, then propagates any
// instruction whose operand is a known-poison value produces a
// potentially-poison result. Special cases:
//
//   - OpStore of poison to a local variable "contaminates" that variable,
//     adding it to the tracked set and re-seeding loads from it.
//   - OpFunctionCall arguments are checked both as values (for pass-by-value)
//     and as pointers (for pass-by-reference).
//
// Values NOT in the potentially-poison set are assumed clean and receive no
// shadow instrumentation. This is an optimization - the shadow variable
// mechanism would produce the same results if applied universally.
//
// FUNCTION CALLS
// --------------
// Cross-function poison flow is handled conservatively without rewriting
// function signatures:
//
// Caller side (OpFunctionCall):
//   - The call result's shadow is the logical AND of all argument shadows,
//     each reduced to scalar bool. If any argument is poison, the result
//     is treated as poison. This could lead to false positives - the callee
//     might not actually use the poison argument in computing its return
//     value.
//   - After the call, shadow variables for any pointer arguments (which
//     correspond to out/inout parameters) are set to "clean" (all true).
//     The callee may have written to these variables, so subsequent loads
//     in the caller should not see stale poison state.
//
// Callee side:
//   - Function parameters (OpFunctionParameter) are NOT treated as poison
//     sources. Only the callee's own local variables are tracked.
//   - OpReturnValue of a poison value is a UB trigger.
//   - OpStore to a function parameter pointer (out/inout) of a poison
//     value is a UB trigger with a dedicated error sub-code.
//
// KNOWN LIMITATIONS
// -----------------
//   - Per-function analysis: the pass cannot track poison across function
//     boundaries precisely. The conservative caller-side approach may
//     produce false positives (treating clean call results as poison when
//     only an unused argument was poison).
//   - Limited domination analysis: a direct store to the whole variable in
//     the entry block before any access is treated as an initializer, and
//     function call pointer arguments are assumed to initialize the variable.
//     Beyond these cases, stores before loads are not recognized as
//     initializers (e.g. stores in non-entry blocks, partial stores via
//     access chains).
//   - Cooperative matrix instructions are skipped (opaque types with
//     implementation-dependent component counts).
//
// CURRENT POISON SOURCES
// ----------------------
//   - Uninitialized variables: OpVariable in Function or Private storage
//     class without an initializer operand.
//
// FUTURE POISON SOURCES (not yet seeded by this pass)
// ---------------------------------------------------
//   The following are additional sources of poison defined by the SPIR-V
//   specification that this pass does not yet seed. If seeded, the existing
//   dataflow analysis and instrumentation would handle them automatically.
//   - OpFRem / OpFMod with a 0 divisor -> result is poison.
//   - Shift instructions (OpShiftLeftLogical, etc.) with shift amount >=
//     bit width -> result is poison.
//   - OpBitFieldInsert / OpBitFieldSExtract / OpBitFieldUExtract with
//     Count or Offset out of the valid range -> result is poison.
//   - OpEmitVertex / OpEmitStreamVertex: output variables become poison
//     after emission (geometry shaders).
//   - Fast math flags (NotNaN, NotInf): if the assumption is violated,
//     the result is poison.
//   - OpDemoteToHelperInvocation: subsequent loads of HelperInvocation
//     built-in may return poison.
//   - OpGroupNonUniformBallotFindLSB / OpGroupNonUniformBallotFindMSB
//     with no bits set -> result is poison.
//

#include "poison_pass.h"
#include "module.h"
#include "type_manager.h"
#include "gpuav/shaders/gpuav_error_codes.h"
#include <spirv/unified1/spirv.hpp>
#include <cassert>
#include <iostream>

#include "generated/gpuav_offline_spirv.h"

namespace gpuav {
namespace spirv {

const static OfflineModule kOfflineModule = {instrumentation_poison_value_comp, instrumentation_poison_value_comp_size,
                                             UseErrorPayloadVariable};
const static OfflineFunction kOfflineFunction = {"inst_poison_report", instrumentation_poison_value_comp_function_0_offset};

PoisonPass::PoisonPass(Module& module) : Pass(module, kOfflineModule) {}

void PoisonPass::PrintDebugInfo() const { std::cout << "PoisonPass instrumentation count: " << instrumentations_count_ << '\n'; }

// =============================================================================
// Phase 1: Static Analysis
// =============================================================================

void PoisonPass::FindUninitializedVariables(Function& function) {
    uninitialized_var_ids_.clear();
    all_local_var_ids_.clear();
    contaminated_var_ids_.clear();

    auto check_variable = [&](const Instruction& inst) {
        auto sc = static_cast<spv::StorageClass>(inst.Word(3));
        if (sc == spv::StorageClassFunction || sc == spv::StorageClassPrivate) {
            all_local_var_ids_.insert(inst.ResultId());
            if (inst.Length() <= 4) {
                uninitialized_var_ids_.insert(inst.ResultId());
            }
        }
    };

    for (auto& block : function.blocks_) {
        for (auto& inst : block->instructions_) {
            if (inst->Opcode() == spv::OpVariable) {
                check_variable(*inst);
            }
        }
    }

    // Module-level Private variables
    for (const auto& inst : module_.types_values_constants_) {
        if (inst->Opcode() == spv::OpVariable) {
            check_variable(*inst);
        }
    }

    // A direct store to the whole variable in the entry block before any other
    // access effectively initializes it. This is a common pattern when compilers
    // emit OpVariable without an initializer followed by an immediate OpStore.
    if (!uninitialized_var_ids_.empty() && !function.blocks_.empty()) {
        BasicBlock& entry_block = *function.blocks_.front();
        std::unordered_set<uint32_t> accessed_before_store;

        for (auto& inst : entry_block.instructions_) {
            if (inst->Opcode() == spv::OpStore) {
                uint32_t ptr_id = inst->Operand(0);
                if (uninitialized_var_ids_.count(ptr_id) && !accessed_before_store.count(ptr_id)) {
                    uninitialized_var_ids_.erase(ptr_id);
                    continue;
                }
            }

            inst->ForEachIdOperand([&](uint32_t id) {
                if (uninitialized_var_ids_.count(id)) {
                    accessed_before_store.insert(id);
                }
            });
        }
    }
}

void PoisonPass::ComputePotentiallyPoisonSet(Function& function) {
    potentially_poison_ids_.clear();

    // Forward pass: build pointer-to-variable map and collect loads per variable.
    // Built from all_local_var_ids_ so contamination detection can resolve pointers
    // to any local variable, not just currently tracked ones.
    std::unordered_set<uint32_t> tracked_vars = uninitialized_var_ids_;
    std::unordered_map<uint32_t, uint32_t> ptr_to_var;                 // pointer id -> base variable id
    std::unordered_map<uint32_t, std::vector<uint32_t>> var_to_loads;  // variable id -> load result ids

    auto resolve_var = [&](uint32_t ptr_id) -> uint32_t {
        if (all_local_var_ids_.count(ptr_id)) return ptr_id;
        auto it = ptr_to_var.find(ptr_id);
        return it != ptr_to_var.end() ? it->second : 0;
    };

    for (auto& block : function.blocks_) {
        for (auto& inst : block->instructions_) {
            const auto op = static_cast<spv::Op>(inst->Opcode());
            if (op == spv::OpAccessChain || op == spv::OpInBoundsAccessChain || op == spv::OpPtrAccessChain ||
                op == spv::OpCopyObject) {
                uint32_t base_id = inst->Operand(0);
                if (all_local_var_ids_.count(base_id)) {
                    ptr_to_var[inst->ResultId()] = base_id;
                } else {
                    auto it = ptr_to_var.find(base_id);
                    if (it != ptr_to_var.end()) {
                        ptr_to_var[inst->ResultId()] = it->second;
                    }
                }
            } else if (op == spv::OpLoad) {
                uint32_t var_id = resolve_var(inst->Operand(0));
                if (var_id != 0) {
                    var_to_loads[var_id].push_back(inst->ResultId());
                }
            } else {
                // Many instructions can consume a tracked pointer without deriving a new
                // pointer (OpStore, OpFunctionCall, OpExtInst for debug info, ray query ops,
                // atomics, etc.). We only care about pointer-deriving instructions, which
                // are exhaustively handled in the if-chain above (OpAccessChain,
                // OpInBoundsAccessChain, OpPtrAccessChain, OpCopyObject).
            }
        }
    }

    auto seed_loads = [&](uint32_t var_id, std::vector<uint32_t>& worklist) {
        auto it = var_to_loads.find(var_id);
        if (it == var_to_loads.end()) return;
        for (uint32_t load_id : it->second) {
            if (potentially_poison_ids_.insert(load_id).second) {
                worklist.push_back(load_id);
            }
        }
    };

    std::vector<uint32_t> worklist;
    for (uint32_t var_id : uninitialized_var_ids_) {
        seed_loads(var_id, worklist);
    }

    // Forward propagation of poison through value-producing instructions
    while (!worklist.empty()) {
        uint32_t poison_id = worklist.back();
        worklist.pop_back();

        for (auto& block : function.blocks_) {
            for (auto& inst : block->instructions_) {
                const auto op = static_cast<spv::Op>(inst->Opcode());

                // Poison stored into a local variable contaminates it
                if (op == spv::OpStore) {
                    uint32_t value_id = inst->Operand(1);
                    if (value_id == poison_id) {
                        uint32_t var_id = resolve_var(inst->Operand(0));
                        if (var_id != 0 && !tracked_vars.count(var_id)) {
                            tracked_vars.insert(var_id);
                            contaminated_var_ids_.insert(var_id);
                            seed_loads(var_id, worklist);
                        }
                    }
                    continue;
                }

                uint32_t result = inst->ResultId();
                if (result == 0 || potentially_poison_ids_.count(result)) continue;

                // Poison sources, not propagation targets
                if (op == spv::OpVariable) continue;
                // No result ID / not data flow
                if (op == spv::OpLabel) continue;
                if (op == spv::OpBranch || op == spv::OpBranchConditional || op == spv::OpSwitch) continue;
                if (op == spv::OpSelectionMerge || op == spv::OpLoopMerge) continue;
                if (op == spv::OpReturn || op == spv::OpReturnValue || op == spv::OpUnreachable) continue;
                // Opaque types with implementation-dependent component counts
                if (op == spv::OpCooperativeMatrixLoadKHR || op == spv::OpCooperativeMatrixStoreKHR ||
                    op == spv::OpCooperativeMatrixMulAddKHR || op == spv::OpCooperativeMatrixLengthKHR)
                    continue;

                // glslang passes function args by pointer, so check both value and pointer args
                if (op == spv::OpFunctionCall) {
                    for (uint32_t w = 4; w < inst->Length(); w++) {
                        uint32_t arg_id = inst->Word(w);
                        if (arg_id == poison_id) {
                            potentially_poison_ids_.insert(result);
                            worklist.push_back(result);
                            break;
                        }
                        uint32_t var_id = resolve_var(arg_id);
                        if (var_id != 0 && tracked_vars.count(var_id)) {
                            potentially_poison_ids_.insert(result);
                            worklist.push_back(result);
                            break;
                        }
                    }
                    continue;
                }

                bool uses_poison = false;
                inst->ForEachIdOperand([&](uint32_t id) {
                    if (id == poison_id) {
                        uses_poison = true;
                    }
                });
                if (uses_poison) {
                    if (potentially_poison_ids_.insert(result).second) {
                        worklist.push_back(result);
                    }
                }
            }
        }
    }

    // Expand uninitialized_var_ids_ to include contaminated variables so they get shadow variables
    uninitialized_var_ids_.insert(contaminated_var_ids_.begin(), contaminated_var_ids_.end());
}

// =============================================================================
// Phase 2: Shadow Type/Variable Creation
// =============================================================================

// Maps an original SPIR-V type to its shadow type (same composite structure, all leaves become bool).
// Results are cached in shadow_type_cache_.
uint32_t PoisonPass::GetOrCreateShadowType(uint32_t type_id) {
    auto it = shadow_type_cache_.find(type_id);
    if (it != shadow_type_cache_.end()) return it->second;

    const Type* type = type_manager_.FindTypeById(type_id);
    if (!type) {
        shadow_type_cache_[type_id] = type_manager_.GetTypeBool().Id();
        return type_manager_.GetTypeBool().Id();
    }

    uint32_t shadow_id = 0;
    switch (type->spv_type_) {
        case SpvType::kBool:
        case SpvType::kInt:
        case SpvType::kFloat:
            shadow_id = type_manager_.GetTypeBool().Id();
            break;
        case SpvType::kVector: {
            uint32_t count = type->meta_.vector.component_count;
            shadow_id = type_manager_.GetTypeVector(type_manager_.GetTypeBool(), count).Id();
            break;
        }
        case SpvType::kVectorIdEXT: {
            const Constant* count = type_manager_.FindConstantById(type->inst_.Word(3));
            if (count) {
                shadow_id = type_manager_.GetTypeVectorIdEXT(type_manager_.GetTypeBool(), *count).Id();
            } else {
                shadow_id = type_manager_.GetTypeBool().Id();
            }
            break;
        }
        case SpvType::kMatrix: {
            uint32_t col_type_id = type->inst_.Word(2);
            uint32_t col_count = type->inst_.Word(3);
            uint32_t shadow_col_id = GetOrCreateShadowType(col_type_id);
            const Type* shadow_col = type_manager_.FindTypeById(shadow_col_id);
            const Constant& col_count_const = type_manager_.GetConstantUInt32(col_count);
            if (shadow_col) {
                shadow_id = type_manager_.GetTypeArray(*shadow_col, col_count_const, false).Id();
            } else {
                shadow_id = type_manager_.GetTypeBool().Id();
            }
            break;
        }
        case SpvType::kArray: {
            uint32_t elem_type_id = type->inst_.Word(2);
            uint32_t shadow_elem_id = GetOrCreateShadowType(elem_type_id);
            const Type* shadow_elem = type_manager_.FindTypeById(shadow_elem_id);
            const Constant* length = type_manager_.FindConstantById(type->inst_.Word(3));
            if (shadow_elem && length) {
                shadow_id = type_manager_.GetTypeArray(*shadow_elem, *length, false).Id();
            } else {
                shadow_id = type_manager_.GetTypeBool().Id();
            }
            break;
        }
        case SpvType::kStruct: {
            std::vector<uint32_t> member_shadow_ids;
            for (uint32_t i = 2; i < type->inst_.Length(); i++) {
                member_shadow_ids.push_back(GetOrCreateShadowType(type->inst_.Word(i)));
            }
            const uint32_t struct_type_id = module_.TakeNextId();
            auto new_inst = std::make_unique<Instruction>(static_cast<uint32_t>(member_shadow_ids.size() + 2), spv::OpTypeStruct);
            std::vector<uint32_t> fill_words = {struct_type_id};
            fill_words.insert(fill_words.end(), member_shadow_ids.begin(), member_shadow_ids.end());
            new_inst->Fill(fill_words);
            shadow_id = type_manager_.AddType(std::move(new_inst), SpvType::kStruct).Id();
            break;
        }
        default:
            shadow_id = type_manager_.GetTypeBool().Id();
            break;
    }

    shadow_type_cache_[type_id] = shadow_id;
    return shadow_id;
}

const PoisonPass::ShadowPtrInfo* PoisonPass::FindShadowPointer(uint32_t ptr_id) const {
    auto it = shadow_pointer_map_.find(ptr_id);
    return it != shadow_pointer_map_.end() ? &it->second : nullptr;
}

// Forward-propagate shadow pointers through pointer-deriving instructions.
// Called for every instruction in the main loop before load/store handling.
// shadow_pointer_map_ is seeded from shadow_var_map_ at the start of each function,
// then this function propagates through:
//   OpAccessChain / OpInBoundsAccessChain / OpPtrAccessChain - mirror with shadow access chain
//   OpCopyObject - propagate shadow pointer directly
void PoisonPass::PropagateShadowPointer(BasicBlock& block, InstructionIt* inst_it, const Instruction& inst) {
    const auto opcode = static_cast<spv::Op>(inst.Opcode());
    const uint32_t result_id = inst.ResultId();

    if (opcode == spv::OpAccessChain || opcode == spv::OpInBoundsAccessChain || opcode == spv::OpPtrAccessChain) {
        uint32_t base_id = inst.Operand(0);
        const ShadowPtrInfo* base_shadow = FindShadowPointer(base_id);
        if (!base_shadow) return;

        // Walk the shadow type through the access chain indices.
        // OpPtrAccessChain has an Element operand (Word 4) that indexes at the pointer level
        // without descending into the type, followed by regular indices at Word 5+.
        // OpAccessChain/OpInBoundsAccessChain have regular indices starting at Word 4.
        const Type* current_type = type_manager_.FindTypeById(base_shadow->shadow_pointee_type_id);
        assert(current_type);

        const uint32_t first_type_index = (opcode == spv::OpPtrAccessChain) ? 5 : 4;
        const uint32_t num_type_indices = inst.Length() - first_type_index;
        for (uint32_t i = 0; i < num_type_indices; i++) {
            switch (current_type->spv_type_) {
                case SpvType::kArray:
                case SpvType::kRuntimeArray:
                case SpvType::kVector:
                case SpvType::kVectorIdEXT:
                case SpvType::kMatrix:
                    current_type = type_manager_.FindTypeById(current_type->inst_.Word(2));
                    break;
                case SpvType::kStruct: {
                    const Constant* idx_const = type_manager_.FindConstantById(inst.Word(first_type_index + i));
                    assert(idx_const);
                    uint32_t member_idx = idx_const->GetValueUint32();
                    assert(member_idx + 2 < current_type->inst_.Length());
                    current_type = type_manager_.FindTypeById(current_type->inst_.Word(2 + member_idx));
                    break;
                }
                default:
                    assert(false);
                    break;
            }
            assert(current_type);
        }

        const Type& ptr_type = type_manager_.GetTypePointer(base_shadow->shadow_sc, *current_type);
        uint32_t new_ptr = module_.TakeNextId();
        std::vector<uint32_t> words = {ptr_type.Id(), new_ptr, base_shadow->shadow_ptr_id};
        for (uint32_t i = 4; i < inst.Length(); i++) {
            words.push_back(inst.Word(i));
        }
        block.CreateInstruction(opcode, words, inst_it);
        shadow_pointer_map_[result_id] = {new_ptr, current_type->Id(), base_shadow->shadow_sc, base_shadow->var_id};
        return;
    }

    if (opcode == spv::OpCopyObject) {
        if (const ShadowPtrInfo* src_shadow = FindShadowPointer(inst.Operand(0))) {
            shadow_pointer_map_[result_id] = *src_shadow;
        }
        return;
    }
}

// Broadcast a scalar bool to match a composite shadow type using OpCompositeConstruct.
// For scalar bool input, just returns it if the target is already bool.
uint32_t PoisonPass::BroadcastShadow(BasicBlock& block, InstructionIt* inst_it, uint32_t scalar_shadow_id,
                                     uint32_t shadow_type_id) {
    const Type* shadow_type = type_manager_.FindTypeById(shadow_type_id);
    if (!shadow_type || shadow_type->spv_type_ == SpvType::kBool) {
        return scalar_shadow_id;
    }

    if (scalar_shadow_id == constant_false_id_) {
        return type_manager_.GetConstantNull(*shadow_type).Id();
    }
    if (scalar_shadow_id == constant_true_id_) {
        return GetAllTrueConstant(shadow_type_id);
    }

    if (shadow_type->spv_type_ == SpvType::kVector || shadow_type->spv_type_ == SpvType::kVectorIdEXT) {
        uint32_t count = shadow_type->meta_.vector.component_count;
        uint32_t result_id = module_.TakeNextId();
        std::vector<uint32_t> words = {shadow_type_id, result_id};
        for (uint32_t i = 0; i < count; i++) words.push_back(scalar_shadow_id);
        block.CreateInstruction(spv::OpCompositeConstruct, words, inst_it);
        return result_id;
    }

    if (shadow_type->spv_type_ == SpvType::kArray) {
        uint32_t elem_type_id = shadow_type->inst_.Word(2);
        uint32_t elem_shadow = BroadcastShadow(block, inst_it, scalar_shadow_id, elem_type_id);
        const Constant* length_const = type_manager_.FindConstantById(shadow_type->inst_.Word(3));
        if (!length_const) return scalar_shadow_id;
        uint32_t length = length_const->GetValueUint32();
        uint32_t result_id = module_.TakeNextId();
        std::vector<uint32_t> words = {shadow_type_id, result_id};
        for (uint32_t i = 0; i < length; i++) words.push_back(elem_shadow);
        block.CreateInstruction(spv::OpCompositeConstruct, words, inst_it);
        return result_id;
    }

    if (shadow_type->spv_type_ == SpvType::kStruct) {
        uint32_t result_id = module_.TakeNextId();
        std::vector<uint32_t> words = {shadow_type_id, result_id};
        for (uint32_t i = 2; i < shadow_type->inst_.Length(); i++) {
            words.push_back(BroadcastShadow(block, inst_it, scalar_shadow_id, shadow_type->inst_.Word(i)));
        }
        block.CreateInstruction(spv::OpCompositeConstruct, words, inst_it);
        return result_id;
    }

    return scalar_shadow_id;
}

// Reduce a composite shadow value to a scalar bool (true = all clean, false = any poison)
uint32_t PoisonPass::ReduceShadowToScalar(BasicBlock& block, InstructionIt* inst_it, uint32_t shadow_id, uint32_t shadow_type_id) {
    const Type& bool_type = type_manager_.GetTypeBool();
    const Type* shadow_type = type_manager_.FindTypeById(shadow_type_id);
    if (!shadow_type || shadow_type->spv_type_ == SpvType::kBool) {
        return shadow_id;
    }

    if (shadow_type->spv_type_ == SpvType::kVector || shadow_type->spv_type_ == SpvType::kVectorIdEXT) {
        uint32_t result_id = module_.TakeNextId();
        block.CreateInstruction(spv::OpAll, {bool_type.Id(), result_id, shadow_id}, inst_it);
        return result_id;
    }

    if (shadow_type->spv_type_ == SpvType::kArray) {
        uint32_t elem_type_id = shadow_type->inst_.Word(2);
        const Constant* length_const = type_manager_.FindConstantById(shadow_type->inst_.Word(3));
        if (!length_const) return constant_false_id_;
        uint32_t length = length_const->GetValueUint32();
        uint32_t combined = constant_true_id_;
        for (uint32_t i = 0; i < length; i++) {
            uint32_t extracted_id = module_.TakeNextId();
            block.CreateInstruction(spv::OpCompositeExtract, {elem_type_id, extracted_id, shadow_id, i}, inst_it);
            uint32_t elem_scalar = ReduceShadowToScalar(block, inst_it, extracted_id, elem_type_id);
            if (combined == constant_true_id_) {
                combined = elem_scalar;
            } else {
                uint32_t and_id = module_.TakeNextId();
                block.CreateInstruction(spv::OpLogicalAnd, {bool_type.Id(), and_id, combined, elem_scalar}, inst_it);
                combined = and_id;
            }
        }
        return combined;
    }

    if (shadow_type->spv_type_ == SpvType::kStruct) {
        uint32_t combined = constant_true_id_;
        for (uint32_t i = 2; i < shadow_type->inst_.Length(); i++) {
            uint32_t member_type_id = shadow_type->inst_.Word(i);
            uint32_t extracted_id = module_.TakeNextId();
            block.CreateInstruction(spv::OpCompositeExtract, {member_type_id, extracted_id, shadow_id, i - 2}, inst_it);
            uint32_t member_scalar = ReduceShadowToScalar(block, inst_it, extracted_id, member_type_id);
            if (combined == constant_true_id_) {
                combined = member_scalar;
            } else {
                uint32_t and_id = module_.TakeNextId();
                block.CreateInstruction(spv::OpLogicalAnd, {bool_type.Id(), and_id, combined, member_scalar}, inst_it);
                combined = and_id;
            }
        }
        return combined;
    }

    return constant_false_id_;
}

void PoisonPass::CreateShadowVariables(Function& function) {
    shadow_var_map_.clear();
    shadow_value_map_.clear();
    shadow_value_type_map_.clear();

    const Type& bool_type = type_manager_.GetTypeBool();

    // false = "poison" (uninitialized), true = "clean" (initialized)
    constant_false_id_ = type_manager_.GetConstantNull(bool_type).Id();

    // Search for an existing OpConstantTrue in the module type/constant section
    constant_true_id_ = 0;
    for (const auto& inst : module_.types_values_constants_) {
        if (inst->Opcode() == spv::OpConstantTrue) {
            constant_true_id_ = inst->ResultId();
            break;
        }
    }

    if (constant_true_id_ == 0) {
        uint32_t id = module_.TakeNextId();
        auto new_inst = std::make_unique<Instruction>(3, spv::OpConstantTrue);
        new_inst->Fill({bool_type.Id(), id});
        constant_true_id_ = id;
        type_manager_.AddConstant(std::move(new_inst), bool_type);
    }

    BasicBlock& entry_block = *function.blocks_.front();

    for (uint32_t var_id : uninitialized_var_ids_) {
        const Instruction* var_inst = function.FindInstruction(var_id);
        if (!var_inst) {
            const Variable* var = type_manager_.FindVariableById(var_id);
            if (var) var_inst = &var->inst_;
        }
        if (!var_inst) continue;

        auto orig_sc = static_cast<spv::StorageClass>(var_inst->Word(3));

        const Type* ptr_type = type_manager_.FindTypeById(var_inst->TypeId());
        if (!ptr_type || ptr_type->spv_type_ != SpvType::kPointer) continue;
        uint32_t pointee_type_id = ptr_type->inst_.Word(3);

        uint32_t shadow_pointee_type_id = GetOrCreateShadowType(pointee_type_id);

        spv::StorageClass shadow_sc = (orig_sc == spv::StorageClassFunction) ? spv::StorageClassFunction : spv::StorageClassPrivate;

        const Type* spt = type_manager_.FindTypeById(shadow_pointee_type_id);
        if (!spt) continue;
        uint32_t shadow_ptr_type_id = type_manager_.GetTypePointer(shadow_sc, *spt).Id();

        bool is_contaminated = contaminated_var_ids_.count(var_id) != 0;
        uint32_t init_id;
        if (is_contaminated) {
            // Contaminated variables were originally initialized by the program;
            // they only become poison later when a poison value is stored into them.
            // Start clean (all-true) to avoid false positives on loads before contamination.
            init_id = GetAllTrueConstant(shadow_pointee_type_id);
        } else {
            // Truly uninitialized variables start as poison (all-false).
            init_id = type_manager_.GetConstantNull(*spt).Id();
        }

        uint32_t shadow_var_id;
        if (shadow_sc == spv::StorageClassFunction) {
            shadow_var_id = module_.TakeNextId();
            auto inject_it = entry_block.instructions_.begin();
            if (inject_it != entry_block.instructions_.end() && (*inject_it)->Opcode() == spv::OpLabel) {
                ++inject_it;
            }
            while (inject_it != entry_block.instructions_.end() && (*inject_it)->Opcode() == spv::OpVariable) {
                ++inject_it;
            }
            entry_block.CreateInstruction(
                spv::OpVariable, {shadow_ptr_type_id, shadow_var_id, static_cast<uint32_t>(shadow_sc), init_id}, &inject_it);
        } else {
            auto var_inst_new = std::make_unique<Instruction>(5, spv::OpVariable);
            shadow_var_id = module_.TakeNextId();
            var_inst_new->Fill({shadow_ptr_type_id, shadow_var_id, static_cast<uint32_t>(shadow_sc), init_id});
            type_manager_.AddVariable(std::move(var_inst_new), *spt);
            module_.AddInterfaceVariables(shadow_var_id, shadow_sc);
        }

        shadow_var_map_[var_id] = {shadow_var_id, shadow_pointee_type_id, shadow_sc};
    }
}

// =============================================================================
// Phase 3: Instrumentation
// =============================================================================

// Returns the shadow for a value. If the value was instrumented, returns its tracked shadow.
// For untracked values, returns a constant: all-true (clean) or all-false (poison).
// expected_shadow_type_id is used by OpPhi to get a type-matched composite constant
// instead of a scalar bool, avoiding type mismatches in the phi's shadow operands.
uint32_t PoisonPass::GetShadowValue(uint32_t id, uint32_t expected_shadow_type_id) {
    auto it = shadow_value_map_.find(id);
    if (it != shadow_value_map_.end()) return it->second;
    if (expected_shadow_type_id != 0) {
        if (IsPotentiallyPoison(id)) {
            const Type* type = type_manager_.FindTypeById(expected_shadow_type_id);
            return type ? type_manager_.GetConstantNull(*type).Id() : constant_false_id_;
        }
        return GetAllTrueConstant(expected_shadow_type_id);
    }
    if (IsPotentiallyPoison(id)) return constant_false_id_;
    return constant_true_id_;
}

// Returns the shadow type for a value. Defaults to scalar bool for untracked values.
uint32_t PoisonPass::GetShadowType(uint32_t id) {
    auto it = shadow_value_type_map_.find(id);
    if (it != shadow_value_type_map_.end()) return it->second;
    return type_manager_.GetTypeBool().Id();
}

void PoisonPass::SetShadowValue(uint32_t value_id, uint32_t shadow_id, uint32_t shadow_type_id) {
    shadow_value_map_[value_id] = shadow_id;
    shadow_value_type_map_[value_id] = shadow_type_id;
}

// Returns (or creates) an OpConstantComposite where every leaf is true (all clean).
// Used to initialize contaminated shadow variables and to represent "definitely not poison"
// for composite types.
uint32_t PoisonPass::GetAllTrueConstant(uint32_t shadow_type_id) {
    const uint32_t bool_type_id = type_manager_.GetTypeBool().Id();
    if (shadow_type_id == bool_type_id) return constant_true_id_;

    auto it = all_true_constants_.find(shadow_type_id);
    if (it != all_true_constants_.end()) return it->second;

    const Type* shadow_type = type_manager_.FindTypeById(shadow_type_id);
    if (!shadow_type) return constant_true_id_;

    uint32_t id = module_.TakeNextId();

    if (shadow_type->spv_type_ == SpvType::kVector || shadow_type->spv_type_ == SpvType::kVectorIdEXT) {
        uint32_t count = shadow_type->meta_.vector.component_count;
        auto new_inst = std::make_unique<Instruction>(3 + count, spv::OpConstantComposite);
        std::vector<uint32_t> words = {shadow_type_id, id};
        for (uint32_t i = 0; i < count; i++) words.push_back(constant_true_id_);
        new_inst->Fill(words);
        type_manager_.AddConstant(std::move(new_inst), *shadow_type);
        all_true_constants_[shadow_type_id] = id;
        return id;
    }

    if (shadow_type->spv_type_ == SpvType::kArray) {
        uint32_t elem_type_id = shadow_type->inst_.Word(2);
        uint32_t elem_true = GetAllTrueConstant(elem_type_id);
        const Constant* length_const = type_manager_.FindConstantById(shadow_type->inst_.Word(3));
        if (!length_const) return constant_true_id_;
        uint32_t length = length_const->GetValueUint32();
        auto new_inst = std::make_unique<Instruction>(3 + length, spv::OpConstantComposite);
        std::vector<uint32_t> words = {shadow_type_id, id};
        for (uint32_t i = 0; i < length; i++) words.push_back(elem_true);
        new_inst->Fill(words);
        type_manager_.AddConstant(std::move(new_inst), *shadow_type);
        all_true_constants_[shadow_type_id] = id;
        return id;
    }

    if (shadow_type->spv_type_ == SpvType::kStruct) {
        std::vector<uint32_t> words = {shadow_type_id, id};
        for (uint32_t i = 2; i < shadow_type->inst_.Length(); i++) {
            words.push_back(GetAllTrueConstant(shadow_type->inst_.Word(i)));
        }
        auto new_inst = std::make_unique<Instruction>(static_cast<uint32_t>(words.size() + 1), spv::OpConstantComposite);
        new_inst->Fill(words);
        type_manager_.AddConstant(std::move(new_inst), *shadow_type);
        all_true_constants_[shadow_type_id] = id;
        return id;
    }

    return constant_true_id_;
}

// Componentwise AND of two shadow values of the same type.
// For bool/vector, emits a single OpLogicalAnd. For struct/array, recursively
// extracts members, ANDs them, and reconstructs.
uint32_t PoisonPass::ComponentwiseAnd(BasicBlock& block, InstructionIt* inst_it, uint32_t a_id, uint32_t b_id,
                                      uint32_t shadow_type_id) {
    if (a_id == b_id) return a_id;

    const Type* shadow_type = type_manager_.FindTypeById(shadow_type_id);
    assert(shadow_type);

    if (shadow_type->spv_type_ == SpvType::kBool || shadow_type->spv_type_ == SpvType::kVector ||
        shadow_type->spv_type_ == SpvType::kVectorIdEXT) {
        uint32_t result_id = module_.TakeNextId();
        block.CreateInstruction(spv::OpLogicalAnd, {shadow_type_id, result_id, a_id, b_id}, inst_it);
        return result_id;
    }

    if (shadow_type->spv_type_ == SpvType::kArray) {
        uint32_t elem_type_id = shadow_type->inst_.Word(2);
        const Constant* length_const = type_manager_.FindConstantById(shadow_type->inst_.Word(3));
        assert(length_const);
        uint32_t length = length_const->GetValueUint32();
        uint32_t result_id = module_.TakeNextId();
        std::vector<uint32_t> words = {shadow_type_id, result_id};
        for (uint32_t i = 0; i < length; i++) {
            uint32_t a_elem = module_.TakeNextId();
            block.CreateInstruction(spv::OpCompositeExtract, {elem_type_id, a_elem, a_id, i}, inst_it);
            uint32_t b_elem = module_.TakeNextId();
            block.CreateInstruction(spv::OpCompositeExtract, {elem_type_id, b_elem, b_id, i}, inst_it);
            words.push_back(ComponentwiseAnd(block, inst_it, a_elem, b_elem, elem_type_id));
        }
        block.CreateInstruction(spv::OpCompositeConstruct, words, inst_it);
        return result_id;
    }

    if (shadow_type->spv_type_ == SpvType::kStruct) {
        uint32_t result_id = module_.TakeNextId();
        std::vector<uint32_t> words = {shadow_type_id, result_id};
        for (uint32_t i = 2; i < shadow_type->inst_.Length(); i++) {
            uint32_t member_type_id = shadow_type->inst_.Word(i);
            uint32_t a_member = module_.TakeNextId();
            block.CreateInstruction(spv::OpCompositeExtract, {member_type_id, a_member, a_id, i - 2}, inst_it);
            uint32_t b_member = module_.TakeNextId();
            block.CreateInstruction(spv::OpCompositeExtract, {member_type_id, b_member, b_id, i - 2}, inst_it);
            words.push_back(ComponentwiseAnd(block, inst_it, a_member, b_member, member_type_id));
        }
        block.CreateInstruction(spv::OpCompositeConstruct, words, inst_it);
        return result_id;
    }

    assert(false);
    return a_id;
}

// Convert a shadow value from one shadow type to another by reducing to scalar
// then broadcasting. Both functions are no-ops when already at the target shape
// (ReduceShadowToScalar returns immediately for bool, BroadcastShadow returns
// immediately when target is bool), so this handles all cases uniformly.
uint32_t PoisonPass::EnsureShadowType(BasicBlock& block, InstructionIt* inst_it, uint32_t shadow_id, uint32_t from_type_id,
                                      uint32_t to_type_id) {
    if (from_type_id == to_type_id) return shadow_id;
    uint32_t scalar = ReduceShadowToScalar(block, inst_it, shadow_id, from_type_id);
    return BroadcastShadow(block, inst_it, scalar, to_type_id);
}

void PoisonPass::EmitPoisonError(BasicBlock& block, InstructionIt* inst_it, uint32_t shadow_id, const Instruction& trigger_inst,
                                 uint32_t error_sub_code) {
    const uint32_t function_def = GetLinkFunction(link_function_id_, kOfflineFunction);

    const Type& bool_type = type_manager_.GetTypeBool();
    const uint32_t void_type = type_manager_.GetTypeVoid().Id();

    // shadow_id is true when clean, false when poison. Negate to get is_poison.
    const uint32_t is_poison_id = module_.TakeNextId();
    block.CreateInstruction(spv::OpLogicalNot, {bool_type.Id(), is_poison_id, shadow_id}, inst_it);

    const uint32_t inst_offset = trigger_inst.GetPositionOffset();
    const uint32_t inst_offset_id = type_manager_.CreateConstantUInt32(inst_offset).Id();
    const uint32_t opcode_id = type_manager_.CreateConstantUInt32(trigger_inst.Opcode()).Id();
    const uint32_t sub_code_id = type_manager_.CreateConstantUInt32(error_sub_code).Id();

    const uint32_t function_result = module_.TakeNextId();
    block.CreateInstruction(spv::OpFunctionCall,
                            {void_type, function_result, function_def, is_poison_id, inst_offset_id, opcode_id, sub_code_id},
                            inst_it);

    module_.need_log_error_ = true;
}

// Main per-function instrumentation loop. Seeds shadow_pointer_map_ from shadow_var_map_,
// then iterates every instruction:
//  - PropagateShadowPointer: forward-propagate shadow pointers through access chains / copy object
//  - OpLoad/OpStore: bridge shadow variables and shadow values (may split the basic block)
//  - Non-poison results: check UB triggers (branch, switch, return, function call cleanup)
//  - Poison results: dispatch to specialized or generic shadow propagation
void PoisonPass::InstrumentFunction(Function& function) {
    std::unordered_set<uint32_t> func_param_ids;
    for (const auto& pre_inst : function.pre_block_inst_) {
        if (pre_inst->Opcode() == spv::OpFunctionParameter) {
            func_param_ids.insert(pre_inst->ResultId());
        }
    }

    shadow_pointer_map_.clear();
    for (auto& [var_id, info] : shadow_var_map_) {
        shadow_pointer_map_[var_id] = {info.shadow_var_id, info.shadow_pointee_type_id, info.shadow_sc, var_id};
    }

    for (auto block_it = function.blocks_.begin(); block_it != function.blocks_.end(); ++block_it) {
        BasicBlock& block = **block_it;

        for (auto inst_it = block.instructions_.begin(); inst_it != block.instructions_.end(); ++inst_it) {
            const Instruction& inst = **inst_it;
            const auto opcode = static_cast<spv::Op>(inst.Opcode());
            const uint32_t result_id = inst.ResultId();

            // Propagate shadow pointers forward through pointer-deriving instructions
            // (OpAccessChain, OpPtrAccessChain, OpCopyObject) so that
            // InstrumentLoad/InstrumentStore can look up the shadow pointer directly.
            PropagateShadowPointer(block, &inst_it, inst);

            // Load/store bridge shadow variables/pointers and shadow values,
            // run regardless of IsPotentiallyPoison, and may split the basic block.
            if (opcode == spv::OpLoad) {
                if (InstrumentLoad(function, block, block_it, &inst_it, inst)) {
                    ++block_it;
                    ++block_it;
                    break;
                }
                continue;
            }
            if (opcode == spv::OpStore) {
                if (InstrumentStore(function, block, block_it, &inst_it, inst, func_param_ids)) {
                    ++block_it;
                    ++block_it;
                    break;
                }
                continue;
            }
            if (result_id == 0 || !IsPotentiallyPoison(result_id)) {
                InstrumentNonPoisonResult(block, &inst_it, inst);
                continue;
            }
            InstrumentPoisonResult(function, block, &inst_it, inst);
        }
    }
}

// After a function call, mark shadow variables for any pointer arguments as clean.
// The callee may have written to out/inout parameters, so we conservatively assume
// they are now initialized.
void PoisonPass::MarkCallArgsClean(BasicBlock& block, InstructionIt* inst_it, const Instruction& call_inst) {
    for (uint32_t w = 4; w < call_inst.Length(); w++) {
        if (const ShadowPtrInfo* spi = FindShadowPointer(call_inst.Word(w))) {
            auto sit = shadow_var_map_.find(spi->var_id);
            if (sit != shadow_var_map_.end()) {
                const ShadowVarInfo& si = sit->second;
                uint32_t all_true = GetAllTrueConstant(si.shadow_pointee_type_id);
                block.CreateInstruction(spv::OpStore, {si.shadow_var_id, all_true}, inst_it);
            }
        }
    }
}

// Handles OpLoad: two cases depending on whether the pointer itself or the loaded value is poison.
//  1. Poison pointer dereference (BDA/variable pointers): the pointer operand is derived from a
//     poison value. Emit an error and wrap the load in a conditional to prevent GPU crash.
//     Returns true (caller must skip 2 blocks for the injected if/else).
//  2. Tracked variable load: the pointer is a tracked local variable's shadow pointer.
//     Load the shadow value and register it for downstream propagation.
bool PoisonPass::InstrumentLoad(Function& function, BasicBlock& block, BasicBlockIt block_it, InstructionIt* inst_it,
                                const Instruction& inst) {
    const uint32_t bool_type_id = type_manager_.GetTypeBool().Id();
    const uint32_t result_id = inst.ResultId();
    uint32_t ptr_id = inst.Operand(0);

    // UB trigger: dereferencing a poison pointer (e.g. BDA from uninit uint64, variable pointers).
    // Must skip the actual load to avoid GPU crash from garbage pointer.
    if (IsPotentiallyPoison(ptr_id)) {
        uint32_t ptr_shadow = GetShadowValue(ptr_id);
        uint32_t ptr_shadow_type = GetShadowType(ptr_id);
        uint32_t scalar_shadow = EnsureShadowType(block, inst_it, ptr_shadow, ptr_shadow_type, bool_type_id);
        if (scalar_shadow != constant_true_id_) {
            EmitPoisonError(block, inst_it, scalar_shadow, inst, glsl::kErrorSubCode_PoisonValue_PoisonPointerDereference);
            instrumentations_count_++;

            // Wrap the load in a conditional: execute only if pointer is clean.
            // InjectFunctionPre moves the load to a "valid" block, creates a phi
            // with null for the "invalid" (skipped) path, and moves remaining
            // instructions to a merge block.
            InjectConditionalData ic_data = InjectFunctionPre(function, block_it, *inst_it);
            ic_data.function_result_id = scalar_shadow;
            InjectFunctionPost(block, ic_data);
            return true;  // caller must skip 2 blocks and break
        }
        return false;
    }

    const ShadowPtrInfo* sp = FindShadowPointer(ptr_id);
    if (!sp || !IsPotentiallyPoison(result_id)) return false;
    uint32_t shadow_ptr_id = sp->shadow_ptr_id;
    uint32_t shadow_load_type_id = sp->shadow_pointee_type_id;

    uint32_t loaded_shadow_id = module_.TakeNextId();
    block.CreateInstruction(spv::OpLoad, {shadow_load_type_id, loaded_shadow_id, shadow_ptr_id}, inst_it);

    SetShadowValue(result_id, loaded_shadow_id, shadow_load_type_id);
    return false;
}

// Handles OpStore: three cases.
//  1. Poison pointer dereference: the pointer operand is itself poison (BDA/variable pointers).
//     Emit an error and wrap the store in a conditional. Returns true.
//  2. External/parameter store of poison: the pointer is NOT a tracked local variable but the
//     stored value is poison. Emit a UB error (external store or store to out/inout param).
//  3. Tracked variable store: update the shadow variable to reflect the stored value's shadow.
bool PoisonPass::InstrumentStore(Function& function, BasicBlock& block, BasicBlockIt block_it, InstructionIt* inst_it,
                                 const Instruction& inst, const std::unordered_set<uint32_t>& func_param_ids) {
    const uint32_t bool_type_id = type_manager_.GetTypeBool().Id();
    uint32_t ptr_id = inst.Operand(0);
    uint32_t stored_value_id = inst.Operand(1);

    // UB trigger: storing through a poison pointer (e.g. BDA from uninit uint64, variable pointers).
    // Must skip the actual store to avoid GPU crash from garbage pointer.
    if (IsPotentiallyPoison(ptr_id)) {
        uint32_t ptr_shadow = GetShadowValue(ptr_id);
        uint32_t ptr_shadow_type = GetShadowType(ptr_id);
        uint32_t scalar_shadow = EnsureShadowType(block, inst_it, ptr_shadow, ptr_shadow_type, bool_type_id);
        if (scalar_shadow != constant_true_id_) {
            EmitPoisonError(block, inst_it, scalar_shadow, inst, glsl::kErrorSubCode_PoisonValue_PoisonPointerDereference);
            instrumentations_count_++;

            InjectConditionalData ic_data = InjectFunctionPre(function, block_it, *inst_it);
            ic_data.function_result_id = scalar_shadow;
            InjectFunctionPost(block, ic_data);
            return true;  // caller must skip 2 blocks and break
        }
        return false;
    }

    const ShadowPtrInfo* sp = FindShadowPointer(ptr_id);

    if (!sp) {
        // Not storing to a tracked variable - check for external store of poison
        if (IsPotentiallyPoison(stored_value_id)) {
            uint32_t sub_code = 0;
            const Instruction* ptr_inst = function.FindInstruction(ptr_id);
            const Type* ptr_type = ptr_inst ? type_manager_.FindTypeById(ptr_inst->TypeId()) : nullptr;
            if (ptr_type && ptr_type->spv_type_ == SpvType::kPointer) {
                auto sc = static_cast<spv::StorageClass>(ptr_type->inst_.Word(2));
                if (!IsLocalStorageClass(sc)) {
                    sub_code = glsl::kErrorSubCode_PoisonValue_UninitializedVariable;
                }
            }
            if (sub_code == 0) {
                uint32_t base_ptr = ptr_id;
                if (ptr_inst && (ptr_inst->Opcode() == spv::OpAccessChain || ptr_inst->Opcode() == spv::OpInBoundsAccessChain ||
                                 ptr_inst->Opcode() == spv::OpPtrAccessChain)) {
                    base_ptr = ptr_inst->Operand(0);
                }
                if (func_param_ids.count(base_ptr)) {
                    sub_code = glsl::kErrorSubCode_PoisonValue_StoreToFunctionParam;
                }
            }
            if (sub_code != 0) {
                uint32_t value_shadow = GetShadowValue(stored_value_id);
                uint32_t value_shadow_type = GetShadowType(stored_value_id);
                uint32_t scalar_shadow = EnsureShadowType(block, inst_it, value_shadow, value_shadow_type, bool_type_id);
                if (scalar_shadow != constant_true_id_) {
                    EmitPoisonError(block, inst_it, scalar_shadow, inst, sub_code);
                    instrumentations_count_++;
                }
            }
        }
        return false;
    }

    // Storing to a tracked variable - update its shadow
    uint32_t value_shadow = GetShadowValue(stored_value_id);
    uint32_t value_shadow_type = GetShadowType(stored_value_id);
    uint32_t converted_shadow = EnsureShadowType(block, inst_it, value_shadow, value_shadow_type, sp->shadow_pointee_type_id);
    block.CreateInstruction(spv::OpStore, {sp->shadow_ptr_id, converted_shadow}, inst_it);
    return false;
}

// Handles instructions whose result is NOT potentially poison (or that have no result),
// but whose operands might be. These are UB triggers where poison flows into control flow
// or escapes the function:
//   OpBranchConditional - poison condition replaced with safe constant after error
//   OpSwitch            - poison selector replaced with zero after error
//   OpReturnValue       - error emitted for poison return
//   OpFunctionCall      - post-call cleanup (mark out/inout args clean via MarkCallArgsClean)
void PoisonPass::InstrumentNonPoisonResult(BasicBlock& block, InstructionIt* inst_it, const Instruction& inst) {
    const uint32_t bool_type_id = type_manager_.GetTypeBool().Id();
    const auto opcode = static_cast<spv::Op>(inst.Opcode());

    if (opcode == spv::OpBranchConditional) {
        uint32_t cond_id = inst.Operand(0);
        if (IsPotentiallyPoison(cond_id)) {
            uint32_t cond_shadow = GetShadowValue(cond_id);
            uint32_t cond_shadow_type = GetShadowType(cond_id);
            uint32_t scalar_shadow = EnsureShadowType(block, inst_it, cond_shadow, cond_shadow_type, bool_type_id);
            if (scalar_shadow != constant_true_id_) {
                auto insert_it = *inst_it;
                bool before_merge = false;
                if (insert_it != block.instructions_.begin()) {
                    auto prev = std::prev(insert_it);
                    auto prev_op = static_cast<spv::Op>((*prev)->Opcode());
                    if (prev_op == spv::OpSelectionMerge || prev_op == spv::OpLoopMerge) {
                        insert_it = prev;
                        before_merge = true;
                    }
                }
                EmitPoisonError(block, &insert_it, scalar_shadow, inst);
                instrumentations_count_++;

                // Poison branch conditions are replaced with safe constants after error reporting, because
                // compilers may optimize away unreachable code
                uint32_t safe_cond_id = module_.TakeNextId();
                block.CreateInstruction(spv::OpSelect, {bool_type_id, safe_cond_id, scalar_shadow, cond_id, constant_false_id_},
                                        &insert_it);

                *inst_it = before_merge ? std::next(insert_it) : insert_it;
                (**inst_it)->UpdateWord(1, safe_cond_id);
            }
        }
    } else if (opcode == spv::OpSwitch) {
        uint32_t selector_id = inst.Operand(0);
        if (IsPotentiallyPoison(selector_id)) {
            uint32_t selector_shadow = GetShadowValue(selector_id);
            uint32_t selector_shadow_type = GetShadowType(selector_id);
            uint32_t scalar_shadow = EnsureShadowType(block, inst_it, selector_shadow, selector_shadow_type, bool_type_id);
            if (scalar_shadow != constant_true_id_) {
                auto insert_it = *inst_it;
                bool before_merge = false;
                if (insert_it != block.instructions_.begin()) {
                    auto prev = std::prev(insert_it);
                    auto prev_op = static_cast<spv::Op>((*prev)->Opcode());
                    if (prev_op == spv::OpSelectionMerge || prev_op == spv::OpLoopMerge) {
                        insert_it = prev;
                        before_merge = true;
                    }
                }
                EmitPoisonError(block, &insert_it, scalar_shadow, inst);
                instrumentations_count_++;

                const uint32_t zero_id = type_manager_.GetConstantZeroUint32().Id();
                uint32_t safe_selector_id = module_.TakeNextId();
                const Type& selector_type = type_manager_.GetTypeInt(32, false);
                block.CreateInstruction(spv::OpSelect, {selector_type.Id(), safe_selector_id, scalar_shadow, selector_id, zero_id},
                                        &insert_it);

                *inst_it = before_merge ? std::next(insert_it) : insert_it;
                (**inst_it)->UpdateWord(1, safe_selector_id);
            }
        }
    } else if (opcode == spv::OpReturnValue) {
        uint32_t value_id = inst.Operand(0);
        if (IsPotentiallyPoison(value_id)) {
            uint32_t value_shadow = GetShadowValue(value_id);
            uint32_t value_shadow_type = GetShadowType(value_id);
            uint32_t scalar_shadow = EnsureShadowType(block, inst_it, value_shadow, value_shadow_type, bool_type_id);
            if (scalar_shadow != constant_true_id_) {
                EmitPoisonError(block, inst_it, scalar_shadow, inst);
                instrumentations_count_++;
            }
        }
    } else if (opcode == spv::OpFunctionCall) {
        MarkCallArgsClean(block, inst_it, inst);
    }
}

void PoisonPass::InstrumentPoisonResult(Function& function, BasicBlock& block, InstructionIt* inst_it, const Instruction& inst) {
    const uint32_t bool_type_id = type_manager_.GetTypeBool().Id();
    const auto opcode = static_cast<spv::Op>(inst.Opcode());
    const uint32_t result_id = inst.ResultId();

    // Cooperative matrix instructions are opaque; treat as clean
    if (opcode == spv::OpCooperativeMatrixLoadKHR || opcode == spv::OpCooperativeMatrixMulAddKHR ||
        opcode == spv::OpCooperativeMatrixLengthKHR) {
        SetShadowValue(result_id, constant_true_id_, bool_type_id);
        return;
    }

    // UB triggers - result-producing error-reporting instructions
    if (opcode == spv::OpFunctionCall) {
        InstrumentFunctionCall(block, inst_it, inst);
        return;
    }
    if (opcode == spv::OpAccessChain || opcode == spv::OpInBoundsAccessChain || opcode == spv::OpPtrAccessChain) {
        InstrumentAccessChain(block, inst_it, inst);
        return;
    }

    // Pass-through - per-component shadow data movement
    if (opcode == spv::OpCompositeExtract) {
        InstrumentCompositeExtract(block, inst_it, inst);
        return;
    }
    if (opcode == spv::OpCompositeInsert) {
        InstrumentCompositeInsert(block, inst_it, inst);
        return;
    }
    if (opcode == spv::OpCompositeConstruct) {
        InstrumentCompositeConstruct(block, inst_it, inst);
        return;
    }
    if (opcode == spv::OpVectorExtractDynamic) {
        InstrumentVectorExtractDynamic(function, block, inst_it, inst);
        return;
    }
    if (opcode == spv::OpVectorInsertDynamic) {
        InstrumentVectorInsertDynamic(block, inst_it, inst);
        return;
    }
    if (opcode == spv::OpVectorShuffle) {
        InstrumentVectorShuffle(function, block, inst_it, inst);
        return;
    }
    if (opcode == spv::OpSelect) {
        InstrumentSelect(block, inst_it, inst);
        return;
    }
    if (opcode == spv::OpPhi) {
        InstrumentPhi(block, inst_it, inst);
        return;
    }
    if (opcode == spv::OpCopyObject) {
        uint32_t src_id = inst.Operand(0);
        SetShadowValue(result_id, GetShadowValue(src_id), GetShadowType(src_id));
        return;
    }
    if (opcode == spv::OpCopyLogical) {
        uint32_t src_id = inst.Operand(0);
        uint32_t src_shadow = GetShadowValue(src_id);
        uint32_t src_shadow_type = GetShadowType(src_id);
        uint32_t result_shadow_type_id = GetOrCreateShadowType(inst.TypeId());
        if (src_shadow_type == result_shadow_type_id) {
            SetShadowValue(result_id, src_shadow, src_shadow_type);
        } else {
            uint32_t result_shadow_id = module_.TakeNextId();
            block.CreateInstruction(spv::OpCopyLogical, {result_shadow_type_id, result_shadow_id, src_shadow}, inst_it);
            SetShadowValue(result_id, result_shadow_id, result_shadow_type_id);
        }
        return;
    }
    if (opcode == spv::OpCompositeConstructReplicateEXT) {
        uint32_t value_id = inst.Operand(0);
        uint32_t value_shadow = GetShadowValue(value_id);
        uint32_t result_shadow_type_id = GetOrCreateShadowType(inst.TypeId());
        uint32_t result_shadow_id = module_.TakeNextId();
        block.CreateInstruction(spv::OpCompositeConstructReplicateEXT, {result_shadow_type_id, result_shadow_id, value_shadow},
                                inst_it);
        SetShadowValue(result_id, result_shadow_id, result_shadow_type_id);
        return;
    }

    // Propagation - generic: AND of all poison operand shadows
    {
        uint32_t result_shadow_type_id = GetOrCreateShadowType(inst.TypeId());
        uint32_t combined_shadow = 0;

        inst.ForEachIdOperand(
            [&](uint32_t id) {
                if (id == result_id || id == inst.TypeId()) return;
                if (!IsPotentiallyPoison(id)) return;

                uint32_t operand_shadow = GetShadowValue(id);
                uint32_t operand_shadow_type = GetShadowType(id);
                uint32_t converted = EnsureShadowType(block, inst_it, operand_shadow, operand_shadow_type, result_shadow_type_id);

                if (combined_shadow == 0) {
                    combined_shadow = converted;
                } else {
                    combined_shadow = ComponentwiseAnd(block, inst_it, combined_shadow, converted, result_shadow_type_id);
                }
            },
            /*include_ambiguous=*/false);

        if (combined_shadow == 0) {
            combined_shadow = GetAllTrueConstant(result_shadow_type_id);
        }
        SetShadowValue(result_id, combined_shadow, result_shadow_type_id);
    }
}

// =============================================================================
// UB triggers - result-producing error-reporting instructions
// =============================================================================

// Conservative cross-function poison propagation: the call result's shadow is the AND of
// all argument shadows (both value and pointer args), reduced to scalar then broadcast
// to match the result type. After the call, pointer args are marked clean (MarkCallArgsClean).
void PoisonPass::InstrumentFunctionCall(BasicBlock& block, InstructionIt* inst_it, const Instruction& inst) {
    const uint32_t bool_type_id = type_manager_.GetTypeBool().Id();
    const uint32_t result_id = inst.ResultId();
    uint32_t result_shadow_type_id = GetOrCreateShadowType(inst.TypeId());
    uint32_t combined_shadow = 0;

    for (uint32_t w = 4; w < inst.Length(); w++) {
        uint32_t arg_id = inst.Word(w);
        uint32_t arg_scalar_shadow = 0;

        if (IsPotentiallyPoison(arg_id)) {
            uint32_t s = GetShadowValue(arg_id);
            arg_scalar_shadow = EnsureShadowType(block, inst_it, s, GetShadowType(arg_id), bool_type_id);
        } else if (const ShadowPtrInfo* spi = FindShadowPointer(arg_id)) {
            auto sit = shadow_var_map_.find(spi->var_id);
            if (sit != shadow_var_map_.end()) {
                const ShadowVarInfo& si = sit->second;
                uint32_t loaded = module_.TakeNextId();
                block.CreateInstruction(spv::OpLoad, {si.shadow_pointee_type_id, loaded, si.shadow_var_id}, inst_it);
                arg_scalar_shadow = EnsureShadowType(block, inst_it, loaded, si.shadow_pointee_type_id, bool_type_id);
            }
        }

        if (arg_scalar_shadow == 0) continue;
        if (combined_shadow == 0) {
            combined_shadow = arg_scalar_shadow;
        } else {
            uint32_t new_combined = module_.TakeNextId();
            block.CreateInstruction(spv::OpLogicalAnd, {bool_type_id, new_combined, combined_shadow, arg_scalar_shadow}, inst_it);
            combined_shadow = new_combined;
        }
    }

    if (combined_shadow == 0) {
        combined_shadow = GetAllTrueConstant(result_shadow_type_id);
    } else {
        combined_shadow = EnsureShadowType(block, inst_it, combined_shadow, bool_type_id, result_shadow_type_id);
    }
    SetShadowValue(result_id, combined_shadow, result_shadow_type_id);
    MarkCallArgsClean(block, inst_it, inst);
}

// A poison access chain index is UB. Emits an error for the first poison index,
// and combines all poison index shadows into a single scalar for the result.
void PoisonPass::InstrumentAccessChain(BasicBlock& block, InstructionIt* inst_it, const Instruction& inst) {
    const uint32_t bool_type_id = type_manager_.GetTypeBool().Id();
    const uint32_t result_id = inst.ResultId();
    const uint32_t first_index_word = 4;

    uint32_t combined = 0;
    bool error_emitted = false;
    for (uint32_t w = first_index_word; w < inst.Length(); w++) {
        uint32_t idx_id = inst.Word(w);
        if (!IsPotentiallyPoison(idx_id)) continue;
        uint32_t idx_shadow = GetShadowValue(idx_id);
        uint32_t idx_shadow_type = GetShadowType(idx_id);
        uint32_t scalar_shadow = EnsureShadowType(block, inst_it, idx_shadow, idx_shadow_type, bool_type_id);

        if (!error_emitted && scalar_shadow != constant_true_id_) {
            EmitPoisonError(block, inst_it, scalar_shadow, inst);
            instrumentations_count_++;
            error_emitted = true;
        }

        if (combined == 0) {
            combined = scalar_shadow;
        } else {
            uint32_t new_combined = module_.TakeNextId();
            block.CreateInstruction(spv::OpLogicalAnd, {bool_type_id, new_combined, combined, scalar_shadow}, inst_it);
            combined = new_combined;
        }
    }
    if (combined != 0) {
        SetShadowValue(result_id, combined, bool_type_id);
    }
}

// =============================================================================
// Pass-through - per-component shadow data movement
// =============================================================================

void PoisonPass::InstrumentCompositeExtract(BasicBlock& block, InstructionIt* inst_it, const Instruction& inst) {
    const uint32_t result_id = inst.ResultId();
    uint32_t composite_id = inst.Operand(0);
    uint32_t result_shadow_type_id = GetOrCreateShadowType(inst.TypeId());
    uint32_t composite_shadow = GetShadowValue(composite_id);
    uint32_t composite_shadow_type = GetShadowType(composite_id);
    const Type* cst = type_manager_.FindTypeById(composite_shadow_type);
    (void)cst;

    assert(cst);
    assert(cst->spv_type_ != SpvType::kBool && cst->spv_type_ != SpvType::kInt && cst->spv_type_ != SpvType::kFloat);
    uint32_t extracted_shadow = module_.TakeNextId();
    std::vector<uint32_t> words = {result_shadow_type_id, extracted_shadow, composite_shadow};
    for (uint32_t w = 4; w < inst.Length(); w++) {
        words.push_back(inst.Word(w));
    }
    block.CreateInstruction(spv::OpCompositeExtract, words, inst_it);
    SetShadowValue(result_id, extracted_shadow, result_shadow_type_id);
}

// Shadow OpCompositeInsert: walks the shadow type through the index chain to find the
// element's shadow type, converts the inserted object's shadow to match, then emits
// a shadow OpCompositeInsert to splice it into the composite shadow.
void PoisonPass::InstrumentCompositeInsert(BasicBlock& block, InstructionIt* inst_it, const Instruction& inst) {
    const uint32_t result_id = inst.ResultId();
    uint32_t object_id = inst.Operand(0);
    uint32_t composite_id = inst.Operand(1);
    uint32_t result_shadow_type_id = GetOrCreateShadowType(inst.TypeId());

    uint32_t composite_shadow = GetShadowValue(composite_id);
    uint32_t composite_shadow_type = GetShadowType(composite_id);
    composite_shadow = EnsureShadowType(block, inst_it, composite_shadow, composite_shadow_type, result_shadow_type_id);

    uint32_t object_shadow = GetShadowValue(object_id);
    uint32_t object_shadow_type = GetShadowType(object_id);
    const Type* result_shadow = type_manager_.FindTypeById(result_shadow_type_id);
    assert(result_shadow);
    const Type* elem_shadow_type = result_shadow;
    for (uint32_t w = 5; w < inst.Length(); w++) {
        uint32_t index = inst.Word(w);
        switch (elem_shadow_type->spv_type_) {
            case SpvType::kVector:
            case SpvType::kVectorIdEXT:
            case SpvType::kArray:
            case SpvType::kRuntimeArray:
                elem_shadow_type = type_manager_.FindTypeById(elem_shadow_type->inst_.Word(2));
                break;
            case SpvType::kStruct:
                assert(index + 2 < elem_shadow_type->inst_.Length());
                elem_shadow_type = type_manager_.FindTypeById(elem_shadow_type->inst_.Word(2 + index));
                break;
            default:
                assert(false);
                break;
        }
        assert(elem_shadow_type);
    }
    object_shadow = EnsureShadowType(block, inst_it, object_shadow, object_shadow_type, elem_shadow_type->Id());

    uint32_t result_shadow_id = module_.TakeNextId();
    std::vector<uint32_t> words = {result_shadow_type_id, result_shadow_id, object_shadow, composite_shadow};
    for (uint32_t w = 5; w < inst.Length(); w++) {
        words.push_back(inst.Word(w));
    }
    block.CreateInstruction(spv::OpCompositeInsert, words, inst_it);
    SetShadowValue(result_id, result_shadow_id, result_shadow_type_id);
}

void PoisonPass::InstrumentCompositeConstruct(BasicBlock& block, InstructionIt* inst_it, const Instruction& inst) {
    const uint32_t result_id = inst.ResultId();
    uint32_t result_shadow_type_id = GetOrCreateShadowType(inst.TypeId());
    uint32_t result_shadow_id = module_.TakeNextId();
    std::vector<uint32_t> words = {result_shadow_type_id, result_shadow_id};
    for (uint32_t w = 3; w < inst.Length(); w++) {
        words.push_back(GetShadowValue(inst.Word(w)));
    }
    block.CreateInstruction(spv::OpCompositeConstruct, words, inst_it);
    SetShadowValue(result_id, result_shadow_id, result_shadow_type_id);
}

void PoisonPass::InstrumentVectorExtractDynamic(Function& function, BasicBlock& block, InstructionIt* inst_it,
                                                const Instruction& inst) {
    const uint32_t bool_type_id = type_manager_.GetTypeBool().Id();
    const uint32_t result_id = inst.ResultId();
    uint32_t vec_id = inst.Operand(0);
    uint32_t idx_id = inst.Operand(1);
    uint32_t result_shadow_type_id = GetOrCreateShadowType(inst.TypeId());

    uint32_t vec_shadow = GetShadowValue(vec_id);
    uint32_t vec_shadow_type = GetShadowType(vec_id);

    const Instruction* vec_def = function.FindInstruction(vec_id);
    if (vec_def) {
        uint32_t expected = GetOrCreateShadowType(vec_def->TypeId());
        vec_shadow = EnsureShadowType(block, inst_it, vec_shadow, vec_shadow_type, expected);
        vec_shadow_type = GetOrCreateShadowType(vec_def->TypeId());
    }

    uint32_t extracted_shadow = module_.TakeNextId();
    block.CreateInstruction(spv::OpVectorExtractDynamic, {result_shadow_type_id, extracted_shadow, vec_shadow, idx_id}, inst_it);

    if (IsPotentiallyPoison(idx_id)) {
        uint32_t idx_shadow = GetShadowValue(idx_id);
        uint32_t idx_shadow_type = GetShadowType(idx_id);
        uint32_t scalar_idx_shadow = EnsureShadowType(block, inst_it, idx_shadow, idx_shadow_type, bool_type_id);
        uint32_t combined = module_.TakeNextId();
        block.CreateInstruction(spv::OpLogicalAnd, {bool_type_id, combined, extracted_shadow, scalar_idx_shadow}, inst_it);
        SetShadowValue(result_id, combined, bool_type_id);
    } else {
        SetShadowValue(result_id, extracted_shadow, result_shadow_type_id);
    }
}

void PoisonPass::InstrumentVectorInsertDynamic(BasicBlock& block, InstructionIt* inst_it, const Instruction& inst) {
    const uint32_t bool_type_id = type_manager_.GetTypeBool().Id();
    const uint32_t result_id = inst.ResultId();
    uint32_t vec_id = inst.Operand(0);
    uint32_t component_id = inst.Operand(1);
    uint32_t idx_id = inst.Operand(2);
    uint32_t result_shadow_type_id = GetOrCreateShadowType(inst.TypeId());

    uint32_t vec_shadow = GetShadowValue(vec_id);
    uint32_t vec_shadow_type = GetShadowType(vec_id);
    vec_shadow = EnsureShadowType(block, inst_it, vec_shadow, vec_shadow_type, result_shadow_type_id);

    uint32_t comp_shadow = GetShadowValue(component_id);

    uint32_t inserted_shadow = module_.TakeNextId();
    block.CreateInstruction(spv::OpVectorInsertDynamic, {result_shadow_type_id, inserted_shadow, vec_shadow, comp_shadow, idx_id},
                            inst_it);

    if (IsPotentiallyPoison(idx_id)) {
        uint32_t idx_shadow = GetShadowValue(idx_id);
        uint32_t idx_shadow_type = GetShadowType(idx_id);
        uint32_t scalar_idx_shadow = EnsureShadowType(block, inst_it, idx_shadow, idx_shadow_type, bool_type_id);
        uint32_t broadcast = BroadcastShadow(block, inst_it, scalar_idx_shadow, result_shadow_type_id);
        inserted_shadow = ComponentwiseAnd(block, inst_it, inserted_shadow, broadcast, result_shadow_type_id);
    }
    SetShadowValue(result_id, inserted_shadow, result_shadow_type_id);
}

void PoisonPass::InstrumentVectorShuffle(Function& function, BasicBlock& block, InstructionIt* inst_it, const Instruction& inst) {
    const uint32_t result_id = inst.ResultId();
    uint32_t vec1_id = inst.Operand(0);
    uint32_t vec2_id = inst.Operand(1);
    uint32_t result_shadow_type_id = GetOrCreateShadowType(inst.TypeId());

    uint32_t vec1_shadow = GetShadowValue(vec1_id);
    uint32_t vec1_shadow_type = GetShadowType(vec1_id);
    uint32_t vec2_shadow = GetShadowValue(vec2_id);
    uint32_t vec2_shadow_type = GetShadowType(vec2_id);

    const Instruction* vec1_def = function.FindInstruction(vec1_id);
    if (vec1_def) {
        uint32_t expected = GetOrCreateShadowType(vec1_def->TypeId());
        vec1_shadow = EnsureShadowType(block, inst_it, vec1_shadow, vec1_shadow_type, expected);
    }
    const Instruction* vec2_def = function.FindInstruction(vec2_id);
    if (vec2_def) {
        uint32_t expected = GetOrCreateShadowType(vec2_def->TypeId());
        vec2_shadow = EnsureShadowType(block, inst_it, vec2_shadow, vec2_shadow_type, expected);
    }

    uint32_t shuffled_shadow = module_.TakeNextId();
    std::vector<uint32_t> words = {result_shadow_type_id, shuffled_shadow, vec1_shadow, vec2_shadow};
    for (uint32_t w = 5; w < inst.Length(); w++) {
        words.push_back(inst.Word(w));
    }
    block.CreateInstruction(spv::OpVectorShuffle, words, inst_it);
    SetShadowValue(result_id, shuffled_shadow, result_shadow_type_id);
}

// Shadow for OpSelect: select between the two operand shadows using the original condition,
// then AND with the condition's own shadow (broadcast to match the result type). This ensures
// a poison condition contaminates all components of the result without lossy scalar reduction.
void PoisonPass::InstrumentSelect(BasicBlock& block, InstructionIt* inst_it, const Instruction& inst) {
    const uint32_t bool_type_id = type_manager_.GetTypeBool().Id();
    const uint32_t result_id = inst.ResultId();
    uint32_t cond_id = inst.Operand(0);
    uint32_t a_id = inst.Operand(1);
    uint32_t b_id = inst.Operand(2);
    uint32_t result_shadow_type_id = GetOrCreateShadowType(inst.TypeId());

    uint32_t cond_shadow = GetShadowValue(cond_id);
    uint32_t cond_shadow_type = GetShadowType(cond_id);
    uint32_t scalar_cond_shadow = EnsureShadowType(block, inst_it, cond_shadow, cond_shadow_type, bool_type_id);

    uint32_t a_shadow = GetShadowValue(a_id);
    uint32_t a_shadow_type = GetShadowType(a_id);
    a_shadow = EnsureShadowType(block, inst_it, a_shadow, a_shadow_type, result_shadow_type_id);

    uint32_t b_shadow = GetShadowValue(b_id);
    uint32_t b_shadow_type = GetShadowType(b_id);
    b_shadow = EnsureShadowType(block, inst_it, b_shadow, b_shadow_type, result_shadow_type_id);

    uint32_t selected_shadow = module_.TakeNextId();
    block.CreateInstruction(spv::OpSelect, {result_shadow_type_id, selected_shadow, cond_id, a_shadow, b_shadow}, inst_it);

    uint32_t cond_broadcast = BroadcastShadow(block, inst_it, scalar_cond_shadow, result_shadow_type_id);
    uint32_t result_shadow = ComponentwiseAnd(block, inst_it, cond_broadcast, selected_shadow, result_shadow_type_id);
    SetShadowValue(result_id, result_shadow, result_shadow_type_id);
}

// Shadow OpPhi mirrors the original: each incoming value gets its shadow (using
// expected_shadow_type_id to get a type-matched constant for untracked operands,
// avoiding type mismatches since all phi operands must have the same type).
void PoisonPass::InstrumentPhi(BasicBlock& block, InstructionIt* inst_it, const Instruction& inst) {
    const uint32_t result_id = inst.ResultId();
    uint32_t result_shadow_type_id = GetOrCreateShadowType(inst.TypeId());
    uint32_t phi_shadow = module_.TakeNextId();
    std::vector<uint32_t> phi_words = {result_shadow_type_id, phi_shadow};
    const uint32_t num_pairs = (inst.Length() - 3) / 2;
    for (uint32_t i = 0; i < num_pairs; i++) {
        uint32_t value_id = inst.Word(3 + i * 2);
        uint32_t block_id = inst.Word(4 + i * 2);
        phi_words.push_back(GetShadowValue(value_id, result_shadow_type_id));
        phi_words.push_back(block_id);
    }
    block.CreateInstruction(spv::OpPhi, phi_words, inst_it);
    SetShadowValue(result_id, phi_shadow, result_shadow_type_id);
}

// =============================================================================
// Main entry point
// =============================================================================

bool PoisonPass::Instrument() {
    for (Function& function : module_.functions_) {
        if (!function.called_from_target_ && function.id_ != module_.target_entry_point_id_) {
            continue;
        }

        // Phase 1: Static analysis
        FindUninitializedVariables(function);
        if (uninitialized_var_ids_.empty()) continue;

        ComputePotentiallyPoisonSet(function);
        if (potentially_poison_ids_.empty()) {
            uninitialized_var_ids_.clear();
            continue;
        }

        CreateShadowVariables(function);
        InstrumentFunction(function);
    }

    return instrumentations_count_ != 0;
}

}  // namespace spirv
}  // namespace gpuav
