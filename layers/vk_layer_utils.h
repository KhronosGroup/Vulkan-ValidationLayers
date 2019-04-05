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
 * Author: Mark Lobodzinski <mark@lunarg.com>
 * Author: Courtney Goeltzenleuchter <courtney@LunarG.com>
 * Author: Dave Houlton <daveh@lunarg.com>
 */

#pragma once

#include <cassert>
#include <cstddef>
#include <functional>
#include <stdbool.h>
#include <string>
#include <vector>
#include <set>
#include "vk_format_utils.h"
#include "vk_layer_logging.h"

#ifndef WIN32
#include <strings.h>  // For ffs()
#else
#include <intrin.h>  // For __lzcnt()
#endif

#ifdef __cplusplus
// Traits objects to allow string_join to operate on collections of const char *
template <typename String>
struct StringJoinSizeTrait {
    static size_t size(const String &str) { return str.size(); }
};

template <>
struct StringJoinSizeTrait<const char *> {
    static size_t size(const char *str) {
        if (!str) return 0;
        return strlen(str);
    }
};
// Similar to perl/python join
//    * String must support size, reserve, append, and be default constructable
//    * StringCollection must support size, const forward iteration, and store
//      strings compatible with String::append
//    * Accessor trait can be set if default accessors (compatible with string
//      and const char *) don't support size(StringCollection::value_type &)
//
// Return type based on sep type
template <typename String = std::string, typename StringCollection = std::vector<String>,
          typename Accessor = StringJoinSizeTrait<typename StringCollection::value_type>>
static inline String string_join(const String &sep, const StringCollection &strings) {
    String joined;
    const size_t count = strings.size();
    if (!count) return joined;

    // Prereserved storage, s.t. we will execute in linear time (avoids reallocation copies)
    size_t reserve = (count - 1) * sep.size();
    for (const auto &str : strings) {
        reserve += Accessor::size(str);  // abstracted to allow const char * type in StringCollection
    }
    joined.reserve(reserve + 1);

    // Seps only occur *between* strings entries, so first is special
    auto current = strings.cbegin();
    joined.append(*current);
    ++current;
    for (; current != strings.cend(); ++current) {
        joined.append(sep);
        joined.append(*current);
    }
    return joined;
}

// Requires StringCollection::value_type has a const char * constructor and is compatible the string_join::String above
template <typename StringCollection = std::vector<std::string>, typename SepString = std::string>
static inline SepString string_join(const char *sep, const StringCollection &strings) {
    return string_join<SepString, StringCollection>(SepString(sep), strings);
}

// Perl/Python style join operation for general types using stream semantics
// Note: won't be as fast as string_join above, but simpler to use (and code)
// Note: Modifiable reference doesn't match the google style but does match std style for stream handling and algorithms
template <typename Stream, typename String, typename ForwardIt>
Stream &stream_join(Stream &stream, const String &sep, ForwardIt first, ForwardIt last) {
    if (first != last) {
        stream << *first;
        ++first;
        while (first != last) {
            stream << sep << *first;
            ++first;
        }
    }
    return stream;
}

// stream_join For whole collections with forward iterators
template <typename Stream, typename String, typename Collection>
Stream &stream_join(Stream &stream, const String &sep, const Collection &values) {
    return stream_join(stream, sep, values.cbegin(), values.cend());
}

typedef void *dispatch_key;
static inline dispatch_key get_dispatch_key(const void *object) { return (dispatch_key) * (VkLayerDispatchTable **)object; }

VK_LAYER_EXPORT VkLayerInstanceCreateInfo *get_chain_info(const VkInstanceCreateInfo *pCreateInfo, VkLayerFunction func);
VK_LAYER_EXPORT VkLayerDeviceCreateInfo *get_chain_info(const VkDeviceCreateInfo *pCreateInfo, VkLayerFunction func);

static inline bool IsPowerOfTwo(unsigned x) { return x && !(x & (x - 1)); }

// Casts to allow various types of less than 64 bits to be cast to and from uint64_t safely and portably
template <typename HandleType, typename Uint>
static inline HandleType CastFromUint(Uint untyped_handle) {
    static_assert(sizeof(HandleType) == sizeof(Uint), "HandleType must be the same size as untyped handle");
    return *reinterpret_cast<HandleType *>(&untyped_handle);
}
template <typename HandleType, typename Uint>
static inline Uint CastToUint(HandleType handle) {
    static_assert(sizeof(HandleType) == sizeof(Uint), "HandleType must be the same size as untyped handle");
    return *reinterpret_cast<Uint *>(&handle);
}

