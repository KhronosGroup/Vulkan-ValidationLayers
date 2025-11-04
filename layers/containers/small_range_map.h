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

#pragma once

#include "containers/range.h"
#include <array>
#include <limits>
#include <utility>

namespace sparse_container {

// The an array based small ordered map for range keys for use as the range map "ImplMap" as an alternate to std::map
//
// Assumes RangeKey::index_type is unsigned (TBD is it useful to generalize to unsigned?)
// Assumes RangeKey implements begin, end, < and (TBD) from template range above
template <typename Key, typename T, typename RangeKey = vvl::range<Key>, size_t N = 64, typename SmallIndex = uint8_t>
class small_range_map {
    using SmallRange = vvl::range<SmallIndex>;

  public:
    using mapped_type = T;
    using key_type = RangeKey;
    using value_type = std::pair<const key_type, mapped_type>;
    using index_type = typename key_type::index_type;

    using size_type = SmallIndex;
    template <typename Map_, typename Value_>
    struct IteratorImpl {
      public:
        using Map = Map_;
        using Value = Value_;
        friend Map;
        Value *operator->() const { return map_->get_value(pos_); }
        Value &operator*() const { return *(map_->get_value(pos_)); }
        IteratorImpl &operator++() {
            pos_ = map_->next_range(pos_);
            return *this;
        }
        IteratorImpl &operator--() {
            pos_ = map_->prev_range(pos_);
            return *this;
        }
        IteratorImpl &operator=(const IteratorImpl &other) {
            map_ = other.map_;
            pos_ = other.pos_;
            return *this;
        }
        bool operator==(const IteratorImpl &other) const {
            if (at_end() && other.at_end()) {
                return true;  // all ends are equal
            }
            return (map_ == other.map_) && (pos_ == other.pos_);
        }
        bool operator!=(const IteratorImpl &other) const { return !(*this == other); }

        // At end()
        IteratorImpl() : map_(nullptr), pos_(N) {}
        IteratorImpl(const IteratorImpl &other) : map_(other.map_), pos_(other.pos_) {}

        // Raw getters to allow for const_iterator conversion below
        Map *get_map() const { return map_; }
        SmallIndex get_pos() const { return pos_; }

        bool at_end() const { return (map_ == nullptr) || (pos_ >= map_->get_limit()); }

      protected:
        IteratorImpl(Map *map, SmallIndex pos) : map_(map), pos_(pos) {}

      private:
        Map *map_;
        SmallIndex pos_;  // the begin of the current small_range
    };
    using iterator = IteratorImpl<small_range_map, value_type>;

    // The const iterator must be derived to allow the conversion from iterator, which iterator doesn't support
    class const_iterator : public IteratorImpl<const small_range_map, const value_type> {
        using Base = IteratorImpl<const small_range_map, const value_type>;
        friend small_range_map;

      public:
        const_iterator(const iterator &it) : Base(it.get_map(), it.get_pos()) {}
        const_iterator() : Base() {}

      private:
        const_iterator(const small_range_map *map, SmallIndex pos) : Base(map, pos) {}
    };

    iterator begin() {
        // Either ranges of 0 is valid and begin is 0 and begin *or* it's invalid an points to the first valid range (or end)
        return iterator(this, ranges_[0].begin);
    }
    const_iterator cbegin() const { return const_iterator(this, ranges_[0].begin); }
    const_iterator begin() const { return cbegin(); }
    iterator end() { return iterator(); }
    const_iterator cend() const { return const_iterator(); }
    const_iterator end() const { return cend(); }

    void clear() {
        const SmallRange clear_range(limit_, 0);
        for (SmallIndex i = 0; i < limit_; ++i) {
            auto &range = ranges_[i];
            if (range.begin == i) {
                // Clean up the backing store
                destruct_value(i);
            }
            range = clear_range;
        }
        size_ = 0;
    }

