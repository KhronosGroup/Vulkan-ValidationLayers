/* Copyright (c) 2015-2017 The Khronos Group Inc.
 * Copyright (c) 2015-2017 Valve Corporation
 * Copyright (c) 2015-2017 LunarG, Inc.
 * Copyright (C) 2015-2017 Google Inc.
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
 */

#define NOMINMAX
#define VALIDATION_ERROR_MAP_IMPL

#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <mutex>

#include "vk_loader_platform.h"
#include "vulkan/vk_layer.h"
#include "vk_layer_config.h"
#include "vk_dispatch_table_helper.h"
#include "vk_typemap_helper.h"

#include "vk_layer_data.h"
#include "vk_layer_logging.h"
#include "vk_layer_extension_utils.h"
#include "vk_layer_utils.h"
#include "vk_layer_dispatch_table.h"

#include "parameter_name.h"
#include "parameter_validation.h"

#if defined __ANDROID__
#include <android/log.h>
#define LOGCONSOLE(...) ((void)__android_log_print(ANDROID_LOG_INFO, "PARAMETER_VALIDATION", __VA_ARGS__))
#else
#define LOGCONSOLE(...)      \
    {                        \
        printf(__VA_ARGS__); \
        printf("\n");        \
    }
#endif

namespace parameter_validation {

extern std::unordered_map<std::string, void *> custom_functions;

extern bool parameter_validation_vkCreateInstance(VkInstance instance, const VkInstanceCreateInfo *pCreateInfo,
                                                  const VkAllocationCallbacks *pAllocator, VkInstance *pInstance);
extern bool parameter_validation_vkDestroyInstance(VkInstance instance, const VkAllocationCallbacks *pAllocator);
extern bool parameter_validation_vkCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo *pCreateInfo,
                                                const VkAllocationCallbacks *pAllocator, VkDevice *pDevice);
extern bool parameter_validation_vkDestroyDevice(VkDevice device, const VkAllocationCallbacks *pAllocator);
extern bool parameter_validation_vkCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo *pCreateInfo,
                                                   const VkAllocationCallbacks *pAllocator, VkQueryPool *pQueryPool);
extern bool parameter_validation_vkCreateDebugReportCallbackEXT(VkInstance instance,
                                                                const VkDebugReportCallbackCreateInfoEXT *pCreateInfo,
                                                                const VkAllocationCallbacks *pAllocator,
                                                                VkDebugReportCallbackEXT *pMsgCallback);
extern bool parameter_validation_vkDestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT msgCallback,
                                                                 const VkAllocationCallbacks *pAllocator);
extern bool parameter_validation_vkCreateDebugUtilsMessengerEXT(VkInstance instance,
                                                                const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                                                const VkAllocationCallbacks *pAllocator,
                                                                VkDebugUtilsMessengerEXT *pMessenger);
extern bool parameter_validation_vkDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger,
                                                                 const VkAllocationCallbacks *pAllocator);
extern bool parameter_validation_vkCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo *pCreateInfo,
                                                     const VkAllocationCallbacks *pAllocator, VkCommandPool *pCommandPool);
extern bool parameter_validation_vkCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo *pCreateInfo,
                                                    const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass);
extern bool parameter_validation_vkDestroyRenderPass(VkDevice device, VkRenderPass renderPass,
                                                     const VkAllocationCallbacks *pAllocator);

// TODO : This can be much smarter, using separate locks for separate global data
std::mutex global_lock;

static uint32_t loader_layer_if_version = CURRENT_LOADER_LAYER_INTERFACE_VERSION;
std::unordered_map<void *, layer_data *> layer_data_map;
std::unordered_map<void *, instance_layer_data *> instance_layer_data_map;

void InitializeManualParameterValidationFunctionPointers(void);

static void init_parameter_validation(instance_layer_data *instance_data, const VkAllocationCallbacks *pAllocator) {
    layer_debug_report_actions(instance_data->report_data, instance_data->logging_callback, pAllocator,
                               "lunarg_parameter_validation");
    layer_debug_messenger_actions(instance_data->report_data, instance_data->logging_messenger, pAllocator,
                                  "lunarg_parameter_validation");
}

static const VkExtensionProperties instance_extensions[] = {{VK_EXT_DEBUG_REPORT_EXTENSION_NAME, VK_EXT_DEBUG_REPORT_SPEC_VERSION},
                                                            {VK_EXT_DEBUG_UTILS_EXTENSION_NAME, VK_EXT_DEBUG_UTILS_SPEC_VERSION}};

static const VkLayerProperties global_layer = {
    "VK_LAYER_LUNARG_parameter_validation",
    VK_LAYER_API_VERSION,
    1,
    "LunarG Validation Layer",
};

static const int MaxParamCheckerStringLength = 256;

template <typename T>
static inline bool in_inclusive_range(const T &value, const T &min, const T &max) {
    // Using only < for generality and || for early abort
    return !((value < min) || (max < value));
}

