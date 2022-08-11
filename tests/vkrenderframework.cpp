/*
 * Copyright (c) 2015-2022 The Khronos Group Inc.
 * Copyright (c) 2015-2022 Valve Corporation
 * Copyright (c) 2015-2022 LunarG, Inc.
 * Copyright (c) 2015-2022 Google, Inc.
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
#include "vk_extension_helper.h"

using std::string;
using std::strncmp;
using std::vector;

template <typename C, typename F>
typename C::iterator RemoveIf(C &container, F &&fn) {
    return container.erase(std::remove_if(container.begin(), container.end(), std::forward<F>(fn)), container.end());
}

ErrorMonitor::ErrorMonitor() {
    MonitorReset();
    ExpectSuccess(kErrorBit);
}

ErrorMonitor::~ErrorMonitor() NOEXCEPT {}

void ErrorMonitor::MonitorReset() {
    message_flags_ = kErrorBit;
    bailout_ = NULL;
    message_found_ = VK_FALSE;
    failure_message_strings_.clear();
    desired_message_strings_.clear();
    ignore_message_strings_.clear();
    allowed_message_strings_.clear();
    other_messages_.clear();
}

void ErrorMonitor::Reset() {
    auto guard = Lock();
    MonitorReset();
}

void ErrorMonitor::SetDesiredFailureMsg(const VkFlags msgFlags, const string msg) { SetDesiredFailureMsg(msgFlags, msg.c_str()); }

void ErrorMonitor::SetDesiredFailureMsg(const VkFlags msgFlags, const char *const msgString) {
    if (NeedCheckSuccess()) {
        VerifyNotFound();
    }

    auto guard = Lock();
    desired_message_strings_.insert(msgString);
    message_flags_ |= msgFlags;
}

void ErrorMonitor::SetAllowedFailureMsg(const char *const msg) {
    auto guard = Lock();
    allowed_message_strings_.emplace_back(msg);
}

void ErrorMonitor::SetUnexpectedError(const char *const msg) {
    if (NeedCheckSuccess()) {
        VerifyNotFound();
    }
    auto guard = Lock();
    ignore_message_strings_.emplace_back(msg);
}

VkBool32 ErrorMonitor::CheckForDesiredMsg(const char *const msgString) {
    VkBool32 result = VK_FALSE;
    auto guard = Lock();
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
    return result;
}

vector<string> ErrorMonitor::GetOtherFailureMsgs() const { return other_messages_; }

VkDebugReportFlagsEXT ErrorMonitor::GetMessageFlags() { return message_flags_; }

bool ErrorMonitor::AnyDesiredMsgFound() const { return message_found_; }

bool ErrorMonitor::AllDesiredMsgsFound() const { return desired_message_strings_.empty(); }

void ErrorMonitor::SetError(const char *const errorString) {
    auto guard = Lock();
    message_found_ = true;
    failure_message_strings_.insert(errorString);
}

void ErrorMonitor::SetBailout(std::atomic<bool> *bailout) {
    auto guard = Lock();
    bailout_ = bailout;
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
    auto guard = Lock();
    desired_message_strings_.insert("");
    message_flags_ = message_flag_mask;
}

void ErrorMonitor::VerifyFound() {
    {
        // the lock must be released before the ExpectSuccess call at the end
        auto guard = Lock();
        // Not receiving expected message(s) is a failure. /Before/ throwing, dump any other messages
        if (!AllDesiredMsgsFound()) {
            DumpFailureMsgs();
            for (const auto &desired_msg : desired_message_strings_) {
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
    }

    ExpectSuccess();
}

void ErrorMonitor::VerifyNotFound() {
    auto guard = Lock();
    // ExpectSuccess() configured us to match anything. Any error is a failure.
    if (AnyDesiredMsgFound()) {
        DumpFailureMsgs();
        for (const auto &msg : failure_message_strings_) {
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
#if defined(VK_USE_PLATFORM_XLIB_KHR)
      m_surface_dpy(nullptr),
      m_surface_window(None),
#endif
#if defined(VK_USE_PLATFORM_XCB_KHR)
      m_surface_xcb_conn(nullptr),
#endif
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
    m_framebuffer_info = LvlInitStruct<VkFramebufferCreateInfo>();
    m_renderPass_info = LvlInitStruct<VkRenderPassCreateInfo>();
    m_renderPassBeginInfo = LvlInitStruct<VkRenderPassBeginInfo>();

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

    if (available_layers_.empty()) {
        available_layers_ = vk_testing::GetGlobalLayers();
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
        available_extensions_ = vk_testing::GetGlobalExtensions();
    }

    const auto IsTheQueriedExtension = [extension_name, spec_version](const VkExtensionProperties &extension) {
        return strncmp(extension_name, extension.extensionName, VK_MAX_EXTENSION_NAME_SIZE) == 0 &&
               extension.specVersion >= spec_version;
    };

    return std::any_of(available_extensions_.begin(), available_extensions_.end(), IsTheQueriedExtension);
}

// Enable device profile as last layer on stack overriding devsim if there, or return if not available
bool VkRenderFramework::OverrideDevsimForDeviceProfileLayer() {
    if (VkTestFramework::m_devsim_layer) {
        if (InstanceLayerSupported("VK_LAYER_LUNARG_device_profile_api")) {
            assert(0 == strncmp(instance_layers_.back(), "VK_LAYER_LUNARG_device_simulation", VK_MAX_EXTENSION_NAME_SIZE));
            instance_layers_.back() = "VK_LAYER_LUNARG_device_profile_api";
        } else {
            printf(
                "             Did not find VK_LAYER_LUNARG_device_profile_api layer; make sure VK_LAYER_PATH is set correctly to "
                "where the validation layers are built, the device profile layer should be in the same directory.\n");
            return false;
        }
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
#ifdef VK_USE_PLATFORM_METAL_EXT
    return {
        VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        &debug_reporter_.debug_create_info_,
        VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR,
        &app_info_,
        static_cast<uint32_t>(instance_layers_.size()),
        instance_layers_.data(),
        static_cast<uint32_t>(instance_extensions_.size()),
        instance_extensions_.data(),
    };
#else
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
#endif
}

inline void CheckDisableCoreValidation(VkValidationFeaturesEXT &features) {
    auto disable = GetEnvironment("VK_LAYER_TESTS_DISABLE_CORE_VALIDATION");
    std::transform(disable.begin(), disable.end(), disable.begin(), ::tolower);
    if (disable == "false" || disable == "0" || disable == "FALSE") {       // default is to change nothing, unless flag is correctly specified
        features.disabledValidationFeatureCount = 0;  // remove all disables to get all validation messages
    }
}

void VkRenderFramework::InitFramework(void * /*unused compatibility parameter*/, void *instance_pnext) {
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
        instance_extensions_.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }

