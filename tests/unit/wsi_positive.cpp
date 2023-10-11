/*
 * Copyright (c) 2023 Valve Corporation
 * Copyright (c) 2023 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */
#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "generated/vk_extension_helper.h"

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
#include "wayland-client.h"
#endif

TEST_F(PositiveWsi, CreateWaylandSurface) {
    TEST_DESCRIPTION("Test creating wayland surface");

#ifndef VK_USE_PLATFORM_WAYLAND_KHR
    GTEST_SKIP() << "test not supported on platform";
#else
    AddSurfaceExtension();

    RETURN_IF_SKIP(Init())

    if (!IsExtensionsEnabled(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME)) {
        GTEST_SKIP() << VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME << " extension not supported.";
    }

    wl_display *display = nullptr;
    wl_registry *registry = nullptr;
    wl_surface *surface = nullptr;
    wl_compositor *compositor = nullptr;
    {
        display = wl_display_connect(nullptr);
        if (!display) {
            GTEST_SKIP() << "couldn't create wayland surface";
        }

        auto global = [](void *data, struct wl_registry *registry, uint32_t id, const char *interface, uint32_t version) {
            (void)version;
            const std::string_view interface_str = interface;
            if (interface_str == "wl_compositor") {
                auto compositor = reinterpret_cast<wl_compositor **>(data);
                *compositor = reinterpret_cast<wl_compositor *>(wl_registry_bind(registry, id, &wl_compositor_interface, 1));
            }
        };

        auto global_remove = [](void *data, struct wl_registry *registry, uint32_t id) {
            (void)data;
            (void)registry;
            (void)id;
        };

        registry = wl_display_get_registry(display);
        ASSERT_TRUE(registry != nullptr);

        const wl_registry_listener registry_listener = {global, global_remove};

        wl_registry_add_listener(registry, &registry_listener, &compositor);

        wl_display_dispatch(display);
        ASSERT_TRUE(compositor);

        surface = wl_compositor_create_surface(compositor);
        ASSERT_TRUE(surface);

        const uint32_t version = wl_surface_get_version(surface);
        ASSERT_TRUE(version > 0);
    }

    VkWaylandSurfaceCreateInfoKHR surface_create_info = vku::InitStructHelper();
    surface_create_info.display = display;
    surface_create_info.surface = surface;

    VkSurfaceKHR vulkan_surface;
    vk::CreateWaylandSurfaceKHR(instance(), &surface_create_info, nullptr, &vulkan_surface);

    vk::DestroySurfaceKHR(instance(), vulkan_surface, nullptr);
    wl_surface_destroy(surface);
    wl_compositor_destroy(compositor);
    wl_registry_destroy(registry);
    wl_display_disconnect(display);
#endif
}

TEST_F(PositiveWsi, CreateXcbSurface) {
    TEST_DESCRIPTION("Test creating xcb surface");

#ifndef VK_USE_PLATFORM_XCB_KHR
    GTEST_SKIP() << "test not supported on platform";
#else
    AddSurfaceExtension();

    RETURN_IF_SKIP(Init())

    if (!IsExtensionsEnabled(VK_KHR_XCB_SURFACE_EXTENSION_NAME)) {
        GTEST_SKIP() << VK_KHR_XCB_SURFACE_EXTENSION_NAME << " not supported.";
    }

    xcb_connection_t *xcb_connection = xcb_connect(nullptr, nullptr);
    ASSERT_TRUE(xcb_connection);

    // NOTE: This is technically an invalid window! (There is no width/height)
    // But there is no robust way to check for a valid window without crashing the app.
    xcb_window_t xcb_window = xcb_generate_id(xcb_connection);
    ASSERT_TRUE(xcb_window != 0);

    VkXcbSurfaceCreateInfoKHR surface_create_info = vku::InitStructHelper();
    surface_create_info.connection = xcb_connection;
    surface_create_info.window = xcb_window;

    VkSurfaceKHR vulkan_surface;
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

    RETURN_IF_SKIP(Init())

    if (!IsExtensionsEnabled(VK_KHR_XLIB_SURFACE_EXTENSION_NAME)) {
        GTEST_SKIP() << VK_KHR_XLIB_SURFACE_EXTENSION_NAME << " not supported.";
    }

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
    RETURN_IF_SKIP(InitFramework())

    if (!IsPlatformMockICD()) {
        GTEST_SKIP() << "Only run test MockICD due to CI stability";
    }

    RETURN_IF_SKIP(InitState())
    InitRenderTarget();
    if (!InitSwapchain()) {
        GTEST_SKIP() << "Cannot create surface or swapchain";
    }

    const POINT pt_zero = {0, 0};

    VkSurfaceFullScreenExclusiveWin32InfoEXT fullscreen_exclusive_win32_info = vku::InitStructHelper();
    fullscreen_exclusive_win32_info.hmonitor = MonitorFromPoint(pt_zero, MONITOR_DEFAULTTOPRIMARY);
    VkSurfaceFullScreenExclusiveInfoEXT fullscreen_exclusive_info = vku::InitStructHelper(&fullscreen_exclusive_win32_info);
    fullscreen_exclusive_info.fullScreenExclusive = VK_FULL_SCREEN_EXCLUSIVE_APPLICATION_CONTROLLED_EXT;

    VkPhysicalDeviceSurfaceInfo2KHR surface_info = vku::InitStructHelper(&fullscreen_exclusive_info);
    surface_info.surface = m_surface;

    VkSurfaceCapabilities2KHR surface_caps = vku::InitStructHelper();
    vk::GetPhysicalDeviceSurfaceCapabilities2KHR(m_device->phy(), &surface_info, &surface_caps);
}
#endif

