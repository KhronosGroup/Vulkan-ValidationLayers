
#include "utils/cast_utils.h"
#include "generated/enum_flag_bits.h"
#include "../framework/layer_validation_tests.h"

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

ParentTest::~ParentTest() {
    if (m_second_device) {
        delete m_second_device;
        m_second_device = nullptr;
    }
}

TEST_F(NegativeParent, FillBuffer) {
    TEST_DESCRIPTION("Test VUID-*-commonparent checks not sharing the same Device");

    RETURN_IF_SKIP(Init())
    auto features = m_device->phy().features();
    m_second_device = new vkt::Device(gpu_, m_device_extension_names, &features, nullptr);

    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.size = 4096;
    buffer_ci.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buffer_ci.queueFamilyIndexCount = 0;
    vkt::Buffer buffer(*m_second_device, buffer_ci);

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdFillBuffer-commonparent");
    vk::CmdFillBuffer(m_commandBuffer->handle(), buffer, 0, VK_WHOLE_SIZE, 0);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(NegativeParent, BindBuffer) {
    TEST_DESCRIPTION("Test VUID-*-commonparent checks not sharing the same Device");

    AddRequiredExtensions(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())
    auto features = m_device->phy().features();
    m_second_device = new vkt::Device(gpu_, m_device_extension_names, &features, nullptr);

    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.size = 4096;
    buffer_ci.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buffer_ci.queueFamilyIndexCount = 0;
    vkt::Buffer buffer;
    buffer.init_no_mem(*m_device, buffer_ci);

    VkMemoryRequirements mem_reqs;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetBufferMemoryRequirements-buffer-parent");
    vk::GetBufferMemoryRequirements(m_second_device->device(), buffer.handle(), &mem_reqs);
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

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindBufferMemoryInfo-commonparent");
    vk::BindBufferMemory2KHR(device(), 1, &bind_buffer_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeParent, BindImage) {
    TEST_DESCRIPTION("Test VUID-*-commonparent checks not sharing the same Device");

    AddRequiredExtensions(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())
    auto features = m_device->phy().features();
    m_second_device = new vkt::Device(gpu_, m_device_extension_names, &features, nullptr);

    VkImageObj image(m_device);
    auto image_ci = VkImageObj::ImageCreateInfo2D(128, 128, 1, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                                                  VK_IMAGE_TILING_OPTIMAL);
    image.Init(image_ci);

    VkMemoryRequirements mem_reqs;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetImageMemoryRequirements-image-parent");
    vk::GetImageMemoryRequirements(m_second_device->device(), image.handle(), &mem_reqs);
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

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindImageMemoryInfo-commonparent");
    vk::BindImageMemory2KHR(device(), 1, &bind_image_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeParent, ImageView) {
    TEST_DESCRIPTION("Test VUID-*-commonparent checks not sharing the same Device");

    RETURN_IF_SKIP(Init())
    auto features = m_device->phy().features();
    m_second_device = new vkt::Device(gpu_, m_device_extension_names, &features, nullptr);

    VkImageObj image(m_device);
    auto image_ci = VkImageObj::ImageCreateInfo2D(128, 128, 1, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT,
                                                  VK_IMAGE_TILING_OPTIMAL);
    image.Init(image_ci);

    VkImageView image_view;
    VkImageViewCreateInfo ivci = vku::InitStructHelper();
    ivci.image = image.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = VK_FORMAT_B8G8R8A8_UNORM;
    ivci.subresourceRange.layerCount = 1;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.levelCount = 1;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCreateImageView-image-09179");
    vk::CreateImageView(m_second_device->device(), &ivci, nullptr, &image_view);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeParent, BindPipeline) {
    TEST_DESCRIPTION("Test binding pipeline from another device");

    RETURN_IF_SKIP(Init())
    auto features = m_device->phy().features();
    m_second_device = new vkt::Device(gpu_, m_device_extension_names, &features, nullptr);

    VkPipelineLayoutCreateInfo pipeline_layout_ci = vku::InitStructHelper();
    pipeline_layout_ci.setLayoutCount = 0;
    vkt::PipelineLayout pipeline_layout(*m_second_device, pipeline_layout_ci);

    VkShaderObj cs(this, kMinimalShaderGlsl, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL_TRY);
    cs.InitFromGLSLTry(false, m_second_device);

    VkComputePipelineCreateInfo pipeline_ci = vku::InitStructHelper();
    pipeline_ci.layout = pipeline_layout.handle();
    pipeline_ci.stage = cs.GetStageCreateInfo();
    vkt::Pipeline pipeline(*m_second_device, pipeline_ci);

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindPipeline-commonparent");
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.handle());
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(NegativeParent, Instance_PhysicalDeviceAndSurface) {
    TEST_DESCRIPTION("Surface from a different instance in vkGetPhysicalDeviceSurfaceSupportKHR");
    AddSurfaceExtension();
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    const auto instance_create_info = GetInstanceCreateInfo();
    Instance instance2;
    ASSERT_EQ(VK_SUCCESS, vk::CreateInstance(&instance_create_info, nullptr, &instance2.handle));

    SurfaceContext surface_context;
    Surface instance2_surface(instance2);
    if (!CreateSurface(surface_context, instance2_surface.handle, instance2)) {
        GTEST_SKIP() << "Cannot create surface";
    }

    VkBool32 supported = VK_FALSE;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetPhysicalDeviceSurfaceSupportKHR-commonparent");
    vk::GetPhysicalDeviceSurfaceSupportKHR(gpu(), m_device->graphics_queue_node_index_, instance2_surface, &supported);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeParent, Instance_DeviceAndSurface) {
    TEST_DESCRIPTION("Surface from a different instance in vkGetDeviceGroupSurfacePresentModesKHR");
    AddSurfaceExtension();
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    const auto instance_create_info = GetInstanceCreateInfo();
    Instance instance2;
    ASSERT_EQ(VK_SUCCESS, vk::CreateInstance(&instance_create_info, nullptr, &instance2.handle));

    SurfaceContext surface_context;
    Surface instance2_surface(instance2);
    if (!CreateSurface(surface_context, instance2_surface.handle, instance2)) {
        GTEST_SKIP() << "Cannot create surface";
    }

    VkDeviceGroupPresentModeFlagsKHR flags = 0;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetDeviceGroupSurfacePresentModesKHR-commonparent");
    vk::GetDeviceGroupSurfacePresentModesKHR(m_device->handle(), instance2_surface, &flags);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeParent, Instance_Surface) {
    TEST_DESCRIPTION("Surface from a different instance in vkCreateSwapchainKHR");
    AddSurfaceExtension();
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())
    if (!InitSurface()) {
        GTEST_SKIP() << "Cannot create surface";
    }
    InitSwapchainInfo();

    const auto instance_create_info = GetInstanceCreateInfo();
    Instance instance2;
    ASSERT_EQ(VK_SUCCESS, vk::CreateInstance(&instance_create_info, nullptr, &instance2.handle));

    SurfaceContext surface_context;
    Surface instance2_surface(instance2);
    if (!CreateSurface(surface_context, instance2_surface.handle, instance2)) {
        GTEST_SKIP() << "Cannot create surface";
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
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSwapchainCreateInfoKHR-commonparent");
    vk::CreateSwapchainKHR(device(), &swapchain_ci, nullptr, &swapchain);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeParent, Device_OldSwapchain) {
    TEST_DESCRIPTION("oldSwapchain from a different device in vkCreateSwapchainKHR");
    AddSurfaceExtension();
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())
    if (!InitSurface()) {
        GTEST_SKIP() << "Cannot create surface";
    }
    InitSwapchainInfo();

    const auto instance_create_info = GetInstanceCreateInfo();
    Instance instance2;
    ASSERT_EQ(VK_SUCCESS, vk::CreateInstance(&instance_create_info, nullptr, &instance2.handle));

    SurfaceContext surface_context;
    Surface instance2_surface(instance2);
    if (!CreateSurface(surface_context, instance2_surface.handle, instance2)) {
        GTEST_SKIP() << "Cannot create surface";
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
    ASSERT_EQ(VK_SUCCESS, vk::CreateSwapchainKHR(instance2_device.device(), &swapchain_ci, nullptr, &other_device_swapchain));

    // oldSwapchain from a different device
    swapchain_ci.surface = m_surface;
    swapchain_ci.oldSwapchain = other_device_swapchain;
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSwapchainCreateInfoKHR-commonparent");
    vk::CreateSwapchainKHR(device(), &swapchain_ci, nullptr, &swapchain);
    m_errorMonitor->VerifyFound();
    vk::DestroySwapchainKHR(instance2_device.device(), other_device_swapchain, nullptr);
}

TEST_F(NegativeParent, Instance_Surface_2) {
    TEST_DESCRIPTION("Surface from a different instance in vkDestroySurfaceKHR");
    AddSurfaceExtension();
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    const auto instance_create_info = GetInstanceCreateInfo();
    Instance instance2;
    ASSERT_EQ(VK_SUCCESS, vk::CreateInstance(&instance_create_info, nullptr, &instance2.handle));

    SurfaceContext surface_context;
    Surface instance2_surface(instance2);
    if (!CreateSurface(surface_context, instance2_surface.handle, instance2)) {
        GTEST_SKIP() << "Cannot create surface";
    }

    // surface from a different instance
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroySurfaceKHR-surface-parent");
    vk::DestroySurfaceKHR(instance(), instance2_surface, nullptr);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeParent, Instance_DebugUtilsMessenger) {
    TEST_DESCRIPTION("VkDebugUtilsMessengerEXT from a different instance in vkDestroyDebugUtilsMessengerEXT");
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

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
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroyDebugUtilsMessengerEXT-messenger-parent");
    vk::DestroyDebugUtilsMessengerEXT(instance(), messenger, nullptr);
    m_errorMonitor->VerifyFound();
    vk::DestroyDebugUtilsMessengerEXT(instance2, messenger, nullptr);
}

TEST_F(NegativeParent, Instance_DebugReportCallback) {
    TEST_DESCRIPTION("VkDebugReportCallbackEXT from a different instance in vkDestroyDebugReportCallbackEXT");
    AddRequiredExtensions(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

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
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroyDebugReportCallbackEXT-callback-parent");
    vk::DestroyDebugReportCallbackEXT(instance(), callback, nullptr);
    m_errorMonitor->VerifyFound();
    vk::DestroyDebugReportCallbackEXT(instance2, callback, nullptr);
}

TEST_F(NegativeParent, PhysicalDevice_Display) {
    TEST_DESCRIPTION("VkDisplayKHR from a different physical device in vkGetDisplayModePropertiesKHR");
    AddRequiredExtensions(VK_KHR_DISPLAY_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

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
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetDisplayModePropertiesKHR-display-parent");
    vk::GetDisplayModePropertiesKHR(gpu(), display, &mode_count, nullptr);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeParent, PhysicalDevice_DisplayMode) {
    TEST_DESCRIPTION("VkDisplayModeKHR from a different physical device in vkGetDisplayPlaneCapabilitiesKHR");
    AddRequiredExtensions(VK_KHR_DISPLAY_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

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
        uint32_t mode_count = 0;
        ASSERT_EQ(VK_SUCCESS, vk::GetDisplayModePropertiesKHR(instance2_gpu, display, &mode_count, nullptr));
        if (mode_count == 0) {
            GTEST_SKIP() << "No display modes found";
        }
        std::vector<VkDisplayModePropertiesKHR> display_modes(mode_count);
        ASSERT_EQ(VK_SUCCESS, vk::GetDisplayModePropertiesKHR(instance2_gpu, display, &mode_count, display_modes.data()));
        display_mode = display_modes[0].displayMode;
        if (display_mode == VK_NULL_HANDLE) {
            GTEST_SKIP() << "Null display mode";
        }
    }
    // display mode from a different physical device
    VkDisplayPlaneCapabilitiesKHR plane_capabilities{};
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetDisplayPlaneCapabilitiesKHR-mode-parent");
    vk::GetDisplayPlaneCapabilitiesKHR(gpu(), display_mode, 0, &plane_capabilities);
    m_errorMonitor->VerifyFound();
}
