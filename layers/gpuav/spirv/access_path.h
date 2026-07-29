/* Copyright (c) 2024-2026 LunarG, Inc.
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

#include <vulkan/vulkan.h>
#include <cstdint>
#include <vector>
#include <memory>
#include "cooperative_matrix.h"
#include "state_tracker/shader_instruction.h"

namespace gpuav {
namespace spirv {
using Instruction = ::spirv::Instruction;

struct Variable;
class Module;
class TypeManager;
struct Function;

// We often want to walk the SSA from an "access" (load, store, atomic, etc) to the Variable it is referencing. There can be a
// single OpAccessChain or multiple, and this struct holds this information.
// Background info: https://github.com/KhronosGroup/SPIRV-Guide/blob/main/chapters/access_chains.md
//
// Note - currently this is very heavily leaned towards use of descriptors, but will work for any variable type
struct AccessPath {
    AccessPath(const Module& module, TypeManager& type_manager_, const Function& function, const Instruction& inst);

    bool is_valid = false;
    bool IsValid(spv::StorageClass storage_class) const;
    bool IsValidBda() const;
    bool IsValidDescriptor() const;

    // The type of the access itself (what type it will store or load)
    const Type* access_type = nullptr;

    // This the %ptr_type in
    //   %ptr_type = OpTypeArray
    //   %ptr = OpTypePointer StorageBuffer %ptr_type
    //   %var = OpVariable %ptr StorageBuffer
    const Type* pointer_type = nullptr;

    // The variable at the end of the access chain
    const Variable* variable = nullptr;

    // List of OpAccessChains from the variable to the "access"
    // - The front() will be closest to the OpVariable
    // - The back() will be closest to the exact spot accesssed
    // This is on purpose as we really will want to loop the OpAccessChain in reserve SSA order
    //
    // Note: GLSL will try to always create a single large OpAccessChain
    std::vector<const Instruction*> ac_list;

    struct Descriptor {
        // Optional variable of seperate sampler descriptor (still null if combinedImageSampler)
        const Variable* sampler_variable = nullptr;
        const Type* sampler_pointer_type = nullptr;
        bool is_combined_image_sampler = false;
        bool HasSampler() const { return sampler_variable != nullptr || is_combined_image_sampler; }

        // The OpLoad to access an image descriptor
        const Instruction* image_load_inst = nullptr;

        // Index into the descriptor array
        // Most access paths are used to get the descriptor variable.
        // This is the ID of the uint that indexes in the array (or constant zero if no array)
        uint32_t index_id = 0;
        // Optional index if there is a seperate sampler as well
        uint32_t sampler_index_id = 0;

        // TODO - Need to handle OffsetIdEXT correctly, this is a dumb hack
        uint32_t heap_offset_member_index = 0;
        uint32_t sampler_heap_offset_member_index = 0;

        VkDescriptorType type = VK_DESCRIPTOR_TYPE_MAX_ENUM;
    } descriptor;

    bool is_bda = false;
    uint32_t bda_alignment = 0;

    CooperativeMatrixAccess coop_mat{};
};

}  // namespace spirv
}  // namespace gpuav
