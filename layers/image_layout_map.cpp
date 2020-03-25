/* Copyright (c) 2019-2020 The Khronos Group Inc.
 * Copyright (c) 2019-2020 Valve Corporation
 * Copyright (c) 2019-2020 LunarG, Inc.
 * Copyright (C) 2019-2020 Google Inc.
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
#ifdef SPARSE_CONTAINER_UNIT_TEST
#include "image_layout_map.h"
#else
#include "core_validation_types.h"
#include "chassis.h"
#include "descriptor_sets.h"
#endif

namespace image_layout_map {
// Storage for the static state
const ImageSubresourceLayoutMap::ConstIterator ImageSubresourceLayoutMap::end_iterator = ImageSubresourceLayoutMap::ConstIterator();

using InitialLayoutStates = ImageSubresourceLayoutMap::InitialLayoutStates;

template <typename StatesMap>
static inline InitialLayoutState* UpdateInitialLayoutStateImpl(StatesMap* initial_layout_state_map,
                                                               InitialLayoutStates* states_storage, const IndexRange& range,
                                                               InitialLayoutState* initial_state, const CMD_BUFFER_STATE& cb_state,
                                                               const IMAGE_VIEW_STATE* view_state) {
    auto& initial_layout_states = *states_storage;
    if (!initial_state) {
        // Allocate on demand...  initial_layout_states_ holds ownership as a unique_ptr, while
        // each subresource has a non-owning copy of the plain pointer.
        initial_state = new InitialLayoutState(cb_state, view_state);
        initial_layout_states.emplace_back(initial_state);
    }
    assert(initial_state);
    sparse_container::update_range_value(*initial_layout_state_map, range, initial_state, WritePolicy::prefer_dest);
    return initial_state;
}

InitialLayoutState::InitialLayoutState(const CMD_BUFFER_STATE& cb_state_, const IMAGE_VIEW_STATE* view_state_)
    : image_view(VK_NULL_HANDLE), aspect_mask(0), label(cb_state_.debug_label) {
    if (view_state_) {
        image_view = view_state_->image_view;
        aspect_mask = view_state_->create_info.subresourceRange.aspectMask;
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
      initial_layout_states_(),
      initial_layout_state_map_(encoder_.SubresourceCount()) {}

ImageSubresourceLayoutMap::ConstIterator ImageSubresourceLayoutMap::Begin(bool always_get_initial) const {
    return Find(image_state_.full_range, /* skip_invalid */ true, always_get_initial);
}

