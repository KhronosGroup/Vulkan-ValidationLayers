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

#include <algorithm>
#include <cassert>
#include <map>
#include <utility>
#include "containers/range.h"

#define RANGE_ASSERT(b) assert(b)

namespace sparse_container {

enum class value_precedence { prefer_source, prefer_dest };

template <typename Iterator, typename Map, typename Range>
Iterator split(Iterator in, Map &map, const Range &range);

// range_map
//
// The range based sparse map implemented on the ImplMap.
// Implements an ordered map of non-overlapping, non-empty ranges
template <typename Key, typename T, typename ImplMap = std::map<vvl::range<Key>, T>>
class range_map {
  private:
    ImplMap impl_map_;
    using ImplIterator = typename ImplMap::iterator;
    using ImplConstIterator = typename ImplMap::const_iterator;
    template <typename IndexType>
    using range = vvl::range<IndexType>;

  public:
    using mapped_type = typename ImplMap::mapped_type;
    using value_type = typename ImplMap::value_type;
    using key_type = typename ImplMap::key_type;
    using index_type = typename key_type::index_type;
    using size_type = typename ImplMap::size_type;

  protected:
    template <typename ThisType>
    using ConstCorrectImplIterator = decltype(std::declval<ThisType>().impl_begin());

    template <typename ThisType, typename WrappedIterator = ConstCorrectImplIterator<ThisType>>
    static WrappedIterator lower_bound_impl(ThisType &that, const key_type &key) {
        if (key.valid()) {
            // ImplMap doesn't give us what want with a direct query, it will give us the first entry contained (if any) in key,
            // not the first entry intersecting key, so, first look for the the first entry that starts at or after key.begin
            // with the operator > in range, we can safely use an empty range for comparison
            auto lower = that.impl_map_.lower_bound(key_type(key.begin, key.begin));

            // If there is a preceding entry it's possible that begin is included, as all we know is that lower.begin >= key.begin
            // or lower is at end
            if (!that.at_impl_begin(lower)) {
                auto prev = lower;
                --prev;
                // If the previous entry includes begin (and we know key.begin > prev.begin) then prev is actually lower
                if (key.begin < prev->first.end) {
                    lower = prev;
                }
            }
            return lower;
        }
        // Key is ill-formed
        return that.impl_end();  // Point safely to nothing.
    }

    ImplIterator lower_bound_impl(const key_type &key) { return lower_bound_impl(*this, key); }

    ImplConstIterator lower_bound_impl(const key_type &key) const { return lower_bound_impl(*this, key); }

    template <typename ThisType, typename WrappedIterator = ConstCorrectImplIterator<ThisType>>
    static WrappedIterator upper_bound_impl(ThisType &that, const key_type &key) {
        if (key.valid()) {
            // the upper bound is the first range that is full greater (upper.begin >= key.end
            // we can get close by looking for the first to exclude key.end, then adjust to account for the fact that key.end is
            // exclusive and we thus ImplMap::upper_bound may be off by one here, i.e. the previous may be the upper bound
            auto upper = that.impl_map_.upper_bound(key_type(key.end, key.end));
            if (!that.at_impl_end(upper) && (upper != that.impl_begin())) {
                auto prev = upper;
                --prev;
                // We know key.end  is >= prev.begin, the only question is whether it's ==
                if (prev->first.begin == key.end) {
                    upper = prev;
                }
            }
            return upper;
        }
        return that.impl_end();  // Point safely to nothing.
    }

    ImplIterator upper_bound_impl(const key_type &key) { return upper_bound_impl(*this, key); }

    ImplConstIterator upper_bound_impl(const key_type &key) const { return upper_bound_impl(*this, key); }

    ImplIterator impl_find(const key_type &key) { return impl_map_.find(key); }
    ImplConstIterator impl_find(const key_type &key) const { return impl_map_.find(key); }
    bool impl_not_found(const key_type &key) const { return impl_end() == impl_find(key); }

    ImplIterator impl_end() { return impl_map_.end(); }
    ImplConstIterator impl_end() const { return impl_map_.end(); }

    ImplIterator impl_begin() { return impl_map_.begin(); }
    ImplConstIterator impl_begin() const { return impl_map_.begin(); }

    inline bool at_impl_end(const ImplIterator &pos) { return pos == impl_end(); }
    inline bool at_impl_end(const ImplConstIterator &pos) const { return pos == impl_end(); }

    inline bool at_impl_begin(const ImplIterator &pos) { return pos == impl_begin(); }
    inline bool at_impl_begin(const ImplConstIterator &pos) const { return pos == impl_begin(); }

    ImplIterator impl_erase(const ImplIterator &pos) { return impl_map_.erase(pos); }

