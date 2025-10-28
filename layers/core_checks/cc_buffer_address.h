/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
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

#include "containers/span.h"
#include "core_checks/core_validation.h"
#include "state_tracker/buffer_state.h"
#include "error_message/logging.h"

#include <array>
#include <functional>
#include <sstream>
#include <string>
#include <string_view>

/* This class aims at helping with the validation of a family of VUIDs referring to the same buffer device address.
   For example, take those VUIDs for VkDescriptorBufferBindingInfoEXT:

   VUID-VkDescriptorBufferBindingInfoEXT-usage-08122
   If usage includes VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT, address must be an address within a valid buffer that was
   created with VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT

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
   Then, just a call to ValidateDeviceAddress is needed to do validation and, eventually, error logging.

   For an example of how to use BufferAddressValidation, see for instance how "VUID-VkDescriptorBufferBindingInfoEXT-usage-08122"
   and friends are validated.

   More details in https://gitlab.khronos.org/vulkan/vulkan/-/merge_requests/7517
 */

template <size_t ChecksCount = 1>
class BufferAddressValidation {
  public:
    using IsInvalidFunction = std::function<bool(const vvl::Buffer&)>;
    using ErrorMsgHeaderFunction = std::function<std::string()>;
    using ErrorMsgBuffer = std::function<std::string(const vvl::Buffer&)>;
    using UpdateCallback = std::function<void(const vvl::Buffer&)>;

    struct VuidAndValidation {
        std::string_view vuid{};
        // Return true if the buffer is invalid
        IsInvalidFunction is_invalid_func = [](const vvl::Buffer&) { return false; };
        // Text appended to error message header (for the VU as a whole)
        ErrorMsgHeaderFunction error_msg_header_func = []() { return "\n"; };
        // List dedicated error per buffer
        ErrorMsgBuffer error_msg_buffer_func = [](const vvl::Buffer&) { return ""; };
    };

    // +1 for extra check for "valid generic VkDeviceAddress" check
    std::array<VuidAndValidation, ChecksCount + 1> vuid_and_validations;
    // There are times the caller will want to update state for each buffer object found
    UpdateCallback update_callback = [](const vvl::Buffer&) {};

    [[nodiscard]] bool ValidateDeviceAddress(const CoreChecks& validator, const Location& device_address_loc,
                                             const LogObjectList& objlist, VkDeviceAddress device_address,
                                             VkDeviceSize range_size = 0) noexcept {
        bool skip = false;
        // There will be an implicit VU like "must be a valid VkDeviceAddress value" and if can't be zero, stateless validation
        // should have caught this already
        if (device_address == 0) {
            return skip;
        }

        vvl::span<vvl::Buffer* const> buffer_list = validator.GetBuffersByAddress(device_address);
        if (buffer_list.empty()) {
            skip |= validator.LogError(
                "VUID-VkDeviceAddress-size-11364", objlist, device_address_loc,
                "(0x%" PRIx64 ") is not a valid buffer address. No call to vkGetBufferDeviceAddress has this buffer in its range.",
                device_address);
        }

        // Checks if memory is in a completely and contiguously to a single VkDeviceMemory object
        // Everyone needs to check for this in order to be a valid VkDeviceAddress
        vuid_and_validations[ChecksCount] = {
            "VUID-VkDeviceAddress-None-10894",
            [](const vvl::Buffer& buffer_state) { return !buffer_state.sparse && !buffer_state.IsMemoryBound(); },
            []() { return "The following buffers are not bound to memory or it has been freed:"; },
            [&validator](const vvl::Buffer& buffer_state) {
                const auto memory_state = buffer_state.MemoryState();
                if (memory_state && memory_state->Destroyed()) {
                    return "buffer is bound to memory (" + validator.FormatHandle(memory_state->Handle()) +
                           ") but it has been freed";
                }
                return std::string("buffer has not been bound to memory");
            }};

        if (!HasValidBuffer(buffer_list)) {
            skip |= LogInvalidBuffers(validator, buffer_list, device_address_loc, objlist, device_address, range_size);
        }
        return skip;
    }

  private:
    // Look for a buffer that satisfies all VUIDs
    [[nodiscard]] bool HasValidBuffer(vvl::span<vvl::Buffer* const> buffer_list) const noexcept;
    // For every vuid, build an error mentioning every buffer from buffer_list that violates it, then log this error
    // using details provided by the other parameters.
    [[nodiscard]] bool LogInvalidBuffers(const CoreChecks& validator, vvl::span<vvl::Buffer* const> buffer_list,
                                         const Location& device_address_loc, const LogObjectList& objlist,
                                         VkDeviceAddress device_address, VkDeviceSize range_size) const noexcept;

