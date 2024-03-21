/*
 * Copyright (c) 2023-2024 Valve Corporation
 * Copyright (c) 2023-2024 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "utils/cast_utils.h"
#include "generated/enum_flag_bits.h"
#include "../framework/layer_validation_tests.h"
#include "../framework/shader_helper.h"
#include "../framework/pipeline_helper.h"

namespace {
VKAPI_ATTR VkBool32 VKAPI_CALL EmptyDebugReportCallback(VkDebugReportFlagsEXT message_flags, VkDebugReportObjectTypeEXT, uint64_t,
                                                        size_t, int32_t, const char *, const char *message, void *user_data) {
    return VK_FALSE;
}
struct Instance {
    VkInstance handle = VK_NULL_HANDLE;
    ~Instance() {
        if (handle != VK_NULL_HANDLE) {
            vk::DestroyInstance(handle, nullptr);
        }
    }
    operator VkInstance() const { return handle; }
};

struct Surface {
    VkInstance instance = VK_NULL_HANDLE;
    VkSurfaceKHR handle = VK_NULL_HANDLE;
    Surface(VkInstance instance) : instance(instance) {}
    ~Surface() {
        if (handle != VK_NULL_HANDLE) {
            vk::DestroySurfaceKHR(instance, handle, nullptr);
        }
    }
    operator VkSurfaceKHR() const { return handle; }
};
}  // namespace

TEST_F(NegativeParent, FillBuffer) {
    TEST_DESCRIPTION("Test VUID-*-commonparent checks not sharing the same Device");

    RETURN_IF_SKIP(Init());
    auto features = m_device->phy().features();
    m_second_device = new vkt::Device(gpu_, m_device_extension_names, &features, nullptr);

    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.size = 4096;
    buffer_ci.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buffer_ci.queueFamilyIndexCount = 0;
    vkt::Buffer buffer(*m_second_device, buffer_ci);

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdFillBuffer-commonparent");
    vk::CmdFillBuffer(m_commandBuffer->handle(), buffer, 0, VK_WHOLE_SIZE, 0);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(NegativeParent, BindBuffer) {
    TEST_DESCRIPTION("Test VUID-*-commonparent checks not sharing the same Device");

    AddRequiredExtensions(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    auto features = m_device->phy().features();
    m_second_device = new vkt::Device(gpu_, m_device_extension_names, &features, nullptr);

    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.size = 4096;
    buffer_ci.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vkt::Buffer buffer(*m_device, buffer_ci, vkt::no_mem);

    VkMemoryRequirements mem_reqs;
    m_errorMonitor->SetDesiredError("VUID-vkGetBufferMemoryRequirements-buffer-parent");
    vk::GetBufferMemoryRequirements(m_second_device->handle(), buffer.handle(), &mem_reqs);
    m_errorMonitor->VerifyFound();
    vk::GetBufferMemoryRequirements(device(), buffer.handle(), &mem_reqs);

    VkMemoryAllocateInfo mem_alloc = vku::InitStructHelper();
    mem_alloc.allocationSize = mem_reqs.size;
    m_device->phy().set_memory_type(mem_reqs.memoryTypeBits, &mem_alloc, 0);
    vkt::DeviceMemory memory(*m_second_device, mem_alloc);

    VkBindBufferMemoryInfo bind_buffer_info = vku::InitStructHelper();
    bind_buffer_info.buffer = buffer.handle();
    bind_buffer_info.memory = memory.handle();
    bind_buffer_info.memoryOffset = 0;

    m_errorMonitor->SetDesiredError("VUID-VkBindBufferMemoryInfo-commonparent");
    vk::BindBufferMemory2KHR(device(), 1, &bind_buffer_info);
    m_errorMonitor->VerifyFound();
}

// Some of these commonparent VUs are for "non-ignored parameters", various spot related in spec
// Spec issue - https://gitlab.khronos.org/vulkan/vulkan/-/merge_requests/6227
TEST_F(NegativeParent, DISABLED_BindImage) {
    TEST_DESCRIPTION("Test VUID-*-commonparent checks not sharing the same Device");

    AddRequiredExtensions(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    auto features = m_device->phy().features();
    m_second_device = new vkt::Device(gpu_, m_device_extension_names, &features, nullptr);

    auto image_ci = vkt::Image::ImageCreateInfo2D(128, 128, 1, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::Image image(*m_device, image_ci, vkt::set_layout);

    VkMemoryRequirements mem_reqs;
    m_errorMonitor->SetDesiredError("VUID-vkGetImageMemoryRequirements-image-parent");
    vk::GetImageMemoryRequirements(m_second_device->handle(), image.handle(), &mem_reqs);
    m_errorMonitor->VerifyFound();
    vk::GetImageMemoryRequirements(device(), image.handle(), &mem_reqs);

    VkMemoryAllocateInfo mem_alloc = vku::InitStructHelper();
    mem_alloc.allocationSize = mem_reqs.size;
    m_device->phy().set_memory_type(mem_reqs.memoryTypeBits, &mem_alloc, 0);
    vkt::DeviceMemory memory(*m_second_device, mem_alloc);

    VkBindImageMemoryInfo bind_image_info = vku::InitStructHelper();
    bind_image_info.image = image.handle();
    bind_image_info.memory = memory.handle();
    bind_image_info.memoryOffset = 0;

    m_errorMonitor->SetDesiredError("VUID-VkBindImageMemoryInfo-commonparent");
    vk::BindImageMemory2KHR(device(), 1, &bind_image_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeParent, ImageView) {
    TEST_DESCRIPTION("Test VUID-*-commonparent checks not sharing the same Device");

    RETURN_IF_SKIP(Init());
    auto features = m_device->phy().features();
    m_second_device = new vkt::Device(gpu_, m_device_extension_names, &features, nullptr);

    auto image_ci = vkt::Image::ImageCreateInfo2D(128, 128, 1, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::Image image(*m_device, image_ci, vkt::set_layout);

    VkImageView image_view;
    VkImageViewCreateInfo ivci = vku::InitStructHelper();
    ivci.image = image.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = VK_FORMAT_B8G8R8A8_UNORM;
    ivci.subresourceRange.layerCount = 1;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.levelCount = 1;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    m_errorMonitor->SetDesiredError("VUID-vkCreateImageView-image-09179");
    vk::CreateImageView(m_second_device->handle(), &ivci, nullptr, &image_view);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeParent, BindPipeline) {
    TEST_DESCRIPTION("Test binding pipeline from another device");

    RETURN_IF_SKIP(Init());
    auto features = m_device->phy().features();
    m_second_device = new vkt::Device(gpu_, m_device_extension_names, &features, nullptr);

    VkPipelineLayoutCreateInfo pipeline_layout_ci = vku::InitStructHelper();
    pipeline_layout_ci.setLayoutCount = 0;
    vkt::PipelineLayout pipeline_layout(*m_second_device, pipeline_layout_ci);

    VkShaderObj cs(this, kMinimalShaderGlsl, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL_TRY);
    cs.InitFromGLSLTry(m_second_device);

    VkComputePipelineCreateInfo pipeline_ci = vku::InitStructHelper();
    pipeline_ci.layout = pipeline_layout.handle();
    pipeline_ci.stage = cs.GetStageCreateInfo();
    vkt::Pipeline pipeline(*m_second_device, pipeline_ci);

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdBindPipeline-commonparent");
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.handle());
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(NegativeParent, RenderPassFramebuffer) {
    TEST_DESCRIPTION("Test RenderPass and Framebuffer");

    RETURN_IF_SKIP(Init());
    InitRenderTarget(); // Renderpass created on first device
    auto features = m_device->phy().features();
    m_second_device = new vkt::Device(gpu_, m_device_extension_names, &features, nullptr);

    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-commonparent");
    vkt::Framebuffer fb(*m_second_device, m_renderPass, 0, nullptr, m_width, m_height);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeParent, RenderPassImagelessFramebuffer) {
    TEST_DESCRIPTION("Test RenderPass and Imageless Framebuffer");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitFramework());
    VkPhysicalDeviceImagelessFramebufferFeatures imageless_framebuffer = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(imageless_framebuffer);
    RETURN_IF_SKIP(InitState(nullptr, &imageless_framebuffer));

    InitRenderTarget();
    auto features = m_device->phy().features();
    m_second_device = new vkt::Device(gpu_, m_device_extension_names, &features, nullptr);

    VkFormat format = VK_FORMAT_B8G8R8A8_UNORM;
    VkFramebufferAttachmentImageInfo framebuffer_attachment_image_info = vku::InitStructHelper();
    framebuffer_attachment_image_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    framebuffer_attachment_image_info.width = 256;
    framebuffer_attachment_image_info.height = 256;
    framebuffer_attachment_image_info.layerCount = 1;
    framebuffer_attachment_image_info.viewFormatCount = 1;
    framebuffer_attachment_image_info.pViewFormats = &format;

    VkFramebufferAttachmentsCreateInfo framebuffer_attachments = vku::InitStructHelper();
    framebuffer_attachments.attachmentImageInfoCount = 1;
    framebuffer_attachments.pAttachmentImageInfos = &framebuffer_attachment_image_info;

    VkFramebufferCreateInfo fb_info = vku::InitStructHelper(&framebuffer_attachments);
    fb_info.renderPass = m_renderPass;
    fb_info.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT;
    fb_info.attachmentCount = 1;
    fb_info.width = m_width;
    fb_info.height = m_height;
    fb_info.layers = 1;
    vkt::Framebuffer fb(*m_device, fb_info);

    auto image_ci = vkt::Image::ImageCreateInfo2D(256, 256, 1, 1, format, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    vkt::Image image(*m_second_device, image_ci, vkt::set_layout);
    vkt::ImageView image_view = image.CreateView();

    VkRenderPassAttachmentBeginInfo render_pass_attachment_bi = vku::InitStructHelper();
    render_pass_attachment_bi.attachmentCount = 1;
    render_pass_attachment_bi.pAttachments = &image_view.handle();

    m_renderPassBeginInfo.pNext = &render_pass_attachment_bi;
    m_renderPassBeginInfo.framebuffer = fb.handle();

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredError("VUID-VkRenderPassBeginInfo-framebuffer-02780");
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(NegativeParent, RenderPassCommandBuffer) {
    TEST_DESCRIPTION("Test RenderPass and Framebuffer");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();  // Renderpass created on first device
    auto features = m_device->phy().features();
    m_second_device = new vkt::Device(gpu_, m_device_extension_names, &features, nullptr);

    vkt::CommandPool command_pool(*m_second_device, m_device->graphics_queue_node_index_, 0);
    vkt::CommandBuffer command_buffer(*m_second_device, &command_pool);

    command_buffer.begin();
    // one for each the framebuffer and renderpass being different from the CommandBuffer
    m_errorMonitor->SetDesiredError("VUID-VkRenderPassBeginInfo-commonparent");
    m_errorMonitor->SetDesiredError("VUID-VkRenderPassBeginInfo-commonparent");
    auto subpass_begin_info = vku::InitStruct<VkSubpassBeginInfoKHR>(nullptr, VK_SUBPASS_CONTENTS_INLINE);
    vk::CmdBeginRenderPass2(command_buffer.handle(), &m_renderPassBeginInfo, &subpass_begin_info);
    m_errorMonitor->VerifyFound();
    command_buffer.end();
}

TEST_F(NegativeParent, Instance_PhysicalDeviceAndSurface) {
    TEST_DESCRIPTION("Surface from a different instance in vkGetPhysicalDeviceSurfaceSupportKHR");
    AddSurfaceExtension();
    RETURN_IF_SKIP(Init());
    const auto instance_create_info = GetInstanceCreateInfo();
    Instance instance2;
    ASSERT_EQ(VK_SUCCESS, vk::CreateInstance(&instance_create_info, nullptr, &instance2.handle));

    SurfaceContext surface_context;
    Surface instance2_surface(instance2);
    ASSERT_EQ(VK_SUCCESS, CreateSurface(surface_context, instance2_surface.handle, instance2));

    VkBool32 supported = VK_FALSE;
    m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceSurfaceSupportKHR-commonparent");
    vk::GetPhysicalDeviceSurfaceSupportKHR(gpu(), m_device->graphics_queue_node_index_, instance2_surface, &supported);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeParent, Instance_DeviceAndSurface) {
    TEST_DESCRIPTION("Surface from a different instance in vkGetDeviceGroupSurfacePresentModesKHR");
    AddSurfaceExtension();
    RETURN_IF_SKIP(Init());
    const auto instance_create_info = GetInstanceCreateInfo();
    Instance instance2;
    ASSERT_EQ(VK_SUCCESS, vk::CreateInstance(&instance_create_info, nullptr, &instance2.handle));

    SurfaceContext surface_context;
    Surface instance2_surface(instance2);
    ASSERT_EQ(VK_SUCCESS, CreateSurface(surface_context, instance2_surface.handle, instance2));

    VkDeviceGroupPresentModeFlagsKHR flags = 0;
    m_errorMonitor->SetDesiredError("VUID-vkGetDeviceGroupSurfacePresentModesKHR-commonparent");
    vk::GetDeviceGroupSurfacePresentModesKHR(m_device->handle(), instance2_surface, &flags);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeParent, Instance_Surface) {
    TEST_DESCRIPTION("Surface from a different instance in vkCreateSwapchainKHR");
    AddSurfaceExtension();
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSurface());
    InitSwapchainInfo();

    const auto instance_create_info = GetInstanceCreateInfo();
    Instance instance2;
    ASSERT_EQ(VK_SUCCESS, vk::CreateInstance(&instance_create_info, nullptr, &instance2.handle));

    SurfaceContext surface_context;
    Surface instance2_surface(instance2);
    if (CreateSurface(surface_context, instance2_surface.handle, instance2) != VK_SUCCESS) {
        GTEST_SKIP() << "Cannot create 2nd surface";
    }

    auto swapchain_ci = vku::InitStruct<VkSwapchainCreateInfoKHR>();
    swapchain_ci.surface = instance2_surface;
    swapchain_ci.minImageCount = m_surface_capabilities.minImageCount;
    swapchain_ci.imageFormat = m_surface_formats[0].format;
    swapchain_ci.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_ci.imageExtent = {m_surface_capabilities.minImageExtent.width, m_surface_capabilities.minImageExtent.height};
    swapchain_ci.imageArrayLayers = 1;
    swapchain_ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_ci.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_ci.compositeAlpha = m_surface_composite_alpha;
    swapchain_ci.presentMode = m_surface_non_shared_present_mode;
    swapchain_ci.clipped = VK_FALSE;
    swapchain_ci.oldSwapchain = VK_NULL_HANDLE;

    // surface from a different instance
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    m_errorMonitor->SetDesiredError("VUID-VkSwapchainCreateInfoKHR-commonparent");
    vk::CreateSwapchainKHR(device(), &swapchain_ci, nullptr, &swapchain);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeParent, Device_OldSwapchain) {
    TEST_DESCRIPTION("oldSwapchain from a different device in vkCreateSwapchainKHR");
    AddSurfaceExtension();
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSurface());
    InitSwapchainInfo();

    const auto instance_create_info = GetInstanceCreateInfo();
    Instance instance2;
    ASSERT_EQ(VK_SUCCESS, vk::CreateInstance(&instance_create_info, nullptr, &instance2.handle));

    SurfaceContext surface_context;
    Surface instance2_surface(instance2);
    if (CreateSurface(surface_context, instance2_surface.handle, instance2) != VK_SUCCESS) {
        GTEST_SKIP() << "Cannot create 2nd surface";
    }

    VkPhysicalDevice instance2_physical_device = VK_NULL_HANDLE;
    {
        uint32_t gpu_count = 0;
        vk::EnumeratePhysicalDevices(instance2, &gpu_count, nullptr);
        assert(gpu_count > 0);
        std::vector<VkPhysicalDevice> physical_devices(gpu_count);
        vk::EnumeratePhysicalDevices(instance2, &gpu_count, physical_devices.data());
        instance2_physical_device = physical_devices[0];
    }
    vkt::Device instance2_device(instance2_physical_device, m_device_extension_names);

    auto swapchain_ci = vku::InitStruct<VkSwapchainCreateInfoKHR>();
    swapchain_ci.surface = instance2_surface;
    swapchain_ci.minImageCount = m_surface_capabilities.minImageCount;
    swapchain_ci.imageFormat = m_surface_formats[0].format;
    swapchain_ci.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_ci.imageExtent = {m_surface_capabilities.minImageExtent.width, m_surface_capabilities.minImageExtent.height};
    swapchain_ci.imageArrayLayers = 1;
    swapchain_ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_ci.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_ci.compositeAlpha = m_surface_composite_alpha;
    swapchain_ci.presentMode = m_surface_non_shared_present_mode;
    swapchain_ci.clipped = VK_FALSE;
    swapchain_ci.oldSwapchain = VK_NULL_HANDLE;

    VkSwapchainKHR other_device_swapchain = VK_NULL_HANDLE;
    ASSERT_EQ(VK_SUCCESS, vk::CreateSwapchainKHR(instance2_device.handle(), &swapchain_ci, nullptr, &other_device_swapchain));

    // oldSwapchain from a different device
    swapchain_ci.surface = m_surface;
    swapchain_ci.oldSwapchain = other_device_swapchain;
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    m_errorMonitor->SetDesiredError("VUID-VkSwapchainCreateInfoKHR-commonparent");
    vk::CreateSwapchainKHR(device(), &swapchain_ci, nullptr, &swapchain);
    m_errorMonitor->VerifyFound();
    vk::DestroySwapchainKHR(instance2_device.handle(), other_device_swapchain, nullptr);
}

TEST_F(NegativeParent, Instance_Surface_2) {
    TEST_DESCRIPTION("Surface from a different instance in vkDestroySurfaceKHR");
    AddSurfaceExtension();
    RETURN_IF_SKIP(Init());
    const auto instance_create_info = GetInstanceCreateInfo();
    Instance instance2;
    ASSERT_EQ(VK_SUCCESS, vk::CreateInstance(&instance_create_info, nullptr, &instance2.handle));

    SurfaceContext surface_context;
    Surface instance2_surface(instance2);
    ASSERT_EQ(VK_SUCCESS, CreateSurface(surface_context, instance2_surface.handle, instance2));

    // surface from a different instance
    m_errorMonitor->SetDesiredError("VUID-vkDestroySurfaceKHR-surface-parent");
    vk::DestroySurfaceKHR(instance(), instance2_surface, nullptr);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeParent, Instance_DebugUtilsMessenger) {
    TEST_DESCRIPTION("VkDebugUtilsMessengerEXT from a different instance in vkDestroyDebugUtilsMessengerEXT");
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    const VkInstanceCreateInfo instance_create_info = GetInstanceCreateInfo();
    Instance instance2;
    ASSERT_EQ(VK_SUCCESS, vk::CreateInstance(&instance_create_info, nullptr, &instance2.handle));

    auto empty_callback = [](const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, DebugUtilsLabelCheckData *data) {};
    DebugUtilsLabelCheckData callback_data{};
    callback_data.callback = empty_callback;

    VkDebugUtilsMessengerEXT messenger = VK_NULL_HANDLE;
    {
        auto messenger_ci = vku::InitStruct<VkDebugUtilsMessengerCreateInfoEXT>();
        messenger_ci.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        messenger_ci.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
        messenger_ci.pfnUserCallback = DebugUtilsCallback;
        messenger_ci.pUserData = &callback_data;
        ASSERT_EQ(VK_SUCCESS, vk::CreateDebugUtilsMessengerEXT(instance2, &messenger_ci, nullptr, &messenger));
    }

    // debug utils messenger from a different instance
    m_errorMonitor->SetDesiredError("VUID-vkDestroyDebugUtilsMessengerEXT-messenger-parent");
    vk::DestroyDebugUtilsMessengerEXT(instance(), messenger, nullptr);
    m_errorMonitor->VerifyFound();
    vk::DestroyDebugUtilsMessengerEXT(instance2, messenger, nullptr);
}

TEST_F(NegativeParent, Instance_DebugReportCallback) {
    TEST_DESCRIPTION("VkDebugReportCallbackEXT from a different instance in vkDestroyDebugReportCallbackEXT");
    AddRequiredExtensions(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    const auto instance_create_info = GetInstanceCreateInfo();
    Instance instance2;
    ASSERT_EQ(VK_SUCCESS, vk::CreateInstance(&instance_create_info, nullptr, &instance2.handle));

    VkDebugReportCallbackEXT callback = VK_NULL_HANDLE;
    {
        auto callback_ci = vku::InitStruct<VkDebugReportCallbackCreateInfoEXT>();
        callback_ci.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT;
        callback_ci.pfnCallback = &EmptyDebugReportCallback;
        ASSERT_EQ(VK_SUCCESS, vk::CreateDebugReportCallbackEXT(instance2, &callback_ci, nullptr, &callback));
    }

    // debug report callback from a different instance
    m_errorMonitor->SetDesiredError("VUID-vkDestroyDebugReportCallbackEXT-callback-parent");
    vk::DestroyDebugReportCallbackEXT(instance(), callback, nullptr);
    m_errorMonitor->VerifyFound();
    vk::DestroyDebugReportCallbackEXT(instance2, callback, nullptr);
}

TEST_F(NegativeParent, PhysicalDevice_Display) {
    TEST_DESCRIPTION("VkDisplayKHR from a different physical device in vkGetDisplayModePropertiesKHR");
    AddRequiredExtensions(VK_KHR_DISPLAY_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    const VkInstanceCreateInfo instance_create_info = GetInstanceCreateInfo();
    Instance instance2;
    ASSERT_EQ(VK_SUCCESS, vk::CreateInstance(&instance_create_info, nullptr, &instance2.handle));

    VkPhysicalDevice instance2_gpu = VK_NULL_HANDLE;
    {
        uint32_t gpu_count = 0;
        vk::EnumeratePhysicalDevices(instance2, &gpu_count, nullptr);
        ASSERT_GT(gpu_count, 0);
        std::vector<VkPhysicalDevice> physical_devices(gpu_count);
        vk::EnumeratePhysicalDevices(instance2, &gpu_count, physical_devices.data());
        instance2_gpu = physical_devices[0];
    }
    VkDisplayKHR display = VK_NULL_HANDLE;
    {
        uint32_t display_count = 0;
        ASSERT_EQ(VK_SUCCESS, vk::GetPhysicalDeviceDisplayPropertiesKHR(instance2_gpu, &display_count, nullptr));
        if (display_count == 0) {
            GTEST_SKIP() << "No VkDisplayKHR displays found";
        }
        std::vector<VkDisplayPropertiesKHR> display_props{display_count};
        ASSERT_EQ(VK_SUCCESS, vk::GetPhysicalDeviceDisplayPropertiesKHR(instance2_gpu, &display_count, display_props.data()));
        display = display_props[0].display;
    }
    // display from a different physical device
    uint32_t mode_count = 0;
    m_errorMonitor->SetDesiredError("VUID-vkGetDisplayModePropertiesKHR-display-parent");
    vk::GetDisplayModePropertiesKHR(gpu(), display, &mode_count, nullptr);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeParent, PhysicalDevice_RegisterDisplayEvent) {
    TEST_DESCRIPTION("VkDisplayKHR from a different physical device in vkRegisterDisplayEventEXT");
    AddRequiredExtensions(VK_EXT_DISPLAY_CONTROL_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    const VkInstanceCreateInfo instance_create_info = GetInstanceCreateInfo();
    Instance instance2;
    ASSERT_EQ(VK_SUCCESS, vk::CreateInstance(&instance_create_info, nullptr, &instance2.handle));

    VkPhysicalDevice instance2_gpu = VK_NULL_HANDLE;
    {
        uint32_t gpu_count = 0;
        vk::EnumeratePhysicalDevices(instance2, &gpu_count, nullptr);
        ASSERT_GT(gpu_count, 0);
        std::vector<VkPhysicalDevice> physical_devices(gpu_count);
        vk::EnumeratePhysicalDevices(instance2, &gpu_count, physical_devices.data());
        instance2_gpu = physical_devices[0];
    }
    VkDisplayKHR display = VK_NULL_HANDLE;
    {
        uint32_t display_count = 0;
        ASSERT_EQ(VK_SUCCESS, vk::GetPhysicalDeviceDisplayPropertiesKHR(instance2_gpu, &display_count, nullptr));
        if (display_count == 0) {
            GTEST_SKIP() << "No VkDisplayKHR displays found";
        }
        std::vector<VkDisplayPropertiesKHR> display_props{display_count};
        ASSERT_EQ(VK_SUCCESS, vk::GetPhysicalDeviceDisplayPropertiesKHR(instance2_gpu, &display_count, display_props.data()));
        display = display_props[0].display;
    }

    VkDisplayEventInfoEXT event_info = vku::InitStructHelper();
    event_info.displayEvent = VK_DISPLAY_EVENT_TYPE_FIRST_PIXEL_OUT_EXT;
    VkFence fence;

    m_errorMonitor->SetDesiredError("VUID-vkRegisterDisplayEventEXT-commonparent");
    vk::RegisterDisplayEventEXT(device(), display, &event_info, nullptr, &fence);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeParent, PhysicalDevice_DisplayMode) {
    TEST_DESCRIPTION("VkDisplayModeKHR from a different physical device in vkGetDisplayPlaneCapabilitiesKHR");
    AddRequiredExtensions(VK_KHR_DISPLAY_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    const VkInstanceCreateInfo instance_create_info = GetInstanceCreateInfo();
    Instance instance2;
    ASSERT_EQ(VK_SUCCESS, vk::CreateInstance(&instance_create_info, nullptr, &instance2.handle));

    VkPhysicalDevice instance2_gpu = VK_NULL_HANDLE;
    {
        uint32_t gpu_count = 0;
        vk::EnumeratePhysicalDevices(instance2, &gpu_count, nullptr);
        ASSERT_GT(gpu_count, 0);
        std::vector<VkPhysicalDevice> physical_devices(gpu_count);
        vk::EnumeratePhysicalDevices(instance2, &gpu_count, physical_devices.data());
        instance2_gpu = physical_devices[0];
    }
    VkDisplayKHR display = VK_NULL_HANDLE;
    {
        uint32_t plane_count = 0;
        ASSERT_EQ(VK_SUCCESS, vk::GetPhysicalDeviceDisplayPlanePropertiesKHR(instance2_gpu, &plane_count, nullptr));
        if (plane_count == 0) {
            GTEST_SKIP() << "No display planes found";
        }
        std::vector<VkDisplayPlanePropertiesKHR> display_planes(plane_count);
        ASSERT_EQ(VK_SUCCESS, vk::GetPhysicalDeviceDisplayPlanePropertiesKHR(instance2_gpu, &plane_count, display_planes.data()));
        display = display_planes[0].currentDisplay;
        if (display == VK_NULL_HANDLE) {
            GTEST_SKIP() << "Null display";
        }
    }
    VkDisplayModeKHR display_mode = VK_NULL_HANDLE;
    {
        VkDisplayModeParametersKHR display_mode_parameters = {{32, 32}, 30};
        VkDisplayModeCreateInfoKHR display_mode_info = vku::InitStructHelper();
        display_mode_info.parameters = display_mode_parameters;
        vk::CreateDisplayModeKHR(instance2_gpu, display, &display_mode_info, nullptr, &display_mode);
        if (display_mode == VK_NULL_HANDLE) {
            GTEST_SKIP() << "Can't create a VkDisplayMode";
        }
    }
    // display mode from a different physical device
    VkDisplayPlaneCapabilitiesKHR plane_capabilities{};
    m_errorMonitor->SetDesiredError("VUID-vkGetDisplayPlaneCapabilitiesKHR-mode-parent");
    vk::GetDisplayPlaneCapabilitiesKHR(gpu(), display_mode, 0, &plane_capabilities);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeParent, PipelineExecutableInfo) {
    TEST_DESCRIPTION("Try making calls without pipelineExecutableInfo.");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_PIPELINE_EXECUTABLE_PROPERTIES_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());

    VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR pipeline_exe_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(pipeline_exe_features);
    RETURN_IF_SKIP(InitState(nullptr, &pipeline_exe_features));
    InitRenderTarget();

    m_second_device = new vkt::Device(gpu_, m_device_extension_names, nullptr, &pipeline_exe_features);

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

    VkPipelineExecutableInfoKHR pipeline_exe_info = vku::InitStructHelper();
    pipeline_exe_info.pipeline = pipe.Handle();
    pipeline_exe_info.executableIndex = 0;

    VkPipelineInfoKHR pipeline_info = vku::InitStructHelper();
    pipeline_info.pipeline = pipe.Handle();

    uint32_t count;
    m_errorMonitor->SetDesiredError("VUID-vkGetPipelineExecutableStatisticsKHR-pipeline-03273");
    vk::GetPipelineExecutableStatisticsKHR(*m_second_device, &pipeline_exe_info, &count, nullptr);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkGetPipelineExecutableInternalRepresentationsKHR-pipeline-03277");
    vk::GetPipelineExecutableInternalRepresentationsKHR(*m_second_device, &pipeline_exe_info, &count, nullptr);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkGetPipelineExecutablePropertiesKHR-pipeline-03271");
    vk::GetPipelineExecutablePropertiesKHR(*m_second_device, &pipeline_info, &count, nullptr);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeParent, UpdateDescriptorSetsBuffer) {
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    auto features = m_device->phy().features();
    m_second_device = new vkt::Device(gpu_, m_device_extension_names, &features, nullptr);

    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.size = 4096;
    buffer_ci.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vkt::Buffer buffer(*m_second_device, buffer_ci);

    OneOffDescriptorSet ds(m_device, {
                                         {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                     });
    ds.WriteDescriptorBufferInfo(0, buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

    m_errorMonitor->SetDesiredError("VUID-vkUpdateDescriptorSets-pDescriptorWrites-06237");
    ds.UpdateDescriptorSets();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeParent, UpdateDescriptorSetsImage) {
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    auto features = m_device->phy().features();
    m_second_device = new vkt::Device(gpu_, m_device_extension_names, &features, nullptr);

    vkt::Image image(*m_second_device, 32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::ImageView image_view = image.CreateView();

    OneOffDescriptorSet ds(m_device, {
                                         {0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 2, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                     });
    ds.WriteDescriptorImageInfo(0, image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);

    m_errorMonitor->SetDesiredError("VUID-vkUpdateDescriptorSets-pDescriptorWrites-06239");
    ds.UpdateDescriptorSets();
    m_errorMonitor->VerifyFound();
}
