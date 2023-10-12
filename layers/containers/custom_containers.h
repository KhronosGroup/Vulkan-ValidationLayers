/* Copyright (c) 2015-2017, 2019-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2017, 2019-2023 Valve Corporation
 * Copyright (c) 2015-2017, 2019-2023 LunarG, Inc.
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

#include <cmath>

#include <cassert>
#include <limits>
#include <memory>
#include <map>
#include <unordered_map>
#include <set>
#include <algorithm>
#include <iterator>
#include <type_traits>
#include <optional>
#include <utility>

#ifdef USE_ROBIN_HOOD_HASHING
#include "robin_hood.h"
#else
#include <unordered_set>
#endif

// namespace aliases to allow map and set implementations to easily be swapped out
namespace vvl {

#ifdef USE_ROBIN_HOOD_HASHING
template <typename T>
using hash = robin_hood::hash<T>;

template <typename Key, typename Hash = robin_hood::hash<Key>, typename KeyEqual = std::equal_to<Key>>
using unordered_set = robin_hood::unordered_set<Key, Hash, KeyEqual>;

template <typename Key, typename T, typename Hash = robin_hood::hash<Key>, typename KeyEqual = std::equal_to<Key>>
using unordered_map = robin_hood::unordered_map<Key, T, Hash, KeyEqual>;

template <typename Key, typename T>
using map_entry = robin_hood::pair<Key, T>;

// robin_hood-compatible insert_iterator (std:: uses the wrong insert method)
// NOTE: std::iterator was deprecated in C++17, and newer versions of libstdc++ appear to mark this as such.
template <typename T>
struct insert_iterator {
    using iterator_category = std::output_iterator_tag;
    using value_type = typename T::value_type;
    using iterator = typename T::iterator;
    using difference_type = void;
    using pointer = void;
    using reference = T &;

    insert_iterator(reference t, iterator i) : container(&t), iter(i) {}

    insert_iterator &operator=(const value_type &value) {
        auto result = container->insert(value);
        iter = result.first;
        ++iter;
        return *this;
    }

    insert_iterator &operator=(value_type &&value) {
        auto result = container->insert(std::move(value));
        iter = result.first;
        ++iter;
        return *this;
    }

    insert_iterator &operator*() { return *this; }

    insert_iterator &operator++() { return *this; }

    insert_iterator &operator++(int) { return *this; }

  private:
    T *container;
    iterator iter;
};
#else
template <typename T>
using hash = std::hash<T>;

template <typename Key, typename Hash = std::hash<Key>, typename KeyEqual = std::equal_to<Key>>
using unordered_set = std::unordered_set<Key, Hash, KeyEqual>;

template <typename Key, typename T, typename Hash = std::hash<Key>, typename KeyEqual = std::equal_to<Key>>
using unordered_map = std::unordered_map<Key, T, Hash, KeyEqual>;

template <typename Key, typename T>
using map_entry = std::pair<Key, T>;

template <typename T>
using insert_iterator = std::insert_iterator<T>;
#endif

}  // namespace vvl

// A vector class with "small string optimization" -- meaning that the class contains a fixed working store for N elements.
// Useful in in situations where the needed size is unknown, but the typical size is known  If size increases beyond the
// fixed capacity, a dynamically allocated working store is created.
//
// NOTE: Unlike std::vector which only requires T to be CopyAssignable and CopyConstructable, small_vector requires T to be
//       MoveAssignable and MoveConstructable
// NOTE: Unlike std::vector, iterators are invalidated by move assignment between small_vector objects effectively the
//       "small string" allocation functions as an incompatible allocator.
template <typename T, size_t N, typename SizeType = uint8_t>
class small_vector {
  public:
    using value_type = T;
    using reference = value_type &;
    using const_reference = const value_type &;
    using pointer = value_type *;
    using const_pointer = const value_type *;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using size_type = SizeType;
    static const size_type kSmallCapacity = N;
    static const size_type kMaxCapacity = std::numeric_limits<size_type>::max();
    static_assert(N <= kMaxCapacity, "size must be less than size_type::max");

    small_vector() : size_(0), capacity_(N), working_store_(GetSmallStore()) {}

    small_vector(std::initializer_list<T> list) : size_(0), capacity_(N), working_store_(GetSmallStore()) { PushBackFrom(list); }

    small_vector(const small_vector &other) : size_(0), capacity_(N), working_store_(GetSmallStore()) { PushBackFrom(other); }

    small_vector(small_vector &&other) : size_(0), capacity_(N), working_store_(GetSmallStore()) {
        if (other.large_store_) {
            MoveLargeStore(other);
        } else {
            PushBackFrom(std::move(other));
        }
        // Per the spec, when constructing from other, other is guaranteed to be empty after the constructor runs
        other.clear();
    }

    small_vector(size_type size, const value_type &value = value_type()) : size_(0), capacity_(N), working_store_(GetSmallStore()) {
        reserve(size);
        auto dest = GetWorkingStore();
        for (size_type i = 0; i < size; i++) {
            new (dest) value_type(value);
            ++dest;
        }
        size_ = size;
    }

    ~small_vector() { clear(); }

    bool operator==(const small_vector &rhs) const {
        if (size_ != rhs.size_) return false;
        auto value = begin();
        for (const auto &rh_value : rhs) {
            if (!(*value == rh_value)) {
                return false;
            }
            ++value;
        }
        return true;
    }

    bool operator!=(const small_vector &rhs) const { return !(*this == rhs); }

    small_vector &operator=(const small_vector &other) {
        if (this != &other) {
            if (other.size_ > capacity_) {
                // Calling reserve would move construct and destroy all current contents, so just clear them before calling
                // PushBackFrom (which does a reserve vs. the now empty this)
                clear();
                PushBackFrom(other);
            } else {
                // The copy will fit into the current allocation
                auto dest = GetWorkingStore();
                auto source = other.GetWorkingStore();

                const auto overlap = std::min(size_, other.size_);
                // Copy assign anywhere we have objects in this
                // Note: usually cheaper than destruct/construct
                for (size_type i = 0; i < overlap; i++) {
                    dest[i] = source[i];
                }

                // Copy construct anywhere we *don't* have objects in this
                for (size_type i = overlap; i < other.size_; i++) {
                    new (dest + i) value_type(source[i]);
                }

                // Any entries in this past other_size_ must be cleaned up...
                for (size_type i = other.size_; i < size_; i++) {
                    dest[i].~value_type();
                }
                size_ = other.size_;
            }
        }
        return *this;
    }

    small_vector &operator=(small_vector &&other) {
        if (this != &other) {
            // Note: move assign doesn't require other to become empty (as does move construction)
            //       so we'll leave other alone except in the large store case, while moving the object
            //       *in* the vector from other
            if (other.large_store_) {
                // Moving the other large store intact is probably best, even if we have to destroy everything in this.
                clear();
                MoveLargeStore(other);
            } else if (other.size_ > capacity_) {
                // If we'd have to reallocate, just clean up minimally and copy normally
                clear();
                PushBackFrom(std::move(other));
            } else {
                // The copy will fit into the current allocation
                auto dest = GetWorkingStore();
                auto source = other.GetWorkingStore();

                const auto overlap = std::min(size_, other.size_);

                // Move assign where we have objects in this
                // Note: usually cheaper than destruct/construct
                for (size_type i = 0; i < overlap; i++) {
                    dest[i] = std::move(source[i]);
                }

                // Move construct where we *don't* have objects in this
                for (size_type i = overlap; i < other.size_; i++) {
                    new (dest + i) value_type(std::move(source[i]));
                }

                // Any entries in this past other_size_ must be cleaned up...
                for (size_type i = other.size_; i < size_; i++) {
                    dest[i].~value_type();
                }
                size_ = other.size_;
            }
        }
        return *this;
    }

    reference operator[](size_type pos) {
        assert(pos < size_);
        return GetWorkingStore()[pos];
    }
    const_reference operator[](size_type pos) const {
        assert(pos < size_);
        return GetWorkingStore()[pos];
    }

    // Like std::vector:: calling front or back on an empty container causes undefined behavior
    reference front() {
        assert(size_ > 0);
        return GetWorkingStore()[0];
    }
    const_reference front() const {
        assert(size_ > 0);
        return GetWorkingStore()[0];
    }
    reference back() {
        assert(size_ > 0);
        return GetWorkingStore()[size_ - 1];
    }
    const_reference back() const {
        assert(size_ > 0);
        return GetWorkingStore()[size_ - 1];
    }

    bool empty() const { return size_ == 0; }

    template <class... Args>
    void emplace_back(Args &&...args) {
        assert(size_ < kMaxCapacity);
        reserve(size_ + 1);
        new (GetWorkingStore() + size_) value_type(args...);
        size_++;
    }

    // Note: probably should update this to reflect C++23 ranges
    template <typename Container>
    void PushBackFrom(const Container &from) {
        assert(from.size() <= kMaxCapacity);
        assert(size_ <= kMaxCapacity - from.size());
        const size_type new_size = size_ + static_cast<size_type>(from.size());
        reserve(new_size);

        auto dest = GetWorkingStore() + size_;
        for (const auto &element : from) {
            new (dest) value_type(element);
            ++dest;
        }
        size_ = new_size;
    }

    template <typename Container>
    void PushBackFrom(Container &&from) {
        assert(from.size() < kMaxCapacity);
        const size_type new_size = size_ + static_cast<size_type>(from.size());
        reserve(new_size);

        auto dest = GetWorkingStore() + size_;
        for (auto &element : from) {
            new (dest) value_type(std::move(element));
            ++dest;
        }
        size_ = new_size;
    }

    void reserve(size_type new_cap) {
        // Since this can't shrink, if we're growing we're newing
        if (new_cap > capacity_) {
            assert(capacity_ >= kSmallCapacity);
            auto new_store = std::unique_ptr<BackingStore[]>(new BackingStore[new_cap]);
            auto working_store = GetWorkingStore();
            for (size_type i = 0; i < size_; i++) {
                new (new_store[i].data) value_type(std::move(working_store[i]));
                working_store[i].~value_type();
            }
            large_store_ = std::move(new_store);
            assert(new_cap > kSmallCapacity);
            capacity_ = new_cap;
        }
        UpdateWorkingStore();
        // No shrink here.
    }

    void clear() {
        // Keep clear minimal to optimize reset functions for enduring objects
        // more work is deferred until destruction (freeing of large_store for example)
        // and we intentionally *aren't* shrinking.  Callers that desire shrink semantics
        // can call shrink_to_fit.
        auto working_store = GetWorkingStore();
        for (size_type i = 0; i < size_; i++) {
            working_store[i].~value_type();
        }
        size_ = 0;
    }

    void resize(size_type count) {
        struct ValueInitTag {  // tag to request value-initialization
            explicit ValueInitTag() = default;
        };
        Resize(count, ValueInitTag{});
    }

    void resize(size_type count, const value_type &value) { Resize(count, value); }

    void shrink_to_fit() {
        if (size_ == 0) {
            // shrink resets to small when empty
            capacity_ = kSmallCapacity;
            large_store_.reset();
            UpdateWorkingStore();
        } else if ((capacity_ > kSmallCapacity) && (capacity_ > size_)) {
            auto source = GetWorkingStore();
            // Keep the source from disappearing until the end of the function
            auto old_store = std::unique_ptr<BackingStore[]>(std::move(large_store_));
            assert(!large_store_);
            if (size_ < kSmallCapacity) {
                capacity_ = kSmallCapacity;
            } else {
                large_store_ = std::unique_ptr<BackingStore[]>(new BackingStore[size_]);
                capacity_ = size_;
            }
            UpdateWorkingStore();
            auto dest = GetWorkingStore();
            for (size_type i = 0; i < size_; i++) {
                dest[i] = std::move(source[i]);
                source[i].~value_type();
            }
        }
    }

    inline iterator begin() { return GetWorkingStore(); }
    inline const_iterator cbegin() const { return GetWorkingStore(); }
    inline const_iterator begin() const { return GetWorkingStore(); }

    inline iterator end() { return GetWorkingStore() + size_; }
    inline const_iterator cend() const { return GetWorkingStore() + size_; }
    inline const_iterator end() const { return GetWorkingStore() + size_; }
    inline size_type size() const { return size_; }
    auto capacity() const { return capacity_; }

    inline pointer data() { return GetWorkingStore(); }
    inline const_pointer data() const { return GetWorkingStore(); }

  protected:
    inline const_pointer ComputeWorkingStore() const {
        assert(large_store_ || (capacity_ == kSmallCapacity));

        const BackingStore *store = large_store_ ? large_store_.get() : small_store_;
        return &store->object;
    }
    inline pointer ComputeWorkingStore() {
        assert(large_store_ || (capacity_ == kSmallCapacity));

        BackingStore *store = large_store_ ? large_store_.get() : small_store_;
        return &store->object;
    }

    void UpdateWorkingStore() { working_store_ = ComputeWorkingStore(); }

    inline const_pointer GetWorkingStore() const {
        DbgWorkingStoreCheck();
        return working_store_;
    }
    inline pointer GetWorkingStore() {
        DbgWorkingStoreCheck();
        return working_store_;
    }

    inline pointer GetSmallStore() { return &small_store_->object; }

    union BackingStore {
        BackingStore() {}
        ~BackingStore() {}

        uint8_t data[sizeof(value_type)];
        value_type object;
    };
    size_type size_;
    size_type capacity_;
    BackingStore small_store_[N];
    std::unique_ptr<BackingStore[]> large_store_;
    value_type *working_store_;

#ifndef NDEBUG
    void DbgWorkingStoreCheck() const { assert(ComputeWorkingStore() == working_store_); };
#else
    void DbgWorkingStoreCheck() const {};
#endif

  private:
    void MoveLargeStore(small_vector &other) {
        assert(other.large_store_);
        assert(other.capacity_ > kSmallCapacity);
        // In move operations, from a small vector with a large store, we can move from it
        large_store_ = std::move(other.large_store_);
        capacity_ = other.capacity_;
        size_ = other.size_;
        UpdateWorkingStore();

        // We've stolen other's large store, must leave it in a valid state
        other.size_ = 0;
        other.capacity_ = kSmallCapacity;
        other.UpdateWorkingStore();
    }

    template <typename T2>
    void Resize(size_type new_size, const T2 &value) {
        if (new_size < size_) {
            auto working_store = GetWorkingStore();
            for (size_type i = new_size; i < size_; i++) {
                working_store[i].~value_type();
            }
            size_ = new_size;
        } else if (new_size > size_) {
            reserve(new_size);
            // if T2 != T and T is not DefaultInsertable, new values will be undefined
            if constexpr (std::is_same_v<T2, T> || std::is_default_constructible_v<T>) {
                for (size_type i = size_; i < new_size; ++i) {
                    if constexpr (std::is_same_v<T2, T>) {
                        emplace_back(value_type(value));
                    } else if constexpr (std::is_default_constructible_v<T>) {
                        emplace_back(value_type());
                    }
                }
                assert(size() == new_size);
            } else {
                size_ = new_size;
            }
        }
    }
};

// This is a wrapper around unordered_map that optimizes for the common case
// of only containing a small number of elements. The first N elements are stored
// inline in the object and don't require hashing or memory (de)allocation.

template <typename Key, typename value_type, typename inner_container_type, typename value_type_helper, int N>
class small_container {
  protected:
    bool small_data_allocated[N];
    value_type small_data[N];

    inner_container_type inner_cont;

    value_type_helper helper;

  public:
    small_container() {
        for (int i = 0; i < N; ++i) {
            small_data_allocated[i] = false;
        }
    }

    class iterator {
        typedef typename inner_container_type::iterator inner_iterator;
        friend class small_container<Key, value_type, inner_container_type, value_type_helper, N>;

        small_container<Key, value_type, inner_container_type, value_type_helper, N> *parent;
        int index;
        inner_iterator it;

      public:
        iterator() {}

        iterator operator++() {
            if (index < N) {
                index++;
                while (index < N && !parent->small_data_allocated[index]) {
                    index++;
                }
                if (index < N) {
                    return *this;
                }
                it = parent->inner_cont.begin();
                return *this;
            }
            ++it;
            return *this;
        }

        bool operator==(const iterator &other) const {
            if ((index < N) != (other.index < N)) {
                return false;
            }
            if (index < N) {
                return (index == other.index);
            }
            return it == other.it;
        }

        bool operator!=(const iterator &other) const { return !(*this == other); }

        value_type &operator*() const {
            if (index < N) {
                return parent->small_data[index];
            }
            return *it;
        }
        value_type *operator->() const {
            if (index < N) {
                return &parent->small_data[index];
            }
            return &*it;
        }
    };

    class const_iterator {
        typedef typename inner_container_type::const_iterator inner_iterator;
        friend class small_container<Key, value_type, inner_container_type, value_type_helper, N>;

        const small_container<Key, value_type, inner_container_type, value_type_helper, N> *parent;
        int index;
        inner_iterator it;

      public:
        const_iterator() {}

        const_iterator operator++() {
            if (index < N) {
                index++;
                while (index < N && !parent->small_data_allocated[index]) {
                    index++;
                }
                if (index < N) {
                    return *this;
                }
                it = parent->inner_cont.begin();
                return *this;
            }
            ++it;
            return *this;
        }

        bool operator==(const const_iterator &other) const {
            if ((index < N) != (other.index < N)) {
                return false;
            }
            if (index < N) {
                return (index == other.index);
            }
            return it == other.it;
        }

        bool operator!=(const const_iterator &other) const { return !(*this == other); }

        const value_type &operator*() const {
            if (index < N) {
                return parent->small_data[index];
            }
            return *it;
        }
        const value_type *operator->() const {
            if (index < N) {
                return &parent->small_data[index];
            }
            return &*it;
        }
    };

    iterator begin() {
        iterator it;
        it.parent = this;
        // If index 0 is allocated, return it, otherwise use operator++ to find the first
        // allocated element.
        it.index = 0;
        if (small_data_allocated[0]) {
            return it;
        }
        ++it;
        return it;
    }

    iterator end() {
        iterator it;
        it.parent = this;
        it.index = N;
        it.it = inner_cont.end();
        return it;
    }

    const_iterator begin() const {
        const_iterator it;
        it.parent = this;
        // If index 0 is allocated, return it, otherwise use operator++ to find the first
        // allocated element.
        it.index = 0;
        if (small_data_allocated[0]) {
            return it;
        }
        ++it;
        return it;
    }

    const_iterator end() const {
        const_iterator it;
        it.parent = this;
        it.index = N;
        it.it = inner_cont.end();
        return it;
    }

    bool contains(const Key &key) const {
        for (int i = 0; i < N; ++i) {
            if (small_data_allocated[i] && helper.compare_equal(small_data[i], key)) {
                return true;
            }
        }
        // check size() first to avoid hashing key unnecessarily.
        if (inner_cont.size() == 0) {
            return false;
        }
        return inner_cont.find(key) != inner_cont.end();
    }

    typename inner_container_type::size_type count(const Key &key) const { return contains(key) ? 1 : 0; }

    std::pair<iterator, bool> insert(const value_type &value) {
        for (int i = 0; i < N; ++i) {
            if (small_data_allocated[i] && helper.compare_equal(small_data[i], value)) {
                iterator it;
                it.parent = this;
                it.index = i;
                return std::make_pair(it, false);
            }
        }
        // check size() first to avoid hashing key unnecessarily.
        auto iter = inner_cont.size() > 0 ? inner_cont.find(helper.get_key(value)) : inner_cont.end();
        if (iter != inner_cont.end()) {
            iterator it;
            it.parent = this;
            it.index = N;
            it.it = iter;
            return std::make_pair(it, false);
        } else {
            for (int i = 0; i < N; ++i) {
                if (!small_data_allocated[i]) {
                    small_data_allocated[i] = true;
                    helper.assign(small_data[i], value);
                    iterator it;
                    it.parent = this;
                    it.index = i;
                    return std::make_pair(it, true);
                }
            }
            iter = inner_cont.insert(value).first;
            iterator it;
            it.parent = this;
            it.index = N;
            it.it = iter;
            return std::make_pair(it, true);
        }
    }

    typename inner_container_type::size_type erase(const Key &key) {
        for (int i = 0; i < N; ++i) {
            if (small_data_allocated[i] && helper.compare_equal(small_data[i], key)) {
                small_data_allocated[i] = false;
                return 1;
            }
        }
        return inner_cont.erase(key);
    }

    typename inner_container_type::size_type size() const {
        auto size = inner_cont.size();
        for (int i = 0; i < N; ++i) {
            if (small_data_allocated[i]) {
                size++;
            }
        }
        return size;
    }

    bool empty() const {
        for (int i = 0; i < N; ++i) {
            if (small_data_allocated[i]) {
                return false;
            }
        }
        return inner_cont.size() == 0;
    }

    void clear() {
        for (int i = 0; i < N; ++i) {
            small_data_allocated[i] = false;
        }
        inner_cont.clear();
    }
};

// Helper function objects to compare/assign/get keys in small_unordered_set/map.
// This helps to abstract away whether value_type is a Key or a pair<Key, T>.
template <typename MapType>
class value_type_helper_map {
    using PairType = typename MapType::value_type;
    using Key = typename std::remove_const<typename PairType::first_type>::type;

  public:
    bool compare_equal(const PairType &lhs, const Key &rhs) const { return lhs.first == rhs; }
    bool compare_equal(const PairType &lhs, const PairType &rhs) const { return lhs.first == rhs.first; }

    void assign(PairType &lhs, const PairType &rhs) const {
        // While the const_cast may be unsatisfactory, we are using small_data as
        // stand-in for placement new and a small-block allocator, so the const_cast
        // is minimal, contained, valid, and allows operators * and -> to avoid copies
        const_cast<Key &>(lhs.first) = rhs.first;
        lhs.second = rhs.second;
    }

    Key get_key(const PairType &value) const { return value.first; }
};

template <typename Key>
class value_type_helper_set {
  public:
    bool compare_equal(const Key &lhs, const Key &rhs) const { return lhs == rhs; }

    void assign(Key &lhs, const Key &rhs) const { lhs = rhs; }

    Key get_key(const Key &value) const { return value; }
};

template <typename Key, typename T, int N = 1>
class small_unordered_map : public small_container<Key, typename vvl::unordered_map<Key, T>::value_type, vvl::unordered_map<Key, T>,
                                                   value_type_helper_map<vvl::unordered_map<Key, T>>, N> {
  public:
    T &operator[](const Key &key) {
        for (int i = 0; i < N; ++i) {
            if (this->small_data_allocated[i] && this->helper.compare_equal(this->small_data[i], key)) {
                return this->small_data[i].second;
            }
        }
        auto iter = this->inner_cont.find(key);
        if (iter != this->inner_cont.end()) {
            return iter->second;
        } else {
            for (int i = 0; i < N; ++i) {
                if (!this->small_data_allocated[i]) {
                    this->small_data_allocated[i] = true;
                    this->helper.assign(this->small_data[i], {key, T()});

                    return this->small_data[i].second;
                }
            }
            return this->inner_cont[key];
        }
    }
};

template <typename Key, int N = 1>
class small_unordered_set : public small_container<Key, Key, vvl::unordered_set<Key>, value_type_helper_set<Key>, N> {};

// For the given data key, look up the layer_data instance from given layer_data_map
template <typename DATA_T>
DATA_T *GetLayerDataPtr(void *data_key, small_unordered_map<void *, DATA_T *, 2> &layer_data_map) {
    /* TODO: We probably should lock here, or have caller lock */
    DATA_T *&got = layer_data_map[data_key];

    if (got == nullptr) {
        got = new DATA_T;
    }

    return got;
}

