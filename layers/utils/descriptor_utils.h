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
#include <string>

namespace spirv {
struct ResourceInterfaceVariable;
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

size_t GetDescriptorBufferSize(const VkPhysicalDeviceDescriptorBufferPropertiesEXT& props, bool robust, VkDescriptorType type);
const char* DescribeDescriptorBufferSize(bool robust, VkDescriptorType type);

bool IsResourceVaribleInMapping(const VkDescriptorSetAndBindingMappingEXT& mapping,
                                const spirv::ResourceInterfaceVariable& resource_variable);
bool ResourceTypeMatchesBinding(VkSpirvResourceTypeFlagsEXT resource_type,
                                const spirv::ResourceInterfaceVariable& resource_variable);
std::string DescribeResourceTypeMismatch(VkSpirvResourceTypeFlagsEXT resource_type,
                                         const spirv::ResourceInterfaceVariable& resource_variable);

VkDeviceSize GetUntypedDescriptorSize(const VkPhysicalDeviceDescriptorHeapPropertiesEXT& props, VkDescriptorType type);
VkDeviceSize GetDescriptorHeapAlignment(const VkPhysicalDeviceDescriptorHeapPropertiesEXT& props, VkDescriptorType type);

// Way to cache vkGetPhysicalDeviceDescriptorSizeEXT as a flat array
// This is a very quick way to use the VkDescriptorType enum and knowledge of the limited VkDescriptorType allowed to make this fast
// and not making it impossible to update if a new VkDescriptorType is added later
struct DeviceExtensions;
struct CachedDescriptorSize {
    void Init(VkPhysicalDevice gpu, const DeviceExtensions& extensions);
    VkDeviceSize GetSize(VkDescriptorType type) const;

  private:
    // The number of valid descriptors that can be queried is known in the spec
    // (see VU 11362)
    VkDeviceSize size_[14];
};