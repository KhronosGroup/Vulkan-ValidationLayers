/* Copyright (c) 2018-2024 The Khronos Group Inc.
 * Copyright (c) 2018-2024 Valve Corporation
 * Copyright (c) 2018-2024 LunarG, Inc.
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

gpuav::Buffer::Buffer(ValidationStateTracker *dev_data, VkBuffer buff, const VkBufferCreateInfo *pCreateInfo,
                      DescriptorHeap &desc_heap_)
    : vvl::Buffer(dev_data, buff, pCreateInfo),
      desc_heap(desc_heap_),
      id(desc_heap.NextId(VulkanTypedHandle(buff, kVulkanObjectTypeBuffer))) {}

void gpuav::Buffer::Destroy() {
    desc_heap.DeleteId(id);
    vvl::Buffer::Destroy();
}

void gpuav::Buffer::NotifyInvalidate(const NodeList &invalid_nodes, bool unlink) {
    desc_heap.DeleteId(id);
    vvl::Buffer::NotifyInvalidate(invalid_nodes, unlink);
}

gpuav::BufferView::BufferView(const std::shared_ptr<vvl::Buffer> &bf, VkBufferView bv, const VkBufferViewCreateInfo *ci,
                              VkFormatFeatureFlags2KHR buf_ff, DescriptorHeap &desc_heap_)
    : vvl::BufferView(bf, bv, ci, buf_ff),
      desc_heap(desc_heap_),
      id(desc_heap.NextId(VulkanTypedHandle(bv, kVulkanObjectTypeBufferView))) {}

void gpuav::BufferView::Destroy() {
    desc_heap.DeleteId(id);
    vvl::BufferView::Destroy();
}

void gpuav::BufferView::NotifyInvalidate(const NodeList &invalid_nodes, bool unlink) {
    desc_heap.DeleteId(id);
    vvl::BufferView::NotifyInvalidate(invalid_nodes, unlink);
}

gpuav::ImageView::ImageView(const std::shared_ptr<vvl::Image> &image_state, VkImageView iv, const VkImageViewCreateInfo *ci,
                            VkFormatFeatureFlags2KHR ff, const VkFilterCubicImageViewImageFormatPropertiesEXT &cubic_props,
                            DescriptorHeap &desc_heap_)
    : vvl::ImageView(image_state, iv, ci, ff, cubic_props),
      desc_heap(desc_heap_),
      id(desc_heap.NextId(VulkanTypedHandle(iv, kVulkanObjectTypeImageView))) {}

void gpuav::ImageView::Destroy() {
    desc_heap.DeleteId(id);
    vvl::ImageView::Destroy();
}

void gpuav::ImageView::NotifyInvalidate(const NodeList &invalid_nodes, bool unlink) {
    desc_heap.DeleteId(id);
    vvl::ImageView::NotifyInvalidate(invalid_nodes, unlink);
}

gpuav::Sampler::Sampler(const VkSampler s, const VkSamplerCreateInfo *pci, DescriptorHeap &desc_heap_)
    : vvl::Sampler(s, pci), desc_heap(desc_heap_), id(desc_heap.NextId(VulkanTypedHandle(s, kVulkanObjectTypeSampler))) {}

void gpuav::Sampler::Destroy() {
    desc_heap.DeleteId(id);
    vvl::Sampler::Destroy();
}

void gpuav::Sampler::NotifyInvalidate(const NodeList &invalid_nodes, bool unlink) {
    desc_heap.DeleteId(id);
    vvl::Sampler::NotifyInvalidate(invalid_nodes, unlink);
}

gpuav::AccelerationStructureKHR::AccelerationStructureKHR(VkAccelerationStructureKHR as,
                                                          const VkAccelerationStructureCreateInfoKHR *ci,
                                                          std::shared_ptr<vvl::Buffer> &&buf_state, DescriptorHeap &desc_heap_)
    : vvl::AccelerationStructureKHR(as, ci, std::move(buf_state)),
      desc_heap(desc_heap_),
      id(desc_heap.NextId(VulkanTypedHandle(as, kVulkanObjectTypeAccelerationStructureKHR))) {}

void gpuav::AccelerationStructureKHR::Destroy() {
    desc_heap.DeleteId(id);
    vvl::AccelerationStructureKHR::Destroy();
}

void gpuav::AccelerationStructureKHR::NotifyInvalidate(const NodeList &invalid_nodes, bool unlink) {
    desc_heap.DeleteId(id);
    vvl::AccelerationStructureKHR::NotifyInvalidate(invalid_nodes, unlink);
}

gpuav::AccelerationStructureNV::AccelerationStructureNV(VkDevice device, VkAccelerationStructureNV as,
                                                        const VkAccelerationStructureCreateInfoNV *ci, DescriptorHeap &desc_heap_)
    : vvl::AccelerationStructureNV(device, as, ci),
      desc_heap(desc_heap_),
      id(desc_heap.NextId(VulkanTypedHandle(as, kVulkanObjectTypeAccelerationStructureNV))) {}

void gpuav::AccelerationStructureNV::Destroy() {
    desc_heap.DeleteId(id);
    vvl::AccelerationStructureNV::Destroy();
}

void gpuav::AccelerationStructureNV::NotifyInvalidate(const NodeList &invalid_nodes, bool unlink) {
    desc_heap.DeleteId(id);
    vvl::AccelerationStructureNV::NotifyInvalidate(invalid_nodes, unlink);
}

gpuav::CommandBuffer::CommandBuffer(gpuav::Validator *ga, VkCommandBuffer cb, const VkCommandBufferAllocateInfo *pCreateInfo,
                                    const vvl::CommandPool *pool)
    : gpu_tracker::CommandBuffer(ga, cb, pCreateInfo, pool), state_(*ga) {}

gpuav::CommandBuffer::~CommandBuffer() { Destroy(); }

void gpuav::CommandBuffer::Destroy() {
    ResetCBState();
    vvl::CommandBuffer::Destroy();
}

void gpuav::CommandBuffer::Reset() {
    vvl::CommandBuffer::Reset();
    ResetCBState();
}

void gpuav::CommandBuffer::ResetCBState() {
    auto gpuav = static_cast<Validator *>(dev_data);
    // Free the device memory and descriptor set(s) associated with a command buffer.

    for (auto &cmd_info : per_command_resources) {
        cmd_info->Destroy(*gpuav);
    }
    per_command_resources.clear();

    for (auto &buffer_info : di_input_buffer_list) {
        vmaDestroyBuffer(gpuav->vmaAllocator, buffer_info.bindless_state_buffer, buffer_info.bindless_state_buffer_allocation);
    }
    di_input_buffer_list.clear();
    current_bindless_buffer = VK_NULL_HANDLE;

    for (auto &as_validation_buffer_info : as_validation_buffers) {
        gpuav->Destroy(as_validation_buffer_info);
    }
    as_validation_buffers.clear();
}

bool gpuav::CommandBuffer::PreProcess() {
    state_.UpdateInstrumentationBuffer(this);
    return !per_command_resources.empty() || has_build_as_cmd;
}

// For the given command buffer, map its debug data buffers and read their contents for analysis.
void gpuav::CommandBuffer::PostProcess(VkQueue queue, const Location &loc) {
    auto *device_state = static_cast<Validator *>(dev_data);
    uint32_t draw_index = 0;
    uint32_t compute_index = 0;
    uint32_t ray_trace_index = 0;
    bool error_found = false;

    for (auto &cmd_info : per_command_resources) {
        uint32_t operation_index = 0;
        if (cmd_info->pipeline_bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS) {
            operation_index = draw_index++;
        } else if (cmd_info->pipeline_bind_point == VK_PIPELINE_BIND_POINT_COMPUTE) {
            operation_index = compute_index++;
        } else if (cmd_info->pipeline_bind_point == VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR) {
            operation_index = ray_trace_index++;
        }

        error_found |= cmd_info->LogErrorIfAny(*device_state, queue, VkHandle(), operation_index);
    }

    // If instrumentation found an error, skip post processing. Errors detected by instrumentation are usually
    // very serious, such as a prematurely destroyed resource and the state needed below is likely invalid.
    if (!error_found) {
        // For each vkCmdBindDescriptorSets()...
        // Some applications repeatedly call vkCmdBindDescriptorSets() with the same descriptor sets, avoid
        // checking them multiple times.
        vvl::unordered_set<VkDescriptorSet> validated_desc_sets;
        for (auto &di_info : di_input_buffer_list) {
            Location draw_loc(vvl::Func::vkCmdDraw);
            // For each descriptor set ...
            for (uint32_t i = 0;  i < di_info.descriptor_set_buffers.size(); i++) {
                auto &set = di_info.descriptor_set_buffers[i];
                if (validated_desc_sets.count(set.state->VkHandle()) > 0) {
                    continue;
                }
                validated_desc_sets.emplace(set.state->VkHandle());
                assert(set.output_state);

                vvl::DescriptorValidator context(state_, *this, *set.state, i, VK_NULL_HANDLE /*framebuffer*/, draw_loc);
                auto used_descs = set.output_state->UsedDescriptors(*set.state);
                // For each used binding ...
                for (const auto &u : used_descs) {
                    auto iter = set.binding_req.find(u.first);
                    vvl::DescriptorBindingInfo binding_info;
                    binding_info.first = u.first;
                    while (iter != set.binding_req.end() && iter->first == u.first) {
                        binding_info.second.emplace_back(iter->second);
                        ++iter;
                    }
                    context.ValidateBinding(binding_info, u.second);
                }
            }
        }
    }
    ProcessAccelerationStructure(queue, loc);
    state_.UpdateCmdBufImageLayouts(*this);
}

