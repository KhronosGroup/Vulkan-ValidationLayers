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

// Implements an ordered map of non-overlapping, non-empty ranges
class AccessMap {
    using ImplMap = std::map<AccessRange, AccessState>;

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

    void Clear() { impl_map_.clear(); }
    iterator Erase(const iterator &pos);
    void Erase(iterator first, iterator last);
    iterator LowerBound(const AccessRange &range);
    const_iterator LowerBound(const AccessRange &range) const;
    iterator Insert(const_iterator hint, const AccessRange &range, const AccessState &access_state);
    iterator Split(const iterator split_it, const index_type &index);
    size_t Size() const { return impl_map_.size(); }

  private:
    // No replacement insert
    std::pair<iterator, bool> Insert(const AccessRange &range, const AccessState &access_state);

  private:
    ImplMap impl_map_;
};

// Forward index iterator, tracking an index value and the apropos lower bound.
// Returns an index_type, lower_bound pair.  Supports ++,  offset, and seek affecting the index,
// lower bound updates as needed. As the index may specify a range for which no entry exist, dereferenced
// iterator includes an "valid" field, true IFF the lower_bound is not end() and contains [index, index +1)
//
// Must be explicitly invalidated when the underlying map is changed.
class CachedLowerBound {
  public:
    using iterator = AccessMap::iterator;
    using index_type = AccessMap::index_type;

    // Both sides of the return pair are const'd because we're returning references/pointers to the *internal* state
    // and we don't want and caller altering internal state.
    struct value_type {
        const index_type &index;
        const iterator &lower_bound;
        const bool &valid;
        value_type(const index_type &index, const iterator &lower_bound, bool &valid)
            : index(index), lower_bound(lower_bound), valid(valid) {}
    };

  private:
    AccessMap *map_;
    const iterator end_;
    value_type pos_;

    index_type index_;
    iterator lower_bound_;
    bool valid_;

    bool is_valid() const { return includes(index_); }

    // Allow reuse of a type with const semantics
    void set_value(const index_type &index, const iterator &it) {
        assert(it == lower_bound(index));
        index_ = index;
        lower_bound_ = it;
        valid_ = is_valid();
    }

    void update(const index_type &index) {
        assert(lower_bound_ == lower_bound(index));
        index_ = index;
        valid_ = is_valid();
    }

    iterator lower_bound(const index_type &index) { return map_->LowerBound(AccessRange(index, index + 1)); }
    bool at_end(const iterator &it) const { return it == end_; }

    bool is_lower_than(const index_type &index, const iterator &it) { return at_end(it) || (index < it->first.end); }

  public:
    // The cached lower bound knows the parent map, and thus can tell us this...
    bool at_end() const { return at_end(lower_bound_); }
    // includes(index) is a convenience function to test if the index would be in the currently cached lower bound
    bool includes(const index_type &index) const { return !at_end() && lower_bound_->first.includes(index); }

    // The return is const because we are sharing the internal state directly.
    const value_type &operator*() const { return pos_; }
    const value_type *operator->() const { return &pos_; }

    // Advance the cached location by 1
    CachedLowerBound &operator++() {
        const index_type next = index_ + 1;
        if (is_lower_than(next, lower_bound_)) {
            update(next);
        } else {
            // if we're past pos_->second, next *must* be the new lower bound.
            // NOTE: that next can't be past end, so lower_bound_ isn't end.
            auto next_it = lower_bound_;
            ++next_it;
            set_value(next, next_it);

            // However we *must* not be past next.
            assert(is_lower_than(next, next_it));
        }

        return *this;
    }

    // seek(index) updates lower_bound for index, updating lower_bound_ as needed.
    void Seek(index_type seek_to) {
        // Optimize seeking to  forward
        if (index_ == seek_to) {
            // seek to self is a NOOP.  To reset lower bound after a map change, use invalidate
        } else if (index_ < seek_to) {
            // See if the current or next ranges are the appropriate lower_bound... should be a common use case
            if (is_lower_than(seek_to, lower_bound_)) {
                // lower_bound_ is still the correct lower bound
                update(seek_to);
            } else {
                // Look to see if the next range is the new lower_bound (and we aren't at end)
                auto next_it = lower_bound_;
                ++next_it;
                if (is_lower_than(seek_to, next_it)) {
                    // next_it is the correct new lower bound
                    set_value(seek_to, next_it);
                } else {
                    // We don't know where we are...  and we aren't going to walk the tree looking for seek_to.
                    set_value(seek_to, lower_bound(seek_to));
                }
            }
        } else {
            // General case... this is += so we're not implmenting optimized negative offset logic
            set_value(seek_to, lower_bound(seek_to));
        }
    }