TEST_F(PositiveWsi, CmdCopySwapchainImage) {
    TEST_DESCRIPTION("Run vkCmdCopyImage with a swapchain image");

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    printf(
        "According to valid usage, VkBindImageMemoryInfo-memory should be NULL. But Android will crash if memory is NULL, "
        "skipping CmdCopySwapchainImage test\n");
    return;
#endif

    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddSurfaceExtension();
    RETURN_IF_SKIP(InitFramework())

    if (IsDriver(VK_DRIVER_ID_MESA_RADV)) {
        // Seeing the same crash as the Android comment above
        GTEST_SKIP() << "This test should not be run on the RADV driver";
    }

    RETURN_IF_SKIP(InitState())
    InitRenderTarget();
    if (!InitSwapchain(VK_IMAGE_USAGE_TRANSFER_DST_BIT)) {
        GTEST_SKIP() << "Cannot create surface or swapchain";
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

    VkImageObj srcImage(m_device);
    srcImage.init(&image_create_info);

    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    VkImageSwapchainCreateInfoKHR image_swapchain_create_info = vku::InitStructHelper();
    image_swapchain_create_info.swapchain = m_swapchain;
    image_create_info.pNext = &image_swapchain_create_info;

    vkt::Image image_from_swapchain(*m_device, image_create_info, vkt::no_mem);

    VkBindImageMemorySwapchainInfoKHR bind_swapchain_info = vku::InitStructHelper();
    bind_swapchain_info.swapchain = m_swapchain;
    bind_swapchain_info.imageIndex = 0;

    VkBindImageMemoryInfo bind_info = vku::InitStructHelper(&bind_swapchain_info);
    bind_info.image = image_from_swapchain.handle();
    bind_info.memory = VK_NULL_HANDLE;
    bind_info.memoryOffset = 0;

    vk::BindImageMemory2(m_device->device(), 1, &bind_info);

    VkImageCopy copy_region = {};
    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.srcSubresource.mipLevel = 0;
    copy_region.srcSubresource.baseArrayLayer = 0;
    copy_region.srcSubresource.layerCount = 1;
    copy_region.dstSubresource = copy_region.srcSubresource;
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstOffset = {0, 0, 0};
    copy_region.extent = {std::min(10u, m_surface_capabilities.minImageExtent.width),
                          std::min(10u, m_surface_capabilities.minImageExtent.height), 1};

    m_commandBuffer->begin();

    vk::CmdCopyImage(m_commandBuffer->handle(), srcImage.handle(), VK_IMAGE_LAYOUT_GENERAL, image_from_swapchain.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
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

    RETURN_IF_SKIP(InitFramework())

    if (IsDriver(VK_DRIVER_ID_MESA_RADV)) {
        // Seeing the same crash as the Android comment above
        GTEST_SKIP() << "This test should not be run on the RADV driver";
    }

    uint32_t physical_device_group_count = 0;
    vk::EnumeratePhysicalDeviceGroups(instance(), &physical_device_group_count, nullptr);

    if (physical_device_group_count == 0) {
        GTEST_SKIP() << "physical_device_group_count is 0, skipping test";
    }

    std::vector<VkPhysicalDeviceGroupProperties> physical_device_group(physical_device_group_count,
                                                                       {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES});
    vk::EnumeratePhysicalDeviceGroups(instance(), &physical_device_group_count, physical_device_group.data());
    VkDeviceGroupDeviceCreateInfo create_device_pnext = vku::InitStructHelper();
    create_device_pnext.physicalDeviceCount = physical_device_group[0].physicalDeviceCount;
    create_device_pnext.pPhysicalDevices = physical_device_group[0].physicalDevices;
    RETURN_IF_SKIP(InitState(nullptr, &create_device_pnext));
    InitRenderTarget();
    if (!InitSwapchain(VK_IMAGE_USAGE_TRANSFER_DST_BIT)) {
        GTEST_SKIP() << "Cannot create surface or swapchain";
    }

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

    VkImageObj src_Image(m_device);
    src_Image.init(&image_create_info);

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
    bind_info.image = peer_image.handle();
    bind_info.memory = VK_NULL_HANDLE;
    bind_info.memoryOffset = 0;

    vk::BindImageMemory2(m_device->device(), 1, &bind_info);

    const auto swapchain_images = GetSwapchainImages(m_swapchain);

    vkt::Fence fence;
    fence.init(*m_device, vkt::Fence::create_info());
    VkFence fence_handle = fence.handle();

    uint32_t image_index;
    vk::AcquireNextImageKHR(m_device->handle(), m_swapchain, kWaitTimeout, VK_NULL_HANDLE, fence_handle, &image_index);
    vk::WaitForFences(m_device->device(), 1, &fence_handle, VK_TRUE, kWaitTimeout);

    m_commandBuffer->begin();

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
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0,
                           nullptr, 0, nullptr, 1, &img_barrier);

    VkImageCopy copy_region = {};
    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.srcSubresource.mipLevel = 0;
    copy_region.srcSubresource.baseArrayLayer = 0;
    copy_region.srcSubresource.layerCount = 1;
    copy_region.dstSubresource = copy_region.srcSubresource;
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstOffset = {0, 0, 0};
    copy_region.extent = {test_extent_value, test_extent_value, 1};
    vk::CmdCopyImage(m_commandBuffer->handle(), src_Image.handle(), VK_IMAGE_LAYOUT_GENERAL, peer_image.handle(),
                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);

    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer();
}

TEST_F(PositiveWsi, SwapchainAcquireImageAndPresent) {
    TEST_DESCRIPTION("Test acquiring swapchain image and then presenting it.");
    AddSurfaceExtension();
    RETURN_IF_SKIP(InitFramework())
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

    VkImageMemoryBarrier img_barrier = vku::InitStructHelper();
    img_barrier.srcAccessMask = 0;
    img_barrier.dstAccessMask = 0;
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    img_barrier.image = swapchain_images[image_index];
    img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    img_barrier.subresourceRange.baseArrayLayer = 0;
    img_barrier.subresourceRange.baseMipLevel = 0;
    img_barrier.subresourceRange.layerCount = 1;
    img_barrier.subresourceRange.levelCount = 1;

    m_commandBuffer->begin();
    vk::CmdPipelineBarrier(*m_commandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr,
                           0, nullptr, 1, &img_barrier);
    m_commandBuffer->end();

    constexpr VkPipelineStageFlags stage_mask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &acquire_semaphore.handle();
    submit_info.pWaitDstStageMask = &stage_mask;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &submit_semaphore.handle();
    ASSERT_EQ(VK_SUCCESS, vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE));

    VkPresentInfoKHR present = vku::InitStructHelper();
    present.waitSemaphoreCount = 1;
    present.pWaitSemaphores = &submit_semaphore.handle();
    present.swapchainCount = 1;
    present.pSwapchains = &m_swapchain;
    present.pImageIndices = &image_index;
    ASSERT_EQ(VK_SUCCESS, vk::QueuePresentKHR(m_default_queue, &present));
    ASSERT_EQ(VK_SUCCESS, vk::QueueWaitIdle(m_default_queue));
}

