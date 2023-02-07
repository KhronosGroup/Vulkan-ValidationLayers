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

#include <cmath>

#include "chassis.h"
#include "stateless/stateless_validation.h"
#include "layer_chassis_dispatch.h"
#include "core_validation_error_enums.h"
#include "enum_flag_bits.h"
#include "vk_layer_data.h"

static const int kMaxParamCheckerStringLength = 256;

namespace {
template <typename T>
inline bool in_inclusive_range(const T &value, const T &min, const T &max) {
    // Using only < for generality and || for early abort
    return !((value < min) || (max < value));
}

struct ImportOperationsInfo {
    const VkImportMemoryHostPointerInfoEXT *host_pointer_info_ext;
    uint32_t total_import_ops;
};

ImportOperationsInfo GetNumberOfImportInfo(const VkMemoryAllocateInfo *pAllocateInfo) {
    uint32_t count = 0;

#ifdef VK_USE_PLATFORM_WIN32_KHR
    // VkImportMemoryWin32HandleInfoKHR with a non-zero handleType value
    auto import_memory_win32_handle = LvlFindInChain<VkImportMemoryWin32HandleInfoKHR>(pAllocateInfo->pNext);
    count += (import_memory_win32_handle && import_memory_win32_handle->handleType);
#endif

    // VkImportMemoryFdInfoKHR with a non-zero handleType value
    auto fd_info_khr = LvlFindInChain<VkImportMemoryFdInfoKHR>(pAllocateInfo->pNext);
    count += (fd_info_khr && fd_info_khr->handleType);

    // VkImportMemoryHostPointerInfoEXT with a non-zero handleType value
    auto host_pointer_info_ext = LvlFindInChain<VkImportMemoryHostPointerInfoEXT>(pAllocateInfo->pNext);
    count += (host_pointer_info_ext && host_pointer_info_ext->handleType);

#ifdef VK_USE_PLATFORM_ANDROID_KHR
    // VkImportAndroidHardwareBufferInfoANDROID with a non-NULL buffer value
    auto import_memory_ahb = LvlFindInChain<VkImportAndroidHardwareBufferInfoANDROID>(pAllocateInfo->pNext);
    count += (import_memory_ahb && import_memory_ahb->buffer);
#endif

#ifdef VK_USE_PLATFORM_FUCHSIA
    // VkImportMemoryZirconHandleInfoFUCHSIA with a non-zero handleType value
    auto import_zircon_fuchsia = LvlFindInChain<VkImportMemoryZirconHandleInfoFUCHSIA>(pAllocateInfo->pNext);
    count += (import_zircon_fuchsia && import_zircon_fuchsia->handleType);

    // VkImportMemoryBufferCollectionFUCHSIA
    auto import_buffer_collection_fuchsia = LvlFindInChain<VkImportMemoryBufferCollectionFUCHSIA>(pAllocateInfo->pNext);
    count += static_cast<bool>(
        import_buffer_collection_fuchsia);  // NOTE: There's no handleType on VkImportMemoryBufferCollectionFUCHSIA, so we
                                            // can't check that, and from the "Valid Usage (Implicit)" collection has to
                                            // always be valid.
#endif

    ImportOperationsInfo info = {};
    info.total_import_ops = count;
    info.host_pointer_info_ext = host_pointer_info_ext;

    return info;
}
}  // namespace

ReadLockGuard StatelessValidation::ReadLock() const { return ReadLockGuard(validation_object_mutex, std::defer_lock); }
WriteLockGuard StatelessValidation::WriteLock() { return WriteLockGuard(validation_object_mutex, std::defer_lock); }

static vvl::unordered_map<VkCommandBuffer, VkCommandPool> secondary_cb_map{};
static std::shared_mutex secondary_cb_map_mutex;
static ReadLockGuard CBReadLock() { return ReadLockGuard(secondary_cb_map_mutex); }
static WriteLockGuard CBWriteLock() { return WriteLockGuard(secondary_cb_map_mutex); }

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

bool StatelessValidation::ValidateApiVersion(uint32_t api_version, uint32_t effective_api_version) const {
    bool skip = false;
    uint32_t api_version_nopatch = VK_MAKE_VERSION(VK_VERSION_MAJOR(api_version), VK_VERSION_MINOR(api_version), 0);
    if (api_version_nopatch != effective_api_version) {
        if ((api_version_nopatch < VK_API_VERSION_1_0) && (api_version != 0)) {
            skip |= LogError(instance, "VUID-VkApplicationInfo-apiVersion-04010",
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
};

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
                                 "vkCreateDevice(): VkDeviceCreateInfo->ppEnabledExtensionNames must not include "
                                 "VK_AMD_negative_viewport_height if api version is greater than or equal to 1.1.");
            } else if (maint1) {
                skip |= LogError(device, "VUID-VkDeviceCreateInfo-ppEnabledExtensionNames-00374",
                                 "vkCreateDevice(): VkDeviceCreateInfo->ppEnabledExtensionNames must not simultaneously include "
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
                             "VkDeviceCreateInfo->ppEnabledExtensionNames must not contain both VK_KHR_buffer_device_address and "
                             "VK_EXT_buffer_device_address.");
        }
    }

    if (pCreateInfo->pNext != NULL && pCreateInfo->pEnabledFeatures) {
        // Check for get_physical_device_properties2 struct
        const auto *features2 = LvlFindInChain<VkPhysicalDeviceFeatures2>(pCreateInfo->pNext);
        if (features2) {
            // Cannot include VkPhysicalDeviceFeatures2 and have non-null pEnabledFeatures
            skip |= LogError(device, "VUID-VkDeviceCreateInfo-pNext-00373",
                             "VkDeviceCreateInfo->pNext includes a VkPhysicalDeviceFeatures2 struct when "
                             "pCreateInfo->pEnabledFeatures is non-NULL.");
        }
    }

    auto features2 = LvlFindInChain<VkPhysicalDeviceFeatures2>(pCreateInfo->pNext);
    const VkPhysicalDeviceFeatures *features = features2 ? &features2->features : pCreateInfo->pEnabledFeatures;
    const auto *robustness2_features = LvlFindInChain<VkPhysicalDeviceRobustness2FeaturesEXT>(pCreateInfo->pNext);
    if (features && robustness2_features && robustness2_features->robustBufferAccess2 && !features->robustBufferAccess) {
        skip |= LogError(device, "VUID-VkPhysicalDeviceRobustness2FeaturesEXT-robustBufferAccess2-04000",
                         "If robustBufferAccess2 is enabled then robustBufferAccess must be enabled.");
    }
    const auto *raytracing_features = LvlFindInChain<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>(pCreateInfo->pNext);
    if (raytracing_features && raytracing_features->rayTracingPipelineShaderGroupHandleCaptureReplayMixed &&
        !raytracing_features->rayTracingPipelineShaderGroupHandleCaptureReplay) {
        skip |= LogError(
            device,
            "VUID-VkPhysicalDeviceRayTracingPipelineFeaturesKHR-rayTracingPipelineShaderGroupHandleCaptureReplayMixed-03575",
            "If rayTracingPipelineShaderGroupHandleCaptureReplayMixed is VK_TRUE, rayTracingPipelineShaderGroupHandleCaptureReplay "
            "must also be VK_TRUE.");
    }
    auto vertex_attribute_divisor_features = LvlFindInChain<VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT>(pCreateInfo->pNext);
    if (vertex_attribute_divisor_features && (!IsExtEnabled(device_extensions.vk_ext_vertex_attribute_divisor))) {
        skip |= LogError(device, kVUID_PVError_ExtensionNotEnabled,
                         "VkDeviceCreateInfo->pNext includes a VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT "
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
                skip |= LogError(instance, "VUID-VkDeviceCreateInfo-pNext-06532",
                                 "vkCreateDevice(): %s structure included in VkPhysicalDeviceVulkan13Features' pNext chain.",
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
        skip |= LogError(instance, "VUID-VkPhysicalDeviceVariablePointersFeatures-variablePointers-01431",
                         "If variablePointers is VK_TRUE then variablePointersStorageBuffer also needs to be VK_TRUE");
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
                         "If multiviewGeometryShader is VK_TRUE then multiview also needs to be VK_TRUE");
    }
    if ((multiview == VK_FALSE) && (multiview_tessellation_shader == VK_TRUE)) {
        skip |= LogError(instance, "VUID-VkPhysicalDeviceMultiviewFeatures-multiviewTessellationShader-00581",
                         "If multiviewTessellationShader is VK_TRUE then multiview also needs to be VK_TRUE");
    }

    return skip;
}

bool StatelessValidation::RequireDeviceExtension(bool flag, char const *function_name, char const *extension_name) const {
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

        const auto *maintenance4_features = LvlFindInChain<VkPhysicalDeviceMaintenance4FeaturesKHR>(device_createinfo_pnext);
        if (maintenance4_features && maintenance4_features->maintenance4) {
            if (pCreateInfo->size > phys_dev_ext_props.maintenance4_props.maxBufferSize) {
                skip |= LogError(device, "VUID-VkBufferCreateInfo-size-06409",
                                 "vkCreateBuffer: pCreateInfo->size is larger than the maximum allowed buffer size "
                                 "VkPhysicalDeviceMaintenance4Properties.maxBufferSize");
            }
        }
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCreateImage(VkDevice device, const VkImageCreateInfo *pCreateInfo,
                                                            const VkAllocationCallbacks *pAllocator, VkImage *pImage) const {
    bool skip = false;

    if (pCreateInfo != nullptr) {
        const VkFormat image_format = pCreateInfo->format;
        const VkImageCreateFlags image_flags = pCreateInfo->flags;
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
            if (image_flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) {
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
        uint32_t max_dim = std::max(std::max(pCreateInfo->extent.width, pCreateInfo->extent.height), pCreateInfo->extent.depth);
        // Max mip levels is different for corner-sampled images vs normal images.
        uint32_t max_mip_levels = (image_flags & VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV)
                                      ? static_cast<uint32_t>(ceil(log2(max_dim)))
                                      : static_cast<uint32_t>(floor(log2(max_dim)) + 1);
        if (max_dim > 0 && pCreateInfo->mipLevels > max_mip_levels) {
            skip |=
                LogError(device, "VUID-VkImageCreateInfo-mipLevels-00958",
                         "vkCreateImage(): pCreateInfo->mipLevels must be less than or equal to "
                         "floor(log2(max(pCreateInfo->extent.width, pCreateInfo->extent.height, pCreateInfo->extent.depth)))+1.");
        }

        if ((image_flags & VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT) && (pCreateInfo->imageType != VK_IMAGE_TYPE_3D)) {
            skip |= LogError(device, "VUID-VkImageCreateInfo-flags-00950",
                             "vkCreateImage(): pCreateInfo->flags contains VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT but "
                             "pCreateInfo->imageType is not VK_IMAGE_TYPE_3D.");
        }

        if ((image_flags & VK_IMAGE_CREATE_2D_VIEW_COMPATIBLE_BIT_EXT) && (pCreateInfo->imageType != VK_IMAGE_TYPE_3D)) {
            skip |= LogError(device, "VUID-VkImageCreateInfo-flags-07755",
                             "vkCreateImage(): pCreateInfo->flags contains VK_IMAGE_CREATE_2D_VIEW_COMPATIBLE_BIT_EXT but "
                             "pCreateInfo->imageType is not VK_IMAGE_TYPE_3D.");
        }

        if ((image_flags & VK_IMAGE_CREATE_SPARSE_BINDING_BIT) && (!physical_device_features.sparseBinding)) {
            skip |= LogError(device, "VUID-VkImageCreateInfo-flags-00969",
                             "vkCreateImage(): pCreateInfo->flags contains VK_IMAGE_CREATE_SPARSE_BINDING_BIT, but the "
                             "VkPhysicalDeviceFeatures::sparseBinding feature is disabled.");
        }

        if ((image_flags & VK_IMAGE_CREATE_SPARSE_ALIASED_BIT) && (!physical_device_features.sparseResidencyAliased)) {
            skip |= LogError(
                device, "VUID-VkImageCreateInfo-flags-01924",
                "vkCreateImage(): the sparseResidencyAliased device feature is disabled: Images cannot be created with the "
                "VK_IMAGE_CREATE_SPARSE_ALIASED_BIT set.");
        }

        // If flags contains VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT or VK_IMAGE_CREATE_SPARSE_ALIASED_BIT, it must also contain
        // VK_IMAGE_CREATE_SPARSE_BINDING_BIT
        if (((image_flags & (VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT | VK_IMAGE_CREATE_SPARSE_ALIASED_BIT)) != 0) &&
            ((image_flags & VK_IMAGE_CREATE_SPARSE_BINDING_BIT) != VK_IMAGE_CREATE_SPARSE_BINDING_BIT)) {
            skip |= LogError(device, "VUID-VkImageCreateInfo-flags-00987",
                             "vkCreateImage: if pCreateInfo->flags contains VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT or "
                             "VK_IMAGE_CREATE_SPARSE_ALIASED_BIT, it must also contain VK_IMAGE_CREATE_SPARSE_BINDING_BIT.");
        }

        // Check for combinations of attributes that are incompatible with having VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT set
        if ((image_flags & VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT) != 0) {
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

        // alias VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV
        if (pCreateInfo->usage & VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR) {
            if (pCreateInfo->imageType != VK_IMAGE_TYPE_2D) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-imageType-02082",
                                 "vkCreateImage: if usage includes VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR (or the "
                                 "alias VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV), imageType must be VK_IMAGE_TYPE_2D.");
            }
            if (pCreateInfo->samples != VK_SAMPLE_COUNT_1_BIT) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-samples-02083",
                                 "vkCreateImage: if usage includes VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR (or the "
                                 "alias VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV), samples must be VK_SAMPLE_COUNT_1_BIT.");
            }
            const auto *shading_rate_image_features =
                LvlFindInChain<VkPhysicalDeviceShadingRateImageFeaturesNV>(device_createinfo_pnext);
            if (shading_rate_image_features && shading_rate_image_features->shadingRateImage &&
                pCreateInfo->tiling != VK_IMAGE_TILING_OPTIMAL) {
                // KHR flag can be non-optimal
                skip |= LogError(device, "VUID-VkImageCreateInfo-shadingRateImage-07727",
                                 "vkCreateImage: if usage includes VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV, tiling must be "
                                 "VK_IMAGE_TILING_OPTIMAL.");
            }
        }

        if (image_flags & VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV) {
            if (pCreateInfo->imageType != VK_IMAGE_TYPE_2D && pCreateInfo->imageType != VK_IMAGE_TYPE_3D) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-flags-02050",
                                 "vkCreateImage: If flags contains VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV, "
                                 "imageType must be VK_IMAGE_TYPE_2D or VK_IMAGE_TYPE_3D.");
            }

            if ((image_flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) || FormatIsDepthOrStencil(image_format)) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-flags-02051",
                                 "vkCreateImage: If flags contains VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV, "
                                 "it must not also contain VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT and format (%s) must not be a "
                                 "depth/stencil format.",
                                 string_VkFormat(image_format));
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

        if (((image_flags & VK_IMAGE_CREATE_SAMPLE_LOCATIONS_COMPATIBLE_DEPTH_BIT_EXT) != 0) &&
            (FormatHasDepth(image_format) == false)) {
            skip |= LogError(device, "VUID-VkImageCreateInfo-flags-01533",
                             "vkCreateImage(): if flags contain VK_IMAGE_CREATE_SAMPLE_LOCATIONS_COMPATIBLE_DEPTH_BIT_EXT the "
                             "format (%s) must be a depth or depth/stencil format.",
                             string_VkFormat(image_format));
        }

        const auto image_stencil_struct = LvlFindInChain<VkImageStencilUsageCreateInfo>(pCreateInfo->pNext);
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

            if (FormatIsDepthOrStencil(image_format)) {
                if ((image_stencil_struct->stencilUsage & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT) != 0) {
                    if (pCreateInfo->extent.width > device_limits.maxFramebufferWidth) {
                        skip |=
                            LogError(device, "VUID-VkImageCreateInfo-Format-02536",
                                     "vkCreateImage(): Depth-stencil image contains VkImageStencilUsageCreateInfo structure with "
                                     "stencilUsage including VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT and image width (%" PRIu32
                                     ") exceeds device "
                                     "maxFramebufferWidth (%" PRIu32 ")",
                                     pCreateInfo->extent.width, device_limits.maxFramebufferWidth);
                    }

                    if (pCreateInfo->extent.height > device_limits.maxFramebufferHeight) {
                        skip |=
                            LogError(device, "VUID-VkImageCreateInfo-format-02537",
                                     "vkCreateImage(): Depth-stencil image contains VkImageStencilUsageCreateInfo structure with "
                                     "stencilUsage including VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT and image height (%" PRIu32
                                     ") exceeds device "
                                     "maxFramebufferHeight (%" PRIu32 ")",
                                     pCreateInfo->extent.height, device_limits.maxFramebufferHeight);
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

        std::vector<uint64_t> image_create_drm_format_modifiers;
        if (IsExtEnabled(device_extensions.vk_ext_image_drm_format_modifier)) {
            const auto drm_format_mod_list = LvlFindInChain<VkImageDrmFormatModifierListCreateInfoEXT>(pCreateInfo->pNext);
            const auto drm_format_mod_explict = LvlFindInChain<VkImageDrmFormatModifierExplicitCreateInfoEXT>(pCreateInfo->pNext);
            if (pCreateInfo->tiling == VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT) {
                if (((drm_format_mod_list != nullptr) && (drm_format_mod_explict != nullptr)) ||
                    ((drm_format_mod_list == nullptr) && (drm_format_mod_explict == nullptr))) {
                    skip |= LogError(device, "VUID-VkImageCreateInfo-tiling-02261",
                                     "vkCreateImage(): Tiling is VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT but pNext must have "
                                     "either VkImageDrmFormatModifierListCreateInfoEXT or "
                                     "VkImageDrmFormatModifierExplicitCreateInfoEXT in the pNext chain");
                } else if (drm_format_mod_explict != nullptr) {
                    image_create_drm_format_modifiers.push_back(drm_format_mod_explict->drmFormatModifier);
                } else if (drm_format_mod_list != nullptr) {
                    for (uint32_t i = 0; i < drm_format_mod_list->drmFormatModifierCount; i++) {
                        image_create_drm_format_modifiers.push_back(*drm_format_mod_list->pDrmFormatModifiers);
                    }
                }
            } else if ((drm_format_mod_list != nullptr) || (drm_format_mod_explict != nullptr)) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-pNext-02262",
                                 "vkCreateImage(): Tiling is not VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT but there is a "
                                 "VkImageDrmFormatModifierListCreateInfoEXT or VkImageDrmFormatModifierExplicitCreateInfoEXT "
                                 "in the pNext chain");
            }

            if (drm_format_mod_explict != nullptr && drm_format_mod_explict->pPlaneLayouts != nullptr) {
                for (uint32_t i = 0; i < drm_format_mod_explict->drmFormatModifierPlaneCount; ++i) {
                    if (drm_format_mod_explict->pPlaneLayouts[i].size != 0) {
                        skip |= LogError(device, "VUID-VkImageDrmFormatModifierExplicitCreateInfoEXT-size-02267",
                                         "vkCreateImage(): size is nonzero (%" PRIu64 ") in element %" PRIu32
                                         " of  VkImageDrmFormatModifierListCreateInfoEXT->pPlanedLayouts.",
                                         drm_format_mod_explict->pPlaneLayouts[i].size, i);
                    }
                    if (pCreateInfo->arrayLayers == 1 && drm_format_mod_explict->pPlaneLayouts[i].arrayPitch != 0) {
                        skip |= LogError(device, "VUID-VkImageDrmFormatModifierExplicitCreateInfoEXT-arrayPitch-02268",
                                         "vkCreateImage(): arrayPitch is nonzero (%" PRIu64 ") in element %" PRIu32
                                         " of VkImageDrmFormatModifierListCreateInfoEXT->pPlanedLayouts "
                                         "with pCreateInfo->arrayLayers being 1.",
                                         drm_format_mod_explict->pPlaneLayouts[i].arrayPitch, i);
                    }
                    if (pCreateInfo->extent.depth == 1 && drm_format_mod_explict->pPlaneLayouts[i].depthPitch != 0) {
                        skip |= LogError(device, "VUID-VkImageDrmFormatModifierExplicitCreateInfoEXT-depthPitch-02269",
                                         "vkCreateImage(): depthPitch is nonzero (%" PRIu64 ") in element %" PRIu32
                                         " of VkImageDrmFormatModifierListCreateInfoEXT->pPlanedLayouts "
                                         "with pCreateInfo->extext.depth being 1.",
                                         drm_format_mod_explict->pPlaneLayouts[i].depthPitch, i);
                    }
                }
            }
        }

        static const uint64_t drm_format_mod_linear = 0;
        bool image_create_maybe_linear = false;
        if (pCreateInfo->tiling == VK_IMAGE_TILING_LINEAR) {
            image_create_maybe_linear = true;
        } else if (pCreateInfo->tiling == VK_IMAGE_TILING_OPTIMAL) {
            image_create_maybe_linear = false;
        } else if (pCreateInfo->tiling == VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT) {
            image_create_maybe_linear =
                (std::find(image_create_drm_format_modifiers.begin(), image_create_drm_format_modifiers.end(),
                           drm_format_mod_linear) != image_create_drm_format_modifiers.end());
        }

        // If multi-sample, validate type, usage, tiling and mip levels.
        if ((pCreateInfo->samples != VK_SAMPLE_COUNT_1_BIT) &&
            ((pCreateInfo->imageType != VK_IMAGE_TYPE_2D) || (image_flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) ||
             (pCreateInfo->mipLevels != 1) || image_create_maybe_linear)) {
            skip |= LogError(device, "VUID-VkImageCreateInfo-samples-02257",
                             "vkCreateImage(): Multi-sample image with incompatible type, usage, tiling, or mips.");
        }

        if ((image_flags & VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT) &&
            ((pCreateInfo->mipLevels != 1) || (pCreateInfo->arrayLayers != 1) || (pCreateInfo->imageType != VK_IMAGE_TYPE_2D) ||
             image_create_maybe_linear)) {
            skip |= LogError(device, "VUID-VkImageCreateInfo-flags-02259",
                             "vkCreateImage(): Multi-device image with incompatible type, usage, tiling, or mips.");
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
        }
        if (image_flags & VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT) {
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
            if (image_flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-flags-02567",
                                 "vkCreateImage: if flags includes VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT, "
                                 "flags must not include VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT.");
            }
            if (pCreateInfo->mipLevels != 1) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-flags-02568",
                                 "vkCreateImage: if flags includes VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT, mipLevels (%" PRIu32
                                 ") must be 1.",
                                 pCreateInfo->mipLevels);
            }
        }

        const auto swapchain_create_info = LvlFindInChain<VkImageSwapchainCreateInfoKHR>(pCreateInfo->pNext);
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
                    skip |= LogError(device, vuid, "%s must have a mipLevels value of 1 instead of %" PRIu32 ".", base_message,
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
                     VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT | VK_IMAGE_CREATE_EXTENDED_USAGE_BIT);
                if ((image_flags & ~valid_flags) != 0) {
                    skip |= LogError(device, vuid, "%s flags are %" PRIu32 "and must only have valid flags set.", base_message,
                                     image_flags);
                }
            }
        }

        // If Chroma subsampled format ( _420_ or _422_ )
        if (FormatIsXChromaSubsampled(image_format) && (SafeModulo(pCreateInfo->extent.width, 2) != 0)) {
            skip |=
                LogError(device, "VUID-VkImageCreateInfo-format-04712",
                         "vkCreateImage(): The format (%s) is X Chroma Subsampled (has _422 or _420 suffix) so the width (=%" PRIu32
                         ") must be a multiple of 2.",
                         string_VkFormat(image_format), pCreateInfo->extent.width);
        }
        if (FormatIsYChromaSubsampled(image_format) && (SafeModulo(pCreateInfo->extent.height, 2) != 0)) {
            skip |= LogError(device, "VUID-VkImageCreateInfo-format-04713",
                             "vkCreateImage(): The format (%s) is Y Chroma Subsampled (has _420 suffix) so the height (=%" PRIu32
                             ") must be a multiple of 2.",
                             string_VkFormat(image_format), pCreateInfo->extent.height);
        }

        const auto format_list_info = LvlFindInChain<VkImageFormatListCreateInfo>(pCreateInfo->pNext);
        if (format_list_info) {
            const uint32_t viewFormatCount = format_list_info->viewFormatCount;
            if (((image_flags & VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT) == 0) && (viewFormatCount > 1)) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-flags-04738",
                                 "vkCreateImage(): If the VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT is not set, then "
                                 "VkImageFormatListCreateInfo::viewFormatCount (%" PRIu32 ") must be 0 or 1.",
                                 viewFormatCount);
            }
            // Check if viewFormatCount is not zero that it is all compatible
            for (uint32_t i = 0; i < viewFormatCount; i++) {
                const bool class_compatible =
                    FormatCompatibilityClass(format_list_info->pViewFormats[i]) == FormatCompatibilityClass(image_format);
                if (!class_compatible) {
                    if (image_flags & VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT) {
                        const bool size_compatible =
                            FormatIsCompressed(format_list_info->pViewFormats[i])
                                ? false
                                : FormatElementSize(format_list_info->pViewFormats[i]) == FormatElementSize(image_format);
                        if (!size_compatible) {
                            skip |= LogError(device, "VUID-VkImageCreateInfo-pNext-06722",
                                             "vkCreateImage(): VkImageFormatListCreateInfo::pViewFormats[%" PRIu32
                                             "] (%s) and VkImageCreateInfo::format (%s) are not compatible or size-compatible.",
                                             i, string_VkFormat(format_list_info->pViewFormats[i]), string_VkFormat(image_format));
                        }
                    } else {
                        skip |= LogError(device, "VUID-VkImageCreateInfo-pNext-06722",
                                         "vkCreateImage(): VkImageFormatListCreateInfo::pViewFormats[%" PRIu32
                                         "] (%s) and VkImageCreateInfo::format (%s) are not compatible.",
                                         i, string_VkFormat(format_list_info->pViewFormats[i]), string_VkFormat(image_format));
                    }
                }
            }
        }

        const auto image_compression_control = LvlFindInChain<VkImageCompressionControlEXT>(pCreateInfo->pNext);
        if (image_compression_control) {
            constexpr VkImageCompressionFlagsEXT AllVkImageCompressionFlagBitsEXT =
                (VK_IMAGE_COMPRESSION_DEFAULT_EXT | VK_IMAGE_COMPRESSION_FIXED_RATE_DEFAULT_EXT |
                 VK_IMAGE_COMPRESSION_FIXED_RATE_EXPLICIT_EXT | VK_IMAGE_COMPRESSION_DISABLED_EXT);
            skip |= ValidateFlags("vkCreateImage", "VkImageCompressionControlEXT::flags", "VkImageCompressionFlagsEXT",
                                  AllVkImageCompressionFlagBitsEXT, image_compression_control->flags, kRequiredSingleBit,
                                  "VUID-VkImageCompressionControlEXT-flags-06747");

            if (image_compression_control->flags == VK_IMAGE_COMPRESSION_FIXED_RATE_EXPLICIT_EXT &&
                !image_compression_control->pFixedRateFlags) {
                skip |= LogError(device, "VUID-VkImageCompressionControlEXT-flags-06748",
                                 "VkImageCompressionControlEXT::pFixedRateFlags is nullptr even though "
                                 "VkImageCompressionControlEXT::flags are %s",
                                 string_VkImageCompressionFlagsEXT(image_compression_control->flags).c_str());
            }
        }
#ifdef VK_USE_PLATFORM_METAL_EXT
        auto export_metal_object_info = LvlFindInChain<VkExportMetalObjectCreateInfoEXT>(pCreateInfo->pNext);
        while (export_metal_object_info) {
            if ((export_metal_object_info->exportObjectType != VK_EXPORT_METAL_OBJECT_TYPE_METAL_TEXTURE_BIT_EXT) &&
                (export_metal_object_info->exportObjectType != VK_EXPORT_METAL_OBJECT_TYPE_METAL_IOSURFACE_BIT_EXT)) {
                skip |=
                    LogError(device, "VUID-VkImageCreateInfo-pNext-06783",
                             "vkCreateImage(): The pNext chain contains a VkExportMetalObjectCreateInfoEXT whose "
                             "exportObjectType = %s, but only VkExportMetalObjectCreateInfoEXT structs with exportObjectType of "
                             "VK_EXPORT_METAL_OBJECT_TYPE_METAL_TEXTURE_BIT_EXT or "
                             "VK_EXPORT_METAL_OBJECT_TYPE_METAL_IOSURFACE_BIT_EXT are allowed",
                             string_VkExportMetalObjectTypeFlagBitsEXT(export_metal_object_info->exportObjectType));
            }
            export_metal_object_info = LvlFindInChain<VkExportMetalObjectCreateInfoEXT>(export_metal_object_info->pNext);
        }
        auto import_metal_texture_info = LvlFindInChain<VkImportMetalTextureInfoEXT>(pCreateInfo->pNext);
        while (import_metal_texture_info) {
            if ((import_metal_texture_info->plane != VK_IMAGE_ASPECT_PLANE_0_BIT) &&
                (import_metal_texture_info->plane != VK_IMAGE_ASPECT_PLANE_1_BIT) &&
                (import_metal_texture_info->plane != VK_IMAGE_ASPECT_PLANE_2_BIT)) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-pNext-06784",
                                 "vkCreateImage(): The pNext chain contains a VkImportMetalTextureInfoEXT whose "
                                 "plane = %s, but only VK_IMAGE_ASPECT_PLANE_0_BIT, VK_IMAGE_ASPECT_PLANE_1_BIT, or "
                                 "VK_IMAGE_ASPECT_PLANE_2_BIT are allowed",
                                 string_VkImageAspectFlags(import_metal_texture_info->plane).c_str());
            }
            auto format_plane_count = FormatPlaneCount(pCreateInfo->format);
            if ((format_plane_count <= 1) && (import_metal_texture_info->plane != VK_IMAGE_ASPECT_PLANE_0_BIT)) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-pNext-06785",
                                 "vkCreateImage(): The pNext chain contains a VkImportMetalTextureInfoEXT whose "
                                 "plane = %s, but only VK_IMAGE_ASPECT_PLANE_0_BIT is allowed for an image created with format %s, "
                                 "which is not multiplaner",
                                 string_VkImageAspectFlags(import_metal_texture_info->plane).c_str(),
                                 string_VkFormat(pCreateInfo->format));
            }
            if ((format_plane_count == 2) && (import_metal_texture_info->plane == VK_IMAGE_ASPECT_PLANE_2_BIT)) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-pNext-06786",
                                 "vkCreateImage(): The pNext chain contains a VkImportMetalTextureInfoEXT whose "
                                 "plane == VK_IMAGE_ASPECT_PLANE_2_BIT, which is not allowed for an image created with format %s, "
                                 "which has only 2 planes",
                                 string_VkFormat(pCreateInfo->format));
            }
            import_metal_texture_info = LvlFindInChain<VkImportMetalTextureInfoEXT>(import_metal_texture_info->pNext);
        }
#endif  // VK_USE_PLATFORM_METAL_EXT
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
                                 "vkCreateImageView(): subresourceRange.layerCount (%" PRIu32
                                 ") must be 6 or VK_REMAINING_ARRAY_LAYERS.",
                                 pCreateInfo->subresourceRange.layerCount);
            }
            if (pCreateInfo->viewType == VK_IMAGE_VIEW_TYPE_CUBE_ARRAY && (pCreateInfo->subresourceRange.layerCount % 6) != 0) {
                skip |= LogError(device, "VUID-VkImageViewCreateInfo-viewType-02961",
                                 "vkCreateImageView(): subresourceRange.layerCount (%" PRIu32
                                 ") must be a multiple of 6 or VK_REMAINING_ARRAY_LAYERS.",
                                 pCreateInfo->subresourceRange.layerCount);
            }
        }

        auto astc_decode_mode = LvlFindInChain<VkImageViewASTCDecodeModeEXT>(pCreateInfo->pNext);
        if (IsExtEnabled(device_extensions.vk_ext_astc_decode_mode) && (astc_decode_mode != nullptr)) {
            if ((astc_decode_mode->decodeMode != VK_FORMAT_R16G16B16A16_SFLOAT) &&
                (astc_decode_mode->decodeMode != VK_FORMAT_R8G8B8A8_UNORM) &&
                (astc_decode_mode->decodeMode != VK_FORMAT_E5B9G9R9_UFLOAT_PACK32)) {
                skip |= LogError(device, "VUID-VkImageViewASTCDecodeModeEXT-decodeMode-02230",
                                 "vkCreateImageView(): VkImageViewASTCDecodeModeEXT::decodeMode must be "
                                 "VK_FORMAT_R16G16B16A16_SFLOAT, VK_FORMAT_R8G8B8A8_UNORM, or VK_FORMAT_E5B9G9R9_UFLOAT_PACK32.");
            }
            if ((FormatIsCompressed_ASTC_LDR(pCreateInfo->format) == false) &&
                (FormatIsCompressed_ASTC_HDR(pCreateInfo->format) == false)) {
                skip |= LogError(device, "VUID-VkImageViewASTCDecodeModeEXT-format-04084",
                                 "vkCreateImageView(): is using a VkImageViewASTCDecodeModeEXT but the image view format is %s and "
                                 "not an ASTC format.",
                                 string_VkFormat(pCreateInfo->format));
            }
        }

        auto ycbcr_conversion = LvlFindInChain<VkSamplerYcbcrConversionInfo>(pCreateInfo->pNext);
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
#ifdef VK_USE_PLATFORM_METAL_EXT
        skip |= ExportMetalObjectsPNextUtil(
            VK_EXPORT_METAL_OBJECT_TYPE_METAL_TEXTURE_BIT_EXT, "VUID-VkImageViewCreateInfo-pNext-06787",
            "vkCreateImageView():", "VK_EXPORT_METAL_OBJECT_TYPE_METAL_TEXTURE_BIT_EXT", pCreateInfo->pNext);
#endif  // VK_USE_PLATFORM_METAL_EXT
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
        if (v1_u32 < v2_u32) {
            return true;
        } else if (v1_u32 == v2_u32 && fract == 0.0f) {
            return true;
        } else {
            return false;
        }
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
    }

    // height
    bool height_healthy = true;
    const bool negative_height_enabled =
        IsExtEnabled(device_extensions.vk_khr_maintenance1) || IsExtEnabled(device_extensions.vk_amd_negative_viewport_height);
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

    // The extension was not created with a feature bit whichs prevents displaying the 2 variations of the VUIDs
    if (!IsExtEnabled(device_extensions.vk_ext_depth_range_unrestricted)) {
        // minDepth
        if (!(viewport.minDepth >= 0.0) || !(viewport.minDepth <= 1.0)) {
            // Also VUID-VkViewport-minDepth-02540
            skip |= LogError(object, "VUID-VkViewport-minDepth-01234",
                             "%s: VK_EXT_depth_range_unrestricted extension is not enabled and %s.minDepth (=%f) is not within the "
                             "[0.0, 1.0] range.",
                             fn_name, parameter_name.get_name().c_str(), viewport.minDepth);
        }

        // maxDepth
        if (!(viewport.maxDepth >= 0.0) || !(viewport.maxDepth <= 1.0)) {
            // Also VUID-VkViewport-maxDepth-02541
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
static SampleOrderInfo sample_order_infos[] = {
    {VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_1X2_PIXELS_NV, 1, 2},
    {VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_2X1_PIXELS_NV, 2, 1},
    {VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_2X2_PIXELS_NV, 2, 2},
    {VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_4X2_PIXELS_NV, 4, 2},
    {VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_2X4_PIXELS_NV, 2, 4},
    {VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_4X4_PIXELS_NV, 4, 4},
};

bool StatelessValidation::ValidateCoarseSampleOrderCustomNV(const VkCoarseSampleOrderCustomNV *order) const {
    bool skip = false;

    SampleOrderInfo *sample_order_info;
    uint32_t info_idx = 0;
    for (sample_order_info = nullptr; info_idx < ARRAY_SIZE(sample_order_infos); ++info_idx) {
        if (sample_order_infos[info_idx].shadingRate == order->shadingRate) {
            sample_order_info = &sample_order_infos[info_idx];
            break;
        }
    }

    if (sample_order_info == nullptr) {
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

    if (order->sampleLocationCount != order->sampleCount * sample_order_info->width * sample_order_info->height) {
        skip |= LogError(device, "VUID-VkCoarseSampleOrderCustomNV-sampleLocationCount-02075",
                         "VkCoarseSampleOrderCustomNV sampleLocationCount (=%" PRIu32
                         ") must "
                         "be equal to the product of sampleCount (=%" PRIu32
                         "), the fragment width for shadingRate "
                         "(=%" PRIu32 "), and the fragment height for shadingRate (=%" PRIu32 ").",
                         order->sampleLocationCount, order->sampleCount, sample_order_info->width, sample_order_info->height);
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
    uint64_t sample_locations_mask = 0;
    for (uint32_t i = 0; i < order->sampleLocationCount; ++i) {
        const VkCoarseSampleLocationNV *sample_loc = &order->pSampleLocations[i];
        if (sample_loc->pixelX >= sample_order_info->width) {
            skip |= LogError(device, "VUID-VkCoarseSampleLocationNV-pixelX-02078",
                             "pixelX must be less than the width (in pixels) of the fragment.");
        }
        if (sample_loc->pixelY >= sample_order_info->height) {
            skip |= LogError(device, "VUID-VkCoarseSampleLocationNV-pixelY-02079",
                             "pixelY must be less than the height (in pixels) of the fragment.");
        }
        if (sample_loc->sample >= order->sampleCount) {
            skip |= LogError(device, "VUID-VkCoarseSampleLocationNV-sample-02080",
                             "sample must be less than the number of coverage samples in each pixel belonging to the fragment.");
        }
        uint32_t idx =
            sample_loc->sample + order->sampleCount * (sample_loc->pixelX + sample_order_info->width * sample_loc->pixelY);
        sample_locations_mask |= 1ULL << idx;
    }

    uint64_t expected_mask = (order->sampleLocationCount == 64) ? ~0ULL : ((1ULL << order->sampleLocationCount) - 1);
    if (sample_locations_mask != expected_mask) {
        skip |= LogError(
            device, "VUID-VkCoarseSampleOrderCustomNV-pSampleLocations-02077",
            "The array pSampleLocations must contain exactly one entry for "
            "every combination of valid values for pixelX, pixelY, and sample in the structure VkCoarseSampleOrderCustomNV.");
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo *pCreateInfo,
                                                                     const VkAllocationCallbacks *pAllocator,
                                                                     VkPipelineLayout *pPipelineLayout) const {
    bool skip = false;
    // Validate layout count against device physical limit
    if (pCreateInfo->setLayoutCount > device_limits.maxBoundDescriptorSets) {
        skip |= LogError(device, "VUID-VkPipelineLayoutCreateInfo-setLayoutCount-00286",
                         "vkCreatePipelineLayout(): setLayoutCount (%" PRIu32
                         ") exceeds physical device maxBoundDescriptorSets limit (%" PRIu32 ").",
                         pCreateInfo->setLayoutCount, device_limits.maxBoundDescriptorSets);
    }

    if (!IsExtEnabled(device_extensions.vk_ext_graphics_pipeline_library)) {
        for (uint32_t i = 0; i < pCreateInfo->setLayoutCount; ++i) {
            if (!pCreateInfo->pSetLayouts[i]) {
                skip |= LogError(device, "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-06561",
                                 "vkCreatePipelineLayout(): pSetLayouts[%" PRIu32
                                 "] is VK_NULL_HANDLE, but VK_EXT_graphics_pipeline_library is not enabled.",
                                 i);
            }
        }
    }

    // Validate Push Constant ranges
    for (uint32_t i = 0; i < pCreateInfo->pushConstantRangeCount; ++i) {
        const uint32_t offset = pCreateInfo->pPushConstantRanges[i].offset;
        const uint32_t size = pCreateInfo->pPushConstantRanges[i].size;
        const uint32_t max_push_constants_size = device_limits.maxPushConstantsSize;
        // Check that offset + size don't exceed the max.
        // Prevent arithetic overflow here by avoiding addition and testing in this order.
        if (offset >= max_push_constants_size) {
            skip |= LogError(device, "VUID-VkPushConstantRange-offset-00294",
                             "vkCreatePipelineLayout(): pCreateInfo->pPushConstantRanges[%" PRIu32 "].offset (%" PRIu32
                             ") that exceeds this "
                             "device's maxPushConstantSize of %" PRIu32 ".",
                             i, offset, max_push_constants_size);
        }
        if (size > max_push_constants_size - offset) {
            skip |= LogError(device, "VUID-VkPushConstantRange-size-00298",
                             "vkCreatePipelineLayout(): pCreateInfo->pPushConstantRanges[%" PRIu32 "] offset (%" PRIu32
                             ") and size (%" PRIu32
                             ") "
                             "together exceeds this device's maxPushConstantSize of %" PRIu32 ".",
                             i, offset, size, max_push_constants_size);
        }

        // size needs to be non-zero and a multiple of 4.
        if (size == 0) {
            skip |= LogError(device, "VUID-VkPushConstantRange-size-00296",
                             "vkCreatePipelineLayout(): pCreateInfo->pPushConstantRanges[%" PRIu32 "].size (%" PRIu32
                             ") is not greater than zero.",
                             i, size);
        }
        if (size & 0x3) {
            skip |= LogError(device, "VUID-VkPushConstantRange-size-00297",
                             "vkCreatePipelineLayout(): pCreateInfo->pPushConstantRanges[%" PRIu32 "].size (%" PRIu32
                             ") is not a multiple of 4.",
                             i, size);
        }

        // offset needs to be a multiple of 4.
        if ((offset & 0x3) != 0) {
            skip |= LogError(device, "VUID-VkPushConstantRange-offset-00295",
                             "vkCreatePipelineLayout(): pCreateInfo->pPushConstantRanges[%" PRIu32 "].offset (%" PRIu32
                             ") is not a multiple of 4.",
                             i, offset);
        }
    }

    // As of 1.0.28, there is a VU that states that a stage flag cannot appear more than once in the list of push constant ranges.
    for (uint32_t i = 0; i < pCreateInfo->pushConstantRangeCount; ++i) {
        for (uint32_t j = i + 1; j < pCreateInfo->pushConstantRangeCount; ++j) {
            if (0 != (pCreateInfo->pPushConstantRanges[i].stageFlags & pCreateInfo->pPushConstantRanges[j].stageFlags)) {
                skip |=
                    LogError(device, "VUID-VkPipelineLayoutCreateInfo-pPushConstantRanges-00292",
                             "vkCreatePipelineLayout() Duplicate stage flags found in ranges %" PRIu32 " and %" PRIu32 ".", i, j);
            }
        }
    }
    return skip;
}

bool StatelessValidation::ValidatePipelineShaderStageCreateInfo(const char *func_name, const char *msg,
                                                                const VkPipelineShaderStageCreateInfo *pCreateInfo) const {
    bool skip = false;

    const auto *required_subgroup_size_features =
        LvlFindInChain<VkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT>(pCreateInfo->pNext);

    if (required_subgroup_size_features) {
        if ((pCreateInfo->flags & VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT_EXT) != 0) {
            skip |= LogError(
                device, "VUID-VkPipelineShaderStageCreateInfo-pNext-02754",
                "%s(): %s->flags (0x%x) includes VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT_EXT while "
                "VkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT is included in the pNext chain.",
                func_name, msg, pCreateInfo->flags);
        }
    }

    return skip;
}

// TODO Issue 4847 - Move logic to be autogenerated
bool StatelessValidation::ValidatePipelineTessellationStateCreateInfo(const VkPipelineTessellationStateCreateInfo &info,
                                                                      uint32_t index) const {
    bool skip = false;

    skip |= ValidateStructType("vkCreateGraphicsPipelines",
                               ParameterName("pCreateInfos[%i].pTessellationState", ParameterName::IndexVector{index}),
                               "VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO", &info,
                               VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO, false, kVUIDUndefined,
                               "VUID-VkPipelineTessellationStateCreateInfo-sType-sType");

    constexpr std::array allowed_structs = {VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_DOMAIN_ORIGIN_STATE_CREATE_INFO};

    skip |= ValidateStructPnext(
        "vkCreateGraphicsPipelines", ParameterName("pCreateInfos[%i].pTessellationState->pNext", ParameterName::IndexVector{index}),
        "VkPipelineTessellationDomainOriginStateCreateInfo", info.pNext, allowed_structs.size(), allowed_structs.data(),
        GeneratedVulkanHeaderVersion, "VUID-VkPipelineTessellationStateCreateInfo-pNext-pNext",
        "VUID-VkPipelineTessellationStateCreateInfo-sType-unique");

    skip |= ValidateReservedFlags("vkCreateGraphicsPipelines",
                                  ParameterName("pCreateInfos[%i].pTessellationState->flags", ParameterName::IndexVector{index}),
                                  info.flags, "VUID-VkPipelineTessellationStateCreateInfo-flags-zerobitmask");

    return skip;
}

// TODO Issue 4847 - Move logic to be autogenerated
bool StatelessValidation::ValidatePipelineVertexInputStateCreateInfo(const VkPipelineVertexInputStateCreateInfo &info,
                                                                     uint32_t index) const {
    bool skip = false;

    constexpr std::array allowed_structs = {VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_DIVISOR_STATE_CREATE_INFO_EXT};
    skip |= ValidateStructPnext(
        "vkCreateGraphicsPipelines", ParameterName("pCreateInfos[%i].pVertexInputState->pNext", ParameterName::IndexVector{index}),
        "VkPipelineVertexInputDivisorStateCreateInfoEXT", info.pNext, allowed_structs.size(), allowed_structs.data(),
        GeneratedVulkanHeaderVersion, "VUID-VkPipelineVertexInputStateCreateInfo-pNext-pNext",
        "VUID-VkPipelineVertexInputStateCreateInfo-sType-unique");
    skip |= ValidateStructType("vkCreateGraphicsPipelines",
                               ParameterName("pCreateInfos[%i].pVertexInputState", ParameterName::IndexVector{index}),
                               "VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO", &info,
                               VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO, false, kVUIDUndefined,
                               "VUID-VkPipelineVertexInputStateCreateInfo-sType-sType");
    skip |= ValidateArray(
        "vkCreateGraphicsPipelines",
        ParameterName("pCreateInfos[%i].pVertexInputState->vertexBindingDescriptionCount", ParameterName::IndexVector{index}),
        "pCreateInfos[i].pVertexInputState->pVertexBindingDescriptions", info.vertexBindingDescriptionCount,
        &info.pVertexBindingDescriptions, false, true, kVUIDUndefined,
        "VUID-VkPipelineVertexInputStateCreateInfo-pVertexBindingDescriptions-parameter");

    skip |= ValidateArray(
        "vkCreateGraphicsPipelines",
        ParameterName("pCreateInfos[%i].pVertexInputState->vertexAttributeDescriptionCount", ParameterName::IndexVector{index}),
        "pCreateInfos[i]->pVertexAttributeDescriptions", info.vertexAttributeDescriptionCount, &info.pVertexAttributeDescriptions,
        false, true, kVUIDUndefined, "VUID-VkPipelineVertexInputStateCreateInfo-pVertexAttributeDescriptions-parameter");

    if (info.pVertexBindingDescriptions != nullptr) {
        for (uint32_t vertex_binding_description_index = 0; vertex_binding_description_index < info.vertexBindingDescriptionCount;
             ++vertex_binding_description_index) {
            skip |=
                ValidateRangedEnum("vkCreateGraphicsPipelines",
                                   ParameterName("pCreateInfos[%i].pVertexInputState->pVertexBindingDescriptions[%i].inputRate",
                                                 ParameterName::IndexVector{index, vertex_binding_description_index}),
                                   "VkVertexInputRate", info.pVertexBindingDescriptions[vertex_binding_description_index].inputRate,
                                   "VUID-VkVertexInputBindingDescription-inputRate-parameter");
        }
    }

    if (info.pVertexAttributeDescriptions != nullptr) {
        for (uint32_t vertex_attribute_description_index = 0;
             vertex_attribute_description_index < info.vertexAttributeDescriptionCount; ++vertex_attribute_description_index) {
            const VkFormat format = info.pVertexAttributeDescriptions[vertex_attribute_description_index].format;
            skip |= ValidateRangedEnum("vkCreateGraphicsPipelines",
                                       ParameterName("pCreateInfos[%i].pVertexInputState->pVertexAttributeDescriptions[%i].format",
                                                     ParameterName::IndexVector{index, vertex_attribute_description_index}),
                                       "VkFormat", info.pVertexAttributeDescriptions[vertex_attribute_description_index].format,
                                       "VUID-VkVertexInputAttributeDescription-format-parameter");
            if (FormatIsDepthOrStencil(format)) {
                // Should never hopefully get here, but there are known driver advertising the wrong feature flags
                // see https://gitlab.khronos.org/vulkan/vulkan/-/merge_requests/4849
                skip |= LogError(device, kVUID_Core_invalidDepthStencilFormat,
                                 "vkCreateGraphicsPipelines: "
                                 "pCreateInfos[%" PRIu32 "].pVertexInputState->pVertexAttributeDescriptions[%" PRIu32
                                 "].format is a "
                                 "depth/stencil format (%s) but depth/stencil formats do not have a defined sizes for "
                                 "alignment, replace with a color format.",
                                 index, vertex_attribute_description_index, string_VkFormat(format));
            }
        }
    }

    skip |= ValidateReservedFlags("vkCreateGraphicsPipelines",
                                  ParameterName("pCreateInfos[%i].pVertexInputState->flags", ParameterName::IndexVector{index}),
                                  info.flags, "VUID-VkPipelineVertexInputStateCreateInfo-flags-zerobitmask");

    return skip;
}

// TODO Issue 4847 - Move logic to be autogenerated
bool StatelessValidation::ValidatePipelineViewportStateCreateInfo(const VkPipelineViewportStateCreateInfo &info,
                                                                  uint32_t index) const {
    bool skip = false;

    skip |= ValidateStructType(
        "vkCreateGraphicsPipelines", ParameterName("pCreateInfos[%i].pViewportState", ParameterName::IndexVector{index}),
        "VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO", &info, VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        false, kVUIDUndefined, "VUID-VkPipelineViewportStateCreateInfo-sType-sType");

    constexpr std::array allowed_structs = {
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_SWIZZLE_STATE_CREATE_INFO_NV,
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_W_SCALING_STATE_CREATE_INFO_NV,
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_EXCLUSIVE_SCISSOR_STATE_CREATE_INFO_NV,
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_SHADING_RATE_IMAGE_STATE_CREATE_INFO_NV,
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_COARSE_SAMPLE_ORDER_STATE_CREATE_INFO_NV,
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_DEPTH_CLIP_CONTROL_CREATE_INFO_EXT,
    };
    skip |= ValidateStructPnext(
        "vkCreateGraphicsPipelines", ParameterName("pCreateInfos[%i].pViewportState->pNext", ParameterName::IndexVector{index}),
        "VkPipelineViewportSwizzleStateCreateInfoNV, VkPipelineViewportWScalingStateCreateInfoNV, "
        "VkPipelineViewportExclusiveScissorStateCreateInfoNV, VkPipelineViewportShadingRateImageStateCreateInfoNV, "
        "VkPipelineViewportCoarseSampleOrderStateCreateInfoNV, VkPipelineViewportDepthClipControlCreateInfoEXT",
        info.pNext, allowed_structs.size(), allowed_structs.data(), GeneratedVulkanHeaderVersion,
        "VUID-VkPipelineViewportStateCreateInfo-pNext-pNext", "VUID-VkPipelineViewportStateCreateInfo-sType-unique");

    skip |= ValidateReservedFlags("vkCreateGraphicsPipelines",
                                  ParameterName("pCreateInfos[%i].pViewportState->flags", ParameterName::IndexVector{index}),
                                  info.flags, "VUID-VkPipelineViewportStateCreateInfo-flags-zerobitmask");

    return skip;
}

// TODO Issue 4847 - Move logic to be autogenerated
bool StatelessValidation::ValidatePipelineMultisampleStateCreateInfo(const VkPipelineMultisampleStateCreateInfo &info,
                                                                     uint32_t index) const {
    bool skip = false;

    skip |= ValidateStructType(
        "vkCreateGraphicsPipelines", ParameterName("pCreateInfos[%i].pMultisampleState", ParameterName::IndexVector{index}),
        "VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO", &info, VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        false, kVUIDUndefined, "VUID-VkPipelineMultisampleStateCreateInfo-sType-sType");

    constexpr std::array allowed_structs = {VK_STRUCTURE_TYPE_PIPELINE_COVERAGE_MODULATION_STATE_CREATE_INFO_NV,
                                            VK_STRUCTURE_TYPE_PIPELINE_COVERAGE_REDUCTION_STATE_CREATE_INFO_NV,
                                            VK_STRUCTURE_TYPE_PIPELINE_COVERAGE_TO_COLOR_STATE_CREATE_INFO_NV,
                                            VK_STRUCTURE_TYPE_PIPELINE_SAMPLE_LOCATIONS_STATE_CREATE_INFO_EXT};
    skip |= ValidateStructPnext(
        "vkCreateGraphicsPipelines", ParameterName("pCreateInfos[%i].pMultisampleState->pNext", ParameterName::IndexVector{index}),
        "VkPipelineCoverageModulationStateCreateInfoNV, VkPipelineCoverageReductionStateCreateInfoNV, "
        "VkPipelineCoverageToColorStateCreateInfoNV, VkPipelineSampleLocationsStateCreateInfoEXT",
        info.pNext, allowed_structs.size(), allowed_structs.data(), GeneratedVulkanHeaderVersion,
        "VUID-VkPipelineMultisampleStateCreateInfo-pNext-pNext", "VUID-VkPipelineMultisampleStateCreateInfo-sType-unique");

    skip |= ValidateReservedFlags("vkCreateGraphicsPipelines",
                                  ParameterName("pCreateInfos[%i].pMultisampleState->flags", ParameterName::IndexVector{index}),
                                  info.flags, "VUID-VkPipelineMultisampleStateCreateInfo-flags-zerobitmask");

    skip |=
        ValidateBool32("vkCreateGraphicsPipelines",
                       ParameterName("pCreateInfos[%i].pMultisampleState->sampleShadingEnable", ParameterName::IndexVector{index}),
                       info.sampleShadingEnable);

    skip |=
        ValidateArray("vkCreateGraphicsPipelines",
                      ParameterName("pCreateInfos[%i].pMultisampleState->rasterizationSamples", ParameterName::IndexVector{index}),
                      ParameterName("pCreateInfos[%i].pMultisampleState->pSampleMask", ParameterName::IndexVector{index}),
                      info.rasterizationSamples, &info.pSampleMask, true, false, kVUIDUndefined, kVUIDUndefined);

    skip |=
        ValidateFlags("vkCreateGraphicsPipelines",
                      ParameterName("pCreateInfos[%i].pMultisampleState->rasterizationSamples", ParameterName::IndexVector{index}),
                      "VkSampleCountFlagBits", AllVkSampleCountFlagBits, info.rasterizationSamples, kRequiredSingleBit,
                      "VUID-VkPipelineMultisampleStateCreateInfo-rasterizationSamples-parameter");

    skip |= ValidateBool32(
        "vkCreateGraphicsPipelines",
        ParameterName("pCreateInfos[%i].pMultisampleState->alphaToCoverageEnable", ParameterName::IndexVector{index}),
        info.alphaToCoverageEnable);

    skip |= ValidateBool32("vkCreateGraphicsPipelines",
                           ParameterName("pCreateInfos[%i].pMultisampleState->alphaToOneEnable", ParameterName::IndexVector{index}),
                           info.alphaToOneEnable);
    return skip;
}

// TODO Issue 4847 - Move logic to be autogenerated
bool StatelessValidation::ValidatePipelineColorBlendAttachmentState(const VkPipelineColorBlendAttachmentState &attachment_state,
                                                                    uint32_t pipe_index, uint32_t attachment_index) const {
    bool skip = false;

    skip |= ValidateBool32("vkCreateGraphicsPipelines",
                           ParameterName("pCreateInfos[%i].pColorBlendState->pAttachments[%i].blendEnable",
                                         ParameterName::IndexVector{pipe_index, attachment_index}),
                           attachment_state.blendEnable);

    skip |= ValidateRangedEnum("vkCreateGraphicsPipelines",
                               ParameterName("pCreateInfos[%i].pColorBlendState->pAttachments[%i].srcColorBlendFactor",
                                             ParameterName::IndexVector{pipe_index, attachment_index}),
                               "VkBlendFactor", attachment_state.srcColorBlendFactor,
                               "VUID-VkPipelineColorBlendAttachmentState-srcColorBlendFactor-parameter");

    skip |= ValidateRangedEnum("vkCreateGraphicsPipelines",
                               ParameterName("pCreateInfos[%i].pColorBlendState->pAttachments[%i].dstColorBlendFactor",
                                             ParameterName::IndexVector{pipe_index, attachment_index}),
                               "VkBlendFactor", attachment_state.dstColorBlendFactor,
                               "VUID-VkPipelineColorBlendAttachmentState-dstColorBlendFactor-parameter");

    skip |= ValidateRangedEnum("vkCreateGraphicsPipelines",
                               ParameterName("pCreateInfos[%i].pColorBlendState->pAttachments[%i].colorBlendOp",
                                             ParameterName::IndexVector{pipe_index, attachment_index}),
                               "VkBlendOp", attachment_state.colorBlendOp,
                               "VUID-VkPipelineColorBlendAttachmentState-colorBlendOp-parameter");

    skip |= ValidateRangedEnum("vkCreateGraphicsPipelines",
                               ParameterName("pCreateInfos[%i].pColorBlendState->pAttachments[%i].srcAlphaBlendFactor",
                                             ParameterName::IndexVector{pipe_index, attachment_index}),
                               "VkBlendFactor", attachment_state.srcAlphaBlendFactor,
                               "VUID-VkPipelineColorBlendAttachmentState-srcAlphaBlendFactor-parameter");

    skip |= ValidateRangedEnum("vkCreateGraphicsPipelines",
                               ParameterName("pCreateInfos[%i].pColorBlendState->pAttachments[%i].dstAlphaBlendFactor",
                                             ParameterName::IndexVector{pipe_index, attachment_index}),
                               "VkBlendFactor", attachment_state.dstAlphaBlendFactor,
                               "VUID-VkPipelineColorBlendAttachmentState-dstAlphaBlendFactor-parameter");

    skip |= ValidateRangedEnum("vkCreateGraphicsPipelines",
                               ParameterName("pCreateInfos[%i].pColorBlendState->pAttachments[%i].alphaBlendOp",
                                             ParameterName::IndexVector{pipe_index, attachment_index}),
                               "VkBlendOp", attachment_state.alphaBlendOp,
                               "VUID-VkPipelineColorBlendAttachmentState-alphaBlendOp-parameter");

    skip |= ValidateFlags("vkCreateGraphicsPipelines",
                          ParameterName("pCreateInfos[%i].pColorBlendState->pAttachments[%i].colorWriteMask",
                                        ParameterName::IndexVector{pipe_index, attachment_index}),
                          "VkColorComponentFlagBits", AllVkColorComponentFlagBits, attachment_state.colorWriteMask, kOptionalFlags,
                          "VUID-VkPipelineColorBlendAttachmentState-colorWriteMask-parameter");

    return skip;
}

// TODO Issue 4847 - Move logic to be autogenerated
bool StatelessValidation::ValidatePipelineColorBlendStateCreateInfo(const VkPipelineColorBlendStateCreateInfo &info,
                                                                    uint32_t index) const {
    bool skip = false;

    skip |= ValidateStructType(
        "vkCreateGraphicsPipelines", ParameterName("pCreateInfos[%i].pColorBlendState", ParameterName::IndexVector{index}),
        "VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO", &info, VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        false, kVUIDUndefined, "VUID-VkPipelineColorBlendStateCreateInfo-sType-sType");

    constexpr std::array allowed_structs = {VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_ADVANCED_STATE_CREATE_INFO_EXT,
                                            VK_STRUCTURE_TYPE_PIPELINE_COLOR_WRITE_CREATE_INFO_EXT};

    skip |= ValidateStructPnext(
        "vkCreateGraphicsPipelines", ParameterName("pCreateInfos[%i].pColorBlendState->pNext", ParameterName::IndexVector{index}),
        "VkPipelineColorBlendAdvancedStateCreateInfoEXT, VkPipelineColorWriteCreateInfoEXT", info.pNext, allowed_structs.size(),
        allowed_structs.data(), GeneratedVulkanHeaderVersion, "VUID-VkPipelineColorBlendStateCreateInfo-pNext-pNext",
        "VUID-VkPipelineColorBlendStateCreateInfo-sType-unique");

    skip |= ValidateFlags("vkCreateGraphicsPipelines",
                          ParameterName("pCreateInfos[%i].pColorBlendState->flags", ParameterName::IndexVector{index}),
                          "VkPipelineColorBlendStateCreateFlagBits", AllVkPipelineColorBlendStateCreateFlagBits, info.flags,
                          kOptionalFlags, "VUID-VkPipelineColorBlendStateCreateInfo-flags-parameter");

    skip |= ValidateBool32("vkCreateGraphicsPipelines",
                           ParameterName("pCreateInfos[%i].pColorBlendState->logicOpEnable", ParameterName::IndexVector{index}),
                           info.logicOpEnable);

    return skip;
}

// TODO Issue 4847 - Move logic to be autogenerated
bool StatelessValidation::ValidatePipelineDepthStencilStateCreateInfo(const VkPipelineDepthStencilStateCreateInfo &info,
                                                                      uint32_t index) const {
    bool skip = false;

    skip |= ValidateStructType("vkCreateGraphicsPipelines",
                               ParameterName("pCreateInfos[%i].pDepthStencilState", ParameterName::IndexVector{index}),
                               "VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO", &info,
                               VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO, false, kVUIDUndefined,
                               "VUID-VkPipelineDepthStencilStateCreateInfo-sType-sType");

    skip |= ValidateStructPnext("vkCreateGraphicsPipelines",
                                ParameterName("pCreateInfos[%i].pDepthStencilState->pNext", ParameterName::IndexVector{index}),
                                nullptr, info.pNext, 0, nullptr, GeneratedVulkanHeaderVersion,
                                "VUID-VkPipelineDepthStencilStateCreateInfo-pNext-pNext", nullptr);

    skip |= ValidateFlags("vkCreateGraphicsPipelines",
                          ParameterName("pCreateInfos[%i].pDepthStencilState->flags", ParameterName::IndexVector{index}),
                          "VkPipelineDepthStencilStateCreateFlagBits", AllVkPipelineDepthStencilStateCreateFlagBits, info.flags,
                          kOptionalFlags, "VUID-VkPipelineDepthStencilStateCreateInfo-flags-parameter");

    skip |= ValidateBool32("vkCreateGraphicsPipelines",
                           ParameterName("pCreateInfos[%i].pDepthStencilState->depthTestEnable", ParameterName::IndexVector{index}),
                           info.depthTestEnable);

    skip |=
        ValidateBool32("vkCreateGraphicsPipelines",
                       ParameterName("pCreateInfos[%i].pDepthStencilState->depthWriteEnable", ParameterName::IndexVector{index}),
                       info.depthWriteEnable);

    skip |= ValidateRangedEnum(
        "vkCreateGraphicsPipelines",
        ParameterName("pCreateInfos[%i].pDepthStencilState->depthCompareOp", ParameterName::IndexVector{index}), "VkCompareOp",
        info.depthCompareOp, "VUID-VkPipelineDepthStencilStateCreateInfo-depthCompareOp-parameter");

    skip |= ValidateBool32(
        "vkCreateGraphicsPipelines",
        ParameterName("pCreateInfos[%i].pDepthStencilState->depthBoundsTestEnable", ParameterName::IndexVector{index}),
        info.depthBoundsTestEnable);

    skip |=
        ValidateBool32("vkCreateGraphicsPipelines",
                       ParameterName("pCreateInfos[%i].pDepthStencilState->stencilTestEnable", ParameterName::IndexVector{index}),
                       info.stencilTestEnable);

    skip |=
        ValidateRangedEnum("vkCreateGraphicsPipelines",
                           ParameterName("pCreateInfos[%i].pDepthStencilState->front.failOp", ParameterName::IndexVector{index}),
                           "VkStencilOp", info.front.failOp, "VUID-VkStencilOpState-failOp-parameter");

    skip |=
        ValidateRangedEnum("vkCreateGraphicsPipelines",
                           ParameterName("pCreateInfos[%i].pDepthStencilState->front.passOp", ParameterName::IndexVector{index}),
                           "VkStencilOp", info.front.passOp, "VUID-VkStencilOpState-passOp-parameter");

    skip |= ValidateRangedEnum(
        "vkCreateGraphicsPipelines",
        ParameterName("pCreateInfos[%i].pDepthStencilState->front.depthFailOp", ParameterName::IndexVector{index}), "VkStencilOp",
        info.front.depthFailOp, "VUID-VkStencilOpState-depthFailOp-parameter");

    skip |= ValidateRangedEnum(
        "vkCreateGraphicsPipelines",
        ParameterName("pCreateInfos[%i].pDepthStencilState->front.compareOp", ParameterName::IndexVector{index}), "VkCompareOp",
        info.front.compareOp, "VUID-VkPipelineDepthStencilStateCreateInfo-depthCompareOp-parameter");

    skip |= ValidateRangedEnum("vkCreateGraphicsPipelines",
                               ParameterName("pCreateInfos[%i].pDepthStencilState->back.failOp", ParameterName::IndexVector{index}),
                               "VkStencilOp", info.back.failOp, "VUID-VkStencilOpState-failOp-parameter");

    skip |= ValidateRangedEnum("vkCreateGraphicsPipelines",
                               ParameterName("pCreateInfos[%i].pDepthStencilState->back.passOp", ParameterName::IndexVector{index}),
                               "VkStencilOp", info.back.passOp, "VUID-VkStencilOpState-passOp-parameter");

    skip |= ValidateRangedEnum(
        "vkCreateGraphicsPipelines",
        ParameterName("pCreateInfos[%i].pDepthStencilState->back.depthFailOp", ParameterName::IndexVector{index}), "VkStencilOp",
        info.back.depthFailOp, "VUID-VkStencilOpState-depthFailOp-parameter");

    skip |= ValidateRangedEnum(
        "vkCreateGraphicsPipelines",
        ParameterName("pCreateInfos[%i].pDepthStencilState->back.compareOp", ParameterName::IndexVector{index}), "VkCompareOp",
        info.back.compareOp, "VUID-VkPipelineDepthStencilStateCreateInfo-depthCompareOp-parameter");

    return skip;
}

// TODO Issue 4847 - Move logic to be autogenerated
bool StatelessValidation::ValidatePipelineInputAssemblyStateCreateInfo(const VkPipelineInputAssemblyStateCreateInfo &info,
                                                                       uint32_t index) const {
    bool skip = false;

    skip |= ValidateStructType("vkCreateGraphicsPipelines",
                               ParameterName("pCreateInfos[%i].pInputAssemblyState", ParameterName::IndexVector{index}),
                               "VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO", &info,
                               VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, false, kVUIDUndefined,
                               "VUID-VkPipelineInputAssemblyStateCreateInfo-sType-sType");

    skip |= ValidateStructPnext("vkCreateGraphicsPipelines",
                                ParameterName("pCreateInfos[%i].pInputAssemblyState->pNext", ParameterName::IndexVector{index}),
                                nullptr, info.pNext, 0, nullptr, GeneratedVulkanHeaderVersion,
                                "VUID-VkPipelineInputAssemblyStateCreateInfo-pNext-pNext", nullptr);

    skip |= ValidateReservedFlags("vkCreateGraphicsPipelines",
                                  ParameterName("pCreateInfos[%i].pInputAssemblyState->flags", ParameterName::IndexVector{index}),
                                  info.flags, "VUID-VkPipelineInputAssemblyStateCreateInfo-flags-zerobitmask");

    skip |=
        ValidateRangedEnum("vkCreateGraphicsPipelines",
                           ParameterName("pCreateInfos[%i].pInputAssemblyState->topology", ParameterName::IndexVector{index}),
                           "VkPrimitiveTopology", info.topology, "VUID-VkPipelineInputAssemblyStateCreateInfo-topology-parameter");

    skip |= ValidateBool32(
        "vkCreateGraphicsPipelines",
        ParameterName("pCreateInfos[%i].pInputAssemblyState->primitiveRestartEnable", ParameterName::IndexVector{index}),
        info.primitiveRestartEnable);

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
            // Create a copy of create_info and set non-included sub-state to null
            auto create_info = pCreateInfos[i];
            const auto *graphics_lib_info = LvlFindInChain<VkGraphicsPipelineLibraryCreateInfoEXT>(create_info.pNext);
            if (graphics_lib_info) {
                if (!(graphics_lib_info->flags & VK_GRAPHICS_PIPELINE_LIBRARY_VERTEX_INPUT_INTERFACE_BIT_EXT)) {
                    create_info.pVertexInputState = nullptr;
                    create_info.pInputAssemblyState = nullptr;
                }
                if (!(graphics_lib_info->flags & VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT)) {
                    create_info.pViewportState = nullptr;
                    create_info.pRasterizationState = nullptr;
                    create_info.pTessellationState = nullptr;
                }
                if (!(graphics_lib_info->flags & VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT)) {
                    create_info.pDepthStencilState = nullptr;
                }
                if (!(graphics_lib_info->flags & VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT)) {
                    create_info.pColorBlendState = nullptr;
                }
                if (!(graphics_lib_info->flags & (VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT |
                                                  VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT))) {
                    create_info.pMultisampleState = nullptr;
                }
                if (!(graphics_lib_info->flags & (VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT |
                                                  VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT))) {
                    create_info.layout = VK_NULL_HANDLE;
                }
                if (!(graphics_lib_info->flags & (VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT |
                                                  VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT |
                                                  VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT))) {
                    create_info.renderPass = VK_NULL_HANDLE;
                    create_info.subpass = 0;
                }
            }

            // Values needed from either dynamic rendering or the subpass description
            uint32_t color_attachment_count = 0;

            if (!create_info.renderPass) {
                if (create_info.pColorBlendState && create_info.pMultisampleState) {
                    const auto rendering_struct = LvlFindInChain<VkPipelineRenderingCreateInfo>(create_info.pNext);
                    // Pipeline has fragment output state
                    if (rendering_struct) {
                        color_attachment_count = rendering_struct->colorAttachmentCount;

                        if ((rendering_struct->depthAttachmentFormat != VK_FORMAT_UNDEFINED)) {
                            skip |= ValidateRangedEnum("VkPipelineRenderingCreateInfo", "stencilAttachmentFormat", "VkFormat",
                                                       rendering_struct->stencilAttachmentFormat,
                                                       "VUID-VkGraphicsPipelineCreateInfo-renderPass-06583");

                            if (!FormatHasDepth(rendering_struct->depthAttachmentFormat)) {
                                skip |= LogError(
                                    device, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06587",
                                    "vkCreateGraphicsPipelines() pCreateInfos[%" PRIu32
                                    "]: VkPipelineRenderingCreateInfo::depthAttachmentFormat (%s) does not have a depth aspect.",
                                    i, string_VkFormat(rendering_struct->depthAttachmentFormat));
                            }
                        }

                        if ((rendering_struct->stencilAttachmentFormat != VK_FORMAT_UNDEFINED)) {
                            skip |= ValidateRangedEnum("VkPipelineRenderingCreateInfo", "stencilAttachmentFormat", "VkFormat",
                                                       rendering_struct->stencilAttachmentFormat,
                                                       "VUID-VkGraphicsPipelineCreateInfo-renderPass-06584");
                            if (!FormatHasStencil(rendering_struct->stencilAttachmentFormat)) {
                                skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06588",
                                                 "vkCreateGraphicsPipelines() pCreateInfos[%" PRIu32
                                                 "]: VkPipelineRenderingCreateInfo::stencilAttachmentFormat  (%s) does not have a "
                                                 "stencil aspect.",
                                                 i, string_VkFormat(rendering_struct->stencilAttachmentFormat));
                            }
                        }

                        if (color_attachment_count != 0) {
                            skip |= ValidateRangedEnumArray(
                                "VkPipelineRenderingCreateInfo", "VUID-VkGraphicsPipelineCreateInfo-renderPass-06579",
                                "colorAttachmentCount", "pColorAttachmentFormats", "VkFormat", color_attachment_count,
                                rendering_struct->pColorAttachmentFormats, true, true);
                        }

                        if (rendering_struct->pColorAttachmentFormats) {
                            for (uint32_t j = 0; j < color_attachment_count; ++j) {
                                skip |= ValidateRangedEnum("VkPipelineRenderingCreateInfo", "pColorAttachmentFormats", "VkFormat",
                                                           rendering_struct->pColorAttachmentFormats[j],
                                                           "VUID-VkGraphicsPipelineCreateInfo-renderPass-06580");
                            }
                        }
                    }

                    // VkAttachmentSampleCountInfoAMD == VkAttachmentSampleCountInfoNV
                    auto attachment_sample_count_info = LvlFindInChain<VkAttachmentSampleCountInfoAMD>(create_info.pNext);
                    if (attachment_sample_count_info && attachment_sample_count_info->pColorAttachmentSamples) {
                        color_attachment_count = attachment_sample_count_info->colorAttachmentCount;

                        for (uint32_t j = 0; j < color_attachment_count; ++j) {
                            skip |= ValidateFlags("vkCreateGraphicsPipelines",
                                                  ParameterName("VkAttachmentSampleCountInfoAMD->pColorAttachmentSamples"),
                                                  "VkSampleCountFlagBits", AllVkSampleCountFlagBits,
                                                  attachment_sample_count_info->pColorAttachmentSamples[j], kRequiredFlags,
                                                  "VUID-VkGraphicsPipelineCreateInfo-pColorAttachmentSamples-06592");
                        }
                    }
                }
            }

            const VkPipelineCreateFlags flags = create_info.flags;
            if (!IsExtEnabled(device_extensions.vk_ext_graphics_pipeline_library)) {
                if (create_info.stageCount == 0) {
                    skip |=
                        LogError(device, "VUID-VkGraphicsPipelineCreateInfo-stageCount-06604",
                                 "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32 "].stageCount is 0, but %s is not enabled", i,
                                 VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
                }
                if ((flags & VK_PIPELINE_CREATE_LIBRARY_BIT_KHR) != 0) {
                    skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-flags-03371",
                                     "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                     "]->flags (0x%x) must not include VK_PIPELINE_CREATE_LIBRARY_BIT_KHR.",
                                     i, flags);
                }

                // TODO while PRIu32 should probably be used instead of %i below, %i is necessary due to
                // ParameterName::IndexFormatSpecifier
                skip |= ValidateStructTypeArray(
                    "vkCreateGraphicsPipelines", ParameterName("pCreateInfos[%i].stageCount", ParameterName::IndexVector{i}),
                    ParameterName("pCreateInfos[%i].pStages", ParameterName::IndexVector{i}),
                    "VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO", create_info.stageCount, create_info.pStages,
                    VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, true, true,
                    "VUID-VkPipelineShaderStageCreateInfo-sType-sType", "VUID-VkGraphicsPipelineCreateInfo-pStages-06600",
                    "VUID-VkGraphicsPipelineCreateInfo-pStages-06600");
                skip |=
                    ValidateStructType("vkCreateGraphicsPipelines",
                                       ParameterName("pCreateInfos[%i].pRasterizationState", ParameterName::IndexVector{i}),
                                       "VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO",
                                       create_info.pRasterizationState, VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
                                       true, "VUID-VkGraphicsPipelineCreateInfo-pRasterizationState-06601",
                                       "VUID-VkPipelineRasterizationStateCreateInfo-sType-sType");
            }

            // <VkDynamicState, index in pDynamicStates, hash for enum key>
            vvl::unordered_map<VkDynamicState, uint32_t, std::hash<int>> dynamic_state_map;
            // TODO probably should check dynamic state from graphics libraries, at least when creating an "executable pipeline"
            if (create_info.pDynamicState != nullptr) {
                const auto &dynamic_state_info = *create_info.pDynamicState;
                for (uint32_t state_index = 0; state_index < dynamic_state_info.dynamicStateCount; ++state_index) {
                    const VkDynamicState dynamic_state = dynamic_state_info.pDynamicStates[state_index];

                    if (vvl::Contains(dynamic_state_map, dynamic_state)) {
                        skip |= LogError(device, "VUID-VkPipelineDynamicStateCreateInfo-pDynamicStates-01442",
                                         "vkCreateGraphicsPipelines: %s was listed twice in the "
                                         "pCreateInfos[%" PRIu32 "].pDynamicState->pDynamicStates array at pDynamicStates[%" PRIu32
                                         "] and pDynamicStates[%" PRIu32 "]",
                                         string_VkDynamicState(dynamic_state), i, dynamic_state_map[dynamic_state], state_index);
                    }

                    dynamic_state_map[dynamic_state] = state_index;
                }
            }

            if (vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_RAY_TRACING_PIPELINE_STACK_SIZE_KHR)) {
                // Not allowed for graphics pipelines
                skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-03578",
                                 "vkCreateGraphicsPipelines: VK_DYNAMIC_STATE_RAY_TRACING_PIPELINE_STACK_SIZE_KHR was listed the "
                                 "pCreateInfos[%" PRIu32 "].pDynamicState->pDynamicStates[%" PRIu32
                                 "] but not allowed in graphic pipelines.",
                                 i, dynamic_state_map[VK_DYNAMIC_STATE_RAY_TRACING_PIPELINE_STACK_SIZE_KHR]);
            }

            if (vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT_EXT) &&
                vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_VIEWPORT)) {
                skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-04132",
                                 "vkCreateGraphicsPipelines: VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT_EXT and "
                                 "VK_DYNAMIC_STATE_VIEWPORT both listed in pCreateInfos[%" PRIu32
                                 "].pDynamicState->pDynamicStates array at pDynamicStates[%" PRIu32 "] and pDynamicStates[%" PRIu32
                                 "] respectfully.",
                                 i, dynamic_state_map[VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT_EXT],
                                 dynamic_state_map[VK_DYNAMIC_STATE_VIEWPORT]);
            }

            if (vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT_EXT) &&
                vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_SCISSOR)) {
                skip |= LogError(
                    device, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-04133",
                    "vkCreateGraphicsPipelines: VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT_EXT and VK_DYNAMIC_STATE_SCISSOR "
                    "both listed in pCreateInfos[%" PRIu32 "].pDynamicState->pDynamicStates array at pDynamicStates[%" PRIu32
                    "] and pDynamicStates[%" PRIu32 "] respectfully.",
                    i, dynamic_state_map[VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT_EXT], dynamic_state_map[VK_DYNAMIC_STATE_SCISSOR]);
            }

            auto feedback_struct = LvlFindInChain<VkPipelineCreationFeedbackCreateInfoEXT>(create_info.pNext);
            if ((feedback_struct != nullptr) && (feedback_struct->pipelineStageCreationFeedbackCount != 0 &&
                                                 feedback_struct->pipelineStageCreationFeedbackCount != create_info.stageCount)) {
                skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-pipelineStageCreationFeedbackCount-06594",
                                 "vkCreateGraphicsPipelines(): in pCreateInfo[%" PRIu32
                                 "], VkPipelineCreationFeedbackEXT::pipelineStageCreationFeedbackCount"
                                 "(=%" PRIu32 ") must equal VkGraphicsPipelineCreateInfo::stageCount(=%" PRIu32 ").",
                                 i, feedback_struct->pipelineStageCreationFeedbackCount, create_info.stageCount);
            }

            // helpers for bool used multiple times below
            const bool has_dynamic_viewport = vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_VIEWPORT);
            const bool has_dynamic_scissor = vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_SCISSOR);
            const bool has_dynamic_viewport_w_scaling_nv =
                vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_NV);
            const bool has_dynamic_exclusive_scissor_nv =
                vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_NV);
            const bool has_dynamic_viewport_with_count =
                vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT_EXT);
            const bool has_dynamic_scissor_with_count =
                vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT_EXT);

            // Validation for parameters excluded from the generated validation code due to a 'noautovalidity' tag in vk.xml

            // Collect active stages and other information
            // Only want to loop through pStages once
            uint32_t active_shaders = 0;
            if (create_info.pStages != nullptr) {
                for (uint32_t stage_index = 0; stage_index < create_info.stageCount; ++stage_index) {
                    active_shaders |= create_info.pStages[stage_index].stage;

                    skip |= ValidateRequiredPointer(
                        "vkCreateGraphicsPipelines",
                        ParameterName("pCreateInfos[%i].stage[%i].pName", ParameterName::IndexVector{i, stage_index}),
                        create_info.pStages[stage_index].pName, "VUID-VkPipelineShaderStageCreateInfo-pName-parameter");

                    if (create_info.pStages[stage_index].pName) {
                        skip |= ValidateString(
                            "vkCreateGraphicsPipelines",
                            ParameterName("pCreateInfos[%i].pStages[%i].pName", ParameterName::IndexVector{i, stage_index}),
                            "VUID-VkPipelineShaderStageCreateInfo-pName-parameter", create_info.pStages[stage_index].pName);
                    }

                    std::stringstream msg;
                    msg << "pCreateInfos[%" << i << "].pStages[%" << stage_index << "]";
                    ValidatePipelineShaderStageCreateInfo("vkCreateGraphicsPipelines", msg.str().c_str(),
                                                          &create_info.pStages[stage_index]);
                }
            }

            if ((active_shaders & VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT) &&
                (active_shaders & VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT)) {
                if (create_info.pTessellationState == nullptr) {
                    skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-pStages-00731",
                                     "vkCreateGraphicsPipelines: if pCreateInfos[%" PRIu32
                                     "].pStages includes a tessellation control "
                                     "shader stage and a tessellation evaluation shader stage, "
                                     "pCreateInfos[%" PRIu32 "].pTessellationState must not be NULL.",
                                     i, i);
                } else {
                    skip |= ValidatePipelineTessellationStateCreateInfo(*create_info.pTessellationState, i);

                    if (create_info.pTessellationState->patchControlPoints == 0 ||
                        create_info.pTessellationState->patchControlPoints > device_limits.maxTessellationPatchSize) {
                        skip |=
                            LogError(device, "VUID-VkPipelineTessellationStateCreateInfo-patchControlPoints-01214",
                                     "vkCreateGraphicsPipelines: invalid parameter "
                                     "pCreateInfos[%" PRIu32 "].pTessellationState->patchControlPoints value %" PRIu32
                                     ". patchControlPoints "
                                     "should be >0 and <=%" PRIu32 ".",
                                     i, create_info.pTessellationState->patchControlPoints, device_limits.maxTessellationPatchSize);
                    }
                }
            }

            if (!(active_shaders & VK_SHADER_STAGE_MESH_BIT_NV) && (create_info.pInputAssemblyState != nullptr)) {
                skip |= ValidatePipelineInputAssemblyStateCreateInfo(*create_info.pInputAssemblyState, i);
            }

            if (!(active_shaders & VK_SHADER_STAGE_MESH_BIT_NV) && (create_info.pVertexInputState != nullptr)) {
                auto const &vertex_input_state = create_info.pVertexInputState;

                skip |= ValidatePipelineVertexInputStateCreateInfo(*vertex_input_state, i);

                if (vertex_input_state->vertexBindingDescriptionCount > device_limits.maxVertexInputBindings) {
                    skip |= LogError(device, "VUID-VkPipelineVertexInputStateCreateInfo-vertexBindingDescriptionCount-00613",
                                     "vkCreateGraphicsPipelines: pararameter "
                                     "pCreateInfo[%" PRIu32 "].pVertexInputState->vertexBindingDescriptionCount (%" PRIu32
                                     ") is "
                                     "greater than VkPhysicalDeviceLimits::maxVertexInputBindings (%" PRIu32 ").",
                                     i, vertex_input_state->vertexBindingDescriptionCount, device_limits.maxVertexInputBindings);
                }

                if (vertex_input_state->vertexAttributeDescriptionCount > device_limits.maxVertexInputAttributes) {
                    skip |=
                        LogError(device, "VUID-VkPipelineVertexInputStateCreateInfo-vertexAttributeDescriptionCount-00614",
                                 "vkCreateGraphicsPipelines: pararameter "
                                 "pCreateInfo[%" PRIu32 "].pVertexInputState->vertexAttributeDescriptionCount (%" PRIu32
                                 ") is "
                                 "greater than VkPhysicalDeviceLimits::maxVertexInputAttributes (%" PRIu32 ").",
                                 i, vertex_input_state->vertexAttributeDescriptionCount, device_limits.maxVertexInputAttributes);
                }

                vvl::unordered_set<uint32_t> vertex_bindings(vertex_input_state->vertexBindingDescriptionCount);
                for (uint32_t d = 0; d < vertex_input_state->vertexBindingDescriptionCount; ++d) {
                    auto const &vertex_bind_desc = vertex_input_state->pVertexBindingDescriptions[d];
                    auto const &binding_it = vertex_bindings.find(vertex_bind_desc.binding);
                    if (binding_it != vertex_bindings.cend()) {
                        skip |= LogError(device, "VUID-VkPipelineVertexInputStateCreateInfo-pVertexBindingDescriptions-00616",
                                         "vkCreateGraphicsPipelines: parameter "
                                         "pCreateInfo[%" PRIu32 "].pVertexInputState->pVertexBindingDescription[%" PRIu32
                                         "].binding "
                                         "(%" PRIu32 ") is already in pVertexBindingDescription[%" PRIu32 "]",
                                         i, d, vertex_bind_desc.binding, *binding_it);
                    }
                    vertex_bindings.insert(vertex_bind_desc.binding);

                    if (vertex_bind_desc.binding >= device_limits.maxVertexInputBindings) {
                        skip |= LogError(device, "VUID-VkVertexInputBindingDescription-binding-00618",
                                         "vkCreateGraphicsPipelines: parameter "
                                         "pCreateInfos[%" PRIu32 "].pVertexInputState->pVertexBindingDescriptions[%" PRIu32
                                         "].binding (%" PRIu32
                                         ") is "
                                         "greater than or equal to VkPhysicalDeviceLimits::maxVertexInputBindings (%" PRIu32 ").",
                                         i, d, vertex_bind_desc.binding, device_limits.maxVertexInputBindings);
                    }

                    if (vertex_bind_desc.stride > device_limits.maxVertexInputBindingStride) {
                        skip |= LogError(device, "VUID-VkVertexInputBindingDescription-stride-00619",
                                         "vkCreateGraphicsPipelines: parameter "
                                         "pCreateInfos[%" PRIu32 "].pVertexInputState->pVertexBindingDescriptions[%" PRIu32
                                         "].stride (%" PRIu32
                                         ") is greater "
                                         "than VkPhysicalDeviceLimits::maxVertexInputBindingStride (%" PRIu32 ").",
                                         i, d, vertex_bind_desc.stride, device_limits.maxVertexInputBindingStride);
                    }
                }

                vvl::unordered_set<uint32_t> attribute_locations(vertex_input_state->vertexAttributeDescriptionCount);
                for (uint32_t d = 0; d < vertex_input_state->vertexAttributeDescriptionCount; ++d) {
                    auto const &vertex_attrib_desc = vertex_input_state->pVertexAttributeDescriptions[d];
                    auto const &location_it = attribute_locations.find(vertex_attrib_desc.location);
                    if (location_it != attribute_locations.cend()) {
                        skip |= LogError(device, "VUID-VkPipelineVertexInputStateCreateInfo-pVertexAttributeDescriptions-00617",
                                         "vkCreateGraphicsPipelines: parameter "
                                         "pCreateInfo[%" PRIu32 "].pVertexInputState->pVertexAttributeDescriptions[%" PRIu32
                                         "].location (%" PRIu32 ") is already in pVertexAttributeDescriptions[%" PRIu32 "].",
                                         i, d, vertex_attrib_desc.location, *location_it);
                    }
                    attribute_locations.insert(vertex_attrib_desc.location);

                    auto const &binding_it = vertex_bindings.find(vertex_attrib_desc.binding);
                    if (binding_it == vertex_bindings.cend()) {
                        skip |= LogError(device, "VUID-VkPipelineVertexInputStateCreateInfo-binding-00615",
                                         "vkCreateGraphicsPipelines: parameter "
                                         " pCreateInfo[%" PRIu32 "].pVertexInputState->pVertexAttributeDescriptions[%" PRIu32
                                         "].binding (%" PRIu32
                                         ") does not exist "
                                         "in any pCreateInfo[%" PRIu32 "].pVertexInputState->pVertexBindingDescription.",
                                         i, d, vertex_attrib_desc.binding, i);
                    }

                    if (vertex_attrib_desc.location >= device_limits.maxVertexInputAttributes) {
                        skip |= LogError(device, "VUID-VkVertexInputAttributeDescription-location-00620",
                                         "vkCreateGraphicsPipelines: parameter "
                                         "pCreateInfos[%" PRIu32 "].pVertexInputState->pVertexAttributeDescriptions[%" PRIu32
                                         "].location (%" PRIu32
                                         ") is "
                                         "greater than or equal to VkPhysicalDeviceLimits::maxVertexInputAttributes (%" PRIu32 ").",
                                         i, d, vertex_attrib_desc.location, device_limits.maxVertexInputAttributes);
                    }

                    if (vertex_attrib_desc.binding >= device_limits.maxVertexInputBindings) {
                        skip |= LogError(device, "VUID-VkVertexInputAttributeDescription-binding-00621",
                                         "vkCreateGraphicsPipelines: parameter "
                                         "pCreateInfos[%" PRIu32 "].pVertexInputState->pVertexAttributeDescriptions[%" PRIu32
                                         "].binding (%" PRIu32
                                         ") is "
                                         "greater than or equal to VkPhysicalDeviceLimits::maxVertexInputBindings (%" PRIu32 ").",
                                         i, d, vertex_attrib_desc.binding, device_limits.maxVertexInputBindings);
                    }

                    if (vertex_attrib_desc.offset > device_limits.maxVertexInputAttributeOffset) {
                        skip |= LogError(device, "VUID-VkVertexInputAttributeDescription-offset-00622",
                                         "vkCreateGraphicsPipelines: parameter "
                                         "pCreateInfos[%" PRIu32 "].pVertexInputState->pVertexAttributeDescriptions[%" PRIu32
                                         "].offset (%" PRIu32
                                         ") is "
                                         "greater than VkPhysicalDeviceLimits::maxVertexInputAttributeOffset (%" PRIu32 ").",
                                         i, d, vertex_attrib_desc.offset, device_limits.maxVertexInputAttributeOffset);
                    }

                    VkFormatProperties properties;
                    DispatchGetPhysicalDeviceFormatProperties(physical_device, vertex_attrib_desc.format, &properties);
                    if ((properties.bufferFeatures & VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT) == 0) {
                        skip |= LogError(device, "VUID-VkVertexInputAttributeDescription-format-00623",
                                         "vkCreateGraphicsPipelines: pCreateInfo[%" PRIu32
                                         "].pVertexInputState->vertexAttributeDescriptions[%d].format "
                                         "(%s) is not a supported vertex buffer format.",
                                         i, d, string_VkFormat(vertex_attrib_desc.format));
                    }
                }
            }

            // pViewportState, pMultisampleState, pDepthStencilState, and pColorBlendState ignored when rasterization is disabled
            if ((create_info.pRasterizationState != nullptr) &&
                (create_info.pRasterizationState->rasterizerDiscardEnable == VK_FALSE)) {
                if (create_info.pViewportState == nullptr) {
                    skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-rasterizerDiscardEnable-00750",
                                     "vkCreateGraphicsPipelines: Rasterization is enabled (pCreateInfos[%" PRIu32
                                     "].pRasterizationState->rasterizerDiscardEnable is VK_FALSE), but pCreateInfos[%" PRIu32
                                     "].pViewportState (=NULL) is not a valid pointer.",
                                     i, i);
                } else {
                    const auto &viewport_state = *create_info.pViewportState;
                    skip |= ValidatePipelineViewportStateCreateInfo(*create_info.pViewportState, i);

                    auto exclusive_scissor_struct =
                        LvlFindInChain<VkPipelineViewportExclusiveScissorStateCreateInfoNV>(viewport_state.pNext);
                    auto shading_rate_image_struct =
                        LvlFindInChain<VkPipelineViewportShadingRateImageStateCreateInfoNV>(viewport_state.pNext);
                    auto coarse_sample_order_struct =
                        LvlFindInChain<VkPipelineViewportCoarseSampleOrderStateCreateInfoNV>(viewport_state.pNext);
                    const auto vp_swizzle_struct = LvlFindInChain<VkPipelineViewportSwizzleStateCreateInfoNV>(viewport_state.pNext);
                    const auto vp_w_scaling_struct =
                        LvlFindInChain<VkPipelineViewportWScalingStateCreateInfoNV>(viewport_state.pNext);
                    const auto depth_clip_control_struct =
                        LvlFindInChain<VkPipelineViewportDepthClipControlCreateInfoEXT>(viewport_state.pNext);

                    if (!physical_device_features.multiViewport) {
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
                    }

                    // Viewport count
                    if (viewport_state.viewportCount > device_limits.maxViewports) {
                        skip |=
                            LogError(device, "VUID-VkPipelineViewportStateCreateInfo-viewportCount-01218",
                                     "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32 "].pViewportState->viewportCount (=%" PRIu32
                                     ") is greater than VkPhysicalDeviceLimits::maxViewports (=%" PRIu32 ").",
                                     i, viewport_state.viewportCount, device_limits.maxViewports);
                    }
                    if (has_dynamic_viewport_with_count) {
                        if (viewport_state.viewportCount != 0) {
                            skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-03379",
                                             "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32
                                             "].pViewportState->viewportCount (=%" PRIu32
                                             ") must be zero when VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT_EXT is used.",
                                             i, viewport_state.viewportCount);
                        }
                    } else {
                        if (viewport_state.viewportCount == 0) {
                            const char *vuid = IsExtEnabled(device_extensions.vk_ext_extended_dynamic_state)
                                                   ? "VUID-VkPipelineViewportStateCreateInfo-viewportCount-04135"
                                                   : "VUID-VkPipelineViewportStateCreateInfo-viewportCount-arraylength";
                            skip |= LogError(
                                device, vuid,
                                "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32
                                "].pViewportState->viewportCount can't be 0 unless VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT is used.",
                                i);
                        }

                        if (!physical_device_features.multiViewport && (viewport_state.viewportCount > 1)) {
                            skip |= LogError(device, "VUID-VkPipelineViewportStateCreateInfo-viewportCount-01216",
                                             "vkCreateGraphicsPipelines: The VkPhysicalDeviceFeatures::multiViewport feature is "
                                             "disabled, but pCreateInfos[%" PRIu32 "].pViewportState->viewportCount (=%" PRIu32
                                             ") is not 1.",
                                             i, viewport_state.viewportCount);
                        }
                    }

                    // Scissor count
                    if (viewport_state.scissorCount > device_limits.maxViewports) {
                        skip |=
                            LogError(device, "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01219",
                                     "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32 "].pViewportState->scissorCount (=%" PRIu32
                                     ") is greater than VkPhysicalDeviceLimits::maxViewports (=%" PRIu32 ").",
                                     i, viewport_state.scissorCount, device_limits.maxViewports);
                    }
                    if (has_dynamic_scissor_with_count) {
                        if (viewport_state.scissorCount != 0) {
                            skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-03380",
                                             "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32
                                             "].pViewportState->scissorCount (=%" PRIu32
                                             ") must be zero when VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT_EXT is used.",
                                             i, viewport_state.viewportCount);
                        }
                    } else {
                        if (viewport_state.scissorCount == 0) {
                            const char *vuid = IsExtEnabled(device_extensions.vk_ext_extended_dynamic_state)
                                                   ? "VUID-VkPipelineViewportStateCreateInfo-scissorCount-04136"
                                                   : "VUID-VkPipelineViewportStateCreateInfo-scissorCount-arraylength";
                            skip |= LogError(
                                device, vuid,
                                "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32
                                "].pViewportState->scissorCount can't be 0 unless VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT is used.",
                                i);
                        }

                        if (!physical_device_features.multiViewport && (viewport_state.scissorCount > 1)) {
                            skip |= LogError(device, "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01217",
                                             "vkCreateGraphicsPipelines: The VkPhysicalDeviceFeatures::multiViewport feature is "
                                             "disabled, but pCreateInfos[%" PRIu32 "].pViewportState->scissorCount (=%" PRIu32
                                             ") is not 1.",
                                             i, viewport_state.scissorCount);
                        }
                    }

                    if (!has_dynamic_scissor && viewport_state.pScissors) {
                        for (uint32_t scissor_i = 0; scissor_i < viewport_state.scissorCount; ++scissor_i) {
                            const auto &scissor = viewport_state.pScissors[scissor_i];

                            if (scissor.offset.x < 0) {
                                skip |= LogError(device, "VUID-VkPipelineViewportStateCreateInfo-x-02821",
                                                 "vkCreateGraphicsPipelines: offset.x (=%" PRIi32 ") of pCreateInfos[%" PRIu32
                                                 "].pViewportState->pScissors[%" PRIu32 "] is negative.",
                                                 scissor.offset.x, i, scissor_i);
                            }

                            if (scissor.offset.y < 0) {
                                skip |= LogError(device, "VUID-VkPipelineViewportStateCreateInfo-x-02821",
                                                 "vkCreateGraphicsPipelines: offset.y (=%" PRIi32 ") of pCreateInfos[%" PRIu32
                                                 "].pViewportState->pScissors[%" PRIu32 "] is negative.",
                                                 scissor.offset.y, i, scissor_i);
                            }

                            const int64_t x_sum =
                                static_cast<int64_t>(scissor.offset.x) + static_cast<int64_t>(scissor.extent.width);
                            if (x_sum > std::numeric_limits<int32_t>::max()) {
                                skip |= LogError(device, "VUID-VkPipelineViewportStateCreateInfo-offset-02822",
                                                 "vkCreateGraphicsPipelines: offset.x + extent.width (=%" PRIi32 " + %" PRIu32
                                                 " = %" PRIi64 ") of pCreateInfos[%" PRIu32 "].pViewportState->pScissors[%" PRIu32
                                                 "] will overflow int32_t.",
                                                 scissor.offset.x, scissor.extent.width, x_sum, i, scissor_i);
                            }

                            const int64_t y_sum =
                                static_cast<int64_t>(scissor.offset.y) + static_cast<int64_t>(scissor.extent.height);
                            if (y_sum > std::numeric_limits<int32_t>::max()) {
                                skip |= LogError(device, "VUID-VkPipelineViewportStateCreateInfo-offset-02823",
                                                 "vkCreateGraphicsPipelines: offset.y + extent.height (=%" PRIi32 " + %" PRIu32
                                                 " = %" PRIi64 ") of pCreateInfos[%" PRIu32 "].pViewportState->pScissors[%" PRIu32
                                                 "] will overflow int32_t.",
                                                 scissor.offset.y, scissor.extent.height, y_sum, i, scissor_i);
                            }
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

                    if (viewport_state.scissorCount != viewport_state.viewportCount) {
                        if (!IsExtEnabled(device_extensions.vk_ext_extended_dynamic_state) ||
                            (!has_dynamic_viewport_with_count && !has_dynamic_scissor_with_count)) {
                            const char *vuid = IsExtEnabled(device_extensions.vk_ext_extended_dynamic_state)
                                                   ? "VUID-VkPipelineViewportStateCreateInfo-scissorCount-04134"
                                                   : "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01220";
                            skip |= LogError(
                                device, vuid,
                                "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32 "].pViewportState->scissorCount (=%" PRIu32
                                ") is not identical to pCreateInfos[%" PRIu32 "].pViewportState->viewportCount (=%" PRIu32 ").",
                                i, viewport_state.scissorCount, i, viewport_state.viewportCount);
                        }
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

                    if (!vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_VIEWPORT_SHADING_RATE_PALETTE_NV) &&
                        shading_rate_image_struct && shading_rate_image_struct->viewportCount > 0 &&
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

                    if (has_dynamic_viewport_w_scaling_nv && !IsExtEnabled(device_extensions.vk_nv_clip_space_w_scaling)) {
                        skip |= LogError(device, kVUID_PVError_ExtensionNotEnabled,
                                         "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32
                                         "].pDynamicState->pDynamicStates contains VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_NV, but "
                                         "VK_NV_clip_space_w_scaling extension is not enabled.",
                                         i);
                    }

                    if (vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_DISCARD_RECTANGLE_EXT) &&
                        !IsExtEnabled(device_extensions.vk_ext_discard_rectangles)) {
                        skip |= LogError(device, kVUID_PVError_ExtensionNotEnabled,
                                         "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32
                                         "].pDynamicState->pDynamicStates contains VK_DYNAMIC_STATE_DISCARD_RECTANGLE_EXT, but "
                                         "VK_EXT_discard_rectangles extension is not enabled.",
                                         i);
                    }

                    if (vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT) &&
                        !IsExtEnabled(device_extensions.vk_ext_sample_locations)) {
                        skip |= LogError(device, kVUID_PVError_ExtensionNotEnabled,
                                         "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32
                                         "].pDynamicState->pDynamicStates contains VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT, but "
                                         "VK_EXT_sample_locations extension is not enabled.",
                                         i);
                    }

                    if (has_dynamic_exclusive_scissor_nv && !IsExtEnabled(device_extensions.vk_nv_scissor_exclusive)) {
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

                    if (depth_clip_control_struct) {
                        const auto *depth_clip_control_features =
                            LvlFindInChain<VkPhysicalDeviceDepthClipControlFeaturesEXT>(device_createinfo_pnext);
                        const bool enabled_depth_clip_control =
                            depth_clip_control_features && depth_clip_control_features->depthClipControl;
                        if (depth_clip_control_struct->negativeOneToOne && !enabled_depth_clip_control) {
                            skip |= LogError(device, "VUID-VkPipelineViewportDepthClipControlCreateInfoEXT-negativeOneToOne-06470",
                                             "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32
                                             "].pViewportState has negativeOneToOne set to VK_TRUE in the pNext chain, but the "
                                             "depthClipControl feature is not enabled. ",
                                             i);
                        }
                    }
                }

                const bool is_frag_out_graphics_lib =
                    graphics_lib_info &&
                    ((graphics_lib_info->flags & VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT) != 0);
                if (is_frag_out_graphics_lib && (create_info.pMultisampleState == nullptr)) {
                    skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-rasterizerDiscardEnable-00751",
                                     "vkCreateGraphicsPipelines: if pCreateInfos[%" PRIu32
                                     "].pRasterizationState->rasterizerDiscardEnable "
                                     "is VK_FALSE, pCreateInfos[%" PRIu32 "].pMultisampleState must not be NULL.",
                                     i, i);
                } else {
                    // It is possible for pCreateInfos[i].pMultisampleState to be null when creating a graphics library
                    if (create_info.pMultisampleState) {
                        skip |= ValidatePipelineMultisampleStateCreateInfo(*create_info.pMultisampleState, i);

                        if (create_info.pMultisampleState->sampleShadingEnable == VK_TRUE) {
                            if (!physical_device_features.sampleRateShading) {
                                skip |= LogError(device, "VUID-VkPipelineMultisampleStateCreateInfo-sampleShadingEnable-00784",
                                                 "vkCreateGraphicsPipelines(): parameter "
                                                 "pCreateInfos[%" PRIu32 "].pMultisampleState->sampleShadingEnable.",
                                                 i);
                            }
                            // TODO Add documentation issue about when minSampleShading must be in range and when it is ignored
                            // For now a "least noise" test *only* when sampleShadingEnable is VK_TRUE.
                            if (!in_inclusive_range(create_info.pMultisampleState->minSampleShading, 0.F, 1.0F)) {
                                skip |= LogError(device,

                                                 "VUID-VkPipelineMultisampleStateCreateInfo-minSampleShading-00786",
                                                 "vkCreateGraphicsPipelines(): parameter pCreateInfos[%" PRIu32
                                                 "].pMultisampleState->minSampleShading.",
                                                 i);
                            }
                        }
                    }

                    const auto *line_state =
                        LvlFindInChain<VkPipelineRasterizationLineStateCreateInfoEXT>(create_info.pRasterizationState->pNext);
                    const bool dynamic_line_raster_mode =
                        vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_LINE_RASTERIZATION_MODE_EXT);
                    const bool dynamic_line_stipple_enable =
                        vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_LINE_STIPPLE_ENABLE_EXT);
                    if (line_state) {
                        if (line_state->stippledLineEnable && !dynamic_line_stipple_enable) {
                            if (line_state->lineStippleFactor < 1 || line_state->lineStippleFactor > 256) {
                                skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-stippledLineEnable-02767",
                                                 "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                                 "] lineStippleFactor = %" PRIu32 " must be in the range [1,256].",
                                                 i, line_state->lineStippleFactor);
                            }
                        }

                        if (!dynamic_line_raster_mode) {
                            if ((line_state->lineRasterizationMode == VK_LINE_RASTERIZATION_MODE_BRESENHAM_EXT ||
                                 line_state->lineRasterizationMode == VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH_EXT)) {
                                if (create_info.pMultisampleState && create_info.pMultisampleState->alphaToCoverageEnable) {
                                    skip |= LogError(
                                        device, "VUID-VkGraphicsPipelineCreateInfo-lineRasterizationMode-02766",
                                        "vkCreateGraphicsPipelines(): Bresenham/Smooth line rasterization not supported with "
                                        "pCreateInfos[%" PRIu32 "].pMultisampleState->alphaToCoverageEnable == VK_TRUE.",
                                        i);
                                }
                                if (create_info.pMultisampleState && create_info.pMultisampleState->alphaToOneEnable) {
                                    skip |= LogError(
                                        device, "VUID-VkGraphicsPipelineCreateInfo-lineRasterizationMode-02766",
                                        "vkCreateGraphicsPipelines(): Bresenham/Smooth line rasterization not supported with "
                                        "pCreateInfos[%" PRIu32 "].pMultisampleState->alphaToOneEnable == VK_TRUE.",
                                        i);
                                }
                                if (create_info.pMultisampleState && create_info.pMultisampleState->sampleShadingEnable) {
                                    skip |= LogError(
                                        device, "VUID-VkGraphicsPipelineCreateInfo-lineRasterizationMode-02766",
                                        "vkCreateGraphicsPipelines(): Bresenham/Smooth line rasterization not supported with "
                                        "pCreateInfos[%" PRIu32 "].pMultisampleState->sampleShadingEnable == VK_TRUE.",
                                        i);
                                }
                            }

                            const auto *line_features =
                                LvlFindInChain<VkPhysicalDeviceLineRasterizationFeaturesEXT>(device_createinfo_pnext);
                            if (line_state->lineRasterizationMode == VK_LINE_RASTERIZATION_MODE_RECTANGULAR_EXT &&
                                (!line_features || !line_features->rectangularLines)) {
                                skip |= LogError(
                                    device, "VUID-VkPipelineRasterizationLineStateCreateInfoEXT-lineRasterizationMode-02768",
                                    "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                    "] lineRasterizationMode = "
                                    "VK_LINE_RASTERIZATION_MODE_RECTANGULAR_EXT but the rectangularLines feature is not enabled.",
                                    i);
                            }
                            if (line_state->lineRasterizationMode == VK_LINE_RASTERIZATION_MODE_BRESENHAM_EXT &&
                                (!line_features || !line_features->bresenhamLines)) {
                                skip |= LogError(
                                    device, "VUID-VkPipelineRasterizationLineStateCreateInfoEXT-lineRasterizationMode-02769",
                                    "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                    "] lineRasterizationMode = "
                                    "VK_LINE_RASTERIZATION_MODE_BRESENHAM_EXT but the bresenhamLines feature is not enabled.",
                                    i);
                            }
                            if (line_state->lineRasterizationMode == VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH_EXT &&
                                (!line_features || !line_features->smoothLines)) {
                                skip |= LogError(
                                    device, "VUID-VkPipelineRasterizationLineStateCreateInfoEXT-lineRasterizationMode-02770",
                                    "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                    "] lineRasterizationMode = "
                                    "VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH_EXT but the smoothLines feature is not enabled.",
                                    i);
                            }
                            if (line_state->stippledLineEnable && !dynamic_line_stipple_enable) {
                                if (line_state->lineRasterizationMode == VK_LINE_RASTERIZATION_MODE_RECTANGULAR_EXT &&
                                    (!line_features || !line_features->stippledRectangularLines)) {
                                    skip |= LogError(device,
                                                     "VUID-VkPipelineRasterizationLineStateCreateInfoEXT-stippledLineEnable-02771",
                                                     "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                                     "] lineRasterizationMode = "
                                                     "VK_LINE_RASTERIZATION_MODE_RECTANGULAR_EXT and stippledLineEnable is "
                                                     "VK_TRUE, but the stippledRectangularLines feature is not enabled.",
                                                     i);
                                }
                                if (line_state->lineRasterizationMode == VK_LINE_RASTERIZATION_MODE_BRESENHAM_EXT &&
                                    (!line_features || !line_features->stippledBresenhamLines)) {
                                    skip |= LogError(device,
                                                     "VUID-VkPipelineRasterizationLineStateCreateInfoEXT-stippledLineEnable-02772",
                                                     "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                                     "] lineRasterizationMode = "
                                                     "VK_LINE_RASTERIZATION_MODE_BRESENHAM_EXT and stippledLineEnable is VK_TRUE, "
                                                     "but the stippledBresenhamLines feature is not enabled.",
                                                     i);
                                }
                                if (line_state->lineRasterizationMode == VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH_EXT &&
                                    (!line_features || !line_features->stippledSmoothLines)) {
                                    skip |= LogError(device,
                                                     "VUID-VkPipelineRasterizationLineStateCreateInfoEXT-stippledLineEnable-02773",
                                                     "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                                     "] lineRasterizationMode = "
                                                     "VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH_EXT and stippledLineEnable is "
                                                     "VK_TRUE, but the stippledSmoothLines feature is not enabled.",
                                                     i);
                                }
                                if (line_state->lineRasterizationMode == VK_LINE_RASTERIZATION_MODE_DEFAULT_EXT &&
                                    (!line_features || !line_features->stippledRectangularLines || !device_limits.strictLines)) {
                                    skip |= LogError(
                                        device, "VUID-VkPipelineRasterizationLineStateCreateInfoEXT-stippledLineEnable-02774",
                                        "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                        "] lineRasterizationMode = "
                                        "VK_LINE_RASTERIZATION_MODE_DEFAULT_EXT and stippledLineEnable is VK_TRUE, "
                                        "but the stippledRectangularLines feature is not enabled or strictLines is VK_FALSE.",
                                        i);
                                }
                            }
                        }
                    }
                }

                bool uses_color_attachment = false;
                bool uses_depthstencil_attachment = false;
                VkSubpassDescriptionFlags subpass_flags = 0;
                {
                    std::unique_lock<std::mutex> lock(renderpass_map_mutex);
                    const auto subpasses_uses_it = renderpasses_states.find(create_info.renderPass);
                    if (subpasses_uses_it != renderpasses_states.end()) {
                        const auto &subpasses_uses = subpasses_uses_it->second;
                        if (subpasses_uses.subpasses_using_color_attachment.count(create_info.subpass)) {
                            uses_color_attachment = true;
                        }
                        if (subpasses_uses.subpasses_using_depthstencil_attachment.count(create_info.subpass)) {
                            uses_depthstencil_attachment = true;
                        }
                        subpass_flags = subpasses_uses.subpasses_flags[create_info.subpass];

                        color_attachment_count = subpasses_uses.color_attachment_count;
                    }
                    lock.unlock();
                }

                if (create_info.pDepthStencilState != nullptr && uses_depthstencil_attachment) {
                    skip |= ValidatePipelineDepthStencilStateCreateInfo(*create_info.pDepthStencilState, i);

                    if ((create_info.pDepthStencilState->flags &
                         VK_PIPELINE_DEPTH_STENCIL_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_DEPTH_ACCESS_BIT_ARM) != 0) {
                        const auto *rasterization_order_attachment_access_feature =
                            LvlFindInChain<VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesARM>(device_createinfo_pnext);
                        const bool rasterization_order_depth_attachment_access_feature_enabled =
                            rasterization_order_attachment_access_feature &&
                            rasterization_order_attachment_access_feature->rasterizationOrderDepthAttachmentAccess == VK_TRUE;
                        if (!rasterization_order_depth_attachment_access_feature_enabled) {
                            skip |= LogError(
                                device, "VUID-VkPipelineDepthStencilStateCreateInfo-rasterizationOrderDepthAttachmentAccess-06463",
                                "VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesARM::"
                                "rasterizationOrderDepthAttachmentAccess == VK_FALSE, but "
                                "VkPipelineDepthStencilStateCreateInfo::flags == %s",
                                string_VkPipelineDepthStencilStateCreateFlags(create_info.pDepthStencilState->flags).c_str());
                        }

                        if ((subpass_flags & VK_SUBPASS_DESCRIPTION_RASTERIZATION_ORDER_ATTACHMENT_DEPTH_ACCESS_BIT_ARM) == 0) {
                            skip |= LogError(
                                device, "VUID-VkGraphicsPipelineCreateInfo-flags-06485",
                                "VkPipelineDepthStencilStateCreateInfo::flags == %s but "
                                "VkRenderPassCreateInfo::VkSubpassDescription::flags == %s",
                                string_VkPipelineDepthStencilStateCreateFlags(create_info.pDepthStencilState->flags).c_str(),
                                string_VkSubpassDescriptionFlags(subpass_flags).c_str());
                        }
                    }

                    if ((create_info.pDepthStencilState->flags &
                         VK_PIPELINE_DEPTH_STENCIL_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_STENCIL_ACCESS_BIT_ARM) != 0) {
                        const auto *rasterization_order_attachment_access_feature =
                            LvlFindInChain<VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesARM>(device_createinfo_pnext);
                        const bool rasterization_order_stencil_attachment_access_feature_enabled =
                            rasterization_order_attachment_access_feature &&
                            rasterization_order_attachment_access_feature->rasterizationOrderStencilAttachmentAccess == VK_TRUE;
                        if (!rasterization_order_stencil_attachment_access_feature_enabled) {
                            skip |= LogError(
                                device,
                                "VUID-VkPipelineDepthStencilStateCreateInfo-rasterizationOrderStencilAttachmentAccess-06464",
                                "VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesARM::"
                                "rasterizationOrderStencilAttachmentAccess == VK_FALSE, but "
                                "VkPipelineDepthStencilStateCreateInfo::flags == %s",
                                string_VkPipelineDepthStencilStateCreateFlags(create_info.pDepthStencilState->flags).c_str());
                        }

                        if ((subpass_flags & VK_SUBPASS_DESCRIPTION_RASTERIZATION_ORDER_ATTACHMENT_STENCIL_ACCESS_BIT_ARM) == 0) {
                            skip |= LogError(
                                device, "VUID-VkGraphicsPipelineCreateInfo-flags-06486",
                                "VkPipelineDepthStencilStateCreateInfo::flags == %s but "
                                "VkRenderPassCreateInfo::VkSubpassDescription::flags == %s",
                                string_VkPipelineDepthStencilStateCreateFlags(create_info.pDepthStencilState->flags).c_str(),
                                string_VkSubpassDescriptionFlags(subpass_flags).c_str());
                        }
                    }
                }

                if (create_info.pColorBlendState != nullptr && uses_color_attachment) {
                    auto const &color_blend_state = *create_info.pColorBlendState;
                    skip |= ValidatePipelineColorBlendStateCreateInfo(color_blend_state, i);

                    if ((color_blend_state.flags &
                         VK_PIPELINE_COLOR_BLEND_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_BIT_ARM) != 0) {
                        const auto *rasterization_order_attachment_access_feature =
                            LvlFindInChain<VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesARM>(device_createinfo_pnext);
                        const bool rasterization_order_color_attachment_access_feature_enabled =
                            rasterization_order_attachment_access_feature &&
                            rasterization_order_attachment_access_feature->rasterizationOrderColorAttachmentAccess == VK_TRUE;

                        if (!rasterization_order_color_attachment_access_feature_enabled) {
                            skip |= LogError(
                                device, "VUID-VkPipelineColorBlendStateCreateInfo-rasterizationOrderColorAttachmentAccess-06465",
                                "VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesARM::"
                                "rasterizationColorAttachmentAccess == VK_FALSE, but "
                                "VkPipelineColorBlendStateCreateInfo::flags == %s",
                                string_VkPipelineColorBlendStateCreateFlags(color_blend_state.flags).c_str());
                        }

                        if ((subpass_flags & VK_SUBPASS_DESCRIPTION_RASTERIZATION_ORDER_ATTACHMENT_COLOR_ACCESS_BIT_ARM) == 0) {
                            skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-flags-06484",
                                             "VkPipelineColorBlendStateCreateInfo::flags == %s but "
                                             "VkRenderPassCreateInfo::VkSubpassDescription::flags == %s",
                                             string_VkPipelineColorBlendStateCreateFlags(color_blend_state.flags).c_str(),
                                             string_VkSubpassDescriptionFlags(subpass_flags).c_str());
                        }
                    }

                    if (color_blend_state.pAttachments != nullptr) {
                        const VkBlendOp first_color_blend_op = color_blend_state.pAttachments[0].colorBlendOp;
                        const VkBlendOp first_alpha_blend_op = color_blend_state.pAttachments[0].alphaBlendOp;
                        for (uint32_t attachment_index = 0; attachment_index < color_blend_state.attachmentCount;
                             ++attachment_index) {
                            const VkPipelineColorBlendAttachmentState attachment_state =
                                color_blend_state.pAttachments[attachment_index];

                            skip |= ValidatePipelineColorBlendAttachmentState(attachment_state, i, attachment_index);

                            // if blendEnabled is false, these values are ignored
                            if (attachment_state.blendEnable) {
                                bool advance_blend = false;
                                if (IsAdvanceBlendOperation(attachment_state.colorBlendOp)) {
                                    advance_blend = true;
                                    if (phys_dev_ext_props.blend_operation_advanced_props.advancedBlendAllOperations == VK_FALSE) {
                                        // This VUID checks if a subset of advance blend ops are allowed
                                        switch (attachment_state.colorBlendOp) {
                                            case VK_BLEND_OP_ZERO_EXT:
                                            case VK_BLEND_OP_SRC_EXT:
                                            case VK_BLEND_OP_DST_EXT:
                                            case VK_BLEND_OP_SRC_OVER_EXT:
                                            case VK_BLEND_OP_DST_OVER_EXT:
                                            case VK_BLEND_OP_SRC_IN_EXT:
                                            case VK_BLEND_OP_DST_IN_EXT:
                                            case VK_BLEND_OP_SRC_OUT_EXT:
                                            case VK_BLEND_OP_DST_OUT_EXT:
                                            case VK_BLEND_OP_SRC_ATOP_EXT:
                                            case VK_BLEND_OP_DST_ATOP_EXT:
                                            case VK_BLEND_OP_XOR_EXT:
                                            case VK_BLEND_OP_INVERT_EXT:
                                            case VK_BLEND_OP_INVERT_RGB_EXT:
                                            case VK_BLEND_OP_LINEARDODGE_EXT:
                                            case VK_BLEND_OP_LINEARBURN_EXT:
                                            case VK_BLEND_OP_VIVIDLIGHT_EXT:
                                            case VK_BLEND_OP_LINEARLIGHT_EXT:
                                            case VK_BLEND_OP_PINLIGHT_EXT:
                                            case VK_BLEND_OP_HARDMIX_EXT:
                                            case VK_BLEND_OP_PLUS_EXT:
                                            case VK_BLEND_OP_PLUS_CLAMPED_EXT:
                                            case VK_BLEND_OP_PLUS_CLAMPED_ALPHA_EXT:
                                            case VK_BLEND_OP_PLUS_DARKER_EXT:
                                            case VK_BLEND_OP_MINUS_EXT:
                                            case VK_BLEND_OP_MINUS_CLAMPED_EXT:
                                            case VK_BLEND_OP_CONTRAST_EXT:
                                            case VK_BLEND_OP_INVERT_OVG_EXT:
                                            case VK_BLEND_OP_RED_EXT:
                                            case VK_BLEND_OP_GREEN_EXT:
                                            case VK_BLEND_OP_BLUE_EXT: {
                                                skip |= LogError(
                                                    device,
                                                    "VUID-VkPipelineColorBlendAttachmentState-advancedBlendAllOperations-01409",
                                                    "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32
                                                    "].pColorBlendState->pAttachments[%" PRIu32
                                                    "].colorBlendOp (%s) is not valid when "
                                                    "VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT::"
                                                    "advancedBlendAllOperations is "
                                                    "VK_FALSE",
                                                    i, attachment_index, string_VkBlendOp(attachment_state.colorBlendOp));
                                                break;
                                            }
                                            default:
                                                break;
                                        }
                                    }

                                    if (phys_dev_ext_props.blend_operation_advanced_props.advancedBlendIndependentBlend ==
                                            VK_FALSE &&
                                        attachment_state.colorBlendOp != first_color_blend_op) {
                                        skip |= LogError(
                                            device, "VUID-VkPipelineColorBlendAttachmentState-advancedBlendIndependentBlend-01407",
                                            "vkCreateGraphicsPipelines: advancedBlendIndependentBlend is set to VK_FALSE, but "
                                            "pCreateInfos[%" PRIu32 "].pColorBlendState->pAttachments[%" PRIu32
                                            "].colorBlendOp (%s) is not same the other attachments (%s).",
                                            i, attachment_index, string_VkBlendOp(attachment_state.colorBlendOp),
                                            string_VkBlendOp(first_color_blend_op));
                                    }
                                }

                                if (IsAdvanceBlendOperation(attachment_state.alphaBlendOp)) {
                                    advance_blend = true;
                                    if (phys_dev_ext_props.blend_operation_advanced_props.advancedBlendIndependentBlend ==
                                            VK_FALSE &&
                                        attachment_state.alphaBlendOp != first_alpha_blend_op) {
                                        skip |= LogError(
                                            device, "VUID-VkPipelineColorBlendAttachmentState-advancedBlendIndependentBlend-01408",
                                            "vkCreateGraphicsPipelines: advancedBlendIndependentBlend is set to VK_FALSE, but "
                                            "pCreateInfos[%" PRIu32 "].pColorBlendState->pAttachments[%" PRIu32
                                            "].alphaBlendOp (%s) is not same the other attachments (%s).",
                                            i, attachment_index, string_VkBlendOp(attachment_state.alphaBlendOp),
                                            string_VkBlendOp(first_alpha_blend_op));
                                    }
                                }

                                if (advance_blend) {
                                    if (attachment_state.colorBlendOp != attachment_state.alphaBlendOp) {
                                        skip |= LogError(device, "VUID-VkPipelineColorBlendAttachmentState-colorBlendOp-01406",
                                                         "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32
                                                         "].pColorBlendState->pAttachments[%" PRIu32
                                                         "] has different colorBlendOp (%s) and alphaBlendOp (%s) but one of "
                                                         "them is an advance blend operation.",
                                                         i, attachment_index, string_VkBlendOp(attachment_state.colorBlendOp),
                                                         string_VkBlendOp(attachment_state.alphaBlendOp));
                                    } else if (color_attachment_count >
                                               phys_dev_ext_props.blend_operation_advanced_props.advancedBlendMaxColorAttachments) {
                                        // color_attachment_count is found one of multiple spots above
                                        //
                                        // error can guarantee it is the same VkBlendOp
                                        skip |= LogError(
                                            device, "VUID-VkPipelineColorBlendAttachmentState-colorBlendOp-01410",
                                            "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32
                                            "].pColorBlendState->pAttachments[%" PRIu32
                                            "] has an advance blend operation (%s) but the colorAttachmentCount (%" PRIu32
                                            ") for the subpass is greater than advancedBlendMaxColorAttachments (%" PRIu32 ").",
                                            i, attachment_index, string_VkBlendOp(attachment_state.colorBlendOp),
                                            color_attachment_count,
                                            phys_dev_ext_props.blend_operation_advanced_props.advancedBlendMaxColorAttachments);
                                    }
                                }
                            }
                        }
                    }

                    // If logicOpEnable is VK_TRUE, logicOp must be a valid VkLogicOp value
                    if (color_blend_state.logicOpEnable == VK_TRUE) {
                        skip |= ValidateRangedEnum(
                            "vkCreateGraphicsPipelines",
                            ParameterName("pCreateInfos[%i].pColorBlendState->logicOp", ParameterName::IndexVector{i}), "VkLogicOp",
                            color_blend_state.logicOp, "VUID-VkPipelineColorBlendStateCreateInfo-logicOpEnable-00607");
                    }

                    const bool dynamic_not_set =
                        (!vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT) ||
                         !vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT) ||
                         !vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT));

                    // If any of the dynamic states are not set still need a valid array
                    if ((color_blend_state.attachmentCount > 0) && dynamic_not_set) {
                        const char *vuid = IsExtEnabled(device_extensions.vk_ext_extended_dynamic_state3)
                                               ? "VUID-VkPipelineColorBlendStateCreateInfo-pAttachments-07353"
                                               : "VUID-VkPipelineColorBlendStateCreateInfo-pAttachments-07354";

                        skip |= ValidateArray(
                            "vkCreateGraphicsPipelines",
                            ParameterName("pCreateInfos[%i].pColorBlendState->attachmentCount", ParameterName::IndexVector{i}),
                            ParameterName("pCreateInfos[%i].pColorBlendState->pAttachments", ParameterName::IndexVector{i}),
                            color_blend_state.attachmentCount, &color_blend_state.pAttachments, false, true, kVUIDUndefined, vuid);
                    }

                    auto color_write = LvlFindInChain<VkPipelineColorWriteCreateInfoEXT>(color_blend_state.pNext);
                    if (color_write && (color_write->attachmentCount != color_blend_state.attachmentCount) && dynamic_not_set) {
                        const char *vuid = IsExtEnabled(device_extensions.vk_ext_extended_dynamic_state3)
                                               ? "VUID-VkPipelineColorWriteCreateInfoEXT-attachmentCount-07608"
                                               : "VUID-VkPipelineColorWriteCreateInfoEXT-attachmentCount-04802";
                        skip |= LogError(device, vuid,
                                         "vkCreateGraphicsPipelines(): VkPipelineColorWriteCreateInfoEXT in the pNext chain of "
                                         "pCreateInfo[%" PRIu32 "].pColorBlendState has different attachmentCount (%" PRIu32
                                         ") than pColorBlendState.attachmentCount (%" PRIu32 ").",
                                         i, color_write->attachmentCount, color_blend_state.attachmentCount);
                    }
                }
            }

            if (flags & VK_PIPELINE_CREATE_DERIVATIVE_BIT) {
                if (create_info.basePipelineIndex != -1) {
                    if (create_info.basePipelineHandle != VK_NULL_HANDLE) {
                        skip |=
                            LogError(device, "VUID-VkGraphicsPipelineCreateInfo-flags-00724",
                                     "vkCreateGraphicsPipelines parameter, pCreateInfos[%" PRIu32
                                     "]->basePipelineHandle, must be "
                                     "VK_NULL_HANDLE if pCreateInfos->flags contains the VK_PIPELINE_CREATE_DERIVATIVE_BIT flag "
                                     "and pCreateInfos->basePipelineIndex is not -1.",
                                     i);
                    }
                }

                if (create_info.basePipelineHandle != VK_NULL_HANDLE) {
                    if (create_info.basePipelineIndex != -1) {
                        skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-flags-00725",
                                         "vkCreateGraphicsPipelines parameter, pCreateInfos[%" PRIu32
                                         "]->basePipelineIndex, must be -1 if "
                                         "pCreateInfos->flags contains the VK_PIPELINE_CREATE_DERIVATIVE_BIT flag and "
                                         "pCreateInfos->basePipelineHandle is not VK_NULL_HANDLE.",
                                         i);
                    }
                } else {
                    if (static_cast<uint32_t>(create_info.basePipelineIndex) >= createInfoCount) {
                        skip |=
                            LogError(device, "VUID-VkGraphicsPipelineCreateInfo-flags-00723",
                                     "vkCreateGraphicsPipelines parameter pCreateInfos[%" PRIu32 "]->basePipelineIndex (%" PRId32
                                     ") must be a valid"
                                     "index into the pCreateInfos array, of size %" PRIu32 ".",
                                     i, create_info.basePipelineIndex, createInfoCount);
                    }
                }
            }

            if (create_info.pRasterizationState) {
                if (!IsExtEnabled(device_extensions.vk_nv_fill_rectangle)) {
                    if (create_info.pRasterizationState->polygonMode == VK_POLYGON_MODE_FILL_RECTANGLE_NV) {
                        skip |=
                            LogError(device, "VUID-VkPipelineRasterizationStateCreateInfo-polygonMode-01414",
                                     "vkCreateGraphicsPipelines parameter, VkPolygonMode "
                                     "pCreateInfos->pRasterizationState->polygonMode cannot be VK_POLYGON_MODE_FILL_RECTANGLE_NV "
                                     "if the extension VK_NV_fill_rectangle is not enabled.");
                    } else if ((create_info.pRasterizationState->polygonMode != VK_POLYGON_MODE_FILL) &&
                               (physical_device_features.fillModeNonSolid == false)) {
                        skip |= LogError(device, "VUID-VkPipelineRasterizationStateCreateInfo-polygonMode-01413",
                                         "vkCreateGraphicsPipelines parameter, VkPolygonMode "
                                         "pCreateInfos[%" PRIu32
                                         "]->pRasterizationState->polygonMode cannot be VK_POLYGON_MODE_POINT or "
                                         "VK_POLYGON_MODE_LINE if VkPhysicalDeviceFeatures->fillModeNonSolid is false.",
                                         i);
                    }
                } else {
                    if ((create_info.pRasterizationState->polygonMode != VK_POLYGON_MODE_FILL) &&
                        (create_info.pRasterizationState->polygonMode != VK_POLYGON_MODE_FILL_RECTANGLE_NV) &&
                        (physical_device_features.fillModeNonSolid == false)) {
                        skip |=
                            LogError(device, "VUID-VkPipelineRasterizationStateCreateInfo-polygonMode-01507",
                                     "vkCreateGraphicsPipelines parameter, VkPolygonMode "
                                     "pCreateInfos[%" PRIu32
                                     "]->pRasterizationState->polygonMode must be VK_POLYGON_MODE_FILL or "
                                     "VK_POLYGON_MODE_FILL_RECTANGLE_NV if VkPhysicalDeviceFeatures->fillModeNonSolid is false.",
                                     i);
                    }
                }

                if (!vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_LINE_WIDTH) && !physical_device_features.wideLines &&
                    (create_info.pRasterizationState->lineWidth != 1.0f)) {
                    skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-00749",
                                     "The line width state is static (pCreateInfos[%" PRIu32
                                     "].pDynamicState->pDynamicStates does not contain VK_DYNAMIC_STATE_LINE_WIDTH) and "
                                     "VkPhysicalDeviceFeatures::wideLines is disabled, but pCreateInfos[%" PRIu32
                                     "].pRasterizationState->lineWidth (=%f) is not 1.0.",
                                     i, i, create_info.pRasterizationState->lineWidth);
                }
            }

            // Validate no flags not allowed are used
            if ((flags & VK_PIPELINE_CREATE_DISPATCH_BASE) != 0) {
                skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-flags-00764",
                                 "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                 "]->flags (0x%x) must not include "
                                 "VK_PIPELINE_CREATE_DISPATCH_BASE.",
                                 i, flags);
            }
            if ((flags & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_ANY_HIT_SHADERS_BIT_KHR) != 0) {
                skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-flags-03372",
                                 "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                 "]->flags (0x%x) must not include "
                                 "VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_ANY_HIT_SHADERS_BIT_KHR.",
                                 i, flags);
            }
            if ((flags & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_CLOSEST_HIT_SHADERS_BIT_KHR) != 0) {
                skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-flags-03373",
                                 "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                 "]->flags (0x%x) must not include "
                                 "VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_CLOSEST_HIT_SHADERS_BIT_KHR.",
                                 i, flags);
            }
            if ((flags & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_MISS_SHADERS_BIT_KHR) != 0) {
                skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-flags-03374",
                                 "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                 "]->flags (0x%x) must not include "
                                 "VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_MISS_SHADERS_BIT_KHR.",
                                 i, flags);
            }
            if ((flags & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_INTERSECTION_SHADERS_BIT_KHR) != 0) {
                skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-flags-03375",
                                 "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                 "]->flags (0x%x) must not include "
                                 "VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_INTERSECTION_SHADERS_BIT_KHR.",
                                 i, flags);
            }
            if ((flags & VK_PIPELINE_CREATE_RAY_TRACING_SKIP_TRIANGLES_BIT_KHR) != 0) {
                skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-flags-03376",
                                 "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                 "]->flags (0x%x) must not include "
                                 "VK_PIPELINE_CREATE_RAY_TRACING_SKIP_TRIANGLES_BIT_KHR.",
                                 i, flags);
            }
            if ((flags & VK_PIPELINE_CREATE_RAY_TRACING_SKIP_AABBS_BIT_KHR) != 0) {
                skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-flags-03377",
                                 "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                 "]->flags (0x%x) must not include "
                                 "VK_PIPELINE_CREATE_RAY_TRACING_SKIP_AABBS_BIT_KHR.",
                                 i, flags);
            }
            if ((flags & VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR) != 0) {
                skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-flags-03577",
                                 "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                 "]->flags (0x%x) must not include "
                                 "VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR.",
                                 i, flags);
            }
            if ((flags & VK_PIPELINE_CREATE_RAY_TRACING_ALLOW_MOTION_BIT_NV) != 0) {
                skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-flags-04947",
                                 "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                 "]->flags (0x%x) must not include "
                                 "VK_PIPELINE_CREATE_RAY_TRACING_ALLOW_MOTION_BIT_NV.",
                                 i, flags);
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
        skip |=
            ValidateString("vkCreateComputePipelines", ParameterName("pCreateInfos[%i].stage.pName", ParameterName::IndexVector{i}),
                           "VUID-VkPipelineShaderStageCreateInfo-pName-parameter", pCreateInfos[i].stage.pName);
        auto feedback_struct = LvlFindInChain<VkPipelineCreationFeedbackCreateInfoEXT>(pCreateInfos[i].pNext);
        if (feedback_struct) {
            const uint32_t feedback_count = feedback_struct->pipelineStageCreationFeedbackCount;
            if ((feedback_count != 0) && (feedback_count != 1)) {
                skip |= LogError(
                    device, "VUID-VkComputePipelineCreateInfo-pipelineStageCreationFeedbackCount-06566",
                    "vkCreateComputePipelines(): VkPipelineCreationFeedbackCreateInfo::pipelineStageCreationFeedbackCount (%" PRIu32
                    ") is not 0 or 1 in pCreateInfos[%" PRIu32 "].",
                    feedback_count, i);
            }
        }

        // Make sure compute stage is selected
        if (pCreateInfos[i].stage.stage != VK_SHADER_STAGE_COMPUTE_BIT) {
            skip |= LogError(device, "VUID-VkComputePipelineCreateInfo-stage-00701",
                             "vkCreateComputePipelines(): the pCreateInfo[%" PRIu32
                             "].stage.stage (%s) is not VK_SHADER_STAGE_COMPUTE_BIT",
                             i, string_VkShaderStageFlagBits(pCreateInfos[i].stage.stage));
        }

        const VkPipelineCreateFlags flags = pCreateInfos[i].flags;
        // Validate no flags not allowed are used
        if ((flags & VK_PIPELINE_CREATE_LIBRARY_BIT_KHR) != 0) {
            skip |= LogError(device, "VUID-VkComputePipelineCreateInfo-flags-03364",
                             "vkCreateComputePipelines(): pCreateInfos[%" PRIu32
                             "]->flags (0x%x) must not include VK_PIPELINE_CREATE_LIBRARY_BIT_KHR.",
                             i, flags);
        }
        if ((flags & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_ANY_HIT_SHADERS_BIT_KHR) != 0) {
            skip |= LogError(device, "VUID-VkComputePipelineCreateInfo-flags-03365",
                             "vkCreateComputePipelines(): pCreateInfos[%" PRIu32
                             "]->flags (0x%x) must not include "
                             "VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_ANY_HIT_SHADERS_BIT_KHR.",
                             i, flags);
        }
        if ((flags & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_CLOSEST_HIT_SHADERS_BIT_KHR) != 0) {
            skip |= LogError(device, "VUID-VkComputePipelineCreateInfo-flags-03366",
                             "vkCreateComputePipelines(): pCreateInfos[%" PRIu32
                             "]->flags (0x%x) must not include "
                             "VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_CLOSEST_HIT_SHADERS_BIT_KHR.",
                             i, flags);
        }
        if ((flags & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_MISS_SHADERS_BIT_KHR) != 0) {
            skip |= LogError(device, "VUID-VkComputePipelineCreateInfo-flags-03367",
                             "vkCreateComputePipelines(): pCreateInfos[%" PRIu32
                             "]->flags (0x%x) must not include "
                             "VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_MISS_SHADERS_BIT_KHR.",
                             i, flags);
        }
        if ((flags & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_INTERSECTION_SHADERS_BIT_KHR) != 0) {
            skip |= LogError(device, "VUID-VkComputePipelineCreateInfo-flags-03368",
                             "vkCreateComputePipelines(): pCreateInfos[%" PRIu32
                             "]->flags (0x%x) must not include "
                             "VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_INTERSECTION_SHADERS_BIT_KHR.",
                             i, flags);
        }
        if ((flags & VK_PIPELINE_CREATE_RAY_TRACING_SKIP_TRIANGLES_BIT_KHR) != 0) {
            skip |= LogError(device, "VUID-VkComputePipelineCreateInfo-flags-03369",
                             "vkCreateComputePipelines(): pCreateInfos[%" PRIu32
                             "]->flags (0x%x) must not include "
                             "VK_PIPELINE_CREATE_RAY_TRACING_SKIP_TRIANGLES_BIT_KHR.",
                             i, flags);
        }
        if ((flags & VK_PIPELINE_CREATE_RAY_TRACING_SKIP_AABBS_BIT_KHR) != 0) {
            skip |= LogError(device, "VUID-VkComputePipelineCreateInfo-flags-03370",
                             "vkCreateComputePipelines(): pCreateInfos[%" PRIu32
                             "]->flags (0x%x) must not include "
                             "VK_PIPELINE_CREATE_RAY_TRACING_SKIP_AABBS_BIT_KHR.",
                             i, flags);
        }
        if ((flags & VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR) != 0) {
            skip |= LogError(device, "VUID-VkComputePipelineCreateInfo-flags-03576",
                             "vkCreateComputePipelines(): pCreateInfos[%" PRIu32
                             "]->flags (0x%x) must not include "
                             "VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR.",
                             i, flags);
        }
        if ((flags & VK_PIPELINE_CREATE_RAY_TRACING_ALLOW_MOTION_BIT_NV) != 0) {
            skip |= LogError(device, "VUID-VkComputePipelineCreateInfo-flags-04945",
                             "vkCreateComputePipelines(): pCreateInfos[%" PRIu32
                             "]->flags (0x%x) must not include "
                             "VK_PIPELINE_CREATE_RAY_TRACING_ALLOW_MOTION_BIT_NV.",
                             i, flags);
        }
        if ((flags & VK_PIPELINE_CREATE_INDIRECT_BINDABLE_BIT_NV) != 0) {
            skip |= LogError(device, "VUID-VkComputePipelineCreateInfo-flags-02874",
                             "vkCreateComputePipelines(): pCreateInfos[%" PRIu32
                             "]->flags (0x%x) must not include "
                             "VK_PIPELINE_CREATE_INDIRECT_BINDABLE_BIT_NV.",
                             i, flags);
        }
        if (flags & VK_PIPELINE_CREATE_DERIVATIVE_BIT) {
            if (pCreateInfos[i].basePipelineIndex != -1) {
                if (pCreateInfos[i].basePipelineHandle != VK_NULL_HANDLE) {
                    skip |= LogError(device, "VUID-VkComputePipelineCreateInfo-flags-00699",
                                     "vkCreateComputePipelines parameter, pCreateInfos[%" PRIu32
                                     "]->basePipelineHandle, must be VK_NULL_HANDLE if pCreateInfos->flags contains the "
                                     "VK_PIPELINE_CREATE_DERIVATIVE_BIT flag and pCreateInfos->basePipelineIndex is not -1.",
                                     i);
                }
            }

            if (pCreateInfos[i].basePipelineHandle != VK_NULL_HANDLE) {
                if (pCreateInfos[i].basePipelineIndex != -1) {
                    skip |= LogError(
                        device, "VUID-VkComputePipelineCreateInfo-flags-00700",
                        "vkCreateComputePipelines parameter, pCreateInfos[%" PRIu32
                        "]->basePipelineIndex, must be -1 if pCreateInfos->flags contains the VK_PIPELINE_CREATE_DERIVATIVE_BIT "
                        "flag and pCreateInfos->basePipelineHandle is not VK_NULL_HANDLE.",
                        i);
                }
            } else {
                if (static_cast<uint32_t>(pCreateInfos[i].basePipelineIndex) >= createInfoCount) {
                    skip |= LogError(device, "VUID-VkComputePipelineCreateInfo-flags-00698",
                                     "vkCreateComputePipelines parameter pCreateInfos[%" PRIu32 "]->basePipelineIndex (%" PRIi32
                                     ") must be a valid index into the pCreateInfos array, of size %" PRIu32 ".",
                                     i, pCreateInfos[i].basePipelineIndex, createInfoCount);
                }
            }
        }

        std::stringstream msg;
        msg << "pCreateInfos[%" << i << "].stage";
        ValidatePipelineShaderStageCreateInfo("vkCreateComputePipelines", msg.str().c_str(), &pCreateInfos[i].stage);
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
        const auto *sampler_reduction = LvlFindInChain<VkSamplerReductionModeCreateInfo>(pCreateInfo->pNext);
        if (pCreateInfo->compareEnable == VK_TRUE) {
            skip |= ValidateRangedEnum("vkCreateSampler", "pCreateInfo->compareOp", "VkCompareOp", pCreateInfo->compareOp,
                                       "VUID-VkSamplerCreateInfo-compareEnable-01080");
            if (sampler_reduction != nullptr) {
                if (sampler_reduction->reductionMode != VK_SAMPLER_REDUCTION_MODE_WEIGHTED_AVERAGE) {
                    skip |= LogError(device, "VUID-VkSamplerCreateInfo-compareEnable-01423",
                                     "vkCreateSampler(): copmareEnable is true so the sampler reduction mode must be "
                                     "VK_SAMPLER_REDUCTION_MODE_WEIGHTED_AVERAGE.");
                }
            }
        }
        if (sampler_reduction && sampler_reduction->reductionMode != VK_SAMPLER_REDUCTION_MODE_WEIGHTED_AVERAGE) {
            if (!IsExtEnabled(device_extensions.vk_ext_filter_cubic)) {
                if (pCreateInfo->magFilter == VK_FILTER_CUBIC_EXT || pCreateInfo->minFilter == VK_FILTER_CUBIC_EXT) {
                    skip |= LogError(device, "VUID-VkSamplerCreateInfo-magFilter-01422",
                                     "vkCreateSampler(): sampler reduction mode is %s, magFilter is %s and minFilter is %s, but "
                                     "extension %s is not enabled.",
                                     string_VkSamplerReductionMode(sampler_reduction->reductionMode),
                                     string_VkFilter(pCreateInfo->magFilter), string_VkFilter(pCreateInfo->minFilter),
                                     VK_EXT_FILTER_CUBIC_EXTENSION_NAME);
                }
            }
        }

        // If any of addressModeU, addressModeV or addressModeW are VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, borderColor must be a
        // valid VkBorderColor value
        if ((pCreateInfo->addressModeU == VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER) ||
            (pCreateInfo->addressModeV == VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER) ||
            (pCreateInfo->addressModeW == VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER)) {
            skip |= ValidateRangedEnum("vkCreateSampler", "pCreateInfo->borderColor", "VkBorderColor", pCreateInfo->borderColor,
                                       "VUID-VkSamplerCreateInfo-addressModeU-01078");
        }

        // Checks for the IMG cubic filtering extension
        if (IsExtEnabled(device_extensions.vk_img_filter_cubic)) {
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

        const auto *sampler_conversion = LvlFindInChain<VkSamplerYcbcrConversionInfo>(pCreateInfo->pNext);
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

        if (pCreateInfo->borderColor == VK_BORDER_COLOR_INT_CUSTOM_EXT ||
            pCreateInfo->borderColor == VK_BORDER_COLOR_FLOAT_CUSTOM_EXT) {
            if (!IsExtEnabled(device_extensions.vk_ext_custom_border_color)) {
                skip |= LogError(device, kVUID_PVError_ExtensionNotEnabled,
                                 "VkSamplerCreateInfo->borderColor is %s but %s is not enabled.\n",
                                 string_VkBorderColor(pCreateInfo->borderColor), VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME);
            }
            auto custom_create_info = LvlFindInChain<VkSamplerCustomBorderColorCreateInfoEXT>(pCreateInfo->pNext);
            if (!custom_create_info) {
                skip |= LogError(
                    device, "VUID-VkSamplerCreateInfo-borderColor-04011",
                    "VkSamplerCreateInfo->borderColor is set to %s but there is no VkSamplerCustomBorderColorCreateInfoEXT "
                    "struct in pNext chain.\n",
                    string_VkBorderColor(pCreateInfo->borderColor));
            } else {
                if ((custom_create_info->format != VK_FORMAT_UNDEFINED) && !FormatIsDepthAndStencil(custom_create_info->format) &&
                    ((pCreateInfo->borderColor == VK_BORDER_COLOR_INT_CUSTOM_EXT &&
                      !FormatIsSampledInt(custom_create_info->format)) ||
                     (pCreateInfo->borderColor == VK_BORDER_COLOR_FLOAT_CUSTOM_EXT &&
                      !FormatIsSampledFloat(custom_create_info->format)))) {
                    skip |=
                        LogError(device, "VUID-VkSamplerCustomBorderColorCreateInfoEXT-format-07605",
                                 "VkSamplerCreateInfo->borderColor is %s but VkSamplerCustomBorderColorCreateInfoEXT.format = %s "
                                 "whose type does not match\n",
                                 string_VkBorderColor(pCreateInfo->borderColor), string_VkFormat(custom_create_info->format));
                    ;
                }
            }
        }

        const auto *border_color_component_mapping =
            LvlFindInChain<VkSamplerBorderColorComponentMappingCreateInfoEXT>(pCreateInfo->pNext);
        if (border_color_component_mapping) {
            const auto *border_color_swizzle_features =
                LvlFindInChain<VkPhysicalDeviceBorderColorSwizzleFeaturesEXT>(device_createinfo_pnext);
            bool border_color_swizzle_features_enabled =
                border_color_swizzle_features && border_color_swizzle_features->borderColorSwizzle;
            if (!border_color_swizzle_features_enabled) {
                skip |= LogError(device, "VUID-VkSamplerBorderColorComponentMappingCreateInfoEXT-borderColorSwizzle-06437",
                                 "vkCreateSampler(): The borderColorSwizzle feature must be enabled to use "
                                 "VkPhysicalDeviceBorderColorSwizzleFeaturesEXT");
            }
        }
    }

    return skip;
}

bool StatelessValidation::ValidateMutableDescriptorTypeCreateInfo(const VkDescriptorSetLayoutCreateInfo &create_info,
                                                                  const VkMutableDescriptorTypeCreateInfoEXT &mutable_create_info,
                                                                  const char *func_name) const {
    bool skip = false;

    for (uint32_t i = 0; i < create_info.bindingCount; ++i) {
        uint32_t mutable_type_count = 0;
        if (mutable_create_info.mutableDescriptorTypeListCount > i) {
            mutable_type_count = mutable_create_info.pMutableDescriptorTypeLists[i].descriptorTypeCount;
        }
        if (create_info.pBindings[i].descriptorType == VK_DESCRIPTOR_TYPE_MUTABLE_EXT) {
            if (mutable_type_count == 0) {
                skip |= LogError(device, "VUID-VkMutableDescriptorTypeListEXT-descriptorTypeCount-04597",
                                 "%s: VkDescriptorSetLayoutCreateInfo::pBindings[%" PRIu32
                                 "].descriptorType is VK_DESCRIPTOR_TYPE_MUTABLE_EXT, but "
                                 "VkMutableDescriptorTypeCreateInfoEXT::pMutableDescriptorTypeLists[%" PRIu32
                                 "].descriptorTypeCount is 0.",
                                 func_name, i, i);
            }
        } else {
            if (mutable_type_count > 0) {
                skip |= LogError(device, "VUID-VkMutableDescriptorTypeListEXT-descriptorTypeCount-04599",
                                 "%s: VkDescriptorSetLayoutCreateInfo::pBindings[%" PRIu32
                                 "].descriptorType is %s, but "
                                 "VkMutableDescriptorTypeCreateInfoEXT::pMutableDescriptorTypeLists[%" PRIu32
                                 "].descriptorTypeCount is not 0.",
                                 func_name, i, string_VkDescriptorType(create_info.pBindings[i].descriptorType), i);
            }
        }
    }

    for (uint32_t j = 0; j < mutable_create_info.mutableDescriptorTypeListCount; ++j) {
        for (uint32_t k = 0; k < mutable_create_info.pMutableDescriptorTypeLists[j].descriptorTypeCount; ++k) {
            switch (mutable_create_info.pMutableDescriptorTypeLists[j].pDescriptorTypes[k]) {
                case VK_DESCRIPTOR_TYPE_MUTABLE_EXT:
                    skip |= LogError(device, "VUID-VkMutableDescriptorTypeListEXT-pDescriptorTypes-04600",
                                     "%s: VkMutableDescriptorTypeCreateInfoEXT::pMutableDescriptorTypeLists[%" PRIu32
                                     "].pDescriptorTypes[%" PRIu32 "] is VK_DESCRIPTOR_TYPE_MUTABLE_EXT.",
                                     func_name, j, k);
                    break;
                case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
                    skip |= LogError(device, "VUID-VkMutableDescriptorTypeListEXT-pDescriptorTypes-04601",
                                     "%s: VkMutableDescriptorTypeCreateInfoEXT::pMutableDescriptorTypeLists[%" PRIu32
                                     "].pDescriptorTypes[%" PRIu32 "] is VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC.",
                                     func_name, j, k);
                    break;
                case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
                    skip |= LogError(device, "VUID-VkMutableDescriptorTypeListEXT-pDescriptorTypes-04602",
                                     "%s: VkMutableDescriptorTypeCreateInfoEXT::pMutableDescriptorTypeLists[%" PRIu32
                                     "].pDescriptorTypes[%" PRIu32 "] is VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC.",
                                     func_name, j, k);
                    break;
                case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT:
                    skip |= LogError(device, "VUID-VkMutableDescriptorTypeListEXT-pDescriptorTypes-04603",
                                     "%s: VkMutableDescriptorTypeCreateInfoEXT::pMutableDescriptorTypeLists[%" PRIu32
                                     "].pDescriptorTypes[%" PRIu32 "] is VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT.",
                                     func_name, j, k);
                    break;
                default:
                    break;
            }
            for (uint32_t l = k + 1; l < mutable_create_info.pMutableDescriptorTypeLists[j].descriptorTypeCount; ++l) {
                if (mutable_create_info.pMutableDescriptorTypeLists[j].pDescriptorTypes[k] ==
                    mutable_create_info.pMutableDescriptorTypeLists[j].pDescriptorTypes[l]) {
                    skip |=
                        LogError(device, "VUID-VkMutableDescriptorTypeListEXT-pDescriptorTypes-04598",
                                 "%s: VkMutableDescriptorTypeCreateInfoEXT::pMutableDescriptorTypeLists[%" PRIu32
                                 "].pDescriptorTypes[%" PRIu32
                                 "] and VkMutableDescriptorTypeCreateInfoEXT::pMutableDescriptorTypeLists[%" PRIu32
                                 "].pDescriptorTypes[%" PRIu32 "] are both %s.",
                                 func_name, j, k, j, l,
                                 string_VkDescriptorType(mutable_create_info.pMutableDescriptorTypeLists[j].pDescriptorTypes[k]));
                }
            }
        }
    }

    return skip;
}

#ifdef VK_USE_PLATFORM_METAL_EXT
bool StatelessValidation::ExportMetalObjectsPNextUtil(VkExportMetalObjectTypeFlagBitsEXT bit, const char *vuid,
                                                      const char *api_call, const char *sType, const void *pNext) const {
    bool skip = false;
    auto export_metal_object_info = LvlFindInChain<VkExportMetalObjectCreateInfoEXT>(pNext);
    while (export_metal_object_info) {
        if (export_metal_object_info->exportObjectType != bit) {
            std::stringstream message;
            message << api_call
                    << " The pNext chain contains a VkExportMetalObjectCreateInfoEXT whose "
                       "exportObjectType = %s, but only VkExportMetalObjectCreateInfoEXT structs with exportObjectType of "
                    << sType << " are allowed";
            skip |= LogError(device, vuid, message.str().c_str(),
                             string_VkExportMetalObjectTypeFlagBitsEXT(export_metal_object_info->exportObjectType));
        }
        export_metal_object_info = LvlFindInChain<VkExportMetalObjectCreateInfoEXT>(export_metal_object_info->pNext);
    }
    return skip;
}
#endif  // VK_USE_PLATFORM_METAL_EXT

bool StatelessValidation::manual_PreCallValidateCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo *pCreateInfo,
                                                                const VkAllocationCallbacks *pAllocator,
                                                                VkSemaphore *pSemaphore) const {
    bool skip = false;
#ifdef VK_USE_PLATFORM_METAL_EXT
    skip |= ExportMetalObjectsPNextUtil(
        VK_EXPORT_METAL_OBJECT_TYPE_METAL_SHARED_EVENT_BIT_EXT, "VUID-VkSemaphoreCreateInfo-pNext-06789",
        "vkCreateSemaphore():", "VK_EXPORT_METAL_OBJECT_TYPE_METAL_SHARED_EVENT_BIT_EXT", pCreateInfo->pNext);
#endif  // VK_USE_PLATFORM_METAL_EXT
    return skip;
}
bool StatelessValidation::manual_PreCallValidateCreateEvent(VkDevice device, const VkEventCreateInfo *pCreateInfo,
                                                            const VkAllocationCallbacks *pAllocator, VkEvent *pEvent) const {
    bool skip = false;
#ifdef VK_USE_PLATFORM_METAL_EXT
    skip |= ExportMetalObjectsPNextUtil(
        VK_EXPORT_METAL_OBJECT_TYPE_METAL_SHARED_EVENT_BIT_EXT, "VUID-VkEventCreateInfo-pNext-06790",
        "vkCreateEvent():", "VK_EXPORT_METAL_OBJECT_TYPE_METAL_SHARED_EVENT_BIT_EXT", pCreateInfo->pNext);
#endif  // VK_USE_PLATFORM_METAL_EXT
    return skip;
}
bool StatelessValidation::manual_PreCallValidateCreateBufferView(VkDevice device, const VkBufferViewCreateInfo *pCreateInfo,
                                                                 const VkAllocationCallbacks *pAllocator,
                                                                 VkBufferView *pBufferView) const {
    bool skip = false;
#ifdef VK_USE_PLATFORM_METAL_EXT
    skip |= ExportMetalObjectsPNextUtil(
        VK_EXPORT_METAL_OBJECT_TYPE_METAL_TEXTURE_BIT_EXT, "VUID-VkBufferViewCreateInfo-pNext-06782",
        "vkCreateBufferView():", "VK_EXPORT_METAL_OBJECT_TYPE_METAL_TEXTURE_BIT_EXT", pCreateInfo->pNext);
#endif  // VK_USE_PLATFORM_METAL_EXT
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCreateDescriptorSetLayout(VkDevice device,
                                                                          const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
                                                                          const VkAllocationCallbacks *pAllocator,
                                                                          VkDescriptorSetLayout *pSetLayout) const {
    bool skip = false;

    const auto *mutable_descriptor_type = LvlFindInChain<VkMutableDescriptorTypeCreateInfoEXT>(pCreateInfo->pNext);
    const auto *mutable_descriptor_type_features =
        LvlFindInChain<VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT>(device_createinfo_pnext);
    bool mutable_descriptor_type_features_enabled =
        mutable_descriptor_type_features && mutable_descriptor_type_features->mutableDescriptorType == VK_TRUE;

    // Validation for parameters excluded from the generated validation code due to a 'noautovalidity' tag in vk.xml
    if (pCreateInfo->pBindings != nullptr) {
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
                                             "pCreateInfo->pBindings[%" PRIu32 "].pImmutableSamplers[%" PRIu32
                                             "] specified as VK_NULL_HANDLE",
                                             i, descriptor_index);
                        }
                    }
                }

                // If descriptorCount is not 0, stageFlags must be a valid combination of VkShaderStageFlagBits values
                if ((pCreateInfo->pBindings[i].stageFlags != 0) &&
                    ((pCreateInfo->pBindings[i].stageFlags & (~AllVkShaderStageFlagBits)) != 0)) {
                    skip |= LogError(device, "VUID-VkDescriptorSetLayoutBinding-descriptorCount-00283",
                                     "vkCreateDescriptorSetLayout(): if pCreateInfo->pBindings[%" PRIu32
                                     "].descriptorCount is not 0, "
                                     "pCreateInfo->pBindings[%" PRIu32
                                     "].stageFlags must be a valid combination of VkShaderStageFlagBits "
                                     "values.",
                                     i, i);
                }

                if ((pCreateInfo->pBindings[i].descriptorType == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT) &&
                    (pCreateInfo->pBindings[i].stageFlags != 0) &&
                    (pCreateInfo->pBindings[i].stageFlags != VK_SHADER_STAGE_FRAGMENT_BIT)) {
                    skip |= LogError(device, "VUID-VkDescriptorSetLayoutBinding-descriptorType-01510",
                                     "vkCreateDescriptorSetLayout(): if pCreateInfo->pBindings[%" PRIu32
                                     "].descriptorCount is not 0 and "
                                     "descriptorType is VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT then pCreateInfo->pBindings[%" PRIu32
                                     "].stageFlags "
                                     "must be 0 or VK_SHADER_STAGE_FRAGMENT_BIT but is currently %s",
                                     i, i, string_VkShaderStageFlags(pCreateInfo->pBindings[i].stageFlags).c_str());
                }

                if (pCreateInfo->pBindings[i].descriptorType == VK_DESCRIPTOR_TYPE_MUTABLE_EXT) {
                    if (mutable_descriptor_type) {
                        if (i >= mutable_descriptor_type->mutableDescriptorTypeListCount) {
                            skip |= LogError(device, "VUID-VkDescriptorSetLayoutCreateInfo-pBindings-07303",
                                             "vkCreateDescriptorSetLayout(): pCreateInfo->pBindings[%" PRIu32
                                             "].descriptorType is VK_DESCRIPTOR_TYPE_MUTABLE_EXT but "
                                             "VkMutableDescriptorTypeCreateInfoEXT::mutableDescriptorTypeListCount is %" PRIu32
                                             " (not large enough to contain index %" PRIu32 ")",
                                             i, mutable_descriptor_type->mutableDescriptorTypeListCount, i);
                        }
                    } else {
                        skip |= LogError(device, "VUID-VkDescriptorSetLayoutCreateInfo-pBindings-07303",
                                         "vkCreateDescriptorSetLayout(): pCreateInfo->pBindings[%" PRIu32
                                         "].descriptorType is VK_DESCRIPTOR_TYPE_MUTABLE_EXT but "
                                         "VkMutableDescriptorTypeCreateInfoEXT is not included in the pNext chain.",
                                         i);
                    }
                    if (pCreateInfo->pBindings[i].pImmutableSamplers) {
                        skip |= LogError(device, "VUID-VkDescriptorSetLayoutCreateInfo-descriptorType-04594",
                                         "vkCreateDescriptorSetLayout(): pCreateInfo->pBindings[%" PRIu32
                                         "].descriptorType is VK_DESCRIPTOR_TYPE_MUTABLE_EXT but "
                                         "pImmutableSamplers is not NULL.",
                                         i);
                    }
                    if (!mutable_descriptor_type_features_enabled) {
                        skip |= LogError(
                            device, "VUID-VkDescriptorSetLayoutCreateInfo-mutableDescriptorType-04595",
                            "vkCreateDescriptorSetLayout(): pCreateInfo->pBindings[%" PRIu32
                            "].descriptorType is VK_DESCRIPTOR_TYPE_MUTABLE_EXT but "
                            "VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT::mutableDescriptorType feature is not enabled.",
                            i);
                    }
                }

                if (pCreateInfo->flags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR &&
                    pCreateInfo->pBindings[i].descriptorType == VK_DESCRIPTOR_TYPE_MUTABLE_EXT) {
                    skip |= LogError(device, "VUID-VkDescriptorSetLayoutCreateInfo-flags-04591",
                                     "vkCreateDescriptorSetLayout(): pCreateInfo->flags contains "
                                     "VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR, but pCreateInfo->pBindings[%" PRIu32
                                     "].descriptorType is VK_DESCRIPTOR_TYPE_MUTABLE_EXT.",
                                     i);
                }

                if (pCreateInfo->flags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT &&
                    ((pCreateInfo->pBindings[i].descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) ||
                     (pCreateInfo->pBindings[i].descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC))) {
                    skip |=
                        LogError(device, "VUID-VkDescriptorSetLayoutCreateInfo-flags-08000",
                                 "vkCreateDescriptorSetLayout(): pCreateInfo->flags contains "
                                 "VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT, but pCreateInfo->pBindings[%" PRIu32
                                 "].descriptorType is VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC or "
                                 "VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC.",
                                 i);
                }
            }
        }

        if (mutable_descriptor_type) {
            skip |=
                ValidateMutableDescriptorTypeCreateInfo(*pCreateInfo, *mutable_descriptor_type, "vkDescriptorSetLayoutCreateInfo");
        }
    }

    // TODO - Remove these 2 extension checks once the enum-to-extensions logic is generated
    // mostly likely will fail test trying to hit these
    if (pCreateInfo->flags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR &&
        !IsExtEnabled(device_extensions.vk_khr_push_descriptor)) {
        skip |= LogError(
            device, kVUID_Core_DrawState_ExtensionNotEnabled,
            "vkCreateDescriptorSetLayout(): Attempted to use %s in %s but its required extension %s has not been enabled.\n",
            "VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR", "VkDescriptorSetLayoutCreateInfo::flags",
            VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    }
    if (pCreateInfo->flags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT &&
        !IsExtEnabled(device_extensions.vk_ext_descriptor_indexing)) {
        skip |= LogError(
            device, kVUID_Core_DrawState_ExtensionNotEnabled,
            "vkCreateDescriptorSetLayout(): Attemped to use %s in %s but its required extension %s has not been enabled.\n",
            "VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT", "VkDescriptorSetLayoutCreateInfo::flags",
            VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    }

    if ((pCreateInfo->flags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR) &&
        (pCreateInfo->flags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_HOST_ONLY_POOL_BIT_EXT)) {
        skip |= LogError(device, "VUID-VkDescriptorSetLayoutCreateInfo-flags-04590",
                         "vkCreateDescriptorSetLayout(): pCreateInfo->flags contains both "
                         "VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR and "
                         "VK_DESCRIPTOR_SET_LAYOUT_CREATE_HOST_ONLY_POOL_BIT_EXT.");
    }
    if ((pCreateInfo->flags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT) &&
        (pCreateInfo->flags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_HOST_ONLY_POOL_BIT_EXT)) {
        skip |= LogError(device, "VUID-VkDescriptorSetLayoutCreateInfo-flags-04592",
                         "vkCreateDescriptorSetLayout(): pCreateInfo->flags contains both "
                         "VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT and "
                         "VK_DESCRIPTOR_SET_LAYOUT_CREATE_HOST_ONLY_POOL_BIT_EXT.");
    }
    if (pCreateInfo->flags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_HOST_ONLY_POOL_BIT_EXT && !mutable_descriptor_type_features_enabled) {
        skip |= LogError(device, "VUID-VkDescriptorSetLayoutCreateInfo-flags-04596",
                         "vkCreateDescriptorSetLayout(): pCreateInfo->flags contains "
                         "VK_DESCRIPTOR_SET_LAYOUT_CREATE_HOST_ONLY_POOL_BIT_EXT, but "
                         "VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT::mutableDescriptorType feature is not enabled.");
    }

    if ((pCreateInfo->flags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_EMBEDDED_IMMUTABLE_SAMPLERS_BIT_EXT) &&
        !(pCreateInfo->flags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT)) {
        skip |= LogError(device, "VUID-VkDescriptorSetLayoutCreateInfo-flags-08001",
                         "vkCreateDescriptorSetLayout(): pCreateInfo->flags contains "
                         "VK_DESCRIPTOR_SET_LAYOUT_CREATE_EMBEDDED_IMMUTABLE_SAMPLERS_BIT_EXT but not "
                         "VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT.");
    }

    if ((pCreateInfo->flags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT) &&
        (pCreateInfo->flags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT)) {
        skip |= LogError(device, "VUID-VkDescriptorSetLayoutCreateInfo-flags-08002",
                         "vkCreateDescriptorSetLayout(): pCreateInfo->flags contains both "
                         "VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT and "
                         "VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT.");
    }

    if ((pCreateInfo->flags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT) &&
        (pCreateInfo->flags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_HOST_ONLY_POOL_BIT_VALVE)) {
        skip |= LogError(device, "VUID-VkDescriptorSetLayoutCreateInfo-flags-08003",
                         "vkCreateDescriptorSetLayout(): pCreateInfo->flags contains both "
                         "VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT and "
                         "VK_DESCRIPTOR_SET_LAYOUT_CREATE_HOST_ONLY_POOL_BIT_VALVE.");
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool,
                                                                   uint32_t descriptorSetCount,
                                                                   const VkDescriptorSet *pDescriptorSets) const {
    // Validation for parameters excluded from the generated validation code due to a 'noautovalidity' tag in vk.xml
    // This is an array of handles, where the elements are allowed to be VK_NULL_HANDLE, and does not require any validation beyond
    // ValidateArray()
    return ValidateArray("vkFreeDescriptorSets", "descriptorSetCount", "pDescriptorSets", descriptorSetCount, &pDescriptorSets,
                         true, true, kVUIDUndefined, kVUIDUndefined);
}

bool StatelessValidation::ValidateWriteDescriptorSet(const char *vkCallingFunction, const uint32_t descriptorWriteCount,
                                                     const VkWriteDescriptorSet *pDescriptorWrites,
                                                     const bool isPushDescriptor) const {
    bool skip = false;

    if (pDescriptorWrites != NULL) {
        for (uint32_t i = 0; i < descriptorWriteCount; ++i) {
            // descriptorCount must be greater than 0
            if (pDescriptorWrites[i].descriptorCount == 0) {
                skip |= LogError(device, "VUID-VkWriteDescriptorSet-descriptorCount-arraylength",
                                 "%s(): parameter pDescriptorWrites[%" PRIu32 "].descriptorCount must be greater than 0.",
                                 vkCallingFunction, i);
            }

            // If called from vkCmdPushDescriptorSetKHR, the dstSet member is ignored.
            if (!isPushDescriptor) {
                // dstSet must be a valid VkDescriptorSet handle
                skip |= ValidateRequiredHandle(vkCallingFunction,
                                               ParameterName("pDescriptorWrites[%i].dstSet", ParameterName::IndexVector{i}),
                                               pDescriptorWrites[i].dstSet);
            }

            if ((pDescriptorWrites[i].descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER) ||
                (pDescriptorWrites[i].descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) ||
                (pDescriptorWrites[i].descriptorType == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE) ||
                (pDescriptorWrites[i].descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE) ||
                (pDescriptorWrites[i].descriptorType == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT)) {
                if (pDescriptorWrites[i].pImageInfo == nullptr) {
                    if (!isPushDescriptor) {
                        // If descriptorType is VK_DESCRIPTOR_TYPE_SAMPLER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                        // VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE or
                        // VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, pImageInfo must be a pointer to an array of descriptorCount valid
                        // VkDescriptorImageInfo structures. Valid imageView handles are checked in
                        // ObjectLifetimes::ValidateDescriptorWrite.
                        skip |= LogError(
                            device, "VUID-vkUpdateDescriptorSets-pDescriptorWrites-06493",
                            "%s(): if pDescriptorWrites[%" PRIu32
                            "].descriptorType is VK_DESCRIPTOR_TYPE_SAMPLER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, "
                            "VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE or "
                            "VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, pDescriptorWrites[%" PRIu32 "].pImageInfo must not be NULL.",
                            vkCallingFunction, i, i);
                    } else if ((pDescriptorWrites[i].descriptorType == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE) ||
                               (pDescriptorWrites[i].descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE) ||
                               (pDescriptorWrites[i].descriptorType == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT)) {
                        // If called from vkCmdPushDescriptorSetKHR, pImageInfo is only requred for descriptor types
                        // VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, and
                        // VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT
                        skip |= LogError(device, "VUID-vkCmdPushDescriptorSetKHR-pDescriptorWrites-06494",
                                         "%s(): if pDescriptorWrites[%" PRIu32
                                         "].descriptorType is VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE "
                                         "or VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, pDescriptorWrites[%" PRIu32
                                         "].pImageInfo must not be NULL.",
                                         vkCallingFunction, i, i);
                    }
                } else if (pDescriptorWrites[i].descriptorType != VK_DESCRIPTOR_TYPE_SAMPLER) {
                    // If descriptorType is VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                    // VK_DESCRIPTOR_TYPE_STORAGE_IMAGE or VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, the imageLayout
                    // member of any given element of pImageInfo must be a valid VkImageLayout
                    for (uint32_t descriptor_index = 0; descriptor_index < pDescriptorWrites[i].descriptorCount;
                         ++descriptor_index) {
                        skip |= ValidateRangedEnum(vkCallingFunction,
                                                   ParameterName("pDescriptorWrites[%i].pImageInfo[%i].imageLayout",
                                                                 ParameterName::IndexVector{i, descriptor_index}),
                                                   "VkImageLayout", pDescriptorWrites[i].pImageInfo[descriptor_index].imageLayout,
                                                   kVUIDUndefined);
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
                                     "%s(): if pDescriptorWrites[%" PRIu32
                                     "].descriptorType is "
                                     "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, "
                                     "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC or VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, "
                                     "pDescriptorWrites[%" PRIu32 "].pBufferInfo must not be NULL.",
                                     vkCallingFunction, i, i);
                } else {
                    const auto *robustness2_features =
                        LvlFindInChain<VkPhysicalDeviceRobustness2FeaturesEXT>(device_createinfo_pnext);
                    if (robustness2_features && robustness2_features->nullDescriptor) {
                        for (uint32_t descriptor_index = 0; descriptor_index < pDescriptorWrites[i].descriptorCount;
                             ++descriptor_index) {
                            if (pDescriptorWrites[i].pBufferInfo[descriptor_index].buffer == VK_NULL_HANDLE &&
                                (pDescriptorWrites[i].pBufferInfo[descriptor_index].offset != 0 ||
                                 pDescriptorWrites[i].pBufferInfo[descriptor_index].range != VK_WHOLE_SIZE)) {
                                skip |= LogError(device, "VUID-VkDescriptorBufferInfo-buffer-02999",
                                                 "%s(): if pDescriptorWrites[%" PRIu32
                                                 "].buffer is VK_NULL_HANDLE, "
                                                 "offset (%" PRIu64 ") must be zero and range (%" PRIu64 ") must be VK_WHOLE_SIZE.",
                                                 vkCallingFunction, i, pDescriptorWrites[i].pBufferInfo[descriptor_index].offset,
                                                 pDescriptorWrites[i].pBufferInfo[descriptor_index].range);
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
                VkDeviceSize uniform_alignment = device_limits.minUniformBufferOffsetAlignment;
                for (uint32_t j = 0; j < pDescriptorWrites[i].descriptorCount; j++) {
                    if (pDescriptorWrites[i].pBufferInfo != NULL) {
                        if (SafeModulo(pDescriptorWrites[i].pBufferInfo[j].offset, uniform_alignment) != 0) {
                            skip |=
                                LogError(device, "VUID-VkWriteDescriptorSet-descriptorType-00327",
                                         "%s(): pDescriptorWrites[%" PRIu32 "].pBufferInfo[%" PRIu32 "].offset (0x%" PRIxLEAST64
                                         ") must be a multiple of device limit minUniformBufferOffsetAlignment 0x%" PRIxLEAST64 ".",
                                         vkCallingFunction, i, j, pDescriptorWrites[i].pBufferInfo[j].offset, uniform_alignment);
                        }
                    }
                }
            } else if ((pDescriptorWrites[i].descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) ||
                       (pDescriptorWrites[i].descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)) {
                VkDeviceSize storage_alignment = device_limits.minStorageBufferOffsetAlignment;
                for (uint32_t j = 0; j < pDescriptorWrites[i].descriptorCount; j++) {
                    if (pDescriptorWrites[i].pBufferInfo != NULL) {
                        if (SafeModulo(pDescriptorWrites[i].pBufferInfo[j].offset, storage_alignment) != 0) {
                            skip |=
                                LogError(device, "VUID-VkWriteDescriptorSet-descriptorType-00328",
                                         "%s(): pDescriptorWrites[%" PRIu32 "].pBufferInfo[%" PRIu32 "].offset (0x%" PRIxLEAST64
                                         ") must be a multiple of device limit minStorageBufferOffsetAlignment 0x%" PRIxLEAST64 ".",
                                         vkCallingFunction, i, j, pDescriptorWrites[i].pBufferInfo[j].offset, storage_alignment);
                        }
                    }
                }
            }
            // pNext chain must be either NULL or a pointer to a valid instance of VkWriteDescriptorSetAccelerationStructureKHR
            // or VkWriteDescriptorSetInlineUniformBlockEX
            if (pDescriptorWrites[i].descriptorType == VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR) {
                const auto *pnext_struct = LvlFindInChain<VkWriteDescriptorSetAccelerationStructureKHR>(pDescriptorWrites[i].pNext);
                if (!pnext_struct || (pnext_struct->accelerationStructureCount != pDescriptorWrites[i].descriptorCount)) {
                    skip |= LogError(device, "VUID-VkWriteDescriptorSet-descriptorType-02382",
                                     "%s(): If descriptorType is VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, the pNext"
                                     "chain must include a VkWriteDescriptorSetAccelerationStructureKHR structure whose "
                                     "accelerationStructureCount %" PRIu32 " member equals descriptorCount %" PRIu32 ".",
                                     vkCallingFunction, pnext_struct ? pnext_struct->accelerationStructureCount : -1,
                                     pDescriptorWrites[i].descriptorCount);
                }
                // further checks only if we have right structtype
                if (pnext_struct) {
                    if (pnext_struct->accelerationStructureCount != pDescriptorWrites[i].descriptorCount) {
                        skip |= LogError(
                            device, "VUID-VkWriteDescriptorSetAccelerationStructureKHR-accelerationStructureCount-02236",
                            "%s(): accelerationStructureCount %" PRIu32 " must be equal to descriptorCount %" PRIu32
                            " in the extended structure "
                            ".",
                            vkCallingFunction, pnext_struct->accelerationStructureCount, pDescriptorWrites[i].descriptorCount);
                    }
                    if (pnext_struct->accelerationStructureCount == 0) {
                        skip |= LogError(device,
                                         "VUID-VkWriteDescriptorSetAccelerationStructureKHR-accelerationStructureCount-arraylength",
                                         "%s(): accelerationStructureCount must be greater than 0 .", vkCallingFunction);
                    }
                    const auto *robustness2_features =
                        LvlFindInChain<VkPhysicalDeviceRobustness2FeaturesEXT>(device_createinfo_pnext);
                    if (robustness2_features && robustness2_features->nullDescriptor == VK_FALSE) {
                        for (uint32_t j = 0; j < pnext_struct->accelerationStructureCount; ++j) {
                            if (pnext_struct->pAccelerationStructures[j] == VK_NULL_HANDLE) {
                                skip |= LogError(device,
                                                 "VUID-VkWriteDescriptorSetAccelerationStructureKHR-pAccelerationStructures-03580",
                                                 "%s(): If the nullDescriptor feature is not enabled, each member of "
                                                 "pAccelerationStructures must not be VK_NULL_HANDLE.",
                                                 vkCallingFunction);
                            }
                        }
                    }
                }
            } else if (pDescriptorWrites[i].descriptorType == VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV) {
                const auto *pnext_struct = LvlFindInChain<VkWriteDescriptorSetAccelerationStructureNV>(pDescriptorWrites[i].pNext);
                if (!pnext_struct || (pnext_struct->accelerationStructureCount != pDescriptorWrites[i].descriptorCount)) {
                    skip |= LogError(device, "VUID-VkWriteDescriptorSet-descriptorType-03817",
                                     "%s(): If descriptorType is VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV, the pNext"
                                     "chain must include a VkWriteDescriptorSetAccelerationStructureNV structure whose "
                                     "accelerationStructureCount %" PRIu32 " member equals descriptorCount %" PRIu32 ".",
                                     vkCallingFunction, pnext_struct ? pnext_struct->accelerationStructureCount : -1,
                                     pDescriptorWrites[i].descriptorCount);
                }
                // further checks only if we have right structtype
                if (pnext_struct) {
                    if (pnext_struct->accelerationStructureCount != pDescriptorWrites[i].descriptorCount) {
                        skip |= LogError(
                            device, "VUID-VkWriteDescriptorSetAccelerationStructureNV-accelerationStructureCount-03747",
                            "%s(): accelerationStructureCount %" PRIu32 " must be equal to descriptorCount %" PRIu32
                            " in the extended structure "
                            ".",
                            vkCallingFunction, pnext_struct->accelerationStructureCount, pDescriptorWrites[i].descriptorCount);
                    }
                    if (pnext_struct->accelerationStructureCount == 0) {
                        skip |= LogError(device,
                                         "VUID-VkWriteDescriptorSetAccelerationStructureNV-accelerationStructureCount-arraylength",
                                         "%s(): accelerationStructureCount must be greater than 0 .", vkCallingFunction);
                    }
                    const auto *robustness2_features =
                        LvlFindInChain<VkPhysicalDeviceRobustness2FeaturesEXT>(device_createinfo_pnext);
                    if (robustness2_features && robustness2_features->nullDescriptor == VK_FALSE) {
                        for (uint32_t j = 0; j < pnext_struct->accelerationStructureCount; ++j) {
                            if (pnext_struct->pAccelerationStructures[j] == VK_NULL_HANDLE) {
                                skip |= LogError(device,
                                                 "VUID-VkWriteDescriptorSetAccelerationStructureNV-pAccelerationStructures-03749",
                                                 "%s(): If the nullDescriptor feature is not enabled, each member of "
                                                 "pAccelerationStructures must not be VK_NULL_HANDLE.",
                                                 vkCallingFunction);
                            }
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
    return ValidateWriteDescriptorSet("vkUpdateDescriptorSets", descriptorWriteCount, pDescriptorWrites, false);
}

template <typename RenderPassCreateInfoGeneric>
bool StatelessValidation::CreateRenderPassGeneric(VkDevice device, const RenderPassCreateInfoGeneric *pCreateInfo,
                                                  const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass,
                                                  RenderPassCreateVersion rp_version) const {
    bool skip = false;
    uint32_t max_color_attachments = device_limits.maxColorAttachments;
    const bool use_rp2 = (rp_version == RENDER_PASS_VERSION_2);
    const char *func_name = (use_rp2) ? "vkCreateRenderPass2" : "vkCreateRenderPass";
    const char *vuid = nullptr;
    VkBool32 separate_depth_stencil_layouts = false;
    const auto *vulkan_12_features = LvlFindInChain<VkPhysicalDeviceVulkan12Features>(device_createinfo_pnext);
    if (vulkan_12_features) {
        separate_depth_stencil_layouts = vulkan_12_features->separateDepthStencilLayouts;
    } else {
        const auto *separate_depth_stencil_layouts_features =
            LvlFindInChain<VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures>(device_createinfo_pnext);
        if (separate_depth_stencil_layouts_features) {
            separate_depth_stencil_layouts = separate_depth_stencil_layouts_features->separateDepthStencilLayouts;
        }
    }

    VkBool32 attachment_feedback_loop_layout = false;
    const auto *attachment_feedback_loop_layout_features =
        LvlFindInChain<VkPhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT>(device_createinfo_pnext);
    if (attachment_feedback_loop_layout_features) {
        attachment_feedback_loop_layout = attachment_feedback_loop_layout_features->attachmentFeedbackLoopLayout;
    }

    VkBool32 synchronization2 = false;
    const auto *vulkan_13_features = LvlFindInChain<VkPhysicalDeviceVulkan13Features>(device_createinfo_pnext);
    if (vulkan_13_features) {
        synchronization2 = vulkan_13_features->synchronization2;
    } else {
        const auto *synchronization2_features = LvlFindInChain<VkPhysicalDeviceSynchronization2Features>(device_createinfo_pnext);
        if (synchronization2_features) {
            synchronization2 = synchronization2_features->synchronization2;
        }
    }
    for (uint32_t i = 0; i < pCreateInfo->attachmentCount; ++i) {
        // if not null, also confirms rp2 is being used
        const auto *attachment_description_stencil_layout =
            (use_rp2) ? LvlFindInChain<VkAttachmentDescriptionStencilLayout>(
                            reinterpret_cast<VkAttachmentDescription2 const *>(&pCreateInfo->pAttachments[i])->pNext)
                      : nullptr;

        const VkFormat attachment_format = pCreateInfo->pAttachments[i].format;
        const VkImageLayout initial_layout = pCreateInfo->pAttachments[i].initialLayout;
        const VkImageLayout final_layout = pCreateInfo->pAttachments[i].finalLayout;
        if (attachment_format == VK_FORMAT_UNDEFINED) {
            vuid = use_rp2 ? "VUID-VkAttachmentDescription2-format-06698" : "VUID-VkAttachmentDescription-format-06698";
            skip |=
                LogError(device, vuid, "%s: pCreateInfo->pAttachments[%" PRIu32 "].format is VK_FORMAT_UNDEFINED.", func_name, i);
        }
        if (final_layout == VK_IMAGE_LAYOUT_UNDEFINED || final_layout == VK_IMAGE_LAYOUT_PREINITIALIZED) {
            vuid = use_rp2 ? "VUID-VkAttachmentDescription2-finalLayout-00843" : "VUID-VkAttachmentDescription-finalLayout-00843";
            skip |= LogError(device, vuid,
                             "%s: pCreateInfo->pAttachments[%" PRIu32
                             "].finalLayout must not be VK_IMAGE_LAYOUT_UNDEFINED or "
                             "VK_IMAGE_LAYOUT_PREINITIALIZED.",
                             func_name, i);
        }
        if (!separate_depth_stencil_layouts) {
            if (initial_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL ||
                initial_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL ||
                initial_layout == VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL ||
                initial_layout == VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-separateDepthStencilLayouts-03284"
                               : "VUID-VkAttachmentDescription-separateDepthStencilLayouts-03284";
                skip |= LogError(device, vuid,
                                 "%s: pCreateInfo->pAttachments[%" PRIu32
                                 "].initialLayout must not be VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, "
                                 "VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL, or "
                                 "VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL",
                                 func_name, i);
            }
            if (final_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL ||
                final_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL ||
                final_layout == VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL ||
                final_layout == VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-separateDepthStencilLayouts-03285"
                               : "VUID-VkAttachmentDescription-separateDepthStencilLayouts-03285";
                skip |= LogError(device, vuid,
                                 "%s: pCreateInfo->pAttachments[%" PRIu32
                                 "].finalLayout must not be VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, "
                                 "VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL, or "
                                 "VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL",
                                 func_name, i);
            }
        }
        if (!attachment_feedback_loop_layout) {
            if (initial_layout == VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-attachmentFeedbackLoopLayout-07309"
                               : "VUID-VkAttachmentDescription-attachmentFeedbackLoopLayout-07309";
                skip |= LogError(device, vuid,
                                 "%s: pCreateInfo->pAttachments[%" PRIu32
                                 "] initialLayout is VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT but the "
                                 "attachmentFeedbackLoopLayout feature is not enabled",
                                 func_name, i);
            }
            if (final_layout == VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-attachmentFeedbackLoopLayout-07310"
                               : "VUID-VkAttachmentDescription-attachmentFeedbackLoopLayout-07310";
                skip |= LogError(device, vuid,
                                 "%s: pCreateInfo->pAttachments[%" PRIu32
                                 "] finalLayout is VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT but the "
                                 "attachmentFeedbackLoopLayout feature is not enabled",
                                 func_name, i);
            }
        }
        if (!synchronization2) {
            if (initial_layout == VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR ||
                initial_layout == VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL_KHR) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-synchronization2-06908"
                               : "VUID-VkAttachmentDescription-synchronization2-06908";
                skip |= LogError(
                    device, vuid,
                    "%s: pCreateInfo->pAttachments[%" PRIu32
                    "] initialLayout is VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR or VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL_KHR but the "
                    "synchronization2 feature is not enabled",
                    func_name, i);
            }
            if (final_layout == VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR || final_layout == VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL_KHR) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-synchronization2-06909"
                               : "VUID-VkAttachmentDescription-synchronization2-06909";
                skip |= LogError(
                    device, vuid,
                    "%s: pCreateInfo->pAttachments[%" PRIu32
                    "] finalLayout is VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR or VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL_KHR but the "
                    "synchronization2 feature is not enabled",
                    func_name, i);
            }
        }
        if (!FormatIsDepthOrStencil(attachment_format)) {  // color format
            if (initial_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL ||
                initial_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL ||
                initial_layout == VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL ||
                initial_layout == VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-format-03286" : "VUID-VkAttachmentDescription-format-03286";
                skip |= LogError(device, vuid,
                                 "%s: pCreateInfo->pAttachments[%" PRIu32
                                 "].initialLayout must not be VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, "
                                 "VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL, or "
                                 "VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL",
                                 func_name, i);
            }
            if (final_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL ||
                final_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL ||
                final_layout == VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL ||
                final_layout == VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-format-03287" : "VUID-VkAttachmentDescription-format-03287";
                skip |= LogError(device, vuid,
                                 "%s: pCreateInfo->pAttachments[%" PRIu32
                                 "].finalLayout must not be VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, "
                                 "VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL, or "
                                 "VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL",
                                 func_name, i);
            }
        } else if (FormatIsDepthAndStencil(attachment_format)) {
            if (initial_layout == VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL ||
                initial_layout == VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-format-06906" : "VUID-VkAttachmentDescription-format-06906";
                skip |= LogError(device, vuid,
                                 "%s: pCreateInfo->pAttachments[%" PRIu32
                                 "].initialLayout must not be "
                                 "VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL or "
                                 "VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL",
                                 func_name, i);
            }
            if (final_layout == VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL ||
                final_layout == VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-format-06907" : "VUID-VkAttachmentDescription-format-06907";
                skip |= LogError(device, vuid,
                                 "%s: pCreateInfo->pAttachments[%" PRIu32
                                 "].finalLayout must not be "
                                 "VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL or "
                                 "VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL",
                                 func_name, i);
            }

            if (!attachment_description_stencil_layout) {
                if (initial_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL ||
                    initial_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL) {
                    vuid = use_rp2 ? "VUID-VkAttachmentDescription2-format-06249" : "VUID-VkAttachmentDescription-format-06242";
                    skip |=
                        LogError(device, vuid,
                                 "%s: pCreateInfo->pAttachments[%" PRIu32
                                 "].initialLayout must not be "
                                 "VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL or "
                                 "VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL with no VkAttachmentDescriptionStencilLayout provided",
                                 func_name, i);
                }
                if (final_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL ||
                    final_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL) {
                    vuid = use_rp2 ? "VUID-VkAttachmentDescription2-format-06250" : "VUID-VkAttachmentDescription-format-06243";
                    skip |=
                        LogError(device, vuid,
                                 "%s: pCreateInfo->pAttachments[%" PRIu32
                                 "].finalLayout must not be "
                                 "VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL or "
                                 "VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL with no VkAttachmentDescriptionStencilLayout provided",
                                 func_name, i);
                }
            }
        } else if (FormatIsDepthOnly(attachment_format)) {
            if (initial_layout == VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL ||
                initial_layout == VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-format-03290" : "VUID-VkAttachmentDescription-format-03290";
                skip |= LogError(device, vuid,
                                 "%s: pCreateInfo->pAttachments[%" PRIu32
                                 "].initialLayout must not be "
                                 "VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL or"
                                 "VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL",
                                 func_name, i);
            }
            if (final_layout == VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL ||
                final_layout == VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-format-03291" : "VUID-VkAttachmentDescription-format-03291";
                skip |= LogError(device, vuid,
                                 "%s: pCreateInfo->pAttachments[%" PRIu32
                                 "].finalLayout must not be "
                                 "VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL or "
                                 "VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL",
                                 func_name, i);
            }
        } else if (FormatIsStencilOnly(attachment_format) && !attachment_description_stencil_layout) {
            if (initial_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL ||
                initial_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-format-06247" : "VUID-VkAttachmentDescription-format-03292";
                skip |= LogError(device, vuid,
                                 "%s: pCreateInfo->pAttachments[%" PRIu32
                                 "].initialLayout must not be "
                                 "VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL or"
                                 "VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL",
                                 func_name, i);
            }
            if (final_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL ||
                final_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-format-06248" : "VUID-VkAttachmentDescription-format-03293";
                skip |= LogError(device, vuid,
                                 "%s: pCreateInfo->pAttachments[%" PRIu32
                                 "].finalLayout must not be "
                                 "VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL or "
                                 "VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL",
                                 func_name, i);
            }
        }
        if (attachment_description_stencil_layout) {
            const VkImageLayout stencil_initial_layout = attachment_description_stencil_layout->stencilInitialLayout;
            const VkImageLayout stencil_final_layout = attachment_description_stencil_layout->stencilFinalLayout;

            if (stencil_initial_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL ||
                stencil_initial_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL ||
                stencil_initial_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL ||
                stencil_initial_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
                stencil_initial_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL ||
                stencil_initial_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL ||
                stencil_initial_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL) {
                skip |= LogError(device, "VUID-VkAttachmentDescriptionStencilLayout-stencilInitialLayout-03308",
                                 "%s: pCreateInfo->pAttachments[%" PRIu32
                                 "] VkAttachmentDescriptionStencilLayout.stencilInitialLayout must not be "
                                 "VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, "
                                 "VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL, "
                                 "VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, "
                                 "VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, "
                                 "VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL, or "
                                 "VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL.",
                                 func_name, i);
            }
            if (stencil_final_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL ||
                stencil_final_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL ||
                stencil_final_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL ||
                stencil_final_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
                stencil_final_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL ||
                stencil_final_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL ||
                stencil_final_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL) {
                skip |= LogError(device, "VUID-VkAttachmentDescriptionStencilLayout-stencilFinalLayout-03309",
                                 "%s: pCreateInfo->pAttachments[%" PRIu32
                                 "] VkAttachmentDescriptionStencilLayout.stencilFinalLayout must not be "
                                 "VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, "
                                 "VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL, "
                                 "VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, "
                                 "VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, "
                                 "VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL, or "
                                 "VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL.",
                                 func_name, i);
            }
            if (stencil_final_layout == VK_IMAGE_LAYOUT_UNDEFINED || stencil_final_layout == VK_IMAGE_LAYOUT_PREINITIALIZED) {
                skip |=
                    LogError(device, "VUID-VkAttachmentDescriptionStencilLayout-stencilFinalLayout-03310",
                             "%s: pCreateInfo->pAttachments[%" PRIu32
                             "] VkAttachmentDescriptionStencilLayout.stencilFinalLayout must not be VK_IMAGE_LAYOUT_UNDEFINED, or "
                             "VK_IMAGE_LAYOUT_PREINITIALIZED.",
                             func_name, i);
            }
        }

        if (FormatIsDepthOrStencil(attachment_format)) {
            if (initial_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-format-03281" : "VUID-VkAttachmentDescription-format-03281";
                skip |= LogError(device, vuid,
                                 "%s: pCreateInfo->pAttachments[%" PRIu32
                                 "].initialLayout must not be "
                                 "VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL when using a Depth or Stencil format",
                                 func_name, i);
            }
            if (final_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-format-03283" : "VUID-VkAttachmentDescription-format-03283";
                skip |= LogError(device, vuid,
                                 "%s: pCreateInfo->pAttachments[%" PRIu32
                                 "].finalLayout must not be "
                                 "VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL when using a Depth or Stencil format",
                                 func_name, i);
            }
        }
        if (FormatIsColor(attachment_format)) {
            if (initial_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
                initial_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-format-03280" : "VUID-VkAttachmentDescription-format-03280";
                skip |= LogError(device, vuid,
                                 "%s: pCreateInfo->pAttachments[%" PRIu32
                                 "].initialLayout must not be "
                                 "VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL or "
                                 "VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL when using a Color format",
                                 func_name, i);

            } else if (initial_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL ||
                       initial_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-format-06487" : "VUID-VkAttachmentDescription-format-06487";
                skip |= LogError(device, vuid,
                                 "%s: pCreateInfo->pAttachments[%" PRIu32
                                 "].initialLayout must not be "
                                 "VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL or "
                                 "VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL when using a Color format",
                                 func_name, i);
            }
            if (final_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
                final_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-format-03282" : "VUID-VkAttachmentDescription-format-03282";
                skip |= LogError(device, vuid,
                                 "%s: pCreateInfo->pAttachments[%" PRIu32
                                 "].finalLayout must not be "
                                 "VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL or "
                                 "VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL when using a Color format",
                                 func_name, i);
            } else if (final_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL ||
                       final_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-format-06488" : "VUID-VkAttachmentDescription-format-06488";
                skip |= LogError(device, vuid,
                                 "%s: pCreateInfo->pAttachments[%" PRIu32
                                 "].finalLayout must not be "
                                 "VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL or "
                                 "VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL when using a Color format",
                                 func_name, i);
            }
        }
        if (FormatIsColor(attachment_format) || FormatHasDepth(attachment_format)) {
            if (pCreateInfo->pAttachments[i].loadOp == VK_ATTACHMENT_LOAD_OP_LOAD && initial_layout == VK_IMAGE_LAYOUT_UNDEFINED) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-format-06699" : "VUID-VkAttachmentDescription-format-06699";
                skip |= LogError(
                    device, vuid,
                    "%s: pCreateInfo->pAttachments[%" PRIu32
                    "] format is %s and loadOp is VK_ATTACHMENT_LOAD_OP_LOAD, but initialLayout is VK_IMAGE_LAYOUT_UNDEFINED.",
                    func_name, i, string_VkFormat(attachment_format));
            }
        }
        if (FormatHasStencil(attachment_format) && pCreateInfo->pAttachments[i].stencilLoadOp == VK_ATTACHMENT_LOAD_OP_LOAD) {
            if (initial_layout == VK_IMAGE_LAYOUT_UNDEFINED) {
                if (use_rp2) {
                    skip |= LogError(device, "VUID-VkAttachmentDescription2-pNext-06704",
                                     "%s: pCreateInfo->pAttachments[%" PRIu32
                                     "] format includes stencil aspect and stencilLoadOp is VK_ATTACHMENT_LOAD_OP_LOAD, but "
                                     "the initialLayout is VK_IMAGE_LAYOUT_UNDEFINED.",
                                     func_name, i);
                } else if (!IsExtEnabled(device_extensions.vk_khr_separate_depth_stencil_layouts)) {
                    vuid = use_rp2 ? "VUID-VkAttachmentDescription2-format-06703" : "VUID-VkAttachmentDescription-format-06700";
                    skip |= LogError(device, vuid,
                                     "%s: pCreateInfo->pAttachments[%" PRIu32
                                     "] format is %s and stencilLoadOp is VK_ATTACHMENT_LOAD_OP_LOAD, but initialLayout is "
                                     "VK_IMAGE_LAYOUT_UNDEFINED.",
                                     func_name, i, string_VkFormat(attachment_format));
                }
            }

            // rp2 can have seperate depth/stencil layout and need to look in pNext
            if (attachment_description_stencil_layout) {
                if (attachment_description_stencil_layout->stencilInitialLayout == VK_IMAGE_LAYOUT_UNDEFINED) {
                    skip |= LogError(device, "VUID-VkAttachmentDescription2-pNext-06705",
                                     "%s: pCreateInfo->pAttachments[%" PRIu32
                                     "] format includes stencil aspect and stencilLoadOp is VK_ATTACHMENT_LOAD_OP_LOAD, but "
                                     "the VkAttachmentDescriptionStencilLayout::stencilInitialLayout is VK_IMAGE_LAYOUT_UNDEFINED.",
                                     func_name, i);
                }
            }
        }
    }

    for (uint32_t i = 0; i < pCreateInfo->subpassCount; ++i) {
        if (pCreateInfo->pSubpasses[i].colorAttachmentCount > max_color_attachments) {
            vuid = use_rp2 ? "VUID-VkSubpassDescription2-colorAttachmentCount-03063"
                           : "VUID-VkSubpassDescription-colorAttachmentCount-00845";
            skip |= LogError(device, vuid,
                             "%s: Cannot create a render pass with %d color attachments in pCreateInfo->pSubpasses[%u]. Max is %d.",
                             func_name, pCreateInfo->pSubpasses[i].colorAttachmentCount, i, max_color_attachments);
        }
    }

    for (uint32_t i = 0; i < pCreateInfo->dependencyCount; ++i) {
        const auto &dependency = pCreateInfo->pDependencies[i];

        // Need to check first so layer doesn't segfault from out of bound array access
        // src subpass bound check
        if ((dependency.srcSubpass != VK_SUBPASS_EXTERNAL) && (dependency.srcSubpass >= pCreateInfo->subpassCount)) {
            vuid = use_rp2 ? "VUID-VkRenderPassCreateInfo2-srcSubpass-02526" : "VUID-VkRenderPassCreateInfo-pDependencies-06866";
            skip |= LogError(device, vuid,
                             "%s: pCreateInfo->pDependencies[%u].srcSubpass index (%u) has to be less than subpassCount (%u)",
                             func_name, i, dependency.srcSubpass, pCreateInfo->subpassCount);
        }

        // dst subpass bound check
        if ((dependency.dstSubpass != VK_SUBPASS_EXTERNAL) && (dependency.dstSubpass >= pCreateInfo->subpassCount)) {
            vuid = use_rp2 ? "VUID-VkRenderPassCreateInfo2-dstSubpass-02527" : "VUID-VkRenderPassCreateInfo-pDependencies-06867";
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

bool StatelessValidation::manual_PreCallValidateCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo *pCreateInfo,
                                                                 const VkAllocationCallbacks *pAllocator,
                                                                 VkRenderPass *pRenderPass) const {
    return CreateRenderPassGeneric(device, pCreateInfo, pAllocator, pRenderPass, RENDER_PASS_VERSION_1);
}

bool StatelessValidation::manual_PreCallValidateCreateRenderPass2(VkDevice device, const VkRenderPassCreateInfo2 *pCreateInfo,
                                                                  const VkAllocationCallbacks *pAllocator,
                                                                  VkRenderPass *pRenderPass) const {
    return CreateRenderPassGeneric(device, pCreateInfo, pAllocator, pRenderPass, RENDER_PASS_VERSION_2);
}

bool StatelessValidation::manual_PreCallValidateCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2 *pCreateInfo,
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
    // ValidateArray()
    skip |= ValidateArray("vkFreeCommandBuffers", "commandBufferCount", "pCommandBuffers", commandBufferCount, &pCommandBuffers,
                          true, true, kVUIDUndefined, kVUIDUndefined);
    return skip;
}

bool StatelessValidation::manual_PreCallValidateBeginCommandBuffer(VkCommandBuffer commandBuffer,
                                                                   const VkCommandBufferBeginInfo *pBeginInfo) const {
    bool skip = false;

    // VkCommandBufferInheritanceInfo validation, due to a 'noautovalidity' of pBeginInfo->pInheritanceInfo in vkBeginCommandBuffer
    const char *cmd_name = "vkBeginCommandBuffer";
    bool cb_is_secondary;
    {
        auto lock = CBReadLock();
        cb_is_secondary = (secondary_cb_map.find(commandBuffer) != secondary_cb_map.end());
    }

    if (cb_is_secondary) {
        // Implicit VUs
        // validate only sType here; pointer has to be validated in core_validation
        const bool k_not_required = false;
        const char *k_no_vuid = nullptr;
        const VkCommandBufferInheritanceInfo *info = pBeginInfo->pInheritanceInfo;
        skip |= ValidateStructType(cmd_name, "pBeginInfo->pInheritanceInfo", "VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO",
                                   info, VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO, k_not_required, k_no_vuid,
                                   "VUID-VkCommandBufferInheritanceInfo-sType-sType");

        if (info) {
            constexpr std::array allowed_structs = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_CONDITIONAL_RENDERING_INFO_EXT,
                                                    VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_RENDERING_INFO_KHR,
                                                    VK_STRUCTURE_TYPE_ATTACHMENT_SAMPLE_COUNT_INFO_AMD,
                                                    VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_VIEWPORT_SCISSOR_INFO_NV};
            skip |= ValidateStructPnext(
                cmd_name, "pBeginInfo->pInheritanceInfo->pNext", "VkCommandBufferInheritanceConditionalRenderingInfoEXT",
                info->pNext, allowed_structs.size(), allowed_structs.data(), GeneratedVulkanHeaderVersion,
                "VUID-VkCommandBufferInheritanceInfo-pNext-pNext", "VUID-VkCommandBufferInheritanceInfo-sType-unique");

            skip |= ValidateBool32(cmd_name, "pBeginInfo->pInheritanceInfo->occlusionQueryEnable", info->occlusionQueryEnable);

            // Explicit VUs
            if (!physical_device_features.inheritedQueries && info->occlusionQueryEnable == VK_TRUE) {
                skip |= LogError(
                    commandBuffer, "VUID-VkCommandBufferInheritanceInfo-occlusionQueryEnable-00056",
                    "%s: Inherited queries feature is disabled, but pBeginInfo->pInheritanceInfo->occlusionQueryEnable is VK_TRUE.",
                    cmd_name);
            }

            if (physical_device_features.inheritedQueries) {
                skip |= ValidateFlags(cmd_name, "pBeginInfo->pInheritanceInfo->queryFlags", "VkQueryControlFlagBits",
                                      AllVkQueryControlFlagBits, info->queryFlags, kOptionalFlags,
                                      "VUID-VkCommandBufferInheritanceInfo-queryFlags-00057");
            } else {  // !inheritedQueries
                skip |= ValidateReservedFlags(cmd_name, "pBeginInfo->pInheritanceInfo->queryFlags", info->queryFlags,
                                              "VUID-VkCommandBufferInheritanceInfo-queryFlags-02788");
            }

            if (physical_device_features.pipelineStatisticsQuery) {
                skip |=
                    ValidateFlags(cmd_name, "pBeginInfo->pInheritanceInfo->pipelineStatistics", "VkQueryPipelineStatisticFlagBits",
                                  AllVkQueryPipelineStatisticFlagBits, info->pipelineStatistics, kOptionalFlags,
                                  "VUID-VkCommandBufferInheritanceInfo-pipelineStatistics-02789");
            } else {  // !pipelineStatisticsQuery
                skip |=
                    ValidateReservedFlags(cmd_name, "pBeginInfo->pInheritanceInfo->pipelineStatistics", info->pipelineStatistics,
                                          "VUID-VkCommandBufferInheritanceInfo-pipelineStatistics-00058");
            }

            const auto *conditional_rendering = LvlFindInChain<VkCommandBufferInheritanceConditionalRenderingInfoEXT>(info->pNext);
            if (conditional_rendering) {
                const auto *cr_features = LvlFindInChain<VkPhysicalDeviceConditionalRenderingFeaturesEXT>(device_createinfo_pnext);
                const auto inherited_conditional_rendering = cr_features && cr_features->inheritedConditionalRendering;
                if (!inherited_conditional_rendering && conditional_rendering->conditionalRenderingEnable == VK_TRUE) {
                    skip |= LogError(
                        commandBuffer,
                        "VUID-VkCommandBufferInheritanceConditionalRenderingInfoEXT-conditionalRenderingEnable-01977",
                        "vkBeginCommandBuffer: Inherited conditional rendering is disabled, but "
                        "pBeginInfo->pInheritanceInfo->pNext<VkCommandBufferInheritanceConditionalRenderingInfoEXT> is VK_TRUE.");
                }
            }

            auto p_inherited_viewport_scissor_info = LvlFindInChain<VkCommandBufferInheritanceViewportScissorInfoNV>(info->pNext);
            if (p_inherited_viewport_scissor_info != nullptr && !physical_device_features.multiViewport &&
                p_inherited_viewport_scissor_info->viewportScissor2D == VK_TRUE &&
                p_inherited_viewport_scissor_info->viewportDepthCount != 1) {
                skip |= LogError(commandBuffer, "VUID-VkCommandBufferInheritanceViewportScissorInfoNV-viewportScissor2D-04783",
                                 "vkBeginCommandBuffer: multiViewport feature is disabled, but "
                                 "VkCommandBufferInheritanceViewportScissorInfoNV::viewportScissor2D in "
                                 "pBeginInfo->pInheritanceInfo->pNext is VK_TRUE and viewportDepthCount is not 1.");
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
            if (x_sum > vvl::kI32Max) {
                skip |= LogError(commandBuffer, "VUID-vkCmdSetScissor-offset-00596",
                                 "vkCmdSetScissor: offset.x + extent.width (=%" PRIi32 " + %" PRIu32 " = %" PRIi64
                                 ") of pScissors[%" PRIu32 "] will overflow int32_t.",
                                 scissor.offset.x, scissor.extent.width, x_sum, scissor_i);
            }

            const int64_t y_sum = static_cast<int64_t>(scissor.offset.y) + static_cast<int64_t>(scissor.extent.height);
            if (y_sum > vvl::kI32Max) {
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
                                                                uint32_t drawCount, uint32_t stride) const {
    bool skip = false;

    if (!physical_device_features.multiDrawIndirect && ((drawCount > 1))) {
        skip |= LogError(device, "VUID-vkCmdDrawIndirect-drawCount-02718",
                         "vkCmdDrawIndirect(): Device feature multiDrawIndirect disabled: count must be 0 or 1 but is %" PRIu32 "",
                         drawCount);
    }
    if (drawCount > device_limits.maxDrawIndirectCount) {
        skip |=
            LogError(commandBuffer, "VUID-vkCmdDrawIndirect-drawCount-02719",
                     "vkCmdDrawIndirect(): drawCount (%" PRIu32 ") is not less than or equal to the maximum allowed (%" PRIu32 ").",
                     drawCount, device_limits.maxDrawIndirectCount);
    }
    if (offset & 3) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDrawIndirect-offset-02710",
                         "vkCmdDrawIndirect(): offset (%" PRIxLEAST64 ") must be a multiple of 4.", offset);
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                       VkDeviceSize offset, uint32_t drawCount,
                                                                       uint32_t stride) const {
    bool skip = false;
    if (!physical_device_features.multiDrawIndirect && ((drawCount > 1))) {
        skip |= LogError(
            device, "VUID-vkCmdDrawIndexedIndirect-drawCount-02718",
            "vkCmdDrawIndexedIndirect(): Device feature multiDrawIndirect disabled: count must be 0 or 1 but is %" PRIu32 "",
            drawCount);
    }
    if (drawCount > device_limits.maxDrawIndirectCount) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDrawIndexedIndirect-drawCount-02719",
                         "vkCmdDrawIndexedIndirect(): drawCount (%" PRIu32
                         ") is not less than or equal to the maximum allowed (%" PRIu32 ").",
                         drawCount, device_limits.maxDrawIndirectCount);
    }
    if (offset & 3) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDrawIndexedIndirect-offset-02710",
                         "vkCmdDrawIndexedIndirect(): offset (%" PRIxLEAST64 ") must be a multiple of 4.", offset);
    }
    return skip;
}

bool StatelessValidation::ValidateCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkDeviceSize offset,
                                                       VkDeviceSize countBufferOffset, CMD_TYPE cmd_type) const {
    bool skip = false;
    if (offset & 3) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDrawIndirectCount-offset-02710",
                         "%s: parameter, VkDeviceSize offset (0x%" PRIxLEAST64 "), is not a multiple of 4.",
                         CommandTypeString(cmd_type), offset);
    }

    if (countBufferOffset & 3) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDrawIndirectCount-countBufferOffset-02716",
                         "%s: parameter, VkDeviceSize countBufferOffset (0x%" PRIxLEAST64 "), is not a multiple of 4.",
                         CommandTypeString(cmd_type), countBufferOffset);
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                     VkDeviceSize offset, VkBuffer countBuffer,
                                                                     VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                     uint32_t stride) const {
    return ValidateCmdDrawIndirectCount(commandBuffer, offset, countBufferOffset, CMD_DRAWINDIRECTCOUNT);
}

bool StatelessValidation::manual_PreCallValidateCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                        VkDeviceSize offset, VkBuffer countBuffer,
                                                                        VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                        uint32_t stride) const {
    return ValidateCmdDrawIndirectCount(commandBuffer, offset, countBufferOffset, CMD_DRAWINDIRECTCOUNTAMD);
}

bool StatelessValidation::manual_PreCallValidateCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                        VkDeviceSize offset, VkBuffer countBuffer,
                                                                        VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                        uint32_t stride) const {
    return ValidateCmdDrawIndirectCount(commandBuffer, offset, countBufferOffset, CMD_DRAWINDIRECTCOUNTKHR);
}

bool StatelessValidation::ValidateCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkDeviceSize offset,
                                                              VkDeviceSize countBufferOffset, CMD_TYPE cmd_type) const {
    bool skip = false;
    if (offset & 3) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDrawIndexedIndirectCount-offset-02710",
                         "%s: parameter, VkDeviceSize offset (0x%" PRIxLEAST64 "), is not a multiple of 4.",
                         CommandTypeString(cmd_type), offset);
    }

    if (countBufferOffset & 3) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDrawIndexedIndirectCount-countBufferOffset-02716",
                         "%s: parameter, VkDeviceSize countBufferOffset (0x%" PRIxLEAST64 "), is not a multiple of 4.",
                         CommandTypeString(cmd_type), countBufferOffset);
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                            VkDeviceSize offset, VkBuffer countBuffer,
                                                                            VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                            uint32_t stride) const {
    return ValidateCmdDrawIndexedIndirectCount(commandBuffer, offset, countBufferOffset, CMD_DRAWINDEXEDINDIRECTCOUNT);
}

bool StatelessValidation::manual_PreCallValidateCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                               VkDeviceSize offset, VkBuffer countBuffer,
                                                                               VkDeviceSize countBufferOffset,
                                                                               uint32_t maxDrawCount, uint32_t stride) const {
    return ValidateCmdDrawIndexedIndirectCount(commandBuffer, offset, countBufferOffset, CMD_DRAWINDEXEDINDIRECTCOUNTAMD);
}

bool StatelessValidation::manual_PreCallValidateCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                               VkDeviceSize offset, VkBuffer countBuffer,
                                                                               VkDeviceSize countBufferOffset,
                                                                               uint32_t maxDrawCount, uint32_t stride) const {
    return ValidateCmdDrawIndexedIndirectCount(commandBuffer, offset, countBufferOffset, CMD_DRAWINDEXEDINDIRECTCOUNTKHR);
}

bool StatelessValidation::manual_PreCallValidateCmdDrawMultiEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                                                const VkMultiDrawInfoEXT *pVertexInfo, uint32_t instanceCount,
                                                                uint32_t firstInstance, uint32_t stride) const {
    bool skip = false;
    if (stride & 3) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDrawMultiEXT-stride-04936",
                         "CmdDrawMultiEXT: parameter, uint32_t stride (%" PRIu32 ") is not a multiple of 4.", stride);
    }
    if (drawCount && nullptr == pVertexInfo) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDrawMultiEXT-drawCount-04935",
                         "CmdDrawMultiEXT: parameter, VkMultiDrawInfoEXT *pVertexInfo must be a valid pointer to memory containing "
                         "one or more valid instances of VkMultiDrawInfoEXT structures");
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                                                       const VkMultiDrawIndexedInfoEXT *pIndexInfo,
                                                                       uint32_t instanceCount, uint32_t firstInstance,
                                                                       uint32_t stride, const int32_t *pVertexOffset) const {
    bool skip = false;
    if (stride & 3) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDrawMultiIndexedEXT-stride-04941",
                         "CmdDrawMultiIndexedEXT: parameter, uint32_t stride (%" PRIu32 ") is not a multiple of 4.", stride);
    }
    if (drawCount && nullptr == pIndexInfo) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDrawMultiIndexedEXT-drawCount-04940",
                         "CmdDrawMultiIndexedEXT: parameter, VkMultiDrawIndexedInfoEXT *pIndexInfo must be a valid pointer to "
                         "memory containing one or more valid instances of VkMultiDrawIndexedInfoEXT structures");
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                                                    const VkClearAttachment *pAttachments, uint32_t rectCount,
                                                                    const VkClearRect *pRects) const {
    bool skip = false;
    for (uint32_t rect = 0; rect < rectCount; rect++) {
        if (pRects[rect].layerCount == 0) {
            skip |= LogError(commandBuffer, "VUID-vkCmdClearAttachments-layerCount-01934",
                             "CmdClearAttachments(): pRects[%" PRIu32 "].layerCount is zero.", rect);
        }
        if (pRects[rect].rect.extent.width == 0) {
            skip |= LogError(commandBuffer, "VUID-vkCmdClearAttachments-rect-02682",
                             "CmdClearAttachments(): pRects[%" PRIu32 "].rect.extent.width is zero.", rect);
        }
        if (pRects[rect].rect.extent.height == 0) {
            skip |= LogError(commandBuffer, "VUID-vkCmdClearAttachments-rect-02683",
                             "CmdClearAttachments(): pRects[%" PRIu32 "].rect.extent.height is zero.", rect);
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

bool StatelessValidation::manual_PreCallValidateCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer,
                                                              uint32_t regionCount, const VkBufferCopy *pRegions) const {
    bool skip = false;

    if (pRegions != nullptr) {
        for (uint32_t i = 0; i < regionCount; i++) {
            if (pRegions[i].size == 0) {
                skip |= LogError(device, "VUID-VkBufferCopy-size-01988",
                                 "vkCmdCopyBuffer() pRegions[%" PRIu32 "].size must be greater than zero", i);
            }
        }
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer,
                                                                  const VkCopyBufferInfo2KHR *pCopyBufferInfo) const {
    bool skip = false;

    if (pCopyBufferInfo->pRegions != nullptr) {
        for (uint32_t i = 0; i < pCopyBufferInfo->regionCount; i++) {
            if (pCopyBufferInfo->pRegions[i].size == 0) {
                skip |= LogError(device, "VUID-VkBufferCopy2-size-01988",
                                 "vkCmdCopyBuffer2KHR() pCopyBufferInfo->pRegions[%" PRIu32 "].size must be greater than zero", i);
            }
        }
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdCopyBuffer2(VkCommandBuffer commandBuffer,
                                                               const VkCopyBufferInfo2 *pCopyBufferInfo) const {
    bool skip = false;

    if (pCopyBufferInfo->pRegions != nullptr) {
        for (uint32_t i = 0; i < pCopyBufferInfo->regionCount; i++) {
            if (pCopyBufferInfo->pRegions[i].size == 0) {
                skip |= LogError(device, "VUID-VkBufferCopy2-size-01988",
                                 "vkCmdCopyBuffer2() pCopyBufferInfo->pRegions[%" PRIu32 "].size must be greater than zero", i);
            }
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

bool StatelessValidation::ValidateSwapchainCreateInfo(const char *func_name, VkSwapchainCreateInfoKHR const *pCreateInfo) const {
    bool skip = false;

    if (pCreateInfo != nullptr) {
        // Validation for parameters excluded from the generated validation code due to a 'noautovalidity' tag in vk.xml
        if (pCreateInfo->imageSharingMode == VK_SHARING_MODE_CONCURRENT) {
            // If imageSharingMode is VK_SHARING_MODE_CONCURRENT, queueFamilyIndexCount must be greater than 1
            if (pCreateInfo->queueFamilyIndexCount <= 1) {
                skip |= LogError(device, "VUID-VkSwapchainCreateInfoKHR-imageSharingMode-01278",
                                 "%s: if pCreateInfo->imageSharingMode is VK_SHARING_MODE_CONCURRENT, "
                                 "pCreateInfo->queueFamilyIndexCount must be greater than 1.",
                                 func_name);
            }

            // If imageSharingMode is VK_SHARING_MODE_CONCURRENT, pQueueFamilyIndices must be a pointer to an array of
            // queueFamilyIndexCount uint32_t values
            if (pCreateInfo->pQueueFamilyIndices == nullptr) {
                skip |= LogError(device, "VUID-VkSwapchainCreateInfoKHR-imageSharingMode-01277",
                                 "%s: if pCreateInfo->imageSharingMode is VK_SHARING_MODE_CONCURRENT, "
                                 "pCreateInfo->pQueueFamilyIndices must be a pointer to an array of "
                                 "pCreateInfo->queueFamilyIndexCount uint32_t values.",
                                 func_name);
            }
        }

        skip |= ValidateGreaterThanZero(pCreateInfo->imageArrayLayers, "pCreateInfo->imageArrayLayers",
                                        "VUID-VkSwapchainCreateInfoKHR-imageArrayLayers-01275", func_name);

        // Validate VK_KHR_image_format_list VkImageFormatListCreateInfo
        const auto format_list_info = LvlFindInChain<VkImageFormatListCreateInfo>(pCreateInfo->pNext);
        if (format_list_info) {
            const uint32_t viewFormatCount = format_list_info->viewFormatCount;
            if (((pCreateInfo->flags & VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR) == 0) && (viewFormatCount > 1)) {
                skip |= LogError(device, "VUID-VkSwapchainCreateInfoKHR-flags-04100",
                                 "%s: If the VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR is not set, then "
                                 "VkImageFormatListCreateInfo::viewFormatCount (%" PRIu32
                                 ") must be 0 or 1 if it is in the pNext chain.",
                                 func_name, viewFormatCount);
            }

            // Using the first format, compare the rest of the formats against it that they are compatible
            for (uint32_t i = 1; i < viewFormatCount; i++) {
                if (FormatCompatibilityClass(format_list_info->pViewFormats[0]) !=
                    FormatCompatibilityClass(format_list_info->pViewFormats[i])) {
                    skip |= LogError(device, "VUID-VkSwapchainCreateInfoKHR-pNext-04099",
                                     "%s: VkImageFormatListCreateInfo::pViewFormats[0] (%s) and "
                                     "VkImageFormatListCreateInfo::pViewFormats[%" PRIu32
                                     "] (%s) are not compatible in the pNext chain.",
                                     func_name, string_VkFormat(format_list_info->pViewFormats[0]), i,
                                     string_VkFormat(format_list_info->pViewFormats[i]));
                }
            }
        }

        // Validate VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR
        if ((pCreateInfo->flags & VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR) != 0) {
            if (!IsExtEnabled(device_extensions.vk_khr_swapchain_mutable_format)) {
                skip |= LogError(device, kVUID_PVError_ExtensionNotEnabled,
                                 "%s: pCreateInfo->flags contains VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR which requires the "
                                 "VK_KHR_swapchain_mutable_format extension, which has not been enabled.",
                                 func_name);
            } else {
                if (format_list_info == nullptr) {
                    skip |= LogError(
                        device, "VUID-VkSwapchainCreateInfoKHR-flags-03168",
                        "%s: pCreateInfo->flags contains VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR but the pNext chain of "
                        "pCreateInfo does not contain an instance of VkImageFormatListCreateInfo.",
                        func_name);
                } else if (format_list_info->viewFormatCount == 0) {
                    skip |= LogError(
                        device, "VUID-VkSwapchainCreateInfoKHR-flags-03168",
                        "%s: pCreateInfo->flags contains VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR but the viewFormatCount "
                        "member of VkImageFormatListCreateInfo in the pNext chain is zero.",
                        func_name);
                } else {
                    bool found_base_format = false;
                    for (uint32_t i = 0; i < format_list_info->viewFormatCount; ++i) {
                        if (format_list_info->pViewFormats[i] == pCreateInfo->imageFormat) {
                            found_base_format = true;
                            break;
                        }
                    }
                    if (!found_base_format) {
                        skip |=
                            LogError(device, "VUID-VkSwapchainCreateInfoKHR-flags-03168",
                                     "%s: pCreateInfo->flags contains VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR but none of the "
                                     "elements of the pViewFormats member of VkImageFormatListCreateInfo match "
                                     "pCreateInfo->imageFormat.",
                                     func_name);
                    }
                }
            }
        }
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR *pCreateInfo,
                                                                   const VkAllocationCallbacks *pAllocator,
                                                                   VkSwapchainKHR *pSwapchain) const {
    bool skip = false;
    skip |= ValidateSwapchainCreateInfo("vkCreateSwapchainKHR()", pCreateInfo);
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount,
                                                                          const VkSwapchainCreateInfoKHR *pCreateInfos,
                                                                          const VkAllocationCallbacks *pAllocator,
                                                                          VkSwapchainKHR *pSwapchains) const {
    bool skip = false;
    if (pCreateInfos) {
        for (uint32_t i = 0; i < swapchainCount; i++) {
            std::stringstream func_name;
            func_name << "vkCreateSharedSwapchainsKHR[" << swapchainCount << "]()";
            skip |= ValidateSwapchainCreateInfo(func_name.str().c_str(), &pCreateInfos[i]);
        }
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR *pPresentInfo) const {
    bool skip = false;

    if (pPresentInfo && pPresentInfo->pNext) {
        const auto *present_regions = LvlFindInChain<VkPresentRegionsKHR>(pPresentInfo->pNext);
        if (present_regions) {
            // TODO: This and all other pNext extension dependencies should be added to code-generation
            skip |= RequireDeviceExtension(IsExtEnabled(device_extensions.vk_khr_incremental_present), "vkQueuePresentKHR",
                                           VK_KHR_INCREMENTAL_PRESENT_EXTENSION_NAME);
            if (present_regions->swapchainCount != pPresentInfo->swapchainCount) {
                skip |= LogError(device, "VUID-VkPresentRegionsKHR-swapchainCount-01260",
                                 "QueuePresentKHR(): pPresentInfo->swapchainCount has a value of %i but VkPresentRegionsKHR "
                                 "extension swapchainCount is %i. These values must be equal.",
                                 pPresentInfo->swapchainCount, present_regions->swapchainCount);
            }
            skip |= ValidateStructPnext("QueuePresentKHR", "pCreateInfo->pNext->pNext", NULL, present_regions->pNext, 0, NULL,
                                        GeneratedVulkanHeaderVersion, "VUID-VkPresentInfoKHR-pNext-pNext",
                                        "VUID-VkPresentInfoKHR-sType-unique");
            skip |= ValidateArray("QueuePresentKHR", "pCreateInfo->pNext->swapchainCount", "pCreateInfo->pNext->pRegions",
                                  present_regions->swapchainCount, &present_regions->pRegions, true, false, kVUIDUndefined,
                                  kVUIDUndefined);
            for (uint32_t i = 0; i < present_regions->swapchainCount; ++i) {
                skip |= ValidateArray("QueuePresentKHR", "pCreateInfo->pNext->pRegions[].rectangleCount",
                                      "pCreateInfo->pNext->pRegions[].pRectangles", present_regions->pRegions[i].rectangleCount,
                                      &present_regions->pRegions[i].pRectangles, true, false, kVUIDUndefined, kVUIDUndefined);
            }
        }
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCreateDisplayModeKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                                     const VkDisplayModeCreateInfoKHR *pCreateInfo,
                                                                     const VkAllocationCallbacks *pAllocator,
                                                                     VkDisplayModeKHR *pMode) const {
    bool skip = false;

    const VkDisplayModeParametersKHR display_mode_parameters = pCreateInfo->parameters;
    if (display_mode_parameters.visibleRegion.width == 0) {
        skip |= LogError(device, "VUID-VkDisplayModeParametersKHR-width-01990",
                         "vkCreateDisplayModeKHR(): pCreateInfo->parameters.visibleRegion.width must be greater than 0.");
    }
    if (display_mode_parameters.visibleRegion.height == 0) {
        skip |= LogError(device, "VUID-VkDisplayModeParametersKHR-height-01991",
                         "vkCreateDisplayModeKHR(): pCreateInfo->parameters.visibleRegion.height must be greater than 0.");
    }
    if (display_mode_parameters.refreshRate == 0) {
        skip |= LogError(device, "VUID-VkDisplayModeParametersKHR-refreshRate-01992",
                         "vkCreateDisplayModeKHR(): pCreateInfo->parameters.refreshRate must be greater than 0.");
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

#ifdef VK_USE_PLATFORM_WAYLAND_KHR

bool StatelessValidation::manual_PreCallValidateCreateWaylandSurfaceKHR(VkInstance instance,
                                                                        const VkWaylandSurfaceCreateInfoKHR *pCreateInfo,
                                                                        const VkAllocationCallbacks *pAllocator,
                                                                        VkSurfaceKHR *pSurface) const {
    bool skip = false;

    const auto display = pCreateInfo->display;
    const auto surface = pCreateInfo->surface;

    // Beyond checking for nullptr not much we can do.
    // EX: Calling wl_display_get_registry on an invalid display will crash the application.
    if (display == nullptr) {
        skip |= LogError(device, "VUID-VkWaylandSurfaceCreateInfoKHR-display-01304",
                         "vkCreateWaylandSurfaceKHR: display must be valid Wayland wl_display but display is NULL!");
    }

    // Beyond checking for nullptr not much we can do.
    // EX: Calling wl_surface_get_version on an invalid surface will crash the application.
    if (surface == nullptr) {
        skip |= LogError(device, "VUID-VkWaylandSurfaceCreateInfoKHR-surface-01305",
                         "vkCreateWaylandSurfaceKHR: surface must be valid Wayland wl_surface but surface is NULL!");
    }

    return skip;
}

#endif  // VK_USE_PLATFORM_WAYLAND_KHR

static bool MutableDescriptorTypePartialOverlap(const VkDescriptorPoolCreateInfo *pCreateInfo, uint32_t i, uint32_t j) {
    bool partial_overlap = false;

    constexpr std::array all_descriptor_types = {
        VK_DESCRIPTOR_TYPE_SAMPLER,
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
        VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
        VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
        VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT,
        VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
        VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV,
    };

    const auto *mutable_descriptor_type = LvlFindInChain<VkMutableDescriptorTypeCreateInfoEXT>(pCreateInfo->pNext);
    if (mutable_descriptor_type) {
        vvl::span<const VkDescriptorType> first_types, second_types;

        if (mutable_descriptor_type->mutableDescriptorTypeListCount > i) {
            const uint32_t descriptorTypeCount = mutable_descriptor_type->pMutableDescriptorTypeLists[i].descriptorTypeCount;
            auto *pDescriptorTypes = mutable_descriptor_type->pMutableDescriptorTypeLists[i].pDescriptorTypes;
            first_types = vvl::make_span(pDescriptorTypes, descriptorTypeCount);
        } else {
            first_types = vvl::make_span(all_descriptor_types.data(), all_descriptor_types.size());
        }

        if (mutable_descriptor_type->mutableDescriptorTypeListCount > j) {
            const uint32_t descriptorTypeCount = mutable_descriptor_type->pMutableDescriptorTypeLists[j].descriptorTypeCount;
            auto *pDescriptorTypes = mutable_descriptor_type->pMutableDescriptorTypeLists[j].pDescriptorTypes;
            second_types = vvl::make_span(pDescriptorTypes, descriptorTypeCount);
        } else {
            second_types = vvl::make_span(all_descriptor_types.data(), all_descriptor_types.size());
        }

        bool complete_overlap = first_types.size() == second_types.size();
        bool disjoint = true;
        for (const auto first_type : first_types) {
            bool found = false;
            for (const auto second_type : second_types) {
                if (first_type == second_type) {
                    found = true;
                    break;
                }
            }
            if (found) {
                disjoint = false;
            } else {
                complete_overlap = false;
            }
            if (!disjoint && !complete_overlap) {
                partial_overlap = true;
                break;
            }
        }
    }

    return partial_overlap;
}

bool StatelessValidation::manual_PreCallValidateCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo *pCreateInfo,
                                                                     const VkAllocationCallbacks *pAllocator,
                                                                     VkDescriptorPool *pDescriptorPool) const {
    bool skip = false;

    if (pCreateInfo) {
        if (pCreateInfo->maxSets <= 0) {
            skip |= LogError(device, "VUID-VkDescriptorPoolCreateInfo-maxSets-00301",
                             "vkCreateDescriptorPool(): pCreateInfo->maxSets is not greater than 0.");
        }

        const auto *mutable_descriptor_type_features =
            LvlFindInChain<VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT>(device_createinfo_pnext);
        bool mutable_descriptor_type_enabled =
            mutable_descriptor_type_features && mutable_descriptor_type_features->mutableDescriptorType == VK_TRUE;

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
                if (pCreateInfo->pPoolSizes[i].type == VK_DESCRIPTOR_TYPE_MUTABLE_EXT && !mutable_descriptor_type_enabled) {
                    skip |=
                        LogError(device, "VUID-VkDescriptorPoolCreateInfo-mutableDescriptorType-04608",
                                 "vkCreateDescriptorPool(): pCreateInfo->pPoolSizes[%" PRIu32
                                 "].type is VK_DESCRIPTOR_TYPE_MUTABLE_EXT "
                                 ", but VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT::mutableDescriptorType is not enabled.",
                                 i);
                }
                if (pCreateInfo->pPoolSizes[i].type == VK_DESCRIPTOR_TYPE_MUTABLE_EXT) {
                    for (uint32_t j = i + 1; j < pCreateInfo->poolSizeCount; ++j) {
                        if (pCreateInfo->pPoolSizes[j].type == VK_DESCRIPTOR_TYPE_MUTABLE_EXT) {
                            if (MutableDescriptorTypePartialOverlap(pCreateInfo, i, j)) {
                                skip |= LogError(device, "VUID-VkDescriptorPoolCreateInfo-pPoolSizes-04787",
                                                 "vkCreateDescriptorPool(): pCreateInfo->pPoolSizes[%" PRIu32
                                                 "].type and pCreateInfo->pPoolSizes[%" PRIu32
                                                 "].type are both VK_DESCRIPTOR_TYPE_MUTABLE_EXT "
                                                 " and have sets which partially overlap.",
                                                 i, j);
                            }
                        }
                    }
                }
            }
        }

        if (pCreateInfo->flags & VK_DESCRIPTOR_POOL_CREATE_HOST_ONLY_BIT_EXT && (!mutable_descriptor_type_enabled)) {
            skip |= LogError(device, "VUID-VkDescriptorPoolCreateInfo-flags-04609",
                             "vkCreateDescriptorPool(): pCreateInfo->flags contains VK_DESCRIPTOR_POOL_CREATE_HOST_ONLY_BIT_EXT, "
                             "but VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT::mutableDescriptorType is not enabled.");
        }
        if ((pCreateInfo->flags & VK_DESCRIPTOR_POOL_CREATE_HOST_ONLY_BIT_EXT) &&
            (pCreateInfo->flags & VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT)) {
            skip |= LogError(device, "VUID-VkDescriptorPoolCreateInfo-flags-04607",
                             "vkCreateDescriptorPool(): pCreateInfo->flags must not contain both "
                             "VK_DESCRIPTOR_POOL_CREATE_HOST_ONLY_BIT_EXT and VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT");
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

    if (offset & 3) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDispatchIndirect-offset-02710",
                         "vkCmdDispatchIndirect(): offset (%" PRIxLEAST64 ") must be a multiple of 4.", offset);
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
    return ValidateWriteDescriptorSet("vkCmdPushDescriptorSetKHR", descriptorWriteCount, pDescriptorWrites, true);
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
            if (x_sum > vvl::kI32Max) {
                skip |= LogError(commandBuffer, "VUID-vkCmdSetExclusiveScissorNV-offset-02038",
                                 "vkCmdSetExclusiveScissorNV: offset.x + extent.width (=%" PRIi32 " + %" PRIu32 " = %" PRIi64
                                 ") of pScissors[%" PRIu32 "] will overflow int32_t.",
                                 scissor.offset.x, scissor.extent.width, x_sum, scissor_i);
            }

            const int64_t y_sum = static_cast<int64_t>(scissor.offset.y) + static_cast<int64_t>(scissor.extent.height);
            if (y_sum > vvl::kI32Max) {
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
    const uint64_t sum = static_cast<uint64_t>(firstViewport) + static_cast<uint64_t>(viewportCount);
    if ((sum < 1) || (sum > device_limits.maxViewports)) {
        skip |= LogError(commandBuffer, "VUID-vkCmdSetViewportWScalingNV-firstViewport-01324",
                         "vkCmdSetViewportWScalingNV: firstViewport + viewportCount (=%" PRIu32 " + %" PRIu32 " = %" PRIu64
                         ") must be between 1 and VkPhysicalDeviceLimits::maxViewports (=%" PRIu32 "), inculsive.",
                         firstViewport, viewportCount, sum, device_limits.maxViewports);
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

    if (taskCount > phys_dev_ext_props.mesh_shader_props_nv.maxDrawMeshTasksCount) {
        skip |= LogError(
            commandBuffer, "VUID-vkCmdDrawMeshTasksNV-taskCount-02119",
            "vkCmdDrawMeshTasksNV() parameter, uint32_t taskCount (0x%" PRIxLEAST32
            "), must be less than or equal to VkPhysicalDeviceMeshShaderPropertiesNV::maxDrawMeshTasksCount (0x%" PRIxLEAST32 ").",
            taskCount, phys_dev_ext_props.mesh_shader_props_nv.maxDrawMeshTasksCount);
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                           VkDeviceSize offset, uint32_t drawCount,
                                                                           uint32_t stride) const {
    bool skip = false;
    if (offset & 3) {
        skip |= LogError(
            commandBuffer, "VUID-vkCmdDrawMeshTasksIndirectNV-offset-02710",
            "vkCmdDrawMeshTasksIndirectNV() parameter, VkDeviceSize offset (0x%" PRIxLEAST64 "), is not a multiple of 4.", offset);
    }
    if (drawCount > 1 && ((stride & 3) || stride < sizeof(VkDrawMeshTasksIndirectCommandNV))) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDrawMeshTasksIndirectNV-drawCount-02146",
                         "vkCmdDrawMeshTasksIndirectNV() parameter, uint32_t stride (0x%" PRIxLEAST32
                         "), is not a multiple of 4 or smaller than sizeof (VkDrawMeshTasksIndirectCommandNV).",
                         stride);
    }
    if (!physical_device_features.multiDrawIndirect && ((drawCount > 1))) {
        skip |= LogError(
            commandBuffer, "VUID-vkCmdDrawMeshTasksIndirectNV-drawCount-02718",
            "vkCmdDrawMeshTasksIndirectNV(): Device feature multiDrawIndirect disabled: count must be 0 or 1 but is %" PRIu32 "",
            drawCount);
    }
    if (drawCount > device_limits.maxDrawIndirectCount) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDrawMeshTasksIndirectNV-drawCount-02719",
                         "vkCmdDrawMeshTasksIndirectNV: drawCount (%" PRIu32
                         ") is not less than or equal to the maximum allowed (%" PRIu32 ").",
                         drawCount, device_limits.maxDrawIndirectCount);
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

bool StatelessValidation::manual_PreCallValidateCmdDrawMeshTasksEXT(VkCommandBuffer commandBuffer, uint32_t groupCountX,
                                                                    uint32_t groupCountY, uint32_t groupCountZ) const {
    bool skip = false;

    if (groupCountX > phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupCount[0]) {
        skip |= LogError(
            commandBuffer, "VUID-vkCmdDrawMeshTasksEXT-TaskEXT-07322",
            "vkCmdDrawMeshTasksEXT() parameter, uint32_t groupCountX (0x%" PRIxLEAST32
            "), must be less than or equal to VkPhysicalDeviceMeshShaderPropertiesEXT::maxTaskWorkGroupCount[0] (0x%" PRIxLEAST32
            ").",
            groupCountX, phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupCount[0]);
    }
    if (groupCountY > phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupCount[1]) {
        skip |= LogError(
            commandBuffer, "VUID-vkCmdDrawMeshTasksEXT-TaskEXT-07323",
            "vkCmdDrawMeshTasksEXT() parameter, uint32_t groupCountY (0x%" PRIxLEAST32
            "), must be less than or equal to VkPhysicalDeviceMeshShaderPropertiesEXT::maxTaskWorkGroupCount[1] (0x%" PRIxLEAST32
            ").",
            groupCountY, phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupCount[1]);
    }
    if (groupCountZ > phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupCount[2]) {
        skip |= LogError(
            commandBuffer, "VUID-vkCmdDrawMeshTasksEXT-TaskEXT-07324",
            "vkCmdDrawMeshTasksEXT() parameter, uint32_t groupCountZ (0x%" PRIxLEAST32
            "), must be less than or equal to VkPhysicalDeviceMeshShaderPropertiesEXT::maxTaskWorkGroupCount[2] (0x%" PRIxLEAST32
            ").",
            groupCountZ, phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupCount[2]);
    }

    uint32_t maxTaskWorkGroupTotalCount = phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupTotalCount;
    uint64_t invocations = static_cast<uint64_t>(groupCountX) * static_cast<uint64_t>(groupCountY);
    // Prevent overflow.
    bool fail = false;
    if (invocations > vvl::MaxTypeValue(maxTaskWorkGroupTotalCount) || invocations > maxTaskWorkGroupTotalCount) {
        fail = true;
    }
    if (!fail) {
        invocations *= static_cast<uint64_t>(groupCountZ);
        if (invocations > vvl::MaxTypeValue(maxTaskWorkGroupTotalCount) || invocations > maxTaskWorkGroupTotalCount) {
            fail = true;
        }
    }
    if (fail) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDrawMeshTasksEXT-TaskEXT-07325",
                         "vkCmdDrawMeshTasksEXT(): The product of groupCountX (0x%" PRIxLEAST32 "), groupCountY (0x%" PRIxLEAST32
                         ") and groupCountZ (0x%" PRIxLEAST32
                         ") must be less than or equal to "
                         "VkPhysicalDeviceMeshShaderPropertiesEXT::maxTaskWorkGroupTotalCount (0x%" PRIxLEAST32 ").",
                         groupCountX, groupCountY, groupCountZ, maxTaskWorkGroupTotalCount);
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdDrawMeshTasksIndirectEXT(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                            VkDeviceSize offset, uint32_t drawCount,
                                                                            uint32_t stride) const {
    bool skip = false;

    // TODO: vkMapMemory() and check the contents of buffer at offset
    // issue #4547 (https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/4547)
    if (!physical_device_features.multiDrawIndirect && ((drawCount > 1))) {
        skip |= LogError(
            commandBuffer, "VUID-vkCmdDrawMeshTasksIndirectEXT-drawCount-02718",
            "vkCmdDrawMeshTasksIndirectEXT(): Device feature multiDrawIndirect disabled: count must be 0 or 1 but is %" PRIu32 "",
            drawCount);
    }
    if (drawCount > device_limits.maxDrawIndirectCount) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDrawMeshTasksIndirectEXT-drawCount-02719",
                         "vkCmdDrawMeshTasksIndirectEXT: drawCount (%" PRIu32
                         ") is not less than or equal to the maximum allowed (%" PRIu32 ").",
                         drawCount, device_limits.maxDrawIndirectCount);
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
    return ValidateArray("vkEnumerateDeviceExtensionProperties", "pPropertyCount", "pProperties", pPropertyCount, &pProperties,
                         true, false, false, kVUIDUndefined, "VUID-vkEnumerateDeviceExtensionProperties-pProperties-parameter");
}

void StatelessValidation::PostCallRecordCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo *pCreateInfo,
                                                         const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass,
                                                         VkResult result) {
    if (result != VK_SUCCESS) return;
    RecordRenderPass(*pRenderPass, pCreateInfo);
}

void StatelessValidation::PostCallRecordCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2 *pCreateInfo,
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

void StatelessValidation::PostCallRecordAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo *pAllocateInfo,
                                                               VkCommandBuffer *pCommandBuffers, VkResult result) {
    if ((result == VK_SUCCESS) && pAllocateInfo && (pAllocateInfo->level == VK_COMMAND_BUFFER_LEVEL_SECONDARY)) {
        auto lock = CBWriteLock();
        for (uint32_t cb_index = 0; cb_index < pAllocateInfo->commandBufferCount; cb_index++) {
            secondary_cb_map.emplace(pCommandBuffers[cb_index], pAllocateInfo->commandPool);
        }
    }
}

void StatelessValidation::PostCallRecordFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount,
                                                           const VkCommandBuffer *pCommandBuffers) {
    auto lock = CBWriteLock();
    for (uint32_t cb_index = 0; cb_index < commandBufferCount; cb_index++) {
        secondary_cb_map.erase(pCommandBuffers[cb_index]);
    }
}

void StatelessValidation::PostCallRecordDestroyCommandPool(VkDevice device, VkCommandPool commandPool,
                                                           const VkAllocationCallbacks *pAllocator) {
    auto lock = CBWriteLock();
    for (auto item = secondary_cb_map.begin(); item != secondary_cb_map.end();) {
        if (item->second == commandPool) {
            item = secondary_cb_map.erase(item);
        } else {
            ++item;
        }
    }
}

bool StatelessValidation::manual_PreCallValidateAllocateMemory(VkDevice device, const VkMemoryAllocateInfo *pAllocateInfo,
                                                               const VkAllocationCallbacks *pAllocator,
                                                               VkDeviceMemory *pMemory) const {
    bool skip = false;

    if (pAllocateInfo) {
        auto chained_prio_struct = LvlFindInChain<VkMemoryPriorityAllocateInfoEXT>(pAllocateInfo->pNext);
        if (chained_prio_struct && (chained_prio_struct->priority < 0.0f || chained_prio_struct->priority > 1.0f)) {
            skip |= LogError(device, "VUID-VkMemoryPriorityAllocateInfoEXT-priority-02602",
                             "priority (=%f) must be between `0` and `1`, inclusive.", chained_prio_struct->priority);
        }

        VkMemoryAllocateFlags flags = 0;
        auto flags_info = LvlFindInChain<VkMemoryAllocateFlagsInfo>(pAllocateInfo->pNext);
        if (flags_info) {
            flags = flags_info->flags;
        }

        const ImportOperationsInfo import_info = GetNumberOfImportInfo(pAllocateInfo);

        auto opaque_alloc_info = LvlFindInChain<VkMemoryOpaqueCaptureAddressAllocateInfo>(pAllocateInfo->pNext);
        if (opaque_alloc_info && opaque_alloc_info->opaqueCaptureAddress != 0) {
            if (!(flags & VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT)) {
                skip |= LogError(device, "VUID-VkMemoryAllocateInfo-opaqueCaptureAddress-03329",
                                 "If opaqueCaptureAddress is non-zero, VkMemoryAllocateFlagsInfo::flags must include "
                                 "VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT.");
            }

            if (import_info.host_pointer_info_ext) {
                skip |= LogError(
                    device, "VUID-VkMemoryAllocateInfo-pNext-03332",
                    "If the pNext chain includes a VkImportMemoryHostPointerInfoEXT structure, opaqueCaptureAddress must be zero.");
            }

            if (import_info.total_import_ops > 0) {
                skip |= LogError(device, "VUID-VkMemoryAllocateInfo-opaqueCaptureAddress-03333",
                                 "If the parameters define an import operation, opaqueCaptureAddress must be zero.");
            }
        }

        if (import_info.total_import_ops > 1) {
            skip |=
                LogError(device, "VUID-VkMemoryAllocateInfo-None-06657",
                         "The parameters must not define more than 1 import operation. User defined %" PRIu32 " import operations",
                         import_info.total_import_ops);
        }

        auto export_memory = LvlFindInChain<VkExportMemoryAllocateInfo>(pAllocateInfo->pNext);
        if (export_memory) {
            auto export_memory_nv = LvlFindInChain<VkExportMemoryAllocateInfoNV>(pAllocateInfo->pNext);
            if (export_memory_nv) {
                skip |= LogError(device, "VUID-VkMemoryAllocateInfo-pNext-00640",
                                 "pNext chain of VkMemoryAllocateInfo includes both VkExportMemoryAllocateInfo and "
                                 "VkExportMemoryAllocateInfoNV");
            }
#ifdef VK_USE_PLATFORM_WIN32_KHR
            auto export_memory_win32_nv = LvlFindInChain<VkExportMemoryWin32HandleInfoNV>(pAllocateInfo->pNext);
            if (export_memory_win32_nv) {
                skip |= LogError(device, "VUID-VkMemoryAllocateInfo-pNext-00640",
                                 "pNext chain of VkMemoryAllocateInfo includes both VkExportMemoryAllocateInfo and "
                                 "VkExportMemoryWin32HandleInfoNV");
            }
#endif
        }

        if (flags) {
            VkBool32 capture_replay = false;
            VkBool32 buffer_device_address = false;
            const auto *vulkan_12_features = LvlFindInChain<VkPhysicalDeviceVulkan12Features>(device_createinfo_pnext);
            if (vulkan_12_features) {
                capture_replay = vulkan_12_features->bufferDeviceAddressCaptureReplay;
                buffer_device_address = vulkan_12_features->bufferDeviceAddress;
            } else {
                const auto *bda_features = LvlFindInChain<VkPhysicalDeviceBufferDeviceAddressFeatures>(device_createinfo_pnext);
                if (bda_features) {
                    capture_replay = bda_features->bufferDeviceAddressCaptureReplay;
                    buffer_device_address = bda_features->bufferDeviceAddress;
                }
            }
            if ((flags & VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT) && !capture_replay) {
                skip |= LogError(device, "VUID-VkMemoryAllocateInfo-flags-03330",
                                 "If VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT is set, "
                                 "bufferDeviceAddressCaptureReplay must be enabled.");
            }
            if ((flags & VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT) && !buffer_device_address) {
                skip |= LogError(device, "VUID-VkMemoryAllocateInfo-flags-03331",
                                 "If VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT is set, bufferDeviceAddress must be enabled.");
            }
        }
#ifdef VK_USE_PLATFORM_METAL_EXT
        skip |= ExportMetalObjectsPNextUtil(
            VK_EXPORT_METAL_OBJECT_TYPE_METAL_BUFFER_BIT_EXT, "VUID-VkMemoryAllocateInfo-pNext-06780",
            "vkAllocateMemory():", "VK_EXPORT_METAL_OBJECT_TYPE_METAL_BUFFER_BIT_EXT", pAllocateInfo->pNext);
#endif  // VK_USE_PLATFORM_METAL_EXT
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
        const uint32_t index_element_size = GetIndexAlignment(triangles.indexType);
        if (index_element_size > 0 && SafeModulo(triangles.indexOffset, index_element_size) != 0) {
            skip |= LogError(object_handle, "VUID-VkGeometryTrianglesNV-indexOffset-02432", "%s", func_name);
        }

        if (triangles.indexType == VK_INDEX_TYPE_NONE_NV) {
            if (triangles.indexCount != 0) {
                skip |= LogError(object_handle, "VUID-VkGeometryTrianglesNV-indexCount-02436", "%s", func_name);
            }
            if (triangles.indexData != VK_NULL_HANDLE) {
                skip |= LogError(object_handle, "VUID-VkGeometryTrianglesNV-indexData-02434", "%s", func_name);
            }
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
    if (info.type == VK_ACCELERATION_STRUCTURE_TYPE_GENERIC_KHR) {
        skip |= LogError(object_handle, "VUID-VkAccelerationStructureInfoNV-type-04623",
                         "VkAccelerationStructureInfoNV: type is invalid VK_ACCELERATION_STRUCTURE_TYPE_GENERIC_KHR.");
    }
    if (info.flags & VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_NV &&
        info.flags & VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_NV) {
        skip |= LogError(object_handle, "VUID-VkAccelerationStructureInfoNV-flags-02592",
                         "VkAccelerationStructureInfoNV: If flags has the VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_NV"
                         "bit set, then it must not have the VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_NV bit set.");
    }
    if (info.geometryCount > phys_dev_ext_props.ray_tracing_props_nv.maxGeometryCount) {
        skip |= LogError(object_handle,
                         is_cmd ? "VUID-vkCmdBuildAccelerationStructureNV-geometryCount-02241"
                                : "VUID-VkAccelerationStructureInfoNV-geometryCount-02422",
                         "VkAccelerationStructureInfoNV: geometryCount must be less than or equal to "
                         "VkPhysicalDeviceRayTracingPropertiesNV::maxGeometryCount.");
    }
    if (info.instanceCount > phys_dev_ext_props.ray_tracing_props_nv.maxInstanceCount) {
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
        if (total_triangle_count > phys_dev_ext_props.ray_tracing_props_nv.maxTriangleCount) {
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
                                 "VkAccelerationStructureInfoNV: info.pGeometries[%" PRIu32
                                 "].geometryType does not match "
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
        ValidateFlags(func_name, "info.flags", "VkBuildAccelerationStructureFlagBitsNV", AllVkBuildAccelerationStructureFlagBitsNV,
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
    const auto *acceleration_structure_features =
        LvlFindInChain<VkPhysicalDeviceAccelerationStructureFeaturesKHR>(device_createinfo_pnext);
    if (!acceleration_structure_features ||
        (acceleration_structure_features && acceleration_structure_features->accelerationStructure == VK_FALSE)) {
        skip |= LogError(device, "VUID-vkCreateAccelerationStructureKHR-accelerationStructure-03611",
                         "vkCreateAccelerationStructureKHR(): The accelerationStructure feature must be enabled");
    }
    if (pCreateInfo) {
        if (pCreateInfo->createFlags & VK_ACCELERATION_STRUCTURE_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_KHR &&
            (!acceleration_structure_features ||
             (acceleration_structure_features &&
              acceleration_structure_features->accelerationStructureCaptureReplay == VK_FALSE))) {
            skip |=
                LogError(device, "VUID-VkAccelerationStructureCreateInfoKHR-createFlags-03613",
                         "vkCreateAccelerationStructureKHR(): If createFlags includes "
                         "VK_ACCELERATION_STRUCTURE_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_KHR, "
                         "VkPhysicalDeviceAccelerationStructureFeaturesKHR::accelerationStructureCaptureReplay must be VK_TRUE");
        }
        if (pCreateInfo->deviceAddress &&
            !(pCreateInfo->createFlags & VK_ACCELERATION_STRUCTURE_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_KHR)) {
            skip |= LogError(device, "VUID-VkAccelerationStructureCreateInfoKHR-deviceAddress-03612",
                             "vkCreateAccelerationStructureKHR(): If deviceAddress is not zero, createFlags must include "
                             "VK_ACCELERATION_STRUCTURE_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_KHR");
        }
        if (pCreateInfo->deviceAddress && (!acceleration_structure_features ||
                                           (acceleration_structure_features &&
                                            acceleration_structure_features->accelerationStructureCaptureReplay == VK_FALSE))) {
            skip |= LogError(
                device, "VUID-vkCreateAccelerationStructureKHR-deviceAddress-03488",
                "VkAccelerationStructureCreateInfoKHR(): VkAccelerationStructureCreateInfoKHR::deviceAddress is not zero, but "
                "VkPhysicalDeviceAccelerationStructureFeaturesKHR::accelerationStructureCaptureReplay is not enabled.");
        }
        if (SafeModulo(pCreateInfo->offset, 256) != 0) {
            skip |= LogError(device, "VUID-VkAccelerationStructureCreateInfoKHR-offset-03734",
                             "vkCreateAccelerationStructureKHR(): offset %" PRIu64 " must be a multiple of 256 bytes",
                             pCreateInfo->offset);
        }

        const auto *descriptor_buffer_features =
            LvlFindInChain<VkPhysicalDeviceDescriptorBufferFeaturesEXT>(device_createinfo_pnext);
        if ((pCreateInfo->createFlags & VK_ACCELERATION_STRUCTURE_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT) &&
            (!descriptor_buffer_features ||
             (descriptor_buffer_features && descriptor_buffer_features->descriptorBufferCaptureReplay == VK_FALSE))) {
            skip |= LogError(device, "VUID-VkAccelerationStructureCreateInfoKHR-createFlags-08108",
                             "vkCreateAccelerationStructureKHR(): the descriptorBufferCaptureReplay device feature is disabled: "
                             "Acceleration structures cannot be created with "
                             "the VK_ACCELERATION_STRUCTURE_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT.");
        }

        const auto *opaque_capture_descriptor_buffer =
            LvlFindInChain<VkOpaqueCaptureDescriptorDataCreateInfoEXT>(pCreateInfo->pNext);
        if (opaque_capture_descriptor_buffer &&
            !(pCreateInfo->createFlags & VK_ACCELERATION_STRUCTURE_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT)) {
            skip |=
                LogError(device, "VUID-VkAccelerationStructureCreateInfoKHR-pNext-08109",
                         "vkCreateAccelerationStructureKHR(): VkOpaqueCaptureDescriptorDataCreateInfoEXT is in pNext chain, but "
                         "VK_ACCELERATION_STRUCTURE_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT is not set.");
        }
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

bool StatelessValidation::manual_PreCallValidateCmdWriteAccelerationStructuresPropertiesNV(
    VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount, const VkAccelerationStructureNV *pAccelerationStructures,
    VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery) const {
    bool skip = false;
    if (queryType != VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_NV) {
        skip |= LogError(device, "VUID-vkCmdWriteAccelerationStructuresPropertiesNV-queryType-06216",
                         "vkCmdWriteAccelerationStructuresPropertiesNV: queryType must be "
                         "VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_NV.");
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
        for (uint32_t stage_index = 0; stage_index < pCreateInfos[i].stageCount; ++stage_index) {
            std::stringstream msg;
            msg << "pCreateInfos[%" << i << "].pStages[%" << stage_index << "]";
            ValidatePipelineShaderStageCreateInfo("vkCreateRayTracingPipelinesNV", msg.str().c_str(),
                                                  &pCreateInfos[i].pStages[stage_index]);
        }
        auto feedback_struct = LvlFindInChain<VkPipelineCreationFeedbackCreateInfoEXT>(pCreateInfos[i].pNext);
        if ((feedback_struct != nullptr) && (feedback_struct->pipelineStageCreationFeedbackCount != 0) &&
            (feedback_struct->pipelineStageCreationFeedbackCount != pCreateInfos[i].stageCount)) {
            skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoNV-pipelineStageCreationFeedbackCount-06651",
                             "vkCreateRayTracingPipelinesNV(): in pCreateInfo[%" PRIu32
                             "], VkPipelineCreationFeedbackEXT::pipelineStageCreationFeedbackCount"
                             "(=%" PRIu32 ") must equal VkRayTracingPipelineCreateInfoNV::stageCount(=%" PRIu32 ").",
                             i, feedback_struct->pipelineStageCreationFeedbackCount, pCreateInfos[i].stageCount);
        }

        const auto *vulkan_13_features = LvlFindInChain<VkPhysicalDeviceVulkan13Features>(device_createinfo_pnext);
        const auto *pipeline_cache_contol_features =
            LvlFindInChain<VkPhysicalDevicePipelineCreationCacheControlFeaturesEXT>(device_createinfo_pnext);
        if ((!vulkan_13_features || vulkan_13_features->pipelineCreationCacheControl == VK_FALSE) &&
            (!pipeline_cache_contol_features || pipeline_cache_contol_features->pipelineCreationCacheControl == VK_FALSE)) {
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
                if (pCreateInfos[i].basePipelineIndex > static_cast<int32_t>(i)) {
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
        if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR) {
            skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoNV-flags-03588",
                             "vkCreateRayTracingPipelinesNV: flags must not include "
                             "VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR.");
        }
        if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_DISPATCH_BASE) {
            skip |= LogError(device, "VUID-vkCreateRayTracingPipelinesNV-flags-03816",
                             "vkCreateRayTracingPipelinesNV: flags must not contain the VK_PIPELINE_CREATE_DISPATCH_BASE flag.");
        }
        if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_RAY_TRACING_ALLOW_MOTION_BIT_NV) {
            skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoNV-flags-04948",
                             "vkCreateRayTracingPipelinesNV: flags must not contain the "
                             "VK_PIPELINE_CREATE_RAY_TRACING_ALLOW_MOTION_BIT_NV flag.");
        }
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCreateRayTracingPipelinesKHR(
    VkDevice device, VkDeferredOperationKHR deferredOperation, VkPipelineCache pipelineCache, uint32_t createInfoCount,
    const VkRayTracingPipelineCreateInfoKHR *pCreateInfos, const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines) const {
    bool skip = false;
    const auto *raytracing_features = LvlFindInChain<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>(device_createinfo_pnext);
    if (!raytracing_features || raytracing_features->rayTracingPipeline == VK_FALSE) {
        skip |= LogError(device, "VUID-vkCreateRayTracingPipelinesKHR-rayTracingPipeline-03586",
                         "vkCreateRayTracingPipelinesKHR(): The rayTracingPipeline feature must be enabled.");
    }
    for (uint32_t i = 0; i < createInfoCount; i++) {
        for (uint32_t stage_index = 0; stage_index < pCreateInfos[i].stageCount; ++stage_index) {
            std::stringstream stage_ss;
            stage_ss << "pCreateInfos[" << i << "].pStages[" << stage_index << "]";
            const std::string stage_str = stage_ss.str();
            ValidatePipelineShaderStageCreateInfo("vkCreateRayTracingPipelinesKHR", stage_str.c_str(),
                                                  &pCreateInfos[i].pStages[stage_index]);

            const auto stage = pCreateInfos[i].pStages[stage_index].stage;
            constexpr std::array allowed_stages = {VK_SHADER_STAGE_RAYGEN_BIT_KHR,       VK_SHADER_STAGE_ANY_HIT_BIT_KHR,
                                                   VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR,  VK_SHADER_STAGE_MISS_BIT_KHR,
                                                   VK_SHADER_STAGE_INTERSECTION_BIT_KHR, VK_SHADER_STAGE_CALLABLE_BIT_KHR};
            if (std::find(allowed_stages.begin(), allowed_stages.end(), stage) == allowed_stages.end()) {
                skip |=
                    LogError(device, "VUID-VkRayTracingPipelineCreateInfoKHR-stage-06899",
                             "vkCreateRayTracingPipelinesKHR(): %s is %s.", stage_str.c_str(), string_VkShaderStageFlagBits(stage));
            }
        }
        if (!raytracing_features || (raytracing_features && raytracing_features->rayTraversalPrimitiveCulling == VK_FALSE)) {
            if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_RAY_TRACING_SKIP_AABBS_BIT_KHR) {
                skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoKHR-rayTraversalPrimitiveCulling-03596",
                                 "vkCreateRayTracingPipelinesKHR(): If the rayTraversalPrimitiveCulling feature is not enabled, "
                                 "flags must not include VK_PIPELINE_CREATE_RAY_TRACING_SKIP_AABBS_BIT_KHR.");
            }
            if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_RAY_TRACING_SKIP_TRIANGLES_BIT_KHR) {
                skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoKHR-rayTraversalPrimitiveCulling-03597",
                                 "vkCreateRayTracingPipelinesKHR(): If the rayTraversalPrimitiveCulling feature is not enabled, "
                                 "flags must not include VK_PIPELINE_CREATE_RAY_TRACING_SKIP_TRIANGLES_BIT_KHR.");
            }
        }
        auto feedback_struct = LvlFindInChain<VkPipelineCreationFeedbackCreateInfoEXT>(pCreateInfos[i].pNext);
        if ((feedback_struct != nullptr) && (feedback_struct->pipelineStageCreationFeedbackCount != 0) &&
            (feedback_struct->pipelineStageCreationFeedbackCount != pCreateInfos[i].stageCount)) {
            skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoKHR-pipelineStageCreationFeedbackCount-06652",
                             "vkCreateRayTracingPipelinesKHR(): in pCreateInfo[%" PRIu32
                             "], When chained to VkRayTracingPipelineCreateInfoKHR, "
                             "VkPipelineCreationFeedbackEXT::pipelineStageCreationFeedbackCount"
                             "(=%" PRIu32 ") must equal VkRayTracingPipelineCreateInfoKHR::stageCount(=%" PRIu32 ").",
                             i, feedback_struct->pipelineStageCreationFeedbackCount, pCreateInfos[i].stageCount);
        }
        const auto *vulkan_13_features = LvlFindInChain<VkPhysicalDeviceVulkan13Features>(device_createinfo_pnext);
        const auto *pipeline_cache_contol_features =
            LvlFindInChain<VkPhysicalDevicePipelineCreationCacheControlFeaturesEXT>(device_createinfo_pnext);
        if ((!vulkan_13_features || vulkan_13_features->pipelineCreationCacheControl == VK_FALSE) &&
            (!pipeline_cache_contol_features || pipeline_cache_contol_features->pipelineCreationCacheControl == VK_FALSE)) {
            if (pCreateInfos[i].flags & (VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT_EXT |
                                         VK_PIPELINE_CREATE_EARLY_RETURN_ON_FAILURE_BIT_EXT)) {
                skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoKHR-pipelineCreationCacheControl-02905",
                                 "vkCreateRayTracingPipelinesKHR(): If the pipelineCreationCacheControl feature is not enabled,"
                                 "flags must not include VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT_EXT or"
                                 "VK_PIPELINE_CREATE_EARLY_RETURN_ON_FAILURE_BIT_EXT.");
            }
        }
        if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_INDIRECT_BINDABLE_BIT_NV) {
            skip |=
                LogError(device, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-02904",
                         "vkCreateRayTracingPipelinesKHR(): flags must not include VK_PIPELINE_CREATE_INDIRECT_BINDABLE_BIT_NV.");
        }
        if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_LIBRARY_BIT_KHR) {
            if (pCreateInfos[i].pLibraryInterface == NULL) {
                skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-03465",
                                 "vkCreateRayTracingPipelinesKHR(): If flags includes VK_PIPELINE_CREATE_LIBRARY_BIT_KHR, "
                                 "pLibraryInterface must not be NULL.");
            }
        }
        if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_DISPATCH_BASE) {
            skip |= LogError(device, "VUID-vkCreateRayTracingPipelinesKHR-flags-03816",
                             "vkCreateRayTracingPipelinesKHR(): flags must not contain the VK_PIPELINE_CREATE_DISPATCH_BASE flag.");
        }
        for (uint32_t group_index = 0; group_index < pCreateInfos[i].groupCount; ++group_index) {
            if ((pCreateInfos[i].pGroups[group_index].type == VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR) ||
                (pCreateInfos[i].pGroups[group_index].type == VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR)) {
                if ((pCreateInfos[i].flags & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_ANY_HIT_SHADERS_BIT_KHR) &&
                    (pCreateInfos[i].pGroups[group_index].anyHitShader == VK_SHADER_UNUSED_KHR)) {
                    skip |= LogError(
                        device, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-03470",
                        "vkCreateRayTracingPipelinesKHR(): If flags includes "
                        "VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_ANY_HIT_SHADERS_BIT_KHR,"
                        "for any element of pGroups with a type of VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR"
                        "or VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR, the anyHitShader of that element "
                        "must not be VK_SHADER_UNUSED_KHR");
                }
                if ((pCreateInfos[i].flags & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_CLOSEST_HIT_SHADERS_BIT_KHR) &&
                    (pCreateInfos[i].pGroups[group_index].closestHitShader == VK_SHADER_UNUSED_KHR)) {
                    skip |= LogError(
                        device, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-03471",
                        "vkCreateRayTracingPipelinesKHR(): If flags includes "
                        "VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_CLOSEST_HIT_SHADERS_BIT_KHR,"
                        "for any element of pGroups with a type of VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR"
                        "or VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR, the closestHitShader of that "
                        "element must not be VK_SHADER_UNUSED_KHR");
                }
            }
            if (raytracing_features && raytracing_features->rayTracingPipelineShaderGroupHandleCaptureReplay == VK_TRUE &&
                pCreateInfos[i].pGroups[group_index].pShaderGroupCaptureReplayHandle) {
                if (!(pCreateInfos[i].flags & VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR)) {
                    skip |= LogError(
                        device, "VUID-VkRayTracingPipelineCreateInfoKHR-rayTracingPipelineShaderGroupHandleCaptureReplay-03599",
                        "vkCreateRayTracingPipelinesKHR(): If "
                        "VkPhysicalDeviceRayTracingPipelineFeaturesKHR::rayTracingPipelineShaderGroupHandleCaptureReplay is "
                        "VK_TRUE and the pShaderGroupCaptureReplayHandle member of any element of pGroups is not NULL, flags must "
                        "include VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR.");
                }
            }
        }
        if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_DERIVATIVE_BIT) {
            if (pCreateInfos[i].basePipelineIndex != -1) {
                if (pCreateInfos[i].basePipelineHandle != VK_NULL_HANDLE) {
                    skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-03423",
                                     "vkCreateRayTracingPipelinesKHR(): parameter, pCreateInfos->basePipelineHandle, must be "
                                     "VK_NULL_HANDLE if pCreateInfos->flags contains the VK_PIPELINE_CREATE_DERIVATIVE_BIT flag "
                                     "and pCreateInfos->basePipelineIndex is not -1.");
                }
                if (pCreateInfos[i].basePipelineIndex > static_cast<int32_t>(i)) {
                    skip |=
                        LogError(device, "VUID-vkCreateRayTracingPipelinesKHR-flags-03415",
                                 "vkCreateRayTracingPipelinesKHR(): If the flags member of any element of pCreateInfos contains the"
                                 "VK_PIPELINE_CREATE_DERIVATIVE_BIT flag, and the basePipelineIndex member of that same element is"
                                 "not -1, basePipelineIndex must be less than the index into pCreateInfos that corresponds to that "
                                 "element.");
                }
            }
            if (pCreateInfos[i].basePipelineHandle == VK_NULL_HANDLE) {
                if (static_cast<uint32_t>(pCreateInfos[i].basePipelineIndex) >= createInfoCount) {
                    skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-03422",
                                     "vkCreateRayTracingPipelinesKHR(): if flags contains the VK_PIPELINE_CREATE_DERIVATIVE_BIT and"
                                     "basePipelineHandle is VK_NULL_HANDLE, basePipelineIndex (%" PRId32
                                     ") must be a valid into the calling"
                                     "commands pCreateInfos parameter %" PRIu32 ".",
                                     pCreateInfos[i].basePipelineIndex, createInfoCount);
                }
            } else {
                if (pCreateInfos[i].basePipelineIndex != -1) {
                    skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-03424",
                                     "vkCreateRayTracingPipelinesKHR(): if flags contains the VK_PIPELINE_CREATE_DERIVATIVE_BIT and"
                                     "basePipelineHandle is not VK_NULL_HANDLE, basePipelineIndex must be -1.");
                }
            }
        }
        if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR &&
            (raytracing_features && raytracing_features->rayTracingPipelineShaderGroupHandleCaptureReplay == VK_FALSE)) {
            skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-03598",
                             "vkCreateRayTracingPipelinesKHR(): If flags includes "
                             "VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR, "
                             "rayTracingPipelineShaderGroupHandleCaptureReplay must be enabled.");
        }
        const bool library_enabled = IsExtEnabled(device_extensions.vk_khr_pipeline_library);
        if (!library_enabled && (pCreateInfos[i].pLibraryInfo || pCreateInfos[i].pLibraryInterface)) {
            skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoKHR-pLibraryInfo-03595",
                             "vkCreateRayTracingPipelinesKHR(): If the VK_KHR_pipeline_library extension is not enabled, "
                             "pLibraryInfo and pLibraryInterface must be NULL.");
        }
        if (pCreateInfos[i].pLibraryInfo) {
            if (pCreateInfos[i].pLibraryInfo->libraryCount == 0) {
                if (pCreateInfos[i].stageCount == 0) {
                    skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoKHR-pLibraryInfo-03600",
                                     "vkCreateRayTracingPipelinesKHR(): If pLibraryInfo is not NULL and its libraryCount is 0, "
                                     "stageCount must not be 0.");
                }
                if (pCreateInfos[i].groupCount == 0) {
                    skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoKHR-pLibraryInfo-03601",
                                     "vkCreateRayTracingPipelinesKHR(): If pLibraryInfo is not NULL and its libraryCount is 0, "
                                     "groupCount must not be 0.");
                }
            } else {
                if (pCreateInfos[i].pLibraryInterface == NULL) {
                    skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoKHR-pLibraryInfo-03590",
                                     "vkCreateRayTracingPipelinesKHR(): If pLibraryInfo is not NULL and its libraryCount member "
                                     "is greater than 0, its "
                                     "pLibraryInterface member must not be NULL.");
                }
            }
        }
        if (pCreateInfos[i].pLibraryInterface) {
            if (pCreateInfos[i].pLibraryInterface->maxPipelineRayHitAttributeSize >
                phys_dev_ext_props.ray_tracing_props_khr.maxRayHitAttributeSize) {
                skip |= LogError(device, "VUID-VkRayTracingPipelineInterfaceCreateInfoKHR-maxPipelineRayHitAttributeSize-03605",
                                 "vkCreateRayTracingPipelinesKHR(): maxPipelineRayHitAttributeSize must be less than or equal to "
                                 "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::maxRayHitAttributeSize.");
            }
        }
        if (deferredOperation != VK_NULL_HANDLE) {
            if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_EARLY_RETURN_ON_FAILURE_BIT_EXT) {
                skip |=
                    LogError(device, "VUID-vkCreateRayTracingPipelinesKHR-deferredOperation-03587",
                             "vkCreateRayTracingPipelinesKHR(): If deferredOperation is not VK_NULL_HANDLE, the flags member of "
                             "elements of pCreateInfos must not include VK_PIPELINE_CREATE_EARLY_RETURN_ON_FAILURE_BIT_EXT.");
            }
        }
        if (pCreateInfos[i].pDynamicState) {
            for (uint32_t j = 0; j < pCreateInfos[i].pDynamicState->dynamicStateCount; ++j) {
                if (pCreateInfos[i].pDynamicState->pDynamicStates[j] != VK_DYNAMIC_STATE_RAY_TRACING_PIPELINE_STACK_SIZE_KHR) {
                    skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoKHR-pDynamicStates-03602",
                                     "vkCreateRayTracingPipelinesKHR(): pCreateInfos[%" PRIu32
                                     "].pDynamicState->pDynamicStates[%" PRIu32 "] is %s.",
                                     i, j, string_VkDynamicState(pCreateInfos[i].pDynamicState->pDynamicStates[j]));
                }
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
    if (!IsExtEnabled(device_extensions.vk_khr_swapchain))
        skip |= OutputExtensionError("vkGetDeviceGroupSurfacePresentModes2EXT", VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    if (!IsExtEnabled(device_extensions.vk_khr_get_surface_capabilities2))
        skip |= OutputExtensionError("vkGetDeviceGroupSurfacePresentModes2EXT", VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);
    if (!IsExtEnabled(device_extensions.vk_khr_surface))
        skip |= OutputExtensionError("vkGetDeviceGroupSurfacePresentModes2EXT", VK_KHR_SURFACE_EXTENSION_NAME);
    if (!IsExtEnabled(device_extensions.vk_khr_get_physical_device_properties2))
        skip |=
            OutputExtensionError("vkGetDeviceGroupSurfacePresentModes2EXT", VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    if (!IsExtEnabled(device_extensions.vk_ext_full_screen_exclusive))
        skip |= OutputExtensionError("vkGetDeviceGroupSurfacePresentModes2EXT", VK_EXT_FULL_SCREEN_EXCLUSIVE_EXTENSION_NAME);
    skip |= ValidateStructType(
        "vkGetDeviceGroupSurfacePresentModes2EXT", "pSurfaceInfo", "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR",
        pSurfaceInfo, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR, true,
        "VUID-vkGetDeviceGroupSurfacePresentModes2EXT-pSurfaceInfo-parameter", "VUID-VkPhysicalDeviceSurfaceInfo2KHR-sType-sType");
    if (pSurfaceInfo != NULL) {
        constexpr std::array allowed_structs = {VK_STRUCTURE_TYPE_SURFACE_FULL_SCREEN_EXCLUSIVE_INFO_EXT,
                                                VK_STRUCTURE_TYPE_SURFACE_FULL_SCREEN_EXCLUSIVE_WIN32_INFO_EXT};

        skip |= ValidateStructPnext("vkGetDeviceGroupSurfacePresentModes2EXT", "pSurfaceInfo->pNext",
                                    "VkSurfaceFullScreenExclusiveInfoEXT, VkSurfaceFullScreenExclusiveWin32InfoEXT",
                                    pSurfaceInfo->pNext, allowed_structs.size(), allowed_structs.data(),
                                    GeneratedVulkanHeaderVersion, "VUID-VkPhysicalDeviceSurfaceInfo2KHR-pNext-pNext",
                                    "VUID-VkPhysicalDeviceSurfaceInfo2KHR-sType-unique");

        if (pSurfaceInfo->surface == VK_NULL_HANDLE && !instance_extensions.vk_google_surfaceless_query) {
            skip |= LogError(device, "VUID-vkGetPhysicalDeviceSurfacePresentModes2EXT-pSurfaceInfo-06521",
                             "vkGetPhysicalDeviceSurfacePresentModes2EXT: pSurfaceInfo->surface is VK_NULL_HANDLE and "
                             "VK_GOOGLE_surfaceless_query is not enabled.");
        }

        skip |= ValidateRequiredHandle("vkGetDeviceGroupSurfacePresentModes2EXT", "pSurfaceInfo->surface", pSurfaceInfo->surface);
    }
    return skip;
}
#endif

bool StatelessValidation::manual_PreCallValidateCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo *pCreateInfo,
                                                                  const VkAllocationCallbacks *pAllocator,
                                                                  VkFramebuffer *pFramebuffer) const {
    // Validation for pAttachments which is excluded from the generated validation code due to a 'noautovalidity' tag in vk.xml
    bool skip = false;
    if ((pCreateInfo->flags & VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT) == 0) {
        skip |= ValidateArray("vkCreateFramebuffer", "attachmentCount", "pAttachments", pCreateInfo->attachmentCount,
                              &pCreateInfo->pAttachments, false, true, kVUIDUndefined, kVUIDUndefined);
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdSetLineStippleEXT(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor,
                                                                     uint16_t lineStipplePattern) const {
    bool skip = false;

    if (lineStippleFactor < 1 || lineStippleFactor > 256) {
        skip |= LogError(commandBuffer, "VUID-vkCmdSetLineStippleEXT-lineStippleFactor-02776",
                         "vkCmdSetLineStippleEXT::lineStippleFactor=%" PRIu32 " is not in [1,256].", lineStippleFactor);
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

    const auto *index_type_uint8_features = LvlFindInChain<VkPhysicalDeviceIndexTypeUint8FeaturesEXT>(device_createinfo_pnext);
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
        skip |=
            LogError(commandBuffer, "VUID-vkCmdBindVertexBuffers-firstBinding-00624",
                     "vkCmdBindVertexBuffers() firstBinding (%" PRIu32 ") must be less than maxVertexInputBindings (%" PRIu32 ")",
                     firstBinding, device_limits.maxVertexInputBindings);
    } else if ((firstBinding + bindingCount) > device_limits.maxVertexInputBindings) {
        skip |= LogError(commandBuffer, "VUID-vkCmdBindVertexBuffers-firstBinding-00625",
                         "vkCmdBindVertexBuffers() sum of firstBinding (%" PRIu32 ") and bindingCount (%" PRIu32
                         ") must be less than "
                         "maxVertexInputBindings (%" PRIu32 ")",
                         firstBinding, bindingCount, device_limits.maxVertexInputBindings);
    }

    for (uint32_t i = 0; i < bindingCount; ++i) {
        if (pBuffers[i] == VK_NULL_HANDLE) {
            const auto *robustness2_features = LvlFindInChain<VkPhysicalDeviceRobustness2FeaturesEXT>(device_createinfo_pnext);
            if (!(robustness2_features && robustness2_features->nullDescriptor)) {
                skip |=
                    LogError(commandBuffer, "VUID-vkCmdBindVertexBuffers-pBuffers-04001",
                             "vkCmdBindVertexBuffers() required parameter pBuffers[%" PRIu32 "] specified as VK_NULL_HANDLE", i);
            } else {
                if (pOffsets[i] != 0) {
                    skip |= LogError(commandBuffer, "VUID-vkCmdBindVertexBuffers-pBuffers-04002",
                                     "vkCmdBindVertexBuffers() pBuffers[%" PRIu32 "] is VK_NULL_HANDLE, but pOffsets[%" PRIu32
                                     "] is not 0",
                                     i, i);
                }
            }
        }
    }

    return skip;
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
        skip |= LogError(counterBuffer, "VUID-vkCmdDrawIndirectByteCountEXT-vertexStride-02289",
                         "vkCmdDrawIndirectByteCountEXT: vertexStride (%" PRIu32
                         ") must be between 0 and maxTransformFeedbackBufferDataStride (%" PRIu32 ").",
                         vertexStride, phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBufferDataStride);
    }

    if ((counterOffset % 4) != 0) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDrawIndirectByteCountEXT-counterBufferOffset-04568",
                         "vkCmdDrawIndirectByteCountEXT(): offset (%" PRIu32 ") must be a multiple of 4.", counterOffset);
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
    const auto *ycbcr_features = LvlFindInChain<VkPhysicalDeviceSamplerYcbcrConversionFeatures>(device_createinfo_pnext);
    if ((ycbcr_features == nullptr) || (ycbcr_features->samplerYcbcrConversion == VK_FALSE)) {
        const auto *vulkan_11_features = LvlFindInChain<VkPhysicalDeviceVulkan11Features>(device_createinfo_pnext);
        if ((vulkan_11_features == nullptr) || (vulkan_11_features->samplerYcbcrConversion == VK_FALSE)) {
            skip |= LogError(device, "VUID-vkCreateSamplerYcbcrConversion-None-01648",
                             "%s: samplerYcbcrConversion must be enabled.", apiName);
        }
    }

#ifdef VK_USE_PLATFORM_ANDROID_KHR
    const VkExternalFormatANDROID *external_format_android = LvlFindInChain<VkExternalFormatANDROID>(pCreateInfo);
    const bool is_external_format = external_format_android != nullptr && external_format_android->externalFormat != 0;
#else
    const bool is_external_format = false;
#endif

    const VkFormat format = pCreateInfo->format;

    // If there is a VkExternalFormatANDROID with externalFormat != 0, the value of components is ignored.
    if (!is_external_format) {
        const VkComponentMapping components = pCreateInfo->components;
        // XChroma Subsampled is same as "the format has a _422 or _420 suffix" from spec
        if (FormatIsXChromaSubsampled(format) == true) {
            if ((components.g != VK_COMPONENT_SWIZZLE_G) && (components.g != VK_COMPONENT_SWIZZLE_IDENTITY)) {
                skip |=
                    LogError(device, "VUID-VkSamplerYcbcrConversionCreateInfo-components-02581",
                             "%s: When using a XChroma subsampled format (%s) the components.g needs to be VK_COMPONENT_SWIZZLE_G "
                             "or VK_COMPONENT_SWIZZLE_IDENTITY, but is %s.",
                             apiName, string_VkFormat(format), string_VkComponentSwizzle(components.g));
            }

            if ((components.a != VK_COMPONENT_SWIZZLE_A) && (components.a != VK_COMPONENT_SWIZZLE_IDENTITY) &&
                (components.a != VK_COMPONENT_SWIZZLE_ONE) && (components.a != VK_COMPONENT_SWIZZLE_ZERO)) {
                skip |= LogError(
                    device, "VUID-VkSamplerYcbcrConversionCreateInfo-components-02582",
                    "%s: When using a XChroma subsampled format (%s) the components.a needs to be VK_COMPONENT_SWIZZLE_A or "
                    "VK_COMPONENT_SWIZZLE_IDENTITY or VK_COMPONENT_SWIZZLE_ONE or VK_COMPONENT_SWIZZLE_ZERO, but is %s.",
                    apiName, string_VkFormat(format), string_VkComponentSwizzle(components.a));
            }

            if ((components.r != VK_COMPONENT_SWIZZLE_R) && (components.r != VK_COMPONENT_SWIZZLE_IDENTITY) &&
                (components.r != VK_COMPONENT_SWIZZLE_B)) {
                skip |=
                    LogError(device, "VUID-VkSamplerYcbcrConversionCreateInfo-components-02583",
                             "%s: When using a XChroma subsampled format (%s) the components.r needs to be VK_COMPONENT_SWIZZLE_R "
                             "or VK_COMPONENT_SWIZZLE_IDENTITY or VK_COMPONENT_SWIZZLE_B, but is %s.",
                             apiName, string_VkFormat(format), string_VkComponentSwizzle(components.r));
            }

            if ((components.b != VK_COMPONENT_SWIZZLE_B) && (components.b != VK_COMPONENT_SWIZZLE_IDENTITY) &&
                (components.b != VK_COMPONENT_SWIZZLE_R)) {
                skip |=
                    LogError(device, "VUID-VkSamplerYcbcrConversionCreateInfo-components-02584",
                             "%s: When using a XChroma subsampled format (%s) the components.b needs to be VK_COMPONENT_SWIZZLE_B "
                             "or VK_COMPONENT_SWIZZLE_IDENTITY or VK_COMPONENT_SWIZZLE_R, but is %s.",
                             apiName, string_VkFormat(format), string_VkComponentSwizzle(components.b));
            }

            // If one is identity, both need to be
            const bool r_identity = ((components.r == VK_COMPONENT_SWIZZLE_R) || (components.r == VK_COMPONENT_SWIZZLE_IDENTITY));
            const bool b_identity = ((components.b == VK_COMPONENT_SWIZZLE_B) || (components.b == VK_COMPONENT_SWIZZLE_IDENTITY));
            if ((r_identity != b_identity) && ((r_identity == true) || (b_identity == true))) {
                skip |=
                    LogError(device, "VUID-VkSamplerYcbcrConversionCreateInfo-components-02585",
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
                skip |= LogError(
                    device, vuid,
                    "%s: The ycbcrModel is not VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY so components.r (%s), "
                    "components.g (%s), nor components.b (%s) can't be VK_COMPONENT_SWIZZLE_ZERO or VK_COMPONENT_SWIZZLE_ONE.",
                    apiName, string_VkComponentSwizzle(components.r), string_VkComponentSwizzle(components.g),
                    string_VkComponentSwizzle(components.b));
            }

            // "must not correspond to a component which contains zero or one as a consequence of conversion to RGBA"
            // 4 component format = no issue
            // 3 = no [a]
            // 2 = no [b,a]
            // 1 = no [g,b,a]
            // depth/stencil = no [g,b,a] (shouldn't ever occur, but no VU preventing it)
            const uint32_t component_count = (FormatIsDepthOrStencil(format) == true) ? 1 : FormatComponentCount(format);

            if ((component_count < 4) && ((components.r == VK_COMPONENT_SWIZZLE_A) || (components.g == VK_COMPONENT_SWIZZLE_A) ||
                                          (components.b == VK_COMPONENT_SWIZZLE_A))) {
                skip |= LogError(device, vuid,
                                 "%s: The ycbcrModel is not VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY so components.r (%s), "
                                 "components.g (%s), or components.b (%s) can't be VK_COMPONENT_SWIZZLE_A.",
                                 apiName, string_VkComponentSwizzle(components.r), string_VkComponentSwizzle(components.g),
                                 string_VkComponentSwizzle(components.b));
            } else if ((component_count < 3) &&
                       ((components.r == VK_COMPONENT_SWIZZLE_B) || (components.g == VK_COMPONENT_SWIZZLE_B) ||
                        (components.b == VK_COMPONENT_SWIZZLE_B) || (components.b == VK_COMPONENT_SWIZZLE_IDENTITY))) {
                skip |= LogError(device, vuid,
                                 "%s: The ycbcrModel is not VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY so components.r (%s), "
                                 "components.g (%s), or components.b (%s) can't be VK_COMPONENT_SWIZZLE_B "
                                 "(components.b also can't be VK_COMPONENT_SWIZZLE_IDENTITY).",
                                 apiName, string_VkComponentSwizzle(components.r), string_VkComponentSwizzle(components.g),
                                 string_VkComponentSwizzle(components.b));
            } else if ((component_count < 2) &&
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

bool StatelessValidation::ValidateExternalSemaphoreHandleType(VkSemaphore semaphore, const char *vuid, const char *caller,
                                                              VkExternalSemaphoreHandleTypeFlagBits handle_type,
                                                              VkExternalSemaphoreHandleTypeFlags allowed_types) const {
    bool skip = false;
    if (0 == (handle_type & allowed_types)) {
        skip |= LogError(semaphore, vuid, "%s(): handleType %s is not one of the supported handleTypes (%s).", caller,
                         string_VkExternalSemaphoreHandleTypeFlagBits(handle_type),
                         string_VkExternalSemaphoreHandleTypeFlags(allowed_types).c_str());
    }
    return skip;
}

bool StatelessValidation::ValidateExternalFenceHandleType(VkFence fence, const char *vuid, const char *caller,
                                                          VkExternalFenceHandleTypeFlagBits handle_type,
                                                          VkExternalFenceHandleTypeFlags allowed_types) const {
    bool skip = false;
    if (0 == (handle_type & allowed_types)) {
        skip |= LogError(fence, vuid, "%s(): handleType %s is not one of the supported handleTypes (%s).", caller,
                         string_VkExternalFenceHandleTypeFlagBits(handle_type),
                         string_VkExternalFenceHandleTypeFlags(allowed_types).c_str());
    }
    return skip;
}

static constexpr VkExternalSemaphoreHandleTypeFlags kSemFdHandleTypes =
    VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT | VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT;

bool StatelessValidation::manual_PreCallValidateGetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR *info,
                                                                  int *pFd) const {
    return ValidateExternalSemaphoreHandleType(info->semaphore, "VUID-VkSemaphoreGetFdInfoKHR-handleType-01136",
                                               "vkGetSemaphoreFdKHR", info->handleType, kSemFdHandleTypes);
}

bool StatelessValidation::manual_PreCallValidateImportSemaphoreFdKHR(VkDevice device,
                                                                     const VkImportSemaphoreFdInfoKHR *info) const {
    bool skip = false;
    const char *func_name = "vkImportSemaphoreFdKHR";

    skip |= ValidateExternalSemaphoreHandleType(info->semaphore, "VUID-VkImportSemaphoreFdInfoKHR-handleType-01143", func_name,
                                                info->handleType, kSemFdHandleTypes);

    if (info->handleType == VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT &&
        (info->flags & VK_SEMAPHORE_IMPORT_TEMPORARY_BIT) == 0) {
        skip |= LogError(info->semaphore, "VUID-VkImportSemaphoreFdInfoKHR-handleType-07307",
                         "%s(): handleType is VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT so"
                         " VK_SEMAPHORE_IMPORT_TEMPORARY_BIT must be set, but flags is 0x%x",
                         func_name, info->flags);
    }
    return skip;
}

static constexpr VkExternalFenceHandleTypeFlags kFenceFdHandleTypes =
    VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_FD_BIT | VK_EXTERNAL_FENCE_HANDLE_TYPE_SYNC_FD_BIT;

bool StatelessValidation::manual_PreCallValidateGetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR *info, int *pFd) const {
    return ValidateExternalFenceHandleType(info->fence, "VUID-VkFenceGetFdInfoKHR-handleType-01456", "vkGetFenceFdKHR",
                                           info->handleType, kFenceFdHandleTypes);
}

bool StatelessValidation::manual_PreCallValidateImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR *info) const {
    bool skip = false;
    const char *func_name = "vkImportFenceFdKHR";

    skip |= ValidateExternalFenceHandleType(info->fence, "VUID-VkImportFenceFdInfoKHR-handleType-01464", func_name,
                                            info->handleType, kFenceFdHandleTypes);

    if (info->handleType == VK_EXTERNAL_FENCE_HANDLE_TYPE_SYNC_FD_BIT && (info->flags & VK_FENCE_IMPORT_TEMPORARY_BIT) == 0) {
        skip |= LogError(info->fence, "VUID-VkImportFenceFdInfoKHR-handleType-07306",
                         "%s(): handleType is VK_EXTERNAL_FENCE_HANDLE_TYPE_SYNC_FD_BIT so"
                         " VK_FENCE_IMPORT_TEMPORARY_BIT must be set, but flags is 0x%x",
                         func_name, info->flags);
    }
    return skip;
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
static constexpr VkExternalSemaphoreHandleTypeFlags kSemWin32HandleTypes = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT |
                                                                           VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT |
                                                                           VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_D3D12_FENCE_BIT;

bool StatelessValidation::manual_PreCallValidateImportSemaphoreWin32HandleKHR(
    VkDevice device, const VkImportSemaphoreWin32HandleInfoKHR *info) const {
    bool skip = false;
    const char *func_name = "vkImportSemaphoreWin32HandleKHR";

    skip |= ValidateExternalSemaphoreHandleType(info->semaphore, "VUID-VkImportSemaphoreWin32HandleInfoKHR-handleType-01140",
                                                func_name, info->handleType, kSemWin32HandleTypes);

    static constexpr auto kNameAllowedTypes =
        VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT | VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_D3D12_FENCE_BIT;
    if ((info->handleType & kNameAllowedTypes) == 0 && info->name) {
        skip |= LogError(info->semaphore, "VUID-VkImportSemaphoreWin32HandleInfoKHR-handleType-01466",
                         "%s(): name (%p) must be NULL if handleType is %s", func_name, info->name,
                         string_VkExternalSemaphoreHandleTypeFlagBits(info->handleType));
    }
    if (info->handle && info->name) {
        skip |= LogError(info->semaphore, "VUID-VkImportSemaphoreWin32HandleInfoKHR-handle-01469",
                         "%s(): both handle (%p) and name (%p) are non-NULL", func_name, info->handle, info->name);
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateGetSemaphoreWin32HandleKHR(VkDevice device,
                                                                           const VkSemaphoreGetWin32HandleInfoKHR *info,
                                                                           HANDLE *pHandle) const {
    return ValidateExternalSemaphoreHandleType(info->semaphore, "VUID-VkSemaphoreGetWin32HandleInfoKHR-handleType-01131",
                                               "vkGetSemaphoreWin32HandleKHR", info->handleType, kSemWin32HandleTypes);
}

static constexpr VkExternalFenceHandleTypeFlags kFenceWin32HandleTypes =
    VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_WIN32_BIT | VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT;

bool StatelessValidation::manual_PreCallValidateImportFenceWin32HandleKHR(VkDevice device,
                                                                          const VkImportFenceWin32HandleInfoKHR *info) const {
    bool skip = false;
    const char *func_name = "vkImportFenceWin32HandleKHR";

    skip |= ValidateExternalFenceHandleType(info->fence, func_name, "VUID-VkImportFenceWin32HandleInfoKHR-handleType-01457",
                                            info->handleType, kFenceWin32HandleTypes);

    static constexpr auto kNameAllowedTypes = VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_WIN32_BIT;
    if ((info->handleType & kNameAllowedTypes) == 0 && info->name) {
        skip |= LogError(info->fence, "VUID-VkImportFenceWin32HandleInfoKHR-handleType-01459",
                         "%s(): name (%p) must be NULL if handleType is %s", func_name, info->name,
                         string_VkExternalFenceHandleTypeFlagBits(info->handleType));
    }
    if (info->handle && info->name) {
        skip |= LogError(info->fence, "VUID-VkImportFenceWin32HandleInfoKHR-handle-01462",
                         "%s(): both handle (%p) and name (%p) are non-NULL", func_name, info->handle, info->name);
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateGetFenceWin32HandleKHR(VkDevice device, const VkFenceGetWin32HandleInfoKHR *info,
                                                                       HANDLE *pHandle) const {
    return ValidateExternalFenceHandleType(info->fence, "vkGetFenceWin32HandleKHR",
                                           "VUID-VkFenceGetWin32HandleInfoKHR-handleType-01452", info->handleType,
                                           kFenceWin32HandleTypes);
}
#endif

bool StatelessValidation::manual_PreCallValidateCopyAccelerationStructureToMemoryKHR(
    VkDevice device, VkDeferredOperationKHR deferredOperation, const VkCopyAccelerationStructureToMemoryInfoKHR *pInfo) const {
    bool skip = false;
    if (pInfo->mode != VK_COPY_ACCELERATION_STRUCTURE_MODE_SERIALIZE_KHR) {
        skip |= LogError(device, "VUID-VkCopyAccelerationStructureToMemoryInfoKHR-mode-03412",
                         "vkCopyAccelerationStructureToMemoryKHR: mode must be VK_COPY_ACCELERATION_STRUCTURE_MODE_SERIALIZE_KHR.");
    }
    const auto *acc_struct_features = LvlFindInChain<VkPhysicalDeviceAccelerationStructureFeaturesKHR>(device_createinfo_pnext);
    if (!acc_struct_features || acc_struct_features->accelerationStructureHostCommands == VK_FALSE) {
        skip |= LogError(
            device, "VUID-vkCopyAccelerationStructureToMemoryKHR-accelerationStructureHostCommands-03584",
            "vkCopyAccelerationStructureToMemoryKHR: The "
            "VkPhysicalDeviceAccelerationStructureFeaturesKHR::accelerationStructureHostCommands feature must be enabled.");
    }
    skip |= ValidateRequiredPointer("vkCopyAccelerationStructureToMemoryKHR", "pInfo->dst.hostAddress", pInfo->dst.hostAddress,
                                    "VUID-vkCopyAccelerationStructureToMemoryKHR-pInfo-03732");
    if (SafeModulo((VkDeviceSize)pInfo->dst.hostAddress, 16) != 0) {
        skip |= LogError(device, "VUID-vkCopyAccelerationStructureToMemoryKHR-pInfo-03751",
                         "vkCopyAccelerationStructureToMemoryKHR(): pInfo->dst.hostAddress must be aligned to 16 bytes.");
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
    if (SafeModulo(pInfo->dst.deviceAddress, 256) != 0) {
        skip |= LogError(device, "VUID-vkCmdCopyAccelerationStructureToMemoryKHR-pInfo-03740",
                         "vkCmdCopyAccelerationStructureToMemoryKHR(): pInfo->dst.deviceAddress (0x%" PRIx64
                         ") must be aligned to 256 bytes.",
                         pInfo->dst.deviceAddress);
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
    VkDevice device, VkDeferredOperationKHR deferredOperation, const VkCopyAccelerationStructureInfoKHR *pInfo) const {
    bool skip = false;
    skip |= ValidateCopyAccelerationStructureInfoKHR(pInfo, "vkCopyAccelerationStructureKHR()");
    const auto *acc_struct_features = LvlFindInChain<VkPhysicalDeviceAccelerationStructureFeaturesKHR>(device_createinfo_pnext);
    if (!acc_struct_features || acc_struct_features->accelerationStructureHostCommands == VK_FALSE) {
        skip |= LogError(
            device, "VUID-vkCopyAccelerationStructureKHR-accelerationStructureHostCommands-03582",
            "vkCopyAccelerationStructureKHR: The "
            "VkPhysicalDeviceAccelerationStructureFeaturesKHR::accelerationStructureHostCommands feature must be enabled.");
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdCopyAccelerationStructureKHR(
    VkCommandBuffer commandBuffer, const VkCopyAccelerationStructureInfoKHR *pInfo) const {
    bool skip = false;
    skip |= ValidateCopyAccelerationStructureInfoKHR(pInfo, "vkCmdCopyAccelerationStructureKHR()");
    return skip;
}

bool StatelessValidation::ValidateCopyMemoryToAccelerationStructureInfoKHR(const VkCopyMemoryToAccelerationStructureInfoKHR *pInfo,
                                                                           const char *api_name, bool is_cmd) const {
    bool skip = false;
    if (pInfo->mode != VK_COPY_ACCELERATION_STRUCTURE_MODE_DESERIALIZE_KHR) {
        skip |= LogError(device, "VUID-VkCopyMemoryToAccelerationStructureInfoKHR-mode-03413",
                         "(%s): mode must be VK_COPY_ACCELERATION_STRUCTURE_MODE_DESERIALIZE_KHR.", api_name);
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCopyMemoryToAccelerationStructureKHR(
    VkDevice device, VkDeferredOperationKHR deferredOperation, const VkCopyMemoryToAccelerationStructureInfoKHR *pInfo) const {
    bool skip = false;
    skip |= ValidateCopyMemoryToAccelerationStructureInfoKHR(pInfo, "vkCopyMemoryToAccelerationStructureKHR()", true);
    const auto *acc_struct_features = LvlFindInChain<VkPhysicalDeviceAccelerationStructureFeaturesKHR>(device_createinfo_pnext);
    if (!acc_struct_features || acc_struct_features->accelerationStructureHostCommands == VK_FALSE) {
        skip |= LogError(
            device, "VUID-vkCopyMemoryToAccelerationStructureKHR-accelerationStructureHostCommands-03583",
            "vkCopyMemoryToAccelerationStructureKHR: The "
            "VkPhysicalDeviceAccelerationStructureFeaturesKHR::accelerationStructureHostCommands feature must be enabled.");
    }
    skip |= ValidateRequiredPointer("vkCopyMemoryToAccelerationStructureKHR", "pInfo->src.hostAddress", pInfo->src.hostAddress,
                                    "VUID-vkCopyMemoryToAccelerationStructureKHR-pInfo-03729");
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdCopyMemoryToAccelerationStructureKHR(
    VkCommandBuffer commandBuffer, const VkCopyMemoryToAccelerationStructureInfoKHR *pInfo) const {
    bool skip = false;
    skip |= ValidateCopyMemoryToAccelerationStructureInfoKHR(pInfo, "vkCmdCopyMemoryToAccelerationStructureKHR()", false);
    if (SafeModulo(pInfo->src.deviceAddress, 256) != 0) {
        skip |= LogError(device, "VUID-vkCmdCopyMemoryToAccelerationStructureKHR-pInfo-03743",
                         "vkCmdCopyMemoryToAccelerationStructureKHR(): pInfo->src.deviceAddress (0x%" PRIx64
                         ") must be aligned to 256 bytes.",
                         pInfo->src.deviceAddress);
    }
    return skip;
}
bool StatelessValidation::manual_PreCallValidateCmdWriteAccelerationStructuresPropertiesKHR(
    VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount, const VkAccelerationStructureKHR *pAccelerationStructures,
    VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery) const {
    bool skip = false;
    if (!(queryType == VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR ||
          queryType == VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR)) {
        if (!IsExtEnabled(device_extensions.vk_khr_ray_tracing_maintenance1)) {
            skip |= LogError(device, "VUID-vkCmdWriteAccelerationStructuresPropertiesKHR-queryType-03432",
                             "vkCmdWriteAccelerationStructuresPropertiesKHR: queryType (%s) is invalid.",
                             string_VkQueryType(queryType));
        } else if (!(queryType == VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SIZE_KHR ||
                     queryType == VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_BOTTOM_LEVEL_POINTERS_KHR)) {
            skip |= LogError(device, "VUID-vkCmdWriteAccelerationStructuresPropertiesKHR-queryType-06742",
                             "vkCmdWriteAccelerationStructuresPropertiesKHR: queryType (%s) must be is invalid.",
                             string_VkQueryType(queryType));
        }
    }
    return skip;
}
bool StatelessValidation::manual_PreCallValidateWriteAccelerationStructuresPropertiesKHR(
    VkDevice device, uint32_t accelerationStructureCount, const VkAccelerationStructureKHR *pAccelerationStructures,
    VkQueryType queryType, size_t dataSize, void *pData, size_t stride) const {
    bool skip = false;
    const auto *acc_structure_features = LvlFindInChain<VkPhysicalDeviceAccelerationStructureFeaturesKHR>(device_createinfo_pnext);
    if (!acc_structure_features || acc_structure_features->accelerationStructureHostCommands == VK_FALSE) {
        skip |= LogError(
            device, "VUID-vkWriteAccelerationStructuresPropertiesKHR-accelerationStructureHostCommands-03585",
            "vkCmdWriteAccelerationStructuresPropertiesKHR: The "
            "VkPhysicalDeviceAccelerationStructureFeaturesKHR::accelerationStructureHostCommands feature must be enabled.");
    }
    if (dataSize < accelerationStructureCount * stride) {
        skip |= LogError(device, "VUID-vkWriteAccelerationStructuresPropertiesKHR-dataSize-03452",
                         "vkWriteAccelerationStructuresPropertiesKHR: dataSize (%zu) must be greater than or equal to "
                         "accelerationStructureCount (%" PRIu32 ") *stride(%zu).",
                         dataSize, accelerationStructureCount, stride);
    }

    if (!(queryType == VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR ||
          queryType == VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR)) {
        if (!IsExtEnabled(device_extensions.vk_khr_ray_tracing_maintenance1)) {
            skip |= LogError(device, "VUID-vkWriteAccelerationStructuresPropertiesKHR-queryType-03432",
                             "vkWriteAccelerationStructuresPropertiesKHR: queryType (%s) must be "
                             "VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR or "
                             "VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR.",
                             string_VkQueryType(queryType));
        } else if (!(queryType == VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SIZE_KHR ||
                     queryType == VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_BOTTOM_LEVEL_POINTERS_KHR)) {
            skip |= LogError(device, "VUID-vkWriteAccelerationStructuresPropertiesKHR-queryType-06742",
                             "vkWriteAccelerationStructuresPropertiesKHR: queryType (%s) must be "
                             "VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SIZE_KHR or "
                             "VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_BOTTOM_LEVEL_POINTERS_KHR or "
                             "VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR or "
                             "VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR.",
                             string_VkQueryType(queryType));
        }
    }

    if (SafeModulo(stride, sizeof(VkDeviceSize)) != 0) {
        if (queryType == VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR) {
            skip |= LogError(device, "VUID-vkWriteAccelerationStructuresPropertiesKHR-queryType-03448",
                             "vkWriteAccelerationStructuresPropertiesKHR: If queryType is "
                             "VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR,"
                             "then stride (%zu) must be a multiple of the size of VkDeviceSize",
                             stride);
        } else if (queryType == VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR) {
            skip |= LogError(device, "VUID-vkWriteAccelerationStructuresPropertiesKHR-queryType-03450",
                             "vkWriteAccelerationStructuresPropertiesKHR: If queryType is "
                             "VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR,"
                             "then stride (%zu) must be a multiple of the size of VkDeviceSize",
                             stride);
        } else if (queryType == VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SIZE_KHR) {
            skip |= LogError(device, "VUID-vkWriteAccelerationStructuresPropertiesKHR-queryType-06731",
                             "vkWriteAccelerationStructuresPropertiesKHR: If queryType is "
                             "VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SIZE_KHR,"
                             "then stride (%zu) must be a multiple of the size of VkDeviceSize",
                             stride);
        } else if (queryType == VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_BOTTOM_LEVEL_POINTERS_KHR) {
            skip |= LogError(device, "VUID-vkWriteAccelerationStructuresPropertiesKHR-queryType-06733",
                             "vkWriteAccelerationStructuresPropertiesKHR: If queryType is "
                             "VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_BOTTOM_LEVEL_POINTERS_KHR,"
                             "then stride (%zu) must be a multiple of the size of VkDeviceSize",
                             stride);
        }
    }
    return skip;
}
bool StatelessValidation::manual_PreCallValidateGetRayTracingCaptureReplayShaderGroupHandlesKHR(
    VkDevice device, VkPipeline pipeline, uint32_t firstGroup, uint32_t groupCount, size_t dataSize, void *pData) const {
    bool skip = false;
    const auto *raytracing_features = LvlFindInChain<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>(device_createinfo_pnext);
    if (!raytracing_features || raytracing_features->rayTracingPipelineShaderGroupHandleCaptureReplay == VK_FALSE) {
        skip |= LogError(
            device, "VUID-vkGetRayTracingCaptureReplayShaderGroupHandlesKHR-rayTracingPipelineShaderGroupHandleCaptureReplay-03606",
            "vkGetRayTracingCaptureReplayShaderGroupHandlesKHR:VkPhysicalDeviceRayTracingPipelineFeaturesKHR::"
            "rayTracingPipelineShaderGroupHandleCaptureReplay must be enabled to call this function.");
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdTraceRaysKHR(VkCommandBuffer commandBuffer,
                                                                const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable,
                                                                const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable,
                                                                const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable,
                                                                const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable,
                                                                uint32_t width, uint32_t height, uint32_t depth) const {
    bool skip = false;
    // RayGen
    if (pRaygenShaderBindingTable->size != pRaygenShaderBindingTable->stride) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysKHR-size-04023",
                         "vkCmdTraceRaysKHR: The size member of pRayGenShaderBindingTable must be equal to its stride member");
    }
    if (SafeModulo(pRaygenShaderBindingTable->deviceAddress, phys_dev_ext_props.ray_tracing_props_khr.shaderGroupBaseAlignment) !=
        0) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysKHR-pRayGenShaderBindingTable-03682",
                         "vkCmdTraceRaysKHR: pRaygenShaderBindingTable->deviceAddress must be a multiple of "
                         "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::shaderGroupBaseAlignment.");
    }
    // Callable
    if (SafeModulo(pCallableShaderBindingTable->stride, phys_dev_ext_props.ray_tracing_props_khr.shaderGroupHandleAlignment) != 0) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysKHR-stride-03694",
                         "vkCmdTraceRaysKHR: The stride member of pCallableShaderBindingTable must be a multiple of "
                         "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::shaderGroupHandleAlignment.");
    }
    if (pCallableShaderBindingTable->stride > phys_dev_ext_props.ray_tracing_props_khr.maxShaderGroupStride) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysKHR-stride-04041",
                         "vkCmdTraceRaysKHR: The stride member of pCallableShaderBindingTable must be"
                         "less than or equal to VkPhysicalDeviceRayTracingPipelinePropertiesKHR::maxShaderGroupStride.");
    }
    if (SafeModulo(pCallableShaderBindingTable->deviceAddress, phys_dev_ext_props.ray_tracing_props_khr.shaderGroupBaseAlignment) !=
        0) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysKHR-pCallableShaderBindingTable-03693",
                         "vkCmdTraceRaysKHR: pCallableShaderBindingTable->deviceAddress must be a multiple of "
                         "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::shaderGroupBaseAlignment.");
    }
    // hitShader
    if (SafeModulo(pHitShaderBindingTable->stride, phys_dev_ext_props.ray_tracing_props_khr.shaderGroupHandleAlignment) != 0) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysKHR-stride-03690",
                         "vkCmdTraceRaysKHR: The stride member of pHitShaderBindingTable must be a multiple of "
                         "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::shaderGroupHandleAlignment.");
    }
    if (pHitShaderBindingTable->stride > phys_dev_ext_props.ray_tracing_props_khr.maxShaderGroupStride) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysKHR-stride-04035",
                         "vkCmdTraceRaysKHR: TThe stride member of pHitShaderBindingTable must be less than or equal to "
                         "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::maxShaderGroupStride");
    }
    if (SafeModulo(pHitShaderBindingTable->deviceAddress, phys_dev_ext_props.ray_tracing_props_khr.shaderGroupBaseAlignment) != 0) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysKHR-pHitShaderBindingTable-03689",
                         "vkCmdTraceRaysKHR: pHitShaderBindingTable->deviceAddress must be a multiple of "
                         "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::shaderGroupBaseAlignment.");
    }
    // missShader
    if (SafeModulo(pMissShaderBindingTable->stride, phys_dev_ext_props.ray_tracing_props_khr.shaderGroupHandleAlignment) != 0) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysKHR-stride-03686",
                         "vkCmdTraceRaysKHR: The stride member of pMissShaderBindingTable must be a multiple of "
                         "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::shaderGroupHandleAlignment");
    }
    if (pMissShaderBindingTable->stride > phys_dev_ext_props.ray_tracing_props_khr.maxShaderGroupStride) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysKHR-stride-04029",
                         "vkCmdTraceRaysKHR: The stride member of pMissShaderBindingTable must be"
                         "less than or equal to VkPhysicalDeviceRayTracingPipelinePropertiesKHR::maxShaderGroupStride.");
    }
    if (SafeModulo(pMissShaderBindingTable->deviceAddress, phys_dev_ext_props.ray_tracing_props_khr.shaderGroupBaseAlignment) !=
        0) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysKHR-pMissShaderBindingTable-03685",
                         "vkCmdTraceRaysKHR: pMissShaderBindingTable->deviceAddress must be a multiple of "
                         "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::shaderGroupBaseAlignment.");
    }
    if (width * depth * height > phys_dev_ext_props.ray_tracing_props_khr.maxRayDispatchInvocationCount) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysKHR-width-03641",
                         "vkCmdTraceRaysKHR: width {times} height {times} depth must be less than or equal to "
                         "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::maxRayDispatchInvocationCount");
    }
    if (width > device_limits.maxComputeWorkGroupCount[0] * device_limits.maxComputeWorkGroupSize[0]) {
        skip |=
            LogError(device, "VUID-vkCmdTraceRaysKHR-width-03638",
                     "vkCmdTraceRaysKHR: width must be less than or equal to VkPhysicalDeviceLimits::maxComputeWorkGroupCount[0] "
                     "{times} VkPhysicalDeviceLimits::maxComputeWorkGroupSize[0]");
    }

    if (height > device_limits.maxComputeWorkGroupCount[1] * device_limits.maxComputeWorkGroupSize[1]) {
        skip |=
            LogError(device, "VUID-vkCmdTraceRaysKHR-height-03639",
                     "vkCmdTraceRaysKHR: height must be less than or equal to VkPhysicalDeviceLimits::maxComputeWorkGroupCount[1] "
                     "{times} VkPhysicalDeviceLimits::maxComputeWorkGroupSize[1]");
    }

    if (depth > device_limits.maxComputeWorkGroupCount[2] * device_limits.maxComputeWorkGroupSize[2]) {
        skip |=
            LogError(device, "VUID-vkCmdTraceRaysKHR-depth-03640",
                     "vkCmdTraceRaysKHR: depth must be less than or equal to VkPhysicalDeviceLimits::maxComputeWorkGroupCount[2] "
                     "{times} VkPhysicalDeviceLimits::maxComputeWorkGroupSize[2]");
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdTraceRaysIndirectKHR(
    VkCommandBuffer commandBuffer, const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable,
    const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable, const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable,
    const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable, VkDeviceAddress indirectDeviceAddress) const {
    bool skip = false;
    const auto *raytracing_features = LvlFindInChain<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>(device_createinfo_pnext);
    if (!raytracing_features || raytracing_features->rayTracingPipelineTraceRaysIndirect == VK_FALSE) {
        skip |= LogError(
            device, "VUID-vkCmdTraceRaysIndirectKHR-rayTracingPipelineTraceRaysIndirect-03637",
            "vkCmdTraceRaysIndirectKHR: the VkPhysicalDeviceRayTracingPipelineFeaturesKHR::rayTracingPipelineTraceRaysIndirect "
            "feature must be enabled.");
    }
    // RayGen
    if (pRaygenShaderBindingTable->size != pRaygenShaderBindingTable->stride) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysIndirectKHR-size-04023",
                         "vkCmdTraceRaysKHR: The size member of pRayGenShaderBindingTable must be equal to its stride member");
    }
    if (SafeModulo(pRaygenShaderBindingTable->deviceAddress, phys_dev_ext_props.ray_tracing_props_khr.shaderGroupBaseAlignment) !=
        0) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysIndirectKHR-pRayGenShaderBindingTable-03682",
                         "vkCmdTraceRaysIndirectKHR: pRaygenShaderBindingTable->deviceAddress must be a multiple of "
                         "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::shaderGroupBaseAlignment.");
    }
    // Callabe
    if (SafeModulo(pCallableShaderBindingTable->stride, phys_dev_ext_props.ray_tracing_props_khr.shaderGroupHandleAlignment) != 0) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysIndirectKHR-stride-03694",
                         "vkCmdTraceRaysIndirectKHR: The stride member of pCallableShaderBindingTable must be a multiple of "
                         "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::shaderGroupHandleAlignment.");
    }
    if (pCallableShaderBindingTable->stride > phys_dev_ext_props.ray_tracing_props_khr.maxShaderGroupStride) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysIndirectKHR-stride-04041",
                         "vkCmdTraceRaysIndirectKHR: The stride member of pCallableShaderBindingTable must be less than or equal "
                         "to VkPhysicalDeviceRayTracingPipelinePropertiesKHR::maxShaderGroupStride.");
    }
    if (SafeModulo(pCallableShaderBindingTable->deviceAddress, phys_dev_ext_props.ray_tracing_props_khr.shaderGroupBaseAlignment) !=
        0) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysIndirectKHR-pCallableShaderBindingTable-03693",
                         "vkCmdTraceRaysIndirectKHR: pCallableShaderBindingTable->deviceAddress must be a multiple of "
                         "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::shaderGroupBaseAlignment.");
    }
    // hitShader
    if (SafeModulo(pHitShaderBindingTable->stride, phys_dev_ext_props.ray_tracing_props_khr.shaderGroupHandleAlignment) != 0) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysIndirectKHR-stride-03690",
                         "vkCmdTraceRaysIndirectKHR: The stride member of pHitShaderBindingTable must be a multiple of "
                         "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::shaderGroupHandleAlignment.");
    }
    if (pHitShaderBindingTable->stride > phys_dev_ext_props.ray_tracing_props_khr.maxShaderGroupStride) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysIndirectKHR-stride-04035",
                         "vkCmdTraceRaysIndirectKHR: The stride member of pHitShaderBindingTable must be less than or equal to "
                         "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::maxShaderGroupStride.");
    }
    if (SafeModulo(pHitShaderBindingTable->deviceAddress, phys_dev_ext_props.ray_tracing_props_khr.shaderGroupBaseAlignment) != 0) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysIndirectKHR-pHitShaderBindingTable-03689",
                         "vkCmdTraceRaysIndirectKHR: pHitShaderBindingTable->deviceAddress must be a multiple of "
                         "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::shaderGroupBaseAlignment.");
    }
    // missShader
    if (SafeModulo(pMissShaderBindingTable->stride, phys_dev_ext_props.ray_tracing_props_khr.shaderGroupHandleAlignment) != 0) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysIndirectKHR-stride-03686",
                         "vkCmdTraceRaysIndirectKHR:The stride member of pMissShaderBindingTable must be a multiple of "
                         "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::shaderGroupHandleAlignment.");
    }
    if (pMissShaderBindingTable->stride > phys_dev_ext_props.ray_tracing_props_khr.maxShaderGroupStride) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysIndirectKHR-stride-04029",
                         "vkCmdTraceRaysIndirectKHR: The stride member of pMissShaderBindingTable must be less than or equal to "
                         "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::maxShaderGroupStride.");
    }
    if (SafeModulo(pMissShaderBindingTable->deviceAddress, phys_dev_ext_props.ray_tracing_props_khr.shaderGroupBaseAlignment) !=
        0) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysIndirectKHR-pMissShaderBindingTable-03685",
                         "vkCmdTraceRaysIndirectKHR: pMissShaderBindingTable->deviceAddress must be a multiple of "
                         "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::shaderGroupBaseAlignment.");
    }

    if (SafeModulo(indirectDeviceAddress, 4) != 0) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysIndirectKHR-indirectDeviceAddress-03634",
                         "vkCmdTraceRaysIndirectKHR: indirectDeviceAddress must be a multiple of 4.");
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdTraceRaysIndirect2KHR(VkCommandBuffer commandBuffer,
                                                                         VkDeviceAddress indirectDeviceAddress) const {
    bool skip = false;
    const auto *raytracing_features = LvlFindInChain<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>(device_createinfo_pnext);
    if (!raytracing_features || raytracing_features->rayTracingPipelineTraceRaysIndirect == VK_FALSE) {
        skip |= LogError(
            device, "VUID-vkCmdTraceRaysIndirect2KHR-rayTracingPipelineTraceRaysIndirect2-03637",
            "vkCmdTraceRaysIndirect2KHR: the VkPhysicalDeviceRayTracingPipelineFeaturesKHR::rayTracingPipelineTraceRaysIndirect "
            "feature must be enabled.");
    }

    if (SafeModulo(indirectDeviceAddress, 4) != 0) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysIndirect2KHR-indirectDeviceAddress-03634",
                         "vkCmdTraceRaysIndirect2KHR: indirectDeviceAddress must be a multiple of 4.");
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
    if (SafeModulo(callableShaderBindingOffset, phys_dev_ext_props.ray_tracing_props_nv.shaderGroupBaseAlignment) != 0) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysNV-callableShaderBindingOffset-02462",
                         "vkCmdTraceRaysNV: callableShaderBindingOffset must be a multiple of "
                         "VkPhysicalDeviceRayTracingPropertiesNV::shaderGroupBaseAlignment.");
    }
    if (SafeModulo(callableShaderBindingStride, phys_dev_ext_props.ray_tracing_props_nv.shaderGroupHandleSize) != 0) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysNV-callableShaderBindingStride-02465",
                         "vkCmdTraceRaysNV: callableShaderBindingStride must be a multiple of "
                         "VkPhysicalDeviceRayTracingPropertiesNV::shaderGroupHandleSize.");
    }
    if (callableShaderBindingStride > phys_dev_ext_props.ray_tracing_props_nv.maxShaderGroupStride) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysNV-callableShaderBindingStride-02468",
                         "vkCmdTraceRaysNV: callableShaderBindingStride must be less than or equal to "
                         "VkPhysicalDeviceRayTracingPropertiesNV::maxShaderGroupStride. ");
    }

    // hitShader
    if (SafeModulo(hitShaderBindingOffset, phys_dev_ext_props.ray_tracing_props_nv.shaderGroupBaseAlignment) != 0) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysNV-hitShaderBindingOffset-02460",
                         "vkCmdTraceRaysNV: hitShaderBindingOffset must be a multiple of "
                         "VkPhysicalDeviceRayTracingPropertiesNV::shaderGroupBaseAlignment.");
    }
    if (SafeModulo(hitShaderBindingStride, phys_dev_ext_props.ray_tracing_props_nv.shaderGroupHandleSize) != 0) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysNV-hitShaderBindingStride-02464",
                         "vkCmdTraceRaysNV: hitShaderBindingStride must be a multiple of "
                         "VkPhysicalDeviceRayTracingPropertiesNV::shaderGroupHandleSize.");
    }
    if (hitShaderBindingStride > phys_dev_ext_props.ray_tracing_props_nv.maxShaderGroupStride) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysNV-hitShaderBindingStride-02467",
                         "vkCmdTraceRaysNV: hitShaderBindingStride must be less than or equal to "
                         "VkPhysicalDeviceRayTracingPropertiesNV::maxShaderGroupStride.");
    }

    // missShader
    if (SafeModulo(missShaderBindingOffset, phys_dev_ext_props.ray_tracing_props_nv.shaderGroupBaseAlignment) != 0) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysNV-missShaderBindingOffset-02458",
                         "vkCmdTraceRaysNV: missShaderBindingOffset must be a multiple of "
                         "VkPhysicalDeviceRayTracingPropertiesNV::shaderGroupBaseAlignment.");
    }
    if (SafeModulo(missShaderBindingStride, phys_dev_ext_props.ray_tracing_props_nv.shaderGroupHandleSize) != 0) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysNV-missShaderBindingStride-02463",
                         "vkCmdTraceRaysNV: missShaderBindingStride must be a multiple of "
                         "VkPhysicalDeviceRayTracingPropertiesNV::shaderGroupHandleSize.");
    }
    if (missShaderBindingStride > phys_dev_ext_props.ray_tracing_props_nv.maxShaderGroupStride) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysNV-missShaderBindingStride-02466",
                         "vkCmdTraceRaysNV: missShaderBindingStride must be less than or equal to "
                         "VkPhysicalDeviceRayTracingPropertiesNV::maxShaderGroupStride.");
    }

    // raygenShader
    if (SafeModulo(raygenShaderBindingOffset, phys_dev_ext_props.ray_tracing_props_nv.shaderGroupBaseAlignment) != 0) {
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

bool StatelessValidation::manual_PreCallValidateGetDeviceAccelerationStructureCompatibilityKHR(
    VkDevice device, const VkAccelerationStructureVersionInfoKHR *pVersionInfo,
    VkAccelerationStructureCompatibilityKHR *pCompatibility) const {
    bool skip = false;
    const auto *ray_query_features = LvlFindInChain<VkPhysicalDeviceRayQueryFeaturesKHR>(device_createinfo_pnext);
    const auto *raytracing_features = LvlFindInChain<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>(device_createinfo_pnext);
    if ((!raytracing_features && !ray_query_features) || ((ray_query_features && !(ray_query_features->rayQuery)) ||
                                                          (raytracing_features && !raytracing_features->rayTracingPipeline))) {
        skip |= LogError(device, "VUID-vkGetDeviceAccelerationStructureCompatibilityKHR-rayTracingPipeline-03661",
                         "vkGetDeviceAccelerationStructureCompatibilityKHR: The rayTracing or rayQuery feature must be enabled.");
    }
    return skip;
}

bool StatelessValidation::ValidateCmdSetViewportWithCount(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                                          const VkViewport *pViewports, CMD_TYPE cmd_type) const {
    bool skip = false;
    const char *api_call = CommandTypeString(cmd_type);

    if (!physical_device_features.multiViewport) {
        if (viewportCount != 1) {
            skip |= LogError(commandBuffer, "VUID-vkCmdSetViewportWithCount-viewportCount-03395",
                             "%s: The multiViewport feature is disabled, but viewportCount (=%" PRIu32 ") is not 1.", api_call,
                             viewportCount);
        }
    } else {  // multiViewport enabled
        if (viewportCount < 1 || viewportCount > device_limits.maxViewports) {
            skip |= LogError(commandBuffer, "VUID-vkCmdSetViewportWithCount-viewportCount-03394",
                             "%s:  viewportCount (=%" PRIu32
                             ") must "
                             "not be greater than VkPhysicalDeviceLimits::maxViewports (=%" PRIu32 ").",
                             api_call, viewportCount, device_limits.maxViewports);
        }
    }

    if (pViewports) {
        for (uint32_t viewport_i = 0; viewport_i < viewportCount; ++viewport_i) {
            const auto &viewport = pViewports[viewport_i];  // will crash on invalid ptr
            skip |= manual_PreCallValidateViewport(
                viewport, api_call, ParameterName("pViewports[%i]", ParameterName::IndexVector{viewport_i}), commandBuffer);
        }
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdSetViewportWithCountEXT(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                                                           const VkViewport *pViewports) const {
    bool skip = false;
    skip = ValidateCmdSetViewportWithCount(commandBuffer, viewportCount, pViewports, CMD_SETVIEWPORTWITHCOUNTEXT);
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdSetViewportWithCount(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                                                        const VkViewport *pViewports) const {
    bool skip = false;
    skip = ValidateCmdSetViewportWithCount(commandBuffer, viewportCount, pViewports, CMD_SETVIEWPORTWITHCOUNT);
    return skip;
}

bool StatelessValidation::ValidateCmdSetScissorWithCount(VkCommandBuffer commandBuffer, uint32_t scissorCount,
                                                         const VkRect2D *pScissors, CMD_TYPE cmd_type) const {
    bool skip = false;
    const char *api_call = CommandTypeString(cmd_type);

    if (!physical_device_features.multiViewport) {
        if (scissorCount != 1) {
            skip |= LogError(commandBuffer, "VUID-vkCmdSetScissorWithCount-scissorCount-03398",
                             "%s: scissorCount (=%" PRIu32
                             ") must "
                             "be 1 when the multiViewport feature is disabled.",
                             api_call, scissorCount);
        }
    } else {  // multiViewport enabled
        if (scissorCount == 0) {
            skip |= LogError(commandBuffer, "VUID-vkCmdSetScissorWithCount-scissorCount-03397",
                             "%s: scissorCount (=%" PRIu32
                             ") must "
                             "be great than zero.",
                             api_call, scissorCount);
        } else if (scissorCount > device_limits.maxViewports) {
            skip |= LogError(commandBuffer, "VUID-vkCmdSetScissorWithCount-scissorCount-03397",
                             "%s: scissorCount (=%" PRIu32
                             ") must "
                             "not be greater than VkPhysicalDeviceLimits::maxViewports (=%" PRIu32 ").",
                             api_call, scissorCount, device_limits.maxViewports);
        }
    }

    if (pScissors) {
        for (uint32_t scissor_i = 0; scissor_i < scissorCount; ++scissor_i) {
            const auto &scissor = pScissors[scissor_i];  // will crash on invalid ptr

            if (scissor.offset.x < 0) {
                skip |= LogError(commandBuffer, "VUID-vkCmdSetScissorWithCount-x-03399",
                                 "%s: pScissors[%" PRIu32 "].offset.x (=%" PRIi32 ") is negative.", api_call, scissor_i,
                                 scissor.offset.x);
            }

            if (scissor.offset.y < 0) {
                skip |= LogError(commandBuffer, "VUID-vkCmdSetScissorWithCount-x-03399",
                                 "%s: pScissors[%" PRIu32 "].offset.y (=%" PRIi32 ") is negative.", api_call, scissor_i,
                                 scissor.offset.y);
            }

            const int64_t x_sum = static_cast<int64_t>(scissor.offset.x) + static_cast<int64_t>(scissor.extent.width);
            if (x_sum > vvl::kI32Max) {
                skip |= LogError(commandBuffer, "VUID-vkCmdSetScissorWithCount-offset-03400",
                                 "%s: offset.x + extent.width (=%" PRIi32 " + %" PRIu32 " = %" PRIi64 ") of pScissors[%" PRIu32
                                 "] will overflow int32_t.",
                                 api_call, scissor.offset.x, scissor.extent.width, x_sum, scissor_i);
            }

            const int64_t y_sum = static_cast<int64_t>(scissor.offset.y) + static_cast<int64_t>(scissor.extent.height);
            if (y_sum > vvl::kI32Max) {
                skip |= LogError(commandBuffer, "VUID-vkCmdSetScissorWithCount-offset-03401",
                                 "%s: offset.y + extent.height (=%" PRIi32 " + %" PRIu32 " = %" PRIi64 ") of pScissors[%" PRIu32
                                 "] will overflow int32_t.",
                                 api_call, scissor.offset.y, scissor.extent.height, y_sum, scissor_i);
            }
        }
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdSetScissorWithCountEXT(VkCommandBuffer commandBuffer, uint32_t scissorCount,
                                                                          const VkRect2D *pScissors) const {
    bool skip = false;
    skip = ValidateCmdSetScissorWithCount(commandBuffer, scissorCount, pScissors, CMD_SETSCISSORWITHCOUNTEXT);
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdSetScissorWithCount(VkCommandBuffer commandBuffer, uint32_t scissorCount,
                                                                       const VkRect2D *pScissors) const {
    bool skip = false;
    skip = ValidateCmdSetScissorWithCount(commandBuffer, scissorCount, pScissors, CMD_SETSCISSORWITHCOUNT);
    return skip;
}

bool StatelessValidation::ValidateCmdBindVertexBuffers2(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                                        const VkBuffer *pBuffers, const VkDeviceSize *pOffsets,
                                                        const VkDeviceSize *pSizes, const VkDeviceSize *pStrides,
                                                        CMD_TYPE cmd_type) const {
    bool skip = false;
    const char *api_call = CommandTypeString(cmd_type);

    // Check VUID-vkCmdBindVertexBuffers2-bindingCount-arraylength
    {
        const bool vuidCondition = (pSizes != nullptr) || (pStrides != nullptr);
        const bool vuidExpectation = bindingCount > 0;
        if (vuidCondition) {
            if (!vuidExpectation) {
                const char *not_null_msg = "";
                if ((pSizes != nullptr) && (pStrides != nullptr))
                    not_null_msg = "pSizes and pStrides are not NULL";
                else if (pSizes != nullptr)
                    not_null_msg = "pSizes is not NULL";
                else
                    not_null_msg = "pStrides is not NULL";
                const char *vuid_breach_msg = "%s: %s, so bindingCount must be greater that 0.";
                skip |= LogError(commandBuffer, "VUID-vkCmdBindVertexBuffers2-bindingCount-arraylength", vuid_breach_msg, api_call,
                                 not_null_msg);
            }
        }
    }

    if (firstBinding >= device_limits.maxVertexInputBindings) {
        skip |= LogError(commandBuffer, "VUID-vkCmdBindVertexBuffers2-firstBinding-03355",
                         "%s firstBinding (%" PRIu32 ") must be less than maxVertexInputBindings (%" PRIu32 ")", api_call,
                         firstBinding, device_limits.maxVertexInputBindings);
    } else if ((firstBinding + bindingCount) > device_limits.maxVertexInputBindings) {
        skip |= LogError(commandBuffer, "VUID-vkCmdBindVertexBuffers2-firstBinding-03356",
                         "%s sum of firstBinding (%" PRIu32 ") and bindingCount (%" PRIu32
                         ") must be less than "
                         "maxVertexInputBindings (%" PRIu32 ")",
                         api_call, firstBinding, bindingCount, device_limits.maxVertexInputBindings);
    }

    for (uint32_t i = 0; i < bindingCount; ++i) {
        if (pBuffers[i] == VK_NULL_HANDLE) {
            const auto *robustness2_features = LvlFindInChain<VkPhysicalDeviceRobustness2FeaturesEXT>(device_createinfo_pnext);
            if (!(robustness2_features && robustness2_features->nullDescriptor)) {
                skip |= LogError(commandBuffer, "VUID-vkCmdBindVertexBuffers2-pBuffers-04111",
                                 "%s required parameter pBuffers[%" PRIu32 "] specified as VK_NULL_HANDLE", api_call, i);
            } else {
                if (pOffsets[i] != 0) {
                    skip |=
                        LogError(commandBuffer, "VUID-vkCmdBindVertexBuffers2-pBuffers-04112",
                                 "%s pBuffers[%" PRIu32 "] is VK_NULL_HANDLE, but pOffsets[%" PRIu32 "] is not 0", api_call, i, i);
                }
            }
        }
        if (pStrides) {
            if (pStrides[i] > device_limits.maxVertexInputBindingStride) {
                skip |=
                    LogError(commandBuffer, "VUID-vkCmdBindVertexBuffers2-pStrides-03362",
                             "%s pStrides[%" PRIu32 "] (%" PRIu64 ") must be less than maxVertexInputBindingStride (%" PRIu32 ")",
                             api_call, i, pStrides[i], device_limits.maxVertexInputBindingStride);
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
    skip = ValidateCmdBindVertexBuffers2(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes, pStrides,
                                         CMD_BINDVERTEXBUFFERS2EXT);
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdBindVertexBuffers2(VkCommandBuffer commandBuffer, uint32_t firstBinding,
                                                                      uint32_t bindingCount, const VkBuffer *pBuffers,
                                                                      const VkDeviceSize *pOffsets, const VkDeviceSize *pSizes,
                                                                      const VkDeviceSize *pStrides) const {
    bool skip = false;
    skip = ValidateCmdBindVertexBuffers2(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes, pStrides,
                                         CMD_BINDVERTEXBUFFERS2);
    return skip;
}

bool StatelessValidation::ValidateAccelerationStructureBuildGeometryInfoKHR(
    const VkAccelerationStructureBuildGeometryInfoKHR *pInfos, uint32_t infoCount, uint64_t total_primitive_count,
    const char *api_name) const {
    bool skip = false;
    for (uint32_t i = 0; i < infoCount; ++i) {
        if (pInfos[i].type == VK_ACCELERATION_STRUCTURE_TYPE_GENERIC_KHR) {
            skip |= LogError(device, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-03654",
                             "%s(): type must not be VK_ACCELERATION_STRUCTURE_TYPE_GENERIC_KHR.", api_name);
        }
        if (pInfos[i].flags & VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR &&
            pInfos[i].flags & VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR) {
            skip |= LogError(device, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-flags-03796",
                             "%s(): If flags has the VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR bit set,"
                             "then it must not have the VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR bit set.",
                             api_name);
        }
        if (pInfos[i].pGeometries && pInfos[i].ppGeometries) {
            skip |=
                LogError(device, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-pGeometries-03788",
                         "%s(): Only one of pGeometries or ppGeometries can be a valid pointer, the other must be NULL", api_name);
        }
        if (pInfos[i].type == VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR && pInfos[i].geometryCount != 1) {
            skip |= LogError(device, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-03790",
                             "%s(): If type is VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR, geometryCount must be 1", api_name);
        }
        if (pInfos[i].type == VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR &&
            pInfos[i].geometryCount > phys_dev_ext_props.acc_structure_props.maxGeometryCount) {
            skip |= LogError(device, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-03793",
                             "%s(): If type is VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR then geometryCount must be"
                             " less than or equal to VkPhysicalDeviceAccelerationStructurePropertiesKHR::maxGeometryCount",
                             api_name);
        }

        const VkAccelerationStructureGeometryKHR *pGeometry = pInfos[i].pGeometries    ? pInfos[i].pGeometries
                                                              : pInfos[i].ppGeometries ? pInfos[i].ppGeometries[0]
                                                                                       : nullptr;
        if (pGeometry) {
            if (total_primitive_count > phys_dev_ext_props.acc_structure_props.maxPrimitiveCount) {
                switch (pGeometry->geometryType) {
                    case VK_GEOMETRY_TYPE_TRIANGLES_KHR:
                        skip |= LogError(device, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-03795",
                                         "%s(): total number of triangles in all geometries (%" PRIu64
                                         ") is superior to VkPhysicalDeviceAccelerationStructurePropertiesKHR::maxPrimitiveCount "
                                         "(%" PRIu64 ")",
                                         api_name, total_primitive_count, phys_dev_ext_props.acc_structure_props.maxPrimitiveCount);
                        break;
                    case VK_GEOMETRY_TYPE_AABBS_KHR:
                        skip |= LogError(device, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-03794",
                                         "%s(): total number of AABBs in all geometries (%" PRIu64
                                         ") is superior to VkPhysicalDeviceAccelerationStructurePropertiesKHR::maxPrimitiveCount "
                                         "(%" PRIu64 ")",
                                         api_name, total_primitive_count, phys_dev_ext_props.acc_structure_props.maxPrimitiveCount);
                        break;
                    default:
                        break;
                }
            }
        }

        if (pInfos[i].pGeometries) {
            for (uint32_t j = 0; j < pInfos[i].geometryCount; ++j) {
                skip |= ValidateRangedEnum(
                    api_name, ParameterName("pInfos[%i].pGeometries[%i].geometryType", ParameterName::IndexVector{i, j}),
                    "VkGeometryTypeKHR", pInfos[i].pGeometries[j].geometryType,
                    "VUID-VkAccelerationStructureGeometryKHR-geometryType-parameter");
                if (pInfos[i].pGeometries[j].geometryType == VK_GEOMETRY_TYPE_TRIANGLES_KHR) {
                    constexpr std::array allowed_structs = {
                        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_TRIANGLES_OPACITY_MICROMAP_EXT};

                    skip |= ValidateStructType(
                        api_name, ParameterName("pInfos[%i].pGeometries[%i].geometry.triangles", ParameterName::IndexVector{i, j}),
                        "VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR",
                        &(pInfos[i].pGeometries[j].geometry.triangles),
                        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR, false, kVUIDUndefined,
                        "VUID-VkAccelerationStructureGeometryTrianglesDataKHR-sType-sType");
                    skip |= ValidateStructPnext(
                        api_name,
                        ParameterName("pInfos[%i].pGeometries[%i].geometry.triangles.pNext", ParameterName::IndexVector{i, j}),
                        NULL, pInfos[i].pGeometries[j].geometry.triangles.pNext, allowed_structs.size(), allowed_structs.data(),
                        GeneratedVulkanHeaderVersion, "VUID-VkAccelerationStructureGeometryTrianglesDataKHR-pNext-pNext",
                        kVUIDUndefined);
                    skip |= ValidateRangedEnum(api_name,
                                               ParameterName("pInfos[%i].pGeometries[%i].geometry.triangles.vertexFormat",
                                                             ParameterName::IndexVector{i, j}),
                                               "VkFormat", pInfos[i].pGeometries[j].geometry.triangles.vertexFormat,
                                               "VUID-VkAccelerationStructureGeometryTrianglesDataKHR-vertexFormat-parameter");
                    skip |= ValidateStructType(api_name, "pInfos[i].pGeometries[j].geometry.triangles",
                                               "VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR",
                                               &pInfos[i].pGeometries[j].geometry.triangles,
                                               VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR, true,
                                               "VUID-VkAccelerationStructureGeometryKHR-triangles-parameter", kVUIDUndefined);
                    skip |= ValidateRangedEnum(
                        api_name,
                        ParameterName("pInfos[%i].pGeometries[%i].geometry.triangles.indexType", ParameterName::IndexVector{i, j}),
                        "VkIndexType", pInfos[i].pGeometries[j].geometry.triangles.indexType,
                        "VUID-VkAccelerationStructureGeometryTrianglesDataKHR-indexType-parameter");

                    if (pInfos[i].pGeometries[j].geometry.triangles.vertexStride > vvl::kU32Max) {
                        skip |= LogError(device, "VUID-VkAccelerationStructureGeometryTrianglesDataKHR-vertexStride-03819",
                                         "%s():vertexStride must be less than or equal to 2^32-1", api_name);
                    }
                    if (pInfos[i].pGeometries[j].geometry.triangles.indexType != VK_INDEX_TYPE_UINT16 &&
                        pInfos[i].pGeometries[j].geometry.triangles.indexType != VK_INDEX_TYPE_UINT32 &&
                        pInfos[i].pGeometries[j].geometry.triangles.indexType != VK_INDEX_TYPE_NONE_KHR) {
                        skip |=
                            LogError(device, "VUID-VkAccelerationStructureGeometryTrianglesDataKHR-indexType-03798",
                                     "%s():indexType must be VK_INDEX_TYPE_UINT16, VK_INDEX_TYPE_UINT32, or VK_INDEX_TYPE_NONE_KHR",
                                     api_name);
                    }
                }
                if (pInfos[i].pGeometries[j].geometryType == VK_GEOMETRY_TYPE_INSTANCES_KHR) {
                    skip |= ValidateStructType(api_name, "pInfos[i].pGeometries[j].geometry.instances",
                                               "VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR",
                                               &pInfos[i].pGeometries[j].geometry.instances,
                                               VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR, true,
                                               "VUID-VkAccelerationStructureGeometryKHR-instances-parameter", kVUIDUndefined);
                    skip |= ValidateStructType(
                        api_name, ParameterName("pInfos[%i].pGeometries[%i].geometry.instances", ParameterName::IndexVector{i, j}),
                        "VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR",
                        &(pInfos[i].pGeometries[j].geometry.instances),
                        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR, false, kVUIDUndefined,
                        "VUID-VkAccelerationStructureGeometryInstancesDataKHR-sType-sType");
                    skip |= ValidateStructPnext(
                        api_name,
                        ParameterName("pInfos[%i].pGeometries[%i].geometry.instances.pNext", ParameterName::IndexVector{i, j}),
                        NULL, pInfos[i].pGeometries[j].geometry.instances.pNext, 0, NULL, GeneratedVulkanHeaderVersion,
                        "VUID-VkAccelerationStructureGeometryInstancesDataKHR-pNext-pNext", kVUIDUndefined);

                    skip |= ValidateBool32(api_name,
                                           ParameterName("pInfos[%i].pGeometries[%i].geometry.instances.arrayOfPointers",
                                                         ParameterName::IndexVector{i, j}),
                                           pInfos[i].pGeometries[j].geometry.instances.arrayOfPointers);
                }
                if (pInfos[i].pGeometries[j].geometryType == VK_GEOMETRY_TYPE_AABBS_KHR) {
                    skip |= ValidateStructType(api_name, "pInfos[i].pGeometries[j].geometry.aabbs",
                                               "VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR",
                                               &pInfos[i].pGeometries[j].geometry.aabbs,
                                               VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR, true,
                                               "VUID-VkAccelerationStructureGeometryKHR-aabbs-parameter", kVUIDUndefined);
                    skip |= ValidateStructType(
                        api_name, ParameterName("pInfos[%i].pGeometries[%i].geometry.aabbs", ParameterName::IndexVector{i, j}),
                        "VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR",
                        &(pInfos[i].pGeometries[j].geometry.aabbs),
                        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR, false, kVUIDUndefined,
                        "VUID-VkAccelerationStructureGeometryAabbsDataKHR-sType-sType");
                    skip |= ValidateStructPnext(
                        api_name,
                        ParameterName("pInfos[%i].pGeometries[%i].geometry.aabbs.pNext", ParameterName::IndexVector{i, j}), NULL,
                        pInfos[i].pGeometries[j].geometry.aabbs.pNext, 0, NULL, GeneratedVulkanHeaderVersion,
                        "VUID-VkAccelerationStructureGeometryAabbsDataKHR-pNext-pNext", kVUIDUndefined);
                    if (pInfos[i].pGeometries[j].geometry.aabbs.stride % 8) {
                        skip |= LogError(device, "VUID-VkAccelerationStructureGeometryAabbsDataKHR-stride-03545",
                                         "%s(): stride is not a multiple of 8.", api_name);
                    }
                    if (pInfos[i].pGeometries[j].geometry.aabbs.stride > vvl::kU32Max) {
                        skip |= LogError(device, "VUID-VkAccelerationStructureGeometryAabbsDataKHR-stride-03820",
                                         "%s(): stride must be less than or equal to 2^32-1.", api_name);
                    }
                }
                if (pInfos[i].type == VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR &&
                    pInfos[i].pGeometries[j].geometryType != VK_GEOMETRY_TYPE_INSTANCES_KHR) {
                    skip |= LogError(device, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-03789",
                                     "%s(): If type is VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR, the geometryType member"
                                     " of elements of either pGeometries or ppGeometries must be VK_GEOMETRY_TYPE_INSTANCES_KHR.",
                                     api_name);
                }
                if (pInfos[i].type == VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR) {
                    if (pInfos[i].pGeometries[j].geometryType == VK_GEOMETRY_TYPE_INSTANCES_KHR) {
                        skip |= LogError(device, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-03791",
                                         "%s(): If type is VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR the geometryType member "
                                         "of elements of"
                                         " either pGeometries or ppGeometries must not be VK_GEOMETRY_TYPE_INSTANCES_KHR.",
                                         api_name);
                    }
                    if (pInfos[i].pGeometries[j].geometryType != pInfos[i].pGeometries[0].geometryType) {
                        skip |= LogError(device, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-03792",
                                         "%s(): If type is VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR then the geometryType"
                                         " member of each geometry in either pGeometries or ppGeometries must be the same.",
                                         api_name);
                    }
                }
            }
        }
        if (pInfos[i].ppGeometries != NULL) {
            for (uint32_t j = 0; j < pInfos[i].geometryCount; ++j) {
                skip |= ValidateRangedEnum(
                    api_name, ParameterName("pInfos[%i].ppGeometries[%i]->geometryType", ParameterName::IndexVector{i, j}),
                    "VkGeometryTypeKHR", pInfos[i].ppGeometries[j]->geometryType,
                    "VUID-VkAccelerationStructureGeometryKHR-geometryType-parameter");
                if (pInfos[i].ppGeometries[j]->geometryType == VK_GEOMETRY_TYPE_TRIANGLES_KHR) {
                    skip |= ValidateStructType(api_name, "pInfos[i].pGeometries[j].geometry.triangles",
                                               "VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR",
                                               &pInfos[i].ppGeometries[j]->geometry.triangles,
                                               VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR, true,
                                               "VUID-VkAccelerationStructureGeometryKHR-triangles-parameter", kVUIDUndefined);
                    skip |= ValidateStructType(
                        api_name,
                        ParameterName("pInfos[%i].ppGeometries[%i]->geometry.triangles", ParameterName::IndexVector{i, j}),
                        "VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR",
                        &(pInfos[i].ppGeometries[j]->geometry.triangles),
                        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR, false, kVUIDUndefined,
                        "VUID-VkAccelerationStructureGeometryTrianglesDataKHR-sType-sType");
                    skip |= ValidateStructPnext(
                        api_name,
                        ParameterName("pInfos[%i].ppGeometries[%i]->geometry.triangles.pNext", ParameterName::IndexVector{i, j}),
                        NULL, pInfos[i].ppGeometries[j]->geometry.triangles.pNext, 0, NULL, GeneratedVulkanHeaderVersion,
                        "VUID-VkAccelerationStructureGeometryTrianglesDataKHR-pNext-pNext", kVUIDUndefined);
                    skip |= ValidateRangedEnum(api_name,
                                               ParameterName("pInfos[%i].ppGeometries[%i]->geometry.triangles.vertexFormat",
                                                             ParameterName::IndexVector{i, j}),
                                               "VkFormat", pInfos[i].ppGeometries[j]->geometry.triangles.vertexFormat,
                                               "VUID-VkAccelerationStructureGeometryTrianglesDataKHR-vertexFormat-parameter");
                    skip |= ValidateRangedEnum(api_name,
                                               ParameterName("pInfos[%i].ppGeometries[%i]->geometry.triangles.indexType",
                                                             ParameterName::IndexVector{i, j}),
                                               "VkIndexType", pInfos[i].ppGeometries[j]->geometry.triangles.indexType,
                                               "VUID-VkAccelerationStructureGeometryTrianglesDataKHR-indexType-parameter");
                    if (pInfos[i].ppGeometries[j]->geometry.triangles.vertexStride > vvl::kU32Max) {
                        skip |= LogError(device, "VUID-VkAccelerationStructureGeometryTrianglesDataKHR-vertexStride-03819",
                                         "%s():vertexStride must be less than or equal to 2^32-1", api_name);
                    }
                    if (pInfos[i].ppGeometries[j]->geometry.triangles.indexType != VK_INDEX_TYPE_UINT16 &&
                        pInfos[i].ppGeometries[j]->geometry.triangles.indexType != VK_INDEX_TYPE_UINT32 &&
                        pInfos[i].ppGeometries[j]->geometry.triangles.indexType != VK_INDEX_TYPE_NONE_KHR) {
                        skip |=
                            LogError(device, "VUID-VkAccelerationStructureGeometryTrianglesDataKHR-indexType-03798",
                                     "%s():indexType must be VK_INDEX_TYPE_UINT16, VK_INDEX_TYPE_UINT32, or VK_INDEX_TYPE_NONE_KHR",
                                     api_name);
                    }
                }
                if (pInfos[i].ppGeometries[j]->geometryType == VK_GEOMETRY_TYPE_INSTANCES_KHR) {
                    skip |= ValidateStructType(api_name, "pInfos[i].pGeometries[j].geometry.instances",
                                               "VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR",
                                               &pInfos[i].ppGeometries[j]->geometry.instances,
                                               VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR, true,
                                               "VUID-VkAccelerationStructureGeometryKHR-instances-parameter", kVUIDUndefined);
                    skip |= ValidateStructType(
                        api_name,
                        ParameterName("pInfos[%i].ppGeometries[%i]->geometry.instances", ParameterName::IndexVector{i, j}),
                        "VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR",
                        &(pInfos[i].ppGeometries[j]->geometry.instances),
                        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR, false, kVUIDUndefined,
                        "VUID-VkAccelerationStructureGeometryInstancesDataKHR-sType-sType");
                    skip |= ValidateStructPnext(
                        api_name,
                        ParameterName("pInfos[%i].ppGeometries[%i]->geometry.instances.pNext", ParameterName::IndexVector{i, j}),
                        NULL, pInfos[i].ppGeometries[j]->geometry.instances.pNext, 0, NULL, GeneratedVulkanHeaderVersion,
                        "VUID-VkAccelerationStructureGeometryInstancesDataKHR-pNext-pNext", kVUIDUndefined);
                    skip |= ValidateBool32(api_name,
                                           ParameterName("pInfos[%i].ppGeometries[%i]->geometry.instances.arrayOfPointers",
                                                         ParameterName::IndexVector{i, j}),
                                           pInfos[i].ppGeometries[j]->geometry.instances.arrayOfPointers);
                }
                if (pInfos[i].ppGeometries[j]->geometryType == VK_GEOMETRY_TYPE_AABBS_KHR) {
                    skip |= ValidateStructType(api_name, "pInfos[i].pGeometries[j].geometry.aabbs",
                                               "VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR",
                                               &pInfos[i].ppGeometries[j]->geometry.aabbs,
                                               VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR, true,
                                               "VUID-VkAccelerationStructureGeometryKHR-aabbs-parameter", kVUIDUndefined);
                    skip |= ValidateStructType(
                        api_name, ParameterName("pInfos[%i].ppGeometries[%i]->geometry.aabbs", ParameterName::IndexVector{i, j}),
                        "VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR",
                        &(pInfos[i].ppGeometries[j]->geometry.aabbs),
                        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR, false, kVUIDUndefined,
                        "VUID-VkAccelerationStructureGeometryAabbsDataKHR-sType-sType");
                    skip |= ValidateStructPnext(
                        api_name,
                        ParameterName("pInfos[%i].ppGeometries[%i]->geometry.aabbs.pNext", ParameterName::IndexVector{i, j}), NULL,
                        pInfos[i].ppGeometries[j]->geometry.aabbs.pNext, 0, NULL, GeneratedVulkanHeaderVersion,
                        "VUID-VkAccelerationStructureGeometryAabbsDataKHR-pNext-pNext", kVUIDUndefined);
                    if (pInfos[i].ppGeometries[j]->geometry.aabbs.stride > vvl::kU32Max) {
                        skip |= LogError(device, "VUID-VkAccelerationStructureGeometryAabbsDataKHR-stride-03820",
                                         "%s():stride must be less than or equal to 2^32-1", api_name);
                    }
                    if (pInfos[i].ppGeometries[j]->geometry.aabbs.stride % 8) {
                        skip |= LogError(device, "VUID-VkAccelerationStructureGeometryAabbsDataKHR-stride-03545",
                                         "%s(): stride is not a multiple of 8.", api_name);
                    }
                }
                if (pInfos[i].type == VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR &&
                    pInfos[i].ppGeometries[j]->geometryType != VK_GEOMETRY_TYPE_INSTANCES_KHR) {
                    skip |= LogError(device, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-03789",
                                     "%s(): If type is VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR, the geometryType member"
                                     " of elements of either pGeometries or ppGeometries must be VK_GEOMETRY_TYPE_INSTANCES_KHR",
                                     api_name);
                }
                if (pInfos[i].type == VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR) {
                    if (pInfos[i].ppGeometries[j]->geometryType == VK_GEOMETRY_TYPE_INSTANCES_KHR) {
                        skip |= LogError(device, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-03791",
                                         "%s(): If type is VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR the geometryType member "
                                         "of elements of"
                                         " either pGeometries or ppGeometries must not be VK_GEOMETRY_TYPE_INSTANCES_KHR",
                                         api_name);
                    }
                    if (pInfos[i].ppGeometries[j]->geometryType != pInfos[i].ppGeometries[0]->geometryType) {
                        skip |= LogError(device, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-03792",
                                         "%s(): If type is VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR then the geometryType"
                                         " member of each geometry in either pGeometries or ppGeometries must be the same.",
                                         api_name);
                    }
                }
            }
        }
    }
    return skip;
}
bool StatelessValidation::manual_PreCallValidateCmdBuildAccelerationStructuresKHR(
    VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
    const VkAccelerationStructureBuildRangeInfoKHR *const *ppBuildRangeInfos) const {
    bool skip = false;
    skip |= ValidateAccelerationStructureBuildGeometryInfoKHR(pInfos, infoCount, 0, "vkCmdBuildAccelerationStructuresKHR");
    for (uint32_t i = 0; i < infoCount; ++i) {
        if (SafeModulo(pInfos[i].scratchData.deviceAddress,
                       phys_dev_ext_props.acc_structure_props.minAccelerationStructureScratchOffsetAlignment) != 0) {
            skip |= LogError(device, "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03710",
                             "vkCmdBuildAccelerationStructuresKHR:For each element of pInfos, its "
                             "scratchData.deviceAddress member must be a multiple of "
                             "VkPhysicalDeviceAccelerationStructurePropertiesKHR::minAccelerationStructureScratchOffsetAlignment.");
        }
        for (uint32_t k = 0; k < infoCount; ++k) {
            if (i == k) continue;
            bool found = false;
            if (pInfos[i].dstAccelerationStructure == pInfos[k].dstAccelerationStructure) {
                skip |=
                    LogError(device, "VUID-vkCmdBuildAccelerationStructuresKHR-dstAccelerationStructure-03698",
                             "vkCmdBuildAccelerationStructuresKHR:The dstAccelerationStructure member of any element (%" PRIu32
                             ") of pInfos must "
                             "not be "
                             "the same acceleration structure as the dstAccelerationStructure member of any other element (%" PRIu32
                             ") of pInfos.",
                             i, k);
                found = true;
            }
            if (pInfos[i].srcAccelerationStructure == pInfos[k].dstAccelerationStructure) {
                skip |=
                    LogError(device, "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03403",
                             "vkCmdBuildAccelerationStructuresKHR:The srcAccelerationStructure member of any element (%" PRIu32
                             ") of pInfos must "
                             "not be "
                             "the same acceleration structure as the dstAccelerationStructure member of any other element (%" PRIu32
                             ") of pInfos.",
                             i, k);
                found = true;
            }
            if (found) break;
        }
        for (uint32_t j = 0; j < pInfos[i].geometryCount; ++j) {
            if (pInfos[i].pGeometries) {
                if (pInfos[i].pGeometries[j].geometryType == VK_GEOMETRY_TYPE_INSTANCES_KHR) {
                    if (pInfos[i].pGeometries[j].geometry.instances.arrayOfPointers == VK_TRUE) {
                        if (SafeModulo(pInfos[i].pGeometries[j].geometry.instances.data.deviceAddress, 8) != 0) {
                            skip |= LogError(device, "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03716",
                                             "vkCmdBuildAccelerationStructuresKHR:For any element of pInfos[i].pGeometries with a "
                                             "geometryType of VK_GEOMETRY_TYPE_INSTANCES_KHR, if geometry.arrayOfPointers is "
                                             "VK_TRUE, geometry.data->deviceAddress must be aligned to 8 bytes.");
                        }
                    } else {
                        if (SafeModulo(pInfos[i].pGeometries[j].geometry.instances.data.deviceAddress, 16) != 0) {
                            skip |=
                                LogError(device, "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03715",
                                         "vkCmdBuildAccelerationStructuresKHR:For any element of pInfos[i].pGeometries with a "
                                         "geometryType of VK_GEOMETRY_TYPE_INSTANCES_KHR, if geometry.arrayOfPointers is VK_FALSE, "
                                         "geometry.data->deviceAddress must be aligned to 16 bytes.");
                        }
                    }
                } else if (pInfos[i].pGeometries[j].geometryType == VK_GEOMETRY_TYPE_AABBS_KHR) {
                    if (SafeModulo(pInfos[i].pGeometries[j].geometry.instances.data.deviceAddress, 8) != 0) {
                        skip |= LogError(
                            device, "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03714",
                            "vkCmdBuildAccelerationStructuresKHR:For any element of pInfos[i].pGeometries with a "
                            "geometryType of VK_GEOMETRY_TYPE_AABBS_KHR, geometry.data->deviceAddress must be aligned to 8 bytes.");
                    }
                } else if (pInfos[i].pGeometries[j].geometryType == VK_GEOMETRY_TYPE_TRIANGLES_KHR) {
                    if (SafeModulo(pInfos[i].pGeometries[j].geometry.triangles.transformData.deviceAddress, 16) != 0) {
                        skip |= LogError(device, "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03810",
                                         "vkCmdBuildAccelerationStructuresKHR:For any element of pInfos[i].pGeometries "
                                         "with a geometryType of VK_GEOMETRY_TYPE_TRIANGLES_KHR, "
                                         "geometry.transformData->deviceAddress must be aligned to 16 bytes.");
                    }
                }
            } else if (pInfos[i].ppGeometries) {
                if (pInfos[i].ppGeometries[j]->geometryType == VK_GEOMETRY_TYPE_INSTANCES_KHR) {
                    if (pInfos[i].ppGeometries[j]->geometry.instances.arrayOfPointers == VK_TRUE) {
                        if (SafeModulo(pInfos[i].ppGeometries[j]->geometry.instances.data.deviceAddress, 8) != 0) {
                            skip |= LogError(device, "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03716",
                                             "vkCmdBuildAccelerationStructuresKHR:For any element of pInfos[i].pGeometries with a "
                                             "geometryType of VK_GEOMETRY_TYPE_INSTANCES_KHR, if geometry.arrayOfPointers is "
                                             "VK_TRUE, geometry.data->deviceAddress must be aligned to 8 bytes.");
                        }
                    } else {
                        if (SafeModulo(pInfos[i].ppGeometries[j]->geometry.instances.data.deviceAddress, 16) != 0) {
                            skip |=
                                LogError(device, "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03715",
                                         "vkCmdBuildAccelerationStructuresKHR:For any element of pInfos[i].pGeometries with a "
                                         "geometryType of VK_GEOMETRY_TYPE_INSTANCES_KHR, if geometry.arrayOfPointers is VK_FALSE, "
                                         "geometry.data->deviceAddress must be aligned to 16 bytes.");
                        }
                    }
                } else if (pInfos[i].ppGeometries[j]->geometryType == VK_GEOMETRY_TYPE_AABBS_KHR) {
                    if (SafeModulo(pInfos[i].ppGeometries[j]->geometry.instances.data.deviceAddress, 8) != 0) {
                        skip |= LogError(
                            device, "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03714",
                            "vkCmdBuildAccelerationStructuresKHR:For any element of pInfos[i].pGeometries with a "
                            "geometryType of VK_GEOMETRY_TYPE_AABBS_KHR, geometry.data->deviceAddress must be aligned to 8 bytes.");
                    }
                } else if (pInfos[i].ppGeometries[j]->geometryType == VK_GEOMETRY_TYPE_TRIANGLES_KHR) {
                    if (SafeModulo(pInfos[i].ppGeometries[j]->geometry.triangles.transformData.deviceAddress, 16) != 0) {
                        skip |= LogError(device, "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03810",
                                         "vkCmdBuildAccelerationStructuresKHR:For any element of pInfos[i].pGeometries "
                                         "with a geometryType of VK_GEOMETRY_TYPE_TRIANGLES_KHR, "
                                         "geometry.transformData->deviceAddress must be aligned to 16 bytes.");
                    }
                }
            }
        }
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdBuildAccelerationStructuresIndirectKHR(
    VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
    const VkDeviceAddress *pIndirectDeviceAddresses, const uint32_t *pIndirectStrides,
    const uint32_t *const *ppMaxPrimitiveCounts) const {
    bool skip = false;
    skip |= ValidateAccelerationStructureBuildGeometryInfoKHR(pInfos, infoCount, 0, "vkCmdBuildAccelerationStructuresIndirectKHR");
    const auto *ray_tracing_acceleration_structure_features =
        LvlFindInChain<VkPhysicalDeviceAccelerationStructureFeaturesKHR>(device_createinfo_pnext);
    if (!ray_tracing_acceleration_structure_features ||
        ray_tracing_acceleration_structure_features->accelerationStructureIndirectBuild == VK_FALSE) {
        skip |= LogError(
            device, "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-accelerationStructureIndirectBuild-03650",
            "vkCmdBuildAccelerationStructuresIndirectKHR: The "
            "VkPhysicalDeviceAccelerationStructureFeaturesKHR::accelerationStructureIndirectBuild feature must be enabled.");
    }
    for (uint32_t i = 0; i < infoCount; ++i) {
        if (SafeModulo(pInfos[i].scratchData.deviceAddress,
                       phys_dev_ext_props.acc_structure_props.minAccelerationStructureScratchOffsetAlignment) != 0) {
            skip |= LogError(device, "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03710",
                             "vkCmdBuildAccelerationStructuresIndirectKHR:For each element of pInfos, its "
                             "scratchData.deviceAddress member must be a multiple of "
                             "VkPhysicalDeviceAccelerationStructurePropertiesKHR::minAccelerationStructureScratchOffsetAlignment.");
        }
        for (uint32_t k = 0; k < infoCount; ++k) {
            if (i == k) continue;
            if (pInfos[i].srcAccelerationStructure == pInfos[k].dstAccelerationStructure) {
                skip |= LogError(
                    device, "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03403",
                    "vkCmdBuildAccelerationStructuresIndirectKHR:The srcAccelerationStructure member of any element (%" PRIu32
                    ") "
                    "of pInfos must not be the same acceleration structure as the dstAccelerationStructure member of "
                    "any other element [%" PRIu32 ") of pInfos.",
                    i, k);
                break;
            }
        }
        for (uint32_t j = 0; j < pInfos[i].geometryCount; ++j) {
            if (pInfos[i].pGeometries) {
                if (pInfos[i].pGeometries[j].geometryType == VK_GEOMETRY_TYPE_INSTANCES_KHR) {
                    if (pInfos[i].pGeometries[j].geometry.instances.arrayOfPointers == VK_TRUE) {
                        if (SafeModulo(pInfos[i].pGeometries[j].geometry.instances.data.deviceAddress, 8) != 0) {
                            skip |= LogError(
                                device, "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03716",
                                "vkCmdBuildAccelerationStructuresIndirectKHR:For any element of pInfos[i].pGeometries with a "
                                "geometryType of VK_GEOMETRY_TYPE_INSTANCES_KHR, if geometry.arrayOfPointers is "
                                "VK_TRUE, geometry.data->deviceAddress must be aligned to 8 bytes.");
                        }
                    } else {
                        if (SafeModulo(pInfos[i].pGeometries[j].geometry.instances.data.deviceAddress, 16) != 0) {
                            skip |= LogError(
                                device, "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03715",
                                "vkCmdBuildAccelerationStructuresIndirectKHR:For any element of pInfos[i].pGeometries with a "
                                "geometryType of VK_GEOMETRY_TYPE_INSTANCES_KHR, if geometry.arrayOfPointers is VK_FALSE, "
                                "geometry.data->deviceAddress must be aligned to 16 bytes.");
                        }
                    }
                }
                if (pInfos[i].pGeometries[j].geometryType == VK_GEOMETRY_TYPE_AABBS_KHR) {
                    if (SafeModulo(pInfos[i].pGeometries[j].geometry.instances.data.deviceAddress, 8) != 0) {
                        skip |= LogError(
                            device, "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03714",
                            "vkCmdBuildAccelerationStructuresIndirectKHR:For any element of pInfos[i].pGeometries with a "
                            "geometryType of VK_GEOMETRY_TYPE_AABBS_KHR, geometry.data->deviceAddress must be aligned to 8 bytes.");
                    }
                }
                if (pInfos[i].pGeometries[j].geometryType == VK_GEOMETRY_TYPE_TRIANGLES_KHR) {
                    if (SafeModulo(pInfos[i].pGeometries[j].geometry.triangles.indexData.deviceAddress, 16) != 0) {
                        skip |= LogError(device, "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03810",
                                         "vkCmdBuildAccelerationStructuresIndirectKHR:For any element of pInfos[i].pGeometries "
                                         "with a geometryType of VK_GEOMETRY_TYPE_TRIANGLES_KHR, "
                                         "geometry.transformData->deviceAddress must be aligned to 16 bytes.");
                    }
                }
            } else if (pInfos[i].ppGeometries) {
                if (pInfos[i].ppGeometries[j]->geometryType == VK_GEOMETRY_TYPE_INSTANCES_KHR) {
                    if (pInfos[i].ppGeometries[j]->geometry.instances.arrayOfPointers == VK_TRUE) {
                        if (SafeModulo(pInfos[i].ppGeometries[j]->geometry.instances.data.deviceAddress, 8) != 0) {
                            skip |= LogError(
                                device, "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03716",
                                "vkCmdBuildAccelerationStructuresIndirectKHR:For any element of pInfos[i].pGeometries with a "
                                "geometryType of VK_GEOMETRY_TYPE_INSTANCES_KHR, if geometry.arrayOfPointers is "
                                "VK_TRUE, geometry.data->deviceAddress must be aligned to 8 bytes.");
                        }
                    } else {
                        if (SafeModulo(pInfos[i].ppGeometries[j]->geometry.instances.data.deviceAddress, 16) != 0) {
                            skip |= LogError(
                                device, "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03715",
                                "vkCmdBuildAccelerationStructuresIndirectKHR:For any element of pInfos[i].pGeometries with a "
                                "geometryType of VK_GEOMETRY_TYPE_INSTANCES_KHR, if geometry.arrayOfPointers is VK_FALSE, "
                                "geometry.data->deviceAddress must be aligned to 16 bytes.");
                        }
                    }
                }
                if (pInfos[i].ppGeometries[j]->geometryType == VK_GEOMETRY_TYPE_AABBS_KHR) {
                    if (SafeModulo(pInfos[i].ppGeometries[j]->geometry.instances.data.deviceAddress, 8) != 0) {
                        skip |= LogError(
                            device, "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03714",
                            "vkCmdBuildAccelerationStructuresIndirectKHR:For any element of pInfos[i].pGeometries with a "
                            "geometryType of VK_GEOMETRY_TYPE_AABBS_KHR, geometry.data->deviceAddress must be aligned to 8 bytes.");
                    }
                }
                if (pInfos[i].ppGeometries[j]->geometryType == VK_GEOMETRY_TYPE_TRIANGLES_KHR) {
                    if (SafeModulo(pInfos[i].ppGeometries[j]->geometry.triangles.indexData.deviceAddress, 16) != 0) {
                        skip |= LogError(device, "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03810",
                                         "vkCmdBuildAccelerationStructuresIndirectKHR:For any element of pInfos[i].pGeometries "
                                         "with a geometryType of VK_GEOMETRY_TYPE_TRIANGLES_KHR, "
                                         "geometry.transformData->deviceAddress must be aligned to 16 bytes.");
                    }
                }
            }
        }
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateBuildAccelerationStructuresKHR(
    VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount,
    const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
    const VkAccelerationStructureBuildRangeInfoKHR *const *ppBuildRangeInfos) const {
    bool skip = false;
    skip |= ValidateAccelerationStructureBuildGeometryInfoKHR(pInfos, infoCount, 0, "vkBuildAccelerationStructuresKHR");
    const auto *ray_tracing_acceleration_structure_features =
        LvlFindInChain<VkPhysicalDeviceAccelerationStructureFeaturesKHR>(device_createinfo_pnext);
    if (!ray_tracing_acceleration_structure_features ||
        ray_tracing_acceleration_structure_features->accelerationStructureHostCommands == VK_FALSE) {
        skip |=
            LogError(device, "VUID-vkBuildAccelerationStructuresKHR-accelerationStructureHostCommands-03581",
                     "vkBuildAccelerationStructuresKHR: The "
                     "VkPhysicalDeviceAccelerationStructureFeaturesKHR::accelerationStructureHostCommands feature must be enabled");
    }
    for (uint32_t i = 0; i < infoCount; ++i) {
        for (uint32_t j = 0; j < infoCount; ++j) {
            if (i == j) continue;
            bool found = false;
            if (pInfos[i].dstAccelerationStructure == pInfos[j].dstAccelerationStructure) {
                skip |=
                    LogError(device, "VUID-vkBuildAccelerationStructuresKHR-dstAccelerationStructure-03698",
                             "vkBuildAccelerationStructuresKHR(): The dstAccelerationStructure member of any element (%" PRIu32
                             ") of pInfos must "
                             "not be "
                             "the same acceleration structure as the dstAccelerationStructure member of any other element (%" PRIu32
                             ") of pInfos.",
                             i, j);
                found = true;
            }
            if (pInfos[i].srcAccelerationStructure == pInfos[j].dstAccelerationStructure) {
                skip |=
                    LogError(device, "VUID-vkBuildAccelerationStructuresKHR-pInfos-03403",
                             "vkBuildAccelerationStructuresKHR(): The srcAccelerationStructure member of any element (%" PRIu32
                             ") of pInfos must "
                             "not be "
                             "the same acceleration structure as the dstAccelerationStructure member of any other element (%" PRIu32
                             ") of pInfos.",
                             i, j);
                found = true;
            }
            if (found) break;
        }
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateGetAccelerationStructureBuildSizesKHR(
    VkDevice device, VkAccelerationStructureBuildTypeKHR buildType, const VkAccelerationStructureBuildGeometryInfoKHR *pBuildInfo,
    const uint32_t *pMaxPrimitiveCounts, VkAccelerationStructureBuildSizesInfoKHR *pSizeInfo) const {
    bool skip = false;
    uint64_t total_primitive_count = 0;
    if (pBuildInfo && pMaxPrimitiveCounts) {
        for (uint32_t i = 0; i < pBuildInfo->geometryCount; ++i) {
            total_primitive_count += pMaxPrimitiveCounts[i];
        }
    }
    skip |= ValidateAccelerationStructureBuildGeometryInfoKHR(pBuildInfo, 1, total_primitive_count,
                                                              "vkGetAccelerationStructureBuildSizesKHR");
    const auto *ray_tracing_pipeline_features =
        LvlFindInChain<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>(device_createinfo_pnext);
    const auto *ray_query_features = LvlFindInChain<VkPhysicalDeviceRayQueryFeaturesKHR>(device_createinfo_pnext);
    if (!((ray_tracing_pipeline_features && ray_tracing_pipeline_features->rayTracingPipeline == VK_TRUE) ||
          (ray_query_features && ray_query_features->rayQuery == VK_TRUE))) {
        skip |= LogError(device, "VUID-vkGetAccelerationStructureBuildSizesKHR-rayTracingPipeline-03617",
                         "vkGetAccelerationStructureBuildSizesKHR: The rayTracingPipeline or rayQuery feature must be enabled");
    }
    if (pBuildInfo) {
        if (pBuildInfo->geometryCount != 0 && !pMaxPrimitiveCounts) {
            skip |= LogError(device, "VUID-vkGetAccelerationStructureBuildSizesKHR-pBuildInfo-03619",
                             "vkGetAccelerationStructureBuildSizesKHR: If pBuildInfo->geometryCount is not 0, pMaxPrimitiveCounts "
                             "must be a valid pointer to an array of pBuildInfo->geometryCount uint32_t values");
        }
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdSetVertexInputEXT(
    VkCommandBuffer commandBuffer, uint32_t vertexBindingDescriptionCount,
    const VkVertexInputBindingDescription2EXT *pVertexBindingDescriptions, uint32_t vertexAttributeDescriptionCount,
    const VkVertexInputAttributeDescription2EXT *pVertexAttributeDescriptions) const {
    bool skip = false;
    const auto *vertex_attribute_divisor_features =
        LvlFindInChain<VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT>(device_createinfo_pnext);

    if (vertexBindingDescriptionCount > device_limits.maxVertexInputBindings) {
        skip |=
            LogError(device, "VUID-vkCmdSetVertexInputEXT-vertexBindingDescriptionCount-04791",
                     "vkCmdSetVertexInputEXT(): vertexBindingDescriptionCount is greater than the maxVertexInputBindings limit");
    }

    if (vertexAttributeDescriptionCount > device_limits.maxVertexInputAttributes) {
        skip |= LogError(
            device, "VUID-vkCmdSetVertexInputEXT-vertexAttributeDescriptionCount-04792",
            "vkCmdSetVertexInputEXT(): vertexAttributeDescriptionCount is greater than the maxVertexInputAttributes limit");
    }

    for (uint32_t attribute = 0; attribute < vertexAttributeDescriptionCount; ++attribute) {
        bool binding_found = false;
        for (uint32_t binding = 0; binding < vertexBindingDescriptionCount; ++binding) {
            if (pVertexAttributeDescriptions[attribute].binding == pVertexBindingDescriptions[binding].binding) {
                binding_found = true;
                break;
            }
        }
        if (!binding_found) {
            skip |= LogError(
                device, "VUID-vkCmdSetVertexInputEXT-binding-04793",
                "vkCmdSetVertexInputEXT(): pVertexAttributeDescriptions[%" PRIu32 "] references an unspecified binding", attribute);
        }
    }

    // check for distinct values
    {
        vvl::unordered_set<uint32_t> vertex_bindings(vertexBindingDescriptionCount);
        for (uint32_t i = 0; i < vertexBindingDescriptionCount; ++i) {
            const uint32_t binding = pVertexBindingDescriptions[i].binding;
            auto const &binding_it = vertex_bindings.find(binding);
            if (binding_it != vertex_bindings.cend()) {
                skip |= LogError(device, "VUID-vkCmdSetVertexInputEXT-pVertexBindingDescriptions-04794",
                                 "vkCmdSetVertexInputEXT(): binding description for pVertexBindingDescriptions[%" PRIu32
                                 "] is already in pVertexBindingDescriptions[%" PRIu32 "]",
                                 binding, *binding_it);
                break;
            }
            vertex_bindings.insert(binding);
        }

        vvl::unordered_set<uint32_t> vertex_locations(vertexAttributeDescriptionCount);
        for (uint32_t i = 0; i < vertexAttributeDescriptionCount; ++i) {
            const uint32_t location = pVertexAttributeDescriptions[i].location;
            auto const &location_it = vertex_locations.find(location);
            if (location_it != vertex_locations.cend()) {
                skip |= LogError(device, "VUID-vkCmdSetVertexInputEXT-pVertexAttributeDescriptions-04795",
                                 "vkCmdSetVertexInputEXT(): attribute location for pVertexAttributeDescriptions[%" PRIu32
                                 "] is already in pVertexAttributeDescriptions[%" PRIu32 "]",
                                 location, *location_it);
                break;
            }
            vertex_locations.insert(location);
        }
    }

    for (uint32_t binding = 0; binding < vertexBindingDescriptionCount; ++binding) {
        if (pVertexBindingDescriptions[binding].binding > device_limits.maxVertexInputBindings) {
            skip |= LogError(device, "VUID-VkVertexInputBindingDescription2EXT-binding-04796",
                             "vkCmdSetVertexInputEXT(): pVertexBindingDescriptions[%" PRIu32
                             "].binding is greater than maxVertexInputBindings",
                             binding);
        }

        if (pVertexBindingDescriptions[binding].stride > device_limits.maxVertexInputBindingStride) {
            skip |= LogError(device, "VUID-VkVertexInputBindingDescription2EXT-stride-04797",
                             "vkCmdSetVertexInputEXT(): pVertexBindingDescriptions[%" PRIu32
                             "].stride is greater than maxVertexInputBindingStride",
                             binding);
        }

        if (pVertexBindingDescriptions[binding].divisor == 0 &&
            (!vertex_attribute_divisor_features || !vertex_attribute_divisor_features->vertexAttributeInstanceRateZeroDivisor)) {
            skip |= LogError(device, "VUID-VkVertexInputBindingDescription2EXT-divisor-04798",
                             "vkCmdSetVertexInputEXT(): pVertexBindingDescriptions[%" PRIu32
                             "].divisor is zero but "
                             "vertexAttributeInstanceRateZeroDivisor is not enabled",
                             binding);
        }

        if (pVertexBindingDescriptions[binding].divisor > 1) {
            if (!vertex_attribute_divisor_features || !vertex_attribute_divisor_features->vertexAttributeInstanceRateDivisor) {
                skip |= LogError(device, "VUID-VkVertexInputBindingDescription2EXT-divisor-04799",
                                 "vkCmdSetVertexInputEXT(): pVertexBindingDescriptions[%" PRIu32
                                 "].divisor is greater than one but "
                                 "vertexAttributeInstanceRateDivisor is not enabled",
                                 binding);
            } else {
                if (pVertexBindingDescriptions[binding].divisor >
                    phys_dev_ext_props.vertex_attribute_divisor_props.maxVertexAttribDivisor) {
                    skip |= LogError(device, "VUID-VkVertexInputBindingDescription2EXT-divisor-06226",
                                     "vkCmdSetVertexInputEXT(): pVertexBindingDescriptions[%" PRIu32
                                     "].divisor is greater than maxVertexAttribDivisor",
                                     binding);
                }

                if (pVertexBindingDescriptions[binding].inputRate != VK_VERTEX_INPUT_RATE_INSTANCE) {
                    skip |= LogError(device, "VUID-VkVertexInputBindingDescription2EXT-divisor-06227",
                                     "vkCmdSetVertexInputEXT(): pVertexBindingDescriptions[%" PRIu32
                                     "].divisor is greater than 1 but inputRate "
                                     "is not VK_VERTEX_INPUT_RATE_INSTANCE",
                                     binding);
                }
            }
        }
    }

    for (uint32_t attribute = 0; attribute < vertexAttributeDescriptionCount; ++attribute) {
        if (pVertexAttributeDescriptions[attribute].location > device_limits.maxVertexInputAttributes) {
            skip |= LogError(device, "VUID-VkVertexInputAttributeDescription2EXT-location-06228",
                             "vkCmdSetVertexInputEXT(): pVertexAttributeDescriptions[%" PRIu32
                             "].location is greater than maxVertexInputAttributes",
                             attribute);
        }

        if (pVertexAttributeDescriptions[attribute].binding > device_limits.maxVertexInputBindings) {
            skip |= LogError(device, "VUID-VkVertexInputAttributeDescription2EXT-binding-06229",
                             "vkCmdSetVertexInputEXT(): pVertexAttributeDescriptions[%" PRIu32
                             "].binding is greater than maxVertexInputBindings",
                             attribute);
        }

        if (pVertexAttributeDescriptions[attribute].offset > device_limits.maxVertexInputAttributeOffset) {
            skip |= LogError(device, "VUID-VkVertexInputAttributeDescription2EXT-offset-06230",
                             "vkCmdSetVertexInputEXT(): pVertexAttributeDescriptions[%" PRIu32
                             "].offset is greater than maxVertexInputAttributeOffset",
                             attribute);
        }

        VkFormatProperties properties;
        DispatchGetPhysicalDeviceFormatProperties(physical_device, pVertexAttributeDescriptions[attribute].format, &properties);
        if ((properties.bufferFeatures & VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT) == 0) {
            skip |= LogError(device, "VUID-VkVertexInputAttributeDescription2EXT-format-04805",
                             "vkCmdSetVertexInputEXT(): pVertexAttributeDescriptions[%" PRIu32
                             "].format is not a "
                             "VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT supported format",
                             attribute);
        }
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout,
                                                                 VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size,
                                                                 const void *pValues) const {
    bool skip = false;
    const uint32_t max_push_constants_size = device_limits.maxPushConstantsSize;
    // Check that offset + size don't exceed the max.
    // Prevent arithetic overflow here by avoiding addition and testing in this order.
    if (offset >= max_push_constants_size) {
        skip |=
            LogError(device, "VUID-vkCmdPushConstants-offset-00370",
                     "vkCmdPushConstants(): offset (%" PRIu32 ") that exceeds this device's maxPushConstantSize of %" PRIu32 ".",
                     offset, max_push_constants_size);
    }
    if (size > max_push_constants_size - offset) {
        skip |= LogError(device, "VUID-vkCmdPushConstants-size-00371",
                         "vkCmdPushConstants(): offset (%" PRIu32 ") and size (%" PRIu32
                         ") that exceeds this device's maxPushConstantSize of %" PRIu32 ".",
                         offset, size, max_push_constants_size);
    }

    // size needs to be non-zero and a multiple of 4.
    if (size & 0x3) {
        skip |= LogError(device, "VUID-vkCmdPushConstants-size-00369",
                         "vkCmdPushConstants(): size (%" PRIu32 ") must be a multiple of 4.", size);
    }

    // offset needs to be a multiple of 4.
    if ((offset & 0x3) != 0) {
        skip |= LogError(device, "VUID-vkCmdPushConstants-offset-00368",
                         "vkCmdPushConstants(): offset (%" PRIu32 ") must be a multiple of 4.", offset);
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateMergePipelineCaches(VkDevice device, VkPipelineCache dstCache,
                                                                    uint32_t srcCacheCount,
                                                                    const VkPipelineCache *pSrcCaches) const {
    bool skip = false;
    if (pSrcCaches) {
        for (uint32_t index0 = 0; index0 < srcCacheCount; ++index0) {
            if (pSrcCaches[index0] == dstCache) {
                skip |= LogError(instance, "VUID-vkMergePipelineCaches-dstCache-00770",
                                 "vkMergePipelineCaches(): dstCache %s is in pSrcCaches list.",
                                 report_data->FormatHandle(dstCache).c_str());
                break;
            }
        }
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image,
                                                                   VkImageLayout imageLayout, const VkClearColorValue *pColor,
                                                                   uint32_t rangeCount,
                                                                   const VkImageSubresourceRange *pRanges) const {
    bool skip = false;
    if (!pColor) {
        skip |=
            LogError(commandBuffer, "VUID-vkCmdClearColorImage-pColor-04961", "vkCmdClearColorImage(): pColor must not be null");
    }
    return skip;
}

bool StatelessValidation::ValidateCmdBeginRenderPass(const VkRenderPassBeginInfo *const rp_begin, CMD_TYPE cmd_type) const {
    bool skip = false;
    if ((rp_begin->clearValueCount != 0) && !rp_begin->pClearValues) {
        skip |= LogError(rp_begin->renderPass, "VUID-VkRenderPassBeginInfo-clearValueCount-04962",
                         "%s: VkRenderPassBeginInfo::clearValueCount != 0 (%" PRIu32
                         "), but VkRenderPassBeginInfo::pClearValues is null.",
                         CommandTypeString(cmd_type), rp_begin->clearValueCount);
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdBeginRenderPass(VkCommandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                                   VkSubpassContents) const {
    bool skip = ValidateCmdBeginRenderPass(pRenderPassBegin, CMD_BEGINRENDERPASS);
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdBeginRenderPass2KHR(VkCommandBuffer,
                                                                       const VkRenderPassBeginInfo *pRenderPassBegin,
                                                                       const VkSubpassBeginInfo *) const {
    bool skip = ValidateCmdBeginRenderPass(pRenderPassBegin, CMD_BEGINRENDERPASS2KHR);
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdBeginRenderPass2(VkCommandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                                    const VkSubpassBeginInfo *) const {
    bool skip = ValidateCmdBeginRenderPass(pRenderPassBegin, CMD_BEGINRENDERPASS2);
    return skip;
}

static bool UniqueRenderingInfoImageViews(const VkRenderingInfo *pRenderingInfo, VkImageView imageView) {
    bool unique_views = true;
    for (uint32_t i = 0; i < pRenderingInfo->colorAttachmentCount; ++i) {
        if (pRenderingInfo->pColorAttachments[i].imageView == imageView) {
            unique_views = false;
        }

        if (pRenderingInfo->pColorAttachments[i].resolveImageView == imageView) {
            unique_views = false;
        }
    }

    if (pRenderingInfo->pDepthAttachment) {
        if (pRenderingInfo->pDepthAttachment->imageView == imageView) {
            unique_views = false;
        }

        if (pRenderingInfo->pDepthAttachment->resolveImageView == imageView) {
            unique_views = false;
        }
    }

    if (pRenderingInfo->pStencilAttachment) {
        if (pRenderingInfo->pStencilAttachment->imageView == imageView) {
            unique_views = false;
        }

        if (pRenderingInfo->pStencilAttachment->resolveImageView == imageView) {
            unique_views = false;
        }
    }
    return unique_views;
}

bool StatelessValidation::ValidateCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo *pRenderingInfo,
                                                    CMD_TYPE cmd_type) const {
    bool skip = false;
    const char *func_name = CommandTypeString(cmd_type);

    if (pRenderingInfo->viewMask == 0 && pRenderingInfo->layerCount == 0) {
        skip |= LogError(commandBuffer, "VUID-VkRenderingInfo-viewMask-06069",
                         "%s(): If viewMask is 0 (%" PRIu32 "), layerCount must not be 0 (%" PRIu32 ").", func_name,
                         pRenderingInfo->viewMask, pRenderingInfo->layerCount);
    }

    if (pRenderingInfo->colorAttachmentCount > device_limits.maxColorAttachments) {
        skip |= LogError(commandBuffer, "VUID-VkRenderingInfo-colorAttachmentCount-06106",
                         "%s(): colorAttachmentCount (%u) must be less than or equal to "
                         "VkPhysicalDeviceLimits::maxColorAttachments (%u).",
                         func_name, pRenderingInfo->colorAttachmentCount, device_limits.maxColorAttachments);
    }

    const auto rendering_fragment_shading_rate_attachment_info =
        LvlFindInChain<VkRenderingFragmentShadingRateAttachmentInfoKHR>(pRenderingInfo->pNext);
    if (rendering_fragment_shading_rate_attachment_info &&
        (rendering_fragment_shading_rate_attachment_info->imageView != VK_NULL_HANDLE)) {
        if (UniqueRenderingInfoImageViews(pRenderingInfo, rendering_fragment_shading_rate_attachment_info->imageView) == false) {
            skip |= LogError(commandBuffer, "VUID-VkRenderingInfo-imageView-06125",
                             "%s(): imageView or resolveImageView member of pDepthAttachment, pStencilAttachment, or any element "
                             "of pColorAttachments must not equal VkRenderingFragmentShadingRateAttachmentInfoKHR->imageView.",
                             func_name);
        }

        const VkImageLayout image_layout = rendering_fragment_shading_rate_attachment_info->imageLayout;
        if (image_layout != VK_IMAGE_LAYOUT_GENERAL &&
            image_layout != VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR) {
            skip |= LogError(commandBuffer, "VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-06147",
                             "%s(): VkRenderingFragmentShadingRateAttachmentInfoKHR->layout (%s) must be VK_IMAGE_LAYOUT_GENERAL "
                             "or VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR.",
                             func_name, string_VkImageLayout(image_layout));
        }

        if (!IsPowerOfTwo(rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.width)) {
            skip |= LogError(commandBuffer, "VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-06149",
                             "%s(): shadingRateAttachmentTexelSize.width (%u) must be a power of two.", func_name,
                             rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.width);
        }

        const uint32_t max_frs_attach_texel_width =
            phys_dev_ext_props.fragment_shading_rate_props.maxFragmentShadingRateAttachmentTexelSize.width;
        if (rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.width > max_frs_attach_texel_width) {
            skip |= LogError(commandBuffer, "VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-06150",
                             "%s(): shadingRateAttachmentTexelSize.width (%u) must be less than or equal to "
                             "maxFragmentShadingRateAttachmentTexelSize.width (%u).",
                             func_name, rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.width,
                             max_frs_attach_texel_width);
        }

        const uint32_t min_frs_attach_texel_width =
            phys_dev_ext_props.fragment_shading_rate_props.minFragmentShadingRateAttachmentTexelSize.width;
        if (rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.width < min_frs_attach_texel_width) {
            skip |= LogError(commandBuffer, "VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-06151",
                             "%s(): shadingRateAttachmentTexelSize.width (%u) must be greater than or equal to "
                             "minFragmentShadingRateAttachmentTexelSize.width (%u).",
                             func_name, rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.width,
                             min_frs_attach_texel_width);
        }

        if (!IsPowerOfTwo(rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.height)) {
            skip |= LogError(commandBuffer, "VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-06152",
                             "%s(): shadingRateAttachmentTexelSize.height (%u) must be a power of two.", func_name,
                             rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.height);
        }

        const uint32_t max_frs_attach_texel_height =
            phys_dev_ext_props.fragment_shading_rate_props.maxFragmentShadingRateAttachmentTexelSize.height;
        if (rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.height > max_frs_attach_texel_height) {
            skip |= LogError(commandBuffer, "VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-06153",
                             "%s(): shadingRateAttachmentTexelSize.height (%u) must be less than or equal to "
                             "maxFragmentShadingRateAttachmentTexelSize.height (%u).",
                             func_name, rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.height,
                             max_frs_attach_texel_height);
        }

        const uint32_t min_frs_attach_texel_height =
            phys_dev_ext_props.fragment_shading_rate_props.minFragmentShadingRateAttachmentTexelSize.height;
        if (rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.height < min_frs_attach_texel_height) {
            skip |= LogError(commandBuffer, "VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-06154",
                             "%s(): shadingRateAttachmentTexelSize.height (%u) must be greater than or equal to "
                             "minFragmentShadingRateAttachmentTexelSize.height (%u).",
                             func_name, rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.height,
                             min_frs_attach_texel_height);
        }

        const uint32_t max_frs_attach_texel_aspect_ratio =
            phys_dev_ext_props.fragment_shading_rate_props.maxFragmentShadingRateAttachmentTexelSizeAspectRatio;
        if ((rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.width /
             rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.height) >
            max_frs_attach_texel_aspect_ratio) {
            skip |= LogError(
                commandBuffer, "VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-06155",
                "%s(): the quotient of shadingRateAttachmentTexelSize.width (%u) and shadingRateAttachmentTexelSize.height (%u) "
                "must be less than or equal to maxFragmentShadingRateAttachmentTexelSizeAspectRatio (%u).",
                func_name, rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.width,
                rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.height,
                max_frs_attach_texel_aspect_ratio);
        }

        if ((rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.height /
             rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.width) >
            max_frs_attach_texel_aspect_ratio) {
            skip |= LogError(
                commandBuffer, "VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-06156",
                "%s(): the quotient of shadingRateAttachmentTexelSize.height (%u) and shadingRateAttachmentTexelSize.width (%u) "
                "must be less than or equal to maxFragmentShadingRateAttachmentTexelSizeAspectRatio (%u).",
                func_name, rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.height,
                rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.width,
                max_frs_attach_texel_aspect_ratio);
        }
    }

    const auto fragment_density_map_attachment_info =
        LvlFindInChain<VkRenderingFragmentDensityMapAttachmentInfoEXT>(pRenderingInfo->pNext);
    if (fragment_density_map_attachment_info && (fragment_density_map_attachment_info->imageView != VK_NULL_HANDLE)) {
        if (UniqueRenderingInfoImageViews(pRenderingInfo, fragment_density_map_attachment_info->imageView) == false) {
            skip |=
                LogError(commandBuffer, "VUID-VkRenderingInfo-imageView-06116",
                         "%s(): imageView or resolveImageView member of pDepthAttachment, pStencilAttachment, or any"
                         "element of pColorAttachments must not equal VkRenderingFragmentDensityMapAttachmentInfoEXT->imageView.",
                         func_name);
        }

        if (fragment_density_map_attachment_info->imageLayout != VK_IMAGE_LAYOUT_GENERAL &&
            fragment_density_map_attachment_info->imageLayout != VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT) {
            skip |= LogError(commandBuffer, "VUID-VkRenderingFragmentDensityMapAttachmentInfoEXT-imageView-06157",
                             "%s(): VkRenderingFragmentDensityMapAttachmentInfoEXT::imageView is not VK_NULL_HANDLE, but "
                             "VkRenderingFragmentDensityMapAttachmentInfoEXT::imageLayout is %s.",
                             func_name, string_VkImageLayout(fragment_density_map_attachment_info->imageLayout));
        }

        if (rendering_fragment_shading_rate_attachment_info &&
            (rendering_fragment_shading_rate_attachment_info->imageView == fragment_density_map_attachment_info->imageView)) {
            skip |= LogError(commandBuffer, "VUID-VkRenderingInfo-imageView-06126",
                             "%s(): VkRenderingFragmentDensityMapAttachmentInfoEXT::imageView and "
                             "VkRenderingFragmentShadingRateAttachmentInfoKHR::imageView are the same.",
                             func_name);
        }
    }

    for (uint32_t j = 0; j < pRenderingInfo->colorAttachmentCount; ++j) {
        if (pRenderingInfo->pColorAttachments[j].imageView != VK_NULL_HANDLE) {
            const VkImageLayout image_layout = pRenderingInfo->pColorAttachments[j].imageLayout;
            const VkResolveModeFlagBits resolve_mode = pRenderingInfo->pColorAttachments[j].resolveMode;
            const VkImageLayout resolve_image_layout = pRenderingInfo->pColorAttachments[j].resolveImageLayout;
            if (image_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
                image_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL) {
                skip |= LogError(commandBuffer, "VUID-VkRenderingInfo-colorAttachmentCount-06090",
                                 "%s(): imageLayout must not be "
                                 "VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL or "
                                 "VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL.",
                                 func_name);
            }

            if (resolve_mode != VK_RESOLVE_MODE_NONE) {
                if (resolve_image_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
                    resolve_image_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL) {
                    skip |= LogError(commandBuffer, "VUID-VkRenderingInfo-colorAttachmentCount-06091",
                                     "%s(): resolveImageLayout must not be "
                                     "VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL or "
                                     "VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL.",
                                     func_name);
                }
            }

            if (IsExtEnabled(device_extensions.vk_khr_maintenance2)) {
                if (image_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL ||
                    image_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL) {
                    skip |= LogError(commandBuffer, "VUID-VkRenderingInfo-colorAttachmentCount-06096",
                                     "%s(): imageLayout must not be "
                                     "VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL "
                                     "or VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL.",
                                     func_name);
                }

                if (resolve_mode != VK_RESOLVE_MODE_NONE) {
                    if (resolve_image_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL ||
                        resolve_image_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL) {
                        skip |= LogError(commandBuffer, "VUID-VkRenderingInfo-colorAttachmentCount-06097",
                                         "%s(): resolveImageLayout must not be "
                                         "VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL or "
                                         "VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL.",
                                         func_name);
                    }
                }
            }

            if (image_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL ||
                image_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL ||
                image_layout == VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL ||
                image_layout == VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL) {
                skip |= LogError(commandBuffer, "VUID-VkRenderingInfo-colorAttachmentCount-06100",
                                 "%s(): imageLayout must not be VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL"
                                 " or VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL"
                                 " or VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL"
                                 " or VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL.",
                                 func_name);
            }

            if (resolve_mode != VK_RESOLVE_MODE_NONE) {
                if (resolve_image_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL ||
                    resolve_image_layout == VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL) {
                    skip |= LogError(commandBuffer, "VUID-VkRenderingInfo-colorAttachmentCount-06101",
                                     "%s(): resolveImageLayout must not be VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL or "
                                     "VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL.",
                                     func_name);
                }
            }
        }
    }

    if (pRenderingInfo->pDepthAttachment && pRenderingInfo->pDepthAttachment->imageView != VK_NULL_HANDLE) {
        const VkImageLayout layout = pRenderingInfo->pDepthAttachment->imageLayout;
        if (layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
            skip |=
                LogError(commandBuffer, "VUID-VkRenderingInfo-pDepthAttachment-06092",
                         "%s(): pDepthAttachment->imageLayout is can't be VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL.", func_name);
        } else if (layout == VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL || layout == VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL) {
            skip |= LogError(commandBuffer, "VUID-VkRenderingInfo-pDepthAttachment-07732",
                             "%s(): pDepthAttachment->imageLayout is can't be VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL or "
                             "VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL.",
                             func_name);
        }

        if (pRenderingInfo->pDepthAttachment->resolveMode != VK_RESOLVE_MODE_NONE) {
            const VkImageLayout resolve_layout = pRenderingInfo->pDepthAttachment->resolveImageLayout;
            if (resolve_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
                skip |= LogError(commandBuffer, "VUID-VkRenderingInfo-pDepthAttachment-06093",
                                 "%s(): pDepthAttachment->resolveImageLayout must not be VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL.",
                                 func_name);
            } else if (resolve_layout == VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL ||
                       resolve_layout == VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL) {
                skip |= LogError(commandBuffer, "VUID-VkRenderingInfo-pDepthAttachment-07733",
                                 "%s(): pDepthAttachment->resolveImageLayout must not be "
                                 "VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL or VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL.",
                                 func_name);
            }

            if (IsExtEnabled(device_extensions.vk_khr_maintenance2) &&
                resolve_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL) {
                skip |= LogError(commandBuffer, "VUID-VkRenderingInfo-pDepthAttachment-06098",
                                 "%s(): resolveImageLayout must not be "
                                 "VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL.",
                                 func_name);
            }

            if (!(pRenderingInfo->pDepthAttachment->resolveMode &
                  phys_dev_ext_props.depth_stencil_resolve_props.supportedDepthResolveModes)) {
                skip |= LogError(device, "VUID-VkRenderingInfo-pDepthAttachment-06102",
                                 "%s(): Includes a resolveMode structure with invalid mode=%u.", func_name,
                                 pRenderingInfo->pDepthAttachment->resolveMode);
            }
        }
    }

    if (pRenderingInfo->pStencilAttachment != nullptr && pRenderingInfo->pStencilAttachment->imageView != VK_NULL_HANDLE) {
        const VkImageLayout layout = pRenderingInfo->pStencilAttachment->imageLayout;
        if (layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
            skip |=
                LogError(commandBuffer, "VUID-VkRenderingInfo-pStencilAttachment-06094",
                         "%s(): pStencilAttachment->imageLayout is can't be VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL.", func_name);
        } else if (layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL || layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL) {
            skip |= LogError(commandBuffer, "VUID-VkRenderingInfo-pStencilAttachment-07734",
                             "%s(): pStencilAttachment->imageLayout is can't be VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL or "
                             "VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL.",
                             func_name);
        }

        if (pRenderingInfo->pStencilAttachment->resolveMode != VK_RESOLVE_MODE_NONE) {
            const VkImageLayout resolve_layout = pRenderingInfo->pStencilAttachment->resolveImageLayout;
            if (resolve_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
                skip |=
                    LogError(commandBuffer, "VUID-VkRenderingInfo-pStencilAttachment-06095",
                             "%s(): pStencilAttachment->resolveImageLayout must not be VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL.",
                             func_name);
            } else if (resolve_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL ||
                       resolve_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL) {
                skip |=
                    LogError(commandBuffer, "VUID-VkRenderingInfo-pStencilAttachment-07735",
                             "%s(): pStencilAttachment->resolveImageLayout must not be VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL.",
                             func_name);
            }

            if (IsExtEnabled(device_extensions.vk_khr_maintenance2) &&
                resolve_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL) {
                skip |= LogError(commandBuffer, "VUID-VkRenderingInfo-pStencilAttachment-06099",
                                 "%s(): resolveImageLayout must not be "
                                 "VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL.",
                                 func_name);
            }

            if (!(pRenderingInfo->pStencilAttachment->resolveMode &
                  phys_dev_ext_props.depth_stencil_resolve_props.supportedStencilResolveModes)) {
                skip |= LogError(device, "VUID-VkRenderingInfo-pStencilAttachment-06103",
                                 "%s(): Includes a resolveMode structure with invalid mode (%s).", func_name,
                                 string_VkResolveModeFlagBits(pRenderingInfo->pStencilAttachment->resolveMode));
            }
        }
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdBeginRenderingKHR(VkCommandBuffer commandBuffer,
                                                                     const VkRenderingInfo *pRenderingInfo) const {
    bool skip = ValidateCmdBeginRendering(commandBuffer, pRenderingInfo, CMD_BEGINRENDERINGKHR);
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdBeginRendering(VkCommandBuffer commandBuffer,
                                                                  const VkRenderingInfo *pRenderingInfo) const {
    bool skip = ValidateCmdBeginRendering(commandBuffer, pRenderingInfo, CMD_BEGINRENDERING);
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer,
                                                                          uint32_t firstDiscardRectangle,
                                                                          uint32_t discardRectangleCount,
                                                                          const VkRect2D *pDiscardRectangles) const {
    bool skip = false;

    if (pDiscardRectangles) {
        for (uint32_t i = 0; i < discardRectangleCount; ++i) {
            const int64_t x_sum =
                static_cast<int64_t>(pDiscardRectangles[i].offset.x) + static_cast<int64_t>(pDiscardRectangles[i].extent.width);
            if (x_sum > std::numeric_limits<int32_t>::max()) {
                skip |= LogError(device, "VUID-vkCmdSetDiscardRectangleEXT-offset-00588",
                                 "vkCmdSetDiscardRectangleEXT(): offset.x + extent.width (=%" PRIi32 " + %" PRIu32 " = %" PRIi64
                                 ") of pDiscardRectangles[%" PRIu32 "] will overflow int32_t.",
                                 pDiscardRectangles[i].offset.x, pDiscardRectangles[i].extent.width, x_sum, i);
            }

            const int64_t y_sum =
                static_cast<int64_t>(pDiscardRectangles[i].offset.y) + static_cast<int64_t>(pDiscardRectangles[i].extent.height);
            if (y_sum > std::numeric_limits<int32_t>::max()) {
                skip |= LogError(device, "VUID-vkCmdSetDiscardRectangleEXT-offset-00589",
                                 "vkCmdSetDiscardRectangleEXT(): offset.y + extent.height (=%" PRIi32 " + %" PRIu32 " = %" PRIi64
                                 ") of pDiscardRectangles[%" PRIu32 "] will overflow int32_t.",
                                 pDiscardRectangles[i].offset.y, pDiscardRectangles[i].extent.height, y_sum, i);
            }
        }
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery,
                                                                    uint32_t queryCount, size_t dataSize, void *pData,
                                                                    VkDeviceSize stride, VkQueryResultFlags flags) const {
    bool skip = false;

    if ((flags & VK_QUERY_RESULT_WITH_STATUS_BIT_KHR) && (flags & VK_QUERY_RESULT_WITH_AVAILABILITY_BIT)) {
        skip |= LogError(device, "VUID-vkGetQueryPoolResults-flags-04811",
                         "vkGetQueryPoolResults(): flags include both VK_QUERY_RESULT_WITH_STATUS_BIT_KHR bit and "
                         "VK_QUERY_RESULT_WITH_AVAILABILITY_BIT bit.");
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdBeginConditionalRenderingEXT(
    VkCommandBuffer commandBuffer, const VkConditionalRenderingBeginInfoEXT *pConditionalRenderingBegin) const {
    bool skip = false;

    if ((pConditionalRenderingBegin->offset & 3) != 0) {
        skip |= LogError(commandBuffer, "VUID-VkConditionalRenderingBeginInfoEXT-offset-01984",
                         "vkCmdBeginConditionalRenderingEXT(): pConditionalRenderingBegin->offset (%" PRIu64
                         ") is not a multiple of 4.",
                         pConditionalRenderingBegin->offset);
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice,
                                                                                   VkSurfaceKHR surface,
                                                                                   uint32_t *pSurfaceFormatCount,
                                                                                   VkSurfaceFormatKHR *pSurfaceFormats) const {
    bool skip = false;
    if (surface == VK_NULL_HANDLE && !instance_extensions.vk_google_surfaceless_query) {
        skip |= LogError(
            physicalDevice, "VUID-vkGetPhysicalDeviceSurfaceFormatsKHR-surface-06524",
            "vkGetPhysicalDeviceSurfaceFormatsKHR(): surface is VK_NULL_HANDLE and VK_GOOGLE_surfaceless_query is not enabled.");
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice,
                                                                                        VkSurfaceKHR surface,
                                                                                        uint32_t *pPresentModeCount,
                                                                                        VkPresentModeKHR *pPresentModes) const {
    bool skip = false;
    if (surface == VK_NULL_HANDLE && !instance_extensions.vk_google_surfaceless_query) {
        skip |= LogError(
            physicalDevice, "VUID-vkGetPhysicalDeviceSurfacePresentModesKHR-surface-06524",
            "vkGetPhysicalDeviceSurfacePresentModesKHR: surface is VK_NULL_HANDLE and VK_GOOGLE_surfaceless_query is not enabled.");
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateGetPhysicalDeviceSurfaceCapabilities2KHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo,
    VkSurfaceCapabilities2KHR *pSurfaceCapabilities) const {
    bool skip = false;
    if (pSurfaceInfo && pSurfaceInfo->surface == VK_NULL_HANDLE && !instance_extensions.vk_google_surfaceless_query) {
        skip |= LogError(physicalDevice, "VUID-vkGetPhysicalDeviceSurfaceCapabilities2KHR-pSurfaceInfo-06520",
                         "vkGetPhysicalDeviceSurfaceCapabilities2KHR: pSurfaceInfo->surface is VK_NULL_HANDLE and "
                         "VK_GOOGLE_surfaceless_query is not enabled.");
    }
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    const auto *capabilities_full_screen_exclusive =
        LvlFindInChain<VkSurfaceCapabilitiesFullScreenExclusiveEXT>(pSurfaceCapabilities->pNext);
    if (capabilities_full_screen_exclusive) {
        const auto *full_screen_exclusive_win32_info =
            LvlFindInChain<VkSurfaceFullScreenExclusiveWin32InfoEXT>(pSurfaceInfo->pNext);
        if (!full_screen_exclusive_win32_info) {
            skip |= LogError(device, "VUID-vkGetPhysicalDeviceSurfaceCapabilities2KHR-pNext-02671",
                             "vkGetPhysicalDeviceSurfaceCapabilities2KHR(): pSurfaceCapabilities->pNext contains "
                             "VkSurfaceCapabilitiesFullScreenExclusiveEXT, but pSurfaceInfo->pNext does not contain "
                             "VkSurfaceFullScreenExclusiveWin32InfoEXT");
        }
    }
#endif

    if (IsExtEnabled(device_extensions.vk_ext_surface_maintenance1)) {
        const auto *surface_present_mode_compatibility =
            LvlFindInChain<VkSurfacePresentModeCompatibilityEXT>(pSurfaceCapabilities->pNext);
        const auto *surface_present_scaling_compatibilities =
            LvlFindInChain<VkSurfacePresentScalingCapabilitiesEXT>(pSurfaceCapabilities->pNext);

        if (!(LvlFindInChain<VkSurfacePresentModeEXT>(pSurfaceInfo->pNext))) {
            if (surface_present_mode_compatibility) {
                skip |= LogError(device, "VUID-vkGetPhysicalDeviceSurfaceCapabilities2KHR-pNext-07776",
                                 "vkGetPhysicalDeviceSurfaceCapabilities2KHR(): VK_EXT_surface_maintenance1 is enabled and "
                                 "pSurfaceCapabilities->pNext contains VkSurfacePresentModeCompatibilityEXT, but "
                                 "pSurfaceInfo->pNext does not contain a VkSurfacePresentModeEXT structure.");
            }

            if (surface_present_scaling_compatibilities) {
                skip |= LogError(device, "VUID-vkGetPhysicalDeviceSurfaceCapabilities2KHR-pNext-07777",
                                 "vkGetPhysicalDeviceSurfaceCapabilities2KHR(): VK_EXT_surface_maintenance1 is enabled and "
                                 "pSurfaceCapabilities->pNext contains VkSurfacePresentScalingCapabilitiesEXT, but "
                                 "pSurfaceInfo->pNext does not contain a VkSurfacePresentModeEXT structure.");
            }
        }

        if (IsExtEnabled(instance_extensions.vk_google_surfaceless_query)) {
            if (pSurfaceInfo->surface == VK_NULL_HANDLE) {
                if (surface_present_mode_compatibility) {
                    skip |=
                        LogError(physicalDevice, "VUID-vkGetPhysicalDeviceSurfaceCapabilities2KHR-pNext-07778",
                                 "vkGetPhysicalDeviceSurfaceCapabilities2KHR: VK_EXT_surface_maintenance1 and "
                                 "VK_GOOGLE_surfaceless_query are enabled and pSurfaceCapabilities->pNext contains a "
                                 "VkSurfacePresentModeCompatibilityEXT structure, but pSurfaceInfo->surface is VK_NULL_HANDLE.");
                }

                if (surface_present_scaling_compatibilities) {
                    skip |=
                        LogError(physicalDevice, "VUID-vkGetPhysicalDeviceSurfaceCapabilities2KHR-pNext-07779",
                                 "vkGetPhysicalDeviceSurfaceCapabilities2KHR: VK_EXT_surface_maintenance1 and "
                                 "VK_GOOGLE_surfaceless_query are enabled and pSurfaceCapabilities->pNext contains a "
                                 "VkSurfacePresentScalingCapabilitiesEXT structure, but pSurfaceInfo->surface is VK_NULL_HANDLE.");
                }
            }
        }
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateGetPhysicalDeviceSurfaceFormats2KHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo, uint32_t *pSurfaceFormatCount,
    VkSurfaceFormat2KHR *pSurfaceFormats) const {
    bool skip = false;
    if (pSurfaceInfo && pSurfaceInfo->surface == VK_NULL_HANDLE && !instance_extensions.vk_google_surfaceless_query) {
        skip |= LogError(physicalDevice, "VUID-vkGetPhysicalDeviceSurfaceFormats2KHR-pSurfaceInfo-06521",
                         "vkGetPhysicalDeviceSurfaceFormats2KHR: pSurfaceInfo->surface is VK_NULL_HANDLE and "
                         "VK_GOOGLE_surfaceless_query is not enabled.");
    }
    return skip;
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
bool StatelessValidation::manual_PreCallValidateGetPhysicalDeviceSurfacePresentModes2EXT(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo, uint32_t *pPresentModeCount,
    VkPresentModeKHR *pPresentModes) const {
    bool skip = false;
    if (pSurfaceInfo && pSurfaceInfo->surface == VK_NULL_HANDLE && !instance_extensions.vk_google_surfaceless_query) {
        skip |= LogError(physicalDevice, "VUID-vkGetPhysicalDeviceSurfacePresentModes2EXT-pSurfaceInfo-06521",
                         "vkGetPhysicalDeviceSurfacePresentModes2EXT: pSurfaceInfo->surface is VK_NULL_HANDLE and "
                         "VK_GOOGLE_surfaceless_query is not enabled.");
    }
    return skip;
}

#endif  // VK_USE_PLATFORM_WIN32_KHR

bool StatelessValidation::ValidateDeviceImageMemoryRequirements(VkDevice device, const VkDeviceImageMemoryRequirementsKHR *pInfo,
                                                                const char *func_name) const {
    bool skip = false;

    if (pInfo && pInfo->pCreateInfo) {
        const auto *image_swapchain_create_info = LvlFindInChain<VkImageSwapchainCreateInfoKHR>(pInfo->pCreateInfo);
        if (image_swapchain_create_info) {
            skip |= LogError(device, "VUID-VkDeviceImageMemoryRequirementsKHR-pCreateInfo-06416",
                             "%s(): pInfo->pCreateInfo->pNext chain contains VkImageSwapchainCreateInfoKHR.", func_name);
        }
        const auto *drm_format_modifier_create_info =
            LvlFindInChain<VkImageDrmFormatModifierExplicitCreateInfoEXT>(pInfo->pCreateInfo);
        if (drm_format_modifier_create_info) {
            skip |= LogError(device, "VUID-VkDeviceImageMemoryRequirements-pCreateInfo-06776",
                             "%s(): pInfo->pCreateInfo->pNext chain contains VkImageDrmFormatModifierExplicitCreateInfoEXT.",
                             func_name);
        }

        if ((pInfo->pCreateInfo->flags & VK_IMAGE_CREATE_DISJOINT_BIT) != 0) {
            if (FormatIsMultiplane(pInfo->pCreateInfo->format) && (pInfo->planeAspect == VK_IMAGE_ASPECT_NONE_KHR)) {
                skip |= LogError(device, "VUID-VkDeviceImageMemoryRequirementsKHR-pCreateInfo-06417",
                                 "%s(): Must not specify VK_IMAGE_ASPECT_NONE_KHR with a multi-planar format and disjoint flag.",
                                 func_name);
            }
        }
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateGetDeviceImageMemoryRequirementsKHR(
    VkDevice device, const VkDeviceImageMemoryRequirements *pInfo, VkMemoryRequirements2 *pMemoryRequirements) const {
    bool skip = false;

    skip |= ValidateDeviceImageMemoryRequirements(device, pInfo, "vkGetDeviceImageMemoryRequirementsKHR");

    return skip;
}

bool StatelessValidation::manual_PreCallValidateGetDeviceImageSparseMemoryRequirementsKHR(
    VkDevice device, const VkDeviceImageMemoryRequirements *pInfo, uint32_t *pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2 *pSparseMemoryRequirements) const {
    bool skip = false;

    skip |= ValidateDeviceImageMemoryRequirements(device, pInfo, "vkGetDeviceImageSparseMemoryRequirementsKHR");

    return skip;
}

#ifdef VK_USE_PLATFORM_METAL_EXT
bool StatelessValidation::manual_PreCallValidateExportMetalObjectsEXT(VkDevice device,
                                                                      VkExportMetalObjectsInfoEXT *pMetalObjectsInfo) const {
    bool skip = false;

    static_assert(AllVkExportMetalObjectTypeFlagBitsEXT == 0x3F, "Add new ExportMetalObjects support to VVL!");

    constexpr std::array allowed_structs = {
        VK_STRUCTURE_TYPE_EXPORT_METAL_BUFFER_INFO_EXT,       VK_STRUCTURE_TYPE_EXPORT_METAL_COMMAND_QUEUE_INFO_EXT,
        VK_STRUCTURE_TYPE_EXPORT_METAL_DEVICE_INFO_EXT,       VK_STRUCTURE_TYPE_EXPORT_METAL_IO_SURFACE_INFO_EXT,
        VK_STRUCTURE_TYPE_EXPORT_METAL_SHARED_EVENT_INFO_EXT, VK_STRUCTURE_TYPE_EXPORT_METAL_TEXTURE_INFO_EXT,
    };
    skip |= ValidateStructPnext("vkExportMetalObjectsEXT", "pMetalObjectsInfo->pNext",
                                "VkExportMetalBufferInfoEXT, VkExportMetalCommandQueueInfoEXT, VkExportMetalDeviceInfoEXT, "
                                "VkExportMetalIOSurfaceInfoEXT, VkExportMetalSharedEventInfoEXT, VkExportMetalTextureInfoEXT",
                                pMetalObjectsInfo->pNext, allowed_structs.size(), allowed_structs.data(),
                                GeneratedVulkanHeaderVersion, "VUID-VkExportMetalObjectsInfoEXT-pNext-pNext",
                                "VUID-VkExportMetalObjectsInfoEXT-sType-unique", false, true);
    return skip;
}
#endif  // VK_USE_PLATFORM_METAL_EXT
