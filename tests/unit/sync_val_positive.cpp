/*
 * Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
 * Copyright (c) 2015-2024 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/descriptor_helper.h"
#include "../framework/render_pass_helper.h"
#include "../framework/thread_helper.h"
#include "../framework/queue_submit_context.h"

class PositiveSyncVal : public VkSyncValTest {};

void VkSyncValTest::InitSyncValFramework(bool disable_queue_submit_validation) {
    // Enable synchronization validation
    features_ = {VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT, nullptr, 1u, enables_, 4, disables_};

    // Optionally enable core validation (by disabling nothing)
    if (!m_syncval_disable_core) {
        features_.disabledValidationFeatureCount = 0;
    }

    // Optionally disable syncval submit validation
    static const char *kDisableQueuSubmitSyncValidation[] = {"VALIDATION_CHECK_DISABLE_SYNCHRONIZATION_VALIDATION_QUEUE_SUBMIT"};
    static const VkLayerSettingEXT settings[] = {
        {OBJECT_LAYER_NAME, "disables", VK_LAYER_SETTING_TYPE_STRING_EXT, 1, kDisableQueuSubmitSyncValidation}};
    // The pNext of qs_settings is modified by InitFramework that's why it can't
    // be static (should be separate instance per stack frame). Also we show
    // explicitly that it's not const (InitFramework casts const pNext to non-const).
    VkLayerSettingsCreateInfoEXT qs_settings{VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr,
                                             static_cast<uint32_t>(std::size(settings)), settings};
    if (disable_queue_submit_validation) {
        features_.pNext = &qs_settings;
    }
    InitFramework(&features_);
}

TEST_F(PositiveSyncVal, CmdClearAttachmentLayer) {
    TEST_DESCRIPTION(
        "Clear one attachment layer and copy to a different one."
        "This checks for bug regression that produced a false-positive WAW hazard.");

    // VK_EXT_load_store_op_none is needed to disable render pass load/store accesses, so clearing
    // attachment inside a render pass can create hazards with the copy operations outside render pass.
    AddRequiredExtensions(VK_EXT_LOAD_STORE_OP_NONE_EXTENSION_NAME);

    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    const uint32_t width = 256;
    const uint32_t height = 128;
    const uint32_t layers = 2;
    const VkFormat rt_format = VK_FORMAT_B8G8R8A8_UNORM;
    const auto transfer_usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    const auto rt_usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | transfer_usage;

    vkt::Image image(*m_device, width, height, 1, rt_format, transfer_usage);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    vkt::Image rt(*m_device, vkt::Image::ImageCreateInfo2D(width, height, 1, layers, rt_format, rt_usage));
    rt.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    auto attachment_without_load_store = [](VkFormat format) {
        VkAttachmentDescription attachment = {};
        attachment.format = format;
        attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        attachment.loadOp = VK_ATTACHMENT_LOAD_OP_NONE_KHR;
        attachment.storeOp = VK_ATTACHMENT_STORE_OP_NONE_KHR;
        attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_NONE_KHR;
        attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_NONE_KHR;
        attachment.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
        attachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
        return attachment;
    };
    const VkAttachmentDescription attachment = attachment_without_load_store(rt_format);
    const VkAttachmentReference color_ref = {0, VK_IMAGE_LAYOUT_GENERAL};

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_ref;

    VkRenderPassCreateInfo rpci = vku::InitStructHelper();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attachment;
    vkt::RenderPass render_pass(*m_device, rpci);

    vkt::ImageView rt_view = rt.CreateView(VK_IMAGE_VIEW_TYPE_2D_ARRAY, 0, 1, 0, layers);
    VkFramebufferCreateInfo fbci = vku::InitStructHelper();
    fbci.flags = 0;
    fbci.renderPass = render_pass;
    fbci.attachmentCount = 1;
    fbci.pAttachments = &rt_view.handle();
    fbci.width = width;
    fbci.height = height;
    fbci.layers = layers;
    vkt::Framebuffer framebuffer(*m_device, fbci);

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.renderPass = render_pass;
    pipe.CreateGraphicsPipeline();

    VkImageCopy copy_region = {};
    copy_region.srcSubresource = {VkImageAspectFlags(VK_IMAGE_ASPECT_COLOR_BIT), 0, 0, 1};
    copy_region.dstSubresource = {VkImageAspectFlags(VK_IMAGE_ASPECT_COLOR_BIT), 0, 0 /* copy only to layer 0 */, 1};
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstOffset = {0, 0, 0};
    copy_region.extent = {width, height, 1};

    VkClearRect clear_rect = {};
    clear_rect.rect.offset = {0, 0};
    clear_rect.rect.extent = {width, height};
    clear_rect.baseArrayLayer = 1;  // skip layer 0, clear only layer 1
    clear_rect.layerCount = 1;
    const VkClearAttachment clear_attachment = {VK_IMAGE_ASPECT_COLOR_BIT};

    m_commandBuffer->begin();
    // Write 1: Copy to render target's layer 0
    vk::CmdCopyImage(*m_commandBuffer, image, VK_IMAGE_LAYOUT_GENERAL, rt, VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    m_commandBuffer->BeginRenderPass(render_pass, framebuffer, width, height);
    // Write 2: Clear render target's layer 1
    vk::CmdClearAttachments(*m_commandBuffer, 1, &clear_attachment, 1, &clear_rect);
    vk::CmdEndRenderPass(*m_commandBuffer);
    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer();
    m_default_queue->wait();
}

