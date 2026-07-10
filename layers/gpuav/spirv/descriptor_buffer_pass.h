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
#pragma once

#include <stdint.h>
#include <vulkan/vulkan_core.h>
#include "type_manager.h"
#include "pass.h"

namespace gpuav {
namespace spirv {

class DescriptorBufferPass : public Pass {
  public:
    DescriptorBufferPass(Module& module);
    const char* Name() const final { return "DescriptorBufferPass"; }
    bool Instrument() final;
    void PrintDebugInfo() const final;

  private:
    struct InstructionMeta {
        const Instruction* target_instruction = nullptr;

        AccessPath access_path;
    };

    bool RequiresInstrumentation(const Function& function, const Instruction& inst, InstructionMeta& meta);
    uint32_t CreateFunctionCall(BasicBlock& block, InstructionIt* inst_it, const InstructionMeta& meta);

    // < original ID, new CopyObject ID >
    vvl::unordered_map<uint32_t, uint32_t> copy_object_map_;

    uint32_t link_function_id_ = 0;
    uint32_t GetLinkFunctionId();
};

}  // namespace spirv
}  // namespace gpuav