static bool validate_string(debug_report_data *report_data, const char *apiName, const ParameterName &stringName,
                            const std::string &vuid, const char *validateString) {
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

static bool ValidateDeviceQueueFamily(layer_data *device_data, uint32_t queue_family, const char *cmd_name,
                                      const char *parameter_name, const std::string &error_code, bool optional = false) {
    bool skip = false;

    if (!optional && queue_family == VK_QUEUE_FAMILY_IGNORED) {
        skip |= log_msg(device_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT,
                        HandleToUint64(device_data->device), error_code,
                        "%s: %s is VK_QUEUE_FAMILY_IGNORED, but it is required to provide a valid queue family index value.",
                        cmd_name, parameter_name);
    } else if (device_data->queueFamilyIndexMap.find(queue_family) == device_data->queueFamilyIndexMap.end()) {
        skip |=
            log_msg(device_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT,
                    HandleToUint64(device_data->device), error_code,
                    "%s: %s (= %" PRIu32
                    ") is not one of the queue families given via VkDeviceQueueCreateInfo structures when the device was created.",
                    cmd_name, parameter_name, queue_family);
    }

    return skip;
}

static bool ValidateQueueFamilies(layer_data *device_data, uint32_t queue_family_count, const uint32_t *queue_families,
                                  const char *cmd_name, const char *array_parameter_name, const std::string &unique_error_code,
                                  const std::string &valid_error_code, bool optional = false) {
    bool skip = false;
    if (queue_families) {
        std::unordered_set<uint32_t> set;
        for (uint32_t i = 0; i < queue_family_count; ++i) {
            std::string parameter_name = std::string(array_parameter_name) + "[" + std::to_string(i) + "]";

            if (set.count(queue_families[i])) {
                skip |= log_msg(device_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT,
                                HandleToUint64(device_data->device), "VUID-VkDeviceCreateInfo-queueFamilyIndex-00372",
                                "%s: %s (=%" PRIu32 ") is not unique within %s array.", cmd_name, parameter_name.c_str(),
                                queue_families[i], array_parameter_name);
            } else {
                set.insert(queue_families[i]);
                skip |= ValidateDeviceQueueFamily(device_data, queue_families[i], cmd_name, parameter_name.c_str(),
                                                  valid_error_code, optional);
            }
        }
    }
    return skip;
}

static bool validate_api_version(const instance_layer_data *instance_data, uint32_t api_version, uint32_t effective_api_version) {
    bool skip = false;
    uint32_t api_version_nopatch = VK_MAKE_VERSION(VK_VERSION_MAJOR(api_version), VK_VERSION_MINOR(api_version), 0);
    if (api_version_nopatch != effective_api_version) {
        if (api_version_nopatch < VK_API_VERSION_1_0) {
            skip |= log_msg(instance_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT,
                            HandleToUint64(instance_data->instance), kVUIDUndefined,
                            "Invalid CreateInstance->pCreateInfo->pApplicationInfo.apiVersion number (0x%08x). "
                            "Using VK_API_VERSION_%" PRIu32 "_%" PRIu32 ".",
                            api_version, VK_VERSION_MAJOR(effective_api_version), VK_VERSION_MINOR(effective_api_version));
        } else {
            skip |= log_msg(instance_data->report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT,
                            HandleToUint64(instance_data->instance), kVUIDUndefined,
                            "Unrecognized CreateInstance->pCreateInfo->pApplicationInfo.apiVersion number (0x%08x). "
                            "Assuming VK_API_VERSION_%" PRIu32 "_%" PRIu32 ".",
                            api_version, VK_VERSION_MAJOR(effective_api_version), VK_VERSION_MINOR(effective_api_version));
        }
    }
    return skip;
}

template <typename ExtensionState>
static bool validate_extension_reqs(const instance_layer_data *instance_data, const ExtensionState &extensions,
                                    const std::string &vuid, const char *extension_type, const char *extension_name) {
    bool skip = false;
    if (!extension_name) {
        return skip;  // Robust to invalid char *
    }
    auto info = ExtensionState::get_info(extension_name);

    if (!info.state) {
        return skip;  // Unknown extensions cannot be checked so report OK
    }

    // Check agains the reqs list in the info
    std::vector<const char *> missing;
    for (const auto &req : info.requires) {
        if (!(extensions.*(req.enabled))) {
            missing.push_back(req.name);
        }
    }

    // Report any missing requirements
    if (missing.size()) {
        std::string missing_joined_list = string_join(", ", missing);
        skip |= log_msg(instance_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT,
                        HandleToUint64(instance_data->instance), vuid, "Missing required extensions for %s extension %s, %s.",
                        extension_type, extension_name, missing_joined_list.c_str());
    }
    return skip;
}

bool validate_instance_extensions(const instance_layer_data *instance_data, const VkInstanceCreateInfo *pCreateInfo) {
    bool skip = false;
    for (uint32_t i = 0; i < pCreateInfo->enabledExtensionCount; i++) {
        skip |=
            validate_extension_reqs(instance_data, instance_data->extensions, "VUID-vkCreateInstance-ppEnabledExtensionNames-01388",
                                    "instance", pCreateInfo->ppEnabledExtensionNames[i]);
    }

    return skip;
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateInstance(const VkInstanceCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator,
                                                VkInstance *pInstance) {
    VkResult result = VK_ERROR_VALIDATION_FAILED_EXT;

    VkLayerInstanceCreateInfo *chain_info = get_chain_info(pCreateInfo, VK_LAYER_LINK_INFO);
    assert(chain_info != nullptr);
    assert(chain_info->u.pLayerInfo != nullptr);

    PFN_vkGetInstanceProcAddr fpGetInstanceProcAddr = chain_info->u.pLayerInfo->pfnNextGetInstanceProcAddr;
    PFN_vkCreateInstance fpCreateInstance = (PFN_vkCreateInstance)fpGetInstanceProcAddr(NULL, "vkCreateInstance");
    if (fpCreateInstance == NULL) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    // Advance the link info for the next element on the chain
    chain_info->u.pLayerInfo = chain_info->u.pLayerInfo->pNext;

    result = fpCreateInstance(pCreateInfo, pAllocator, pInstance);

    if (result == VK_SUCCESS) {
        InitializeManualParameterValidationFunctionPointers();
        auto my_instance_data = GetLayerDataPtr(get_dispatch_key(*pInstance), instance_layer_data_map);
        assert(my_instance_data != nullptr);

        layer_init_instance_dispatch_table(*pInstance, &my_instance_data->dispatch_table, fpGetInstanceProcAddr);
        my_instance_data->instance = *pInstance;
        my_instance_data->report_data =
            debug_utils_create_instance(&my_instance_data->dispatch_table, *pInstance, pCreateInfo->enabledExtensionCount,
                                        pCreateInfo->ppEnabledExtensionNames);

        // Look for one or more debug report create info structures
        // and setup a callback(s) for each one found.
        if (!layer_copy_tmp_debug_messengers(pCreateInfo->pNext, &my_instance_data->num_tmp_debug_messengers,
                                             &my_instance_data->tmp_messenger_create_infos,
                                             &my_instance_data->tmp_debug_messengers)) {
            if (my_instance_data->num_tmp_debug_messengers > 0) {
                // Setup the temporary callback(s) here to catch early issues:
                if (layer_enable_tmp_debug_messengers(my_instance_data->report_data, my_instance_data->num_tmp_debug_messengers,
                                                      my_instance_data->tmp_messenger_create_infos,
                                                      my_instance_data->tmp_debug_messengers)) {
                    // Failure of setting up one or more of the callback.
                    // Therefore, clean up and don't use those callbacks:
                    layer_free_tmp_debug_messengers(my_instance_data->tmp_messenger_create_infos,
                                                    my_instance_data->tmp_debug_messengers);
                    my_instance_data->num_tmp_debug_messengers = 0;
                }
            }
        }
        if (!layer_copy_tmp_report_callbacks(pCreateInfo->pNext, &my_instance_data->num_tmp_report_callbacks,
                                             &my_instance_data->tmp_report_create_infos, &my_instance_data->tmp_report_callbacks)) {
            if (my_instance_data->num_tmp_report_callbacks > 0) {
                // Setup the temporary callback(s) here to catch early issues:
                if (layer_enable_tmp_report_callbacks(my_instance_data->report_data, my_instance_data->num_tmp_report_callbacks,
                                                      my_instance_data->tmp_report_create_infos,
                                                      my_instance_data->tmp_report_callbacks)) {
                    // Failure of setting up one or more of the callback.
                    // Therefore, clean up and don't use those callbacks:
                    layer_free_tmp_report_callbacks(my_instance_data->tmp_report_create_infos,
                                                    my_instance_data->tmp_report_callbacks);
                    my_instance_data->num_tmp_report_callbacks = 0;
                }
            }
        }

        init_parameter_validation(my_instance_data, pAllocator);
        // Note: From the spec--
        //  Providing a NULL VkInstanceCreateInfo::pApplicationInfo or providing an apiVersion of 0 is equivalent to providing
        //  an apiVersion of VK_MAKE_VERSION(1, 0, 0).  (a.k.a. VK_API_VERSION_1_0)
        uint32_t api_version = (pCreateInfo->pApplicationInfo && pCreateInfo->pApplicationInfo->apiVersion)
                                   ? pCreateInfo->pApplicationInfo->apiVersion
                                   : VK_API_VERSION_1_0;

        uint32_t effective_api_version = my_instance_data->extensions.InitFromInstanceCreateInfo(api_version, pCreateInfo);

        // Ordinarily we'd check these before calling down the chain, but none of the layer support is in place until now, if we
        // survive we can report the issue now.
        validate_api_version(my_instance_data, api_version, effective_api_version);
        validate_instance_extensions(my_instance_data, pCreateInfo);

        parameter_validation_vkCreateInstance(*pInstance, pCreateInfo, pAllocator, pInstance);

        if (pCreateInfo->pApplicationInfo) {
            if (pCreateInfo->pApplicationInfo->pApplicationName) {
                validate_string(
                    my_instance_data->report_data, "vkCreateInstance", "pCreateInfo->VkApplicationInfo->pApplicationName",
                    "VUID-VkApplicationInfo-pApplicationName-parameter", pCreateInfo->pApplicationInfo->pApplicationName);
            }

            if (pCreateInfo->pApplicationInfo->pEngineName) {
                validate_string(my_instance_data->report_data, "vkCreateInstance", "pCreateInfo->VkApplicationInfo->pEngineName",
                                "VUID-VkApplicationInfo-pEngineName-parameter", pCreateInfo->pApplicationInfo->pEngineName);
            }
        }

        // Disable the tmp callbacks:
        if (my_instance_data->num_tmp_debug_messengers > 0) {
            layer_disable_tmp_debug_messengers(my_instance_data->report_data, my_instance_data->num_tmp_debug_messengers,
                                               my_instance_data->tmp_debug_messengers);
        }
        if (my_instance_data->num_tmp_report_callbacks > 0) {
            layer_disable_tmp_report_callbacks(my_instance_data->report_data, my_instance_data->num_tmp_report_callbacks,
                                               my_instance_data->tmp_report_callbacks);
        }
    }

    return result;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyInstance(VkInstance instance, const VkAllocationCallbacks *pAllocator) {
    // Grab the key before the instance is destroyed.
    dispatch_key key = get_dispatch_key(instance);
    bool skip = false;
    auto instance_data = GetLayerDataPtr(key, instance_layer_data_map);

    // Enable the temporary callback(s) here to catch vkDestroyInstance issues:
    bool callback_setup = false;
    if (instance_data->num_tmp_debug_messengers > 0) {
        if (!layer_enable_tmp_debug_messengers(instance_data->report_data, instance_data->num_tmp_debug_messengers,
                                               instance_data->tmp_messenger_create_infos, instance_data->tmp_debug_messengers)) {
            callback_setup = true;
        }
    }
    if (instance_data->num_tmp_report_callbacks > 0) {
        if (!layer_enable_tmp_report_callbacks(instance_data->report_data, instance_data->num_tmp_report_callbacks,
                                               instance_data->tmp_report_create_infos, instance_data->tmp_report_callbacks)) {
            callback_setup = true;
        }
    }

    skip |= parameter_validation_vkDestroyInstance(instance, pAllocator);

    // Disable and cleanup the temporary callback(s):
    if (callback_setup) {
        layer_disable_tmp_debug_messengers(instance_data->report_data, instance_data->num_tmp_debug_messengers,
                                           instance_data->tmp_debug_messengers);
        layer_disable_tmp_report_callbacks(instance_data->report_data, instance_data->num_tmp_report_callbacks,
                                           instance_data->tmp_report_callbacks);
    }
    if (instance_data->num_tmp_debug_messengers > 0) {
        layer_free_tmp_debug_messengers(instance_data->tmp_messenger_create_infos, instance_data->tmp_debug_messengers);
        instance_data->num_tmp_debug_messengers = 0;
    }
    if (instance_data->num_tmp_report_callbacks > 0) {
        layer_free_tmp_report_callbacks(instance_data->tmp_report_create_infos, instance_data->tmp_report_callbacks);
        instance_data->num_tmp_report_callbacks = 0;
    }

    if (!skip) {
        instance_data->dispatch_table.DestroyInstance(instance, pAllocator);

        // Clean up logging callback, if any
        while (instance_data->logging_messenger.size() > 0) {
            VkDebugUtilsMessengerEXT messenger = instance_data->logging_messenger.back();
            layer_destroy_messenger_callback(instance_data->report_data, messenger, pAllocator);
            instance_data->logging_messenger.pop_back();
        }
        while (instance_data->logging_callback.size() > 0) {
            VkDebugReportCallbackEXT callback = instance_data->logging_callback.back();
            layer_destroy_report_callback(instance_data->report_data, callback, pAllocator);
            instance_data->logging_callback.pop_back();
        }

        layer_debug_utils_destroy_instance(instance_data->report_data);
    }

    FreeLayerDataPtr(key, instance_layer_data_map);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugReportCallbackEXT(VkInstance instance,
                                                              const VkDebugReportCallbackCreateInfoEXT *pCreateInfo,
                                                              const VkAllocationCallbacks *pAllocator,
                                                              VkDebugReportCallbackEXT *pMsgCallback) {
    bool skip = parameter_validation_vkCreateDebugReportCallbackEXT(instance, pCreateInfo, pAllocator, pMsgCallback);
    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;

    auto instance_data = GetLayerDataPtr(get_dispatch_key(instance), instance_layer_data_map);
    VkResult result = instance_data->dispatch_table.CreateDebugReportCallbackEXT(instance, pCreateInfo, pAllocator, pMsgCallback);
    if (result == VK_SUCCESS) {
        result = layer_create_report_callback(instance_data->report_data, false, pCreateInfo, pAllocator, pMsgCallback);
        // If something happened during this call, clean up the message callback that was created earlier in the lower levels
        if (VK_SUCCESS != result) {
            instance_data->dispatch_table.DestroyDebugReportCallbackEXT(instance, *pMsgCallback, pAllocator);
        }
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT msgCallback,
                                                           const VkAllocationCallbacks *pAllocator) {
    bool skip = parameter_validation_vkDestroyDebugReportCallbackEXT(instance, msgCallback, pAllocator);
    if (!skip) {
        auto instance_data = GetLayerDataPtr(get_dispatch_key(instance), instance_layer_data_map);
        instance_data->dispatch_table.DestroyDebugReportCallbackEXT(instance, msgCallback, pAllocator);
        layer_destroy_report_callback(instance_data->report_data, msgCallback, pAllocator);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugUtilsMessengerEXT(VkInstance instance,
                                                              const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                                              const VkAllocationCallbacks *pAllocator,
                                                              VkDebugUtilsMessengerEXT *pMessenger) {
    bool skip = parameter_validation_vkCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);
    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;

    auto instance_data = GetLayerDataPtr(get_dispatch_key(instance), instance_layer_data_map);
    VkResult result = instance_data->dispatch_table.CreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);
    if (VK_SUCCESS == result) {
        result = layer_create_messenger_callback(instance_data->report_data, false, pCreateInfo, pAllocator, pMessenger);
        // If something happened during this call, clean up the message callback that was created earlier in the lower levels
        if (VK_SUCCESS != result) {
            instance_data->dispatch_table.DestroyDebugUtilsMessengerEXT(instance, *pMessenger, pAllocator);
        }
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger,
                                                           const VkAllocationCallbacks *pAllocator) {
    bool skip = parameter_validation_vkDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
    if (!skip) {
        auto instance_data = GetLayerDataPtr(get_dispatch_key(instance), instance_layer_data_map);
        instance_data->dispatch_table.DestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
        layer_destroy_messenger_callback(instance_data->report_data, messenger, pAllocator);
    }
}

template <typename ExtensionState>
static bool extension_state_by_name(const ExtensionState &extensions, const char *extension_name) {
    if (!extension_name) return false;  // null strings specify nothing
    auto info = ExtensionState::get_info(extension_name);
    bool state = info.state ? extensions.*(info.state) : false;  // unknown extensions can't be enabled in extension struct
    return state;
}

static bool ValidateDeviceCreateInfo(instance_layer_data *instance_data, VkPhysicalDevice physicalDevice,
                                     const VkDeviceCreateInfo *pCreateInfo, const DeviceExtensions &extensions) {
    bool skip = false;
    bool maint1 = false;
    bool negative_viewport = false;

    if ((pCreateInfo->enabledLayerCount > 0) && (pCreateInfo->ppEnabledLayerNames != NULL)) {
        for (size_t i = 0; i < pCreateInfo->enabledLayerCount; i++) {
            skip |= validate_string(instance_data->report_data, "vkCreateDevice", "pCreateInfo->ppEnabledLayerNames",
                                    "VUID-VkDeviceCreateInfo-ppEnabledLayerNames-parameter", pCreateInfo->ppEnabledLayerNames[i]);
        }
    }

    if ((pCreateInfo->enabledExtensionCount > 0) && (pCreateInfo->ppEnabledExtensionNames != NULL)) {
        maint1 = extension_state_by_name(extensions, VK_KHR_MAINTENANCE1_EXTENSION_NAME);
        negative_viewport = extension_state_by_name(extensions, VK_AMD_NEGATIVE_VIEWPORT_HEIGHT_EXTENSION_NAME);

        for (size_t i = 0; i < pCreateInfo->enabledExtensionCount; i++) {
            skip |= validate_string(instance_data->report_data, "vkCreateDevice", "pCreateInfo->ppEnabledExtensionNames",
                                    "VUID-VkDeviceCreateInfo-ppEnabledExtensionNames-parameter",
                                    pCreateInfo->ppEnabledExtensionNames[i]);
            skip |= validate_extension_reqs(instance_data, extensions, "VUID-vkCreateDevice-ppEnabledExtensionNames-01387",
                                            "device", pCreateInfo->ppEnabledExtensionNames[i]);
        }
    }

    if (maint1 && negative_viewport) {
        skip |= log_msg(instance_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        "VUID-VkDeviceCreateInfo-ppEnabledExtensionNames-00374",
                        "VkDeviceCreateInfo->ppEnabledExtensionNames must not simultaneously include VK_KHR_maintenance1 and "
                        "VK_AMD_negative_viewport_height.");
    }

    if (pCreateInfo->pNext != NULL && pCreateInfo->pEnabledFeatures) {
        // Check for get_physical_device_properties2 struct
        const auto *features2 = lvl_find_in_chain<VkPhysicalDeviceFeatures2KHR>(pCreateInfo->pNext);
        if (features2) {
            // Cannot include VkPhysicalDeviceFeatures2KHR and have non-null pEnabledFeatures
            skip |= log_msg(instance_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            kVUID_PVError_InvalidUsage,
                            "VkDeviceCreateInfo->pNext includes a VkPhysicalDeviceFeatures2KHR struct when "
                            "pCreateInfo->pEnabledFeatures is non-NULL.");
        }
    }

    // Validate pCreateInfo->pQueueCreateInfos
    if (pCreateInfo->pQueueCreateInfos) {
        std::unordered_set<uint32_t> set;

        for (uint32_t i = 0; i < pCreateInfo->queueCreateInfoCount; ++i) {
            const uint32_t requested_queue_family = pCreateInfo->pQueueCreateInfos[i].queueFamilyIndex;
            if (requested_queue_family == VK_QUEUE_FAMILY_IGNORED) {
                skip |= log_msg(instance_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT, HandleToUint64(physicalDevice),
                                "VUID-VkDeviceQueueCreateInfo-queueFamilyIndex-00381",
                                "vkCreateDevice: pCreateInfo->pQueueCreateInfos[%" PRIu32
                                "].queueFamilyIndex is VK_QUEUE_FAMILY_IGNORED, but it is required to provide a valid queue family "
                                "index value.",
                                i);
            } else if (set.count(requested_queue_family)) {
                skip |= log_msg(instance_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT, HandleToUint64(physicalDevice),
                                "VUID-VkDeviceCreateInfo-queueFamilyIndex-00372",
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
                        skip |= log_msg(instance_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                        VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT, HandleToUint64(physicalDevice),
                                        "VUID-VkDeviceQueueCreateInfo-pQueuePriorities-00383",
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

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo *pCreateInfo,
                                              const VkAllocationCallbacks *pAllocator, VkDevice *pDevice) {
    // NOTE: Don't validate physicalDevice or any dispatchable object as the first parameter. We couldn't get here if it was wrong!

    VkResult result = VK_ERROR_VALIDATION_FAILED_EXT;
    bool skip = false;
    auto my_instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    assert(my_instance_data != nullptr);

    // Query and save physical device limits for this device, needed for validation
    VkPhysicalDeviceProperties device_properties = {};
    my_instance_data->dispatch_table.GetPhysicalDeviceProperties(physicalDevice, &device_properties);

    // Set up the extension structure also for validation.
    DeviceExtensions extensions;
    uint32_t api_version =
        extensions.InitFromDeviceCreateInfo(&my_instance_data->extensions, device_properties.apiVersion, pCreateInfo);

    std::unique_lock<std::mutex> lock(global_lock);

    skip |= parameter_validation_vkCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);

    if (pCreateInfo != NULL) skip |= ValidateDeviceCreateInfo(my_instance_data, physicalDevice, pCreateInfo, extensions);

    if (!skip) {
        VkLayerDeviceCreateInfo *chain_info = get_chain_info(pCreateInfo, VK_LAYER_LINK_INFO);
        assert(chain_info != nullptr);
        assert(chain_info->u.pLayerInfo != nullptr);

        PFN_vkGetInstanceProcAddr fpGetInstanceProcAddr = chain_info->u.pLayerInfo->pfnNextGetInstanceProcAddr;
        PFN_vkGetDeviceProcAddr fpGetDeviceProcAddr = chain_info->u.pLayerInfo->pfnNextGetDeviceProcAddr;
        PFN_vkCreateDevice fpCreateDevice = (PFN_vkCreateDevice)fpGetInstanceProcAddr(my_instance_data->instance, "vkCreateDevice");
        if (fpCreateDevice == NULL) {
            return VK_ERROR_INITIALIZATION_FAILED;
        }

        // Advance the link info for the next element on the chain
        chain_info->u.pLayerInfo = chain_info->u.pLayerInfo->pNext;

        lock.unlock();

        result = fpCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);

        lock.lock();

        if (result == VK_SUCCESS) {
            layer_data *my_device_data = GetLayerDataPtr(get_dispatch_key(*pDevice), layer_data_map);
            assert(my_device_data != nullptr);

            my_device_data->report_data = layer_debug_utils_create_device(my_instance_data->report_data, *pDevice);
            layer_init_device_dispatch_table(*pDevice, &my_device_data->dispatch_table, fpGetDeviceProcAddr);

            my_device_data->api_version = api_version;
            my_device_data->extensions = extensions;

            // Store createdevice data
            if ((pCreateInfo != nullptr) && (pCreateInfo->pQueueCreateInfos != nullptr)) {
                for (uint32_t i = 0; i < pCreateInfo->queueCreateInfoCount; ++i) {
                    my_device_data->queueFamilyIndexMap.insert(std::make_pair(pCreateInfo->pQueueCreateInfos[i].queueFamilyIndex,
                                                                              pCreateInfo->pQueueCreateInfos[i].queueCount));
                }
            }

            memcpy(&my_device_data->device_limits, &device_properties.limits, sizeof(VkPhysicalDeviceLimits));
            my_device_data->physical_device = physicalDevice;
            my_device_data->device = *pDevice;

            // Save app-enabled features in this device's layer_data structure
            // The enabled features can come from either pEnabledFeatures, or from the pNext chain
            const VkPhysicalDeviceFeatures *enabled_features_found = pCreateInfo->pEnabledFeatures;
            if ((nullptr == enabled_features_found) && my_device_data->extensions.vk_khr_get_physical_device_properties_2) {
                const auto *features2 = lvl_find_in_chain<VkPhysicalDeviceFeatures2KHR>(pCreateInfo->pNext);
                if (features2) {
                    enabled_features_found = &(features2->features);
                }
            }
            if (enabled_features_found) {
                my_device_data->physical_device_features = *enabled_features_found;
            } else {
                memset(&my_device_data->physical_device_features, 0, sizeof(VkPhysicalDeviceFeatures));
            }
        }
    }

    return result;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDevice(VkDevice device, const VkAllocationCallbacks *pAllocator) {
    dispatch_key key = get_dispatch_key(device);
    bool skip = false;
    layer_data *device_data = GetLayerDataPtr(key, layer_data_map);
    {
        std::unique_lock<std::mutex> lock(global_lock);
        skip |= parameter_validation_vkDestroyDevice(device, pAllocator);
    }

    if (!skip) {
        layer_debug_utils_destroy_device(device);
        device_data->dispatch_table.DestroyDevice(device, pAllocator);
    }
    FreeLayerDataPtr(key, layer_data_map);
}

bool pv_vkGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue *pQueue) {
    bool skip = false;
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);

    skip |= ValidateDeviceQueueFamily(device_data, queueFamilyIndex, "vkGetDeviceQueue", "queueFamilyIndex",
                                      "VUID-vkGetDeviceQueue-queueFamilyIndex-00384");
    const auto &queue_data = device_data->queueFamilyIndexMap.find(queueFamilyIndex);
    if (queue_data != device_data->queueFamilyIndexMap.end() && queue_data->second <= queueIndex) {
        skip |= log_msg(device_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT,
                        HandleToUint64(device), "VUID-vkGetDeviceQueue-queueIndex-00385",
                        "vkGetDeviceQueue: queueIndex (=%" PRIu32
                        ") is not less than the number of queues requested from queueFamilyIndex (=%" PRIu32
                        ") when the device was created (i.e. is not less than %" PRIu32 ").",
                        queueIndex, queueFamilyIndex, queue_data->second);
    }
    return skip;
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo *pCreateInfo,
                                                   const VkAllocationCallbacks *pAllocator, VkCommandPool *pCommandPool) {
    layer_data *local_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    VkResult result = VK_ERROR_VALIDATION_FAILED_EXT;
    std::unique_lock<std::mutex> lock(global_lock);

    skip |= ValidateDeviceQueueFamily(local_data, pCreateInfo->queueFamilyIndex, "vkCreateCommandPool",
                                      "pCreateInfo->queueFamilyIndex", "VUID-vkCreateCommandPool-queueFamilyIndex-01937");

    skip |= parameter_validation_vkCreateCommandPool(device, pCreateInfo, pAllocator, pCommandPool);

    lock.unlock();
    if (!skip) {
        result = local_data->dispatch_table.CreateCommandPool(device, pCreateInfo, pAllocator, pCommandPool);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo *pCreateInfo,
                                                 const VkAllocationCallbacks *pAllocator, VkQueryPool *pQueryPool) {
    VkResult result = VK_ERROR_VALIDATION_FAILED_EXT;
    bool skip = false;
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);

    skip |= parameter_validation_vkCreateQueryPool(device, pCreateInfo, pAllocator, pQueryPool);

    // Validation for parameters excluded from the generated validation code due to a 'noautovalidity' tag in vk.xml
    if (pCreateInfo != nullptr) {
        // If queryType is VK_QUERY_TYPE_PIPELINE_STATISTICS, pipelineStatistics must be a valid combination of
        // VkQueryPipelineStatisticFlagBits values
        if ((pCreateInfo->queryType == VK_QUERY_TYPE_PIPELINE_STATISTICS) && (pCreateInfo->pipelineStatistics != 0) &&
            ((pCreateInfo->pipelineStatistics & (~AllVkQueryPipelineStatisticFlagBits)) != 0)) {
            skip |= log_msg(device_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            "VUID-VkQueryPoolCreateInfo-queryType-00792",
                            "vkCreateQueryPool(): if pCreateInfo->queryType is VK_QUERY_TYPE_PIPELINE_STATISTICS, "
                            "pCreateInfo->pipelineStatistics must be a valid combination of VkQueryPipelineStatisticFlagBits "
                            "values.");
        }
    }
    if (!skip) {
        result = device_data->dispatch_table.CreateQueryPool(device, pCreateInfo, pAllocator, pQueryPool);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo *pCreateInfo,
                                                  const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    VkResult result = VK_ERROR_VALIDATION_FAILED_EXT;

    {
        std::unique_lock<std::mutex> lock(global_lock);
        skip |= parameter_validation_vkCreateRenderPass(device, pCreateInfo, pAllocator, pRenderPass);

        typedef bool (*PFN_manual_vkCreateRenderPass)(VkDevice device, const VkRenderPassCreateInfo *pCreateInfo,
                                                      const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass);
        PFN_manual_vkCreateRenderPass custom_func = (PFN_manual_vkCreateRenderPass)custom_functions["vkCreateRenderPass"];
        if (custom_func != nullptr) {
            skip |= custom_func(device, pCreateInfo, pAllocator, pRenderPass);
        }
    }

    if (!skip) {
        result = device_data->dispatch_table.CreateRenderPass(device, pCreateInfo, pAllocator, pRenderPass);

        // track the state necessary for checking vkCreateGraphicsPipeline (subpass usage of depth and color attachments)
        if (result == VK_SUCCESS) {
            std::unique_lock<std::mutex> lock(global_lock);
            const auto renderPass = *pRenderPass;
            auto &renderpass_state = device_data->renderpasses_states[renderPass];

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
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks *pAllocator) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;

    {
        std::unique_lock<std::mutex> lock(global_lock);
        skip |= parameter_validation_vkDestroyRenderPass(device, renderPass, pAllocator);

        typedef bool (*PFN_manual_vkDestroyRenderPass)(VkDevice device, VkRenderPass renderPass,
                                                       const VkAllocationCallbacks *pAllocator);
        PFN_manual_vkDestroyRenderPass custom_func = (PFN_manual_vkDestroyRenderPass)custom_functions["vkDestroyRenderPass"];
        if (custom_func != nullptr) {
            skip |= custom_func(device, renderPass, pAllocator);
        }
    }

    if (!skip) {
        device_data->dispatch_table.DestroyRenderPass(device, renderPass, pAllocator);

        // track the state necessary for checking vkCreateGraphicsPipeline (subpass usage of depth and color attachments)
        {
            std::unique_lock<std::mutex> lock(global_lock);
            device_data->renderpasses_states.erase(renderPass);
        }
    }
}

bool pv_vkCreateBuffer(VkDevice device, const VkBufferCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator,
                       VkBuffer *pBuffer) {
    bool skip = false;
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    debug_report_data *report_data = device_data->report_data;

    const LogMiscParams log_misc{report_data, VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, VK_NULL_HANDLE, "vkCreateBuffer"};

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
            } else {
                skip |= ValidateQueueFamilies(device_data, pCreateInfo->queueFamilyIndexCount, pCreateInfo->pQueueFamilyIndices,
                                              "vkCreateBuffer", "pCreateInfo->pQueueFamilyIndices", kVUID_PVError_InvalidUsage,
                                              kVUID_PVError_InvalidUsage, false);
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

bool pv_vkCreateImage(VkDevice device, const VkImageCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator,
                      VkImage *pImage) {
    bool skip = false;
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    debug_report_data *report_data = device_data->report_data;

    const LogMiscParams log_misc{report_data, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, VK_NULL_HANDLE, "vkCreateImage"};

    if (pCreateInfo != nullptr) {
        if ((device_data->physical_device_features.textureCompressionETC2 == false) &&
            FormatIsCompressed_ETC2_EAC(pCreateInfo->format)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            kVUID_PVError_DeviceFeature,
                            "vkCreateImage(): Attempting to create VkImage with format %s. The textureCompressionETC2 feature is "
                            "not enabled: neither ETC2 nor EAC formats can be used to create images.",
                            string_VkFormat(pCreateInfo->format));
        }

        if ((device_data->physical_device_features.textureCompressionASTC_LDR == false) &&
            FormatIsCompressed_ASTC_LDR(pCreateInfo->format)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            kVUID_PVError_DeviceFeature,
                            "vkCreateImage(): Attempting to create VkImage with format %s. The textureCompressionASTC_LDR feature "
                            "is not enabled: ASTC formats cannot be used to create images.",
                            string_VkFormat(pCreateInfo->format));
        }

        if ((device_data->physical_device_features.textureCompressionBC == false) && FormatIsCompressed_BC(pCreateInfo->format)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            kVUID_PVError_DeviceFeature,
                            "vkCreateImage(): Attempting to create VkImage with format %s. The textureCompressionBC feature is not "
                            "enabled: BC compressed formats cannot be used to create images.",
                            string_VkFormat(pCreateInfo->format));
        }

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
            } else {
                skip |= ValidateQueueFamilies(device_data, pCreateInfo->queueFamilyIndexCount, pCreateInfo->pQueueFamilyIndices,
                                              "vkCreateImage", "pCreateInfo->pQueueFamilyIndices", kVUID_PVError_InvalidUsage,
                                              kVUID_PVError_InvalidUsage, false);
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
             (pCreateInfo->tiling != VK_IMAGE_TILING_OPTIMAL) || (pCreateInfo->mipLevels != 1))) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            "VUID-VkImageCreateInfo-samples-00962",
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

        // mipLevels must be less than or equal to floor(log2(max(extent.width,extent.height,extent.depth)))+1
        uint32_t maxDim = std::max(std::max(pCreateInfo->extent.width, pCreateInfo->extent.height), pCreateInfo->extent.depth);
        if (maxDim > 0 && pCreateInfo->mipLevels > (floor(log2(maxDim)) + 1)) {
            skip |=
                log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        "VUID-VkImageCreateInfo-mipLevels-00958",
                        "vkCreateImage(): pCreateInfo->mipLevels must be less than or equal to "
                        "floor(log2(max(pCreateInfo->extent.width, pCreateInfo->extent.height, pCreateInfo->extent.depth)))+1.");
        }

        if ((pCreateInfo->flags & VK_IMAGE_CREATE_SPARSE_BINDING_BIT) && (!device_data->physical_device_features.sparseBinding)) {
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
            if ((VK_FALSE == device_data->physical_device_features.sparseResidencyImage2D) &&
                (VK_IMAGE_TYPE_2D == pCreateInfo->imageType)) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                "VUID-VkImageCreateInfo-imageType-00971",
                                "vkCreateImage: cannot specify VK_IMAGE_CREATE_SPARSE_BINDING_BIT for 2D image if corresponding "
                                "feature is not enabled on the device.");
            }

            // Sparse 3D image when device doesn't support it
            if ((VK_FALSE == device_data->physical_device_features.sparseResidencyImage3D) &&
                (VK_IMAGE_TYPE_3D == pCreateInfo->imageType)) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                "VUID-VkImageCreateInfo-imageType-00972",
                                "vkCreateImage: cannot specify VK_IMAGE_CREATE_SPARSE_BINDING_BIT for 3D image if corresponding "
                                "feature is not enabled on the device.");
            }

            // Multi-sample 2D image when device doesn't support it
            if (VK_IMAGE_TYPE_2D == pCreateInfo->imageType) {
                if ((VK_FALSE == device_data->physical_device_features.sparseResidency2Samples) &&
                    (VK_SAMPLE_COUNT_2_BIT == pCreateInfo->samples)) {
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                    "VUID-VkImageCreateInfo-imageType-00973",
                                    "vkCreateImage: cannot specify VK_IMAGE_CREATE_SPARSE_BINDING_BIT for 2-sample image if "
                                    "corresponding feature is not enabled on the device.");
                } else if ((VK_FALSE == device_data->physical_device_features.sparseResidency4Samples) &&
                           (VK_SAMPLE_COUNT_4_BIT == pCreateInfo->samples)) {
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                    "VUID-VkImageCreateInfo-imageType-00974",
                                    "vkCreateImage: cannot specify VK_IMAGE_CREATE_SPARSE_BINDING_BIT for 4-sample image if "
                                    "corresponding feature is not enabled on the device.");
                } else if ((VK_FALSE == device_data->physical_device_features.sparseResidency8Samples) &&
                           (VK_SAMPLE_COUNT_8_BIT == pCreateInfo->samples)) {
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                    "VUID-VkImageCreateInfo-imageType-00975",
                                    "vkCreateImage: cannot specify VK_IMAGE_CREATE_SPARSE_BINDING_BIT for 8-sample image if "
                                    "corresponding feature is not enabled on the device.");
                } else if ((VK_FALSE == device_data->physical_device_features.sparseResidency16Samples) &&
                           (VK_SAMPLE_COUNT_16_BIT == pCreateInfo->samples)) {
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                    "VUID-VkImageCreateInfo-imageType-00976",
                                    "vkCreateImage: cannot specify VK_IMAGE_CREATE_SPARSE_BINDING_BIT for 16-sample image if "
                                    "corresponding feature is not enabled on the device.");
                }
            }
        }
    }
    return skip;
}