// Image transition ensures that image data is made visible and available when necessary.
// The user's responsibility is only to properly define the barrier. This test checks that
// writing to the image immediately after the transition  does not produce WAW hazard against
// the writes performed by the transition.
TEST_F(PositiveSyncVal, WriteToImageAfterTransition) {
    TEST_DESCRIPTION("Perform image transition then copy to image from buffer");
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    constexpr uint32_t width = 256;
    constexpr uint32_t height = 128;
    constexpr VkFormat format = VK_FORMAT_B8G8R8A8_UNORM;

    vkt::Buffer buffer(*m_device, width * height * 4, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    vkt::Image image(*m_device, width, height, 1, format, VK_IMAGE_USAGE_TRANSFER_DST_BIT);

    VkImageMemoryBarrier barrier = vku::InitStructHelper();
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.layerCount = 1;

    VkBufferImageCopy region = {};
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.layerCount = 1;
    region.imageExtent.width = width;
    region.imageExtent.height = height;
    region.imageExtent.depth = 1;

    m_commandBuffer->begin();
    vk::CmdPipelineBarrier(*m_commandBuffer, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr,
                           1, &barrier);
    vk::CmdCopyBufferToImage(*m_commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    m_commandBuffer->end();
}

// Regression test for vkWaitSemaphores timeout while waiting for vkSignalSemaphore.
// This scenario is not an issue for validation objects that use fine grained locking.
// https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/4968
TEST_F(PositiveSyncVal, SignalAndWaitSemaphoreOnHost) {
    TEST_DESCRIPTION("Signal semaphore on the host and wait on the host");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitSyncValFramework());
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    RETURN_IF_SKIP(InitState());
    if (IsPlatformMockICD()) {
        // Mock does not support proper ordering of events, e.g. wait can return before signal
        GTEST_SKIP() << "Test not supported by MockICD";
    }

    constexpr uint64_t max_signal_value = 10'000;

    VkSemaphoreTypeCreateInfo semaphore_type_info = vku::InitStructHelper();
    semaphore_type_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
    const VkSemaphoreCreateInfo create_info = vku::InitStructHelper(&semaphore_type_info);
    vkt::Semaphore semaphore(*m_device, create_info);

    std::atomic<bool> bailout{false};
    m_errorMonitor->SetBailout(&bailout);

    // Send signals
    auto signaling_thread = std::thread{[&] {
        uint64_t last_signalled_value = 0;
        while (last_signalled_value != max_signal_value) {
            VkSemaphoreSignalInfo signal_info = vku::InitStructHelper();
            signal_info.semaphore = semaphore;
            signal_info.value = ++last_signalled_value;
            ASSERT_EQ(VK_SUCCESS, vk::SignalSemaphore(*m_device, &signal_info));
            if (bailout.load()) {
                break;
            }
        }
    }};
    // Wait for each signal
    uint64_t wait_value = 1;
    while (wait_value <= max_signal_value) {
        VkSemaphoreWaitInfo wait_info = vku::InitStructHelper();
        wait_info.flags = VK_SEMAPHORE_WAIT_ANY_BIT;
        wait_info.semaphoreCount = 1;
        wait_info.pSemaphores = &semaphore.handle();
        wait_info.pValues = &wait_value;
        ASSERT_EQ(VK_SUCCESS, vk::WaitSemaphores(*m_device, &wait_info, vvl::kU64Max));
        ++wait_value;
        if (bailout.load()) {
            break;
        }
    }
    signaling_thread.join();
}

// Regression test for vkGetSemaphoreCounterValue timeout while waiting for vkSignalSemaphore.
// This scenario is not an issue for validation objects that use fine grained locking.
// https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/4968
TEST_F(PositiveSyncVal, SignalAndGetSemaphoreCounter) {
    TEST_DESCRIPTION("Singal semaphore on the host and regularly read semaphore payload value");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitSyncValFramework());
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    RETURN_IF_SKIP(InitState());
    if (IsPlatformMockICD()) {
        // Mock does not support precise semaphore counter reporting
        GTEST_SKIP() << "Test not supported by MockICD";
    }

    constexpr uint64_t max_signal_value = 1'000;

    VkSemaphoreTypeCreateInfo semaphore_type_info = vku::InitStructHelper();
    semaphore_type_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
    const VkSemaphoreCreateInfo create_info = vku::InitStructHelper(&semaphore_type_info);
    vkt::Semaphore semaphore(*m_device, create_info);

    std::atomic<bool> bailout{false};
    m_errorMonitor->SetBailout(&bailout);

    // Send signals
    auto signaling_thread = std::thread{[&] {
        uint64_t last_signalled_value = 0;
        while (last_signalled_value != max_signal_value) {
            VkSemaphoreSignalInfo signal_info = vku::InitStructHelper();
            signal_info.semaphore = semaphore;
            signal_info.value = ++last_signalled_value;
            ASSERT_EQ(VK_SUCCESS, vk::SignalSemaphore(*m_device, &signal_info));
            if (bailout.load()) {
                break;
            }
        }
    }};
    // Spin until semaphore payload value equals maximum signaled value
    uint64_t counter = 0;
    while (counter != max_signal_value) {
        ASSERT_EQ(VK_SUCCESS, vk::GetSemaphoreCounterValue(*m_device, semaphore, &counter));
        if (bailout.load()) {
            break;
        }
    }
    signaling_thread.join();
}

TEST_F(PositiveSyncVal, GetSemaphoreCounterFromMultipleThreads) {
    TEST_DESCRIPTION("Read semaphore counter value from multiple threads");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitSyncValFramework());
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    RETURN_IF_SKIP(InitState());
    if (IsPlatformMockICD()) {
        // Mock does not support precise semaphore counter reporting
        GTEST_SKIP() << "Test not supported by MockICD";
    }

    constexpr uint64_t max_signal_value = 15'000;

    VkSemaphoreTypeCreateInfo semaphore_type_info = vku::InitStructHelper();
    semaphore_type_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
    const VkSemaphoreCreateInfo create_info = vku::InitStructHelper(&semaphore_type_info);
    vkt::Semaphore semaphore(*m_device, create_info);

    std::atomic<bool> bailout{false};
    m_errorMonitor->SetBailout(&bailout);

    constexpr int waiter_count = 24;
    ThreadTimeoutHelper timeout_helper(waiter_count + 1 /* signaling thread*/);

    // Start a bunch of waiter threads
    auto waiting_thread = [&]() {
        auto timeout_guard = timeout_helper.ThreadGuard();
        uint64_t counter = 0;
        while (counter != max_signal_value) {
            ASSERT_EQ(VK_SUCCESS, vk::GetSemaphoreCounterValue(*m_device, semaphore, &counter));
            if (bailout.load()) {
                break;
            }
        }
    };
    std::vector<std::thread> waiters;
    for (int i = 0; i < waiter_count; i++) {
        waiters.emplace_back(waiting_thread);
    }
    // The signaling thread advances semaphore's payload value
    auto signaling_thread = std::thread([&] {
        auto timeout_guard = timeout_helper.ThreadGuard();
        uint64_t last_signalled_value = 0;
        while (last_signalled_value != max_signal_value) {
            VkSemaphoreSignalInfo signal_info = vku::InitStructHelper();
            signal_info.semaphore = semaphore;
            signal_info.value = ++last_signalled_value;
            ASSERT_EQ(VK_SUCCESS, vk::SignalSemaphore(*m_device, &signal_info));
            if (bailout.load()) {
                break;
            }
        }
    });

    // In the regression case we expect crashes or deadlocks.
    // Timeout helper will unstack CI machines in case of a deadlock.
#ifdef NDEBUG
    constexpr int wait_time = 100;
#else
    // Use large timeout value in case of bad CI performance.
    // On Windows debug build on development machine (4 cores) takes less than 10 seconds.
    // Some CI runs of debug build on Linux machine took around 60 seconds and there were
    // few timeouts with 100 seconds. Use larger values to test if it's stable enough.
    // Another option is to reduce thread count or iteration count for debug build.
    constexpr int wait_time = 300;
#endif

    if (!timeout_helper.WaitForThreads(wait_time)) {
        ADD_FAILURE() << "The waiting time for the worker threads exceeded the maximum limit: " << wait_time << " seconds.";
        return;
    }
    for (auto &waiter : waiters) {
        waiter.join();
    }
    signaling_thread.join();
}

TEST_F(PositiveSyncVal, ShaderReferencesNotBoundSet) {
    TEST_DESCRIPTION("Shader references a descriptor set that was not bound. SyncVal should not crash if core checks are disabled");
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    const VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr};
    const vkt::DescriptorSetLayout set_layout(*m_device, {binding});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&set_layout, &set_layout});
    OneOffDescriptorSet set(m_device, {binding});

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    // Bind set 0.
    vk::CmdBindDescriptorSets(*m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &set.set_, 0, nullptr);
    vk::CmdBindPipeline(*m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    // Core checks prevent SyncVal from running when error is found. This test has core checks disabled and also invalid
    // setup where a shader uses not bound set 1.
    // Check that syncval does not cause out of bounds access (PerSet has single element (index 0), shader set index is 1).
    vk::CmdDraw(*m_commandBuffer, 3, 1, 0, 0);

    vk::CmdEndRenderPass(*m_commandBuffer);
    m_commandBuffer->end();
}

