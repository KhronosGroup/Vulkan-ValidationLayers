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
#include "pass.h"

struct VkDescriptorSetAndBindingMappingEXT;

namespace gpuav {
namespace spirv {

class DescriptorHeapPass : public Pass {
  public:
    DescriptorHeapPass(Module& module);
    const char* Name() const final { return "DescriptorHeapPass"; }
    bool Instrument() final;
    void PrintDebugInfo() const final;

  private:
    struct DescriptorMeta {
        const Instruction* var_inst = nullptr;
        uint32_t descriptor_set = 0;
        uint32_t descriptor_binding = 0;
        uint32_t descriptor_index_id = 0;
    };

    struct InstructionMeta {
        const Instruction* target_instruction = nullptr;

        DescriptorMeta resource;
        DescriptorMeta sampler;

        bool is_combined_image_sampler = false;

        // Used to move OpSampledImage into block we access it
        const Instruction* image_inst = nullptr;

        bool HasSampler() const { return sampler.var_inst != nullptr; }
    };

    bool RequiresInstrumentation(const Function& function, const Instruction& inst, InstructionMeta& meta);
    uint32_t CreateFunctionCall(BasicBlock& block, InstructionIt* inst_it, const InstructionMeta& meta, bool is_sampler);

    const VkDescriptorSetAndBindingMappingEXT* GetMapping(uint32_t descriptor_set, uint32_t descriptor_binding);

    // < original ID, new CopyObject ID >
    vvl::unordered_map<uint32_t, uint32_t> copy_object_map_;

    // Function IDs to link in
    enum FunctionNames {
        MAPPING_CONSTANT_OFFSET = 0,
        MAPPING_PUSH_INDEX = 1,
        MAPPING_INDIRECT_INDEX = 2,
        MAPPING_INDIRECT_INDEX_ARRAY = 3,
        MAPPING_RESOURCE_HEAP_DATA = 4,
        MAPPING_PUSH_DATA = 5,
        MAPPING_PUSH_ADDRESS = 6,
        MAPPING_INDIRECT_ADDRESS = 7,
        FUNC_COUNT = 8,
    };
    uint32_t link_function_id_[FUNC_COUNT]{};
    uint32_t GetLinkFunctionId(const FunctionNames func_name);
};

}  // namespace spirv
}  // namespace gpuav