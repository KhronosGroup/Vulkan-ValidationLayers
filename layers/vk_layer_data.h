/* Copyright (c) 2015-2017, 2019 The Khronos Group Inc.
 * Copyright (c) 2015-2017, 2019 Valve Corporation
 * Copyright (c) 2015-2017, 2019 LunarG, Inc.
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
 */

#ifndef LAYER_DATA_H
#define LAYER_DATA_H

#include <cassert>
#include <unordered_map>

// This is a wrapper around unordered_map that optimizes for the common case
// of only containing a small number of elements. The first N elements are stored
// inline in the object and don't require hashing or memory (de)allocation.
template <typename Key, typename T, int N = 1>
class small_unordered_map {
    bool small_data_allocated[N];
    using value_type = std::pair<const Key, T>;
    value_type small_data[N];

    std::unordered_map<Key, T> inner_map;

  public:
    small_unordered_map() {
        for (int i = 0; i < N; ++i) {
            small_data_allocated[i] = false;
        }
    }

    class iterator {
        typedef typename std::unordered_map<Key, T>::iterator inner_iterator;
        friend class small_unordered_map<Key, T, N>;

        small_unordered_map<Key, T, N> *parent;
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
                it = parent->inner_map.begin();
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
        it.it = inner_map.end();
        return it;
    }

    bool contains(const Key &key) const {
        for (int i = 0; i < N; ++i) {
            if (small_data[i].first == key && small_data_allocated[i]) {
                return true;
            }
        }
        // check size() first to avoid hashing key unnecessarily.
        if (inner_map.size() == 0) {
            return false;
        }
        return inner_map.find(key) != inner_map.end();
    }

    T &operator[](const Key &key) {
        for (int i = 0; i < N; ++i) {
            if (small_data[i].first == key && small_data_allocated[i]) {
                return small_data[i].second;
            }
        }
        auto iter = inner_map.find(key);
        if (iter != inner_map.end()) {
            return iter->second;
        } else {
            for (int i = 0; i < N; ++i) {
                if (!small_data_allocated[i]) {
                    small_data_allocated[i] = true;

                    // While the const_cast may be unsatisfactory, we are using small_data as
                    // stand-in for placement new and a small-block allocator, so the const_cast
                    // is minimal, contained, valid, and allows operators * and -> to avoid copies
                    const_cast<Key &>(small_data[i].first) = key;
                    small_data[i].second = T();

                    return small_data[i].second;
                }
            }
            return inner_map[key];
        }
    }

    std::pair<iterator, bool> insert(const value_type &value) {
        for (int i = 0; i < N; ++i) {
            if (small_data[i].first == value.first && small_data_allocated[i]) {
                iterator it;
                it.parent = this;
                it.index = i;
                return std::make_pair(it, false);
            }
        }
        // check size() first to avoid hashing key unnecessarily.
        auto iter = inner_map.size() > 0 ? inner_map.find(value.first) : inner_map.end();
        if (iter != inner_map.end()) {
            iterator it;
            it.parent = this;
            it.index = N;
            it.it = iter;
            return std::make_pair(it, false);
        } else {
            for (int i = 0; i < N; ++i) {
                if (!small_data_allocated[i]) {
                    small_data_allocated[i] = true;
                    const_cast<Key &>(small_data[i].first) = value.first;
                    small_data[i].second = value.second;
                    iterator it;
                    it.parent = this;
                    it.index = i;
                    return std::make_pair(it, true);
                }
            }
            iter = inner_map.insert(value).first;
            iterator it;
            it.parent = this;
            it.index = N;
            it.it = iter;
            return std::make_pair(it, true);
        }
    }

    typename std::unordered_map<Key, T>::size_type erase(const Key &key) {
        for (int i = 0; i < N; ++i) {
            if (small_data[i].first == key && small_data_allocated[i]) {
                small_data_allocated[i] = false;
                return 1;
            }
        }
        return inner_map.erase(key);
    }

    void clear() {
        for (int i = 0; i < N; ++i) {
            small_data_allocated[i] = false;
        }
        inner_map.clear();
    }
};

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
    typename std::unordered_map<void *, DATA_T *>::const_iterator got;

    /* TODO: We probably should lock here, or have caller lock */
    got = layer_data_map.find(data_key);

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

#endif  // LAYER_DATA_H
