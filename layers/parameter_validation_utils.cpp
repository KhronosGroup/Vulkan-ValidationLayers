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
 * Author: Mark Lobodzinski <mark@LunarG.com>
 * Author: John Zulauf <jzulauf@lunarg.com>
 */

#define NOMINMAX

#include <math.h>

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
                                          const char *validateString) {
    bool skip = false;

    VkStringErrorFlags result = vk_string_validate(MaxParamCheckerStringLength, validateString);

    if (result == VK_STRING_ERROR_NONE) {
        return skip;
    } else if (result & VK_STRING_ERROR_LENGTH) {
        skip = log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, vuid,
                       "%s: string %s exceeds max length %d", apiName, stringName.get_name().c_str(), MaxParamCheckerStringLength);
    } else if (result & VK_STRING_ERROR_BAD_DATA) {
        skip = log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, vuid,
                       "%s: string %s contains invalid characters or is badly formed", apiName, stringName.get_name().c_str());
    }
    return skip;
}

bool StatelessValidation::validate_api_version(uint32_t api_version, uint32_t effective_api_version) {
    bool skip = false;
    uint32_t api_version_nopatch = VK_MAKE_VERSION(VK_VERSION_MAJOR(api_version), VK_VERSION_MINOR(api_version), 0);
    if (api_version_nopatch != effective_api_version) {
        if (api_version_nopatch < VK_API_VERSION_1_0) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT,
                            HandleToUint64(instance), kVUIDUndefined,
                            "Invalid CreateInstance->pCreateInfo->pApplicationInfo.apiVersion number (0x%08x). "
                            "Using VK_API_VERSION_%" PRIu32 "_%" PRIu32 ".",
                            api_version, VK_VERSION_MAJOR(effective_api_version), VK_VERSION_MINOR(effective_api_version));
        } else {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT,
                            HandleToUint64(instance), kVUIDUndefined,
                            "Unrecognized CreateInstance->pCreateInfo->pApplicationInfo.apiVersion number (0x%08x). "
                            "Assuming VK_API_VERSION_%" PRIu32 "_%" PRIu32 ".",
                            api_version, VK_VERSION_MAJOR(effective_api_version), VK_VERSION_MINOR(effective_api_version));
        }
    }
    return skip;
}

bool StatelessValidation::validate_instance_extensions(const VkInstanceCreateInfo *pCreateInfo) {
    bool skip = false;
    for (uint32_t i = 0; i < pCreateInfo->enabledExtensionCount; i++) {
        skip |= validate_extension_reqs(instance_extensions, "VUID-vkCreateInstance-ppEnabledExtensionNames-01388", "instance",
                                        pCreateInfo->ppEnabledExtensionNames[i]);
    }

    return skip;
}

template <typename ExtensionState>
bool extension_state_by_name(const ExtensionState &extensions, const char *extension_name) {
    if (!extension_name) return false;  // null strings specify nothing
    auto info = ExtensionState::get_info(extension_name);
    bool state = info.state ? extensions.*(info.state) : false;  // unknown extensions can't be enabled in extension struct
    return state;
}

bool StatelessValidation::manual_PreCallValidateCreateInstance(const VkInstanceCreateInfo *pCreateInfo,
                                                               const VkAllocationCallbacks *pAllocator, VkInstance *pInstance) {
    bool skip = false;
    // Note: From the spec--
    //  Providing a NULL VkInstanceCreateInfo::pApplicationInfo or providing an apiVersion of 0 is equivalent to providing
    //  an apiVersion of VK_MAKE_VERSION(1, 0, 0).  (a.k.a. VK_API_VERSION_1_0)
    uint32_t local_api_version = (pCreateInfo->pApplicationInfo && pCreateInfo->pApplicationInfo->apiVersion)
                                     ? pCreateInfo->pApplicationInfo->apiVersion
                                     : VK_API_VERSION_1_0;
    skip |= validate_api_version(local_api_version, api_version);
    skip |= validate_instance_extensions(pCreateInfo);
    return skip;
}

void StatelessValidation::PostCallRecordCreateInstance(const VkInstanceCreateInfo *pCreateInfo,
                                                       const VkAllocationCallbacks *pAllocator, VkInstance *pInstance,
                                                       VkResult result) {
    auto instance_data = GetLayerDataPtr(get_dispatch_key(*pInstance), layer_data_map);
    // Copy extension data into local object
    if (result != VK_SUCCESS) return;
    this->instance_extensions = instance_data->instance_extensions;
}

void StatelessValidation::PostCallRecordQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR *pPresentInfo, VkResult result) {
    for (uint32_t i = 0; i < pPresentInfo->swapchainCount; ++i) {
        auto swapchains_result = pPresentInfo->pResults ? pPresentInfo->pResults[i] : result;
        if (swapchains_result == VK_SUBOPTIMAL_KHR) {
            log_msg(report_data, VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT,
                    HandleToUint64(pPresentInfo->pSwapchains[i]), kVUID_PVPerfWarn_SuboptimalSwapchain,
                    "vkQueuePresentKHR: %s :VK_SUBOPTIMAL_KHR was returned. VK_SUBOPTIMAL_KHR - Presentation will still succeed, "
                    "subject to the window resize behavior, but the swapchain is no longer configured optimally for the surface it "
                    "targets. Applications should query updated surface information and recreate their swapchain at the next "
                    "convenient opportunity.",
                    report_data->FormatHandle(pPresentInfo->pSwapchains[i]).c_str());
        }
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
        phys_dev_ext_props.ray_tracing_props = ray_tracing_props;
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
    tmp_features2_state.pNext = SafePnextCopy(pCreateInfo->pNext);
    stateless_validation->physical_device_features2 = tmp_features2_state;
}

bool StatelessValidation::manual_PreCallValidateCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo *pCreateInfo,
                                                             const VkAllocationCallbacks *pAllocator, VkDevice *pDevice) {
    bool skip = false;
    bool maint1 = false;
    bool negative_viewport = false;

    if ((pCreateInfo->enabledLayerCount > 0) && (pCreateInfo->ppEnabledLayerNames != NULL)) {
        for (size_t i = 0; i < pCreateInfo->enabledLayerCount; i++) {
            skip |= validate_string("vkCreateDevice", "pCreateInfo->ppEnabledLayerNames",
                                    "VUID-VkDeviceCreateInfo-ppEnabledLayerNames-parameter", pCreateInfo->ppEnabledLayerNames[i]);
        }
    }

    if ((pCreateInfo->enabledExtensionCount > 0) && (pCreateInfo->ppEnabledExtensionNames != NULL)) {
        maint1 = extension_state_by_name(device_extensions, VK_KHR_MAINTENANCE1_EXTENSION_NAME);
        negative_viewport = extension_state_by_name(device_extensions, VK_AMD_NEGATIVE_VIEWPORT_HEIGHT_EXTENSION_NAME);

        for (size_t i = 0; i < pCreateInfo->enabledExtensionCount; i++) {
            skip |= validate_string("vkCreateDevice", "pCreateInfo->ppEnabledExtensionNames",
                                    "VUID-VkDeviceCreateInfo-ppEnabledExtensionNames-parameter",
                                    pCreateInfo->ppEnabledExtensionNames[i]);
            skip |= validate_extension_reqs(device_extensions, "VUID-vkCreateDevice-ppEnabledExtensionNames-01387", "device",
                                            pCreateInfo->ppEnabledExtensionNames[i]);
        }
    }

    if (maint1 && negative_viewport) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        "VUID-VkDeviceCreateInfo-ppEnabledExtensionNames-00374",
                        "VkDeviceCreateInfo->ppEnabledExtensionNames must not simultaneously include VK_KHR_maintenance1 and "
                        "VK_AMD_negative_viewport_height.");
    }

    if (pCreateInfo->pNext != NULL && pCreateInfo->pEnabledFeatures) {
        // Check for get_physical_device_properties2 struct
        const auto *features2 = lvl_find_in_chain<VkPhysicalDeviceFeatures2KHR>(pCreateInfo->pNext);
        if (features2) {
            // Cannot include VkPhysicalDeviceFeatures2KHR and have non-null pEnabledFeatures
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            kVUID_PVError_InvalidUsage,
                            "VkDeviceCreateInfo->pNext includes a VkPhysicalDeviceFeatures2KHR struct when "
                            "pCreateInfo->pEnabledFeatures is non-NULL.");
        }
    }

    auto features2 = lvl_find_in_chain<VkPhysicalDeviceFeatures2>(pCreateInfo->pNext);
    if (features2) {
        if (!instance_extensions.vk_khr_get_physical_device_properties_2) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            kVUID_PVError_ExtensionNotEnabled,
                            "VkDeviceCreateInfo->pNext includes a VkPhysicalDeviceFeatures2 struct, "
                            "VK_KHR_get_physical_device_properties2 must be enabled when it creates an instance.");
        }
    }

    auto vertex_attribute_divisor_features =
        lvl_find_in_chain<VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT>(pCreateInfo->pNext);
    if (vertex_attribute_divisor_features) {
        bool extension_found = false;
        for (uint32_t i = 0; i < pCreateInfo->enabledExtensionCount; ++i) {
            if (0 == strncmp(pCreateInfo->ppEnabledExtensionNames[i], VK_EXT_VERTEX_ATTRIBUTE_DIVISOR_EXTENSION_NAME,
                             VK_MAX_EXTENSION_NAME_SIZE)) {
                extension_found = true;
                break;
            }
        }
        if (!extension_found) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            kVUID_PVError_ExtensionNotEnabled,
                            "VkDeviceCreateInfo->pNext includes a VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT "
                            "struct, VK_EXT_vertex_attribute_divisor must be enabled when it creates a device.");
        }
    }

    // Validate pCreateInfo->pQueueCreateInfos
    if (pCreateInfo->pQueueCreateInfos) {
        std::unordered_set<uint32_t> set;

        for (uint32_t i = 0; i < pCreateInfo->queueCreateInfoCount; ++i) {
            const uint32_t requested_queue_family = pCreateInfo->pQueueCreateInfos[i].queueFamilyIndex;
            if (requested_queue_family == VK_QUEUE_FAMILY_IGNORED) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT,
                                HandleToUint64(physicalDevice), "VUID-VkDeviceQueueCreateInfo-queueFamilyIndex-00381",
                                "vkCreateDevice: pCreateInfo->pQueueCreateInfos[%" PRIu32
                                "].queueFamilyIndex is VK_QUEUE_FAMILY_IGNORED, but it is required to provide a valid queue family "
                                "index value.",
                                i);
            } else if (set.count(requested_queue_family)) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT,
                                HandleToUint64(physicalDevice), "VUID-VkDeviceCreateInfo-queueFamilyIndex-00372",
                                "vkCreateDevice: pCreateInfo->pQueueCreateInfos[%" PRIu32 "].queueFamilyIndex (=%" PRIu32
                                ") is not unique within pCreateInfo->pQueueCreateInfos array.",
                                i, requested_queue_family);
            } else {
                set.insert(requested_queue_family);
            }

            if (pCreateInfo->pQueueCreateInfos[i].pQueuePriorities != nullptr) {
                for (uint32_t j = 0; j < pCreateInfo->pQueueCreateInfos[i].queueCount; ++j) {
                    const float queue_priority = pCreateInfo->pQueueCreateInfos[i].pQueuePriorities[j];
                    if (!(queue_priority >= 0.f) || !(queue_priority <= 1.f)) {
                        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT,
                                        HandleToUint64(physicalDevice), "VUID-VkDeviceQueueCreateInfo-pQueuePriorities-00383",
                                        "vkCreateDevice: pCreateInfo->pQueueCreateInfos[%" PRIu32 "].pQueuePriorities[%" PRIu32
                                        "] (=%f) is not between 0 and 1 (inclusive).",
                                        i, j, queue_priority);
                    }
                }
            }
        }
    }

    return skip;
}

bool StatelessValidation::require_device_extension(bool flag, char const *function_name, char const *extension_name) {
    if (!flag) {
        return log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                       kVUID_PVError_ExtensionNotEnabled,
                       "%s() called even though the %s extension was not enabled for this VkDevice.", function_name,
                       extension_name);
    }

    return false;
}

