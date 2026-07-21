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
#pragma once

#include <vulkan/vulkan.h>
#include <cstdint>
#include <vector>
#include <memory>
#include "containers/custom_containers.h"
#include "containers/limits.h"
#include "cooperative_matrix.h"
#include "state_tracker/shader_instruction.h"
#include "generated/spirv_grammar_helper.h"

namespace gpuav {
namespace spirv {
using Instruction = ::spirv::Instruction;

class Module;
class TypeManager;
struct Function;

// These are the constant operations that we plan to handle in for shader instrumentation
static constexpr bool ConstantOperation(uint32_t opcode) {
    switch (opcode) {
        case spv::OpConstant:
        case spv::OpConstantTrue:
        case spv::OpConstantFalse:
        case spv::OpConstantComposite:
        case spv::OpConstantNull:
            return true;
        // Using a spec constant is bad as we might alias the value and have it change on use at pipeline creation time
        case spv::OpSpecConstant:
        case spv::OpSpecConstantTrue:
        case spv::OpSpecConstantFalse:
        case spv::OpSpecConstantComposite:
        case spv::OpSpecConstantOp:  // always must be in function block
        default:
            return false;
    }
}

// There is a LOT that can be done with types, but for simplicity it only does what is needed.
// The main thing is to try find the type so we don't add a duplicate (but not end of the world if 1 or 2 are duplicated as a
// trade-off to doing complex logic to resolve more complex types). The class also takes advantage that while Instrumenting we are
// always aware of our types we are adding or just explictly found.
struct Type {
    Type(SpvType spv_type, const Instruction& inst, const TypeManager& type_manager);

    bool operator==(Type const& other) const;
    uint32_t Id() const { return inst_.ResultId(); }

    // Helpers to detect what the type is
    bool IsArray() const;
    bool IsSignedInt() const;
    bool IsIVec3(const TypeManager& type_manager) const;
    // If returns 0, means it is a scalar
    uint32_t VectorSize() const;
    // 64-bit floats/int take up 2 dwords
    bool Is64Bit() const;

    const SpvType spv_type_;
    const Instruction& inst_;

    // Some metadata depending on the type. Some things can't be detected without full view of the module, instead of passing in
    // TypeManager as an argument to helper functions, it can just get the information once at construction time Want to keep this
    // *simple* and at most 8 bytes (since Type is aligned up to 24 bytes currently)
    struct VectorMeta {
        uint32_t component_count;
    };
    struct MatrixMeta {
        uint32_t component_count;
    };
    struct ArrayMeta {
        uint32_t length;
    };
    struct ScalarMeta {
        uint32_t bit_width;
        bool is_signed;
    };
    union Meta {
        VectorMeta vector;
        MatrixMeta matrix;
        ArrayMeta array;
        ScalarMeta scalar;
    };
    const Meta meta_;
    static Meta SetMeta(SpvType spv_type, const Instruction& inst, const TypeManager& type_manager);
};

static bool IsSpecConstant(uint32_t opcode) {
    return opcode == spv::OpSpecConstant || opcode == spv::OpSpecConstantTrue || opcode == spv::OpSpecConstantFalse ||
           opcode == spv::OpSpecConstantComposite || opcode == spv::OpSpecConstantOp;
}

// Represents a OpConstant* or OpSpecConstant*
// (Currently doesn't handle OpSpecConstantOp)
struct Constant {
    Constant(const Type& type, const Instruction& inst)
        : type_(type), inst_(inst), is_spec_constant_(IsSpecConstant(inst.Opcode())) {}

    uint32_t Id() const { return inst_.ResultId(); }

    // Only for cases where we know the constant value
    uint32_t GetValueUint32() const;
    uint64_t GetValueUint64(bool is_signed) const;

    const Type& type_;
    const Instruction& inst_;
    // We currently freeze spec constants and do our own constant folding, but we have this here as a way to catch any edge cases we
    // have missed in constant folding
    const bool is_spec_constant_;
};

struct DescriptorInterface {
    // Set/Binding for decorations
    uint32_t set = vvl::kNoIndex32;
    uint32_t binding = vvl::kNoIndex32;

    // For SPV_EXT_descriptor_heap
    bool is_resource_heap = false;
    bool is_sampler_heap = false;
    bool IsHeap() const { return is_resource_heap || is_sampler_heap; }
};

// Represents a global OpVariable found before the first function
struct Variable {
    Variable(const Module& module, const Type& type, const Instruction& inst)
        : type_(type), inst_(inst), interface_(FindDescriptorInterface(module, inst)) {}

    uint32_t Id() const { return inst_.ResultId(); }
    spv::StorageClass StorageClass() const { return inst_.StorageClass(); }
    const Type* PointerType(const TypeManager& type_manager_) const;

    const Type& type_;
    const Instruction& inst_;

    const DescriptorInterface interface_;

    // Help used to know if you have a PushConstant, Input/Output, etc instead
    bool IsDescriptor() const {
        return interface_.IsHeap() || (interface_.set != vvl::kNoIndex32 && interface_.binding != vvl::kNoIndex32);
    }