void gpuav::CommandBuffer::ProcessAccelerationStructure(VkQueue queue, const Location &loc) {
    if (!has_build_as_cmd) {
        return;
    }
    for (const auto &as_validation_buffer_info : as_validation_buffers) {
        glsl::AccelerationStructureBuildValidationBuffer *mapped_validation_buffer = nullptr;

        VkResult result = vmaMapMemory(state_.vmaAllocator, as_validation_buffer_info.buffer_allocation,
                                       reinterpret_cast<void **>(&mapped_validation_buffer));
        if (result == VK_SUCCESS) {
            if (mapped_validation_buffer->invalid_handle_found > 0) {
                const std::array<uint32_t, 2> invalid_handles = {mapped_validation_buffer->invalid_handle_bits_0,
                                                                 mapped_validation_buffer->invalid_handle_bits_1};
                const uint64_t invalid_handle = vvl_bit_cast<uint64_t>(invalid_handles);

                state_.LogError(
                    "UNASSIGNED-AccelerationStructure", as_validation_buffer_info.acceleration_structure, loc,
                    "Attempted to build top level acceleration structure using invalid bottom level acceleration structure "
                    "handle (%" PRIu64 ")",
                    invalid_handle);
            }
            vmaUnmapMemory(state_.vmaAllocator, as_validation_buffer_info.buffer_allocation);
        }
    }
}

gpuav::Queue::Queue(Validator &state, VkQueue q, uint32_t index, VkDeviceQueueCreateFlags flags, const VkQueueFamilyProperties &qfp)
    : gpu_tracker::Queue(state, q, index, flags, qfp) {}

uint64_t gpuav::Queue::PreSubmit(std::vector<vvl::QueueSubmission> &&submissions) {
    auto loc = submissions[0].loc.Get();
    static_cast<gpuav::Validator&>(state_).UpdateBDABuffer(loc);
    return gpu_tracker::Queue::PreSubmit(std::move(submissions));
}
