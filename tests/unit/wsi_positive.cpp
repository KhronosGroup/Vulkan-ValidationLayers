/*
 * Copyright (c) 2023-2025 Valve Corporation
 * Copyright (c) 2023-2025 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */
#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"

std::optional<VkPhysicalDeviceGroupProperties> WsiTest::FindPhysicalDeviceGroup() {
    uint32_t physical_device_group_count = 0;
    vk::EnumeratePhysicalDeviceGroups(instance(), &physical_device_group_count, nullptr);
    if (physical_device_group_count == 0) {
        return {};
    }
    std::vector<VkPhysicalDeviceGroupProperties> physical_device_groups(physical_device_group_count,
                                                                        {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES});
    vk::EnumeratePhysicalDeviceGroups(instance(), &physical_device_group_count, physical_device_groups.data());
    for (const auto &physical_device_group : physical_device_groups) {
        for (uint32_t k = 0; k < physical_device_group.physicalDeviceCount; k++) {
            if (physical_device_group.physicalDevices[k] == Gpu()) {
                return physical_device_group;
            }
        }
    }
    return {};
}

class PositiveWsi : public WsiTest {};

TEST_F(PositiveWsi, CreateWaylandSurface) {
    TEST_DESCRIPTION("Test creating wayland surface");

#ifndef VK_USE_PLATFORM_WAYLAND_KHR
    GTEST_SKIP() << "test not supported on platform";
#else
    AddSurfaceExtension();
    AddRequiredExtensions(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    WaylandContext wayland_ctx;
    if (!wayland_ctx.Init()) {
        GTEST_SKIP() << "Failed to create wayland context.";
    }

    VkWaylandSurfaceCreateInfoKHR surface_create_info = vku::InitStructHelper();
    surface_create_info.display = wayland_ctx.display;
    surface_create_info.surface = wayland_ctx.surface;

    VkSurfaceKHR vulkan_surface;
    vk::CreateWaylandSurfaceKHR(instance(), &surface_create_info, nullptr, &vulkan_surface);

    vk::DestroySurfaceKHR(instance(), vulkan_surface, nullptr);
    wayland_ctx.Release();
#endif
}

TEST_F(PositiveWsi, CreateXcbSurface) {
    TEST_DESCRIPTION("Test creating xcb surface");

#ifndef VK_USE_PLATFORM_XCB_KHR
    GTEST_SKIP() << "test not supported on platform";
#else
    AddSurfaceExtension();
    AddRequiredExtensions(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    xcb_connection_t *xcb_connection = xcb_connect(nullptr, nullptr);
    ASSERT_TRUE(xcb_connection);

    // NOTE: This is technically an invalid window! (There is no width/height)
    // But there is no robust way to check for a valid window without crashing the app.
    xcb_window_t xcb_window = xcb_generate_id(xcb_connection);
    ASSERT_TRUE(xcb_window != 0);

    VkXcbSurfaceCreateInfoKHR surface_create_info = vku::InitStructHelper();
    surface_create_info.connection = xcb_connection;
    surface_create_info.window = xcb_window;

    VkSurfaceKHR vulkan_surface{};
    vk::CreateXcbSurfaceKHR(instance(), &surface_create_info, nullptr, &vulkan_surface);

    vk::DestroySurfaceKHR(instance(), vulkan_surface, nullptr);
    xcb_destroy_window(xcb_connection, xcb_window);
    xcb_disconnect(xcb_connection);
#endif
}

TEST_F(PositiveWsi, CreateX11Surface) {
    TEST_DESCRIPTION("Test creating x11 surface");

#ifndef VK_USE_PLATFORM_XLIB_KHR
    GTEST_SKIP() << "test not supported on platform";
#else
    AddSurfaceExtension();
    AddRequiredExtensions(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    if (std::getenv("DISPLAY") == nullptr) {
        GTEST_SKIP() << "Test requires working display\n";
    }

    Display *x11_display = XOpenDisplay(nullptr);
    ASSERT_TRUE(x11_display != nullptr);

    const int screen = DefaultScreen(x11_display);

    const Window x11_window = XCreateSimpleWindow(x11_display, RootWindow(x11_display, screen), 0, 0, 128, 128, 1,
                                                  BlackPixel(x11_display, screen), WhitePixel(x11_display, screen));

    VkSurfaceKHR vulkan_surface;
    VkXlibSurfaceCreateInfoKHR surface_create_info = vku::InitStructHelper();
    surface_create_info.dpy = x11_display;
    surface_create_info.window = x11_window;
    vk::CreateXlibSurfaceKHR(instance(), &surface_create_info, nullptr, &vulkan_surface);
    vk::DestroySurfaceKHR(instance(), vulkan_surface, nullptr);

    XDestroyWindow(x11_display, x11_window);
    XCloseDisplay(x11_display);
#endif
}

#if defined(VK_USE_PLATFORM_WIN32_KHR)
TEST_F(PositiveWsi, GetPhysicalDeviceSurfaceCapabilities2KHRWithFullScreenEXT) {
    TEST_DESCRIPTION("Test vkAcquireFullScreenExclusiveModeEXT.");

    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddSurfaceExtension();
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_FULL_SCREEN_EXCLUSIVE_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    if (!IsPlatformMockICD()) {
        GTEST_SKIP() << "Only run test MockICD due to CI stability";
    }

    InitRenderTarget();
    RETURN_IF_SKIP(InitSwapchain());

    const POINT pt_zero = {0, 0};

    VkSurfaceFullScreenExclusiveWin32InfoEXT fullscreen_exclusive_win32_info = vku::InitStructHelper();
    fullscreen_exclusive_win32_info.hmonitor = MonitorFromPoint(pt_zero, MONITOR_DEFAULTTOPRIMARY);
    VkSurfaceFullScreenExclusiveInfoEXT fullscreen_exclusive_info = vku::InitStructHelper(&fullscreen_exclusive_win32_info);
    fullscreen_exclusive_info.fullScreenExclusive = VK_FULL_SCREEN_EXCLUSIVE_APPLICATION_CONTROLLED_EXT;

    VkPhysicalDeviceSurfaceInfo2KHR surface_info = vku::InitStructHelper(&fullscreen_exclusive_info);
    surface_info.surface = m_surface.Handle();

    VkSurfaceCapabilities2KHR surface_caps = vku::InitStructHelper();
    vk::GetPhysicalDeviceSurfaceCapabilities2KHR(m_device->Physical(), &surface_info, &surface_caps);
}
#endif

TEST_F(PositiveWsi, CmdCopySwapchainImage) {
    TEST_DESCRIPTION("Run vkCmdCopyImage with a swapchain image");

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    GTEST_SKIP()
        << "According to valid usage, VkBindImageMemoryInfo-memory should be NULL. But Android will crash if memory is NULL, "
           "skipping test";
#endif

    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddSurfaceExtension();
    RETURN_IF_SKIP(Init());
    InitRenderTarget();
    RETURN_IF_SKIP(InitSwapchain(VK_IMAGE_USAGE_TRANSFER_DST_BIT));

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = m_surface_formats[0].format;
    image_create_info.extent.width = m_surface_capabilities.minImageExtent.width;
    image_create_info.extent.height = m_surface_capabilities.minImageExtent.height;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkt::Image srcImage(*m_device, image_create_info, vkt::set_layout);

    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    VkImageSwapchainCreateInfoKHR image_swapchain_create_info = vku::InitStructHelper();
    image_swapchain_create_info.swapchain = m_swapchain;
    image_create_info.pNext = &image_swapchain_create_info;

    vkt::Image image_from_swapchain(*m_device, image_create_info, vkt::no_mem);

    VkBindImageMemorySwapchainInfoKHR bind_swapchain_info = vku::InitStructHelper();
    bind_swapchain_info.swapchain = m_swapchain;
    bind_swapchain_info.imageIndex = 0;

    VkBindImageMemoryInfo bind_info = vku::InitStructHelper(&bind_swapchain_info);
    bind_info.image = image_from_swapchain;
    bind_info.memory = VK_NULL_HANDLE;
    bind_info.memoryOffset = 0;

    vk::BindImageMemory2(device(), 1, &bind_info);

    VkImageCopy copy_region = {};
    copy_region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstOffset = {0, 0, 0};
    copy_region.extent = {std::min(10u, m_surface_capabilities.minImageExtent.width),
                          std::min(10u, m_surface_capabilities.minImageExtent.height), 1};

    m_command_buffer.Begin();

    vk::CmdCopyImage(m_command_buffer, srcImage, VK_IMAGE_LAYOUT_GENERAL, image_from_swapchain, VK_IMAGE_LAYOUT_GENERAL, 1,
                     &copy_region);
}

TEST_F(PositiveWsi, TransferImageToSwapchainDeviceGroup) {
    TEST_DESCRIPTION("Transfer an image to a swapchain's image  between device group");

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    GTEST_SKIP()
        << "According to valid usage, VkBindImageMemoryInfo-memory should be NULL. But Android will crash if memory is NULL, "
           "skipping test";
#endif

    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddSurfaceExtension();

    RETURN_IF_SKIP(InitFramework());

    const auto physical_device_group = FindPhysicalDeviceGroup();
    if (!physical_device_group.has_value()) {
        GTEST_SKIP() << "cannot find physical device group that contains selected physical device";
    }

    VkDeviceGroupDeviceCreateInfo create_device_pnext = vku::InitStructHelper();
    create_device_pnext.physicalDeviceCount = physical_device_group->physicalDeviceCount;
    create_device_pnext.pPhysicalDevices = physical_device_group->physicalDevices;
    RETURN_IF_SKIP(InitState(nullptr, &create_device_pnext));
    InitRenderTarget();
    RETURN_IF_SKIP(InitSwapchain(VK_IMAGE_USAGE_TRANSFER_DST_BIT));

    constexpr uint32_t test_extent_value = 10;
    if (m_surface_capabilities.minImageExtent.width < test_extent_value ||
        m_surface_capabilities.minImageExtent.height < test_extent_value) {
        GTEST_SKIP() << "minImageExtent is not large enough";
    }

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = m_surface_formats[0].format;
    image_create_info.extent.width = m_surface_capabilities.minImageExtent.width;
    image_create_info.extent.height = m_surface_capabilities.minImageExtent.height;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkt::Image src_Image(*m_device, image_create_info, vkt::set_layout);

    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    VkImageSwapchainCreateInfoKHR image_swapchain_create_info = vku::InitStructHelper();
    image_swapchain_create_info.swapchain = m_swapchain;
    image_create_info.pNext = &image_swapchain_create_info;

    vkt::Image peer_image(*m_device, image_create_info, vkt::no_mem);

    VkBindImageMemoryDeviceGroupInfo bind_devicegroup_info = vku::InitStructHelper();
    std::array<uint32_t, 1> deviceIndices = {{0}};
    bind_devicegroup_info.deviceIndexCount = static_cast<uint32_t>(deviceIndices.size());
    bind_devicegroup_info.pDeviceIndices = deviceIndices.data();
    bind_devicegroup_info.splitInstanceBindRegionCount = 0;
    bind_devicegroup_info.pSplitInstanceBindRegions = nullptr;

    VkBindImageMemorySwapchainInfoKHR bind_swapchain_info = vku::InitStructHelper(&bind_devicegroup_info);
    bind_swapchain_info.swapchain = m_swapchain;
    bind_swapchain_info.imageIndex = 0;

    VkBindImageMemoryInfo bind_info = vku::InitStructHelper(&bind_swapchain_info);
    bind_info.image = peer_image;
    bind_info.memory = VK_NULL_HANDLE;
    bind_info.memoryOffset = 0;

    vk::BindImageMemory2(device(), 1, &bind_info);
    // Can transition layout after the memory is bound
    peer_image.SetLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    const auto swapchain_images = m_swapchain.GetImages();

    vkt::Fence fence(*m_device);
    const uint32_t image_index = m_swapchain.AcquireNextImage(fence, kWaitTimeout);
    vk::WaitForFences(device(), 1, &fence.handle(), VK_TRUE, kWaitTimeout);

    m_command_buffer.Begin();

    VkImageMemoryBarrier img_barrier = vku::InitStructHelper();
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    img_barrier.image = swapchain_images[image_index];
    img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    img_barrier.subresourceRange.baseArrayLayer = 0;
    img_barrier.subresourceRange.baseMipLevel = 0;
    img_barrier.subresourceRange.layerCount = 1;
    img_barrier.subresourceRange.levelCount = 1;
    vk::CmdPipelineBarrier(m_command_buffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr,
                           0, nullptr, 1, &img_barrier);

    VkImageCopy copy_region = {};
    copy_region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstOffset = {0, 0, 0};
    copy_region.extent = {test_extent_value, test_extent_value, 1};
    vk::CmdCopyImage(m_command_buffer, src_Image, VK_IMAGE_LAYOUT_GENERAL, peer_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                     &copy_region);

    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveWsi, SwapchainAcquireImageAndPresent) {
    TEST_DESCRIPTION("Test acquiring swapchain image and then presenting it.");
    AddSurfaceExtension();
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSwapchain());

    const vkt::Semaphore acquire_semaphore(*m_device);
    const auto swapchain_images = m_swapchain.GetImages();
    const uint32_t image_index = m_swapchain.AcquireNextImage(acquire_semaphore, kWaitTimeout);
    SetPresentImageLayout(swapchain_images[image_index]);

    m_default_queue->Present(m_swapchain, image_index, acquire_semaphore);
    m_default_queue->Wait();
}

TEST_F(PositiveWsi, SwapchainAcquireImageAndWaitForFence) {
    TEST_DESCRIPTION("Test waiting on swapchain image with a fence.");
    AddSurfaceExtension();
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSwapchain());
    const auto swapchain_images = m_swapchain.GetImages();
    for (auto image : swapchain_images) {
        SetPresentImageLayout(image);
    }

    const vkt::Fence fence(*m_device);
    const uint32_t image_index = m_swapchain.AcquireNextImage(fence, kWaitTimeout);
    vk::WaitForFences(device(), 1, &fence.handle(), VK_TRUE, kWaitTimeout);
    m_default_queue->Present(m_swapchain, image_index, vkt::no_semaphore);
    m_default_queue->Wait();
}

TEST_F(PositiveWsi, WaitForAcquireFenceAndIgnoreSemaphore) {
    TEST_DESCRIPTION("Image acquire specifies both semaphore and fence to signal. Only fence is being waited on.");
    AddSurfaceExtension();
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSwapchain());
    const auto swapchain_images = m_swapchain.GetImages();
    for (auto image : swapchain_images) {
        SetPresentImageLayout(image);
    }

    // Ask image acquire operation to signal both a semaphore and a fence
    const vkt::Semaphore semaphore(*m_device);
    const vkt::Fence fence(*m_device);
    uint32_t image_index = 0;
    vk::AcquireNextImageKHR(device(), m_swapchain, kWaitTimeout, semaphore, fence, &image_index);
    vk::WaitForFences(device(), 1, &fence.handle(), VK_TRUE, kWaitTimeout);

    // Present without waiting for the semaphore. That's fine because we waited on the fence
    m_default_queue->Present(m_swapchain, image_index, vkt::no_semaphore);

    m_default_queue->Wait();
}