#ifdef VK_USE_PLATFORM_METAL_EXT
    instance_extensions_.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#endif

    RemoveIf(instance_layers_, LayerNotSupportedWithReporting);
    RemoveIf(instance_extensions_, ExtensionNotSupportedWithReporting);

    auto ici = GetInstanceCreateInfo();

    // If is validation features then check for disabled validation

    auto bos = reinterpret_cast<VkBaseOutStructure *>(instance_pnext);
    if (instance_pnext && bos->sType == VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT) {
        auto features = reinterpret_cast<VkValidationFeaturesEXT *>(instance_pnext);
        CheckDisableCoreValidation(*features);
    }

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

    // Choose a physical device
    uint32_t gpu_count = 0;
    const VkResult err = vk::EnumeratePhysicalDevices(instance_, &gpu_count, nullptr);
    ASSERT_TRUE(err == VK_SUCCESS || err == VK_INCOMPLETE) << vk_result_string(err);
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

    debug_reporter_.Create(instance_);

    if (print_driver_info && !driver_printed) {
        auto driver_properties = LvlInitStruct<VkPhysicalDeviceDriverProperties>();
        auto physical_device_properties2 = LvlInitStruct<VkPhysicalDeviceProperties2>(&driver_properties);
        vk::GetPhysicalDeviceProperties2(gpu_, &physical_device_properties2);
        printf("Driver Name = %s\n", driver_properties.driverName);
        printf("Driver Info = %s\n", driver_properties.driverInfo);

        driver_printed = true;
    }

    for (const auto &ext : m_required_extensions) {
        AddRequestedDeviceExtensions(ext);
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

bool VkRenderFramework::IsExtensionsEnabled(const char *ext_name) const {
    return (CanEnableDeviceExtension(ext_name) || CanEnableInstanceExtension(ext_name));
}

bool VkRenderFramework::AreRequiredExtensionsEnabled() const {
    for (const auto &ext : m_required_extensions) {
        // `ext` may refer to an instance or device extension
        if (!CanEnableDeviceExtension(ext) && !CanEnableInstanceExtension(ext)) {
            return false;
        }
    }
    return true;
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

    debug_reporter_.Destroy(instance_);

    if (m_surface != VK_NULL_HANDLE) {
        vk::DestroySurfaceKHR(instance_, m_surface, nullptr);
        m_surface = VK_NULL_HANDLE;
    }

    vk::DestroyInstance(instance_, nullptr);
    instance_ = NULL;  // In case we want to re-initialize

    debug_reporter_.error_monitor_.Finish();
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

// static
bool VkRenderFramework::IgnoreDisableChecks() {
    static const bool skip_disable_checks = GetEnvironment("VK_LAYER_TESTS_IGNORE_DISABLE_CHECKS") != "";
    return skip_disable_checks;
}

bool VkRenderFramework::IsPlatform(PlatformType platform) {
    if (VkRenderFramework::IgnoreDisableChecks()) {
        return false;
    } else {
        return (!vk_gpu_table.find(platform)->second.compare(physDevProps().deviceName));
    }
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

bool VkRenderFramework::InitSurface() { return InitSurface(m_width, m_height, m_surface); }

bool VkRenderFramework::InitSurface(float width, float height) { return InitSurface(width, height, m_surface); }

#ifdef VK_USE_PLATFORM_WIN32_KHR
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

bool VkRenderFramework::InitSurface(float width, float height, VkSurfaceKHR &surface) {
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

    VkWin32SurfaceCreateInfoKHR surface_create_info = LvlInitStruct<VkWin32SurfaceCreateInfoKHR>();
    surface_create_info.hinstance = window_instance;
    surface_create_info.hwnd = window;
    VkResult err = vk::CreateWin32SurfaceKHR(instance(), &surface_create_info, nullptr, &surface);
    if (err != VK_SUCCESS) return false;
#endif

#if defined(VK_USE_PLATFORM_ANDROID_KHR) && defined(VALIDATION_APK)
    VkAndroidSurfaceCreateInfoKHR surface_create_info = LvlInitStruct<VkAndroidSurfaceCreateInfoKHR>();
    surface_create_info.window = VkTestFramework::window;
    VkResult err = vk::CreateAndroidSurfaceKHR(instance(), &surface_create_info, nullptr, &m_surface);
    if (err != VK_SUCCESS) return false;
#endif

#if defined(VK_USE_PLATFORM_XLIB_KHR)
    assert(m_surface_dpy == nullptr);
    m_surface_dpy = XOpenDisplay(NULL);
    if (m_surface_dpy) {
        int s = DefaultScreen(m_surface_dpy);
        m_surface_window = XCreateSimpleWindow(m_surface_dpy, RootWindow(m_surface_dpy, s), 0, 0, (int)m_width, (int)m_height, 1,
                                               BlackPixel(m_surface_dpy, s), WhitePixel(m_surface_dpy, s));
        VkXlibSurfaceCreateInfoKHR surface_create_info = LvlInitStruct<VkXlibSurfaceCreateInfoKHR>();
        surface_create_info.dpy = m_surface_dpy;
        surface_create_info.window = m_surface_window;
        VkResult err = vk::CreateXlibSurfaceKHR(instance(), &surface_create_info, nullptr, &m_surface);
        if (err != VK_SUCCESS) return false;
    }
#endif

#if defined(VK_USE_PLATFORM_XCB_KHR)
    if (m_surface == VK_NULL_HANDLE) {
        assert(m_surface_xcb_conn == nullptr);
        m_surface_xcb_conn = xcb_connect(NULL, NULL);
        if (m_surface_xcb_conn) {
            xcb_window_t window = xcb_generate_id(m_surface_xcb_conn);
            VkXcbSurfaceCreateInfoKHR surface_create_info = LvlInitStruct<VkXcbSurfaceCreateInfoKHR>();
            surface_create_info.connection = m_surface_xcb_conn;
            surface_create_info.window = window;
            VkResult err = vk::CreateXcbSurfaceKHR(instance(), &surface_create_info, nullptr, &m_surface);
            if (err != VK_SUCCESS) return false;
        }
    }
#endif
    return (m_surface == VK_NULL_HANDLE) ? false : true;
}

// Makes query to get information about swapchain needed to create a valid swapchain object each test creating a swapchain will need
void VkRenderFramework::InitSwapchainInfo() {
    const VkPhysicalDevice physicalDevice = gpu();

    vk::GetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, m_surface, &m_surface_capabilities);

    uint32_t format_count;
    vk::GetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_surface, &format_count, nullptr);
    if (format_count != 0) {
        m_surface_formats.resize(format_count);
        vk::GetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_surface, &format_count, m_surface_formats.data());
    }

    uint32_t present_mode_count;
    vk::GetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_surface, &present_mode_count, nullptr);
    if (present_mode_count != 0) {
        m_surface_present_modes.resize(present_mode_count);
        vk::GetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_surface, &present_mode_count, m_surface_present_modes.data());

        // Shared Present mode has different requirements most tests won't actually want
        // Implementation required to support a non-shared present mode
        for (size_t i = 0; i < m_surface_present_modes.size(); i++) {
            const VkPresentModeKHR present_mode = m_surface_present_modes[i];
            if ((present_mode != VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR) &&
                (present_mode != VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR)) {
                m_surface_non_shared_present_mode = present_mode;
                break;
            }
        }
    }

