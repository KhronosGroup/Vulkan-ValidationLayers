/* Copyright (c) 2021-2023 The Khronos Group Inc.
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
 *
 * The Shader Module file is in charge of all things around creating and parsing an internal representation of a shader module
 */

#pragma once

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <unordered_map>
#include <vector>

#include "state_tracker/shader_instruction.h"
#include "state_tracker/base_node.h"
#include "state_tracker/sampler_state.h"
#include <spirv/unified1/spirv.hpp>
#include "spirv-tools/optimizer.hpp"

class PIPELINE_STATE;

// This is the common info for both OpDecorate and OpMemberDecorate
// Used to keep track of all decorations applied to any instruction
struct DecorationBase {
    enum FlagBit {
        patch_bit = 1 << 0,
        block_bit = 1 << 1,
        buffer_block_bit = 1 << 2,
        nonwritable_bit = 1 << 3,
        nonreadable_bit = 1 << 4,
        per_vertex_bit = 1 << 5,
        passthrough_bit = 1 << 6,
        aliased_bit = 1 << 7,
        input_attachment_bit = 1 << 8,
        per_task_nv = 1 << 9,
        per_primitive_ext = 1 << 10,
    };
    static constexpr uint32_t kInvalidValue = std::numeric_limits<uint32_t>::max();

    // bits to know if things have been set or not by a Decoration
    uint32_t flags = 0;

    // When being used as an User-defined Variable (input, output, rtx)
    uint32_t location = kInvalidValue;
    uint32_t component = 0;

    // A given object can only have a single BuiltIn OpDecoration
    uint32_t builtin = kInvalidValue;

    void Add(uint32_t decoration, uint32_t value);
    bool Has(FlagBit flag_bit) const { return (flags & flag_bit) != 0; }
};

// subset only for OpDecorate
// Can't have nested structs with OpMemberDecorate, this class prevents accidently creating a 2nd level of member decorations,
struct DecorationSet : public DecorationBase {
    // For descriptors
    uint32_t set = 0;
    uint32_t binding = 0;

    // Value of InputAttachmentIndex the variable starts
    uint32_t input_attachment_index_start = kInvalidValue;

    // <index into struct, DecorationBase>
    vvl::unordered_map<uint32_t, DecorationBase> member_decorations;

    void Add(uint32_t decoration, uint32_t value);
    bool HasBuiltIn() const;
};

// A slot is a <Location, Component> mapping
struct InterfaceSlot {
    // A Location is made up of 4 Components
    // Example: Location 2, Component 1
    // L0 : [ C0, C1, C2, C3 ]
    // L1 : [ C0, C1, C2, C3 ]
    // L2 : [ C0, C1, C2, C3 ]
    //            ^
    // index == 9 == (Location * 4) + Component
    const uint32_t slot = 0;  // default

    // Information about the variable type
    // Easier to find this information once then re-look each time (mainly for structs)
    const uint32_t type = 0;  // Opcode of OpType*
    const uint32_t bit_width = 0;

    uint32_t Location() const { return slot / 4; }
    uint32_t Component() const { return slot % 4; }
    std::string Describe() const;
    // Having a single uint32_t slot allows a 64-bit Vec3 to pass in (Loc 0, Comp 5) and have it automatically mean (Loc 1, Comp 1)
    InterfaceSlot(uint32_t location, uint32_t component, uint32_t type, uint32_t bit_width)
        : slot(GetSlotValue(location, component)), type(type), bit_width(bit_width) {}
    InterfaceSlot(uint32_t slot, uint32_t type, uint32_t bit_width) : slot(slot), type(type), bit_width(bit_width) {}

    bool operator<(const InterfaceSlot &rhs) const { return slot < rhs.slot; }
    bool operator==(const InterfaceSlot &rhs) const { return slot == rhs.slot; }
    struct Hash {
        std::size_t operator()(const InterfaceSlot &object) const { return object.slot; }
    };

    uint32_t GetSlotValue(uint32_t location, uint32_t component) { return (location * 4) + component; }
};

