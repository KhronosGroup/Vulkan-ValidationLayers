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
#pragma once

#ifndef RANGE_VECTOR_H_
#define RANGE_VECTOR_H_

#include <algorithm>
#include <cassert>
#include <map>
#include <utility>

#define RANGE_ASSERT(b) assert(b)

namespace sparse_container {
// range_map
//
// Implements an ordered map of non-overlapping, non-empty ranges
//
template <typename Index>
struct range {
    using index_type = Index;
    index_type begin;  // Inclusive lower bound of range
    index_type end;    // Exlcusive upper bound of range

    inline bool empty() const { return begin == end; }
    inline bool valid() const { return begin <= end; }
    inline bool invalid() const { return !valid(); }
    inline bool non_empty() const { return begin < end; }  //  valid and !empty

    inline bool is_prior_to(const range &other) const { return end == other.begin; }
    inline bool is_subsequent_to(const range &other) const { return begin == other.end; }
    inline bool includes(const index_type &index) const { return (begin <= index) && (index < end); }
    inline bool includes(const range &other) const { return (begin <= other.begin) && (other.end <= end); }
    inline bool excludes(const index_type &index) const { return (index < begin) || (end <= index); }
    inline bool excludes(const range &other) const { return (other.end <= begin) || (end <= other.begin); }
    inline bool intersects(const range &other) const { return includes(other.begin) || other.includes(begin); }
    inline index_type distance() const { return end - begin; }

    inline bool operator==(const range &rhs) const { return (begin == rhs.begin) && (end == rhs.end); }
    inline bool operator!=(const range &rhs) const { return (begin != rhs.begin) || (end != rhs.end); }

    inline range &operator-=(const index_type &offset) {
        begin = begin - offset;
        end = end - offset;
        return *this;
    }

    inline range &operator+=(const index_type &offset) {
        begin = begin + offset;
        end = end + offset;
        return *this;
    }

    // for a reversible/transitive < operator compare first on begin and then end
    // only less or begin is less or if end is less when begin is equal
    bool operator<(const range &rhs) const {
        bool result = false;
        if (invalid()) {
            // all invalid < valid, allows map/set validity check by looking at begin()->first
            // all invalid are equal, thus only equal if this is invalid and rhs is valid
            result = rhs.valid();
        } else if (begin < rhs.begin) {
            result = true;
        } else if ((begin == rhs.begin) && (end < rhs.end)) {
            result = true;  // Simple common case -- boundary case require equality check for correctness.
        }
        return result;
    }

    // use as "strictly less/greater than" to check for non-overlapping ranges
    bool strictly_less(const range &rhs) const { return end <= rhs.begin; }
    bool strictly_less(const index_type &index) const { return end <= index; }
    bool strictly_greater(const range &rhs) const { return rhs.end <= begin; }
    bool strictly_greater(const index_type &index) const { return index < begin; }

    range &operator=(const range &rhs) {
        begin = rhs.begin;
        end = rhs.end;
        return *this;
    }

    range operator&(const range &rhs) const {
        if (includes(rhs.begin)) {
            return range(rhs.begin, std::min(end, rhs.end));
        } else if (rhs.includes(begin)) {
            return range(begin, std::min(end, rhs.end));
        }
        return range();  // Empty default range on non-intersection
    }

    range() : begin(), end() {}
    range(const index_type &begin_, const index_type &end_) : begin(begin_), end(end_) {}
};

template <typename Container>
using const_correct_iterator = decltype(std::declval<Container>().begin());

template <typename Key, typename T>
class range_map {
  public:
  protected:
    using RangeKey = range<Key>;
    using MapKey = RangeKey;
    using ImplMap = std::map<MapKey, T>;
    ImplMap impl_map_;
    using ImplIterator = typename ImplMap::iterator;
    using ImplConstIterator = typename ImplMap::const_iterator;

  public:
    using mapped_type = typename ImplMap::mapped_type;
    using value_type = typename ImplMap::value_type;
    using key_type = typename ImplMap::key_type;
    using index_type = typename key_type::index_type;

    struct split_op_keep_both {
        static constexpr bool keep_lower() { return true; }
        static constexpr bool keep_upper() { return true; }
    };