bool pv_vkCreateImageView(VkDevice device, const VkImageViewCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator,
                          VkImageView *pView) {
    bool skip = false;
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    debug_report_data *report_data = device_data->report_data;

    if (pCreateInfo != nullptr) {
        // Validate chained VkImageViewUsageCreateInfo struct, if present
        if (nullptr != pCreateInfo->pNext) {
            auto chained_ivuci_struct = lvl_find_in_chain<VkImageViewUsageCreateInfoKHR>(pCreateInfo->pNext);
            if (chained_ivuci_struct) {
                if (0 == chained_ivuci_struct->usage) {
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                    "VUID-VkImageViewUsageCreateInfo-usage-requiredbitmask",
                                    "vkCreateImageView: Chained VkImageViewUsageCreateInfo usage field must not be 0.");
                } else if (chained_ivuci_struct->usage & ~AllVkImageUsageFlagBits) {
                    std::stringstream ss;
                    ss << "vkCreateImageView: Chained VkImageViewUsageCreateInfo usage field (0x" << std::hex
                       << chained_ivuci_struct->usage << ") contains invalid flag bits.";
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                    "VUID-VkImageViewUsageCreateInfo-usage-parameter", "%s", ss.str().c_str());
                }
            }
        }
    }
    return skip;
}

