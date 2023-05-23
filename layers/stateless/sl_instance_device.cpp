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

static const int kMaxParamCheckerStringLength = 256;

bool StatelessValidation::ValidateString(const char *apiName, const ParameterName &stringName, const std::string &vuid,
                                         const char *validateString) const {
    bool skip = false;

    VkStringErrorFlags result = vk_string_validate(kMaxParamCheckerStringLength, validateString);

    if (result == VK_STRING_ERROR_NONE) {
        return skip;
    } else if (result & VK_STRING_ERROR_LENGTH) {
        skip = LogError(device, vuid, "%s: string %s exceeds max length %d", apiName, stringName.get_name().c_str(),
                        kMaxParamCheckerStringLength);
    } else if (result & VK_STRING_ERROR_BAD_DATA) {
        skip = LogError(device, vuid, "%s: string %s contains invalid characters or is badly formed", apiName,
                        stringName.get_name().c_str());
    }
    return skip;
}

bool StatelessValidation::ValidateApiVersion(uint32_t api_version, APIVersion effective_api_version) const {
    bool skip = false;
    uint32_t api_version_nopatch = VK_MAKE_VERSION(VK_VERSION_MAJOR(api_version), VK_VERSION_MINOR(api_version), 0);
    if (effective_api_version != api_version_nopatch) {
        if ((api_version_nopatch < VK_API_VERSION_1_0) && (api_version != 0)) {
            skip |= LogError(instance, "VUID-VkApplicationInfo-apiVersion-04010",
                             "Invalid CreateInstance->pCreateInfo->pApplicationInfo.apiVersion number (0x%08x). "
                             "Using VK_API_VERSION_%" PRIu32 "_%" PRIu32 ".",
                             api_version, effective_api_version.major(), effective_api_version.minor());
        } else {
            skip |= LogWarning(instance, kVUIDUndefined,
                               "Unrecognized CreateInstance->pCreateInfo->pApplicationInfo.apiVersion number (0x%08x). "
                               "Assuming VK_API_VERSION_%" PRIu32 "_%" PRIu32 ".",
                               api_version, effective_api_version.major(), effective_api_version.minor());
        }
    }
    return skip;
}

