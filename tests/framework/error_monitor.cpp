/*
 * Copyright (c) 2015-2022 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
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
#include "error_monitor.h"

#ifdef VK_USE_PLATFORM_ANDROID_KHR
// Note. VK_EXT_debug_report is deprecated by the VK_EXT_debug_utils extension.
// However, we still support this old extension due to CI running old Android devices.
static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugReportFlagsEXT message_flags, VkDebugReportObjectTypeEXT, uint64_t,
                                                    size_t, int32_t, const char *, const char *message, void *user_data) {
    auto *error_monitor = reinterpret_cast<ErrorMonitor *>(user_data);

    if (message_flags & error_monitor->GetMessageFlags()) {
        return error_monitor->CheckForDesiredMsg(message);
    }
    return VK_FALSE;
}
#else

static inline LogMessageTypeFlags DebugAnnotFlagsToMsgTypeFlags(VkDebugUtilsMessageSeverityFlagBitsEXT da_severity,
                                                                VkDebugUtilsMessageTypeFlagsEXT da_type) {
    LogMessageTypeFlags msg_type_flags = 0;
    if ((da_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) != 0) {
        msg_type_flags |= kErrorBit;
    } else if ((da_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) != 0) {
        if ((da_type & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) != 0) {
            msg_type_flags |= kPerformanceWarningBit;
        } else {
            msg_type_flags |= kWarningBit;
        }
    } else if ((da_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) != 0) {
        msg_type_flags |= kInformationBit;
    } else if ((da_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) != 0) {
        msg_type_flags |= kVerboseBit;
    }
    return msg_type_flags;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                                                    VkDebugUtilsMessageTypeFlagsEXT message_types,
                                                    const VkDebugUtilsMessengerCallbackDataEXT *callback_data, void *user_data) {
    const auto message_flags = DebugAnnotFlagsToMsgTypeFlags(message_severity, message_types);
    const char *message = callback_data->pMessage;
    auto *error_monitor = reinterpret_cast<ErrorMonitor *>(user_data);

    if (message_flags & error_monitor->GetMessageFlags()) {
        return error_monitor->CheckForDesiredMsg(message);
    }
    return VK_FALSE;
}
#endif

ErrorMonitor::ErrorMonitor(bool print_all_errors) : print_all_errors_(print_all_errors) {
    MonitorReset();
    ExpectSuccess(kErrorBit);
#if !defined(VK_USE_PLATFORM_ANDROID_KHR)
    debug_create_info_ = {VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
                          nullptr,
                          0,
                          VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                              VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT,
                          VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
                          &DebugCallback,
                          this};
#else
    debug_create_info_ = {VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT, nullptr,
                          VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT |
                              VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
                              VK_DEBUG_REPORT_DEBUG_BIT_EXT,
                          &DebugCallback, this};
#endif
}

void ErrorMonitor::CreateCallback(VkInstance instance) noexcept {
    assert(instance);
    assert(!debug_obj_);

#if !defined(VK_USE_PLATFORM_ANDROID_KHR)
    assert(vk::CreateDebugUtilsMessengerEXT != nullptr);
    const VkResult result = vk::CreateDebugUtilsMessengerEXT(instance, &debug_create_info_, nullptr, &debug_obj_);
#else
    assert(vk::CreateDebugReportCallbackEXT != nullptr);
    const VkResult result = vk::CreateDebugReportCallbackEXT(instance, &debug_create_info_, nullptr, &debug_obj_);
#endif
    if (result != VK_SUCCESS) {
        assert(false);
        debug_obj_ = VK_NULL_HANDLE;
    }
}

void ErrorMonitor::DestroyCallback(VkInstance instance) noexcept {
    assert(instance);
    assert(debug_obj_);  // valid to call with null object, but probably bug

#if !defined(VK_USE_PLATFORM_ANDROID_KHR)
    vk::DestroyDebugUtilsMessengerEXT(instance, debug_obj_, nullptr);
#else
    vk::DestroyDebugReportCallbackEXT(instance, debug_obj_, nullptr);
#endif
    debug_obj_ = VK_NULL_HANDLE;
}

void ErrorMonitor::MonitorReset() {
    message_flags_ = kErrorBit;
    bailout_ = nullptr;
    message_found_ = false;
    failure_message_strings_.clear();
    desired_message_strings_.clear();
    ignore_message_strings_.clear();
    allowed_message_strings_.clear();
}

void ErrorMonitor::Reset() {
    auto guard = Lock();
    MonitorReset();
}

void ErrorMonitor::SetDesiredFailureMsg(const VkFlags msgFlags, const std::string &msg) {
    SetDesiredFailureMsg(msgFlags, msg.c_str());
}

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
    std::string error_string(msgString);
    bool found_expected = false;

    if (print_all_errors_) {
        std::cout << error_string << "\n\n";
    }
    if (!IgnoreMessage(error_string)) {
        for (auto desired_msg_it = desired_message_strings_.begin(); desired_msg_it != desired_message_strings_.end();
             ++desired_msg_it) {
            if (error_string.find(*desired_msg_it) != std::string::npos) {
                found_expected = true;
                failure_message_strings_.insert(error_string);
                message_found_ = true;
                result = VK_TRUE;
                // Remove a maximum of one failure message from the set
                // Multiset mutation is acceptable because `break` causes flow of control to exit the for loop
                desired_message_strings_.erase(desired_msg_it);
                break;
            }
        }

        if (!found_expected && !allowed_message_strings_.empty()) {
            for (auto allowed_msg_it = allowed_message_strings_.begin(); allowed_msg_it != allowed_message_strings_.end();
                 ++allowed_msg_it) {
                if (error_string.find(*allowed_msg_it) != std::string::npos) {
                    found_expected = true;
                    break;
                }
            }
        }

        if (!found_expected) {
            result = VK_TRUE;
            ADD_FAILURE() << error_string;
        }
    }
    return result;
}

VkDebugReportFlagsEXT ErrorMonitor::GetMessageFlags() { return message_flags_; }

bool ErrorMonitor::AnyDesiredMsgFound() const { return message_found_; }

void ErrorMonitor::SetError(const char *const errorString) {
    auto guard = Lock();
    message_found_ = true;
    failure_message_strings_.insert(errorString);
}

void ErrorMonitor::SetBailout(std::atomic<bool> *bailout) {
    auto guard = Lock();
    bailout_ = bailout;
}

void ErrorMonitor::ExpectSuccess(VkDebugReportFlagsEXT const message_flag_mask) {
    // Match ANY message matching specified type
    auto guard = Lock();
    desired_message_strings_.clear();
    message_flags_ = message_flag_mask;
}

bool ErrorMonitor::ExpectingSuccess() const {
    return (desired_message_strings_.size() == 1) && (desired_message_strings_.count("") == 1 && ignore_message_strings_.empty());
}

bool ErrorMonitor::NeedCheckSuccess() const { return ExpectingSuccess(); }

void ErrorMonitor::VerifyFound() {
    {
        // The lock must be released before the ExpectSuccess call at the end
        auto guard = Lock();
        // Not receiving expected message(s) is a failure.
        if (!desired_message_strings_.empty()) {
            for (const auto &desired_msg : desired_message_strings_) {
                ADD_FAILURE() << "Did not receive expected error '" << desired_msg << "'";
            }
        }
        MonitorReset();
    }

    ExpectSuccess();
}

void ErrorMonitor::Finish() {
    VerifyNotFound();
    Reset();
}

void ErrorMonitor::VerifyNotFound() {
    auto guard = Lock();
    // ExpectSuccess() configured us to match anything. Any error is a failure.
    if (AnyDesiredMsgFound()) {
        for (const auto &msg : failure_message_strings_) {
            ADD_FAILURE() << "Expected to succeed but got error: " << msg;
        }
    }
    MonitorReset();
}

bool ErrorMonitor::IgnoreMessage(std::string const &msg) const {
    if (ignore_message_strings_.empty()) {
        return false;
    }

    return std::find_if(ignore_message_strings_.begin(), ignore_message_strings_.end(), [&msg](std::string const &str) {
               return msg.find(str) != std::string::npos;
           }) != ignore_message_strings_.end();
}
