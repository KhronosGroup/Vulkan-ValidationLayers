/* Copyright (c) 2025 The Khronos Group Inc.
 * Copyright (c) 2025 Valve Corporation
 * Copyright (c) 2025 LunarG, Inc.
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

#include <cstdint>
#include "gpuav/validation_cmd/gpuav_copy_memory_indirect.h"
#include "generated/dispatch_functions.h"
#include "generated/error_location_helper.h"
#include "gpuav/core/gpuav.h"
#include "gpuav/core/gpuav_validation_pipeline.h"
#include "gpuav/resources/gpuav_shader_resources.h"
#include "gpuav/validation_cmd/gpuav_validation_cmd_common.h"
#include "gpuav/resources/gpuav_state_trackers.h"
#include "gpuav/shaders/gpuav_error_header.h"
#include "gpuav/shaders/validation_cmd/push_data.h"
#include "generated/gpuav_offline_spirv.h"

namespace gpuav {
namespace valcmd {

static constexpr std::string_view kWarningCopyMemoryIndirectCommand =
    "WARNING - Memory in VkCopyMemoryIndirectCommandKHR was update with safe values to ensure vkCmdCopyMemoryIndirectKHR won't "
    "crash in the driver.";
static constexpr std::string_view kWarningCopyMemoryToImageIndirectCommand =
    "WARNING - Memory in VkCopyMemoryToImageIndirectCommandKHR was update with safe values to ensure "
    "vkCmdCopyMemoryToImageIndirectKHR won't crash in the driver.";

struct CopyMemoryIndirectValidationShader {
    static size_t GetSpirvSize() { return validation_cmd_copy_memory_indirect_comp_size * sizeof(uint32_t); }
    static const uint32_t *GetSpirv() { return validation_cmd_copy_memory_indirect_comp; }

    struct EmptyPushData {
    } push_constants;
    valpipe::BoundStorageBuffer api_input_buffer_binding = {glsl::kPreCopyMemoryIndirectBinding};

    static std::vector<VkDescriptorSetLayoutBinding> GetDescriptorSetLayoutBindings() {
        return {{glsl::kPreCopyMemoryIndirectBinding, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}};
    }

    std::vector<VkWriteDescriptorSet> GetDescriptorWrites(VkDescriptorSet desc_set) const {
        std::vector<VkWriteDescriptorSet> desc_writes(1);

        desc_writes[0] = vku::InitStructHelper();
        desc_writes[0].dstSet = desc_set;
        desc_writes[0].dstBinding = api_input_buffer_binding.binding;
        desc_writes[0].dstArrayElement = 0;
        desc_writes[0].descriptorCount = 1;
        desc_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        desc_writes[0].pBufferInfo = &api_input_buffer_binding.info;

        return desc_writes;
    }
};

void CopyMemoryIndirect(Validator &gpuav, const Location &loc, CommandBufferSubState &cb_state,
                        const CopyMemoryIndirectCommon &api_copy_info) {
    if (!gpuav.gpuav_settings.validate_copy_memory_indirect) {
        return;
    }

    if (api_copy_info.count == 0) {
        return;
    }

    ValidationCommandsCommon &val_cmd_common =
        cb_state.shared_resources_cache.GetOrCreate<ValidationCommandsCommon>(gpuav, cb_state, loc);
    valpipe::ComputePipeline<CopyMemoryIndirectValidationShader> &validation_pipeline =
        gpuav.shared_resources_manager.GetOrCreate<valpipe::ComputePipeline<CopyMemoryIndirectValidationShader>>(
            gpuav, loc, val_cmd_common.error_logging_desc_set_layout_);
    if (!validation_pipeline.valid) {
        gpuav.InternalError(cb_state.VkHandle(), loc, "Failed to create CopyMemoryIndirectValidationShader.");
        return;
    }

    valpipe::RestorablePipelineState restorable_state(cb_state, VK_PIPELINE_BIND_POINT_COMPUTE);

    // Setup shader resources
    // ---
    {
        CopyMemoryIndirectValidationShader shader_resources;

        // Needs to be kept in sync with copy_memory_indirect.comp
        struct CopyMemoryIndirectApiData {
            uint64_t range_address;
            uint32_t range_size;
            uint32_t range_stride;
            uint32_t copy_count;
            // Tried to make boolean, found GLSL alignment was off from C++
            // So keeping these as dwords
            uint32_t is_image_copy;
            uint32_t safe_mode;
        };

        const VkDeviceSize buffer_size = sizeof(CopyMemoryIndirectApiData);
        vko::BufferRange api_input_buffer_range = cb_state.gpu_resources_manager.GetHostCoherentBufferRange(buffer_size);

        auto gpu_region_ptr = (CopyMemoryIndirectApiData *)api_input_buffer_range.offset_mapped_ptr;
        gpu_region_ptr->range_address = api_copy_info.address_range.address;
        // While these are VkDeviceSize, there is no known way these can be over 4GB
        gpu_region_ptr->range_size = (uint32_t)api_copy_info.address_range.size;
        gpu_region_ptr->range_stride = (uint32_t)api_copy_info.address_range.stride;
        gpu_region_ptr->copy_count = api_copy_info.count;
        gpu_region_ptr->is_image_copy = loc.function == vvl::Func::vkCmdCopyMemoryToImageIndirectKHR;
        gpu_region_ptr->safe_mode = gpuav.gpuav_settings.safe_mode;

        shader_resources.api_input_buffer_binding.info = api_input_buffer_range.GetDescriptorBufferInfo();

        if (!BindShaderResources(validation_pipeline, gpuav, cb_state, cb_state.compute_index, cb_state.GetErrorLoggerIndex(),
                                 shader_resources)) {
            gpuav.InternalError(cb_state.VkHandle(), loc, "Failed to GetManagedDescriptorSet in BindShaderResources");
            return;
        }
    }

    // Setup validation pipeline
    // ---
    {
        DispatchCmdBindPipeline(cb_state.VkHandle(), VK_PIPELINE_BIND_POINT_COMPUTE, validation_pipeline.pipeline);

        // For simplicity, only validate up to maxComputeWorkGroupCount copies
        uint32_t work_group_count = std::min(api_copy_info.count, gpuav.phys_dev_props.limits.maxComputeWorkGroupCount[0]);
        // we set "local_size_x = 32" in GLSL shader
        work_group_count = work_group_count / 32 + uint32_t(work_group_count % 32 > 0);

        DispatchCmdDispatch(cb_state.VkHandle(), work_group_count, 1, 1);

        // If in safe mode, invalid copies specified in the indirect buffer will be set to "safe" values by the validation shader,
        // so accesses to it need to be synchronised.
        if (gpuav.gpuav_settings.safe_mode) {
            // TODO - Would be better to use a VkBufferMemoryBarrier, but that would involve having to figure out what the indirect
            // VkBuffer handle is
            VkMemoryBarrier2 memory_barrier = vku::InitStructHelper();
            memory_barrier.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
            memory_barrier.srcAccessMask = VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT;
            memory_barrier.dstStageMask = VK_PIPELINE_STAGE_2_COPY_INDIRECT_BIT_KHR;
            memory_barrier.dstAccessMask = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
            VkDependencyInfo dep_info = vku::InitStructHelper();
            dep_info.dependencyFlags = 0;
            dep_info.memoryBarrierCount = 1;
            dep_info.pMemoryBarriers = &memory_barrier;
            DispatchCmdPipelineBarrier2(cb_state.VkHandle(), &dep_info);
        }
    }

    CommandBufferSubState::ErrorLoggerFunc error_logger = [&gpuav, api_copy_info](const uint32_t *error_record,
                                                                                  const Location &loc_with_debug_region,
                                                                                  const LogObjectList &objlist) {
        bool skip = false;
        using namespace glsl;

        const uint32_t error_group = error_record[kHeaderShaderIdErrorOffset] >> kErrorGroupShift;
        if (error_group != kErrorGroupGpuCopyMemoryIndirect) {
            return skip;
        }

        const uint32_t error_sub_code = (error_record[kHeaderShaderIdErrorOffset] & kErrorSubCodeMask) >> kErrorSubCodeShift;
        const uint64_t payload_start = GetUint64(&error_record[kValCmdErrorPayloadDword_0]);
        const uint32_t copy_index = error_record[kValCmdErrorPayloadDword_2];
        const VkDeviceSize offset = api_copy_info.address_range.stride * copy_index;
        switch (error_sub_code) {
            case kErrorSubCodePreCopyMemoryIndirectSrcAddressInvalid: {
                const uint64_t src_address = payload_start;
                skip |= gpuav.LogError("VUID-VkCopyMemoryIndirectCommandKHR-srcAddress-parameter", objlist, loc_with_debug_region,
                                       "VkCopyMemoryIndirectInfoKHR::copyAddressRange.address (0x%" PRIx64 ") + (stride [%" PRIu64
                                       "] * copy [%" PRIu32 "]) is 0x%" PRIx64
                                       " which has a VkCopyMemoryIndirectCommandKHR::srcAddress of 0x%" PRIx64
                                       " which is not a valid VkDeviceAddress.\n%s",
                                       api_copy_info.address_range.address, api_copy_info.address_range.stride, copy_index,
                                       api_copy_info.address_range.address + offset, src_address,
                                       gpuav.gpuav_settings.safe_mode ? kWarningCopyMemoryIndirectCommand.data() : "");

                break;
            }
            case kErrorSubCodePreCopyMemoryIndirectSrcAddressAligned: {
                const uint64_t src_address = payload_start;
                skip |= gpuav.LogError("VUID-VkCopyMemoryIndirectCommandKHR-srcAddress-10958", objlist, loc_with_debug_region,
                                       "VkCopyMemoryIndirectInfoKHR::copyAddressRange.address (0x%" PRIx64 ") + (stride [%" PRIu64
                                       "] * copy [%" PRIu32 "]) is 0x%" PRIx64
                                       " which has a VkCopyMemoryIndirectCommandKHR::srcAddress of 0x%" PRIx64
                                       " which is not aligned to 4.\n%s",
                                       api_copy_info.address_range.address, api_copy_info.address_range.stride, copy_index,
                                       api_copy_info.address_range.address + offset, src_address,
                                       gpuav.gpuav_settings.safe_mode ? kWarningCopyMemoryIndirectCommand.data() : "");

                break;
            }
            case kErrorSubCodePreCopyMemoryIndirectDstAddressInvalid: {
                const uint64_t dst_address = payload_start;
                skip |= gpuav.LogError("VUID-VkCopyMemoryIndirectCommandKHR-dstAddress-parameter", objlist, loc_with_debug_region,
                                       "VkCopyMemoryIndirectInfoKHR::copyAddressRange.address (0x%" PRIx64 ") + (stride [%" PRIu64
                                       "] * copy [%" PRIu32 "]) is 0x%" PRIx64
                                       " which has a VkCopyMemoryIndirectCommandKHR::dstAddress of 0x%" PRIx64
                                       " which is not a valid VkDeviceAddress.\n%s",
                                       api_copy_info.address_range.address, api_copy_info.address_range.stride, copy_index,
                                       api_copy_info.address_range.address + offset, dst_address,
                                       gpuav.gpuav_settings.safe_mode ? kWarningCopyMemoryIndirectCommand.data() : "");

                break;
            }
            case kErrorSubCodePreCopyMemoryIndirectDstAddressAligned: {
                const uint64_t dst_address = payload_start;
                skip |= gpuav.LogError("VUID-VkCopyMemoryIndirectCommandKHR-dstAddress-10959", objlist, loc_with_debug_region,
                                       "VkCopyMemoryIndirectInfoKHR::copyAddressRange.address (0x%" PRIx64 ") + (stride [%" PRIu64
                                       "] * copy [%" PRIu32 "]) is 0x%" PRIx64
                                       " which has a VkCopyMemoryIndirectCommandKHR::dstAddress of 0x%" PRIx64
                                       " which is not aligned to 4.\n%s",
                                       api_copy_info.address_range.address, api_copy_info.address_range.stride, copy_index,
                                       api_copy_info.address_range.address + offset, dst_address,
                                       gpuav.gpuav_settings.safe_mode ? kWarningCopyMemoryIndirectCommand.data() : "");

                break;
            }
            case kErrorSubCodePreCopyMemoryIndirectSizeAligned: {
                const uint64_t command_size = payload_start;
                skip |= gpuav.LogError("VUID-VkCopyMemoryIndirectCommandKHR-size-10960", objlist, loc_with_debug_region,
                                       "VkCopyMemoryIndirectInfoKHR::copyAddressRange.address (0x%" PRIx64 ") + (stride [%" PRIu64
                                       "] * copy [%" PRIu32 "]) is 0x%" PRIx64
                                       " which has a VkCopyMemoryIndirectCommandKHR::size of %" PRIu64
                                       " which is not aligned to 4.\n%s",
                                       api_copy_info.address_range.address, api_copy_info.address_range.stride, copy_index,
                                       api_copy_info.address_range.address + offset, command_size,
                                       gpuav.gpuav_settings.safe_mode ? kWarningCopyMemoryIndirectCommand.data() : "");

                break;
            }
            case kErrorSubCodePreCopyMemoryToImageIndirectSrcAddressAligned: {
                const uint64_t src_address = payload_start;
                skip |=
                    gpuav.LogError("VUID-VkCopyMemoryToImageIndirectCommandKHR-srcAddress-10963", objlist, loc_with_debug_region,
                                   "VkCopyMemoryToImageIndirectInfoKHR::copyAddressRange.address (0x%" PRIx64
                                   ") + (stride [%" PRIu64 "] * copy [%" PRIu32 "]) is 0x%" PRIx64
                                   " which has a VkCopyMemoryToImageIndirectCommandKHR::srcAddress of 0x%" PRIx64
                                   " which is not aligned to 4.\n%s",
                                   api_copy_info.address_range.address, api_copy_info.address_range.stride, copy_index,
                                   api_copy_info.address_range.address + offset, src_address,
                                   gpuav.gpuav_settings.safe_mode ? kWarningCopyMemoryToImageIndirectCommand.data() : "");

                break;
            }
            case kErrorSubCodePreCopyMemoryToImageIndirectSrcAddressInvalid: {
                const uint64_t src_address = payload_start;
                skip |= gpuav.LogError("VUID-VkCopyMemoryToImageIndirectCommandKHR-srcAddress-parameter", objlist,
                                       loc_with_debug_region,
                                       "VkCopyMemoryToImageIndirectInfoKHR::copyAddressRange.address (0x%" PRIx64
                                       ") + (stride [%" PRIu64 "] * copy [%" PRIu32 "]) is 0x%" PRIx64
                                       " which has a VkCopyMemoryToImageIndirectCommandKHR::srcAddress of 0x%" PRIx64
                                       " which is not a valid VkDeviceAddress.\n%s",
                                       api_copy_info.address_range.address, api_copy_info.address_range.stride, copy_index,
                                       api_copy_info.address_range.address + offset, src_address,
                                       gpuav.gpuav_settings.safe_mode ? kWarningCopyMemoryToImageIndirectCommand.data() : "");

                break;
            }
            case kErrorSubCodePreCopyMemoryToImageIndirectBufferRowLength: {
                const uint32_t buffer_row_length = error_record[kValCmdErrorPayloadDword_0];
                const uint32_t image_extent_width = error_record[kValCmdErrorPayloadDword_1];
                skip |= gpuav.LogError("VUID-VkCopyMemoryToImageIndirectCommandKHR-bufferRowLength-10964", objlist,
                                       loc_with_debug_region,
                                       "VkCopyMemoryToImageIndirectInfoKHR::copyAddressRange.address (0x%" PRIx64
                                       ") + (stride [%" PRIu64 "] * copy [%" PRIu32 "]) is 0x%" PRIx64
                                       " which has a VkCopyMemoryToImageIndirectCommandKHR::bufferRowLength of %" PRIu32
                                       " is less than the imageExtent.width (%" PRIu32 ")\n%s",
                                       api_copy_info.address_range.address, api_copy_info.address_range.stride, copy_index,
                                       api_copy_info.address_range.address + offset, buffer_row_length, image_extent_width,
                                       gpuav.gpuav_settings.safe_mode ? kWarningCopyMemoryToImageIndirectCommand.data() : "");

                break;
            }
            case kErrorSubCodePreCopyMemoryToImageIndirectBufferImageHeight: {
                const uint32_t buffer_image_height = error_record[kValCmdErrorPayloadDword_0];
                const uint32_t image_extent_height = error_record[kValCmdErrorPayloadDword_1];
                skip |= gpuav.LogError("VUID-VkCopyMemoryToImageIndirectCommandKHR-bufferImageHeight-10965", objlist,
                                       loc_with_debug_region,
                                       "VkCopyMemoryToImageIndirectInfoKHR::copyAddressRange.address (0x%" PRIx64
                                       ") + (stride [%" PRIu64 "] * copy [%" PRIu32 "]) is 0x%" PRIx64
                                       " which has a VkCopyMemoryToImageIndirectCommandKHR::bufferImageHeight of %" PRIu32
                                       " is less than the imageExtent.height (%" PRIu32 ")\n%s",
                                       api_copy_info.address_range.address, api_copy_info.address_range.stride, copy_index,
                                       api_copy_info.address_range.address + offset, buffer_image_height, image_extent_height,
                                       gpuav.gpuav_settings.safe_mode ? kWarningCopyMemoryToImageIndirectCommand.data() : "");

                break;
            }
            default:
                break;
        }

        return skip;
    };

    cb_state.AddCommandErrorLogger(loc, nullptr, std::move(error_logger));
}

}  // namespace valcmd
}  // namespace gpuav
