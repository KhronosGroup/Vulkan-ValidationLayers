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

#include <vulkan/utility/vk_struct_helper.hpp>
#include "sync/sync_utils.h"
#include "state_tracker/cmd_buffer_state.h"

[[maybe_unused]] static const char *kVUID_PVError_RequiredParameter = "UNASSIGNED-GeneralParameterError-RequiredParameter";
[[maybe_unused]] static const char *kVUID_PVError_UnrecognizedValue = "UNASSIGNED-GeneralParameterError-UnrecognizedValue";
[[maybe_unused]] static const char *kVUID_PVError_ExtensionNotEnabled = "UNASSIGNED-GeneralParameterError-ExtensionNotEnabled";
[[maybe_unused]] static const char *kVUID_PVError_ApiVersionViolation = "UNASSIGNED-API-Version-Violation";

extern std::vector<std::pair<uint32_t, uint32_t>> custom_stype_info;

// String returned by string_VkStructureType for an unrecognized type.
const std::string UnsupportedStructureTypeString = "Unhandled VkStructureType";

class StatelessValidation : public ValidationObject {
    using Func = vvl::Func;
    using Struct = vvl::Struct;
    using Field = vvl::Field;

  public:
    VkPhysicalDeviceLimits device_limits = {};
    safe_VkPhysicalDeviceFeatures2 physical_device_features2;
    void *device_createinfo_pnext;
    const VkPhysicalDeviceFeatures &physical_device_features = physical_device_features2.features;
    vvl::unordered_map<VkPhysicalDevice, VkPhysicalDeviceProperties *> physical_device_properties_map;
    vvl::unordered_map<VkPhysicalDevice, vvl::unordered_set<std::string>> device_extensions_enumerated{};

    // This was a special case where it was decided to use the extension version for validation
    // https://gitlab.khronos.org/vulkan/vulkan/-/merge_requests/5671
    inline static uint32_t discard_rectangles_extension_version = 0;
    inline static uint32_t scissor_exclusive_extension_version = 0;

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

    bool ValidateNotZero(bool is_zero, const std::string &vuid, const Location &loc) const;

    bool ValidateRequiredPointer(const Location &loc, const void *value, const std::string &vuid) const;

