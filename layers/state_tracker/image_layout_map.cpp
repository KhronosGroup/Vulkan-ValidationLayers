/* Copyright (c) 2019-2023 The Khronos Group Inc.
 * Copyright (c) 2019-2023 Valve Corporation
 * Copyright (c) 2019-2023 LunarG, Inc.
 * Copyright (C) 2019-2023 Google Inc.
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
#ifndef SPARSE_CONTAINER_UNIT_TEST
#include "state_tracker/image_state.h"
#include "state_tracker/cmd_buffer_state.h"
#endif

namespace image_layout_map {
using InitialLayoutStates = ImageSubresourceLayoutMap::InitialLayoutStates;
using LayoutEntry = ImageSubresourceLayoutMap::LayoutEntry;

template <typename LayoutsMap>
static bool UpdateLayoutStateImpl(LayoutsMap& layouts, InitialLayoutStates& initial_layout_states, const IndexRange& range,
                                  LayoutEntry& new_entry, const CMD_BUFFER_STATE& cb_state, const IMAGE_VIEW_STATE* view_state) {
    using CachedLowerBound = typename sparse_container::cached_lower_bound_impl<LayoutsMap>;
    CachedLowerBound pos(layouts, range.begin);
    if (!range.includes(pos->index)) {
        return false;
    }
    bool updated_current = false;
    while (range.includes(pos->index)) {
        if (!pos->valid) {
            // Fill in the leading space (or in the case of pos at end the trailing space
            const auto start = pos->index;
            auto it = pos->lower_bound;
            const auto limit = (it != layouts.end()) ? std::min(it->first.begin, range.end) : range.end;
            if (new_entry.state == nullptr) {
                // Allocate on demand...  initial_layout_states_ holds ownership, while
                // each subresource has a non-owning copy of the plain pointer.
                initial_layout_states.emplace_back(cb_state, view_state);
                new_entry.state = &initial_layout_states.back();
            }
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
                assert(orig_entry.state != nullptr);
                updated_current |= orig_entry.Update(new_entry);
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

InitialLayoutState::InitialLayoutState(const CMD_BUFFER_STATE& cb_state_, const IMAGE_VIEW_STATE* view_state_)
    : image_view(VK_NULL_HANDLE), aspect_mask(0), label(cb_state_.debug_label) {
    if (view_state_) {
        image_view = view_state_->image_view();
        aspect_mask = view_state_->normalized_subresource_range.aspectMask;
    }
}
bool ImageSubresourceLayoutMap::SubresourceLayout::operator==(const ImageSubresourceLayoutMap::SubresourceLayout& rhs) const {
    bool is_equal =
        (current_layout == rhs.current_layout) && (initial_layout == rhs.initial_layout) && (subresource == rhs.subresource);
    return is_equal;
}
ImageSubresourceLayoutMap::ImageSubresourceLayoutMap(const IMAGE_STATE& image_state)
    : image_state_(image_state),
      encoder_(image_state.subresource_encoder),
      layouts_(encoder_.SubresourceCount()),
      initial_layout_states_() {}

// Use the unwrapped maps from the BothMap in the actual implementation
template <typename LayoutMap>
static bool SetSubresourceRangeLayoutImpl(LayoutMap& layouts, InitialLayoutStates& initial_layout_states, RangeGenerator& range_gen,
                                          const CMD_BUFFER_STATE& cb_state, VkImageLayout layout, VkImageLayout expected_layout) {
    bool updated = false;
    LayoutEntry entry(expected_layout, layout);
    for (; range_gen->non_empty(); ++range_gen) {
        updated |= UpdateLayoutStateImpl(layouts, initial_layout_states, *range_gen, entry, cb_state, nullptr);
    }
    return updated;
}

bool ImageSubresourceLayoutMap::SetSubresourceRangeLayout(const CMD_BUFFER_STATE& cb_state, const VkImageSubresourceRange& range,
                                                          VkImageLayout layout, VkImageLayout expected_layout) {
    if (expected_layout == kInvalidLayout) {
        // Set the initial layout to the set layout as we had no other layout to reference
        expected_layout = layout;
    }
    if (!InRange(range)) return false;  // Don't even try to track bogus subreources

    RangeGenerator range_gen(encoder_, range);
    if (layouts_.SmallMode()) {
        return SetSubresourceRangeLayoutImpl(layouts_.GetSmallMap(), initial_layout_states_, range_gen, cb_state, layout,
                                             expected_layout);
    } else {
        assert(!layouts_.Tristate());
        return SetSubresourceRangeLayoutImpl(layouts_.GetBigMap(), initial_layout_states_, range_gen, cb_state, layout,
                                             expected_layout);
    }
}

// Use the unwrapped maps from the BothMap in the actual implementation
template <typename LayoutMap>
static void SetSubresourceRangeInitialLayoutImpl(LayoutMap& layouts, InitialLayoutStates& initial_layout_states,
                                                 RangeGenerator& range_gen, const CMD_BUFFER_STATE& cb_state, VkImageLayout layout,
                                                 const IMAGE_VIEW_STATE* view_state) {
    LayoutEntry entry(layout);
    for (; range_gen->non_empty(); ++range_gen) {
        UpdateLayoutStateImpl(layouts, initial_layout_states, *range_gen, entry, cb_state, view_state);
    }
}

// Unwrap the BothMaps entry here as this is a performance hotspot.
void ImageSubresourceLayoutMap::SetSubresourceRangeInitialLayout(const CMD_BUFFER_STATE& cb_state,
                                                                 const VkImageSubresourceRange& range, VkImageLayout layout) {
    if (!InRange(range)) return;  // Don't even try to track bogus subreources

    RangeGenerator range_gen(encoder_, range);
    if (layouts_.SmallMode()) {
        SetSubresourceRangeInitialLayoutImpl(layouts_.GetSmallMap(), initial_layout_states_, range_gen, cb_state, layout, nullptr);
    } else {
        assert(!layouts_.Tristate());
        SetSubresourceRangeInitialLayoutImpl(layouts_.GetBigMap(), initial_layout_states_, range_gen, cb_state, layout, nullptr);
    }
}

// Unwrap the BothMaps entry here as this is a performance hotspot.
void ImageSubresourceLayoutMap::SetSubresourceRangeInitialLayout(const CMD_BUFFER_STATE& cb_state, VkImageLayout layout,
                                                                 const IMAGE_VIEW_STATE& view_state) {
    RangeGenerator range_gen(view_state.range_generator);
    if (layouts_.SmallMode()) {
        SetSubresourceRangeInitialLayoutImpl(layouts_.GetSmallMap(), initial_layout_states_, range_gen, cb_state, layout,
                                             &view_state);
    } else {
        assert(!layouts_.Tristate());
        SetSubresourceRangeInitialLayoutImpl(layouts_.GetBigMap(), initial_layout_states_, range_gen, cb_state, layout,
                                             &view_state);
    }
}

// TODO: make sure this paranoia check is sufficient and not too much.
uintptr_t ImageSubresourceLayoutMap::CompatibilityKey() const {
    return (reinterpret_cast<uintptr_t>(&image_state_) ^ encoder_.AspectMask());
}

bool ImageSubresourceLayoutMap::UpdateFrom(const ImageSubresourceLayoutMap& other) {
    // Must be from matching images for the reinterpret cast to be valid
    assert(CompatibilityKey() == other.CompatibilityKey());
    if (CompatibilityKey() != other.CompatibilityKey()) return false;

    // NOTE -- we are copying plain state pointers from 'other' which owns them in a vector.  This works because
    //         currently this function is only used to import from secondary command buffers, destruction of which
    //         invalidate the referencing primary command buffer, meaning that the dangling pointer will either be
    //         cleaned up in invalidation, on not referenced by validation code.
    return sparse_container::splice(layouts_, other.layouts_, LayoutEntry::Updater());
}

}  // namespace image_layout_map
