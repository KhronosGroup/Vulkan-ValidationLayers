/*
 * Copyright (c) 2015-2020 The Khronos Group Inc.
 * Copyright (c) 2015-2020 Valve Corporation
 * Copyright (c) 2015-2020 LunarG, Inc.
 * Copyright (c) 2015-2020 Google, Inc.
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
 *
 * Author: Courtney Goeltzenleuchter <courtney@LunarG.com>
 * Author: Tony Barbour <tony@LunarG.com>
 * Author: Dave Houlton <daveh@lunarg.com>
 */

#include "vkrenderframework.h"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <utility>
#include <vector>

#include "vk_format_utils.h"

using std::string;
using std::strncmp;
using std::vector;

template <typename C, typename F>
typename C::iterator RemoveIf(C &container, F &&fn) {
    return container.erase(std::remove_if(container.begin(), container.end(), std::forward<F>(fn)), container.end());
}

ErrorMonitor::ErrorMonitor() {
    test_platform_thread_create_mutex(&mutex_);
    MonitorReset();
}

ErrorMonitor::~ErrorMonitor() NOEXCEPT { test_platform_thread_delete_mutex(&mutex_); }

void ErrorMonitor::MonitorReset() {
    message_flags_ = 0;
    bailout_ = NULL;
    message_found_ = VK_FALSE;
    failure_message_strings_.clear();
    desired_message_strings_.clear();
    ignore_message_strings_.clear();
    allowed_message_strings_.clear();
    other_messages_.clear();
}

void ErrorMonitor::Reset() {
    test_platform_thread_lock_mutex(&mutex_);
    MonitorReset();
    test_platform_thread_unlock_mutex(&mutex_);
}

void ErrorMonitor::SetDesiredFailureMsg(const VkFlags msgFlags, const string msg) { SetDesiredFailureMsg(msgFlags, msg.c_str()); }

void ErrorMonitor::SetDesiredFailureMsg(const VkFlags msgFlags, const char *const msgString) {
    test_platform_thread_lock_mutex(&mutex_);
    desired_message_strings_.insert(msgString);
    message_flags_ |= msgFlags;
    test_platform_thread_unlock_mutex(&mutex_);
}

void ErrorMonitor::SetAllowedFailureMsg(const char *const msg) {
    test_platform_thread_lock_mutex(&mutex_);
    allowed_message_strings_.emplace_back(msg);
    test_platform_thread_unlock_mutex(&mutex_);
}

void ErrorMonitor::SetUnexpectedError(const char *const msg) {
    test_platform_thread_lock_mutex(&mutex_);
    ignore_message_strings_.emplace_back(msg);
    test_platform_thread_unlock_mutex(&mutex_);
}

VkBool32 ErrorMonitor::CheckForDesiredMsg(const char *const msgString) {
    VkBool32 result = VK_FALSE;
    test_platform_thread_lock_mutex(&mutex_);
    if (bailout_ != nullptr) {
        *bailout_ = true;
    }
    string errorString(msgString);
    bool found_expected = false;

    if (!IgnoreMessage(errorString)) {
        for (auto desired_msg_it = desired_message_strings_.begin(); desired_msg_it != desired_message_strings_.end();
             ++desired_msg_it) {
            if ((*desired_msg_it).length() == 0) {
                // An empty desired_msg string "" indicates a positive test - not expecting an error.
                // Return true to avoid calling layers/driver with this error.
                // And don't erase the "" string, so it remains if another error is found.
                result = VK_TRUE;
                found_expected = true;
                message_found_ = true;
                failure_message_strings_.insert(errorString);
            } else if (errorString.find(*desired_msg_it) != string::npos) {
                found_expected = true;
                failure_message_strings_.insert(errorString);
                message_found_ = true;
                result = VK_TRUE;
                // Remove a maximum of one failure message from the set
                // Multiset mutation is acceptable because `break` causes flow of control to exit the for loop
                desired_message_strings_.erase(desired_msg_it);
                break;
            }
        }

        if (!found_expected && allowed_message_strings_.size()) {
            for (auto allowed_msg_it = allowed_message_strings_.begin(); allowed_msg_it != allowed_message_strings_.end();
                 ++allowed_msg_it) {
                if (errorString.find(*allowed_msg_it) != string::npos) {
                    found_expected = true;
                    break;
                }
            }
        }

        if (!found_expected) {
            printf("Unexpected: %s\n", msgString);
            other_messages_.push_back(errorString);
        }
    }

    test_platform_thread_unlock_mutex(&mutex_);
    return result;
}

vector<string> ErrorMonitor::GetOtherFailureMsgs() const { return other_messages_; }

VkDebugReportFlagsEXT ErrorMonitor::GetMessageFlags() { return message_flags_; }

bool ErrorMonitor::AnyDesiredMsgFound() const { return message_found_; }

bool ErrorMonitor::AllDesiredMsgsFound() const { return desired_message_strings_.empty(); }

void ErrorMonitor::SetError(const char *const errorString) {
    test_platform_thread_lock_mutex(&mutex_);
    message_found_ = true;
    failure_message_strings_.insert(errorString);
    test_platform_thread_unlock_mutex(&mutex_);
}

void ErrorMonitor::SetBailout(bool *bailout) {
    test_platform_thread_lock_mutex(&mutex_);
    bailout_ = bailout;
    test_platform_thread_unlock_mutex(&mutex_);
}

void ErrorMonitor::DumpFailureMsgs() const {
    vector<string> otherMsgs = GetOtherFailureMsgs();
    if (otherMsgs.size()) {
        std::cout << "Other error messages logged for this test were:" << std::endl;
        for (auto iter = otherMsgs.begin(); iter != otherMsgs.end(); iter++) {
            std::cout << "     " << *iter << std::endl;
        }
    }
}

void ErrorMonitor::ExpectSuccess(VkDebugReportFlagsEXT const message_flag_mask) {
    // Match ANY message matching specified type
    test_platform_thread_lock_mutex(&mutex_);
    desired_message_strings_.insert("");
    message_flags_ = message_flag_mask;
    test_platform_thread_unlock_mutex(&mutex_);
}

void ErrorMonitor::VerifyFound() {
    test_platform_thread_lock_mutex(&mutex_);
    // Not receiving expected message(s) is a failure. /Before/ throwing, dump any other messages
    if (!AllDesiredMsgsFound()) {
        DumpFailureMsgs();
        for (const auto desired_msg : desired_message_strings_) {
            ADD_FAILURE() << "Did not receive expected error '" << desired_msg << "'";
        }
    } else if (GetOtherFailureMsgs().size() > 0) {
        // Fail test case for any unexpected errors
#if defined(ANDROID)
        // This will get unexpected errors into the adb log
        for (auto msg : other_messages_) {
            __android_log_print(ANDROID_LOG_INFO, "VulkanLayerValidationTests", "[ UNEXPECTED_ERR ] '%s'", msg.c_str());
        }
#else
        ADD_FAILURE() << "Received unexpected error(s).";
#endif
    }
    MonitorReset();
    test_platform_thread_unlock_mutex(&mutex_);
}

void ErrorMonitor::VerifyNotFound() {
    test_platform_thread_lock_mutex(&mutex_);
    // ExpectSuccess() configured us to match anything. Any error is a failure.
    if (AnyDesiredMsgFound()) {
        DumpFailureMsgs();
        for (const auto msg : failure_message_strings_) {
            ADD_FAILURE() << "Expected to succeed but got error: " << msg;
        }
    } else if (GetOtherFailureMsgs().size() > 0) {
        // Fail test case for any unexpected errors
#if defined(ANDROID)
        // This will get unexpected errors into the adb log
        for (auto msg : other_messages_) {
            __android_log_print(ANDROID_LOG_INFO, "VulkanLayerValidationTests", "[ UNEXPECTED_ERR ] '%s'", msg.c_str());
        }
#else
        ADD_FAILURE() << "Received unexpected error(s).";
#endif
    }
    MonitorReset();
    test_platform_thread_unlock_mutex(&mutex_);
}

bool ErrorMonitor::IgnoreMessage(string const &msg) const {
    if (ignore_message_strings_.empty()) {
        return false;
    }

    return std::find_if(ignore_message_strings_.begin(), ignore_message_strings_.end(),
                        [&msg](string const &str) { return msg.find(str) != string::npos; }) != ignore_message_strings_.end();
}

void DebugReporter::Create(VkInstance instance) NOEXCEPT {
    assert(instance);
    assert(!debug_obj_);

    auto DebugCreate = reinterpret_cast<DebugCreateFnType>(vk::GetInstanceProcAddr(instance, debug_create_fn_name_));
    if (!DebugCreate) return;

    const VkResult err = DebugCreate(instance, &debug_create_info_, nullptr, &debug_obj_);
    if (err) debug_obj_ = VK_NULL_HANDLE;
}

