/* Copyright (c) 2021-2022 The Khronos Group Inc.
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
 * Author: Spencer Fricke <s.fricke@samsung.com>
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

#include "shader_instruction.h"
#include "base_node.h"
#include "sampler_state.h"
#include <spirv/unified1/spirv.hpp>
#include "spirv-tools/optimizer.hpp"

class PIPELINE_STATE;

// A forward iterator over spirv instructions. Provides easy access to len, opcode, and content words
// without the caller needing to care too much about the physical SPIRV module layout.
//
// For more information of the physical module layout to help understand this struct:
// https://github.com/KhronosGroup/SPIRV-Guide/blob/master/chapters/parsing_instructions.md
struct spirv_inst_iter {
    std::vector<uint32_t>::const_iterator zero;
    std::vector<uint32_t>::const_iterator it;

    uint32_t len() const {
        auto result = *it >> 16;
        assert(result > 0);
        return result;
    }

    uint32_t opcode() const { return *it & 0x0ffffu; }

    uint32_t const &word(uint32_t n) const {
        assert(n < len());
        return it[n];
    }

    uint32_t offset() const { return (uint32_t)(it - zero); }

    spirv_inst_iter() {}

    spirv_inst_iter(std::vector<uint32_t>::const_iterator zero, std::vector<uint32_t>::const_iterator it) : zero(zero), it(it) {}

    bool operator==(spirv_inst_iter const &other) const { return it == other.it; }

    bool operator!=(spirv_inst_iter const &other) const { return it != other.it; }

    bool operator!=(std::vector<uint32_t>::const_iterator other) const { return it != other; }

    spirv_inst_iter operator++(int) {  // x++
        spirv_inst_iter ii = *this;
        it += len();
        return ii;
    }

    spirv_inst_iter operator++() {  // ++x;
        it += len();
        return *this;
    }

    // The iterator and the value are the same thing.
    spirv_inst_iter &operator*() { return *this; }
    spirv_inst_iter const &operator*() const { return *this; }
};

// Information about a OpVariable used as an interface in the shader
struct InterfaceVariable {
    uint32_t id;
    uint32_t type_id;

    uint32_t offset;

    // List of samplers that sample a given image. The index of array is index of image.
    std::vector<layer_data::unordered_set<SamplerUsedByImage>> samplers_used_by_image;

    // For storage images - list of < OpImageWrite : Texel component length >
    std::vector<std::pair<Instruction, uint32_t>> write_without_formats_component_count_list;

    bool is_patch{false};
    bool is_block_member{false};
    bool is_relaxed_precision{false};
    bool is_readable{false};
    bool is_writable{false};
    bool is_atomic_operation{false};
    bool is_sampler_sampled{false};
    bool is_sampler_implicitLod_dref_proj{false};
    bool is_sampler_bias_offset{false};
    bool is_read_without_format{false};   // For storage images
    bool is_write_without_format{false};  // For storage images
    bool is_dref_operation{false};

    InterfaceVariable() : id(0), type_id(0), offset(0) {}

    InterfaceVariable(const Instruction *insn) : id(insn->Word(2)), type_id(insn->Word(1)), offset(0) {}
};

std::vector<uint32_t> FindEntrypointInterfaces(const Instruction &entrypoint);

enum FORMAT_TYPE {
    FORMAT_TYPE_FLOAT = 1,  // UNORM, SNORM, FLOAT, USCALED, SSCALED, SRGB -- anything we consider float in the shader
    FORMAT_TYPE_SINT = 2,
    FORMAT_TYPE_UINT = 4,
};

// <Location, Component>
typedef std::pair<uint32_t, uint32_t> location_t;

struct DecorationSet {
    enum {
        location_bit = 1 << 0,
        patch_bit = 1 << 1,
        relaxed_precision_bit = 1 << 2,
        block_bit = 1 << 3,
        buffer_block_bit = 1 << 4,
        component_bit = 1 << 5,
        input_attachment_index_bit = 1 << 6,
        descriptor_set_bit = 1 << 7,
        binding_bit = 1 << 8,
        nonwritable_bit = 1 << 9,
        builtin_bit = 1 << 10,
        nonreadable_bit = 1 << 11,
        per_vertex_bit = 1 << 12,
        passthrough_bit = 1 << 13,
        aliased_bit = 1 << 14,
    };
    static constexpr uint32_t kInvalidValue = std::numeric_limits<uint32_t>::max();

    uint32_t flags = 0;
    uint32_t location = kInvalidValue;
    uint32_t component = 0;
    uint32_t input_attachment_index = 0;
    uint32_t descriptor_set = 0;
    uint32_t binding = 0;
    uint32_t builtin = kInvalidValue;
    uint32_t spec_const_id = kInvalidValue;

    void Add(uint32_t decoration, uint32_t value);
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
        layer_data::unordered_set<uint32_t> accessible_ids;

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
        layer_data::unordered_map<uint32_t, const Instruction *> definitions;

        layer_data::unordered_map<uint32_t, DecorationSet> decorations;
        // <Specialization constant ID -> target ID> mapping
        layer_data::unordered_map<uint32_t, uint32_t> spec_const_map;
        // Find all decoration instructions to prevent relooping module later - many checks need this info
        std::vector<const Instruction *> decoration_inst;
        std::vector<const Instruction *> member_decoration_inst;
        // Find all variable instructions to prevent relookping module later
        std::vector<const Instruction *> variable_inst;
        // Execution are not tied to an entry point and are their own mapping tied to entry point function
        // [OpEntryPoint function <id> operand] : [Execution Mode Instruction list]
        layer_data::unordered_map<uint32_t, std::vector<const Instruction *>> execution_mode_inst;
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
        layer_data::unordered_map<const Instruction *, uint32_t> image_write_load_id_map;  // <OpImageWrite, load id>
        std::vector<uint32_t> atomic_pointer_ids;
        std::vector<uint32_t> store_pointer_ids;
        std::vector<uint32_t> atomic_store_pointer_ids;
        std::vector<uint32_t> sampler_load_ids;  // tracks all sampling operations
        std::vector<uint32_t> sampler_implicitLod_dref_proj_load_ids;
        std::vector<uint32_t> sampler_bias_offset_load_ids;
        std::vector<uint32_t> image_dref_load_ids;
        std::vector<std::pair<uint32_t, uint32_t>> sampled_image_load_ids;                       // <image, sampler>
        layer_data::unordered_map<uint32_t, uint32_t> load_members;                              // <result id, pointer>
        layer_data::unordered_map<uint32_t, std::pair<uint32_t, uint32_t>> accesschain_members;  // <result id, <base,index[0]>>
        layer_data::unordered_map<uint32_t, uint32_t> image_texel_pointer_members;               // <result id, image>
    };

    // This is the SPIR-V module data content
    const std::vector<uint32_t> words_;

    const bool has_valid_spirv{false};
    const StaticData static_data_;

    uint32_t gpu_validation_shader_id{std::numeric_limits<uint32_t>::max()};

    SHADER_MODULE_STATE(const uint32_t *code, std::size_t count, spv_target_env env = SPV_ENV_VULKAN_1_0)
        : BASE_NODE(static_cast<VkShaderModule>(VK_NULL_HANDLE), kVulkanObjectTypeShaderModule),
          words_(code, code + (count / sizeof(uint32_t))),
          static_data_(*this) {
        PreprocessShaderBinary(env);
    }

    template <typename SpirvContainer>
    SHADER_MODULE_STATE(const SpirvContainer &spirv)
        : SHADER_MODULE_STATE(spirv.data(), spirv.size() * sizeof(typename SpirvContainer::value_type)) {}

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
    const layer_data::unordered_set<uint32_t> *GetAccessibleIds(const Instruction &entrypoint) const {
        for (const auto &entry_point : static_data_.entry_points) {
            if (entry_point.entrypoint_insn == entrypoint) {
                return &entry_point.accessible_ids;
            }
        }
        return nullptr;
    }

    const layer_data::unordered_map<uint32_t, std::vector<const Instruction *>> &GetExecutionModeInstructions() const {
        return static_data_.execution_mode_inst;
    }

    const std::vector<const Instruction *> &GetBuiltinDecorationList() const { return static_data_.builtin_decoration_inst; }

    const layer_data::unordered_map<uint32_t, uint32_t> &GetSpecConstMap() const { return static_data_.spec_const_map; }

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

    // Expose begin() / end() to enable range-based for
    spirv_inst_iter begin() const { return spirv_inst_iter(words_.begin(), words_.begin() + 5); }  // First insn
    spirv_inst_iter end() const { return spirv_inst_iter(words_.begin(), words_.end()); }          // Just past last insn

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
    int32_t GetShaderResourceDimensionality(const InterfaceVariable &resource) const;
    uint32_t GetLocationsConsumedByType(uint32_t type, bool strip_array_level) const;
    uint32_t GetComponentsConsumedByType(uint32_t type, bool strip_array_level) const;
    uint32_t GetFundamentalType(uint32_t type) const;
    const Instruction *GetStructType(const Instruction *insn, bool is_array_of_verts) const;

    uint32_t DescriptorTypeToReqs(uint32_t type_id) const;

    bool IsBuiltInWritten(const Instruction *builtin_insn, const Instruction &entrypoint) const;

    // State tracking helpers for collecting interface information
    void FindVariableDescriptorType(bool is_storage_buffer, InterfaceVariable &interface_var) const;
    std::vector<std::pair<DescriptorSlot, InterfaceVariable>> CollectInterfaceByDescriptorSlot(
        std::optional<Instruction> entrypoint) const;
    layer_data::unordered_set<uint32_t> CollectWritableOutputLocationinFS(const Instruction &entrypoint) const;
    bool CollectInterfaceBlockMembers(std::map<location_t, InterfaceVariable> *out, bool is_array_of_verts, bool is_patch,
                                      const Instruction *variable_insn) const;
    std::map<location_t, InterfaceVariable> CollectInterfaceByLocation(const Instruction &entrypoint, spv::StorageClass sinterface,
                                                                       bool is_array_of_verts) const;
    std::vector<uint32_t> CollectBuiltinBlockMembers(const Instruction &entrypoint, uint32_t storageClass) const;
    std::vector<std::pair<uint32_t, InterfaceVariable>> CollectInterfaceByInputAttachmentIndex(const Instruction &entrypoint) const;

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

    bool HasInputAttachmentCapability() const {
        return std::any_of(static_data_.capability_list.begin(), static_data_.capability_list.end(),
                           [](const spv::Capability &capability) { return capability == spv::CapabilityInputAttachment; });
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
    void SetUsedStructMember(const uint32_t variable_id, layer_data::unordered_set<uint32_t> &accessible_ids,
                             const StructInfo &data) const;
};

#endif  // VULKAN_SHADER_MODULE_H
