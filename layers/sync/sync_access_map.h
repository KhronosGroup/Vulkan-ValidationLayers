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
#pragma once

#include "sync/sync_common.h"
#include "sync/sync_access_state.h"
#include "containers/range.h"
#include "containers/container_utils.h"

#include <algorithm>
#include <cassert>
#include <map>
#include <utility>

namespace syncval {

// There are two types of comparisons of AccessMap ranges:
//  a)  Two non-empty, non-overlapping ranges.
//      This happens when we compare two ranges from the access map, for example, during insert operation.
//      The "less" comparison that works here is: (range_a.end <= range_b.begin)
//
//  b)  Non-empty range vs empty range. The empty range here is a single point passed to LowerBound.
//      We compare this single point against non-empty ranges from the access map.
//      The "less" comparison from a) works for b) too, except one configuration, when point A coincides
//      with the beginning of non-empty range B. The comparison must return false here (e.g. not less).
//      That's because the lower bound for such point A is a range B itself. The comparision from a) has
//      to be extended with additional check: (range_a.begin < range_b.begin). This correctly handles
//      this special case and does not affect result of (range_a.end <= range_b.begin) in all other cases.
struct AccessMapCompare {
    bool operator()(const AccessRange &a, const AccessRange &b) const { return a.end <= b.begin && a.begin < b.begin; }
};

// Implements an ordered map of non-overlapping, non-empty ranges
class AccessMap {
    using ImplMap = std::map<AccessRange, AccessState, AccessMapCompare>;

  public:
    using index_type = ResourceAddress;
    using value_type = ImplMap::value_type;
    using iterator = ImplMap::iterator;
    using const_iterator = ImplMap::const_iterator;

  public:
    iterator begin() { return impl_map_.begin(); }
    const_iterator begin() const { return impl_map_.begin(); }
    iterator end() { return impl_map_.end(); }
    const_iterator end() const { return impl_map_.end(); }

    iterator LowerBound(ResourceAddress range_begin);
    const_iterator LowerBound(ResourceAddress range_begin) const;
    size_t Size() const { return impl_map_.size(); }

    void Clear() { impl_map_.clear(); }
    iterator Erase(const iterator &pos);
    void Erase(iterator first, iterator last);
    iterator Insert(const_iterator hint, const AccessRange &range, const AccessState &access_state);
    iterator Split(const iterator split_it, const index_type &index);

    AccessMap() : impl_map_(AccessMapCompare()) {}

  private:
    // No replacement insert
    std::pair<iterator, bool> Insert(const AccessRange &range, const AccessState &access_state);

  private:
    ImplMap impl_map_;
};

// The locator tracks an index value and its corresponding lower bound in the access map.
// Since the index may fall within a gap (no existing access map entry there),
// an "inside_lower_bound_range" flag is used to detect this.
// The locator must not be used after the underlying map is modified. Create a new locator instead.
template <typename TAccessMap>
class TAccessMapLocator {
  public:
    using index_type = AccessMap::index_type;
    using iterator = decltype(TAccessMap().begin());

    TAccessMapLocator(TAccessMap &map, index_type index);
    TAccessMapLocator(TAccessMap &map, index_type index, const iterator &index_lower_bound);

    // Set current location to provided value and update lower bound if necessary
    void Seek(index_type seek_to);

    // Distance from current location to the next change in access map.
    // The next change is either the end of the current range or the beginning
    // of the next range. Return 0 if lower_bound points to the end of access map.
    index_type DistanceToEdge() const;

  private:
    iterator LowerBoundForIndex(index_type index) const { return map_->LowerBound(index); }
    bool InsideLowerBoundRange() const { return lower_bound != map_->end() && lower_bound->first.includes(index); }
    bool TrySeekLocal(index_type seek_to);

  private:
    TAccessMap *map_;

  public:
    // Current location in the access map address space
    index_type index;

    // Lower bound for the current index.
    // That's either existing range in the access map or the end sentinel
    iterator lower_bound;

    // If the current location (index) is inside the lower bound range
    bool inside_lower_bound_range;
};

using AccessMapLocator = TAccessMapLocator<AccessMap>;
using ConstAccessMapLocator = TAccessMapLocator<const AccessMap>;

// Traverse access maps over the same range in parallel.
// NextRange advances to the next point where either map starts or finishes a range segment.
// Returns a range over which the two maps do not transition ranges.
class ParallelIterator {
  public:
    using index_type = AccessRange::index_type;
    using iterator = AccessMap::iterator;

