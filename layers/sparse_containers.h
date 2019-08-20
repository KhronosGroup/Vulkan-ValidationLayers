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
#ifndef SPARSE_CONTAINERS_H_
#define SPARSE_CONTAINERS_H_
#define NOMINMAX
#include <cassert>
#include <memory>
#include <unordered_map>
#include <vector>

namespace sparse_container {
// SparseVector:
//
// Defines a sparse single-dimensional container which is targeted for three distinct use cases
// 1) Large range of indices sparsely populated ("Sparse access" below)
// 2) Large range of indices where all values are the same ("Sparse access" below)
// 3) Large range of values densely populated (more that 1/4 full) ("Dense access" below)
// 4) Small range of values where direct access is most efficient ("Dense access" below)
//
// To update semantics are supported bases on kSetReplaces:
//    true -- updates to already set (valid) indices replace current value
//    false -- updates to already set (valid) indicies are ignored.
//
// Theory of operation:
//
// When created, a sparse vector is created (based on size relative to
// the kSparseThreshold) in either Sparse or Dense access mode.
//
// In "Sparse access" mode individual values are stored in a map keyed
// by the index.  A "full range" value (if set) defines the value of all
// entries not present in the map. Setting a full range value via
//
//     SetRange(range_min, range_max, full_range_value )
//
// either clears the map (kSetReplaces==true) or prevents further
// updates to the vector (kSetReplaces==false).  If the map becomes
// more than // 1/kConversionThreshold (4) full, the SparseVector is
// converted into "Dense access" mode.  Entries are copied from map,
// with non-present indices set to the default value (kDefaultValue)
// or the full range value (if present).
//
// In "Dense access" mode, values are  stored in a vector the size of
// the valid range indexed by the incoming index value minus range_min_.
// The same upate semantic applies bases on kSetReplaces.
//
// Note that when kSparseThreshold
//
// Access:
//
// NOTE all "end" indices (in construction or access) are *exclusive*.
//
// Given the variable semantics and effective compression of Sparse
// access mode, all access is through Get, Set, and SetRange functions
// and a constant iterator. Get return either the value found (using
// the current access mode) or the kDefaultValue.  Set and SetRange
// return whether or not state was updated, in order to support dirty
// bit updates for any dependent state.
//
// The iterator ConstIterator provides basic, "by value" access. The
// "by value" nature of the access reflect the compressed nature
// operators *, ++, ==, and != are provided, with the latter two only
// suitable for comparisons vs. cend.  The iterator skips all
// kDefaultValue entries in either access mode, returning a std::pair
// containing {IndexType, ValueType}. The multiple access modes give
// the iterator a bit more complexity than is optimal, but hides the
// underlying complexity from the callers.
//
// TODO: Update iterator to use a reference (likely using
// reference_wrapper...)

template <typename IndexType_, typename T, bool kSetReplaces, T kDefaultValue = T(), size_t kSparseThreshold = 16>
class SparseVector {
   public:
    typedef IndexType_ IndexType;
    typedef T value_type;
    typedef value_type ValueType;
    typedef std::unordered_map<IndexType, ValueType> SparseType;
    typedef std::vector<ValueType> DenseType;

    SparseVector(IndexType start, IndexType end)
        : range_min_(start), range_max_(end), threshold_((end - start) / kConversionThreshold) {
        assert(end > start);
        Reset();
    }

    // Initial access mode is set based on range size vs. kSparseThreshold.  Either sparse_ or dense_ is always set, but only
    // ever one at a time
    void Reset() {
        has_full_range_value_ = false;
        full_range_value_ = kDefaultValue;
        size_t count = range_max_ - range_min_;
        if (kSparseThreshold && (count > kSparseThreshold)) {
            sparse_.reset(new SparseType());
            dense_.reset();
        } else {
            sparse_.reset();
            dense_.reset(new DenseType(count, kDefaultValue));
        }
    }

    const ValueType &Get(const IndexType index) const {
        // Note that here (and similarly below, the 'IsSparse' clause is
        // eliminated as dead code in release builds if kSparseThreshold==0
        if (IsSparse()) {
            if (!sparse_->empty()) {  // Don't attempt lookup in empty map
                auto it = sparse_->find(index);
                if (it != sparse_->cend()) {
                    return it->second;
                }
            }
            // If there is a full_range_value, return it, but there isn't a full_range_value_, it's set to  kDefaultValue
            // so it's still the correct this to return
            return full_range_value_;
        } else {
            // Direct access
            assert(dense_.get());
            const ValueType &value = (*dense_)[index - range_min_];
            return value;
        }
    }