bool StatelessValidation::ValidateInstanceExtensions(const VkInstanceCreateInfo *pCreateInfo) const {
    bool skip = false;
    // Create and use a local instance extension object, as an actual instance has not been created yet
    uint32_t specified_version = (pCreateInfo->pApplicationInfo ? pCreateInfo->pApplicationInfo->apiVersion : VK_API_VERSION_1_0);
    InstanceExtensions local_instance_extensions;
    local_instance_extensions.InitFromInstanceCreateInfo(specified_version, pCreateInfo);

    for (uint32_t i = 0; i < pCreateInfo->enabledExtensionCount; i++) {
        skip |= ValidateExtensionReqs(local_instance_extensions, "VUID-vkCreateInstance-ppEnabledExtensionNames-01388", "instance",
                                      pCreateInfo->ppEnabledExtensionNames[i]);
    }
    if (pCreateInfo->flags & VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR &&
        !local_instance_extensions.vk_khr_portability_enumeration) {
        skip |= LogError(instance, "VUID-VkInstanceCreateInfo-flags-06559",
                         "vkCreateInstance(): pCreateInfo->flags has VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR set, but "
                         "pCreateInfo->ppEnabledExtensionNames does not include VK_KHR_portability_enumeration");
    }

    return skip;
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

bool StatelessValidation::ValidateValidationFeatures(const VkInstanceCreateInfo *pCreateInfo,
                                                     const VkValidationFeaturesEXT *validation_features) const {
    bool skip = false;
    bool debug_printf = false;
    bool gpu_assisted = false;
    bool reserve_slot = false;
    for (uint32_t i = 0; i < validation_features->enabledValidationFeatureCount; i++) {
        switch (validation_features->pEnabledValidationFeatures[i]) {
            case VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT:
                gpu_assisted = true;
                break;

            case VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT:
                debug_printf = true;
                break;

            case VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT:
                reserve_slot = true;
                break;

            default:
                break;
        }
    }
    if (reserve_slot && !gpu_assisted) {
        skip |= LogError(instance, "VUID-VkValidationFeaturesEXT-pEnabledValidationFeatures-02967",
                         "If VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT is in pEnabledValidationFeatures, "
                         "VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT must also be in pEnabledValidationFeatures.");
    }
    if (gpu_assisted && debug_printf) {
        skip |= LogError(instance, "VUID-VkValidationFeaturesEXT-pEnabledValidationFeatures-02968",
                         "If VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT is in pEnabledValidationFeatures, "
                         "VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT must not also be in pEnabledValidationFeatures.");
    }

    return skip;
}

template <typename ExtensionState>
ExtEnabled extension_state_by_name(const ExtensionState &extensions, const char *extension_name) {
    if (!extension_name) return kNotEnabled;  // null strings specify nothing
    auto info = ExtensionState::get_info(extension_name);
    ExtEnabled state =
        info.state ? extensions.*(info.state) : kNotEnabled;  // unknown extensions can't be enabled in extension struct
    return state;
}

bool StatelessValidation::manual_PreCallValidateCreateInstance(const VkInstanceCreateInfo *pCreateInfo,
                                                               const VkAllocationCallbacks *pAllocator,
                                                               VkInstance *pInstance) const {
    bool skip = false;
    // Note: From the spec--
    //  Providing a NULL VkInstanceCreateInfo::pApplicationInfo or providing an apiVersion of 0 is equivalent to providing
    //  an apiVersion of VK_MAKE_VERSION(1, 0, 0).  (a.k.a. VK_API_VERSION_1_0)
    uint32_t local_api_version = (pCreateInfo->pApplicationInfo && pCreateInfo->pApplicationInfo->apiVersion)
                                     ? pCreateInfo->pApplicationInfo->apiVersion
                                     : VK_API_VERSION_1_0;
    skip |= ValidateApiVersion(local_api_version, api_version);
    skip |= ValidateInstanceExtensions(pCreateInfo);
    const auto *validation_features = LvlFindInChain<VkValidationFeaturesEXT>(pCreateInfo->pNext);
    if (validation_features) skip |= ValidateValidationFeatures(pCreateInfo, validation_features);

#ifdef VK_USE_PLATFORM_METAL_EXT
    auto export_metal_object_info = LvlFindInChain<VkExportMetalObjectCreateInfoEXT>(pCreateInfo->pNext);
    while (export_metal_object_info) {
        if ((export_metal_object_info->exportObjectType != VK_EXPORT_METAL_OBJECT_TYPE_METAL_DEVICE_BIT_EXT) &&
            (export_metal_object_info->exportObjectType != VK_EXPORT_METAL_OBJECT_TYPE_METAL_COMMAND_QUEUE_BIT_EXT)) {
            skip |= LogError(instance, "VUID-VkInstanceCreateInfo-pNext-06779",
                             "vkCreateInstance(): The pNext chain contains a VkExportMetalObjectCreateInfoEXT whose "
                             "exportObjectType = %s, but only VkExportMetalObjectCreateInfoEXT structs with exportObjectType of "
                             "VK_EXPORT_METAL_OBJECT_TYPE_METAL_DEVICE_BIT_EXT or "
                             "VK_EXPORT_METAL_OBJECT_TYPE_METAL_COMMAND_QUEUE_BIT_EXT are allowed",
                             string_VkExportMetalObjectTypeFlagBitsEXT(export_metal_object_info->exportObjectType));
        }
        export_metal_object_info = LvlFindInChain<VkExportMetalObjectCreateInfoEXT>(export_metal_object_info->pNext);
    }
#endif  // VK_USE_PLATFORM_METAL_EXT

    return skip;
}

void StatelessValidation::PostCallRecordCreateInstance(const VkInstanceCreateInfo *pCreateInfo,
                                                       const VkAllocationCallbacks *pAllocator, VkInstance *pInstance,
                                                       VkResult result) {
    auto instance_data = GetLayerDataPtr(get_dispatch_key(*pInstance), layer_data_map);
    // Copy extension data into local object
    if (result != VK_SUCCESS) return;
    this->instance_extensions = instance_data->instance_extensions;
    this->device_extensions = instance_data->device_extensions;
}

void StatelessValidation::CommonPostCallRecordEnumeratePhysicalDevice(const VkPhysicalDevice *phys_devices, const int count) {
    // Assume phys_devices is valid
    assert(phys_devices);
    for (int i = 0; i < count; ++i) {
        const auto &phys_device = phys_devices[i];
        if (0 == physical_device_properties_map.count(phys_device)) {
            auto phys_dev_props = new VkPhysicalDeviceProperties;
            DispatchGetPhysicalDeviceProperties(phys_device, phys_dev_props);
            physical_device_properties_map[phys_device] = phys_dev_props;

            // Enumerate the Device Ext Properties to save the PhysicalDevice supported extension state
            uint32_t ext_count = 0;
            vvl::unordered_set<std::string> dev_exts_enumerated{};
            std::vector<VkExtensionProperties> ext_props{};
            instance_dispatch_table.EnumerateDeviceExtensionProperties(phys_device, nullptr, &ext_count, nullptr);
            ext_props.resize(ext_count);
            instance_dispatch_table.EnumerateDeviceExtensionProperties(phys_device, nullptr, &ext_count, ext_props.data());
            for (uint32_t j = 0; j < ext_count; j++) {
                dev_exts_enumerated.insert(ext_props[j].extensionName);

                std::string_view extension_name = ext_props[j].extensionName;
                if (extension_name == "VK_EXT_discard_rectangles") {
                    discard_rectangles_extension_version = ext_props[j].specVersion;
                } else if (extension_name == "VK_NV_scissor_exclusive") {
                    scissor_exclusive_extension_version = ext_props[j].specVersion;
                }
            }
            device_extensions_enumerated[phys_device] = std::move(dev_exts_enumerated);
        }
    }
}

void StatelessValidation::PostCallRecordEnumeratePhysicalDevices(VkInstance instance, uint32_t *pPhysicalDeviceCount,
                                                                 VkPhysicalDevice *pPhysicalDevices, VkResult result) {
    if ((VK_SUCCESS != result) && (VK_INCOMPLETE != result)) {
        return;
    }

    if (pPhysicalDeviceCount && pPhysicalDevices) {
        CommonPostCallRecordEnumeratePhysicalDevice(pPhysicalDevices, *pPhysicalDeviceCount);
    }
}

void StatelessValidation::PostCallRecordEnumeratePhysicalDeviceGroups(
    VkInstance instance, uint32_t *pPhysicalDeviceGroupCount, VkPhysicalDeviceGroupProperties *pPhysicalDeviceGroupProperties,
    VkResult result) {
    if ((VK_SUCCESS != result) && (VK_INCOMPLETE != result)) {
        return;
    }

    if (pPhysicalDeviceGroupCount && pPhysicalDeviceGroupProperties) {
        for (uint32_t i = 0; i < *pPhysicalDeviceGroupCount; i++) {
            const auto &group = pPhysicalDeviceGroupProperties[i];
            CommonPostCallRecordEnumeratePhysicalDevice(group.physicalDevices, group.physicalDeviceCount);
        }
    }
}

void StatelessValidation::PreCallRecordDestroyInstance(VkInstance instance, const VkAllocationCallbacks *pAllocator) {
    for (auto it = physical_device_properties_map.begin(); it != physical_device_properties_map.end();) {
        delete (it->second);
        it = physical_device_properties_map.erase(it);
    }
}

void StatelessValidation::GetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice,
                                                       VkPhysicalDeviceProperties2 &pProperties) const {
    if (api_version >= VK_API_VERSION_1_1) {
        DispatchGetPhysicalDeviceProperties2(physicalDevice, &pProperties);
    } else if (IsExtEnabled(device_extensions.vk_khr_get_physical_device_properties2)) {
        DispatchGetPhysicalDeviceProperties2KHR(physicalDevice, &pProperties);
    }
}