    template <typename T1, typename T2>
    bool ValidateArray(const Location &count_loc, const Location &array_loc, T1 count, const T2 *array, bool countRequired,
                       bool arrayRequired, const char *count_required_vuid, const char *array_required_vuid) const {
        bool skip = false;

        // Count parameters not tagged as optional cannot be 0
        if (countRequired && (count == 0)) {
            skip |= LogError(count_required_vuid, device, count_loc, "must be greater than 0.");
        }

        // Array parameters not tagged as optional cannot be NULL, unless the count is 0
        if (arrayRequired && (count != 0) && (*array == nullptr)) {
            skip |= LogError(array_required_vuid, device, array_loc, "is NULL.");
        }

        return skip;
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
     * @param loc Name of API call being validated.
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
    bool ValidateArray(const Location &count_loc, const Location &array_loc, const T1 *count, const T2 *array,
                       bool countPtrRequired, bool countValueRequired, bool arrayRequired, const char *count_required_vuid,
                       const char *array_required_vuid) const {
        bool skip = false;

        if (count == nullptr) {
            if (countPtrRequired) {
                skip |= LogError(kVUID_PVError_RequiredParameter, device, count_loc, "is NULL.");
            }
        } else {
            skip |= ValidateArray(count_loc, array_loc, *array ? (*count) : 0, &array, countValueRequired, arrayRequired,
                                  count_required_vuid, array_required_vuid);
        }

        return skip;
    }

    /**
     * Validate a pointer to a Vulkan structure.
     *
     * Verify that a required pointer to a structure is not NULL.  If the pointer is
     * not NULL, verify that each structure's sType field is set to the correct
     * VkStructureType value.
     *
     * @param loc Name of API call being validated.
     * @param sTypeName Name of expected VkStructureType value.
     * @param value Pointer to the struct to validate.
     * @param sType VkStructureType for structure validation.
     * @param required The parameter may not be NULL when true.
     * @return Boolean value indicating that the call should be skipped.
     */
    template <typename T>
    bool ValidateStructType(const Location &loc, const char *sTypeName, const T *value, VkStructureType sType, bool required,
                            const char *struct_vuid, const char *stype_vuid) const {
        bool skip = false;

        if (value == nullptr) {
            if (required) {
                skip |= LogError(struct_vuid, device, loc, "is NULL.");
            }
        } else if (value->sType != sType) {
            skip |= LogError(stype_vuid, device, loc.dot(Field::sType), "must be %s.", sTypeName);
        }

        return skip;
    }

    /**
     * Validate an array of Vulkan structures
     *
     * Verify that required count and array parameters are not 0 or NULL.  If
     * the array contains 1 or more structures, verify that each structure's
     * sType field is set to the correct VkStructureType value.
     *
     * @param count_loc Name of count parameter.
     * @param array_loc Name of array parameter.
     * @param sTypeName Name of expected VkStructureType value.
     * @param count Number of elements in the array.
     * @param array Array to validate.
     * @param sType VkStructureType for structure validation.
     * @param countRequired The 'count' parameter may not be 0 when true.
     * @param arrayRequired The 'array' parameter may not be NULL when true.
     * @return Boolean value indicating that the call should be skipped.
     */
    template <typename T>
    bool ValidateStructTypeArray(const Location &count_loc, const Location &array_loc, const char *sTypeName, uint32_t count,
                                 const T *array, VkStructureType sType, bool countRequired, bool arrayRequired,
                                 const char *stype_vuid, const char *param_vuid, const char *count_required_vuid) const {
        bool skip = false;

        if ((count == 0) || (array == nullptr)) {
            skip |=
                ValidateArray(count_loc, array_loc, count, &array, countRequired, arrayRequired, count_required_vuid, param_vuid);
        } else {
            // Verify that all structs in the array have the correct type
            for (uint32_t i = 0; i < count; ++i) {
                if (array[i].sType != sType) {
                    skip |= LogError(stype_vuid, device, array_loc.dot(i).dot(Field::sType), "must be %s", sTypeName);
                }
            }
        }

        return skip;
    }

    /**
     * Validate an pointer type array of Vulkan structures
     *
     * Verify that required count and array parameters are not 0 or NULL.  If
     * the array contains 1 or more structures, verify that each structure's
     * sType field is set to the correct VkStructureType value.
     *
     * @param count_loc Name of count parameter.
     * @param array_loc Name of array parameter.
     * @param sTypeName Name of expected VkStructureType value.
     * @param count Number of elements in the array.
     * @param array Array to validate.
     * @param sType VkStructureType for structure validation.
     * @param countRequired The 'count' parameter may not be 0 when true.
     * @param arrayRequired The 'array' parameter may not be NULL when true.
     * @return Boolean value indicating that the call should be skipped.
     */
    template <typename T>
    bool ValidateStructPointerTypeArray(const Location &count_loc, const Location &array_loc, const char *sTypeName, uint32_t count,
                                        const T *array, VkStructureType sType, bool countRequired, bool arrayRequired,
                                        const char *stype_vuid, const char *param_vuid, const char *count_required_vuid) const {
        bool skip = false;

        if ((count == 0) || (array == nullptr)) {
            skip |=
                ValidateArray(count_loc, array_loc, count, &array, countRequired, arrayRequired, count_required_vuid, param_vuid);
        } else {
            // Verify that all structs in the array have the correct type
            for (uint32_t i = 0; i < count; ++i) {
                if (array[i]->sType != sType) {
                    skip |= LogError(stype_vuid, device, array_loc.dot(i).dot(Field::sType), "must be %s", sTypeName);
                }
            }
        }

        return skip;
    }

    /**
     * Validate an array of Vulkan structures.
     *
     * Verify that required count and array parameters are not NULL.  If count
     * is not NULL and its value is not optional, verify that it is not 0.
     * If the array contains 1 or more structures, verify that each structure's
     * sType field is set to the correct VkStructureType value.
     *
     * @param count_loc Name of count parameter.
     * @param array_loc Name of array parameter.
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
    bool ValidateStructTypeArray(const Location &count_loc, const Location &array_loc, const char *sTypeName, uint32_t *count,
                                 const T *array, VkStructureType sType, bool countPtrRequired, bool countValueRequired,
                                 bool arrayRequired, const char *stype_vuid, const char *param_vuid,
                                 const char *count_required_vuid) const {
        bool skip = false;

        if (count == nullptr) {
            if (countPtrRequired) {
                skip |= LogError(kVUID_PVError_RequiredParameter, device, count_loc, "is NULL.");
            }
        } else {
            skip |= ValidateStructTypeArray(count_loc, array_loc, sTypeName, (*count), array, sType,
                                            countValueRequired && (array != nullptr), arrayRequired, stype_vuid, param_vuid,
                                            count_required_vuid);
        }

        return skip;
    }

    /**
     * Validate a Vulkan handle.
     *
     * Verify that the specified handle is not VK_NULL_HANDLE.
     *
     * @param loc Name of API call being validated.
     * @param value Handle to validate.
     * @return Boolean value indicating that the call should be skipped.
     */
    template <typename T>
    bool ValidateRequiredHandle(const Location &loc, T value) const {
        bool skip = false;

        if (value == VK_NULL_HANDLE) {
            skip |= LogError(kVUID_PVError_RequiredParameter, device, loc, "is VK_NULL_HANDLE.");
        }
        return skip;
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
     * @param count_loc Name of count parameter.
     * @param array_loc Name of array parameter.
     * @param count Number of elements in the array.
     * @param array Array to validate.
     * @param count_required The 'count' parameter may not be 0 when true.
     * @param array_required The 'array' parameter may not be NULL when true.
     * @param count_required_vuid The VUID for the '*count' parameter.
     * @return Boolean value indicating that the call should be skipped.
     */
    template <typename T>
    bool ValidateHandleArray(const Location &count_loc, const Location &array_loc, uint32_t count, const T *array,
                             bool count_required, bool array_required, const char *count_required_vuid) const {
        bool skip = false;

        if ((count == 0) || (array == nullptr)) {
            skip |= ValidateArray(count_loc, array_loc, count, &array, count_required, array_required, count_required_vuid,
                                  kVUIDUndefined);
        } else {
            // Verify that no handles in the array are VK_NULL_HANDLE
            for (uint32_t i = 0; i < count; ++i) {
                if (array[i] == VK_NULL_HANDLE) {
                    skip |= LogError(kVUID_PVError_RequiredParameter, device, array_loc.dot(i), "is VK_NULL_HANDLE.");
                }
            }
        }

        return skip;
    }

    bool ValidateStringArray(const Location &count_loc, const Location &array_loc, uint32_t count, const char *const *array,
                             bool countRequired, bool arrayRequired, const char *count_required_vuid,
                             const char *array_required_vuid) const;

    bool CheckPromotedApiAgainstVulkanVersion(VkInstance instance, const Location &loc, const uint32_t promoted_version) const;
    bool CheckPromotedApiAgainstVulkanVersion(VkPhysicalDevice pdev, const Location &loc, const uint32_t promoted_version) const;
    bool SupportedByPdev(const VkPhysicalDevice physical_device, const std::string &ext_name) const;

    bool ValidatePnextStructContents(const Location &loc, const VkBaseOutStructure *header, const char *pnext_vuid,
                                     bool is_physdev_api = false, bool is_const_param = true) const;

    bool ValidateStructPnext(const Location &loc, const void *next, size_t allowed_type_count, const VkStructureType *allowed_types,
                             uint32_t header_version, const char *pnext_vuid, const char *stype_vuid,
                             const bool is_physdev_api = false, const bool is_const_param = true) const;

    bool ValidateBool32(const Location &loc, VkBool32 value) const;

    bool ValidateBool32Array(const Location &count_loc, const Location &array_loc, uint32_t count, const VkBool32 *array,
                             bool countRequired, bool arrayRequired) const;

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
     * @param loc Name of API call being validated.
     * @param enumName Name of the enumeration being validated.
     * @param valid_values The list of valid values for the enumeration.
     * @param value Enumeration value to validate.
     * @return Boolean value indicating that the call should be skipped.
     */
    template <typename T>
    bool ValidateRangedEnum(const Location &loc, const char *enumName, T value, const char *vuid) const {
        bool skip = false;
        const auto valid_values = ValidParamValues<T>();

        if (std::find(valid_values.begin(), valid_values.end(), value) == valid_values.end()) {
            skip |= LogError(vuid, device, loc,
                             "(%" PRIu32
                             ") does not fall within the begin..end range of the core %s enumeration tokens and is "
                             "not an extension added token.",
                             value, enumName);
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
     * @param count_loc Name of count parameter.
     * @param array_loc Name of array parameter.
     * @param enumName Name of the enumeration being validated.
     * @param valid_values The list of valid values for the enumeration.
     * @param count Number of enumeration values in the array.
     * @param array Array of enumeration values to validate.
     * @param countRequired The 'count' parameter may not be 0 when true.
     * @param arrayRequired The 'array' parameter may not be NULL when true.
     * @return Boolean value indicating that the call should be skipped.
     */
    template <typename T>
    bool ValidateRangedEnumArray(const Location &count_loc, const Location &array_loc, const char *enumName, uint32_t count,
                                 const T *array, bool countRequired, bool arrayRequired) const {
        bool skip = false;
        const auto valid_values = ValidParamValues<T>();

        if ((count == 0) || (array == nullptr)) {
            skip |=
                ValidateArray(count_loc, array_loc, count, &array, countRequired, arrayRequired, kVUIDUndefined, kVUIDUndefined);
        } else {
            for (uint32_t i = 0; i < count; ++i) {
                if (std::find(valid_values.begin(), valid_values.end(), array[i]) == valid_values.end()) {
                    skip |= LogError(kVUID_PVError_UnrecognizedValue, device, array_loc.dot(i),
                                     "(%" PRIu32
                                     ") does not fall within the begin..end range of the core %s "
                                     "enumeration tokens and is not an extension added token",
                                     array[i], enumName);
                }
            }
        }

        return skip;
    }

    template <typename T>
    bool ValidateRangedEnumArray(const Location &count_loc, const Location &array_loc, const char *vuid, const char *enumName,
                                 uint32_t count, const T *array, bool countRequired, bool arrayRequired) const {
        bool skip = false;
        const auto valid_values = ValidParamValues<T>();

        if ((count == 0) || (array == nullptr)) {
            skip |= ValidateArray(count_loc, array_loc, count, &array, countRequired, arrayRequired, vuid, vuid);
        } else {
            for (uint32_t i = 0; i < count; ++i) {
                if (std::find(valid_values.begin(), valid_values.end(), array[i]) == valid_values.end()) {
                    skip |= LogError(vuid, device, array_loc.dot(i),
                                     "(%" PRIu32
                                     ") does not fall within the begin..end range of the core %s "
                                     "enumeration tokens and is not an extension added token",
                                     array[i], enumName);
                }
            }
        }

        return skip;
    }

    bool ValidateReservedFlags(const Location &loc, VkFlags value, const char *vuid) const;

    enum FlagType { kRequiredFlags, kOptionalFlags, kRequiredSingleBit, kOptionalSingleBit };

    // helper to implement validation of both 32 bit and 64 bit flags.
    template <typename FlagTypedef>
    bool ValidateFlagsImplementation(const Location &loc, const char *flag_bits_name, FlagTypedef all_flags, FlagTypedef value,
                                     const FlagType flag_type, const char *vuid, const char *flags_zero_vuid = nullptr) const;

    bool ValidateFlags(const Location &loc, const char *flag_bits_name, VkFlags all_flags, VkFlags value, const FlagType flag_type,
                       const char *vuid, const char *flags_zero_vuid = nullptr) const;

    bool ValidateFlags(const Location &loc, const char *flag_bits_name, VkFlags64 all_flags, VkFlags64 value,
                       const FlagType flag_type, const char *vuid, const char *flags_zero_vuid = nullptr) const;

    bool ValidateFlagsArray(const Location &count_loc, const Location &array_loc, const char *flag_bits_name, VkFlags all_flags,
                            uint32_t count, const VkFlags *array, bool count_required, const char *array_required_vuid) const;

    template <typename ExtensionState>
    bool ValidateExtensionReqs(const ExtensionState &extensions, const char *vuid, const char *extension_type,
                               const char *extension_name, const Location &extension_loc) const;

    bool ValidateSubpassGraphicsFlags(VkDevice device, const VkRenderPassCreateInfo2 *pCreateInfo, uint32_t subpass,
                                      VkPipelineStageFlags2 stages, const char *vuid, const Location &loc) const;

    bool ValidateCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo2 *pCreateInfo,
                                  const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass,
                                  const ErrorObject &error_obj) const;

    void RecordRenderPass(VkRenderPass renderPass, const VkRenderPassCreateInfo2 *pCreateInfo);

    // Pre/PostCallRecord declarations
    void PostCallRecordCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo *pCreateInfo,
                                        const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass,
                                        const RecordObject &record_obj) override;
    void PostCallRecordCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2 *pCreateInfo,
                                            const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass,
                                            const RecordObject &record_obj) override;
    void PostCallRecordDestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks *pAllocator,
                                         const RecordObject &record_obj) override;
    void PostCallRecordAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo *pAllocateInfo,
                                              VkCommandBuffer *pCommandBuffers, const RecordObject &record_obj) override;
    void PostCallRecordFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount,
                                          const VkCommandBuffer *pCommandBuffers, const RecordObject &record_obj) override;
    void PostCallRecordDestroyCommandPool(VkDevice device, VkCommandPool commandPool, const VkAllocationCallbacks *pAllocator,
                                          const RecordObject &record_obj) override;
    void GetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2 &pProperties) const;
    void PostCallRecordCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo *pCreateInfo,
                                    const VkAllocationCallbacks *pAllocator, VkDevice *pDevice,
                                    const RecordObject &record_obj) override;
    void PostCallRecordCreateInstance(const VkInstanceCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator,
                                      VkInstance *pInstance, const RecordObject &record_obj) override;

    void CommonPostCallRecordEnumeratePhysicalDevice(const VkPhysicalDevice *phys_devices, const int count);
    void PostCallRecordEnumeratePhysicalDevices(VkInstance instance, uint32_t *pPhysicalDeviceCount,
                                                VkPhysicalDevice *pPhysicalDevices, const RecordObject &record_obj) override;

    void PostCallRecordEnumeratePhysicalDeviceGroups(VkInstance instance, uint32_t *pPhysicalDeviceGroupCount,
                                                     VkPhysicalDeviceGroupProperties *pPhysicalDeviceGroupProperties,
                                                     const RecordObject &record_obj) override;

    bool ValidateString(const Location &loc, const std::string &vuid, const char *validateString) const;

    bool ValidateCoarseSampleOrderCustomNV(const VkCoarseSampleOrderCustomNV *order, const Location &order_loc) const;

    bool ValidateGeometryTrianglesNV(const VkGeometryTrianglesNV &triangles, VkAccelerationStructureNV object_handle,
                                     const Location &loc) const;
    bool ValidateGeometryAABBNV(const VkGeometryAABBNV &geometry, VkAccelerationStructureNV object_handle,
                                const Location &loc) const;
    bool ValidateGeometryNV(const VkGeometryNV &geometry, VkAccelerationStructureNV object_handle, const Location &loc) const;
    bool ValidateAccelerationStructureInfoNV(const VkAccelerationStructureInfoNV &info, VkAccelerationStructureNV object_handle,
                                             const Location &loc) const;
    bool ValidateSwapchainCreateInfo(VkSwapchainCreateInfoKHR const *pCreateInfo, const Location &loc) const;

    bool OutputExtensionError(const Location &loc, const std::string &extension_name) const;

    void PreCallRecordDestroyInstance(VkInstance instance, const VkAllocationCallbacks *pAllocator) override;

    bool manual_PreCallValidateCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo *pCreateInfo,
                                               const VkAllocationCallbacks *pAllocator, VkQueryPool *pQueryPool,
                                               const ErrorObject &error_obj) const;

    bool manual_PreCallValidateCreateInstance(const VkInstanceCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator,
                                              VkInstance *pInstance, const ErrorObject &error_obj) const;

    bool manual_PreCallValidateCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo *pCreateInfo,
                                            const VkAllocationCallbacks *pAllocator, VkDevice *pDevice,
                                            const ErrorObject &error_obj) const;

    bool manual_PreCallValidateCreateBuffer(VkDevice device, const VkBufferCreateInfo *pCreateInfo,
                                            const VkAllocationCallbacks *pAllocator, VkBuffer *pBuffer,
                                            const ErrorObject &error_obj) const;

    bool manual_PreCallValidateCreateImage(VkDevice device, const VkImageCreateInfo *pCreateInfo,
                                           const VkAllocationCallbacks *pAllocator, VkImage *pImage,
                                           const ErrorObject &error_obj) const;

    bool manual_PreCallValidateCreateImageView(VkDevice device, const VkImageViewCreateInfo *pCreateInfo,
                                               const VkAllocationCallbacks *pAllocator, VkImageView *pView,
                                               const ErrorObject &error_obj) const;

    bool ValidateViewport(const VkViewport &viewport, VkCommandBuffer object, const Location &loc) const;

    bool manual_PreCallValidateCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo *pCreateInfo,
                                                    const VkAllocationCallbacks *pAllocator, VkPipelineLayout *pPipelineLayout,
                                                    const ErrorObject &error_obj) const;

    bool ValidatePipelineShaderStageCreateInfo(const VkPipelineShaderStageCreateInfo *pCreateInfo, const Location &loc) const;
    bool ValidatePipelineTessellationStateCreateInfo(const VkPipelineTessellationStateCreateInfo &info, uint32_t index,
                                                     const Location &loc) const;
    bool ValidatePipelineVertexInputStateCreateInfo(const VkPipelineVertexInputStateCreateInfo &info, uint32_t index,
                                                    const Location &loc) const;
    bool ValidatePipelineViewportStateCreateInfo(const VkPipelineViewportStateCreateInfo &info, uint32_t index,
                                                 const Location &loc) const;
    bool ValidatePipelineMultisampleStateCreateInfo(const VkPipelineMultisampleStateCreateInfo &info, uint32_t index,
                                                    const Location &loc) const;
    bool ValidatePipelineColorBlendAttachmentState(const VkPipelineColorBlendAttachmentState &attachment_state, uint32_t pipe_index,
                                                   uint32_t attachment_index, const Location &loc) const;
    bool ValidatePipelineColorBlendStateCreateInfo(const VkPipelineColorBlendStateCreateInfo &info, uint32_t index,
                                                   const Location &loc) const;
    bool ValidatePipelineDepthStencilStateCreateInfo(const VkPipelineDepthStencilStateCreateInfo &info, uint32_t index,
                                                     const Location &loc) const;
    bool ValidatePipelineInputAssemblyStateCreateInfo(const VkPipelineInputAssemblyStateCreateInfo &info, uint32_t index,
                                                      const Location &loc) const;
    bool manual_PreCallValidateCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                       const VkGraphicsPipelineCreateInfo *pCreateInfos,
                                                       const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                       const ErrorObject &error_obj) const;
    bool manual_PreCallValidateCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                      const VkComputePipelineCreateInfo *pCreateInfos,
                                                      const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                      const ErrorObject &error_obj) const;

    bool manual_PreCallValidateCreateSampler(VkDevice device, const VkSamplerCreateInfo *pCreateInfo,
                                             const VkAllocationCallbacks *pAllocator, VkSampler *pSampler,
                                             const ErrorObject &error_obj) const;
    bool ValidateMutableDescriptorTypeCreateInfo(const VkDescriptorSetLayoutCreateInfo &create_info,
                                                 const VkMutableDescriptorTypeCreateInfoEXT &mutable_create_info,
                                                 const Location &loc) const;
    bool manual_PreCallValidateCreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
                                                         const VkAllocationCallbacks *pAllocator, VkDescriptorSetLayout *pSetLayout,
                                                         const ErrorObject &error_obj) const;

    bool manual_PreCallValidateCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo *pCreateInfo,
                                               const VkAllocationCallbacks *pAllocator, VkSemaphore *pSemaphore,
                                               const ErrorObject &error_obj) const;

    bool manual_PreCallValidateCreateEvent(VkDevice device, const VkEventCreateInfo *pCreateInfo,
                                           const VkAllocationCallbacks *pAllocator, VkEvent *pEvent,
                                           const ErrorObject &error_obj) const;

    bool manual_PreCallValidateCreateBufferView(VkDevice device, const VkBufferViewCreateInfo *pCreateInfo,
                                                const VkAllocationCallbacks *pAllocator, VkBufferView *pBufferView,
                                                const ErrorObject &error_obj) const;

