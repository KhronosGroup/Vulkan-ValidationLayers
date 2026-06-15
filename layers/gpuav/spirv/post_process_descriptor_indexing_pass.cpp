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

#include "post_process_descriptor_indexing_pass.h"

#include "module.h"
#include "generated/gpuav_offline_spirv.h"
#include "gpuav/shaders/gpuav_shaders_constants.h"
#include "type_manager.h"
#include "utils/hash_util.h"

#include <iostream>

namespace gpuav {
namespace spirv {

const static OfflineModule kOfflineModule = {instrumentation_post_process_descriptor_index_comp,
                                             instrumentation_post_process_descriptor_index_comp_size};

const static OfflineFunction kOfflineFunction = {"inst_post_process_descriptor_index",
                                                 instrumentation_post_process_descriptor_index_comp_function_0_offset};

PostProcessDescriptorIndexingPass::PostProcessDescriptorIndexingPass(Module& module) : Pass(module, kOfflineModule) {
    module.use_bda_ = true;
}

// By appending the LinkInfo, it will attempt at linking stage to add the function.
uint32_t PostProcessDescriptorIndexingPass::GetLinkFunctionId() { return GetLinkFunction(link_function_id_, kOfflineFunction); }

void PostProcessDescriptorIndexingPass::CreateFunctionCall(BasicBlock& block, InstructionIt* inst_it, const InstructionMeta& meta) {
    const DescriptorInterface& interface = meta.access_path.variable->interface_;
    const Constant& set_constant = type_manager_.GetConstantUInt32(interface.set);
    const Constant& binding_constant = type_manager_.GetConstantUInt32(interface.binding);
    const uint32_t descriptor_index_id = CastToUint32(meta.access_path.descriptor_index_id, block, inst_it);  // might be int32

    const auto& layout_lut = module_.interface_.instrumentation_dsl.set_index_to_bindings_layout_lut;
    BindingLayout binding_layout = layout_lut[interface.set][interface.binding];
    const Constant& binding_layout_offset = type_manager_.GetConstantUInt32(binding_layout.start);
    const Constant& variable_id_constant = type_manager_.GetConstantUInt32(meta.access_path.variable->Id());

    const uint32_t inst_position = meta.target_instruction->GetPositionOffset();
    const uint32_t inst_position_id = type_manager_.CreateConstantUInt32(inst_position).Id();

    const uint32_t function_result = module_.TakeNextId();
    const uint32_t function_def = GetLinkFunctionId();
    const uint32_t void_type = type_manager_.GetTypeVoid().Id();

    block.CreateInstruction(spv::OpFunctionCall,
                            {void_type, function_result, function_def, set_constant.Id(), binding_constant.Id(),
                             descriptor_index_id, binding_layout_offset.Id(), variable_id_constant.Id(), inst_position_id},
                            inst_it);
}

bool PostProcessDescriptorIndexingPass::RequiresInstrumentation(const Function& function, const Instruction& inst,
                                                                InstructionMeta& meta) {
    meta.access_path = type_manager_.BuildAccessPath(function, inst);
    if (!meta.access_path.IsValid() || !meta.access_path.variable->IsDescriptor()) {
        return false;
    }

    if (meta.access_path.variable->interface_.set >= glsl::kDebugInputBindlessMaxDescSets) {
        module_.InternalWarning(Name(), "Tried to use a descriptor slot over the current max limit");
        return false;
    }

    meta.target_instruction = &inst;

    return true;
}

bool PostProcessDescriptorIndexingPass::Instrument() {
    if (module_.interface_.instrumentation_dsl.set_index_to_bindings_layout_lut.empty()) {
        return false;  // If there is no bindings, nothing to instrument
    }

    for (Function& function : module_.functions_) {
        if (!function.called_from_target_) {
            continue;
        }

        FunctionDuplicateTracker function_duplicate_tracker;

        for (auto block_it = function.blocks_.begin(); block_it != function.blocks_.end(); ++block_it) {
            BasicBlock& current_block = **block_it;

            cf_.Update(current_block);
            if (debug_disable_loops_ && cf_.in_loop) {
                continue;
            }

            auto& block_instructions = current_block.instructions_;

            // We only need to instrument the set/binding/index/variable combo once per block
            BlockDuplicateTracker& block_duplicate_tracker = function_duplicate_tracker.GetAndUpdate(current_block);
            DescriptroIndexPushConstantAccess pc_access;

            for (auto inst_it = block_instructions.begin(); inst_it != block_instructions.end(); ++inst_it) {
                pc_access.Update(module_, inst_it);

                InstructionMeta meta;
                if (!RequiresInstrumentation(function, *(inst_it->get()), meta)) {
                    continue;
                }

                const uint32_t hash_descriptor_index_id = pc_access.next_alias_id == meta.access_path.descriptor_index_id
                                                              ? pc_access.descriptor_index_id
                                                              : meta.access_path.descriptor_index_id;
                uint32_t hash_content[4] = {meta.access_path.variable->interface_.set,
                                            meta.access_path.variable->interface_.binding, hash_descriptor_index_id,
                                            meta.access_path.variable->Id()};
                const uint32_t hash = hash_util::Hash32(hash_content, sizeof(uint32_t) * 4);
                if (function_duplicate_tracker.FindAndUpdate(block_duplicate_tracker, hash)) {
                    continue;  // duplicate detected
                }

                if (MaxInstrumentationsCountReached()) {
                    return instrumentations_count_ != 0;
                }
                instrumentations_count_++;

                CreateFunctionCall(current_block, &inst_it, meta);
            }
        }
    }

    return (instrumentations_count_ != 0);
}

void PostProcessDescriptorIndexingPass::PrintDebugInfo() const {
    std::cout << "PostProcessDescriptorIndexingPass instrumentation count: " << instrumentations_count_ << '\n';
}

}  // namespace spirv
}  // namespace gpuav