bool pv_VkViewport(const layer_data *device_data, const VkViewport &viewport, const char *fn_name, const char *param_name,
                   VkDebugReportObjectTypeEXT object_type, uint64_t object = 0) {
    bool skip = false;
    debug_report_data *report_data = device_data->report_data;

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
    const auto max_w = device_data->device_limits.maxViewportDimensions[0];

    if (!(viewport.width > 0.0f)) {
        width_healthy = false;
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, object_type, object, "VUID-VkViewport-width-01770",
                        "%s: %s.width (=%f) is not greater than 0.0.", fn_name, param_name, viewport.width);
    } else if (!(f_lte_u32_exact(viewport.width, max_w) || f_lte_u32_direct(viewport.width, max_w))) {
        width_healthy = false;
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, object_type, object, "VUID-VkViewport-width-01771",
                        "%s: %s.width (=%f) exceeds VkPhysicalDeviceLimits::maxViewportDimensions[0] (=%" PRIu32 ").", fn_name,
                        param_name, viewport.width, max_w);
    } else if (!f_lte_u32_exact(viewport.width, max_w) && f_lte_u32_direct(viewport.width, max_w)) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, object_type, object, kVUID_PVError_NONE,
                        "%s: %s.width (=%f) technically exceeds VkPhysicalDeviceLimits::maxViewportDimensions[0] (=%" PRIu32
                        "), but it is within the static_cast<float>(maxViewportDimensions[0]) limit.",
                        fn_name, param_name, viewport.width, max_w);
    }

    // height
    bool height_healthy = true;
    const bool negative_height_enabled = device_data->api_version >= VK_API_VERSION_1_1 ||
                                         device_data->extensions.vk_khr_maintenance1 ||
                                         device_data->extensions.vk_amd_negative_viewport_height;
    const auto max_h = device_data->device_limits.maxViewportDimensions[1];

    if (!negative_height_enabled && !(viewport.height > 0.0f)) {
        height_healthy = false;
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, object_type, object, "VUID-VkViewport-height-01772",
                        "%s: %s.height (=%f) is not greater 0.0.", fn_name, param_name, viewport.height);
    } else if (!(f_lte_u32_exact(fabsf(viewport.height), max_h) || f_lte_u32_direct(fabsf(viewport.height), max_h))) {
        height_healthy = false;

        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, object_type, object, "VUID-VkViewport-height-01773",
                        "%s: Absolute value of %s.height (=%f) exceeds VkPhysicalDeviceLimits::maxViewportDimensions[1] (=%" PRIu32
                        ").",
                        fn_name, param_name, viewport.height, max_h);
    } else if (!f_lte_u32_exact(fabsf(viewport.height), max_h) && f_lte_u32_direct(fabsf(viewport.height), max_h)) {
        height_healthy = false;

        skip |= log_msg(
            report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, object_type, object, kVUID_PVError_NONE,
            "%s: Absolute value of %s.height (=%f) technically exceeds VkPhysicalDeviceLimits::maxViewportDimensions[1] (=%" PRIu32
            "), but it is within the static_cast<float>(maxViewportDimensions[1]) limit.",
            fn_name, param_name, viewport.height, max_h);
    }

    // x
    bool x_healthy = true;
    if (!(viewport.x >= device_data->device_limits.viewportBoundsRange[0])) {
        x_healthy = false;
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, object_type, object, "VUID-VkViewport-x-01774",
                        "%s: %s.x (=%f) is less than VkPhysicalDeviceLimits::viewportBoundsRange[0] (=%f).", fn_name, param_name,
                        viewport.x, device_data->device_limits.viewportBoundsRange[0]);
    }

    // x + width
    if (x_healthy && width_healthy) {
        const float right_bound = viewport.x + viewport.width;
        if (!(right_bound <= device_data->device_limits.viewportBoundsRange[1])) {
            skip |=
                log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, object_type, object, "VUID-VkViewport-x-01232",
                        "%s: %s.x + %s.width (=%f + %f = %f) is greater than VkPhysicalDeviceLimits::viewportBoundsRange[1] (=%f).",
                        fn_name, param_name, param_name, viewport.x, viewport.width, right_bound,
                        device_data->device_limits.viewportBoundsRange[1]);
        }
    }

    // y
    bool y_healthy = true;
    if (!(viewport.y >= device_data->device_limits.viewportBoundsRange[0])) {
        y_healthy = false;
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, object_type, object, "VUID-VkViewport-y-01775",
                        "%s: %s.y (=%f) is less than VkPhysicalDeviceLimits::viewportBoundsRange[0] (=%f).", fn_name, param_name,
                        viewport.y, device_data->device_limits.viewportBoundsRange[0]);
    } else if (negative_height_enabled && !(viewport.y <= device_data->device_limits.viewportBoundsRange[1])) {
        y_healthy = false;
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, object_type, object, "VUID-VkViewport-y-01776",
                        "%s: %s.y (=%f) exceeds VkPhysicalDeviceLimits::viewportBoundsRange[1] (=%f).", fn_name, param_name,
                        viewport.y, device_data->device_limits.viewportBoundsRange[1]);
    }

    // y + height
    if (y_healthy && height_healthy) {
        const float boundary = viewport.y + viewport.height;

        if (!(boundary <= device_data->device_limits.viewportBoundsRange[1])) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, object_type, object, "VUID-VkViewport-y-01233",
                            "%s: %s.y + %s.height (=%f + %f = %f) exceeds VkPhysicalDeviceLimits::viewportBoundsRange[1] (=%f).",
                            fn_name, param_name, param_name, viewport.y, viewport.height, boundary,
                            device_data->device_limits.viewportBoundsRange[1]);
        } else if (negative_height_enabled && !(boundary >= device_data->device_limits.viewportBoundsRange[0])) {
            skip |= log_msg(
                report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, object_type, object, "VUID-VkViewport-y-01777",
                "%s: %s.y + %s.height (=%f + %f = %f) is less than VkPhysicalDeviceLimits::viewportBoundsRange[0] (=%f).", fn_name,
                param_name, param_name, viewport.y, viewport.height, boundary, device_data->device_limits.viewportBoundsRange[0]);
        }
    }

    if (!device_data->extensions.vk_ext_depth_range_unrestricted) {
        // minDepth
        if (!(viewport.minDepth >= 0.0) || !(viewport.minDepth <= 1.0)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, object_type, object, "VUID-VkViewport-minDepth-01234",

                            "%s: VK_EXT_depth_range_unrestricted extension is not enabled and %s.minDepth (=%f) is not within the "
                            "[0.0, 1.0] range.",
                            fn_name, param_name, viewport.minDepth);
        }

        // maxDepth
        if (!(viewport.maxDepth >= 0.0) || !(viewport.maxDepth <= 1.0)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, object_type, object, "VUID-VkViewport-maxDepth-01235",

                            "%s: VK_EXT_depth_range_unrestricted extension is not enabled and %s.maxDepth (=%f) is not within the "
                            "[0.0, 1.0] range.",
                            fn_name, param_name, viewport.maxDepth);
        }
    }

    return skip;
}