TEST_F(PositiveWsi, SwapchainAcquireImageAndWaitForFence) {
    TEST_DESCRIPTION("Test waiting on swapchain image with a fence.");
    AddSurfaceExtension();
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())
    if (!InitSwapchain()) {
        GTEST_SKIP() << "Cannot create surface or swapchain";
    }

    const vkt::Fence fence(*m_device);
    const auto swapchain_images = GetSwapchainImages(m_swapchain);

    uint32_t image_index = 0;
    ASSERT_EQ(VK_SUCCESS, vk::AcquireNextImageKHR(device(), m_swapchain, kWaitTimeout, VK_NULL_HANDLE, fence, &image_index));
    ASSERT_EQ(VK_SUCCESS, vk::WaitForFences(device(), 1, &fence.handle(), VK_TRUE, kWaitTimeout));

    VkImageMemoryBarrier img_barrier = vku::InitStructHelper();
    img_barrier.srcAccessMask = 0;
    img_barrier.dstAccessMask = 0;
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    img_barrier.image = swapchain_images[image_index];
    img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    img_barrier.subresourceRange.baseArrayLayer = 0;
    img_barrier.subresourceRange.baseMipLevel = 0;
    img_barrier.subresourceRange.layerCount = 1;
    img_barrier.subresourceRange.levelCount = 1;

    m_commandBuffer->begin();
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0,
                           nullptr, 0, nullptr, 1, &img_barrier);
    m_commandBuffer->end();

    constexpr VkPipelineStageFlags stage_mask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    submit_info.pWaitDstStageMask = &stage_mask;
    ASSERT_EQ(VK_SUCCESS, vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE));
    ASSERT_EQ(VK_SUCCESS, vk::QueueWaitIdle(m_default_queue));

    VkPresentInfoKHR present = vku::InitStructHelper();
    present.swapchainCount = 1;
    present.pSwapchains = &m_swapchain;
    present.pImageIndices = &image_index;
    ASSERT_EQ(VK_SUCCESS, vk::QueuePresentKHR(m_default_queue, &present));
    ASSERT_EQ(VK_SUCCESS, vk::QueueWaitIdle(m_default_queue));
}