void DebugReporter::Destroy(VkInstance instance) NOEXCEPT {
    assert(instance);
    assert(debug_obj_);  // valid to call with null object, but probably bug

    auto DebugDestroy = reinterpret_cast<DebugDestroyFnType>(vk::GetInstanceProcAddr(instance, debug_destroy_fn_name_));
    assert(DebugDestroy);

    DebugDestroy(instance, debug_obj_, nullptr);
    debug_obj_ = VK_NULL_HANDLE;
}

#ifdef VK_USE_PLATFORM_ANDROID_KHR
VKAPI_ATTR VkBool32 VKAPI_CALL DebugReporter::DebugCallback(VkDebugReportFlagsEXT message_flags, VkDebugReportObjectTypeEXT,
                                                            uint64_t, size_t, int32_t, const char *, const char *message,
                                                            void *user_data) {
#else
VKAPI_ATTR VkBool32 VKAPI_CALL DebugReporter::DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                                                            VkDebugUtilsMessageTypeFlagsEXT message_types,
                                                            const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
                                                            void *user_data) {
    const auto message_flags = DebugAnnotFlagsToReportFlags(message_severity, message_types);
    const char *message = callback_data->pMessage;
#endif
    ErrorMonitor *errMonitor = (ErrorMonitor *)user_data;

    if (message_flags & errMonitor->GetMessageFlags()) {
        return errMonitor->CheckForDesiredMsg(message);
    }
    return VK_FALSE;
}

VkRenderFramework::VkRenderFramework()
    : instance_(NULL),
      m_device(NULL),
      m_commandPool(VK_NULL_HANDLE),
      m_commandBuffer(NULL),
      m_renderPass(VK_NULL_HANDLE),
      m_framebuffer(VK_NULL_HANDLE),
      m_surface(VK_NULL_HANDLE),
      m_swapchain(VK_NULL_HANDLE),
      m_addRenderPassSelfDependency(false),
      m_width(256.0),   // default window width
      m_height(256.0),  // default window height
      m_render_target_fmt(VK_FORMAT_R8G8B8A8_UNORM),
      m_depth_stencil_fmt(VK_FORMAT_UNDEFINED),
      m_clear_via_load_op(true),
      m_depth_clear_color(1.0),
      m_stencil_clear_color(0),
      m_depthStencil(NULL) {
    m_framebuffer_info = lvl_init_struct<VkFramebufferCreateInfo>();
    m_renderPass_info = lvl_init_struct<VkRenderPassCreateInfo>();
    m_renderPassBeginInfo = lvl_init_struct<VkRenderPassBeginInfo>();

    // clear the back buffer to dark grey
    m_clear_color.float32[0] = 0.25f;
    m_clear_color.float32[1] = 0.25f;
    m_clear_color.float32[2] = 0.25f;
    m_clear_color.float32[3] = 0.0f;
}

VkRenderFramework::~VkRenderFramework() { ShutdownFramework(); }

VkPhysicalDevice VkRenderFramework::gpu() {
    EXPECT_NE((VkInstance)0, instance_);  // Invalid to request gpu before instance exists
    return gpu_;
}

VkPhysicalDeviceProperties VkRenderFramework::physDevProps() {
    EXPECT_NE((VkPhysicalDevice)0, gpu_);  // Invalid to request physical device properties before gpu
    return physDevProps_;
}

// Return true if layer name is found and spec+implementation values are >= requested values
bool VkRenderFramework::InstanceLayerSupported(const char *const layer_name, const uint32_t spec_version,
                                               const uint32_t impl_version) {
    const auto layers = vk_testing::GetGlobalLayers();

    for (const auto &layer : layers) {
        if (0 == strncmp(layer_name, layer.layerName, VK_MAX_EXTENSION_NAME_SIZE)) {
            return layer.specVersion >= spec_version && layer.implementationVersion >= impl_version;
        }
    }
    return false;
}

// Return true if extension name is found and spec value is >= requested spec value
// WARNING: for simplicity, does not cover layers' extensions
bool VkRenderFramework::InstanceExtensionSupported(const char *const extension_name, const uint32_t spec_version) {
    // WARNING: assume debug extensions are always supported, which are usually provided by layers
    if (0 == strncmp(extension_name, VK_EXT_DEBUG_UTILS_EXTENSION_NAME, VK_MAX_EXTENSION_NAME_SIZE)) return true;
    if (0 == strncmp(extension_name, VK_EXT_DEBUG_REPORT_EXTENSION_NAME, VK_MAX_EXTENSION_NAME_SIZE)) return true;

    const auto extensions = vk_testing::GetGlobalExtensions();

    const auto IsTheQueriedExtension = [extension_name, spec_version](const VkExtensionProperties &extension) {
        return strncmp(extension_name, extension.extensionName, VK_MAX_EXTENSION_NAME_SIZE) == 0 &&
               extension.specVersion >= spec_version;
    };

    return std::any_of(extensions.begin(), extensions.end(), IsTheQueriedExtension);
}

// Enable device profile as last layer on stack overriding devsim if there, or return if not available
bool VkRenderFramework::EnableDeviceProfileLayer() {
    if (InstanceLayerSupported("VK_LAYER_LUNARG_device_profile_api")) {
        if (VkTestFramework::m_devsim_layer) {
            assert(0 == strncmp(instance_layers_.back(), "VK_LAYER_LUNARG_device_simulation", VK_MAX_EXTENSION_NAME_SIZE));
            instance_layers_.back() = "VK_LAYER_LUNARG_device_profile_api";
        } else {
            instance_layers_.push_back("VK_LAYER_LUNARG_device_profile_api");
        }
    } else {
        printf("             Did not find VK_LAYER_LUNARG_device_profile_api layer; skipped.\n");
        return false;
    }
    return true;
}

// Return true if instance exists and extension name is in the list
bool VkRenderFramework::InstanceExtensionEnabled(const char *ext_name) {
    if (!instance_) return false;

    return std::any_of(instance_extensions_.begin(), instance_extensions_.end(),
                       [ext_name](const char *e) { return 0 == strncmp(ext_name, e, VK_MAX_EXTENSION_NAME_SIZE); });
}
// Return true if extension name is found and spec value is >= requested spec value
bool VkRenderFramework::DeviceExtensionSupported(const char *extension_name, const uint32_t spec_version) const {
    if (!instance_ || !gpu_) {
        EXPECT_NE((VkInstance)0, instance_);  // Complain, not cool without an instance
        EXPECT_NE((VkPhysicalDevice)0, gpu_);
        return false;
    }

    const vk_testing::PhysicalDevice device_obj(gpu_);

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

// Return true if device is created and extension name is found in the list
bool VkRenderFramework::DeviceExtensionEnabled(const char *ext_name) {
    if (NULL == m_device) return false;

    bool ext_found = false;
    for (auto ext : m_device_extension_names) {
        if (!strncmp(ext, ext_name, VK_MAX_EXTENSION_NAME_SIZE)) {
            ext_found = true;
            break;
        }
    }
    return ext_found;
}

// Some tests may need to be skipped if the devsim layer is in use.
bool VkRenderFramework::DeviceSimulation() { return m_devsim_layer; }

VkInstanceCreateInfo VkRenderFramework::GetInstanceCreateInfo() const {
    return {
        VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        &debug_reporter_.debug_create_info_,
        0,
        &app_info_,
        static_cast<uint32_t>(instance_layers_.size()),
        instance_layers_.data(),
        static_cast<uint32_t>(instance_extensions_.size()),
        instance_extensions_.data(),
    };
}

void VkRenderFramework::InitFramework(void * /*unused compatibility parameter*/, void *instance_pnext) {
    ASSERT_EQ((VkInstance)0, instance_);

    const auto LayerNotSupportedWithReporting = [](const char *layer) {
        if (InstanceLayerSupported(layer))
            return false;
        else {
            ADD_FAILURE() << "InitFramework(): Requested layer \"" << layer << "\" is not supported. It will be disabled.";
            return true;
        }
    };
    const auto ExtensionNotSupportedWithReporting = [](const char *extension) {
        if (InstanceExtensionSupported(extension))
            return false;
        else {
            ADD_FAILURE() << "InitFramework(): Requested extension \"" << extension << "\" is not supported. It will be disabled.";
            return true;
        }
    };

    RemoveIf(instance_layers_, LayerNotSupportedWithReporting);
    RemoveIf(instance_extensions_, ExtensionNotSupportedWithReporting);

    auto ici = GetInstanceCreateInfo();

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

    ASSERT_VK_SUCCESS(vk::CreateInstance(&ici, nullptr, &instance_));
    if (instance_pnext) reinterpret_cast<VkBaseOutStructure *>(last_pnext)->pNext = nullptr;  // reset back borrowed pNext chain

    uint32_t gpu_count = 1;
    const VkResult err = vk::EnumeratePhysicalDevices(instance_, &gpu_count, &gpu_);
    ASSERT_TRUE(err == VK_SUCCESS || err == VK_INCOMPLETE) << vk_result_string(err);
    ASSERT_GT(gpu_count, (uint32_t)0) << "No GPU (i.e. VkPhysicalDevice) available";

    vk::GetPhysicalDeviceProperties(gpu_, &physDevProps_);
    debug_reporter_.Create(instance_);
}

void VkRenderFramework::ShutdownFramework() {
    debug_reporter_.error_monitor_.Reset();

    // Nothing to shut down without a VkInstance
    if (!instance_) return;

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

    // reset the driver
    delete m_device;
    m_device = nullptr;

    debug_reporter_.Destroy(instance_);

    vk::DestroyInstance(instance_, nullptr);
    instance_ = NULL;  // In case we want to re-initialize
}

ErrorMonitor &VkRenderFramework::Monitor() { return debug_reporter_.error_monitor_; }

void VkRenderFramework::GetPhysicalDeviceFeatures(VkPhysicalDeviceFeatures *features) {
    if (NULL == m_device) {
        VkDeviceObj *temp_device = new VkDeviceObj(0, gpu_, m_device_extension_names);
        *features = temp_device->phy().features();
        delete (temp_device);
    } else {
        *features = m_device->phy().features();
    }
}

bool VkRenderFramework::IsPlatform(PlatformType platform) {
    return (!vk_gpu_table.find(platform)->second.compare(physDevProps().deviceName));
}

void VkRenderFramework::GetPhysicalDeviceProperties(VkPhysicalDeviceProperties *props) { *props = physDevProps_; }

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

    m_device = new VkDeviceObj(0, gpu_, m_device_extension_names, features, create_device_pnext);
    m_device->SetDeviceQueue();

    m_depthStencil = new VkDepthStencilObj(m_device);

    m_render_target_fmt = VkTestFramework::GetFormat(instance_, m_device);

    m_lineWidth = 1.0f;

    m_depthBiasConstantFactor = 0.0f;
    m_depthBiasClamp = 0.0f;
    m_depthBiasSlopeFactor = 0.0f;

    m_blendConstants[0] = 1.0f;
    m_blendConstants[1] = 1.0f;
    m_blendConstants[2] = 1.0f;
    m_blendConstants[3] = 1.0f;

    m_minDepthBounds = 0.f;
    m_maxDepthBounds = 1.f;

    m_compareMask = 0xff;
    m_writeMask = 0xff;
    m_reference = 0;

    m_commandPool = new VkCommandPoolObj(m_device, m_device->graphics_queue_node_index_, flags);

    m_commandBuffer = new VkCommandBufferObj(m_device, m_commandPool);
}

