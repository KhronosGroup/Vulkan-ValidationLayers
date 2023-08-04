/* Copyright (c) 2021-2022 The Khronos Group Inc.
 * Copyright (c) 2021-2023 Valve Corporation
 * Copyright (c) 2021-2023 LunarG, Inc.
 * Copyright (C) 2021-2022 Google Inc.
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
#include <string>
#include <sstream>
#include <limits>

#include "generated/error_location_helper.h"
#include "containers/custom_containers.h"
namespace core_error {

struct Location {
    static const uint32_t kNoIndex = vvl::kU32Max;

    // name of the vulkan function we're checking
    Func function;

    Struct structure;
    Field field;
    // optional index if checking an array.
    uint32_t index;
    const Location* prev;

    Location(Func func, Struct s, Field f = Field::Empty, uint32_t i = kNoIndex)
        : function(func), structure(s), field(f), index(i), prev(nullptr) {}
    Location(Func func, Field f = Field::Empty, uint32_t i = kNoIndex)
        : function(func), structure(Struct::Empty), field(f), index(i), prev(nullptr) {}
    Location(const Location& prev_loc, Struct s, Field f, uint32_t i)
        : function(prev_loc.function), structure(s), field(f), index(i), prev(&prev_loc) {}

    void AppendFields(std::ostream &out) const;
    std::string Fields() const {
        std::stringstream out;
        AppendFields(out);
        return out.str();
    }
    std::string Message() const {
        std::stringstream out;
        out << StringFunc() << "(): ";
        AppendFields(out);
        return out.str();
    }

    // the dot() method is for walking down into a structure that is being validated
    // eg:  loc.dot(Field::pMemoryBarriers, 5).dot(Field::srcStagemask)
    Location dot(Struct s, Field sub_field, uint32_t sub_index = kNoIndex) const {
        Location result(*this, s, sub_field, sub_index);
        return result;
    }
    Location dot(Field sub_field, uint32_t sub_index = kNoIndex) const {
        Location result(*this, this->structure, sub_field, sub_index);
        return result;
    }

    const char* StringFunc() const { return String(function); }
    const char* StringStruct() const { return String(structure); }
    const char* StringField() const { return String(field); }
};

template <typename VuidFunctor>
struct LocationVuidAdapter {
    const Location loc;
    VuidFunctor vuid_functor;
    const char* FuncName() const {
        // the returned reference from loc must be valid for lifespan of loc, at least.
        return loc.StringFunc();
    }
    const char* Vuid() const {
        // the returned reference from functor must be valid for lifespan of vuid_functor, at least.
        const std::string& vuid = vuid_functor(loc);
        return vuid.c_str();
    }
    template <typename... Args>
    LocationVuidAdapter(const Location& loc_, const Args&... args) : loc(loc_), vuid_functor(args...) {}
};

struct LocationCapture {
    LocationCapture(const Location& loc);
    const Location& Get() const { return capture.back(); }

  protected:
    // TODO: Optimize this for "new" minimization
    using CaptureStore = small_vector<Location, 2>;
    const Location* Capture(const Location& loc, CaptureStore::size_type depth);
    CaptureStore capture;
};

// Key for use in tables of VUIDs.
//
// Fuzzy match rules:
//  key.function OR key.structure may be Empty
//  loc.structure may be Empty
//  key.field may be Empty
//  if key.recurse_field is true, key.field can match loc.field or any fields in loc.prev
//
struct Key {
    // If a new member is added, update operator<
    Func function;
    Struct structure;
    Field field;
    bool recurse_field;
    Key(Struct r, Field f = Field::Empty, bool recurse = false)
        : function(Func::Empty), structure(r), field(f), recurse_field(recurse) {}
    Key(Func fn, Field f = Field::Empty, bool recurse = false)
        : function(fn), structure(Struct::Empty), field(f), recurse_field(recurse) {}
};

bool operator<(const Key& lhs, const Key& rhs);
bool operator==(const Key& key, const Location& loc);

// Entry in a VUID lookup table
struct Entry {
    Key k;
    std::string v;
};

// look for a matching VUID in a vector or array-ish table
template <typename Table>
static const std::string& FindVUID(const Location& loc, const Table& table) {
    static const std::string empty;
    auto predicate = [&loc](const Entry& entry) { return entry.k == loc; };

    // consistency check: there should never be more than 1 match in a table
    assert(std::count_if(table.begin(), table.end(), predicate) <= 1);

    const auto pos = std::find_if(table.begin(), table.end(), predicate);
    return (pos != table.end()) ? pos->v : empty;
}

// 2-level look up where the outer container is a map where we need to find
// different VUIDs for different values of an enum or bitfield
template <typename OuterKey, typename Table>
static const std::string& FindVUID(OuterKey key, const Location& loc, const Table& table) {
    static const std::string empty;
    const auto entry = table.find(key);
    if (entry != table.end()) {
        return FindVUID(loc, entry->second);
    }
    return empty;
}

}  // namespace core_error
