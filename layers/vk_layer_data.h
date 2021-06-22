/* Copyright (c) 2015-2017, 2019-2021 The Khronos Group Inc.
 * Copyright (c) 2015-2017, 2019-2021 Valve Corporation
 * Copyright (c) 2015-2017, 2019-2021 LunarG, Inc.
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
 * Author: Tobin Ehlis <tobine@google.com>
 * Author: Jeff Bolz <jbolz@nvidia.com>
 * Author: John Zulauf <jzulauf@lunarg.com>

 */

#ifndef LAYER_DATA_H
#define LAYER_DATA_H

#include <cassert>
#include <limits>
#include <memory>
#include <map>
#include <unordered_map>
#include <set>
#include <algorithm>
#include <iterator>
#include <type_traits>

#ifdef USE_ROBIN_HOOD_HASHING
#include "robin_hood.h"
#else
#include <unordered_set>
#endif

// namespace aliases to allow map and set implementations to easily be swapped out
namespace layer_data {

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
template <typename T>
class insert_iterator : public std::iterator<std::output_iterator_tag, void, void, void, void> {
  public:
    typedef typename T::value_type value_type;
    typedef typename T::iterator iterator;
    insert_iterator(T &t, iterator i) : container(&t), iter(i) {}

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
    typename T::iterator iter;
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

#if __cplusplus < 201402L
// Temporary workaround for c++11. Remove with std >= c++14.
template<typename T, typename... Args>
constexpr std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
#else
template <typename T>
constexpr auto make_unique = std::make_unique<T>;
#endif

}  // namespace layer_data

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

    small_vector() : size_(0), capacity_(N) {}

    small_vector(const small_vector &other) : size_(0), capacity_(N) {
        reserve(other.size_);
        auto dest = GetWorkingStore();
        for (const auto &value : other) {
            new (dest) value_type(value);
            ++dest;
        }
        size_ = other.size_;
    }

    small_vector(small_vector &&other) : size_(0), capacity_(N) {
        if (other.large_store_) {
            // Can just take ownership of the other large store
            large_store_ = std::move(other.large_store_);
            capacity_ = other.capacity_;
            other.capacity_ = kSmallCapacity;
        } else {
            auto dest = GetWorkingStore();
            for (auto &value : other) {
                new (dest) value_type(std::move(value));
                value.~value_type();
                ++dest;
            }
        }
        size_ = other.size_;
        other.size_ = 0;
    }

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