bool StatelessValidation::manual_PreCallValidateCreateBuffer(VkDevice device, const VkBufferCreateInfo *pCreateInfo,
                                                             const VkAllocationCallbacks *pAllocator, VkBuffer *pBuffer) {
    bool skip = false;

    const LogMiscParams log_misc{VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, VK_NULL_HANDLE, "vkCreateBuffer"};

    if (pCreateInfo != nullptr) {
        skip |= ValidateGreaterThanZero(pCreateInfo->size, "pCreateInfo->size", "VUID-VkBufferCreateInfo-size-00912", log_misc);

        // Validation for parameters excluded from the generated validation code due to a 'noautovalidity' tag in vk.xml
        if (pCreateInfo->sharingMode == VK_SHARING_MODE_CONCURRENT) {
            // If sharingMode is VK_SHARING_MODE_CONCURRENT, queueFamilyIndexCount must be greater than 1
            if (pCreateInfo->queueFamilyIndexCount <= 1) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                "VUID-VkBufferCreateInfo-sharingMode-00914",
                                "vkCreateBuffer: if pCreateInfo->sharingMode is VK_SHARING_MODE_CONCURRENT, "
                                "pCreateInfo->queueFamilyIndexCount must be greater than 1.");
            }

            // If sharingMode is VK_SHARING_MODE_CONCURRENT, pQueueFamilyIndices must be a pointer to an array of
            // queueFamilyIndexCount uint32_t values
            if (pCreateInfo->pQueueFamilyIndices == nullptr) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                "VUID-VkBufferCreateInfo-sharingMode-00913",
                                "vkCreateBuffer: if pCreateInfo->sharingMode is VK_SHARING_MODE_CONCURRENT, "
                                "pCreateInfo->pQueueFamilyIndices must be a pointer to an array of "
                                "pCreateInfo->queueFamilyIndexCount uint32_t values.");
            }
        }

        // If flags contains VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT or VK_BUFFER_CREATE_SPARSE_ALIASED_BIT, it must also contain
        // VK_BUFFER_CREATE_SPARSE_BINDING_BIT
        if (((pCreateInfo->flags & (VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT | VK_BUFFER_CREATE_SPARSE_ALIASED_BIT)) != 0) &&
            ((pCreateInfo->flags & VK_BUFFER_CREATE_SPARSE_BINDING_BIT) != VK_BUFFER_CREATE_SPARSE_BINDING_BIT)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            "VUID-VkBufferCreateInfo-flags-00918",
                            "vkCreateBuffer: if pCreateInfo->flags contains VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT or "
                            "VK_BUFFER_CREATE_SPARSE_ALIASED_BIT, it must also contain VK_BUFFER_CREATE_SPARSE_BINDING_BIT.");
        }
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCreateImage(VkDevice device, const VkImageCreateInfo *pCreateInfo,
                                                            const VkAllocationCallbacks *pAllocator, VkImage *pImage) {
    bool skip = false;

    const LogMiscParams log_misc{VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, VK_NULL_HANDLE, "vkCreateImage"};

    if (pCreateInfo != nullptr) {
        // Validation for parameters excluded from the generated validation code due to a 'noautovalidity' tag in vk.xml
        if (pCreateInfo->sharingMode == VK_SHARING_MODE_CONCURRENT) {
            // If sharingMode is VK_SHARING_MODE_CONCURRENT, queueFamilyIndexCount must be greater than 1
            if (pCreateInfo->queueFamilyIndexCount <= 1) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                "VUID-VkImageCreateInfo-sharingMode-00942",
                                "vkCreateImage(): if pCreateInfo->sharingMode is VK_SHARING_MODE_CONCURRENT, "
                                "pCreateInfo->queueFamilyIndexCount must be greater than 1.");
            }

            // If sharingMode is VK_SHARING_MODE_CONCURRENT, pQueueFamilyIndices must be a pointer to an array of
            // queueFamilyIndexCount uint32_t values
            if (pCreateInfo->pQueueFamilyIndices == nullptr) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                "VUID-VkImageCreateInfo-sharingMode-00941",
                                "vkCreateImage(): if pCreateInfo->sharingMode is VK_SHARING_MODE_CONCURRENT, "
                                "pCreateInfo->pQueueFamilyIndices must be a pointer to an array of "
                                "pCreateInfo->queueFamilyIndexCount uint32_t values.");
            }
        }

        skip |= ValidateGreaterThanZero(pCreateInfo->extent.width, "pCreateInfo->extent.width",
                                        "VUID-VkImageCreateInfo-extent-00944", log_misc);
        skip |= ValidateGreaterThanZero(pCreateInfo->extent.height, "pCreateInfo->extent.height",
                                        "VUID-VkImageCreateInfo-extent-00945", log_misc);
        skip |= ValidateGreaterThanZero(pCreateInfo->extent.depth, "pCreateInfo->extent.depth",
                                        "VUID-VkImageCreateInfo-extent-00946", log_misc);

        skip |= ValidateGreaterThanZero(pCreateInfo->mipLevels, "pCreateInfo->mipLevels", "VUID-VkImageCreateInfo-mipLevels-00947",
                                        log_misc);
        skip |= ValidateGreaterThanZero(pCreateInfo->arrayLayers, "pCreateInfo->arrayLayers",
                                        "VUID-VkImageCreateInfo-arrayLayers-00948", log_misc);

        // InitialLayout must be PREINITIALIZED or UNDEFINED
        if ((pCreateInfo->initialLayout != VK_IMAGE_LAYOUT_UNDEFINED) &&
            (pCreateInfo->initialLayout != VK_IMAGE_LAYOUT_PREINITIALIZED)) {
            skip |= log_msg(
                report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                "VUID-VkImageCreateInfo-initialLayout-00993",
                "vkCreateImage(): initialLayout is %s, must be VK_IMAGE_LAYOUT_UNDEFINED or VK_IMAGE_LAYOUT_PREINITIALIZED.",
                string_VkImageLayout(pCreateInfo->initialLayout));
        }

        // If imageType is VK_IMAGE_TYPE_1D, both extent.height and extent.depth must be 1
        if ((pCreateInfo->imageType == VK_IMAGE_TYPE_1D) &&
            ((pCreateInfo->extent.height != 1) || (pCreateInfo->extent.depth != 1))) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            "VUID-VkImageCreateInfo-imageType-00956",
                            "vkCreateImage(): if pCreateInfo->imageType is VK_IMAGE_TYPE_1D, both pCreateInfo->extent.height and "
                            "pCreateInfo->extent.depth must be 1.");
        }

        if (pCreateInfo->imageType == VK_IMAGE_TYPE_2D) {
            if (pCreateInfo->flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) {
                if (pCreateInfo->extent.width != pCreateInfo->extent.height) {
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                                    VK_NULL_HANDLE, "VUID-VkImageCreateInfo-imageType-00954",
                                    "vkCreateImage(): pCreateInfo->flags contains VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT, but "
                                    "pCreateInfo->extent.width (=%" PRIu32 ") and pCreateInfo->extent.height (=%" PRIu32
                                    ") are not equal.",
                                    pCreateInfo->extent.width, pCreateInfo->extent.height);
                }

                if (pCreateInfo->arrayLayers < 6) {
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                                    VK_NULL_HANDLE, "VUID-VkImageCreateInfo-imageType-00954",
                                    "vkCreateImage(): pCreateInfo->flags contains VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT, but "
                                    "pCreateInfo->arrayLayers (=%" PRIu32 ") is not greater than or equal to 6.",
                                    pCreateInfo->arrayLayers);
                }
            }

            if (pCreateInfo->extent.depth != 1) {
                skip |=
                    log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            "VUID-VkImageCreateInfo-imageType-00957",
                            "vkCreateImage(): if pCreateInfo->imageType is VK_IMAGE_TYPE_2D, pCreateInfo->extent.depth must be 1.");
            }
        }

        // 3D image may have only 1 layer
        if ((pCreateInfo->imageType == VK_IMAGE_TYPE_3D) && (pCreateInfo->arrayLayers != 1)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            "VUID-VkImageCreateInfo-imageType-00961",
                            "vkCreateImage(): if pCreateInfo->imageType is VK_IMAGE_TYPE_3D, pCreateInfo->arrayLayers must be 1.");
        }

        // If multi-sample, validate type, usage, tiling and mip levels.
        if ((pCreateInfo->samples != VK_SAMPLE_COUNT_1_BIT) &&
            ((pCreateInfo->imageType != VK_IMAGE_TYPE_2D) || (pCreateInfo->flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) ||
             (pCreateInfo->mipLevels != 1) || (pCreateInfo->tiling != VK_IMAGE_TILING_OPTIMAL))) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            "VUID-VkImageCreateInfo-samples-02257",
                            "vkCreateImage(): Multi-sample image with incompatible type, usage, tiling, or mips.");
        }

        if (0 != (pCreateInfo->usage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT)) {
            VkImageUsageFlags legal_flags = (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
                                             VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);
            // At least one of the legal attachment bits must be set
            if (0 == (pCreateInfo->usage & legal_flags)) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                "VUID-VkImageCreateInfo-usage-00966",
                                "vkCreateImage(): Transient attachment image without a compatible attachment flag set.");
            }
            // No flags other than the legal attachment bits may be set
            legal_flags |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
            if (0 != (pCreateInfo->usage & ~legal_flags)) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                "VUID-VkImageCreateInfo-usage-00963",
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
                log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        "VUID-VkImageCreateInfo-mipLevels-00958",
                        "vkCreateImage(): pCreateInfo->mipLevels must be less than or equal to "
                        "floor(log2(max(pCreateInfo->extent.width, pCreateInfo->extent.height, pCreateInfo->extent.depth)))+1.");
        }

        if ((pCreateInfo->flags & VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT) && (pCreateInfo->imageType != VK_IMAGE_TYPE_3D)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, VK_NULL_HANDLE,
                            "VUID-VkImageCreateInfo-flags-00950",
                            "vkCreateImage(): pCreateInfo->flags contains VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT but "
                            "pCreateInfo->imageType is not VK_IMAGE_TYPE_3D.");
        }

        if ((pCreateInfo->flags & VK_IMAGE_CREATE_SPARSE_BINDING_BIT) && (!physical_device_features.sparseBinding)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, VK_NULL_HANDLE,
                            "VUID-VkImageCreateInfo-flags-00969",
                            "vkCreateImage(): pCreateInfo->flags contains VK_IMAGE_CREATE_SPARSE_BINDING_BIT, but the "
                            "VkPhysicalDeviceFeatures::sparseBinding feature is disabled.");
        }

        // If flags contains VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT or VK_IMAGE_CREATE_SPARSE_ALIASED_BIT, it must also contain
        // VK_IMAGE_CREATE_SPARSE_BINDING_BIT
        if (((pCreateInfo->flags & (VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT | VK_IMAGE_CREATE_SPARSE_ALIASED_BIT)) != 0) &&
            ((pCreateInfo->flags & VK_IMAGE_CREATE_SPARSE_BINDING_BIT) != VK_IMAGE_CREATE_SPARSE_BINDING_BIT)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            "VUID-VkImageCreateInfo-flags-00987",
                            "vkCreateImage: if pCreateInfo->flags contains VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT or "
                            "VK_IMAGE_CREATE_SPARSE_ALIASED_BIT, it must also contain VK_IMAGE_CREATE_SPARSE_BINDING_BIT.");
        }

        // Check for combinations of attributes that are incompatible with having VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT set
        if ((pCreateInfo->flags & VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT) != 0) {
            // Linear tiling is unsupported
            if (VK_IMAGE_TILING_LINEAR == pCreateInfo->tiling) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                kVUID_PVError_InvalidUsage,
                                "vkCreateImage: if pCreateInfo->flags contains VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT then image "
                                "tiling of VK_IMAGE_TILING_LINEAR is not supported");
            }

            // Sparse 1D image isn't valid
            if (VK_IMAGE_TYPE_1D == pCreateInfo->imageType) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                "VUID-VkImageCreateInfo-imageType-00970",
                                "vkCreateImage: cannot specify VK_IMAGE_CREATE_SPARSE_BINDING_BIT for 1D image.");
            }

            // Sparse 2D image when device doesn't support it
            if ((VK_FALSE == physical_device_features.sparseResidencyImage2D) && (VK_IMAGE_TYPE_2D == pCreateInfo->imageType)) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                "VUID-VkImageCreateInfo-imageType-00971",
                                "vkCreateImage: cannot specify VK_IMAGE_CREATE_SPARSE_BINDING_BIT for 2D image if corresponding "
                                "feature is not enabled on the device.");
            }

            // Sparse 3D image when device doesn't support it
            if ((VK_FALSE == physical_device_features.sparseResidencyImage3D) && (VK_IMAGE_TYPE_3D == pCreateInfo->imageType)) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                "VUID-VkImageCreateInfo-imageType-00972",
                                "vkCreateImage: cannot specify VK_IMAGE_CREATE_SPARSE_BINDING_BIT for 3D image if corresponding "
                                "feature is not enabled on the device.");
            }

            // Multi-sample 2D image when device doesn't support it
            if (VK_IMAGE_TYPE_2D == pCreateInfo->imageType) {
                if ((VK_FALSE == physical_device_features.sparseResidency2Samples) &&
                    (VK_SAMPLE_COUNT_2_BIT == pCreateInfo->samples)) {
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                    "VUID-VkImageCreateInfo-imageType-00973",
                                    "vkCreateImage: cannot specify VK_IMAGE_CREATE_SPARSE_BINDING_BIT for 2-sample image if "
                                    "corresponding feature is not enabled on the device.");
                } else if ((VK_FALSE == physical_device_features.sparseResidency4Samples) &&
                           (VK_SAMPLE_COUNT_4_BIT == pCreateInfo->samples)) {
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                    "VUID-VkImageCreateInfo-imageType-00974",
                                    "vkCreateImage: cannot specify VK_IMAGE_CREATE_SPARSE_BINDING_BIT for 4-sample image if "
                                    "corresponding feature is not enabled on the device.");
                } else if ((VK_FALSE == physical_device_features.sparseResidency8Samples) &&
                           (VK_SAMPLE_COUNT_8_BIT == pCreateInfo->samples)) {
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                    "VUID-VkImageCreateInfo-imageType-00975",
                                    "vkCreateImage: cannot specify VK_IMAGE_CREATE_SPARSE_BINDING_BIT for 8-sample image if "
                                    "corresponding feature is not enabled on the device.");
                } else if ((VK_FALSE == physical_device_features.sparseResidency16Samples) &&
                           (VK_SAMPLE_COUNT_16_BIT == pCreateInfo->samples)) {
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                    "VUID-VkImageCreateInfo-imageType-00976",
                                    "vkCreateImage: cannot specify VK_IMAGE_CREATE_SPARSE_BINDING_BIT for 16-sample image if "
                                    "corresponding feature is not enabled on the device.");
                }
            }
        }

        if (pCreateInfo->usage & VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV) {
            if (pCreateInfo->imageType != VK_IMAGE_TYPE_2D) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                "VUID-VkImageCreateInfo-imageType-02082",
                                "vkCreateImage: if usage includes VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV, "
                                "imageType must be VK_IMAGE_TYPE_2D.");
            }
            if (pCreateInfo->samples != VK_SAMPLE_COUNT_1_BIT) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                "VUID-VkImageCreateInfo-samples-02083",
                                "vkCreateImage: if usage includes VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV, "
                                "samples must be VK_SAMPLE_COUNT_1_BIT.");
            }
            if (pCreateInfo->tiling != VK_IMAGE_TILING_OPTIMAL) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                "VUID-VkImageCreateInfo-tiling-02084",
                                "vkCreateImage: if usage includes VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV, "
                                "tiling must be VK_IMAGE_TILING_OPTIMAL.");
            }
        }

        if (pCreateInfo->flags & VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV) {
            if (pCreateInfo->imageType != VK_IMAGE_TYPE_2D && pCreateInfo->imageType != VK_IMAGE_TYPE_3D) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                "VUID-VkImageCreateInfo-flags-02050",
                                "vkCreateImage: If flags contains VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV, "
                                "imageType must be VK_IMAGE_TYPE_2D or VK_IMAGE_TYPE_3D.");
            }

            if ((pCreateInfo->flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) || FormatIsDepthOrStencil(pCreateInfo->format)) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                "VUID-VkImageCreateInfo-flags-02051",
                                "vkCreateImage: If flags contains VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV, "
                                "it must not also contain VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT and format must "
                                "not be a depth/stencil format.");
            }

            if (pCreateInfo->imageType == VK_IMAGE_TYPE_2D && (pCreateInfo->extent.width == 1 || pCreateInfo->extent.height == 1)) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                "VUID-VkImageCreateInfo-flags-02052",
                                "vkCreateImage: If flags contains VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV and "
                                "imageType is VK_IMAGE_TYPE_2D, extent.width and extent.height must be "
                                "greater than 1.");
            } else if (pCreateInfo->imageType == VK_IMAGE_TYPE_3D &&
                       (pCreateInfo->extent.width == 1 || pCreateInfo->extent.height == 1 || pCreateInfo->extent.depth == 1)) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                "VUID-VkImageCreateInfo-flags-02053",
                                "vkCreateImage: If flags contains VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV and "
                                "imageType is VK_IMAGE_TYPE_3D, extent.width, extent.height, and extent.depth "
                                "must be greater than 1.");
            }
        }
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateViewport(const VkViewport &viewport, const char *fn_name,
                                                         const ParameterName &parameter_name,
                                                         VkDebugReportObjectTypeEXT object_type, uint64_t object = 0) {
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
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, object_type, object, "VUID-VkViewport-width-01770",
                        "%s: %s.width (=%f) is not greater than 0.0.", fn_name, parameter_name.get_name().c_str(), viewport.width);
    } else if (!(f_lte_u32_exact(viewport.width, max_w) || f_lte_u32_direct(viewport.width, max_w))) {
        width_healthy = false;
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, object_type, object, "VUID-VkViewport-width-01771",
                        "%s: %s.width (=%f) exceeds VkPhysicalDeviceLimits::maxViewportDimensions[0] (=%" PRIu32 ").", fn_name,
                        parameter_name.get_name().c_str(), viewport.width, max_w);
    } else if (!f_lte_u32_exact(viewport.width, max_w) && f_lte_u32_direct(viewport.width, max_w)) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, object_type, object, kVUID_PVError_NONE,
                        "%s: %s.width (=%f) technically exceeds VkPhysicalDeviceLimits::maxViewportDimensions[0] (=%" PRIu32
                        "), but it is within the static_cast<float>(maxViewportDimensions[0]) limit.",
                        fn_name, parameter_name.get_name().c_str(), viewport.width, max_w);
    }

    // height
    bool height_healthy = true;
    const bool negative_height_enabled = api_version >= VK_API_VERSION_1_1 || device_extensions.vk_khr_maintenance1 ||
                                         device_extensions.vk_amd_negative_viewport_height;
    const auto max_h = device_limits.maxViewportDimensions[1];

    if (!negative_height_enabled && !(viewport.height > 0.0f)) {
        height_healthy = false;
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, object_type, object, "VUID-VkViewport-height-01772",
                        "%s: %s.height (=%f) is not greater 0.0.", fn_name, parameter_name.get_name().c_str(), viewport.height);
    } else if (!(f_lte_u32_exact(fabsf(viewport.height), max_h) || f_lte_u32_direct(fabsf(viewport.height), max_h))) {
        height_healthy = false;

        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, object_type, object, "VUID-VkViewport-height-01773",
                        "%s: Absolute value of %s.height (=%f) exceeds VkPhysicalDeviceLimits::maxViewportDimensions[1] (=%" PRIu32
                        ").",
                        fn_name, parameter_name.get_name().c_str(), viewport.height, max_h);
    } else if (!f_lte_u32_exact(fabsf(viewport.height), max_h) && f_lte_u32_direct(fabsf(viewport.height), max_h)) {
        height_healthy = false;

        skip |= log_msg(
            report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, object_type, object, kVUID_PVError_NONE,
            "%s: Absolute value of %s.height (=%f) technically exceeds VkPhysicalDeviceLimits::maxViewportDimensions[1] (=%" PRIu32
            "), but it is within the static_cast<float>(maxViewportDimensions[1]) limit.",
            fn_name, parameter_name.get_name().c_str(), viewport.height, max_h);
    }

    // x
    bool x_healthy = true;
    if (!(viewport.x >= device_limits.viewportBoundsRange[0])) {
        x_healthy = false;
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, object_type, object, "VUID-VkViewport-x-01774",
                        "%s: %s.x (=%f) is less than VkPhysicalDeviceLimits::viewportBoundsRange[0] (=%f).", fn_name,
                        parameter_name.get_name().c_str(), viewport.x, device_limits.viewportBoundsRange[0]);
    }

    // x + width
    if (x_healthy && width_healthy) {
        const float right_bound = viewport.x + viewport.width;
        if (!(right_bound <= device_limits.viewportBoundsRange[1])) {
            skip |=
                log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, object_type, object, "VUID-VkViewport-x-01232",
                        "%s: %s.x + %s.width (=%f + %f = %f) is greater than VkPhysicalDeviceLimits::viewportBoundsRange[1] (=%f).",
                        fn_name, parameter_name.get_name().c_str(), parameter_name.get_name().c_str(), viewport.x, viewport.width,
                        right_bound, device_limits.viewportBoundsRange[1]);
        }
    }

    // y
    bool y_healthy = true;
    if (!(viewport.y >= device_limits.viewportBoundsRange[0])) {
        y_healthy = false;
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, object_type, object, "VUID-VkViewport-y-01775",
                        "%s: %s.y (=%f) is less than VkPhysicalDeviceLimits::viewportBoundsRange[0] (=%f).", fn_name,
                        parameter_name.get_name().c_str(), viewport.y, device_limits.viewportBoundsRange[0]);
    } else if (negative_height_enabled && !(viewport.y <= device_limits.viewportBoundsRange[1])) {
        y_healthy = false;
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, object_type, object, "VUID-VkViewport-y-01776",
                        "%s: %s.y (=%f) exceeds VkPhysicalDeviceLimits::viewportBoundsRange[1] (=%f).", fn_name,
                        parameter_name.get_name().c_str(), viewport.y, device_limits.viewportBoundsRange[1]);
    }

    // y + height
    if (y_healthy && height_healthy) {
        const float boundary = viewport.y + viewport.height;

        if (!(boundary <= device_limits.viewportBoundsRange[1])) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, object_type, object, "VUID-VkViewport-y-01233",
                            "%s: %s.y + %s.height (=%f + %f = %f) exceeds VkPhysicalDeviceLimits::viewportBoundsRange[1] (=%f).",
                            fn_name, parameter_name.get_name().c_str(), parameter_name.get_name().c_str(), viewport.y,
                            viewport.height, boundary, device_limits.viewportBoundsRange[1]);
        } else if (negative_height_enabled && !(boundary >= device_limits.viewportBoundsRange[0])) {
            skip |=
                log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, object_type, object, "VUID-VkViewport-y-01777",
                        "%s: %s.y + %s.height (=%f + %f = %f) is less than VkPhysicalDeviceLimits::viewportBoundsRange[0] (=%f).",
                        fn_name, parameter_name.get_name().c_str(), parameter_name.get_name().c_str(), viewport.y, viewport.height,
                        boundary, device_limits.viewportBoundsRange[0]);
        }
    }

    if (!device_extensions.vk_ext_depth_range_unrestricted) {
        // minDepth
        if (!(viewport.minDepth >= 0.0) || !(viewport.minDepth <= 1.0)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, object_type, object, "VUID-VkViewport-minDepth-01234",

                            "%s: VK_EXT_depth_range_unrestricted extension is not enabled and %s.minDepth (=%f) is not within the "
                            "[0.0, 1.0] range.",
                            fn_name, parameter_name.get_name().c_str(), viewport.minDepth);
        }

        // maxDepth
        if (!(viewport.maxDepth >= 0.0) || !(viewport.maxDepth <= 1.0)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, object_type, object, "VUID-VkViewport-maxDepth-01235",

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

bool StatelessValidation::ValidateCoarseSampleOrderCustomNV(const VkCoarseSampleOrderCustomNV *order) {
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
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        "VUID-VkCoarseSampleOrderCustomNV-shadingRate-02073",
                        "VkCoarseSampleOrderCustomNV shadingRate must be a shading rate "
                        "that generates fragments with more than one pixel.");
        return skip;
    }

    if (order->sampleCount == 0 || (order->sampleCount & (order->sampleCount - 1)) ||
        !(order->sampleCount & device_limits.framebufferNoAttachmentsSampleCounts)) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        "VUID-VkCoarseSampleOrderCustomNV-sampleCount-02074",
                        "VkCoarseSampleOrderCustomNV sampleCount (=%" PRIu32
                        ") must "
                        "correspond to a sample count enumerated in VkSampleCountFlags whose corresponding bit "
                        "is set in framebufferNoAttachmentsSampleCounts.",
                        order->sampleCount);
    }

    if (order->sampleLocationCount != order->sampleCount * sampleOrderInfo->width * sampleOrderInfo->height) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        "VUID-VkCoarseSampleOrderCustomNV-sampleLocationCount-02075",
                        "VkCoarseSampleOrderCustomNV sampleLocationCount (=%" PRIu32
                        ") must "
                        "be equal to the product of sampleCount (=%" PRIu32
                        "), the fragment width for shadingRate "
                        "(=%" PRIu32 "), and the fragment height for shadingRate (=%" PRIu32 ").",
                        order->sampleLocationCount, order->sampleCount, sampleOrderInfo->width, sampleOrderInfo->height);
    }

    if (order->sampleLocationCount > phys_dev_ext_props.shading_rate_image_props.shadingRateMaxCoarseSamples) {
        skip |= log_msg(
            report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
            "VUID-VkCoarseSampleOrderCustomNV-sampleLocationCount-02076",
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
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            "VUID-VkCoarseSampleLocationNV-pixelX-02078",
                            "pixelX must be less than the width (in pixels) of the fragment.");
        }
        if (sampleLoc->pixelY >= sampleOrderInfo->height) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            "VUID-VkCoarseSampleLocationNV-pixelY-02079",
                            "pixelY must be less than the height (in pixels) of the fragment.");
        }
        if (sampleLoc->sample >= order->sampleCount) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            "VUID-VkCoarseSampleLocationNV-sample-02080",
                            "sample must be less than the number of coverage samples in each pixel belonging to the fragment.");
        }
        uint32_t idx = sampleLoc->sample + order->sampleCount * (sampleLoc->pixelX + sampleOrderInfo->width * sampleLoc->pixelY);
        sampleLocationsMask |= 1ULL << idx;
    }

    uint64_t expectedMask = (order->sampleLocationCount == 64) ? ~0ULL : ((1ULL << order->sampleLocationCount) - 1);
    if (sampleLocationsMask != expectedMask) {
        skip |= log_msg(
            report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
            "VUID-VkCoarseSampleOrderCustomNV-pSampleLocations-02077",
            "The array pSampleLocations must contain exactly one entry for "
            "every combination of valid values for pixelX, pixelY, and sample in the structure VkCoarseSampleOrderCustomNV.");
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache,
                                                                        uint32_t createInfoCount,
                                                                        const VkGraphicsPipelineCreateInfo *pCreateInfos,
                                                                        const VkAllocationCallbacks *pAllocator,
                                                                        VkPipeline *pPipelines) {
    bool skip = false;

    if (pCreateInfos != nullptr) {
        for (uint32_t i = 0; i < createInfoCount; ++i) {
            bool has_dynamic_viewport = false;
            bool has_dynamic_scissor = false;
            bool has_dynamic_line_width = false;
            bool has_dynamic_viewport_w_scaling_nv = false;
            bool has_dynamic_discard_rectangle_ext = false;
            bool has_dynamic_sample_locations_ext = false;
            bool has_dynamic_exclusive_scissor_nv = false;
            bool has_dynamic_shading_rate_palette_nv = false;
            bool has_dynamic_line_stipple = false;
            if (pCreateInfos[i].pDynamicState != nullptr) {
                const auto &dynamic_state_info = *pCreateInfos[i].pDynamicState;
                for (uint32_t state_index = 0; state_index < dynamic_state_info.dynamicStateCount; ++state_index) {
                    const auto &dynamic_state = dynamic_state_info.pDynamicStates[state_index];
                    if (dynamic_state == VK_DYNAMIC_STATE_VIEWPORT) has_dynamic_viewport = true;
                    if (dynamic_state == VK_DYNAMIC_STATE_SCISSOR) has_dynamic_scissor = true;
                    if (dynamic_state == VK_DYNAMIC_STATE_LINE_WIDTH) has_dynamic_line_width = true;
                    if (dynamic_state == VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_NV) has_dynamic_viewport_w_scaling_nv = true;
                    if (dynamic_state == VK_DYNAMIC_STATE_DISCARD_RECTANGLE_EXT) has_dynamic_discard_rectangle_ext = true;
                    if (dynamic_state == VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT) has_dynamic_sample_locations_ext = true;
                    if (dynamic_state == VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_NV) has_dynamic_exclusive_scissor_nv = true;
                    if (dynamic_state == VK_DYNAMIC_STATE_VIEWPORT_SHADING_RATE_PALETTE_NV)
                        has_dynamic_shading_rate_palette_nv = true;
                    if (dynamic_state == VK_DYNAMIC_STATE_LINE_STIPPLE_EXT) has_dynamic_line_stipple = true;
                }
            }

            auto feedback_struct = lvl_find_in_chain<VkPipelineCreationFeedbackCreateInfoEXT>(pCreateInfos[i].pNext);
            if ((feedback_struct != nullptr) &&
                (feedback_struct->pipelineStageCreationFeedbackCount != pCreateInfos[i].stageCount)) {
                skip |=
                    log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, VK_NULL_HANDLE,
                            "VUID-VkPipelineCreationFeedbackCreateInfoEXT-pipelineStageCreationFeedbackCount-02668",
                            "vkCreateGraphicsPipelines(): in pCreateInfo[%" PRIu32
                            "], VkPipelineCreationFeedbackEXT::pipelineStageCreationFeedbackCount"
                            "(=%" PRIu32 ") must equal VkGraphicsPipelineCreateInfo::stageCount(=%" PRIu32 ").",
                            i, feedback_struct->pipelineStageCreationFeedbackCount, pCreateInfos[i].stageCount);
            }

            // Validation for parameters excluded from the generated validation code due to a 'noautovalidity' tag in vk.xml

            // Collect active stages
            uint32_t active_shaders = 0;
            for (uint32_t stages = 0; stages < pCreateInfos[i].stageCount; stages++) {
                active_shaders |= pCreateInfos[i].pStages->stage;
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
                                              "VUID-VkPipelineTessellationStateCreateInfo-pNext-pNext");

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
                                              "VUID-VkPipelineInputAssemblyStateCreateInfo-pNext-pNext");

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
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                    "VUID-VkPipelineVertexInputStateCreateInfo-flags-zerobitmask",
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
                                              "VUID-VkPipelineVertexInputStateCreateInfo-pNext-pNext");
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
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                    "VUID-VkPipelineVertexInputStateCreateInfo-vertexBindingDescriptionCount-00613",
                                    "vkCreateGraphicsPipelines: pararameter "
                                    "pCreateInfo[%d].pVertexInputState->vertexBindingDescriptionCount (%u) is "
                                    "greater than VkPhysicalDeviceLimits::maxVertexInputBindings (%u).",
                                    i, vertex_input_state->vertexBindingDescriptionCount, device_limits.maxVertexInputBindings);
                }

                if (vertex_input_state->vertexAttributeDescriptionCount > device_limits.maxVertexInputAttributes) {
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                    "VUID-VkPipelineVertexInputStateCreateInfo-vertexAttributeDescriptionCount-00614",
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
                        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                        "VUID-VkPipelineVertexInputStateCreateInfo-pVertexBindingDescriptions-00616",
                                        "vkCreateGraphicsPipelines: parameter "
                                        "pCreateInfo[%d].pVertexInputState->pVertexBindingDescription[%d].binding "
                                        "(%" PRIu32 ") is not distinct.",
                                        i, d, vertex_bind_desc.binding);
                    }
                    vertex_bindings.insert(vertex_bind_desc.binding);

                    if (vertex_bind_desc.binding >= device_limits.maxVertexInputBindings) {
                        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                        "VUID-VkVertexInputBindingDescription-binding-00618",
                                        "vkCreateGraphicsPipelines: parameter "
                                        "pCreateInfos[%u].pVertexInputState->pVertexBindingDescriptions[%u].binding (%u) is "
                                        "greater than or equal to VkPhysicalDeviceLimits::maxVertexInputBindings (%u).",
                                        i, d, vertex_bind_desc.binding, device_limits.maxVertexInputBindings);
                    }

                    if (vertex_bind_desc.stride > device_limits.maxVertexInputBindingStride) {
                        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                        "VUID-VkVertexInputBindingDescription-stride-00619",
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
                        skip |= log_msg(
                            report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            "VUID-VkPipelineVertexInputStateCreateInfo-pVertexAttributeDescriptions-00617",
                            "vkCreateGraphicsPipelines: parameter "
                            "pCreateInfo[%d].pVertexInputState->vertexAttributeDescriptions[%d].location (%u) is not distinct.",
                            i, d, vertex_attrib_desc.location);
                    }
                    attribute_locations.insert(vertex_attrib_desc.location);

                    auto const &binding_it = vertex_bindings.find(vertex_attrib_desc.binding);
                    if (binding_it == vertex_bindings.cend()) {
                        skip |= log_msg(
                            report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            "VUID-VkPipelineVertexInputStateCreateInfo-binding-00615",
                            "vkCreateGraphicsPipelines: parameter "
                            " pCreateInfo[%d].pVertexInputState->vertexAttributeDescriptions[%d].binding (%u) does not exist "
                            "in any pCreateInfo[%d].pVertexInputState->pVertexBindingDescription.",
                            i, d, vertex_attrib_desc.binding, i);
                    }

                    if (vertex_attrib_desc.location >= device_limits.maxVertexInputAttributes) {
                        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                        "VUID-VkVertexInputAttributeDescription-location-00620",
                                        "vkCreateGraphicsPipelines: parameter "
                                        "pCreateInfos[%u].pVertexInputState->pVertexAttributeDescriptions[%u].location (%u) is "
                                        "greater than or equal to VkPhysicalDeviceLimits::maxVertexInputAttributes (%u).",
                                        i, d, vertex_attrib_desc.location, device_limits.maxVertexInputAttributes);
                    }

                    if (vertex_attrib_desc.binding >= device_limits.maxVertexInputBindings) {
                        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                        "VUID-VkVertexInputAttributeDescription-binding-00621",
                                        "vkCreateGraphicsPipelines: parameter "
                                        "pCreateInfos[%u].pVertexInputState->pVertexAttributeDescriptions[%u].binding (%u) is "
                                        "greater than or equal to VkPhysicalDeviceLimits::maxVertexInputBindings (%u).",
                                        i, d, vertex_attrib_desc.binding, device_limits.maxVertexInputBindings);
                    }

                    if (vertex_attrib_desc.offset > device_limits.maxVertexInputAttributeOffset) {
                        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                        "VUID-VkVertexInputAttributeDescription-offset-00622",
                                        "vkCreateGraphicsPipelines: parameter "
                                        "pCreateInfos[%u].pVertexInputState->pVertexAttributeDescriptions[%u].offset (%u) is "
                                        "greater than VkPhysicalDeviceLimits::maxVertexInputAttributeOffset (%u).",
                                        i, d, vertex_attrib_desc.offset, device_limits.maxVertexInputAttributeOffset);
                    }
                }
            }

            if (pCreateInfos[i].pStages != nullptr) {
                bool has_control = false;
                bool has_eval = false;

                for (uint32_t stage_index = 0; stage_index < pCreateInfos[i].stageCount; ++stage_index) {
                    if (pCreateInfos[i].pStages[stage_index].stage == VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT) {
                        has_control = true;
                    } else if (pCreateInfos[i].pStages[stage_index].stage == VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT) {
                        has_eval = true;
                    }
                }

                // pTessellationState is ignored without both tessellation control and tessellation evaluation shaders stages
                if (has_control && has_eval) {
                    if (pCreateInfos[i].pTessellationState == nullptr) {
                        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                        "VUID-VkGraphicsPipelineCreateInfo-pStages-00731",
                                        "vkCreateGraphicsPipelines: if pCreateInfos[%d].pStages includes a tessellation control "
                                        "shader stage and a tessellation evaluation shader stage, "
                                        "pCreateInfos[%d].pTessellationState must not be NULL.",
                                        i, i);
                    } else {
                        const VkStructureType allowed_type =
                            VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_DOMAIN_ORIGIN_STATE_CREATE_INFO;
                        skip |= validate_struct_pnext(
                            "vkCreateGraphicsPipelines",
                            ParameterName("pCreateInfos[%i].pTessellationState->pNext", ParameterName::IndexVector{i}),
                            "VkPipelineTessellationDomainOriginStateCreateInfo", pCreateInfos[i].pTessellationState->pNext, 1,
                            &allowed_type, GeneratedVulkanHeaderVersion, "VUID-VkGraphicsPipelineCreateInfo-pNext-pNext");

                        skip |= validate_reserved_flags(
                            "vkCreateGraphicsPipelines",
                            ParameterName("pCreateInfos[%i].pTessellationState->flags", ParameterName::IndexVector{i}),
                            pCreateInfos[i].pTessellationState->flags,
                            "VUID-VkPipelineTessellationStateCreateInfo-flags-zerobitmask");

                        if (pCreateInfos[i].pTessellationState->sType !=
                            VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO) {
                            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                            "VUID-VkPipelineTessellationStateCreateInfo-sType-sType",
                                            "vkCreateGraphicsPipelines: parameter pCreateInfos[%d].pTessellationState->sType must "
                                            "be VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO.",
                                            i);
                        }

                        if (pCreateInfos[i].pTessellationState->patchControlPoints == 0 ||
                            pCreateInfos[i].pTessellationState->patchControlPoints > device_limits.maxTessellationPatchSize) {
                            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                            "VUID-VkPipelineTessellationStateCreateInfo-patchControlPoints-01214",
                                            "vkCreateGraphicsPipelines: invalid parameter "
                                            "pCreateInfos[%d].pTessellationState->patchControlPoints value %u. patchControlPoints "
                                            "should be >0 and <=%u.",
                                            i, pCreateInfos[i].pTessellationState->patchControlPoints,
                                            device_limits.maxTessellationPatchSize);
                        }
                    }
                }
            }

            // pViewportState, pMultisampleState, pDepthStencilState, and pColorBlendState ignored when rasterization is disabled
            if ((pCreateInfos[i].pRasterizationState != nullptr) &&
                (pCreateInfos[i].pRasterizationState->rasterizerDiscardEnable == VK_FALSE)) {
                if (pCreateInfos[i].pViewportState == nullptr) {
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT,
                                    VK_NULL_HANDLE, "VUID-VkGraphicsPipelineCreateInfo-rasterizerDiscardEnable-00750",
                                    "vkCreateGraphicsPipelines: Rasterization is enabled (pCreateInfos[%" PRIu32
                                    "].pRasterizationState->rasterizerDiscardEnable is VK_FALSE), but pCreateInfos[%" PRIu32
                                    "].pViewportState (=NULL) is not a valid pointer.",
                                    i, i);
                } else {
                    const auto &viewport_state = *pCreateInfos[i].pViewportState;

                    if (viewport_state.sType != VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO) {
                        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT,
                                        VK_NULL_HANDLE, "VUID-VkPipelineViewportStateCreateInfo-sType-sType",
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
                        allowed_structs_VkPipelineViewportStateCreateInfo, 65,
                        "VUID-VkPipelineViewportStateCreateInfo-pNext-pNext");

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

                    if (!physical_device_features.multiViewport) {
                        if (viewport_state.viewportCount != 1) {
                            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT,
                                            VK_NULL_HANDLE, "VUID-VkPipelineViewportStateCreateInfo-viewportCount-01216",
                                            "vkCreateGraphicsPipelines: The VkPhysicalDeviceFeatures::multiViewport feature is "
                                            "disabled, but pCreateInfos[%" PRIu32 "].pViewportState->viewportCount (=%" PRIu32
                                            ") is not 1.",
                                            i, viewport_state.viewportCount);
                        }

                        if (viewport_state.scissorCount != 1) {
                            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT,
                                            VK_NULL_HANDLE, "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01217",
                                            "vkCreateGraphicsPipelines: The VkPhysicalDeviceFeatures::multiViewport feature is "
                                            "disabled, but pCreateInfos[%" PRIu32 "].pViewportState->scissorCount (=%" PRIu32
                                            ") is not 1.",
                                            i, viewport_state.scissorCount);
                        }

                        if (exclusive_scissor_struct && (exclusive_scissor_struct->exclusiveScissorCount != 0 &&
                                                         exclusive_scissor_struct->exclusiveScissorCount != 1)) {
                            skip |=
                                log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT,
                                        VK_NULL_HANDLE,
                                        "VUID-VkPipelineViewportExclusiveScissorStateCreateInfoNV-exclusiveScissorCount-02027",
                                        "vkCreateGraphicsPipelines: The VkPhysicalDeviceFeatures::multiViewport feature is "
                                        "disabled, but pCreateInfos[%" PRIu32
                                        "] VkPipelineViewportExclusiveScissorStateCreateInfoNV::exclusiveScissorCount (=%" PRIu32
                                        ") is not 1.",
                                        i, exclusive_scissor_struct->exclusiveScissorCount);
                        }

                        if (shading_rate_image_struct &&
                            (shading_rate_image_struct->viewportCount != 0 && shading_rate_image_struct->viewportCount != 1)) {
                            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT,
                                            VK_NULL_HANDLE,
                                            "VUID-VkPipelineViewportShadingRateImageStateCreateInfoNV-viewportCount-02054",
                                            "vkCreateGraphicsPipelines: The VkPhysicalDeviceFeatures::multiViewport feature is "
                                            "disabled, but pCreateInfos[%" PRIu32
                                            "] VkPipelineViewportShadingRateImageStateCreateInfoNV::viewportCount (=%" PRIu32
                                            ") is neither 0 nor 1.",
                                            i, shading_rate_image_struct->viewportCount);
                        }

                    } else {  // multiViewport enabled
                        if (viewport_state.viewportCount == 0) {
                            skip |= log_msg(
                                report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT,
                                VK_NULL_HANDLE, "VUID-VkPipelineViewportStateCreateInfo-viewportCount-arraylength",
                                "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32 "].pViewportState->viewportCount is 0.", i);
                        } else if (viewport_state.viewportCount > device_limits.maxViewports) {
                            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT,
                                            VK_NULL_HANDLE, "VUID-VkPipelineViewportStateCreateInfo-viewportCount-01218",
                                            "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32
                                            "].pViewportState->viewportCount (=%" PRIu32
                                            ") is greater than VkPhysicalDeviceLimits::maxViewports (=%" PRIu32 ").",
                                            i, viewport_state.viewportCount, device_limits.maxViewports);
                        }

                        if (viewport_state.scissorCount == 0) {
                            skip |= log_msg(
                                report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT,
                                VK_NULL_HANDLE, "VUID-VkPipelineViewportStateCreateInfo-scissorCount-arraylength",
                                "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32 "].pViewportState->scissorCount is 0.", i);
                        } else if (viewport_state.scissorCount > device_limits.maxViewports) {
                            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT,
                                            VK_NULL_HANDLE, "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01219",
                                            "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32
                                            "].pViewportState->scissorCount (=%" PRIu32
                                            ") is greater than VkPhysicalDeviceLimits::maxViewports (=%" PRIu32 ").",
                                            i, viewport_state.scissorCount, device_limits.maxViewports);
                        }
                    }

                    if (exclusive_scissor_struct && exclusive_scissor_struct->exclusiveScissorCount > device_limits.maxViewports) {
                        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT,
                                        VK_NULL_HANDLE,
                                        "VUID-VkPipelineViewportExclusiveScissorStateCreateInfoNV-exclusiveScissorCount-02028",
                                        "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32 "] exclusiveScissorCount (=%" PRIu32
                                        ") is greater than VkPhysicalDeviceLimits::maxViewports (=%" PRIu32 ").",
                                        i, exclusive_scissor_struct->exclusiveScissorCount, device_limits.maxViewports);
                    }

                    if (shading_rate_image_struct && shading_rate_image_struct->viewportCount > device_limits.maxViewports) {
                        skip |=
                            log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT,
                                    VK_NULL_HANDLE, "VUID-VkPipelineViewportShadingRateImageStateCreateInfoNV-viewportCount-02055",
                                    "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32
                                    "] VkPipelineViewportShadingRateImageStateCreateInfoNV viewportCount (=%" PRIu32
                                    ") is greater than VkPhysicalDeviceLimits::maxViewports (=%" PRIu32 ").",
                                    i, shading_rate_image_struct->viewportCount, device_limits.maxViewports);
                    }

                    if (viewport_state.scissorCount != viewport_state.viewportCount) {
                        skip |=
                            log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT,
                                    VK_NULL_HANDLE, "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01220",
                                    "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32 "].pViewportState->scissorCount (=%" PRIu32
                                    ") is not identical to pCreateInfos[%" PRIu32 "].pViewportState->viewportCount (=%" PRIu32 ").",
                                    i, viewport_state.scissorCount, i, viewport_state.viewportCount);
                    }

                    if (exclusive_scissor_struct && exclusive_scissor_struct->exclusiveScissorCount != 0 &&
                        exclusive_scissor_struct->exclusiveScissorCount != viewport_state.viewportCount) {
                        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT,
                                        VK_NULL_HANDLE,
                                        "VUID-VkPipelineViewportExclusiveScissorStateCreateInfoNV-exclusiveScissorCount-02029",
                                        "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32 "] exclusiveScissorCount (=%" PRIu32
                                        ") must be zero or identical to pCreateInfos[%" PRIu32
                                        "].pViewportState->viewportCount (=%" PRIu32 ").",
                                        i, exclusive_scissor_struct->exclusiveScissorCount, i, viewport_state.viewportCount);
                    }

                    if (shading_rate_image_struct && shading_rate_image_struct->shadingRateImageEnable &&
                        shading_rate_image_struct->viewportCount != viewport_state.viewportCount) {
                        skip |= log_msg(
                            report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, VK_NULL_HANDLE,
                            "VUID-VkPipelineViewportShadingRateImageStateCreateInfoNV-shadingRateImageEnable-02056",
                            "vkCreateGraphicsPipelines: If shadingRateImageEnable is enabled, pCreateInfos[%" PRIu32
                            "] "
                            "VkPipelineViewportShadingRateImageStateCreateInfoNV viewportCount (=%" PRIu32
                            ") must identical to pCreateInfos[%" PRIu32 "].pViewportState->viewportCount (=%" PRIu32 ").",
                            i, shading_rate_image_struct->viewportCount, i, viewport_state.viewportCount);
                    }

                    if (!has_dynamic_viewport && viewport_state.viewportCount > 0 && viewport_state.pViewports == nullptr) {
                        skip |= log_msg(
                            report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, VK_NULL_HANDLE,
                            "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-00747",
                            "vkCreateGraphicsPipelines: The viewport state is static (pCreateInfos[%" PRIu32
                            "].pDynamicState->pDynamicStates does not contain VK_DYNAMIC_STATE_VIEWPORT), but pCreateInfos[%" PRIu32
                            "].pViewportState->pViewports (=NULL) is an invalid pointer.",
                            i, i);
                    }

                    if (!has_dynamic_scissor && viewport_state.scissorCount > 0 && viewport_state.pScissors == nullptr) {
                        skip |= log_msg(
                            report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, VK_NULL_HANDLE,
                            "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-00748",
                            "vkCreateGraphicsPipelines: The scissor state is static (pCreateInfos[%" PRIu32
                            "].pDynamicState->pDynamicStates does not contain VK_DYNAMIC_STATE_SCISSOR), but pCreateInfos[%" PRIu32
                            "].pViewportState->pScissors (=NULL) is an invalid pointer.",
                            i, i);
                    }

                    if (!has_dynamic_exclusive_scissor_nv && exclusive_scissor_struct &&
                        exclusive_scissor_struct->exclusiveScissorCount > 0 &&
                        exclusive_scissor_struct->pExclusiveScissors == nullptr) {
                        skip |=
                            log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT,
                                    VK_NULL_HANDLE, "VUID-VkPipelineViewportExclusiveScissorStateCreateInfoNV-pDynamicStates-02030",
                                    "vkCreateGraphicsPipelines: The exclusive scissor state is static (pCreateInfos[%" PRIu32
                                    "].pDynamicState->pDynamicStates does not contain VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_NV), but "
                                    "pCreateInfos[%" PRIu32 "] pExclusiveScissors (=NULL) is an invalid pointer.",
                                    i, i);
                    }

                    if (!has_dynamic_shading_rate_palette_nv && shading_rate_image_struct &&
                        shading_rate_image_struct->viewportCount > 0 &&
                        shading_rate_image_struct->pShadingRatePalettes == nullptr) {
                        skip |= log_msg(
                            report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, VK_NULL_HANDLE,
                            "VUID-VkPipelineViewportShadingRateImageStateCreateInfoNV-pDynamicStates-02057",
                            "vkCreateGraphicsPipelines: The shading rate palette state is static (pCreateInfos[%" PRIu32
                            "].pDynamicState->pDynamicStates does not contain VK_DYNAMIC_STATE_VIEWPORT_SHADING_RATE_PALETTE_NV), "
                            "but pCreateInfos[%" PRIu32 "] pShadingRatePalettes (=NULL) is an invalid pointer.",
                            i, i);
                    }

                    if (vp_swizzle_struct) {
                        if (vp_swizzle_struct->viewportCount != viewport_state.viewportCount) {
                            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT,
                                            VK_NULL_HANDLE, "VUID-VkPipelineViewportSwizzleStateCreateInfoNV-viewportCount-01215",
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
                                                                   VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT);
                        }
                    }

                    if (has_dynamic_viewport_w_scaling_nv && !device_extensions.vk_nv_clip_space_w_scaling) {
                        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT,
                                        VK_NULL_HANDLE, kVUID_PVError_ExtensionNotEnabled,
                                        "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32
                                        "].pDynamicState->pDynamicStates contains VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_NV, but "
                                        "VK_NV_clip_space_w_scaling extension is not enabled.",
                                        i);
                    }

                    if (has_dynamic_discard_rectangle_ext && !device_extensions.vk_ext_discard_rectangles) {
                        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT,
                                        VK_NULL_HANDLE, kVUID_PVError_ExtensionNotEnabled,
                                        "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32
                                        "].pDynamicState->pDynamicStates contains VK_DYNAMIC_STATE_DISCARD_RECTANGLE_EXT, but "
                                        "VK_EXT_discard_rectangles extension is not enabled.",
                                        i);
                    }

                    if (has_dynamic_sample_locations_ext && !device_extensions.vk_ext_sample_locations) {
                        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT,
                                        VK_NULL_HANDLE, kVUID_PVError_ExtensionNotEnabled,
                                        "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32
                                        "].pDynamicState->pDynamicStates contains VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT, but "
                                        "VK_EXT_sample_locations extension is not enabled.",
                                        i);
                    }

                    if (has_dynamic_exclusive_scissor_nv && !device_extensions.vk_nv_scissor_exclusive) {
                        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT,
                                        VK_NULL_HANDLE, kVUID_PVError_ExtensionNotEnabled,
                                        "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32
                                        "].pDynamicState->pDynamicStates contains VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_NV, but "
                                        "VK_NV_scissor_exclusive extension is not enabled.",
                                        i);
                    }

                    if (coarse_sample_order_struct &&
                        coarse_sample_order_struct->sampleOrderType != VK_COARSE_SAMPLE_ORDER_TYPE_CUSTOM_NV &&
                        coarse_sample_order_struct->customSampleOrderCount != 0) {
                        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT,
                                        VK_NULL_HANDLE,
                                        "VUID-VkPipelineViewportCoarseSampleOrderStateCreateInfoNV-sampleOrderType-02072",
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
                }

                if (pCreateInfos[i].pMultisampleState == nullptr) {
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                    "VUID-VkGraphicsPipelineCreateInfo-rasterizerDiscardEnable-00751",
                                    "vkCreateGraphicsPipelines: if pCreateInfos[%d].pRasterizationState->rasterizerDiscardEnable "
                                    "is VK_FALSE, pCreateInfos[%d].pMultisampleState must not be NULL.",
                                    i, i);
                } else {
                    const VkStructureType valid_next_stypes[] = {LvlTypeMap<VkPipelineCoverageModulationStateCreateInfoNV>::kSType,
                                                                 LvlTypeMap<VkPipelineCoverageToColorStateCreateInfoNV>::kSType,
                                                                 LvlTypeMap<VkPipelineSampleLocationsStateCreateInfoEXT>::kSType};
                    const char *valid_struct_names =
                        "VkPipelineCoverageModulationStateCreateInfoNV, VkPipelineCoverageToColorStateCreateInfoNV, "
                        "VkPipelineSampleLocationsStateCreateInfoEXT";
                    skip |= validate_struct_pnext(
                        "vkCreateGraphicsPipelines",
                        ParameterName("pCreateInfos[%i].pMultisampleState->pNext", ParameterName::IndexVector{i}),
                        valid_struct_names, pCreateInfos[i].pMultisampleState->pNext, 3, valid_next_stypes,
                        GeneratedVulkanHeaderVersion, "VUID-VkPipelineMultisampleStateCreateInfo-pNext-pNext");

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
                        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                        kVUID_PVError_InvalidStructSType,
                                        "vkCreateGraphicsPipelines: parameter pCreateInfos[%d].pMultisampleState->sType must be "
                                        "VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO",
                                        i);
                    }
                    if (pCreateInfos[i].pMultisampleState->sampleShadingEnable == VK_TRUE) {
                        if (!physical_device_features.sampleRateShading) {
                            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                            "VUID-VkPipelineMultisampleStateCreateInfo-sampleShadingEnable-00784",
                                            "vkCreateGraphicsPipelines(): parameter "
                                            "pCreateInfos[%d].pMultisampleState->sampleShadingEnable.",
                                            i);
                        }
                        // TODO Add documentation issue about when minSampleShading must be in range and when it is ignored
                        // For now a "least noise" test *only* when sampleShadingEnable is VK_TRUE.
                        if (!in_inclusive_range(pCreateInfos[i].pMultisampleState->minSampleShading, 0.F, 1.0F)) {
                            skip |= log_msg(
                                report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
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
                                    log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                            "VUID-VkGraphicsPipelineCreateInfo-lineRasterizationMode-02766",
                                            "vkCreateGraphicsPipelines(): Bresenham/Smooth line rasterization not supported with "
                                            "pCreateInfos[%d].pMultisampleState->alphaToCoverageEnable == VK_TRUE.",
                                            i);
                            }
                            if (pCreateInfos[i].pMultisampleState->alphaToOneEnable) {
                                skip |=
                                    log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                            "VUID-VkGraphicsPipelineCreateInfo-lineRasterizationMode-02766",
                                            "vkCreateGraphicsPipelines(): Bresenham/Smooth line rasterization not supported with "
                                            "pCreateInfos[%d].pMultisampleState->alphaToOneEnable == VK_TRUE.",
                                            i);
                            }
                            if (pCreateInfos[i].pMultisampleState->sampleShadingEnable) {
                                skip |=
                                    log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                            "VUID-VkGraphicsPipelineCreateInfo-lineRasterizationMode-02766",
                                            "vkCreateGraphicsPipelines(): Bresenham/Smooth line rasterization not supported with "
                                            "pCreateInfos[%d].pMultisampleState->sampleShadingEnable == VK_TRUE.",
                                            i);
                            }
                        }
                        if (line_state->stippledLineEnable && !has_dynamic_line_stipple) {
                            if (line_state->lineStippleFactor < 1 || line_state->lineStippleFactor > 256) {
                                skip |=
                                    log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                            "VUID-VkGraphicsPipelineCreateInfo-stippledLineEnable-02767",
                                            "vkCreateGraphicsPipelines(): pCreateInfos[%d] lineStippleFactor = %d must be in the "
                                            "range [1,256].",
                                            i, line_state->lineStippleFactor);
                            }
                        }
                        const auto *line_features =
                            lvl_find_in_chain<VkPhysicalDeviceLineRasterizationFeaturesEXT>(physical_device_features2.pNext);
                        if (line_state->lineRasterizationMode == VK_LINE_RASTERIZATION_MODE_RECTANGULAR_EXT &&
                            (!line_features || !line_features->rectangularLines)) {
                            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                            "VUID-VkPipelineRasterizationLineStateCreateInfoEXT-lineRasterizationMode-02768",
                                            "vkCreateGraphicsPipelines(): pCreateInfos[%d] lineRasterizationMode = "
                                            "VK_LINE_RASTERIZATION_MODE_RECTANGULAR_EXT requires the rectangularLines feature.",
                                            i);
                        }
                        if (line_state->lineRasterizationMode == VK_LINE_RASTERIZATION_MODE_BRESENHAM_EXT &&
                            (!line_features || !line_features->bresenhamLines)) {
                            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                            "VUID-VkPipelineRasterizationLineStateCreateInfoEXT-lineRasterizationMode-02769",
                                            "vkCreateGraphicsPipelines(): pCreateInfos[%d] lineRasterizationMode = "
                                            "VK_LINE_RASTERIZATION_MODE_BRESENHAM_EXT requires the bresenhamLines feature.",
                                            i);
                        }
                        if (line_state->lineRasterizationMode == VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH_EXT &&
                            (!line_features || !line_features->smoothLines)) {
                            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                            "VUID-VkPipelineRasterizationLineStateCreateInfoEXT-lineRasterizationMode-02770",
                                            "vkCreateGraphicsPipelines(): pCreateInfos[%d] lineRasterizationMode = "
                                            "VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH_EXT requires the smoothLines feature.",
                                            i);
                        }
                        if (line_state->stippledLineEnable) {
                            if (line_state->lineRasterizationMode == VK_LINE_RASTERIZATION_MODE_RECTANGULAR_EXT &&
                                (!line_features || !line_features->stippledRectangularLines)) {
                                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT,
                                                0, "VUID-VkPipelineRasterizationLineStateCreateInfoEXT-stippledLineEnable-02771",
                                                "vkCreateGraphicsPipelines(): pCreateInfos[%d] lineRasterizationMode = "
                                                "VK_LINE_RASTERIZATION_MODE_RECTANGULAR_EXT with stipple requires the "
                                                "stippledRectangularLines feature.",
                                                i);
                            }
                            if (line_state->lineRasterizationMode == VK_LINE_RASTERIZATION_MODE_BRESENHAM_EXT &&
                                (!line_features || !line_features->stippledBresenhamLines)) {
                                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT,
                                                0, "VUID-VkPipelineRasterizationLineStateCreateInfoEXT-stippledLineEnable-02772",
                                                "vkCreateGraphicsPipelines(): pCreateInfos[%d] lineRasterizationMode = "
                                                "VK_LINE_RASTERIZATION_MODE_BRESENHAM_EXT with stipple requires the "
                                                "stippledBresenhamLines feature.",
                                                i);
                            }
                            if (line_state->lineRasterizationMode == VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH_EXT &&
                                (!line_features || !line_features->stippledSmoothLines)) {
                                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT,
                                                0, "VUID-VkPipelineRasterizationLineStateCreateInfoEXT-stippledLineEnable-02773",
                                                "vkCreateGraphicsPipelines(): pCreateInfos[%d] lineRasterizationMode = "
                                                "VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH_EXT with stipple requires the "
                                                "stippledSmoothLines feature.",
                                                i);
                            }
                            if (line_state->lineRasterizationMode == VK_LINE_RASTERIZATION_MODE_DEFAULT_EXT &&
                                (!line_features || !line_features->stippledSmoothLines || !device_limits.strictLines)) {
                                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT,
                                                0, "VUID-VkPipelineRasterizationLineStateCreateInfoEXT-stippledLineEnable-02774",
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
                        "VUID-VkPipelineDepthStencilStateCreateInfo-pNext-pNext");

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
                        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                        kVUID_PVError_InvalidStructSType,
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
                        "VUID-VkPipelineColorBlendStateCreateInfo-pNext-pNext");

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
                        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                        kVUID_PVError_InvalidStructSType,
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
                        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                        "VUID-VkGraphicsPipelineCreateInfo-flags-00724",
                                        "vkCreateGraphicsPipelines parameter, pCreateInfos->basePipelineHandle, must be "
                                        "VK_NULL_HANDLE if pCreateInfos->flags contains the VK_PIPELINE_CREATE_DERIVATIVE_BIT flag "
                                        "and pCreateInfos->basePipelineIndex is not -1.");
                    }
                }

                if (pCreateInfos[i].basePipelineHandle != VK_NULL_HANDLE) {
                    if (pCreateInfos[i].basePipelineIndex != -1) {
                        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                        "VUID-VkGraphicsPipelineCreateInfo-flags-00725",
                                        "vkCreateGraphicsPipelines parameter, pCreateInfos->basePipelineIndex, must be -1 if "
                                        "pCreateInfos->flags contains the VK_PIPELINE_CREATE_DERIVATIVE_BIT flag and "
                                        "pCreateInfos->basePipelineHandle is not VK_NULL_HANDLE.");
                    }
                }
            }

            if (pCreateInfos[i].pRasterizationState) {
                if (!device_extensions.vk_nv_fill_rectangle) {
                    if (pCreateInfos[i].pRasterizationState->polygonMode == VK_POLYGON_MODE_FILL_RECTANGLE_NV) {
                        skip |=
                            log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                    "VUID-VkPipelineRasterizationStateCreateInfo-polygonMode-01414",
                                    "vkCreateGraphicsPipelines parameter, VkPolygonMode "
                                    "pCreateInfos->pRasterizationState->polygonMode cannot be VK_POLYGON_MODE_FILL_RECTANGLE_NV "
                                    "if the extension VK_NV_fill_rectangle is not enabled.");
                    } else if ((pCreateInfos[i].pRasterizationState->polygonMode != VK_POLYGON_MODE_FILL) &&
                               (physical_device_features.fillModeNonSolid == false)) {
                        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                        kVUID_PVError_DeviceFeature,
                                        "vkCreateGraphicsPipelines parameter, VkPolygonMode "
                                        "pCreateInfos->pRasterizationState->polygonMode cannot be VK_POLYGON_MODE_POINT or "
                                        "VK_POLYGON_MODE_LINE if VkPhysicalDeviceFeatures->fillModeNonSolid is false.");
                    }
                } else {
                    if ((pCreateInfos[i].pRasterizationState->polygonMode != VK_POLYGON_MODE_FILL) &&
                        (pCreateInfos[i].pRasterizationState->polygonMode != VK_POLYGON_MODE_FILL_RECTANGLE_NV) &&
                        (physical_device_features.fillModeNonSolid == false)) {
                        skip |=
                            log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                    "VUID-VkPipelineRasterizationStateCreateInfo-polygonMode-01507",
                                    "vkCreateGraphicsPipelines parameter, VkPolygonMode "
                                    "pCreateInfos->pRasterizationState->polygonMode must be VK_POLYGON_MODE_FILL or "
                                    "VK_POLYGON_MODE_FILL_RECTANGLE_NV if VkPhysicalDeviceFeatures->fillModeNonSolid is false.");
                    }
                }

                if (!has_dynamic_line_width && !physical_device_features.wideLines &&
                    (pCreateInfos[i].pRasterizationState->lineWidth != 1.0f)) {
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, 0,
                                    "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-00749",
                                    "The line width state is static (pCreateInfos[%" PRIu32
                                    "].pDynamicState->pDynamicStates does not contain VK_DYNAMIC_STATE_LINE_WIDTH) and "
                                    "VkPhysicalDeviceFeatures::wideLines is disabled, but pCreateInfos[%" PRIu32
                                    "].pRasterizationState->lineWidth (=%f) is not 1.0.",
                                    i, i, pCreateInfos[i].pRasterizationState->lineWidth);
                }
            }

            for (size_t j = 0; j < pCreateInfos[i].stageCount; j++) {
                skip |= validate_string("vkCreateGraphicsPipelines",
                                        ParameterName("pCreateInfos[%i].pStages[%i].pName", ParameterName::IndexVector{i, j}),
                                        "VUID-VkGraphicsPipelineCreateInfo-pStages-parameter", pCreateInfos[i].pStages[j].pName);
            }
        }
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache,
                                                                       uint32_t createInfoCount,
                                                                       const VkComputePipelineCreateInfo *pCreateInfos,
                                                                       const VkAllocationCallbacks *pAllocator,
                                                                       VkPipeline *pPipelines) {
    bool skip = false;
    for (uint32_t i = 0; i < createInfoCount; i++) {
        skip |= validate_string("vkCreateComputePipelines",
                                ParameterName("pCreateInfos[%i].stage.pName", ParameterName::IndexVector{i}),
                                "VUID-VkPipelineShaderStageCreateInfo-pName-parameter", pCreateInfos[i].stage.pName);
        auto feedback_struct = lvl_find_in_chain<VkPipelineCreationFeedbackCreateInfoEXT>(pCreateInfos[i].pNext);
        if ((feedback_struct != nullptr) && (feedback_struct->pipelineStageCreationFeedbackCount != 1)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, VK_NULL_HANDLE,
                            "VUID-VkPipelineCreationFeedbackCreateInfoEXT-pipelineStageCreationFeedbackCount-02669",
                            "vkCreateComputePipelines(): in pCreateInfo[%" PRIu32
                            "], VkPipelineCreationFeedbackEXT::pipelineStageCreationFeedbackCount must equal 1, found %" PRIu32 ".",
                            i, feedback_struct->pipelineStageCreationFeedbackCount);
        }
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCreateSampler(VkDevice device, const VkSamplerCreateInfo *pCreateInfo,
                                                              const VkAllocationCallbacks *pAllocator, VkSampler *pSampler) {
    bool skip = false;

    if (pCreateInfo != nullptr) {
        const auto &features = physical_device_features;
        const auto &limits = device_limits;

        if (pCreateInfo->anisotropyEnable == VK_TRUE) {
            if (!in_inclusive_range(pCreateInfo->maxAnisotropy, 1.0F, limits.maxSamplerAnisotropy)) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                "VUID-VkSamplerCreateInfo-anisotropyEnable-01071",
                                "vkCreateSampler(): value of %s must be in range [1.0, %f] %s, but %f found.",
                                "pCreateInfo->maxAnisotropy", limits.maxSamplerAnisotropy,
                                "VkPhysicalDeviceLimits::maxSamplerAnistropy", pCreateInfo->maxAnisotropy);
            }

            // Anistropy cannot be enabled in sampler unless enabled as a feature
            if (features.samplerAnisotropy == VK_FALSE) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                "VUID-VkSamplerCreateInfo-anisotropyEnable-01070",
                                "vkCreateSampler(): Anisotropic sampling feature is not enabled, %s must be VK_FALSE.",
                                "pCreateInfo->anisotropyEnable");
            }
        }

        if (pCreateInfo->unnormalizedCoordinates == VK_TRUE) {
            if (pCreateInfo->minFilter != pCreateInfo->magFilter) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                "VUID-VkSamplerCreateInfo-unnormalizedCoordinates-01072",
                                "vkCreateSampler(): when pCreateInfo->unnormalizedCoordinates is VK_TRUE, "
                                "pCreateInfo->minFilter (%s) and pCreateInfo->magFilter (%s) must be equal.",
                                string_VkFilter(pCreateInfo->minFilter), string_VkFilter(pCreateInfo->magFilter));
            }
            if (pCreateInfo->mipmapMode != VK_SAMPLER_MIPMAP_MODE_NEAREST) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                "VUID-VkSamplerCreateInfo-unnormalizedCoordinates-01073",
                                "vkCreateSampler(): when pCreateInfo->unnormalizedCoordinates is VK_TRUE, "
                                "pCreateInfo->mipmapMode (%s) must be VK_SAMPLER_MIPMAP_MODE_NEAREST.",
                                string_VkSamplerMipmapMode(pCreateInfo->mipmapMode));
            }
            if (pCreateInfo->minLod != 0.0f || pCreateInfo->maxLod != 0.0f) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                "VUID-VkSamplerCreateInfo-unnormalizedCoordinates-01074",
                                "vkCreateSampler(): when pCreateInfo->unnormalizedCoordinates is VK_TRUE, "
                                "pCreateInfo->minLod (%f) and pCreateInfo->maxLod (%f) must both be zero.",
                                pCreateInfo->minLod, pCreateInfo->maxLod);
            }
            if ((pCreateInfo->addressModeU != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE &&
                 pCreateInfo->addressModeU != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER) ||
                (pCreateInfo->addressModeV != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE &&
                 pCreateInfo->addressModeV != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER)) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                "VUID-VkSamplerCreateInfo-unnormalizedCoordinates-01075",
                                "vkCreateSampler(): when pCreateInfo->unnormalizedCoordinates is VK_TRUE, "
                                "pCreateInfo->addressModeU (%s) and pCreateInfo->addressModeV (%s) must both be "
                                "VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE or VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER.",
                                string_VkSamplerAddressMode(pCreateInfo->addressModeU),
                                string_VkSamplerAddressMode(pCreateInfo->addressModeV));
            }
            if (pCreateInfo->anisotropyEnable == VK_TRUE) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                "VUID-VkSamplerCreateInfo-unnormalizedCoordinates-01076",
                                "vkCreateSampler(): pCreateInfo->anisotropyEnable and pCreateInfo->unnormalizedCoordinates must "
                                "not both be VK_TRUE.");
            }
            if (pCreateInfo->compareEnable == VK_TRUE) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                "VUID-VkSamplerCreateInfo-unnormalizedCoordinates-01077",
                                "vkCreateSampler(): pCreateInfo->compareEnable and pCreateInfo->unnormalizedCoordinates must "
                                "not both be VK_TRUE.");
            }
        }

        // If compareEnable is VK_TRUE, compareOp must be a valid VkCompareOp value
        if (pCreateInfo->compareEnable == VK_TRUE) {
            skip |= validate_ranged_enum("vkCreateSampler", "pCreateInfo->compareOp", "VkCompareOp", AllVkCompareOpEnums,
                                         pCreateInfo->compareOp, "VUID-VkSamplerCreateInfo-compareEnable-01080");
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
                log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        "VUID-VkSamplerCreateInfo-addressModeU-01079",
                        "vkCreateSampler(): A VkSamplerAddressMode value is set to VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE "
                        "but the VK_KHR_sampler_mirror_clamp_to_edge extension has not been enabled.");
        }

        // Checks for the IMG cubic filtering extension
        if (device_extensions.vk_img_filter_cubic) {
            if ((pCreateInfo->anisotropyEnable == VK_TRUE) &&
                ((pCreateInfo->minFilter == VK_FILTER_CUBIC_IMG) || (pCreateInfo->magFilter == VK_FILTER_CUBIC_IMG))) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                "VUID-VkSamplerCreateInfo-magFilter-01081",
                                "vkCreateSampler(): Anisotropic sampling must not be VK_TRUE when either minFilter or magFilter "
                                "are VK_FILTER_CUBIC_IMG.");
            }
        }
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCreateDescriptorSetLayout(VkDevice device,
                                                                          const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
                                                                          const VkAllocationCallbacks *pAllocator,
                                                                          VkDescriptorSetLayout *pSetLayout) {
    bool skip = false;

    // Validation for parameters excluded from the generated validation code due to a 'noautovalidity' tag in vk.xml
    if ((pCreateInfo != nullptr) && (pCreateInfo->pBindings != nullptr)) {
        for (uint32_t i = 0; i < pCreateInfo->bindingCount; ++i) {
            if (pCreateInfo->pBindings[i].descriptorCount != 0) {
                // If descriptorType is VK_DESCRIPTOR_TYPE_SAMPLER or VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, and descriptorCount
                // is not 0 and pImmutableSamplers is not NULL, pImmutableSamplers must be a pointer to an array of descriptorCount
                // valid VkSampler handles
                if (((pCreateInfo->pBindings[i].descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER) ||
                     (pCreateInfo->pBindings[i].descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)) &&
                    (pCreateInfo->pBindings[i].pImmutableSamplers != nullptr)) {
                    for (uint32_t descriptor_index = 0; descriptor_index < pCreateInfo->pBindings[i].descriptorCount;
                         ++descriptor_index) {
                        if (pCreateInfo->pBindings[i].pImmutableSamplers[descriptor_index] == VK_NULL_HANDLE) {
                            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                            kVUID_PVError_RequiredParameter,
                                            "vkCreateDescriptorSetLayout: required parameter "
                                            "pCreateInfo->pBindings[%d].pImmutableSamplers[%d] specified as VK_NULL_HANDLE",
                                            i, descriptor_index);
                        }
                    }
                }

                // If descriptorCount is not 0, stageFlags must be a valid combination of VkShaderStageFlagBits values
                if ((pCreateInfo->pBindings[i].stageFlags != 0) &&
                    ((pCreateInfo->pBindings[i].stageFlags & (~AllVkShaderStageFlagBits)) != 0)) {
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                    "VUID-VkDescriptorSetLayoutBinding-descriptorCount-00283",
                                    "vkCreateDescriptorSetLayout(): if pCreateInfo->pBindings[%d].descriptorCount is not 0, "
                                    "pCreateInfo->pBindings[%d].stageFlags must be a valid combination of VkShaderStageFlagBits "
                                    "values.",
                                    i, i);
                }
            }
        }
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool,
                                                                   uint32_t descriptorSetCount,
                                                                   const VkDescriptorSet *pDescriptorSets) {
    // Validation for parameters excluded from the generated validation code due to a 'noautovalidity' tag in vk.xml
    // This is an array of handles, where the elements are allowed to be VK_NULL_HANDLE, and does not require any validation beyond
    // validate_array()
    return validate_array("vkFreeDescriptorSets", "descriptorSetCount", "pDescriptorSets", descriptorSetCount, &pDescriptorSets,
                          true, true, kVUIDUndefined, kVUIDUndefined);
}