bool pv_vkCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                  const VkGraphicsPipelineCreateInfo *pCreateInfos, const VkAllocationCallbacks *pAllocator,
                                  VkPipeline *pPipelines) {
    bool skip = false;
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    debug_report_data *report_data = device_data->report_data;

    if (pCreateInfos != nullptr) {
        for (uint32_t i = 0; i < createInfoCount; ++i) {
            bool has_dynamic_viewport = false;
            bool has_dynamic_scissor = false;
            bool has_dynamic_line_width = false;
            bool has_dynamic_viewport_w_scaling_nv = false;
            bool has_dynamic_discard_rectangle_ext = false;
            bool has_dynamic_sample_locations_ext = false;
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
                }
            }

            // Validation for parameters excluded from the generated validation code due to a 'noautovalidity' tag in vk.xml
            if (pCreateInfos[i].pVertexInputState != nullptr) {
                auto const &vertex_input_state = pCreateInfos[i].pVertexInputState;

                if (vertex_input_state->vertexBindingDescriptionCount > device_data->device_limits.maxVertexInputBindings) {
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                    "VUID-VkPipelineVertexInputStateCreateInfo-vertexBindingDescriptionCount-00613",
                                    "vkCreateGraphicsPipelines: pararameter "
                                    "pCreateInfo[%d].pVertexInputState->vertexBindingDescriptionCount (%u) is "
                                    "greater than VkPhysicalDeviceLimits::maxVertexInputBindings (%u).",
                                    i, vertex_input_state->vertexBindingDescriptionCount,
                                    device_data->device_limits.maxVertexInputBindings);
                }

                if (vertex_input_state->vertexAttributeDescriptionCount > device_data->device_limits.maxVertexInputAttributes) {
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                    "VUID-VkPipelineVertexInputStateCreateInfo-vertexAttributeDescriptionCount-00614",
                                    "vkCreateGraphicsPipelines: pararameter "
                                    "pCreateInfo[%d].pVertexInputState->vertexAttributeDescriptionCount (%u) is "
                                    "greater than VkPhysicalDeviceLimits::maxVertexInputAttributes (%u).",
                                    i, vertex_input_state->vertexBindingDescriptionCount,
                                    device_data->device_limits.maxVertexInputAttributes);
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

                    if (vertex_bind_desc.binding >= device_data->device_limits.maxVertexInputBindings) {
                        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                        "VUID-VkVertexInputBindingDescription-binding-00618",
                                        "vkCreateGraphicsPipelines: parameter "
                                        "pCreateInfos[%u].pVertexInputState->pVertexBindingDescriptions[%u].binding (%u) is "
                                        "greater than or equal to VkPhysicalDeviceLimits::maxVertexInputBindings (%u).",
                                        i, d, vertex_bind_desc.binding, device_data->device_limits.maxVertexInputBindings);
                    }

                    if (vertex_bind_desc.stride > device_data->device_limits.maxVertexInputBindingStride) {
                        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                        "VUID-VkVertexInputBindingDescription-stride-00619",
                                        "vkCreateGraphicsPipelines: parameter "
                                        "pCreateInfos[%u].pVertexInputState->pVertexBindingDescriptions[%u].stride (%u) is greater "
                                        "than VkPhysicalDeviceLimits::maxVertexInputBindingStride (%u).",
                                        i, d, vertex_bind_desc.stride, device_data->device_limits.maxVertexInputBindingStride);
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

                    if (vertex_attrib_desc.location >= device_data->device_limits.maxVertexInputAttributes) {
                        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                        "VUID-VkVertexInputAttributeDescription-location-00620",
                                        "vkCreateGraphicsPipelines: parameter "
                                        "pCreateInfos[%u].pVertexInputState->pVertexAttributeDescriptions[%u].location (%u) is "
                                        "greater than or equal to VkPhysicalDeviceLimits::maxVertexInputAttributes (%u).",
                                        i, d, vertex_attrib_desc.location, device_data->device_limits.maxVertexInputAttributes);
                    }

                    if (vertex_attrib_desc.binding >= device_data->device_limits.maxVertexInputBindings) {
                        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                        "VUID-VkVertexInputAttributeDescription-binding-00621",
                                        "vkCreateGraphicsPipelines: parameter "
                                        "pCreateInfos[%u].pVertexInputState->pVertexAttributeDescriptions[%u].binding (%u) is "
                                        "greater than or equal to VkPhysicalDeviceLimits::maxVertexInputBindings (%u).",
                                        i, d, vertex_attrib_desc.binding, device_data->device_limits.maxVertexInputBindings);
                    }

                    if (vertex_attrib_desc.offset > device_data->device_limits.maxVertexInputAttributeOffset) {
                        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                        "VUID-VkVertexInputAttributeDescription-offset-00622",
                                        "vkCreateGraphicsPipelines: parameter "
                                        "pCreateInfos[%u].pVertexInputState->pVertexAttributeDescriptions[%u].offset (%u) is "
                                        "greater than VkPhysicalDeviceLimits::maxVertexInputAttributeOffset (%u).",
                                        i, d, vertex_attrib_desc.offset, device_data->device_limits.maxVertexInputAttributeOffset);
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
                        skip |= validate_struct_pnext(
                            report_data, "vkCreateGraphicsPipelines",
                            ParameterName("pCreateInfos[%i].pTessellationState->pNext", ParameterName::IndexVector{i}), NULL,
                            pCreateInfos[i].pTessellationState->pNext, 0, NULL, GeneratedHeaderVersion,
                            "VUID-VkGraphicsPipelineCreateInfo-pNext-pNext");

                        skip |= validate_reserved_flags(
                            report_data, "vkCreateGraphicsPipelines",
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
                            pCreateInfos[i].pTessellationState->patchControlPoints >
                                device_data->device_limits.maxTessellationPatchSize) {
                            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                            "VUID-VkPipelineTessellationStateCreateInfo-patchControlPoints-01214",
                                            "vkCreateGraphicsPipelines: invalid parameter "
                                            "pCreateInfos[%d].pTessellationState->patchControlPoints value %u. patchControlPoints "
                                            "should be >0 and <=%u.",
                                            i, pCreateInfos[i].pTessellationState->patchControlPoints,
                                            device_data->device_limits.maxTessellationPatchSize);
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
                        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_W_SCALING_STATE_CREATE_INFO_NV};
                    skip |= validate_struct_pnext(
                        report_data, "vkCreateGraphicsPipelines",
                        ParameterName("pCreateInfos[%i].pViewportState->pNext", ParameterName::IndexVector{i}),
                        "VkPipelineViewportSwizzleStateCreateInfoNV, VkPipelineViewportWScalingStateCreateInfoNV",
                        viewport_state.pNext, ARRAY_SIZE(allowed_structs_VkPipelineViewportStateCreateInfo),
                        allowed_structs_VkPipelineViewportStateCreateInfo, 65,
                        "VUID-VkPipelineViewportStateCreateInfo-pNext-pNext");

                    skip |= validate_reserved_flags(
                        report_data, "vkCreateGraphicsPipelines",
                        ParameterName("pCreateInfos[%i].pViewportState->flags", ParameterName::IndexVector{i}),
                        viewport_state.flags, "VUID-VkPipelineViewportStateCreateInfo-flags-zerobitmask");

                    if (!device_data->physical_device_features.multiViewport) {
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
                    } else {  // multiViewport enabled
                        if (viewport_state.viewportCount == 0) {
                            skip |= log_msg(
                                report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT,
                                VK_NULL_HANDLE, "VUID-VkPipelineViewportStateCreateInfo-viewportCount-arraylength",
                                "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32 "].pViewportState->viewportCount is 0.", i);
                        } else if (viewport_state.viewportCount > device_data->device_limits.maxViewports) {
                            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT,
                                            VK_NULL_HANDLE, "VUID-VkPipelineViewportStateCreateInfo-viewportCount-01218",
                                            "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32
                                            "].pViewportState->viewportCount (=%" PRIu32
                                            ") is greater than VkPhysicalDeviceLimits::maxViewports (=%" PRIu32 ").",
                                            i, viewport_state.viewportCount, device_data->device_limits.maxViewports);
                        }

                        if (viewport_state.scissorCount == 0) {
                            skip |= log_msg(
                                report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT,
                                VK_NULL_HANDLE, "VUID-VkPipelineViewportStateCreateInfo-scissorCount-arraylength",
                                "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32 "].pViewportState->scissorCount is 0.", i);
                        } else if (viewport_state.scissorCount > device_data->device_limits.maxViewports) {
                            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT,
                                            VK_NULL_HANDLE, "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01219",
                                            "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32
                                            "].pViewportState->scissorCount (=%" PRIu32
                                            ") is greater than VkPhysicalDeviceLimits::maxViewports (=%" PRIu32 ").",
                                            i, viewport_state.scissorCount, device_data->device_limits.maxViewports);
                        }
                    }

                    if (viewport_state.scissorCount != viewport_state.viewportCount) {
                        skip |=
                            log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT,
                                    VK_NULL_HANDLE, "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01220",
                                    "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32 "].pViewportState->scissorCount (=%" PRIu32
                                    ") is not identical to pCreateInfos[%" PRIu32 "].pViewportState->viewportCount (=%" PRIu32 ").",
                                    i, viewport_state.scissorCount, i, viewport_state.viewportCount);
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

                    // validate the VkViewports
                    if (!has_dynamic_viewport && viewport_state.pViewports) {
                        for (uint32_t viewport_i = 0; viewport_i < viewport_state.viewportCount; ++viewport_i) {
                            const auto &viewport = viewport_state.pViewports[viewport_i];  // will crash on invalid ptr
                            const char fn_name[] = "vkCreateGraphicsPipelines";
                            const std::string param_name = "pCreateInfos[" + std::to_string(i) + "].pViewportState->pViewports[" +
                                                           std::to_string(viewport_i) + "]";
                            skip |= pv_VkViewport(device_data, viewport, fn_name, param_name.c_str(),
                                                  VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT);
                        }
                    }

                    if (has_dynamic_viewport_w_scaling_nv && !device_data->extensions.vk_nv_clip_space_w_scaling) {
                        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT,
                                        VK_NULL_HANDLE, kVUID_PVError_ExtensionNotEnabled,
                                        "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32
                                        "].pDynamicState->pDynamicStates contains VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_NV, but "
                                        "VK_NV_clip_space_w_scaling extension is not enabled.",
                                        i);
                    }

                    if (has_dynamic_discard_rectangle_ext && !device_data->extensions.vk_ext_discard_rectangles) {
                        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT,
                                        VK_NULL_HANDLE, kVUID_PVError_ExtensionNotEnabled,
                                        "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32
                                        "].pDynamicState->pDynamicStates contains VK_DYNAMIC_STATE_DISCARD_RECTANGLE_EXT, but "
                                        "VK_EXT_discard_rectangles extension is not enabled.",
                                        i);
                    }

                    if (has_dynamic_sample_locations_ext && !device_data->extensions.vk_ext_sample_locations) {
                        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT,
                                        VK_NULL_HANDLE, kVUID_PVError_ExtensionNotEnabled,
                                        "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32
                                        "].pDynamicState->pDynamicStates contains VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT, but "
                                        "VK_EXT_sample_locations extension is not enabled.",
                                        i);
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
                        report_data, "vkCreateGraphicsPipelines",
                        ParameterName("pCreateInfos[%i].pMultisampleState->pNext", ParameterName::IndexVector{i}),
                        valid_struct_names, pCreateInfos[i].pMultisampleState->pNext, 3, valid_next_stypes, GeneratedHeaderVersion,
                        "VUID-VkPipelineMultisampleStateCreateInfo-pNext-pNext");

                    skip |= validate_reserved_flags(
                        report_data, "vkCreateGraphicsPipelines",
                        ParameterName("pCreateInfos[%i].pMultisampleState->flags", ParameterName::IndexVector{i}),
                        pCreateInfos[i].pMultisampleState->flags, "VUID-VkPipelineMultisampleStateCreateInfo-flags-zerobitmask");

                    skip |= validate_bool32(
                        report_data, "vkCreateGraphicsPipelines",
                        ParameterName("pCreateInfos[%i].pMultisampleState->sampleShadingEnable", ParameterName::IndexVector{i}),
                        pCreateInfos[i].pMultisampleState->sampleShadingEnable);

                    skip |= validate_array(
                        report_data, "vkCreateGraphicsPipelines",
                        ParameterName("pCreateInfos[%i].pMultisampleState->rasterizationSamples", ParameterName::IndexVector{i}),
                        ParameterName("pCreateInfos[%i].pMultisampleState->pSampleMask", ParameterName::IndexVector{i}),
                        pCreateInfos[i].pMultisampleState->rasterizationSamples, &pCreateInfos[i].pMultisampleState->pSampleMask,
                        true, false, kVUIDUndefined, kVUIDUndefined);

                    skip |= validate_bool32(
                        report_data, "vkCreateGraphicsPipelines",
                        ParameterName("pCreateInfos[%i].pMultisampleState->alphaToCoverageEnable", ParameterName::IndexVector{i}),
                        pCreateInfos[i].pMultisampleState->alphaToCoverageEnable);

                    skip |= validate_bool32(
                        report_data, "vkCreateGraphicsPipelines",
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
                        if (!device_data->physical_device_features.sampleRateShading) {
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
                }

                bool uses_color_attachment = false;
                bool uses_depthstencil_attachment = false;
                {
                    const auto subpasses_uses_it = device_data->renderpasses_states.find(pCreateInfos[i].renderPass);
                    if (subpasses_uses_it != device_data->renderpasses_states.end()) {
                        const auto &subpasses_uses = subpasses_uses_it->second;
                        if (subpasses_uses.subpasses_using_color_attachment.count(pCreateInfos[i].subpass))
                            uses_color_attachment = true;
                        if (subpasses_uses.subpasses_using_depthstencil_attachment.count(pCreateInfos[i].subpass))
                            uses_depthstencil_attachment = true;
                    }
                }

                if (pCreateInfos[i].pDepthStencilState != nullptr && uses_depthstencil_attachment) {
                    skip |= validate_struct_pnext(
                        report_data, "vkCreateGraphicsPipelines",
                        ParameterName("pCreateInfos[%i].pDepthStencilState->pNext", ParameterName::IndexVector{i}), NULL,
                        pCreateInfos[i].pDepthStencilState->pNext, 0, NULL, GeneratedHeaderVersion,
                        "VUID-VkPipelineDepthStencilStateCreateInfo-pNext-pNext");

                    skip |= validate_reserved_flags(
                        report_data, "vkCreateGraphicsPipelines",
                        ParameterName("pCreateInfos[%i].pDepthStencilState->flags", ParameterName::IndexVector{i}),
                        pCreateInfos[i].pDepthStencilState->flags, "VUID-VkPipelineDepthStencilStateCreateInfo-flags-zerobitmask");

                    skip |= validate_bool32(
                        report_data, "vkCreateGraphicsPipelines",
                        ParameterName("pCreateInfos[%i].pDepthStencilState->depthTestEnable", ParameterName::IndexVector{i}),
                        pCreateInfos[i].pDepthStencilState->depthTestEnable);

                    skip |= validate_bool32(
                        report_data, "vkCreateGraphicsPipelines",
                        ParameterName("pCreateInfos[%i].pDepthStencilState->depthWriteEnable", ParameterName::IndexVector{i}),
                        pCreateInfos[i].pDepthStencilState->depthWriteEnable);

                    skip |= validate_ranged_enum(
                        report_data, "vkCreateGraphicsPipelines",
                        ParameterName("pCreateInfos[%i].pDepthStencilState->depthCompareOp", ParameterName::IndexVector{i}),
                        "VkCompareOp", AllVkCompareOpEnums, pCreateInfos[i].pDepthStencilState->depthCompareOp,
                        "VUID-VkPipelineDepthStencilStateCreateInfo-depthCompareOp-parameter");

                    skip |= validate_bool32(
                        report_data, "vkCreateGraphicsPipelines",
                        ParameterName("pCreateInfos[%i].pDepthStencilState->depthBoundsTestEnable", ParameterName::IndexVector{i}),
                        pCreateInfos[i].pDepthStencilState->depthBoundsTestEnable);

                    skip |= validate_bool32(
                        report_data, "vkCreateGraphicsPipelines",
                        ParameterName("pCreateInfos[%i].pDepthStencilState->stencilTestEnable", ParameterName::IndexVector{i}),
                        pCreateInfos[i].pDepthStencilState->stencilTestEnable);

                    skip |= validate_ranged_enum(
                        report_data, "vkCreateGraphicsPipelines",
                        ParameterName("pCreateInfos[%i].pDepthStencilState->front.failOp", ParameterName::IndexVector{i}),
                        "VkStencilOp", AllVkStencilOpEnums, pCreateInfos[i].pDepthStencilState->front.failOp,
                        "VUID-VkStencilOpState-failOp-parameter");

                    skip |= validate_ranged_enum(
                        report_data, "vkCreateGraphicsPipelines",
                        ParameterName("pCreateInfos[%i].pDepthStencilState->front.passOp", ParameterName::IndexVector{i}),
                        "VkStencilOp", AllVkStencilOpEnums, pCreateInfos[i].pDepthStencilState->front.passOp,
                        "VUID-VkStencilOpState-passOp-parameter");

                    skip |= validate_ranged_enum(
                        report_data, "vkCreateGraphicsPipelines",
                        ParameterName("pCreateInfos[%i].pDepthStencilState->front.depthFailOp", ParameterName::IndexVector{i}),
                        "VkStencilOp", AllVkStencilOpEnums, pCreateInfos[i].pDepthStencilState->front.depthFailOp,
                        "VUID-VkStencilOpState-depthFailOp-parameter");

                    skip |= validate_ranged_enum(
                        report_data, "vkCreateGraphicsPipelines",
                        ParameterName("pCreateInfos[%i].pDepthStencilState->front.compareOp", ParameterName::IndexVector{i}),
                        "VkCompareOp", AllVkCompareOpEnums, pCreateInfos[i].pDepthStencilState->front.compareOp,
                        "VUID-VkPipelineDepthStencilStateCreateInfo-depthCompareOp-parameter");

                    skip |= validate_ranged_enum(
                        report_data, "vkCreateGraphicsPipelines",
                        ParameterName("pCreateInfos[%i].pDepthStencilState->back.failOp", ParameterName::IndexVector{i}),
                        "VkStencilOp", AllVkStencilOpEnums, pCreateInfos[i].pDepthStencilState->back.failOp,
                        "VUID-VkStencilOpState-failOp-parameter");

                    skip |= validate_ranged_enum(
                        report_data, "vkCreateGraphicsPipelines",
                        ParameterName("pCreateInfos[%i].pDepthStencilState->back.passOp", ParameterName::IndexVector{i}),
                        "VkStencilOp", AllVkStencilOpEnums, pCreateInfos[i].pDepthStencilState->back.passOp,
                        "VUID-VkStencilOpState-passOp-parameter");

                    skip |= validate_ranged_enum(
                        report_data, "vkCreateGraphicsPipelines",
                        ParameterName("pCreateInfos[%i].pDepthStencilState->back.depthFailOp", ParameterName::IndexVector{i}),
                        "VkStencilOp", AllVkStencilOpEnums, pCreateInfos[i].pDepthStencilState->back.depthFailOp,
                        "VUID-VkStencilOpState-depthFailOp-parameter");

                    skip |= validate_ranged_enum(
                        report_data, "vkCreateGraphicsPipelines",
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

                if (pCreateInfos[i].pColorBlendState != nullptr && uses_color_attachment) {
                    skip |= validate_struct_pnext(
                        report_data, "vkCreateGraphicsPipelines",
                        ParameterName("pCreateInfos[%i].pColorBlendState->pNext", ParameterName::IndexVector{i}), NULL,
                        pCreateInfos[i].pColorBlendState->pNext, 0, NULL, GeneratedHeaderVersion,
                        "VUID-VkPipelineColorBlendStateCreateInfo-pNext-pNext");

                    skip |= validate_reserved_flags(
                        report_data, "vkCreateGraphicsPipelines",
                        ParameterName("pCreateInfos[%i].pColorBlendState->flags", ParameterName::IndexVector{i}),
                        pCreateInfos[i].pColorBlendState->flags, "VUID-VkPipelineColorBlendStateCreateInfo-flags-zerobitmask");

                    skip |= validate_bool32(
                        report_data, "vkCreateGraphicsPipelines",
                        ParameterName("pCreateInfos[%i].pColorBlendState->logicOpEnable", ParameterName::IndexVector{i}),
                        pCreateInfos[i].pColorBlendState->logicOpEnable);

                    skip |= validate_array(
                        report_data, "vkCreateGraphicsPipelines",
                        ParameterName("pCreateInfos[%i].pColorBlendState->attachmentCount", ParameterName::IndexVector{i}),
                        ParameterName("pCreateInfos[%i].pColorBlendState->pAttachments", ParameterName::IndexVector{i}),
                        pCreateInfos[i].pColorBlendState->attachmentCount, &pCreateInfos[i].pColorBlendState->pAttachments, false,
                        true, kVUIDUndefined, kVUIDUndefined);

                    if (pCreateInfos[i].pColorBlendState->pAttachments != NULL) {
                        for (uint32_t attachmentIndex = 0; attachmentIndex < pCreateInfos[i].pColorBlendState->attachmentCount;
                             ++attachmentIndex) {
                            skip |= validate_bool32(report_data, "vkCreateGraphicsPipelines",
                                                    ParameterName("pCreateInfos[%i].pColorBlendState->pAttachments[%i].blendEnable",
                                                                  ParameterName::IndexVector{i, attachmentIndex}),
                                                    pCreateInfos[i].pColorBlendState->pAttachments[attachmentIndex].blendEnable);

                            skip |= validate_ranged_enum(
                                report_data, "vkCreateGraphicsPipelines",
                                ParameterName("pCreateInfos[%i].pColorBlendState->pAttachments[%i].srcColorBlendFactor",
                                              ParameterName::IndexVector{i, attachmentIndex}),
                                "VkBlendFactor", AllVkBlendFactorEnums,
                                pCreateInfos[i].pColorBlendState->pAttachments[attachmentIndex].srcColorBlendFactor,
                                "VUID-VkPipelineColorBlendAttachmentState-srcColorBlendFactor-parameter");

                            skip |= validate_ranged_enum(
                                report_data, "vkCreateGraphicsPipelines",
                                ParameterName("pCreateInfos[%i].pColorBlendState->pAttachments[%i].dstColorBlendFactor",
                                              ParameterName::IndexVector{i, attachmentIndex}),
                                "VkBlendFactor", AllVkBlendFactorEnums,
                                pCreateInfos[i].pColorBlendState->pAttachments[attachmentIndex].dstColorBlendFactor,
                                "VUID-VkPipelineColorBlendAttachmentState-dstColorBlendFactor-parameter");

                            skip |= validate_ranged_enum(
                                report_data, "vkCreateGraphicsPipelines",
                                ParameterName("pCreateInfos[%i].pColorBlendState->pAttachments[%i].colorBlendOp",
                                              ParameterName::IndexVector{i, attachmentIndex}),
                                "VkBlendOp", AllVkBlendOpEnums,
                                pCreateInfos[i].pColorBlendState->pAttachments[attachmentIndex].colorBlendOp,
                                "VUID-VkPipelineColorBlendAttachmentState-colorBlendOp-parameter");

                            skip |= validate_ranged_enum(
                                report_data, "vkCreateGraphicsPipelines",
                                ParameterName("pCreateInfos[%i].pColorBlendState->pAttachments[%i].srcAlphaBlendFactor",
                                              ParameterName::IndexVector{i, attachmentIndex}),
                                "VkBlendFactor", AllVkBlendFactorEnums,
                                pCreateInfos[i].pColorBlendState->pAttachments[attachmentIndex].srcAlphaBlendFactor,
                                "VUID-VkPipelineColorBlendAttachmentState-srcAlphaBlendFactor-parameter");

                            skip |= validate_ranged_enum(
                                report_data, "vkCreateGraphicsPipelines",
                                ParameterName("pCreateInfos[%i].pColorBlendState->pAttachments[%i].dstAlphaBlendFactor",
                                              ParameterName::IndexVector{i, attachmentIndex}),
                                "VkBlendFactor", AllVkBlendFactorEnums,
                                pCreateInfos[i].pColorBlendState->pAttachments[attachmentIndex].dstAlphaBlendFactor,
                                "VUID-VkPipelineColorBlendAttachmentState-dstAlphaBlendFactor-parameter");

                            skip |= validate_ranged_enum(
                                report_data, "vkCreateGraphicsPipelines",
                                ParameterName("pCreateInfos[%i].pColorBlendState->pAttachments[%i].alphaBlendOp",
                                              ParameterName::IndexVector{i, attachmentIndex}),
                                "VkBlendOp", AllVkBlendOpEnums,
                                pCreateInfos[i].pColorBlendState->pAttachments[attachmentIndex].alphaBlendOp,
                                "VUID-VkPipelineColorBlendAttachmentState-alphaBlendOp-parameter");

                            skip |=
                                validate_flags(report_data, "vkCreateGraphicsPipelines",
                                               ParameterName("pCreateInfos[%i].pColorBlendState->pAttachments[%i].colorWriteMask",
                                                             ParameterName::IndexVector{i, attachmentIndex}),
                                               "VkColorComponentFlagBits", AllVkColorComponentFlagBits,
                                               pCreateInfos[i].pColorBlendState->pAttachments[attachmentIndex].colorWriteMask,
                                               false, false, "VUID-VkPipelineColorBlendAttachmentState-colorWriteMask-parameter");
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
                            report_data, "vkCreateGraphicsPipelines",
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
                if ((pCreateInfos[i].pRasterizationState->polygonMode != VK_POLYGON_MODE_FILL) &&
                    (device_data->physical_device_features.fillModeNonSolid == false)) {
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                    kVUID_PVError_DeviceFeature,
                                    "vkCreateGraphicsPipelines parameter, VkPolygonMode "
                                    "pCreateInfos->pRasterizationState->polygonMode cannot be VK_POLYGON_MODE_POINT or "
                                    "VK_POLYGON_MODE_LINE if VkPhysicalDeviceFeatures->fillModeNonSolid is false.");
                }

                if (!has_dynamic_line_width && !device_data->physical_device_features.wideLines &&
                    (pCreateInfos[i].pRasterizationState->lineWidth != 1.0f)) {
                    skip |=
                        log_msg(device_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT,
                                0, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-00749",
                                "The line width state is static (pCreateInfos[%" PRIu32
                                "].pDynamicState->pDynamicStates does not contain VK_DYNAMIC_STATE_LINE_WIDTH) and "
                                "VkPhysicalDeviceFeatures::wideLines is disabled, but pCreateInfos[%" PRIu32
                                "].pRasterizationState->lineWidth (=%f) is not 1.0.",
                                i, i, pCreateInfos[i].pRasterizationState->lineWidth);
                }
            }

            for (size_t j = 0; j < pCreateInfos[i].stageCount; j++) {
                skip |= validate_string(device_data->report_data, "vkCreateGraphicsPipelines",
                                        ParameterName("pCreateInfos[%i].pStages[%i].pName", ParameterName::IndexVector{i, j}),
                                        "VUID-VkGraphicsPipelineCreateInfo-pStages-parameter", pCreateInfos[i].pStages[j].pName);
            }
        }
    }

    return skip;
}

bool pv_vkCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                 const VkComputePipelineCreateInfo *pCreateInfos, const VkAllocationCallbacks *pAllocator,
                                 VkPipeline *pPipelines) {
    bool skip = false;
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);

    for (uint32_t i = 0; i < createInfoCount; i++) {
        skip |= validate_string(device_data->report_data, "vkCreateComputePipelines",
                                ParameterName("pCreateInfos[%i].stage.pName", ParameterName::IndexVector{i}),
                                "VUID-VkPipelineShaderStageCreateInfo-pName-parameter", pCreateInfos[i].stage.pName);
    }

    return skip;
}

