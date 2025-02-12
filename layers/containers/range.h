/* Copyright (c) 2025 The Khronos Group Inc.
 * Copyright (c) 2025 Valve Corporation
 * Copyright (c) 2025 LunarG, Inc.
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

#include <cstdint>
#include <sstream>

namespace vvl {

template <typename Index>
struct range {
    using index_type = Index;
    index_type begin;  // Inclusive lower bound of range
    index_type end;    // Exlcusive upper bound of range

    bool empty() const { return begin == end; }
    bool valid() const { return begin <= end; }
    bool invalid() const { return !valid(); }
    bool non_empty() const { return begin < end; }  //  valid and !empty

    bool is_prior_to(const range &other) const { return end == other.begin; }
    bool is_subsequent_to(const range &other) const { return begin == other.end; }
    bool includes(const index_type &index) const { return (begin <= index) && (index < end); }
    bool includes(const range &other) const { return (begin <= other.begin) && (other.end <= end); }
    bool excludes(const index_type &index) const { return (index < begin) || (end <= index); }
    bool excludes(const range &other) const { return (other.end <= begin) || (end <= other.begin); }
    bool intersects(const range &other) const { return includes(other.begin) || other.includes(begin); }
    index_type distance() const { return end - begin; }

    bool operator==(const range &rhs) const { return (begin == rhs.begin) && (end == rhs.end); }
    bool operator!=(const range &rhs) const { return (begin != rhs.begin) || (end != rhs.end); }

    range &operator-=(const index_type &offset) {
        begin = begin - offset;
        end = end - offset;
        return *this;
    }

    range &operator+=(const index_type &offset) {
        begin = begin + offset;
        end = end + offset;
        return *this;
    }

    range operator+(const index_type &offset) const { return range(begin + offset, end + offset); }

    // for a reversible/transitive < operator compare first on begin and then end
    // only less or begin is less or if end is less when begin is equal
    bool operator<(const range &rhs) const {
        bool result = false;
        if (invalid()) {
            // all invalid < valid, allows map/set validity check by looking at begin()->first
            // all invalid are equal, thus only equal if this is invalid and rhs is valid
            result = rhs.valid();
        } else if (begin < rhs.begin) {
            result = true;
        } else if ((begin == rhs.begin) && (end < rhs.end)) {
            result = true;  // Simple common case -- boundary case require equality check for correctness.
        }
        return result;
    }

    // use as "strictly less/greater than" to check for non-overlapping ranges
    bool strictly_less(const range &rhs) const { return end <= rhs.begin; }
    bool strictly_less(const index_type &index) const { return end <= index; }
    bool strictly_greater(const range &rhs) const { return rhs.end <= begin; }
    bool strictly_greater(const index_type &index) const { return index < begin; }

    range &operator=(const range &rhs) {
        begin = rhs.begin;
        end = rhs.end;
        return *this;
    }

    // Compute ranges intersection. Returns empty range on non-intersection
    range operator&(const range &rhs) const {
        if (includes(rhs.begin)) {
            return range(rhs.begin, std::min(end, rhs.end));
        } else if (rhs.includes(begin)) {
            return range(begin, std::min(end, rhs.end));
        }
        return range();
    }

    index_type size() const { return end - begin; }
    range() : begin(), end() {}
    range(const index_type &begin_, const index_type &end_) : begin(begin_), end(end_) {}
    range(const range &other) : begin(other.begin), end(other.end) {}
};

template <typename Range>
class range_view {
  public:
    using index_type = typename Range::index_type;
    class iterator {
      public:
        iterator &operator++() {
            ++current;
            return *this;
        }
        const index_type &operator*() const { return current; }
        bool operator!=(const iterator &rhs) const { return current != rhs.current; }
        iterator(index_type value) : current(value) {}

      private:
        index_type current;
    };
    range_view(const Range &range) : range_(range) {}
    const iterator begin() const { return iterator(range_.begin); }
    const iterator end() const { return iterator(range_.end); }

  private:
    const Range &range_;
};

template <typename Range>
std::string string_range(const Range &range) {
    std::stringstream ss;
    ss << "[" << range.begin << ", " << range.end << ')';
    return ss.str();
}

template <typename Range>
std::string string_range_hex(const Range &range) {
    std::stringstream ss;
    ss << std::hex << "[0x" << range.begin << ", 0x" << range.end << ')';
    return ss.str();
}

}  // namespace vvl

// Returns the intersection of the ranges [x, x + x_size) and [y, y + y_size)
static inline vvl::range<int64_t> GetRangeIntersection(int64_t x, uint64_t x_size, int64_t y, uint64_t y_size) {
    int64_t intersection_min = std::max(x, y);
    int64_t intersection_max = std::min(x + static_cast<int64_t>(x_size), y + static_cast<int64_t>(y_size));

    return {intersection_min, intersection_max};
}