    // Find entry with an exact key match (uncommon use case)
    iterator find(const key_type &key) {
        RANGE_ASSERT(in_bounds(key));
        if (key.begin < limit_) {
            const SmallIndex small_begin = static_cast<SmallIndex>(key.begin);
            const auto &range = ranges_[small_begin];
            if (range.begin == small_begin) {
                const auto small_end = static_cast<SmallIndex>(key.end);
                if (range.end == small_end) return iterator(this, small_begin);
            }
        }
        return end();
    }
    const_iterator find(const key_type &key) const {
        RANGE_ASSERT(in_bounds(key));
        if (key.begin < limit_) {
            const SmallIndex small_begin = static_cast<SmallIndex>(key.begin);
            const auto &range = ranges_[small_begin];
            if (range.begin == small_begin) {
                const auto small_end = static_cast<SmallIndex>(key.end);
                if (range.end == small_end) return const_iterator(this, small_begin);
            }
        }
        return end();
    }

    iterator find(const index_type &index) {
        if (index < get_limit()) {
            const SmallIndex small_index = static_cast<SmallIndex>(index);
            const auto &range = ranges_[small_index];
            if (range.valid()) {
                return iterator(this, range.begin);
            }
        }
        return end();
    }

    const_iterator find(const index_type &index) const {
        if (index < get_limit()) {
            const SmallIndex small_index = static_cast<SmallIndex>(index);
            const auto &range = ranges_[small_index];
            if (range.valid()) {
                return const_iterator(this, range.begin);
            }
        }
        return end();
    }

    size_type size() const { return size_; }
    bool empty() const { return 0 == size_; }

    iterator erase(const_iterator pos) {
        RANGE_ASSERT(pos.map_ == this);
        return erase_impl(pos.get_pos());
    }

    iterator erase(iterator pos) {
        RANGE_ASSERT(pos.map_ == this);
        return erase_impl(pos.get_pos());
    }

    // Must be called with rvalue or lvalue of value_type
    template <typename Value>
    iterator emplace(Value &&value) {
        const auto &key = value.first;
        RANGE_ASSERT(in_bounds(key));
        if (key.begin >= limit_) return end();  // Invalid key (end is checked in "is_open")
        const SmallRange range(static_cast<SmallIndex>(key.begin), static_cast<SmallIndex>(key.end));
        if (is_open(key)) {
            // This needs to be the fast path, but I don't see how we can avoid the sanity checks above
            for (auto i = range.begin; i < range.end; ++i) {
                ranges_[i] = range;
            }
            // Update the next information for the previous unused slots (as stored in begin invalidly)
            auto prev = range.begin;
            while (prev > 0) {
                --prev;
                if (ranges_[prev].valid()) break;
                ranges_[prev].begin = range.begin;
            }
            // Placement new into the storage interpreted as Value
            construct_value(range.begin, value_type(std::forward<Value>(value)));
            auto next = range.end;
            // update the previous range information for the next unsed slots (as stored in end invalidly)
            while (next < limit_) {
                // End is exclusive... increment *after* update
                if (ranges_[next].valid()) break;
                ranges_[next].end = range.end;
                ++next;
            }
            return iterator(this, range.begin);
        } else {
            // Can't insert into occupied ranges.
            // if ranges_[key.begin] is valid then this is the collision (starting at .begin
            // if it's invalid .begin points to the overlapping entry from is_open (or end if key was out of range)
            return iterator(this, ranges_[range.begin].begin);
        }
    }

    // As hint is going to be ignored, make it as lightweight as possible, by reference and no conversion construction
    template <typename Value>
    iterator emplace_hint([[maybe_unused]] const const_iterator &hint, Value &&value) {
        // We have direct access so we can drop the hint
        return emplace(std::forward<Value>(value));
    }

    template <typename Value>
    iterator emplace_hint([[maybe_unused]] const iterator &hint, Value &&value) {
        // We have direct access so we can drop the hint
        return emplace(std::forward<Value>(value));
    }

    // Again, hint is going to be ignored, make it as lightweight as possible, by reference and no conversion construction
    iterator insert([[maybe_unused]] const const_iterator &hint, const value_type &value) { return emplace(value); }
    iterator insert([[maybe_unused]] const iterator &hint, const value_type &value) { return emplace(value); }

    std::pair<iterator, bool> insert(const value_type &value) {
        const auto &key = value.first;
        RANGE_ASSERT(in_bounds(key));
        if (key.begin >= limit_) return std::make_pair(end(), false);  // Invalid key, not inserted.
        if (is_open(key)) {
            return std::make_pair(emplace(value), true);
        }
        // If invalid we point to the subsequent range that collided, if valid begin is the start of the valid range
        const auto &collision_begin = ranges_[key.begin].begin;
        RANGE_ASSERT(ranges_[collision_begin].valid());
        return std::make_pair(iterator(this, collision_begin), false);
    }