    // Advance the cached location by offset.
    void offset(const index_type &offset) {
        const index_type next = index_ + offset;
        Seek(next);
    }

    // invalidate() resets the the lower_bound_ cache, needed after insert/erase/overwrite/split operations
    // Pass index by value in case we are invalidating to index_ and set_value does a modify-in-place on index_
    void Invalidate(index_type index) { set_value(index, lower_bound(index)); }

    // For times when the application knows what it's done to the underlying map... (with assert in set_value)
    void Invalidate(const iterator &hint, index_type index) { set_value(index, hint); }

    CachedLowerBound(AccessMap &map, const index_type &index)
        : map_(&map),
          end_(map.end()),
          pos_(index_, lower_bound_, valid_),
          index_(index),
          lower_bound_(lower_bound(index)),
          valid_(is_valid()) {}
};

class CachedConstLowerBound {
  public:
    using iterator = AccessMap::const_iterator;
    using index_type = AccessMap::index_type;

    // Both sides of the return pair are const'd because we're returning references/pointers to the *internal* state
    // and we don't want and caller altering internal state.
    struct value_type {
        const index_type &index;
        const iterator &lower_bound;
        const bool &valid;
        value_type(const index_type &index_, const iterator &lower_bound_, bool &valid_)
            : index(index_), lower_bound(lower_bound_), valid(valid_) {}
    };

  private:
    const AccessMap *map_;
    const iterator end_;
    value_type pos_;

    index_type index_;
    iterator lower_bound_;
    bool valid_;

    bool is_valid() const { return includes(index_); }

    // Allow reuse of a type with const semantics
    void set_value(const index_type &index, const iterator &it) {
        assert(it == lower_bound(index));
        index_ = index;
        lower_bound_ = it;
        valid_ = is_valid();
    }

    void update(const index_type &index) {
        assert(lower_bound_ == lower_bound(index));
        index_ = index;
        valid_ = is_valid();
    }

    iterator lower_bound(const index_type &index) { return map_->LowerBound(AccessRange(index, index + 1)); }
    bool at_end(const iterator &it) const { return it == end_; }

    bool is_lower_than(const index_type &index, const iterator &it) { return at_end(it) || (index < it->first.end); }

  public:
    // The cached lower bound knows the parent map, and thus can tell us this...
    bool at_end() const { return at_end(lower_bound_); }
    // includes(index) is a convenience function to test if the index would be in the currently cached lower bound
    bool includes(const index_type &index) const { return !at_end() && lower_bound_->first.includes(index); }

    // The return is const because we are sharing the internal state directly.
    const value_type &operator*() const { return pos_; }
    const value_type *operator->() const { return &pos_; }

    // Advance the cached location by 1
    void operator++() {
        const index_type next = index_ + 1;
        if (is_lower_than(next, lower_bound_)) {
            update(next);
        } else {
            // if we're past pos_->second, next *must* be the new lower bound.
            // NOTE: that next can't be past end, so lower_bound_ isn't end.
            auto next_it = lower_bound_;
            ++next_it;
            set_value(next, next_it);

            // However we *must* not be past next.
            assert(is_lower_than(next, next_it));
        }
    }

    // seek(index) updates lower_bound for index, updating lower_bound_ as needed.
    void Seek(index_type seek_to) {
        // Optimize seeking to  forward
        if (index_ == seek_to) {
            // seek to self is a NOOP.  To reset lower bound after a map change, use invalidate
        } else if (index_ < seek_to) {
            // See if the current or next ranges are the appropriate lower_bound... should be a common use case
            if (is_lower_than(seek_to, lower_bound_)) {
                // lower_bound_ is still the correct lower bound
                update(seek_to);
            } else {
                // Look to see if the next range is the new lower_bound (and we aren't at end)
                auto next_it = lower_bound_;
                ++next_it;
                if (is_lower_than(seek_to, next_it)) {
                    // next_it is the correct new lower bound
                    set_value(seek_to, next_it);
                } else {
                    // We don't know where we are...  and we aren't going to walk the tree looking for seek_to.
                    set_value(seek_to, lower_bound(seek_to));
                }
            }
        } else {
            // General case... this is += so we're not implmenting optimized negative offset logic
            set_value(seek_to, lower_bound(seek_to));
        }
    }