void VkRenderFramework::InitViewport(float width, float height) {
    VkViewport viewport;
    VkRect2D scissor;
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = 1.f * width;
    viewport.height = 1.f * height;
    viewport.minDepth = 0.f;
    viewport.maxDepth = 1.f;
    m_viewports.push_back(viewport);

    scissor.extent.width = (int32_t)width;
    scissor.extent.height = (int32_t)height;
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    m_scissors.push_back(scissor);

    m_width = width;
    m_height = height;
}

void VkRenderFramework::InitViewport() { InitViewport(m_width, m_height); }

bool VkRenderFramework::InitSurface() { return InitSurface(m_width, m_height); }

#ifdef VK_USE_PLATFORM_WIN32_KHR
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

bool VkRenderFramework::InitSurface(float width, float height) {
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    HINSTANCE window_instance = GetModuleHandle(nullptr);
    const char class_name[] = "test";
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = window_instance;
    wc.lpszClassName = class_name;
    RegisterClass(&wc);
    HWND window = CreateWindowEx(0, class_name, 0, 0, 0, 0, (int)m_width, (int)m_height, NULL, NULL, window_instance, NULL);
    ShowWindow(window, SW_HIDE);

    VkWin32SurfaceCreateInfoKHR surface_create_info = {};
    surface_create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surface_create_info.hinstance = window_instance;
    surface_create_info.hwnd = window;
    VkResult err = vk::CreateWin32SurfaceKHR(instance(), &surface_create_info, nullptr, &m_surface);
    if (err != VK_SUCCESS) return false;
#endif

#if defined(VK_USE_PLATFORM_ANDROID_KHR) && defined(VALIDATION_APK)
    VkAndroidSurfaceCreateInfoKHR surface_create_info = {};
    surface_create_info.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
    surface_create_info.window = VkTestFramework::window;
    VkResult err = vk::CreateAndroidSurfaceKHR(instance(), &surface_create_info, nullptr, &m_surface);
    if (err != VK_SUCCESS) return false;
#endif

#if defined(VK_USE_PLATFORM_XLIB_KHR)
    Display *dpy = XOpenDisplay(NULL);
    if (dpy) {
        int s = DefaultScreen(dpy);
        Window window = XCreateSimpleWindow(dpy, RootWindow(dpy, s), 0, 0, (int)m_width, (int)m_height, 1, BlackPixel(dpy, s),
                                            WhitePixel(dpy, s));
        VkXlibSurfaceCreateInfoKHR surface_create_info = {};
        surface_create_info.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
        surface_create_info.dpy = dpy;
        surface_create_info.window = window;
        VkResult err = vk::CreateXlibSurfaceKHR(instance(), &surface_create_info, nullptr, &m_surface);
        if (err != VK_SUCCESS) return false;
    }
#endif

#if defined(VK_USE_PLATFORM_XCB_KHR)
    if (m_surface == VK_NULL_HANDLE) {
        xcb_connection_t *connection = xcb_connect(NULL, NULL);
        if (connection) {
            xcb_window_t window = xcb_generate_id(connection);
            VkXcbSurfaceCreateInfoKHR surface_create_info = {};
            surface_create_info.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
            surface_create_info.connection = connection;
            surface_create_info.window = window;
            VkResult err = vk::CreateXcbSurfaceKHR(instance(), &surface_create_info, nullptr, &m_surface);
            if (err != VK_SUCCESS) return false;
        }
    }
#endif

    return (m_surface == VK_NULL_HANDLE) ? false : true;
}

bool VkRenderFramework::InitSwapchain(VkImageUsageFlags imageUsage, VkSurfaceTransformFlagBitsKHR preTransform) {
    if (InitSurface()) {
        return InitSwapchain(m_surface, imageUsage, preTransform);
    }
    return false;
}

bool VkRenderFramework::InitSwapchain(VkSurfaceKHR &surface, VkImageUsageFlags imageUsage,
                                      VkSurfaceTransformFlagBitsKHR preTransform) {
    VkSurfaceCapabilitiesKHR capabilities;
    vk::GetPhysicalDeviceSurfaceCapabilitiesKHR(m_device->phy().handle(), surface, &capabilities);

    uint32_t format_count;
    vk::GetPhysicalDeviceSurfaceFormatsKHR(m_device->phy().handle(), surface, &format_count, nullptr);
    vector<VkSurfaceFormatKHR> formats;
    if (format_count != 0) {
        formats.resize(format_count);
        vk::GetPhysicalDeviceSurfaceFormatsKHR(m_device->phy().handle(), surface, &format_count, formats.data());
    }

    uint32_t present_mode_count;
    vk::GetPhysicalDeviceSurfacePresentModesKHR(m_device->phy().handle(), surface, &present_mode_count, nullptr);
    vector<VkPresentModeKHR> present_modes;
    if (present_mode_count != 0) {
        present_modes.resize(present_mode_count);
        vk::GetPhysicalDeviceSurfacePresentModesKHR(m_device->phy().handle(), surface, &present_mode_count, present_modes.data());
    }

    VkSwapchainCreateInfoKHR swapchain_create_info = {};
    swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_create_info.pNext = 0;
    swapchain_create_info.surface = surface;
    swapchain_create_info.minImageCount = capabilities.minImageCount;
    swapchain_create_info.imageFormat = formats[0].format;
    swapchain_create_info.imageColorSpace = formats[0].colorSpace;
    swapchain_create_info.imageExtent = {capabilities.minImageExtent.width, capabilities.minImageExtent.height};
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = imageUsage;
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.preTransform = preTransform;
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
#else
    swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
#endif
    swapchain_create_info.presentMode = present_modes[0];
    swapchain_create_info.clipped = VK_FALSE;
    swapchain_create_info.oldSwapchain = 0;

    VkResult err = vk::CreateSwapchainKHR(device(), &swapchain_create_info, nullptr, &m_swapchain);
    if (err != VK_SUCCESS) {
        return false;
    }
    uint32_t imageCount = 0;
    vk::GetSwapchainImagesKHR(device(), m_swapchain, &imageCount, nullptr);
    vector<VkImage> swapchainImages;
    swapchainImages.resize(imageCount);
    vk::GetSwapchainImagesKHR(device(), m_swapchain, &imageCount, swapchainImages.data());
    return true;
}

