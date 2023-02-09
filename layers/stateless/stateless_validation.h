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

#pragma once

#include "parameter_name.h"
#include "vk_typemap_helper.h"
#include "sync/sync_utils.h"
#include "state_tracker/cmd_buffer_state.h"

[[maybe_unused]] static const char *kVUID_PVError_RequiredParameter = "UNASSIGNED-GeneralParameterError-RequiredParameter";
[[maybe_unused]] static const char *kVUID_PVError_UnrecognizedValue = "UNASSIGNED-GeneralParameterError-UnrecognizedValue";
[[maybe_unused]] static const char *kVUID_PVError_ExtensionNotEnabled = "UNASSIGNED-GeneralParameterError-ExtensionNotEnabled";
[[maybe_unused]] static const char *kVUID_PVError_ApiVersionViolation = "UNASSIGNED-API-Version-Violation";

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
    vvl::unordered_map<VkPhysicalDevice, VkPhysicalDeviceProperties *> physical_device_properties_map;
    vvl::unordered_map<VkPhysicalDevice, vvl::unordered_set<std::string>> device_extensions_enumerated{};

    // Override chassis read/write locks for this validation object
    // This override takes a deferred lock. i.e. it is not acquired.
    ReadLockGuard ReadLock() const override;
    WriteLockGuard WriteLock() override;

    // Device extension properties -- storing properties gathered from VkPhysicalDeviceProperties2::pNext chain
    struct DeviceExtensionProperties {
        VkPhysicalDeviceShadingRateImagePropertiesNV shading_rate_image_props;
        VkPhysicalDeviceMeshShaderPropertiesNV mesh_shader_props_nv;
        VkPhysicalDeviceMeshShaderPropertiesEXT mesh_shader_props_ext;
        VkPhysicalDeviceRayTracingPropertiesNV ray_tracing_props_nv;
        VkPhysicalDeviceRayTracingPipelinePropertiesKHR ray_tracing_props_khr;
        VkPhysicalDeviceAccelerationStructurePropertiesKHR acc_structure_props;
        VkPhysicalDeviceTransformFeedbackPropertiesEXT transform_feedback_props;
        VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT vertex_attribute_divisor_props;
        VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT blend_operation_advanced_props;
        VkPhysicalDeviceMaintenance4PropertiesKHR maintenance4_props;
        VkPhysicalDeviceFragmentShadingRatePropertiesKHR fragment_shading_rate_props;
        VkPhysicalDeviceDepthStencilResolveProperties depth_stencil_resolve_props;
    };
    DeviceExtensionProperties phys_dev_ext_props = {};

    struct SubpassesUsageStates {
        vvl::unordered_set<uint32_t> subpasses_using_color_attachment;
        vvl::unordered_set<uint32_t> subpasses_using_depthstencil_attachment;
        std::vector<VkSubpassDescriptionFlags> subpasses_flags;
        uint32_t color_attachment_count;
    };

    // Though this validation object is predominantly statless, the Framebuffer checks are greatly simplified by creating and
    // updating a map of the renderpass usage states, and these accesses need thread protection. Use a mutex separate from the
    // parent object's to maintain that functionality.
    mutable std::mutex renderpass_map_mutex;
    vvl::unordered_map<VkRenderPass, SubpassesUsageStates> renderpasses_states;

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
            ss << api_name << ": parameter " << parameter_name.get_name() << " (= " << value << ") is not greater than "
               << lower_bound;
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
    bool ValidateRequiredPointer(const char *apiName, const ParameterName &parameterName, const void *value,
                                 const std::string &vuid) const {
        bool skip_call = false;

        if (value == nullptr) {
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
    bool ValidateArray(const char *apiName, const ParameterName &countName, const ParameterName &arrayName, T1 count,
                       const T2 *array, bool countRequired, bool arrayRequired, const char *count_required_vuid,
                       const char *array_required_vuid) const {
        bool skip_call = false;

        // Count parameters not tagged as optional cannot be 0
        if (countRequired && (count == 0)) {
            skip_call |= LogError(device, count_required_vuid, "%s: parameter %s must be greater than 0.", apiName,
                                  countName.get_name().c_str());
        }

        // Array parameters not tagged as optional cannot be NULL, unless the count is 0
        if (arrayRequired && (count != 0) && (*array == nullptr)) {
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
     * @param count_required_vuid The VUID for the '*count' parameter.
     * @param array_required_vuid The VUID for the 'array' parameter.
     * @return Boolean value indicating that the call should be skipped.
     */
    template <typename T1, typename T2>
    bool ValidateArray(const char *apiName, const ParameterName &countName, const ParameterName &arrayName, const T1 *count,
                       const T2 *array, bool countPtrRequired, bool countValueRequired, bool arrayRequired,
                       const char *count_required_vuid, const char *array_required_vuid) const {
        bool skip_call = false;

        if (count == nullptr) {
            if (countPtrRequired) {
                skip_call |= LogError(device, kVUID_PVError_RequiredParameter, "%s: required parameter %s specified as NULL",
                                      apiName, countName.get_name().c_str());
            }
        } else {
            skip_call |= ValidateArray(apiName, countName, arrayName, *array ? (*count) : 0, &array, countValueRequired,
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
    bool ValidateStructType(const char *apiName, const ParameterName &parameterName, const char *sTypeName, const T *value,
                            VkStructureType sType, bool required, const char *struct_vuid, const char *stype_vuid) const {
        bool skip_call = false;

        if (value == nullptr) {
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
    bool ValidateStructTypeArray(const char *apiName, const ParameterName &countName, const ParameterName &arrayName,
                                 const char *sTypeName, uint32_t count, const T *array, VkStructureType sType, bool countRequired,
                                 bool arrayRequired, const char *stype_vuid, const char *param_vuid,
                                 const char *count_required_vuid) const {
        bool skip_call = false;

        if ((count == 0) || (array == nullptr)) {
            skip_call |= ValidateArray(apiName, countName, arrayName, count, &array, countRequired, arrayRequired,
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
     * Validate an pointer type array of Vulkan structures
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
    bool ValidateStructPointerTypeArray(const char *apiName, const ParameterName &countName, const ParameterName &arrayName,
                                        const char *sTypeName, uint32_t count, const T *array, VkStructureType sType,
                                        bool countRequired, bool arrayRequired, const char *stype_vuid, const char *param_vuid,
                                        const char *count_required_vuid) const {
        bool skip_call = false;

        if ((count == 0) || (array == nullptr)) {
            skip_call |= ValidateArray(apiName, countName, arrayName, count, &array, countRequired, arrayRequired,
                                       count_required_vuid, param_vuid);
        } else {
            // Verify that all structs in the array have the correct type
            for (uint32_t i = 0; i < count; ++i) {
                if (array[i]->sType != sType) {
                    skip_call |= LogError(device, stype_vuid, "%s: parameter %s[%d]->sType must be %s", apiName,
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
    bool ValidateStructTypeArray(const char *apiName, const ParameterName &countName, const ParameterName &arrayName,
                                 const char *sTypeName, uint32_t *count, const T *array, VkStructureType sType,
                                 bool countPtrRequired, bool countValueRequired, bool arrayRequired, const char *stype_vuid,
                                 const char *param_vuid, const char *count_required_vuid) const {
        bool skip_call = false;

        if (count == nullptr) {
            if (countPtrRequired) {
                skip_call |= LogError(device, kVUID_PVError_RequiredParameter, "%s: required parameter %s specified as NULL",
                                      apiName, countName.get_name().c_str());
            }
        } else {
            skip_call |= ValidateStructTypeArray(apiName, countName, arrayName, sTypeName, (*count), array, sType,
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
    bool ValidateRequiredHandle(const char *api_name, const ParameterName &parameter_name, T value) const {
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
     *       that are allowed to contain VK_NULL_HANDLE, use ValidateArray() instead.
     *
     * @param api_name Name of API call being validated.
     * @param count_name Name of count parameter.
     * @param array_name Name of array parameter.
     * @param count Number of elements in the array.
     * @param array Array to validate.
     * @param count_required The 'count' parameter may not be 0 when true.
     * @param array_required The 'array' parameter may not be NULL when true.
     * @param count_required_vuid The VUID for the '*count' parameter.
     * @return Boolean value indicating that the call should be skipped.
     */
    template <typename T>
    bool ValidateHandleArray(const char *api_name, const ParameterName &count_name, const ParameterName &array_name, uint32_t count,
                             const T *array, bool count_required, bool array_required, const char *count_required_vuid) const {
        bool skip_call = false;

        if ((count == 0) || (array == nullptr)) {
            skip_call |= ValidateArray(api_name, count_name, array_name, count, &array, count_required, array_required,
                                       count_required_vuid, kVUIDUndefined);
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
    bool ValidateStringArray(const char *apiName, const ParameterName &countName, const ParameterName &arrayName, uint32_t count,
                             const char *const *array, bool countRequired, bool arrayRequired, const char *count_required_vuid,
                             const char *array_required_vuid) const {
        bool skip_call = false;

        if ((count == 0) || (array == nullptr)) {
            skip_call |= ValidateArray(apiName, countName, arrayName, count, &array, countRequired, arrayRequired,
                                       count_required_vuid, array_required_vuid);
        } else {
            // Verify that strings in the array are not NULL
            for (uint32_t i = 0; i < count; ++i) {
                if (array[i] == nullptr) {
                    skip_call |= LogError(device, array_required_vuid, "%s: required parameter %s[%d] specified as NULL", apiName,
                                          arrayName.get_name().c_str(), i);
                }
            }
        }

        return skip_call;
    }

    // Forward declarations
    bool CheckPromotedApiAgainstVulkanVersion(VkInstance instance, const char *api_name, const uint32_t promoted_version) const;
    bool CheckPromotedApiAgainstVulkanVersion(VkPhysicalDevice pdev, const char *api_name, const uint32_t promoted_version) const;
    bool SupportedByPdev(const VkPhysicalDevice physical_device, const std::string &ext_name) const;

    bool ValidatePnextStructContents(const char *api_name, const ParameterName &parameter_name, const VkBaseOutStructure *header,
                                     const char *pnext_vuid, bool is_physdev_api = false, bool is_const_param = true) const;

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
    bool ValidateStructPnext(const char *api_name, const ParameterName &parameter_name, const char *allowed_struct_names,
                             const void *next, size_t allowed_type_count, const VkStructureType *allowed_types,
                             uint32_t header_version, const char *pnext_vuid, const char *stype_vuid,
                             const bool is_physdev_api = false, const bool is_const_param = true) const {
        bool skip_call = false;

        if (next != nullptr) {
            vvl::unordered_set<const void *> cycle_check;
            vvl::unordered_set<VkStructureType, vvl::hash<int>> unique_stype_check;

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

                while (current != nullptr) {
                    if (((strncmp(api_name, "vkCreateInstance", strlen(api_name)) != 0) ||
                         (current->sType != VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO)) &&
                        ((strncmp(api_name, "vkCreateDevice", strlen(api_name)) != 0) ||
                         (current->sType != VK_STRUCTURE_TYPE_LOADER_DEVICE_CREATE_INFO))) {
                        std::string type_name = string_VkStructureType(current->sType);
                        if (unique_stype_check.find(current->sType) != unique_stype_check.end() &&
                            !IsDuplicatePnext(current->sType)) {
                            // stype_vuid will only be null if there are no listed pNext and will hit disclaimer check
                            std::string message = "%s: %s chain contains duplicate structure types: %s appears multiple times.";
                            skip_call |= LogError(device, stype_vuid, message.c_str(), api_name, parameter_name.get_name().c_str(),
                                                  type_name.c_str());
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
                            skip_call |= ValidatePnextStructContents(api_name, parameter_name, current, pnext_vuid, is_physdev_api,
                                                                     is_const_param);
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
     * Generate an error if a VkBool32 value is neither VK_TRUE nor VK_FALSE.
     *
     * @param apiName Name of API call being validated.
     * @param parameterName Name of parameter being validated.
     * @param value Boolean value to validate.
     * @return Boolean value indicating that the call should be skipped.
     */
    bool ValidateBool32(const char *apiName, const ParameterName &parameterName, VkBool32 value) const {
        bool skip_call = false;
        if ((value != VK_TRUE) && (value != VK_FALSE)) {
            skip_call |= LogError(device, kVUID_PVError_UnrecognizedValue,
                                  "%s: value of %s (%d) is neither VK_TRUE nor VK_FALSE. Applications MUST not pass any other "
                                  "values than VK_TRUE or VK_FALSE into a Vulkan implementation where a VkBool32 is expected.",
                                  apiName, parameterName.get_name().c_str(), value);
        }
        return skip_call;
    }

    /**
     * Validate an array of VkBool32 values.
     *
     * Generate an error if any VkBool32 value in an array is neither VK_TRUE nor VK_FALSE.
     *
     * @param apiName Name of API call being validated.
     * @param countName Name of count parameter.
     * @param arrayName Name of array parameter.
     * @param count Number of values in the array.
     * @param array Array of VkBool32 values to validate.
     * @param countRequired The 'count' parameter may not be 0 when true.
     * @param arrayRequired The 'array' parameter may not be NULL when true.
     * @return Boolean value indicating that the call should be skipped.
     */
    bool ValidateBool32Array(const char *apiName, const ParameterName &countName, const ParameterName &arrayName, uint32_t count,
                             const VkBool32 *array, bool countRequired, bool arrayRequired) const {
        bool skip_call = false;

        if ((count == 0) || (array == nullptr)) {
            skip_call |= ValidateArray(apiName, countName, arrayName, count, &array, countRequired, arrayRequired, kVUIDUndefined,
                                       kVUIDUndefined);
        } else {
            for (uint32_t i = 0; i < count; ++i) {
                if ((array[i] != VK_TRUE) && (array[i] != VK_FALSE)) {
                    skip_call |=
                        LogError(device, kVUID_PVError_UnrecognizedValue,
                                 "%s: value of %s[%d] (%d) is neither VK_TRUE nor VK_FALSE. Applications MUST not pass any other "
                                 "values than VK_TRUE or VK_FALSE into a Vulkan implementation where a VkBool32 is expected.",
                                 apiName, arrayName.get_name().c_str(), i, array[i]);
                }
            }
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
    bool ValidateRangedEnum(const char *apiName, const ParameterName &parameterName, const char *enumName, T value,
                            const char *vuid) const {
        bool skip = false;
        const auto valid_values = ValidParamValues<T>();

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
    bool ValidateRangedEnumArray(const char *apiName, const ParameterName &countName, const ParameterName &arrayName,
                                 const char *enumName, uint32_t count, const T *array, bool countRequired,
                                 bool arrayRequired) const {
        bool skip_call = false;
        const auto valid_values = ValidParamValues<T>();

        if ((count == 0) || (array == nullptr)) {
            skip_call |= ValidateArray(apiName, countName, arrayName, count, &array, countRequired, arrayRequired, kVUIDUndefined,
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

    template <typename T>
    bool ValidateRangedEnumArray(const char *apiName, const char *vuid, const ParameterName &countName,
                                 const ParameterName &arrayName, const char *enumName, uint32_t count, const T *array,
                                 bool countRequired, bool arrayRequired) const {
        bool skip_call = false;
        const auto valid_values = ValidParamValues<T>();

        if ((count == 0) || (array == nullptr)) {
            skip_call |= ValidateArray(apiName, countName, arrayName, count, &array, countRequired, arrayRequired, vuid, vuid);
        } else {
            for (uint32_t i = 0; i < count; ++i) {
                if (std::find(valid_values.begin(), valid_values.end(), array[i]) == valid_values.end()) {
                    skip_call |= LogError(device, vuid,
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
    bool ValidateReservedFlags(const char *api_name, const ParameterName &parameter_name, VkFlags value, const char *vuid) const {
        bool skip_call = false;

        if (value != 0) {
            skip_call |= LogError(device, vuid, "%s: parameter %s must be 0.", api_name, parameter_name.get_name().c_str());
        }

        return skip_call;
    }

    enum FlagType { kRequiredFlags, kOptionalFlags, kRequiredSingleBit, kOptionalSingleBit };

    // helper to implement validation of both 32 bit and 64 bit flags.
    template <typename FlagTypedef>
    bool ValidateFlagsImplementation(const char *api_name, const ParameterName &parameter_name, const char *flag_bits_name,
                                     FlagTypedef all_flags, FlagTypedef value, const FlagType flag_type, const char *vuid,
                                     const char *flags_zero_vuid = nullptr) const {
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

        const auto HasMaxOneBitSet = [](const FlagTypedef f) {
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
     * Validate a 32 bit Vulkan bitmask value.
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
    bool ValidateFlags(const char *api_name, const ParameterName &parameter_name, const char *flag_bits_name, VkFlags all_flags,
                       VkFlags value, const FlagType flag_type, const char *vuid, const char *flags_zero_vuid = nullptr) const {
        return ValidateFlagsImplementation<VkFlags>(api_name, parameter_name, flag_bits_name, all_flags, value, flag_type, vuid,
                                                    flags_zero_vuid);
    }

    /**
     * Validate a 64 bit Vulkan bitmask value.
     *
     * Generate a warning if a value with a VkFlags64 derived type does not contain valid flag bits
     * for that type.
     *
     * @param api_name Name of API call being validated.
     * @param parameter_name Name of parameter being validated.
     * @param flag_bits_name Name of the VkFlags64 type being validated.
     * @param all_flags A bit mask combining all valid flag bits for the VkFlags64 type being validated.
     * @param value VkFlags64 value to validate.
     * @param flag_type The type of flag, like optional, or single bit.
     * @param vuid VUID used for flag that is outside defined bits (or has more than one bit for Bits type).
     * @param flags_zero_vuid VUID used for non-optional Flags that are zero.
     * @return Boolean value indicating that the call should be skipped.
     */
    bool ValidateFlags(const char *api_name, const ParameterName &parameter_name, const char *flag_bits_name, VkFlags64 all_flags,
                       VkFlags64 value, const FlagType flag_type, const char *vuid, const char *flags_zero_vuid = nullptr) const {
        return ValidateFlagsImplementation<VkFlags64>(api_name, parameter_name, flag_bits_name, all_flags, value, flag_type, vuid,
                                                      flags_zero_vuid);
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
     * @param array_required_vuid The VUID for the 'array' parameter.
     * @return Boolean value indicating that the call should be skipped.
     */
    bool ValidateFlagsArray(const char *api_name, const ParameterName &count_name, const ParameterName &array_name,
                            const char *flag_bits_name, VkFlags all_flags, uint32_t count, const VkFlags *array,
                            bool count_required, const char *array_required_vuid) const {
        bool skip_call = false;

        if (array == nullptr) {
            // Flag arrays always need to have a valid array
            skip_call |= ValidateArray(api_name, count_name, array_name, count, &array, count_required, true, kVUIDUndefined,
                                       array_required_vuid);
        } else {
            // Verify that all VkFlags values in the array
            for (uint32_t i = 0; i < count; ++i) {
                if ((array[i] & (~all_flags)) != 0) {
                    skip_call |= LogError(device, kVUID_PVError_UnrecognizedValue,
                                          "%s: value of %s[%d] contains flag bits that are not recognized members of %s", api_name,
                                          array_name.get_name().c_str(), i, flag_bits_name);
                }
            }
        }

        return skip_call;
    }

    template <typename ExtensionState>
    bool ValidateExtensionReqs(const ExtensionState &extensions, const char *vuid, const char *extension_type,
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
                                      uint32_t dependency_index, uint32_t subpass, VkPipelineStageFlags2KHR stages,
                                      const char *vuid, const char *target, const char *func_name) const {
        bool skip = false;
        // make sure we consider all of the expanded and un-expanded graphics bits to be valid
        const auto kExcludeStages = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT_KHR | VK_PIPELINE_STAGE_2_COPY_BIT_KHR |
                                    VK_PIPELINE_STAGE_2_RESOLVE_BIT_KHR | VK_PIPELINE_STAGE_2_BLIT_BIT_KHR |
                                    VK_PIPELINE_STAGE_2_CLEAR_BIT_KHR;
        const auto kMetaGraphicsStages = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT_KHR | VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT_KHR |
                                         VK_PIPELINE_STAGE_2_PRE_RASTERIZATION_SHADERS_BIT_KHR;
        const auto kGraphicsStages =
            (sync_utils::ExpandPipelineStages(VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, VK_QUEUE_GRAPHICS_BIT) | kMetaGraphicsStages) &
            ~kExcludeStages;

        const auto IsPipeline = [pCreateInfo](uint32_t subpass, const VkPipelineBindPoint stage) {
            if (subpass == VK_SUBPASS_EXTERNAL || subpass >= pCreateInfo->subpassCount)
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
                             func_name, dependency_index, target,
                             sync_utils::StringPipelineStageFlags(stages & ~kGraphicsStages).c_str(), target, subpass);
        }

        return skip;
    }

    template <typename RenderPassCreateInfoGeneric>
    bool CreateRenderPassGeneric(VkDevice device, const RenderPassCreateInfoGeneric *pCreateInfo,
                                 const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass,
                                 RenderPassCreateVersion rp_version) const;

    template <typename T>
    void RecordRenderPass(VkRenderPass renderPass, const T *pCreateInfo) {
        std::unique_lock<std::mutex> lock(renderpass_map_mutex);
        auto &renderpass_state = renderpasses_states[renderPass];
        lock.unlock();

        renderpass_state.subpasses_flags.resize(pCreateInfo->subpassCount);
        for (uint32_t subpass = 0; subpass < pCreateInfo->subpassCount; ++subpass) {
            bool uses_color = false;
            renderpass_state.color_attachment_count = pCreateInfo->pSubpasses[subpass].colorAttachmentCount;

            for (uint32_t i = 0; i < pCreateInfo->pSubpasses[subpass].colorAttachmentCount && !uses_color; ++i)
                if (pCreateInfo->pSubpasses[subpass].pColorAttachments[i].attachment != VK_ATTACHMENT_UNUSED) uses_color = true;

            bool uses_depthstencil = false;
            if (pCreateInfo->pSubpasses[subpass].pDepthStencilAttachment)
                if (pCreateInfo->pSubpasses[subpass].pDepthStencilAttachment->attachment != VK_ATTACHMENT_UNUSED)
                    uses_depthstencil = true;

            if (uses_color) renderpass_state.subpasses_using_color_attachment.insert(subpass);
            if (uses_depthstencil) renderpass_state.subpasses_using_depthstencil_attachment.insert(subpass);
            renderpass_state.subpasses_flags[subpass] = pCreateInfo->pSubpasses[subpass].flags;
        }
    }

    // Pre/PostCallRecord declarations
    void PostCallRecordCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo *pCreateInfo,
                                        const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass,
                                        VkResult result) override;
    void PostCallRecordCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2 *pCreateInfo,
                                            const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass,
                                            VkResult result) override;
    void PostCallRecordDestroyRenderPass(VkDevice device, VkRenderPass renderPass,
                                         const VkAllocationCallbacks *pAllocator) override;
    void PostCallRecordAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo *pAllocateInfo,
                                              VkCommandBuffer *pCommandBuffers, VkResult result) override;
    void PostCallRecordFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount,
                                          const VkCommandBuffer *pCommandBuffers) override;
    void PostCallRecordDestroyCommandPool(VkDevice device, VkCommandPool commandPool,
                                          const VkAllocationCallbacks *pAllocator) override;
    void GetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2 &pProperties) const;
    void PostCallRecordCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo *pCreateInfo,
                                    const VkAllocationCallbacks *pAllocator, VkDevice *pDevice, VkResult result) override;
    void PostCallRecordCreateInstance(const VkInstanceCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator,
                                      VkInstance *pInstance, VkResult result) override;

    void CommonPostCallRecordEnumeratePhysicalDevice(const VkPhysicalDevice *phys_devices, const int count);
    void PostCallRecordEnumeratePhysicalDevices(VkInstance instance, uint32_t *pPhysicalDeviceCount,
                                                VkPhysicalDevice *pPhysicalDevices, VkResult result) override;

    void PostCallRecordEnumeratePhysicalDeviceGroups(VkInstance instance, uint32_t *pPhysicalDeviceGroupCount,
                                                     VkPhysicalDeviceGroupProperties *pPhysicalDeviceGroupProperties,
                                                     VkResult result) override;

    bool RequireDeviceExtension(bool flag, char const *function_name, char const *extension_name) const;

    bool ValidateInstanceExtensions(const VkInstanceCreateInfo *pCreateInfo) const;

    bool ValidateValidationFeatures(const VkInstanceCreateInfo *pCreateInfo,
                                    const VkValidationFeaturesEXT *validation_features) const;

    bool ValidateApiVersion(uint32_t api_version, uint32_t effective_api_version) const;

    bool ValidateString(const char *apiName, const ParameterName &stringName, const std::string &vuid,
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
                                      CMD_TYPE cmd_type) const;
    bool ValidateCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkDeviceSize offset, VkDeviceSize countBufferOffset,
                                             CMD_TYPE cmd_type) const;

    bool ValidateSwapchainCreateInfo(const char *func_name, VkSwapchainCreateInfoKHR const *pCreateInfo) const;

    bool OutputExtensionError(const std::string &api_name, const std::string &extension_name) const;

    void PreCallRecordDestroyInstance(VkInstance instance, const VkAllocationCallbacks *pAllocator) override;

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

    bool manual_PreCallValidateCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo *pCreateInfo,
                                                    const VkAllocationCallbacks *pAllocator,
                                                    VkPipelineLayout *pPipelineLayout) const;

    bool ValidatePipelineShaderStageCreateInfo(const char *func_name, const char *msg,
                                               const VkPipelineShaderStageCreateInfo *pCreateInfo) const;
    bool ValidatePipelineTessellationStateCreateInfo(const VkPipelineTessellationStateCreateInfo &info, uint32_t index) const;
    bool ValidatePipelineVertexInputStateCreateInfo(const VkPipelineVertexInputStateCreateInfo &info, uint32_t index) const;
    bool ValidatePipelineViewportStateCreateInfo(const VkPipelineViewportStateCreateInfo &info, uint32_t index) const;
    bool ValidatePipelineMultisampleStateCreateInfo(const VkPipelineMultisampleStateCreateInfo &info, uint32_t index) const;
    bool ValidatePipelineColorBlendAttachmentState(const VkPipelineColorBlendAttachmentState &attachment_state, uint32_t pipe_index,
                                                   uint32_t attachment_index) const;
    bool ValidatePipelineColorBlendStateCreateInfo(const VkPipelineColorBlendStateCreateInfo &info, uint32_t index) const;
    bool ValidatePipelineDepthStencilStateCreateInfo(const VkPipelineDepthStencilStateCreateInfo &info, uint32_t index) const;
    bool ValidatePipelineInputAssemblyStateCreateInfo(const VkPipelineInputAssemblyStateCreateInfo &info, uint32_t index) const;
    bool manual_PreCallValidateCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                       const VkGraphicsPipelineCreateInfo *pCreateInfos,
                                                       const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines) const;
    bool manual_PreCallValidateCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                      const VkComputePipelineCreateInfo *pCreateInfos,
                                                      const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines) const;

    bool manual_PreCallValidateCreateSampler(VkDevice device, const VkSamplerCreateInfo *pCreateInfo,
                                             const VkAllocationCallbacks *pAllocator, VkSampler *pSampler) const;
    bool ValidateMutableDescriptorTypeCreateInfo(const VkDescriptorSetLayoutCreateInfo &create_info,
                                                 const VkMutableDescriptorTypeCreateInfoEXT &mutable_create_info,
                                                 const char *func_name) const;
    bool manual_PreCallValidateCreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
                                                         const VkAllocationCallbacks *pAllocator,
                                                         VkDescriptorSetLayout *pSetLayout) const;

    bool manual_PreCallValidateCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo *pCreateInfo,
                                               const VkAllocationCallbacks *pAllocator, VkSemaphore *pSemaphore) const;

    bool manual_PreCallValidateCreateEvent(VkDevice device, const VkEventCreateInfo *pCreateInfo,
                                           const VkAllocationCallbacks *pAllocator, VkEvent *pEvent) const;

    bool manual_PreCallValidateCreateBufferView(VkDevice device, const VkBufferViewCreateInfo *pCreateInfo,
                                                const VkAllocationCallbacks *pAllocator, VkBufferView *pBufferView) const;

#ifdef VK_USE_PLATFORM_METAL_EXT
    bool ExportMetalObjectsPNextUtil(VkExportMetalObjectTypeFlagBitsEXT bit, const char *vuid, const char *api_call,
                                     const char *sType, const void *pNext) const;
#endif  // VK_USE_PLATFORM_METAL_EXT

    bool ValidateWriteDescriptorSet(const char *vkCallingFunction, const uint32_t descriptorWriteCount,
                                    const VkWriteDescriptorSet *pDescriptorWrites, const bool isPushDescriptor) const;
    bool manual_PreCallValidateUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount,
                                                    const VkWriteDescriptorSet *pDescriptorWrites, uint32_t descriptorCopyCount,
                                                    const VkCopyDescriptorSet *pDescriptorCopies) const;

    bool manual_PreCallValidateFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t descriptorSetCount,
                                                  const VkDescriptorSet *pDescriptorSets) const;

    bool manual_PreCallValidateCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo *pCreateInfo,
                                                const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass) const;

    bool manual_PreCallValidateCreateRenderPass2(VkDevice device, const VkRenderPassCreateInfo2 *pCreateInfo,
                                                 const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass) const;

    bool manual_PreCallValidateCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2 *pCreateInfo,
                                                    const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass) const;

    bool manual_PreCallValidateFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount,
                                                  const VkCommandBuffer *pCommandBuffers) const;

    bool manual_PreCallValidateBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo *pBeginInfo) const;

    bool manual_PreCallValidateCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                              const VkViewport *pViewports) const;

    bool manual_PreCallValidateCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount,
                                             const VkRect2D *pScissors) const;
    bool manual_PreCallValidateCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth) const;

    bool manual_PreCallValidateCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                               uint32_t drawCount, uint32_t stride) const;

    bool manual_PreCallValidateCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                      uint32_t drawCount, uint32_t stride) const;

    bool manual_PreCallValidateCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                    VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                    uint32_t stride) const;

    bool manual_PreCallValidateCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                       uint32_t stride) const;

    bool manual_PreCallValidateCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                       uint32_t stride) const;

    bool manual_PreCallValidateCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                           VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                           uint32_t maxDrawCount, uint32_t stride) const;

    bool manual_PreCallValidateCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                              VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                              uint32_t maxDrawCount, uint32_t stride) const;

    bool manual_PreCallValidateCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                              VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                              uint32_t maxDrawCount, uint32_t stride) const;

    bool manual_PreCallValidateCmdDrawMultiEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                               const VkMultiDrawInfoEXT *pVertexInfo, uint32_t instanceCount,
                                               uint32_t firstInstance, uint32_t stride) const;

    bool manual_PreCallValidateCmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                                      const VkMultiDrawIndexedInfoEXT *pIndexInfo, uint32_t instanceCount,
                                                      uint32_t firstInstance, uint32_t stride, const int32_t *pVertexOffset) const;

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
    bool manual_PreCallValidateGetPhysicalDeviceImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format,
                                                                      VkImageType type, VkImageTiling tiling,
                                                                      VkImageUsageFlags usage, VkImageCreateFlags flags,
                                                                      VkImageFormatProperties *pImageFormatProperties) const;

    bool manual_PreCallValidateCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer,
                                             uint32_t regionCount, const VkBufferCopy *pRegions) const;

    bool manual_PreCallValidateCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2KHR *pCopyBufferInfo) const;

    bool manual_PreCallValidateCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2 *pCopyBufferInfo) const;

    bool manual_PreCallValidateCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                               VkDeviceSize dataSize, const void *pData) const;

    bool manual_PreCallValidateCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                             VkDeviceSize size, uint32_t data) const;

    bool manual_PreCallValidateCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR *pCreateInfo,
                                                  const VkAllocationCallbacks *pAllocator, VkSwapchainKHR *pSwapchain) const;
    bool manual_PreCallValidateCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount,
                                                         const VkSwapchainCreateInfoKHR *pCreateInfos,
                                                         const VkAllocationCallbacks *pAllocator,
                                                         VkSwapchainKHR *pSwapchains) const;
    bool manual_PreCallValidateQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR *pPresentInfo) const;
    bool manual_PreCallValidateCreateDisplayModeKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                    const VkDisplayModeCreateInfoKHR *pCreateInfo,
                                                    const VkAllocationCallbacks *pAllocator, VkDisplayModeKHR *pMode) const;