#ifdef VK_USE_PLATFORM_METAL_EXT
    bool ExportMetalObjectsPNextUtil(VkExportMetalObjectTypeFlagBitsEXT bit, const char *vuid, const Location &loc,
                                     const char *sType, const void *pNext) const;
#endif  // VK_USE_PLATFORM_METAL_EXT

    bool ValidateWriteDescriptorSet(const Location &loc, const uint32_t descriptorWriteCount,
                                    const VkWriteDescriptorSet *pDescriptorWrites, const bool isPushDescriptor) const;
    bool manual_PreCallValidateUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount,
                                                    const VkWriteDescriptorSet *pDescriptorWrites, uint32_t descriptorCopyCount,
                                                    const VkCopyDescriptorSet *pDescriptorCopies,
                                                    const ErrorObject &error_obj) const;

    bool manual_PreCallValidateFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t descriptorSetCount,
                                                  const VkDescriptorSet *pDescriptorSets, const ErrorObject &error_obj) const;

    bool manual_PreCallValidateCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo *pCreateInfo,
                                                const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass,
                                                const ErrorObject &error_obj) const;

    bool manual_PreCallValidateCreateRenderPass2(VkDevice device, const VkRenderPassCreateInfo2 *pCreateInfo,
                                                 const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass,
                                                 const ErrorObject &error_obj) const;

    bool manual_PreCallValidateCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2 *pCreateInfo,
                                                    const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass,
                                                    const ErrorObject &error_obj) const;

    bool manual_PreCallValidateFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount,
                                                  const VkCommandBuffer *pCommandBuffers, const ErrorObject &error_obj) const;

    bool manual_PreCallValidateBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo *pBeginInfo,
                                                  const ErrorObject &error_obj) const;

    bool manual_PreCallValidateCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                              const VkViewport *pViewports, const ErrorObject &error_obj) const;

    bool manual_PreCallValidateCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount,
                                             const VkRect2D *pScissors, const ErrorObject &error_obj) const;
    bool manual_PreCallValidateCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth, const ErrorObject &error_obj) const;

    bool manual_PreCallValidateCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                                   const VkClearAttachment *pAttachments, uint32_t rectCount,
                                                   const VkClearRect *pRects, const ErrorObject &error_obj) const;

    bool manual_PreCallValidateGetPhysicalDeviceImageFormatProperties2(VkPhysicalDevice physicalDevice,
                                                                       const VkPhysicalDeviceImageFormatInfo2 *pImageFormatInfo,
                                                                       VkImageFormatProperties2 *pImageFormatProperties,
                                                                       const ErrorObject &error_obj) const;
    bool manual_PreCallValidateGetPhysicalDeviceImageFormatProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                          const VkPhysicalDeviceImageFormatInfo2 *pImageFormatInfo,
                                                                          VkImageFormatProperties2 *pImageFormatProperties,
                                                                          const ErrorObject &error_obj) const;
    bool manual_PreCallValidateGetPhysicalDeviceImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format,
                                                                      VkImageType type, VkImageTiling tiling,
                                                                      VkImageUsageFlags usage, VkImageCreateFlags flags,
                                                                      VkImageFormatProperties *pImageFormatProperties,
                                                                      const ErrorObject &error_obj) const;

    bool manual_PreCallValidateCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer,
                                             uint32_t regionCount, const VkBufferCopy *pRegions,
                                             const ErrorObject &error_obj) const;

    bool manual_PreCallValidateCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2KHR *pCopyBufferInfo,
                                                 const ErrorObject &error_obj) const;

    bool manual_PreCallValidateCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2 *pCopyBufferInfo,
                                              const ErrorObject &error_obj) const;

    bool manual_PreCallValidateCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                               VkDeviceSize dataSize, const void *pData, const ErrorObject &error_obj) const;

    bool manual_PreCallValidateCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                             VkDeviceSize size, uint32_t data, const ErrorObject &error_obj) const;

    bool manual_PreCallValidateCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR *pCreateInfo,
                                                  const VkAllocationCallbacks *pAllocator, VkSwapchainKHR *pSwapchain,
                                                  const ErrorObject &error_obj) const;
    bool manual_PreCallValidateCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount,
                                                         const VkSwapchainCreateInfoKHR *pCreateInfos,
                                                         const VkAllocationCallbacks *pAllocator, VkSwapchainKHR *pSwapchains,
                                                         const ErrorObject &error_obj) const;
    bool manual_PreCallValidateQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR *pPresentInfo,
                                               const ErrorObject &error_obj) const;
    bool manual_PreCallValidateCreateDisplayModeKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                    const VkDisplayModeCreateInfoKHR *pCreateInfo,
                                                    const VkAllocationCallbacks *pAllocator, VkDisplayModeKHR *pMode,
                                                    const ErrorObject &error_obj) const;

