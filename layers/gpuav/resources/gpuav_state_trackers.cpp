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

#include "gpuav/shaders/gpuav_shaders_constants.h"
#include "gpuav/core/gpuav.h"
#include "gpuav/core/gpuav_constants.h"
#include "gpuav/descriptor_validation/gpuav_descriptor_validation.h"
#include "gpuav/shaders/gpuav_error_header.h"
#include "gpuav/debug_printf/debug_printf.h"
#include "containers/limits.h"

#include "profiling/profiling.h"

namespace gpuav {

CommandBufferSubState::CommandBufferSubState(Validator &gpuav, vvl::CommandBuffer &cb)
    : vvl::CommandBufferSubState(cb), gpu_resources_manager(gpuav), cmd_errors_counts_buffer_(gpuav), gpuav_(gpuav) {
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
        error_output_buffer_range_ = gpu_resources_manager.GetHostVisibleBufferRange(glsl::kErrorBufferByteSize);
        if (error_output_buffer_range_.buffer == VK_NULL_HANDLE) {
            return;
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
    }
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

    on_instrumentation_desc_set_update_functions.clear();
    on_cb_completion_functions.clear();
    on_pre_cb_submission_functions.clear();
    shared_resources_cache.Clear();

    if (should_destroy) {
        gpu_resources_manager.DestroyResources();
    } else {
        gpu_resources_manager.ReturnResources();
    }
    per_command_error_loggers.clear();

    if (should_destroy && instrumentation_desc_set_layout_ != VK_NULL_HANDLE) {
        DispatchDestroyDescriptorSetLayout(gpuav_.device, instrumentation_desc_set_layout_, nullptr);
        instrumentation_desc_set_layout_ = VK_NULL_HANDLE;
    }

    if (should_destroy) {
        error_output_buffer_range_ = {};
        cmd_errors_counts_buffer_.Destroy();
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

bool CommandBufferSubState::PreSubmit(QueueSubState &queue, const Location &loc) {
    if (!on_pre_cb_submission_functions.empty()) {
        vko::CommandPool &cb_pool =
            queue.shared_resources_cache.GetOrCreate<vko::CommandPool>(gpuav_, queue.base.queue_family_index, loc);
        auto [per_submission_cb, fence] = cb_pool.GetCommandBuffer(loc);
        if (per_submission_cb == VK_NULL_HANDLE) {
            return false;
        }
        VkCommandBufferBeginInfo cb_bi = vku::InitStructHelper();
        cb_bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        DispatchBeginCommandBuffer(per_submission_cb, &cb_bi);
        for (auto &pre_submission_func : on_pre_cb_submission_functions) {
            pre_submission_func(gpuav_, *this, per_submission_cb);
        }
        DispatchEndCommandBuffer(per_submission_cb);

        VkSubmitInfo submit_info = vku::InitStructHelper();
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &per_submission_cb;
        const VkResult result = DispatchQueueSubmit(queue.base.VkHandle(), 1, &submit_info, fence);
        if (result != VK_SUCCESS) {
            gpuav_.InternalError(queue.Handle(), loc, "Failed to submit per submission command buffer");
        }
    }

    return true;
}

bool CommandBufferSubState::NeedsPostProcess() { return error_output_buffer_range_.buffer != VK_NULL_HANDLE; }

// For the given command buffer, map its debug data buffers and read their contents for analysis.
void CommandBufferSubState::OnCompletion(VkQueue queue, const std::vector<std::string> &initial_label_stack, const Location &loc) {
    VVL_ZoneScoped;

    // CommandBuffer::Destroy can happen on an other thread,
    // so when getting here after acquiring command buffer's lock,
    // make sure there are still things to process
    if (!NeedsPostProcess()) {
        return;
    }

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
                error_logger(error_record_ptr, objlist, initial_label_stack);

                // Next record
                error_record_ptr += record_size;
                record_size = error_record_ptr[glsl::kHeaderErrorRecordSizeOffset];
            }

            VVL_TracyPlot("GPU-AV errors count", int64_t(total_words / glsl::kErrorRecordSize));

            // Clear the written size and any error messages. Note that this preserves the first word, which contains flags.
            assert(glsl::kErrorBufferByteSize > cst::stream_output_data_offset);
            memset(&error_output_buffer_ptr[cst::stream_output_flags_offset + 1], 0,
                   size_t(error_output_buffer_range_.size) - sizeof(uint32_t));
        }
        error_output_buffer_ptr[cst::stream_output_size_offset] = 0;
    }

    cmd_errors_counts_buffer_.Clear();
    if (gpuav_.aborted_) {
        return;
    }

    bool success = true;
    LabelLogging label_logging = {initial_label_stack, action_cmd_i_to_label_cmd_i_map};
    for (auto &on_cb_completion_func : on_cb_completion_functions) {
        success = on_cb_completion_func(gpuav_, *this, label_logging, loc);
        if (!success) {
            break;
        }
    }
}

QueueSubState::QueueSubState(Validator &gpuav, vvl::Queue &q) : vvl::QueueSubState(q), gpuav_(gpuav), timeline_khr_(false) {}

QueueSubState::~QueueSubState() {
    shared_resources_cache.Clear();

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
            success = gpu_cb.PreSubmit(*this, loc);
            if (!success) {
                return;
            }
            for (auto *secondary_cb : gpu_cb.base.linked_command_buffers) {
                auto secondary_guard = secondary_cb->ReadLock();
                auto &secondary_gpu_cb = SubState(*secondary_cb);
                success = secondary_gpu_cb.PreSubmit(*this, loc);
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
                gpu_cb.OnCompletion(VkHandle(), cb_submission.initial_label_stack, loc);
                for (vvl::CommandBuffer *secondary_cb : gpu_cb.base.linked_command_buffers) {
                    auto secondary_guard = secondary_cb->WriteLock();
                    auto &secondary_gpu_cb = SubState(*secondary_cb);
                    secondary_gpu_cb.OnCompletion(VkHandle(), cb_submission.initial_label_stack, loc);
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
