/* Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (C) 2015-2023 Google Inc.
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

#include "stateless/stateless_validation.h"

bool StatelessValidation::CheckPromotedApiAgainstVulkanVersion(VkInstance instance, const Location &loc,
                                                               const uint32_t promoted_version) const {
    bool skip = false;
    if (api_version < promoted_version) {
        skip = LogError(kVUID_PVError_ApiVersionViolation, instance, loc,
                        "Attempted to call with an effective API version of %s"
                        "but this API was not promoted until version %s.",
                        StringAPIVersion(api_version).c_str(), StringAPIVersion(promoted_version).c_str());
    }
    return skip;
}

bool StatelessValidation::CheckPromotedApiAgainstVulkanVersion(VkPhysicalDevice pdev, const Location &loc,
                                                               const uint32_t promoted_version) const {
    bool skip = false;
    const auto &target_pdev = physical_device_properties_map.find(pdev);
    if (target_pdev != physical_device_properties_map.end()) {
        auto effective_api_version = std::min(APIVersion(target_pdev->second->apiVersion), api_version);
        if (effective_api_version < promoted_version) {
            skip = LogError(
                kVUID_PVError_ApiVersionViolation, instance, loc,
                "Attempted to call with an effective API version of %s, "
                "which is the minimum of version requested in pApplicationInfo (%s) and supported by this physical device (%s), "
                "but this API was not promoted until version %s.",
                StringAPIVersion(effective_api_version).c_str(), StringAPIVersion(api_version).c_str(),
                StringAPIVersion(target_pdev->second->apiVersion).c_str(), StringAPIVersion(promoted_version).c_str());
        }
    }
    return skip;
}

bool StatelessValidation::OutputExtensionError(const Location &loc, const std::string &extension_name) const {
    return LogError(kVUID_PVError_ExtensionNotEnabled, instance, loc,
                    "function required extension %s which has not been enabled.\n", extension_name.c_str());
}

bool StatelessValidation::SupportedByPdev(const VkPhysicalDevice physical_device, const std::string &ext_name) const {
    if (instance_extensions.vk_khr_get_physical_device_properties2) {
        // Struct is legal IF it's supported
        const auto &dev_exts_enumerated = device_extensions_enumerated.find(physical_device);
        if (dev_exts_enumerated == device_extensions_enumerated.end()) return true;
        auto enum_iter = dev_exts_enumerated->second.find(ext_name);
        if (enum_iter != dev_exts_enumerated->second.cend()) {
            return true;
        }
    }
    return false;
}

static const int kMaxParamCheckerStringLength = 256;
bool StatelessValidation::ValidateString(const Location &loc, const std::string &vuid, const char *validateString) const {
    bool skip = false;

    VkStringErrorFlags result = vk_string_validate(kMaxParamCheckerStringLength, validateString);

    if (result == VK_STRING_ERROR_NONE) {
        return skip;
    } else if (result & VK_STRING_ERROR_LENGTH) {
        skip = LogError(vuid, device, loc, "exceeds max length %" PRIu32 ".", kMaxParamCheckerStringLength);
    } else if (result & VK_STRING_ERROR_BAD_DATA) {
        skip = LogError(vuid, device, loc, "contains invalid characters or is badly formed.");
    }
    return skip;
}

bool StatelessValidation::ValidateNotZero(bool is_zero, const std::string &vuid, const Location &loc) const {
    bool skip = false;
    if (is_zero) {
        skip |= LogError(vuid, device, loc, "is zero.");
    }
    return skip;
}

/**
 * Validate a required pointer.
 *
 * Verify that a required pointer is not NULL.
 *
 * @param loc Name of API call being validated.
 * @param value Pointer to validate.
 * @return Boolean value indicating that the call should be skipped.
 */
bool StatelessValidation::ValidateRequiredPointer(const Location &loc, const void *value, const std::string &vuid) const {
    bool skip = false;

    if (value == nullptr) {
        skip |= LogError(vuid, device, loc, "is NULL.");
    }

    return skip;
}

/**
 * Validate string array count and content.
 *
 * Verify that required count and array parameters are not 0 or NULL.  If the
 * count parameter is not optional, verify that it is not 0.  If the array
 * parameter is NULL, and it is not optional, verify that count is 0.  If the
 * array parameter is not NULL, verify that none of the strings are NULL.
 *
 * @param count_loc Name of count parameter.
 * @param array_loc Name of array parameter.
 * @param count Number of strings in the array.
 * @param array Array of strings to validate.
 * @param countRequired The 'count' parameter may not be 0 when true.
 * @param arrayRequired The 'array' parameter may not be NULL when true.
 * @return Boolean value indicating that the call should be skipped.
 */