#ifdef VK_USE_PLATFORM_WIN32_KHR
    bool manual_PreCallValidateCreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR *pCreateInfo,
                                                     const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface,
                                                     const ErrorObject &error_obj) const;
#endif  // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    bool manual_PreCallValidateCreateWaylandSurfaceKHR(VkInstance instance, const VkWaylandSurfaceCreateInfoKHR *pCreateInfo,
                                                       const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface,
                                                       const ErrorObject &error_obj) const;
#endif  // VK_USE_PLATFORM_WAYLAND_KHR

#ifdef VK_USE_PLATFORM_XCB_KHR
    bool manual_PreCallValidateCreateXcbSurfaceKHR(VkInstance instance, const VkXcbSurfaceCreateInfoKHR *pCreateInfo,
                                                   const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface,
                                                   const ErrorObject &error_obj) const;
#endif  // VK_USE_PLATFORM_WAYLAND_KHR

#ifdef VK_USE_PLATFORM_XLIB_KHR
    bool manual_PreCallValidateCreateXlibSurfaceKHR(VkInstance instance, const VkXlibSurfaceCreateInfoKHR *pCreateInfo,
                                                    const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface,
                                                    const ErrorObject &error_obj) const;
#endif  // VK_USE_PLATFORM_WAYLAND_KHR

    bool manual_PreCallValidateCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo *pCreateInfo,
                                                    const VkAllocationCallbacks *pAllocator, VkDescriptorPool *pDescriptorPool,
                                                    const ErrorObject &error_obj) const;
    bool manual_PreCallValidateCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                       VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                                       const VkWriteDescriptorSet *pDescriptorWrites,
                                                       const ErrorObject &error_obj) const;
    bool manual_PreCallValidateCmdSetExclusiveScissorNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                                        uint32_t exclusiveScissorCount, const VkRect2D *pExclusiveScissors,
                                                        const ErrorObject &error_obj) const;
    bool manual_PreCallValidateCmdSetViewportShadingRatePaletteNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                                  uint32_t viewportCount,
                                                                  const VkShadingRatePaletteNV *pShadingRatePalettes,
                                                                  const ErrorObject &error_obj) const;

    bool manual_PreCallValidateCmdSetCoarseSampleOrderNV(VkCommandBuffer commandBuffer, VkCoarseSampleOrderTypeNV sampleOrderType,
                                                         uint32_t customSampleOrderCount,
                                                         const VkCoarseSampleOrderCustomNV *pCustomSampleOrders,
                                                         const ErrorObject &error_obj) const;

    bool manual_PreCallValidateEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char *pLayerName,
                                                                  uint32_t *pPropertyCount,
                                                                  VkExtensionProperties *pProperties) const;
    bool manual_PreCallValidateAllocateMemory(VkDevice device, const VkMemoryAllocateInfo *pAllocateInfo,
                                              const VkAllocationCallbacks *pAllocator, VkDeviceMemory *pMemory,
                                              const ErrorObject &error_obj) const;

    bool manual_PreCallValidateCreateAccelerationStructureNV(VkDevice device,
                                                             const VkAccelerationStructureCreateInfoNV *pCreateInfo,
                                                             const VkAllocationCallbacks *pAllocator,
                                                             VkAccelerationStructureNV *pAccelerationStructure,
                                                             const ErrorObject &error_obj) const;
    bool manual_PreCallValidateCreateAccelerationStructureKHR(VkDevice device,
                                                              const VkAccelerationStructureCreateInfoKHR *pCreateInfo,
                                                              const VkAllocationCallbacks *pAllocator,
                                                              VkAccelerationStructureKHR *pAccelerationStructure,
                                                              const ErrorObject &error_obj) const;
    bool manual_PreCallValidateCmdBuildAccelerationStructureNV(VkCommandBuffer commandBuffer,
                                                               const VkAccelerationStructureInfoNV *pInfo, VkBuffer instanceData,
                                                               VkDeviceSize instanceOffset, VkBool32 update,
                                                               VkAccelerationStructureNV dst, VkAccelerationStructureNV src,
                                                               VkBuffer scratch, VkDeviceSize scratchOffset,
                                                               const ErrorObject &error_obj) const;
    bool manual_PreCallValidateGetAccelerationStructureHandleNV(VkDevice device, VkAccelerationStructureNV accelerationStructure,
                                                                size_t dataSize, void *pData, const ErrorObject &error_obj) const;

    bool manual_PreCallValidateCmdWriteAccelerationStructuresPropertiesNV(VkCommandBuffer commandBuffer,
                                                                          uint32_t accelerationStructureCount,
                                                                          const VkAccelerationStructureNV *pAccelerationStructures,
                                                                          VkQueryType queryType, VkQueryPool queryPool,
                                                                          uint32_t firstQuery, const ErrorObject &error_obj) const;
    bool manual_PreCallValidateCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                           const VkRayTracingPipelineCreateInfoNV *pCreateInfos,
                                                           const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                           const ErrorObject &error_obj) const;
    bool manual_PreCallValidateCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                            VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                            const VkRayTracingPipelineCreateInfoKHR *pCreateInfos,
                                                            const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                            const ErrorObject &error_obj) const;
    bool manual_PreCallValidateCmdSetViewportWScalingNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                        uint32_t viewportCount, const VkViewportWScalingNV *pViewportWScalings,
                                                        const ErrorObject &error_obj) const;

    bool manual_PreCallValidateCreateShadersEXT(VkDevice device, uint32_t createInfoCount,
                                                const VkShaderCreateInfoEXT *pCreateInfos, const VkAllocationCallbacks *pAllocator,
                                                VkShaderEXT *pShaders, const ErrorObject &error_obj) const;
    bool manual_PreCallValidateGetShaderBinaryDataEXT(VkDevice device, VkShaderEXT shader, size_t *pDataSize, void *pData,
                                                      const ErrorObject &error_obj) const;