TEST_F(PositiveWsi, SwapchainImageLayout) {
    AddSurfaceExtension();
    AddRequiredExtensions(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())
    if (!InitSwapchain()) {
        GTEST_SKIP() << "Cannot create surface or swapchain, skipping CmdCopySwapchainImage test";
    }
    const auto swapchainImages = GetSwapchainImages(m_swapchain);
    const vkt::Fence fence(*m_device);
    uint32_t image_index = 0;
    ASSERT_EQ(VK_SUCCESS,
        vk::AcquireNextImageKHR(m_device->handle(), m_swapchain, kWaitTimeout, VK_NULL_HANDLE, fence.handle(), &image_index));
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
    ivci.image = swapchainImages[image_index];
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = m_surface_formats[0].format;
    ivci.components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                       VK_COMPONENT_SWIZZLE_IDENTITY};
    ivci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    vkt::ImageView view(*m_device, ivci);
    ASSERT_TRUE(view.initialized());
    VkFramebufferCreateInfo fci = vku::InitStructHelper();
    fci.renderPass = rp1.handle();
    fci.attachmentCount = 1;
    fci.pAttachments = &view.handle();
    fci.width = 1;
    fci.height = 1;
    fci.layers = 1;
    vkt::Framebuffer fb1(*m_device, fci);
    ASSERT_TRUE(fb1.initialized());
    fci.renderPass = rp2.handle();
    vkt::Framebuffer fb2(*m_device, fci);
    ASSERT_TRUE(fb2.initialized());
    VkRenderPassBeginInfo rpbi =
        vku::InitStruct<VkRenderPassBeginInfo>(nullptr, rp1.handle(), fb1.handle(), VkRect2D{{0, 0}, {1u, 1u}}, 0u, nullptr);
    m_commandBuffer->begin();
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    m_commandBuffer->EndRenderPass();
    rpbi.framebuffer = fb2.handle();
    rpbi.renderPass = rp2.handle();
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    m_commandBuffer->EndRenderPass();

    VkImageMemoryBarrier img_barrier = vku::InitStructHelper();
    img_barrier.srcAccessMask = 0;
    img_barrier.dstAccessMask = 0;
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    img_barrier.image = swapchainImages[image_index];
    img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    img_barrier.subresourceRange.baseArrayLayer = 0;
    img_barrier.subresourceRange.baseMipLevel = 0;
    img_barrier.subresourceRange.layerCount = 1;
    img_barrier.subresourceRange.levelCount = 1;
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0,
                           nullptr, 0, nullptr, 1, &img_barrier);
    m_commandBuffer->end();
    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.waitSemaphoreCount = 0;
    submit_info.pWaitSemaphores = NULL;
    submit_info.pWaitDstStageMask = NULL;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    submit_info.signalSemaphoreCount = 0;
    submit_info.pSignalSemaphores = NULL;
    vk::WaitForFences(m_device->device(), 1, &fence.handle(), VK_TRUE, kWaitTimeout);
    vk::ResetFences(m_device->device(), 1, &fence.handle());
    vk::QueueSubmit(m_default_queue, 1, &submit_info, fence.handle());
    vk::WaitForFences(m_device->device(), 1, &fence.handle(), VK_TRUE, kWaitTimeout);
}