bool StatelessValidation::ValidateStringArray(const Location &count_loc, const Location &array_loc, uint32_t count,
                                              const char *const *array, bool countRequired, bool arrayRequired,
                                              const char *count_required_vuid, const char *array_required_vuid) const {
    bool skip = false;

    if ((count == 0) || (array == nullptr)) {
        skip |= ValidateArray(count_loc, array_loc, count, &array, countRequired, arrayRequired, count_required_vuid,
                              array_required_vuid);
    } else {
        // Verify that strings in the array are not NULL
        for (uint32_t i = 0; i < count; ++i) {
            if (array[i] == nullptr) {
                skip |= LogError(array_required_vuid, device, array_loc.dot(i), "is NULL.");
            }
        }
    }

    return skip;
}

/**
 * Validate a structure's pNext member.
 *
 * Verify that the specified pNext value points to the head of a list of
 * allowed extension structures.  If no extension structures are allowed,
 * verify that pNext is null.
 *
 * @param loc Name of API call being validated.
 * @param next Pointer to validate.
 * @param allowed_type_count Total number of allowed structure types.
 * @param allowed_types Array of structure types allowed for pNext.
 * @param header_version Version of header defining the pNext validation rules.
 * @return Boolean value indicating that the call should be skipped.
 */
bool StatelessValidation::ValidateStructPnext(const Location &loc, const void *next, size_t allowed_type_count,
                                              const VkStructureType *allowed_types, uint32_t header_version, const char *pnext_vuid,
                                              const char *stype_vuid, const bool is_physdev_api, const bool is_const_param) const {
    bool skip = false;
    const Location pNext_loc = loc.dot(Field::pNext);
    const char *api_name = loc.StringFunc();

    if (next != nullptr) {
        vvl::unordered_set<const void *> cycle_check;
        vvl::unordered_set<VkStructureType, vvl::hash<int>> unique_stype_check;
        const char *disclaimer =
            "This error is based on the Valid Usage documentation for version %" PRIu32
            " of the Vulkan header.  It is possible that "
            "you are using a struct from a private extension or an extension that was added to a later version of the Vulkan "
            "header, in which case the use of %s is undefined and may not work correctly with validation enabled";

        if ((allowed_type_count == 0) && (custom_stype_info.size() == 0)) {
            std::string message = "must be NULL. ";
            message += disclaimer;
            skip |= LogError(pnext_vuid, device, pNext_loc, message.c_str(), header_version, pNext_loc.Fields().c_str());
        } else {
            const VkStructureType *start = allowed_types;
            const VkStructureType *end = allowed_types + allowed_type_count;
            const VkBaseOutStructure *current = reinterpret_cast<const VkBaseOutStructure *>(next);

            while (current != nullptr) {
                if (((strncmp(api_name, "vkCreateInstance", strlen(api_name)) != 0) ||
                     (current->sType != VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO)) &&
                    ((strncmp(api_name, "vkCreateDevice", strlen(api_name)) != 0) ||
                     (current->sType != VK_STRUCTURE_TYPE_LOADER_DEVICE_CREATE_INFO))) {
                    std::string type_name = string_VkStructureType(current->sType);
                    if (unique_stype_check.find(current->sType) != unique_stype_check.end() && !IsDuplicatePnext(current->sType)) {
                        // stype_vuid will only be null if there are no listed pNext and will hit disclaimer check
                        skip |= LogError(stype_vuid, device, pNext_loc,
                                         "chain contains duplicate structure types: %s appears multiple times.", type_name.c_str());
                    } else {
                        unique_stype_check.insert(current->sType);
                    }

                    // Search custom stype list -- if sType found, skip this entirely
                    bool custom = false;
                    for (const auto &item : custom_stype_info) {
                        if (item.first == current->sType) {
                            custom = true;
                            break;
                        }
                    }
                    if (!custom) {
                        if (std::find(start, end, current->sType) == end) {
                            if (type_name.compare(UnsupportedStructureTypeString) == 0) {
                                std::string message = "chain includes a structure with unknown VkStructureType (%" PRIu32 "). ";
                                message += disclaimer;
                                skip |= LogError(pnext_vuid, device, pNext_loc, message.c_str(), current->sType, header_version,
                                                 pNext_loc.Fields().c_str());
                            } else {
                                std::string message = "chain includes a structure with unexpected VkStructureType %s. ";
                                message += disclaimer;
                                skip |= LogError(pnext_vuid, device, pNext_loc, message.c_str(), type_name.c_str(), header_version,
                                                 pNext_loc.Fields().c_str());
                            }
                        }
                        // Send Location without pNext field so the pNext() connector can be used
                        skip |= ValidatePnextStructContents(loc, current, pnext_vuid, is_physdev_api, is_const_param);
                    }
                }
                current = reinterpret_cast<const VkBaseOutStructure *>(current->pNext);
            }
        }
    }

    return skip;
}

