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
#include "parallel_hashmap/phmap.h"
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
using hash = phmap::Hash<T>;

template <typename Key, typename Hash = phmap::Hash<Key>, typename KeyEqual = std::equal_to<Key>>
using unordered_set = phmap::flat_hash_set<Key, Hash, KeyEqual>;

template <typename Key, typename T, typename Hash = phmap::Hash<Key>, typename KeyEqual = std::equal_to<Key>>
using unordered_map = phmap::flat_hash_map<Key, T, Hash, KeyEqual>;

template <typename Key, typename T>
using map_entry = phmap::Pair<Key, T>;

#else
template <typename T>
using hash = std::hash<T>;

template <typename Key, typename Hash = std::hash<Key>, typename KeyEqual = std::equal_to<Key>>
using unordered_set = std::unordered_set<Key, Hash, KeyEqual>;

template <typename Key, typename T, typename Hash = std::hash<Key>, typename KeyEqual = std::equal_to<Key>>
using unordered_map = std::unordered_map<Key, T, Hash, KeyEqual>;

template <typename Key, typename T>
using map_entry = std::pair<Key, T>;

#endif

#if 1
template <typename Key, typename T, int BucketsLog2 = 2, typename Hash = std::hash<Key>>
using concurrent_unordered_map = vku::concurrent::unordered_map<Key, T, BucketsLog2, vvl::unordered_map<Key, T, Hash>>;
#else
template <typename Key, typename T, int BucketsLog2 = 2>
using concurrent_unordered_map = vku::concurrent_unordered_map<Key, T, BucketsLog2, vvl::unordered_map<Key, T>>;
#endif

}  // namespace vvl