// Ensure that the size changing casts are *static* to ensure portability
template <typename HandleType>
static inline HandleType CastFromUint64(uint64_t untyped_handle) {
    static_assert(sizeof(HandleType) <= sizeof(uint64_t), "HandleType must be not larger than the untyped handle size");
    // Since size mismatched reinterpret casts are strongly non-portable we use std::condtional to find the appropriate
    // unsigned integer type for the reinterpret cast we are using.  C++11 doesn't have anything like a switch, for the type
    // conditionals, so the various cases are nested in the "false" type of std::conditional.
    // The formatting makes it clear, but each indent is else if.
    typedef
        typename std::conditional<sizeof(HandleType) == sizeof(uint8_t), uint8_t,
                                  typename std::conditional<sizeof(HandleType) == sizeof(uint16_t), uint16_t,
                                                            typename std::conditional<sizeof(HandleType) == sizeof(uint32_t),
                                                                                      uint32_t, uint64_t>::type>::type>::type Uint;
    return CastFromUint<HandleType, Uint>(static_cast<Uint>(untyped_handle));
}

template <typename HandleType>
static uint64_t CastToUint64(HandleType handle) {
    static_assert(sizeof(HandleType) <= sizeof(uint64_t), "HandleType must be not larger than the untyped handle size");
    typedef
        typename std::conditional<sizeof(HandleType) == sizeof(uint8_t), uint8_t,
                                  typename std::conditional<sizeof(HandleType) == sizeof(uint16_t), uint16_t,
                                                            typename std::conditional<sizeof(HandleType) == sizeof(uint32_t),
                                                                                      uint32_t, uint64_t>::type>::type>::type Uint;
    return static_cast<uint64_t>(CastToUint<HandleType, Uint>(handle));
}

// Convenience functions to case between handles and the types the handles abstract, reflecting the Vulkan handle scheme, where
// Handles are either pointers (dispatchable) or sizeof(uint64_t) (non-dispatchable), s.t. full size-safe casts are used and
// we ensure that handles are large enough to contain the underlying type.
template <typename HandleType, typename ValueType>
void CastToHandle(ValueType value, HandleType *handle) {
    static_assert(sizeof(HandleType) >= sizeof(ValueType), "HandleType must large enough to hold internal value");
    *handle = CastFromUint64<HandleType>(CastToUint64<ValueType>(value));
}
// This form is conveniently "inline" but inconveniently requires both template arguments.
template <typename HandleType, typename ValueType>
HandleType CastToHandle(ValueType value) {
    HandleType handle;
    CastToHandle(value, &handle);
    return handle;
}

template <typename HandleType, typename ValueType>
void CastFromHandle(HandleType handle, ValueType *value) {
    static_assert(sizeof(HandleType) >= sizeof(ValueType), "HandleType must large enough to hold internal value");
    *value = CastFromUint64<ValueType>(CastToUint64<HandleType>(handle));
}
template <typename HandleType, typename ValueType>
ValueType CastFromHandle(HandleType handle) {
    ValueType value;
    CastFromHandle(handle, &value);
    return value;
}

extern "C" {
#endif

#define VK_LAYER_API_VERSION VK_MAKE_VERSION(1, 0, VK_HEADER_VERSION)

typedef enum VkStringErrorFlagBits {
    VK_STRING_ERROR_NONE = 0x00000000,
    VK_STRING_ERROR_LENGTH = 0x00000001,
    VK_STRING_ERROR_BAD_DATA = 0x00000002,
} VkStringErrorFlagBits;
typedef VkFlags VkStringErrorFlags;

VK_LAYER_EXPORT void layer_debug_report_actions(debug_report_data *report_data,
                                                std::vector<VkDebugReportCallbackEXT> &logging_callback,
                                                const VkAllocationCallbacks *pAllocator, const char *layer_identifier);

VK_LAYER_EXPORT void layer_debug_messenger_actions(debug_report_data *report_data,
                                                   std::vector<VkDebugUtilsMessengerEXT> &logging_messenger,
                                                   const VkAllocationCallbacks *pAllocator, const char *layer_identifier);

VK_LAYER_EXPORT VkStringErrorFlags vk_string_validate(const int max_length, const char *char_array);
VK_LAYER_EXPORT bool white_list(const char *item, const std::set<std::string> &whitelist);

static inline int u_ffs(int val) {
#ifdef WIN32
    unsigned long bit_pos = 0;
    if (_BitScanForward(&bit_pos, val) != 0) {
        bit_pos += 1;
    }
    return bit_pos;
#else
    return ffs(val);
#endif
}

#ifdef __cplusplus
}
#endif