TEST_F(PositiveWsi, WaitForAcquireSemaphoreAndIgnoreFence) {
    TEST_DESCRIPTION("Image acquire specifies both semaphore and fence to signal. Only semaphore is being waited on.");
    AddSurfaceExtension();
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSwapchain());
    const auto swapchain_images = m_swapchain.GetImages();
    for (auto image : swapchain_images) {
        SetPresentImageLayout(image);
    }

    // Ask image acquire operation to signal both a semaphore and a fence
    const vkt::Semaphore semaphore(*m_device);
    const vkt::Fence fence(*m_device);
    uint32_t image_index = 0;
    vk::AcquireNextImageKHR(device(), m_swapchain, kWaitTimeout, semaphore, fence, &image_index);

    // Present without waiting on the fence. That's fine because present waits for the semaphore
    m_default_queue->Present(m_swapchain, image_index, semaphore);

    // NOTE: this test validates vkQueuePresentKHR.
    // At this point it's fine to wait for the fence to avoid in-use errors during test exit
    // (QueueWaitIdle does not wait for the fence signaled by the non-queue operation - AcquireNextImageKHR).
    vk::WaitForFences(device(), 1, &fence.handle(), VK_TRUE, kWaitTimeout);

    m_default_queue->Wait();
}

TEST_F(PositiveWsi, RetireSubmissionUsingAcquireFence) {
    TEST_DESCRIPTION("Acquire fence can be used to determine that submission from previous frame finished.");
    AddSurfaceExtension();
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSwapchain());
    const auto swapchain_images = m_swapchain.GetImages();
    for (auto image : swapchain_images) {
        SetPresentImageLayout(image);
    }

    std::vector<vkt::CommandBuffer> command_buffers;
    std::vector<vkt::Semaphore> submit_semaphores;
    for (size_t i = 0; i < swapchain_images.size(); i++) {
        command_buffers.emplace_back(*m_device, m_command_pool);
        submit_semaphores.emplace_back(*m_device);
    }
    const vkt::Fence acquire_fence(*m_device);

    const int frame_count = 10;
    for (int i = 0; i < frame_count; i++) {
        const uint32_t image_index = m_swapchain.AcquireNextImage(acquire_fence, kWaitTimeout);

        // 1) wait on the fence -> image was acquired
        // 2) image was acquired -> image was presented in one of the previous frames
        //    (except for the first few frames, where the image is presented for the first time)
        // 3) image was presented -> corresponding present waited on the submit semaphore
        // 4) submit semaphore was waited -> corresponding submit finished execution and signaled semaphore
        //
        // In summary: waiting on the acquire fence (with specific frame setup) means that one of the
        // previous submission has finished execution and it should be safe to re-use corresponding command buffer.
        vk::WaitForFences(device(), 1, &acquire_fence.handle(), VK_TRUE, kWaitTimeout);
        vk::ResetFences(device(), 1, &acquire_fence.handle());

        // There should not be in-use errors when we re-use command buffer that corresponds to the acquired image index.
        command_buffers[image_index].Begin();
        command_buffers[image_index].End();

        m_default_queue->Submit(command_buffers[image_index], vkt::Signal(submit_semaphores[image_index]));
        m_default_queue->Present(m_swapchain, image_index, submit_semaphores[image_index]);
    }
    m_default_queue->Wait();
}

TEST_F(PositiveWsi, RetireSubmissionUsingAcquireFence2) {
    TEST_DESCRIPTION("Test that retiring submission using acquire fence works correctly after swapchain was changed.");
    AddSurfaceExtension();
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSwapchain());
    auto swapchain_images = m_swapchain.GetImages();
    for (auto image : swapchain_images) {
        SetPresentImageLayout(image);
    }

    std::vector<vkt::CommandBuffer> command_buffers;
    std::vector<vkt::Semaphore> submit_semaphores;
    for (size_t i = 0; i < swapchain_images.size(); i++) {
        command_buffers.emplace_back(*m_device, m_command_pool);
        submit_semaphores.emplace_back(*m_device);
    }
    const vkt::Fence acquire_fence(*m_device);
    uint32_t image_index = m_swapchain.AcquireNextImage(acquire_fence, kWaitTimeout);
    vk::WaitForFences(device(), 1, &acquire_fence.handle(), VK_TRUE, kWaitTimeout);
    vk::ResetFences(device(), 1, &acquire_fence.handle());
    command_buffers[image_index].Begin();
    command_buffers[image_index].End();

    m_default_queue->Submit(command_buffers[image_index], vkt::Signal(submit_semaphores[image_index]));
    m_default_queue->Present(m_swapchain, image_index, submit_semaphores[image_index]);

    // Here the application decides to destroy swapchain (e.g. resize event)
    m_swapchain.Destroy();

    // At this point there's a pending frame we need to sync with.
    // WaitForFences(acquire_fence) logic can't be used, because swapchain was destroyed and its acquire
    // fence can't be waited on. Application can use arbitrary logic to sync with the previous frames.
    // After swapchain is re-created we can continue to use WaitForFences(acquire_fence) sync model.
    //
    // Here we just wait on the queue.
    // If this line is removed we can get in-use error when begin command buffer.
    m_default_queue->Wait();

    // Create new swapchain.
    m_swapchain = CreateSwapchain(m_surface.Handle(), VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR);

    image_index = m_swapchain.AcquireNextImage(acquire_fence, kWaitTimeout);

    vk::WaitForFences(device(), 1, &acquire_fence.handle(), VK_TRUE, kWaitTimeout);
    vk::ResetFences(device(), 1, &acquire_fence.handle());

    command_buffers[image_index].Begin();
    command_buffers[image_index].End();
    m_default_queue->Wait();
}

TEST_F(PositiveWsi, RetireSubmissionUsingAcquireFence3) {
    // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8880
    TEST_DESCRIPTION("Test that retiring submission using acquire fence works correctly when using differnt fences.");
    AddSurfaceExtension();
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSwapchain());
    const auto swapchain_images = m_swapchain.GetImages();
    for (auto image : swapchain_images) {
        SetPresentImageLayout(image);
    }

    std::vector<vkt::Fence> acquire_fences;
    vkt::Fence acquire_fence(*m_device);  // extra acquire fence

    std::vector<vkt::CommandBuffer> command_buffers;
    std::vector<vkt::Semaphore> submit_semaphores;
    for (size_t i = 0; i < swapchain_images.size(); i++) {
        acquire_fences.emplace_back(*m_device);
        command_buffers.emplace_back(*m_device, m_command_pool);
        command_buffers[i].Begin();
        command_buffers[i].End();
        submit_semaphores.emplace_back(*m_device);
    }

    const int frame_count = 10;
    for (int i = 0; i < frame_count; i++) {
        uint32_t image_index = 0;
        vk::AcquireNextImageKHR(device(), m_swapchain, kWaitTimeout, VK_NULL_HANDLE, acquire_fence, &image_index);
        acquire_fence.Wait(kWaitTimeout);
        acquire_fence.Reset();

        m_default_queue->Submit(command_buffers[image_index], vkt::Signal(submit_semaphores[image_index]));
        m_default_queue->Present(m_swapchain, image_index, submit_semaphores[image_index]);
        std::swap(acquire_fences[image_index], acquire_fence);
    }
    m_default_queue->Wait();
}

TEST_F(PositiveWsi, SwapchainImageLayout) {
    AddSurfaceExtension();
    AddRequiredExtensions(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSwapchain());
    const auto swapchain_images = m_swapchain.GetImages();
    const vkt::Fence fence(*m_device);
    uint32_t image_index = 0;
    {
        VkResult result{};
        image_index = m_swapchain.AcquireNextImage(fence, kWaitTimeout, &result);
        ASSERT_TRUE(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR);
        fence.Wait(vvl::kU32Max);
    }

    VkAttachmentDescription attach[] = {
        {0, m_surface_formats[0].format, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
    };
    VkAttachmentReference att_ref = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    VkSubpassDescription subpass = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &att_ref, nullptr, nullptr, 0, nullptr};
    VkRenderPassCreateInfo rpci = vku::InitStructHelper();
    rpci.attachmentCount = 1;
    rpci.pAttachments = attach;
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    vkt::RenderPass rp1(*m_device, rpci);
    ASSERT_TRUE(rp1.initialized());

    attach[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    vkt::RenderPass rp2(*m_device, rpci);
    ASSERT_TRUE(rp2.initialized());

    VkImageViewCreateInfo ivci = vku::InitStructHelper();
    ivci.image = swapchain_images[image_index];
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = m_surface_formats[0].format;
    ivci.components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                       VK_COMPONENT_SWIZZLE_IDENTITY};
    ivci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    vkt::ImageView view(*m_device, ivci);

    vkt::Framebuffer fb1(*m_device, rp1, 1, &view.handle(), 1, 1);
    vkt::Framebuffer fb2(*m_device, rp2, 1, &view.handle(), 1, 1);

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(rp1, fb1);
    m_command_buffer.EndRenderPass();
    m_command_buffer.BeginRenderPass(rp2, fb2);
    m_command_buffer.EndRenderPass();

    VkImageMemoryBarrier present_transition = vku::InitStructHelper();
    present_transition.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    present_transition.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    present_transition.image = swapchain_images[image_index];
    present_transition.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    vk::CmdPipelineBarrier(m_command_buffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0,
                           nullptr, 0, nullptr, 1, &present_transition);
    m_command_buffer.End();

    fence.Wait(kWaitTimeout);
    fence.Reset();
    m_default_queue->Submit(m_command_buffer, fence);
    fence.Wait(kWaitTimeout);
}

TEST_F(PositiveWsi, SwapchainPresentShared) {
    TEST_DESCRIPTION("Acquire shared presentable image and Present multiple times without failure.");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SHARED_PRESENTABLE_IMAGE_EXTENSION_NAME);
    AddSurfaceExtension();
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSurface());
    InitSwapchainInfo();

    VkBool32 supported;
    vk::GetPhysicalDeviceSurfaceSupportKHR(Gpu(), m_device->graphics_queue_node_index_, m_surface.Handle(), &supported);
    if (!supported) {
        GTEST_SKIP() << "Graphics queue does not support present";
    }

    VkPresentModeKHR shared_present_mode = m_surface_non_shared_present_mode;
    for (size_t i = 0; i < m_surface_present_modes.size(); i++) {
        const VkPresentModeKHR present_mode = m_surface_present_modes[i];
        if ((present_mode == VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR) ||
            (present_mode == VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR)) {
            shared_present_mode = present_mode;
            break;
        }
    }
    if (shared_present_mode == m_surface_non_shared_present_mode) {
        GTEST_SKIP() << "Cannot find supported shared present mode";
    }

    VkSharedPresentSurfaceCapabilitiesKHR shared_present_capabilities = vku::InitStructHelper();
    VkSurfaceCapabilities2KHR capabilities = vku::InitStructHelper(&shared_present_capabilities);
    VkPhysicalDeviceSurfaceInfo2KHR surface_info = vku::InitStructHelper();
    surface_info.surface = m_surface.Handle();
    vk::GetPhysicalDeviceSurfaceCapabilities2KHR(Gpu(), &surface_info, &capabilities);

    // This was recently added to CTS, but some drivers might not correctly advertise the flag
    if ((shared_present_capabilities.sharedPresentSupportedUsageFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) == 0) {
        GTEST_SKIP() << "Driver was suppose to support VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT";
    }

    VkSwapchainCreateInfoKHR swapchain_create_info = vku::InitStructHelper();
    swapchain_create_info.surface = m_surface.Handle();
    swapchain_create_info.minImageCount = 1;
    swapchain_create_info.imageFormat = m_surface_formats[0].format;
    swapchain_create_info.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_create_info.imageExtent = m_surface_capabilities.minImageExtent;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;  // implementations must support
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_create_info.compositeAlpha = m_surface_composite_alpha;
    swapchain_create_info.presentMode = shared_present_mode;
    swapchain_create_info.clipped = VK_FALSE;
    swapchain_create_info.oldSwapchain = 0;

    m_swapchain.Init(*m_device, swapchain_create_info);
    const auto images = m_swapchain.GetImages();

    vkt::Fence fence(*m_device);
    const uint32_t image_index = m_swapchain.AcquireNextImage(fence, kWaitTimeout);
    vk::WaitForFences(device(), 1, &fence.handle(), true, kWaitTimeout);

    SetPresentImageLayout(images[image_index]);

    m_default_queue->Present(m_swapchain, image_index, vkt::no_semaphore);

    // Presenting image multiple times is valid in the shared present mode.
    //
    // If a swapchain is created with presentMode set to either VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR or
    // VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR, a single presentable image can be acquired, referred to as a shared
    // presentable image. A shared presentable image may be concurrently accessed by the application and the presentation engine,
    // without transitioning the image’s layout after it is initially presented.
    //
    // - With VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR, the presentation engine is only required to update to the latest contents
    // of a shared presentable image after a present. The application must call vkQueuePresentKHR to guarantee an update. However,
    // the presentation engine may update from it at any time.
    for (uint32_t i = 0; i < 5; ++i) {
        m_default_queue->Present(m_swapchain, image_index, vkt::no_semaphore);
    }
}

TEST_F(PositiveWsi, CreateSurface) {
    TEST_DESCRIPTION("Create and destroy a surface without ever creating a swapchain");

    AddSurfaceExtension();
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSurface());
    DestroySwapchain();  // cleans up both surface and swapchain, if they were created
}

