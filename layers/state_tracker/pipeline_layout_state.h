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

#pragma once

#include <vector>
#include <memory>
#include "state_tracker/state_object.h"
#include "state_tracker/descriptor_set_layouts.h"
#include "utils/hash_util.h"
#include "utils/hash_vk_types.h"
#include "containers/span.h"

// Fwd declarations -- including descriptor_set.h creates an ugly include loop
namespace vvl {
class DeviceState;
class DescriptorSetLayout;
class DescriptorSetLayoutDef;
}  // namespace vvl

// Canonical dictionary for the pipeline layout's layout of descriptorsetlayouts
using DescriptorSetLayoutDef = vvl::DescriptorSetLayoutDef;
using DescriptorSetLayoutId = std::shared_ptr<const DescriptorSetLayoutDef>;
using PipelineLayoutSetLayoutsDef = std::vector<DescriptorSetLayoutId>;
using PipelineLayoutSetLayoutsDict =
    hash_util::Dictionary<PipelineLayoutSetLayoutsDef, hash_util::IsOrderedContainer<PipelineLayoutSetLayoutsDef>>;
using PipelineLayoutSetLayoutsId = PipelineLayoutSetLayoutsDict::Id;

// Canonical dictionary for PushConstantRanges
using PushConstantRangesDict = hash_util::Dictionary<PushConstantRanges>;
using PushConstantRangesId = PushConstantRangesDict::Id;

// Defines/stores a compatibility defintion for set N
// The "layout layout" must store at least set+1 entries, but only the first set+1 are considered for hash and equality testing
// Note: the "cannonical" data are referenced by Id, not including handle or device specific state
// Note: hash and equality only consider layout_id entries [0, set] for determining uniqueness
struct PipelineLayoutCompatDef {
    uint32_t set;
    PushConstantRangesId push_constant_ranges;
    PipelineLayoutSetLayoutsId set_layouts_id;
    // VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT or VK_SHADER_CREATE_INDEPENDENT_SETS_BIT_KHR
    bool is_independent_sets;
    // When report mismatch errors, it is confusing for ShaderObject because there is no 'vkCmdBindPipeline' and the PipelineLayout
    // we are comparing is just a "fake" wrapper our the VkShaderCreateInfoEXT::pSetLayouts to make the pipeline and shaderObject
    // logic the same
    bool from_shader_object;

    PipelineLayoutCompatDef(const uint32_t set_index, const PushConstantRangesId pcr_id, const PipelineLayoutSetLayoutsId sl_id,
                            bool is_independent_sets, bool from_shader_object)
        : set(set_index),
          push_constant_ranges(pcr_id),
          set_layouts_id(sl_id),
          is_independent_sets(is_independent_sets),
          from_shader_object(from_shader_object) {}
    size_t hash() const;

    bool operator==(const PipelineLayoutCompatDef &other) const;
    std::string DescribeDifference(const PipelineLayoutCompatDef &other) const;
};

// Canonical dictionary for PipelineLayoutCompat records
using PipelineLayoutCompatDict = hash_util::Dictionary<PipelineLayoutCompatDef, hash_util::HasHashMember<PipelineLayoutCompatDef>>;
using PipelineLayoutCompatId = PipelineLayoutCompatDict::Id;

PushConstantRangesId GetCanonicalId(uint32_t pushConstantRangeCount, const VkPushConstantRange *pPushConstantRanges);

namespace vvl {

// Store layouts and pushconstants for PipelineLayout
class PipelineLayout : public StateObject {
  public:
    const DescriptorSetLayoutList set_layouts;
    // canonical form IDs for the "compatible for set" contents
    const PushConstantRangesId push_constant_ranges_layout;
    VkPipelineLayoutCreateFlags create_flags;

    // To match the shader object variation
    bool is_independent_set;
    // When the sets are using VK_EXT_descriptor_buffer
    bool has_descriptor_buffer;
    // Way to quick prevent searching if we know there are no immutable samplers
    bool has_immutable_samplers;

    // table of "compatible for set N" canonical forms for trivial accept validation
    const std::vector<PipelineLayoutCompatId> set_compat_ids;

    PipelineLayout(DeviceState &dev_data, VkPipelineLayout handle, const VkPipelineLayoutCreateInfo *pCreateInfo);
    // Merge 2 or more non-overlapping layouts
    PipelineLayout(const vvl::span<const PipelineLayout *const> &layouts);
    template <typename Container>
    PipelineLayout(const Container &layouts) : PipelineLayout(vvl::span<const PipelineLayout *const>{layouts}) {}

    VkPipelineLayout VkHandle() const { return handle_.Cast<VkPipelineLayout>(); }

    VkPipelineLayoutCreateFlags CreateFlags() const { return create_flags; }
    bool IsIndependentSets() const { return (create_flags & VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT) != 0; }
};

}  // namespace vvl

std::vector<PipelineLayoutCompatId> GetCompatForSet(const vvl::DescriptorSetLayoutList& set_layouts,
                                                    const PushConstantRangesId& push_constant_ranges, bool is_independent_sets,
                                                    bool from_shader_object);
