/* Copyright (c) 2015-2023 The Khronos Group Inc.
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

#include "state_tracker/buffer_state.h"
#include "core_validation.h"
#include "state_tracker/state_tracker.h"
#include "containers/custom_containers.h"
#include "error_message/logging.h"

#include <array>
#include <functional>
#include <iterator>
#include <string>
#include <string_view>

/* This class aims at helping with the validation of a family of VUIDs referring to the same buffer device address.
   For example, take those VUIDs for VkDescriptorBufferBindingInfoEXT:

   VUID-VkDescriptorBufferBindingInfoEXT-usage-08122:
   If usage includes VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT, address must be an address within a valid buffer that was
   created with VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT
   VUID-VkDescriptorBufferBindingInfoEXT-usage-08123:
   If usage includes VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT, address must be an address within a valid buffer that was
   created with VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT
   VUID-VkDescriptorBufferBindingInfoEXT-usage-08124:
   If usage includes VK_BUFFER_USAGE_PUSH_DESCRIPTORS_DESCRIPTOR_BUFFER_BIT_EXT, address must be an address within a valid buffer
   that was created with VK_BUFFER_USAGE_PUSH_DESCRIPTORS_DESCRIPTOR_BUFFER_BIT_EXT

   For usage to be valid, since the mentioned address can refer to multiple buffers, one must find a buffer that satisfies *all*
   of them. One must *not* consider those VUIDs independantly, each time trying to find a buffer that satisfies the considered VUID
   but not necessarily the others in the family.

   The VVL heuristic wants that the vast majority of the time, functions calls and structures are valid, thus validation
   should be fast and avoid to do things related to error logging unless necessary. To comply with that, buffer address
   validation is done in two passes: one to look for a buffer satisfying all VUIDs of the considered family. If none
   is found, another pass is done, this time building a per VUID error message (not per buffer) regrouping all buffers
   violating it. Outputting for each buffer every VUID it violates would lead to unnecessary log clutter.
   This two pass process is tedious to do without an helper class, hence BufferAddressValidation was created.
   The idea is to ask for user to provide the only necessary data:
   a VUID, how it is validated, what to log when a buffer violates it, and a snippet of text appended to the error message header.
   Then, just a call to LogErrorsIfNoValidBuffer is needed to do validation and, eventually, error logging.

   For an example of how to use BufferAddressValidation, see for instance how "VUID-VkDescriptorBufferBindingInfoEXT-usage-08122"
   and friends are validated.
 */
template <size_t N>
class BufferAddressValidation {
  public:
    // Return true if and only if VU is verified
    using ValidationFunction = std::function<bool(const ValidationStateTracker::BUFFER_STATE_PTR&, std::string* out_error_msg)>;
    using ErrorMsgHeaderSuffixFunction = std::function<std::string()>;

    struct VuidAndValidation {
        std::string_view vuid{};
        LogObjectList objlist{};
        ValidationFunction validation_func = [](const ValidationStateTracker::BUFFER_STATE_PTR&, std::string* out_error_msg) {
            return true;
        };
        ErrorMsgHeaderSuffixFunction error_msg_header_suffix_func = []() { return "\n"; };  // text appended to error message header
    };

    // Look for a buffer that satisfies all VUIDs
    [[nodiscard]] bool HasValidBuffer(vvl::span<const ValidationStateTracker::BUFFER_STATE_PTR> buffer_list) const noexcept;
    // Look for a buffer that does not satisfy one of the VUIDs
    [[nodiscard]] bool HasInvalidBuffer(vvl::span<const ValidationStateTracker::BUFFER_STATE_PTR> buffer_list) const noexcept;
    // For every vuid, build an error mentioning every buffer from buffer_list that violates it, then log this error
    // using details provided by the other parameters.
    [[nodiscard]] bool LogInvalidBuffers(const CoreChecks& checker,
                                         vvl::span<const ValidationStateTracker::BUFFER_STATE_PTR> buffer_list,
                                         std::string_view api_name, const std::string& device_address_name,
                                         VkDeviceAddress device_address) const noexcept;

    [[nodiscard]] bool LogErrorsIfNoValidBuffer(const CoreChecks& checker,
                                                vvl::span<const ValidationStateTracker::BUFFER_STATE_PTR> buffer_list,
                                                std::string_view api_name, const std::string& device_address_name,
                                                VkDeviceAddress device_address) const noexcept {
        bool skip = false;
        if (!HasValidBuffer(buffer_list)) {
            skip |= LogInvalidBuffers(checker, buffer_list, api_name, device_address_name, device_address);
        }
        return skip;
    }
    [[nodiscard]] bool LogErrorsIfInvalidBufferFound(const CoreChecks& checker,
                                                vvl::span<const ValidationStateTracker::BUFFER_STATE_PTR> buffer_list,
                                                std::string_view api_name, const std::string& device_address_name,
                                                VkDeviceAddress device_address) const noexcept {
        bool skip = false;
        if (HasInvalidBuffer(buffer_list)) {
            skip |= LogInvalidBuffers(checker, buffer_list, api_name, device_address_name, device_address);
        }
        return skip;
    }

