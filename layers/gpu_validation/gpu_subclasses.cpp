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
#include "spirv-tools/instrument.hpp"
#include "gpu_shaders/gpu_error_header.h"

gpuav::Buffer::Buffer(ValidationStateTracker &dev_data, VkBuffer buff, const VkBufferCreateInfo *pCreateInfo,
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

gpuav::CommandBuffer::CommandBuffer(gpuav::Validator &gpuav, VkCommandBuffer handle, const VkCommandBufferAllocateInfo *pCreateInfo,
                                    const vvl::CommandPool *pool)
    : gpu_tracker::CommandBuffer(gpuav, handle, pCreateInfo, pool), state_(gpuav) {
    if (gpuav.aborted) {
        return;
    }

    AllocateResources();
}

void gpuav::CommandBuffer::AllocateResources() {
    using Func = vvl::Func;

    auto gpuav = static_cast<Validator *>(&dev_data);

    VkResult result = VK_SUCCESS;

    const VkShaderStageFlags all_stages_flags = VK_SHADER_STAGE_ALL_GRAPHICS | VK_SHADER_STAGE_COMPUTE_BIT |
                                                VK_SHADER_STAGE_MESH_BIT_EXT | VK_SHADER_STAGE_TASK_BIT_EXT |
                                                kShaderStageAllRayTracing;

    // Instrumentation descriptor set layout
    {
        assert(!gpuav->validation_bindings_.empty());
        VkDescriptorSetLayoutCreateInfo instrumentation_desc_set_layout_ci = vku::InitStructHelper();
        instrumentation_desc_set_layout_ci.bindingCount = static_cast<uint32_t>(gpuav->validation_bindings_.size());
        instrumentation_desc_set_layout_ci.pBindings = gpuav->validation_bindings_.data();
        result = DispatchCreateDescriptorSetLayout(gpuav->device, &instrumentation_desc_set_layout_ci, nullptr,
                                                   &instrumentation_desc_set_layout_);
        if (result != VK_SUCCESS) {
            gpuav->ReportSetupProblem(gpuav->device, Location(Func::vkAllocateCommandBuffers),
                                      "Unable to create instrumentation descriptor set layout. Aborting GPU-AV");
            gpuav->aborted = true;
            return;
        }
    }

    // Error output buffer
    if (!gpuav->AllocateOutputMem(error_output_buffer_, Location(Func::vkAllocateCommandBuffers))) {
        return;
    }

    // Commands errors counts buffer
    {
        VkBufferCreateInfo buffer_info = vku::InitStructHelper();
        buffer_info.size = GetCmdErrorsCountsBufferByteSize();
        buffer_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        VmaAllocationCreateInfo alloc_info = {};
        alloc_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        alloc_info.pool = gpuav->output_buffer_pool;
        result = vmaCreateBuffer(gpuav->vmaAllocator, &buffer_info, &alloc_info, &cmd_errors_counts_buffer_.buffer,
                                 &cmd_errors_counts_buffer_.allocation, nullptr);
        if (result != VK_SUCCESS) {
            gpuav->ReportSetupProblem(
                gpuav->device, Location(Func::vkAllocateCommandBuffers),
                "Unable to allocate device memory for commands errors counts buffer. Device could become unstable.", true);
            gpuav->aborted = true;
            return;
        }

        ClearCmdErrorsCountsBuffer();
        if (gpuav->aborted) {
            return;
        }
    }

    // Update validation commands common descriptor set
    {
        const std::vector<VkDescriptorSetLayoutBinding> validation_cmd_bindings = {
            // Error output buffer
            {glsl::kBindingDiagErrorBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, all_stages_flags, nullptr},
            // Buffer holding action command index in command buffer
            {glsl::kBindingDiagActionIndex, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1, all_stages_flags, nullptr},
            // Buffer holding a resource index from the per command buffer command resources list
            {glsl::kBindingDiagCmdResourceIndex, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1, all_stages_flags, nullptr},
            // Commands errors counts buffer
            {glsl::kBindingDiagCmdErrorsCount, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, all_stages_flags, nullptr},
        };

        VkDescriptorSetLayoutCreateInfo validation_cmd_desc_set_layout_ci = vku::InitStructHelper();
        validation_cmd_desc_set_layout_ci.bindingCount = static_cast<uint32_t>(validation_cmd_bindings.size());
        validation_cmd_desc_set_layout_ci.pBindings = validation_cmd_bindings.data();
        result = DispatchCreateDescriptorSetLayout(gpuav->device, &validation_cmd_desc_set_layout_ci, nullptr,
                                                   &validation_cmd_desc_set_layout_);
        if (result != VK_SUCCESS) {
            gpuav->ReportSetupProblem(gpuav->device, Location(Func::vkAllocateCommandBuffers),
                                      "Unable to create descriptor set layout used for validation commands. Aborting GPU-AV");
            gpuav->aborted = true;
            return;
        }

        assert(validation_cmd_desc_pool_ == VK_NULL_HANDLE);
        assert(validation_cmd_desc_set_ == VK_NULL_HANDLE);
        result = gpuav->desc_set_manager->GetDescriptorSet(&validation_cmd_desc_pool_, validation_cmd_desc_set_layout_,
                                                           &validation_cmd_desc_set_);
        if (result != VK_SUCCESS) {
            gpuav->ReportSetupProblem(gpuav->device, Location(Func::vkAllocateCommandBuffers),
                                      "Unable to create descriptor set used for validation commands. Aborting GPU-AV");
            gpuav->aborted = true;
            return;
        }

        std::array<VkWriteDescriptorSet, 4> validation_cmd_descriptor_writes = {};
        assert(validation_cmd_bindings.size() == validation_cmd_descriptor_writes.size());

        VkDescriptorBufferInfo error_output_buffer_desc_info = {};

        assert(error_output_buffer_.buffer != VK_NULL_HANDLE);
        error_output_buffer_desc_info.buffer = error_output_buffer_.buffer;
        error_output_buffer_desc_info.offset = 0;
        error_output_buffer_desc_info.range = VK_WHOLE_SIZE;

        validation_cmd_descriptor_writes[0] = vku::InitStructHelper();
        validation_cmd_descriptor_writes[0].dstBinding = glsl::kBindingDiagErrorBuffer;
        validation_cmd_descriptor_writes[0].descriptorCount = 1;
        validation_cmd_descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        validation_cmd_descriptor_writes[0].pBufferInfo = &error_output_buffer_desc_info;
        validation_cmd_descriptor_writes[0].dstSet = GetValidationCmdCommonDescriptorSet();

        VkDescriptorBufferInfo cmd_indices_buffer_desc_info = {};

        assert(error_output_buffer_.buffer != VK_NULL_HANDLE);
        cmd_indices_buffer_desc_info.buffer = gpuav->indices_buffer.buffer;
        cmd_indices_buffer_desc_info.offset = 0;
        cmd_indices_buffer_desc_info.range = sizeof(uint32_t);

        validation_cmd_descriptor_writes[1] = vku::InitStructHelper();
        validation_cmd_descriptor_writes[1].dstBinding = glsl::kBindingDiagActionIndex;
        validation_cmd_descriptor_writes[1].descriptorCount = 1;
        validation_cmd_descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        validation_cmd_descriptor_writes[1].pBufferInfo = &cmd_indices_buffer_desc_info;
        validation_cmd_descriptor_writes[1].dstSet = GetValidationCmdCommonDescriptorSet();

        validation_cmd_descriptor_writes[2] = validation_cmd_descriptor_writes[1];
        validation_cmd_descriptor_writes[2].dstBinding = glsl::kBindingDiagCmdResourceIndex;

        VkDescriptorBufferInfo cmd_errors_count_buffer_desc_info = {};
        cmd_errors_count_buffer_desc_info.buffer = GetCmdErrorsCountsBuffer();
        cmd_errors_count_buffer_desc_info.offset = 0;
        cmd_errors_count_buffer_desc_info.range = VK_WHOLE_SIZE;

        validation_cmd_descriptor_writes[3] = vku::InitStructHelper();
        validation_cmd_descriptor_writes[3].dstBinding = glsl::kBindingDiagCmdErrorsCount;
        validation_cmd_descriptor_writes[3].descriptorCount = 1;
        validation_cmd_descriptor_writes[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        validation_cmd_descriptor_writes[3].pBufferInfo = &cmd_errors_count_buffer_desc_info;
        validation_cmd_descriptor_writes[3].dstSet = GetValidationCmdCommonDescriptorSet();

        DispatchUpdateDescriptorSets(gpuav->device, static_cast<uint32_t>(validation_cmd_descriptor_writes.size()),
                                     validation_cmd_descriptor_writes.data(), 0, NULL);
    }
}

gpuav::CommandBuffer::~CommandBuffer() { Destroy(); }

void gpuav::CommandBuffer::Destroy() {
    ResetCBState();
    auto gpuav = static_cast<Validator *>(&dev_data);

    if (instrumentation_desc_set_layout_ != VK_NULL_HANDLE) {
        DispatchDestroyDescriptorSetLayout(gpuav->device, instrumentation_desc_set_layout_, nullptr);
        instrumentation_desc_set_layout_ = VK_NULL_HANDLE;
    }

    if (validation_cmd_desc_set_layout_ != VK_NULL_HANDLE) {
        DispatchDestroyDescriptorSetLayout(gpuav->device, validation_cmd_desc_set_layout_, nullptr);
        validation_cmd_desc_set_layout_ = VK_NULL_HANDLE;
    }

    vvl::CommandBuffer::Destroy();
}

void gpuav::CommandBuffer::Reset() {
    vvl::CommandBuffer::Reset();
    ResetCBState();
    // TODO: Calling AllocateResources in Reset like so is a kind of a hack,
    // relying on CommandBuffer internal logic to work.
    // Tried to call it in ResetCBState, hang on command buffer mutex :/
    AllocateResources();
}

void gpuav::CommandBuffer::ResetCBState() {
    auto gpuav = static_cast<Validator *>(&dev_data);
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

    error_output_buffer_.Destroy(gpuav->vmaAllocator);
    cmd_errors_counts_buffer_.Destroy(gpuav->vmaAllocator);

    gpuav->desc_set_manager->PutBackDescriptorSet(validation_cmd_desc_pool_, validation_cmd_desc_set_);
    validation_cmd_desc_pool_ = VK_NULL_HANDLE;
    validation_cmd_desc_set_ = VK_NULL_HANDLE;

    draw_index = compute_index = trace_rays_index = 0;
}

void gpuav::CommandBuffer::ClearCmdErrorsCountsBuffer() const {
    auto gpuav = static_cast<Validator *>(&dev_data);
    uint32_t *cmd_errors_counts_buffer_ptr = nullptr;
    VkResult result = vmaMapMemory(gpuav->vmaAllocator, cmd_errors_counts_buffer_.allocation,
                                   reinterpret_cast<void **>(&cmd_errors_counts_buffer_ptr));
    if (result != VK_SUCCESS) {
        gpuav->ReportSetupProblem(gpuav->device, Location(vvl::Func::vkAllocateCommandBuffers),
                                  "Unable to map device memory for commands errors counts buffer. Device could become unstable.",
                                  true);
        gpuav->aborted = true;
        return;
    }
    std::memset(cmd_errors_counts_buffer_ptr, 0, static_cast<size_t>(GetCmdErrorsCountsBufferByteSize()));
    vmaUnmapMemory(gpuav->vmaAllocator, cmd_errors_counts_buffer_.allocation);
}

bool gpuav::CommandBuffer::PreProcess() {
    state_.UpdateInstrumentationBuffer(this);
    return !per_command_resources.empty() || has_build_as_cmd;
}

// For the given command buffer, map its debug data buffers and read their contents for analysis.
void gpuav::CommandBuffer::PostProcess(VkQueue queue, const Location &loc) {
    auto gpuav = static_cast<Validator *>(&dev_data);
    bool error_found = false;
    uint32_t *error_output_buffer_ptr = nullptr;
    VkResult result =
        vmaMapMemory(gpuav->vmaAllocator, error_output_buffer_.allocation, reinterpret_cast<void **>(&error_output_buffer_ptr));
    assert(result == VK_SUCCESS);
    if (result == VK_SUCCESS) {
        // The second word in the debug output buffer is the number of words that would have
        // been written by the shader instrumentation, if there was enough room in the buffer we provided.
        // The number of words actually written by the shaders is determined by the size of the buffer
        // we provide via the descriptor. So, we process only the number of words that can fit in the
        // buffer.
        const uint32_t total_words = error_output_buffer_ptr[spvtools::kDebugOutputSizeOffset];
        // A zero here means that the shader instrumentation didn't write anything.
        if (total_words != 0) {
            uint32_t *const error_records_start = &error_output_buffer_ptr[spvtools::kDebugOutputDataOffset];
            assert(gpuav->output_buffer_byte_size > spvtools::kDebugOutputDataOffset);
            uint32_t *const error_records_end =
                error_output_buffer_ptr + (gpuav->output_buffer_byte_size - spvtools::kDebugOutputDataOffset);

            uint32_t *error_record = error_records_start;
            uint32_t record_size = error_record[gpuav::glsl::kHeaderErrorRecordSizeOffset];
            assert(record_size == glsl::kErrorRecordSize);

            while (record_size > 0 && (error_record + record_size) <= error_records_end) {
                const uint32_t resource_index = error_record[gpuav::glsl::kHeaderCommandResourceIdOffset];
                assert(resource_index < per_command_resources.size());
                auto &cmd_info = per_command_resources[resource_index];
                const LogObjectList objlist(queue, VkHandle());
                cmd_info->LogValidationMessage(*gpuav, queue, VkHandle(), error_record, cmd_info->operation_index, objlist);

                // Next record
                error_record += record_size;
                record_size = error_record[gpuav::glsl::kHeaderErrorRecordSizeOffset];
            }

            // Clear the written size and any error messages. Note that this preserves the first word, which contains flags.
            assert(gpuav->output_buffer_byte_size > spvtools::kDebugOutputDataOffset);
            memset(&error_output_buffer_ptr[spvtools::kDebugOutputDataOffset], 0,
                   gpuav->output_buffer_byte_size - spvtools::kDebugOutputDataOffset * sizeof(uint32_t));
        }
        error_output_buffer_ptr[spvtools::kDebugOutputSizeOffset] = 0;
        vmaUnmapMemory(gpuav->vmaAllocator, error_output_buffer_.allocation);
    }

    ClearCmdErrorsCountsBuffer();
    if (gpuav->aborted) {
        return;
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

    state_.UpdateCmdBufImageLayouts(*this);
}



gpuav::Queue::Queue(Validator &state, VkQueue q, uint32_t index, VkDeviceQueueCreateFlags flags, const VkQueueFamilyProperties &qfp)
    : gpu_tracker::Queue(state, q, index, flags, qfp) {}

uint64_t gpuav::Queue::PreSubmit(std::vector<vvl::QueueSubmission> &&submissions) {
    auto loc = submissions[0].loc.Get();
    auto &gpuav = static_cast<gpuav::Validator &>(state_);
    gpuav.UpdateBDABuffer(loc);
    if (gpuav.aborted) {
        return 0;
    }
    return gpu_tracker::Queue::PreSubmit(std::move(submissions));
}
