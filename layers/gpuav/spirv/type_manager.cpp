/* Copyright (c) 2024-2026 LunarG, Inc.
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
#include <vulkan/vulkan_core.h>
#include <cassert>
#include <cstdint>
#include <spirv/unified1/spirv.hpp>
#include "containers/container_utils.h"
#include "generated/spirv_grammar_helper.h"
#include "gpuav/descriptor_validation/gpuav_descriptor_validation.h"
#include "module.h"

namespace gpuav {
namespace spirv {

// Simplest way to check if same type is see if items line up.
// Even if types have an RefId, it should be the same unless there are already duplicated types.
bool Type::operator==(Type const& other) const {
    if ((spv_type_ != other.spv_type_) || (inst_.Length() != other.inst_.Length())) {
        return false;
    }
    // word[1] is the result ID which might be different
    for (uint32_t i = 2; i < inst_.Length(); i++) {
        if (inst_.Word(i) != other.inst_.Word(i)) {
            return false;
        }
    }
    return true;
}

DescriptorInterface Variable::FindDescriptorInterface(const Module& module, const Instruction& inst) {
    DescriptorInterface descriptor;
    // Only allowed descriptor storage classes
    // https://docs.vulkan.org/spec/latest/chapters/interfaces.html#interfaces-resources-storage-class-correspondence
    if (IsValueIn(inst.StorageClass(), {
                                           spv::StorageClassUniform,
                                           spv::StorageClassUniformConstant,
                                           spv::StorageClassStorageBuffer,
                                           spv::StorageClassTileAttachmentQCOM,
                                       })) {
        const uint32_t variable_id = inst.ResultId();
        for (const auto& annotation : module.annotations_) {
            if (annotation->Opcode() == spv::OpDecorate && annotation->Word(1) == variable_id) {
                if (annotation->Word(2) == spv::DecorationDescriptorSet) {
                    descriptor.set = annotation->Word(3);
                } else if (annotation->Word(2) == spv::DecorationBinding) {
                    descriptor.binding = annotation->Word(3);
                } else if (annotation->Word(2) == spv::DecorationBuiltIn && annotation->Word(3) == spv::BuiltInResourceHeapEXT) {
                    descriptor.is_resource_heap = true;
                } else if (annotation->Word(2) == spv::DecorationBuiltIn && annotation->Word(3) == spv::BuiltInSamplerHeapEXT) {
                    descriptor.is_sampler_heap = true;
                }
            }
        }
    }
    return descriptor;
}

// return %A in:
//   %B = OpTypePointer Input %A
//   %C = OpVariable %B Input
const Type* Variable::PointerType(const TypeManager& type_manager_) const {
    // If we are hitting kUntypedPointer, the logic need to get the type info either from the Base Type of the UntypedAccessChain or
    // if it cares about the type, it can find it at the access type (example to help show how to getting the type
    // https://godbolt.org/z/ejf1TGx8Y)
    assert(type_.spv_type_ == SpvType::kPointer || type_.spv_type_ == SpvType::kForwardPointer);
    uint32_t type_id = type_.inst_.Word(3);
    return type_manager_.FindTypeById(type_id);
}

const Type& TypeManager::AddType(std::unique_ptr<Instruction> new_inst, SpvType spv_type) {
    const auto& inst = module_.types_values_constants_.emplace_back(std::move(new_inst));

    id_to_type_[inst->ResultId()] = std::make_unique<Type>(spv_type, *inst, *this);
    const Type* new_type = id_to_type_[inst->ResultId()].get();

    switch (spv_type) {
        case SpvType::kVoid:
            void_type = new_type;
            break;
        case SpvType::kBool:
            bool_type = new_type;
            break;
        case SpvType::kSampler:
            sampler_type = new_type;
            break;
        case SpvType::kRayQueryKHR:
            ray_query_type = new_type;
            break;
        case SpvType::kAccelerationStructureKHR:
            acceleration_structure_type = new_type;
            break;
        case SpvType::kInt:
            int_types_.push_back(new_type);
            break;
        case SpvType::kFloat:
            float_types_.push_back(new_type);
            break;
        case SpvType::kVector:
            vector_types_.push_back(new_type);
            break;
        case SpvType::kMatrix:
            matrix_types_.push_back(new_type);
            break;
        case SpvType::kImage:
            image_types_.push_back(new_type);
            break;
        case SpvType::kSampledImage:
            sampled_image_types_.push_back(new_type);
            break;
        case SpvType::kArray:
            array_types_.push_back(new_type);
            break;
        case SpvType::kRuntimeArray:
            runtime_array_types_.push_back(new_type);
            break;
        case SpvType::kPointer:
            pointer_types_.push_back(new_type);
            break;
        case SpvType::kForwardPointer:
            forward_pointer_types_.push_back(new_type);
            break;
        case SpvType::kFunction:
            function_types_.push_back(new_type);
            break;
        case SpvType::kCooperativeMatrixKHR:
            coop_mat_types_.push_back(new_type);
            break;
        case SpvType::kStruct:
            break;  // don't track structs currently
        case SpvType::kVectorIdEXT:
            break;  // don't track coopvec currently
        case SpvType::kHitObjectNV:
        case SpvType::kHitObjectEXT:
            break;  // don't track hit objects currently
        case SpvType::kBufferEXT:
            break;
        case SpvType::kUntypedPointerKHR:
            break;
        case SpvType::kTensorLayoutNV:
        case SpvType::kTensorViewNV:
            break;
        default:
            assert(false && "unsupported SpvType");
            break;
    }

    return *new_type;
}

// We don't want to waste time trying to look up potential recursive struct type
// This is added for those we want to spend time to not duplicate and link with.
// We also will hit spirv-val errors if using 2 OpTypeStruct, even if same internals
// (https://gitlab.khronos.org/spirv/SPIR-V/-/issues/918)
void TypeManager::AddStructTypeForLinking(const Type* new_type) {
    assert(new_type && new_type->spv_type_ == SpvType::kStruct);
    linking_struct_types_.push_back(new_type);
}

uint32_t TypeManager::FindLinkingStructType(const Instruction& inst, vvl::unordered_map<uint32_t, uint32_t>& id_swap_map) const {
    for (const auto& struct_type : linking_struct_types_) {
        if (struct_type->inst_.Length() != inst.Length()) continue;
        // Assume currently structs are not nested and only need to examine one level
        const uint32_t length = inst.Length();
        bool found = true;
        for (uint32_t i = 2; i < length; i++) {
            const Type* type_a = FindTypeById(struct_type->inst_.Word(i));
            const Type* type_b = FindTypeById(id_swap_map[inst.Word(i)]);
            if (!type_a || !type_b || type_a->Id() != type_b->Id()) {
                found = false;
                break;
            }
        }
        if (found) {
            return struct_type->Id();
        }
    }
    return 0;
}

const Type* TypeManager::FindTypeById(uint32_t id) const {
    auto type = id_to_type_.find(id);
    return (type == id_to_type_.end()) ? nullptr : type->second.get();
}

const Type* TypeManager::FindFunctionType(const Instruction& inst) const {
    const uint32_t inst_length = inst.Length();
    for (const auto& type : function_types_) {
        if (type->inst_.Length() != inst_length) {
            continue;
        }
        // Start at the Result Type ID (skip ResultID and the base word)
        bool found = true;
        for (uint32_t i = 2; i < inst_length; i++) {
            if (type->inst_.Word(i) != inst.Word(i)) {
                found = false;
                break;
            }
        }
        if (found) {
            return type;
        }
    }
    return nullptr;
}

// ONLY USE IF NEEDED!
// The goal of Function::FindInstruction is you should know the instruction is in the Function.
// For walking the indexes of an access chain in this pass, we want a global lookup
// This manily happens because the value of the index could be both a OpConstant value or loaded in the function.
const Type* TypeManager::FindTypeGlobal(const Function& function, uint32_t id) const {
    if (auto ret = function.FindInstruction(id)) {
        return FindTypeById(ret->TypeId());
    }
    if (auto ret = module_.type_manager_.FindConstantById(id)) {
        return &ret->type_;
    }
    if (auto ret = module_.type_manager_.FindVariableById(id)) {
        return &ret->type_;
    }
    return nullptr;
}

const Type& TypeManager::GetTypeVoid() {
    if (void_type) {
        return *void_type;
    };

    const uint32_t type_id = module_.TakeNextId();
    auto new_inst = std::make_unique<Instruction>(2, spv::OpTypeVoid);
    new_inst->Fill({type_id});
    return AddType(std::move(new_inst), SpvType::kVoid);
}

const Type& TypeManager::GetTypeBool() {
    if (bool_type) {
        return *bool_type;
    };

    const uint32_t type_id = module_.TakeNextId();
    auto new_inst = std::make_unique<Instruction>(2, spv::OpTypeBool);
    new_inst->Fill({type_id});
    return AddType(std::move(new_inst), SpvType::kBool);
}

const Type& TypeManager::GetTypeSampler() {
    if (sampler_type) {
        return *sampler_type;
    }

    const uint32_t type_id = module_.TakeNextId();
    auto new_inst = std::make_unique<Instruction>(2, spv::OpTypeSampler);
    new_inst->Fill({type_id});
    return AddType(std::move(new_inst), SpvType::kSampler);
}

const Type& TypeManager::GetTypeRayQuery() {
    if (ray_query_type) {
        return *ray_query_type;
    }

    const uint32_t type_id = module_.TakeNextId();
    auto new_inst = std::make_unique<Instruction>(2, spv::OpTypeRayQueryKHR);
    new_inst->Fill({type_id});
    return AddType(std::move(new_inst), SpvType::kRayQueryKHR);
}

const Type& TypeManager::GetTypeAccelerationStructure() {
    if (acceleration_structure_type) {
        return *acceleration_structure_type;
    }

    const uint32_t type_id = module_.TakeNextId();
    auto new_inst = std::make_unique<Instruction>(2, spv::OpTypeAccelerationStructureKHR);
    new_inst->Fill({type_id});
    return AddType(std::move(new_inst), SpvType::kAccelerationStructureKHR);
}

const Type& TypeManager::GetTypeInt(uint32_t bit_width, bool is_signed) {
    for (const auto type : int_types_) {
        if (type->meta_.scalar.bit_width == bit_width && type->meta_.scalar.is_signed == is_signed) {
            return *type;
        }
    }

    const uint32_t type_id = module_.TakeNextId();
    const uint32_t signed_word = is_signed ? 1 : 0;
    auto new_inst = std::make_unique<Instruction>(4, spv::OpTypeInt);
    new_inst->Fill({type_id, bit_width, signed_word});
    return AddType(std::move(new_inst), SpvType::kInt);
}

const Type& TypeManager::GetTypeFloat(uint32_t bit_width) {
    for (const auto type : float_types_) {
        if (type->meta_.scalar.bit_width == bit_width) {
            return *type;
        }
    }

    const uint32_t type_id = module_.TakeNextId();
    auto new_inst = std::make_unique<Instruction>(3, spv::OpTypeFloat);
    new_inst->Fill({type_id, bit_width});
    return AddType(std::move(new_inst), SpvType::kFloat);
}

const Type& TypeManager::GetTypeArray(const Type& element_type, const Constant& length, bool get_explicit_layout) {
    if (get_explicit_layout || !IsExplicitLayoutType(element_type)) {
        for (const auto type : array_types_) {
            const Type* this_element_type = FindTypeById(type->inst_.Word(2));
            if (this_element_type && (*this_element_type == element_type)) {
                if (type->inst_.Word(3) == length.Id()) {
                    return *type;
                }
            }
        }
    }

    const uint32_t type_id = module_.TakeNextId();
    auto new_inst = std::make_unique<Instruction>(4, spv::OpTypeArray);
    new_inst->Fill({type_id, element_type.Id(), length.Id()});
    return AddType(std::move(new_inst), SpvType::kArray);
}

const Type& TypeManager::GetTypeRuntimeArray(const Type& element_type, bool get_explicit_layout) {
    if (get_explicit_layout || !IsExplicitLayoutType(element_type)) {
        for (const auto type : runtime_array_types_) {
            const Type* this_element_type = FindTypeById(type->inst_.Word(2));
            if (this_element_type && (*this_element_type == element_type)) {
                return *type;
            }
        }
    }

    const uint32_t type_id = module_.TakeNextId();
    auto new_inst = std::make_unique<Instruction>(3, spv::OpTypeRuntimeArray);
    new_inst->Fill({type_id, element_type.Id()});
    return AddType(std::move(new_inst), SpvType::kRuntimeArray);
}

const Type& TypeManager::GetTypeVector(const Type& component_type, uint32_t component_count) {
    for (const auto type : vector_types_) {
        if (type->meta_.vector.component_count != component_count) {
            continue;
        }

        const Type* vector_component_type = FindTypeById(type->inst_.Word(2));
        if (vector_component_type && (*vector_component_type == component_type)) {
            return *type;
        }
    }

    const uint32_t type_id = module_.TakeNextId();
    auto new_inst = std::make_unique<Instruction>(4, spv::OpTypeVector);
    new_inst->Fill({type_id, component_type.Id(), component_count});
    return AddType(std::move(new_inst), SpvType::kVector);
}

const Type& TypeManager::GetTypeMatrix(const Type& column_type, uint32_t column_count) {
    for (const auto type : matrix_types_) {
        if (type->meta_.matrix.component_count != column_count) {
            continue;
        }

        const Type* matrix_column_type = FindTypeById(type->inst_.Word(2));
        if (matrix_column_type && (*matrix_column_type == column_type)) {
            return *type;
        }
    }

    const uint32_t type_id = module_.TakeNextId();
    auto new_inst = std::make_unique<Instruction>(4, spv::OpTypeMatrix);
    new_inst->Fill({type_id, column_type.Id(), column_count});
    return AddType(std::move(new_inst), SpvType::kMatrix);
}

const Type& TypeManager::GetTypeSampledImage(const Type& image_type) {
    for (const auto type : sampled_image_types_) {
        const Type* this_image_type = FindTypeById(type->inst_.Word(2));
        if (this_image_type && (*this_image_type == image_type)) {
            return *type;
        }
    }

    const uint32_t type_id = module_.TakeNextId();
    auto new_inst = std::make_unique<Instruction>(3, spv::OpTypeSampledImage);
    new_inst->Fill({type_id, image_type.Id()});
    return AddType(std::move(new_inst), SpvType::kSampledImage);
}

const Type& TypeManager::GetTypePointer(spv::StorageClass storage_class, const Type& pointer_type, bool get_explicit_layout) {
    if (get_explicit_layout || !IsExplicitLayoutType(pointer_type)) {
        for (const auto type : pointer_types_) {
            if (type->inst_.Word(2) != storage_class) {
                continue;
            }

            const Type* this_pointer_type = FindTypeById(type->inst_.Word(3));
            if (this_pointer_type && (*this_pointer_type == pointer_type)) {
                return *type;
            }
        }
    }

    const uint32_t type_id = module_.TakeNextId();
    auto new_inst = std::make_unique<Instruction>(4, spv::OpTypePointer);
    new_inst->Fill({type_id, uint32_t(storage_class), pointer_type.Id()});
    return AddType(std::move(new_inst), SpvType::kPointer);
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
        case spv::BuiltInLocalInvocationIndex:
        case spv::BuiltInSubgroupSize:
        case spv::BuiltInSubgroupLocalInvocationId: {
            const Type& uint_32 = GetTypeInt(32, false);
            return GetTypePointer(spv::StorageClassInput, uint_32);
        }
        case spv::BuiltInWorkgroupSize:
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
            assert(false && "unhandled BuiltIn");
            return *(id_to_type_.begin()->second);
        }
    }
}

// Just currently always make a new one
const Type& TypeManager::GetTypeUntypedPointer(spv::StorageClass storage_class) {
    const uint32_t type_id = module_.TakeNextId();
    auto new_inst = std::make_unique<Instruction>(3, spv::OpTypeUntypedPointerKHR);
    new_inst->Fill({type_id, uint32_t(storage_class)});
    return AddType(std::move(new_inst), SpvType::kUntypedPointerKHR);
}

uint32_t TypeManager::GetTypeBytesSize(const Type& type) {
    switch (type.spv_type_) {
        case SpvType::kFloat:
        case SpvType::kInt:
        case SpvType::kBool:
            return type.meta_.scalar.bit_width / 8u;
        case SpvType::kVector:
        case SpvType::kVectorIdEXT: {
            const Type* count_type = FindTypeById(type.inst_.Operand(0));
            return type.meta_.vector.component_count * GetTypeBytesSize(*count_type);
        }
        case SpvType::kMatrix: {
            const Type* column_type = FindTypeById(type.inst_.Operand(0));
            return type.meta_.matrix.component_count * GetTypeBytesSize(*column_type);
        }
        case SpvType::kPointer:
            assert(type.inst_.Operand(0) == spv::StorageClassPhysicalStorageBuffer && "unexpected pointer type");
            // always will be PhysicalStorageBuffer64 addressing model
            return 8u;
        case SpvType::kArray: {
            const Type* element_type = FindTypeById(type.inst_.Operand(0));
            return type.meta_.array.length * GetTypeBytesSize(*element_type);
        }
        case SpvType::kStruct: {
            // Get the offset of the last member and then figure out it's size
            // Note: the largest offset doesn't have to be the last element index of the struct
            uint32_t last_offset = 0;
            uint32_t last_offset_index = 0;
            const uint32_t struct_id = type.inst_.ResultId();

            // cached lookup if we already have seen this struct
            {
                auto it = struct_size_map_.find(struct_id);
                if (it != struct_size_map_.end()) {
                    return it->second;
                }
            }

            for (const auto& annotation : module_.annotations_) {
                if (annotation->Opcode() == spv::OpMemberDecorate && annotation->Word(1) == struct_id &&
                    annotation->Word(3) == spv::DecorationOffset) {
                    const uint32_t index = annotation->Word(2);
                    const uint32_t offset = annotation->Word(4);
                    if (offset > last_offset) {
                        last_offset = offset;
                        last_offset_index = index;
                    }
                }
            }

            const Type* last_element_type = FindTypeById(type.inst_.Operand(last_offset_index));
            const uint32_t last_length = GetTypeBytesSize(*last_element_type);
            const uint32_t struct_size = last_offset + last_length;
            struct_size_map_[struct_id] = struct_size;
            return struct_size;
        }
        case SpvType::kRuntimeArray:
            assert(false && "unsupported type");
            break;
        default:
            assert(false && "unexpected type");
            break;
    }
    return 0;
}

const Constant& TypeManager::AddConstant(std::unique_ptr<Instruction> new_inst, const Type& type) {
    const auto& inst = module_.types_values_constants_.emplace_back(std::move(new_inst));

    id_to_constant_[inst->ResultId()] = std::make_unique<Constant>(type, *inst);
    const Constant* new_constant = id_to_constant_[inst->ResultId()].get();

    if (inst->Opcode() == spv::OpConstant) {
        if (type.spv_type_ == SpvType::kInt && type.meta_.scalar.bit_width == 32) {
            int_32bit_constants_.emplace_back(new_constant);
        } else if (type.spv_type_ == SpvType::kFloat) {
            if (type.meta_.scalar.bit_width == 16) {
                float_16bit_constants_.emplace_back(new_constant);
            } else if (type.meta_.scalar.bit_width == 32) {
                float_32bit_constants_.emplace_back(new_constant);
            }
        }
    } else if (inst->Opcode() == spv::OpConstantNull) {
        null_constants_.emplace_back(new_constant);
    } else if (inst->Opcode() == spv::OpConstantTrue) {
        bool_true_constants_ = new_constant;
    } else if (inst->Opcode() == spv::OpConstantFalse) {
        bool_false_constants_ = new_constant;
    }

    return *new_constant;
}

const Constant* TypeManager::FindConstantInt32(uint32_t type_id, uint32_t value) const {
    for (const auto constant : int_32bit_constants_) {
        if (constant->type_.Id() == type_id && value == constant->inst_.Word(3)) {
            return constant;
        }
    }
    return nullptr;
}

const Constant* TypeManager::FindConstantFloat16(uint32_t type_id, uint32_t value) const {
    for (const auto constant : float_16bit_constants_) {
        if (constant->type_.Id() == type_id && value == constant->inst_.Word(3)) {
            return constant;
        }
    }
    return nullptr;
}

const Constant* TypeManager::FindConstantFloat32(uint32_t type_id, uint32_t value) const {
    for (const auto constant : float_32bit_constants_) {
        if (constant->type_.Id() == type_id && value == constant->inst_.Word(3)) {
            return constant;
        }
    }
    return nullptr;
}

const Constant* TypeManager::FindConstantById(uint32_t id) const {
    auto constant = id_to_constant_.find(id);
    return (constant == id_to_constant_.end()) ? nullptr : constant->second.get();
}

const Constant& TypeManager::CreateConstantUInt32(uint32_t value) {
    const Type& type = GetTypeInt(32, 0);
    const uint32_t constant_id = module_.TakeNextId();
    auto new_inst = std::make_unique<Instruction>(4, spv::OpConstant);
    new_inst->Fill({type.Id(), constant_id, value});
    return AddConstant(std::move(new_inst), type);
}

// Used when not sure if 8-bit float, 64-bit int, or something inbetween
const Constant& TypeManager::CreateConstantScalar(uint64_t value, const Type& type, uint32_t result_id) {
    const bool is_64bit = type.Is64Bit();

    auto new_inst = std::make_unique<Instruction>(is_64bit ? 5 : 4, spv::OpConstant);

    if (result_id == 0) {
        result_id = module_.TakeNextId();
    }

    std::vector<uint32_t> words = {type.Id(), result_id, static_cast<uint32_t>(value & 0xFFFFFFFF)};
    if (is_64bit) {
        words.emplace_back(static_cast<uint32_t>(value >> 32));
    }
    new_inst->Fill(words);

    return AddConstant(std::move(new_inst), type);
}

const Constant& TypeManager::GetConstantUInt32(uint32_t value) {
    if (value == 0) {
        return GetConstantZeroUint32();
    }

    const Type& uint32_type = GetTypeInt(32, 0);
    const Constant* constant = FindConstantInt32(uint32_type.Id(), value);
    if (!constant) {
        constant = &CreateConstantUInt32(value);
    }
    return *constant;
}

// Useful to canonicalize integer constants to uint32.
uint32_t TypeManager::GetConstantUInt32FromId(uint32_t id) {
    uint32_t value = FindConstantById(id)->GetValueUint32();
    return GetConstantUInt32(value).Id();
}

// Gets the OpConstantTrue/OpConstantFalse
const Constant& TypeManager::GetConstantBool(bool is_true) {
    const Type& bool_type = GetTypeBool();
    if (is_true) {
        if (!bool_true_constants_) {
            const uint32_t constant_id = module_.TakeNextId();
            auto new_inst = std::make_unique<Instruction>(3, spv::OpConstantTrue);
            new_inst->Fill({bool_type.Id(), constant_id});
            AddConstant(std::move(new_inst), bool_type);  // sets member var
        }
        return *bool_true_constants_;
    } else {
        if (!bool_false_constants_) {
            const uint32_t constant_id = module_.TakeNextId();
            auto new_inst = std::make_unique<Instruction>(3, spv::OpConstantFalse);
            new_inst->Fill({bool_type.Id(), constant_id});
            AddConstant(std::move(new_inst), bool_type);  // sets member var
        }
        return *bool_false_constants_;
    };
}
const Constant& TypeManager::GetConstantZeroUint32() {
    if (!uint_32bit_zero_constants_) {
        const Type& uint_32_type = GetTypeInt(32, 0);
        uint_32bit_zero_constants_ = FindConstantInt32(uint_32_type.Id(), 0);
        if (!uint_32bit_zero_constants_) {
            uint_32bit_zero_constants_ = &CreateConstantUInt32(0);
        }
    }
    return *uint_32bit_zero_constants_;
}

// It is common to use uint32_t(1), so having it cached is helpful
const Constant& TypeManager::GetConstantOneUint32() {
    if (!uint_32bit_one_constants_) {
        const Type& uint_32_type = GetTypeInt(32, 0);
        uint_32bit_one_constants_ = FindConstantInt32(uint_32_type.Id(), 1);
        if (!uint_32bit_one_constants_) {
            uint_32bit_one_constants_ = &CreateConstantUInt32(1);
        }
    }
    return *uint_32bit_one_constants_;
}

// It is common to use float16(0) as a default, so having it cached is helpful
const Constant& TypeManager::GetConstantZeroFloat16() {
    if (!float_16bit_zero_constants_) {
        const Type& float_16_type = GetTypeFloat(16);
        float_16bit_zero_constants_ = FindConstantFloat16(float_16_type.Id(), 0);
        if (!float_16bit_zero_constants_) {
            const uint32_t constant_id = module_.TakeNextId();
            auto new_inst = std::make_unique<Instruction>(4, spv::OpConstant);
            new_inst->Fill({float_16_type.Id(), constant_id, 0});
            float_16bit_zero_constants_ = &AddConstant(std::move(new_inst), float_16_type);
        }
    }
    return *float_16bit_zero_constants_;
}

// It is common to use float(0) as a default, so having it cached is helpful
const Constant& TypeManager::GetConstantZeroFloat32() {
    if (!float_32bit_zero_constants_) {
        const Type& float_32_type = GetTypeFloat(32);
        float_32bit_zero_constants_ = FindConstantFloat32(float_32_type.Id(), 0);
        if (!float_32bit_zero_constants_) {
            const uint32_t constant_id = module_.TakeNextId();
            auto new_inst = std::make_unique<Instruction>(4, spv::OpConstant);
            new_inst->Fill({float_32_type.Id(), constant_id, 0});
            float_32bit_zero_constants_ = &AddConstant(std::move(new_inst), float_32_type);
        }
    }
    return *float_32bit_zero_constants_;
}

// It is common to use vec3(0) as a default, so having it cached is helpful
const Constant& TypeManager::GetConstantZeroVec3() {
    if (!vec3_zero_constants_) {
        const Type& float_32_type = GetTypeFloat(32);
        const Type& vec3_type = GetTypeVector(float_32_type, 3);
        const uint32_t float32_0_id = GetConstantZeroFloat32().Id();

        const uint32_t constant_id = module_.TakeNextId();
        auto new_inst = std::make_unique<Instruction>(6, spv::OpConstantComposite);
        new_inst->Fill({vec3_type.Id(), constant_id, float32_0_id, float32_0_id, float32_0_id});
        vec3_zero_constants_ = &AddConstant(std::move(new_inst), vec3_type);
    }
    return *vec3_zero_constants_;
}

// It is common to use uvec4(0) as a default, so having it cached is helpful
const Constant& TypeManager::GetConstantZeroUvec4() {
    if (!uvec4_zero_constants_) {
        const Type& uint32_type = GetTypeInt(32, false);
        const Type& uvec4_type = GetTypeVector(uint32_type, 4);
        const uint32_t uint32_0_id = GetConstantZeroUint32().Id();

        const uint32_t constant_id = module_.TakeNextId();
        auto new_inst = std::make_unique<Instruction>(7, spv::OpConstantComposite);
        new_inst->Fill({uvec4_type.Id(), constant_id, uint32_0_id, uint32_0_id, uint32_0_id, uint32_0_id});
        uvec4_zero_constants_ = &AddConstant(std::move(new_inst), uvec4_type);
    }
    return *uvec4_zero_constants_;
}

const Constant& TypeManager::GetConstantZeroVector(const Type& vector_type) {
    assert(vector_type.spv_type_ == SpvType::kVector);
    const Type* component_type = FindTypeById(vector_type.inst_.Word(2));

    const uint32_t vector_length = vector_type.VectorSize();
    auto new_inst = std::make_unique<Instruction>(3 + vector_length, spv::OpConstantComposite);

    const uint32_t constant_id = module_.TakeNextId();
    std::vector<uint32_t> words = {vector_type.Id(), constant_id};

    const uint32_t null_type_id = GetConstantNull(*component_type).Id();
    for (uint32_t i = 0; i < vector_length; i++) {
        words.emplace_back(null_type_id);
    }
    new_inst->Fill(words);

    return AddConstant(std::move(new_inst), vector_type);
}

const Constant& TypeManager::GetConstantNull(const Type& type) {
    for (const auto& constant : null_constants_) {
        if (constant->type_.Id() == type.Id()) {
            return *constant;
        }
    }

    const uint32_t constant_id = module_.TakeNextId();
    auto new_inst = std::make_unique<Instruction>(3, spv::OpConstantNull);
    new_inst->Fill({type.Id(), constant_id});
    return AddConstant(std::move(new_inst), type);
}

const AccessPath TypeManager::BuildAccessPath(const Function& function, const Instruction& inst) {
    AccessPath path;

    if (!inst.IsMemoryAccess()) {
        return path;
    }

    const spv::Op opcode = (spv::Op)inst.Opcode();
    path.access_type = FindTypeById(inst.TypeId());
    // if it is a store, then we need to look a the type it loading
    if (!path.access_type) {
        if (opcode == spv::OpStore || opcode == spv::OpCooperativeMatrixStoreKHR) {
            path.access_type = FindTypeGlobal(function, inst.Operand(1));  // object id
        } else if (opcode == spv::OpImageWrite) {
            path.access_type = FindTypeGlobal(function, inst.Operand(2));  // texel id
        } else if (opcode == spv::OpAtomicStore) {
            path.access_type = FindTypeGlobal(function, inst.Operand(3));  // value id
        } else {
            assert(false);  // not being handled
        }
    }
    assert(path.access_type);

    const bool image_sampler_access = path.access_type->spv_type_ == SpvType::kImage ||
                                      path.access_type->spv_type_ == SpvType::kSampledImage ||
                                      path.access_type->spv_type_ == SpvType::kSampler;

    // This is just loading the image handle, this alone is the not the access.
    // There will be an access (OpImageWrite, OpImageSampleImplicitLod, etc) later which has the real access information
    if (opcode == spv::OpLoad && image_sampler_access) {
        return path;
    }

    const Instruction* sampler_load_inst = nullptr;
    uint32_t ptr_id = OpcodeImageAccessPosition(opcode);
    // Basically there are 2 flows, images and non-images
    const bool image_access = ptr_id != 0;
    if (image_access) {
        path.image_load_inst = function.FindInstruction(inst.Word(ptr_id));
        const Instruction* load_inst = path.image_load_inst;
        uint32_t load_operand = 0;
        while (load_inst && (load_inst->Opcode() == spv::OpSampledImage || load_inst->Opcode() == spv::OpImage ||
                             load_inst->Opcode() == spv::OpCopyObject)) {
            if (load_inst->Opcode() == spv::OpSampledImage) {
                sampler_load_inst = function.FindInstruction(load_inst->Operand(1));
            }

            load_operand = load_inst->Operand(0);
            load_inst = function.FindInstruction(load_operand);
        }

        // Note - we never "store" an image, we only load its handle and store the "texel" data
        if (!load_inst || load_inst->Opcode() != spv::OpLoad) {
            // TODO - should be able to remove this check, its invalid SPIR-V
            // https://gitlab.khronos.org/vulkan/vulkan/-/merge_requests/7753
            assert(IsUndef(load_operand));
            return path;
        }

        path.is_combined_image_sampler = sampler_load_inst == nullptr && ImageSampleOperation(opcode);

        // From here the load should look like a non-image access
        ptr_id = load_inst->Operand(0);
    } else {
        // |Operand 0| works for both Store/Load
        ptr_id = inst.Operand(0);
    }

    // Buffer/Image Descriptor will always have an access chains, but some cases can have direct access.
    // TaskPayload can be a scalar that does a direct variable access
    // An non-array AccelerationStructure (which uses UniformConstant storage class)
    path.variable = FindVariableById(ptr_id);
    const Instruction* next_access_chain = nullptr;
    if (!path.variable) {
        next_access_chain = function.FindInstruction(ptr_id);

        // We need to walk down possibly multiple chained OpAccessChains or OpCopyObject to get the variable
        while (next_access_chain && next_access_chain->IsNonPtrAccessChain()) {
            // inserting in front allows us to walk over the loop from the front
            path.ac_list.insert(path.ac_list.begin(), next_access_chain);

            const uint32_t base_operand = next_access_chain->IsUntypedAccessChain() ? 1 : 0;
            const uint32_t access_chain_base_id = next_access_chain->Operand(base_operand);
            path.variable = FindVariableById(access_chain_base_id);
            if (path.variable) {
                break;  // found
            }
            next_access_chain = function.FindInstruction(access_chain_base_id);
        }

        if (next_access_chain->Opcode() == spv::OpBufferPointerEXT) {
            spv::StorageClass buffer_ptr_sc = FindTypeById(next_access_chain->TypeId())->inst_.StorageClass();
            // https://gitlab.khronos.org/vulkan/vulkan/-/issues/4858
            // It should be a bug to use 1.0 BufferBlock
            path.descriptor_type = buffer_ptr_sc == spv::StorageClassStorageBuffer ? gpuav::descriptor::TYPE_STORAGE_BUFFER
                                                                                   : gpuav::descriptor::TYPE_UNIFORM_BUFFER;

            const uint32_t buffer_pointer_id = next_access_chain->Operand(0);
            // For now assume this is a 1D array into the descriptor array
            // https://gitlab.khronos.org/spirv/SPIR-V/-/issues/942
            next_access_chain = function.FindInstruction(buffer_pointer_id);
            assert(next_access_chain->Opcode() == spv::OpUntypedAccessChainKHR);
            path.ac_list.insert(path.ac_list.begin(), next_access_chain);
            const uint32_t untyped_variable_id = next_access_chain->Operand(1);
            path.variable = FindVariableById(untyped_variable_id);
        } else if (next_access_chain->Opcode() == spv::OpImageTexelPointer) {
            path.descriptor_type = gpuav::descriptor::TYPE_IMAGE_STORAGE;
            const Instruction* access_chain_inst = function.FindInstruction(next_access_chain->Operand(0));
            if (access_chain_inst && access_chain_inst->IsNonPtrAccessChain()) {
                next_access_chain = access_chain_inst;
                path.ac_list.insert(path.ac_list.begin(), next_access_chain);
                path.variable = FindVariableById(access_chain_inst->Operand(0));
            } else {
                // if no array, will point right to a variable
                path.variable = FindVariableById(next_access_chain->Operand(0));
            }
        }
    }

    if (!path.variable) {
        // Two know spots this occur is Function Variables and PhysicalStorageBuffer access
        assert((next_access_chain->Opcode() == spv::OpVariable && next_access_chain->StorageClass() == spv::StorageClassFunction) ||
               FindTypeById(next_access_chain->TypeId())->spv_type_ == SpvType::kPointer);
        return path;  // not a valid access path
    }

    // Welcome to SPV_KHR_untyped_pointers soldier!
    // Untyped we get the pointer type from the last access chain
    // But typed, the OpVariable had it
    if (next_access_chain && next_access_chain->IsUntypedAccessChain()) {
        const uint32_t pointer_type_id = next_access_chain->Operand(0);
        path.pointer_type = FindTypeById(pointer_type_id);
    } else {
        path.pointer_type = path.variable->PointerType(*this);
    }
    assert(path.pointer_type);

    // Everything else is just for descriptor variable access
    if (!path.variable->IsDescriptor()) {
        return path;
    }

    if (path.pointer_type->IsArray()) {
        assert(next_access_chain);  // no way to have an array otherwise
        const uint32_t index_0_operand = next_access_chain->IsUntypedAccessChain() ? 2 : 1;
        path.descriptor_index_id = next_access_chain->Operand(index_0_operand);
    } else {
        // There is no array of this descriptor, so we essentially have an array of 1
        path.descriptor_index_id = GetConstantZeroUint32().Id();

        // Hack for Offset in Heaps until get better understanding
        if (path.variable->interface_.IsHeap() && path.pointer_type->spv_type_ == SpvType::kStruct) {
            assert(next_access_chain->IsUntypedAccessChain() && next_access_chain->Length() == 7);
            // https://godbolt.org/z/hWz84zdTW - this is required to be a constant
            const Constant* struct_member_index_constant = FindConstantById(next_access_chain->Operand(2));
            assert(struct_member_index_constant);
            path.heap_offset_member_index = struct_member_index_constant->GetValueUint32();
            path.descriptor_index_id = next_access_chain->Operand(3);
        }
    }

    // When using a SAMPLED_IMAGE and SAMPLER, they are accessed together so we need check for 2 descriptors
    if (sampler_load_inst) {
        assert(sampler_load_inst->Opcode() == spv::OpLoad);

        ptr_id = sampler_load_inst->Operand(0);
        path.sampler_variable = FindVariableById(ptr_id);
        if (path.sampler_variable) {
            path.sampler_descriptor_index_id = GetConstantZeroUint32().Id();
        } else {
            // descriptor array
            // this is a lazy way to assume the sampler is can only be 1D and a single access chain away
            next_access_chain = function.FindInstruction(ptr_id);
            assert(next_access_chain->IsNonPtrAccessChain());

            const uint32_t base_operand = next_access_chain->IsUntypedAccessChain() ? 1 : 0;
            const uint32_t access_chain_base_id = next_access_chain->Operand(base_operand);
            path.sampler_variable = FindVariableById(access_chain_base_id);

            const uint32_t index_0_operand = base_operand + 1;
            path.sampler_descriptor_index_id = next_access_chain->Operand(index_0_operand);
        }
    }

    // no way this can ever be a sampler
    const uint8_t invalid_type = gpuav::descriptor::TYPE_SAMPLER;
    if (path.descriptor_type == invalid_type) {
        if (image_access) {
            const Type* image_type = FindTypeById(path.image_load_inst->TypeId());
            assert(image_type && (image_type->spv_type_ == SpvType::kImage || image_type->spv_type_ == SpvType::kSampledImage));

            if (image_type->spv_type_ == SpvType::kSampledImage) {
                path.descriptor_type = gpuav::descriptor::TYPE_IMAGE_SAMPLED;
            } else {
                const VkDescriptorType image_descriptor_type = image_type->inst_.GetImageType();
                if (image_descriptor_type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE) {
                    path.descriptor_type = gpuav::descriptor::TYPE_IMAGE_SAMPLED;
                } else if (image_descriptor_type == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT) {
                    path.descriptor_type = gpuav::descriptor::TYPE_IMAGE_INPUT_ATTACHMENT;
                } else if (image_descriptor_type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE) {
                    path.descriptor_type = gpuav::descriptor::TYPE_IMAGE_STORAGE;
                } else if (image_descriptor_type == VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER) {
                    path.descriptor_type = gpuav::descriptor::TYPE_IMAGE_TEXEL_BUFFER_STORAGE;
                } else if (image_descriptor_type == VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER) {
                    path.descriptor_type = gpuav::descriptor::TYPE_IMAGE_TEXEL_BUFFER_UNIFORM;
                }
            }
        } else if (path.access_type->spv_type_ == SpvType::kAccelerationStructureKHR) {
            path.descriptor_type = gpuav::descriptor::TYPE_ACCELERATION_STRUCTURE;
        } else {
            spv::StorageClass access_sc = FindTypeById(path.ac_list.front()->TypeId())->inst_.StorageClass();
            if (access_sc == spv::StorageClassStorageBuffer) {
                path.descriptor_type = gpuav::descriptor::TYPE_STORAGE_BUFFER;
            } else if (access_sc == spv::StorageClassUniform) {
                path.descriptor_type = gpuav::descriptor::TYPE_UNIFORM_BUFFER;

                // handles the dumb issue where 1.0 shaders "Uniform" could be really a Storage Buffer
                // https://github.com/KhronosGroup/Vulkan-Guide/blob/main/chapters/extensions/shader_features.adoc#vk_khr_storage_buffer_storage_class
                const uint32_t spirv_version_1_3 = 0x00010300;  // Vulkan 1.1
                if (module_.header_.version < spirv_version_1_3) {
                    // Here we have a Vulkan 1.0 shader and need to just make sure there is no BufferBlock on the struct
                    const uint32_t block_type_id =
                        path.pointer_type->IsArray() ? path.pointer_type->inst_.Operand(0) : path.pointer_type->Id();
                    for (const auto& annotation : module_.annotations_) {
                        if (annotation->Opcode() == spv::OpDecorate && annotation->Word(1) == block_type_id &&
                            spv::Decoration(annotation->Word(2)) == spv::DecorationBufferBlock) {
                            path.descriptor_type = gpuav::descriptor::TYPE_STORAGE_BUFFER;
                            break;
                        }
                    }
                }
            }
        }
    }
    assert(path.descriptor_type != invalid_type);

    return path;
}

const Variable& TypeManager::AddVariable(std::unique_ptr<Instruction> new_inst, const Type& type) {
    const auto& inst = module_.types_values_constants_.emplace_back(std::move(new_inst));

    id_to_variable_[inst->ResultId()] = std::make_unique<Variable>(module_, type, *inst);
    const Variable* new_variable = id_to_variable_[inst->ResultId()].get();

    if (new_variable->StorageClass() == spv::StorageClassInput) {
        input_variables_.push_back(new_variable);
    } else if (new_variable->StorageClass() == spv::StorageClassOutput) {
        output_variables_.push_back(new_variable);
    } else if (new_variable->StorageClass() == spv::StorageClassPushConstant) {
        // This is found in the entrypoint starting in 1.4
        const uint32_t spirv_version_1_4 = 0x00010400;
        if (module_.header_.version < spirv_version_1_4) {
            if (!push_constant_variable_) {
                push_constant_variable_ = new_variable;
            } else {
                assert(module_.entry_points_.size() > 1);
                module_.InternalWarning(
                    "AddVariable",
                    "Found 2 different OpVariable, can't determine which entrypoint, can be fixed updating SPIR-V to 1.4+");
            }
        }
    } else if (new_variable->StorageClass() == spv::StorageClassWorkgroup) {
        shared_memory_variables_.push_back(new_variable);
    } else if (new_variable->StorageClass() == spv::StorageClassTaskPayloadWorkgroupEXT) {
        task_payload_variables_.push_back(new_variable);
    }

    return *new_variable;
}

// Note - this does not include Function variables and will not find them
// currently no need to track them for any GPU-AV checks
const Variable* TypeManager::FindVariableById(uint32_t id) const {
    auto variable = id_to_variable_.find(id);
    return (variable == id_to_variable_.end()) ? nullptr : variable->second.get();
}

const Variable* TypeManager::FindPushConstantVariable() const { return push_constant_variable_; }

void TypeManager::AddPushConstantVariable() {
    const uint32_t spirv_version_1_4 = 0x00010400;
    if (module_.header_.version >= spirv_version_1_4) {
        const Instruction* entry_point = module_.GetTargetEntryPoint();
        uint32_t word = entry_point->GetEntryPointInterfaceStart();
        const uint32_t total_words = entry_point->Length();
        for (; word < total_words; word++) {
            const uint32_t interface_id = entry_point->Word(word);
            const Variable* variable = FindVariableById(interface_id);
            if (variable && variable->StorageClass() == spv::StorageClassPushConstant) {
                push_constant_variable_ = variable;
                return;
            }
        }
    }
}

Type::Meta Type::SetMeta(SpvType spv_type, const Instruction& inst, const TypeManager& type_manager) {
    Meta m{};  // Zero-initialize (unions don't zero-init automatically)
    if (spv_type == SpvType::kVector) {
        m.vector.component_count = inst.Word(3);
    } else if (spv_type == SpvType::kVectorIdEXT) {
        const Constant* count = type_manager.FindConstantById(inst.Word(3));
        assert(count && !count->is_spec_constant_);
        m.vector.component_count = count->GetValueUint32();
    } else if (spv_type == SpvType::kMatrix) {
        m.matrix.component_count = inst.Word(3);
    } else if (spv_type == SpvType::kArray) {
        const Constant* count = type_manager.FindConstantById(inst.Word(3));
        assert(count && !count->is_spec_constant_);
        m.array.length = count->GetValueUint32();
    } else if (spv_type == SpvType::kFloat) {
        m.scalar.bit_width = inst.Word(2);
    } else if (spv_type == SpvType::kInt) {
        m.scalar.bit_width = inst.Word(2);
        m.scalar.is_signed = inst.Word(3) == 1;
    } else if (spv_type == SpvType::kBool) {
        // "Boolean values considered as 32-bit integer values for the purpose of this calculation"
        m.scalar.bit_width = 32;
    }
    return m;
}

Type::Type(SpvType spv_type, const Instruction& inst, const TypeManager& type_manager)
    : spv_type_(spv_type), inst_(inst), meta_(SetMeta(spv_type, inst, type_manager)) {}

bool Type::IsArray() const { return spv_type_ == SpvType::kArray || spv_type_ == SpvType::kRuntimeArray; }

bool Type::IsSignedInt() const { return spv_type_ == SpvType::kInt && meta_.scalar.is_signed; }

bool Type::IsIVec3(const TypeManager& type_manager) const {
    if (spv_type_ == SpvType::kVector) {
        const Type* vector_component_type = type_manager.FindTypeById(inst_.Word(2));
        if (vector_component_type && vector_component_type->IsSignedInt()) {
            return true;
        }
    }
    return false;
}

uint32_t Type::VectorSize() const {
    return (spv_type_ == SpvType::kVector || spv_type_ == SpvType::kVectorIdEXT) ? meta_.vector.component_count : 0;
}

bool Type::Is64Bit() const {
    if (spv_type_ == SpvType::kFloat || spv_type_ == SpvType::kInt) {
        return meta_.scalar.bit_width == 64;
    }
    return false;
}

// Current use for SharedMemoryDataRace where each scalar is a slot and we need to know where inside a data type the slot index is.
uint32_t TypeManager::GetScalarElementCount(const Type& type) const {
    switch (type.spv_type_) {
        case SpvType::kStruct: {
            uint32_t count = 0;
            uint32_t members = type.inst_.Length() - 2;
            for (uint32_t i = 0; i < members; ++i) {
                count += GetScalarElementCount(*FindChildType(type, i));
            }
            return count;
        }
        case SpvType::kArray: {
            const Type* element_type = FindTypeById(type.inst_.Word(2));
            return type.meta_.array.length * GetScalarElementCount(*element_type);
        }
        case SpvType::kVectorIdEXT:
        case SpvType::kVector:
            return type.meta_.vector.component_count;
        case SpvType::kMatrix:
            return type.meta_.matrix.component_count * GetScalarElementCount(*FindTypeById(type.inst_.Word(2)));
        case SpvType::kInt:
        case SpvType::kFloat:
        case SpvType::kBool:
            return 1;
        default:
            assert(false);
            return 0;
    }
}

uint32_t Constant::GetValueUint32() const {
    assert(inst_.Opcode() == spv::OpConstant || inst_.Opcode() == spv::OpConstantNull);
    return inst_.Opcode() == spv::OpConstantNull ? 0 : inst_.Word(3);
}

uint64_t Constant::GetValueUint64(bool is_signed) const {
    const uint32_t bit_width = type_.meta_.scalar.bit_width;

    uint64_t value = inst_.Word(3);
    if (bit_width == 64) {
        value |= (static_cast<uint64_t>(inst_.Word(4)) << 32);
    }

    // Sign-extend if necessary (8, 16, 32 bit negative numbers)
    if (is_signed && bit_width < 64) {
        uint64_t sign_bit = 1ULL << (bit_width - 1);
        if (value & sign_bit) {
            uint64_t mask = ~0ULL << bit_width;
            value |= mask;
        }
    }
    return value;
}

void TypeManager::AddUndef(std::unique_ptr<Instruction> new_inst) {
    const auto& inst = module_.types_values_constants_.emplace_back(std::move(new_inst));
    undef_ids_.insert(inst->ResultId());
}

bool TypeManager::IsUndef(uint32_t id) const { return undef_ids_.find(id) != undef_ids_.end(); }

// Not ideal to use, the caller really should already know which type it wants
const Type* TypeManager::FindChildType(const Type& type, uint32_t idx) const {
    switch (type.spv_type_) {
        case SpvType::kPointer:
            assert(idx == 0);
            return FindTypeById(type.inst_.Word(3));
        case SpvType::kStruct:
            return FindTypeById(type.inst_.Word(idx + 2));
        case SpvType::kArray:
            assert(idx == 0);
            return FindTypeById(type.inst_.Word(2));
        case SpvType::kVectorIdEXT:
        case SpvType::kVector:
        case SpvType::kMatrix:
            assert(idx == 0);
            return FindTypeById(type.inst_.Word(2));
        case SpvType::kInt:
        case SpvType::kFloat:
        case SpvType::kBool:
            return nullptr;
        default:
            assert(0);
            return nullptr;
    }
}

// This stems from the fact even if you have two OpTypeStruct that are the EXACT SAME, it is not valid to mix and match them
// (https://gitlab.khronos.org/spirv/SPIR-V/-/issues/918) The idea is when linking, to ignore anything that has any reference to an
// explicit layout types (like structs) that might collide with our incoming GLSL code
bool TypeManager::IsExplicitLayoutType(const Type& type) const { return type.spv_type_ == SpvType::kStruct; }

}  // namespace spirv
}  // namespace gpuav