template <typename DATA_T>
void FreeLayerDataPtr(void *data_key, small_unordered_map<void *, DATA_T *, 2> &layer_data_map) {
    delete layer_data_map[data_key];
    layer_data_map.erase(data_key);
}

// For the given data key, look up the layer_data instance from given layer_data_map
template <typename DATA_T>
DATA_T *GetLayerDataPtr(void *data_key, std::unordered_map<void *, DATA_T *> &layer_data_map) {
    DATA_T *debug_data;
    /* TODO: We probably should lock here, or have caller lock */
    auto got = layer_data_map.find(data_key);

    if (got == layer_data_map.end()) {
        debug_data = new DATA_T;
        layer_data_map[(void *)data_key] = debug_data;
    } else {
        debug_data = got->second;
    }

    return debug_data;
}

template <typename DATA_T>
void FreeLayerDataPtr(void *data_key, std::unordered_map<void *, DATA_T *> &layer_data_map) {
    auto got = layer_data_map.find(data_key);
    assert(got != layer_data_map.end());

    delete got->second;
    layer_data_map.erase(got);
}

namespace vvl {

inline constexpr std::in_place_t in_place{};

// Partial implementation of std::span for C++11
template <typename T>
class span {
  public:
    using pointer = T *;
    using const_pointer = T const *;
    using iterator = pointer;
    using const_iterator = const_pointer;

