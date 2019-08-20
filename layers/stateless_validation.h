/* Copyright (c) 2015-2019 The Khronos Group Inc.
 * Copyright (c) 2015-2019 Valve Corporation
 * Copyright (c) 2015-2019 LunarG, Inc.
 * Copyright (C) 2015-2019 Google Inc.
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
 * Author: Dustin Graves <dustin@lunarg.com>
 * Author: Mark Lobodzinski <mark@lunarg.com>
 */

#pragma once

#include "parameter_name.h"
#include "vk_typemap_helper.h"

// Suppress unused warning on Linux
#if defined(__GNUC__)
#define DECORATE_UNUSED __attribute__((unused))
#else
#define DECORATE_UNUSED
#endif

static const char DECORATE_UNUSED *kVUID_PVError_NONE = "UNASSIGNED-GeneralParameterError-Info";
static const char DECORATE_UNUSED *kVUID_PVError_InvalidUsage = "UNASSIGNED-GeneralParameterError-InvalidUsage";
static const char DECORATE_UNUSED *kVUID_PVError_InvalidStructSType = "UNASSIGNED-GeneralParameterError-InvalidStructSType";
static const char DECORATE_UNUSED *kVUID_PVError_InvalidStructPNext = "UNASSIGNED-GeneralParameterError-InvalidStructPNext";
static const char DECORATE_UNUSED *kVUID_PVError_RequiredParameter = "UNASSIGNED-GeneralParameterError-RequiredParameter";
static const char DECORATE_UNUSED *kVUID_PVError_ReservedParameter = "UNASSIGNED-GeneralParameterError-ReservedParameter";
static const char DECORATE_UNUSED *kVUID_PVError_UnrecognizedValue = "UNASSIGNED-GeneralParameterError-UnrecognizedValue";
static const char DECORATE_UNUSED *kVUID_PVError_DeviceLimit = "UNASSIGNED-GeneralParameterError-DeviceLimit";
static const char DECORATE_UNUSED *kVUID_PVError_DeviceFeature = "UNASSIGNED-GeneralParameterError-DeviceFeature";
static const char DECORATE_UNUSED *kVUID_PVError_FailureCode = "UNASSIGNED-GeneralParameterError-FailureCode";
static const char DECORATE_UNUSED *kVUID_PVError_ExtensionNotEnabled = "UNASSIGNED-GeneralParameterError-ExtensionNotEnabled";
static const char DECORATE_UNUSED *kVUID_PVPerfWarn_SuboptimalSwapchain = "UNASSIGNED-GeneralParameterPerfWarn-SuboptimalSwapchain";

#undef DECORATE_UNUSED

extern const uint32_t GeneratedVulkanHeaderVersion;

extern const VkQueryPipelineStatisticFlags AllVkQueryPipelineStatisticFlagBits;
extern const VkColorComponentFlags AllVkColorComponentFlagBits;
extern const VkShaderStageFlags AllVkShaderStageFlagBits;
extern const VkQueryControlFlags AllVkQueryControlFlagBits;
extern const VkImageUsageFlags AllVkImageUsageFlagBits;
extern const VkSampleCountFlags AllVkSampleCountFlagBits;

extern const std::vector<VkCompareOp> AllVkCompareOpEnums;
extern const std::vector<VkStencilOp> AllVkStencilOpEnums;
extern const std::vector<VkBlendFactor> AllVkBlendFactorEnums;
extern const std::vector<VkBlendOp> AllVkBlendOpEnums;
extern const std::vector<VkLogicOp> AllVkLogicOpEnums;
extern const std::vector<VkBorderColor> AllVkBorderColorEnums;
extern const std::vector<VkImageLayout> AllVkImageLayoutEnums;
extern const std::vector<VkFormat> AllVkFormatEnums;
extern const std::vector<VkVertexInputRate> AllVkVertexInputRateEnums;
extern const std::vector<VkPrimitiveTopology> AllVkPrimitiveTopologyEnums;

// String returned by string_VkStructureType for an unrecognized type.
const std::string UnsupportedStructureTypeString = "Unhandled VkStructureType";

// String returned by string_VkResult for an unrecognized type.
const std::string UnsupportedResultString = "Unhandled VkResult";

// The base value used when computing the offset for an enumeration token value that is added by an extension.
// When validating enumeration tokens, any value >= to this value is considered to be provided by an extension.
// See Appendix C.10 "Assigning Extension Token Values" from the Vulkan specification
const uint32_t ExtEnumBaseValue = 1000000000;

// The value of all VK_xxx_MAX_ENUM tokens
const uint32_t MaxEnumValue = 0x7FFFFFFF;

// Misc parameters of log_msg that are likely constant per command (or low frequency change)
struct LogMiscParams {
    VkDebugReportObjectTypeEXT objectType;
    uint64_t srcObject;
    const char *api_name;
};

class StatelessValidation : public ValidationObject {
   public:
    VkPhysicalDeviceLimits device_limits = {};
    safe_VkPhysicalDeviceFeatures2 physical_device_features2;
    const VkPhysicalDeviceFeatures &physical_device_features = physical_device_features2.features;

    // Override chassis read/write locks for this validation object
    // This override takes a deferred lock. i.e. it is not acquired.
    std::unique_lock<std::mutex> write_lock() { return std::unique_lock<std::mutex>(validation_object_mutex, std::defer_lock); }

    // Device extension properties -- storing properties gathered from VkPhysicalDeviceProperties2KHR::pNext chain
    struct DeviceExtensionProperties {
        VkPhysicalDeviceShadingRateImagePropertiesNV shading_rate_image_props;
        VkPhysicalDeviceMeshShaderPropertiesNV mesh_shader_props;
        VkPhysicalDeviceRayTracingPropertiesNV ray_tracing_props;
    };
    DeviceExtensionProperties phys_dev_ext_props = {};

    struct SubpassesUsageStates {
        std::unordered_set<uint32_t> subpasses_using_color_attachment;
        std::unordered_set<uint32_t> subpasses_using_depthstencil_attachment;
    };

    // Though this validation object is predominantly statless, the Framebuffer checks are greatly simplified by creating and
    // updating a map of the renderpass usage states, and these accesses need thread protection. Use a mutex separate from the
    // parent object's to maintain that functionality.
    std::mutex renderpass_map_mutex;
    std::unordered_map<VkRenderPass, SubpassesUsageStates> renderpasses_states;

