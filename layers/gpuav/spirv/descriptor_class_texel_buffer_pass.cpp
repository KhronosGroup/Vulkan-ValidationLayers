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

#include "descriptor_class_texel_buffer_pass.h"
#include "module.h"
#include <spirv/unified1/spirv.hpp>
#include <iostream>

#include "generated/instrumentation_descriptor_class_texel_buffer_comp.h"
#include "gpuav/shaders/gpuav_shaders_constants.h"

namespace gpuav {
namespace spirv {

static LinkInfo link_info = {instrumentation_descriptor_class_texel_buffer_comp,
                             instrumentation_descriptor_class_texel_buffer_comp_size, 0, "inst_descriptor_class_texel_buffer"};

DescriptorClassTexelBufferPass::DescriptorClassTexelBufferPass(Module& module) : Pass(module) {
    module.use_bda_ = true;
    link_info.function_id = 0;  // reset each pass
}

// By appending the LinkInfo, it will attempt at linking stage to add the function.
uint32_t DescriptorClassTexelBufferPass::GetLinkFunctionId() {
    if (link_info.function_id == 0) {
        link_info.function_id = module_.TakeNextId();
        module_.link_info_.push_back(link_info);
    }
    return link_info.function_id;
}

uint32_t DescriptorClassTexelBufferPass::CreateFunctionCall(BasicBlock& block, InstructionIt* inst_it,
                                                            const InjectionData& injection_data, const InstructionMeta& meta) {
    assert(meta.access_chain_inst && meta.var_inst);
    const Constant& set_constant = module_.type_manager_.GetConstantUInt32(meta.descriptor_set);
    const Constant& binding_constant = module_.type_manager_.GetConstantUInt32(meta.descriptor_binding);
    const uint32_t descriptor_index_id = CastToUint32(meta.descriptor_index_id, block, inst_it);  // might be int32

    const uint32_t opcode = meta.target_instruction->Opcode();
    const uint32_t image_operand_position = OpcodeImageOperandsPosition(opcode);
    if (meta.target_instruction->Length() > image_operand_position) {
        const uint32_t image_operand_word = meta.target_instruction->Word(image_operand_position);
        if ((image_operand_word & (spv::ImageOperandsConstOffsetMask | spv::ImageOperandsOffsetMask)) != 0) {
            // TODO - Add support if there are image operands (like offset)
        }
    }

    // Use the imageFetch() parameter to decide the offset
    // TODO - This assumes no depth/arrayed/ms from RequiresInstrumentation
    const uint32_t descriptor_offset_id = CastToUint32(meta.target_instruction->Operand(1), block, inst_it);

    BindingLayout binding_layout = module_.set_index_to_bindings_layout_lut_[meta.descriptor_set][meta.descriptor_binding];
    const Constant& binding_layout_offset = module_.type_manager_.GetConstantUInt32(binding_layout.start);

    const uint32_t function_result = module_.TakeNextId();
    const uint32_t function_def = GetLinkFunctionId();
    const uint32_t bool_type = module_.type_manager_.GetTypeBool().Id();

    block.CreateInstruction(
        spv::OpFunctionCall,
        {bool_type, function_result, function_def, injection_data.inst_position_id, injection_data.stage_info_id, set_constant.Id(),
         binding_constant.Id(), descriptor_index_id, descriptor_offset_id, binding_layout_offset.Id()},
        inst_it);

    return function_result;
}

bool DescriptorClassTexelBufferPass::RequiresInstrumentation(const Function& function, const Instruction& inst,
                                                             InstructionMeta& meta) {
    const uint32_t opcode = inst.Opcode();

    if (opcode != spv::OpImageFetch && opcode != spv::OpImageWrite && opcode != spv::OpImageRead) {
        return false;
    }
    const uint32_t image_word = OpcodeImageAccessPosition(opcode);

    meta.image_inst = function.FindInstruction(inst.Word(image_word));
    if (!meta.image_inst) return false;
    const Type* image_type = module_.type_manager_.FindTypeById(meta.image_inst->TypeId());
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
    const Instruction* load_inst = meta.image_inst;
    while (load_inst && (load_inst->Opcode() == spv::OpSampledImage || load_inst->Opcode() == spv::OpImage ||
                         load_inst->Opcode() == spv::OpCopyObject)) {
        load_inst = function.FindInstruction(load_inst->Operand(0));
    }
    if (!load_inst || load_inst->Opcode() != spv::OpLoad) {
        return false;  // TODO: Handle additional possibilities?
    }

    meta.var_inst = function.FindInstruction(load_inst->Operand(0));
    if (!meta.var_inst) {
        // can be a global variable
        const Variable* global_var = module_.type_manager_.FindVariableById(load_inst->Operand(0));
        meta.var_inst = global_var ? &global_var->inst_ : nullptr;
    }
    if (!meta.var_inst || (meta.var_inst->Opcode() != spv::OpAccessChain && meta.var_inst->Opcode() != spv::OpVariable)) {
        return false;
    }

    // If OpVariable, access_chain_inst_ is never checked because it should be a direct image access
    meta.access_chain_inst = meta.var_inst;

    if (meta.var_inst->Opcode() == spv::OpAccessChain) {
        meta.descriptor_index_id = meta.var_inst->Operand(1);

        if (meta.var_inst->Length() > 5) {
            module_.InternalError(Name(), "OpAccessChain has more than 1 indexes. 2D Texel Buffers not supported");
            return false;
        }

        const Variable* variable = module_.type_manager_.FindVariableById(meta.var_inst->Operand(0));
        if (!variable) {
            module_.InternalError(Name(), "OpAccessChain base is not a variable");
            return false;
        }
        meta.var_inst = &variable->inst_;
    } else {
        // There is no array of this descriptor, so we essentially have an array of 1
        meta.descriptor_index_id = module_.type_manager_.GetConstantZeroUint32().Id();
    }

    uint32_t variable_id = meta.var_inst->ResultId();
    for (const auto& annotation : module_.annotations_) {
        if (annotation->Opcode() == spv::OpDecorate && annotation->Word(1) == variable_id) {
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

    // Save information to be used to make the Function
    meta.target_instruction = &inst;

    return true;
}

void DescriptorClassTexelBufferPass::PrintDebugInfo() const {
    std::cout << "DescriptorClassTexelBufferPass instrumentation count: " << instrumentations_count_ << '\n';
}

// Created own Instrument() because need to control finding the largest offset in a given block
bool DescriptorClassTexelBufferPass::Instrument() {
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
            for (auto inst_it = block_instructions.begin(); inst_it != block_instructions.end(); ++inst_it) {
                InstructionMeta meta;
                // Every instruction is analyzed by the specific pass and lets us know if we need to inject a function or not
                if (!RequiresInstrumentation(*function, *(inst_it->get()), meta)) continue;

                if (IsMaxInstrumentationsCount()) continue;
                instrumentations_count_++;

                InjectionData injection_data = GetInjectionData(*function, current_block, inst_it, *meta.target_instruction);

                // inst_it is updated to the instruction after the new function call, it will not add/remove any Blocks
                CreateFunctionCall(current_block, &inst_it, injection_data, meta);
            }
        }
    }

    return instrumentations_count_ != 0;
}

}  // namespace spirv
}  // namespace gpuav