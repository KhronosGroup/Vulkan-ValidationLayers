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

#include "pipeline_layout_state.h"
#include "state_tracker.h"
#include "descriptor_sets.h"

// Dictionary of canonical form of the pipeline set layout of descriptor set layouts
static PipelineLayoutSetLayoutsDict pipeline_layout_set_layouts_dict;

// Dictionary of canonical form of the "compatible for set" records
static PipelineLayoutCompatDict pipeline_layout_compat_dict;

static PushConstantRangesDict push_constant_ranges_dict;

size_t PipelineLayoutCompatDef::hash() const {
    hash_util::HashCombiner hc;
    // The set number is integral to the CompatDef's distinctiveness
    hc << set << push_constant_ranges.get();
    const auto &descriptor_set_layouts = *set_layouts_id.get();
    for (uint32_t i = 0; i <= set; i++) {
        hc << descriptor_set_layouts[i].get();
    }
    return hc.Value();
}

bool PipelineLayoutCompatDef::operator==(const PipelineLayoutCompatDef &other) const {
    if ((set != other.set) || (push_constant_ranges != other.push_constant_ranges)) {
        return false;
    }

    if (set_layouts_id == other.set_layouts_id) {
        // if it's the same set_layouts_id, then *any* subset will match
        return true;
    }

    // They aren't exactly the same PipelineLayoutSetLayouts, so we need to check if the required subsets match
    const auto &descriptor_set_layouts = *set_layouts_id.get();
    assert(set < descriptor_set_layouts.size());
    const auto &other_ds_layouts = *other.set_layouts_id.get();
    assert(set < other_ds_layouts.size());
    for (uint32_t i = 0; i <= set; i++) {
        if (descriptor_set_layouts[i] != other_ds_layouts[i]) {
            return false;
        }
    }
    return true;
}

static PipelineLayoutCompatId GetCanonicalId(const uint32_t set_index, const PushConstantRangesId pcr_id,
                                             const PipelineLayoutSetLayoutsId set_layouts_id) {
    return pipeline_layout_compat_dict.look_up(PipelineLayoutCompatDef(set_index, pcr_id, set_layouts_id));
}

// For repeatable sorting, not very useful for "memory in range" search
struct PushConstantRangeCompare {
    bool operator()(const VkPushConstantRange *lhs, const VkPushConstantRange *rhs) const {
        if (lhs->offset == rhs->offset) {
            if (lhs->size == rhs->size) {
                // The comparison is arbitrary, but avoids false aliasing by comparing all fields.
                return lhs->stageFlags < rhs->stageFlags;
            }
            // If the offsets are the same then sorting by the end of range is useful for validation
            return lhs->size < rhs->size;
        }
        return lhs->offset < rhs->offset;
    }
};

static PushConstantRangesId GetCanonicalId(const VkPipelineLayoutCreateInfo *info) {
    if (!info->pPushConstantRanges) {
        // Hand back the empty entry (creating as needed)...
        return push_constant_ranges_dict.look_up(PushConstantRanges());
    }

    // Sort the input ranges to ensure equivalent ranges map to the same id
    std::set<const VkPushConstantRange *, PushConstantRangeCompare> sorted;
    for (uint32_t i = 0; i < info->pushConstantRangeCount; i++) {
        sorted.insert(info->pPushConstantRanges + i);
    }

    PushConstantRanges ranges;
    ranges.reserve(sorted.size());
    for (const auto *range : sorted) {
        ranges.emplace_back(*range);
    }
    return push_constant_ranges_dict.look_up(std::move(ranges));
}

static PushConstantRangesId GetPushConstantRangesFromLayouts(const layer_data::span<const PIPELINE_LAYOUT_STATE *const> &layouts) {
    PushConstantRangesId ret{};
    for (const auto *layout : layouts) {
        if (layout && layout->push_constant_ranges) {
            ret = layout->push_constant_ranges;

            if (ret->size() > 0) {
                return ret;
            }
        }
    }
    return ret;
}

