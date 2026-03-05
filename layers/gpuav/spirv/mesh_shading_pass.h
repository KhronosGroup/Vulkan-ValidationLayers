/* Copyright (c) 2025-2026 LunarG, Inc.
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

// Create a pass to instrument things dedicated to Mesh/Task shaders
class MeshShading : public Pass {
  public:
    MeshShading(Module& module);
    const char* Name() const final { return "MeshShading"; }
    bool Instrument() final;
    void PrintDebugInfo() const final;

  private:
    // This is metadata tied to a single instruction gathered during RequiresInstrumentation() to be used later
    struct InstructionMeta {
        const Instruction* target_instruction = nullptr;
        // linked function index
        uint32_t function_id;
    };

    bool RequiresInstrumentation(const Function& function, const Instruction& inst, InstructionMeta& meta);
    uint32_t CreateFunctionCall(BasicBlock& block, InstructionIt* inst_it, const InstructionMeta& meta);

    uint32_t GetLinkFunctionId(const InstructionMeta& meta);

    // Function IDs to link in
    enum FunctionNames {
        SET_MESH_OUTPUT = 0,
        TASK_PAYLOAD_ALWAYS = 1,
        FUNC_COUNT = 2,
    };
    uint32_t link_function_id_[FUNC_COUNT]{};

    // OpExecutionMode OutputVertices
    uint32_t output_vertices_id_ = 0;
    // OpExecutionModeId OutputPrimitivesEXT
    uint32_t output_primitives_id_ = 0;

    bool guard_all_task_payloads_ = false;
};

}  // namespace spirv
}  // namespace gpuav