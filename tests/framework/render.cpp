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
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "render.h"

#include <cassert>
#include <cstring>

#include <vulkan/utility/vk_format_utils.h>

#include "generated/vk_extension_helper.h"
#include "utils/vk_layer_utils.h"
#include "layer_validation_tests.h"

using std::string;
using std::strncmp;
using std::vector;

template <typename C, typename F>
typename C::iterator RemoveIf(C &container, F &&fn) {
    return container.erase(std::remove_if(container.begin(), container.end(), std::forward<F>(fn)), container.end());
}

VkRenderFramework::VkRenderFramework()
    : instance_(NULL),
      m_device(NULL),
      m_commandPool(VK_NULL_HANDLE),
      m_commandBuffer(NULL),
      m_renderPass(VK_NULL_HANDLE),
      m_framebuffer(VK_NULL_HANDLE),
      m_addRenderPassSelfDependency(false),
      m_width(256),   // default window width
      m_height(256),  // default window height
      m_render_target_fmt(VK_FORMAT_R8G8B8A8_UNORM),
      m_depth_stencil_fmt(VK_FORMAT_UNDEFINED),
      m_depth_stencil_layout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL),
      m_clear_via_load_op(true),
      m_depth_clear_color(1.0),
      m_stencil_clear_color(0),
      m_depthStencil(NULL) {
    m_framebuffer_info = vku::InitStructHelper();
    m_renderPass_info = vku::InitStructHelper();
    m_renderPassBeginInfo = vku::InitStructHelper();

    // clear the back buffer to dark grey
    m_clear_color.float32[0] = 0.25f;
    m_clear_color.float32[1] = 0.25f;
    m_clear_color.float32[2] = 0.25f;
    m_clear_color.float32[3] = 0.0f;
}

VkRenderFramework::~VkRenderFramework() {
    ShutdownFramework();
    m_errorMonitor->Finish();
}

VkPhysicalDevice VkRenderFramework::gpu() const {
    EXPECT_NE((VkInstance)0, instance_);  // Invalid to request gpu before instance exists
    return gpu_;
}

const VkPhysicalDeviceProperties &VkRenderFramework::physDevProps() const {
    EXPECT_NE((VkPhysicalDevice)0, gpu_);  // Invalid to request physical device properties before gpu
    return physDevProps_;
}

// Return true if layer name is found and spec+implementation values are >= requested values
bool VkRenderFramework::InstanceLayerSupported(const char *const layer_name, const uint32_t spec_version,
                                               const uint32_t impl_version) {

    if (available_layers_.empty()) {
        available_layers_ = vkt::GetGlobalLayers();
    }

    for (const auto &layer : available_layers_) {
        if (0 == strncmp(layer_name, layer.layerName, VK_MAX_EXTENSION_NAME_SIZE)) {
            return layer.specVersion >= spec_version && layer.implementationVersion >= impl_version;
        }
    }
    return false;
}

// Return true if extension name is found and spec value is >= requested spec value
// WARNING: for simplicity, does not cover layers' extensions
bool VkRenderFramework::InstanceExtensionSupported(const char *const extension_name, const uint32_t spec_version) {
    // WARNING: assume debug and validation feature extensions are always supported, which are usually provided by layers
    if (0 == strncmp(extension_name, VK_EXT_DEBUG_UTILS_EXTENSION_NAME, VK_MAX_EXTENSION_NAME_SIZE)) return true;
    if (0 == strncmp(extension_name, VK_EXT_DEBUG_REPORT_EXTENSION_NAME, VK_MAX_EXTENSION_NAME_SIZE)) return true;
    if (0 == strncmp(extension_name, VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME, VK_MAX_EXTENSION_NAME_SIZE)) return true;

    if (available_extensions_.empty()) {
        available_extensions_ = vkt::GetGlobalExtensions();
    }

    const auto IsTheQueriedExtension = [extension_name, spec_version](const VkExtensionProperties &extension) {
        return strncmp(extension_name, extension.extensionName, VK_MAX_EXTENSION_NAME_SIZE) == 0 &&
               extension.specVersion >= spec_version;
    };

    return std::any_of(available_extensions_.begin(), available_extensions_.end(), IsTheQueriedExtension);
}

// Return true if extension name is found and spec value is >= requested spec value
bool VkRenderFramework::DeviceExtensionSupported(const char *extension_name, const uint32_t spec_version) const {
    if (!instance_ || !gpu_) {
        EXPECT_NE((VkInstance)0, instance_);  // Complain, not cool without an instance
        EXPECT_NE((VkPhysicalDevice)0, gpu_);
        return false;
    }

    const vkt::PhysicalDevice device_obj(gpu_);

    const auto enabled_layers = instance_layers_;  // assumes instance_layers_ contains enabled layers

    auto extensions = device_obj.extensions();
    for (const auto &layer : enabled_layers) {
        const auto layer_extensions = device_obj.extensions(layer);
        extensions.insert(extensions.end(), layer_extensions.begin(), layer_extensions.end());
    }

    const auto IsTheQueriedExtension = [extension_name, spec_version](const VkExtensionProperties &extension) {
        return strncmp(extension_name, extension.extensionName, VK_MAX_EXTENSION_NAME_SIZE) == 0 &&
               extension.specVersion >= spec_version;
    };

    return std::any_of(extensions.begin(), extensions.end(), IsTheQueriedExtension);
}

VkInstanceCreateInfo VkRenderFramework::GetInstanceCreateInfo() const {
    VkInstanceCreateInfo info = vku::InitStructHelper();
    info.pNext = m_errorMonitor->GetDebugCreateInfo();
#if defined(VK_USE_PLATFORM_METAL_EXT)
    info.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif
    info.pApplicationInfo = &app_info_;
    info.enabledLayerCount = size32(instance_layers_);
    info.ppEnabledLayerNames = instance_layers_.data();
    info.enabledExtensionCount = size32(m_instance_extension_names);
    info.ppEnabledExtensionNames = m_instance_extension_names.data();
    return info;
}

inline void CheckDisableCoreValidation(VkValidationFeaturesEXT &features) {
    auto disable = GetEnvironment("VK_LAYER_TESTS_DISABLE_CORE_VALIDATION");
    vvl::ToLower(disable);
    if (disable == "false" || disable == "0" || disable == "FALSE") {       // default is to change nothing, unless flag is correctly specified
        features.disabledValidationFeatureCount = 0;                        // remove all disables to get all validation messages
    }
}

void *VkRenderFramework::SetupValidationSettings(void *first_pnext) {
    auto validation = GetEnvironment("VK_LAYER_TESTS_VALIDATION_FEATURES");
    vvl::ToLower(validation);
    VkValidationFeaturesEXT *features = vku::FindStructInPNextChain<VkValidationFeaturesEXT>(first_pnext);
    if (features) {
        CheckDisableCoreValidation(*features);
    }
    if (validation == "all" || validation == "core" || validation == "none") {
        if (!features) {
            features = &m_validation_features;
            features->sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
            features->pNext = first_pnext;
            first_pnext = features;
        }

        if (validation == "all") {
            features->enabledValidationFeatureCount = 4;
            features->pEnabledValidationFeatures = validation_enable_all;
            features->disabledValidationFeatureCount = 0;
        } else if (validation == "core") {
            features->disabledValidationFeatureCount = 0;
        } else if (validation == "none") {
            features->disabledValidationFeatureCount = 1;
            features->pDisabledValidationFeatures = &validation_disable_all;
            features->enabledValidationFeatureCount = 0;
        }
    }

    return first_pnext;
}