    iterator split(const iterator whole_it, const index_type &index) {
        const auto &key = whole_it->first;

        if (!key.includes(index)) {
            return whole_it;  // If we don't have a valid split point, just return the iterator
        }

        const auto small_key = make_small_range(key);
        key_type lower_key(key.begin, index);
        if (lower_key.empty()) {
            return whole_it;  // this is a noop we're keeping the upper half which is the same as whole_it;
        }

        // Upper range cannot be empty (because the split point is included)
        const auto small_lower_key = make_small_range(lower_key);
        const SmallRange small_upper_key{small_lower_key.end, small_key.end};

        // Note: create the upper section before the lower, as processing the lower may erase it
        RANGE_ASSERT(!small_upper_key.empty());
        const key_type upper_key{lower_key.end, key.end};
        construct_value(small_upper_key.begin, std::make_pair(upper_key, get_value(small_key.begin)->second));

        for (auto i = small_upper_key.begin; i < small_upper_key.end; ++i) {
            ranges_[i] = small_upper_key;
        }

        resize_value(small_key.begin, lower_key.end);
        rerange_end(small_lower_key.begin, small_lower_key.end, small_lower_key.end);
        SmallIndex split_index = small_lower_key.begin;
        return iterator(this, split_index);
    }

    // For the value.first range rewrite the range...
    template <typename Value>
    iterator overwrite_range(Value &&value) {
        const auto &key = value.first;

        // Small map only has a restricted range supported
        RANGE_ASSERT(in_bounds(key));
        if (key.end > get_limit()) {
            return end();
        }

        const auto small_key = make_small_range(key);
        clear_out_range(small_key, /* valid clear range */ true);
        construct_value(small_key.begin, std::forward<Value>(value));
        return iterator(this, small_key.begin);
    }

    // We don't need a hint...
    template <typename Value>
    iterator overwrite_range([[maybe_unused]] const iterator &hint, Value &&value) {
        return overwrite_range(std::forward<Value>(value));
    }

    // For the range erase all contents within range, trimming any overlapping ranges
    iterator erase_range(const key_type &range) {
        // Small map only has a restricted range supported
        RANGE_ASSERT(in_bounds(range));
        if (range.end > get_limit() || range.empty()) {
            return end();
        }
        const auto empty = clear_out_range(make_small_range(range), /* valid clear range */ false);
        return iterator(this, empty.end);
    }

    template <typename Iterator>
    iterator erase(const Iterator &first, const Iterator &last) {
        RANGE_ASSERT(this == first.map_);
        RANGE_ASSERT(this == last.map_);
        auto first_pos = !first.at_end() ? first.pos_ : limit_;
        auto last_pos = !last.at_end() ? last.pos_ : limit_;
        RANGE_ASSERT(first_pos <= last_pos);
        const SmallRange clear_me(first_pos, last_pos);
        if (!clear_me.empty()) {
            const SmallRange empty_range(find_empty_left(clear_me), last_pos);
            clear_and_set_range(empty_range.begin, empty_range.end, make_invalid_range(empty_range));
        }
        return iterator(this, last_pos);
    }

    iterator lower_bound(const key_type &key) { return iterator(this, lower_bound_impl(this, key)); }
    const_iterator lower_bound(const key_type &key) const { return const_iterator(this, lower_bound_impl(this, key)); }

    iterator upper_bound(const key_type &key) { return iterator(this, upper_bound_impl(this, key)); }
    const_iterator upper_bound(const key_type &key) const { return const_iterator(this, upper_bound_impl(this, key)); }

    small_range_map(index_type limit = N) : size_(0), limit_(static_cast<SmallIndex>(limit)) {
        RANGE_ASSERT(limit <= std::numeric_limits<SmallIndex>::max());
        init_range();
    }

    // Only valid for empty maps
    void set_limit(size_t limit) {
        RANGE_ASSERT(size_ == 0);
        RANGE_ASSERT(limit <= std::numeric_limits<SmallIndex>::max());
        limit_ = static_cast<SmallIndex>(limit);
        init_range();
    }
    inline index_type get_limit() const { return static_cast<index_type>(limit_); }