#if defined(VK_USE_PLATFORM_WIN32_KHR)
TEST_F(PositiveWsi, CreateSwapchainFullscreenExclusive) {
    TEST_DESCRIPTION(
        "Test creating a swapchain with VkSurfaceFullScreenExclusiveWin32InfoEXT and VK_FULL_SCREEN_EXCLUSIVE_DEFAULT_EXT");

    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddSurfaceExtension();
    AddRequiredExtensions(VK_EXT_FULL_SCREEN_EXCLUSIVE_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    if (!IsPlatformMockICD()) {
        GTEST_SKIP() << "Only run test MockICD due to CI stability";
    }

    InitRenderTarget();
    RETURN_IF_SKIP(InitSwapchain());
    VkSurfaceFullScreenExclusiveInfoEXT surface_full_screen_exlusive_info = vku::InitStructHelper();
    surface_full_screen_exlusive_info.fullScreenExclusive = VK_FULL_SCREEN_EXCLUSIVE_DEFAULT_EXT;

    VkSwapchainCreateInfoKHR swapchain_create_info = vku::InitStructHelper(&surface_full_screen_exlusive_info);
    swapchain_create_info.flags = 0;
    swapchain_create_info.surface = m_surface.Handle();
    swapchain_create_info.minImageCount = m_surface_capabilities.minImageCount;
    swapchain_create_info.imageFormat = m_surface_formats[0].format;
    swapchain_create_info.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_create_info.imageExtent = m_surface_capabilities.minImageExtent;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_create_info.compositeAlpha = m_surface_composite_alpha;
    swapchain_create_info.presentMode = m_surface_non_shared_present_mode;
    swapchain_create_info.clipped = VK_FALSE;

    vkt::Swapchain swapchain(*m_device, swapchain_create_info);
}
#endif

#if defined(VK_USE_PLATFORM_WIN32_KHR)
TEST_F(PositiveWsi, CreateSwapchainFullscreenExclusive2) {
    TEST_DESCRIPTION(
        "Test creating a swapchain with VkSurfaceFullScreenExclusiveWin32InfoEXT and "
        "VK_FULL_SCREEN_EXCLUSIVE_APPLICATION_CONTROLLED_EXT");

    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddSurfaceExtension();
    AddRequiredExtensions(VK_EXT_FULL_SCREEN_EXCLUSIVE_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    if (!IsPlatformMockICD()) {
        GTEST_SKIP() << "Only run test MockICD due to CI stability";
    }

    InitRenderTarget();
    RETURN_IF_SKIP(InitSwapchain());

    const POINT pt_zero = {0, 0};

    VkSurfaceFullScreenExclusiveWin32InfoEXT fullscreen_exclusive_win32_info = vku::InitStructHelper();
    fullscreen_exclusive_win32_info.hmonitor = MonitorFromPoint(pt_zero, MONITOR_DEFAULTTOPRIMARY);
    VkSurfaceFullScreenExclusiveInfoEXT surface_full_screen_exlusive_info = vku::InitStructHelper(&fullscreen_exclusive_win32_info);
    surface_full_screen_exlusive_info.fullScreenExclusive = VK_FULL_SCREEN_EXCLUSIVE_APPLICATION_CONTROLLED_EXT;

    VkSwapchainCreateInfoKHR swapchain_create_info = vku::InitStructHelper(&surface_full_screen_exlusive_info);
    swapchain_create_info.flags = 0;
    swapchain_create_info.surface = m_surface.Handle();
    swapchain_create_info.minImageCount = m_surface_capabilities.minImageCount;
    swapchain_create_info.imageFormat = m_surface_formats[0].format;
    swapchain_create_info.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_create_info.imageExtent = m_surface_capabilities.minImageExtent;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_create_info.compositeAlpha = m_surface_composite_alpha;
    swapchain_create_info.presentMode = m_surface_non_shared_present_mode;
    swapchain_create_info.clipped = VK_FALSE;

    vkt::Swapchain swapchain(*m_device, swapchain_create_info);
}
#endif

TEST_F(PositiveWsi, SwapchainImageFormatProps) {
    TEST_DESCRIPTION("Try using special format props on a swapchain image");

    AddSurfaceExtension();
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSwapchain());

    // HACK: I know InitSwapchain() will pick first supported format
    VkSurfaceFormatKHR format_tmp;
    {
        uint32_t format_count = 1;
        const VkResult err = vk::GetPhysicalDeviceSurfaceFormatsKHR(Gpu(), m_surface.Handle(), &format_count, &format_tmp);
        ASSERT_TRUE(err == VK_SUCCESS || err == VK_INCOMPLETE) << string_VkResult(err);
    }
    const VkFormat format = format_tmp.format;

    VkFormatProperties format_props;
    vk::GetPhysicalDeviceFormatProperties(Gpu(), format, &format_props);
    if (!(format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT)) {
        GTEST_SKIP() << "We need VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT feature";
    }

    VkAttachmentReference attach = {};
    attach.layout = VK_IMAGE_LAYOUT_GENERAL;

    VkSubpassDescription subpass = {};
    subpass.pColorAttachments = &attach;
    subpass.colorAttachmentCount = 1;

    VkAttachmentDescription attach_desc = {};
    attach_desc.format = format;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    attach_desc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;

    VkRenderPassCreateInfo rpci = vku::InitStructHelper();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach_desc;

    vkt::RenderPass render_pass(*m_device, rpci);

    VkPipelineColorBlendAttachmentState pcbas = {};
    pcbas.blendEnable = VK_TRUE;  // !!!
    pcbas.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.renderPass = render_pass;
    pipe.cb_attachments_ = pcbas;
    pipe.CreateGraphicsPipeline();

    const auto swapchain_images = m_swapchain.GetImages();
    const vkt::Fence fence(*m_device);

    uint32_t image_index;
    {
        VkResult result{};
        image_index = m_swapchain.AcquireNextImage(fence, kWaitTimeout, &result);
        ASSERT_TRUE(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR);
        fence.Wait(vvl::kU32Max);
    }

    VkImageViewCreateInfo ivci = vku::InitStructHelper();
    ivci.image = swapchain_images[image_index];
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = format;
    ivci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    vkt::ImageView image_view(*m_device, ivci);
    vkt::Framebuffer framebuffer(*m_device, render_pass, 1, &image_view.handle(), 1, 1);

    vkt::CommandBuffer cmdbuff(*m_device, m_command_pool);
    cmdbuff.Begin();
    cmdbuff.BeginRenderPass(render_pass, framebuffer);

    vk::CmdBindPipeline(cmdbuff, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
}

TEST_F(PositiveWsi, SwapchainExclusiveModeQueueFamilyPropertiesReferences) {
    TEST_DESCRIPTION("Try using special format props on a swapchain image");

    AddSurfaceExtension();
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSurface());
    InitSwapchainInfo();

    VkBool32 supported;
    vk::GetPhysicalDeviceSurfaceSupportKHR(Gpu(), m_device->graphics_queue_node_index_, m_surface.Handle(), &supported);
    if (!supported) {
        GTEST_SKIP() << "Graphics queue does not support present";
    }

    VkImageUsageFlags imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    VkSurfaceTransformFlagBitsKHR preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;

    VkSwapchainCreateInfoKHR swapchain_create_info = vku::InitStructHelper();
    swapchain_create_info.surface = m_surface.Handle();
    swapchain_create_info.minImageCount = m_surface_capabilities.minImageCount;
    swapchain_create_info.imageFormat = m_surface_formats[0].format;
    swapchain_create_info.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_create_info.imageExtent = m_surface_capabilities.minImageExtent;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = imageUsage;
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.preTransform = preTransform;
    swapchain_create_info.compositeAlpha = m_surface_composite_alpha;
    swapchain_create_info.presentMode = m_surface_non_shared_present_mode;
    swapchain_create_info.clipped = VK_FALSE;
    swapchain_create_info.oldSwapchain = 0;

    swapchain_create_info.queueFamilyIndexCount = 4094967295;  // This SHOULD get ignored
    uint32_t bogus_int = 99;
    swapchain_create_info.pQueueFamilyIndices = &bogus_int;

    vkt::Swapchain swapchain(*m_device, swapchain_create_info);
}

TEST_F(PositiveWsi, InitSwapchain) {
    TEST_DESCRIPTION("Make sure InitSwapchain is not producing anying invalid usage");
    AddSurfaceExtension();
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSwapchain());
    DestroySwapchain();
}

TEST_F(PositiveWsi, DestroySwapchainWithBoundImages) {
    TEST_DESCRIPTION("Try destroying a swapchain which has multiple images");

    AddSurfaceExtension();
    AddRequiredExtensions(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSwapchain());

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = m_surface_formats[0].format;
    image_create_info.extent.width = m_surface_capabilities.minImageExtent.width;
    image_create_info.extent.height = m_surface_capabilities.minImageExtent.height;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkImageSwapchainCreateInfoKHR image_swapchain_create_info = vku::InitStructHelper();
    image_swapchain_create_info.swapchain = m_swapchain;

    image_create_info.pNext = &image_swapchain_create_info;
    std::vector<vkt::Image> images(m_surface_capabilities.minImageCount);

    int i = 0;
    for (auto &image : images) {
        image.InitNoMemory(*m_device, image_create_info);
        VkBindImageMemorySwapchainInfoKHR bind_swapchain_info = vku::InitStructHelper();
        bind_swapchain_info.swapchain = m_swapchain;
        bind_swapchain_info.imageIndex = i++;

        VkBindImageMemoryInfo bind_info = vku::InitStructHelper(&bind_swapchain_info);
        bind_info.image = image;
        bind_info.memory = VK_NULL_HANDLE;
        bind_info.memoryOffset = 0;

        vk::BindImageMemory2KHR(device(), 1, &bind_info);
    }
}

#if !defined(VK_USE_PLATFORM_ANDROID_KHR)
// Protected swapchains are guaranteed in Android Loader
// VK_KHR_surface_protected_capabilities is needed for other platforms
// Without device to test with, blocking this test from non-Android platforms for now
TEST_F(PositiveWsi, DISABLED_ProtectedSwapchainImageColorAttachment) {
#else
TEST_F(PositiveWsi, ProtectedSwapchainImageColorAttachment) {
#endif
    TEST_DESCRIPTION(
        "Make sure images from protected swapchain are considered protected image when writing to it as a color attachment");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddSurfaceExtension();
    AddRequiredExtensions(VK_KHR_SURFACE_PROTECTED_CAPABILITIES_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());

    VkPhysicalDeviceProtectedMemoryFeatures protected_memory_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(protected_memory_features);

    if (protected_memory_features.protectedMemory == VK_FALSE) {
        GTEST_SKIP() << "protectedMemory feature not supported, skipped.";
    };

    // Turns m_command_buffer into a unprotected command buffer
    RETURN_IF_SKIP(InitState(nullptr, &protected_memory_features));
    RETURN_IF_SKIP(InitSurface());
    InitSwapchainInfo();

    // Create protected swapchain
    VkBool32 supported;
    vk::GetPhysicalDeviceSurfaceSupportKHR(Gpu(), m_device->graphics_queue_node_index_, m_surface.Handle(), &supported);
    if (!supported) {
        GTEST_SKIP() << "Graphics queue does not support present, skipping test";
    }

    VkImageUsageFlags imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    VkSurfaceTransformFlagBitsKHR preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;

    VkSwapchainCreateInfoKHR swapchain_create_info = vku::InitStructHelper();
    swapchain_create_info.flags = VK_SWAPCHAIN_CREATE_PROTECTED_BIT_KHR;
    swapchain_create_info.surface = m_surface.Handle();
    swapchain_create_info.minImageCount = m_surface_capabilities.minImageCount;
    swapchain_create_info.imageFormat = m_surface_formats[0].format;
    swapchain_create_info.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_create_info.imageExtent = m_surface_capabilities.minImageExtent;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = imageUsage;
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.preTransform = preTransform;
    swapchain_create_info.compositeAlpha = m_surface_composite_alpha;
    swapchain_create_info.presentMode = m_surface_non_shared_present_mode;
    swapchain_create_info.clipped = VK_FALSE;
    swapchain_create_info.oldSwapchain = 0;
    swapchain_create_info.queueFamilyIndexCount = 4094967295;  // This SHOULD get ignored
    uint32_t bogus_int = 99;
    swapchain_create_info.pQueueFamilyIndices = &bogus_int;
    m_swapchain.Init(*m_device, swapchain_create_info);
    ASSERT_TRUE(m_swapchain.initialized());

    // Get VkImage from swapchain which should be protected
    const auto swapchain_images = m_swapchain.GetImages();
    VkImage protected_image = swapchain_images.at(0);  // only need 1 image to test

    // Create a protected image view
    VkImageViewCreateInfo image_view_create_info = {
        VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        nullptr,
        0,
        protected_image,
        VK_IMAGE_VIEW_TYPE_2D,
        swapchain_create_info.imageFormat,
        {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
         VK_COMPONENT_SWIZZLE_IDENTITY},
        {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
    };
    vkt::ImageView image_view(*m_device, image_view_create_info);

    // A renderpass and framebuffer that contains a protected color image view
    VkAttachmentDescription attachments[1] = {{0, swapchain_create_info.imageFormat, VK_SAMPLE_COUNT_1_BIT,
                                               VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                               VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                               VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL}};
    VkAttachmentReference references[1] = {{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL}};
    VkSubpassDescription subpass = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, references, nullptr, nullptr, 0, nullptr};
    VkSubpassDependency dependency = {0,
                                      0,
                                      VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                      VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                      VK_ACCESS_SHADER_WRITE_BIT,
                                      VK_ACCESS_SHADER_WRITE_BIT,
                                      VK_DEPENDENCY_BY_REGION_BIT};
    // Use framework render pass and framebuffer so pipeline helper uses it
    VkRenderPassCreateInfo rp_info = vku::InitStructHelper();
    rp_info.attachmentCount = 1;
    rp_info.pAttachments = attachments;
    rp_info.subpassCount = 1;
    rp_info.pSubpasses = &subpass;
    rp_info.dependencyCount = 1;
    rp_info.pDependencies = &dependency;
    ASSERT_EQ(VK_SUCCESS, vk::CreateRenderPass(device(), &rp_info, nullptr, &m_renderPass));
    vkt::Framebuffer fb(*m_device, m_renderPass, 1, &image_view.handle(), swapchain_create_info.imageExtent.width,
                        swapchain_create_info.imageExtent.height);

    // basic pipeline to allow for a valid vkCmdDraw()
    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);
    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();

    // Create a protected command buffer/pool to use
    vkt::CommandPool protectedCommandPool(*m_device, m_device->graphics_queue_node_index_, VK_COMMAND_POOL_CREATE_PROTECTED_BIT);
    vkt::CommandBuffer protectedCommandBuffer(*m_device, protectedCommandPool);

    protectedCommandBuffer.Begin();
    VkRect2D render_area = {{0, 0}, swapchain_create_info.imageExtent};
    VkRenderPassBeginInfo render_pass_begin =
        vku::InitStruct<VkRenderPassBeginInfo>(nullptr, m_renderPass, fb.handle(), render_area, 0u, nullptr);
    vk::CmdBeginRenderPass(protectedCommandBuffer, &render_pass_begin, VK_SUBPASS_CONTENTS_INLINE);
    vk::CmdBindPipeline(protectedCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    // This should be valid since the framebuffer color attachment is a protected swapchain image
    vk::CmdDraw(protectedCommandBuffer, 3, 1, 0, 0);
    vk::CmdEndRenderPass(protectedCommandBuffer);
    protectedCommandBuffer.End();
}

TEST_F(PositiveWsi, CreateSwapchainWithPresentModeInfo) {
    TEST_DESCRIPTION("Try destroying a swapchain which has multiple images");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddSurfaceExtension();
    AddRequiredExtensions(VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::swapchainMaintenance1);
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSurface());
    InitSwapchainInfo();

    // Implementations must support.
    // Also most likely to have lower minImageCount than reported for other present modes
    // (although this is implementation dependant)
    const auto present_mode = VK_PRESENT_MODE_FIFO_KHR;

    VkSurfacePresentModeEXT surface_present_mode = vku::InitStructHelper();
    surface_present_mode.presentMode = present_mode;
    VkPhysicalDeviceSurfaceInfo2KHR surface_info = vku::InitStructHelper(&surface_present_mode);
    surface_info.surface = m_surface.Handle();

    VkSurfaceCapabilities2KHR surface_caps = vku::InitStructHelper();
    vk::GetPhysicalDeviceSurfaceCapabilities2KHR(m_device->Physical(), &surface_info, &surface_caps);

    VkSwapchainPresentModesCreateInfoEXT swapchain_present_mode_create_info = vku::InitStructHelper();
    swapchain_present_mode_create_info.presentModeCount = 1;
    swapchain_present_mode_create_info.pPresentModes = &present_mode;
    VkSwapchainCreateInfoKHR swapchain_create_info = vku::InitStructHelper(&swapchain_present_mode_create_info);
    swapchain_create_info.surface = m_surface.Handle();
    swapchain_create_info.minImageCount = surface_caps.surfaceCapabilities.minImageCount;
    swapchain_create_info.imageFormat = m_surface_formats[0].format;
    swapchain_create_info.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_create_info.imageExtent = surface_caps.surfaceCapabilities.minImageExtent;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;  // implementations must support
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_create_info.compositeAlpha = m_surface_composite_alpha;
    swapchain_create_info.presentMode = present_mode;
    swapchain_create_info.clipped = VK_FALSE;
    swapchain_create_info.oldSwapchain = 0;

    m_swapchain.Init(*m_device, swapchain_create_info);
}