bool StatelessValidation::manual_PreCallValidateUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount,
                                                                     const VkWriteDescriptorSet *pDescriptorWrites,
                                                                     uint32_t descriptorCopyCount,
                                                                     const VkCopyDescriptorSet *pDescriptorCopies) {
    bool skip = false;
    // Validation for parameters excluded from the generated validation code due to a 'noautovalidity' tag in vk.xml
    if (pDescriptorWrites != NULL) {
        for (uint32_t i = 0; i < descriptorWriteCount; ++i) {
            // descriptorCount must be greater than 0
            if (pDescriptorWrites[i].descriptorCount == 0) {
                skip |=
                    log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            "VUID-VkWriteDescriptorSet-descriptorCount-arraylength",
                            "vkUpdateDescriptorSets(): parameter pDescriptorWrites[%d].descriptorCount must be greater than 0.", i);
            }

            // dstSet must be a valid VkDescriptorSet handle
            skip |= validate_required_handle("vkUpdateDescriptorSets",
                                             ParameterName("pDescriptorWrites[%i].dstSet", ParameterName::IndexVector{i}),
                                             pDescriptorWrites[i].dstSet);

            if ((pDescriptorWrites[i].descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER) ||
                (pDescriptorWrites[i].descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) ||
                (pDescriptorWrites[i].descriptorType == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE) ||
                (pDescriptorWrites[i].descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE) ||
                (pDescriptorWrites[i].descriptorType == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT)) {
                // If descriptorType is VK_DESCRIPTOR_TYPE_SAMPLER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                // VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE or VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
                // pImageInfo must be a pointer to an array of descriptorCount valid VkDescriptorImageInfo structures
                if (pDescriptorWrites[i].pImageInfo == nullptr) {
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                    "VUID-VkWriteDescriptorSet-descriptorType-00322",
                                    "vkUpdateDescriptorSets(): if pDescriptorWrites[%d].descriptorType is "
                                    "VK_DESCRIPTOR_TYPE_SAMPLER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, "
                                    "VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE or "
                                    "VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, pDescriptorWrites[%d].pImageInfo must not be NULL.",
                                    i, i);
                } else if (pDescriptorWrites[i].descriptorType != VK_DESCRIPTOR_TYPE_SAMPLER) {
                    // If descriptorType is VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                    // VK_DESCRIPTOR_TYPE_STORAGE_IMAGE or VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, the imageView and imageLayout
                    // members of any given element of pImageInfo must be a valid VkImageView and VkImageLayout, respectively
                    for (uint32_t descriptor_index = 0; descriptor_index < pDescriptorWrites[i].descriptorCount;
                         ++descriptor_index) {
                        skip |= validate_required_handle("vkUpdateDescriptorSets",
                                                         ParameterName("pDescriptorWrites[%i].pImageInfo[%i].imageView",
                                                                       ParameterName::IndexVector{i, descriptor_index}),
                                                         pDescriptorWrites[i].pImageInfo[descriptor_index].imageView);
                        skip |= validate_ranged_enum("vkUpdateDescriptorSets",
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
                if (pDescriptorWrites[i].pBufferInfo == nullptr) {
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                    "VUID-VkWriteDescriptorSet-descriptorType-00324",
                                    "vkUpdateDescriptorSets(): if pDescriptorWrites[%d].descriptorType is "
                                    "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, "
                                    "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC or VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, "
                                    "pDescriptorWrites[%d].pBufferInfo must not be NULL.",
                                    i, i);
                } else {
                    for (uint32_t descriptorIndex = 0; descriptorIndex < pDescriptorWrites[i].descriptorCount; ++descriptorIndex) {
                        skip |= validate_required_handle("vkUpdateDescriptorSets",
                                                         ParameterName("pDescriptorWrites[%i].pBufferInfo[%i].buffer",
                                                                       ParameterName::IndexVector{i, descriptorIndex}),
                                                         pDescriptorWrites[i].pBufferInfo[descriptorIndex].buffer);
                    }
                }
            } else if ((pDescriptorWrites[i].descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER) ||
                       (pDescriptorWrites[i].descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER)) {
                // If descriptorType is VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER or VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,
                // pTexelBufferView must be a pointer to an array of descriptorCount valid VkBufferView handles
                if (pDescriptorWrites[i].pTexelBufferView == nullptr) {
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                    "VUID-VkWriteDescriptorSet-descriptorType-00323",
                                    "vkUpdateDescriptorSets(): if pDescriptorWrites[%d].descriptorType is "
                                    "VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER or VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, "
                                    "pDescriptorWrites[%d].pTexelBufferView must not be NULL.",
                                    i, i);
                } else {
                    for (uint32_t descriptor_index = 0; descriptor_index < pDescriptorWrites[i].descriptorCount;
                         ++descriptor_index) {
                        skip |= validate_required_handle("vkUpdateDescriptorSets",
                                                         ParameterName("pDescriptorWrites[%i].pTexelBufferView[%i]",
                                                                       ParameterName::IndexVector{i, descriptor_index}),
                                                         pDescriptorWrites[i].pTexelBufferView[descriptor_index]);
                    }
                }
            }

            if ((pDescriptorWrites[i].descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) ||
                (pDescriptorWrites[i].descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC)) {
                VkDeviceSize uniformAlignment = device_limits.minUniformBufferOffsetAlignment;
                for (uint32_t j = 0; j < pDescriptorWrites[i].descriptorCount; j++) {
                    if (pDescriptorWrites[i].pBufferInfo != NULL) {
                        if (SafeModulo(pDescriptorWrites[i].pBufferInfo[j].offset, uniformAlignment) != 0) {
                            skip |=
                                log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT,
                                        0, "VUID-VkWriteDescriptorSet-descriptorType-00327",
                                        "vkUpdateDescriptorSets(): pDescriptorWrites[%d].pBufferInfo[%d].offset (0x%" PRIxLEAST64
                                        ") must be a multiple of device limit minUniformBufferOffsetAlignment 0x%" PRIxLEAST64 ".",
                                        i, j, pDescriptorWrites[i].pBufferInfo[j].offset, uniformAlignment);
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
                                log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT,
                                        0, "VUID-VkWriteDescriptorSet-descriptorType-00328",
                                        "vkUpdateDescriptorSets(): pDescriptorWrites[%d].pBufferInfo[%d].offset (0x%" PRIxLEAST64
                                        ") must be a multiple of device limit minStorageBufferOffsetAlignment 0x%" PRIxLEAST64 ".",
                                        i, j, pDescriptorWrites[i].pBufferInfo[j].offset, storageAlignment);
                        }
                    }
                }
            }
        }
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo *pCreateInfo,
                                                                 const VkAllocationCallbacks *pAllocator,
                                                                 VkRenderPass *pRenderPass) {
    return CreateRenderPassGeneric(device, pCreateInfo, pAllocator, pRenderPass, RENDER_PASS_VERSION_1);
}

