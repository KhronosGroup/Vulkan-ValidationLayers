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
#include "utils/image_layout_utils.h"

using IndexRange = subresource_adapter::IndexRange;
using RangeGenerator = subresource_adapter::RangeGenerator;

template <typename LayoutsMap>
static bool UpdateLayoutMapRange(LayoutsMap& layout_map, const IndexRange& range, const ImageLayoutState& new_entry) {
    using CachedLowerBound = typename sparse_container::cached_lower_bound_impl<LayoutsMap>;
    CachedLowerBound pos(layout_map, range.begin);
    bool updated_current = false;
    while (range.includes(pos->index)) {
        if (!pos->valid) {
            // Fill in the leading space (or in the case of pos at end the trailing space
            const auto start = pos->index;
            auto it = pos->lower_bound;
            const auto limit = (it != layout_map.end()) ? std::min(it->first.begin, range.end) : range.end;
            auto insert_result = layout_map.insert(it, std::make_pair(IndexRange(start, limit), new_entry));
            pos.invalidate(insert_result, start);
            pos.seek(limit);
            updated_current = true;
        }
        // Note that after the "fill" operation pos may have become valid so we check again
        if (pos->valid) {
            ImageLayoutState entry = pos->lower_bound->second;  // intentional copy
            // existing entry always has first layout initialized (current layout might be unknown though)
            assert(entry.first_layout != kInvalidLayout);

            // TODO: does it make sense to initialize current layout together with the first layout (set current as first),
            // assuming that submit time validation will detect mismatch between first layout and global layout?
            // Are there contexts when this does not work?

            const bool update_current = new_entry.current_layout != kInvalidLayout &&
                                        !ImageLayoutMatches(entry.aspect_mask, new_entry.current_layout, entry.current_layout);

            auto intersected_range = pos->lower_bound->first & range;

            if (!intersected_range.empty() && update_current) {
                entry.current_layout = new_entry.current_layout;
                auto overwrite_result = layout_map.overwrite_range(pos->lower_bound, std::make_pair(intersected_range, entry));
                // If we didn't cover the whole range, we'll need to go around again
                pos.invalidate(overwrite_result, intersected_range.begin);
                pos.seek(intersected_range.end);
                updated_current = true;
            } else {
                // Point just past the end of this section,  if it's within the given range, it will get filled next iteration
                // ++pos could move us past the end of range (which would exit the loop) so we don't use it.
                pos.seek(pos->lower_bound->first.end);
            }
        }
    }

    return updated_current;
}

template <typename LayoutMap>
static bool UpdateLayoutMap(LayoutMap& image_layout_map, RangeGenerator&& range_gen, const ImageLayoutState& entry) {
    bool updated = false;
    // Unwrap the BothMaps entry here as this is a performance hotspot
    if (image_layout_map.UsesSmallMap()) {
        auto& layout_map = image_layout_map.GetSmallMap();
        for (; range_gen->non_empty(); ++range_gen) {
            updated |= UpdateLayoutMapRange(layout_map, *range_gen, entry);
        }
    } else {
        auto& layout_map = image_layout_map.GetBigMap();
        for (; range_gen->non_empty(); ++range_gen) {
            updated |= UpdateLayoutMapRange(layout_map, *range_gen, entry);
        }
    }
    return updated;
}

template <typename LayoutMap>
static bool IterateLayoutMapRanges(
    const LayoutMap& image_layout_map, RangeGenerator&& gen,
    std::function<bool(const IndexRange& range, const typename LayoutMap::mapped_type& layout_state)>&& func) {
    for (; gen->non_empty(); ++gen) {
        for (auto pos = image_layout_map.lower_bound(*gen); pos != image_layout_map.end() && gen->intersects(pos->first); ++pos) {
            // TODO: Usually func returns skip status. Often we accumulate skip and do not initiate immediate return.
            // Investigate if this function should accumuate skip value instead of immediate return.
            if (func(pos->first, pos->second)) {
                return true;
            }
        }
    }
    return false;
}

bool UpdateCurrentLayout(CommandBufferImageLayoutMap& image_layout_map, RangeGenerator&& range_gen, VkImageLayout layout,
                         VkImageLayout expected_layout, VkImageAspectFlags aspect_mask) {
    assert(layout != kInvalidLayout);
    ImageLayoutState entry{};
    entry.current_layout = layout;
    entry.first_layout = (expected_layout != kInvalidLayout) ? expected_layout : layout;
    entry.aspect_mask = aspect_mask;
    return UpdateLayoutMap(image_layout_map, std::move(range_gen), entry);
}

void TrackFirstLayout(CommandBufferImageLayoutMap& image_layout_map, RangeGenerator&& range_gen, VkImageLayout expected_layout,
                      VkImageAspectFlags aspect_mask, const char* submit_time_layout_mismatch_vuid) {
    assert(expected_layout != kInvalidLayout);
    ImageLayoutState entry{};
    entry.current_layout = kInvalidLayout;
    entry.first_layout = expected_layout;
    entry.aspect_mask = aspect_mask;
    entry.submit_time_layout_mismatch_vuid = submit_time_layout_mismatch_vuid;
    UpdateLayoutMap(image_layout_map, std::move(range_gen), entry);
}

bool ForEachMatchingLayoutMapRange(const CommandBufferImageLayoutMap& image_layout_map, RangeGenerator&& gen,
                                   std::function<bool(const IndexRange& range, const ImageLayoutState& entry)>&& func) {
    return IterateLayoutMapRanges(image_layout_map, std::move(gen), std::move(func));
}

bool ForEachMatchingLayoutMapRange(const ImageLayoutMap& image_layout_map, RangeGenerator&& gen,
                                   std::function<bool(const ImageLayoutMap::key_type& range, VkImageLayout image_layout)>&& func) {
    return IterateLayoutMapRanges(image_layout_map, std::move(gen), std::move(func));
}