TEST_F(PositiveSyncVal, PresentAfterSubmit2AutomaticVisibility) {
    TEST_DESCRIPTION("Waiting on the semaphore makes available image accesses visible to the presentation engine.");
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

    m_commandBuffer->begin();
    vk::CmdPipelineBarrier2(*m_commandBuffer, &dep_info);
    m_commandBuffer->end();

    VkSemaphoreSubmitInfo wait_info = vku::InitStructHelper();
    wait_info.semaphore = acquire_semaphore;
    wait_info.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkCommandBufferSubmitInfo command_buffer_info = vku::InitStructHelper();
    command_buffer_info.commandBuffer = *m_commandBuffer;

    VkSemaphoreSubmitInfo signal_info = vku::InitStructHelper();
    signal_info.semaphore = submit_semaphore;
    signal_info.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo2 submit = vku::InitStructHelper();
    submit.waitSemaphoreInfoCount = 1;
    submit.pWaitSemaphoreInfos = &wait_info;
    submit.commandBufferInfoCount = 1;
    submit.pCommandBufferInfos = &command_buffer_info;
    submit.signalSemaphoreInfoCount = 1;
    submit.pSignalSemaphoreInfos = &signal_info;
    ASSERT_EQ(VK_SUCCESS, vk::QueueSubmit2(m_default_queue->handle(), 1, &submit, VK_NULL_HANDLE));

    VkPresentInfoKHR present = vku::InitStructHelper();
    present.waitSemaphoreCount = 1;
    present.pWaitSemaphores = &submit_semaphore.handle();
    present.swapchainCount = 1;
    present.pSwapchains = &m_swapchain;
    present.pImageIndices = &image_index;
    ASSERT_EQ(VK_SUCCESS, vk::QueuePresentKHR(m_default_queue->handle(), &present));
    m_default_queue->wait();
}

TEST_F(PositiveSyncVal, PresentAfterSubmitAutomaticVisibility) {
    TEST_DESCRIPTION("Waiting on the semaphore makes available image accesses visible to the presentation engine.");
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

    m_commandBuffer->begin();
    vk::CmdPipelineBarrier(*m_commandBuffer, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &layout_transition);
    m_commandBuffer->end();

    constexpr VkPipelineStageFlags semaphore_wait_stage = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submit = vku::InitStructHelper();
    submit.waitSemaphoreCount = 1;
    submit.pWaitSemaphores = &acquire_semaphore.handle();
    submit.pWaitDstStageMask = &semaphore_wait_stage;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &m_commandBuffer->handle();
    submit.signalSemaphoreCount = 1;
    submit.pSignalSemaphores = &submit_semaphore.handle();
    ASSERT_EQ(VK_SUCCESS, vk::QueueSubmit(m_default_queue->handle(), 1, &submit, VK_NULL_HANDLE));

    VkPresentInfoKHR present = vku::InitStructHelper();
    present.waitSemaphoreCount = 1;
    present.pWaitSemaphores = &submit_semaphore.handle();
    present.swapchainCount = 1;
    present.pSwapchains = &m_swapchain;
    present.pImageIndices = &image_index;
    ASSERT_EQ(VK_SUCCESS, vk::QueuePresentKHR(m_default_queue->handle(), &present));
    m_default_queue->wait();
}

TEST_F(PositiveSyncVal, PresentAfterSubmitNoneDstStage) {
    TEST_DESCRIPTION("Test that QueueSubmit's signal semaphore behaves the same way as QueueSubmit2 with ALL_COMMANDS signal.");
    AddSurfaceExtension();
    SetTargetApiVersion(VK_API_VERSION_1_3);
    VkPhysicalDeviceSynchronization2Features sync2_features = vku::InitStructHelper();
    sync2_features.synchronization2 = VK_TRUE;
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState(nullptr, &sync2_features));
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

    m_commandBuffer->begin();
    vk::CmdPipelineBarrier2(*m_commandBuffer, &dep_info);
    m_commandBuffer->end();

    constexpr VkPipelineStageFlags semaphore_wait_stage = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submit = vku::InitStructHelper();
    submit.waitSemaphoreCount = 1;
    submit.pWaitSemaphores = &acquire_semaphore.handle();
    submit.pWaitDstStageMask = &semaphore_wait_stage;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &m_commandBuffer->handle();
    submit.signalSemaphoreCount = 1;
    submit.pSignalSemaphores = &submit_semaphore.handle();

    // The goal of this test is to use QueueSubmit API (not QueueSubmit2) to
    // ensure syncval correctly converts SubmitInfo to SubmitInfo2 with
    // regard to signal semaphore.
    vk::QueueSubmit(m_default_queue->handle(), 1, &submit, VK_NULL_HANDLE);

    VkPresentInfoKHR present = vku::InitStructHelper();
    present.waitSemaphoreCount = 1;
    present.pWaitSemaphores = &submit_semaphore.handle();
    present.swapchainCount = 1;
    present.pSwapchains = &m_swapchain;
    present.pImageIndices = &image_index;

    vk::QueuePresentKHR(m_default_queue->handle(), &present);
    m_device->wait();
}

