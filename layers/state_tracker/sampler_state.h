/* Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
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
#pragma once
#include "state_tracker/base_node.h"

// Note: some of the types in this header are needed by both the DescriptorSet and Pipeline
// state objects. It is helpful to have a separate header to avoid circular #include madness.
struct DescriptorSlot {
    uint32_t set;
    uint32_t binding;

    DescriptorSlot(uint32_t s, uint32_t b) : set(s), binding(b) {}
};

inline bool operator==(const DescriptorSlot &lhs, const DescriptorSlot &rhs) noexcept {
    return lhs.set == rhs.set && lhs.binding == rhs.binding;
}

struct SamplerUsedByImage {
    DescriptorSlot sampler_slot;
    uint32_t sampler_index;
};

inline bool operator==(const SamplerUsedByImage &a, const SamplerUsedByImage &b) noexcept {
    return a.sampler_slot == b.sampler_slot && a.sampler_index == b.sampler_index;
}

namespace std {
template <>
struct hash<DescriptorSlot> {
    size_t operator()(DescriptorSlot slot) const noexcept { return hash<uint32_t>()(slot.set) ^ hash<uint32_t>()(slot.binding); }
};
template <>
struct hash<SamplerUsedByImage> {
    size_t operator()(SamplerUsedByImage s) const noexcept {
        return hash<DescriptorSlot>()(s.sampler_slot) ^ hash<uint32_t>()(s.sampler_index);
    }
};
}  // namespace std

class SAMPLER_STATE : public BASE_NODE {
  public:
    const VkSamplerCreateInfo createInfo;
    const VkSamplerYcbcrConversion samplerConversion;
    const VkSamplerCustomBorderColorCreateInfoEXT customCreateInfo;

    SAMPLER_STATE(const VkSampler *ps, const VkSamplerCreateInfo *pci)
        : BASE_NODE(*ps, kVulkanObjectTypeSampler),
          createInfo(*pci),
          samplerConversion(GetConversion(pci)),
          customCreateInfo(GetCustomCreateInfo(pci)) {}

    VkSampler sampler() const { return handle_.Cast<VkSampler>(); }

  private:
    static inline VkSamplerYcbcrConversion GetConversion(const VkSamplerCreateInfo *pci) {
        auto *conversionInfo = LvlFindInChain<VkSamplerYcbcrConversionInfo>(pci->pNext);
        return conversionInfo ? conversionInfo->conversion : VK_NULL_HANDLE;
    }
    static inline VkSamplerCustomBorderColorCreateInfoEXT GetCustomCreateInfo(const VkSamplerCreateInfo *pci) {
        VkSamplerCustomBorderColorCreateInfoEXT result{};
        auto cbci = LvlFindInChain<VkSamplerCustomBorderColorCreateInfoEXT>(pci->pNext);
        if (cbci) result = *cbci;
        return result;
    }
};

class SAMPLER_YCBCR_CONVERSION_STATE : public BASE_NODE {
  public:
    const VkFormatFeatureFlags2KHR format_features;
    const VkFormat format;
    const VkFilter chromaFilter;
    const uint64_t external_format;

    SAMPLER_YCBCR_CONVERSION_STATE(VkSamplerYcbcrConversion ycbcr, const VkSamplerYcbcrConversionCreateInfo *info,
                                   VkFormatFeatureFlags2KHR features)
        : BASE_NODE(ycbcr, kVulkanObjectTypeSamplerYcbcrConversion),
          format_features(features),
          format(info->format),
          chromaFilter(info->chromaFilter),
          external_format(GetExternalFormat(info)) {}

    VkSamplerYcbcrConversion ycbcr_conversion() const { return handle_.Cast<VkSamplerYcbcrConversion>(); }

  private:
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    uint64_t GetExternalFormat(const VkSamplerYcbcrConversionCreateInfo *info) {
        const VkExternalFormatANDROID *ext_format_android = LvlFindInChain<VkExternalFormatANDROID>(info->pNext);
        return ext_format_android ? ext_format_android->externalFormat : 0;
    }
#else
    uint64_t GetExternalFormat(const VkSamplerYcbcrConversionCreateInfo *info) { return 0; }
#endif  // VK_USE_PLATFORM_ANDROID_KHR
};
