/* Copyright (c) 2023-2024 The Khronos Group Inc.
 * Copyright (c) 2023-2024 Valve Corporation
 * Copyright (c) 2023-2024 LunarG, Inc.
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
#include "state_tracker/state_tracker.h"
#include "state_tracker/descriptor_sets.h"
#include "utils/shader_utils.h"
#include "drawdispatch/drawdispatch_vuids.h"

namespace vvl {
using DescriptorBindingInfo = std::pair<uint32_t, std::vector<DescriptorRequirement>>;

class DescriptorValidator {
 public:
   DescriptorValidator(ValidationStateTracker& dev, vvl::CommandBuffer& cb, vvl::DescriptorSet& set, VkFramebuffer fb,
                       const Location& l)
       : dev_state(dev), cb_state(cb), descriptor_set(set), framebuffer(fb), loc(l), vuids(GetDrawDispatchVuid(loc.function)) {}

   template <typename T>
   std::string FormatHandle(T&& h) const {
       return dev_state.FormatHandle(std::forward<T>(h));
    }

    bool ValidateBinding(const DescriptorBindingInfo& binding_info, const vvl::DescriptorBinding& binding) const;
    bool ValidateBinding(const DescriptorBindingInfo& binding_info, const std::vector<uint32_t> &indices);

 private:
    template <typename T>
    bool ValidateDescriptors(const DescriptorBindingInfo& binding_info, const T& binding) const;

    template <typename T>
    bool ValidateDescriptors(const DescriptorBindingInfo& binding_info, const T& binding, const std::vector<uint32_t>& indices);


    bool ValidateDescriptor(const DescriptorBindingInfo& binding_info, uint32_t index,
                            VkDescriptorType descriptor_type, const vvl::BufferDescriptor& descriptor) const;
    bool ValidateDescriptor(const DescriptorBindingInfo& binding_info, uint32_t index,
                            VkDescriptorType descriptor_type, const vvl::ImageDescriptor& descriptor) const;
    bool ValidateDescriptor(const DescriptorBindingInfo& binding_info, uint32_t index,
                            VkDescriptorType descriptor_type, const vvl::ImageSamplerDescriptor& descriptor) const;
    bool ValidateDescriptor(const DescriptorBindingInfo& binding_info, uint32_t index,
                            VkDescriptorType descriptor_type, const vvl::TexelDescriptor& descriptor) const;
    bool ValidateDescriptor(const DescriptorBindingInfo& binding_info, uint32_t index,
                            VkDescriptorType descriptor_type,
                            const vvl::AccelerationStructureDescriptor& descriptor) const;
    bool ValidateDescriptor(const DescriptorBindingInfo& binding_info, uint32_t index,
                            VkDescriptorType descriptor_type, const vvl::SamplerDescriptor& descriptor) const;

    // helper for the common parts of ImageSamplerDescriptor and SamplerDescriptor validation
    bool ValidateSamplerDescriptor(const DescriptorBindingInfo& binding_info, uint32_t index, VkSampler sampler, bool is_immutable,
                                   const vvl::Sampler* sampler_state) const;

    ValidationStateTracker& dev_state;
    vvl::CommandBuffer& cb_state;
    vvl::DescriptorSet& descriptor_set;
    const VkFramebuffer framebuffer;
    const Location& loc;
    const DrawDispatchVuid& vuids;

};
}
