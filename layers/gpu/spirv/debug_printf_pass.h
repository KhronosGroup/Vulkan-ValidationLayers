/* Copyright (c) 2024 LunarG, Inc.
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
#include "pass.h"

namespace gpu {
namespace spirv {

struct Type;

// Create a pass to instrument NonSemantic.DebugPrintf (GL_EXT_debug_printf) instructions
class DebugPrintfPass : public Pass {
  public:
    DebugPrintfPass(Module& module, u32 binding_slot = 0) : Pass(module), binding_slot_(binding_slot) {}
    bool Run() final;
    void PrintDebugInfo() final;

  private:
    bool AnalyzeInstruction(const Instruction& inst);
    void CreateFunctionCall(BasicBlockIt block_it, InstructionIt* inst_it);
    void CreateFunctionParams(u32 argument_id, const Type& argument_type, std::vector<u32>& params, BasicBlock& block,
                              InstructionIt* inst_it);
    void CreateDescriptorSet();
    void CreateBufferWriteFunction(u32 argument_count, u32 function_id);
    void Reset() final;

    bool Validate(const Function& current_function);

    const u32 binding_slot_;
    u32 ext_import_id_ = 0;

    // <number of arguments in the function call, function id>
    vvl::unordered_map<u32, u32> function_id_map_;
    u32 GetLinkFunctionId(u32 argument_count);

    u32 output_buffer_variable_id_ = 0;

    // Used to detect where 64-bit floats are
    u32 double_bitmask_ = 0;
    // Used to detect where signed ints are 8 or 16 bits
    u32 signed_8_bitmask_ = 0;
    u32 signed_16_bitmask_ = 0;
    // Count number of parameters the CPU will need to print out
    // This expands vectors and accounts for 64-bit parameters
    u32 expanded_parameter_count_ = 0;
};

}  // namespace spirv
}  // namespace gpu