// Common info needed for all OpVariable
struct VariableBase {
    const uint32_t id;
    const uint32_t type_id;
    const spv::StorageClass storage_class;
    const DecorationSet &decorations;
    const Instruction *struct_type;  // null if no struct type
    // If variable is accessed from mulitple entrypoint, create a seperate VariableBase object as info about it being access will be
    // different
    const VkShaderStageFlagBits stage;
    VariableBase(const SHADER_MODULE_STATE &module_state, const Instruction &insn, VkShaderStageFlagBits stage);
};

// These are Input/Output OpVariable that go in-between stages
// (also for example the input to a Vertex and output of the Fragment).
// These are always ints/floats (not images or samplers).
// Besides the input vertex binding, all of these are fully known at pipeline creation time
//
// These include both BuiltIns and User Defined, while there are difference in member variables, the variables are needed for the
// common logic so its easier using the same object in the end
struct StageInteraceVariable : public VariableBase {
    // Only will be true in BuiltIns
    const bool is_patch;
    const bool is_per_vertex;   // VK_KHR_fragment_shader_barycentric
    const bool is_per_task_nv;  // VK_NV_mesh_shader

    const bool is_array_interface;
    const Instruction &base_type;
    const bool is_builtin;

    const std::vector<InterfaceSlot> interface_slots;  // Only for User Defined variables
    const std::vector<uint32_t> builtin_block;
    uint32_t total_builtin_components = 0;

    StageInteraceVariable(const SHADER_MODULE_STATE &module_state, const Instruction &insn, VkShaderStageFlagBits stage);

  protected:
    static bool IsArrayInterface(const StageInteraceVariable &variable);
    static const Instruction &FindBaseType(const StageInteraceVariable &variable, const SHADER_MODULE_STATE &module_state);
    static bool IsBuiltin(const StageInteraceVariable &variable, const SHADER_MODULE_STATE &module_state);
    static std::vector<InterfaceSlot> GetInterfaceSlots(const StageInteraceVariable &variable,
                                                        const SHADER_MODULE_STATE &module_state);
    static std::vector<uint32_t> GetBuiltinBlock(const StageInteraceVariable &variable, const SHADER_MODULE_STATE &module_state);
    static uint32_t GetBuiltinComponents(const StageInteraceVariable &variable, const SHADER_MODULE_STATE &module_state);
};

// vkspec.html#interfaces-resources describes 'Shader Resource Interface'
// These are the OpVariable attached to descriptors.
// The slots are known at Pipeline creation time, but the type images/sampler/etc is
// not known until the descriptors are bound.
// The main purpose of this struct is to track what operations are statically done so
// at draw/submit time we can cross reference with the last bound descriptor.
struct ResourceInterfaceVariable : public VariableBase {
    // If the type is a OpTypeArray save the length
    uint32_t array_length = 0;
    bool runtime_array = false;  // OpTypeRuntimeArray - can't validate length until run time

    // List of samplers that sample a given image. The index of array is index of image.
    std::vector<vvl::unordered_set<SamplerUsedByImage>> samplers_used_by_image;

    // For storage images - list of < OpImageWrite : Texel component length >
    std::vector<std::pair<Instruction, uint32_t>> write_without_formats_component_count_list;

    // A variable can have an array of indexes, need to track which are written to
    // can't use bitset because number of indexes isn't known until runtime
    // This array will match the OpTypeArray and not consider the InputAttachmentIndex
    std::vector<bool> input_attachment_index_read;

    // Sampled Type width of the OpTypeImage the variable points to, 0 if doesn't use the image
    uint32_t image_sampled_type_width = 0;

    // Type once array/pointer are stripped
    // most likly will be OpTypeImage, OpTypeStruct, OpTypeSampler, or OpTypeAccelerationStructureKHR
    spv::Op base_type;

    bool is_read_from{false};   // has operation to reads from the variable
    bool is_written_to{false};  // has operation to writes to the variable