bool pv_vkCreateSampler(VkDevice device, const VkSamplerCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator,
                        VkSampler *pSampler) {
    bool skip = false;
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    debug_report_data *report_data = device_data->report_data;

    if (pCreateInfo != nullptr) {
        const auto &features = device_data->physical_device_features;
        const auto &limits = device_data->device_limits;

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
            skip |=
                validate_ranged_enum(report_data, "vkCreateSampler", "pCreateInfo->compareOp", "VkCompareOp", AllVkCompareOpEnums,
                                     pCreateInfo->compareOp, "VUID-VkSamplerCreateInfo-compareEnable-01080");
        }

        // If any of addressModeU, addressModeV or addressModeW are VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, borderColor must be a
        // valid VkBorderColor value
        if ((pCreateInfo->addressModeU == VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER) ||
            (pCreateInfo->addressModeV == VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER) ||
            (pCreateInfo->addressModeW == VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER)) {
            skip |= validate_ranged_enum(report_data, "vkCreateSampler", "pCreateInfo->borderColor", "VkBorderColor",
                                         AllVkBorderColorEnums, pCreateInfo->borderColor,
                                         "VUID-VkSamplerCreateInfo-addressModeU-01078");
        }

        // If any of addressModeU, addressModeV or addressModeW are VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE, the
        // VK_KHR_sampler_mirror_clamp_to_edge extension must be enabled
        if (!device_data->extensions.vk_khr_sampler_mirror_clamp_to_edge &&
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
        if (device_data->extensions.vk_img_filter_cubic) {
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

bool pv_vkCreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
                                    const VkAllocationCallbacks *pAllocator, VkDescriptorSetLayout *pSetLayout) {
    bool skip = false;
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    debug_report_data *report_data = device_data->report_data;

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

bool pv_vkFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t descriptorSetCount,
                             const VkDescriptorSet *pDescriptorSets) {
    bool skip = false;
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    debug_report_data *report_data = device_data->report_data;

    // Validation for parameters excluded from the generated validation code due to a 'noautovalidity' tag in vk.xml
    // This is an array of handles, where the elements are allowed to be VK_NULL_HANDLE, and does not require any validation beyond
    // validate_array()
    skip |= validate_array(report_data, "vkFreeDescriptorSets", "descriptorSetCount", "pDescriptorSets", descriptorSetCount,
                           &pDescriptorSets, true, true, kVUIDUndefined, kVUIDUndefined);
    return skip;
}

bool pv_vkUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount, const VkWriteDescriptorSet *pDescriptorWrites,
                               uint32_t descriptorCopyCount, const VkCopyDescriptorSet *pDescriptorCopies) {
    bool skip = false;
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    debug_report_data *report_data = device_data->report_data;

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
            skip |= validate_required_handle(report_data, "vkUpdateDescriptorSets",
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
                        skip |= validate_required_handle(report_data, "vkUpdateDescriptorSets",
                                                         ParameterName("pDescriptorWrites[%i].pImageInfo[%i].imageView",
                                                                       ParameterName::IndexVector{i, descriptor_index}),
                                                         pDescriptorWrites[i].pImageInfo[descriptor_index].imageView);
                        skip |= validate_ranged_enum(report_data, "vkUpdateDescriptorSets",
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
                        skip |= validate_required_handle(report_data, "vkUpdateDescriptorSets",
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
                        skip |= validate_required_handle(report_data, "vkUpdateDescriptorSets",
                                                         ParameterName("pDescriptorWrites[%i].pTexelBufferView[%i]",
                                                                       ParameterName::IndexVector{i, descriptor_index}),
                                                         pDescriptorWrites[i].pTexelBufferView[descriptor_index]);
                    }
                }
            }

            if ((pDescriptorWrites[i].descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) ||
                (pDescriptorWrites[i].descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC)) {
                VkDeviceSize uniformAlignment = device_data->device_limits.minUniformBufferOffsetAlignment;
                for (uint32_t j = 0; j < pDescriptorWrites[i].descriptorCount; j++) {
                    if (pDescriptorWrites[i].pBufferInfo != NULL) {
                        if (SafeModulo(pDescriptorWrites[i].pBufferInfo[j].offset, uniformAlignment) != 0) {
                            skip |=
                                log_msg(device_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                        VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT, 0,
                                        "VUID-VkWriteDescriptorSet-descriptorType-00327",
                                        "vkUpdateDescriptorSets(): pDescriptorWrites[%d].pBufferInfo[%d].offset (0x%" PRIxLEAST64
                                        ") must be a multiple of device limit minUniformBufferOffsetAlignment 0x%" PRIxLEAST64 ".",
                                        i, j, pDescriptorWrites[i].pBufferInfo[j].offset, uniformAlignment);
                        }
                    }
                }
            } else if ((pDescriptorWrites[i].descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) ||
                       (pDescriptorWrites[i].descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)) {
                VkDeviceSize storageAlignment = device_data->device_limits.minStorageBufferOffsetAlignment;
                for (uint32_t j = 0; j < pDescriptorWrites[i].descriptorCount; j++) {
                    if (pDescriptorWrites[i].pBufferInfo != NULL) {
                        if (SafeModulo(pDescriptorWrites[i].pBufferInfo[j].offset, storageAlignment) != 0) {
                            skip |=
                                log_msg(device_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                        VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT, 0,
                                        "VUID-VkWriteDescriptorSet-descriptorType-00328",
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

bool pv_vkCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator,
                           VkRenderPass *pRenderPass) {
    bool skip = false;
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    uint32_t max_color_attachments = device_data->device_limits.maxColorAttachments;

    for (uint32_t i = 0; i < pCreateInfo->attachmentCount; ++i) {
        if (pCreateInfo->pAttachments[i].format == VK_FORMAT_UNDEFINED) {
            std::stringstream ss;
            ss << "vkCreateRenderPass: pCreateInfo->pAttachments[" << i << "].format is VK_FORMAT_UNDEFINED. ";
            skip |= log_msg(device_data->report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            "VUID-VkAttachmentDescription-format-parameter", "%s", ss.str().c_str());
        }
        if (pCreateInfo->pAttachments[i].finalLayout == VK_IMAGE_LAYOUT_UNDEFINED ||
            pCreateInfo->pAttachments[i].finalLayout == VK_IMAGE_LAYOUT_PREINITIALIZED) {
            skip |= log_msg(device_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            "VUID-VkAttachmentDescription-finalLayout-00843",
                            "pCreateInfo->pAttachments[%d].finalLayout must not be VK_IMAGE_LAYOUT_UNDEFINED or "
                            "VK_IMAGE_LAYOUT_PREINITIALIZED.",
                            i);
        }
    }

    for (uint32_t i = 0; i < pCreateInfo->subpassCount; ++i) {
        if (pCreateInfo->pSubpasses[i].colorAttachmentCount > max_color_attachments) {
            skip |= log_msg(device_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            "VUID-VkSubpassDescription-colorAttachmentCount-00845",
                            "Cannot create a render pass with %d color attachments. Max is %d.",
                            pCreateInfo->pSubpasses[i].colorAttachmentCount, max_color_attachments);
        }
    }
    return skip;
}

bool pv_vkFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount,
                             const VkCommandBuffer *pCommandBuffers) {
    bool skip = false;
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    debug_report_data *report_data = device_data->report_data;

    // Validation for parameters excluded from the generated validation code due to a 'noautovalidity' tag in vk.xml
    // This is an array of handles, where the elements are allowed to be VK_NULL_HANDLE, and does not require any validation beyond
    // validate_array()
    skip |= validate_array(report_data, "vkFreeCommandBuffers", "commandBufferCount", "pCommandBuffers", commandBufferCount,
                           &pCommandBuffers, true, true, kVUIDUndefined, kVUIDUndefined);
    return skip;
}

bool pv_vkBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo *pBeginInfo) {
    bool skip = false;
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    debug_report_data *report_data = device_data->report_data;
    const VkCommandBufferInheritanceInfo *pInfo = pBeginInfo->pInheritanceInfo;

    // Validation for parameters excluded from the generated validation code due to a 'noautovalidity' tag in vk.xml
    // TODO: pBeginInfo->pInheritanceInfo must not be NULL if commandBuffer is a secondary command buffer
    skip |= validate_struct_type(report_data, "vkBeginCommandBuffer", "pBeginInfo->pInheritanceInfo",
                                 "VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO", pBeginInfo->pInheritanceInfo,
                                 VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO, false,
                                 "VUID_vkBeginCommandBuffer-pBeginInfo-parameter", "VUID_VkCommandBufferBeginInfo-sType-sType");

    if (pBeginInfo->pInheritanceInfo != NULL) {
        skip |= validate_struct_pnext(report_data, "vkBeginCommandBuffer", "pBeginInfo->pInheritanceInfo->pNext", NULL,
                                      pBeginInfo->pInheritanceInfo->pNext, 0, NULL, GeneratedHeaderVersion,
                                      "VUID-VkCommandBufferBeginInfo-pNext-pNext");

        skip |= validate_bool32(report_data, "vkBeginCommandBuffer", "pBeginInfo->pInheritanceInfo->occlusionQueryEnable",
                                pBeginInfo->pInheritanceInfo->occlusionQueryEnable);

        // TODO: This only needs to be validated when the inherited queries feature is enabled
        // skip |= validate_flags(report_data, "vkBeginCommandBuffer", "pBeginInfo->pInheritanceInfo->queryFlags",
        // "VkQueryControlFlagBits", AllVkQueryControlFlagBits, pBeginInfo->pInheritanceInfo->queryFlags, false);

        // TODO: This must be 0 if the pipeline statistics queries feature is not enabled
        skip |= validate_flags(report_data, "vkBeginCommandBuffer", "pBeginInfo->pInheritanceInfo->pipelineStatistics",
                               "VkQueryPipelineStatisticFlagBits", AllVkQueryPipelineStatisticFlagBits,
                               pBeginInfo->pInheritanceInfo->pipelineStatistics, false, false, kVUIDUndefined);
    }

    if (pInfo != NULL) {
        if ((device_data->physical_device_features.inheritedQueries == VK_FALSE) && (pInfo->occlusionQueryEnable != VK_FALSE)) {
            skip |= log_msg(device_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(commandBuffer), "VUID-VkCommandBufferInheritanceInfo-occlusionQueryEnable-00056",
                            "Cannot set inherited occlusionQueryEnable in vkBeginCommandBuffer() when device does not support "
                            "inheritedQueries.");
        }
        if ((device_data->physical_device_features.inheritedQueries != VK_FALSE) && (pInfo->occlusionQueryEnable != VK_FALSE)) {
            skip |= validate_flags(device_data->report_data, "vkBeginCommandBuffer", "pBeginInfo->pInheritanceInfo->queryFlags",
                                   "VkQueryControlFlagBits", AllVkQueryControlFlagBits, pInfo->queryFlags, false, false,
                                   "VUID-VkCommandBufferInheritanceInfo-queryFlags-00057");
        }
    }

    return skip;
}

bool pv_vkCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                         const VkViewport *pViewports) {
    bool skip = false;
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    if (!device_data->physical_device_features.multiViewport) {
        if (firstViewport != 0) {
            skip |= log_msg(device_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(commandBuffer), "VUID-vkCmdSetViewport-firstViewport-01224",
                            "vkCmdSetViewport: The multiViewport feature is disabled, but firstViewport (=%" PRIu32 ") is not 0.",
                            firstViewport);
        }
        if (viewportCount > 1) {
            skip |= log_msg(device_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(commandBuffer), "VUID-vkCmdSetViewport-viewportCount-01225",
                            "vkCmdSetViewport: The multiViewport feature is disabled, but viewportCount (=%" PRIu32 ") is not 1.",
                            viewportCount);
        }
    } else {  // multiViewport enabled
        const uint64_t sum = static_cast<uint64_t>(firstViewport) + static_cast<uint64_t>(viewportCount);
        if (sum > device_data->device_limits.maxViewports) {
            skip |= log_msg(device_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(commandBuffer), "VUID-vkCmdSetViewport-firstViewport-01223",
                            "vkCmdSetViewport: firstViewport + viewportCount (=%" PRIu32 " + %" PRIu32 " = %" PRIu64
                            ") is greater than VkPhysicalDeviceLimits::maxViewports (=%" PRIu32 ").",
                            firstViewport, viewportCount, sum, device_data->device_limits.maxViewports);
        }
    }

    if (pViewports) {
        for (uint32_t viewport_i = 0; viewport_i < viewportCount; ++viewport_i) {
            const auto &viewport = pViewports[viewport_i];  // will crash on invalid ptr
            const char fn_name[] = "vkCmdSetViewport";
            const std::string param_name = "pViewports[" + std::to_string(viewport_i) + "]";
            skip |= pv_VkViewport(device_data, viewport, fn_name, param_name.c_str(),
                                  VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, HandleToUint64(commandBuffer));
        }
    }

    return skip;
}

bool pv_vkCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount, const VkRect2D *pScissors) {
    bool skip = false;
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    debug_report_data *report_data = device_data->report_data;

    if (!device_data->physical_device_features.multiViewport) {
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
        if (sum > device_data->device_limits.maxViewports) {
            skip |= log_msg(device_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(commandBuffer), "VUID-vkCmdSetScissor-firstScissor-00592",
                            "vkCmdSetScissor: firstScissor + scissorCount (=%" PRIu32 " + %" PRIu32 " = %" PRIu64
                            ") is greater than VkPhysicalDeviceLimits::maxViewports (=%" PRIu32 ").",
                            firstScissor, scissorCount, sum, device_data->device_limits.maxViewports);
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

bool pv_vkCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth) {
    bool skip = false;
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    debug_report_data *report_data = device_data->report_data;

    if (!device_data->physical_device_features.wideLines && (lineWidth != 1.0f)) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), "VUID-vkCmdSetLineWidth-lineWidth-00788",
                        "VkPhysicalDeviceFeatures::wideLines is disabled, but lineWidth (=%f) is not 1.0.", lineWidth);
    }

    return skip;
}

bool pv_vkCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                  uint32_t firstInstance) {
    bool skip = false;
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (vertexCount == 0) {
        // TODO: Verify against Valid Usage section. I don't see a non-zero vertexCount listed, may need to add that and make
        // this an error or leave as is.
        skip |= log_msg(device_data->report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        kVUID_PVError_RequiredParameter, "vkCmdDraw parameter, uint32_t vertexCount, is 0");
    }

    if (instanceCount == 0) {
        // TODO: Verify against Valid Usage section. I don't see a non-zero instanceCount listed, may need to add that and make
        // this an error or leave as is.
        skip |= log_msg(device_data->report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        kVUID_PVError_RequiredParameter, "vkCmdDraw parameter, uint32_t instanceCount, is 0");
    }
    return skip;
}

