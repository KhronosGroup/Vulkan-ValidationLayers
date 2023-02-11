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
#ifndef VULKAN_SHADER_MODULE_H
#define VULKAN_SHADER_MODULE_H

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

// Used to keep track of all decorations applied to any instruction
struct DecorationSet {
    enum FlagBit {
        patch_bit = 1 << 0,
        block_bit = 1 << 1,
        buffer_block_bit = 1 << 2,
        nonwritable_bit = 1 << 3,
        builtin_bit = 1 << 4,
        nonreadable_bit = 1 << 5,
        per_vertex_bit = 1 << 6,
        passthrough_bit = 1 << 7,
        aliased_bit = 1 << 8,
    };
    static constexpr uint32_t kInvalidValue = std::numeric_limits<uint32_t>::max();

    // bits to know if things have been set or not by a Decoration
    uint32_t flags = 0;

    // When being used as an User-defined Variable (input, output, rtx)
    uint32_t location = kInvalidValue;
    uint32_t component = 0;

    // For input attachments
    uint32_t input_attachment_index = 0;

    // For descriptors
    uint32_t set = 0;
    uint32_t binding = 0;

    uint32_t builtin = kInvalidValue;

    void Add(uint32_t decoration, uint32_t value);
    bool Has(FlagBit flag_bit) const { return (flags & flag_bit) != 0; }
};

// vkspec.html#interfaces-iointerfaces-user describes 'User-defined Variable Interface'
// These are Input/Output OpVariable that go in-between stages
// (also for example the input to a Vertex and output of the Fragment).
// These are always ints/floats (not images or samplers).
// Besides the input vertex binding, all of these are fully known at pipeline creation time
struct UserDefinedInterfaceVariable {
    uint32_t id;
    uint32_t type_id;

    // if a block with multiple location, track which offset to only check the first
    uint32_t offset;

    bool is_patch{false};

    UserDefinedInterfaceVariable() : id(0), type_id(0), offset(0) {}

    UserDefinedInterfaceVariable(const Instruction *insn) : id(insn->Word(2)), type_id(insn->Word(1)), offset(0) {}
};

// vkspec.html#interfaces-resources describes 'Shader Resource Interface'
// These are the OpVariable attached to descriptors.
// The slots are known at Pipeline creation time, but the type images/sampler/etc is
// not known until the descriptors are bound.
// The main purpose of this struct is to track what operations are statically done so
// at draw/submit time we can cross reference with the last bound descriptor.
struct ResourceInterfaceVariable {
    uint32_t id;
    uint32_t type_id;
    spv::StorageClass storage_class;

    VkShaderStageFlagBits stage;
    DecorationSet decorations;

    // List of samplers that sample a given image. The index of array is index of image.
    std::vector<vvl::unordered_set<SamplerUsedByImage>> samplers_used_by_image;

    // For storage images - list of < OpImageWrite : Texel component length >
    std::vector<std::pair<Instruction, uint32_t>> write_without_formats_component_count_list;

    // Sampled Type width of the OpTypeImage the variable points to, 0 if doesn't use the image
    uint32_t image_sampled_type_width = 0;

    bool is_readable{false};
    bool is_writable{false};
    bool is_atomic_operation{false};
    bool is_sampler_sampled{false};
    bool is_sampler_implicitLod_dref_proj{false};
    bool is_sampler_bias_offset{false};
    bool is_read_without_format{false};   // For storage images
    bool is_write_without_format{false};  // For storage images
    bool is_dref_operation{false};

    ResourceInterfaceVariable(const SHADER_MODULE_STATE &module_state, const Instruction *insn, VkShaderStageFlagBits stage);
};

std::vector<uint32_t> FindEntrypointInterfaces(const Instruction &entrypoint);

enum FORMAT_TYPE {
    FORMAT_TYPE_FLOAT = 1,  // UNORM, SNORM, FLOAT, USCALED, SSCALED, SRGB -- anything we consider float in the shader
    FORMAT_TYPE_SINT = 2,
    FORMAT_TYPE_UINT = 4,
};