TEST_F(PositiveWsi, SwapchainPresentShared) {
    TEST_DESCRIPTION("Acquire shared presentable image and Present multiple times without failure.");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SHARED_PRESENTABLE_IMAGE_EXTENSION_NAME);
    AddSurfaceExtension();

    RETURN_IF_SKIP(InitFramework())

    RETURN_IF_SKIP(InitState())
    if (!InitSurface()) {
        GTEST_SKIP() << "Cannot create surface";
    }
    InitSwapchainInfo();

    VkBool32 supported;
    vk::GetPhysicalDeviceSurfaceSupportKHR(gpu(), m_device->graphics_queue_node_index_, m_surface, &supported);
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
    surface_info.surface = m_surface;
    vk::GetPhysicalDeviceSurfaceCapabilities2KHR(gpu(), &surface_info, &capabilities);

    // This was recently added to CTS, but some drivers might not correctly advertise the flag
    if ((shared_present_capabilities.sharedPresentSupportedUsageFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) == 0) {
        GTEST_SKIP() << "Driver was suppose to support VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT";
    }

    VkSwapchainCreateInfoKHR swapchain_create_info = vku::InitStructHelper();
    swapchain_create_info.surface = m_surface;
    swapchain_create_info.minImageCount = 1;
    swapchain_create_info.imageFormat = m_surface_formats[0].format;
    swapchain_create_info.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_create_info.imageExtent = {m_surface_capabilities.minImageExtent.width, m_surface_capabilities.minImageExtent.height};
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;  // implementations must support
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_create_info.compositeAlpha = m_surface_composite_alpha;
    swapchain_create_info.presentMode = shared_present_mode;
    swapchain_create_info.clipped = VK_FALSE;
    swapchain_create_info.oldSwapchain = 0;

    vk::CreateSwapchainKHR(device(), &swapchain_create_info, nullptr, &m_swapchain);
    const auto images = GetSwapchainImages(m_swapchain);

    uint32_t image_index;
    vkt::Fence fence;
    fence.init(*m_device, vkt::Fence::create_info());
    vk::AcquireNextImageKHR(device(), m_swapchain, kWaitTimeout, VK_NULL_HANDLE, fence.handle(), &image_index);
    vk::WaitForFences(device(), 1, &fence.handle(), true, kWaitTimeout);

    SetImageLayout(m_device, VK_IMAGE_ASPECT_COLOR_BIT, images[image_index], VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR);

    VkPresentInfoKHR present = vku::InitStructHelper();
    present.waitSemaphoreCount = 0;
    present.swapchainCount = 1;
    present.pSwapchains = &m_swapchain;
    present.pImageIndices = &image_index;
    vk::QueuePresentKHR(m_default_queue, &present);

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
        vk::QueuePresentKHR(m_default_queue, &present);
    }
}

TEST_F(PositiveWsi, CreateSurface) {
    TEST_DESCRIPTION("Create and destroy a surface without ever creating a swapchain");

    AddSurfaceExtension();

    RETURN_IF_SKIP(InitFramework())

    RETURN_IF_SKIP(InitState())

    if (!InitSurface()) {
        GTEST_SKIP() << "Cannot create surface";
    }
    DestroySwapchain();  // cleans up both surface and swapchain, if they were created
}

