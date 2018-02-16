/* Copyright (c) 2015-2016 The Khronos Group Inc.
 * Copyright (c) 2015-2016 Valve Corporation
 * Copyright (c) 2015-2016 LunarG, Inc.
 * Copyright (C) 2015-2016 Google Inc.
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
 * Author: John Zulauf <jzulauf@lunarg.com>
 */
#ifndef HASH_UTIL_H_
#define HASH_UTIL_H_

#define NOMINMAX
#include <cstdint>
#include <functional>
#include <limits>
#include <type_traits>
#include <vector>

// Hash and equality utilities for supporting hashing containers (e.g. unordered_set, unordered_map)
namespace hash_util {

// True iff both pointers are null or both are non-null
template <typename T>
bool similar_for_nullity(const T *const lhs, const T *const rhs) {
    return ((lhs != nullptr) && (rhs != nullptr)) || ((lhs == nullptr) && (rhs == nullptr));
}

// Wrap std hash to avoid manual casts for the holes in std::hash (in C++11)
template <typename Value>
size_t HashWithUnderlying(Value value, typename std::enable_if<!std::is_enum<Value>::value, void *>::type = nullptr) {
    return std::hash<Value>()(value);
}

template <typename Value>
size_t HashWithUnderlying(Value value, typename std::enable_if<std::is_enum<Value>::value, void *>::type = nullptr) {
    using Underlying = typename std::underlying_type<Value>::type;
    return std::hash<Underlying>()(static_cast<const Underlying &>(value));
}

class HashCombiner {
   public:
    using Key = size_t;

    template <typename Value>
    struct WrappedHash {
        size_t operator()(const Value &value) const { return HashWithUnderlying(value); }
    };

    HashCombiner(Key combined = 0) : combined_(combined) {}
    // magic and combination algorithm based on boost::hash_combine
    // http://www.boost.org/doc/libs/1_43_0/doc/html/hash/reference.html#boost.hash_combine
    // Magic value is 2^size / ((1-sqrt(5)/2)
    static const uint64_t kMagic = sizeof(Key) > 4 ? Key(0x9e3779b97f4a7c16UL) : Key(0x9e3779b9U);

    // If you need to override the default hash
    template <typename Value, typename Hasher = WrappedHash<Value>>
    HashCombiner &Combine(const Value &value) {
        combined_ ^= Hasher()(value) + kMagic + (combined_ << 6) + (combined_ >> 2);
        return *this;
    }

    template <typename Iterator, typename Hasher = WrappedHash<typename std::iterator_traits<Iterator>::value_type>>
    HashCombiner &Combine(Iterator first, Iterator end) {
        using Value = typename std::iterator_traits<Iterator>::value_type;
        auto current = first;
        for (; current != end; ++current) {
            Combine<Value, Hasher>(*current);
        }
        return *this;
    }

    template <typename Value, typename Hasher = WrappedHash<Value>>
    HashCombiner &Combine(const std::vector<Value> &vector) {
        return Combine(vector.cbegin(), vector.cend());
    }

    template <typename Value>
    HashCombiner &operator<<(const Value &value) {
        return Combine(value);
    }

    Key Value() const { return combined_; }
    void Reset(Key combined = 0) { combined_ = combined; }

   private:
    Key combined_;
};

// A template to inherit std::hash overloads from when T::hash() is defined
template <typename T>
struct HasHashMember {
    size_t operator()(const T &value) const { return value.hash(); }
};

// A template to inherit std::hash overloads from when is an *ordered* constainer
template <typename T>
struct IsOrderedContainer {
    size_t operator()(const T &value) const { return HashCombiner().Combine(value.cbegin(), value.cend()).Value(); }
};

}  // namespace hash_util

#endif  // HASH_UTILS_H_