TEST_F(PositiveSyncVal, SeparateAvailabilityAndVisibilityForBuffer) {
    TEST_DESCRIPTION("Use separate barriers for availability and visibility operations.");
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    constexpr VkDeviceSize size = 1024;
    const vkt::Buffer staging_buffer(*m_device, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    const vkt::Buffer buffer(*m_device, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    VkBufferCopy region = {};
    region.size = size;

    m_commandBuffer->begin();
    // Perform a copy
    vk::CmdCopyBuffer(*m_commandBuffer, staging_buffer, buffer, 1, &region);

    // Make writes available
    VkBufferMemoryBarrier barrier_a = vku::InitStructHelper();
    barrier_a.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier_a.dstAccessMask = 0;
    barrier_a.buffer = buffer;
    barrier_a.size = VK_WHOLE_SIZE;
    vk::CmdPipelineBarrier(*m_commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 1,
                           &barrier_a, 0, nullptr);

    // Make writes visible
    VkBufferMemoryBarrier barrier_b = vku::InitStructHelper();
    barrier_b.srcAccessMask = 0;  // already available
    barrier_b.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier_b.buffer = buffer;
    barrier_b.size = VK_WHOLE_SIZE;
    vk::CmdPipelineBarrier(*m_commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 1,
                           &barrier_b, 0, nullptr);

    // Perform one more copy. Should not generate WAW.
    vk::CmdCopyBuffer(*m_commandBuffer, staging_buffer, buffer, 1, &region);
    m_commandBuffer->end();
}

TEST_F(PositiveSyncVal, LayoutTransitionWithAlreadyAvailableImage) {
    TEST_DESCRIPTION(
        "Image barrier makes image available but not visible. A subsequent layout transition barrier should not generate hazards. "
        "Available memory is automatically made visible to a layout transition.");
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    constexpr VkDeviceSize buffer_size = 64 * 64 * 4;
    const vkt::Buffer buffer(*m_device, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

    vkt::Image image(*m_device, 64, 64, 1, VK_FORMAT_R8G8B8A8_UNORM,
                     VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    m_commandBuffer->begin();

    // Copy data from buffer to image
    VkBufferImageCopy region = {};
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.layerCount = 1;
    region.imageExtent.width = 64;
    region.imageExtent.height = 64;
    region.imageExtent.depth = 1;
    vk::CmdCopyBufferToImage(*m_commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    // Make writes available
    VkImageMemoryBarrier barrier_a = vku::InitStructHelper();
    barrier_a.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier_a.dstAccessMask = 0;
    barrier_a.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier_a.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier_a.image = image;
    barrier_a.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    vk::CmdPipelineBarrier(*m_commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0,
                           nullptr, 1, &barrier_a);

    // Transition to new layout. Available memory should automatically be made visible to the layout transition.
    VkImageMemoryBarrier barrier_b = vku::InitStructHelper();
    barrier_b.srcAccessMask = 0;  // already available
    barrier_b.dstAccessMask = 0;  // for this test we don't care if the memory is visible after the transition or not
    barrier_b.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier_b.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    barrier_b.image = image;
    barrier_b.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    vk::CmdPipelineBarrier(*m_commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0,
                           nullptr, 0, nullptr, 1, &barrier_b);
    m_commandBuffer->end();
}

TEST_F(PositiveSyncVal, ImageArrayDynamicIndexing) {
    TEST_DESCRIPTION("Access different elements of the image array using dynamic indexing. There should be no hazards");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitSyncValFramework());
    VkPhysicalDeviceVulkan12Features features12 = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(features12);
    if (features12.runtimeDescriptorArray != VK_TRUE) {
        GTEST_SKIP() << "runtimeDescriptorArray not supported and is required";
    }
    RETURN_IF_SKIP(InitState(nullptr, &features12));
    InitRenderTarget();

    constexpr VkDescriptorType descriptor_type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    constexpr VkImageLayout image_layout = VK_IMAGE_LAYOUT_GENERAL;

    vkt::Image images[4];
    vkt::ImageView views[4];
    for (int i = 0; i < 4; i++) {
        images[i].Init(*m_device, 64, 64, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_STORAGE_BIT);
        ASSERT_TRUE(images[i].initialized());
        images[i].SetLayout(image_layout);
        views[i] = images[i].CreateView();
    }

    const OneOffDescriptorSet::Bindings bindings = {
        {0, descriptor_type, 4, VK_SHADER_STAGE_ALL, nullptr},
    };
    OneOffDescriptorSet descriptor_set(m_device, bindings);
    descriptor_set.WriteDescriptorImageInfo(0, views[0], VK_NULL_HANDLE, descriptor_type, image_layout, 0);
    descriptor_set.WriteDescriptorImageInfo(0, views[1], VK_NULL_HANDLE, descriptor_type, image_layout, 1);
    descriptor_set.WriteDescriptorImageInfo(0, views[2], VK_NULL_HANDLE, descriptor_type, image_layout, 2);
    descriptor_set.WriteDescriptorImageInfo(0, views[3], VK_NULL_HANDLE, descriptor_type, image_layout, 3);
    descriptor_set.UpdateDescriptorSets();

    // Write to image 0 or 1
    const char fsSource[] = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable
        layout(set=0, binding=0, rgba8) uniform image2D image_array[];
        void main() {
           imageStore(image_array[nonuniformEXT(int(gl_FragCoord.x) % 2)], ivec2(1, 1), vec4(1.0, 0.5, 0.2, 1.0));
        }
    )glsl";
    const VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);
    CreatePipelineHelper gfx_pipe(*this);
    gfx_pipe.shader_stages_ = {gfx_pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    gfx_pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&descriptor_set.layout_});
    gfx_pipe.CreateGraphicsPipeline();

    // Read from image 2 or 3
    char const *cs_source = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable
        layout(set=0, binding=0, rgba8) uniform image2D image_array[];
        void main() {
            vec4 data = imageLoad(image_array[nonuniformEXT(2 + (gl_LocalInvocationID.x % 2))], ivec2(1, 1));
        }
    )glsl";
    CreateComputePipelineHelper cs_pipe(*this);
    cs_pipe.dsl_bindings_ = bindings;
    cs_pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    cs_pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&descriptor_set.layout_});
    cs_pipe.CreateComputePipeline();

    m_commandBuffer->begin();
    // Graphics pipeline writes
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(*m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, gfx_pipe.pipeline_);
    vk::CmdBindDescriptorSets(*m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, gfx_pipe.pipeline_layout_, 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(*m_commandBuffer, 3, 1, 0, 0);
    m_commandBuffer->EndRenderPass();
    // Compute pipeline reads
    vk::CmdBindPipeline(*m_commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, cs_pipe.pipeline_);
    vk::CmdBindDescriptorSets(*m_commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, cs_pipe.pipeline_layout_, 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDispatch(*m_commandBuffer, 1, 1, 1);
    m_commandBuffer->end();

    m_default_queue->submit(*m_commandBuffer);
    m_default_queue->wait();
}

TEST_F(PositiveSyncVal, ImageArrayConstantIndexing) {
    TEST_DESCRIPTION("Access different elements of the image array using constant indices. There should be no hazards");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(InitSyncValFramework());
    VkPhysicalDeviceFeatures2 features2 = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(features2);
    if (!features2.features.fragmentStoresAndAtomics) {
        GTEST_SKIP() << "Test requires (unsupported) fragmentStoresAndAtomics";
    }
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    vkt::Image image(*m_device, 64, 64, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_STORAGE_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    const vkt::ImageView view = image.CreateView();

    const OneOffDescriptorSet::Bindings bindings = {
        {0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 2, VK_SHADER_STAGE_ALL, nullptr},
    };
    OneOffDescriptorSet descriptor_set(m_device, bindings);
    descriptor_set.WriteDescriptorImageInfo(0, view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_IMAGE_LAYOUT_GENERAL, 0);
    descriptor_set.WriteDescriptorImageInfo(0, view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_IMAGE_LAYOUT_GENERAL, 1);
    descriptor_set.UpdateDescriptorSets();

    // Write to image 0
    const char fsSource[] = R"glsl(
        #version 450
        layout(set=0, binding=0, rgba8) uniform image2D image_array[];
        void main() {
           imageStore(image_array[0], ivec2(1, 1), vec4(1.0, 0.5, 0.2, 1.0));
        }
    )glsl";
    const VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);
    CreatePipelineHelper gfx_pipe(*this);
    gfx_pipe.shader_stages_ = {gfx_pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    gfx_pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&descriptor_set.layout_});
    gfx_pipe.CreateGraphicsPipeline();

    // Read from image 1
    char const *cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0, rgba8) uniform image2D image_array[];
        void main() {
            vec4 data = imageLoad(image_array[1], ivec2(1, 1));
        }
    )glsl";
    CreateComputePipelineHelper cs_pipe(*this);
    cs_pipe.dsl_bindings_ = bindings;
    cs_pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    cs_pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&descriptor_set.layout_});
    cs_pipe.CreateComputePipeline();

    m_commandBuffer->begin();
    // Graphics pipeline writes
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(*m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, gfx_pipe.pipeline_);
    vk::CmdBindDescriptorSets(*m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, gfx_pipe.pipeline_layout_, 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(*m_commandBuffer, 3, 1, 0, 0);
    m_commandBuffer->EndRenderPass();
    // Compute pipeline reads
    vk::CmdBindPipeline(*m_commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, cs_pipe.pipeline_);
    vk::CmdBindDescriptorSets(*m_commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, cs_pipe.pipeline_layout_, 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDispatch(*m_commandBuffer, 1, 1, 1);
    m_commandBuffer->end();

    m_default_queue->submit(*m_commandBuffer);
    m_default_queue->wait();
}

TEST_F(PositiveSyncVal, TexelBufferArrayConstantIndexing) {
    TEST_DESCRIPTION("Access different elements of the texel buffer array using constant indices. There should be no hazards");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(InitSyncValFramework());
    VkPhysicalDeviceFeatures2 features2 = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(features2);
    if (!features2.features.fragmentStoresAndAtomics) {
        GTEST_SKIP() << "Test requires (unsupported) fragmentStoresAndAtomics";
    }
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.size = 1024;
    buffer_ci.usage = VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
    const vkt::Buffer buffer0(*m_device, buffer_ci);
    const vkt::Buffer buffer1(*m_device, buffer_ci);
    const vkt::BufferView buffer_view0(*m_device, vkt::BufferView::createInfo(buffer0, VK_FORMAT_R8G8B8A8_UINT));
    const vkt::BufferView buffer_view1(*m_device, vkt::BufferView::createInfo(buffer1, VK_FORMAT_R8G8B8A8_UINT));

    const OneOffDescriptorSet::Bindings bindings = {
        {0, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 2, VK_SHADER_STAGE_ALL, nullptr},
    };
    OneOffDescriptorSet descriptor_set(m_device, bindings);
    descriptor_set.WriteDescriptorBufferView(0, buffer_view0, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 0);
    descriptor_set.WriteDescriptorBufferView(0, buffer_view1, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1);
    descriptor_set.UpdateDescriptorSets();

    // Write to texel buffer 0
    const char fsSource[] = R"glsl(
        #version 450
        layout(set=0, binding=0, rgba8ui) uniform uimageBuffer texel_buffer_array[];
        void main() {
           imageStore(texel_buffer_array[0], 0, uvec4(1, 2, 3, 42));
        }
    )glsl";
    const VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);
    CreatePipelineHelper gfx_pipe(*this);
    gfx_pipe.shader_stages_ = {gfx_pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    gfx_pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&descriptor_set.layout_});
    gfx_pipe.CreateGraphicsPipeline();

    // Read from texel buffer 1
    char const *cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0, rgba8ui) uniform uimageBuffer texel_buffer_array[];
        void main() {
            uvec4 data = imageLoad(texel_buffer_array[1], 0);
        }
    )glsl";
    CreateComputePipelineHelper cs_pipe(*this);
    cs_pipe.dsl_bindings_ = bindings;
    cs_pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    cs_pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&descriptor_set.layout_});
    cs_pipe.CreateComputePipeline();

    m_commandBuffer->begin();
    // Graphics pipeline writes
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(*m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, gfx_pipe.pipeline_);
    vk::CmdBindDescriptorSets(*m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, gfx_pipe.pipeline_layout_, 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(*m_commandBuffer, 3, 1, 0, 0);
    m_commandBuffer->EndRenderPass();
    // Compute pipeline reads
    vk::CmdBindPipeline(*m_commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, cs_pipe.pipeline_);
    vk::CmdBindDescriptorSets(*m_commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, cs_pipe.pipeline_layout_, 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDispatch(*m_commandBuffer, 1, 1, 1);
    m_commandBuffer->end();

    m_default_queue->submit(*m_commandBuffer);
    m_default_queue->wait();
}

TEST_F(PositiveSyncVal, QSBufferCopyHazardsDisabled) {
    RETURN_IF_SKIP(InitSyncValFramework(true));  // Disable QueueSubmit validation
    RETURN_IF_SKIP(InitState());

    QSTestContext test(m_device, m_device->graphics_queues()[0]);
    if (!test.Valid()) {
        GTEST_SKIP() << "Test requires a valid queue object.";
    }

    test.RecordCopy(test.cba, test.buffer_a, test.buffer_b);
    test.RecordCopy(test.cbb, test.buffer_c, test.buffer_a);

    VkSubmitInfo submit1 = vku::InitStructHelper();
    submit1.commandBufferCount = 2;
    VkCommandBuffer two_cbs[2] = {test.h_cba, test.h_cbb};
    submit1.pCommandBuffers = two_cbs;

    // This should be a hazard if we didn't disable it at InitSyncValFramework time
    vk::QueueSubmit(test.q0, 1, &submit1, VK_NULL_HANDLE);

    test.DeviceWait();
}

TEST_F(PositiveSyncVal, QSTransitionWithSrcNoneStage) {
    TEST_DESCRIPTION(
        "Two submission batches synchronized with binary semaphore. Layout transition in the second batch should not interfere "
        "with image read in the previous batch.");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    VkPhysicalDeviceSynchronization2Features sync2_features = vku::InitStructHelper();
    sync2_features.synchronization2 = VK_TRUE;
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState(nullptr, &sync2_features));

    vkt::Image image(*m_device, 64, 64, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView view = image.CreateView();

    vkt::Semaphore semaphore(*m_device);

    // Submit 0: shader reads image data (read access)
    const OneOffDescriptorSet::Bindings bindings = {{0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr}};
    OneOffDescriptorSet descriptor_set(m_device, bindings);
    descriptor_set.WriteDescriptorImageInfo(0, view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_IMAGE_LAYOUT_GENERAL);
    descriptor_set.UpdateDescriptorSets();

    char const *cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0, rgba8) uniform image2D image;
        void main() {
            vec4 data = imageLoad(image, ivec2(1, 1));
        }
    )glsl";
    CreateComputePipelineHelper cs_pipe(*this);
    cs_pipe.dsl_bindings_ = bindings;
    cs_pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    cs_pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&descriptor_set.layout_});
    cs_pipe.CreateComputePipeline();

    vkt::CommandBuffer cb(*m_device, m_commandPool);
    cb.begin();
    vk::CmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_COMPUTE, cs_pipe.pipeline_);
    vk::CmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_COMPUTE, cs_pipe.pipeline_layout_, 0, 1, &descriptor_set.set_, 0, nullptr);
    vk::CmdDispatch(cb, 1, 1, 1);
    cb.end();

    VkCommandBufferSubmitInfo cbuf_info = vku::InitStructHelper();
    cbuf_info.commandBuffer = cb;
    VkSemaphoreSubmitInfo signal_info = vku::InitStructHelper();
    signal_info.semaphore = semaphore;
    signal_info.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    VkSubmitInfo2 submit = vku::InitStructHelper();
    submit.commandBufferInfoCount = 1;
    submit.pCommandBufferInfos = &cbuf_info;
    submit.signalSemaphoreInfoCount = 1;
    submit.pSignalSemaphoreInfos = &signal_info;
    vk::QueueSubmit2(*m_default_queue, 1, &submit, VK_NULL_HANDLE);

    // Submit 1: transition image layout (write access)
    VkImageMemoryBarrier2 layout_transition = vku::InitStructHelper();
    // NOTE: this test check that using NONE as source stage for transition works correctly.
    // Wait on ALL_COMMANDS semaphore should protect this submission from previous accesses.
    layout_transition.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
    layout_transition.srcAccessMask = 0;
    layout_transition.dstAccessMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    layout_transition.dstAccessMask = 0;
    layout_transition.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    layout_transition.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    layout_transition.image = image;
    layout_transition.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    VkDependencyInfoKHR dep_info = vku::InitStructHelper();
    dep_info.imageMemoryBarrierCount = 1;
    dep_info.pImageMemoryBarriers = &layout_transition;

    vkt::CommandBuffer cb2(*m_device, m_commandPool);
    cb2.begin();
    vk::CmdPipelineBarrier2(cb2, &dep_info);
    cb2.end();

    VkCommandBufferSubmitInfo cbuf_info2 = vku::InitStructHelper();
    cbuf_info2.commandBuffer = cb2;
    VkSemaphoreSubmitInfo wait_info2 = vku::InitStructHelper();
    wait_info2.semaphore = semaphore;
    wait_info2.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    VkSubmitInfo2 submit2 = vku::InitStructHelper();
    submit2.waitSemaphoreInfoCount = 1;
    submit2.pWaitSemaphoreInfos = &wait_info2;
    submit2.commandBufferInfoCount = 1;
    submit2.pCommandBufferInfos = &cbuf_info2;

    vk::QueueSubmit2(*m_default_queue, 1, &submit2, VK_NULL_HANDLE);
    m_default_queue->wait();
}