    template <typename Value>
    ImplIterator impl_insert(const ImplIterator &hint, Value &&value) {
        RANGE_ASSERT(impl_not_found(value.first));
        RANGE_ASSERT(value.first.non_empty());
        return impl_map_.emplace_hint(hint, std::forward<Value>(value));
    }
    ImplIterator impl_insert(const ImplIterator &hint, const key_type &key, const mapped_type &value) {
        return impl_insert(hint, std::make_pair(key, value));
    }

    ImplIterator impl_insert(const ImplIterator &hint, const index_type &begin, const index_type &end, const mapped_type &value) {
        return impl_insert(hint, key_type(begin, end), value);
    }

    ImplIterator split_impl(const ImplIterator &split_it, const index_type &index) {
        const auto range = split_it->first;

        if (!range.includes(index)) {
            return split_it;  // If we don't have a valid split point, just return the iterator
        }

        key_type lower_range(range.begin, index);

        if (lower_range.empty()) {
            // This is a noop, we're keeping the upper half which is the same as split_it
            return split_it;
        }

        // Save the contents and erase
        auto value = split_it->second;
        auto next_it = impl_map_.erase(split_it);

        key_type upper_range(index, range.end);
        assert(!upper_range.empty());  // Upper range cannot be empty

        // Copy value to the upper range
        // NOTE: we insert from upper to lower because that's what emplace_hint can do in constant time
        RANGE_ASSERT(impl_map_.find(upper_range) == impl_map_.end());
        next_it = impl_map_.emplace_hint(next_it, std::make_pair(upper_range, value));

        // Move value to the lower range (we can move since the upper range already got a copy of value)
        RANGE_ASSERT(impl_map_.find(lower_range) == impl_map_.end());
        next_it = impl_map_.emplace_hint(next_it, std::make_pair(lower_range, std::move(value)));

        // Iterator to the beginning of the lower range
        return next_it;
    }

    ImplIterator split_impl_keep_only_lower(const ImplIterator &split_it, const index_type &index) {
        const auto range = split_it->first;

        if (!range.includes(index)) {
            return split_it;  // If we don't have a valid split point, just return the iterator
        }

        key_type lower_range(range.begin, index);

        // Save the contents and erase
        auto value = split_it->second;
        auto next_it = impl_map_.erase(split_it);

        if (lower_range.empty()) {
            // This effectively an erase because this function does not keep upper range and lower is empty
            return next_it;
        }

        RANGE_ASSERT(impl_map_.find(lower_range) == impl_map_.end());
        next_it = impl_map_.emplace_hint(next_it, std::make_pair(lower_range, std::move(value)));

        // Iterator to the beginning of the lower range
        return next_it;
    }

    template <typename TouchOp>
    ImplIterator impl_erase_range(const key_type &bounds, ImplIterator lower, const TouchOp &touch_mapped_value) {
        // Logic assumes we are starting at a valid lower bound
        RANGE_ASSERT(!at_impl_end(lower));
        RANGE_ASSERT(lower == lower_bound_impl(bounds));

        // Trim/infill the beginning if needed
        auto current = lower;
        const auto first_begin = current->first.begin;
        if (bounds.begin > first_begin) {
            // Preserve the portion of lower bound excluded from bounds
            if (current->first.end <= bounds.end) {
                // If current ends within the erased bound we can discard the the upper portion of current
                current = split_impl_keep_only_lower(current, bounds.begin);
            } else {
                // Keep the upper portion of current for the later split below
                current = split_impl(current, bounds.begin);
            }
            // Exclude the preserved portion
            ++current;
            RANGE_ASSERT(current == lower_bound_impl(bounds));
        }

        // Loop over completely contained entries and erase them
        while (!at_impl_end(current) && (current->first.end <= bounds.end)) {
            if (touch_mapped_value(current->second)) {
                current = impl_erase(current);
            } else {
                ++current;
            }
        }

        if (!at_impl_end(current) && current->first.includes(bounds.end)) {
            // last entry extends past the end of the bounds range, snip to only erase the bounded section
            current = split_impl(current, bounds.end);
            // test if lower_bound (eventually) computed in split_impl is not empty.
            // If it is not empty, then it contains values inside the bounds range,
            // they need to be touched
            if ((current->first & bounds).non_empty()) {
                if (touch_mapped_value(current->second)) {
                    current = impl_erase(current);
                } else {
                    // make current point to upper bound
                    ++current;
                }
            }
        }

        RANGE_ASSERT(current == upper_bound_impl(bounds));
        return current;
    }

    template <typename ValueType, typename WrappedIterator_>
    struct iterator_impl {
      public:
        friend class range_map;
        using WrappedIterator = WrappedIterator_;