/**
 * Validate a VkBool32 value.
 *
 * Generate an error if a VkBool32 value is neither VK_TRUE nor VK_FALSE.
 *
 * @param loc Name of API call being validated.
 * @param value Boolean value to validate.
 * @return Boolean value indicating that the call should be skipped.
 */
bool StatelessValidation::ValidateBool32(const Location &loc, VkBool32 value) const {
    bool skip = false;
    if ((value != VK_TRUE) && (value != VK_FALSE)) {
        skip |= LogError(kVUID_PVError_UnrecognizedValue, device, loc,
                         "(%" PRIu32
                         ") is neither VK_TRUE nor VK_FALSE. Applications MUST not pass any other "
                         "values than VK_TRUE or VK_FALSE into a Vulkan implementation where a VkBool32 is expected.",
                         value);
    }
    return skip;
}

/**
 * Validate an array of VkBool32 values.
 *
 * Generate an error if any VkBool32 value in an array is neither VK_TRUE nor VK_FALSE.
 *
 * @param count_loc Name of count parameter.
 * @param array_loc Name of array parameter.
 * @param count Number of values in the array.
 * @param array Array of VkBool32 values to validate.
 * @param countRequired The 'count' parameter may not be 0 when true.
 * @param arrayRequired The 'array' parameter may not be NULL when true.
 * @return Boolean value indicating that the call should be skipped.
 */
bool StatelessValidation::ValidateBool32Array(const Location &count_loc, const Location &array_loc, uint32_t count,
                                              const VkBool32 *array, bool countRequired, bool arrayRequired) const {
    bool skip = false;

    if ((count == 0) || (array == nullptr)) {
        skip |= ValidateArray(count_loc, array_loc, count, &array, countRequired, arrayRequired, kVUIDUndefined, kVUIDUndefined);
    } else {
        for (uint32_t i = 0; i < count; ++i) {
            if ((array[i] != VK_TRUE) && (array[i] != VK_FALSE)) {
                skip |= LogError(kVUID_PVError_UnrecognizedValue, device, array_loc.dot(i),
                                 "(%" PRIu32
                                 ") is neither VK_TRUE nor VK_FALSE. Applications MUST not pass any other "
                                 "values than VK_TRUE or VK_FALSE into a Vulkan implementation where a VkBool32 is expected.",
                                 array[i]);
            }
        }
    }

    return skip;
}

/**
 * Verify that a reserved VkFlags value is zero.
 *
 * Verify that the specified value is zero, to check VkFlags values that are reserved for
 * future use.
 *
 * @param loc Name of API call being validated.
 * @param value Value to validate.
 * @return Boolean value indicating that the call should be skipped.
 */
bool StatelessValidation::ValidateReservedFlags(const Location &loc, VkFlags value, const char *vuid) const {
    bool skip = false;

    if (value != 0) {
        skip |= LogError(vuid, device, loc, "must be 0.");
    }

    return skip;
}

// helper to implement validation of both 32 bit and 64 bit flags.
template <typename FlagTypedef>
bool StatelessValidation::ValidateFlagsImplementation(const Location &loc, const char *flag_bits_name, FlagTypedef all_flags,
                                                      FlagTypedef value, const FlagType flag_type, const char *vuid,
                                                      const char *flags_zero_vuid) const {
    bool skip = false;

    if ((value & ~all_flags) != 0) {
        skip |= LogError(vuid, device, loc, "contains flag bits that are not recognized members of %s.", flag_bits_name);
    }

    const bool required = flag_type == kRequiredFlags || flag_type == kRequiredSingleBit;
    const char *zero_vuid = flag_type == kRequiredFlags ? flags_zero_vuid : vuid;
    if (required && value == 0) {
        skip |= LogError(zero_vuid, device, loc, "is zero.");
    }

    const auto HasMaxOneBitSet = [](const FlagTypedef f) {
        // Decrement flips bits from right upto first 1.
        // Rest stays same, and if there was any other 1s &ded together they would be non-zero. QED
        return f == 0 || !(f & (f - 1));
    };

    const bool is_bits_type = flag_type == kRequiredSingleBit || flag_type == kOptionalSingleBit;
    if (is_bits_type && !HasMaxOneBitSet(value)) {
        skip |= LogError(vuid, device, loc, "contains multiple members of %s when only a single value is allowed.", flag_bits_name);
    }

    return skip;
}