void VkRenderFramework::InitFramework(void *instance_pnext) {
    ASSERT_EQ((VkInstance)0, instance_);

    const auto LayerNotSupportedWithReporting = [this](const char *layer) {
        if (InstanceLayerSupported(layer))
            return false;
        else {
            ADD_FAILURE() << "InitFramework(): Requested layer \"" << layer << "\" is not supported. It will be disabled.";
            return true;
        }
    };
    const auto ExtensionNotSupportedWithReporting = [this](const char *extension) {
        if (InstanceExtensionSupported(extension))
            return false;
        else {
            ADD_FAILURE() << "InitFramework(): Requested extension \"" << extension << "\" is not supported. It will be disabled.";
            return true;
        }
    };

    static bool driver_printed = false;
    static bool print_driver_info = GetEnvironment("VK_LAYER_TESTS_PRINT_DRIVER") != "";
    if (print_driver_info && !driver_printed &&
        InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }

    // Beginning with the 1.3.216 Vulkan SDK, the VK_KHR_PORTABILITY_subset extension is mandatory.
#ifdef VK_USE_PLATFORM_METAL_EXT
    AddRequiredExtensions(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
#else
    // Note by default VK_KHRONOS_PROFILES_EMULATE_PORTABILITY is true.
    if (auto str = GetEnvironment("VK_KHRONOS_PROFILES_EMULATE_PORTABILITY"); !str.empty() && str != "false") {
        AddRequiredExtensions(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        AddRequiredExtensions(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
    }
#endif

    RemoveIf(instance_layers_, LayerNotSupportedWithReporting);
    RemoveIf(m_instance_extension_names, ExtensionNotSupportedWithReporting);

    auto ici = GetInstanceCreateInfo();

    // If is validation features then check for disabled validation

    instance_pnext = SetupValidationSettings(instance_pnext);

    // concatenate pNexts
    void *last_pnext = nullptr;
    if (instance_pnext) {
        last_pnext = instance_pnext;
        while (reinterpret_cast<const VkBaseOutStructure *>(last_pnext)->pNext)
            last_pnext = reinterpret_cast<VkBaseOutStructure *>(last_pnext)->pNext;

        void *&link = reinterpret_cast<void *&>(reinterpret_cast<VkBaseOutStructure *>(last_pnext)->pNext);
        link = const_cast<void *>(ici.pNext);
        ici.pNext = instance_pnext;
    }

    ASSERT_EQ(VK_SUCCESS, vk::CreateInstance(&ici, nullptr, &instance_));
    if (instance_pnext) reinterpret_cast<VkBaseOutStructure *>(last_pnext)->pNext = nullptr;  // reset back borrowed pNext chain

    vk::ResetAllExtensions();
    for (const char *instance_ext_name : m_instance_extension_names) {
        vk::InitInstanceExtension(instance_, instance_ext_name);
    }

    // Choose a physical device
    uint32_t gpu_count = 0;
    const VkResult err = vk::EnumeratePhysicalDevices(instance_, &gpu_count, nullptr);
    ASSERT_TRUE(err == VK_SUCCESS || err == VK_INCOMPLETE) << string_VkResult(err);
    ASSERT_GT(gpu_count, (uint32_t)0) << "No GPU (i.e. VkPhysicalDevice) available";

    std::vector<VkPhysicalDevice> phys_devices(gpu_count);
    vk::EnumeratePhysicalDevices(instance_, &gpu_count, phys_devices.data());

    const int phys_device_index = VkTestFramework::m_phys_device_index;
    if ((phys_device_index >= 0) && (phys_device_index < static_cast<int>(gpu_count))) {
        gpu_ = phys_devices[phys_device_index];
        vk::GetPhysicalDeviceProperties(gpu_, &physDevProps_);
        m_gpu_index = phys_device_index;
    } else {
        // Specify a "physical device priority" with larger values meaning higher priority.
        std::array<int, VK_PHYSICAL_DEVICE_TYPE_CPU + 1> device_type_rank;
        device_type_rank[VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU] = 4;
        device_type_rank[VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU] = 3;
        device_type_rank[VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU] = 2;
        device_type_rank[VK_PHYSICAL_DEVICE_TYPE_CPU] = 1;
        device_type_rank[VK_PHYSICAL_DEVICE_TYPE_OTHER] = 0;

        // Initialize physical device and properties with first device found
        gpu_ = phys_devices[0];
        m_gpu_index = 0;
        vk::GetPhysicalDeviceProperties(gpu_, &physDevProps_);

        // See if there are any higher priority devices found
        for (size_t i = 1; i < phys_devices.size(); ++i) {
            VkPhysicalDeviceProperties tmp_props;
            vk::GetPhysicalDeviceProperties(phys_devices[i], &tmp_props);
            if (device_type_rank[tmp_props.deviceType] > device_type_rank[physDevProps_.deviceType]) {
                physDevProps_ = tmp_props;
                gpu_ = phys_devices[i];
                m_gpu_index = i;
            }
        }
    }

    m_errorMonitor->CreateCallback(instance_);

    if (print_driver_info && !driver_printed) {
        VkPhysicalDeviceDriverProperties driver_properties = vku::InitStructHelper();
        VkPhysicalDeviceProperties2 physical_device_properties2 = vku::InitStructHelper(&driver_properties);
        vk::GetPhysicalDeviceProperties2(gpu_, &physical_device_properties2);
        printf("Driver Name = %s\n", driver_properties.driverName);
        printf("Driver Info = %s\n", driver_properties.driverInfo);

        driver_printed = true;
    }

    APIVersion used_version = std::min(m_instance_api_version, APIVersion(physDevProps_.apiVersion));
    if (used_version < m_target_api_version) {
        GTEST_SKIP() << "At least Vulkan version 1." << m_target_api_version.Minor() << " is required";
    }

    for (const auto &ext : m_required_extensions) {
        AddRequestedDeviceExtensions(ext);
    }

    if (!std::all_of(m_required_extensions.begin(), m_required_extensions.end(),
                     [&](const char *ext) -> bool { return IsExtensionsEnabled(ext); })) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    // If the user requested wsi extension(s), only 1 needs to be enabled.
    if (!m_wsi_extensions.empty()) {
        if (!std::any_of(m_wsi_extensions.begin(), m_wsi_extensions.end(),
                         [&](const char *ext) -> bool { return CanEnableInstanceExtension(ext); })) {
            GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
        }
    }

    for (const auto &ext : m_optional_extensions) {
        AddRequestedDeviceExtensions(ext);
    }
}

void VkRenderFramework::AddRequiredExtensions(const char *ext_name) {
    m_required_extensions.push_back(ext_name);
    AddRequestedInstanceExtensions(ext_name);
}

void VkRenderFramework::AddOptionalExtensions(const char *ext_name) {
    m_optional_extensions.push_back(ext_name);
    AddRequestedInstanceExtensions(ext_name);
}

void VkRenderFramework::AddWsiExtensions(const char *ext_name) {
    m_wsi_extensions.push_back(ext_name);
    AddRequestedInstanceExtensions(ext_name);
}

bool VkRenderFramework::IsExtensionsEnabled(const char *ext_name) const {
    return (CanEnableDeviceExtension(ext_name) || CanEnableInstanceExtension(ext_name));
}

std::string VkRenderFramework::RequiredExtensionsNotSupported() const {
    std::stringstream ss;
    bool first = true;
    for (const auto &ext : m_required_extensions) {
        if (!CanEnableDeviceExtension(ext) && !CanEnableInstanceExtension(ext)) {
            if (first) {
                first = false;
            } else {
                ss << ", ";
            }
            ss << ext;
        }
    }
    if (!m_wsi_extensions.empty() && ss.str().empty()) {
        ss << "Unable to find at least 1 supported WSI extension";
    }
    return ss.str();
}

bool VkRenderFramework::AddRequestedInstanceExtensions(const char *ext_name) {
    if (CanEnableInstanceExtension(ext_name)) {
        return true;
    }

    const auto &instance_exts_map = InstanceExtensions::get_info_map();
    bool is_instance_ext = false;
    if (instance_exts_map.count(ext_name) > 0) {
        if (!InstanceExtensionSupported(ext_name)) {
            return false;
        } else {
            is_instance_ext = true;
        }
    }

    // Different tables need to be used for extension dependency lookup depending on whether `ext_name` refers to a device or
    // instance extension
    if (is_instance_ext) {
        const auto &info = InstanceExtensions::get_info(ext_name);
        for (const auto &req : info.requirements) {
            if (0 == strncmp(req.name, "VK_VERSION", 10)) {
                continue;
            }
            if (!AddRequestedInstanceExtensions(req.name)) {
                return false;
            }
        }
        m_instance_extension_names.push_back(ext_name);
    } else {
        const auto &info = DeviceExtensions::get_info(ext_name);
        for (const auto &req : info.requirements) {
            if (!AddRequestedInstanceExtensions(req.name)) {
                return false;
            }
        }
    }

    return true;
}

bool VkRenderFramework::CanEnableInstanceExtension(const std::string &inst_ext_name) const {
    return std::any_of(m_instance_extension_names.cbegin(), m_instance_extension_names.cend(),
                       [&inst_ext_name](const char *ext) { return inst_ext_name == ext; });
}

bool VkRenderFramework::AddRequestedDeviceExtensions(const char *dev_ext_name) {
    // Check if the extension has already been added
    if (CanEnableDeviceExtension(dev_ext_name)) {
        return true;
    }

    // If this is an instance extension, just return true under the assumption instance extensions do not depend on any device
    // extensions.
    const auto &instance_exts_map = InstanceExtensions::get_info_map();
    if (instance_exts_map.count(dev_ext_name) != 0) {
        return true;
    }

    if (!DeviceExtensionSupported(gpu(), nullptr, dev_ext_name)) {
        return false;
    }
    m_device_extension_names.push_back(dev_ext_name);

    const auto &info = DeviceExtensions::get_info(dev_ext_name);
    for (const auto &req : info.requirements) {
        if (!AddRequestedDeviceExtensions(req.name)) {
            return false;
        }
    }
    return true;
}

bool VkRenderFramework::CanEnableDeviceExtension(const std::string &dev_ext_name) const {
    return std::any_of(m_device_extension_names.cbegin(), m_device_extension_names.cend(),
                       [&dev_ext_name](const char *ext) { return dev_ext_name == ext; });
}

void VkRenderFramework::ShutdownFramework() {
    // Nothing to shut down without a VkInstance
    if (!instance_) return;

    if (m_device && m_device->device() != VK_NULL_HANDLE) {
        vk::DeviceWaitIdle(device());
    }

    delete m_commandBuffer;
    m_commandBuffer = nullptr;
    delete m_commandPool;
    m_commandPool = nullptr;
    if (m_framebuffer) vk::DestroyFramebuffer(device(), m_framebuffer, NULL);
    m_framebuffer = VK_NULL_HANDLE;
    if (m_renderPass) vk::DestroyRenderPass(device(), m_renderPass, NULL);
    m_renderPass = VK_NULL_HANDLE;

    m_renderTargets.clear();

    delete m_depthStencil;
    m_depthStencil = nullptr;

    DestroySwapchain();

    // reset the driver
    delete m_device;
    m_device = nullptr;

    m_errorMonitor->DestroyCallback(instance_);

    DestroySurface(m_surface);
    DestroySurfaceContext(m_surface_context);

    vk::DestroyInstance(instance_, nullptr);
    instance_ = NULL;  // In case we want to re-initialize
    vk::ResetAllExtensions();
}

ErrorMonitor &VkRenderFramework::Monitor() { return monitor_; }

void VkRenderFramework::GetPhysicalDeviceFeatures(VkPhysicalDeviceFeatures *features) {
    vk::GetPhysicalDeviceFeatures(gpu(), features);
}

// static
bool VkRenderFramework::IgnoreDisableChecks() {
    static const bool skip_disable_checks = GetEnvironment("VK_LAYER_TESTS_IGNORE_DISABLE_CHECKS") != "";
    return skip_disable_checks;
}

static const std::string mock_icd_device_name = "Vulkan Mock Device";
bool VkRenderFramework::IsPlatformMockICD() {
    if (VkRenderFramework::IgnoreDisableChecks()) {
        return false;
    } else {
        return 0 == mock_icd_device_name.compare(physDevProps().deviceName);
    }
}

void VkRenderFramework::GetPhysicalDeviceProperties(VkPhysicalDeviceProperties *props) { *props = physDevProps_; }

VkFormat VkRenderFramework::GetRenderTargetFormat() {
    VkFormatProperties format_props = {};
    vk::GetPhysicalDeviceFormatProperties(gpu_, VK_FORMAT_B8G8R8A8_UNORM, &format_props);
    if (format_props.linearTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT ||
        format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT) {
        return VK_FORMAT_B8G8R8A8_UNORM;
    }
    vk::GetPhysicalDeviceFormatProperties(gpu_, VK_FORMAT_R8G8B8A8_UNORM, &format_props);
    if (format_props.linearTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT ||
        format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT) {
        return VK_FORMAT_R8G8B8A8_UNORM;
    }
    // According to VulkanCapsViewer rgba8/bgra8 support with optimal tiling + color_attachment is 99.45% across all platforms
    assert(false);
    return VK_FORMAT_UNDEFINED;
}

void VkRenderFramework::InitState(VkPhysicalDeviceFeatures *features, void *create_device_pnext,
                                  const VkCommandPoolCreateFlags flags) {
    const auto ExtensionNotSupportedWithReporting = [this](const char *extension) {
        if (DeviceExtensionSupported(extension))
            return false;
        else {
            ADD_FAILURE() << "InitState(): Requested device extension \"" << extension
                          << "\" is not supported. It will be disabled.";
            return true;
        }
    };

    RemoveIf(m_device_extension_names, ExtensionNotSupportedWithReporting);

    m_device = new vkt::Device(gpu_, m_device_extension_names, features, create_device_pnext);

    for (const char *device_ext_name : m_device_extension_names) {
        vk::InitDeviceExtension(instance_, *m_device, device_ext_name);
    }

    m_default_queue = m_device->graphics_queues()[0]->handle();

    m_depthStencil = new VkImageObj(m_device);

    m_render_target_fmt = GetRenderTargetFormat();

    m_commandPool = new vkt::CommandPool(*m_device, m_device->graphics_queue_node_index_, flags);

    m_commandBuffer = new vkt::CommandBuffer(m_device, m_commandPool);
}

bool VkRenderFramework::InitSurface() {
    // NOTE: Currently InitSurface can leak the WIN32 handle if called multiple times without first calling DestroySurfaceContext.
    // This is intentional. Each swapchain/surface combo needs a unique HWND.
    return CreateSurface(m_surface_context, m_surface);
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

bool VkRenderFramework::CreateSurface(SurfaceContext &surface_context, VkSurfaceKHR &surface, VkInstance custom_instance) {
    const VkInstance surface_instance = (custom_instance != VK_NULL_HANDLE) ? custom_instance : instance();
    (void)surface_instance;
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    if (IsExtensionsEnabled(VK_KHR_WIN32_SURFACE_EXTENSION_NAME)) {
        HINSTANCE window_instance = GetModuleHandle(nullptr);
        const char class_name[] = "test";
        WNDCLASS wc = {};
        wc.lpfnWndProc = WindowProc;
        wc.hInstance = window_instance;
        wc.lpszClassName = class_name;
        RegisterClass(&wc);
        HWND window = CreateWindowEx(0, class_name, 0, 0, 0, 0, (int)m_width, (int)m_height, NULL, NULL, window_instance, NULL);
        ShowWindow(window, SW_HIDE);

        VkWin32SurfaceCreateInfoKHR surface_create_info = vku::InitStructHelper();
        surface_create_info.hinstance = window_instance;
        surface_create_info.hwnd = window;
        return VK_SUCCESS == vk::CreateWin32SurfaceKHR(surface_instance, &surface_create_info, nullptr, &surface);
    }
#endif

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    if (IsExtensionsEnabled(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME)) {
        VkAndroidSurfaceCreateInfoKHR surface_create_info = vku::InitStructHelper();
        surface_create_info.window = VkTestFramework::window;
        return VK_SUCCESS == vk::CreateAndroidSurfaceKHR(surface_instance, &surface_create_info, nullptr, &surface);
    }
#endif

#if defined(VK_USE_PLATFORM_XLIB_KHR)
    if (IsExtensionsEnabled(VK_KHR_XLIB_SURFACE_EXTENSION_NAME)) {
        surface_context.m_surface_dpy = XOpenDisplay(nullptr);
        if (surface_context.m_surface_dpy) {
            int s = DefaultScreen(surface_context.m_surface_dpy);
            surface_context.m_surface_window = XCreateSimpleWindow(
                surface_context.m_surface_dpy, RootWindow(surface_context.m_surface_dpy, s), 0, 0, (int)m_width, (int)m_height, 1,
                BlackPixel(surface_context.m_surface_dpy, s), WhitePixel(surface_context.m_surface_dpy, s));
            VkXlibSurfaceCreateInfoKHR surface_create_info = vku::InitStructHelper();
            surface_create_info.dpy = surface_context.m_surface_dpy;
            surface_create_info.window = surface_context.m_surface_window;
            return VK_SUCCESS == vk::CreateXlibSurfaceKHR(surface_instance, &surface_create_info, nullptr, &surface);
        }
    }
#endif

#if defined(VK_USE_PLATFORM_XCB_KHR)
    if (IsExtensionsEnabled(VK_KHR_XCB_SURFACE_EXTENSION_NAME)) {
        surface_context.m_surface_xcb_conn = xcb_connect(nullptr, nullptr);
        if (surface_context.m_surface_xcb_conn) {
            xcb_window_t window = xcb_generate_id(surface_context.m_surface_xcb_conn);
            VkXcbSurfaceCreateInfoKHR surface_create_info = vku::InitStructHelper();
            surface_create_info.connection = surface_context.m_surface_xcb_conn;
            surface_create_info.window = window;
            return VK_SUCCESS == vk::CreateXcbSurfaceKHR(surface_instance, &surface_create_info, nullptr, &surface);
        }
    }
#endif
    return surface != VK_NULL_HANDLE;
}

void VkRenderFramework::DestroySurface() {
    DestroySurface(m_surface);
    m_surface = VK_NULL_HANDLE;
    DestroySurfaceContext(m_surface_context);
    m_surface_context = {};
}

void VkRenderFramework::DestroySurface(VkSurfaceKHR &surface) {
    if (surface != VK_NULL_HANDLE) {
        vk::DestroySurfaceKHR(instance(), surface, nullptr);
    }
}
#if defined(VK_USE_PLATFORM_XLIB_KHR)
int IgnoreXErrors(Display *, XErrorEvent *) { return 0; }
#endif

void VkRenderFramework::DestroySurfaceContext(SurfaceContext &surface_context) {
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    if (surface_context.m_win32Window != nullptr) {
        DestroyWindow(surface_context.m_win32Window);
    }
#endif

#if defined(VK_USE_PLATFORM_XLIB_KHR)
    if (surface_context.m_surface_dpy != nullptr) {
        // Ignore BadDrawable errors we seem to get during shutdown.
        // The default error handler will exit() and end the test suite.
        XSetErrorHandler(IgnoreXErrors);
        XDestroyWindow(surface_context.m_surface_dpy, surface_context.m_surface_window);
        surface_context.m_surface_window = None;
        XCloseDisplay(surface_context.m_surface_dpy);
        surface_context.m_surface_dpy = nullptr;
        XSetErrorHandler(nullptr);
    }
#endif
#if defined(VK_USE_PLATFORM_XCB_KHR)
    if (surface_context.m_surface_xcb_conn != nullptr) {
        xcb_disconnect(surface_context.m_surface_xcb_conn);
        surface_context.m_surface_xcb_conn = nullptr;
    }
#endif
}

// Queries the info needed to create a swapchain and assigns it to the member variables of VkRenderFramework
void VkRenderFramework::InitSwapchainInfo() {
    auto info = GetSwapchainInfo(m_surface);
    m_surface_capabilities = info.surface_capabilities;
    m_surface_formats = info.surface_formats;
    m_surface_present_modes = info.surface_present_modes;
    m_surface_non_shared_present_mode = info.surface_non_shared_present_mode;
    m_surface_composite_alpha = info.surface_composite_alpha;
}

// Makes query to get information about swapchain needed to create a valid swapchain object each test creating a swapchain will
// need
SurfaceInformation VkRenderFramework::GetSwapchainInfo(const VkSurfaceKHR surface) {
    const VkPhysicalDevice physicalDevice = gpu();

    assert(surface != VK_NULL_HANDLE);

    SurfaceInformation info{};

    vk::GetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &info.surface_capabilities);

    uint32_t format_count;
    vk::GetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &format_count, nullptr);
    if (format_count != 0) {
        info.surface_formats.resize(format_count);
        vk::GetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &format_count, info.surface_formats.data());
    }

    uint32_t present_mode_count;
    vk::GetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &present_mode_count, nullptr);
    if (present_mode_count != 0) {
        info.surface_present_modes.resize(present_mode_count);
        vk::GetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &present_mode_count,
                                                    info.surface_present_modes.data());

        // Shared Present mode has different requirements most tests won't actually want
        // Implementation required to support a non-shared present mode
        for (size_t i = 0; i < info.surface_present_modes.size(); i++) {
            const VkPresentModeKHR present_mode = info.surface_present_modes[i];
            if ((present_mode != VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR) &&
                (present_mode != VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR)) {
                info.surface_non_shared_present_mode = present_mode;
                break;
            }
        }
    }