bool StatelessValidation::manual_PreCallValidateCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2KHR *pCreateInfo,
                                                                     const VkAllocationCallbacks *pAllocator,
                                                                     VkRenderPass *pRenderPass) {
    return CreateRenderPassGeneric(device, pCreateInfo, pAllocator, pRenderPass, RENDER_PASS_VERSION_2);
}

bool StatelessValidation::manual_PreCallValidateFreeCommandBuffers(VkDevice device, VkCommandPool commandPool,
                                                                   uint32_t commandBufferCount,
                                                                   const VkCommandBuffer *pCommandBuffers) {
    bool skip = false;

    // Validation for parameters excluded from the generated validation code due to a 'noautovalidity' tag in vk.xml
    // This is an array of handles, where the elements are allowed to be VK_NULL_HANDLE, and does not require any validation beyond
    // validate_array()
    skip |= validate_array("vkFreeCommandBuffers", "commandBufferCount", "pCommandBuffers", commandBufferCount, &pCommandBuffers,
                           true, true, kVUIDUndefined, kVUIDUndefined);
    return skip;
}

bool StatelessValidation::manual_PreCallValidateBeginCommandBuffer(VkCommandBuffer commandBuffer,
                                                                   const VkCommandBufferBeginInfo *pBeginInfo) {
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
            GeneratedVulkanHeaderVersion, "VUID-VkCommandBufferInheritanceInfo-pNext-pNext");

        skip |= validate_bool32(cmd_name, "pBeginInfo->pInheritanceInfo->occlusionQueryEnable", pInfo->occlusionQueryEnable);

        // Explicit VUs
        if (!physical_device_features.inheritedQueries && pInfo->occlusionQueryEnable == VK_TRUE) {
            skip |= log_msg(
                report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                HandleToUint64(commandBuffer), "VUID-VkCommandBufferInheritanceInfo-occlusionQueryEnable-00056",
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
            const auto *cr_features =
                lvl_find_in_chain<VkPhysicalDeviceConditionalRenderingFeaturesEXT>(physical_device_features2.pNext);
            const auto inherited_conditional_rendering = cr_features && cr_features->inheritedConditionalRendering;
            if (!inherited_conditional_rendering && conditional_rendering->conditionalRenderingEnable == VK_TRUE) {
                skip |= log_msg(
                    report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                    HandleToUint64(commandBuffer),
                    "VUID-VkCommandBufferInheritanceConditionalRenderingInfoEXT-conditionalRenderingEnable-01977",
                    "vkBeginCommandBuffer: Inherited conditional rendering is disabled, but "
                    "pBeginInfo->pInheritanceInfo->pNext<VkCommandBufferInheritanceConditionalRenderingInfoEXT> is VK_TRUE.");
            }
        }
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                               uint32_t viewportCount, const VkViewport *pViewports) {
    bool skip = false;

    if (!physical_device_features.multiViewport) {
        if (firstViewport != 0) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(commandBuffer), "VUID-vkCmdSetViewport-firstViewport-01224",
                            "vkCmdSetViewport: The multiViewport feature is disabled, but firstViewport (=%" PRIu32 ") is not 0.",
                            firstViewport);
        }
        if (viewportCount > 1) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(commandBuffer), "VUID-vkCmdSetViewport-viewportCount-01225",
                            "vkCmdSetViewport: The multiViewport feature is disabled, but viewportCount (=%" PRIu32 ") is not 1.",
                            viewportCount);
        }
    } else {  // multiViewport enabled
        const uint64_t sum = static_cast<uint64_t>(firstViewport) + static_cast<uint64_t>(viewportCount);
        if (sum > device_limits.maxViewports) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(commandBuffer), "VUID-vkCmdSetViewport-firstViewport-01223",
                            "vkCmdSetViewport: firstViewport + viewportCount (=%" PRIu32 " + %" PRIu32 " = %" PRIu64
                            ") is greater than VkPhysicalDeviceLimits::maxViewports (=%" PRIu32 ").",
                            firstViewport, viewportCount, sum, device_limits.maxViewports);
        }
    }

    if (pViewports) {
        for (uint32_t viewport_i = 0; viewport_i < viewportCount; ++viewport_i) {
            const auto &viewport = pViewports[viewport_i];  // will crash on invalid ptr
            const char *fn_name = "vkCmdSetViewport";
            skip |= manual_PreCallValidateViewport(viewport, fn_name,
                                                   ParameterName("pViewports[%i]", ParameterName::IndexVector{viewport_i}),
                                                   VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, HandleToUint64(commandBuffer));
        }
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor,
                                                              uint32_t scissorCount, const VkRect2D *pScissors) {
    bool skip = false;

    if (!physical_device_features.multiViewport) {
        if (firstScissor != 0) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(commandBuffer), "VUID-vkCmdSetScissor-firstScissor-00593",
                            "vkCmdSetScissor: The multiViewport feature is disabled, but firstScissor (=%" PRIu32 ") is not 0.",
                            firstScissor);
        }
        if (scissorCount > 1) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(commandBuffer), "VUID-vkCmdSetScissor-scissorCount-00594",
                            "vkCmdSetScissor: The multiViewport feature is disabled, but scissorCount (=%" PRIu32 ") is not 1.",
                            scissorCount);
        }
    } else {  // multiViewport enabled
        const uint64_t sum = static_cast<uint64_t>(firstScissor) + static_cast<uint64_t>(scissorCount);
        if (sum > device_limits.maxViewports) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(commandBuffer), "VUID-vkCmdSetScissor-firstScissor-00592",
                            "vkCmdSetScissor: firstScissor + scissorCount (=%" PRIu32 " + %" PRIu32 " = %" PRIu64
                            ") is greater than VkPhysicalDeviceLimits::maxViewports (=%" PRIu32 ").",
                            firstScissor, scissorCount, sum, device_limits.maxViewports);
        }
    }

    if (pScissors) {
        for (uint32_t scissor_i = 0; scissor_i < scissorCount; ++scissor_i) {
            const auto &scissor = pScissors[scissor_i];  // will crash on invalid ptr

            if (scissor.offset.x < 0) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                                HandleToUint64(commandBuffer), "VUID-vkCmdSetScissor-x-00595",
                                "vkCmdSetScissor: pScissors[%" PRIu32 "].offset.x (=%" PRIi32 ") is negative.", scissor_i,
                                scissor.offset.x);
            }

            if (scissor.offset.y < 0) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                                HandleToUint64(commandBuffer), "VUID-vkCmdSetScissor-x-00595",
                                "vkCmdSetScissor: pScissors[%" PRIu32 "].offset.y (=%" PRIi32 ") is negative.", scissor_i,
                                scissor.offset.y);
            }

            const int64_t x_sum = static_cast<int64_t>(scissor.offset.x) + static_cast<int64_t>(scissor.extent.width);
            if (x_sum > INT32_MAX) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                                HandleToUint64(commandBuffer), "VUID-vkCmdSetScissor-offset-00596",
                                "vkCmdSetScissor: offset.x + extent.width (=%" PRIi32 " + %" PRIu32 " = %" PRIi64
                                ") of pScissors[%" PRIu32 "] will overflow int32_t.",
                                scissor.offset.x, scissor.extent.width, x_sum, scissor_i);
            }

            const int64_t y_sum = static_cast<int64_t>(scissor.offset.y) + static_cast<int64_t>(scissor.extent.height);
            if (y_sum > INT32_MAX) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                                HandleToUint64(commandBuffer), "VUID-vkCmdSetScissor-offset-00597",
                                "vkCmdSetScissor: offset.y + extent.height (=%" PRIi32 " + %" PRIu32 " = %" PRIi64
                                ") of pScissors[%" PRIu32 "] will overflow int32_t.",
                                scissor.offset.y, scissor.extent.height, y_sum, scissor_i);
            }
        }
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth) {
    bool skip = false;

    if (!physical_device_features.wideLines && (lineWidth != 1.0f)) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), "VUID-vkCmdSetLineWidth-lineWidth-00788",
                        "VkPhysicalDeviceFeatures::wideLines is disabled, but lineWidth (=%f) is not 1.0.", lineWidth);
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount,
                                                        uint32_t firstVertex, uint32_t firstInstance) {
    bool skip = false;
    if (vertexCount == 0) {
        // TODO: Verify against Valid Usage section. I don't see a non-zero vertexCount listed, may need to add that and make
        // this an error or leave as is.
        skip |= log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        kVUID_PVError_RequiredParameter, "vkCmdDraw parameter, uint32_t vertexCount, is 0");
    }

    if (instanceCount == 0) {
        // TODO: Verify against Valid Usage section. I don't see a non-zero instanceCount listed, may need to add that and make
        // this an error or leave as is.
        skip |= log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        kVUID_PVError_RequiredParameter, "vkCmdDraw parameter, uint32_t instanceCount, is 0");
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                                uint32_t count, uint32_t stride) {
    bool skip = false;

    if (!physical_device_features.multiDrawIndirect && ((count > 1))) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        kVUID_PVError_DeviceFeature,
                        "CmdDrawIndirect(): Device feature multiDrawIndirect disabled: count must be 0 or 1 but is %d", count);
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                       VkDeviceSize offset, uint32_t count, uint32_t stride) {
    bool skip = false;
    if (!physical_device_features.multiDrawIndirect && ((count > 1))) {
        skip |= log_msg(
            report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, kVUID_PVError_DeviceFeature,
            "CmdDrawIndexedIndirect(): Device feature multiDrawIndirect disabled: count must be 0 or 1 but is %d", count);
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                                                    const VkClearAttachment *pAttachments, uint32_t rectCount,
                                                                    const VkClearRect *pRects) {
    bool skip = false;
    for (uint32_t rect = 0; rect < rectCount; rect++) {
        if (pRects[rect].layerCount == 0) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(commandBuffer), "VUID-vkCmdClearAttachments-layerCount-01934",
                            "CmdClearAttachments(): pRects[%d].layerCount is zero.", rect);
        }
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage,
                                                             VkImageLayout srcImageLayout, VkImage dstImage,
                                                             VkImageLayout dstImageLayout, uint32_t regionCount,
                                                             const VkImageCopy *pRegions) {
    bool skip = false;

    VkImageAspectFlags legal_aspect_flags =
        VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_METADATA_BIT;
    if (device_extensions.vk_khr_sampler_ycbcr_conversion) {
        legal_aspect_flags |= (VK_IMAGE_ASPECT_PLANE_0_BIT_KHR | VK_IMAGE_ASPECT_PLANE_1_BIT_KHR | VK_IMAGE_ASPECT_PLANE_2_BIT_KHR);
    }

    if (pRegions != nullptr) {
        if ((pRegions->srcSubresource.aspectMask & legal_aspect_flags) == 0) {
            skip |= log_msg(
                report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                "VUID-VkImageSubresourceLayers-aspectMask-parameter",
                "vkCmdCopyImage() parameter, VkImageAspect pRegions->srcSubresource.aspectMask, is an unrecognized enumerator.");
        }
        if ((pRegions->dstSubresource.aspectMask & legal_aspect_flags) == 0) {
            skip |= log_msg(
                report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                "VUID-VkImageSubresourceLayers-aspectMask-parameter",
                "vkCmdCopyImage() parameter, VkImageAspect pRegions->dstSubresource.aspectMask, is an unrecognized enumerator.");
        }
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage,
                                                             VkImageLayout srcImageLayout, VkImage dstImage,
                                                             VkImageLayout dstImageLayout, uint32_t regionCount,
                                                             const VkImageBlit *pRegions, VkFilter filter) {
    bool skip = false;

    VkImageAspectFlags legal_aspect_flags =
        VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_METADATA_BIT;
    if (device_extensions.vk_khr_sampler_ycbcr_conversion) {
        legal_aspect_flags |= (VK_IMAGE_ASPECT_PLANE_0_BIT_KHR | VK_IMAGE_ASPECT_PLANE_1_BIT_KHR | VK_IMAGE_ASPECT_PLANE_2_BIT_KHR);
    }

    if (pRegions != nullptr) {
        if ((pRegions->srcSubresource.aspectMask & legal_aspect_flags) == 0) {
            skip |= log_msg(
                report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                kVUID_PVError_UnrecognizedValue,
                "vkCmdBlitImage() parameter, VkImageAspect pRegions->srcSubresource.aspectMask, is an unrecognized enumerator");
        }
        if ((pRegions->dstSubresource.aspectMask & legal_aspect_flags) == 0) {
            skip |= log_msg(
                report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                kVUID_PVError_UnrecognizedValue,
                "vkCmdBlitImage() parameter, VkImageAspect pRegions->dstSubresource.aspectMask, is an unrecognized enumerator");
        }
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer,
                                                                     VkImage dstImage, VkImageLayout dstImageLayout,
                                                                     uint32_t regionCount, const VkBufferImageCopy *pRegions) {
    bool skip = false;

    VkImageAspectFlags legal_aspect_flags =
        VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_METADATA_BIT;
    if (device_extensions.vk_khr_sampler_ycbcr_conversion) {
        legal_aspect_flags |= (VK_IMAGE_ASPECT_PLANE_0_BIT_KHR | VK_IMAGE_ASPECT_PLANE_1_BIT_KHR | VK_IMAGE_ASPECT_PLANE_2_BIT_KHR);
    }

    if (pRegions != nullptr) {
        if ((pRegions->imageSubresource.aspectMask & legal_aspect_flags) == 0) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            kVUID_PVError_UnrecognizedValue,
                            "vkCmdCopyBufferToImage() parameter, VkImageAspect pRegions->imageSubresource.aspectMask, is an "
                            "unrecognized enumerator");
        }
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage,
                                                                     VkImageLayout srcImageLayout, VkBuffer dstBuffer,
                                                                     uint32_t regionCount, const VkBufferImageCopy *pRegions) {
    bool skip = false;

    VkImageAspectFlags legal_aspect_flags =
        VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_METADATA_BIT;
    if (device_extensions.vk_khr_sampler_ycbcr_conversion) {
        legal_aspect_flags |= (VK_IMAGE_ASPECT_PLANE_0_BIT_KHR | VK_IMAGE_ASPECT_PLANE_1_BIT_KHR | VK_IMAGE_ASPECT_PLANE_2_BIT_KHR);
    }

    if (pRegions != nullptr) {
        if ((pRegions->imageSubresource.aspectMask & legal_aspect_flags) == 0) {
            log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                    kVUID_PVError_UnrecognizedValue,
                    "vkCmdCopyImageToBuffer parameter, VkImageAspect pRegions->imageSubresource.aspectMask, is an unrecognized "
                    "enumerator");
        }
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer,
                                                                VkDeviceSize dstOffset, VkDeviceSize dataSize, const void *pData) {
    bool skip = false;

    if (dstOffset & 3) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        "VUID-vkCmdUpdateBuffer-dstOffset-00036",
                        "vkCmdUpdateBuffer() parameter, VkDeviceSize dstOffset (0x%" PRIxLEAST64 "), is not a multiple of 4.",
                        dstOffset);
    }

    if ((dataSize <= 0) || (dataSize > 65536)) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        "VUID-vkCmdUpdateBuffer-dataSize-00037",
                        "vkCmdUpdateBuffer() parameter, VkDeviceSize dataSize (0x%" PRIxLEAST64
                        "), must be greater than zero and less than or equal to 65536.",
                        dataSize);
    } else if (dataSize & 3) {
        skip |=
            log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                    "VUID-vkCmdUpdateBuffer-dataSize-00038",
                    "vkCmdUpdateBuffer() parameter, VkDeviceSize dataSize (0x%" PRIxLEAST64 "), is not a multiple of 4.", dataSize);
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer,
                                                              VkDeviceSize dstOffset, VkDeviceSize size, uint32_t data) {
    bool skip = false;

    if (dstOffset & 3) {
        skip |=
            log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                    "VUID-vkCmdFillBuffer-dstOffset-00025",
                    "vkCmdFillBuffer() parameter, VkDeviceSize dstOffset (0x%" PRIxLEAST64 "), is not a multiple of 4.", dstOffset);
    }

    if (size != VK_WHOLE_SIZE) {
        if (size <= 0) {
            skip |=
                log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        "VUID-vkCmdFillBuffer-size-00026",
                        "vkCmdFillBuffer() parameter, VkDeviceSize size (0x%" PRIxLEAST64 "), must be greater than zero.", size);
        } else if (size & 3) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            "VUID-vkCmdFillBuffer-size-00028",
                            "vkCmdFillBuffer() parameter, VkDeviceSize size (0x%" PRIxLEAST64 "), is not a multiple of 4.", size);
        }
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR *pCreateInfo,
                                                                   const VkAllocationCallbacks *pAllocator,
                                                                   VkSwapchainKHR *pSwapchain) {
    bool skip = false;

    const LogMiscParams log_misc{VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT, VK_NULL_HANDLE, "vkCreateSwapchainKHR"};

    if (pCreateInfo != nullptr) {
        // Validation for parameters excluded from the generated validation code due to a 'noautovalidity' tag in vk.xml
        if (pCreateInfo->imageSharingMode == VK_SHARING_MODE_CONCURRENT) {
            // If imageSharingMode is VK_SHARING_MODE_CONCURRENT, queueFamilyIndexCount must be greater than 1
            if (pCreateInfo->queueFamilyIndexCount <= 1) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                "VUID-VkSwapchainCreateInfoKHR-imageSharingMode-01278",
                                "vkCreateSwapchainKHR(): if pCreateInfo->imageSharingMode is VK_SHARING_MODE_CONCURRENT, "
                                "pCreateInfo->queueFamilyIndexCount must be greater than 1.");
            }

            // If imageSharingMode is VK_SHARING_MODE_CONCURRENT, pQueueFamilyIndices must be a pointer to an array of
            // queueFamilyIndexCount uint32_t values
            if (pCreateInfo->pQueueFamilyIndices == nullptr) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                "VUID-VkSwapchainCreateInfoKHR-imageSharingMode-01277",
                                "vkCreateSwapchainKHR(): if pCreateInfo->imageSharingMode is VK_SHARING_MODE_CONCURRENT, "
                                "pCreateInfo->pQueueFamilyIndices must be a pointer to an array of "
                                "pCreateInfo->queueFamilyIndexCount uint32_t values.");
            }
        }

        skip |= ValidateGreaterThanZero(pCreateInfo->imageArrayLayers, "pCreateInfo->imageArrayLayers",
                                        "VUID-VkSwapchainCreateInfoKHR-imageArrayLayers-01275", log_misc);
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR *pPresentInfo) {
    bool skip = false;

    if (pPresentInfo && pPresentInfo->pNext) {
        const auto *present_regions = lvl_find_in_chain<VkPresentRegionsKHR>(pPresentInfo->pNext);
        if (present_regions) {
            // TODO: This and all other pNext extension dependencies should be added to code-generation
            skip |= require_device_extension(device_extensions.vk_khr_incremental_present, "vkQueuePresentKHR",
                                             VK_KHR_INCREMENTAL_PRESENT_EXTENSION_NAME);
            if (present_regions->swapchainCount != pPresentInfo->swapchainCount) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                kVUID_PVError_InvalidUsage,
                                "QueuePresentKHR(): pPresentInfo->swapchainCount has a value of %i but VkPresentRegionsKHR "
                                "extension swapchainCount is %i. These values must be equal.",
                                pPresentInfo->swapchainCount, present_regions->swapchainCount);
            }
            skip |= validate_struct_pnext("QueuePresentKHR", "pCreateInfo->pNext->pNext", NULL, present_regions->pNext, 0, NULL,
                                          GeneratedVulkanHeaderVersion, "VUID-VkPresentInfoKHR-pNext-pNext");
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
                                                                      VkSurfaceKHR *pSurface) {
    bool skip = false;

    if (pCreateInfo->hwnd == nullptr) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        "VUID-VkWin32SurfaceCreateInfoKHR-hwnd-01308",
                        "vkCreateWin32SurfaceKHR(): hwnd must be a valid Win32 HWND but hwnd is NULL.");
    }

    return skip;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

