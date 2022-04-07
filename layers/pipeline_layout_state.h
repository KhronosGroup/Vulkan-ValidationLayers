/* Copyright (c) 2015-2022 The Khronos Group Inc.
 * Copyright (c) 2015-2022 Valve Corporation
 * Copyright (c) 2015-2022 LunarG, Inc.
 * Copyright (C) 2015-2022 Google Inc.
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
 * Author: Jeremy Gebben <jeremyg@lunarg.com>
 * Author: Nathaniel Cesario <nathaniel@lunarg.com>
 */

#pragma once

#include <vector>
#include <memory>
#include "base_node.h"
#include "hash_util.h"
#include "hash_vk_types.h"
#include "state_tracker.h"

// Fwd declarations -- including descriptor_set.h creates an ugly include loop
namespace cvdescriptorset {
class DescriptorSetLayout;
class DescriptorSetLayoutDef;
}  // namespace cvdescriptorset

class ValidationStateTracker;

// Canonical dictionary for the pipeline layout's layout of descriptorsetlayouts
using DescriptorSetLayoutDef = cvdescriptorset::DescriptorSetLayoutDef;
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
    PipelineLayoutCompatDef(const uint32_t set_index, const PushConstantRangesId pcr_id, const PipelineLayoutSetLayoutsId sl_id)
        : set(set_index), push_constant_ranges(pcr_id), set_layouts_id(sl_id) {}
    size_t hash() const;
    bool operator==(const PipelineLayoutCompatDef &other) const;
};

// Canonical dictionary for PipelineLayoutCompat records
using PipelineLayoutCompatDict = hash_util::Dictionary<PipelineLayoutCompatDef, hash_util::HasHashMember<PipelineLayoutCompatDef>>;
using PipelineLayoutCompatId = PipelineLayoutCompatDict::Id;

// Store layouts and pushconstants for PipelineLayout
class PIPELINE_LAYOUT_STATE : public BASE_NODE {
  public:
    using SetLayoutVector = std::vector<std::shared_ptr<cvdescriptorset::DescriptorSetLayout const>>;
    const SetLayoutVector set_layouts;
    // canonical form IDs for the "compatible for set" contents
    const PushConstantRangesId push_constant_ranges;
    // table of "compatible for set N" cannonical forms for trivial accept validation
    const std::vector<PipelineLayoutCompatId> compat_for_set;
    VkPipelineLayoutCreateFlags create_flags;

    PIPELINE_LAYOUT_STATE(ValidationStateTracker *dev_data, VkPipelineLayout l, const VkPipelineLayoutCreateInfo *pCreateInfo);
    // Merge 2 or more non-overlapping layouts
    PIPELINE_LAYOUT_STATE(const layer_data::span<const PIPELINE_LAYOUT_STATE *const> &layouts);
    template <typename Container>
    PIPELINE_LAYOUT_STATE(const Container &layouts)
        : PIPELINE_LAYOUT_STATE(layer_data::span<const PIPELINE_LAYOUT_STATE *const>{layouts}) {}

    VkPipelineLayout layout() const { return handle_.Cast<VkPipelineLayout>(); }

    std::shared_ptr<cvdescriptorset::DescriptorSetLayout const> GetDsl(uint32_t set) const {
        std::shared_ptr<cvdescriptorset::DescriptorSetLayout const> dsl = nullptr;
        if (set < set_layouts.size()) {
            dsl = set_layouts[set];
        }
        return dsl;
    }

    VkPipelineLayoutCreateFlags CreateFlags() const { return create_flags; }
};