#ifdef VK_USE_PLATFORM_ANDROID_KHR
    info.surface_composite_alpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
#else
    info.surface_composite_alpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
#endif

    return info;
}

bool VkRenderFramework::InitSwapchain(VkImageUsageFlags imageUsage, VkSurfaceTransformFlagBitsKHR preTransform) {
    if (InitSurface()) {
        return CreateSwapchain(m_surface, imageUsage, preTransform, m_swapchain);
    }
    return false;
}

bool VkRenderFramework::CreateSwapchain(VkSurfaceKHR &surface, VkImageUsageFlags imageUsage,
                                        VkSurfaceTransformFlagBitsKHR preTransform, VkSwapchainKHR &swapchain,
                                        VkSwapchainKHR oldSwapchain) {
    VkBool32 supported;
    vk::GetPhysicalDeviceSurfaceSupportKHR(gpu(), m_device->graphics_queue_node_index_, surface, &supported);
    if (!supported) {
        // Graphics queue does not support present
        return false;
    }

    SurfaceInformation info = GetSwapchainInfo(surface);

    // If this is being called from InitSwapchain, we need to also initialize all the VkRenderFramework
    // data associated with the swapchain since many tests use those variables. We can do this by checking
    // if the surface parameters address is the same as VkRenderFramework::m_surface
    if (&surface == &m_surface) {
        InitSwapchainInfo();
    }

    VkSwapchainCreateInfoKHR swapchain_create_info = vku::InitStructHelper();
    swapchain_create_info.surface = surface;
    swapchain_create_info.minImageCount = info.surface_capabilities.minImageCount;
    swapchain_create_info.imageFormat = info.surface_formats[0].format;
    swapchain_create_info.imageColorSpace = info.surface_formats[0].colorSpace;
    swapchain_create_info.imageExtent = {info.surface_capabilities.minImageExtent.width,
                                         info.surface_capabilities.minImageExtent.height};
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = imageUsage;
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.preTransform = preTransform;
    swapchain_create_info.compositeAlpha = info.surface_composite_alpha;
    swapchain_create_info.presentMode = info.surface_non_shared_present_mode;
    swapchain_create_info.clipped = VK_FALSE;
    swapchain_create_info.oldSwapchain = oldSwapchain;

    VkResult result = vk::CreateSwapchainKHR(device(), &swapchain_create_info, nullptr, &swapchain);
    if (result != VK_SUCCESS) return false;
    // We must call vkGetSwapchainImagesKHR after creating the swapchain because the Validation Layer variables
    // for the swapchain image count are set inside that call. Otherwise, various validation fails due to
    // thinking that the swapchain image count is zero.
    GetSwapchainImages(swapchain);
    return true;
}