#ifdef VK_USE_PLATFORM_WIN32_KHR
    bool manual_PreCallValidateCreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR *pCreateInfo,
                                                     const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface) const;
#endif  // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    bool manual_PreCallValidateCreateWaylandSurfaceKHR(VkInstance instance, const VkWaylandSurfaceCreateInfoKHR *pCreateInfo,
                                                       const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface) const;
#endif  // VK_USE_PLATFORM_WAYLAND_KHR

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

    bool manual_PreCallValidateCmdDrawMeshTasksEXT(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                                   uint32_t groupCountZ) const;
    bool manual_PreCallValidateCmdDrawMeshTasksIndirectEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                           uint32_t drawCount, uint32_t stride) const;

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

    bool manual_PreCallValidateCmdWriteAccelerationStructuresPropertiesNV(VkCommandBuffer commandBuffer,
                                                                          uint32_t accelerationStructureCount,
                                                                          const VkAccelerationStructureNV *pAccelerationStructures,
                                                                          VkQueryType queryType, VkQueryPool queryPool,
                                                                          uint32_t firstQuery) const;
    bool manual_PreCallValidateCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                           const VkRayTracingPipelineCreateInfoNV *pCreateInfos,
                                                           const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines) const;
    bool manual_PreCallValidateCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                            VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                            const VkRayTracingPipelineCreateInfoKHR *pCreateInfos,
                                                            const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines) const;
    bool manual_PreCallValidateCmdSetViewportWScalingNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                        uint32_t viewportCount,
                                                        const VkViewportWScalingNV *pViewportWScalings) const;