    // Type of resource type (vkspec.html#interfaces-resources-storage-class-correspondence)
    bool is_storage_image{false};
    bool is_storage_texel_buffer{false};
    bool is_storage_buffer{false};
    bool is_input_attachment{false};

    bool is_atomic_operation{false};
    bool is_sampler_sampled{false};
    bool is_sampler_implicitLod_dref_proj{false};
    bool is_sampler_bias_offset{false};
    bool is_read_without_format{false};   // For storage images
    bool is_write_without_format{false};  // For storage images
    bool is_dref_operation{false};

    ResourceInterfaceVariable(const SHADER_MODULE_STATE &module_state, const Instruction &insn, VkShaderStageFlagBits stage);
};

enum FORMAT_TYPE {
    FORMAT_TYPE_FLOAT = 1,  // UNORM, SNORM, FLOAT, USCALED, SSCALED, SRGB -- anything we consider float in the shader
    FORMAT_TYPE_SINT = 2,
    FORMAT_TYPE_UINT = 4,
};

struct SHADER_MODULE_STATE : public BASE_NODE {
    // Contains all the details for a OpTypeStruct
    struct StructInfo {
        uint32_t offset;
        uint32_t size;                                 // A scalar size or a struct size. Not consider array
        std::vector<uint32_t> array_length_hierarchy;  // multi-dimensional array, mat, vec. mat is combined with 2 array.
                                                       // e.g :array[2] -> {2}, array[2][3][4] -> {2,3,4}, mat4[2] ->{2,4,4},
        std::vector<uint32_t> array_block_size;        // When index increases, how many data increases.
                                                 // e.g : array[2][3][4] -> {12,4,1}, it means if the first index increases one, the
                                                 // array gets 12 data. If the second index increases one, the array gets 4 data.

        // OpTypeStruct can have OpTypeStruct inside it so need to track the struct-in-struct chain
        std::vector<StructInfo> struct_members;  // If the data is not a struct, it's empty.
        StructInfo *root;

        StructInfo() : offset(0), size(0), root(nullptr) {}

        bool IsUsed() const {
            if (!root) return false;
            return !root->used_bytes.empty();
        }

        std::vector<uint8_t> *GetUsedbytes() const {
            if (!root) return nullptr;
            return &root->used_bytes;
        }

        std::string GetLocationDesc(uint32_t index_used_bytes) const {
            std::string desc = "";
            if (array_length_hierarchy.size() > 0) {
                desc += " index:";
                for (const auto block_size : array_block_size) {
                    desc += "[";
                    desc += std::to_string(index_used_bytes / (block_size * size));
                    desc += "]";
                    index_used_bytes = index_used_bytes % (block_size * size);
                }
            }
            const int struct_members_size = static_cast<int>(struct_members.size());
            if (struct_members_size > 0) {
                desc += " member:";
                for (int i = struct_members_size - 1; i >= 0; --i) {
                    if (index_used_bytes > struct_members[i].offset) {
                        desc += std::to_string(i);
                        desc += struct_members[i].GetLocationDesc(index_used_bytes - struct_members[i].offset);
                        break;
                    }
                }
            } else {
                desc += " offset:";
                desc += std::to_string(index_used_bytes);
            }
            return desc;
        }

      private:
        std::vector<uint8_t> used_bytes;  // This only works for root. 0: not used. 1: used. The totally array * size.
    };

    struct EntryPoint {
        // "A module must not have two OpEntryPoint instructions with the same Execution Model and the same Name string."
        // There is no single unique item for a single entry point
        const Instruction &entrypoint_insn;  // OpEntryPoint instruction
        const VkShaderStageFlagBits stage;
        const uint32_t id;
        const std::string name;
        // All ids that can be accessed from the entry point
        vvl::unordered_set<uint32_t> accessible_ids;