TEST_F(PositiveSyncVal, QSTransitionWithSrcNoneStage2) {
    TEST_DESCRIPTION(
        "Two submission batches synchronized with binary semaphore. Layout transition in the second batch should not interfere "
        "with image write in the first batch.");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    VkPhysicalDeviceSynchronization2Features sync2_features = vku::InitStructHelper();
    sync2_features.synchronization2 = VK_TRUE;
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState(nullptr, &sync2_features));

    vkt::Image image(*m_device, 64, 64, 1, VK_FORMAT_R8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    VkImageMemoryBarrier2 layout_transition = vku::InitStructHelper();
    layout_transition.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
    layout_transition.srcAccessMask = VK_ACCESS_2_NONE;
    layout_transition.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
    layout_transition.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    layout_transition.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    layout_transition.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    layout_transition.image = image;
    layout_transition.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    VkDependencyInfo dep_info = vku::InitStructHelper();
    dep_info.imageMemoryBarrierCount = 1;
    dep_info.pImageMemoryBarriers = &layout_transition;

    vkt::Semaphore semaphore(*m_device);

    // Submit 1: Clear image (WRITE access)
    vkt::CommandBuffer cb(*m_device, m_commandPool);
    cb.begin();
    vk::CmdClearColorImage(cb, image, VK_IMAGE_LAYOUT_GENERAL, &m_clear_color, 1, &layout_transition.subresourceRange);
    cb.end();

    VkCommandBufferSubmitInfo cbuf_info = vku::InitStructHelper();
    cbuf_info.commandBuffer = cb;
    VkSemaphoreSubmitInfo signal_info = vku::InitStructHelper();
    signal_info.semaphore = semaphore;
    signal_info.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    VkSubmitInfo2 submit = vku::InitStructHelper();
    submit.commandBufferInfoCount = 1;
    submit.pCommandBufferInfos = &cbuf_info;
    submit.signalSemaphoreInfoCount = 1;
    submit.pSignalSemaphoreInfos = &signal_info;
    vk::QueueSubmit2(*m_default_queue, 1, &submit, VK_NULL_HANDLE);

    // Submit 2: Transition layout (WRITE access)
    vkt::CommandBuffer cb2(*m_device, m_commandPool);
    cb2.begin();
    vk::CmdPipelineBarrier2(cb2, &dep_info);
    cb2.end();

    VkCommandBufferSubmitInfo cbuf_info2 = vku::InitStructHelper();
    cbuf_info2.commandBuffer = cb2;
    VkSemaphoreSubmitInfo wait_info2 = vku::InitStructHelper();
    wait_info2.semaphore = semaphore;
    wait_info2.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    VkSubmitInfo2 submit2 = vku::InitStructHelper();
    submit2.waitSemaphoreInfoCount = 1;
    submit2.pWaitSemaphoreInfos = &wait_info2;
    submit2.commandBufferInfoCount = 1;
    submit2.pCommandBufferInfos = &cbuf_info2;
    vk::QueueSubmit2(*m_default_queue, 1, &submit2, VK_NULL_HANDLE);
    m_default_queue->wait();
}