bool StatelessValidation::manual_PreCallValidateCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo *pCreateInfo,
                                                                     const VkAllocationCallbacks *pAllocator,
                                                                     VkDescriptorPool *pDescriptorPool) {
    bool skip = false;

    if (pCreateInfo) {
        if (pCreateInfo->maxSets <= 0) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT,
                            VK_NULL_HANDLE, "VUID-VkDescriptorPoolCreateInfo-maxSets-00301",
                            "vkCreateDescriptorPool(): pCreateInfo->maxSets is not greater than 0.");
        }

        if (pCreateInfo->pPoolSizes) {
            for (uint32_t i = 0; i < pCreateInfo->poolSizeCount; ++i) {
                if (pCreateInfo->pPoolSizes[i].descriptorCount <= 0) {
                    skip |= log_msg(
                        report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT, VK_NULL_HANDLE,
                        "VUID-VkDescriptorPoolSize-descriptorCount-00302",
                        "vkCreateDescriptorPool(): pCreateInfo->pPoolSizes[%" PRIu32 "].descriptorCount is not greater than 0.", i);
                }
                if (pCreateInfo->pPoolSizes[i].type == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT &&
                    (pCreateInfo->pPoolSizes[i].descriptorCount % 4) != 0) {
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT,
                                    VK_NULL_HANDLE, "VUID-VkDescriptorPoolSize-type-02218",
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
                                                            uint32_t groupCountY, uint32_t groupCountZ) {
    bool skip = false;

    if (groupCountX > device_limits.maxComputeWorkGroupCount[0]) {
        skip |=
            log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                    HandleToUint64(commandBuffer), "VUID-vkCmdDispatch-groupCountX-00386",
                    "vkCmdDispatch(): groupCountX (%" PRIu32 ") exceeds device limit maxComputeWorkGroupCount[0] (%" PRIu32 ").",
                    groupCountX, device_limits.maxComputeWorkGroupCount[0]);
    }

    if (groupCountY > device_limits.maxComputeWorkGroupCount[1]) {
        skip |=
            log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                    HandleToUint64(commandBuffer), "VUID-vkCmdDispatch-groupCountY-00387",
                    "vkCmdDispatch(): groupCountY (%" PRIu32 ") exceeds device limit maxComputeWorkGroupCount[1] (%" PRIu32 ").",
                    groupCountY, device_limits.maxComputeWorkGroupCount[1]);
    }

    if (groupCountZ > device_limits.maxComputeWorkGroupCount[2]) {
        skip |=
            log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                    HandleToUint64(commandBuffer), "VUID-vkCmdDispatch-groupCountZ-00388",
                    "vkCmdDispatch(): groupCountZ (%" PRIu32 ") exceeds device limit maxComputeWorkGroupCount[2] (%" PRIu32 ").",
                    groupCountZ, device_limits.maxComputeWorkGroupCount[2]);
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                    VkDeviceSize offset) {
    bool skip = false;

    if ((offset % 4) != 0) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), "VUID-vkCmdDispatchIndirect-offset-02710",
                        "vkCmdDispatchIndirect(): offset (%" PRIu64 ") must be a multiple of 4.", offset);
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX,
                                                                   uint32_t baseGroupY, uint32_t baseGroupZ, uint32_t groupCountX,
                                                                   uint32_t groupCountY, uint32_t groupCountZ) {
    bool skip = false;

    // Paired if {} else if {} tests used to avoid any possible uint underflow
    uint32_t limit = device_limits.maxComputeWorkGroupCount[0];
    if (baseGroupX >= limit) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), "VUID-vkCmdDispatchBase-baseGroupX-00421",
                        "vkCmdDispatch(): baseGroupX (%" PRIu32
                        ") equals or exceeds device limit maxComputeWorkGroupCount[0] (%" PRIu32 ").",
                        baseGroupX, limit);
    } else if (groupCountX > (limit - baseGroupX)) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), "VUID-vkCmdDispatchBase-groupCountX-00424",
                        "vkCmdDispatchBaseKHR(): baseGroupX (%" PRIu32 ") + groupCountX (%" PRIu32
                        ") exceeds device limit maxComputeWorkGroupCount[0] (%" PRIu32 ").",
                        baseGroupX, groupCountX, limit);
    }

    limit = device_limits.maxComputeWorkGroupCount[1];
    if (baseGroupY >= limit) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), "VUID-vkCmdDispatchBase-baseGroupX-00422",
                        "vkCmdDispatch(): baseGroupY (%" PRIu32
                        ") equals or exceeds device limit maxComputeWorkGroupCount[1] (%" PRIu32 ").",
                        baseGroupY, limit);
    } else if (groupCountY > (limit - baseGroupY)) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), "VUID-vkCmdDispatchBase-groupCountY-00425",
                        "vkCmdDispatchBaseKHR(): baseGroupY (%" PRIu32 ") + groupCountY (%" PRIu32
                        ") exceeds device limit maxComputeWorkGroupCount[1] (%" PRIu32 ").",
                        baseGroupY, groupCountY, limit);
    }

    limit = device_limits.maxComputeWorkGroupCount[2];
    if (baseGroupZ >= limit) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), "VUID-vkCmdDispatchBase-baseGroupZ-00423",
                        "vkCmdDispatch(): baseGroupZ (%" PRIu32
                        ") equals or exceeds device limit maxComputeWorkGroupCount[2] (%" PRIu32 ").",
                        baseGroupZ, limit);
    } else if (groupCountZ > (limit - baseGroupZ)) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), "VUID-vkCmdDispatchBase-groupCountZ-00426",
                        "vkCmdDispatchBaseKHR(): baseGroupZ (%" PRIu32 ") + groupCountZ (%" PRIu32
                        ") exceeds device limit maxComputeWorkGroupCount[2] (%" PRIu32 ").",
                        baseGroupZ, groupCountZ, limit);
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdSetExclusiveScissorNV(VkCommandBuffer commandBuffer,
                                                                         uint32_t firstExclusiveScissor,
                                                                         uint32_t exclusiveScissorCount,
                                                                         const VkRect2D *pExclusiveScissors) {
    bool skip = false;

    if (!physical_device_features.multiViewport) {
        if (firstExclusiveScissor != 0) {
            skip |=
                log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), "VUID-vkCmdSetExclusiveScissorNV-firstExclusiveScissor-02035",
                        "vkCmdSetExclusiveScissorNV: The multiViewport feature is disabled, but firstExclusiveScissor (=%" PRIu32
                        ") is not 0.",
                        firstExclusiveScissor);
        }
        if (exclusiveScissorCount > 1) {
            skip |=
                log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), "VUID-vkCmdSetExclusiveScissorNV-exclusiveScissorCount-02036",
                        "vkCmdSetExclusiveScissorNV: The multiViewport feature is disabled, but exclusiveScissorCount (=%" PRIu32
                        ") is not 1.",
                        exclusiveScissorCount);
        }
    } else {  // multiViewport enabled
        const uint64_t sum = static_cast<uint64_t>(firstExclusiveScissor) + static_cast<uint64_t>(exclusiveScissorCount);
        if (sum > device_limits.maxViewports) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(commandBuffer), "VUID-vkCmdSetExclusiveScissorNV-firstExclusiveScissor-02034",
                            "vkCmdSetExclusiveScissorNV: firstExclusiveScissor + exclusiveScissorCount (=%" PRIu32 " + %" PRIu32
                            " = %" PRIu64 ") is greater than VkPhysicalDeviceLimits::maxViewports (=%" PRIu32 ").",
                            firstExclusiveScissor, exclusiveScissorCount, sum, device_limits.maxViewports);
        }
    }

    if (firstExclusiveScissor >= device_limits.maxViewports) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), "VUID-vkCmdSetExclusiveScissorNV-firstExclusiveScissor-02033",
                        "vkCmdSetExclusiveScissorNV: firstExclusiveScissor (=%" PRIu32 ") must be less than maxViewports (=%" PRIu32
                        ").",
                        firstExclusiveScissor, device_limits.maxViewports);
    }

    if (pExclusiveScissors) {
        for (uint32_t scissor_i = 0; scissor_i < exclusiveScissorCount; ++scissor_i) {
            const auto &scissor = pExclusiveScissors[scissor_i];  // will crash on invalid ptr

            if (scissor.offset.x < 0) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                                HandleToUint64(commandBuffer), "VUID-vkCmdSetExclusiveScissorNV-x-02037",
                                "vkCmdSetExclusiveScissorNV: pScissors[%" PRIu32 "].offset.x (=%" PRIi32 ") is negative.",
                                scissor_i, scissor.offset.x);
            }

            if (scissor.offset.y < 0) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                                HandleToUint64(commandBuffer), "VUID-vkCmdSetExclusiveScissorNV-x-02037",
                                "vkCmdSetExclusiveScissorNV: pScissors[%" PRIu32 "].offset.y (=%" PRIi32 ") is negative.",
                                scissor_i, scissor.offset.y);
            }

            const int64_t x_sum = static_cast<int64_t>(scissor.offset.x) + static_cast<int64_t>(scissor.extent.width);
            if (x_sum > INT32_MAX) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                                HandleToUint64(commandBuffer), "VUID-vkCmdSetExclusiveScissorNV-offset-02038",
                                "vkCmdSetExclusiveScissorNV: offset.x + extent.width (=%" PRIi32 " + %" PRIu32 " = %" PRIi64
                                ") of pScissors[%" PRIu32 "] will overflow int32_t.",
                                scissor.offset.x, scissor.extent.width, x_sum, scissor_i);
            }

            const int64_t y_sum = static_cast<int64_t>(scissor.offset.y) + static_cast<int64_t>(scissor.extent.height);
            if (y_sum > INT32_MAX) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                                HandleToUint64(commandBuffer), "VUID-vkCmdSetExclusiveScissorNV-offset-02039",
                                "vkCmdSetExclusiveScissorNV: offset.y + extent.height (=%" PRIi32 " + %" PRIu32 " = %" PRIi64
                                ") of pScissors[%" PRIu32 "] will overflow int32_t.",
                                scissor.offset.y, scissor.extent.height, y_sum, scissor_i);
            }
        }
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdSetViewportShadingRatePaletteNV(
    VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
    const VkShadingRatePaletteNV *pShadingRatePalettes) {
    bool skip = false;

    if (!physical_device_features.multiViewport) {
        if (firstViewport != 0) {
            skip |=
                log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), "VUID-vkCmdSetViewportShadingRatePaletteNV-firstViewport-02068",
                        "vkCmdSetViewportShadingRatePaletteNV: The multiViewport feature is disabled, but firstViewport (=%" PRIu32
                        ") is not 0.",
                        firstViewport);
        }
        if (viewportCount > 1) {
            skip |=
                log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), "VUID-vkCmdSetViewportShadingRatePaletteNV-viewportCount-02069",
                        "vkCmdSetViewportShadingRatePaletteNV: The multiViewport feature is disabled, but viewportCount (=%" PRIu32
                        ") is not 1.",
                        viewportCount);
        }
    }

    if (firstViewport >= device_limits.maxViewports) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), "VUID-vkCmdSetViewportShadingRatePaletteNV-firstViewport-02066",
                        "vkCmdSetViewportShadingRatePaletteNV: firstViewport (=%" PRIu32
                        ") must be less than maxViewports (=%" PRIu32 ").",
                        firstViewport, device_limits.maxViewports);
    }

    const uint64_t sum = static_cast<uint64_t>(firstViewport) + static_cast<uint64_t>(viewportCount);
    if (sum > device_limits.maxViewports) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), "VUID-vkCmdSetViewportShadingRatePaletteNV-firstViewport-02067",
                        "vkCmdSetViewportShadingRatePaletteNV: firstViewport + viewportCount (=%" PRIu32 " + %" PRIu32 " = %" PRIu64
                        ") is greater than VkPhysicalDeviceLimits::maxViewports (=%" PRIu32 ").",
                        firstViewport, viewportCount, sum, device_limits.maxViewports);
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdSetCoarseSampleOrderNV(VkCommandBuffer commandBuffer,
                                                                          VkCoarseSampleOrderTypeNV sampleOrderType,
                                                                          uint32_t customSampleOrderCount,
                                                                          const VkCoarseSampleOrderCustomNV *pCustomSampleOrders) {
    bool skip = false;

    if (sampleOrderType != VK_COARSE_SAMPLE_ORDER_TYPE_CUSTOM_NV && customSampleOrderCount != 0) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), "VUID-vkCmdSetCoarseSampleOrderNV-sampleOrderType-02081",
                        "vkCmdSetCoarseSampleOrderNV: If sampleOrderType is not VK_COARSE_SAMPLE_ORDER_TYPE_CUSTOM_NV, "
                        "customSampleOrderCount must be 0.");
    }

    for (uint32_t order_i = 0; order_i < customSampleOrderCount; ++order_i) {
        skip |= ValidateCoarseSampleOrderCustomNV(&pCustomSampleOrders[order_i]);
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount,
                                                                   uint32_t firstTask) {
    bool skip = false;

    if (taskCount > phys_dev_ext_props.mesh_shader_props.maxDrawMeshTasksCount) {
        skip |= log_msg(
            report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
            HandleToUint64(commandBuffer), "VUID-vkCmdDrawMeshTasksNV-taskCount-02119",
            "vkCmdDrawMeshTasksNV() parameter, uint32_t taskCount (0x%" PRIxLEAST32
            "), must be less than or equal to VkPhysicalDeviceMeshShaderPropertiesNV::maxDrawMeshTasksCount (0x%" PRIxLEAST32 ").",
            taskCount, phys_dev_ext_props.mesh_shader_props.maxDrawMeshTasksCount);
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                           VkDeviceSize offset, uint32_t drawCount,
                                                                           uint32_t stride) {
    bool skip = false;
    static const int condition_multiples = 0b0011;
    if (offset & condition_multiples) {
        skip |= log_msg(
            report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
            HandleToUint64(commandBuffer), "VUID-vkCmdDrawMeshTasksIndirectNV-offset-02710",
            "vkCmdDrawMeshTasksIndirectNV() parameter, VkDeviceSize offset (0x%" PRIxLEAST64 "), is not a multiple of 4.", offset);
    }
    if (drawCount > 1 && ((stride & condition_multiples) || stride < sizeof(VkDrawMeshTasksIndirectCommandNV))) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), "VUID-vkCmdDrawMeshTasksIndirectNV-drawCount-02146",
                        "vkCmdDrawMeshTasksIndirectNV() parameter, uint32_t stride (0x%" PRIxLEAST32
                        "), is not a multiple of 4 or smaller than sizeof (VkDrawMeshTasksIndirectCommandNV).",
                        stride);
    }
    if (!physical_device_features.multiDrawIndirect && ((drawCount > 1))) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), "VUID-vkCmdDrawMeshTasksIndirectNV-drawCount-02718",
                        "vkCmdDrawMeshTasksIndirectNV(): Device feature multiDrawIndirect disabled: count must be 0 or 1 but is %d",
                        drawCount);
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                                VkDeviceSize offset, VkBuffer countBuffer,
                                                                                VkDeviceSize countBufferOffset,
                                                                                uint32_t maxDrawCount, uint32_t stride) {
    bool skip = false;

    if (offset & 3) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), "VUID-vkCmdDrawMeshTasksIndirectCountNV-offset-02710",
                        "vkCmdDrawMeshTasksIndirectCountNV() parameter, VkDeviceSize offset (0x%" PRIxLEAST64
                        "), is not a multiple of 4.",
                        offset);
    }

    if (countBufferOffset & 3) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), "VUID-vkCmdDrawMeshTasksIndirectCountNV-countBufferOffset-02716",
                        "vkCmdDrawMeshTasksIndirectCountNV() parameter, VkDeviceSize countBufferOffset (0x%" PRIxLEAST64
                        "), is not a multiple of 4.",
                        countBufferOffset);
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo *pCreateInfo,
                                                                const VkAllocationCallbacks *pAllocator, VkQueryPool *pQueryPool) {
    bool skip = false;

    // Validation for parameters excluded from the generated validation code due to a 'noautovalidity' tag in vk.xml
    if (pCreateInfo != nullptr) {
        // If queryType is VK_QUERY_TYPE_PIPELINE_STATISTICS, pipelineStatistics must be a valid combination of
        // VkQueryPipelineStatisticFlagBits values
        if ((pCreateInfo->queryType == VK_QUERY_TYPE_PIPELINE_STATISTICS) && (pCreateInfo->pipelineStatistics != 0) &&
            ((pCreateInfo->pipelineStatistics & (~AllVkQueryPipelineStatisticFlagBits)) != 0)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            "VUID-VkQueryPoolCreateInfo-queryType-00792",
                            "vkCreateQueryPool(): if pCreateInfo->queryType is VK_QUERY_TYPE_PIPELINE_STATISTICS, "
                            "pCreateInfo->pipelineStatistics must be a valid combination of VkQueryPipelineStatisticFlagBits "
                            "values.");
        }
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice,
                                                                                   const char *pLayerName, uint32_t *pPropertyCount,
                                                                                   VkExtensionProperties *pProperties) {
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
                                                               const VkAllocationCallbacks *pAllocator, VkDeviceMemory *pMemory) {
    bool skip = false;

    if (pAllocateInfo) {
        auto chained_prio_struct = lvl_find_in_chain<VkMemoryPriorityAllocateInfoEXT>(pAllocateInfo->pNext);
        if (chained_prio_struct && (chained_prio_struct->priority < 0.0f || chained_prio_struct->priority > 1.0f)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            "VUID-VkMemoryPriorityAllocateInfoEXT-priority-02602",
                            "priority (=%f) must be between `0` and `1`, inclusive.", chained_prio_struct->priority);
        }
    }
    return skip;
}