TEST_F(PositiveWsi, RegisterDisplayEvent) {
    TEST_DESCRIPTION("Call vkRegisterDisplayEventEXT");
    AddRequiredExtensions(VK_EXT_DISPLAY_CONTROL_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    uint32_t prop_count = 0;
    vk::GetPhysicalDeviceDisplayPropertiesKHR(Gpu(), &prop_count, nullptr);
    if (prop_count == 0) {
        GTEST_SKIP() << "No VkDisplayKHR properties to query";
    }

    std::vector<VkDisplayPropertiesKHR> display_props{prop_count};
    vk::GetPhysicalDeviceDisplayPropertiesKHR(Gpu(), &prop_count, display_props.data());
    VkDisplayKHR display = display_props[0].display;

    VkDisplayEventInfoEXT event_info = vku::InitStructHelper();
    event_info.displayEvent = VK_DISPLAY_EVENT_TYPE_FIRST_PIXEL_OUT_EXT;
    VkFence fence;

    vk::RegisterDisplayEventEXT(device(), display, &event_info, nullptr, &fence);

    vk::DestroyFence(device(), fence, nullptr);
}

TEST_F(PositiveWsi, SurfacelessQueryTest) {
    TEST_DESCRIPTION("Ensure affected API calls can be made with surfacless query extension");

    AddRequiredExtensions(VK_GOOGLE_SURFACELESS_QUERY_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());

    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "VK_GOOGLE_surfaceless_query not supported on desktop";
    }

    // Use the VK_GOOGLE_surfaceless_query extension to query the available formats and
    // colorspaces by using a VK_NULL_HANDLE for the VkSurfaceKHR handle.
    uint32_t count;
    vk::GetPhysicalDeviceSurfaceFormatsKHR(Gpu(), VK_NULL_HANDLE, &count, nullptr);
    std::vector<VkSurfaceFormatKHR> surface_formats(count);
    vk::GetPhysicalDeviceSurfaceFormatsKHR(Gpu(), VK_NULL_HANDLE, &count, surface_formats.data());

    vk::GetPhysicalDeviceSurfacePresentModesKHR(Gpu(), VK_NULL_HANDLE, &count, nullptr);
    std::vector<VkPresentModeKHR> present_modes(count);
    vk::GetPhysicalDeviceSurfacePresentModesKHR(Gpu(), VK_NULL_HANDLE, &count, present_modes.data());
}

TEST_F(PositiveWsi, PhysicalDeviceSurfaceSupport) {
    TEST_DESCRIPTION("Test if physical device supports surface.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddSurfaceExtension();
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSurface());

    VkBool32 supported;
    vk::GetPhysicalDeviceSurfaceSupportKHR(Gpu(), 0, m_surface.Handle(), &supported);

    if (supported) {
        uint32_t count;
        vk::GetPhysicalDeviceSurfaceFormatsKHR(Gpu(), m_surface.Handle(), &count, nullptr);
    }
}

TEST_F(PositiveWsi, AcquireImageBeforeGettingSwapchainImages) {
    TEST_DESCRIPTION("Call vkAcquireNextImageKHR before vkGetSwapchainImagesKHR");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddSurfaceExtension();
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSurface());

    VkBool32 supported;
    vk::GetPhysicalDeviceSurfaceSupportKHR(Gpu(), m_device->graphics_queue_node_index_, m_surface.Handle(), &supported);
    if (!supported) {
        GTEST_SKIP() << "Surface not supported.";
    }

    SurfaceInformation info = GetSwapchainInfo(m_surface.Handle());
    InitSwapchainInfo();

    VkSwapchainCreateInfoKHR swapchain_create_info = vku::InitStructHelper();
    swapchain_create_info.surface = m_surface.Handle();
    swapchain_create_info.minImageCount = info.surface_capabilities.minImageCount;
    swapchain_create_info.imageFormat = info.surface_formats[0].format;
    swapchain_create_info.imageColorSpace = info.surface_formats[0].colorSpace;
    swapchain_create_info.imageExtent = info.surface_capabilities.minImageExtent;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_create_info.compositeAlpha = info.surface_composite_alpha;
    swapchain_create_info.presentMode = info.surface_non_shared_present_mode;
    swapchain_create_info.clipped = VK_FALSE;
    swapchain_create_info.oldSwapchain = VK_NULL_HANDLE;

    vkt::Swapchain swapchain(*m_device, swapchain_create_info);

    vkt::Fence fence(*m_device);
    uint32_t image_index = swapchain.AcquireNextImage(fence, kWaitTimeout);
    vk::WaitForFences(device(), 1u, &fence.handle(), VK_FALSE, kWaitTimeout);

    const std::vector<VkImage> swapchain_images = swapchain.GetImages();

    SetPresentImageLayout(swapchain_images[image_index]);
    m_default_queue->Present(swapchain, image_index, vkt::no_semaphore);
}

// https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/7025
TEST_F(PositiveWsi, PresentFenceWaitsForSubmission) {
    TEST_DESCRIPTION("Use present fence to wait for submission");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddSurfaceExtension();
    AddRequiredExtensions(VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::swapchainMaintenance1);
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSwapchain());

    // Warm up. Show that we can reset command buffer after waiting on **submit** fence
    {
        m_command_buffer.Begin();
        m_command_buffer.End();

        vkt::Fence submit_fence(*m_device);
        m_default_queue->Submit(m_command_buffer, submit_fence);

        vk::WaitForFences(device(), 1, &submit_fence.handle(), VK_TRUE, kWaitTimeout);

        // It's safe to reset command buffer because we waited on the fence
        m_command_buffer.Reset();
    }

    // Main performance. Show that we can reset command buffer after waiting on **present** fence
    {
        const vkt::Semaphore acquire_semaphore(*m_device);
        const vkt::Semaphore submit_semaphore(*m_device);

        const auto swapchain_images = m_swapchain.GetImages();
        const uint32_t image_index = m_swapchain.AcquireNextImage(acquire_semaphore, kWaitTimeout);
        SetPresentImageLayout(swapchain_images[image_index]);

        m_command_buffer.Begin();
        m_command_buffer.End();

        m_default_queue->Submit(m_command_buffer, vkt::Wait(acquire_semaphore), vkt::Signal(submit_semaphore));

        vkt::Fence present_fence(*m_device);
        VkSwapchainPresentFenceInfoEXT present_fence_info = vku::InitStructHelper();
        present_fence_info.swapchainCount = 1;
        present_fence_info.pFences = &present_fence.handle();

        m_default_queue->Present(m_swapchain, image_index, submit_semaphore, &present_fence_info);
        present_fence.Wait(kWaitTimeout);

        // It should be safe to reset command buffer after waiting on present fence:
        //      wait on present fence ->
        //      present was initiated ->
        //      submit semaphore signaled ->
        //      QueueSubmit workload has completed ->
        //      command buffer is no longer in use and we can reset it.
        m_command_buffer.Reset();
    }
    m_default_queue->Wait();
}

TEST_F(PositiveWsi, PresentFenceRetiresPresentQueueOperation) {
    // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8047
    // The regression will cause occasional failures of this test. The reproducibility
    // is very machine dependent and in some configurations the failures can be
    // extremely rare. We also found configurations (slower laptop) where it was relatively
    // easy to reproduce (still could take some time, tens of seconds and up to few minutes).
    //
    // NOTE: there are known bugs in the current queue progress tracking, when
    // a submission might retire too early (happens for multiple queues, but present
    // operation might be an example for a single queue). Reworking queue tracking
    // from threading approach to a single manager that collects submits and resolves
    // them on request should fix the known issues, but also will bring deterministic
    // behavior to the issues like the one being tested here. The idea that resolve
    // operation, even if non trivial, still will be a localized piece of code comparing
    // to conceptually simple model of queues that process submissions one at a time
    // but with more complex synchronization and non-deterministic behavior.
    TEST_DESCRIPTION("Check that the wait on the present fence retires present queue operation");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddSurfaceExtension();
    AddRequiredExtensions(VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::swapchainMaintenance1);
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSwapchain());

    const auto swapchain_images = m_swapchain.GetImages();
    for (auto image : swapchain_images) {
        SetPresentImageLayout(image);
    }

    struct Frame {
        vkt::Semaphore image_acquired;
        vkt::Semaphore submit_finished;
        vkt::Fence present_finished_fence;
        uint32_t frame = 0;  // for debugging
    };
    std::vector<Frame> frames;

    // TODO: iteration count can be reduced (100?) if queue simulation is done in more deterministic way
    for (uint32_t i = 0; i < 500; i++) {
        // Remove completed frames
        for (auto it = frames.begin(); it != frames.end();) {
            if (it->present_finished_fence.GetStatus() == VK_SUCCESS) {
                // NOTE: Root cause of the issue. The present fence processed regular queue submissions,
                // but not the one associated with a present operation. The present batch usually was
                // lucky enough to get through, before we start the following "erase", which deletes the
                // present batch semaphore. When the queue thread was not fast enough, then in-use state
                // of present semaphore was properly detected (VUID-vkDestroySemaphore-semaphore-05149).
                it = frames.erase(it);
            } else {
                ++it;
            }
        }
        // Add new frame
        frames.emplace_back(Frame{vkt::Semaphore(*m_device), vkt::Semaphore(*m_device), vkt::Fence(*m_device), i});
        const Frame &frame = frames.back();

        const uint32_t image_index = m_swapchain.AcquireNextImage(frame.image_acquired, kWaitTimeout);

        m_default_queue->Submit(vkt::no_cmd, vkt::Wait(frame.image_acquired), vkt::Signal(frame.submit_finished));

        VkSwapchainPresentFenceInfoEXT present_fence_info = vku::InitStructHelper();
        present_fence_info.swapchainCount = 1;
        present_fence_info.pFences = &frame.present_finished_fence.handle();

        m_default_queue->Present(m_swapchain, image_index, frame.submit_finished, &present_fence_info);
    }
    m_default_queue->Wait();
}

TEST_F(PositiveWsi, QueueWaitsForPresentFence) {
    // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8376
    // https://gitlab.khronos.org/vulkan/vulkan/-/issues/3962
    TEST_DESCRIPTION("QueueWaitIdle waits for present fence");
    AddSurfaceExtension();
    AddRequiredExtensions(VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::swapchainMaintenance1);
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSwapchain());

    const vkt::Semaphore acquire_semaphore(*m_device);

    const auto swapchain_images = m_swapchain.GetImages();
    const uint32_t image_index = m_swapchain.AcquireNextImage(acquire_semaphore, kWaitTimeout);
    SetPresentImageLayout(swapchain_images[image_index]);

    vkt::Fence present_fence(*m_device);
    VkSwapchainPresentFenceInfoEXT present_fence_info = vku::InitStructHelper();
    present_fence_info.swapchainCount = 1;
    present_fence_info.pFences = &present_fence.handle();

    m_default_queue->Present(m_swapchain, image_index, acquire_semaphore, &present_fence_info);

    // QueueWaitIdle (and also DeviceWaitIdle) can wait for present fences.
    m_default_queue->Wait();

    // This should not report in-use error
    present_fence.Reset();
}

TEST_F(PositiveWsi, QueueWaitsForPresentFence2) {
    TEST_DESCRIPTION("QueueWaitIdle waits for present fence");
    AddSurfaceExtension();
    AddRequiredExtensions(VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::swapchainMaintenance1);
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSwapchain());

    SurfaceContext surface_context;
    vkt::Surface surface2;
    CreateSurface(surface_context, surface2);
    vkt::Swapchain swapchain2 =
        CreateSwapchain(surface2.Handle(), VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR);

    const vkt::Semaphore acquire_semaphore(*m_device);
    const auto swapchain_images = m_swapchain.GetImages();
    const uint32_t image_index = m_swapchain.AcquireNextImage(acquire_semaphore, kWaitTimeout);

    const vkt::Semaphore acquire_semaphore2(*m_device);
    const auto swapchain_images2 = swapchain2.GetImages();
    const uint32_t image_index2 = swapchain2.AcquireNextImage(acquire_semaphore2, kWaitTimeout);

    SetPresentImageLayout(swapchain_images[image_index]);
    SetPresentImageLayout(swapchain_images2[image_index2]);

    vkt::Fence present_fence(*m_device);
    vkt::Fence present_fence2(*m_device);
    const VkFence present_fences[2] = {present_fence, present_fence2};
    VkSwapchainPresentFenceInfoEXT present_fence_info = vku::InitStructHelper();
    present_fence_info.swapchainCount = 2;
    present_fence_info.pFences = present_fences;

    const VkSemaphore wait_semaphores[2] = {acquire_semaphore, acquire_semaphore2};
    const VkSwapchainKHR swapchains[2] = {m_swapchain, swapchain2};
    const uint32_t image_indices[2]{image_index, image_index2};
    VkPresentInfoKHR present = vku::InitStructHelper(&present_fence_info);
    present.waitSemaphoreCount = 2;
    present.pWaitSemaphores = wait_semaphores;
    present.swapchainCount = 2;
    present.pSwapchains = swapchains;
    present.pImageIndices = image_indices;
    vk::QueuePresentKHR(*m_default_queue, &present);

    m_default_queue->Wait();

    present_fence.Reset();
    present_fence2.Reset();
}

TEST_F(PositiveWsi, PresentFenceRetiresPresentSemaphores) {
    // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8578
    TEST_DESCRIPTION("Delete present wait semaphore after waiting on present fence");
    AddSurfaceExtension();
    AddRequiredExtensions(VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::swapchainMaintenance1);
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSwapchain());

    SurfaceContext surface_context2;
    vkt::Surface surface2;
    CreateSurface(surface_context2, surface2);
    vkt::Swapchain swapchain2 =
        CreateSwapchain(surface2.Handle(), VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR);

    vkt::Semaphore acquire_semaphore(*m_device);
    const auto swapchain_images = m_swapchain.GetImages();
    const uint32_t image_index = m_swapchain.AcquireNextImage(acquire_semaphore, kWaitTimeout);
    SetPresentImageLayout(swapchain_images[image_index]);

    vkt::Semaphore acquire_semaphore2(*m_device);
    const auto swapchain_images2 = swapchain2.GetImages();
    const uint32_t image_index2 = swapchain2.AcquireNextImage(acquire_semaphore2, kWaitTimeout);
    SetPresentImageLayout(swapchain_images2[image_index2]);

    const VkSemaphore acquire_semaphores_handles[2] = {acquire_semaphore, acquire_semaphore2};
    const VkSwapchainKHR swapchain_handles[2] = {m_swapchain, swapchain2};
    const VkPipelineStageFlags wait_stage_masks[2] = {VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT};

    vkt::Semaphore submit_semaphore(*m_device);

    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.waitSemaphoreCount = 2;
    submit_info.pWaitSemaphores = acquire_semaphores_handles;
    submit_info.pWaitDstStageMask = wait_stage_masks;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &submit_semaphore.handle();
    vk::QueueSubmit(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);

    vkt::Fence present_fence(*m_device);
    vkt::Fence present_fence2(*m_device);
    const VkFence present_fences_handles[2] = {present_fence, present_fence2};

    VkSwapchainPresentFenceInfoEXT present_fence_info = vku::InitStructHelper();
    present_fence_info.swapchainCount = 2;
    present_fence_info.pFences = present_fences_handles;

    const uint32_t image_indices[2] = {image_index, image_index2};

    VkPresentInfoKHR present = vku::InitStructHelper(&present_fence_info);
    present.waitSemaphoreCount = 1;
    present.pWaitSemaphores = &submit_semaphore.handle();
    present.swapchainCount = 2;
    present.pSwapchains = swapchain_handles;
    present.pImageIndices = image_indices;
    vk::QueuePresentKHR(*m_default_queue, &present);

    present_fence.Wait(kWaitTimeout);

    // Waiting on any present fence must retire all present wait semaphores.
    // It was not the case in the original issue when multiple images were presented.
    // Deleting semaphore after the fence wait resulted in semaphore in-use error.
    submit_semaphore = {};

    present_fence2.Wait(kWaitTimeout);
}

