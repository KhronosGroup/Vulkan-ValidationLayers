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

using IndexRange = vvl::range<subresource_adapter::IndexType>;
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
            auto intersected_range = pos->lower_bound->first & range;
            if (!intersected_range.empty() && pos->lower_bound->second.CurrentWillChange(new_entry.current_layout)) {
                LayoutEntry orig_entry = pos->lower_bound->second;  // intentional copy

                // existing entry always has initial layout initialized (current layout might be unknown though)
                assert(orig_entry.initial_layout != kInvalidLayout);

                orig_entry.current_layout = new_entry.current_layout;
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

CommandBufferImageLayoutMap::CommandBufferImageLayoutMap(const vvl::Image& image_state)
    : subresource_adapter::BothRangeMap<LayoutEntry, 16>(image_state.subresource_encoder.SubresourceCount()),
      image_state_(image_state) {}

bool CommandBufferImageLayoutMap::SetSubresourceRangeLayout(const VkImageSubresourceRange& range, VkImageLayout layout,
                                                    VkImageLayout expected_layout) {
    if (!image_state_.subresource_encoder.InRange(range)) {
        return false;  // Don't even try to track bogus subresources
    }

    RangeGenerator range_gen(image_state_.subresource_encoder, range);
    const LayoutEntry entry = LayoutEntry::ForCurrentLayout(layout, expected_layout);
    bool updated = false;
    if (UsesSmallMap()) {
        auto& layout_map = GetSmallMap();
        for (; range_gen->non_empty(); ++range_gen) {
            updated |= UpdateLayoutStateImpl(layout_map, *range_gen, entry);
        }
    } else {
        auto& layout_map = GetBigMap();
        for (; range_gen->non_empty(); ++range_gen) {
            updated |= UpdateLayoutStateImpl(layout_map, *range_gen, entry);
        }
    }
    return updated;
}

void CommandBufferImageLayoutMap::SetSubresourceRangeInitialLayout(const VkImageSubresourceRange& range, VkImageLayout layout) {
    if (!image_state_.subresource_encoder.InRange(range)) {
        return;
    }
    RangeGenerator range_gen(image_state_.subresource_encoder, range);

    const LayoutEntry entry = LayoutEntry::ForExpectedLayout(layout);
    // Unwrap the BothMaps entry here as this is a performance hotspot
    if (UsesSmallMap()) {
        auto& layout_map = GetSmallMap();
        for (; range_gen->non_empty(); ++range_gen) {
            UpdateLayoutStateImpl(layout_map, *range_gen, entry);
        }
    } else {
        auto& layout_map = GetBigMap();
        for (; range_gen->non_empty(); ++range_gen) {
            UpdateLayoutStateImpl(layout_map, *range_gen, entry);
        }
    }
}

void CommandBufferImageLayoutMap::SetSubresourceRangeInitialLayout(VkImageLayout layout, const vvl::ImageView& view_state) {
    RangeGenerator range_gen(view_state.range_generator);
    const LayoutEntry entry = LayoutEntry::ForExpectedLayout(layout, view_state.normalized_subresource_range.aspectMask);

    // Unwrap the BothMaps entry here as this is a performance hotspot
    if (UsesSmallMap()) {
        auto& layout_map = GetSmallMap();
        for (; range_gen->non_empty(); ++range_gen) {
            UpdateLayoutStateImpl(layout_map, *range_gen, entry);
        }
    } else {
        auto& layout_map = GetBigMap();
        for (; range_gen->non_empty(); ++range_gen) {
            UpdateLayoutStateImpl(layout_map, *range_gen, entry);
        }
    }
}

uint32_t CommandBufferImageLayoutMap::GetImageId() const { return image_state_.GetId(); }

void CommandBufferImageLayoutMap::UpdateFrom(const CommandBufferImageLayoutMap& other) {
    struct Updater {
        void update(LayoutEntry& dst, const LayoutEntry& src) const {
            if (dst.CurrentWillChange(src.current_layout)) {
                dst.current_layout = src.current_layout;
            }
        }
        std::optional<LayoutEntry> insert(const LayoutEntry& src) const { return std::optional<LayoutEntry>(vvl::in_place, src); }
    };
    sparse_container::splice(*this, other, Updater());
}

VkImageSubresource CommandBufferImageLayoutMap::Decode(subresource_adapter::IndexType index) const {
    const auto subres = image_state_.subresource_encoder.Decode(index);
    return image_state_.subresource_encoder.MakeVkSubresource(subres);
}

bool CommandBufferImageLayoutMap::AnyInRange(const VkImageSubresourceRange& normalized_subresource_range,
                                     std::function<bool(const RangeType& range, const LayoutEntry& state)>&& func) const {
    subresource_adapter::RangeGenerator range_gen =
        image_state_.subresource_encoder.InRange(normalized_subresource_range)
            ? subresource_adapter::RangeGenerator(image_state_.subresource_encoder, normalized_subresource_range)
            : subresource_adapter::RangeGenerator{};

    return AnyInRange(std::move(range_gen), std::move(func));
}

bool CommandBufferImageLayoutMap::AnyInRange(RangeGenerator&& gen,
                                     std::function<bool(const RangeType& range, const LayoutEntry& state)>&& func) const {
    for (; gen->non_empty(); ++gen) {
        for (auto pos = lower_bound(*gen); (pos != end()) && (gen->intersects(pos->first)); ++pos) {
            if (func(pos->first, pos->second)) {
                return true;
            }
        }
    }
    return false;
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

bool LayoutEntry::CurrentWillChange(VkImageLayout new_layout) const {
    return new_layout != kInvalidLayout && current_layout != new_layout;
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
