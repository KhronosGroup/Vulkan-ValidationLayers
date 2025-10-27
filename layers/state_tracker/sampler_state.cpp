/* Copyright (c) 2025 The Khronos Group Inc.
 * Copyright (c) 2025 LunarG, Inc.
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

#include "sampler_state.h"
#include <sstream>
#include "utils/image_utils.h"

namespace vvl {

Sampler::Sampler(const VkSampler handle, const VkSamplerCreateInfo *pCreateInfo)
    : StateObject(handle, kVulkanObjectTypeSampler),
      safe_create_info(pCreateInfo),
      create_info(*safe_create_info.ptr()),
      sampler_conversion(GetConversion(pCreateInfo)),
      customCreateInfo(GetCustomCreateInfo(pCreateInfo)) {}

SamplerYcbcrConversion::SamplerYcbcrConversion(VkSamplerYcbcrConversion handle,
                                               const VkSamplerYcbcrConversionCreateInfo *pCreateInfo,
                                               VkFormatFeatureFlags2 features)
    : StateObject(handle, kVulkanObjectTypeSamplerYcbcrConversion),
      safe_create_info(pCreateInfo),
      create_info(*safe_create_info.ptr()),
      format_features(features),
      external_format(GetExternalFormat(pCreateInfo->pNext)) {}

bool SamplerYcbcrConversion::operator!=(const SamplerYcbcrConversion &rhs) const {
    return (create_info.format != rhs.create_info.format) || (create_info.ycbcrModel != rhs.create_info.ycbcrModel) ||
           (create_info.ycbcrRange != rhs.create_info.ycbcrRange) || (create_info.components.r != rhs.create_info.components.r) ||
           (create_info.components.g != rhs.create_info.components.g) ||
           (create_info.components.b != rhs.create_info.components.b) ||
           (create_info.components.a != rhs.create_info.components.a) ||
           (create_info.xChromaOffset != rhs.create_info.xChromaOffset) ||
           (create_info.yChromaOffset != rhs.create_info.yChromaOffset) ||
           (create_info.chromaFilter != rhs.create_info.chromaFilter) ||
           (create_info.forceExplicitReconstruction != rhs.create_info.forceExplicitReconstruction) ||
           (external_format != rhs.external_format);
}

std::string SamplerYcbcrConversion::Describe() const {
    std::stringstream ss;
    ss << " format (" << string_VkFormat(create_info.format) << ")\n";
    ss << " ycbcrModel (" << string_VkSamplerYcbcrModelConversion(create_info.ycbcrModel) << ")\n";
    ss << " ycbcrRange (" << string_VkSamplerYcbcrRange(create_info.ycbcrRange) << ")\n";
    ss << " components (" << string_VkComponentSwizzle(create_info.components.r) << ", "
       << string_VkComponentSwizzle(create_info.components.g) << ", " << string_VkComponentSwizzle(create_info.components.b) << ", "
       << string_VkComponentSwizzle(create_info.components.a) << ")\n";
    ss << " xChromaOffset (" << string_VkChromaLocation(create_info.xChromaOffset) << ")\n";
    ss << " yChromaOffset (" << string_VkChromaLocation(create_info.yChromaOffset) << ")\n";
    ss << " chromaFilter (" << string_VkFilter(create_info.chromaFilter) << ")\n";
    ss << " forceExplicitReconstruction (" << (create_info.forceExplicitReconstruction ? "VK_TRUE" : "VK_FALSE") << ")\n";
    ss << " externalFormat (" << external_format << ")\n";
    return ss.str();
}

}  // namespace vvl