std::vector<VkImage> VkRenderFramework::GetSwapchainImages(const VkSwapchainKHR swapchain) {
    uint32_t imageCount = 0;
    vk::GetSwapchainImagesKHR(device(), swapchain, &imageCount, nullptr);
    vector<VkImage> swapchainImages;
    swapchainImages.resize(imageCount);
    vk::GetSwapchainImagesKHR(device(), swapchain, &imageCount, swapchainImages.data());
    return swapchainImages;
}

void VkRenderFramework::DestroySwapchain() {
    if (m_device && m_device->device() != VK_NULL_HANDLE) {
        vk::DeviceWaitIdle(device());
        if (m_swapchain != VK_NULL_HANDLE) {
            vk::DestroySwapchainKHR(device(), m_swapchain, nullptr);
            m_swapchain = VK_NULL_HANDLE;
        }
    }
}

void VkRenderFramework::InitRenderTarget() { InitRenderTarget(1); }

void VkRenderFramework::InitRenderTarget(uint32_t targets) { InitRenderTarget(targets, NULL); }

void VkRenderFramework::InitRenderTarget(VkImageView *dsBinding) { InitRenderTarget(1, dsBinding); }

void VkRenderFramework::InitRenderTarget(uint32_t targets, VkImageView *dsBinding) {
    vector<VkAttachmentDescription> &attachments = m_renderPass_attachments;
    vector<VkAttachmentReference> color_references;
    vector<VkImageView> &bindings = m_framebuffer_attachments;
    attachments.reserve(targets + 1);  // +1 for dsBinding
    color_references.reserve(targets);
    bindings.reserve(targets + 1);  // +1 for dsBinding

    VkAttachmentDescription att = {};
    att.format = m_render_target_fmt;
    att.samples = VK_SAMPLE_COUNT_1_BIT;
    att.loadOp = (m_clear_via_load_op) ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
    att.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    att.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    att.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    att.initialLayout = (m_clear_via_load_op) ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    att.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference ref = {};
    ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    m_renderPassClearValues.clear();
    VkClearValue clear = {};
    clear.color = m_clear_color;

    for (uint32_t i = 0; i < targets; i++) {
        attachments.push_back(att);

        ref.attachment = i;
        color_references.push_back(ref);

        m_renderPassClearValues.push_back(clear);

        std::unique_ptr<VkImageObj> img(new VkImageObj(m_device));

        VkFormatProperties props;

        vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), m_render_target_fmt, &props);

        if (props.linearTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT) {
            img->Init(m_width, m_height, 1, m_render_target_fmt,
                      VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                      VK_IMAGE_TILING_LINEAR);
        } else if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT) {
            img->Init(m_width, m_height, 1, m_render_target_fmt,
                      VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                      VK_IMAGE_TILING_OPTIMAL);
        } else {
            FAIL() << "Neither Linear nor Optimal allowed for render target";
        }

        bindings.push_back(img->targetView(m_render_target_fmt));
        m_renderTargets.push_back(std::move(img));
    }

    m_renderPass_subpasses.clear();
    m_renderPass_subpasses.resize(1);
    VkSubpassDescription &subpass = m_renderPass_subpasses[0];
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.flags = 0;
    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = NULL;
    subpass.colorAttachmentCount = targets;
    subpass.pColorAttachments = color_references.data();
    subpass.pResolveAttachments = NULL;

    VkAttachmentReference ds_reference;
    if (dsBinding) {
        att.format = m_depth_stencil_fmt;
        att.loadOp = (m_clear_via_load_op) ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
        att.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        att.stencilLoadOp = (m_clear_via_load_op) ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
        att.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
        att.initialLayout = m_depth_stencil_layout;
        att.finalLayout = m_depth_stencil_layout;
        attachments.push_back(att);

        clear.depthStencil.depth = m_depth_clear_color;
        clear.depthStencil.stencil = m_stencil_clear_color;
        m_renderPassClearValues.push_back(clear);

        bindings.push_back(*dsBinding);

        ds_reference.attachment = targets;
        ds_reference.layout = m_depth_stencil_layout;
        subpass.pDepthStencilAttachment = &ds_reference;
    } else {
        subpass.pDepthStencilAttachment = NULL;
    }

    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments = NULL;

    VkRenderPassCreateInfo &rp_info = m_renderPass_info;
    rp_info = vku::InitStructHelper();
    rp_info.attachmentCount = attachments.size();
    rp_info.pAttachments = attachments.data();
    rp_info.subpassCount = m_renderPass_subpasses.size();
    rp_info.pSubpasses = m_renderPass_subpasses.data();

    m_renderPass_dependencies.clear();
    if (m_addRenderPassSelfDependency) {
        m_renderPass_dependencies.resize(1);
        VkSubpassDependency &subpass_dep = m_renderPass_dependencies[0];
        // Add a subpass self-dependency to subpass 0 of default renderPass
        subpass_dep.srcSubpass = 0;
        subpass_dep.dstSubpass = 0;
        // Just using all framebuffer-space pipeline stages in order to get a reasonably large
        //  set of bits that can be used for both src & dst
        subpass_dep.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                                   VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpass_dep.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                                   VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        // Add all of the gfx mem access bits that correlate to the fb-space pipeline stages
        subpass_dep.srcAccessMask = VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_SHADER_READ_BIT |
                                    VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                                    VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        subpass_dep.dstAccessMask = VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_SHADER_READ_BIT |
                                    VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                                    VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        // Must include dep_by_region bit when src & dst both include framebuffer-space stages
        subpass_dep.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    }

    if (m_additionalSubpassDependencies.size()) {
        m_renderPass_dependencies.reserve(m_additionalSubpassDependencies.size() + m_renderPass_dependencies.size());
        m_renderPass_dependencies.insert(m_renderPass_dependencies.end(), m_additionalSubpassDependencies.begin(),
                                         m_additionalSubpassDependencies.end());
    }

    if (m_renderPass_dependencies.size()) {
        rp_info.dependencyCount = static_cast<uint32_t>(m_renderPass_dependencies.size());
        rp_info.pDependencies = m_renderPass_dependencies.data();
    } else {
        rp_info.dependencyCount = 0;
        rp_info.pDependencies = nullptr;
    }

    vk::CreateRenderPass(device(), &rp_info, NULL, &m_renderPass);
    // Create Framebuffer and RenderPass with color attachments and any
    // depth/stencil attachment
    VkFramebufferCreateInfo &fb_info = m_framebuffer_info;
    fb_info = vku::InitStructHelper();
    fb_info.renderPass = m_renderPass;
    fb_info.attachmentCount = bindings.size();
    fb_info.pAttachments = bindings.data();
    fb_info.width = m_width;
    fb_info.height = m_height;
    fb_info.layers = 1;

    vk::CreateFramebuffer(device(), &fb_info, NULL, &m_framebuffer);

    m_renderPassBeginInfo.renderPass = m_renderPass;
    m_renderPassBeginInfo.framebuffer = m_framebuffer;
    m_renderPassBeginInfo.renderArea.extent.width = m_width;
    m_renderPassBeginInfo.renderArea.extent.height = m_height;
    m_renderPassBeginInfo.clearValueCount = m_renderPassClearValues.size();
    m_renderPassBeginInfo.pClearValues = m_renderPassClearValues.data();
}