void StatelessValidation::PostCallRecordCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo *pCreateInfo,
                                                     const VkAllocationCallbacks *pAllocator, VkDevice *pDevice, VkResult result) {
    auto device_data = GetLayerDataPtr(get_dispatch_key(*pDevice), layer_data_map);
    if (result != VK_SUCCESS) return;
    ValidationObject *validation_data = GetValidationObject(device_data->object_dispatch, LayerObjectTypeParameterValidation);
    StatelessValidation *stateless_validation = static_cast<StatelessValidation *>(validation_data);

    // Parmeter validation also uses extension data
    stateless_validation->device_extensions = this->device_extensions;

    VkPhysicalDeviceProperties device_properties = {};
    // Need to get instance and do a getlayerdata call...
    DispatchGetPhysicalDeviceProperties(physicalDevice, &device_properties);
    memcpy(&stateless_validation->device_limits, &device_properties.limits, sizeof(VkPhysicalDeviceLimits));

    if (IsExtEnabled(device_extensions.vk_nv_shading_rate_image)) {
        // Get the needed shading rate image limits
        auto shading_rate_image_props = LvlInitStruct<VkPhysicalDeviceShadingRateImagePropertiesNV>();
        auto prop2 = LvlInitStruct<VkPhysicalDeviceProperties2>(&shading_rate_image_props);
        GetPhysicalDeviceProperties2(physicalDevice, prop2);
        phys_dev_ext_props.shading_rate_image_props = shading_rate_image_props;
    }

    if (IsExtEnabled(device_extensions.vk_nv_mesh_shader)) {
        // Get the needed mesh shader limits
        auto mesh_shader_props = LvlInitStruct<VkPhysicalDeviceMeshShaderPropertiesNV>();
        auto prop2 = LvlInitStruct<VkPhysicalDeviceProperties2>(&mesh_shader_props);
        GetPhysicalDeviceProperties2(physicalDevice, prop2);
        phys_dev_ext_props.mesh_shader_props_nv = mesh_shader_props;
    }

    if (IsExtEnabled(device_extensions.vk_ext_mesh_shader)) {
        // Get the needed mesh shader EXT limits
        auto mesh_shader_props_ext = LvlInitStruct<VkPhysicalDeviceMeshShaderPropertiesEXT>();
        auto prop2 = LvlInitStruct<VkPhysicalDeviceProperties2>(&mesh_shader_props_ext);
        GetPhysicalDeviceProperties2(physicalDevice, prop2);
        phys_dev_ext_props.mesh_shader_props_ext = mesh_shader_props_ext;
    }

    if (IsExtEnabled(device_extensions.vk_nv_ray_tracing)) {
        // Get the needed ray tracing limits
        auto ray_tracing_props = LvlInitStruct<VkPhysicalDeviceRayTracingPropertiesNV>();
        auto prop2 = LvlInitStruct<VkPhysicalDeviceProperties2>(&ray_tracing_props);
        GetPhysicalDeviceProperties2(physicalDevice, prop2);
        phys_dev_ext_props.ray_tracing_props_nv = ray_tracing_props;
    }

    if (IsExtEnabled(device_extensions.vk_khr_ray_tracing_pipeline)) {
        // Get the needed ray tracing limits
        auto ray_tracing_props = LvlInitStruct<VkPhysicalDeviceRayTracingPipelinePropertiesKHR>();
        auto prop2 = LvlInitStruct<VkPhysicalDeviceProperties2>(&ray_tracing_props);
        GetPhysicalDeviceProperties2(physicalDevice, prop2);
        phys_dev_ext_props.ray_tracing_props_khr = ray_tracing_props;
    }

    if (IsExtEnabled(device_extensions.vk_khr_acceleration_structure)) {
        // Get the needed ray tracing acc structure limits
        auto acc_structure_props = LvlInitStruct<VkPhysicalDeviceAccelerationStructurePropertiesKHR>();
        auto prop2 = LvlInitStruct<VkPhysicalDeviceProperties2>(&acc_structure_props);
        GetPhysicalDeviceProperties2(physicalDevice, prop2);
        phys_dev_ext_props.acc_structure_props = acc_structure_props;
    }

    if (IsExtEnabled(device_extensions.vk_ext_transform_feedback)) {
        // Get the needed transform feedback limits
        auto transform_feedback_props = LvlInitStruct<VkPhysicalDeviceTransformFeedbackPropertiesEXT>();
        auto prop2 = LvlInitStruct<VkPhysicalDeviceProperties2>(&transform_feedback_props);
        GetPhysicalDeviceProperties2(physicalDevice, prop2);
        phys_dev_ext_props.transform_feedback_props = transform_feedback_props;
    }

    if (IsExtEnabled(device_extensions.vk_ext_vertex_attribute_divisor)) {
        // Get the needed vertex attribute divisor limits
        auto vertex_attribute_divisor_props = LvlInitStruct<VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT>();
        auto prop2 = LvlInitStruct<VkPhysicalDeviceProperties2>(&vertex_attribute_divisor_props);
        GetPhysicalDeviceProperties2(physicalDevice, prop2);
        phys_dev_ext_props.vertex_attribute_divisor_props = vertex_attribute_divisor_props;
    }

    if (IsExtEnabled(device_extensions.vk_ext_blend_operation_advanced)) {
        // Get the needed blend operation advanced properties
        auto blend_operation_advanced_props = LvlInitStruct<VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT>();
        auto prop2 = LvlInitStruct<VkPhysicalDeviceProperties2>(&blend_operation_advanced_props);
        GetPhysicalDeviceProperties2(physicalDevice, prop2);
        phys_dev_ext_props.blend_operation_advanced_props = blend_operation_advanced_props;
    }

    if (IsExtEnabled(device_extensions.vk_khr_maintenance4)) {
        // Get the needed maintenance4 properties
        auto maintance4_props = LvlInitStruct<VkPhysicalDeviceMaintenance4PropertiesKHR>();
        auto prop2 = LvlInitStruct<VkPhysicalDeviceProperties2>(&maintance4_props);
        GetPhysicalDeviceProperties2(physicalDevice, prop2);
        phys_dev_ext_props.maintenance4_props = maintance4_props;
    }

    if (IsExtEnabled(device_extensions.vk_khr_fragment_shading_rate)) {
        auto fragment_shading_rate_props = LvlInitStruct<VkPhysicalDeviceFragmentShadingRatePropertiesKHR>();
        auto prop2 = LvlInitStruct<VkPhysicalDeviceProperties2>(&fragment_shading_rate_props);
        GetPhysicalDeviceProperties2(physicalDevice, prop2);
        phys_dev_ext_props.fragment_shading_rate_props = fragment_shading_rate_props;
    }

    if (IsExtEnabled(device_extensions.vk_khr_depth_stencil_resolve)) {
        auto depth_stencil_resolve_props = LvlInitStruct<VkPhysicalDeviceDepthStencilResolveProperties>();
        auto prop2 = LvlInitStruct<VkPhysicalDeviceProperties2>(&depth_stencil_resolve_props);
        GetPhysicalDeviceProperties2(physicalDevice, prop2);
        phys_dev_ext_props.depth_stencil_resolve_props = depth_stencil_resolve_props;
    }

    stateless_validation->phys_dev_ext_props = this->phys_dev_ext_props;

    // Save app-enabled features in this device's validation object
    // The enabled features can come from either pEnabledFeatures, or from the pNext chain
    const auto *features2 = LvlFindInChain<VkPhysicalDeviceFeatures2>(pCreateInfo->pNext);
    safe_VkPhysicalDeviceFeatures2 tmp_features2_state;
    tmp_features2_state.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    if (features2) {
        tmp_features2_state.features = features2->features;
    } else if (pCreateInfo->pEnabledFeatures) {
        tmp_features2_state.features = *pCreateInfo->pEnabledFeatures;
    } else {
        tmp_features2_state.features = {};
    }
    // Use pCreateInfo->pNext to get full chain
    stateless_validation->device_createinfo_pnext = SafePnextCopy(pCreateInfo->pNext);
    stateless_validation->physical_device_features2 = tmp_features2_state;
}