bool StatelessValidation::ValidateGeometryTrianglesNV(const VkGeometryTrianglesNV &triangles,
                                                      VkDebugReportObjectTypeEXT object_type, uint64_t object_handle,
                                                      const char *func_name) const {
    bool skip = false;

    if (triangles.vertexFormat != VK_FORMAT_R32G32B32_SFLOAT && triangles.vertexFormat != VK_FORMAT_R16G16B16_SFLOAT &&
        triangles.vertexFormat != VK_FORMAT_R16G16B16_SNORM && triangles.vertexFormat != VK_FORMAT_R32G32_SFLOAT &&
        triangles.vertexFormat != VK_FORMAT_R16G16_SFLOAT && triangles.vertexFormat != VK_FORMAT_R16G16_SNORM) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, object_type, object_handle,
                        "VUID-VkGeometryTrianglesNV-vertexFormat-02430", "%s", func_name);
    } else {
        uint32_t vertex_component_size = 0;
        if (triangles.vertexFormat == VK_FORMAT_R32G32B32_SFLOAT || triangles.vertexFormat == VK_FORMAT_R32G32_SFLOAT) {
            vertex_component_size = 4;
        } else if (triangles.vertexFormat == VK_FORMAT_R16G16B16_SFLOAT || triangles.vertexFormat == VK_FORMAT_R16G16B16_SNORM ||
                   triangles.vertexFormat == VK_FORMAT_R16G16_SFLOAT || triangles.vertexFormat == VK_FORMAT_R16G16_SNORM) {
            vertex_component_size = 2;
        }
        if (vertex_component_size > 0 && SafeModulo(triangles.vertexOffset, vertex_component_size) != 0) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, object_type, object_handle,
                            "VUID-VkGeometryTrianglesNV-vertexOffset-02429", "%s", func_name);
        }
    }

    if (triangles.indexType != VK_INDEX_TYPE_UINT32 && triangles.indexType != VK_INDEX_TYPE_UINT16 &&
        triangles.indexType != VK_INDEX_TYPE_NONE_NV) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, object_type, object_handle,
                        "VUID-VkGeometryTrianglesNV-indexType-02433", "%s", func_name);
    } else {
        uint32_t index_element_size = 0;
        if (triangles.indexType == VK_INDEX_TYPE_UINT32) {
            index_element_size = 4;
        } else if (triangles.indexType == VK_INDEX_TYPE_UINT16) {
            index_element_size = 2;
        }
        if (index_element_size > 0 && SafeModulo(triangles.indexOffset, index_element_size) != 0) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, object_type, object_handle,
                            "VUID-VkGeometryTrianglesNV-indexOffset-02432", "%s", func_name);
        }
    }
    if (triangles.indexType == VK_INDEX_TYPE_NONE_NV) {
        if (triangles.indexCount != 0) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, object_type, object_handle,
                            "VUID-VkGeometryTrianglesNV-indexCount-02436", "%s", func_name);
        }
        if (triangles.indexData != VK_NULL_HANDLE) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, object_type, object_handle,
                            "VUID-VkGeometryTrianglesNV-indexData-02434", "%s", func_name);
        }
    }

    if (SafeModulo(triangles.transformOffset, 16) != 0) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, object_type, object_handle,
                        "VUID-VkGeometryTrianglesNV-transformOffset-02438", "%s", func_name);
    }

    return skip;
}