    struct split_op_keep_lower {
        static constexpr bool keep_lower() { return true; }
        static constexpr bool keep_upper() { return false; }
    };

    struct split_op_keep_upper {
        static constexpr bool keep_lower() { return false; }
        static constexpr bool keep_upper() { return true; }
    };

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

    template <typename SplitOp>
    ImplIterator split_impl(const ImplIterator &split_it, const index_type &index, const SplitOp &) {
        // Make sure contains the split point
        if (!split_it->first.includes(index)) return split_it;  // If we don't have a valid split point, just return the iterator

        const auto range = split_it->first;
        key_type lower_range(range.begin, index);
        if (lower_range.empty() && SplitOp::keep_upper()) {
            return split_it;  // this is a noop we're keeping the upper half which is the same as split_it;
        }
        // Save the contents of it and erase it
        auto value = std::move(split_it->second);
        auto next_it = impl_map_.erase(split_it);  // Keep this, just in case the split point results in an empty "keep" set

        if (lower_range.empty() && !SplitOp::keep_upper()) {
            // This effectively an erase...
            return next_it;
        }
        // Upper range cannot be empty
        key_type upper_range(index, range.end);
        key_type move_range;
        key_type copy_range;

        // Were either going to keep one or both of the split pieces.  If we keep both, we'll copy value to the upper,
        // and move to the lower, and return the lower, else move to, and return the kept one.
        if (SplitOp::keep_lower() && !lower_range.empty()) {
            move_range = lower_range;
            if (SplitOp::keep_upper()) {
                copy_range = upper_range;  // only need a valid copy range if we keep both.
            }
        } else if (SplitOp::keep_upper()) {  // We're not keeping the lower split because it's either empty or not wanted
            move_range = upper_range;        // this will be non_empty as index is included ( < end) in the original range)
        }

        // we insert from upper to lower because that's what emplace_hint can do in constant time. (not log time in C++11)
        if (!copy_range.empty()) {
            // We have a second range to create, so do it by copy
            RANGE_ASSERT(impl_map_.find(copy_range) == impl_map_.end());
            next_it = impl_map_.emplace_hint(next_it, std::make_pair(copy_range, value));
        }

        if (!move_range.empty()) {
            // Whether we keep one or both, the one we return gets value moved to it, as the other one already has a copy
            RANGE_ASSERT(impl_map_.find(move_range) == impl_map_.end());
            next_it = impl_map_.emplace_hint(next_it, std::make_pair(move_range, std::move(value)));
        }

        // point to the beginning of the inserted elements (or the next from the erase
        return next_it;
    }