    // Set a indexes value, based on the access mode, update semantics are enforced within the access mode specific function
    bool Set(const IndexType index, const ValueType &value) {
        bool updated = false;
        if (IsSparse()) {
            updated = SetSparse(index, value);
        } else {
            assert(dense_.get());
            updated = SetDense(index, value);
        }
        return updated;
    }

    // Set a range of values based on access mode, with some update semantics applied a the range level
    bool SetRange(const IndexType start, IndexType end, ValueType value) {
        bool updated = false;
        if (IsSparse()) {
            if (!kSetReplaces && HasFullRange()) return false;  // We have full coverage, we can change this no more

            bool is_full_range = IsFullRange(start, end);
            if (kSetReplaces && is_full_range) {
                updated = value != full_range_value_;
                full_range_value_ = value;
                if (HasSparseSubranges()) {
                    updated = true;
                    sparse_->clear();  // full range replaces all subranges
                }
                // has_full_range_value_ state of the full_range_value_ to avoid ValueType comparisons
                has_full_range_value_ = value != kDefaultValue;
            } else if (!kSetReplaces && (value != kDefaultValue) && is_full_range && !HasFullRange()) {
                // With the update only invalid semantics, the value becomes the fallback, and will prevent other updates
                full_range_value_ = value;
                has_full_range_value_ = true;
                updated = true;
                // Clean up the sparse map a bit
                for (auto it = sparse_->begin(); it != sparse_->end();) {  // no increment clause because of erase below
                    if (it->second == value) {
                        it = sparse_->erase(it);  // remove redundant entries
                    } else {
                        ++it;
                    }
                }
            } else {
                for (IndexType index = start; index < end; ++index) {
                    // NOTE: We can't use SetSparse here, because this may be converted to dense access mid update
                    updated |= Set(index, value);
                }
            }
        } else {
            // Note that "Dense Access" does away with the full_range_value_ logic, storing empty entries using kDefaultValue
            assert(dense_);
            for (IndexType index = start; index < end; ++index) {
                updated = SetDense(index, value);
            }
        }
        return updated;
    }

    // Set only the non-default values from another sparse vector
    bool Merge(const SparseVector &from) {
        // Must not set from Sparse arracy with larger bounds...
        assert((range_min_ <= from.range_min_) && (range_max_ >= from.range_max_));
        bool updated = false;
        if (from.IsSparse()) {
            if (from.HasFullRange() && !from.HasSparseSubranges()) {
                // Short cut to copy a full range if that's all we have
                updated |= SetRange(from.range_min_, from.range_max_, from.full_range_value_);
            } else {
                // Have to do it the complete (potentially) slow way
                // TODO add sorted keys to iterator to reduce hash lookups
                for (auto it = from.cbegin(); it != from.cend(); ++it) {
                    const IndexType index = (*it).first;
                    const ValueType &value = (*it).second;
                    Set(index, value);
                }
            }
        } else {
            assert(from.dense_);
            DenseType &ray = *from.dense_;
            for (IndexType entry = from.range_min_; entry < from.range_max_; ++entry) {
                IndexType index = entry - from.range_min_;
                if (ray[index] != kDefaultValue) {
                    updated |= Set(entry, ray[index]);
                }
            }
        }
        return updated;
    }

    friend class ConstIterator;
    class ConstIterator {
       public:
        using SparseType = typename SparseVector::SparseType;
        using SparseIterator = typename SparseType::const_iterator;
        using IndexType = typename SparseVector::IndexType;
        using ValueType = typename SparseVector::ValueType;
        using IteratorValueType = std::pair<IndexType, ValueType>;
        const IteratorValueType &operator*() const { return current_value_; }

        ConstIterator &operator++() {
            if (delegated_) {  // implies sparse
                ++it_sparse_;
                if (it_sparse_ == vec_->sparse_->cend()) {
                    the_end_ = true;
                    current_value_.first = vec_->range_max_;
                    current_value_.second = SparseVector::DefaultValue();
                } else {
                    current_value_.first = it_sparse_->first;
                    current_value_.second = it_sparse_->second;
                }
            } else {
                index_++;
                SetCurrentValue();
            }
            return *this;
        }
        bool operator!=(const ConstIterator &rhs) const {
            return (the_end_ != rhs.the_end_);  // Just good enough for cend checks
        }

        bool operator==(const ConstIterator &rhs) const {
            return (the_end_ == rhs.the_end_);  // Just good enough for cend checks
        }

