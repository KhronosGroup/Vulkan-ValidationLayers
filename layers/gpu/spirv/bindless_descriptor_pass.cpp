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

#include "bindless_descriptor_pass.h"
#include "module.h"
#include <spirv/unified1/spirv.hpp>
#include <iostream>

#include "generated/instrumentation_bindless_descriptor_comp.h"
#include "gpu/shaders/gpu_shaders_constants.h"

namespace gpu {
namespace spirv {

static LinkInfo link_info = {instrumentation_bindless_descriptor_comp, instrumentation_bindless_descriptor_comp_size,
                             LinkFunctions::inst_bindless_descriptor, 0, "inst_bindless_descriptor"};

// By appending the LinkInfo, it will attempt at linking stage to add the function.
uint32_t BindlessDescriptorPass::GetLinkFunctionId() {
    if (link_function_id == 0) {
        link_function_id = module_.TakeNextId();
        link_info.function_id = link_function_id;
        module_.link_info_.push_back(link_info);
    }
    return link_function_id;
}

uint32_t BindlessDescriptorPass::CreateFunctionCall(BasicBlock& block, InstructionIt* inst_it,
                                                    const InjectionData& injection_data) {
    const Constant& set_constant = module_.type_manager_.GetConstantUInt32(descriptor_set_);
    const Constant& binding_constant = module_.type_manager_.GetConstantUInt32(descriptor_binding_);
    const uint32_t descriptor_index_id = CastToUint32(descriptor_index_id_, block, inst_it);  // might be int32

    if (image_inst_) {
        // Get Texel buffer offset
        const uint32_t opcode = target_instruction_->Opcode();
        if (opcode == spv::OpImageRead || opcode == spv::OpImageFetch || opcode == spv::OpImageWrite) {
            const uint32_t image_operand_position = OpcodeImageOperandsPosition(opcode);
            if (target_instruction_->Length() > image_operand_position) {
                const uint32_t image_operand_word = target_instruction_->Word(image_operand_position);
                if ((image_operand_word & (spv::ImageOperandsConstOffsetMask | spv::ImageOperandsOffsetMask)) != 0) {
                    // TODO - Add support if there are image operands (like offset)
                }
            }

            const Type* image_type = module_.type_manager_.FindTypeById(image_inst_->TypeId());
            const uint32_t dim = image_type->inst_.Operand(1);
            if (dim == spv::DimBuffer) {
                const uint32_t depth = image_type->inst_.Operand(2);
                const uint32_t arrayed = image_type->inst_.Operand(3);
                const uint32_t multi_sampling = image_type->inst_.Operand(4);
                if (depth == 0 && arrayed == 0 && multi_sampling == 0) {
                    descriptor_offset_id_ = CastToUint32(target_instruction_->Operand(1), block, inst_it);
                }
            }
        } else {
            // if not a direct read/write/fetch, will be a OpSampledImage
            // "All OpSampledImage instructions must be in the same block in which their Result <id> are consumed"
            // the simple way around this is to add a OpCopyObject to be consumed by the target instruction
            uint32_t image_id = target_instruction_->Operand(0);
            const Instruction* sampled_image_inst = block.function_.FindInstruction(image_id);
            // TODO - Add tests to understand what else can be here other then OpSampledImage
            if (sampled_image_inst->Opcode() == spv::OpSampledImage) {
                const uint32_t type_id = sampled_image_inst->TypeId();
                const uint32_t copy_id = module_.TakeNextId();
                const_cast<Instruction*>(target_instruction_)->ReplaceOperandId(image_id, copy_id);

                // incase the OpSampledImage is shared, copy the previous OpCopyObject
                auto copied = copy_object_map_.find(image_id);
                if (copied != copy_object_map_.end()) {
                    image_id = copied->second;
                    block.CreateInstruction(spv::OpCopyObject, {type_id, copy_id, image_id}, inst_it);
                } else {
                    copy_object_map_.emplace(image_id, copy_id);
                    // slower, but need to guarantee it is placed after a OpSampledImage
                    block.function_.CreateInstruction(spv::OpCopyObject, {type_id, copy_id, image_id}, image_id);
                }
            }
        }
    } else {
        // For now, only do bounds check for non-aggregate types
        // TODO - Do bounds check for aggregate loads and stores
        assert(access_chain_inst_ && var_inst_);
        const Type* pointer_type = module_.type_manager_.FindTypeById(access_chain_inst_->TypeId());
        const Type* pointee_type = module_.type_manager_.FindTypeById(pointer_type->inst_.Word(3));
        if (pointee_type && pointee_type->spv_type_ != SpvType::kArray && pointee_type->spv_type_ != SpvType::kRuntimeArray &&
            pointee_type->spv_type_ != SpvType::kStruct) {
            descriptor_offset_id_ = GetLastByte(*var_inst_, *access_chain_inst_, block, inst_it);  // Get Last Byte Index
        }
    }

    if (descriptor_offset_id_ == 0) {
        descriptor_offset_id_ = module_.type_manager_.GetConstantZeroUint32().Id();
    }

    const uint32_t function_result = module_.TakeNextId();
    const uint32_t function_def = GetLinkFunctionId();
    const uint32_t bool_type = module_.type_manager_.GetTypeBool().Id();

    block.CreateInstruction(
        spv::OpFunctionCall,
        {bool_type, function_result, function_def, injection_data.inst_position_id, injection_data.stage_info_id, set_constant.Id(),
         binding_constant.Id(), descriptor_index_id, descriptor_offset_id_},
        inst_it);

    return function_result;
}

void BindlessDescriptorPass::Reset() {
    access_chain_inst_ = nullptr;
    var_inst_ = nullptr;
    image_inst_ = nullptr;
    target_instruction_ = nullptr;
    descriptor_set_ = 0;
    descriptor_binding_ = 0;
    descriptor_index_id_ = 0;
    descriptor_offset_id_ = 0;
}

bool BindlessDescriptorPass::AnalyzeInstruction(const Function& function, const Instruction& inst) {
    const uint32_t opcode = inst.Opcode();

    if (opcode == spv::OpLoad || opcode == spv::OpStore) {
        // For now only have non-bindless support for buffers
        if (!module_.has_bindless_descriptors_) return false;

        // TODO - Should have loop to walk Load/Store to the Pointer,
        // this case will not cover things such as OpCopyObject or double OpAccessChains
        access_chain_inst_ = function.FindInstruction(inst.Operand(0));
        if (!access_chain_inst_ || access_chain_inst_->Opcode() != spv::OpAccessChain) {
            return false;
        }

        const Variable* variable = module_.type_manager_.FindVariableById(access_chain_inst_->Operand(0));
        if (!variable) {
            return false;
        }
        var_inst_ = &variable->inst_;

        uint32_t storage_class = variable->StorageClass();
        if (storage_class != spv::StorageClassUniform && storage_class != spv::StorageClassStorageBuffer) {
            return false;
        }

        const Type* pointer_type = variable->PointerType(module_.type_manager_);

        // Check for deprecated storage block form
        if (storage_class == spv::StorageClassUniform) {
            const uint32_t block_type_id = (pointer_type->inst_.IsArray()) ? pointer_type->inst_.Operand(0) : pointer_type->Id();
            if (module_.type_manager_.FindTypeById(block_type_id)->spv_type_ != SpvType::kStruct) {
                module_.InternalError(Name(), "Uniform variable block type is not OpTypeStruct");
                return false;
            }

            const bool block_found = GetDecoration(block_type_id, spv::DecorationBlock) != nullptr;

            // If block decoration not found, verify deprecated form of SSBO
            if (!block_found) {
                if (GetDecoration(block_type_id, spv::DecorationBufferBlock) == nullptr) {
                    module_.InternalError(Name(), "Uniform variable block decoration not found");
                    return false;
                }
                storage_class = spv::StorageClassStorageBuffer;
            }
        }

        // A load through a descriptor array will have at least 3 operands. We
        // do not want to instrument loads of descriptors here which are part of
        // an image-based reference.
        if (pointer_type->inst_.IsArray() && access_chain_inst_->Length() >= 6) {
            descriptor_index_id_ = access_chain_inst_->Operand(1);
        } else {
            descriptor_index_id_ = module_.type_manager_.GetConstantZeroUint32().Id();
        }

    } else {
        // TODO - Once all non-bindless passes are added, this check can be places at top of Run()
        if (!module_.has_bindless_descriptors_ &&
            (opcode == spv::OpImageFetch || opcode == spv::OpImageRead || opcode == spv::OpImageWrite)) {
            return false;
        }

        // Reference is not load or store, so ifi it isn't a image-based reference, move on
        const uint32_t image_word = OpcodeImageAccessPosition(opcode);
        if (image_word == 0) {
            return false;
        }
        if (opcode == spv::OpImageTexelPointer || opcode == spv::OpImage) {
            return false;  // need to test if we can support these
        }

        image_inst_ = function.FindInstruction(inst.Word(image_word));
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
                module_.InternalError(Name(), "OpAccessChain has more than 1 indexes");
                return false;
            }

            const Variable* variable = module_.type_manager_.FindVariableById(var_inst_->Operand(0));
            if (!variable) {
                module_.InternalError(Name(), "OpAccessChain base is not a variable");
                return false;
            }
            var_inst_ = &variable->inst_;
        } else {
            descriptor_index_id_ = module_.type_manager_.GetConstantZeroUint32().Id();
        }
    }

    assert(var_inst_);
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

void BindlessDescriptorPass::PrintDebugInfo() {
    std::cout << "BindlessDescriptorPass instrumentation count: " << instrumented_count_ << '\n';
}

}  // namespace spirv
}  // namespace gpu