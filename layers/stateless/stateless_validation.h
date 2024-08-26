/* Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
 * Copyright (C) 2015-2024 Google Inc.
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
#include "utils/vk_layer_utils.h"
#include "generated/chassis.h"
#include "generated/device_features.h"

class StatelessValidation : public ValidationObject {
    using Func = vvl::Func;
    using Struct = vvl::Struct;
    using Field = vvl::Field;

  public:
    VkPhysicalDeviceLimits device_limits = {};
    vvl::unordered_map<VkPhysicalDevice, VkPhysicalDeviceProperties *> physical_device_properties_map;
    vvl::unordered_map<VkPhysicalDevice, vvl::unordered_set<vvl::Extension>> device_extensions_enumerated{};
    // We have a copy of this in Stateless and ValidationStateTracker, could move the ValidationObject, but we don't have a way to
    // set it at the ValidationObject level
    DeviceFeatures enabled_features = {};

    // This was a special case where it was decided to use the extension version for validation
    // https://gitlab.khronos.org/vulkan/vulkan/-/merge_requests/5671
    inline static u32 discard_rectangles_extension_version = 0;
    inline static u32 scissor_exclusive_extension_version = 0;

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
        VkPhysicalDeviceVertexAttributeDivisorPropertiesKHR vertex_attribute_divisor_props;
        VkPhysicalDeviceMaintenance4PropertiesKHR maintenance4_props;
        VkPhysicalDeviceFragmentShadingRatePropertiesKHR fragment_shading_rate_props;
        VkPhysicalDeviceDepthStencilResolveProperties depth_stencil_resolve_props;
        VkPhysicalDeviceExternalMemoryHostPropertiesEXT external_memory_host_props;
        VkPhysicalDeviceRenderPassStripedPropertiesARM renderpass_striped_props;
    };
    DeviceExtensionProperties phys_dev_ext_props = {};

    struct SubpassesUsageStates {
        vvl::unordered_set<u32> subpasses_using_color_attachment;
        vvl::unordered_set<u32> subpasses_using_depthstencil_attachment;
    };

    // Though this validation object is predominantly statless, the Framebuffer checks are greatly simplified by creating and
    // updating a map of the renderpass usage states, and these accesses need thread protection. Use a mutex separate from the
    // parent object's to maintain that functionality.
    mutable std::mutex renderpass_map_mutex;
    vvl::unordered_map<VkRenderPass, SubpassesUsageStates> renderpasses_states;

    // Constructor for stateles validation tracking
    StatelessValidation() { container_type = LayerObjectTypeParameterValidation; }
    ~StatelessValidation() {}

    bool ValidateNotZero(bool is_zero, const std::string &vuid, const Location &loc) const;

    bool ValidateRequiredPointer(const Location &loc, const void *value, const std::string &vuid) const;

    bool ValidateAllocationCallbacks(const VkAllocationCallbacks &callback, const Location &loc) const;

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
    bool ValidatePointerArray(const Location &count_loc, const Location &array_loc, const T1 *count, const T2 *array,
                              bool countPtrRequired, bool countValueRequired, bool arrayRequired,
                              const char *count_ptr_required_vuid, const char *count_required_vuid,
                              const char *array_required_vuid) const {
        bool skip = false;

        if (count == nullptr) {
            if (countPtrRequired) {
                skip |= LogError(count_ptr_required_vuid, device, count_loc, "is NULL.");
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
     * @param value Pointer to the struct to validate.
     * @param sType VkStructureType for structure validation.
     * @param required The parameter may not be NULL when true.
     * @return Boolean value indicating that the call should be skipped.
     */
    template <typename T>
    bool ValidateStructType(const Location &loc, const T *value, VkStructureType sType, bool required, const char *struct_vuid,
                            const char *stype_vuid) const {
        bool skip = false;

        if (value == nullptr) {
            if (required) {
                skip |= LogError(struct_vuid, device, loc, "is NULL.");
            }
        } else if (value->sType != sType) {
            skip |= LogError(stype_vuid, device, loc.dot(Field::sType), "must be %s.", string_VkStructureType(sType));
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
     * @param count Number of elements in the array.
     * @param array Array to validate.
     * @param sType VkStructureType for structure validation.
     * @param countRequired The 'count' parameter may not be 0 when true.
     * @param arrayRequired The 'array' parameter may not be NULL when true.
     * @return Boolean value indicating that the call should be skipped.
     */
    template <typename T>
    bool ValidateStructTypeArray(const Location &count_loc, const Location &array_loc, u32 count, const T *array,
                                 VkStructureType sType, bool countRequired, bool arrayRequired, const char *stype_vuid,
                                 const char *param_vuid, const char *count_required_vuid) const {
        bool skip = false;

        if ((array == nullptr) || (count == 0)) {
            skip |=
                ValidateArray(count_loc, array_loc, count, &array, countRequired, arrayRequired, count_required_vuid, param_vuid);
        } else {
            // Verify that all structs in the array have the correct type
            for (u32 i = 0; i < count; ++i) {
                if (array[i].sType != sType) {
                    skip |= LogError(stype_vuid, device, array_loc.dot(i).dot(Field::sType), "must be %s",
                                     string_VkStructureType(sType));
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
     * @param count Number of elements in the array.
     * @param array Array to validate.
     * @param sType VkStructureType for structure validation.
     * @param countRequired The 'count' parameter may not be 0 when true.
     * @param arrayRequired The 'array' parameter may not be NULL when true.
     * @return Boolean value indicating that the call should be skipped.
     */
    template <typename T>
    bool ValidateStructPointerTypeArray(const Location &count_loc, const Location &array_loc, u32 count, const T *array,
                                        VkStructureType sType, bool countRequired, bool arrayRequired, const char *stype_vuid,
                                        const char *param_vuid, const char *count_required_vuid) const {
        bool skip = false;

        if ((array == nullptr) || (count == 0)) {
            skip |=
                ValidateArray(count_loc, array_loc, count, &array, countRequired, arrayRequired, count_required_vuid, param_vuid);
        } else {
            // Verify that all structs in the array have the correct type
            for (u32 i = 0; i < count; ++i) {
                if (array[i]->sType != sType) {
                    skip |= LogError(stype_vuid, device, array_loc.dot(i).dot(Field::sType), "must be %s",
                                     string_VkStructureType(sType));
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
     * @param count Pointer to the number of elements in the array.
     * @param array Array to validate.
     * @param sType VkStructureType for structure validation.
     * @param countPtrRequired The 'count' parameter may not be NULL when true.
     * @param countValueRequired The '*count' value may not be 0 when true.
     * @param arrayRequired The 'array' parameter may not be NULL when true.
     * @return Boolean value indicating that the call should be skipped.
     */
    template <typename T>
    bool ValidateStructTypeArray(const Location &count_loc, const Location &array_loc, u32 *count, const T *array,
                                 VkStructureType sType, bool countPtrRequired, bool countValueRequired, bool arrayRequired,
                                 const char *stype_vuid, const char *param_vuid, const char *count_ptr_required_vuid,
                                 const char *count_required_vuid) const {
        bool skip = false;

        if (count == nullptr) {
            if (countPtrRequired) {
                skip |= LogError(count_ptr_required_vuid, device, count_loc, "is NULL.");
            }
        } else {
            skip |= ValidateStructTypeArray(count_loc, array_loc, (*count), array, sType, countValueRequired && (array != nullptr),
                                            arrayRequired, stype_vuid, param_vuid, count_required_vuid);
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
            skip |= LogError("UNASSIGNED-GeneralParameterError-RequiredHandle", device, loc, "is VK_NULL_HANDLE.");
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
    bool ValidateHandleArray(const Location &count_loc, const Location &array_loc, u32 count, const T *array, bool count_required,
                             bool array_required, const char *count_required_vuid) const {
        bool skip = false;

        if ((array == nullptr) || (count == 0)) {
            skip |= ValidateArray(count_loc, array_loc, count, &array, count_required, array_required, count_required_vuid,
                                  kVUIDUndefined);
        } else {
            // Verify that no handles in the array are VK_NULL_HANDLE
            for (u32 i = 0; i < count; ++i) {
                if (array[i] == VK_NULL_HANDLE) {
                    skip |= LogError("UNASSIGNED-GeneralParameterError-RequiredHandleArray", device, array_loc.dot(i),
                                     "is VK_NULL_HANDLE.");
                }
            }
        }

        return skip;
    }

    bool ValidateStringArray(const Location &count_loc, const Location &array_loc, u32 count, const char *const *array,
                             bool countRequired, bool arrayRequired, const char *count_required_vuid,
                             const char *array_required_vuid) const;

    bool CheckPromotedApiAgainstVulkanVersion(VkInstance instance, const Location &loc, const u32 promoted_version) const;
    bool CheckPromotedApiAgainstVulkanVersion(VkPhysicalDevice pdev, const Location &loc, const u32 promoted_version) const;
    bool SupportedByPdev(const VkPhysicalDevice physical_device, vvl::Extension extension) const;

    bool ValidatePnextFeatureStructContents(const Location &loc, const VkBaseOutStructure *header, const char *pnext_vuid,
                                            VkPhysicalDevice caller_physical_device = VK_NULL_HANDLE,
                                            bool is_const_param = true) const;
    bool ValidatePnextPropertyStructContents(const Location &loc, const VkBaseOutStructure *header, const char *pnext_vuid,
                                             VkPhysicalDevice caller_physical_device = VK_NULL_HANDLE,
                                             bool is_const_param = true) const;
    bool ValidatePnextStructContents(const Location &loc, const VkBaseOutStructure *header, const char *pnext_vuid,
                                     VkPhysicalDevice caller_physical_device = VK_NULL_HANDLE, bool is_const_param = true) const;

    bool ValidateStructPnext(const Location &loc, const void *next, size_t allowed_type_count, const VkStructureType *allowed_types,
                             u32 header_version, const char *pnext_vuid, const char *stype_vuid,
                             VkPhysicalDevice caller_physical_device = VK_NULL_HANDLE, const bool is_const_param = true) const;

    bool ValidateBool32(const Location &loc, VkBool32 value) const;

    bool ValidateBool32Array(const Location &count_loc, const Location &array_loc, u32 count, const VkBool32 *array,
                             bool countRequired, bool arrayRequired, const char *count_required_vuid,
                             const char *array_required_vuid) const;

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
     * @param name Name of the enumeration being validated.
     * @param valid_values The list of valid values for the enumeration.
     * @param value Enumeration value to validate.
     * @return Boolean value indicating that the call should be skipped.
     */
    template <typename T>
    bool ValidateRangedEnum(const Location &loc, vvl::Enum name, T value, const char *vuid) const {
        bool skip = false;
        ValidValue result = IsValidEnumValue(value);

        if (result == ValidValue::NotFound) {
            skip |= LogError(vuid, device, loc,
                             "(%" PRIu32
                             ") does not fall within the begin..end range of the %s enumeration tokens and is "
                             "not an extension added token.",
                             value, String(name));
        } else if (result == ValidValue::NoExtension && device != VK_NULL_HANDLE) {
            // If called from an instance function, there is no device to base extension support off of
            auto extensions = GetEnumExtensions(value);
            skip |= LogError(vuid, device, loc, "(%" PRIu32 ") requires the extensions %s.", value, String(extensions).c_str());
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
     * @param name Name of the enumeration being validated.
     * @param count Number of enumeration values in the array.
     * @param array Array of enumeration values to validate.
     * @param countRequired The 'count' parameter may not be 0 when true.
     * @param arrayRequired The 'array' parameter may not be NULL when true.
     * @param count_required_vuid The VUID for the '*count' parameter.
     * @param array_required_vuid The VUID for the 'array' parameter.
     * @return Boolean value indicating that the call should be skipped.
     */
    template <typename T>
    bool ValidateRangedEnumArray(const Location &count_loc, const Location &array_loc, vvl::Enum name, u32 count, const T *array,
                                 bool countRequired, bool arrayRequired, const char *count_required_vuid,
                                 const char *array_required_vuid) const {
        bool skip = false;

        if ((array == nullptr) || (count == 0)) {
            skip |= ValidateArray(count_loc, array_loc, count, &array, countRequired, arrayRequired, count_required_vuid,
                                  array_required_vuid);
        } else {
            for (u32 i = 0; i < count; ++i) {
                ValidValue result = IsValidEnumValue(array[i]);
                if (result == ValidValue::NotFound) {
                    skip |= LogError(array_required_vuid, device, array_loc.dot(i),
                                     "(%" PRIu32
                                     ") does not fall within the begin..end range of the %s enumeration tokens and is "
                                     "not an extension added token.",
                                     array[i], String(name));
                } else if (result == ValidValue::NoExtension && device != VK_NULL_HANDLE) {
                    // If called from an instance function, there is no device to base extension support off of
                    auto extensions = GetEnumExtensions(array[i]);
                    skip |= LogError(array_required_vuid, device, array_loc.dot(i), "(%" PRIu32 ") requires the extensions %s.",
                                     array[i], String(extensions).c_str());
                }
            }
        }

        return skip;
    }

    bool ValidateReservedFlags(const Location &loc, VkFlags value, const char *vuid) const;

    enum FlagType { kRequiredFlags, kOptionalFlags, kRequiredSingleBit, kOptionalSingleBit };

    // helper to implement validation of both 32 bit and 64 bit flags.
    template <typename FlagTypedef>
    bool ValidateFlagsImplementation(const Location &loc, vvl::FlagBitmask flag_bitmask, FlagTypedef all_flags, FlagTypedef value,
                                     const FlagType flag_type, const char *vuid, const char *flags_zero_vuid = nullptr) const;

    bool ValidateFlags(const Location &loc, vvl::FlagBitmask flag_bitmask, VkFlags all_flags, VkFlags value,
                       const FlagType flag_type, const char *vuid, const char *flags_zero_vuid = nullptr) const;

    bool ValidateFlags(const Location &loc, vvl::FlagBitmask flag_bitmask, VkFlags64 all_flags, VkFlags64 value,
                       const FlagType flag_type, const char *vuid, const char *flags_zero_vuid = nullptr) const;

    bool ValidateFlagsArray(const Location &count_loc, const Location &array_loc, vvl::FlagBitmask flag_bitmask, VkFlags all_flags,
                            u32 count, const VkFlags *array, bool count_required, const char *count_required_vuid,
                            const char *array_required_vuid) const;

    template <typename T>
    ValidValue IsValidEnumValue(T value) const;
    template <typename T>
    vvl::Extensions GetEnumExtensions(T value) const;

    // VkFlags values don't have a way overload, so need to use vvl::FlagBitmask
    vvl::Extensions IsValidFlagValue(vvl::FlagBitmask flag_bitmask, VkFlags value, const DeviceExtensions &device_extensions) const;
    vvl::Extensions IsValidFlag64Value(vvl::FlagBitmask flag_bitmask, VkFlags64 value,
                                       const DeviceExtensions &device_extensions) const;

    template <typename ExtensionState>
    bool ValidateExtensionReqs(const ExtensionState &extensions, const char *vuid, const char *extension_type,
                               vvl::Extension extension, const Location &extension_loc) const;

    bool ValidateSubpassGraphicsFlags(VkDevice device, const VkRenderPassCreateInfo2 *pCreateInfo, u32 subpass,
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
    void PostCallRecordCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo *pCreateInfo,
                                    const VkAllocationCallbacks *pAllocator, VkDevice *pDevice,
                                    const RecordObject &record_obj) override;
    void PostCallRecordCreateInstance(const VkInstanceCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator,
                                      VkInstance *pInstance, const RecordObject &record_obj) override;

    void CommonPostCallRecordEnumeratePhysicalDevice(const VkPhysicalDevice *phys_devices, const int count);
    void PostCallRecordEnumeratePhysicalDevices(VkInstance instance, u32 *pPhysicalDeviceCount, VkPhysicalDevice *pPhysicalDevices,
                                                const RecordObject &record_obj) override;

    void PostCallRecordEnumeratePhysicalDeviceGroups(VkInstance instance, u32 *pPhysicalDeviceGroupCount,
                                                     VkPhysicalDeviceGroupProperties *pPhysicalDeviceGroupProperties,
                                                     const RecordObject &record_obj) override;

    bool ValidateString(const Location &loc, const std::string &vuid, const char *validateString) const;

    bool ValidateCoarseSampleOrderCustomNV(const VkCoarseSampleOrderCustomNV &order, const Location &order_loc) const;

    bool ValidateGeometryTrianglesNV(const VkGeometryTrianglesNV &triangles, VkAccelerationStructureNV object_handle,
                                     const Location &loc) const;
    bool ValidateGeometryAABBNV(const VkGeometryAABBNV &geometry, VkAccelerationStructureNV object_handle,
                                const Location &loc) const;
    bool ValidateGeometryNV(const VkGeometryNV &geometry, VkAccelerationStructureNV object_handle, const Location &loc) const;
    bool ValidateAccelerationStructureInfoNV(const VkAccelerationStructureInfoNV &info, VkAccelerationStructureNV object_handle,
                                             const Location &loc) const;
    bool ValidateSwapchainCreateInfo(const VkSwapchainCreateInfoKHR &create_info, const Location &loc) const;

    bool OutputExtensionError(const Location &loc, const vvl::Extensions &exentsions) const;

    void PreCallRecordDestroyInstance(VkInstance instance, const VkAllocationCallbacks *pAllocator,
                                      const RecordObject &record_obj) override;

    bool manual_PreCallValidateGetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice,
                                                            VkPhysicalDeviceProperties2 *pProperties,
                                                            const ErrorObject &error_obj) const;
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
    bool ValidateCreateImageSparse(const VkImageCreateInfo &create_info, const Location &create_info_loc) const;
    bool ValidateCreateImageFragmentShadingRate(const VkImageCreateInfo &create_info, const Location &create_info_loc) const;
    bool ValidateCreateImageCornerSampled(const VkImageCreateInfo &create_info, const Location &create_info_loc) const;
    bool ValidateCreateImageStencilUsage(const VkImageCreateInfo &create_info, const Location &create_info_loc) const;
    bool ValidateCreateImageSwapchain(const VkImageCreateInfo &create_info, const Location &create_info_loc) const;
    bool ValidateCreateImageMetalObject(const VkImageCreateInfo &create_info, const Location &create_info_loc) const;

    bool manual_PreCallValidateCreateImageView(VkDevice device, const VkImageViewCreateInfo *pCreateInfo,
                                               const VkAllocationCallbacks *pAllocator, VkImageView *pView,
                                               const ErrorObject &error_obj) const;

    bool manual_PreCallValidateGetDeviceImageSubresourceLayoutKHR(VkDevice device, const VkDeviceImageSubresourceInfoKHR *pInfo,
                                                                  VkSubresourceLayout2KHR *pLayout,
                                                                  const ErrorObject &error_obj) const;

    bool ValidateViewport(const VkViewport &viewport, VkCommandBuffer object, const Location &loc) const;

    bool manual_PreCallValidateCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo *pCreateInfo,
                                                    const VkAllocationCallbacks *pAllocator, VkPipelineLayout *pPipelineLayout,
                                                    const ErrorObject &error_obj) const;

    bool ValidatePipelineShaderStageCreateInfoCommon(const VkPipelineShaderStageCreateInfo &create_info, const Location &loc) const;
    bool ValidatePipelineRenderingCreateInfo(const VkPipelineRenderingCreateInfo &rendering_struct, const Location &loc) const;
    bool ValidateCreateGraphicsPipelinesFlags(const VkPipelineCreateFlags2KHR flags, const Location &flags_loc) const;
    bool manual_PreCallValidateCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, u32 createInfoCount,
                                                       const VkGraphicsPipelineCreateInfo *pCreateInfos,
                                                       const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                       const ErrorObject &error_obj) const;
    bool ValidateCreateComputePipelinesFlags(const VkPipelineCreateFlags2KHR flags, const Location &flags_loc) const;
    bool manual_PreCallValidateCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, u32 createInfoCount,
                                                      const VkComputePipelineCreateInfo *pCreateInfos,
                                                      const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                      const ErrorObject &error_obj) const;

    bool ValidateSamplerFilterMinMax(const VkSamplerCreateInfo &create_info, const Location &create_info_loc) const;
    bool ValidateSamplerCustomBoarderColor(const VkSamplerCreateInfo &create_info, const Location &create_info_loc) const;
    bool ValidateSamplerSubsampled(const VkSamplerCreateInfo &create_info, const Location &create_info_loc) const;
    bool ValidateSamplerImageProcessingQCOM(const VkSamplerCreateInfo &create_info, const Location &create_info_loc) const;
    bool manual_PreCallValidateCreateSampler(VkDevice device, const VkSamplerCreateInfo *pCreateInfo,
                                             const VkAllocationCallbacks *pAllocator, VkSampler *pSampler,
                                             const ErrorObject &error_obj) const;
    bool ValidateMutableDescriptorTypeCreateInfo(const VkDescriptorSetLayoutCreateInfo &create_info,
                                                 const VkMutableDescriptorTypeCreateInfoEXT &mutable_create_info,
                                                 const Location &loc) const;
    bool ValidateDescriptorSetLayoutCreateInfo(const VkDescriptorSetLayoutCreateInfo &create_info,
                                               const Location &create_info_loc) const;
    bool manual_PreCallValidateCreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
                                                         const VkAllocationCallbacks *pAllocator, VkDescriptorSetLayout *pSetLayout,
                                                         const ErrorObject &error_obj) const;
    bool manual_PreCallValidateGetDescriptorSetLayoutSupport(VkDevice device, const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
                                                             VkDescriptorSetLayoutSupport *pSupport,
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

    bool ValidateWriteDescriptorSet(const Location &loc, const u32 descriptorWriteCount,
                                    const VkWriteDescriptorSet *pDescriptorWrites) const;
    bool manual_PreCallValidateUpdateDescriptorSets(VkDevice device, u32 descriptorWriteCount,
                                                    const VkWriteDescriptorSet *pDescriptorWrites, u32 descriptorCopyCount,
                                                    const VkCopyDescriptorSet *pDescriptorCopies,
                                                    const ErrorObject &error_obj) const;

    bool manual_PreCallValidateFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, u32 descriptorSetCount,
                                                  const VkDescriptorSet *pDescriptorSets, const ErrorObject &error_obj) const;

    bool manual_PreCallValidateCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo *pCreateInfo,
                                                const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass,
                                                const ErrorObject &error_obj) const;

    bool manual_PreCallValidateCreateRenderPass2(VkDevice device, const VkRenderPassCreateInfo2 *pCreateInfo,
                                                 const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass,
                                                 const ErrorObject &error_obj) const;

    bool manual_PreCallValidateFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, u32 commandBufferCount,
                                                  const VkCommandBuffer *pCommandBuffers, const ErrorObject &error_obj) const;

    bool manual_PreCallValidateBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo *pBeginInfo,
                                                  const ErrorObject &error_obj) const;

    bool manual_PreCallValidateCmdSetViewport(VkCommandBuffer commandBuffer, u32 firstViewport, u32 viewportCount,
                                              const VkViewport *pViewports, const ErrorObject &error_obj) const;

    bool manual_PreCallValidateCmdSetScissor(VkCommandBuffer commandBuffer, u32 firstScissor, u32 scissorCount,
                                             const VkRect2D *pScissors, const ErrorObject &error_obj) const;
    bool manual_PreCallValidateCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth, const ErrorObject &error_obj) const;

    bool manual_PreCallValidateCmdClearAttachments(VkCommandBuffer commandBuffer, u32 attachmentCount,
                                                   const VkClearAttachment *pAttachments, u32 rectCount, const VkClearRect *pRects,
                                                   const ErrorObject &error_obj) const;

    bool manual_PreCallValidateGetPhysicalDeviceImageFormatProperties2(VkPhysicalDevice physicalDevice,
                                                                       const VkPhysicalDeviceImageFormatInfo2 *pImageFormatInfo,
                                                                       VkImageFormatProperties2 *pImageFormatProperties,
                                                                       const ErrorObject &error_obj) const;
    bool manual_PreCallValidateGetPhysicalDeviceImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format,
                                                                      VkImageType type, VkImageTiling tiling,
                                                                      VkImageUsageFlags usage, VkImageCreateFlags flags,
                                                                      VkImageFormatProperties *pImageFormatProperties,
                                                                      const ErrorObject &error_obj) const;

    bool manual_PreCallValidateCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, u32 regionCount,
                                             const VkBufferCopy *pRegions, const ErrorObject &error_obj) const;

    bool manual_PreCallValidateCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2 *pCopyBufferInfo,
                                              const ErrorObject &error_obj) const;

    bool manual_PreCallValidateCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                               VkDeviceSize dataSize, const void *pData, const ErrorObject &error_obj) const;

    bool manual_PreCallValidateCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                             VkDeviceSize size, u32 data, const ErrorObject &error_obj) const;

    bool manual_PreCallValidateCmdBindDescriptorBuffersEXT(VkCommandBuffer commandBuffer, u32 bufferCount,
                                                           const VkDescriptorBufferBindingInfoEXT *pBindingInfos,
                                                           const ErrorObject &error_obj) const;

    bool manual_PreCallValidateGetPhysicalDeviceExternalBufferProperties(
        VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalBufferInfo *pExternalBufferInfo,
        VkExternalBufferProperties *pExternalBufferProperties, const ErrorObject &error_obj) const;

    bool manual_PreCallValidateCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR *pCreateInfo,
                                                  const VkAllocationCallbacks *pAllocator, VkSwapchainKHR *pSwapchain,
                                                  const ErrorObject &error_obj) const;
    bool manual_PreCallValidateCreateSharedSwapchainsKHR(VkDevice device, u32 swapchainCount,
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

#ifdef VK_USE_PLATFORM_ANDROID_KHR
    bool manual_PreCallValidateCreateAndroidSurfaceKHR(VkInstance instance, const VkAndroidSurfaceCreateInfoKHR *pCreateInfo,
                                                       const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface,
                                                       const ErrorObject &error_obj) const;

#endif  // VK_USE_PLATFORM_ANDROID_KHR

    bool manual_PreCallValidateCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo *pCreateInfo,
                                                    const VkAllocationCallbacks *pAllocator, VkDescriptorPool *pDescriptorPool,
                                                    const ErrorObject &error_obj) const;
    bool manual_PreCallValidateCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                       VkPipelineLayout layout, u32 set, u32 descriptorWriteCount,
                                                       const VkWriteDescriptorSet *pDescriptorWrites,
                                                       const ErrorObject &error_obj) const;
    bool manual_PreCallValidateCmdPushDescriptorSet2KHR(VkCommandBuffer commandBuffer,
                                                        const VkPushDescriptorSetInfoKHR *pPushDescriptorSetInfo,
                                                        const ErrorObject &error_obj) const;
    bool manual_PreCallValidateCmdSetExclusiveScissorNV(VkCommandBuffer commandBuffer, u32 firstExclusiveScissor,
                                                        u32 exclusiveScissorCount, const VkRect2D *pExclusiveScissors,
                                                        const ErrorObject &error_obj) const;
    bool manual_PreCallValidateCmdSetViewportShadingRatePaletteNV(VkCommandBuffer commandBuffer, u32 firstViewport,
                                                                  u32 viewportCount,
                                                                  const VkShadingRatePaletteNV *pShadingRatePalettes,
                                                                  const ErrorObject &error_obj) const;

    bool manual_PreCallValidateCmdSetCoarseSampleOrderNV(VkCommandBuffer commandBuffer, VkCoarseSampleOrderTypeNV sampleOrderType,
                                                         u32 customSampleOrderCount,
                                                         const VkCoarseSampleOrderCustomNV *pCustomSampleOrders,
                                                         const ErrorObject &error_obj) const;

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
    bool manual_PreCallValidateDestroyAccelerationStructureKHR(VkDevice device, VkAccelerationStructureKHR accelerationStructure,
                                                               const VkAllocationCallbacks *pAllocator,
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
                                                                          u32 accelerationStructureCount,
                                                                          const VkAccelerationStructureNV *pAccelerationStructures,
                                                                          VkQueryType queryType, VkQueryPool queryPool,
                                                                          u32 firstQuery, const ErrorObject &error_obj) const;
    bool ValidateCreateRayTracingPipelinesFlagsNV(const VkPipelineCreateFlags2KHR flags, const Location &flags_loc) const;
    bool ValidateCreateRayTracingPipelinesFlagsKHR(const VkPipelineCreateFlags2KHR flags, const Location &flags_loc) const;
    bool manual_PreCallValidateCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, u32 createInfoCount,
                                                           const VkRayTracingPipelineCreateInfoNV *pCreateInfos,
                                                           const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                           const ErrorObject &error_obj) const;
    bool manual_PreCallValidateCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                            VkPipelineCache pipelineCache, u32 createInfoCount,
                                                            const VkRayTracingPipelineCreateInfoKHR *pCreateInfos,
                                                            const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                            const ErrorObject &error_obj) const;
    bool manual_PreCallValidateCmdSetViewportWScalingNV(VkCommandBuffer commandBuffer, u32 firstViewport, u32 viewportCount,
                                                        const VkViewportWScalingNV *pViewportWScalings,
                                                        const ErrorObject &error_obj) const;

    bool manual_PreCallValidateCreateShadersEXT(VkDevice device, u32 createInfoCount, const VkShaderCreateInfoEXT *pCreateInfos,
                                                const VkAllocationCallbacks *pAllocator, VkShaderEXT *pShaders,
                                                const ErrorObject &error_obj) const;
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

    bool manual_PreCallValidateCmdSetLineStippleKHR(VkCommandBuffer commandBuffer, u32 lineStippleFactor, u16 lineStipplePattern,
                                                    const ErrorObject &error_obj) const;

    bool ValidateCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType,
                                    const Location &loc) const;
    bool manual_PreCallValidateCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                  VkIndexType indexType, const ErrorObject &error_obj) const;
    bool manual_PreCallValidateCmdBindIndexBuffer2KHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                      VkDeviceSize size, VkIndexType indexType, const ErrorObject &error_obj) const;
    bool manual_PreCallValidateCmdBindVertexBuffers(VkCommandBuffer commandBuffer, u32 firstBinding, u32 bindingCount,
                                                    const VkBuffer *pBuffers, const VkDeviceSize *pOffsets,
                                                    const ErrorObject &error_obj) const;

    bool manual_PreCallValidateSetDebugUtilsObjectNameEXT(VkDevice device, const VkDebugUtilsObjectNameInfoEXT *pNameInfo,
                                                          const ErrorObject &error_obj) const;

    bool manual_PreCallValidateSetDebugUtilsObjectTagEXT(VkDevice device, const VkDebugUtilsObjectTagInfoEXT *pTagInfo,
                                                         const ErrorObject &error_obj) const;

    bool manual_PreCallValidateAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, u64 timeout, VkSemaphore semaphore,
                                                   VkFence fence, u32 *pImageIndex, const ErrorObject &error_obj) const;

    bool manual_PreCallValidateAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR *pAcquireInfo,
                                                    u32 *pImageIndex, const ErrorObject &error_obj) const;

    bool manual_PreCallValidateCmdBindTransformFeedbackBuffersEXT(VkCommandBuffer commandBuffer, u32 firstBinding, u32 bindingCount,
                                                                  const VkBuffer *pBuffers, const VkDeviceSize *pOffsets,
                                                                  const VkDeviceSize *pSizes, const ErrorObject &error_obj) const;

    bool manual_PreCallValidateCmdBeginTransformFeedbackEXT(VkCommandBuffer commandBuffer, u32 firstCounterBuffer,
                                                            u32 counterBufferCount, const VkBuffer *pCounterBuffers,
                                                            const VkDeviceSize *pCounterBufferOffsets,
                                                            const ErrorObject &error_obj) const;

    bool manual_PreCallValidateCmdEndTransformFeedbackEXT(VkCommandBuffer commandBuffer, u32 firstCounterBuffer,
                                                          u32 counterBufferCount, const VkBuffer *pCounterBuffers,
                                                          const VkDeviceSize *pCounterBufferOffsets,
                                                          const ErrorObject &error_obj) const;

    bool manual_PreCallValidateCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, u32 instanceCount, u32 firstInstance,
                                                           VkBuffer counterBuffer, VkDeviceSize counterBufferOffset,
                                                           u32 counterOffset, u32 vertexStride, const ErrorObject &error_obj) const;

    bool manual_PreCallValidateCreateSamplerYcbcrConversion(VkDevice device, const VkSamplerYcbcrConversionCreateInfo *pCreateInfo,
                                                            const VkAllocationCallbacks *pAllocator,
                                                            VkSamplerYcbcrConversion *pYcbcrConversion,
                                                            const ErrorObject &error_obj) const;

    bool manual_PreCallValidateGetDescriptorEXT(VkDevice device, const VkDescriptorGetInfoEXT *pDescriptorInfo, size_t dataSize,
                                                void *pDescriptor, const ErrorObject &error_obj) const;
    bool manual_PreCallValidateCmdSetDescriptorBufferOffsets2EXT(
        VkCommandBuffer commandBuffer, const VkSetDescriptorBufferOffsetsInfoEXT *pSetDescriptorBufferOffsetsInfo,
        const ErrorObject &error_obj) const;
    bool manual_PreCallValidateCmdBindDescriptorBufferEmbeddedSamplers2EXT(
        VkCommandBuffer commandBuffer,
        const VkBindDescriptorBufferEmbeddedSamplersInfoEXT *pBindDescriptorBufferEmbeddedSamplersInfo,
        const ErrorObject &error_obj) const;
    bool manual_PreCallValidateCmdPushDescriptorSetWithTemplate2KHR(
        VkCommandBuffer commandBuffer, const VkPushDescriptorSetWithTemplateInfoKHR *pPushDescriptorSetWithTemplateInfo,
        const ErrorObject &error_obj) const;
    bool manual_PreCallValidateCmdBindDescriptorSets2KHR(VkCommandBuffer commandBuffer,
                                                         const VkBindDescriptorSetsInfoKHR *pBindDescriptorSetsInfo,
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

    bool manual_PreCallValidateGetMemoryHostPointerPropertiesEXT(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType,
                                                                 const void *pHostPointer,
                                                                 VkMemoryHostPointerPropertiesEXT *pMemoryHostPointerProperties,
                                                                 const ErrorObject &error_obj) const;

#ifdef VK_USE_PLATFORM_WIN32_KHR
    bool manual_PreCallValidateGetMemoryWin32HandleKHR(VkDevice device, const VkMemoryGetWin32HandleInfoKHR *pGetWin32HandleInfo,
                                                       HANDLE *pHandle, const ErrorObject &error_obj) const;
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
    bool ValidateCopyAccelerationStructureInfoKHR(const VkCopyAccelerationStructureInfoKHR &as_info,
                                                  const VulkanTypedHandle &handle, const Location &info_loc) const;
    bool ValidateCopyMemoryToAccelerationStructureInfoKHR(const VkCopyMemoryToAccelerationStructureInfoKHR &as_info,
                                                          const VulkanTypedHandle &handle, const Location &loc) const;

    bool manual_PreCallValidateCopyMemoryToAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                                    const VkCopyMemoryToAccelerationStructureInfoKHR *pInfo,
                                                                    const ErrorObject &error_obj) const;
    bool manual_PreCallValidateCmdCopyMemoryToAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                                                       const VkCopyMemoryToAccelerationStructureInfoKHR *pInfo,
                                                                       const ErrorObject &error_obj) const;

    bool manual_PreCallValidateCmdWriteAccelerationStructuresPropertiesKHR(
        VkCommandBuffer commandBuffer, u32 accelerationStructureCount, const VkAccelerationStructureKHR *pAccelerationStructures,
        VkQueryType queryType, VkQueryPool queryPool, u32 firstQuery, const ErrorObject &error_obj) const;
    bool manual_PreCallValidateWriteAccelerationStructuresPropertiesKHR(VkDevice device, u32 accelerationStructureCount,
                                                                        const VkAccelerationStructureKHR *pAccelerationStructures,
                                                                        VkQueryType queryType, size_t dataSize, void *pData,
                                                                        size_t stride, const ErrorObject &error_obj) const;
    bool manual_PreCallValidateGetRayTracingCaptureReplayShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline, u32 firstGroup,
                                                                               u32 groupCount, size_t dataSize, void *pData,
                                                                               const ErrorObject &error_obj) const;

    bool manual_PreCallValidateCmdBuildAccelerationStructuresIndirectKHR(VkCommandBuffer commandBuffer, u32 infoCount,
                                                                         const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
                                                                         const VkDeviceAddress *pIndirectDeviceAddresses,
                                                                         const u32 *pIndirectStrides,
                                                                         const u32 *const *ppMaxPrimitiveCounts,
                                                                         const ErrorObject &error_obj) const;

    bool manual_PreCallValidateGetDeviceAccelerationStructureCompatibilityKHR(
        VkDevice device, const VkAccelerationStructureVersionInfoKHR *pVersionInfo,
        VkAccelerationStructureCompatibilityKHR *pCompatibility, const ErrorObject &error_obj) const;
    bool manual_PreCallValidateCmdSetViewportWithCount(VkCommandBuffer commandBuffer, u32 viewportCount,
                                                       const VkViewport *pViewports, const ErrorObject &error_obj) const;

    bool manual_PreCallValidateCmdSetScissorWithCount(VkCommandBuffer commandBuffer, u32 scissorCount, const VkRect2D *pScissors,
                                                      const ErrorObject &error_obj) const;
    bool manual_PreCallValidateCmdBindVertexBuffers2(VkCommandBuffer commandBuffer, u32 firstBinding, u32 bindingCount,
                                                     const VkBuffer *pBuffers, const VkDeviceSize *pOffsets,
                                                     const VkDeviceSize *pSizes, const VkDeviceSize *pStrides,
                                                     const ErrorObject &error_obj) const;

    [[nodiscard]] bool ValidateTotalPrimitivesCount(u64 total_triangles_count, u64 total_aabbs_count,
                                                    const VulkanTypedHandle &handle, const Location &loc) const;
    [[nodiscard]] bool ValidateAccelerationStructureBuildGeometryInfoKHR(const VkAccelerationStructureBuildGeometryInfoKHR &info,
                                                                         const VulkanTypedHandle &handle,
                                                                         const Location &info_loc) const;
    bool manual_PreCallValidateCmdBuildAccelerationStructuresKHR(
        VkCommandBuffer commandBuffer, u32 infoCount, const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
        const VkAccelerationStructureBuildRangeInfoKHR *const *ppBuildRangeInfos, const ErrorObject &error_obj) const;

    bool manual_PreCallValidateBuildAccelerationStructuresKHR(
        VkDevice device, VkDeferredOperationKHR deferredOperation, u32 infoCount,
        const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
        const VkAccelerationStructureBuildRangeInfoKHR *const *ppBuildRangeInfos, const ErrorObject &error_obj) const;

    bool manual_PreCallValidateGetAccelerationStructureBuildSizesKHR(VkDevice device, VkAccelerationStructureBuildTypeKHR buildType,
                                                                     const VkAccelerationStructureBuildGeometryInfoKHR *pBuildInfo,
                                                                     const u32 *pMaxPrimitiveCounts,
                                                                     VkAccelerationStructureBuildSizesInfoKHR *pSizeInfo,
                                                                     const ErrorObject &error_obj) const;

    bool ValidateTraceRaysRaygenShaderBindingTable(VkCommandBuffer commandBuffer,
                                                   const VkStridedDeviceAddressRegionKHR &raygen_shader_binding_table,
                                                   const Location &table_loc) const;
    bool ValidateTraceRaysMissShaderBindingTable(VkCommandBuffer commandBuffer,
                                                 const VkStridedDeviceAddressRegionKHR &miss_shader_binding_table,
                                                 const Location &table_loc) const;
    bool ValidateTraceRaysHitShaderBindingTable(VkCommandBuffer commandBuffer,
                                                const VkStridedDeviceAddressRegionKHR &hit_shader_binding_table,
                                                const Location &table_loc) const;
    bool ValidateTraceRaysCallableShaderBindingTable(VkCommandBuffer commandBuffer,
                                                     const VkStridedDeviceAddressRegionKHR &callable_shader_binding_table,
                                                     const Location &table_loc) const;

    bool manual_PreCallValidateCmdTraceRaysKHR(VkCommandBuffer commandBuffer,
                                               const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable,
                                               const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable,
                                               const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable,
                                               const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable, u32 width,
                                               u32 height, u32 depth, const ErrorObject &error_obj) const;

    bool manual_PreCallValidateCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer,
                                                       const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable,
                                                       const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable,
                                                       const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable,
                                                       const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable,
                                                       VkDeviceAddress indirectDeviceAddress, const ErrorObject &error_obj) const;
    bool manual_PreCallValidateCmdTraceRaysIndirect2KHR(VkCommandBuffer commandBuffer, VkDeviceAddress indirectDeviceAddress,
                                                        const ErrorObject &error_obj) const;

    bool manual_PreCallValidateCmdSetVertexInputEXT(VkCommandBuffer commandBuffer, u32 vertexBindingDescriptionCount,
                                                    const VkVertexInputBindingDescription2EXT *pVertexBindingDescriptions,
                                                    u32 vertexAttributeDescriptionCount,
                                                    const VkVertexInputAttributeDescription2EXT *pVertexAttributeDescriptions,
                                                    const ErrorObject &error_obj) const;

    bool ValidateCmdPushConstants(VkCommandBuffer commandBuffer, u32 offset, u32 size, const Location &loc) const;
    bool manual_PreCallValidateCmdPushConstants2KHR(VkCommandBuffer commandBuffer, const VkPushConstantsInfoKHR *pPushConstantsInfo,
                                                    const ErrorObject &error_obj) const;
    bool manual_PreCallValidateCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout,
                                                VkShaderStageFlags stageFlags, u32 offset, u32 size, const void *pValues,
                                                const ErrorObject &error_obj) const;

    bool manual_PreCallValidateMergePipelineCaches(VkDevice device, VkPipelineCache dstCache, u32 srcCacheCount,
                                                   const VkPipelineCache *pSrcCaches, const ErrorObject &error_obj) const;
    bool manual_PreCallValidateGetPipelinePropertiesEXT(VkDevice device, const VkPipelineInfoEXT *pPipelineInfo,
                                                        VkBaseOutStructure *pPipelineProperties,
                                                        const ErrorObject &error_obj) const;

    bool manual_PreCallValidateCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                  const VkClearColorValue *pColor, u32 rangeCount,
                                                  const VkImageSubresourceRange *pRanges, const ErrorObject &error_obj) const;

    bool ValidateRenderPassStripeBeginInfo(VkCommandBuffer commandBuffer, const void *pNext, const VkRect2D render_area,
                                           const Location &loc) const;
    bool ValidateCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *const rp_begin,
                                    const ErrorObject &error_obj) const;
    bool manual_PreCallValidateCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                  VkSubpassContents, const ErrorObject &error_obj) const;
    bool manual_PreCallValidateCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                   const VkSubpassBeginInfo *, const ErrorObject &error_obj) const;
    bool manual_PreCallValidateCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo *pRenderingInfo,
                                                 const ErrorObject &error_obj) const;
    bool ValidateBeginRenderingFragmentShadingRateAttachment(
        VkCommandBuffer commandBuffer, const VkRenderingInfo &rendering_info,
        const VkRenderingFragmentShadingRateAttachmentInfoKHR &rendering_fsr_attachment_info,
        const Location &rendering_info_loc) const;

    bool manual_PreCallValidateCmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer, u32 firstDiscardRectangle,
                                                         u32 discardRectangleCount, const VkRect2D *pDiscardRectangles,
                                                         const ErrorObject &error_obj) const;
    bool manual_PreCallValidateGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, u32 firstQuery, u32 queryCount,
                                                   size_t dataSize, void *pData, VkDeviceSize stride, VkQueryResultFlags flags,
                                                   const ErrorObject &error_obj) const;
    bool manual_PreCallValidateCmdBeginConditionalRenderingEXT(VkCommandBuffer commandBuffer,
                                                               const VkConditionalRenderingBeginInfoEXT *pConditionalRenderingBegin,
                                                               const ErrorObject &error_obj) const;

    bool ValidateDeviceImageMemoryRequirements(VkDevice device, const VkDeviceImageMemoryRequirements &memory_requirements,
                                               const Location &loc) const;

    bool manual_PreCallValidateGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                  u32 *pSurfaceFormatCount, VkSurfaceFormatKHR *pSurfaceFormats,
                                                                  const ErrorObject &error_obj) const;

    bool manual_PreCallValidateGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                       u32 *pPresentModeCount, VkPresentModeKHR *pPresentModes,
                                                                       const ErrorObject &error_obj) const;

    bool manual_PreCallValidateGetPhysicalDeviceSurfaceCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                                        const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo,
                                                                        VkSurfaceCapabilities2KHR *pSurfaceCapabilities,
                                                                        const ErrorObject &error_obj) const;

    bool manual_PreCallValidateGetPhysicalDeviceSurfaceFormats2KHR(VkPhysicalDevice physicalDevice,
                                                                   const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo,
                                                                   u32 *pSurfaceFormatCount, VkSurfaceFormat2KHR *pSurfaceFormats,
                                                                   const ErrorObject &error_obj) const;

    bool manual_PreCallValidateCmdSetDiscardRectangleEnableEXT(VkCommandBuffer commandBuffer, VkBool32 discardRectangleEnable,
                                                               const ErrorObject &error_obj) const;
    bool manual_PreCallValidateCmdSetDiscardRectangleModeEXT(VkCommandBuffer commandBuffer,
                                                             VkDiscardRectangleModeEXT discardRectangleMode,
                                                             const ErrorObject &error_obj) const;
    bool manual_PreCallValidateCmdSetExclusiveScissorEnableNV(VkCommandBuffer commandBuffer, u32 firstExclusiveScissor,
                                                              u32 exclusiveScissorCount, const VkBool32 *pExclusiveScissorEnables,
                                                              const ErrorObject &error_obj) const;

    bool manual_PreCallValidateSetDeviceMemoryPriorityEXT(VkDevice device, VkDeviceMemory memory, float priority,
                                                          const ErrorObject &error_obj) const;

