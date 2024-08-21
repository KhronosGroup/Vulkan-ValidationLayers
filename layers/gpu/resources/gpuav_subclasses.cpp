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

#include "gpu/resources/gpuav_subclasses.h"

#include "gpu/resources/gpu_shader_resources.h"
#include "gpu/core/gpuav.h"
#include "gpu/core/gpuav_constants.h"
#include "gpu/descriptor_validation/gpuav_image_layout.h"
#include "gpu/descriptor_validation/gpuav_descriptor_validation.h"
#include "gpu/shaders/gpu_error_header.h"

namespace gpuav {

Buffer::Buffer(ValidationStateTracker &dev_data, VkBuffer buff, const VkBufferCreateInfo *pCreateInfo, DescriptorHeap &desc_heap_)
    : vvl::Buffer(dev_data, buff, pCreateInfo),
      desc_heap(desc_heap_),
      id(desc_heap.NextId(VulkanTypedHandle(buff, kVulkanObjectTypeBuffer))) {}

void Buffer::Destroy() {
    desc_heap.DeleteId(id);
    vvl::Buffer::Destroy();
}

void Buffer::NotifyInvalidate(const NodeList &invalid_nodes, bool unlink) {
    desc_heap.DeleteId(id);
    vvl::Buffer::NotifyInvalidate(invalid_nodes, unlink);
}

BufferView::BufferView(const std::shared_ptr<vvl::Buffer> &bf, VkBufferView bv, const VkBufferViewCreateInfo *ci,
                       VkFormatFeatureFlags2KHR buf_ff, DescriptorHeap &desc_heap_)
    : vvl::BufferView(bf, bv, ci, buf_ff),
      desc_heap(desc_heap_),
      id(desc_heap.NextId(VulkanTypedHandle(bv, kVulkanObjectTypeBufferView))) {}

void BufferView::Destroy() {
    desc_heap.DeleteId(id);
    vvl::BufferView::Destroy();
}

void BufferView::NotifyInvalidate(const NodeList &invalid_nodes, bool unlink) {
    desc_heap.DeleteId(id);
    vvl::BufferView::NotifyInvalidate(invalid_nodes, unlink);
}

ImageView::ImageView(const std::shared_ptr<vvl::Image> &image_state, VkImageView iv, const VkImageViewCreateInfo *ci,
                     VkFormatFeatureFlags2KHR ff, const VkFilterCubicImageViewImageFormatPropertiesEXT &cubic_props,
                     DescriptorHeap &desc_heap_)
    : vvl::ImageView(image_state, iv, ci, ff, cubic_props),
      desc_heap(desc_heap_),
      id(desc_heap.NextId(VulkanTypedHandle(iv, kVulkanObjectTypeImageView))) {}

void ImageView::Destroy() {
    desc_heap.DeleteId(id);
    vvl::ImageView::Destroy();
}

void ImageView::NotifyInvalidate(const NodeList &invalid_nodes, bool unlink) {
    desc_heap.DeleteId(id);
    vvl::ImageView::NotifyInvalidate(invalid_nodes, unlink);
}

Sampler::Sampler(const VkSampler s, const VkSamplerCreateInfo *pci, DescriptorHeap &desc_heap_)
    : vvl::Sampler(s, pci), desc_heap(desc_heap_), id(desc_heap.NextId(VulkanTypedHandle(s, kVulkanObjectTypeSampler))) {}

void Sampler::Destroy() {
    desc_heap.DeleteId(id);
    vvl::Sampler::Destroy();
}

void Sampler::NotifyInvalidate(const NodeList &invalid_nodes, bool unlink) {
    desc_heap.DeleteId(id);
    vvl::Sampler::NotifyInvalidate(invalid_nodes, unlink);
}

AccelerationStructureKHR::AccelerationStructureKHR(VkAccelerationStructureKHR as, const VkAccelerationStructureCreateInfoKHR *ci,
                                                   std::shared_ptr<vvl::Buffer> &&buf_state, DescriptorHeap &desc_heap_)
    : vvl::AccelerationStructureKHR(as, ci, std::move(buf_state)),
      desc_heap(desc_heap_),
      id(desc_heap.NextId(VulkanTypedHandle(as, kVulkanObjectTypeAccelerationStructureKHR))) {}

void AccelerationStructureKHR::Destroy() {
    desc_heap.DeleteId(id);
    vvl::AccelerationStructureKHR::Destroy();
}

void AccelerationStructureKHR::NotifyInvalidate(const NodeList &invalid_nodes, bool unlink) {
    desc_heap.DeleteId(id);
    vvl::AccelerationStructureKHR::NotifyInvalidate(invalid_nodes, unlink);
}

AccelerationStructureNV::AccelerationStructureNV(VkDevice device, VkAccelerationStructureNV as,
                                                 const VkAccelerationStructureCreateInfoNV *ci, DescriptorHeap &desc_heap_)
    : vvl::AccelerationStructureNV(device, as, ci),
      desc_heap(desc_heap_),
      id(desc_heap.NextId(VulkanTypedHandle(as, kVulkanObjectTypeAccelerationStructureNV))) {}

void AccelerationStructureNV::Destroy() {
    desc_heap.DeleteId(id);
    vvl::AccelerationStructureNV::Destroy();
}

void AccelerationStructureNV::NotifyInvalidate(const NodeList &invalid_nodes, bool unlink) {
    desc_heap.DeleteId(id);
    vvl::AccelerationStructureNV::NotifyInvalidate(invalid_nodes, unlink);
}

CommandBuffer::CommandBuffer(Validator &gpuav, VkCommandBuffer handle, const VkCommandBufferAllocateInfo *pCreateInfo,
                             const vvl::CommandPool *pool)
    : gpu_tracker::CommandBuffer(gpuav, handle, pCreateInfo, pool),
      gpu_resources_manager(gpuav.vma_allocator_, *gpuav.desc_set_manager_),
      state_(gpuav) {
    Location loc(vvl::Func::vkAllocateCommandBuffers);
    AllocateResources(loc);
}

static bool AllocateErrorLogsBuffer(Validator &gpuav, gpu::DeviceMemoryBlock &error_logs_mem, const Location &loc) {
    VkBufferCreateInfo buffer_info = vku::InitStructHelper();
    buffer_info.size = glsl::kErrorBufferByteSize;
    buffer_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    VmaAllocationCreateInfo alloc_info = {};
    alloc_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    alloc_info.pool = gpuav.output_buffer_pool_;
    VkResult result = vmaCreateBuffer(gpuav.vma_allocator_, &buffer_info, &alloc_info, &error_logs_mem.buffer,
                                      &error_logs_mem.allocation, nullptr);
    if (result != VK_SUCCESS) {
        gpuav.InternalError(gpuav.device, loc, "Unable to allocate device memory for error output buffer.", true);
        return false;
    }

    uint32_t *output_buffer_ptr;
    result = vmaMapMemory(gpuav.vma_allocator_, error_logs_mem.allocation, reinterpret_cast<void **>(&output_buffer_ptr));
    if (result == VK_SUCCESS) {
        memset(output_buffer_ptr, 0, glsl::kErrorBufferByteSize);
        if (gpuav.gpuav_settings.shader_instrumentation.bindless_descriptor) {
            output_buffer_ptr[cst::stream_output_flags_offset] = cst::inst_buffer_oob_enabled;
        }
        vmaUnmapMemory(gpuav.vma_allocator_, error_logs_mem.allocation);
    } else {
        gpuav.InternalError(gpuav.device, loc, "Unable to map device memory allocated for error output buffer.", true);
        return false;
    }

    return true;
}

void CommandBuffer::AllocateResources(const Location &loc) {
    auto gpuav = static_cast<Validator *>(&dev_data);

    VkResult result = VK_SUCCESS;

    // Instrumentation descriptor set layout
    if (instrumentation_desc_set_layout_ == VK_NULL_HANDLE) {
        assert(!gpuav->instrumentation_bindings_.empty());
        VkDescriptorSetLayoutCreateInfo instrumentation_desc_set_layout_ci = vku::InitStructHelper();
        instrumentation_desc_set_layout_ci.bindingCount = static_cast<uint32_t>(gpuav->instrumentation_bindings_.size());
        instrumentation_desc_set_layout_ci.pBindings = gpuav->instrumentation_bindings_.data();
        result = DispatchCreateDescriptorSetLayout(gpuav->device, &instrumentation_desc_set_layout_ci, nullptr,
                                                   &instrumentation_desc_set_layout_);
        if (result != VK_SUCCESS) {
            gpuav->InternalError(gpuav->device, loc, "Unable to create instrumentation descriptor set layout.");
            return;
        }
    }

    // Error output buffer
    if (!AllocateErrorLogsBuffer(*gpuav, error_output_buffer_, loc)) {
        return;
    }

    // Commands errors counts buffer
    {
        VkBufferCreateInfo buffer_info = vku::InitStructHelper();
        buffer_info.size = GetCmdErrorsCountsBufferByteSize();
        buffer_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        VmaAllocationCreateInfo alloc_info = {};
        alloc_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        alloc_info.pool = gpuav->output_buffer_pool_;
        result = vmaCreateBuffer(gpuav->vma_allocator_, &buffer_info, &alloc_info, &cmd_errors_counts_buffer_.buffer,
                                 &cmd_errors_counts_buffer_.allocation, nullptr);
        if (result != VK_SUCCESS) {
            gpuav->InternalError(gpuav->device, loc, "Unable to allocate device memory for commands errors counts buffer.", true);
            return;
        }

        ClearCmdErrorsCountsBuffer(loc);
        if (gpuav->aborted_) return;
    }

    // BDA snapshot
    if (gpuav->gpuav_settings.shader_instrumentation.buffer_device_address) {
        VkBufferCreateInfo buffer_info = vku::InitStructHelper();
        buffer_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        VmaAllocationCreateInfo alloc_info = {};
        buffer_info.size = GetBdaRangesBufferByteSize();
        // This buffer could be very large if an application uses many buffers. Allocating it as HOST_CACHED
        // and manually flushing it at the end of the state updates is faster than using HOST_COHERENT.
        alloc_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
        result = vmaCreateBuffer(gpuav->vma_allocator_, &buffer_info, &alloc_info, &bda_ranges_snapshot_.buffer,
                                 &bda_ranges_snapshot_.allocation, nullptr);
        if (result != VK_SUCCESS) {
            gpuav->InternalError(gpuav->device, loc, "Unable to allocate device memory for buffer device address data.", true);
            return;
        }
    }

    // Update validation commands common descriptor set
    {
        const std::vector<VkDescriptorSetLayoutBinding> validation_cmd_bindings = {
            // Error output buffer
            {glsl::kBindingDiagErrorBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
            // Buffer holding action command index in command buffer
            {glsl::kBindingDiagActionIndex, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_ALL, nullptr},
            // Buffer holding a resource index from the per command buffer command resources list
            {glsl::kBindingDiagCmdResourceIndex, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_ALL, nullptr},
            // Commands errors counts buffer
            {glsl::kBindingDiagCmdErrorsCount, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
        };

        if (validation_cmd_desc_set_layout_ == VK_NULL_HANDLE) {
            VkDescriptorSetLayoutCreateInfo validation_cmd_desc_set_layout_ci = vku::InitStructHelper();
            validation_cmd_desc_set_layout_ci.bindingCount = static_cast<uint32_t>(validation_cmd_bindings.size());
            validation_cmd_desc_set_layout_ci.pBindings = validation_cmd_bindings.data();
            result = DispatchCreateDescriptorSetLayout(gpuav->device, &validation_cmd_desc_set_layout_ci, nullptr,
                                                       &validation_cmd_desc_set_layout_);
            if (result != VK_SUCCESS) {
                gpuav->InternalError(gpuav->device, loc, "Unable to create descriptor set layout used for validation commands.");
                return;
            }
        }

        assert(validation_cmd_desc_pool_ == VK_NULL_HANDLE);
        assert(validation_cmd_desc_set_ == VK_NULL_HANDLE);
        result = gpuav->desc_set_manager_->GetDescriptorSet(&validation_cmd_desc_pool_, validation_cmd_desc_set_layout_,
                                                            &validation_cmd_desc_set_);
        if (result != VK_SUCCESS) {
            gpuav->InternalError(gpuav->device, loc, "Unable to create descriptor set used for validation commands.");
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
        cmd_indices_buffer_desc_info.buffer = gpuav->indices_buffer_.buffer;
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

bool CommandBuffer::UpdateBdaRangesBuffer(const Location &loc) {
    auto gpuav = static_cast<Validator *>(&dev_data);

    // By supplying a "date"
    if (!gpuav->gpuav_settings.shader_instrumentation.buffer_device_address ||
        bda_ranges_snapshot_version_ == gpuav->buffer_device_address_ranges_version) {
        return true;
    }

    // Update buffer device address table
    // ---
    VkDeviceAddress *bda_table_ptr = nullptr;
    assert(bda_ranges_snapshot_.allocation);
    VkResult result =
        vmaMapMemory(gpuav->vma_allocator_, bda_ranges_snapshot_.allocation, reinterpret_cast<void **>(&bda_table_ptr));
    if (result != VK_SUCCESS) {
        gpuav->InternalError(gpuav->device, loc, "Unable to map device memory in UpdateBdaRangesBuffer.", true);
        return false;
    }

    // Buffer device address table layout
    // Ranges are sorted from low to high, and do not overlap
    // QWord 0 | Number of *ranges* (1 range occupies 2 QWords)
    // QWord 1 | Range 1 begin
    // QWord 2 | Range 1 end
    // QWord 3 | Range 2 begin
    // QWord 4 | Range 2 end
    // QWord 5 | ...

    const size_t max_recordable_ranges =
        static_cast<size_t>((GetBdaRangesBufferByteSize() - sizeof(uint64_t)) / (2 * sizeof(VkDeviceAddress)));
    auto bda_ranges = reinterpret_cast<ValidationStateTracker::BufferAddressRange *>(bda_table_ptr + 1);
    const auto [ranges_to_update_count, total_address_ranges_count] =
        gpuav->GetBufferAddressRanges(bda_ranges, max_recordable_ranges);
    bda_table_ptr[0] = ranges_to_update_count;

    if (total_address_ranges_count > size_t(gpuav->gpuav_settings.max_bda_in_use)) {
        std::ostringstream problem_string;
        problem_string << "Number of buffer device addresses ranges in use (" << total_address_ranges_count
                       << ") is greater than khronos_validation.gpuav_max_buffer_device_addresses ("
                       << gpuav->gpuav_settings.max_bda_in_use
                       << "). Truncating buffer device address table could result in invalid validation.";
        gpuav->InternalError(gpuav->device, loc, problem_string.str().c_str());
        return false;
    }

    // Post update cleanups
    // ---
    // Flush the BDA buffer before un-mapping so that the new state is visible to the GPU
    result = vmaFlushAllocation(gpuav->vma_allocator_, bda_ranges_snapshot_.allocation, 0, VK_WHOLE_SIZE);
    vmaUnmapMemory(gpuav->vma_allocator_, bda_ranges_snapshot_.allocation);
    bda_ranges_snapshot_version_ = gpuav->buffer_device_address_ranges_version;

    return true;
}

VkDeviceSize CommandBuffer::GetBdaRangesBufferByteSize() const {
    auto gpuav = static_cast<Validator *>(&dev_data);
    return (1                                           // 1 QWORD for the number of address ranges
            + 2 * gpuav->gpuav_settings.max_bda_in_use  // 2 QWORDS per address range
            ) *
           8;
}

CommandBuffer::~CommandBuffer() { Destroy(); }

void CommandBuffer::Destroy() {
    {
        auto guard = WriteLock();
        ResetCBState();
    }
    vvl::CommandBuffer::Destroy();
}

void CommandBuffer::Reset(const Location &loc) {
    vvl::CommandBuffer::Reset(loc);
    ResetCBState();
    // TODO: Calling AllocateResources in Reset like so is a kind of a hack,
    // relying on CommandBuffer internal logic to work.
    // Tried to call it in ResetCBState, hang on command buffer mutex :/
    AllocateResources(loc);
}

void CommandBuffer::ResetCBState() {
    auto gpuav = static_cast<Validator *>(&dev_data);
    // Free the device memory and descriptor set(s) associated with a command buffer.

    gpu_resources_manager.DestroyResources();
    per_command_error_loggers.clear();

    for (auto &buffer_info : di_input_buffer_list) {
        vmaDestroyBuffer(gpuav->vma_allocator_, buffer_info.bindless_state.buffer, buffer_info.bindless_state.allocation);
    }
    di_input_buffer_list.clear();
    current_bindless_buffer = VK_NULL_HANDLE;

    error_output_buffer_.Destroy(gpuav->vma_allocator_);
    cmd_errors_counts_buffer_.Destroy(gpuav->vma_allocator_);
    bda_ranges_snapshot_.Destroy(gpuav->vma_allocator_);
    bda_ranges_snapshot_version_ = 0;

    if (validation_cmd_desc_pool_ != VK_NULL_HANDLE && validation_cmd_desc_set_ != VK_NULL_HANDLE) {
        gpuav->desc_set_manager_->PutBackDescriptorSet(validation_cmd_desc_pool_, validation_cmd_desc_set_);
        validation_cmd_desc_pool_ = VK_NULL_HANDLE;
        validation_cmd_desc_set_ = VK_NULL_HANDLE;
    }

    if (instrumentation_desc_set_layout_ != VK_NULL_HANDLE) {
        DispatchDestroyDescriptorSetLayout(gpuav->device, instrumentation_desc_set_layout_, nullptr);
        instrumentation_desc_set_layout_ = VK_NULL_HANDLE;
    }

    if (validation_cmd_desc_set_layout_ != VK_NULL_HANDLE) {
        DispatchDestroyDescriptorSetLayout(gpuav->device, validation_cmd_desc_set_layout_, nullptr);
        validation_cmd_desc_set_layout_ = VK_NULL_HANDLE;
    }

    draw_index = 0;
    compute_index = 0;
    trace_rays_index = 0;
}

void CommandBuffer::ClearCmdErrorsCountsBuffer(const Location &loc) const {
    auto gpuav = static_cast<Validator *>(&dev_data);
    uint32_t *cmd_errors_counts_buffer_ptr = nullptr;
    VkResult result = vmaMapMemory(gpuav->vma_allocator_, cmd_errors_counts_buffer_.allocation,
                                   reinterpret_cast<void **>(&cmd_errors_counts_buffer_ptr));
    if (result != VK_SUCCESS) {
        gpuav->InternalError(gpuav->device, loc, "Unable to map device memory for commands errors counts buffer.", true);
        return;
    }
    std::memset(cmd_errors_counts_buffer_ptr, 0, static_cast<size_t>(GetCmdErrorsCountsBufferByteSize()));
    vmaUnmapMemory(gpuav->vma_allocator_, cmd_errors_counts_buffer_.allocation);
}

bool CommandBuffer::PreProcess(const Location &loc) {
    auto gpuav = static_cast<Validator *>(&dev_data);

    bool succeeded = UpdateBindlessStateBuffer(*gpuav, *this, loc);
    if (!succeeded) {
        return false;
    }

    succeeded = UpdateBdaRangesBuffer(loc);
    if (!succeeded) {
        return false;
    }

    return !per_command_error_loggers.empty() || has_build_as_cmd;
}

bool CommandBuffer::NeedsPostProcess() { return !error_output_buffer_.IsNull(); }

// For the given command buffer, map its debug data buffers and read their contents for analysis.
void CommandBuffer::PostProcess(VkQueue queue, const Location &loc) {
    // CommandBuffer::Destroy can happen on an other thread,
    // so when getting here after acquiring command buffer's lock,
    // make sure there are still things to process
    if (!NeedsPostProcess()) {
        return;
    }

    auto gpuav = static_cast<Validator *>(&dev_data);
    bool skip = false;
    uint32_t *error_output_buffer_ptr = nullptr;
    VkResult result =
        vmaMapMemory(gpuav->vma_allocator_, error_output_buffer_.allocation, reinterpret_cast<void **>(&error_output_buffer_ptr));
    assert(result == VK_SUCCESS);
    if (result == VK_SUCCESS) {
        // The second word in the debug output buffer is the number of words that would have
        // been written by the shader instrumentation, if there was enough room in the buffer we provided.
        // The number of words actually written by the shaders is determined by the size of the buffer
        // we provide via the descriptor. So, we process only the number of words that can fit in the
        // buffer.
        const uint32_t total_words = error_output_buffer_ptr[cst::stream_output_size_offset];
        // A zero here means that the shader instrumentation didn't write anything.
        if (total_words != 0) {
            uint32_t *const error_records_start = &error_output_buffer_ptr[cst::stream_output_data_offset];
            assert(glsl::kErrorBufferByteSize > cst::stream_output_data_offset);
            uint32_t *const error_records_end =
                error_output_buffer_ptr + (glsl::kErrorBufferByteSize - cst::stream_output_data_offset);

            uint32_t *error_record_ptr = error_records_start;
            uint32_t record_size = error_record_ptr[glsl::kHeaderErrorRecordSizeOffset];
            assert(record_size == glsl::kErrorRecordSize);

            while (record_size > 0 && (error_record_ptr + record_size) <= error_records_end) {
                const uint32_t error_logger_i = error_record_ptr[glsl::kHeaderCommandResourceIdOffset];
                assert(error_logger_i < per_command_error_loggers.size());
                auto &error_logger = per_command_error_loggers[error_logger_i];
                const LogObjectList objlist(queue, VkHandle());
                skip |= error_logger(*gpuav, error_record_ptr, objlist);

                // Next record
                error_record_ptr += record_size;
                record_size = error_record_ptr[glsl::kHeaderErrorRecordSizeOffset];
            }

            // Clear the written size and any error messages. Note that this preserves the first word, which contains flags.
            assert(glsl::kErrorBufferByteSize > cst::stream_output_data_offset);
            memset(&error_output_buffer_ptr[cst::stream_output_data_offset], 0,
                   glsl::kErrorBufferByteSize - cst::stream_output_data_offset * sizeof(uint32_t));
        }
        error_output_buffer_ptr[cst::stream_output_size_offset] = 0;
        vmaUnmapMemory(gpuav->vma_allocator_, error_output_buffer_.allocation);
    }

    ClearCmdErrorsCountsBuffer(loc);
    if (gpuav->aborted_) return;

    // If instrumentation found an error, skip post processing. Errors detected by instrumentation are usually
    // very serious, such as a prematurely destroyed resource and the state needed below is likely invalid.
    bool gpuav_success = false;
    if (!skip) {
        gpuav_success = ValidateBindlessDescriptorSets(loc);
    }

    if (gpuav_success) {
        UpdateCmdBufImageLayouts(state_, *this);
    }
}

}  // namespace gpuav