bool StatelessValidation::ValidateGeometryAABBNV(const VkGeometryAABBNV &aabbs, VkDebugReportObjectTypeEXT object_type,
                                                 uint64_t object_handle, const char *func_name) const {
    bool skip = false;

    if (SafeModulo(aabbs.offset, 8) != 0) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, object_type, object_handle,
                        "VUID-VkGeometryAABBNV-offset-02440", "%s", func_name);
    }
    if (SafeModulo(aabbs.stride, 8) != 0) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, object_type, object_handle,
                        "VUID-VkGeometryAABBNV-stride-02441", "%s", func_name);
    }

    return skip;
}

bool StatelessValidation::ValidateGeometryNV(const VkGeometryNV &geometry, VkDebugReportObjectTypeEXT object_type,
                                             uint64_t object_handle, const char *func_name) const {
    bool skip = false;
    if (geometry.geometryType == VK_GEOMETRY_TYPE_TRIANGLES_NV) {
        skip = ValidateGeometryTrianglesNV(geometry.geometry.triangles, object_type, object_handle, func_name);
    } else if (geometry.geometryType == VK_GEOMETRY_TYPE_AABBS_NV) {
        skip = ValidateGeometryAABBNV(geometry.geometry.aabbs, object_type, object_handle, func_name);
    }
    return skip;
}

