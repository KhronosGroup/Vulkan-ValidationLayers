/* Copyright (c) 2015-2017, 2019-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2017, 2019-2023 Valve Corporation
 * Copyright (c) 2015-2017, 2019-2023 LunarG, Inc.
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

#include <cassert>
#include <cstddef>
#include <cstring>
#include <functional>
#include <string>
#include <vector>
#include <bitset>
#include <shared_mutex>

#include <vulkan/utility/vk_format_utils.h>

#include "cast_utils.h"
#include "generated/vk_extension_helper.h"
#include "error_message/logging.h"

#ifndef WIN32
#include <strings.h>  // For ffs()
#else
#include <intrin.h>  // For __lzcnt()
#endif

#define STRINGIFY(s) STRINGIFY_HELPER(s)
#define STRINGIFY_HELPER(s) #s

#if defined __PRETTY_FUNCTION__
#define VVL_PRETTY_FUNCTION __PRETTY_FUNCTION__
#else
// For MSVC
#if defined(__FUNCSIG__)
#define VVL_PRETTY_FUNCTION __FUNCSIG__
#else
#define VVL_PRETTY_FUNCTION __FILE__ ":" STRINGIFY(__LINE__)
#endif
#endif

static inline VkExtent3D CastTo3D(const VkExtent2D &d2) {
    VkExtent3D d3 = {d2.width, d2.height, 1};
    return d3;
}

static inline VkOffset3D CastTo3D(const VkOffset2D &d2) {
    VkOffset3D d3 = {d2.x, d2.y, 0};
    return d3;
}

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

static inline std::string string_trim(const std::string &s) {
    const char *whitespace = " \t\f\v\n\r";

    const auto trimmed_beg = s.find_first_not_of(whitespace);
    if (trimmed_beg == std::string::npos) return "";

    const auto trimmed_end = s.find_last_not_of(whitespace);
    assert(trimmed_end != std::string::npos && trimmed_beg <= trimmed_end);

    return s.substr(trimmed_beg, trimmed_end - trimmed_beg + 1);
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

VkLayerInstanceCreateInfo *get_chain_info(const VkInstanceCreateInfo *pCreateInfo, VkLayerFunction func);
VkLayerDeviceCreateInfo *get_chain_info(const VkDeviceCreateInfo *pCreateInfo, VkLayerFunction func);

template <typename T>
constexpr bool IsPowerOfTwo(T x) {
    static_assert(std::numeric_limits<T>::is_integer, "Unsigned integer required.");
    static_assert(std::is_unsigned<T>::value, "Unsigned integer required.");
    return x && !(x & (x - 1));
}

// Returns the 0-based index of the MSB, like the x86 bit scan reverse (bsr) instruction
// Note: an input mask of 0 yields -1
static inline int MostSignificantBit(uint32_t mask) {
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

// Compute a binomial coefficient
template <typename T>
constexpr T binom(T n, T k) {
    static_assert(std::numeric_limits<T>::is_integer, "Unsigned integer required.");
    static_assert(std::is_unsigned<T>::value, "Unsigned integer required.");
    assert(n >= k);
    if (n == 0) {
        return 0;
    }
    if (k == 0) {
        return 1;
    }

    T numerator = 1;
    T denominator = 1;
    for (T i = 1; i <= k; ++i) {
        numerator *= n - i + 1;
        denominator *= i;
    }

    return numerator / denominator;
}

template <typename FlagBits, typename Flags>
FlagBits LeastSignificantFlag(Flags flags) {
    const int bit_shift = LeastSignificantBit(flags);
    assert(bit_shift != -1);
    return static_cast<FlagBits>(1ull << bit_shift);
}

// Iterates over all set bits and calls the callback with a bit mask corresponding to each flag.
// FlagBits and Flags follow Vulkan naming convensions for flag types.
// An example of a more efficient implementation: https://lemire.me/blog/2018/02/21/iterating-over-set-bits-quickly/
template <typename FlagBits, typename Flags, typename Callback>
void IterateFlags(Flags flags, Callback callback) {
    uint32_t bit_shift = 0;
    while (flags) {
        if (flags & 1) {
            callback(static_cast<FlagBits>(1ull << bit_shift));
        }
        flags >>= 1;
        ++bit_shift;
    }
}

static inline uint32_t SampleCountSize(VkSampleCountFlagBits sample_count) {
    uint32_t size = 0;
    switch (sample_count) {
        case VK_SAMPLE_COUNT_1_BIT:
            size = 1;
            break;
        case VK_SAMPLE_COUNT_2_BIT:
            size = 2;
            break;
        case VK_SAMPLE_COUNT_4_BIT:
            size = 4;
            break;
        case VK_SAMPLE_COUNT_8_BIT:
            size = 8;
            break;
        case VK_SAMPLE_COUNT_16_BIT:
            size = 16;
            break;
        case VK_SAMPLE_COUNT_32_BIT:
            size = 32;
            break;
        case VK_SAMPLE_COUNT_64_BIT:
            size = 64;
            break;
        default:
            size = 0;
    }
    return size;
}

static inline bool IsImageLayoutReadOnly(VkImageLayout layout) {
    constexpr std::array read_only_layouts = {
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
    };
    return std::any_of(read_only_layouts.begin(), read_only_layouts.end(),
                       [layout](const VkImageLayout read_only_layout) { return layout == read_only_layout; });
}

static inline bool IsImageLayoutDepthOnly(VkImageLayout layout) {
    constexpr std::array depth_only_layouts = {VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL};
    return std::any_of(depth_only_layouts.begin(), depth_only_layouts.end(),
                       [layout](const VkImageLayout read_only_layout) { return layout == read_only_layout; });
}

static inline bool IsImageLayoutDepthReadOnly(VkImageLayout layout) {
    constexpr std::array read_only_layouts = {
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
    };
    return std::any_of(read_only_layouts.begin(), read_only_layouts.end(),
                       [layout](const VkImageLayout read_only_layout) { return layout == read_only_layout; });
}

static inline bool IsImageLayoutStencilOnly(VkImageLayout layout) {
    constexpr std::array depth_only_layouts = {VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL,
                                               VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL};
    return std::any_of(depth_only_layouts.begin(), depth_only_layouts.end(),
                       [layout](const VkImageLayout read_only_layout) { return layout == read_only_layout; });
}

static inline bool IsImageLayoutStencilReadOnly(VkImageLayout layout) {
    constexpr std::array read_only_layouts = {
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
    };
    return std::any_of(read_only_layouts.begin(), read_only_layouts.end(),
                       [layout](const VkImageLayout read_only_layout) { return layout == read_only_layout; });
}

static inline bool IsIdentitySwizzle(VkComponentMapping components) {
    // clang-format off
    return (
        ((components.r == VK_COMPONENT_SWIZZLE_IDENTITY) || (components.r == VK_COMPONENT_SWIZZLE_R)) &&
        ((components.g == VK_COMPONENT_SWIZZLE_IDENTITY) || (components.g == VK_COMPONENT_SWIZZLE_G)) &&
        ((components.b == VK_COMPONENT_SWIZZLE_IDENTITY) || (components.b == VK_COMPONENT_SWIZZLE_B)) &&
        ((components.a == VK_COMPONENT_SWIZZLE_IDENTITY) || (components.a == VK_COMPONENT_SWIZZLE_A))
    );
    // clang-format on
}

static inline uint32_t GetIndexAlignment(VkIndexType indexType) {
    switch (indexType) {
        case VK_INDEX_TYPE_UINT16:
            return 2;
        case VK_INDEX_TYPE_UINT32:
            return 4;
        case VK_INDEX_TYPE_UINT8_EXT:
            return 1;
        case VK_INDEX_TYPE_NONE_KHR:  // alias VK_INDEX_TYPE_NONE_NV
            return 0;
        default:
            // Not a real index type. Express no alignment requirement here; we expect upper layer
            // to have already picked up on the enum being nonsense.
            return 1;
    }
}

// vkspec.html#formats-planes-image-aspect
static inline bool IsValidPlaneAspect(VkFormat format, VkImageAspectFlags aspect_mask) {
    const uint32_t planes = vkuFormatPlaneCount(format);
    constexpr VkImageAspectFlags valid_planes =
        VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT | VK_IMAGE_ASPECT_PLANE_2_BIT;

    if (((aspect_mask & valid_planes) == aspect_mask) && (aspect_mask != 0)) {
        if ((planes == 3) || ((planes == 2) && ((aspect_mask & VK_IMAGE_ASPECT_PLANE_2_BIT) == 0))) {
            return true;
        }
    }
    return false;  // Expects calls to make sure it is a multi-planar format
}

static inline bool IsOnlyOneValidPlaneAspect(VkFormat format, VkImageAspectFlags aspect_mask) {
    const bool multiple_bits = aspect_mask != 0 && !IsPowerOfTwo(aspect_mask);
    return !multiple_bits && IsValidPlaneAspect(format, aspect_mask);
}

static inline bool IsMultiplePlaneAspect(VkImageAspectFlags aspect_mask) {
    // If checking for multiple planes, there will already be another check if valid for plane count
    constexpr VkImageAspectFlags valid_planes =
        VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT | VK_IMAGE_ASPECT_PLANE_2_BIT;
    const VkImageAspectFlags planes = aspect_mask & valid_planes;
    return planes != 0 && !IsPowerOfTwo(planes);
}

static inline bool IsAnyPlaneAspect(VkImageAspectFlags aspect_mask) {
    constexpr VkImageAspectFlags valid_planes =
        VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT | VK_IMAGE_ASPECT_PLANE_2_BIT;
    return (aspect_mask & valid_planes) != 0;
}

// all "advanced blend operation" found in spec
static inline bool IsAdvanceBlendOperation(const VkBlendOp blend_op) {
    return (static_cast<int>(blend_op) >= VK_BLEND_OP_ZERO_EXT) && (static_cast<int>(blend_op) <= VK_BLEND_OP_BLUE_EXT);
}

// Helper for Dual-Source Blending
static inline bool IsSecondaryColorInputBlendFactor(VkBlendFactor blend_factor) {
    return (blend_factor == VK_BLEND_FACTOR_SRC1_COLOR || blend_factor == VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR ||
            blend_factor == VK_BLEND_FACTOR_SRC1_ALPHA || blend_factor == VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA);
}

// Check if size is in range
static inline bool IsBetweenInclusive(VkDeviceSize value, VkDeviceSize min, VkDeviceSize max) {
    return (value >= min) && (value <= max);
}

static inline bool IsBetweenInclusive(const VkExtent2D &value, const VkExtent2D &min, const VkExtent2D &max) {
    return IsBetweenInclusive(value.width, min.width, max.width) && IsBetweenInclusive(value.height, min.height, max.height);
}

static inline bool IsBetweenInclusive(float value, float min, float max) { return (value >= min) && (value <= max); }

// Check if value is integer multiple of granularity
static inline bool IsIntegerMultipleOf(VkDeviceSize value, VkDeviceSize granularity) {
    if (granularity == 0) {
        return value == 0;
    } else {
        return (value % granularity) == 0;
    }
}

static inline bool IsIntegerMultipleOf(const VkOffset2D &value, const VkOffset2D &granularity) {
    return IsIntegerMultipleOf(value.x, granularity.x) && IsIntegerMultipleOf(value.y, granularity.y);
}

// Perform a zero-tolerant modulo operation
static inline VkDeviceSize SafeModulo(VkDeviceSize dividend, VkDeviceSize divisor) {
    VkDeviceSize result = 0;
    if (divisor != 0) {
        result = dividend % divisor;
    }
    return result;
}

static inline VkDeviceSize SafeDivision(VkDeviceSize dividend, VkDeviceSize divisor) {
    VkDeviceSize result = 0;
    if (divisor != 0) {
        result = dividend / divisor;
    }
    return result;
}

inline std::optional<VkDeviceSize> ComputeValidSize(VkDeviceSize offset, VkDeviceSize size, VkDeviceSize whole_size) {
    std::optional<VkDeviceSize> valid_size;
    if (offset < whole_size) {
        if (size == VK_WHOLE_SIZE) {
            valid_size.emplace(whole_size - offset);
        } else if ((offset + size) <= whole_size) {
            valid_size.emplace(size);
        }
    }
    return valid_size;
}

// Only 32 bit fields should need a bit count
static inline uint32_t GetBitSetCount(uint32_t field) {
    std::bitset<32> view_bits(field);
    return static_cast<uint32_t>(view_bits.count());
}

static inline uint32_t FullMipChainLevels(VkExtent3D extent) {
    // uint cast applies floor()
    return 1u + static_cast<uint32_t>(log2(std::max({extent.height, extent.width, extent.depth})));
}

// Returns the effective extent of an image subresource, adjusted for mip level and array depth.
[[nodiscard]] inline VkExtent3D GetEffectiveExtent(const VkImageCreateInfo &ci, const VkImageAspectFlags aspect_mask,
                                                   const uint32_t mip_level) {
    // Return zero extent if mip level doesn't exist
    if (mip_level >= ci.mipLevels) {
        return VkExtent3D{0, 0, 0};
    }

    VkExtent3D extent = ci.extent;

    // If multi-plane, adjust per-plane extent
    const VkFormat format = ci.format;
    if (vkuFormatIsMultiplane(format)) {
        VkExtent2D divisors = vkuFindMultiplaneExtentDivisors(format, static_cast<VkImageAspectFlagBits>(aspect_mask));
        extent.width /= divisors.width;
        extent.height /= divisors.height;
    }

    // Mip Maps
    {
        const uint32_t corner = (ci.flags & VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV) ? 1 : 0;
        const uint32_t min_size = 1 + corner;
        const std::array dimensions = {&extent.width, &extent.height, &extent.depth};
        for (uint32_t *dim : dimensions) {
            // Don't allow mip adjustment to create 0 dim, but pass along a 0 if that's what subresource specified
            if (*dim == 0) {
                continue;
            }
            *dim >>= mip_level;
            *dim = std::max(min_size, *dim);
        }
    }

    // Image arrays have an effective z extent that isn't diminished by mip level
    if (VK_IMAGE_TYPE_3D != ci.imageType) {
        extent.depth = ci.arrayLayers;
    }

    return extent;
}

// Returns the effective extent of an image subresource, adjusted for mip level and array depth.
[[nodiscard]] inline VkExtent3D GetEffectiveExtent(const VkImageCreateInfo &ci, const VkImageSubresourceRange &range) {
    return GetEffectiveExtent(ci, range.aspectMask, range.baseMipLevel);
}

// Calculates the number of mip levels a VkImageView references.
constexpr uint32_t ResolveRemainingLevels(const VkImageCreateInfo &ci, VkImageSubresourceRange const &range) {
    return (range.levelCount == VK_REMAINING_MIP_LEVELS) ? (ci.mipLevels - range.baseMipLevel) : range.levelCount;
}

// Calculates the number of mip layers a VkImageView references.
constexpr uint32_t ResolveRemainingLayers(const VkImageCreateInfo &ci, VkImageSubresourceRange const &range) {
    return (range.layerCount == VK_REMAINING_ARRAY_LAYERS) ? (ci.arrayLayers - range.baseArrayLayer) : range.layerCount;
}

// Used to get the VkExternalFormatANDROID without having to use ifdef in logic
// Result of zero is same of not having pNext struct
constexpr uint64_t GetExternalFormat(const void *pNext) {
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    if (pNext) {
        const auto *external_format = vku::FindStructInPNextChain<VkExternalFormatANDROID>(pNext);
        if (external_format) {
            return external_format->externalFormat;
        }
    }
#endif
    (void)pNext;
    return 0;
}

// Find whether or not an element is in list
// Two definitions, to be able to do the following calls:
// IsValueIn(1, {1, 2, 3});
// std::array arr {1, 2, 3};
// IsValueIn(1, arr);
template <typename T, typename RANGE>
bool IsValueIn(const T &v, const RANGE &range) {
    return std::find(std::begin(range), std::end(range), v) != std::end(range);
}

template <typename T>
bool IsValueIn(const T &v, const std::initializer_list<T> &list) {
    return IsValueIn<T, decltype(list)>(v, list);
}

#define VK_LAYER_API_VERSION VK_HEADER_VERSION_COMPLETE

typedef enum VkStringErrorFlagBits {
    VK_STRING_ERROR_NONE = 0x00000000,
    VK_STRING_ERROR_LENGTH = 0x00000001,
    VK_STRING_ERROR_BAD_DATA = 0x00000002,
} VkStringErrorFlagBits;
typedef VkFlags VkStringErrorFlags;

void layer_debug_messenger_actions(debug_report_data *report_data, const char *layer_identifier);

VkStringErrorFlags vk_string_validate(const int max_length, const char *char_array);
bool white_list(const char *item, const std::set<std::string> &whitelist);
std::string GetTempFilePath();

// Aliases to avoid excessive typing. We can't easily auto these away because
// there are virtual methods in ValidationObject which return lock guards
// and those cannot use return type deduction.
typedef std::shared_lock<std::shared_mutex> ReadLockGuard;
typedef std::unique_lock<std::shared_mutex> WriteLockGuard;

// helper class for the very common case of getting and then locking a command buffer (or other state object)
template <typename T, typename Guard>
class LockedSharedPtr : public std::shared_ptr<T> {
  public:
    LockedSharedPtr(std::shared_ptr<T> &&ptr, Guard &&guard) : std::shared_ptr<T>(std::move(ptr)), guard_(std::move(guard)) {}
    LockedSharedPtr() : std::shared_ptr<T>(), guard_() {}

  private:
    Guard guard_;
};

// https://en.cppreference.com/w/cpp/thread/hardware_destructive_interference_size
// https://en.wikipedia.org/wiki/False_sharing
// TODO use C++20 to check for std::hardware_destructive_interference_size feature support.
constexpr std::size_t get_hardware_destructive_interference_size() { return 64; }

// Limited concurrent_unordered_map that supports internally-synchronized
// insert/erase/access. Splits locking across N buckets and uses shared_mutex
// for read/write locking. Iterators are not supported. The following
// operations are supported:
//
// insert_or_assign: Insert a new element or update an existing element.
// insert: Insert a new element and return whether it was inserted.
// erase: Remove an element.
// contains: Returns true if the key is in the map.
// find: Returns != end() if found, value is in ret->second.
// pop: Erases and returns the erased value if found.
//
// find/end: find returns a vaguely iterator-like type that can be compared to
// end and can use iter->second to retrieve the reference. This is to ease porting
// for existing code that combines the existence check and lookup in a single
// operation (and thus a single lock). i.e.:
//
//      auto iter = map.find(key);
//      if (iter != map.end()) {
//          T t = iter->second;
//          ...
//
// snapshot: Return an array of elements (key, value pairs) that satisfy an optional
// predicate. This can be used as a substitute for iterators in exceptional cases.
template <typename Key, typename T, int BUCKETSLOG2 = 2, typename Hash = vvl::hash<Key>>
class vl_concurrent_unordered_map {
  public:
    template <typename... Args>
    void insert_or_assign(const Key &key, Args &&...args) {
        uint32_t h = ConcurrentMapHashObject(key);
        WriteLockGuard lock(locks[h].lock);
        maps[h][key] = {std::forward<Args>(args)...};
    }

    template <typename... Args>
    bool insert(const Key &key, Args &&...args) {
        uint32_t h = ConcurrentMapHashObject(key);
        WriteLockGuard lock(locks[h].lock);
        auto ret = maps[h].emplace(key, std::forward<Args>(args)...);
        return ret.second;
    }

    // returns size_type
    size_t erase(const Key &key) {
        uint32_t h = ConcurrentMapHashObject(key);
        WriteLockGuard lock(locks[h].lock);
        return maps[h].erase(key);
    }

    bool contains(const Key &key) const {
        uint32_t h = ConcurrentMapHashObject(key);
        ReadLockGuard lock(locks[h].lock);
        return maps[h].count(key) != 0;
    }

    // type returned by find() and end().
    class FindResult {
      public:
        FindResult(bool a, T b) : result(a, std::move(b)) {}

        // == and != only support comparing against end()
        bool operator==(const FindResult &other) const {
            if (result.first == false && other.result.first == false) {
                return true;
            }
            return false;
        }
        bool operator!=(const FindResult &other) const { return !(*this == other); }

        // Make -> act kind of like an iterator.
        std::pair<bool, T> *operator->() { return &result; }
        const std::pair<bool, T> *operator->() const { return &result; }

      private:
        // (found, reference to element)
        std::pair<bool, T> result;
    };

    // find()/end() return a FindResult containing a copy of the value. For end(),
    // return a default value.
    FindResult end() const { return FindResult(false, T()); }
    FindResult cend() const { return end(); }

    FindResult find(const Key &key) const {
        uint32_t h = ConcurrentMapHashObject(key);
        ReadLockGuard lock(locks[h].lock);

        auto itr = maps[h].find(key);
        const bool found = itr != maps[h].end();

        if (found) {
            return FindResult(true, itr->second);
        } else {
            return end();
        }
    }

    FindResult pop(const Key &key) {
        uint32_t h = ConcurrentMapHashObject(key);
        WriteLockGuard lock(locks[h].lock);

        auto itr = maps[h].find(key);
        const bool found = itr != maps[h].end();

        if (found) {
            auto ret = FindResult(true, itr->second);
            maps[h].erase(itr);
            return ret;
        } else {
            return end();
        }
    }

    std::vector<std::pair<const Key, T>> snapshot(std::function<bool(T)> f = nullptr) const {
        std::vector<std::pair<const Key, T>> ret;
        for (int h = 0; h < BUCKETS; ++h) {
            ReadLockGuard lock(locks[h].lock);
            for (const auto &j : maps[h]) {
                if (!f || f(j.second)) {
                    ret.emplace_back(j.first, j.second);
                }
            }
        }
        return ret;
    }

    void clear() {
        for (int h = 0; h < BUCKETS; ++h) {
            WriteLockGuard lock(locks[h].lock);
            maps[h].clear();
        }
    }

    size_t size() const {
        size_t result = 0;
        for (int h = 0; h < BUCKETS; ++h) {
            ReadLockGuard lock(locks[h].lock);
            result += maps[h].size();
        }
        return result;
    }

    bool empty() const {
        bool result = 0;
        for (int h = 0; h < BUCKETS; ++h) {
            ReadLockGuard lock(locks[h].lock);
            result |= maps[h].empty();
        }
        return result;
    }

  private:
    static const int BUCKETS = (1 << BUCKETSLOG2);

    vvl::unordered_map<Key, T, Hash> maps[BUCKETS];
    struct alignas(get_hardware_destructive_interference_size()) AlignedSharedMutex {
        std::shared_mutex lock;
    };
    mutable std::array<AlignedSharedMutex, BUCKETS> locks;

    uint32_t ConcurrentMapHashObject(const Key &object) const {
        uint64_t u64 = (uint64_t)(uintptr_t)object;
        uint32_t hash = (uint32_t)(u64 >> 32) + (uint32_t)u64;
        hash ^= (hash >> BUCKETSLOG2) ^ (hash >> (2 * BUCKETSLOG2));
        hash &= (BUCKETS - 1);
        return hash;
    }
};

static constexpr VkPipelineStageFlags2KHR kFramebufferStagePipelineStageFlags =
    (VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
     VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

static constexpr VkAccessFlags2 kShaderTileImageAllowedAccessFlags =
    VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
    VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

static constexpr bool HasNonFramebufferStagePipelineStageFlags(VkPipelineStageFlags2KHR inflags) {
    return (inflags & ~kFramebufferStagePipelineStageFlags) != 0;
}

static constexpr bool HasFramebufferStagePipelineStageFlags(VkPipelineStageFlags2KHR inflags) {
    return (inflags & kFramebufferStagePipelineStageFlags) != 0;
}

static constexpr bool HasNonShaderTileImageAccessFlags(VkAccessFlags2 in_flags) {
    return ((in_flags & ~kShaderTileImageAllowedAccessFlags) != 0);
}

namespace vvl {

static inline void ToLower(std::string &str) {
    // std::tolower() returns int which can cause compiler warnings
    transform(str.begin(), str.end(), str.begin(),
              [](char c) { return static_cast<char>(std::tolower(c)); });
}

static inline void ToUpper(std::string &str) {
    // std::toupper() returns int which can cause compiler warnings
    transform(str.begin(), str.end(), str.begin(),
              [](char c) { return static_cast<char>(std::toupper(c)); });
}

// The standard does not specify the value of data() for zero-sized contatiners as being null or non-null,
// only that it is not dereferenceable.
//
// Vulkan VUID's OTOH frequently require NULLs for zero-sized entries, or for option entries with non-zero counts
template <typename T>
const typename T::value_type *DataOrNull(const T &container) {
    if (!container.empty()) {
        return container.data();
    }
    return nullptr;
}


// Workaround for static_assert(false) before C++ 23 arrives
// https://en.cppreference.com/w/cpp/language/static_assert
// https://cplusplus.github.io/CWG/issues/2518.html
template <typename>
inline constexpr bool dependent_false_v = false;

}  // namespace vvl
