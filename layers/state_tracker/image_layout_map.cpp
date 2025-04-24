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
#include "state_tracker/cmd_buffer_state.h"

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
                assert(orig_entry.aspect_mask.has_value());
                orig_entry.Update(new_entry);  // this returns true because of CurrentWillChange check above
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
    if (expected_layout == kInvalidLayout) {
        // Set the initial layout to the set layout as we had no other layout to reference
        expected_layout = layout;
    }

    RangeGenerator range_gen(encoder_, range);
    const LayoutEntry entry(expected_layout, layout);
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
    const LayoutEntry entry(layout);
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
    LayoutEntry entry(layout, view_state);
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
    if (CompatibilityKey() != other.CompatibilityKey()) return false;

    // NOTE -- we are copying plain state pointers from 'other' which owns them in a vector.  This works because
    //         currently this function is only used to import from secondary command buffers, destruction of which
    //         invalidate the referencing primary command buffer, meaning that the dangling pointer will either be
    //         cleaned up in invalidation, on not referenced by validation code.
    return sparse_container::splice(layout_map_, other.layout_map_, LayoutEntry::Updater());
}

ImageLayoutRegistry::LayoutEntry::LayoutEntry(VkImageLayout initial, VkImageLayout current)
    : initial_layout(initial), current_layout(current), aspect_mask(0) {
    assert(current_layout != kInvalidLayout);
}

ImageLayoutRegistry::LayoutEntry::LayoutEntry(VkImageLayout initial)
    : initial_layout(initial), current_layout(kInvalidLayout), aspect_mask(0) {
    assert(initial_layout != kInvalidLayout);
}

ImageLayoutRegistry::LayoutEntry::LayoutEntry(VkImageLayout initial, const vvl::ImageView& view_state)
    : initial_layout(initial), current_layout(kInvalidLayout), aspect_mask(view_state.normalized_subresource_range.aspectMask) {
    assert(initial_layout != kInvalidLayout);
}

bool ImageLayoutRegistry::LayoutEntry::CurrentWillChange(VkImageLayout new_layout) const {
    return new_layout != kInvalidLayout && current_layout != new_layout;
}

bool ImageLayoutRegistry::LayoutEntry::Update(const LayoutEntry& src) {
    bool updated_current = false;
    // current_layout can be updated repeatedly.
    if (CurrentWillChange(src.current_layout)) {
        current_layout = src.current_layout;
        updated_current = true;
    }
    // initial_layout and state cannot be updated once they have a valid value.
    if (initial_layout == kInvalidLayout) {
        initial_layout = src.initial_layout;
    }
    if (!aspect_mask.has_value()) {
        aspect_mask = src.aspect_mask;
    }
    return updated_current;
}

}  // namespace image_layout_map
