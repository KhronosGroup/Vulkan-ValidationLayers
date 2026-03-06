/* Copyright (c) 2026 The Khronos Group Inc.
 * Copyright (c) 2026 Valve Corporation
 * Copyright (c) 2026 LunarG, Inc.
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

#include "containers/array_range_map.h"
#include "containers/range_map.h"
#include <variant>

namespace sparse_container {

using IndexType = uint64_t;

// Adds "small map optimization" to  vvl::range_map.
// Note that N must be < uint8_t max
template <typename T, size_t N>
class SmallRangeMap {
    using ArrayMap = array_range_map<IndexType, T, vvl::range<IndexType>, N>;
    using ArrayMapIterator = typename ArrayMap::iterator;
    using ArrayMapConstIterator = typename ArrayMap::const_iterator;

    using RangeMap = range_map<IndexType, T>;
    using RangeMapIterator = typename RangeMap::iterator;
    using RangeMapConstIterator = typename RangeMap::const_iterator;

  public:
    using index_type = IndexType;
    using key_type = vvl::range<IndexType>;
    using mapped_type = T;
    using value_type = std::pair<const key_type, mapped_type>;

    template <typename Value, typename ArrayIt, typename RangeIt>
    class IteratorImpl {
      public:
        Value* operator->() const {
            if (is_array_it_) {
                return array_it_.operator->();
            } else {
                return range_it_.operator->();
            }
        }
        Value& operator*() const {
            if (is_array_it_) {
                return array_it_.operator*();
            } else {
                return range_it_.operator*();
            }
        }
        IteratorImpl& operator++() {
            if (is_array_it_) {
                array_it_.operator++();
            } else {
                range_it_.operator++();
            }
            return *this;
        }
        IteratorImpl& operator--() {
            if (is_array_it_) {
                array_it_.operator--();
            } else {
                range_it_.operator--();
            }
            return *this;
        }
        IteratorImpl& operator=(const IteratorImpl& other) {
            is_array_it_ = other.is_array_it_;
            array_it_ = other.array_it_;
            range_it_ = other.range_it_;
            return *this;
        }
        bool operator==(const IteratorImpl& other) const {
            // It's enough just to compare both iterators.
            return array_it_ == other.array_it_ && range_it_ == other.range_it_;
        }
        bool operator!=(const IteratorImpl& other) const { return !(*this == other); }

        IteratorImpl() = default;
        IteratorImpl(const IteratorImpl& other) = default;
        IteratorImpl(const ArrayIt& it) : is_array_it_(true), array_it_(it) {}
        IteratorImpl(const RangeIt& it) : is_array_it_(false), range_it_(it) {}

      private:
        friend SmallRangeMap;

        bool is_array_it_ = false;
        ArrayIt array_it_;
        RangeIt range_it_;
    };

    using iterator = IteratorImpl<value_type, ArrayMapIterator, RangeMapIterator>;
    // TODO change const iterator to derived class if iterator -> const_iterator constructor is needed
    using const_iterator = IteratorImpl<const value_type, ArrayMapConstIterator, RangeMapConstIterator>;

    iterator begin() {
        if (UsesArrayMap()) {
            return iterator(GetArrayMap().begin());
        } else {
            return iterator(GetRangeMap().begin());
        }
    }
    const_iterator cbegin() const {
        if (UsesArrayMap()) {
            return const_iterator(GetArrayMap().begin());
        } else {
            return const_iterator(GetRangeMap().begin());
        }
    }
    const_iterator begin() const { return cbegin(); }

    iterator end() {
        if (UsesArrayMap()) {
            return iterator(GetArrayMap().end());
        } else {
            return iterator(GetRangeMap().end());
        }
    }
    const_iterator cend() const {
        if (UsesArrayMap()) {
            return const_iterator(GetArrayMap().end());
        } else {
            return const_iterator(GetRangeMap().end());
        }
    }
    const_iterator end() const { return cend(); }

    iterator find(const key_type& key) {
        if (UsesArrayMap()) {
            return iterator(GetArrayMap().find(key));
        } else {
            return iterator(GetRangeMap().find(key));
        }
    }

    const_iterator find(const key_type& key) const {
        if (UsesArrayMap()) {
            return const_iterator(GetArrayMap().find(key));
        } else {
            return const_iterator(GetRangeMap().find(key));
        }
    }

    iterator find(const index_type& index) {
        if (UsesArrayMap()) {
            return iterator(GetArrayMap().find(index));
        } else {
            return iterator(GetRangeMap().find(index));
        }
    }

    const_iterator find(const index_type& index) const {
        if (UsesArrayMap()) {
            return const_iterator(GetArrayMap().find(index));
        } else {
            return const_iterator(GetRangeMap().find(index));
        }
    }

    // TODO -- this is supposed to be a const_iterator, which is constructable from an iterator
    void insert(const iterator& hint, const value_type& value) {
        if (UsesArrayMap()) {
            assert(hint.is_array_it_);
            GetArrayMap().insert(hint.array_it_, value);
        } else {
            assert(!hint.is_array_it_);
            GetRangeMap().insert(hint.range_it_, value);
        }
    }

    iterator lower_bound(const key_type& key) {
        if (UsesArrayMap()) {
            return iterator(GetArrayMap().lower_bound(key));
        } else {
            return iterator(GetRangeMap().lower_bound(key));
        }
    }

    const_iterator lower_bound(const key_type& key) const {
        if (UsesArrayMap()) {
            return const_iterator(GetArrayMap().lower_bound(key));
        } else {
            return const_iterator(GetRangeMap().lower_bound(key));
        }
    }

    template <typename Value>
    iterator overwrite_range(const iterator& lower, Value&& value) {
        if (UsesArrayMap()) {
            assert(lower.is_array_it_);
            return GetArrayMap().overwrite_range(lower.array_it_, std::forward<Value>(value));
        } else {
            assert(!lower.is_array_it_);
            return GetRangeMap().overwrite_range(lower.range_it_, std::forward<Value>(value));
        }
    }

    // With power comes responsibility (🕷).  You can get to the underlying maps, s.t. in inner loops, the "SmallMode" checks can be
    // avoided per call, just be sure and Get the correct one.
    const ArrayMap& GetArrayMap() const {
        assert(UsesArrayMap());
        return std::get<ArrayMap>(map_);
    }
    ArrayMap& GetArrayMap() {
        assert(UsesArrayMap());
        return std::get<ArrayMap>(map_);
    }
    const RangeMap& GetRangeMap() const {
        assert(!UsesArrayMap());
        return std::get<RangeMap>(map_);
    }
    RangeMap& GetRangeMap() {
        assert(!UsesArrayMap());
        return std::get<RangeMap>(map_);
    }

    SmallRangeMap() = delete;

    SmallRangeMap(index_type limit) {
        if (limit <= N) {
            map_ = ArrayMap(limit);
        } else {
            map_ = RangeMap();
        }
    }

    bool empty() const {
        if (UsesArrayMap()) {
            return GetArrayMap().empty();
        } else {
            return GetRangeMap().empty();
        }
    }

    size_t size() const {
        if (UsesArrayMap()) {
            return GetArrayMap().size();
        } else {
            return GetRangeMap().size();
        }
    }

    bool UsesArrayMap() const { return std::holds_alternative<ArrayMap>(map_); }

  private:
    std::variant<ArrayMap, RangeMap> map_;
};

}  // namespace sparse_container
