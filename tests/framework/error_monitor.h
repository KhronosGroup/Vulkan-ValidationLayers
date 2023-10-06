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
#pragma once

#include "test_common.h"
#include <unordered_set>

// ErrorMonitor Usage:
//
// Call SetDesiredFailureMsg with a string to be compared against all
// encountered log messages, or a validation error enum identifying
// desired error message. Passing NULL or VALIDATION_ERROR_MAX_ENUM
// will match all log messages. logMsg will return true for skipCall
// only if msg is matched or NULL.
//
// Call VerifyFound to determine if all desired failure messages
// were encountered. Call VerifyNotFound to determine if any unexpected
// failure was encountered.
class ErrorMonitor {
  public:
    ErrorMonitor(bool print_all_errors);
    ~ErrorMonitor() noexcept = default;

    void CreateCallback(VkInstance instance) noexcept;
    void DestroyCallback(VkInstance instance) noexcept;

    ErrorMonitor(const ErrorMonitor &) = delete;
    ErrorMonitor &operator=(const ErrorMonitor &) = delete;
    ErrorMonitor(ErrorMonitor &&) noexcept = delete;
    ErrorMonitor &operator=(ErrorMonitor &&) noexcept = delete;

    // Set monitor to pristine state
    void Reset();

    // ErrorMonitor will look for an error message containing the specified string(s)
    void SetDesiredFailureMsg(const VkFlags msgFlags, const std::string &msg);
    void SetDesiredFailureMsg(const VkFlags msgFlags, const char *const msgString);

    // Set an error that the error monitor will ignore. Do not use this function if you are creating a new test.
    // TODO: This is stopgap to block new unexpected errors from being introduced. The long-term goal is to remove the use of this
    // function and its definition.
    void SetUnexpectedError(const char *const msg);

    // Set an error that should not cause a test failure
    void SetAllowedFailureMsg(const char *const msg);

    VkBool32 CheckForDesiredMsg(const char *const msgString);
    VkDebugReportFlagsEXT GetMessageFlags();
    void SetError(const char *const errorString);
    void SetBailout(std::atomic<bool> *bailout);

    // Helpers
    void VerifyFound();
    void Finish();

    const auto *GetDebugCreateInfo() const { return &debug_create_info_; }

    // ExpectSuccess now takes an optional argument allowing a custom combination of debug flags
    void ExpectSuccess(VkDebugReportFlagsEXT const message_flag_mask = kErrorBit);

  private:
    bool ExpectingSuccess() const;
    bool NeedCheckSuccess() const;
    void VerifyNotFound();
    // TODO: This is stopgap to block new unexpected errors from being introduced. The long-term goal is to remove the use of this
    // function and its definition.
    bool IgnoreMessage(std::string const &msg) const;
    bool AnyDesiredMsgFound() const;
    void MonitorReset();
    std::unique_lock<std::mutex> Lock() const { return std::unique_lock<std::mutex>(mutex_); }

#if !defined(VK_USE_PLATFORM_ANDROID_KHR)
    VkDebugUtilsMessengerEXT debug_obj_ = VK_NULL_HANDLE;
    VkDebugUtilsMessengerCreateInfoEXT debug_create_info_{};
#else
    VkDebugReportCallbackEXT debug_obj_ = VK_NULL_HANDLE;
    VkDebugReportCallbackCreateInfoEXT debug_create_info_{};
#endif

    VkFlags message_flags_{};
    std::unordered_multiset<std::string> desired_message_strings_;
    std::unordered_multiset<std::string> failure_message_strings_;
    std::vector<std::string> ignore_message_strings_;
    std::vector<std::string> allowed_message_strings_;
    mutable std::mutex mutex_;
    std::atomic<bool> *bailout_{};
    bool message_found_{};
    bool print_all_errors_{};
};