#ifdef VK_USE_PLATFORM_WIN32_KHR
    bool PreCallValidateGetDeviceGroupSurfacePresentModes2EXT(VkDevice device, const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo,
                                                              VkDeviceGroupPresentModeFlagsKHR *pModes) const override;
#endif  // VK_USE_PLATFORM_WIN32_KHR

    bool manual_PreCallValidateCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo *pCreateInfo,
                                                 const VkAllocationCallbacks *pAllocator, VkFramebuffer *pFramebuffer) const;

    bool manual_PreCallValidateCmdSetLineStippleEXT(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor,
                                                    uint16_t lineStipplePattern) const;

    bool manual_PreCallValidateCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                  VkIndexType indexType) const;
    bool manual_PreCallValidateCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                                    const VkBuffer *pBuffers, const VkDeviceSize *pOffsets) const;

    bool ValidateDebugUtilsObjectNameInfoEXT(const std::string &api_name, VkDevice device,
                                             const VkDebugUtilsObjectNameInfoEXT *pNameInfo) const;
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

    bool ValidateExternalSemaphoreHandleType(VkSemaphore semaphore, const char *vuid, const char *caller,
                                             VkExternalSemaphoreHandleTypeFlagBits handle_type,
                                             VkExternalSemaphoreHandleTypeFlags allowed_types) const;
    bool ValidateExternalFenceHandleType(VkFence fence, const char *vuid, const char *caller,
                                         VkExternalFenceHandleTypeFlagBits handle_type,
                                         VkExternalFenceHandleTypeFlags allowed_types) const;
    bool manual_PreCallValidateImportSemaphoreFdKHR(VkDevice device,
                                                    const VkImportSemaphoreFdInfoKHR *pImportSemaphoreFdInfo) const;
    bool manual_PreCallValidateGetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR *pGetFdInfo, int *pFd) const;

    bool manual_PreCallValidateImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR *pImportFenceFdInfo) const;
    bool manual_PreCallValidateGetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR *pGetFdInfo, int *pFd) const;

