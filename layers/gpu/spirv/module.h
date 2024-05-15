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
#include <vector>
#include <memory>
#include "link.h"
#include "instruction.h"
#include "function_basic_block.h"
#include "type_manager.h"

namespace gpuav {
namespace spirv {

struct ModuleHeader {
    uint32_t magic_number;
    uint32_t version;
    uint32_t generator;
    uint32_t bound;
    uint32_t schema;
};

// This is the "brain" of SPIR-V logic, it stores the memory of all the Instructions and is the main context.
// There are other helper classes that are charge of handling the various parts of the module.
class Module {
  public:
    Module(std::vector<uint32_t> words, uint32_t shader_id, uint32_t output_buffer_descriptor_set);

    // Memory that holds all the actual SPIR-V data, replicate the "Logical Layout of a Module" of SPIR-V.
    // Divided into sections to make easier to modify each part at different times, but still keeps it simple to write out all the
    // instructions to a binary format.
    ModuleHeader header_;
    InstructionList capabilities_;
    InstructionList extensions_;
    InstructionList ext_inst_imports_;
    InstructionList memory_model_;
    InstructionList entry_points_;
    InstructionList execution_modes_;
    InstructionList debug_source_;
    InstructionList debug_name_;
    InstructionList debug_module_processed_;
    InstructionList annotations_;
    InstructionList types_values_constants_;
    FunctionList functions_;

    // Handles all types and constants
    TypeManager type_manager_;

    // When adding a new instruction with result ID, will need to grab the next ID
    uint32_t TakeNextId();

    // Order of functions that will try to be linked in
    std::vector<LinkInfo> link_info_;
    void LinkFunction(const LinkInfo& info);

    // The class is designed to be written out to a binary file.
    void ToBinary(std::vector<uint32_t>& out);

    // Passes that can be ran
    void RunPassBindlessDescriptor();
    void RunPassBufferDeviceAddress();
    void RunPassRayQuery();

    // Helpers
    bool HasCapability(spv::Capability capability);
    void AddCapability(spv::Capability capability);

  private:
    // provides a way to map back and know which original SPIR-V this was from
    const uint32_t shader_id_;
    // Will replace the "OpDecorate DescriptorSet" for the output buffer in the incoming linked module
    // This allows anything to be set in the GLSL for the set value, as we change it at runtime
    const uint32_t output_buffer_descriptor_set_;
};

}  // namespace spirv
}  // namespace gpuav