    small_vector &operator=(const small_vector &other) {
        if (this != &other) {
            reserve(other.size_);  // reserve doesn't shrink!
            auto dest = GetWorkingStore();
            auto source = other.GetWorkingStore();

            const auto overlap = std::min(size_, other.size_);
            // Copy assign anywhere we have objects in this
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
        return *this;
    }

    small_vector &operator=(small_vector &&other) {
        if (this != &other) {
            if (other.large_store_) {
                clear();  // need to clean up any objects this owns.
                // Can just take ownership of the other large store
                large_store_ = std::move(other.large_store_);
                capacity_ = other.capacity_;
                size_ = other.size_;

                other.capacity_ = kSmallCapacity;
            } else {
                // Other is using the small_store
                auto source = other.begin();
                iterator dest;
                if (large_store_) {
                    // If this is using large store do a wholesale clobber of it.
                    ClearAndReset();
                    dest = GetWorkingStore();
                } else {
                    // This is also using small store, so move assign where both have valid values
                    dest = GetWorkingStore();
                    // Move values where both vectors have valid values
                    for (size_type i = 0; i < std::min(size_, other.size_); i++) {
                        *dest = std::move(*source);
                        source->~value_type();
                        ++dest;
                        ++source;
                    }
                }

                // Other is bigger, placement new into the working store
                // NOTE: this loop only runs when other is bigger
                for (size_type i = size_; i < other.size_; i++) {
                    new (dest) value_type(std::move(*source));
                    source->~value_type();
                    ++dest;
                    ++source;
                }
                // Other is smaller, clean up the excess entries
                // NOTE: this loop only runs when this is bigger
                for (size_type i = other.size_; i < size_; i++) {
                    dest->~value_type();
                    ++dest;
                }

                size_ = other.size_;
            }

            // When we're done other has no valid contents (all are moved or destructed)
            other.size_ = 0;
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

    // Like std::vector::back, calling back on an empty container causes undefined behavior
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

    void reserve(size_type new_cap) {
        // Since this can't shrink, if we're growing we're newing
        if (new_cap > capacity_) {
            assert(capacity_ >= kSmallCapacity);
            auto new_store = std::unique_ptr<BackingStore[]>(new BackingStore[new_cap]);
            auto new_values = reinterpret_cast<pointer>(new_store.get());
            auto working_store = GetWorkingStore();
            for (size_type i = 0; i < size_; i++) {
                new (new_values + i) value_type(std::move(working_store[i]));
                working_store[i].~value_type();
            }
            large_store_ = std::move(new_store);
        }
        // No shrink here.
    }

    void clear() {
        auto working_store = GetWorkingStore();
        for (size_type i = 0; i < size_; i++) {
            working_store[i].~value_type();
        }
        size_ = 0;
    }

    inline iterator begin() { return GetWorkingStore(); }
    inline const_iterator cbegin() const { return GetWorkingStore(); }
    inline const_iterator begin() const { return GetWorkingStore(); }

    inline iterator end() { return GetWorkingStore() + size_; }
    inline const_iterator cend() const { return GetWorkingStore() + size_; }
    inline const_iterator end() const { return GetWorkingStore() + size_; }
    inline size_type size() const { return size_; }

  protected:
    inline const_pointer GetWorkingStore() const {
        const BackingStore *store = large_store_ ? large_store_.get() : small_store_;
        return reinterpret_cast<const_pointer>(store);
    }
    inline pointer GetWorkingStore() {
        BackingStore *store = large_store_ ? large_store_.get() : small_store_;
        return reinterpret_cast<pointer>(store);
    }

    void ClearAndReset() {
        clear();
        large_store_.reset();
        capacity_ = kSmallCapacity;
    }

    struct alignas(alignof(value_type)) BackingStore {
        uint8_t data[sizeof(value_type)];
    };
    size_type size_;
    size_type capacity_;
    BackingStore small_store_[N];
    std::unique_ptr<BackingStore[]> large_store_;
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
class small_unordered_map
    : public small_container<Key, typename layer_data::unordered_map<Key, T>::value_type, layer_data::unordered_map<Key, T>,
                             value_type_helper_map<layer_data::unordered_map<Key, T>>, N> {
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
class small_unordered_set : public small_container<Key, Key, layer_data::unordered_set<Key>, value_type_helper_set<Key>, N> {};

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

namespace layer_data {

struct in_place_t {};
static constexpr in_place_t in_place{};

// A C++11 approximation of std::optional
template <typename T>
class optional {
  protected:
    union Store {
        Store(){};   // Do nothing.  That's the point.
        ~Store(){};  // Not safe to destroy this object outside of its stateful container to clean up T if any.
        typename std::aligned_storage<sizeof(T), alignof(T)>::type backing;
        T obj;
    };

  public:
    optional() : init_(false) {}

    template <typename... Args>
    explicit optional(in_place_t, const Args &...args) { emplace(args...); }
    optional(const optional &other) : init_(false) { *this = other; }
    optional(optional &&other) : init_(false) { *this = std::move(other); }

    ~optional() { DeInit(); }

    template <typename... Args>
    T &emplace(const Args &...args) {
        init_ = true;
        new (&store_.backing) T(args...);
        return store_.obj;
    }
    T *operator&() {
        if (init_) return &store_.obj;
        return nullptr;
    }
    const T *operator&() const {
        if (init_) return &store_.obj;
        return nullptr;
    }
    T *operator->() {
        if (init_) return &store_.obj;
        return nullptr;
    }
    const T *operator->() const {
        if (init_) return &store_.obj;
        return nullptr;
    }
    operator bool() const { return init_; }
    bool has_value() const { return init_; }

    optional &operator=(const optional &other) {
        if (other.has_value()) {
            if (has_value()) {
                store_.obj = other.store_.obj;
            } else {
                emplace(other.store_.obj);
            }
        } else {
            DeInit();
        }
        return *this;
    }

    optional &operator=(optional &&other) {
        if (other.has_value()) {
            if (has_value()) {
                store_.obj = std::move(other.store_.obj);
            } else {
                emplace(std::move(other.store_.obj));
            }
        } else {
            DeInit();
        }
        return *this;
    }

    T& operator*() & {
        assert(init_);
        return store_.obj;
    }
    const T& operator*() const& {
        assert(init_);
        return store_.obj;
    }
    T&& operator*() && {
        assert(init_);
        return std::move(store_.obj);
    }
    const T&& operator*() const&& {
        assert(init_);
        return std::move(store_.obj);
    }
  protected:
    inline void DeInit() {
        if (init_) {
            store_.obj.~T();
            init_ = false;
        }
    }
    Store store_;
    bool init_;
};
}  // namespace layer_data
#endif  // LAYER_DATA_H
