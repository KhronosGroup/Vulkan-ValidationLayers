/* Copyright (c) 2026 The Khronos Group Inc.
 * Copyright (c) 2026 Valve Corporation
 * Copyright (c) 2026 LunarG, Inc.
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

#include "sync_val_tests.h"

class NegativeSyncValDeferred : public VkSyncValTest {};
class PositiveSyncValDeferred : public VkSyncValTest {};

TEST_F(NegativeSyncValDeferred, BasicDeferredFullValidation) {
    TEST_DESCRIPTION("Report a hazard at the fence wait instead of during recording or submission");
    RETURN_IF_SKIP(InitDeferredSyncVal());

    vkt::Buffer buffer(*m_device, 4, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    vkt::Fence fence(*m_device);

    m_command_buffer.Begin();
    vk::CmdFillBuffer(m_command_buffer, buffer, 0, 4, 0);
    vk::CmdFillBuffer(m_command_buffer, buffer, 0, 4, 0);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer, fence);

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    fence.Wait(kWaitTimeout);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSyncValDeferred, DeferredFullValidationAfterError) {
    TEST_DESCRIPTION("Check that state tracking continues after a deferred validation error");
    RETURN_IF_SKIP(InitDeferredSyncVal());

    vkt::Buffer buffer_a(*m_device, 4, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    vkt::Buffer buffer_b(*m_device, 4, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    vkt::Buffer buffer_c(*m_device, 4, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    vkt::CommandBuffer cb0(*m_device, m_command_pool);
    vkt::CommandBuffer cb1(*m_device, m_command_pool);
    vkt::CommandBuffer cb2(*m_device, m_command_pool);
    vkt::Fence fence(*m_device);

    cb0.Begin();
    vk::CmdFillBuffer(cb0, buffer_a, 0, 4, 0);
    cb0.End();

    cb1.Begin();
    vk::CmdFillBuffer(cb1, buffer_a, 0, 4, 0);
    vk::CmdFillBuffer(cb1, buffer_b, 0, 4, 0);
    cb1.End();

    cb2.Begin();
    cb2.Copy(buffer_b, buffer_c);
    cb2.End();

    m_default_queue->Submit(cb0);
    m_default_queue->Submit(cb1);
    m_default_queue->Submit(cb2, fence);

    // The second report requires keeping cb1's write to buffer_b despite its error on buffer_a
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-READ-AFTER-WRITE");
    fence.Wait(kWaitTimeout);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSyncValDeferred, MultiBatchSecondaryReplay) {
    TEST_DESCRIPTION("Detect a hazard between two batches executing the same secondary command buffer");
    RETURN_IF_SKIP(InitDeferredSyncVal());

    vkt::Buffer buffer_a(*m_device, 4, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    vkt::CommandBuffer cb0(*m_device, m_command_pool);
    vkt::CommandBuffer cb1(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    vkt::Fence fence(*m_device);

    cb1.Begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
    vk::CmdFillBuffer(cb1, buffer_a, 0, 4, 0);
    cb1.End();

    cb0.Begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
    cb0.ExecuteCommands(cb1);
    cb0.End();

    VkSubmitInfo submits[2] = {vku::InitStructHelper(), vku::InitStructHelper()};
    submits[0].commandBufferCount = 1;
    submits[0].pCommandBuffers = &cb0.handle();
    submits[1].commandBufferCount = 1;
    submits[1].pCommandBuffers = &cb0.handle();
    vk::QueueSubmit(*m_default_queue, 2, submits, fence);

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    fence.Wait(kWaitTimeout);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSyncValDeferred, ValidateAfterCommandBufferReset) {
    TEST_DESCRIPTION("Validate submitted commands after command buffer reset and destruction");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    RETURN_IF_SKIP(InitDeferredSyncVal());

    if (!m_second_queue) {
        GTEST_SKIP() << "Two queues are needed";
    }
    vkt::Queue* queue0 = m_default_queue;
    vkt::Queue* queue1 = m_second_queue;
    vkt::CommandBuffer cb1(*m_device, m_second_command_pool);

    vkt::Semaphore semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);
    vkt::Fence fence0(*m_device);
    vkt::Fence fence1(*m_device);

    vkt::Buffer buffer_a(*m_device, 4, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    cb1.Begin();
    vk::CmdFillBuffer(cb1, buffer_a, 0, 4, 0);
    vk::CmdFillBuffer(cb1, buffer_a, 0, 4, 0);
    cb1.End();

    // The first queue keeps retire-time replay blocked
    queue0->Submit2(vkt::no_cmd, vkt::TimelineWait(semaphore, 1), fence0);

    queue1->Submit2(cb1, fence1);
    fence1.Wait(kWaitTimeout);

    // The original recording must survive both replacement and destruction of its command buffer
    cb1.Reset();
    cb1.Begin();
    cb1.End();
    cb1.Destroy();

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    semaphore.Signal(1);
    fence0.Wait(kWaitTimeout);
    m_errorMonitor->VerifyFound();
    m_device->Wait();
}

TEST_F(PositiveSyncValDeferred, FenceWaitReplay) {
    TEST_DESCRIPTION("Preserve fence synchronization during deferred replay");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    RETURN_IF_SKIP(InitDeferredSyncVal());

    if (!m_second_queue) {
        GTEST_SKIP() << "Two queues are needed";
    }
    vkt::Semaphore semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);
    vkt::Fence fence(*m_device);
    vkt::Buffer buffer(*m_device, 4, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    m_second_command_buffer.Begin();
    vk::CmdFillBuffer(m_second_command_buffer, buffer, 0, 4, 0);
    m_second_command_buffer.End();

    // Keep retire-time replay blocked when the host waits on the fence
    m_default_queue->Submit2(vkt::no_cmd, vkt::TimelineWait(semaphore, 1));

    m_second_queue->Submit2(m_second_command_buffer, fence);

    // Can't be replayed immediately, blocked by submit. Adds fence replay entry
    fence.Wait(kWaitTimeout);

    m_second_queue->Submit2(m_second_command_buffer);
    semaphore.Signal(1);
    m_device->Wait();
}

TEST_F(NegativeSyncValDeferred, ValidateAfterBufferDestruction) {
    TEST_DESCRIPTION("Validate submitted commands after buffer destruction");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    RETURN_IF_SKIP(InitDeferredSyncVal());

    if (!m_second_queue) {
        GTEST_SKIP() << "Two queues are needed";
    }
    vkt::Semaphore semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);
    vkt::Fence fence(*m_device);
    vkt::Buffer buffer(*m_device, 4, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    m_second_command_buffer.Begin();
    vk::CmdFillBuffer(m_second_command_buffer, buffer, 0, 4, 0);
    vk::CmdFillBuffer(m_second_command_buffer, buffer, 0, 4, 0);
    m_second_command_buffer.End();

    // Keep retire-time replay blocked until the buffer is destroyed
    m_default_queue->Submit2(vkt::no_cmd, vkt::TimelineWait(semaphore, 1));

    m_second_queue->Submit2(m_second_command_buffer, fence);
    fence.Wait(kWaitTimeout);
    buffer.Destroy();

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    semaphore.Signal(1);
    m_device->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSyncValDeferred, WaitBeforeSignalReplay) {
    TEST_DESCRIPTION("Check deferred replay with existing wait-before-signal handling");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    RETURN_IF_SKIP(InitDeferredSyncVal());

    vkt::Buffer buffer(*m_device, 4, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    vkt::Semaphore semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);
    vkt::Fence fence(*m_device);

    m_command_buffer.Begin();
    vk::CmdFillBuffer(m_command_buffer, buffer, 0, 4, 0);
    vk::CmdFillBuffer(m_command_buffer, buffer, 0, 4, 0);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer, vkt::TimelineWait(semaphore, 1), fence);
    semaphore.Signal(1);

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    fence.Wait(kWaitTimeout);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSyncValDeferred, ReachQueueCommandLimit) {
    TEST_DESCRIPTION("The 512th queued command forces deferred validation to make progress");
    RETURN_IF_SKIP(InitDeferredSyncVal());

    vkt::Buffer buffer_a(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    m_command_buffer.Begin();
    vk::CmdFillBuffer(m_command_buffer, buffer_a, 0, 256, 0);
    vk::CmdFillBuffer(m_command_buffer, buffer_a, 0, 256, 1);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    for (uint32_t i = 0; i < 510; ++i) {
        m_default_queue->Submit(vkt::no_cmd);
    }
    // 512th entry forces drain of the enqueued commands
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    m_default_queue->Submit(vkt::no_cmd);
    m_errorMonitor->VerifyFound();
    m_default_queue->Wait();
}

TEST_F(NegativeSyncValDeferred, RepeatedTimelinePolling) {
    TEST_DESCRIPTION("Repeated timeline polling must not fill the command queue");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    RETURN_IF_SKIP(InitDeferredSyncVal());
    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test requires the semaphore counter value from the driver";
    }

    vkt::Buffer buffer_a(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    vkt::Semaphore semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);
    vkt::Fence fence(*m_device);
    m_command_buffer.Begin();
    vk::CmdFillBuffer(m_command_buffer, buffer_a, 0, VK_WHOLE_SIZE, 0);
    vk::CmdFillBuffer(m_command_buffer, buffer_a, 0, VK_WHOLE_SIZE, 1);
    m_command_buffer.End();

    semaphore.Signal(1);
    m_default_queue->Submit(m_command_buffer, fence);
    // Check that implementation skips repeated values so these polls do not
    // feel the command queue, and so do not force validation
    for (uint32_t i = 0; i < 513; ++i) {
        semaphore.GetCounterValue();
    }

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    fence.Wait(kWaitTimeout);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSyncValDeferred, ValidateAfterSwapchainDestruction) {
    TEST_DESCRIPTION("Validate acquired image accesses after swapchain destruction");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddSurfaceExtension();
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitDeferredSyncVal());
    if (!m_second_queue) {
        GTEST_SKIP() << "Two queues are needed";
    }
    RETURN_IF_SKIP(InitSwapchain());

    const auto swapchain_images = m_swapchain.GetImages();
    vkt::Semaphore semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);
    vkt::Semaphore acquire_semaphore(*m_device);
    vkt::Fence fence(*m_device);

    // Keep retire-time replay blocked until the swapchain is destroyed
    m_second_queue->Submit2(vkt::no_cmd, vkt::TimelineWait(semaphore, 1));

    const uint32_t image_index = m_swapchain.AcquireNextImage(acquire_semaphore, kWaitTimeout);

    // The layout transition writes the image without synchronizing with the acquire read.
    // Its empty source scope does not connect it to the semaphore wait
    VkImageMemoryBarrier2 layout_transition = vku::InitStructHelper();
    layout_transition.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    layout_transition.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    layout_transition.image = swapchain_images[image_index];
    layout_transition.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    m_command_buffer.Begin();
    m_command_buffer.Barrier(layout_transition);
    m_command_buffer.End();
    m_default_queue->Submit2(m_command_buffer, vkt::Wait(acquire_semaphore, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT),
                             fence);
    fence.Wait(kWaitTimeout);
    m_swapchain.Destroy();

    semaphore.Signal(1);
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-READ");
    m_device->Wait();
    m_errorMonitor->VerifyFound();
}