    // do an ranged insert that splits existing ranges at the boundaries, and writes value to any non-initialized sub-ranges
    range<ImplIterator> infill_and_split(const key_type &bounds, const mapped_type &value, ImplIterator lower, bool split_bounds) {
        auto pos = lower;
        if (at_impl_end(pos)) return range<ImplIterator>(pos, pos);  // defensive...

        // Logic assumes we are starting at lower bound
        RANGE_ASSERT(lower == lower_bound_impl(bounds));

        // Trim/infil the beginning if needed
        const auto first_begin = pos->first.begin;
        if (bounds.begin > first_begin && split_bounds) {
            pos = split_impl(pos, bounds.begin, split_op_keep_both());
            lower = pos;
            ++lower;
            RANGE_ASSERT(lower == lower_bound_impl(bounds));
        } else if (bounds.begin < first_begin) {
            pos = impl_insert(pos, bounds.begin, first_begin, value);
            lower = pos;
            RANGE_ASSERT(lower == lower_bound_impl(bounds));
        }

        // in the trim case pos starts one before lower_bound, but that allows trimming a single entry range in loop.
        // NOTE that the loop is trimming and infilling at pos + 1
        while (!at_impl_end(pos) && pos->first.begin < bounds.end) {
            auto last_end = pos->first.end;
            // check for in-fill
            ++pos;
            if (at_impl_end(pos)) {
                if (last_end < bounds.end) {
                    // Gap after last entry in impl_map and before end,
                    pos = impl_insert(pos, last_end, bounds.end, value);
                    ++pos;  // advances to impl_end, as we're at upper boundary
                    RANGE_ASSERT(at_impl_end(pos));
                }
            } else if (pos->first.begin != last_end) {
                // we have a gap between last entry and current... fill, but not beyond bounds
                if (bounds.includes(pos->first.begin)) {
                    pos = impl_insert(pos, last_end, pos->first.begin, value);
                    //  don't further advance pos, because we may need to split the next entry and thus can't skip it.
                } else if (last_end < bounds.end) {
                    // Non-zero length final gap in-bounds
                    pos = impl_insert(pos, last_end, bounds.end, value);
                    ++pos;  // advances back to the out of bounds entry which we inserted just before
                    RANGE_ASSERT(!bounds.includes(pos->first.begin));
                }
            } else if (pos->first.includes(bounds.end)) {
                if (split_bounds) {
                    // extends past the end of the bounds range, snip to only include the bounded section
                    // NOTE: this splits pos, but the upper half of the split should now be considered upper_bound
                    // for the range
                    pos = split_impl(pos, bounds.end, split_op_keep_both());
                }
                // advance to the upper haf of the split which will be upper_bound  or to next which will both be out of bounds
                ++pos;
                RANGE_ASSERT(!bounds.includes(pos->first.begin));
            }
        }
        // Return the current position which should be the upper_bound for bounds
        RANGE_ASSERT(pos == upper_bound_impl(bounds));
        return range<ImplIterator>(lower, pos);
    }

