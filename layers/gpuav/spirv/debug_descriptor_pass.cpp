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

#include "debug_descriptor_pass.h"
#include "module.h"
#include <cstdint>
#include <cstring>
#include <iostream>
#include <spirv/unified1/spirv.hpp>

namespace gpuav {
namespace spirv {

// Replace when we have spirv header
[[maybe_unused]] static const uint32_t debug_descriptor_encoding = 1;
[[maybe_unused]] static const uint32_t debug_descriptor_all_encoding = 2;
[[maybe_unused]] static const uint32_t debug_options_none = 0;
[[maybe_unused]] static const uint32_t debug_options_return_early = 1;
[[maybe_unused]] static const uint32_t debug_options_shader_abort = 2;
[[maybe_unused]] static const uint32_t debug_options_use_null_value = 3;

bool DebugDescriptorPass::RequiresInstrumentation(const Instruction& inst, InstructionMeta& meta) {
    if (inst.Opcode() == spv::OpExtInst && inst.Word(3) == ext_import_id_) {
        meta.target_instruction = &inst;
        meta.dump_all = inst.Word(4) == debug_descriptor_all_encoding;
        return true;
    }
    return false;
}

void DebugDescriptorPass::CreateFunctionCall(BasicBlock& block, InstructionIt* inst_it, const InstructionMeta& meta) {
    (void)block;
    (void)inst_it;
    (void)meta;

    uint32_t slot = binding_slot_;
    (void)slot;  // to silent warning until used
}

bool DebugDescriptorPass::Instrument() {
    if (module_.interface_.descriptor_mode != vvl::DescriptorModeBuffer &&
        module_.interface_.descriptor_mode != vvl::DescriptorModeHeap) {
        return false;
    }

    for (const auto& inst : module_.ext_inst_imports_) {
        const char* import_string = inst->GetAsString(2);
        if (strcmp(import_string, "NonSemantic.DebugDescriptor") == 0) {
            ext_import_id_ = inst->ResultId();
            break;
        }
    }

    if (ext_import_id_ == 0) {
        return false;  // no debug descriptor strings found, early return
    }

    for (Function& function : module_.functions_) {
        if (!function.called_from_target_) {
            continue;
        }

        for (auto block_it = function.blocks_.begin(); block_it != function.blocks_.end(); ++block_it) {
            BasicBlock& current_block = **block_it;

            cf_.Update(current_block);
            if (debug_disable_loops_ && cf_.in_loop) {
                continue;
            }

            auto& block_instructions = current_block.instructions_;

            for (auto inst_it = block_instructions.begin(); inst_it != block_instructions.end(); ++inst_it) {
                InstructionMeta meta;
                if (!RequiresInstrumentation(*(inst_it->get()), meta)) {
                    continue;
                }

                instrumentations_count_++;

                CreateFunctionCall(current_block, &inst_it, meta);
            }
        }
    }

    return instrumentations_count_ != 0;
}

void DebugDescriptorPass::PrintDebugInfo() const {
    std::cout << "DebugDescriptorPass instrumentation count: " << instrumentations_count_ << '\n';
}

bool DebugDescriptorPass::Validate(const Function& current_function, const InstructionMeta& meta) {
    // TODO - Decide if there is stuff we want to validate here or not
    (void)current_function;
    (void)meta;
    return true;
}

}  // namespace spirv
}  // namespace gpuav
