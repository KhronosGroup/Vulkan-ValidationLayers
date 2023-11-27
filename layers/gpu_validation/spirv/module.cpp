/* Copyright (c) 2023 LunarG, Inc.
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

#include "module.h"
#include <spirv/unified1/spirv.hpp>
#include "gpu_shaders/gpu_shaders_constants.h"

#include "ray_tracing_runtime_pass.h"

namespace gpuav {
namespace spirv {

Module::Module(std::vector<uint32_t> words, uint32_t shader_id, uint32_t output_buffer_descriptor_set)
    : shader_id_(shader_id), output_buffer_descriptor_set_(output_buffer_descriptor_set), type_manager_(*this) {
    uint32_t instruction_count = 0;
    std::vector<uint32_t>::const_iterator it = words.cbegin();
    header_.magic_number = *it++;
    header_.version = *it++;
    header_.generator = *it++;
    header_.bound = *it++;
    header_.schema = *it++;
    // Parse everything up until the first function and sort into seperate lists
    while (it != words.cend()) {
        const uint32_t opcode = *it & 0x0ffffu;
        const uint32_t length = *it >> 16;
        if (opcode == spv::OpFunction) {
            break;
        }
        auto new_insn = std::make_unique<Instruction>(it, instruction_count++);

        SpvType spv_type = GetSpvType(new_insn->Opcode());

        if (opcode == spv::OpCapability) {
            capabilities_.emplace_back(std::move(new_insn));
        } else if (opcode == spv::Op::OpExtension) {
            extensions_.emplace_back(std::move(new_insn));
        } else if (opcode == spv::Op::OpExtInstImport) {
            ext_inst_imports_.emplace_back(std::move(new_insn));
        } else if (opcode == spv::Op::OpMemoryModel) {
            memory_model_.emplace_back(std::move(new_insn));
        } else if (opcode == spv::Op::OpEntryPoint) {
            entry_points_.emplace_back(std::move(new_insn));
        } else if (opcode == spv::Op::OpExecutionMode || opcode == spv::Op::OpExecutionModeId) {
            execution_modes_.emplace_back(std::move(new_insn));
        } else if (DebugOperation(opcode)) {
            debug_infos_.emplace_back(std::move(new_insn));
        } else if (AnnotationOperation(opcode)) {
            annotations_.emplace_back(std::move(new_insn));
        } else if (ConstantOperation(opcode)) {
            const Type* type = nullptr;
            switch (opcode) {
                case spv::OpConstantTrue:
                case spv::OpConstantFalse:
                    type = &type_manager_.GetTypeBool();
                    break;
                case spv::OpConstant:
                case spv::OpConstantNull:
                case spv::OpConstantComposite:
                    type = type_manager_.FindTypeById(new_insn->TypeId());
                    break;
                default:
                    assert(false && "unhandled constant");
                    break;
            }
            type_manager_.AddConstant(std::move(new_insn), *type);
        } else if (opcode == spv::OpVariable) {
            types_values_constants_.emplace_back(std::move(new_insn));
        } else if (spv_type != SpvType::Empty) {
            type_manager_.AddType(std::move(new_insn), spv_type);
        } else {
            // unknown instruction, try and just keep in last section to not just crash
            // example: OpSpecConstant
            types_values_constants_.emplace_back(std::move(new_insn));
        }
        it += length;
    }

    // each function is broken up to 3 stage, pre/during/post basic_blocks
    BasicBlock* current_block = nullptr;
    Function* current_function = nullptr;
    bool block_found = false;
    bool function_end_found = false;
    while (it != words.cend()) {
        const uint32_t opcode = *it & 0x0ffffu;
        const uint32_t length = *it >> 16;
        auto new_insn = std::make_unique<Instruction>(it, instruction_count++);

        if (opcode == spv::OpFunctionEnd) {
            function_end_found = true;
        }

        if (opcode == spv::OpFunction) {
            auto new_function = std::make_unique<Function>(*this, std::move(new_insn));
            auto& added_function = functions_.emplace_back(std::move(new_function));
            current_function = &(*added_function);
            block_found = false;
            function_end_found = false;
        } else if (opcode == spv::OpLabel) {
            block_found = true;
            auto new_block = std::make_unique<BasicBlock>(std::move(new_insn));
            auto& added_block = current_function->blocks_.emplace_back(std::move(new_block));
            current_block = &(*added_block);
        } else if (function_end_found) {
            current_function->post_block_inst_.emplace_back(std::move(new_insn));
        } else if (block_found) {
            current_block->instructions_.emplace_back(std::move(new_insn));
        } else {
            current_function->pre_block_inst_.emplace_back(std::move(new_insn));
        }

        it += length;
    }
}

void Module::RunPassRayTracingRuntime() {
    RayTracingRuntimePass pass(*this);
    pass.Run();
}

uint32_t Module::TakeNextId() {
    // SPIR-V limit.
    static constexpr uint32_t kDefaultMaxIdBound = 0x3FFFFF;
    assert(header_.bound < kDefaultMaxIdBound);
    return header_.bound++;
}

// walk through each list and append the buffer
void Module::ToBinary(std::vector<uint32_t>& out) {
    out.clear();
    out.push_back(header_.magic_number);
    out.push_back(header_.version);
    out.push_back(header_.generator);
    out.push_back(header_.bound);
    out.push_back(header_.schema);

    for (const auto& inst : capabilities_) {
        inst->ToBinary(out);
    }
    for (const auto& inst : extensions_) {
        inst->ToBinary(out);
    }
    for (const auto& inst : ext_inst_imports_) {
        inst->ToBinary(out);
    }
    for (const auto& inst : memory_model_) {
        inst->ToBinary(out);
    }
    for (const auto& inst : entry_points_) {
        inst->ToBinary(out);
    }
    for (const auto& inst : execution_modes_) {
        inst->ToBinary(out);
    }
    for (const auto& inst : debug_infos_) {
        inst->ToBinary(out);
    }
    for (const auto& inst : annotations_) {
        inst->ToBinary(out);
    }
    for (const auto& inst : types_values_constants_) {
        inst->ToBinary(out);
    }
    for (const auto& function : functions_) {
        function->ToBinary(out);
    }
}

// Takes the current module and injects the function into it
// This is done by first apply any new Types/Constants/Variables and then copying in the instructions of the Function
void Module::LinkFunction(const LinkInfo& info) {
    // track the incoming SSA IDs with what they are in the module
    vvl::unordered_map<uint32_t, uint32_t> id_swap_map;
    const uint32_t function_type_id = TakeNextId();

    // Track all decorations and add after when have full id_swap_map
    InstructionList decorations;

    // We need to apply variable to the Entry Point interface if using SPIR-V 1.4+
    uint32_t storage_buffer_variable_id = 0;

    // find all constant and types, add any the module doesn't have
    uint32_t offset = 5;  // skip header
    while (offset < info.word_count) {
        const uint32_t* insn_word = &info.words[offset];
        const uint32_t opcode = *insn_word & 0x0ffffu;
        const uint32_t length = *insn_word >> 16;
        if (opcode == spv::OpFunction) {
            break;
        }

        auto new_insn = std::make_unique<Instruction>(insn_word);
        const uint32_t old_result_id = new_insn->ResultId();

        SpvType spv_type = GetSpvType(opcode);
        if (spv_type != SpvType::Empty) {
            // will find (or create if not found) the matching OpType
            uint32_t type_id = 0;
            switch (spv_type) {
                case SpvType::kVoid:
                    type_id = type_manager_.GetTypeVoid().Id();
                    break;
                case SpvType::kBool:
                    type_id = type_manager_.GetTypeBool().Id();
                    break;
                case SpvType::kSampler:
                    type_id = type_manager_.GetTypeSampler().Id();
                    break;
                case SpvType::kRayQueryKHR:
                    type_id = type_manager_.GetTypeRayQuery().Id();
                    break;
                case SpvType::kAccelerationStructureKHR:
                    type_id = type_manager_.GetTypeAccelerationStructure().Id();
                    break;
                case SpvType::kInt: {
                    uint32_t bit_width = new_insn->Word(2);
                    bool is_signed = new_insn->Word(3) != 0;
                    type_id = type_manager_.GetTypeInt(bit_width, is_signed).Id();
                    break;
                }
                case SpvType::kFloat: {
                    uint32_t bit_width = new_insn->Word(2);
                    type_id = type_manager_.GetTypeFloat(bit_width).Id();
                    break;
                }
                case SpvType::kArray: {
                    const Type* element_type = type_manager_.FindTypeById(id_swap_map[new_insn->Word(2)]);
                    const Type* length_type = type_manager_.FindTypeById(id_swap_map[new_insn->Word(3)]);
                    type_id = type_manager_.GetTypeArray(*element_type, *length_type).Id();
                    break;
                }
                case SpvType::kRuntimeArray: {
                    const Type* element_type = type_manager_.FindTypeById(id_swap_map[new_insn->Word(2)]);
                    type_id = type_manager_.GetTypeRuntimeArray(*element_type).Id();
                    break;
                }
                case SpvType::kVector: {
                    const Type* component_type = type_manager_.FindTypeById(id_swap_map[new_insn->Word(2)]);
                    uint32_t component_count = new_insn->Word(3);
                    type_id = type_manager_.GetTypeVector(*component_type, component_count).Id();
                    break;
                }
                case SpvType::kMatrix: {
                    const Type* column_type = type_manager_.FindTypeById(id_swap_map[new_insn->Word(2)]);
                    uint32_t column_count = new_insn->Word(3);
                    type_id = type_manager_.GetTypeMatrix(*column_type, column_count).Id();
                    break;
                }
                case SpvType::kSampledImage: {
                    const Type* image_type = type_manager_.FindTypeById(id_swap_map[new_insn->Word(2)]);
                    type_id = type_manager_.GetTypeSampledImage(*image_type).Id();
                    break;
                }
                case SpvType::kPointer: {
                    spv::StorageClass storage_class = spv::StorageClass(new_insn->Word(2));
                    const Type* pointer_type = type_manager_.FindTypeById(id_swap_map[new_insn->Word(3)]);
                    type_id = type_manager_.GetTypePointer(storage_class, *pointer_type).Id();
                    break;
                }
                case SpvType::kStruct:
                case SpvType::kFunction: {
                    // For OpTypeStruct, we just add it regardless since low chance to find for the amount of time to search all
                    // struct (which there can be quite a bit of) For OpTypeFunction, we will only have one and custom function
                    // likely won't match anything neither
                    const uint32_t new_result_id = (spv_type == SpvType::kFunction) ? function_type_id : TakeNextId();
                    new_insn->ReplaceResultId(new_result_id);
                    new_insn->ReplaceLinkedId(id_swap_map);
                    type_id = type_manager_.AddType(std::move(new_insn), spv_type).Id();
                    break;
                }
                default:
                    break;
            }

            id_swap_map[old_result_id] = type_id;

        } else if (ConstantOperation(opcode)) {
            const Type& type = *type_manager_.FindTypeById(id_swap_map[new_insn->TypeId()]);
            const Constant* constant = nullptr;
            // for simplicity, just create a new constant for things other than 32-bit OpConstant as there are rarely-to-none
            // composite/null/true/false constants in linked functions. The extra logic to try and find them is much larger and cost
            // time failing most the searches.
            if (opcode == spv::OpConstant) {
                const uint32_t constant_value = new_insn->Word(3);
                if (type.insn_.Opcode() == spv::OpTypeInt && type.insn_.Word(2) == 32) {
                    constant = type_manager_.FindConstantInt32(type.Id(), constant_value);
                } else if (type.insn_.Opcode() == spv::OpTypeFloat && type.insn_.Word(2) == 32) {
                    constant = type_manager_.FindConstantFloat32(type.Id(), constant_value);
                }

                // Replace LinkConstants
                if (constant_value == gpuav::glsl::kLinkShaderId) {
                    new_insn->words_[3] = shader_id_;
                }
            }

            if (!constant) {
                const uint32_t new_result_id = TakeNextId();
                new_insn->ReplaceResultId(new_result_id);
                new_insn->ReplaceLinkedId(id_swap_map);
                constant = &type_manager_.AddConstant(std::move(new_insn), type);
            }
            id_swap_map[old_result_id] = constant->Id();
        } else if (opcode == spv::OpVariable) {
            // Add in all variables outside of functions
            const uint32_t new_result_id = TakeNextId();
            id_swap_map[old_result_id] = new_result_id;
            new_insn->ReplaceResultId(new_result_id);
            new_insn->ReplaceLinkedId(id_swap_map);
            types_values_constants_.push_back(std::move(new_insn));

            assert(storage_buffer_variable_id == 0);  // only should have 1 OpVariable here
            storage_buffer_variable_id = new_result_id;
        } else if (opcode == spv::OpDecorate || opcode == spv::OpMemberDecorate) {
            decorations.push_back(std::make_unique<Instruction>(insn_word));
        }

        offset += length;
    }

    // because flow-control instructions (ex. OpBranch) do forward references to IDs, do an initial loop to get all OpLabel to have
    // in id_swap_map
    uint32_t offset_copy = offset;
    while (offset_copy < info.word_count) {
        const uint32_t* insn_word = &info.words[offset_copy];
        const uint32_t opcode = *insn_word & 0x0ffffu;
        const uint32_t length = *insn_word >> 16;
        if (opcode == spv::OpLabel) {
            Instruction insn(insn_word);
            uint32_t new_result_id = TakeNextId();
            id_swap_map[insn.ResultId()] = new_result_id;
        }
        offset_copy += length;
    }

    // Add function and copy all instructions to it, while adjusting any IDs
    auto& new_function = functions_.emplace_back(std::make_unique<Function>(*this));
    while (offset < info.word_count) {
        const uint32_t* insn_word = &info.words[offset];
        auto new_insn = std::make_unique<Instruction>(insn_word);
        const uint32_t opcode = new_insn->Opcode();
        const uint32_t length = new_insn->Length();

        if (opcode == spv::OpFunction) {
            new_insn->words_[1] = id_swap_map[new_insn->words_[1]];
            new_insn->words_[2] = info.function_id;
            new_insn->words_[4] = function_type_id;
        } else if (opcode == spv::OpLabel) {
            uint32_t new_result_id = id_swap_map[new_insn->ResultId()];
            new_insn->ReplaceResultId(new_result_id);
        } else {
            uint32_t result_id = new_insn->ResultId();
            if (result_id != 0) {
                uint32_t new_result_id = TakeNextId();
                id_swap_map[result_id] = new_result_id;
                new_insn->ReplaceResultId(new_result_id);
            }
            new_insn->ReplaceLinkedId(id_swap_map);
        }

        // To make simpler, just put everything in a single list as we have no need to do any modifications to the CFG logic for the
        // linked function
        new_function->pre_block_inst_.emplace_back(std::move(new_insn));
        offset += length;
    }

    // Apply decorations
    for (auto& decoration : decorations) {
        if (decoration->Word(2) == spv::DecorationLinkageAttributes) {
            continue;  // remove linkage info
        } else if (decoration->Word(2) == spv::DecorationDescriptorSet) {
            // only should be one DescriptorSet to update
            decoration->words_[3] = output_buffer_descriptor_set_;
        }
        decoration->ReplaceLinkedId(id_swap_map);
        annotations_.push_back(std::move(decoration));
    }

    // Update entrypoint interface if 1.4+
    const uint32_t spirv_version_1_4 = 0x00010400;
    if (header_.version >= spirv_version_1_4) {
        // Currently just apply to all Entrypoint as it should be ok to have a global variable in there even if it can't dynamically
        // touch the new function
        for (auto& entry_point : entry_points_) {
            entry_point->AppendWord(storage_buffer_variable_id);
        }
    }
}

}  // namespace spirv
}  // namespace gpuav