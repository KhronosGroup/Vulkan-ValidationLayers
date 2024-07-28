/* Copyright (c) 2024 The Khronos Group Inc.
 * Copyright (c) 2024 Valve Corporation
 * Copyright (c) 2024 LunarG, Inc.
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

#include "sync_stats.h"

#if VVL_ENABLE_SYNCVAL_STATS != 0
#include "sync_commandbuffer.h"
#include "utils/vk_layer_utils.h"

#include <iostream>

namespace syncval_stats {

void Value32::Update(uint32_t new_value) { u32.store(new_value); }
uint32_t Value32::Add(uint32_t n) { return u32.fetch_add(n); }
uint32_t Value32::Sub(uint32_t n) { return u32.fetch_sub(n); }

void ValueMax32::Update(uint32_t new_value) {
    value.Update(new_value);
    vvl::atomic_fetch_max(max_value.u32, new_value);
}

void ValueMax32::Add(uint32_t n) {
    uint32_t new_value = value.Add(n);
    vvl::atomic_fetch_max(max_value.u32, new_value);
}

void ValueMax32::Sub(uint32_t n) {
    uint32_t new_value = value.Sub(n);
    vvl::atomic_fetch_max(max_value.u32, new_value);
}

Stats::~Stats() {
    if (report_on_destruction) {
        const std::string report = CreateReport();
        std::cout << report;
    }
}

void Stats::AddCommandBufferContext() { command_buffer_context_counter.Add(1); }
void Stats::RemoveCommandBufferContext() { command_buffer_context_counter.Sub(1); }

void Stats::AddHandleRecord(uint32_t count) { handle_record_counter.Add(count); }
void Stats::RemoveHandleRecord(uint32_t count) { handle_record_counter.Sub(count); }

void Stats::ReportOnDestruction() { report_on_destruction = true; }

std::string Stats::CreateReport() {
    std::ostringstream str;
    {
        uint32_t cb_contex = command_buffer_context_counter.value.u32;
        uint32_t cb_context_max = command_buffer_context_counter.max_value.u32;

        str << "CommandBufferAccessContext:\n";
        str << "\tcount = " << cb_contex << '\n';
        str << "\tmax_count = " << cb_context_max << '\n';
    }
    {
        uint32_t handle_record = handle_record_counter.value.u32;
        uint64_t handle_record_memory = handle_record * sizeof(HandleRecord);
        uint32_t handle_record_max = handle_record_counter.max_value.u32;
        uint64_t handle_record_max_memory = handle_record_max * sizeof(HandleRecord);

        str << "HandleRecord:\n";
        str << "\tcount = " << handle_record << '\n';
        str << "\tmemory = " << handle_record_memory << " bytes\n";
        str << "\tmax_count = " << handle_record_max << '\n';
        str << "\tmax_memory = " << handle_record_max_memory << " bytes\n";
    }
    return str.str();
}

}  // namespace syncval_stats
#endif  // VVL_ENABLE_SYNCVAL_STATS != 0