static PIPELINE_LAYOUT_STATE::SetLayoutVector GetSetLayouts(ValidationStateTracker *dev_data,
                                                            const VkPipelineLayoutCreateInfo *pCreateInfo) {
    PIPELINE_LAYOUT_STATE::SetLayoutVector set_layouts(pCreateInfo->setLayoutCount);

    for (uint32_t i = 0; i < pCreateInfo->setLayoutCount; ++i) {
        set_layouts[i] = dev_data->Get<cvdescriptorset::DescriptorSetLayout>(pCreateInfo->pSetLayouts[i]);
    }
    return set_layouts;
}

static PIPELINE_LAYOUT_STATE::SetLayoutVector GetSetLayouts(const layer_data::span<const PIPELINE_LAYOUT_STATE *const> &layouts) {
    PIPELINE_LAYOUT_STATE::SetLayoutVector set_layouts;
    size_t num_layouts = 0;
    for (const auto &layout : layouts) {
        if (layout && (layout->set_layouts.size() > num_layouts)) {
            num_layouts = layout->set_layouts.size();
        }
    }

    if (!num_layouts) {
        return {};
    }

    // TODO is this done correctly?
    set_layouts.reserve(num_layouts);
    for (size_t i = 0; i < num_layouts; ++i) {
        const PIPELINE_LAYOUT_STATE *used_layout = nullptr;
        for (const auto *layout : layouts) {
            if (layout) {
                if (layout->set_layouts.size() > i) {
                    // This _could_ be the layout we're looking for
                    used_layout = layout;

                    if ((layout->set_layouts[i]) && (layout->set_layouts[i]->GetBindingCount() > 0)) {
                        // This is the layout we're looking for. Any subsequent ones that match must be identical to this one.
                        break;
                    }
                }
            }
        }
        set_layouts.emplace_back(used_layout->set_layouts[i]);
    }
    return set_layouts;
}

static std::vector<PipelineLayoutCompatId> GetCompatForSet(const PIPELINE_LAYOUT_STATE::SetLayoutVector &set_layouts,
                                                           const PushConstantRangesId &push_constant_ranges) {
    PipelineLayoutSetLayoutsDef set_layout_ids(set_layouts.size());
    for (size_t i = 0; i < set_layouts.size(); i++) {
        if (set_layouts[i]) {
            set_layout_ids[i] = set_layouts[i]->GetLayoutId();
        }
    }
    auto set_layouts_id = pipeline_layout_set_layouts_dict.look_up(set_layout_ids);

    std::vector<PipelineLayoutCompatId> compat_for_set;
    compat_for_set.reserve(set_layouts.size());

    for (uint32_t i = 0; i < set_layouts.size(); i++) {
        compat_for_set.emplace_back(GetCanonicalId(i, push_constant_ranges, set_layouts_id));
    }
    return compat_for_set;
}

VkPipelineLayoutCreateFlags GetCreateFlags(const layer_data::span<const PIPELINE_LAYOUT_STATE *const> &layouts) {
    VkPipelineLayoutCreateFlags flags = 0;
    for (const auto &layout : layouts) {
        if (layout) {
            flags |= layout->CreateFlags();
        }
    }
    return flags;
}

PIPELINE_LAYOUT_STATE::PIPELINE_LAYOUT_STATE(ValidationStateTracker *dev_data, VkPipelineLayout l,
                                             const VkPipelineLayoutCreateInfo *pCreateInfo)
    : BASE_NODE(l, kVulkanObjectTypePipelineLayout),
      set_layouts(GetSetLayouts(dev_data, pCreateInfo)),
      push_constant_ranges(GetCanonicalId(pCreateInfo)),
      compat_for_set(GetCompatForSet(set_layouts, push_constant_ranges)),
      create_flags(pCreateInfo->flags) {}

PIPELINE_LAYOUT_STATE::PIPELINE_LAYOUT_STATE(const layer_data::span<const PIPELINE_LAYOUT_STATE *const> &layouts)
    : BASE_NODE(static_cast<VkPipelineLayout>(VK_NULL_HANDLE), kVulkanObjectTypePipelineLayout),
      set_layouts(GetSetLayouts(layouts)),
      push_constant_ranges(GetPushConstantRangesFromLayouts(layouts)),  // TODO is this correct?
      compat_for_set(GetCompatForSet(set_layouts, push_constant_ranges)),
      create_flags(GetCreateFlags(layouts)) {}
