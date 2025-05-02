/* Copyright (c) 2024-2025 LunarG, Inc.
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

#include "descriptor_class_general_buffer_pass.h"
#include "generated/spirv_grammar_helper.h"
#include "state_tracker/shader_instruction.h"
#include "utils/vk_layer_utils.h"
#include "module.h"
#include <spirv/unified1/spirv.hpp>
#include <iostream>
#include "generated/device_features.h"

#include "generated/gpuav_offline_spirv.h"
#include "gpuav/shaders/gpuav_shaders_constants.h"

namespace gpuav {
namespace spirv {

const static OfflineModule kOfflineModule = {instrumentation_descriptor_class_general_buffer_comp,
                                             instrumentation_descriptor_class_general_buffer_comp_size, UseErrorPayloadVariable};

const static OfflineFunction kOfflineFunction = {"inst_descriptor_class_general_buffer",
                                                 instrumentation_descriptor_class_general_buffer_comp_function_0_offset};

DescriptorClassGeneralBufferPass::DescriptorClassGeneralBufferPass(Module& module) : Pass(module, kOfflineModule) {
    module.use_bda_ = true;
}

// By appending the LinkInfo, it will attempt at linking stage to add the function.
uint32_t DescriptorClassGeneralBufferPass::GetLinkFunctionId() { return GetLinkFunction(link_function_id_, kOfflineFunction); }

void DescriptorClassGeneralBufferPass::CreateFunctionCall(BasicBlock& block, InstructionIt* inst_it, const InstructionMeta& meta) {
    assert(!meta.access_chain_insts.empty());
    const Constant& set_constant = module_.type_manager_.GetConstantUInt32(meta.descriptor_set);
    const uint32_t descriptor_index_id = CastToUint32(meta.descriptor_index_id, block, inst_it);  // might be int32

    const uint32_t descriptor_offset_id =
        GetLastByte(*meta.descriptor_type, meta.access_chain_insts, block, inst_it);  // Get Last Byte Index

    BindingLayout binding_layout = module_.set_index_to_bindings_layout_lut_[meta.descriptor_set][meta.descriptor_binding];
    const Constant& binding_layout_offset = module_.type_manager_.GetConstantUInt32(binding_layout.start);

    const uint32_t inst_position = meta.target_instruction->GetPositionIndex();
    const uint32_t inst_position_id = module_.type_manager_.CreateConstantUInt32(inst_position).Id();

    const uint32_t function_result = module_.TakeNextId();
    const uint32_t function_def = GetLinkFunctionId();
    const uint32_t void_type = module_.type_manager_.GetTypeVoid().Id();

    block.CreateInstruction(spv::OpFunctionCall,
                            {void_type, function_result, function_def, inst_position_id, set_constant.Id(), descriptor_index_id,
                             descriptor_offset_id, binding_layout_offset.Id()},
                            inst_it);

    module_.need_log_error_ = true;
}

bool DescriptorClassGeneralBufferPass::RequiresInstrumentation(const Function& function, const Instruction& inst,
                                                               InstructionMeta& meta) {
    const uint32_t opcode = inst.Opcode();

    if (!IsValueIn(spv::Op(opcode), {spv::OpLoad, spv::OpStore, spv::OpAtomicStore, spv::OpAtomicLoad, spv::OpAtomicExchange})) {
        return false;
    }

    const Instruction* next_access_chain = function.FindInstruction(inst.Operand(0));
    if (!next_access_chain || !next_access_chain->IsNonPtrAccessChain()) {
        return false;
    }

    const Variable* variable = nullptr;
    // We need to walk down possibly multiple chained OpAccessChains or OpCopyObject to get the variable
    while (next_access_chain && next_access_chain->IsNonPtrAccessChain()) {
        meta.access_chain_insts.push_back(next_access_chain);
        const uint32_t access_chain_base_id = next_access_chain->Operand(0);
        variable = module_.type_manager_.FindVariableById(access_chain_base_id);
        if (variable) {
            break;  // found
        }
        next_access_chain = function.FindInstruction(access_chain_base_id);
    }
    if (!variable) {
        return false;
    }

    uint32_t storage_class = variable->StorageClass();
    if (storage_class != spv::StorageClassUniform && storage_class != spv::StorageClassStorageBuffer) {
        return false;
    }

    meta.descriptor_type = variable->PointerType(module_.type_manager_);
    if (!meta.descriptor_type || meta.descriptor_type->spv_type_ == SpvType::kRuntimeArray) {
        return false;  // TODO - Currently we mark these as "bindless"
    }

    const bool is_descriptor_array = meta.descriptor_type->IsArray();
    meta.descriptor_id = is_descriptor_array ? meta.descriptor_type->inst_.Operand(0) : meta.descriptor_type->Id();

    // Check for deprecated storage block form
    if (storage_class == spv::StorageClassUniform) {
        assert(module_.type_manager_.FindTypeById(meta.descriptor_id)->spv_type_ == SpvType::kStruct && "unexpected block type");

        const bool block_found = GetDecoration(meta.descriptor_id, spv::DecorationBlock) != nullptr;

        // If block decoration not found, verify deprecated form of SSBO
        if (!block_found) {
            assert(GetDecoration(meta.descriptor_id, spv::DecorationBufferBlock) != nullptr && "block decoration not found");
            storage_class = spv::StorageClassStorageBuffer;
        }
    }

    // Grab front() as it will be the "final" type we access
    const Type* value_type = module_.type_manager_.FindValueTypeById(meta.access_chain_insts.front()->TypeId());
    if (!value_type) return false;

    if (is_descriptor_array) {
        // Because you can't have 2D array of descriptors, the first index of the last accessChain is the descriptor index
        meta.descriptor_index_id = meta.access_chain_insts.back()->Operand(1);
    } else {
        // There is no array of this descriptor, so we essentially have an array of 1
        meta.descriptor_index_id = module_.type_manager_.GetConstantZeroUint32().Id();
    }

    for (const auto& annotation : module_.annotations_) {
        if (annotation->Opcode() == spv::OpDecorate && annotation->Word(1) == variable->Id()) {
            if (annotation->Word(2) == spv::DecorationDescriptorSet) {
                meta.descriptor_set = annotation->Word(3);
            } else if (annotation->Word(2) == spv::DecorationBinding) {
                meta.descriptor_binding = annotation->Word(3);
            }
        }
    }

    if (meta.descriptor_set >= glsl::kDebugInputBindlessMaxDescSets) {
        module_.InternalWarning(Name(), "Tried to use a descriptor slot over the current max limit");
        return false;
    }

    if (!module_.settings_.safe_mode) {
        meta.access_offset = FindOffsetInStruct(meta.descriptor_id, is_descriptor_array, meta.access_chain_insts);
    }

    // Save information to be used to make the Function
    meta.target_instruction = &inst;

    return true;
}

void DescriptorClassGeneralBufferPass::PrintDebugInfo() const {
    std::cout << "DescriptorClassGeneralBufferPass instrumentation count: " << instrumentations_count_ << '\n';
}

// Created own Instrument() because need to control finding the largest offset in a given block
bool DescriptorClassGeneralBufferPass::Instrument() {
    if (module_.set_index_to_bindings_layout_lut_.empty()) {
        return false;  // If there is no bindings, nothing to instrument
    }

    // Can safely loop function list as there is no injecting of new Functions until linking time
    for (const auto& function : module_.functions_) {
        if (function->instrumentation_added_) continue;

        for (auto block_it = function->blocks_.begin(); block_it != function->blocks_.end(); ++block_it) {
            BasicBlock& current_block = **block_it;

            cf_.Update(current_block);
            if (debug_disable_loops_ && cf_.in_loop) continue;

            auto& block_instructions = current_block.instructions_;

            // < Descriptor SSA ID, Highest offset byte that will be accessed >
            vvl::unordered_map<uint32_t, uint32_t> block_highest_offset_map;

            if (!module_.settings_.safe_mode) {
                // Pre-pass loop the Block to get the highest offset accessed (statically known)
                // Do here before we inject instructions into the block list below
                for (auto inst_it = block_instructions.begin(); inst_it != block_instructions.end(); ++inst_it) {
                    InstructionMeta meta;
                    // Every instruction is analyzed by the specific pass and lets us know if we need to inject a function or not
                    if (!RequiresInstrumentation(*function, *(inst_it->get()), meta)) continue;

                    if (meta.access_offset != 0) {
                        // set offset for the first loop of the block
                        auto map_it = block_highest_offset_map.find(meta.descriptor_id);
                        if (map_it == block_highest_offset_map.end()) {
                            block_highest_offset_map[meta.descriptor_id] = meta.access_offset;
                        } else {
                            map_it->second = std::max(map_it->second, meta.access_offset);
                        }
                    }
                }
            }

            for (auto inst_it = block_instructions.begin(); inst_it != block_instructions.end(); ++inst_it) {
                InstructionMeta meta;
                // Every instruction is analyzed by the specific pass and lets us know if we need to inject a function or not
                if (!RequiresInstrumentation(*function, *(inst_it->get()), meta)) continue;

                if (!module_.settings_.safe_mode && meta.access_offset != 0) {
                    const uint32_t block_highest_offset = block_highest_offset_map[meta.descriptor_id];
                    if (meta.access_offset < block_highest_offset) {
                        continue;  // skipping because other instruction in block will be a higher offset
                    }
                }

                if (IsMaxInstrumentationsCount()) continue;
                instrumentations_count_++;

                // inst_it is updated to the instruction after the new function call, it will not add/remove any Blocks
                CreateFunctionCall(current_block, &inst_it, meta);
            }
        }
    }

    if (instrumentations_count_ > 75) {
        module_.InternalWarning(
            "GPUAV-Compile-time-general-buffer",
            "This shader will be very slow to compile and runtime performance may also be slow. This is due to the number of OOB "
            "checks for storage/uniform "
            "buffers. Turn on the |gpuav_force_on_robustness| setting to skip these checks and improve GPU-AV performance.");
    }

    return instrumentations_count_ != 0;
}

}  // namespace spirv
}  // namespace gpuav