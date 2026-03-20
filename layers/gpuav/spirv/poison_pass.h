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

// Poison Pass - models poison value propagation and detects UB from their use at runtime.

#pragma once

#include "pass.h"
#include "gpuav/shaders/gpuav_error_codes.h"
#include <spirv/unified1/spirv.hpp>
#include <unordered_map>
#include <unordered_set>

namespace gpuav {
namespace spirv {

class Module;
struct Function;

class PoisonPass : public Pass {
  public:
    PoisonPass(Module& module);
    const char* Name() const final { return "PoisonPass"; }
    bool Instrument() final;
    void PrintDebugInfo() const final;

  private:
    // Static analysis to limit which variables are tracked
    void FindUninitializedVariables(Function& function);
    void ComputePotentiallyPoisonSet(Function& function);

    // Shadow types and variables
    uint32_t GetOrCreateShadowType(uint32_t type_id);
    void CreateShadowVariables(Function& function);

    // Instrumentation
    void InstrumentFunction(Function& function);
    bool InstrumentLoad(Function& function, BasicBlock& block, BasicBlockIt block_it, InstructionIt* inst_it,
                        const Instruction& inst);
    bool InstrumentStore(Function& function, BasicBlock& block, BasicBlockIt block_it, InstructionIt* inst_it,
                         const Instruction& inst, const std::unordered_set<uint32_t>& func_param_ids);
    void InstrumentNonPoisonResult(BasicBlock& block, InstructionIt* inst_it, const Instruction& inst);
    void InstrumentPoisonResult(Function& function, BasicBlock& block, InstructionIt* inst_it, const Instruction& inst);

    // Pass-through - per-component shadow data movement
    void InstrumentCompositeExtract(BasicBlock& block, InstructionIt* inst_it, const Instruction& inst);
    void InstrumentCompositeInsert(BasicBlock& block, InstructionIt* inst_it, const Instruction& inst);
    void InstrumentCompositeConstruct(BasicBlock& block, InstructionIt* inst_it, const Instruction& inst);
    void InstrumentVectorExtractDynamic(Function& function, BasicBlock& block, InstructionIt* inst_it, const Instruction& inst);
    void InstrumentVectorInsertDynamic(BasicBlock& block, InstructionIt* inst_it, const Instruction& inst);
    void InstrumentVectorShuffle(Function& function, BasicBlock& block, InstructionIt* inst_it, const Instruction& inst);
    void InstrumentSelect(BasicBlock& block, InstructionIt* inst_it, const Instruction& inst);
    void InstrumentPhi(BasicBlock& block, InstructionIt* inst_it, const Instruction& inst);

    // UB triggers - result-producing error-reporting instructions
    void InstrumentFunctionCall(BasicBlock& block, InstructionIt* inst_it, const Instruction& inst);
    void InstrumentAccessChain(BasicBlock& block, InstructionIt* inst_it, const Instruction& inst);
    void MarkCallArgsClean(BasicBlock& block, InstructionIt* inst_it, const Instruction& call_inst);
    void EmitPoisonError(BasicBlock& block, InstructionIt* inst_it, uint32_t shadow_id, const Instruction& trigger_inst,
                         uint32_t error_sub_code = glsl::kErrorSubCode_PoisonValue_UninitializedVariable);

    // Shadow value tracking (per-component: shadow type mirrors original type structure)
    uint32_t GetShadowValue(uint32_t id, uint32_t expected_shadow_type_id = 0);
    uint32_t GetShadowType(uint32_t id);
    void SetShadowValue(uint32_t value_id, uint32_t shadow_id, uint32_t shadow_type_id);
    uint32_t EnsureShadowType(BasicBlock& block, InstructionIt* inst_it, uint32_t shadow_id, uint32_t from_type_id,
                              uint32_t to_type_id);
    uint32_t GetAllTrueConstant(uint32_t shadow_type_id);

    bool IsPotentiallyPoison(uint32_t id) const { return potentially_poison_ids_.count(id) != 0; }
    uint32_t BroadcastShadow(BasicBlock& block, InstructionIt* inst_it, uint32_t scalar_shadow_id, uint32_t shadow_type_id);
    uint32_t ReduceShadowToScalar(BasicBlock& block, InstructionIt* inst_it, uint32_t shadow_id, uint32_t shadow_type_id);
    uint32_t ComponentwiseAnd(BasicBlock& block, InstructionIt* inst_it, uint32_t a_id, uint32_t b_id, uint32_t shadow_type_id);

    // Shadow pointer tracking: maps an original pointer id to its shadow pointer id.
    // Propagated forward through OpAccessChain, OpPtrAccessChain, OpCopyObject.
    struct ShadowPtrInfo {
        uint32_t shadow_ptr_id;
        uint32_t shadow_pointee_type_id;
        spv::StorageClass shadow_sc;
        uint32_t var_id;  // base tracked variable this pointer derives from
    };
    void PropagateShadowPointer(BasicBlock& block, InstructionIt* inst_it, const Instruction& inst);
    const ShadowPtrInfo* FindShadowPointer(uint32_t ptr_id) const;
    static bool IsLocalStorageClass(spv::StorageClass sc) {
        return sc == spv::StorageClassFunction || sc == spv::StorageClassPrivate || sc == spv::StorageClassWorkgroup;
    }

    struct ShadowVarInfo {
        uint32_t shadow_var_id;
        uint32_t shadow_pointee_type_id;
        spv::StorageClass shadow_sc;
    };

    std::unordered_set<uint32_t> uninitialized_var_ids_;
    std::unordered_set<uint32_t> all_local_var_ids_;
    std::unordered_set<uint32_t> contaminated_var_ids_;
    std::unordered_set<uint32_t> potentially_poison_ids_;
    std::unordered_map<uint32_t, uint32_t> shadow_type_cache_;
    std::unordered_map<uint32_t, ShadowVarInfo> shadow_var_map_;
    std::unordered_map<uint32_t, uint32_t> shadow_value_map_;
    std::unordered_map<uint32_t, uint32_t> shadow_value_type_map_;
    std::unordered_map<uint32_t, uint32_t> all_true_constants_;
    std::unordered_map<uint32_t, ShadowPtrInfo> shadow_pointer_map_;

    uint32_t constant_true_id_ = 0;
    uint32_t constant_false_id_ = 0;

    uint32_t link_function_id_ = 0;
};

}  // namespace spirv
}  // namespace gpuav
