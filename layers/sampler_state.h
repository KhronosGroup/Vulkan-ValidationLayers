/* Copyright (c) 2015-2021 The Khronos Group Inc.
 * Copyright (c) 2015-2021 Valve Corporation
 * Copyright (c) 2015-2021 LunarG, Inc.
 * Copyright (C) 2015-2021 Google Inc.
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
 *
 * Author: Courtney Goeltzenleuchter <courtneygo@google.com>
 * Author: Tobin Ehlis <tobine@google.com>
 * Author: Chris Forbes <chrisf@ijw.co.nz>
 * Author: Mark Lobodzinski <mark@lunarg.com>
 * Author: Dave Houlton <daveh@lunarg.com>
 * Author: John Zulauf <jzulauf@lunarg.com>
 * Author: Tobias Hector <tobias.hector@amd.com>
 */
#pragma once
#include "base_node.h"

// Note: some of the types in this header are needed by both the DescriptorSet and Pipeline
// state objects. It is helpful to have a separate header to avoid circular #include madness.

typedef std::pair<unsigned, unsigned> descriptor_slot_t;

struct SamplerUsedByImage {
    descriptor_slot_t sampler_slot;
    uint32_t sampler_index;
};

inline bool operator==(const SamplerUsedByImage &a, const SamplerUsedByImage &b) NOEXCEPT {
    return a.sampler_slot == b.sampler_slot && a.sampler_index == b.sampler_index;
}

namespace std {
template <>
struct less<SamplerUsedByImage> {
    bool operator()(const SamplerUsedByImage &left, const SamplerUsedByImage &right) const { return false; }
};
}  // namespace std

class SAMPLER_STATE : public BASE_NODE {
  public:
    VkSamplerCreateInfo createInfo;
    VkSamplerYcbcrConversion samplerConversion = VK_NULL_HANDLE;
    VkSamplerCustomBorderColorCreateInfoEXT customCreateInfo = {};

    SAMPLER_STATE(const VkSampler *ps, const VkSamplerCreateInfo *pci)
        : BASE_NODE(*ps, kVulkanObjectTypeSampler), createInfo(*pci) {
        auto *conversionInfo = LvlFindInChain<VkSamplerYcbcrConversionInfo>(pci->pNext);
        if (conversionInfo) samplerConversion = conversionInfo->conversion;
        auto cbci = LvlFindInChain<VkSamplerCustomBorderColorCreateInfoEXT>(pci->pNext);
        if (cbci) customCreateInfo = *cbci;
    }

    VkSampler sampler() const { return handle_.Cast<VkSampler>(); }
};

class SAMPLER_YCBCR_CONVERSION_STATE : public BASE_NODE {
  public:
    VkFormatFeatureFlags format_features;
    VkFormat format;
    VkFilter chromaFilter;

    SAMPLER_YCBCR_CONVERSION_STATE(VkSamplerYcbcrConversion ycbcr, const VkSamplerYcbcrConversionCreateInfo *info,
                                   VkFormatFeatureFlags features)
        : BASE_NODE(ycbcr, kVulkanObjectTypeSamplerYcbcrConversion),
          format_features(features),
          format(info->format),
          chromaFilter(info->chromaFilter) {}

    VkSamplerYcbcrConversion ycbcr_conversion() const { return handle_.Cast<VkSamplerYcbcrConversion>(); }
};
