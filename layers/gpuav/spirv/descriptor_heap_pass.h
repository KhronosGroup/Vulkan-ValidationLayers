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
    struct UntypedLayout {
        // If zero, means it is implicitly zero
        uint32_t offset_id = 0;
        uint32_t array_stride_id = 0;
        // Temp value until we properly support MultidimensionalArray
        bool is_multidimensional_array = false;
    };

    struct InstructionMeta {
        const Instruction* target_instruction = nullptr;

        AccessPath access_path;

        // Internal encoded index to map into InstrumentationStatus::Device
        uint32_t mapping_index_resource = 0;
        uint32_t mapping_index_sampler = 0;

        // This is information we "could" find at CreateFunctionCall but want done earlier to so we can hash it
        // ----
        // Null if untyped pointers
        const VkDescriptorSetAndBindingMappingEXT* mapping_ptr = nullptr;
        const VkDescriptorSetAndBindingMappingEXT* mapping_ptr_sampler = nullptr;
        UntypedLayout untyped_layout_resource;
        UntypedLayout untyped_layout_sampler;
        uint32_t Hash(const uint32_t descriptor_index, VkDescriptorType vk_type) const;

        bool instrument_seperate_sampler = false;
    };

    bool RequiresInstrumentation(const Function& function, const Instruction& inst, InstructionMeta& meta);
    uint32_t CreateFunctionCall(BasicBlock& block, InstructionIt* inst_it, const InstructionMeta& meta, bool is_seperate_sampler);
    uint32_t CreateFunctionCallSampler(BasicBlock& block, InstructionIt* inst_it, const InstructionMeta& meta,
                                       uint32_t non_sampler_result);
    uint32_t CreateFunctionCallCombinedSampler(BasicBlock& block, InstructionIt* inst_it, const InstructionMeta& meta,
                                               const VkDescriptorSetAndBindingMappingEXT& mapping,
                                               const uint32_t descriptor_index_id);

    bool ResourceTypeMatchesBinding(VkSpirvResourceTypeFlagsEXT resource_type, const AccessPath& access_path,
                                    bool is_sampler) const;
    uint32_t GetMapping(const AccessPath& access_path, bool is_sampler) const;
    uint32_t GetMinBufferAlignment(const InstructionMeta& meta) const;

    UntypedLayout GetUntypedLayout(const Type& pointer_type, uint32_t heap_offset_member_index);

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
        UNTYPED = 8,
        DESCRIPTOR_HASHING = 9,
        FUNC_COUNT = 10,
    };
    uint32_t link_function_id_[FUNC_COUNT]{};
    uint32_t GetLinkFunctionId(const FunctionNames func_name);

    const VkPhysicalDeviceDescriptorHeapPropertiesEXT& descriptor_heap_props;
};

}  // namespace spirv
}  // namespace gpuav