        std::vector<ResourceInterfaceVariable> resource_interface_variables;
        std::vector<StageInteraceVariable> stage_interface_variables;
        // Easier to lookup without having to check for the is_builtin bool
        // "Built-in interface variables" - vkspec.html#interfaces-iointerfaces-builtin
        std::vector<const StageInteraceVariable *> built_in_variables;
        // "User-defined Variable Interface" - vkspec.html#interfaces-iointerfaces-user
        std::vector<const StageInteraceVariable *> user_defined_interface_variables;

        // Lookup map from Interface slot to the variable in that spot
        // spirv-val guarantees no overlap so 2 variables won't have same slot
        std::unordered_map<InterfaceSlot, const StageInteraceVariable *, InterfaceSlot::Hash> input_interface_slots;
        std::unordered_map<InterfaceSlot, const StageInteraceVariable *, InterfaceSlot::Hash> output_interface_slots;
        // Uesd for limit check
        const StageInteraceVariable *max_input_slot_variable = nullptr;
        const StageInteraceVariable *max_output_slot_variable = nullptr;
        const InterfaceSlot *max_input_slot = nullptr;
        const InterfaceSlot *max_output_slot = nullptr;
        uint32_t builtin_input_components = 0;
        uint32_t builtin_output_components = 0;

        StructInfo push_constant_used_in_shader;

        bool emit_vertex_geometry{false};

        // Mark if a BuiltIn is written to
        bool written_builtin_point_size{false};
        bool written_builtin_primitive_shading_rate_khr{false};
        bool written_builtin_viewport_index{false};
        bool written_builtin_viewport_mask_nv{false};

        bool has_passthrough{false};

        EntryPoint(const SHADER_MODULE_STATE &module_state, const Instruction &entrypoint);
    };

    // Static/const data extracted from a SPIRV module at initialization time
    // The goal of this struct is to move everything that is ready only into here
    struct StaticData {
        StaticData() = default;
        StaticData(const SHADER_MODULE_STATE &module_state);
        StaticData &operator=(StaticData &&) = default;
        StaticData(StaticData &&) = default;

        // List of all instructions in the order they appear in the binary
        std::vector<Instruction> instructions;
        // Instructions that can be referenced by Ids
        // A mapping of <id> to the first word of its def. this is useful because walking type
        // trees, constant expressions, etc requires jumping all over the instruction stream.
        vvl::unordered_map<uint32_t, const Instruction *> definitions;

        vvl::unordered_map<uint32_t, DecorationSet> decorations;
        DecorationSet empty_decoration;  // all zero values, allows use to return a reference and not a copy each time
        // <Specialization constant ID -> target ID> mapping
        vvl::unordered_map<uint32_t, uint32_t> spec_const_map;
        // Find all decoration instructions to prevent relooping module later - many checks need this info
        std::vector<const Instruction *> decoration_inst;
        std::vector<const Instruction *> member_decoration_inst;
        // Find all variable instructions to prevent relookping module later
        std::vector<const Instruction *> variable_inst;
        // Execution are not tied to an entry point and are their own mapping tied to entry point function
        // [OpEntryPoint function <id> operand] : [Execution Mode Instruction list]
        vvl::unordered_map<uint32_t, std::vector<const Instruction *>> execution_mode_inst;
        // both OpDecorate and OpMemberDecorate builtin instructions
        std::vector<const Instruction *> builtin_decoration_inst;
        // BuiltIn we just care about existing or not, don't have to be written to
        bool has_builtin_layer{false};
        bool has_builtin_fully_covered{false};
        bool has_builtin_workgroup_size{false};
        uint32_t builtin_workgroup_size_id = 0;

        std::vector<const Instruction *> atomic_inst;
        std::vector<spv::Capability> capability_list;

        bool has_specialization_constants{false};
        bool has_invocation_repack_instruction{false};

        // EntryPoint has pointer references inside it that need to be preserved
        std::vector<std::shared_ptr<EntryPoint>> entry_points;

        bool has_group_decoration{false};