#ifdef VK_USE_PLATFORM_ANDROID_KHR
    m_surface_composite_alpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
#else
    m_surface_composite_alpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
#endif
}

bool VkRenderFramework::InitSwapchain(VkImageUsageFlags imageUsage, VkSurfaceTransformFlagBitsKHR preTransform) {
    if (InitSurface()) {
        return InitSwapchain(m_surface, imageUsage, preTransform);
    }
    return false;
}

bool VkRenderFramework::InitSwapchain(VkSurfaceKHR &surface, VkImageUsageFlags imageUsage,
                                      VkSurfaceTransformFlagBitsKHR preTransform) {
    return InitSwapchain(surface, imageUsage, preTransform, m_swapchain);
}

bool VkRenderFramework::InitSwapchain(VkSurfaceKHR &surface, VkImageUsageFlags imageUsage,
                                      VkSurfaceTransformFlagBitsKHR preTransform, VkSwapchainKHR &swapchain,
                                      VkSwapchainKHR oldSwapchain) {

    VkBool32 supported;
    vk::GetPhysicalDeviceSurfaceSupportKHR(gpu(), m_device->graphics_queue_node_index_, surface, &supported);
    if (!supported) {
        // Graphics queue does not support present
        return false;
    }
    InitSwapchainInfo();

    VkSwapchainCreateInfoKHR swapchain_create_info = LvlInitStruct<VkSwapchainCreateInfoKHR>();
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
    swapchain_create_info.oldSwapchain = oldSwapchain;

    VkResult err = vk::CreateSwapchainKHR(device(), &swapchain_create_info, nullptr, &swapchain);
    if (err != VK_SUCCESS) {
        return false;
    }
    uint32_t imageCount = 0;
    vk::GetSwapchainImagesKHR(device(), swapchain, &imageCount, nullptr);
    vector<VkImage> swapchainImages;
    swapchainImages.resize(imageCount);
    vk::GetSwapchainImagesKHR(device(), swapchain, &imageCount, swapchainImages.data());
    return true;
}