TEST_F(PositiveWsi, DifferentPerPresentModeImageCount) {
    TEST_DESCRIPTION("Create swapchain with per present mode minImageCount that is less than surface's general minImageCount");
#ifndef VK_USE_PLATFORM_WAYLAND_KHR
    GTEST_SKIP() << "Test requires wayland platform support";
#else
    AddSurfaceExtension();
    AddRequiredExtensions(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::swapchainMaintenance1);
    RETURN_IF_SKIP(Init());

    WaylandContext wayland_ctx;
    if (!wayland_ctx.Init()) {
        GTEST_SKIP() << "Failed to create wayland context.";
    }

    VkWaylandSurfaceCreateInfoKHR surface_create_info = vku::InitStructHelper();
    surface_create_info.display = wayland_ctx.display;
    surface_create_info.surface = wayland_ctx.surface;

    VkSurfaceKHR surface;
    vk::CreateWaylandSurfaceKHR(instance(), &surface_create_info, nullptr, &surface);
    auto info = GetSwapchainInfo(surface);

    const auto present_mode = VK_PRESENT_MODE_FIFO_KHR;  // Implementations must support

    VkSurfaceCapabilities2KHR surface_caps = vku::InitStructHelper();
    VkPhysicalDeviceSurfaceInfo2KHR surface_info = vku::InitStructHelper();
    surface_info.surface = surface;
    vk::GetPhysicalDeviceSurfaceCapabilities2KHR(Gpu(), &surface_info, &surface_caps);
    const uint32_t general_min_image_count = surface_caps.surfaceCapabilities.minImageCount;

    VkSurfacePresentModeEXT surface_present_mode = vku::InitStructHelper();
    surface_present_mode.presentMode = present_mode;
    surface_info.pNext = &surface_present_mode;
    vk::GetPhysicalDeviceSurfaceCapabilities2KHR(Gpu(), &surface_info, &surface_caps);
    const uint32_t per_present_mode_min_image_count = surface_caps.surfaceCapabilities.minImageCount;

    if (per_present_mode_min_image_count >= general_min_image_count) {
        vk::DestroySurfaceKHR(instance(), surface, nullptr);
        wayland_ctx.Release();
        GTEST_SKIP() << "Can't find present mode that uses less images than a general case";
    }

    VkSwapchainPresentModesCreateInfoEXT swapchain_present_mode_create_info = vku::InitStructHelper();
    swapchain_present_mode_create_info.presentModeCount = 1;
    swapchain_present_mode_create_info.pPresentModes = &present_mode;

    VkSwapchainCreateInfoKHR swapchain_create_info = vku::InitStructHelper(&swapchain_present_mode_create_info);
    swapchain_create_info.surface = surface;
    swapchain_create_info.minImageCount = per_present_mode_min_image_count;
    swapchain_create_info.imageFormat = info.surface_formats[0].format;
    swapchain_create_info.imageColorSpace = info.surface_formats[0].colorSpace;
    swapchain_create_info.imageExtent = surface_caps.surfaceCapabilities.minImageExtent;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain_create_info.presentMode = present_mode;
    swapchain_create_info.clipped = VK_FALSE;
    swapchain_create_info.oldSwapchain = 0;

    { vkt::Swapchain swapchain(*m_device, swapchain_create_info); }

    vk::DestroySurfaceKHR(instance(), surface, nullptr);
    wayland_ctx.Release();
#endif
}

TEST_F(PositiveWsi, ReleaseSwapchainImages) {
    TEST_DESCRIPTION("Test vkReleaseSwapchainImagesEXT");

    AddSurfaceExtension();
    AddRequiredExtensions(VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::swapchainMaintenance1);
    RETURN_IF_SKIP(Init());

    RETURN_IF_SKIP(InitSurface());

    VkBool32 supported;
    vk::GetPhysicalDeviceSurfaceSupportKHR(Gpu(), m_device->graphics_queue_node_index_, m_surface.Handle(), &supported);
    if (!supported) {
        GTEST_SKIP() << "Graphics queue does not support present";
    }

    SurfaceInformation info = GetSwapchainInfo(m_surface.Handle());
    const uint32_t imageCount = 4;

    if (info.surface_capabilities.maxImageCount < imageCount) {
        GTEST_SKIP() << "Test maxImageCount to be at least 4";
    }
    InitSwapchainInfo();

    VkSwapchainCreateInfoKHR swapchain_create_info = vku::InitStructHelper();
    swapchain_create_info.surface = m_surface.Handle();
    swapchain_create_info.minImageCount = info.surface_capabilities.maxImageCount;
    swapchain_create_info.imageFormat = info.surface_formats[0].format;
    swapchain_create_info.imageColorSpace = info.surface_formats[0].colorSpace;
    swapchain_create_info.imageExtent = info.surface_capabilities.minImageExtent;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_create_info.compositeAlpha = info.surface_composite_alpha;
    swapchain_create_info.presentMode = info.surface_non_shared_present_mode;
    swapchain_create_info.clipped = VK_FALSE;
    swapchain_create_info.oldSwapchain = VK_NULL_HANDLE;

    m_swapchain = vkt::Swapchain(*m_device, swapchain_create_info);

    ASSERT_TRUE(m_swapchain.initialized());

    vkt::Fence fence(*m_device);
    std::vector<uint32_t> release_indices;
    uint32_t present_index = 0u;

    for (uint32_t i = 0; i < imageCount - 1; ++i) {
        uint32_t image_index = m_swapchain.AcquireNextImage(fence, kWaitTimeout);
        if (i == 1) {
            present_index = image_index;
        } else {
            release_indices.push_back(image_index);
        }
        vk::WaitForFences(device(), 1, &fence.handle(), VK_TRUE, kWaitTimeout);
        vk::ResetFences(device(), 1, &fence.handle());
    }

    const auto swapchain_images = m_swapchain.GetImages();
    SetPresentImageLayout(swapchain_images[present_index]);

    m_default_queue->Present(m_swapchain, present_index, vkt::no_semaphore);

    VkReleaseSwapchainImagesInfoEXT releaseInfo = vku::InitStructHelper();
    releaseInfo.swapchain = m_swapchain;
    releaseInfo.imageIndexCount = (uint32_t)release_indices.size();
    releaseInfo.pImageIndices = release_indices.data();
    vk::ReleaseSwapchainImagesEXT(device(), &releaseInfo);

    vk::DeviceWaitIdle(device());
}

TEST_F(PositiveWsi, ReleaseAndAcquireSwapchainImages) {
    TEST_DESCRIPTION("Test vkReleaseSwapchainImagesEXT");

    AddSurfaceExtension();
    AddRequiredExtensions(VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::swapchainMaintenance1);
    RETURN_IF_SKIP(Init());

    RETURN_IF_SKIP(InitSwapchain());

    const auto swapchain_images = m_swapchain.GetImages();
    vkt::Fence fence(*m_device);

    for (uint32_t i = 0; i < 64; ++i) {
        uint32_t image_index = m_swapchain.AcquireNextImage(fence, vvl::kU64Max);
        vk::WaitForFences(device(), 1, &fence.handle(), VK_TRUE, kWaitTimeout);
        vk::ResetFences(device(), 1, &fence.handle());

        VkReleaseSwapchainImagesInfoEXT releaseInfo = vku::InitStructHelper();
        releaseInfo.swapchain = m_swapchain;
        releaseInfo.imageIndexCount = 1u;
        releaseInfo.pImageIndices = &image_index;
        vk::ReleaseSwapchainImagesEXT(device(), &releaseInfo);
    }

    vk::DeviceWaitIdle(device());
}

TEST_F(PositiveWsi, MultiSwapchainPresentWithOneBadSwapchain) {
    // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8753
    TEST_DESCRIPTION("Present swapchains with a single QueuePresent command. One of the swapchains is out of date.");
    AddSurfaceExtension();
    RETURN_IF_SKIP(SupportMultiSwapchain());
    RETURN_IF_SKIP(SupportSurfaceResize());
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSwapchain());

    // This test make the second swapchain invalid (VK_ERROR_OUT_OF_DATE_KHR) and then try to present both swapchains.
    // Presentation failure due to the second swapchain should not break state tracking for the first swapchain.
    // In the origianl issue, state tracking for the first swapchain was skipped during QueuePresent and acquired
    // images were never released. This generated false positives that too many images was acquired by the first swapchain.

    SurfaceContext surface_context2;
    vkt::Surface surface2;
    CreateSurface(surface_context2, surface2);
    auto swapchain2 =
        CreateSwapchain(surface2.Handle(), VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR);
    const VkSwapchainKHR swapchain_handles[2] = {m_swapchain, swapchain2};

    auto cleanup_resources = [&] { m_default_queue->Wait(); };
    const auto swapchain_images = m_swapchain.GetImages();
    for (auto image : swapchain_images) {
        SetPresentImageLayout(image);
    }
    const auto swapchain_images2 = swapchain2.GetImages();
    for (auto image2 : swapchain_images2) {
        SetPresentImageLayout(image2);
    }

    vkt::Semaphore acquire_semaphore(*m_device);
    vkt::Semaphore acquire_semaphore2(*m_device);
    const VkSemaphore acquire_semaphore_handles[2] = {acquire_semaphore, acquire_semaphore2};

    std::vector<vkt::Semaphore> submit_semaphores;
    for (size_t i = 0; i < swapchain_images.size(); i++) {
        submit_semaphores.emplace_back(*m_device);
    }

    vkt::Fence frame_fence(*m_device);

    const VkPipelineStageFlags wait_stage_masks[2] = {VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT};

    // The image index from the second swapchain.
    uint32_t image_index2{};

    // Resize second swapchain window. This potentially generates VK_ERROR_OUT_OF_DATE_KHR in QueuePresent.
    surface_context2.Resize(m_width / 2, m_height / 2);

    // The first frame.
    // Presentation to the second swapchain fails due to resized window.
    {
        VkResult acquire_result2{};
        image_index2 = swapchain2.AcquireNextImage(acquire_semaphore2, kWaitTimeout, &acquire_result2);
        if (acquire_result2 != VK_SUCCESS) {
            cleanup_resources();
            GTEST_SKIP() << "Cannot acquire image from the second swapchain. The test is designed for a scenario when it is "
                            "possible to acquire image after window resize (works on windows nvidia drivers)";
        }

        const uint32_t image_index = m_swapchain.AcquireNextImage(acquire_semaphore, kWaitTimeout);
        const uint32_t image_indices[2] = {image_index, image_index2};

        VkSubmitInfo submit_info = vku::InitStructHelper();
        submit_info.waitSemaphoreCount = 2;
        submit_info.pWaitSemaphores = acquire_semaphore_handles;
        submit_info.pWaitDstStageMask = wait_stage_masks;
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &submit_semaphores[image_index].handle();
        vk::QueueSubmit(m_default_queue->handle(), 1, &submit_info, frame_fence);

        VkPresentInfoKHR present = vku::InitStructHelper();
        present.waitSemaphoreCount = 1;
        present.pWaitSemaphores = &submit_semaphores[image_index].handle();
        present.swapchainCount = 2;
        present.pSwapchains = swapchain_handles;
        present.pImageIndices = image_indices;

        VkResult present_result = vk::QueuePresentKHR(*m_default_queue, &present);
        if (present_result != VK_ERROR_OUT_OF_DATE_KHR) {
            cleanup_resources();
            GTEST_SKIP() << "Cannot generate VK_ERROR_OUT_OF_DATE_KHR state required for this test";
        }
    }

    // All other frames.
    for (uint32_t i = 0; i < 5; i++) {
        frame_fence.Wait(kWaitTimeout);
        frame_fence.Reset();

        // The test checks that image acquire from the first swapchain does not generate validation error that no images left.
        // The second swapchain should not affect acquired image tracking in the first swapchain.
        const uint32_t image_index = m_swapchain.AcquireNextImage(acquire_semaphore, vvl::kU64Max);

        // Do not try to acquire images from the second swapchain, it is broken.
        // Suppress error that we present not acquired image.
        m_errorMonitor->SetAllowedFailureMsg("VUID-VkPresentInfoKHR-pImageIndices-01430");

        // image_index presentation should succeed, image_index2 should fail.
        const uint32_t image_indices[2] = {image_index, image_index2};

        m_default_queue->Submit(vkt::no_cmd, vkt::Wait(acquire_semaphore), vkt::Signal(submit_semaphores[image_index]), frame_fence);

        VkPresentInfoKHR present = vku::InitStructHelper();
        present.waitSemaphoreCount = 1;
        present.pWaitSemaphores = &submit_semaphores[image_index].handle();
        present.swapchainCount = 2;
        present.pSwapchains = swapchain_handles;
        present.pImageIndices = image_indices;
        vk::QueuePresentKHR(*m_default_queue, &present);
    }
    cleanup_resources();
}

TEST_F(PositiveWsi, MixKHRAndKHR2SurfaceCapsQueries) {
    // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8772
    TEST_DESCRIPTION("Mixing KHR and KHR2 surface queries should not break VVL surface caps caching");

    AddRequiredExtensions(VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::swapchainMaintenance1);
    AddSurfaceExtension();
    RETURN_IF_SKIP(SupportSurfaceResize());
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSurface());
    InitSwapchainInfo();

    // KHR2 query with present mode
    VkSurfacePresentModeEXT surface_present_mode = vku::InitStructHelper();
    surface_present_mode.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    VkPhysicalDeviceSurfaceInfo2KHR surface_info = vku::InitStructHelper(&surface_present_mode);
    surface_info.surface = m_surface.Handle();
    VkSurfaceCapabilities2KHR surface_caps2 = vku::InitStructHelper();
    vk::GetPhysicalDeviceSurfaceCapabilities2KHR(m_device->Physical(), &surface_info, &surface_caps2);

    // Resize
    m_surface_context.Resize(m_surface_capabilities.currentExtent.width + 25, m_surface_capabilities.currentExtent.height);

    // KHR query
    VkSurfaceCapabilitiesKHR surface_caps;
    vk::GetPhysicalDeviceSurfaceCapabilitiesKHR(Gpu(), m_surface.Handle(), &surface_caps);

    VkSwapchainCreateInfoKHR swapchain_ci = vku::InitStructHelper();
    swapchain_ci.surface = m_surface.Handle();
    swapchain_ci.minImageCount = surface_caps.minImageCount;
    swapchain_ci.imageFormat = m_surface_formats[0].format;
    swapchain_ci.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_ci.imageExtent = surface_caps.maxImageExtent;
    swapchain_ci.imageArrayLayers = 1;
    swapchain_ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_ci.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_ci.compositeAlpha = m_surface_composite_alpha;
    swapchain_ci.presentMode = m_surface_non_shared_present_mode;

    vkt::Swapchain swapchain(*m_device, swapchain_ci);
}