// Use the unwrapped maps from the BothMap in the actual implementation
template <typename LayoutMap, typename InitialStateMap>
static bool SetSubresourceRangeLayoutImpl(LayoutMap* current_layouts, LayoutMap* initial_layouts,
                                          InitialStateMap* initial_state_map, InitialLayoutStates* initial_layout_states,
                                          RangeGenerator* range_gen_arg, const CMD_BUFFER_STATE& cb_state, VkImageLayout layout,
                                          VkImageLayout expected_layout) {
    bool updated = false;
    auto& range_gen = *range_gen_arg;
    InitialLayoutState* initial_state = nullptr;
    // Empty range are the range tombstones
    for (; range_gen->non_empty(); ++range_gen) {
        // In order to track whether we've changed anything, we'll do this in a slightly convoluted way...
        // We'll traverse the range looking for values different from ours, then overwrite the range.
        bool updated_current =
            sparse_container::update_range_value(*current_layouts, *range_gen, layout, WritePolicy::prefer_source);
        if (updated_current) {
            updated = true;
            bool updated_init =
                sparse_container::update_range_value(*initial_layouts, *range_gen, expected_layout, WritePolicy::prefer_dest);
            if (updated_init) {
                initial_state = UpdateInitialLayoutStateImpl(initial_state_map, initial_layout_states, *range_gen, initial_state,
                                                             cb_state, nullptr);
            }
        }
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
    if (layouts_.initial.SmallMode()) {
        return SetSubresourceRangeLayoutImpl(&layouts_.current.GetSmallMap(), &layouts_.initial.GetSmallMap(),
                                             &initial_layout_state_map_.GetSmallMap(), &initial_layout_states_, &range_gen,
                                             cb_state, layout, expected_layout);
    } else {
        assert(!layouts_.initial.Tristate());
        return SetSubresourceRangeLayoutImpl(&layouts_.current.GetBigMap(), &layouts_.initial.GetBigMap(),
                                             &initial_layout_state_map_.GetBigMap(), &initial_layout_states_, &range_gen, cb_state,
                                             layout, expected_layout);
    }
}

// Use the unwrapped maps from the BothMap in the actual implementation
template <typename LayoutMap, typename InitialStateMap>
static bool SetSubresourceRangeInitialLayoutImpl(LayoutMap* initial_layouts, InitialStateMap* initial_state_map,
                                                 InitialLayoutStates* initial_layout_states, RangeGenerator* range_gen_arg,
                                                 const CMD_BUFFER_STATE& cb_state, VkImageLayout layout,
                                                 const IMAGE_VIEW_STATE* view_state) {
    bool updated = false;
    InitialLayoutState* initial_state = nullptr;
    auto& range_gen = *range_gen_arg;

    for (; range_gen->non_empty(); ++range_gen) {
        bool updated_range = sparse_container::update_range_value(*initial_layouts, *range_gen, layout, WritePolicy::prefer_dest);
        if (updated_range) {
            initial_state = UpdateInitialLayoutStateImpl(initial_state_map, initial_layout_states, *range_gen, initial_state,
                                                         cb_state, view_state);
            updated = true;
        }
    }
    return updated;
}

// Unwrap the BothMaps entry here as this is a performance hotspot.
bool ImageSubresourceLayoutMap::SetSubresourceRangeInitialLayout(const CMD_BUFFER_STATE& cb_state,
                                                                 const VkImageSubresourceRange& range, VkImageLayout layout,
                                                                 const IMAGE_VIEW_STATE* view_state) {
    if (!InRange(range)) return false;  // Don't even try to track bogus subreources

    RangeGenerator range_gen(encoder_, range);
    assert(layouts_.initial.GetMode() == initial_layout_state_map_.GetMode());
    if (layouts_.initial.SmallMode()) {
        return SetSubresourceRangeInitialLayoutImpl(&layouts_.initial.GetSmallMap(), &initial_layout_state_map_.GetSmallMap(),
                                                    &initial_layout_states_, &range_gen, cb_state, layout, view_state);
    } else {
        assert(!layouts_.initial.Tristate());
        return SetSubresourceRangeInitialLayoutImpl(&layouts_.initial.GetBigMap(), &initial_layout_state_map_.GetBigMap(),
                                                    &initial_layout_states_, &range_gen, cb_state, layout, view_state);
    }
}

// Unwrap the BothMaps entry here as this is a performance hotspot.
bool ImageSubresourceLayoutMap::SetSubresourceRangeInitialLayout(const CMD_BUFFER_STATE& cb_state, VkImageLayout layout,
                                                                 const IMAGE_VIEW_STATE& view_state) {
    RangeGenerator range_gen(view_state.range_generator);
    assert(layouts_.initial.GetMode() == initial_layout_state_map_.GetMode());
    if (layouts_.initial.SmallMode()) {
        return SetSubresourceRangeInitialLayoutImpl(&layouts_.initial.GetSmallMap(), &initial_layout_state_map_.GetSmallMap(),
                                                    &initial_layout_states_, &range_gen, cb_state, layout, &view_state);
    } else {
        assert(!layouts_.initial.Tristate());
        return SetSubresourceRangeInitialLayoutImpl(&layouts_.initial.GetBigMap(), &initial_layout_state_map_.GetBigMap(),
                                                    &initial_layout_states_, &range_gen, cb_state, layout, &view_state);
    }
}

static VkImageLayout FindInMap(IndexType index, const ImageSubresourceLayoutMap::RangeMap& map) {
    auto found = map.find(index);
    VkImageLayout value = kInvalidLayout;
    if (found != map.end()) {
        value = found->second;
    }
    return value;
}
VkImageLayout ImageSubresourceLayoutMap::GetSubresourceLayout(const VkImageSubresource& subresource) const {
    IndexType index = encoder_.Encode(subresource);
    return FindInMap(index, layouts_.current);
}

VkImageLayout ImageSubresourceLayoutMap::GetSubresourceInitialLayout(const VkImageSubresource& subresource) const {
    IndexType index = encoder_.Encode(subresource);
    return FindInMap(index, layouts_.initial);
}

// Saves an encode to fetch both in the same call
ImageSubresourceLayoutMap::Layouts ImageSubresourceLayoutMap::GetSubresourceLayouts(const VkImageSubresource& subresource,
                                                                                    bool always_get_initial) const {
    IndexType index = encoder_.Encode(subresource);
    Layouts layouts{FindInMap(index, layouts_.current), kInvalidLayout};
    if (always_get_initial || (layouts.current_layout != kInvalidLayout)) {
        layouts.initial_layout = FindInMap(index, layouts_.initial);
    }
    return layouts;
}

const InitialLayoutState* ImageSubresourceLayoutMap::GetSubresourceInitialLayoutState(const IndexType index) const {
    const auto found = initial_layout_state_map_.find(index);
    if (found != initial_layout_state_map_.end()) {
        return found->second;
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
    using Arbiter = sparse_container::value_precedence;

    using sparse_container::range;
    // Must be from matching images for the reinterpret cast to be valid
    assert(CompatibilityKey() == other.CompatibilityKey());
    if (CompatibilityKey() != other.CompatibilityKey()) return false;

    bool updated = false;
    updated |= sparse_container::splice(&layouts_.initial, other.layouts_.initial, Arbiter::prefer_dest);
    updated |= sparse_container::splice(&layouts_.current, other.layouts_.current, Arbiter::prefer_source);
    // NOTE -- we are copying plain pointers from 'other' which owns them as unique_ptr.  This works because
    //         currently this function is only used to import from secondary command buffers, destruction of which
    //         invalidate the referencing primary command buffer, meaning that the dangling pointer will either be
    //         cleaned up in invalidation, on not referenced by validation code.
    sparse_container::splice(&initial_layout_state_map_, other.initial_layout_state_map_, Arbiter::prefer_dest);

    return updated;
}

// This is the same constant value range, subreource position advance logic as ForRange above, but suitable for use with
// an Increment operator.
void ImageSubresourceLayoutMap::ConstIterator::UpdateRangeAndValue() {
    bool not_found = true;
    while (range_gen_->non_empty() && not_found) {
        if (!parallel_it_->range.includes(current_index_)) {  // NOTE: empty ranges can't include anything
            parallel_it_.seek(current_index_);
        }
        if (parallel_it_->range.empty() && skip_invalid_) {
            // We're past the end of mapped data, and we aren't interested, so we're done
            // Set end condtion....
            ForceEndCondition();
        }
        // Search within the current range_ for a constant valid constant value interval
        // The while condition allows the parallel iterator to advance constant value ranges as needed.
        while (range_gen_->includes(current_index_) && not_found) {
            pos_.current_layout = kInvalidLayout;
            pos_.initial_layout = kInvalidLayout;
            constant_value_bound_ = range_gen_->end;
            // The generated range can validly traverse past the end of stored data
            if (!parallel_it_->range.empty()) {
                pos_.current_layout = sparse_container::evaluate(parallel_it_->pos_A, kInvalidLayout);
                if (pos_.current_layout == kInvalidLayout || always_get_initial_) {
                    pos_.initial_layout = sparse_container::evaluate(parallel_it_->pos_B, kInvalidLayout);
                }
                // The constant value bound marks the end of contiguous (w.r.t. range_gen_) indices with the same value, allowing
                // Increment (for example) to forgo this logic until finding a new range is needed.
                constant_value_bound_ = std::min(parallel_it_->range.end, constant_value_bound_);
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

                // Advance the parallel it if needed and possible
                // NOTE: We don't need to seek, as current_index_ can only be in the current or next constant value range
                if (!parallel_it_->range.empty() && !parallel_it_->range.includes(current_index_)) {
                    ++parallel_it_;
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
ImageSubresourceLayoutMap::ConstIterator::ConstIterator(const RangeMap& current, const RangeMap& initial, const Encoder& encoder,
                                                        const VkImageSubresourceRange& subres, bool skip_invalid,
                                                        bool always_get_initial)
    : range_gen_(encoder, subres),
      parallel_it_(current, initial, range_gen_->begin),
      skip_invalid_(skip_invalid),
      always_get_initial_(always_get_initial),
      pos_(),
      current_index_(range_gen_->begin),
      constant_value_bound_() {
    UpdateRangeAndValue();
}

}  // namespace image_layout_map