bool StatelessValidation::ValidateAccelerationStructureInfoNV(const VkAccelerationStructureInfoNV &info,
                                                              VkDebugReportObjectTypeEXT object_type, uint64_t object_handle,
                                                              const char *func_name) const {
    bool skip = false;
    if (info.type == VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV && info.geometryCount != 0) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, object_type, object_handle,
                        "VUID-VkAccelerationStructureInfoNV-type-02425",
                        "VkAccelerationStructureInfoNV: If type is VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV then "
                        "geometryCount must be 0.");
    }
    if (info.type == VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV && info.instanceCount != 0) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, object_type, object_handle,
                        "VUID-VkAccelerationStructureInfoNV-type-02426",
                        "VkAccelerationStructureInfoNV: If type is VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV then "
                        "instanceCount must be 0.");
    }
    if (info.flags & VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_NV &&
        info.flags & VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_NV) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, object_type, object_handle,
                        "VUID-VkAccelerationStructureInfoNV-flags-02592",
                        "VkAccelerationStructureInfoNV: If flags has the VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_NV"
                        "bit set, then it must not have the VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_NV bit set.");
    }
    if (info.geometryCount > phys_dev_ext_props.ray_tracing_props.maxGeometryCount) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, object_type, object_handle,
                        "VUID-VkAccelerationStructureInfoNV-geometryCount-02422",
                        "VkAccelerationStructureInfoNV: geometryCount must be less than or equal to "
                        "VkPhysicalDeviceRayTracingPropertiesNV::maxGeometryCount.");
    }
    if (info.instanceCount > phys_dev_ext_props.ray_tracing_props.maxInstanceCount) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, object_type, object_handle,
                        "VUID-VkAccelerationStructureInfoNV-instanceCount-02423",
                        "VkAccelerationStructureInfoNV: instanceCount must be less than or equal to "
                        "VkPhysicalDeviceRayTracingPropertiesNV::maxInstanceCount.");
    }
    if (info.type == VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV && info.geometryCount > 0) {
        uint64_t total_triangle_count = 0;
        for (uint32_t i = 0; i < info.geometryCount; i++) {
            const VkGeometryNV &geometry = info.pGeometries[i];

            skip |= ValidateGeometryNV(geometry, object_type, object_handle, func_name);

            if (geometry.geometryType != VK_GEOMETRY_TYPE_TRIANGLES_NV) {
                continue;
            }
            total_triangle_count += geometry.geometry.triangles.indexCount / 3;
        }
        if (total_triangle_count > phys_dev_ext_props.ray_tracing_props.maxTriangleCount) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, object_type, object_handle,
                            "VUID-VkAccelerationStructureInfoNV-maxTriangleCount-02424",
                            "VkAccelerationStructureInfoNV: The total number of triangles in all geometries must be less than "
                            "or equal to VkPhysicalDeviceRayTracingPropertiesNV::maxTriangleCount.");
        }
    }
    if (info.type == VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV && info.geometryCount > 1) {
        const VkGeometryTypeNV first_geometry_type = info.pGeometries[0].geometryType;
        for (uint32_t i = 1; i < info.geometryCount; i++) {
            const VkGeometryNV &geometry = info.pGeometries[i];
            if (geometry.geometryType != first_geometry_type) {
                // TODO: update fake VUID below with the real one once it is generated.
                skip |=
                    log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV_EXT,
                            0, "UNASSIGNED-VkAccelerationStructureInfoNV-pGeometries-XXXX",
                            "VkAccelerationStructureInfoNV: info.pGeometries[%d].geometryType does not match "
                            "info.pGeometries[0].geometryType.",
                            i);
            }
        }
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCreateAccelerationStructureNV(
    VkDevice device, const VkAccelerationStructureCreateInfoNV *pCreateInfo, const VkAllocationCallbacks *pAllocator,
    VkAccelerationStructureNV *pAccelerationStructure) {
    bool skip = false;

    if (pCreateInfo) {
        if ((pCreateInfo->compactedSize != 0) &&
            ((pCreateInfo->info.geometryCount != 0) || (pCreateInfo->info.instanceCount != 0))) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            "VUID-VkAccelerationStructureCreateInfoNV-compactedSize-02421",
                            "vkCreateAccelerationStructureNV(): pCreateInfo->compactedSize nonzero (%" PRIu64
                            ") with info.geometryCount (%" PRIu32 ") or info.instanceCount (%" PRIu32 ") nonzero.",
                            pCreateInfo->compactedSize, pCreateInfo->info.geometryCount, pCreateInfo->info.instanceCount);
        }

        skip |= ValidateAccelerationStructureInfoNV(pCreateInfo->info, VK_DEBUG_REPORT_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV_EXT, 0,
                                                    "vkCreateAccelerationStructureNV()");
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdBuildAccelerationStructureNV(
    VkCommandBuffer commandBuffer, const VkAccelerationStructureInfoNV *pInfo, VkBuffer instanceData, VkDeviceSize instanceOffset,
    VkBool32 update, VkAccelerationStructureNV dst, VkAccelerationStructureNV src, VkBuffer scratch, VkDeviceSize scratchOffset) {
    bool skip = false;

    if (pInfo != nullptr) {
        skip |= ValidateAccelerationStructureInfoNV(*pInfo, VK_DEBUG_REPORT_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV_EXT,
                                                    HandleToUint64(dst), "vkCmdBuildAccelerationStructureNV()");
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateGetAccelerationStructureHandleNV(VkDevice device,
                                                                                 VkAccelerationStructureNV accelerationStructure,
                                                                                 size_t dataSize, void *pData) {
    bool skip = false;
    if (dataSize < 8) {
        skip = log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV_EXT,
                       HandleToUint64(accelerationStructure), "VUID-vkGetAccelerationStructureHandleNV-dataSize-02240",
                       "vkGetAccelerationStructureHandleNV(): dataSize must be greater than or equal to 8.");
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache,
                                                                            uint32_t createInfoCount,
                                                                            const VkRayTracingPipelineCreateInfoNV *pCreateInfos,
                                                                            const VkAllocationCallbacks *pAllocator,
                                                                            VkPipeline *pPipelines) {
    bool skip = false;

    for (uint32_t i = 0; i < createInfoCount; i++) {
        auto feedback_struct = lvl_find_in_chain<VkPipelineCreationFeedbackCreateInfoEXT>(pCreateInfos[i].pNext);
        if ((feedback_struct != nullptr) && (feedback_struct->pipelineStageCreationFeedbackCount != pCreateInfos[i].stageCount)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, VK_NULL_HANDLE,
                            "VUID-VkPipelineCreationFeedbackCreateInfoEXT-pipelineStageCreationFeedbackCount-02670",
                            "vkCreateRayTracingPipelinesNV(): in pCreateInfo[%" PRIu32
                            "], VkPipelineCreationFeedbackEXT::pipelineStageCreationFeedbackCount"
                            "(=%" PRIu32 ") must equal VkRayTracingPipelineCreateInfoNV::stageCount(=%" PRIu32 ").",
                            i, feedback_struct->pipelineStageCreationFeedbackCount, pCreateInfos[i].stageCount);
        }
    }

    return skip;
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
bool StatelessValidation::PreCallValidateGetDeviceGroupSurfacePresentModes2EXT(VkDevice device,
                                                                               const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo,
                                                                               VkDeviceGroupPresentModeFlagsKHR *pModes) {
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
                                      "VUID-VkPhysicalDeviceSurfaceInfo2KHR-pNext-pNext");

        skip |= validate_required_handle("vkGetDeviceGroupSurfacePresentModes2EXT", "pSurfaceInfo->surface", pSurfaceInfo->surface);
    }
    return skip;
}
#endif

bool StatelessValidation::manual_PreCallValidateCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo *pCreateInfo,
                                                                  const VkAllocationCallbacks *pAllocator,
                                                                  VkFramebuffer *pFramebuffer) {
    // Validation for pAttachments which is excluded from the generated validation code due to a 'noautovalidity' tag in vk.xml
    bool skip = false;
    if ((pCreateInfo->flags & VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT_KHR) == 0) {
        skip |= validate_array("vkCreateFramebuffer", "attachmentCount", "pAttachments", pCreateInfo->attachmentCount,
                               &pCreateInfo->pAttachments, false, true, kVUIDUndefined, kVUIDUndefined);
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdSetLineStippleEXT(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor,
                                                                     uint16_t lineStipplePattern) {
    bool skip = false;

    if (lineStippleFactor < 1 || lineStippleFactor > 256) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), "VUID-vkCmdSetLineStippleEXT-lineStippleFactor-02776",
                        "vkCmdSetLineStippleEXT::lineStippleFactor=%d is not in [1,256].", lineStippleFactor);
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                   VkDeviceSize offset, VkIndexType indexType) {
    bool skip = false;

    if (indexType == VK_INDEX_TYPE_NONE_NV) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), "VUID-vkCmdBindIndexBuffer-indexType-02507",
                        "vkCmdBindIndexBuffer() indexType must not be VK_INDEX_TYPE_NONE_NV.");
    }

    const auto *index_type_uint8_features =
        lvl_find_in_chain<VkPhysicalDeviceIndexTypeUint8FeaturesEXT>(physical_device_features2.pNext);
    if (indexType == VK_INDEX_TYPE_UINT8_EXT && !index_type_uint8_features->indexTypeUint8) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), "VUID-vkCmdBindIndexBuffer-indexType-02765",
                        "vkCmdBindIndexBuffer() indexType is VK_INDEX_TYPE_UINT8_EXT but indexTypeUint8 feature is not enabled.");
    }

    return skip;
}