    struct Error {
        LogObjectList objlist;
        std::string error_msg;
        bool Empty() const { return error_msg.empty(); }
    };
};

template <size_t ChecksCount>
bool BufferAddressValidation<ChecksCount>::HasValidBuffer(vvl::span<vvl::Buffer* const> buffer_list) const noexcept {
    bool any_buffer_found = false;
    for (const auto& buffer : buffer_list) {
        ASSERT_AND_CONTINUE(buffer);

        // Call here as we will need to update once for each buffer
        update_callback(*buffer);

        bool is_buffer_valid = true;
        // Once we find any buffer is valid, can just skip checking
        if (!any_buffer_found) {
            for (const auto& vav : vuid_and_validations) {
                if (vav.is_invalid_func(*buffer)) {
                    is_buffer_valid = false;
                    break;
                }
            }
        }

        any_buffer_found |= is_buffer_valid;
    }
    return any_buffer_found;
}

template <size_t ChecksCount>
bool BufferAddressValidation<ChecksCount>::LogInvalidBuffers(const CoreChecks& validator, vvl::span<vvl::Buffer* const> buffer_list,
                                                             const Location& device_address_loc, const LogObjectList& objlist,
                                                             VkDeviceAddress device_address,
                                                             VkDeviceSize range_size) const noexcept {
    std::array<Error, ChecksCount + 1> errors;

    // Build error message beginning. Then, only per buffer error needs to be appended.
    std::string error_msg_beginning;

    // Some checks only care about the address, but for those that have a range, print it here so it is the same across all error
    // messages
    if (range_size != 0) {
        std::stringstream ss;
        ss << "[0x" << std::hex << device_address << ", 0x" << (device_address + range_size) << ") (";
        if (range_size == VK_WHOLE_SIZE) {
            ss << "VK_WHOLE_SIZE";  // Not really sure what this means in practice yet, but alert the user
        } else {
            ss << std::dec << range_size << " bytes";
        }
        ss << ") has no buffer(s) associated that are valid.\n";
        error_msg_beginning = ss.str();
    } else {
        std::stringstream ss;
        ss << "(0x" << std::hex << device_address << ") has no buffer(s) associated that are valid.\n";
        error_msg_beginning = ss.str();
    }

    // For each buffer, and for each violated VUID, build an error message
    for (const auto& buffer : buffer_list) {
        ASSERT_AND_CONTINUE(buffer);

        for (size_t i = 0; i < (ChecksCount + 1); ++i) {
            [[maybe_unused]] const auto& [vuid, is_invalid_func, error_msg_header_func, error_msg_buffer_func] =
                vuid_and_validations[i];

            if (!is_invalid_func(*buffer)) {
                continue;
            }

            // Add faulty buffer to current vuid LogObjectList
            errors[i].objlist.add(buffer->Handle());

            auto& error_msg = errors[i].error_msg;
            // Append faulty buffer error message
            if (error_msg.empty()) {
                error_msg += error_msg_beginning;
                error_msg += error_msg_header_func();
                error_msg += ":\n";
            }

            // Always print the buffer range/size
            error_msg += "  ";  // small indent help to visualize
            error_msg += validator.FormatHandle(buffer->Handle());
            error_msg += ", size ";
            error_msg += std::to_string(buffer->create_info.size);
            error_msg += ", range ";
            error_msg += string_range_hex(buffer->DeviceAddressRange());
            error_msg += " ";
            error_msg += error_msg_buffer_func(*buffer);
            error_msg += "\n";
        }
    }

    // Output the error messages
    bool skip = false;
    for (size_t i = 0; i < (ChecksCount + 1); ++i) {
        const auto& vav = vuid_and_validations[i];
        auto& error = errors[i];
        if (!error.Empty()) {
            // Add user provided handles, typically the current command buffer or the device
            for (const auto& obj : objlist) {
                error.objlist.add(obj);
            }
            skip |= validator.LogError(vav.vuid.data(), error.objlist, device_address_loc, "%s", error.error_msg.c_str());
        }
    }

    return skip;
}

[[maybe_unused]] static std::string PrintBufferRanges(const CoreChecks& validator, vvl::span<vvl::Buffer* const> buffers) {
    std::ostringstream ss;
    for (const auto& buffer : buffers) {
        ss << "  " << validator.FormatHandle(buffer->Handle()) << " : size " << buffer->create_info.size << " : range "
           << string_range_hex(buffer->DeviceAddressRange()) << '\n';
    }
    return ss.str();
}