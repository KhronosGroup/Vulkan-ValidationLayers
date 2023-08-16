/* Copyright (c) 2018-2023 The Khronos Group Inc.
 * Copyright (c) 2018-2023 Valve Corporation
 * Copyright (c) 2018-2023 LunarG, Inc.
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

#include "gpu_subclasses.h"
#include "gpu_validation.h"
#include "gpu_vuids.h"
#include "drawdispatch/descriptor_validator.h"

gpuav_state::Buffer::Buffer(ValidationStateTracker *dev_data, VkBuffer buff, const VkBufferCreateInfo *pCreateInfo,
                            DescriptorHeap &desc_heap_)
    : BUFFER_STATE(dev_data, buff, pCreateInfo),
      desc_heap(desc_heap_),
      id(desc_heap.NextId(VulkanTypedHandle(buff, kVulkanObjectTypeBuffer))) {}

void gpuav_state::Buffer::Destroy() {
    desc_heap.DeleteId(id);
    BUFFER_STATE::Destroy();
}

void gpuav_state::Buffer::NotifyInvalidate(const NodeList &invalid_nodes, bool unlink) {
    desc_heap.DeleteId(id);
    BUFFER_STATE::NotifyInvalidate(invalid_nodes, unlink);
}

gpuav_state::BufferView::BufferView(const std::shared_ptr<BUFFER_STATE> &bf, VkBufferView bv, const VkBufferViewCreateInfo *ci,
                                    VkFormatFeatureFlags2KHR buf_ff, DescriptorHeap &desc_heap_)
    : BUFFER_VIEW_STATE(bf, bv, ci, buf_ff),
      desc_heap(desc_heap_),
      id(desc_heap.NextId(VulkanTypedHandle(bv, kVulkanObjectTypeBufferView))) {}

void gpuav_state::BufferView::Destroy() {
    desc_heap.DeleteId(id);
    BUFFER_VIEW_STATE::Destroy();
}

void gpuav_state::BufferView::NotifyInvalidate(const NodeList &invalid_nodes, bool unlink) {
    desc_heap.DeleteId(id);
    BUFFER_VIEW_STATE::NotifyInvalidate(invalid_nodes, unlink);
}

gpuav_state::ImageView::ImageView(const std::shared_ptr<IMAGE_STATE> &image_state, VkImageView iv, const VkImageViewCreateInfo *ci,
                                  VkFormatFeatureFlags2KHR ff, const VkFilterCubicImageViewImageFormatPropertiesEXT &cubic_props,
                                  DescriptorHeap &desc_heap_)
    : IMAGE_VIEW_STATE(image_state, iv, ci, ff, cubic_props),
      desc_heap(desc_heap_),
      id(desc_heap.NextId(VulkanTypedHandle(iv, kVulkanObjectTypeImageView))) {}

void gpuav_state::ImageView::Destroy() {
    desc_heap.DeleteId(id);
    IMAGE_VIEW_STATE::Destroy();
}

void gpuav_state::ImageView::NotifyInvalidate(const NodeList &invalid_nodes, bool unlink) {
    desc_heap.DeleteId(id);
    IMAGE_VIEW_STATE::NotifyInvalidate(invalid_nodes, unlink);
}

gpuav_state::Sampler::Sampler(const VkSampler s, const VkSamplerCreateInfo *pci, DescriptorHeap &desc_heap_)
    : SAMPLER_STATE(s, pci), desc_heap(desc_heap_), id(desc_heap.NextId(VulkanTypedHandle(s, kVulkanObjectTypeSampler))) {}

void gpuav_state::Sampler::Destroy() {
    desc_heap.DeleteId(id);
    SAMPLER_STATE::Destroy();
}

void gpuav_state::Sampler::NotifyInvalidate(const NodeList &invalid_nodes, bool unlink) {
    desc_heap.DeleteId(id);
    SAMPLER_STATE::NotifyInvalidate(invalid_nodes, unlink);
}

gpuav_state::AccelerationStructureKHR::AccelerationStructureKHR(VkAccelerationStructureKHR as,
                                                                const VkAccelerationStructureCreateInfoKHR *ci,
                                                                std::shared_ptr<BUFFER_STATE> &&buf_state, VkDeviceAddress address,
                                                                DescriptorHeap &desc_heap_)
    : ACCELERATION_STRUCTURE_STATE_KHR(as, ci, std::move(buf_state), address),
      desc_heap(desc_heap_),
      id(desc_heap.NextId(VulkanTypedHandle(as, kVulkanObjectTypeAccelerationStructureKHR))) {}

void gpuav_state::AccelerationStructureKHR::Destroy() {
    desc_heap.DeleteId(id);
    ACCELERATION_STRUCTURE_STATE_KHR::Destroy();
}

void gpuav_state::AccelerationStructureKHR::NotifyInvalidate(const NodeList &invalid_nodes, bool unlink) {
    desc_heap.DeleteId(id);
    ACCELERATION_STRUCTURE_STATE_KHR::NotifyInvalidate(invalid_nodes, unlink);
}

gpuav_state::AccelerationStructureNV::AccelerationStructureNV(VkDevice device, VkAccelerationStructureNV as,
                                                              const VkAccelerationStructureCreateInfoNV *ci,
                                                              DescriptorHeap &desc_heap_)
    : ACCELERATION_STRUCTURE_STATE_NV(device, as, ci),
      desc_heap(desc_heap_),
      id(desc_heap.NextId(VulkanTypedHandle(as, kVulkanObjectTypeAccelerationStructureNV))) {}

void gpuav_state::AccelerationStructureNV::Destroy() {
    desc_heap.DeleteId(id);
    ACCELERATION_STRUCTURE_STATE_NV::Destroy();
}

void gpuav_state::AccelerationStructureNV::NotifyInvalidate(const NodeList &invalid_nodes, bool unlink) {
    desc_heap.DeleteId(id);
    ACCELERATION_STRUCTURE_STATE_NV::NotifyInvalidate(invalid_nodes, unlink);
}

gpuav_state::CommandBuffer::CommandBuffer(GpuAssisted *ga, VkCommandBuffer cb, const VkCommandBufferAllocateInfo *pCreateInfo,
                                          const COMMAND_POOL_STATE *pool)
    : gpu_utils_state::CommandBuffer(ga, cb, pCreateInfo, pool) {}

gpuav_state::CommandBuffer::~CommandBuffer() { Destroy(); }

void gpuav_state::CommandBuffer::Destroy() {
    ResetCBState();
    CMD_BUFFER_STATE::Destroy();
}

void gpuav_state::CommandBuffer::Reset() {
    CMD_BUFFER_STATE::Reset();
    ResetCBState();
}

void gpuav_state::CommandBuffer::ResetCBState() {
    auto gpuav = static_cast<GpuAssisted *>(dev_data);
    // Free the device memory and descriptor set(s) associated with a command buffer.
    for (auto &cmd_info : per_draw_buffer_list) {
        gpuav->DestroyBuffer(cmd_info);
    }
    per_draw_buffer_list.clear();

    for (auto &buffer_info : di_input_buffer_list) {
        vmaDestroyBuffer(gpuav->vmaAllocator, buffer_info.bindless_state_buffer, buffer_info.bindless_state_buffer_allocation);
    }
    di_input_buffer_list.clear();
    current_bindless_buffer = VK_NULL_HANDLE;

    for (auto &as_validation_buffer_info : as_validation_buffers) {
        gpuav->DestroyBuffer(as_validation_buffer_info);
    }
    as_validation_buffers.clear();
}

// For the given command buffer, map its debug data buffers and read their contents for analysis.
void gpuav_state::CommandBuffer::Process(VkQueue queue, const Location &loc) {
    auto *device_state = static_cast<GpuAssisted *>(dev_data);
    if (has_draw_cmd || has_trace_rays_cmd || has_dispatch_cmd) {
        uint32_t draw_index = 0;
        uint32_t compute_index = 0;
        uint32_t ray_trace_index = 0;

        for (auto &cmd_info : per_draw_buffer_list) {
            char *data;
            gpuav_state::DescBindingInfo *di_info = nullptr;
            if (cmd_info.desc_binding_index != vvl::kU32Max) {
                di_info = &di_input_buffer_list[cmd_info.desc_binding_index];
            }
            std::vector<gpuav_state::DescSetState> empty;

            uint32_t operation_index = 0;
            if (cmd_info.pipeline_bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS) {
                operation_index = draw_index;
                draw_index++;
            } else if (cmd_info.pipeline_bind_point == VK_PIPELINE_BIND_POINT_COMPUTE) {
                operation_index = compute_index;
                compute_index++;
            } else if (cmd_info.pipeline_bind_point == VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR) {
                operation_index = ray_trace_index;
                ray_trace_index++;
            } else {
                assert(false);
            }

            VkResult result = vmaMapMemory(device_state->vmaAllocator, cmd_info.output_mem_block.allocation, (void **)&data);
            if (result == VK_SUCCESS) {
                device_state->AnalyzeAndGenerateMessages(*this, queue, cmd_info, operation_index, (uint32_t *)data,
                                                         di_info ? di_info->descriptor_set_buffers : empty, loc);
                vmaUnmapMemory(device_state->vmaAllocator, cmd_info.output_mem_block.allocation);
            }
        }
        // For each vkCmdBindDescriptorSets()...
        // Some applications repeatedly call vkCmdBindDescriptorSets() with the same descriptor sets, avoid
        // checking them multiple times.
        vvl::unordered_set<VkDescriptorSet> validated_desc_sets;
        for (auto &di_info : di_input_buffer_list) {
            Location draw_loc(vvl::Func::vkCmdDraw);
            // For each descriptor set ...
            for (auto &set : di_info.descriptor_set_buffers) {
                if (validated_desc_sets.count(set.state->GetSet()) > 0) {
                    continue;
                }
                validated_desc_sets.emplace(set.state->GetSet());
                assert(set.output_state);

                vvl::DescriptorValidator context(*device_state, *this, *set.state, VK_NULL_HANDLE /*framebuffer*/, draw_loc);
                auto used_descs = set.output_state->UsedDescriptors(*set.state);
                // For each used binding ...
                for (const auto &u : used_descs) {
                    auto iter = set.binding_req.find(u.first);
                    vvl::DescriptorBindingInfo binding_info{u.first, (iter != set.binding_req.end()) ? iter->second : DescriptorRequirement()};
                    context.ValidateBinding(binding_info, u.second);
                }
            }
        }
    }
    ProcessAccelerationStructure(queue);
}