  protected:
    static DescriptorInterface FindDescriptorInterface(const Module& module, const Instruction& inst);
};

// We often want to walk the SSA from an "access" (load, store, atomic, etc) to the Variable it is referencing. There can be a
// single OpAccessChain or multiple, and this struct holds this information.
// Background info: https://github.com/KhronosGroup/SPIRV-Guide/blob/main/chapters/access_chains.md
//
// Note - currently this is very heavily leaned towards use of descriptors, but will work for any variable type
struct AccessPath {
    // The type of the access itself (what type it will store or load)
    const Type* access_type = nullptr;

    // This the %ptr_type in
    //   %ptr_type = OpTypeArray
    //   %ptr = OpTypePointer StorageBuffer %ptr_type
    //   %var = OpVariable %ptr StorageBuffer
    const Type* pointer_type = nullptr;

    // The variable at the end of the access chain
    const Variable* variable = nullptr;

    bool IsValid() const { return access_type != nullptr && pointer_type != nullptr && variable != nullptr; }

    // List of OpAccessChains from the variable to the "access"
    // - The front() will be closest to the OpVariable
    // - The back() will be closest to the exact spot accesssed
    // This is on purpose as we really will want to loop the OpAccessChain in reserve SSA order
    //
    // Note: GLSL will try to always create a single large OpAccessChain
    std::vector<const Instruction*> ac_list;

    //
    // Descriptor variable access related info
    //

    // Optional variable of seperate sampler descriptor (still null if combinedImageSampler)
    const Variable* sampler_variable = nullptr;
    const Type* sampler_pointer_type = nullptr;
    bool is_combined_image_sampler = false;
    bool HasSampler() const { return sampler_variable != nullptr || is_combined_image_sampler; }

    // The OpLoad to access an image descriptor
    const Instruction* image_load_inst = nullptr;

    // Most access paths are used to get the descriptor variable.
    // This is the ID of the uint that indexes in the array (or constant zero if no array)
    uint32_t descriptor_index_id = 0;
    // Optional index if there is a seperate sampler as well
    uint32_t sampler_descriptor_index_id = 0;

    // TODO - Need to handle OffsetIdEXT correctly, this is a dumb hack
    uint32_t heap_offset_member_index = 0;
    uint32_t sampler_heap_offset_member_index = 0;

    VkDescriptorType descriptor_type = VK_DESCRIPTOR_TYPE_MAX_ENUM;

    CooperativeMatrixAccess coop_mat{};
};

// In charge of tracking all Types, Constants, and Variable in the module.
// Since both Variable and Constant both rely on Types, the Types are the core thing we track
//
// Function naming guide:
//      Find*() - searches if type/constant/var is already there, will return null if not
//       Get*() - searches if type/constant/var is already there, will create one if not
//    Create*() - just makes the type/constant/var, doesn't attempt to search for a duplicate
class TypeManager {
  public:
    TypeManager(Module& module) : module_(module) {}

    const Type& AddType(std::unique_ptr<Instruction> new_inst, SpvType spv_type);

    const Type* FindTypeById(uint32_t id) const;
    const Type* FindFunctionType(const Instruction& inst) const;
    const Type* FindTypeGlobal(const Function& function, uint32_t id) const;
    // There shouldn't be a case where we need to query for a specific type, but then not add it if not found.
    const Type& GetTypeVoid();
    const Type& GetTypeBool();
    const Type& GetTypeSampler();
    const Type& GetTypeRayQuery();
    const Type& GetTypeAccelerationStructure();
    const Type& GetTypeInt(uint32_t bit_width, bool is_signed);
    const Type& GetTypeFloat(uint32_t bit_width);
    const Type& GetTypeArray(const Type& element_type, const Constant& length, bool get_explicit_layout = true);
    const Type& GetTypeRuntimeArray(const Type& element_type, bool get_explicit_layout = true);
    const Type& GetTypeVector(const Type& component_type, uint32_t component_count);
    const Type& GetTypeMatrix(const Type& column_type, uint32_t column_count);
    const Type& GetTypeSampledImage(const Type& image_type);
    const Type& GetTypePointer(spv::StorageClass storage_class, const Type& pointer_type, bool get_explicit_layout = true);
    const Type& GetTypePointerBuiltInInput(spv::BuiltIn built_in);

    // Returns the total size in 'bytes' of any OpType*
    uint32_t GetTypeBytesSize(const Type& type);

    // Special struct type helpers for Linking
    void AddStructTypeForLinking(const Type* new_type);
    uint32_t FindLinkingStructType(const Instruction& inst, vvl::unordered_map<uint32_t, uint32_t>& id_swap_map) const;

