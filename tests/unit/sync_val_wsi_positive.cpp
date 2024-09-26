/* Copyright (c) 2024 The Khronos Group Inc.
 * Copyright (c) 2024 Valve Corporation
 * Copyright (c) 2024 LunarG, Inc.
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

#include <thread>
#include "../framework/layer_validation_tests.h"

struct PositiveSyncValWsi : public VkSyncValTest {};

TEST_F(PositiveSyncValWsi, PresentAfterSubmit2AutomaticVisibility) {
    TEST_DESCRIPTION("Waiting on the semaphore makes available image accesses visible to the presentation engine.");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddSurfaceExtension();
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());
    RETURN_IF_SKIP(InitSwapchain());
    vkt::Semaphore acquire_semaphore(*m_device);
    vkt::Semaphore submit_semaphore(*m_device);
    const auto swapchain_images = GetSwapchainImages(m_swapchain);

    uint32_t image_index = 0;
    vk::AcquireNextImageKHR(device(), m_swapchain, kWaitTimeout, acquire_semaphore, VK_NULL_HANDLE, &image_index);

    VkImageMemoryBarrier2 layout_transition = vku::InitStructHelper();
    // this creates execution dependency with submit's wait semaphore, so layout
    // transition does not start before image is acquired.
    layout_transition.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    layout_transition.srcAccessMask = 0;

    // this creates execution dependency with submit's signal operation, so layout
    // transition finishes before presentation starts.
    layout_transition.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;

    // dstAccessMask makes accesses visible only to the device.
    // Also, any writes to swapchain images that are made available, are
    // automatically made visible to the presentation engine reads.
    // This test checks that presentation engine accesses are not reported as hazards.
    layout_transition.dstAccessMask = 0;

    layout_transition.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    layout_transition.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    layout_transition.image = swapchain_images[image_index];
    layout_transition.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    layout_transition.subresourceRange.baseMipLevel = 0;
    layout_transition.subresourceRange.levelCount = 1;
    layout_transition.subresourceRange.baseArrayLayer = 0;
    layout_transition.subresourceRange.layerCount = 1;

    VkDependencyInfoKHR dep_info = vku::InitStructHelper();
    dep_info.imageMemoryBarrierCount = 1;
    dep_info.pImageMemoryBarriers = &layout_transition;

    m_command_buffer.begin();
    vk::CmdPipelineBarrier2(m_command_buffer, &dep_info);
    m_command_buffer.end();

    m_default_queue->Submit2(m_command_buffer, acquire_semaphore, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, submit_semaphore,
                             VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT);

    VkPresentInfoKHR present = vku::InitStructHelper();
    present.waitSemaphoreCount = 1;
    present.pWaitSemaphores = &submit_semaphore.handle();
    present.swapchainCount = 1;
    present.pSwapchains = &m_swapchain;
    present.pImageIndices = &image_index;
    ASSERT_EQ(VK_SUCCESS, vk::QueuePresentKHR(m_default_queue->handle(), &present));
    m_default_queue->Wait();
}

TEST_F(PositiveSyncValWsi, PresentAfterSubmitAutomaticVisibility) {
    TEST_DESCRIPTION("Waiting on the semaphore makes available image accesses visible to the presentation engine.");
    AddSurfaceExtension();
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());
    RETURN_IF_SKIP(InitSwapchain());
    vkt::Semaphore acquire_semaphore(*m_device);
    vkt::Semaphore submit_semaphore(*m_device);
    const auto swapchain_images = GetSwapchainImages(m_swapchain);

    uint32_t image_index = 0;
    vk::AcquireNextImageKHR(device(), m_swapchain, kWaitTimeout, acquire_semaphore, VK_NULL_HANDLE, &image_index);

    VkImageMemoryBarrier layout_transition = vku::InitStructHelper();
    layout_transition.srcAccessMask = 0;

    // dstAccessMask makes accesses visible only to the device.
    // Also, any writes to swapchain images that are made available, are
    // automatically made visible to the presentation engine reads.
    // This test checks that presentation engine accesses are not reported as hazards.
    layout_transition.dstAccessMask = 0;

    layout_transition.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    layout_transition.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    layout_transition.image = swapchain_images[image_index];
    layout_transition.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    layout_transition.subresourceRange.baseMipLevel = 0;
    layout_transition.subresourceRange.levelCount = 1;
    layout_transition.subresourceRange.baseArrayLayer = 0;
    layout_transition.subresourceRange.layerCount = 1;

    m_command_buffer.begin();
    vk::CmdPipelineBarrier(m_command_buffer, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &layout_transition);
    m_command_buffer.end();

    m_default_queue->Submit(m_command_buffer, acquire_semaphore, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, submit_semaphore);

    VkPresentInfoKHR present = vku::InitStructHelper();
    present.waitSemaphoreCount = 1;
    present.pWaitSemaphores = &submit_semaphore.handle();
    present.swapchainCount = 1;
    present.pSwapchains = &m_swapchain;
    present.pImageIndices = &image_index;
    ASSERT_EQ(VK_SUCCESS, vk::QueuePresentKHR(m_default_queue->handle(), &present));
    m_default_queue->Wait();
}

TEST_F(PositiveSyncValWsi, PresentAfterSubmitNoneDstStage) {
    TEST_DESCRIPTION("Test that QueueSubmit's signal semaphore behaves the same way as QueueSubmit2 with ALL_COMMANDS signal.");
    AddSurfaceExtension();
    SetTargetApiVersion(VK_API_VERSION_1_3);
    VkPhysicalDeviceSynchronization2Features sync2_features = vku::InitStructHelper();
    sync2_features.synchronization2 = VK_TRUE;
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState(nullptr, &sync2_features));
    RETURN_IF_SKIP(InitSwapchain());
    vkt::Semaphore acquire_semaphore(*m_device);
    vkt::Semaphore submit_semaphore(*m_device);
    const auto swapchain_images = GetSwapchainImages(m_swapchain);

    uint32_t image_index = 0;
    vk::AcquireNextImageKHR(device(), m_swapchain, kWaitTimeout, acquire_semaphore, VK_NULL_HANDLE, &image_index);

    VkImageMemoryBarrier2 layout_transition = vku::InitStructHelper();
    layout_transition.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    layout_transition.srcAccessMask = 0;
    // Specify NONE as destination stage to detect issues during conversion SubmitInfo -> SubmitInfo2
    layout_transition.dstStageMask = VK_PIPELINE_STAGE_2_NONE;
    layout_transition.dstAccessMask = 0;
    layout_transition.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    layout_transition.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    layout_transition.image = swapchain_images[image_index];
    layout_transition.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    VkDependencyInfoKHR dep_info = vku::InitStructHelper();
    dep_info.imageMemoryBarrierCount = 1;
    dep_info.pImageMemoryBarriers = &layout_transition;

    m_command_buffer.begin();
    vk::CmdPipelineBarrier2(m_command_buffer, &dep_info);
    m_command_buffer.end();

    // The goal of this test is to use QueueSubmit API (not QueueSubmit2) to
    // ensure syncval correctly converts SubmitInfo to SubmitInfo2 with
    // regard to signal semaphore.
    m_default_queue->Submit(m_command_buffer, acquire_semaphore, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, submit_semaphore);

    VkPresentInfoKHR present = vku::InitStructHelper();
    present.waitSemaphoreCount = 1;
    present.pWaitSemaphores = &submit_semaphore.handle();
    present.swapchainCount = 1;
    present.pSwapchains = &m_swapchain;
    present.pImageIndices = &image_index;

    vk::QueuePresentKHR(m_default_queue->handle(), &present);
    m_device->Wait();
}

TEST_F(PositiveSyncValWsi, ThreadedSubmitAndFenceWaitAndPresent) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/7250");
    AddSurfaceExtension();
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());
    RETURN_IF_SKIP(InitSwapchain());

    const auto swapchain_images = GetSwapchainImages(m_swapchain);
    {
        vkt::CommandBuffer cmd(*m_device, m_command_pool);
        cmd.begin();
        for (VkImage image : swapchain_images) {
            VkImageMemoryBarrier transition = vku::InitStructHelper();
            transition.srcAccessMask = 0;
            transition.dstAccessMask = 0;
            transition.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            transition.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            transition.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            transition.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            transition.image = image;
            transition.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            transition.subresourceRange.baseMipLevel = 0;
            transition.subresourceRange.levelCount = 1;
            transition.subresourceRange.baseArrayLayer = 0;
            transition.subresourceRange.layerCount = 1;
            vk::CmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0,
                                   nullptr, 1, &transition);
        }
        cmd.end();
        m_default_queue->Submit(cmd);
        m_default_queue->Wait();
    }

    constexpr int N = 1'000;
    std::mutex queue_mutex;

    // Worker thread submits accesses and waits on the fence.
    std::thread thread([&] {
        const int size = 1024 * 128;
        vkt::Buffer src(*m_device, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
        vkt::Buffer dst(*m_device, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
        VkBufferCopy copy_info{};
        copy_info.size = size;

        vkt::Fence fence(*m_device);
        for (int i = 0; i < N; i++) {
            m_command_buffer.begin();
            vk::CmdCopyBuffer(m_command_buffer, src, dst, 1, &copy_info);
            m_command_buffer.end();
            {
                std::unique_lock<std::mutex> lock(queue_mutex);
                m_default_queue->Submit(m_command_buffer, fence);
            }
            vk::WaitForFences(device(), 1, &fence.handle(), VK_TRUE, kWaitTimeout);
            vk::ResetFences(device(), 1, &fence.handle());
        }
    });

    // Main thread submits empty batches and presents images
    {
        vkt::Semaphore acquire_semaphore(*m_device);
        vkt::Semaphore submit_semaphore(*m_device);
        vkt::Fence fence(*m_device);

        for (int i = 0; i < N; i++) {
            uint32_t image_index = 0;
            vk::AcquireNextImageKHR(device(), m_swapchain, kWaitTimeout, acquire_semaphore, VK_NULL_HANDLE, &image_index);
            {
                std::unique_lock<std::mutex> lock(queue_mutex);

                m_default_queue->Submit(vkt::no_cmd, acquire_semaphore, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                                        submit_semaphore, fence);

                VkPresentInfoKHR present = vku::InitStructHelper();
                present.waitSemaphoreCount = 1;
                present.pWaitSemaphores = &submit_semaphore.handle();
                present.swapchainCount = 1;
                present.pSwapchains = &m_swapchain;
                present.pImageIndices = &image_index;
                vk::QueuePresentKHR(*m_default_queue, &present);
            }
            vk::WaitForFences(device(), 1, &fence.handle(), VK_TRUE, kWaitTimeout);
            vk::ResetFences(device(), 1, &fence.handle());
        }
        {
            // We did not synchronize with the presentation request from the last iteration.
            // Wait on the queue to ensure submit semaphore used by presentation request is not in use.
            std::unique_lock<std::mutex> lock(queue_mutex);
            m_default_queue->Wait();
        }
    }
    thread.join();
}

// TODO: make this a shared function, since WSI tests also use it
static void SetImageLayoutPresentSrc(vkt::Queue& queue, vkt::Device& device, VkImage image) {
    vkt::CommandPool pool(device, device.graphics_queue_node_index_);
    vkt::CommandBuffer cmd_buf(device, pool);

    cmd_buf.begin();
    VkImageMemoryBarrier layout_barrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                                        nullptr,
                                        0,
                                        VK_ACCESS_MEMORY_READ_BIT,
                                        VK_IMAGE_LAYOUT_UNDEFINED,
                                        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                                        VK_QUEUE_FAMILY_IGNORED,
                                        VK_QUEUE_FAMILY_IGNORED,
                                        image,
                                        {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}};

    vk::CmdPipelineBarrier(cmd_buf.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr,
                           0, nullptr, 1, &layout_barrier);
    cmd_buf.end();
    queue.Submit(cmd_buf);
    queue.Wait();
}

TEST_F(PositiveSyncValWsi, WaitForFencesWithPresentBatches) {
    TEST_DESCRIPTION("Check that WaitForFences applies tagged waits to present batches");
    AddSurfaceExtension();
    RETURN_IF_SKIP(InitSyncVal());
    RETURN_IF_SKIP(InitSwapchain());
    const auto swapchain_images = GetSwapchainImages(m_swapchain);
    for (auto image : swapchain_images) {
        SetImageLayoutPresentSrc(*m_default_queue, *m_device, image);
    }

    vkt::Semaphore acquire_semaphore(*m_device);
    vkt::Semaphore submit_semaphore(*m_device);

    vkt::Semaphore acquire_semaphore2(*m_device);
    vkt::Semaphore submit_semaphore2(*m_device);

    vkt::Fence fence(*m_device);

    vkt::Buffer buffer(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    vkt::Buffer src_buffer(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    vkt::Buffer dst_buffer(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    auto present = [this](const vkt::Semaphore& submit_semaphore, uint32_t image_index) {
        VkPresentInfoKHR present = vku::InitStructHelper();
        present.waitSemaphoreCount = 1;
        present.pWaitSemaphores = &submit_semaphore.handle();
        present.swapchainCount = 1;
        present.pSwapchains = &m_swapchain;
        present.pImageIndices = &image_index;
        vk::QueuePresentKHR(m_default_queue->handle(), &present);
    };

    // Frame 0
    {
        uint32_t image_index = 0;
        vk::AcquireNextImageKHR(device(), m_swapchain, kWaitTimeout, acquire_semaphore, VK_NULL_HANDLE, &image_index);

        m_command_buffer.begin();
        m_command_buffer.Copy(src_buffer, buffer);
        m_command_buffer.end();

        m_default_queue->Submit(m_command_buffer, acquire_semaphore, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, submit_semaphore, fence);
        present(submit_semaphore, image_index);
    }
    // Frame 1
    {
        uint32_t image_index = 0;
        vk::AcquireNextImageKHR(device(), m_swapchain, kWaitTimeout, acquire_semaphore2, VK_NULL_HANDLE, &image_index);

        // TODO: Present should be able to accept semaphore from Acquire directly, but due to
        // another bug we need this intermediate sumbit. Remove it and make present to wait
        // on image_ready_semaphore semaphore when acquire->present direct synchronization is fixed.
        m_default_queue->Submit(vkt::no_cmd, acquire_semaphore2, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, submit_semaphore2);

        present(submit_semaphore2, image_index);
    }
    // Frame 2
    {
        // The goal of this test is to ensure that this wait is applied to the
        // batches resulted from queue presentation operations. Those batches
        // import accesses from regular submits.
        vk::WaitForFences(*m_device, 1, &fence.handle(), VK_TRUE, kWaitTimeout);

        uint32_t image_index = 0;
        vk::AcquireNextImageKHR(device(), m_swapchain, kWaitTimeout, acquire_semaphore, VK_NULL_HANDLE, &image_index);

        // If WaitForFences leaks accesses from present batches the following copy will cause submit time hazard.
        m_command_buffer.begin();
        m_command_buffer.Copy(buffer, dst_buffer);
        m_command_buffer.end();
        m_default_queue->Submit(m_command_buffer, vkt::wait, acquire_semaphore);
    }
    m_default_queue->Wait();
}
