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
#include "generated/vk_extension_helper.h"

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
#include "wayland-client.h"
#endif

class PositiveWsi : public VkPositiveLayerTest {};

TEST_F(PositiveWsi, CreateWaylandSurface) {
    TEST_DESCRIPTION("Test creating wayland surface");

#ifndef VK_USE_PLATFORM_WAYLAND_KHR
    GTEST_SKIP() << "test not supported on platform";
#else
    AddSurfaceExtension();

    ASSERT_NO_FATAL_FAILURE(Init());

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }

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

    auto surface_create_info = LvlInitStruct<VkWaylandSurfaceCreateInfoKHR>();
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

    ASSERT_NO_FATAL_FAILURE(Init());

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }

    if (!IsExtensionsEnabled(VK_KHR_XCB_SURFACE_EXTENSION_NAME)) {
        GTEST_SKIP() << VK_KHR_XCB_SURFACE_EXTENSION_NAME << " not supported.";
    }

    xcb_connection_t *xcb_connection = xcb_connect(nullptr, nullptr);
    ASSERT_TRUE(xcb_connection);

    // NOTE: This is technically an invalid window! (There is no width/height)
    // But there is no robust way to check for a valid window without crashing the app.
    xcb_window_t xcb_window = xcb_generate_id(xcb_connection);
    ASSERT_TRUE(xcb_window != 0);

    auto surface_create_info = LvlInitStruct<VkXcbSurfaceCreateInfoKHR>();
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

    ASSERT_NO_FATAL_FAILURE(Init());

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }

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
    auto surface_create_info = LvlInitStruct<VkXlibSurfaceCreateInfoKHR>();
    surface_create_info.dpy = x11_display;
    surface_create_info.window = x11_window;
    vk::CreateXlibSurfaceKHR(instance(), &surface_create_info, nullptr, &vulkan_surface);
    vk::DestroySurfaceKHR(instance(), vulkan_surface, nullptr);

    XDestroyWindow(x11_display, x11_window);
    XCloseDisplay(x11_display);
#endif
}

