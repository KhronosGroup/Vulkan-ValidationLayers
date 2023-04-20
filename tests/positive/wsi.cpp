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

class VkPositiveWsiTest : public VkPositiveLayerTest {};

TEST_F(VkPositiveWsiTest, CreateWaylandSurface) {
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

TEST_F(VkPositiveWsiTest, CreateXcbSurface) {
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

TEST_F(VkPositiveWsiTest, CreateX11Surface) {
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
TEST_F(VkPositiveWsiTest, GetPhysicalDeviceSurfaceCapabilities2KHRWithFullscreenEXT) {
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
