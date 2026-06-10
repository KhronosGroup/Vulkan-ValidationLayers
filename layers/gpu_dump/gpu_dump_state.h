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
#include "containers/custom_containers.h"
#include "state_tracker/state_tracker.h"
#include "state_tracker/cmd_buffer_state.h"

namespace gpudump {
class GpuDump;
struct MappingInfo;

struct HeapAccess {
    VkDescriptorType descriptor_type = VK_DESCRIPTOR_TYPE_MAX_ENUM;
    // These are zero because if not found, it means it is implicitly zero
    // This also allows for better hashes being the same
    uint32_t heap_offset = 0;
    uint32_t array_stride = 0;
    uint32_t descriptor_index = vvl::kNoIndex32;

    struct compare {
        bool operator()(HeapAccess const& lhs, HeapAccess const& rhs) const {
            return lhs.descriptor_type == rhs.descriptor_type && lhs.heap_offset == rhs.heap_offset &&
                   lhs.array_stride == rhs.array_stride && lhs.descriptor_index == rhs.descriptor_index;
        }
    };

    // We don't want duplicate accesses being spammed
    struct hash {
        std::size_t operator()(HeapAccess const& access) const {
            hash_util::HashCombiner hc;
            hc << access.descriptor_type << access.heap_offset << access.array_stride << access.descriptor_index;
            return hc.Value();
        }
    };
};
using HeapAccesses = vvl::unordered_set<HeapAccess, HeapAccess::hash, HeapAccess::compare>;

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

  private:
    void DumpDescriptors(const LastBound& last_bound, const Location& loc) const;
    bool DumpDescriptorBuffer(std::ostringstream& ss, const LastBound& last_bound) const;
    // Return true if warning found
    bool DumpDescriptorHeap(std::ostringstream& ss, const LastBound& last_bound) const;
    bool DumpDescriptorHeapMapping(std::ostringstream& ss, const MappingInfo& mapping_info) const;
    vvl::unordered_map<uint32_t, HeapAccesses> DumpDescriptorHeapUntypedFindAccess(const spirv::Module& module,
                                                                                   const spirv::EntryPoint& entrypoint) const;
    bool DumpDescriptorHeapUntyped(std::ostringstream& ss, const ShaderStageState& stage) const;

    bool DumpCopyMemoryIndirectCommon(std::ostringstream& ss, uint32_t copy_count,
                                      VkStridedDeviceAddressRangeKHR copy_address_range) const;
    void DumpCopyMemoryIndirect(const VkCopyMemoryIndirectInfoKHR& info, const Location& loc) const;
    void DumpCopyMemoryToImageIndirect(const VkCopyMemoryToImageIndirectInfoKHR& info, const Location& loc) const;

    void DumpDeviceGeneratedCommands(const VkGeneratedCommandsInfoEXT& info, VkPipelineBindPoint bind_point,
                                     const Location& loc) const;

    std::vector<uint8_t> push_data_value;
};

static inline CommandBufferSubState& SubState(vvl::CommandBuffer& cb) {
    return *static_cast<CommandBufferSubState*>(cb.SubState(LayerObjectTypeGpuDump));
}

static inline const CommandBufferSubState& SubState(const vvl::CommandBuffer& cb) {
    return *static_cast<const CommandBufferSubState*>(cb.SubState(LayerObjectTypeGpuDump));
}

}  // namespace gpudump
