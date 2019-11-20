/* Copyright (c) 2019 The Khronos Group Inc.
 * Copyright (c) 2019 Valve Corporation
 * Copyright (c) 2019 LunarG, Inc.
 * Copyright (C) 2019 Google Inc.
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
#include "core_validation_types.h"
#include "chassis.h"
#include "descriptor_sets.h"

namespace image_layout_map {
// Storage for the static state
const ImageSubresourceLayoutMap::ConstIterator ImageSubresourceLayoutMap::end_iterator = ImageSubresourceLayoutMap::ConstIterator();

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
    : encoder_(image_state.full_range),
      image_state_(image_state),
      layouts_(),
      initial_layout_states_(),
      initial_layout_state_map_() {}

ImageSubresourceLayoutMap::ConstIterator ImageSubresourceLayoutMap::Begin(bool always_get_initial) const {
    return Find(image_state_.full_range, /* skip_invalid */ true, always_get_initial);
}
bool ImageSubresourceLayoutMap::SetSubresourceRangeLayout(const CMD_BUFFER_STATE& cb_state, const VkImageSubresourceRange& range,
                                                          VkImageLayout layout, VkImageLayout expected_layout) {
    bool updated = false;
    if (expected_layout == kInvalidLayout) {
        // Set the initial layout to the set layout as we had no other layout to reference
        expected_layout = layout;
    }
    if (!InRange(range)) return false;  // Don't even try to track bogus subreources

    InitialLayoutState* initial_state = nullptr;
    RangeGenerator range_gen(encoder_, range);
    // Empty range are the range tombstones
    for (; range_gen->non_empty(); ++range_gen) {
        // In order to track whether we've changed anything, we'll do this in a slightly convoluted way...
        // We'll traverse the range looking for values different from ours, then overwrite the range.
        auto lower = layouts_.current.lower_bound(*range_gen);
        bool all_same = false;
        bool contiguous = false;
        if (layouts_.current.is_contiguous(*range_gen, lower)) {
            // The whole range is set to a value, see if assigning to it will change anything...
            all_same = true;
            contiguous = true;
            for (auto pos = lower; (pos != layouts_.current.end()) && pos->first.intersects(*range_gen) && all_same; ++pos) {
                all_same = pos->second == layout;
            }
        }
        if (!all_same) {
            // We only need to try setting anything, if we changed any of the layout values above
            layouts_.current.overwrite_range(lower, std::make_pair(*range_gen, layout));
            updated = true;
            // We insert only into gaps (this is a write once semantic), and if the range
            // isn't already contiguous, i.e. has no gaps.
            if (!contiguous) {
                // Note: Can't use "lower" as the hint here, because it's the lower_bound of the *current* map only
                layouts_.initial.insert_range(std::make_pair(*range_gen, expected_layout), NoSplit());
                initial_state = UpdateInitialLayoutState(*range_gen, initial_state, cb_state, nullptr);
            }
        }
    }
    return updated;
}
bool ImageSubresourceLayoutMap::SetSubresourceRangeInitialLayout(const CMD_BUFFER_STATE& cb_state,
                                                                 const VkImageSubresourceRange& range, VkImageLayout layout,
                                                                 const IMAGE_VIEW_STATE* view_state) {
    bool updated = false;
    if (!InRange(range)) return false;  // Don't even try to track bogus subreources

    InitialLayoutState* initial_state = nullptr;
    RangeGenerator range_gen(encoder_, range);

    for (; range_gen->non_empty(); ++range_gen) {
        auto lower = layouts_.initial.lower_bound(*range_gen);
        bool update_needed = !layouts_.initial.is_contiguous(*range_gen, lower);
        if (update_needed) {
            layouts_.initial.insert_range(lower, std::make_pair(*range_gen, layout), NoSplit());
            initial_state = UpdateInitialLayoutState(*range_gen, initial_state, cb_state, view_state);
            updated = true;
        }
    }
    return updated;
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

const InitialLayoutState* ImageSubresourceLayoutMap::GetSubresourceInitialLayoutState(const VkImageSubresource subresource) const {
    if (!InRange(subresource)) return nullptr;
    const auto index = encoder_.Encode(subresource);
    const auto found = initial_layout_state_map_.find(index);
    if (found != initial_layout_state_map_.end()) {
        return found->second;
    }
    return nullptr;
}

// TODO: make sure this paranoia check is sufficient and not too much.
uintptr_t ImageSubresourceLayoutMap::CompatibilityKey() const {
    return (reinterpret_cast<const uintptr_t>(&image_state_) ^ encoder_.AspectMask());
}

bool ImageSubresourceLayoutMap::UpdateFrom(const ImageSubresourceLayoutMap& other) {
    using Arbiter = sparse_container::splice_precedence;

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
InitialLayoutState* ImageSubresourceLayoutMap::UpdateInitialLayoutState(const IndexRange& range, InitialLayoutState* initial_state,
                                                                        const CMD_BUFFER_STATE& cb_state,
                                                                        const IMAGE_VIEW_STATE* view_state) {
    if (!initial_state) {
        // Allocate on demand...  initial_layout_states_ holds ownership as a unique_ptr, while
        // each subresource has a non-owning copy of the plain pointer.
        initial_state = new InitialLayoutState(cb_state, view_state);
        initial_layout_states_.emplace_back(initial_state);
    }
    assert(initial_state);
    initial_layout_state_map_.insert_range(std::make_pair(range, initial_state), NoSplit());
    return initial_state;
}

// Loop over the given range calling the callback, primarily for
// validation checks.  By default the initial_value is only looked
// up if the set value isn't found.
bool ImageSubresourceLayoutMap::ForRange(const VkImageSubresourceRange& range, const Callback& callback, bool skip_invalid,
                                         bool always_get_initial) const {
    if (!InRange(range)) return false;  // Don't even try to process bogus subreources

    RangeGenerator range_gen(encoder_, range);
    SubresourceGenerator& subres_gen = range_gen.GetSubresourceGenerator();
    ParallelIterator<const RangeMap, const RangeMap> parallel_it(layouts_.current, layouts_.initial, range_gen->begin);

    bool keep_on = true;
    IndexType current;
    for (; range_gen->non_empty(); ++range_gen) {
        current = range_gen->begin;
        if (!parallel_it->range.includes(current)) {  // NOTE: empty ranges can't include anything
            parallel_it.seek(current);
        }
        if (parallel_it->range.empty() && skip_invalid) {
            // We're past the end of mapped data, and we aren't interested, so we're done
            break;
        }
        while (range_gen->includes(current)) {
            VkImageLayout layout = kInvalidLayout;
            VkImageLayout initial_layout = kInvalidLayout;
            IndexType constant_value_bound = range_gen->end;
            // The generated range can validly traverse past the end of stored data
            if (!parallel_it->range.empty()) {
                layout = sparse_container::evaluate(parallel_it->pos_A, kInvalidLayout);
                if (layout == kInvalidLayout || always_get_initial) {
                    initial_layout = sparse_container::evaluate(parallel_it->pos_B, kInvalidLayout);
                }
                constant_value_bound = std::min(parallel_it->range.end, constant_value_bound);
            }

            if (!skip_invalid || (layout != kInvalidLayout) || (initial_layout != kInvalidLayout)) {
                for (; current < constant_value_bound; current++, ++subres_gen) {
                    keep_on = callback(*subres_gen, layout, initial_layout);
                    if (!keep_on) return keep_on;  // False value from the callback aborts the range traversal
                }
            } else {
                subres_gen.Seek(constant_value_bound);  // Move the subresource to the end of the skipped range
                current = constant_value_bound;
            }
            // Advance the parallel it if needed and possible
            if (!parallel_it->range.empty() && !parallel_it->range.includes(current)) {
                ++parallel_it;
            }
        }
        // ++range_gen will update subres_gen.
    }
    return keep_on;
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
