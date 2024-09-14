/* Copyright (c) 2020-2024 The Khronos Group Inc.
 * Copyright (c) 2020-2024 Valve Corporation
 * Copyright (c) 2020-2024 LunarG, Inc.
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

#include "gpu/core/gpu_state_tracker.h"
#include "gpu/instrumentation/gpu_shader_instrumentor.h"

namespace gpu_tracker {

CommandBuffer::CommandBuffer(gpu::GpuShaderInstrumentor &shader_instrumentor, VkCommandBuffer handle,
                             const VkCommandBufferAllocateInfo *pCreateInfo, const vvl::CommandPool *pool)
    : vvl::CommandBuffer(shader_instrumentor, handle, pCreateInfo, pool) {}

Queue::Queue(gpu::GpuShaderInstrumentor &shader_instrumentor, VkQueue q, uint32_t family_index, uint32_t queue_index,
             VkDeviceQueueCreateFlags flags, const VkQueueFamilyProperties &queueFamilyProperties, bool timeline_khr)
    : vvl::Queue(shader_instrumentor, q, family_index, queue_index, flags, queueFamilyProperties),
      shader_instrumentor_(shader_instrumentor),
      timeline_khr_(timeline_khr) {}

Queue::~Queue() {
    if (barrier_command_buffer_) {
        DispatchFreeCommandBuffers(shader_instrumentor_.device, barrier_command_pool_, 1, &barrier_command_buffer_);
        barrier_command_buffer_ = VK_NULL_HANDLE;
    }
    if (barrier_command_pool_) {
        DispatchDestroyCommandPool(shader_instrumentor_.device, barrier_command_pool_, nullptr);
        barrier_command_pool_ = VK_NULL_HANDLE;
    }
    if (barrier_sem_) {
        DispatchDestroySemaphore(shader_instrumentor_.device, barrier_sem_, nullptr);
        barrier_sem_ = VK_NULL_HANDLE;
    }
}

// Submit a memory barrier on graphics queues.
// Lazy-create and record the needed command buffer.
void Queue::SubmitBarrier(const Location &loc, uint64_t seq) {
    if (barrier_command_pool_ == VK_NULL_HANDLE) {
        VkResult result = VK_SUCCESS;

        VkCommandPoolCreateInfo pool_create_info = vku::InitStructHelper();
        pool_create_info.queueFamilyIndex = queue_family_index;
        result = DispatchCreateCommandPool(shader_instrumentor_.device, &pool_create_info, nullptr, &barrier_command_pool_);
        if (result != VK_SUCCESS) {
            shader_instrumentor_.InternalError(vvl::Queue::VkHandle(), loc, "Unable to create command pool for barrier CB.");
            barrier_command_pool_ = VK_NULL_HANDLE;
            return;
        }

        VkCommandBufferAllocateInfo buffer_alloc_info = vku::InitStructHelper();
        buffer_alloc_info.commandPool = barrier_command_pool_;
        buffer_alloc_info.commandBufferCount = 1;
        buffer_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        result = DispatchAllocateCommandBuffers(shader_instrumentor_.device, &buffer_alloc_info, &barrier_command_buffer_);
        if (result != VK_SUCCESS) {
            shader_instrumentor_.InternalError(vvl::Queue::VkHandle(), loc, "Unable to create barrier command buffer.");
            DispatchDestroyCommandPool(shader_instrumentor_.device, barrier_command_pool_, nullptr);
            barrier_command_pool_ = VK_NULL_HANDLE;
            barrier_command_buffer_ = VK_NULL_HANDLE;
            return;
        }

        VkSemaphoreTypeCreateInfoKHR semaphore_type_create_info = vku::InitStructHelper();
        semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE_KHR;
        semaphore_type_create_info.initialValue = 0;

        VkSemaphoreCreateInfo semaphore_create_info = vku::InitStructHelper(&semaphore_type_create_info);

        result = DispatchCreateSemaphore(shader_instrumentor_.device, &semaphore_create_info, nullptr, &barrier_sem_);
        if (result != VK_SUCCESS) {
            shader_instrumentor_.InternalError(shader_instrumentor_.device, loc, "Unable to create barrier semaphore.");
            DispatchDestroyCommandPool(shader_instrumentor_.device, barrier_command_pool_, nullptr);
            barrier_command_pool_ = VK_NULL_HANDLE;
            barrier_command_buffer_ = VK_NULL_HANDLE;
            return;
        }

        // Hook up command buffer dispatch
        shader_instrumentor_.vk_set_device_loader_data_(shader_instrumentor_.device, barrier_command_buffer_);

        // Record a global memory barrier to force availability of device memory operations to the host domain.
        VkCommandBufferBeginInfo barrier_cmd_buffer_begin_info = vku::InitStructHelper();
        barrier_cmd_buffer_begin_info.flags |= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        result = DispatchBeginCommandBuffer(barrier_command_buffer_, &barrier_cmd_buffer_begin_info, false);
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
        VkTimelineSemaphoreSubmitInfoKHR timeline_semaphore_submit_info = vku::InitStructHelper();
        timeline_semaphore_submit_info.signalSemaphoreValueCount = 1;
        timeline_semaphore_submit_info.pSignalSemaphoreValues = &seq;

        VkSubmitInfo submit_info = vku::InitStructHelper(&timeline_semaphore_submit_info);

        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &barrier_command_buffer_;
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &barrier_sem_;

        DispatchQueueSubmit(vvl::Queue::VkHandle(), 1, &submit_info, VK_NULL_HANDLE);
    }
}

vvl::PreSubmitResult Queue::PreSubmit(std::vector<vvl::QueueSubmission> &&submissions) {
    for (const auto &submission : submissions) {
        auto loc = submission.loc.Get();
        for (auto &cb : submission.cbs) {
            auto gpu_cb = std::static_pointer_cast<CommandBuffer>(cb);
            auto guard = gpu_cb->ReadLock();
            gpu_cb->PreProcess(loc);
            for (auto *secondary_cb : gpu_cb->linkedCommandBuffers) {
                auto secondary_guard = secondary_cb->ReadLock();
                auto *secondary_gpu_cb = static_cast<CommandBuffer *>(secondary_cb);
                secondary_gpu_cb->PreProcess(loc);
            }
        }
    }
    return vvl::Queue::PreSubmit(std::move(submissions));
}

void Queue::PostSubmit(vvl::QueueSubmission &submission) {
    vvl::Queue::PostSubmit(submission);
    if (submission.end_batch) {
        auto loc = submission.loc.Get();
        SubmitBarrier(loc, submission.seq);
    }
}

void Queue::Retire(vvl::QueueSubmission &submission) {
    vvl::Queue::Retire(submission);
    retiring_.emplace_back(submission.cbs);
    if (submission.end_batch) {
        VkSemaphoreWaitInfo wait_info = vku::InitStructHelper();
        wait_info.semaphoreCount = 1;
        wait_info.pSemaphores = &barrier_sem_;
        wait_info.pValues = &submission.seq;

        if (timeline_khr_) {
            DispatchWaitSemaphoresKHR(shader_instrumentor_.device, &wait_info, 1000000000);
        } else {
            DispatchWaitSemaphores(shader_instrumentor_.device, &wait_info, 1000000000);
        }

        for (auto &cbs : retiring_) {
            for (auto &cb : cbs) {
                auto gpu_cb = std::static_pointer_cast<CommandBuffer>(cb);
                auto guard = gpu_cb->WriteLock();
                auto loc = submission.loc.Get();
                gpu_cb->PostProcess(VkHandle(), loc);
                for (auto *secondary_cb : gpu_cb->linkedCommandBuffers) {
                    auto *secondary_gpu_cb = static_cast<CommandBuffer *>(secondary_cb);
                    auto secondary_guard = secondary_gpu_cb->WriteLock();
                    secondary_gpu_cb->PostProcess(VkHandle(), loc);
                }
            }
        }
        retiring_.clear();
    }
}
}  // namespace gpu_tracker