TEST_F(PositiveSyncVal, QSTransitionAndRead) {
    TEST_DESCRIPTION("Transition and read image in different submits synchronized via ALL_COMMANDS semaphore");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    VkPhysicalDeviceSynchronization2Features sync2_features = vku::InitStructHelper();
    sync2_features.synchronization2 = VK_TRUE;
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState(nullptr, &sync2_features));

    vkt::Image image(*m_device, 64, 64, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_STORAGE_BIT);
    vkt::ImageView view = image.CreateView();

    // Submit0: transition image and signal semaphore with ALL_COMMANDS scope
    VkImageMemoryBarrier2 layout_transition = vku::InitStructHelper();
    layout_transition.srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
    layout_transition.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    layout_transition.dstAccessMask = VK_PIPELINE_STAGE_2_NONE;
    layout_transition.dstAccessMask = 0;
    layout_transition.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    layout_transition.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    layout_transition.image = image;
    layout_transition.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    VkDependencyInfoKHR dep_info = vku::InitStructHelper();
    dep_info.imageMemoryBarrierCount = 1;
    dep_info.pImageMemoryBarriers = &layout_transition;

    vkt::CommandBuffer cb(*m_device, m_commandPool);
    cb.begin();
    vk::CmdPipelineBarrier2(cb, &dep_info);
    cb.end();

    vkt::Semaphore semaphore(*m_device);

    VkCommandBufferSubmitInfo cbuf_info = vku::InitStructHelper();
    cbuf_info.commandBuffer = cb;
    VkSemaphoreSubmitInfo signal_info = vku::InitStructHelper();
    signal_info.semaphore = semaphore;
    signal_info.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    VkSubmitInfo2 submit = vku::InitStructHelper();
    submit.commandBufferInfoCount = 1;
    submit.pCommandBufferInfos = &cbuf_info;
    submit.signalSemaphoreInfoCount = 1;
    submit.pSignalSemaphoreInfos = &signal_info;
    vk::QueueSubmit2(*m_default_queue, 1, &submit, VK_NULL_HANDLE);

    // Submit1: wait for the semaphore and read image in the shader
    const OneOffDescriptorSet::Bindings bindings = {{0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr}};
    OneOffDescriptorSet descriptor_set(m_device, bindings);
    descriptor_set.WriteDescriptorImageInfo(0, view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_IMAGE_LAYOUT_GENERAL);
    descriptor_set.UpdateDescriptorSets();

    char const *cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0, rgba8) uniform image2D image;
        void main() {
            vec4 data = imageLoad(image, ivec2(1, 1));
        }
    )glsl";
    CreateComputePipelineHelper cs_pipe(*this);
    cs_pipe.dsl_bindings_ = bindings;
    cs_pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    cs_pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&descriptor_set.layout_});
    cs_pipe.CreateComputePipeline();

    vkt::CommandBuffer cb2(*m_device, m_commandPool);
    cb2.begin();
    vk::CmdBindPipeline(cb2, VK_PIPELINE_BIND_POINT_COMPUTE, cs_pipe.pipeline_);
    vk::CmdBindDescriptorSets(cb2, VK_PIPELINE_BIND_POINT_COMPUTE, cs_pipe.pipeline_layout_, 0, 1, &descriptor_set.set_, 0,
                              nullptr);
    vk::CmdDispatch(cb2, 1, 1, 1);
    cb2.end();

    VkCommandBufferSubmitInfo cbuf_info2 = vku::InitStructHelper();
    cbuf_info2.commandBuffer = cb2;
    VkSemaphoreSubmitInfo wait_info2 = vku::InitStructHelper();
    wait_info2.semaphore = semaphore;
    wait_info2.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    VkSubmitInfo2 submit2 = vku::InitStructHelper();
    submit2.waitSemaphoreInfoCount = 1;
    submit2.pWaitSemaphoreInfos = &wait_info2;
    submit2.commandBufferInfoCount = 1;
    submit2.pCommandBufferInfos = &cbuf_info2;
    vk::QueueSubmit2(*m_default_queue, 1, &submit2, VK_NULL_HANDLE);
    m_default_queue->wait();
}