      private:
        WrappedIterator pos_;

        // Create an iterator at a specific internal state -- only from the parent container
        iterator_impl(const WrappedIterator &pos) : pos_(pos) {}

      public:
        iterator_impl() : iterator_impl(WrappedIterator()) {}
        iterator_impl(const iterator_impl &other) : pos_(other.pos_) {}

        iterator_impl &operator=(const iterator_impl &rhs) {
            pos_ = rhs.pos_;
            return *this;
        }

        inline bool operator==(const iterator_impl &rhs) const { return pos_ == rhs.pos_; }

        inline bool operator!=(const iterator_impl &rhs) const { return pos_ != rhs.pos_; }

        ValueType &operator*() const { return *pos_; }
        ValueType *operator->() const { return &*pos_; }

        iterator_impl &operator++() {
            ++pos_;
            return *this;
        }

        iterator_impl &operator--() {
            --pos_;
            return *this;
        }

        // To allow for iterator -> const_iterator construction
        // NOTE: while it breaks strict encapsulation, it does so less than friend
        const WrappedIterator &get_pos() const { return pos_; };
    };

  public:
    using iterator = iterator_impl<value_type, ImplIterator>;

    // The const iterator must be derived to allow the conversion from iterator, which iterator doesn't support
    class const_iterator : public iterator_impl<const value_type, ImplConstIterator> {
        using Base = iterator_impl<const value_type, ImplConstIterator>;
        friend range_map;

      public:
        const_iterator &operator=(const const_iterator &other) {
            Base::operator=(other);
            return *this;
        }
        const_iterator(const const_iterator &other) : Base(other){};
        const_iterator(const iterator &it) : Base(ImplConstIterator(it.get_pos())) {}
        const_iterator() : Base() {}

      private:
        const_iterator(const ImplConstIterator &pos) : Base(pos) {}
    };

  private:
    inline bool at_end(const iterator &it) { return at_impl_end(it.pos_); }
    inline bool at_end(const const_iterator &it) const { return at_impl_end(it.pos_); }
    inline bool at_begin(const iterator &it) { return at_impl_begin(it.pos_); }

  public:
    iterator end() { return iterator(impl_map_.end()); }                          //  policy and bounds don't matter for end
    const_iterator end() const { return const_iterator(impl_map_.end()); }        //  policy and bounds don't matter for end
    iterator begin() { return iterator(impl_map_.begin()); }                      // with default policy, and thus no bounds
    const_iterator begin() const { return const_iterator(impl_map_.begin()); }    // with default policy, and thus no bounds
    const_iterator cbegin() const { return const_iterator(impl_map_.cbegin()); }  // with default policy, and thus no bounds
    const_iterator cend() const { return const_iterator(impl_map_.cend()); }      // with default policy, and thus no bounds

    iterator erase(const iterator &pos) {
        RANGE_ASSERT(!at_end(pos));
        return iterator(impl_erase(pos.pos_));
    }

    iterator erase(range<iterator> bounds) {
        auto current = bounds.begin.pos_;
        while (current != bounds.end.pos_) {
            RANGE_ASSERT(!at_impl_end(current));
            current = impl_map_.erase(current);
        }
        RANGE_ASSERT(current == bounds.end.pos_);
        return current;
    }

    iterator erase(iterator first, iterator last) { return erase(range<iterator>(first, last)); }

    // Before trying to erase a range, function touch_mapped_value is called on the mapped value.
    // touch_mapped_value is allowed to have it's parameter type to be non const reference.
    // If it returns true, regular erase will occur.
    // Else, range is kept.
    template <typename TouchOp>
    iterator erase_range_or_touch(const key_type &bounds, const TouchOp &touch_mapped_value) {
        auto lower = lower_bound_impl(bounds);

        if (at_impl_end(lower) || !bounds.intersects(lower->first)) {
            // There is nothing in this range lower bound is above bound
            return iterator(lower);
        }
        auto next = impl_erase_range(bounds, lower, touch_mapped_value);
        return iterator(next);
    }

    iterator erase_range(const key_type &bounds) {
        return erase_range_or_touch(bounds, [](const auto &) { return true; });
    }

    void clear() { impl_map_.clear(); }

    iterator find(const key_type &key) { return iterator(impl_map_.find(key)); }

    const_iterator find(const key_type &key) const { return const_iterator(impl_map_.find(key)); }

    iterator find(const index_type &index) {
        auto lower = lower_bound(range<index_type>(index, index + 1));
        if (!at_end(lower) && lower->first.includes(index)) {
            return lower;
        }
        return end();
    }

    const_iterator find(const index_type &index) const {
        auto lower = lower_bound(key_type(index, index + 1));
        if (!at_end(lower) && lower->first.includes(index)) {
            return lower;
        }
        return end();
    }