    span() = default;
    span(pointer start, size_t n) : data_(start), count_(n) {}
    template <typename Iterator>
    span(Iterator start, Iterator end) : data_(&(*start)), count_(end - start) {}
    template <typename Container>
    span(Container &c) : data_(c.data()), count_(c.size()) {}

    iterator begin() { return data_; }
    const_iterator begin() const { return data_; }

    iterator end() { return data_ + count_; }
    const_iterator end() const { return data_ + count_; }

    T &operator[](int i) { return data_[i]; }
    const T &operator[](int i) const { return data_[i]; }

    T &front() { return *data_; }
    const T &front() const { return *data_; }

    T &back() { return *(data_ + (count_ - 1)); }
    const T &back() const { return *(data_ + (count_ - 1)); }

    size_t size() const { return count_; }
    bool empty() const { return count_ == 0; }

    pointer data() { return data_; }
    const_pointer data() const { return data_; }

  private:
    pointer data_ = {};
    size_t count_ = 0;
};

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

template <typename BaseType>
using base_type =
    typename std::remove_reference<typename std::remove_const<typename std::remove_pointer<BaseType>::type>::type>::type;

// Helper for thread local Validate -> Record phase data
// Define T unique to each entrypoint which will persist data
// Use only in with singleton (leaf) validation objects
// State machine transition state changes of payload relative to TlsGuard object lifecycle:
//  State INIT: bool(payload_)
//  State RESET: NOT bool(payload_)
//    * PreCallValidate* phase
//        * Initialized with skip (in PreCallValidate*)
//            * RESET -> INIT
//        * Destruct with skip == true
//           * INIT -> RESET
//    * PreCallRecord* phase (optional IF PostCallRecord present)
//        * Initialized w/o skip (set "persist_" IFF PostCallRecord present)
//           * Must be in state INIT
//        * Destruct with NOT persist_
//           * INIT -> RESET
//    * PreCallRecord* phase (optional IF PreCallRecord present)
//        * Initialized w/o skip ("persist_" *must be false)
//           * Must be in state INIT
//        * Destruct
//           * INIT -> RESET

struct TlsGuardPersist {};
template <typename T>
class TlsGuard {
  public:
    // For use on inital references -- Validate phase
    template <typename... Args>
    TlsGuard(bool *skip, Args &&...args) : skip_(skip), persist_(false) {
        // Record phase calls are required to clean up payload
        assert(!payload_);
        payload_.emplace(std::forward<Args>(args)...);
    }
    // For use on non-terminal persistent references (PreRecord phase IFF PostRecord is also present.
    TlsGuard(const TlsGuardPersist &) : skip_(nullptr), persist_(true) { assert(payload_); }
    // For use on terminal persistent references
    // Validate phase calls are required to setup payload
    // PreCallRecord calls are required to preserve (persist_) payload, if PostCallRecord calls will use
    TlsGuard() : skip_(nullptr), persist_(false) { assert(payload_); }
    ~TlsGuard() {
        assert(payload_);
        if (!persist_ && (!skip_ || *skip_)) payload_.reset();
    }

