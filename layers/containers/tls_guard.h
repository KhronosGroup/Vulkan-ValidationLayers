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
#include <optional>
#include <utility>

namespace vvl {

// Helper for thread local Validate -> Record phase data
// Define T unique to each entrypoint which will persist data
// Use only in with singleton (leaf) validation objects
// State machine transition state changes of payload relative to TlsGuard object lifecycle:
//  State INIT: bool(payload_)
//  State RESET: NOT bool(payload_)
//    * PreCallValidate* phase
//        * Initialized with skip (in PreCallValidate*)
//            * RESET -> INIT
//        * Destruct with skip == true
//           * INIT -> RESET
//    * PreCallRecord* phase (optional IF PostCallRecord present)
//        * Initialized w/o skip (set "persist_" IFF PostCallRecord present)
//           * Must be in state INIT
//        * Destruct with NOT persist_
//           * INIT -> RESET
//    * PreCallRecord* phase (optional IF PreCallRecord present)
//        * Initialized w/o skip ("persist_" *must be false)
//           * Must be in state INIT
//        * Destruct
//           * INIT -> RESET

struct TlsGuardPersist {};
template <typename T>
class TlsGuard {
  public:
    // For use on inital references -- Validate phase
    template <typename... Args>
    TlsGuard(bool *skip, Args &&...args) : skip_(skip), persist_(false) {
        // Record phase calls are required to clean up payload
        assert(!payload_);
        payload_.emplace(std::forward<Args>(args)...);
    }
    // For use on non-terminal persistent references (PreRecord phase IFF PostRecord is also present.
    TlsGuard(const TlsGuardPersist &) : skip_(nullptr), persist_(true) { assert(payload_); }
    // For use on terminal persistent references
    // Validate phase calls are required to setup payload
    // PreCallRecord calls are required to preserve (persist_) payload, if PostCallRecord calls will use
    TlsGuard() : skip_(nullptr), persist_(false) { assert(payload_); }
    ~TlsGuard() {
        assert(payload_);
        if (!persist_ && (!skip_ || *skip_)) payload_.reset();
    }

    T &operator*() & {
        assert(payload_);
        return *payload_;
    }
    const T &operator*() const & {
        assert(payload_);
        return *payload_;
    }
    T &&operator*() && {
        assert(payload_);
        return std::move(*payload_);
    }
    T *operator->() { return &(*payload_); }

    operator bool() { return payload_.has_value(); }

  private:
    inline thread_local static std::optional<T> payload_{};
    bool *skip_;
    bool persist_;
};
}  // namespace vvl