void VkRenderFramework::DestroySwapchain() {
    if (m_swapchain != VK_NULL_HANDLE) {
        vk::DestroySwapchainKHR(device(), m_swapchain, nullptr);
        m_swapchain = VK_NULL_HANDLE;
    }
    if (m_surface != VK_NULL_HANDLE) {
        vk::DestroySurfaceKHR(instance(), m_surface, nullptr);
        m_surface = VK_NULL_HANDLE;
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
    att.initialLayout = (m_clear_via_load_op) ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_GENERAL;
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
            img->Init((uint32_t)m_width, (uint32_t)m_height, 1, m_render_target_fmt,
                      VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                      VK_IMAGE_TILING_LINEAR);
        } else if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT) {
            img->Init((uint32_t)m_width, (uint32_t)m_height, 1, m_render_target_fmt,
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
        ;
        att.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        att.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        att.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
        att.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        att.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        attachments.push_back(att);

        clear.depthStencil.depth = m_depth_clear_color;
        clear.depthStencil.stencil = m_stencil_clear_color;
        m_renderPassClearValues.push_back(clear);

        bindings.push_back(*dsBinding);

        ds_reference.attachment = targets;
        ds_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        subpass.pDepthStencilAttachment = &ds_reference;
    } else {
        subpass.pDepthStencilAttachment = NULL;
    }

    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments = NULL;

    VkRenderPassCreateInfo &rp_info = m_renderPass_info;
    rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
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
        rp_info.dependencyCount = 1;
        rp_info.pDependencies = &subpass_dep;
    } else {
        rp_info.dependencyCount = 0;
        rp_info.pDependencies = nullptr;
    }

    vk::CreateRenderPass(device(), &rp_info, NULL, &m_renderPass);
    // Create Framebuffer and RenderPass with color attachments and any
    // depth/stencil attachment
    VkFramebufferCreateInfo &fb_info = m_framebuffer_info;
    fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fb_info.pNext = NULL;
    fb_info.renderPass = m_renderPass;
    fb_info.attachmentCount = bindings.size();
    fb_info.pAttachments = bindings.data();
    fb_info.width = (uint32_t)m_width;
    fb_info.height = (uint32_t)m_height;
    fb_info.layers = 1;

    vk::CreateFramebuffer(device(), &fb_info, NULL, &m_framebuffer);

    m_renderPassBeginInfo.renderPass = m_renderPass;
    m_renderPassBeginInfo.framebuffer = m_framebuffer;
    m_renderPassBeginInfo.renderArea.extent.width = (int32_t)m_width;
    m_renderPassBeginInfo.renderArea.extent.height = (int32_t)m_height;
    m_renderPassBeginInfo.clearValueCount = m_renderPassClearValues.size();
    m_renderPassBeginInfo.pClearValues = m_renderPassClearValues.data();
}

void VkRenderFramework::DestroyRenderTarget() {
    vk::DestroyRenderPass(device(), m_renderPass, nullptr);
    m_renderPass = VK_NULL_HANDLE;
    vk::DestroyFramebuffer(device(), m_framebuffer, nullptr);
    m_framebuffer = VK_NULL_HANDLE;
}

VkDeviceObj::VkDeviceObj(uint32_t id, VkPhysicalDevice obj) : vk_testing::Device(obj), id(id) {
    init();

    props = phy().properties();
    queue_props = phy().queue_properties();
}

VkDeviceObj::VkDeviceObj(uint32_t id, VkPhysicalDevice obj, vector<const char *> &extension_names,
                         VkPhysicalDeviceFeatures *features, void *create_device_pnext)
    : vk_testing::Device(obj), id(id) {
    init(extension_names, features, create_device_pnext);

    props = phy().properties();
    queue_props = phy().queue_properties();
}

uint32_t VkDeviceObj::QueueFamilyMatching(VkQueueFlags with, VkQueueFlags without, bool all_bits) {
    // Find a queue family with and without desired capabilities
    for (uint32_t i = 0; i < queue_props.size(); i++) {
        auto flags = queue_props[i].queueFlags;
        bool matches = all_bits ? (flags & with) == with : (flags & with) != 0;
        if (matches && ((flags & without) == 0) && (queue_props[i].queueCount > 0)) {
            return i;
        }
    }
    return UINT32_MAX;
}

void VkDeviceObj::SetDeviceQueue() {
    ASSERT_NE(true, graphics_queues().empty());
    m_queue = graphics_queues()[0]->handle();
}

VkQueueObj *VkDeviceObj::GetDefaultQueue() {
    if (graphics_queues().empty()) return nullptr;
    return graphics_queues()[0];
}

VkQueueObj *VkDeviceObj::GetDefaultComputeQueue() {
    if (compute_queues().empty()) return nullptr;
    return compute_queues()[0];
}

VkDescriptorSetLayoutObj::VkDescriptorSetLayoutObj(const VkDeviceObj *device,
                                                   const vector<VkDescriptorSetLayoutBinding> &descriptor_set_bindings,
                                                   VkDescriptorSetLayoutCreateFlags flags, void *pNext) {
    VkDescriptorSetLayoutCreateInfo dsl_ci = {};
    dsl_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    dsl_ci.pNext = pNext;
    dsl_ci.flags = flags;
    dsl_ci.bindingCount = static_cast<uint32_t>(descriptor_set_bindings.size());
    dsl_ci.pBindings = descriptor_set_bindings.data();

    init(*device, dsl_ci);
}

VkDescriptorSetObj::VkDescriptorSetObj(VkDeviceObj *device) : m_device(device), m_nextSlot(0) {}