#if defined(VK_USE_PLATFORM_XLIB_KHR)
int IgnoreXErrors(Display *, XErrorEvent *) { return 0; }
#endif

void VkRenderFramework::DestroySwapchain() {
    if (m_device && m_device->device() != VK_NULL_HANDLE) {
        vk::DeviceWaitIdle(device());
        if (m_swapchain != VK_NULL_HANDLE) {
            vk::DestroySwapchainKHR(device(), m_swapchain, nullptr);
            m_swapchain = VK_NULL_HANDLE;
        }
    }

#if defined(VK_USE_PLATFORM_XLIB_KHR)
    if (m_surface_dpy != nullptr) {
        // Ignore BadDrawable errors we seem to get during shutdown.
        // The default error handler will exit() and end the test suite.
        XSetErrorHandler(IgnoreXErrors);
        XDestroyWindow(m_surface_dpy, m_surface_window);
        m_surface_window = None;
        XCloseDisplay(m_surface_dpy);
        m_surface_dpy = nullptr;
        XSetErrorHandler(nullptr);
    }
#endif
#if defined(VK_USE_PLATFORM_XCB_KHR)
    if (m_surface_xcb_conn != nullptr) {
        xcb_disconnect(m_surface_xcb_conn);
        m_surface_xcb_conn = nullptr;
    }
#endif

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
        att.stencilLoadOp = (m_clear_via_load_op) ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
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
    rp_info = LvlInitStruct<VkRenderPassCreateInfo>();
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
    fb_info = LvlInitStruct<VkFramebufferCreateInfo>();
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

bool VkRenderFramework::InitFrameworkAndRetrieveFeatures(VkPhysicalDeviceFeatures2KHR &features2) {
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }
    else {
        printf("Instance extension %s not supported, skipping test\n",
            VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return false;
    }
    InitFramework();

    // Cycle through device extensions and check for support
    for (auto extension : m_device_extension_names) {
        if (!DeviceExtensionSupported(extension)) {
            printf("Device extension %s is not supported\n", extension);
            return false;
        }
    }
    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(),
            "vkGetPhysicalDeviceFeatures2KHR");

    if (vkGetPhysicalDeviceFeatures2KHR) {
        vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);
        return true;
    }
    else {
        printf("Cannot use vkGetPhysicalDeviceFeatures to determine available features\n");
        return false;
    }
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
    m_queue_obj = graphics_queues()[0];
    m_queue = m_queue_obj->handle();
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
    VkDescriptorSetLayoutCreateInfo dsl_ci = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>(pNext);
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