TEST_F(PositiveWsi, MixKHRAndKHR2SurfaceCapsQueries2) {
    // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8772
    TEST_DESCRIPTION("Mixing KHR and KHR2 surface queries should not break VVL surface caps caching");

    AddRequiredExtensions(VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::swapchainMaintenance1);
    AddSurfaceExtension();
    RETURN_IF_SKIP(SupportSurfaceResize());
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSurface());
    InitSwapchainInfo();

    // KHR query
    VkSurfaceCapabilitiesKHR surface_caps;
    vk::GetPhysicalDeviceSurfaceCapabilitiesKHR(Gpu(), m_surface.Handle(), &surface_caps);

    // Resize
    m_surface_context.Resize(m_surface_capabilities.currentExtent.width + 25, m_surface_capabilities.currentExtent.height);

    // KHR2 query with present mode
    VkSurfacePresentModeEXT surface_present_mode = vku::InitStructHelper();
    surface_present_mode.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    VkPhysicalDeviceSurfaceInfo2KHR surface_info = vku::InitStructHelper(&surface_present_mode);
    surface_info.surface = m_surface.Handle();
    VkSurfaceCapabilities2KHR surface_caps2 = vku::InitStructHelper();
    vk::GetPhysicalDeviceSurfaceCapabilities2KHR(m_device->Physical(), &surface_info, &surface_caps2);

    VkSwapchainCreateInfoKHR swapchain_ci = vku::InitStructHelper();
    swapchain_ci.surface = m_surface.Handle();
    swapchain_ci.minImageCount = surface_caps2.surfaceCapabilities.minImageCount;
    swapchain_ci.imageFormat = m_surface_formats[0].format;
    swapchain_ci.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_ci.imageExtent = surface_caps2.surfaceCapabilities.maxImageExtent;
    swapchain_ci.imageArrayLayers = 1;
    swapchain_ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_ci.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_ci.compositeAlpha = m_surface_composite_alpha;
    swapchain_ci.presentMode = m_surface_non_shared_present_mode;

    vkt::Swapchain swapchain(*m_device, swapchain_ci);
}

TEST_F(PositiveWsi, CreateSwapchainImagesWithConcurrentSharingMode) {
    TEST_DESCRIPTION("Create images from swapchain with concurrent sharing mode");

    AddSurfaceExtension();
    RETURN_IF_SKIP(Init());
    if (!m_second_queue) {
        GTEST_SKIP() << "Two queues are needed to run this test";
    }
    RETURN_IF_SKIP(InitSurface());
    InitSwapchainInfo();

    uint32_t queue_family_indices[] = {m_default_queue->family_index, m_second_queue->family_index};

    VkSurfaceCapabilitiesKHR surface_caps;
    vk::GetPhysicalDeviceSurfaceCapabilitiesKHR(Gpu(), m_surface.Handle(), &surface_caps);

    VkImageFormatProperties img_format_props;
    vk::GetPhysicalDeviceImageFormatProperties(Gpu(), m_surface_formats[0].format, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL,
                                               VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, 0, &img_format_props);
    VkExtent2D img_ext = {std::min(m_surface_capabilities.maxImageExtent.width, img_format_props.maxExtent.width),
                          std::min(m_surface_capabilities.maxImageExtent.height, img_format_props.maxExtent.height)};

    VkSwapchainCreateInfoKHR swapchain_ci = vku::InitStructHelper();
    swapchain_ci.surface = m_surface.Handle();
    swapchain_ci.minImageCount = surface_caps.minImageCount;
    swapchain_ci.imageFormat = m_surface_formats[0].format;
    swapchain_ci.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_ci.imageExtent = img_ext;
    swapchain_ci.imageArrayLayers = 1u;
    swapchain_ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_ci.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    swapchain_ci.queueFamilyIndexCount = 2u;
    swapchain_ci.pQueueFamilyIndices = queue_family_indices;
    swapchain_ci.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_ci.compositeAlpha = m_surface_composite_alpha;
    swapchain_ci.presentMode = m_surface_non_shared_present_mode;
    vkt::Swapchain swapchain(*m_device, swapchain_ci);

    VkImageSwapchainCreateInfoKHR image_swapchain_ci = vku::InitStructHelper();
    image_swapchain_ci.swapchain = swapchain.handle();

    VkImageCreateInfo image_create_info = vku::InitStructHelper(&image_swapchain_ci);
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = swapchain_ci.imageFormat;
    image_create_info.extent.width = swapchain_ci.imageExtent.width;
    image_create_info.extent.height = swapchain_ci.imageExtent.height;
    image_create_info.extent.depth = 1u;
    image_create_info.mipLevels = 1u;
    image_create_info.arrayLayers = 1u;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_create_info.sharingMode = VK_SHARING_MODE_CONCURRENT;
    image_create_info.queueFamilyIndexCount = 2u;
    image_create_info.pQueueFamilyIndices = queue_family_indices;
    vkt::Image image(*m_device, image_create_info, vkt::no_mem);
}

TEST_F(PositiveWsi, CreateSwapchainImagesWithExclusiveSharingMode) {
    TEST_DESCRIPTION("Create images from swapchain with exclusive sharing mode");

    AddSurfaceExtension();
    RETURN_IF_SKIP(Init());
    if (!m_second_queue) {
        GTEST_SKIP() << "Two queues are needed to run this test";
    }
    RETURN_IF_SKIP(InitSurface());
    InitSwapchainInfo();

    uint32_t queue_family_indices[] = {m_default_queue->family_index, m_second_queue->family_index};

    VkSurfaceCapabilitiesKHR surface_caps;
    vk::GetPhysicalDeviceSurfaceCapabilitiesKHR(Gpu(), m_surface.Handle(), &surface_caps);

    VkImageFormatProperties img_format_props;
    vk::GetPhysicalDeviceImageFormatProperties(Gpu(), m_surface_formats[0].format, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL,
                                               VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, 0, &img_format_props);
    VkExtent2D img_ext = {std::min(m_surface_capabilities.maxImageExtent.width, img_format_props.maxExtent.width),
                          std::min(m_surface_capabilities.maxImageExtent.height, img_format_props.maxExtent.height)};

    VkSwapchainCreateInfoKHR swapchain_ci = vku::InitStructHelper();
    swapchain_ci.surface = m_surface.Handle();
    swapchain_ci.minImageCount = surface_caps.minImageCount;
    swapchain_ci.imageFormat = m_surface_formats[0].format;
    swapchain_ci.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_ci.imageExtent = img_ext;
    swapchain_ci.imageArrayLayers = 1u;
    swapchain_ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_ci.queueFamilyIndexCount = 1u;
    swapchain_ci.pQueueFamilyIndices = queue_family_indices;
    swapchain_ci.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_ci.compositeAlpha = m_surface_composite_alpha;
    swapchain_ci.presentMode = m_surface_non_shared_present_mode;
    vkt::Swapchain swapchain(*m_device, swapchain_ci);

    VkImageSwapchainCreateInfoKHR image_swapchain_ci = vku::InitStructHelper();
    image_swapchain_ci.swapchain = swapchain.handle();

    VkImageCreateInfo image_create_info = vku::InitStructHelper(&image_swapchain_ci);
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = swapchain_ci.imageFormat;
    image_create_info.extent.width = swapchain_ci.imageExtent.width;
    image_create_info.extent.height = swapchain_ci.imageExtent.height;
    image_create_info.extent.depth = 1u;
    image_create_info.mipLevels = 1u;
    image_create_info.arrayLayers = 1u;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_create_info.queueFamilyIndexCount = 1u;
    image_create_info.pQueueFamilyIndices = queue_family_indices;
    vkt::Image image(*m_device, image_create_info, vkt::no_mem);
}

TEST_F(PositiveWsi, CreateSwapchainWithOldSwapchain) {
    AddSurfaceExtension();
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSurface());
    InitSwapchainInfo();

    VkSurfaceCapabilitiesKHR surface_caps;
    vk::GetPhysicalDeviceSurfaceCapabilitiesKHR(Gpu(), m_surface.Handle(), &surface_caps);

    VkSwapchainCreateInfoKHR swapchain_ci = vku::InitStructHelper();
    swapchain_ci.surface = m_surface.Handle();
    swapchain_ci.minImageCount = surface_caps.minImageCount;
    swapchain_ci.imageFormat = m_surface_formats[0].format;
    swapchain_ci.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_ci.imageExtent = surface_caps.minImageExtent;
    swapchain_ci.imageArrayLayers = 1u;
    swapchain_ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_ci.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_ci.compositeAlpha = m_surface_composite_alpha;
    swapchain_ci.presentMode = m_surface_non_shared_present_mode;
    vkt::Swapchain swapchain1(*m_device, swapchain_ci);

    swapchain_ci.oldSwapchain = swapchain1;
    vkt::Swapchain swapchain2(*m_device, swapchain_ci);
}

TEST_F(PositiveWsi, OldSwapchainFromAnotherSurface) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/10112");
    AddSurfaceExtension();
    RETURN_IF_SKIP(Init());
    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Will leak in ANSN because headless machine uses xvfb, but can only handle a single surface";
    }
    RETURN_IF_SKIP(InitSurface());
    InitSwapchainInfo();

    VkSurfaceCapabilitiesKHR surface_caps;
    vk::GetPhysicalDeviceSurfaceCapabilitiesKHR(Gpu(), m_surface.Handle(), &surface_caps);

    VkSwapchainCreateInfoKHR swapchain_ci = vku::InitStructHelper();
    swapchain_ci.surface = m_surface.Handle();
    swapchain_ci.minImageCount = surface_caps.minImageCount;
    swapchain_ci.imageFormat = m_surface_formats[0].format;
    swapchain_ci.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_ci.imageExtent = surface_caps.minImageExtent;
    swapchain_ci.imageArrayLayers = 1u;
    swapchain_ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_ci.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_ci.compositeAlpha = m_surface_composite_alpha;
    swapchain_ci.presentMode = m_surface_non_shared_present_mode;
    vkt::Swapchain swapchain1(*m_device, swapchain_ci);

    vkt::Surface surface2{};
    VkResult result = CreateSurface(m_surface_context, surface2);
    if (result != VK_SUCCESS) {
        GTEST_SKIP() << "Failed to create surface.";
    }

    swapchain_ci.oldSwapchain = swapchain1;
    swapchain_ci.surface = surface2.Handle();
    vkt::Swapchain swapchain2(*m_device, swapchain_ci);
}

TEST_F(PositiveWsi, UseAcquireFenceToDeletePresentSemaphore) {
    // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/9587
    TEST_DESCRIPTION("Use acquire fence to safely delete present semaphore from previous present operations");
    AddSurfaceExtension();
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSwapchain());
    const auto swapchain_images = m_swapchain.GetImages();
    for (auto image : swapchain_images) {
        SetPresentImageLayout(image);
    }

    // Frame 0
    vkt::Semaphore acquire_semaphore0(*m_device);
    uint32_t image_index = m_swapchain.AcquireNextImage(acquire_semaphore0, kWaitTimeout);
    if (image_index != 0) {
        GTEST_SKIP() << "test scenario assumes the first acquired image index is 0";
    }
    vkt::Semaphore present_semaphore0(*m_device);
    m_default_queue->Submit(vkt::no_cmd, vkt::Wait(acquire_semaphore0), vkt::Signal(present_semaphore0));
    m_default_queue->Present(m_swapchain, image_index, present_semaphore0);

    // Frame 1
    vkt::Semaphore acquire_semaphore1(*m_device);
    image_index = m_swapchain.AcquireNextImage(acquire_semaphore1, kWaitTimeout);
    if (image_index != 1) {
        m_default_queue->Wait();
        GTEST_SKIP() << "test scenario assumes the second acquired image index is 1";
    }
    vkt::Semaphore present_semaphore1(*m_device);
    m_default_queue->Submit(vkt::no_cmd, vkt::Wait(acquire_semaphore1), vkt::Signal(present_semaphore1));
    m_default_queue->Present(m_swapchain, image_index, present_semaphore1);

    // Frame 2
    vkt::Fence acquire_fence2(*m_device);
    image_index = m_swapchain.AcquireNextImage(acquire_fence2, kWaitTimeout);
    if (image_index != 0) {
        m_default_queue->Wait();
        GTEST_SKIP() << "test scenario assumes the third acquired image index is 0";
    }
    acquire_fence2.Wait(kWaitTimeout);

    // This test checks that destroying present semaphore from frame 0 does not generate in-use error.
    present_semaphore0.Destroy();

    m_default_queue->Wait();
}

TEST_F(PositiveWsi, ExampleHowToReusePresentSemaphores) {
    TEST_DESCRIPTION("Example of how to safely reuse present semaphores by allocating one per swapchain image");
    AddSurfaceExtension();
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSwapchain());
    const auto swapchain_images = m_swapchain.GetImages();
    for (auto image : swapchain_images) {
        SetPresentImageLayout(image);
    }

    // Use single fence to wait for every frame (not very effective but it's fine for testing purposes)
    vkt::Fence frame_fence(*m_device, VK_FENCE_CREATE_SIGNALED_BIT);

    // The acquire semaphore should be indexed by the current frame buffering index (0 in this case, 0/1 for double buffering).
    vkt::Semaphore acquire_semaphore(*m_device);

    // Present semaphores (signaled by submit and waited on by present) are allocated per swapchain image.
    // When a swapchain image is acquired, we know that the previous presentation of this image has finished,
    // so the associated semaphore is no longer in use.
    //
    // IMPORTANT: Present semaphores array should be indexed by the acquired image index.
    std::vector<vkt::Semaphore> present_semaphores;
    for (size_t i = 0; i < swapchain_images.size(); i++) {
        present_semaphores.emplace_back(*m_device);
    }

    for (uint32_t i = 0; i < 10; i++) {
        frame_fence.Wait(kWaitTimeout);
        frame_fence.Reset();

        uint32_t image_index = m_swapchain.AcquireNextImage(acquire_semaphore, kWaitTimeout);
        m_default_queue->Submit(vkt::no_cmd, vkt::Wait(acquire_semaphore), vkt::Signal(present_semaphores[image_index]),
                                frame_fence);
        m_default_queue->Present(m_swapchain, image_index, present_semaphores[image_index]);
    }

    m_default_queue->Wait();
}

TEST_F(PositiveWsi, ExampleHowToReusePresentSemaphores2) {
    TEST_DESCRIPTION("Example of how to safely reuse present semaphores by using presentation fence");
    AddSurfaceExtension();
    AddRequiredExtensions(VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::swapchainMaintenance1);
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSwapchain());

    const auto swapchain_images = m_swapchain.GetImages();
    for (auto image : swapchain_images) {
        SetPresentImageLayout(image);
    }

    vkt::CommandBuffer command_buffers[2] = {vkt::CommandBuffer{*m_device, m_command_pool},
                                             vkt::CommandBuffer(*m_device, m_command_pool)};

    // The acquire semaphores should be indexed by the current frame buffering index.
    vkt::Semaphore acquire_semaphores[2] = {*m_device, *m_device};

    // The present semaphores can also be indexed by the current frame index if we use presentation fence
    // and associate presentation fence with each buffered frame.
    vkt::Semaphore present_semaphores[2] = {*m_device, *m_device};

    vkt::Fence present_fences[2] = {{*m_device, VK_FENCE_CREATE_SIGNALED_BIT}, {*m_device, VK_FENCE_CREATE_SIGNALED_BIT}};

    int frame_index = 0;
    for (uint32_t i = 0; i < 10; i++) {
        vkt::Fence &present_fence = present_fences[frame_index];
        vkt::CommandBuffer &command_buffer = command_buffers[frame_index];
        vkt::Semaphore &acquire_semaphore = acquire_semaphores[frame_index];
        vkt::Semaphore &present_semaphore = present_semaphores[frame_index];

        present_fence.Wait(kWaitTimeout);
        present_fence.Reset();

        command_buffer.Begin();
        command_buffer.End();

        uint32_t image_index = m_swapchain.AcquireNextImage(acquire_semaphore, kWaitTimeout);
        m_default_queue->Submit(command_buffer, vkt::Wait(acquire_semaphore), vkt::Signal(present_semaphore));

        VkSwapchainPresentFenceInfoEXT present_fence_info = vku::InitStructHelper();
        present_fence_info.swapchainCount = 1;
        present_fence_info.pFences = &present_fence.handle();

        m_default_queue->Present(m_swapchain, image_index, present_semaphore, &present_fence_info);
        frame_index = 1 - frame_index;  // 0 or 1
    }
    m_default_queue->Wait();
}