VkDescriptorSetObj::~VkDescriptorSetObj() NOEXCEPT {
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

int VkDescriptorSetObj::AppendBuffer(VkDescriptorType type, VkConstantBufferObj &constantBuffer) {
    assert(type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER || type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC ||
           type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER || type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC);
    VkDescriptorSetLayoutBinding binding = {};
    binding.descriptorType = type;
    binding.descriptorCount = 1;
    binding.binding = m_layout_bindings.size();
    binding.stageFlags = VK_SHADER_STAGE_ALL;
    binding.pImmutableSamplers = NULL;

    m_layout_bindings.push_back(binding);
    m_type_counts[type] += binding.descriptorCount;

    m_writes.push_back(vk_testing::Device::write_descriptor_set(vk_testing::DescriptorSet(), m_nextSlot, 0, type, 1,
                                                                &constantBuffer.m_descriptorBufferInfo));

    return m_nextSlot++;
}

int VkDescriptorSetObj::AppendSamplerTexture(VkSamplerObj *sampler, VkTextureObj *texture) {
    VkDescriptorSetLayoutBinding binding = {};
    binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    binding.descriptorCount = 1;
    binding.binding = m_layout_bindings.size();
    binding.stageFlags = VK_SHADER_STAGE_ALL;
    binding.pImmutableSamplers = NULL;

    m_layout_bindings.push_back(binding);
    m_type_counts[VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER] += binding.descriptorCount;
    VkDescriptorImageInfo tmp = texture->DescriptorImageInfo();
    tmp.sampler = sampler->handle();
    m_imageSamplerDescriptors.push_back(tmp);

    m_writes.push_back(vk_testing::Device::write_descriptor_set(vk_testing::DescriptorSet(), m_nextSlot, 0,
                                                                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &tmp));

    return m_nextSlot++;
}

VkPipelineLayout VkDescriptorSetObj::GetPipelineLayout() const { return m_pipeline_layout.handle(); }

VkDescriptorSet VkDescriptorSetObj::GetDescriptorSetHandle() const {
    if (m_set)
        return m_set->handle();
    else
        return VK_NULL_HANDLE;
}

void VkDescriptorSetObj::CreateVKDescriptorSet(VkCommandBufferObj *commandBuffer) {
    if (m_type_counts.size()) {
        // create VkDescriptorPool
        VkDescriptorPoolSize poolSize;
        vector<VkDescriptorPoolSize> sizes;
        for (auto it = m_type_counts.begin(); it != m_type_counts.end(); ++it) {
            poolSize.descriptorCount = it->second;
            poolSize.type = it->first;
            sizes.push_back(poolSize);
        }
        VkDescriptorPoolCreateInfo pool = {};
        pool.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool.poolSizeCount = sizes.size();
        pool.maxSets = 1;
        pool.pPoolSizes = sizes.data();
        init(*m_device, pool);
    }

    // create VkDescriptorSetLayout
    VkDescriptorSetLayoutCreateInfo layout = {};
    layout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout.bindingCount = m_layout_bindings.size();
    layout.pBindings = m_layout_bindings.data();

    m_layout.init(*m_device, layout);
    vector<const vk_testing::DescriptorSetLayout *> layouts;
    layouts.push_back(&m_layout);

    // create VkPipelineLayout
    VkPipelineLayoutCreateInfo pipeline_layout = {};
    pipeline_layout.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
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

VkRenderpassObj::VkRenderpassObj(VkDeviceObj *dev, const VkFormat format) {
    // Create a renderPass with a single color attachment
    VkAttachmentReference attach = {};
    attach.layout = VK_IMAGE_LAYOUT_GENERAL;

    VkSubpassDescription subpass = {};
    subpass.pColorAttachments = &attach;
    subpass.colorAttachmentCount = 1;

    VkRenderPassCreateInfo rpci = {};
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;

    VkAttachmentDescription attach_desc = {};
    attach_desc.format = format;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    rpci.pAttachments = &attach_desc;
    rpci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

    device = dev->device();
    vk::CreateRenderPass(device, &rpci, NULL, &m_renderpass);
}

VkRenderpassObj::VkRenderpassObj(VkDeviceObj *dev, VkFormat format, bool depthStencil) {
    if (!depthStencil) {
        VkRenderpassObj(dev, format);
    } else {
        // Create a renderPass with a depth/stencil attachment
        VkAttachmentReference attach = {};
        attach.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = {};
        subpass.pDepthStencilAttachment = &attach;

        VkRenderPassCreateInfo rpci = {};
        rpci.subpassCount = 1;
        rpci.pSubpasses = &subpass;
        rpci.attachmentCount = 1;

        VkAttachmentDescription attach_desc = {};
        attach_desc.format = format;
        attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
        attach_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attach_desc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        rpci.pAttachments = &attach_desc;
        rpci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

        device = dev->device();
        vk::CreateRenderPass(device, &rpci, NULL, &m_renderpass);
    }
}

VkRenderpassObj::~VkRenderpassObj() NOEXCEPT { vk::DestroyRenderPass(device, m_renderpass, NULL); }

VkImageObj::VkImageObj(VkDeviceObj *dev) {
    m_device = dev;
    m_descriptorImageInfo.imageView = VK_NULL_HANDLE;
    m_descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    m_arrayLayers = 0;
    m_mipLevels = 0;
}

// clang-format off
void VkImageObj::ImageMemoryBarrier(VkCommandBufferObj *cmd_buf, VkImageAspectFlags aspect,
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

void VkImageObj::SetLayout(VkCommandBufferObj *cmd_buf, VkImageAspectFlags aspect, VkImageLayout image_layout) {
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

    VkCommandPoolObj pool(m_device, m_device->graphics_queue_node_index_);
    VkCommandBufferObj cmd_buf(m_device, &pool);

    /* Build command buffer to set image layout in the driver */
    cmd_buf.begin();
    SetLayout(&cmd_buf, aspect, image_layout);
    cmd_buf.end();

    cmd_buf.QueueCommandBuffer();
}

bool VkImageObj::IsCompatible(const VkImageUsageFlags usages, const VkFormatFeatureFlags features) {
    VkFormatFeatureFlags all_feature_flags =
        VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT | VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT |
        VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT | VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT |
        VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT | VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT |
        VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT |
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT |
        VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;
    if (m_device->IsEnabledExtension(VK_IMG_FILTER_CUBIC_EXTENSION_NAME)) {
        all_feature_flags |= VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_CUBIC_BIT_IMG;
    }

    if (m_device->IsEnabledExtension(VK_KHR_MAINTENANCE1_EXTENSION_NAME)) {
        all_feature_flags |= VK_FORMAT_FEATURE_TRANSFER_SRC_BIT_KHR | VK_FORMAT_FEATURE_TRANSFER_DST_BIT_KHR;
    }

    if (m_device->IsEnabledExtension(VK_EXT_SAMPLER_FILTER_MINMAX_EXTENSION_NAME)) {
        all_feature_flags |= VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_MINMAX_BIT_EXT;
    }

    if (m_device->IsEnabledExtension(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME)) {
        all_feature_flags |= VK_FORMAT_FEATURE_MIDPOINT_CHROMA_SAMPLES_BIT_KHR |
                             VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT_KHR |
                             VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT_KHR |
                             VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_BIT_KHR |
                             VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_FORCEABLE_BIT_KHR |
                             VK_FORMAT_FEATURE_DISJOINT_BIT_KHR | VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT_KHR;
    }

    if ((features & all_feature_flags) == 0) return false;  // whole format unsupported

    if ((usages & VK_IMAGE_USAGE_SAMPLED_BIT) && !(features & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)) return false;
    if ((usages & VK_IMAGE_USAGE_STORAGE_BIT) && !(features & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT)) return false;
    if ((usages & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) && !(features & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT)) return false;
    if ((usages & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) && !(features & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT))
        return false;

    if (m_device->IsEnabledExtension(VK_KHR_MAINTENANCE1_EXTENSION_NAME)) {
        // WORKAROUND: for DevSim not reporting extended enums, and possibly some drivers too
        const auto all_nontransfer_feature_flags =
            all_feature_flags ^ (VK_FORMAT_FEATURE_TRANSFER_SRC_BIT_KHR | VK_FORMAT_FEATURE_TRANSFER_DST_BIT_KHR);
        const bool transfer_probably_supported_anyway = (features & all_nontransfer_feature_flags) > 0;
        if (!transfer_probably_supported_anyway) {
            if ((usages & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) && !(features & VK_FORMAT_FEATURE_TRANSFER_SRC_BIT_KHR)) return false;
            if ((usages & VK_IMAGE_USAGE_TRANSFER_DST_BIT) && !(features & VK_FORMAT_FEATURE_TRANSFER_DST_BIT_KHR)) return false;
        }
    }

    return true;
}
VkImageCreateInfo VkImageObj::ImageCreateInfo2D(uint32_t const width, uint32_t const height, uint32_t const mipLevels,
                                                uint32_t const layers, VkFormat const format, VkFlags const usage,
                                                VkImageTiling const requested_tiling, const std::vector<uint32_t> *queue_families) {
    VkImageCreateInfo imageCreateInfo = vk_testing::Image::create_info();
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
    VkFormatProperties image_fmt;
    // Touch up create info for tiling compatiblity...
    auto usage = create_info.usage;
    VkImageTiling requested_tiling = create_info.tiling;
    VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;

    vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), create_info.format, &image_fmt);

    if (requested_tiling == VK_IMAGE_TILING_LINEAR) {
        if (IsCompatible(usage, image_fmt.linearTilingFeatures)) {
            tiling = VK_IMAGE_TILING_LINEAR;
        } else if (IsCompatible(usage, image_fmt.optimalTilingFeatures)) {
            tiling = VK_IMAGE_TILING_OPTIMAL;
        } else {
            FAIL() << "VkImageObj::init() error: unsupported tiling configuration. Usage: " << std::hex << std::showbase << usage
                   << ", supported linear features: " << image_fmt.linearTilingFeatures;
        }
    } else if (IsCompatible(usage, image_fmt.optimalTilingFeatures)) {
        tiling = VK_IMAGE_TILING_OPTIMAL;
    } else if (IsCompatible(usage, image_fmt.linearTilingFeatures)) {
        tiling = VK_IMAGE_TILING_LINEAR;
    } else {
        FAIL() << "VkImageObj::init() error: unsupported tiling configuration. Usage: " << std::hex << std::showbase << usage
               << ", supported optimal features: " << image_fmt.optimalTilingFeatures;
    }

    VkImageCreateInfo imageCreateInfo = create_info;
    imageCreateInfo.tiling = tiling;

    m_mipLevels = imageCreateInfo.mipLevels;
    m_arrayLayers = imageCreateInfo.arrayLayers;

    Layout(imageCreateInfo.initialLayout);
    if (memory)
        vk_testing::Image::init(*m_device, imageCreateInfo, reqs);
    else
        vk_testing::Image::init_no_mem(*m_device, imageCreateInfo);
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

    VkImageAspectFlags image_aspect = 0;
    const auto format = create_info.format;
    if (FormatIsDepthAndStencil(format)) {
        image_aspect = VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_DEPTH_BIT;
    } else if (FormatIsDepthOnly(format)) {
        image_aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
    } else if (FormatIsStencilOnly(format)) {
        image_aspect = VK_IMAGE_ASPECT_STENCIL_BIT;
    } else {  // color
        image_aspect = VK_IMAGE_ASPECT_COLOR_BIT;
    }
    SetLayout(image_aspect, newLayout);
}

void VkImageObj::init(const VkImageCreateInfo *create_info) {
    VkFormatProperties image_fmt;
    vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), create_info->format, &image_fmt);

    switch (create_info->tiling) {
        case VK_IMAGE_TILING_OPTIMAL:
            if (!IsCompatible(create_info->usage, image_fmt.optimalTilingFeatures)) {
                FAIL() << "VkImageObj::init() error: unsupported tiling configuration. Usage: " << std::hex << std::showbase
                       << create_info->usage << ", supported optimal features: " << image_fmt.optimalTilingFeatures;
            }
            break;
        case VK_IMAGE_TILING_LINEAR:
            if (!IsCompatible(create_info->usage, image_fmt.linearTilingFeatures)) {
                FAIL() << "VkImageObj::init() error: unsupported tiling configuration. Usage: " << std::hex << std::showbase
                       << create_info->usage << ", supported linear features: " << image_fmt.linearTilingFeatures;
            }
            break;
        default:
            break;
    }
    Layout(create_info->initialLayout);

    vk_testing::Image::init(*m_device, *create_info, 0);
    m_mipLevels = create_info->mipLevels;
    m_arrayLayers = create_info->arrayLayers;

    VkImageAspectFlags image_aspect = 0;
    if (FormatIsDepthAndStencil(create_info->format)) {
        image_aspect = VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_DEPTH_BIT;
    } else if (FormatIsDepthOnly(create_info->format)) {
        image_aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
    } else if (FormatIsStencilOnly(create_info->format)) {
        image_aspect = VK_IMAGE_ASPECT_STENCIL_BIT;
    } else {  // color
        image_aspect = VK_IMAGE_ASPECT_COLOR_BIT;
    }
    SetLayout(image_aspect, VK_IMAGE_LAYOUT_GENERAL);
}