    ImplIterator impl_erase_range(const key_type &bounds, ImplIterator lower) {
        // Logic assumes we are starting at a valid lower bound
        RANGE_ASSERT(!at_impl_end(lower));
        RANGE_ASSERT(lower == lower_bound_impl(bounds));

        // Trim/infil the beginning if needed
        auto current = lower;
        const auto first_begin = current->first.begin;
        if (bounds.begin > first_begin) {
            // Preserve the portion of lower bound excluded from bounds
            current = split_impl(current, bounds.begin, split_op_keep_lower());
            // Exclude the preserved portion
            ++current;
            RANGE_ASSERT(current == lower_bound_impl(bounds));
        }

        // Loop over completely contained entries and erase them
        while (!at_impl_end(current) && (current->first.end <= bounds.end)) {
            current = impl_erase(current);
        }

        if (!at_impl_end(current) && current->first.includes(bounds.end)) {
            // last entry extends past the end of the bounds range, snip to only erase the bounded section
            current = split_impl(current, bounds.end, split_op_keep_upper());
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
        iterator_impl() : iterator_impl(WrappedIterator()){};
        iterator_impl(const iterator_impl &other) : pos_(other.pos_){};

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
    };

  public:
    using iterator = iterator_impl<value_type, ImplIterator>;
    using const_iterator = iterator_impl<const value_type, ImplConstIterator>;

  protected:
    inline bool at_end(const iterator &it) { return at_impl_end(it.pos_); }
    inline bool at_end(const const_iterator &it) const { return at_impl_end(it.pos_); }
    inline bool at_begin(const iterator &it) { return at_impl_begin(it.pos_); }

    template <typename That, typename Iterator>
    static bool is_contiguous_impl(That *const that, const key_type &range, const Iterator &lower) {
        // Search range or intersection is empty
        if (lower == that->impl_end() || lower->first.excludes(range)) return false;

        if (lower->first.includes(range)) {
            return true;  // there is one entry that contains the whole key range
        }

        bool contiguous = true;
        for (auto pos = lower; contiguous && pos != that->impl_end() && range.includes(pos->first.begin); ++pos) {
            // if current doesn't cover the rest of the key range, check to see that the next is extant and abuts
            if (pos->first.end < range.end) {
                auto next = pos;
                contiguous = (next != that->impl_end()) && pos->first.is_prior_to(next->first);
            }
        }
        return contiguous;
    }

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

    iterator erase_range(const key_type &bounds) {
        auto lower = lower_bound_impl(bounds);

        if (at_impl_end(lower) || !bounds.intersects(lower->first)) {
            // There is nothing in this range lower bound is above bound
            return iterator(lower);
        }
        auto next = impl_erase_range(bounds, lower);
        return iterator(next);
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

    struct insert_range_no_split_bounds {
        const static bool split_boundaries = false;
    };

    struct insert_range_split_bounds {
        const static bool split_boundaries = true;
    };

    // Hint for this insert range is the *lower* bound of the insert as opposed to the upper_bound used
    // as a non-range hint
    template <typename Value, typename Split = insert_range_split_bounds>
    range<iterator> insert_range(const iterator &lower, Value &&value, const Split &split = Split()) {
        const key_type &bounds = value.first;
        // We're not robust to a bad hint, so detect it with extreme prejudice
        // TODO: Add bad hint test to make this robust...
        RANGE_ASSERT(lower == lower_bound(bounds));
        range<ImplIterator> impl_bounds(lower.pos_, lower.pos_);

        if (at_impl_end(impl_bounds.begin) || !bounds.intersects(impl_bounds.begin->first)) {
            // There is nothing in this range lower bound is above bound
            // Generate the needed range (and we're done...)
            impl_bounds.begin = impl_insert(impl_bounds.begin, std::forward<Value>(value));
        } else {
            // Splitting from an occupied range, trim and infill (with value) as needed
            RANGE_ASSERT(impl_bounds.begin->first.intersects(bounds));  // Must construct at the lower boundary of range
            impl_bounds = infill_and_split(bounds, value.second, impl_bounds.begin, Split::split_boundaries);
        }

        return range<iterator>(iterator(impl_bounds.begin), iterator(impl_bounds.end));
    }

    template <typename Value, typename Split = insert_range_split_bounds>
    range<iterator> insert_range(Value &&value, const Split &split = Split()) {
        return insert_range(lower_bound(value.first), value, split);
    }

    iterator lower_bound(const key_type &key) { return iterator(lower_bound_impl(key)); }

    const_iterator lower_bound(const key_type &key) const { return const_iterator(lower_bound_impl(key)); }

    iterator upper_bound(const key_type &key) { return iterator(upper_bound_impl(key)); }

    const_iterator upper_bound(const key_type &key) const { return const_iterator(upper_bound_impl(key)); }

    range<iterator> bounds(const key_type &key) { return {lower_bound(key), upper_bound(key)}; }
    range<const_iterator> cbounds(const key_type &key) const { return {lower_bound(key), upper_bound(key)}; }
    range<const_iterator> bounds(const key_type &key) const { return cbounds(key); }

    using insert_pair = std::pair<iterator, bool>;

    // This is traditional no replacement insert.
    template <typename Value>
    insert_pair insert(Value &&value) {
        const auto &key = value.first;
        if (!key.non_empty()) {
            // It's an invalid key, early bail pointing to end
            return std::make_pair(end(), false);
        }

        // Look for range conflicts (and an insertion point, which makes the lower_bound *not* wasted work)
        // we don't have to check upper if just check that lower doesn't intersect (which it would if lower != upper)
        auto lower = lower_bound_impl(key);
        if (at_impl_end(lower) || !lower->first.intersects(key)) {
            // range is not even paritally overlapped, and lower is strictly > than key
            auto impl_insert = impl_map_.emplace_hint(lower, std::forward<Value>(value));
            // auto impl_insert = impl_map_.emplace(value);
            iterator wrap_it(impl_insert);
            return std::make_pair(wrap_it, true);
        }
        // We don't replace
        return std::make_pair(iterator(lower), false);
    };

    iterator merge_adjacent(const range<iterator> &bounds) {
        if (at_end(bounds.begin)) return bounds.begin;

        auto anchor = bounds.begin;
        while (anchor != bounds.end) {
            RANGE_ASSERT(!at_end(anchor));
            auto current = anchor;
            auto next = current;
            ++next;
            // Walk from anchor to find adjoining ranges that have the same value
            while (next != bounds.end && next->first.is_subsequent_to(current->first) && next->second == anchor->second) {
                current = next;
                ++next;
            }
            if (current != anchor) {
                // the while loop above advanced at least onces, so we have something to merge
                value_type merged = std::make_pair(key_type(anchor->first.begin, current->first.end), std::move(anchor->second));
                next = erase(range<iterator>(anchor, next));
                impl_map_.emplace_hint(next.pos_, merged);
            }
            // Reset the anchor for the next merge search
            anchor = next;
        }
        RANGE_ASSERT(anchor == bounds.end);
        return anchor;
    }

    iterator merge_adjacent(iterator start) { return merge_adjacent(range<iterator>(start, end())); }

    iterator merge_adjacent() { return merge_adjacent(range<iterator>(begin(), end())); }

    template <typename SplitOp>
    iterator split(const iterator whole_it, const index_type &index, const SplitOp &split_op) {
        auto split_it = split_impl(whole_it.pos_, index, split_op);
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
            insert_hint = impl_erase_range(value.first, lower_impl);
        }
        auto inserted = impl_insert(insert_hint, std::forward<Value>(value));
        return iterator(inserted);
    }

    template <typename Value>
    iterator overwrite_range(Value &&value) {
        auto lower = lower_bound(value.first);
        return overwrite_range(lower, value);
    }

    // Need const_iterator/const and iterator/non-const variants using a common implementation
    bool is_contiguous(const key_type &key, const const_iterator &lower) const {
        // We're not robust to a bad lower, so detect it with extreme prejudice
        // TODO: Add bad test to make this robust...
        RANGE_ASSERT(lower == lower_bound(key));
        return is_contiguous_impl(this, lower.pos_);
    }

    bool is_contiguous(const key_type &key, const iterator &lower) {
        // We're not robust to a bad lower, so detect it with extreme prejudice
        // TODO: Add bad lower test to make this robust...
        RANGE_ASSERT(lower == lower_bound(key));
        return is_contiguous_impl(this, key, lower.pos_);
    }

    // we don't need a non-const version of this variant
    bool is_contiguous(const key_type &key) const { return is_contiguous_impl_(this, key, lower_bound(key).pos_); }

    bool empty() const { return impl_map_.empty(); }
    size_t size() const { return impl_map_.size(); }
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
    value_type pos_;

    index_type index_;
    iterator lower_bound_;
    bool valid_;

    bool is_valid() const { return includes(index_); }

    // Allow reuse of a type with const semantics
    void set_value(const index_type &index, const iterator &it) {
        index_ = index;
        lower_bound_ = it;
        valid_ = is_valid();
    }
    void update(const index_type &index) {
        iterator keep_it = lower_bound_;  // by copy set modifies in place
        set_value(index, keep_it);
    }
    inline iterator lower_bound(const index_type &index) { return map_->lower_bound(key_type(index, index + 1)); }
    inline bool at_end(const iterator &it) const { return it == map_->end(); }
    inline bool at_end() const { return at_end(lower_bound_); }

    bool is_lower_than(const index_type &index, const iterator &it) { return at_end(it) || (index < it->first.end); }

  public:
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

    // seek(index) updates lower_bound for index and invalidates the cached lower_bound
    value_type &seek(const index_type &index) {
        set_value(index, lower_bound(index));
        return pos_;
    }

    // Advance the cached location by offset.
    cached_lower_bound_impl &offset(const index_type &offset) {
        const index_type next = index_ + offset;
        if (index_ < next) {  // The needed for overflow or signed indices
            // See if the current or next ranges are the appropriate lower_bound... should be a common use case
            if (is_lower_than(next, lower_bound_)) {
                // lower_bound_ is still the correct lower bound
                update(next);
            } else {
                // Look to see if the next range is the new lower_bound (and we aren't at end)
                auto next_it = lower_bound_;
                ++next_it;
                if (is_lower_than(next, next_it)) {
                    // next_it is the correct new lower bound
                    set_value(next, next_it);
                } else {
                    // We don't know where we are...  and we aren't going to walk the tree looking for next.
                    seek(next);
                }
            }
        } else {
            // General case... this is += so we're not implmenting optimized negative offset logic
            seek(next);
        }
        return *this;
    }

    // invalidate() resets the the lower_bound_ cache, needed after insert/erase/overwrite/split operations
    value_type &invalidate() {
        index_type index = index_;  // copy as set modifies in place.
        return seek(index);
    }

    // Allow a hint for a *valid* lower bound for current index
    // TODO: if the fail-over becomes a hot-spot, the hint logic could be far more clever (looking at previous/next...)
    value_type &invalidate(const iterator &hint) {
        if ((hint != map_->end()) && hint->first.includes(index_)) {
            auto index = index_;  // by copy set modifies in place
            set_value(index, hint);
        } else {
            invalidate();
        }
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

    // Default constructed object reports valid (correctly) as false, but otherwise will fail (assert) under nearly any use.
    cached_lower_bound_impl() : map_(nullptr), pos_(index_, lower_bound_, valid_), index_(0), lower_bound_(), valid_(false) {}
    cached_lower_bound_impl(Map &map, const index_type &index) : map_(&map), pos_(index_, lower_bound_, valid_) { seek(index); }
};

template <typename CachedLowerBound, typename MappedType = typename CachedLowerBound::mapped_type>
const MappedType &evaluate(const CachedLowerBound &clb, const MappedType &default_value) {
    if (clb->valid) {
        return clb->lower_bound->second;
    }
    return default_value;
}

// Parallel iterator
// Traverse to range maps over the the same range, but without assumptions of aligned ranges.
// ++ increments to the next point where on of the two maps changes range, giving a range over which the two
// maps do not transition ranges
template <typename MapA, typename MapB, typename KeyType = typename MapA::key_type>
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
        pos_A_.seek(static_cast<index_type_A>(index));
        range_ = key_type(index, index + compute_delta());
        return *this;
    }
    parallel_iterator &invalidate_B() {
        const index_type index = range_.begin;
        pos_B_.seek(static_cast<index_type_B>(index));
        range_ = key_type(index, index + compute_delta());
        return *this;
    }