#ifdef VK_USE_PLATFORM_WIN32_KHR
    bool manual_PreCallValidateImportSemaphoreWin32HandleKHR(
        VkDevice device, const VkImportSemaphoreWin32HandleInfoKHR *pImportSemaphoreWin32HandleInfo) const;
    bool manual_PreCallValidateGetSemaphoreWin32HandleKHR(VkDevice device,
                                                          const VkSemaphoreGetWin32HandleInfoKHR *pGetWin32HandleInfo,
                                                          HANDLE *pHandle) const;

    bool manual_PreCallValidateImportFenceWin32HandleKHR(VkDevice device,
                                                         const VkImportFenceWin32HandleInfoKHR *pImportFenceWin32HandleInfo) const;
    bool manual_PreCallValidateGetFenceWin32HandleKHR(VkDevice device, const VkFenceGetWin32HandleInfoKHR *pGetWin32HandleInfo,
                                                      HANDLE *pHandle) const;
#endif

    bool manual_PreCallValidateCopyAccelerationStructureToMemoryKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                                    const VkCopyAccelerationStructureToMemoryInfoKHR *pInfo) const;

    bool manual_PreCallValidateCmdCopyAccelerationStructureToMemoryKHR(
        VkCommandBuffer commandBuffer, const VkCopyAccelerationStructureToMemoryInfoKHR *pInfo) const;

    bool manual_PreCallValidateCopyAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                            const VkCopyAccelerationStructureInfoKHR *pInfo) const;

    bool manual_PreCallValidateCmdCopyAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                                               const VkCopyAccelerationStructureInfoKHR *pInfo) const;
    bool ValidateCopyAccelerationStructureInfoKHR(const VkCopyAccelerationStructureInfoKHR *pInfo, const char *api_name) const;
    bool ValidateCopyMemoryToAccelerationStructureInfoKHR(const VkCopyMemoryToAccelerationStructureInfoKHR *pInfo,
                                                          const char *api_name, bool is_cmd = false) const;

    bool manual_PreCallValidateCopyMemoryToAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
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
                                               const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable,
                                               const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable,
                                               const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable,
                                               const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable, uint32_t width,
                                               uint32_t height, uint32_t depth) const;

    bool manual_PreCallValidateCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer,
                                                       const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable,
                                                       const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable,
                                                       const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable,
                                                       const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable,
                                                       VkDeviceAddress indirectDeviceAddress) const;

    bool manual_PreCallValidateCmdTraceRaysIndirect2KHR(VkCommandBuffer commandBuffer, VkDeviceAddress indirectDeviceAddress) const;

    bool manual_PreCallValidateCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer,
                                              VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer,
                                              VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride,
                                              VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset,
                                              VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer,
                                              VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride,
                                              uint32_t width, uint32_t height, uint32_t depth) const;

    bool manual_PreCallValidateCmdBuildAccelerationStructuresIndirectKHR(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                                                         const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
                                                                         const VkDeviceAddress *pIndirectDeviceAddresses,
                                                                         const uint32_t *pIndirectStrides,
                                                                         const uint32_t *const *ppMaxPrimitiveCounts) const;

    bool manual_PreCallValidateGetDeviceAccelerationStructureCompatibilityKHR(
        VkDevice device, const VkAccelerationStructureVersionInfoKHR *pVersionInfo,
        VkAccelerationStructureCompatibilityKHR *pCompatibility) const;
    bool ValidateCmdSetViewportWithCount(VkCommandBuffer commandBuffer, uint32_t viewportCount, const VkViewport *pViewports,
                                         CMD_TYPE cmd_type) const;
    bool manual_PreCallValidateCmdSetViewportWithCountEXT(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                                          const VkViewport *pViewports) const;
    bool manual_PreCallValidateCmdSetViewportWithCount(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                                       const VkViewport *pViewports) const;

    bool ValidateCmdSetScissorWithCount(VkCommandBuffer commandBuffer, uint32_t scissorCount, const VkRect2D *pScissors,
                                        CMD_TYPE cmd_type) const;
    bool manual_PreCallValidateCmdSetScissorWithCountEXT(VkCommandBuffer commandBuffer, uint32_t scissorCount,
                                                         const VkRect2D *pScissors) const;
    bool manual_PreCallValidateCmdSetScissorWithCount(VkCommandBuffer commandBuffer, uint32_t scissorCount,
                                                      const VkRect2D *pScissors) const;
    bool ValidateCmdBindVertexBuffers2(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                       const VkBuffer *pBuffers, const VkDeviceSize *pOffsets, const VkDeviceSize *pSizes,
                                       const VkDeviceSize *pStrides, CMD_TYPE cmd_type) const;
    bool manual_PreCallValidateCmdBindVertexBuffers2EXT(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                                        const VkBuffer *pBuffers, const VkDeviceSize *pOffsets,
                                                        const VkDeviceSize *pSizes, const VkDeviceSize *pStrides) const;
    bool manual_PreCallValidateCmdBindVertexBuffers2(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                                     const VkBuffer *pBuffers, const VkDeviceSize *pOffsets,
                                                     const VkDeviceSize *pSizes, const VkDeviceSize *pStrides) const;
    bool ValidateAccelerationStructureBuildGeometryInfoKHR(const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
                                                           uint32_t infoCount, uint64_t total_primitive_count,
                                                           const char *api_name) const;
    bool manual_PreCallValidateCmdBuildAccelerationStructuresKHR(
        VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
        const VkAccelerationStructureBuildRangeInfoKHR *const *ppBuildRangeInfos) const;

    bool manual_PreCallValidateBuildAccelerationStructuresKHR(
        VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount,
        const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
        const VkAccelerationStructureBuildRangeInfoKHR *const *ppBuildRangeInfos) const;

    bool manual_PreCallValidateGetAccelerationStructureBuildSizesKHR(VkDevice device, VkAccelerationStructureBuildTypeKHR buildType,
                                                                     const VkAccelerationStructureBuildGeometryInfoKHR *pBuildInfo,
                                                                     const uint32_t *pMaxPrimitiveCounts,
                                                                     VkAccelerationStructureBuildSizesInfoKHR *pSizeInfo) const;

    bool manual_PreCallValidateCmdSetVertexInputEXT(
        VkCommandBuffer commandBuffer, uint32_t vertexBindingDescriptionCount,
        const VkVertexInputBindingDescription2EXT *pVertexBindingDescriptions, uint32_t vertexAttributeDescriptionCount,
        const VkVertexInputAttributeDescription2EXT *pVertexAttributeDescriptions) const;

    bool manual_PreCallValidateCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout,
                                                VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size,
                                                const void *pValues) const;

    bool manual_PreCallValidateMergePipelineCaches(VkDevice device, VkPipelineCache dstCache, uint32_t srcCacheCount,
                                                   const VkPipelineCache *pSrcCaches) const;

    bool manual_PreCallValidateCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                  const VkClearColorValue *pColor, uint32_t rangeCount,
                                                  const VkImageSubresourceRange *pRanges) const;

    bool ValidateCmdBeginRenderPass(const VkRenderPassBeginInfo *const rp_begin, CMD_TYPE cmd_type) const;
    bool manual_PreCallValidateCmdBeginRenderPass(VkCommandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                  VkSubpassContents) const;
    bool manual_PreCallValidateCmdBeginRenderPass2KHR(VkCommandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                      const VkSubpassBeginInfo *) const;
    bool manual_PreCallValidateCmdBeginRenderPass2(VkCommandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                   const VkSubpassBeginInfo *) const;
    bool ValidateCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo *pRenderingInfo, CMD_TYPE cmd_type) const;
    bool manual_PreCallValidateCmdBeginRenderingKHR(VkCommandBuffer commandBuffer, const VkRenderingInfo *pRenderingInfo) const;
    bool manual_PreCallValidateCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo *pRenderingInfo) const;

    bool manual_PreCallValidateCmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer, uint32_t firstDiscardRectangle,
                                                         uint32_t discardRectangleCount, const VkRect2D *pDiscardRectangles) const;
    bool manual_PreCallValidateGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                                   size_t dataSize, void *pData, VkDeviceSize stride,
                                                   VkQueryResultFlags flags) const;
    bool manual_PreCallValidateCmdBeginConditionalRenderingEXT(
        VkCommandBuffer commandBuffer, const VkConditionalRenderingBeginInfoEXT *pConditionalRenderingBegin) const;

    bool ValidateDeviceImageMemoryRequirements(VkDevice device, const VkDeviceImageMemoryRequirementsKHR *pInfo,
                                               const char *func_name) const;

    bool manual_PreCallValidateGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                  uint32_t *pSurfaceFormatCount,
                                                                  VkSurfaceFormatKHR *pSurfaceFormats) const;

    bool manual_PreCallValidateGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                       uint32_t *pPresentModeCount,
                                                                       VkPresentModeKHR *pPresentModes) const;

    bool manual_PreCallValidateGetPhysicalDeviceSurfaceCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                                        const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo,
                                                                        VkSurfaceCapabilities2KHR *pSurfaceCapabilities) const;

    bool manual_PreCallValidateGetPhysicalDeviceSurfaceFormats2KHR(VkPhysicalDevice physicalDevice,
                                                                   const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo,
                                                                   uint32_t *pSurfaceFormatCount,
                                                                   VkSurfaceFormat2KHR *pSurfaceFormats) const;