#if defined(VVL_TESTS_ENABLE_EXCLUSIVE_FULLSCREEN) && defined(VK_USE_PLATFORM_WIN32_KHR)
TEST_F(PositiveWsi, GetPhysicalDeviceSurfaceCapabilities2KHRWithFullscreenEXT) {
    TEST_DESCRIPTION("Test vkAcquireFullScreenExclusiveModeEXT.");

    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddSurfaceExtension();
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_FULL_SCREEN_EXCLUSIVE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    if (!InitSwapchain()) {
        GTEST_SKIP() << "Cannot create surface or swapchain";
    }

    const POINT pt_zero = {0, 0};

    auto fullscreen_exclusive_win32_info = LvlInitStruct<VkSurfaceFullScreenExclusiveWin32InfoEXT>();
    fullscreen_exclusive_win32_info.hmonitor = MonitorFromPoint(pt_zero, MONITOR_DEFAULTTOPRIMARY);
    auto fullscreen_exclusive_info = LvlInitStruct<VkSurfaceFullScreenExclusiveInfoEXT>(&fullscreen_exclusive_win32_info);
    fullscreen_exclusive_info.fullScreenExclusive = VK_FULL_SCREEN_EXCLUSIVE_APPLICATION_CONTROLLED_EXT;

    auto surface_info = LvlInitStruct<VkPhysicalDeviceSurfaceInfo2KHR>(&fullscreen_exclusive_info);
    surface_info.surface = m_surface;

    auto surface_caps = LvlInitStruct<VkSurfaceCapabilities2KHR>();
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
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }

    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    if (IsDriver(VK_DRIVER_ID_MESA_RADV)) {
        // Seeing the same crash as the Android comment above
        GTEST_SKIP() << "This test should not be run on the RADV driver";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    if (!InitSwapchain(VK_IMAGE_USAGE_TRANSFER_DST_BIT)) {
        GTEST_SKIP() << "Cannot create surface or swapchain";
    }

    auto image_create_info = LvlInitStruct<VkImageCreateInfo>();
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

    auto image_swapchain_create_info = LvlInitStruct<VkImageSwapchainCreateInfoKHR>();
    image_swapchain_create_info.swapchain = m_swapchain;
    image_create_info.pNext = &image_swapchain_create_info;

    vk_testing::Image image_from_swapchain(*m_device, image_create_info, vk_testing::no_mem);

    auto bind_swapchain_info = LvlInitStruct<VkBindImageMemorySwapchainInfoKHR>();
    bind_swapchain_info.swapchain = m_swapchain;
    bind_swapchain_info.imageIndex = 0;

    auto bind_info = LvlInitStruct<VkBindImageMemoryInfo>(&bind_swapchain_info);
    bind_info.image = image_from_swapchain.handle();
    bind_info.memory = VK_NULL_HANDLE;
    bind_info.memoryOffset = 0;

    vk::BindImageMemory2(m_device->device(), 1, &bind_info);

    VkImageCopy copy_region = {};
    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.srcSubresource.mipLevel = 0;
    copy_region.dstSubresource.mipLevel = 0;
    copy_region.srcSubresource.baseArrayLayer = 0;
    copy_region.dstSubresource.baseArrayLayer = 0;
    copy_region.srcSubresource.layerCount = 1;
    copy_region.dstSubresource.layerCount = 1;
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

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }

    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

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
    VkDeviceGroupDeviceCreateInfo create_device_pnext = LvlInitStruct<VkDeviceGroupDeviceCreateInfo>();
    create_device_pnext.physicalDeviceCount = physical_device_group[0].physicalDeviceCount;
    create_device_pnext.pPhysicalDevices = physical_device_group[0].physicalDevices;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &create_device_pnext));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    if (!InitSwapchain(VK_IMAGE_USAGE_TRANSFER_DST_BIT)) {
        GTEST_SKIP() << "Cannot create surface or swapchain";
    }

    constexpr uint32_t test_extent_value = 10;
    if (m_surface_capabilities.minImageExtent.width < test_extent_value ||
        m_surface_capabilities.minImageExtent.height < test_extent_value) {
        GTEST_SKIP() << "minImageExtent is not large enough";
    }

    auto image_create_info = LvlInitStruct<VkImageCreateInfo>();
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

    auto image_swapchain_create_info = LvlInitStruct<VkImageSwapchainCreateInfoKHR>();
    image_swapchain_create_info.swapchain = m_swapchain;
    image_create_info.pNext = &image_swapchain_create_info;

    vk_testing::Image peer_image(*m_device, image_create_info, vk_testing::no_mem);

    auto bind_devicegroup_info = LvlInitStruct<VkBindImageMemoryDeviceGroupInfo>();
    std::array<uint32_t, 1> deviceIndices = {{0}};
    bind_devicegroup_info.deviceIndexCount = static_cast<uint32_t>(deviceIndices.size());
    bind_devicegroup_info.pDeviceIndices = deviceIndices.data();
    bind_devicegroup_info.splitInstanceBindRegionCount = 0;
    bind_devicegroup_info.pSplitInstanceBindRegions = nullptr;

    auto bind_swapchain_info = LvlInitStruct<VkBindImageMemorySwapchainInfoKHR>(&bind_devicegroup_info);
    bind_swapchain_info.swapchain = m_swapchain;
    bind_swapchain_info.imageIndex = 0;

    auto bind_info = LvlInitStruct<VkBindImageMemoryInfo>(&bind_swapchain_info);
    bind_info.image = peer_image.handle();
    bind_info.memory = VK_NULL_HANDLE;
    bind_info.memoryOffset = 0;

    vk::BindImageMemory2(m_device->device(), 1, &bind_info);

    uint32_t swapchain_images_count = 0;
    vk::GetSwapchainImagesKHR(device(), m_swapchain, &swapchain_images_count, nullptr);
    std::vector<VkImage> swapchain_images;
    swapchain_images.resize(swapchain_images_count);
    vk::GetSwapchainImagesKHR(device(), m_swapchain, &swapchain_images_count, swapchain_images.data());

    vk_testing::Fence fence;
    fence.init(*m_device, VkFenceObj::create_info());
    VkFence fence_handle = fence.handle();

    uint32_t image_index;
    vk::AcquireNextImageKHR(m_device->handle(), m_swapchain, kWaitTimeout, VK_NULL_HANDLE, fence_handle, &image_index);
    vk::WaitForFences(m_device->device(), 1, &fence_handle, VK_TRUE, kWaitTimeout);

    m_commandBuffer->begin();

    auto img_barrier = LvlInitStruct<VkImageMemoryBarrier>();
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
    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.srcSubresource.mipLevel = 0;
    copy_region.dstSubresource.mipLevel = 0;
    copy_region.srcSubresource.baseArrayLayer = 0;
    copy_region.dstSubresource.baseArrayLayer = 0;
    copy_region.srcSubresource.layerCount = 1;
    copy_region.dstSubresource.layerCount = 1;
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstOffset = {0, 0, 0};
    copy_region.extent = {test_extent_value, test_extent_value, 1};
    vk::CmdCopyImage(m_commandBuffer->handle(), src_Image.handle(), VK_IMAGE_LAYOUT_GENERAL, peer_image.handle(),
                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);

    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer();
}

