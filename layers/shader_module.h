/* Copyright (c) 2021 The Khronos Group Inc.
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

#include "base_node.h"
#include "pipeline_state.h"
#include <spirv/unified1/spirv.hpp>
#include "spirv-tools/optimizer.hpp"

// A forward iterator over spirv instructions. Provides easy access to len, opcode, and content words
// without the caller needing to care too much about the physical SPIRV module layout.
struct spirv_inst_iter {
    std::vector<uint32_t>::const_iterator zero;
    std::vector<uint32_t>::const_iterator it;

    uint32_t len() const {
        auto result = *it >> 16;
        assert(result > 0);
        return result;
    }

    uint32_t opcode() const { return *it & 0x0ffffu; }

    uint32_t const &word(unsigned n) const {
        assert(n < len());
        return it[n];
    }

    uint32_t offset() const { return (uint32_t)(it - zero); }

    spirv_inst_iter() {}

    spirv_inst_iter(std::vector<uint32_t>::const_iterator zero, std::vector<uint32_t>::const_iterator it) : zero(zero), it(it) {}

    bool operator==(spirv_inst_iter const &other) const { return it == other.it; }

    bool operator!=(spirv_inst_iter const &other) const { return it != other.it; }

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

// Utils taking a spirv_inst_iter
uint32_t GetConstantValue(const spirv_inst_iter &itr);
std::vector<uint32_t> FindEntrypointInterfaces(const spirv_inst_iter &entrypoint);

enum FORMAT_TYPE {
    FORMAT_TYPE_FLOAT = 1,  // UNORM, SNORM, FLOAT, USCALED, SSCALED, SRGB -- anything we consider float in the shader
    FORMAT_TYPE_SINT = 2,
    FORMAT_TYPE_UINT = 4,
};

typedef std::pair<unsigned, unsigned> location_t;

struct decoration_set {
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

    void merge(decoration_set const &other);

    void add(uint32_t decoration, uint32_t value);
};

struct function_set {
    unsigned id;
    unsigned offset;
    unsigned length;
    std::unordered_multimap<uint32_t, uint32_t> op_lists;  // key: spv::Op,  value: offset

    function_set() : id(0), offset(0), length(0) {}
};

struct builtin_set {
    uint32_t offset;  // offset to instruction (OpDecorate or OpMemberDecorate)
    spv::BuiltIn builtin;

    builtin_set(uint32_t offset, spv::BuiltIn builtin) : offset(offset), builtin(builtin) {}
};

struct shader_struct_member {
    uint32_t offset;
    uint32_t size;                                 // A scalar size or a struct size. Not consider array
    std::vector<uint32_t> array_length_hierarchy;  // multi-dimensional array, mat, vec. mat is combined with 2 array.
                                                   // e.g :array[2] -> {2}, array[2][3][4] -> {2,3,4}, mat4[2] ->{2,4,4},
    std::vector<uint32_t> array_block_size;        // When index increases, how many data increases.
                                             // e.g : array[2][3][4] -> {12,4,1}, it means if the first index increases one, the
                                             // array gets 12 data. If the second index increases one, the array gets 4 data.
    std::vector<shader_struct_member> struct_members;  // If the data is not a struct, it's empty.
    shader_struct_member *root;

    shader_struct_member() : offset(0), size(0), root(nullptr) {}

    bool IsUsed() const {
        if (!root) return false;
        return root->used_bytes.size() ? true : false;
    }

    std::vector<uint8_t> *GetUsedbytes() const {
        if (!root) return nullptr;
        return &root->used_bytes;
    }

    std::string GetLocationDesc(uint32_t index_used_bytes) const;

  private:
    std::vector<uint8_t> used_bytes;  // This only works for root. 0: not used. 1: used. The totally array * size.
};

struct shader_module_used_operators;

struct SHADER_MODULE_STATE : public BASE_NODE {
    // The spirv image itself
    std::vector<uint32_t> words;
    // A mapping of <id> to the first word of its def. this is useful because walking type
    // trees, constant expressions, etc requires jumping all over the instruction stream.
    layer_data::unordered_map<unsigned, unsigned> def_index;
    layer_data::unordered_map<unsigned, decoration_set> decorations;
    // <Specialization constant ID -> target ID> mapping
    layer_data::unordered_map<uint32_t, uint32_t> spec_const_map;
    // Find all decoration instructions to prevent relooping module later - many checks need this info
    std::vector<spirv_inst_iter> decoration_inst;
    std::vector<spirv_inst_iter> member_decoration_inst;
    // Execution are not tied to an entry point and are their own mapping tied to entry point function
    // [OpEntryPoint function <id> operand] : [Execution Mode Instruction list]
    layer_data::unordered_map<uint32_t, std::vector<spirv_inst_iter>> execution_mode_inst;
    // both OpDecorate and OpMemberDecorate builtin instructions
    std::vector<builtin_set> builtin_decoration_list;
    struct EntryPoint {
        uint32_t offset;  // into module to get OpEntryPoint instruction
        VkShaderStageFlagBits stage;
        std::unordered_multimap<unsigned, unsigned> decorate_list;  // key: spv::Op,  value: offset
        std::vector<function_set> function_set_list;
        shader_struct_member push_constant_used_in_shader;
    };
    // entry point is not unqiue to single value so need multimap
    std::unordered_multimap<std::string, EntryPoint> entry_points;
    bool multiple_entry_points{false};
    bool has_valid_spirv;
    bool has_specialization_constants{false};
    uint32_t gpu_validation_shader_id;

    SHADER_MODULE_STATE(VkShaderModuleCreateInfo const *pCreateInfo, VkShaderModule shaderModule, spv_target_env env,
                        uint32_t unique_shader_id)
        : BASE_NODE(shaderModule, kVulkanObjectTypeShaderModule),
          words(),
          def_index(),
          has_valid_spirv(true),
          gpu_validation_shader_id(unique_shader_id) {
        words = PreprocessShaderBinary((uint32_t *)pCreateInfo->pCode, pCreateInfo->codeSize, env);
        BuildDefIndex();
    }

    SHADER_MODULE_STATE()
        : BASE_NODE(static_cast<VkShaderModule>(VK_NULL_HANDLE), kVulkanObjectTypeShaderModule),
          has_valid_spirv(false),
          gpu_validation_shader_id(UINT32_MAX) {}

    VkShaderModule vk_shader_module() const { return handle_.Cast<VkShaderModule>(); }

    decoration_set get_decorations(unsigned id) const {
        // return the actual decorations for this id, or a default set.
        auto it = decorations.find(id);
        if (it != decorations.end()) return it->second;
        return decoration_set();
    }

    // Expose begin() / end() to enable range-based for
    spirv_inst_iter begin() const { return spirv_inst_iter(words.begin(), words.begin() + 5); }  // First insn
    spirv_inst_iter end() const { return spirv_inst_iter(words.begin(), words.end()); }          // Just past last insn
    // Given an offset into the module, produce an iterator there.
    spirv_inst_iter at(unsigned offset) const { return spirv_inst_iter(words.begin(), words.begin() + offset); }

    // Gets an iterator to the definition of an id
    spirv_inst_iter get_def(unsigned id) const {
        auto it = def_index.find(id);
        if (it == def_index.end()) {
            return end();
        }
        return at(it->second);
    }

    // Used to populate the shader module object
    void BuildDefIndex();
    std::vector<uint32_t> PreprocessShaderBinary(uint32_t *src_binary, size_t binary_size, spv_target_env env);

    // Used to get human readable strings for error messages
    void DescribeTypeInner(std::ostringstream &ss, unsigned type) const;
    std::string DescribeType(unsigned type) const;

    layer_data::unordered_set<uint32_t> MarkAccessibleIds(spirv_inst_iter entrypoint) const;
    void ProcessExecutionModes(const spirv_inst_iter &entrypoint, PIPELINE_STATE *pipeline) const;

    const EntryPoint *FindEntrypointStruct(char const *name, VkShaderStageFlagBits stageBits) const;
    spirv_inst_iter FindEntrypoint(char const *name, VkShaderStageFlagBits stageBits) const;
    bool FindLocalSize(const spirv_inst_iter &entrypoint, uint32_t &local_size_x, uint32_t &local_size_y,
                       uint32_t &local_size_z) const;

    spirv_inst_iter GetConstantDef(unsigned id) const;
    uint32_t GetConstantValueById(unsigned id) const;
    int32_t GetShaderResourceDimensionality(const interface_var &resource) const;
    unsigned GetLocationsConsumedByType(unsigned type, bool strip_array_level) const;
    unsigned GetComponentsConsumedByType(unsigned type, bool strip_array_level) const;
    unsigned GetFundamentalType(unsigned type) const;
    spirv_inst_iter GetStructType(spirv_inst_iter def, bool is_array_of_verts) const;

    void DefineStructMember(const spirv_inst_iter &it, const std::vector<uint32_t> &memberDecorate_offsets,
                            shader_struct_member &data) const;
    void RunUsedArray(uint32_t offset, std::vector<uint32_t> array_indices, uint32_t access_chain_word_index,
                      spirv_inst_iter &access_chain_it, const shader_struct_member &data) const;
    void RunUsedStruct(uint32_t offset, uint32_t access_chain_word_index, spirv_inst_iter &access_chain_it,
                       const shader_struct_member &data) const;
    void SetUsedStructMember(const uint32_t variable_id, const std::vector<function_set> &function_set_list,
                             const shader_struct_member &data) const;

    // Push consants
    void SetPushConstantUsedInShader();

    uint32_t DescriptorTypeToReqs(uint32_t type_id) const;

    bool IsBuiltInWritten(spirv_inst_iter builtin_instr, spirv_inst_iter entrypoint) const;

    // State tracking helpers for collecting interface information
    void IsSpecificDescriptorType(const spirv_inst_iter &id_it, bool is_storage_buffer, bool is_check_writable,
                                  interface_var &out_interface_var, shader_module_used_operators &used_operators) const;
    std::vector<std::pair<descriptor_slot_t, interface_var>> CollectInterfaceByDescriptorSlot(
        layer_data::unordered_set<uint32_t> const &accessible_ids, bool *has_writable_descriptor,
        bool *has_atomic_descriptor) const;
    layer_data::unordered_set<uint32_t> CollectWritableOutputLocationinFS(const VkPipelineShaderStageCreateInfo &stage_info) const;
    bool CollectInterfaceBlockMembers(std::map<location_t, interface_var> *out, bool is_array_of_verts, uint32_t id,
                                      uint32_t type_id, bool is_patch, int first_location) const;
    std::map<location_t, interface_var> CollectInterfaceByLocation(spirv_inst_iter entrypoint, spv::StorageClass sinterface,
                                                                   bool is_array_of_verts) const;
    std::vector<uint32_t> CollectBuiltinBlockMembers(spirv_inst_iter entrypoint, uint32_t storageClass) const;
    std::vector<std::pair<uint32_t, interface_var>> CollectInterfaceByInputAttachmentIndex(
        layer_data::unordered_set<uint32_t> const &accessible_ids) const;
};

// TODO - Most things below are agnostic of even the shader module and more of pure SPIR-V utils
//        Stuff like this could be part of a future auto-generated file from the spirv grammar json
bool AtomicOperation(uint32_t opcode);
bool GroupOperation(uint32_t opcode);

#endif  // VULKAN_SHADER_MODULE_H
