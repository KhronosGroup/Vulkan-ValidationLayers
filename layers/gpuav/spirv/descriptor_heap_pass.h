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

#include "vulkan/vulkan_core.h"

namespace gpuav {
namespace spirv {

// Will make sure Descriptor Heap accesses are valid
class DescriptorHeapPass : public Pass {
  public:
    DescriptorHeapPass(Module& module, VkPhysicalDeviceDescriptorHeapPropertiesEXT heap_props,
                       VkPhysicalDeviceDescriptorHeapTensorPropertiesARM heap_tensor_props);
    const char* Name() const final { return "DescriptorHeapPass"; }

    bool Instrument() final;
    void PrintDebugInfo() const final;

  private:
    VkPhysicalDeviceDescriptorHeapPropertiesEXT heap_props_{};
    VkPhysicalDeviceDescriptorHeapTensorPropertiesARM heap_tensor_props_{};

    // This is metadata tied to a single instruction gathered during RequiresInstrumentation() to be used later
    struct InstructionMeta {
        const Instruction* target_instruction = nullptr;

        uint32_t variable_id = 0;
        const Type* descriptor_type = nullptr;
        CooperativeMatrixAccess coop_mat_access{};
        std::vector<const Instruction*> access_chain_insts;

        bool post_process = false;
        bool buffer_pointer = false;
        bool sampler_pointer = false;
        bool image_pointer = false;
        bool image_texel_pointer = false;
        bool acceleration_structure_pointer = false;
        bool tensor_pointer = false;

        uint32_t offset_id = 0;
        uint32_t array_stride_id = 0;
        uint32_t storage_class = 0;
        uint32_t image_format = 0;
        uint32_t image_dim = 0;
        uint32_t image_arrayed = 0;
        uint32_t image_multisampled = 0;
        uint32_t image_sampled = 0;
    };

    bool HeapPointerRequiresInstrumentation(const Function& function, const Instruction& inst, InstructionMeta& meta);
    bool RequiresInstrumentation(const Function& function, const Instruction& inst, InstructionMeta& meta);
    void CreateFunctionCall(BasicBlock& block, InstructionIt* inst_it, const InstructionMeta& meta);

    // Function IDs to link in
    uint32_t link_function_id_ = 0;

    uint32_t slot_index_ = 0;
};

}  // namespace spirv
}  // namespace gpuav