  private:
    inline bool in_bounds(index_type index) const { return index < get_limit(); }
    inline bool in_bounds(const RangeKey &key) const { return key.begin < get_limit() && key.end <= get_limit(); }

    inline SmallRange make_small_range(const RangeKey &key) const {
        RANGE_ASSERT(in_bounds(key));
        return SmallRange(static_cast<SmallIndex>(key.begin), static_cast<SmallIndex>(key.end));
    }

    inline SmallRange make_invalid_range(const SmallRange &key) const { return SmallRange(key.end, key.begin); }

    bool is_open(const key_type &key) const {
        // Remebering that invalid range.begin is the beginning the next used range.
        const auto small_key = make_small_range(key);
        const auto &range = ranges_[small_key.begin];
        return !range.valid() && small_key.end <= range.begin;
    }
    // Only call this with a valid beginning index
    iterator erase_impl(SmallIndex erase_index) {
        RANGE_ASSERT(erase_index == ranges_[erase_index].begin);
        auto &range = ranges_[erase_index];
        destruct_value(erase_index);
        // Need to update the ranges to accommodate the erasure
        SmallIndex prev = 0;  // This is correct for the case erase_index is 0....
        if (erase_index != 0) {
            prev = prev_range(erase_index);
            // This works if prev is valid or invalid, because the invalid end will be either 0 (and correct) or the end of the
            // prior valid range and the valid end will be the end of the previous range (and thus correct)
            prev = ranges_[prev].end;
        }
        auto next = next_range(erase_index);
        // We have to be careful of next == limit_...
        if (next < limit_) {
            next = ranges_[next].begin;
        }
        // Rewrite both adjoining and newly empty entries
        SmallRange infill(next, prev);
        for (auto i = prev; i < next; ++i) {
            ranges_[i] = infill;
        }
        return iterator(this, next);
    }
    // This implements the "range lower bound logic" directly on the ranges
    template <typename Map>
    static SmallIndex lower_bound_impl(Map *const that, const key_type &key) {
        if (!that->in_bounds(key.begin)) return that->limit_;
        // If range is invalid, then begin points to the next valid (or end) with must be the lower bound
        // If range is valid, the begin points to a the lowest range that interects key
        const auto lb = that->ranges_[static_cast<SmallIndex>(key.begin)].begin;
        return lb;
    }

    template <typename Map>
    static SmallIndex upper_bound_impl(Map *that, const key_type &key) {
        const auto limit = that->get_limit();
        if (key.end >= limit) return that->limit_;  //  at end
        const auto &end_range = that->ranges_[key.end];
        // If range is invalid, then begin points to the next valid (or end) with must be the upper bound (key < range because
        auto ub = end_range.begin;
        // If range is valid, the begin points to a range that may interects key, which is be upper iff range.begin == key.end
        if (end_range.valid() && (key.end > end_range.begin)) {
            // the ub candidate *intersects* the key, so we have to go to the next range.
            ub = that->next_range(end_range.begin);
        }
        return ub;
    }

    // This is and inclusive "inuse", the entry itself
    SmallIndex find_inuse_right(const SmallRange &range) const {
        if (range.end >= limit_) return limit_;
        // if range is valid, begin is the first use (== range.end), else it's the first used after the invalid range
        return ranges_[range.end].begin;
    }
    // This is an exclusive "inuse", the end of the previous range
    SmallIndex find_inuse_left(const SmallRange &range) const {
        if (range.begin == 0) return 0;
        // if range is valid, end is the end of the first use (== range.begin), else it's the end of the in use range before the
        // invalid range
        return ranges_[range.begin - 1].end;
    }
    SmallRange find_empty(const SmallRange &range) const { return SmallRange(find_inuse_left(range), find_inuse_right(range)); }

