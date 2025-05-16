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

using IndexRange = subresource_adapter::IndexRange;
using RangeGenerator = subresource_adapter::RangeGenerator;

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
            LayoutEntry entry = pos->lower_bound->second;  // intentional copy
            // existing entry always has first layout initialized (current layout might be unknown though)
            // 
            // TODO: does it make sense to initialize current layout together with the first layout (set current as first),
            // assuming that submit time validation will detect mismatch between first layout and global layout?
            // Are there contexts when this does not work?
            assert(entry.first_layout != kInvalidLayout);

            const bool update_current =
                new_entry.current_layout != kInvalidLayout && new_entry.current_layout != entry.current_layout;

            auto intersected_range = pos->lower_bound->first & range;

            if (!intersected_range.empty() && update_current) {
                entry.current_layout = new_entry.current_layout;
                auto overwrite_result = layouts.overwrite_range(pos->lower_bound, std::make_pair(intersected_range, entry));
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

CommandBufferImageLayoutMap::CommandBufferImageLayoutMap(const vvl::Image& image_state)
    : subresource_adapter::BothRangeMap<LayoutEntry, 16>(image_state.subresource_encoder.SubresourceCount()),
      image_id(image_state.GetId()) {}

bool UpdateCurrentLayout(CommandBufferImageLayoutMap& image_layout_map, RangeGenerator&& range_gen, VkImageLayout layout,
                         VkImageLayout expected_layout) {
    assert(layout != kInvalidLayout);
    LayoutEntry entry{};
    entry.current_layout = layout;
    // The first layout will be written in layout map only if it was not specified before
    entry.first_layout = (expected_layout != kInvalidLayout) ? expected_layout : layout;

    bool updated = false;

    // Unwrap the BothMaps entry here as this is a performance hotspot
    if (image_layout_map.UsesSmallMap()) {
        auto& layout_map = image_layout_map.GetSmallMap();
        for (; range_gen->non_empty(); ++range_gen) {
            updated |= UpdateLayoutStateImpl(layout_map, *range_gen, entry);
        }
    } else {
        auto& layout_map = image_layout_map.GetBigMap();
        for (; range_gen->non_empty(); ++range_gen) {
            updated |= UpdateLayoutStateImpl(layout_map, *range_gen, entry);
        }
    }
    return updated;
}

void TrackFirstLayout(CommandBufferImageLayoutMap& image_layout_map, RangeGenerator&& range_gen, VkImageLayout expected_layout,
                      VkImageAspectFlags aspect_mask) {
    assert(expected_layout != kInvalidLayout);
    LayoutEntry entry{};
    entry.current_layout = kInvalidLayout;
    entry.first_layout = expected_layout;
    entry.aspect_mask = aspect_mask;

    // Unwrap the BothMaps entry here as this is a performance hotspot
    if (image_layout_map.UsesSmallMap()) {
        auto& layout_map = image_layout_map.GetSmallMap();
        for (; range_gen->non_empty(); ++range_gen) {
            UpdateLayoutStateImpl(layout_map, *range_gen, entry);
        }
    } else {
        auto& layout_map = image_layout_map.GetBigMap();
        for (; range_gen->non_empty(); ++range_gen) {
            UpdateLayoutStateImpl(layout_map, *range_gen, entry);
        }
    }
}

bool AnyInRange(const CommandBufferImageLayoutMap& image_layout_map, const vvl::Image& image_state,
                const VkImageSubresourceRange& normalized_subresource_range,
                std::function<bool(const IndexRange& range, const LayoutEntry& state)>&& func) {
    RangeGenerator range_gen = image_state.subresource_encoder.InRange(normalized_subresource_range)
                                   ? RangeGenerator(image_state.subresource_encoder, normalized_subresource_range)
                                   : RangeGenerator{};

    return AnyInRange(image_layout_map, std::move(range_gen), std::move(func));
}

bool AnyInRange(const CommandBufferImageLayoutMap& image_layout_map, RangeGenerator&& gen,
                std::function<bool(const IndexRange& range, const LayoutEntry& state)>&& func) {
    for (; gen->non_empty(); ++gen) {
        for (auto pos = image_layout_map.lower_bound(*gen); (pos != image_layout_map.end()) && (gen->intersects(pos->first));
             ++pos) {
            if (func(pos->first, pos->second)) {
                return true;
            }
        }
    }
    return false;
}

bool AnyInRange(const ImageLayoutMap& image_layout_map, RangeGenerator& gen,
                std::function<bool(const ImageLayoutMap::key_type& range, VkImageLayout image_layout)>&& func) {
    for (; gen->non_empty(); ++gen) {
        for (auto pos = image_layout_map.lower_bound(*gen); (pos != image_layout_map.end()) && (gen->intersects(pos->first));
             ++pos) {
            if (func(pos->first, pos->second)) {
                return true;
            }
        }
    }
    return false;
}