    T &operator*() & {
        assert(payload_);
        return *payload_;
    }
    const T &operator*() const & {
        assert(payload_);
        return *payload_;
    }
    T &&operator*() && {
        assert(payload_);
        return std::move(*payload_);
    }
    T *operator->() { return &(*payload_); }

  private:
    inline thread_local static std::optional<T> payload_{};
    bool *skip_;
    bool persist_;
};

// Only use this if you aren't planning to use what you would have gotten from a find.
template <typename Container, typename Key = typename Container::key_type>
bool Contains(const Container &container, const Key &key) {
    return container.find(key) != container.cend();
}

// EraseIf is not implemented as std::erase(std::remove_if(...), ...) for two reasons:
//   1) Robin Hood containers don't support two-argument erase functions
//   2) STL remove_if requires the predicate to be const w.r.t the value-type, and std::erase_if doesn't AFAICT
template <typename Container, typename Predicate>
typename Container::size_type EraseIf(Container &c, Predicate &&p) {
    const auto before_size = c.size();
    auto pos = c.begin();
    while (pos != c.end()) {
        if (p(*pos)) {
            pos = c.erase(pos);
        } else {
            ++pos;
        }
    }
    return before_size - c.size();
}

template <typename T>
constexpr T MaxTypeValue([[maybe_unused]] T) {
    return std::numeric_limits<T>::max();
}

template <typename T>
constexpr T MinTypeValue([[maybe_unused]] T) {
    return std::numeric_limits<T>::min();
}

// Typesafe UINT32_MAX
constexpr auto kU32Max = std::numeric_limits<uint32_t>::max();
// Typesafe UINT64_MAX
constexpr auto kU64Max = std::numeric_limits<uint64_t>::max();
// Typesafe INT32_MAX
constexpr auto kI32Max = std::numeric_limits<int32_t>::max();
// Typesafe INT64_MAX
constexpr auto kI64Max = std::numeric_limits<int64_t>::max();

template <typename T>
T GetQuotientCeil(T numerator, T denominator) {
    denominator = std::max(denominator, T{1});
    return static_cast<T>(std::ceil(static_cast<double>(numerator) / static_cast<double>(denominator)));
}

}  // namespace vvl
