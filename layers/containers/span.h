/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
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
#include <cstddef>
#include <type_traits>
#include <utility>

namespace vvl {
// Partial implementation of std::span for C++11
template <typename T, typename Iterator>
class enumeration {
  public:
    using pointer = T *;
    using const_pointer = T const *;
    using iterator = Iterator;
    using const_iterator = const Iterator;

    enumeration() = default;
    enumeration(pointer start, size_t n) : data_(start), count_(n) {}
    template <typename Position>
    enumeration(Position start, Position end) : data_(&(*start)), count_(end - start) {}
    template <typename Container>
    enumeration(Container &c) : data_(c.data()), count_(c.size()) {}

    iterator begin() { return data_; }
    const_iterator begin() const { return data_; }

    iterator end() { return data_ + count_; }
    const_iterator end() const { return data_ + count_; }

    T &operator[](size_t i) { return data_[i]; }
    const T &operator[](size_t i) const { return data_[i]; }

    T &front() { return *data_; }
    const T &front() const { return *data_; }

    T &back() { return *(data_ + (count_ - 1)); }
    const T &back() const { return *(data_ + (count_ - 1)); }

    size_t size() const { return count_; }
    bool empty() const { return count_ == 0; }

    pointer data() const { return data_; }

  private:
    pointer data_ = {};
    size_t count_ = 0;
};

template <typename T, typename IndexType = size_t>
class IndexedIterator {
  public:
    IndexedIterator(T *data, IndexType index = 0) : index_(index), data_(data) {}

    std::pair<IndexType, T &> operator*() { return std::pair<IndexType, T &>(index_, *data_); }
    std::pair<IndexType, T> operator*() const { return std::make_pair(index_, *data_); }

    // prefix increment
    IndexedIterator<T, IndexType> &operator++() {
        ++data_;
        ++index_;
        return *this;
    }

    // postfix increment
    IndexedIterator<T, IndexType> operator++(int) {
        IndexedIterator<T, IndexType> old = *this;
        operator++();
        return old;
    }

    bool operator==(const IndexedIterator<T, IndexType> &rhs) const {
        // No need to compare indices, just compare pointers
        // And given the implementation of enumeration::end(),
        // where no index is given when constructing an iterator,
        // index_ will default to 0 for end(), which is wrong, but we can live with it for now
        return data_ == rhs.data_;
    }
    bool operator!=(const IndexedIterator<T, IndexType> &rhs) const { return data_ != rhs.data_; }

  public:
    IndexType index_ = 0;
    T *data_;
};

template <typename T>
using span = enumeration<T, T *>;

//
// Allow type inference that using the constructor doesn't allow in C++11
template <typename T>
span<T> make_span(T *begin, size_t count) {
    return span<T>(begin, count);
}
template <typename T>
span<T> make_span(T *begin, T *end) {
    return make_span<T>(begin, end);
}

template <typename T, typename IndexType>
auto enumerate(T *begin, IndexType count) {
    return enumeration<T, IndexedIterator<T, IndexType>>(begin, count);
}
template <typename T>
auto enumerate(T *begin, T *end) {
    return enumeration<T, IndexedIterator<T>>(begin, end);
}

template <typename Container>
auto enumerate(Container &container) {
    return enumeration<typename Container::value_type,
                       IndexedIterator<typename Container::value_type, typename Container::size_type>>(container.data(),
                                                                                                       container.size());
}

template <typename Container>
auto enumerate(const Container &container) {
    return enumeration<std::add_const_t<typename Container::value_type>,
                       IndexedIterator<std::add_const_t<typename Container::value_type>, typename Container::size_type>>(
        container.data(), container.size());
}
}  // namespace vvl