void gpuav_state::CommandBuffer::ProcessAccelerationStructure(VkQueue queue) {
    if (!has_build_as_cmd) {
        return;
    }
    auto *device_state = static_cast<GpuAssisted *>(dev_data);
    for (const auto &as_validation_buffer_info : as_validation_buffers) {
        gpuav_glsl::AccelerationStructureBuildValidationBuffer *mapped_validation_buffer = nullptr;

        VkResult result = vmaMapMemory(device_state->vmaAllocator, as_validation_buffer_info.buffer_allocation,
                                       reinterpret_cast<void **>(&mapped_validation_buffer));
        if (result == VK_SUCCESS) {
            if (mapped_validation_buffer->invalid_handle_found > 0) {
                const std::array<uint32_t, 2> invalid_handles = {mapped_validation_buffer->invalid_handle_bits_0,
                                                                 mapped_validation_buffer->invalid_handle_bits_1};
                const uint64_t invalid_handle = vvl_bit_cast<uint64_t>(invalid_handles);

                // TODO - pass in Locaiton correctly
                const Location loc(vvl::Func::vkQueueSubmit);
                device_state->LogError(
                    "UNASSIGNED-AccelerationStructure", as_validation_buffer_info.acceleration_structure, loc,
                    "Attempted to build top level acceleration structure using invalid bottom level acceleration structure "
                    "handle (%" PRIu64 ")",
                    invalid_handle);
            }
            vmaUnmapMemory(device_state->vmaAllocator, as_validation_buffer_info.buffer_allocation);
        }
    }
}