/**
 * Validate a 32 bit Vulkan bitmask value.
 *
 * Generate a warning if a value with a VkFlags derived type does not contain valid flag bits
 * for that type.
 *
 * @param loc Name of API call being validated.
 * @param flag_bits_name Name of the VkFlags type being validated.
 * @param all_flags A bit mask combining all valid flag bits for the VkFlags type being validated.
 * @param value VkFlags value to validate.
 * @param flag_type The type of flag, like optional, or single bit.
 * @param vuid VUID used for flag that is outside defined bits (or has more than one bit for Bits type).
 * @param flags_zero_vuid VUID used for non-optional Flags that are zero.
 * @return Boolean value indicating that the call should be skipped.
 */
bool StatelessValidation::ValidateFlags(const Location &loc, const char *flag_bits_name, VkFlags all_flags, VkFlags value,
                                        const FlagType flag_type, const char *vuid, const char *flags_zero_vuid) const {
    return ValidateFlagsImplementation<VkFlags>(loc, flag_bits_name, all_flags, value, flag_type, vuid, flags_zero_vuid);
}

/**
 * Validate a 64 bit Vulkan bitmask value.
 *
 * Generate a warning if a value with a VkFlags64 derived type does not contain valid flag bits
 * for that type.
 *
 * @param loc Name of API call being validated.
 * @param flag_bits_name Name of the VkFlags64 type being validated.
 * @param all_flags A bit mask combining all valid flag bits for the VkFlags64 type being validated.
 * @param value VkFlags64 value to validate.
 * @param flag_type The type of flag, like optional, or single bit.
 * @param vuid VUID used for flag that is outside defined bits (or has more than one bit for Bits type).
 * @param flags_zero_vuid VUID used for non-optional Flags that are zero.
 * @return Boolean value indicating that the call should be skipped.
 */
bool StatelessValidation::ValidateFlags(const Location &loc, const char *flag_bits_name, VkFlags64 all_flags, VkFlags64 value,
                                        const FlagType flag_type, const char *vuid, const char *flags_zero_vuid) const {
    return ValidateFlagsImplementation<VkFlags64>(loc, flag_bits_name, all_flags, value, flag_type, vuid, flags_zero_vuid);
}

/**
 * Validate an array of Vulkan bitmask values.
 *
 * Generate a warning if a value with a VkFlags derived type does not contain valid flag bits
 * for that type.
 *
 * @param count_loc Name of parameter being validated.
 * @param array_loc Name of parameter being validated.
 * @param flag_bits_name Name of the VkFlags type being validated.
 * @param all_flags A bitmask combining all valid flag bits for the VkFlags type being validated.
 * @param count Number of VkFlags values in the array.
 * @param array Array of VkFlags value to validate.
 * @param count_required The 'count' parameter may not be 0 when true.
 * @param array_required_vuid The VUID for the 'array' parameter.
 * @return Boolean value indicating that the call should be skipped.
 */
bool StatelessValidation::ValidateFlagsArray(const Location &count_loc, const Location &array_loc, const char *flag_bits_name,
                                             VkFlags all_flags, uint32_t count, const VkFlags *array, bool count_required,
                                             const char *array_required_vuid) const {
    bool skip = false;

    if (array == nullptr) {
        // Flag arrays always need to have a valid array
        skip |= ValidateArray(count_loc, array_loc, count, &array, count_required, true, kVUIDUndefined, array_required_vuid);
    } else {
        // Verify that all VkFlags values in the array
        for (uint32_t i = 0; i < count; ++i) {
            if ((array[i] & (~all_flags)) != 0) {
                skip |= LogError(kVUID_PVError_UnrecognizedValue, device, array_loc.dot(i),
                                 "contains flag bits that are not recognized members of %s.", flag_bits_name);
            }
        }
    }

    return skip;
}