#ifdef VK_USE_PLATFORM_WIN32_KHR
    bool manual_PreCallValidateGetPhysicalDeviceSurfacePresentModes2EXT(VkPhysicalDevice physicalDevice,
                                                                        const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo,
                                                                        u32 *pPresentModeCount, VkPresentModeKHR *pPresentModes,
                                                                        const ErrorObject &error_obj) const;
#endif  // VK_USE_PLATFORM_WIN32_KHR

    bool manual_PreCallValidateGetDeviceImageMemoryRequirements(VkDevice device, const VkDeviceImageMemoryRequirements *pInfo,
                                                                VkMemoryRequirements2 *pMemoryRequirements,
                                                                const ErrorObject &error_obj) const;
    bool manual_PreCallValidateGetDeviceImageSparseMemoryRequirements(VkDevice device, const VkDeviceImageMemoryRequirements *pInfo,
                                                                      u32 *pSparseMemoryRequirementCount,
                                                                      VkSparseImageMemoryRequirements2 *pSparseMemoryRequirements,
                                                                      const ErrorObject &error_obj) const;
    bool manual_PreCallValidateQueueBindSparse(VkQueue queue, u32 bindInfoCount, const VkBindSparseInfo *pBindInfo, VkFence fence,
                                               const ErrorObject &error_obj) const;

#ifdef VK_USE_PLATFORM_METAL_EXT
    bool manual_PreCallValidateExportMetalObjectsEXT(VkDevice device, VkExportMetalObjectsInfoEXT *pMetalObjectsInfo,
                                                     const ErrorObject &error_obj) const;
#endif  // VK_USE_PLATFORM_METAL_EXT

    bool ValidateAllocateMemoryExternal(VkDevice device, const VkMemoryAllocateInfo &allocate_info, VkMemoryAllocateFlags flags,
                                        const Location &allocate_info_loc) const;
#include "generated/stateless_validation_helper.h"
};  // Class StatelessValidation

// This is put outside the class because we were getting errors for:
//   explicit specialization in non-namespace scope ‘class StatelessValidation’
#include "generated/valid_enum_values.h"