        // The iterator has two modes:
        //     delegated:
        //         where we are in sparse access mode and have no full_range_value
        //         and thus can delegate our iteration to underlying map
        //     non-delegated:
        //         either dense mode or we have a full range value and thus
        //         must iterate over the whole range
        ConstIterator(const SparseVector &vec) : vec_(&vec) {
            if (!vec_->IsSparse() || vec_->HasFullRange()) {
                // Must iterated over entire ranges skipping (in the case of dense access), invalid entries
                delegated_ = false;
                index_ = vec_->range_min_;
                SetCurrentValue();  // Skips invalid and sets the_end_
            } else if (vec_->HasSparseSubranges()) {
                // The subranges store the non-default values... and their is no full range value
                delegated_ = true;
                it_sparse_ = vec_->sparse_->cbegin();
                current_value_.first = it_sparse_->first;
                current_value_.second = it_sparse_->second;
                the_end_ = false;  // the sparse map is non-empty (per HasSparseSubranges() above)
            } else {
                // Sparse, but with no subranges
                the_end_ = true;
            }
        }

        ConstIterator() : vec_(nullptr), the_end_(true) {}

       protected:
        const SparseVector *vec_;
        bool the_end_;
        SparseIterator it_sparse_;
        bool delegated_;
        IndexType index_;
        ValueType value_;

        IteratorValueType current_value_;

        // in the non-delegated case we use normal accessors and skip default values.
        void SetCurrentValue() {
            the_end_ = true;
            while (index_ < vec_->range_max_) {
                value_ = vec_->Get(index_);
                if (value_ != SparseVector::DefaultValue()) {
                    the_end_ = false;
                    current_value_ = IteratorValueType(index_, value_);
                    break;
                }
                index_++;
            }
        }
    };
    typedef ConstIterator const_iterator;

    ConstIterator cbegin() const { return ConstIterator(*this); }
    ConstIterator cend() const { return ConstIterator(); }

    IndexType RangeMax() const { return range_max_; }
    IndexType RangeMin() const { return range_min_; }

    static const unsigned kConversionThreshold = 4;
    const IndexType range_min_;  // exclusive
    const IndexType range_max_;  // exclusive
    const IndexType threshold_;  // exclusive

    // Data for sparse mode
    // We have a short cut for full range values when in sparse mode
    bool has_full_range_value_;
    ValueType full_range_value_;
    std::unique_ptr<SparseType> sparse_;

    // Data for dense mode
    std::unique_ptr<DenseType> dense_;

    static const ValueType &DefaultValue() {
        static ValueType value = kDefaultValue;
        return value;
    }
    // Note that IsSparse is compile-time reducible if kSparseThreshold is zero...
    inline bool IsSparse() const { return kSparseThreshold && sparse_.get(); }
    bool IsFullRange(IndexType start, IndexType end) const { return (start == range_min_) && (end == range_max_); }
    bool IsFullRangeValue(const ValueType &value) const { return has_full_range_value_ && (value == full_range_value_); }
    bool HasFullRange() const { return IsSparse() && has_full_range_value_; }
    bool HasSparseSubranges() const { return IsSparse() && !sparse_->empty(); }

    // This is called unconditionally, to encapsulate the conversion criteria and logic here
    void SparseToDenseConversion() {
        // If we're using more threshold of the sparse range, convert to dense_
        if (IsSparse() && (sparse_->size() > threshold_)) {
            ValueType default_value = HasFullRange() ? full_range_value_ : kDefaultValue;
            dense_.reset(new DenseType((range_max_ - range_min_), default_value));
            DenseType &ray = *dense_;
            for (auto const &item : *sparse_) {
                ray[item.first - range_min_] = item.second;
            }
            sparse_.reset();
            has_full_range_value_ = false;
        }
    }

    // Dense access mode setter with update semantics implemented
    bool SetDense(IndexType index, const ValueType &value) {
        bool updated = false;
        ValueType &current_value = (*dense_)[index - range_min_];
        if ((kSetReplaces || current_value == kDefaultValue) && (value != current_value)) {
            current_value = value;
            updated = true;
        }
        return updated;
    }

    // Sparse access mode setter with update full range and update semantics implemented
    bool SetSparse(IndexType index, const ValueType &value) {
        if (!kSetReplaces && HasFullRange()) {
            return false;  // We have full coverage, we can change this no more
        }

        if (kSetReplaces && IsFullRangeValue(value) && HasSparseSubranges()) {
            auto erasure = sparse_->erase(index);  // Remove duplicate record from map
            return erasure > 0;
        }

        // Use insert to reduce the number of hash lookups
        auto map_pair = std::make_pair(index, value);
        auto insert_pair = sparse_->insert(map_pair);
        auto &it = insert_pair.first;  // use references to avoid nested pair accesses
        const bool inserted = insert_pair.second;
        bool updated = false;
        if (inserted) {
            updated = true;
            SparseToDenseConversion();
        } else if (kSetReplaces && value != it->second) {
            // Only replace value if semantics allow it and it has changed.
            it->second = value;
            updated = true;
        }
        return updated;
    }
};

}  // namespace sparse_container
#endif
