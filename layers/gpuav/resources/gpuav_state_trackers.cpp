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

#include "gpuav/resources/gpuav_state_trackers.h"

#include "gpuav/resources/gpuav_shader_resources.h"
#include "gpuav/core/gpuav.h"
#include "gpuav/core/gpuav_constants.h"
#include "gpuav/descriptor_validation/gpuav_image_layout.h"
#include "gpuav/descriptor_validation/gpuav_descriptor_validation.h"
#include "gpuav/shaders/gpuav_error_header.h"
#include "gpuav/debug_printf/debug_printf.h"
#include "containers/limits.h"

#include "profiling/profiling.h"

namespace gpuav {

CommandBufferSubState::CommandBufferSubState(Validator &gpuav, vvl::CommandBuffer &cb)
    : vvl::CommandBufferSubState(cb),
      gpu_resources_manager(gpuav),
      gpuav_(gpuav),
      cmd_errors_counts_buffer_(gpuav),
      bda_ranges_snapshot_(gpuav) {
    Location loc(vvl::Func::vkAllocateCommandBuffers);
    AllocateResources(loc);
}

CommandBufferSubState::~CommandBufferSubState() {}

void CommandBufferSubState::AllocateResources(const Location &loc) {
    VkResult result = VK_SUCCESS;

    // Instrumentation descriptor set layout
    if (instrumentation_desc_set_layout_ == VK_NULL_HANDLE) {
        assert(!gpuav_.instrumentation_bindings_.empty());
        VkDescriptorSetLayoutCreateInfo instrumentation_desc_set_layout_ci = vku::InitStructHelper();
        instrumentation_desc_set_layout_ci.bindingCount = static_cast<uint32_t>(gpuav_.instrumentation_bindings_.size());
        instrumentation_desc_set_layout_ci.pBindings = gpuav_.instrumentation_bindings_.data();
        result = DispatchCreateDescriptorSetLayout(gpuav_.device, &instrumentation_desc_set_layout_ci, nullptr,
                                                   &instrumentation_desc_set_layout_);
        if (result != VK_SUCCESS) {
            gpuav_.InternalError(gpuav_.device, loc, "Unable to create instrumentation descriptor set layout.");
            return;
        }
    }

    // Error output buffer
    {
        if (error_output_buffer_range_.buffer == VK_NULL_HANDLE) {
            error_output_buffer_range_ = gpu_resources_manager.GetHostVisibleBufferRange(glsl::kErrorBufferByteSize);
            if (error_output_buffer_range_.buffer == VK_NULL_HANDLE) {
                return;
            }
        }
        memset(error_output_buffer_range_.offset_mapped_ptr, 0, (size_t)error_output_buffer_range_.size);
        if (gpuav_.gpuav_settings.shader_instrumentation.descriptor_checks) {
            ((uint32_t *)error_output_buffer_range_.offset_mapped_ptr)[cst::stream_output_flags_offset] =
                cst::inst_buffer_oob_enabled;
        }
    }

    // Commands errors counts buffer
    {
        if (cmd_errors_counts_buffer_.IsDestroyed()) {
            VkBufferCreateInfo buffer_info = vku::InitStructHelper();
            buffer_info.size = GetCmdErrorsCountsBufferByteSize();
            buffer_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
            VmaAllocationCreateInfo alloc_info = {};
            alloc_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            alloc_info.preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            const bool success = cmd_errors_counts_buffer_.Create(&buffer_info, &alloc_info);
            if (!success) {
                return;
            }
        }

        cmd_errors_counts_buffer_.Clear();
        if (gpuav_.aborted_) return;
    }

    // BDA snapshot
    if (gpuav_.gpuav_settings.shader_instrumentation.buffer_device_address) {
        if (bda_ranges_snapshot_.IsDestroyed()) {
            VkBufferCreateInfo buffer_info = vku::InitStructHelper();
            buffer_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
            VmaAllocationCreateInfo alloc_info = {};
            buffer_info.size = GetBdaRangesBufferByteSize();
            // This buffer could be very large if an application uses many buffers. Allocating it as HOST_CACHED
            // and manually flushing it at the end of the state updates is faster than using HOST_COHERENT.
            alloc_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
            bool success = bda_ranges_snapshot_.Create(&buffer_info, &alloc_info);
            if (!success) {
                return;
            }
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

        if (error_logging_desc_set_layout_ == VK_NULL_HANDLE) {
            VkDescriptorSetLayoutCreateInfo validation_cmd_desc_set_layout_ci = vku::InitStructHelper();
            validation_cmd_desc_set_layout_ci.bindingCount = static_cast<uint32_t>(validation_cmd_bindings.size());
            validation_cmd_desc_set_layout_ci.pBindings = validation_cmd_bindings.data();
            result = DispatchCreateDescriptorSetLayout(gpuav_.device, &validation_cmd_desc_set_layout_ci, nullptr,
                                                       &error_logging_desc_set_layout_);
            if (result != VK_SUCCESS) {
                gpuav_.InternalError(gpuav_.device, loc, "Unable to create descriptor set layout used for validation commands.");
                return;
            }
        }

        assert((validation_cmd_desc_pool_ == VK_NULL_HANDLE) == (error_logging_desc_set_ == VK_NULL_HANDLE));
        if (validation_cmd_desc_pool_ == VK_NULL_HANDLE && error_logging_desc_set_ == VK_NULL_HANDLE) {
            result = gpuav_.desc_set_manager_->GetDescriptorSet(&validation_cmd_desc_pool_, error_logging_desc_set_layout_,
                                                                &error_logging_desc_set_);
            if (result != VK_SUCCESS) {
                gpuav_.InternalError(gpuav_.device, loc, "Unable to create descriptor set used for validation commands.");
                return;
            }
        }

        std::array<VkWriteDescriptorSet, 4> validation_cmd_descriptor_writes = {};
        assert(validation_cmd_bindings.size() == validation_cmd_descriptor_writes.size());

        VkDescriptorBufferInfo error_output_buffer_desc_info = {};

        error_output_buffer_desc_info.buffer = error_output_buffer_range_.buffer;
        error_output_buffer_desc_info.offset = error_output_buffer_range_.offset;
        error_output_buffer_desc_info.range = error_output_buffer_range_.size;

        validation_cmd_descriptor_writes[0] = vku::InitStructHelper();
        validation_cmd_descriptor_writes[0].dstBinding = glsl::kBindingDiagErrorBuffer;
        validation_cmd_descriptor_writes[0].descriptorCount = 1;
        validation_cmd_descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        validation_cmd_descriptor_writes[0].pBufferInfo = &error_output_buffer_desc_info;
        validation_cmd_descriptor_writes[0].dstSet = GetErrorLoggingDescSet();

        VkDescriptorBufferInfo cmd_indices_buffer_desc_info = {};

        assert(!gpuav_.indices_buffer_.IsDestroyed());
        cmd_indices_buffer_desc_info.buffer = gpuav_.indices_buffer_.VkHandle();
        cmd_indices_buffer_desc_info.offset = 0;
        cmd_indices_buffer_desc_info.range = sizeof(uint32_t);

        validation_cmd_descriptor_writes[1] = vku::InitStructHelper();
        validation_cmd_descriptor_writes[1].dstBinding = glsl::kBindingDiagActionIndex;
        validation_cmd_descriptor_writes[1].descriptorCount = 1;
        validation_cmd_descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        validation_cmd_descriptor_writes[1].pBufferInfo = &cmd_indices_buffer_desc_info;
        validation_cmd_descriptor_writes[1].dstSet = GetErrorLoggingDescSet();

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
        validation_cmd_descriptor_writes[3].dstSet = GetErrorLoggingDescSet();

        DispatchUpdateDescriptorSets(gpuav_.device, static_cast<uint32_t>(validation_cmd_descriptor_writes.size()),
                                     validation_cmd_descriptor_writes.data(), 0, NULL);
    }
}

bool CommandBufferSubState::UpdateBdaRangesBuffer(const Location &loc) {
    // By supplying a "date"
    if (!gpuav_.gpuav_settings.shader_instrumentation.buffer_device_address ||
        bda_ranges_snapshot_version_ == gpuav_.device_state->buffer_device_address_ranges_version) {
        return true;
    }

    // Update buffer device address table
    // ---
    auto bda_table_ptr = (uint32_t *)bda_ranges_snapshot_.GetMappedPtr();

    // Buffer device address table layout
    // Ranges are sorted from low to high, and do not overlap
    // QWord 0 | split up into two dwords
    //     DWord 0 | Number of *ranges* (1 range occupies 2 QWords)
    //     DWord 1 | unused
    // QWord 1 | Range 1 begin
    // QWord 2 | Range 1 end
    // QWord 3 | Range 2 begin
    // QWord 4 | Range 2 end
    // QWord 5 | ...

    const size_t max_recordable_ranges =
        static_cast<size_t>((GetBdaRangesBufferByteSize() - sizeof(uint64_t)) / (2 * sizeof(VkDeviceAddress)));
    auto bda_ranges = reinterpret_cast<vvl::DeviceState::BufferAddressRange *>(bda_table_ptr + 2);
    const auto [ranges_to_update_count, total_address_ranges_count] =
        gpuav_.device_state->GetBufferAddressRanges(bda_ranges, max_recordable_ranges);
    // Cast here instead of having to cast inside the shader
    bda_table_ptr[0] = static_cast<uint32_t>(ranges_to_update_count);

    if (total_address_ranges_count > size_t(gpuav_.gpuav_settings.max_bda_in_use)) {
        std::ostringstream problem_string;
        problem_string << "Number of buffer device addresses ranges in use (" << total_address_ranges_count
                       << ") is greater than khronos_validation.gpuav_max_buffer_device_addresses ("
                       << gpuav_.gpuav_settings.max_bda_in_use
                       << "). Truncating buffer device address table could result in invalid validation.";
        gpuav_.InternalError(gpuav_.device, loc, problem_string.str().c_str());
        return false;
    }

    // Post update cleanups
    // ---
    // Flush the BDA buffer before un-mapping so that the new state is visible to the GPU
    bda_ranges_snapshot_.FlushAllocation();
    bda_ranges_snapshot_version_ = gpuav_.device_state->buffer_device_address_ranges_version;

    return true;
}

VkDeviceSize CommandBufferSubState::GetBdaRangesBufferByteSize() const {
    return (1                                           // 2 QWORD for the number of address ranges
            + 2 * gpuav_.gpuav_settings.max_bda_in_use  // 2 QWORDS per address range
            ) *
           8;
}

void CommandBufferSubState::Destroy() { ResetCBState(true); }

void CommandBufferSubState::Reset(const Location &loc) {
    ResetCBState(false);
    // TODO: Calling AllocateResources in Reset like so is a kind of a hack,
    // relying on CommandBuffer internal logic to work.
    // Tried to call it in ResetCBState, hang on command buffer mutex :/
    AllocateResources(loc);
}

void CommandBufferSubState::ResetCBState(bool should_destroy) {
    // Free or return to cache GPU resources

    max_actions_cmd_validation_reached_ = false;

    for (DebugPrintfBufferInfo &printf_buffer_info : debug_printf_buffer_infos) {
        printf_buffer_info.output_mem_buffer.Destroy();
    }
    debug_printf_buffer_infos.clear();

    if (should_destroy) {
        gpu_resources_manager.DestroyResources();
    } else {
        gpu_resources_manager.ReturnResources();
    }
    per_command_error_loggers.clear();

    for (DescriptorBindingCommand &descriptor_binding_cmd : descriptor_binding_commands) {
        descriptor_binding_cmd.descritpor_state_ssbo_buffer.Destroy();
        descriptor_binding_cmd.post_process_ssbo_buffer.Destroy();
    }
    descriptor_binding_commands.clear();
    descriptor_indexing_buffer = VK_NULL_HANDLE;
    post_process_buffer_lut = VK_NULL_HANDLE;

    if (should_destroy) {
        error_output_buffer_range_ = {};
        cmd_errors_counts_buffer_.Destroy();
        bda_ranges_snapshot_.Destroy();
        bda_ranges_snapshot_version_ = 0;
    }

    if (should_destroy && validation_cmd_desc_pool_ != VK_NULL_HANDLE && error_logging_desc_set_ != VK_NULL_HANDLE) {
        gpuav_.desc_set_manager_->PutBackDescriptorSet(validation_cmd_desc_pool_, error_logging_desc_set_);
        validation_cmd_desc_pool_ = VK_NULL_HANDLE;
        error_logging_desc_set_ = VK_NULL_HANDLE;
    }

    if (should_destroy && instrumentation_desc_set_layout_ != VK_NULL_HANDLE) {
        DispatchDestroyDescriptorSetLayout(gpuav_.device, instrumentation_desc_set_layout_, nullptr);
        instrumentation_desc_set_layout_ = VK_NULL_HANDLE;
    }

    if (should_destroy && error_logging_desc_set_layout_ != VK_NULL_HANDLE) {
        DispatchDestroyDescriptorSetLayout(gpuav_.device, error_logging_desc_set_layout_, nullptr);
        error_logging_desc_set_layout_ = VK_NULL_HANDLE;
    }

    draw_index = 0;
    compute_index = 0;
    trace_rays_index = 0;
    action_command_count = 0;
}

void CommandBufferSubState::IncrementCommandCount(Validator &gpuav, VkPipelineBindPoint bind_point, const Location &loc) {
    action_command_count++;
    if (action_command_count >= glsl::kMaxActionsPerCommandBuffer) {
        if (action_command_count == glsl::kMaxActionsPerCommandBuffer) {
            gpuav.LogWarning("GPU-AV::Max action per command buffer reached", VkHandle(), loc,
                             "Reached maximum validation commands count for command buffer ( %" PRIu32
                             " ). No more draw/dispatch/trace rays commands will be validated inside this command buffer.",
                             glsl::kMaxActionsPerCommandBuffer);
        }
        max_actions_cmd_validation_reached_ = true;
    }
    if (bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS) {
        draw_index++;
    } else if (bind_point == VK_PIPELINE_BIND_POINT_COMPUTE) {
        compute_index++;
    } else if (bind_point == VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR) {
        trace_rays_index++;
    }
}

std::string CommandBufferSubState::GetDebugLabelRegion(uint32_t label_command_i,
                                                       const std::vector<std::string> &initial_label_stack) const {
    std::string debug_region_name;
    if (label_command_i != vvl::kU32Max) {
        debug_region_name = base.GetDebugRegionName(base.GetLabelCommands(), label_command_i, initial_label_stack);
    } else {
        // label_command_i == vvl::kU32Max => when the instrumented command was recorded,
        // no debug label region was yet opened in the corresponding command buffer,
        // but still a region might have been started in another previously submitted
        // command buffer. So just compute region name from initial_label_stack.
        for (const std::string &label_name : initial_label_stack) {
            if (!debug_region_name.empty()) {
                debug_region_name += "::";
            }
            debug_region_name += label_name;
        }
    }
    return debug_region_name;
}

bool CommandBufferSubState::PreProcess(const Location &loc) {
    bool succeeded = descriptor::UpdateDescriptorStateSSBO(gpuav_, *this, loc);
    if (!succeeded) {
        return false;
    }

    succeeded = UpdateBdaRangesBuffer(loc);
    if (!succeeded) {
        return false;
    }

    return true;
}

bool CommandBufferSubState::NeedsPostProcess() { return error_output_buffer_range_.buffer != VK_NULL_HANDLE; }

// For the given command buffer, map its debug data buffers and read their contents for analysis.
void CommandBufferSubState::PostProcess(VkQueue queue, const std::vector<std::string> &initial_label_stack, const Location &loc) {
    VVL_ZoneScoped;

    // For the given command buffer, map its debug data buffers and read their contents for analysis.
    for (DebugPrintfBufferInfo &printf_buffer_info : debug_printf_buffer_infos) {
        auto printf_output_ptr = (char *)printf_buffer_info.output_mem_buffer.GetMappedPtr();
        debug_printf::AnalyzeAndGenerateMessage(gpuav_, VkHandle(), queue, printf_buffer_info, (uint32_t *)printf_output_ptr, loc);
    }

    // CommandBuffer::Destroy can happen on an other thread,
    // so when getting here after acquiring command buffer's lock,
    // make sure there are still things to process
    if (!NeedsPostProcess()) {
        return;
    }

    bool skip = false;
    {
        auto error_output_buffer_ptr = (uint32_t *)error_output_buffer_range_.offset_mapped_ptr;

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
                const uint32_t error_logger_i = error_record_ptr[glsl::kHeaderActionIdOffset] & glsl::kCommandResourceIdMask;
                assert(error_logger_i < per_command_error_loggers.size());
                auto &error_logger = per_command_error_loggers[error_logger_i];
                const LogObjectList objlist(queue, VkHandle());
                skip |= error_logger(error_record_ptr, objlist, initial_label_stack);

                // Next record
                error_record_ptr += record_size;
                record_size = error_record_ptr[glsl::kHeaderErrorRecordSizeOffset];
            }

            VVL_TracyPlot("GPU-AV errors count", int64_t(total_words / glsl::kErrorRecordSize));

            // Clear the written size and any error messages. Note that this preserves the first word, which contains flags.
            assert(glsl::kErrorBufferByteSize > cst::stream_output_data_offset);
            memset(&error_output_buffer_ptr[cst::stream_output_data_offset], 0,
                   glsl::kErrorBufferByteSize - cst::stream_output_data_offset * sizeof(uint32_t));
        }
        error_output_buffer_ptr[cst::stream_output_size_offset] = 0;
    }