bool pv_vkCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count, uint32_t stride) {
    bool skip = false;
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    if (!device_data->physical_device_features.multiDrawIndirect && ((count > 1))) {
        skip |= log_msg(device_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        kVUID_PVError_DeviceFeature,
                        "CmdDrawIndirect(): Device feature multiDrawIndirect disabled: count must be 0 or 1 but is %d", count);
    }
    return skip;
}

bool pv_vkCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count,
                                 uint32_t stride) {
    bool skip = false;
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!device_data->physical_device_features.multiDrawIndirect && ((count > 1))) {
        skip |=
            log_msg(device_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                    kVUID_PVError_DeviceFeature,
                    "CmdDrawIndexedIndirect(): Device feature multiDrawIndirect disabled: count must be 0 or 1 but is %d", count);
    }
    return skip;
}

bool pv_vkCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                       VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy *pRegions) {
    bool skip = false;
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    VkImageAspectFlags legal_aspect_flags =
        VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_METADATA_BIT;
    if (device_data->extensions.vk_khr_sampler_ycbcr_conversion) {
        legal_aspect_flags |= (VK_IMAGE_ASPECT_PLANE_0_BIT_KHR | VK_IMAGE_ASPECT_PLANE_1_BIT_KHR | VK_IMAGE_ASPECT_PLANE_2_BIT_KHR);
    }

    if (pRegions != nullptr) {
        if ((pRegions->srcSubresource.aspectMask & legal_aspect_flags) == 0) {
            skip |= log_msg(
                device_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                "VUID-VkImageSubresourceLayers-aspectMask-parameter",
                "vkCmdCopyImage() parameter, VkImageAspect pRegions->srcSubresource.aspectMask, is an unrecognized enumerator.");
        }
        if ((pRegions->dstSubresource.aspectMask & legal_aspect_flags) == 0) {
            skip |= log_msg(
                device_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                "VUID-VkImageSubresourceLayers-aspectMask-parameter",
                "vkCmdCopyImage() parameter, VkImageAspect pRegions->dstSubresource.aspectMask, is an unrecognized enumerator.");
        }
    }
    return skip;
}

bool pv_vkCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                       VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit *pRegions, VkFilter filter) {
    bool skip = false;
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    VkImageAspectFlags legal_aspect_flags =
        VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_METADATA_BIT;
    if (device_data->extensions.vk_khr_sampler_ycbcr_conversion) {
        legal_aspect_flags |= (VK_IMAGE_ASPECT_PLANE_0_BIT_KHR | VK_IMAGE_ASPECT_PLANE_1_BIT_KHR | VK_IMAGE_ASPECT_PLANE_2_BIT_KHR);
    }

    if (pRegions != nullptr) {
        if ((pRegions->srcSubresource.aspectMask & legal_aspect_flags) == 0) {
            skip |= log_msg(
                device_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                kVUID_PVError_UnrecognizedValue,
                "vkCmdBlitImage() parameter, VkImageAspect pRegions->srcSubresource.aspectMask, is an unrecognized enumerator");
        }
        if ((pRegions->dstSubresource.aspectMask & legal_aspect_flags) == 0) {
            skip |= log_msg(
                device_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                kVUID_PVError_UnrecognizedValue,
                "vkCmdBlitImage() parameter, VkImageAspect pRegions->dstSubresource.aspectMask, is an unrecognized enumerator");
        }
    }
    return skip;
}

bool pv_vkCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage, VkImageLayout dstImageLayout,
                               uint32_t regionCount, const VkBufferImageCopy *pRegions) {
    bool skip = false;
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    VkImageAspectFlags legal_aspect_flags =
        VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_METADATA_BIT;
    if (device_data->extensions.vk_khr_sampler_ycbcr_conversion) {
        legal_aspect_flags |= (VK_IMAGE_ASPECT_PLANE_0_BIT_KHR | VK_IMAGE_ASPECT_PLANE_1_BIT_KHR | VK_IMAGE_ASPECT_PLANE_2_BIT_KHR);
    }

    if (pRegions != nullptr) {
        if ((pRegions->imageSubresource.aspectMask & legal_aspect_flags) == 0) {
            skip |= log_msg(device_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            kVUID_PVError_UnrecognizedValue,
                            "vkCmdCopyBufferToImage() parameter, VkImageAspect pRegions->imageSubresource.aspectMask, is an "
                            "unrecognized enumerator");
        }
    }
    return skip;
}

bool pv_vkCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkBuffer dstBuffer,
                               uint32_t regionCount, const VkBufferImageCopy *pRegions) {
    bool skip = false;
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    VkImageAspectFlags legal_aspect_flags =
        VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_METADATA_BIT;
    if (device_data->extensions.vk_khr_sampler_ycbcr_conversion) {
        legal_aspect_flags |= (VK_IMAGE_ASPECT_PLANE_0_BIT_KHR | VK_IMAGE_ASPECT_PLANE_1_BIT_KHR | VK_IMAGE_ASPECT_PLANE_2_BIT_KHR);
    }

    if (pRegions != nullptr) {
        if ((pRegions->imageSubresource.aspectMask & legal_aspect_flags) == 0) {
            log_msg(device_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                    kVUID_PVError_UnrecognizedValue,
                    "vkCmdCopyImageToBuffer parameter, VkImageAspect pRegions->imageSubresource.aspectMask, is an unrecognized "
                    "enumerator");
        }
    }
    return skip;
}

bool pv_vkCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize dataSize,
                          const void *pData) {
    bool skip = false;
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    if (dstOffset & 3) {
        skip |= log_msg(device_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        "VUID-vkCmdUpdateBuffer-dstOffset-00036",
                        "vkCmdUpdateBuffer() parameter, VkDeviceSize dstOffset (0x%" PRIxLEAST64 "), is not a multiple of 4.",
                        dstOffset);
    }

    if ((dataSize <= 0) || (dataSize > 65536)) {
        skip |= log_msg(device_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        "VUID-vkCmdUpdateBuffer-dataSize-00037",
                        "vkCmdUpdateBuffer() parameter, VkDeviceSize dataSize (0x%" PRIxLEAST64
                        "), must be greater than zero and less than or equal to 65536.",
                        dataSize);
    } else if (dataSize & 3) {
        skip |=
            log_msg(device_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                    "VUID-vkCmdUpdateBuffer-dataSize-00038",
                    "vkCmdUpdateBuffer() parameter, VkDeviceSize dataSize (0x%" PRIxLEAST64 "), is not a multiple of 4.", dataSize);
    }
    return skip;
}

bool pv_vkCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size,
                        uint32_t data) {
    bool skip = false;
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    if (dstOffset & 3) {
        skip |=
            log_msg(device_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                    "VUID-vkCmdFillBuffer-dstOffset-00025",
                    "vkCmdFillBuffer() parameter, VkDeviceSize dstOffset (0x%" PRIxLEAST64 "), is not a multiple of 4.", dstOffset);
    }

    if (size != VK_WHOLE_SIZE) {
        if (size <= 0) {
            skip |=
                log_msg(device_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        "VUID-vkCmdFillBuffer-size-00026",
                        "vkCmdFillBuffer() parameter, VkDeviceSize size (0x%" PRIxLEAST64 "), must be greater than zero.", size);
        } else if (size & 3) {
            skip |= log_msg(device_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            "VUID-vkCmdFillBuffer-size-00028",
                            "vkCmdFillBuffer() parameter, VkDeviceSize size (0x%" PRIxLEAST64 "), is not a multiple of 4.", size);
        }
    }
    return skip;
}

VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateInstanceLayerProperties(uint32_t *pCount, VkLayerProperties *pProperties) {
    return util_GetLayerProperties(1, &global_layer, pCount, pProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice, uint32_t *pCount,
                                                                VkLayerProperties *pProperties) {
    return util_GetLayerProperties(1, &global_layer, pCount, pProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateInstanceExtensionProperties(const char *pLayerName, uint32_t *pCount,
                                                                      VkExtensionProperties *pProperties) {
    if (pLayerName && !strcmp(pLayerName, global_layer.layerName))
        return util_GetExtensionProperties(1, instance_extensions, pCount, pProperties);

    return VK_ERROR_LAYER_NOT_PRESENT;
}

VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char *pLayerName,
                                                                    uint32_t *pPropertyCount, VkExtensionProperties *pProperties) {
    // Parameter_validation does not have any physical device extensions
    if (pLayerName && !strcmp(pLayerName, global_layer.layerName))
        return util_GetExtensionProperties(0, NULL, pPropertyCount, pProperties);

    instance_layer_data *local_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    bool skip = validate_array(local_data->report_data, "vkEnumerateDeviceExtensionProperties", "pPropertyCount", "pProperties",
                               pPropertyCount, &pProperties, true, false, false, kVUIDUndefined,
                               "VUID-vkEnumerateDeviceExtensionProperties-pProperties-parameter");
    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;

    return local_data->dispatch_table.EnumerateDeviceExtensionProperties(physicalDevice, NULL, pPropertyCount, pProperties);
}

static bool require_device_extension(layer_data *device_data, bool flag, char const *function_name, char const *extension_name) {
    if (!flag) {
        return log_msg(device_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                       kVUID_PVError_ExtensionNotEnabled,
                       "%s() called even though the %s extension was not enabled for this VkDevice.", function_name,
                       extension_name);
    }

    return false;
}

bool pv_vkCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR *pCreateInfo, const VkAllocationCallbacks *pAllocator,
                             VkSwapchainKHR *pSwapchain) {
    bool skip = false;
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    debug_report_data *report_data = device_data->report_data;

    const LogMiscParams log_misc{report_data, VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT, VK_NULL_HANDLE,
                                 "vkCreateSwapchainKHR"};

    if (pCreateInfo != nullptr) {
        if ((device_data->physical_device_features.textureCompressionETC2 == false) &&
            FormatIsCompressed_ETC2_EAC(pCreateInfo->imageFormat)) {
            skip |= log_msg(
                report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, kVUID_PVError_DeviceFeature,
                "vkCreateSwapchainKHR(): Attempting to create swapchain VkImage with format %s. The textureCompressionETC2 "
                "feature is not enabled: neither ETC2 nor EAC formats can be used to create images.",
                string_VkFormat(pCreateInfo->imageFormat));
        }

        if ((device_data->physical_device_features.textureCompressionASTC_LDR == false) &&
            FormatIsCompressed_ASTC_LDR(pCreateInfo->imageFormat)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            kVUID_PVError_DeviceFeature,
                            "vkCreateSwapchainKHR(): Attempting to create swapchain VkImage with format %s. The "
                            "textureCompressionASTC_LDR feature is not enabled: ASTC formats cannot be used to create images.",
                            string_VkFormat(pCreateInfo->imageFormat));
        }

        if ((device_data->physical_device_features.textureCompressionBC == false) &&
            FormatIsCompressed_BC(pCreateInfo->imageFormat)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            kVUID_PVError_DeviceFeature,
                            "vkCreateSwapchainKHR(): Attempting to create swapchain VkImage with format %s. The "
                            "textureCompressionBC feature is not enabled: BC compressed formats cannot be used to create images.",
                            string_VkFormat(pCreateInfo->imageFormat));
        }

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
            } else {
                skip |= ValidateQueueFamilies(device_data, pCreateInfo->queueFamilyIndexCount, pCreateInfo->pQueueFamilyIndices,
                                              "vkCreateSwapchainKHR", "pCreateInfo->pQueueFamilyIndices",
                                              kVUID_PVError_InvalidUsage, kVUID_PVError_InvalidUsage, false);
            }
        }

        skip |= ValidateGreaterThanZero(pCreateInfo->imageArrayLayers, "pCreateInfo->imageArrayLayers",
                                        "VUID-VkSwapchainCreateInfoKHR-imageArrayLayers-01275", log_misc);
    }

    return skip;
}