// <Location, Component>
typedef std::pair<uint32_t, uint32_t> location_t;

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
            return root->used_bytes.size() ? true : false;
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
        const std::string name;
        // All ids that can be accessed from the entry point
        vvl::unordered_set<uint32_t> accessible_ids;

        std::vector<UserDefinedInterfaceVariable> user_defined_interface_variables;
        std::vector<ResourceInterfaceVariable> resource_interface_variables;
        vvl::unordered_set<uint32_t> attachment_indexes;

        StructInfo push_constant_used_in_shader;

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
        std::vector<const Instruction *> atomic_inst;
        std::vector<spv::Capability> capability_list;

        bool has_specialization_constants{false};
        bool has_invocation_repack_instruction{false};

        std::vector<EntryPoint> entry_points;

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

    explicit SHADER_MODULE_STATE(vvl::span<const uint32_t> code, spv_target_env env = SPV_ENV_VULKAN_1_0)
        : BASE_NODE(static_cast<VkShaderModule>(VK_NULL_HANDLE), kVulkanObjectTypeShaderModule),
          words_(code.begin(), code.end()),
          static_data_(*this) {
        PreprocessShaderBinary(env);
    }

    SHADER_MODULE_STATE(const VkShaderModuleCreateInfo &create_info, VkShaderModule shaderModule, spv_target_env env,
                        uint32_t unique_shader_id)
        : BASE_NODE(shaderModule, kVulkanObjectTypeShaderModule),
          words_(create_info.pCode, create_info.pCode + create_info.codeSize / sizeof(uint32_t)),
          has_valid_spirv(true),
          static_data_(*this),
          gpu_validation_shader_id(unique_shader_id) {
        PreprocessShaderBinary(env);
    }

    SHADER_MODULE_STATE() : BASE_NODE(static_cast<VkShaderModule>(VK_NULL_HANDLE), kVulkanObjectTypeShaderModule) {}

    const Instruction *FindDef(uint32_t id) const {
        auto it = static_data_.definitions.find(id);
        if (it == static_data_.definitions.end()) return nullptr;
        return it->second;
    }

    const std::vector<Instruction> &GetInstructions() const { return static_data_.instructions; }
    const std::vector<const Instruction *> &GetDecorationInstructions() const { return static_data_.decoration_inst; }
    const std::vector<const Instruction *> &GetMemberDecorationInstructions() const { return static_data_.member_decoration_inst; }
    const std::vector<const Instruction *> &GetAtomicInstructions() const { return static_data_.atomic_inst; }
    const std::vector<const Instruction *> &GetVariableInstructions() const { return static_data_.variable_inst; }
    const vvl::unordered_set<uint32_t> *GetAccessibleIds(const Instruction &entrypoint) const {
        for (const auto &entry_point : static_data_.entry_points) {
            if (entry_point.entrypoint_insn == entrypoint) {
                return &entry_point.accessible_ids;
            }
        }
        return nullptr;
    }
    const vvl::unordered_set<uint32_t> *GetAttachmentIndexes(const Instruction &entrypoint) const {
        for (const auto &entry_point : static_data_.entry_points) {
            if (entry_point.entrypoint_insn == entrypoint) {
                return &entry_point.attachment_indexes;
            }
        }
        return nullptr;
    }
    const std::vector<ResourceInterfaceVariable> *GetResourceInterfaceVariable(const Instruction &entrypoint) const {
        for (const auto &entry_point : static_data_.entry_points) {
            if (entry_point.entrypoint_insn == entrypoint) {
                return &entry_point.resource_interface_variables;
            }
        }
        return nullptr;
    }

    const vvl::unordered_map<uint32_t, std::vector<const Instruction *>> &GetExecutionModeInstructions() const {
        return static_data_.execution_mode_inst;
    }

    const std::vector<const Instruction *> &GetBuiltinDecorationList() const { return static_data_.builtin_decoration_inst; }

    const vvl::unordered_map<uint32_t, uint32_t> &GetSpecConstMap() const { return static_data_.spec_const_map; }

    bool HasSpecConstants() const { return static_data_.has_specialization_constants; }
    bool HasInvocationRepackInstruction() const { return static_data_.has_invocation_repack_instruction; }

    bool HasMultipleEntryPoints() const { return static_data_.entry_points.size() > 1; }

    VkShaderModule vk_shader_module() const { return handle_.Cast<VkShaderModule>(); }

    DecorationSet GetDecorationSet(uint32_t id) const {
        // return the actual decorations for this id, or a default set.
        auto it = static_data_.decorations.find(id);
        if (it != static_data_.decorations.end()) return it->second;
        return DecorationSet();
    }

    // Used to get human readable strings for error messages
    void DescribeTypeInner(std::ostringstream &ss, uint32_t type) const;
    std::string DescribeType(uint32_t type) const;

    std::optional<VkPrimitiveTopology> GetTopology(const Instruction &entrypoint) const;
    // TODO (https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/2450)
    // Since we currently don't support multiple entry points, this is a helper to return the topology
    // for the "first" (and for our purposes _only_) entrypoint.
    std::optional<VkPrimitiveTopology> GetTopology() const;

    const StructInfo *FindEntrypointPushConstant(char const *name, VkShaderStageFlagBits stageBits) const;
    std::optional<Instruction> FindEntrypoint(char const *name, VkShaderStageFlagBits stageBits) const;
    bool FindLocalSize(const Instruction &entrypoint, uint32_t &local_size_x, uint32_t &local_size_y, uint32_t &local_size_z) const;

    const Instruction *GetConstantDef(uint32_t id) const;
    uint32_t GetConstantValueById(uint32_t id) const;
    spv::Dim GetShaderResourceDimensionality(const ResourceInterfaceVariable &resource) const;
    uint32_t GetLocationsConsumedByType(uint32_t type, bool strip_array_level) const;
    uint32_t GetComponentsConsumedByType(uint32_t type, bool strip_array_level) const;
    uint32_t GetFundamentalType(uint32_t type) const;
    const Instruction *GetStructType(const Instruction *insn, bool is_array_of_verts) const;

    uint32_t DescriptorTypeToReqs(uint32_t type_id) const;

    bool IsBuiltInWritten(const Instruction *builtin_insn, const Instruction &entrypoint) const;

    // State tracking helpers for collecting interface information
    vvl::unordered_set<uint32_t> CollectWritableOutputLocationinFS(const Instruction &entrypoint) const;
    bool CollectInterfaceBlockMembers(std::map<location_t, UserDefinedInterfaceVariable> *out, bool is_array_of_verts,
                                      bool is_patch, const Instruction *variable_insn) const;
    std::map<location_t, UserDefinedInterfaceVariable> CollectInterfaceByLocation(const Instruction &entrypoint,
                                                                                  spv::StorageClass sinterface,
                                                                                  bool is_array_of_verts) const;
    std::vector<uint32_t> CollectBuiltinBlockMembers(const Instruction &entrypoint, uint32_t storageClass) const;

    uint32_t GetNumComponentsInBaseType(const Instruction *insn) const;
    uint32_t GetTypeBitsSize(const Instruction *insn) const;
    uint32_t GetTypeBytesSize(const Instruction *insn) const;
    uint32_t GetBaseType(const Instruction *insn) const;
    uint32_t GetTypeId(uint32_t id) const;
    uint32_t GetTexelComponentCount(const Instruction &insn) const;

    bool WritesToGlLayer() const {
        return std::any_of(static_data_.builtin_decoration_inst.begin(), static_data_.builtin_decoration_inst.end(),
                           [](const Instruction *insn) { return insn->GetBuiltIn() == spv::BuiltInLayer; });
    }

    bool HasCapability(spv::Capability find_capability) const {
        return std::any_of(static_data_.capability_list.begin(), static_data_.capability_list.end(),
                           [find_capability](const spv::Capability &capability) { return capability == find_capability; });
    }

    // Used to set push constants values at shader module initialization time
    static void SetPushConstantUsedInShader(const SHADER_MODULE_STATE &module_state,
                                            std::vector<SHADER_MODULE_STATE::EntryPoint> &entry_points);

  private:
    // Functions used for initialization only
    // Used to populate the shader module object
    void PreprocessShaderBinary(spv_target_env env);

    // The following are all helper functions to set the push constants values by tracking if the values are accessed in the entry
    // point functions and which offset in the structs are used
    uint32_t UpdateOffset(uint32_t offset, const std::vector<uint32_t> &array_indices, const StructInfo &data) const;
    void SetUsedBytes(uint32_t offset, const std::vector<uint32_t> &array_indices, const StructInfo &data) const;
    void DefineStructMember(const Instruction *insn, std::vector<const Instruction *> &member_decorate_insn,
                            StructInfo &data) const;
    void RunUsedArray(uint32_t offset, std::vector<uint32_t> array_indices, uint32_t access_chain_word_index,
                      const Instruction *access_chain, const StructInfo &data) const;
    void RunUsedStruct(uint32_t offset, uint32_t access_chain_word_index, const Instruction *access_chain,
                       const StructInfo &data) const;
    void SetUsedStructMember(const uint32_t variable_id, vvl::unordered_set<uint32_t> &accessible_ids,
                             const StructInfo &data) const;
};

#endif  // VULKAN_SHADER_MODULE_H