VkResult VkImageObj::CopyImage(VkImageObj &src_image) {
    VkImageLayout src_image_layout, dest_image_layout;

    VkCommandPoolObj pool(m_device, m_device->graphics_queue_node_index_);
    VkCommandBufferObj cmd_buf(m_device, &pool);

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

    VkCommandPoolObj pool(m_device, m_device->graphics_queue_node_index_);
    VkCommandBufferObj cmd_buf(m_device, &pool);

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

VkTextureObj::VkTextureObj(VkDeviceObj *device, uint32_t *colors) : VkImageObj(device) {
    m_device = device;
    const VkFormat tex_format = VK_FORMAT_B8G8R8A8_UNORM;
    uint32_t tex_colors[2] = {0xffff0000, 0xff00ff00};
    void *data;
    uint32_t x, y;
    VkImageObj stagingImage(device);
    VkMemoryPropertyFlags reqs = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    stagingImage.Init(16, 16, 1, tex_format, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                      VK_IMAGE_TILING_LINEAR, reqs);
    VkSubresourceLayout layout = stagingImage.subresource_layout(subresource(VK_IMAGE_ASPECT_COLOR_BIT, 0, 0));

    if (colors == NULL) colors = tex_colors;

    VkImageViewCreateInfo view = {};
    view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view.pNext = NULL;
    view.image = VK_NULL_HANDLE;
    view.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view.format = tex_format;
    view.components.r = VK_COMPONENT_SWIZZLE_R;
    view.components.g = VK_COMPONENT_SWIZZLE_G;
    view.components.b = VK_COMPONENT_SWIZZLE_B;
    view.components.a = VK_COMPONENT_SWIZZLE_A;
    view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view.subresourceRange.baseMipLevel = 0;
    view.subresourceRange.levelCount = 1;
    view.subresourceRange.baseArrayLayer = 0;
    view.subresourceRange.layerCount = 1;

    /* create image */
    Init(16, 16, 1, tex_format, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_TILING_OPTIMAL);
    stagingImage.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);

    /* create image view */
    view.image = handle();
    m_textureView.init(*m_device, view);
    m_descriptorImageInfo.imageView = m_textureView.handle();

    data = stagingImage.MapMemory();

    for (y = 0; y < extent().height; y++) {
        uint32_t *row = (uint32_t *)((char *)data + layout.rowPitch * y);
        for (x = 0; x < extent().width; x++) row[x] = colors[(x & 1) ^ (y & 1)];
    }
    stagingImage.UnmapMemory();
    stagingImage.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    VkImageObj::CopyImage(stagingImage);
}

VkSamplerObj::VkSamplerObj(VkDeviceObj *device) {
    m_device = device;

    VkSamplerCreateInfo samplerCreateInfo;
    memset(&samplerCreateInfo, 0, sizeof(samplerCreateInfo));
    samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCreateInfo.magFilter = VK_FILTER_NEAREST;
    samplerCreateInfo.minFilter = VK_FILTER_NEAREST;
    samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.mipLodBias = 0.0;
    samplerCreateInfo.anisotropyEnable = VK_FALSE;
    samplerCreateInfo.maxAnisotropy = 1;
    samplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;
    samplerCreateInfo.minLod = 0.0;
    samplerCreateInfo.maxLod = 0.0;
    samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;

    init(*m_device, samplerCreateInfo);
}

/*
 * Basic ConstantBuffer constructor. Then use create methods to fill in the
 * details.
 */
VkConstantBufferObj::VkConstantBufferObj(VkDeviceObj *device, VkBufferUsageFlags usage) {
    m_device = device;

    memset(&m_descriptorBufferInfo, 0, sizeof(m_descriptorBufferInfo));

    // Special case for usages outside of original limits of framework
    if ((VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT) != usage) {
        init_no_mem(*m_device, create_info(0, usage));
    }
}

VkConstantBufferObj::VkConstantBufferObj(VkDeviceObj *device, VkDeviceSize allocationSize, const void *data,
                                         VkBufferUsageFlags usage) {
    m_device = device;

    memset(&m_descriptorBufferInfo, 0, sizeof(m_descriptorBufferInfo));

    VkMemoryPropertyFlags reqs = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    if ((VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT) == usage) {
        init_as_src_and_dst(*m_device, allocationSize, reqs);
    } else {
        init(*m_device, create_info(allocationSize, usage), reqs);
    }

    void *pData = memory().map();
    memcpy(pData, data, static_cast<size_t>(allocationSize));
    memory().unmap();

    /*
     * Constant buffers are going to be used as vertex input buffers
     * or as shader uniform buffers. So, we'll create the shaderbuffer
     * descriptor here so it's ready if needed.
     */
    this->m_descriptorBufferInfo.buffer = handle();
    this->m_descriptorBufferInfo.offset = 0;
    this->m_descriptorBufferInfo.range = allocationSize;
}

VkPipelineShaderStageCreateInfo const &VkShaderObj::GetStageCreateInfo() const { return m_stage_info; }

VkShaderObj::VkShaderObj(VkDeviceObj *device, const char *shader_code, VkShaderStageFlagBits stage, VkRenderFramework *framework,
                         char const *name, bool debug, VkSpecializationInfo *specInfo, uint32_t spirv_minor_version) {
    m_device = device;
    m_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    m_stage_info.pNext = nullptr;
    m_stage_info.flags = 0;
    m_stage_info.stage = stage;
    m_stage_info.module = VK_NULL_HANDLE;
    m_stage_info.pName = name;
    m_stage_info.pSpecializationInfo = specInfo;

    vector<unsigned int> spv;
    framework->GLSLtoSPV(&device->props.limits, stage, shader_code, spv, debug, spirv_minor_version);

    VkShaderModuleCreateInfo moduleCreateInfo = {};
    moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    moduleCreateInfo.codeSize = spv.size() * sizeof(unsigned int);
    moduleCreateInfo.pCode = spv.data();

    init(*m_device, moduleCreateInfo);
    m_stage_info.module = handle();
}

VkShaderObj::VkShaderObj(VkDeviceObj *device, const string spv_source, VkShaderStageFlagBits stage, VkRenderFramework *framework,
                         char const *name, VkSpecializationInfo *specInfo) {
    m_device = device;
    m_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    m_stage_info.pNext = nullptr;
    m_stage_info.flags = 0;
    m_stage_info.stage = stage;
    m_stage_info.module = VK_NULL_HANDLE;
    m_stage_info.pName = name;
    m_stage_info.pSpecializationInfo = specInfo;

    vector<unsigned int> spv;
    framework->ASMtoSPV(SPV_ENV_VULKAN_1_0, 0, spv_source.data(), spv);

    VkShaderModuleCreateInfo moduleCreateInfo = {};
    moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    moduleCreateInfo.codeSize = spv.size() * sizeof(unsigned int);
    moduleCreateInfo.pCode = spv.data();

    init(*m_device, moduleCreateInfo);
    m_stage_info.module = handle();
}

VkPipelineLayoutObj::VkPipelineLayoutObj(VkDeviceObj *device, const vector<const VkDescriptorSetLayoutObj *> &descriptor_layouts,
                                         const vector<VkPushConstantRange> &push_constant_ranges) {
    VkPipelineLayoutCreateInfo pl_ci = {};
    pl_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pl_ci.pushConstantRangeCount = static_cast<uint32_t>(push_constant_ranges.size());
    pl_ci.pPushConstantRanges = push_constant_ranges.data();

    auto descriptor_layouts_unwrapped = MakeTestbindingHandles<const vk_testing::DescriptorSetLayout>(descriptor_layouts);

    init(*device, pl_ci, descriptor_layouts_unwrapped);
}

void VkPipelineLayoutObj::Reset() { *this = VkPipelineLayoutObj(); }

