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

#include "descriptor_buffer_pass.h"
#include <vulkan/vulkan_core.h>
#include "link.h"
#include "module.h"
#include <cassert>
#include <cstdint>
#include <spirv/unified1/spirv.hpp>
#include <iostream>

#include "generated/gpuav_offline_spirv.h"
#include "pass.h"
#include "type_manager.h"
#include "utils/descriptor_utils.h"
#include "utils/hash_util.h"

namespace gpuav {
namespace spirv {

const static OfflineModule kOfflineModule = {instrumentation_descriptor_buffer_comp, instrumentation_descriptor_buffer_comp_size,
                                             UseErrorPayloadVariable};

const static OfflineFunction kOfflineFunction = {"inst_buffer_descriptor_hashing",
                                                 instrumentation_descriptor_buffer_comp_function_0_offset};

DescriptorBufferPass::DescriptorBufferPass(Module& module) : Pass(module, kOfflineModule) { module.use_bda_ = true; }

uint32_t DescriptorBufferPass::GetLinkFunctionId() { return GetLinkFunction(link_function_id_, kOfflineFunction); }

uint32_t DescriptorBufferPass::CreateFunctionCall(BasicBlock& block, InstructionIt* inst_it, const InstructionMeta& meta) {
    const DescriptorInterface& interface = meta.access_path.variable->interface_;
    const uint32_t desc_set_id = type_manager_.GetConstantUInt32(interface.set).Id();
    const uint32_t desc_binding_id = type_manager_.GetConstantUInt32(interface.binding).Id();
    const uint32_t desc_index_id = CastToUint32(meta.access_path.descriptor_index_id, block, inst_it);  // might be int32

    const uint32_t inst_position = meta.target_instruction->GetPositionOffset();
    const uint32_t inst_position_id = type_manager_.CreateConstantUInt32(inst_position).Id();

    const uint32_t function_result = module_.TakeNextId();
    const uint32_t function_def = GetLinkFunctionId();
    const uint32_t bool_type = type_manager_.GetTypeBool().Id();

    const VkDescriptorType vk_desc_type = meta.access_path.descriptor_type;
    uint8_t desc_type_mask = (uint8_t)GetMaskFromDescriptorType(vk_desc_type);
    const uint32_t desc_type_id = type_manager_.GetConstantUInt32(desc_type_mask).Id();

    const uint32_t desc_size_value = (uint32_t)module_.settings_.cached_descriptor_size->GetSize(vk_desc_type, false);
    const uint32_t desc_size_id = type_manager_.GetConstantUInt32(desc_size_value).Id();

    block.CreateInstruction(spv::OpFunctionCall,
                            {bool_type, function_result, function_def, inst_position_id, desc_set_id, desc_binding_id,
                             desc_index_id, desc_type_id, desc_size_id},
                            inst_it);

    module_.need_log_error_ = true;

    return function_result;
}

bool DescriptorBufferPass::RequiresInstrumentation(const Function& function, const Instruction& inst, InstructionMeta& meta) {
    meta.access_path = type_manager_.BuildAccessPath(function, inst);
    if (!meta.access_path.IsValid() || !meta.access_path.variable->IsDescriptor()) {
        return false;
    }
    if (meta.access_path.descriptor_type == VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR) {
        return false;  // not supported yet
    }
    if (meta.access_path.descriptor_type == VK_DESCRIPTOR_TYPE_SAMPLER ||
        meta.access_path.descriptor_type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
        // Need to be cautious of Embedded Samplers as well when adding support
        return false;  // not supported yet
    }

    meta.target_instruction = &inst;

    return true;
}

bool DescriptorBufferPass::Instrument() {
    if (!module_.settings_.descriptor_hashing) {
        return false;
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

            if (current_block.IsLoopHeader()) {
                continue;  // Currently can't properly handle injecting CFG logic into a loop header block
            }
            auto& block_instructions = current_block.instructions_;

            // We only need to instrument the set/binding/index combo once per block (in unsafe mode)
            BlockDuplicateTracker& block_duplicate_tracker = function_duplicate_tracker.GetAndUpdate(current_block);
            DescriptroIndexPushConstantAccess pc_access;

            for (auto inst_it = block_instructions.begin(); inst_it != block_instructions.end(); ++inst_it) {
                if (!module_.settings_.safe_mode) {
                    pc_access.Update(module_, inst_it);
                }

                InstructionMeta meta;
                // Every instruction is analyzed by the specific pass and lets us know if we need to inject a function or not
                if (!RequiresInstrumentation(function, *(inst_it->get()), meta)) {
                    continue;
                }

                if (!module_.settings_.safe_mode) {
                    const uint32_t hash_descriptor_index_id = pc_access.next_alias_id == meta.access_path.descriptor_index_id
                                                                  ? pc_access.descriptor_index_id
                                                                  : meta.access_path.descriptor_index_id;
                    uint32_t hash_content[3] = {meta.access_path.variable->interface_.set,
                                                meta.access_path.variable->interface_.binding, hash_descriptor_index_id};
                    const uint32_t hash = hash_util::Hash32(hash_content, sizeof(uint32_t) * 3);
                    if (function_duplicate_tracker.FindAndUpdate(block_duplicate_tracker, hash)) {
                        continue;  // duplicate detected
                    }
                }

                if (MaxInstrumentationsCountReached()) {
                    return instrumentations_count_ != 0;
                }
                instrumentations_count_++;

                if (!module_.settings_.safe_mode) {
                    CreateFunctionCall(current_block, &inst_it, meta);
                } else {
                    InjectConditionalData ic_data = InjectFunctionPre(function, block_it, inst_it);
                    ic_data.function_result_id = CreateFunctionCall(current_block, nullptr, meta);
                    InjectFunctionPost(current_block, ic_data);
                    // Skip the newly added valid and invalid block. Start searching again from newly split merge block
                    block_it++;
                    block_it++;
                    break;
                }
            }
        }
    }

    return instrumentations_count_ != 0;
}

void DescriptorBufferPass::PrintDebugInfo() const {
    std::cout << "DescriptorBufferPass instrumentation count: " << instrumentations_count_ << "\n";
}

}  // namespace spirv
}  // namespace gpuav