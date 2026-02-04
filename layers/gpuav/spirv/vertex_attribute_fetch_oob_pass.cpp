/* Copyright (c) 2025-2026 The Khronos Group Inc.
 * Copyright (c) 2025-2026 Valve Corporation
 * Copyright (c) 2025-2026 LunarG, Inc.
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

#include "vertex_attribute_fetch_oob_pass.h"
#include "module.h"
#include "generated/gpuav_offline_spirv.h"

#include <iostream>

#include "profiling/profiling.h"

namespace gpuav {
namespace spirv {

const static OfflineModule kOfflineModule = {instrumentation_vertex_attribute_fetch_oob_vert,
                                             instrumentation_vertex_attribute_fetch_oob_vert_size};

const static OfflineFunction kOfflineFunction = {"inst_vertex_attribute_fetch_oob",
                                                 instrumentation_vertex_attribute_fetch_oob_vert_function_0_offset};

VertexAttributeFetchOobPass::VertexAttributeFetchOobPass(Module& module) : Pass(module, kOfflineModule) {}

uint32_t VertexAttributeFetchOobPass::GetLinkFunctionId() { return GetLinkFunction(link_function_id_, kOfflineFunction); }

bool VertexAttributeFetchOobPass::Instrument() {
    for (const auto& entry_point_inst : module_.entry_points_) {
        const uint32_t execution_model = entry_point_inst->Word(1);
        if (execution_model != spv::ExecutionModelVertex) {
            continue;
        } else if (module_.target_entry_point_id_ != entry_point_inst->Word(2)) {
            continue;  // If there are multiple vertex shaders in the module
        }

        // Handle edge case where there is no vertex input actually
        // the entry point must list all input variables
        {
            bool found_input = false;

            uint32_t word = entry_point_inst->GetEntryPointInterfaceStart();
            const uint32_t total_words = entry_point_inst->Length();
            for (; word < total_words; word++) {
                const uint32_t interface_id = entry_point_inst->Word(word);
                const Variable* variable = type_manager_.FindVariableById(interface_id);
                // guaranteed by spirv-val to be a OpVariable
                assert(variable);
                if (variable->StorageClass() == spv::StorageClassInput) {
                    found_input = true;
                    break;
                }
            }

            if (!found_input) {
                // vertex shader has no input, nothing to validate
                return false;
            }
        }

        for (Function& function : module_.functions_) {
            if (function.id_ != module_.target_entry_point_id_) {
                continue;
            }

            BasicBlock& first_block = function.GetFirstBlock();
            InstructionIt first_injectable_instruction = first_block.GetFirstInjectableInstrution();

            const uint32_t stage_info_id = GetStageInfo(function, first_block, first_injectable_instruction);

            InstructionIt stage_info_inst_it;
            for (auto inst_it = first_block.instructions_.begin(); inst_it != first_block.instructions_.end(); ++inst_it) {
                if ((*inst_it)->ResultId() == stage_info_id) {
                    stage_info_inst_it = inst_it;
                    break;
                }
            }
            ++stage_info_inst_it;

            std::vector<uint32_t> index_validation_inst_words;
            const uint32_t void_type = type_manager_.GetTypeVoid().Id();
            const uint32_t function_result = module_.TakeNextId();
            const uint32_t function_def = GetLinkFunctionId();

            first_block.CreateInstruction(spv::OpFunctionCall, {void_type, function_result, function_def, stage_info_id},
                                          &stage_info_inst_it);

            instrumentation_performed = true;
            return true;
        }
    }
    return false;
}

void VertexAttributeFetchOobPass::PrintDebugInfo() const {
    std::cout << "VertexAttributeFetchOobPass instrumentation performed: " << std::boolalpha << instrumentation_performed << '\n';
    VVL_TracyPlot(__FUNCTION__, int64_t(instrumentation_performed));
}

}  // namespace spirv

}  // namespace gpuav