VkPipelineObj::VkPipelineObj(VkDeviceObj *device) {
    m_device = device;

    m_vi_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    m_vi_state.pNext = nullptr;
    m_vi_state.flags = 0;
    m_vi_state.vertexBindingDescriptionCount = 0;
    m_vi_state.pVertexBindingDescriptions = nullptr;
    m_vi_state.vertexAttributeDescriptionCount = 0;
    m_vi_state.pVertexAttributeDescriptions = nullptr;

    m_ia_state.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    m_ia_state.pNext = nullptr;
    m_ia_state.flags = 0;
    m_ia_state.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    m_ia_state.primitiveRestartEnable = VK_FALSE;

    m_te_state = nullptr;

    m_vp_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    m_vp_state.pNext = VK_NULL_HANDLE;
    m_vp_state.flags = 0;
    m_vp_state.viewportCount = 1;
    m_vp_state.scissorCount = 1;
    m_vp_state.pViewports = nullptr;
    m_vp_state.pScissors = nullptr;

    m_rs_state.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    m_rs_state.pNext = &m_line_state;
    m_rs_state.flags = 0;
    m_rs_state.depthClampEnable = VK_FALSE;
    m_rs_state.rasterizerDiscardEnable = VK_FALSE;
    m_rs_state.polygonMode = VK_POLYGON_MODE_FILL;
    m_rs_state.cullMode = VK_CULL_MODE_BACK_BIT;
    m_rs_state.frontFace = VK_FRONT_FACE_CLOCKWISE;
    m_rs_state.depthBiasEnable = VK_FALSE;
    m_rs_state.depthBiasConstantFactor = 0.0f;
    m_rs_state.depthBiasClamp = 0.0f;
    m_rs_state.depthBiasSlopeFactor = 0.0f;
    m_rs_state.lineWidth = 1.0f;

    m_line_state.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_LINE_STATE_CREATE_INFO_EXT;
    m_line_state.pNext = nullptr;
    m_line_state.lineRasterizationMode = VK_LINE_RASTERIZATION_MODE_DEFAULT_EXT;
    m_line_state.stippledLineEnable = VK_FALSE;
    m_line_state.lineStippleFactor = 0;
    m_line_state.lineStipplePattern = 0;

    m_ms_state.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    m_ms_state.pNext = nullptr;
    m_ms_state.flags = 0;
    m_ms_state.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    m_ms_state.sampleShadingEnable = VK_FALSE;
    m_ms_state.minSampleShading = 0.0f;
    m_ms_state.pSampleMask = nullptr;
    m_ms_state.alphaToCoverageEnable = VK_FALSE;
    m_ms_state.alphaToOneEnable = VK_FALSE;

    m_ds_state = nullptr;

    memset(&m_cb_state, 0, sizeof(m_cb_state));
    m_cb_state.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    m_cb_state.blendConstants[0] = 1.0f;
    m_cb_state.blendConstants[1] = 1.0f;
    m_cb_state.blendConstants[2] = 1.0f;
    m_cb_state.blendConstants[3] = 1.0f;

    memset(&m_pd_state, 0, sizeof(m_pd_state));
}

void VkPipelineObj::AddShader(VkShaderObj *shader) { m_shaderStages.push_back(shader->GetStageCreateInfo()); }

void VkPipelineObj::AddShader(VkPipelineShaderStageCreateInfo const &createInfo) { m_shaderStages.push_back(createInfo); }

void VkPipelineObj::AddVertexInputAttribs(VkVertexInputAttributeDescription *vi_attrib, uint32_t count) {
    m_vi_state.pVertexAttributeDescriptions = vi_attrib;
    m_vi_state.vertexAttributeDescriptionCount = count;
}

void VkPipelineObj::AddVertexInputBindings(VkVertexInputBindingDescription *vi_binding, uint32_t count) {
    m_vi_state.pVertexBindingDescriptions = vi_binding;
    m_vi_state.vertexBindingDescriptionCount = count;
}

void VkPipelineObj::AddColorAttachment(uint32_t binding, const VkPipelineColorBlendAttachmentState &att) {
    if (binding + 1 > m_colorAttachments.size()) {
        m_colorAttachments.resize(binding + 1);
    }
    m_colorAttachments[binding] = att;
}

void VkPipelineObj::SetDepthStencil(const VkPipelineDepthStencilStateCreateInfo *ds_state) { m_ds_state = ds_state; }

void VkPipelineObj::SetViewport(const vector<VkViewport> viewports) {
    m_viewports = viewports;
    // If we explicitly set a null viewport, pass it through to create info
    // but preserve viewportCount because it musn't change
    if (m_viewports.size() == 0) {
        m_vp_state.pViewports = nullptr;
    }
}

void VkPipelineObj::SetScissor(const vector<VkRect2D> scissors) {
    m_scissors = scissors;
    // If we explicitly set a null scissor, pass it through to create info
    // but preserve scissorCount because it musn't change
    if (m_scissors.size() == 0) {
        m_vp_state.pScissors = nullptr;
    }
}

void VkPipelineObj::MakeDynamic(VkDynamicState state) {
    /* Only add a state once */
    for (auto it = m_dynamic_state_enables.begin(); it != m_dynamic_state_enables.end(); it++) {
        if ((*it) == state) return;
    }
    m_dynamic_state_enables.push_back(state);
}

void VkPipelineObj::SetMSAA(const VkPipelineMultisampleStateCreateInfo *ms_state) { m_ms_state = *ms_state; }

void VkPipelineObj::SetInputAssembly(const VkPipelineInputAssemblyStateCreateInfo *ia_state) { m_ia_state = *ia_state; }

void VkPipelineObj::SetRasterization(const VkPipelineRasterizationStateCreateInfo *rs_state) {
    m_rs_state = *rs_state;
    m_rs_state.pNext = &m_line_state;
}

void VkPipelineObj::SetTessellation(const VkPipelineTessellationStateCreateInfo *te_state) { m_te_state = te_state; }

void VkPipelineObj::SetLineState(const VkPipelineRasterizationLineStateCreateInfoEXT *line_state) { m_line_state = *line_state; }

void VkPipelineObj::InitGraphicsPipelineCreateInfo(VkGraphicsPipelineCreateInfo *gp_ci) {
    gp_ci->stageCount = m_shaderStages.size();
    gp_ci->pStages = m_shaderStages.size() ? m_shaderStages.data() : nullptr;

    m_vi_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    gp_ci->pVertexInputState = &m_vi_state;

    m_ia_state.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    gp_ci->pInputAssemblyState = &m_ia_state;

    gp_ci->sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    gp_ci->pNext = NULL;
    gp_ci->flags = 0;

    m_cb_state.attachmentCount = m_colorAttachments.size();
    m_cb_state.pAttachments = m_colorAttachments.data();

    if (m_viewports.size() > 0) {
        m_vp_state.viewportCount = m_viewports.size();
        m_vp_state.pViewports = m_viewports.data();
    } else {
        MakeDynamic(VK_DYNAMIC_STATE_VIEWPORT);
    }

    if (m_scissors.size() > 0) {
        m_vp_state.scissorCount = m_scissors.size();
        m_vp_state.pScissors = m_scissors.data();
    } else {
        MakeDynamic(VK_DYNAMIC_STATE_SCISSOR);
    }

    memset(&m_pd_state, 0, sizeof(m_pd_state));
    if (m_dynamic_state_enables.size() > 0) {
        m_pd_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        m_pd_state.dynamicStateCount = m_dynamic_state_enables.size();
        m_pd_state.pDynamicStates = m_dynamic_state_enables.data();
        gp_ci->pDynamicState = &m_pd_state;
    }

    gp_ci->subpass = 0;
    gp_ci->pViewportState = &m_vp_state;
    gp_ci->pRasterizationState = &m_rs_state;
    gp_ci->pMultisampleState = &m_ms_state;
    gp_ci->pDepthStencilState = m_ds_state;
    gp_ci->pColorBlendState = &m_cb_state;
    gp_ci->pTessellationState = m_te_state;
}

VkResult VkPipelineObj::CreateVKPipeline(VkPipelineLayout layout, VkRenderPass render_pass, VkGraphicsPipelineCreateInfo *gp_ci) {
    VkGraphicsPipelineCreateInfo info = {};

    // if not given a CreateInfo, create and initialize a local one.
    if (gp_ci == nullptr) {
        gp_ci = &info;
        InitGraphicsPipelineCreateInfo(gp_ci);
    }

    gp_ci->layout = layout;
    gp_ci->renderPass = render_pass;

    return init_try(*m_device, *gp_ci);
}

VkCommandBufferObj::VkCommandBufferObj(VkDeviceObj *device, VkCommandPoolObj *pool, VkCommandBufferLevel level, VkQueueObj *queue) {
    m_device = device;
    if (queue) {
        m_queue = queue;
    } else {
        m_queue = m_device->GetDefaultQueue();
    }
    assert(m_queue);

    auto create_info = vk_testing::CommandBuffer::create_info(pool->handle());
    create_info.level = level;
    init(*device, create_info);
}