#ifdef VK_USE_PLATFORM_WIN32_KHR
    bool PreCallValidateGetDeviceGroupSurfacePresentModes2EXT(VkDevice device, const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo,
                                                              VkDeviceGroupPresentModeFlagsKHR *pModes,
                                                              const ErrorObject &error_obj) const override;
#endif  // VK_USE_PLATFORM_WIN32_KHR

    bool manual_PreCallValidateCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo *pCreateInfo,
                                                 const VkAllocationCallbacks *pAllocator, VkFramebuffer *pFramebuffer,
                                                 const ErrorObject &error_obj) const;

    bool manual_PreCallValidateCmdSetLineStippleEXT(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor,
                                                    uint16_t lineStipplePattern, const ErrorObject &error_obj) const;

    bool manual_PreCallValidateCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                  VkIndexType indexType, const ErrorObject &error_obj) const;
    bool manual_PreCallValidateCmdBindIndexBuffer2KHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                      VkDeviceSize size, VkIndexType indexType, const ErrorObject &error_obj) const;
    bool manual_PreCallValidateCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                                    const VkBuffer *pBuffers, const VkDeviceSize *pOffsets,
                                                    const ErrorObject &error_obj) const;

    bool manual_PreCallValidateSetDebugUtilsObjectNameEXT(VkDevice device, const VkDebugUtilsObjectNameInfoEXT *pNameInfo,
                                                          const ErrorObject &error_obj) const;

    bool manual_PreCallValidateSetDebugUtilsObjectTagEXT(VkDevice device, const VkDebugUtilsObjectTagInfoEXT *pTagInfo,
                                                         const ErrorObject &error_obj) const;

    bool manual_PreCallValidateAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout,
                                                   VkSemaphore semaphore, VkFence fence, uint32_t *pImageIndex,
                                                   const ErrorObject &error_obj) const;

    bool manual_PreCallValidateAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR *pAcquireInfo,
                                                    uint32_t *pImageIndex, const ErrorObject &error_obj) const;

    bool manual_PreCallValidateCmdBindTransformFeedbackBuffersEXT(VkCommandBuffer commandBuffer, uint32_t firstBinding,
                                                                  uint32_t bindingCount, const VkBuffer *pBuffers,
                                                                  const VkDeviceSize *pOffsets, const VkDeviceSize *pSizes,
                                                                  const ErrorObject &error_obj) const;

    bool manual_PreCallValidateCmdBeginTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                                            uint32_t counterBufferCount, const VkBuffer *pCounterBuffers,
                                                            const VkDeviceSize *pCounterBufferOffsets,
                                                            const ErrorObject &error_obj) const;

    bool manual_PreCallValidateCmdEndTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                                          uint32_t counterBufferCount, const VkBuffer *pCounterBuffers,
                                                          const VkDeviceSize *pCounterBufferOffsets,
                                                          const ErrorObject &error_obj) const;

    bool manual_PreCallValidateCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount,
                                                           uint32_t firstInstance, VkBuffer counterBuffer,
                                                           VkDeviceSize counterBufferOffset, uint32_t counterOffset,
                                                           uint32_t vertexStride, const ErrorObject &error_obj) const;

    bool manual_PreCallValidateCreateSamplerYcbcrConversion(VkDevice device, const VkSamplerYcbcrConversionCreateInfo *pCreateInfo,
                                                            const VkAllocationCallbacks *pAllocator,
                                                            VkSamplerYcbcrConversion *pYcbcrConversion,
                                                            const ErrorObject &error_obj) const;
    bool manual_PreCallValidateCreateSamplerYcbcrConversionKHR(VkDevice device,
                                                               const VkSamplerYcbcrConversionCreateInfo *pCreateInfo,
                                                               const VkAllocationCallbacks *pAllocator,
                                                               VkSamplerYcbcrConversion *pYcbcrConversion,
                                                               const ErrorObject &error_obj) const;

    bool manual_PreCallValidateGetMemoryFdKHR(VkDevice device, const VkMemoryGetFdInfoKHR *pGetFdInfo, int *pFd,
                                              const ErrorObject &error_obj) const;
    bool manual_PreCallValidateGetMemoryFdPropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, int fd,
                                                        VkMemoryFdPropertiesKHR *pMemoryFdProperties,
                                                        const ErrorObject &error_obj) const;
    bool ValidateExternalSemaphoreHandleType(VkSemaphore semaphore, const char *vuid, const Location &handle_type_loc,
                                             VkExternalSemaphoreHandleTypeFlagBits handle_type,
                                             VkExternalSemaphoreHandleTypeFlags allowed_types) const;
    bool ValidateExternalFenceHandleType(VkFence fence, const char *vuid, const Location &handle_type_loc,
                                         VkExternalFenceHandleTypeFlagBits handle_type,
                                         VkExternalFenceHandleTypeFlags allowed_types) const;
    bool manual_PreCallValidateImportSemaphoreFdKHR(VkDevice device, const VkImportSemaphoreFdInfoKHR *pImportSemaphoreFdInfo,
                                                    const ErrorObject &error_obj) const;
    bool manual_PreCallValidateGetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR *pGetFdInfo, int *pFd,
                                                 const ErrorObject &error_obj) const;

    bool manual_PreCallValidateImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR *pImportFenceFdInfo,
                                                const ErrorObject &error_obj) const;
    bool manual_PreCallValidateGetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR *pGetFdInfo, int *pFd,
                                             const ErrorObject &error_obj) const;