TEST_F(PositiveWsi, SignalPresentSemaphoreAfterFenceWait) {
    TEST_DESCRIPTION("Signal present wait semaphore after waiting on the presentation fence");
    AddSurfaceExtension();
    AddRequiredExtensions(VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::swapchainMaintenance1);
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSwapchain());
    const auto swapchain_images = m_swapchain.GetImages();
    for (auto image : swapchain_images) {
        SetPresentImageLayout(image);
    }

    vkt::Semaphore acquire_semaphore(*m_device);
    uint32_t image_index = m_swapchain.AcquireNextImage(acquire_semaphore, kWaitTimeout);

    vkt::Fence present_fence(*m_device);
    VkSwapchainPresentFenceInfoEXT present_fence_info = vku::InitStructHelper();
    present_fence_info.swapchainCount = 1;
    present_fence_info.pFences = &present_fence.handle();

    vkt::Semaphore present_semaphore(*m_device);
    m_default_queue->Submit(vkt::no_cmd, vkt::Wait(acquire_semaphore), vkt::Signal(present_semaphore));
    m_default_queue->Present(m_swapchain, image_index, present_semaphore, &present_fence_info);

    // Test that after waiting on the present fence it's safe to signal present semaphore again
    vk::WaitForFences(device(), 1, &present_fence.handle(), VK_TRUE, kWaitTimeout);
    m_default_queue->Submit(vkt::no_cmd, vkt::Signal(present_semaphore));

    m_default_queue->Wait();
}

TEST_F(PositiveWsi, SignalPresentSemaphoreAfterQueueWait) {
    TEST_DESCRIPTION("Signal present wait semaphore after waiting on device queue. Only works for pre-swapchain-maintenance1");
    AddSurfaceExtension();
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSwapchain());
    const auto swapchain_images = m_swapchain.GetImages();
    for (auto image : swapchain_images) {
        SetPresentImageLayout(image);
    }

    vkt::Semaphore acquire_semaphore(*m_device);
    uint32_t image_index = m_swapchain.AcquireNextImage(acquire_semaphore, kWaitTimeout);

    vkt::Semaphore present_semaphore(*m_device);
    m_default_queue->Submit(vkt::no_cmd, vkt::Wait(acquire_semaphore), vkt::Signal(present_semaphore));
    m_default_queue->Present(m_swapchain, image_index, present_semaphore);

    m_default_queue->Wait();
    m_default_queue->Submit(vkt::no_cmd, vkt::Signal(present_semaphore));
    m_default_queue->Wait();
}

TEST_F(PositiveWsi, GetDeviceGroupSurfacePresentModes) {
    AddSurfaceExtension();
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSwapchain());

    VkDeviceGroupPresentModeFlagsKHR present_mode_flags;
    vk::GetDeviceGroupSurfacePresentModesKHR(*m_device, m_surface.Handle(), &present_mode_flags);
}

TEST_F(PositiveWsi, ProgressOnPresentOnlyQueue) {
    TEST_DESCRIPTION("Enqueue presentation requests on the dedicated queue");
    AddSurfaceExtension();
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSwapchain());
    if (!m_second_queue) {
        GTEST_SKIP() << "Two queues are needed to run this test";
    }
    VkBool32 supported;
    vk::GetPhysicalDeviceSurfaceSupportKHR(Gpu(), m_second_queue->family_index, m_surface.Handle(), &supported);
    if (!supported) {
        GTEST_SKIP() << "The second queue does not support present";
    }
    const auto swapchain_images = m_swapchain.GetImages();
    for (auto image : swapchain_images) {
        SetPresentImageLayout(image);
    }

    std::vector<vkt::Semaphore> present_wait_semaphores;
    for (size_t i = 0; i < swapchain_images.size(); i++) {
        present_wait_semaphores.emplace_back(*m_device);
    }

    vkt::Fence frame_fences[2] = {{*m_device, VK_FENCE_CREATE_SIGNALED_BIT}, {*m_device, VK_FENCE_CREATE_SIGNALED_BIT}};
    vkt::Semaphore acquire_semaphores[2] = {*m_device, *m_device};
    vkt::CommandBuffer command_buffers[2] = {vkt::CommandBuffer{*m_device, m_command_pool},
                                             vkt::CommandBuffer{*m_device, m_command_pool}};
    uint32_t frame_index = 0;

    // NOTE: This test can be used for manual inspection of memory usage.
    // Increase frame count and observe that the test does not continuously allocate memory.
    const int frame_count = 100;
    for (int i = 0; i < frame_count; i++) {
        const vkt::Fence &frame_fence = frame_fences[frame_index];
        const vkt::Semaphore &acquire_semaphore = acquire_semaphores[frame_index];
        vkt::CommandBuffer &command_buffer = command_buffers[frame_index];

        frame_fence.Wait(kWaitTimeout);
        frame_fence.Reset();

        const uint32_t image_index = m_swapchain.AcquireNextImage(acquire_semaphore, kWaitTimeout);
        const vkt::Semaphore &present_wait_semaphore = present_wait_semaphores[image_index];

        command_buffer.Begin();
        command_buffer.End();

        m_default_queue->Submit(command_buffer, vkt::Wait(acquire_semaphore), vkt::Signal(present_wait_semaphore), frame_fence);
        m_second_queue->Present(m_swapchain, image_index, present_wait_semaphore);
        frame_index = 1 - frame_index;  // 0 or 1
    }
    m_default_queue->Wait();
    m_second_queue->Wait();
}

TEST_F(PositiveWsi, SharedPresentAndPresentSemaphoreReuse) {
    // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/10201
    TEST_DESCRIPTION("Present semaphore in-use check is disabled when shared present mode is used without swapchain maintenance1");
    AddRequiredExtensions(VK_KHR_SHARED_PRESENTABLE_IMAGE_EXTENSION_NAME);
    AddSurfaceExtension();
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSurface());
    InitSwapchainInfo();

    bool found = false;
    for (VkPresentModeKHR present_mode : m_surface_present_modes) {
        found |= (present_mode == VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR);
    }
    if (!found) {
        GTEST_SKIP() << "Cannot find shared present mode";
    }

    VkSwapchainCreateInfoKHR swapchain_ci = vku::InitStructHelper();
    swapchain_ci.surface = m_surface.Handle();
    swapchain_ci.minImageCount = 1;
    swapchain_ci.imageFormat = m_surface_formats[0].format;
    swapchain_ci.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_ci.imageExtent = m_surface_capabilities.minImageExtent;
    swapchain_ci.imageArrayLayers = 1;
    swapchain_ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;  // implementations must support
    swapchain_ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_ci.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_ci.compositeAlpha = m_surface_composite_alpha;
    swapchain_ci.presentMode = VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR;

    vkt::Swapchain swapchain(*m_device, swapchain_ci);
    const auto images = swapchain.GetImages();

    vkt::Fence fence(*m_device);
    const uint32_t image_index = swapchain.AcquireNextImage(fence, kWaitTimeout);
    fence.Wait(kWaitTimeout);
    SetPresentImageLayout(images[image_index]);

    vkt::Semaphore semaphore(*m_device);

    m_default_queue->Submit(vkt::no_cmd, vkt::Signal(semaphore));
    m_default_queue->Present(swapchain, image_index, semaphore);

    // For poor apps without swapchain_maintenance1 that use shared present modes
    // and call AcquireNextImage only once, the present semaphore in-use check is disabled.
    // The app doesn't have an official ways to do this better.
    // If supported, swapchain_maintenance1 should be used in such scenario.
    m_default_queue->Submit(vkt::no_cmd, vkt::Signal(semaphore));
    m_default_queue->Present(swapchain, image_index, semaphore);
    m_default_queue->Wait();
}

TEST_F(PositiveWsi, SharedPresentReuseSemaphoreAfterDestroy) {
    TEST_DESCRIPTION("After swapchain with shared present mode is destroyed the present semaphore can be reused");
    AddRequiredExtensions(VK_KHR_SHARED_PRESENTABLE_IMAGE_EXTENSION_NAME);
    AddSurfaceExtension();
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSurface());
    InitSwapchainInfo();

    bool found = false;
    for (VkPresentModeKHR present_mode : m_surface_present_modes) {
        found |= (present_mode == VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR);
    }
    if (!found) {
        GTEST_SKIP() << "Cannot find shared present mode";
    }

    VkSwapchainCreateInfoKHR swapchain_ci = vku::InitStructHelper();
    swapchain_ci.surface = m_surface.Handle();
    swapchain_ci.minImageCount = 1;
    swapchain_ci.imageFormat = m_surface_formats[0].format;
    swapchain_ci.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_ci.imageExtent = m_surface_capabilities.minImageExtent;
    swapchain_ci.imageArrayLayers = 1;
    swapchain_ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;  // implementations must support
    swapchain_ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_ci.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_ci.compositeAlpha = m_surface_composite_alpha;
    swapchain_ci.presentMode = VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR;

    vkt::Swapchain swapchain(*m_device, swapchain_ci);
    const auto images = swapchain.GetImages();
    for (auto image : images) {
        SetPresentImageLayout(image);
    }

    vkt::Fence fence(*m_device);
    const uint32_t image_index = swapchain.AcquireNextImage(fence, kWaitTimeout);
    fence.Wait(kWaitTimeout);
    fence.Reset();

    vkt::Semaphore semaphore(*m_device);
    vkt::Semaphore semaphore2(*m_device);

    m_default_queue->Submit(vkt::no_cmd, vkt::Signal(semaphore));
    m_default_queue->Present(swapchain, image_index, semaphore);
    m_default_queue->Submit(vkt::no_cmd, vkt::Signal(semaphore2));
    m_default_queue->Present(swapchain, image_index, semaphore2);

    swapchain_ci.minImageCount = m_surface_capabilities.minImageCount;
    swapchain_ci.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    swapchain_ci.oldSwapchain = swapchain;
    vkt::Swapchain swapchain2(*m_device, swapchain_ci);
    const auto images2 = swapchain2.GetImages();

    const uint32_t image_index2 = swapchain2.AcquireNextImage(fence, kWaitTimeout);
    fence.Wait(kWaitTimeout);
    fence.Reset();

    // Destroy swapchain!
    swapchain.Destroy();

    // Transition layout manually, because SetPresentImageLayout calls QueueWaitIdle which
    // resets semaphore swapchain state and this is not what we want for this test.
    VkImageMemoryBarrier present_transition = vku::InitStructHelper();
    present_transition.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    present_transition.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    present_transition.image = images2[image_index2];
    present_transition.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    m_command_buffer.Begin();
    vk::CmdPipelineBarrier(m_command_buffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr,
                           0, nullptr, 1, &present_transition);
    m_command_buffer.End();

    // Test that semaphore does not assume it is still in use by swapchain that was just deleted.
    // Swapchain2 is created with FIFO mode in order to enable semaphore-in-use-by-swapchain check
    // (it is disabled for shared present modes)
    m_default_queue->Submit(m_command_buffer, vkt::Signal(semaphore));
    m_default_queue->Present(swapchain2, image_index2, semaphore);

    m_default_queue->Wait();
}

TEST_F(PositiveWsi, PresentIdWait2) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddSurfaceExtension();
    AddRequiredExtensions(VK_KHR_PRESENT_ID_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_PRESENT_WAIT_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::presentId2);
    AddRequiredFeature(vkt::Feature::presentWait2);
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSurface());
    RETURN_IF_SKIP(InitSwapchainInfo());

    VkSwapchainCreateInfoKHR swapchain_ci = vku::InitStructHelper();
    swapchain_ci.flags = VK_SWAPCHAIN_CREATE_PRESENT_ID_2_BIT_KHR | VK_SWAPCHAIN_CREATE_PRESENT_WAIT_2_BIT_KHR;
    swapchain_ci.surface = m_surface.Handle();
    swapchain_ci.minImageCount = m_surface_capabilities.minImageCount;
    swapchain_ci.imageFormat = m_surface_formats[0].format;
    swapchain_ci.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_ci.imageExtent = m_surface_capabilities.minImageExtent;
    swapchain_ci.imageArrayLayers = 1u;
    swapchain_ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_ci.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_ci.compositeAlpha = m_surface_composite_alpha;
    swapchain_ci.presentMode = m_surface_non_shared_present_mode;
    swapchain_ci.clipped = VK_FALSE;
    swapchain_ci.oldSwapchain = VK_NULL_HANDLE;
    vkt::Swapchain swapchain(*m_device, swapchain_ci);

    VkSurfaceCapabilitiesPresentWait2KHR present_wait_2_capabilities = vku::InitStructHelper();
    VkSurfaceCapabilitiesPresentId2KHR present_id_2_capabilities = vku::InitStructHelper(&present_wait_2_capabilities);
    VkSurfaceCapabilities2KHR capabilities2 = vku::InitStructHelper(&present_id_2_capabilities);
    VkPhysicalDeviceSurfaceInfo2KHR surface_info = vku::InitStructHelper();
    surface_info.surface = m_surface.Handle();
    vk::GetPhysicalDeviceSurfaceCapabilities2KHR(gpu_, &surface_info, &capabilities2);

    if (!present_id_2_capabilities.presentId2Supported || !present_wait_2_capabilities.presentWait2Supported) {
        GTEST_SKIP() << "presentId2 and presentWait2 are not supported for the surface";
    }

    const auto images = swapchain.GetImages();

    uint64_t present_id_value = 1u;
    // TODO - understand why this timeout if set the loop to a large value
    for (uint32_t i = 0; i < 4; ++i) {
        vkt::Fence fence(*m_device);
        const uint32_t image_index = swapchain.AcquireNextImage(fence, kWaitTimeout);
        vk::WaitForFences(device(), 1, &fence.handle(), true, kWaitTimeout);

        SetPresentImageLayout(images[image_index]);

        VkPresentId2KHR present_id = vku::InitStructHelper();
        present_id.swapchainCount = 1u;
        present_id.pPresentIds = &present_id_value;

        m_default_queue->Present(swapchain, image_index, vkt::no_semaphore, &present_id);

        VkPresentWait2InfoKHR present_wait_2_info = vku::InitStructHelper();
        present_wait_2_info.presentId = present_id_value;
        present_wait_2_info.timeout = kWaitTimeout;
        vk::WaitForPresent2KHR(device(), swapchain, &present_wait_2_info);

        ++present_id_value;
    }
}

