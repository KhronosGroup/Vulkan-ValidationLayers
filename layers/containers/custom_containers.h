/* Copyright (c) 2015-2017, 2019-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2017, 2019-2025 Valve Corporation
 * Copyright (c) 2015-2017, 2019-2025 LunarG, Inc.
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
#include <algorithm>
#include <iterator>
#include <utility>

#ifdef USE_CUSTOM_HASH_MAP
#include "robin_hood.h"
#else
#include <map>
#include <set>
#include <unordered_set>
#include <unordered_map>
#endif

#include <vulkan/utility/vk_concurrent_unordered_map.hpp>

// namespace aliases to allow map and set implementations to easily be swapped out
namespace vvl {

#ifdef USE_CUSTOM_HASH_MAP
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

#if 1
template <typename Key, typename T, int BucketsLog2 = 2, typename Hash = std::hash<Key>>
using concurrent_unordered_map = vku::concurrent::unordered_map<Key, T, BucketsLog2, vvl::unordered_map<Key, T, Hash>>;
#else
template <typename Key, typename T, int BucketsLog2 = 2>
using concurrent_unordered_map = vku::concurrent_unordered_map<Key, T, BucketsLog2, vvl::unordered_map<Key, T>>;
#endif

}  // namespace vvl