    iterator lower_bound(const key_type &key) { return iterator(lower_bound_impl(key)); }

    const_iterator lower_bound(const key_type &key) const { return const_iterator(lower_bound_impl(key)); }

    iterator upper_bound(const key_type &key) { return iterator(upper_bound_impl(key)); }

    const_iterator upper_bound(const key_type &key) const { return const_iterator(upper_bound_impl(key)); }

    using insert_pair = std::pair<iterator, bool>;

    // This is traditional no replacement insert.
    insert_pair insert(const value_type &value) {
        const auto &key = value.first;
        if (!key.non_empty()) {
            // It's an invalid key, early bail pointing to end
            return std::make_pair(end(), false);
        }

        // Look for range conflicts (and an insertion point, which makes the lower_bound *not* wasted work)
        // we don't have to check upper if just check that lower doesn't intersect (which it would if lower != upper)
        auto lower = lower_bound_impl(key);
        if (at_impl_end(lower) || !lower->first.intersects(key)) {
            // range is not even partially overlapped, and lower is strictly > than key
            auto impl_insert = impl_map_.emplace_hint(lower, value);
            // auto impl_insert = impl_map_.emplace(value);
            iterator wrap_it(impl_insert);
            return std::make_pair(wrap_it, true);
        }
        // We don't replace
        return std::make_pair(iterator(lower), false);
    };

    iterator insert(const_iterator hint, const value_type &value) {
        bool hint_open;
        ImplConstIterator impl_next = hint.pos_;
        if (impl_map_.empty()) {
            hint_open = true;
        } else if (impl_next == impl_map_.cbegin()) {
            hint_open = value.first.strictly_less(impl_next->first);
        } else if (impl_next == impl_map_.cend()) {
            auto impl_prev = impl_next;
            --impl_prev;
            hint_open = value.first.strictly_greater(impl_prev->first);
        } else {
            auto impl_prev = impl_next;
            --impl_prev;
            hint_open = value.first.strictly_greater(impl_prev->first) && value.first.strictly_less(impl_next->first);
        }

        if (!hint_open) {
            // Hint was unhelpful, fall back to the non-hinted version
            auto plain_insert = insert(value);
            return plain_insert.first;
        }

        auto impl_insert = impl_map_.insert(impl_next, value);
        return iterator(impl_insert);
    }

    iterator split(const iterator whole_it, const index_type &index) {
        auto split_it = split_impl(whole_it.pos_, index);
        return iterator(split_it);
    }

    // The overwrite hint here is lower.... and if it's not right... this fails
    template <typename Value>
    iterator overwrite_range(const iterator &lower, Value &&value) {
        // We're not robust to a bad hint, so detect it with extreme prejudice
        // TODO: Add bad hint test to make this robust...
        auto lower_impl = lower.pos_;
        auto insert_hint = lower_impl;
        if (!at_impl_end(lower_impl)) {
            // If we're at end (and the hint is good, there's nothing to erase
            RANGE_ASSERT(lower == lower_bound(value.first));
            insert_hint = impl_erase_range(value.first, lower_impl, [](const auto &) { return true; });
        }
        auto inserted = impl_insert(insert_hint, std::forward<Value>(value));
        return iterator(inserted);
    }

    template <typename Value>
    iterator overwrite_range(Value &&value) {
        auto lower = lower_bound(value.first);
        return overwrite_range(lower, value);
    }

    bool empty() const { return impl_map_.empty(); }
    size_type size() const { return impl_map_.size(); }
};

template <typename Container>
using const_correct_iterator = decltype(std::declval<Container>().begin());

// Forward index iterator, tracking an index value and the appropos lower bound
// returns an index_type, lower_bound pair.  Supports ++,  offset, and seek affecting the index,
// lower bound updates as needed. As the index may specify a range for which no entry exist, dereferenced
// iterator includes an "valid" field, true IFF the lower_bound is not end() and contains [index, index +1)
//
// Must be explicitly invalidated when the underlying map is changed.
template <typename Map>
class cached_lower_bound_impl {
    using plain_map_type = typename std::remove_const<Map>::type;  // Allow instatiation with const or non-const Map
  public:
    using iterator = const_correct_iterator<Map>;
    using key_type = typename plain_map_type::key_type;
    using mapped_type = typename plain_map_type::mapped_type;
    // Both sides of the return pair are const'd because we're returning references/pointers to the *internal* state
    // and we don't want and caller altering internal state.
    using index_type = typename Map::index_type;
    struct value_type {
        const index_type &index;
        const iterator &lower_bound;
        const bool &valid;
        value_type(const index_type &index_, const iterator &lower_bound_, bool &valid_)
            : index(index_), lower_bound(lower_bound_), valid(valid_) {}
    };

