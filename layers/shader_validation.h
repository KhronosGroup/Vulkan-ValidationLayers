/* Copyright (c) 2015-2019 The Khronos Group Inc.
 * Copyright (c) 2015-2019 Valve Corporation
 * Copyright (c) 2015-2019 LunarG, Inc.
 * Copyright (C) 2015-2019 Google Inc.
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
 * Author: Chris Forbes <chrisf@ijw.co.nz>
 */
#ifndef VULKAN_SHADER_VALIDATION_H
#define VULKAN_SHADER_VALIDATION_H

//////#include <SPIRV/spirv.hpp>
//////#include <spirv_tools_commit_id.h>
//////#include "spirv-tools/optimizer.hpp"

////////// A forward iterator over spirv instructions. Provides easy access to len, opcode, and content words
////////// without the caller needing to care too much about the physical SPIRV module layout.
////////struct spirv_inst_iter {
////////    std::vector<uint32_t>::const_iterator zero;
////////    std::vector<uint32_t>::const_iterator it;
////////
////////    uint32_t len() {
////////        auto result = *it >> 16;
////////        assert(result > 0);
////////        return result;
////////    }
////////
////////    uint32_t opcode() { return *it & 0x0ffffu; }
////////
////////    uint32_t const &word(unsigned n) {
////////        assert(n < len());
////////        return it[n];
////////    }
////////
////////    uint32_t offset() { return (uint32_t)(it - zero); }
////////
////////    spirv_inst_iter() {}
////////
////////    spirv_inst_iter(std::vector<uint32_t>::const_iterator zero, std::vector<uint32_t>::const_iterator it) : zero(zero), it(it) {}
////////
////////    bool operator==(spirv_inst_iter const &other) { return it == other.it; }
////////
////////    bool operator!=(spirv_inst_iter const &other) { return it != other.it; }
////////
////////    spirv_inst_iter operator++(int) {  // x++
////////        spirv_inst_iter ii = *this;
////////        it += len();
////////        return ii;
////////    }
////////
////////    spirv_inst_iter operator++() {  // ++x;
////////        it += len();
////////        return *this;
////////    }
////////
////////    // The iterator and the value are the same thing.
////////    spirv_inst_iter &operator*() { return *this; }
////////    spirv_inst_iter const &operator*() const { return *this; }
////////};
////////
////////struct SHADER_MODULE_STATE {
////////    // The spirv image itself
////////    std::vector<uint32_t> words;
////////    // A mapping of <id> to the first word of its def. this is useful because walking type
////////    // trees, constant expressions, etc requires jumping all over the instruction stream.
////////    std::unordered_map<unsigned, unsigned> def_index;
////////    bool has_valid_spirv;
////////    VkShaderModule vk_shader_module;
////////    uint32_t gpu_validation_shader_id;
////////
////////    // LUGMAL -- we don't need to do this, just return source.
////////    std::vector<uint32_t> PreprocessShaderBinary(uint32_t *src_binary, size_t binary_size, spv_target_env env) {
////////        std::vector<uint32_t> src(src_binary, src_binary + binary_size / sizeof(uint32_t));
////////
////////        // Return the original module.
////////        return src;
////////    }
////////
////////    SHADER_MODULE_STATE(VkShaderModuleCreateInfo const *pCreateInfo, VkShaderModule shaderModule, spv_target_env env,
////////                        uint32_t unique_shader_id)
////////        : words(PreprocessShaderBinary((uint32_t *)pCreateInfo->pCode, pCreateInfo->codeSize, env)),
////////          def_index(),
////////          has_valid_spirv(true),
////////          vk_shader_module(shaderModule),
////////          gpu_validation_shader_id(unique_shader_id) {
////////    }
////////
////////    SHADER_MODULE_STATE() : has_valid_spirv(false), vk_shader_module(VK_NULL_HANDLE) {}
////////
////////    // Expose begin() / end() to enable range-based for
////////    spirv_inst_iter begin() const { return spirv_inst_iter(words.begin(), words.begin() + 5); }  // First insn
////////    spirv_inst_iter end() const { return spirv_inst_iter(words.begin(), words.end()); }          // Just past last insn
////////    // Given an offset into the module, produce an iterator there.
////////    spirv_inst_iter at(unsigned offset) const { return spirv_inst_iter(words.begin(), words.begin() + offset); }
////////
////////    // Gets an iterator to the definition of an id
////////    spirv_inst_iter get_def(unsigned id) const {
////////        auto it = def_index.find(id);
////////        if (it == def_index.end()) {
////////            return end();
////////        }
////////        return at(it->second);
////////    }
////////};
////////
////////
////////typedef std::pair<unsigned, unsigned> descriptor_slot_t;

#endif  // VULKAN_SHADER_VALIDATION_H