#ifdef VK_USE_PLATFORM_WIN32_KHR
    bool manual_PreCallValidateGetMemoryWin32HandlePropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType,
                                                                 HANDLE handle,
                                                                 VkMemoryWin32HandlePropertiesKHR *pMemoryWin32HandleProperties,
                                                                 const ErrorObject &error_obj) const;
    bool manual_PreCallValidateImportSemaphoreWin32HandleKHR(
        VkDevice device, const VkImportSemaphoreWin32HandleInfoKHR *pImportSemaphoreWin32HandleInfo,
        const ErrorObject &error_obj) const;
    bool manual_PreCallValidateGetSemaphoreWin32HandleKHR(VkDevice device,
                                                          const VkSemaphoreGetWin32HandleInfoKHR *pGetWin32HandleInfo,
                                                          HANDLE *pHandle, const ErrorObject &error_obj) const;

    bool manual_PreCallValidateImportFenceWin32HandleKHR(VkDevice device,
                                                         const VkImportFenceWin32HandleInfoKHR *pImportFenceWin32HandleInfo,
                                                         const ErrorObject &error_obj) const;
    bool manual_PreCallValidateGetFenceWin32HandleKHR(VkDevice device, const VkFenceGetWin32HandleInfoKHR *pGetWin32HandleInfo,
                                                      HANDLE *pHandle, const ErrorObject &error_obj) const;
