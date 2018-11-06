/* Copyright (c) 2015-2017 The Khronos Group Inc.
 * Copyright (c) 2015-2017 Valve Corporation
 * Copyright (c) 2015-2017 LunarG, Inc.
 * Copyright (C) 2015-2017 Google Inc.
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

#include <spirv_tools_commit_id.h>

// A forward iterator over spirv instructions. Provides easy access to len, opcode, and content words
// without the caller needing to care too much about the physical SPIRV module layout.
struct spirv_inst_iter {
    std::vector<uint32_t>::const_iterator zero;
    std::vector<uint32_t>::const_iterator it;

    uint32_t len() {
        auto result = *it >> 16;
        assert(result > 0);
        return result;
    }

    uint32_t opcode() { return *it & 0x0ffffu; }

    uint32_t const &word(unsigned n) {
        assert(n < len());
        return it[n];
    }

    uint32_t offset() { return (uint32_t)(it - zero); }

    spirv_inst_iter() {}

    spirv_inst_iter(std::vector<uint32_t>::const_iterator zero, std::vector<uint32_t>::const_iterator it) : zero(zero), it(it) {}

    bool operator==(spirv_inst_iter const &other) { return it == other.it; }

    bool operator!=(spirv_inst_iter const &other) { return it != other.it; }

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

struct shader_module {
    // The spirv image itself
    std::vector<uint32_t> words;
    // A mapping of <id> to the first word of its def. this is useful because walking type
    // trees, constant expressions, etc requires jumping all over the instruction stream.
    std::unordered_map<unsigned, unsigned> def_index;
    bool has_valid_spirv;
    VkShaderModule vk_shader_module;

    shader_module(VkShaderModuleCreateInfo const *pCreateInfo, VkShaderModule shaderModule)
        : words((uint32_t *)pCreateInfo->pCode, (uint32_t *)pCreateInfo->pCode + pCreateInfo->codeSize / sizeof(uint32_t)),
          def_index(),
          has_valid_spirv(true),
          vk_shader_module(shaderModule) {
        BuildDefIndex();
    }

    shader_module() : has_valid_spirv(false), vk_shader_module(VK_NULL_HANDLE) {}

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

    void BuildDefIndex();
};

class ValidationCache {
    // hashes of shaders that have passed validation before, and can be skipped.
    // we don't store negative results, as we would have to also store what was
    // wrong with them; also, we expect they will get fixed, so we're less
    // likely to see them again.
    std::unordered_set<uint32_t> good_shader_hashes;
    ValidationCache() {}

   public:
    static VkValidationCacheEXT Create(VkValidationCacheCreateInfoEXT const *pCreateInfo) {
        auto cache = new ValidationCache();
        cache->Load(pCreateInfo);
        return VkValidationCacheEXT(cache);
    }

    void Load(VkValidationCacheCreateInfoEXT const *pCreateInfo) {
        const auto headerSize = 2 * sizeof(uint32_t) + VK_UUID_SIZE;
        auto size = headerSize;
        if (!pCreateInfo->pInitialData || pCreateInfo->initialDataSize < size) return;

        uint32_t const *data = (uint32_t const *)pCreateInfo->pInitialData;
        if (data[0] != size) return;
        if (data[1] != VK_VALIDATION_CACHE_HEADER_VERSION_ONE_EXT) return;
        uint8_t expected_uuid[VK_UUID_SIZE];
        Sha1ToVkUuid(SPIRV_TOOLS_COMMIT_ID, expected_uuid);
        if (memcmp(&data[2], expected_uuid, VK_UUID_SIZE) != 0) return;  // different version

        data = (uint32_t const *)(reinterpret_cast<uint8_t const *>(data) + headerSize);

        for (; size < pCreateInfo->initialDataSize; data++, size += sizeof(uint32_t)) {
            good_shader_hashes.insert(*data);
        }
    }

    void Write(size_t *pDataSize, void *pData) {
        const auto headerSize = 2 * sizeof(uint32_t) + VK_UUID_SIZE;  // 4 bytes for header size + 4 bytes for version number + UUID
        if (!pData) {
            *pDataSize = headerSize + good_shader_hashes.size() * sizeof(uint32_t);
            return;
        }

        if (*pDataSize < headerSize) {
            *pDataSize = 0;
            return;  // Too small for even the header!
        }

        uint32_t *out = (uint32_t *)pData;
        size_t actualSize = headerSize;

        // Write the header
        *out++ = headerSize;
        *out++ = VK_VALIDATION_CACHE_HEADER_VERSION_ONE_EXT;
        Sha1ToVkUuid(SPIRV_TOOLS_COMMIT_ID, reinterpret_cast<uint8_t *>(out));
        out = (uint32_t *)(reinterpret_cast<uint8_t *>(out) + VK_UUID_SIZE);

        for (auto it = good_shader_hashes.begin(); it != good_shader_hashes.end() && actualSize < *pDataSize;
             it++, out++, actualSize += sizeof(uint32_t)) {
            *out = *it;
        }

        *pDataSize = actualSize;
    }

    void Merge(ValidationCache const *other) {
        good_shader_hashes.reserve(good_shader_hashes.size() + other->good_shader_hashes.size());
        for (auto h : other->good_shader_hashes) good_shader_hashes.insert(h);
    }

    static uint32_t MakeShaderHash(VkShaderModuleCreateInfo const *smci);

    bool Contains(uint32_t hash) { return good_shader_hashes.count(hash) != 0; }

    void Insert(uint32_t hash) { good_shader_hashes.insert(hash); }

   private:
    void Sha1ToVkUuid(const char *sha1_str, uint8_t uuid[VK_UUID_SIZE]) {
        // Convert sha1_str from a hex string to binary. We only need VK_UUID_BYTES of
        // output, so pad with zeroes if the input string is shorter than that, and truncate
        // if it's longer.
        char padded_sha1_str[2 * VK_UUID_SIZE + 1] = {};
        strncpy(padded_sha1_str, sha1_str, 2 * VK_UUID_SIZE + 1);
        char byte_str[3] = {};
        for (uint32_t i = 0; i < VK_UUID_SIZE; ++i) {
            byte_str[0] = padded_sha1_str[2 * i + 0];
            byte_str[1] = padded_sha1_str[2 * i + 1];
            uuid[i] = static_cast<uint8_t>(strtol(byte_str, NULL, 16));
        }
    }
};

bool ValidateAndCapturePipelineShaderState(layer_data *dev_data, PIPELINE_STATE *pPipeline);
bool ValidateComputePipeline(layer_data *dev_data, PIPELINE_STATE *pPipeline);
bool ValidateRayTracingPipelineNV(layer_data *dev_data, PIPELINE_STATE *pipeline);
typedef std::pair<unsigned, unsigned> descriptor_slot_t;
bool PreCallValidateCreateShaderModule(layer_data *dev_data, VkShaderModuleCreateInfo const *pCreateInfo, bool *spirv_valid);

#endif  // VULKAN_SHADER_VALIDATION_H
