/* Copyright (c) 2024-2025 LunarG, Inc.
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

#include <stdint.h>
#include <spirv/unified1/spirv.hpp>
#include "function_basic_block.h"
#include "link.h"
#include "containers/custom_containers.h"

namespace gpuav {
namespace spirv {

class Module;
struct Variable;
struct BasicBlock;
struct Type;

// The goal is to have the complex conditional inject control flow to be in a single spot
// To allow this, we create all the pieces for Pass, let it make its own function call, then use this data to apply the final
// control flow logic
struct InjectConditionalData {
    uint32_t merge_block_label;
    uint32_t valid_block_label;
    uint32_t invalid_block_label;
    uint32_t function_result_id;
    BasicBlockIt merge_block_it;
};

// Common helpers for all passes
// The pass takes the Module object and modifies it as needed
class Pass {
  public:
    // Needed to know where an error/warning comes from
    virtual const char* Name() const = 0;
    // Return true if code was instrumented/modified in anyway
    virtual bool Instrument() = 0;
    // Requiring because this becomes important/helpful while debugging
    virtual void PrintDebugInfo() const = 0;
    // Wrapper that each pass can use to start
    bool Run();

    // Finds (and creates if needed) decoration and returns the OpVariable it points to
    const Variable& GetBuiltinVariable(uint32_t built_in);

    // Returns the ID for OpCompositeConstruct it creates
    uint32_t GetStageInfo(Function& function, const BasicBlock& target_block_it, InstructionIt& out_inst_it);

    const Instruction* GetDecoration(uint32_t id, spv::Decoration decoration) const;
    const Instruction* GetMemberDecoration(uint32_t id, uint32_t member_index, spv::Decoration decoration) const;

    uint32_t FindTypeByteSize(uint32_t type_id, uint32_t matrix_stride = 0, bool col_major = false, bool in_matrix = false) const;
    uint32_t GetLastByte(const Type& descriptor_type, const std::vector<const Instruction*>& access_chain_insts, BasicBlock& block,
                         InstructionIt* inst_it);
    uint32_t FindOffsetInStruct(uint32_t struct_id, bool is_descriptor_array,
                                const std::vector<const Instruction*>& access_chain_insts) const;

    // Generate SPIR-V needed to help convert things to be uniformly uint32_t
    // If no inst_it is passed in, any new instructions will be added to end of the Block
    uint32_t ConvertTo32(uint32_t id, BasicBlock& block, InstructionIt* inst_it) const;
    uint32_t CastToUint32(uint32_t id, BasicBlock& block, InstructionIt* inst_it) const;

    bool IsMaxInstrumentationsCount() const;

    InjectConditionalData InjectFunctionPre(Function& function, const BasicBlockIt original_block_it, InstructionIt inst_it);
    void InjectFunctionPost(BasicBlock& original_block, const InjectConditionalData& ic_data);

  protected:
    Pass(Module& module, const OfflineModule& offline) : module_(module), link_info_(offline) {}
    Module& module_;

    // As various things are modifiying the instruction streams, we need to get back to where we were.
    // (normally set in the RequiresInstrumentation call)
    InstructionIt FindTargetInstruction(BasicBlock& block, const Instruction& target_instruction) const;

    uint32_t instrumentations_count_ = 0;

    // This is a very basic amount of Control Flow helpers to help track during any pass
    struct ControlFlow {
        bool in_loop = false;
        uint32_t merge_target_id = 0;
        void Update(const BasicBlock& block);
    } cf_;

    // Build up link info for each pass and the pass will apply it to the module if it contains functions
    LinkInfo link_info_;
    uint32_t GetLinkFunction(uint32_t& link_function_id, const OfflineFunction& offline);

    // Currently a just used to quickly see GPU runtime diff if the entire loop is not instrumented
    // (Still deciding how to properly handle slow loops, but need examine more traces)
    const bool debug_disable_loops_ = false;
};

// Push Constants can be used to determine the index into descriptor arrays (Example: https://godbolt.org/z/jTEaaExov)
// From examining many large shaders, the same access is made, but generated as a different OpLoad.
// spirv-opt is not going to remove the duplicate loads because it is designed to allow the compiler to decide how long
// it wants a single OpLoad to "live" (for register spilling algorithms)
struct DescriptroIndexPushConstantAccess {
    // The "final" ID that will determine the descriptor index
    uint32_t descriptor_index_id = 0;
    // We exploit the fact we only need to track the next ID that is an alias to |descriptor_index_id|
    // The goal is this is only updated as we keep finding a matching descriptor index
    uint32_t next_alias_id = 0;

    // Which member inside the PC Block (expressed as ID of the OpConsant)
    uint32_t member_index = 0;
    // Sometimes a single OpIAdd is applied to the PC value
    uint32_t add_id_value = 0;

    void Update(const Module& module, InstructionIt inst_it);
};

// We want to remove redundant instrumentation as it adds overhead to both compile time and runtime
// We create a block-scope tracking of all things with instrumentation
struct BlockDuplicateTracker {
    // hash or the arguments making it unique/same
    vvl::unordered_set<uint32_t> hashes;

    // The current goal is not to remove 100% of things as the trade-off to add something like SPIRV-Tools
    // PostDominatorAnalysis is high. Just trying to find the "simple" if/else cases removes many spots
    uint32_t merge_select_predecessor = 0;
    uint32_t branch_conditional_predecessor = 0;
    uint32_t switch_cases_predecessor = 0;  // will include default as well
};

// The function level used to hold the blocks
struct FunctionDuplicateTracker {
    BlockDuplicateTracker& GetAndUpdate(BasicBlock& block);

    // Return true if found a duplicate
    bool FindAndUpdate(BlockDuplicateTracker& block, uint32_t hash);

  private:
    vvl::unordered_map<uint32_t, BlockDuplicateTracker> blocks_;
};

}  // namespace spirv
}  // namespace gpuav