  private:
    Map *const map_;
    const iterator end_;
    value_type pos_;

    index_type index_;
    iterator lower_bound_;
    bool valid_;

    bool is_valid() const { return includes(index_); }

    // Allow reuse of a type with const semantics
    void set_value(const index_type &index, const iterator &it) {
        RANGE_ASSERT(it == lower_bound(index));
        index_ = index;
        lower_bound_ = it;
        valid_ = is_valid();
    }

    void update(const index_type &index) {
        RANGE_ASSERT(lower_bound_ == lower_bound(index));
        index_ = index;
        valid_ = is_valid();
    }

    inline iterator lower_bound(const index_type &index) { return map_->lower_bound(key_type(index, index + 1)); }
    inline bool at_end(const iterator &it) const { return it == end_; }

    bool is_lower_than(const index_type &index, const iterator &it) { return at_end(it) || (index < it->first.end); }

  public:
    // The cached lower bound knows the parent map, and thus can tell us this...
    inline bool at_end() const { return at_end(lower_bound_); }
    // includes(index) is a convenience function to test if the index would be in the currently cached lower bound
    bool includes(const index_type &index) const { return !at_end() && lower_bound_->first.includes(index); }

    // The return is const because we are sharing the internal state directly.
    const value_type &operator*() const { return pos_; }
    const value_type *operator->() const { return &pos_; }

    // Advance the cached location by 1
    cached_lower_bound_impl &operator++() {
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
            RANGE_ASSERT(is_lower_than(next, next_it));
        }

        return *this;
    }

    // seek(index) updates lower_bound for index, updating lower_bound_ as needed.
    cached_lower_bound_impl &seek(const index_type &seek_to) {
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
        return *this;
    }

    // Advance the cached location by offset.
    cached_lower_bound_impl &offset(const index_type &offset) {
        const index_type next = index_ + offset;
        return seek(next);
    }

    // invalidate() resets the the lower_bound_ cache, needed after insert/erase/overwrite/split operations
    // Pass index by value in case we are invalidating to index_ and set_value does a modify-in-place on index_
    cached_lower_bound_impl &invalidate(index_type index) {
        set_value(index, lower_bound(index));
        return *this;
    }

    // For times when the application knows what it's done to the underlying map... (with assert in set_value)
    cached_lower_bound_impl &invalidate(const iterator &hint, index_type index) {
        set_value(index, hint);
        return *this;
    }

    cached_lower_bound_impl &invalidate() { return invalidate(index_); }

    // Allow a hint for a *valid* lower bound for current index
    // TODO: if the fail-over becomes a hot-spot, the hint logic could be far more clever (looking at previous/next...)
    cached_lower_bound_impl &invalidate(const iterator &hint) {
        if ((hint != end_) && hint->first.includes(index_)) {
            auto index = index_;  // by copy set modifies in place
            set_value(index, hint);
        } else {
            invalidate();
        }
        return *this;
    }

    // The offset in index type to the next change (the end of the current range, or the transition from invalid to
    // valid.  If invalid and at_end, returns index_type(0)
    index_type distance_to_edge() {
        if (valid_) {
            // Distance to edge of
            return lower_bound_->first.end - index_;
        } else if (at_end()) {
            return index_type(0);
        } else {
            return lower_bound_->first.begin - index_;
        }
    }

    Map &map() { return *map_; }
    const Map &map() const { return *map_; }

    // Default constructed object reports valid (correctly) as false, but otherwise will fail (assert) under nearly any use.
    cached_lower_bound_impl()
        : map_(nullptr), end_(), pos_(index_, lower_bound_, valid_), index_(0), lower_bound_(), valid_(false) {}
    cached_lower_bound_impl(Map &map, const index_type &index)
        : map_(&map),
          end_(map.end()),
          pos_(index_, lower_bound_, valid_),
          index_(index),
          lower_bound_(lower_bound(index)),
          valid_(is_valid()) {}
};

template <typename CachedLowerBound, typename MappedType = typename CachedLowerBound::mapped_type>
const MappedType &evaluate(const CachedLowerBound &clb, const MappedType &default_value) {
    if (clb->valid) {
        return clb->lower_bound->second;
    }
    return default_value;
}

// Split a range into pieces bound by the intersection of the iterator's range and the supplied range
template <typename Iterator, typename Map, typename Range>
Iterator split(Iterator in, Map &map, const Range &range) {
    assert(in != map.end());  // Not designed for use with invalid iterators...
    const auto in_range = in->first;
    const auto split_range = in_range & range;

    if (split_range.empty()) return map.end();

    auto pos = in;
    if (split_range.begin != in_range.begin) {
        pos = map.split(pos, split_range.begin);
        ++pos;
    }
    if (split_range.end != in_range.end) {
        pos = map.split(pos, split_range.end);
    }
    return pos;
}

