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

#include "non_bindless_oob_buffer_pass.h"
#include "module.h"
#include <spirv/unified1/spirv.hpp>
#include <iostream>

#include "generated/instrumentation_non_bindless_oob_buffer_comp.h"
#include "gpu/shaders/gpu_shaders_constants.h"

namespace gpu {
namespace spirv {

NonBindlessOOBBufferPass::NonBindlessOOBBufferPass(Module& module) : Pass(module) { module.use_bda_ = true; }

static LinkInfo link_info = {instrumentation_non_bindless_oob_buffer_comp, instrumentation_non_bindless_oob_buffer_comp_size,
                             LinkFunctions::inst_non_bindless_oob_buffer, 0, "inst_non_bindless_oob_buffer"};

// By appending the LinkInfo, it will attempt at linking stage to add the function.
uint32_t NonBindlessOOBBufferPass::GetLinkFunctionId() {
    if (link_function_id == 0) {
        link_function_id = module_.TakeNextId();
        link_info.function_id = link_function_id;
        module_.link_info_.push_back(link_info);
    }
    return link_function_id;
}

uint32_t NonBindlessOOBBufferPass::CreateFunctionCall(BasicBlock& block, InstructionIt* inst_it,
                                                      const InjectionData& injection_data) {
    assert(access_chain_inst_ && var_inst_);
    const Constant& set_constant = module_.type_manager_.GetConstantUInt32(descriptor_set_);
    const Constant& binding_constant = module_.type_manager_.GetConstantUInt32(descriptor_binding_);
    const uint32_t descriptor_index_id = CastToUint32(descriptor_index_id_, block, inst_it);  // might be int32

    // For now, only do bounds check for non-aggregate types
    // TODO - Do bounds check for aggregate loads and stores
    const Type* pointer_type = module_.type_manager_.FindTypeById(access_chain_inst_->TypeId());
    const Type* pointee_type = module_.type_manager_.FindTypeById(pointer_type->inst_.Word(3));
    if (pointee_type && pointee_type->spv_type_ != SpvType::kArray && pointee_type->spv_type_ != SpvType::kRuntimeArray &&
        pointee_type->spv_type_ != SpvType::kStruct) {
        descriptor_offset_id_ = GetLastByte(*var_inst_, *access_chain_inst_, block, inst_it);  // Get Last Byte Index
    } else {
        descriptor_offset_id_ = module_.type_manager_.GetConstantZeroUint32().Id();
    }

    const uint32_t function_result = module_.TakeNextId();
    const uint32_t function_def = GetLinkFunctionId();
    const uint32_t bool_type = module_.type_manager_.GetTypeBool().Id();

    block.CreateInstruction(
        spv::OpFunctionCall,
        {bool_type, function_result, function_def, injection_data.inst_position_id, injection_data.stage_info_id,
         descriptor_array_size_id_, set_constant.Id(), binding_constant.Id(), descriptor_index_id, descriptor_offset_id_},
        inst_it);

    return function_result;
}

void NonBindlessOOBBufferPass::Reset() {
    access_chain_inst_ = nullptr;
    var_inst_ = nullptr;
    target_instruction_ = nullptr;
    descriptor_array_size_id_ = 0;
    descriptor_set_ = 0;
    descriptor_binding_ = 0;
    descriptor_index_id_ = 0;
    descriptor_offset_id_ = 0;
}

bool NonBindlessOOBBufferPass::AnalyzeInstruction(const Function& function, const Instruction& inst) {
    const uint32_t opcode = inst.Opcode();

    if (opcode != spv::OpLoad && opcode != spv::OpStore) {
        return false;
    }

    // TODO - Should have loop to walk Load/Store to the Pointer,
    // this case will not cover things such as OpCopyObject or double OpAccessChains
    access_chain_inst_ = function.FindInstruction(inst.Operand(0));
    if (!access_chain_inst_ || access_chain_inst_->Opcode() != spv::OpAccessChain) {
        return false;
    }

    const uint32_t variable_id = access_chain_inst_->Operand(0);
    const Variable* variable = module_.type_manager_.FindVariableById(variable_id);
    if (!variable) {
        return false;
    }
    var_inst_ = &variable->inst_;

    uint32_t storage_class = variable->StorageClass();
    if (storage_class != spv::StorageClassUniform && storage_class != spv::StorageClassStorageBuffer) {
        return false;
    }

    const Type* pointer_type = variable->PointerType(module_.type_manager_);
    if (pointer_type->inst_.Opcode() == spv::OpTypeRuntimeArray) {
        return false;  // Currently we mark these as "bindless"
    }

    const bool is_descriptor_array = pointer_type->inst_.Opcode() == spv::OpTypeArray;
    if (is_descriptor_array) {
        const Constant* array_size_const = module_.type_manager_.FindConstantById(pointer_type->inst_.Operand(1));
        if (!array_size_const) {
            return false;  // TODO - Handle Spec Constants here
        }
        descriptor_array_size_id_ = array_size_const->Id();
    } else {
        descriptor_array_size_id_ = module_.type_manager_.GetConstantUInt32(1).Id();
    }

    // Check for deprecated storage block form
    if (storage_class == spv::StorageClassUniform) {
        const uint32_t block_type_id = is_descriptor_array ? pointer_type->inst_.Operand(0) : pointer_type->Id();
        assert(module_.type_manager_.FindTypeById(block_type_id)->spv_type_ == SpvType::kStruct && "unexpected block type");

        const bool block_found = GetDecoration(block_type_id, spv::DecorationBlock) != nullptr;

        // If block decoration not found, verify deprecated form of SSBO
        if (!block_found) {
            assert(GetDecoration(block_type_id, spv::DecorationBufferBlock) != nullptr && "block decoration not found");
            storage_class = spv::StorageClassStorageBuffer;
        }
    }

    // A load through a descriptor array will have at least 3 operands. We
    // do not want to instrument loads of descriptors here which are part of
    // an image-based reference.
    if (is_descriptor_array && access_chain_inst_->Length() >= 6) {
        descriptor_index_id_ = access_chain_inst_->Operand(1);
    } else {
        // There is no array of this descriptor, so we essentially have an array of 1
        descriptor_index_id_ = module_.type_manager_.GetConstantZeroUint32().Id();
    }

    for (const auto& annotation : module_.annotations_) {
        if (annotation->Opcode() == spv::OpDecorate && annotation->Word(1) == variable_id) {
            if (annotation->Word(2) == spv::DecorationDescriptorSet) {
                descriptor_set_ = annotation->Word(3);
            } else if (annotation->Word(2) == spv::DecorationBinding) {
                descriptor_binding_ = annotation->Word(3);
            }
        }
    }

    if (descriptor_set_ >= gpuav::glsl::kDebugInputBindlessMaxDescSets) {
        module_.InternalWarning(Name(), "Tried to use a descriptor slot over the current max limit");
        return false;
    }

    // Save information to be used to make the Function
    target_instruction_ = &inst;

    return true;
}

void NonBindlessOOBBufferPass::PrintDebugInfo() {
    std::cout << "NonBindlessOOBBufferPass instrumentation count: " << instrumented_count_ << '\n';
}

// Created own Run() because need to control finding the largest offset in a given block
bool NonBindlessOOBBufferPass::Run() {
    if (module_.has_bindless_descriptors_) return false;
    // Can safely loop function list as there is no injecting of new Functions until linking time
    for (const auto& function : module_.functions_) {
        for (auto block_it = function->blocks_.begin(); block_it != function->blocks_.end(); ++block_it) {
            if ((*block_it)->loop_header_) {
                continue;  // Currently can't properly handle injecting CFG logic into a loop header block
            }
            auto& block_instructions = (*block_it)->instructions_;
            for (auto inst_it = block_instructions.begin(); inst_it != block_instructions.end(); ++inst_it) {
                // Every instruction is analyzed by the specific pass and lets us know if we need to inject a function or not
                if (!AnalyzeInstruction(*function, *(inst_it->get()))) continue;

                if (module_.max_instrumented_count_ != 0 && instrumented_count_ >= module_.max_instrumented_count_) {
                    return true;  // hit limit
                }
                instrumented_count_++;

                // Add any debug information to pass into the function call
                InjectionData injection_data;
                injection_data.stage_info_id = GetStageInfo(*function, block_it, inst_it);
                const uint32_t inst_position = target_instruction_->position_index_;
                auto inst_position_constant = module_.type_manager_.CreateConstantUInt32(inst_position);
                injection_data.inst_position_id = inst_position_constant.Id();

                // inst_it is updated to the instruction after the new function call, it will not add/remove any Blocks
                CreateFunctionCall(**block_it, &inst_it, injection_data);
                Reset();
            }
        }
    }

    return instrumented_count_ != 0;
}

}  // namespace spirv
}  // namespace gpu