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
#include <algorithm>
#include <memory>
#include <vector>

namespace vvl {

inline constexpr std::in_place_t in_place{};

// Only use this if you aren't planning to use what you would have gotten from a find.
template <typename Container, typename Key = typename Container::key_type>
bool Contains(const Container &container, const Key &key) {
    return container.find(key) != container.cend();
}

//
// if (vvl::Contains(objects_vector, candidate)) { candidate->jump(); }
//
template <typename T>
bool Contains(const std::vector<T> &v, const T &value) {
    return std::find(v.cbegin(), v.cend(), value) != v.cend();
}
// Overload for the case of shared_ptr<const T> and shared_ptr<T>.
// They are convertible but conversion is not performed during template type deduction.
template <typename T>
bool Contains(const std::vector<std::shared_ptr<const T>> &v, const std::shared_ptr<T> &value) {
    return std::find(v.cbegin(), v.cend(), value) != v.cend();
}

//
// if (auto* thing = vvl::Find(map, key)) { thing->jump(); }
//
template <typename Container, typename Key = typename Container::key_type, typename Value = typename Container::mapped_type>
Value *Find(Container &container, const Key &key) {
    auto it = container.find(key);
    return (it != container.end()) ? &it->second : nullptr;
}
template <typename Container, typename Key = typename Container::key_type, typename Value = typename Container::mapped_type>
const Value *Find(const Container &container, const Key &key) {
    auto it = container.find(key);
    return (it != container.cend()) ? &it->second : nullptr;
}

//
// auto& thing = vvl::FindExisting(map, key);
//
template <typename Container, typename Key = typename Container::key_type, typename Value = typename Container::mapped_type>
Value &FindExisting(Container &container, const Key &key) {
    auto it = container.find(key);
    assert(it != container.end());
    return it->second;
}

template <typename Container, typename Key = typename Container::key_type, typename Value = typename Container::mapped_type>
const Value &FindExisting(const Container &container, const Key &key) {
    auto it = container.find(key);
    assert(it != container.end());
    return it->second;
}

template <typename T>
void Append(std::vector<T> &dst, const std::vector<T> &src) {
    dst.insert(dst.end(), src.begin(), src.end());
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

// Replace with the std version after VVL switches to C++20.
// https://en.cppreference.com/w/cpp/container/vector/erase2
template <typename T, typename Pred>
typename std::vector<T>::size_type erase_if(std::vector<T> &c, Pred pred) {
    auto it = std::remove_if(c.begin(), c.end(), pred);
    auto r = c.end() - it;
    c.erase(it, c.end());
    return r;
}

}  // namespace vvl