void VkRenderFramework::InitDynamicRenderTarget(VkFormat format) {
    if (format != VK_FORMAT_UNDEFINED) {
        m_render_target_fmt = format;
    }

    m_renderPassClearValues.clear();
    VkClearValue clear = {};
    clear.color = m_clear_color;

    std::unique_ptr<VkImageObj> img(new VkImageObj(m_device));

    VkFormatProperties props;

    vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), m_render_target_fmt, &props);

    if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT) {
        img->Init(m_width, m_height, 1, m_render_target_fmt,
                  VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                  VK_IMAGE_TILING_OPTIMAL);
    } else {
        FAIL() << "Optimal tiling not allowed for render target";
    }

    m_framebuffer_attachments.push_back(img->targetView(m_render_target_fmt));
    m_renderTargets.push_back(std::move(img));
}

VkImageView VkRenderFramework::GetDynamicRenderTarget() const {
    assert(m_framebuffer_attachments.size() == 1);
    return m_framebuffer_attachments[0];
}

void VkRenderFramework::DestroyRenderTarget() {
    vk::DestroyRenderPass(device(), m_renderPass, nullptr);
    m_renderPass = VK_NULL_HANDLE;
    vk::DestroyFramebuffer(device(), m_framebuffer, nullptr);
    m_framebuffer = VK_NULL_HANDLE;
}

VkDescriptorSetObj::VkDescriptorSetObj(vkt::Device *device) : m_device(device), m_nextSlot(0) {}

VkDescriptorSetObj::~VkDescriptorSetObj() noexcept {
    if (m_set) {
        delete m_set;
    }
}

int VkDescriptorSetObj::AppendDummy() {
    /* request a descriptor but do not update it */
    VkDescriptorSetLayoutBinding binding = {};
    binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    binding.descriptorCount = 1;
    binding.binding = m_layout_bindings.size();
    binding.stageFlags = VK_SHADER_STAGE_ALL;
    binding.pImmutableSamplers = NULL;

    m_layout_bindings.push_back(binding);
    m_type_counts[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER] += binding.descriptorCount;

    return m_nextSlot++;
}

int VkDescriptorSetObj::AppendSamplerTexture(VkDescriptorImageInfo &image_info) {
    VkDescriptorSetLayoutBinding binding = {};
    binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    binding.descriptorCount = 1;
    binding.binding = m_layout_bindings.size();
    binding.stageFlags = VK_SHADER_STAGE_ALL;
    binding.pImmutableSamplers = NULL;

    m_layout_bindings.push_back(binding);
    m_type_counts[VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER] += binding.descriptorCount;
    m_imageSamplerDescriptors.push_back(image_info);

    m_writes.push_back(vkt::Device::write_descriptor_set(vkt::DescriptorSet(), m_nextSlot, 0,
                                                         VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &image_info));

    return m_nextSlot++;
}

