/* Copyright (c) 2025 The Khronos Group Inc.
 * Copyright (c) 2025 Valve Corporation
 * Copyright (c) 2025 LunarG, Inc.
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

#include "../framework/sync_val_tests.h"
#include <thread>

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
    const auto swapchain_images = m_swapchain.GetImages();
    const uint32_t image_index = m_swapchain.AcquireNextImage(acquire_semaphore, kWaitTimeout);

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

    m_command_buffer.Begin();
    m_command_buffer.Barrier(layout_transition);
    m_command_buffer.End();

    m_default_queue->Submit2(m_command_buffer, vkt::Wait(acquire_semaphore, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT),
                             vkt::Signal(submit_semaphore, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT));
    m_default_queue->Present(m_swapchain, image_index, submit_semaphore);
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
    const auto swapchain_images = m_swapchain.GetImages();
    const uint32_t image_index = m_swapchain.AcquireNextImage(acquire_semaphore, kWaitTimeout);

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

    m_command_buffer.Begin();
    vk::CmdPipelineBarrier(m_command_buffer, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &layout_transition);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer, vkt::Wait(acquire_semaphore, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT),
                            vkt::Signal(submit_semaphore));
    m_default_queue->Present(m_swapchain, image_index, submit_semaphore);
    m_default_queue->Wait();
}

TEST_F(PositiveSyncValWsi, PresentAfterSubmitNoneDstStage) {
    TEST_DESCRIPTION("Test that QueueSubmit's signal semaphore behaves the same way as QueueSubmit2 with ALL_COMMANDS signal.");
    AddSurfaceExtension();
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());
    RETURN_IF_SKIP(InitSwapchain());
    vkt::Semaphore acquire_semaphore(*m_device);
    vkt::Semaphore submit_semaphore(*m_device);
    const auto swapchain_images = m_swapchain.GetImages();
    const uint32_t image_index = m_swapchain.AcquireNextImage(acquire_semaphore, kWaitTimeout);

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

    m_command_buffer.Begin();
    m_command_buffer.Barrier(layout_transition);
    m_command_buffer.End();

    // The goal of this test is to use QueueSubmit API (not QueueSubmit2) to
    // ensure syncval correctly converts SubmitInfo to SubmitInfo2 with ALL_COMMANDS signal semaphore.
    m_default_queue->Submit(m_command_buffer, vkt::Wait(acquire_semaphore, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT),
                            vkt::Signal(submit_semaphore));

    m_default_queue->Present(m_swapchain, image_index, submit_semaphore);
    m_device->Wait();
}

TEST_F(PositiveSyncValWsi, ThreadedSubmitAndFenceWaitAndPresent) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/7250");
    AddSurfaceExtension();
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());
    RETURN_IF_SKIP(InitSwapchain());

    const auto swapchain_images = m_swapchain.GetImages();
    {
        vkt::CommandBuffer cmd(*m_device, m_command_pool);
        cmd.Begin();
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
        cmd.End();
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
            m_command_buffer.Begin();
            vk::CmdCopyBuffer(m_command_buffer, src, dst, 1, &copy_info);
            m_command_buffer.End();
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

        std::vector<vkt::Semaphore> submit_semaphores;
        for (size_t i = 0; i < swapchain_images.size(); i++) {
            submit_semaphores.emplace_back(*m_device);
        }

        vkt::Fence fence(*m_device);

        for (int i = 0; i < N; i++) {
            const uint32_t image_index = m_swapchain.AcquireNextImage(acquire_semaphore, kWaitTimeout);
            {
                std::unique_lock<std::mutex> lock(queue_mutex);
                m_default_queue->Submit(vkt::no_cmd, vkt::Wait(acquire_semaphore, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT),
                                        vkt::Signal(submit_semaphores[image_index]), fence);
                m_default_queue->Present(m_swapchain, image_index, submit_semaphores[image_index]);
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

TEST_F(PositiveSyncValWsi, WaitForFencesWithPresentBatches) {
    TEST_DESCRIPTION("Check that WaitForFences applies tagged waits to present batches");
    AddSurfaceExtension();
    RETURN_IF_SKIP(InitSyncVal());
    RETURN_IF_SKIP(InitSwapchain());
    const auto swapchain_images = m_swapchain.GetImages();
    for (auto image : swapchain_images) {
        SetPresentImageLayout(image);
    }

    vkt::Semaphore acquire_semaphore(*m_device);
    vkt::Semaphore submit_semaphore(*m_device);

    vkt::Semaphore acquire_semaphore2(*m_device);
    vkt::Semaphore submit_semaphore2(*m_device);

    vkt::Fence fence(*m_device);

    vkt::Buffer buffer(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    vkt::Buffer src_buffer(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    vkt::Buffer dst_buffer(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    // Frame 0
    {
        const uint32_t image_index = m_swapchain.AcquireNextImage(acquire_semaphore, kWaitTimeout);

        m_command_buffer.Begin();
        m_command_buffer.Copy(src_buffer, buffer);
        m_command_buffer.End();

        m_default_queue->Submit(m_command_buffer, vkt::Wait(acquire_semaphore), vkt::Signal(submit_semaphore), fence);
        m_default_queue->Present(m_swapchain, image_index, submit_semaphore);
    }
    // Frame 1
    {
        const uint32_t image_index = m_swapchain.AcquireNextImage(acquire_semaphore2, kWaitTimeout);

        // TODO: Present should be able to accept semaphore from Acquire directly, but due to
        // another bug we need this intermediate sumbit. Remove it and make present to wait
        // on image_ready_semaphore semaphore when acquire->present direct synchronization is fixed.
        m_default_queue->Submit(vkt::no_cmd, vkt::Wait(acquire_semaphore2), vkt::Signal(submit_semaphore2));

        m_default_queue->Present(m_swapchain, image_index, submit_semaphore2);
    }
    // Frame 2
    {
        // The goal of this test is to ensure that this wait is applied to the
        // batches resulted from queue presentation operations. Those batches
        // import accesses from regular submits.
        vk::WaitForFences(*m_device, 1, &fence.handle(), VK_TRUE, kWaitTimeout);

        m_swapchain.AcquireNextImage(acquire_semaphore, kWaitTimeout);  // do not need to keep result

        // If WaitForFences leaks accesses from present batches the following copy will cause submit time hazard.
        m_command_buffer.Begin();
        m_command_buffer.Copy(buffer, dst_buffer);
        m_command_buffer.End();
        m_default_queue->Submit(m_command_buffer, vkt::Wait(acquire_semaphore));
    }
    m_default_queue->Wait();
}

TEST_F(PositiveSyncValWsi, RecreateBuffer) {
    TEST_DESCRIPTION("Recreate buffer on each simulation iteration. Use acquire fence synchronization approach.");
    AddSurfaceExtension();
    RETURN_IF_SKIP(InitSyncVal());
    RETURN_IF_SKIP(InitSwapchain());

    const auto swapchain_images = m_swapchain.GetImages();

    std::vector<vkt::Fence> acquire_fences;
    vkt::Fence current_fence(*m_device);
    std::vector<vkt::CommandBuffer> command_buffers;
    std::vector<vkt::Semaphore> submit_semaphores;

    std::vector<vkt::Buffer> src_buffers(swapchain_images.size());
    std::vector<vkt::Buffer> dst_buffers(swapchain_images.size());

    for (VkImage image : swapchain_images) {
        SetPresentImageLayout(image);
    }
    for (size_t i = 0; i < swapchain_images.size(); i++) {
        acquire_fences.emplace_back(*m_device);
        command_buffers.emplace_back(*m_device, m_command_pool);
        submit_semaphores.emplace_back(*m_device);
    }

    // NOTE: This test can be used for manual inspection of memory usage.
    // Increase frame count and observe that the test does not continuously allocate memory.
    // Syncval should not track ranges of deleted resources.
    const int frame_count = 100;

    for (int i = 0; i < frame_count; i++) {
        const uint32_t image_index = m_swapchain.AcquireNextImage(current_fence, kWaitTimeout);
        current_fence.Wait(kWaitTimeout);
        current_fence.Reset();

        auto &src_buffer = src_buffers[image_index];
        src_buffer.Destroy();
        src_buffer = vkt::Buffer(*m_device, 1024, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

        auto &dst_buffer = dst_buffers[image_index];
        dst_buffer.Destroy();
        dst_buffer = vkt::Buffer(*m_device, 1024, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

        auto &command_buffer = command_buffers[image_index];
        command_buffer.Begin();
        command_buffer.Copy(src_buffer, dst_buffer);
        command_buffer.End();

        auto &submit_semaphore = submit_semaphores[image_index];
        m_default_queue->Submit(command_buffer, vkt::Signal(submit_semaphore));
        m_default_queue->Present(m_swapchain, image_index, submit_semaphore);
        std::swap(acquire_fences[image_index], current_fence);
    }
    m_default_queue->Wait();
}

TEST_F(PositiveSyncValWsi, RecreateImage) {
    TEST_DESCRIPTION("Recreate image on each simulation iteration. Use acquire fence synchronization approach.");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddSurfaceExtension();
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitSyncVal());
    RETURN_IF_SKIP(InitSwapchain());

    constexpr uint32_t width = 256;
    constexpr uint32_t height = 128;
    constexpr VkFormat format = VK_FORMAT_B8G8R8A8_UNORM;

    const auto swapchain_images = m_swapchain.GetImages();

    std::vector<vkt::Fence> acquire_fences;
    vkt::Fence current_fence(*m_device);
    std::vector<vkt::CommandBuffer> command_buffers;
    std::vector<vkt::Semaphore> submit_semaphores;

    const vkt::Buffer src_buffer(*m_device, width * height * 4, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    std::vector<vkt::Image> dst_images(swapchain_images.size());

    for (auto image : swapchain_images) {
        SetPresentImageLayout(image);
    }
    for (size_t i = 0; i < swapchain_images.size(); i++) {
        acquire_fences.emplace_back(*m_device);
        command_buffers.emplace_back(*m_device, m_command_pool);
        submit_semaphores.emplace_back(*m_device);
    }

    // NOTE: This test can be used for manual inspection of memory usage.
    // Increase frame count and observe that the test does not continuously allocate memory.
    // Syncval should not track ranges of deleted resources.
    const int frame_count = 100;

    for (int i = 0; i < frame_count; i++) {
        const uint32_t image_index = m_swapchain.AcquireNextImage(current_fence, kWaitTimeout);
        current_fence.Wait(kWaitTimeout);
        current_fence.Reset();

        auto &dst_image = dst_images[image_index];
        dst_image.Destroy();
        dst_image = vkt::Image(*m_device, width, height, format, VK_IMAGE_USAGE_TRANSFER_DST_BIT);

        VkBufferImageCopy region = {};
        region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
        region.imageExtent = {width, height, 1};

        VkImageMemoryBarrier2 layout_transition = vku::InitStructHelper();
        layout_transition.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
        layout_transition.srcAccessMask = VK_ACCESS_2_NONE;
        layout_transition.dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
        layout_transition.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        layout_transition.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        layout_transition.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        layout_transition.image = dst_image;
        layout_transition.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

        auto &command_buffer = command_buffers[image_index];
        command_buffer.Begin();
        command_buffer.Barrier(layout_transition);
        vk::CmdCopyBufferToImage(command_buffer, src_buffer, dst_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
        command_buffer.End();

        auto &submit_semaphore = submit_semaphores[image_index];
        m_default_queue->Submit(command_buffer, vkt::Signal(submit_semaphore));
        m_default_queue->Present(m_swapchain, image_index, submit_semaphore);
        std::swap(acquire_fences[image_index], current_fence);
    }
    m_default_queue->Wait();
}