    // Clear out the given range, trimming as needed.  The clear_range can be set as valid or invalid
    SmallRange clear_out_range(const SmallRange &clear_range, bool valid_clear_range) {
        // By copy to avoid reranging side affect
        auto first_range = ranges_[clear_range.begin];

        // fast path for matching ranges...
        if (first_range == clear_range) {
            // clobber the existing value
            destruct_value(clear_range.begin);
            if (valid_clear_range) {
                return clear_range;  // This is the overwrite fastpath for matching range
            } else {
                const auto empty_range = find_empty(clear_range);
                rerange(empty_range, make_invalid_range(empty_range));
                return empty_range;
            }
        }

        SmallRange empty_left(clear_range.begin, clear_range.begin);
        SmallRange empty_right(clear_range.end, clear_range.end);

        // The clearout is entirely within a single extant range, trim and set.
        if (first_range.valid() && first_range.includes(clear_range)) {
            // Shuffle around first_range, three cases...
            if (first_range.begin < clear_range.begin) {
                // We have a lower trimmed area to preserve.
                resize_value(first_range.begin, clear_range.begin);
                rerange_end(first_range.begin, clear_range.begin, clear_range.begin);
                if (first_range.end > clear_range.end) {
                    // And an upper portion of first that needs to copy from the lower
                    construct_value(clear_range.end, std::make_pair(key_type(clear_range.end, first_range.end),
                                                                    get_value(first_range.begin)->second));
                    rerange_begin(clear_range.end, first_range.end, clear_range.end);
                } else {
                    RANGE_ASSERT(first_range.end == clear_range.end);
                    empty_right.end = find_inuse_right(clear_range);
                }
            } else {
                RANGE_ASSERT(first_range.end > clear_range.end);
                RANGE_ASSERT(first_range.begin == clear_range.begin);
                // Only an upper trimmed area to preserve, so move the first range value to the upper trim zone.
                resize_value_right(first_range, clear_range.end);
                rerange_begin(clear_range.end, first_range.end, clear_range.end);
                empty_left.begin = find_inuse_left(clear_range);
            }
        } else {
            if (first_range.valid()) {
                if (first_range.begin < clear_range.begin) {
                    // Trim left.
                    RANGE_ASSERT(first_range.end < clear_range.end);  // we handled the "includes" case above
                    resize_value(first_range.begin, clear_range.begin);
                    rerange_end(first_range.begin, clear_range.begin, clear_range.begin);
                }
            } else {
                empty_left.begin = find_inuse_left(clear_range);
            }

            // rewrite excluded portion of final range
            if (clear_range.end < limit_) {
                const auto &last_range = ranges_[clear_range.end];
                if (last_range.valid()) {
                    // for a valid adjoining range we don't have to change empty_right, but we may have to trim
                    if (last_range.begin < clear_range.end) {
                        resize_value_right(last_range, clear_range.end);
                        rerange_begin(clear_range.end, last_range.end, clear_range.end);
                    }
                } else {
                    // Note: invalid ranges "begin" and the next inuse range (or end)
                    empty_right.end = last_range.begin;
                }
            }
        }

        const SmallRange empty(empty_left.begin, empty_right.end);
        // Clear out the contents
        for (auto i = empty.begin; i < empty.end; ++i) {
            const auto &range = ranges_[i];
            if (range.begin == i) {
                RANGE_ASSERT(range.valid());
                // Clean up the backing store
                destruct_value(i);
            }
        }

        // Rewrite the ranges
        if (valid_clear_range) {
            rerange_begin(empty_left.begin, empty_left.end, clear_range.begin);
            rerange(clear_range, clear_range);
            rerange_end(empty_right.begin, empty_right.end, clear_range.end);
        } else {
            rerange(empty, make_invalid_range(empty));
        }
        RANGE_ASSERT(empty.end == limit_ || ranges_[empty.end].valid());
        RANGE_ASSERT(empty.begin == 0 || ranges_[empty.begin - 1].valid());
        return empty;
    }

    void init_range() {
        const SmallRange init_val(limit_, 0);
        for (SmallIndex i = 0; i < limit_; ++i) {
            ranges_[i] = init_val;
            in_use_[i] = false;
        }
    }
    value_type *get_value(SmallIndex index) {
        RANGE_ASSERT(index < limit_);  // Must be inbounds
        return reinterpret_cast<value_type *>(&(key_values_[index]));
    }
    const value_type *get_value(SmallIndex index) const {
        RANGE_ASSERT(index < limit_);                 // Must be inbounds
        RANGE_ASSERT(index == ranges_[index].begin);  // Must be the record at begin
        return reinterpret_cast<const value_type *>(&(key_values_[index]));
    }