bool pv_vkQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR *pPresentInfo) {
    bool skip = false;
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(queue), layer_data_map);

    if (pPresentInfo && pPresentInfo->pNext) {
        const auto *present_regions = lvl_find_in_chain<VkPresentRegionsKHR>(pPresentInfo->pNext);
        if (present_regions) {
            // TODO: This and all other pNext extension dependencies should be added to code-generation
            skip |= require_device_extension(device_data, device_data->extensions.vk_khr_incremental_present, "vkQueuePresentKHR",
                                             VK_KHR_INCREMENTAL_PRESENT_EXTENSION_NAME);
            if (present_regions->swapchainCount != pPresentInfo->swapchainCount) {
                skip |= log_msg(device_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                kVUID_PVError_InvalidUsage,
                                "QueuePresentKHR(): pPresentInfo->swapchainCount has a value of %i but VkPresentRegionsKHR "
                                "extension swapchainCount is %i. These values must be equal.",
                                pPresentInfo->swapchainCount, present_regions->swapchainCount);
            }
            skip |=
                validate_struct_pnext(device_data->report_data, "QueuePresentKHR", "pCreateInfo->pNext->pNext", NULL,
                                      present_regions->pNext, 0, NULL, GeneratedHeaderVersion, "VUID-VkPresentInfoKHR-pNext-pNext");
            skip |= validate_array(device_data->report_data, "QueuePresentKHR", "pCreateInfo->pNext->swapchainCount",
                                   "pCreateInfo->pNext->pRegions", present_regions->swapchainCount, &present_regions->pRegions,
                                   true, false, kVUIDUndefined, kVUIDUndefined);
            for (uint32_t i = 0; i < present_regions->swapchainCount; ++i) {
                skip |= validate_array(device_data->report_data, "QueuePresentKHR", "pCreateInfo->pNext->pRegions[].rectangleCount",
                                       "pCreateInfo->pNext->pRegions[].pRectangles", present_regions->pRegions[i].rectangleCount,
                                       &present_regions->pRegions[i].pRectangles, true, false, kVUIDUndefined, kVUIDUndefined);
            }
        }
    }

    return skip;
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
bool pv_vkCreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR *pCreateInfo,
                                const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface) {
    auto device_data = GetLayerDataPtr(get_dispatch_key(instance), instance_layer_data_map);
    bool skip = false;

    if (pCreateInfo->hwnd == nullptr) {
        skip |= log_msg(device_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        "VUID-VkWin32SurfaceCreateInfoKHR-hwnd-01308",
                        "vkCreateWin32SurfaceKHR(): hwnd must be a valid Win32 HWND but hwnd is NULL.");
    }

    return skip;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

bool pv_vkDebugMarkerSetObjectNameEXT(VkDevice device, const VkDebugMarkerObjectNameInfoEXT *pNameInfo) {
    auto device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (pNameInfo->pObjectName) {
        device_data->report_data->debugObjectNameMap->insert(
            std::make_pair<uint64_t, std::string>((uint64_t &&) pNameInfo->object, pNameInfo->pObjectName));
    } else {
        device_data->report_data->debugObjectNameMap->erase(pNameInfo->object);
    }
    return false;
}

bool pv_vkCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo *pCreateInfo,
                               const VkAllocationCallbacks *pAllocator, VkDescriptorPool *pDescriptorPool) {
    auto device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;

    if (pCreateInfo) {
        if (pCreateInfo->maxSets <= 0) {
            skip |=
                log_msg(device_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT,
                        VK_NULL_HANDLE, "VUID-VkDescriptorPoolCreateInfo-maxSets-00301",
                        "vkCreateDescriptorPool(): pCreateInfo->maxSets is not greater than 0.");
        }

        if (pCreateInfo->pPoolSizes) {
            for (uint32_t i = 0; i < pCreateInfo->poolSizeCount; ++i) {
                if (pCreateInfo->pPoolSizes[i].descriptorCount <= 0) {
                    skip |= log_msg(
                        device_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT,
                        VK_NULL_HANDLE, "VUID-VkDescriptorPoolSize-descriptorCount-00302",
                        "vkCreateDescriptorPool(): pCreateInfo->pPoolSizes[%" PRIu32 "].descriptorCount is not greater than 0.", i);
                }
            }
        }
    }

    return skip;
}

bool pv_vkCmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
    bool skip = false;
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    if (groupCountX > device_data->device_limits.maxComputeWorkGroupCount[0]) {
        skip |=
            log_msg(device_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                    HandleToUint64(commandBuffer), "VUID-vkCmdDispatch-groupCountX-00386",
                    "vkCmdDispatch(): groupCountX (%" PRIu32 ") exceeds device limit maxComputeWorkGroupCount[0] (%" PRIu32 ").",
                    groupCountX, device_data->device_limits.maxComputeWorkGroupCount[0]);
    }

    if (groupCountY > device_data->device_limits.maxComputeWorkGroupCount[1]) {
        skip |=
            log_msg(device_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                    HandleToUint64(commandBuffer), "VUID-vkCmdDispatch-groupCountY-00387",
                    "vkCmdDispatch(): groupCountY (%" PRIu32 ") exceeds device limit maxComputeWorkGroupCount[1] (%" PRIu32 ").",
                    groupCountY, device_data->device_limits.maxComputeWorkGroupCount[1]);
    }

    if (groupCountZ > device_data->device_limits.maxComputeWorkGroupCount[2]) {
        skip |=
            log_msg(device_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                    HandleToUint64(commandBuffer), "VUID-vkCmdDispatch-groupCountZ-00388",
                    "vkCmdDispatch(): groupCountZ (%" PRIu32 ") exceeds device limit maxComputeWorkGroupCount[2] (%" PRIu32 ").",
                    groupCountZ, device_data->device_limits.maxComputeWorkGroupCount[2]);
    }

    return skip;
}

bool pv_vkCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ,
                             uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
    bool skip = false;
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    // Paired if {} else if {} tests used to avoid any possible uint underflow
    uint32_t limit = device_data->device_limits.maxComputeWorkGroupCount[0];
    if (baseGroupX >= limit) {
        skip |= log_msg(device_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), "VUID-vkCmdDispatchBase-baseGroupX-00421",
                        "vkCmdDispatch(): baseGroupX (%" PRIu32
                        ") equals or exceeds device limit maxComputeWorkGroupCount[0] (%" PRIu32 ").",
                        baseGroupX, limit);
    } else if (groupCountX > (limit - baseGroupX)) {
        skip |= log_msg(device_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), "VUID-vkCmdDispatchBase-groupCountX-00424",
                        "vkCmdDispatchBaseKHR(): baseGroupX (%" PRIu32 ") + groupCountX (%" PRIu32
                        ") exceeds device limit maxComputeWorkGroupCount[0] (%" PRIu32 ").",
                        baseGroupX, groupCountX, limit);
    }

    limit = device_data->device_limits.maxComputeWorkGroupCount[1];
    if (baseGroupY >= limit) {
        skip |= log_msg(device_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), "VUID-vkCmdDispatchBase-baseGroupX-00422",
                        "vkCmdDispatch(): baseGroupY (%" PRIu32
                        ") equals or exceeds device limit maxComputeWorkGroupCount[1] (%" PRIu32 ").",
                        baseGroupY, limit);
    } else if (groupCountY > (limit - baseGroupY)) {
        skip |= log_msg(device_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), "VUID-vkCmdDispatchBase-groupCountY-00425",
                        "vkCmdDispatchBaseKHR(): baseGroupY (%" PRIu32 ") + groupCountY (%" PRIu32
                        ") exceeds device limit maxComputeWorkGroupCount[1] (%" PRIu32 ").",
                        baseGroupY, groupCountY, limit);
    }

    limit = device_data->device_limits.maxComputeWorkGroupCount[2];
    if (baseGroupZ >= limit) {
        skip |= log_msg(device_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), "VUID-vkCmdDispatchBase-baseGroupZ-00423",
                        "vkCmdDispatch(): baseGroupZ (%" PRIu32
                        ") equals or exceeds device limit maxComputeWorkGroupCount[2] (%" PRIu32 ").",
                        baseGroupZ, limit);
    } else if (groupCountZ > (limit - baseGroupZ)) {
        skip |= log_msg(device_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), "VUID-vkCmdDispatchBase-groupCountZ-00426",
                        "vkCmdDispatchBaseKHR(): baseGroupZ (%" PRIu32 ") + groupCountZ (%" PRIu32
                        ") exceeds device limit maxComputeWorkGroupCount[2] (%" PRIu32 ").",
                        baseGroupZ, groupCountZ, limit);
    }

    return skip;
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetDeviceProcAddr(VkDevice device, const char *funcName) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!ApiParentExtensionEnabled(funcName, device_data->extensions.device_extension_set)) {
        return nullptr;
    }
    const auto item = name_to_funcptr_map.find(funcName);
    if (item != name_to_funcptr_map.end()) {
        return reinterpret_cast<PFN_vkVoidFunction>(item->second);
    }
    const auto &table = device_data->dispatch_table;
    if (!table.GetDeviceProcAddr) return nullptr;
    return table.GetDeviceProcAddr(device, funcName);
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetInstanceProcAddr(VkInstance instance, const char *funcName) {
    const auto item = name_to_funcptr_map.find(funcName);
    if (item != name_to_funcptr_map.end()) {
        return reinterpret_cast<PFN_vkVoidFunction>(item->second);
    }

    auto instance_data = GetLayerDataPtr(get_dispatch_key(instance), instance_layer_data_map);
    auto &table = instance_data->dispatch_table;
    if (!table.GetInstanceProcAddr) return nullptr;
    return table.GetInstanceProcAddr(instance, funcName);
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetPhysicalDeviceProcAddr(VkInstance instance, const char *funcName) {
    assert(instance);
    auto instance_data = GetLayerDataPtr(get_dispatch_key(instance), instance_layer_data_map);

    if (!instance_data->dispatch_table.GetPhysicalDeviceProcAddr) return nullptr;
    return instance_data->dispatch_table.GetPhysicalDeviceProcAddr(instance, funcName);
}

// If additional validation is needed outside of the generated checks, a manual routine can be added to this file
// and the address filled in here. The autogenerated source will call these routines if the pointers are not NULL.
void InitializeManualParameterValidationFunctionPointers() {
    custom_functions["vkGetDeviceQueue"] = (void *)pv_vkGetDeviceQueue;
    custom_functions["vkCreateBuffer"] = (void *)pv_vkCreateBuffer;
    custom_functions["vkCreateImage"] = (void *)pv_vkCreateImage;
    custom_functions["vkCreateImageView"] = (void *)pv_vkCreateImageView;
    custom_functions["vkCreateGraphicsPipelines"] = (void *)pv_vkCreateGraphicsPipelines;
    custom_functions["vkCreateComputePipelines"] = (void *)pv_vkCreateComputePipelines;
    custom_functions["vkCreateSampler"] = (void *)pv_vkCreateSampler;
    custom_functions["vkCreateDescriptorSetLayout"] = (void *)pv_vkCreateDescriptorSetLayout;
    custom_functions["vkFreeDescriptorSets"] = (void *)pv_vkFreeDescriptorSets;
    custom_functions["vkUpdateDescriptorSets"] = (void *)pv_vkUpdateDescriptorSets;
    custom_functions["vkCreateRenderPass"] = (void *)pv_vkCreateRenderPass;
    custom_functions["vkBeginCommandBuffer"] = (void *)pv_vkBeginCommandBuffer;
    custom_functions["vkCmdSetViewport"] = (void *)pv_vkCmdSetViewport;
    custom_functions["vkCmdSetScissor"] = (void *)pv_vkCmdSetScissor;
    custom_functions["vkCmdSetLineWidth"] = (void *)pv_vkCmdSetLineWidth;
    custom_functions["vkCmdDraw"] = (void *)pv_vkCmdDraw;
    custom_functions["vkCmdDrawIndirect"] = (void *)pv_vkCmdDrawIndirect;
    custom_functions["vkCmdDrawIndexedIndirect"] = (void *)pv_vkCmdDrawIndexedIndirect;
    custom_functions["vkCmdCopyImage"] = (void *)pv_vkCmdCopyImage;
    custom_functions["vkCmdBlitImage"] = (void *)pv_vkCmdBlitImage;
    custom_functions["vkCmdCopyBufferToImage"] = (void *)pv_vkCmdCopyBufferToImage;
    custom_functions["vkCmdCopyImageToBuffer"] = (void *)pv_vkCmdCopyImageToBuffer;
    custom_functions["vkCmdUpdateBuffer"] = (void *)pv_vkCmdUpdateBuffer;
    custom_functions["vkCmdFillBuffer"] = (void *)pv_vkCmdFillBuffer;
    custom_functions["vkCreateSwapchainKHR"] = (void *)pv_vkCreateSwapchainKHR;
    custom_functions["vkQueuePresentKHR"] = (void *)pv_vkQueuePresentKHR;
    custom_functions["vkCreateDescriptorPool"] = (void *)pv_vkCreateDescriptorPool;
    custom_functions["vkCmdDispatch"] = (void *)pv_vkCmdDispatch;
    custom_functions["vkCmdDispatchBaseKHR"] = (void *)pv_vkCmdDispatchBaseKHR;
}

}  // namespace parameter_validation

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateInstanceExtensionProperties(const char *pLayerName, uint32_t *pCount,
                                                                                      VkExtensionProperties *pProperties) {
    return parameter_validation::vkEnumerateInstanceExtensionProperties(pLayerName, pCount, pProperties);
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateInstanceLayerProperties(uint32_t *pCount,
                                                                                  VkLayerProperties *pProperties) {
    return parameter_validation::vkEnumerateInstanceLayerProperties(pCount, pProperties);
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice, uint32_t *pCount,
                                                                                VkLayerProperties *pProperties) {
    // the layer command handles VK_NULL_HANDLE just fine internally
    assert(physicalDevice == VK_NULL_HANDLE);
    return parameter_validation::vkEnumerateDeviceLayerProperties(VK_NULL_HANDLE, pCount, pProperties);
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice,
                                                                                    const char *pLayerName, uint32_t *pCount,
                                                                                    VkExtensionProperties *pProperties) {
    // the layer command handles VK_NULL_HANDLE just fine internally
    assert(physicalDevice == VK_NULL_HANDLE);
    return parameter_validation::vkEnumerateDeviceExtensionProperties(VK_NULL_HANDLE, pLayerName, pCount, pProperties);
}

VK_LAYER_EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetDeviceProcAddr(VkDevice dev, const char *funcName) {
    return parameter_validation::vkGetDeviceProcAddr(dev, funcName);
}

VK_LAYER_EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetInstanceProcAddr(VkInstance instance, const char *funcName) {
    return parameter_validation::vkGetInstanceProcAddr(instance, funcName);
}

VK_LAYER_EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vk_layerGetPhysicalDeviceProcAddr(VkInstance instance,
                                                                                           const char *funcName) {
    return parameter_validation::vkGetPhysicalDeviceProcAddr(instance, funcName);
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL VKAPI_CALL
vkNegotiateLoaderLayerInterfaceVersion(VkNegotiateLayerInterface *pVersionStruct) {
    assert(pVersionStruct != NULL);
    assert(pVersionStruct->sType == LAYER_NEGOTIATE_INTERFACE_STRUCT);

    // Fill in the function pointers if our version is at least capable of having the structure contain them.
    if (pVersionStruct->loaderLayerInterfaceVersion >= 2) {
        pVersionStruct->pfnGetInstanceProcAddr = vkGetInstanceProcAddr;
        pVersionStruct->pfnGetDeviceProcAddr = vkGetDeviceProcAddr;
        pVersionStruct->pfnGetPhysicalDeviceProcAddr = vk_layerGetPhysicalDeviceProcAddr;
    }

    if (pVersionStruct->loaderLayerInterfaceVersion < CURRENT_LOADER_LAYER_INTERFACE_VERSION) {
        parameter_validation::loader_layer_if_version = pVersionStruct->loaderLayerInterfaceVersion;
    } else if (pVersionStruct->loaderLayerInterfaceVersion > CURRENT_LOADER_LAYER_INTERFACE_VERSION) {
        pVersionStruct->loaderLayerInterfaceVersion = CURRENT_LOADER_LAYER_INTERFACE_VERSION;
    }

    return VK_SUCCESS;
}