void VkDescriptorSetObj::CreateVKDescriptorSet(vkt::CommandBuffer *commandBuffer) {
    if (m_type_counts.size()) {
        // create VkDescriptorPool
        VkDescriptorPoolSize poolSize;
        vector<VkDescriptorPoolSize> sizes;
        for (auto it = m_type_counts.begin(); it != m_type_counts.end(); ++it) {
            poolSize.descriptorCount = it->second;
            poolSize.type = it->first;
            sizes.push_back(poolSize);
        }
        VkDescriptorPoolCreateInfo pool = vku::InitStructHelper();
        pool.poolSizeCount = sizes.size();
        pool.maxSets = 1;
        pool.pPoolSizes = sizes.data();
        init(*m_device, pool);
    }

    // create VkDescriptorSetLayout
    VkDescriptorSetLayoutCreateInfo layout = vku::InitStructHelper();
    layout.bindingCount = m_layout_bindings.size();
    layout.pBindings = m_layout_bindings.data();

    m_layout.init(*m_device, layout);
    vector<const vkt::DescriptorSetLayout *> layouts;
    layouts.push_back(&m_layout);

    // create VkPipelineLayout
    VkPipelineLayoutCreateInfo pipeline_layout = vku::InitStructHelper();
    pipeline_layout.setLayoutCount = layouts.size();
    pipeline_layout.pSetLayouts = NULL;

    m_pipeline_layout.init(*m_device, pipeline_layout, layouts);

    if (m_type_counts.size()) {
        // create VkDescriptorSet
        m_set = alloc_sets(*m_device, m_layout);

        // build the update array
        size_t imageSamplerCount = 0;
        for (vector<VkWriteDescriptorSet>::iterator it = m_writes.begin(); it != m_writes.end(); it++) {
            it->dstSet = m_set->handle();
            if (it->descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
                it->pImageInfo = &m_imageSamplerDescriptors[imageSamplerCount++];
        }

        // do the updates
        m_device->update_descriptor_sets(m_writes);
    }
}

VkImageObj::VkImageObj(vkt::Device *dev) {
    m_device = dev;
    m_descriptorImageInfo.imageView = VK_NULL_HANDLE;
    m_descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    m_arrayLayers = 0;
    m_mipLevels = 0;
}

// clang-format off
void VkImageObj::ImageMemoryBarrier(vkt::CommandBuffer *cmd_buf, VkImageAspectFlags aspect,
                                    VkFlags output_mask /*=
                                    VK_ACCESS_HOST_WRITE_BIT |
                                    VK_ACCESS_SHADER_WRITE_BIT |
                                    VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                                    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
                                    VK_MEMORY_OUTPUT_COPY_BIT*/,
                                    VkFlags input_mask /*=
                                    VK_ACCESS_HOST_READ_BIT |
                                    VK_ACCESS_INDIRECT_COMMAND_READ_BIT |
                                    VK_ACCESS_INDEX_READ_BIT |
                                    VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT |
                                    VK_ACCESS_UNIFORM_READ_BIT |
                                    VK_ACCESS_SHADER_READ_BIT |
                                    VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                                    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                    VK_MEMORY_INPUT_COPY_BIT*/, VkImageLayout image_layout,
                                    VkPipelineStageFlags src_stages, VkPipelineStageFlags dest_stages,
                                    uint32_t srcQueueFamilyIndex, uint32_t dstQueueFamilyIndex) {
    // clang-format on
    const VkImageSubresourceRange subresourceRange = subresource_range(aspect, 0, m_mipLevels, 0, m_arrayLayers);
    VkImageMemoryBarrier barrier;
    barrier = image_memory_barrier(output_mask, input_mask, Layout(), image_layout, subresourceRange, srcQueueFamilyIndex,
                                   dstQueueFamilyIndex);

    VkImageMemoryBarrier *pmemory_barrier = &barrier;

    // write barrier to the command buffer
    vk::CmdPipelineBarrier(cmd_buf->handle(), src_stages, dest_stages, VK_DEPENDENCY_BY_REGION_BIT, 0, NULL, 0, NULL, 1,
                           pmemory_barrier);
}

void VkImageObj::SetLayout(vkt::CommandBuffer *cmd_buf, VkImageAspectFlags aspect, VkImageLayout image_layout) {
    VkFlags src_mask, dst_mask;
    const VkFlags all_cache_outputs = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                                      VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
    const VkFlags all_cache_inputs = VK_ACCESS_HOST_READ_BIT | VK_ACCESS_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_INDEX_READ_BIT |
                                     VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT | VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_SHADER_READ_BIT |
                                     VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                     VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_MEMORY_READ_BIT;

    const VkFlags shader_read_inputs = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_MEMORY_READ_BIT;

    if (image_layout == m_descriptorImageInfo.imageLayout) {
        return;
    }

    // Attempt to narrow the src_mask, by what the image could have validly been used for in it's current layout
    switch (m_descriptorImageInfo.imageLayout) {
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            src_mask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            src_mask = shader_read_inputs;
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            src_mask = VK_ACCESS_TRANSFER_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            src_mask = VK_ACCESS_TRANSFER_READ_BIT;
            break;
        case VK_IMAGE_LAYOUT_UNDEFINED:
            src_mask = 0;
            break;
        default:
            src_mask = all_cache_outputs;  // Only need to worry about writes, as the stage mask will protect reads
    }

    // Narrow the dst mask by the valid accesss for the new layout
    switch (image_layout) {
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            // NOTE: not sure why shader read is here...
            dst_mask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_TRANSFER_READ_BIT;
            break;

        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            dst_mask = VK_ACCESS_TRANSFER_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            dst_mask = shader_read_inputs;
            break;

        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            dst_mask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            dst_mask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;

        default:
            // Must wait all read and write operations for the completion of the layout tranisition
            dst_mask = all_cache_inputs | all_cache_outputs;
            break;
    }

    ImageMemoryBarrier(cmd_buf, aspect, src_mask, dst_mask, image_layout);
    m_descriptorImageInfo.imageLayout = image_layout;
}

void VkImageObj::SetLayout(VkImageAspectFlags aspect, VkImageLayout image_layout) {
    if (image_layout == m_descriptorImageInfo.imageLayout) {
        return;
    }

    vkt::CommandPool pool(*m_device, m_device->graphics_queue_node_index_);
    vkt::CommandBuffer cmd_buf(m_device, &pool);

    /* Build command buffer to set image layout in the driver */
    cmd_buf.begin();
    SetLayout(&cmd_buf, aspect, image_layout);
    cmd_buf.end();

    cmd_buf.QueueCommandBuffer();
}

bool VkImageObj::IsCompatible(const VkImageUsageFlags usages, const VkFormatFeatureFlags2 features) {
    VkFormatFeatureFlags2 all_feature_flags =
        VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_2_STORAGE_IMAGE_BIT |
        VK_FORMAT_FEATURE_2_STORAGE_IMAGE_ATOMIC_BIT | VK_FORMAT_FEATURE_2_UNIFORM_TEXEL_BUFFER_BIT |
        VK_FORMAT_FEATURE_2_STORAGE_TEXEL_BUFFER_BIT | VK_FORMAT_FEATURE_2_STORAGE_TEXEL_BUFFER_ATOMIC_BIT |
        VK_FORMAT_FEATURE_2_VERTEX_BUFFER_BIT | VK_FORMAT_FEATURE_2_COLOR_ATTACHMENT_BIT |
        VK_FORMAT_FEATURE_2_COLOR_ATTACHMENT_BLEND_BIT | VK_FORMAT_FEATURE_2_DEPTH_STENCIL_ATTACHMENT_BIT |
        VK_FORMAT_FEATURE_2_BLIT_SRC_BIT | VK_FORMAT_FEATURE_2_BLIT_DST_BIT | VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_FILTER_LINEAR_BIT;
    if (m_device->IsEnabledExtension(VK_IMG_FILTER_CUBIC_EXTENSION_NAME)) {
        all_feature_flags |= VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_FILTER_CUBIC_BIT_EXT;
    }

    if (m_device->IsEnabledExtension(VK_KHR_MAINTENANCE_1_EXTENSION_NAME)) {
        all_feature_flags |= VK_FORMAT_FEATURE_2_TRANSFER_SRC_BIT_KHR | VK_FORMAT_FEATURE_2_TRANSFER_DST_BIT_KHR;
    }

    if (m_device->IsEnabledExtension(VK_EXT_SAMPLER_FILTER_MINMAX_EXTENSION_NAME)) {
        all_feature_flags |= VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_FILTER_MINMAX_BIT;
    }

    if (m_device->IsEnabledExtension(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME)) {
        all_feature_flags |= VK_FORMAT_FEATURE_2_MIDPOINT_CHROMA_SAMPLES_BIT_KHR |
                             VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT_KHR |
                             VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT_KHR |
                             VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_BIT_KHR |
                             VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_FORCEABLE_BIT_KHR |
                             VK_FORMAT_FEATURE_2_DISJOINT_BIT_KHR | VK_FORMAT_FEATURE_2_COSITED_CHROMA_SAMPLES_BIT_KHR;
    }

    if (m_device->IsEnabledExtension(VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME)) {
        all_feature_flags |= VK_FORMAT_FEATURE_2_STORAGE_READ_WITHOUT_FORMAT_BIT_KHR |
                             VK_FORMAT_FEATURE_2_STORAGE_WRITE_WITHOUT_FORMAT_BIT_KHR |
                             VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_DEPTH_COMPARISON_BIT_KHR;
    }

    if ((features & all_feature_flags) == 0) return false;  // whole format unsupported

    if ((usages & VK_IMAGE_USAGE_SAMPLED_BIT) && !(features & VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_BIT)) return false;
    if ((usages & VK_IMAGE_USAGE_STORAGE_BIT) && !(features & VK_FORMAT_FEATURE_2_STORAGE_IMAGE_BIT)) return false;
    if ((usages & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) && !(features & VK_FORMAT_FEATURE_2_COLOR_ATTACHMENT_BIT)) return false;
    if ((usages & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) && !(features & VK_FORMAT_FEATURE_2_DEPTH_STENCIL_ATTACHMENT_BIT))
        return false;

    return true;
}
VkImageCreateInfo VkImageObj::ImageCreateInfo2D(uint32_t const width, uint32_t const height, uint32_t const mipLevels,
                                                uint32_t const layers, VkFormat const format, VkFlags const usage,
                                                VkImageTiling const requested_tiling, const std::vector<uint32_t> *queue_families) {
    VkImageCreateInfo imageCreateInfo = vkt::Image::create_info();
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format = format;
    imageCreateInfo.extent.width = width;
    imageCreateInfo.extent.height = height;
    imageCreateInfo.mipLevels = mipLevels;
    imageCreateInfo.arrayLayers = layers;
    imageCreateInfo.tiling = requested_tiling;  // This will be touched up below...
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    // Automatically set sharing mode etc. based on queue family information
    if (queue_families && (queue_families->size() > 1)) {
        imageCreateInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
        imageCreateInfo.queueFamilyIndexCount = static_cast<uint32_t>(queue_families->size());
        imageCreateInfo.pQueueFamilyIndices = queue_families->data();
    }
    imageCreateInfo.usage = usage;
    return imageCreateInfo;
}
void VkImageObj::InitNoLayout(uint32_t const width, uint32_t const height, uint32_t const mipLevels, VkFormat const format,
                              VkFlags const usage, VkImageTiling const requested_tiling, VkMemoryPropertyFlags const reqs,
                              const vector<uint32_t> *queue_families, bool memory) {
    InitNoLayout(ImageCreateInfo2D(width, height, mipLevels, 1, format, usage, requested_tiling, queue_families), reqs, memory);
}

void VkImageObj::InitNoLayout(const VkImageCreateInfo &create_info, VkMemoryPropertyFlags const reqs, bool memory) {
    VkFormatFeatureFlags2 linear_tiling_features;
    VkFormatFeatureFlags2 optimal_tiling_features;
    // Touch up create info for tiling compatiblity...
    auto usage = create_info.usage;
    VkImageTiling requested_tiling = create_info.tiling;
    VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;

    if (m_device->IsEnabledExtension(VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME)) {
        VkFormatProperties3KHR fmt_props_3 = vku::InitStructHelper();
        VkFormatProperties2 fmt_props_2 = vku::InitStructHelper(&fmt_props_3);
        vk::GetPhysicalDeviceFormatProperties2(m_device->phy().handle(), create_info.format, &fmt_props_2);
        linear_tiling_features = fmt_props_3.linearTilingFeatures;
        optimal_tiling_features = fmt_props_3.optimalTilingFeatures;
    } else {
        VkFormatProperties format_properties;
        vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), create_info.format, &format_properties);
        linear_tiling_features = format_properties.linearTilingFeatures;
        optimal_tiling_features = format_properties.optimalTilingFeatures;
    }

    if ((create_info.flags & VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT) != 0) {
        tiling = requested_tiling;
    } else if (requested_tiling == VK_IMAGE_TILING_LINEAR) {
        if (IsCompatible(usage, linear_tiling_features)) {
            tiling = VK_IMAGE_TILING_LINEAR;
        } else if (IsCompatible(usage, optimal_tiling_features)) {
            tiling = VK_IMAGE_TILING_OPTIMAL;
        } else {
            FAIL() << "VkImageObj::init() error: unsupported tiling configuration. Usage: " << std::hex << std::showbase << usage
                   << ", supported linear features: " << linear_tiling_features;
        }
    } else if (IsCompatible(usage, optimal_tiling_features)) {
        tiling = VK_IMAGE_TILING_OPTIMAL;
    } else if (IsCompatible(usage, linear_tiling_features)) {
        tiling = VK_IMAGE_TILING_LINEAR;
    } else {
        FAIL() << "VkImageObj::init() error: unsupported tiling configuration. Usage: " << std::hex << std::showbase << usage
               << ", supported optimal features: " << optimal_tiling_features;
    }

    VkImageCreateInfo imageCreateInfo = create_info;
    imageCreateInfo.tiling = tiling;

    m_mipLevels = imageCreateInfo.mipLevels;
    m_arrayLayers = imageCreateInfo.arrayLayers;

    Layout(imageCreateInfo.initialLayout);
    if (memory)
        vkt::Image::init(*m_device, imageCreateInfo, reqs);
    else
        vkt::Image::init_no_mem(*m_device, imageCreateInfo);
}