    // The return is const because we are sharing the internal state directly.
    const value_type &operator*() const { return pos_; }
    const value_type *operator->() const { return &pos_; }
};

enum class splice_precedence { prefer_source, prefer_dest };

template <typename RangeMap, typename SourceIterator = typename RangeMap::const_iterator>
bool splice(RangeMap *to, const RangeMap &from, splice_precedence arbiter, SourceIterator begin, SourceIterator end) {
    if (from.empty() || (begin == end) || (begin == from.cend())) return false;  // nothing to merge.

    using ParallelIterator = parallel_iterator<RangeMap, const RangeMap>;
    using Key = typename RangeMap::key_type;
    using CachedLowerBound = cached_lower_bound_impl<RangeMap>;
    using ConstCachedLowerBound = cached_lower_bound_impl<const RangeMap>;
    using NoSplit = typename RangeMap::insert_range_no_split_bounds;
    ParallelIterator par_it(*to, from, begin->first.begin);
    bool updated = false;
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
                // Only rewrite this range if source is preferred (and the value differs)
                // TODO determine if equality checks are always wanted. (for example heavyweight values)
                if (arbiter == splice_precedence::prefer_source && (write_it->second != read_it->second)) {
                    // Both ranges occupied and source is preferred and from differs from to
                    if (write_it->first == range) {
                        // we're writing the whole destination range, so just set the value
                        write_it->second = read_it->second;
                    } else {
                        to->overwrite_range(write_it, std::make_pair(range, read_it->second));
                        par_it.invalidate_A();  // we've changed map 'to' behind to_lb's back... let it know.
                    }
                    updated = true;
                }
            } else {
                // Insert into the gap.
                to->insert_range(write_it, std::make_pair(range, read_it->second), NoSplit());
                par_it.invalidate_A();  // we've changed map 'to' behind to_lb's back... let it know.
                updated = true;
            }
        }
        ++par_it;  // next range over which both 'to' and 'from' stay constant
    }
    return updated;
}
// And short hand for "from begin to end"
template <typename RangeMap>
bool splice(RangeMap *to, const RangeMap &from, splice_precedence arbiter) {
    return splice(to, from, arbiter, from.cbegin(), from.cend());
}

}  // namespace sparse_container

#endif
