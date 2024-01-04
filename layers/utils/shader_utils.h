/* Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
 * Copyright (C) 2015-2023 Google Inc.
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
 * The Shader Validation file is in charge of taking the Shader Module data and validating it
 */

#pragma once

#include "vulkan/vulkan.h"
#include "utils/vk_layer_utils.h"
#include "generated/spirv_tools_commit_id.h"

#include <spirv/unified1/spirv.hpp>
#include <spirv-tools/libspirv.h>
#include <spirv-tools/optimizer.hpp>

struct DeviceFeatures;
struct DeviceExtensions;
class APIVersion;

namespace spirv {
struct ResourceInterfaceVariable;
}  // namespace spirv

struct DescriptorRequirement {
    uint64_t revalidate_hash;
    const spirv::ResourceInterfaceVariable *variable;
    DescriptorRequirement() : revalidate_hash(0), variable(nullptr) {}
};

enum class ShaderObjectStage : uint32_t {
    VERTEX = 0u,
    TESSELLATION_CONTROL,
    TESSELLATION_EVALUATION,
    GEOMETRY,
    FRAGMENT,
    COMPUTE,
    TASK,
    MESH,

    LAST = 8u,
};

constexpr uint32_t kShaderObjectStageCount = 8u;

inline ShaderObjectStage VkShaderStageToShaderObjectStage(VkShaderStageFlagBits stage) {
    switch (stage) {
        case VK_SHADER_STAGE_VERTEX_BIT:
            return ShaderObjectStage::VERTEX;
        case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
            return ShaderObjectStage::TESSELLATION_CONTROL;
        case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
            return ShaderObjectStage::TESSELLATION_EVALUATION;
        case VK_SHADER_STAGE_GEOMETRY_BIT:
            return ShaderObjectStage::GEOMETRY;
        case VK_SHADER_STAGE_FRAGMENT_BIT:
            return ShaderObjectStage::FRAGMENT;
        case VK_SHADER_STAGE_COMPUTE_BIT:
            return ShaderObjectStage::COMPUTE;
        case VK_SHADER_STAGE_TASK_BIT_EXT:
            return ShaderObjectStage::TASK;
        case VK_SHADER_STAGE_MESH_BIT_EXT:
            return ShaderObjectStage::MESH;
        default:
            break;
    }
    return ShaderObjectStage::LAST;
}

inline bool operator==(const DescriptorRequirement &a, const DescriptorRequirement &b) noexcept {
    return a.revalidate_hash == b.revalidate_hash;
}

inline bool operator<(const DescriptorRequirement &a, const DescriptorRequirement &b) noexcept {
    return a.revalidate_hash < b.revalidate_hash;
}

// < binding index (of descriptor set) : meta data >
typedef std::unordered_multimap<uint32_t, DescriptorRequirement> BindingVariableMap;

// Capture which slots (set#->bindings) are actually used by the shaders of this pipeline
using ActiveSlotMap = vvl::unordered_map<uint32_t, BindingVariableMap>;

struct safe_VkPipelineShaderStageCreateInfo;
struct safe_VkShaderCreateInfoEXT;
struct safe_VkSpecializationInfo;

namespace vvl {
struct ShaderModule;
}  // namespace vvl

namespace spirv {
struct Module;
struct EntryPoint;
class Instruction;
}  // namespace spirv

struct PipelineStageState {
    // We use this over a spirv::Module because there are times we need to create empty objects
    std::shared_ptr<const vvl::ShaderModule> module_state;
    std::shared_ptr<const spirv::Module> spirv_state;
    const safe_VkPipelineShaderStageCreateInfo *pipeline_create_info;
    const safe_VkShaderCreateInfoEXT *shader_object_create_info;
    // If null, means it is an empty object, no SPIR-V backing it
    std::shared_ptr<const spirv::EntryPoint> entrypoint;

    PipelineStageState(const safe_VkPipelineShaderStageCreateInfo *pipeline_create_info,
                       const safe_VkShaderCreateInfoEXT *shader_object_create_info,
                       std::shared_ptr<const vvl::ShaderModule> module_state, std::shared_ptr<const spirv::Module> spirv_state);

