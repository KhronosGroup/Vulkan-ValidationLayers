/* Copyright (c) 2015-2017, 2019-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2017, 2019-2025 Valve Corporation
 * Copyright (c) 2015-2017, 2019-2025 LunarG, Inc.
 * Modifications Copyright (C) 2022 RasterGrid Kft.
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

#include <bitset>
#include <cassert>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <type_traits>

#ifdef WIN32
#include <intrin.h>  // For __lzcnt()
#else
#include <strings.h>  // For ffs()
#endif

template <typename T>
constexpr bool IsPowerOfTwo(T x) {
    static_assert(std::is_integral_v<T> && std::is_unsigned_v<T>, "Unsigned integer required");
    return x && !(x & (x - 1));
}

template <typename T>
constexpr uint32_t GetBitSetCount(T value) {
    static_assert(std::is_integral_v<T> && std::is_unsigned_v<T>, "Unsigned integer required");
    static_assert(sizeof(T) == 4 || sizeof(T) == 8, "32 or 64 bit value is expected");
    return static_cast<uint32_t>(std::bitset<sizeof(T) * 8>(value).count());
}

template <typename T>
constexpr bool IsSingleBitSet(T flags) {
    static_assert(std::is_integral_v<T> && std::is_unsigned_v<T>, "Unsigned integer required");
    return IsPowerOfTwo(flags);
}

// Returns the 0-based index of the MSB, like the x86 bit scan reverse (bsr) instruction
// Note: an input mask of 0 yields -1
[[maybe_unused]] static inline int MostSignificantBit(uint32_t mask) {
#if defined __GNUC__
    return mask ? __builtin_clz(mask) ^ 31 : -1;
#elif defined _MSC_VER
    unsigned long bit_pos;
    return _BitScanReverse(&bit_pos, mask) ? int(bit_pos) : -1;
#else
    for (int k = 31; k >= 0; --k) {
        if (((mask >> k) & 1) != 0) {
            return k;
        }
    }
    return -1;
#endif
}

static inline int u_ffs(int val) {
#ifdef WIN32
    unsigned long bit_pos = 0;
    if (_BitScanForward(&bit_pos, val) != 0) {
        bit_pos += 1;
    }
    return bit_pos;

#elif defined __GNUC__
    return __builtin_ffs(val);
#else
    return ffs(val);
#endif
}

// Given p2 a power of two, returns smallest multiple of p2 greater than or equal to x
// Different than std::align in that it simply aligns an unsigned integer, when std::align aligns a virtual address and does the
// necessary bookkeeping to be able to correctly free memory at the new address
template <typename T>
constexpr T Align(T x, T p2) {
    static_assert(std::numeric_limits<T>::is_integer, "Unsigned integer required.");
    static_assert(std::is_unsigned<T>::value, "Unsigned integer required.");
    assert(IsPowerOfTwo(p2));
    return (x + p2 - 1) & ~(p2 - 1);
}

// Returns the 0-based index of the LSB. An input mask of 0 yields -1
static inline int LeastSignificantBit(uint32_t mask) { return u_ffs(static_cast<int>(mask)) - 1; }

template <typename FlagBits, typename Flags>
FlagBits LeastSignificantFlag(Flags flags) {
    const int bit_shift = LeastSignificantBit(flags);
    assert(bit_shift != -1);
    return static_cast<FlagBits>(1ull << bit_shift);
}
