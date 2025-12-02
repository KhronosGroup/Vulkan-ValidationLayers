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

#include "gpuav/core/gpuav.h"
#include "gpuav/resources/gpuav_shader_resources.h"
#include "gpuav/resources/gpuav_state_trackers.h"
#include "gpuav/resources/gpuav_vulkan_objects.h"
#include "gpuav/shaders/gpuav_error_codes.h"
#include "gpuav/shaders/gpuav_error_header.h"

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

    cb.on_instrumentation_error_logger_register_functions.emplace_back([](Validator& gpuav, CommandBufferSubState& cb,
                                                                          const LastBound& last_bound) {
        CommandBufferSubState::InstrumentationErrorLogger inst_error_logger = [](Validator& gpuav, const Location&,
                                                                                 const uint32_t* error_record,
                                                                                 std::string& out_error_msg,
                                                                                 std::string& out_vuid_msg) {
            using namespace glsl;
            bool error_found = false;
            const uint32_t error_group = error_record[glsl::kHeaderShaderIdErrorOffset] >> glsl::kErrorGroupShift;
            if (error_group != kErrorGroupInstBufferDeviceAddress) {
                return error_found;
            }
            error_found = true;

            std::ostringstream strm;

            const uint32_t payload = error_record[kInstLogErrorParameterOffset_2];
            const bool is_write = ((payload >> kInstBuffAddrAccessPayloadShiftIsWrite) & 1) != 0;
            const bool is_struct = ((payload >> kInstBuffAddrAccessPayloadShiftIsStruct) & 1) != 0;

            const uint64_t address = *reinterpret_cast<const uint64_t*>(error_record + kInstLogErrorParameterOffset_0);

            const uint32_t error_sub_code = (error_record[kHeaderShaderIdErrorOffset] & kErrorSubCodeMask) >> kErrorSubCodeShift;
            switch (error_sub_code) {
                case kErrorSubCodeBufferDeviceAddressUnallocRef: {
                    const char* access_type = is_write ? "written" : "read";
                    const uint32_t byte_size = payload & kInstBuffAddrAccessPayloadMaskAccessInfo;
                    strm << "Out of bounds access: " << byte_size << " bytes " << access_type << " at buffer device address 0x"
                         << std::hex << address << '.';
                    if (is_struct) {
                        // Added because glslang currently has no way to seperate out the struct (Slang does as of 2025.6.2)
                        strm << " This " << (is_write ? "write" : "read")
                             << " corresponds to a full OpTypeStruct load. While not all members of the struct might be accessed, "
                                "it is up "
                                "to the source language or tooling to detect that and reflect it in the SPIR-V.";
                    }
                    out_vuid_msg = "VUID-RuntimeSpirv-PhysicalStorageBuffer64-11819";

                } break;
                case kErrorSubCodeBufferDeviceAddressAlignment: {
                    const char* access_type = is_write ? "OpStore" : "OpLoad";
                    const uint32_t alignment = (payload & kInstBuffAddrAccessPayloadMaskAccessInfo);
                    strm << "Unaligned pointer access: The " << access_type << " at buffer device address 0x" << std::hex << address
                         << " is not aligned to the instruction Aligned operand of " << std::dec << alignment << '.';
                    out_vuid_msg = "VUID-RuntimeSpirv-PhysicalStorageBuffer64-06315";

                } break;
                default:
                    error_found = false;
                    break;
            }
            out_error_msg += strm.str();
            return error_found;
        };

        return inst_error_logger;
    });

    cb.on_instrumentation_desc_set_update_functions.emplace_back([](CommandBufferSubState& cb, VkPipelineBindPoint, const Location&,
                                                                    VkDescriptorBufferInfo& out_buffer_info,
                                                                    uint32_t& out_dst_binding) {
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