    cmd_errors_counts_buffer_.Clear();
    if (gpuav_.aborted_) return;

    // If instrumentation found an error, skip post processing. Errors detected by instrumentation are usually
    // very serious, such as a prematurely destroyed resource and the state needed below is likely invalid.
    bool gpuav_success = false;
    if (!skip && gpuav_.gpuav_settings.shader_instrumentation.post_process_descriptor_indexing) {
        LabelLogging label_logging = {initial_label_stack, action_cmd_i_to_label_cmd_i_map};
        gpuav_success = ValidateBindlessDescriptorSets(loc, label_logging);
    }

    if (gpuav_success) {
        UpdateCmdBufImageLayouts(gpuav_, base);
    }
}

QueueSubState::QueueSubState(Validator &gpuav, vvl::Queue &q) : vvl::QueueSubState(q), gpuav_(gpuav), timeline_khr_(false) {}

QueueSubState::~QueueSubState() {
    if (barrier_command_buffer_) {
        DispatchFreeCommandBuffers(gpuav_.device, barrier_command_pool_, 1, &barrier_command_buffer_);
        barrier_command_buffer_ = VK_NULL_HANDLE;
    }
    if (barrier_command_pool_) {
        DispatchDestroyCommandPool(gpuav_.device, barrier_command_pool_, nullptr);
        barrier_command_pool_ = VK_NULL_HANDLE;
    }
    if (barrier_sem_) {
        DispatchDestroySemaphore(gpuav_.device, barrier_sem_, nullptr);
        barrier_sem_ = VK_NULL_HANDLE;
    }
}