    template <typename Value>
    void construct_value(SmallIndex index, Value &&value) {
        RANGE_ASSERT(!in_use_[index]);
        new (get_value(index)) value_type(std::forward<Value>(value));
        in_use_[index] = true;
        ++size_;
    }

    void destruct_value(SmallIndex index) {
        // there are times when the range and destruct logic clash... allow for double attempted deletes
        if (in_use_[index]) {
            RANGE_ASSERT(size_ > 0);
            --size_;
            get_value(index)->~value_type();
            in_use_[index] = false;
        }
    }

    // No need to move around the value, when just the key is moving
    // Use the destructor/placement new just in case of a complex key with range's semantics
    // Note: Call resize before rewriting ranges_
    void resize_value(SmallIndex current_begin, index_type new_end) {
        // Destroy and rewrite the key in place
        RANGE_ASSERT(ranges_[current_begin].end != new_end);
        key_type new_key(current_begin, new_end);
        key_type *key = const_cast<key_type *>(&get_value(current_begin)->first);
        key->~key_type();
        new (key) key_type(new_key);
    }

    inline void rerange_end(SmallIndex old_begin, SmallIndex new_end, SmallIndex new_end_value) {
        for (auto i = old_begin; i < new_end; ++i) {
            ranges_[i].end = new_end_value;
        }
    }
    inline void rerange_begin(SmallIndex new_begin, SmallIndex old_end, SmallIndex new_begin_value) {
        for (auto i = new_begin; i < old_end; ++i) {
            ranges_[i].begin = new_begin_value;
        }
    }
    inline void rerange(const SmallRange &range, const SmallRange &range_value) {
        for (auto i = range.begin; i < range.end; ++i) {
            ranges_[i] = range_value;
        }
    }

    // for resize right need both begin and end...
    void resize_value_right(const SmallRange &current_range, index_type new_begin) {
        // Use move semantics for (potentially) heavyweight mapped_type's
        RANGE_ASSERT(current_range.begin != new_begin);
        // Move second from it's current location and update the first at the same time
        construct_value(static_cast<SmallIndex>(new_begin),
                        std::make_pair(key_type(new_begin, current_range.end), std::move(get_value(current_range.begin)->second)));
        destruct_value(current_range.begin);
    }

    // Now we can walk a range and rewrite it cleaning up any live contents
    void clear_and_set_range(SmallIndex rewrite_begin, SmallIndex rewrite_end, const SmallRange &new_range) {
        for (auto i = rewrite_begin; i < rewrite_end; ++i) {
            auto &range = ranges_[i];
            if (i == range.begin) {
                destruct_value(i);
            }
            range = new_range;
        }
    }

    SmallIndex next_range(SmallIndex current) const {
        SmallIndex next = ranges_[current].end;
        // If the next range is invalid, skip to the next range, which *must* be (or be end)
        if ((next < limit_) && !ranges_[next].valid()) {
            // For invalid ranges, begin is the beginning of the next range
            next = ranges_[next].begin;
            RANGE_ASSERT(next == limit_ || ranges_[next].valid());
        }
        return next;
    }

    SmallIndex prev_range(SmallIndex current) const {
        if (current == 0) {
            return 0;
        }

        auto prev = current - 1;
        if (ranges_[prev].valid()) {
            // For valid ranges, the range denoted by begin (as that's where the backing store keeps values
            prev = ranges_[prev].begin;
        } else if (prev != 0) {
            // Invalid but not off the front, we can recur (only once) from the end of the prev range to get the answer
            // For invalid ranges this is the end of the previous range
            prev = prev_range(ranges_[prev].end);
        }
        return prev;
    }

    friend iterator;
    friend const_iterator;
    // Stores range boundaries only
    //     open ranges, stored as inverted, invalid range (begining of next, end of prev]
    //     inuse(begin, end) for all entries  on (begin, end]
    SmallIndex size_;
    SmallIndex limit_;
    std::array<SmallRange, N> ranges_;
    std::array<std::pair<key_type, mapped_type>, N> key_values_;
    std::array<bool, N> in_use_;
};

}  // namespace sparse_container
