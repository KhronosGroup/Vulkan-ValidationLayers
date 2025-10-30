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

#pragma once
#include <vector>
#include <memory>

namespace spirv {
struct ResourceInterfaceVariable;
}  // namespace spirv

namespace vvl {
class DescriptorSetLayout;

// In Vulkan 1.0 we stored our list of vvl::DescriptorSetLayout in the PipelineLayout object.
// Then ShaderObjects came out and doesn't need the PipelineLayout when creating the SPIR-V (just the list of DSL).
//
// This serves as a common way to hold the vector<vvl::DescriptorSetLayout> we need when handling SPIR-V parsing
struct DescriptorSetLayoutList {
  DescriptorSetLayoutList() {}
  explicit DescriptorSetLayoutList(size_t size);

  std::vector<std::shared_ptr<vvl::DescriptorSetLayout const>> list;

  const vvl::DescriptorSetLayout* FindFromVariable(const spirv::ResourceInterfaceVariable &variable) const;
  bool HasYcbcrSamplers() const;
};

}  // namespace vvl