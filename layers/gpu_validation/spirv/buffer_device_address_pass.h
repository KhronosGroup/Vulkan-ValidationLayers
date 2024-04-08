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

namespace gpuav {
namespace spirv {

class Module;
struct Function;
struct BasicBlock;

// Create a pass to instrument physical buffer address checking
// This pass instruments all physical buffer address references to check that
// all referenced bytes fall in a valid buffer. If the reference is
// invalid, a record is written to the debug output buffer (if space allows)
// and a null value is returned.
//
// For OpStore we will just ignore the store if it is invalid, example:
// Before:
//     bda.data[index] = value;
// After:
//    if (isValid(bda.data, index)) {
//         bda.data[index] = value;
//    }
//
// For OpLoad we replace the value with Zero (via Phi node) if it is invalid, example
// Before:
//     int X = bda.data[index];
//     int Y = bda.data[X];
// After:
//    if (isValid(bda.data, index)) {
//         int X = bda.data[index];
//    } else {
//         int X = 0;
//    }
//    if (isValid(bda.data, X)) {
//         int Y = bda.data[X];
//    } else {
//         int Y = 0;
//    }
class BufferDeviceAddressPass : public Pass {
  public:
    BufferDeviceAddressPass(Module& module) : Pass(module) {}

  private:
    bool AnalyzeInstruction(const Function& function, const Instruction& inst) final;
    uint32_t CreateFunctionCall(BasicBlock& block) final;
    void Reset() final;

    uint32_t link_function_id = 0;
    uint32_t GetLinkFunctionId();

    const Instruction* target_instruction_ = nullptr;
    uint32_t type_length_ = 0;
    uint32_t access_opcode_ = 0;
};

}  // namespace spirv
}  // namespace gpuav