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
#pragma once
#include "chassis/layer_object_id.h"
#include "state_tracker/state_tracker.h"
#include "state_tracker/cmd_buffer_state.h"

namespace gpudump {
class GpuDump;

class CommandBufferSubState : public vvl::CommandBufferSubState {
  public:
    explicit CommandBufferSubState(vvl::CommandBuffer& cb, GpuDump& dev_data);
    GpuDump& dev_data;

    void RecordActionCommand(LastBound& last_bound, const Location& loc) final;

    void RecordExecuteGeneratedCommands(const VkGeneratedCommandsInfoEXT& info, VkPipelineBindPoint bind_point,
                                        const Location& loc) final;

    void RecordCopyMemoryIndirect(const VkCopyMemoryIndirectInfoKHR& info, const Location& loc) final;
    void RecordCopyMemoryToImageIndirect(const VkCopyMemoryToImageIndirectInfoKHR& info, const Location& loc) final;

  private:
    void DumpDescriptors(const LastBound& last_bound, const Location& loc) const;
    void DumpDescriptorBuffer(std::ostringstream& ss, const LastBound& last_bound) const;
    void DumpDescriptorHeap(std::ostringstream& ss, const LastBound& last_bound) const;

    void DumpCopyMemoryIndirectCommon(std::ostringstream& ss, uint32_t copy_count,
                                      VkStridedDeviceAddressRangeKHR copy_address_range) const;
    void DumpCopyMemoryIndirect(const VkCopyMemoryIndirectInfoKHR& info, const Location& loc) const;
    void DumpCopyMemoryToImageIndirect(const VkCopyMemoryToImageIndirectInfoKHR& info, const Location& loc) const;

    void DumpDeviceGeneratedCommands(const VkGeneratedCommandsInfoEXT& info, VkPipelineBindPoint bind_point,
                                     const Location& loc) const;
};

static inline CommandBufferSubState& SubState(vvl::CommandBuffer& cb) {
    return *static_cast<CommandBufferSubState*>(cb.SubState(LayerObjectTypeGpuDump));
}

static inline const CommandBufferSubState& SubState(const vvl::CommandBuffer& cb) {
    return *static_cast<const CommandBufferSubState*>(cb.SubState(LayerObjectTypeGpuDump));
}

}  // namespace gpudump
