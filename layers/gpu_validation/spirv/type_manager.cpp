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

#include "type_manager.h"
#include "module.h"

namespace gpuav {
namespace spirv {

// Simplest way to check if same type is see if items line up.
// Even if types have an RefId, it should be the same unless there are already duplicated types.
bool Type::operator==(Type const& other) const {
    if ((spv_type_ != other.spv_type_) || (insn_.Length() != other.insn_.Length())) {
        return false;
    }
    // word[1] is the result ID which might be different
    for (uint32_t i = 2; i < insn_.Length(); i++) {
        if (insn_.words_[i] != other.insn_.words_[i]) {
            return false;
        }
    }
    return true;
}

// return %A in:
//   %B = OpTypePointer Input %A
//   %C = OpVariable %B Input
const Type* Variable::PointerType(TypeManager& type_manager_) const {
    assert(type_.insn_.Opcode() == spv::OpTypePointer);
    uint32_t type_id = type_.insn_.Word(3);
    return type_manager_.FindTypeById(type_id);
}

const Type& TypeManager::AddType(std::unique_ptr<Instruction> new_insn, SpvType spv_type) {
    const auto& insn = module_.types_values_constants_.emplace_back(std::move(new_insn));

    const Type& new_type = types_.emplace_back(spv_type, *insn);
    id_to_type_[insn->ResultId()] = &new_type;

    switch (spv_type) {
        case SpvType::kVoid:
            void_type = &new_type;
            break;
        case SpvType::kBool:
            bool_type = &new_type;
            break;
        case SpvType::kSampler:
            sampler_type = &new_type;
            break;
        case SpvType::kRayQueryKHR:
            ray_query_type = &new_type;
            break;
        case SpvType::kAccelerationStructureKHR:
            acceleration_structure_type = &new_type;
            break;
        case SpvType::kInt:
            int_types_.push_back(&new_type);
            break;
        case SpvType::kFloat:
            float_types_.push_back(&new_type);
            break;
        case SpvType::kVector:
            vector_types_.push_back(&new_type);
            break;
        case SpvType::kMatrix:
            matrix_types_.push_back(&new_type);
            break;
        case SpvType::kImage:
            image_types_.push_back(&new_type);
            break;
        case SpvType::kSampledImage:
            sampled_image_types_.push_back(&new_type);
            break;
        case SpvType::kArray:
            array_types_.push_back(&new_type);
            break;
        case SpvType::kRuntimeArray:
            runtime_array_types_.push_back(&new_type);
            break;
        case SpvType::kPointer:
            pointer_types_.push_back(&new_type);
            break;
        case SpvType::kForwardPointer:
            forward_pointer_types_.push_back(&new_type);
            break;
        case SpvType::kFunction:
            function_types_.push_back(&new_type);
            break;
        case SpvType::kStruct:
            break;  // don't track structs currently
        default:
            assert(false && "unsupported SpvType");
            break;
    }

    return new_type;
}

const Type* TypeManager::FindTypeById(uint32_t id) const {
    auto type = id_to_type_.find(id);
    return (type == id_to_type_.end()) ? nullptr : type->second;
}

const Type& TypeManager::GetTypeVoid() {
    if (void_type) {
        return *void_type;
    };

    const uint32_t type_id = module_.TakeNextId();
    auto new_insn = std::make_unique<Instruction>(2, spv::OpTypeVoid);
    new_insn->Fill({type_id});
    return AddType(std::move(new_insn), SpvType::kVoid);
}

const Type& TypeManager::GetTypeBool() {
    if (bool_type) {
        return *bool_type;
    };

    const uint32_t type_id = module_.TakeNextId();
    auto new_insn = std::make_unique<Instruction>(2, spv::OpTypeBool);
    new_insn->Fill({type_id});
    return AddType(std::move(new_insn), SpvType::kBool);
}

const Type& TypeManager::GetTypeSampler() {
    if (sampler_type) {
        return *sampler_type;
    }

    const uint32_t type_id = module_.TakeNextId();
    auto new_insn = std::make_unique<Instruction>(2, spv::OpTypeSampler);
    new_insn->Fill({type_id});
    return AddType(std::move(new_insn), SpvType::kSampler);
}

const Type& TypeManager::GetTypeRayQuery() {
    if (ray_query_type) {
        return *ray_query_type;
    }

    const uint32_t type_id = module_.TakeNextId();
    auto new_insn = std::make_unique<Instruction>(2, spv::OpTypeRayQueryKHR);
    new_insn->Fill({type_id});
    return AddType(std::move(new_insn), SpvType::kRayQueryKHR);
}

const Type& TypeManager::GetTypeAccelerationStructure() {
    if (acceleration_structure_type) {
        return *acceleration_structure_type;
    }

    const uint32_t type_id = module_.TakeNextId();
    auto new_insn = std::make_unique<Instruction>(2, spv::OpTypeAccelerationStructureKHR);
    new_insn->Fill({type_id});
    return AddType(std::move(new_insn), SpvType::kAccelerationStructureKHR);
}

const Type& TypeManager::GetTypeInt(uint32_t bit_width, bool is_signed) {
    for (const auto type : int_types_) {
        const auto& words = type->insn_.words_;
        const bool int_is_signed = words[3] != 0;
        if (words[2] == bit_width && int_is_signed == is_signed) {
            return *type;
        }
    }

    const uint32_t type_id = module_.TakeNextId();
    const uint32_t signed_word = is_signed ? 1 : 0;
    auto new_insn = std::make_unique<Instruction>(4, spv::OpTypeInt);
    new_insn->Fill({type_id, bit_width, signed_word});
    return AddType(std::move(new_insn), SpvType::kInt);
}

const Type& TypeManager::GetTypeFloat(uint32_t bit_width) {
    for (const auto type : float_types_) {
        const auto& words = type->insn_.words_;
        if ((words[2] == bit_width)) {
            return *type;
        }
    }

    const uint32_t type_id = module_.TakeNextId();
    auto new_insn = std::make_unique<Instruction>(3, spv::OpTypeFloat);
    new_insn->Fill({type_id, bit_width});
    return AddType(std::move(new_insn), SpvType::kFloat);
}

const Type& TypeManager::GetTypeArray(const Type& element_type, const Type& length_type) {
    for (const auto type : runtime_array_types_) {
        const Type* this_element_type = FindTypeById(type->insn_.Word(2));
        if (this_element_type && (*this_element_type == element_type)) {
            const Type* this_length_type = FindTypeById(type->insn_.Word(3));
            if (this_length_type && (*this_length_type == length_type)) {
                return *type;
            }
        }
    }

    const uint32_t type_id = module_.TakeNextId();
    auto new_insn = std::make_unique<Instruction>(4, spv::OpTypeArray);
    new_insn->Fill({type_id, element_type.Id(), length_type.Id()});
    return AddType(std::move(new_insn), SpvType::kArray);
}

const Type& TypeManager::GetTypeRuntimeArray(const Type& element_type) {
    for (const auto type : runtime_array_types_) {
        const Type* this_element_type = FindTypeById(type->insn_.Word(2));
        if (this_element_type && (*this_element_type == element_type)) {
            return *type;
        }
    }

    const uint32_t type_id = module_.TakeNextId();
    auto new_insn = std::make_unique<Instruction>(3, spv::OpTypeRuntimeArray);
    new_insn->Fill({type_id, element_type.Id()});
    return AddType(std::move(new_insn), SpvType::kRuntimeArray);
}

const Type& TypeManager::GetTypeVector(const Type& component_type, uint32_t component_count) {
    for (const auto type : vector_types_) {
        const auto& words = type->insn_.words_;
        if (words[3] != component_count) {
            continue;
        }

        const Type* vector_component_type = FindTypeById(words[2]);
        if (vector_component_type && (*vector_component_type == component_type)) {
            return *type;
        }
    }

    const uint32_t type_id = module_.TakeNextId();
    auto new_insn = std::make_unique<Instruction>(4, spv::OpTypeVector);
    new_insn->Fill({type_id, component_type.Id(), component_count});
    return AddType(std::move(new_insn), SpvType::kVector);
}

const Type& TypeManager::GetTypeMatrix(const Type& column_type, uint32_t column_count) {
    for (const auto type : matrix_types_) {
        const auto& words = type->insn_.words_;
        if (words[3] != column_count) {
            continue;
        }

        const Type* matrix_column_type = FindTypeById(words[2]);
        if (matrix_column_type && (*matrix_column_type == column_type)) {
            return *type;
        }
    }

    const uint32_t type_id = module_.TakeNextId();
    auto new_insn = std::make_unique<Instruction>(4, spv::OpTypeMatrix);
    new_insn->Fill({type_id, column_type.Id(), column_count});
    return AddType(std::move(new_insn), SpvType::kMatrix);
}

const Type& TypeManager::GetTypeSampledImage(const Type& image_type) {
    for (const auto type : sampled_image_types_) {
        const auto& words = type->insn_.words_;
        const Type* this_image_type = FindTypeById(words[2]);
        if (this_image_type && (*this_image_type == image_type)) {
            return *type;
        }
    }

    const uint32_t type_id = module_.TakeNextId();
    auto new_insn = std::make_unique<Instruction>(3, spv::OpTypeSampledImage);
    new_insn->Fill({type_id, image_type.Id()});
    return AddType(std::move(new_insn), SpvType::kSampledImage);
}

const Type& TypeManager::GetTypePointer(spv::StorageClass storage_class, const Type& pointer_type) {
    for (const auto type : pointer_types_) {
        const auto& words = type->insn_.words_;
        if (words[2] != storage_class) {
            continue;
        }

        const Type* this_pointer_type = FindTypeById(words[3]);
        if (this_pointer_type && (*this_pointer_type == pointer_type)) {
            return *type;
        }
    }

    const uint32_t type_id = module_.TakeNextId();
    auto new_insn = std::make_unique<Instruction>(4, spv::OpTypePointer);
    new_insn->Fill({type_id, uint32_t(storage_class), pointer_type.Id()});
    return AddType(std::move(new_insn), SpvType::kPointer);
}

const Type& TypeManager::GetTypePointerBuiltInInput(spv::BuiltIn built_in) {
    switch (built_in) {
        case spv::BuiltInFragCoord: {
            const Type& float_32 = GetTypeFloat(32);
            const Type& vec4 = GetTypeVector(float_32, 4);
            return GetTypePointer(spv::StorageClassInput, vec4);
        }
        case spv::BuiltInVertexIndex:
        case spv::BuiltInInstanceIndex:
        case spv::BuiltInPrimitiveId:
        case spv::BuiltInInvocationId:
        case spv::BuiltInSubgroupLocalInvocationId: {
            const Type& uint_32 = GetTypeInt(32, false);
            return GetTypePointer(spv::StorageClassInput, uint_32);
        }
        case spv::BuiltInGlobalInvocationId:
        case spv::BuiltInLaunchIdKHR: {
            const Type& uint_32 = GetTypeInt(32, false);
            const Type& vec3 = GetTypeVector(uint_32, 3);
            return GetTypePointer(spv::StorageClassInput, vec3);
        }
        case spv::BuiltInTessCoord: {
            const Type& float_32 = GetTypeFloat(32);
            const Type& vec3 = GetTypeVector(float_32, 3);
            return GetTypePointer(spv::StorageClassInput, vec3);
        }
        case spv::BuiltInSubgroupLtMask: {
            const Type& uint_32 = GetTypeInt(32, false);
            const Type& vec4 = GetTypeVector(uint_32, 4);
            return GetTypePointer(spv::StorageClassInput, vec4);
        }
        default: {
            assert(false && "unhandled builtin");
            return types_.front();
        }
    }
}

const Constant& TypeManager::AddConstant(std::unique_ptr<Instruction> new_insn, const Type& type) {
    const auto& insn = module_.types_values_constants_.emplace_back(std::move(new_insn));

    const Constant& new_constant = constants_.emplace_back(type, *insn);
    id_to_constant_[insn->ResultId()] = &new_constant;

    if (insn->Opcode() == spv::OpConstant) {
        if (type.insn_.Opcode() == spv::OpTypeInt && type.insn_.Word(2) == 32) {
            int_32bit_constants_.push_back(&new_constant);
        } else if (type.insn_.Opcode() == spv::OpTypeFloat && type.insn_.Word(2) == 32) {
            float_32bit_constants_.push_back(&new_constant);
        }
    }

    return new_constant;
}

const Constant* TypeManager::FindConstantInt32(uint32_t type_id, uint32_t value) const {
    for (const auto constant : int_32bit_constants_) {
        if (constant->type_.Id() == type_id && value == constant->insn_.Word(3)) {
            return constant;
        }
    }
    return nullptr;
}

const Constant* TypeManager::FindConstantFloat32(uint32_t type_id, uint32_t value) const {
    for (const auto constant : float_32bit_constants_) {
        if (constant->type_.Id() == type_id && value == constant->insn_.Word(3)) {
            return constant;
        }
    }
    return nullptr;
}

const Constant* TypeManager::FindConstantById(uint32_t id) const {
    auto constant = id_to_constant_.find(id);
    return (constant == id_to_constant_.end()) ? nullptr : constant->second;
}

const Constant& TypeManager::CreateConstantUInt32(uint32_t value) {
    const Type& type = GetTypeInt(32, 0);
    const uint32_t type_id = module_.TakeNextId();
    auto new_insn = std::make_unique<Instruction>(4, spv::OpConstant);
    new_insn->Fill({type.Id(), type_id, value});
    return AddConstant(std::move(new_insn), type);
}

// It is common to use uint32_t(0) as a default, so having it cached is helpful
const Constant& TypeManager::GetConstantZeroUint32() {
    if (!uint_32bit_zero_constants_) {
        const Type& uint32_type = GetTypeInt(32, 0);
        uint_32bit_zero_constants_ = FindConstantInt32(uint32_type.Id(), 0);
        if (!uint_32bit_zero_constants_) {
            uint_32bit_zero_constants_ = &CreateConstantUInt32(0);
        }
    }
    return *uint_32bit_zero_constants_;
}

const Variable& TypeManager::AddVariable(std::unique_ptr<Instruction> new_insn, const Type& type) {
    const auto& insn = module_.types_values_constants_.emplace_back(std::move(new_insn));

    const Variable& new_variable = variables_.emplace_back(type, *insn);
    id_to_variable_[insn->ResultId()] = &new_variable;

    if (new_variable.StorageClass() == spv::StorageClassInput) {
        input_variables_.push_back(&new_variable);
    } else if (new_variable.StorageClass() == spv::StorageClassOutput) {
        output_variables_.push_back(&new_variable);
    }

    return new_variable;
}

const Variable* TypeManager::FindVariableById(uint32_t id) const {
    auto variable = id_to_variable_.find(id);
    return (variable == id_to_variable_.end()) ? nullptr : variable->second;
}

}  // namespace spirv
}  // namespace gpuav