#if defined(VK_USE_PLATFORM_WIN32_KHR)
TEST_F(PositiveWsi, CreateSwapchainFullscreenExclusive) {
    TEST_DESCRIPTION(
        "Test creating a swapchain with VkSurfaceFullScreenExclusiveWin32InfoEXT and VK_FULL_SCREEN_EXCLUSIVE_DEFAULT_EXT");

    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddSurfaceExtension();
    AddRequiredExtensions(VK_EXT_FULL_SCREEN_EXCLUSIVE_EXTENSION_NAME);

    RETURN_IF_SKIP(InitFramework())

    if (!IsPlatformMockICD()) {
        GTEST_SKIP() << "Only run test MockICD due to CI stability";
    }

    RETURN_IF_SKIP(InitState())
    InitRenderTarget();
    if (!InitSwapchain()) {
        GTEST_SKIP() << "Cannot create surface or swapchain";
    }

    VkSurfaceFullScreenExclusiveInfoEXT surface_full_screen_exlusive_info = vku::InitStructHelper();
    surface_full_screen_exlusive_info.fullScreenExclusive = VK_FULL_SCREEN_EXCLUSIVE_DEFAULT_EXT;

    VkSwapchainCreateInfoKHR swapchain_create_info = vku::InitStructHelper(&surface_full_screen_exlusive_info);
    swapchain_create_info.flags = 0;
    swapchain_create_info.surface = m_surface;
    swapchain_create_info.minImageCount = m_surface_capabilities.minImageCount;
    swapchain_create_info.imageFormat = m_surface_formats[0].format;
    swapchain_create_info.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_create_info.imageExtent = {m_surface_capabilities.minImageExtent.width, m_surface_capabilities.minImageExtent.height};
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_create_info.compositeAlpha = m_surface_composite_alpha;
    swapchain_create_info.presentMode = m_surface_non_shared_present_mode;
    swapchain_create_info.clipped = VK_FALSE;

    VkSwapchainKHR swapchain = VK_NULL_HANDLE;

    vk::CreateSwapchainKHR(device(), &swapchain_create_info, nullptr, &swapchain);
    vk::DestroySwapchainKHR(device(), swapchain, nullptr);
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

    RETURN_IF_SKIP(InitFramework())

    if (!IsPlatformMockICD()) {
        GTEST_SKIP() << "Only run test MockICD due to CI stability";
    }

    RETURN_IF_SKIP(InitState())
    InitRenderTarget();
    if (!InitSwapchain()) {
        GTEST_SKIP() << "Cannot create surface or swapchain";
    }

    const POINT pt_zero = {0, 0};

    VkSurfaceFullScreenExclusiveWin32InfoEXT fullscreen_exclusive_win32_info = vku::InitStructHelper();
    fullscreen_exclusive_win32_info.hmonitor = MonitorFromPoint(pt_zero, MONITOR_DEFAULTTOPRIMARY);
    VkSurfaceFullScreenExclusiveInfoEXT surface_full_screen_exlusive_info = vku::InitStructHelper(&fullscreen_exclusive_win32_info);
    surface_full_screen_exlusive_info.fullScreenExclusive = VK_FULL_SCREEN_EXCLUSIVE_APPLICATION_CONTROLLED_EXT;

    VkSwapchainCreateInfoKHR swapchain_create_info = vku::InitStructHelper(&surface_full_screen_exlusive_info);
    swapchain_create_info.flags = 0;
    swapchain_create_info.surface = m_surface;
    swapchain_create_info.minImageCount = m_surface_capabilities.minImageCount;
    swapchain_create_info.imageFormat = m_surface_formats[0].format;
    swapchain_create_info.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_create_info.imageExtent = {m_surface_capabilities.minImageExtent.width, m_surface_capabilities.minImageExtent.height};
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_create_info.compositeAlpha = m_surface_composite_alpha;
    swapchain_create_info.presentMode = m_surface_non_shared_present_mode;
    swapchain_create_info.clipped = VK_FALSE;

    VkSwapchainKHR swapchain = VK_NULL_HANDLE;

    vk::CreateSwapchainKHR(device(), &swapchain_create_info, nullptr, &swapchain);
    vk::DestroySwapchainKHR(device(), swapchain, nullptr);
}
#endif