VkDescriptorSetLayout VkDescriptorSetObj::GetDescriptorSetLayout() const { return m_layout.handle(); }

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
        VkDescriptorPoolCreateInfo pool = LvlInitStruct<VkDescriptorPoolCreateInfo>();
        pool.poolSizeCount = sizes.size();
        pool.maxSets = 1;
        pool.pPoolSizes = sizes.data();
        init(*m_device, pool);
    }

    // create VkDescriptorSetLayout
    VkDescriptorSetLayoutCreateInfo layout = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>();
    layout.bindingCount = m_layout_bindings.size();
    layout.pBindings = m_layout_bindings.data();

    m_layout.init(*m_device, layout);
    vector<const vk_testing::DescriptorSetLayout *> layouts;
    layouts.push_back(&m_layout);

    // create VkPipelineLayout
    VkPipelineLayoutCreateInfo pipeline_layout = LvlInitStruct<VkPipelineLayoutCreateInfo>();
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

    VkRenderPassCreateInfo rpci = LvlInitStruct<VkRenderPassCreateInfo>();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;

    VkAttachmentDescription attach_desc = {};
    attach_desc.format = format;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    attach_desc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;

    rpci.pAttachments = &attach_desc;

    init(*dev, rpci);
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

        VkRenderPassCreateInfo rpci = LvlInitStruct<VkRenderPassCreateInfo>();
        rpci.subpassCount = 1;
        rpci.pSubpasses = &subpass;
        rpci.attachmentCount = 1;

        VkAttachmentDescription attach_desc = {};
        attach_desc.format = format;
        attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
        attach_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attach_desc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        rpci.pAttachments = &attach_desc;

        init(*dev, rpci);
    }
}

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

    if (m_device->IsEnabledExtension(VK_KHR_MAINTENANCE_1_EXTENSION_NAME)) {
        // WORKAROUND: for DevSim not reporting extended enums, and possibly some drivers too
        const auto all_nontransfer_feature_flags =
            all_feature_flags ^ (VK_FORMAT_FEATURE_2_TRANSFER_SRC_BIT_KHR | VK_FORMAT_FEATURE_2_TRANSFER_DST_BIT_KHR);
        const bool transfer_probably_supported_anyway = (features & all_nontransfer_feature_flags) > 0;
        if (!transfer_probably_supported_anyway) {
            if ((usages & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) && !(features & VK_FORMAT_FEATURE_2_TRANSFER_SRC_BIT_KHR)) return false;
            if ((usages & VK_IMAGE_USAGE_TRANSFER_DST_BIT) && !(features & VK_FORMAT_FEATURE_2_TRANSFER_DST_BIT_KHR)) return false;
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
    VkFormatFeatureFlags2 linear_tiling_features;
    VkFormatFeatureFlags2 optimal_tiling_features;
    // Touch up create info for tiling compatiblity...
    auto usage = create_info.usage;
    VkImageTiling requested_tiling = create_info.tiling;
    VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;

    if (m_device->IsEnabledExtension(VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME)) {
        auto fmt_props_3 = LvlInitStruct<VkFormatProperties3KHR>();
        auto fmt_props_2 = LvlInitStruct<VkFormatProperties2>(&fmt_props_3);
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

    VkImageAspectFlags image_aspect = aspect_mask(create_info.format);
    SetLayout(image_aspect, newLayout);
}

void VkImageObj::init(const VkImageCreateInfo *create_info) {
    VkFormatFeatureFlags2 linear_tiling_features;
    VkFormatFeatureFlags2 optimal_tiling_features;

    if (m_device->IsEnabledExtension(VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME)) {
        auto fmt_props_3 = LvlInitStruct<VkFormatProperties3KHR>();
        auto fmt_props_2 = LvlInitStruct<VkFormatProperties2>(&fmt_props_3);
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

void VkImageObj::init_no_mem(const vk_testing::Device &dev, const VkImageCreateInfo &info) {
    vk_testing::Image::init_no_mem(dev, info);
    Layout(info.initialLayout);
    m_mipLevels = info.mipLevels;
    m_arrayLayers = info.arrayLayers;
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

    VkImageViewCreateInfo view = LvlInitStruct<VkImageViewCreateInfo>();
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

    VkSamplerCreateInfo samplerCreateInfo = LvlInitStruct<VkSamplerCreateInfo>();
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

VkShaderObj::VkShaderObj(VkRenderFramework *framework, const char *source, VkShaderStageFlagBits stage, const spv_target_env env,
                         SpvSourceType source_type, const VkSpecializationInfo *spec_info, char const *name, bool debug)
    : m_framework(*framework), m_device(*(framework->DeviceObj())), m_source(source), m_spv_env(env) {
    m_stage_info = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
    m_stage_info.flags = 0;
    m_stage_info.stage = stage;
    m_stage_info.module = VK_NULL_HANDLE;
    m_stage_info.pName = name;
    m_stage_info.pSpecializationInfo = spec_info;
    if (source_type == SPV_SOURCE_GLSL) {
        InitFromGLSL(debug);
    } else if (source_type == SPV_SOURCE_ASM) {
        InitFromASM();
    }
}

bool VkShaderObj::InitFromGLSL(bool debug) {
    std::vector<uint32_t> spv;
    m_framework.GLSLtoSPV(&m_device.props.limits, m_stage_info.stage, m_source, spv, debug, m_spv_env);

    VkShaderModuleCreateInfo moduleCreateInfo = LvlInitStruct<VkShaderModuleCreateInfo>();
    moduleCreateInfo.codeSize = spv.size() * sizeof(uint32_t);
    moduleCreateInfo.pCode = spv.data();

    init(m_device, moduleCreateInfo);
    m_stage_info.module = handle();
    return VK_NULL_HANDLE != handle();
}

// Because shaders are currently validated at pipeline creation time, there are test cases that might fail shader module creation
// due to supplying an invalid/unknown SPIR-V capability/operation. This is called after VkShaderObj creation when tests are found
// to crash on a CI device
VkResult VkShaderObj::InitFromGLSLTry(bool debug, const VkDeviceObj *custom_device) {
    std::vector<uint32_t> spv;
    // 99% of tests just use the framework's VkDevice, but this allows for tests to use custom device object
    // Can't set at contructor time since all reference members need to be initialized then.
    VkPhysicalDeviceLimits limits = (custom_device) ? custom_device->props.limits : m_device.props.limits;
    m_framework.GLSLtoSPV(&limits, m_stage_info.stage, m_source, spv, debug, m_spv_env);

    VkShaderModuleCreateInfo moduleCreateInfo = LvlInitStruct<VkShaderModuleCreateInfo>();
    moduleCreateInfo.codeSize = spv.size() * sizeof(uint32_t);
    moduleCreateInfo.pCode = spv.data();

    const auto result = init_try(((custom_device) ? *custom_device : m_device), moduleCreateInfo);
    m_stage_info.module = handle();
    return result;
}

bool VkShaderObj::InitFromASM() {
    vector<uint32_t> spv;
    m_framework.ASMtoSPV(m_spv_env, 0, m_source, spv);

    VkShaderModuleCreateInfo moduleCreateInfo = LvlInitStruct<VkShaderModuleCreateInfo>();
    moduleCreateInfo.codeSize = spv.size() * sizeof(uint32_t);
    moduleCreateInfo.pCode = spv.data();

    init(m_device, moduleCreateInfo);
    m_stage_info.module = handle();
    return VK_NULL_HANDLE != handle();
}

VkResult VkShaderObj::InitFromASMTry() {
    vector<uint32_t> spv;
    m_framework.ASMtoSPV(m_spv_env, 0, m_source, spv);

    VkShaderModuleCreateInfo moduleCreateInfo = LvlInitStruct<VkShaderModuleCreateInfo>();
    moduleCreateInfo.codeSize = spv.size() * sizeof(uint32_t);
    moduleCreateInfo.pCode = spv.data();

    const auto result = init_try(m_device, moduleCreateInfo);
    m_stage_info.module = handle();
    return result;
}

// static
std::unique_ptr<VkShaderObj> VkShaderObj::CreateFromGLSL(VkRenderFramework &framework, VkShaderStageFlagBits stage,
                                                         const std::string &code, const char *entry_point,
                                                         const VkSpecializationInfo *spec_info, const spv_target_env spv_env,
                                                         bool debug) {
    auto shader = layer_data::make_unique<VkShaderObj>(&framework, code.c_str(), stage, spv_env, SPV_SOURCE_GLSL_TRY, spec_info,
                                                       entry_point, debug);
    if (VK_SUCCESS == shader->InitFromGLSLTry(debug)) {
        return shader;
    }
    return {};
}

// static
std::unique_ptr<VkShaderObj> VkShaderObj::CreateFromASM(VkRenderFramework &framework, VkShaderStageFlagBits stage,
                                                        const std::string &code, const char *entry_point,
                                                        const VkSpecializationInfo *spec_info, const spv_target_env spv_env) {
    auto shader =
        layer_data::make_unique<VkShaderObj>(&framework, code.c_str(), stage, spv_env, SPV_SOURCE_ASM_TRY, spec_info, entry_point);
    if (VK_SUCCESS == shader->InitFromASMTry()) {
        return shader;
    }
    return {};
}

VkPipelineLayoutObj::VkPipelineLayoutObj(VkDeviceObj *device, const vector<const VkDescriptorSetLayoutObj *> &descriptor_layouts,
                                         const vector<VkPushConstantRange> &push_constant_ranges,
                                         VkPipelineLayoutCreateFlags flags) {
    VkPipelineLayoutCreateInfo pl_ci = LvlInitStruct<VkPipelineLayoutCreateInfo>();
    pl_ci.flags = flags;
    pl_ci.pushConstantRangeCount = static_cast<uint32_t>(push_constant_ranges.size());
    pl_ci.pPushConstantRanges = push_constant_ranges.data();

    auto descriptor_layouts_unwrapped = MakeTestbindingHandles<const vk_testing::DescriptorSetLayout>(descriptor_layouts);

    init(*device, pl_ci, descriptor_layouts_unwrapped);
}

void VkPipelineLayoutObj::Reset() { *this = VkPipelineLayoutObj(); }

VkPipelineObj::VkPipelineObj(VkDeviceObj *device) {
    m_device = device;

    m_vi_state = LvlInitStruct<VkPipelineVertexInputStateCreateInfo>();
    m_vi_state.flags = 0;
    m_vi_state.vertexBindingDescriptionCount = 0;
    m_vi_state.pVertexBindingDescriptions = nullptr;
    m_vi_state.vertexAttributeDescriptionCount = 0;
    m_vi_state.pVertexAttributeDescriptions = nullptr;

    m_ia_state = LvlInitStruct<VkPipelineInputAssemblyStateCreateInfo>();
    m_ia_state.flags = 0;
    m_ia_state.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    m_ia_state.primitiveRestartEnable = VK_FALSE;

    m_te_state = nullptr;

    m_vp_state = LvlInitStruct<VkPipelineViewportStateCreateInfo>();
    m_vp_state.flags = 0;
    m_vp_state.viewportCount = 1;
    m_vp_state.scissorCount = 1;
    m_vp_state.pViewports = nullptr;
    m_vp_state.pScissors = nullptr;

    m_rs_state = LvlInitStruct<VkPipelineRasterizationStateCreateInfo>(&m_line_state);
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

    m_line_state = LvlInitStruct<VkPipelineRasterizationLineStateCreateInfoEXT>();
    m_line_state.lineRasterizationMode = VK_LINE_RASTERIZATION_MODE_DEFAULT_EXT;
    m_line_state.stippledLineEnable = VK_FALSE;
    m_line_state.lineStippleFactor = 0;
    m_line_state.lineStipplePattern = 0;

    m_ms_state = LvlInitStruct<VkPipelineMultisampleStateCreateInfo>();
    m_ms_state.flags = 0;
    m_ms_state.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    m_ms_state.sampleShadingEnable = VK_FALSE;
    m_ms_state.minSampleShading = 0.0f;
    m_ms_state.pSampleMask = nullptr;
    m_ms_state.alphaToCoverageEnable = VK_FALSE;
    m_ms_state.alphaToOneEnable = VK_FALSE;

    m_ds_state = nullptr;

    memset(&m_cb_state, 0, sizeof(m_cb_state));
    m_cb_state = LvlInitStruct<VkPipelineColorBlendStateCreateInfo>();
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

    gp_ci->pVertexInputState = &m_vi_state;
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
        if (std::find(m_dynamic_state_enables.cbegin(), m_dynamic_state_enables.cend(), VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT) ==
            m_dynamic_state_enables.cend()) {
            MakeDynamic(VK_DYNAMIC_STATE_VIEWPORT);
            m_vp_state.viewportCount = 1;
        } else {
            m_vp_state.viewportCount = 0;
        }
        m_vp_state.pViewports = nullptr;
    }

    if (m_scissors.size() > 0) {
        m_vp_state.scissorCount = m_scissors.size();
        m_vp_state.pScissors = m_scissors.data();
    } else {
        if (std::find(m_dynamic_state_enables.cbegin(), m_dynamic_state_enables.cend(), VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT) ==
            m_dynamic_state_enables.cend()) {
            MakeDynamic(VK_DYNAMIC_STATE_SCISSOR);
        }
        m_vp_state.scissorCount = 1;
        m_vp_state.pScissors = nullptr;
    }

    memset(&m_pd_state, 0, sizeof(m_pd_state));
    if (m_dynamic_state_enables.size() > 0) {
        m_pd_state = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
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

void VkCommandBufferObj::Init(VkDeviceObj *device, VkCommandPoolObj *pool, VkCommandBufferLevel level, VkQueueObj *queue) {
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

VkCommandBufferObj::VkCommandBufferObj(VkDeviceObj *device, VkCommandPoolObj *pool, VkCommandBufferLevel level, VkQueueObj *queue) {
    Init(device, pool, level, queue);
}

void VkCommandBufferObj::PipelineBarrier(VkPipelineStageFlags src_stages, VkPipelineStageFlags dest_stages,
                                         VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount,
                                         const VkMemoryBarrier *pMemoryBarriers, uint32_t bufferMemoryBarrierCount,
                                         const VkBufferMemoryBarrier *pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount,
                                         const VkImageMemoryBarrier *pImageMemoryBarriers) {
    vk::CmdPipelineBarrier(handle(), src_stages, dest_stages, dependencyFlags, memoryBarrierCount, pMemoryBarriers,
                           bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
}

void VkCommandBufferObj::PipelineBarrier2KHR(const VkDependencyInfoKHR *pDependencyInfo) {
    auto fpCmdPipelineBarrier2KHR =
        (PFN_vkCmdPipelineBarrier2KHR)vk::GetDeviceProcAddr(m_device->device(), "vkCmdPipelineBarrier2KHR");
    assert(fpCmdPipelineBarrier2KHR != nullptr);

    fpCmdPipelineBarrier2KHR(handle(), pDependencyInfo);
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

void VkCommandBufferObj::BeginRenderPass(const VkRenderPassBeginInfo &info, VkSubpassContents contents) {
    vk::CmdBeginRenderPass(handle(), &info, contents);
}

void VkCommandBufferObj::NextSubpass(VkSubpassContents contents) { vk::CmdNextSubpass(handle(), contents); }

void VkCommandBufferObj::EndRenderPass() { vk::CmdEndRenderPass(handle()); }

void VkCommandBufferObj::BeginRendering(const VkRenderingInfoKHR &renderingInfo) {
    PFN_vkCmdBeginRenderingKHR vkCmdBeginRenderingKHR =
        (PFN_vkCmdBeginRenderingKHR)vk::GetDeviceProcAddr(m_device->device(), "vkCmdBeginRenderingKHR");
    assert(vkCmdBeginRenderingKHR != nullptr);

    vkCmdBeginRenderingKHR(handle(), &renderingInfo);
}

void VkCommandBufferObj::EndRendering() {
    PFN_vkCmdEndRenderingKHR vkCmdEndRenderingKHR =
        (PFN_vkCmdEndRenderingKHR)vk::GetDeviceProcAddr(m_device->device(), "vkCmdEndRenderingKHR");
    assert(vkCmdEndRenderingKHR != nullptr);

    vkCmdEndRenderingKHR(handle());
}

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

void VkCommandBufferObj::QueueCommandBuffer(bool check_success) {
    VkFenceObj null_fence;
    QueueCommandBuffer(null_fence, check_success);
}

void VkCommandBufferObj::QueueCommandBuffer(const VkFenceObj &fence, bool check_success, bool submit_2) {
    VkResult err = VK_SUCCESS;

    if (submit_2) {
        err = m_queue->submit2(*this, fence, check_success);
    } else {
        err = m_queue->submit(*this, fence, check_success);
    }
    if (check_success) {
        ASSERT_VK_SUCCESS(err);
    }

    err = m_queue->wait();
    if (check_success) {
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

void VkCommandPoolObj::Init(VkDeviceObj *device, uint32_t queue_family_index, VkCommandPoolCreateFlags flags) {
    init(*device, vk_testing::CommandPool::create_info(queue_family_index, flags));
}

VkCommandPoolObj::VkCommandPoolObj(VkDeviceObj *device, uint32_t queue_family_index, VkCommandPoolCreateFlags flags) {
    Init(device, queue_family_index, flags);
}

bool VkDepthStencilObj::Initialized() { return m_initialized; }
VkDepthStencilObj::VkDepthStencilObj(VkDeviceObj *device) : VkImageObj(device) { m_initialized = false; }

VkImageView *VkDepthStencilObj::BindInfo() { return &m_attachmentBindInfo; }

VkFormat VkDepthStencilObj::Format() const { return this->m_depth_stencil_fmt; }

void VkDepthStencilObj::Init(VkDeviceObj *device, int32_t width, int32_t height, VkFormat format, VkImageUsageFlags usage,
                             VkImageAspectFlags aspect) {
    VkImageViewCreateInfo view_info = LvlInitStruct<VkImageViewCreateInfo>();

    m_device = device;
    m_initialized = true;
    m_depth_stencil_fmt = format;

    /* create image */
    VkImageObj::Init(width, height, 1, m_depth_stencil_fmt, usage, VK_IMAGE_TILING_OPTIMAL);

    // allows for overriding by caller
    if (aspect == 0) {
        aspect = VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_DEPTH_BIT;
        if (FormatIsDepthOnly(format))
            aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
        else if (FormatIsStencilOnly(format))
            aspect = VK_IMAGE_ASPECT_STENCIL_BIT;
    }
    SetLayout(aspect, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

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
