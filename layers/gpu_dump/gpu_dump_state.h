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
#include <vulkan/vulkan_core.h>
#include <cstdint>
#include "chassis/layer_object_id.h"
#include "state_tracker/state_tracker.h"
#include "state_tracker/cmd_buffer_state.h"

namespace gpudump {
class GpuDump;
struct MappingInfo;
struct DumpInfo;
struct WarnInfo;

class CommandBufferSubState : public vvl::CommandBufferSubState {
  public:
    explicit CommandBufferSubState(vvl::CommandBuffer& cb, GpuDump& dev_data);
    GpuDump& dev_data;

    void RecordActionCommand(LastBound& last_bound, const Location& loc) final;

    void RecordPushData(const VkPushDataInfoEXT& push_data_info) final;
    void ClearPushData() final;

    void RecordExecuteGeneratedCommands(const VkGeneratedCommandsInfoEXT& info, VkPipelineBindPoint bind_point,
                                        const Location& loc) final;

    void RecordCopyMemoryIndirect(const VkCopyMemoryIndirectInfoKHR& info, const Location& loc) final;
    void RecordCopyMemoryToImageIndirect(const VkCopyMemoryToImageIndirectInfoKHR& info, const Location& loc) final;

    std::vector<uint8_t> push_data_value;

  private:
    void DumpDescriptors(const LastBound& last_bound, const Location& loc) const;
    bool DumpDescriptorBuffer(std::ostringstream& ss, const LastBound& last_bound) const;

    // Return true if warning found
    bool DumpDescriptorHeap(std::ostringstream& ss, const LastBound& last_bound) const;
    VkDeviceSize GetPushData(std::ostringstream& ss, WarnInfo& warn, uint32_t offset, uint32_t size) const;
    void DumpDescriptorHeapConstantOffset(std::ostringstream& ss, DumpInfo& dump, WarnInfo& warn,
                                          const VkDescriptorMappingSourceConstantOffsetEXT& map_data) const;
    void DumpDescriptorHeapPushIndex(std::ostringstream& ss, DumpInfo& dump, WarnInfo& warn,
                                     const VkDescriptorMappingSourcePushIndexEXT& map_data) const;
    void DumpDescriptorHeapIndirectIndex(std::ostringstream& ss, DumpInfo& dump, WarnInfo& warn,
                                         const VkDescriptorMappingSourceIndirectIndexEXT& map_data) const;
    void DumpDescriptorHeapIndirectIndexArray(std::ostringstream& ss, DumpInfo& dump, WarnInfo& warn,
                                              const VkDescriptorMappingSourceIndirectIndexArrayEXT& map_data) const;
    void DumpDescriptorHeapHeapData(std::ostringstream& ss, DumpInfo& dump, WarnInfo& warn,
                                    const VkDescriptorMappingSourceHeapDataEXT& map_data) const;
    void DumpDescriptorHeapPushAddress(std::ostringstream& ss, WarnInfo& warn, uint32_t pushAddressOffset) const;
    void DumpDescriptorHeapIndirectAddress(std::ostringstream& ss, DumpInfo& dump, WarnInfo& warn,
                                           const VkDescriptorMappingSourceIndirectAddressEXT& map_data) const;
    void DumpDescriptorHeapShaderRecordIndex(std::ostringstream& ss, DumpInfo& dump, WarnInfo& warn,
                                             const VkDescriptorMappingSourceShaderRecordIndexEXT& map_data) const;
    void DumpDescriptorHeapShaderRecordAddress(std::ostringstream& ss, uint32_t shaderRecordAddressOffset) const;
    bool DumpDescriptorHeapMapping(std::ostringstream& ss, const MappingInfo& mapping_info) const;

    bool DumpDescriptorHeapUntyped(std::ostringstream& ss, const ShaderStageState& stage) const;

    bool DumpCopyMemoryIndirectCommon(std::ostringstream& ss, uint32_t copy_count,
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