TEST_F(PositiveWsi, SwapchainImageFormatProps) {
    TEST_DESCRIPTION("Try using special format props on a swapchain image");

    AddSurfaceExtension();
    RETURN_IF_SKIP(InitFramework())

    RETURN_IF_SKIP(InitState())

    if (!InitSwapchain()) {
        GTEST_SKIP() << "Cannot create surface or swapchain";
    }

    // HACK: I know InitSwapchain() will pick first supported format
    VkSurfaceFormatKHR format_tmp;
    {
        uint32_t format_count = 1;
        const VkResult err = vk::GetPhysicalDeviceSurfaceFormatsKHR(gpu(), m_surface, &format_count, &format_tmp);
        ASSERT_TRUE(err == VK_SUCCESS || err == VK_INCOMPLETE) << string_VkResult(err);
    }
    const VkFormat format = format_tmp.format;

    VkFormatProperties format_props;
    vk::GetPhysicalDeviceFormatProperties(gpu(), format, &format_props);
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
    pipe.InitState();
    pipe.gp_ci_.renderPass = render_pass.handle();
    pipe.cb_attachments_[0] = pcbas;
    pipe.CreateGraphicsPipeline();

    const auto swapchain_images = GetSwapchainImages(m_swapchain);
    const vkt::Fence fence(*m_device);

    uint32_t image_index;
    ASSERT_EQ(VK_SUCCESS, vk::AcquireNextImageKHR(device(), m_swapchain, kWaitTimeout, VK_NULL_HANDLE, fence.handle(), &image_index));
    fence.wait(vvl::kU32Max);

    VkImageViewCreateInfo ivci = vku::InitStructHelper();
    ivci.image = swapchain_images[image_index];
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = format;
    ivci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    vkt::ImageView image_view(*m_device, ivci);

    VkFramebufferCreateInfo fbci = vku::InitStructHelper();
    fbci.renderPass = render_pass.handle();
    fbci.attachmentCount = 1;
    fbci.pAttachments = &image_view.handle();
    fbci.width = 1;
    fbci.height = 1;
    fbci.layers = 1;
    vkt::Framebuffer framebuffer(*m_device, fbci);

    vkt::CommandBuffer cmdbuff(DeviceObj(), m_commandPool);
    cmdbuff.begin();
    VkRenderPassBeginInfo rpbi = vku::InitStructHelper();
    rpbi.renderPass = render_pass.handle();
    rpbi.framebuffer = framebuffer.handle();
    rpbi.renderArea = {{0, 0}, {1, 1}};
    cmdbuff.BeginRenderPass(rpbi);

    vk::CmdBindPipeline(cmdbuff.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
}

TEST_F(PositiveWsi, SwapchainExclusiveModeQueueFamilyPropertiesReferences) {
    TEST_DESCRIPTION("Try using special format props on a swapchain image");

    AddSurfaceExtension();

    RETURN_IF_SKIP(InitFramework())

    RETURN_IF_SKIP(InitState())
    if (!InitSurface()) {
        GTEST_SKIP() << "Cannot create surface";
    }
    InitSwapchainInfo();

    VkBool32 supported;
    vk::GetPhysicalDeviceSurfaceSupportKHR(gpu(), m_device->graphics_queue_node_index_, m_surface, &supported);
    if (!supported) {
        GTEST_SKIP() << "Graphics queue does not support present";
    }

    auto surface = m_surface;
    VkImageUsageFlags imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    VkSurfaceTransformFlagBitsKHR preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;

    VkSwapchainCreateInfoKHR swapchain_create_info = vku::InitStructHelper();
    swapchain_create_info.surface = surface;
    swapchain_create_info.minImageCount = m_surface_capabilities.minImageCount;
    swapchain_create_info.imageFormat = m_surface_formats[0].format;
    swapchain_create_info.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_create_info.imageExtent = {m_surface_capabilities.minImageExtent.width, m_surface_capabilities.minImageExtent.height};
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

    vk::CreateSwapchainKHR(device(), &swapchain_create_info, nullptr, &m_swapchain);
}

TEST_F(VkPositiveLayerTest, InitSwapchain) {
    TEST_DESCRIPTION("Make sure InitSwapchain is not producing anying invalid usage");

    AddSurfaceExtension();

    RETURN_IF_SKIP(InitFramework())

    RETURN_IF_SKIP(InitState())

    if (InitSwapchain()) {
        DestroySwapchain();
    }
}

TEST_F(VkPositiveLayerTest, DestroySwapchainWithBoundImages) {
    TEST_DESCRIPTION("Try destroying a swapchain which has multiple images");

    AddSurfaceExtension();
    AddRequiredExtensions(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    RETURN_IF_SKIP(InitState())
    if (!InitSwapchain()) {
        GTEST_SKIP() << "Cannot create surface or swapchain";
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

    VkImageSwapchainCreateInfoKHR image_swapchain_create_info = vku::InitStructHelper();
    image_swapchain_create_info.swapchain = m_swapchain;

    image_create_info.pNext = &image_swapchain_create_info;
    std::vector<vkt::Image> images(m_surface_capabilities.minImageCount);

    int i = 0;
    for (auto &image : images) {
        image.init_no_mem(*m_device, image_create_info);
        VkBindImageMemorySwapchainInfoKHR bind_swapchain_info = vku::InitStructHelper();
        bind_swapchain_info.swapchain = m_swapchain;
        bind_swapchain_info.imageIndex = i++;

        VkBindImageMemoryInfo bind_info = vku::InitStructHelper(&bind_swapchain_info);
        bind_info.image = image.handle();
        bind_info.memory = VK_NULL_HANDLE;
        bind_info.memoryOffset = 0;

        vk::BindImageMemory2KHR(m_device->device(), 1, &bind_info);
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
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceProtectedMemoryFeatures protected_memory_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(protected_memory_features);

    if (protected_memory_features.protectedMemory == VK_FALSE) {
        GTEST_SKIP() << "protectedMemory feature not supported, skipped.";
    };

    // Turns m_commandBuffer into a unprotected command buffer
    RETURN_IF_SKIP(InitState(nullptr, &protected_memory_features));

    if (!InitSurface()) {
        GTEST_SKIP() << "Cannot create surface, skipping test";
    }
    InitSwapchainInfo();

    // Create protected swapchain
    VkBool32 supported;
    vk::GetPhysicalDeviceSurfaceSupportKHR(gpu(), m_device->graphics_queue_node_index_, m_surface, &supported);
    if (!supported) {
        GTEST_SKIP() << "Graphics queue does not support present, skipping test";
    }

    auto surface = m_surface;
    VkImageUsageFlags imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    VkSurfaceTransformFlagBitsKHR preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;

    VkSwapchainCreateInfoKHR swapchain_create_info = vku::InitStructHelper();
    swapchain_create_info.flags = VK_SWAPCHAIN_CREATE_PROTECTED_BIT_KHR;
    swapchain_create_info.surface = surface;
    swapchain_create_info.minImageCount = m_surface_capabilities.minImageCount;
    swapchain_create_info.imageFormat = m_surface_formats[0].format;
    swapchain_create_info.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_create_info.imageExtent = {m_surface_capabilities.minImageExtent.width, m_surface_capabilities.minImageExtent.height};
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
    ASSERT_EQ(VK_SUCCESS, vk::CreateSwapchainKHR(device(), &swapchain_create_info, nullptr, &m_swapchain));

    // Get VkImage from swapchain which should be protected
    const auto swapchain_images = GetSwapchainImages(m_swapchain);
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
    m_renderPass_info = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 1, attachments, 1, &subpass, 1, &dependency};
    ASSERT_EQ(VK_SUCCESS, vk::CreateRenderPass(device(), &m_renderPass_info, nullptr, &m_renderPass));
    m_framebuffer_info = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                          nullptr,
                          0,
                          m_renderPass,
                          1,
                          &image_view.handle(),
                          swapchain_create_info.imageExtent.width,
                          swapchain_create_info.imageExtent.height,
                          1};
    ASSERT_EQ(VK_SUCCESS, vk::CreateFramebuffer(device(), &m_framebuffer_info, nullptr, &m_framebuffer));

    // basic pipeline to allow for a valid vkCmdDraw()
    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);
    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    // Create a protected command buffer/pool to use
    vkt::CommandPool protectedCommandPool(*m_device, m_device->graphics_queue_node_index_, VK_COMMAND_POOL_CREATE_PROTECTED_BIT);
    vkt::CommandBuffer protectedCommandBuffer(m_device, &protectedCommandPool);

    protectedCommandBuffer.begin();
    VkRect2D render_area = {{0, 0}, swapchain_create_info.imageExtent};
    VkRenderPassBeginInfo render_pass_begin =
        vku::InitStruct<VkRenderPassBeginInfo>(nullptr, m_renderPass, m_framebuffer, render_area, 0u, nullptr);
    vk::CmdBeginRenderPass(protectedCommandBuffer.handle(), &render_pass_begin, VK_SUBPASS_CONTENTS_INLINE);
    vk::CmdBindPipeline(protectedCommandBuffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    // This should be valid since the framebuffer color attachment is a protected swapchain image
    vk::CmdDraw(protectedCommandBuffer.handle(), 3, 1, 0, 0);
    vk::CmdEndRenderPass(protectedCommandBuffer.handle());
    protectedCommandBuffer.end();
}

TEST_F(PositiveWsi, CreateSwapchainWithPresentModeInfo) {
    TEST_DESCRIPTION("Try destroying a swapchain which has multiple images");

    AddSurfaceExtension();
    AddRequiredExtensions(VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    RETURN_IF_SKIP(InitState())
    if (!InitSurface()) {
        GTEST_SKIP() << "Cannot create surface or swapchain";
    }
    InitSwapchainInfo();

    // Implementations must support.
    // Also most likely to have lower minImageCount than reported for other present modes
    // (although this is implementation dependant)
    const auto present_mode = VK_PRESENT_MODE_FIFO_KHR;

    VkSurfacePresentModeEXT surface_present_mode = vku::InitStructHelper();
    surface_present_mode.presentMode = present_mode;
    VkPhysicalDeviceSurfaceInfo2KHR surface_info = vku::InitStructHelper(&surface_present_mode);
    surface_info.surface = m_surface;

    VkSurfaceCapabilities2KHR surface_caps = vku::InitStructHelper();
    vk::GetPhysicalDeviceSurfaceCapabilities2KHR(m_device->phy(), &surface_info, &surface_caps);

    VkSwapchainPresentModesCreateInfoEXT swapchain_present_mode_create_info = vku::InitStructHelper();
    swapchain_present_mode_create_info.presentModeCount = 1;
    swapchain_present_mode_create_info.pPresentModes = &present_mode;
    VkSwapchainCreateInfoKHR swapchain_create_info = vku::InitStructHelper(&swapchain_present_mode_create_info);
    swapchain_create_info.surface = m_surface;
    swapchain_create_info.minImageCount = surface_caps.surfaceCapabilities.minImageCount;
    swapchain_create_info.imageFormat = m_surface_formats[0].format;
    swapchain_create_info.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_create_info.imageExtent = {surface_caps.surfaceCapabilities.minImageExtent.width,
                                         surface_caps.surfaceCapabilities.minImageExtent.height};
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;  // implementations must support
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_create_info.compositeAlpha = m_surface_composite_alpha;
    swapchain_create_info.presentMode = present_mode;
    swapchain_create_info.clipped = VK_FALSE;
    swapchain_create_info.oldSwapchain = 0;

    vk::CreateSwapchainKHR(device(), &swapchain_create_info, nullptr, &m_swapchain);
}