    // Constructor for stateles validation tracking
    // StatelessValidation() : {}
    /**
     * Validate a minimum value.
     *
     * Verify that the specified value is greater than the specified lower bound.
     *
     * @param api_name Name of API call being validated.
     * @param parameter_name Name of parameter being validated.
     * @param value Value to validate.
     * @param lower_bound Lower bound value to use for validation.
     * @return Boolean value indicating that the call should be skipped.
     */
    template <typename T>
    bool ValidateGreaterThan(const T value, const T lower_bound, const ParameterName &parameter_name, const std::string &vuid,
                             const LogMiscParams &misc) {
        bool skip_call = false;

        if (value <= lower_bound) {
            std::ostringstream ss;
            ss << misc.api_name << ": parameter " << parameter_name.get_name() << " (= " << value << ") is greater than "
               << lower_bound;
            skip_call |=
                log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, misc.objectType, misc.srcObject, vuid, "%s", ss.str().c_str());
        }

        return skip_call;
    }

    template <typename T>
    bool ValidateGreaterThanZero(const T value, const ParameterName &parameter_name, const std::string &vuid,
                                 const LogMiscParams &misc) {
        return ValidateGreaterThan(value, T{0}, parameter_name, vuid, misc);
    }
    /**
     * Validate a required pointer.
     *
     * Verify that a required pointer is not NULL.
     *
     * @param apiName Name of API call being validated.
     * @param parameterName Name of parameter being validated.
     * @param value Pointer to validate.
     * @return Boolean value indicating that the call should be skipped.
     */
    bool validate_required_pointer(const char *apiName, const ParameterName &parameterName, const void *value,
                                   const std::string &vuid) {
        bool skip_call = false;

        if (value == NULL) {
            skip_call |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, vuid,
                                 "%s: required parameter %s specified as NULL.", apiName, parameterName.get_name().c_str());
        }

        return skip_call;
    }

    /**
     * Validate array count and pointer to array.
     *
     * Verify that required count and array parameters are not 0 or NULL.  If the
     * count parameter is not optional, verify that it is not 0.  If the array
     * parameter is NULL, and it is not optional, verify that count is 0.
     *
     * @param apiName Name of API call being validated.
     * @param countName Name of count parameter.
     * @param arrayName Name of array parameter.
     * @param count Number of elements in the array.
     * @param array Array to validate.
     * @param countRequired The 'count' parameter may not be 0 when true.
     * @param arrayRequired The 'array' parameter may not be NULL when true.
     * @return Boolean value indicating that the call should be skipped.
     */
    template <typename T1, typename T2>
    bool validate_array(const char *apiName, const ParameterName &countName, const ParameterName &arrayName, T1 count,
                        const T2 *array, bool countRequired, bool arrayRequired, const char *count_required_vuid,
                        const char *array_required_vuid) {
        bool skip_call = false;

        // Count parameters not tagged as optional cannot be 0
        if (countRequired && (count == 0)) {
            skip_call |=
                log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, count_required_vuid,
                        "%s: parameter %s must be greater than 0.", apiName, countName.get_name().c_str());
        }

        // Array parameters not tagged as optional cannot be NULL, unless the count is 0
        if (arrayRequired && (count != 0) && (*array == NULL)) {
            skip_call |=
                log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, array_required_vuid,
                        "%s: required parameter %s specified as NULL.", apiName, arrayName.get_name().c_str());
        }

        return skip_call;
    }

    /**
     * Validate pointer to array count and pointer to array.
     *
     * Verify that required count and array parameters are not NULL.  If count
     * is not NULL and its value is not optional, verify that it is not 0.  If the
     * array parameter is NULL, and it is not optional, verify that count is 0.
     * The array parameter will typically be optional for this case (where count is
     * a pointer), allowing the caller to retrieve the available count.
     *
     * @param apiName Name of API call being validated.
     * @param countName Name of count parameter.
     * @param arrayName Name of array parameter.
     * @param count Pointer to the number of elements in the array.
     * @param array Array to validate.
     * @param countPtrRequired The 'count' parameter may not be NULL when true.
     * @param countValueRequired The '*count' value may not be 0 when true.
     * @param arrayRequired The 'array' parameter may not be NULL when true.
     * @return Boolean value indicating that the call should be skipped.
     */
    template <typename T1, typename T2>
    bool validate_array(const char *apiName, const ParameterName &countName, const ParameterName &arrayName, const T1 *count,
                        const T2 *array, bool countPtrRequired, bool countValueRequired, bool arrayRequired,
                        const char *count_required_vuid, const char *array_required_vuid) {
        bool skip_call = false;

        if (count == NULL) {
            if (countPtrRequired) {
                skip_call |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                     kVUID_PVError_RequiredParameter, "%s: required parameter %s specified as NULL", apiName,
                                     countName.get_name().c_str());
            }
        } else {
            skip_call |= validate_array(apiName, countName, arrayName, *array ? (*count) : 0, &array, countValueRequired,
                                        arrayRequired, count_required_vuid, array_required_vuid);
        }

        return skip_call;
    }

    /**
     * Validate a pointer to a Vulkan structure.
     *
     * Verify that a required pointer to a structure is not NULL.  If the pointer is
     * not NULL, verify that each structure's sType field is set to the correct
     * VkStructureType value.
     *
     * @param apiName Name of API call being validated.
     * @param parameterName Name of struct parameter being validated.
     * @param sTypeName Name of expected VkStructureType value.
     * @param value Pointer to the struct to validate.
     * @param sType VkStructureType for structure validation.
     * @param required The parameter may not be NULL when true.
     * @return Boolean value indicating that the call should be skipped.
     */
    template <typename T>
    bool validate_struct_type(const char *apiName, const ParameterName &parameterName, const char *sTypeName, const T *value,
                              VkStructureType sType, bool required, const char *struct_vuid, const char *stype_vuid) {
        bool skip_call = false;

        if (value == NULL) {
            if (required) {
                skip_call |=
                    log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, struct_vuid,
                            "%s: required parameter %s specified as NULL", apiName, parameterName.get_name().c_str());
            }
        } else if (value->sType != sType) {
            skip_call |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, stype_vuid,
                                 "%s: parameter %s->sType must be %s.", apiName, parameterName.get_name().c_str(), sTypeName);
        }

        return skip_call;
    }

    /**
     * Validate an array of Vulkan structures
     *
     * Verify that required count and array parameters are not 0 or NULL.  If
     * the array contains 1 or more structures, verify that each structure's
     * sType field is set to the correct VkStructureType value.
     *
     * @param apiName Name of API call being validated.
     * @param countName Name of count parameter.
     * @param arrayName Name of array parameter.
     * @param sTypeName Name of expected VkStructureType value.
     * @param count Number of elements in the array.
     * @param array Array to validate.
     * @param sType VkStructureType for structure validation.
     * @param countRequired The 'count' parameter may not be 0 when true.
     * @param arrayRequired The 'array' parameter may not be NULL when true.
     * @return Boolean value indicating that the call should be skipped.
     */
    template <typename T>
    bool validate_struct_type_array(const char *apiName, const ParameterName &countName, const ParameterName &arrayName,
                                    const char *sTypeName, uint32_t count, const T *array, VkStructureType sType,
                                    bool countRequired, bool arrayRequired, const char *stype_vuid, const char *param_vuid,
                                    const char *count_required_vuid) {
        bool skip_call = false;

        if ((count == 0) || (array == NULL)) {
            skip_call |= validate_array(apiName, countName, arrayName, count, &array, countRequired, arrayRequired,
                                        count_required_vuid, param_vuid);
        } else {
            // Verify that all structs in the array have the correct type
            for (uint32_t i = 0; i < count; ++i) {
                if (array[i].sType != sType) {
                    skip_call |=
                        log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, stype_vuid,
                                "%s: parameter %s[%d].sType must be %s", apiName, arrayName.get_name().c_str(), i, sTypeName);
                }
            }
        }

        return skip_call;
    }

    /**
     * Validate an array of Vulkan structures.
     *
     * Verify that required count and array parameters are not NULL.  If count
     * is not NULL and its value is not optional, verify that it is not 0.
     * If the array contains 1 or more structures, verify that each structure's
     * sType field is set to the correct VkStructureType value.
     *
     * @param apiName Name of API call being validated.
     * @param countName Name of count parameter.
     * @param arrayName Name of array parameter.
     * @param sTypeName Name of expected VkStructureType value.
     * @param count Pointer to the number of elements in the array.
     * @param array Array to validate.
     * @param sType VkStructureType for structure validation.
     * @param countPtrRequired The 'count' parameter may not be NULL when true.
     * @param countValueRequired The '*count' value may not be 0 when true.
     * @param arrayRequired The 'array' parameter may not be NULL when true.
     * @return Boolean value indicating that the call should be skipped.
     */
    template <typename T>
    bool validate_struct_type_array(const char *apiName, const ParameterName &countName, const ParameterName &arrayName,
                                    const char *sTypeName, uint32_t *count, const T *array, VkStructureType sType,
                                    bool countPtrRequired, bool countValueRequired, bool arrayRequired, const char *stype_vuid,
                                    const char *param_vuid, const char *count_required_vuid) {
        bool skip_call = false;

        if (count == NULL) {
            if (countPtrRequired) {
                skip_call |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                     kVUID_PVError_RequiredParameter, "%s: required parameter %s specified as NULL", apiName,
                                     countName.get_name().c_str());
            }
        } else {
            skip_call |= validate_struct_type_array(apiName, countName, arrayName, sTypeName, (*count), array, sType,
                                                    countValueRequired, arrayRequired, stype_vuid, param_vuid, count_required_vuid);
        }

        return skip_call;
    }

    /**
     * Validate a Vulkan handle.
     *
     * Verify that the specified handle is not VK_NULL_HANDLE.
     *
     * @param api_name Name of API call being validated.
     * @param parameter_name Name of struct parameter being validated.
     * @param value Handle to validate.
     * @return Boolean value indicating that the call should be skipped.
     */
    template <typename T>
    bool validate_required_handle(const char *api_name, const ParameterName &parameter_name, T value) {
        bool skip_call = false;

        if (value == VK_NULL_HANDLE) {
            skip_call |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                 kVUID_PVError_RequiredParameter, "%s: required parameter %s specified as VK_NULL_HANDLE", api_name,
                                 parameter_name.get_name().c_str());
        }

        return skip_call;
    }

    /**
     * Validate an array of Vulkan handles.
     *
     * Verify that required count and array parameters are not NULL.  If count
     * is not NULL and its value is not optional, verify that it is not 0.
     * If the array contains 1 or more handles, verify that no handle is set to
     * VK_NULL_HANDLE.
     *
     * @note This function is only intended to validate arrays of handles when none
     *       of the handles are allowed to be VK_NULL_HANDLE.  For arrays of handles
     *       that are allowed to contain VK_NULL_HANDLE, use validate_array() instead.
     *
     * @param api_name Name of API call being validated.
     * @param count_name Name of count parameter.
     * @param array_name Name of array parameter.
     * @param count Number of elements in the array.
     * @param array Array to validate.
     * @param count_required The 'count' parameter may not be 0 when true.
     * @param array_required The 'array' parameter may not be NULL when true.
     * @return Boolean value indicating that the call should be skipped.
     */
    template <typename T>
    bool validate_handle_array(const char *api_name, const ParameterName &count_name, const ParameterName &array_name,
                               uint32_t count, const T *array, bool count_required, bool array_required) {
        bool skip_call = false;

        if ((count == 0) || (array == NULL)) {
            skip_call |= validate_array(api_name, count_name, array_name, count, &array, count_required, array_required,
                                        kVUIDUndefined, kVUIDUndefined);
        } else {
            // Verify that no handles in the array are VK_NULL_HANDLE
            for (uint32_t i = 0; i < count; ++i) {
                if (array[i] == VK_NULL_HANDLE) {
                    skip_call |=
                        log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                kVUID_PVError_RequiredParameter, "%s: required parameter %s[%d] specified as VK_NULL_HANDLE",
                                api_name, array_name.get_name().c_str(), i);
                }
            }
        }

        return skip_call;
    }

    /**
     * Validate string array count and content.
     *
     * Verify that required count and array parameters are not 0 or NULL.  If the
     * count parameter is not optional, verify that it is not 0.  If the array
     * parameter is NULL, and it is not optional, verify that count is 0.  If the
     * array parameter is not NULL, verify that none of the strings are NULL.
     *
     * @param apiName Name of API call being validated.
     * @param countName Name of count parameter.
     * @param arrayName Name of array parameter.
     * @param count Number of strings in the array.
     * @param array Array of strings to validate.
     * @param countRequired The 'count' parameter may not be 0 when true.
     * @param arrayRequired The 'array' parameter may not be NULL when true.
     * @return Boolean value indicating that the call should be skipped.
     */
    bool validate_string_array(const char *apiName, const ParameterName &countName, const ParameterName &arrayName, uint32_t count,
                               const char *const *array, bool countRequired, bool arrayRequired, const char *count_required_vuid,
                               const char *array_required_vuid) {
        bool skip_call = false;

        if ((count == 0) || (array == NULL)) {
            skip_call |= validate_array(apiName, countName, arrayName, count, &array, countRequired, arrayRequired,
                                        count_required_vuid, array_required_vuid);
        } else {
            // Verify that strings in the array are not NULL
            for (uint32_t i = 0; i < count; ++i) {
                if (array[i] == NULL) {
                    skip_call |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                         kVUID_PVError_RequiredParameter, "%s: required parameter %s[%d] specified as NULL",
                                         apiName, arrayName.get_name().c_str(), i);
                }
            }
        }

        return skip_call;
    }

    // Forward declaration for pNext validation
    bool ValidatePnextStructContents(const char *api_name, const ParameterName &parameter_name, const VkBaseOutStructure *header);

    /**
     * Validate a structure's pNext member.
     *
     * Verify that the specified pNext value points to the head of a list of
     * allowed extension structures.  If no extension structures are allowed,
     * verify that pNext is null.
     *
     * @param api_name Name of API call being validated.
     * @param parameter_name Name of parameter being validated.
     * @param allowed_struct_names Names of allowed structs.
     * @param next Pointer to validate.
     * @param allowed_type_count Total number of allowed structure types.
     * @param allowed_types Array of structure types allowed for pNext.
     * @param header_version Version of header defining the pNext validation rules.
     * @return Boolean value indicating that the call should be skipped.
     */
    bool validate_struct_pnext(const char *api_name, const ParameterName &parameter_name, const char *allowed_struct_names,
                               const void *next, size_t allowed_type_count, const VkStructureType *allowed_types,
                               uint32_t header_version, const char *vuid) {
        bool skip_call = false;

        // TODO: The valid pNext structure types are not recursive. Each structure has its own list of valid sTypes for pNext.
        // Codegen a map of vectors containing the allowable pNext types for each struct and use that here -- also simplifies parms.
        if (next != NULL) {
            std::unordered_set<const void *> cycle_check;
            std::unordered_set<VkStructureType, std::hash<int>> unique_stype_check;

            const char *disclaimer =
                "This warning is based on the Valid Usage documentation for version %d of the Vulkan header.  It is possible that "
                "you "
                "are "
                "using a struct from a private extension or an extension that was added to a later version of the Vulkan header, "
                "in "
                "which "
                "case your use of %s is perfectly valid but is not guaranteed to work correctly with validation enabled";

            if (allowed_type_count == 0) {
                std::string message = "%s: value of %s must be NULL. ";
                message += disclaimer;
                skip_call |= log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, vuid,
                                     message.c_str(), api_name, parameter_name.get_name().c_str(), header_version,
                                     parameter_name.get_name().c_str());
            } else {
                const VkStructureType *start = allowed_types;
                const VkStructureType *end = allowed_types + allowed_type_count;
                const VkBaseOutStructure *current = reinterpret_cast<const VkBaseOutStructure *>(next);

                cycle_check.insert(next);

                while (current != NULL) {
                    if (((strncmp(api_name, "vkCreateInstance", strlen(api_name)) != 0) ||
                         (current->sType != VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO)) &&
                        ((strncmp(api_name, "vkCreateDevice", strlen(api_name)) != 0) ||
                         (current->sType != VK_STRUCTURE_TYPE_LOADER_DEVICE_CREATE_INFO))) {
                        if (cycle_check.find(current->pNext) != cycle_check.end()) {
                            std::string message = "%s: %s chain contains a cycle -- pNext pointer " PRIx64 " is repeated.";
                            skip_call |=
                                log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                        kVUID_PVError_InvalidStructPNext, message.c_str(), api_name,
                                        parameter_name.get_name().c_str(), reinterpret_cast<uint64_t>(next));
                            break;
                        } else {
                            cycle_check.insert(current->pNext);
                        }

                        std::string type_name = string_VkStructureType(current->sType);
                        if (unique_stype_check.find(current->sType) != unique_stype_check.end()) {
                            std::string message = "%s: %s chain contains duplicate structure types: %s appears multiple times.";
                            skip_call |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                                 VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, kVUID_PVError_InvalidStructPNext,
                                                 message.c_str(), api_name, parameter_name.get_name().c_str(), type_name.c_str());
                        } else {
                            unique_stype_check.insert(current->sType);
                        }

                        if (std::find(start, end, current->sType) == end) {
                            if (type_name == UnsupportedStructureTypeString) {
                                std::string message =
                                    "%s: %s chain includes a structure with unknown VkStructureType (%d); Allowed structures are "
                                    "[%s]. ";
                                message += disclaimer;
                                skip_call |=
                                    log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT,
                                            0, vuid, message.c_str(), api_name, parameter_name.get_name().c_str(), current->sType,
                                            allowed_struct_names, header_version, parameter_name.get_name().c_str());
                            } else {
                                std::string message =
                                    "%s: %s chain includes a structure with unexpected VkStructureType %s; Allowed structures are "
                                    "[%s]. ";
                                message += disclaimer;
                                skip_call |= log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT,
                                                     VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, vuid, message.c_str(), api_name,
                                                     parameter_name.get_name().c_str(), type_name.c_str(), allowed_struct_names,
                                                     header_version, parameter_name.get_name().c_str());
                            }
                        }
                        skip_call |= ValidatePnextStructContents(api_name, parameter_name, current);
                    }
                    current = reinterpret_cast<const VkBaseOutStructure *>(current->pNext);
                }
            }
        }

        return skip_call;
    }

    /**
     * Validate a VkBool32 value.
     *
     * Generate a warning if a VkBool32 value is neither VK_TRUE nor VK_FALSE.
     *
     * @param apiName Name of API call being validated.
     * @param parameterName Name of parameter being validated.
     * @param value Boolean value to validate.
     * @return Boolean value indicating that the call should be skipped.
     */
    bool validate_bool32(const char *apiName, const ParameterName &parameterName, VkBool32 value) {
        bool skip_call = false;

        if ((value != VK_TRUE) && (value != VK_FALSE)) {
            skip_call |= log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                 kVUID_PVError_UnrecognizedValue, "%s: value of %s (%d) is neither VK_TRUE nor VK_FALSE", apiName,
                                 parameterName.get_name().c_str(), value);
        }

        return skip_call;
    }

    /**
     * Validate a Vulkan enumeration value.
     *
     * Generate a warning if an enumeration token value does not fall within the core enumeration
     * begin and end token values, and was not added to the enumeration by an extension.  Extension
     * provided enumerations use the equation specified in Appendix C.10 of the Vulkan specification,
     * with 1,000,000,000 as the base token value.
     *
     * @note This function does not expect to process enumerations defining bitmask flag bits.
     *
     * @param apiName Name of API call being validated.
     * @param parameterName Name of parameter being validated.
     * @param enumName Name of the enumeration being validated.
     * @param valid_values The list of valid values for the enumeration.
     * @param value Enumeration value to validate.
     * @return Boolean value indicating that the call should be skipped.
     */
    template <typename T>
    bool validate_ranged_enum(const char *apiName, const ParameterName &parameterName, const char *enumName,
                              const std::vector<T> &valid_values, T value, const char *vuid) {
        bool skip = false;

        if (std::find(valid_values.begin(), valid_values.end(), value) == valid_values.end()) {
            skip |=
                log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, vuid,
                        "%s: value of %s (%d) does not fall within the begin..end range of the core %s enumeration tokens and is "
                        "not an extension added token.",
                        apiName, parameterName.get_name().c_str(), value, enumName);
        }

        return skip;
    }

    /**
     * Validate an array of Vulkan enumeration value.
     *
     * Process all enumeration token values in the specified array and generate a warning if a value
     * does not fall within the core enumeration begin and end token values, and was not added to
     * the enumeration by an extension.  Extension provided enumerations use the equation specified
     * in Appendix C.10 of the Vulkan specification, with 1,000,000,000 as the base token value.
     *
     * @note This function does not expect to process enumerations defining bitmask flag bits.
     *
     * @param apiName Name of API call being validated.
     * @param countName Name of count parameter.
     * @param arrayName Name of array parameter.
     * @param enumName Name of the enumeration being validated.
     * @param valid_values The list of valid values for the enumeration.
     * @param count Number of enumeration values in the array.
     * @param array Array of enumeration values to validate.
     * @param countRequired The 'count' parameter may not be 0 when true.
     * @param arrayRequired The 'array' parameter may not be NULL when true.
     * @return Boolean value indicating that the call should be skipped.
     */
    template <typename T>
    bool validate_ranged_enum_array(const char *apiName, const ParameterName &countName, const ParameterName &arrayName,
                                    const char *enumName, const std::vector<T> &valid_values, uint32_t count, const T *array,
                                    bool countRequired, bool arrayRequired) {
        bool skip_call = false;

        if ((count == 0) || (array == NULL)) {
            skip_call |= validate_array(apiName, countName, arrayName, count, &array, countRequired, arrayRequired, kVUIDUndefined,
                                        kVUIDUndefined);
        } else {
            for (uint32_t i = 0; i < count; ++i) {
                if (std::find(valid_values.begin(), valid_values.end(), array[i]) == valid_values.end()) {
                    skip_call |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                         kVUID_PVError_UnrecognizedValue,
                                         "%s: value of %s[%d] (%d) does not fall within the begin..end range of the core %s "
                                         "enumeration tokens and is not an extension added token",
                                         apiName, arrayName.get_name().c_str(), i, array[i], enumName);
                }
            }
        }

        return skip_call;
    }

    /**
     * Verify that a reserved VkFlags value is zero.
     *
     * Verify that the specified value is zero, to check VkFlags values that are reserved for
     * future use.
     *
     * @param api_name Name of API call being validated.
     * @param parameter_name Name of parameter being validated.
     * @param value Value to validate.
     * @return Boolean value indicating that the call should be skipped.
     */
    bool validate_reserved_flags(const char *api_name, const ParameterName &parameter_name, VkFlags value, const char *vuid) {
        bool skip_call = false;

        if (value != 0) {
            skip_call |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, vuid,
                                 "%s: parameter %s must be 0.", api_name, parameter_name.get_name().c_str());
        }

        return skip_call;
    }

    enum FlagType { kRequiredFlags, kOptionalFlags, kRequiredSingleBit, kOptionalSingleBit };

    /**
     * Validate a Vulkan bitmask value.
     *
     * Generate a warning if a value with a VkFlags derived type does not contain valid flag bits
     * for that type.
     *
     * @param api_name Name of API call being validated.
     * @param parameter_name Name of parameter being validated.
     * @param flag_bits_name Name of the VkFlags type being validated.
     * @param all_flags A bit mask combining all valid flag bits for the VkFlags type being validated.
     * @param value VkFlags value to validate.
     * @param flag_type The type of flag, like optional, or single bit.
     * @param vuid VUID used for flag that is outside defined bits (or has more than one bit for Bits type).
     * @param flags_zero_vuid VUID used for non-optional Flags that are zero.
     * @return Boolean value indicating that the call should be skipped.
     */
    bool validate_flags(const char *api_name, const ParameterName &parameter_name, const char *flag_bits_name, VkFlags all_flags,
                        VkFlags value, const FlagType flag_type, const char *vuid, const char *flags_zero_vuid = nullptr) {
        bool skip_call = false;

        if ((value & ~all_flags) != 0) {
            skip_call |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, vuid,
                                 "%s: value of %s contains flag bits that are not recognized members of %s", api_name,
                                 parameter_name.get_name().c_str(), flag_bits_name);
        }

        const bool required = flag_type == kRequiredFlags || flag_type == kRequiredSingleBit;
        const char *zero_vuid = flag_type == kRequiredFlags ? flags_zero_vuid : vuid;
        if (required && value == 0) {
            skip_call |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, zero_vuid,
                                 "%s: value of %s must not be 0.", api_name, parameter_name.get_name().c_str());
        }

        const auto HasMaxOneBitSet = [](const VkFlags f) {
            // Decrement flips bits from right upto first 1.
            // Rest stays same, and if there was any other 1s &ded together they would be non-zero. QED
            return f == 0 || !(f & (f - 1));
        };

        const bool is_bits_type = flag_type == kRequiredSingleBit || flag_type == kOptionalSingleBit;
        if (is_bits_type && !HasMaxOneBitSet(value)) {
            skip_call |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, vuid,
                                 "%s: value of %s contains multiple members of %s when only a single value is allowed", api_name,
                                 parameter_name.get_name().c_str(), flag_bits_name);
        }

        return skip_call;
    }

    /**
     * Validate an array of Vulkan bitmask values.
     *
     * Generate a warning if a value with a VkFlags derived type does not contain valid flag bits
     * for that type.
     *
     * @param api_name Name of API call being validated.
     * @param count_name Name of parameter being validated.
     * @param array_name Name of parameter being validated.
     * @param flag_bits_name Name of the VkFlags type being validated.
     * @param all_flags A bitmask combining all valid flag bits for the VkFlags type being validated.
     * @param count Number of VkFlags values in the array.
     * @param array Array of VkFlags value to validate.
     * @param count_required The 'count' parameter may not be 0 when true.
     * @param array_required The 'array' parameter may not be NULL when true.
     * @return Boolean value indicating that the call should be skipped.
     */
    bool validate_flags_array(const char *api_name, const ParameterName &count_name, const ParameterName &array_name,
                              const char *flag_bits_name, VkFlags all_flags, uint32_t count, const VkFlags *array,
                              bool count_required, bool array_required) {
        bool skip_call = false;

        if ((count == 0) || (array == NULL)) {
            skip_call |= validate_array(api_name, count_name, array_name, count, &array, count_required, array_required,
                                        kVUIDUndefined, kVUIDUndefined);
        } else {
            // Verify that all VkFlags values in the array
            for (uint32_t i = 0; i < count; ++i) {
                if (array[i] == 0) {
                    // Current XML registry logic for validity generation uses the array parameter's optional tag to determine if
                    // elements in the array are allowed be 0
                    if (array_required) {
                        skip_call |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                             kVUID_PVError_RequiredParameter, "%s: value of %s[%d] must not be 0", api_name,
                                             array_name.get_name().c_str(), i);
                    }
                } else if ((array[i] & (~all_flags)) != 0) {
                    skip_call |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                         kVUID_PVError_UnrecognizedValue,
                                         "%s: value of %s[%d] contains flag bits that are not recognized members of %s", api_name,
                                         array_name.get_name().c_str(), i, flag_bits_name);
                }
            }
        }

        return skip_call;
    }

    template <typename ExtensionState>
    bool validate_extension_reqs(const ExtensionState &extensions, const char *vuid, const char *extension_type,
                                 const char *extension_name) {
        bool skip = false;
        if (!extension_name) {
            return skip;  // Robust to invalid char *
        }
        auto info = ExtensionState::get_info(extension_name);

        if (!info.state) {
            return skip;  // Unknown extensions cannot be checked so report OK
        }

        // Check against the required list in the info
        std::vector<const char *> missing;
        for (const auto &req : info.requires) {
            if (!(extensions.*(req.enabled))) {
                missing.push_back(req.name);
            }
        }

        // Report any missing requirements
        if (missing.size()) {
            std::string missing_joined_list = string_join(", ", missing);
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT,
                            HandleToUint64(instance), vuid, "Missing extension%s required by the %s extension %s: %s.",
                            ((missing.size() > 1) ? "s" : ""), extension_type, extension_name, missing_joined_list.c_str());
        }
        return skip;
    }

    enum RenderPassCreateVersion { RENDER_PASS_VERSION_1 = 0, RENDER_PASS_VERSION_2 = 1 };

    template <typename RenderPassCreateInfoGeneric>
    bool ValidateSubpassGraphicsFlags(const debug_report_data *report_data, const RenderPassCreateInfoGeneric *pCreateInfo,
                                      uint32_t dependency_index, uint32_t subpass, VkPipelineStageFlags stages, const char *vuid,
                                      const char *target) {
        const VkPipelineStageFlags kCommonStages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT | VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        const VkPipelineStageFlags kFramebufferStages =
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
            VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        const VkPipelineStageFlags kPrimitiveShadingPipelineStages =
            kCommonStages | VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT | VK_PIPELINE_STAGE_VERTEX_INPUT_BIT |
            VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT |
            VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT | VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT |
            VK_PIPELINE_STAGE_TRANSFORM_FEEDBACK_BIT_EXT | VK_PIPELINE_STAGE_SHADING_RATE_IMAGE_BIT_NV | kFramebufferStages;
        const VkPipelineStageFlags kMeshShadingPipelineStages =
            kCommonStages | VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT | VK_PIPELINE_STAGE_TASK_SHADER_BIT_NV |
            VK_PIPELINE_STAGE_MESH_SHADER_BIT_NV | VK_PIPELINE_STAGE_SHADING_RATE_IMAGE_BIT_NV | kFramebufferStages;
        const VkPipelineStageFlags kFragmentDensityStages = VK_PIPELINE_STAGE_FRAGMENT_DENSITY_PROCESS_BIT_EXT;
        const VkPipelineStageFlags kConditionalRenderingStages = VK_PIPELINE_STAGE_CONDITIONAL_RENDERING_BIT_EXT;
        const VkPipelineStageFlags kCommandProcessingPipelineStages = kCommonStages | VK_PIPELINE_STAGE_COMMAND_PROCESS_BIT_NVX;

        const VkPipelineStageFlags kGraphicsStages = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT | kPrimitiveShadingPipelineStages |
                                                     kMeshShadingPipelineStages | kFragmentDensityStages |
                                                     kConditionalRenderingStages | kCommandProcessingPipelineStages;

        bool skip = false;

        const auto IsPipeline = [pCreateInfo](uint32_t subpass, const VkPipelineBindPoint stage) {
            if (subpass == VK_SUBPASS_EXTERNAL)
                return false;
            else
                return pCreateInfo->pSubpasses[subpass].pipelineBindPoint == stage;
        };

        const bool is_all_graphics_stages = (stages & ~kGraphicsStages) == 0;
        if (IsPipeline(subpass, VK_PIPELINE_BIND_POINT_GRAPHICS) && !is_all_graphics_stages) {
            skip |=
                log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT, 0, vuid,
                        "Dependency pDependencies[%" PRIu32
                        "] specifies a %sStageMask that contains stages (%s) that are not part "
                        "of the Graphics pipeline, as specified by the %sSubpass (= %" PRIu32 ") in pipelineBindPoint.",
                        dependency_index, target, string_VkPipelineStageFlags(stages & ~kGraphicsStages).c_str(), target, subpass);
        }

        return skip;
    };

    template <typename RenderPassCreateInfoGeneric>
    bool CreateRenderPassGeneric(VkDevice device, const RenderPassCreateInfoGeneric *pCreateInfo,
                                 const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass,
                                 RenderPassCreateVersion rp_version) {
        bool skip = false;
        uint32_t max_color_attachments = device_limits.maxColorAttachments;
        bool use_rp2 = (rp_version == RENDER_PASS_VERSION_2);
        const char *vuid;

        for (uint32_t i = 0; i < pCreateInfo->attachmentCount; ++i) {
            if (pCreateInfo->pAttachments[i].format == VK_FORMAT_UNDEFINED) {
                std::stringstream ss;
                ss << (use_rp2 ? "vkCreateRenderPass2KHR" : "vkCreateRenderPass") << ": pCreateInfo->pAttachments[" << i
                   << "].format is VK_FORMAT_UNDEFINED. ";
                vuid =
                    use_rp2 ? "VUID-VkAttachmentDescription2KHR-format-parameter" : "VUID-VkAttachmentDescription-format-parameter";
                skip |= log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, vuid,
                                "%s", ss.str().c_str());
            }
            if (pCreateInfo->pAttachments[i].finalLayout == VK_IMAGE_LAYOUT_UNDEFINED ||
                pCreateInfo->pAttachments[i].finalLayout == VK_IMAGE_LAYOUT_PREINITIALIZED) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2KHR-finalLayout-03061"
                               : "VUID-VkAttachmentDescription-finalLayout-00843";
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, vuid,
                                "pCreateInfo->pAttachments[%d].finalLayout must not be VK_IMAGE_LAYOUT_UNDEFINED or "
                                "VK_IMAGE_LAYOUT_PREINITIALIZED.",
                                i);
            }
        }

        for (uint32_t i = 0; i < pCreateInfo->subpassCount; ++i) {
            if (pCreateInfo->pSubpasses[i].colorAttachmentCount > max_color_attachments) {
                vuid = use_rp2 ? "VUID-VkSubpassDescription2KHR-colorAttachmentCount-03063"
                               : "VUID-VkSubpassDescription-colorAttachmentCount-00845";
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, vuid,
                                "Cannot create a render pass with %d color attachments. Max is %d.",
                                pCreateInfo->pSubpasses[i].colorAttachmentCount, max_color_attachments);
            }
        }

        for (uint32_t i = 0; i < pCreateInfo->dependencyCount; ++i) {
            const auto &dependency = pCreateInfo->pDependencies[i];

            // Spec currently only supports Graphics pipeline in render pass -- so only that pipeline is currently checked
            vuid =
                use_rp2 ? "VUID-VkRenderPassCreateInfo2KHR-pDependencies-03054" : "VUID-VkRenderPassCreateInfo-pDependencies-00837";
            skip |= ValidateSubpassGraphicsFlags(report_data, pCreateInfo, i, dependency.srcSubpass, dependency.srcStageMask, vuid,
                                                 "src");

            vuid =
                use_rp2 ? "VUID-VkRenderPassCreateInfo2KHR-pDependencies-03055" : "VUID-VkRenderPassCreateInfo-pDependencies-00838";
            skip |= ValidateSubpassGraphicsFlags(report_data, pCreateInfo, i, dependency.dstSubpass, dependency.dstStageMask, vuid,
                                                 "dst");
        }

        return skip;
    }

    template <typename T>
    void RecordRenderPass(VkRenderPass renderPass, const T *pCreateInfo) {
        std::unique_lock<std::mutex> lock(renderpass_map_mutex);
        auto &renderpass_state = renderpasses_states[renderPass];
        lock.unlock();

        for (uint32_t subpass = 0; subpass < pCreateInfo->subpassCount; ++subpass) {
            bool uses_color = false;
            for (uint32_t i = 0; i < pCreateInfo->pSubpasses[subpass].colorAttachmentCount && !uses_color; ++i)
                if (pCreateInfo->pSubpasses[subpass].pColorAttachments[i].attachment != VK_ATTACHMENT_UNUSED) uses_color = true;

            bool uses_depthstencil = false;
            if (pCreateInfo->pSubpasses[subpass].pDepthStencilAttachment)
                if (pCreateInfo->pSubpasses[subpass].pDepthStencilAttachment->attachment != VK_ATTACHMENT_UNUSED)
                    uses_depthstencil = true;

            if (uses_color) renderpass_state.subpasses_using_color_attachment.insert(subpass);
            if (uses_depthstencil) renderpass_state.subpasses_using_depthstencil_attachment.insert(subpass);
        }
    }

    bool require_device_extension(bool flag, char const *function_name, char const *extension_name);

    bool validate_instance_extensions(const VkInstanceCreateInfo *pCreateInfo);

    bool validate_api_version(uint32_t api_version, uint32_t effective_api_version);

    bool validate_string(const char *apiName, const ParameterName &stringName, const std::string &vuid, const char *validateString);

    bool ValidateCoarseSampleOrderCustomNV(const VkCoarseSampleOrderCustomNV *order);

    bool ValidateQueueFamilies(uint32_t queue_family_count, const uint32_t *queue_families, const char *cmd_name,
                               const char *array_parameter_name, const std::string &unique_error_code,
                               const std::string &valid_error_code, bool optional);

    bool ValidateDeviceQueueFamily(uint32_t queue_family, const char *cmd_name, const char *parameter_name,
                                   const std::string &error_code, bool optional);

    bool ValidateGeometryTrianglesNV(const VkGeometryTrianglesNV &triangles, VkDebugReportObjectTypeEXT object_type,
                                     uint64_t object_handle, const char *func_name) const;
    bool ValidateGeometryAABBNV(const VkGeometryAABBNV &geometry, VkDebugReportObjectTypeEXT object_type, uint64_t object_handle,
                                const char *func_name) const;
    bool ValidateGeometryNV(const VkGeometryNV &geometry, VkDebugReportObjectTypeEXT object_type, uint64_t object_handle,
                            const char *func_name) const;
    bool ValidateAccelerationStructureInfoNV(const VkAccelerationStructureInfoNV &info, VkDebugReportObjectTypeEXT object_type,
                                             uint64_t object_handle, const char *func_nam) const;

    bool OutputExtensionError(const std::string &api_name, const std::string &extension_name);

    void PostCallRecordCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo *pCreateInfo,
                                        const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass, VkResult result);
    void PostCallRecordCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2KHR *pCreateInfo,
                                            const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass, VkResult result);
    void PostCallRecordDestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks *pAllocator);
    void PostCallRecordCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo *pCreateInfo,
                                    const VkAllocationCallbacks *pAllocator, VkDevice *pDevice, VkResult result);

    void PostCallRecordCreateInstance(const VkInstanceCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator,
                                      VkInstance *pInstance, VkResult result);

    void PostCallRecordQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR *pPresentInfo, VkResult result);

    bool manual_PreCallValidateCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo *pCreateInfo,
                                               const VkAllocationCallbacks *pAllocator, VkQueryPool *pQueryPool);

    bool manual_PreCallValidateCreateInstance(const VkInstanceCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator,
                                              VkInstance *pInstance);

    bool manual_PreCallValidateCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo *pCreateInfo,
                                            const VkAllocationCallbacks *pAllocator, VkDevice *pDevice);

    bool manual_PreCallValidateCreateBuffer(VkDevice device, const VkBufferCreateInfo *pCreateInfo,
                                            const VkAllocationCallbacks *pAllocator, VkBuffer *pBuffer);

    bool manual_PreCallValidateCreateImage(VkDevice device, const VkImageCreateInfo *pCreateInfo,
                                           const VkAllocationCallbacks *pAllocator, VkImage *pImage);

    bool manual_PreCallValidateViewport(const VkViewport &viewport, const char *fn_name, const ParameterName &parameter_name,
                                        VkDebugReportObjectTypeEXT object_type, uint64_t object);

    bool manual_PreCallValidateCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                       const VkGraphicsPipelineCreateInfo *pCreateInfos,
                                                       const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines);
    bool manual_PreCallValidateCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                      const VkComputePipelineCreateInfo *pCreateInfos,
                                                      const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines);

    bool manual_PreCallValidateCreateSampler(VkDevice device, const VkSamplerCreateInfo *pCreateInfo,
                                             const VkAllocationCallbacks *pAllocator, VkSampler *pSampler);
    bool manual_PreCallValidateCreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
                                                         const VkAllocationCallbacks *pAllocator,
                                                         VkDescriptorSetLayout *pSetLayout);

    bool manual_PreCallValidateUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount,
                                                    const VkWriteDescriptorSet *pDescriptorWrites, uint32_t descriptorCopyCount,
                                                    const VkCopyDescriptorSet *pDescriptorCopies);

    bool manual_PreCallValidateFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t descriptorSetCount,
                                                  const VkDescriptorSet *pDescriptorSets);

    bool manual_PreCallValidateCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo *pCreateInfo,
                                                const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass);

    bool manual_PreCallValidateCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2KHR *pCreateInfo,
                                                    const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass);

    bool manual_PreCallValidateFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount,
                                                  const VkCommandBuffer *pCommandBuffers);

    bool manual_PreCallValidateBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo *pBeginInfo);

    bool manual_PreCallValidateCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                              const VkViewport *pViewports);

    bool manual_PreCallValidateCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount,
                                             const VkRect2D *pScissors);
    bool manual_PreCallValidateCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth);

    bool manual_PreCallValidateCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount,
                                       uint32_t firstVertex, uint32_t firstInstance);

    bool manual_PreCallValidateCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count,
                                               uint32_t stride);

    bool manual_PreCallValidateCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                      uint32_t count, uint32_t stride);

    bool manual_PreCallValidateCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                                   const VkClearAttachment *pAttachments, uint32_t rectCount,
                                                   const VkClearRect *pRects);

    bool manual_PreCallValidateCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                            VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                            const VkImageCopy *pRegions);

    bool manual_PreCallValidateCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                            VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                            const VkImageBlit *pRegions, VkFilter filter);

    bool manual_PreCallValidateCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                                    VkImageLayout dstImageLayout, uint32_t regionCount,
                                                    const VkBufferImageCopy *pRegions);

    bool manual_PreCallValidateCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                    VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy *pRegions);

    bool manual_PreCallValidateCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                               VkDeviceSize dataSize, const void *pData);

    bool manual_PreCallValidateCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                             VkDeviceSize size, uint32_t data);

    bool manual_PreCallValidateCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR *pCreateInfo,
                                                  const VkAllocationCallbacks *pAllocator, VkSwapchainKHR *pSwapchain);
    bool manual_PreCallValidateQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR *pPresentInfo);

