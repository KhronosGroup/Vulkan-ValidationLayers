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
#include "error_location.h"
#include "utils/vk_layer_utils.h"
#include <map>

void Location::AppendFields(std::ostream& out) const {
    if (prev) {
        prev->AppendFields(out);
        const bool needs_dot = prev->structure != vvl::Struct::Empty || prev->field != vvl::Field::Empty;
        if (needs_dot) {
            out << ".";
        }
    }
    if (field != vvl::Field::Empty) {
        if (isPNext && structure != vvl::Struct::Empty) {
            out << "pNext<" << vvl::String(structure) << ">.";
        }
        out << vvl::String(field);
        if (index != Location::kNoIndex) {
            out << "[" << index << "]";
        }
    }
}

std::string Location::Fields() const {
    std::stringstream out;
    AppendFields(out);
    return out.str();
}

std::string Location::Message() const {
    std::stringstream out;
    out << StringFunc() << "(): ";
    AppendFields(out);
    return out.str();
}

namespace vvl {
LocationCapture::LocationCapture(const Location& loc) { Capture(loc, 1); }

const Location* LocationCapture::Capture(const Location& loc, CaptureStore::size_type depth) {
    const Location* prev_capture = nullptr;
    if (loc.prev) {
        prev_capture = Capture(*loc.prev, depth + 1);
    } else {
        capture.reserve(depth);
    }

    capture.emplace_back(loc);
    capture.back().prev = prev_capture;
    return &(capture.back());
}

bool operator<(const Key& lhs, const Key& rhs) {
    if (lhs.function < rhs.function) {
        return true;
    } else if (lhs.function > rhs.function) {
        return false;
    }

    if (lhs.structure < rhs.structure) {
        return true;
    } else if (lhs.structure > rhs.structure) {
        return false;
    }

    if (lhs.field < rhs.field) {
        return true;
    } else if (lhs.field > rhs.field) {
        return false;
    }

    if (lhs.recurse_field < rhs.recurse_field) {
        return true;
    } else if (lhs.recurse_field > rhs.recurse_field) {
        return false;
    }

    return false;
}

bool operator==(const Key& key, const Location& loc) {
    assert(key.function != Func::Empty || key.structure != Struct::Empty);
    assert(loc.function != Func::Empty);
    if (key.function != Func::Empty) {
        if (key.function != loc.function) {
            return false;
        }
    }
    if (key.structure != Struct::Empty) {
        if (key.structure != loc.structure) {
            return false;
        }
    }
    if (key.field == Field::Empty) {
        return true;
    }
    if (key.field == loc.field) {
        return true;
    }
    if (key.recurse_field) {
        const Location *prev = loc.prev;
        while (prev != nullptr) {
            if (key.field == prev->field) {
                return true;
            }
            prev = prev->prev;
        }
    }
    return false;
}

}  // namespace vvl