void VkCommandBufferObj::PipelineBarrier(VkPipelineStageFlags src_stages, VkPipelineStageFlags dest_stages,
                                         VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount,
                                         const VkMemoryBarrier *pMemoryBarriers, uint32_t bufferMemoryBarrierCount,
                                         const VkBufferMemoryBarrier *pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount,
                                         const VkImageMemoryBarrier *pImageMemoryBarriers) {
    vk::CmdPipelineBarrier(handle(), src_stages, dest_stages, dependencyFlags, memoryBarrierCount, pMemoryBarriers,
                           bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
}

void VkCommandBufferObj::ClearAllBuffers(const vector<std::unique_ptr<VkImageObj>> &color_objs, VkClearColorValue clear_color,
                                         VkDepthStencilObj *depth_stencil_obj, float depth_clear_value,
                                         uint32_t stencil_clear_value) {
    // whatever we want to do, we do it to the whole buffer
    VkImageSubresourceRange subrange = {};
    // srRange.aspectMask to be set later
    subrange.baseMipLevel = 0;
    // TODO: Mali device crashing with VK_REMAINING_MIP_LEVELS
    subrange.levelCount = 1;  // VK_REMAINING_MIP_LEVELS;
    subrange.baseArrayLayer = 0;
    // TODO: Mesa crashing with VK_REMAINING_ARRAY_LAYERS
    subrange.layerCount = 1;  // VK_REMAINING_ARRAY_LAYERS;

    const VkImageLayout clear_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

    for (const auto &color_obj : color_objs) {
        subrange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        color_obj->Layout(VK_IMAGE_LAYOUT_UNDEFINED);
        color_obj->SetLayout(this, subrange.aspectMask, clear_layout);
        ClearColorImage(color_obj->image(), clear_layout, &clear_color, 1, &subrange);
    }

    if (depth_stencil_obj && depth_stencil_obj->Initialized()) {
        subrange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        if (FormatIsDepthOnly(depth_stencil_obj->format())) subrange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        if (FormatIsStencilOnly(depth_stencil_obj->format())) subrange.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;

        depth_stencil_obj->Layout(VK_IMAGE_LAYOUT_UNDEFINED);
        depth_stencil_obj->SetLayout(this, subrange.aspectMask, clear_layout);

        VkClearDepthStencilValue clear_value = {depth_clear_value, stencil_clear_value};
        ClearDepthStencilImage(depth_stencil_obj->handle(), clear_layout, &clear_value, 1, &subrange);
    }
}

void VkCommandBufferObj::FillBuffer(VkBuffer buffer, VkDeviceSize offset, VkDeviceSize fill_size, uint32_t data) {
    vk::CmdFillBuffer(handle(), buffer, offset, fill_size, data);
}

void VkCommandBufferObj::UpdateBuffer(VkBuffer buffer, VkDeviceSize dstOffset, VkDeviceSize dataSize, const void *pData) {
    vk::CmdUpdateBuffer(handle(), buffer, dstOffset, dataSize, pData);
}

void VkCommandBufferObj::CopyImage(VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout,
                                   uint32_t regionCount, const VkImageCopy *pRegions) {
    vk::CmdCopyImage(handle(), srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
}

void VkCommandBufferObj::ResolveImage(VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                      VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageResolve *pRegions) {
    vk::CmdResolveImage(handle(), srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
}

void VkCommandBufferObj::ClearColorImage(VkImage image, VkImageLayout imageLayout, const VkClearColorValue *pColor,
                                         uint32_t rangeCount, const VkImageSubresourceRange *pRanges) {
    vk::CmdClearColorImage(handle(), image, imageLayout, pColor, rangeCount, pRanges);
}

void VkCommandBufferObj::ClearDepthStencilImage(VkImage image, VkImageLayout imageLayout, const VkClearDepthStencilValue *pColor,
                                                uint32_t rangeCount, const VkImageSubresourceRange *pRanges) {
    vk::CmdClearDepthStencilImage(handle(), image, imageLayout, pColor, rangeCount, pRanges);
}

void VkCommandBufferObj::BuildAccelerationStructure(VkAccelerationStructureObj *as, VkBuffer scratchBuffer) {
    BuildAccelerationStructure(as, scratchBuffer, VK_NULL_HANDLE);
}

void VkCommandBufferObj::BuildAccelerationStructure(VkAccelerationStructureObj *as, VkBuffer scratchBuffer, VkBuffer instanceData) {
    PFN_vkCmdBuildAccelerationStructureNV vkCmdBuildAccelerationStructureNV =
        (PFN_vkCmdBuildAccelerationStructureNV)vk::GetDeviceProcAddr(as->dev(), "vkCmdBuildAccelerationStructureNV");
    assert(vkCmdBuildAccelerationStructureNV != nullptr);

    vkCmdBuildAccelerationStructureNV(handle(), &as->info(), instanceData, 0, VK_FALSE, as->handle(), VK_NULL_HANDLE, scratchBuffer,
                                      0);
}

void VkCommandBufferObj::PrepareAttachments(const vector<std::unique_ptr<VkImageObj>> &color_atts,
                                            VkDepthStencilObj *depth_stencil_att) {
    for (const auto &color_att : color_atts) {
        color_att->SetLayout(this, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    }

    if (depth_stencil_att && depth_stencil_att->Initialized()) {
        VkImageAspectFlags aspect = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        if (FormatIsDepthOnly(depth_stencil_att->Format())) aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
        if (FormatIsStencilOnly(depth_stencil_att->Format())) aspect = VK_IMAGE_ASPECT_STENCIL_BIT;

        depth_stencil_att->SetLayout(this, aspect, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    }
}

void VkCommandBufferObj::BeginRenderPass(const VkRenderPassBeginInfo &info) {
    vk::CmdBeginRenderPass(handle(), &info, VK_SUBPASS_CONTENTS_INLINE);
}

void VkCommandBufferObj::EndRenderPass() { vk::CmdEndRenderPass(handle()); }

void VkCommandBufferObj::SetViewport(uint32_t firstViewport, uint32_t viewportCount, const VkViewport *pViewports) {
    vk::CmdSetViewport(handle(), firstViewport, viewportCount, pViewports);
}

void VkCommandBufferObj::SetStencilReference(VkStencilFaceFlags faceMask, uint32_t reference) {
    vk::CmdSetStencilReference(handle(), faceMask, reference);
}

void VkCommandBufferObj::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset,
                                     uint32_t firstInstance) {
    vk::CmdDrawIndexed(handle(), indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void VkCommandBufferObj::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) {
    vk::CmdDraw(handle(), vertexCount, instanceCount, firstVertex, firstInstance);
}

void VkCommandBufferObj::QueueCommandBuffer(bool checkSuccess) {
    VkFenceObj nullFence;
    QueueCommandBuffer(nullFence, checkSuccess);
}

void VkCommandBufferObj::QueueCommandBuffer(const VkFenceObj &fence, bool checkSuccess) {
    VkResult err = VK_SUCCESS;

    err = m_queue->submit(*this, fence, checkSuccess);
    if (checkSuccess) {
        ASSERT_VK_SUCCESS(err);
    }

    err = m_queue->wait();
    if (checkSuccess) {
        ASSERT_VK_SUCCESS(err);
    }

    // TODO: Determine if we really want this serialization here
    // Wait for work to finish before cleaning up.
    vk::DeviceWaitIdle(m_device->device());
}

void VkCommandBufferObj::BindDescriptorSet(VkDescriptorSetObj &descriptorSet) {
    VkDescriptorSet set_obj = descriptorSet.GetDescriptorSetHandle();

    // bind pipeline, vertex buffer (descriptor set) and WVP (dynamic buffer view)
    if (set_obj) {
        vk::CmdBindDescriptorSets(handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, descriptorSet.GetPipelineLayout(), 0, 1, &set_obj, 0,
                                  NULL);
    }
}

void VkCommandBufferObj::BindIndexBuffer(VkBufferObj *indexBuffer, VkDeviceSize offset, VkIndexType indexType) {
    vk::CmdBindIndexBuffer(handle(), indexBuffer->handle(), offset, indexType);
}

void VkCommandBufferObj::BindVertexBuffer(VkConstantBufferObj *vertexBuffer, VkDeviceSize offset, uint32_t binding) {
    vk::CmdBindVertexBuffers(handle(), binding, 1, &vertexBuffer->handle(), &offset);
}

VkCommandPoolObj::VkCommandPoolObj(VkDeviceObj *device, uint32_t queue_family_index, VkCommandPoolCreateFlags flags) {
    init(*device, vk_testing::CommandPool::create_info(queue_family_index, flags));
}

bool VkDepthStencilObj::Initialized() { return m_initialized; }
VkDepthStencilObj::VkDepthStencilObj(VkDeviceObj *device) : VkImageObj(device) { m_initialized = false; }

VkImageView *VkDepthStencilObj::BindInfo() { return &m_attachmentBindInfo; }

VkFormat VkDepthStencilObj::Format() const { return this->m_depth_stencil_fmt; }

void VkDepthStencilObj::Init(VkDeviceObj *device, int32_t width, int32_t height, VkFormat format, VkImageUsageFlags usage) {
    VkImageViewCreateInfo view_info = {};

    m_device = device;
    m_initialized = true;
    m_depth_stencil_fmt = format;

    /* create image */
    VkImageObj::Init(width, height, 1, m_depth_stencil_fmt, usage, VK_IMAGE_TILING_OPTIMAL);

    VkImageAspectFlags aspect = VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_DEPTH_BIT;
    if (FormatIsDepthOnly(format))
        aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
    else if (FormatIsStencilOnly(format))
        aspect = VK_IMAGE_ASPECT_STENCIL_BIT;

    SetLayout(aspect, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.pNext = NULL;
    view_info.image = VK_NULL_HANDLE;
    view_info.subresourceRange.aspectMask = aspect;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;
    view_info.flags = 0;
    view_info.format = m_depth_stencil_fmt;
    view_info.image = handle();
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    m_imageView.init(*m_device, view_info);

    m_attachmentBindInfo = m_imageView.handle();
}