TEST_F(PositiveWsi, DestroySemaphoreUsedByOldSwapchain) {
    // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/10364
    TEST_DESCRIPTION("Safely destroy semaphores used by the old swapchain");
    AddSurfaceExtension();
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSurface());
    const SurfaceInformation info = GetSwapchainInfo(m_surface.Handle());

    VkSwapchainCreateInfoKHR swapchain_ci = vku::InitStructHelper();
    swapchain_ci.surface = m_surface.Handle();
    swapchain_ci.minImageCount = info.surface_capabilities.minImageCount;
    swapchain_ci.imageFormat = info.surface_formats[0].format;
    swapchain_ci.imageColorSpace = info.surface_formats[0].colorSpace;
    swapchain_ci.imageExtent = info.surface_capabilities.minImageExtent;
    swapchain_ci.imageArrayLayers = 1;
    swapchain_ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_ci.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_ci.compositeAlpha = info.surface_composite_alpha;
    swapchain_ci.presentMode = VK_PRESENT_MODE_FIFO_KHR;

    // Create swapchain and acquire the image (but do not present it yet)
    vkt::Swapchain swapchain(*m_device, swapchain_ci);
    const auto swapchain_images = swapchain.GetImages();
    for (auto image : swapchain_images) {
        SetPresentImageLayout(image);
    }
    vkt::Semaphore semaphore(*m_device);
    uint32_t image_index = swapchain.AcquireNextImage(semaphore, kWaitTimeout);

    // Create new_swapchain that specifies oldSwapchain
    swapchain_ci.oldSwapchain = swapchain;
    vkt::Swapchain new_swapchain(*m_device, swapchain_ci);
    const auto new_swapchain_images = new_swapchain.GetImages();
    if (new_swapchain_images.size() != 2) {
        GTEST_SKIP() << "The test requires swapchain with 2 images";
    }
    for (auto image : new_swapchain_images) {
        SetPresentImageLayout(image);
    }

    // Present already acquired image from the old swapchain.
    // This operation might succeed or not (VkResult status), but it is allowed to do this.
    m_default_queue->Present(swapchain, image_index, semaphore);

    // Acquire present images from the new swapchain until we reacquire some image the second time.
    // a) Get image 0
    vkt::Semaphore semaphore0(*m_device);
    image_index = new_swapchain.AcquireNextImage(semaphore0, kWaitTimeout);
    if (image_index != 0) {
        GTEST_SKIP() << "This test requires specific sequence of swapchain image indices. The first image index needs to be 0";
    }
    m_default_queue->Present(new_swapchain, image_index, semaphore0);

    // b) Get image 1
    vkt::Semaphore semaphore1(*m_device);
    image_index = new_swapchain.AcquireNextImage(semaphore1, kWaitTimeout);
    if (image_index != 1) {
        GTEST_SKIP() << "This test requires specific sequence of swapchain image indices. The second image index needs to be 1";
    }
    m_default_queue->Present(new_swapchain, image_index, semaphore1);

    // c) Get Image 0 again
    vkt::Fence fence(*m_device);
    image_index = new_swapchain.AcquireNextImage(fence, kWaitTimeout);
    if (image_index != 0) {
        GTEST_SKIP() << "This test requires specific sequence of swapchain image indices. The third image index needs to be 0";
    }
    fence.Wait(kWaitTimeout);

    // The image 0 from the new swapchain was reacquired and its fence was waited on.
    // It means that the previous (first) presentation of image 0 from new swapchain has completed.
    // Then it also means that the old swapchain presentation is also completed,
    // so we can reuse semaphore used with the old swapchain.
    m_default_queue->Submit(vkt::no_cmd, vkt::Signal(semaphore));

    m_default_queue->Wait();
}

TEST_F(PositiveWsi, DestroySemaphoreUsedByOldSwapchain2) {
    // NOTE: this test is similar to DestroySemaphoreUsedByOldSwapchain.
    // The only difference it pairs Present with Submit and uses Fence to wait for Acquire.
    AddSurfaceExtension();
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSurface());
    const SurfaceInformation info = GetSwapchainInfo(m_surface.Handle());

    VkSwapchainCreateInfoKHR swapchain_ci = vku::InitStructHelper();
    swapchain_ci.surface = m_surface.Handle();
    swapchain_ci.minImageCount = info.surface_capabilities.minImageCount;
    swapchain_ci.imageFormat = info.surface_formats[0].format;
    swapchain_ci.imageColorSpace = info.surface_formats[0].colorSpace;
    swapchain_ci.imageExtent = info.surface_capabilities.minImageExtent;
    swapchain_ci.imageArrayLayers = 1;
    swapchain_ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_ci.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_ci.compositeAlpha = info.surface_composite_alpha;
    swapchain_ci.presentMode = VK_PRESENT_MODE_FIFO_KHR;

    vkt::Swapchain swapchain(*m_device, swapchain_ci);
    const auto swapchain_images = swapchain.GetImages();
    for (auto image : swapchain_images) {
        SetPresentImageLayout(image);
    }
    vkt::Fence fence(*m_device);
    uint32_t image_index = swapchain.AcquireNextImage(fence, kWaitTimeout);
    fence.Wait(kWaitTimeout);

    swapchain_ci.oldSwapchain = swapchain;
    vkt::Swapchain new_swapchain(*m_device, swapchain_ci);
    const auto new_swapchain_images = new_swapchain.GetImages();
    if (new_swapchain_images.size() != 2) {
        GTEST_SKIP() << "The test requires swapchain with 2 images";
    }
    for (auto image : new_swapchain_images) {
        SetPresentImageLayout(image);
    }

    vkt::Semaphore semaphore(*m_device);
    m_default_queue->Submit(vkt::no_cmd, vkt::Signal(semaphore));
    m_default_queue->Present(swapchain, image_index, semaphore);

    vkt::Fence fence0(*m_device);
    image_index = new_swapchain.AcquireNextImage(fence0, kWaitTimeout);
    if (image_index != 0) {
        GTEST_SKIP() << "This test requires specific sequence of swapchain image indices. The first image index needs to be 0";
    }
    fence0.Wait(kWaitTimeout);
    vkt::Semaphore semaphore0(*m_device);
    m_default_queue->Submit(vkt::no_cmd, vkt::Signal(semaphore0));
    m_default_queue->Present(new_swapchain, image_index, semaphore0);

    vkt::Fence fence1(*m_device);
    image_index = new_swapchain.AcquireNextImage(fence1, kWaitTimeout);
    if (image_index != 1) {
        GTEST_SKIP() << "This test requires specific sequence of swapchain image indices. The second image index needs to be 1";
    }
    fence1.Wait(kWaitTimeout);
    vkt::Semaphore semaphore1(*m_device);
    m_default_queue->Submit(vkt::no_cmd, vkt::Signal(semaphore1));
    m_default_queue->Present(new_swapchain, image_index, semaphore1);

    vkt::Fence fence2(*m_device);
    image_index = new_swapchain.AcquireNextImage(fence2, kWaitTimeout);
    if (image_index != 0) {
        GTEST_SKIP() << "This test requires specific sequence of swapchain image indices. The third image index needs to be 0";
    }
    fence2.Wait(kWaitTimeout);

    m_default_queue->Submit(vkt::no_cmd, vkt::Signal(semaphore));
    m_default_queue->Wait();
}

TEST_F(PositiveWsi, DestroySwapchainBeforeLayoutValidation) {
    // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/10455
    TEST_DESCRIPTION("Delete swapchain before global layout validation takes place");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddSurfaceExtension();
    AddRequiredExtensions(VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::swapchainMaintenance1);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSurface());
    const SurfaceInformation info = GetSwapchainInfo(m_surface.Handle());

    VkSwapchainCreateInfoKHR swapchain_ci = vku::InitStructHelper();
    swapchain_ci.surface = m_surface.Handle();
    swapchain_ci.minImageCount = info.surface_capabilities.minImageCount;
    swapchain_ci.imageFormat = info.surface_formats[0].format;
    swapchain_ci.imageColorSpace = info.surface_formats[0].colorSpace;
    swapchain_ci.imageExtent = info.surface_capabilities.minImageExtent;
    swapchain_ci.imageArrayLayers = 1;
    swapchain_ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_ci.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_ci.compositeAlpha = info.surface_composite_alpha;
    swapchain_ci.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    vkt::Swapchain swapchain(*m_device, swapchain_ci);

    const auto swapchain_images = swapchain.GetImages();

    vkt::Semaphore render_semaphore(*m_device);
    vkt::Fence fence(*m_device);

    const uint32_t image_index = swapchain.AcquireNextImage(fence, kWaitTimeout);
    fence.Wait(kWaitTimeout);

    VkImageMemoryBarrier2 layout_transition = vku::InitStructHelper();
    layout_transition.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    layout_transition.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    layout_transition.image = swapchain_images[image_index];
    layout_transition.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    m_command_buffer.Begin();
    m_command_buffer.Barrier(layout_transition);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer, vkt::Signal(render_semaphore));
    m_default_queue->Present(swapchain, image_index, render_semaphore);

    swapchain_ci.oldSwapchain = swapchain.handle();
    vkt::Swapchain swapchain2(*m_device, swapchain_ci);
    swapchain.Destroy();

    // In current implementation global layout validation is performed by the queue threads and it happens
    // usually at sync points, for example, during the next Wait(). Test that layout validation does not
    // cause false positives here.
    m_default_queue->Wait();
}

TEST_F(PositiveWsi, PresentTimings) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddSurfaceExtension();
    AddRequiredExtensions(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_PRESENT_TIMING_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_PRESENT_ID_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_PRESENT_WAIT_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::presentId2);
    AddRequiredFeature(vkt::Feature::presentWait2);
    AddRequiredFeature(vkt::Feature::presentTiming);
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSurface());
    InitSwapchainInfo();

    VkSurfaceCapabilitiesPresentWait2KHR present_wait_2_capabilities = vku::InitStructHelper();
    VkSurfaceCapabilitiesPresentId2KHR present_id_2_capabilities = vku::InitStructHelper(&present_wait_2_capabilities);
    VkSurfaceCapabilities2KHR capabilities2 = vku::InitStructHelper(&present_id_2_capabilities);
    VkPhysicalDeviceSurfaceInfo2KHR surface_info = vku::InitStructHelper();
    surface_info.surface = m_surface.Handle();
    vk::GetPhysicalDeviceSurfaceCapabilities2KHR(gpu_, &surface_info, &capabilities2);

    if (!present_id_2_capabilities.presentId2Supported || !present_wait_2_capabilities.presentWait2Supported) {
        GTEST_SKIP() << "presentId2 and presentWait2 are not supported for the surface";
    }

    VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;

    uint32_t present_mode_count = 0;
    vk::GetPhysicalDeviceSurfacePresentModesKHR(gpu_, m_surface.Handle(), &present_mode_count, nullptr);
    std::vector<VkPresentModeKHR> present_modes(present_mode_count);
    vk::GetPhysicalDeviceSurfacePresentModesKHR(gpu_, m_surface.Handle(), &present_mode_count, present_modes.data());

    bool found = false;
    for (const auto& available_mode : present_modes) {
        if (available_mode == present_mode) {
            found = true;
            break;
        }
    }
    if (!found) {
        GTEST_SKIP() << "Required present mode " << present_mode << " not supported";
    }

    VkSwapchainCreateInfoKHR swapchain_ci = vku::InitStructHelper();
    swapchain_ci.flags = VK_SWAPCHAIN_CREATE_PRESENT_TIMING_BIT_EXT | VK_SWAPCHAIN_CREATE_PRESENT_ID_2_BIT_KHR;
    swapchain_ci.surface = m_surface.Handle();
    swapchain_ci.minImageCount = m_surface_capabilities.minImageCount;
    swapchain_ci.imageFormat = m_surface_formats[0].format;
    swapchain_ci.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_ci.imageExtent = m_surface_capabilities.minImageExtent;
    swapchain_ci.imageArrayLayers = 1;
    swapchain_ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_ci.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_ci.compositeAlpha = m_surface_composite_alpha;
    swapchain_ci.presentMode = present_mode;
    swapchain_ci.clipped = VK_FALSE;
    swapchain_ci.oldSwapchain = 0;
    vkt::Swapchain swapchain(*m_device, swapchain_ci);
    const auto images = swapchain.GetImages();

    vkt::Fence fence(*m_device);
    const uint32_t image_index = swapchain.AcquireNextImage(fence, kWaitTimeout);
    vk::WaitForFences(device(), 1, &fence.handle(), true, kWaitTimeout);

    SetPresentImageLayout(images[image_index]);

    uint64_t present_id_value = 1u;
    VkPresentId2KHR present_id = vku::InitStructHelper();
    present_id.swapchainCount = 1u;
    present_id.pPresentIds = &present_id_value;

    VkPresentTimingInfoEXT present_timing_info = vku::InitStructHelper();
    present_timing_info.flags = VK_PRESENT_TIMING_INFO_PRESENT_AT_RELATIVE_TIME_BIT_EXT;
    present_timing_info.targetTime = 1u;
    present_timing_info.presentStageQueries = VK_PRESENT_STAGE_IMAGE_FIRST_PIXEL_OUT_BIT_EXT;

    VkPresentTimingsInfoEXT present_timings_info = vku::InitStructHelper(&present_id);
    present_timings_info.swapchainCount = 1u;
    present_timings_info.pTimingInfos = &present_timing_info;
    m_default_queue->Present(swapchain, image_index, vkt::no_semaphore, &present_timings_info);

    VkPastPresentationTimingInfoEXT past_presentation_timing_info = vku::InitStructHelper();
    past_presentation_timing_info.flags = VK_PAST_PRESENTATION_TIMING_ALLOW_OUT_OF_ORDER_RESULTS_BIT_EXT;
    past_presentation_timing_info.swapchain = swapchain;

    VkPresentStageTimeEXT present_stage_time;
    present_stage_time.stage = VK_PRESENT_STAGE_IMAGE_FIRST_PIXEL_OUT_BIT_EXT;
    present_stage_time.time = 0u;

    VkPastPresentationTimingEXT past_presentation_timing = vku::InitStructHelper();
    past_presentation_timing.presentId = present_id_value;
    past_presentation_timing.targetTime = 1u;
    past_presentation_timing.presentStageCount = 1u;
    past_presentation_timing.pPresentStages = &present_stage_time;
    past_presentation_timing.timeDomain = VK_TIME_DOMAIN_PRESENT_STAGE_LOCAL_EXT;

    VkPastPresentationTimingPropertiesEXT past_presentation_timing_properties = vku::InitStructHelper();
    past_presentation_timing_properties.presentationTimingCount = 1u;
    past_presentation_timing_properties.pPresentationTimings = &past_presentation_timing;
    vk::GetPastPresentationTimingEXT(device(), &past_presentation_timing_info, &past_presentation_timing_properties);

    std::vector<VkPastPresentationTimingEXT> past_presentation_timings(past_presentation_timing_properties.presentationTimingCount);
    for (uint32_t i = 0; i < past_presentation_timing_properties.presentationTimingCount; ++i) {
        past_presentation_timings[i] = vku::InitStructHelper();
        past_presentation_timings[i].presentStageCount = 1u;
        past_presentation_timings[i].pPresentStages = &present_stage_time;
    }
    past_presentation_timing_properties.pPresentationTimings = past_presentation_timings.data();
    vk::GetPastPresentationTimingEXT(device(), &past_presentation_timing_info, &past_presentation_timing_properties);
}
