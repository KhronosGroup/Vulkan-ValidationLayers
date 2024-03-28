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

#include "generated/inst_bindless_descriptor_comp.h"

namespace gpuav {
namespace spirv {

static LinkInfo link_info = {inst_bindless_descriptor_comp, inst_bindless_descriptor_comp_size,
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

uint32_t BindlessDescriptorPass::FindTypeByteSize(uint32_t type_id, uint32_t matrix_stride, bool col_major, bool in_matrix) {
    const Type& type = *module_.type_manager_.FindTypeById(type_id);
    switch (type.spv_type_) {
        case SpvType::kPointer:
            return 8;  // Assuming PhysicalStorageBuffer pointer
            break;
        case SpvType::kMatrix: {
            assert(matrix_stride != 0 && "missing matrix stride");
            if (col_major) {
                return type.inst_.Word(3) * matrix_stride;
            } else {
                const Type* vector_type = module_.type_manager_.FindTypeById(type.inst_.Word(2));
                return vector_type->inst_.Word(3) * matrix_stride;
            }
        }
        case SpvType::kVector: {
            uint32_t size = type.inst_.Word(3);
            const Type* component_type = module_.type_manager_.FindTypeById(type.inst_.Word(2));
            // if vector in row major matrix, the vector is strided so return the number of bytes spanned by the vector
            if (in_matrix && !col_major && matrix_stride > 0) {
                return (size - 1) * matrix_stride + FindTypeByteSize(component_type->Id());
            } else if (component_type->spv_type_ == SpvType::kFloat || component_type->spv_type_ == SpvType::kInt) {
                const uint32_t width = component_type->inst_.Word(2);
                size *= width;
            } else {
                assert(false && "unexpected type");
            }
            return size / 8;
        }
        case SpvType::kFloat:
        case SpvType::kInt: {
            const uint32_t width = type.inst_.Word(2);
            return width / 8;
        }
        default:
            break;
    }
    return 1;
}

// Find outermost buffer type and its access chain index.
// Because access chains indexes can be runtime values, we need to build arithmetic logic in the SPIR-V to get the runtime value of
// the indexing
uint32_t BindlessDescriptorPass::GetLastByte(BasicBlock& block) {
    const Type* pointer_type = module_.type_manager_.FindTypeById(var_inst_->TypeId());
    const Type* descriptor_type = module_.type_manager_.FindTypeById(pointer_type->inst_.Word(3));

    uint32_t current_type_id = 0;
    uint32_t ac_word_index = 4;

    if (descriptor_type->spv_type_ == SpvType::kArray || descriptor_type->spv_type_ == SpvType::kRuntimeArray) {
        current_type_id = descriptor_type->inst_.Operand(0);
        ac_word_index++;
    } else if (descriptor_type->spv_type_ == SpvType::kStruct) {
        current_type_id = descriptor_type->Id();
    } else {
        assert(false && "unexpected descriptor type");
        return 0;
    }

    const Type& uint32_type = module_.type_manager_.GetTypeInt(32, false);

    // instruction that will have calculated the sum of the byte offset
    uint32_t sum_id = 0;

    uint32_t matrix_stride = 0;
    bool col_major = false;
    uint32_t matrix_stride_id = 0;
    bool in_matrix = false;

    while (ac_word_index < access_chain_inst_->Length()) {
        const uint32_t ac_index_id = access_chain_inst_->Word(ac_word_index);
        uint32_t current_offset_id = 0;

        const Type* current_type = module_.type_manager_.FindTypeById(current_type_id);
        switch (current_type->spv_type_) {
            case SpvType::kArray:
            case SpvType::kRuntimeArray: {
                // Get array stride and multiply by current index
                uint32_t arr_stride = GetDecoration(current_type_id, spv::DecorationArrayStride)->Word(3);
                const uint32_t arr_stride_id = module_.type_manager_.GetConstantUInt32(arr_stride).Id();
                const uint32_t ac_index_id_32 = ConvertTo32(ac_index_id, block);

                current_offset_id = module_.TakeNextId();
                block.CreateInstruction(spv::OpIMul, {uint32_type.Id(), current_offset_id, arr_stride_id, ac_index_id_32});

                // Get element type for next step
                current_type_id = current_type->inst_.Operand(0);
            } break;
            case SpvType::kMatrix: {
                assert(matrix_stride != 0 && "missing matrix stride");
                matrix_stride_id = module_.type_manager_.GetConstantUInt32(matrix_stride).Id();
                uint32_t vec_type_id = current_type->inst_.Operand(0);

                // If column major, multiply column index by matrix stride, otherwise by vector component size and save matrix
                // stride for vector (row) index
                uint32_t col_stride_id = 0;
                if (col_major) {
                    col_stride_id = matrix_stride_id;
                } else {
                    const uint32_t component_type_id = module_.type_manager_.FindTypeById(vec_type_id)->inst_.Operand(0);
                    const uint32_t col_stride = FindTypeByteSize(component_type_id);
                    col_stride_id = module_.type_manager_.GetConstantUInt32(col_stride).Id();
                }

                const uint32_t ac_index_id_32 = ConvertTo32(ac_index_id, block);
                current_offset_id = module_.TakeNextId();
                block.CreateInstruction(spv::OpIMul, {uint32_type.Id(), current_offset_id, col_stride_id, ac_index_id_32});

                // Get element type for next step
                current_type_id = vec_type_id;
                in_matrix = true;
            } break;
            case SpvType::kVector: {
                // If inside a row major matrix type, multiply index by matrix stride,
                // else multiply by component size
                const uint32_t component_type_id = current_type->inst_.Operand(0);
                const uint32_t ac_index_id_32 = ConvertTo32(ac_index_id, block);
                if (in_matrix && !col_major) {
                    current_offset_id = module_.TakeNextId();
                    block.CreateInstruction(spv::OpIMul, {uint32_type.Id(), current_offset_id, matrix_stride_id, ac_index_id_32});
                } else {
                    const uint32_t component_type_size = FindTypeByteSize(component_type_id);
                    const uint32_t size_id = module_.type_manager_.GetConstantUInt32(component_type_size).Id();

                    current_offset_id = module_.TakeNextId();
                    block.CreateInstruction(spv::OpIMul, {uint32_type.Id(), current_offset_id, size_id, ac_index_id_32});
                }
                // Get element type for next step
                current_type_id = component_type_id;
            } break;
            case SpvType::kStruct: {
                // Get buffer byte offset for the referenced member
                const Constant* member_constant = module_.type_manager_.FindConstantById(ac_index_id);
                uint32_t member_index = member_constant->inst_.Operand(0);
                uint32_t member_offset = GetMemeberDecoration(current_type_id, member_index, spv::DecorationOffset)->Word(4);
                current_offset_id = module_.type_manager_.GetConstantUInt32(member_offset).Id();

                // Look for matrix stride for this member if there is one. The matrix
                // stride is not on the matrix type, but in a OpMemberDecorate on the
                // enclosing struct type at the member index. If none found, reset
                // stride to 0.
                const Instruction* decoration_matrix_stride =
                    GetMemeberDecoration(current_type_id, member_index, spv::DecorationMatrixStride);
                matrix_stride = decoration_matrix_stride ? decoration_matrix_stride->Word(4) : 0;

                const Instruction* decoration_col_major =
                    GetMemeberDecoration(current_type_id, member_index, spv::DecorationColMajor);
                col_major = decoration_col_major != nullptr;

                // Get element type for next step
                current_type_id = current_type->inst_.Operand(member_index);
            } break;
            default: {
                assert(false && "unexpected non-composite type");
            } break;
        }

        if (sum_id == 0) {
            sum_id = current_offset_id;
        } else {
            const uint32_t new_sum_id = module_.TakeNextId();
            block.CreateInstruction(spv::OpIAdd, {uint32_type.Id(), new_sum_id, sum_id, current_offset_id});
            sum_id = new_sum_id;
        }
        ac_word_index++;
    }

    // Add in offset of last byte of referenced object
    uint32_t bsize = FindTypeByteSize(current_type_id, matrix_stride, col_major, in_matrix);
    uint32_t last = bsize - 1;

    const uint32_t last_id = module_.type_manager_.GetConstantUInt32(last).Id();

    const uint32_t new_sum_id = module_.TakeNextId();
    block.CreateInstruction(spv::OpIAdd, {uint32_type.Id(), new_sum_id, sum_id, last_id});
    return new_sum_id;
}

uint32_t BindlessDescriptorPass::CreateFunctionCall(BasicBlock& block) {
    // Add any debug information to pass into the function call
    const uint32_t stage_info_id = GetStageInfo(block.function_);
    const uint32_t inst_position = target_instruction_->position_index_;
    auto inst_position_constant = module_.type_manager_.CreateConstantUInt32(inst_position);

    const Constant& set_constant = module_.type_manager_.GetConstantUInt32(descriptor_set_);
    const Constant& binding_constant = module_.type_manager_.GetConstantUInt32(descriptor_binding_);
    const uint32_t descriptor_index_id = CastToUint32(descriptor_index_id_, block);  // might be int32

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
                const uint32_t ms = image_type->inst_.Operand(4);
                if (depth == 0 && arrayed == 0 && ms == 0) {
                    descriptor_offset_id_ = CastToUint32(target_instruction_->Operand(1), block);
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
                    block.CreateInstruction(spv::OpCopyObject, {type_id, copy_id, image_id});
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
            descriptor_offset_id_ = GetLastByte(block);  // Get Last Byte Index
        }
    }

    if (descriptor_offset_id_ == 0) {
        descriptor_offset_id_ = module_.type_manager_.GetConstantZeroUint32().Id();
    }

    const uint32_t function_result = module_.TakeNextId();
    const uint32_t function_def = GetLinkFunctionId();
    const uint32_t bool_type = module_.type_manager_.GetTypeBool().Id();

    block.CreateInstruction(spv::OpFunctionCall,
                            {bool_type, function_result, function_def, inst_position_constant.Id(), stage_info_id,
                             set_constant.Id(), binding_constant.Id(), descriptor_index_id, descriptor_offset_id_});

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

        const uint32_t pointer_type_id = variable->type_.inst_.Operand(1);
        const Type* pointer_type = module_.type_manager_.FindTypeById(pointer_type_id);

        // Check for deprecated storage block form
        if (storage_class == spv::StorageClassUniform) {
            const uint32_t block_type_id = (pointer_type->inst_.IsArray()) ? pointer_type->inst_.Operand(0) : pointer_type_id;
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
        if (pointer_type->inst_.IsArray() && access_chain_inst_->Length() >= 6) {
            descriptor_index_id_ = access_chain_inst_->Operand(1);
        } else {
            descriptor_index_id_ = module_.type_manager_.GetConstantZeroUint32().Id();
        }

    } else {
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
                assert(false && "OpAccessChain has more than 1 indexes");
                return false;
            }

            const Variable* variable = module_.type_manager_.FindVariableById(var_inst_->Operand(0));
            if (!variable) {
                assert(false && "OpAccessChain base is not a variable");
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

    // Save information to be used to make the Function
    target_instruction_ = &inst;

    return true;
}

}  // namespace spirv
}  // namespace gpuav