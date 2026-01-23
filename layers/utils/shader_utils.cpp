/* Copyright (c) 2015-2026 The Khronos Group Inc.
 * Copyright (c) 2015-2026 Valve Corporation
 * Copyright (c) 2015-2026 LunarG, Inc.
 * Copyright (C) 2015-2026 Google Inc.
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

#include "generated/spirv_grammar_helper.h"
#include "generated/spirv_tools_commit_id.h"
#include "state_tracker/shader_module.h"

#include <cstring>
#include <fstream>
#include <sstream>
#include <string>

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
    const auto header_size = 2 * sizeof(uint32_t) + VK_UUID_SIZE;  // 4 bytes for header size + 4 bytes for version number + UUID
    if (!pData) {
        *pDataSize = header_size + good_shader_hashes_.size() * sizeof(uint32_t);
        return;
    }

    if (*pDataSize < header_size) {
        *pDataSize = 0;
        return;  // Too small for even the header!
    }

    uint32_t *out = (uint32_t *)pData;
    size_t actual_size = header_size;

    // Write the header
    *out++ = header_size;
    *out++ = VK_VALIDATION_CACHE_HEADER_VERSION_ONE_EXT;
    GetUUID(reinterpret_cast<uint8_t *>(out));
    out = (uint32_t *)(reinterpret_cast<uint8_t *>(out) + VK_UUID_SIZE);

    {
        auto guard = ReadLock();
        for (auto it = good_shader_hashes_.begin(); it != good_shader_hashes_.end() && actual_size < *pDataSize;
             it++, out++, actual_size += sizeof(uint32_t)) {
            *out = *it;
        }
    }

    *pDataSize = actual_size;
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

// This is used to help dump SPIR-V while debugging intermediate phases of any altercations to the SPIR-V
void DumpSpirvToFile(const std::string &file_path, const uint32_t *spirv, size_t spirv_dwords_count) {
    std::ofstream debug_file(file_path, std::ios::out | std::ios::binary);
    debug_file.write(reinterpret_cast<const char *>(spirv), spirv_dwords_count * sizeof(uint32_t));
}

bool ResourceTypeMatchesBinding(VkSpirvResourceTypeFlagsEXT resource_type,
                                const spirv::ResourceInterfaceVariable& resource_variable) {
    if (resource_type == VK_SPIRV_RESOURCE_TYPE_ALL_EXT) {
        return true;
    }

    const uint32_t opcode = resource_variable.base_type.Opcode();
    if ((resource_type & VK_SPIRV_RESOURCE_TYPE_SAMPLER_BIT_EXT) != 0 && opcode == spv::OpTypeSampler) {
        return true;
    }
    if ((resource_type & VK_SPIRV_RESOURCE_TYPE_SAMPLED_IMAGE_BIT_EXT) != 0 && opcode == spv::OpTypeImage &&
        resource_variable.base_type.Word(7) == 1) {
        return true;
    }
    if ((resource_type & VK_SPIRV_RESOURCE_TYPE_READ_ONLY_IMAGE_BIT_EXT) != 0 && opcode == spv::OpTypeImage &&
        resource_variable.base_type.Word(7) == 2 && resource_variable.decorations.Has(spirv::DecorationSet::nonwritable_bit)) {
        return true;
    }
    if ((resource_type & VK_SPIRV_RESOURCE_TYPE_READ_WRITE_IMAGE_BIT_EXT) != 0 && opcode == spv::OpTypeImage &&
        resource_variable.base_type.Word(7) == 2 && !resource_variable.decorations.Has(spirv::DecorationSet::nonwritable_bit)) {
        return true;
    }
    if ((resource_type & VK_SPIRV_RESOURCE_TYPE_COMBINED_SAMPLED_IMAGE_BIT_EXT) != 0 && resource_variable.is_type_sampled_image) {
        return true;
    }
    if ((resource_type & VK_SPIRV_RESOURCE_TYPE_UNIFORM_BUFFER_BIT_EXT) != 0 && resource_variable.is_uniform_buffer) {
        return true;
    }
    if ((resource_type & VK_SPIRV_RESOURCE_TYPE_READ_ONLY_STORAGE_BUFFER_BIT_EXT) != 0 && resource_variable.is_storage_buffer &&
        resource_variable.decorations.Has(spirv::DecorationSet::nonwritable_bit)) {
        return true;
    }
    if ((resource_type & VK_SPIRV_RESOURCE_TYPE_READ_WRITE_STORAGE_BUFFER_BIT_EXT) != 0 && resource_variable.is_storage_buffer &&
        !resource_variable.decorations.Has(spirv::DecorationSet::nonwritable_bit)) {
        return true;
    }
    if ((resource_type & VK_SPIRV_RESOURCE_TYPE_ACCELERATION_STRUCTURE_BIT_EXT) != 0 &&
        opcode == spv::OpTypeAccelerationStructureKHR) {
        return true;
    }
    if ((resource_type & VK_SPIRV_RESOURCE_TYPE_TENSOR_BIT_ARM) != 0 && opcode == spv::OpTypeTensorARM) {
        return true;
    }

    return false;
}

// Only currently want to report the first mismatch
std::string DescribeResourceTypeMismatch(VkSpirvResourceTypeFlagsEXT resource_type,
                                         const spirv::ResourceInterfaceVariable& resource_variable) {
    const uint32_t opcode = resource_variable.base_type.Opcode();

    if ((resource_type & VK_SPIRV_RESOURCE_TYPE_SAMPLER_BIT_EXT) != 0 && opcode != spv::OpTypeSampler) {
        return "base type is " + std::string(string_SpvOpcode(opcode)) + ", not OpTypeSampler";
    }
    if ((resource_type & VK_SPIRV_RESOURCE_TYPE_SAMPLED_IMAGE_BIT_EXT) != 0) {
        if (opcode != spv::OpTypeImage) {
            return "base type is " + std::string(string_SpvOpcode(opcode)) + ", not OpTypeImage";
        } else if (resource_variable.base_type.Word(7) != 1) {
            return "OpTypeImage Sampled is " + std::to_string(resource_variable.base_type.Word(7)) + ", not 1";
        }
    }
    if ((resource_type & VK_SPIRV_RESOURCE_TYPE_READ_ONLY_IMAGE_BIT_EXT) != 0) {
        if (opcode != spv::OpTypeImage) {
            return "base type is " + std::string(string_SpvOpcode(opcode)) + ", not OpTypeImage";
        } else if (resource_variable.base_type.Word(7) != 2) {
            return "OpTypeImage Sampled is " + std::to_string(resource_variable.base_type.Word(7)) + ", not 2";
        } else if (!resource_variable.decorations.Has(spirv::DecorationSet::nonwritable_bit)) {
            return "is not decorated with NonWritable";
        }
    }
    if ((resource_type & VK_SPIRV_RESOURCE_TYPE_READ_WRITE_IMAGE_BIT_EXT) != 0) {
        if (opcode != spv::OpTypeImage) {
            return "base type is " + std::string(string_SpvOpcode(opcode)) + ", not OpTypeImage";
        } else if (resource_variable.base_type.Word(7) != 2) {
            return "OpTypeImage Sampled is " + std::to_string(resource_variable.base_type.Word(7)) + ", not 2";
        } else if (resource_variable.decorations.Has(spirv::DecorationSet::nonwritable_bit)) {
            return "is decorated with NonWritable";
        }
    }
    if ((resource_type & VK_SPIRV_RESOURCE_TYPE_COMBINED_SAMPLED_IMAGE_BIT_EXT) != 0 && !resource_variable.is_type_sampled_image) {
        return "is not OpTypeSampledImage";
    }
    if ((resource_type & VK_SPIRV_RESOURCE_TYPE_UNIFORM_BUFFER_BIT_EXT) != 0 && !resource_variable.is_uniform_buffer) {
        if (resource_variable.storage_class != spv::StorageClassUniform) {
            return "is not Uniform StorageClass";
        } else if (!resource_variable.type_struct_info) {
            return "is not a OpTypeStruct";
        } else if (!resource_variable.type_struct_info->decorations.Has(spirv::DecorationSet::block_bit)) {
            return "is not decorated with Block";
        }
    }
    if ((resource_type & VK_SPIRV_RESOURCE_TYPE_READ_ONLY_STORAGE_BUFFER_BIT_EXT) != 0) {
        if (!resource_variable.decorations.Has(spirv::DecorationSet::nonwritable_bit)) {
            return "is not decorated with NonWritable";
        } else if (!resource_variable.type_struct_info) {
            return "is not a OpTypeStruct";
        } else if (!resource_variable.is_storage_buffer) {
            return "is not a storage buffer";  // simplified
        }
    }
    if ((resource_type & VK_SPIRV_RESOURCE_TYPE_READ_WRITE_STORAGE_BUFFER_BIT_EXT) != 0) {
        if (!resource_variable.decorations.Has(spirv::DecorationSet::nonwritable_bit)) {
            return "is decorated with NonWritable";
        } else if (!resource_variable.type_struct_info) {
            return "is not a OpTypeStruct";
        } else if (!resource_variable.is_storage_buffer) {
            return "is not a storage buffer";  // simplified
        }
    }
    if ((resource_type & VK_SPIRV_RESOURCE_TYPE_ACCELERATION_STRUCTURE_BIT_EXT) != 0 &&
        opcode == spv::OpTypeAccelerationStructureKHR) {
        return "base type is " + std::string(string_SpvOpcode(opcode)) + ", not OpTypeAccelerationStructureKHR";
    }
    if ((resource_type & VK_SPIRV_RESOURCE_TYPE_TENSOR_BIT_ARM) != 0 && opcode == spv::OpTypeTensorARM) {
        return "base type is " + std::string(string_SpvOpcode(opcode)) + ", not OpTypeTensorARM";
    }

    return "[UNKNOWN]";
}