// Submit a memory barrier on graphics queues.
// Lazy-create and record the needed command buffer.
void QueueSubState::SubmitBarrier(const Location &loc, uint64_t seq) {
    if (barrier_command_pool_ == VK_NULL_HANDLE) {
        VkResult result = VK_SUCCESS;

        VkCommandPoolCreateInfo pool_create_info = vku::InitStructHelper();
        pool_create_info.queueFamilyIndex = base.queue_family_index;
        result = DispatchCreateCommandPool(gpuav_.device, &pool_create_info, nullptr, &barrier_command_pool_);
        if (result != VK_SUCCESS) {
            gpuav_.InternalError(VkHandle(), loc, "Unable to create command pool for barrier CB.");
            barrier_command_pool_ = VK_NULL_HANDLE;
            return;
        }

        VkCommandBufferAllocateInfo buffer_alloc_info = vku::InitStructHelper();
        buffer_alloc_info.commandPool = barrier_command_pool_;
        buffer_alloc_info.commandBufferCount = 1;
        buffer_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        result = DispatchAllocateCommandBuffers(gpuav_.device, &buffer_alloc_info, &barrier_command_buffer_);
        if (result != VK_SUCCESS) {
            gpuav_.InternalError(VkHandle(), loc, "Unable to create barrier command buffer.");
            DispatchDestroyCommandPool(gpuav_.device, barrier_command_pool_, nullptr);
            barrier_command_pool_ = VK_NULL_HANDLE;
            barrier_command_buffer_ = VK_NULL_HANDLE;
            return;
        }

        VkSemaphoreTypeCreateInfo semaphore_type_create_info = vku::InitStructHelper();
        semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
        semaphore_type_create_info.initialValue = 0;

        VkSemaphoreCreateInfo semaphore_create_info = vku::InitStructHelper(&semaphore_type_create_info);

        result = DispatchCreateSemaphore(gpuav_.device, &semaphore_create_info, nullptr, &barrier_sem_);
        if (result != VK_SUCCESS) {
            gpuav_.InternalError(gpuav_.device, loc, "Unable to create barrier semaphore.");
            DispatchDestroyCommandPool(gpuav_.device, barrier_command_pool_, nullptr);
            barrier_command_pool_ = VK_NULL_HANDLE;
            barrier_command_buffer_ = VK_NULL_HANDLE;
            return;
        }

        // Hook up command buffer dispatch
        gpuav_.vk_set_device_loader_data_(gpuav_.device, barrier_command_buffer_);

        // Record a global memory barrier to force availability of device memory operations to the host domain.
        VkCommandBufferBeginInfo barrier_cmd_buffer_begin_info = vku::InitStructHelper();
        barrier_cmd_buffer_begin_info.flags |= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        result = DispatchBeginCommandBuffer(barrier_command_buffer_, &barrier_cmd_buffer_begin_info);
        if (result == VK_SUCCESS) {
            VkMemoryBarrier memory_barrier = vku::InitStructHelper();
            memory_barrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
            memory_barrier.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
            DispatchCmdPipelineBarrier(barrier_command_buffer_, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_HOST_BIT, 0,
                                       1, &memory_barrier, 0, nullptr, 0, nullptr);
            DispatchEndCommandBuffer(barrier_command_buffer_);
        }
    }

    if (barrier_command_buffer_ != VK_NULL_HANDLE) {
        VkTimelineSemaphoreSubmitInfo timeline_semaphore_submit_info = vku::InitStructHelper();
        timeline_semaphore_submit_info.signalSemaphoreValueCount = 1;
        timeline_semaphore_submit_info.pSignalSemaphoreValues = &seq;

        VkSubmitInfo submit_info = vku::InitStructHelper(&timeline_semaphore_submit_info);

        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &barrier_command_buffer_;
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &barrier_sem_;

        DispatchQueueSubmit(VkHandle(), 1, &submit_info, VK_NULL_HANDLE);
    }
}