    bool AddVuidValidation(VuidAndValidation&& vuid_and_validation) noexcept {
        for (size_t i = 0; i < N; ++i) {
            if (vuidsAndValidationFunctions[i].vuid.empty()) {
                vuidsAndValidationFunctions[i] = std::move(vuid_and_validation);
                return true;
            }
        }
        return false;
    }

  public:
    // public to enable aggregate initialization
    std::array<VuidAndValidation, N> vuidsAndValidationFunctions;

  private:
    struct Error {
        LogObjectList objlist;
        std::string error_msg;
        bool Empty() const { return error_msg.empty(); }
    };
};

template <size_t N>
bool BufferAddressValidation<N>::HasValidBuffer(
    vvl::span<const ValidationStateTracker::BUFFER_STATE_PTR> buffer_list) const noexcept {
    for (const auto& buffer : buffer_list) {
        assert(buffer);

        bool valid_buffer_found = true;
        for (const auto& vuidAndValidation : vuidsAndValidationFunctions) {
            if (!vuidAndValidation.validation_func(buffer, nullptr)) {
                valid_buffer_found = false;
                break;
            }
        }
        if (valid_buffer_found) return true;
    }

    return false;
}

template <size_t N>
bool BufferAddressValidation<N>::HasInvalidBuffer(
    vvl::span<const ValidationStateTracker::BUFFER_STATE_PTR> buffer_list) const noexcept {
    for (const auto& buffer : buffer_list) {
        assert(buffer);

        for (const auto& vuidAndValidation : vuidsAndValidationFunctions) {
            if (!vuidAndValidation.validation_func(buffer, nullptr)) {
                return true;
            }
        }
    }

    return false;
}

template <size_t N>
bool BufferAddressValidation<N>::LogInvalidBuffers(const CoreChecks& checker,
                                                   vvl::span<const ValidationStateTracker::BUFFER_STATE_PTR> buffer_list,
                                                   std::string_view api_name, const std::string& device_address_name,
                                                   VkDeviceAddress device_address) const noexcept {
    std::array<Error, N> errors;

    // Build error message beginning. Then, only per buffer error needs to be appended.
    std::string error_msg_beginning(api_name);
    {
        const std::string address_string = [&]() {
            std::stringstream address_ss;
            address_ss << "0x" << std::hex << device_address;
            return address_ss.str();
        }();
        error_msg_beginning += ": No buffer associated to ";
        error_msg_beginning += device_address_name;
        error_msg_beginning += " (";
        error_msg_beginning += address_string;
        error_msg_beginning +=
            ") was found such that valid usage passes. "
            "At least one buffer associated to this device address must be valid. ";
    }

    // For each buffer, and for each violated VUID, build an error message
    for (const auto& buffer : buffer_list) {
        assert(buffer);

        for (size_t i = 0; i < N; ++i) {
            [[maybe_unused]] const auto& [vuid, objlist, validation_func, error_msg_header_suffix_func] =
                vuidsAndValidationFunctions[i];

            // Fill buffer_error with error if there is one, and if validation function did fill a buffer error message
            if (std::string buffer_error; !validation_func(buffer, &buffer_error) && !buffer_error.empty()) {
                auto& error_objlist = errors[i].objlist;
                auto& error_msg = errors[i].error_msg;

                if (error_objlist.empty()) {
                    // Add user provided handles, typically the current command buffer or the device
                    for (const auto& obj : objlist) {
                        error_objlist.add(obj);
                    }
                }
                // Add faulty buffer to current vuid LogObjectList
                error_objlist.add(buffer->Handle());

                // Append faulty buffer error message
                if (error_msg.empty()) {
                    error_msg += error_msg_beginning;
                    error_msg += error_msg_header_suffix_func();
                }
                const auto invalid_buffer_index = error_objlist.size() - 1;
                error_msg += "Object " + std::to_string(invalid_buffer_index) + ": " + buffer_error + '\n';
            }
        }
    }

    // Output the error messages
    bool skip = false;
    for (size_t i = 0; i < N; ++i) {
        const auto& vuidAndValidation = vuidsAndValidationFunctions[i];
        const auto& error = errors[i];
        if (!error.Empty()) {
            skip |= checker.LogError(error.objlist, vuidAndValidation.vuid.data(), "%s", error.error_msg.c_str());
        }
    }

    return skip;
}
