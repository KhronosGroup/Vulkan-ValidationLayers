/* Copyright (c) 2019-2021 The Khronos Group Inc.
 * Copyright (c) 2019-2021 Valve Corporation
 * Copyright (c) 2019-2021 LunarG, Inc.
 * Copyright (C) 2019-2021 Google Inc.
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
#include "image_layout_map.h"
#ifndef SPARSE_CONTAINER_UNIT_TEST
#include "image_state.h"
#include "cmd_buffer_state.h"
#endif

namespace image_layout_map {
// Storage for the static state
const ImageSubresourceLayoutMap::ConstIterator ImageSubresourceLayoutMap::end_iterator = ImageSubresourceLayoutMap::ConstIterator();

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
            layouts.insert(it, std::make_pair(IndexRange(start, limit), new_entry));
            // We inserted before pos->lower_bound, so pos->lower_bound isn't invalid, but the associated index *is* and seek
            // will fix this (and move the state to valid)
            pos.seek(limit);
            updated_current = true;
        }
        // Note that after the "fill" operation pos may have become valid so we check again
        if (pos->valid) {
            if (pos->lower_bound->second.CurrentWillChange(new_entry.current_layout)) {
                LayoutEntry orig_entry = pos->lower_bound->second; //intentional copy
                assert(orig_entry.state != nullptr);
                updated_current |= orig_entry.Update(new_entry);

                layouts.overwrite_range(std::make_pair(range, orig_entry));
                break;
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

ImageSubresourceLayoutMap::ConstIterator ImageSubresourceLayoutMap::Begin(bool always_get_initial) const {
    return Find(image_state_.full_range, /* skip_invalid */ true, always_get_initial);
}

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

// Saves an encode to fetch both in the same call
const ImageSubresourceLayoutMap::LayoutEntry* ImageSubresourceLayoutMap::GetSubresourceLayouts(
    const VkImageSubresource& subresource) const {
    IndexType index = encoder_.Encode(subresource);
    auto found = layouts_.find(index);
    if (found != layouts_.end()) {
        return &found->second;
    }
    return nullptr;
}

const InitialLayoutState* ImageSubresourceLayoutMap::GetSubresourceInitialLayoutState(const IndexType index) const {
    const auto found = layouts_.find(index);
    if (found != layouts_.end()) {
        return found->second.state;
    }
    return nullptr;
}

const InitialLayoutState* ImageSubresourceLayoutMap::GetSubresourceInitialLayoutState(const VkImageSubresource& subresource) const {
    if (!InRange(subresource)) return nullptr;
    const auto index = encoder_.Encode(subresource);
    return GetSubresourceInitialLayoutState(index);
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

// This is the same constant value range, subreource position advance logic as ForRange above, but suitable for use with
// an Increment operator.
void ImageSubresourceLayoutMap::ConstIterator::UpdateRangeAndValue() {
    bool not_found = true;
    if (layouts_ == nullptr || layouts_->empty()) {
        return;
    }
    while (iter_ != layouts_->end() && range_gen_->non_empty() && not_found) {
        if (!iter_->first.includes(current_index_)) {  // NOTE: empty ranges can't include anything
            iter_ = layouts_->find(current_index_);
        }
        if (iter_ == layouts_->end() || (iter_->first.empty() && skip_invalid_)) {
            // We're past the end of mapped data, and we aren't interested, so we're done
            // Set end condtion....
            ForceEndCondition();
        }
        // Search within the current range_ for a constant valid constant value interval
        // The while condition allows the iterator to advance constant value ranges as needed.
        while (iter_ != layouts_->end() && range_gen_->includes(current_index_) && not_found) {
            pos_.current_layout = kInvalidLayout;
            pos_.initial_layout = kInvalidLayout;
            constant_value_bound_ = range_gen_->end;
            // The generated range can validly traverse past the end of stored data
            if (!iter_->first.empty()) {
                const LayoutEntry& entry = iter_->second;
                pos_.current_layout = entry.current_layout;
                if (pos_.current_layout == kInvalidLayout || always_get_initial_) {
                    pos_.initial_layout = entry.initial_layout;
                }

                // The constant value bound marks the end of contiguous (w.r.t. range_gen_) indices with the same value, allowing
                // Increment (for example) to forgo this logic until finding a new range is needed.
                constant_value_bound_ = std::min(iter_->first.end, constant_value_bound_);
            }
            if (!skip_invalid_ || (pos_.current_layout != kInvalidLayout) || (pos_.initial_layout != kInvalidLayout)) {
                // we found it ... set the position and exit condition.
                pos_.subresource = range_gen_.GetSubresource();
                not_found = false;
            } else {
                // We're skipping this constant value range, set the index to the exclusive end and look again
                // Note that we ONLY need to Seek the Subresource generator on a skip condition.
                range_gen_.GetSubresourceGenerator().Seek(
                    constant_value_bound_);  // Move the subresource to the end of the skipped range
                current_index_ = constant_value_bound_;

                // Advance the iterator it if needed and possible
                // NOTE: We don't need to seek, as current_index_ can only be in the current or next constant value range
                if (!iter_->first.empty() && !iter_->first.includes(current_index_)) {
                    ++iter_;
                }
            }
        }

        if (not_found) {
            // ++range_gen will update subres_gen.
            ++range_gen_;
            current_index_ = range_gen_->begin;
        }
    }

    if (range_gen_->empty()) {
        ForceEndCondition();
    }
}

void ImageSubresourceLayoutMap::ConstIterator::Increment() {
    ++current_index_;
    ++(range_gen_.GetSubresourceGenerator());
    if (constant_value_bound_ <= current_index_) {
        UpdateRangeAndValue();
    } else {
        pos_.subresource = range_gen_.GetSubresource();
    }
}

void ImageSubresourceLayoutMap::ConstIterator::IncrementInterval() {
    // constant_value_bound_ is the exclusive upper bound of the constant value range.
    // When current index is set to point to that, UpdateRangeAndValue skips to the next constant value range,
    // setting that state as the current position / state for the iterator.
    current_index_ = constant_value_bound_;
    UpdateRangeAndValue();
}

ImageSubresourceLayoutMap::ConstIterator::ConstIterator(const RangeMap& layouts, const Encoder& encoder,
                                                        const VkImageSubresourceRange& subres, bool skip_invalid,
                                                        bool always_get_initial)
    : range_gen_(encoder, subres),
      layouts_(&layouts),
      iter_(layouts.begin()),
      skip_invalid_(skip_invalid),
      always_get_initial_(always_get_initial),
      pos_(),
      current_index_(range_gen_->begin),
      constant_value_bound_() {
    UpdateRangeAndValue();
}

}  // namespace image_layout_map