    const char *GetPName() const;
    VkShaderStageFlagBits GetStage() const;
    safe_VkSpecializationInfo *GetSpecializationInfo() const;
    const void *GetPNext() const;
    bool GetInt32ConstantValue(const spirv::Instruction &insn, uint32_t *value) const;
};

using StageStateVec = std::vector<PipelineStageState>;

class ValidationCache {
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

        auto guard = WriteLock();
        for (; size < pCreateInfo->initialDataSize; data++, size += sizeof(uint32_t)) {
            good_shader_hashes_.insert(*data);
        }
    }

    void Write(size_t *pDataSize, void *pData) {
        const auto headerSize = 2 * sizeof(uint32_t) + VK_UUID_SIZE;  // 4 bytes for header size + 4 bytes for version number + UUID
        if (!pData) {
            *pDataSize = headerSize + good_shader_hashes_.size() * sizeof(uint32_t);
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

        {
            auto guard = ReadLock();
            for (auto it = good_shader_hashes_.begin(); it != good_shader_hashes_.end() && actualSize < *pDataSize;
                 it++, out++, actualSize += sizeof(uint32_t)) {
                *out = *it;
            }
        }

        *pDataSize = actualSize;
    }

    void Merge(ValidationCache const *other) {
        // self-merging is invalid, but avoid deadlock below just in case.
        if (other == this) {
            return;
        }
        auto other_guard = other->ReadLock();
        auto guard = WriteLock();
        good_shader_hashes_.reserve(good_shader_hashes_.size() + other->good_shader_hashes_.size());
        for (auto h : other->good_shader_hashes_) good_shader_hashes_.insert(h);
    }

    bool Contains(uint32_t hash) {
        auto guard = ReadLock();
        return good_shader_hashes_.count(hash) != 0;
    }

    void Insert(uint32_t hash) {
        auto guard = WriteLock();
        good_shader_hashes_.insert(hash);
    }

  private:
    ValidationCache() {}
    ReadLockGuard ReadLock() const { return ReadLockGuard(lock_); }
    WriteLockGuard WriteLock() { return WriteLockGuard(lock_); }

    void Sha1ToVkUuid(const char *sha1_str, uint8_t *uuid) {
        // Convert sha1_str from a hex string to binary. We only need VK_UUID_SIZE bytes of
        // output, so pad with zeroes if the input string is shorter than that, and truncate
        // if it's longer.
#if defined(__GNUC__) && (__GNUC__ > 8)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-truncation"
#endif
        char padded_sha1_str[2 * VK_UUID_SIZE + 1] = {};  // 2 hex digits == 1 byte
        std::strncpy(padded_sha1_str, sha1_str, 2 * VK_UUID_SIZE);
#if defined(__GNUC__) && (__GNUC__ > 8)
#pragma GCC diagnostic pop
#endif
        for (uint32_t i = 0; i < VK_UUID_SIZE; ++i) {
            const char byte_str[] = {padded_sha1_str[2 * i + 0], padded_sha1_str[2 * i + 1], '\0'};
            uuid[i] = static_cast<uint8_t>(std::strtoul(byte_str, nullptr, 16));
        }
    }

    // hashes of shaders that have passed validation before, and can be skipped.
    // we don't store negative results, as we would have to also store what was
    // wrong with them; also, we expect they will get fixed, so we're less
    // likely to see them again.
    vvl::unordered_set<uint32_t> good_shader_hashes_;
    mutable std::shared_mutex lock_;
};

spv_target_env PickSpirvEnv(const APIVersion &api_version, bool spirv_1_4);

void AdjustValidatorOptions(const DeviceExtensions &device_extensions, const DeviceFeatures &enabled_features,
                            spvtools::ValidatorOptions &options);

void GetActiveSlots(ActiveSlotMap &active_slots, const std::shared_ptr<const spirv::EntryPoint> &entrypoint);
ActiveSlotMap GetActiveSlots(const StageStateVec &stage_states);
ActiveSlotMap GetActiveSlots(const std::shared_ptr<const spirv::EntryPoint> &entrypoint);

uint32_t GetMaxActiveSlot(const ActiveSlotMap &active_slots);