#endif

    bool manual_PreCallValidateCopyAccelerationStructureToMemoryKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                                    const VkCopyAccelerationStructureToMemoryInfoKHR *pInfo,
                                                                    const ErrorObject &error_obj) const;

    bool manual_PreCallValidateCmdCopyAccelerationStructureToMemoryKHR(VkCommandBuffer commandBuffer,
                                                                       const VkCopyAccelerationStructureToMemoryInfoKHR *pInfo,
                                                                       const ErrorObject &error_obj) const;

    bool manual_PreCallValidateCopyAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                            const VkCopyAccelerationStructureInfoKHR *pInfo,
                                                            const ErrorObject &error_obj) const;

    bool manual_PreCallValidateCmdCopyAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                                               const VkCopyAccelerationStructureInfoKHR *pInfo,
                                                               const ErrorObject &error_obj) const;
    bool ValidateCopyAccelerationStructureInfoKHR(const VkCopyAccelerationStructureInfoKHR *pInfo, const VulkanTypedHandle &handle,
                                                  const Location &info_loc) const;
    bool ValidateCopyMemoryToAccelerationStructureInfoKHR(const VkCopyMemoryToAccelerationStructureInfoKHR *pInfo,
                                                          const VulkanTypedHandle &handle, const Location &loc) const;

    bool manual_PreCallValidateCopyMemoryToAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                                    const VkCopyMemoryToAccelerationStructureInfoKHR *pInfo,
                                                                    const ErrorObject &error_obj) const;
    bool manual_PreCallValidateCmdCopyMemoryToAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                                                       const VkCopyMemoryToAccelerationStructureInfoKHR *pInfo,
                                                                       const ErrorObject &error_obj) const;

    bool manual_PreCallValidateCmdWriteAccelerationStructuresPropertiesKHR(
        VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount,
        const VkAccelerationStructureKHR *pAccelerationStructures, VkQueryType queryType, VkQueryPool queryPool,
        uint32_t firstQuery, const ErrorObject &error_obj) const;
    bool manual_PreCallValidateWriteAccelerationStructuresPropertiesKHR(VkDevice device, uint32_t accelerationStructureCount,
                                                                        const VkAccelerationStructureKHR *pAccelerationStructures,
                                                                        VkQueryType queryType, size_t dataSize, void *pData,
                                                                        size_t stride, const ErrorObject &error_obj) const;
    bool manual_PreCallValidateGetRayTracingCaptureReplayShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline,
                                                                               uint32_t firstGroup, uint32_t groupCount,
                                                                               size_t dataSize, void *pData,
                                                                               const ErrorObject &error_obj) const;

    bool manual_PreCallValidateCmdBuildAccelerationStructuresIndirectKHR(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                                                         const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
                                                                         const VkDeviceAddress *pIndirectDeviceAddresses,
                                                                         const uint32_t *pIndirectStrides,
                                                                         const uint32_t *const *ppMaxPrimitiveCounts,
                                                                         const ErrorObject &error_obj) const;

    bool manual_PreCallValidateGetDeviceAccelerationStructureCompatibilityKHR(
        VkDevice device, const VkAccelerationStructureVersionInfoKHR *pVersionInfo,
        VkAccelerationStructureCompatibilityKHR *pCompatibility, const ErrorObject &error_obj) const;
    bool ValidateCmdSetViewportWithCount(VkCommandBuffer commandBuffer, uint32_t viewportCount, const VkViewport *pViewports,
                                         const ErrorObject &error_obj) const;
    bool manual_PreCallValidateCmdSetViewportWithCountEXT(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                                          const VkViewport *pViewports, const ErrorObject &error_obj) const;
    bool manual_PreCallValidateCmdSetViewportWithCount(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                                       const VkViewport *pViewports, const ErrorObject &error_obj) const;

    bool ValidateCmdSetScissorWithCount(VkCommandBuffer commandBuffer, uint32_t scissorCount, const VkRect2D *pScissors,
                                        const ErrorObject &error_obj) const;
    bool manual_PreCallValidateCmdSetScissorWithCountEXT(VkCommandBuffer commandBuffer, uint32_t scissorCount,
                                                         const VkRect2D *pScissors, const ErrorObject &error_obj) const;
    bool manual_PreCallValidateCmdSetScissorWithCount(VkCommandBuffer commandBuffer, uint32_t scissorCount,
                                                      const VkRect2D *pScissors, const ErrorObject &error_obj) const;
    bool manual_PreCallValidateCmdBindVertexBuffers2EXT(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                                        const VkBuffer *pBuffers, const VkDeviceSize *pOffsets,
                                                        const VkDeviceSize *pSizes, const VkDeviceSize *pStrides,
                                                        const ErrorObject &error_obj) const;
    bool manual_PreCallValidateCmdBindVertexBuffers2(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                                     const VkBuffer *pBuffers, const VkDeviceSize *pOffsets,
                                                     const VkDeviceSize *pSizes, const VkDeviceSize *pStrides,
                                                     const ErrorObject &error_obj) const;
    bool ValidateAccelerationStructureBuildGeometryInfoKHR(const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
                                                           uint32_t infoCount, uint64_t total_primitive_count,
                                                           const VulkanTypedHandle &handle, const Location &loc) const;
    bool manual_PreCallValidateCmdBuildAccelerationStructuresKHR(
        VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
        const VkAccelerationStructureBuildRangeInfoKHR *const *ppBuildRangeInfos, const ErrorObject &error_obj) const;

    bool manual_PreCallValidateBuildAccelerationStructuresKHR(
        VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount,
        const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
        const VkAccelerationStructureBuildRangeInfoKHR *const *ppBuildRangeInfos, const ErrorObject &error_obj) const;

    bool manual_PreCallValidateGetAccelerationStructureBuildSizesKHR(VkDevice device, VkAccelerationStructureBuildTypeKHR buildType,
                                                                     const VkAccelerationStructureBuildGeometryInfoKHR *pBuildInfo,
                                                                     const uint32_t *pMaxPrimitiveCounts,
                                                                     VkAccelerationStructureBuildSizesInfoKHR *pSizeInfo,
                                                                     const ErrorObject &error_obj) const;

    bool manual_PreCallValidateCmdSetVertexInputEXT(VkCommandBuffer commandBuffer, uint32_t vertexBindingDescriptionCount,
                                                    const VkVertexInputBindingDescription2EXT *pVertexBindingDescriptions,
                                                    uint32_t vertexAttributeDescriptionCount,
                                                    const VkVertexInputAttributeDescription2EXT *pVertexAttributeDescriptions,
                                                    const ErrorObject &error_obj) const;

    bool manual_PreCallValidateCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout,
                                                VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void *pValues,
                                                const ErrorObject &error_obj) const;

    bool manual_PreCallValidateMergePipelineCaches(VkDevice device, VkPipelineCache dstCache, uint32_t srcCacheCount,
                                                   const VkPipelineCache *pSrcCaches, const ErrorObject &error_obj) const;

    bool manual_PreCallValidateCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                  const VkClearColorValue *pColor, uint32_t rangeCount,
                                                  const VkImageSubresourceRange *pRanges, const ErrorObject &error_obj) const;

    bool ValidateCmdBeginRenderPass(const VkRenderPassBeginInfo *const rp_begin, const ErrorObject &error_obj) const;
    bool manual_PreCallValidateCmdBeginRenderPass(VkCommandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin, VkSubpassContents,
                                                  const ErrorObject &error_obj) const;
    bool manual_PreCallValidateCmdBeginRenderPass2KHR(VkCommandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                      const VkSubpassBeginInfo *, const ErrorObject &error_obj) const;
    bool manual_PreCallValidateCmdBeginRenderPass2(VkCommandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                   const VkSubpassBeginInfo *, const ErrorObject &error_obj) const;
    bool manual_PreCallValidateCmdBeginRenderingKHR(VkCommandBuffer commandBuffer, const VkRenderingInfo *pRenderingInfo,
                                                    const ErrorObject &error_obj) const;
    bool manual_PreCallValidateCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo *pRenderingInfo,
                                                 const ErrorObject &error_obj) const;

    bool manual_PreCallValidateCmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer, uint32_t firstDiscardRectangle,
                                                         uint32_t discardRectangleCount, const VkRect2D *pDiscardRectangles,
                                                         const ErrorObject &error_obj) const;
    bool manual_PreCallValidateGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                                   size_t dataSize, void *pData, VkDeviceSize stride, VkQueryResultFlags flags,
                                                   const ErrorObject &error_obj) const;
    bool manual_PreCallValidateCmdBeginConditionalRenderingEXT(VkCommandBuffer commandBuffer,
                                                               const VkConditionalRenderingBeginInfoEXT *pConditionalRenderingBegin,
                                                               const ErrorObject &error_obj) const;

    bool ValidateDeviceImageMemoryRequirements(VkDevice device, const VkDeviceImageMemoryRequirementsKHR *pInfo,
                                               const Location &loc) const;

    bool manual_PreCallValidateGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                  uint32_t *pSurfaceFormatCount,
                                                                  VkSurfaceFormatKHR *pSurfaceFormats,
                                                                  const ErrorObject &error_obj) const;

    bool manual_PreCallValidateGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                       uint32_t *pPresentModeCount, VkPresentModeKHR *pPresentModes,
                                                                       const ErrorObject &error_obj) const;

    bool manual_PreCallValidateGetPhysicalDeviceSurfaceCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                                        const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo,
                                                                        VkSurfaceCapabilities2KHR *pSurfaceCapabilities,
                                                                        const ErrorObject &error_obj) const;

    bool manual_PreCallValidateGetPhysicalDeviceSurfaceFormats2KHR(VkPhysicalDevice physicalDevice,
                                                                   const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo,
                                                                   uint32_t *pSurfaceFormatCount,
                                                                   VkSurfaceFormat2KHR *pSurfaceFormats,
                                                                   const ErrorObject &error_obj) const;

    bool manual_PreCallValidateCmdSetDiscardRectangleEnableEXT(VkCommandBuffer commandBuffer, VkBool32 discardRectangleEnable,
                                                               const ErrorObject &error_obj) const;
    bool manual_PreCallValidateCmdSetDiscardRectangleModeEXT(VkCommandBuffer commandBuffer,
                                                             VkDiscardRectangleModeEXT discardRectangleMode,
                                                             const ErrorObject &error_obj) const;
    bool manual_PreCallValidateCmdSetExclusiveScissorEnableNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                                              uint32_t exclusiveScissorCount,
                                                              const VkBool32 *pExclusiveScissorEnables,
                                                              const ErrorObject &error_obj) const;

