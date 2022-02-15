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

static PIPELINE_LAYOUT_STATE::SetLayoutVector GetSetLayouts(ValidationStateTracker *dev_data,
                                                            const VkPipelineLayoutCreateInfo *pCreateInfo) {
    PIPELINE_LAYOUT_STATE::SetLayoutVector set_layouts(pCreateInfo->setLayoutCount);

    for (uint32_t i = 0; i < pCreateInfo->setLayoutCount; ++i) {
        set_layouts[i] = dev_data->Get<cvdescriptorset::DescriptorSetLayout>(pCreateInfo->pSetLayouts[i]);
    }
    return set_layouts;
}

static std::vector<PipelineLayoutCompatId> GetCompatForSet(const PIPELINE_LAYOUT_STATE::SetLayoutVector &set_layouts,
                                                           const PushConstantRangesId &push_constant_ranges) {
    PipelineLayoutSetLayoutsDef set_layout_ids(set_layouts.size());
    for (size_t i = 0; i < set_layouts.size(); i++) {
        set_layout_ids[i] = set_layouts[i]->GetLayoutId();
    }
    auto set_layouts_id = pipeline_layout_set_layouts_dict.look_up(set_layout_ids);

    std::vector<PipelineLayoutCompatId> compat_for_set;
    compat_for_set.reserve(set_layouts.size());

    for (uint32_t i = 0; i < set_layouts.size(); i++) {
        compat_for_set.emplace_back(GetCanonicalId(i, push_constant_ranges, set_layouts_id));
    }
    return compat_for_set;
}

PIPELINE_LAYOUT_STATE::PIPELINE_LAYOUT_STATE(ValidationStateTracker *dev_data, VkPipelineLayout l,
                                             const VkPipelineLayoutCreateInfo *pCreateInfo)
    : BASE_NODE(l, kVulkanObjectTypePipelineLayout),
      set_layouts(GetSetLayouts(dev_data, pCreateInfo)),
      push_constant_ranges(GetCanonicalId(pCreateInfo)),
      compat_for_set(GetCompatForSet(set_layouts, push_constant_ranges)) {}