        // Tracks accesses (load, store, atomic) to the instruction calling them
        // Example: the OpLoad does the "access" but need to know if a OpImageRead uses that OpLoad later
        std::vector<uint32_t> image_read_load_ids;
        std::vector<uint32_t> image_write_load_ids;
        vvl::unordered_map<const Instruction *, uint32_t> image_write_load_id_map;  // <OpImageWrite, load id>
        std::vector<uint32_t> atomic_pointer_ids;
        std::vector<uint32_t> store_pointer_ids;
        std::vector<uint32_t> atomic_store_pointer_ids;
        std::vector<uint32_t> sampler_load_ids;  // tracks all sampling operations
        std::vector<uint32_t> sampler_implicitLod_dref_proj_load_ids;
        std::vector<uint32_t> sampler_bias_offset_load_ids;
        std::vector<uint32_t> image_dref_load_ids;
        std::vector<std::pair<uint32_t, uint32_t>> sampled_image_load_ids;                       // <image, sampler>
        vvl::unordered_map<uint32_t, uint32_t> load_members;                              // <result id, pointer>
        vvl::unordered_map<uint32_t, std::pair<uint32_t, uint32_t>> accesschain_members;  // <result id, <base,index[0]>>
        vvl::unordered_map<uint32_t, uint32_t> image_texel_pointer_members;               // <result id, image>
    };

    // This is the SPIR-V module data content
    const std::vector<uint32_t> words_;

    const bool has_valid_spirv{false};
    const StaticData static_data_;

    uint32_t gpu_validation_shader_id{std::numeric_limits<uint32_t>::max()};

    explicit SHADER_MODULE_STATE(vvl::span<const uint32_t> code)
        : BASE_NODE(static_cast<VkShaderModule>(VK_NULL_HANDLE), kVulkanObjectTypeShaderModule),
          words_(code.begin(), code.end()),
          static_data_(*this) {}

    SHADER_MODULE_STATE(const VkShaderModuleCreateInfo &create_info, VkShaderModule shaderModule, uint32_t unique_shader_id)
        : BASE_NODE(shaderModule, kVulkanObjectTypeShaderModule),
          words_(create_info.pCode, create_info.pCode + create_info.codeSize / sizeof(uint32_t)),
          has_valid_spirv(true),
          static_data_(*this),
          gpu_validation_shader_id(unique_shader_id) {}

    SHADER_MODULE_STATE() : BASE_NODE(static_cast<VkShaderModule>(VK_NULL_HANDLE), kVulkanObjectTypeShaderModule) {}

    const Instruction *FindDef(uint32_t id) const {
        auto it = static_data_.definitions.find(id);
        if (it == static_data_.definitions.end()) return nullptr;
        return it->second;
    }

    const std::vector<Instruction> &GetInstructions() const { return static_data_.instructions; }
    const std::vector<const Instruction *> &GetDecorationInstructions() const { return static_data_.decoration_inst; }
    const std::vector<const Instruction *> &GetAtomicInstructions() const { return static_data_.atomic_inst; }
    const std::vector<const Instruction *> &GetVariableInstructions() const { return static_data_.variable_inst; }
    const std::vector<ResourceInterfaceVariable> *GetResourceInterfaceVariable(const Instruction &entrypoint) const {
        for (const auto &entry_point : static_data_.entry_points) {
            if (entry_point->entrypoint_insn == entrypoint) {
                return &entry_point->resource_interface_variables;
            }
        }
        return nullptr;
    }

    const std::vector<const Instruction *> FindVariableAccesses(uint32_t variable_id, const std::vector<uint32_t> &access_ids,
                                                                bool atomic) const;

    const vvl::unordered_map<uint32_t, std::vector<const Instruction *>> &GetExecutionModeInstructions() const {
        return static_data_.execution_mode_inst;
    }

    const vvl::unordered_map<uint32_t, uint32_t> &GetSpecConstMap() const { return static_data_.spec_const_map; }

    bool HasSpecConstants() const { return static_data_.has_specialization_constants; }
    bool HasInvocationRepackInstruction() const { return static_data_.has_invocation_repack_instruction; }

