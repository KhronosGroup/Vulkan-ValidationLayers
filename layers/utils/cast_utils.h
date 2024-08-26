/* Copyright (c) 2019-2024 The Khronos Group Inc.
 * Copyright (c) 2019-2024 Valve Corporation
 * Copyright (c) 2019-2024 LunarG, Inc.
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

#include "utils/numerical_types.h"

#include <cassert>
#include <cstdint>
#include <cstring>
#include <type_traits>

#define CAST_TO_FROM_UTILS
// Casts to allow various types of less than 64 bits to be cast to and from u64 safely and portably
template <typename HandleType, typename Uint>
static inline HandleType CastFromUint(Uint untyped_handle) {
    static_assert(sizeof(HandleType) == sizeof(Uint), "HandleType must be the same size as untyped handle");
    HandleType var{};
    std::memcpy(&var, &untyped_handle, sizeof(Uint));
    return var;
}
template <typename HandleType, typename Uint>
static inline Uint CastToUint(HandleType handle) {
    static_assert(sizeof(HandleType) == sizeof(Uint), "HandleType must be the same size as untyped handle");
    Uint var{};
    std::memcpy(&var, &handle, sizeof(Uint));
    return var;
}

// Implementation of C++20 bit_cast
// https://en.cppreference.com/w/cpp/numeric/bit_cast
template <class To, class From>
std::enable_if_t<sizeof(To) == sizeof(From) && std::is_trivially_copyable_v<From> && std::is_trivially_copyable_v<To>, To>
vvl_bit_cast(const From &src) noexcept {
    static_assert(std::is_trivially_constructible_v<To>,
                  "This implementation additionally requires "
                  "destination type to be trivially constructible");
    To dst;
    std::memcpy(&dst, &src, sizeof(To));
    return dst;
}

// Ensure that the size changing casts are *static* to ensure portability
template <typename HandleType>
static inline HandleType CastFromUint64(u64 untyped_handle) {
    static_assert(sizeof(HandleType) <= sizeof(u64), "HandleType must be not larger than the untyped handle size");
    typedef typename std::conditional<
        sizeof(HandleType) == sizeof(u8), u8,
        typename std::conditional<sizeof(HandleType) == sizeof(u16), u16,
                                  typename std::conditional<sizeof(HandleType) == sizeof(u32), u32, u64>::type>::type>::type Uint;
    return CastFromUint<HandleType, Uint>(static_cast<Uint>(untyped_handle));
}

template <typename HandleType>
static inline u64 CastToUint64(HandleType handle) {
    static_assert(sizeof(HandleType) <= sizeof(u64), "HandleType must be not larger than the untyped handle size");
    typedef typename std::conditional<
        sizeof(HandleType) == sizeof(u8), u8,
        typename std::conditional<sizeof(HandleType) == sizeof(u16), u16,
                                  typename std::conditional<sizeof(HandleType) == sizeof(u32), u32, u64>::type>::type>::type Uint;
    return static_cast<u64>(CastToUint<HandleType, Uint>(handle));
}

// Convenience functions to case between handles and the types the handles abstract, reflecting the Vulkan handle scheme, where
// Handles are either pointers (dispatchable) or sizeof(u64) (non-dispatchable), s.t. full size-safe casts are used and
// we ensure that handles are large enough to contain the underlying type.
template <typename HandleType, typename ValueType>
void CastToHandle(ValueType value, HandleType *handle) {
    static_assert(sizeof(HandleType) >= sizeof(ValueType), "HandleType must large enough to hold internal value");
    *handle = CastFromUint64<HandleType>(CastToUint64<ValueType>(value));
}
// This form is conveniently "inline", you should only need to specify the handle type (the value type being deducible from the arg)
template <typename HandleType, typename ValueType>
HandleType CastToHandle(ValueType value) {
    HandleType handle;
    CastToHandle(value, &handle);
    return handle;
}

template <typename ValueType, typename HandleType>
void CastFromHandle(HandleType handle, ValueType *value) {
    static_assert(sizeof(HandleType) >= sizeof(ValueType), "HandleType must large enough to hold internal value");
    *value = CastFromUint64<ValueType>(CastToUint64<HandleType>(handle));
}
template <typename ValueType, typename HandleType>
ValueType CastFromHandle(HandleType handle) {
    ValueType value;
    CastFromHandle(handle, &value);
    return value;
}