    // Advance the cached location by offset.
    void offset(const index_type &offset) {
        const index_type next = index_ + offset;
        Seek(next);
    }

    CachedConstLowerBound(const AccessMap &map, const index_type &index)
        : map_(&map),
          end_(map.end()),
          pos_(index_, lower_bound_, valid_),
          index_(index),
          lower_bound_(lower_bound(index)),
          valid_(is_valid()) {}
};

// The offset in index type to the next change (the end of the current range, or the transition from invalid to valid).
// If invalid and at_end, returns 0.
template <typename TCachedLowerBound>
AccessMap::index_type distance_to_edge(const TCachedLowerBound &pos) {
    if (pos->valid) {
        // Distance to edge of
        return pos->lower_bound->first.end - pos->index;
    } else if (pos.at_end()) {
        return 0;
    } else {
        return pos->lower_bound->first.begin - pos->index;
    }
}

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
    assert((pos == end) || (pos == map.LowerBound(range)) || pos->first.strictly_less(range));

    if (range.empty()) {
        return pos;
    }
    if (pos == end) {
        // Only pass pos == end for range tail after last entry
        assert(end == map.LowerBound(range));
    } else if (pos->first.strictly_less(range)) {
        // pos isn't lower_bound for range (it's less than range), however, if range is monotonically increasing it's likely
        // the next entry in the map will be the lower bound.

        // If the new (pos + 1) *isn't* stricly_less and pos is,
        // (pos + 1) must be the lower_bound, otherwise we have to look for it O(log n)
        ++pos;
        if ((pos != end) && pos->first.strictly_less(range)) {
            pos = map.LowerBound(range);
        }
        assert(pos == map.LowerBound(range));
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
    auto pos = map.LowerBound(range);
    InfillUpdateRange(map, pos, range, ops);
}

// Traverse access maps over the the same range, but without assumptions of aligned ranges.
// ++ increments to the next point where one of the two maps changes range, giving a range
// over which the two maps do not transition ranges
class ParallelIterator {
  public:
    using index_type = AccessRange::index_type;
    using iterator_A = AccessMap::iterator;
    using lower_bound_A = CachedLowerBound;
    using iterator_B = AccessMap::const_iterator;
    using lower_bound_B = CachedConstLowerBound;

    // This is the value we'll always be returning, but the referenced object will be updated by the operations
    struct value_type {
        const AccessRange &range;
        const lower_bound_A &pos_A;
        const lower_bound_B &pos_B;
        value_type(const AccessRange &range, const lower_bound_A &pos_A, const lower_bound_B &pos_B)
            : range(range), pos_A(pos_A), pos_B(pos_B) {}
    };

    ParallelIterator(AccessMap &map_A, const AccessMap &map_B, index_type index)
        : pos_A_(map_A, index), pos_B_(map_B, index), range_(index, index + ComputeDelta()), pos_(range_, pos_A_, pos_B_) {}

    // Advance to the next spot one of the two maps changes
    void operator++();

    // Seeks to a specific index in both maps reseting range.
    // Cannot guarantee range.begin is on edge boundary, but range.end will be.
    // Lower bound objects assumed to invalidate their cached lower bounds on seek
    void Seek(index_type index);

    // Invalidates the lower_bound caches, reseting range.
    // Cannot guarantee range.begin is on edge boundary, but range.end will be
    void InvalidateA();
    void InvalidateA(const iterator_A &hint);

    const value_type &operator*() const { return pos_; }
    const value_type *operator->() const { return &pos_; }

  private:
    lower_bound_A pos_A_;
    lower_bound_B pos_B_;
    AccessRange range_;
    value_type pos_;

    index_type ComputeDelta();
};

}  // namespace syncval
