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

#include "../framework/layer_validation_tests.h"

struct NegativeSyncValWsi : public VkSyncValTest {};

// Wrap FAIL:
//  * DRY for common messages
//  * for test stability reasons sometimes cleanup code is required *prior* to the return hidden in FAIL
//  * result_arg_ *can* (should) have side-effect, but is referenced exactly once
//  * label_ must be converitble to bool, and *should* *not* have side-effects
//    * "{}" or ";" are valid clean_ values for noop
#define REQUIRE_SUCCESS(result_arg_, label_)                            \
    {                                                                   \
        const VkResult result_ = (result_arg_);                         \
        if (result_ != VK_SUCCESS) {                                    \
            {                                                           \
                m_device->Wait();                                       \
            }                                                           \
            if (bool(label_)) {                                         \
                FAIL() << string_VkResult(result_) << ": " << (label_); \
            } else {                                                    \
                FAIL() << string_VkResult(result_);                     \
            }                                                           \
        }                                                               \
    }

TEST_F(NegativeSyncValWsi, PresentAcquire) {
    TEST_DESCRIPTION("Try destroying a swapchain presentable image with vkDestroyImage");

    AddSurfaceExtension();
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());
    RETURN_IF_SKIP(InitSwapchain());
    uint32_t image_count;
    std::vector<VkImage> images;
    ASSERT_EQ(VK_SUCCESS, vk::GetSwapchainImagesKHR(device(), m_swapchain, &image_count, nullptr));
    images.resize(image_count, VK_NULL_HANDLE);
    ASSERT_EQ(VK_SUCCESS, vk::GetSwapchainImagesKHR(device(), m_swapchain, &image_count, images.data()));

    std::vector<bool> image_used(images.size(), false);
    vkt::Fence fence(*m_device);

    // Loop through the indices until we find one we are reusing...
    // When fence is non-null this can timeout so we need to track results
    auto present_image = [this](uint32_t index, vkt::Semaphore* sem, vkt::Fence* fence) {
        VkResult result = VK_SUCCESS;
        if (fence) {
            result = fence->wait(kWaitTimeout);
            if (VK_SUCCESS == result) {
                fence->reset();
            }
        }

        if (VK_SUCCESS == result) {
            VkPresentInfoKHR present_info = vku::InitStructHelper();
            present_info.swapchainCount = 1;
            present_info.pSwapchains = &m_swapchain;
            present_info.pImageIndices = &index;
            VkSemaphore h_sem = VK_NULL_HANDLE;
            if (sem) {
                h_sem = sem->handle();
                present_info.waitSemaphoreCount = 1;
                present_info.pWaitSemaphores = &h_sem;
            }
            vk::QueuePresentKHR(m_default_queue->handle(), &present_info);
        }
        return result;
    };

    // Acquire can always timeout, so we need to track results
    auto acquire_used_image = [this, &image_used, &present_image](vkt::Semaphore* sem, vkt::Fence* fence, uint32_t& index) {
        VkSemaphore h_sem = sem ? sem->handle() : VK_NULL_HANDLE;
        VkFence h_fence = fence ? fence->handle() : VK_NULL_HANDLE;
        VkResult result = VK_SUCCESS;

        while (true) {
            result = vk::AcquireNextImageKHR(m_device->handle(), m_swapchain, kWaitTimeout, h_sem, h_fence, &index);
            if ((result != VK_SUCCESS) || image_used[index]) break;

            result = present_image(index, sem, fence);
            if (result != VK_SUCCESS) break;
            image_used[index] = true;
        }
        return result;
    };

    auto write_barrier_cb = [this](const VkImage h_image, VkImageLayout from, VkImageLayout to) {
        VkImageSubresourceRange full_image{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
        VkImageMemoryBarrier image_barrier = vku::InitStructHelper();
        image_barrier.srcAccessMask = 0U;
        image_barrier.dstAccessMask = 0U;
        image_barrier.oldLayout = from;
        image_barrier.newLayout = to;
        image_barrier.image = h_image;

        image_barrier.subresourceRange = full_image;
        m_command_buffer.begin();
        vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0,
                               nullptr, 0, nullptr, 1, &image_barrier);
        m_command_buffer.end();
    };

    // Transition swapchain images to PRESENT_SRC layout for presentation
    for (VkImage image : images) {
        write_barrier_cb(image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
        m_default_queue->Submit(m_command_buffer);
        m_device->Wait();
        m_command_buffer.reset();
    }

    uint32_t acquired_index = 0;
    REQUIRE_SUCCESS(acquire_used_image(nullptr, &fence, acquired_index), "acquire_used_image");

    write_barrier_cb(images[acquired_index], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    // Look for errors between the acquire and first use...
    // No sync operations...
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-PRESENT");
    m_default_queue->Submit(m_command_buffer);
    m_errorMonitor->VerifyFound();

    // Sync operations that should ignore present operations
    m_device->Wait();
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-PRESENT");
    m_default_queue->Submit(m_command_buffer);
    m_errorMonitor->VerifyFound();

    // Finally we wait for the fence associated with the acquire
    REQUIRE_SUCCESS(vk::WaitForFences(m_device->handle(), 1, &fence.handle(), VK_TRUE, kWaitTimeout), "WaitForFences");
    fence.reset();
    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();

    // Release the image back to the present engine, so we don't run out
    present_image(acquired_index, nullptr, nullptr);  // present without fence can't timeout

    vkt::Semaphore sem(*m_device);
    REQUIRE_SUCCESS(acquire_used_image(&sem, nullptr, acquired_index), "acquire_used_image");

    m_command_buffer.reset();
    write_barrier_cb(images[acquired_index], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    // The wait mask doesn't match the operations in the command buffer
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-READ");
    m_default_queue->Submit(m_command_buffer, vkt::wait, sem, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    m_errorMonitor->VerifyFound();

    // Now then wait mask matches the operations in the command buffer
    m_default_queue->Submit(m_command_buffer, vkt::wait, sem, VK_PIPELINE_STAGE_TRANSFER_BIT);

    // Try presenting without waiting for the ILT to finish
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-PRESENT-AFTER-WRITE");
    present_image(acquired_index, nullptr, nullptr);  // present without fence can't timeout
    m_errorMonitor->VerifyFound();

    // Let the ILT complete, and the release the image back
    m_device->Wait();
    present_image(acquired_index, nullptr, nullptr);  // present without fence can't timeout

    REQUIRE_SUCCESS(acquire_used_image(VK_NULL_HANDLE, &fence, acquired_index), "acquire_used_index");
    REQUIRE_SUCCESS(fence.wait(kWaitTimeout), "WaitForFences");

    m_command_buffer.reset();
    write_barrier_cb(images[acquired_index], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    fence.reset();
    m_default_queue->Submit(m_command_buffer, vkt::signal, sem);

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-PRESENT-AFTER-WRITE");
    present_image(acquired_index, nullptr, nullptr);  // present without fence can't timeout
    m_errorMonitor->VerifyFound();

    present_image(acquired_index, &sem, nullptr);  // present without fence can't timeout
    m_device->Wait();
}

TEST_F(NegativeSyncValWsi, PresentDoesNotWaitForSubmit2) {
    TEST_DESCRIPTION("Present does not specify semaphore to wait for submit.");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddSurfaceExtension();
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());
    RETURN_IF_SKIP(InitSwapchain());
    const vkt::Semaphore acquire_semaphore(*m_device);
    const vkt::Semaphore submit_semaphore(*m_device);
    const auto swapchain_images = GetSwapchainImages(m_swapchain);

    uint32_t image_index = 0;
    ASSERT_EQ(VK_SUCCESS,
              vk::AcquireNextImageKHR(device(), m_swapchain, kWaitTimeout, acquire_semaphore, VK_NULL_HANDLE, &image_index));

    VkImageMemoryBarrier2 layout_transition = vku::InitStructHelper();
    layout_transition.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    layout_transition.srcAccessMask = 0;
    layout_transition.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
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
    present.waitSemaphoreCount = 0;  // DO NOT wait on submit. This should generate present after write (ILT) harard.
    present.pWaitSemaphores = nullptr;
    present.swapchainCount = 1;
    present.pSwapchains = &m_swapchain;
    present.pImageIndices = &image_index;

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-PRESENT-AFTER-WRITE");
    vk::QueuePresentKHR(m_default_queue->handle(), &present);
    m_errorMonitor->VerifyFound();
    m_device->Wait();
}

TEST_F(NegativeSyncValWsi, PresentDoesNotWaitForSubmit) {
    TEST_DESCRIPTION("Present does not specify semaphore to wait for submit.");
    AddSurfaceExtension();
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());
    RETURN_IF_SKIP(InitSwapchain());
    const vkt::Semaphore acquire_semaphore(*m_device);
    const vkt::Semaphore submit_semaphore(*m_device);
    const auto swapchain_images = GetSwapchainImages(m_swapchain);

    uint32_t image_index = 0;
    ASSERT_EQ(VK_SUCCESS,
              vk::AcquireNextImageKHR(device(), m_swapchain, kWaitTimeout, acquire_semaphore, VK_NULL_HANDLE, &image_index));

    VkImageMemoryBarrier layout_transition = vku::InitStructHelper();
    layout_transition.srcAccessMask = 0;
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
    present.waitSemaphoreCount = 0;  // DO NOT wait on submit. This should generate present after write (ILT) harard.
    present.pWaitSemaphores = nullptr;
    present.swapchainCount = 1;
    present.pSwapchains = &m_swapchain;
    present.pImageIndices = &image_index;

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-PRESENT-AFTER-WRITE");
    vk::QueuePresentKHR(m_default_queue->handle(), &present);
    m_errorMonitor->VerifyFound();
    m_device->Wait();
}
