/* Copyright (c) 2018-2025 The Khronos Group Inc.
 * Copyright (c) 2018-2025 Valve Corporation
 * Copyright (c) 2018-2025 LunarG, Inc.
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

#include <vulkan/vulkan_core.h>
#include "gpuav/core/gpuav.h"
#include "gpuav/core/gpuav_validation_pipeline.h"
#include "gpuav/validation_cmd/gpuav_validation_cmd_common.h"
#include "gpuav/resources/gpuav_state_trackers.h"
#include "gpuav/shaders/gpuav_error_header.h"
#include "gpuav/shaders/gpuav_shaders_constants.h"
#include "gpuav/shaders/validation_cmd/push_data.h"
#include "generated/gpuav_offline_spirv.h"
#include "containers/limits.h"

namespace gpuav {
namespace valcmd {

struct CopyBufferToImageValidationShader {
    static size_t GetSpirvSize() { return validation_cmd_copy_buffer_to_image_comp_size * sizeof(uint32_t); }
    static const uint32_t *GetSpirv() { return validation_cmd_copy_buffer_to_image_comp; }

    struct EmptyPushData {
    } push_constants;
    valpipe::BoundStorageBuffer src_buffer_binding = {glsl::kPreCopyBufferToImageBinding_SrcBuffer};
    valpipe::BoundStorageBuffer copy_src_regions_buffer_binding = {glsl::kPreCopyBufferToImageBinding_CopySrcRegions};

    static std::vector<VkDescriptorSetLayoutBinding> GetDescriptorSetLayoutBindings() {
        std::vector<VkDescriptorSetLayoutBinding> bindings = {
            {glsl::kPreCopyBufferToImageBinding_SrcBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT,
             nullptr},
            {glsl::kPreCopyBufferToImageBinding_CopySrcRegions, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT,
             nullptr}};

        return bindings;
    }

    std::vector<VkWriteDescriptorSet> GetDescriptorWrites(VkDescriptorSet desc_set) const {
        std::vector<VkWriteDescriptorSet> desc_writes(2);

        desc_writes[0] = vku::InitStructHelper();
        desc_writes[0].dstSet = desc_set;
        desc_writes[0].dstBinding = src_buffer_binding.binding;
        desc_writes[0].dstArrayElement = 0;
        desc_writes[0].descriptorCount = 1;
        desc_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        desc_writes[0].pBufferInfo = &src_buffer_binding.info;

        desc_writes[1] = vku::InitStructHelper();
        desc_writes[1].dstSet = desc_set;
        desc_writes[1].dstBinding = copy_src_regions_buffer_binding.binding;
        desc_writes[1].dstArrayElement = 0;
        desc_writes[1].descriptorCount = 1;
        desc_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        desc_writes[1].pBufferInfo = &copy_src_regions_buffer_binding.info;

        return desc_writes;
    }
};

void CopyBufferToImage(Validator &gpuav, const Location &loc, CommandBufferSubState &cb_state,
                       const VkCopyBufferToImageInfo2 *copy_buffer_to_img_info) {
    if (!gpuav.gpuav_settings.validate_buffer_copies) {
        return;
    }

    // No need to perform validation if VK_EXT_depth_range_unrestricted is enabled
    if (IsExtEnabled(gpuav.extensions.vk_ext_depth_range_unrestricted)) {
        return;
    }

    if (cb_state.max_actions_cmd_validation_reached_) {
        return;
    }

    auto image_state = gpuav.Get<vvl::Image>(copy_buffer_to_img_info->dstImage);
    if (!image_state) {
        gpuav.InternalError(cb_state.VkHandle(), loc, "AllocatePreCopyBufferToImageValidationResources: Unrecognized image.");
        return;
    }

    // Only need to perform validation for depth image having a depth format that is not unsigned normalized.
    // For unsigned normalized formats, depth is by definition in range [0, 1]
    if (!IsValueIn(image_state->create_info.format, {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT})) {
        return;
    }

    valpipe::ComputePipeline<CopyBufferToImageValidationShader> &validation_pipeline =
        gpuav.shared_resources_manager.GetOrCreate<valpipe::ComputePipeline<CopyBufferToImageValidationShader>>(
            gpuav, loc, cb_state.GetErrorLoggingDescSetLayout());
    if (!validation_pipeline.valid) {
        return;
    }

    valpipe::RestorablePipelineState restorable_state(cb_state, VK_PIPELINE_BIND_POINT_COMPUTE);

    uint32_t group_count_x = 0;
    // Setup shader resources
    // ---
    {
        CopyBufferToImageValidationShader shader_resources;

        // Allocate buffer that will be used to store pRegions
        uint32_t max_texels_count_in_regions = copy_buffer_to_img_info->pRegions[0].imageExtent.width *
                                               copy_buffer_to_img_info->pRegions[0].imageExtent.height *
                                               copy_buffer_to_img_info->pRegions[0].imageExtent.depth;

        // Needs to be kept in sync with copy_buffer_to_image.comp
        struct BufferImageCopy {
            uint32_t src_buffer_byte_offset;
            uint32_t start_layer;
            uint32_t layer_count;
            uint32_t row_extent;
            uint32_t slice_extent;
            uint32_t layer_extent;
            uint32_t pad_[2];
            int32_t image_offset[4];
            uint32_t image_extent[4];
        };

        const VkDeviceSize uniform_buffer_constants_byte_size = (4 +  // image extent
                                                                 1 +  // block size
                                                                 1 +  // gpu copy regions count
                                                                 2    // pad
                                                                 ) *
                                                                sizeof(uint32_t);

        const VkDeviceSize buffer_size =
            uniform_buffer_constants_byte_size + sizeof(BufferImageCopy) * copy_buffer_to_img_info->regionCount;
        vko::BufferRange copy_src_regions_mem_buffer_range = cb_state.gpu_resources_manager.GetHostVisibleBufferRange(buffer_size);
        if (copy_src_regions_mem_buffer_range.buffer == VK_NULL_HANDLE) {
            return;
        }

        auto gpu_regions_u32_ptr = (uint32_t *)copy_src_regions_mem_buffer_range.offset_mapped_ptr;

        const uint32_t block_size = image_state->create_info.format == VK_FORMAT_D32_SFLOAT ? 4 : 5;
        uint32_t gpu_regions_count = 0;
        BufferImageCopy *gpu_regions_ptr =
            reinterpret_cast<BufferImageCopy *>(&gpu_regions_u32_ptr[uniform_buffer_constants_byte_size / sizeof(uint32_t)]);
        for (const auto &cpu_region : vvl::make_span(copy_buffer_to_img_info->pRegions, copy_buffer_to_img_info->regionCount)) {
            if (cpu_region.imageSubresource.aspectMask != VK_IMAGE_ASPECT_DEPTH_BIT) {
                continue;
            }

            // Read offset above kU32Max cannot be indexed in the validation shader
            if (const VkDeviceSize max_buffer_read_offset =
                    cpu_region.bufferOffset + static_cast<VkDeviceSize>(block_size) * cpu_region.imageExtent.width *
                                                  cpu_region.imageExtent.height * cpu_region.imageExtent.depth;
                max_buffer_read_offset > static_cast<VkDeviceSize>(vvl::kU32Max)) {
                continue;
            }

            BufferImageCopy &gpu_region = gpu_regions_ptr[gpu_regions_count];
            gpu_region.src_buffer_byte_offset = static_cast<uint32_t>(cpu_region.bufferOffset);
            gpu_region.start_layer = cpu_region.imageSubresource.baseArrayLayer;
            gpu_region.layer_count = cpu_region.imageSubresource.layerCount;
            gpu_region.row_extent = std::max(cpu_region.bufferRowLength, image_state->create_info.extent.width * block_size);
            gpu_region.slice_extent =
                std::max(cpu_region.bufferImageHeight, image_state->create_info.extent.height * gpu_region.row_extent);
            gpu_region.layer_extent = image_state->create_info.extent.depth * gpu_region.slice_extent;
            gpu_region.image_offset[0] = cpu_region.imageOffset.x;
            gpu_region.image_offset[1] = cpu_region.imageOffset.y;
            gpu_region.image_offset[2] = cpu_region.imageOffset.z;
            gpu_region.image_offset[3] = 0;
            gpu_region.image_extent[0] = cpu_region.imageExtent.width;
            gpu_region.image_extent[1] = cpu_region.imageExtent.height;
            gpu_region.image_extent[2] = cpu_region.imageExtent.depth;
            gpu_region.image_extent[3] = 0;

            max_texels_count_in_regions =
                std::max(max_texels_count_in_regions,
                         cpu_region.imageExtent.width * cpu_region.imageExtent.height * cpu_region.imageExtent.depth);

            ++gpu_regions_count;

            if (gpu_regions_count == 0) {
                // Nothing to validate
                return;
            }
        }

        gpu_regions_u32_ptr[0] = image_state->create_info.extent.width;
        gpu_regions_u32_ptr[1] = image_state->create_info.extent.height;
        gpu_regions_u32_ptr[2] = image_state->create_info.extent.depth;
        gpu_regions_u32_ptr[3] = 0;
        gpu_regions_u32_ptr[4] = block_size;
        gpu_regions_u32_ptr[5] = gpu_regions_count;
        gpu_regions_u32_ptr[6] = 0;
        gpu_regions_u32_ptr[7] = 0;

        shader_resources.src_buffer_binding.info = {copy_buffer_to_img_info->srcBuffer, 0, VK_WHOLE_SIZE};
        shader_resources.copy_src_regions_buffer_binding.info = {copy_src_regions_mem_buffer_range.buffer,
                                                                 copy_src_regions_mem_buffer_range.offset,
                                                                 copy_src_regions_mem_buffer_range.size};

        if (!BindShaderResources(validation_pipeline, gpuav, cb_state, cb_state.compute_index,
                                 uint32_t(cb_state.per_command_error_loggers.size()), shader_resources)) {
            return;
        }

        group_count_x = max_texels_count_in_regions / 64 + uint32_t(max_texels_count_in_regions % 64 > 0);
    }

    // Setup validation pipeline
    // ---
    {
        DispatchCmdBindPipeline(cb_state.VkHandle(), VK_PIPELINE_BIND_POINT_COMPUTE, validation_pipeline.pipeline);

        DispatchCmdDispatch(cb_state.VkHandle(), group_count_x, 1, 1);
    }

    CommandBufferSubState::ErrorLoggerFunc error_logger = [&gpuav, loc, src_buffer = copy_buffer_to_img_info->srcBuffer](
                                                              const uint32_t *error_record, const LogObjectList &objlist,
                                                              const std::vector<std::string> &) {
        bool skip = false;
        using namespace glsl;

        const uint32_t error_group = error_record[kHeaderShaderIdErrorOffset] >> kErrorGroupShift;
        if (error_group != kErrorGroupGpuCopyBufferToImage) {
            return skip;
        }

        const uint32_t error_sub_code = (error_record[kHeaderShaderIdErrorOffset] & kErrorSubCodeMask) >> kErrorSubCodeShift;
        switch (error_sub_code) {
            case kErrorSubCodePreCopyBufferToImageBufferTexel: {
                const uint32_t texel_offset = error_record[kPreActionParamOffset_0];
                LogObjectList objlist_and_src_buffer = objlist;
                objlist_and_src_buffer.add(src_buffer);
                const char *vuid = loc.function == vvl::Func::vkCmdCopyBufferToImage
                                       ? "VUID-vkCmdCopyBufferToImage-pRegions-07931"
                                       : "VUID-VkCopyBufferToImageInfo2-pRegions-07931";
                skip |= gpuav.LogError(vuid, objlist_and_src_buffer, loc,
                                       "Source buffer %s has a float value at offset %" PRIu32 " that is not in the range [0, 1].",
                                       gpuav.FormatHandle(src_buffer).c_str(), texel_offset);

                break;
            }
            default:
                break;
        }

        return skip;
    };

    cb_state.per_command_error_loggers.emplace_back(std::move(error_logger));
}
}  // namespace valcmd
}  // namespace gpuav
