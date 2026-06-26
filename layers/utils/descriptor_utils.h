/* Copyright (c) 2015-2026 The Khronos Group Inc.
 * Copyright (c) 2015-2026 Valve Corporation
 * Copyright (c) 2015-2026 LunarG, Inc.
 * Copyright (C) 2015-2026 Google Inc.
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

#include <vulkan/vulkan_core.h>
#include <cstdint>
#include <string>
#include "generated/error_location_helper.h"

namespace spirv {
struct ResourceInterfaceVariable;
}
namespace vvl {
class DeviceState;
}

static inline const VkSamplerCreateInfo* GetEmbeddedSampler(const VkDescriptorSetAndBindingMappingEXT& mapping) {
    switch (mapping.source) {
        case VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT:
            return mapping.sourceData.constantOffset.pEmbeddedSampler;
        case VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_PUSH_INDEX_EXT:
            return mapping.sourceData.pushIndex.pEmbeddedSampler;
        case VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_EXT:
            return mapping.sourceData.indirectIndex.pEmbeddedSampler;
        case VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_ARRAY_EXT:
            return mapping.sourceData.indirectIndexArray.pEmbeddedSampler;
        case VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_SHADER_RECORD_INDEX_EXT:
            return mapping.sourceData.shaderRecordIndex.pEmbeddedSampler;
        default:
            return nullptr;
    }
}

uint32_t CountDescriptorHeapEmbeddedSamplers(const void* pNext);

static constexpr bool IsDescriptorHeapAddr(const VkDescriptorType type) {
    return (type == VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR) || (type == VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV) ||
           (type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) || (type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) ||
           (type == VK_DESCRIPTOR_TYPE_PARTITIONED_ACCELERATION_STRUCTURE_NV);
}

static constexpr bool IsDescriptorHeapTexelBuffer(const VkDescriptorType type) {
    return (type == VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER) || (type == VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER);
}

static constexpr bool IsDescriptorHeapImage(const VkDescriptorType type) {
    return (type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE) || (type == VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM) ||
           (type == VK_DESCRIPTOR_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM) || (type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE) ||
           (type == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT);
}

static constexpr bool IsDescriptorHeapTensor(const VkDescriptorType type) { return (type == VK_DESCRIPTOR_TYPE_TENSOR_ARM); }

const char* DescribeDescriptorBufferSize(bool robust, VkDescriptorType type);

bool IsResourceVaribleInMapping(const VkDescriptorSetAndBindingMappingEXT& mapping,
                                const spirv::ResourceInterfaceVariable& resource_variable);
bool ResourceTypeMatchesBinding(VkSpirvResourceTypeFlagsEXT resource_type,
                                const spirv::ResourceInterfaceVariable& resource_variable);
std::string DescribeResourceTypeMismatch(VkSpirvResourceTypeFlagsEXT resource_type,
                                         const spirv::ResourceInterfaceVariable& resource_variable);

VkDeviceSize GetDescriptorHeapAlignment(const VkPhysicalDeviceDescriptorHeapPropertiesEXT& props, VkDescriptorType type);
vvl::Field GetDescriptorHeapAlignmentField(VkDescriptorType type);

bool HasCombinedImageSamplerIndex(const VkDescriptorSetAndBindingMappingEXT& mapping);

// Way to cache vkGetPhysicalDeviceDescriptorSizeEXT as a flat array
// This is a very quick way to use the VkDescriptorType enum and knowledge of the limited VkDescriptorType allowed to make this fast
// and not making it impossible to update if a new VkDescriptorType is added later
struct DeviceExtensions;
struct CachedDescriptorSize {
    void Init(const vvl::DeviceState& device_state);
    VkDeviceSize GetSize(VkDescriptorType type, bool use_heap = true) const;

  private:
    // The number of valid descriptors that can be queried is known in the spec
    // (see VU 11362)
    VkDeviceSize heap_size_[14];    // VK_EXT_descriptor_heap
    VkDeviceSize buffer_size_[14];  // VK_EXT_descriptor_buffer
};

// We need a way to compress the VkDescriptorType enum to cover all the "real" types
// (those seen in vkGetPhysicalDeviceDescriptorSizeEXT).
//
// This all should be capable of being packed in 4 bits as there are only 14 known types currently
enum class vvlDescriptorType : uint8_t {
    // Sampler is a speical case, it is never by itself and instead is is provided along with an image
    Sampler = 0x0,          // VK_DESCRIPTOR_TYPE_SAMPLER
    CombinedSampler = 0x1,  // VK_DESCRIPTOR_TYPE_SAMPLER part of a COMBINED_IMAGE_SAMPLER

    // Buffers
    UniformBuffer = 0x2,  // VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
    StorageBuffer = 0x3,  // VK_DESCRIPTOR_TYPE_STORAGE_BUFFER

    // RT
    AccelerationStructure = 0x4,  // VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR

    // (currently 0x5, 0x6, 0x7 are not being used)

    // Images
    ImageSampled = 0x8,             // VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE
    ImageStorage = 0x9,             // VK_DESCRIPTOR_TYPE_STORAGE_IMAGE
    ImageTexelBufferUniform = 0xA,  // VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER
    ImageTexelBufferStorage = 0xB,  // VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER
    ImageInputAttachment = 0xC,     // VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT
};

inline constexpr uint8_t vvlDescriptorBufferMask = 0x2;
inline constexpr uint8_t vvlDescriptorImageMask = 0x8;

constexpr VkDescriptorType GetDescriptorTypeFromMask(vvlDescriptorType vvl_type) noexcept {
    switch (vvl_type) {
        case vvlDescriptorType::Sampler:
        case vvlDescriptorType::CombinedSampler:
            return VK_DESCRIPTOR_TYPE_SAMPLER;
        case vvlDescriptorType::UniformBuffer:
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        case vvlDescriptorType::StorageBuffer:
            return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        case vvlDescriptorType::AccelerationStructure:
            return VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
        case vvlDescriptorType::ImageSampled:
            return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        case vvlDescriptorType::ImageStorage:
            return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        case vvlDescriptorType::ImageTexelBufferUniform:
            return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
        case vvlDescriptorType::ImageTexelBufferStorage:
            return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
        case vvlDescriptorType::ImageInputAttachment:
            return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    }
    assert(false && "Invalid compressed descriptor type mask");
    return VK_DESCRIPTOR_TYPE_MAX_ENUM;
}

constexpr vvlDescriptorType GetMaskFromDescriptorType(VkDescriptorType vk_type) noexcept {
    switch (vk_type) {
        case VK_DESCRIPTOR_TYPE_SAMPLER:
            return vvlDescriptorType::Sampler;
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
            return vvlDescriptorType::UniformBuffer;
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
            return vvlDescriptorType::StorageBuffer;
        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
            return vvlDescriptorType::AccelerationStructure;
        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
            return vvlDescriptorType::ImageSampled;
        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
            return vvlDescriptorType::ImageStorage;
        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
            return vvlDescriptorType::ImageTexelBufferUniform;
        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
            return vvlDescriptorType::ImageTexelBufferStorage;
        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
            return vvlDescriptorType::ImageInputAttachment;
        default:
            break;
    }

    assert(false && "Unsupported VkDescriptorType for compression");
    return static_cast<vvlDescriptorType>(0xFF);
}