#ifdef VK_USE_PLATFORM_WIN32_KHR
    bool manual_PreCallValidateCreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR *pCreateInfo,
                                                     const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface);
#endif  // VK_USE_PLATFORM_WIN32_KHR

    bool manual_PreCallValidateCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo *pCreateInfo,
                                                    const VkAllocationCallbacks *pAllocator, VkDescriptorPool *pDescriptorPool);
    bool manual_PreCallValidateCmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                           uint32_t groupCountZ);

    bool manual_PreCallValidateCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset);

    bool manual_PreCallValidateCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                                  uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY,
                                                  uint32_t groupCountZ);
    bool manual_PreCallValidateCmdSetExclusiveScissorNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                                        uint32_t exclusiveScissorCount, const VkRect2D *pExclusiveScissors);
    bool manual_PreCallValidateCmdSetViewportShadingRatePaletteNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                                  uint32_t viewportCount,
                                                                  const VkShadingRatePaletteNV *pShadingRatePalettes);

    bool manual_PreCallValidateCmdSetCoarseSampleOrderNV(VkCommandBuffer commandBuffer, VkCoarseSampleOrderTypeNV sampleOrderType,
                                                         uint32_t customSampleOrderCount,
                                                         const VkCoarseSampleOrderCustomNV *pCustomSampleOrders);

    bool manual_PreCallValidateCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask);
    bool manual_PreCallValidateCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                          uint32_t drawCount, uint32_t stride);

    bool manual_PreCallValidateCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                               VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                               uint32_t maxDrawCount, uint32_t stride);

    bool manual_PreCallValidateEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char *pLayerName,
                                                                  uint32_t *pPropertyCount, VkExtensionProperties *pProperties);
    bool manual_PreCallValidateAllocateMemory(VkDevice device, const VkMemoryAllocateInfo *pAllocateInfo,
                                              const VkAllocationCallbacks *pAllocator, VkDeviceMemory *pMemory);

    bool manual_PreCallValidateCreateAccelerationStructureNV(VkDevice device,
                                                             const VkAccelerationStructureCreateInfoNV *pCreateInfo,
                                                             const VkAllocationCallbacks *pAllocator,
                                                             VkAccelerationStructureNV *pAccelerationStructure);
    bool manual_PreCallValidateCmdBuildAccelerationStructureNV(VkCommandBuffer commandBuffer,
                                                               const VkAccelerationStructureInfoNV *pInfo, VkBuffer instanceData,
                                                               VkDeviceSize instanceOffset, VkBool32 update,
                                                               VkAccelerationStructureNV dst, VkAccelerationStructureNV src,
                                                               VkBuffer scratch, VkDeviceSize scratchOffset);
    bool manual_PreCallValidateGetAccelerationStructureHandleNV(VkDevice device, VkAccelerationStructureNV accelerationStructure,
                                                                size_t dataSize, void *pData);
    bool manual_PreCallValidateCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                           const VkRayTracingPipelineCreateInfoNV *pCreateInfos,
                                                           const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines);

#ifdef VK_USE_PLATFORM_WIN32_KHR
    bool PreCallValidateGetDeviceGroupSurfacePresentModes2EXT(VkDevice device, const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo,
                                                              VkDeviceGroupPresentModeFlagsKHR *pModes);
#endif  // VK_USE_PLATFORM_WIN32_KHR

    bool manual_PreCallValidateCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo *pCreateInfo,
                                                 const VkAllocationCallbacks *pAllocator, VkFramebuffer *pFramebuffer);

    bool manual_PreCallValidateCmdSetLineStippleEXT(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor,
                                                    uint16_t lineStipplePattern);

    bool manual_PreCallValidateCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                  VkIndexType indexType);

#include "parameter_validation.h"
};  // Class StatelessValidation