// Apply an operation over a range map, infilling where content is absent, updating where content is present.
// The passed pos must *either* be strictly less than range or *is* lower_bound (which may be end)
// Trims to range boundaries.
// infill op doesn't have to alter map, but mustn't invalidate iterators passed to it. (i.e. no erasure)
// infill data (default mapped value or other initial value) is contained with ops.
// update allows existing ranges to be updated (merged, whatever) based on data contained in ops.  All iterators
// passed to update are already trimmed to fit within range.
template <typename RangeMap, typename InfillUpdateOps, typename Iterator = typename RangeMap::iterator>
Iterator infill_update_range(RangeMap &map, Iterator pos, const typename RangeMap::key_type &range, const InfillUpdateOps &ops) {
    using KeyType = typename RangeMap::key_type;
    using IndexType = typename RangeMap::index_type;

    const auto end = map.end();
    assert((pos == end) || (pos == map.lower_bound(range)) || pos->first.strictly_less(range));

    if (range.empty()) return pos;
    if (pos == end) {
        // Only pass pos == end for range tail after last entry
        assert(end == map.lower_bound(range));
    } else if (pos->first.strictly_less(range)) {
        // pos isn't lower_bound for range (it's less than range), however, if range is monotonically increasing it's likely
        // the next entry in the map will be the lower bound.

        // If the new (pos + 1) *isn't* stricly_less and pos is,
        // (pos + 1) must be the lower_bound, otherwise we have to look for it O(log n)
        ++pos;
        if ((pos != end) && pos->first.strictly_less(range)) {
            pos = map.lower_bound(range);
        }
        assert(pos == map.lower_bound(range));
    }

    if ((pos != end) && (range.begin > pos->first.begin)) {
        // lower bound starts before the range, trim and advance
        pos = map.split(pos, range.begin);
        ++pos;
    }

    IndexType current_begin = range.begin;
    while (pos != end && current_begin < range.end) {
        if (current_begin < pos->first.begin) {
            // The current_begin is pointing to the beginning of a gap to infill (we supply pos for "insert in front of" calls)
            ops.infill(map, pos, KeyType(current_begin, std::min(range.end, pos->first.begin)));
            // Advance current begin, but *not* pos as it's the next valid value. (infill shall not invalidate pos)
            current_begin = pos->first.begin;
        } else {
            // The current_begin is pointing to the next existing value to update
            assert(current_begin == pos->first.begin);

            // We need to run the update operation on the valid portion of the current value.
            // If this entry overlaps end-of-range we need to trim it to the range
            if (pos->first.end > range.end) {
                pos = map.split(pos, range.end);
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
        ops.infill(map, pos, KeyType(current_begin, range.end));
    }

    return pos;
}

template <typename RangeMap, typename InfillUpdateOps>
void infill_update_range(RangeMap &map, const typename RangeMap::key_type &range, const InfillUpdateOps &ops) {
    if (range.empty()) return;
    auto pos = map.lower_bound(range);
    infill_update_range(map, pos, range, ops);
}

// Parallel iterator
// Traverse to range maps over the the same range, but without assumptions of aligned ranges.
// ++ increments to the next point where on of the two maps changes range, giving a range over which the two
// maps do not transition ranges
template <typename MapA, typename MapB = MapA, typename KeyType = typename MapA::key_type>
class parallel_iterator {
  public:
    using key_type = KeyType;
    using index_type = typename key_type::index_type;

    // The traits keep the iterator/const_interator consistent with the constness of the map.
    using map_type_A = MapA;
    using plain_map_type_A = typename std::remove_const<MapA>::type;  // Allow instatiation with const or non-const Map
    using key_type_A = typename plain_map_type_A::key_type;
    using index_type_A = typename plain_map_type_A::index_type;
    using iterator_A = const_correct_iterator<map_type_A>;
    using lower_bound_A = cached_lower_bound_impl<map_type_A>;

    using map_type_B = MapB;
    using plain_map_type_B = typename std::remove_const<MapB>::type;
    using key_type_B = typename plain_map_type_B::key_type;
    using index_type_B = typename plain_map_type_B::index_type;
    using iterator_B = const_correct_iterator<map_type_B>;
    using lower_bound_B = cached_lower_bound_impl<map_type_B>;

    // This is the value we'll always be returning, but the referenced object will be updated by the operations
    struct value_type {
        const key_type &range;
        const lower_bound_A &pos_A;
        const lower_bound_B &pos_B;
        value_type(const key_type &range_, const lower_bound_A &pos_A_, const lower_bound_B &pos_B_)
            : range(range_), pos_A(pos_A_), pos_B(pos_B_) {}
    };

  private:
    lower_bound_A pos_A_;
    lower_bound_B pos_B_;
    key_type range_;
    value_type pos_;
    index_type compute_delta() {
        auto delta_A = pos_A_.distance_to_edge();
        auto delta_B = pos_B_.distance_to_edge();
        index_type delta_min;

        // If either A or B are at end, there distance is *0*, so shouldn't be considered in the "distance to edge"
        if (delta_A == 0) {  // lower A is at end
            delta_min = static_cast<index_type>(delta_B);
        } else if (delta_B == 0) {  // lower B is at end
            delta_min = static_cast<index_type>(delta_A);
        } else {
            // Neither are at end, use the nearest edge, s.t. over this range A and B are both constant
            delta_min = std::min(static_cast<index_type>(delta_A), static_cast<index_type>(delta_B));
        }
        return delta_min;
    }

  public:
    // Default constructed object will report range empty (for end checks), but otherwise is unsafe to use
    parallel_iterator() : pos_A_(), pos_B_(), range_(), pos_(range_, pos_A_, pos_B_) {}
    parallel_iterator(map_type_A &map_A, map_type_B &map_B, index_type index)
        : pos_A_(map_A, static_cast<index_type_A>(index)),
          pos_B_(map_B, static_cast<index_type_B>(index)),
          range_(index, index + compute_delta()),
          pos_(range_, pos_A_, pos_B_) {}

    // Advance to the next spot one of the two maps changes
    parallel_iterator &operator++() {
        const auto start = range_.end;         // we computed this the last time we set range
        const auto delta = range_.distance();  // we computed this the last time we set range
        RANGE_ASSERT(delta != 0);              // Trying to increment past end

        pos_A_.offset(static_cast<index_type_A>(delta));
        pos_B_.offset(static_cast<index_type_B>(delta));

        range_ = key_type(start, start + compute_delta());  // find the next boundary (must be after offset)
        RANGE_ASSERT(pos_A_->index == start);
        RANGE_ASSERT(pos_B_->index == start);

        return *this;
    }

    // Seeks to a specific index in both maps reseting range.  Cannot guarantee range.begin is on edge boundary,
    /// but range.end will be.  Lower bound objects assumed to invalidate their cached lower bounds on seek.
    parallel_iterator &seek(const index_type &index) {
        pos_A_.seek(static_cast<index_type_A>(index));
        pos_B_.seek(static_cast<index_type_B>(index));
        range_ = key_type(index, index + compute_delta());
        RANGE_ASSERT(pos_A_->index == index);
        RANGE_ASSERT(pos_A_->index == pos_B_->index);
        return *this;
    }

    // Invalidates the lower_bound caches, reseting range.  Cannot guarantee range.begin is on edge boundary,
    // but range.end will be.
    parallel_iterator &invalidate() {
        const index_type start = range_.begin;
        seek(start);
        return *this;
    }

    parallel_iterator &invalidate_A() {
        const index_type index = range_.begin;
        pos_A_.invalidate(static_cast<index_type_A>(index));
        range_ = key_type(index, index + compute_delta());
        return *this;
    }

    parallel_iterator &invalidate_A(const iterator_A &hint) {
        const index_type index = range_.begin;
        pos_A_.invalidate(hint, static_cast<index_type_A>(index));
        range_ = key_type(index, index + compute_delta());
        return *this;
    }

    parallel_iterator &invalidate_B() {
        const index_type index = range_.begin;
        pos_B_.invalidate(static_cast<index_type_B>(index));
        range_ = key_type(index, index + compute_delta());
        return *this;
    }

    parallel_iterator &invalidate_B(const iterator_B &hint) {
        const index_type index = range_.begin;
        pos_B_.invalidate(hint, static_cast<index_type_B>(index));
        range_ = key_type(index, index + compute_delta());
        return *this;
    }

    parallel_iterator &trim_A() {
        if (pos_A_->valid && (range_ != pos_A_->lower_bound->first)) {
            split(pos_A_->lower_bound, pos_A_.map(), range_);
            invalidate_A();
        }
        return *this;
    }

    // The return is const because we are sharing the internal state directly.
    const value_type &operator*() const { return pos_; }
    const value_type *operator->() const { return &pos_; }
};

template <typename DstRangeMap, typename SrcRangeMap, typename Updater>
void splice(DstRangeMap &to, const SrcRangeMap &from, const Updater &updater) {
    typename SrcRangeMap::const_iterator begin = from.cbegin();
    typename SrcRangeMap::const_iterator end = from.cend();

    if (from.empty() || begin == end || begin == from.cend()) {
        return;  // nothing to merge
    }

    using ParallelIterator = parallel_iterator<DstRangeMap, const SrcRangeMap>;
    using Key = typename SrcRangeMap::key_type;
    using CachedLowerBound = cached_lower_bound_impl<DstRangeMap>;
    using ConstCachedLowerBound = cached_lower_bound_impl<const SrcRangeMap>;
    ParallelIterator par_it(to, from, begin->first.begin);
    while (par_it->range.non_empty() && par_it->pos_B->lower_bound != end) {
        const Key &range = par_it->range;
        const CachedLowerBound &to_lb = par_it->pos_A;
        const ConstCachedLowerBound &from_lb = par_it->pos_B;
        if (from_lb->valid) {
            auto read_it = from_lb->lower_bound;
            auto write_it = to_lb->lower_bound;
            // Because of how the parallel iterator walk, "to" is valid over the whole range or it isn't (ranges don't span
            // transitions between map entries or between valid and invalid ranges)
            if (to_lb->valid) {
                if (write_it->first == range) {
                    // if the source and destination ranges match we can overwrite everything
                    updater.update(write_it->second, read_it->second);
                } else {
                    // otherwise we need to split the destination range.
                    auto value_to_update = write_it->second;  // intentional copy
                    updater.update(value_to_update, read_it->second);
                    auto intersected_range = write_it->first & range;
                    to.overwrite_range(to_lb->lower_bound, std::make_pair(intersected_range, value_to_update));
                    par_it.invalidate_A();  // we've changed map 'to' behind to_lb's back... let it know.
                }
            } else {
                // Insert into the gap.
                auto opt = updater.insert(read_it->second);
                if (opt) {
                    to.insert(write_it, std::make_pair(range, std::move(*opt)));
                    par_it.invalidate_A();  // we've changed map 'to' behind to_lb's back... let it know.
                }
            }
        }
        ++par_it;  // next range over which both 'to' and 'from' stay constant
    }
}

template <typename Map, typename Range = typename Map::key_type, typename MapValue = typename Map::mapped_type>
bool update_range_value(Map &map, const Range &range, MapValue &&value, value_precedence precedence) {
    using CachedLowerBound = typename sparse_container::cached_lower_bound_impl<Map>;
    CachedLowerBound pos(map, range.begin);

    bool updated = false;
    while (range.includes(pos->index)) {
        if (!pos->valid) {
            if (precedence == value_precedence::prefer_source) {
                // We can convert this into and overwrite...
                map.overwrite_range(pos->lower_bound, std::make_pair(range, std::forward<MapValue>(value)));
                return true;
            }
            // Fill in the leading space (or in the case of pos at end the trailing space
            const auto start = pos->index;
            auto it = pos->lower_bound;
            const auto limit = (it != map.end()) ? std::min(it->first.begin, range.end) : range.end;
            map.insert(it, std::make_pair(Range(start, limit), value));
            // We inserted before pos->lower_bound, so pos->lower_bound isn't invalid, but the associated index *is* and seek
            // will fix this (and move the state to valid)
            pos.seek(limit);
            updated = true;
        }
        // Note that after the "fill" operation pos may have become valid so we check again
        if (pos->valid) {
            if ((precedence == value_precedence::prefer_source) && (pos->lower_bound->second != value)) {
                // We've found a place where we're changing the value, at this point might as well simply over write the range
                // and be done with it. (save on later merge operations....)
                pos.seek(range.begin);
                map.overwrite_range(pos->lower_bound, std::make_pair(range, std::forward<MapValue>(value)));
                return true;

            } else {
                // "prefer_dest" means don't overwrite existing values, so we'll skip this interval.
                // Point just past the end of this section,  if it's within the given range, it will get filled next iteration
                // ++pos could move us past the end of range (which would exit the loop) so we don't use it.
                pos.seek(pos->lower_bound->first.end);
            }
        }
    }
    return updated;
}

//  combines directly adjacent ranges with equal RangeMap::mapped_type .
template <typename RangeMap>
void consolidate(RangeMap &map) {
    using Value = typename RangeMap::value_type;
    using Key = typename RangeMap::key_type;
    using It = typename RangeMap::iterator;

    It current = map.begin();
    const It map_end = map.end();

    // To be included in a merge range there must be no gap in the Key space, and the mapped_type values must match
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
            Value merged_value = std::make_pair(Key(merge_first->first.begin, merge_last->first.end), merge_last->second);
            // Note that current points to merge_last + 1, and is valid even if at map_end for these operations
            map.erase(merge_first, current);
            map.insert(current, std::move(merged_value));
        }
    }
}

}  // namespace sparse_container