TEST_F(PositiveSyncVal, DynamicRenderingColorResolve) {
    TEST_DESCRIPTION("Test color resolve with dynamic rendering");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    VkPhysicalDeviceSynchronization2Features sync2_features = vku::InitStructHelper();
    sync2_features.synchronization2 = VK_TRUE;
    VkPhysicalDeviceDynamicRenderingFeatures dynamic_rendering_features = vku::InitStructHelper(&sync2_features);
    dynamic_rendering_features.dynamicRendering = VK_TRUE;
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState(nullptr, &dynamic_rendering_features));

    const uint32_t width = 64;
    const uint32_t height = 64;
    const VkFormat color_format = VK_FORMAT_R8G8B8A8_UNORM;
    const VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    auto color_ci = vkt::Image::ImageCreateInfo2D(width, height, 1, 1, color_format, usage);
    color_ci.samples = VK_SAMPLE_COUNT_4_BIT;  // guaranteed by framebufferColorSampleCounts
    vkt::Image color_image(*m_device, color_ci, vkt::set_layout);
    color_image.SetLayout(VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL);
    vkt::ImageView color_image_view = color_image.CreateView(VK_IMAGE_ASPECT_COLOR_BIT);

    auto color_resolved_ci = vkt::Image::ImageCreateInfo2D(width, height, 1, 1, color_format, usage);
    vkt::Image color_resolved_image(*m_device, color_resolved_ci, vkt::set_layout);
    color_resolved_image.SetLayout(VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL);
    vkt::ImageView color_resolved_image_view = color_resolved_image.CreateView(VK_IMAGE_ASPECT_COLOR_BIT);

    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageView = color_image_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
    color_attachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
    color_attachment.resolveImageView = color_resolved_image_view;
    color_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.clearValue.color = m_clear_color;

    VkRenderingInfo rendering_info = vku::InitStructHelper();
    rendering_info.renderArea.extent = {width, height};
    rendering_info.layerCount = 1;
    rendering_info.colorAttachmentCount = 1;
    rendering_info.pColorAttachments = &color_attachment;

    m_commandBuffer->begin();
    vk::CmdBeginRendering(*m_commandBuffer, &rendering_info);
    vk::CmdEndRendering(*m_commandBuffer);
    m_commandBuffer->end();
}

TEST_F(PositiveSyncVal, DynamicRenderingDepthResolve) {
    TEST_DESCRIPTION("Test depth resolve with dynamic rendering");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    VkPhysicalDeviceSynchronization2Features sync2_features = vku::InitStructHelper();
    sync2_features.synchronization2 = VK_TRUE;
    VkPhysicalDeviceDynamicRenderingFeatures dynamic_rendering_features = vku::InitStructHelper(&sync2_features);
    dynamic_rendering_features.dynamicRendering = VK_TRUE;
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState(nullptr, &dynamic_rendering_features));

    VkPhysicalDeviceDepthStencilResolveProperties resolve_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(resolve_properties);
    if ((resolve_properties.supportedDepthResolveModes & VK_RESOLVE_MODE_MIN_BIT) == 0) {
        GTEST_SKIP() << "VK_RESOLVE_MODE_MIN_BIT not supported";
    }

    const uint32_t width = 64;
    const uint32_t height = 64;
    const VkFormat depth_format = FindSupportedDepthOnlyFormat(gpu());
    const VkImageUsageFlags usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    auto depth_ci = vkt::Image::ImageCreateInfo2D(width, height, 1, 1, depth_format, usage);
    depth_ci.samples = VK_SAMPLE_COUNT_4_BIT;  // guaranteed by framebufferDepthSampleCounts
    vkt::Image depth_image(*m_device, depth_ci, vkt::set_layout);
    depth_image.SetLayout(VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL);
    vkt::ImageView depth_image_view = depth_image.CreateView(VK_IMAGE_ASPECT_DEPTH_BIT);

    auto depth_resolved_ci = vkt::Image::ImageCreateInfo2D(width, height, 1, 1, depth_format, usage);
    vkt::Image depth_resolved_image(*m_device, depth_resolved_ci, vkt::set_layout);
    depth_resolved_image.SetLayout(VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL);
    vkt::ImageView depth_resolved_image_view = depth_resolved_image.CreateView(VK_IMAGE_ASPECT_DEPTH_BIT);

    VkRenderingAttachmentInfo depth_attachment = vku::InitStructHelper();
    depth_attachment.imageView = depth_image_view;
    depth_attachment.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
    depth_attachment.resolveMode = VK_RESOLVE_MODE_MIN_BIT;
    depth_attachment.resolveImageView = depth_resolved_image_view;
    depth_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
    depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depth_attachment.clearValue.depthStencil.depth = 1.0f;

    VkRenderingInfo rendering_info = vku::InitStructHelper();
    rendering_info.renderArea.extent = {width, height};
    rendering_info.layerCount = 1;
    rendering_info.pDepthAttachment = &depth_attachment;

    m_commandBuffer->begin();
    vk::CmdBeginRendering(*m_commandBuffer, &rendering_info);
    vk::CmdEndRendering(*m_commandBuffer);
    m_commandBuffer->end();
}