    const Constant& AddConstant(std::unique_ptr<Instruction> new_inst, const Type& type);
    const Constant* FindConstantById(uint32_t id) const;
    const Constant* FindConstantInt32(uint32_t type_id, uint32_t value) const;
    const Constant* FindConstantFloat16(uint32_t type_id, uint32_t value) const;
    const Constant* FindConstantFloat32(uint32_t type_id, uint32_t value) const;
    // most constants are uint
    const Constant& CreateConstantUInt32(uint32_t value);
    const Constant& CreateConstantScalar(uint64_t value, const Type& type, uint32_t result_id = 0);
    const Constant& GetConstantUInt32(uint32_t value);
    uint32_t GetConstantUInt32FromId(uint32_t id);  // get a uint32 constant from an integer constant of any type
    const Constant& GetConstantBool(bool is_true);
    const Constant& GetConstantZeroUint32();
    const Constant& GetConstantOneUint32();
    const Constant& GetConstantZeroFloat16();
    const Constant& GetConstantZeroFloat32();
    const Constant& GetConstantZeroVec3();
    const Constant& GetConstantZeroUvec4();
    const Constant& GetConstantZeroVector(const Type& vector_type);
    const Constant& GetConstantNull(const Type& type);

    const AccessPath BuildAccessPath(const Function& function, const Instruction& inst);
    const CooperativeMatrixAccess BuildCooperativeMatrixAccess(const Function& function, const Instruction& inst);

    const Variable& AddVariable(std::unique_ptr<Instruction> new_inst, const Type& type);
    const Variable* FindVariableById(uint32_t id) const;
    void OverridePushConstantVariable(const Variable* new_variable);
    const Variable* FindPushConstantVariable() const;
    const std::vector<const Variable*>& GetSharedMemoryVariables() const { return shared_memory_variables_; }
    const std::vector<const Variable*>& GetTaskPayloadVariables() const { return task_payload_variables_; }

    const Type* FindChildType(const Type& type, uint32_t idx) const;

    uint32_t GetScalarElementCount(const Type& type) const;

    void AddUndef(std::unique_ptr<Instruction> new_inst);
    bool IsUndef(uint32_t id) const;

    void FindArrayOfPSBStructWithRuntime(vvl::unordered_set<uint32_t>& out_struct_ids);

  private:
    Module& module_;

    // Currently we don't worry about duplicated types. If duplicate types are added from the original SPIR-V, we just use the first
    // one we fine. We should only be adding a new object because it currently doesn't exists.
    vvl::unordered_map<uint32_t, std::unique_ptr<Type>> id_to_type_;
    vvl::unordered_map<uint32_t, std::unique_ptr<Constant>> id_to_constant_;
    vvl::unordered_map<uint32_t, std::unique_ptr<Variable>> id_to_variable_;

    // Create faster lookups for specific types
    // some types are base types and only will be one
    const Type* void_type = nullptr;
    const Type* bool_type = nullptr;
    const Type* sampler_type = nullptr;
    const Type* ray_query_type = nullptr;
    const Type* acceleration_structure_type = nullptr;
    std::vector<const Type*> int_types_;
    std::vector<const Type*> float_types_;
    std::vector<const Type*> vector_types_;
    std::vector<const Type*> matrix_types_;
    std::vector<const Type*> image_types_;
    std::vector<const Type*> sampled_image_types_;
    std::vector<const Type*> array_types_;
    std::vector<const Type*> runtime_array_types_;
    std::vector<const Type*> coop_mat_types_;
    std::vector<const Type*> pointer_types_;
    std::vector<const Type*> forward_pointer_types_;
    std::vector<const Type*> function_types_;
    // Only for types we want to avoid when linking
    std::vector<const Type*> linking_struct_types_;

    std::vector<const Constant*> int_32bit_constants_;
    std::vector<const Constant*> float_16bit_constants_;
    std::vector<const Constant*> float_32bit_constants_;
    const Constant* bool_true_constants_ = nullptr;
    const Constant* bool_false_constants_ = nullptr;
    const Constant* uint_32bit_zero_constants_ = nullptr;
    const Constant* uint_32bit_one_constants_ = nullptr;
    const Constant* float_16bit_zero_constants_ = nullptr;
    const Constant* float_32bit_zero_constants_ = nullptr;
    const Constant* vec3_zero_constants_ = nullptr;
    const Constant* uvec4_zero_constants_ = nullptr;
    std::vector<const Constant*> null_constants_;

    std::vector<const Variable*> input_variables_;
    std::vector<const Variable*> output_variables_;
    // There is invalid to have more than 1 push constant variable per entrypoint
    const Variable* push_constant_variable_ = nullptr;
    std::vector<const Variable*> shared_memory_variables_;
    std::vector<const Variable*> task_payload_variables_;

    // Save the length of a struct so we don't have to look it up everytime
    // <struct_id, struct size>
    vvl::unordered_map<uint32_t, uint32_t> struct_size_map_;

    // The use of OpUndef is sometime misused, we store all OpUndef here as way to check if we have it one by accident
    vvl::unordered_set<uint32_t> undef_ids_;

    bool IsExplicitLayoutType(const Type& type) const;
};

}  // namespace spirv
}  // namespace gpuav