#ifdef VK_USE_PLATFORM_WIN32_KHR
    bool manual_PreCallValidateGetPhysicalDeviceSurfacePresentModes2EXT(VkPhysicalDevice physicalDevice,
                                                                        const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo,
                                                                        uint32_t *pPresentModeCount,
                                                                        VkPresentModeKHR *pPresentModes) const;
#endif  // VK_USE_PLATFORM_WIN32_KHR

    bool manual_PreCallValidateGetDeviceImageMemoryRequirementsKHR(VkDevice device, const VkDeviceImageMemoryRequirements *pInfo,
                                                                   VkMemoryRequirements2 *pMemoryRequirements) const;
    bool manual_PreCallValidateGetDeviceImageSparseMemoryRequirementsKHR(
        VkDevice device, const VkDeviceImageMemoryRequirements *pInfo, uint32_t *pSparseMemoryRequirementCount,
        VkSparseImageMemoryRequirements2 *pSparseMemoryRequirements) const;

#ifdef VK_USE_PLATFORM_METAL_EXT
    bool manual_PreCallValidateExportMetalObjectsEXT(VkDevice device, VkExportMetalObjectsInfoEXT *pMetalObjectsInfo) const;
#endif  // VK_USE_PLATFORM_METAL_EXT

#include "parameter_validation.h"
};  // Class StatelessValidation
