/* Copyright (c) 2019-2025 The Khronos Group Inc.
 * Copyright (c) 2019-2025 Valve Corporation
 * Copyright (c) 2019-2025 LunarG, Inc.
 * Copyright (C) 2019-2025 Google Inc.
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
 * John Zulauf <jzulauf@lunarg.com>
 *
 */
#include "state_tracker/image_layout_map.h"
#include "state_tracker/image_state.h"

namespace image_layout_map {
using LayoutEntry = ImageLayoutRegistry::LayoutEntry;

template <typename LayoutsMap>
static bool UpdateLayoutStateImpl(LayoutsMap& layouts, const IndexRange& range, const LayoutEntry& new_entry) {
    using CachedLowerBound = typename sparse_container::cached_lower_bound_impl<LayoutsMap>;
    CachedLowerBound pos(layouts, range.begin);
    bool updated_current = false;
    while (range.includes(pos->index)) {
        if (!pos->valid) {
            // Fill in the leading space (or in the case of pos at end the trailing space
            const auto start = pos->index;
            auto it = pos->lower_bound;
            const auto limit = (it != layouts.end()) ? std::min(it->first.begin, range.end) : range.end;
            auto insert_result = layouts.insert(it, std::make_pair(IndexRange(start, limit), new_entry));
            pos.invalidate(insert_result, start);
            pos.seek(limit);
            updated_current = true;
        }
        // Note that after the "fill" operation pos may have become valid so we check again
        if (pos->valid) {
            auto intersected_range = pos->lower_bound->first & range;
            if (!intersected_range.empty() && pos->lower_bound->second.CurrentWillChange(new_entry.current_layout)) {
                LayoutEntry orig_entry = pos->lower_bound->second;  // intentional copy
                orig_entry.Update(new_entry);                       // this returns true because of CurrentWillChange check above
                updated_current = true;
                auto overwrite_result = layouts.overwrite_range(pos->lower_bound, std::make_pair(intersected_range, orig_entry));
                // If we didn't cover the whole range, we'll need to go around again
                pos.invalidate(overwrite_result, intersected_range.begin);
                pos.seek(intersected_range.end);
            } else {
                // Point just past the end of this section,  if it's within the given range, it will get filled next iteration
                // ++pos could move us past the end of range (which would exit the loop) so we don't use it.
                pos.seek(pos->lower_bound->first.end);
            }
        }
    }

    return updated_current;
}

ImageLayoutRegistry::ImageLayoutRegistry(const vvl::Image& image_state)
    : image_state_(image_state), encoder_(image_state.subresource_encoder), layout_map_(encoder_.SubresourceCount()) {}

bool ImageLayoutRegistry::SetSubresourceRangeLayout(const VkImageSubresourceRange& range, VkImageLayout layout,
                                                    VkImageLayout expected_layout) {
    if (!InRange(range)) {
        return false;  // Don't even try to track bogus subresources
    }

    RangeGenerator range_gen(encoder_, range);
    const LayoutEntry entry = LayoutEntry::ForCurrentLayout(layout, expected_layout);
    bool updated = false;
    if (layout_map_.UsesSmallMap()) {
        auto& layout_map = layout_map_.GetSmallMap();
        for (; range_gen->non_empty(); ++range_gen) {
            updated |= UpdateLayoutStateImpl(layout_map, *range_gen, entry);
        }
    } else {
        auto& layout_map = layout_map_.GetBigMap();
        for (; range_gen->non_empty(); ++range_gen) {
            updated |= UpdateLayoutStateImpl(layout_map, *range_gen, entry);
        }
    }
    return updated;
}

// Unwrap the BothMaps entry here as this is a performance hotspot.
void ImageLayoutRegistry::SetSubresourceRangeInitialLayout(const VkImageSubresourceRange& range, VkImageLayout layout) {
    if (!InRange(range)) {
        return;  // Don't even try to track bogus subreources
    }

    RangeGenerator range_gen(encoder_, range);
    const LayoutEntry entry = LayoutEntry::ForExpectedLayout(layout);
    if (layout_map_.UsesSmallMap()) {
        auto& layout_map = layout_map_.GetSmallMap();
        for (; range_gen->non_empty(); ++range_gen) {
            UpdateLayoutStateImpl(layout_map, *range_gen, entry);
        }
    } else {
        auto& layout_map = layout_map_.GetBigMap();
        for (; range_gen->non_empty(); ++range_gen) {
            UpdateLayoutStateImpl(layout_map, *range_gen, entry);
        }
    }
}

// Unwrap the BothMaps entry here as this is a performance hotspot.
void ImageLayoutRegistry::SetSubresourceRangeInitialLayout(VkImageLayout layout, const vvl::ImageView& view_state) {
    RangeGenerator range_gen(view_state.range_generator);
    const LayoutEntry entry = LayoutEntry::ForExpectedLayout(layout, view_state.normalized_subresource_range.aspectMask);
    if (layout_map_.UsesSmallMap()) {
        auto& layout_map = layout_map_.GetSmallMap();
        for (; range_gen->non_empty(); ++range_gen) {
            UpdateLayoutStateImpl(layout_map, *range_gen, entry);
        }
    } else {
        auto& layout_map = layout_map_.GetBigMap();
        for (; range_gen->non_empty(); ++range_gen) {
            UpdateLayoutStateImpl(layout_map, *range_gen, entry);
        }
    }
}

// TODO: make sure this paranoia check is sufficient and not too much.
uintptr_t ImageLayoutRegistry::CompatibilityKey() const {
    return (reinterpret_cast<uintptr_t>(&image_state_) ^ encoder_.AspectMask());
}

uint32_t ImageLayoutRegistry::GetImageId() const { return image_state_.GetId(); }

bool ImageLayoutRegistry::UpdateFrom(const ImageLayoutRegistry& other) {
    // Must be from matching images for the reinterpret cast to be valid
    assert(CompatibilityKey() == other.CompatibilityKey());
    if (CompatibilityKey() != other.CompatibilityKey()) {
        return false;
    }
    return sparse_container::splice(layout_map_, other.layout_map_, LayoutEntry::Updater());
}

LayoutEntry LayoutEntry::ForCurrentLayout(VkImageLayout current_layout, VkImageLayout expected_layout) {
    assert(current_layout != kInvalidLayout);
    LayoutEntry entry{};
    entry.initial_layout = (expected_layout != kInvalidLayout) ? expected_layout : current_layout;
    entry.current_layout = current_layout;
    return entry;
}

LayoutEntry LayoutEntry::ForExpectedLayout(VkImageLayout expected_layout, VkImageAspectFlags aspect_mask) {
    assert(expected_layout != kInvalidLayout);
    LayoutEntry entry{};
    entry.initial_layout = expected_layout;
    entry.current_layout = kInvalidLayout;
    entry.aspect_mask = aspect_mask;
    return entry;
}

bool ImageLayoutRegistry::LayoutEntry::CurrentWillChange(VkImageLayout new_layout) const {
    return new_layout != kInvalidLayout && current_layout != new_layout;
}

bool ImageLayoutRegistry::LayoutEntry::Update(const LayoutEntry& src) {
    bool updated_current = false;
    if (CurrentWillChange(src.current_layout)) {
        current_layout = src.current_layout;
        updated_current = true;
    }
    return updated_current;
}

}  // namespace image_layout_map

bool ImageLayoutRangeMap::AnyInRange(RangeGenerator& gen,
                                     std::function<bool(const key_type& range, const mapped_type& state)>&& func) const {
    for (; gen->non_empty(); ++gen) {
        for (auto pos = lower_bound(*gen); (pos != end()) && (gen->intersects(pos->first)); ++pos) {
            if (func(pos->first, pos->second)) {
                return true;
            }
        }
    }
    return false;
}