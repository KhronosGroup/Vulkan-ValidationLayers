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

#include "generated/instrumentation_descriptor_class_general_buffer_comp.h"
#include "gpuav/shaders/gpuav_shaders_constants.h"

namespace gpuav {
namespace spirv {

static LinkInfo link_info = {instrumentation_descriptor_class_general_buffer_comp,
                             instrumentation_descriptor_class_general_buffer_comp_size, 0, "inst_descriptor_class_general_buffer"};

DescriptorClassGeneralBufferPass::DescriptorClassGeneralBufferPass(Module& module)
    : Pass(module), unsafe_mode_(module.settings_.unsafe_mode) {
    module.use_bda_ = true;
    link_info.function_id = 0;  // reset each pass
}

// By appending the LinkInfo, it will attempt at linking stage to add the function.
uint32_t DescriptorClassGeneralBufferPass::GetLinkFunctionId() {
    if (link_info.function_id == 0) {
        link_info.function_id = module_.TakeNextId();
        module_.link_info_.push_back(link_info);
    }
    return link_info.function_id;
}

// Finds the offset into the SSBO/UBO an instruction would access
// If it is a non-constant value, will return zero to indicate its a runtime value
//
// If shader looks for 'a' in a descriptor like
//
// struct X {
//    uint a;
//    uint b;
// }
//
// it will return `3` because it covers [0, 3] bytes of the descriptor
// (This matches the GetLastByte() check)
uint32_t DescriptorClassGeneralBufferPass::FindLastByteOffset(uint32_t descriptor_id, bool is_descriptor_array,
                                                              const std::vector<const Instruction*>& access_chain_insts) const {
    assert(!access_chain_insts.empty());
    uint32_t last_byte_offset = 0;
    const uint32_t reset_ac_word = 4;  // points to first "Index" operand of an OpAccessChain
    uint32_t ac_word_index = reset_ac_word;

    if (is_descriptor_array) {
        ac_word_index++;  // this jumps over the array of descriptors so we first start on the descriptor itself
    }

    uint32_t matrix_stride = 0;
    bool col_major = false;
    bool in_matrix = false;

    auto access_chain_iter = access_chain_insts.rbegin();

    // This occurs in things like Slang where they have a single OpAccessChain for the descriptor
    // (GLSL/HLSL will combine 2 indexes into the last OpAccessChain)
    if (ac_word_index >= (*access_chain_iter)->Length()) {
        ++access_chain_iter;
        ac_word_index = reset_ac_word;
    }

    uint32_t current_type_id = descriptor_id;
    // Walk down access chains to build up the offset
    while (access_chain_iter != access_chain_insts.rend()) {
        const uint32_t ac_index_id = (*access_chain_iter)->Word(ac_word_index);
        const Constant* index_constant = module_.type_manager_.FindConstantById(ac_index_id);
        if (!index_constant || index_constant->inst_.Opcode() != spv::OpConstant) {
            return 0;  // Access Chain has dynamic value
        }
        const uint32_t constant_value = index_constant->inst_.Word(3);

        uint32_t current_offset = 0;

        const Type* current_type = module_.type_manager_.FindTypeById(current_type_id);
        switch (current_type->spv_type_) {
            case SpvType::kArray:
            case SpvType::kRuntimeArray: {
                // Get array stride and multiply by current index
                const uint32_t array_stride = GetDecoration(current_type_id, spv::DecorationArrayStride)->Word(3);
                current_offset = constant_value * array_stride;

                current_type_id = current_type->inst_.Operand(0);  // Get element type for next step
            } break;
            case SpvType::kMatrix: {
                if (matrix_stride == 0) {
                    module_.InternalError(Name(), "FindLastByteOffset is missing matrix stride");
                }
                in_matrix = true;
                uint32_t vec_type_id = current_type->inst_.Operand(0);

                // If column major, multiply column index by matrix stride, otherwise by vector component size and save matrix
                // stride for vector (row) index
                uint32_t col_stride = 0;
                if (col_major) {
                    col_stride = matrix_stride;
                } else {
                    const uint32_t component_type_id = module_.type_manager_.FindTypeById(vec_type_id)->inst_.Operand(0);
                    col_stride = FindTypeByteSize(component_type_id);
                }

                current_offset = constant_value * col_stride;

                current_type_id = vec_type_id;  // Get element type for next step
            } break;
            case SpvType::kVector: {
                // If inside a row major matrix type, multiply index by matrix stride,
                // else multiply by component size
                const uint32_t component_type_id = current_type->inst_.Operand(0);

                if (in_matrix && !col_major) {
                    current_offset = constant_value * matrix_stride;
                } else {
                    const uint32_t component_type_size = FindTypeByteSize(component_type_id);
                    current_offset = constant_value * component_type_size;
                }

                current_type_id = component_type_id;  // Get element type for next step
            } break;
            case SpvType::kStruct: {
                // Get buffer byte offset for the referenced member
                current_offset = GetMemberDecoration(current_type_id, constant_value, spv::DecorationOffset)->Word(4);

                // Look for matrix stride for this member if there is one. The matrix
                // stride is not on the matrix type, but in a OpMemberDecorate on the
                // enclosing struct type at the member index. If none is found, reset
                // stride to 0.
                const Instruction* decoration_matrix_stride =
                    GetMemberDecoration(current_type_id, constant_value, spv::DecorationMatrixStride);
                matrix_stride = decoration_matrix_stride ? decoration_matrix_stride->Word(4) : 0;

                const Instruction* decoration_col_major =
                    GetMemberDecoration(current_type_id, constant_value, spv::DecorationColMajor);
                col_major = decoration_col_major != nullptr;

                current_type_id = current_type->inst_.Operand(constant_value);  // Get element type for next step
            } break;
            default: {
                module_.InternalError(Name(), "FindLastByteOffset has unexpected non-composite type");
            } break;
        }

        last_byte_offset += current_offset;

        ac_word_index++;
        if (ac_word_index >= (*access_chain_iter)->Length()) {
            ++access_chain_iter;
            ac_word_index = reset_ac_word;
        }
    }

    // Add in offset of last byte of referenced object
    const uint32_t accessed_type_size = FindTypeByteSize(current_type_id, matrix_stride, col_major, in_matrix);
    const uint32_t last_byte_index = accessed_type_size - 1;
    last_byte_offset += last_byte_index;

    return last_byte_offset;
}

uint32_t DescriptorClassGeneralBufferPass::CreateFunctionCall(BasicBlock& block, InstructionIt* inst_it,
                                                              const InjectionData& injection_data, const InstructionMeta& meta) {
    assert(!meta.access_chain_insts.empty());
    const Constant& set_constant = module_.type_manager_.GetConstantUInt32(meta.descriptor_set);
    const Constant& binding_constant = module_.type_manager_.GetConstantUInt32(meta.descriptor_binding);
    const uint32_t descriptor_index_id = CastToUint32(meta.descriptor_index_id, block, inst_it);  // might be int32

    const uint32_t descriptor_offset_id =
        GetLastByte(*meta.descriptor_type, meta.access_chain_insts, block, inst_it);  // Get Last Byte Index

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

bool DescriptorClassGeneralBufferPass::RequiresInstrumentation(const Function& function, const Instruction& inst,
                                                               InstructionMeta& meta, bool pre_pass) {
    const uint32_t opcode = inst.Opcode();

    if (!IsValueIn(spv::Op(opcode), {spv::OpLoad, spv::OpStore, spv::OpAtomicStore, spv::OpAtomicLoad, spv::OpAtomicExchange})) {
        return false;
    }

    const Instruction* next_access_chain = function.FindInstruction(inst.Operand(0));
    if (!next_access_chain || next_access_chain->Opcode() != spv::OpAccessChain) {
        return false;
    }

    const Variable* variable = nullptr;
    // We need to walk down possibly multiple chained OpAccessChains or OpCopyObject to get the variable
    while (next_access_chain && next_access_chain->Opcode() == spv::OpAccessChain) {
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
    const uint32_t descriptor_id = is_descriptor_array ? meta.descriptor_type->inst_.Operand(0) : meta.descriptor_type->Id();

    // Check for deprecated storage block form
    if (storage_class == spv::StorageClassUniform) {
        assert(module_.type_manager_.FindTypeById(descriptor_id)->spv_type_ == SpvType::kStruct && "unexpected block type");

        const bool block_found = GetDecoration(descriptor_id, spv::DecorationBlock) != nullptr;

        // If block decoration not found, verify deprecated form of SSBO
        if (!block_found) {
            assert(GetDecoration(descriptor_id, spv::DecorationBufferBlock) != nullptr && "block decoration not found");
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

    if (unsafe_mode_) {
        const uint32_t offset = FindLastByteOffset(descriptor_id, is_descriptor_array, meta.access_chain_insts);
        // If no offset, its dynamic and ignore completly
        if (offset != 0) {
            if (pre_pass) {
                // set offset for the first loop of the block
                auto map_it = block_highest_offset_map_.find(descriptor_id);
                if (map_it == block_highest_offset_map_.end()) {
                    block_highest_offset_map_[descriptor_id] = offset;
                } else {
                    map_it->second = std::max(map_it->second, offset);
                }
            } else {
                const uint32_t block_highest_offset = block_highest_offset_map_[descriptor_id];
                if (offset < block_highest_offset) {
                    return false;  // skipping because other instruction in block will be a higher offset
                }
            }
        }
    }

    // Save information to be used to make the Function
    meta.target_instruction = &inst;

    return true;
}

bool DescriptorClassGeneralBufferPass::EarlySkip() const {
    if (module_.set_index_to_bindings_layout_lut_.empty()) {
        return true;  // If there is no bindings, nothing to instrument
    }
    return false;
}

void DescriptorClassGeneralBufferPass::PrintDebugInfo() const {
    std::cout << "DescriptorClassGeneralBufferPass instrumentation count: " << instrumentations_count_ << '\n';
}

// Created own Instrument() because need to control finding the largest offset in a given block
bool DescriptorClassGeneralBufferPass::Instrument() {
    // Can safely loop function list as there is no injecting of new Functions until linking time
    for (const auto& function : module_.functions_) {
        if (function->instrumentation_added_) continue;
        for (auto block_it = function->blocks_.begin(); block_it != function->blocks_.end(); ++block_it) {
            if ((*block_it)->loop_header_) {
                continue;  // Currently can't properly handle injecting CFG logic into a loop header block
            }
            auto& block_instructions = (*block_it)->instructions_;

            if (unsafe_mode_) {
                // Loop the Block once to get the highest offset
                // Do here before we inject instructions into the block list below
                for (auto inst_it = block_instructions.begin(); inst_it != block_instructions.end(); ++inst_it) {
                    // dummy object - need to be clear for access_chain_insts vector
                    InstructionMeta meta;
                    // Every instruction is analyzed by the specific pass and lets us know if we need to inject a function or not
                    if (!RequiresInstrumentation(*function, *(inst_it->get()), meta, true)) continue;
                }
            }

            for (auto inst_it = block_instructions.begin(); inst_it != block_instructions.end(); ++inst_it) {
                InstructionMeta meta;
                // Every instruction is analyzed by the specific pass and lets us know if we need to inject a function or not
                if (!RequiresInstrumentation(*function, *(inst_it->get()), meta, false)) continue;

                if (module_.settings_.max_instrumentations_count != 0 &&
                    instrumentations_count_ >= module_.settings_.max_instrumentations_count) {
                    return true;  // hit limit
                }
                instrumentations_count_++;

                // Add any debug information to pass into the function call
                InjectionData injection_data;
                injection_data.stage_info_id = GetStageInfo(*function, block_it, inst_it);
                const uint32_t inst_position = meta.target_instruction->GetPositionIndex();
                auto inst_position_constant = module_.type_manager_.CreateConstantUInt32(inst_position);
                injection_data.inst_position_id = inst_position_constant.Id();

                // inst_it is updated to the instruction after the new function call, it will not add/remove any Blocks
                CreateFunctionCall(**block_it, &inst_it, injection_data, meta);
            }
        }
    }

    return instrumentations_count_ != 0;
}

}  // namespace spirv
}  // namespace gpuav