TEST_F(PositiveWsi, SwapchainImageLayout) {
    AddSurfaceExtension();
    AddRequiredExtensions(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());
    if (!InitSwapchain()) {
        GTEST_SKIP() << "Cannot create surface or swapchain, skipping CmdCopySwapchainImage test";
    }
    uint32_t image_index, image_count;
    vk::GetSwapchainImagesKHR(m_device->handle(), m_swapchain, &image_count, nullptr);
    std::vector<VkImage> swapchainImages(image_count, VK_NULL_HANDLE);
    vk::GetSwapchainImagesKHR(m_device->handle(), m_swapchain, &image_count, swapchainImages.data());
    auto fenceci = LvlInitStruct<VkFenceCreateInfo>();
    vk_testing::Fence fence(*m_device, fenceci);
    ASSERT_VK_SUCCESS(
        vk::AcquireNextImageKHR(m_device->handle(), m_swapchain, kWaitTimeout, VK_NULL_HANDLE, fence.handle(), &image_index));
    VkAttachmentDescription attach[] = {
        {0, m_surface_formats[0].format, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
    };
    VkAttachmentReference att_ref = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    VkSubpassDescription subpass = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &att_ref, nullptr, nullptr, 0, nullptr};
    auto rpci = LvlInitStruct<VkRenderPassCreateInfo>();
    rpci.attachmentCount = 1;
    rpci.pAttachments = attach;
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    vk_testing::RenderPass rp1(*m_device, rpci);
    ASSERT_TRUE(rp1.initialized());

    attach[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    vk_testing::RenderPass rp2(*m_device, rpci);
    ASSERT_TRUE(rp2.initialized());

    auto ivci = LvlInitStruct<VkImageViewCreateInfo>();
    ivci.image = swapchainImages[image_index];
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = m_surface_formats[0].format;
    ivci.components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                       VK_COMPONENT_SWIZZLE_IDENTITY};
    ivci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    vk_testing::ImageView view(*m_device, ivci);
    ASSERT_TRUE(view.initialized());
    auto fci = LvlInitStruct<VkFramebufferCreateInfo>();
    fci.renderPass = rp1.handle();
    fci.attachmentCount = 1;
    fci.pAttachments = &view.handle();
    fci.width = 1;
    fci.height = 1;
    fci.layers = 1;
    vk_testing::Framebuffer fb1(*m_device, fci);
    ASSERT_TRUE(fb1.initialized());
    fci.renderPass = rp2.handle();
    vk_testing::Framebuffer fb2(*m_device, fci);
    ASSERT_TRUE(fb2.initialized());
    VkRenderPassBeginInfo rpbi =
        LvlInitStruct<VkRenderPassBeginInfo>(nullptr, rp1.handle(), fb1.handle(), VkRect2D{{0, 0}, {1u, 1u}}, 0u, nullptr);
    m_commandBuffer->begin();
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    vk::CmdEndRenderPass(m_commandBuffer->handle());
    rpbi.framebuffer = fb2.handle();
    rpbi.renderPass = rp2.handle();
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    vk::CmdEndRenderPass(m_commandBuffer->handle());

    VkImageMemoryBarrier img_barrier = LvlInitStruct<VkImageMemoryBarrier>();
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
    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.waitSemaphoreCount = 0;
    submit_info.pWaitSemaphores = NULL;
    submit_info.pWaitDstStageMask = NULL;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    submit_info.signalSemaphoreCount = 0;
    submit_info.pSignalSemaphores = NULL;
    vk::WaitForFences(m_device->device(), 1, &fence.handle(), VK_TRUE, kWaitTimeout);
    vk::ResetFences(m_device->device(), 1, &fence.handle());
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, fence.handle());
    vk::WaitForFences(m_device->device(), 1, &fence.handle(), VK_TRUE, kWaitTimeout);
}

TEST_F(PositiveWsi, SwapchainPresentShared) {
    TEST_DESCRIPTION("Acquire shared presentable image and Present multiple times without failure.");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SHARED_PRESENTABLE_IMAGE_EXTENSION_NAME);
    AddSurfaceExtension();

    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
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

    auto shared_present_capabilities = LvlInitStruct<VkSharedPresentSurfaceCapabilitiesKHR>();
    auto capabilities = LvlInitStruct<VkSurfaceCapabilities2KHR>(&shared_present_capabilities);
    auto surface_info = LvlInitStruct<VkPhysicalDeviceSurfaceInfo2KHR>();
    surface_info.surface = m_surface;
    vk::GetPhysicalDeviceSurfaceCapabilities2KHR(gpu(), &surface_info, &capabilities);

    // This was recently added to CTS, but some drivers might not correctly advertise the flag
    if ((shared_present_capabilities.sharedPresentSupportedUsageFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) == 0) {
        GTEST_SKIP() << "Driver was suppose to support VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT";
    }

    auto swapchain_create_info = LvlInitStruct<VkSwapchainCreateInfoKHR>();
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

    uint32_t image_count;
    vk::GetSwapchainImagesKHR(device(), m_swapchain, &image_count, nullptr);
    std::vector<VkImage> images(image_count);
    vk::GetSwapchainImagesKHR(device(), m_swapchain, &image_count, images.data());

    uint32_t image_index;
    VkFenceObj fence;
    fence.init(*m_device, VkFenceObj::create_info());
    vk::AcquireNextImageKHR(device(), m_swapchain, kWaitTimeout, VK_NULL_HANDLE, fence.handle(), &image_index);
    vk::WaitForFences(device(), 1, &fence.handle(), true, kWaitTimeout);

    SetImageLayout(m_device, VK_IMAGE_ASPECT_COLOR_BIT, images[image_index], VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR);

    auto present = LvlInitStruct<VkPresentInfoKHR>();
    present.waitSemaphoreCount = 0;
    present.swapchainCount = 1;
    present.pSwapchains = &m_swapchain;
    present.pImageIndices = &image_index;
    vk::QueuePresentKHR(m_device->m_queue, &present);

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
        vk::QueuePresentKHR(m_device->m_queue, &present);
    }
}