void VkImageObj::Init(uint32_t const width, uint32_t const height, uint32_t const mipLevels, VkFormat const format,
                      VkFlags const usage, VkImageTiling const requested_tiling, VkMemoryPropertyFlags const reqs,
                      const vector<uint32_t> *queue_families, bool memory) {
    Init(ImageCreateInfo2D(width, height, mipLevels, 1, format, usage, requested_tiling, queue_families), reqs, memory);
}

void VkImageObj::Init(const VkImageCreateInfo &create_info, VkMemoryPropertyFlags const reqs, bool memory) {
    InitNoLayout(create_info, reqs, memory);

    if (!initialized() || !memory) return;  // We don't have a valid handle from early stage init, and thus SetLayout will fail

    VkImageLayout newLayout;
    const auto usage = create_info.usage;
    if (usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
        newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    else if (usage & VK_IMAGE_USAGE_SAMPLED_BIT)
        newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    else
        newLayout = m_descriptorImageInfo.imageLayout;

    VkImageAspectFlags image_aspect = aspect_mask(create_info.format);
    SetLayout(image_aspect, newLayout);
}

void VkImageObj::init(const VkImageCreateInfo *create_info) {
    VkFormatFeatureFlags2 linear_tiling_features;
    VkFormatFeatureFlags2 optimal_tiling_features;

    if (m_device->IsEnabledExtension(VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME)) {
        VkFormatProperties3KHR fmt_props_3 = vku::InitStructHelper();
        VkFormatProperties2 fmt_props_2 = vku::InitStructHelper(&fmt_props_3);
        vk::GetPhysicalDeviceFormatProperties2(m_device->phy().handle(), create_info->format, &fmt_props_2);
        linear_tiling_features = fmt_props_3.linearTilingFeatures;
        optimal_tiling_features = fmt_props_3.optimalTilingFeatures;
    } else {
        VkFormatProperties format_properties;
        vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), create_info->format, &format_properties);
        linear_tiling_features = format_properties.linearTilingFeatures;
        optimal_tiling_features = format_properties.optimalTilingFeatures;
    }

    const bool mutable_format = (create_info->flags & VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT) != 0;
    switch (create_info->tiling) {
        case VK_IMAGE_TILING_OPTIMAL:
            if (!mutable_format && !IsCompatible(create_info->usage, optimal_tiling_features)) {
                FAIL() << "VkImageObj::init() error: unsupported tiling configuration. Usage: " << std::hex << std::showbase
                       << create_info->usage << ", supported optimal features: " << optimal_tiling_features;
            }
            break;
        case VK_IMAGE_TILING_LINEAR:
            if (!mutable_format && !IsCompatible(create_info->usage, linear_tiling_features)) {
                FAIL() << "VkImageObj::init() error: unsupported tiling configuration. Usage: " << std::hex << std::showbase
                       << create_info->usage << ", supported linear features: " << linear_tiling_features;
            }
            break;
        default:
            break;
    }
    Layout(create_info->initialLayout);

    vkt::Image::init(*m_device, *create_info, 0);
    m_mipLevels = create_info->mipLevels;
    m_arrayLayers = create_info->arrayLayers;

    VkImageAspectFlags image_aspect = 0;
    if (vkuFormatIsDepthAndStencil(create_info->format)) {
        image_aspect = VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_DEPTH_BIT;
    } else if (vkuFormatIsDepthOnly(create_info->format)) {
        image_aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
    } else if (vkuFormatIsStencilOnly(create_info->format)) {
        image_aspect = VK_IMAGE_ASPECT_STENCIL_BIT;
    } else {  // color
        image_aspect = VK_IMAGE_ASPECT_COLOR_BIT;
    }
    SetLayout(image_aspect, VK_IMAGE_LAYOUT_GENERAL);
}

void VkImageObj::init_no_mem(const vkt::Device &dev, const VkImageCreateInfo &info) {
    vkt::Image::init_no_mem(dev, info);
    Layout(info.initialLayout);
    m_mipLevels = info.mipLevels;
    m_arrayLayers = info.arrayLayers;
}

