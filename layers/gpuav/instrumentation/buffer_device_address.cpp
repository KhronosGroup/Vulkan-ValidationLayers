/* Copyright (c) 2024-2025 LunarG, Inc.
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

#include "gpuav/instrumentation/buffer_device_address.h"

#include "gpuav/core/gpuav.h"
#include "gpuav/resources/gpuav_shader_resources.h"
#include "gpuav/resources/gpuav_state_trackers.h"
#include "gpuav/resources/gpuav_vulkan_objects.h"

namespace gpuav {

struct BufferDeviceAddressCbState {
    BufferDeviceAddressCbState(CommandBufferSubState& cb) {
        bda_ranges_snapshot_ptr = cb.gpu_resources_manager.GetDeviceLocalBufferRange(sizeof(VkDeviceAddress));
    }

    vko::BufferRange bda_ranges_snapshot_ptr{};
};

void RegisterBufferDeviceAddressValidation(Validator& gpuav, CommandBufferSubState& cb) {
    if (!gpuav.gpuav_settings.shader_instrumentation.buffer_device_address) {
        return;
    }

    cb.on_instrumentation_desc_set_update_functions.emplace_back(
        [](CommandBufferSubState& cb, VkPipelineBindPoint, VkDescriptorBufferInfo& out_buffer_info, uint32_t& out_dst_binding) {
            BufferDeviceAddressCbState& bda_cb_state = cb.shared_resources_cache.GetOrCreate<BufferDeviceAddressCbState>(cb);
            out_buffer_info.buffer = bda_cb_state.bda_ranges_snapshot_ptr.buffer;
            out_buffer_info.offset = bda_cb_state.bda_ranges_snapshot_ptr.offset;
            out_buffer_info.range = bda_cb_state.bda_ranges_snapshot_ptr.size;

            out_dst_binding = glsl::kBindingInstBufferDeviceAddress;
        });

    cb.on_pre_cb_submission_functions.emplace_back([](Validator& gpuav, CommandBufferSubState& cb,
                                                      VkCommandBuffer per_submission_cb) {
        BufferDeviceAddressCbState* bda_cb_state = cb.shared_resources_cache.TryGet<BufferDeviceAddressCbState>();
        // Can happen if command buffer did not record any action command
        if (!bda_cb_state) {
            return;
        }

        // Update buffer device address (BDA) table
        // One snapshot update per CB submission, to prevent concurrent submissions of the same CB to write and read
        // to the same snapshot.
        const size_t bda_ranges_count = gpuav.device_state->GetBufferAddressRangesCount();
        const VkDeviceSize bda_table_byte_size = 2 * sizeof(uint32_t) + 2 * sizeof(VkDeviceAddress) * bda_ranges_count;
        vko::BufferRange bda_table = cb.gpu_resources_manager.GetHostCachedBufferRange(bda_table_byte_size);

        auto bda_table_ranges_u32_ptr = (uint32_t*)bda_table.offset_mapped_ptr;
        *bda_table_ranges_u32_ptr = (uint32_t)bda_ranges_count;
        gpuav.device_state->GetBufferAddressRanges((vvl::DeviceState::BufferAddressRange*)(bda_table_ranges_u32_ptr + 2));
        cb.gpu_resources_manager.FlushAllocation(bda_table);

        // Fill a GPU buffer with a pointer to the BDA table
        vko::BufferRange bda_table_ptr = cb.gpu_resources_manager.GetHostCoherentBufferRange(sizeof(VkDeviceAddress));
        *(VkDeviceAddress*)bda_table_ptr.offset_mapped_ptr = bda_table.offset_address;

        // Dispatch a copy command, copying the per CB submission BDA table pointer to the BDA table pointer created at
        // "on_instrumentation_desc_set_update_functions" time, so that CB submission accesses correct BDA snapshot.
        {
            VkBufferMemoryBarrier barrier_write_after_read = vku::InitStructHelper();
            barrier_write_after_read.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            barrier_write_after_read.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier_write_after_read.buffer = bda_cb_state->bda_ranges_snapshot_ptr.buffer;
            barrier_write_after_read.offset = bda_cb_state->bda_ranges_snapshot_ptr.offset;
            barrier_write_after_read.size = bda_cb_state->bda_ranges_snapshot_ptr.size;

            DispatchCmdPipelineBarrier(per_submission_cb, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                       0, 0, nullptr, 1, &barrier_write_after_read, 0, nullptr);

            VkBufferCopy copy;
            copy.srcOffset = bda_table_ptr.offset;
            copy.dstOffset = bda_cb_state->bda_ranges_snapshot_ptr.offset;
            copy.size = sizeof(VkDeviceAddress);
            DispatchCmdCopyBuffer(per_submission_cb, bda_table_ptr.buffer, bda_cb_state->bda_ranges_snapshot_ptr.buffer, 1, &copy);

            VkBufferMemoryBarrier barrier_read_before_write = vku::InitStructHelper();
            barrier_read_before_write.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier_read_before_write.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            barrier_read_before_write.buffer = bda_cb_state->bda_ranges_snapshot_ptr.buffer;
            barrier_read_before_write.offset = bda_cb_state->bda_ranges_snapshot_ptr.offset;
            barrier_read_before_write.size = bda_cb_state->bda_ranges_snapshot_ptr.size;

            DispatchCmdPipelineBarrier(per_submission_cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
                                       nullptr, 1, &barrier_read_before_write, 0, nullptr);
        }
    });
}

}  // namespace gpuav
