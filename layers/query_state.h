/* Copyright (c) 2015-2021 The Khronos Group Inc.
 * Copyright (c) 2015-2021 Valve Corporation
 * Copyright (c) 2015-2021 LunarG, Inc.
 * Copyright (C) 2015-2021 Google Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
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
 * Author: Courtney Goeltzenleuchter <courtneygo@google.com>
 * Author: Tobin Ehlis <tobine@google.com>
 * Author: Chris Forbes <chrisf@ijw.co.nz>
 * Author: Mark Lobodzinski <mark@lunarg.com>
 * Author: Dave Houlton <daveh@lunarg.com>
 * Author: John Zulauf <jzulauf@lunarg.com>
 * Author: Tobias Hector <tobias.hector@amd.com>
 */
#pragma once
#include "base_node.h"
#include "hash_vk_types.h"

class QUERY_POOL_STATE : public BASE_NODE {
  public:
    VkQueryPoolCreateInfo createInfo;

    bool has_perf_scope_command_buffer;
    bool has_perf_scope_render_pass;
    uint32_t n_performance_passes;
    uint32_t perf_counter_index_count;
    VkQueryControlFlags control_flags;

    QUERY_POOL_STATE(VkQueryPool qp, const VkQueryPoolCreateInfo *pCreateInfo, uint32_t index_count, uint32_t n_perf_pass,
                     bool has_cb, bool has_rb)
        : BASE_NODE(qp, kVulkanObjectTypeQueryPool),
          createInfo(*pCreateInfo),
          has_perf_scope_command_buffer(has_cb),
          has_perf_scope_render_pass(has_rb),
          n_performance_passes(n_perf_pass),
          perf_counter_index_count(index_count) {}

    VkQueryPool pool() const { return handle_.Cast<VkQueryPool>(); }
};

struct QueryObject {
    VkQueryPool pool;
    uint32_t query;
    // These next two fields are *not* used in hash or comparison, they are effectively a data payload
    uint32_t index;  // must be zero if !indexed
    uint32_t perf_pass;
    bool indexed;
    // Command index in the command buffer where the end of the query was
    // recorded (equal to the number of commands in the command buffer before
    // the end of the query).
    uint64_t endCommandIndex;

    QueryObject(VkQueryPool pool_, uint32_t query_)
        : pool(pool_), query(query_), index(0), perf_pass(0), indexed(false), endCommandIndex(0) {}
    QueryObject(VkQueryPool pool_, uint32_t query_, uint32_t index_)
        : pool(pool_), query(query_), index(index_), perf_pass(0), indexed(true), endCommandIndex(0) {}
    QueryObject(const QueryObject &obj)
        : pool(obj.pool),
          query(obj.query),
          index(obj.index),
          perf_pass(obj.perf_pass),
          indexed(obj.indexed),
          endCommandIndex(obj.endCommandIndex) {}
    QueryObject(const QueryObject &obj, uint32_t perf_pass_)
        : pool(obj.pool),
          query(obj.query),
          index(obj.index),
          perf_pass(perf_pass_),
          indexed(obj.indexed),
          endCommandIndex(obj.endCommandIndex) {}
    bool operator<(const QueryObject &rhs) const {
        return (pool == rhs.pool) ? ((query == rhs.query) ? (perf_pass < rhs.perf_pass) : (query < rhs.query)) : pool < rhs.pool;
    }
};

inline bool operator==(const QueryObject &query1, const QueryObject &query2) {
    return ((query1.pool == query2.pool) && (query1.query == query2.query) && (query1.perf_pass == query2.perf_pass));
}

enum QueryState {
    QUERYSTATE_UNKNOWN,    // Initial state.
    QUERYSTATE_RESET,      // After resetting.
    QUERYSTATE_RUNNING,    // Query running.
    QUERYSTATE_ENDED,      // Query ended but results may not be available.
    QUERYSTATE_AVAILABLE,  // Results available.
};

typedef std::map<QueryObject, QueryState> QueryMap;

enum QueryResultType {
    QUERYRESULT_UNKNOWN,
    QUERYRESULT_NO_DATA,
    QUERYRESULT_SOME_DATA,
    QUERYRESULT_WAIT_ON_RESET,
    QUERYRESULT_WAIT_ON_RUNNING,
};

inline const char *string_QueryResultType(QueryResultType result_type) {
    switch (result_type) {
        case QUERYRESULT_UNKNOWN:
            return "query may be in an unknown state";
        case QUERYRESULT_NO_DATA:
            return "query may return no data";
        case QUERYRESULT_SOME_DATA:
            return "query will return some data or availability bit";
        case QUERYRESULT_WAIT_ON_RESET:
            return "waiting on a query that has been reset and not issued yet";
        case QUERYRESULT_WAIT_ON_RUNNING:
            return "waiting on a query that has not ended yet";
    }
    assert(false);
    return "UNKNOWN QUERY STATE";  // Unreachable.
}

namespace std {
template <>
struct hash<QueryObject> {
    size_t operator()(QueryObject query) const throw() {
        return hash<uint64_t>()((uint64_t)(query.pool)) ^
               hash<uint64_t>()(static_cast<uint64_t>(query.query) | (static_cast<uint64_t>(query.perf_pass) << 32));
    }
};

}  // namespace std
