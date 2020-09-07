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
 * Author: Mark Lobodzinski <mark@LunarG.com>
 * Author: John Zulauf <jzulauf@lunarg.com>
 */

#include <cmath>

#include "chassis.h"
#include "stateless_validation.h"
#include "layer_chassis_dispatch.h"

static const int MaxParamCheckerStringLength = 256;

template <typename T>
inline bool in_inclusive_range(const T &value, const T &min, const T &max) {
    // Using only < for generality and || for early abort
    return !((value < min) || (max < value));
}

bool StatelessValidation::validate_string(const char *apiName, const ParameterName &stringName, const std::string &vuid,
                                          const char *validateString) const {
    bool skip = false;

    VkStringErrorFlags result = vk_string_validate(MaxParamCheckerStringLength, validateString);

    if (result == VK_STRING_ERROR_NONE) {
        return skip;
    } else if (result & VK_STRING_ERROR_LENGTH) {
        skip = LogError(device, vuid, "%s: string %s exceeds max length %d", apiName, stringName.get_name().c_str(),
                        MaxParamCheckerStringLength);
    } else if (result & VK_STRING_ERROR_BAD_DATA) {
        skip = LogError(device, vuid, "%s: string %s contains invalid characters or is badly formed", apiName,
                        stringName.get_name().c_str());
    }
    return skip;
}

bool StatelessValidation::validate_api_version(uint32_t api_version, uint32_t effective_api_version) const {
    bool skip = false;
    uint32_t api_version_nopatch = VK_MAKE_VERSION(VK_VERSION_MAJOR(api_version), VK_VERSION_MINOR(api_version), 0);
    if (api_version_nopatch != effective_api_version) {
        if (api_version_nopatch < VK_API_VERSION_1_0) {
            skip |= LogError(instance, kVUIDUndefined,
                             "Invalid CreateInstance->pCreateInfo->pApplicationInfo.apiVersion number (0x%08x). "
                             "Using VK_API_VERSION_%" PRIu32 "_%" PRIu32 ".",
                             api_version, VK_VERSION_MAJOR(effective_api_version), VK_VERSION_MINOR(effective_api_version));
        } else {
            skip |= LogWarning(instance, kVUIDUndefined,
                               "Unrecognized CreateInstance->pCreateInfo->pApplicationInfo.apiVersion number (0x%08x). "
                               "Assuming VK_API_VERSION_%" PRIu32 "_%" PRIu32 ".",
                               api_version, VK_VERSION_MAJOR(effective_api_version), VK_VERSION_MINOR(effective_api_version));
        }
    }
    return skip;
}

bool StatelessValidation::validate_instance_extensions(const VkInstanceCreateInfo *pCreateInfo) const {
    bool skip = false;
    // Create and use a local instance extension object, as an actual instance has not been created yet
    uint32_t specified_version = (pCreateInfo->pApplicationInfo ? pCreateInfo->pApplicationInfo->apiVersion : VK_API_VERSION_1_0);
    InstanceExtensions local_instance_extensions;
    local_instance_extensions.InitFromInstanceCreateInfo(specified_version, pCreateInfo);

    for (uint32_t i = 0; i < pCreateInfo->enabledExtensionCount; i++) {
        skip |= validate_extension_reqs(local_instance_extensions, "VUID-vkCreateInstance-ppEnabledExtensionNames-01388",
                                        "instance", pCreateInfo->ppEnabledExtensionNames[i]);
    }

    return skip;
}

bool StatelessValidation::SupportedByPdev(const VkPhysicalDevice physical_device, const std::string ext_name) const {
    if (instance_extensions.vk_khr_get_physical_device_properties_2) {
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

bool StatelessValidation::validate_validation_features(const VkInstanceCreateInfo *pCreateInfo,
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
    skip |= validate_api_version(local_api_version, api_version);
    skip |= validate_instance_extensions(pCreateInfo);
    const auto *validation_features = lvl_find_in_chain<VkValidationFeaturesEXT>(pCreateInfo->pNext);
    if (validation_features) skip |= validate_validation_features(pCreateInfo, validation_features);

    return skip;
}

void StatelessValidation::PostCallRecordCreateInstance(const VkInstanceCreateInfo *pCreateInfo,
                                                       const VkAllocationCallbacks *pAllocator, VkInstance *pInstance,
                                                       VkResult result) {
    auto instance_data = GetLayerDataPtr(get_dispatch_key(*pInstance), layer_data_map);
    // Copy extension data into local object
    if (result != VK_SUCCESS) return;
    this->instance_extensions = instance_data->instance_extensions;

    uint32_t pdev_count = 0;
    DispatchEnumeratePhysicalDevices(*pInstance, &pdev_count, nullptr);
    std::vector<VkPhysicalDevice> physical_devices;
    physical_devices.resize(pdev_count);
    DispatchEnumeratePhysicalDevices(*pInstance, &pdev_count, physical_devices.data());

    for (uint32_t i = 0; i < physical_devices.size(); i++) {
        auto phys_dev_props = new VkPhysicalDeviceProperties;
        DispatchGetPhysicalDeviceProperties(physical_devices[i], phys_dev_props);
        physical_device_properties_map[physical_devices[i]] = phys_dev_props;

        // Enumerate the Device Ext Properties to save the PhysicalDevice supported extension state
        uint32_t ext_count = 0;
        std::unordered_set<std::string> dev_exts_enumerated{};
        std::vector<VkExtensionProperties> ext_props{};
        instance_dispatch_table.EnumerateDeviceExtensionProperties(physical_devices[i], nullptr, &ext_count, nullptr);
        ext_props.resize(ext_count);
        instance_dispatch_table.EnumerateDeviceExtensionProperties(physical_devices[i], nullptr, &ext_count, ext_props.data());
        for (uint32_t j = 0; j < ext_count; j++) {
            dev_exts_enumerated.insert(ext_props[j].extensionName);
        }
        device_extensions_enumerated[physical_devices[i]] = std::move(dev_exts_enumerated);
    }
}

void StatelessValidation::PreCallRecordDestroyInstance(VkInstance instance, const VkAllocationCallbacks *pAllocator) {
    for (auto it = physical_device_properties_map.begin(); it != physical_device_properties_map.end();) {
        delete (it->second);
        it = physical_device_properties_map.erase(it);
    }
};

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

    if (device_extensions.vk_nv_shading_rate_image) {
        // Get the needed shading rate image limits
        auto shading_rate_image_props = lvl_init_struct<VkPhysicalDeviceShadingRateImagePropertiesNV>();
        auto prop2 = lvl_init_struct<VkPhysicalDeviceProperties2KHR>(&shading_rate_image_props);
        DispatchGetPhysicalDeviceProperties2KHR(physicalDevice, &prop2);
        phys_dev_ext_props.shading_rate_image_props = shading_rate_image_props;
    }

    if (device_extensions.vk_nv_mesh_shader) {
        // Get the needed mesh shader limits
        auto mesh_shader_props = lvl_init_struct<VkPhysicalDeviceMeshShaderPropertiesNV>();
        auto prop2 = lvl_init_struct<VkPhysicalDeviceProperties2KHR>(&mesh_shader_props);
        DispatchGetPhysicalDeviceProperties2KHR(physicalDevice, &prop2);
        phys_dev_ext_props.mesh_shader_props = mesh_shader_props;
    }

    if (device_extensions.vk_nv_ray_tracing) {
        // Get the needed ray tracing limits
        auto ray_tracing_props = lvl_init_struct<VkPhysicalDeviceRayTracingPropertiesNV>();
        auto prop2 = lvl_init_struct<VkPhysicalDeviceProperties2KHR>(&ray_tracing_props);
        DispatchGetPhysicalDeviceProperties2KHR(physicalDevice, &prop2);
        phys_dev_ext_props.ray_tracing_propsNV = ray_tracing_props;
    }

    if (device_extensions.vk_khr_ray_tracing) {
        // Get the needed ray tracing limits
        auto ray_tracing_props = lvl_init_struct<VkPhysicalDeviceRayTracingPropertiesKHR>();
        auto prop2 = lvl_init_struct<VkPhysicalDeviceProperties2KHR>(&ray_tracing_props);
        DispatchGetPhysicalDeviceProperties2KHR(physicalDevice, &prop2);
        phys_dev_ext_props.ray_tracing_propsKHR = ray_tracing_props;
    }

    if (device_extensions.vk_ext_transform_feedback) {
        // Get the needed transform feedback limits
        auto transform_feedback_props = lvl_init_struct<VkPhysicalDeviceTransformFeedbackPropertiesEXT>();
        auto prop2 = lvl_init_struct<VkPhysicalDeviceProperties2KHR>(&transform_feedback_props);
        DispatchGetPhysicalDeviceProperties2KHR(physicalDevice, &prop2);
        phys_dev_ext_props.transform_feedback_props = transform_feedback_props;
    }

    stateless_validation->phys_dev_ext_props = this->phys_dev_ext_props;

    // Save app-enabled features in this device's validation object
    // The enabled features can come from either pEnabledFeatures, or from the pNext chain
    const auto *features2 = lvl_find_in_chain<VkPhysicalDeviceFeatures2>(pCreateInfo->pNext);
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
        skip |= validate_string("vkCreateDevice", "pCreateInfo->ppEnabledLayerNames",
                                "VUID-VkDeviceCreateInfo-ppEnabledLayerNames-parameter", pCreateInfo->ppEnabledLayerNames[i]);
    }

    for (size_t i = 0; i < pCreateInfo->enabledExtensionCount; i++) {
        skip |=
            validate_string("vkCreateDevice", "pCreateInfo->ppEnabledExtensionNames",
                            "VUID-VkDeviceCreateInfo-ppEnabledExtensionNames-parameter", pCreateInfo->ppEnabledExtensionNames[i]);
        skip |= validate_extension_reqs(device_extensions, "VUID-vkCreateDevice-ppEnabledExtensionNames-01387", "device",
                                        pCreateInfo->ppEnabledExtensionNames[i]);
    }

    {
        bool maint1 = IsExtEnabled(extension_state_by_name(device_extensions, VK_KHR_MAINTENANCE1_EXTENSION_NAME));
        bool negative_viewport =
            IsExtEnabled(extension_state_by_name(device_extensions, VK_AMD_NEGATIVE_VIEWPORT_HEIGHT_EXTENSION_NAME));
        if (maint1 && negative_viewport) {
            skip |= LogError(device, "VUID-VkDeviceCreateInfo-ppEnabledExtensionNames-00374",
                             "VkDeviceCreateInfo->ppEnabledExtensionNames must not simultaneously include VK_KHR_maintenance1 and "
                             "VK_AMD_negative_viewport_height.");
        }
    }

    {
        bool khr_bda = IsExtEnabled(extension_state_by_name(device_extensions, VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME));
        bool ext_bda = IsExtEnabled(extension_state_by_name(device_extensions, VK_EXT_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME));
        if (khr_bda && ext_bda) {
            skip |= LogError(device, "VUID-VkDeviceCreateInfo-ppEnabledExtensionNames-03328",
                             "VkDeviceCreateInfo->ppEnabledExtensionNames must not contain both VK_KHR_buffer_device_address and "
                             "VK_EXT_buffer_device_address.");
        }
    }

    if (pCreateInfo->pNext != NULL && pCreateInfo->pEnabledFeatures) {
        // Check for get_physical_device_properties2 struct
        const auto *features2 = lvl_find_in_chain<VkPhysicalDeviceFeatures2KHR>(pCreateInfo->pNext);
        if (features2) {
            // Cannot include VkPhysicalDeviceFeatures2KHR and have non-null pEnabledFeatures
            skip |= LogError(device, "VUID-VkDeviceCreateInfo-pNext-00373",
                             "VkDeviceCreateInfo->pNext includes a VkPhysicalDeviceFeatures2KHR struct when "
                             "pCreateInfo->pEnabledFeatures is non-NULL.");
        }
    }

    auto features2 = lvl_find_in_chain<VkPhysicalDeviceFeatures2>(pCreateInfo->pNext);
    const VkPhysicalDeviceFeatures *features = features2 ? &features2->features : pCreateInfo->pEnabledFeatures;
    const auto *robustness2_features = lvl_find_in_chain<VkPhysicalDeviceRobustness2FeaturesEXT>(pCreateInfo->pNext);
    if (features && robustness2_features && robustness2_features->robustBufferAccess2 && !features->robustBufferAccess) {
        skip |= LogError(device, "VUID-VkPhysicalDeviceRobustness2FeaturesEXT-robustBufferAccess2-04000",
                         "If robustBufferAccess2 is enabled then robustBufferAccess must be enabled.");
    }
    const auto *raytracing_features = lvl_find_in_chain<VkPhysicalDeviceRayTracingFeaturesKHR>(pCreateInfo->pNext);
    if (raytracing_features && raytracing_features->rayTracingShaderGroupHandleCaptureReplayMixed &&
        !raytracing_features->rayTracingShaderGroupHandleCaptureReplay) {
        skip |= LogError(device, "VUID-VkPhysicalDeviceRayTracingFeaturesKHR-rayTracingShaderGroupHandleCaptureReplayMixed-03348",
                         "If rayTracingShaderGroupHandleCaptureReplayMixed is VK_TRUE, rayTracingShaderGroupHandleCaptureReplay "
                         "must also be VK_TRUE.");
    }
    auto vertex_attribute_divisor_features =
        lvl_find_in_chain<VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT>(pCreateInfo->pNext);
    if (vertex_attribute_divisor_features && (!device_extensions.vk_ext_vertex_attribute_divisor)) {
        skip |= LogError(device, kVUID_PVError_ExtensionNotEnabled,
                         "VkDeviceCreateInfo->pNext includes a VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT "
                         "struct, VK_EXT_vertex_attribute_divisor must be enabled when it creates a device.");
    }

    const auto *vulkan_11_features = lvl_find_in_chain<VkPhysicalDeviceVulkan11Features>(pCreateInfo->pNext);
    if (vulkan_11_features) {
        const VkBaseOutStructure *current = reinterpret_cast<const VkBaseOutStructure *>(pCreateInfo->pNext);
        while (current) {
            if (current->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES ||
                current->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES ||
                current->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VARIABLE_POINTERS_FEATURES ||
                current->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROTECTED_MEMORY_FEATURES ||
                current->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES ||
                current->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES) {
                skip |= LogError(
                    instance, "VUID-VkDeviceCreateInfo-pNext-02829",
                    "If the pNext chain includes a VkPhysicalDeviceVulkan11Features structure, then it must not include a "
                    "VkPhysicalDevice16BitStorageFeatures, VkPhysicalDeviceMultiviewFeatures, "
                    "VkPhysicalDeviceVariablePointersFeatures, VkPhysicalDeviceProtectedMemoryFeatures, "
                    "VkPhysicalDeviceSamplerYcbcrConversionFeatures, or VkPhysicalDeviceShaderDrawParametersFeatures structure");
                break;
            }
            current = reinterpret_cast<const VkBaseOutStructure *>(current->pNext);
        }
    }

    const auto *vulkan_12_features = lvl_find_in_chain<VkPhysicalDeviceVulkan12Features>(pCreateInfo->pNext);
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
                    "If the pNext chain includes a VkPhysicalDeviceVulkan12Features structure, then it must not include a "
                    "VkPhysicalDevice8BitStorageFeatures, VkPhysicalDeviceShaderAtomicInt64Features, "
                    "VkPhysicalDeviceShaderFloat16Int8Features, VkPhysicalDeviceDescriptorIndexingFeatures, "
                    "VkPhysicalDeviceScalarBlockLayoutFeatures, VkPhysicalDeviceImagelessFramebufferFeatures, "
                    "VkPhysicalDeviceUniformBufferStandardLayoutFeatures, VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures, "
                    "VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures, VkPhysicalDeviceHostQueryResetFeatures, "
                    "VkPhysicalDeviceTimelineSemaphoreFeatures, VkPhysicalDeviceBufferDeviceAddressFeatures, or "
                    "VkPhysicalDeviceVulkanMemoryModelFeatures structure");
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
                    instance, "VUID-VkDeviceCreateInfo-ppEnabledExtensions-02831",
                    "vkCreateDevice(): %s is enabled but VkPhysicalDeviceVulkan12Features::drawIndirectCount is not VK_TRUE.",
                    VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME);
            }
            if ((0 == strncmp(extension, VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME, VK_MAX_EXTENSION_NAME_SIZE)) &&
                (vulkan_12_features->samplerMirrorClampToEdge == VK_FALSE)) {
                skip |= LogError(instance, "VUID-VkDeviceCreateInfo-ppEnabledExtensions-02832",
                                 "vkCreateDevice(): %s is enabled but VkPhysicalDeviceVulkan12Features::samplerMirrorClampToEdge "
                                 "is not VK_TRUE.",
                                 VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME);
            }
            if ((0 == strncmp(extension, VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME, VK_MAX_EXTENSION_NAME_SIZE)) &&
                (vulkan_12_features->descriptorIndexing == VK_FALSE)) {
                skip |= LogError(
                    instance, "VUID-VkDeviceCreateInfo-ppEnabledExtensions-02833",
                    "vkCreateDevice(): %s is enabled but VkPhysicalDeviceVulkan12Features::descriptorIndexing is not VK_TRUE.",
                    VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
            }
            if ((0 == strncmp(extension, VK_EXT_SAMPLER_FILTER_MINMAX_EXTENSION_NAME, VK_MAX_EXTENSION_NAME_SIZE)) &&
                (vulkan_12_features->samplerFilterMinmax == VK_FALSE)) {
                skip |= LogError(
                    instance, "VUID-VkDeviceCreateInfo-ppEnabledExtensions-02834",
                    "vkCreateDevice(): %s is enabled but VkPhysicalDeviceVulkan12Features::samplerFilterMinmax is not VK_TRUE.",
                    VK_EXT_SAMPLER_FILTER_MINMAX_EXTENSION_NAME);
            }
            if ((0 == strncmp(extension, VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME, VK_MAX_EXTENSION_NAME_SIZE)) &&
                ((vulkan_12_features->shaderOutputViewportIndex == VK_FALSE) ||
                 (vulkan_12_features->shaderOutputLayer == VK_FALSE))) {
                skip |=
                    LogError(instance, "VUID-VkDeviceCreateInfo-ppEnabledExtensions-02835",
                             "vkCreateDevice(): %s is enabled but both VkPhysicalDeviceVulkan12Features::shaderOutputViewportIndex "
                             "and VkPhysicalDeviceVulkan12Features::shaderOutputLayer are not VK_TRUE.",
                             VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME);
            }
        }
    }

    // Validate pCreateInfo->pQueueCreateInfos
    if (pCreateInfo->pQueueCreateInfos) {
        std::unordered_set<uint32_t> set;

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
            } else if (set.count(requested_queue_family)) {
                skip |= LogError(physicalDevice, "VUID-VkDeviceCreateInfo-queueFamilyIndex-00372",
                                 "vkCreateDevice: pCreateInfo->pQueueCreateInfos[%" PRIu32 "].queueFamilyIndex (=%" PRIu32
                                 ") is not unique within pCreateInfo->pQueueCreateInfos array.",
                                 i, requested_queue_family);
            } else {
                set.insert(requested_queue_family);
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
            VkBool32 protectedMemory = VK_FALSE;
            const VkPhysicalDeviceProtectedMemoryFeatures *protected_features =
                lvl_find_in_chain<VkPhysicalDeviceProtectedMemoryFeatures>(pCreateInfo->pNext);
            if (protected_features) {
                protectedMemory = protected_features->protectedMemory;
            } else if (vulkan_11_features) {
                protectedMemory = vulkan_11_features->protectedMemory;
            }
            if ((queue_create_info.flags == VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT) && (protectedMemory == VK_FALSE)) {
                skip |= LogError(physicalDevice, "VUID-VkDeviceQueueCreateInfo-flags-02861",
                                 "vkCreateDevice: pCreateInfo->flags set to VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT without the "
                                 "protectedMemory feature being set as well.");
            }
        }
    }

    // feature dependencies for VK_KHR_variable_pointers
    const auto *variable_pointers_features = lvl_find_in_chain<VkPhysicalDeviceVariablePointersFeatures>(pCreateInfo->pNext);
    VkBool32 variablePointers = VK_FALSE;
    VkBool32 variablePointersStorageBuffer = VK_FALSE;
    if (vulkan_11_features) {
        variablePointers = vulkan_11_features->variablePointers;
        variablePointersStorageBuffer = vulkan_11_features->variablePointersStorageBuffer;
    } else if (variable_pointers_features) {
        variablePointers = variable_pointers_features->variablePointers;
        variablePointersStorageBuffer = variable_pointers_features->variablePointersStorageBuffer;
    }
    if ((variablePointers == VK_TRUE) && (variablePointersStorageBuffer == VK_FALSE)) {
        skip |= LogError(instance, "VUID-VkPhysicalDeviceVariablePointersFeatures-variablePointers-01431",
                         "If variablePointers is VK_TRUE then variablePointersStorageBuffer also needs to be VK_TRUE");
    }

    // feature dependencies for VK_KHR_multiview
    const auto *multiview_features = lvl_find_in_chain<VkPhysicalDeviceMultiviewFeatures>(pCreateInfo->pNext);
    VkBool32 multiview = VK_FALSE;
    VkBool32 multiviewGeometryShader = VK_FALSE;
    VkBool32 multiviewTessellationShader = VK_FALSE;
    if (vulkan_11_features) {
        multiview = vulkan_11_features->multiview;
        multiviewGeometryShader = vulkan_11_features->multiviewGeometryShader;
        multiviewTessellationShader = vulkan_11_features->multiviewTessellationShader;
    } else if (multiview_features) {
        multiview = multiview_features->multiview;
        multiviewGeometryShader = multiview_features->multiviewGeometryShader;
        multiviewTessellationShader = multiview_features->multiviewTessellationShader;
    }
    if ((multiview == VK_FALSE) && (multiviewGeometryShader == VK_TRUE)) {
        skip |= LogError(instance, "VUID-VkPhysicalDeviceMultiviewFeatures-multiviewGeometryShader-00580",
                         "If multiviewGeometryShader is VK_TRUE then multiview also needs to be VK_TRUE");
    }
    if ((multiview == VK_FALSE) && (multiviewTessellationShader == VK_TRUE)) {
        skip |= LogError(instance, "VUID-VkPhysicalDeviceMultiviewFeatures-multiviewTessellationShader-00581",
                         "If multiviewTessellationShader is VK_TRUE then multiview also needs to be VK_TRUE");
    }

    return skip;
}

bool StatelessValidation::require_device_extension(bool flag, char const *function_name, char const *extension_name) const {
    if (!flag) {
        return LogError(device, kVUID_PVError_ExtensionNotEnabled,
                        "%s() called even though the %s extension was not enabled for this VkDevice.", function_name,
                        extension_name);
    }

    return false;
}

bool StatelessValidation::manual_PreCallValidateCreateBuffer(VkDevice device, const VkBufferCreateInfo *pCreateInfo,
                                                             const VkAllocationCallbacks *pAllocator, VkBuffer *pBuffer) const {
    bool skip = false;

    if (pCreateInfo != nullptr) {
        skip |=
            ValidateGreaterThanZero(pCreateInfo->size, "pCreateInfo->size", "VUID-VkBufferCreateInfo-size-00912", "vkCreateBuffer");

        // Validation for parameters excluded from the generated validation code due to a 'noautovalidity' tag in vk.xml
        if (pCreateInfo->sharingMode == VK_SHARING_MODE_CONCURRENT) {
            // If sharingMode is VK_SHARING_MODE_CONCURRENT, queueFamilyIndexCount must be greater than 1
            if (pCreateInfo->queueFamilyIndexCount <= 1) {
                skip |= LogError(device, "VUID-VkBufferCreateInfo-sharingMode-00914",
                                 "vkCreateBuffer: if pCreateInfo->sharingMode is VK_SHARING_MODE_CONCURRENT, "
                                 "pCreateInfo->queueFamilyIndexCount must be greater than 1.");
            }

            // If sharingMode is VK_SHARING_MODE_CONCURRENT, pQueueFamilyIndices must be a pointer to an array of
            // queueFamilyIndexCount uint32_t values
            if (pCreateInfo->pQueueFamilyIndices == nullptr) {
                skip |= LogError(device, "VUID-VkBufferCreateInfo-sharingMode-00913",
                                 "vkCreateBuffer: if pCreateInfo->sharingMode is VK_SHARING_MODE_CONCURRENT, "
                                 "pCreateInfo->pQueueFamilyIndices must be a pointer to an array of "
                                 "pCreateInfo->queueFamilyIndexCount uint32_t values.");
            }
        }

        if ((pCreateInfo->flags & VK_BUFFER_CREATE_SPARSE_BINDING_BIT) && (!physical_device_features.sparseBinding)) {
            skip |= LogError(device, "VUID-VkBufferCreateInfo-flags-00915",
                             "vkCreateBuffer(): the sparseBinding device feature is disabled: Buffers cannot be created with the "
                             "VK_BUFFER_CREATE_SPARSE_BINDING_BIT set.");
        }

        if ((pCreateInfo->flags & VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT) && (!physical_device_features.sparseResidencyBuffer)) {
            skip |=
                LogError(device, "VUID-VkBufferCreateInfo-flags-00916",
                         "vkCreateBuffer(): the sparseResidencyBuffer device feature is disabled: Buffers cannot be created with "
                         "the VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT set.");
        }

        if ((pCreateInfo->flags & VK_BUFFER_CREATE_SPARSE_ALIASED_BIT) && (!physical_device_features.sparseResidencyAliased)) {
            skip |=
                LogError(device, "VUID-VkBufferCreateInfo-flags-00917",
                         "vkCreateBuffer(): the sparseResidencyAliased device feature is disabled: Buffers cannot be created with "
                         "the VK_BUFFER_CREATE_SPARSE_ALIASED_BIT set.");
        }

        // If flags contains VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT or VK_BUFFER_CREATE_SPARSE_ALIASED_BIT, it must also contain
        // VK_BUFFER_CREATE_SPARSE_BINDING_BIT
        if (((pCreateInfo->flags & (VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT | VK_BUFFER_CREATE_SPARSE_ALIASED_BIT)) != 0) &&
            ((pCreateInfo->flags & VK_BUFFER_CREATE_SPARSE_BINDING_BIT) != VK_BUFFER_CREATE_SPARSE_BINDING_BIT)) {
            skip |= LogError(device, "VUID-VkBufferCreateInfo-flags-00918",
                             "vkCreateBuffer: if pCreateInfo->flags contains VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT or "
                             "VK_BUFFER_CREATE_SPARSE_ALIASED_BIT, it must also contain VK_BUFFER_CREATE_SPARSE_BINDING_BIT.");
        }
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCreateImage(VkDevice device, const VkImageCreateInfo *pCreateInfo,
                                                            const VkAllocationCallbacks *pAllocator, VkImage *pImage) const {
    bool skip = false;

    if (pCreateInfo != nullptr) {
        // Validation for parameters excluded from the generated validation code due to a 'noautovalidity' tag in vk.xml
        if (pCreateInfo->sharingMode == VK_SHARING_MODE_CONCURRENT) {
            // If sharingMode is VK_SHARING_MODE_CONCURRENT, queueFamilyIndexCount must be greater than 1
            if (pCreateInfo->queueFamilyIndexCount <= 1) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-sharingMode-00942",
                                 "vkCreateImage(): if pCreateInfo->sharingMode is VK_SHARING_MODE_CONCURRENT, "
                                 "pCreateInfo->queueFamilyIndexCount must be greater than 1.");
            }

            // If sharingMode is VK_SHARING_MODE_CONCURRENT, pQueueFamilyIndices must be a pointer to an array of
            // queueFamilyIndexCount uint32_t values
            if (pCreateInfo->pQueueFamilyIndices == nullptr) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-sharingMode-00941",
                                 "vkCreateImage(): if pCreateInfo->sharingMode is VK_SHARING_MODE_CONCURRENT, "
                                 "pCreateInfo->pQueueFamilyIndices must be a pointer to an array of "
                                 "pCreateInfo->queueFamilyIndexCount uint32_t values.");
            }
        }

        skip |= ValidateGreaterThanZero(pCreateInfo->extent.width, "pCreateInfo->extent.width",
                                        "VUID-VkImageCreateInfo-extent-00944", "vkCreateImage");
        skip |= ValidateGreaterThanZero(pCreateInfo->extent.height, "pCreateInfo->extent.height",
                                        "VUID-VkImageCreateInfo-extent-00945", "vkCreateImage");
        skip |= ValidateGreaterThanZero(pCreateInfo->extent.depth, "pCreateInfo->extent.depth",
                                        "VUID-VkImageCreateInfo-extent-00946", "vkCreateImage");

        skip |= ValidateGreaterThanZero(pCreateInfo->mipLevels, "pCreateInfo->mipLevels", "VUID-VkImageCreateInfo-mipLevels-00947",
                                        "vkCreateImage");
        skip |= ValidateGreaterThanZero(pCreateInfo->arrayLayers, "pCreateInfo->arrayLayers",
                                        "VUID-VkImageCreateInfo-arrayLayers-00948", "vkCreateImage");

        // InitialLayout must be PREINITIALIZED or UNDEFINED
        if ((pCreateInfo->initialLayout != VK_IMAGE_LAYOUT_UNDEFINED) &&
            (pCreateInfo->initialLayout != VK_IMAGE_LAYOUT_PREINITIALIZED)) {
            skip |= LogError(
                device, "VUID-VkImageCreateInfo-initialLayout-00993",
                "vkCreateImage(): initialLayout is %s, must be VK_IMAGE_LAYOUT_UNDEFINED or VK_IMAGE_LAYOUT_PREINITIALIZED.",
                string_VkImageLayout(pCreateInfo->initialLayout));
        }

        // If imageType is VK_IMAGE_TYPE_1D, both extent.height and extent.depth must be 1
        if ((pCreateInfo->imageType == VK_IMAGE_TYPE_1D) &&
            ((pCreateInfo->extent.height != 1) || (pCreateInfo->extent.depth != 1))) {
            skip |= LogError(device, "VUID-VkImageCreateInfo-imageType-00956",
                             "vkCreateImage(): if pCreateInfo->imageType is VK_IMAGE_TYPE_1D, both pCreateInfo->extent.height and "
                             "pCreateInfo->extent.depth must be 1.");
        }

        if (pCreateInfo->imageType == VK_IMAGE_TYPE_2D) {
            if (pCreateInfo->flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) {
                if (pCreateInfo->extent.width != pCreateInfo->extent.height) {
                    skip |= LogError(device, "VUID-VkImageCreateInfo-imageType-00954",
                                     "vkCreateImage(): pCreateInfo->flags contains VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT, but "
                                     "pCreateInfo->extent.width (=%" PRIu32 ") and pCreateInfo->extent.height (=%" PRIu32
                                     ") are not equal.",
                                     pCreateInfo->extent.width, pCreateInfo->extent.height);
                }

                if (pCreateInfo->arrayLayers < 6) {
                    skip |= LogError(device, "VUID-VkImageCreateInfo-imageType-00954",
                                     "vkCreateImage(): pCreateInfo->flags contains VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT, but "
                                     "pCreateInfo->arrayLayers (=%" PRIu32 ") is not greater than or equal to 6.",
                                     pCreateInfo->arrayLayers);
                }
            }

            if (pCreateInfo->extent.depth != 1) {
                skip |= LogError(
                    device, "VUID-VkImageCreateInfo-imageType-00957",
                    "vkCreateImage(): if pCreateInfo->imageType is VK_IMAGE_TYPE_2D, pCreateInfo->extent.depth must be 1.");
            }
        }

        // 3D image may have only 1 layer
        if ((pCreateInfo->imageType == VK_IMAGE_TYPE_3D) && (pCreateInfo->arrayLayers != 1)) {
            skip |= LogError(device, "VUID-VkImageCreateInfo-imageType-00961",
                             "vkCreateImage(): if pCreateInfo->imageType is VK_IMAGE_TYPE_3D, pCreateInfo->arrayLayers must be 1.");
        }

        // If multi-sample, validate type, usage, tiling and mip levels.
        if ((pCreateInfo->samples != VK_SAMPLE_COUNT_1_BIT) &&
            ((pCreateInfo->imageType != VK_IMAGE_TYPE_2D) || (pCreateInfo->flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) ||
             (pCreateInfo->mipLevels != 1) || (pCreateInfo->tiling != VK_IMAGE_TILING_OPTIMAL))) {
            skip |= LogError(device, "VUID-VkImageCreateInfo-samples-02257",
                             "vkCreateImage(): Multi-sample image with incompatible type, usage, tiling, or mips.");
        }

        if (0 != (pCreateInfo->usage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT)) {
            VkImageUsageFlags legal_flags = (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
                                             VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);
            // At least one of the legal attachment bits must be set
            if (0 == (pCreateInfo->usage & legal_flags)) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-usage-00966",
                                 "vkCreateImage(): Transient attachment image without a compatible attachment flag set.");
            }
            // No flags other than the legal attachment bits may be set
            legal_flags |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
            if (0 != (pCreateInfo->usage & ~legal_flags)) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-usage-00963",
                                 "vkCreateImage(): Transient attachment image with incompatible usage flags set.");
            }
        }

        // mipLevels must be less than or equal to the number of levels in the complete mipmap chain
        uint32_t maxDim = std::max(std::max(pCreateInfo->extent.width, pCreateInfo->extent.height), pCreateInfo->extent.depth);
        // Max mip levels is different for corner-sampled images vs normal images.
        uint32_t maxMipLevels = (pCreateInfo->flags & VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV) ? (uint32_t)(ceil(log2(maxDim)))
                                                                                             : (uint32_t)(floor(log2(maxDim)) + 1);
        if (maxDim > 0 && pCreateInfo->mipLevels > maxMipLevels) {
            skip |=
                LogError(device, "VUID-VkImageCreateInfo-mipLevels-00958",
                         "vkCreateImage(): pCreateInfo->mipLevels must be less than or equal to "
                         "floor(log2(max(pCreateInfo->extent.width, pCreateInfo->extent.height, pCreateInfo->extent.depth)))+1.");
        }

        if ((pCreateInfo->flags & VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT) && (pCreateInfo->imageType != VK_IMAGE_TYPE_3D)) {
            skip |= LogError(device, "VUID-VkImageCreateInfo-flags-00950",
                             "vkCreateImage(): pCreateInfo->flags contains VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT but "
                             "pCreateInfo->imageType is not VK_IMAGE_TYPE_3D.");
        }

        if ((pCreateInfo->flags & VK_IMAGE_CREATE_SPARSE_BINDING_BIT) && (!physical_device_features.sparseBinding)) {
            skip |= LogError(device, "VUID-VkImageCreateInfo-flags-00969",
                             "vkCreateImage(): pCreateInfo->flags contains VK_IMAGE_CREATE_SPARSE_BINDING_BIT, but the "
                             "VkPhysicalDeviceFeatures::sparseBinding feature is disabled.");
        }

        if ((pCreateInfo->flags & VK_IMAGE_CREATE_SPARSE_ALIASED_BIT) && (!physical_device_features.sparseResidencyAliased)) {
            skip |= LogError(
                device, "VUID-VkImageCreateInfo-flags-01924",
                "vkCreateImage(): the sparseResidencyAliased device feature is disabled: Images cannot be created with the "
                "VK_IMAGE_CREATE_SPARSE_ALIASED_BIT set.");
        }

        // If flags contains VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT or VK_IMAGE_CREATE_SPARSE_ALIASED_BIT, it must also contain
        // VK_IMAGE_CREATE_SPARSE_BINDING_BIT
        if (((pCreateInfo->flags & (VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT | VK_IMAGE_CREATE_SPARSE_ALIASED_BIT)) != 0) &&
            ((pCreateInfo->flags & VK_IMAGE_CREATE_SPARSE_BINDING_BIT) != VK_IMAGE_CREATE_SPARSE_BINDING_BIT)) {
            skip |= LogError(device, "VUID-VkImageCreateInfo-flags-00987",
                             "vkCreateImage: if pCreateInfo->flags contains VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT or "
                             "VK_IMAGE_CREATE_SPARSE_ALIASED_BIT, it must also contain VK_IMAGE_CREATE_SPARSE_BINDING_BIT.");
        }

        // Check for combinations of attributes that are incompatible with having VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT set
        if ((pCreateInfo->flags & VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT) != 0) {
            // Linear tiling is unsupported
            if (VK_IMAGE_TILING_LINEAR == pCreateInfo->tiling) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-tiling-04121",
                                 "vkCreateImage: if pCreateInfo->flags contains VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT then image "
                                 "tiling of VK_IMAGE_TILING_LINEAR is not supported");
            }

            // Sparse 1D image isn't valid
            if (VK_IMAGE_TYPE_1D == pCreateInfo->imageType) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-imageType-00970",
                                 "vkCreateImage: cannot specify VK_IMAGE_CREATE_SPARSE_BINDING_BIT for 1D image.");
            }

            // Sparse 2D image when device doesn't support it
            if ((VK_FALSE == physical_device_features.sparseResidencyImage2D) && (VK_IMAGE_TYPE_2D == pCreateInfo->imageType)) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-imageType-00971",
                                 "vkCreateImage: cannot specify VK_IMAGE_CREATE_SPARSE_BINDING_BIT for 2D image if corresponding "
                                 "feature is not enabled on the device.");
            }

            // Sparse 3D image when device doesn't support it
            if ((VK_FALSE == physical_device_features.sparseResidencyImage3D) && (VK_IMAGE_TYPE_3D == pCreateInfo->imageType)) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-imageType-00972",
                                 "vkCreateImage: cannot specify VK_IMAGE_CREATE_SPARSE_BINDING_BIT for 3D image if corresponding "
                                 "feature is not enabled on the device.");
            }

            // Multi-sample 2D image when device doesn't support it
            if (VK_IMAGE_TYPE_2D == pCreateInfo->imageType) {
                if ((VK_FALSE == physical_device_features.sparseResidency2Samples) &&
                    (VK_SAMPLE_COUNT_2_BIT == pCreateInfo->samples)) {
                    skip |= LogError(device, "VUID-VkImageCreateInfo-imageType-00973",
                                     "vkCreateImage: cannot specify VK_IMAGE_CREATE_SPARSE_BINDING_BIT for 2-sample image if "
                                     "corresponding feature is not enabled on the device.");
                } else if ((VK_FALSE == physical_device_features.sparseResidency4Samples) &&
                           (VK_SAMPLE_COUNT_4_BIT == pCreateInfo->samples)) {
                    skip |= LogError(device, "VUID-VkImageCreateInfo-imageType-00974",
                                     "vkCreateImage: cannot specify VK_IMAGE_CREATE_SPARSE_BINDING_BIT for 4-sample image if "
                                     "corresponding feature is not enabled on the device.");
                } else if ((VK_FALSE == physical_device_features.sparseResidency8Samples) &&
                           (VK_SAMPLE_COUNT_8_BIT == pCreateInfo->samples)) {
                    skip |= LogError(device, "VUID-VkImageCreateInfo-imageType-00975",
                                     "vkCreateImage: cannot specify VK_IMAGE_CREATE_SPARSE_BINDING_BIT for 8-sample image if "
                                     "corresponding feature is not enabled on the device.");
                } else if ((VK_FALSE == physical_device_features.sparseResidency16Samples) &&
                           (VK_SAMPLE_COUNT_16_BIT == pCreateInfo->samples)) {
                    skip |= LogError(device, "VUID-VkImageCreateInfo-imageType-00976",
                                     "vkCreateImage: cannot specify VK_IMAGE_CREATE_SPARSE_BINDING_BIT for 16-sample image if "
                                     "corresponding feature is not enabled on the device.");
                }
            }
        }

        if (pCreateInfo->usage & VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV) {
            if (pCreateInfo->imageType != VK_IMAGE_TYPE_2D) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-imageType-02082",
                                 "vkCreateImage: if usage includes VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV, "
                                 "imageType must be VK_IMAGE_TYPE_2D.");
            }
            if (pCreateInfo->samples != VK_SAMPLE_COUNT_1_BIT) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-samples-02083",
                                 "vkCreateImage: if usage includes VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV, "
                                 "samples must be VK_SAMPLE_COUNT_1_BIT.");
            }
            if (pCreateInfo->tiling != VK_IMAGE_TILING_OPTIMAL) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-tiling-02084",
                                 "vkCreateImage: if usage includes VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV, "
                                 "tiling must be VK_IMAGE_TILING_OPTIMAL.");
            }
        }

        if (pCreateInfo->flags & VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV) {
            if (pCreateInfo->imageType != VK_IMAGE_TYPE_2D && pCreateInfo->imageType != VK_IMAGE_TYPE_3D) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-flags-02050",
                                 "vkCreateImage: If flags contains VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV, "
                                 "imageType must be VK_IMAGE_TYPE_2D or VK_IMAGE_TYPE_3D.");
            }

            if ((pCreateInfo->flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) || FormatIsDepthOrStencil(pCreateInfo->format)) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-flags-02051",
                                 "vkCreateImage: If flags contains VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV, "
                                 "it must not also contain VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT and format must "
                                 "not be a depth/stencil format.");
            }

            if (pCreateInfo->imageType == VK_IMAGE_TYPE_2D && (pCreateInfo->extent.width == 1 || pCreateInfo->extent.height == 1)) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-flags-02052",
                                 "vkCreateImage: If flags contains VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV and "
                                 "imageType is VK_IMAGE_TYPE_2D, extent.width and extent.height must be "
                                 "greater than 1.");
            } else if (pCreateInfo->imageType == VK_IMAGE_TYPE_3D &&
                       (pCreateInfo->extent.width == 1 || pCreateInfo->extent.height == 1 || pCreateInfo->extent.depth == 1)) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-flags-02053",
                                 "vkCreateImage: If flags contains VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV and "
                                 "imageType is VK_IMAGE_TYPE_3D, extent.width, extent.height, and extent.depth "
                                 "must be greater than 1.");
            }
        }

        if (((pCreateInfo->flags & VK_IMAGE_CREATE_SAMPLE_LOCATIONS_COMPATIBLE_DEPTH_BIT_EXT) != 0) &&
            (FormatHasDepth(pCreateInfo->format) == false)) {
            skip |= LogError(device, "VUID-VkImageCreateInfo-flags-01533",
                             "vkCreateImage(): if flags contain VK_IMAGE_CREATE_SAMPLE_LOCATIONS_COMPATIBLE_DEPTH_BIT_EXT the "
                             "format must be a depth or depth/stencil format.");
        }

        const auto image_stencil_struct = lvl_find_in_chain<VkImageStencilUsageCreateInfoEXT>(pCreateInfo->pNext);
        if (image_stencil_struct != nullptr) {
            if ((image_stencil_struct->stencilUsage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT) != 0) {
                VkImageUsageFlags legal_flags = (VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);
                // No flags other than the legal attachment bits may be set
                legal_flags |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
                if ((image_stencil_struct->stencilUsage & ~legal_flags) != 0) {
                    skip |= LogError(device, "VUID-VkImageStencilUsageCreateInfo-stencilUsage-02539",
                                     "vkCreateImage(): in pNext chain, VkImageStencilUsageCreateInfo::stencilUsage includes "
                                     "VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT, it must not include bits other than "
                                     "VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT or VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT");
                }
            }

            if (FormatIsDepthOrStencil(pCreateInfo->format)) {
                if ((image_stencil_struct->stencilUsage & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT) != 0) {
                    if (pCreateInfo->extent.width > device_limits.maxFramebufferWidth) {
                        skip |=
                            LogError(device, "VUID-VkImageCreateInfo-Format-02536",
                                     "vkCreateImage(): Depth-stencil image contains VkImageStencilUsageCreateInfo structure with "
                                     "stencilUsage including VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT and image width exceeds device "
                                     "maxFramebufferWidth");
                    }

                    if (pCreateInfo->extent.height > device_limits.maxFramebufferHeight) {
                        skip |=
                            LogError(device, "VUID-VkImageCreateInfo-format-02537",
                                     "vkCreateImage(): Depth-stencil image contains VkImageStencilUsageCreateInfo structure with "
                                     "stencilUsage including VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT and image height exceeds device "
                                     "maxFramebufferHeight");
                    }
                }

                if (!physical_device_features.shaderStorageImageMultisample &&
                    ((image_stencil_struct->stencilUsage & VK_IMAGE_USAGE_STORAGE_BIT) != 0) &&
                    (pCreateInfo->samples != VK_SAMPLE_COUNT_1_BIT)) {
                    skip |=
                        LogError(device, "VUID-VkImageCreateInfo-format-02538",
                                 "vkCreateImage(): Depth-stencil image contains VkImageStencilUsageCreateInfo structure with "
                                 "stencilUsage including VK_IMAGE_USAGE_STORAGE_BIT and the multisampled storage images feature is "
                                 "not enabled, image samples must be VK_SAMPLE_COUNT_1_BIT");
                }

                if (((pCreateInfo->usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0) &&
                    ((image_stencil_struct->stencilUsage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) == 0)) {
                    skip |= LogError(
                        device, "VUID-VkImageCreateInfo-format-02795",
                        "vkCreateImage(): Depth-stencil image in which usage includes VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT "
                        "contains VkImageStencilUsageCreateInfo structure, VkImageStencilUsageCreateInfo::stencilUsage must  "
                        "also include VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT");
                } else if (((pCreateInfo->usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) == 0) &&
                           ((image_stencil_struct->stencilUsage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0)) {
                    skip |= LogError(
                        device, "VUID-VkImageCreateInfo-format-02796",
                        "vkCreateImage(): Depth-stencil image in which usage does not include "
                        "VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT "
                        "contains VkImageStencilUsageCreateInfo structure, VkImageStencilUsageCreateInfo::stencilUsage must  "
                        "also not include VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT");
                }

                if (((pCreateInfo->usage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT) != 0) &&
                    ((image_stencil_struct->stencilUsage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT) == 0)) {
                    skip |= LogError(
                        device, "VUID-VkImageCreateInfo-format-02797",
                        "vkCreateImage(): Depth-stencil image in which usage includes VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT "
                        "contains VkImageStencilUsageCreateInfo structure, VkImageStencilUsageCreateInfo::stencilUsage must  "
                        "also include VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT");
                } else if (((pCreateInfo->usage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT) == 0) &&
                           ((image_stencil_struct->stencilUsage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT) != 0)) {
                    skip |= LogError(
                        device, "VUID-VkImageCreateInfo-format-02798",
                        "vkCreateImage(): Depth-stencil image in which usage does not include "
                        "VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT "
                        "contains VkImageStencilUsageCreateInfo structure, VkImageStencilUsageCreateInfo::stencilUsage must  "
                        "also not include VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT");
                }
            }
        }

        if ((!physical_device_features.shaderStorageImageMultisample) && ((pCreateInfo->usage & VK_IMAGE_USAGE_STORAGE_BIT) != 0) &&
            (pCreateInfo->samples != VK_SAMPLE_COUNT_1_BIT)) {
            skip |= LogError(device, "VUID-VkImageCreateInfo-usage-00968",
                             "vkCreateImage(): usage contains VK_IMAGE_USAGE_STORAGE_BIT and the multisampled storage images "
                             "feature is not enabled, image samples must be VK_SAMPLE_COUNT_1_BIT");
        }

        if (device_extensions.vk_ext_image_drm_format_modifier) {
            const auto drm_format_mod_list = lvl_find_in_chain<VkImageDrmFormatModifierListCreateInfoEXT>(pCreateInfo->pNext);
            const auto drm_format_mod_explict =
                lvl_find_in_chain<VkImageDrmFormatModifierExplicitCreateInfoEXT>(pCreateInfo->pNext);
            if (pCreateInfo->tiling == VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT) {
                if (((drm_format_mod_list != nullptr) && (drm_format_mod_explict != nullptr)) ||
                    ((drm_format_mod_list == nullptr) && (drm_format_mod_explict == nullptr))) {
                    skip |= LogError(device, "VUID-VkImageCreateInfo-tiling-02261",
                                     "vkCreateImage(): Tiling is VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT but pNext must have "
                                     "either VkImageDrmFormatModifierListCreateInfoEXT or "
                                     "VkImageDrmFormatModifierExplicitCreateInfoEXT in the pNext chain");
                }
            } else if ((drm_format_mod_list != nullptr) || (drm_format_mod_explict != nullptr)) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-pNext-02262",
                                 "vkCreateImage(): Tiling is not VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT but there is a "
                                 "VkImageDrmFormatModifierListCreateInfoEXT or VkImageDrmFormatModifierExplicitCreateInfoEXT "
                                 "in the pNext chain");
            }
        }

        if (pCreateInfo->usage & VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT) {
            if (pCreateInfo->imageType != VK_IMAGE_TYPE_2D) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-flags-02557",
                                 "vkCreateImage: if usage includes VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT, "
                                 "imageType must be VK_IMAGE_TYPE_2D.");
            }
            if (pCreateInfo->samples != VK_SAMPLE_COUNT_1_BIT) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-samples-02558",
                                 "vkCreateImage: if usage includes VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT, "
                                 "samples must be VK_SAMPLE_COUNT_1_BIT.");
            }
            if (pCreateInfo->tiling != VK_IMAGE_TILING_OPTIMAL) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-tiling-02084",
                                 "vkCreateImage: if usage includes VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT, "
                                 "tiling must be VK_IMAGE_TILING_OPTIMAL.");
            }
        }
        if (pCreateInfo->flags & VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT) {
            if (pCreateInfo->tiling != VK_IMAGE_TILING_OPTIMAL) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-flags-02565",
                                 "vkCreateImage: if usage includes VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT, "
                                 "tiling must be VK_IMAGE_TILING_OPTIMAL.");
            }
            if (pCreateInfo->imageType != VK_IMAGE_TYPE_2D) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-flags-02566",
                                 "vkCreateImage: if flags includes VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT, "
                                 "imageType must be VK_IMAGE_TYPE_2D.");
            }
            if (pCreateInfo->flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-flags-02567",
                                 "vkCreateImage: if flags includes VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT, "
                                 "flags must not include VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT.");
            }
            if (pCreateInfo->mipLevels != 1) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-flags-02568",
                                 "vkCreateImage: if flags includes VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT, mipLevels (%d) must be 1.",
                                 pCreateInfo->mipLevels);
            }
        }

        const auto swapchain_create_info = lvl_find_in_chain<VkImageSwapchainCreateInfoKHR>(pCreateInfo->pNext);
        if (swapchain_create_info != nullptr) {
            if (swapchain_create_info->swapchain != VK_NULL_HANDLE) {
                // All the following fall under the same VU that checks that the swapchain image uses parameters limited by the
                // table in #swapchain-wsi-image-create-info. Breaking up into multiple checks allows for more useful information
                // returned why this error occured. Check for matching Swapchain flags is done later in state tracking validation
                const char *vuid = "VUID-VkImageSwapchainCreateInfoKHR-swapchain-00995";
                const char *base_message = "vkCreateImage(): The image used for creating a presentable swapchain image";

                if (pCreateInfo->imageType != VK_IMAGE_TYPE_2D) {
                    // also implicitly forces the check above that extent.depth is 1
                    skip |= LogError(device, vuid, "%s must have a imageType value VK_IMAGE_TYPE_2D instead of %s.", base_message,
                                     string_VkImageType(pCreateInfo->imageType));
                }
                if (pCreateInfo->mipLevels != 1) {
                    skip |= LogError(device, vuid, "%s must have a mipLevels value of 1 instead of %u.", base_message,
                                     pCreateInfo->mipLevels);
                }
                if (pCreateInfo->samples != VK_SAMPLE_COUNT_1_BIT) {
                    skip |= LogError(device, vuid, "%s must have a samples value of VK_SAMPLE_COUNT_1_BIT instead of %s.",
                                     base_message, string_VkSampleCountFlagBits(pCreateInfo->samples));
                }
                if (pCreateInfo->tiling != VK_IMAGE_TILING_OPTIMAL) {
                    skip |= LogError(device, vuid, "%s must have a tiling value of VK_IMAGE_TILING_OPTIMAL instead of %s.",
                                     base_message, string_VkImageTiling(pCreateInfo->tiling));
                }
                if (pCreateInfo->initialLayout != VK_IMAGE_LAYOUT_UNDEFINED) {
                    skip |= LogError(device, vuid, "%s must have a initialLayout value of VK_IMAGE_LAYOUT_UNDEFINED instead of %s.",
                                     base_message, string_VkImageLayout(pCreateInfo->initialLayout));
                }
                const VkImageCreateFlags valid_flags =
                    (VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT | VK_IMAGE_CREATE_PROTECTED_BIT |
                     VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT | VK_IMAGE_CREATE_EXTENDED_USAGE_BIT_KHR);
                if ((pCreateInfo->flags & ~valid_flags) != 0) {
                    skip |= LogError(device, vuid, "%s flags are %" PRIu32 "and must only have valid flags set.", base_message,
                                     pCreateInfo->flags);
                }
            }
        }
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCreateImageView(VkDevice device, const VkImageViewCreateInfo *pCreateInfo,
                                                                const VkAllocationCallbacks *pAllocator, VkImageView *pView) const {
    bool skip = false;

    if (pCreateInfo != nullptr) {
        // Validate feature set if using CUBE_ARRAY
        if ((pCreateInfo->viewType == VK_IMAGE_VIEW_TYPE_CUBE_ARRAY) && (physical_device_features.imageCubeArray == false)) {
            skip |= LogError(pCreateInfo->image, "VUID-VkImageViewCreateInfo-viewType-01004",
                             "vkCreateImageView(): pCreateInfo->viewType can't be VK_IMAGE_VIEW_TYPE_CUBE_ARRAY without "
                             "enabling the imageCubeArray feature.");
        }

        if (pCreateInfo->subresourceRange.layerCount != VK_REMAINING_ARRAY_LAYERS) {
            if (pCreateInfo->viewType == VK_IMAGE_VIEW_TYPE_CUBE && pCreateInfo->subresourceRange.layerCount != 6) {
                skip |= LogError(device, "VUID-VkImageViewCreateInfo-viewType-02960",
                                 "vkCreateImageView(): subresourceRange.layerCount (%d) must be 6 or VK_REMAINING_ARRAY_LAYERS.",
                                 pCreateInfo->subresourceRange.layerCount);
            }
            if (pCreateInfo->viewType == VK_IMAGE_VIEW_TYPE_CUBE_ARRAY && (pCreateInfo->subresourceRange.layerCount % 6) != 0) {
                skip |= LogError(
                    device, "VUID-VkImageViewCreateInfo-viewType-02961",
                    "vkCreateImageView(): subresourceRange.layerCount (%d) must be a multiple of 6 or VK_REMAINING_ARRAY_LAYERS.",
                    pCreateInfo->subresourceRange.layerCount);
            }
        }

        auto astc_decode_mode = lvl_find_in_chain<VkImageViewASTCDecodeModeEXT>(pCreateInfo->pNext);
        if ((device_extensions.vk_ext_astc_decode_mode) && (astc_decode_mode != nullptr)) {
            if ((astc_decode_mode->decodeMode != VK_FORMAT_R16G16B16A16_SFLOAT) &&
                (astc_decode_mode->decodeMode != VK_FORMAT_R8G8B8A8_UNORM) &&
                (astc_decode_mode->decodeMode != VK_FORMAT_E5B9G9R9_UFLOAT_PACK32)) {
                skip |= LogError(device, "VUID-VkImageViewASTCDecodeModeEXT-decodeMode-02230",
                                 "vkCreateImageView(): VkImageViewASTCDecodeModeEXT::decodeMode must be "
                                 "VK_FORMAT_R16G16B16A16_SFLOAT, VK_FORMAT_R8G8B8A8_UNORM, or VK_FORMAT_E5B9G9R9_UFLOAT_PACK32.");
            }
            if (FormatIsCompressed_ASTC(pCreateInfo->format) == false) {
                skip |= LogError(device, "VUID-VkImageViewASTCDecodeModeEXT-format-04084",
                                 "vkCreateImageView(): is using a VkImageViewASTCDecodeModeEXT but the image view format is %s and "
                                 "not an ASTC format.",
                                 string_VkFormat(pCreateInfo->format));
            }
        }

        auto ycbcr_conversion = lvl_find_in_chain<VkSamplerYcbcrConversionInfo>(pCreateInfo->pNext);
        if (ycbcr_conversion != nullptr) {
            if (ycbcr_conversion->conversion != VK_NULL_HANDLE) {
                if (IsIdentitySwizzle(pCreateInfo->components) == false) {
                    skip |= LogError(
                        device, "VUID-VkImageViewCreateInfo-pNext-01970",
                        "vkCreateImageView(): If there is a VkSamplerYcbcrConversion, the imageView must "
                        "be created with the identity swizzle. Here are the actual swizzle values:\n"
                        "r swizzle = %s\n"
                        "g swizzle = %s\n"
                        "b swizzle = %s\n"
                        "a swizzle = %s\n",
                        string_VkComponentSwizzle(pCreateInfo->components.r), string_VkComponentSwizzle(pCreateInfo->components.g),
                        string_VkComponentSwizzle(pCreateInfo->components.b), string_VkComponentSwizzle(pCreateInfo->components.a));
                }
            }
        }
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateViewport(const VkViewport &viewport, const char *fn_name,
                                                         const ParameterName &parameter_name, VkCommandBuffer object) const {
    bool skip = false;

    // Note: for numerical correctness
    //       - float comparisons should expect NaN (comparison always false).
    //       - VkPhysicalDeviceLimits::maxViewportDimensions is uint32_t, not float -> careful.

    const auto f_lte_u32_exact = [](const float v1_f, const uint32_t v2_u32) {
        if (std::isnan(v1_f)) return false;
        if (v1_f <= 0.0f) return true;

        float intpart;
        const float fract = modff(v1_f, &intpart);

        assert(std::numeric_limits<float>::radix == 2);
        const float u32_max_plus1 = ldexpf(1.0f, 32);  // hopefully exact
        if (intpart >= u32_max_plus1) return false;

        uint32_t v1_u32 = static_cast<uint32_t>(intpart);
        if (v1_u32 < v2_u32)
            return true;
        else if (v1_u32 == v2_u32 && fract == 0.0f)
            return true;
        else
            return false;
    };

    const auto f_lte_u32_direct = [](const float v1_f, const uint32_t v2_u32) {
        const float v2_f = static_cast<float>(v2_u32);  // not accurate for > radix^digits; and undefined rounding mode
        return (v1_f <= v2_f);
    };

    // width
    bool width_healthy = true;
    const auto max_w = device_limits.maxViewportDimensions[0];

    if (!(viewport.width > 0.0f)) {
        width_healthy = false;
        skip |= LogError(object, "VUID-VkViewport-width-01770", "%s: %s.width (=%f) is not greater than 0.0.", fn_name,
                         parameter_name.get_name().c_str(), viewport.width);
    } else if (!(f_lte_u32_exact(viewport.width, max_w) || f_lte_u32_direct(viewport.width, max_w))) {
        width_healthy = false;
        skip |= LogError(object, "VUID-VkViewport-width-01771",
                         "%s: %s.width (=%f) exceeds VkPhysicalDeviceLimits::maxViewportDimensions[0] (=%" PRIu32 ").", fn_name,
                         parameter_name.get_name().c_str(), viewport.width, max_w);
    } else if (!f_lte_u32_exact(viewport.width, max_w) && f_lte_u32_direct(viewport.width, max_w)) {
        skip |= LogWarning(object, kVUID_PVError_NONE,
                           "%s: %s.width (=%f) technically exceeds VkPhysicalDeviceLimits::maxViewportDimensions[0] (=%" PRIu32
                           "), but it is within the static_cast<float>(maxViewportDimensions[0]) limit.",
                           fn_name, parameter_name.get_name().c_str(), viewport.width, max_w);
    }

    // height
    bool height_healthy = true;
    const bool negative_height_enabled = device_extensions.vk_khr_maintenance1 || device_extensions.vk_amd_negative_viewport_height;
    const auto max_h = device_limits.maxViewportDimensions[1];

    if (!negative_height_enabled && !(viewport.height > 0.0f)) {
        height_healthy = false;
        skip |= LogError(object, "VUID-VkViewport-height-01772", "%s: %s.height (=%f) is not greater 0.0.", fn_name,
                         parameter_name.get_name().c_str(), viewport.height);
    } else if (!(f_lte_u32_exact(fabsf(viewport.height), max_h) || f_lte_u32_direct(fabsf(viewport.height), max_h))) {
        height_healthy = false;

        skip |= LogError(object, "VUID-VkViewport-height-01773",
                         "%s: Absolute value of %s.height (=%f) exceeds VkPhysicalDeviceLimits::maxViewportDimensions[1] (=%" PRIu32
                         ").",
                         fn_name, parameter_name.get_name().c_str(), viewport.height, max_h);
    } else if (!f_lte_u32_exact(fabsf(viewport.height), max_h) && f_lte_u32_direct(fabsf(viewport.height), max_h)) {
        height_healthy = false;

        skip |= LogWarning(
            object, kVUID_PVError_NONE,
            "%s: Absolute value of %s.height (=%f) technically exceeds VkPhysicalDeviceLimits::maxViewportDimensions[1] (=%" PRIu32
            "), but it is within the static_cast<float>(maxViewportDimensions[1]) limit.",
            fn_name, parameter_name.get_name().c_str(), viewport.height, max_h);
    }

    // x
    bool x_healthy = true;
    if (!(viewport.x >= device_limits.viewportBoundsRange[0])) {
        x_healthy = false;
        skip |= LogError(object, "VUID-VkViewport-x-01774",
                         "%s: %s.x (=%f) is less than VkPhysicalDeviceLimits::viewportBoundsRange[0] (=%f).", fn_name,
                         parameter_name.get_name().c_str(), viewport.x, device_limits.viewportBoundsRange[0]);
    }

    // x + width
    if (x_healthy && width_healthy) {
        const float right_bound = viewport.x + viewport.width;
        if (!(right_bound <= device_limits.viewportBoundsRange[1])) {
            skip |= LogError(
                object, "VUID-VkViewport-x-01232",
                "%s: %s.x + %s.width (=%f + %f = %f) is greater than VkPhysicalDeviceLimits::viewportBoundsRange[1] (=%f).",
                fn_name, parameter_name.get_name().c_str(), parameter_name.get_name().c_str(), viewport.x, viewport.width,
                right_bound, device_limits.viewportBoundsRange[1]);
        }
    }

    // y
    bool y_healthy = true;
    if (!(viewport.y >= device_limits.viewportBoundsRange[0])) {
        y_healthy = false;
        skip |= LogError(object, "VUID-VkViewport-y-01775",
                         "%s: %s.y (=%f) is less than VkPhysicalDeviceLimits::viewportBoundsRange[0] (=%f).", fn_name,
                         parameter_name.get_name().c_str(), viewport.y, device_limits.viewportBoundsRange[0]);
    } else if (negative_height_enabled && !(viewport.y <= device_limits.viewportBoundsRange[1])) {
        y_healthy = false;
        skip |= LogError(object, "VUID-VkViewport-y-01776",
                         "%s: %s.y (=%f) exceeds VkPhysicalDeviceLimits::viewportBoundsRange[1] (=%f).", fn_name,
                         parameter_name.get_name().c_str(), viewport.y, device_limits.viewportBoundsRange[1]);
    }

    // y + height
    if (y_healthy && height_healthy) {
        const float boundary = viewport.y + viewport.height;

        if (!(boundary <= device_limits.viewportBoundsRange[1])) {
            skip |= LogError(object, "VUID-VkViewport-y-01233",
                             "%s: %s.y + %s.height (=%f + %f = %f) exceeds VkPhysicalDeviceLimits::viewportBoundsRange[1] (=%f).",
                             fn_name, parameter_name.get_name().c_str(), parameter_name.get_name().c_str(), viewport.y,
                             viewport.height, boundary, device_limits.viewportBoundsRange[1]);
        } else if (negative_height_enabled && !(boundary >= device_limits.viewportBoundsRange[0])) {
            skip |=
                LogError(object, "VUID-VkViewport-y-01777",
                         "%s: %s.y + %s.height (=%f + %f = %f) is less than VkPhysicalDeviceLimits::viewportBoundsRange[0] (=%f).",
                         fn_name, parameter_name.get_name().c_str(), parameter_name.get_name().c_str(), viewport.y, viewport.height,
                         boundary, device_limits.viewportBoundsRange[0]);
        }
    }

    if (!device_extensions.vk_ext_depth_range_unrestricted) {
        // minDepth
        if (!(viewport.minDepth >= 0.0) || !(viewport.minDepth <= 1.0)) {
            skip |= LogError(object, "VUID-VkViewport-minDepth-01234",

                             "%s: VK_EXT_depth_range_unrestricted extension is not enabled and %s.minDepth (=%f) is not within the "
                             "[0.0, 1.0] range.",
                             fn_name, parameter_name.get_name().c_str(), viewport.minDepth);
        }

        // maxDepth
        if (!(viewport.maxDepth >= 0.0) || !(viewport.maxDepth <= 1.0)) {
            skip |= LogError(object, "VUID-VkViewport-maxDepth-01235",

                             "%s: VK_EXT_depth_range_unrestricted extension is not enabled and %s.maxDepth (=%f) is not within the "
                             "[0.0, 1.0] range.",
                             fn_name, parameter_name.get_name().c_str(), viewport.maxDepth);
        }
    }

    return skip;
}

struct SampleOrderInfo {
    VkShadingRatePaletteEntryNV shadingRate;
    uint32_t width;
    uint32_t height;
};

// All palette entries with more than one pixel per fragment
static SampleOrderInfo sampleOrderInfos[] = {
    {VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_1X2_PIXELS_NV, 1, 2},
    {VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_2X1_PIXELS_NV, 2, 1},
    {VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_2X2_PIXELS_NV, 2, 2},
    {VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_4X2_PIXELS_NV, 4, 2},
    {VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_2X4_PIXELS_NV, 2, 4},
    {VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_4X4_PIXELS_NV, 4, 4},
};

bool StatelessValidation::ValidateCoarseSampleOrderCustomNV(const VkCoarseSampleOrderCustomNV *order) const {
    bool skip = false;

    SampleOrderInfo *sampleOrderInfo;
    uint32_t infoIdx = 0;
    for (sampleOrderInfo = nullptr; infoIdx < ARRAY_SIZE(sampleOrderInfos); ++infoIdx) {
        if (sampleOrderInfos[infoIdx].shadingRate == order->shadingRate) {
            sampleOrderInfo = &sampleOrderInfos[infoIdx];
            break;
        }
    }

    if (sampleOrderInfo == nullptr) {
        skip |= LogError(device, "VUID-VkCoarseSampleOrderCustomNV-shadingRate-02073",
                         "VkCoarseSampleOrderCustomNV shadingRate must be a shading rate "
                         "that generates fragments with more than one pixel.");
        return skip;
    }

    if (order->sampleCount == 0 || (order->sampleCount & (order->sampleCount - 1)) ||
        !(order->sampleCount & device_limits.framebufferNoAttachmentsSampleCounts)) {
        skip |= LogError(device, "VUID-VkCoarseSampleOrderCustomNV-sampleCount-02074",
                         "VkCoarseSampleOrderCustomNV sampleCount (=%" PRIu32
                         ") must "
                         "correspond to a sample count enumerated in VkSampleCountFlags whose corresponding bit "
                         "is set in framebufferNoAttachmentsSampleCounts.",
                         order->sampleCount);
    }

    if (order->sampleLocationCount != order->sampleCount * sampleOrderInfo->width * sampleOrderInfo->height) {
        skip |= LogError(device, "VUID-VkCoarseSampleOrderCustomNV-sampleLocationCount-02075",
                         "VkCoarseSampleOrderCustomNV sampleLocationCount (=%" PRIu32
                         ") must "
                         "be equal to the product of sampleCount (=%" PRIu32
                         "), the fragment width for shadingRate "
                         "(=%" PRIu32 "), and the fragment height for shadingRate (=%" PRIu32 ").",
                         order->sampleLocationCount, order->sampleCount, sampleOrderInfo->width, sampleOrderInfo->height);
    }

    if (order->sampleLocationCount > phys_dev_ext_props.shading_rate_image_props.shadingRateMaxCoarseSamples) {
        skip |= LogError(
            device, "VUID-VkCoarseSampleOrderCustomNV-sampleLocationCount-02076",
            "VkCoarseSampleOrderCustomNV sampleLocationCount (=%" PRIu32
            ") must "
            "be less than or equal to VkPhysicalDeviceShadingRateImagePropertiesNV shadingRateMaxCoarseSamples (=%" PRIu32 ").",
            order->sampleLocationCount, phys_dev_ext_props.shading_rate_image_props.shadingRateMaxCoarseSamples);
    }

    // Accumulate a bitmask tracking which (x,y,sample) tuples are seen. Expect
    // the first width*height*sampleCount bits to all be set. Note: There is no
    // guarantee that 64 bits is enough, but practically it's unlikely for an
    // implementation to support more than 32 bits for samplemask.
    assert(phys_dev_ext_props.shading_rate_image_props.shadingRateMaxCoarseSamples <= 64);
    uint64_t sampleLocationsMask = 0;
    for (uint32_t i = 0; i < order->sampleLocationCount; ++i) {
        const VkCoarseSampleLocationNV *sampleLoc = &order->pSampleLocations[i];
        if (sampleLoc->pixelX >= sampleOrderInfo->width) {
            skip |= LogError(device, "VUID-VkCoarseSampleLocationNV-pixelX-02078",
                             "pixelX must be less than the width (in pixels) of the fragment.");
        }
        if (sampleLoc->pixelY >= sampleOrderInfo->height) {
            skip |= LogError(device, "VUID-VkCoarseSampleLocationNV-pixelY-02079",
                             "pixelY must be less than the height (in pixels) of the fragment.");
        }
        if (sampleLoc->sample >= order->sampleCount) {
            skip |= LogError(device, "VUID-VkCoarseSampleLocationNV-sample-02080",
                             "sample must be less than the number of coverage samples in each pixel belonging to the fragment.");
        }
        uint32_t idx = sampleLoc->sample + order->sampleCount * (sampleLoc->pixelX + sampleOrderInfo->width * sampleLoc->pixelY);
        sampleLocationsMask |= 1ULL << idx;
    }

    uint64_t expectedMask = (order->sampleLocationCount == 64) ? ~0ULL : ((1ULL << order->sampleLocationCount) - 1);
    if (sampleLocationsMask != expectedMask) {
        skip |= LogError(
            device, "VUID-VkCoarseSampleOrderCustomNV-pSampleLocations-02077",
            "The array pSampleLocations must contain exactly one entry for "
            "every combination of valid values for pixelX, pixelY, and sample in the structure VkCoarseSampleOrderCustomNV.");
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache,
                                                                        uint32_t createInfoCount,
                                                                        const VkGraphicsPipelineCreateInfo *pCreateInfos,
                                                                        const VkAllocationCallbacks *pAllocator,
                                                                        VkPipeline *pPipelines) const {
    bool skip = false;

    if (pCreateInfos != nullptr) {
        for (uint32_t i = 0; i < createInfoCount; ++i) {
            bool has_dynamic_viewport = false;
            bool has_dynamic_scissor = false;
            bool has_dynamic_line_width = false;
            bool has_dynamic_depth_bias = false;
            bool has_dynamic_blend_constant = false;
            bool has_dynamic_depth_bounds = false;
            bool has_dynamic_stencil_compare = false;
            bool has_dynamic_stencil_write = false;
            bool has_dynamic_stencil_reference = false;
            bool has_dynamic_viewport_w_scaling_nv = false;
            bool has_dynamic_discard_rectangle_ext = false;
            bool has_dynamic_sample_locations_ext = false;
            bool has_dynamic_exclusive_scissor_nv = false;
            bool has_dynamic_shading_rate_palette_nv = false;
            bool has_dynamic_viewport_course_sample_order_nv = false;
            bool has_dynamic_line_stipple = false;
            bool has_dynamic_cull_mode = false;
            bool has_dynamic_front_face = false;
            bool has_dynamic_primitive_topology = false;
            bool has_dynamic_viewport_with_count = false;
            bool has_dynamic_scissor_with_count = false;
            bool has_dynamic_vertex_input_binding_stride = false;
            bool has_dynamic_depth_test_enable = false;
            bool has_dynamic_depth_write_enable = false;
            bool has_dynamic_depth_compare_op = false;
            bool has_dynamic_depth_bounds_test_enable = false;
            bool has_dynamic_stencil_test_enable = false;
            bool has_dynamic_stencil_op = false;
            if (pCreateInfos[i].pDynamicState != nullptr) {
                const auto &dynamic_state_info = *pCreateInfos[i].pDynamicState;
                for (uint32_t state_index = 0; state_index < dynamic_state_info.dynamicStateCount; ++state_index) {
                    const auto &dynamic_state = dynamic_state_info.pDynamicStates[state_index];
                    if (dynamic_state == VK_DYNAMIC_STATE_VIEWPORT) {
                        if (has_dynamic_viewport == true) {
                            skip |= LogError(device, "VUID-VkPipelineDynamicStateCreateInfo-pDynamicStates-01442",
                                             "vkCreateGraphicsPipelines: VK_DYNAMIC_STATE_VIEWPORT was listed twice in the "
                                             "pCreateInfos[%d].pDynamicState->pDynamicStates array",
                                             i);
                        }
                        has_dynamic_viewport = true;
                    }
                    if (dynamic_state == VK_DYNAMIC_STATE_SCISSOR) {
                        if (has_dynamic_scissor == true) {
                            skip |= LogError(device, "VUID-VkPipelineDynamicStateCreateInfo-pDynamicStates-01442",
                                             "vkCreateGraphicsPipelines: VK_DYNAMIC_STATE_SCISSOR was listed twice in the "
                                             "pCreateInfos[%d].pDynamicState->pDynamicStates array",
                                             i);
                        }
                        has_dynamic_scissor = true;
                    }
                    if (dynamic_state == VK_DYNAMIC_STATE_LINE_WIDTH) {
                        if (has_dynamic_line_width == true) {
                            skip |= LogError(device, "VUID-VkPipelineDynamicStateCreateInfo-pDynamicStates-01442",
                                             "vkCreateGraphicsPipelines: VK_DYNAMIC_STATE_LINE_WIDTH was listed twice in the "
                                             "pCreateInfos[%d].pDynamicState->pDynamicStates array",
                                             i);
                        }
                        has_dynamic_line_width = true;
                    }
                    if (dynamic_state == VK_DYNAMIC_STATE_DEPTH_BIAS) {
                        if (has_dynamic_depth_bias == true) {
                            skip |= LogError(device, "VUID-VkPipelineDynamicStateCreateInfo-pDynamicStates-01442",
                                             "vkCreateGraphicsPipelines: VK_DYNAMIC_STATE_DEPTH_BIAS was listed twice in the "
                                             "pCreateInfos[%d].pDynamicState->pDynamicStates array",
                                             i);
                        }
                        has_dynamic_depth_bias = true;
                    }
                    if (dynamic_state == VK_DYNAMIC_STATE_BLEND_CONSTANTS) {
                        if (has_dynamic_blend_constant == true) {
                            skip |= LogError(device, "VUID-VkPipelineDynamicStateCreateInfo-pDynamicStates-01442",
                                             "vkCreateGraphicsPipelines: VK_DYNAMIC_STATE_BLEND_CONSTANTS was listed twice in the "
                                             "pCreateInfos[%d].pDynamicState->pDynamicStates array",
                                             i);
                        }
                        has_dynamic_blend_constant = true;
                    }
                    if (dynamic_state == VK_DYNAMIC_STATE_DEPTH_BOUNDS) {
                        if (has_dynamic_depth_bounds == true) {
                            skip |= LogError(device, "VUID-VkPipelineDynamicStateCreateInfo-pDynamicStates-01442",
                                             "vkCreateGraphicsPipelines: VK_DYNAMIC_STATE_DEPTH_BOUNDS was listed twice in the "
                                             "pCreateInfos[%d].pDynamicState->pDynamicStates array",
                                             i);
                        }
                        has_dynamic_depth_bounds = true;
                    }
                    if (dynamic_state == VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK) {
                        if (has_dynamic_stencil_compare == true) {
                            skip |= LogError(device, "VUID-VkPipelineDynamicStateCreateInfo-pDynamicStates-01442",
                                             "vkCreateGraphicsPipelines: VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK was listed twice in "
                                             "the pCreateInfos[%d].pDynamicState->pDynamicStates array",
                                             i);
                        }
                        has_dynamic_stencil_compare = true;
                    }
                    if (dynamic_state == VK_DYNAMIC_STATE_STENCIL_WRITE_MASK) {
                        if (has_dynamic_stencil_write == true) {
                            skip |= LogError(device, "VUID-VkPipelineDynamicStateCreateInfo-pDynamicStates-01442",
                                             "vkCreateGraphicsPipelines: VK_DYNAMIC_STATE_STENCIL_WRITE_MASK was listed twice in "
                                             "the pCreateInfos[%d].pDynamicState->pDynamicStates array",
                                             i);
                        }
                        has_dynamic_stencil_write = true;
                    }
                    if (dynamic_state == VK_DYNAMIC_STATE_STENCIL_REFERENCE) {
                        if (has_dynamic_stencil_reference == true) {
                            skip |= LogError(device, "VUID-VkPipelineDynamicStateCreateInfo-pDynamicStates-01442",
                                             "vkCreateGraphicsPipelines: VK_DYNAMIC_STATE_STENCIL_REFERENCE was listed twice in "
                                             "the pCreateInfos[%d].pDynamicState->pDynamicStates array",
                                             i);
                        }
                        has_dynamic_stencil_reference = true;
                    }
                    if (dynamic_state == VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_NV) {
                        if (has_dynamic_viewport_w_scaling_nv == true) {
                            skip |= LogError(device, "VUID-VkPipelineDynamicStateCreateInfo-pDynamicStates-01442",
                                             "vkCreateGraphicsPipelines: VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_NV was listed twice "
                                             "in the pCreateInfos[%d].pDynamicState->pDynamicStates array",
                                             i);
                        }
                        has_dynamic_viewport_w_scaling_nv = true;
                    }
                    if (dynamic_state == VK_DYNAMIC_STATE_DISCARD_RECTANGLE_EXT) {
                        if (has_dynamic_discard_rectangle_ext == true) {
                            skip |= LogError(device, "VUID-VkPipelineDynamicStateCreateInfo-pDynamicStates-01442",
                                             "vkCreateGraphicsPipelines: VK_DYNAMIC_STATE_DISCARD_RECTANGLE_EXT was listed twice "
                                             "in the pCreateInfos[%d].pDynamicState->pDynamicStates array",
                                             i);
                        }
                        has_dynamic_discard_rectangle_ext = true;
                    }
                    if (dynamic_state == VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT) {
                        if (has_dynamic_sample_locations_ext == true) {
                            skip |= LogError(device, "VUID-VkPipelineDynamicStateCreateInfo-pDynamicStates-01442",
                                             "vkCreateGraphicsPipelines: VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT was listed twice in "
                                             "the pCreateInfos[%d].pDynamicState->pDynamicStates array",
                                             i);
                        }
                        has_dynamic_sample_locations_ext = true;
                    }
                    if (dynamic_state == VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_NV) {
                        if (has_dynamic_exclusive_scissor_nv == true) {
                            skip |= LogError(device, "VUID-VkPipelineDynamicStateCreateInfo-pDynamicStates-01442",
                                             "vkCreateGraphicsPipelines: VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_NV was listed twice in "
                                             "the pCreateInfos[%d].pDynamicState->pDynamicStates array",
                                             i);
                        }
                        has_dynamic_exclusive_scissor_nv = true;
                    }
                    if (dynamic_state == VK_DYNAMIC_STATE_VIEWPORT_SHADING_RATE_PALETTE_NV) {
                        if (has_dynamic_shading_rate_palette_nv == true) {
                            skip |= LogError(device, "VUID-VkPipelineDynamicStateCreateInfo-pDynamicStates-01442",
                                             "vkCreateGraphicsPipelines: VK_DYNAMIC_STATE_VIEWPORT_SHADING_RATE_PALETTE_NV was "
                                             "listed twice in the pCreateInfos[%d].pDynamicState->pDynamicStates array",
                                             i);
                        }
                        has_dynamic_shading_rate_palette_nv = true;
                    }
                    if (dynamic_state == VK_DYNAMIC_STATE_VIEWPORT_COARSE_SAMPLE_ORDER_NV) {
                        if (has_dynamic_viewport_course_sample_order_nv == true) {
                            skip |= LogError(device, "VUID-VkPipelineDynamicStateCreateInfo-pDynamicStates-01442",
                                             "vkCreateGraphicsPipelines: VK_DYNAMIC_STATE_VIEWPORT_COARSE_SAMPLE_ORDER_NV was "
                                             "listed twice in the pCreateInfos[%d].pDynamicState->pDynamicStates array",
                                             i);
                        }
                        has_dynamic_viewport_course_sample_order_nv = true;
                    }
                    if (dynamic_state == VK_DYNAMIC_STATE_LINE_STIPPLE_EXT) {
                        if (has_dynamic_line_stipple == true) {
                            skip |= LogError(device, "VUID-VkPipelineDynamicStateCreateInfo-pDynamicStates-01442",
                                             "vkCreateGraphicsPipelines: VK_DYNAMIC_STATE_LINE_STIPPLE_EXT was listed twice in the "
                                             "pCreateInfos[%d].pDynamicState->pDynamicStates array",
                                             i);
                        }
                        has_dynamic_line_stipple = true;
                    }
                    if (dynamic_state == VK_DYNAMIC_STATE_CULL_MODE_EXT) {
                        if (has_dynamic_cull_mode) {
                            skip |= LogError(device, "VUID-VkPipelineDynamicStateCreateInfo-pDynamicStates-01442",
                                             "vkCreateGraphicsPipelines: VK_DYNAMIC_STATE_CULL_MODE_EXT was listed twice in the "
                                             "pCreateInfos[%d].pDynamicState->pDynamicStates array",
                                             i);
                        }
                        has_dynamic_cull_mode = true;
                    }
                    if (dynamic_state == VK_DYNAMIC_STATE_FRONT_FACE_EXT) {
                        if (has_dynamic_front_face) {
                            skip |= LogError(device, "VUID-VkPipelineDynamicStateCreateInfo-pDynamicStates-01442",
                                             "vkCreateGraphicsPipelines: VK_DYNAMIC_STATE_FRONT_FACE_EXT was listed twice in the "
                                             "pCreateInfos[%d].pDynamicState->pDynamicStates array",
                                             i);
                        }
                        has_dynamic_front_face = true;
                    }
                    if (dynamic_state == VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY_EXT) {
                        if (has_dynamic_primitive_topology) {
                            skip |= LogError(
                                device, "VUID-VkPipelineDynamicStateCreateInfo-pDynamicStates-01442",
                                "vkCreateGraphicsPipelines: VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY_EXT was listed twice in the "
                                "pCreateInfos[%d].pDynamicState->pDynamicStates array",
                                i);
                        }
                        has_dynamic_primitive_topology = true;
                    }
                    if (dynamic_state == VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT_EXT) {
                        if (has_dynamic_viewport_with_count) {
                            skip |= LogError(
                                device, "VUID-VkPipelineDynamicStateCreateInfo-pDynamicStates-01442",
                                "vkCreateGraphicsPipelines: VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT_EXT was listed twice in the "
                                "pCreateInfos[%d].pDynamicState->pDynamicStates array",
                                i);
                        }
                        has_dynamic_viewport_with_count = true;
                    }
                    if (dynamic_state == VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT_EXT) {
                        if (has_dynamic_scissor_with_count) {
                            skip |= LogError(
                                device, "VUID-VkPipelineDynamicStateCreateInfo-pDynamicStates-01442",
                                "vkCreateGraphicsPipelines: VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT_EXT was listed twice in the "
                                "pCreateInfos[%d].pDynamicState->pDynamicStates array",
                                i);
                        }
                        has_dynamic_scissor_with_count = true;
                    }
                    if (dynamic_state == VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE_EXT) {
                        if (has_dynamic_vertex_input_binding_stride) {
                            skip |= LogError(device, "VUID-VkPipelineDynamicStateCreateInfo-pDynamicStates-01442",
                                             "vkCreateGraphicsPipelines: VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE_EXT was "
                                             "listed twice in the "
                                             "pCreateInfos[%d].pDynamicState->pDynamicStates array",
                                             i);
                        }
                        has_dynamic_vertex_input_binding_stride = true;
                    }
                    if (dynamic_state == VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE_EXT) {
                        if (has_dynamic_depth_test_enable) {
                            skip |= LogError(
                                device, "VUID-VkPipelineDynamicStateCreateInfo-pDynamicStates-01442",
                                "vkCreateGraphicsPipelines: VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE_EXT was listed twice in the "
                                "pCreateInfos[%d].pDynamicState->pDynamicStates array",
                                i);
                        }
                        has_dynamic_depth_test_enable = true;
                    }
                    if (dynamic_state == VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE_EXT) {
                        if (has_dynamic_depth_write_enable) {
                            skip |= LogError(
                                device, "VUID-VkPipelineDynamicStateCreateInfo-pDynamicStates-01442",
                                "vkCreateGraphicsPipelines: VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE_EXT was listed twice in the "
                                "pCreateInfos[%d].pDynamicState->pDynamicStates array",
                                i);
                        }
                        has_dynamic_depth_write_enable = true;
                    }
                    if (dynamic_state == VK_DYNAMIC_STATE_DEPTH_COMPARE_OP_EXT) {
                        if (has_dynamic_depth_compare_op) {
                            skip |=
                                LogError(device, "VUID-VkPipelineDynamicStateCreateInfo-pDynamicStates-01442",
                                         "vkCreateGraphicsPipelines: VK_DYNAMIC_STATE_DEPTH_COMPARE_OP_EXT was listed twice in the "
                                         "pCreateInfos[%d].pDynamicState->pDynamicStates array",
                                         i);
                        }
                        has_dynamic_depth_compare_op = true;
                    }
                    if (dynamic_state == VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE_EXT) {
                        if (has_dynamic_depth_bounds_test_enable) {
                            skip |= LogError(
                                device, "VUID-VkPipelineDynamicStateCreateInfo-pDynamicStates-01442",
                                "vkCreateGraphicsPipelines: VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE_EXT was listed twice in the "
                                "pCreateInfos[%d].pDynamicState->pDynamicStates array",
                                i);
                        }
                        has_dynamic_depth_bounds_test_enable = true;
                    }
                    if (dynamic_state == VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE_EXT) {
                        if (has_dynamic_stencil_test_enable) {
                            skip |= LogError(
                                device, "VUID-VkPipelineDynamicStateCreateInfo-pDynamicStates-01442",
                                "vkCreateGraphicsPipelines: VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE_EXT was listed twice in the "
                                "pCreateInfos[%d].pDynamicState->pDynamicStates array",
                                i);
                        }
                        has_dynamic_stencil_test_enable = true;
                    }
                    if (dynamic_state == VK_DYNAMIC_STATE_STENCIL_OP_EXT) {
                        if (has_dynamic_stencil_op) {
                            skip |= LogError(device, "VUID-VkPipelineDynamicStateCreateInfo-pDynamicStates-01442",
                                             "vkCreateGraphicsPipelines: VK_DYNAMIC_STATE_STENCIL_OP_EXT was listed twice in the "
                                             "pCreateInfos[%d].pDynamicState->pDynamicStates array",
                                             i);
                        }
                        has_dynamic_stencil_op = true;
                    }
                }
            }

            auto feedback_struct = lvl_find_in_chain<VkPipelineCreationFeedbackCreateInfoEXT>(pCreateInfos[i].pNext);
            if ((feedback_struct != nullptr) &&
                (feedback_struct->pipelineStageCreationFeedbackCount != pCreateInfos[i].stageCount)) {
                skip |= LogError(device, "VUID-VkPipelineCreationFeedbackCreateInfoEXT-pipelineStageCreationFeedbackCount-02668",
                                 "vkCreateGraphicsPipelines(): in pCreateInfo[%" PRIu32
                                 "], VkPipelineCreationFeedbackEXT::pipelineStageCreationFeedbackCount"
                                 "(=%" PRIu32 ") must equal VkGraphicsPipelineCreateInfo::stageCount(=%" PRIu32 ").",
                                 i, feedback_struct->pipelineStageCreationFeedbackCount, pCreateInfos[i].stageCount);
            }

            // Validation for parameters excluded from the generated validation code due to a 'noautovalidity' tag in vk.xml

            // Collect active stages and other information
            // Only want to loop through pStages once
            uint32_t active_shaders = 0;
            bool has_eval = false;
            bool has_control = false;
            if (pCreateInfos[i].pStages != nullptr) {
                for (uint32_t stage_index = 0; stage_index < pCreateInfos[i].stageCount; ++stage_index) {
                    active_shaders |= pCreateInfos[i].pStages[stage_index].stage;

                    if (pCreateInfos[i].pStages[stage_index].stage == VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT) {
                        has_control = true;
                    } else if (pCreateInfos[i].pStages[stage_index].stage == VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT) {
                        has_eval = true;
                    }

                    skip |= validate_string(
                        "vkCreateGraphicsPipelines",
                        ParameterName("pCreateInfos[%i].pStages[%i].pName", ParameterName::IndexVector{i, stage_index}),
                        "VUID-VkGraphicsPipelineCreateInfo-pStages-parameter", pCreateInfos[i].pStages[stage_index].pName);
                }
            }

            if ((active_shaders & VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT) &&
                (active_shaders & VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT) && (pCreateInfos[i].pTessellationState != nullptr)) {
                skip |= validate_struct_type("vkCreateGraphicsPipelines", "pCreateInfos[i].pTessellationState",
                                             "VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO",
                                             pCreateInfos[i].pTessellationState,
                                             VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO, false, kVUIDUndefined,
                                             "VUID-VkPipelineTessellationStateCreateInfo-sType-sType");

                const VkStructureType allowed_structs_VkPipelineTessellationStateCreateInfo[] = {
                    VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_DOMAIN_ORIGIN_STATE_CREATE_INFO};

                skip |= validate_struct_pnext("vkCreateGraphicsPipelines", "pCreateInfos[i].pTessellationState->pNext",
                                              "VkPipelineTessellationDomainOriginStateCreateInfo",
                                              pCreateInfos[i].pTessellationState->pNext,
                                              ARRAY_SIZE(allowed_structs_VkPipelineTessellationStateCreateInfo),
                                              allowed_structs_VkPipelineTessellationStateCreateInfo, GeneratedVulkanHeaderVersion,
                                              "VUID-VkPipelineTessellationStateCreateInfo-pNext-pNext",
                                              "VUID-VkPipelineTessellationStateCreateInfo-sType-unique");

                skip |= validate_reserved_flags("vkCreateGraphicsPipelines", "pCreateInfos[i].pTessellationState->flags",
                                                pCreateInfos[i].pTessellationState->flags,
                                                "VUID-VkPipelineTessellationStateCreateInfo-flags-zerobitmask");
            }

            if (!(active_shaders & VK_SHADER_STAGE_MESH_BIT_NV) && (pCreateInfos[i].pInputAssemblyState != nullptr)) {
                skip |= validate_struct_type("vkCreateGraphicsPipelines", "pCreateInfos[i].pInputAssemblyState",
                                             "VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO",
                                             pCreateInfos[i].pInputAssemblyState,
                                             VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, false, kVUIDUndefined,
                                             "VUID-VkPipelineInputAssemblyStateCreateInfo-sType-sType");

                skip |= validate_struct_pnext("vkCreateGraphicsPipelines", "pCreateInfos[i].pInputAssemblyState->pNext", NULL,
                                              pCreateInfos[i].pInputAssemblyState->pNext, 0, NULL, GeneratedVulkanHeaderVersion,
                                              "VUID-VkPipelineInputAssemblyStateCreateInfo-pNext-pNext", nullptr);

                skip |= validate_reserved_flags("vkCreateGraphicsPipelines", "pCreateInfos[i].pInputAssemblyState->flags",
                                                pCreateInfos[i].pInputAssemblyState->flags,
                                                "VUID-VkPipelineInputAssemblyStateCreateInfo-flags-zerobitmask");

                skip |= validate_ranged_enum("vkCreateGraphicsPipelines", "pCreateInfos[i].pInputAssemblyState->topology",
                                             "VkPrimitiveTopology", AllVkPrimitiveTopologyEnums,
                                             pCreateInfos[i].pInputAssemblyState->topology,
                                             "VUID-VkPipelineInputAssemblyStateCreateInfo-topology-parameter");

                skip |= validate_bool32("vkCreateGraphicsPipelines", "pCreateInfos[i].pInputAssemblyState->primitiveRestartEnable",
                                        pCreateInfos[i].pInputAssemblyState->primitiveRestartEnable);
            }

            if (!(active_shaders & VK_SHADER_STAGE_MESH_BIT_NV) && (pCreateInfos[i].pVertexInputState != nullptr)) {
                auto const &vertex_input_state = pCreateInfos[i].pVertexInputState;

                if (pCreateInfos[i].pVertexInputState->flags != 0) {
                    skip |= LogError(device, "VUID-VkPipelineVertexInputStateCreateInfo-flags-zerobitmask",
                                     "vkCreateGraphicsPipelines: pararameter "
                                     "pCreateInfos[%d].pVertexInputState->flags (%u) is reserved and must be zero.",
                                     i, vertex_input_state->flags);
                }

                const VkStructureType allowed_structs_VkPipelineVertexInputStateCreateInfo[] = {
                    VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_DIVISOR_STATE_CREATE_INFO_EXT};
                skip |= validate_struct_pnext("vkCreateGraphicsPipelines", "pCreateInfos[i].pVertexInputState->pNext",
                                              "VkPipelineVertexInputDivisorStateCreateInfoEXT",
                                              pCreateInfos[i].pVertexInputState->pNext, 1,
                                              allowed_structs_VkPipelineVertexInputStateCreateInfo, GeneratedVulkanHeaderVersion,
                                              "VUID-VkPipelineVertexInputStateCreateInfo-pNext-pNext",
                                              "VUID-VkPipelineVertexInputStateCreateInfo-sType-unique");
                skip |= validate_struct_type("vkCreateGraphicsPipelines", "pCreateInfos[i].pVertexInputState",
                                             "VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO", vertex_input_state,
                                             VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO, false, kVUIDUndefined,
                                             "VUID-VkPipelineVertexInputStateCreateInfo-sType-sType");
                skip |=
                    validate_array("vkCreateGraphicsPipelines", "pCreateInfos[i].pVertexInputState->vertexBindingDescriptionCount",
                                   "pCreateInfos[i].pVertexInputState->pVertexBindingDescriptions",
                                   pCreateInfos[i].pVertexInputState->vertexBindingDescriptionCount,
                                   &pCreateInfos[i].pVertexInputState->pVertexBindingDescriptions, false, true, kVUIDUndefined,
                                   "VUID-VkPipelineVertexInputStateCreateInfo-pVertexBindingDescriptions-parameter");

                skip |= validate_array(
                    "vkCreateGraphicsPipelines", "pCreateInfos[i].pVertexInputState->vertexAttributeDescriptionCount",
                    "pCreateInfos[i]->pVertexAttributeDescriptions", vertex_input_state->vertexAttributeDescriptionCount,
                    &vertex_input_state->pVertexAttributeDescriptions, false, true, kVUIDUndefined,
                    "VUID-VkPipelineVertexInputStateCreateInfo-pVertexAttributeDescriptions-parameter");

                if (pCreateInfos[i].pVertexInputState->pVertexBindingDescriptions != NULL) {
                    for (uint32_t vertexBindingDescriptionIndex = 0;
                         vertexBindingDescriptionIndex < pCreateInfos[i].pVertexInputState->vertexBindingDescriptionCount;
                         ++vertexBindingDescriptionIndex) {
                        skip |= validate_ranged_enum(
                            "vkCreateGraphicsPipelines",
                            "pCreateInfos[i].pVertexInputState->pVertexBindingDescriptions[j].inputRate", "VkVertexInputRate",
                            AllVkVertexInputRateEnums,
                            pCreateInfos[i].pVertexInputState->pVertexBindingDescriptions[vertexBindingDescriptionIndex].inputRate,
                            "VUID-VkVertexInputBindingDescription-inputRate-parameter");
                    }
                }

                if (pCreateInfos[i].pVertexInputState->pVertexAttributeDescriptions != NULL) {
                    for (uint32_t vertexAttributeDescriptionIndex = 0;
                         vertexAttributeDescriptionIndex < pCreateInfos[i].pVertexInputState->vertexAttributeDescriptionCount;
                         ++vertexAttributeDescriptionIndex) {
                        skip |= validate_ranged_enum(
                            "vkCreateGraphicsPipelines",
                            "pCreateInfos[i].pVertexInputState->pVertexAttributeDescriptions[i].format", "VkFormat",
                            AllVkFormatEnums,
                            pCreateInfos[i].pVertexInputState->pVertexAttributeDescriptions[vertexAttributeDescriptionIndex].format,
                            "VUID-VkVertexInputAttributeDescription-format-parameter");
                    }
                }

                if (vertex_input_state->vertexBindingDescriptionCount > device_limits.maxVertexInputBindings) {
                    skip |= LogError(device, "VUID-VkPipelineVertexInputStateCreateInfo-vertexBindingDescriptionCount-00613",
                                     "vkCreateGraphicsPipelines: pararameter "
                                     "pCreateInfo[%d].pVertexInputState->vertexBindingDescriptionCount (%u) is "
                                     "greater than VkPhysicalDeviceLimits::maxVertexInputBindings (%u).",
                                     i, vertex_input_state->vertexBindingDescriptionCount, device_limits.maxVertexInputBindings);
                }

                if (vertex_input_state->vertexAttributeDescriptionCount > device_limits.maxVertexInputAttributes) {
                    skip |=
                        LogError(device, "VUID-VkPipelineVertexInputStateCreateInfo-vertexAttributeDescriptionCount-00614",
                                 "vkCreateGraphicsPipelines: pararameter "
                                 "pCreateInfo[%d].pVertexInputState->vertexAttributeDescriptionCount (%u) is "
                                 "greater than VkPhysicalDeviceLimits::maxVertexInputAttributes (%u).",
                                 i, vertex_input_state->vertexAttributeDescriptionCount, device_limits.maxVertexInputAttributes);
                }

                std::unordered_set<uint32_t> vertex_bindings(vertex_input_state->vertexBindingDescriptionCount);
                for (uint32_t d = 0; d < vertex_input_state->vertexBindingDescriptionCount; ++d) {
                    auto const &vertex_bind_desc = vertex_input_state->pVertexBindingDescriptions[d];
                    auto const &binding_it = vertex_bindings.find(vertex_bind_desc.binding);
                    if (binding_it != vertex_bindings.cend()) {
                        skip |= LogError(device, "VUID-VkPipelineVertexInputStateCreateInfo-pVertexBindingDescriptions-00616",
                                         "vkCreateGraphicsPipelines: parameter "
                                         "pCreateInfo[%d].pVertexInputState->pVertexBindingDescription[%d].binding "
                                         "(%" PRIu32 ") is not distinct.",
                                         i, d, vertex_bind_desc.binding);
                    }
                    vertex_bindings.insert(vertex_bind_desc.binding);

                    if (vertex_bind_desc.binding >= device_limits.maxVertexInputBindings) {
                        skip |= LogError(device, "VUID-VkVertexInputBindingDescription-binding-00618",
                                         "vkCreateGraphicsPipelines: parameter "
                                         "pCreateInfos[%u].pVertexInputState->pVertexBindingDescriptions[%u].binding (%u) is "
                                         "greater than or equal to VkPhysicalDeviceLimits::maxVertexInputBindings (%u).",
                                         i, d, vertex_bind_desc.binding, device_limits.maxVertexInputBindings);
                    }

                    if (vertex_bind_desc.stride > device_limits.maxVertexInputBindingStride) {
                        skip |=
                            LogError(device, "VUID-VkVertexInputBindingDescription-stride-00619",
                                     "vkCreateGraphicsPipelines: parameter "
                                     "pCreateInfos[%u].pVertexInputState->pVertexBindingDescriptions[%u].stride (%u) is greater "
                                     "than VkPhysicalDeviceLimits::maxVertexInputBindingStride (%u).",
                                     i, d, vertex_bind_desc.stride, device_limits.maxVertexInputBindingStride);
                    }
                }

                std::unordered_set<uint32_t> attribute_locations(vertex_input_state->vertexAttributeDescriptionCount);
                for (uint32_t d = 0; d < vertex_input_state->vertexAttributeDescriptionCount; ++d) {
                    auto const &vertex_attrib_desc = vertex_input_state->pVertexAttributeDescriptions[d];
                    auto const &location_it = attribute_locations.find(vertex_attrib_desc.location);
                    if (location_it != attribute_locations.cend()) {
                        skip |= LogError(
                            device, "VUID-VkPipelineVertexInputStateCreateInfo-pVertexAttributeDescriptions-00617",
                            "vkCreateGraphicsPipelines: parameter "
                            "pCreateInfo[%d].pVertexInputState->vertexAttributeDescriptions[%d].location (%u) is not distinct.",
                            i, d, vertex_attrib_desc.location);
                    }
                    attribute_locations.insert(vertex_attrib_desc.location);

                    auto const &binding_it = vertex_bindings.find(vertex_attrib_desc.binding);
                    if (binding_it == vertex_bindings.cend()) {
                        skip |= LogError(
                            device, "VUID-VkPipelineVertexInputStateCreateInfo-binding-00615",
                            "vkCreateGraphicsPipelines: parameter "
                            " pCreateInfo[%d].pVertexInputState->vertexAttributeDescriptions[%d].binding (%u) does not exist "
                            "in any pCreateInfo[%d].pVertexInputState->pVertexBindingDescription.",
                            i, d, vertex_attrib_desc.binding, i);
                    }

                    if (vertex_attrib_desc.location >= device_limits.maxVertexInputAttributes) {
                        skip |= LogError(device, "VUID-VkVertexInputAttributeDescription-location-00620",
                                         "vkCreateGraphicsPipelines: parameter "
                                         "pCreateInfos[%u].pVertexInputState->pVertexAttributeDescriptions[%u].location (%u) is "
                                         "greater than or equal to VkPhysicalDeviceLimits::maxVertexInputAttributes (%u).",
                                         i, d, vertex_attrib_desc.location, device_limits.maxVertexInputAttributes);
                    }

                    if (vertex_attrib_desc.binding >= device_limits.maxVertexInputBindings) {
                        skip |= LogError(device, "VUID-VkVertexInputAttributeDescription-binding-00621",
                                         "vkCreateGraphicsPipelines: parameter "
                                         "pCreateInfos[%u].pVertexInputState->pVertexAttributeDescriptions[%u].binding (%u) is "
                                         "greater than or equal to VkPhysicalDeviceLimits::maxVertexInputBindings (%u).",
                                         i, d, vertex_attrib_desc.binding, device_limits.maxVertexInputBindings);
                    }

                    if (vertex_attrib_desc.offset > device_limits.maxVertexInputAttributeOffset) {
                        skip |= LogError(device, "VUID-VkVertexInputAttributeDescription-offset-00622",
                                         "vkCreateGraphicsPipelines: parameter "
                                         "pCreateInfos[%u].pVertexInputState->pVertexAttributeDescriptions[%u].offset (%u) is "
                                         "greater than VkPhysicalDeviceLimits::maxVertexInputAttributeOffset (%u).",
                                         i, d, vertex_attrib_desc.offset, device_limits.maxVertexInputAttributeOffset);
                    }
                }
            }

            // pTessellationState is ignored without both tessellation control and tessellation evaluation shaders stages
            if (has_control && has_eval) {
                if (pCreateInfos[i].pTessellationState == nullptr) {
                    skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-pStages-00731",
                                     "vkCreateGraphicsPipelines: if pCreateInfos[%d].pStages includes a tessellation control "
                                     "shader stage and a tessellation evaluation shader stage, "
                                     "pCreateInfos[%d].pTessellationState must not be NULL.",
                                     i, i);
                } else {
                    const VkStructureType allowed_type = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_DOMAIN_ORIGIN_STATE_CREATE_INFO;
                    skip |= validate_struct_pnext(
                        "vkCreateGraphicsPipelines",
                        ParameterName("pCreateInfos[%i].pTessellationState->pNext", ParameterName::IndexVector{i}),
                        "VkPipelineTessellationDomainOriginStateCreateInfo", pCreateInfos[i].pTessellationState->pNext, 1,
                        &allowed_type, GeneratedVulkanHeaderVersion, "VUID-VkGraphicsPipelineCreateInfo-pNext-pNext",
                        "VUID-VkGraphicsPipelineCreateInfo-sType-unique");

                    skip |= validate_reserved_flags(
                        "vkCreateGraphicsPipelines",
                        ParameterName("pCreateInfos[%i].pTessellationState->flags", ParameterName::IndexVector{i}),
                        pCreateInfos[i].pTessellationState->flags, "VUID-VkPipelineTessellationStateCreateInfo-flags-zerobitmask");

                    if (pCreateInfos[i].pTessellationState->patchControlPoints == 0 ||
                        pCreateInfos[i].pTessellationState->patchControlPoints > device_limits.maxTessellationPatchSize) {
                        skip |= LogError(device, "VUID-VkPipelineTessellationStateCreateInfo-patchControlPoints-01214",
                                         "vkCreateGraphicsPipelines: invalid parameter "
                                         "pCreateInfos[%d].pTessellationState->patchControlPoints value %u. patchControlPoints "
                                         "should be >0 and <=%u.",
                                         i, pCreateInfos[i].pTessellationState->patchControlPoints,
                                         device_limits.maxTessellationPatchSize);
                    }
                }
            }

            // pViewportState, pMultisampleState, pDepthStencilState, and pColorBlendState ignored when rasterization is disabled
            if ((pCreateInfos[i].pRasterizationState != nullptr) &&
                (pCreateInfos[i].pRasterizationState->rasterizerDiscardEnable == VK_FALSE)) {
                if (pCreateInfos[i].pViewportState == nullptr) {
                    skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-rasterizerDiscardEnable-00750",
                                     "vkCreateGraphicsPipelines: Rasterization is enabled (pCreateInfos[%" PRIu32
                                     "].pRasterizationState->rasterizerDiscardEnable is VK_FALSE), but pCreateInfos[%" PRIu32
                                     "].pViewportState (=NULL) is not a valid pointer.",
                                     i, i);
                } else {
                    const auto &viewport_state = *pCreateInfos[i].pViewportState;

                    if (viewport_state.sType != VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO) {
                        skip |= LogError(device, "VUID-VkPipelineViewportStateCreateInfo-sType-sType",
                                         "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32
                                         "].pViewportState->sType is not VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO.",
                                         i);
                    }

                    const VkStructureType allowed_structs_VkPipelineViewportStateCreateInfo[] = {
                        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_SWIZZLE_STATE_CREATE_INFO_NV,
                        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_W_SCALING_STATE_CREATE_INFO_NV,
                        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_EXCLUSIVE_SCISSOR_STATE_CREATE_INFO_NV,
                        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_SHADING_RATE_IMAGE_STATE_CREATE_INFO_NV,
                        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_COARSE_SAMPLE_ORDER_STATE_CREATE_INFO_NV,
                    };
                    skip |= validate_struct_pnext(
                        "vkCreateGraphicsPipelines",
                        ParameterName("pCreateInfos[%i].pViewportState->pNext", ParameterName::IndexVector{i}),
                        "VkPipelineViewportSwizzleStateCreateInfoNV, VkPipelineViewportWScalingStateCreateInfoNV, "
                        "VkPipelineViewportExclusiveScissorStateCreateInfoNV, VkPipelineViewportShadingRateImageStateCreateInfoNV, "
                        "VkPipelineViewportCoarseSampleOrderStateCreateInfoNV",
                        viewport_state.pNext, ARRAY_SIZE(allowed_structs_VkPipelineViewportStateCreateInfo),
                        allowed_structs_VkPipelineViewportStateCreateInfo, 65, "VUID-VkPipelineViewportStateCreateInfo-pNext-pNext",
                        "VUID-VkPipelineViewportStateCreateInfo-sType-unique");

                    skip |= validate_reserved_flags(
                        "vkCreateGraphicsPipelines",
                        ParameterName("pCreateInfos[%i].pViewportState->flags", ParameterName::IndexVector{i}),
                        viewport_state.flags, "VUID-VkPipelineViewportStateCreateInfo-flags-zerobitmask");

                    auto exclusive_scissor_struct = lvl_find_in_chain<VkPipelineViewportExclusiveScissorStateCreateInfoNV>(
                        pCreateInfos[i].pViewportState->pNext);
                    auto shading_rate_image_struct = lvl_find_in_chain<VkPipelineViewportShadingRateImageStateCreateInfoNV>(
                        pCreateInfos[i].pViewportState->pNext);
                    auto coarse_sample_order_struct = lvl_find_in_chain<VkPipelineViewportCoarseSampleOrderStateCreateInfoNV>(
                        pCreateInfos[i].pViewportState->pNext);
                    const auto vp_swizzle_struct =
                        lvl_find_in_chain<VkPipelineViewportSwizzleStateCreateInfoNV>(pCreateInfos[i].pViewportState->pNext);
                    const auto vp_w_scaling_struct =
                        lvl_find_in_chain<VkPipelineViewportWScalingStateCreateInfoNV>(pCreateInfos[i].pViewportState->pNext);

                    if (!physical_device_features.multiViewport) {
                        if (viewport_state.viewportCount != 1) {
                            skip |= LogError(device, "VUID-VkPipelineViewportStateCreateInfo-viewportCount-01216",
                                             "vkCreateGraphicsPipelines: The VkPhysicalDeviceFeatures::multiViewport feature is "
                                             "disabled, but pCreateInfos[%" PRIu32 "].pViewportState->viewportCount (=%" PRIu32
                                             ") is not 1.",
                                             i, viewport_state.viewportCount);
                        }

                        if (viewport_state.scissorCount != 1) {
                            skip |= LogError(device, "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01217",
                                             "vkCreateGraphicsPipelines: The VkPhysicalDeviceFeatures::multiViewport feature is "
                                             "disabled, but pCreateInfos[%" PRIu32 "].pViewportState->scissorCount (=%" PRIu32
                                             ") is not 1.",
                                             i, viewport_state.scissorCount);
                        }

                        if (exclusive_scissor_struct && (exclusive_scissor_struct->exclusiveScissorCount != 0 &&
                                                         exclusive_scissor_struct->exclusiveScissorCount != 1)) {
                            skip |= LogError(
                                device, "VUID-VkPipelineViewportExclusiveScissorStateCreateInfoNV-exclusiveScissorCount-02027",
                                "vkCreateGraphicsPipelines: The VkPhysicalDeviceFeatures::multiViewport feature is "
                                "disabled, but pCreateInfos[%" PRIu32
                                "] VkPipelineViewportExclusiveScissorStateCreateInfoNV::exclusiveScissorCount (=%" PRIu32
                                ") is not 1.",
                                i, exclusive_scissor_struct->exclusiveScissorCount);
                        }

                        if (shading_rate_image_struct &&
                            (shading_rate_image_struct->viewportCount != 0 && shading_rate_image_struct->viewportCount != 1)) {
                            skip |= LogError(device, "VUID-VkPipelineViewportShadingRateImageStateCreateInfoNV-viewportCount-02054",
                                             "vkCreateGraphicsPipelines: The VkPhysicalDeviceFeatures::multiViewport feature is "
                                             "disabled, but pCreateInfos[%" PRIu32
                                             "] VkPipelineViewportShadingRateImageStateCreateInfoNV::viewportCount (=%" PRIu32
                                             ") is neither 0 nor 1.",
                                             i, shading_rate_image_struct->viewportCount);
                        }

                    } else {  // multiViewport enabled
                        if (viewport_state.viewportCount == 0) {
                            if (!has_dynamic_viewport_with_count) {
                                skip |= LogError(
                                    device, "VUID-VkPipelineViewportStateCreateInfo-viewportCount-arraylength",
                                    "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32 "].pViewportState->viewportCount is 0.", i);
                            }
                        } else if (viewport_state.viewportCount > device_limits.maxViewports) {
                            skip |= LogError(device, "VUID-VkPipelineViewportStateCreateInfo-viewportCount-01218",
                                             "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32
                                             "].pViewportState->viewportCount (=%" PRIu32
                                             ") is greater than VkPhysicalDeviceLimits::maxViewports (=%" PRIu32 ").",
                                             i, viewport_state.viewportCount, device_limits.maxViewports);
                        } else if (has_dynamic_viewport_with_count) {
                            skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-03379",
                                             "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32
                                             "].pViewportState->viewportCount (=%" PRIu32
                                             ") must be zero when VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT_EXT is used.",
                                             i, viewport_state.viewportCount);
                        }

                        if (viewport_state.scissorCount == 0) {
                            if (!has_dynamic_scissor_with_count) {
                                skip |= LogError(
                                    device, "VUID-VkPipelineViewportStateCreateInfo-scissorCount-arraylength",
                                    "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32 "].pViewportState->scissorCount is 0.", i);
                            }
                        } else if (viewport_state.scissorCount > device_limits.maxViewports) {
                            skip |= LogError(device, "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01219",
                                             "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32
                                             "].pViewportState->scissorCount (=%" PRIu32
                                             ") is greater than VkPhysicalDeviceLimits::maxViewports (=%" PRIu32 ").",
                                             i, viewport_state.scissorCount, device_limits.maxViewports);
                        } else if (has_dynamic_scissor_with_count) {
                            skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-03380",
                                             "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32
                                             "].pViewportState->scissorCount (=%" PRIu32
                                             ") must be zero when VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT_EXT is used.",
                                             i, viewport_state.viewportCount);
                        }
                    }

                    if (exclusive_scissor_struct && exclusive_scissor_struct->exclusiveScissorCount > device_limits.maxViewports) {
                        skip |=
                            LogError(device, "VUID-VkPipelineViewportExclusiveScissorStateCreateInfoNV-exclusiveScissorCount-02028",
                                     "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32 "] exclusiveScissorCount (=%" PRIu32
                                     ") is greater than VkPhysicalDeviceLimits::maxViewports (=%" PRIu32 ").",
                                     i, exclusive_scissor_struct->exclusiveScissorCount, device_limits.maxViewports);
                    }

                    if (shading_rate_image_struct && shading_rate_image_struct->viewportCount > device_limits.maxViewports) {
                        skip |= LogError(device, "VUID-VkPipelineViewportShadingRateImageStateCreateInfoNV-viewportCount-02055",
                                         "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32
                                         "] VkPipelineViewportShadingRateImageStateCreateInfoNV viewportCount (=%" PRIu32
                                         ") is greater than VkPhysicalDeviceLimits::maxViewports (=%" PRIu32 ").",
                                         i, shading_rate_image_struct->viewportCount, device_limits.maxViewports);
                    }

                    if (viewport_state.scissorCount != viewport_state.viewportCount &&
                        !(has_dynamic_viewport_with_count || has_dynamic_scissor_with_count)) {
                        skip |= LogError(device, "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01220",
                                         "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32
                                         "].pViewportState->scissorCount (=%" PRIu32 ") is not identical to pCreateInfos[%" PRIu32
                                         "].pViewportState->viewportCount (=%" PRIu32 ").",
                                         i, viewport_state.scissorCount, i, viewport_state.viewportCount);
                    }

                    if (exclusive_scissor_struct && exclusive_scissor_struct->exclusiveScissorCount != 0 &&
                        exclusive_scissor_struct->exclusiveScissorCount != viewport_state.viewportCount) {
                        skip |=
                            LogError(device, "VUID-VkPipelineViewportExclusiveScissorStateCreateInfoNV-exclusiveScissorCount-02029",
                                     "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32 "] exclusiveScissorCount (=%" PRIu32
                                     ") must be zero or identical to pCreateInfos[%" PRIu32
                                     "].pViewportState->viewportCount (=%" PRIu32 ").",
                                     i, exclusive_scissor_struct->exclusiveScissorCount, i, viewport_state.viewportCount);
                    }

                    if (shading_rate_image_struct && shading_rate_image_struct->shadingRateImageEnable &&
                        shading_rate_image_struct->viewportCount != viewport_state.viewportCount) {
                        skip |= LogError(
                            device, "VUID-VkPipelineViewportShadingRateImageStateCreateInfoNV-shadingRateImageEnable-02056",
                            "vkCreateGraphicsPipelines: If shadingRateImageEnable is enabled, pCreateInfos[%" PRIu32
                            "] "
                            "VkPipelineViewportShadingRateImageStateCreateInfoNV viewportCount (=%" PRIu32
                            ") must identical to pCreateInfos[%" PRIu32 "].pViewportState->viewportCount (=%" PRIu32 ").",
                            i, shading_rate_image_struct->viewportCount, i, viewport_state.viewportCount);
                    }

                    if (!has_dynamic_viewport && viewport_state.viewportCount > 0 && viewport_state.pViewports == nullptr) {
                        skip |= LogError(
                            device, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-00747",
                            "vkCreateGraphicsPipelines: The viewport state is static (pCreateInfos[%" PRIu32
                            "].pDynamicState->pDynamicStates does not contain VK_DYNAMIC_STATE_VIEWPORT), but pCreateInfos[%" PRIu32
                            "].pViewportState->pViewports (=NULL) is an invalid pointer.",
                            i, i);
                    }

                    if (!has_dynamic_scissor && viewport_state.scissorCount > 0 && viewport_state.pScissors == nullptr) {
                        skip |= LogError(
                            device, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-00748",
                            "vkCreateGraphicsPipelines: The scissor state is static (pCreateInfos[%" PRIu32
                            "].pDynamicState->pDynamicStates does not contain VK_DYNAMIC_STATE_SCISSOR), but pCreateInfos[%" PRIu32
                            "].pViewportState->pScissors (=NULL) is an invalid pointer.",
                            i, i);
                    }

                    if (!has_dynamic_exclusive_scissor_nv && exclusive_scissor_struct &&
                        exclusive_scissor_struct->exclusiveScissorCount > 0 &&
                        exclusive_scissor_struct->pExclusiveScissors == nullptr) {
                        skip |=
                            LogError(device, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-04056",
                                     "vkCreateGraphicsPipelines: The exclusive scissor state is static (pCreateInfos[%" PRIu32
                                     "].pDynamicState->pDynamicStates does not contain VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_NV), but "
                                     "pCreateInfos[%" PRIu32 "] pExclusiveScissors (=NULL) is an invalid pointer.",
                                     i, i);
                    }

                    if (!has_dynamic_shading_rate_palette_nv && shading_rate_image_struct &&
                        shading_rate_image_struct->viewportCount > 0 &&
                        shading_rate_image_struct->pShadingRatePalettes == nullptr) {
                        skip |= LogError(
                            device, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-04057",
                            "vkCreateGraphicsPipelines: The shading rate palette state is static (pCreateInfos[%" PRIu32
                            "].pDynamicState->pDynamicStates does not contain VK_DYNAMIC_STATE_VIEWPORT_SHADING_RATE_PALETTE_NV), "
                            "but pCreateInfos[%" PRIu32 "] pShadingRatePalettes (=NULL) is an invalid pointer.",
                            i, i);
                    }

                    if (vp_swizzle_struct) {
                        if (vp_swizzle_struct->viewportCount != viewport_state.viewportCount) {
                            skip |= LogError(device, "VUID-VkPipelineViewportSwizzleStateCreateInfoNV-viewportCount-01215",
                                             "vkCreateGraphicsPipelines: The viewport swizzle state vieport count of %" PRIu32
                                             " does "
                                             "not match the viewport count of %" PRIu32 " in VkPipelineViewportStateCreateInfo.",
                                             vp_swizzle_struct->viewportCount, viewport_state.viewportCount);
                        }
                    }

                    // validate the VkViewports
                    if (!has_dynamic_viewport && viewport_state.pViewports) {
                        for (uint32_t viewport_i = 0; viewport_i < viewport_state.viewportCount; ++viewport_i) {
                            const auto &viewport = viewport_state.pViewports[viewport_i];  // will crash on invalid ptr
                            const char *fn_name = "vkCreateGraphicsPipelines";
                            skip |= manual_PreCallValidateViewport(viewport, fn_name,
                                                                   ParameterName("pCreateInfos[%i].pViewportState->pViewports[%i]",
                                                                                 ParameterName::IndexVector{i, viewport_i}),
                                                                   VkCommandBuffer(0));
                        }
                    }

                    if (has_dynamic_viewport_w_scaling_nv && !device_extensions.vk_nv_clip_space_w_scaling) {
                        skip |= LogError(device, kVUID_PVError_ExtensionNotEnabled,
                                         "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32
                                         "].pDynamicState->pDynamicStates contains VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_NV, but "
                                         "VK_NV_clip_space_w_scaling extension is not enabled.",
                                         i);
                    }

                    if (has_dynamic_discard_rectangle_ext && !device_extensions.vk_ext_discard_rectangles) {
                        skip |= LogError(device, kVUID_PVError_ExtensionNotEnabled,
                                         "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32
                                         "].pDynamicState->pDynamicStates contains VK_DYNAMIC_STATE_DISCARD_RECTANGLE_EXT, but "
                                         "VK_EXT_discard_rectangles extension is not enabled.",
                                         i);
                    }

                    if (has_dynamic_sample_locations_ext && !device_extensions.vk_ext_sample_locations) {
                        skip |= LogError(device, kVUID_PVError_ExtensionNotEnabled,
                                         "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32
                                         "].pDynamicState->pDynamicStates contains VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT, but "
                                         "VK_EXT_sample_locations extension is not enabled.",
                                         i);
                    }

                    if (has_dynamic_exclusive_scissor_nv && !device_extensions.vk_nv_scissor_exclusive) {
                        skip |= LogError(device, kVUID_PVError_ExtensionNotEnabled,
                                         "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32
                                         "].pDynamicState->pDynamicStates contains VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_NV, but "
                                         "VK_NV_scissor_exclusive extension is not enabled.",
                                         i);
                    }

                    if (coarse_sample_order_struct &&
                        coarse_sample_order_struct->sampleOrderType != VK_COARSE_SAMPLE_ORDER_TYPE_CUSTOM_NV &&
                        coarse_sample_order_struct->customSampleOrderCount != 0) {
                        skip |= LogError(device, "VUID-VkPipelineViewportCoarseSampleOrderStateCreateInfoNV-sampleOrderType-02072",
                                         "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32
                                         "] "
                                         "VkPipelineViewportCoarseSampleOrderStateCreateInfoNV sampleOrderType is not "
                                         "VK_COARSE_SAMPLE_ORDER_TYPE_CUSTOM_NV and customSampleOrderCount is not 0.",
                                         i);
                    }

                    if (coarse_sample_order_struct) {
                        for (uint32_t order_i = 0; order_i < coarse_sample_order_struct->customSampleOrderCount; ++order_i) {
                            skip |= ValidateCoarseSampleOrderCustomNV(&coarse_sample_order_struct->pCustomSampleOrders[order_i]);
                        }
                    }

                    if (vp_w_scaling_struct && (vp_w_scaling_struct->viewportWScalingEnable == VK_TRUE)) {
                        if (vp_w_scaling_struct->viewportCount != viewport_state.viewportCount) {
                            skip |= LogError(device, "VUID-VkPipelineViewportStateCreateInfo-viewportWScalingEnable-01726",
                                             "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32
                                             "] "
                                             "VkPipelineViewportWScalingStateCreateInfoNV.viewportCount (=%" PRIu32
                                             ") "
                                             "is not equal to VkPipelineViewportStateCreateInfo.viewportCount (=%" PRIu32 ").",
                                             i, vp_w_scaling_struct->viewportCount, viewport_state.viewportCount);
                        }
                        if (!has_dynamic_viewport_w_scaling_nv && !vp_w_scaling_struct->pViewportWScalings) {
                            skip |= LogError(
                                device, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-01715",
                                "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32
                                "] "
                                "VkPipelineViewportWScalingStateCreateInfoNV.pViewportWScalings (=NULL) is not a valid array.",
                                i);
                        }
                    }
                }

                if (pCreateInfos[i].pMultisampleState == nullptr) {
                    skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-rasterizerDiscardEnable-00751",
                                     "vkCreateGraphicsPipelines: if pCreateInfos[%d].pRasterizationState->rasterizerDiscardEnable "
                                     "is VK_FALSE, pCreateInfos[%d].pMultisampleState must not be NULL.",
                                     i, i);
                } else {
                    const VkStructureType valid_next_stypes[] = {LvlTypeMap<VkPipelineCoverageModulationStateCreateInfoNV>::kSType,
                                                                 LvlTypeMap<VkPipelineCoverageReductionStateCreateInfoNV>::kSType,
                                                                 LvlTypeMap<VkPipelineCoverageToColorStateCreateInfoNV>::kSType,
                                                                 LvlTypeMap<VkPipelineSampleLocationsStateCreateInfoEXT>::kSType};
                    const char *valid_struct_names =
                        "VkPipelineCoverageModulationStateCreateInfoNV, VkPipelineCoverageToColorStateCreateInfoNV, "
                        "VkPipelineSampleLocationsStateCreateInfoEXT";
                    skip |= validate_struct_pnext(
                        "vkCreateGraphicsPipelines",
                        ParameterName("pCreateInfos[%i].pMultisampleState->pNext", ParameterName::IndexVector{i}),
                        valid_struct_names, pCreateInfos[i].pMultisampleState->pNext, 4, valid_next_stypes,
                        GeneratedVulkanHeaderVersion, "VUID-VkPipelineMultisampleStateCreateInfo-pNext-pNext",
                        "VUID-VkPipelineMultisampleStateCreateInfo-sType-unique");

                    skip |= validate_reserved_flags(
                        "vkCreateGraphicsPipelines",
                        ParameterName("pCreateInfos[%i].pMultisampleState->flags", ParameterName::IndexVector{i}),
                        pCreateInfos[i].pMultisampleState->flags, "VUID-VkPipelineMultisampleStateCreateInfo-flags-zerobitmask");

                    skip |= validate_bool32(
                        "vkCreateGraphicsPipelines",
                        ParameterName("pCreateInfos[%i].pMultisampleState->sampleShadingEnable", ParameterName::IndexVector{i}),
                        pCreateInfos[i].pMultisampleState->sampleShadingEnable);

                    skip |= validate_array(
                        "vkCreateGraphicsPipelines",
                        ParameterName("pCreateInfos[%i].pMultisampleState->rasterizationSamples", ParameterName::IndexVector{i}),
                        ParameterName("pCreateInfos[%i].pMultisampleState->pSampleMask", ParameterName::IndexVector{i}),
                        pCreateInfos[i].pMultisampleState->rasterizationSamples, &pCreateInfos[i].pMultisampleState->pSampleMask,
                        true, false, kVUIDUndefined, kVUIDUndefined);

                    skip |= validate_flags(
                        "vkCreateGraphicsPipelines",
                        ParameterName("pCreateInfos[%i].pMultisampleState->rasterizationSamples", ParameterName::IndexVector{i}),
                        "VkSampleCountFlagBits", AllVkSampleCountFlagBits, pCreateInfos[i].pMultisampleState->rasterizationSamples,
                        kRequiredSingleBit, "VUID-VkPipelineMultisampleStateCreateInfo-rasterizationSamples-parameter");

                    skip |= validate_bool32(
                        "vkCreateGraphicsPipelines",
                        ParameterName("pCreateInfos[%i].pMultisampleState->alphaToCoverageEnable", ParameterName::IndexVector{i}),
                        pCreateInfos[i].pMultisampleState->alphaToCoverageEnable);

                    skip |= validate_bool32(
                        "vkCreateGraphicsPipelines",
                        ParameterName("pCreateInfos[%i].pMultisampleState->alphaToOneEnable", ParameterName::IndexVector{i}),
                        pCreateInfos[i].pMultisampleState->alphaToOneEnable);

                    if (pCreateInfos[i].pMultisampleState->sType != VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO) {
                        skip |= LogError(device, "VUID-VkPipelineMultisampleStateCreateInfo-sType-sType",
                                         "vkCreateGraphicsPipelines: parameter pCreateInfos[%d].pMultisampleState->sType must be "
                                         "VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO",
                                         i);
                    }
                    if (pCreateInfos[i].pMultisampleState->sampleShadingEnable == VK_TRUE) {
                        if (!physical_device_features.sampleRateShading) {
                            skip |= LogError(device, "VUID-VkPipelineMultisampleStateCreateInfo-sampleShadingEnable-00784",
                                             "vkCreateGraphicsPipelines(): parameter "
                                             "pCreateInfos[%d].pMultisampleState->sampleShadingEnable.",
                                             i);
                        }
                        // TODO Add documentation issue about when minSampleShading must be in range and when it is ignored
                        // For now a "least noise" test *only* when sampleShadingEnable is VK_TRUE.
                        if (!in_inclusive_range(pCreateInfos[i].pMultisampleState->minSampleShading, 0.F, 1.0F)) {
                            skip |= LogError(
                                device,

                                "VUID-VkPipelineMultisampleStateCreateInfo-minSampleShading-00786",
                                "vkCreateGraphicsPipelines(): parameter pCreateInfos[%d].pMultisampleState->minSampleShading.", i);
                        }
                    }

                    const auto *line_state = lvl_find_in_chain<VkPipelineRasterizationLineStateCreateInfoEXT>(
                        pCreateInfos[i].pRasterizationState->pNext);

                    if (line_state) {
                        if ((line_state->lineRasterizationMode == VK_LINE_RASTERIZATION_MODE_BRESENHAM_EXT ||
                             line_state->lineRasterizationMode == VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH_EXT)) {
                            if (pCreateInfos[i].pMultisampleState->alphaToCoverageEnable) {
                                skip |=
                                    LogError(device, "VUID-VkGraphicsPipelineCreateInfo-lineRasterizationMode-02766",
                                             "vkCreateGraphicsPipelines(): Bresenham/Smooth line rasterization not supported with "
                                             "pCreateInfos[%d].pMultisampleState->alphaToCoverageEnable == VK_TRUE.",
                                             i);
                            }
                            if (pCreateInfos[i].pMultisampleState->alphaToOneEnable) {
                                skip |=
                                    LogError(device, "VUID-VkGraphicsPipelineCreateInfo-lineRasterizationMode-02766",
                                             "vkCreateGraphicsPipelines(): Bresenham/Smooth line rasterization not supported with "
                                             "pCreateInfos[%d].pMultisampleState->alphaToOneEnable == VK_TRUE.",
                                             i);
                            }
                            if (pCreateInfos[i].pMultisampleState->sampleShadingEnable) {
                                skip |=
                                    LogError(device, "VUID-VkGraphicsPipelineCreateInfo-lineRasterizationMode-02766",
                                             "vkCreateGraphicsPipelines(): Bresenham/Smooth line rasterization not supported with "
                                             "pCreateInfos[%d].pMultisampleState->sampleShadingEnable == VK_TRUE.",
                                             i);
                            }
                        }
                        if (line_state->stippledLineEnable && !has_dynamic_line_stipple) {
                            if (line_state->lineStippleFactor < 1 || line_state->lineStippleFactor > 256) {
                                skip |=
                                    LogError(device, "VUID-VkGraphicsPipelineCreateInfo-stippledLineEnable-02767",
                                             "vkCreateGraphicsPipelines(): pCreateInfos[%d] lineStippleFactor = %d must be in the "
                                             "range [1,256].",
                                             i, line_state->lineStippleFactor);
                            }
                        }
                        const auto *line_features =
                            lvl_find_in_chain<VkPhysicalDeviceLineRasterizationFeaturesEXT>(device_createinfo_pnext);
                        if (line_state->lineRasterizationMode == VK_LINE_RASTERIZATION_MODE_RECTANGULAR_EXT &&
                            (!line_features || !line_features->rectangularLines)) {
                            skip |=
                                LogError(device, "VUID-VkPipelineRasterizationLineStateCreateInfoEXT-lineRasterizationMode-02768",
                                         "vkCreateGraphicsPipelines(): pCreateInfos[%d] lineRasterizationMode = "
                                         "VK_LINE_RASTERIZATION_MODE_RECTANGULAR_EXT requires the rectangularLines feature.",
                                         i);
                        }
                        if (line_state->lineRasterizationMode == VK_LINE_RASTERIZATION_MODE_BRESENHAM_EXT &&
                            (!line_features || !line_features->bresenhamLines)) {
                            skip |=
                                LogError(device, "VUID-VkPipelineRasterizationLineStateCreateInfoEXT-lineRasterizationMode-02769",
                                         "vkCreateGraphicsPipelines(): pCreateInfos[%d] lineRasterizationMode = "
                                         "VK_LINE_RASTERIZATION_MODE_BRESENHAM_EXT requires the bresenhamLines feature.",
                                         i);
                        }
                        if (line_state->lineRasterizationMode == VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH_EXT &&
                            (!line_features || !line_features->smoothLines)) {
                            skip |=
                                LogError(device, "VUID-VkPipelineRasterizationLineStateCreateInfoEXT-lineRasterizationMode-02770",
                                         "vkCreateGraphicsPipelines(): pCreateInfos[%d] lineRasterizationMode = "
                                         "VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH_EXT requires the smoothLines feature.",
                                         i);
                        }
                        if (line_state->stippledLineEnable) {
                            if (line_state->lineRasterizationMode == VK_LINE_RASTERIZATION_MODE_RECTANGULAR_EXT &&
                                (!line_features || !line_features->stippledRectangularLines)) {
                                skip |=
                                    LogError(device, "VUID-VkPipelineRasterizationLineStateCreateInfoEXT-stippledLineEnable-02771",
                                             "vkCreateGraphicsPipelines(): pCreateInfos[%d] lineRasterizationMode = "
                                             "VK_LINE_RASTERIZATION_MODE_RECTANGULAR_EXT with stipple requires the "
                                             "stippledRectangularLines feature.",
                                             i);
                            }
                            if (line_state->lineRasterizationMode == VK_LINE_RASTERIZATION_MODE_BRESENHAM_EXT &&
                                (!line_features || !line_features->stippledBresenhamLines)) {
                                skip |=
                                    LogError(device, "VUID-VkPipelineRasterizationLineStateCreateInfoEXT-stippledLineEnable-02772",
                                             "vkCreateGraphicsPipelines(): pCreateInfos[%d] lineRasterizationMode = "
                                             "VK_LINE_RASTERIZATION_MODE_BRESENHAM_EXT with stipple requires the "
                                             "stippledBresenhamLines feature.",
                                             i);
                            }
                            if (line_state->lineRasterizationMode == VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH_EXT &&
                                (!line_features || !line_features->stippledSmoothLines)) {
                                skip |=
                                    LogError(device, "VUID-VkPipelineRasterizationLineStateCreateInfoEXT-stippledLineEnable-02773",
                                             "vkCreateGraphicsPipelines(): pCreateInfos[%d] lineRasterizationMode = "
                                             "VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH_EXT with stipple requires the "
                                             "stippledSmoothLines feature.",
                                             i);
                            }
                            if (line_state->lineRasterizationMode == VK_LINE_RASTERIZATION_MODE_DEFAULT_EXT &&
                                (!line_features || !line_features->stippledSmoothLines || !device_limits.strictLines)) {
                                skip |=
                                    LogError(device, "VUID-VkPipelineRasterizationLineStateCreateInfoEXT-stippledLineEnable-02774",
                                             "vkCreateGraphicsPipelines(): pCreateInfos[%d] lineRasterizationMode = "
                                             "VK_LINE_RASTERIZATION_MODE_DEFAULT_EXT with stipple requires the "
                                             "stippledRectangularLines and strictLines features.",
                                             i);
                            }
                        }
                    }
                }

                bool uses_color_attachment = false;
                bool uses_depthstencil_attachment = false;
                {
                    std::unique_lock<std::mutex> lock(renderpass_map_mutex);
                    const auto subpasses_uses_it = renderpasses_states.find(pCreateInfos[i].renderPass);
                    if (subpasses_uses_it != renderpasses_states.end()) {
                        const auto &subpasses_uses = subpasses_uses_it->second;
                        if (subpasses_uses.subpasses_using_color_attachment.count(pCreateInfos[i].subpass))
                            uses_color_attachment = true;
                        if (subpasses_uses.subpasses_using_depthstencil_attachment.count(pCreateInfos[i].subpass))
                            uses_depthstencil_attachment = true;
                    }
                    lock.unlock();
                }

                if (pCreateInfos[i].pDepthStencilState != nullptr && uses_depthstencil_attachment) {
                    skip |= validate_struct_pnext(
                        "vkCreateGraphicsPipelines",
                        ParameterName("pCreateInfos[%i].pDepthStencilState->pNext", ParameterName::IndexVector{i}), NULL,
                        pCreateInfos[i].pDepthStencilState->pNext, 0, NULL, GeneratedVulkanHeaderVersion,
                        "VUID-VkPipelineDepthStencilStateCreateInfo-pNext-pNext", nullptr);

                    skip |= validate_reserved_flags(
                        "vkCreateGraphicsPipelines",
                        ParameterName("pCreateInfos[%i].pDepthStencilState->flags", ParameterName::IndexVector{i}),
                        pCreateInfos[i].pDepthStencilState->flags, "VUID-VkPipelineDepthStencilStateCreateInfo-flags-zerobitmask");

                    skip |= validate_bool32(
                        "vkCreateGraphicsPipelines",
                        ParameterName("pCreateInfos[%i].pDepthStencilState->depthTestEnable", ParameterName::IndexVector{i}),
                        pCreateInfos[i].pDepthStencilState->depthTestEnable);

                    skip |= validate_bool32(
                        "vkCreateGraphicsPipelines",
                        ParameterName("pCreateInfos[%i].pDepthStencilState->depthWriteEnable", ParameterName::IndexVector{i}),
                        pCreateInfos[i].pDepthStencilState->depthWriteEnable);

                    skip |= validate_ranged_enum(
                        "vkCreateGraphicsPipelines",
                        ParameterName("pCreateInfos[%i].pDepthStencilState->depthCompareOp", ParameterName::IndexVector{i}),
                        "VkCompareOp", AllVkCompareOpEnums, pCreateInfos[i].pDepthStencilState->depthCompareOp,
                        "VUID-VkPipelineDepthStencilStateCreateInfo-depthCompareOp-parameter");

                    skip |= validate_bool32(
                        "vkCreateGraphicsPipelines",
                        ParameterName("pCreateInfos[%i].pDepthStencilState->depthBoundsTestEnable", ParameterName::IndexVector{i}),
                        pCreateInfos[i].pDepthStencilState->depthBoundsTestEnable);

                    skip |= validate_bool32(
                        "vkCreateGraphicsPipelines",
                        ParameterName("pCreateInfos[%i].pDepthStencilState->stencilTestEnable", ParameterName::IndexVector{i}),
                        pCreateInfos[i].pDepthStencilState->stencilTestEnable);

                    skip |= validate_ranged_enum(
                        "vkCreateGraphicsPipelines",
                        ParameterName("pCreateInfos[%i].pDepthStencilState->front.failOp", ParameterName::IndexVector{i}),
                        "VkStencilOp", AllVkStencilOpEnums, pCreateInfos[i].pDepthStencilState->front.failOp,
                        "VUID-VkStencilOpState-failOp-parameter");

                    skip |= validate_ranged_enum(
                        "vkCreateGraphicsPipelines",
                        ParameterName("pCreateInfos[%i].pDepthStencilState->front.passOp", ParameterName::IndexVector{i}),
                        "VkStencilOp", AllVkStencilOpEnums, pCreateInfos[i].pDepthStencilState->front.passOp,
                        "VUID-VkStencilOpState-passOp-parameter");

                    skip |= validate_ranged_enum(
                        "vkCreateGraphicsPipelines",
                        ParameterName("pCreateInfos[%i].pDepthStencilState->front.depthFailOp", ParameterName::IndexVector{i}),
                        "VkStencilOp", AllVkStencilOpEnums, pCreateInfos[i].pDepthStencilState->front.depthFailOp,
                        "VUID-VkStencilOpState-depthFailOp-parameter");

                    skip |= validate_ranged_enum(
                        "vkCreateGraphicsPipelines",
                        ParameterName("pCreateInfos[%i].pDepthStencilState->front.compareOp", ParameterName::IndexVector{i}),
                        "VkCompareOp", AllVkCompareOpEnums, pCreateInfos[i].pDepthStencilState->front.compareOp,
                        "VUID-VkPipelineDepthStencilStateCreateInfo-depthCompareOp-parameter");

                    skip |= validate_ranged_enum(
                        "vkCreateGraphicsPipelines",
                        ParameterName("pCreateInfos[%i].pDepthStencilState->back.failOp", ParameterName::IndexVector{i}),
                        "VkStencilOp", AllVkStencilOpEnums, pCreateInfos[i].pDepthStencilState->back.failOp,
                        "VUID-VkStencilOpState-failOp-parameter");

                    skip |= validate_ranged_enum(
                        "vkCreateGraphicsPipelines",
                        ParameterName("pCreateInfos[%i].pDepthStencilState->back.passOp", ParameterName::IndexVector{i}),
                        "VkStencilOp", AllVkStencilOpEnums, pCreateInfos[i].pDepthStencilState->back.passOp,
                        "VUID-VkStencilOpState-passOp-parameter");

                    skip |= validate_ranged_enum(
                        "vkCreateGraphicsPipelines",
                        ParameterName("pCreateInfos[%i].pDepthStencilState->back.depthFailOp", ParameterName::IndexVector{i}),
                        "VkStencilOp", AllVkStencilOpEnums, pCreateInfos[i].pDepthStencilState->back.depthFailOp,
                        "VUID-VkStencilOpState-depthFailOp-parameter");

                    skip |= validate_ranged_enum(
                        "vkCreateGraphicsPipelines",
                        ParameterName("pCreateInfos[%i].pDepthStencilState->back.compareOp", ParameterName::IndexVector{i}),
                        "VkCompareOp", AllVkCompareOpEnums, pCreateInfos[i].pDepthStencilState->back.compareOp,
                        "VUID-VkPipelineDepthStencilStateCreateInfo-depthCompareOp-parameter");

                    if (pCreateInfos[i].pDepthStencilState->sType != VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO) {
                        skip |= LogError(device, "VUID-VkPipelineDepthStencilStateCreateInfo-sType-sType",
                                         "vkCreateGraphicsPipelines: parameter pCreateInfos[%d].pDepthStencilState->sType must be "
                                         "VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO",
                                         i);
                    }
                }

                const VkStructureType allowed_structs_VkPipelineColorBlendStateCreateInfo[] = {
                    VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_ADVANCED_STATE_CREATE_INFO_EXT};

                if (pCreateInfos[i].pColorBlendState != nullptr && uses_color_attachment) {
                    skip |= validate_struct_type("vkCreateGraphicsPipelines",
                                                 ParameterName("pCreateInfos[%i].pColorBlendState", ParameterName::IndexVector{i}),
                                                 "VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO",
                                                 pCreateInfos[i].pColorBlendState,
                                                 VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO, false, kVUIDUndefined,
                                                 "VUID-VkPipelineColorBlendStateCreateInfo-sType-sType");

                    skip |= validate_struct_pnext(
                        "vkCreateGraphicsPipelines",
                        ParameterName("pCreateInfos[%i].pColorBlendState->pNext", ParameterName::IndexVector{i}),
                        "VkPipelineColorBlendAdvancedStateCreateInfoEXT", pCreateInfos[i].pColorBlendState->pNext,
                        ARRAY_SIZE(allowed_structs_VkPipelineColorBlendStateCreateInfo),
                        allowed_structs_VkPipelineColorBlendStateCreateInfo, GeneratedVulkanHeaderVersion,
                        "VUID-VkPipelineColorBlendStateCreateInfo-pNext-pNext",
                        "VUID-VkPipelineColorBlendStateCreateInfo-sType-unique");

                    skip |= validate_reserved_flags(
                        "vkCreateGraphicsPipelines",
                        ParameterName("pCreateInfos[%i].pColorBlendState->flags", ParameterName::IndexVector{i}),
                        pCreateInfos[i].pColorBlendState->flags, "VUID-VkPipelineColorBlendStateCreateInfo-flags-zerobitmask");

                    skip |= validate_bool32(
                        "vkCreateGraphicsPipelines",
                        ParameterName("pCreateInfos[%i].pColorBlendState->logicOpEnable", ParameterName::IndexVector{i}),
                        pCreateInfos[i].pColorBlendState->logicOpEnable);

                    skip |= validate_array(
                        "vkCreateGraphicsPipelines",
                        ParameterName("pCreateInfos[%i].pColorBlendState->attachmentCount", ParameterName::IndexVector{i}),
                        ParameterName("pCreateInfos[%i].pColorBlendState->pAttachments", ParameterName::IndexVector{i}),
                        pCreateInfos[i].pColorBlendState->attachmentCount, &pCreateInfos[i].pColorBlendState->pAttachments, false,
                        true, kVUIDUndefined, kVUIDUndefined);

                    if (pCreateInfos[i].pColorBlendState->pAttachments != NULL) {
                        for (uint32_t attachmentIndex = 0; attachmentIndex < pCreateInfos[i].pColorBlendState->attachmentCount;
                             ++attachmentIndex) {
                            skip |= validate_bool32("vkCreateGraphicsPipelines",
                                                    ParameterName("pCreateInfos[%i].pColorBlendState->pAttachments[%i].blendEnable",
                                                                  ParameterName::IndexVector{i, attachmentIndex}),
                                                    pCreateInfos[i].pColorBlendState->pAttachments[attachmentIndex].blendEnable);

                            skip |= validate_ranged_enum(
                                "vkCreateGraphicsPipelines",
                                ParameterName("pCreateInfos[%i].pColorBlendState->pAttachments[%i].srcColorBlendFactor",
                                              ParameterName::IndexVector{i, attachmentIndex}),
                                "VkBlendFactor", AllVkBlendFactorEnums,
                                pCreateInfos[i].pColorBlendState->pAttachments[attachmentIndex].srcColorBlendFactor,
                                "VUID-VkPipelineColorBlendAttachmentState-srcColorBlendFactor-parameter");

                            skip |= validate_ranged_enum(
                                "vkCreateGraphicsPipelines",
                                ParameterName("pCreateInfos[%i].pColorBlendState->pAttachments[%i].dstColorBlendFactor",
                                              ParameterName::IndexVector{i, attachmentIndex}),
                                "VkBlendFactor", AllVkBlendFactorEnums,
                                pCreateInfos[i].pColorBlendState->pAttachments[attachmentIndex].dstColorBlendFactor,
                                "VUID-VkPipelineColorBlendAttachmentState-dstColorBlendFactor-parameter");

                            skip |= validate_ranged_enum(
                                "vkCreateGraphicsPipelines",
                                ParameterName("pCreateInfos[%i].pColorBlendState->pAttachments[%i].colorBlendOp",
                                              ParameterName::IndexVector{i, attachmentIndex}),
                                "VkBlendOp", AllVkBlendOpEnums,
                                pCreateInfos[i].pColorBlendState->pAttachments[attachmentIndex].colorBlendOp,
                                "VUID-VkPipelineColorBlendAttachmentState-colorBlendOp-parameter");

                            skip |= validate_ranged_enum(
                                "vkCreateGraphicsPipelines",
                                ParameterName("pCreateInfos[%i].pColorBlendState->pAttachments[%i].srcAlphaBlendFactor",
                                              ParameterName::IndexVector{i, attachmentIndex}),
                                "VkBlendFactor", AllVkBlendFactorEnums,
                                pCreateInfos[i].pColorBlendState->pAttachments[attachmentIndex].srcAlphaBlendFactor,
                                "VUID-VkPipelineColorBlendAttachmentState-srcAlphaBlendFactor-parameter");

                            skip |= validate_ranged_enum(
                                "vkCreateGraphicsPipelines",
                                ParameterName("pCreateInfos[%i].pColorBlendState->pAttachments[%i].dstAlphaBlendFactor",
                                              ParameterName::IndexVector{i, attachmentIndex}),
                                "VkBlendFactor", AllVkBlendFactorEnums,
                                pCreateInfos[i].pColorBlendState->pAttachments[attachmentIndex].dstAlphaBlendFactor,
                                "VUID-VkPipelineColorBlendAttachmentState-dstAlphaBlendFactor-parameter");

                            skip |= validate_ranged_enum(
                                "vkCreateGraphicsPipelines",
                                ParameterName("pCreateInfos[%i].pColorBlendState->pAttachments[%i].alphaBlendOp",
                                              ParameterName::IndexVector{i, attachmentIndex}),
                                "VkBlendOp", AllVkBlendOpEnums,
                                pCreateInfos[i].pColorBlendState->pAttachments[attachmentIndex].alphaBlendOp,
                                "VUID-VkPipelineColorBlendAttachmentState-alphaBlendOp-parameter");

                            skip |=
                                validate_flags("vkCreateGraphicsPipelines",
                                               ParameterName("pCreateInfos[%i].pColorBlendState->pAttachments[%i].colorWriteMask",
                                                             ParameterName::IndexVector{i, attachmentIndex}),
                                               "VkColorComponentFlagBits", AllVkColorComponentFlagBits,
                                               pCreateInfos[i].pColorBlendState->pAttachments[attachmentIndex].colorWriteMask,
                                               kOptionalFlags, "VUID-VkPipelineColorBlendAttachmentState-colorWriteMask-parameter");
                        }
                    }

                    if (pCreateInfos[i].pColorBlendState->sType != VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO) {
                        skip |= LogError(device, "VUID-VkPipelineColorBlendStateCreateInfo-sType-sType",
                                         "vkCreateGraphicsPipelines: parameter pCreateInfos[%d].pColorBlendState->sType must be "
                                         "VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO",
                                         i);
                    }

                    // If logicOpEnable is VK_TRUE, logicOp must be a valid VkLogicOp value
                    if (pCreateInfos[i].pColorBlendState->logicOpEnable == VK_TRUE) {
                        skip |= validate_ranged_enum(
                            "vkCreateGraphicsPipelines",
                            ParameterName("pCreateInfos[%i].pColorBlendState->logicOp", ParameterName::IndexVector{i}), "VkLogicOp",
                            AllVkLogicOpEnums, pCreateInfos[i].pColorBlendState->logicOp,
                            "VUID-VkPipelineColorBlendStateCreateInfo-logicOpEnable-00607");
                    }
                }
            }

            if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_DERIVATIVE_BIT) {
                if (pCreateInfos[i].basePipelineIndex != -1) {
                    if (pCreateInfos[i].basePipelineHandle != VK_NULL_HANDLE) {
                        skip |=
                            LogError(device, "VUID-VkGraphicsPipelineCreateInfo-flags-00724",
                                     "vkCreateGraphicsPipelines parameter, pCreateInfos[%u]->basePipelineHandle, must be "
                                     "VK_NULL_HANDLE if pCreateInfos->flags contains the VK_PIPELINE_CREATE_DERIVATIVE_BIT flag "
                                     "and pCreateInfos->basePipelineIndex is not -1.",
                                     i);
                    }
                }

                if (pCreateInfos[i].basePipelineHandle != VK_NULL_HANDLE) {
                    if (pCreateInfos[i].basePipelineIndex != -1) {
                        skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-flags-00725",
                                         "vkCreateGraphicsPipelines parameter, pCreateInfos[%u]->basePipelineIndex, must be -1 if "
                                         "pCreateInfos->flags contains the VK_PIPELINE_CREATE_DERIVATIVE_BIT flag and "
                                         "pCreateInfos->basePipelineHandle is not VK_NULL_HANDLE.",
                                         i);
                    }
                } else {
                    if (static_cast<uint32_t>(pCreateInfos[i].basePipelineIndex) >= createInfoCount) {
                        skip |=
                            LogError(device, "VUID-VkGraphicsPipelineCreateInfo-flags-00723",
                                     "vkCreateGraphicsPipelines parameter pCreateInfos[%u]->basePipelineIndex (%d) must be a valid"
                                     "index into the pCreateInfos array, of size %d.",
                                     i, pCreateInfos[i].basePipelineIndex, createInfoCount);
                    }
                }
            }

            if ((pCreateInfos[i].flags & VK_PIPELINE_CREATE_DISPATCH_BASE) != 0) {
                skip |= LogError(
                    device, "VUID-VkGraphicsPipelineCreateInfo-flags-00764",
                    "vkCreateGraphicsPipelines parameter pCreateInfos[%u]->flags must not contain VK_PIPELINE_CREATE_DISPATCH_BASE",
                    i);
            }

            if (pCreateInfos[i].pRasterizationState) {
                if (!device_extensions.vk_nv_fill_rectangle) {
                    if (pCreateInfos[i].pRasterizationState->polygonMode == VK_POLYGON_MODE_FILL_RECTANGLE_NV) {
                        skip |=
                            LogError(device, "VUID-VkPipelineRasterizationStateCreateInfo-polygonMode-01414",
                                     "vkCreateGraphicsPipelines parameter, VkPolygonMode "
                                     "pCreateInfos->pRasterizationState->polygonMode cannot be VK_POLYGON_MODE_FILL_RECTANGLE_NV "
                                     "if the extension VK_NV_fill_rectangle is not enabled.");
                    } else if ((pCreateInfos[i].pRasterizationState->polygonMode != VK_POLYGON_MODE_FILL) &&
                               (physical_device_features.fillModeNonSolid == false)) {
                        skip |= LogError(device, "VUID-VkPipelineRasterizationStateCreateInfo-polygonMode-01413",
                                         "vkCreateGraphicsPipelines parameter, VkPolygonMode "
                                         "pCreateInfos[%u]->pRasterizationState->polygonMode cannot be VK_POLYGON_MODE_POINT or "
                                         "VK_POLYGON_MODE_LINE if VkPhysicalDeviceFeatures->fillModeNonSolid is false.",
                                         i);
                    }
                } else {
                    if ((pCreateInfos[i].pRasterizationState->polygonMode != VK_POLYGON_MODE_FILL) &&
                        (pCreateInfos[i].pRasterizationState->polygonMode != VK_POLYGON_MODE_FILL_RECTANGLE_NV) &&
                        (physical_device_features.fillModeNonSolid == false)) {
                        skip |=
                            LogError(device, "VUID-VkPipelineRasterizationStateCreateInfo-polygonMode-01507",
                                     "vkCreateGraphicsPipelines parameter, VkPolygonMode "
                                     "pCreateInfos[%u]->pRasterizationState->polygonMode must be VK_POLYGON_MODE_FILL or "
                                     "VK_POLYGON_MODE_FILL_RECTANGLE_NV if VkPhysicalDeviceFeatures->fillModeNonSolid is false.",
                                     i);
                    }
                }

                if (!has_dynamic_line_width && !physical_device_features.wideLines &&
                    (pCreateInfos[i].pRasterizationState->lineWidth != 1.0f)) {
                    skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-00749",
                                     "The line width state is static (pCreateInfos[%" PRIu32
                                     "].pDynamicState->pDynamicStates does not contain VK_DYNAMIC_STATE_LINE_WIDTH) and "
                                     "VkPhysicalDeviceFeatures::wideLines is disabled, but pCreateInfos[%" PRIu32
                                     "].pRasterizationState->lineWidth (=%f) is not 1.0.",
                                     i, i, pCreateInfos[i].pRasterizationState->lineWidth);
                }
            }
        }
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache,
                                                                       uint32_t createInfoCount,
                                                                       const VkComputePipelineCreateInfo *pCreateInfos,
                                                                       const VkAllocationCallbacks *pAllocator,
                                                                       VkPipeline *pPipelines) const {
    bool skip = false;
    for (uint32_t i = 0; i < createInfoCount; i++) {
        skip |= validate_string("vkCreateComputePipelines",
                                ParameterName("pCreateInfos[%i].stage.pName", ParameterName::IndexVector{i}),
                                "VUID-VkPipelineShaderStageCreateInfo-pName-parameter", pCreateInfos[i].stage.pName);
        auto feedback_struct = lvl_find_in_chain<VkPipelineCreationFeedbackCreateInfoEXT>(pCreateInfos[i].pNext);
        if ((feedback_struct != nullptr) && (feedback_struct->pipelineStageCreationFeedbackCount != 1)) {
            skip |=
                LogError(device, "VUID-VkPipelineCreationFeedbackCreateInfoEXT-pipelineStageCreationFeedbackCount-02669",
                         "vkCreateComputePipelines(): in pCreateInfo[%" PRIu32
                         "], VkPipelineCreationFeedbackEXT::pipelineStageCreationFeedbackCount must equal 1, found %" PRIu32 ".",
                         i, feedback_struct->pipelineStageCreationFeedbackCount);
        }

        // Make sure compute stage is selected
        if (pCreateInfos[i].stage.stage != VK_SHADER_STAGE_COMPUTE_BIT) {
            skip |= LogError(device, "VUID-VkComputePipelineCreateInfo-stage-00701",
                             "vkCreateComputePipelines(): the pCreateInfo[%u].stage.stage (%s) is not VK_SHADER_STAGE_COMPUTE_BIT",
                             i, string_VkShaderStageFlagBits(pCreateInfos[i].stage.stage));
        }
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCreateSampler(VkDevice device, const VkSamplerCreateInfo *pCreateInfo,
                                                              const VkAllocationCallbacks *pAllocator, VkSampler *pSampler) const {
    bool skip = false;

    if (pCreateInfo != nullptr) {
        const auto &features = physical_device_features;
        const auto &limits = device_limits;

        if (pCreateInfo->anisotropyEnable == VK_TRUE) {
            if (!in_inclusive_range(pCreateInfo->maxAnisotropy, 1.0F, limits.maxSamplerAnisotropy)) {
                skip |= LogError(device, "VUID-VkSamplerCreateInfo-anisotropyEnable-01071",
                                 "vkCreateSampler(): value of %s must be in range [1.0, %f] %s, but %f found.",
                                 "pCreateInfo->maxAnisotropy", limits.maxSamplerAnisotropy,
                                 "VkPhysicalDeviceLimits::maxSamplerAnistropy", pCreateInfo->maxAnisotropy);
            }

            // Anistropy cannot be enabled in sampler unless enabled as a feature
            if (features.samplerAnisotropy == VK_FALSE) {
                skip |= LogError(device, "VUID-VkSamplerCreateInfo-anisotropyEnable-01070",
                                 "vkCreateSampler(): Anisotropic sampling feature is not enabled, %s must be VK_FALSE.",
                                 "pCreateInfo->anisotropyEnable");
            }
        }

        if (pCreateInfo->unnormalizedCoordinates == VK_TRUE) {
            if (pCreateInfo->minFilter != pCreateInfo->magFilter) {
                skip |= LogError(device, "VUID-VkSamplerCreateInfo-unnormalizedCoordinates-01072",
                                 "vkCreateSampler(): when pCreateInfo->unnormalizedCoordinates is VK_TRUE, "
                                 "pCreateInfo->minFilter (%s) and pCreateInfo->magFilter (%s) must be equal.",
                                 string_VkFilter(pCreateInfo->minFilter), string_VkFilter(pCreateInfo->magFilter));
            }
            if (pCreateInfo->mipmapMode != VK_SAMPLER_MIPMAP_MODE_NEAREST) {
                skip |= LogError(device, "VUID-VkSamplerCreateInfo-unnormalizedCoordinates-01073",
                                 "vkCreateSampler(): when pCreateInfo->unnormalizedCoordinates is VK_TRUE, "
                                 "pCreateInfo->mipmapMode (%s) must be VK_SAMPLER_MIPMAP_MODE_NEAREST.",
                                 string_VkSamplerMipmapMode(pCreateInfo->mipmapMode));
            }
            if (pCreateInfo->minLod != 0.0f || pCreateInfo->maxLod != 0.0f) {
                skip |= LogError(device, "VUID-VkSamplerCreateInfo-unnormalizedCoordinates-01074",
                                 "vkCreateSampler(): when pCreateInfo->unnormalizedCoordinates is VK_TRUE, "
                                 "pCreateInfo->minLod (%f) and pCreateInfo->maxLod (%f) must both be zero.",
                                 pCreateInfo->minLod, pCreateInfo->maxLod);
            }
            if ((pCreateInfo->addressModeU != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE &&
                 pCreateInfo->addressModeU != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER) ||
                (pCreateInfo->addressModeV != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE &&
                 pCreateInfo->addressModeV != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER)) {
                skip |= LogError(device, "VUID-VkSamplerCreateInfo-unnormalizedCoordinates-01075",
                                 "vkCreateSampler(): when pCreateInfo->unnormalizedCoordinates is VK_TRUE, "
                                 "pCreateInfo->addressModeU (%s) and pCreateInfo->addressModeV (%s) must both be "
                                 "VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE or VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER.",
                                 string_VkSamplerAddressMode(pCreateInfo->addressModeU),
                                 string_VkSamplerAddressMode(pCreateInfo->addressModeV));
            }
            if (pCreateInfo->anisotropyEnable == VK_TRUE) {
                skip |= LogError(device, "VUID-VkSamplerCreateInfo-unnormalizedCoordinates-01076",
                                 "vkCreateSampler(): pCreateInfo->anisotropyEnable and pCreateInfo->unnormalizedCoordinates must "
                                 "not both be VK_TRUE.");
            }
            if (pCreateInfo->compareEnable == VK_TRUE) {
                skip |= LogError(device, "VUID-VkSamplerCreateInfo-unnormalizedCoordinates-01077",
                                 "vkCreateSampler(): pCreateInfo->compareEnable and pCreateInfo->unnormalizedCoordinates must "
                                 "not both be VK_TRUE.");
            }
        }

        // If compareEnable is VK_TRUE, compareOp must be a valid VkCompareOp value
        if (pCreateInfo->compareEnable == VK_TRUE) {
            skip |= validate_ranged_enum("vkCreateSampler", "pCreateInfo->compareOp", "VkCompareOp", AllVkCompareOpEnums,
                                         pCreateInfo->compareOp, "VUID-VkSamplerCreateInfo-compareEnable-01080");
            const auto *sampler_reduction = lvl_find_in_chain<VkSamplerReductionModeCreateInfo>(pCreateInfo->pNext);
            if (sampler_reduction != nullptr) {
                if (sampler_reduction->reductionMode != VK_SAMPLER_REDUCTION_MODE_WEIGHTED_AVERAGE) {
                    skip |= LogError(
                        device, "VUID-VkSamplerCreateInfo-compareEnable-01423",
                        "copmareEnable is true so the sampler reduction mode must be VK_SAMPLER_REDUCTION_MODE_WEIGHTED_AVERAGE.");
                }
            }
        }

        // If any of addressModeU, addressModeV or addressModeW are VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, borderColor must be a
        // valid VkBorderColor value
        if ((pCreateInfo->addressModeU == VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER) ||
            (pCreateInfo->addressModeV == VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER) ||
            (pCreateInfo->addressModeW == VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER)) {
            skip |= validate_ranged_enum("vkCreateSampler", "pCreateInfo->borderColor", "VkBorderColor", AllVkBorderColorEnums,
                                         pCreateInfo->borderColor, "VUID-VkSamplerCreateInfo-addressModeU-01078");
        }

        // If any of addressModeU, addressModeV or addressModeW are VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE, the
        // VK_KHR_sampler_mirror_clamp_to_edge extension must be enabled
        if (!device_extensions.vk_khr_sampler_mirror_clamp_to_edge &&
            ((pCreateInfo->addressModeU == VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE) ||
             (pCreateInfo->addressModeV == VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE) ||
             (pCreateInfo->addressModeW == VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE))) {
            skip |=
                LogError(device, "VUID-VkSamplerCreateInfo-addressModeU-01079",
                         "vkCreateSampler(): A VkSamplerAddressMode value is set to VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE "
                         "but the VK_KHR_sampler_mirror_clamp_to_edge extension has not been enabled.");
        }

        // Checks for the IMG cubic filtering extension
        if (device_extensions.vk_img_filter_cubic) {
            if ((pCreateInfo->anisotropyEnable == VK_TRUE) &&
                ((pCreateInfo->minFilter == VK_FILTER_CUBIC_IMG) || (pCreateInfo->magFilter == VK_FILTER_CUBIC_IMG))) {
                skip |= LogError(device, "VUID-VkSamplerCreateInfo-magFilter-01081",
                                 "vkCreateSampler(): Anisotropic sampling must not be VK_TRUE when either minFilter or magFilter "
                                 "are VK_FILTER_CUBIC_IMG.");
            }
        }

        // Check for valid Lod range
        if (pCreateInfo->minLod > pCreateInfo->maxLod) {
            skip |=
                LogError(device, "VUID-VkSamplerCreateInfo-maxLod-01973",
                         "vkCreateSampler(): minLod (%f) is greater than maxLod (%f)", pCreateInfo->minLod, pCreateInfo->maxLod);
        }

        // Check mipLodBias to device limit
        if (pCreateInfo->mipLodBias > limits.maxSamplerLodBias) {
            skip |= LogError(device, "VUID-VkSamplerCreateInfo-mipLodBias-01069",
                             "vkCreateSampler(): mipLodBias (%f) is greater than VkPhysicalDeviceLimits::maxSamplerLodBias (%f)",
                             pCreateInfo->mipLodBias, limits.maxSamplerLodBias);
        }

        const auto *sampler_conversion = lvl_find_in_chain<VkSamplerYcbcrConversionInfo>(pCreateInfo->pNext);
        if (sampler_conversion != nullptr) {
            if ((pCreateInfo->addressModeU != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE) ||
                (pCreateInfo->addressModeV != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE) ||
                (pCreateInfo->addressModeW != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE) ||
                (pCreateInfo->anisotropyEnable != VK_FALSE) || (pCreateInfo->unnormalizedCoordinates != VK_FALSE)) {
                skip |= LogError(
                    device, "VUID-VkSamplerCreateInfo-addressModeU-01646",
                    "vkCreateSampler():  SamplerYCbCrConversion is enabled: "
                    "addressModeU (%s), addressModeV (%s), addressModeW (%s) must be CLAMP_TO_EDGE, and anisotropyEnable (%s) "
                    "and unnormalizedCoordinates (%s) must be VK_FALSE.",
                    string_VkSamplerAddressMode(pCreateInfo->addressModeU), string_VkSamplerAddressMode(pCreateInfo->addressModeV),
                    string_VkSamplerAddressMode(pCreateInfo->addressModeW), pCreateInfo->anisotropyEnable ? "VK_TRUE" : "VK_FALSE",
                    pCreateInfo->unnormalizedCoordinates ? "VK_TRUE" : "VK_FALSE");
            }
        }

        if (pCreateInfo->flags & VK_SAMPLER_CREATE_SUBSAMPLED_BIT_EXT) {
            if (pCreateInfo->minFilter != pCreateInfo->magFilter) {
                skip |= LogError(device, "VUID-VkSamplerCreateInfo-flags-02574",
                                 "vkCreateSampler(): when flags includes VK_SAMPLER_CREATE_SUBSAMPLED_BIT_EXT, "
                                 "pCreateInfo->minFilter (%s) and pCreateInfo->magFilter (%s) must be equal.",
                                 string_VkFilter(pCreateInfo->minFilter), string_VkFilter(pCreateInfo->magFilter));
            }
            if (pCreateInfo->mipmapMode != VK_SAMPLER_MIPMAP_MODE_NEAREST) {
                skip |= LogError(device, "VUID-VkSamplerCreateInfo-flags-02575",
                                 "vkCreateSampler(): when flags includes VK_SAMPLER_CREATE_SUBSAMPLED_BIT_EXT, "
                                 "pCreateInfo->mipmapMode (%s) must be VK_SAMPLER_MIPMAP_MODE_NEAREST.",
                                 string_VkSamplerMipmapMode(pCreateInfo->mipmapMode));
            }
            if (pCreateInfo->minLod != 0.0 || pCreateInfo->maxLod != 0.0) {
                skip |= LogError(device, "VUID-VkSamplerCreateInfo-flags-02576",
                                 "vkCreateSampler(): when flags includes VK_SAMPLER_CREATE_SUBSAMPLED_BIT_EXT, "
                                 "pCreateInfo->minLod (%f) and pCreateInfo->maxLod (%f) must be zero.",
                                 pCreateInfo->minLod, pCreateInfo->maxLod);
            }
            if (((pCreateInfo->addressModeU != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE) &&
                 (pCreateInfo->addressModeU != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER)) ||
                ((pCreateInfo->addressModeV != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE) &&
                 (pCreateInfo->addressModeV != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER))) {
                skip |= LogError(device, "VUID-VkSamplerCreateInfo-flags-02577",
                                 "vkCreateSampler(): when flags includes VK_SAMPLER_CREATE_SUBSAMPLED_BIT_EXT, "
                                 "pCreateInfo->addressModeU (%s) and pCreateInfo->addressModeV (%s) must be "
                                 "VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE or VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER",
                                 string_VkSamplerAddressMode(pCreateInfo->addressModeU),
                                 string_VkSamplerAddressMode(pCreateInfo->addressModeV));
            }
            if (pCreateInfo->anisotropyEnable) {
                skip |= LogError(device, "VUID-VkSamplerCreateInfo-flags-02578",
                                 "vkCreateSampler(): when flags includes VK_SAMPLER_CREATE_SUBSAMPLED_BIT_EXT, "
                                 "pCreateInfo->anisotropyEnable must be VK_FALSE");
            }
            if (pCreateInfo->compareEnable) {
                skip |= LogError(device, "VUID-VkSamplerCreateInfo-flags-02579",
                                 "vkCreateSampler(): when flags includes VK_SAMPLER_CREATE_SUBSAMPLED_BIT_EXT, "
                                 "pCreateInfo->compareEnable must be VK_FALSE");
            }
            if (pCreateInfo->unnormalizedCoordinates) {
                skip |= LogError(device, "VUID-VkSamplerCreateInfo-flags-02580",
                                 "vkCreateSampler(): when flags includes VK_SAMPLER_CREATE_SUBSAMPLED_BIT_EXT, "
                                 "pCreateInfo->unnormalizedCoordinates must be VK_FALSE");
            }
        }
    }

    if (pCreateInfo->borderColor == VK_BORDER_COLOR_INT_CUSTOM_EXT ||
        pCreateInfo->borderColor == VK_BORDER_COLOR_FLOAT_CUSTOM_EXT) {
        if (!device_extensions.vk_ext_custom_border_color) {
            skip |= LogError(device, kVUID_PVError_ExtensionNotEnabled,
                             "VkSamplerCreateInfo->borderColor is %s but %s is not enabled.\n",
                             string_VkBorderColor(pCreateInfo->borderColor), VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME);
        }
        auto custom_create_info = lvl_find_in_chain<VkSamplerCustomBorderColorCreateInfoEXT>(pCreateInfo->pNext);
        if (!custom_create_info) {
            skip |=
                LogError(device, "VUID-VkSamplerCreateInfo-borderColor-04011",
                         "VkSamplerCreateInfo->borderColor is set to %s but there is no VkSamplerCustomBorderColorCreateInfoEXT "
                         "struct in pNext chain.\n",
                         string_VkBorderColor(pCreateInfo->borderColor));
        } else {
            if ((custom_create_info->format != VK_FORMAT_UNDEFINED) &&
                ((pCreateInfo->borderColor == VK_BORDER_COLOR_INT_CUSTOM_EXT && !FormatIsSampledInt(custom_create_info->format)) ||
                 (pCreateInfo->borderColor == VK_BORDER_COLOR_FLOAT_CUSTOM_EXT &&
                  !FormatIsSampledFloat(custom_create_info->format)))) {
                skip |= LogError(device, "VUID-VkSamplerCustomBorderColorCreateInfoEXT-format-04013",
                                 "VkSamplerCreateInfo->borderColor is %s but VkSamplerCustomBorderColorCreateInfoEXT.format = %s "
                                 "whose type does not match\n",
                                 string_VkBorderColor(pCreateInfo->borderColor), string_VkFormat(custom_create_info->format));
                ;
            }
        }
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCreateDescriptorSetLayout(VkDevice device,
                                                                          const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
                                                                          const VkAllocationCallbacks *pAllocator,
                                                                          VkDescriptorSetLayout *pSetLayout) const {
    bool skip = false;

    // Validation for parameters excluded from the generated validation code due to a 'noautovalidity' tag in vk.xml
    if ((pCreateInfo != nullptr) && (pCreateInfo->pBindings != nullptr)) {
        for (uint32_t i = 0; i < pCreateInfo->bindingCount; ++i) {
            if (pCreateInfo->pBindings[i].descriptorCount != 0) {
                if (((pCreateInfo->pBindings[i].descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER) ||
                     (pCreateInfo->pBindings[i].descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)) &&
                    (pCreateInfo->pBindings[i].pImmutableSamplers != nullptr)) {
                    for (uint32_t descriptor_index = 0; descriptor_index < pCreateInfo->pBindings[i].descriptorCount;
                         ++descriptor_index) {
                        if (pCreateInfo->pBindings[i].pImmutableSamplers[descriptor_index] == VK_NULL_HANDLE) {
                            skip |= LogError(device, "VUID-VkDescriptorSetLayoutBinding-descriptorType-00282",
                                             "vkCreateDescriptorSetLayout: required parameter "
                                             "pCreateInfo->pBindings[%d].pImmutableSamplers[%d] specified as VK_NULL_HANDLE",
                                             i, descriptor_index);
                        }
                    }
                }

                // If descriptorCount is not 0, stageFlags must be a valid combination of VkShaderStageFlagBits values
                if ((pCreateInfo->pBindings[i].stageFlags != 0) &&
                    ((pCreateInfo->pBindings[i].stageFlags & (~AllVkShaderStageFlagBits)) != 0)) {
                    skip |= LogError(device, "VUID-VkDescriptorSetLayoutBinding-descriptorCount-00283",
                                     "vkCreateDescriptorSetLayout(): if pCreateInfo->pBindings[%d].descriptorCount is not 0, "
                                     "pCreateInfo->pBindings[%d].stageFlags must be a valid combination of VkShaderStageFlagBits "
                                     "values.",
                                     i, i);
                }

                if ((pCreateInfo->pBindings[i].descriptorType == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT) &&
                    (pCreateInfo->pBindings[i].stageFlags != 0) &&
                    (pCreateInfo->pBindings[i].stageFlags != VK_SHADER_STAGE_FRAGMENT_BIT)) {
                    skip |=
                        LogError(device, "VUID-VkDescriptorSetLayoutBinding-descriptorType-01510",
                                 "vkCreateDescriptorSetLayout(): if pCreateInfo->pBindings[%d].descriptorCount is not 0 and "
                                 "descriptorType is VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT then pCreateInfo->pBindings[%d].stageFlags "
                                 "must be 0 or VK_SHADER_STAGE_FRAGMENT_BIT but is currently %s",
                                 i, i, string_VkShaderStageFlags(pCreateInfo->pBindings[i].stageFlags).c_str());
                }
            }
        }
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool,
                                                                   uint32_t descriptorSetCount,
                                                                   const VkDescriptorSet *pDescriptorSets) const {
    // Validation for parameters excluded from the generated validation code due to a 'noautovalidity' tag in vk.xml
    // This is an array of handles, where the elements are allowed to be VK_NULL_HANDLE, and does not require any validation beyond
    // validate_array()
    return validate_array("vkFreeDescriptorSets", "descriptorSetCount", "pDescriptorSets", descriptorSetCount, &pDescriptorSets,
                          true, true, kVUIDUndefined, kVUIDUndefined);
}

bool StatelessValidation::validate_WriteDescriptorSet(const char *vkCallingFunction, const uint32_t descriptorWriteCount,
                                                      const VkWriteDescriptorSet *pDescriptorWrites,
                                                      const bool validateDstSet) const {
    bool skip = false;

    if (pDescriptorWrites != NULL) {
        for (uint32_t i = 0; i < descriptorWriteCount; ++i) {
            // descriptorCount must be greater than 0
            if (pDescriptorWrites[i].descriptorCount == 0) {
                skip |=
                    LogError(device, "VUID-VkWriteDescriptorSet-descriptorCount-arraylength",
                             "%s(): parameter pDescriptorWrites[%d].descriptorCount must be greater than 0.", vkCallingFunction, i);
            }

            // If called from vkCmdPushDescriptorSetKHR, the dstSet member is ignored.
            if (validateDstSet) {
                // dstSet must be a valid VkDescriptorSet handle
                skip |= validate_required_handle(vkCallingFunction,
                                                 ParameterName("pDescriptorWrites[%i].dstSet", ParameterName::IndexVector{i}),
                                                 pDescriptorWrites[i].dstSet);
            }

            if ((pDescriptorWrites[i].descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER) ||
                (pDescriptorWrites[i].descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) ||
                (pDescriptorWrites[i].descriptorType == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE) ||
                (pDescriptorWrites[i].descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE) ||
                (pDescriptorWrites[i].descriptorType == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT)) {
                // If descriptorType is VK_DESCRIPTOR_TYPE_SAMPLER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                // VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE or VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
                // pImageInfo must be a pointer to an array of descriptorCount valid VkDescriptorImageInfo structures.
                // Valid imageView handles are checked in ObjectLifetimes::ValidateDescriptorWrite.
                if (pDescriptorWrites[i].pImageInfo == nullptr) {
                    skip |= LogError(device, "VUID-VkWriteDescriptorSet-descriptorType-00322",
                                     "%s(): if pDescriptorWrites[%d].descriptorType is "
                                     "VK_DESCRIPTOR_TYPE_SAMPLER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, "
                                     "VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE or "
                                     "VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, pDescriptorWrites[%d].pImageInfo must not be NULL.",
                                     vkCallingFunction, i, i);
                } else if (pDescriptorWrites[i].descriptorType != VK_DESCRIPTOR_TYPE_SAMPLER) {
                    // If descriptorType is VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                    // VK_DESCRIPTOR_TYPE_STORAGE_IMAGE or VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, the imageLayout
                    // member of any given element of pImageInfo must be a valid VkImageLayout
                    for (uint32_t descriptor_index = 0; descriptor_index < pDescriptorWrites[i].descriptorCount;
                         ++descriptor_index) {
                        skip |= validate_ranged_enum(vkCallingFunction,
                                                     ParameterName("pDescriptorWrites[%i].pImageInfo[%i].imageLayout",
                                                                   ParameterName::IndexVector{i, descriptor_index}),
                                                     "VkImageLayout", AllVkImageLayoutEnums,
                                                     pDescriptorWrites[i].pImageInfo[descriptor_index].imageLayout, kVUIDUndefined);
                    }
                }
            } else if ((pDescriptorWrites[i].descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) ||
                       (pDescriptorWrites[i].descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) ||
                       (pDescriptorWrites[i].descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) ||
                       (pDescriptorWrites[i].descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)) {
                // If descriptorType is VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                // VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC or VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, pBufferInfo must be a
                // pointer to an array of descriptorCount valid VkDescriptorBufferInfo structures
                // Valid buffer handles are checked in ObjectLifetimes::ValidateDescriptorWrite.
                if (pDescriptorWrites[i].pBufferInfo == nullptr) {
                    skip |= LogError(device, "VUID-VkWriteDescriptorSet-descriptorType-00324",
                                     "%s(): if pDescriptorWrites[%d].descriptorType is "
                                     "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, "
                                     "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC or VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, "
                                     "pDescriptorWrites[%d].pBufferInfo must not be NULL.",
                                     vkCallingFunction, i, i);
                } else {
                    const auto *robustness2_features =
                        lvl_find_in_chain<VkPhysicalDeviceRobustness2FeaturesEXT>(device_createinfo_pnext);
                    if (robustness2_features && robustness2_features->nullDescriptor) {
                        for (uint32_t descriptorIndex = 0; descriptorIndex < pDescriptorWrites[i].descriptorCount;
                             ++descriptorIndex) {
                            if (pDescriptorWrites[i].pBufferInfo[descriptorIndex].buffer == VK_NULL_HANDLE &&
                                (pDescriptorWrites[i].pBufferInfo[descriptorIndex].offset != 0 ||
                                 pDescriptorWrites[i].pBufferInfo[descriptorIndex].range != VK_WHOLE_SIZE)) {
                                skip |= LogError(device, "VUID-VkDescriptorBufferInfo-buffer-02999",
                                                 "%s(): if pDescriptorWrites[%d].buffer is VK_NULL_HANDLE, "
                                                 "offset (%" PRIu64 ") must be zero and range (%" PRIu64 ") must be VK_WHOLE_SIZE.",
                                                 vkCallingFunction, i, pDescriptorWrites[i].pBufferInfo[descriptorIndex].offset,
                                                 pDescriptorWrites[i].pBufferInfo[descriptorIndex].range);
                            }
                        }
                    }
                }
            } else if ((pDescriptorWrites[i].descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER) ||
                       (pDescriptorWrites[i].descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER)) {
                // Valid bufferView handles are checked in ObjectLifetimes::ValidateDescriptorWrite.
            }

            if ((pDescriptorWrites[i].descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) ||
                (pDescriptorWrites[i].descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC)) {
                VkDeviceSize uniformAlignment = device_limits.minUniformBufferOffsetAlignment;
                for (uint32_t j = 0; j < pDescriptorWrites[i].descriptorCount; j++) {
                    if (pDescriptorWrites[i].pBufferInfo != NULL) {
                        if (SafeModulo(pDescriptorWrites[i].pBufferInfo[j].offset, uniformAlignment) != 0) {
                            skip |=
                                LogError(device, "VUID-VkWriteDescriptorSet-descriptorType-00327",
                                         "%s(): pDescriptorWrites[%d].pBufferInfo[%d].offset (0x%" PRIxLEAST64
                                         ") must be a multiple of device limit minUniformBufferOffsetAlignment 0x%" PRIxLEAST64 ".",
                                         vkCallingFunction, i, j, pDescriptorWrites[i].pBufferInfo[j].offset, uniformAlignment);
                        }
                    }
                }
            } else if ((pDescriptorWrites[i].descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) ||
                       (pDescriptorWrites[i].descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)) {
                VkDeviceSize storageAlignment = device_limits.minStorageBufferOffsetAlignment;
                for (uint32_t j = 0; j < pDescriptorWrites[i].descriptorCount; j++) {
                    if (pDescriptorWrites[i].pBufferInfo != NULL) {
                        if (SafeModulo(pDescriptorWrites[i].pBufferInfo[j].offset, storageAlignment) != 0) {
                            skip |=
                                LogError(device, "VUID-VkWriteDescriptorSet-descriptorType-00328",
                                         "%s(): pDescriptorWrites[%d].pBufferInfo[%d].offset (0x%" PRIxLEAST64
                                         ") must be a multiple of device limit minStorageBufferOffsetAlignment 0x%" PRIxLEAST64 ".",
                                         vkCallingFunction, i, j, pDescriptorWrites[i].pBufferInfo[j].offset, storageAlignment);
                        }
                    }
                }
            }
            // pNext chain must be either NULL or a pointer to a valid instance of VkWriteDescriptorSetAccelerationStructureKHR
            // or VkWriteDescriptorSetInlineUniformBlockEX
            if (pDescriptorWrites[i].pNext) {
                if (pDescriptorWrites[i].descriptorType == VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR) {
                    const auto *pnext_struct =
                        lvl_find_in_chain<VkWriteDescriptorSetAccelerationStructureKHR>(pDescriptorWrites[i].pNext);
                    if (!pnext_struct || (pnext_struct->accelerationStructureCount != pDescriptorWrites[i].descriptorCount)) {
                        skip |= LogError(device, "VUID-VkWriteDescriptorSet-descriptorType-02382",
                                         "%s(): If descriptorType is VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, the pNext"
                                         "chain must include a VkWriteDescriptorSetAccelerationStructureKHR structure whose "
                                         "accelerationStructureCount %d member equals descriptorCount %d.",
                                         vkCallingFunction, pnext_struct ? pnext_struct->accelerationStructureCount : -1,
                                         pDescriptorWrites[i].descriptorCount);
                    }
                    // further checks only if we have right structtype
                    if (pnext_struct) {
                        if (pnext_struct->accelerationStructureCount != pDescriptorWrites[i].descriptorCount) {
                            skip |= LogError(
                                device, "VUID-VkWriteDescriptorSetAccelerationStructureKHR-accelerationStructureCount-02236",
                                "%s(): accelerationStructureCount %d must be equal to descriptorCount %d in the extended structure "
                                ".",
                                vkCallingFunction, pnext_struct->accelerationStructureCount, pDescriptorWrites[i].descriptorCount);
                        }
                        if (pnext_struct->accelerationStructureCount == 0) {
                            skip |= LogError(
                                device, "VUID-VkWriteDescriptorSetAccelerationStructureKHR-accelerationStructureCount-arraylength",
                                "%s(): accelerationStructureCount must be greater than 0 .");
                        }
                    }
                }
            }
        }
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount,
                                                                     const VkWriteDescriptorSet *pDescriptorWrites,
                                                                     uint32_t descriptorCopyCount,
                                                                     const VkCopyDescriptorSet *pDescriptorCopies) const {
    return validate_WriteDescriptorSet("vkUpdateDescriptorSets", descriptorWriteCount, pDescriptorWrites);
}

bool StatelessValidation::manual_PreCallValidateCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo *pCreateInfo,
                                                                 const VkAllocationCallbacks *pAllocator,
                                                                 VkRenderPass *pRenderPass) const {
    return CreateRenderPassGeneric(device, pCreateInfo, pAllocator, pRenderPass, RENDER_PASS_VERSION_1);
}

bool StatelessValidation::manual_PreCallValidateCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2KHR *pCreateInfo,
                                                                     const VkAllocationCallbacks *pAllocator,
                                                                     VkRenderPass *pRenderPass) const {
    return CreateRenderPassGeneric(device, pCreateInfo, pAllocator, pRenderPass, RENDER_PASS_VERSION_2);
}

bool StatelessValidation::manual_PreCallValidateFreeCommandBuffers(VkDevice device, VkCommandPool commandPool,
                                                                   uint32_t commandBufferCount,
                                                                   const VkCommandBuffer *pCommandBuffers) const {
    bool skip = false;

    // Validation for parameters excluded from the generated validation code due to a 'noautovalidity' tag in vk.xml
    // This is an array of handles, where the elements are allowed to be VK_NULL_HANDLE, and does not require any validation beyond
    // validate_array()
    skip |= validate_array("vkFreeCommandBuffers", "commandBufferCount", "pCommandBuffers", commandBufferCount, &pCommandBuffers,
                           true, true, kVUIDUndefined, kVUIDUndefined);
    return skip;
}

bool StatelessValidation::manual_PreCallValidateBeginCommandBuffer(VkCommandBuffer commandBuffer,
                                                                   const VkCommandBufferBeginInfo *pBeginInfo) const {
    bool skip = false;

    // VkCommandBufferInheritanceInfo validation, due to a 'noautovalidity' of pBeginInfo->pInheritanceInfo in vkBeginCommandBuffer
    const char *cmd_name = "vkBeginCommandBuffer";
    const VkCommandBufferInheritanceInfo *pInfo = pBeginInfo->pInheritanceInfo;

    // Implicit VUs
    // validate only sType here; pointer has to be validated in core_validation
    const bool kNotRequired = false;
    const char *kNoVUID = nullptr;
    skip |= validate_struct_type(cmd_name, "pBeginInfo->pInheritanceInfo", "VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO",
                                 pInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO, kNotRequired, kNoVUID,
                                 "VUID-VkCommandBufferInheritanceInfo-sType-sType");

    if (pInfo) {
        const VkStructureType allowed_structs_VkCommandBufferInheritanceInfo[] = {
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_CONDITIONAL_RENDERING_INFO_EXT};
        skip |= validate_struct_pnext(
            cmd_name, "pBeginInfo->pInheritanceInfo->pNext", "VkCommandBufferInheritanceConditionalRenderingInfoEXT", pInfo->pNext,
            ARRAY_SIZE(allowed_structs_VkCommandBufferInheritanceInfo), allowed_structs_VkCommandBufferInheritanceInfo,
            GeneratedVulkanHeaderVersion, "VUID-VkCommandBufferInheritanceInfo-pNext-pNext",
            "VUID-VkCommandBufferInheritanceInfo-sType-unique");

        skip |= validate_bool32(cmd_name, "pBeginInfo->pInheritanceInfo->occlusionQueryEnable", pInfo->occlusionQueryEnable);

        // Explicit VUs
        if (!physical_device_features.inheritedQueries && pInfo->occlusionQueryEnable == VK_TRUE) {
            skip |= LogError(
                commandBuffer, "VUID-VkCommandBufferInheritanceInfo-occlusionQueryEnable-00056",
                "%s: Inherited queries feature is disabled, but pBeginInfo->pInheritanceInfo->occlusionQueryEnable is VK_TRUE.",
                cmd_name);
        }

        if (physical_device_features.inheritedQueries) {
            skip |= validate_flags(cmd_name, "pBeginInfo->pInheritanceInfo->queryFlags", "VkQueryControlFlagBits",
                                   AllVkQueryControlFlagBits, pInfo->queryFlags, kOptionalFlags,
                                   "VUID-VkCommandBufferInheritanceInfo-queryFlags-00057");
        } else {  // !inheritedQueries
            skip |= validate_reserved_flags(cmd_name, "pBeginInfo->pInheritanceInfo->queryFlags", pInfo->queryFlags,
                                            "VUID-VkCommandBufferInheritanceInfo-queryFlags-02788");
        }

        if (physical_device_features.pipelineStatisticsQuery) {
            skip |= validate_flags(cmd_name, "pBeginInfo->pInheritanceInfo->pipelineStatistics", "VkQueryPipelineStatisticFlagBits",
                                   AllVkQueryPipelineStatisticFlagBits, pInfo->pipelineStatistics, kOptionalFlags,
                                   "VUID-VkCommandBufferInheritanceInfo-pipelineStatistics-02789");
        } else {  // !pipelineStatisticsQuery
            skip |= validate_reserved_flags(cmd_name, "pBeginInfo->pInheritanceInfo->pipelineStatistics", pInfo->pipelineStatistics,
                                            "VUID-VkCommandBufferInheritanceInfo-pipelineStatistics-00058");
        }

        const auto *conditional_rendering = lvl_find_in_chain<VkCommandBufferInheritanceConditionalRenderingInfoEXT>(pInfo->pNext);
        if (conditional_rendering) {
            const auto *cr_features = lvl_find_in_chain<VkPhysicalDeviceConditionalRenderingFeaturesEXT>(device_createinfo_pnext);
            const auto inherited_conditional_rendering = cr_features && cr_features->inheritedConditionalRendering;
            if (!inherited_conditional_rendering && conditional_rendering->conditionalRenderingEnable == VK_TRUE) {
                skip |= LogError(
                    commandBuffer, "VUID-VkCommandBufferInheritanceConditionalRenderingInfoEXT-conditionalRenderingEnable-01977",
                    "vkBeginCommandBuffer: Inherited conditional rendering is disabled, but "
                    "pBeginInfo->pInheritanceInfo->pNext<VkCommandBufferInheritanceConditionalRenderingInfoEXT> is VK_TRUE.");
            }
        }
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                               uint32_t viewportCount, const VkViewport *pViewports) const {
    bool skip = false;

    if (!physical_device_features.multiViewport) {
        if (firstViewport != 0) {
            skip |= LogError(commandBuffer, "VUID-vkCmdSetViewport-firstViewport-01224",
                             "vkCmdSetViewport: The multiViewport feature is disabled, but firstViewport (=%" PRIu32 ") is not 0.",
                             firstViewport);
        }
        if (viewportCount > 1) {
            skip |= LogError(commandBuffer, "VUID-vkCmdSetViewport-viewportCount-01225",
                             "vkCmdSetViewport: The multiViewport feature is disabled, but viewportCount (=%" PRIu32 ") is not 1.",
                             viewportCount);
        }
    } else {  // multiViewport enabled
        const uint64_t sum = static_cast<uint64_t>(firstViewport) + static_cast<uint64_t>(viewportCount);
        if (sum > device_limits.maxViewports) {
            skip |= LogError(commandBuffer, "VUID-vkCmdSetViewport-firstViewport-01223",
                             "vkCmdSetViewport: firstViewport + viewportCount (=%" PRIu32 " + %" PRIu32 " = %" PRIu64
                             ") is greater than VkPhysicalDeviceLimits::maxViewports (=%" PRIu32 ").",
                             firstViewport, viewportCount, sum, device_limits.maxViewports);
        }
    }

    if (pViewports) {
        for (uint32_t viewport_i = 0; viewport_i < viewportCount; ++viewport_i) {
            const auto &viewport = pViewports[viewport_i];  // will crash on invalid ptr
            const char *fn_name = "vkCmdSetViewport";
            skip |= manual_PreCallValidateViewport(
                viewport, fn_name, ParameterName("pViewports[%i]", ParameterName::IndexVector{viewport_i}), commandBuffer);
        }
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor,
                                                              uint32_t scissorCount, const VkRect2D *pScissors) const {
    bool skip = false;

    if (!physical_device_features.multiViewport) {
        if (firstScissor != 0) {
            skip |= LogError(commandBuffer, "VUID-vkCmdSetScissor-firstScissor-00593",
                             "vkCmdSetScissor: The multiViewport feature is disabled, but firstScissor (=%" PRIu32 ") is not 0.",
                             firstScissor);
        }
        if (scissorCount > 1) {
            skip |= LogError(commandBuffer, "VUID-vkCmdSetScissor-scissorCount-00594",
                             "vkCmdSetScissor: The multiViewport feature is disabled, but scissorCount (=%" PRIu32 ") is not 1.",
                             scissorCount);
        }
    } else {  // multiViewport enabled
        const uint64_t sum = static_cast<uint64_t>(firstScissor) + static_cast<uint64_t>(scissorCount);
        if (sum > device_limits.maxViewports) {
            skip |= LogError(commandBuffer, "VUID-vkCmdSetScissor-firstScissor-00592",
                             "vkCmdSetScissor: firstScissor + scissorCount (=%" PRIu32 " + %" PRIu32 " = %" PRIu64
                             ") is greater than VkPhysicalDeviceLimits::maxViewports (=%" PRIu32 ").",
                             firstScissor, scissorCount, sum, device_limits.maxViewports);
        }
    }

    if (pScissors) {
        for (uint32_t scissor_i = 0; scissor_i < scissorCount; ++scissor_i) {
            const auto &scissor = pScissors[scissor_i];  // will crash on invalid ptr

            if (scissor.offset.x < 0) {
                skip |= LogError(commandBuffer, "VUID-vkCmdSetScissor-x-00595",
                                 "vkCmdSetScissor: pScissors[%" PRIu32 "].offset.x (=%" PRIi32 ") is negative.", scissor_i,
                                 scissor.offset.x);
            }

            if (scissor.offset.y < 0) {
                skip |= LogError(commandBuffer, "VUID-vkCmdSetScissor-x-00595",
                                 "vkCmdSetScissor: pScissors[%" PRIu32 "].offset.y (=%" PRIi32 ") is negative.", scissor_i,
                                 scissor.offset.y);
            }

            const int64_t x_sum = static_cast<int64_t>(scissor.offset.x) + static_cast<int64_t>(scissor.extent.width);
            if (x_sum > INT32_MAX) {
                skip |= LogError(commandBuffer, "VUID-vkCmdSetScissor-offset-00596",
                                 "vkCmdSetScissor: offset.x + extent.width (=%" PRIi32 " + %" PRIu32 " = %" PRIi64
                                 ") of pScissors[%" PRIu32 "] will overflow int32_t.",
                                 scissor.offset.x, scissor.extent.width, x_sum, scissor_i);
            }

            const int64_t y_sum = static_cast<int64_t>(scissor.offset.y) + static_cast<int64_t>(scissor.extent.height);
            if (y_sum > INT32_MAX) {
                skip |= LogError(commandBuffer, "VUID-vkCmdSetScissor-offset-00597",
                                 "vkCmdSetScissor: offset.y + extent.height (=%" PRIi32 " + %" PRIu32 " = %" PRIi64
                                 ") of pScissors[%" PRIu32 "] will overflow int32_t.",
                                 scissor.offset.y, scissor.extent.height, y_sum, scissor_i);
            }
        }
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth) const {
    bool skip = false;

    if (!physical_device_features.wideLines && (lineWidth != 1.0f)) {
        skip |= LogError(commandBuffer, "VUID-vkCmdSetLineWidth-lineWidth-00788",
                         "VkPhysicalDeviceFeatures::wideLines is disabled, but lineWidth (=%f) is not 1.0.", lineWidth);
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                                uint32_t count, uint32_t stride) const {
    bool skip = false;

    if (!physical_device_features.multiDrawIndirect && ((count > 1))) {
        skip |= LogError(device, kVUID_PVError_DeviceFeature,
                         "CmdDrawIndirect(): Device feature multiDrawIndirect disabled: count must be 0 or 1 but is %d", count);
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                       VkDeviceSize offset, uint32_t count, uint32_t stride) const {
    bool skip = false;
    if (!physical_device_features.multiDrawIndirect && ((count > 1))) {
        skip |=
            LogError(device, kVUID_PVError_DeviceFeature,
                     "CmdDrawIndexedIndirect(): Device feature multiDrawIndirect disabled: count must be 0 or 1 but is %d", count);
    }
    return skip;
}

bool StatelessValidation::ValidateCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkDeviceSize offset,
                                                       VkDeviceSize countBufferOffset, bool khr) const {
    bool skip = false;
    const char *apiName = khr ? "vkCmdDrawIndirectCountKHR()" : "vkCmdDrawIndirectCount()";
    if (offset & 3) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDrawIndirectCount-offset-02710",
                         "%s: parameter, VkDeviceSize offset (0x%" PRIxLEAST64 "), is not a multiple of 4.", apiName, offset);
    }

    if (countBufferOffset & 3) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDrawIndirectCount-countBufferOffset-02716",
                         "%s: parameter, VkDeviceSize countBufferOffset (0x%" PRIxLEAST64 "), is not a multiple of 4.", apiName,
                         countBufferOffset);
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                     VkDeviceSize offset, VkBuffer countBuffer,
                                                                     VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                     uint32_t stride) const {
    return ValidateCmdDrawIndirectCount(commandBuffer, offset, countBufferOffset, false);
}

bool StatelessValidation::manual_PreCallValidateCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                        VkDeviceSize offset, VkBuffer countBuffer,
                                                                        VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                        uint32_t stride) const {
    return ValidateCmdDrawIndirectCount(commandBuffer, offset, countBufferOffset, true);
}

bool StatelessValidation::ValidateCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkDeviceSize offset,
                                                              VkDeviceSize countBufferOffset, bool khr) const {
    bool skip = false;
    const char *apiName = khr ? "vkCmdDrawIndexedIndirectCountKHR()" : "vkCmdDrawIndexedIndirectCount()";
    if (offset & 3) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDrawIndexedIndirectCount-offset-02710",
                         "%s: parameter, VkDeviceSize offset (0x%" PRIxLEAST64 "), is not a multiple of 4.", apiName, offset);
    }

    if (countBufferOffset & 3) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDrawIndexedIndirectCount-countBufferOffset-02716",
                         "%s: parameter, VkDeviceSize countBufferOffset (0x%" PRIxLEAST64 "), is not a multiple of 4.", apiName,
                         countBufferOffset);
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                            VkDeviceSize offset, VkBuffer countBuffer,
                                                                            VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                            uint32_t stride) const {
    return ValidateCmdDrawIndexedIndirectCount(commandBuffer, offset, countBufferOffset, false);
}

bool StatelessValidation::manual_PreCallValidateCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                               VkDeviceSize offset, VkBuffer countBuffer,
                                                                               VkDeviceSize countBufferOffset,
                                                                               uint32_t maxDrawCount, uint32_t stride) const {
    return ValidateCmdDrawIndexedIndirectCount(commandBuffer, offset, countBufferOffset, true);
}

bool StatelessValidation::manual_PreCallValidateCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                                                    const VkClearAttachment *pAttachments, uint32_t rectCount,
                                                                    const VkClearRect *pRects) const {
    bool skip = false;
    for (uint32_t rect = 0; rect < rectCount; rect++) {
        if (pRects[rect].layerCount == 0) {
            skip |= LogError(commandBuffer, "VUID-vkCmdClearAttachments-layerCount-01934",
                             "CmdClearAttachments(): pRects[%d].layerCount is zero.", rect);
        }
        if (pRects[rect].rect.extent.width == 0) {
            skip |= LogError(commandBuffer, "VUID-vkCmdClearAttachments-rect-02682",
                             "CmdClearAttachments(): pRects[%d].rect.extent.width is zero.", rect);
        }
        if (pRects[rect].rect.extent.height == 0) {
            skip |= LogError(commandBuffer, "VUID-vkCmdClearAttachments-rect-02683",
                             "CmdClearAttachments(): pRects[%d].rect.extent.height is zero.", rect);
        }
    }
    return skip;
}

bool StatelessValidation::ValidateGetPhysicalDeviceImageFormatProperties2(VkPhysicalDevice physicalDevice,
                                                                          const VkPhysicalDeviceImageFormatInfo2 *pImageFormatInfo,
                                                                          VkImageFormatProperties2 *pImageFormatProperties,
                                                                          const char *apiName) const {
    bool skip = false;

    if (pImageFormatInfo != nullptr) {
        const auto image_stencil_struct = lvl_find_in_chain<VkImageStencilUsageCreateInfoEXT>(pImageFormatInfo->pNext);
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

bool StatelessValidation::manual_PreCallValidateCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage,
                                                             VkImageLayout srcImageLayout, VkImage dstImage,
                                                             VkImageLayout dstImageLayout, uint32_t regionCount,
                                                             const VkImageCopy *pRegions) const {
    bool skip = false;

    VkImageAspectFlags legal_aspect_flags =
        VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_METADATA_BIT;
    if (device_extensions.vk_khr_sampler_ycbcr_conversion) {
        legal_aspect_flags |= (VK_IMAGE_ASPECT_PLANE_0_BIT_KHR | VK_IMAGE_ASPECT_PLANE_1_BIT_KHR | VK_IMAGE_ASPECT_PLANE_2_BIT_KHR);
    }

    if (pRegions != nullptr) {
        if ((pRegions->srcSubresource.aspectMask & legal_aspect_flags) == 0) {
            skip |= LogError(
                device, "VUID-VkImageSubresourceLayers-aspectMask-parameter",
                "vkCmdCopyImage() parameter, VkImageAspect pRegions->srcSubresource.aspectMask, is an unrecognized enumerator.");
        }
        if ((pRegions->dstSubresource.aspectMask & legal_aspect_flags) == 0) {
            skip |= LogError(
                device, "VUID-VkImageSubresourceLayers-aspectMask-parameter",
                "vkCmdCopyImage() parameter, VkImageAspect pRegions->dstSubresource.aspectMask, is an unrecognized enumerator.");
        }
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage,
                                                             VkImageLayout srcImageLayout, VkImage dstImage,
                                                             VkImageLayout dstImageLayout, uint32_t regionCount,
                                                             const VkImageBlit *pRegions, VkFilter filter) const {
    bool skip = false;

    VkImageAspectFlags legal_aspect_flags =
        VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_METADATA_BIT;
    if (device_extensions.vk_khr_sampler_ycbcr_conversion) {
        legal_aspect_flags |= (VK_IMAGE_ASPECT_PLANE_0_BIT_KHR | VK_IMAGE_ASPECT_PLANE_1_BIT_KHR | VK_IMAGE_ASPECT_PLANE_2_BIT_KHR);
    }

    if (pRegions != nullptr) {
        if ((pRegions->srcSubresource.aspectMask & legal_aspect_flags) == 0) {
            skip |= LogError(
                device, kVUID_PVError_UnrecognizedValue,
                "vkCmdBlitImage() parameter, VkImageAspect pRegions->srcSubresource.aspectMask, is an unrecognized enumerator");
        }
        if ((pRegions->dstSubresource.aspectMask & legal_aspect_flags) == 0) {
            skip |= LogError(
                device, kVUID_PVError_UnrecognizedValue,
                "vkCmdBlitImage() parameter, VkImageAspect pRegions->dstSubresource.aspectMask, is an unrecognized enumerator");
        }
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer,
                                                              uint32_t regionCount, const VkBufferCopy *pRegions) const {
    bool skip = false;

    if (pRegions != nullptr) {
        for (uint32_t i = 0; i < regionCount; i++) {
            if (pRegions[i].size == 0) {
                skip |= LogError(device, "VUID-VkBufferCopy-size-01988",
                                 "vkCmdCopyBuffer() pRegions[%u].size must be greater than zero", i);
            }
        }
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer,
                                                                     VkImage dstImage, VkImageLayout dstImageLayout,
                                                                     uint32_t regionCount,
                                                                     const VkBufferImageCopy *pRegions) const {
    bool skip = false;

    VkImageAspectFlags legal_aspect_flags =
        VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_METADATA_BIT;
    if (device_extensions.vk_khr_sampler_ycbcr_conversion) {
        legal_aspect_flags |= (VK_IMAGE_ASPECT_PLANE_0_BIT_KHR | VK_IMAGE_ASPECT_PLANE_1_BIT_KHR | VK_IMAGE_ASPECT_PLANE_2_BIT_KHR);
    }

    if (pRegions != nullptr) {
        if ((pRegions->imageSubresource.aspectMask & legal_aspect_flags) == 0) {
            skip |= LogError(device, kVUID_PVError_UnrecognizedValue,
                             "vkCmdCopyBufferToImage() parameter, VkImageAspect pRegions->imageSubresource.aspectMask, is an "
                             "unrecognized enumerator");
        }
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage,
                                                                     VkImageLayout srcImageLayout, VkBuffer dstBuffer,
                                                                     uint32_t regionCount,
                                                                     const VkBufferImageCopy *pRegions) const {
    bool skip = false;

    VkImageAspectFlags legal_aspect_flags =
        VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_METADATA_BIT;
    if (device_extensions.vk_khr_sampler_ycbcr_conversion) {
        legal_aspect_flags |= (VK_IMAGE_ASPECT_PLANE_0_BIT_KHR | VK_IMAGE_ASPECT_PLANE_1_BIT_KHR | VK_IMAGE_ASPECT_PLANE_2_BIT_KHR);
    }

    if (pRegions != nullptr) {
        if ((pRegions->imageSubresource.aspectMask & legal_aspect_flags) == 0) {
            skip |= LogError(
                device, kVUID_PVError_UnrecognizedValue,
                "vkCmdCopyImageToBuffer parameter, VkImageAspect pRegions->imageSubresource.aspectMask, is an unrecognized "
                "enumerator");
        }
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer,
                                                                VkDeviceSize dstOffset, VkDeviceSize dataSize,
                                                                const void *pData) const {
    bool skip = false;

    if (dstOffset & 3) {
        skip |= LogError(device, "VUID-vkCmdUpdateBuffer-dstOffset-00036",
                         "vkCmdUpdateBuffer() parameter, VkDeviceSize dstOffset (0x%" PRIxLEAST64 "), is not a multiple of 4.",
                         dstOffset);
    }

    if ((dataSize <= 0) || (dataSize > 65536)) {
        skip |= LogError(device, "VUID-vkCmdUpdateBuffer-dataSize-00037",
                         "vkCmdUpdateBuffer() parameter, VkDeviceSize dataSize (0x%" PRIxLEAST64
                         "), must be greater than zero and less than or equal to 65536.",
                         dataSize);
    } else if (dataSize & 3) {
        skip |= LogError(device, "VUID-vkCmdUpdateBuffer-dataSize-00038",
                         "vkCmdUpdateBuffer() parameter, VkDeviceSize dataSize (0x%" PRIxLEAST64 "), is not a multiple of 4.",
                         dataSize);
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer,
                                                              VkDeviceSize dstOffset, VkDeviceSize size, uint32_t data) const {
    bool skip = false;

    if (dstOffset & 3) {
        skip |= LogError(device, "VUID-vkCmdFillBuffer-dstOffset-00025",
                         "vkCmdFillBuffer() parameter, VkDeviceSize dstOffset (0x%" PRIxLEAST64 "), is not a multiple of 4.",
                         dstOffset);
    }

    if (size != VK_WHOLE_SIZE) {
        if (size <= 0) {
            skip |=
                LogError(device, "VUID-vkCmdFillBuffer-size-00026",
                         "vkCmdFillBuffer() parameter, VkDeviceSize size (0x%" PRIxLEAST64 "), must be greater than zero.", size);
        } else if (size & 3) {
            skip |= LogError(device, "VUID-vkCmdFillBuffer-size-00028",
                             "vkCmdFillBuffer() parameter, VkDeviceSize size (0x%" PRIxLEAST64 "), is not a multiple of 4.", size);
        }
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR *pCreateInfo,
                                                                   const VkAllocationCallbacks *pAllocator,
                                                                   VkSwapchainKHR *pSwapchain) const {
    bool skip = false;

    if (pCreateInfo != nullptr) {
        // Validation for parameters excluded from the generated validation code due to a 'noautovalidity' tag in vk.xml
        if (pCreateInfo->imageSharingMode == VK_SHARING_MODE_CONCURRENT) {
            // If imageSharingMode is VK_SHARING_MODE_CONCURRENT, queueFamilyIndexCount must be greater than 1
            if (pCreateInfo->queueFamilyIndexCount <= 1) {
                skip |= LogError(device, "VUID-VkSwapchainCreateInfoKHR-imageSharingMode-01278",
                                 "vkCreateSwapchainKHR(): if pCreateInfo->imageSharingMode is VK_SHARING_MODE_CONCURRENT, "
                                 "pCreateInfo->queueFamilyIndexCount must be greater than 1.");
            }

            // If imageSharingMode is VK_SHARING_MODE_CONCURRENT, pQueueFamilyIndices must be a pointer to an array of
            // queueFamilyIndexCount uint32_t values
            if (pCreateInfo->pQueueFamilyIndices == nullptr) {
                skip |= LogError(device, "VUID-VkSwapchainCreateInfoKHR-imageSharingMode-01277",
                                 "vkCreateSwapchainKHR(): if pCreateInfo->imageSharingMode is VK_SHARING_MODE_CONCURRENT, "
                                 "pCreateInfo->pQueueFamilyIndices must be a pointer to an array of "
                                 "pCreateInfo->queueFamilyIndexCount uint32_t values.");
            }
        }

        skip |= ValidateGreaterThanZero(pCreateInfo->imageArrayLayers, "pCreateInfo->imageArrayLayers",
                                        "VUID-VkSwapchainCreateInfoKHR-imageArrayLayers-01275", "vkCreateSwapchainKHR");
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR *pPresentInfo) const {
    bool skip = false;

    if (pPresentInfo && pPresentInfo->pNext) {
        const auto *present_regions = lvl_find_in_chain<VkPresentRegionsKHR>(pPresentInfo->pNext);
        if (present_regions) {
            // TODO: This and all other pNext extension dependencies should be added to code-generation
            skip |= require_device_extension(IsExtEnabled(device_extensions.vk_khr_incremental_present), "vkQueuePresentKHR",
                                             VK_KHR_INCREMENTAL_PRESENT_EXTENSION_NAME);
            if (present_regions->swapchainCount != pPresentInfo->swapchainCount) {
                skip |= LogError(device, "VUID-VkPresentRegionsKHR-swapchainCount-01260",
                                 "QueuePresentKHR(): pPresentInfo->swapchainCount has a value of %i but VkPresentRegionsKHR "
                                 "extension swapchainCount is %i. These values must be equal.",
                                 pPresentInfo->swapchainCount, present_regions->swapchainCount);
            }
            skip |= validate_struct_pnext("QueuePresentKHR", "pCreateInfo->pNext->pNext", NULL, present_regions->pNext, 0, NULL,
                                          GeneratedVulkanHeaderVersion, "VUID-VkPresentInfoKHR-pNext-pNext",
                                          "VUID-VkPresentInfoKHR-sType-unique");
            skip |= validate_array("QueuePresentKHR", "pCreateInfo->pNext->swapchainCount", "pCreateInfo->pNext->pRegions",
                                   present_regions->swapchainCount, &present_regions->pRegions, true, false, kVUIDUndefined,
                                   kVUIDUndefined);
            for (uint32_t i = 0; i < present_regions->swapchainCount; ++i) {
                skip |= validate_array("QueuePresentKHR", "pCreateInfo->pNext->pRegions[].rectangleCount",
                                       "pCreateInfo->pNext->pRegions[].pRectangles", present_regions->pRegions[i].rectangleCount,
                                       &present_regions->pRegions[i].pRectangles, true, false, kVUIDUndefined, kVUIDUndefined);
            }
        }
    }

    return skip;
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
bool StatelessValidation::manual_PreCallValidateCreateWin32SurfaceKHR(VkInstance instance,
                                                                      const VkWin32SurfaceCreateInfoKHR *pCreateInfo,
                                                                      const VkAllocationCallbacks *pAllocator,
                                                                      VkSurfaceKHR *pSurface) const {
    bool skip = false;

    if (pCreateInfo->hwnd == nullptr) {
        skip |= LogError(device, "VUID-VkWin32SurfaceCreateInfoKHR-hwnd-01308",
                         "vkCreateWin32SurfaceKHR(): hwnd must be a valid Win32 HWND but hwnd is NULL.");
    }

    return skip;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

bool StatelessValidation::manual_PreCallValidateCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo *pCreateInfo,
                                                                     const VkAllocationCallbacks *pAllocator,
                                                                     VkDescriptorPool *pDescriptorPool) const {
    bool skip = false;

    if (pCreateInfo) {
        if (pCreateInfo->maxSets <= 0) {
            skip |= LogError(device, "VUID-VkDescriptorPoolCreateInfo-maxSets-00301",
                             "vkCreateDescriptorPool(): pCreateInfo->maxSets is not greater than 0.");
        }

        if (pCreateInfo->pPoolSizes) {
            for (uint32_t i = 0; i < pCreateInfo->poolSizeCount; ++i) {
                if (pCreateInfo->pPoolSizes[i].descriptorCount <= 0) {
                    skip |= LogError(
                        device, "VUID-VkDescriptorPoolSize-descriptorCount-00302",
                        "vkCreateDescriptorPool(): pCreateInfo->pPoolSizes[%" PRIu32 "].descriptorCount is not greater than 0.", i);
                }
                if (pCreateInfo->pPoolSizes[i].type == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT &&
                    (pCreateInfo->pPoolSizes[i].descriptorCount % 4) != 0) {
                    skip |= LogError(device, "VUID-VkDescriptorPoolSize-type-02218",
                                     "vkCreateDescriptorPool(): pCreateInfo->pPoolSizes[%" PRIu32
                                     "].type is VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT "
                                     " and pCreateInfo->pPoolSizes[%" PRIu32 "].descriptorCount is not a multiple of 4.",
                                     i, i);
                }
            }
        }
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX,
                                                            uint32_t groupCountY, uint32_t groupCountZ) const {
    bool skip = false;

    if (groupCountX > device_limits.maxComputeWorkGroupCount[0]) {
        skip |=
            LogError(commandBuffer, "VUID-vkCmdDispatch-groupCountX-00386",
                     "vkCmdDispatch(): groupCountX (%" PRIu32 ") exceeds device limit maxComputeWorkGroupCount[0] (%" PRIu32 ").",
                     groupCountX, device_limits.maxComputeWorkGroupCount[0]);
    }

    if (groupCountY > device_limits.maxComputeWorkGroupCount[1]) {
        skip |=
            LogError(commandBuffer, "VUID-vkCmdDispatch-groupCountY-00387",
                     "vkCmdDispatch(): groupCountY (%" PRIu32 ") exceeds device limit maxComputeWorkGroupCount[1] (%" PRIu32 ").",
                     groupCountY, device_limits.maxComputeWorkGroupCount[1]);
    }

    if (groupCountZ > device_limits.maxComputeWorkGroupCount[2]) {
        skip |=
            LogError(commandBuffer, "VUID-vkCmdDispatch-groupCountZ-00388",
                     "vkCmdDispatch(): groupCountZ (%" PRIu32 ") exceeds device limit maxComputeWorkGroupCount[2] (%" PRIu32 ").",
                     groupCountZ, device_limits.maxComputeWorkGroupCount[2]);
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                    VkDeviceSize offset) const {
    bool skip = false;

    if ((offset % 4) != 0) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDispatchIndirect-offset-02710",
                         "vkCmdDispatchIndirect(): offset (%" PRIu64 ") must be a multiple of 4.", offset);
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX,
                                                                   uint32_t baseGroupY, uint32_t baseGroupZ, uint32_t groupCountX,
                                                                   uint32_t groupCountY, uint32_t groupCountZ) const {
    bool skip = false;

    // Paired if {} else if {} tests used to avoid any possible uint underflow
    uint32_t limit = device_limits.maxComputeWorkGroupCount[0];
    if (baseGroupX >= limit) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDispatchBase-baseGroupX-00421",
                         "vkCmdDispatch(): baseGroupX (%" PRIu32
                         ") equals or exceeds device limit maxComputeWorkGroupCount[0] (%" PRIu32 ").",
                         baseGroupX, limit);
    } else if (groupCountX > (limit - baseGroupX)) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDispatchBase-groupCountX-00424",
                         "vkCmdDispatchBaseKHR(): baseGroupX (%" PRIu32 ") + groupCountX (%" PRIu32
                         ") exceeds device limit maxComputeWorkGroupCount[0] (%" PRIu32 ").",
                         baseGroupX, groupCountX, limit);
    }

    limit = device_limits.maxComputeWorkGroupCount[1];
    if (baseGroupY >= limit) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDispatchBase-baseGroupX-00422",
                         "vkCmdDispatch(): baseGroupY (%" PRIu32
                         ") equals or exceeds device limit maxComputeWorkGroupCount[1] (%" PRIu32 ").",
                         baseGroupY, limit);
    } else if (groupCountY > (limit - baseGroupY)) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDispatchBase-groupCountY-00425",
                         "vkCmdDispatchBaseKHR(): baseGroupY (%" PRIu32 ") + groupCountY (%" PRIu32
                         ") exceeds device limit maxComputeWorkGroupCount[1] (%" PRIu32 ").",
                         baseGroupY, groupCountY, limit);
    }

    limit = device_limits.maxComputeWorkGroupCount[2];
    if (baseGroupZ >= limit) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDispatchBase-baseGroupZ-00423",
                         "vkCmdDispatch(): baseGroupZ (%" PRIu32
                         ") equals or exceeds device limit maxComputeWorkGroupCount[2] (%" PRIu32 ").",
                         baseGroupZ, limit);
    } else if (groupCountZ > (limit - baseGroupZ)) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDispatchBase-groupCountZ-00426",
                         "vkCmdDispatchBaseKHR(): baseGroupZ (%" PRIu32 ") + groupCountZ (%" PRIu32
                         ") exceeds device limit maxComputeWorkGroupCount[2] (%" PRIu32 ").",
                         baseGroupZ, groupCountZ, limit);
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer,
                                                                        VkPipelineBindPoint pipelineBindPoint,
                                                                        VkPipelineLayout layout, uint32_t set,
                                                                        uint32_t descriptorWriteCount,
                                                                        const VkWriteDescriptorSet *pDescriptorWrites) const {
    return validate_WriteDescriptorSet("vkCmdPushDescriptorSetKHR", descriptorWriteCount, pDescriptorWrites, false);
}

bool StatelessValidation::manual_PreCallValidateCmdSetExclusiveScissorNV(VkCommandBuffer commandBuffer,
                                                                         uint32_t firstExclusiveScissor,
                                                                         uint32_t exclusiveScissorCount,
                                                                         const VkRect2D *pExclusiveScissors) const {
    bool skip = false;

    if (!physical_device_features.multiViewport) {
        if (firstExclusiveScissor != 0) {
            skip |=
                LogError(commandBuffer, "VUID-vkCmdSetExclusiveScissorNV-firstExclusiveScissor-02035",
                         "vkCmdSetExclusiveScissorNV: The multiViewport feature is disabled, but firstExclusiveScissor (=%" PRIu32
                         ") is not 0.",
                         firstExclusiveScissor);
        }
        if (exclusiveScissorCount > 1) {
            skip |=
                LogError(commandBuffer, "VUID-vkCmdSetExclusiveScissorNV-exclusiveScissorCount-02036",
                         "vkCmdSetExclusiveScissorNV: The multiViewport feature is disabled, but exclusiveScissorCount (=%" PRIu32
                         ") is not 1.",
                         exclusiveScissorCount);
        }
    } else {  // multiViewport enabled
        const uint64_t sum = static_cast<uint64_t>(firstExclusiveScissor) + static_cast<uint64_t>(exclusiveScissorCount);
        if (sum > device_limits.maxViewports) {
            skip |= LogError(commandBuffer, "VUID-vkCmdSetExclusiveScissorNV-firstExclusiveScissor-02034",
                             "vkCmdSetExclusiveScissorNV: firstExclusiveScissor + exclusiveScissorCount (=%" PRIu32 " + %" PRIu32
                             " = %" PRIu64 ") is greater than VkPhysicalDeviceLimits::maxViewports (=%" PRIu32 ").",
                             firstExclusiveScissor, exclusiveScissorCount, sum, device_limits.maxViewports);
        }
    }

    if (firstExclusiveScissor >= device_limits.maxViewports) {
        skip |= LogError(commandBuffer, "VUID-vkCmdSetExclusiveScissorNV-firstExclusiveScissor-02033",
                         "vkCmdSetExclusiveScissorNV: firstExclusiveScissor (=%" PRIu32
                         ") must be less than maxViewports (=%" PRIu32 ").",
                         firstExclusiveScissor, device_limits.maxViewports);
    }

    if (pExclusiveScissors) {
        for (uint32_t scissor_i = 0; scissor_i < exclusiveScissorCount; ++scissor_i) {
            const auto &scissor = pExclusiveScissors[scissor_i];  // will crash on invalid ptr

            if (scissor.offset.x < 0) {
                skip |= LogError(commandBuffer, "VUID-vkCmdSetExclusiveScissorNV-x-02037",
                                 "vkCmdSetExclusiveScissorNV: pScissors[%" PRIu32 "].offset.x (=%" PRIi32 ") is negative.",
                                 scissor_i, scissor.offset.x);
            }

            if (scissor.offset.y < 0) {
                skip |= LogError(commandBuffer, "VUID-vkCmdSetExclusiveScissorNV-x-02037",
                                 "vkCmdSetExclusiveScissorNV: pScissors[%" PRIu32 "].offset.y (=%" PRIi32 ") is negative.",
                                 scissor_i, scissor.offset.y);
            }

            const int64_t x_sum = static_cast<int64_t>(scissor.offset.x) + static_cast<int64_t>(scissor.extent.width);
            if (x_sum > INT32_MAX) {
                skip |= LogError(commandBuffer, "VUID-vkCmdSetExclusiveScissorNV-offset-02038",
                                 "vkCmdSetExclusiveScissorNV: offset.x + extent.width (=%" PRIi32 " + %" PRIu32 " = %" PRIi64
                                 ") of pScissors[%" PRIu32 "] will overflow int32_t.",
                                 scissor.offset.x, scissor.extent.width, x_sum, scissor_i);
            }

            const int64_t y_sum = static_cast<int64_t>(scissor.offset.y) + static_cast<int64_t>(scissor.extent.height);
            if (y_sum > INT32_MAX) {
                skip |= LogError(commandBuffer, "VUID-vkCmdSetExclusiveScissorNV-offset-02039",
                                 "vkCmdSetExclusiveScissorNV: offset.y + extent.height (=%" PRIi32 " + %" PRIu32 " = %" PRIi64
                                 ") of pScissors[%" PRIu32 "] will overflow int32_t.",
                                 scissor.offset.y, scissor.extent.height, y_sum, scissor_i);
            }
        }
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdSetViewportWScalingNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                                         uint32_t viewportCount,
                                                                         const VkViewportWScalingNV *pViewportWScalings) const {
    bool skip = false;
    if (firstViewport >= device_limits.maxViewports) {
        skip |= LogError(commandBuffer, "VUID-vkCmdSetViewportWScalingNV-firstViewport-01323",
                         "vkCmdSetViewportWScalingNV: firstViewport (=%" PRIu32 ") must be less than maxViewports (=%" PRIu32 ").",
                         firstViewport, device_limits.maxViewports);
    } else {
        const uint64_t sum = static_cast<uint64_t>(firstViewport) + static_cast<uint64_t>(viewportCount);
        if ((sum < 1) || (sum > device_limits.maxViewports)) {
            skip |= LogError(commandBuffer, "VUID-vkCmdSetViewportWScalingNV-firstViewport-01324",
                             "vkCmdSetViewportWScalingNV: firstViewport + viewportCount (=%" PRIu32 " + %" PRIu32 " = %" PRIu64
                             ") must be between 1 and VkPhysicalDeviceLimits::maxViewports (=%" PRIu32 "), inculsive.",
                             firstViewport, viewportCount, sum, device_limits.maxViewports);
        }
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdSetViewportShadingRatePaletteNV(
    VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
    const VkShadingRatePaletteNV *pShadingRatePalettes) const {
    bool skip = false;

    if (!physical_device_features.multiViewport) {
        if (firstViewport != 0) {
            skip |=
                LogError(commandBuffer, "VUID-vkCmdSetViewportShadingRatePaletteNV-firstViewport-02068",
                         "vkCmdSetViewportShadingRatePaletteNV: The multiViewport feature is disabled, but firstViewport (=%" PRIu32
                         ") is not 0.",
                         firstViewport);
        }
        if (viewportCount > 1) {
            skip |=
                LogError(commandBuffer, "VUID-vkCmdSetViewportShadingRatePaletteNV-viewportCount-02069",
                         "vkCmdSetViewportShadingRatePaletteNV: The multiViewport feature is disabled, but viewportCount (=%" PRIu32
                         ") is not 1.",
                         viewportCount);
        }
    }

    if (firstViewport >= device_limits.maxViewports) {
        skip |= LogError(commandBuffer, "VUID-vkCmdSetViewportShadingRatePaletteNV-firstViewport-02066",
                         "vkCmdSetViewportShadingRatePaletteNV: firstViewport (=%" PRIu32
                         ") must be less than maxViewports (=%" PRIu32 ").",
                         firstViewport, device_limits.maxViewports);
    }

    const uint64_t sum = static_cast<uint64_t>(firstViewport) + static_cast<uint64_t>(viewportCount);
    if (sum > device_limits.maxViewports) {
        skip |= LogError(commandBuffer, "VUID-vkCmdSetViewportShadingRatePaletteNV-firstViewport-02067",
                         "vkCmdSetViewportShadingRatePaletteNV: firstViewport + viewportCount (=%" PRIu32 " + %" PRIu32
                         " = %" PRIu64 ") is greater than VkPhysicalDeviceLimits::maxViewports (=%" PRIu32 ").",
                         firstViewport, viewportCount, sum, device_limits.maxViewports);
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdSetCoarseSampleOrderNV(
    VkCommandBuffer commandBuffer, VkCoarseSampleOrderTypeNV sampleOrderType, uint32_t customSampleOrderCount,
    const VkCoarseSampleOrderCustomNV *pCustomSampleOrders) const {
    bool skip = false;

    if (sampleOrderType != VK_COARSE_SAMPLE_ORDER_TYPE_CUSTOM_NV && customSampleOrderCount != 0) {
        skip |= LogError(commandBuffer, "VUID-vkCmdSetCoarseSampleOrderNV-sampleOrderType-02081",
                         "vkCmdSetCoarseSampleOrderNV: If sampleOrderType is not VK_COARSE_SAMPLE_ORDER_TYPE_CUSTOM_NV, "
                         "customSampleOrderCount must be 0.");
    }

    for (uint32_t order_i = 0; order_i < customSampleOrderCount; ++order_i) {
        skip |= ValidateCoarseSampleOrderCustomNV(&pCustomSampleOrders[order_i]);
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount,
                                                                   uint32_t firstTask) const {
    bool skip = false;

    if (taskCount > phys_dev_ext_props.mesh_shader_props.maxDrawMeshTasksCount) {
        skip |= LogError(
            commandBuffer, "VUID-vkCmdDrawMeshTasksNV-taskCount-02119",
            "vkCmdDrawMeshTasksNV() parameter, uint32_t taskCount (0x%" PRIxLEAST32
            "), must be less than or equal to VkPhysicalDeviceMeshShaderPropertiesNV::maxDrawMeshTasksCount (0x%" PRIxLEAST32 ").",
            taskCount, phys_dev_ext_props.mesh_shader_props.maxDrawMeshTasksCount);
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                           VkDeviceSize offset, uint32_t drawCount,
                                                                           uint32_t stride) const {
    bool skip = false;
    static const int condition_multiples = 0b0011;
    if (offset & condition_multiples) {
        skip |= LogError(
            commandBuffer, "VUID-vkCmdDrawMeshTasksIndirectNV-offset-02710",
            "vkCmdDrawMeshTasksIndirectNV() parameter, VkDeviceSize offset (0x%" PRIxLEAST64 "), is not a multiple of 4.", offset);
    }
    if (drawCount > 1 && ((stride & condition_multiples) || stride < sizeof(VkDrawMeshTasksIndirectCommandNV))) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDrawMeshTasksIndirectNV-drawCount-02146",
                         "vkCmdDrawMeshTasksIndirectNV() parameter, uint32_t stride (0x%" PRIxLEAST32
                         "), is not a multiple of 4 or smaller than sizeof (VkDrawMeshTasksIndirectCommandNV).",
                         stride);
    }
    if (!physical_device_features.multiDrawIndirect && ((drawCount > 1))) {
        skip |= LogError(
            commandBuffer, "VUID-vkCmdDrawMeshTasksIndirectNV-drawCount-02718",
            "vkCmdDrawMeshTasksIndirectNV(): Device feature multiDrawIndirect disabled: count must be 0 or 1 but is %d", drawCount);
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                                VkDeviceSize offset, VkBuffer countBuffer,
                                                                                VkDeviceSize countBufferOffset,
                                                                                uint32_t maxDrawCount, uint32_t stride) const {
    bool skip = false;

    if (offset & 3) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDrawMeshTasksIndirectCountNV-offset-02710",
                         "vkCmdDrawMeshTasksIndirectCountNV() parameter, VkDeviceSize offset (0x%" PRIxLEAST64
                         "), is not a multiple of 4.",
                         offset);
    }

    if (countBufferOffset & 3) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDrawMeshTasksIndirectCountNV-countBufferOffset-02716",
                         "vkCmdDrawMeshTasksIndirectCountNV() parameter, VkDeviceSize countBufferOffset (0x%" PRIxLEAST64
                         "), is not a multiple of 4.",
                         countBufferOffset);
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo *pCreateInfo,
                                                                const VkAllocationCallbacks *pAllocator,
                                                                VkQueryPool *pQueryPool) const {
    bool skip = false;

    // Validation for parameters excluded from the generated validation code due to a 'noautovalidity' tag in vk.xml
    if (pCreateInfo != nullptr) {
        // If queryType is VK_QUERY_TYPE_PIPELINE_STATISTICS, pipelineStatistics must be a valid combination of
        // VkQueryPipelineStatisticFlagBits values
        if ((pCreateInfo->queryType == VK_QUERY_TYPE_PIPELINE_STATISTICS) && (pCreateInfo->pipelineStatistics != 0) &&
            ((pCreateInfo->pipelineStatistics & (~AllVkQueryPipelineStatisticFlagBits)) != 0)) {
            skip |= LogError(device, "VUID-VkQueryPoolCreateInfo-queryType-00792",
                             "vkCreateQueryPool(): if pCreateInfo->queryType is VK_QUERY_TYPE_PIPELINE_STATISTICS, "
                             "pCreateInfo->pipelineStatistics must be a valid combination of VkQueryPipelineStatisticFlagBits "
                             "values.");
        }
        if (pCreateInfo->queryCount == 0) {
            skip |= LogError(device, "VUID-VkQueryPoolCreateInfo-queryCount-02763",
                             "vkCreateQueryPool(): queryCount must be greater than zero.");
        }
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice,
                                                                                   const char *pLayerName, uint32_t *pPropertyCount,
                                                                                   VkExtensionProperties *pProperties) const {
    return validate_array("vkEnumerateDeviceExtensionProperties", "pPropertyCount", "pProperties", pPropertyCount, &pProperties,
                          true, false, false, kVUIDUndefined, "VUID-vkEnumerateDeviceExtensionProperties-pProperties-parameter");
}

void StatelessValidation::PostCallRecordCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo *pCreateInfo,
                                                         const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass,
                                                         VkResult result) {
    if (result != VK_SUCCESS) return;
    RecordRenderPass(*pRenderPass, pCreateInfo);
}

void StatelessValidation::PostCallRecordCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2KHR *pCreateInfo,
                                                             const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass,
                                                             VkResult result) {
    // Track the state necessary for checking vkCreateGraphicsPipeline (subpass usage of depth and color attachments)
    if (result != VK_SUCCESS) return;
    RecordRenderPass(*pRenderPass, pCreateInfo);
}

void StatelessValidation::PostCallRecordDestroyRenderPass(VkDevice device, VkRenderPass renderPass,
                                                          const VkAllocationCallbacks *pAllocator) {
    // Track the state necessary for checking vkCreateGraphicsPipeline (subpass usage of depth and color attachments)
    std::unique_lock<std::mutex> lock(renderpass_map_mutex);
    renderpasses_states.erase(renderPass);
}

bool StatelessValidation::manual_PreCallValidateAllocateMemory(VkDevice device, const VkMemoryAllocateInfo *pAllocateInfo,
                                                               const VkAllocationCallbacks *pAllocator,
                                                               VkDeviceMemory *pMemory) const {
    bool skip = false;

    if (pAllocateInfo) {
        auto chained_prio_struct = lvl_find_in_chain<VkMemoryPriorityAllocateInfoEXT>(pAllocateInfo->pNext);
        if (chained_prio_struct && (chained_prio_struct->priority < 0.0f || chained_prio_struct->priority > 1.0f)) {
            skip |= LogError(device, "VUID-VkMemoryPriorityAllocateInfoEXT-priority-02602",
                             "priority (=%f) must be between `0` and `1`, inclusive.", chained_prio_struct->priority);
        }

        VkMemoryAllocateFlags flags = 0;
        auto flags_info = lvl_find_in_chain<VkMemoryAllocateFlagsInfo>(pAllocateInfo->pNext);
        if (flags_info) {
            flags = flags_info->flags;
        }

        auto opaque_alloc_info = lvl_find_in_chain<VkMemoryOpaqueCaptureAddressAllocateInfoKHR>(pAllocateInfo->pNext);
        if (opaque_alloc_info && opaque_alloc_info->opaqueCaptureAddress != 0) {
            if (!(flags & VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_KHR)) {
                skip |= LogError(device, "VUID-VkMemoryAllocateInfo-opaqueCaptureAddress-03329",
                                 "If opaqueCaptureAddress is non-zero, VkMemoryAllocateFlagsInfo::flags must include "
                                 "VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_KHR.");
            }

#ifdef VK_USE_PLATFORM_WIN32_KHR
            auto import_memory_win32_handle = lvl_find_in_chain<VkImportMemoryWin32HandleInfoKHR>(pAllocateInfo->pNext);
#endif
            auto import_memory_fd = lvl_find_in_chain<VkImportMemoryFdInfoKHR>(pAllocateInfo->pNext);
            auto import_memory_host_pointer = lvl_find_in_chain<VkImportMemoryHostPointerInfoEXT>(pAllocateInfo->pNext);
#ifdef VK_USE_PLATFORM_ANDROID_KHR
            auto import_memory_ahb = lvl_find_in_chain<VkImportAndroidHardwareBufferInfoANDROID>(pAllocateInfo->pNext);
#endif

            if (import_memory_host_pointer) {
                skip |= LogError(
                    device, "VUID-VkMemoryAllocateInfo-pNext-03332",
                    "If the pNext chain includes a VkImportMemoryHostPointerInfoEXT structure, opaqueCaptureAddress must be zero.");
            }
            if (
#ifdef VK_USE_PLATFORM_WIN32_KHR
                (import_memory_win32_handle && import_memory_win32_handle->handleType) ||
#endif
                (import_memory_fd && import_memory_fd->handleType) ||
#ifdef VK_USE_PLATFORM_ANDROID_KHR
                (import_memory_ahb && import_memory_ahb->buffer) ||
#endif
                (import_memory_host_pointer && import_memory_host_pointer->handleType)) {
                skip |= LogError(device, "VUID-VkMemoryAllocateInfo-opaqueCaptureAddress-03333",
                                 "If the parameters define an import operation, opaqueCaptureAddress must be zero.");
            }
        }

        if (flags) {
            VkBool32 capture_replay = false;
            VkBool32 buffer_device_address = false;
            const auto *vulkan_12_features = lvl_find_in_chain<VkPhysicalDeviceVulkan12Features>(device_createinfo_pnext);
            if (vulkan_12_features) {
                capture_replay = vulkan_12_features->bufferDeviceAddressCaptureReplay;
                buffer_device_address = vulkan_12_features->bufferDeviceAddress;
            } else {
                const auto *bda_features =
                    lvl_find_in_chain<VkPhysicalDeviceBufferDeviceAddressFeaturesKHR>(device_createinfo_pnext);
                if (bda_features) {
                    capture_replay = bda_features->bufferDeviceAddressCaptureReplay;
                    buffer_device_address = bda_features->bufferDeviceAddress;
                }
            }
            if ((flags & VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_KHR) && !capture_replay) {
                skip |= LogError(device, "VUID-VkMemoryAllocateInfo-flags-03330",
                                 "If VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_KHR is set, "
                                 "bufferDeviceAddressCaptureReplay must be enabled.");
            }
            if ((flags & VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR) && !buffer_device_address) {
                skip |= LogError(device, "VUID-VkMemoryAllocateInfo-flags-03331",
                                 "If VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR is set, bufferDeviceAddress must be enabled.");
            }
        }
    }
    return skip;
}

bool StatelessValidation::ValidateGeometryTrianglesNV(const VkGeometryTrianglesNV &triangles,
                                                      VkAccelerationStructureNV object_handle, const char *func_name) const {
    bool skip = false;

    if (triangles.vertexFormat != VK_FORMAT_R32G32B32_SFLOAT && triangles.vertexFormat != VK_FORMAT_R16G16B16_SFLOAT &&
        triangles.vertexFormat != VK_FORMAT_R16G16B16_SNORM && triangles.vertexFormat != VK_FORMAT_R32G32_SFLOAT &&
        triangles.vertexFormat != VK_FORMAT_R16G16_SFLOAT && triangles.vertexFormat != VK_FORMAT_R16G16_SNORM) {
        skip |= LogError(object_handle, "VUID-VkGeometryTrianglesNV-vertexFormat-02430", "%s", func_name);
    } else {
        uint32_t vertex_component_size = 0;
        if (triangles.vertexFormat == VK_FORMAT_R32G32B32_SFLOAT || triangles.vertexFormat == VK_FORMAT_R32G32_SFLOAT) {
            vertex_component_size = 4;
        } else if (triangles.vertexFormat == VK_FORMAT_R16G16B16_SFLOAT || triangles.vertexFormat == VK_FORMAT_R16G16B16_SNORM ||
                   triangles.vertexFormat == VK_FORMAT_R16G16_SFLOAT || triangles.vertexFormat == VK_FORMAT_R16G16_SNORM) {
            vertex_component_size = 2;
        }
        if (vertex_component_size > 0 && SafeModulo(triangles.vertexOffset, vertex_component_size) != 0) {
            skip |= LogError(object_handle, "VUID-VkGeometryTrianglesNV-vertexOffset-02429", "%s", func_name);
        }
    }

    if (triangles.indexType != VK_INDEX_TYPE_UINT32 && triangles.indexType != VK_INDEX_TYPE_UINT16 &&
        triangles.indexType != VK_INDEX_TYPE_NONE_NV) {
        skip |= LogError(object_handle, "VUID-VkGeometryTrianglesNV-indexType-02433", "%s", func_name);
    } else {
        uint32_t index_element_size = 0;
        if (triangles.indexType == VK_INDEX_TYPE_UINT32) {
            index_element_size = 4;
        } else if (triangles.indexType == VK_INDEX_TYPE_UINT16) {
            index_element_size = 2;
        }
        if (index_element_size > 0 && SafeModulo(triangles.indexOffset, index_element_size) != 0) {
            skip |= LogError(object_handle, "VUID-VkGeometryTrianglesNV-indexOffset-02432", "%s", func_name);
        }
    }
    if (triangles.indexType == VK_INDEX_TYPE_NONE_NV) {
        if (triangles.indexCount != 0) {
            skip |= LogError(object_handle, "VUID-VkGeometryTrianglesNV-indexCount-02436", "%s", func_name);
        }
        if (triangles.indexData != VK_NULL_HANDLE) {
            skip |= LogError(object_handle, "VUID-VkGeometryTrianglesNV-indexData-02434", "%s", func_name);
        }
    }

    if (SafeModulo(triangles.transformOffset, 16) != 0) {
        skip |= LogError(object_handle, "VUID-VkGeometryTrianglesNV-transformOffset-02438", "%s", func_name);
    }

    return skip;
}

bool StatelessValidation::ValidateGeometryAABBNV(const VkGeometryAABBNV &aabbs, VkAccelerationStructureNV object_handle,
                                                 const char *func_name) const {
    bool skip = false;

    if (SafeModulo(aabbs.offset, 8) != 0) {
        skip |= LogError(object_handle, "VUID-VkGeometryAABBNV-offset-02440", "%s", func_name);
    }
    if (SafeModulo(aabbs.stride, 8) != 0) {
        skip |= LogError(object_handle, "VUID-VkGeometryAABBNV-stride-02441", "%s", func_name);
    }

    return skip;
}

bool StatelessValidation::ValidateGeometryNV(const VkGeometryNV &geometry, VkAccelerationStructureNV object_handle,
                                             const char *func_name) const {
    bool skip = false;
    if (geometry.geometryType == VK_GEOMETRY_TYPE_TRIANGLES_NV) {
        skip = ValidateGeometryTrianglesNV(geometry.geometry.triangles, object_handle, func_name);
    } else if (geometry.geometryType == VK_GEOMETRY_TYPE_AABBS_NV) {
        skip = ValidateGeometryAABBNV(geometry.geometry.aabbs, object_handle, func_name);
    }
    return skip;
}

bool StatelessValidation::ValidateAccelerationStructureInfoNV(const VkAccelerationStructureInfoNV &info,
                                                              VkAccelerationStructureNV object_handle, const char *func_name,
                                                              bool is_cmd) const {
    bool skip = false;
    if (info.type == VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV && info.geometryCount != 0) {
        skip |= LogError(object_handle, "VUID-VkAccelerationStructureInfoNV-type-02425",
                         "VkAccelerationStructureInfoNV: If type is VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV then "
                         "geometryCount must be 0.");
    }
    if (info.type == VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV && info.instanceCount != 0) {
        skip |= LogError(object_handle, "VUID-VkAccelerationStructureInfoNV-type-02426",
                         "VkAccelerationStructureInfoNV: If type is VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV then "
                         "instanceCount must be 0.");
    }
    if (info.flags & VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_NV &&
        info.flags & VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_NV) {
        skip |= LogError(object_handle, "VUID-VkAccelerationStructureInfoNV-flags-02592",
                         "VkAccelerationStructureInfoNV: If flags has the VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_NV"
                         "bit set, then it must not have the VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_NV bit set.");
    }
    if (info.geometryCount > phys_dev_ext_props.ray_tracing_propsNV.maxGeometryCount) {
        skip |= LogError(object_handle,
                         is_cmd ? "VUID-vkCmdBuildAccelerationStructureNV-geometryCount-02241"
                                : "VUID-VkAccelerationStructureInfoNV-geometryCount-02422",
                         "VkAccelerationStructureInfoNV: geometryCount must be less than or equal to "
                         "VkPhysicalDeviceRayTracingPropertiesNV::maxGeometryCount.");
    }
    if (info.instanceCount > phys_dev_ext_props.ray_tracing_propsNV.maxInstanceCount) {
        skip |= LogError(object_handle, "VUID-VkAccelerationStructureInfoNV-instanceCount-02423",
                         "VkAccelerationStructureInfoNV: instanceCount must be less than or equal to "
                         "VkPhysicalDeviceRayTracingPropertiesNV::maxInstanceCount.");
    }
    if (info.type == VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV && info.geometryCount > 0) {
        uint64_t total_triangle_count = 0;
        for (uint32_t i = 0; i < info.geometryCount; i++) {
            const VkGeometryNV &geometry = info.pGeometries[i];

            skip |= ValidateGeometryNV(geometry, object_handle, func_name);

            if (geometry.geometryType != VK_GEOMETRY_TYPE_TRIANGLES_NV) {
                continue;
            }
            total_triangle_count += geometry.geometry.triangles.indexCount / 3;
        }
        if (total_triangle_count > phys_dev_ext_props.ray_tracing_propsNV.maxTriangleCount) {
            skip |= LogError(object_handle, "VUID-VkAccelerationStructureInfoNV-maxTriangleCount-02424",
                             "VkAccelerationStructureInfoNV: The total number of triangles in all geometries must be less than "
                             "or equal to VkPhysicalDeviceRayTracingPropertiesNV::maxTriangleCount.");
        }
    }
    if (info.type == VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV && info.geometryCount > 1) {
        const VkGeometryTypeNV first_geometry_type = info.pGeometries[0].geometryType;
        for (uint32_t i = 1; i < info.geometryCount; i++) {
            const VkGeometryNV &geometry = info.pGeometries[i];
            if (geometry.geometryType != first_geometry_type) {
                skip |= LogError(device, "VUID-VkAccelerationStructureInfoNV-type-02786",
                                 "VkAccelerationStructureInfoNV: info.pGeometries[%d].geometryType does not match "
                                 "info.pGeometries[0].geometryType.",
                                 i);
            }
        }
    }
    for (uint32_t geometry_index = 0; geometry_index < info.geometryCount; ++geometry_index) {
        if (!(info.pGeometries[geometry_index].geometryType == VK_GEOMETRY_TYPE_TRIANGLES_NV ||
              info.pGeometries[geometry_index].geometryType == VK_GEOMETRY_TYPE_AABBS_NV)) {
            skip |= LogError(device, "VUID-VkGeometryNV-geometryType-03503",
                             "VkGeometryNV: geometryType must be VK_GEOMETRY_TYPE_TRIANGLES_NV"
                             "or VK_GEOMETRY_TYPE_AABBS_NV.");
        }
    }
    skip |=
        validate_flags(func_name, "info.flags", "VkBuildAccelerationStructureFlagBitsNV", AllVkBuildAccelerationStructureFlagBitsNV,
                       info.flags, kOptionalFlags, "VUID-VkAccelerationStructureInfoNV-flags-parameter");
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCreateAccelerationStructureNV(
    VkDevice device, const VkAccelerationStructureCreateInfoNV *pCreateInfo, const VkAllocationCallbacks *pAllocator,
    VkAccelerationStructureNV *pAccelerationStructure) const {
    bool skip = false;
    if (pCreateInfo) {
        if ((pCreateInfo->compactedSize != 0) &&
            ((pCreateInfo->info.geometryCount != 0) || (pCreateInfo->info.instanceCount != 0))) {
            skip |= LogError(device, "VUID-VkAccelerationStructureCreateInfoNV-compactedSize-02421",
                             "vkCreateAccelerationStructureNV(): pCreateInfo->compactedSize nonzero (%" PRIu64
                             ") with info.geometryCount (%" PRIu32 ") or info.instanceCount (%" PRIu32 ") nonzero.",
                             pCreateInfo->compactedSize, pCreateInfo->info.geometryCount, pCreateInfo->info.instanceCount);
        }

        skip |= ValidateAccelerationStructureInfoNV(pCreateInfo->info, VkAccelerationStructureNV(0),
                                                    "vkCreateAccelerationStructureNV()", false);
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdBuildAccelerationStructureNV(VkCommandBuffer commandBuffer,
                                                                                const VkAccelerationStructureInfoNV *pInfo,
                                                                                VkBuffer instanceData, VkDeviceSize instanceOffset,
                                                                                VkBool32 update, VkAccelerationStructureNV dst,
                                                                                VkAccelerationStructureNV src, VkBuffer scratch,
                                                                                VkDeviceSize scratchOffset) const {
    bool skip = false;

    if (pInfo != nullptr) {
        skip |= ValidateAccelerationStructureInfoNV(*pInfo, dst, "vkCmdBuildAccelerationStructureNV()", true);
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCreateAccelerationStructureKHR(
    VkDevice device, const VkAccelerationStructureCreateInfoKHR *pCreateInfo, const VkAllocationCallbacks *pAllocator,
    VkAccelerationStructureKHR *pAccelerationStructure) const {
    bool skip = false;

    if (pCreateInfo) {
        for (uint32_t i = 0; i < pCreateInfo->maxGeometryCount; ++i) {
            if (pCreateInfo->type == VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR && pCreateInfo->compactedSize == 0) {
                if (pCreateInfo->pGeometryInfos[i].geometryType != VK_GEOMETRY_TYPE_INSTANCES_KHR) {
                    skip |= LogError(device, "VUID-VkAccelerationStructureCreateInfoKHR-type-03496",
                                     "VkAccelerationStructureCreateInfoKHR: Top-level acceleration structure "
                                     "pGeometryInfos[%d].geometryType must be VK_GEOMETRY_TYPE_INSTANCES_KHR",
                                     i);
                }
            }

            if (pCreateInfo->type == VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR && pCreateInfo->compactedSize == 0) {
                if (pCreateInfo->pGeometryInfos[i].geometryType == VK_GEOMETRY_TYPE_INSTANCES_KHR) {
                    skip |= LogError(device, "VUID-VkAccelerationStructureCreateInfoKHR-type-03497",
                                     "VkAccelerationStructureCreateInfoKHR: Bottom-level acceleration structure "
                                     "pGeometryInfos[%d].geometryType must not be VK_GEOMETRY_TYPE_INSTANCES_KHR",
                                     i);
                }
            }
            if (pCreateInfo->pGeometryInfos[i].geometryType == VK_GEOMETRY_TYPE_TRIANGLES_KHR) {
                if (!(pCreateInfo->pGeometryInfos[i].indexType == VK_INDEX_TYPE_UINT16 ||
                      pCreateInfo->pGeometryInfos[i].indexType == VK_INDEX_TYPE_UINT32 ||
                      pCreateInfo->pGeometryInfos[i].indexType == VK_INDEX_TYPE_NONE_KHR)) {
                    skip |= LogError(
                        device, "VUID-VkAccelerationStructureCreateGeometryTypeInfoKHR-geometryType-03502",
                        "VkAccelerationStructureCreateInfoKHR: If geometryType is VK_GEOMETRY_TYPE_TRIANGLES_KHR, indexType"
                        "must be VK_INDEX_TYPE_UINT16, VK_INDEX_TYPE_UINT32, or VK_INDEX_TYPE_NONE_KHR.");
                }
            }
            if (pCreateInfo->type == VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR) {
                if (pCreateInfo->pGeometryInfos[i].maxPrimitiveCount > phys_dev_ext_props.ray_tracing_propsKHR.maxInstanceCount) {
                    skip |= LogError(device, "VUID-VkAccelerationStructureCreateInfoKHR-type-03492",
                                     "VkAccelerationStructureCreateInfoKHR: If type is VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR"
                                     "then pGeometryInfos->maxPrimitiveCount %d  must be less than or equal to "
                                     "VkPhysicalDeviceRayTracingPropertiesKHR::maxInstanceCount %d.",
                                     pCreateInfo->pGeometryInfos[i].maxPrimitiveCount,
                                     phys_dev_ext_props.ray_tracing_propsKHR.maxInstanceCount);
                }
            }
        }

        if (pCreateInfo->type == VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR && pCreateInfo->compactedSize == 0 &&
            pCreateInfo->maxGeometryCount != 1) {
            skip |= LogError(device, "VUID-VkAccelerationStructureCreateInfoKHR-type-03495",
                             "VkAccelerationStructureCreateInfoKHR: If type is VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR"
                             "and compactedSize is 0, maxGeometryCount must be 1.");
        }
        // or VUID-VkAccelerationStructureCreateInfoKHR-compactedSize-03490
        if (pCreateInfo->compactedSize == 0 && pCreateInfo->maxGeometryCount == 0) {
            skip |= LogError(device, "VUID-VkAccelerationStructureCreateInfoKHR-compactedSize-02993",
                             "VkAccelerationStructureCreateInfoKHR: If compactedSize is 0 then maxGeometryCount must not be 0.");
        }

        if (pCreateInfo->flags & VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR &&
            pCreateInfo->flags & VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR) {
            skip |= LogError(
                device, "VUID-VkAccelerationStructureCreateInfoKHR-flags-03499",
                "VkAccelerationStructureCreateInfoKHR: If flags has the VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR"
                "bit set, then it must not have the VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR bit set.");
        }

        if (pCreateInfo->compactedSize != 0 && pCreateInfo->maxGeometryCount != 0) {
            skip |= LogError(device, "VUID-VkAccelerationStructureCreateInfoKHR-compactedSize-03490",
                             "VkAccelerationStructureCreateInfoKHR: pCreateInfo->compactedSize nonzero (%" PRIu64
                             ") with maxGeometryCount (%" PRIu32 ") nonzero.",
                             pCreateInfo->compactedSize, pCreateInfo->maxGeometryCount);
        }

        if (pCreateInfo->type == VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV && pCreateInfo->maxGeometryCount > 1) {
            const VkGeometryTypeKHR first_geometry_type = pCreateInfo->pGeometryInfos[0].geometryType;
            for (uint32_t i = 1; i < pCreateInfo->maxGeometryCount; i++) {
                const VkGeometryTypeKHR geometry_type = pCreateInfo->pGeometryInfos[i].geometryType;
                if (geometry_type != first_geometry_type) {
                    skip |= LogError(device, "VUID-VkAccelerationStructureCreateInfoKHR-type-03498",
                                     "VkAccelerationStructureCreateInfoKHR: pGeometryInfos[%d].geometryType does not match "
                                     "pGeometryInfos[0].geometryType.",
                                     i);
                }
            }
        }
        if (pCreateInfo->type == VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR &&
            (pCreateInfo->maxGeometryCount > phys_dev_ext_props.ray_tracing_propsKHR.maxGeometryCount)) {
            skip |= LogError(device, "VUID-VkAccelerationStructureCreateInfoKHR-type-03491",
                             "VkAccelerationStructureCreateInfoKHR: If type is VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR"
                             "then maxGeometryCount %d must be less than or equal to VkPhysicalDeviceRayTracingPropertiesKHR "
                             "maxGeometryCount %d.",
                             pCreateInfo->maxGeometryCount, phys_dev_ext_props.ray_tracing_propsKHR.maxGeometryCount);
        }
    }
    const auto *raytracing_features = lvl_find_in_chain<VkPhysicalDeviceRayTracingFeaturesKHR>(device_createinfo_pnext);
    if (!raytracing_features || raytracing_features->rayTracingAccelerationStructureCaptureReplay == VK_FALSE) {
        if (pCreateInfo->deviceAddress != 0) {
            skip |=
                LogError(device, "VUID-VkAccelerationStructureCreateInfoKHR-deviceAddress-03500",
                         "VkAccelerationStructureCreateInfoKHR: If deviceAddress is not 0, "
                         "VkPhysicalDeviceRayTracingFeaturesKHR::rayTracingAccelerationStructureCaptureReplay must be VK_TRUE.");
        }
    }
    if (!raytracing_features || !(raytracing_features->rayQuery == VK_TRUE || raytracing_features->rayTracing == VK_TRUE)) {
        skip |= LogError(device, "VUID-vkCreateAccelerationStructureKHR-rayTracing-03487",
                         "vkCreateAccelerationStructureKHR: The rayTracing or rayQuery feature must be enabled.");
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateGetAccelerationStructureHandleNV(VkDevice device,
                                                                                 VkAccelerationStructureNV accelerationStructure,
                                                                                 size_t dataSize, void *pData) const {
    bool skip = false;
    if (dataSize < 8) {
        skip = LogError(accelerationStructure, "VUID-vkGetAccelerationStructureHandleNV-dataSize-02240",
                        "vkGetAccelerationStructureHandleNV(): dataSize must be greater than or equal to 8.");
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache,
                                                                            uint32_t createInfoCount,
                                                                            const VkRayTracingPipelineCreateInfoNV *pCreateInfos,
                                                                            const VkAllocationCallbacks *pAllocator,
                                                                            VkPipeline *pPipelines) const {
    bool skip = false;

    for (uint32_t i = 0; i < createInfoCount; i++) {
        auto feedback_struct = lvl_find_in_chain<VkPipelineCreationFeedbackCreateInfoEXT>(pCreateInfos[i].pNext);
        if ((feedback_struct != nullptr) && (feedback_struct->pipelineStageCreationFeedbackCount != pCreateInfos[i].stageCount)) {
            skip |= LogError(device, "VUID-VkPipelineCreationFeedbackCreateInfoEXT-pipelineStageCreationFeedbackCount-02969",
                             "vkCreateRayTracingPipelinesNV(): in pCreateInfo[%" PRIu32
                             "], VkPipelineCreationFeedbackEXT::pipelineStageCreationFeedbackCount"
                             "(=%" PRIu32 ") must equal VkRayTracingPipelineCreateInfoNV::stageCount(=%" PRIu32 ").",
                             i, feedback_struct->pipelineStageCreationFeedbackCount, pCreateInfos[i].stageCount);
        }

        const auto *pipeline_cache_contol_features =
            lvl_find_in_chain<VkPhysicalDevicePipelineCreationCacheControlFeaturesEXT>(device_createinfo_pnext);
        if (!pipeline_cache_contol_features || pipeline_cache_contol_features->pipelineCreationCacheControl == VK_FALSE) {
            if (pCreateInfos[i].flags & (VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT_EXT |
                                         VK_PIPELINE_CREATE_EARLY_RETURN_ON_FAILURE_BIT_EXT)) {
                skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoNV-pipelineCreationCacheControl-02905",
                                 "vkCreateRayTracingPipelinesNV(): If the pipelineCreationCacheControl feature is not enabled,"
                                 "flags must not include VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT_EXT or"
                                 "VK_PIPELINE_CREATE_EARLY_RETURN_ON_FAILURE_BIT_EXT.");
            }
        }

        if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_INDIRECT_BINDABLE_BIT_NV) {
            skip |=
                LogError(device, "VUID-VkRayTracingPipelineCreateInfoNV-flags-02904",
                         "vkCreateRayTracingPipelinesNV(): flags must not include VK_PIPELINE_CREATE_INDIRECT_BINDABLE_BIT_NV.");
        }
        if ((pCreateInfos[i].flags & VK_PIPELINE_CREATE_DEFER_COMPILE_BIT_NV) &&
            (pCreateInfos[i].flags & VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT_EXT)) {
            skip |=
                LogError(device, "VUID-VkRayTracingPipelineCreateInfoNV-flags-02957",
                         "vkCreateRayTracingPipelinesNV(): flags must not include both VK_PIPELINE_CREATE_DEFER_COMPILE_BIT_NV and"
                         "VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT_EXT at the same time.");
        }
        if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_DERIVATIVE_BIT) {
            if (pCreateInfos[i].basePipelineIndex != -1) {
                if (pCreateInfos[i].basePipelineHandle != VK_NULL_HANDLE) {
                    skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoNV-flags-03423",
                                     "vkCreateRayTracingPipelinesNV parameter, pCreateInfos->basePipelineHandle, must be "
                                     "VK_NULL_HANDLE if pCreateInfos->flags contains the VK_PIPELINE_CREATE_DERIVATIVE_BIT flag "
                                     "and pCreateInfos->basePipelineIndex is not -1.");
                }
                if (pCreateInfos[i].basePipelineIndex > (int32_t)(i)) {
                    skip |=
                        LogError(device, "VUID-vkCreateRayTracingPipelinesNV-flags-03415",
                                 "vkCreateRayTracingPipelinesNV: If the flags member of any element of pCreateInfos contains the"
                                 "VK_PIPELINE_CREATE_DERIVATIVE_BIT flag, and the basePipelineIndex member of that same element"
                                 "is not -1, basePipelineIndex must be less than the index into pCreateInfos that corresponds to "
                                 "that element.");
                }
            }
            if (pCreateInfos[i].basePipelineHandle == VK_NULL_HANDLE) {
                if (static_cast<uint32_t>(pCreateInfos[i].basePipelineIndex) >= createInfoCount) {
                    skip |=
                        LogError(device, "VUID-VkRayTracingPipelineCreateInfoNV-flags-03422",
                                 "vkCreateRayTracingPipelinesNV if flags contains the VK_PIPELINE_CREATE_DERIVATIVE_BIT and"
                                 "basePipelineHandle is VK_NULL_HANDLE, basePipelineIndex must be a valid index into the calling"
                                 "commands pCreateInfos parameter.");
                }
            } else {
                if (pCreateInfos[i].basePipelineIndex != -1) {
                    skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoNV-flags-03424",
                                     "vkCreateRayTracingPipelinesNV if flags contains the VK_PIPELINE_CREATE_DERIVATIVE_BIT and"
                                     "basePipelineHandle is not VK_NULL_HANDLE, basePipelineIndex must be -1.");
                }
            }
        }
        if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_LIBRARY_BIT_KHR) {
            skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoNV-flags-03456",
                             "vkCreateRayTracingPipelinesNV: flags must not include VK_PIPELINE_CREATE_LIBRARY_BIT_KHR.");
        }
        if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_ANY_HIT_SHADERS_BIT_KHR) {
            skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoNV-flags-03458",
                             "vkCreateRayTracingPipelinesNV: flags must not include "
                             "VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_ANY_HIT_SHADERS_BIT_KHR.");
        }
        if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_CLOSEST_HIT_SHADERS_BIT_KHR) {
            skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoNV-flags-03459",
                             "vkCreateRayTracingPipelinesNV: flags must not include "
                             "VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_CLOSEST_HIT_SHADERS_BIT_KHR.");
        }
        if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_MISS_SHADERS_BIT_KHR) {
            skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoNV-flags-03460",
                             "vkCreateRayTracingPipelinesNV: flags must not include "
                             "VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_MISS_SHADERS_BIT_KHR.");
        }
        if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_INTERSECTION_SHADERS_BIT_KHR) {
            skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoNV-flags-03461",
                             "vkCreateRayTracingPipelinesNV: flags must not include "
                             "VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_INTERSECTION_SHADERS_BIT_KHR.");
        }
        if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_RAY_TRACING_SKIP_AABBS_BIT_KHR) {
            skip |= LogError(
                device, "VUID-VkRayTracingPipelineCreateInfoNV-flags-03462",
                "vkCreateRayTracingPipelinesNV: flags must not include VK_PIPELINE_CREATE_RAY_TRACING_SKIP_AABBS_BIT_KHR.");
        }
        if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_RAY_TRACING_SKIP_TRIANGLES_BIT_KHR) {
            skip |= LogError(
                device, "VUID-VkRayTracingPipelineCreateInfoNV-flags-03463",
                "vkCreateRayTracingPipelinesNV: flags must not include VK_PIPELINE_CREATE_RAY_TRACING_SKIP_TRIANGLES_BIT_KHR .");
        }
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCreateRayTracingPipelinesKHR(VkDevice device, VkPipelineCache pipelineCache,
                                                                             uint32_t createInfoCount,
                                                                             const VkRayTracingPipelineCreateInfoKHR *pCreateInfos,
                                                                             const VkAllocationCallbacks *pAllocator,
                                                                             VkPipeline *pPipelines) const {
    bool skip = false;
    const auto *raytracing_features = lvl_find_in_chain<VkPhysicalDeviceRayTracingFeaturesKHR>(device_createinfo_pnext);
    if (!raytracing_features || raytracing_features->rayTracing == VK_FALSE) {
        skip |= LogError(device, "VUID-vkCreateRayTracingPipelinesKHR-rayTracing-03455",
                         "vkCreateRayTracingPipelinesKHR(): The rayTracing feature must be enabled.");
    }
    for (uint32_t i = 0; i < createInfoCount; i++) {
        auto feedback_struct = lvl_find_in_chain<VkPipelineCreationFeedbackCreateInfoEXT>(pCreateInfos[i].pNext);
        if ((feedback_struct != nullptr) && (feedback_struct->pipelineStageCreationFeedbackCount != pCreateInfos[i].stageCount)) {
            skip |= LogError(device, "VUID-VkPipelineCreationFeedbackCreateInfoEXT-pipelineStageCreationFeedbackCount-02670",
                             "vkCreateRayTracingPipelinesKHR(): in pCreateInfo[%" PRIu32
                             "], VkPipelineCreationFeedbackEXT::pipelineStageCreationFeedbackCount"
                             "(=%" PRIu32 ") must equal VkRayTracingPipelineCreateInfoKHR::stageCount(=%" PRIu32 ").",
                             i, feedback_struct->pipelineStageCreationFeedbackCount, pCreateInfos[i].stageCount);
        }
        const auto *pipeline_cache_contol_features =
            lvl_find_in_chain<VkPhysicalDevicePipelineCreationCacheControlFeaturesEXT>(device_createinfo_pnext);
        if (!pipeline_cache_contol_features || pipeline_cache_contol_features->pipelineCreationCacheControl == VK_FALSE) {
            if (pCreateInfos[i].flags & (VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT_EXT |
                                         VK_PIPELINE_CREATE_EARLY_RETURN_ON_FAILURE_BIT_EXT)) {
                skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoKHR-pipelineCreationCacheControl-02905",
                                 "vkCreateRayTracingPipelinesKHR(): If the pipelineCreationCacheControl feature is not enabled,"
                                 "flags must not include VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT_EXT or"
                                 "VK_PIPELINE_CREATE_EARLY_RETURN_ON_FAILURE_BIT_EXT.");
            }
        }
        if (!raytracing_features || raytracing_features->rayTracingPrimitiveCulling == VK_FALSE) {
            if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_RAY_TRACING_SKIP_AABBS_BIT_KHR) {
                skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoKHR-rayTracingPrimitiveCulling-03472",
                                 "vkCreateRayTracingPipelinesKHR(): If the rayTracingPrimitiveCulling feature is not enabled,"
                                 "flags must not include VK_PIPELINE_CREATE_RAY_TRACING_SKIP_AABBS_BIT_KHR.");
            }
            if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_RAY_TRACING_SKIP_TRIANGLES_BIT_KHR) {
                skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoKHR-rayTracingPrimitiveCulling-03473",
                                 "vkCreateRayTracingPipelinesKHR(): If the rayTracingPrimitiveCulling feature is not enabled,"
                                 "flags must not include VK_PIPELINE_CREATE_RAY_TRACING_SKIP_TRIANGLES_BIT_KHR .");
            }
        }

        if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_INDIRECT_BINDABLE_BIT_NV) {
            skip |=
                LogError(device, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-02904",
                         "vkCreateRayTracingPipelinesKHR(): flags must not include VK_PIPELINE_CREATE_INDIRECT_BINDABLE_BIT_NV.");
        }
        if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_LIBRARY_BIT_KHR) {
            if (pCreateInfos[i].pLibraryInterface == NULL)
                skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-03465",
                                 "If flags includes VK_PIPELINE_CREATE_LIBRARY_BIT_KHR, pLibraryInterface must not be NULL.");
        }
        for (uint32_t group_index = 0; group_index < pCreateInfos[i].groupCount; ++group_index) {
            if ((pCreateInfos[i].pGroups[group_index].type == VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR) ||
                (pCreateInfos[i].pGroups[group_index].type == VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR)) {
                if ((pCreateInfos[i].flags & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_ANY_HIT_SHADERS_BIT_KHR) &&
                    (pCreateInfos[i].pGroups[group_index].anyHitShader == VK_SHADER_UNUSED_KHR)) {
                    skip |= LogError(
                        device, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-03470",
                        "If flags includes VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_ANY_HIT_SHADERS_BIT_KHR,"
                        "for any element of pGroups with a type of VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR"
                        "or VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR, the anyHitShader of that element "
                        "must not be VK_SHADER_UNUSED_KHR");
                }
                if ((pCreateInfos[i].flags & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_CLOSEST_HIT_SHADERS_BIT_KHR) &&
                    (pCreateInfos[i].pGroups[group_index].closestHitShader == VK_SHADER_UNUSED_KHR)) {
                    skip |= LogError(
                        device, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-03471",
                        "If flags includes VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_CLOSEST_HIT_SHADERS_BIT_KHR,"
                        "for any element of pGroups with a type of VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR"
                        "or VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR, the closestHitShader of that "
                        "element must not be VK_SHADER_UNUSED_KHR");
                }
            }
        }
        if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_DERIVATIVE_BIT) {
            if (pCreateInfos[i].basePipelineIndex != -1) {
                if (pCreateInfos[i].basePipelineHandle != VK_NULL_HANDLE) {
                    skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-03423",
                                     "vkCreateRayTracingPipelinesKHR parameter, pCreateInfos->basePipelineHandle, must be "
                                     "VK_NULL_HANDLE if pCreateInfos->flags contains the VK_PIPELINE_CREATE_DERIVATIVE_BIT flag "
                                     "and pCreateInfos->basePipelineIndex is not -1.");
                }
                if (pCreateInfos[i].basePipelineIndex > (int32_t)i) {
                    skip |=
                        LogError(device, "VUID-vkCreateRayTracingPipelinesKHR-flags-03415",
                                 "vkCreateRayTracingPipelinesKHR: If the flags member of any element of pCreateInfos contains the"
                                 "VK_PIPELINE_CREATE_DERIVATIVE_BIT flag, and the basePipelineIndex member of that same element is"
                                 "not -1, basePipelineIndex must be less than the index into pCreateInfos that corresponds to that "
                                 "element.");
                }
            }
            if (pCreateInfos[i].basePipelineHandle == VK_NULL_HANDLE) {
                if (static_cast<uint32_t>(pCreateInfos[i].basePipelineIndex) >= createInfoCount) {
                    skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-03422",
                                     "vkCreateRayTracingPipelinesKHR if flags contains the VK_PIPELINE_CREATE_DERIVATIVE_BIT and"
                                     "basePipelineHandle is VK_NULL_HANDLE, basePipelineIndex (%d) must be a valid into the calling"
                                     "commands pCreateInfos parameter %d.",
                                     pCreateInfos[i].basePipelineIndex, createInfoCount);
                }
            } else {
                if (pCreateInfos[i].basePipelineIndex != -1) {
                    skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-03424",
                                     "vkCreateRayTracingPipelinesKHR if flags contains the VK_PIPELINE_CREATE_DERIVATIVE_BIT and"
                                     "basePipelineHandle is not VK_NULL_HANDLE, basePipelineIndex must be -1.");
                }
            }
        }
        if (pCreateInfos[i].libraries.libraryCount == 0) {
            if (pCreateInfos[i].stageCount == 0) {
                skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoKHR-libraries-02958",
                                 "If libraries.libraryCount is zero, then stageCount must not be zero .");
            }
            if (pCreateInfos[i].groupCount == 0) {
                skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoKHR-libraries-02959",
                                 "If libraries.libraryCount is zero, then groupCount must not be zero .");
            }
        } else {
            if (pCreateInfos[i].pLibraryInterface == NULL) {
                skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoKHR-libraryCount-03466",
                                 "If the libraryCount member of libraries is greater than 0, pLibraryInterface must not be NULL.");
            }
        }
    }

    return skip;
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
bool StatelessValidation::PreCallValidateGetDeviceGroupSurfacePresentModes2EXT(VkDevice device,
                                                                               const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo,
                                                                               VkDeviceGroupPresentModeFlagsKHR *pModes) const {
    bool skip = false;
    if (!device_extensions.vk_khr_swapchain)
        skip |= OutputExtensionError("vkGetDeviceGroupSurfacePresentModes2EXT", VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    if (!device_extensions.vk_khr_get_surface_capabilities_2)
        skip |= OutputExtensionError("vkGetDeviceGroupSurfacePresentModes2EXT", VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);
    if (!device_extensions.vk_khr_surface)
        skip |= OutputExtensionError("vkGetDeviceGroupSurfacePresentModes2EXT", VK_KHR_SURFACE_EXTENSION_NAME);
    if (!device_extensions.vk_khr_get_physical_device_properties_2)
        skip |=
            OutputExtensionError("vkGetDeviceGroupSurfacePresentModes2EXT", VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    if (!device_extensions.vk_ext_full_screen_exclusive)
        skip |= OutputExtensionError("vkGetDeviceGroupSurfacePresentModes2EXT", VK_EXT_FULL_SCREEN_EXCLUSIVE_EXTENSION_NAME);
    skip |= validate_struct_type(
        "vkGetDeviceGroupSurfacePresentModes2EXT", "pSurfaceInfo", "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR",
        pSurfaceInfo, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR, true,
        "VUID-vkGetDeviceGroupSurfacePresentModes2EXT-pSurfaceInfo-parameter", "VUID-VkPhysicalDeviceSurfaceInfo2KHR-sType-sType");
    if (pSurfaceInfo != NULL) {
        const VkStructureType allowed_structs_VkPhysicalDeviceSurfaceInfo2KHR[] = {
            VK_STRUCTURE_TYPE_SURFACE_FULL_SCREEN_EXCLUSIVE_INFO_EXT,
            VK_STRUCTURE_TYPE_SURFACE_FULL_SCREEN_EXCLUSIVE_WIN32_INFO_EXT};

        skip |= validate_struct_pnext("vkGetDeviceGroupSurfacePresentModes2EXT", "pSurfaceInfo->pNext",
                                      "VkSurfaceFullScreenExclusiveInfoEXT, VkSurfaceFullScreenExclusiveWin32InfoEXT",
                                      pSurfaceInfo->pNext, ARRAY_SIZE(allowed_structs_VkPhysicalDeviceSurfaceInfo2KHR),
                                      allowed_structs_VkPhysicalDeviceSurfaceInfo2KHR, GeneratedVulkanHeaderVersion,
                                      "VUID-VkPhysicalDeviceSurfaceInfo2KHR-pNext-pNext",
                                      "VUID-VkPhysicalDeviceSurfaceInfo2KHR-sType-unique");

        skip |= validate_required_handle("vkGetDeviceGroupSurfacePresentModes2EXT", "pSurfaceInfo->surface", pSurfaceInfo->surface);
    }
    return skip;
}
#endif

bool StatelessValidation::manual_PreCallValidateCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo *pCreateInfo,
                                                                  const VkAllocationCallbacks *pAllocator,
                                                                  VkFramebuffer *pFramebuffer) const {
    // Validation for pAttachments which is excluded from the generated validation code due to a 'noautovalidity' tag in vk.xml
    bool skip = false;
    if ((pCreateInfo->flags & VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT_KHR) == 0) {
        skip |= validate_array("vkCreateFramebuffer", "attachmentCount", "pAttachments", pCreateInfo->attachmentCount,
                               &pCreateInfo->pAttachments, false, true, kVUIDUndefined, kVUIDUndefined);
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdSetLineStippleEXT(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor,
                                                                     uint16_t lineStipplePattern) const {
    bool skip = false;

    if (lineStippleFactor < 1 || lineStippleFactor > 256) {
        skip |= LogError(commandBuffer, "VUID-vkCmdSetLineStippleEXT-lineStippleFactor-02776",
                         "vkCmdSetLineStippleEXT::lineStippleFactor=%d is not in [1,256].", lineStippleFactor);
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                   VkDeviceSize offset, VkIndexType indexType) const {
    bool skip = false;

    if (indexType == VK_INDEX_TYPE_NONE_NV) {
        skip |= LogError(commandBuffer, "VUID-vkCmdBindIndexBuffer-indexType-02507",
                         "vkCmdBindIndexBuffer() indexType must not be VK_INDEX_TYPE_NONE_NV.");
    }

    const auto *index_type_uint8_features = lvl_find_in_chain<VkPhysicalDeviceIndexTypeUint8FeaturesEXT>(device_createinfo_pnext);
    if (indexType == VK_INDEX_TYPE_UINT8_EXT && (!index_type_uint8_features || !index_type_uint8_features->indexTypeUint8)) {
        skip |= LogError(commandBuffer, "VUID-vkCmdBindIndexBuffer-indexType-02765",
                         "vkCmdBindIndexBuffer() indexType is VK_INDEX_TYPE_UINT8_EXT but indexTypeUint8 feature is not enabled.");
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding,
                                                                     uint32_t bindingCount, const VkBuffer *pBuffers,
                                                                     const VkDeviceSize *pOffsets) const {
    bool skip = false;
    if (firstBinding > device_limits.maxVertexInputBindings) {
        skip |= LogError(commandBuffer, "VUID-vkCmdBindVertexBuffers-firstBinding-00624",
                         "vkCmdBindVertexBuffers() firstBinding (%u) must be less than maxVertexInputBindings (%u)", firstBinding,
                         device_limits.maxVertexInputBindings);
    } else if ((firstBinding + bindingCount) > device_limits.maxVertexInputBindings) {
        skip |= LogError(commandBuffer, "VUID-vkCmdBindVertexBuffers-firstBinding-00625",
                         "vkCmdBindVertexBuffers() sum of firstBinding (%u) and bindingCount (%u) must be less than "
                         "maxVertexInputBindings (%u)",
                         firstBinding, bindingCount, device_limits.maxVertexInputBindings);
    }

    for (uint32_t i = 0; i < bindingCount; ++i) {
        if (pBuffers[i] == VK_NULL_HANDLE) {
            const auto *robustness2_features = lvl_find_in_chain<VkPhysicalDeviceRobustness2FeaturesEXT>(device_createinfo_pnext);
            if (!(robustness2_features && robustness2_features->nullDescriptor)) {
                skip |= LogError(commandBuffer, "VUID-vkCmdBindVertexBuffers-pBuffers-04001",
                                 "vkCmdBindVertexBuffers() required parameter pBuffers[%d] specified as VK_NULL_HANDLE", i);
            } else {
                if (pOffsets[i] != 0) {
                    skip |= LogError(commandBuffer, "VUID-vkCmdBindVertexBuffers-pBuffers-04002",
                                     "vkCmdBindVertexBuffers() pBuffers[%d] is VK_NULL_HANDLE, but pOffsets[%d] is not 0", i, i);
                }
            }
        }
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateSetDebugUtilsObjectNameEXT(VkDevice device,
                                                                           const VkDebugUtilsObjectNameInfoEXT *pNameInfo) const {
    bool skip = false;
    if (pNameInfo->objectType == VK_OBJECT_TYPE_UNKNOWN) {
        skip |= LogError(device, "VUID-VkDebugUtilsObjectNameInfoEXT-objectType-02589",
                         "vkSetDebugUtilsObjectNameEXT() pNameInfo->objectType cannot be VK_OBJECT_TYPE_UNKNOWN.");
    }
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

bool StatelessValidation::manual_PreCallValidateAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout,
                                                                    VkSemaphore semaphore, VkFence fence,
                                                                    uint32_t *pImageIndex) const {
    bool skip = false;

    if (semaphore == VK_NULL_HANDLE && fence == VK_NULL_HANDLE) {
        skip |= LogError(swapchain, "VUID-vkAcquireNextImageKHR-semaphore-01780",
                         "vkAcquireNextImageKHR: semaphore and fence are both VK_NULL_HANDLE.");
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR *pAcquireInfo,
                                                                     uint32_t *pImageIndex) const {
    bool skip = false;

    if (pAcquireInfo->semaphore == VK_NULL_HANDLE && pAcquireInfo->fence == VK_NULL_HANDLE) {
        skip |= LogError(pAcquireInfo->swapchain, "VUID-VkAcquireNextImageInfoKHR-semaphore-01782",
                         "vkAcquireNextImage2KHR: pAcquireInfo->semaphore and pAcquireInfo->fence are both VK_NULL_HANDLE.");
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdBindTransformFeedbackBuffersEXT(VkCommandBuffer commandBuffer,
                                                                                   uint32_t firstBinding, uint32_t bindingCount,
                                                                                   const VkBuffer *pBuffers,
                                                                                   const VkDeviceSize *pOffsets,
                                                                                   const VkDeviceSize *pSizes) const {
    bool skip = false;

    char const *const cmd_name = "CmdBindTransformFeedbackBuffersEXT";
    for (uint32_t i = 0; i < bindingCount; ++i) {
        if (pOffsets[i] & 3) {
            skip |= LogError(commandBuffer, "VUID-vkCmdBindTransformFeedbackBuffersEXT-pOffsets-02359",
                             "%s: pOffsets[%" PRIu32 "](0x%" PRIxLEAST64 ") is not a multiple of 4.", cmd_name, i, pOffsets[i]);
        }
    }

    if (firstBinding >= phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBuffers) {
        skip |= LogError(commandBuffer, "VUID-vkCmdBindTransformFeedbackBuffersEXT-firstBinding-02356",
                         "%s: The firstBinding(%" PRIu32
                         ") index is greater than or equal to "
                         "VkPhysicalDeviceTransformFeedbackPropertiesEXT::maxTransformFeedbackBuffers(%" PRIu32 ").",
                         cmd_name, firstBinding, phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBuffers);
    }

    if (firstBinding + bindingCount > phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBuffers) {
        skip |=
            LogError(commandBuffer, "VUID-vkCmdBindTransformFeedbackBuffersEXT-firstBinding-02357",
                     "%s: The sum of firstBinding(%" PRIu32 ") and bindCount(%" PRIu32
                     ") is greater than VkPhysicalDeviceTransformFeedbackPropertiesEXT::maxTransformFeedbackBuffers(%" PRIu32 ").",
                     cmd_name, firstBinding, bindingCount, phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBuffers);
    }

    for (uint32_t i = 0; i < bindingCount; ++i) {
        // pSizes is optional and may be nullptr.
        if (pSizes != nullptr) {
            if (pSizes[i] != VK_WHOLE_SIZE &&
                pSizes[i] > phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBufferSize) {
                skip |= LogError(commandBuffer, "VUID-vkCmdBindTransformFeedbackBuffersEXT-pSize-02361",
                                 "%s: pSizes[%" PRIu32 "] (0x%" PRIxLEAST64
                                 ") is not VK_WHOLE_SIZE and is greater than "
                                 "VkPhysicalDeviceTransformFeedbackPropertiesEXT::maxTransformFeedbackBufferSize.",
                                 cmd_name, i, pSizes[i]);
            }
        }
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdBeginTransformFeedbackEXT(VkCommandBuffer commandBuffer,
                                                                             uint32_t firstCounterBuffer,
                                                                             uint32_t counterBufferCount,
                                                                             const VkBuffer *pCounterBuffers,
                                                                             const VkDeviceSize *pCounterBufferOffsets) const {
    bool skip = false;

    char const *const cmd_name = "CmdBeginTransformFeedbackEXT";
    if (firstCounterBuffer >= phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBuffers) {
        skip |= LogError(commandBuffer, "VUID-vkCmdBeginTransformFeedbackEXT-firstCounterBuffer-02368",
                         "%s: The firstCounterBuffer(%" PRIu32
                         ") index is greater than or equal to "
                         "VkPhysicalDeviceTransformFeedbackPropertiesEXT::maxTransformFeedbackBuffers(%" PRIu32 ").",
                         cmd_name, firstCounterBuffer, phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBuffers);
    }

    if (firstCounterBuffer + counterBufferCount > phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBuffers) {
        skip |=
            LogError(commandBuffer, "VUID-vkCmdBeginTransformFeedbackEXT-firstCounterBuffer-02369",
                     "%s: The sum of firstCounterBuffer(%" PRIu32 ") and counterBufferCount(%" PRIu32
                     ") is greater than VkPhysicalDeviceTransformFeedbackPropertiesEXT::maxTransformFeedbackBuffers(%" PRIu32 ").",
                     cmd_name, firstCounterBuffer, counterBufferCount,
                     phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBuffers);
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdEndTransformFeedbackEXT(VkCommandBuffer commandBuffer,
                                                                           uint32_t firstCounterBuffer, uint32_t counterBufferCount,
                                                                           const VkBuffer *pCounterBuffers,
                                                                           const VkDeviceSize *pCounterBufferOffsets) const {
    bool skip = false;

    char const *const cmd_name = "CmdEndTransformFeedbackEXT";
    if (firstCounterBuffer >= phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBuffers) {
        skip |= LogError(commandBuffer, "VUID-vkCmdEndTransformFeedbackEXT-firstCounterBuffer-02376",
                         "%s: The firstCounterBuffer(%" PRIu32
                         ") index is greater than or equal to "
                         "VkPhysicalDeviceTransformFeedbackPropertiesEXT::maxTransformFeedbackBuffers(%" PRIu32 ").",
                         cmd_name, firstCounterBuffer, phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBuffers);
    }

    if (firstCounterBuffer + counterBufferCount > phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBuffers) {
        skip |=
            LogError(commandBuffer, "VUID-vkCmdEndTransformFeedbackEXT-firstCounterBuffer-02377",
                     "%s: The sum of firstCounterBuffer(%" PRIu32 ") and counterBufferCount(%" PRIu32
                     ") is greater than VkPhysicalDeviceTransformFeedbackPropertiesEXT::maxTransformFeedbackBuffers(%" PRIu32 ").",
                     cmd_name, firstCounterBuffer, counterBufferCount,
                     phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBuffers);
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount,
                                                                            uint32_t firstInstance, VkBuffer counterBuffer,
                                                                            VkDeviceSize counterBufferOffset,
                                                                            uint32_t counterOffset, uint32_t vertexStride) const {
    bool skip = false;

    if ((vertexStride <= 0) || (vertexStride > phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBufferDataStride)) {
        skip |= LogError(
            counterBuffer, "VUID-vkCmdDrawIndirectByteCountEXT-vertexStride-02289",
            "vkCmdDrawIndirectByteCountEXT: vertexStride (%d) must be between 0 and maxTransformFeedbackBufferDataStride (%d).",
            vertexStride, phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBufferDataStride);
    }

    return skip;
}

bool StatelessValidation::ValidateCreateSamplerYcbcrConversion(VkDevice device,
                                                               const VkSamplerYcbcrConversionCreateInfo *pCreateInfo,
                                                               const VkAllocationCallbacks *pAllocator,
                                                               VkSamplerYcbcrConversion *pYcbcrConversion,
                                                               const char *apiName) const {
    bool skip = false;

    // Check samplerYcbcrConversion feature is set
    const auto *ycbcr_features = lvl_find_in_chain<VkPhysicalDeviceSamplerYcbcrConversionFeatures>(device_createinfo_pnext);
    if ((ycbcr_features == nullptr) || (ycbcr_features->samplerYcbcrConversion == VK_FALSE)) {
        const auto *vulkan_11_features = lvl_find_in_chain<VkPhysicalDeviceVulkan11Features>(device_createinfo_pnext);
        if ((vulkan_11_features == nullptr) || (vulkan_11_features->samplerYcbcrConversion == VK_FALSE)) {
            skip |= LogError(device, "VUID-vkCreateSamplerYcbcrConversion-None-01648",
                             "%s: samplerYcbcrConversion must be enabled.", apiName);
        }
    }

    const VkFormat format = pCreateInfo->format;
    const VkComponentMapping components = pCreateInfo->components;
    // XChroma Subsampled is same as "the format has a _422 or _420 suffix" from spec
    if (FormatIsXChromaSubsampled(format) == true) {
        if ((components.g != VK_COMPONENT_SWIZZLE_G) && (components.g != VK_COMPONENT_SWIZZLE_IDENTITY)) {
            skip |= LogError(device, "VUID-VkSamplerYcbcrConversionCreateInfo-components-02581",
                             "%s: When using a XChroma subsampled format (%s) the components.g needs to be VK_COMPONENT_SWIZZLE_G "
                             "or VK_COMPONENT_SWIZZLE_IDENTITY, but is %s.",
                             apiName, string_VkFormat(format), string_VkComponentSwizzle(components.g));
        }

        if ((components.a != VK_COMPONENT_SWIZZLE_A) && (components.a != VK_COMPONENT_SWIZZLE_IDENTITY) &&
            (components.a != VK_COMPONENT_SWIZZLE_ONE) && (components.a != VK_COMPONENT_SWIZZLE_ZERO)) {
            skip |=
                LogError(device, "VUID-VkSamplerYcbcrConversionCreateInfo-components-02582",
                         "%s: When using a XChroma subsampled format (%s) the components.a needs to be VK_COMPONENT_SWIZZLE_A or "
                         "VK_COMPONENT_SWIZZLE_IDENTITY or VK_COMPONENT_SWIZZLE_ONE or VK_COMPONENT_SWIZZLE_ZERO, but is %s.",
                         apiName, string_VkFormat(format), string_VkComponentSwizzle(components.a));
        }

        if ((components.r != VK_COMPONENT_SWIZZLE_R) && (components.r != VK_COMPONENT_SWIZZLE_IDENTITY) &&
            (components.r != VK_COMPONENT_SWIZZLE_B)) {
            skip |= LogError(device, "VUID-VkSamplerYcbcrConversionCreateInfo-components-02583",
                             "%s: When using a XChroma subsampled format (%s) the components.r needs to be VK_COMPONENT_SWIZZLE_R "
                             "or VK_COMPONENT_SWIZZLE_IDENTITY or VK_COMPONENT_SWIZZLE_B, but is %s.",
                             apiName, string_VkFormat(format), string_VkComponentSwizzle(components.r));
        }

        if ((components.b != VK_COMPONENT_SWIZZLE_B) && (components.b != VK_COMPONENT_SWIZZLE_IDENTITY) &&
            (components.b != VK_COMPONENT_SWIZZLE_R)) {
            skip |= LogError(device, "VUID-VkSamplerYcbcrConversionCreateInfo-components-02584",
                             "%s: When using a XChroma subsampled format (%s) the components.b needs to be VK_COMPONENT_SWIZZLE_B "
                             "or VK_COMPONENT_SWIZZLE_IDENTITY or VK_COMPONENT_SWIZZLE_R, but is %s.",
                             apiName, string_VkFormat(format), string_VkComponentSwizzle(components.b));
        }

        // If one is identity, both need to be
        const bool rIdentity = ((components.r == VK_COMPONENT_SWIZZLE_R) || (components.r == VK_COMPONENT_SWIZZLE_IDENTITY));
        const bool bIdentity = ((components.b == VK_COMPONENT_SWIZZLE_B) || (components.b == VK_COMPONENT_SWIZZLE_IDENTITY));
        if ((rIdentity != bIdentity) && ((rIdentity == true) || (bIdentity == true))) {
            skip |= LogError(device, "VUID-VkSamplerYcbcrConversionCreateInfo-components-02585",
                             "%s: When using a XChroma subsampled format (%s) if either the components.r (%s) or components.b (%s) "
                             "are an identity swizzle, then both need to be an identity swizzle.",
                             apiName, string_VkFormat(format), string_VkComponentSwizzle(components.r),
                             string_VkComponentSwizzle(components.b));
        }
    }

    if (pCreateInfo->ycbcrModel != VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY) {
        // Checks same VU multiple ways in order to give a more useful error message
        const char *vuid = "VUID-VkSamplerYcbcrConversionCreateInfo-ycbcrModel-01655";
        if ((components.r == VK_COMPONENT_SWIZZLE_ONE) || (components.r == VK_COMPONENT_SWIZZLE_ZERO) ||
            (components.g == VK_COMPONENT_SWIZZLE_ONE) || (components.g == VK_COMPONENT_SWIZZLE_ZERO) ||
            (components.b == VK_COMPONENT_SWIZZLE_ONE) || (components.b == VK_COMPONENT_SWIZZLE_ZERO)) {
            skip |=
                LogError(device, vuid,
                         "%s: The ycbcrModel is not VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY so components.r (%s), "
                         "components.g (%s), nor components.b (%s) can't be VK_COMPONENT_SWIZZLE_ZERO or VK_COMPONENT_SWIZZLE_ONE.",
                         apiName, string_VkComponentSwizzle(components.r), string_VkComponentSwizzle(components.g),
                         string_VkComponentSwizzle(components.b));
        }

        // "must not correspond to a channel which contains zero or one as a consequence of conversion to RGBA"
        // 4 channel format = no issue
        // 3 = no [a]
        // 2 = no [b,a]
        // 1 = no [g,b,a]
        // depth/stencil = no [g,b,a] (shouldn't ever occur, but no VU preventing it)
        const uint32_t channels = (FormatIsDepthOrStencil(format) == true) ? 1 : FormatChannelCount(format);

        if ((channels < 4) && ((components.r == VK_COMPONENT_SWIZZLE_A) || (components.g == VK_COMPONENT_SWIZZLE_A) ||
                               (components.b == VK_COMPONENT_SWIZZLE_A))) {
            skip |= LogError(device, vuid,
                             "%s: The ycbcrModel is not VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY so components.r (%s), "
                             "components.g (%s), or components.b (%s) can't be VK_COMPONENT_SWIZZLE_A.",
                             apiName, string_VkComponentSwizzle(components.r), string_VkComponentSwizzle(components.g),
                             string_VkComponentSwizzle(components.b));
        } else if ((channels < 3) &&
                   ((components.r == VK_COMPONENT_SWIZZLE_B) || (components.g == VK_COMPONENT_SWIZZLE_B) ||
                    (components.b == VK_COMPONENT_SWIZZLE_B) || (components.b == VK_COMPONENT_SWIZZLE_IDENTITY))) {
            skip |= LogError(device, vuid,
                             "%s: The ycbcrModel is not VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY so components.r (%s), "
                             "components.g (%s), or components.b (%s) can't be VK_COMPONENT_SWIZZLE_B "
                             "(components.b also can't be VK_COMPONENT_SWIZZLE_IDENTITY).",
                             apiName, string_VkComponentSwizzle(components.r), string_VkComponentSwizzle(components.g),
                             string_VkComponentSwizzle(components.b));
        } else if ((channels < 2) &&
                   ((components.r == VK_COMPONENT_SWIZZLE_G) || (components.g == VK_COMPONENT_SWIZZLE_G) ||
                    (components.g == VK_COMPONENT_SWIZZLE_IDENTITY) || (components.b == VK_COMPONENT_SWIZZLE_G))) {
            skip |= LogError(device, vuid,
                             "%s: The ycbcrModel is not VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY so components.r (%s), "
                             "components.g (%s), or components.b (%s) can't be VK_COMPONENT_SWIZZLE_G "
                             "(components.g also can't be VK_COMPONENT_SWIZZLE_IDENTITY).",
                             apiName, string_VkComponentSwizzle(components.r), string_VkComponentSwizzle(components.g),
                             string_VkComponentSwizzle(components.b));
        }
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCreateSamplerYcbcrConversion(VkDevice device,
                                                                             const VkSamplerYcbcrConversionCreateInfo *pCreateInfo,
                                                                             const VkAllocationCallbacks *pAllocator,
                                                                             VkSamplerYcbcrConversion *pYcbcrConversion) const {
    return ValidateCreateSamplerYcbcrConversion(device, pCreateInfo, pAllocator, pYcbcrConversion,
                                                "vkCreateSamplerYcbcrConversion");
}

bool StatelessValidation::manual_PreCallValidateCreateSamplerYcbcrConversionKHR(
    VkDevice device, const VkSamplerYcbcrConversionCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator,
    VkSamplerYcbcrConversion *pYcbcrConversion) const {
    return ValidateCreateSamplerYcbcrConversion(device, pCreateInfo, pAllocator, pYcbcrConversion,
                                                "vkCreateSamplerYcbcrConversionKHR");
}

bool StatelessValidation::manual_PreCallValidateImportSemaphoreFdKHR(
    VkDevice device, const VkImportSemaphoreFdInfoKHR *pImportSemaphoreFdInfo) const {
    bool skip = false;
    VkExternalSemaphoreHandleTypeFlags supported_handle_types =
        VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT | VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT;

    if (0 == (pImportSemaphoreFdInfo->handleType & supported_handle_types)) {
        skip |= LogError(device, "VUID-VkImportSemaphoreFdInfoKHR-handleType-01143",
                         "vkImportSemaphoreFdKHR() to semaphore %s handleType %s is not one of the supported handleTypes (%s).",
                         report_data->FormatHandle(pImportSemaphoreFdInfo->semaphore).c_str(),
                         string_VkExternalSemaphoreHandleTypeFlagBits(pImportSemaphoreFdInfo->handleType),
                         string_VkExternalSemaphoreHandleTypeFlags(supported_handle_types).c_str());
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCopyAccelerationStructureToMemoryKHR(
    VkDevice device, const VkCopyAccelerationStructureToMemoryInfoKHR *pInfo) const {
    bool skip = false;
    const auto *raytracing_features = lvl_find_in_chain<VkPhysicalDeviceRayTracingFeaturesKHR>(device_createinfo_pnext);
    if (!raytracing_features || raytracing_features->rayTracingHostAccelerationStructureCommands == VK_FALSE) {
        skip |=
            LogError(device, "", "VUID-vkCopyAccelerationStructureToMemoryKHR-rayTracingHostAccelerationStructureCommands-03447",
                     "vkCopyAccelerationStructureToMemoryKHR: the "
                     "VkPhysicalDeviceRayTracingFeaturesKHR::rayTracingHostAccelerationStructureCommands feature must be enabled.");
    }
    if (pInfo->mode != VK_COPY_ACCELERATION_STRUCTURE_MODE_SERIALIZE_KHR) {
        skip |= LogError(device, "VUID-VkCopyAccelerationStructureToMemoryInfoKHR-mode-03412",
                         "vkCopyAccelerationStructureToMemoryKHR: mode must be VK_COPY_ACCELERATION_STRUCTURE_MODE_SERIALIZE_KHR.");
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdCopyAccelerationStructureToMemoryKHR(
    VkCommandBuffer commandBuffer, const VkCopyAccelerationStructureToMemoryInfoKHR *pInfo) const {
    bool skip = false;
    if (pInfo->mode != VK_COPY_ACCELERATION_STRUCTURE_MODE_SERIALIZE_KHR) {
        skip |=  // to update VUID to VkCmdCopyAccelerationStructureToMemoryInfoKHR after spec update
            LogError(commandBuffer, "VUID-VkCopyAccelerationStructureToMemoryInfoKHR-mode-03412",
                     "vkCmdCopyAccelerationStructureToMemoryKHR: mode must be VK_COPY_ACCELERATION_STRUCTURE_MODE_SERIALIZE_KHR.");
    }
    const auto *pnext_struct = lvl_find_in_chain<VkDeferredOperationInfoKHR>(pInfo->pNext);
    if (pnext_struct) {
        skip |= LogError(
            commandBuffer, "VUID-vkCmdCopyAccelerationStructureToMemoryKHR-pNext-03560",
            "vkCmdCopyAccelerationStructureToMemoryKHR: The VkDeferredOperationInfoKHR structure must not be included in the"
            "pNext chain of the VkCopyAccelerationStructureToMemoryInfoKHR structure.");
    }
    return skip;
}

bool StatelessValidation::ValidateCopyAccelerationStructureInfoKHR(const VkCopyAccelerationStructureInfoKHR *pInfo,
                                                                   const char *api_name) const {
    bool skip = false;
    if (!(pInfo->mode == VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_KHR ||
          pInfo->mode == VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_KHR)) {
        skip |= LogError(device, "VUID-VkCopyAccelerationStructureInfoKHR-mode-03410",
                         "(%s): mode must be VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_KHR"
                         "or VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_KHR.",
                         api_name);
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCopyAccelerationStructureKHR(
    VkDevice device, const VkCopyAccelerationStructureInfoKHR *pInfo) const {
    bool skip = false;
    skip |= ValidateCopyAccelerationStructureInfoKHR(pInfo, "vkCopyAccelerationStructureKHR()");
    const auto *raytracing_features = lvl_find_in_chain<VkPhysicalDeviceRayTracingFeaturesKHR>(device_createinfo_pnext);
    if (!raytracing_features || raytracing_features->rayTracingHostAccelerationStructureCommands == VK_FALSE) {
        skip |= LogError(
            device, "VUID-vkCopyAccelerationStructureKHR-rayTracingHostAccelerationStructureCommands-03441",
            "vkCopyAccelerationStructureKHR(): the "
            "VkPhysicalDeviceRayTracingFeaturesKHR::rayTracingHostAccelerationStructureCommands feature must be enabled .");
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdCopyAccelerationStructureKHR(
    VkCommandBuffer commandBuffer, const VkCopyAccelerationStructureInfoKHR *pInfo) const {
    bool skip = false;
    const auto *pnext_struct = lvl_find_in_chain<VkDeferredOperationInfoKHR>(pInfo->pNext);
    if (pnext_struct) {
        skip |= LogError(device, "VUID-vkCmdCopyAccelerationStructureKHR-pNext-03557",
                         "vkCmdCopyAccelerationStructureKHR(): The VkDeferredOperationInfoKHR structure must not be included in "
                         "the pNext chain of the VkCopyAccelerationStructureInfoKHR structure.");
    }
    skip |= ValidateCopyAccelerationStructureInfoKHR(pInfo, "vkCmdCopyAccelerationStructureKHR()");
    return skip;
}

bool StatelessValidation::ValidateCopyMemoryToAccelerationStructureInfoKHR(const VkCopyMemoryToAccelerationStructureInfoKHR *pInfo,
                                                                           const char *api_name, bool is_cmd) const {
    bool skip = false;
    if (pInfo->mode != VK_COPY_ACCELERATION_STRUCTURE_MODE_DESERIALIZE_KHR) {
        skip |= LogError(device,
                         is_cmd ? "VUID-vkCmdCopyMemoryToAccelerationStructureKHR-mode-03413"
                                : "VUID-VkCopyMemoryToAccelerationStructureInfoKHR-mode-03413",
                         "(%s): mode must be VK_COPY_ACCELERATION_STRUCTURE_MODE_DESERIALIZE_KHR.", api_name);
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCopyMemoryToAccelerationStructureKHR(
    VkDevice device, const VkCopyMemoryToAccelerationStructureInfoKHR *pInfo) const {
    bool skip = false;
    skip |= ValidateCopyMemoryToAccelerationStructureInfoKHR(pInfo, "vkCopyMemoryToAccelerationStructureKHR()", true);
    const auto *raytracing_features = lvl_find_in_chain<VkPhysicalDeviceRayTracingFeaturesKHR>(device_createinfo_pnext);
    if (!raytracing_features || raytracing_features->rayTracingHostAccelerationStructureCommands == VK_FALSE) {
        skip |=
            LogError(device, "VUID-vkCopyMemoryToAccelerationStructureKHR-rayTracingHostAccelerationStructureCommands-03444",
                     "vkCopyMemoryToAccelerationStructureKHR() :the "
                     "VkPhysicalDeviceRayTracingFeaturesKHR::rayTracingHostAccelerationStructureCommands feature must be enabled.");
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdCopyMemoryToAccelerationStructureKHR(
    VkCommandBuffer commandBuffer, const VkCopyMemoryToAccelerationStructureInfoKHR *pInfo) const {
    bool skip = false;
    const auto *pnext_struct = lvl_find_in_chain<VkDeferredOperationInfoKHR>(pInfo->pNext);
    if (pnext_struct) {
        skip |= LogError(device, "VUID-vkCmdCopyMemoryToAccelerationStructureKHR-pNext-03564",
                         "vkCmdCopyMemoryToAccelerationStructureKHR: The VkDeferredOperationInfoKHR structure must"
                         "not be included in the pNext chain of the VkCopyMemoryToAccelerationStructureInfoKHR structure.");
    }
    skip |= ValidateCopyMemoryToAccelerationStructureInfoKHR(pInfo, "vkCmdCopyMemoryToAccelerationStructureKHR()", false);
    return skip;
}
bool StatelessValidation::manual_PreCallValidateCmdWriteAccelerationStructuresPropertiesKHR(
    VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount, const VkAccelerationStructureKHR *pAccelerationStructures,
    VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery) const {
    bool skip = false;
    if (!(queryType == VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR ||
          queryType == VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR)) {
        skip |= LogError(device, "VUID-vkCmdWriteAccelerationStructuresPropertiesKHR-queryType-03432",
                         "vkCmdWriteAccelerationStructuresPropertiesKHR: queryType must be "
                         "VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR or "
                         "VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR.");
    }
    return skip;
}
bool StatelessValidation::manual_PreCallValidateWriteAccelerationStructuresPropertiesKHR(
    VkDevice device, uint32_t accelerationStructureCount, const VkAccelerationStructureKHR *pAccelerationStructures,
    VkQueryType queryType, size_t dataSize, void *pData, size_t stride) const {
    bool skip = false;
    if (dataSize < accelerationStructureCount * stride) {
        skip |= LogError(device, "VUID-vkWriteAccelerationStructuresPropertiesKHR-dataSize-03452",
                         "vkWriteAccelerationStructuresPropertiesKHR: dataSize (%zu) must be greater than or equal to "
                         "accelerationStructureCount (%d) *stride(%zu).",
                         dataSize, accelerationStructureCount, stride);
    }
    if (!(queryType == VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR ||
          queryType == VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR)) {
        skip |= LogError(device, "VUID-vkWriteAccelerationStructuresPropertiesKHR-queryType-03432",
                         "vkWriteAccelerationStructuresPropertiesKHR: queryType must be "
                         "VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR or "
                         "VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR.");
    }
    if (queryType == VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR) {
        if (SafeModulo(stride, sizeof(VkDeviceSize)) != 0) {
            skip |= LogError(device, "VUID-vkWriteAccelerationStructuresPropertiesKHR-queryType-03448",
                             "vkWriteAccelerationStructuresPropertiesKHR: If queryType is "
                             "VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR,"
                             "then stride (%zu) must be a multiple of the size of VkDeviceSize",
                             stride);
        }
    }
    if (queryType == VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR) {
        if (SafeModulo(stride, sizeof(VkDeviceSize)) != 0) {
            skip |= LogError(device, "VUID-vkWriteAccelerationStructuresPropertiesKHR-queryType-03450",
                             "vkWriteAccelerationStructuresPropertiesKHR: If queryType is "
                             "VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR,"
                             "then stride (%zu) must be a multiple of the size of VkDeviceSize",
                             stride);
        }
    }
    const auto *raytracing_features = lvl_find_in_chain<VkPhysicalDeviceRayTracingFeaturesKHR>(device_createinfo_pnext);
    if (!raytracing_features || raytracing_features->rayTracingHostAccelerationStructureCommands == VK_FALSE) {
        skip |=
            LogError(device, "VUID-vkWriteAccelerationStructuresPropertiesKHR-rayTracingHostAccelerationStructureCommands-03454",
                     "vkWriteAccelerationStructuresPropertiesKHR: the "
                     "vkPhysicalDeviceRayTracingFeaturesKHR::rayTracingHostAccelerationStructureCommands"
                     "feature must be enabled ");
    }
    return skip;
}
bool StatelessValidation::manual_PreCallValidateGetRayTracingCaptureReplayShaderGroupHandlesKHR(
    VkDevice device, VkPipeline pipeline, uint32_t firstGroup, uint32_t groupCount, size_t dataSize, void *pData) const {
    bool skip = false;
    const auto *raytracing_features = lvl_find_in_chain<VkPhysicalDeviceRayTracingFeaturesKHR>(device_createinfo_pnext);
    if (!raytracing_features || raytracing_features->rayTracingShaderGroupHandleCaptureReplay == VK_FALSE) {
        skip |= LogError(device,
                         "VUID-vkGetRayTracingCaptureReplayShaderGroupHandlesKHR-rayTracingShaderGroupHandleCaptureReplay-03485",
                         "vkGetRayTracingCaptureReplayShaderGroupHandlesKHR: "
                         "VkPhysicalDeviceRayTracingFeaturesKHR::rayTracingShaderGroupHandleCaptureReplay"
                         "must be enabled to call this function.");
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdTraceRaysKHR(VkCommandBuffer commandBuffer,
                                                                const VkStridedBufferRegionKHR *pRaygenShaderBindingTable,
                                                                const VkStridedBufferRegionKHR *pMissShaderBindingTable,
                                                                const VkStridedBufferRegionKHR *pHitShaderBindingTable,
                                                                const VkStridedBufferRegionKHR *pCallableShaderBindingTable,
                                                                uint32_t width, uint32_t height, uint32_t depth) const {
    bool skip = false;
    if (SafeModulo(pCallableShaderBindingTable->offset, phys_dev_ext_props.ray_tracing_propsKHR.shaderGroupBaseAlignment) != 0) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysKHR-offset-04038",
                         "vkCmdTraceRaysKHR: The offset member of pCallableShaderBindingTable"
                         "must be a multiple of VkPhysicalDeviceRayTracingPropertiesKHR::shaderGroupBaseAlignment.");
    }
    if (SafeModulo(pCallableShaderBindingTable->stride, phys_dev_ext_props.ray_tracing_propsKHR.shaderGroupHandleSize) != 0) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysKHR-stride-04040",
                         "vkCmdTraceRaysKHR: The stride member of pCallableShaderBindingTable must be a multiple"
                         "of VkPhysicalDeviceRayTracingPropertiesKHR::shaderGroupHandleSize.");
    }
    if (pCallableShaderBindingTable->stride > phys_dev_ext_props.ray_tracing_propsKHR.maxShaderGroupStride) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysKHR-stride-04041",
                         "vkCmdTraceRaysKHR: The stride member of pCallableShaderBindingTable must be"
                         "less than or equal to VkPhysicalDeviceRayTracingPropertiesKHR::maxShaderGroupStride.");
    }
    // hitShader
    if (SafeModulo(pHitShaderBindingTable->offset, phys_dev_ext_props.ray_tracing_propsKHR.shaderGroupBaseAlignment) != 0) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysKHR-offset-04032",
                         "vkCmdTraceRaysKHR: The offset member of pHitShaderBindingTable must be a multiple"
                         "of VkPhysicalDeviceRayTracingPropertiesKHR::shaderGroupBaseAlignment.");
    }
    if (SafeModulo(pHitShaderBindingTable->stride, phys_dev_ext_props.ray_tracing_propsKHR.shaderGroupHandleSize) != 0) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysKHR-stride-04034",
                         "vkCmdTraceRaysKHR: The stride member of pHitShaderBindingTable must be a multiple"
                         "of VkPhysicalDeviceRayTracingPropertiesKHR::shaderGroupHandleSize.");
    }
    if (pHitShaderBindingTable->stride > phys_dev_ext_props.ray_tracing_propsKHR.maxShaderGroupStride) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysKHR-stride-04035",
                         "vkCmdTraceRaysKHR: The stride member of pHitShaderBindingTable must be"
                         "less than or equal to VkPhysicalDeviceRayTracingPropertiesKHR::maxShaderGroupStride.");
    }

    // missShader
    if (SafeModulo(pMissShaderBindingTable->offset, phys_dev_ext_props.ray_tracing_propsKHR.shaderGroupBaseAlignment) != 0) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysKHR-offset-04026",
                         "vkCmdTraceRaysKHR: The offset member of pMissShaderBindingTable must be a multiple"
                         "of VkPhysicalDeviceRayTracingPropertiesKHR::shaderGroupBaseAlignment.");
    }
    if (SafeModulo(pMissShaderBindingTable->stride, phys_dev_ext_props.ray_tracing_propsKHR.shaderGroupHandleSize) != 0) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysKHR-stride-04028",
                         "vkCmdTraceRaysKHR: The stride member of pMissShaderBindingTable must be a multiple"
                         "of VkPhysicalDeviceRayTracingPropertiesKHR::shaderGroupHandleSize.");
    }
    if (pMissShaderBindingTable->stride > phys_dev_ext_props.ray_tracing_propsKHR.maxShaderGroupStride) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysKHR-stride-04029",
                         "vkCmdTraceRaysKHR: The stride member of pMissShaderBindingTable must be"
                         "less than or equal to VkPhysicalDeviceRayTracingPropertiesKHR::maxShaderGroupStride.");
    }

    // raygenShader
    if (SafeModulo(pRaygenShaderBindingTable->offset, phys_dev_ext_props.ray_tracing_propsKHR.shaderGroupBaseAlignment) != 0) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysKHR-pRayGenShaderBindingTable-04021",
                         "vkCmdTraceRaysKHR: pRayGenShaderBindingTable->offset must be a multiple"
                         "of VkPhysicalDeviceRayTracingPropertiesKHR::shaderGroupBaseAlignment.");
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer,
                                                                        const VkStridedBufferRegionKHR *pRaygenShaderBindingTable,
                                                                        const VkStridedBufferRegionKHR *pMissShaderBindingTable,
                                                                        const VkStridedBufferRegionKHR *pHitShaderBindingTable,
                                                                        const VkStridedBufferRegionKHR *pCallableShaderBindingTable,
                                                                        VkBuffer buffer, VkDeviceSize offset) const {
    bool skip = false;
    const auto *raytracing_features = lvl_find_in_chain<VkPhysicalDeviceRayTracingFeaturesKHR>(device_createinfo_pnext);
    if (!raytracing_features || raytracing_features->rayTracingIndirectTraceRays == VK_FALSE) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysIndirectKHR-rayTracingIndirectTraceRays-03518",
                         "vkCmdTraceRaysIndirectKHR: the VkPhysicalDeviceRayTracingFeaturesKHR::rayTracingIndirectTraceRays "
                         "feature must be enabled.");
    }
    if (SafeModulo(pCallableShaderBindingTable->offset, phys_dev_ext_props.ray_tracing_propsKHR.shaderGroupBaseAlignment) != 0) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysIndirectKHR-offset-04038",
                         "vkCmdTraceRaysIndirectKHR: The offset member of pCallableShaderBindingTable"
                         "must be a multiple of VkPhysicalDeviceRayTracingPropertiesKHR::shaderGroupBaseAlignment.");
    }
    if (SafeModulo(pCallableShaderBindingTable->stride, phys_dev_ext_props.ray_tracing_propsKHR.shaderGroupHandleSize) != 0) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysIndirectKHR-stride-04040",
                         "vkCmdTraceRaysIndirectKHR: The stride member of pCallableShaderBindingTable must be a multiple"
                         "of VkPhysicalDeviceRayTracingPropertiesKHR::shaderGroupHandleSize.");
    }
    if (pCallableShaderBindingTable->stride > phys_dev_ext_props.ray_tracing_propsKHR.maxShaderGroupStride) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysIndirectKHR-stride-04041",
                         "vkCmdTraceRaysIndirectKHR: The stride member of pCallableShaderBindingTable must be"
                         "less than or equal to VkPhysicalDeviceRayTracingPropertiesKHR::maxShaderGroupStride.");
    }
    // hitShader
    if (SafeModulo(pHitShaderBindingTable->offset, phys_dev_ext_props.ray_tracing_propsKHR.shaderGroupBaseAlignment) != 0) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysIndirectKHR-offset-04032",
                         "vkCmdTraceRaysIndirectKHR: The offset member of pHitShaderBindingTable must be a multiple"
                         "of VkPhysicalDeviceRayTracingPropertiesKHR::shaderGroupBaseAlignment.");
    }
    if (SafeModulo(pHitShaderBindingTable->stride, phys_dev_ext_props.ray_tracing_propsKHR.shaderGroupHandleSize) != 0) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysIndirectKHR-stride-04034",
                         "vkCmdTraceRaysIndirectKHR: The stride member of pHitShaderBindingTable must be a multiple"
                         "of VkPhysicalDeviceRayTracingPropertiesKHR::shaderGroupHandleSize.");
    }
    if (pHitShaderBindingTable->stride > phys_dev_ext_props.ray_tracing_propsKHR.maxShaderGroupStride) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysIndirectKHR-stride-04035",
                         "vkCmdTraceRaysIndirectKHR: The stride member of pHitShaderBindingTable must be"
                         "less than or equal to VkPhysicalDeviceRayTracingPropertiesKHR::maxShaderGroupStride.");
    }

    // missShader
    if (SafeModulo(pMissShaderBindingTable->offset, phys_dev_ext_props.ray_tracing_propsKHR.shaderGroupBaseAlignment) != 0) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysIndirectKHR-offset-04026",
                         "vkCmdTraceRaysIndirectKHR: The offset member of pMissShaderBindingTable must be a multiple"
                         "of VkPhysicalDeviceRayTracingPropertiesKHR::shaderGroupBaseAlignment.");
    }
    if (SafeModulo(pMissShaderBindingTable->stride, phys_dev_ext_props.ray_tracing_propsKHR.shaderGroupHandleSize) != 0) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysIndirectKHR-stride-04028",
                         "vkCmdTraceRaysIndirectKHR: The stride member of pMissShaderBindingTable must be a multiple"
                         "of VkPhysicalDeviceRayTracingPropertiesKHR::shaderGroupHandleSize.");
    }
    if (pMissShaderBindingTable->stride > phys_dev_ext_props.ray_tracing_propsKHR.maxShaderGroupStride) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysIndirectKHR-stride-04029",
                         "vkCmdTraceRaysIndirectKHR: The stride member of pMissShaderBindingTable must be"
                         "less than or equal to VkPhysicalDeviceRayTracingPropertiesKHR::maxShaderGroupStride.");
    }

    // raygenShader
    if (SafeModulo(pRaygenShaderBindingTable->offset, phys_dev_ext_props.ray_tracing_propsKHR.shaderGroupBaseAlignment) != 0) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysIndirectKHR-pRayGenShaderBindingTable-04021",
                         "vkCmdTraceRaysIndirectKHR: pRayGenShaderBindingTable->offset must be a multiple"
                         "of VkPhysicalDeviceRayTracingPropertiesKHR::shaderGroupBaseAlignment.");
    }
    return skip;
}
bool StatelessValidation::manual_PreCallValidateCmdTraceRaysNV(
    VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer, VkDeviceSize raygenShaderBindingOffset,
    VkBuffer missShaderBindingTableBuffer, VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride,
    VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset, VkDeviceSize hitShaderBindingStride,
    VkBuffer callableShaderBindingTableBuffer, VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride,
    uint32_t width, uint32_t height, uint32_t depth) const {
    bool skip = false;
    if (SafeModulo(callableShaderBindingOffset, phys_dev_ext_props.ray_tracing_propsNV.shaderGroupBaseAlignment) != 0) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysNV-callableShaderBindingOffset-02462",
                         "vkCmdTraceRaysNV: callableShaderBindingOffset must be a multiple of "
                         "VkPhysicalDeviceRayTracingPropertiesNV::shaderGroupBaseAlignment.");
    }
    if (SafeModulo(callableShaderBindingStride, phys_dev_ext_props.ray_tracing_propsNV.shaderGroupHandleSize) != 0) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysNV-callableShaderBindingStride-02465",
                         "vkCmdTraceRaysNV: callableShaderBindingStride must be a multiple of "
                         "VkPhysicalDeviceRayTracingPropertiesNV::shaderGroupHandleSize.");
    }
    if (callableShaderBindingStride > phys_dev_ext_props.ray_tracing_propsNV.maxShaderGroupStride) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysNV-callableShaderBindingStride-02468",
                         "vkCmdTraceRaysNV: callableShaderBindingStride must be less than or equal to "
                         "VkPhysicalDeviceRayTracingPropertiesNV::maxShaderGroupStride. ");
    }

    // hitShader
    if (SafeModulo(hitShaderBindingOffset, phys_dev_ext_props.ray_tracing_propsNV.shaderGroupBaseAlignment) != 0) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysNV-hitShaderBindingOffset-02460",
                         "vkCmdTraceRaysNV: hitShaderBindingOffset must be a multiple of "
                         "VkPhysicalDeviceRayTracingPropertiesNV::shaderGroupBaseAlignment.");
    }
    if (SafeModulo(hitShaderBindingStride, phys_dev_ext_props.ray_tracing_propsNV.shaderGroupHandleSize) != 0) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysNV-hitShaderBindingStride-02464",
                         "vkCmdTraceRaysNV: hitShaderBindingStride must be a multiple of "
                         "VkPhysicalDeviceRayTracingPropertiesNV::shaderGroupHandleSize.");
    }
    if (hitShaderBindingStride > phys_dev_ext_props.ray_tracing_propsNV.maxShaderGroupStride) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysNV-hitShaderBindingStride-02467",
                         "vkCmdTraceRaysNV: hitShaderBindingStride must be less than or equal to "
                         "VkPhysicalDeviceRayTracingPropertiesNV::maxShaderGroupStride.");
    }

    // missShader
    if (SafeModulo(missShaderBindingOffset, phys_dev_ext_props.ray_tracing_propsNV.shaderGroupBaseAlignment) != 0) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysNV-missShaderBindingOffset-02458",
                         "vkCmdTraceRaysNV: missShaderBindingOffset must be a multiple of "
                         "VkPhysicalDeviceRayTracingPropertiesNV::shaderGroupBaseAlignment.");
    }
    if (SafeModulo(missShaderBindingStride, phys_dev_ext_props.ray_tracing_propsNV.shaderGroupHandleSize) != 0) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysNV-missShaderBindingStride-02463",
                         "vkCmdTraceRaysNV: missShaderBindingStride must be a multiple of "
                         "VkPhysicalDeviceRayTracingPropertiesNV::shaderGroupHandleSize.");
    }
    if (missShaderBindingStride > phys_dev_ext_props.ray_tracing_propsNV.maxShaderGroupStride) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysNV-missShaderBindingStride-02466",
                         "vkCmdTraceRaysNV: missShaderBindingStride must be less than or equal to "
                         "VkPhysicalDeviceRayTracingPropertiesNV::maxShaderGroupStride.");
    }

    // raygenShader
    if (SafeModulo(raygenShaderBindingOffset, phys_dev_ext_props.ray_tracing_propsNV.shaderGroupBaseAlignment) != 0) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysNV-raygenShaderBindingOffset-02456",
                         "vkCmdTraceRaysNV: raygenShaderBindingOffset must be a multiple of "
                         "VkPhysicalDeviceRayTracingPropertiesNV::shaderGroupBaseAlignment.");
    }
    if (width > device_limits.maxComputeWorkGroupCount[0]) {
        skip |=
            LogError(device, "VUID-vkCmdTraceRaysNV-width-02469",
                     "vkCmdTraceRaysNV: width must be less than or equal to VkPhysicalDeviceLimits::maxComputeWorkGroupCount[o].");
    }
    if (height > device_limits.maxComputeWorkGroupCount[1]) {
        skip |=
            LogError(device, "VUID-vkCmdTraceRaysNV-height-02470",
                     "vkCmdTraceRaysNV: height must be less than or equal to VkPhysicalDeviceLimits::maxComputeWorkGroupCount[1].");
    }
    if (depth > device_limits.maxComputeWorkGroupCount[2]) {
        skip |=
            LogError(device, "VUID-vkCmdTraceRaysNV-depth-02471",
                     "vkCmdTraceRaysNV: depth must be less than or equal to VkPhysicalDeviceLimits::maxComputeWorkGroupCount[2].");
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdBuildAccelerationStructureIndirectKHR(
    VkCommandBuffer commandBuffer, const VkAccelerationStructureBuildGeometryInfoKHR *pInfo, VkBuffer indirectBuffer,
    VkDeviceSize indirectOffset, uint32_t indirectStride) const {
    bool skip = false;
    const auto *raytracing_features = lvl_find_in_chain<VkPhysicalDeviceRayTracingFeaturesKHR>(device_createinfo_pnext);
    if (!raytracing_features || raytracing_features->rayTracingIndirectAccelerationStructureBuild == VK_FALSE) {
        skip |= LogError(
            device, "VUID-vkCmdBuildAccelerationStructureIndirectKHR-rayTracingIndirectAccelerationStructureBuild-03535",
            "vkCmdBuildAccelerationStructureIndirectKHR: The "
            "VkPhysicalDeviceRayTracingFeaturesKHR::rayTracingIndirectAccelerationStructureBuild feature must be enabled.");
    }
    const auto *pnext_struct = lvl_find_in_chain<VkDeferredOperationInfoKHR>(pInfo->pNext);
    if (pnext_struct) {
        skip |=
            LogError(device, "VUID-vkCmdBuildAccelerationStructureIndirectKHR-pNext-03536",
                     "vkCmdBuildAccelerationStructureIndirectKHR: The VkDeferredOperationInfoKHR structure must not be included in "
                     "the pNext chain of any of the provided VkAccelerationStructureBuildGeometryInfoKHR structures.");
    }
    return false;
}

bool StatelessValidation::manual_PreCallValidateGetDeviceAccelerationStructureCompatibilityKHR(
    VkDevice device, const VkAccelerationStructureVersionKHR *version) const {
    bool skip = false;
    const auto *raytracing_features = lvl_find_in_chain<VkPhysicalDeviceRayTracingFeaturesKHR>(device_createinfo_pnext);
    if (!raytracing_features || !(raytracing_features->rayQuery || raytracing_features->rayTracing)) {
        skip |= LogError(device, "VUID-vkGetDeviceAccelerationStructureCompatibilityKHR-rayTracing-03565",
                         "vkGetDeviceAccelerationStructureCompatibilityKHR: The rayTracing or rayQuery feature must be enabled.");
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateBuildAccelerationStructureKHR(
    VkDevice device, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
    const VkAccelerationStructureBuildOffsetInfoKHR *const *ppOffsetInfos) const {
    bool skip = false;
    const auto *raytracing_features = lvl_find_in_chain<VkPhysicalDeviceRayTracingFeaturesKHR>(device_createinfo_pnext);
    if (!raytracing_features || raytracing_features->rayTracingHostAccelerationStructureCommands == VK_FALSE) {
        skip |= LogError(device, "VUID-vkBuildAccelerationStructureKHR-rayTracingHostAccelerationStructureCommands-03439",
                         "vkBuildAccelerationStructureKHR: The "
                         "vkPhysicalDeviceRayTracingFeaturesKHR::rayTracingHostAccelerationStructureCommands"
                         "feature must be enabled .");
    }
    return skip;
}
bool StatelessValidation::manual_PreCallValidateCmdBuildAccelerationStructureKHR(
    VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
    const VkAccelerationStructureBuildOffsetInfoKHR *const *ppOffsetInfos) const {
    bool skip = false;
    for (uint32_t i = 0; i < infoCount; ++i) {
        const auto *pnext_struct = lvl_find_in_chain<VkDeferredOperationInfoKHR>(pInfos->pNext);
        if (pnext_struct) {
            skip |=
                LogError(commandBuffer, "VUID-vkCmdBuildAccelerationStructureKHR-pNext-03532",
                         "vkCmdBuildAccelerationStructureKHR: The VkDeferredOperationInfoKHR structure must not be included in the"
                         "pNext chain of any of the provided VkAccelerationStructureBuildGeometryInfoKHR structures.");
        }
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdSetViewportWithCountEXT(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                                                           const VkViewport *pViewports) const {
    bool skip = false;

    if (!physical_device_features.multiViewport) {
        if (viewportCount != 1) {
            skip |= LogError(commandBuffer, "VUID-vkCmdSetViewportWithCountEXT-viewportCount-03395",
                             "vkCmdSetViewportWithCountEXT: The multiViewport feature is disabled, but viewportCount (=%" PRIu32
                             ") is not 1.",
                             viewportCount);
        }
    } else {  // multiViewport enabled
        if (viewportCount < 1 || viewportCount > device_limits.maxViewports) {
            skip |= LogError(commandBuffer, "VUID-vkCmdSetViewportWithCountEXT-viewportCount-03394",
                             "vkCmdSetViewportWithCountEXT:  viewportCount (=%" PRIu32
                             ") must "
                             "not be greater than VkPhysicalDeviceLimits::maxViewports (=%" PRIu32 ").",
                             viewportCount, device_limits.maxViewports);
        }
    }

    if (pViewports) {
        for (uint32_t viewport_i = 0; viewport_i < viewportCount; ++viewport_i) {
            const auto &viewport = pViewports[viewport_i];  // will crash on invalid ptr
            const char *fn_name = "vkCmdSetViewportWithCountEXT";
            skip |= manual_PreCallValidateViewport(
                viewport, fn_name, ParameterName("pViewports[%i]", ParameterName::IndexVector{viewport_i}), commandBuffer);
        }
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdSetScissorWithCountEXT(VkCommandBuffer commandBuffer, uint32_t scissorCount,
                                                                          const VkRect2D *pScissors) const {
    bool skip = false;

    if (!physical_device_features.multiViewport) {
        if (scissorCount != 1) {
            skip |= LogError(commandBuffer, "VUID-vkCmdSetScissorWithCountEXT-scissorCount-03398",
                             "vkCmdSetScissorWithCountEXT: scissorCount (=%" PRIu32
                             ") must "
                             "be 1 when the multiViewport feature is disabled.",
                             scissorCount);
        }
    } else {  // multiViewport enabled
        if (scissorCount == 0) {
            skip |= LogError(commandBuffer, "VUID-vkCmdSetScissorWithCountEXT-scissorCount-03397",
                             "vkCmdSetScissorWithCountEXT: scissorCount (=%" PRIu32
                             ") must "
                             "be great than zero.",
                             scissorCount);
        } else if (scissorCount > device_limits.maxViewports) {
            skip |= LogError(commandBuffer, "VUID-vkCmdSetScissorWithCountEXT-scissorCount-03397",
                             "vkCmdSetScissorWithCountEXT: scissorCount (=%" PRIu32
                             ") must "
                             "not be greater than VkPhysicalDeviceLimits::maxViewports (=%" PRIu32 ").",
                             scissorCount, device_limits.maxViewports);
        }
    }

    if (pScissors) {
        for (uint32_t scissor_i = 0; scissor_i < scissorCount; ++scissor_i) {
            const auto &scissor = pScissors[scissor_i];  // will crash on invalid ptr

            if (scissor.offset.x < 0) {
                skip |= LogError(commandBuffer, "VUID-vkCmdSetScissorWithCountEXT-x-03399",
                                 "vkCmdSetScissor: pScissors[%" PRIu32 "].offset.x (=%" PRIi32 ") is negative.", scissor_i,
                                 scissor.offset.x);
            }

            if (scissor.offset.y < 0) {
                skip |= LogError(commandBuffer, "VUID-vkCmdSetScissorWithCountEXT-x-03399",
                                 "vkCmdSetScissor: pScissors[%" PRIu32 "].offset.y (=%" PRIi32 ") is negative.", scissor_i,
                                 scissor.offset.y);
            }

            const int64_t x_sum = static_cast<int64_t>(scissor.offset.x) + static_cast<int64_t>(scissor.extent.width);
            if (x_sum > INT32_MAX) {
                skip |= LogError(commandBuffer, "VUID-vkCmdSetScissorWithCountEXT-offset-03400",
                                 "vkCmdSetScissor: offset.x + extent.width (=%" PRIi32 " + %" PRIu32 " = %" PRIi64
                                 ") of pScissors[%" PRIu32 "] will overflow int32_t.",
                                 scissor.offset.x, scissor.extent.width, x_sum, scissor_i);
            }

            const int64_t y_sum = static_cast<int64_t>(scissor.offset.y) + static_cast<int64_t>(scissor.extent.height);
            if (y_sum > INT32_MAX) {
                skip |= LogError(commandBuffer, "VUID-vkCmdSetScissorWithCountEXT-offset-03401",
                                 "vkCmdSetScissor: offset.y + extent.height (=%" PRIi32 " + %" PRIu32 " = %" PRIi64
                                 ") of pScissors[%" PRIu32 "] will overflow int32_t.",
                                 scissor.offset.y, scissor.extent.height, y_sum, scissor_i);
            }
        }
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdBindVertexBuffers2EXT(VkCommandBuffer commandBuffer, uint32_t firstBinding,
                                                                         uint32_t bindingCount, const VkBuffer *pBuffers,
                                                                         const VkDeviceSize *pOffsets, const VkDeviceSize *pSizes,
                                                                         const VkDeviceSize *pStrides) const {
    bool skip = false;
    if (firstBinding >= device_limits.maxVertexInputBindings) {
        skip |= LogError(commandBuffer, "VUID-vkCmdBindVertexBuffers2EXT-firstBinding-03355",
                         "vkCmdBindVertexBuffers2EXT() firstBinding (%u) must be less than maxVertexInputBindings (%u)",
                         firstBinding, device_limits.maxVertexInputBindings);
    } else if ((firstBinding + bindingCount) > device_limits.maxVertexInputBindings) {
        skip |= LogError(commandBuffer, "VUID-vkCmdBindVertexBuffers2EXT-firstBinding-03356",
                         "vkCmdBindVertexBuffers2EXT() sum of firstBinding (%u) and bindingCount (%u) must be less than "
                         "maxVertexInputBindings (%u)",
                         firstBinding, bindingCount, device_limits.maxVertexInputBindings);
    }

    for (uint32_t i = 0; i < bindingCount; ++i) {
        if (pBuffers[i] == VK_NULL_HANDLE) {
            const auto *robustness2_features = lvl_find_in_chain<VkPhysicalDeviceRobustness2FeaturesEXT>(device_createinfo_pnext);
            if (!(robustness2_features && robustness2_features->nullDescriptor)) {
                skip |= LogError(commandBuffer, "VUID-vkCmdBindVertexBuffers2EXT-pBuffers-04111",
                                 "vkCmdBindVertexBuffers2EXT() required parameter pBuffers[%d] specified as VK_NULL_HANDLE", i);
            } else {
                if (pOffsets[i] != 0) {
                    skip |=
                        LogError(commandBuffer, "VUID-vkCmdBindVertexBuffers2EXT-pBuffers-04112",
                                 "vkCmdBindVertexBuffers2EXT() pBuffers[%d] is VK_NULL_HANDLE, but pOffsets[%d] is not 0", i, i);
                }
            }
        }
        if (pStrides) {
            if (pStrides[i] > device_limits.maxVertexInputBindingStride) {
                skip |=
                    LogError(commandBuffer, "VUID-vkCmdBindVertexBuffers2EXT-pStrides-03362",
                             "vkCmdBindVertexBuffers2EXT() pStrides[%d] (%u) must be less than maxVertexInputBindingStride (%u)", i,
                             pStrides[i], device_limits.maxVertexInputBindingStride);
            }
        }
    }

    return skip;
}
