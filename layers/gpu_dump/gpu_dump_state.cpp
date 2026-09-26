/* Copyright (c) 2026 Valve Corporation
 * Copyright (c) 2026 LunarG, Inc.
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
#include "gpu_dump/gpu_dump_state.h"
#include "gpu_dump/gpu_dump.h"

namespace gpudump {

void GpuDump::Created(vvl::CommandBuffer& cb_state) {
    cb_state.SetSubState(container_type, std::make_unique<gpudump::CommandBufferSubState>(cb_state, *this));
}

CommandBufferSubState::CommandBufferSubState(vvl::CommandBuffer& cb, GpuDump& dev_data)
    : vvl::CommandBufferSubState(cb), dev_data(dev_data) {}

void CommandBufferSubState::RecordActionCommand(LastBound& last_bound, const Location& loc) {
    if (dev_data.gpu_dump_settings.descriptors) {
        DumpDescriptors(last_bound, loc);
    }
}

void CommandBufferSubState::RecordPushData(const VkPushDataInfoEXT& push_data_info) {
    if (push_data_value.empty()) {
        push_data_value.resize((size_t)base.dev_data.phys_dev_ext_props.descriptor_heap_props.maxPushDataSize);
    }

    memcpy(push_data_value.data() + push_data_info.offset, push_data_info.data.address, push_data_info.data.size);
}

void CommandBufferSubState::ClearPushData() { push_data_value.clear(); }

void CommandBufferSubState::RecordExecuteGeneratedCommands(const VkGeneratedCommandsInfoEXT& info, VkPipelineBindPoint bind_point,
                                                           const Location& loc) {
    if (dev_data.gpu_dump_settings.device_generated_commands) {
        DumpDeviceGeneratedCommands(info, bind_point, loc);
    }
}

void CommandBufferSubState::RecordCopyMemoryIndirect(const VkCopyMemoryIndirectInfoKHR& info, const Location& loc) {
    if (dev_data.gpu_dump_settings.copy_memory_indirect) {
        DumpCopyMemoryIndirect(info, loc);
    }
}

void CommandBufferSubState::RecordCopyMemoryToImageIndirect(const VkCopyMemoryToImageIndirectInfoKHR& info, const Location& loc) {
    if (dev_data.gpu_dump_settings.copy_memory_indirect) {
        DumpCopyMemoryToImageIndirect(info, loc);
    }
}

}  // namespace gpudump
