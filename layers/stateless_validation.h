/* Copyright (c) 2015-2020 The Khronos Group Inc.
 * Copyright (c) 2015-2020 Valve Corporation
 * Copyright (c) 2015-2020 LunarG, Inc.
 * Copyright (C) 2015-2020 Google Inc.
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
static const char DECORATE_UNUSED *kVUID_PVError_ApiVersionViolation = "UNASSIGNED-API-Version-Violation";

extern const uint32_t GeneratedVulkanHeaderVersion;

extern const VkQueryPipelineStatisticFlags AllVkQueryPipelineStatisticFlagBits;
extern const VkColorComponentFlags AllVkColorComponentFlagBits;
extern const VkShaderStageFlags AllVkShaderStageFlagBits;
extern const VkQueryControlFlags AllVkQueryControlFlagBits;
extern const VkImageUsageFlags AllVkImageUsageFlagBits;
extern const VkSampleCountFlags AllVkSampleCountFlagBits;
extern const VkBuildAccelerationStructureFlagsNV AllVkBuildAccelerationStructureFlagBitsNV;
extern const VkGeometryFlagsKHR AllVkGeometryFlagBitsKHR;

extern const std::vector<VkGeometryTypeKHR> AllVkGeometryTypeKHREnums;
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
extern const std::vector<VkIndexType> AllVkIndexTypeEnums;

extern std::vector<std::pair<uint32_t, uint32_t>> custom_stype_info;

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

class StatelessValidation : public ValidationObject {
  public:
    VkPhysicalDeviceLimits device_limits = {};
    safe_VkPhysicalDeviceFeatures2 physical_device_features2;
    void *device_createinfo_pnext;
    const VkPhysicalDeviceFeatures &physical_device_features = physical_device_features2.features;
    std::unordered_map<VkPhysicalDevice, VkPhysicalDeviceProperties *> physical_device_properties_map;
    std::unordered_map<VkPhysicalDevice, std::unordered_set<std::string>> device_extensions_enumerated{};

    // Override chassis read/write locks for this validation object
    // This override takes a deferred lock. i.e. it is not acquired.
    virtual read_lock_guard_t read_lock() { return read_lock_guard_t(validation_object_mutex, std::defer_lock); }
    virtual write_lock_guard_t write_lock() { return write_lock_guard_t(validation_object_mutex, std::defer_lock); }

    // Device extension properties -- storing properties gathered from VkPhysicalDeviceProperties2KHR::pNext chain
    struct DeviceExtensionProperties {
        VkPhysicalDeviceShadingRateImagePropertiesNV shading_rate_image_props;
        VkPhysicalDeviceMeshShaderPropertiesNV mesh_shader_props;
        VkPhysicalDeviceRayTracingPropertiesNV ray_tracing_propsNV;
        VkPhysicalDeviceRayTracingPropertiesKHR ray_tracing_propsKHR;
        VkPhysicalDeviceTransformFeedbackPropertiesEXT transform_feedback_props;
    };
    DeviceExtensionProperties phys_dev_ext_props = {};

    struct SubpassesUsageStates {
        std::unordered_set<uint32_t> subpasses_using_color_attachment;
        std::unordered_set<uint32_t> subpasses_using_depthstencil_attachment;
    };

    // Though this validation object is predominantly statless, the Framebuffer checks are greatly simplified by creating and
    // updating a map of the renderpass usage states, and these accesses need thread protection. Use a mutex separate from the
    // parent object's to maintain that functionality.
    mutable std::mutex renderpass_map_mutex;
    std::unordered_map<VkRenderPass, SubpassesUsageStates> renderpasses_states;

    // Constructor for stateles validation tracking
    StatelessValidation() : device_createinfo_pnext(nullptr) { container_type = LayerObjectTypeParameterValidation; }
    ~StatelessValidation() {
        if (device_createinfo_pnext) {
            FreePnextChain(device_createinfo_pnext);
        }
    }

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
                             const char *api_name) const {
        bool skip_call = false;

        if (value <= lower_bound) {
            std::ostringstream ss;
            ss << api_name << ": parameter " << parameter_name.get_name() << " (= " << value << ") is greater than " << lower_bound;
            skip_call |= LogError(device, vuid, "%s", ss.str().c_str());
        }

        return skip_call;
    }

    template <typename T>
    bool ValidateGreaterThanZero(const T value, const ParameterName &parameter_name, const std::string &vuid,
                                 const char *api_name) const {
        return ValidateGreaterThan(value, T{0}, parameter_name, vuid, api_name);
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
                                   const std::string &vuid) const {
        bool skip_call = false;

        if (value == NULL) {
            skip_call |=
                LogError(device, vuid, "%s: required parameter %s specified as NULL.", apiName, parameterName.get_name().c_str());
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
                        const char *array_required_vuid) const {
        bool skip_call = false;

        // Count parameters not tagged as optional cannot be 0
        if (countRequired && (count == 0)) {
            skip_call |= LogError(device, count_required_vuid, "%s: parameter %s must be greater than 0.", apiName,
                                  countName.get_name().c_str());
        }

        // Array parameters not tagged as optional cannot be NULL, unless the count is 0
        if (arrayRequired && (count != 0) && (*array == NULL)) {
            skip_call |= LogError(device, array_required_vuid, "%s: required parameter %s specified as NULL.", apiName,
                                  arrayName.get_name().c_str());
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
                        const char *count_required_vuid, const char *array_required_vuid) const {
        bool skip_call = false;

        if (count == NULL) {
            if (countPtrRequired) {
                skip_call |= LogError(device, kVUID_PVError_RequiredParameter, "%s: required parameter %s specified as NULL",
                                      apiName, countName.get_name().c_str());
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
                              VkStructureType sType, bool required, const char *struct_vuid, const char *stype_vuid) const {
        bool skip_call = false;

        if (value == NULL) {
            if (required) {
                skip_call |= LogError(device, struct_vuid, "%s: required parameter %s specified as NULL", apiName,
                                      parameterName.get_name().c_str());
            }
        } else if (value->sType != sType) {
            skip_call |= LogError(device, stype_vuid, "%s: parameter %s->sType must be %s.", apiName,
                                  parameterName.get_name().c_str(), sTypeName);
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
                                    const char *count_required_vuid) const {
        bool skip_call = false;

        if ((count == 0) || (array == NULL)) {
            skip_call |= validate_array(apiName, countName, arrayName, count, &array, countRequired, arrayRequired,
                                        count_required_vuid, param_vuid);
        } else {
            // Verify that all structs in the array have the correct type
            for (uint32_t i = 0; i < count; ++i) {
                if (array[i].sType != sType) {
                    skip_call |= LogError(device, stype_vuid, "%s: parameter %s[%d].sType must be %s", apiName,
                                          arrayName.get_name().c_str(), i, sTypeName);
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
                                    const char *param_vuid, const char *count_required_vuid) const {
        bool skip_call = false;

        if (count == NULL) {
            if (countPtrRequired) {
                skip_call |= LogError(device, kVUID_PVError_RequiredParameter, "%s: required parameter %s specified as NULL",
                                      apiName, countName.get_name().c_str());
            }
        } else {
            skip_call |= validate_struct_type_array(apiName, countName, arrayName, sTypeName, (*count), array, sType,
                                                    countValueRequired && (array != nullptr), arrayRequired, stype_vuid, param_vuid,
                                                    count_required_vuid);
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
    bool validate_required_handle(const char *api_name, const ParameterName &parameter_name, T value) const {
        bool skip_call = false;

        if (value == VK_NULL_HANDLE) {
            skip_call |= LogError(device, kVUID_PVError_RequiredParameter, "%s: required parameter %s specified as VK_NULL_HANDLE",
                                  api_name, parameter_name.get_name().c_str());
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
                               uint32_t count, const T *array, bool count_required, bool array_required) const {
        bool skip_call = false;

        if ((count == 0) || (array == NULL)) {
            skip_call |= validate_array(api_name, count_name, array_name, count, &array, count_required, array_required,
                                        kVUIDUndefined, kVUIDUndefined);
        } else {
            // Verify that no handles in the array are VK_NULL_HANDLE
            for (uint32_t i = 0; i < count; ++i) {
                if (array[i] == VK_NULL_HANDLE) {
                    skip_call |= LogError(device, kVUID_PVError_RequiredParameter,
                                          "%s: required parameter %s[%d] specified as VK_NULL_HANDLE", api_name,
                                          array_name.get_name().c_str(), i);
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
                               const char *array_required_vuid) const {
        bool skip_call = false;

        if ((count == 0) || (array == NULL)) {
            skip_call |= validate_array(apiName, countName, arrayName, count, &array, countRequired, arrayRequired,
                                        count_required_vuid, array_required_vuid);
        } else {
            // Verify that strings in the array are not NULL
            for (uint32_t i = 0; i < count; ++i) {
                if (array[i] == NULL) {
                    skip_call |=
                        LogError(device, kVUID_PVError_RequiredParameter, "%s: required parameter %s[%d] specified as NULL",
                                 apiName, arrayName.get_name().c_str(), i);
                }
            }
        }

        return skip_call;
    }

    // Forward declarations
    bool CheckPromotedApiAgainstVulkanVersion(VkInstance instance, const char *api_name, const uint32_t promoted_version) const;
    bool CheckPromotedApiAgainstVulkanVersion(VkPhysicalDevice pdev, const char *api_name, const uint32_t promoted_version) const;
    bool SupportedByPdev(const VkPhysicalDevice physical_device, const std::string ext_name) const;

    bool ValidatePnextStructContents(const char *api_name, const ParameterName &parameter_name, const VkBaseOutStructure *header,
                                     const char *pnext_vuid) const;

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
                               uint32_t header_version, const char *pnext_vuid, const char *stype_vuid) const {
        bool skip_call = false;

        if (next != NULL) {
            std::unordered_set<const void *> cycle_check;
            std::unordered_set<VkStructureType, std::hash<int>> unique_stype_check;

            const char *disclaimer =
                "This error is based on the Valid Usage documentation for version %d of the Vulkan header.  It is possible that "
                "you are using a struct from a private extension or an extension that was added to a later version of the Vulkan "
                "header, in which case the use of %s is undefined and may not work correctly with validation enabled";

            if ((allowed_type_count == 0) && (custom_stype_info.size() == 0)) {
                std::string message = "%s: value of %s must be NULL. ";
                message += disclaimer;
                skip_call |= LogError(device, pnext_vuid, message.c_str(), api_name, parameter_name.get_name().c_str(),
                                      header_version, parameter_name.get_name().c_str());
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
                            std::string message = "%s: %s chain contains a cycle -- pNext pointer %" PRIx64 " is repeated.";
                            skip_call |= LogError(device, kVUID_PVError_InvalidStructPNext, message.c_str(), api_name,
                                                  parameter_name.get_name().c_str(), reinterpret_cast<uint64_t>(next));
                            break;
                        } else {
                            cycle_check.insert(current->pNext);
                        }

                        std::string type_name = string_VkStructureType(current->sType);
                        if (unique_stype_check.find(current->sType) != unique_stype_check.end()) {
                            // stype_vuid will only be null if there are no listed pNext and will hit disclaimer check
                            std::string message = "%s: %s chain contains duplicate structure types: %s appears multiple times.";
                            skip_call |= LogError(device, stype_vuid, message.c_str(), api_name, parameter_name.get_name().c_str(),
                                                  type_name.c_str());
                        } else {
                            unique_stype_check.insert(current->sType);
                        }

                        // Search custom stype list -- if sType found, skip this entirely
                        bool custom = false;
                        for (auto item : custom_stype_info) {
                            if (item.first == current->sType) {
                                custom = true;
                                break;
                            }
                        }
                        if (!custom) {
                            if (std::find(start, end, current->sType) == end) {
                                if (type_name.compare(UnsupportedStructureTypeString) == 0) {
                                    std::string message =
                                        "%s: %s chain includes a structure with unknown VkStructureType (%d); Allowed structures "
                                        "are [%s]. ";
                                    message += disclaimer;
                                    skip_call |= LogError(device, pnext_vuid, message.c_str(), api_name,
                                                          parameter_name.get_name().c_str(), current->sType, allowed_struct_names,
                                                          header_version, parameter_name.get_name().c_str());
                                } else {
                                    std::string message =
                                        "%s: %s chain includes a structure with unexpected VkStructureType %s; Allowed structures "
                                        "are [%s]. ";
                                    message += disclaimer;
                                    skip_call |= LogError(device, pnext_vuid, message.c_str(), api_name,
                                                          parameter_name.get_name().c_str(), type_name.c_str(),
                                                          allowed_struct_names, header_version, parameter_name.get_name().c_str());
                                }
                            }
                            skip_call |= ValidatePnextStructContents(api_name, parameter_name, current, pnext_vuid);
                        }
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
    bool validate_bool32(const char *apiName, const ParameterName &parameterName, VkBool32 value) const {
        bool skip_call = false;

        if ((value != VK_TRUE) && (value != VK_FALSE)) {
            skip_call |= LogWarning(device, kVUID_PVError_UnrecognizedValue, "%s: value of %s (%d) is neither VK_TRUE nor VK_FALSE",
                                    apiName, parameterName.get_name().c_str(), value);
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
                              const std::vector<T> &valid_values, T value, const char *vuid) const {
        bool skip = false;

        if (std::find(valid_values.begin(), valid_values.end(), value) == valid_values.end()) {
            skip |=
                LogError(device, vuid,
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
                                    bool countRequired, bool arrayRequired) const {
        bool skip_call = false;

        if ((count == 0) || (array == NULL)) {
            skip_call |= validate_array(apiName, countName, arrayName, count, &array, countRequired, arrayRequired, kVUIDUndefined,
                                        kVUIDUndefined);
        } else {
            for (uint32_t i = 0; i < count; ++i) {
                if (std::find(valid_values.begin(), valid_values.end(), array[i]) == valid_values.end()) {
                    skip_call |= LogError(device, kVUID_PVError_UnrecognizedValue,
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
    bool validate_reserved_flags(const char *api_name, const ParameterName &parameter_name, VkFlags value, const char *vuid) const {
        bool skip_call = false;

        if (value != 0) {
            skip_call |= LogError(device, vuid, "%s: parameter %s must be 0.", api_name, parameter_name.get_name().c_str());
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
                        VkFlags value, const FlagType flag_type, const char *vuid, const char *flags_zero_vuid = nullptr) const {
        bool skip_call = false;

        if ((value & ~all_flags) != 0) {
            skip_call |= LogError(device, vuid, "%s: value of %s contains flag bits that are not recognized members of %s",
                                  api_name, parameter_name.get_name().c_str(), flag_bits_name);
        }

        const bool required = flag_type == kRequiredFlags || flag_type == kRequiredSingleBit;
        const char *zero_vuid = flag_type == kRequiredFlags ? flags_zero_vuid : vuid;
        if (required && value == 0) {
            skip_call |= LogError(device, zero_vuid, "%s: value of %s must not be 0.", api_name, parameter_name.get_name().c_str());
        }

        const auto HasMaxOneBitSet = [](const VkFlags f) {
            // Decrement flips bits from right upto first 1.
            // Rest stays same, and if there was any other 1s &ded together they would be non-zero. QED
            return f == 0 || !(f & (f - 1));
        };

        const bool is_bits_type = flag_type == kRequiredSingleBit || flag_type == kOptionalSingleBit;
        if (is_bits_type && !HasMaxOneBitSet(value)) {
            skip_call |=
                LogError(device, vuid, "%s: value of %s contains multiple members of %s when only a single value is allowed",
                         api_name, parameter_name.get_name().c_str(), flag_bits_name);
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
                              bool count_required, bool array_required) const {
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
                        skip_call |= LogError(device, kVUID_PVError_RequiredParameter, "%s: value of %s[%d] must not be 0",
                                              api_name, array_name.get_name().c_str(), i);
                    }
                } else if ((array[i] & (~all_flags)) != 0) {
                    skip_call |= LogError(device, kVUID_PVError_UnrecognizedValue,
                                          "%s: value of %s[%d] contains flag bits that are not recognized members of %s", api_name,
                                          array_name.get_name().c_str(), i, flag_bits_name);
                }
            }
        }

        return skip_call;
    }

    template <typename ExtensionState>
    bool validate_extension_reqs(const ExtensionState &extensions, const char *vuid, const char *extension_type,
                                 const char *extension_name) const {
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
        for (const auto &req : info.requirements) {
            if (!(extensions.*(req.enabled))) {
                missing.push_back(req.name);
            }
        }

        // Report any missing requirements
        if (missing.size()) {
            std::string missing_joined_list = string_join(", ", missing);
            skip |= LogError(instance, vuid, "Missing extension%s required by the %s extension %s: %s.",
                             ((missing.size() > 1) ? "s" : ""), extension_type, extension_name, missing_joined_list.c_str());
        }
        return skip;
    }

    enum RenderPassCreateVersion { RENDER_PASS_VERSION_1 = 0, RENDER_PASS_VERSION_2 = 1 };

    template <typename RenderPassCreateInfoGeneric>
    bool ValidateSubpassGraphicsFlags(const debug_report_data *report_data, const RenderPassCreateInfoGeneric *pCreateInfo,
                                      uint32_t dependency_index, uint32_t subpass, VkPipelineStageFlags stages, const char *vuid,
                                      const char *target, const char *func_name) const {
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
        const VkPipelineStageFlags kCommandProcessingPipelineStages = kCommonStages | VK_PIPELINE_STAGE_COMMAND_PREPROCESS_BIT_NV;

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
            skip |= LogError(VkRenderPass(0), vuid,
                             "%s: Dependency pDependencies[%" PRIu32
                             "] specifies a %sStageMask that contains stages (%s) that are not part "
                             "of the Graphics pipeline, as specified by the %sSubpass (= %" PRIu32 ") in pipelineBindPoint.",
                             func_name, dependency_index, target, string_VkPipelineStageFlags(stages & ~kGraphicsStages).c_str(),
                             target, subpass);
        }

        return skip;
    };

    template <typename RenderPassCreateInfoGeneric>
    bool CreateRenderPassGeneric(VkDevice device, const RenderPassCreateInfoGeneric *pCreateInfo,
                                 const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass,
                                 RenderPassCreateVersion rp_version) const {
        bool skip = false;
        uint32_t max_color_attachments = device_limits.maxColorAttachments;
        bool use_rp2 = (rp_version == RENDER_PASS_VERSION_2);
        const char *func_name = (use_rp2) ? "vkCreateRenderPass2" : "vkCreateRenderPass";
        const char *vuid;
        VkBool32 separate_depth_stencil_layouts = false;
        const auto *vulkan_12_features = lvl_find_in_chain<VkPhysicalDeviceVulkan12Features>(device_createinfo_pnext);
        if (vulkan_12_features) {
            separate_depth_stencil_layouts = vulkan_12_features->separateDepthStencilLayouts;
        } else {
            const auto *separate_depth_stencil_layouts_features =
                lvl_find_in_chain<VkPhysicalDeviceSeparateDepthStencilLayoutsFeaturesKHR>(device_createinfo_pnext);
            if (separate_depth_stencil_layouts_features)
                separate_depth_stencil_layouts = separate_depth_stencil_layouts_features->separateDepthStencilLayouts;
        }

        for (uint32_t i = 0; i < pCreateInfo->attachmentCount; ++i) {
            const auto *attachment_description_stencil_layout =
                (use_rp2) ? lvl_find_in_chain<VkAttachmentDescriptionStencilLayoutKHR>(
                                reinterpret_cast<VkAttachmentDescription2KHR const *>(&pCreateInfo->pAttachments[i])->pNext)
                          : 0;

            const VkFormat attachment_format = pCreateInfo->pAttachments[i].format;
            const VkImageLayout initial_layout = pCreateInfo->pAttachments[i].initialLayout;
            const VkImageLayout final_layout = pCreateInfo->pAttachments[i].finalLayout;
            if (attachment_format == VK_FORMAT_UNDEFINED) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-format-parameter" : "VUID-VkAttachmentDescription-format-parameter";
                skip |= LogWarning(device, vuid, "%s: pCreateInfo->pAttachments[%u].format is VK_FORMAT_UNDEFINED.", func_name, i);
            }
            if (final_layout == VK_IMAGE_LAYOUT_UNDEFINED || final_layout == VK_IMAGE_LAYOUT_PREINITIALIZED) {
                vuid =
                    use_rp2 ? "VUID-VkAttachmentDescription2-finalLayout-03061" : "VUID-VkAttachmentDescription-finalLayout-00843";
                skip |= LogError(device, vuid,
                                 "%s: pCreateInfo->pAttachments[%d].finalLayout must not be VK_IMAGE_LAYOUT_UNDEFINED or "
                                 "VK_IMAGE_LAYOUT_PREINITIALIZED.",
                                 func_name, i);
            }
            if (!separate_depth_stencil_layouts) {
                if (pCreateInfo->pAttachments[i].initialLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL_KHR ||
                    pCreateInfo->pAttachments[i].initialLayout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL_KHR ||
                    pCreateInfo->pAttachments[i].initialLayout == VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL_KHR ||
                    pCreateInfo->pAttachments[i].initialLayout == VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL_KHR) {
                    vuid = use_rp2 ? "VUID-VkAttachmentDescription2-separateDepthStencilLayouts-03298"
                                   : "VUID-VkAttachmentDescription-separateDepthStencilLayouts-03284";
                    skip |= LogError(
                        device, vuid,
                        "%s: pCreateInfo->pAttachments[%d].initialLayout must not be VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL_KHR, "
                        "VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL_KHR, VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL_KHR, or "
                        "VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL_KHR",
                        func_name, i);
                }
                if (final_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL_KHR ||
                    final_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL_KHR ||
                    final_layout == VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL_KHR ||
                    final_layout == VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL_KHR) {
                    vuid = use_rp2 ? "VUID-VkAttachmentDescription2-separateDepthStencilLayouts-03299"
                                   : "VUID-VkAttachmentDescription-separateDepthStencilLayouts-03285";
                    skip |= LogError(
                        device, vuid,
                        "%s: pCreateInfo->pAttachments[%d].finalLayout must not be VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL_KHR, "
                        "VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL_KHR, VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL_KHR, or "
                        "VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL_KHR",
                        func_name, i);
                }
            }
            if (!FormatIsDepthOrStencil(attachment_format)) {
                if (initial_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL_KHR ||
                    initial_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL_KHR ||
                    initial_layout == VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL_KHR ||
                    initial_layout == VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL_KHR) {
                    vuid = use_rp2 ? "VUID-VkAttachmentDescription2-format-03300" : "VUID-VkAttachmentDescription-format-03286";
                    skip |= LogError(
                        device, vuid,
                        "%s: pCreateInfo->pAttachments[%d].initialLayout must not be VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL_KHR, "
                        "VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL_KHR, VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL_KHR, or "
                        "VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMA_KHRL",
                        func_name, i);
                }
                if (final_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL_KHR ||
                    final_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL_KHR ||
                    final_layout == VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL_KHR ||
                    final_layout == VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL_KHR) {
                    vuid = use_rp2 ? "VUID-VkAttachmentDescription2-format-03301" : "VUID-VkAttachmentDescription-format-03287";
                    skip |= LogError(
                        device, vuid,
                        "%s: pCreateInfo->pAttachments[%d].finalLayout must not be VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL_KHR, "
                        "VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL_KHR, VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL_KHR, or "
                        "VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL_KHR",
                        func_name, i);
                }
            } else if (FormatIsDepthAndStencil(attachment_format)) {
                if (use_rp2) {
                    if (!attachment_description_stencil_layout) {
                        if (initial_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL_KHR ||
                            initial_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL_KHR) {
                            skip |= LogError(
                                device, "VUID-VkAttachmentDescription2-format-03302",
                                "%s: pCreateInfo->pNext must include an instance of VkAttachmentDescriptionStencilLayoutKHR",
                                func_name);
                        }
                        if (final_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL_KHR ||
                            final_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL_KHR) {
                            skip |= LogError(
                                device, "VUID-VkAttachmentDescription2-format-03303",
                                "%s: pCreateInfo->pNext must include an instance of VkAttachmentDescriptionStencilLayoutKHR",
                                func_name);
                        }
                    }
                } else {
                    if (initial_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL_KHR ||
                        initial_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL_KHR ||
                        initial_layout == VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL_KHR ||
                        initial_layout == VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL_KHR) {
                        skip |= LogError(
                            device, "VUID-VkAttachmentDescription-format-03288",
                            "%s: pCreateInfo->pAttachments[%d].initialLayout must not be "
                            "VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL_KHR, "
                            "VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL_KHR, VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL_KHR, or "
                            "VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL_KHR",
                            func_name, i);
                    }
                    if (final_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL_KHR ||
                        final_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL_KHR ||
                        final_layout == VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL_KHR ||
                        final_layout == VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL_KHR) {
                        skip |= LogError(
                            device, "VUID-VkAttachmentDescription-format-03289",
                            "%s: pCreateInfo->pAttachments[%d].finalLayout must not be "
                            "VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL_KHR, "
                            "VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL_KHR, VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL_KHR, or "
                            "VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL_KHR",
                            func_name, i);
                    }
                }
            } else if (FormatIsDepthOnly(attachment_format)) {
                if (initial_layout == VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL_KHR ||
                    initial_layout == VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL_KHR) {
                    vuid = use_rp2 ? "VUID-VkAttachmentDescription2-format-03304" : "VUID-VkAttachmentDescription-format-03290";
                    skip |= LogError(device, vuid,
                                     "%s: pCreateInfo->pAttachments[%d].initialLayout must not be "
                                     "VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL_KHR, or"
                                     "VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL_KHR",
                                     func_name, i);
                }
                if (final_layout == VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL_KHR ||
                    final_layout == VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL_KHR) {
                    vuid = use_rp2 ? "VUID-VkAttachmentDescription2-format-03305" : "VUID-VkAttachmentDescription-format-03291";
                    skip |= LogError(device, vuid,
                                     "%s: pCreateInfo->pAttachments[%d].finalLayout must not be "
                                     "VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL_KHR, or "
                                     "VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL_KHR",
                                     func_name, i);
                }
            } else if (FormatIsStencilOnly(attachment_format)) {
                if (initial_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL_KHR ||
                    initial_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL_KHR) {
                    vuid = use_rp2 ? "VUID-VkAttachmentDescription2-format-03306" : "VUID-VkAttachmentDescription-format-03292";
                    skip |= LogError(device, vuid,
                                     "%s: pCreateInfo->pAttachments[%d].initialLayout must not be "
                                     "VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL_KHR, or"
                                     "VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL_KHR",
                                     func_name, i);
                }
                if (final_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL_KHR ||
                    final_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL_KHR) {
                    vuid = use_rp2 ? "VUID-VkAttachmentDescription2-format-03307" : "VUID-VkAttachmentDescription-format-03293";
                    skip |= LogError(device, vuid,
                                     "%s: pCreateInfo->pAttachments[%d].finalLayout must not be "
                                     "VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL_KHR, or "
                                     "VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMA_KHRL",
                                     func_name, i);
                }
            }
            if (use_rp2 && attachment_description_stencil_layout) {
                if (attachment_description_stencil_layout->stencilInitialLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL ||
                    attachment_description_stencil_layout->stencilInitialLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL_KHR ||
                    attachment_description_stencil_layout->stencilInitialLayout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL_KHR ||
                    attachment_description_stencil_layout->stencilInitialLayout ==
                        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
                    attachment_description_stencil_layout->stencilInitialLayout ==
                        VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL ||
                    attachment_description_stencil_layout->stencilInitialLayout ==
                        VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL ||
                    attachment_description_stencil_layout->stencilInitialLayout ==
                        VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL) {
                    skip |= LogError(device, "VUID-VkAttachmentDescriptionStencilLayout-stencilInitialLayout-03308",
                                     "%s: VkAttachmentDescriptionStencilLayoutKHR.stencilInitialLayout must not be "
                                     "VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, "
                                     "VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL_KHR, VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL_KHR, "
                                     "VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, "
                                     "VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, "
                                     "VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL, or "
                                     "VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL.",
                                     func_name);
                }
                if (attachment_description_stencil_layout->stencilFinalLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL ||
                    attachment_description_stencil_layout->stencilFinalLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL_KHR ||
                    attachment_description_stencil_layout->stencilFinalLayout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL_KHR ||
                    attachment_description_stencil_layout->stencilFinalLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
                    attachment_description_stencil_layout->stencilFinalLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL ||
                    attachment_description_stencil_layout->stencilFinalLayout ==
                        VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL ||
                    attachment_description_stencil_layout->stencilFinalLayout ==
                        VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL) {
                    skip |= LogError(device, "VUID-VkAttachmentDescriptionStencilLayout-stencilFinalLayout-03309",
                                     "%s: VkAttachmentDescriptionStencilLayoutKHR.stencilFinalLayout must not be "
                                     "VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, "
                                     "VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL_KHR, VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL_KHR, "
                                     "VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, "
                                     "VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, "
                                     "VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL, or "
                                     "VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL.",
                                     func_name);
                }
                if (attachment_description_stencil_layout->stencilFinalLayout == VK_IMAGE_LAYOUT_UNDEFINED ||
                    attachment_description_stencil_layout->stencilFinalLayout == VK_IMAGE_LAYOUT_PREINITIALIZED) {
                    skip |= LogError(
                        device, "VUID-VkAttachmentDescriptionStencilLayout-stencilFinalLayout-03310",
                        "%s: VkAttachmentDescriptionStencilLayoutKHR.stencilFinalLayout must not be VK_IMAGE_LAYOUT_UNDEFINED, or "
                        "VK_IMAGE_LAYOUT_PREINITIALIZED.",
                        func_name);
                }
            }

            if (FormatIsDepthOrStencil(attachment_format)) {
                if (initial_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
                    vuid = use_rp2 ? "VUID-VkAttachmentDescription2-format-03295" : "VUID-VkAttachmentDescription-format-03281";
                    skip |= LogError(device, vuid,
                                     "%s: pCreateInfo->pAttachments[%d].initialLayout must not be "
                                     "VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL when using a Depth or Stencil format",
                                     func_name, i);
                }
                if (final_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
                    vuid = use_rp2 ? "VUID-VkAttachmentDescription2-format-03297" : "VUID-VkAttachmentDescription-format-03283";
                    skip |= LogError(device, vuid,
                                     "%s: pCreateInfo->pAttachments[%d].finalLayout must not be "
                                     "VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL when using a Depth or Stencil format",
                                     func_name, i);
                }
            }
            if (FormatIsColor(attachment_format)) {
                if (initial_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
                    initial_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL ||
                    initial_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL ||
                    initial_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL) {
                    vuid = use_rp2 ? "VUID-VkAttachmentDescription2-format-03294" : "VUID-VkAttachmentDescription-format-03280";
                    skip |= LogError(device, vuid,
                                     "%s: pCreateInfo->pAttachments[%d].initialLayout must not be "
                                     "VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, "
                                     "VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, "
                                     "VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL, or "
                                     "VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL when using a Color format",
                                     func_name, i);
                }
                if (final_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
                    final_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL ||
                    final_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL ||
                    final_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL) {
                    vuid = use_rp2 ? "VUID-VkAttachmentDescription2-format-03296" : "VUID-VkAttachmentDescription-format-03282";
                    skip |= LogError(device, vuid,
                                     "%s: pCreateInfo->pAttachments[%d].finalLayout must not be "
                                     "VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, "
                                     "VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, "
                                     "VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL, or "
                                     "VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL when using a Color format",
                                     func_name, i);
                }
            }
        }

        for (uint32_t i = 0; i < pCreateInfo->subpassCount; ++i) {
            if (pCreateInfo->pSubpasses[i].colorAttachmentCount > max_color_attachments) {
                vuid = use_rp2 ? "VUID-VkSubpassDescription2-colorAttachmentCount-03063"
                               : "VUID-VkSubpassDescription-colorAttachmentCount-00845";
                skip |=
                    LogError(device, vuid,
                             "%s: Cannot create a render pass with %d color attachments in pCreateInfo->pSubpasses[%u]. Max is %d.",
                             func_name, pCreateInfo->pSubpasses[i].colorAttachmentCount, i, max_color_attachments);
            }
        }

        for (uint32_t i = 0; i < pCreateInfo->dependencyCount; ++i) {
            const auto &dependency = pCreateInfo->pDependencies[i];

            // Need to check first so layer doesn't segfault from out of bound array access
            // src subpass bound check
            if ((dependency.srcSubpass != VK_SUBPASS_EXTERNAL) && (dependency.srcSubpass >= pCreateInfo->subpassCount)) {
                vuid = use_rp2 ? "VUID-VkRenderPassCreateInfo2-srcSubpass-02526" : "VUID-VkRenderPassCreateInfo-srcSubpass-02517";
                skip |= LogError(device, vuid,
                                 "%s: pCreateInfo->pDependencies[%u].srcSubpass index (%u) has to be less than subpassCount (%u)",
                                 func_name, i, dependency.srcSubpass, pCreateInfo->subpassCount);
            }

            // dst subpass bound check
            if ((dependency.dstSubpass != VK_SUBPASS_EXTERNAL) && (dependency.dstSubpass >= pCreateInfo->subpassCount)) {
                vuid = use_rp2 ? "VUID-VkRenderPassCreateInfo2-dstSubpass-02527" : "VUID-VkRenderPassCreateInfo-dstSubpass-02518";
                skip |= LogError(device, vuid,
                                 "%s: pCreateInfo->pDependencies[%u].dstSubpass index (%u) has to be less than subpassCount (%u)",
                                 func_name, i, dependency.dstSubpass, pCreateInfo->subpassCount);
            }

            // Spec currently only supports Graphics pipeline in render pass -- so only that pipeline is currently checked
            vuid = use_rp2 ? "VUID-VkRenderPassCreateInfo2-pDependencies-03054" : "VUID-VkRenderPassCreateInfo-pDependencies-00837";
            skip |= ValidateSubpassGraphicsFlags(report_data, pCreateInfo, i, dependency.srcSubpass, dependency.srcStageMask, vuid,
                                                 "src", func_name);

            vuid = use_rp2 ? "VUID-VkRenderPassCreateInfo2-pDependencies-03055" : "VUID-VkRenderPassCreateInfo-pDependencies-00838";
            skip |= ValidateSubpassGraphicsFlags(report_data, pCreateInfo, i, dependency.dstSubpass, dependency.dstStageMask, vuid,
                                                 "dst", func_name);
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

    bool require_device_extension(bool flag, char const *function_name, char const *extension_name) const;

    bool validate_instance_extensions(const VkInstanceCreateInfo *pCreateInfo) const;

    bool validate_validation_features(const VkInstanceCreateInfo *pCreateInfo,
                                      const VkValidationFeaturesEXT *validation_features) const;

    bool validate_api_version(uint32_t api_version, uint32_t effective_api_version) const;

    bool validate_string(const char *apiName, const ParameterName &stringName, const std::string &vuid,
                         const char *validateString) const;

    bool ValidateCoarseSampleOrderCustomNV(const VkCoarseSampleOrderCustomNV *order) const;

    bool ValidateQueueFamilies(uint32_t queue_family_count, const uint32_t *queue_families, const char *cmd_name,
                               const char *array_parameter_name, const std::string &unique_error_code,
                               const std::string &valid_error_code, bool optional);

    bool ValidateDeviceQueueFamily(uint32_t queue_family, const char *cmd_name, const char *parameter_name,
                                   const std::string &error_code, bool optional);

    bool ValidateGeometryTrianglesNV(const VkGeometryTrianglesNV &triangles, VkAccelerationStructureNV object_handle,
                                     const char *func_name) const;
    bool ValidateGeometryAABBNV(const VkGeometryAABBNV &geometry, VkAccelerationStructureNV object_handle,
                                const char *func_name) const;
    bool ValidateGeometryNV(const VkGeometryNV &geometry, VkAccelerationStructureNV object_handle, const char *func_name) const;
    bool ValidateAccelerationStructureInfoNV(const VkAccelerationStructureInfoNV &info, VkAccelerationStructureNV object_handle,
                                             const char *func_nam, bool is_cmd) const;
    bool ValidateCreateSamplerYcbcrConversion(VkDevice device, const VkSamplerYcbcrConversionCreateInfo *pCreateInfo,
                                              const VkAllocationCallbacks *pAllocator, VkSamplerYcbcrConversion *pYcbcrConversion,
                                              const char *apiName) const;
    bool ValidateCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkDeviceSize offset, VkDeviceSize countBufferOffset,
                                      bool khr) const;
    bool ValidateCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkDeviceSize offset, VkDeviceSize countBufferOffset,
                                             bool khr) const;

    bool OutputExtensionError(const std::string &api_name, const std::string &extension_name) const;

    void PostCallRecordCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo *pCreateInfo,
                                        const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass, VkResult result);
    void PostCallRecordCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2KHR *pCreateInfo,
                                            const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass, VkResult result);
    void PostCallRecordDestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks *pAllocator);
    void PostCallRecordCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo *pCreateInfo,
                                    const VkAllocationCallbacks *pAllocator, VkDevice *pDevice, VkResult result);

    void PostCallRecordCreateInstance(const VkInstanceCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator,
                                      VkInstance *pInstance, VkResult result);

    void PreCallRecordDestroyInstance(VkInstance instance, const VkAllocationCallbacks *pAllocator);

    bool manual_PreCallValidateCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo *pCreateInfo,
                                               const VkAllocationCallbacks *pAllocator, VkQueryPool *pQueryPool) const;

    bool manual_PreCallValidateCreateInstance(const VkInstanceCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator,
                                              VkInstance *pInstance) const;

    bool manual_PreCallValidateCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo *pCreateInfo,
                                            const VkAllocationCallbacks *pAllocator, VkDevice *pDevice) const;

    bool manual_PreCallValidateCreateBuffer(VkDevice device, const VkBufferCreateInfo *pCreateInfo,
                                            const VkAllocationCallbacks *pAllocator, VkBuffer *pBuffer) const;

    bool manual_PreCallValidateCreateImage(VkDevice device, const VkImageCreateInfo *pCreateInfo,
                                           const VkAllocationCallbacks *pAllocator, VkImage *pImage) const;

    bool manual_PreCallValidateCreateImageView(VkDevice device, const VkImageViewCreateInfo *pCreateInfo,
                                               const VkAllocationCallbacks *pAllocator, VkImageView *pView) const;

    bool manual_PreCallValidateViewport(const VkViewport &viewport, const char *fn_name, const ParameterName &parameter_name,
                                        VkCommandBuffer object) const;

    bool manual_PreCallValidateCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                       const VkGraphicsPipelineCreateInfo *pCreateInfos,
                                                       const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines) const;
    bool manual_PreCallValidateCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                      const VkComputePipelineCreateInfo *pCreateInfos,
                                                      const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines) const;

    bool manual_PreCallValidateCreateSampler(VkDevice device, const VkSamplerCreateInfo *pCreateInfo,
                                             const VkAllocationCallbacks *pAllocator, VkSampler *pSampler) const;
    bool manual_PreCallValidateCreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
                                                         const VkAllocationCallbacks *pAllocator,
                                                         VkDescriptorSetLayout *pSetLayout) const;

    bool validate_WriteDescriptorSet(const char *vkCallingFunction, const uint32_t descriptorWriteCount,
                                     const VkWriteDescriptorSet *pDescriptorWrites, const bool validateDstSet = true) const;
    bool manual_PreCallValidateUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount,
                                                    const VkWriteDescriptorSet *pDescriptorWrites, uint32_t descriptorCopyCount,
                                                    const VkCopyDescriptorSet *pDescriptorCopies) const;

    bool manual_PreCallValidateFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t descriptorSetCount,
                                                  const VkDescriptorSet *pDescriptorSets) const;

    bool manual_PreCallValidateCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo *pCreateInfo,
                                                const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass) const;

    bool manual_PreCallValidateCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2KHR *pCreateInfo,
                                                    const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass) const;

    bool manual_PreCallValidateFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount,
                                                  const VkCommandBuffer *pCommandBuffers) const;

    bool manual_PreCallValidateBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo *pBeginInfo) const;

    bool manual_PreCallValidateCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                              const VkViewport *pViewports) const;

    bool manual_PreCallValidateCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount,
                                             const VkRect2D *pScissors) const;
    bool manual_PreCallValidateCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth) const;

    bool manual_PreCallValidateCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count,
                                               uint32_t stride) const;

    bool manual_PreCallValidateCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                      uint32_t count, uint32_t stride) const;

    bool manual_PreCallValidateCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                    VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                    uint32_t stride) const;

    bool manual_PreCallValidateCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                       uint32_t stride) const;

    bool manual_PreCallValidateCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                           VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                           uint32_t maxDrawCount, uint32_t stride) const;

    bool manual_PreCallValidateCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                              VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                              uint32_t maxDrawCount, uint32_t stride) const;

    bool manual_PreCallValidateCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                                   const VkClearAttachment *pAttachments, uint32_t rectCount,
                                                   const VkClearRect *pRects) const;

    bool ValidateGetPhysicalDeviceImageFormatProperties2(VkPhysicalDevice physicalDevice,
                                                         const VkPhysicalDeviceImageFormatInfo2 *pImageFormatInfo,
                                                         VkImageFormatProperties2 *pImageFormatProperties,
                                                         const char *apiName) const;
    bool manual_PreCallValidateGetPhysicalDeviceImageFormatProperties2(VkPhysicalDevice physicalDevice,
                                                                       const VkPhysicalDeviceImageFormatInfo2 *pImageFormatInfo,
                                                                       VkImageFormatProperties2 *pImageFormatProperties) const;
    bool manual_PreCallValidateGetPhysicalDeviceImageFormatProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                          const VkPhysicalDeviceImageFormatInfo2 *pImageFormatInfo,
                                                                          VkImageFormatProperties2 *pImageFormatProperties) const;

    bool manual_PreCallValidateCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                            VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                            const VkImageCopy *pRegions) const;

    bool manual_PreCallValidateCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                            VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                            const VkImageBlit *pRegions, VkFilter filter) const;

    bool manual_PreCallValidateCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer,
                                             uint32_t regionCount, const VkBufferCopy *pRegions) const;

    bool manual_PreCallValidateCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                                    VkImageLayout dstImageLayout, uint32_t regionCount,
                                                    const VkBufferImageCopy *pRegions) const;

    bool manual_PreCallValidateCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                    VkBuffer dstBuffer, uint32_t regionCount,
                                                    const VkBufferImageCopy *pRegions) const;

    bool manual_PreCallValidateCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                               VkDeviceSize dataSize, const void *pData) const;

    bool manual_PreCallValidateCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                             VkDeviceSize size, uint32_t data) const;

    bool manual_PreCallValidateCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR *pCreateInfo,
                                                  const VkAllocationCallbacks *pAllocator, VkSwapchainKHR *pSwapchain) const;
    bool manual_PreCallValidateQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR *pPresentInfo) const;

#ifdef VK_USE_PLATFORM_WIN32_KHR
    bool manual_PreCallValidateCreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR *pCreateInfo,
                                                     const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface) const;
#endif  // VK_USE_PLATFORM_WIN32_KHR

    bool manual_PreCallValidateCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo *pCreateInfo,
                                                    const VkAllocationCallbacks *pAllocator,
                                                    VkDescriptorPool *pDescriptorPool) const;
    bool manual_PreCallValidateCmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                           uint32_t groupCountZ) const;

    bool manual_PreCallValidateCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset) const;

    bool manual_PreCallValidateCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                                  uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY,
                                                  uint32_t groupCountZ) const;
    bool manual_PreCallValidateCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                       VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                                       const VkWriteDescriptorSet *pDescriptorWrites) const;
    bool manual_PreCallValidateCmdSetExclusiveScissorNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                                        uint32_t exclusiveScissorCount, const VkRect2D *pExclusiveScissors) const;
    bool manual_PreCallValidateCmdSetViewportShadingRatePaletteNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                                  uint32_t viewportCount,
                                                                  const VkShadingRatePaletteNV *pShadingRatePalettes) const;

    bool manual_PreCallValidateCmdSetCoarseSampleOrderNV(VkCommandBuffer commandBuffer, VkCoarseSampleOrderTypeNV sampleOrderType,
                                                         uint32_t customSampleOrderCount,
                                                         const VkCoarseSampleOrderCustomNV *pCustomSampleOrders) const;

    bool manual_PreCallValidateCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask) const;
    bool manual_PreCallValidateCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                          uint32_t drawCount, uint32_t stride) const;

    bool manual_PreCallValidateCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                               VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                               uint32_t maxDrawCount, uint32_t stride) const;

    bool manual_PreCallValidateEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char *pLayerName,
                                                                  uint32_t *pPropertyCount,
                                                                  VkExtensionProperties *pProperties) const;
    bool manual_PreCallValidateAllocateMemory(VkDevice device, const VkMemoryAllocateInfo *pAllocateInfo,
                                              const VkAllocationCallbacks *pAllocator, VkDeviceMemory *pMemory) const;

    bool manual_PreCallValidateCreateAccelerationStructureNV(VkDevice device,
                                                             const VkAccelerationStructureCreateInfoNV *pCreateInfo,
                                                             const VkAllocationCallbacks *pAllocator,
                                                             VkAccelerationStructureNV *pAccelerationStructure) const;
    bool manual_PreCallValidateCreateAccelerationStructureKHR(VkDevice device,
                                                              const VkAccelerationStructureCreateInfoKHR *pCreateInfo,
                                                              const VkAllocationCallbacks *pAllocator,
                                                              VkAccelerationStructureKHR *pAccelerationStructure) const;
    bool manual_PreCallValidateCmdBuildAccelerationStructureNV(VkCommandBuffer commandBuffer,
                                                               const VkAccelerationStructureInfoNV *pInfo, VkBuffer instanceData,
                                                               VkDeviceSize instanceOffset, VkBool32 update,
                                                               VkAccelerationStructureNV dst, VkAccelerationStructureNV src,
                                                               VkBuffer scratch, VkDeviceSize scratchOffset) const;
    bool manual_PreCallValidateGetAccelerationStructureHandleNV(VkDevice device, VkAccelerationStructureNV accelerationStructure,
                                                                size_t dataSize, void *pData) const;
    bool manual_PreCallValidateCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                           const VkRayTracingPipelineCreateInfoNV *pCreateInfos,
                                                           const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines) const;
    bool manual_PreCallValidateCreateRayTracingPipelinesKHR(VkDevice device, VkPipelineCache pipelineCache,
                                                            uint32_t createInfoCount,
                                                            const VkRayTracingPipelineCreateInfoKHR *pCreateInfos,
                                                            const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines) const;
    bool manual_PreCallValidateCmdSetViewportWScalingNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                        uint32_t viewportCount,
                                                        const VkViewportWScalingNV *pViewportWScalings) const;

#ifdef VK_USE_PLATFORM_WIN32_KHR
    bool PreCallValidateGetDeviceGroupSurfacePresentModes2EXT(VkDevice device, const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo,
                                                              VkDeviceGroupPresentModeFlagsKHR *pModes) const;
#endif  // VK_USE_PLATFORM_WIN32_KHR

    bool manual_PreCallValidateCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo *pCreateInfo,
                                                 const VkAllocationCallbacks *pAllocator, VkFramebuffer *pFramebuffer) const;

    bool manual_PreCallValidateCmdSetLineStippleEXT(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor,
                                                    uint16_t lineStipplePattern) const;

    bool manual_PreCallValidateCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                  VkIndexType indexType) const;
    bool manual_PreCallValidateCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                                    const VkBuffer *pBuffers, const VkDeviceSize *pOffsets) const;

    bool manual_PreCallValidateSetDebugUtilsObjectNameEXT(VkDevice device, const VkDebugUtilsObjectNameInfoEXT *pNameInfo) const;

    bool manual_PreCallValidateSetDebugUtilsObjectTagEXT(VkDevice device, const VkDebugUtilsObjectTagInfoEXT *pTagInfo) const;

    bool manual_PreCallValidateAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout,
                                                   VkSemaphore semaphore, VkFence fence, uint32_t *pImageIndex) const;

    bool manual_PreCallValidateAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR *pAcquireInfo,
                                                    uint32_t *pImageIndex) const;

    bool manual_PreCallValidateCmdBindTransformFeedbackBuffersEXT(VkCommandBuffer commandBuffer, uint32_t firstBinding,
                                                                  uint32_t bindingCount, const VkBuffer *pBuffers,
                                                                  const VkDeviceSize *pOffsets, const VkDeviceSize *pSizes) const;

    bool manual_PreCallValidateCmdBeginTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                                            uint32_t counterBufferCount, const VkBuffer *pCounterBuffers,
                                                            const VkDeviceSize *pCounterBufferOffsets) const;

    bool manual_PreCallValidateCmdEndTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                                          uint32_t counterBufferCount, const VkBuffer *pCounterBuffers,
                                                          const VkDeviceSize *pCounterBufferOffsets) const;

    bool manual_PreCallValidateCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount,
                                                           uint32_t firstInstance, VkBuffer counterBuffer,
                                                           VkDeviceSize counterBufferOffset, uint32_t counterOffset,
                                                           uint32_t vertexStride) const;

    bool manual_PreCallValidateCreateSamplerYcbcrConversion(VkDevice device, const VkSamplerYcbcrConversionCreateInfo *pCreateInfo,
                                                            const VkAllocationCallbacks *pAllocator,
                                                            VkSamplerYcbcrConversion *pYcbcrConversion) const;
    bool manual_PreCallValidateCreateSamplerYcbcrConversionKHR(VkDevice device,
                                                               const VkSamplerYcbcrConversionCreateInfo *pCreateInfo,
                                                               const VkAllocationCallbacks *pAllocator,
                                                               VkSamplerYcbcrConversion *pYcbcrConversion) const;
    bool manual_PreCallValidateImportSemaphoreFdKHR(VkDevice device,
                                                    const VkImportSemaphoreFdInfoKHR *pImportSemaphoreFdInfo) const;

    bool manual_PreCallValidateCopyAccelerationStructureToMemoryKHR(VkDevice device,
                                                                    const VkCopyAccelerationStructureToMemoryInfoKHR *pInfo) const;

    bool manual_PreCallValidateCmdCopyAccelerationStructureToMemoryKHR(
        VkCommandBuffer commandBuffer, const VkCopyAccelerationStructureToMemoryInfoKHR *pInfo) const;

    bool manual_PreCallValidateCopyAccelerationStructureKHR(VkDevice device, const VkCopyAccelerationStructureInfoKHR *pInfo) const;

    bool manual_PreCallValidateCmdCopyAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                                               const VkCopyAccelerationStructureInfoKHR *pInfo) const;
    bool ValidateCopyAccelerationStructureInfoKHR(const VkCopyAccelerationStructureInfoKHR *pInfo, const char *api_name) const;
    bool ValidateCopyMemoryToAccelerationStructureInfoKHR(const VkCopyMemoryToAccelerationStructureInfoKHR *pInfo,
                                                          const char *api_name, bool is_cmd = false) const;

    bool manual_PreCallValidateCopyMemoryToAccelerationStructureKHR(VkDevice device,
                                                                    const VkCopyMemoryToAccelerationStructureInfoKHR *pInfo) const;
    bool manual_PreCallValidateCmdCopyMemoryToAccelerationStructureKHR(
        VkCommandBuffer commandBuffer, const VkCopyMemoryToAccelerationStructureInfoKHR *pInfo) const;

    bool manual_PreCallValidateCmdWriteAccelerationStructuresPropertiesKHR(
        VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount,
        const VkAccelerationStructureKHR *pAccelerationStructures, VkQueryType queryType, VkQueryPool queryPool,
        uint32_t firstQuery) const;
    bool manual_PreCallValidateWriteAccelerationStructuresPropertiesKHR(VkDevice device, uint32_t accelerationStructureCount,
                                                                        const VkAccelerationStructureKHR *pAccelerationStructures,
                                                                        VkQueryType queryType, size_t dataSize, void *pData,
                                                                        size_t stride) const;
    bool manual_PreCallValidateGetRayTracingCaptureReplayShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline,
                                                                               uint32_t firstGroup, uint32_t groupCount,
                                                                               size_t dataSize, void *pData) const;

    bool manual_PreCallValidateCmdTraceRaysKHR(VkCommandBuffer commandBuffer,
                                               const VkStridedBufferRegionKHR *pRaygenShaderBindingTable,
                                               const VkStridedBufferRegionKHR *pMissShaderBindingTable,
                                               const VkStridedBufferRegionKHR *pHitShaderBindingTable,
                                               const VkStridedBufferRegionKHR *pCallableShaderBindingTable, uint32_t width,
                                               uint32_t height, uint32_t depth) const;

    bool manual_PreCallValidateCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer,
                                                       const VkStridedBufferRegionKHR *pRaygenShaderBindingTable,
                                                       const VkStridedBufferRegionKHR *pMissShaderBindingTable,
                                                       const VkStridedBufferRegionKHR *pHitShaderBindingTable,
                                                       const VkStridedBufferRegionKHR *pCallableShaderBindingTable, VkBuffer buffer,
                                                       VkDeviceSize offset) const;

    bool manual_PreCallValidateCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer,
                                              VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer,
                                              VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride,
                                              VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset,
                                              VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer,
                                              VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride,
                                              uint32_t width, uint32_t height, uint32_t depth) const;

    bool manual_PreCallValidateCmdBuildAccelerationStructureIndirectKHR(VkCommandBuffer commandBuffer,
                                                                        const VkAccelerationStructureBuildGeometryInfoKHR *pInfo,
                                                                        VkBuffer indirectBuffer, VkDeviceSize indirectOffset,
                                                                        uint32_t indirectStride) const;

    bool manual_PreCallValidateGetDeviceAccelerationStructureCompatibilityKHR(
        VkDevice device, const VkAccelerationStructureVersionKHR *version) const;

    bool manual_PreCallValidateBuildAccelerationStructureKHR(
        VkDevice device, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
        const VkAccelerationStructureBuildOffsetInfoKHR *const *ppOffsetInfos) const;

    bool manual_PreCallValidateCmdBuildAccelerationStructureKHR(
        VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
        const VkAccelerationStructureBuildOffsetInfoKHR *const *ppOffsetInfos) const;

    bool manual_PreCallValidateCmdSetViewportWithCountEXT(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                                          const VkViewport *pViewports) const;
    bool manual_PreCallValidateCmdSetScissorWithCountEXT(VkCommandBuffer commandBuffer, uint32_t scissorCount,
                                                         const VkRect2D *pScissors) const;
    bool manual_PreCallValidateCmdBindVertexBuffers2EXT(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                                        const VkBuffer *pBuffers, const VkDeviceSize *pOffsets,
                                                        const VkDeviceSize *pSizes, const VkDeviceSize *pStrides) const;

#include "parameter_validation.h"
};  // Class StatelessValidation
