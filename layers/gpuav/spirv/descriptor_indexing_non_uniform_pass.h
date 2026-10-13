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
#include "pass.h"

namespace gpuav {
namespace spirv {

struct AccessPath;

// Create a pass to check that an index into a descriptor array is uniform within the subgroup,
// unless the access is decorated with NonUniform
class DescriptorIndexingNonUniformPass : public Pass {
  public:
    DescriptorIndexingNonUniformPass(Module& module);
    const char* Name() const final { return "DescriptorIndexingNonUniformPass"; }
    bool Instrument() final;
    void PrintDebugInfo() const final;

  private:
    // One descriptor array index to check, there are two for a separate image and sampler
    struct IndexCheck {
        const Variable* variable = nullptr;
        uint32_t index_id = 0;
        // Capability the shader is missing for this descriptor type, zero if only the NonUniform decoration is missing
        uint32_t missing_capability = 0;
    };

    // This is metadata tied to a single instruction gathered during RequiresInstrumentation() to be used later
    struct InstructionMeta {
        const Instruction* target_instruction = nullptr;
        IndexCheck checks[2];
        uint32_t check_count = 0;
    };

    bool RequiresInstrumentation(const Function& function, const Instruction& inst, InstructionMeta& meta);
    void CreateFunctionCall(BasicBlock& block, InstructionIt* inst_it, const Instruction& target_instruction,
                            const IndexCheck& check);
    void AddIndexCheck(InstructionMeta& meta, const Variable* variable, const Type* pointer_type, uint32_t index_id,
                       VkDescriptorType descriptor_type, bool is_decorated) const;
    bool IsDecoratedNonUniform(const Function& function, uint32_t id, bool follow_sampled_image_sampler) const;

    vvl::unordered_set<uint32_t> non_uniform_ids_;
    bool has_descriptor_heap_capability_ = false;

    uint32_t link_function_id_ = 0;
};

}  // namespace spirv
}  // namespace gpuav