bool StatelessValidation::manual_PreCallValidateCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo *pCreateInfo,
                                                             const VkAllocationCallbacks *pAllocator, VkDevice *pDevice) const {
    bool skip = false;

    for (size_t i = 0; i < pCreateInfo->enabledLayerCount; i++) {
        skip |= ValidateString("vkCreateDevice", "pCreateInfo->ppEnabledLayerNames",
                               "VUID-VkDeviceCreateInfo-ppEnabledLayerNames-parameter", pCreateInfo->ppEnabledLayerNames[i]);
    }

    // If this device supports VK_KHR_portability_subset, it must be enabled
    const std::string portability_extension_name("VK_KHR_portability_subset");
    const std::string fragmentmask_extension_name("VK_AMD_shader_fragment_mask");
    const auto &dev_extensions = device_extensions_enumerated.at(physicalDevice);
    const bool portability_supported = dev_extensions.count(portability_extension_name) != 0;
    bool portability_requested = false;
    bool fragmentmask_requested = false;

    for (size_t i = 0; i < pCreateInfo->enabledExtensionCount; i++) {
        skip |=
            ValidateString("vkCreateDevice", "pCreateInfo->ppEnabledExtensionNames",
                           "VUID-VkDeviceCreateInfo-ppEnabledExtensionNames-parameter", pCreateInfo->ppEnabledExtensionNames[i]);
        skip |= ValidateExtensionReqs(device_extensions, "VUID-vkCreateDevice-ppEnabledExtensionNames-01387", "device",
                                      pCreateInfo->ppEnabledExtensionNames[i]);
        if (portability_extension_name == pCreateInfo->ppEnabledExtensionNames[i]) {
            portability_requested = true;
        }
        if (fragmentmask_extension_name == pCreateInfo->ppEnabledExtensionNames[i]) {
            fragmentmask_requested = true;
        }
    }

    if (portability_supported && !portability_requested) {
        skip |= LogError(physicalDevice, "VUID-VkDeviceCreateInfo-pProperties-04451",
                         "vkCreateDevice: VK_KHR_portability_subset must be enabled because physical device %s supports it",
                         report_data->FormatHandle(physicalDevice).c_str());
    }

    {
        const bool maint1 =
            IsExtEnabledByCreateinfo(extension_state_by_name(device_extensions, VK_KHR_MAINTENANCE_1_EXTENSION_NAME));
        bool negative_viewport =
            IsExtEnabledByCreateinfo(extension_state_by_name(device_extensions, VK_AMD_NEGATIVE_VIEWPORT_HEIGHT_EXTENSION_NAME));
        if (negative_viewport) {
            // Only need to check for VK_KHR_MAINTENANCE_1_EXTENSION_NAME if api version is 1.0, otherwise it's deprecated due to
            // integration into api version 1.1
            if (api_version >= VK_API_VERSION_1_1) {
                skip |= LogError(device, "VUID-VkDeviceCreateInfo-ppEnabledExtensionNames-01840",
                                 "vkCreateDevice(): ppEnabledExtensionNames must not include "
                                 "VK_AMD_negative_viewport_height if api version is greater than or equal to 1.1.");
            } else if (maint1) {
                skip |= LogError(device, "VUID-VkDeviceCreateInfo-ppEnabledExtensionNames-00374",
                                 "vkCreateDevice(): ppEnabledExtensionNames must not simultaneously include "
                                 "VK_KHR_maintenance1 and VK_AMD_negative_viewport_height.");
            }
        }
    }

    {
        const auto *descriptor_buffer_features = LvlFindInChain<VkPhysicalDeviceDescriptorBufferFeaturesEXT>(pCreateInfo->pNext);
        if (descriptor_buffer_features && descriptor_buffer_features->descriptorBuffer && fragmentmask_requested) {
            skip |= LogError(device, "VUID-VkDeviceCreateInfo-None-08095",
                             "vkCreateDevice(): If the descriptorBuffer feature is enabled, ppEnabledExtensionNames must not "
                             "contain VK_AMD_shader_fragment_mask.");
        }
    }

    {
        bool khr_bda =
            IsExtEnabledByCreateinfo(extension_state_by_name(device_extensions, VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME));
        bool ext_bda =
            IsExtEnabledByCreateinfo(extension_state_by_name(device_extensions, VK_EXT_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME));
        if (khr_bda && ext_bda) {
            skip |= LogError(device, "VUID-VkDeviceCreateInfo-ppEnabledExtensionNames-03328",
                             "vkCreateDevice(): ppEnabledExtensionNames must not contain both VK_KHR_buffer_device_address and "
                             "VK_EXT_buffer_device_address.");
        }
    }

    if (pCreateInfo->pNext != NULL && pCreateInfo->pEnabledFeatures) {
        // Check for get_physical_device_properties2 struct
        const auto *features2 = LvlFindInChain<VkPhysicalDeviceFeatures2>(pCreateInfo->pNext);
        if (features2) {
            // Cannot include VkPhysicalDeviceFeatures2 and have non-null pEnabledFeatures
            skip |= LogError(device, "VUID-VkDeviceCreateInfo-pNext-00373",
                             "vkCreateDevice(): pNext includes a VkPhysicalDeviceFeatures2 struct when "
                             "pCreateInfo->pEnabledFeatures is non-NULL.");
        }
    }

    auto features2 = LvlFindInChain<VkPhysicalDeviceFeatures2>(pCreateInfo->pNext);
    const VkPhysicalDeviceFeatures *features = features2 ? &features2->features : pCreateInfo->pEnabledFeatures;
    const auto *robustness2_features = LvlFindInChain<VkPhysicalDeviceRobustness2FeaturesEXT>(pCreateInfo->pNext);
    if (features && robustness2_features && robustness2_features->robustBufferAccess2 && !features->robustBufferAccess) {
        skip |= LogError(device, "VUID-VkPhysicalDeviceRobustness2FeaturesEXT-robustBufferAccess2-04000",
                         "vkCreateDevice(): If robustBufferAccess2 is enabled then robustBufferAccess must be enabled.");
    }
    const auto *raytracing_features = LvlFindInChain<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>(pCreateInfo->pNext);
    if (raytracing_features && raytracing_features->rayTracingPipelineShaderGroupHandleCaptureReplayMixed &&
        !raytracing_features->rayTracingPipelineShaderGroupHandleCaptureReplay) {
        skip |= LogError(
            device,
            "VUID-VkPhysicalDeviceRayTracingPipelineFeaturesKHR-rayTracingPipelineShaderGroupHandleCaptureReplayMixed-03575",
            "vkCreateDevice(): If rayTracingPipelineShaderGroupHandleCaptureReplayMixed is VK_TRUE, "
            "rayTracingPipelineShaderGroupHandleCaptureReplay "
            "must also be VK_TRUE.");
    }
    auto vertex_attribute_divisor_features = LvlFindInChain<VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT>(pCreateInfo->pNext);
    if (vertex_attribute_divisor_features && (!IsExtEnabled(device_extensions.vk_ext_vertex_attribute_divisor))) {
        skip |= LogError(device, kVUID_PVError_ExtensionNotEnabled,
                         "vkCreateDevice(): pNext includes a VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT "
                         "struct, VK_EXT_vertex_attribute_divisor must be enabled when it creates a device.");
    }

    const auto *vulkan_11_features = LvlFindInChain<VkPhysicalDeviceVulkan11Features>(pCreateInfo->pNext);
    if (vulkan_11_features) {
        const VkBaseOutStructure *current = reinterpret_cast<const VkBaseOutStructure *>(pCreateInfo->pNext);
        while (current) {
            if (current->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES ||
                current->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES ||
                current->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VARIABLE_POINTERS_FEATURES ||
                current->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROTECTED_MEMORY_FEATURES ||
                current->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES ||
                current->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES) {
                skip |= LogError(instance, "VUID-VkDeviceCreateInfo-pNext-02829",
                                 "vkCreateDevice(): If the pNext chain includes a VkPhysicalDeviceVulkan11Features structure, then "
                                 "it must not include a %s structure",
                                 string_VkStructureType(current->sType));
                break;
            }
            current = reinterpret_cast<const VkBaseOutStructure *>(current->pNext);
        }

        // Check features are enabled if matching extension is passed in as well
        for (uint32_t i = 0; i < pCreateInfo->enabledExtensionCount; i++) {
            const char *extension = pCreateInfo->ppEnabledExtensionNames[i];
            if ((0 == strncmp(extension, VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME, VK_MAX_EXTENSION_NAME_SIZE)) &&
                (vulkan_11_features->shaderDrawParameters == VK_FALSE)) {
                skip |= LogError(
                    instance, "VUID-VkDeviceCreateInfo-ppEnabledExtensionNames-04476",
                    "vkCreateDevice(): %s is enabled but VkPhysicalDeviceVulkan11Features::shaderDrawParameters is not VK_TRUE.",
                    VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME);
            }
        }
    }

    const auto *vulkan_12_features = LvlFindInChain<VkPhysicalDeviceVulkan12Features>(pCreateInfo->pNext);
    if (vulkan_12_features) {
        const VkBaseOutStructure *current = reinterpret_cast<const VkBaseOutStructure *>(pCreateInfo->pNext);
        while (current) {
            if (current->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_8BIT_STORAGE_FEATURES ||
                current->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_INT64_FEATURES ||
                current->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES ||
                current->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES ||
                current->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCALAR_BLOCK_LAYOUT_FEATURES ||
                current->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGELESS_FRAMEBUFFER_FEATURES ||
                current->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_UNIFORM_BUFFER_STANDARD_LAYOUT_FEATURES ||
                current->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_EXTENDED_TYPES_FEATURES ||
                current->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SEPARATE_DEPTH_STENCIL_LAYOUTS_FEATURES ||
                current->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES ||
                current->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES ||
                current->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES ||
                current->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_MEMORY_MODEL_FEATURES) {
                skip |= LogError(
                    instance, "VUID-VkDeviceCreateInfo-pNext-02830",
                    "vkCreateDevice(): If the pNext chain includes a VkPhysicalDeviceVulkan12Features structure, then it must not "
                    "include a %s structure",
                    string_VkStructureType(current->sType));
                break;
            }
            current = reinterpret_cast<const VkBaseOutStructure *>(current->pNext);
        }
        // Check features are enabled if matching extension is passed in as well
        for (uint32_t i = 0; i < pCreateInfo->enabledExtensionCount; i++) {
            const char *extension = pCreateInfo->ppEnabledExtensionNames[i];
            if ((0 == strncmp(extension, VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME, VK_MAX_EXTENSION_NAME_SIZE)) &&
                (vulkan_12_features->drawIndirectCount == VK_FALSE)) {
                skip |= LogError(
                    instance, "VUID-VkDeviceCreateInfo-ppEnabledExtensionNames-02831",
                    "vkCreateDevice(): %s is enabled but VkPhysicalDeviceVulkan12Features::drawIndirectCount is not VK_TRUE.",
                    VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME);
            }
            if ((0 == strncmp(extension, VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME, VK_MAX_EXTENSION_NAME_SIZE)) &&
                (vulkan_12_features->samplerMirrorClampToEdge == VK_FALSE)) {
                skip |= LogError(instance, "VUID-VkDeviceCreateInfo-ppEnabledExtensionNames-02832",
                                 "vkCreateDevice(): %s is enabled but VkPhysicalDeviceVulkan12Features::samplerMirrorClampToEdge "
                                 "is not VK_TRUE.",
                                 VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME);
            }
            if ((0 == strncmp(extension, VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME, VK_MAX_EXTENSION_NAME_SIZE)) &&
                (vulkan_12_features->descriptorIndexing == VK_FALSE)) {
                skip |= LogError(
                    instance, "VUID-VkDeviceCreateInfo-ppEnabledExtensionNames-02833",
                    "vkCreateDevice(): %s is enabled but VkPhysicalDeviceVulkan12Features::descriptorIndexing is not VK_TRUE.",
                    VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
            }
            if ((0 == strncmp(extension, VK_EXT_SAMPLER_FILTER_MINMAX_EXTENSION_NAME, VK_MAX_EXTENSION_NAME_SIZE)) &&
                (vulkan_12_features->samplerFilterMinmax == VK_FALSE)) {
                skip |= LogError(
                    instance, "VUID-VkDeviceCreateInfo-ppEnabledExtensionNames-02834",
                    "vkCreateDevice(): %s is enabled but VkPhysicalDeviceVulkan12Features::samplerFilterMinmax is not VK_TRUE.",
                    VK_EXT_SAMPLER_FILTER_MINMAX_EXTENSION_NAME);
            }
            if ((0 == strncmp(extension, VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME, VK_MAX_EXTENSION_NAME_SIZE)) &&
                ((vulkan_12_features->shaderOutputViewportIndex == VK_FALSE) ||
                 (vulkan_12_features->shaderOutputLayer == VK_FALSE))) {
                skip |=
                    LogError(instance, "VUID-VkDeviceCreateInfo-ppEnabledExtensionNames-02835",
                             "vkCreateDevice(): %s is enabled but both VkPhysicalDeviceVulkan12Features::shaderOutputViewportIndex "
                             "and VkPhysicalDeviceVulkan12Features::shaderOutputLayer are not VK_TRUE.",
                             VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME);
            }
        }
        if (vulkan_12_features->bufferDeviceAddress == VK_TRUE) {
            if (IsExtEnabledByCreateinfo(extension_state_by_name(device_extensions, VK_EXT_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME))) {
                skip |= LogError(instance, "VUID-VkDeviceCreateInfo-pNext-04748",
                                 "vkCreateDevice(): pNext chain includes VkPhysicalDeviceVulkan12Features with bufferDeviceAddress "
                                 "set to VK_TRUE and ppEnabledExtensionNames contains VK_EXT_buffer_device_address");
            }
        }
    }

    const auto *vulkan_13_features = LvlFindInChain<VkPhysicalDeviceVulkan13Features>(pCreateInfo->pNext);
    if (vulkan_13_features) {
        const VkBaseOutStructure *current = reinterpret_cast<const VkBaseOutStructure *>(pCreateInfo->pNext);
        while (current) {
            if (current->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES ||
                current->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_ROBUSTNESS_FEATURES ||
                current->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INLINE_UNIFORM_BLOCK_FEATURES ||
                current->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_FEATURES ||
                current->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_CREATION_CACHE_CONTROL_FEATURES ||
                current->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIVATE_DATA_FEATURES ||
                current->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DEMOTE_TO_HELPER_INVOCATION_FEATURES ||
                current->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_INTEGER_DOT_PRODUCT_FEATURES ||
                current->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_TERMINATE_INVOCATION_FEATURES ||
                current->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_FEATURES ||
                current->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES ||
                current->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXTURE_COMPRESSION_ASTC_HDR_FEATURES ||
                current->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ZERO_INITIALIZE_WORKGROUP_MEMORY_FEATURES) {
                skip |= LogError(
                    instance, "VUID-VkDeviceCreateInfo-pNext-06532",
                    "vkCreateDevice(): If the pNext chain includes a VkPhysicalDeviceVulkan13Features structure, then it must not "
                    "include a %s structure",
                    string_VkStructureType(current->sType));
                break;
            }
            current = reinterpret_cast<const VkBaseOutStructure *>(current->pNext);
        }
    }

    // Validate pCreateInfo->pQueueCreateInfos
    if (pCreateInfo->pQueueCreateInfos) {
        for (uint32_t i = 0; i < pCreateInfo->queueCreateInfoCount; ++i) {
            const VkDeviceQueueCreateInfo &queue_create_info = pCreateInfo->pQueueCreateInfos[i];
            const uint32_t requested_queue_family = queue_create_info.queueFamilyIndex;
            if (requested_queue_family == VK_QUEUE_FAMILY_IGNORED) {
                skip |=
                    LogError(physicalDevice, "VUID-VkDeviceQueueCreateInfo-queueFamilyIndex-00381",
                             "vkCreateDevice: pCreateInfo->pQueueCreateInfos[%" PRIu32
                             "].queueFamilyIndex is VK_QUEUE_FAMILY_IGNORED, but it is required to provide a valid queue family "
                             "index value.",
                             i);
            }

            if (queue_create_info.pQueuePriorities != nullptr) {
                for (uint32_t j = 0; j < queue_create_info.queueCount; ++j) {
                    const float queue_priority = queue_create_info.pQueuePriorities[j];
                    if (!(queue_priority >= 0.f) || !(queue_priority <= 1.f)) {
                        skip |= LogError(physicalDevice, "VUID-VkDeviceQueueCreateInfo-pQueuePriorities-00383",
                                         "vkCreateDevice: pCreateInfo->pQueueCreateInfos[%" PRIu32 "].pQueuePriorities[%" PRIu32
                                         "] (=%f) is not between 0 and 1 (inclusive).",
                                         i, j, queue_priority);
                    }
                }
            }

            // Need to know if protectedMemory feature is passed in preCall to creating the device
            VkBool32 protected_memory = VK_FALSE;
            const VkPhysicalDeviceProtectedMemoryFeatures *protected_features =
                LvlFindInChain<VkPhysicalDeviceProtectedMemoryFeatures>(pCreateInfo->pNext);
            if (protected_features) {
                protected_memory = protected_features->protectedMemory;
            } else if (vulkan_11_features) {
                protected_memory = vulkan_11_features->protectedMemory;
            }
            if (((queue_create_info.flags & VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT) != 0) && (protected_memory == VK_FALSE)) {
                skip |= LogError(physicalDevice, "VUID-VkDeviceQueueCreateInfo-flags-02861",
                                 "vkCreateDevice: pCreateInfo->flags contains VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT without the "
                                 "protectedMemory feature being enabled as well.");
            }
        }
    }

    // feature dependencies for VK_KHR_variable_pointers
    const auto *variable_pointers_features = LvlFindInChain<VkPhysicalDeviceVariablePointersFeatures>(pCreateInfo->pNext);
    VkBool32 variable_pointers = VK_FALSE;
    VkBool32 variable_pointers_storage_buffer = VK_FALSE;
    if (vulkan_11_features) {
        variable_pointers = vulkan_11_features->variablePointers;
        variable_pointers_storage_buffer = vulkan_11_features->variablePointersStorageBuffer;
    } else if (variable_pointers_features) {
        variable_pointers = variable_pointers_features->variablePointers;
        variable_pointers_storage_buffer = variable_pointers_features->variablePointersStorageBuffer;
    }
    if ((variable_pointers == VK_TRUE) && (variable_pointers_storage_buffer == VK_FALSE)) {
        skip |= LogError(
            instance, "VUID-VkPhysicalDeviceVariablePointersFeatures-variablePointers-01431",
            "vkCreateDevice(): If variablePointers is VK_TRUE then variablePointersStorageBuffer also needs to be VK_TRUE");
    }

    // feature dependencies for VK_KHR_multiview
    const auto *multiview_features = LvlFindInChain<VkPhysicalDeviceMultiviewFeatures>(pCreateInfo->pNext);
    VkBool32 multiview = VK_FALSE;
    VkBool32 multiview_geometry_shader = VK_FALSE;
    VkBool32 multiview_tessellation_shader = VK_FALSE;
    if (vulkan_11_features) {
        multiview = vulkan_11_features->multiview;
        multiview_geometry_shader = vulkan_11_features->multiviewGeometryShader;
        multiview_tessellation_shader = vulkan_11_features->multiviewTessellationShader;
    } else if (multiview_features) {
        multiview = multiview_features->multiview;
        multiview_geometry_shader = multiview_features->multiviewGeometryShader;
        multiview_tessellation_shader = multiview_features->multiviewTessellationShader;
    }
    if ((multiview == VK_FALSE) && (multiview_geometry_shader == VK_TRUE)) {
        skip |= LogError(instance, "VUID-VkPhysicalDeviceMultiviewFeatures-multiviewGeometryShader-00580",
                         "vkCreateDevice(): If multiviewGeometryShader is VK_TRUE then multiview also needs to be VK_TRUE");
    }
    if ((multiview == VK_FALSE) && (multiview_tessellation_shader == VK_TRUE)) {
        skip |= LogError(instance, "VUID-VkPhysicalDeviceMultiviewFeatures-multiviewTessellationShader-00581",
                         "vkCreateDevice(): If multiviewTessellationShader is VK_TRUE then multiview also needs to be VK_TRUE");
    }

    return skip;
}

bool StatelessValidation::ValidateGetPhysicalDeviceImageFormatProperties2(VkPhysicalDevice physicalDevice,
                                                                          const VkPhysicalDeviceImageFormatInfo2 *pImageFormatInfo,
                                                                          VkImageFormatProperties2 *pImageFormatProperties,
                                                                          const char *apiName) const {
    bool skip = false;

    if (pImageFormatInfo != nullptr) {
        const auto image_stencil_struct = LvlFindInChain<VkImageStencilUsageCreateInfo>(pImageFormatInfo->pNext);
        if (image_stencil_struct != nullptr) {
            if ((image_stencil_struct->stencilUsage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT) != 0) {
                VkImageUsageFlags legal_flags = (VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);
                // No flags other than the legal attachment bits may be set
                legal_flags |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
                if ((image_stencil_struct->stencilUsage & ~legal_flags) != 0) {
                    skip |= LogError(physicalDevice, "VUID-VkImageStencilUsageCreateInfo-stencilUsage-02539",
                                     "%s(): in pNext chain, VkImageStencilUsageCreateInfo::stencilUsage "
                                     "includes VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT, it must not include bits other than "
                                     "VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT or VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT",
                                     apiName);
                }
            }
        }
        const auto image_drm_format = LvlFindInChain<VkPhysicalDeviceImageDrmFormatModifierInfoEXT>(pImageFormatInfo->pNext);
        if (image_drm_format) {
            if (pImageFormatInfo->tiling != VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT) {
                skip |= LogError(
                    physicalDevice, "VUID-VkPhysicalDeviceImageFormatInfo2-tiling-02249",
                    "%s(): pNext chain of VkPhysicalDeviceImageFormatInfo2 includes VkPhysicalDeviceImageDrmFormatModifierInfoEXT, "
                    "but tiling (%s) is not VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT.",
                    apiName, string_VkImageTiling(pImageFormatInfo->tiling));
            }
            if (image_drm_format->sharingMode == VK_SHARING_MODE_CONCURRENT && image_drm_format->queueFamilyIndexCount <= 1) {
                skip |= LogError(
                    physicalDevice, "VUID-VkPhysicalDeviceImageDrmFormatModifierInfoEXT-sharingMode-02315",
                    "%s: pNext chain of VkPhysicalDeviceImageFormatInfo2 includes VkPhysicalDeviceImageDrmFormatModifierInfoEXT, "
                    "with sharing mode VK_SHARING_MODE_CONCURRENT, but queueFamilyIndexCount is %" PRIu32 ".",
                    apiName, image_drm_format->queueFamilyIndexCount);
            }
        } else {
            if (pImageFormatInfo->tiling == VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT) {
                skip |= LogError(
                    physicalDevice, "VUID-VkPhysicalDeviceImageFormatInfo2-tiling-02249",
                    "%s(): pNext chain of VkPhysicalDeviceImageFormatInfo2 does not include "
                    "VkPhysicalDeviceImageDrmFormatModifierInfoEXT, but tiling is VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT.",
                    apiName);
            }
        }
        if (pImageFormatInfo->tiling == VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT &&
            (pImageFormatInfo->flags & VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT)) {
            const auto format_list = LvlFindInChain<VkImageFormatListCreateInfo>(pImageFormatInfo->pNext);
            if (!format_list || format_list->viewFormatCount == 0) {
                skip |= LogError(
                    physicalDevice, "VUID-VkPhysicalDeviceImageFormatInfo2-tiling-02313",
                    "%s(): tiling is VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT and flags contain VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT "
                    "bit, but the pNext chain does not include VkImageFormatListCreateInfo with non-zero viewFormatCount.",
                    apiName);
            }
        }
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateGetPhysicalDeviceImageFormatProperties2(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2 *pImageFormatInfo,
    VkImageFormatProperties2 *pImageFormatProperties) const {
    return ValidateGetPhysicalDeviceImageFormatProperties2(physicalDevice, pImageFormatInfo, pImageFormatProperties,
                                                           "vkGetPhysicalDeviceImageFormatProperties2");
}

bool StatelessValidation::manual_PreCallValidateGetPhysicalDeviceImageFormatProperties2KHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2 *pImageFormatInfo,
    VkImageFormatProperties2 *pImageFormatProperties) const {
    return ValidateGetPhysicalDeviceImageFormatProperties2(physicalDevice, pImageFormatInfo, pImageFormatProperties,
                                                           "vkGetPhysicalDeviceImageFormatProperties2KHR");
}

bool StatelessValidation::manual_PreCallValidateGetPhysicalDeviceImageFormatProperties(
    VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage,
    VkImageCreateFlags flags, VkImageFormatProperties *pImageFormatProperties) const {
    bool skip = false;

    if (tiling == VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT) {
        skip |= LogError(physicalDevice, "VUID-vkGetPhysicalDeviceImageFormatProperties-tiling-02248",
                         "vkGetPhysicalDeviceImageFormatProperties(): tiling must not be VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT.");
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice,
                                                                                   const char *pLayerName, uint32_t *pPropertyCount,
                                                                                   VkExtensionProperties *pProperties) const {
    return ValidateArray("vkEnumerateDeviceExtensionProperties", "pPropertyCount", "pProperties", pPropertyCount, &pProperties,
                         true, false, false, kVUIDUndefined, "VUID-vkEnumerateDeviceExtensionProperties-pProperties-parameter");
}

bool StatelessValidation::ValidateDebugUtilsObjectNameInfoEXT(const std::string &api_name, VkDevice device,
                                                              const VkDebugUtilsObjectNameInfoEXT *pNameInfo) const {
    bool skip = false;
    if ((pNameInfo->objectType == VK_OBJECT_TYPE_UNKNOWN) && (pNameInfo->objectHandle == HandleToUint64(VK_NULL_HANDLE))) {
        skip |= LogError(device, "VUID-VkDebugUtilsObjectNameInfoEXT-objectType-02589",
                         "%s() objectType is VK_OBJECT_TYPE_UNKNOWN but objectHandle is VK_NULL_HANDLE", api_name.c_str());
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateSetDebugUtilsObjectNameEXT(VkDevice device,
                                                                           const VkDebugUtilsObjectNameInfoEXT *pNameInfo) const {
    bool skip = false;
    if (pNameInfo->objectType == VK_OBJECT_TYPE_UNKNOWN) {
        skip |= LogError(device, "VUID-vkSetDebugUtilsObjectNameEXT-pNameInfo-02587",
                         "vkSetDebugUtilsObjectNameEXT() pNameInfo->objectType cannot be VK_OBJECT_TYPE_UNKNOWN.");
    }
    if (pNameInfo->objectHandle == HandleToUint64(VK_NULL_HANDLE)) {
        skip |= LogError(device, "VUID-vkSetDebugUtilsObjectNameEXT-pNameInfo-02588",
                         "vkSetDebugUtilsObjectNameEXT() pNameInfo->objectHandle cannot be VK_NULL_HANDLE.");
    }
    skip |= ValidateDebugUtilsObjectNameInfoEXT("vkSetDebugUtilsObjectNameEXT", device, pNameInfo);
    return skip;
}

bool StatelessValidation::manual_PreCallValidateSetDebugUtilsObjectTagEXT(VkDevice device,
                                                                          const VkDebugUtilsObjectTagInfoEXT *pTagInfo) const {
    bool skip = false;
    if (pTagInfo->objectType == VK_OBJECT_TYPE_UNKNOWN) {
        skip |= LogError(device, "VUID-VkDebugUtilsObjectTagInfoEXT-objectType-01908",
                         "vkSetDebugUtilsObjectTagEXT() pTagInfo->objectType cannot be VK_OBJECT_TYPE_UNKNOWN.");
    }
    return skip;
}
