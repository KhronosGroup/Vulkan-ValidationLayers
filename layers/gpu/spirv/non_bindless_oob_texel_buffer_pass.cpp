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

#include "non_bindless_oob_texel_buffer_pass.h"
#include "module.h"
#include <spirv/unified1/spirv.hpp>
#include <iostream>

#include "generated/instrumentation_non_bindless_oob_texel_buffer_comp.h"
#include "gpu/shaders/gpu_shaders_constants.h"

namespace gpu {
namespace spirv {

NonBindlessOOBTexelBufferPass::NonBindlessOOBTexelBufferPass(Module& module) : Pass(module) { module.use_bda_ = true; }

static LinkInfo link_info = {instrumentation_non_bindless_oob_texel_buffer_comp,
                             instrumentation_non_bindless_oob_texel_buffer_comp_size,
                             LinkFunctions::inst_non_bindless_oob_texel_buffer, 0, "inst_non_bindless_oob_texel_buffer"};

// By appending the LinkInfo, it will attempt at linking stage to add the function.
uint32_t NonBindlessOOBTexelBufferPass::GetLinkFunctionId() {
    if (link_function_id == 0) {
        link_function_id = module_.TakeNextId();
        link_info.function_id = link_function_id;
        module_.link_info_.push_back(link_info);
    }
    return link_function_id;
}

uint32_t NonBindlessOOBTexelBufferPass::CreateFunctionCall(BasicBlock& block, InstructionIt* inst_it,
                                                           const InjectionData& injection_data) {
    assert(access_chain_inst_ && var_inst_);
    const Constant& set_constant = module_.type_manager_.GetConstantUInt32(descriptor_set_);
    const Constant& binding_constant = module_.type_manager_.GetConstantUInt32(descriptor_binding_);
    const uint32_t descriptor_index_id = CastToUint32(descriptor_index_id_, block, inst_it);  // might be int32

    const uint32_t opcode = target_instruction_->Opcode();
    const uint32_t image_operand_position = OpcodeImageOperandsPosition(opcode);
    if (target_instruction_->Length() > image_operand_position) {
        const uint32_t image_operand_word = target_instruction_->Word(image_operand_position);
        if ((image_operand_word & (spv::ImageOperandsConstOffsetMask | spv::ImageOperandsOffsetMask)) != 0) {
            // TODO - Add support if there are image operands (like offset)
        }
    }

    // Use the imageFetch() parameter to decide the offset
    // TODO - This assumes no depth/arrayed/ms from AnalyzeInstruction
    descriptor_offset_id_ = CastToUint32(target_instruction_->Operand(1), block, inst_it);

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

void NonBindlessOOBTexelBufferPass::Reset() {
    access_chain_inst_ = nullptr;
    var_inst_ = nullptr;
    target_instruction_ = nullptr;
    descriptor_array_size_id_ = 0;
    descriptor_set_ = 0;
    descriptor_binding_ = 0;
    descriptor_index_id_ = 0;
    descriptor_offset_id_ = 0;
}

bool NonBindlessOOBTexelBufferPass::AnalyzeInstruction(const Function& function, const Instruction& inst) {
    const uint32_t opcode = inst.Opcode();

    if (opcode != spv::OpImageFetch && opcode != spv::OpImageWrite && opcode != spv::OpImageRead) {
        return false;
    }
    const uint32_t image_word = OpcodeImageAccessPosition(opcode);

    image_inst_ = function.FindInstruction(inst.Word(image_word));
    if (!image_inst_) return false;
    const Type* image_type = module_.type_manager_.FindTypeById(image_inst_->TypeId());
    if (!image_type) return false;

    const uint32_t dim = image_type->inst_.Operand(1);
    if (dim != spv::DimBuffer) {
        return false;  // It is a Storage Image
    }
    const uint32_t depth = image_type->inst_.Operand(2);
    const uint32_t arrayed = image_type->inst_.Operand(3);
    const uint32_t multi_sampling = image_type->inst_.Operand(4);
    if (depth != 0 || arrayed != 0 || multi_sampling != 0) {
        // TODO - Currently don't support caculating these for getting the OOB offset, so not worst continuing
        return false;
    }

    // walk down to get the actual load
    const Instruction* load_inst = image_inst_;
    while (load_inst && (load_inst->Opcode() == spv::OpSampledImage || load_inst->Opcode() == spv::OpImage ||
                         load_inst->Opcode() == spv::OpCopyObject)) {
        load_inst = function.FindInstruction(load_inst->Operand(0));
    }
    if (!load_inst || load_inst->Opcode() != spv::OpLoad) {
        return false;  // TODO: Handle additional possibilities?
    }

    var_inst_ = function.FindInstruction(load_inst->Operand(0));
    if (!var_inst_) {
        // can be a global variable
        const Variable* global_var = module_.type_manager_.FindVariableById(load_inst->Operand(0));
        var_inst_ = global_var ? &global_var->inst_ : nullptr;
    }
    if (!var_inst_ || (var_inst_->Opcode() != spv::OpAccessChain && var_inst_->Opcode() != spv::OpVariable)) {
        return false;
    }

    // If OpVariable, access_chain_inst_ is never checked because it should be a direct image access
    access_chain_inst_ = var_inst_;

    if (var_inst_->Opcode() == spv::OpAccessChain) {
        descriptor_index_id_ = var_inst_->Operand(1);

        if (var_inst_->Length() > 5) {
            module_.InternalError(Name(), "OpAccessChain has more than 1 indexes. 2D Texel Buffers not supported");
            return false;
        }

        const Variable* variable = module_.type_manager_.FindVariableById(var_inst_->Operand(0));
        if (!variable) {
            module_.InternalError(Name(), "OpAccessChain base is not a variable");
            return false;
        }
        var_inst_ = &variable->inst_;

        const Type* pointer_type = variable->PointerType(module_.type_manager_);
        if (pointer_type->inst_.Opcode() == spv::OpTypeRuntimeArray) {
            return false;  // Currently we mark these as "bindless"
        } else if (pointer_type->inst_.Opcode() == spv::OpTypeArray) {
            const Constant* array_size_const = module_.type_manager_.FindConstantById(pointer_type->inst_.Operand(1));
            if (!array_size_const) {
                return false;  // TODO - Handle Spec Constants here
            }
            descriptor_array_size_id_ = array_size_const->Id();
        } else {
            module_.InternalError(Name(), "OpAccessChain has no array in it");
            return false;
        }

    } else {
        // There is no array of this descriptor, so we essentially have an array of 1
        descriptor_index_id_ = module_.type_manager_.GetConstantZeroUint32().Id();
        descriptor_array_size_id_ = module_.type_manager_.GetConstantUInt32(1).Id();
    }

    uint32_t variable_id = var_inst_->ResultId();
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

void NonBindlessOOBTexelBufferPass::PrintDebugInfo() {
    std::cout << "NonBindlessOOBTexelBufferPass instrumentation count: " << instrumented_count_ << '\n';
}

// Created own Run() because need to control finding the largest offset in a given block
bool NonBindlessOOBTexelBufferPass::Run() {
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