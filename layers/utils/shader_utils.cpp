/* Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
 * Copyright (C) 2015-2023 Google Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
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

#include "shader_utils.h"

#include "state_tracker/device_state.h"
#include "generated/state_tracker_helper.h"
#include "generated/vk_extension_helper.h"
#include "state_tracker/shader_module.h"
#include "state_tracker/shader_instruction.h"
#include "utils/hash_util.h"

#include "generated/spirv_tools_commit_id.h"

void ValidationCache::GetUUID(uint8_t *uuid) {
    const char *sha1_str = SPIRV_TOOLS_COMMIT_ID;
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

    // Replace the last 4 bytes (likely padded with zero anyway)
    std::memcpy(uuid + (VK_UUID_SIZE - sizeof(uint32_t)), &spirv_val_option_hash_, sizeof(uint32_t));
}

void ValidationCache::Load(VkValidationCacheCreateInfoEXT const *pCreateInfo) {
    const auto headerSize = 2 * sizeof(uint32_t) + VK_UUID_SIZE;
    auto size = headerSize;
    if (!pCreateInfo->pInitialData || pCreateInfo->initialDataSize < size) return;

    uint32_t const *data = (uint32_t const *)pCreateInfo->pInitialData;
    if (data[0] != size) return;
    if (data[1] != VK_VALIDATION_CACHE_HEADER_VERSION_ONE_EXT) return;
    uint8_t expected_uuid[VK_UUID_SIZE];
    GetUUID(expected_uuid);
    if (memcmp(&data[2], expected_uuid, VK_UUID_SIZE) != 0) return;  // different version

    data = (uint32_t const *)(reinterpret_cast<uint8_t const *>(data) + headerSize);

    auto guard = WriteLock();
    for (; size < pCreateInfo->initialDataSize; data++, size += sizeof(uint32_t)) {
        good_shader_hashes_.insert(*data);
    }
}

void ValidationCache::Write(size_t *pDataSize, void *pData) {
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
    GetUUID(reinterpret_cast<uint8_t *>(out));
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

void ValidationCache::Merge(ValidationCache const *other) {
    // self-merging is invalid, but avoid deadlock below just in case.
    if (other == this) {
        return;
    }
    auto other_guard = other->ReadLock();
    auto guard = WriteLock();
    good_shader_hashes_.reserve(good_shader_hashes_.size() + other->good_shader_hashes_.size());
    for (auto h : other->good_shader_hashes_) good_shader_hashes_.insert(h);
}

spv_target_env PickSpirvEnv(const APIVersion &api_version, bool spirv_1_4) {
    if (api_version >= VK_API_VERSION_1_3) {
        return SPV_ENV_VULKAN_1_3;
    } else if (api_version >= VK_API_VERSION_1_2) {
        return SPV_ENV_VULKAN_1_2;
    } else if (api_version >= VK_API_VERSION_1_1) {
        if (spirv_1_4) {
            return SPV_ENV_VULKAN_1_1_SPIRV_1_4;
        } else {
            return SPV_ENV_VULKAN_1_1;
        }
    }
    return SPV_ENV_VULKAN_1_0;
}

// Some Vulkan extensions/features are just all done in spirv-val behind optional settings
void AdjustValidatorOptions(const DeviceExtensions &device_extensions, const DeviceFeatures &enabled_features,
                            spvtools::ValidatorOptions &out_options, uint32_t *out_hash) {
    struct Settings {
        bool relax_block_layout;
        bool uniform_buffer_standard_layout;
        bool scalar_block_layout;
        bool workgroup_scalar_block_layout;
        bool allow_local_size_id;
    } settings;

    // VK_KHR_relaxed_block_layout never had a feature bit so just enabling the extension allows relaxed layout
    // Was promotoed in Vulkan 1.1 so anyone using Vulkan 1.1 also gets this for free
    settings.relax_block_layout = IsExtEnabled(device_extensions.vk_khr_relaxed_block_layout);
    // The rest of the settings are controlled from a feature bit, which are set correctly in the state tracking. Regardless of
    // Vulkan version used, the feature bit is needed (also described in the spec).
    settings.uniform_buffer_standard_layout = enabled_features.uniformBufferStandardLayout == VK_TRUE;
    settings.scalar_block_layout = enabled_features.scalarBlockLayout == VK_TRUE;
    settings.workgroup_scalar_block_layout = enabled_features.workgroupMemoryExplicitLayoutScalarBlockLayout == VK_TRUE;
    settings.allow_local_size_id = enabled_features.maintenance4 == VK_TRUE;

    if (settings.relax_block_layout) {
        // --relax-block-layout
        out_options.SetRelaxBlockLayout(true);
    }
    if (settings.uniform_buffer_standard_layout) {
        // --uniform-buffer-standard-layout
        out_options.SetUniformBufferStandardLayout(true);
    }
    if (settings.scalar_block_layout) {
        // --scalar-block-layout
        out_options.SetScalarBlockLayout(true);
    }
    if (settings.workgroup_scalar_block_layout) {
        // --workgroup-scalar-block-layout
        out_options.SetWorkgroupScalarBlockLayout(true);
    }
    if (settings.allow_local_size_id) {
        // --allow-localsizeid
        out_options.SetAllowLocalSizeId(true);
    }

    // Faster validation without friendly names.
    out_options.SetFriendlyNames(false);

    // The spv_validator_options_t in libspirv.h is hidden so we can't just hash that struct, so instead need to create our own.
    if (out_hash) {
        *out_hash = hash_util::ShaderHash(&settings, sizeof(Settings));
    }
}

void GetActiveSlots(ActiveSlotMap &active_slots, const std::shared_ptr<const spirv::EntryPoint> &entrypoint) {
    if (!entrypoint) {
        return;
    }
    // Capture descriptor uses for the pipeline
    for (const auto &variable : entrypoint->resource_interface_variables) {
        // While validating shaders capture which slots are used by the pipeline
        DescriptorRequirement entry;
        entry.variable = &variable;
        entry.revalidate_hash = variable.descriptor_hash;
        active_slots[variable.decorations.set].emplace(variable.decorations.binding, entry);
    }
}

// static
ActiveSlotMap GetActiveSlots(const StageStateVec &stage_states) {
    ActiveSlotMap active_slots;
    for (const auto &stage : stage_states) {
        GetActiveSlots(active_slots, stage.entrypoint);
    }
    return active_slots;
}

ActiveSlotMap GetActiveSlots(const std::shared_ptr<const spirv::EntryPoint> &entrypoint) {
    ActiveSlotMap active_slots;
    GetActiveSlots(active_slots, entrypoint);
    return active_slots;
}

uint32_t GetMaxActiveSlot(const ActiveSlotMap &active_slots) {
    uint32_t max_active_slot = 0;
    for (const auto &entry : active_slots) {
        max_active_slot = std::max(max_active_slot, entry.first);
    }
    return max_active_slot;
}

const char *PipelineStageState::GetPName() const {
    return (pipeline_create_info) ? pipeline_create_info->pName : shader_object_create_info->pName;
}

VkShaderStageFlagBits PipelineStageState::GetStage() const {
    return (pipeline_create_info) ? pipeline_create_info->stage : shader_object_create_info->stage;
}

vku::safe_VkSpecializationInfo *PipelineStageState::GetSpecializationInfo() const {
    return (pipeline_create_info) ? pipeline_create_info->pSpecializationInfo : shader_object_create_info->pSpecializationInfo;
}

const void *PipelineStageState::GetPNext() const {
    return (pipeline_create_info) ? pipeline_create_info->pNext : shader_object_create_info->pNext;
}

bool PipelineStageState::GetInt32ConstantValue(const spirv::Instruction &insn, uint32_t *value) const {
    const spirv::Instruction *type_id = spirv_state->FindDef(insn.Word(1));
    if (type_id->Opcode() != spv::OpTypeInt || type_id->Word(2) != 32) {
        return false;
    }

    if (insn.Opcode() == spv::OpConstant) {
        *value = insn.Word(3);
        return true;
    } else if (insn.Opcode() == spv::OpSpecConstant) {
        *value = insn.Word(3);  // default value
        const auto *spec_info = GetSpecializationInfo();
        const uint32_t spec_id = spirv_state->static_data_.id_to_spec_id.at(insn.Word(2));
        if (spec_info && spec_id < spec_info->mapEntryCount) {
            memcpy(value, (uint8_t *)spec_info->pData + spec_info->pMapEntries[spec_id].offset,
                   spec_info->pMapEntries[spec_id].size);
        }
        return true;
    }

    // This means the value is not known until runtime and will need to be checked in GPU-AV
    return false;
}

PipelineStageState::PipelineStageState(const vku::safe_VkPipelineShaderStageCreateInfo *pipeline_create_info,
                                       const vku::safe_VkShaderCreateInfoEXT *shader_object_create_info,
                                       std::shared_ptr<const vvl::ShaderModule> module_state,
                                       std::shared_ptr<const spirv::Module> spirv_state)
    : module_state(module_state),
      spirv_state(spirv_state),
      pipeline_create_info(pipeline_create_info),
      shader_object_create_info(shader_object_create_info),
      entrypoint(spirv_state ? spirv_state->FindEntrypoint(GetPName(), GetStage()) : nullptr) {}