void QueueSubState::PreSubmit(std::vector<vvl::QueueSubmission> &submissions) {
    bool success = true;
    for (const auto &submission : submissions) {
        auto loc = submission.loc.Get();
        for (auto &cb_submission : submission.cb_submissions) {
            auto guard = cb_submission.cb->ReadLock();
            auto &gpu_cb = SubState(*cb_submission.cb);
            success = gpu_cb.PreProcess(loc);
            if (!success) {
                return;
            }
            for (auto *secondary_cb : gpu_cb.base.linked_command_buffers) {
                auto secondary_guard = secondary_cb->ReadLock();
                auto &secondary_gpu_cb = SubState(*secondary_cb);
                success = secondary_gpu_cb.PreProcess(loc);
                if (!success) {
                    return;
                }
            }
        }
    }
}

void QueueSubState::PostSubmit(vvl::QueueSubmission &submission) {
    if (submission.is_last_submission) {
        auto loc = submission.loc.Get();
        SubmitBarrier(loc, submission.seq);
    }
}

void QueueSubState::Retire(vvl::QueueSubmission &submission) {
    if (submission.loc.Get().function == vvl::Func::vkQueuePresentKHR) {
        // Present batch does not have any GPU-AV work to post process, skip it.
        // This is also needed for correctness. QueuePresent does not have a PostSubmit call
        // that signals barrier_sem_. The following timeline wait must not be called.
        return;
    }
    retiring_.emplace_back(submission.cb_submissions);
    if (submission.is_last_submission) {
        VkSemaphoreWaitInfo wait_info = vku::InitStructHelper();
        wait_info.semaphoreCount = 1;
        wait_info.pSemaphores = &barrier_sem_;
        wait_info.pValues = &submission.seq;

        if (timeline_khr_) {
            DispatchWaitSemaphoresKHR(gpuav_.device, &wait_info, 1'000'000'000);
        } else {
            DispatchWaitSemaphores(gpuav_.device, &wait_info, 1'000'000'000);
        }

        for (std::vector<vvl::CommandBufferSubmission> &cb_submissions : retiring_) {
            for (vvl::CommandBufferSubmission &cb_submission : cb_submissions) {
                auto guard = cb_submission.cb->WriteLock();
                auto &gpu_cb = SubState(*cb_submission.cb);
                auto loc = submission.loc.Get();
                gpu_cb.PostProcess(VkHandle(), cb_submission.initial_label_stack, loc);
                for (vvl::CommandBuffer *secondary_cb : gpu_cb.base.linked_command_buffers) {
                    auto secondary_guard = secondary_cb->WriteLock();
                    auto &secondary_gpu_cb = SubState(*secondary_cb);
                    secondary_gpu_cb.PostProcess(VkHandle(), cb_submission.initial_label_stack, loc);
                }
            }
        }
        retiring_.clear();
    }
}

ImageSubState::ImageSubState(vvl::Image &obj, DescriptorHeap &heap)
    : vvl::ImageSubState(obj), id_tracker(std::in_place, heap, obj.Handle()) {}

void ImageSubState::Destroy() { id_tracker.reset(); }

void ImageSubState::NotifyInvalidate(const vvl::StateObject::NodeList &invalid_nodes, bool unlink) { id_tracker.reset(); }

ImageViewSubState::ImageViewSubState(vvl::ImageView &obj, DescriptorHeap &heap)
    : vvl::ImageViewSubState(obj), id_tracker(std::in_place, heap, obj.Handle()) {}

void ImageViewSubState::Destroy() { id_tracker.reset(); }

void ImageViewSubState::NotifyInvalidate(const vvl::StateObject::NodeList &invalid_nodes, bool unlink) { id_tracker.reset(); }

BufferSubState::BufferSubState(vvl::Buffer &obj, DescriptorHeap &heap)
    : vvl::BufferSubState(obj), id_tracker(std::in_place, heap, obj.Handle()) {}

void BufferSubState::Destroy() { id_tracker.reset(); }

void BufferSubState::NotifyInvalidate(const vvl::StateObject::NodeList &invalid_nodes, bool unlink) { id_tracker.reset(); }

BufferViewSubState::BufferViewSubState(vvl::BufferView &obj, DescriptorHeap &heap)
    : vvl::BufferViewSubState(obj), id_tracker(std::in_place, heap, obj.Handle()) {}

void BufferViewSubState::Destroy() { id_tracker.reset(); }

void BufferViewSubState::NotifyInvalidate(const vvl::StateObject::NodeList &invalid_nodes, bool unlink) { id_tracker.reset(); }

SamplerSubState::SamplerSubState(vvl::Sampler &obj, DescriptorHeap &heap)
    : vvl::SamplerSubState(obj), id_tracker(std::in_place, heap, obj.Handle()) {}

void SamplerSubState::Destroy() { id_tracker.reset(); }

void SamplerSubState::NotifyInvalidate(const vvl::StateObject::NodeList &invalid_nodes, bool unlink) { id_tracker.reset(); }

AccelerationStructureNVSubState::AccelerationStructureNVSubState(vvl::AccelerationStructureNV &obj, DescriptorHeap &heap)
    : vvl::AccelerationStructureNVSubState(obj), id_tracker(std::in_place, heap, obj.Handle()) {}

void AccelerationStructureNVSubState::Destroy() { id_tracker.reset(); }

void AccelerationStructureNVSubState::NotifyInvalidate(const vvl::StateObject::NodeList &invalid_nodes, bool unlink) {
    id_tracker.reset();
}

AccelerationStructureKHRSubState::AccelerationStructureKHRSubState(vvl::AccelerationStructureKHR &obj, DescriptorHeap &heap)
    : vvl::AccelerationStructureKHRSubState(obj), id_tracker(std::in_place, heap, obj.Handle()) {}

void AccelerationStructureKHRSubState::Destroy() { id_tracker.reset(); }

void AccelerationStructureKHRSubState::NotifyInvalidate(const vvl::StateObject::NodeList &invalid_nodes, bool unlink) {
    id_tracker.reset();
}

ShaderObjectSubState::ShaderObjectSubState(vvl::ShaderObject &obj) : vvl::ShaderObjectSubState(obj) {}

}  // namespace gpuav