TEST_F(PositiveSyncVal, FillBuffer) {
    TEST_DESCRIPTION("Synchronize with vkCmdFillBuffer assuming that its writes happen on the CLEAR stage");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    VkPhysicalDeviceSynchronization2Features sync2_features = vku::InitStructHelper();
    sync2_features.synchronization2 = VK_TRUE;
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState(nullptr, &sync2_features));

    constexpr VkDeviceSize size = 1024;
    vkt::Buffer src_buffer(*m_device, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    vkt::Buffer dst_buffer(*m_device, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    VkBufferCopy region{};
    region.size = size;

    VkBufferMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_CLEAR_BIT;
    barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
    barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
    barrier.buffer = src_buffer;
    barrier.size = size;

    VkDependencyInfo dep_info = vku::InitStructHelper();
    dep_info.bufferMemoryBarrierCount = 1;
    dep_info.pBufferMemoryBarriers = &barrier;

    m_commandBuffer->begin();
    vk::CmdFillBuffer(*m_commandBuffer, src_buffer, 0, size, 42);
    vk::CmdPipelineBarrier2(*m_commandBuffer, &dep_info);
    vk::CmdCopyBuffer(*m_commandBuffer, src_buffer, dst_buffer, 1, &region);
    m_commandBuffer->end();
}

TEST_F(PositiveSyncVal, UpdateBuffer) {
    TEST_DESCRIPTION("Synchronize with vkCmdUpdateBuffer assuming that its writes happen on the CLEAR stage");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    VkPhysicalDeviceSynchronization2Features sync2_features = vku::InitStructHelper();
    sync2_features.synchronization2 = VK_TRUE;
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState(nullptr, &sync2_features));

    constexpr VkDeviceSize size = 64;
    vkt::Buffer src_buffer(*m_device, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    vkt::Buffer dst_buffer(*m_device, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    std::array<uint8_t, size> data = {};

    VkBufferCopy region{};
    region.size = size;

    VkBufferMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_CLEAR_BIT;
    barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
    barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
    barrier.buffer = src_buffer;
    barrier.size = size;

    VkDependencyInfo dep_info = vku::InitStructHelper();
    dep_info.bufferMemoryBarrierCount = 1;
    dep_info.pBufferMemoryBarriers = &barrier;

    m_commandBuffer->begin();
    vk::CmdUpdateBuffer(*m_commandBuffer, src_buffer, 0, static_cast<VkDeviceSize>(data.size()), data.data());
    vk::CmdPipelineBarrier2(*m_commandBuffer, &dep_info);
    vk::CmdCopyBuffer(*m_commandBuffer, src_buffer, dst_buffer, 1, &region);
    m_commandBuffer->end();
}

TEST_F(PositiveSyncVal, QSSynchronizedWritesAndAsyncWait) {
    TEST_DESCRIPTION(
        "Graphics queue: Image transition and subsequent image write are synchronized using a pipeline barrier. Transfer queue: "
        "waits for the image layout transition using a semaphore. This should not affect pipeline barrier on the graphics queue");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    VkPhysicalDeviceSynchronization2Features sync2_features = vku::InitStructHelper();
    sync2_features.synchronization2 = VK_TRUE;
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState(nullptr, &sync2_features));

    const std::optional<uint32_t> transfer_family =
        m_device->QueueFamilyMatching(VK_QUEUE_TRANSFER_BIT, (VK_QUEUE_COMPUTE_BIT | VK_QUEUE_GRAPHICS_BIT));
    if (!transfer_family) {
        GTEST_SKIP() << "Transfer-only queue family is not present";
    }
    vkt::Queue *transfer_queue = m_device->queue_family_queues(transfer_family.value())[0].get();

    vkt::Image image(*m_device, 64, 64, 1, VK_FORMAT_R8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::Buffer buffer(*m_device, 64 * 64, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

    VkBufferImageCopy region = {};
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region.imageExtent = {64, 64, 1};

    vkt::Semaphore semaphore(*m_device);

    // Submit 0: perform image layout transition on Graphics queue.
    // Image barrier synchronizes with COPY-WRITE access from Submit 2.
    vkt::CommandBuffer cb0(*m_device, m_commandPool);
    cb0.begin();
    VkImageMemoryBarrier2 image_barrier = vku::InitStructHelper();
    image_barrier.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
    image_barrier.srcAccessMask = VK_ACCESS_2_NONE;
    image_barrier.dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
    image_barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT_KHR;
    image_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    image_barrier.image = image;
    image_barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    VkDependencyInfo dep_info = vku::InitStructHelper();
    dep_info.imageMemoryBarrierCount = 1;
    dep_info.pImageMemoryBarriers = &image_barrier;
    vk::CmdPipelineBarrier2(cb0, &dep_info);
    cb0.end();

    VkCommandBufferSubmitInfo cbuf_info0 = vku::InitStructHelper();
    cbuf_info0.commandBuffer = cb0;
    VkSemaphoreSubmitInfo signal_info0 = vku::InitStructHelper();
    signal_info0.semaphore = semaphore;
    signal_info0.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    VkSubmitInfo2 submit0 = vku::InitStructHelper();
    submit0.commandBufferInfoCount = 1;
    submit0.pCommandBufferInfos = &cbuf_info0;
    submit0.signalSemaphoreInfoCount = 1;
    submit0.pSignalSemaphoreInfos = &signal_info0;
    vk::QueueSubmit2(*m_default_queue, 1, &submit0, VK_NULL_HANDLE);

    // Submit 1: empty submit on Transfer queue that waits for Submit 0.
    VkSemaphoreSubmitInfo wait_info1 = vku::InitStructHelper();
    wait_info1.semaphore = semaphore;
    wait_info1.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    VkSubmitInfo2 submit1 = vku::InitStructHelper();
    submit1.waitSemaphoreInfoCount = 1;
    submit1.pWaitSemaphoreInfos = &wait_info1;
    vk::QueueSubmit2(*transfer_queue, 1, &submit1, VK_NULL_HANDLE);

    // Submit 2: copy to image on Graphics queue. No synchronization is needed because of COPY+WRITE barrier from Submit 0.
    vkt::CommandBuffer cb2(*m_device, m_commandPool);
    cb2.begin();
    vk::CmdCopyBufferToImage(cb2, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    cb2.end();

    VkCommandBufferSubmitInfo cbuf_info2 = vku::InitStructHelper();
    cbuf_info2.commandBuffer = cb2;
    VkSubmitInfo2 submit2 = vku::InitStructHelper();
    submit2.commandBufferInfoCount = 1;
    submit2.pCommandBufferInfos = &cbuf_info2;
    vk::QueueSubmit2(*m_default_queue, 1, &submit2, VK_NULL_HANDLE);

    m_default_queue->wait();
    transfer_queue->wait();
}

// TODO:
// It has to be this test found a bug in the existing code, so it's disabled until it's fixed.
// Draw access happen-after loadOp access so it's enough to synchronize with FRAGMENT_SHADER read.
// last_reads contains both loadOp read access and draw (fragment shader) read access. The code
// synchronizes only with draw (FRAGMENT_SHADER) but not with loadOp (COLOR_ATTACHMENT_OUTPUT),
// and the syncval complains that COLOR_ATTACHMENT_OUTPUT access is not synchronized.
TEST_F(PositiveSyncVal, DISABLED_RenderPassStoreOpNone) {
    TEST_DESCRIPTION("Synchronization with draw command when render pass uses storeOp=NONE");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_LOAD_STORE_OP_NONE_EXTENSION_NAME);
    VkPhysicalDeviceSynchronization2Features sync2_features = vku::InitStructHelper();
    sync2_features.synchronization2 = VK_TRUE;
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState(nullptr, &sync2_features));

    const VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    const VkImageLayout input_attachment_layout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;

    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(format, input_attachment_layout, input_attachment_layout, VK_ATTACHMENT_LOAD_OP_LOAD,
                                VK_ATTACHMENT_STORE_OP_NONE);
    rp.AddAttachmentReference({0, input_attachment_layout});
    rp.AddInputAttachment(0);
    rp.CreateRenderPass();

    vkt::Image image(*m_device, 32, 32, 1, format, VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);
    image.SetLayout(input_attachment_layout);
    vkt::ImageView image_view = image.CreateView();
    vkt::Framebuffer fb(*m_device, rp.Handle(), 1, &image_view.handle());

    VkImageMemoryBarrier2 layout_transition = vku::InitStructHelper();
    // Form an execution dependency with draw command (FRAGMENT_SHADER). Execution dependency is enough to sync with READ.
    layout_transition.srcStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR;
    layout_transition.srcAccessMask = VK_ACCESS_2_NONE;
    layout_transition.dstStageMask = VK_PIPELINE_STAGE_2_NONE;
    layout_transition.dstAccessMask = VK_ACCESS_2_NONE;
    layout_transition.oldLayout = input_attachment_layout;
    layout_transition.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    layout_transition.image = image;
    layout_transition.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    VkDependencyInfo dep_info = vku::InitStructHelper();
    dep_info.imageMemoryBarrierCount = 1;
    dep_info.pImageMemoryBarriers = &layout_transition;

    // Fragment shader READs input attachment.
    VkShaderObj fs(this, kFragmentSubpassLoadGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);
    const VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    OneOffDescriptorSet descriptor_set(m_device, {binding});
    descriptor_set.WriteDescriptorImageInfo(0, image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
                                            input_attachment_layout);
    descriptor_set.UpdateDescriptorSets();
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_[1] = fs.GetStageCreateInfo();
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.gp_ci_.renderPass = rp.Handle();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(rp.Handle(), fb);
    vk::CmdBindPipeline(*m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(*m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &descriptor_set.set_, 0,
                              nullptr);
    vk::CmdDraw(*m_commandBuffer, 1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();

    // This waits for the FRAGMENT_SHADER read before starting with transition.
    // If storeOp other than NONE was used we had to wait for it instead.
    vk::CmdPipelineBarrier2(*m_commandBuffer, &dep_info);
    m_commandBuffer->end();
}
