/* Copyright (c) 2025 The Khronos Group Inc.
 * Copyright (c) 2025 Valve Corporation
 * Copyright (c) 2025 LunarG, Inc.
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
 */

#include "sync_access_map.h"

namespace syncval {

AccessMap::iterator AccessMap::Erase(const iterator &pos) {
    assert(pos != end());
    return impl_map_.erase(pos);
}

void AccessMap::Erase(iterator first, iterator last) {
    auto current = first;
    while (current != last) {
        assert(current != end());
        current = impl_map_.erase(current);
    }
}

AccessMap::iterator AccessMap::LowerBound(const AccessRange &range) {
    assert(range.valid());
    if (range.valid()) {
        // ImplMap doesn't give us what want with a direct query, it will give us the first entry contained (if any) in key,
        // not the first entry intersecting key, so, first look for the the first entry that starts at or after key.begin
        // with the operator > in range, we can safely use an empty range for comparison
        auto lower = impl_map_.lower_bound(AccessRange(range.begin, range.begin));

        // If there is a preceding entry it's possible that begin is included, as all we know is that lower.begin >= key.begin
        // or lower is at end
        if (lower != begin()) {
            auto prev = lower;
            --prev;
            // If the previous entry includes begin (and we know key.begin > prev.begin) then prev is actually lower
            if (range.begin < prev->first.end) {
                lower = prev;
            }
        }
        return lower;
    }
    // Range is ill-formed
    return end();  // Point safely to nothing
}

AccessMap::const_iterator AccessMap::LowerBound(const AccessRange &range) const {
    assert(range.valid());
    if (range.valid()) {
        // ImplMap doesn't give us what want with a direct query, it will give us the first entry contained (if any) in key,
        // not the first entry intersecting key, so, first look for the the first entry that starts at or after key.begin
        // with the operator > in range, we can safely use an empty range for comparison
        auto lower = impl_map_.lower_bound(AccessRange(range.begin, range.begin));

        // If there is a preceding entry it's possible that begin is included, as all we know is that lower.begin >= key.begin
        // or lower is at end
        if (lower != begin()) {
            auto prev = lower;
            --prev;
            // If the previous entry includes begin (and we know key.begin > prev.begin) then prev is actually lower
            if (range.begin < prev->first.end) {
                lower = prev;
            }
        }
        return lower;
    }
    // Range is ill-formed
    return end();  // Point safely to nothing
}

AccessMap::iterator AccessMap::Insert(const_iterator hint, const AccessRange &range, const AccessState &access_state) {
    bool hint_open;
    const_iterator impl_next = hint;
    if (impl_map_.empty()) {
        hint_open = true;
    } else if (impl_next == impl_map_.cbegin()) {
        hint_open = range.strictly_less(impl_next->first);
    } else if (impl_next == impl_map_.cend()) {
        auto impl_prev = impl_next;
        --impl_prev;
        hint_open = range.strictly_greater(impl_prev->first);
    } else {
        auto impl_prev = impl_next;
        --impl_prev;
        hint_open = range.strictly_greater(impl_prev->first) && range.strictly_less(impl_next->first);
    }

    if (!hint_open) {
        // Hint was unhelpful, fall back to the non-hinted version
        auto plain_insert = Insert(range, access_state);
        return plain_insert.first;
    }

    auto impl_insert = impl_map_.insert(impl_next, {range, access_state});
    return iterator(impl_insert);
}

std::pair<AccessMap::iterator, bool> AccessMap::Insert(const AccessRange &range, const AccessState &access_state) {
    if (!range.non_empty()) {
        return std::make_pair(end(), false);
    }
    // Look for range conflicts (and an insertion point, which makes the lower_bound *not* wasted work)
    // we don't have to check upper if just check that lower doesn't intersect (which it would if lower != upper)
    auto lower = LowerBound(range);
    if (lower == end() || !lower->first.intersects(range)) {
        // range is not even partially overlapped, and lower is strictly > than key
        return {impl_map_.emplace_hint(lower, range, access_state), true};
    }
    // We don't replace
    return {lower, false};
}

AccessMap::iterator AccessMap::Split(const iterator split_it, const index_type &index) {
    const auto range = split_it->first;

    if (!range.includes(index)) {
        return split_it;  // If we don't have a valid split point, just return the iterator
    }

    AccessRange lower_range(range.begin, index);

    if (lower_range.empty()) {
        // This is a noop, we're keeping the upper half which is the same as split_it
        return split_it;
    }

    // Save the contents and erase
    auto value = split_it->second;
    auto next_it = impl_map_.erase(split_it);

    AccessRange upper_range(index, range.end);
    assert(!upper_range.empty());  // Upper range cannot be empty

    // Copy value to the upper range
    // NOTE: we insert from upper to lower because that's what emplace_hint can do in constant time
    assert(impl_map_.find(upper_range) == impl_map_.end());
    next_it = impl_map_.emplace_hint(next_it, std::make_pair(upper_range, value));

    // Move value to the lower range (we can move since the upper range already got a copy of value)
    assert(impl_map_.find(lower_range) == impl_map_.end());
    next_it = impl_map_.emplace_hint(next_it, std::make_pair(lower_range, std::move(value)));

    // Iterator to the beginning of the lower range
    return next_it;
}

AccessMap::iterator Split(AccessMap::iterator in, AccessMap &map, const AccessRange &range) {
    assert(in != map.end());  // Not designed for use with invalid iterators...
    const AccessRange in_range = in->first;
    const AccessRange split_range = in_range & range;

    if (split_range.empty()) {
        return map.end();
    }
    auto pos = in;
    if (split_range.begin != in_range.begin) {
        pos = map.Split(pos, split_range.begin);
        ++pos;
    }
    if (split_range.end != in_range.end) {
        pos = map.Split(pos, split_range.end);
    }
    return pos;
}

void UpdateRangeValue(AccessMap &map, const AccessRange &range, const AccessState &access_state) {
    AccessMapLocator pos(map, range.begin);
    while (range.includes(pos.index)) {
        if (!pos.inside_lower_bound_range) {
            // Fill in the leading space (or in the case of pos at end the trailing space)
            const auto start = pos.index;
            auto it = pos.lower_bound;
            const auto limit = (it != map.end()) ? std::min(it->first.begin, range.end) : range.end;
            map.Insert(it, AccessRange(start, limit), access_state);
            // We inserted before pos->lower_bound, so pos->lower_bound isn't invalid, but the associated index *is* and seek
            // will fix this (and move the state to valid)
            pos.Seek(limit);
        }
        // Note that after the "fill" operation pos may have become valid so we check again
        if (pos.inside_lower_bound_range) {
            // "prefer_dest" means don't overwrite existing values, so we'll skip this interval.
            // Point just past the end of this section,  if it's within the given range, it will get filled next iteration
            // ++pos could move us past the end of range (which would exit the loop) so we don't use it.
            pos.Seek(pos.lower_bound->first.end);
        }
    }
}

void Consolidate(AccessMap &map) {
    using It = AccessMap::iterator;

    It current = map.begin();
    const It map_end = map.end();

    // To be included in a merge range there must be no gap in the AccessRange space, and the mapped_type values must match
    auto can_merge = [](const It &last, const It &cur) {
        return cur->first.begin == last->first.end && cur->second == last->second;
    };

    while (current != map_end) {
        // Establish a trival merge range at the current location, advancing current. Merge range is inclusive of merge_last
        const It merge_first = current;
        It merge_last = current;
        ++current;

        // Expand the merge range as much as possible
        while (current != map_end && can_merge(merge_last, current)) {
            merge_last = current;
            ++current;
        }

        // Current isn't in the active merge range. If there is a non-trivial merge range, we resolve it here.
        if (merge_first != merge_last) {
            // IFF there is more than one range in (merge_first, merge_last)  <- again noting the *inclusive* last
            // Create a new Val spanning (first, last), substitute it for the multiple entries.

            const AccessRange merged_range(merge_first->first.begin, merge_last->first.end);
            AccessState access = merge_last->second;

            // Note that current points to merge_last + 1, and is valid even if at map_end for these operations
            map.Erase(merge_first, current);

            map.Insert(current, merged_range, std::move(access));
        }
    }
}

template <typename TAccessMap>
TAccessMapLocator<TAccessMap>::TAccessMapLocator(TAccessMap &map, index_type index) : map_(&map), index(index) {
    lower_bound = LowerBoundForIndex(index);
    inside_lower_bound_range = InsideLowerBoundRange();
}

template <typename TAccessMap>
TAccessMapLocator<TAccessMap>::TAccessMapLocator(TAccessMap &map, index_type index, const iterator &index_lower_bound)
    : map_(&map), index(index), lower_bound(index_lower_bound) {
    assert(LowerBoundForIndex(index) == index_lower_bound);
    inside_lower_bound_range = InsideLowerBoundRange();
}

template <typename TAccessMap>
void TAccessMapLocator<TAccessMap>::Seek(index_type seek_to) {
    if (TrySeekLocal(seek_to)) {
        return;
    }
    index = seek_to;
    lower_bound = LowerBoundForIndex(seek_to);  // Expensive part
    inside_lower_bound_range = InsideLowerBoundRange();
}

template <typename TAccessMap>
bool TAccessMapLocator<TAccessMap>::TrySeekLocal(index_type seek_to) {
    auto is_lower_than = [this](AccessMap::index_type index, const auto &it) { return it == map_->end() || index < it->first.end; };

    // Already here
    if (index == seek_to) {
        return true;
    }
    // The optimization is only for forward movement
    if (index < seek_to) {
        // Check if the current range is still a valid lower bound
        if (is_lower_than(seek_to, lower_bound)) {
            assert(lower_bound == LowerBoundForIndex(seek_to));
            index = seek_to;
            inside_lower_bound_range = InsideLowerBoundRange();
            return true;
        }
        // Check if the next range is a valid lower bound
        auto next_it = lower_bound;
        ++next_it;
        if (is_lower_than(seek_to, next_it)) {
            assert(next_it == LowerBoundForIndex(seek_to));
            index = seek_to;
            lower_bound = next_it;
            inside_lower_bound_range = InsideLowerBoundRange();
            return true;
        }
    }
    return false;  // Need to re-search lower bound
}

template <typename TAccessMap>
AccessMap::index_type TAccessMapLocator<TAccessMap>::DistanceToEdge() const {
    if (lower_bound == map_->end()) {
        return 0;
    }
    const index_type edge = inside_lower_bound_range ? lower_bound->first.end : lower_bound->first.begin;
    return edge - index;
}

// Explicit instantiation of const and non-const locators
template class TAccessMapLocator<AccessMap>;
template class TAccessMapLocator<const AccessMap>;

void ParallelIterator::operator++() {
    const index_type start = range_.end;         // we computed this the last time we set range
    const index_type delta = range_.distance();  // we computed this the last time we set range
    assert(delta != 0);                          // Trying to increment past end

    pos_A_.Seek(pos_A_.index + delta);
    pos_B_.Seek(pos_B_.index + delta);

    range_ = AccessRange(start, start + ComputeDelta());  // find the next boundary (must be after offset)
    assert(pos_A_.index == start);
    assert(pos_B_.index == start);
}

// Seeks to a specific index in both maps reseting range.  Cannot guarantee range.begin is on edge boundary,
/// but range.end will be.  Lower bound objects assumed to invalidate their cached lower bounds on seek.
void ParallelIterator::Seek(index_type index) {
    pos_A_.Seek(index);
    pos_B_.Seek(index);
    range_ = AccessRange(index, index + ComputeDelta());
    assert(pos_A_.index == index);
    assert(pos_B_.index == index);
}

void ParallelIterator::InvalidateA() {
    const index_type index = range_.begin;
    pos_A_ = AccessMapLocator(map_A_, index);
    range_ = AccessRange(index, index + ComputeDelta());
}

void ParallelIterator::InvalidateA(const iterator &hint) {
    const index_type index = range_.begin;
    pos_A_ = AccessMapLocator(map_A_, index, hint);
    range_ = AccessRange(index, index + ComputeDelta());
}

ParallelIterator::index_type ParallelIterator::ComputeDelta() {
    const index_type delta_A = pos_A_.DistanceToEdge();
    const index_type delta_B = pos_B_.DistanceToEdge();

    // If either A or B are at end, there distance is *0*, so shouldn't be considered in the "distance to edge"
    if (delta_A == 0) {  // lower A is at end
        return delta_B;
    } else if (delta_B == 0) {  // lower B is at end
        return delta_A;
    } else {
        // Use the nearest edge, s.t. over this range A and B are both constant
        return std::min(delta_A, delta_B);
    }
}

}  // namespace syncval