VkResult VkImageObj::CopyImage(VkImageObj &src_image) {
    VkImageLayout src_image_layout, dest_image_layout;

    vkt::CommandPool pool(*m_device, m_device->graphics_queue_node_index_);
    vkt::CommandBuffer cmd_buf(m_device, &pool);

    /* Build command buffer to copy staging texture to usable texture */
    cmd_buf.begin();

    /* TODO: Can we determine image aspect from image object? */
    src_image_layout = src_image.Layout();
    src_image.SetLayout(&cmd_buf, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

    dest_image_layout = (this->Layout() == VK_IMAGE_LAYOUT_UNDEFINED) ? VK_IMAGE_LAYOUT_GENERAL : this->Layout();
    this->SetLayout(&cmd_buf, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    VkImageCopy copy_region = {};
    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.srcSubresource.baseArrayLayer = 0;
    copy_region.srcSubresource.mipLevel = 0;
    copy_region.srcSubresource.layerCount = 1;
    copy_region.srcOffset.x = 0;
    copy_region.srcOffset.y = 0;
    copy_region.srcOffset.z = 0;
    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.dstSubresource.baseArrayLayer = 0;
    copy_region.dstSubresource.mipLevel = 0;
    copy_region.dstSubresource.layerCount = 1;
    copy_region.dstOffset.x = 0;
    copy_region.dstOffset.y = 0;
    copy_region.dstOffset.z = 0;
    copy_region.extent = src_image.extent();

    vk::CmdCopyImage(cmd_buf.handle(), src_image.handle(), src_image.Layout(), handle(), Layout(), 1, &copy_region);

    src_image.SetLayout(&cmd_buf, VK_IMAGE_ASPECT_COLOR_BIT, src_image_layout);

    this->SetLayout(&cmd_buf, VK_IMAGE_ASPECT_COLOR_BIT, dest_image_layout);

    cmd_buf.end();

    cmd_buf.QueueCommandBuffer();

    return VK_SUCCESS;
}

// Same as CopyImage, but in the opposite direction
VkResult VkImageObj::CopyImageOut(VkImageObj &dst_image) {
    VkImageLayout src_image_layout, dest_image_layout;

    vkt::CommandPool pool(*m_device, m_device->graphics_queue_node_index_);
    vkt::CommandBuffer cmd_buf(m_device, &pool);

    cmd_buf.begin();

    src_image_layout = this->Layout();
    this->SetLayout(&cmd_buf, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

    dest_image_layout = (dst_image.Layout() == VK_IMAGE_LAYOUT_UNDEFINED) ? VK_IMAGE_LAYOUT_GENERAL : dst_image.Layout();
    dst_image.SetLayout(&cmd_buf, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    VkImageCopy copy_region = {};
    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.srcSubresource.baseArrayLayer = 0;
    copy_region.srcSubresource.mipLevel = 0;
    copy_region.srcSubresource.layerCount = 1;
    copy_region.srcOffset.x = 0;
    copy_region.srcOffset.y = 0;
    copy_region.srcOffset.z = 0;
    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.dstSubresource.baseArrayLayer = 0;
    copy_region.dstSubresource.mipLevel = 0;
    copy_region.dstSubresource.layerCount = 1;
    copy_region.dstOffset.x = 0;
    copy_region.dstOffset.y = 0;
    copy_region.dstOffset.z = 0;
    copy_region.extent = dst_image.extent();

    vk::CmdCopyImage(cmd_buf.handle(), handle(), Layout(), dst_image.handle(), dst_image.Layout(), 1, &copy_region);

    this->SetLayout(&cmd_buf, VK_IMAGE_ASPECT_COLOR_BIT, src_image_layout);

    dst_image.SetLayout(&cmd_buf, VK_IMAGE_ASPECT_COLOR_BIT, dest_image_layout);

    cmd_buf.end();

    cmd_buf.QueueCommandBuffer();

    return VK_SUCCESS;
}

// Return 16x16 pixel block
std::array<std::array<uint32_t, 16>, 16> VkImageObj::Read() {
    VkImageObj stagingImage(m_device);
    VkMemoryPropertyFlags reqs = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    stagingImage.Init(16, 16, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                      VK_IMAGE_TILING_LINEAR, reqs);
    stagingImage.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    VkSubresourceLayout layout = stagingImage.subresource_layout(subresource(VK_IMAGE_ASPECT_COLOR_BIT, 0, 0));
    CopyImageOut(stagingImage);
    void *data = stagingImage.MapMemory();
    std::array<std::array<uint32_t, 16>, 16> m = {};
    if (data) {
        for (uint32_t y = 0; y < stagingImage.extent().height; y++) {
            uint32_t *row = (uint32_t *)((char *)data + layout.rowPitch * y);
            for (uint32_t x = 0; x < stagingImage.extent().width; x++) m[y][x] = row[x];
        }
    }
    stagingImage.UnmapMemory();
    return m;
}

VkPipelineShaderStageCreateInfo const &VkShaderObj::GetStageCreateInfo() const { return m_stage_info; }

VkShaderObj::VkShaderObj(VkRenderFramework *framework, const char *source, VkShaderStageFlagBits stage, const spv_target_env env,
                         SpvSourceType source_type, const VkSpecializationInfo *spec_info, char const *entry_point, bool debug, const void *pNext)
    : m_framework(*framework), m_device(*(framework->DeviceObj())), m_source(source), m_spv_env(env) {
    m_stage_info = vku::InitStructHelper();
    m_stage_info.flags = 0;
    m_stage_info.stage = stage;
    m_stage_info.module = VK_NULL_HANDLE;
    m_stage_info.pName = entry_point;
    m_stage_info.pSpecializationInfo = spec_info;
    if (source_type == SPV_SOURCE_GLSL) {
        InitFromGLSL(debug, pNext);
    } else if (source_type == SPV_SOURCE_ASM) {
        InitFromASM();
    }
}

bool VkShaderObj::InitFromGLSL(bool debug, const void *pNext) {
    std::vector<uint32_t> spv;
    m_framework.GLSLtoSPV(&m_device.phy().limits_, m_stage_info.stage, m_source, spv, debug, m_spv_env);

    VkShaderModuleCreateInfo moduleCreateInfo = vku::InitStructHelper();
    moduleCreateInfo.pNext = pNext;
    moduleCreateInfo.codeSize = spv.size() * sizeof(uint32_t);
    moduleCreateInfo.pCode = spv.data();

    init(m_device, moduleCreateInfo);
    m_stage_info.module = handle();
    return VK_NULL_HANDLE != handle();
}

// Because shaders are currently validated at pipeline creation time, there are test cases that might fail shader module
// creation due to supplying an invalid/unknown SPIR-V capability/operation. This is called after VkShaderObj creation when
// tests are found to crash on a CI device
VkResult VkShaderObj::InitFromGLSLTry(bool debug, const vkt::Device *custom_device) {
    std::vector<uint32_t> spv;
    // 99% of tests just use the framework's VkDevice, but this allows for tests to use custom device object
    // Can't set at contructor time since all reference members need to be initialized then.
    VkPhysicalDeviceLimits limits = (custom_device) ? custom_device->phy().limits_ : m_device.phy().limits_;
    m_framework.GLSLtoSPV(&limits, m_stage_info.stage, m_source, spv, debug, m_spv_env);

    VkShaderModuleCreateInfo moduleCreateInfo = vku::InitStructHelper();
    moduleCreateInfo.codeSize = spv.size() * sizeof(uint32_t);
    moduleCreateInfo.pCode = spv.data();

    const auto result = init_try(((custom_device) ? *custom_device : m_device), moduleCreateInfo);
    m_stage_info.module = handle();
    return result;
}

bool VkShaderObj::InitFromASM() {
    vector<uint32_t> spv;
    m_framework.ASMtoSPV(m_spv_env, 0, m_source, spv);

    VkShaderModuleCreateInfo moduleCreateInfo = vku::InitStructHelper();
    moduleCreateInfo.codeSize = spv.size() * sizeof(uint32_t);
    moduleCreateInfo.pCode = spv.data();

    init(m_device, moduleCreateInfo);
    m_stage_info.module = handle();
    return VK_NULL_HANDLE != handle();
}

VkResult VkShaderObj::InitFromASMTry() {
    vector<uint32_t> spv;
    m_framework.ASMtoSPV(m_spv_env, 0, m_source, spv);

    VkShaderModuleCreateInfo moduleCreateInfo = vku::InitStructHelper();
    moduleCreateInfo.codeSize = spv.size() * sizeof(uint32_t);
    moduleCreateInfo.pCode = spv.data();

    const auto result = init_try(m_device, moduleCreateInfo);
    m_stage_info.module = handle();
    return result;
}

// static
std::unique_ptr<VkShaderObj> VkShaderObj::CreateFromGLSL(VkRenderFramework *framework, const char *source,
                                                         VkShaderStageFlagBits stage, const spv_target_env spv_env,
                                                         const VkSpecializationInfo *spec_info, const char *entry_point,
                                                         bool debug) {
    auto shader =
        std::make_unique<VkShaderObj>(framework, source, stage, spv_env, SPV_SOURCE_GLSL_TRY, spec_info, entry_point, debug);
    if (VK_SUCCESS == shader->InitFromGLSLTry(debug)) {
        return shader;
    }
    return {};
}

// static
std::unique_ptr<VkShaderObj> VkShaderObj::CreateFromASM(VkRenderFramework *framework, const char *source,
                                                        VkShaderStageFlagBits stage, const spv_target_env spv_env,
                                                        const VkSpecializationInfo *spec_info, const char *entry_point) {
    auto shader = std::make_unique<VkShaderObj>(framework, source, stage, spv_env, SPV_SOURCE_ASM_TRY, spec_info, entry_point);
    if (VK_SUCCESS == shader->InitFromASMTry()) {
        return shader;
    }
    return {};
}