    ParallelIterator(AccessMap &map_A, const AccessMap &map_B, index_type index)
        : map_A_(map_A), pos_A(map_A, index), pos_B(map_B, index), range(index, index + ComputeDelta()) {}

    // Must be called when destination map's current range is modified to update cached lower bound.
    // The lower bound corresponds to range.begin position.
    // No guarantee range.begin is on the edge boundary, but range.end is.
    void OnCurrentRangeModified(const iterator &new_lower_bound);

    // Seeks to a specific index in both maps after destination map was potentially modified.
    // No guarantee range.begin is on the edge boundary, but range.end is.
    void SeekAfterModification(index_type index);

    // Advance to the next spot where one of the maps changes
    void NextRange();

  private:
    AccessMap &map_A_;
    index_type ComputeDelta();

  public:
    AccessMapLocator pos_A;
    ConstAccessMapLocator pos_B;
    AccessRange range;
};

// Split a range into pieces bound by the intersection of the iterator's range and the supplied range
AccessMap::iterator Split(AccessMap::iterator in, AccessMap &map, const AccessRange &range);

void UpdateRangeValue(AccessMap &map, const AccessRange &range, const AccessState &access_state);

// Combines directly adjacent ranges with equal AccessState
void Consolidate(AccessMap &map);

// Apply an operation over a range map, infilling where content is absent, updating where content is present.
// The passed pos must *either* be strictly less than range or *is* lower_bound (which may be end)
// Trims to range boundaries.
// infill op doesn't have to alter map, but mustn't invalidate iterators passed to it. (i.e. no erasure)
// infill data (default mapped value or other initial value) is contained with ops.
// update allows existing ranges to be updated (merged, whatever) based on data contained in ops.  All iterators
// passed to update are already trimmed to fit within range.
template <typename InfillUpdateOps>
AccessMap::iterator InfillUpdateRange(AccessMap &map, AccessMap::iterator pos, const AccessRange &range,
                                      const InfillUpdateOps &ops) {
    const auto end = map.end();
    assert((pos == end) || (pos == map.LowerBound(range.begin)) || pos->first.strictly_less(range));

    if (range.empty()) {
        return pos;
    }
    if (pos == end) {
        // Only pass pos == end for range tail after last entry
        assert(end == map.LowerBound(range.begin));
    } else if (pos->first.strictly_less(range)) {
        // pos isn't lower_bound for range (it's less than range), however, if range is monotonically increasing it's likely
        // the next entry in the map will be the lower bound.

        // If the new (pos + 1) *isn't* stricly_less and pos is,
        // (pos + 1) must be the lower_bound, otherwise we have to look for it O(log n)
        ++pos;
        if ((pos != end) && pos->first.strictly_less(range)) {
            pos = map.LowerBound(range.begin);
        }
        assert(pos == map.LowerBound(range.begin));
    }

    if ((pos != end) && (range.begin > pos->first.begin)) {
        // lower bound starts before the range, trim and advance
        pos = map.Split(pos, range.begin);
        ++pos;
    }

    AccessMap::index_type current_begin = range.begin;
    while (pos != end && current_begin < range.end) {
        if (current_begin < pos->first.begin) {
            // The current_begin is pointing to the beginning of a gap to infill (we supply pos for "insert in front of" calls)
            ops.infill(map, pos, AccessRange(current_begin, std::min(range.end, pos->first.begin)));
            // Advance current begin, but *not* pos as it's the next valid value. (infill shall not invalidate pos)
            current_begin = pos->first.begin;
        } else {
            // The current_begin is pointing to the next existing value to update
            assert(current_begin == pos->first.begin);

            // We need to run the update operation on the valid portion of the current value.
            // If this entry overlaps end-of-range we need to trim it to the range
            if (pos->first.end > range.end) {
                pos = map.Split(pos, range.end);
            }

            // We have a valid fully contained range, apply update op
            ops.update(pos);

            // Advance the current location and map entry
            current_begin = pos->first.end;
            ++pos;
        }
    }

    // Fill to the end as needed
    if (current_begin < range.end) {
        ops.infill(map, pos, AccessRange(current_begin, range.end));
    }
    return pos;
}

template <typename InfillUpdateOps>
void InfillUpdateRange(AccessMap &map, const AccessRange &range, const InfillUpdateOps &ops) {
    if (range.empty()) {
        return;
    }
    auto pos = map.LowerBound(range.begin);
    InfillUpdateRange(map, pos, range, ops);
}

}  // namespace syncval