    bool HasMultipleEntryPoints() const { return static_data_.entry_points.size() > 1; }

    VkShaderModule vk_shader_module() const { return handle_.Cast<VkShaderModule>(); }

    const DecorationSet &GetDecorationSet(uint32_t id) const {
        // return the actual decorations for this id, or a default empty set.
        const auto it = static_data_.decorations.find(id);
        return (it != static_data_.decorations.end()) ? it->second : static_data_.empty_decoration;
    }

    // Used to get human readable strings for error messages
    void DescribeTypeInner(std::ostringstream &ss, uint32_t type, uint32_t indent) const;
    std::string DescribeType(uint32_t type) const;

    std::optional<VkPrimitiveTopology> GetTopology(const Instruction &entrypoint) const;

    const StructInfo *FindEntrypointPushConstant(char const *name, VkShaderStageFlagBits stageBits) const;
    std::shared_ptr<const EntryPoint> FindEntrypoint(char const *name, VkShaderStageFlagBits stageBits) const;
    std::optional<Instruction> FindEntrypointInstruction(char const *name, VkShaderStageFlagBits stageBits) const;
    [[nodiscard]] bool FindLocalSize(const Instruction &entrypoint, uint32_t &local_size_x, uint32_t &local_size_y,
                                     uint32_t &local_size_z) const;

    const Instruction *GetConstantDef(uint32_t id) const;
    uint32_t GetConstantValueById(uint32_t id) const;
    spv::Dim GetShaderResourceDimensionality(const ResourceInterfaceVariable &resource) const;
    uint32_t GetLocationsConsumedByType(uint32_t type) const;
    uint32_t GetComponentsConsumedByType(uint32_t type) const;
    uint32_t GetFundamentalType(uint32_t type) const;
    const Instruction *GetStructType(const Instruction *insn) const;

    uint32_t DescriptorTypeToReqs(uint32_t type_id) const;

    bool IsBuiltInWritten(const Instruction *builtin_insn, const Instruction &entrypoint) const;

    uint32_t GetNumComponentsInBaseType(const Instruction *insn) const;
    uint32_t GetTypeBitsSize(const Instruction *insn) const;
    uint32_t GetTypeBytesSize(const Instruction *insn) const;
    uint32_t GetBaseType(const Instruction *insn) const;
    const Instruction *GetBaseTypeInstruction(uint32_t type) const;
    uint32_t GetTypeId(uint32_t id) const;
    uint32_t GetTexelComponentCount(const Instruction &insn) const;

    bool HasCapability(spv::Capability find_capability) const {
        return std::any_of(static_data_.capability_list.begin(), static_data_.capability_list.end(),
                           [find_capability](const spv::Capability &capability) { return capability == find_capability; });
    }

    // Used to set push constants values at shader module initialization time
    static void SetPushConstantUsedInShader(const SHADER_MODULE_STATE &module_state,
                                            std::vector<std::shared_ptr<SHADER_MODULE_STATE::EntryPoint>> &entry_points);

  private:
    // The following are all helper functions to set the push constants values by tracking if the values are accessed in the entry
    // point functions and which offset in the structs are used
    uint32_t UpdateOffset(uint32_t offset, const std::vector<uint32_t> &array_indices, const StructInfo &data) const;
    void SetUsedBytes(uint32_t offset, const std::vector<uint32_t> &array_indices, const StructInfo &data) const;
    void DefineStructMember(const Instruction *insn, StructInfo &data) const;
    void RunUsedArray(uint32_t offset, std::vector<uint32_t> array_indices, uint32_t access_chain_word_index,
                      const Instruction *access_chain, const StructInfo &data) const;
    void RunUsedStruct(uint32_t offset, uint32_t access_chain_word_index, const Instruction *access_chain,
                       const StructInfo &data) const;
    void SetUsedStructMember(const uint32_t variable_id, vvl::unordered_set<uint32_t> &accessible_ids,
                             const StructInfo &data) const;
};
