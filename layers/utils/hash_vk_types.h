/* Copyright (c) 2019, 2021, 2023-2025 The Khronos Group Inc.
 * Copyright (c) 2019, 2021, 2023-2025 Valve Corporation
 * Copyright (c) 2019, 2021, 2023-2025 LunarG, Inc.
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

// Includes everything needed for overloading std::hash
#include "hash_util.h"
#include "vk_struct_compare.h"

#include <vulkan/vulkan.h>
#include <vulkan/utility/vk_safe_struct.hpp>
#include <vulkan/utility/vk_struct_helper.hpp>
#include <vector>

//
// Hash and equality and/or compare functions for selected Vk types (and useful collections thereof)
//

// We do not provide a hasher directly for VkDescriptorSetLayoutBinding structure because
// immutable samplers in the context of pipeline layout compatibility can't be compared
// by comparing their VkSampler handles. The immutable samplers are compared based on their
// create parameters. DescriptorSetLayoutBindingHashingData allows to specify a hash that is
// computed based on sampler create parameters. This hash is a combined hash that combine
// hashes of all samplers from the same binding.
struct DescriptorSetLayoutBindingHashingData {
    const VkDescriptorSetLayoutBinding &binding;
    size_t immutable_samplers_combined_hash = 0;
};

namespace std {
template <>
struct hash<DescriptorSetLayoutBindingHashingData> {
    size_t operator()(const DescriptorSetLayoutBindingHashingData &value) const {
        hash_util::HashCombiner hc;
        const VkDescriptorSetLayoutBinding &binding = value.binding;
        hc << binding.binding << binding.descriptorType << binding.descriptorCount << binding.stageFlags;
        hc << value.immutable_samplers_combined_hash;
        return hc.Value();
    }
};
}  // namespace std

// VkSamplerCreateInfo and its pNexts
static inline void HashCombineSamplerYcbcrConversionInfo(hash_util::HashCombiner &hc, const VkSamplerYcbcrConversionInfo &value) {
    // TODO: clarify if conversion should be hashed based on parameters and not just by handle (similar to immutable samplers)
    hc << value.conversion;
}

static inline void HashCombineSamplerBorderColorComponentMappingCreateInfo(
    hash_util::HashCombiner &hc, const VkSamplerBorderColorComponentMappingCreateInfoEXT &value) {
    hc << value.components.r;
    hc << value.components.g;
    hc << value.components.b;
    hc << value.components.a;
    hc << value.srgb;
}

static inline void HashCombineSamplerCustomBorderColorCreateInfo(hash_util::HashCombiner &hc,
                                                                 const VkSamplerCustomBorderColorCreateInfoEXT &value) {
    hc << value.customBorderColor.uint32[0];
    hc << value.customBorderColor.uint32[1];
    hc << value.customBorderColor.uint32[2];
    hc << value.customBorderColor.uint32[3];
    hc << value.format;
}

static inline void HashCombineSamplerReductionModeCreateInfo(hash_util::HashCombiner &hc,
                                                             const VkSamplerReductionModeCreateInfo *p_value) {
    if (p_value) {
        hc << p_value->reductionMode;
    } else {
        // Default reduction mode
        hc << VK_SAMPLER_REDUCTION_MODE_WEIGHTED_AVERAGE;
    }
}

static inline void HashCombineSamplerCreateInfo(hash_util::HashCombiner &hc, const VkSamplerCreateInfo &value) {
    hc << value.flags;
    hc << value.magFilter;
    hc << value.minFilter;
    hc << value.mipmapMode;
    hc << value.addressModeU;
    hc << value.addressModeV;
    hc << value.addressModeW;
    hc << value.mipLodBias;
    hc << value.anisotropyEnable;
    hc << value.maxAnisotropy;
    hc << value.compareEnable;
    hc << value.compareOp;
    hc << value.minLod;
    hc << value.maxLod;
    hc << value.borderColor;
    hc << value.unnormalizedCoordinates;

    if (auto *ycbcr_conversion = vku::FindStructInPNextChain<VkSamplerYcbcrConversionInfo>(value.pNext)) {
        HashCombineSamplerYcbcrConversionInfo(hc, *ycbcr_conversion);
    }
    if (auto *component_mapping = vku::FindStructInPNextChain<VkSamplerBorderColorComponentMappingCreateInfoEXT>(value.pNext)) {
        HashCombineSamplerBorderColorComponentMappingCreateInfo(hc, *component_mapping);
    }
    if (auto *border_color = vku::FindStructInPNextChain<VkSamplerCustomBorderColorCreateInfoEXT>(value.pNext)) {
        HashCombineSamplerCustomBorderColorCreateInfo(hc, *border_color);
    }
    // NOTE: do not have condition for reduction mode because hashing function runs logic
    // for default reduction mode when pNext is null
    auto *reduction_mode = vku::FindStructInPNextChain<VkSamplerReductionModeCreateInfo>(value.pNext);
    HashCombineSamplerReductionModeCreateInfo(hc, reduction_mode);
}

static inline size_t HashSamplerCreateInfo(const VkSamplerCreateInfo &value) {
    hash_util::HashCombiner hc;
    HashCombineSamplerCreateInfo(hc, value);
    return hc.Value();
}

// VkPushConstantRange
static inline bool operator==(const VkPushConstantRange &lhs, const VkPushConstantRange &rhs) {
    return (lhs.stageFlags == rhs.stageFlags) && (lhs.offset == rhs.offset) && (lhs.size == rhs.size);
}

namespace std {
template <>
struct hash<VkPushConstantRange> {
    size_t operator()(const VkPushConstantRange &value) const {
        hash_util::HashCombiner hc;
        return (hc << value.stageFlags << value.offset << value.size).Value();
    }
};
}  // namespace std

using PushConstantRanges = std::vector<VkPushConstantRange>;

namespace std {
template <>
struct hash<PushConstantRanges> : public hash_util::IsOrderedContainer<PushConstantRanges> {};
}  // namespace std

// VkImageSubresourceRange
static inline bool operator==(const VkImageSubresourceRange &lhs, const VkImageSubresourceRange &rhs) {
    return (lhs.aspectMask == rhs.aspectMask) && (lhs.baseMipLevel == rhs.baseMipLevel) && (lhs.levelCount == rhs.levelCount) &&
           (lhs.baseArrayLayer == rhs.baseArrayLayer) && (lhs.layerCount == rhs.layerCount);
}
namespace std {
template <>
struct hash<VkImageSubresourceRange> {
    size_t operator()(const VkImageSubresourceRange &value) const {
        hash_util::HashCombiner hc;
        hc << value.aspectMask << value.baseMipLevel << value.levelCount << value.baseArrayLayer << value.layerCount;
        return hc.Value();
    }
};
}  // namespace std

static inline bool operator==(const VkShaderModuleIdentifierEXT &a, const VkShaderModuleIdentifierEXT &b) {
    if (a.identifierSize != b.identifierSize) {
        return false;
    }
    const uint32_t copy_size = std::min(VK_MAX_SHADER_MODULE_IDENTIFIER_SIZE_EXT, a.identifierSize);
    for (uint32_t i = 0u; i < copy_size; ++i) {
        if (a.identifier[i] != b.identifier[i]) {
            return false;
        }
    }
    return true;
}
namespace std {
template <>
struct hash<VkShaderModuleIdentifierEXT> {
    size_t operator()(const VkShaderModuleIdentifierEXT &value) const {
        return hash_util::HashCombiner().Combine(value.identifier, value.identifier + value.identifierSize).Value();
    }
};
}  // namespace std