#ifdef VK_USE_PLATFORM_WIN32_KHR
    bool manual_PreCallValidateGetPhysicalDeviceSurfacePresentModes2EXT(VkPhysicalDevice physicalDevice,
                                                                        const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo,
                                                                        uint32_t *pPresentModeCount,
                                                                        VkPresentModeKHR *pPresentModes,
                                                                        const ErrorObject &error_obj) const;
#endif  // VK_USE_PLATFORM_WIN32_KHR

    bool manual_PreCallValidateGetDeviceImageMemoryRequirements(VkDevice device, const VkDeviceImageMemoryRequirements *pInfo,
                                                                VkMemoryRequirements2 *pMemoryRequirements,
                                                                const ErrorObject &error_obj) const;
    bool manual_PreCallValidateGetDeviceImageMemoryRequirementsKHR(VkDevice device, const VkDeviceImageMemoryRequirements *pInfo,
                                                                   VkMemoryRequirements2 *pMemoryRequirements,
                                                                   const ErrorObject &error_obj) const;
    bool manual_PreCallValidateGetDeviceImageSparseMemoryRequirements(VkDevice device, const VkDeviceImageMemoryRequirements *pInfo,
                                                                      uint32_t *pSparseMemoryRequirementCount,
                                                                      VkSparseImageMemoryRequirements2 *pSparseMemoryRequirements,
                                                                      const ErrorObject &error_obj) const;
    bool manual_PreCallValidateGetDeviceImageSparseMemoryRequirementsKHR(
        VkDevice device, const VkDeviceImageMemoryRequirements *pInfo, uint32_t *pSparseMemoryRequirementCount,
        VkSparseImageMemoryRequirements2 *pSparseMemoryRequirements, const ErrorObject &error_obj) const;

    bool manual_PreCallValidateQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo *pBindInfo,
                                               VkFence fence, const ErrorObject &error_obj) const;

#ifdef VK_USE_PLATFORM_METAL_EXT
    bool manual_PreCallValidateExportMetalObjectsEXT(VkDevice device, VkExportMetalObjectsInfoEXT *pMetalObjectsInfo,
                                                     const ErrorObject &error_obj) const;
#endif  // VK_USE_PLATFORM_METAL_EXT

#include "generated/stateless_validation_helper.h"
};  // Class StatelessValidation
