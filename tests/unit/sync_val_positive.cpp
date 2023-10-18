/*
 * Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (c) 2015-2023 Google, Inc.
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
#include "../framework/thread_timeout_helper.h"

class PositiveSyncVal : public VkSyncValTest {};

void VkSyncValTest::InitSyncValFramework(bool enable_queue_submit_validation) {
    // Enable synchronization validation

    // Optional feature definition, add if requested (but they can't be defined at the conditional scope)
    const char *kEnableQueuSubmitSyncValidation[] = {"VALIDATION_CHECK_ENABLE_SYNCHRONIZATION_VALIDATION_QUEUE_SUBMIT"};
    const VkLayerSettingEXT settings[] = {
        {OBJECT_LAYER_NAME, "enables", VK_LAYER_SETTING_TYPE_STRING_EXT, 1, kEnableQueuSubmitSyncValidation}};
    const VkLayerSettingsCreateInfoEXT qs_settings{VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr,
                                                   static_cast<uint32_t>(std::size(settings)), settings};

    if (enable_queue_submit_validation) {
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

    VkImageObj image(m_device);
    image.InitNoLayout(width, height, 1, rt_format, transfer_usage, VK_IMAGE_TILING_OPTIMAL);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    VkImageObj rt(m_device);
    rt.InitNoLayout(VkImageObj::ImageCreateInfo2D(width, height, 1, layers, rt_format, rt_usage, VK_IMAGE_TILING_OPTIMAL));
    rt.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    auto attachment_without_load_store = [](VkFormat format) {
        VkAttachmentDescription attachment = {};
        attachment.format = format;
        attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        attachment.loadOp = VK_ATTACHMENT_LOAD_OP_NONE_EXT;
        attachment.storeOp = VK_ATTACHMENT_STORE_OP_NONE_EXT;
        attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_NONE_EXT;
        attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_NONE_EXT;
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

    VkFramebufferCreateInfo fbci = vku::InitStructHelper();
    fbci.flags = 0;
    fbci.renderPass = render_pass;
    fbci.attachmentCount = 1;
    fbci.pAttachments = &rt.targetView(rt_format, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, layers, VK_IMAGE_VIEW_TYPE_2D_ARRAY);
    fbci.width = width;
    fbci.height = height;
    fbci.layers = layers;
    vkt::Framebuffer framebuffer(*m_device, fbci);

    VkRenderPassBeginInfo rpbi = vku::InitStructHelper();
    rpbi.framebuffer = framebuffer;
    rpbi.renderPass = render_pass;
    rpbi.renderArea.extent.width = width;
    rpbi.renderArea.extent.height = height;

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.renderPass = render_pass;
    pipe.InitState();
    ASSERT_EQ(VK_SUCCESS, pipe.CreateGraphicsPipeline());

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
    vk::CmdBeginRenderPass(*m_commandBuffer, &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    // Write 2: Clear render target's layer 1
    vk::CmdClearAttachments(*m_commandBuffer, 1, &clear_attachment, 1, &clear_rect);
    vk::CmdEndRenderPass(*m_commandBuffer);
    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer();
    vk::QueueWaitIdle(m_default_queue);
}

// Image transition ensures that image data is made visible and available when necessary.
// The user's responsibility is only to properly define the barrier. This test checks that
// writing to the image immediately after the transition  does not produce WAW hazard against
// the writes performed by the transition.
TEST_F(PositiveSyncVal, WriteToImageAfterTransition) {
    TEST_DESCRIPTION("Perform image transition then copy to image from buffer");
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState())

    constexpr uint32_t width = 256;
    constexpr uint32_t height = 128;
    constexpr VkFormat format = VK_FORMAT_B8G8R8A8_UNORM;

    vkt::Buffer buffer(*m_device, width * height * 4, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    VkImageObj image(m_device);
    image.InitNoLayout(width, height, 1, format, VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_TILING_OPTIMAL);

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
    if (IsPlatformMockICD()) {
        // Mock does not support proper ordering of events, e.g. wait can return before signal
        GTEST_SKIP() << "Test not supported by MockICD";
    }
    VkPhysicalDeviceTimelineSemaphoreFeatures timeline_semaphore_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(timeline_semaphore_features);
    RETURN_IF_SKIP(InitState(nullptr, &timeline_semaphore_features));

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
    if (IsPlatformMockICD()) {
        // Mock does not support precise semaphore counter reporting
        GTEST_SKIP() << "Test not supported by MockICD";
    }
    VkPhysicalDeviceTimelineSemaphoreFeatures timeline_semaphore_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(timeline_semaphore_features);
    RETURN_IF_SKIP(InitState(nullptr, &timeline_semaphore_features));

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
    if (IsPlatformMockICD()) {
        // Mock does not support precise semaphore counter reporting
        GTEST_SKIP() << "Test not supported by MockICD";
    }
    VkPhysicalDeviceTimelineSemaphoreFeatures timeline_semaphore_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(timeline_semaphore_features);
    RETURN_IF_SKIP(InitState(nullptr, &timeline_semaphore_features));

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
    RETURN_IF_SKIP(InitState())
    InitRenderTarget();

    const char vs_source[] = R"glsl(
        #version 460
        layout(set=1) layout(binding=0) uniform foo { float x; } bar;
        void main() {
           gl_Position = vec4(bar.x);
        }
    )glsl";
    VkShaderObj vs(this, vs_source, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, kFragmentColorOutputGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    const VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr};
    const vkt::DescriptorSetLayout set_layout(*m_device, {binding});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&set_layout, &set_layout});
    OneOffDescriptorSet set(m_device, {binding});

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
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
    RETURN_IF_SKIP(InitSyncValFramework(true));
    VkPhysicalDeviceSynchronization2FeaturesKHR sync2_features = vku::InitStructHelper();
    sync2_features.synchronization2 = VK_TRUE;
    RETURN_IF_SKIP(InitState(nullptr, &sync2_features));
    if (!InitSwapchain()) {
        GTEST_SKIP() << "Cannot create surface or swapchain";
    }
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
    ASSERT_EQ(VK_SUCCESS, vk::QueueSubmit2(m_default_queue, 1, &submit, VK_NULL_HANDLE));

    VkPresentInfoKHR present = vku::InitStructHelper();
    present.waitSemaphoreCount = 1;
    present.pWaitSemaphores = &submit_semaphore.handle();
    present.swapchainCount = 1;
    present.pSwapchains = &m_swapchain;
    present.pImageIndices = &image_index;
    ASSERT_EQ(VK_SUCCESS, vk::QueuePresentKHR(m_default_queue, &present));
    ASSERT_EQ(VK_SUCCESS, vk::QueueWaitIdle(m_default_queue));
}

TEST_F(PositiveSyncVal, PresentAfterSubmitAutomaticVisibility) {
    TEST_DESCRIPTION("Waiting on the semaphore makes available image accesses visible to the presentation engine.");
    AddSurfaceExtension();
    RETURN_IF_SKIP(InitSyncValFramework(true));
    RETURN_IF_SKIP(InitState())
    if (!InitSwapchain()) {
        GTEST_SKIP() << "Cannot create surface or swapchain";
    }
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
    ASSERT_EQ(VK_SUCCESS, vk::QueueSubmit(m_default_queue, 1, &submit, VK_NULL_HANDLE));

    VkPresentInfoKHR present = vku::InitStructHelper();
    present.waitSemaphoreCount = 1;
    present.pWaitSemaphores = &submit_semaphore.handle();
    present.swapchainCount = 1;
    present.pSwapchains = &m_swapchain;
    present.pImageIndices = &image_index;
    ASSERT_EQ(VK_SUCCESS, vk::QueuePresentKHR(m_default_queue, &present));
    ASSERT_EQ(VK_SUCCESS, vk::QueueWaitIdle(m_default_queue));
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

    VkImageObj image(m_device);
    image.Init(64, 64, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
               VK_IMAGE_TILING_LINEAR);
    ASSERT_TRUE(image.initialized());
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

    VkImageObj images[4] = {m_device, m_device, m_device, m_device};
    VkImageView views[4] = {};
    for (int i = 0; i < 4; i++) {
        images[i].Init(64, 64, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_STORAGE_BIT, VK_IMAGE_TILING_OPTIMAL);
        ASSERT_TRUE(images[i].initialized());
        images[i].SetLayout(image_layout);
        views[i] = images[i].targetView(VK_FORMAT_R8G8B8A8_UNORM);
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

    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_default_queue);
}

TEST_F(PositiveSyncVal, ImageArrayConstantIndexing) {
    TEST_DESCRIPTION("Access different elements of the image array using constant indices. There should be no hazards");
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    VkImageObj image(m_device);
    image.Init(64, 64, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_STORAGE_BIT, VK_IMAGE_TILING_OPTIMAL);
    ASSERT_TRUE(image.initialized());
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    const VkImageView view = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

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

    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_default_queue);
}

TEST_F(PositiveSyncVal, TexelBufferArrayConstantIndexing) {
    TEST_DESCRIPTION("Access different elements of the texel buffer array using constant indices. There should be no hazards");
    RETURN_IF_SKIP(InitSyncValFramework());
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

    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_default_queue);
}
