
// This file is ***GENERATED***.  Do Not Edit.
// See layer_chassis_generator.py for modifications.

/* Copyright (c) 2015-2021 The Khronos Group Inc.
 * Copyright (c) 2015-2021 Valve Corporation
 * Copyright (c) 2015-2021 LunarG, Inc.
 * Copyright (c) 2015-2021 Google Inc.
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
 * Author: Mark Lobodzinski <mark@lunarg.com>
 * Author: Nadav Geva <nadav.geva@amd.com>
 */


#include <string.h>
#include <mutex>

#include "chassis.h"
#include "layer_options.h"
#include "layer_chassis_dispatch.h"

small_unordered_map<void*, ValidationObject*, 2> layer_data_map;

// Global unique object identifier.
std::atomic<uint64_t> global_unique_id(1ULL);
// Map uniqueID to actual object handle. Accesses to the map itself are
// internally synchronized.
vl_concurrent_unordered_map<uint64_t, uint64_t, 4, HashedUint64> unique_id_mapping;

bool wrap_handles = true;

#define OBJECT_LAYER_NAME "VK_LAYER_KHRONOS_validation"
#define OBJECT_LAYER_DESCRIPTION "khronos_validation"

// Include layer validation object definitions
#include "best_practices_validation.h"
#include "core_validation.h"
#include "corechecks_optick_instrumentation.h"
#include "gpu_validation.h"
#include "object_lifetime_validation.h"
#include "debug_printf.h"
#include "stateless_validation.h"
#include "synchronization_validation.h"
#include "thread_safety.h"

// This header file must be included after the above validation object class definitions
#include "chassis_dispatch_helper.h"

// Global list of sType,size identifiers
std::vector<std::pair<uint32_t, uint32_t>> custom_stype_info{};

#ifdef INSTRUMENT_OPTICK
static const bool use_optick_instrumentation = true;
#else
static const bool use_optick_instrumentation = false;
#endif

namespace vulkan_layer_chassis {

static const VkLayerProperties global_layer = {
    OBJECT_LAYER_NAME, VK_LAYER_API_VERSION, 1, "LunarG validation Layer",
};

static const VkExtensionProperties instance_extensions[] = {
#if !defined(VULKANSC)
{VK_EXT_DEBUG_REPORT_EXTENSION_NAME, VK_EXT_DEBUG_REPORT_SPEC_VERSION},
#endif
                                                            {VK_EXT_DEBUG_UTILS_EXTENSION_NAME, VK_EXT_DEBUG_UTILS_SPEC_VERSION},
                                                            {VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME, VK_EXT_VALIDATION_FEATURES_SPEC_VERSION}};
static const VkExtensionProperties device_extensions[] = {
#if !defined(VULKANSC)
    {VK_EXT_VALIDATION_CACHE_EXTENSION_NAME, VK_EXT_VALIDATION_CACHE_SPEC_VERSION},
    {VK_EXT_DEBUG_MARKER_EXTENSION_NAME, VK_EXT_DEBUG_MARKER_SPEC_VERSION},
    {VK_EXT_TOOLING_INFO_EXTENSION_NAME, VK_EXT_TOOLING_INFO_SPEC_VERSION}
#endif
};

typedef enum ApiFunctionType {
    kFuncTypeInst = 0,
    kFuncTypePdev = 1,
    kFuncTypeDev = 2
} ApiFunctionType;

typedef struct {
    ApiFunctionType function_type;
    void* funcptr;
} function_data;

extern const layer_data::unordered_map<std::string, function_data> name_to_funcptr_map;

// Manually written functions

// Check enabled instance extensions against supported instance extension whitelist
static void InstanceExtensionWhitelist(ValidationObject *layer_data, const VkInstanceCreateInfo *pCreateInfo, VkInstance instance) {
    for (uint32_t i = 0; i < pCreateInfo->enabledExtensionCount; i++) {
        // Check for recognized instance extensions
        if (!white_list(pCreateInfo->ppEnabledExtensionNames[i], kInstanceExtensionNames)) {
            layer_data->LogWarning(layer_data->instance, kVUIDUndefined,
                    "Instance Extension %s is not supported by this layer.  Using this extension may adversely affect validation "
                    "results and/or produce undefined behavior.",
                    pCreateInfo->ppEnabledExtensionNames[i]);
        }
    }
}

// Check enabled device extensions against supported device extension whitelist
static void DeviceExtensionWhitelist(ValidationObject *layer_data, const VkDeviceCreateInfo *pCreateInfo, VkDevice device) {
    for (uint32_t i = 0; i < pCreateInfo->enabledExtensionCount; i++) {
        // Check for recognized device extensions
        if (!white_list(pCreateInfo->ppEnabledExtensionNames[i], kDeviceExtensionNames)) {
            layer_data->LogWarning(layer_data->device, kVUIDUndefined,
                    "Device Extension %s is not supported by this layer.  Using this extension may adversely affect validation "
                    "results and/or produce undefined behavior.",
                    pCreateInfo->ppEnabledExtensionNames[i]);
        }
    }
}

static void DeviceExtensionWarnlist(ValidationObject *layer_data, const VkDeviceCreateInfo *pCreateInfo, VkDevice device);

void OutputLayerStatusInfo(ValidationObject *context) {
    std::string list_of_enables;
    std::string list_of_disables;
    for (uint32_t i = 0; i < kMaxEnableFlags; i++) {
        if (context->enabled[i]) {
            if (list_of_enables.size()) list_of_enables.append(", ");
            list_of_enables.append(EnableFlagNameHelper[i]);
        }
    }
    if (list_of_enables.size() == 0) {
        list_of_enables.append("None");
    }
    for (uint32_t i = 0; i < kMaxDisableFlags; i++) {
        if (context->disabled[i]) {
            if (list_of_disables.size()) list_of_disables.append(", ");
            list_of_disables.append(DisableFlagNameHelper[i]);
        }
    }
    if (list_of_disables.size() == 0) {
        list_of_disables.append("None");
    }

    auto settings_info = GetLayerSettingsFileInfo();
    std::string settings_status;
    if (!settings_info->file_found) {
        settings_status = "None. Default location is ";
        settings_status.append(settings_info->location);
        settings_status.append(".");
    } else {
        settings_status = "Found at ";
        settings_status.append(settings_info->location);
        settings_status.append(" specified by ");
        switch (settings_info->source) {
            case kEnvVar:
                settings_status.append("environment variable (VK_LAYER_SETTINGS_PATH).");
                break;
            case kVkConfig:
                settings_status.append("VkConfig application override.");
                break;
            case kLocal:    // Intentionally fall through
            default:
                settings_status.append("default location (current working directory).");
                break;
        }
    }

    // Output layer status information message
    context->LogInfo(context->instance, kVUID_Core_CreatInstance_Status,
        "Khronos Validation Layer Active:\n    Settings File: %s\n    Current Enables: %s.\n    Current Disables: %s.\n",
        settings_status.c_str(), list_of_enables.c_str(), list_of_disables.c_str());

    // Create warning message if user is running debug layers.
#ifndef NDEBUG
    context->LogPerformanceWarning(context->instance, kVUID_Core_CreateInstance_Debug_Warning,
        "VALIDATION LAYERS WARNING: Using debug builds of the validation layers *will* adversely affect performance.");
#endif
}

// Non-code-generated chassis API functions

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL GetDeviceProcAddr(VkDevice device, const char *funcName) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!ApiParentExtensionEnabled(funcName, &layer_data->device_extensions)) {
        return nullptr;
    }
    const auto &item = name_to_funcptr_map.find(funcName);
    if (item != name_to_funcptr_map.end()) {
        if (item->second.function_type != kFuncTypeDev) {
            return nullptr;
        } else {
            return reinterpret_cast<PFN_vkVoidFunction>(item->second.funcptr);
        }
    }
    auto &table = layer_data->device_dispatch_table;
    if (!table.GetDeviceProcAddr) return nullptr;
    return table.GetDeviceProcAddr(device, funcName);
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL GetInstanceProcAddr(VkInstance instance, const char *funcName) {
    const auto &item = name_to_funcptr_map.find(funcName);
    if (item != name_to_funcptr_map.end()) {
        return reinterpret_cast<PFN_vkVoidFunction>(item->second.funcptr);
    }
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    auto &table = layer_data->instance_dispatch_table;
    if (!table.GetInstanceProcAddr) return nullptr;
    return table.GetInstanceProcAddr(instance, funcName);
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL GetPhysicalDeviceProcAddr(VkInstance instance, const char *funcName) {
    const auto &item = name_to_funcptr_map.find(funcName);
    if (item != name_to_funcptr_map.end()) {
        if (item->second.function_type != kFuncTypePdev) {
            return nullptr;
        } else {
            return reinterpret_cast<PFN_vkVoidFunction>(item->second.funcptr);
        }
    }
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    auto &table = layer_data->instance_dispatch_table;
    if (!table.GetPhysicalDeviceProcAddr) return nullptr;
    return table.GetPhysicalDeviceProcAddr(instance, funcName);
}

VKAPI_ATTR VkResult VKAPI_CALL EnumerateInstanceLayerProperties(uint32_t *pCount, VkLayerProperties *pProperties) {
    return util_GetLayerProperties(1, &global_layer, pCount, pProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL EnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice, uint32_t *pCount,
                                                              VkLayerProperties *pProperties) {
    return util_GetLayerProperties(1, &global_layer, pCount, pProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL EnumerateInstanceExtensionProperties(const char *pLayerName, uint32_t *pCount,
                                                                    VkExtensionProperties *pProperties) {
    if (pLayerName && !strcmp(pLayerName, global_layer.layerName))
        return util_GetExtensionProperties(ARRAY_SIZE(instance_extensions), instance_extensions, pCount, pProperties);

    return VK_ERROR_LAYER_NOT_PRESENT;
}

VKAPI_ATTR VkResult VKAPI_CALL EnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char *pLayerName,
                                                                  uint32_t *pCount, VkExtensionProperties *pProperties) {
    if (pLayerName && !strcmp(pLayerName, global_layer.layerName)) return util_GetExtensionProperties(ARRAY_SIZE(device_extensions), device_extensions, pCount, pProperties);
    assert(physicalDevice);
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    return layer_data->instance_dispatch_table.EnumerateDeviceExtensionProperties(physicalDevice, pLayerName, pCount, pProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL CreateInstance(const VkInstanceCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator,
                                              VkInstance *pInstance) {
    VkLayerInstanceCreateInfo* chain_info = get_chain_info(pCreateInfo, VK_LAYER_LINK_INFO);

    assert(chain_info->u.pLayerInfo);
    PFN_vkGetInstanceProcAddr fpGetInstanceProcAddr = chain_info->u.pLayerInfo->pfnNextGetInstanceProcAddr;
    PFN_vkCreateInstance fpCreateInstance = (PFN_vkCreateInstance)fpGetInstanceProcAddr(NULL, "vkCreateInstance");
    if (fpCreateInstance == NULL) return VK_ERROR_INITIALIZATION_FAILED;
    chain_info->u.pLayerInfo = chain_info->u.pLayerInfo->pNext;
    uint32_t specified_version = (pCreateInfo->pApplicationInfo ? pCreateInfo->pApplicationInfo->apiVersion : VK_API_VERSION_1_0);
    uint32_t api_version;
    if (specified_version < VK_API_VERSION_1_1)
        api_version = VK_API_VERSION_1_0;
    else if (specified_version < VK_API_VERSION_1_2)
        api_version = VK_API_VERSION_1_1;
    else
        api_version = VK_API_VERSION_1_2;
    auto report_data = new debug_report_data{};
    report_data->instance_pnext_chain = SafePnextCopy(pCreateInfo->pNext);
    ActivateInstanceDebugCallbacks(report_data);

    // Set up enable and disable features flags
    CHECK_ENABLED local_enables {};
    CHECK_DISABLED local_disables {};
    ConfigAndEnvSettings config_and_env_settings_data {OBJECT_LAYER_DESCRIPTION, pCreateInfo->pNext, local_enables, local_disables,
        report_data->filter_message_ids, &report_data->duplicate_message_limit};
    ProcessConfigAndEnvSettings(&config_and_env_settings_data);
    layer_debug_messenger_actions(report_data, pAllocator, OBJECT_LAYER_DESCRIPTION);

    // Create temporary dispatch vector for pre-calls until instance is created
    std::vector<ValidationObject*> local_object_dispatch;

#if !defined(VULKANSC)
    // Add VOs to dispatch vector. Order here will be the validation dispatch order!
    auto thread_checker_obj = new ThreadSafety(nullptr);
    thread_checker_obj->RegisterValidationObject(!local_disables[thread_safety], api_version, report_data, local_object_dispatch);
#endif

    auto parameter_validation_obj = new StatelessValidation;
    parameter_validation_obj->RegisterValidationObject(!local_disables[stateless_checks], api_version, report_data, local_object_dispatch);

#if !defined(VULKANSC)
    auto object_tracker_obj = new ObjectLifetimes;
    object_tracker_obj->RegisterValidationObject(!local_disables[object_tracking], api_version, report_data, local_object_dispatch);

    auto core_checks_obj = use_optick_instrumentation ? new CoreChecksOptickInstrumented : new CoreChecks;
    core_checks_obj->RegisterValidationObject(!local_disables[core_checks], api_version, report_data, local_object_dispatch);

    auto best_practices_obj = new BestPractices;
    best_practices_obj->RegisterValidationObject(local_enables[best_practices], api_version, report_data, local_object_dispatch);

    auto gpu_assisted_obj = new GpuAssisted;
    gpu_assisted_obj->RegisterValidationObject(local_enables[gpu_validation], api_version, report_data, local_object_dispatch);

    auto debug_printf_obj = new DebugPrintf;
    debug_printf_obj->RegisterValidationObject(local_enables[debug_printf], api_version, report_data, local_object_dispatch);

    auto sync_validation_obj = new SyncValidator;
    sync_validation_obj->RegisterValidationObject(local_enables[sync_validation], api_version, report_data, local_object_dispatch);
#endif

    // If handle wrapping is disabled via the ValidationFeatures extension, override build flag
    if (local_disables[handle_wrapping]) {
        wrap_handles = false;
    }

    // Init dispatch array and call registration functions
    bool skip = false;
    for (auto intercept : local_object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateInstance(pCreateInfo, pAllocator, pInstance);
        if (skip)
#if defined(VK_EXT_debug_report)
            return VK_ERROR_VALIDATION_FAILED_EXT;
#else
            return VK_ERROR_INITIALIZATION_FAILED;
#endif
    }
    for (auto intercept : local_object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateInstance(pCreateInfo, pAllocator, pInstance);
    }

    VkResult result = fpCreateInstance(pCreateInfo, pAllocator, pInstance);
    if (result != VK_SUCCESS) return result;

    auto framework = GetLayerDataPtr(get_dispatch_key(*pInstance), layer_data_map);

    framework->object_dispatch = local_object_dispatch;
    framework->container_type = LayerObjectTypeInstance;
    framework->disabled = local_disables;
    framework->enabled = local_enables;

    framework->instance = *pInstance;
    layer_init_instance_dispatch_table(*pInstance, &framework->instance_dispatch_table, fpGetInstanceProcAddr);
    framework->report_data = report_data;
    framework->api_version = api_version;
    framework->instance_extensions.InitFromInstanceCreateInfo(specified_version, pCreateInfo);

    OutputLayerStatusInfo(framework);

#if !defined(VULKANSC)
    thread_checker_obj->FinalizeInstanceValidationObject(framework, *pInstance);
    object_tracker_obj->FinalizeInstanceValidationObject(framework, *pInstance);
#endif
    parameter_validation_obj->FinalizeInstanceValidationObject(framework, *pInstance);
#if !defined(VULKANSC)
    core_checks_obj->FinalizeInstanceValidationObject(framework, *pInstance);
    best_practices_obj->FinalizeInstanceValidationObject(framework, *pInstance);
    gpu_assisted_obj->FinalizeInstanceValidationObject(framework, *pInstance);
    debug_printf_obj->FinalizeInstanceValidationObject(framework, *pInstance);
    sync_validation_obj->FinalizeInstanceValidationObject(framework, *pInstance);
#endif

    for (auto intercept : framework->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCreateInstance(pCreateInfo, pAllocator, pInstance, result);
    }

    // Delete unused validation objects to avoid memory leak.
    std::vector<ValidationObject*> local_objs = {
#if !defined(VULKANSC)
        thread_checker_obj, object_tracker_obj,
#endif
        parameter_validation_obj,
#if !defined(VULKANSC)
        core_checks_obj, best_practices_obj, gpu_assisted_obj, debug_printf_obj,
        sync_validation_obj,
#endif
    };
    for (auto obj : local_objs) {
        if (std::find(local_object_dispatch.begin(), local_object_dispatch.end(), obj) == local_object_dispatch.end()) {
            delete obj;
        }
    }

    InstanceExtensionWhitelist(framework, pCreateInfo, *pInstance);
    DeactivateInstanceDebugCallbacks(report_data);
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyInstance(VkInstance instance, const VkAllocationCallbacks *pAllocator) {
    dispatch_key key = get_dispatch_key(instance);
    auto layer_data = GetLayerDataPtr(key, layer_data_map);
    ActivateInstanceDebugCallbacks(layer_data->report_data);

    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        (const_cast<const ValidationObject*>(intercept))->PreCallValidateDestroyInstance(instance, pAllocator);
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDestroyInstance(instance, pAllocator);
    }

    layer_data->instance_dispatch_table.DestroyInstance(instance, pAllocator);

    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordDestroyInstance(instance, pAllocator);
    }

    DeactivateInstanceDebugCallbacks(layer_data->report_data);
    FreePnextChain(layer_data->report_data->instance_pnext_chain);

    layer_debug_utils_destroy_instance(layer_data->report_data);

    for (auto item = layer_data->object_dispatch.begin(); item != layer_data->object_dispatch.end(); item++) {
        delete *item;
    }
    FreeLayerDataPtr(key, layer_data_map);
}

VKAPI_ATTR VkResult VKAPI_CALL CreateDevice(VkPhysicalDevice gpu, const VkDeviceCreateInfo *pCreateInfo,
                                            const VkAllocationCallbacks *pAllocator, VkDevice *pDevice) {
    VkLayerDeviceCreateInfo *chain_info = get_chain_info(pCreateInfo, VK_LAYER_LINK_INFO);

    auto instance_interceptor = GetLayerDataPtr(get_dispatch_key(gpu), layer_data_map);

    PFN_vkGetInstanceProcAddr fpGetInstanceProcAddr = chain_info->u.pLayerInfo->pfnNextGetInstanceProcAddr;
    PFN_vkGetDeviceProcAddr fpGetDeviceProcAddr = chain_info->u.pLayerInfo->pfnNextGetDeviceProcAddr;
    PFN_vkCreateDevice fpCreateDevice = (PFN_vkCreateDevice)fpGetInstanceProcAddr(instance_interceptor->instance, "vkCreateDevice");
    if (fpCreateDevice == NULL) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    chain_info->u.pLayerInfo = chain_info->u.pLayerInfo->pNext;

    // Get physical device limits for device
    VkPhysicalDeviceProperties device_properties = {};
    instance_interceptor->instance_dispatch_table.GetPhysicalDeviceProperties(gpu, &device_properties);

    // Setup the validation tables based on the application API version from the instance and the capabilities of the device driver
    uint32_t effective_api_version = std::min(device_properties.apiVersion, instance_interceptor->api_version);

    DeviceExtensions device_extensions = {};
    device_extensions.InitFromDeviceCreateInfo(&instance_interceptor->instance_extensions, effective_api_version, pCreateInfo);
    for (auto item : instance_interceptor->object_dispatch) {
        item->device_extensions = device_extensions;
    }

    safe_VkDeviceCreateInfo modified_create_info(pCreateInfo);

    bool skip = false;
    for (auto intercept : instance_interceptor->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateDevice(gpu, pCreateInfo, pAllocator, pDevice);
        if (skip)
#if defined(VK_EXT_debug_report)
            return VK_ERROR_VALIDATION_FAILED_EXT;
#else
            return VK_ERROR_INITIALIZATION_FAILED;
#endif
    }
    for (auto intercept : instance_interceptor->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateDevice(gpu, pCreateInfo, pAllocator, pDevice, &modified_create_info);
    }

    VkResult result = fpCreateDevice(gpu, reinterpret_cast<VkDeviceCreateInfo *>(&modified_create_info), pAllocator, pDevice);
    if (result != VK_SUCCESS) {
        return result;
    }

    auto device_interceptor = GetLayerDataPtr(get_dispatch_key(*pDevice), layer_data_map);
    device_interceptor->container_type = LayerObjectTypeDevice;

    // Save local info in device object
    device_interceptor->phys_dev_properties.properties = device_properties;
    device_interceptor->api_version = device_interceptor->device_extensions.InitFromDeviceCreateInfo(
        &instance_interceptor->instance_extensions, effective_api_version, pCreateInfo);
    device_interceptor->device_extensions = device_extensions;

    layer_init_device_dispatch_table(*pDevice, &device_interceptor->device_dispatch_table, fpGetDeviceProcAddr);

    device_interceptor->device = *pDevice;
    device_interceptor->physical_device = gpu;
    device_interceptor->instance = instance_interceptor->instance;
    device_interceptor->report_data = instance_interceptor->report_data;

    // Note that this DEFINES THE ORDER IN WHICH THE LAYER VALIDATION OBJECTS ARE CALLED
    auto disables = instance_interceptor->disabled;
    auto enables = instance_interceptor->enabled;

#if !defined(VULKANSC)
    auto thread_safety_obj = new ThreadSafety(reinterpret_cast<ThreadSafety *>(instance_interceptor->GetValidationObject(instance_interceptor->object_dispatch, LayerObjectTypeThreading)));
    thread_safety_obj->InitDeviceValidationObject(!disables[thread_safety], instance_interceptor, device_interceptor);
#endif

    auto stateless_validation_obj = new StatelessValidation;
    stateless_validation_obj->InitDeviceValidationObject(!disables[stateless_checks], instance_interceptor, device_interceptor);

#if !defined(VULKANSC)
    auto object_tracker_obj = new ObjectLifetimes;
    object_tracker_obj->InitDeviceValidationObject(!disables[object_tracking], instance_interceptor, device_interceptor);

    auto core_checks_obj = use_optick_instrumentation ? new CoreChecksOptickInstrumented : new CoreChecks;
    core_checks_obj->InitDeviceValidationObject(!disables[core_checks], instance_interceptor, device_interceptor);

    auto best_practices_obj = new BestPractices;
    best_practices_obj->InitDeviceValidationObject(enables[best_practices], instance_interceptor, device_interceptor);

    auto gpu_assisted_obj = new GpuAssisted;
    gpu_assisted_obj->InitDeviceValidationObject(enables[gpu_validation], instance_interceptor, device_interceptor);

    auto debug_printf_obj = new DebugPrintf;
    debug_printf_obj->InitDeviceValidationObject(enables[debug_printf], instance_interceptor, device_interceptor);

    auto sync_validation_obj = new SyncValidator;
    sync_validation_obj->InitDeviceValidationObject(enables[sync_validation], instance_interceptor, device_interceptor);
#endif

    // Delete unused validation objects to avoid memory leak.
    std::vector<ValidationObject *> local_objs = {
#if !defined(VULKANSC)
        thread_safety_obj,
#endif
        stateless_validation_obj,
#if !defined(VULKANSC)
        object_tracker_obj,
        core_checks_obj, best_practices_obj, gpu_assisted_obj, debug_printf_obj,
        sync_validation_obj,
#endif
    };
    for (auto obj : local_objs) {
        if (std::find(device_interceptor->object_dispatch.begin(), device_interceptor->object_dispatch.end(), obj) ==
            device_interceptor->object_dispatch.end()) {
            delete obj;
        }
    }

    for (auto intercept : instance_interceptor->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCreateDevice(gpu, pCreateInfo, pAllocator, pDevice, result);
    }

    device_interceptor->InitObjectDispatchVectors();

    DeviceExtensionWhitelist(device_interceptor, pCreateInfo, *pDevice);
    DeviceExtensionWarnlist(device_interceptor, pCreateInfo, *pDevice);

    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyDevice(VkDevice device, const VkAllocationCallbacks *pAllocator) {
    dispatch_key key = get_dispatch_key(device);
    auto layer_data = GetLayerDataPtr(key, layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateDestroyDevice(device, pAllocator);
        if (skip) return;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDestroyDevice(device, pAllocator);
    }

    layer_data->device_dispatch_table.DestroyDevice(device, pAllocator);

    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordDestroyDevice(device, pAllocator);
    }

    for (auto item = layer_data->object_dispatch.begin(); item != layer_data->object_dispatch.end(); item++) {
        delete *item;
    }
    FreeLayerDataPtr(key, layer_data_map);
}


// Special-case APIs for which core_validation needs custom parameter lists and/or modifies parameters

VKAPI_ATTR VkResult VKAPI_CALL CreateGraphicsPipelines(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkGraphicsPipelineCreateInfo*         pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;

    create_graphics_pipeline_api_state cgpl_state[LayerObjectTypeMaxEnum]{};

    for (auto intercept : layer_data->object_dispatch) {
        cgpl_state[intercept->container_type].pCreateInfos = pCreateInfos;
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateGraphicsPipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, &(cgpl_state[intercept->container_type]));
        if (skip)
#if defined(VK_EXT_debug_report)
            return VK_ERROR_VALIDATION_FAILED_EXT;
#else
            return VK_ERROR_INITIALIZATION_FAILED;
#endif
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateGraphicsPipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, &(cgpl_state[intercept->container_type]));
    }

    auto usepCreateInfos = (!cgpl_state[LayerObjectTypeGpuAssisted].pCreateInfos) ? pCreateInfos : cgpl_state[LayerObjectTypeGpuAssisted].pCreateInfos;
    if (cgpl_state[LayerObjectTypeDebugPrintf].pCreateInfos) usepCreateInfos = cgpl_state[LayerObjectTypeDebugPrintf].pCreateInfos;

    VkResult result = DispatchCreateGraphicsPipelines(device, pipelineCache, createInfoCount, usepCreateInfos, pAllocator, pPipelines);

    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCreateGraphicsPipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, result, &(cgpl_state[intercept->container_type]));
    }
    return result;
}

// This API saves some core_validation pipeline state state on the stack for performance purposes
VKAPI_ATTR VkResult VKAPI_CALL CreateComputePipelines(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkComputePipelineCreateInfo*          pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;

    create_compute_pipeline_api_state ccpl_state[LayerObjectTypeMaxEnum]{};

    for (auto intercept : layer_data->object_dispatch) {
        ccpl_state[intercept->container_type].pCreateInfos = pCreateInfos;
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateComputePipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, &(ccpl_state[intercept->container_type]));
        if (skip)
#if defined(VK_EXT_debug_report)
            return VK_ERROR_VALIDATION_FAILED_EXT;
#else
            return VK_ERROR_INITIALIZATION_FAILED;
#endif
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateComputePipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, &(ccpl_state[intercept->container_type]));
    }

    auto usepCreateInfos = (!ccpl_state[LayerObjectTypeGpuAssisted].pCreateInfos) ? pCreateInfos : ccpl_state[LayerObjectTypeGpuAssisted].pCreateInfos;
    if (ccpl_state[LayerObjectTypeDebugPrintf].pCreateInfos) usepCreateInfos = ccpl_state[LayerObjectTypeDebugPrintf].pCreateInfos;

    VkResult result = DispatchCreateComputePipelines(device, pipelineCache, createInfoCount, usepCreateInfos, pAllocator, pPipelines);

    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCreateComputePipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, result, &(ccpl_state[intercept->container_type]));
    }
    return result;
}

#if defined(VK_NV_ray_tracing)
VKAPI_ATTR VkResult VKAPI_CALL CreateRayTracingPipelinesNV(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkRayTracingPipelineCreateInfoNV*     pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;

    create_ray_tracing_pipeline_api_state crtpl_state[LayerObjectTypeMaxEnum]{};

    for (auto intercept : layer_data->object_dispatch) {
        crtpl_state[intercept->container_type].pCreateInfos = pCreateInfos;
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateRayTracingPipelinesNV(device, pipelineCache, createInfoCount, pCreateInfos,
                                                                      pAllocator, pPipelines, &(crtpl_state[intercept->container_type]));
        if (skip)
#if defined(VK_EXT_debug_report)
            return VK_ERROR_VALIDATION_FAILED_EXT;
#else
            return VK_ERROR_INITIALIZATION_FAILED;
#endif
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateRayTracingPipelinesNV(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator,
                                                            pPipelines, &(crtpl_state[intercept->container_type]));
    }

    VkResult result = DispatchCreateRayTracingPipelinesNV(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);

    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCreateRayTracingPipelinesNV(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator,
                                                             pPipelines, result, &(crtpl_state[intercept->container_type]));
    }
    return result;
}
#endif

#if defined(VK_KHR_ray_tracing_pipeline)
VKAPI_ATTR VkResult VKAPI_CALL CreateRayTracingPipelinesKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkRayTracingPipelineCreateInfoKHR*    pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;

    create_ray_tracing_pipeline_khr_api_state crtpl_state[LayerObjectTypeMaxEnum]{};

    for (auto intercept : layer_data->object_dispatch) {
        crtpl_state[intercept->container_type].pCreateInfos = pCreateInfos;
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateRayTracingPipelinesKHR(device, deferredOperation, pipelineCache, createInfoCount, pCreateInfos,
                                                                      pAllocator, pPipelines, &(crtpl_state[intercept->container_type]));
        if (skip)
#if defined(VK_EXT_debug_report)
            return VK_ERROR_VALIDATION_FAILED_EXT;
#else
            return VK_ERROR_INITIALIZATION_FAILED;
#endif
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateRayTracingPipelinesKHR(device, deferredOperation, pipelineCache, createInfoCount, pCreateInfos, pAllocator,
                                                            pPipelines, &(crtpl_state[intercept->container_type]));
    }

    VkResult result = DispatchCreateRayTracingPipelinesKHR(device, deferredOperation, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);

    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCreateRayTracingPipelinesKHR(device, deferredOperation, pipelineCache, createInfoCount, pCreateInfos, pAllocator,
                                                             pPipelines, result, &(crtpl_state[intercept->container_type]));
    }
    return result;
}
#endif

// This API needs the ability to modify a down-chain parameter
VKAPI_ATTR VkResult VKAPI_CALL CreatePipelineLayout(
    VkDevice                                    device,
    const VkPipelineLayoutCreateInfo*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkPipelineLayout*                           pPipelineLayout) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;

    create_pipeline_layout_api_state cpl_state{};
    cpl_state.modified_create_info = *pCreateInfo;

    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout);
        if (skip)
#if defined(VK_EXT_debug_report)
            return VK_ERROR_VALIDATION_FAILED_EXT;
#else
            return VK_ERROR_INITIALIZATION_FAILED;
#endif
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout, &cpl_state);
    }
    VkResult result = DispatchCreatePipelineLayout(device, &cpl_state.modified_create_info, pAllocator, pPipelineLayout);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout, result);
    }
    return result;
}

#if !defined(VULKANSC)
// This API needs some local stack data for performance reasons and also may modify a parameter
VKAPI_ATTR VkResult VKAPI_CALL CreateShaderModule(
    VkDevice                                    device,
    const VkShaderModuleCreateInfo*             pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkShaderModule*                             pShaderModule) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;

    create_shader_module_api_state csm_state{};
    csm_state.instrumented_create_info = *pCreateInfo;

    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateShaderModule(device, pCreateInfo, pAllocator, pShaderModule, &csm_state);
        if (skip)
#if defined(VK_EXT_debug_report)
            return VK_ERROR_VALIDATION_FAILED_EXT;
#else
            return VK_ERROR_INITIALIZATION_FAILED;
#endif
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateShaderModule(device, pCreateInfo, pAllocator, pShaderModule, &csm_state);
    }
    VkResult result = DispatchCreateShaderModule(device, &csm_state.instrumented_create_info, pAllocator, pShaderModule);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCreateShaderModule(device, pCreateInfo, pAllocator, pShaderModule, result, &csm_state);
    }
    return result;
}
#endif

VKAPI_ATTR VkResult VKAPI_CALL AllocateDescriptorSets(
    VkDevice                                    device,
    const VkDescriptorSetAllocateInfo*          pAllocateInfo,
    VkDescriptorSet*                            pDescriptorSets) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;

#if !defined(VULKANSC)
    cvdescriptorset::AllocateDescriptorSetsData ads_state[LayerObjectTypeMaxEnum];

    for (auto intercept : layer_data->object_dispatch) {
        ads_state[intercept->container_type].Init(pAllocateInfo->descriptorSetCount);
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateAllocateDescriptorSets(device,
            pAllocateInfo, pDescriptorSets, &(ads_state[intercept->container_type]));
        if (skip)
#if defined(VK_EXT_debug_report)
            return VK_ERROR_VALIDATION_FAILED_EXT;
#else
            return VK_ERROR_INITIALIZATION_FAILED;
#endif
    }
#endif
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordAllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets);
    }
#if !defined(VULKANSC)
    VkResult result = DispatchAllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordAllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets,
            result, &(ads_state[intercept->container_type]));
    }
#endif
#if defined(VULKANSC)
    VkResult result = VK_SUCCESS;
#endif
    return result;
}

// This API needs the ability to modify a down-chain parameter
VKAPI_ATTR VkResult VKAPI_CALL CreateBuffer(
    VkDevice                                    device,
    const VkBufferCreateInfo*                   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkBuffer*                                   pBuffer) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;

    create_buffer_api_state cb_state{};
    cb_state.modified_create_info = *pCreateInfo;

    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateBuffer(device, pCreateInfo, pAllocator, pBuffer);
        if (skip)
#if defined(VK_EXT_debug_report)
            return VK_ERROR_VALIDATION_FAILED_EXT;
#else
            return VK_ERROR_INITIALIZATION_FAILED;
#endif
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateBuffer(device, pCreateInfo, pAllocator, pBuffer, &cb_state);
    }
    VkResult result = DispatchCreateBuffer(device, &cb_state.modified_create_info, pAllocator, pBuffer);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCreateBuffer(device, pCreateInfo, pAllocator, pBuffer, result);
    }
    return result;
}

#if defined(VK_EXT_tooling_info)
// Handle tooling queries manually as this is a request for layer information

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceToolPropertiesEXT(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pToolCount,
    VkPhysicalDeviceToolPropertiesEXT*          pToolProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;

    static const VkPhysicalDeviceToolPropertiesEXT khronos_layer_tool_props = {
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TOOL_PROPERTIES_EXT,
        nullptr,
        "Khronos Validation Layer",
        STRINGIFY(VK_HEADER_VERSION),
        VK_TOOL_PURPOSE_VALIDATION_BIT_EXT | VK_TOOL_PURPOSE_ADDITIONAL_FEATURES_BIT_EXT | VK_TOOL_PURPOSE_DEBUG_REPORTING_BIT_EXT | VK_TOOL_PURPOSE_DEBUG_MARKERS_BIT_EXT,
        "Khronos Validation Layer",
        OBJECT_LAYER_NAME
    };

    auto original_pToolProperties = pToolProperties;


    if (pToolProperties != nullptr) {
        *pToolProperties = khronos_layer_tool_props;
        pToolProperties = ((*pToolCount > 1) ? &pToolProperties[1] : nullptr);
        (*pToolCount)--;
    }

    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceToolPropertiesEXT(physicalDevice, pToolCount, pToolProperties);
        if (skip)
#if defined(VK_EXT_debug_report)
            return VK_ERROR_VALIDATION_FAILED_EXT;
#else
            return VK_ERROR_INITIALIZATION_FAILED;
#endif
    }

    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceToolPropertiesEXT(physicalDevice, pToolCount, pToolProperties);
    }

    VkResult result = DispatchGetPhysicalDeviceToolPropertiesEXT(physicalDevice, pToolCount, pToolProperties);

    if (original_pToolProperties != nullptr) {
        pToolProperties = original_pToolProperties;
    }
    (*pToolCount)++;

    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetPhysicalDeviceToolPropertiesEXT(physicalDevice, pToolCount, pToolProperties, result);
    }
    return result;
}
#endif

#if defined(VK_EXT_validation_cache)
// ValidationCache APIs do not dispatch

VKAPI_ATTR VkResult VKAPI_CALL CreateValidationCacheEXT(
    VkDevice                                    device,
    const VkValidationCacheCreateInfoEXT*       pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkValidationCacheEXT*                       pValidationCache) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    VkResult result = VK_SUCCESS;

    ValidationObject *validation_data = layer_data->GetValidationObject(layer_data->object_dispatch, LayerObjectTypeCoreValidation);
    if (validation_data) {
        auto lock = validation_data->WriteLock();
        result = validation_data->CoreLayerCreateValidationCacheEXT(device, pCreateInfo, pAllocator, pValidationCache);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyValidationCacheEXT(
    VkDevice                                    device,
    VkValidationCacheEXT                        validationCache,
    const VkAllocationCallbacks*                pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);

    ValidationObject *validation_data = layer_data->GetValidationObject(layer_data->object_dispatch, LayerObjectTypeCoreValidation);
    if (validation_data) {
        auto lock = validation_data->WriteLock();
        validation_data->CoreLayerDestroyValidationCacheEXT(device, validationCache, pAllocator);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL MergeValidationCachesEXT(
    VkDevice                                    device,
    VkValidationCacheEXT                        dstCache,
    uint32_t                                    srcCacheCount,
    const VkValidationCacheEXT*                 pSrcCaches) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    VkResult result = VK_SUCCESS;

    ValidationObject *validation_data = layer_data->GetValidationObject(layer_data->object_dispatch, LayerObjectTypeCoreValidation);
    if (validation_data) {
        auto lock = validation_data->WriteLock();
        result = validation_data->CoreLayerMergeValidationCachesEXT(device, dstCache, srcCacheCount, pSrcCaches);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetValidationCacheDataEXT(
    VkDevice                                    device,
    VkValidationCacheEXT                        validationCache,
    size_t*                                     pDataSize,
    void*                                       pData) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    VkResult result = VK_SUCCESS;

    ValidationObject *validation_data = layer_data->GetValidationObject(layer_data->object_dispatch, LayerObjectTypeCoreValidation);
    if (validation_data) {
        auto lock = validation_data->WriteLock();
        result = validation_data->CoreLayerGetValidationCacheDataEXT(device, validationCache, pDataSize, pData);
    }
    return result;

}
#endif
static const std::set<std::string> kDeviceWarnExtensionNames {
    "VK_KHR_dynamic_rendering",
};

static void DeviceExtensionWarnlist(ValidationObject *layer_data, const VkDeviceCreateInfo *pCreateInfo, VkDevice device) {
    for (uint32_t i = 0; i < pCreateInfo->enabledExtensionCount; i++) {
        // Check for recognized device extensions
        if (white_list(pCreateInfo->ppEnabledExtensionNames[i], kDeviceWarnExtensionNames)) {
            layer_data->LogWarning(layer_data->device, kVUIDUndefined,
                    "Device Extension %s support is incomplete, incorrect results are possible.",
                    pCreateInfo->ppEnabledExtensionNames[i]);
        }
    }
}



VKAPI_ATTR VkResult VKAPI_CALL EnumeratePhysicalDevices(
    VkInstance                                  instance,
    uint32_t*                                   pPhysicalDeviceCount,
    VkPhysicalDevice*                           pPhysicalDevices) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateEnumeratePhysicalDevices(instance, pPhysicalDeviceCount, pPhysicalDevices);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordEnumeratePhysicalDevices(instance, pPhysicalDeviceCount, pPhysicalDevices);
    }
    VkResult result = DispatchEnumeratePhysicalDevices(instance, pPhysicalDeviceCount, pPhysicalDevices);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordEnumeratePhysicalDevices(instance, pPhysicalDeviceCount, pPhysicalDevices, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceFeatures(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceFeatures*                   pFeatures) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceFeatures(physicalDevice, pFeatures);
        if (skip) return;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceFeatures(physicalDevice, pFeatures);
    }
    DispatchGetPhysicalDeviceFeatures(physicalDevice, pFeatures);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetPhysicalDeviceFeatures(physicalDevice, pFeatures);
    }
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceFormatProperties(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkFormatProperties*                         pFormatProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceFormatProperties(physicalDevice, format, pFormatProperties);
        if (skip) return;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceFormatProperties(physicalDevice, format, pFormatProperties);
    }
    DispatchGetPhysicalDeviceFormatProperties(physicalDevice, format, pFormatProperties);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetPhysicalDeviceFormatProperties(physicalDevice, format, pFormatProperties);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceImageFormatProperties(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkImageType                                 type,
    VkImageTiling                               tiling,
    VkImageUsageFlags                           usage,
    VkImageCreateFlags                          flags,
    VkImageFormatProperties*                    pImageFormatProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceImageFormatProperties(physicalDevice, format, type, tiling, usage, flags, pImageFormatProperties);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceImageFormatProperties(physicalDevice, format, type, tiling, usage, flags, pImageFormatProperties);
    }
    VkResult result = DispatchGetPhysicalDeviceImageFormatProperties(physicalDevice, format, type, tiling, usage, flags, pImageFormatProperties);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetPhysicalDeviceImageFormatProperties(physicalDevice, format, type, tiling, usage, flags, pImageFormatProperties, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceProperties(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceProperties*                 pProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceProperties(physicalDevice, pProperties);
        if (skip) return;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceProperties(physicalDevice, pProperties);
    }
    DispatchGetPhysicalDeviceProperties(physicalDevice, pProperties);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetPhysicalDeviceProperties(physicalDevice, pProperties);
    }
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceQueueFamilyProperties(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pQueueFamilyPropertyCount,
    VkQueueFamilyProperties*                    pQueueFamilyProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceQueueFamilyProperties(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
        if (skip) return;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceQueueFamilyProperties(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
    }
    DispatchGetPhysicalDeviceQueueFamilyProperties(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetPhysicalDeviceQueueFamilyProperties(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
    }
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceMemoryProperties(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceMemoryProperties*           pMemoryProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceMemoryProperties(physicalDevice, pMemoryProperties);
        if (skip) return;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceMemoryProperties(physicalDevice, pMemoryProperties);
    }
    DispatchGetPhysicalDeviceMemoryProperties(physicalDevice, pMemoryProperties);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetPhysicalDeviceMemoryProperties(physicalDevice, pMemoryProperties);
    }
}

VKAPI_ATTR void VKAPI_CALL GetDeviceQueue(
    VkDevice                                    device,
    uint32_t                                    queueFamilyIndex,
    uint32_t                                    queueIndex,
    VkQueue*                                    pQueue) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetDeviceQueue]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetDeviceQueue(device, queueFamilyIndex, queueIndex, pQueue);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetDeviceQueue]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetDeviceQueue(device, queueFamilyIndex, queueIndex, pQueue);
    }
    DispatchGetDeviceQueue(device, queueFamilyIndex, queueIndex, pQueue);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetDeviceQueue]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetDeviceQueue(device, queueFamilyIndex, queueIndex, pQueue);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL QueueSubmit(
    VkQueue                                     queue,
    uint32_t                                    submitCount,
    const VkSubmitInfo*                         pSubmits,
    VkFence                                     fence) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(queue), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateQueueSubmit]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateQueueSubmit(queue, submitCount, pSubmits, fence);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordQueueSubmit]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordQueueSubmit(queue, submitCount, pSubmits, fence);
    }
    VkResult result = DispatchQueueSubmit(queue, submitCount, pSubmits, fence);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordQueueSubmit]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordQueueSubmit(queue, submitCount, pSubmits, fence, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL QueueWaitIdle(
    VkQueue                                     queue) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(queue), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateQueueWaitIdle]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateQueueWaitIdle(queue);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordQueueWaitIdle]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordQueueWaitIdle(queue);
    }
    VkResult result = DispatchQueueWaitIdle(queue);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordQueueWaitIdle]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordQueueWaitIdle(queue, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL DeviceWaitIdle(
    VkDevice                                    device) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDeviceWaitIdle]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateDeviceWaitIdle(device);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDeviceWaitIdle]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDeviceWaitIdle(device);
    }
    VkResult result = DispatchDeviceWaitIdle(device);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDeviceWaitIdle]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordDeviceWaitIdle(device, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL AllocateMemory(
    VkDevice                                    device,
    const VkMemoryAllocateInfo*                 pAllocateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDeviceMemory*                             pMemory) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateAllocateMemory]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateAllocateMemory(device, pAllocateInfo, pAllocator, pMemory);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordAllocateMemory]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordAllocateMemory(device, pAllocateInfo, pAllocator, pMemory);
    }
    VkResult result = DispatchAllocateMemory(device, pAllocateInfo, pAllocator, pMemory);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordAllocateMemory]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordAllocateMemory(device, pAllocateInfo, pAllocator, pMemory, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL MapMemory(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    VkDeviceSize                                offset,
    VkDeviceSize                                size,
    VkMemoryMapFlags                            flags,
    void**                                      ppData) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateMapMemory]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateMapMemory(device, memory, offset, size, flags, ppData);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordMapMemory]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordMapMemory(device, memory, offset, size, flags, ppData);
    }
    VkResult result = DispatchMapMemory(device, memory, offset, size, flags, ppData);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordMapMemory]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordMapMemory(device, memory, offset, size, flags, ppData, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL UnmapMemory(
    VkDevice                                    device,
    VkDeviceMemory                              memory) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateUnmapMemory]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateUnmapMemory(device, memory);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordUnmapMemory]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordUnmapMemory(device, memory);
    }
    DispatchUnmapMemory(device, memory);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordUnmapMemory]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordUnmapMemory(device, memory);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL FlushMappedMemoryRanges(
    VkDevice                                    device,
    uint32_t                                    memoryRangeCount,
    const VkMappedMemoryRange*                  pMemoryRanges) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateFlushMappedMemoryRanges]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateFlushMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordFlushMappedMemoryRanges]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordFlushMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
    }
    VkResult result = DispatchFlushMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordFlushMappedMemoryRanges]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordFlushMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL InvalidateMappedMemoryRanges(
    VkDevice                                    device,
    uint32_t                                    memoryRangeCount,
    const VkMappedMemoryRange*                  pMemoryRanges) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateInvalidateMappedMemoryRanges]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateInvalidateMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordInvalidateMappedMemoryRanges]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordInvalidateMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
    }
    VkResult result = DispatchInvalidateMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordInvalidateMappedMemoryRanges]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordInvalidateMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL GetDeviceMemoryCommitment(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    VkDeviceSize*                               pCommittedMemoryInBytes) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetDeviceMemoryCommitment]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetDeviceMemoryCommitment(device, memory, pCommittedMemoryInBytes);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetDeviceMemoryCommitment]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetDeviceMemoryCommitment(device, memory, pCommittedMemoryInBytes);
    }
    DispatchGetDeviceMemoryCommitment(device, memory, pCommittedMemoryInBytes);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetDeviceMemoryCommitment]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetDeviceMemoryCommitment(device, memory, pCommittedMemoryInBytes);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL BindBufferMemory(
    VkDevice                                    device,
    VkBuffer                                    buffer,
    VkDeviceMemory                              memory,
    VkDeviceSize                                memoryOffset) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateBindBufferMemory]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateBindBufferMemory(device, buffer, memory, memoryOffset);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordBindBufferMemory]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordBindBufferMemory(device, buffer, memory, memoryOffset);
    }
    VkResult result = DispatchBindBufferMemory(device, buffer, memory, memoryOffset);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordBindBufferMemory]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordBindBufferMemory(device, buffer, memory, memoryOffset, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL BindImageMemory(
    VkDevice                                    device,
    VkImage                                     image,
    VkDeviceMemory                              memory,
    VkDeviceSize                                memoryOffset) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateBindImageMemory]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateBindImageMemory(device, image, memory, memoryOffset);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordBindImageMemory]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordBindImageMemory(device, image, memory, memoryOffset);
    }
    VkResult result = DispatchBindImageMemory(device, image, memory, memoryOffset);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordBindImageMemory]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordBindImageMemory(device, image, memory, memoryOffset, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL GetBufferMemoryRequirements(
    VkDevice                                    device,
    VkBuffer                                    buffer,
    VkMemoryRequirements*                       pMemoryRequirements) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetBufferMemoryRequirements]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetBufferMemoryRequirements(device, buffer, pMemoryRequirements);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetBufferMemoryRequirements]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetBufferMemoryRequirements(device, buffer, pMemoryRequirements);
    }
    DispatchGetBufferMemoryRequirements(device, buffer, pMemoryRequirements);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetBufferMemoryRequirements]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetBufferMemoryRequirements(device, buffer, pMemoryRequirements);
    }
}

VKAPI_ATTR void VKAPI_CALL GetImageMemoryRequirements(
    VkDevice                                    device,
    VkImage                                     image,
    VkMemoryRequirements*                       pMemoryRequirements) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetImageMemoryRequirements]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetImageMemoryRequirements(device, image, pMemoryRequirements);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetImageMemoryRequirements]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetImageMemoryRequirements(device, image, pMemoryRequirements);
    }
    DispatchGetImageMemoryRequirements(device, image, pMemoryRequirements);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetImageMemoryRequirements]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetImageMemoryRequirements(device, image, pMemoryRequirements);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL CreateFence(
    VkDevice                                    device,
    const VkFenceCreateInfo*                    pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFence*                                    pFence) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCreateFence]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateFence(device, pCreateInfo, pAllocator, pFence);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateFence]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateFence(device, pCreateInfo, pAllocator, pFence);
    }
    VkResult result = DispatchCreateFence(device, pCreateInfo, pAllocator, pFence);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateFence]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCreateFence(device, pCreateInfo, pAllocator, pFence, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyFence(
    VkDevice                                    device,
    VkFence                                     fence,
    const VkAllocationCallbacks*                pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDestroyFence]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateDestroyFence(device, fence, pAllocator);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroyFence]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDestroyFence(device, fence, pAllocator);
    }
    DispatchDestroyFence(device, fence, pAllocator);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroyFence]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordDestroyFence(device, fence, pAllocator);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL ResetFences(
    VkDevice                                    device,
    uint32_t                                    fenceCount,
    const VkFence*                              pFences) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateResetFences]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateResetFences(device, fenceCount, pFences);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordResetFences]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordResetFences(device, fenceCount, pFences);
    }
    VkResult result = DispatchResetFences(device, fenceCount, pFences);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordResetFences]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordResetFences(device, fenceCount, pFences, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetFenceStatus(
    VkDevice                                    device,
    VkFence                                     fence) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetFenceStatus]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetFenceStatus(device, fence);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetFenceStatus]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetFenceStatus(device, fence);
    }
    VkResult result = DispatchGetFenceStatus(device, fence);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetFenceStatus]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetFenceStatus(device, fence, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL WaitForFences(
    VkDevice                                    device,
    uint32_t                                    fenceCount,
    const VkFence*                              pFences,
    VkBool32                                    waitAll,
    uint64_t                                    timeout) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateWaitForFences]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateWaitForFences(device, fenceCount, pFences, waitAll, timeout);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordWaitForFences]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordWaitForFences(device, fenceCount, pFences, waitAll, timeout);
    }
    VkResult result = DispatchWaitForFences(device, fenceCount, pFences, waitAll, timeout);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordWaitForFences]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordWaitForFences(device, fenceCount, pFences, waitAll, timeout, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL CreateSemaphore(
    VkDevice                                    device,
    const VkSemaphoreCreateInfo*                pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSemaphore*                                pSemaphore) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCreateSemaphore]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateSemaphore(device, pCreateInfo, pAllocator, pSemaphore);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateSemaphore]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateSemaphore(device, pCreateInfo, pAllocator, pSemaphore);
    }
    VkResult result = DispatchCreateSemaphore(device, pCreateInfo, pAllocator, pSemaphore);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateSemaphore]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCreateSemaphore(device, pCreateInfo, pAllocator, pSemaphore, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroySemaphore(
    VkDevice                                    device,
    VkSemaphore                                 semaphore,
    const VkAllocationCallbacks*                pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDestroySemaphore]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateDestroySemaphore(device, semaphore, pAllocator);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroySemaphore]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDestroySemaphore(device, semaphore, pAllocator);
    }
    DispatchDestroySemaphore(device, semaphore, pAllocator);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroySemaphore]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordDestroySemaphore(device, semaphore, pAllocator);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL CreateEvent(
    VkDevice                                    device,
    const VkEventCreateInfo*                    pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkEvent*                                    pEvent) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCreateEvent]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateEvent(device, pCreateInfo, pAllocator, pEvent);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateEvent]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateEvent(device, pCreateInfo, pAllocator, pEvent);
    }
    VkResult result = DispatchCreateEvent(device, pCreateInfo, pAllocator, pEvent);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateEvent]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCreateEvent(device, pCreateInfo, pAllocator, pEvent, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyEvent(
    VkDevice                                    device,
    VkEvent                                     event,
    const VkAllocationCallbacks*                pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDestroyEvent]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateDestroyEvent(device, event, pAllocator);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroyEvent]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDestroyEvent(device, event, pAllocator);
    }
    DispatchDestroyEvent(device, event, pAllocator);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroyEvent]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordDestroyEvent(device, event, pAllocator);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL GetEventStatus(
    VkDevice                                    device,
    VkEvent                                     event) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetEventStatus]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetEventStatus(device, event);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetEventStatus]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetEventStatus(device, event);
    }
    VkResult result = DispatchGetEventStatus(device, event);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetEventStatus]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetEventStatus(device, event, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL SetEvent(
    VkDevice                                    device,
    VkEvent                                     event) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateSetEvent]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateSetEvent(device, event);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordSetEvent]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordSetEvent(device, event);
    }
    VkResult result = DispatchSetEvent(device, event);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordSetEvent]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordSetEvent(device, event, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL ResetEvent(
    VkDevice                                    device,
    VkEvent                                     event) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateResetEvent]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateResetEvent(device, event);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordResetEvent]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordResetEvent(device, event);
    }
    VkResult result = DispatchResetEvent(device, event);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordResetEvent]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordResetEvent(device, event, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL CreateQueryPool(
    VkDevice                                    device,
    const VkQueryPoolCreateInfo*                pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkQueryPool*                                pQueryPool) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCreateQueryPool]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateQueryPool(device, pCreateInfo, pAllocator, pQueryPool);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateQueryPool]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateQueryPool(device, pCreateInfo, pAllocator, pQueryPool);
    }
    VkResult result = DispatchCreateQueryPool(device, pCreateInfo, pAllocator, pQueryPool);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateQueryPool]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCreateQueryPool(device, pCreateInfo, pAllocator, pQueryPool, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetQueryPoolResults(
    VkDevice                                    device,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount,
    size_t                                      dataSize,
    void*                                       pData,
    VkDeviceSize                                stride,
    VkQueryResultFlags                          flags) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetQueryPoolResults]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetQueryPoolResults(device, queryPool, firstQuery, queryCount, dataSize, pData, stride, flags);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetQueryPoolResults]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetQueryPoolResults(device, queryPool, firstQuery, queryCount, dataSize, pData, stride, flags);
    }
    VkResult result = DispatchGetQueryPoolResults(device, queryPool, firstQuery, queryCount, dataSize, pData, stride, flags);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetQueryPoolResults]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetQueryPoolResults(device, queryPool, firstQuery, queryCount, dataSize, pData, stride, flags, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyBuffer(
    VkDevice                                    device,
    VkBuffer                                    buffer,
    const VkAllocationCallbacks*                pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDestroyBuffer]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateDestroyBuffer(device, buffer, pAllocator);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroyBuffer]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDestroyBuffer(device, buffer, pAllocator);
    }
    DispatchDestroyBuffer(device, buffer, pAllocator);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroyBuffer]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordDestroyBuffer(device, buffer, pAllocator);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL CreateBufferView(
    VkDevice                                    device,
    const VkBufferViewCreateInfo*               pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkBufferView*                               pView) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCreateBufferView]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateBufferView(device, pCreateInfo, pAllocator, pView);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateBufferView]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateBufferView(device, pCreateInfo, pAllocator, pView);
    }
    VkResult result = DispatchCreateBufferView(device, pCreateInfo, pAllocator, pView);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateBufferView]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCreateBufferView(device, pCreateInfo, pAllocator, pView, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyBufferView(
    VkDevice                                    device,
    VkBufferView                                bufferView,
    const VkAllocationCallbacks*                pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDestroyBufferView]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateDestroyBufferView(device, bufferView, pAllocator);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroyBufferView]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDestroyBufferView(device, bufferView, pAllocator);
    }
    DispatchDestroyBufferView(device, bufferView, pAllocator);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroyBufferView]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordDestroyBufferView(device, bufferView, pAllocator);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL CreateImage(
    VkDevice                                    device,
    const VkImageCreateInfo*                    pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkImage*                                    pImage) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCreateImage]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateImage(device, pCreateInfo, pAllocator, pImage);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateImage]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateImage(device, pCreateInfo, pAllocator, pImage);
    }
    VkResult result = DispatchCreateImage(device, pCreateInfo, pAllocator, pImage);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateImage]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCreateImage(device, pCreateInfo, pAllocator, pImage, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyImage(
    VkDevice                                    device,
    VkImage                                     image,
    const VkAllocationCallbacks*                pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDestroyImage]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateDestroyImage(device, image, pAllocator);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroyImage]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDestroyImage(device, image, pAllocator);
    }
    DispatchDestroyImage(device, image, pAllocator);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroyImage]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordDestroyImage(device, image, pAllocator);
    }
}

VKAPI_ATTR void VKAPI_CALL GetImageSubresourceLayout(
    VkDevice                                    device,
    VkImage                                     image,
    const VkImageSubresource*                   pSubresource,
    VkSubresourceLayout*                        pLayout) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetImageSubresourceLayout]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetImageSubresourceLayout(device, image, pSubresource, pLayout);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetImageSubresourceLayout]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetImageSubresourceLayout(device, image, pSubresource, pLayout);
    }
    DispatchGetImageSubresourceLayout(device, image, pSubresource, pLayout);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetImageSubresourceLayout]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetImageSubresourceLayout(device, image, pSubresource, pLayout);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL CreateImageView(
    VkDevice                                    device,
    const VkImageViewCreateInfo*                pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkImageView*                                pView) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCreateImageView]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateImageView(device, pCreateInfo, pAllocator, pView);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateImageView]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateImageView(device, pCreateInfo, pAllocator, pView);
    }
    VkResult result = DispatchCreateImageView(device, pCreateInfo, pAllocator, pView);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateImageView]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCreateImageView(device, pCreateInfo, pAllocator, pView, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyImageView(
    VkDevice                                    device,
    VkImageView                                 imageView,
    const VkAllocationCallbacks*                pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDestroyImageView]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateDestroyImageView(device, imageView, pAllocator);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroyImageView]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDestroyImageView(device, imageView, pAllocator);
    }
    DispatchDestroyImageView(device, imageView, pAllocator);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroyImageView]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordDestroyImageView(device, imageView, pAllocator);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL CreatePipelineCache(
    VkDevice                                    device,
    const VkPipelineCacheCreateInfo*            pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkPipelineCache*                            pPipelineCache) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCreatePipelineCache]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreatePipelineCache(device, pCreateInfo, pAllocator, pPipelineCache);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreatePipelineCache]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreatePipelineCache(device, pCreateInfo, pAllocator, pPipelineCache);
    }
    VkResult result = DispatchCreatePipelineCache(device, pCreateInfo, pAllocator, pPipelineCache);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreatePipelineCache]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCreatePipelineCache(device, pCreateInfo, pAllocator, pPipelineCache, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyPipelineCache(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    const VkAllocationCallbacks*                pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDestroyPipelineCache]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateDestroyPipelineCache(device, pipelineCache, pAllocator);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroyPipelineCache]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDestroyPipelineCache(device, pipelineCache, pAllocator);
    }
    DispatchDestroyPipelineCache(device, pipelineCache, pAllocator);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroyPipelineCache]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordDestroyPipelineCache(device, pipelineCache, pAllocator);
    }
}

VKAPI_ATTR void VKAPI_CALL DestroyPipeline(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    const VkAllocationCallbacks*                pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDestroyPipeline]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateDestroyPipeline(device, pipeline, pAllocator);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroyPipeline]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDestroyPipeline(device, pipeline, pAllocator);
    }
    DispatchDestroyPipeline(device, pipeline, pAllocator);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroyPipeline]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordDestroyPipeline(device, pipeline, pAllocator);
    }
}

VKAPI_ATTR void VKAPI_CALL DestroyPipelineLayout(
    VkDevice                                    device,
    VkPipelineLayout                            pipelineLayout,
    const VkAllocationCallbacks*                pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDestroyPipelineLayout]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateDestroyPipelineLayout(device, pipelineLayout, pAllocator);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroyPipelineLayout]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDestroyPipelineLayout(device, pipelineLayout, pAllocator);
    }
    DispatchDestroyPipelineLayout(device, pipelineLayout, pAllocator);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroyPipelineLayout]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordDestroyPipelineLayout(device, pipelineLayout, pAllocator);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL CreateSampler(
    VkDevice                                    device,
    const VkSamplerCreateInfo*                  pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSampler*                                  pSampler) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCreateSampler]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateSampler(device, pCreateInfo, pAllocator, pSampler);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateSampler]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateSampler(device, pCreateInfo, pAllocator, pSampler);
    }
    VkResult result = DispatchCreateSampler(device, pCreateInfo, pAllocator, pSampler);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateSampler]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCreateSampler(device, pCreateInfo, pAllocator, pSampler, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroySampler(
    VkDevice                                    device,
    VkSampler                                   sampler,
    const VkAllocationCallbacks*                pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDestroySampler]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateDestroySampler(device, sampler, pAllocator);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroySampler]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDestroySampler(device, sampler, pAllocator);
    }
    DispatchDestroySampler(device, sampler, pAllocator);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroySampler]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordDestroySampler(device, sampler, pAllocator);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL CreateDescriptorSetLayout(
    VkDevice                                    device,
    const VkDescriptorSetLayoutCreateInfo*      pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorSetLayout*                      pSetLayout) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCreateDescriptorSetLayout]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateDescriptorSetLayout(device, pCreateInfo, pAllocator, pSetLayout);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateDescriptorSetLayout]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateDescriptorSetLayout(device, pCreateInfo, pAllocator, pSetLayout);
    }
    VkResult result = DispatchCreateDescriptorSetLayout(device, pCreateInfo, pAllocator, pSetLayout);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateDescriptorSetLayout]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCreateDescriptorSetLayout(device, pCreateInfo, pAllocator, pSetLayout, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyDescriptorSetLayout(
    VkDevice                                    device,
    VkDescriptorSetLayout                       descriptorSetLayout,
    const VkAllocationCallbacks*                pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDestroyDescriptorSetLayout]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateDestroyDescriptorSetLayout(device, descriptorSetLayout, pAllocator);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroyDescriptorSetLayout]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDestroyDescriptorSetLayout(device, descriptorSetLayout, pAllocator);
    }
    DispatchDestroyDescriptorSetLayout(device, descriptorSetLayout, pAllocator);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroyDescriptorSetLayout]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordDestroyDescriptorSetLayout(device, descriptorSetLayout, pAllocator);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL CreateDescriptorPool(
    VkDevice                                    device,
    const VkDescriptorPoolCreateInfo*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorPool*                           pDescriptorPool) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCreateDescriptorPool]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateDescriptorPool(device, pCreateInfo, pAllocator, pDescriptorPool);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateDescriptorPool]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateDescriptorPool(device, pCreateInfo, pAllocator, pDescriptorPool);
    }
    VkResult result = DispatchCreateDescriptorPool(device, pCreateInfo, pAllocator, pDescriptorPool);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateDescriptorPool]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCreateDescriptorPool(device, pCreateInfo, pAllocator, pDescriptorPool, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL ResetDescriptorPool(
    VkDevice                                    device,
    VkDescriptorPool                            descriptorPool,
    VkDescriptorPoolResetFlags                  flags) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateResetDescriptorPool]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateResetDescriptorPool(device, descriptorPool, flags);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordResetDescriptorPool]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordResetDescriptorPool(device, descriptorPool, flags);
    }
    VkResult result = DispatchResetDescriptorPool(device, descriptorPool, flags);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordResetDescriptorPool]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordResetDescriptorPool(device, descriptorPool, flags, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL FreeDescriptorSets(
    VkDevice                                    device,
    VkDescriptorPool                            descriptorPool,
    uint32_t                                    descriptorSetCount,
    const VkDescriptorSet*                      pDescriptorSets) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateFreeDescriptorSets]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateFreeDescriptorSets(device, descriptorPool, descriptorSetCount, pDescriptorSets);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordFreeDescriptorSets]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordFreeDescriptorSets(device, descriptorPool, descriptorSetCount, pDescriptorSets);
    }
    VkResult result = DispatchFreeDescriptorSets(device, descriptorPool, descriptorSetCount, pDescriptorSets);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordFreeDescriptorSets]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordFreeDescriptorSets(device, descriptorPool, descriptorSetCount, pDescriptorSets, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL UpdateDescriptorSets(
    VkDevice                                    device,
    uint32_t                                    descriptorWriteCount,
    const VkWriteDescriptorSet*                 pDescriptorWrites,
    uint32_t                                    descriptorCopyCount,
    const VkCopyDescriptorSet*                  pDescriptorCopies) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateUpdateDescriptorSets]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateUpdateDescriptorSets(device, descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordUpdateDescriptorSets]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordUpdateDescriptorSets(device, descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies);
    }
    DispatchUpdateDescriptorSets(device, descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordUpdateDescriptorSets]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordUpdateDescriptorSets(device, descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL CreateFramebuffer(
    VkDevice                                    device,
    const VkFramebufferCreateInfo*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFramebuffer*                              pFramebuffer) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCreateFramebuffer]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateFramebuffer(device, pCreateInfo, pAllocator, pFramebuffer);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateFramebuffer]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateFramebuffer(device, pCreateInfo, pAllocator, pFramebuffer);
    }
    VkResult result = DispatchCreateFramebuffer(device, pCreateInfo, pAllocator, pFramebuffer);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateFramebuffer]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCreateFramebuffer(device, pCreateInfo, pAllocator, pFramebuffer, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyFramebuffer(
    VkDevice                                    device,
    VkFramebuffer                               framebuffer,
    const VkAllocationCallbacks*                pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDestroyFramebuffer]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateDestroyFramebuffer(device, framebuffer, pAllocator);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroyFramebuffer]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDestroyFramebuffer(device, framebuffer, pAllocator);
    }
    DispatchDestroyFramebuffer(device, framebuffer, pAllocator);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroyFramebuffer]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordDestroyFramebuffer(device, framebuffer, pAllocator);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL CreateRenderPass(
    VkDevice                                    device,
    const VkRenderPassCreateInfo*               pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkRenderPass*                               pRenderPass) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCreateRenderPass]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateRenderPass(device, pCreateInfo, pAllocator, pRenderPass);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateRenderPass]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateRenderPass(device, pCreateInfo, pAllocator, pRenderPass);
    }
    VkResult result = DispatchCreateRenderPass(device, pCreateInfo, pAllocator, pRenderPass);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateRenderPass]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCreateRenderPass(device, pCreateInfo, pAllocator, pRenderPass, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyRenderPass(
    VkDevice                                    device,
    VkRenderPass                                renderPass,
    const VkAllocationCallbacks*                pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDestroyRenderPass]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateDestroyRenderPass(device, renderPass, pAllocator);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroyRenderPass]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDestroyRenderPass(device, renderPass, pAllocator);
    }
    DispatchDestroyRenderPass(device, renderPass, pAllocator);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroyRenderPass]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordDestroyRenderPass(device, renderPass, pAllocator);
    }
}

VKAPI_ATTR void VKAPI_CALL GetRenderAreaGranularity(
    VkDevice                                    device,
    VkRenderPass                                renderPass,
    VkExtent2D*                                 pGranularity) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetRenderAreaGranularity]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetRenderAreaGranularity(device, renderPass, pGranularity);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetRenderAreaGranularity]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetRenderAreaGranularity(device, renderPass, pGranularity);
    }
    DispatchGetRenderAreaGranularity(device, renderPass, pGranularity);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetRenderAreaGranularity]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetRenderAreaGranularity(device, renderPass, pGranularity);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL CreateCommandPool(
    VkDevice                                    device,
    const VkCommandPoolCreateInfo*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkCommandPool*                              pCommandPool) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCreateCommandPool]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateCommandPool(device, pCreateInfo, pAllocator, pCommandPool);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateCommandPool]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateCommandPool(device, pCreateInfo, pAllocator, pCommandPool);
    }
    VkResult result = DispatchCreateCommandPool(device, pCreateInfo, pAllocator, pCommandPool);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateCommandPool]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCreateCommandPool(device, pCreateInfo, pAllocator, pCommandPool, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL ResetCommandPool(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    VkCommandPoolResetFlags                     flags) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateResetCommandPool]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateResetCommandPool(device, commandPool, flags);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordResetCommandPool]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordResetCommandPool(device, commandPool, flags);
    }
    VkResult result = DispatchResetCommandPool(device, commandPool, flags);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordResetCommandPool]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordResetCommandPool(device, commandPool, flags, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL AllocateCommandBuffers(
    VkDevice                                    device,
    const VkCommandBufferAllocateInfo*          pAllocateInfo,
    VkCommandBuffer*                            pCommandBuffers) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateAllocateCommandBuffers]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateAllocateCommandBuffers(device, pAllocateInfo, pCommandBuffers);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordAllocateCommandBuffers]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordAllocateCommandBuffers(device, pAllocateInfo, pCommandBuffers);
    }
    VkResult result = DispatchAllocateCommandBuffers(device, pAllocateInfo, pCommandBuffers);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordAllocateCommandBuffers]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordAllocateCommandBuffers(device, pAllocateInfo, pCommandBuffers, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL FreeCommandBuffers(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    uint32_t                                    commandBufferCount,
    const VkCommandBuffer*                      pCommandBuffers) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateFreeCommandBuffers]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateFreeCommandBuffers(device, commandPool, commandBufferCount, pCommandBuffers);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordFreeCommandBuffers]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordFreeCommandBuffers(device, commandPool, commandBufferCount, pCommandBuffers);
    }
    DispatchFreeCommandBuffers(device, commandPool, commandBufferCount, pCommandBuffers);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordFreeCommandBuffers]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordFreeCommandBuffers(device, commandPool, commandBufferCount, pCommandBuffers);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL BeginCommandBuffer(
    VkCommandBuffer                             commandBuffer,
    const VkCommandBufferBeginInfo*             pBeginInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateBeginCommandBuffer]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateBeginCommandBuffer(commandBuffer, pBeginInfo);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordBeginCommandBuffer]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordBeginCommandBuffer(commandBuffer, pBeginInfo);
    }
    VkResult result = DispatchBeginCommandBuffer(commandBuffer, pBeginInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordBeginCommandBuffer]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordBeginCommandBuffer(commandBuffer, pBeginInfo, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL EndCommandBuffer(
    VkCommandBuffer                             commandBuffer) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateEndCommandBuffer]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateEndCommandBuffer(commandBuffer);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordEndCommandBuffer]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordEndCommandBuffer(commandBuffer);
    }
    VkResult result = DispatchEndCommandBuffer(commandBuffer);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordEndCommandBuffer]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordEndCommandBuffer(commandBuffer, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL ResetCommandBuffer(
    VkCommandBuffer                             commandBuffer,
    VkCommandBufferResetFlags                   flags) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateResetCommandBuffer]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateResetCommandBuffer(commandBuffer, flags);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordResetCommandBuffer]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordResetCommandBuffer(commandBuffer, flags);
    }
    VkResult result = DispatchResetCommandBuffer(commandBuffer, flags);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordResetCommandBuffer]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordResetCommandBuffer(commandBuffer, flags, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL CmdBindPipeline(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipeline                                  pipeline) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdBindPipeline]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdBindPipeline]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline);
    }
    DispatchCmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdBindPipeline]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetViewport(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstViewport,
    uint32_t                                    viewportCount,
    const VkViewport*                           pViewports) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetViewport]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetViewport(commandBuffer, firstViewport, viewportCount, pViewports);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetViewport]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetViewport(commandBuffer, firstViewport, viewportCount, pViewports);
    }
    DispatchCmdSetViewport(commandBuffer, firstViewport, viewportCount, pViewports);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetViewport]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetViewport(commandBuffer, firstViewport, viewportCount, pViewports);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetScissor(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstScissor,
    uint32_t                                    scissorCount,
    const VkRect2D*                             pScissors) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetScissor]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetScissor(commandBuffer, firstScissor, scissorCount, pScissors);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetScissor]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetScissor(commandBuffer, firstScissor, scissorCount, pScissors);
    }
    DispatchCmdSetScissor(commandBuffer, firstScissor, scissorCount, pScissors);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetScissor]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetScissor(commandBuffer, firstScissor, scissorCount, pScissors);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetLineWidth(
    VkCommandBuffer                             commandBuffer,
    float                                       lineWidth) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetLineWidth]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetLineWidth(commandBuffer, lineWidth);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetLineWidth]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetLineWidth(commandBuffer, lineWidth);
    }
    DispatchCmdSetLineWidth(commandBuffer, lineWidth);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetLineWidth]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetLineWidth(commandBuffer, lineWidth);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetDepthBias(
    VkCommandBuffer                             commandBuffer,
    float                                       depthBiasConstantFactor,
    float                                       depthBiasClamp,
    float                                       depthBiasSlopeFactor) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetDepthBias]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetDepthBias(commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetDepthBias]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetDepthBias(commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
    }
    DispatchCmdSetDepthBias(commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetDepthBias]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetDepthBias(commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetBlendConstants(
    VkCommandBuffer                             commandBuffer,
    const float                                 blendConstants[4]) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetBlendConstants]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetBlendConstants(commandBuffer, blendConstants);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetBlendConstants]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetBlendConstants(commandBuffer, blendConstants);
    }
    DispatchCmdSetBlendConstants(commandBuffer, blendConstants);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetBlendConstants]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetBlendConstants(commandBuffer, blendConstants);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetDepthBounds(
    VkCommandBuffer                             commandBuffer,
    float                                       minDepthBounds,
    float                                       maxDepthBounds) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetDepthBounds]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetDepthBounds(commandBuffer, minDepthBounds, maxDepthBounds);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetDepthBounds]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetDepthBounds(commandBuffer, minDepthBounds, maxDepthBounds);
    }
    DispatchCmdSetDepthBounds(commandBuffer, minDepthBounds, maxDepthBounds);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetDepthBounds]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetDepthBounds(commandBuffer, minDepthBounds, maxDepthBounds);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetStencilCompareMask(
    VkCommandBuffer                             commandBuffer,
    VkStencilFaceFlags                          faceMask,
    uint32_t                                    compareMask) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetStencilCompareMask]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetStencilCompareMask(commandBuffer, faceMask, compareMask);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetStencilCompareMask]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetStencilCompareMask(commandBuffer, faceMask, compareMask);
    }
    DispatchCmdSetStencilCompareMask(commandBuffer, faceMask, compareMask);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetStencilCompareMask]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetStencilCompareMask(commandBuffer, faceMask, compareMask);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetStencilWriteMask(
    VkCommandBuffer                             commandBuffer,
    VkStencilFaceFlags                          faceMask,
    uint32_t                                    writeMask) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetStencilWriteMask]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetStencilWriteMask(commandBuffer, faceMask, writeMask);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetStencilWriteMask]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetStencilWriteMask(commandBuffer, faceMask, writeMask);
    }
    DispatchCmdSetStencilWriteMask(commandBuffer, faceMask, writeMask);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetStencilWriteMask]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetStencilWriteMask(commandBuffer, faceMask, writeMask);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetStencilReference(
    VkCommandBuffer                             commandBuffer,
    VkStencilFaceFlags                          faceMask,
    uint32_t                                    reference) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetStencilReference]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetStencilReference(commandBuffer, faceMask, reference);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetStencilReference]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetStencilReference(commandBuffer, faceMask, reference);
    }
    DispatchCmdSetStencilReference(commandBuffer, faceMask, reference);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetStencilReference]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetStencilReference(commandBuffer, faceMask, reference);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdBindDescriptorSets(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipelineLayout                            layout,
    uint32_t                                    firstSet,
    uint32_t                                    descriptorSetCount,
    const VkDescriptorSet*                      pDescriptorSets,
    uint32_t                                    dynamicOffsetCount,
    const uint32_t*                             pDynamicOffsets) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdBindDescriptorSets]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdBindDescriptorSets(commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdBindDescriptorSets]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdBindDescriptorSets(commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
    }
    DispatchCmdBindDescriptorSets(commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdBindDescriptorSets]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdBindDescriptorSets(commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdBindIndexBuffer(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkIndexType                                 indexType) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdBindIndexBuffer]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdBindIndexBuffer(commandBuffer, buffer, offset, indexType);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdBindIndexBuffer]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdBindIndexBuffer(commandBuffer, buffer, offset, indexType);
    }
    DispatchCmdBindIndexBuffer(commandBuffer, buffer, offset, indexType);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdBindIndexBuffer]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdBindIndexBuffer(commandBuffer, buffer, offset, indexType);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdBindVertexBuffers(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstBinding,
    uint32_t                                    bindingCount,
    const VkBuffer*                             pBuffers,
    const VkDeviceSize*                         pOffsets) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdBindVertexBuffers]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdBindVertexBuffers(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdBindVertexBuffers]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdBindVertexBuffers(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);
    }
    DispatchCmdBindVertexBuffers(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdBindVertexBuffers]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdBindVertexBuffers(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdDraw(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    vertexCount,
    uint32_t                                    instanceCount,
    uint32_t                                    firstVertex,
    uint32_t                                    firstInstance) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdDraw]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdDraw]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
    }
    DispatchCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdDraw]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdDrawIndexed(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    indexCount,
    uint32_t                                    instanceCount,
    uint32_t                                    firstIndex,
    int32_t                                     vertexOffset,
    uint32_t                                    firstInstance) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdDrawIndexed]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdDrawIndexed]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    }
    DispatchCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdDrawIndexed]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdDrawIndirect(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    uint32_t                                    drawCount,
    uint32_t                                    stride) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdDrawIndirect]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdDrawIndirect(commandBuffer, buffer, offset, drawCount, stride);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdDrawIndirect]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdDrawIndirect(commandBuffer, buffer, offset, drawCount, stride);
    }
    DispatchCmdDrawIndirect(commandBuffer, buffer, offset, drawCount, stride);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdDrawIndirect]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdDrawIndirect(commandBuffer, buffer, offset, drawCount, stride);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdDrawIndexedIndirect(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    uint32_t                                    drawCount,
    uint32_t                                    stride) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdDrawIndexedIndirect]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdDrawIndexedIndirect(commandBuffer, buffer, offset, drawCount, stride);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdDrawIndexedIndirect]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdDrawIndexedIndirect(commandBuffer, buffer, offset, drawCount, stride);
    }
    DispatchCmdDrawIndexedIndirect(commandBuffer, buffer, offset, drawCount, stride);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdDrawIndexedIndirect]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdDrawIndexedIndirect(commandBuffer, buffer, offset, drawCount, stride);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdDispatch(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    groupCountX,
    uint32_t                                    groupCountY,
    uint32_t                                    groupCountZ) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdDispatch]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdDispatch(commandBuffer, groupCountX, groupCountY, groupCountZ);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdDispatch]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdDispatch(commandBuffer, groupCountX, groupCountY, groupCountZ);
    }
    DispatchCmdDispatch(commandBuffer, groupCountX, groupCountY, groupCountZ);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdDispatch]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdDispatch(commandBuffer, groupCountX, groupCountY, groupCountZ);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdDispatchIndirect(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdDispatchIndirect]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdDispatchIndirect(commandBuffer, buffer, offset);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdDispatchIndirect]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdDispatchIndirect(commandBuffer, buffer, offset);
    }
    DispatchCmdDispatchIndirect(commandBuffer, buffer, offset);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdDispatchIndirect]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdDispatchIndirect(commandBuffer, buffer, offset);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdCopyBuffer(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    srcBuffer,
    VkBuffer                                    dstBuffer,
    uint32_t                                    regionCount,
    const VkBufferCopy*                         pRegions) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdCopyBuffer]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdCopyBuffer]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
    }
    DispatchCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdCopyBuffer]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdCopyImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     srcImage,
    VkImageLayout                               srcImageLayout,
    VkImage                                     dstImage,
    VkImageLayout                               dstImageLayout,
    uint32_t                                    regionCount,
    const VkImageCopy*                          pRegions) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdCopyImage]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdCopyImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdCopyImage]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdCopyImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
    }
    DispatchCmdCopyImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdCopyImage]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdCopyImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdBlitImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     srcImage,
    VkImageLayout                               srcImageLayout,
    VkImage                                     dstImage,
    VkImageLayout                               dstImageLayout,
    uint32_t                                    regionCount,
    const VkImageBlit*                          pRegions,
    VkFilter                                    filter) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdBlitImage]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdBlitImage]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter);
    }
    DispatchCmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdBlitImage]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdCopyBufferToImage(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    srcBuffer,
    VkImage                                     dstImage,
    VkImageLayout                               dstImageLayout,
    uint32_t                                    regionCount,
    const VkBufferImageCopy*                    pRegions) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdCopyBufferToImage]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdCopyBufferToImage]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
    }
    DispatchCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdCopyBufferToImage]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdCopyImageToBuffer(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     srcImage,
    VkImageLayout                               srcImageLayout,
    VkBuffer                                    dstBuffer,
    uint32_t                                    regionCount,
    const VkBufferImageCopy*                    pRegions) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdCopyImageToBuffer]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdCopyImageToBuffer]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);
    }
    DispatchCmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdCopyImageToBuffer]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdUpdateBuffer(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    dstBuffer,
    VkDeviceSize                                dstOffset,
    VkDeviceSize                                dataSize,
    const void*                                 pData) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdUpdateBuffer]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdUpdateBuffer(commandBuffer, dstBuffer, dstOffset, dataSize, pData);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdUpdateBuffer]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdUpdateBuffer(commandBuffer, dstBuffer, dstOffset, dataSize, pData);
    }
    DispatchCmdUpdateBuffer(commandBuffer, dstBuffer, dstOffset, dataSize, pData);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdUpdateBuffer]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdUpdateBuffer(commandBuffer, dstBuffer, dstOffset, dataSize, pData);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdFillBuffer(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    dstBuffer,
    VkDeviceSize                                dstOffset,
    VkDeviceSize                                size,
    uint32_t                                    data) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdFillBuffer]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdFillBuffer(commandBuffer, dstBuffer, dstOffset, size, data);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdFillBuffer]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdFillBuffer(commandBuffer, dstBuffer, dstOffset, size, data);
    }
    DispatchCmdFillBuffer(commandBuffer, dstBuffer, dstOffset, size, data);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdFillBuffer]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdFillBuffer(commandBuffer, dstBuffer, dstOffset, size, data);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdClearColorImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     image,
    VkImageLayout                               imageLayout,
    const VkClearColorValue*                    pColor,
    uint32_t                                    rangeCount,
    const VkImageSubresourceRange*              pRanges) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdClearColorImage]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdClearColorImage(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdClearColorImage]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdClearColorImage(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);
    }
    DispatchCmdClearColorImage(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdClearColorImage]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdClearColorImage(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdClearDepthStencilImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     image,
    VkImageLayout                               imageLayout,
    const VkClearDepthStencilValue*             pDepthStencil,
    uint32_t                                    rangeCount,
    const VkImageSubresourceRange*              pRanges) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdClearDepthStencilImage]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdClearDepthStencilImage(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdClearDepthStencilImage]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdClearDepthStencilImage(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges);
    }
    DispatchCmdClearDepthStencilImage(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdClearDepthStencilImage]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdClearDepthStencilImage(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdClearAttachments(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    attachmentCount,
    const VkClearAttachment*                    pAttachments,
    uint32_t                                    rectCount,
    const VkClearRect*                          pRects) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdClearAttachments]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdClearAttachments(commandBuffer, attachmentCount, pAttachments, rectCount, pRects);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdClearAttachments]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdClearAttachments(commandBuffer, attachmentCount, pAttachments, rectCount, pRects);
    }
    DispatchCmdClearAttachments(commandBuffer, attachmentCount, pAttachments, rectCount, pRects);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdClearAttachments]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdClearAttachments(commandBuffer, attachmentCount, pAttachments, rectCount, pRects);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdResolveImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     srcImage,
    VkImageLayout                               srcImageLayout,
    VkImage                                     dstImage,
    VkImageLayout                               dstImageLayout,
    uint32_t                                    regionCount,
    const VkImageResolve*                       pRegions) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdResolveImage]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdResolveImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdResolveImage]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdResolveImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
    }
    DispatchCmdResolveImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdResolveImage]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdResolveImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetEvent(
    VkCommandBuffer                             commandBuffer,
    VkEvent                                     event,
    VkPipelineStageFlags                        stageMask) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetEvent]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetEvent(commandBuffer, event, stageMask);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetEvent]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetEvent(commandBuffer, event, stageMask);
    }
    DispatchCmdSetEvent(commandBuffer, event, stageMask);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetEvent]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetEvent(commandBuffer, event, stageMask);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdResetEvent(
    VkCommandBuffer                             commandBuffer,
    VkEvent                                     event,
    VkPipelineStageFlags                        stageMask) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdResetEvent]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdResetEvent(commandBuffer, event, stageMask);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdResetEvent]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdResetEvent(commandBuffer, event, stageMask);
    }
    DispatchCmdResetEvent(commandBuffer, event, stageMask);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdResetEvent]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdResetEvent(commandBuffer, event, stageMask);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdWaitEvents(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    eventCount,
    const VkEvent*                              pEvents,
    VkPipelineStageFlags                        srcStageMask,
    VkPipelineStageFlags                        dstStageMask,
    uint32_t                                    memoryBarrierCount,
    const VkMemoryBarrier*                      pMemoryBarriers,
    uint32_t                                    bufferMemoryBarrierCount,
    const VkBufferMemoryBarrier*                pBufferMemoryBarriers,
    uint32_t                                    imageMemoryBarrierCount,
    const VkImageMemoryBarrier*                 pImageMemoryBarriers) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdWaitEvents]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdWaitEvents(commandBuffer, eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdWaitEvents]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdWaitEvents(commandBuffer, eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    }
    DispatchCmdWaitEvents(commandBuffer, eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdWaitEvents]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdWaitEvents(commandBuffer, eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdPipelineBarrier(
    VkCommandBuffer                             commandBuffer,
    VkPipelineStageFlags                        srcStageMask,
    VkPipelineStageFlags                        dstStageMask,
    VkDependencyFlags                           dependencyFlags,
    uint32_t                                    memoryBarrierCount,
    const VkMemoryBarrier*                      pMemoryBarriers,
    uint32_t                                    bufferMemoryBarrierCount,
    const VkBufferMemoryBarrier*                pBufferMemoryBarriers,
    uint32_t                                    imageMemoryBarrierCount,
    const VkImageMemoryBarrier*                 pImageMemoryBarriers) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdPipelineBarrier]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdPipelineBarrier]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    }
    DispatchCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdPipelineBarrier]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdBeginQuery(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    query,
    VkQueryControlFlags                         flags) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdBeginQuery]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdBeginQuery(commandBuffer, queryPool, query, flags);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdBeginQuery]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdBeginQuery(commandBuffer, queryPool, query, flags);
    }
    DispatchCmdBeginQuery(commandBuffer, queryPool, query, flags);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdBeginQuery]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdBeginQuery(commandBuffer, queryPool, query, flags);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdEndQuery(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    query) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdEndQuery]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdEndQuery(commandBuffer, queryPool, query);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdEndQuery]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdEndQuery(commandBuffer, queryPool, query);
    }
    DispatchCmdEndQuery(commandBuffer, queryPool, query);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdEndQuery]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdEndQuery(commandBuffer, queryPool, query);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdResetQueryPool(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdResetQueryPool]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdResetQueryPool(commandBuffer, queryPool, firstQuery, queryCount);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdResetQueryPool]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdResetQueryPool(commandBuffer, queryPool, firstQuery, queryCount);
    }
    DispatchCmdResetQueryPool(commandBuffer, queryPool, firstQuery, queryCount);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdResetQueryPool]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdResetQueryPool(commandBuffer, queryPool, firstQuery, queryCount);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdWriteTimestamp(
    VkCommandBuffer                             commandBuffer,
    VkPipelineStageFlagBits                     pipelineStage,
    VkQueryPool                                 queryPool,
    uint32_t                                    query) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdWriteTimestamp]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdWriteTimestamp(commandBuffer, pipelineStage, queryPool, query);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdWriteTimestamp]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdWriteTimestamp(commandBuffer, pipelineStage, queryPool, query);
    }
    DispatchCmdWriteTimestamp(commandBuffer, pipelineStage, queryPool, query);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdWriteTimestamp]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdWriteTimestamp(commandBuffer, pipelineStage, queryPool, query);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdCopyQueryPoolResults(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount,
    VkBuffer                                    dstBuffer,
    VkDeviceSize                                dstOffset,
    VkDeviceSize                                stride,
    VkQueryResultFlags                          flags) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdCopyQueryPoolResults]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdCopyQueryPoolResults(commandBuffer, queryPool, firstQuery, queryCount, dstBuffer, dstOffset, stride, flags);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdCopyQueryPoolResults]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdCopyQueryPoolResults(commandBuffer, queryPool, firstQuery, queryCount, dstBuffer, dstOffset, stride, flags);
    }
    DispatchCmdCopyQueryPoolResults(commandBuffer, queryPool, firstQuery, queryCount, dstBuffer, dstOffset, stride, flags);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdCopyQueryPoolResults]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdCopyQueryPoolResults(commandBuffer, queryPool, firstQuery, queryCount, dstBuffer, dstOffset, stride, flags);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdPushConstants(
    VkCommandBuffer                             commandBuffer,
    VkPipelineLayout                            layout,
    VkShaderStageFlags                          stageFlags,
    uint32_t                                    offset,
    uint32_t                                    size,
    const void*                                 pValues) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdPushConstants]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdPushConstants(commandBuffer, layout, stageFlags, offset, size, pValues);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdPushConstants]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdPushConstants(commandBuffer, layout, stageFlags, offset, size, pValues);
    }
    DispatchCmdPushConstants(commandBuffer, layout, stageFlags, offset, size, pValues);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdPushConstants]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdPushConstants(commandBuffer, layout, stageFlags, offset, size, pValues);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdBeginRenderPass(
    VkCommandBuffer                             commandBuffer,
    const VkRenderPassBeginInfo*                pRenderPassBegin,
    VkSubpassContents                           contents) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdBeginRenderPass]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdBeginRenderPass]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);
    }
    DispatchCmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdBeginRenderPass]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdNextSubpass(
    VkCommandBuffer                             commandBuffer,
    VkSubpassContents                           contents) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdNextSubpass]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdNextSubpass(commandBuffer, contents);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdNextSubpass]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdNextSubpass(commandBuffer, contents);
    }
    DispatchCmdNextSubpass(commandBuffer, contents);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdNextSubpass]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdNextSubpass(commandBuffer, contents);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdEndRenderPass(
    VkCommandBuffer                             commandBuffer) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdEndRenderPass]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdEndRenderPass(commandBuffer);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdEndRenderPass]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdEndRenderPass(commandBuffer);
    }
    DispatchCmdEndRenderPass(commandBuffer);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdEndRenderPass]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdEndRenderPass(commandBuffer);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdExecuteCommands(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    commandBufferCount,
    const VkCommandBuffer*                      pCommandBuffers) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdExecuteCommands]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdExecuteCommands(commandBuffer, commandBufferCount, pCommandBuffers);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdExecuteCommands]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdExecuteCommands(commandBuffer, commandBufferCount, pCommandBuffers);
    }
    DispatchCmdExecuteCommands(commandBuffer, commandBufferCount, pCommandBuffers);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdExecuteCommands]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdExecuteCommands(commandBuffer, commandBufferCount, pCommandBuffers);
    }
}


VKAPI_ATTR VkResult VKAPI_CALL BindBufferMemory2(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindBufferMemoryInfo*               pBindInfos) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateBindBufferMemory2]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateBindBufferMemory2(device, bindInfoCount, pBindInfos);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordBindBufferMemory2]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordBindBufferMemory2(device, bindInfoCount, pBindInfos);
    }
    VkResult result = DispatchBindBufferMemory2(device, bindInfoCount, pBindInfos);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordBindBufferMemory2]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordBindBufferMemory2(device, bindInfoCount, pBindInfos, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL BindImageMemory2(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindImageMemoryInfo*                pBindInfos) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateBindImageMemory2]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateBindImageMemory2(device, bindInfoCount, pBindInfos);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordBindImageMemory2]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordBindImageMemory2(device, bindInfoCount, pBindInfos);
    }
    VkResult result = DispatchBindImageMemory2(device, bindInfoCount, pBindInfos);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordBindImageMemory2]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordBindImageMemory2(device, bindInfoCount, pBindInfos, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL GetDeviceGroupPeerMemoryFeatures(
    VkDevice                                    device,
    uint32_t                                    heapIndex,
    uint32_t                                    localDeviceIndex,
    uint32_t                                    remoteDeviceIndex,
    VkPeerMemoryFeatureFlags*                   pPeerMemoryFeatures) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetDeviceGroupPeerMemoryFeatures]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetDeviceGroupPeerMemoryFeatures(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetDeviceGroupPeerMemoryFeatures]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetDeviceGroupPeerMemoryFeatures(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
    }
    DispatchGetDeviceGroupPeerMemoryFeatures(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetDeviceGroupPeerMemoryFeatures]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetDeviceGroupPeerMemoryFeatures(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetDeviceMask(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    deviceMask) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetDeviceMask]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetDeviceMask(commandBuffer, deviceMask);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetDeviceMask]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetDeviceMask(commandBuffer, deviceMask);
    }
    DispatchCmdSetDeviceMask(commandBuffer, deviceMask);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetDeviceMask]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetDeviceMask(commandBuffer, deviceMask);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdDispatchBase(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    baseGroupX,
    uint32_t                                    baseGroupY,
    uint32_t                                    baseGroupZ,
    uint32_t                                    groupCountX,
    uint32_t                                    groupCountY,
    uint32_t                                    groupCountZ) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdDispatchBase]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdDispatchBase(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdDispatchBase]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdDispatchBase(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
    }
    DispatchCmdDispatchBase(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdDispatchBase]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdDispatchBase(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL EnumeratePhysicalDeviceGroups(
    VkInstance                                  instance,
    uint32_t*                                   pPhysicalDeviceGroupCount,
    VkPhysicalDeviceGroupProperties*            pPhysicalDeviceGroupProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateEnumeratePhysicalDeviceGroups(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordEnumeratePhysicalDeviceGroups(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
    }
    VkResult result = DispatchEnumeratePhysicalDeviceGroups(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordEnumeratePhysicalDeviceGroups(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL GetImageMemoryRequirements2(
    VkDevice                                    device,
    const VkImageMemoryRequirementsInfo2*       pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetImageMemoryRequirements2]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetImageMemoryRequirements2(device, pInfo, pMemoryRequirements);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetImageMemoryRequirements2]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetImageMemoryRequirements2(device, pInfo, pMemoryRequirements);
    }
    DispatchGetImageMemoryRequirements2(device, pInfo, pMemoryRequirements);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetImageMemoryRequirements2]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetImageMemoryRequirements2(device, pInfo, pMemoryRequirements);
    }
}

VKAPI_ATTR void VKAPI_CALL GetBufferMemoryRequirements2(
    VkDevice                                    device,
    const VkBufferMemoryRequirementsInfo2*      pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetBufferMemoryRequirements2]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetBufferMemoryRequirements2(device, pInfo, pMemoryRequirements);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetBufferMemoryRequirements2]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetBufferMemoryRequirements2(device, pInfo, pMemoryRequirements);
    }
    DispatchGetBufferMemoryRequirements2(device, pInfo, pMemoryRequirements);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetBufferMemoryRequirements2]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetBufferMemoryRequirements2(device, pInfo, pMemoryRequirements);
    }
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceFeatures2(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceFeatures2*                  pFeatures) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceFeatures2(physicalDevice, pFeatures);
        if (skip) return;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceFeatures2(physicalDevice, pFeatures);
    }
    DispatchGetPhysicalDeviceFeatures2(physicalDevice, pFeatures);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetPhysicalDeviceFeatures2(physicalDevice, pFeatures);
    }
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceProperties2(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceProperties2*                pProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceProperties2(physicalDevice, pProperties);
        if (skip) return;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceProperties2(physicalDevice, pProperties);
    }
    DispatchGetPhysicalDeviceProperties2(physicalDevice, pProperties);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetPhysicalDeviceProperties2(physicalDevice, pProperties);
    }
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceFormatProperties2(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkFormatProperties2*                        pFormatProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceFormatProperties2(physicalDevice, format, pFormatProperties);
        if (skip) return;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceFormatProperties2(physicalDevice, format, pFormatProperties);
    }
    DispatchGetPhysicalDeviceFormatProperties2(physicalDevice, format, pFormatProperties);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetPhysicalDeviceFormatProperties2(physicalDevice, format, pFormatProperties);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceImageFormatProperties2(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceImageFormatInfo2*     pImageFormatInfo,
    VkImageFormatProperties2*                   pImageFormatProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceImageFormatProperties2(physicalDevice, pImageFormatInfo, pImageFormatProperties);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceImageFormatProperties2(physicalDevice, pImageFormatInfo, pImageFormatProperties);
    }
    VkResult result = DispatchGetPhysicalDeviceImageFormatProperties2(physicalDevice, pImageFormatInfo, pImageFormatProperties);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetPhysicalDeviceImageFormatProperties2(physicalDevice, pImageFormatInfo, pImageFormatProperties, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceQueueFamilyProperties2(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pQueueFamilyPropertyCount,
    VkQueueFamilyProperties2*                   pQueueFamilyProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
        if (skip) return;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
    }
    DispatchGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
    }
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceMemoryProperties2(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceMemoryProperties2*          pMemoryProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceMemoryProperties2(physicalDevice, pMemoryProperties);
        if (skip) return;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceMemoryProperties2(physicalDevice, pMemoryProperties);
    }
    DispatchGetPhysicalDeviceMemoryProperties2(physicalDevice, pMemoryProperties);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetPhysicalDeviceMemoryProperties2(physicalDevice, pMemoryProperties);
    }
}

VKAPI_ATTR void VKAPI_CALL GetDeviceQueue2(
    VkDevice                                    device,
    const VkDeviceQueueInfo2*                   pQueueInfo,
    VkQueue*                                    pQueue) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetDeviceQueue2]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetDeviceQueue2(device, pQueueInfo, pQueue);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetDeviceQueue2]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetDeviceQueue2(device, pQueueInfo, pQueue);
    }
    DispatchGetDeviceQueue2(device, pQueueInfo, pQueue);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetDeviceQueue2]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetDeviceQueue2(device, pQueueInfo, pQueue);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL CreateSamplerYcbcrConversion(
    VkDevice                                    device,
    const VkSamplerYcbcrConversionCreateInfo*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSamplerYcbcrConversion*                   pYcbcrConversion) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCreateSamplerYcbcrConversion]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateSamplerYcbcrConversion(device, pCreateInfo, pAllocator, pYcbcrConversion);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateSamplerYcbcrConversion]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateSamplerYcbcrConversion(device, pCreateInfo, pAllocator, pYcbcrConversion);
    }
    VkResult result = DispatchCreateSamplerYcbcrConversion(device, pCreateInfo, pAllocator, pYcbcrConversion);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateSamplerYcbcrConversion]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCreateSamplerYcbcrConversion(device, pCreateInfo, pAllocator, pYcbcrConversion, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroySamplerYcbcrConversion(
    VkDevice                                    device,
    VkSamplerYcbcrConversion                    ycbcrConversion,
    const VkAllocationCallbacks*                pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDestroySamplerYcbcrConversion]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateDestroySamplerYcbcrConversion(device, ycbcrConversion, pAllocator);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroySamplerYcbcrConversion]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDestroySamplerYcbcrConversion(device, ycbcrConversion, pAllocator);
    }
    DispatchDestroySamplerYcbcrConversion(device, ycbcrConversion, pAllocator);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroySamplerYcbcrConversion]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordDestroySamplerYcbcrConversion(device, ycbcrConversion, pAllocator);
    }
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceExternalBufferProperties(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalBufferInfo*   pExternalBufferInfo,
    VkExternalBufferProperties*                 pExternalBufferProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceExternalBufferProperties(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
        if (skip) return;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceExternalBufferProperties(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
    }
    DispatchGetPhysicalDeviceExternalBufferProperties(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetPhysicalDeviceExternalBufferProperties(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
    }
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceExternalFenceProperties(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalFenceInfo*    pExternalFenceInfo,
    VkExternalFenceProperties*                  pExternalFenceProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceExternalFenceProperties(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
        if (skip) return;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceExternalFenceProperties(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
    }
    DispatchGetPhysicalDeviceExternalFenceProperties(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetPhysicalDeviceExternalFenceProperties(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
    }
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceExternalSemaphoreProperties(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo,
    VkExternalSemaphoreProperties*              pExternalSemaphoreProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceExternalSemaphoreProperties(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
        if (skip) return;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceExternalSemaphoreProperties(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
    }
    DispatchGetPhysicalDeviceExternalSemaphoreProperties(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetPhysicalDeviceExternalSemaphoreProperties(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
    }
}

VKAPI_ATTR void VKAPI_CALL GetDescriptorSetLayoutSupport(
    VkDevice                                    device,
    const VkDescriptorSetLayoutCreateInfo*      pCreateInfo,
    VkDescriptorSetLayoutSupport*               pSupport) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetDescriptorSetLayoutSupport]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetDescriptorSetLayoutSupport(device, pCreateInfo, pSupport);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetDescriptorSetLayoutSupport]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetDescriptorSetLayoutSupport(device, pCreateInfo, pSupport);
    }
    DispatchGetDescriptorSetLayoutSupport(device, pCreateInfo, pSupport);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetDescriptorSetLayoutSupport]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetDescriptorSetLayoutSupport(device, pCreateInfo, pSupport);
    }
}


VKAPI_ATTR void VKAPI_CALL CmdDrawIndirectCount(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdDrawIndirectCount]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdDrawIndirectCount]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    }
    DispatchCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdDrawIndirectCount]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdDrawIndexedIndirectCount(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdDrawIndexedIndirectCount]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdDrawIndexedIndirectCount]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    }
    DispatchCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdDrawIndexedIndirectCount]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL CreateRenderPass2(
    VkDevice                                    device,
    const VkRenderPassCreateInfo2*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkRenderPass*                               pRenderPass) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCreateRenderPass2]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateRenderPass2(device, pCreateInfo, pAllocator, pRenderPass);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateRenderPass2]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateRenderPass2(device, pCreateInfo, pAllocator, pRenderPass);
    }
    VkResult result = DispatchCreateRenderPass2(device, pCreateInfo, pAllocator, pRenderPass);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateRenderPass2]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCreateRenderPass2(device, pCreateInfo, pAllocator, pRenderPass, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL CmdBeginRenderPass2(
    VkCommandBuffer                             commandBuffer,
    const VkRenderPassBeginInfo*                pRenderPassBegin,
    const VkSubpassBeginInfo*                   pSubpassBeginInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdBeginRenderPass2]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdBeginRenderPass2]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
    }
    DispatchCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdBeginRenderPass2]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdNextSubpass2(
    VkCommandBuffer                             commandBuffer,
    const VkSubpassBeginInfo*                   pSubpassBeginInfo,
    const VkSubpassEndInfo*                     pSubpassEndInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdNextSubpass2]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdNextSubpass2(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdNextSubpass2]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdNextSubpass2(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
    }
    DispatchCmdNextSubpass2(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdNextSubpass2]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdNextSubpass2(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdEndRenderPass2(
    VkCommandBuffer                             commandBuffer,
    const VkSubpassEndInfo*                     pSubpassEndInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdEndRenderPass2]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdEndRenderPass2(commandBuffer, pSubpassEndInfo);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdEndRenderPass2]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdEndRenderPass2(commandBuffer, pSubpassEndInfo);
    }
    DispatchCmdEndRenderPass2(commandBuffer, pSubpassEndInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdEndRenderPass2]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdEndRenderPass2(commandBuffer, pSubpassEndInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL ResetQueryPool(
    VkDevice                                    device,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateResetQueryPool]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateResetQueryPool(device, queryPool, firstQuery, queryCount);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordResetQueryPool]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordResetQueryPool(device, queryPool, firstQuery, queryCount);
    }
    DispatchResetQueryPool(device, queryPool, firstQuery, queryCount);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordResetQueryPool]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordResetQueryPool(device, queryPool, firstQuery, queryCount);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL GetSemaphoreCounterValue(
    VkDevice                                    device,
    VkSemaphore                                 semaphore,
    uint64_t*                                   pValue) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetSemaphoreCounterValue]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetSemaphoreCounterValue(device, semaphore, pValue);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetSemaphoreCounterValue]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetSemaphoreCounterValue(device, semaphore, pValue);
    }
    VkResult result = DispatchGetSemaphoreCounterValue(device, semaphore, pValue);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetSemaphoreCounterValue]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetSemaphoreCounterValue(device, semaphore, pValue, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL WaitSemaphores(
    VkDevice                                    device,
    const VkSemaphoreWaitInfo*                  pWaitInfo,
    uint64_t                                    timeout) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateWaitSemaphores]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateWaitSemaphores(device, pWaitInfo, timeout);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordWaitSemaphores]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordWaitSemaphores(device, pWaitInfo, timeout);
    }
    VkResult result = DispatchWaitSemaphores(device, pWaitInfo, timeout);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordWaitSemaphores]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordWaitSemaphores(device, pWaitInfo, timeout, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL SignalSemaphore(
    VkDevice                                    device,
    const VkSemaphoreSignalInfo*                pSignalInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateSignalSemaphore]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateSignalSemaphore(device, pSignalInfo);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordSignalSemaphore]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordSignalSemaphore(device, pSignalInfo);
    }
    VkResult result = DispatchSignalSemaphore(device, pSignalInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordSignalSemaphore]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordSignalSemaphore(device, pSignalInfo, result);
    }
    return result;
}

VKAPI_ATTR VkDeviceAddress VKAPI_CALL GetBufferDeviceAddress(
    VkDevice                                    device,
    const VkBufferDeviceAddressInfo*            pInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetBufferDeviceAddress]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetBufferDeviceAddress(device, pInfo);
        if (skip) return 0;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetBufferDeviceAddress]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetBufferDeviceAddress(device, pInfo);
    }
    VkDeviceAddress result = DispatchGetBufferDeviceAddress(device, pInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetBufferDeviceAddress]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetBufferDeviceAddress(device, pInfo, result);
    }
    return result;
}

VKAPI_ATTR uint64_t VKAPI_CALL GetBufferOpaqueCaptureAddress(
    VkDevice                                    device,
    const VkBufferDeviceAddressInfo*            pInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetBufferOpaqueCaptureAddress]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetBufferOpaqueCaptureAddress(device, pInfo);
        if (skip) return 0;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetBufferOpaqueCaptureAddress]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetBufferOpaqueCaptureAddress(device, pInfo);
    }
    uint64_t result = DispatchGetBufferOpaqueCaptureAddress(device, pInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetBufferOpaqueCaptureAddress]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetBufferOpaqueCaptureAddress(device, pInfo);
    }
    return result;
}

VKAPI_ATTR uint64_t VKAPI_CALL GetDeviceMemoryOpaqueCaptureAddress(
    VkDevice                                    device,
    const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetDeviceMemoryOpaqueCaptureAddress]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetDeviceMemoryOpaqueCaptureAddress(device, pInfo);
        if (skip) return 0;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetDeviceMemoryOpaqueCaptureAddress]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetDeviceMemoryOpaqueCaptureAddress(device, pInfo);
    }
    uint64_t result = DispatchGetDeviceMemoryOpaqueCaptureAddress(device, pInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetDeviceMemoryOpaqueCaptureAddress]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetDeviceMemoryOpaqueCaptureAddress(device, pInfo);
    }
    return result;
}


VKAPI_ATTR void VKAPI_CALL GetCommandPoolMemoryConsumption(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    VkCommandBuffer                             commandBuffer,
    VkCommandPoolMemoryConsumption*             pConsumption) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetCommandPoolMemoryConsumption]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetCommandPoolMemoryConsumption(device, commandPool, commandBuffer, pConsumption);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetCommandPoolMemoryConsumption]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetCommandPoolMemoryConsumption(device, commandPool, commandBuffer, pConsumption);
    }
    DispatchGetCommandPoolMemoryConsumption(device, commandPool, commandBuffer, pConsumption);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetCommandPoolMemoryConsumption]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetCommandPoolMemoryConsumption(device, commandPool, commandBuffer, pConsumption);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL GetFaultData(
    VkDevice                                    device,
    VkFaultQueryBehavior                        faultQueryBehavior,
    VkBool32*                                   pUnrecordedFaults,
    uint32_t*                                   pFaultCount,
    VkFaultData*                                pFaults) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetFaultData]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetFaultData(device, faultQueryBehavior, pUnrecordedFaults, pFaultCount, pFaults);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetFaultData]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetFaultData(device, faultQueryBehavior, pUnrecordedFaults, pFaultCount, pFaults);
    }
    VkResult result = DispatchGetFaultData(device, faultQueryBehavior, pUnrecordedFaults, pFaultCount, pFaults);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetFaultData]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetFaultData(device, faultQueryBehavior, pUnrecordedFaults, pFaultCount, pFaults, result);
    }
    return result;
}


VKAPI_ATTR void VKAPI_CALL DestroySurfaceKHR(
    VkInstance                                  instance,
    VkSurfaceKHR                                surface,
    const VkAllocationCallbacks*                pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateDestroySurfaceKHR(instance, surface, pAllocator);
        if (skip) return;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDestroySurfaceKHR(instance, surface, pAllocator);
    }
    DispatchDestroySurfaceKHR(instance, surface, pAllocator);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordDestroySurfaceKHR(instance, surface, pAllocator);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceSurfaceSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    VkSurfaceKHR                                surface,
    VkBool32*                                   pSupported) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, pSupported);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, pSupported);
    }
    VkResult result = DispatchGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, pSupported);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, pSupported, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceSurfaceCapabilitiesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    VkSurfaceCapabilitiesKHR*                   pSurfaceCapabilities) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, pSurfaceCapabilities);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, pSurfaceCapabilities);
    }
    VkResult result = DispatchGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, pSurfaceCapabilities);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, pSurfaceCapabilities, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceSurfaceFormatsKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    uint32_t*                                   pSurfaceFormatCount,
    VkSurfaceFormatKHR*                         pSurfaceFormats) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, pSurfaceFormatCount, pSurfaceFormats);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, pSurfaceFormatCount, pSurfaceFormats);
    }
    VkResult result = DispatchGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, pSurfaceFormatCount, pSurfaceFormats);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, pSurfaceFormatCount, pSurfaceFormats, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceSurfacePresentModesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    uint32_t*                                   pPresentModeCount,
    VkPresentModeKHR*                           pPresentModes) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, pPresentModeCount, pPresentModes);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, pPresentModeCount, pPresentModes);
    }
    VkResult result = DispatchGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, pPresentModeCount, pPresentModes);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, pPresentModeCount, pPresentModes, result);
    }
    return result;
}


VKAPI_ATTR VkResult VKAPI_CALL CreateSwapchainKHR(
    VkDevice                                    device,
    const VkSwapchainCreateInfoKHR*             pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSwapchainKHR*                             pSwapchain) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCreateSwapchainKHR]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateSwapchainKHR(device, pCreateInfo, pAllocator, pSwapchain);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateSwapchainKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateSwapchainKHR(device, pCreateInfo, pAllocator, pSwapchain);
    }
    VkResult result = DispatchCreateSwapchainKHR(device, pCreateInfo, pAllocator, pSwapchain);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateSwapchainKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCreateSwapchainKHR(device, pCreateInfo, pAllocator, pSwapchain, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetSwapchainImagesKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    uint32_t*                                   pSwapchainImageCount,
    VkImage*                                    pSwapchainImages) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetSwapchainImagesKHR]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetSwapchainImagesKHR(device, swapchain, pSwapchainImageCount, pSwapchainImages);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetSwapchainImagesKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetSwapchainImagesKHR(device, swapchain, pSwapchainImageCount, pSwapchainImages);
    }
    VkResult result = DispatchGetSwapchainImagesKHR(device, swapchain, pSwapchainImageCount, pSwapchainImages);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetSwapchainImagesKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetSwapchainImagesKHR(device, swapchain, pSwapchainImageCount, pSwapchainImages, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL AcquireNextImageKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    uint64_t                                    timeout,
    VkSemaphore                                 semaphore,
    VkFence                                     fence,
    uint32_t*                                   pImageIndex) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateAcquireNextImageKHR]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateAcquireNextImageKHR(device, swapchain, timeout, semaphore, fence, pImageIndex);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordAcquireNextImageKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordAcquireNextImageKHR(device, swapchain, timeout, semaphore, fence, pImageIndex);
    }
    VkResult result = DispatchAcquireNextImageKHR(device, swapchain, timeout, semaphore, fence, pImageIndex);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordAcquireNextImageKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordAcquireNextImageKHR(device, swapchain, timeout, semaphore, fence, pImageIndex, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL QueuePresentKHR(
    VkQueue                                     queue,
    const VkPresentInfoKHR*                     pPresentInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(queue), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateQueuePresentKHR]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateQueuePresentKHR(queue, pPresentInfo);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordQueuePresentKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordQueuePresentKHR(queue, pPresentInfo);
    }
    VkResult result = DispatchQueuePresentKHR(queue, pPresentInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordQueuePresentKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordQueuePresentKHR(queue, pPresentInfo, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetDeviceGroupPresentCapabilitiesKHR(
    VkDevice                                    device,
    VkDeviceGroupPresentCapabilitiesKHR*        pDeviceGroupPresentCapabilities) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetDeviceGroupPresentCapabilitiesKHR]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetDeviceGroupPresentCapabilitiesKHR(device, pDeviceGroupPresentCapabilities);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetDeviceGroupPresentCapabilitiesKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetDeviceGroupPresentCapabilitiesKHR(device, pDeviceGroupPresentCapabilities);
    }
    VkResult result = DispatchGetDeviceGroupPresentCapabilitiesKHR(device, pDeviceGroupPresentCapabilities);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetDeviceGroupPresentCapabilitiesKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetDeviceGroupPresentCapabilitiesKHR(device, pDeviceGroupPresentCapabilities, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetDeviceGroupSurfacePresentModesKHR(
    VkDevice                                    device,
    VkSurfaceKHR                                surface,
    VkDeviceGroupPresentModeFlagsKHR*           pModes) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetDeviceGroupSurfacePresentModesKHR]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetDeviceGroupSurfacePresentModesKHR(device, surface, pModes);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetDeviceGroupSurfacePresentModesKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetDeviceGroupSurfacePresentModesKHR(device, surface, pModes);
    }
    VkResult result = DispatchGetDeviceGroupSurfacePresentModesKHR(device, surface, pModes);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetDeviceGroupSurfacePresentModesKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetDeviceGroupSurfacePresentModesKHR(device, surface, pModes, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDevicePresentRectanglesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    uint32_t*                                   pRectCount,
    VkRect2D*                                   pRects) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDevicePresentRectanglesKHR(physicalDevice, surface, pRectCount, pRects);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDevicePresentRectanglesKHR(physicalDevice, surface, pRectCount, pRects);
    }
    VkResult result = DispatchGetPhysicalDevicePresentRectanglesKHR(physicalDevice, surface, pRectCount, pRects);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetPhysicalDevicePresentRectanglesKHR(physicalDevice, surface, pRectCount, pRects, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL AcquireNextImage2KHR(
    VkDevice                                    device,
    const VkAcquireNextImageInfoKHR*            pAcquireInfo,
    uint32_t*                                   pImageIndex) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateAcquireNextImage2KHR]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateAcquireNextImage2KHR(device, pAcquireInfo, pImageIndex);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordAcquireNextImage2KHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordAcquireNextImage2KHR(device, pAcquireInfo, pImageIndex);
    }
    VkResult result = DispatchAcquireNextImage2KHR(device, pAcquireInfo, pImageIndex);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordAcquireNextImage2KHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordAcquireNextImage2KHR(device, pAcquireInfo, pImageIndex, result);
    }
    return result;
}


VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceDisplayPropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkDisplayPropertiesKHR*                     pProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceDisplayPropertiesKHR(physicalDevice, pPropertyCount, pProperties);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceDisplayPropertiesKHR(physicalDevice, pPropertyCount, pProperties);
    }
    VkResult result = DispatchGetPhysicalDeviceDisplayPropertiesKHR(physicalDevice, pPropertyCount, pProperties);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetPhysicalDeviceDisplayPropertiesKHR(physicalDevice, pPropertyCount, pProperties, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceDisplayPlanePropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkDisplayPlanePropertiesKHR*                pProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceDisplayPlanePropertiesKHR(physicalDevice, pPropertyCount, pProperties);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceDisplayPlanePropertiesKHR(physicalDevice, pPropertyCount, pProperties);
    }
    VkResult result = DispatchGetPhysicalDeviceDisplayPlanePropertiesKHR(physicalDevice, pPropertyCount, pProperties);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetPhysicalDeviceDisplayPlanePropertiesKHR(physicalDevice, pPropertyCount, pProperties, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetDisplayPlaneSupportedDisplaysKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    planeIndex,
    uint32_t*                                   pDisplayCount,
    VkDisplayKHR*                               pDisplays) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetDisplayPlaneSupportedDisplaysKHR(physicalDevice, planeIndex, pDisplayCount, pDisplays);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetDisplayPlaneSupportedDisplaysKHR(physicalDevice, planeIndex, pDisplayCount, pDisplays);
    }
    VkResult result = DispatchGetDisplayPlaneSupportedDisplaysKHR(physicalDevice, planeIndex, pDisplayCount, pDisplays);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetDisplayPlaneSupportedDisplaysKHR(physicalDevice, planeIndex, pDisplayCount, pDisplays, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetDisplayModePropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display,
    uint32_t*                                   pPropertyCount,
    VkDisplayModePropertiesKHR*                 pProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetDisplayModePropertiesKHR(physicalDevice, display, pPropertyCount, pProperties);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetDisplayModePropertiesKHR(physicalDevice, display, pPropertyCount, pProperties);
    }
    VkResult result = DispatchGetDisplayModePropertiesKHR(physicalDevice, display, pPropertyCount, pProperties);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetDisplayModePropertiesKHR(physicalDevice, display, pPropertyCount, pProperties, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL CreateDisplayModeKHR(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display,
    const VkDisplayModeCreateInfoKHR*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDisplayModeKHR*                           pMode) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateDisplayModeKHR(physicalDevice, display, pCreateInfo, pAllocator, pMode);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateDisplayModeKHR(physicalDevice, display, pCreateInfo, pAllocator, pMode);
    }
    VkResult result = DispatchCreateDisplayModeKHR(physicalDevice, display, pCreateInfo, pAllocator, pMode);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCreateDisplayModeKHR(physicalDevice, display, pCreateInfo, pAllocator, pMode, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetDisplayPlaneCapabilitiesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayModeKHR                            mode,
    uint32_t                                    planeIndex,
    VkDisplayPlaneCapabilitiesKHR*              pCapabilities) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetDisplayPlaneCapabilitiesKHR(physicalDevice, mode, planeIndex, pCapabilities);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetDisplayPlaneCapabilitiesKHR(physicalDevice, mode, planeIndex, pCapabilities);
    }
    VkResult result = DispatchGetDisplayPlaneCapabilitiesKHR(physicalDevice, mode, planeIndex, pCapabilities);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetDisplayPlaneCapabilitiesKHR(physicalDevice, mode, planeIndex, pCapabilities, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL CreateDisplayPlaneSurfaceKHR(
    VkInstance                                  instance,
    const VkDisplaySurfaceCreateInfoKHR*        pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateDisplayPlaneSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateDisplayPlaneSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    }
    VkResult result = DispatchCreateDisplayPlaneSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCreateDisplayPlaneSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface, result);
    }
    return result;
}


VKAPI_ATTR VkResult VKAPI_CALL CreateSharedSwapchainsKHR(
    VkDevice                                    device,
    uint32_t                                    swapchainCount,
    const VkSwapchainCreateInfoKHR*             pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkSwapchainKHR*                             pSwapchains) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCreateSharedSwapchainsKHR]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateSharedSwapchainsKHR(device, swapchainCount, pCreateInfos, pAllocator, pSwapchains);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateSharedSwapchainsKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateSharedSwapchainsKHR(device, swapchainCount, pCreateInfos, pAllocator, pSwapchains);
    }
    VkResult result = DispatchCreateSharedSwapchainsKHR(device, swapchainCount, pCreateInfos, pAllocator, pSwapchains);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateSharedSwapchainsKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCreateSharedSwapchainsKHR(device, swapchainCount, pCreateInfos, pAllocator, pSwapchains, result);
    }
    return result;
}


VKAPI_ATTR VkResult VKAPI_CALL GetMemoryFdKHR(
    VkDevice                                    device,
    const VkMemoryGetFdInfoKHR*                 pGetFdInfo,
    int*                                        pFd) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetMemoryFdKHR]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetMemoryFdKHR(device, pGetFdInfo, pFd);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetMemoryFdKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetMemoryFdKHR(device, pGetFdInfo, pFd);
    }
    VkResult result = DispatchGetMemoryFdKHR(device, pGetFdInfo, pFd);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetMemoryFdKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetMemoryFdKHR(device, pGetFdInfo, pFd, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetMemoryFdPropertiesKHR(
    VkDevice                                    device,
    VkExternalMemoryHandleTypeFlagBits          handleType,
    int                                         fd,
    VkMemoryFdPropertiesKHR*                    pMemoryFdProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetMemoryFdPropertiesKHR]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetMemoryFdPropertiesKHR(device, handleType, fd, pMemoryFdProperties);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetMemoryFdPropertiesKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetMemoryFdPropertiesKHR(device, handleType, fd, pMemoryFdProperties);
    }
    VkResult result = DispatchGetMemoryFdPropertiesKHR(device, handleType, fd, pMemoryFdProperties);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetMemoryFdPropertiesKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetMemoryFdPropertiesKHR(device, handleType, fd, pMemoryFdProperties, result);
    }
    return result;
}


VKAPI_ATTR VkResult VKAPI_CALL ImportSemaphoreFdKHR(
    VkDevice                                    device,
    const VkImportSemaphoreFdInfoKHR*           pImportSemaphoreFdInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateImportSemaphoreFdKHR]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateImportSemaphoreFdKHR(device, pImportSemaphoreFdInfo);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordImportSemaphoreFdKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordImportSemaphoreFdKHR(device, pImportSemaphoreFdInfo);
    }
    VkResult result = DispatchImportSemaphoreFdKHR(device, pImportSemaphoreFdInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordImportSemaphoreFdKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordImportSemaphoreFdKHR(device, pImportSemaphoreFdInfo, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetSemaphoreFdKHR(
    VkDevice                                    device,
    const VkSemaphoreGetFdInfoKHR*              pGetFdInfo,
    int*                                        pFd) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetSemaphoreFdKHR]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetSemaphoreFdKHR(device, pGetFdInfo, pFd);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetSemaphoreFdKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetSemaphoreFdKHR(device, pGetFdInfo, pFd);
    }
    VkResult result = DispatchGetSemaphoreFdKHR(device, pGetFdInfo, pFd);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetSemaphoreFdKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetSemaphoreFdKHR(device, pGetFdInfo, pFd, result);
    }
    return result;
}



VKAPI_ATTR VkResult VKAPI_CALL GetSwapchainStatusKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetSwapchainStatusKHR]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetSwapchainStatusKHR(device, swapchain);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetSwapchainStatusKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetSwapchainStatusKHR(device, swapchain);
    }
    VkResult result = DispatchGetSwapchainStatusKHR(device, swapchain);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetSwapchainStatusKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetSwapchainStatusKHR(device, swapchain, result);
    }
    return result;
}


VKAPI_ATTR VkResult VKAPI_CALL ImportFenceFdKHR(
    VkDevice                                    device,
    const VkImportFenceFdInfoKHR*               pImportFenceFdInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateImportFenceFdKHR]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateImportFenceFdKHR(device, pImportFenceFdInfo);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordImportFenceFdKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordImportFenceFdKHR(device, pImportFenceFdInfo);
    }
    VkResult result = DispatchImportFenceFdKHR(device, pImportFenceFdInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordImportFenceFdKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordImportFenceFdKHR(device, pImportFenceFdInfo, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetFenceFdKHR(
    VkDevice                                    device,
    const VkFenceGetFdInfoKHR*                  pGetFdInfo,
    int*                                        pFd) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetFenceFdKHR]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetFenceFdKHR(device, pGetFdInfo, pFd);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetFenceFdKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetFenceFdKHR(device, pGetFdInfo, pFd);
    }
    VkResult result = DispatchGetFenceFdKHR(device, pGetFdInfo, pFd);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetFenceFdKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetFenceFdKHR(device, pGetFdInfo, pFd, result);
    }
    return result;
}


VKAPI_ATTR VkResult VKAPI_CALL EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    uint32_t*                                   pCounterCount,
    VkPerformanceCounterKHR*                    pCounters,
    VkPerformanceCounterDescriptionKHR*         pCounterDescriptions) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(physicalDevice, queueFamilyIndex, pCounterCount, pCounters, pCounterDescriptions);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(physicalDevice, queueFamilyIndex, pCounterCount, pCounters, pCounterDescriptions);
    }
    VkResult result = DispatchEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(physicalDevice, queueFamilyIndex, pCounterCount, pCounters, pCounterDescriptions);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(physicalDevice, queueFamilyIndex, pCounterCount, pCounters, pCounterDescriptions, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkQueryPoolPerformanceCreateInfoKHR*  pPerformanceQueryCreateInfo,
    uint32_t*                                   pNumPasses) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(physicalDevice, pPerformanceQueryCreateInfo, pNumPasses);
        if (skip) return;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(physicalDevice, pPerformanceQueryCreateInfo, pNumPasses);
    }
    DispatchGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(physicalDevice, pPerformanceQueryCreateInfo, pNumPasses);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(physicalDevice, pPerformanceQueryCreateInfo, pNumPasses);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL AcquireProfilingLockKHR(
    VkDevice                                    device,
    const VkAcquireProfilingLockInfoKHR*        pInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateAcquireProfilingLockKHR]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateAcquireProfilingLockKHR(device, pInfo);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordAcquireProfilingLockKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordAcquireProfilingLockKHR(device, pInfo);
    }
    VkResult result = DispatchAcquireProfilingLockKHR(device, pInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordAcquireProfilingLockKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordAcquireProfilingLockKHR(device, pInfo, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL ReleaseProfilingLockKHR(
    VkDevice                                    device) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateReleaseProfilingLockKHR]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateReleaseProfilingLockKHR(device);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordReleaseProfilingLockKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordReleaseProfilingLockKHR(device);
    }
    DispatchReleaseProfilingLockKHR(device);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordReleaseProfilingLockKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordReleaseProfilingLockKHR(device);
    }
}


VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceSurfaceCapabilities2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSurfaceInfo2KHR*      pSurfaceInfo,
    VkSurfaceCapabilities2KHR*                  pSurfaceCapabilities) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceSurfaceCapabilities2KHR(physicalDevice, pSurfaceInfo, pSurfaceCapabilities);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceSurfaceCapabilities2KHR(physicalDevice, pSurfaceInfo, pSurfaceCapabilities);
    }
    VkResult result = DispatchGetPhysicalDeviceSurfaceCapabilities2KHR(physicalDevice, pSurfaceInfo, pSurfaceCapabilities);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetPhysicalDeviceSurfaceCapabilities2KHR(physicalDevice, pSurfaceInfo, pSurfaceCapabilities, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceSurfaceFormats2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSurfaceInfo2KHR*      pSurfaceInfo,
    uint32_t*                                   pSurfaceFormatCount,
    VkSurfaceFormat2KHR*                        pSurfaceFormats) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceSurfaceFormats2KHR(physicalDevice, pSurfaceInfo, pSurfaceFormatCount, pSurfaceFormats);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceSurfaceFormats2KHR(physicalDevice, pSurfaceInfo, pSurfaceFormatCount, pSurfaceFormats);
    }
    VkResult result = DispatchGetPhysicalDeviceSurfaceFormats2KHR(physicalDevice, pSurfaceInfo, pSurfaceFormatCount, pSurfaceFormats);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetPhysicalDeviceSurfaceFormats2KHR(physicalDevice, pSurfaceInfo, pSurfaceFormatCount, pSurfaceFormats, result);
    }
    return result;
}


VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceDisplayProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkDisplayProperties2KHR*                    pProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceDisplayProperties2KHR(physicalDevice, pPropertyCount, pProperties);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceDisplayProperties2KHR(physicalDevice, pPropertyCount, pProperties);
    }
    VkResult result = DispatchGetPhysicalDeviceDisplayProperties2KHR(physicalDevice, pPropertyCount, pProperties);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetPhysicalDeviceDisplayProperties2KHR(physicalDevice, pPropertyCount, pProperties, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceDisplayPlaneProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkDisplayPlaneProperties2KHR*               pProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceDisplayPlaneProperties2KHR(physicalDevice, pPropertyCount, pProperties);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceDisplayPlaneProperties2KHR(physicalDevice, pPropertyCount, pProperties);
    }
    VkResult result = DispatchGetPhysicalDeviceDisplayPlaneProperties2KHR(physicalDevice, pPropertyCount, pProperties);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetPhysicalDeviceDisplayPlaneProperties2KHR(physicalDevice, pPropertyCount, pProperties, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetDisplayModeProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display,
    uint32_t*                                   pPropertyCount,
    VkDisplayModeProperties2KHR*                pProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetDisplayModeProperties2KHR(physicalDevice, display, pPropertyCount, pProperties);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetDisplayModeProperties2KHR(physicalDevice, display, pPropertyCount, pProperties);
    }
    VkResult result = DispatchGetDisplayModeProperties2KHR(physicalDevice, display, pPropertyCount, pProperties);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetDisplayModeProperties2KHR(physicalDevice, display, pPropertyCount, pProperties, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetDisplayPlaneCapabilities2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkDisplayPlaneInfo2KHR*               pDisplayPlaneInfo,
    VkDisplayPlaneCapabilities2KHR*             pCapabilities) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetDisplayPlaneCapabilities2KHR(physicalDevice, pDisplayPlaneInfo, pCapabilities);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetDisplayPlaneCapabilities2KHR(physicalDevice, pDisplayPlaneInfo, pCapabilities);
    }
    VkResult result = DispatchGetDisplayPlaneCapabilities2KHR(physicalDevice, pDisplayPlaneInfo, pCapabilities);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetDisplayPlaneCapabilities2KHR(physicalDevice, pDisplayPlaneInfo, pCapabilities, result);
    }
    return result;
}





VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceFragmentShadingRatesKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pFragmentShadingRateCount,
    VkPhysicalDeviceFragmentShadingRateKHR*     pFragmentShadingRates) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceFragmentShadingRatesKHR(physicalDevice, pFragmentShadingRateCount, pFragmentShadingRates);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceFragmentShadingRatesKHR(physicalDevice, pFragmentShadingRateCount, pFragmentShadingRates);
    }
    VkResult result = DispatchGetPhysicalDeviceFragmentShadingRatesKHR(physicalDevice, pFragmentShadingRateCount, pFragmentShadingRates);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetPhysicalDeviceFragmentShadingRatesKHR(physicalDevice, pFragmentShadingRateCount, pFragmentShadingRates, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL CmdSetFragmentShadingRateKHR(
    VkCommandBuffer                             commandBuffer,
    const VkExtent2D*                           pFragmentSize,
    const VkFragmentShadingRateCombinerOpKHR    combinerOps[2]) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetFragmentShadingRateKHR]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetFragmentShadingRateKHR(commandBuffer, pFragmentSize, combinerOps);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetFragmentShadingRateKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetFragmentShadingRateKHR(commandBuffer, pFragmentSize, combinerOps);
    }
    DispatchCmdSetFragmentShadingRateKHR(commandBuffer, pFragmentSize, combinerOps);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetFragmentShadingRateKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetFragmentShadingRateKHR(commandBuffer, pFragmentSize, combinerOps);
    }
}


VKAPI_ATTR void VKAPI_CALL CmdRefreshObjectsKHR(
    VkCommandBuffer                             commandBuffer,
    const VkRefreshObjectListKHR*               pRefreshObjects) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdRefreshObjectsKHR]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdRefreshObjectsKHR(commandBuffer, pRefreshObjects);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdRefreshObjectsKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdRefreshObjectsKHR(commandBuffer, pRefreshObjects);
    }
    DispatchCmdRefreshObjectsKHR(commandBuffer, pRefreshObjects);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdRefreshObjectsKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdRefreshObjectsKHR(commandBuffer, pRefreshObjects);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceRefreshableObjectTypesKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pRefreshableObjectTypeCount,
    VkObjectType*                               pRefreshableObjectTypes) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceRefreshableObjectTypesKHR(physicalDevice, pRefreshableObjectTypeCount, pRefreshableObjectTypes);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceRefreshableObjectTypesKHR(physicalDevice, pRefreshableObjectTypeCount, pRefreshableObjectTypes);
    }
    VkResult result = DispatchGetPhysicalDeviceRefreshableObjectTypesKHR(physicalDevice, pRefreshableObjectTypeCount, pRefreshableObjectTypes);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetPhysicalDeviceRefreshableObjectTypesKHR(physicalDevice, pRefreshableObjectTypeCount, pRefreshableObjectTypes, result);
    }
    return result;
}


VKAPI_ATTR void VKAPI_CALL CmdSetEvent2KHR(
    VkCommandBuffer                             commandBuffer,
    VkEvent                                     event,
    const VkDependencyInfoKHR*                  pDependencyInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetEvent2KHR]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetEvent2KHR(commandBuffer, event, pDependencyInfo);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetEvent2KHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetEvent2KHR(commandBuffer, event, pDependencyInfo);
    }
    DispatchCmdSetEvent2KHR(commandBuffer, event, pDependencyInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetEvent2KHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetEvent2KHR(commandBuffer, event, pDependencyInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdResetEvent2KHR(
    VkCommandBuffer                             commandBuffer,
    VkEvent                                     event,
    VkPipelineStageFlags2KHR                    stageMask) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdResetEvent2KHR]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdResetEvent2KHR(commandBuffer, event, stageMask);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdResetEvent2KHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdResetEvent2KHR(commandBuffer, event, stageMask);
    }
    DispatchCmdResetEvent2KHR(commandBuffer, event, stageMask);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdResetEvent2KHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdResetEvent2KHR(commandBuffer, event, stageMask);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdWaitEvents2KHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    eventCount,
    const VkEvent*                              pEvents,
    const VkDependencyInfoKHR*                  pDependencyInfos) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdWaitEvents2KHR]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdWaitEvents2KHR(commandBuffer, eventCount, pEvents, pDependencyInfos);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdWaitEvents2KHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdWaitEvents2KHR(commandBuffer, eventCount, pEvents, pDependencyInfos);
    }
    DispatchCmdWaitEvents2KHR(commandBuffer, eventCount, pEvents, pDependencyInfos);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdWaitEvents2KHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdWaitEvents2KHR(commandBuffer, eventCount, pEvents, pDependencyInfos);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdPipelineBarrier2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkDependencyInfoKHR*                  pDependencyInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdPipelineBarrier2KHR]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdPipelineBarrier2KHR(commandBuffer, pDependencyInfo);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdPipelineBarrier2KHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdPipelineBarrier2KHR(commandBuffer, pDependencyInfo);
    }
    DispatchCmdPipelineBarrier2KHR(commandBuffer, pDependencyInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdPipelineBarrier2KHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdPipelineBarrier2KHR(commandBuffer, pDependencyInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdWriteTimestamp2KHR(
    VkCommandBuffer                             commandBuffer,
    VkPipelineStageFlags2KHR                    stage,
    VkQueryPool                                 queryPool,
    uint32_t                                    query) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdWriteTimestamp2KHR]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdWriteTimestamp2KHR(commandBuffer, stage, queryPool, query);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdWriteTimestamp2KHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdWriteTimestamp2KHR(commandBuffer, stage, queryPool, query);
    }
    DispatchCmdWriteTimestamp2KHR(commandBuffer, stage, queryPool, query);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdWriteTimestamp2KHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdWriteTimestamp2KHR(commandBuffer, stage, queryPool, query);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL QueueSubmit2KHR(
    VkQueue                                     queue,
    uint32_t                                    submitCount,
    const VkSubmitInfo2KHR*                     pSubmits,
    VkFence                                     fence) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(queue), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateQueueSubmit2KHR]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateQueueSubmit2KHR(queue, submitCount, pSubmits, fence);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordQueueSubmit2KHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordQueueSubmit2KHR(queue, submitCount, pSubmits, fence);
    }
    VkResult result = DispatchQueueSubmit2KHR(queue, submitCount, pSubmits, fence);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordQueueSubmit2KHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordQueueSubmit2KHR(queue, submitCount, pSubmits, fence, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL CmdWriteBufferMarker2AMD(
    VkCommandBuffer                             commandBuffer,
    VkPipelineStageFlags2KHR                    stage,
    VkBuffer                                    dstBuffer,
    VkDeviceSize                                dstOffset,
    uint32_t                                    marker) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdWriteBufferMarker2AMD]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdWriteBufferMarker2AMD(commandBuffer, stage, dstBuffer, dstOffset, marker);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdWriteBufferMarker2AMD]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdWriteBufferMarker2AMD(commandBuffer, stage, dstBuffer, dstOffset, marker);
    }
    DispatchCmdWriteBufferMarker2AMD(commandBuffer, stage, dstBuffer, dstOffset, marker);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdWriteBufferMarker2AMD]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdWriteBufferMarker2AMD(commandBuffer, stage, dstBuffer, dstOffset, marker);
    }
}

VKAPI_ATTR void VKAPI_CALL GetQueueCheckpointData2NV(
    VkQueue                                     queue,
    uint32_t*                                   pCheckpointDataCount,
    VkCheckpointData2NV*                        pCheckpointData) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(queue), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetQueueCheckpointData2NV]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetQueueCheckpointData2NV(queue, pCheckpointDataCount, pCheckpointData);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetQueueCheckpointData2NV]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetQueueCheckpointData2NV(queue, pCheckpointDataCount, pCheckpointData);
    }
    DispatchGetQueueCheckpointData2NV(queue, pCheckpointDataCount, pCheckpointData);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetQueueCheckpointData2NV]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetQueueCheckpointData2NV(queue, pCheckpointDataCount, pCheckpointData);
    }
}


VKAPI_ATTR void VKAPI_CALL CmdCopyBuffer2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyBufferInfo2KHR*                 pCopyBufferInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdCopyBuffer2KHR]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdCopyBuffer2KHR(commandBuffer, pCopyBufferInfo);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdCopyBuffer2KHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdCopyBuffer2KHR(commandBuffer, pCopyBufferInfo);
    }
    DispatchCmdCopyBuffer2KHR(commandBuffer, pCopyBufferInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdCopyBuffer2KHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdCopyBuffer2KHR(commandBuffer, pCopyBufferInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdCopyImage2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyImageInfo2KHR*                  pCopyImageInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdCopyImage2KHR]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdCopyImage2KHR(commandBuffer, pCopyImageInfo);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdCopyImage2KHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdCopyImage2KHR(commandBuffer, pCopyImageInfo);
    }
    DispatchCmdCopyImage2KHR(commandBuffer, pCopyImageInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdCopyImage2KHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdCopyImage2KHR(commandBuffer, pCopyImageInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdCopyBufferToImage2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyBufferToImageInfo2KHR*          pCopyBufferToImageInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdCopyBufferToImage2KHR]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdCopyBufferToImage2KHR(commandBuffer, pCopyBufferToImageInfo);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdCopyBufferToImage2KHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdCopyBufferToImage2KHR(commandBuffer, pCopyBufferToImageInfo);
    }
    DispatchCmdCopyBufferToImage2KHR(commandBuffer, pCopyBufferToImageInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdCopyBufferToImage2KHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdCopyBufferToImage2KHR(commandBuffer, pCopyBufferToImageInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdCopyImageToBuffer2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyImageToBufferInfo2KHR*          pCopyImageToBufferInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdCopyImageToBuffer2KHR]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdCopyImageToBuffer2KHR(commandBuffer, pCopyImageToBufferInfo);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdCopyImageToBuffer2KHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdCopyImageToBuffer2KHR(commandBuffer, pCopyImageToBufferInfo);
    }
    DispatchCmdCopyImageToBuffer2KHR(commandBuffer, pCopyImageToBufferInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdCopyImageToBuffer2KHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdCopyImageToBuffer2KHR(commandBuffer, pCopyImageToBufferInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdBlitImage2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkBlitImageInfo2KHR*                  pBlitImageInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdBlitImage2KHR]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdBlitImage2KHR(commandBuffer, pBlitImageInfo);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdBlitImage2KHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdBlitImage2KHR(commandBuffer, pBlitImageInfo);
    }
    DispatchCmdBlitImage2KHR(commandBuffer, pBlitImageInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdBlitImage2KHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdBlitImage2KHR(commandBuffer, pBlitImageInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdResolveImage2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkResolveImageInfo2KHR*               pResolveImageInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdResolveImage2KHR]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdResolveImage2KHR(commandBuffer, pResolveImageInfo);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdResolveImage2KHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdResolveImage2KHR(commandBuffer, pResolveImageInfo);
    }
    DispatchCmdResolveImage2KHR(commandBuffer, pResolveImageInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdResolveImage2KHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdResolveImage2KHR(commandBuffer, pResolveImageInfo);
    }
}





VKAPI_ATTR VkResult VKAPI_CALL ReleaseDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateReleaseDisplayEXT(physicalDevice, display);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordReleaseDisplayEXT(physicalDevice, display);
    }
    VkResult result = DispatchReleaseDisplayEXT(physicalDevice, display);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordReleaseDisplayEXT(physicalDevice, display, result);
    }
    return result;
}


VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceSurfaceCapabilities2EXT(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    VkSurfaceCapabilities2EXT*                  pSurfaceCapabilities) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceSurfaceCapabilities2EXT(physicalDevice, surface, pSurfaceCapabilities);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceSurfaceCapabilities2EXT(physicalDevice, surface, pSurfaceCapabilities);
    }
    VkResult result = DispatchGetPhysicalDeviceSurfaceCapabilities2EXT(physicalDevice, surface, pSurfaceCapabilities);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetPhysicalDeviceSurfaceCapabilities2EXT(physicalDevice, surface, pSurfaceCapabilities, result);
    }
    return result;
}


VKAPI_ATTR VkResult VKAPI_CALL DisplayPowerControlEXT(
    VkDevice                                    device,
    VkDisplayKHR                                display,
    const VkDisplayPowerInfoEXT*                pDisplayPowerInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDisplayPowerControlEXT]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateDisplayPowerControlEXT(device, display, pDisplayPowerInfo);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDisplayPowerControlEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDisplayPowerControlEXT(device, display, pDisplayPowerInfo);
    }
    VkResult result = DispatchDisplayPowerControlEXT(device, display, pDisplayPowerInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDisplayPowerControlEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordDisplayPowerControlEXT(device, display, pDisplayPowerInfo, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL RegisterDeviceEventEXT(
    VkDevice                                    device,
    const VkDeviceEventInfoEXT*                 pDeviceEventInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFence*                                    pFence) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateRegisterDeviceEventEXT]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateRegisterDeviceEventEXT(device, pDeviceEventInfo, pAllocator, pFence);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordRegisterDeviceEventEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordRegisterDeviceEventEXT(device, pDeviceEventInfo, pAllocator, pFence);
    }
    VkResult result = DispatchRegisterDeviceEventEXT(device, pDeviceEventInfo, pAllocator, pFence);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordRegisterDeviceEventEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordRegisterDeviceEventEXT(device, pDeviceEventInfo, pAllocator, pFence, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL RegisterDisplayEventEXT(
    VkDevice                                    device,
    VkDisplayKHR                                display,
    const VkDisplayEventInfoEXT*                pDisplayEventInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFence*                                    pFence) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateRegisterDisplayEventEXT]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateRegisterDisplayEventEXT(device, display, pDisplayEventInfo, pAllocator, pFence);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordRegisterDisplayEventEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordRegisterDisplayEventEXT(device, display, pDisplayEventInfo, pAllocator, pFence);
    }
    VkResult result = DispatchRegisterDisplayEventEXT(device, display, pDisplayEventInfo, pAllocator, pFence);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordRegisterDisplayEventEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordRegisterDisplayEventEXT(device, display, pDisplayEventInfo, pAllocator, pFence, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetSwapchainCounterEXT(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    VkSurfaceCounterFlagBitsEXT                 counter,
    uint64_t*                                   pCounterValue) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetSwapchainCounterEXT]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetSwapchainCounterEXT(device, swapchain, counter, pCounterValue);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetSwapchainCounterEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetSwapchainCounterEXT(device, swapchain, counter, pCounterValue);
    }
    VkResult result = DispatchGetSwapchainCounterEXT(device, swapchain, counter, pCounterValue);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetSwapchainCounterEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetSwapchainCounterEXT(device, swapchain, counter, pCounterValue, result);
    }
    return result;
}


VKAPI_ATTR void VKAPI_CALL CmdSetDiscardRectangleEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstDiscardRectangle,
    uint32_t                                    discardRectangleCount,
    const VkRect2D*                             pDiscardRectangles) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetDiscardRectangleEXT]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetDiscardRectangleEXT(commandBuffer, firstDiscardRectangle, discardRectangleCount, pDiscardRectangles);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetDiscardRectangleEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetDiscardRectangleEXT(commandBuffer, firstDiscardRectangle, discardRectangleCount, pDiscardRectangles);
    }
    DispatchCmdSetDiscardRectangleEXT(commandBuffer, firstDiscardRectangle, discardRectangleCount, pDiscardRectangles);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetDiscardRectangleEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetDiscardRectangleEXT(commandBuffer, firstDiscardRectangle, discardRectangleCount, pDiscardRectangles);
    }
}





VKAPI_ATTR void VKAPI_CALL SetHdrMetadataEXT(
    VkDevice                                    device,
    uint32_t                                    swapchainCount,
    const VkSwapchainKHR*                       pSwapchains,
    const VkHdrMetadataEXT*                     pMetadata) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateSetHdrMetadataEXT]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateSetHdrMetadataEXT(device, swapchainCount, pSwapchains, pMetadata);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordSetHdrMetadataEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordSetHdrMetadataEXT(device, swapchainCount, pSwapchains, pMetadata);
    }
    DispatchSetHdrMetadataEXT(device, swapchainCount, pSwapchains, pMetadata);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordSetHdrMetadataEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordSetHdrMetadataEXT(device, swapchainCount, pSwapchains, pMetadata);
    }
}




VKAPI_ATTR VkResult VKAPI_CALL SetDebugUtilsObjectNameEXT(
    VkDevice                                    device,
    const VkDebugUtilsObjectNameInfoEXT*        pNameInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateSetDebugUtilsObjectNameEXT]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateSetDebugUtilsObjectNameEXT(device, pNameInfo);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordSetDebugUtilsObjectNameEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordSetDebugUtilsObjectNameEXT(device, pNameInfo);
    }
    layer_data->report_data->DebugReportSetUtilsObjectName(pNameInfo);
    VkResult result = DispatchSetDebugUtilsObjectNameEXT(device, pNameInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordSetDebugUtilsObjectNameEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordSetDebugUtilsObjectNameEXT(device, pNameInfo, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL SetDebugUtilsObjectTagEXT(
    VkDevice                                    device,
    const VkDebugUtilsObjectTagInfoEXT*         pTagInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateSetDebugUtilsObjectTagEXT]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateSetDebugUtilsObjectTagEXT(device, pTagInfo);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordSetDebugUtilsObjectTagEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordSetDebugUtilsObjectTagEXT(device, pTagInfo);
    }
    VkResult result = DispatchSetDebugUtilsObjectTagEXT(device, pTagInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordSetDebugUtilsObjectTagEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordSetDebugUtilsObjectTagEXT(device, pTagInfo, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL QueueBeginDebugUtilsLabelEXT(
    VkQueue                                     queue,
    const VkDebugUtilsLabelEXT*                 pLabelInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(queue), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateQueueBeginDebugUtilsLabelEXT]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateQueueBeginDebugUtilsLabelEXT(queue, pLabelInfo);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordQueueBeginDebugUtilsLabelEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordQueueBeginDebugUtilsLabelEXT(queue, pLabelInfo);
    }
    BeginQueueDebugUtilsLabel(layer_data->report_data, queue, pLabelInfo);
    DispatchQueueBeginDebugUtilsLabelEXT(queue, pLabelInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordQueueBeginDebugUtilsLabelEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordQueueBeginDebugUtilsLabelEXT(queue, pLabelInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL QueueEndDebugUtilsLabelEXT(
    VkQueue                                     queue) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(queue), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateQueueEndDebugUtilsLabelEXT]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateQueueEndDebugUtilsLabelEXT(queue);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordQueueEndDebugUtilsLabelEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordQueueEndDebugUtilsLabelEXT(queue);
    }
    DispatchQueueEndDebugUtilsLabelEXT(queue);
    EndQueueDebugUtilsLabel(layer_data->report_data, queue);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordQueueEndDebugUtilsLabelEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordQueueEndDebugUtilsLabelEXT(queue);
    }
}

VKAPI_ATTR void VKAPI_CALL QueueInsertDebugUtilsLabelEXT(
    VkQueue                                     queue,
    const VkDebugUtilsLabelEXT*                 pLabelInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(queue), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateQueueInsertDebugUtilsLabelEXT]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateQueueInsertDebugUtilsLabelEXT(queue, pLabelInfo);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordQueueInsertDebugUtilsLabelEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordQueueInsertDebugUtilsLabelEXT(queue, pLabelInfo);
    }
    InsertQueueDebugUtilsLabel(layer_data->report_data, queue, pLabelInfo);
    DispatchQueueInsertDebugUtilsLabelEXT(queue, pLabelInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordQueueInsertDebugUtilsLabelEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordQueueInsertDebugUtilsLabelEXT(queue, pLabelInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdBeginDebugUtilsLabelEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugUtilsLabelEXT*                 pLabelInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdBeginDebugUtilsLabelEXT]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdBeginDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdBeginDebugUtilsLabelEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdBeginDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
    }
    DispatchCmdBeginDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdBeginDebugUtilsLabelEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdBeginDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdEndDebugUtilsLabelEXT(
    VkCommandBuffer                             commandBuffer) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdEndDebugUtilsLabelEXT]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdEndDebugUtilsLabelEXT(commandBuffer);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdEndDebugUtilsLabelEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdEndDebugUtilsLabelEXT(commandBuffer);
    }
    DispatchCmdEndDebugUtilsLabelEXT(commandBuffer);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdEndDebugUtilsLabelEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdEndDebugUtilsLabelEXT(commandBuffer);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdInsertDebugUtilsLabelEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugUtilsLabelEXT*                 pLabelInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdInsertDebugUtilsLabelEXT]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdInsertDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdInsertDebugUtilsLabelEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdInsertDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
    }
    DispatchCmdInsertDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdInsertDebugUtilsLabelEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdInsertDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL CreateDebugUtilsMessengerEXT(
    VkInstance                                  instance,
    const VkDebugUtilsMessengerCreateInfoEXT*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDebugUtilsMessengerEXT*                   pMessenger) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);
    }
    VkResult result = DispatchCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);
    layer_create_messenger_callback(layer_data->report_data, false, pCreateInfo, pAllocator, pMessenger);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyDebugUtilsMessengerEXT(
    VkInstance                                  instance,
    VkDebugUtilsMessengerEXT                    messenger,
    const VkAllocationCallbacks*                pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
        if (skip) return;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
    }
    DispatchDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
    layer_destroy_callback(layer_data->report_data, messenger, pAllocator);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
    }
}

VKAPI_ATTR void VKAPI_CALL SubmitDebugUtilsMessageEXT(
    VkInstance                                  instance,
    VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT             messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateSubmitDebugUtilsMessageEXT(instance, messageSeverity, messageTypes, pCallbackData);
        if (skip) return;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordSubmitDebugUtilsMessageEXT(instance, messageSeverity, messageTypes, pCallbackData);
    }
    DispatchSubmitDebugUtilsMessageEXT(instance, messageSeverity, messageTypes, pCallbackData);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordSubmitDebugUtilsMessageEXT(instance, messageSeverity, messageTypes, pCallbackData);
    }
}



VKAPI_ATTR void VKAPI_CALL CmdSetSampleLocationsEXT(
    VkCommandBuffer                             commandBuffer,
    const VkSampleLocationsInfoEXT*             pSampleLocationsInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetSampleLocationsEXT]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetSampleLocationsEXT(commandBuffer, pSampleLocationsInfo);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetSampleLocationsEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetSampleLocationsEXT(commandBuffer, pSampleLocationsInfo);
    }
    DispatchCmdSetSampleLocationsEXT(commandBuffer, pSampleLocationsInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetSampleLocationsEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetSampleLocationsEXT(commandBuffer, pSampleLocationsInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceMultisamplePropertiesEXT(
    VkPhysicalDevice                            physicalDevice,
    VkSampleCountFlagBits                       samples,
    VkMultisamplePropertiesEXT*                 pMultisampleProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceMultisamplePropertiesEXT(physicalDevice, samples, pMultisampleProperties);
        if (skip) return;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceMultisamplePropertiesEXT(physicalDevice, samples, pMultisampleProperties);
    }
    DispatchGetPhysicalDeviceMultisamplePropertiesEXT(physicalDevice, samples, pMultisampleProperties);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetPhysicalDeviceMultisamplePropertiesEXT(physicalDevice, samples, pMultisampleProperties);
    }
}




VKAPI_ATTR VkResult VKAPI_CALL GetImageDrmFormatModifierPropertiesEXT(
    VkDevice                                    device,
    VkImage                                     image,
    VkImageDrmFormatModifierPropertiesEXT*      pProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetImageDrmFormatModifierPropertiesEXT]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetImageDrmFormatModifierPropertiesEXT(device, image, pProperties);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetImageDrmFormatModifierPropertiesEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetImageDrmFormatModifierPropertiesEXT(device, image, pProperties);
    }
    VkResult result = DispatchGetImageDrmFormatModifierPropertiesEXT(device, image, pProperties);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetImageDrmFormatModifierPropertiesEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetImageDrmFormatModifierPropertiesEXT(device, image, pProperties, result);
    }
    return result;
}




VKAPI_ATTR VkResult VKAPI_CALL GetMemoryHostPointerPropertiesEXT(
    VkDevice                                    device,
    VkExternalMemoryHandleTypeFlagBits          handleType,
    const void*                                 pHostPointer,
    VkMemoryHostPointerPropertiesEXT*           pMemoryHostPointerProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetMemoryHostPointerPropertiesEXT]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetMemoryHostPointerPropertiesEXT(device, handleType, pHostPointer, pMemoryHostPointerProperties);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetMemoryHostPointerPropertiesEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetMemoryHostPointerPropertiesEXT(device, handleType, pHostPointer, pMemoryHostPointerProperties);
    }
    VkResult result = DispatchGetMemoryHostPointerPropertiesEXT(device, handleType, pHostPointer, pMemoryHostPointerProperties);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetMemoryHostPointerPropertiesEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetMemoryHostPointerPropertiesEXT(device, handleType, pHostPointer, pMemoryHostPointerProperties, result);
    }
    return result;
}


VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceCalibrateableTimeDomainsEXT(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pTimeDomainCount,
    VkTimeDomainEXT*                            pTimeDomains) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceCalibrateableTimeDomainsEXT(physicalDevice, pTimeDomainCount, pTimeDomains);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceCalibrateableTimeDomainsEXT(physicalDevice, pTimeDomainCount, pTimeDomains);
    }
    VkResult result = DispatchGetPhysicalDeviceCalibrateableTimeDomainsEXT(physicalDevice, pTimeDomainCount, pTimeDomains);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetPhysicalDeviceCalibrateableTimeDomainsEXT(physicalDevice, pTimeDomainCount, pTimeDomains, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetCalibratedTimestampsEXT(
    VkDevice                                    device,
    uint32_t                                    timestampCount,
    const VkCalibratedTimestampInfoEXT*         pTimestampInfos,
    uint64_t*                                   pTimestamps,
    uint64_t*                                   pMaxDeviation) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetCalibratedTimestampsEXT]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetCalibratedTimestampsEXT(device, timestampCount, pTimestampInfos, pTimestamps, pMaxDeviation);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetCalibratedTimestampsEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetCalibratedTimestampsEXT(device, timestampCount, pTimestampInfos, pTimestamps, pMaxDeviation);
    }
    VkResult result = DispatchGetCalibratedTimestampsEXT(device, timestampCount, pTimestampInfos, pTimestamps, pMaxDeviation);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetCalibratedTimestampsEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetCalibratedTimestampsEXT(device, timestampCount, pTimestampInfos, pTimestamps, pMaxDeviation, result);
    }
    return result;
}










VKAPI_ATTR VkResult VKAPI_CALL CreateHeadlessSurfaceEXT(
    VkInstance                                  instance,
    const VkHeadlessSurfaceCreateInfoEXT*       pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateHeadlessSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateHeadlessSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface);
    }
    VkResult result = DispatchCreateHeadlessSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCreateHeadlessSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface, result);
    }
    return result;
}


VKAPI_ATTR void VKAPI_CALL CmdSetLineStippleEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    lineStippleFactor,
    uint16_t                                    lineStipplePattern) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetLineStippleEXT]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetLineStippleEXT(commandBuffer, lineStippleFactor, lineStipplePattern);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetLineStippleEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetLineStippleEXT(commandBuffer, lineStippleFactor, lineStipplePattern);
    }
    DispatchCmdSetLineStippleEXT(commandBuffer, lineStippleFactor, lineStipplePattern);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetLineStippleEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetLineStippleEXT(commandBuffer, lineStippleFactor, lineStipplePattern);
    }
}




VKAPI_ATTR void VKAPI_CALL CmdSetCullModeEXT(
    VkCommandBuffer                             commandBuffer,
    VkCullModeFlags                             cullMode) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetCullModeEXT]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetCullModeEXT(commandBuffer, cullMode);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetCullModeEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetCullModeEXT(commandBuffer, cullMode);
    }
    DispatchCmdSetCullModeEXT(commandBuffer, cullMode);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetCullModeEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetCullModeEXT(commandBuffer, cullMode);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetFrontFaceEXT(
    VkCommandBuffer                             commandBuffer,
    VkFrontFace                                 frontFace) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetFrontFaceEXT]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetFrontFaceEXT(commandBuffer, frontFace);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetFrontFaceEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetFrontFaceEXT(commandBuffer, frontFace);
    }
    DispatchCmdSetFrontFaceEXT(commandBuffer, frontFace);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetFrontFaceEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetFrontFaceEXT(commandBuffer, frontFace);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetPrimitiveTopologyEXT(
    VkCommandBuffer                             commandBuffer,
    VkPrimitiveTopology                         primitiveTopology) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetPrimitiveTopologyEXT]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetPrimitiveTopologyEXT(commandBuffer, primitiveTopology);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetPrimitiveTopologyEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetPrimitiveTopologyEXT(commandBuffer, primitiveTopology);
    }
    DispatchCmdSetPrimitiveTopologyEXT(commandBuffer, primitiveTopology);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetPrimitiveTopologyEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetPrimitiveTopologyEXT(commandBuffer, primitiveTopology);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetViewportWithCountEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    viewportCount,
    const VkViewport*                           pViewports) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetViewportWithCountEXT]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetViewportWithCountEXT(commandBuffer, viewportCount, pViewports);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetViewportWithCountEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetViewportWithCountEXT(commandBuffer, viewportCount, pViewports);
    }
    DispatchCmdSetViewportWithCountEXT(commandBuffer, viewportCount, pViewports);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetViewportWithCountEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetViewportWithCountEXT(commandBuffer, viewportCount, pViewports);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetScissorWithCountEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    scissorCount,
    const VkRect2D*                             pScissors) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetScissorWithCountEXT]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetScissorWithCountEXT(commandBuffer, scissorCount, pScissors);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetScissorWithCountEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetScissorWithCountEXT(commandBuffer, scissorCount, pScissors);
    }
    DispatchCmdSetScissorWithCountEXT(commandBuffer, scissorCount, pScissors);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetScissorWithCountEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetScissorWithCountEXT(commandBuffer, scissorCount, pScissors);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdBindVertexBuffers2EXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstBinding,
    uint32_t                                    bindingCount,
    const VkBuffer*                             pBuffers,
    const VkDeviceSize*                         pOffsets,
    const VkDeviceSize*                         pSizes,
    const VkDeviceSize*                         pStrides) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdBindVertexBuffers2EXT]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdBindVertexBuffers2EXT(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes, pStrides);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdBindVertexBuffers2EXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdBindVertexBuffers2EXT(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes, pStrides);
    }
    DispatchCmdBindVertexBuffers2EXT(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes, pStrides);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdBindVertexBuffers2EXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdBindVertexBuffers2EXT(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes, pStrides);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetDepthTestEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    depthTestEnable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetDepthTestEnableEXT]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetDepthTestEnableEXT(commandBuffer, depthTestEnable);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetDepthTestEnableEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetDepthTestEnableEXT(commandBuffer, depthTestEnable);
    }
    DispatchCmdSetDepthTestEnableEXT(commandBuffer, depthTestEnable);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetDepthTestEnableEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetDepthTestEnableEXT(commandBuffer, depthTestEnable);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetDepthWriteEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    depthWriteEnable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetDepthWriteEnableEXT]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetDepthWriteEnableEXT(commandBuffer, depthWriteEnable);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetDepthWriteEnableEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetDepthWriteEnableEXT(commandBuffer, depthWriteEnable);
    }
    DispatchCmdSetDepthWriteEnableEXT(commandBuffer, depthWriteEnable);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetDepthWriteEnableEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetDepthWriteEnableEXT(commandBuffer, depthWriteEnable);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetDepthCompareOpEXT(
    VkCommandBuffer                             commandBuffer,
    VkCompareOp                                 depthCompareOp) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetDepthCompareOpEXT]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetDepthCompareOpEXT(commandBuffer, depthCompareOp);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetDepthCompareOpEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetDepthCompareOpEXT(commandBuffer, depthCompareOp);
    }
    DispatchCmdSetDepthCompareOpEXT(commandBuffer, depthCompareOp);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetDepthCompareOpEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetDepthCompareOpEXT(commandBuffer, depthCompareOp);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetDepthBoundsTestEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    depthBoundsTestEnable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetDepthBoundsTestEnableEXT]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetDepthBoundsTestEnableEXT(commandBuffer, depthBoundsTestEnable);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetDepthBoundsTestEnableEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetDepthBoundsTestEnableEXT(commandBuffer, depthBoundsTestEnable);
    }
    DispatchCmdSetDepthBoundsTestEnableEXT(commandBuffer, depthBoundsTestEnable);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetDepthBoundsTestEnableEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetDepthBoundsTestEnableEXT(commandBuffer, depthBoundsTestEnable);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetStencilTestEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    stencilTestEnable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetStencilTestEnableEXT]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetStencilTestEnableEXT(commandBuffer, stencilTestEnable);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetStencilTestEnableEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetStencilTestEnableEXT(commandBuffer, stencilTestEnable);
    }
    DispatchCmdSetStencilTestEnableEXT(commandBuffer, stencilTestEnable);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetStencilTestEnableEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetStencilTestEnableEXT(commandBuffer, stencilTestEnable);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetStencilOpEXT(
    VkCommandBuffer                             commandBuffer,
    VkStencilFaceFlags                          faceMask,
    VkStencilOp                                 failOp,
    VkStencilOp                                 passOp,
    VkStencilOp                                 depthFailOp,
    VkCompareOp                                 compareOp) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetStencilOpEXT]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetStencilOpEXT(commandBuffer, faceMask, failOp, passOp, depthFailOp, compareOp);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetStencilOpEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetStencilOpEXT(commandBuffer, faceMask, failOp, passOp, depthFailOp, compareOp);
    }
    DispatchCmdSetStencilOpEXT(commandBuffer, faceMask, failOp, passOp, depthFailOp, compareOp);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetStencilOpEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetStencilOpEXT(commandBuffer, faceMask, failOp, passOp, depthFailOp, compareOp);
    }
}









VKAPI_ATTR void VKAPI_CALL CmdSetVertexInputEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    vertexBindingDescriptionCount,
    const VkVertexInputBindingDescription2EXT*  pVertexBindingDescriptions,
    uint32_t                                    vertexAttributeDescriptionCount,
    const VkVertexInputAttributeDescription2EXT* pVertexAttributeDescriptions) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetVertexInputEXT]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetVertexInputEXT(commandBuffer, vertexBindingDescriptionCount, pVertexBindingDescriptions, vertexAttributeDescriptionCount, pVertexAttributeDescriptions);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetVertexInputEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetVertexInputEXT(commandBuffer, vertexBindingDescriptionCount, pVertexBindingDescriptions, vertexAttributeDescriptionCount, pVertexAttributeDescriptions);
    }
    DispatchCmdSetVertexInputEXT(commandBuffer, vertexBindingDescriptionCount, pVertexBindingDescriptions, vertexAttributeDescriptionCount, pVertexAttributeDescriptions);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetVertexInputEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetVertexInputEXT(commandBuffer, vertexBindingDescriptionCount, pVertexBindingDescriptions, vertexAttributeDescriptionCount, pVertexAttributeDescriptions);
    }
}

#ifdef VK_USE_PLATFORM_SCI

VKAPI_ATTR VkResult VKAPI_CALL GetFenceSciSyncFenceNV(
    VkDevice                                    device,
    const VkFenceGetSciSyncInfoNV*              pGetSciSyncHandleInfo,
    void*                                       pHandle) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetFenceSciSyncFenceNV]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetFenceSciSyncFenceNV(device, pGetSciSyncHandleInfo, pHandle);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetFenceSciSyncFenceNV]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetFenceSciSyncFenceNV(device, pGetSciSyncHandleInfo, pHandle);
    }
    VkResult result = DispatchGetFenceSciSyncFenceNV(device, pGetSciSyncHandleInfo, pHandle);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetFenceSciSyncFenceNV]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetFenceSciSyncFenceNV(device, pGetSciSyncHandleInfo, pHandle, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetFenceSciSyncObjNV(
    VkDevice                                    device,
    const VkFenceGetSciSyncInfoNV*              pGetSciSyncHandleInfo,
    void*                                       pHandle) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetFenceSciSyncObjNV]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetFenceSciSyncObjNV(device, pGetSciSyncHandleInfo, pHandle);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetFenceSciSyncObjNV]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetFenceSciSyncObjNV(device, pGetSciSyncHandleInfo, pHandle);
    }
    VkResult result = DispatchGetFenceSciSyncObjNV(device, pGetSciSyncHandleInfo, pHandle);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetFenceSciSyncObjNV]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetFenceSciSyncObjNV(device, pGetSciSyncHandleInfo, pHandle, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL ImportFenceSciSyncFenceNV(
    VkDevice                                    device,
    const VkImportFenceSciSyncInfoNV*           pImportFenceSciSyncInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateImportFenceSciSyncFenceNV]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateImportFenceSciSyncFenceNV(device, pImportFenceSciSyncInfo);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordImportFenceSciSyncFenceNV]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordImportFenceSciSyncFenceNV(device, pImportFenceSciSyncInfo);
    }
    VkResult result = DispatchImportFenceSciSyncFenceNV(device, pImportFenceSciSyncInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordImportFenceSciSyncFenceNV]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordImportFenceSciSyncFenceNV(device, pImportFenceSciSyncInfo, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL ImportFenceSciSyncObjNV(
    VkDevice                                    device,
    const VkImportFenceSciSyncInfoNV*           pImportFenceSciSyncInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateImportFenceSciSyncObjNV]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateImportFenceSciSyncObjNV(device, pImportFenceSciSyncInfo);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordImportFenceSciSyncObjNV]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordImportFenceSciSyncObjNV(device, pImportFenceSciSyncInfo);
    }
    VkResult result = DispatchImportFenceSciSyncObjNV(device, pImportFenceSciSyncInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordImportFenceSciSyncObjNV]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordImportFenceSciSyncObjNV(device, pImportFenceSciSyncInfo, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceSciSyncAttributesNV(
    VkPhysicalDevice                            physicalDevice,
    const VkSciSyncAttributesInfoNV*            pSciSyncAttributesInfo,
    NvSciSyncAttrList                           pAttributes) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceSciSyncAttributesNV(physicalDevice, pSciSyncAttributesInfo, pAttributes);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceSciSyncAttributesNV(physicalDevice, pSciSyncAttributesInfo, pAttributes);
    }
    VkResult result = DispatchGetPhysicalDeviceSciSyncAttributesNV(physicalDevice, pSciSyncAttributesInfo, pAttributes);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetPhysicalDeviceSciSyncAttributesNV(physicalDevice, pSciSyncAttributesInfo, pAttributes, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetSemaphoreSciSyncObjNV(
    VkDevice                                    device,
    const VkSemaphoreGetSciSyncInfoNV*          pGetSciSyncInfo,
    void*                                       pHandle) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetSemaphoreSciSyncObjNV]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetSemaphoreSciSyncObjNV(device, pGetSciSyncInfo, pHandle);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetSemaphoreSciSyncObjNV]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetSemaphoreSciSyncObjNV(device, pGetSciSyncInfo, pHandle);
    }
    VkResult result = DispatchGetSemaphoreSciSyncObjNV(device, pGetSciSyncInfo, pHandle);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetSemaphoreSciSyncObjNV]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetSemaphoreSciSyncObjNV(device, pGetSciSyncInfo, pHandle, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL ImportSemaphoreSciSyncObjNV(
    VkDevice                                    device,
    const VkImportSemaphoreSciSyncInfoNV*       pImportSemaphoreSciSyncInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateImportSemaphoreSciSyncObjNV]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateImportSemaphoreSciSyncObjNV(device, pImportSemaphoreSciSyncInfo);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordImportSemaphoreSciSyncObjNV]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordImportSemaphoreSciSyncObjNV(device, pImportSemaphoreSciSyncInfo);
    }
    VkResult result = DispatchImportSemaphoreSciSyncObjNV(device, pImportSemaphoreSciSyncInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordImportSemaphoreSciSyncObjNV]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordImportSemaphoreSciSyncObjNV(device, pImportSemaphoreSciSyncInfo, result);
    }
    return result;
}
#endif // VK_USE_PLATFORM_SCI

#ifdef VK_USE_PLATFORM_SCI

VKAPI_ATTR VkResult VKAPI_CALL GetMemorySciBufNV(
    VkDevice                                    device,
    const VkMemoryGetSciBufInfoNV*              pGetSciBufInfo,
    NvSciBufObj*                                pHandle) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetMemorySciBufNV]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetMemorySciBufNV(device, pGetSciBufInfo, pHandle);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetMemorySciBufNV]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetMemorySciBufNV(device, pGetSciBufInfo, pHandle);
    }
    VkResult result = DispatchGetMemorySciBufNV(device, pGetSciBufInfo, pHandle);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetMemorySciBufNV]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetMemorySciBufNV(device, pGetSciBufInfo, pHandle, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceExternalMemorySciBufPropertiesNV(
    VkPhysicalDevice                            physicalDevice,
    VkExternalMemoryHandleTypeFlagBits          handleType,
    NvSciBufObj                                 handle,
    VkMemorySciBufPropertiesNV*                 pMemorySciBufProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceExternalMemorySciBufPropertiesNV(physicalDevice, handleType, handle, pMemorySciBufProperties);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceExternalMemorySciBufPropertiesNV(physicalDevice, handleType, handle, pMemorySciBufProperties);
    }
    VkResult result = DispatchGetPhysicalDeviceExternalMemorySciBufPropertiesNV(physicalDevice, handleType, handle, pMemorySciBufProperties);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetPhysicalDeviceExternalMemorySciBufPropertiesNV(physicalDevice, handleType, handle, pMemorySciBufProperties, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceSciBufAttributesNV(
    VkPhysicalDevice                            physicalDevice,
    NvSciBufAttrList                            pAttributes) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceSciBufAttributesNV(physicalDevice, pAttributes);
        if (skip) return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceSciBufAttributesNV(physicalDevice, pAttributes);
    }
    VkResult result = DispatchGetPhysicalDeviceSciBufAttributesNV(physicalDevice, pAttributes);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetPhysicalDeviceSciBufAttributesNV(physicalDevice, pAttributes, result);
    }
    return result;
}
#endif // VK_USE_PLATFORM_SCI


VKAPI_ATTR void VKAPI_CALL CmdSetPatchControlPointsEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    patchControlPoints) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetPatchControlPointsEXT]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetPatchControlPointsEXT(commandBuffer, patchControlPoints);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetPatchControlPointsEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetPatchControlPointsEXT(commandBuffer, patchControlPoints);
    }
    DispatchCmdSetPatchControlPointsEXT(commandBuffer, patchControlPoints);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetPatchControlPointsEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetPatchControlPointsEXT(commandBuffer, patchControlPoints);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetRasterizerDiscardEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    rasterizerDiscardEnable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetRasterizerDiscardEnableEXT]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetRasterizerDiscardEnableEXT(commandBuffer, rasterizerDiscardEnable);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetRasterizerDiscardEnableEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetRasterizerDiscardEnableEXT(commandBuffer, rasterizerDiscardEnable);
    }
    DispatchCmdSetRasterizerDiscardEnableEXT(commandBuffer, rasterizerDiscardEnable);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetRasterizerDiscardEnableEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetRasterizerDiscardEnableEXT(commandBuffer, rasterizerDiscardEnable);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetDepthBiasEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    depthBiasEnable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetDepthBiasEnableEXT]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetDepthBiasEnableEXT(commandBuffer, depthBiasEnable);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetDepthBiasEnableEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetDepthBiasEnableEXT(commandBuffer, depthBiasEnable);
    }
    DispatchCmdSetDepthBiasEnableEXT(commandBuffer, depthBiasEnable);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetDepthBiasEnableEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetDepthBiasEnableEXT(commandBuffer, depthBiasEnable);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetLogicOpEXT(
    VkCommandBuffer                             commandBuffer,
    VkLogicOp                                   logicOp) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetLogicOpEXT]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetLogicOpEXT(commandBuffer, logicOp);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetLogicOpEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetLogicOpEXT(commandBuffer, logicOp);
    }
    DispatchCmdSetLogicOpEXT(commandBuffer, logicOp);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetLogicOpEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetLogicOpEXT(commandBuffer, logicOp);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetPrimitiveRestartEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    primitiveRestartEnable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetPrimitiveRestartEnableEXT]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetPrimitiveRestartEnableEXT(commandBuffer, primitiveRestartEnable);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetPrimitiveRestartEnableEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetPrimitiveRestartEnableEXT(commandBuffer, primitiveRestartEnable);
    }
    DispatchCmdSetPrimitiveRestartEnableEXT(commandBuffer, primitiveRestartEnable);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetPrimitiveRestartEnableEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetPrimitiveRestartEnableEXT(commandBuffer, primitiveRestartEnable);
    }
}


VKAPI_ATTR void                                    VKAPI_CALL CmdSetColorWriteEnableEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    attachmentCount,
    const VkBool32*                             pColorWriteEnables) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetColorWriteEnableEXT]) {
        auto lock = intercept->ReadLock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetColorWriteEnableEXT(commandBuffer, attachmentCount, pColorWriteEnables);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetColorWriteEnableEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetColorWriteEnableEXT(commandBuffer, attachmentCount, pColorWriteEnables);
    }
    DispatchCmdSetColorWriteEnableEXT(commandBuffer, attachmentCount, pColorWriteEnables);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetColorWriteEnableEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetColorWriteEnableEXT(commandBuffer, attachmentCount, pColorWriteEnables);
    }
}


// Map of intercepted ApiName to its associated function data
#ifdef _MSC_VER
#pragma warning( suppress: 6262 ) // VS analysis: this uses more than 16 kiB, which is fine here at global scope
#endif
const layer_data::unordered_map<std::string, function_data> name_to_funcptr_map = {
    {"vkCreateInstance", {kFuncTypeInst, (void*)CreateInstance}},
    {"vkDestroyInstance", {kFuncTypeInst, (void*)DestroyInstance}},
    {"vkEnumeratePhysicalDevices", {kFuncTypeInst, (void*)EnumeratePhysicalDevices}},
    {"vkGetPhysicalDeviceFeatures", {kFuncTypePdev, (void*)GetPhysicalDeviceFeatures}},
    {"vkGetPhysicalDeviceFormatProperties", {kFuncTypePdev, (void*)GetPhysicalDeviceFormatProperties}},
    {"vkGetPhysicalDeviceImageFormatProperties", {kFuncTypePdev, (void*)GetPhysicalDeviceImageFormatProperties}},
    {"vkGetPhysicalDeviceProperties", {kFuncTypePdev, (void*)GetPhysicalDeviceProperties}},
    {"vkGetPhysicalDeviceQueueFamilyProperties", {kFuncTypePdev, (void*)GetPhysicalDeviceQueueFamilyProperties}},
    {"vkGetPhysicalDeviceMemoryProperties", {kFuncTypePdev, (void*)GetPhysicalDeviceMemoryProperties}},
    {"vkGetInstanceProcAddr", {kFuncTypeInst, (void*)GetInstanceProcAddr}},
    {"vkGetDeviceProcAddr", {kFuncTypeDev, (void*)GetDeviceProcAddr}},
    {"vkCreateDevice", {kFuncTypePdev, (void*)CreateDevice}},
    {"vkDestroyDevice", {kFuncTypeDev, (void*)DestroyDevice}},
    {"vkEnumerateInstanceExtensionProperties", {kFuncTypeInst, (void*)EnumerateInstanceExtensionProperties}},
    {"vkEnumerateDeviceExtensionProperties", {kFuncTypePdev, (void*)EnumerateDeviceExtensionProperties}},
    {"vkEnumerateInstanceLayerProperties", {kFuncTypeInst, (void*)EnumerateInstanceLayerProperties}},
    {"vkEnumerateDeviceLayerProperties", {kFuncTypePdev, (void*)EnumerateDeviceLayerProperties}},
    {"vkGetDeviceQueue", {kFuncTypeDev, (void*)GetDeviceQueue}},
    {"vkQueueSubmit", {kFuncTypeDev, (void*)QueueSubmit}},
    {"vkQueueWaitIdle", {kFuncTypeDev, (void*)QueueWaitIdle}},
    {"vkDeviceWaitIdle", {kFuncTypeDev, (void*)DeviceWaitIdle}},
    {"vkAllocateMemory", {kFuncTypeDev, (void*)AllocateMemory}},
    {"vkMapMemory", {kFuncTypeDev, (void*)MapMemory}},
    {"vkUnmapMemory", {kFuncTypeDev, (void*)UnmapMemory}},
    {"vkFlushMappedMemoryRanges", {kFuncTypeDev, (void*)FlushMappedMemoryRanges}},
    {"vkInvalidateMappedMemoryRanges", {kFuncTypeDev, (void*)InvalidateMappedMemoryRanges}},
    {"vkGetDeviceMemoryCommitment", {kFuncTypeDev, (void*)GetDeviceMemoryCommitment}},
    {"vkBindBufferMemory", {kFuncTypeDev, (void*)BindBufferMemory}},
    {"vkBindImageMemory", {kFuncTypeDev, (void*)BindImageMemory}},
    {"vkGetBufferMemoryRequirements", {kFuncTypeDev, (void*)GetBufferMemoryRequirements}},
    {"vkGetImageMemoryRequirements", {kFuncTypeDev, (void*)GetImageMemoryRequirements}},
    {"vkCreateFence", {kFuncTypeDev, (void*)CreateFence}},
    {"vkDestroyFence", {kFuncTypeDev, (void*)DestroyFence}},
    {"vkResetFences", {kFuncTypeDev, (void*)ResetFences}},
    {"vkGetFenceStatus", {kFuncTypeDev, (void*)GetFenceStatus}},
    {"vkWaitForFences", {kFuncTypeDev, (void*)WaitForFences}},
    {"vkCreateSemaphore", {kFuncTypeDev, (void*)CreateSemaphore}},
    {"vkDestroySemaphore", {kFuncTypeDev, (void*)DestroySemaphore}},
    {"vkCreateEvent", {kFuncTypeDev, (void*)CreateEvent}},
    {"vkDestroyEvent", {kFuncTypeDev, (void*)DestroyEvent}},
    {"vkGetEventStatus", {kFuncTypeDev, (void*)GetEventStatus}},
    {"vkSetEvent", {kFuncTypeDev, (void*)SetEvent}},
    {"vkResetEvent", {kFuncTypeDev, (void*)ResetEvent}},
    {"vkCreateQueryPool", {kFuncTypeDev, (void*)CreateQueryPool}},
    {"vkGetQueryPoolResults", {kFuncTypeDev, (void*)GetQueryPoolResults}},
    {"vkCreateBuffer", {kFuncTypeDev, (void*)CreateBuffer}},
    {"vkDestroyBuffer", {kFuncTypeDev, (void*)DestroyBuffer}},
    {"vkCreateBufferView", {kFuncTypeDev, (void*)CreateBufferView}},
    {"vkDestroyBufferView", {kFuncTypeDev, (void*)DestroyBufferView}},
    {"vkCreateImage", {kFuncTypeDev, (void*)CreateImage}},
    {"vkDestroyImage", {kFuncTypeDev, (void*)DestroyImage}},
    {"vkGetImageSubresourceLayout", {kFuncTypeDev, (void*)GetImageSubresourceLayout}},
    {"vkCreateImageView", {kFuncTypeDev, (void*)CreateImageView}},
    {"vkDestroyImageView", {kFuncTypeDev, (void*)DestroyImageView}},
    {"vkCreatePipelineCache", {kFuncTypeDev, (void*)CreatePipelineCache}},
    {"vkDestroyPipelineCache", {kFuncTypeDev, (void*)DestroyPipelineCache}},
    {"vkCreateGraphicsPipelines", {kFuncTypeDev, (void*)CreateGraphicsPipelines}},
    {"vkCreateComputePipelines", {kFuncTypeDev, (void*)CreateComputePipelines}},
    {"vkDestroyPipeline", {kFuncTypeDev, (void*)DestroyPipeline}},
    {"vkCreatePipelineLayout", {kFuncTypeDev, (void*)CreatePipelineLayout}},
    {"vkDestroyPipelineLayout", {kFuncTypeDev, (void*)DestroyPipelineLayout}},
    {"vkCreateSampler", {kFuncTypeDev, (void*)CreateSampler}},
    {"vkDestroySampler", {kFuncTypeDev, (void*)DestroySampler}},
    {"vkCreateDescriptorSetLayout", {kFuncTypeDev, (void*)CreateDescriptorSetLayout}},
    {"vkDestroyDescriptorSetLayout", {kFuncTypeDev, (void*)DestroyDescriptorSetLayout}},
    {"vkCreateDescriptorPool", {kFuncTypeDev, (void*)CreateDescriptorPool}},
    {"vkResetDescriptorPool", {kFuncTypeDev, (void*)ResetDescriptorPool}},
    {"vkAllocateDescriptorSets", {kFuncTypeDev, (void*)AllocateDescriptorSets}},
    {"vkFreeDescriptorSets", {kFuncTypeDev, (void*)FreeDescriptorSets}},
    {"vkUpdateDescriptorSets", {kFuncTypeDev, (void*)UpdateDescriptorSets}},
    {"vkCreateFramebuffer", {kFuncTypeDev, (void*)CreateFramebuffer}},
    {"vkDestroyFramebuffer", {kFuncTypeDev, (void*)DestroyFramebuffer}},
    {"vkCreateRenderPass", {kFuncTypeDev, (void*)CreateRenderPass}},
    {"vkDestroyRenderPass", {kFuncTypeDev, (void*)DestroyRenderPass}},
    {"vkGetRenderAreaGranularity", {kFuncTypeDev, (void*)GetRenderAreaGranularity}},
    {"vkCreateCommandPool", {kFuncTypeDev, (void*)CreateCommandPool}},
    {"vkResetCommandPool", {kFuncTypeDev, (void*)ResetCommandPool}},
    {"vkAllocateCommandBuffers", {kFuncTypeDev, (void*)AllocateCommandBuffers}},
    {"vkFreeCommandBuffers", {kFuncTypeDev, (void*)FreeCommandBuffers}},
    {"vkBeginCommandBuffer", {kFuncTypeDev, (void*)BeginCommandBuffer}},
    {"vkEndCommandBuffer", {kFuncTypeDev, (void*)EndCommandBuffer}},
    {"vkResetCommandBuffer", {kFuncTypeDev, (void*)ResetCommandBuffer}},
    {"vkCmdBindPipeline", {kFuncTypeDev, (void*)CmdBindPipeline}},
    {"vkCmdSetViewport", {kFuncTypeDev, (void*)CmdSetViewport}},
    {"vkCmdSetScissor", {kFuncTypeDev, (void*)CmdSetScissor}},
    {"vkCmdSetLineWidth", {kFuncTypeDev, (void*)CmdSetLineWidth}},
    {"vkCmdSetDepthBias", {kFuncTypeDev, (void*)CmdSetDepthBias}},
    {"vkCmdSetBlendConstants", {kFuncTypeDev, (void*)CmdSetBlendConstants}},
    {"vkCmdSetDepthBounds", {kFuncTypeDev, (void*)CmdSetDepthBounds}},
    {"vkCmdSetStencilCompareMask", {kFuncTypeDev, (void*)CmdSetStencilCompareMask}},
    {"vkCmdSetStencilWriteMask", {kFuncTypeDev, (void*)CmdSetStencilWriteMask}},
    {"vkCmdSetStencilReference", {kFuncTypeDev, (void*)CmdSetStencilReference}},
    {"vkCmdBindDescriptorSets", {kFuncTypeDev, (void*)CmdBindDescriptorSets}},
    {"vkCmdBindIndexBuffer", {kFuncTypeDev, (void*)CmdBindIndexBuffer}},
    {"vkCmdBindVertexBuffers", {kFuncTypeDev, (void*)CmdBindVertexBuffers}},
    {"vkCmdDraw", {kFuncTypeDev, (void*)CmdDraw}},
    {"vkCmdDrawIndexed", {kFuncTypeDev, (void*)CmdDrawIndexed}},
    {"vkCmdDrawIndirect", {kFuncTypeDev, (void*)CmdDrawIndirect}},
    {"vkCmdDrawIndexedIndirect", {kFuncTypeDev, (void*)CmdDrawIndexedIndirect}},
    {"vkCmdDispatch", {kFuncTypeDev, (void*)CmdDispatch}},
    {"vkCmdDispatchIndirect", {kFuncTypeDev, (void*)CmdDispatchIndirect}},
    {"vkCmdCopyBuffer", {kFuncTypeDev, (void*)CmdCopyBuffer}},
    {"vkCmdCopyImage", {kFuncTypeDev, (void*)CmdCopyImage}},
    {"vkCmdBlitImage", {kFuncTypeDev, (void*)CmdBlitImage}},
    {"vkCmdCopyBufferToImage", {kFuncTypeDev, (void*)CmdCopyBufferToImage}},
    {"vkCmdCopyImageToBuffer", {kFuncTypeDev, (void*)CmdCopyImageToBuffer}},
    {"vkCmdUpdateBuffer", {kFuncTypeDev, (void*)CmdUpdateBuffer}},
    {"vkCmdFillBuffer", {kFuncTypeDev, (void*)CmdFillBuffer}},
    {"vkCmdClearColorImage", {kFuncTypeDev, (void*)CmdClearColorImage}},
    {"vkCmdClearDepthStencilImage", {kFuncTypeDev, (void*)CmdClearDepthStencilImage}},
    {"vkCmdClearAttachments", {kFuncTypeDev, (void*)CmdClearAttachments}},
    {"vkCmdResolveImage", {kFuncTypeDev, (void*)CmdResolveImage}},
    {"vkCmdSetEvent", {kFuncTypeDev, (void*)CmdSetEvent}},
    {"vkCmdResetEvent", {kFuncTypeDev, (void*)CmdResetEvent}},
    {"vkCmdWaitEvents", {kFuncTypeDev, (void*)CmdWaitEvents}},
    {"vkCmdPipelineBarrier", {kFuncTypeDev, (void*)CmdPipelineBarrier}},
    {"vkCmdBeginQuery", {kFuncTypeDev, (void*)CmdBeginQuery}},
    {"vkCmdEndQuery", {kFuncTypeDev, (void*)CmdEndQuery}},
    {"vkCmdResetQueryPool", {kFuncTypeDev, (void*)CmdResetQueryPool}},
    {"vkCmdWriteTimestamp", {kFuncTypeDev, (void*)CmdWriteTimestamp}},
    {"vkCmdCopyQueryPoolResults", {kFuncTypeDev, (void*)CmdCopyQueryPoolResults}},
    {"vkCmdPushConstants", {kFuncTypeDev, (void*)CmdPushConstants}},
    {"vkCmdBeginRenderPass", {kFuncTypeDev, (void*)CmdBeginRenderPass}},
    {"vkCmdNextSubpass", {kFuncTypeDev, (void*)CmdNextSubpass}},
    {"vkCmdEndRenderPass", {kFuncTypeDev, (void*)CmdEndRenderPass}},
    {"vkCmdExecuteCommands", {kFuncTypeDev, (void*)CmdExecuteCommands}},
    {"vkBindBufferMemory2", {kFuncTypeDev, (void*)BindBufferMemory2}},
    {"vkBindImageMemory2", {kFuncTypeDev, (void*)BindImageMemory2}},
    {"vkGetDeviceGroupPeerMemoryFeatures", {kFuncTypeDev, (void*)GetDeviceGroupPeerMemoryFeatures}},
    {"vkCmdSetDeviceMask", {kFuncTypeDev, (void*)CmdSetDeviceMask}},
    {"vkCmdDispatchBase", {kFuncTypeDev, (void*)CmdDispatchBase}},
    {"vkEnumeratePhysicalDeviceGroups", {kFuncTypeInst, (void*)EnumeratePhysicalDeviceGroups}},
    {"vkGetImageMemoryRequirements2", {kFuncTypeDev, (void*)GetImageMemoryRequirements2}},
    {"vkGetBufferMemoryRequirements2", {kFuncTypeDev, (void*)GetBufferMemoryRequirements2}},
    {"vkGetPhysicalDeviceFeatures2", {kFuncTypePdev, (void*)GetPhysicalDeviceFeatures2}},
    {"vkGetPhysicalDeviceProperties2", {kFuncTypePdev, (void*)GetPhysicalDeviceProperties2}},
    {"vkGetPhysicalDeviceFormatProperties2", {kFuncTypePdev, (void*)GetPhysicalDeviceFormatProperties2}},
    {"vkGetPhysicalDeviceImageFormatProperties2", {kFuncTypePdev, (void*)GetPhysicalDeviceImageFormatProperties2}},
    {"vkGetPhysicalDeviceQueueFamilyProperties2", {kFuncTypePdev, (void*)GetPhysicalDeviceQueueFamilyProperties2}},
    {"vkGetPhysicalDeviceMemoryProperties2", {kFuncTypePdev, (void*)GetPhysicalDeviceMemoryProperties2}},
    {"vkGetDeviceQueue2", {kFuncTypeDev, (void*)GetDeviceQueue2}},
    {"vkCreateSamplerYcbcrConversion", {kFuncTypeDev, (void*)CreateSamplerYcbcrConversion}},
    {"vkDestroySamplerYcbcrConversion", {kFuncTypeDev, (void*)DestroySamplerYcbcrConversion}},
    {"vkGetPhysicalDeviceExternalBufferProperties", {kFuncTypePdev, (void*)GetPhysicalDeviceExternalBufferProperties}},
    {"vkGetPhysicalDeviceExternalFenceProperties", {kFuncTypePdev, (void*)GetPhysicalDeviceExternalFenceProperties}},
    {"vkGetPhysicalDeviceExternalSemaphoreProperties", {kFuncTypePdev, (void*)GetPhysicalDeviceExternalSemaphoreProperties}},
    {"vkGetDescriptorSetLayoutSupport", {kFuncTypeDev, (void*)GetDescriptorSetLayoutSupport}},
    {"vkCmdDrawIndirectCount", {kFuncTypeDev, (void*)CmdDrawIndirectCount}},
    {"vkCmdDrawIndexedIndirectCount", {kFuncTypeDev, (void*)CmdDrawIndexedIndirectCount}},
    {"vkCreateRenderPass2", {kFuncTypeDev, (void*)CreateRenderPass2}},
    {"vkCmdBeginRenderPass2", {kFuncTypeDev, (void*)CmdBeginRenderPass2}},
    {"vkCmdNextSubpass2", {kFuncTypeDev, (void*)CmdNextSubpass2}},
    {"vkCmdEndRenderPass2", {kFuncTypeDev, (void*)CmdEndRenderPass2}},
    {"vkResetQueryPool", {kFuncTypeDev, (void*)ResetQueryPool}},
    {"vkGetSemaphoreCounterValue", {kFuncTypeDev, (void*)GetSemaphoreCounterValue}},
    {"vkWaitSemaphores", {kFuncTypeDev, (void*)WaitSemaphores}},
    {"vkSignalSemaphore", {kFuncTypeDev, (void*)SignalSemaphore}},
    {"vkGetBufferDeviceAddress", {kFuncTypeDev, (void*)GetBufferDeviceAddress}},
    {"vkGetBufferOpaqueCaptureAddress", {kFuncTypeDev, (void*)GetBufferOpaqueCaptureAddress}},
    {"vkGetDeviceMemoryOpaqueCaptureAddress", {kFuncTypeDev, (void*)GetDeviceMemoryOpaqueCaptureAddress}},
    {"vkGetCommandPoolMemoryConsumption", {kFuncTypeDev, (void*)GetCommandPoolMemoryConsumption}},
    {"vkGetFaultData", {kFuncTypeDev, (void*)GetFaultData}},
    {"vkDestroySurfaceKHR", {kFuncTypeInst, (void*)DestroySurfaceKHR}},
    {"vkGetPhysicalDeviceSurfaceSupportKHR", {kFuncTypePdev, (void*)GetPhysicalDeviceSurfaceSupportKHR}},
    {"vkGetPhysicalDeviceSurfaceCapabilitiesKHR", {kFuncTypePdev, (void*)GetPhysicalDeviceSurfaceCapabilitiesKHR}},
    {"vkGetPhysicalDeviceSurfaceFormatsKHR", {kFuncTypePdev, (void*)GetPhysicalDeviceSurfaceFormatsKHR}},
    {"vkGetPhysicalDeviceSurfacePresentModesKHR", {kFuncTypePdev, (void*)GetPhysicalDeviceSurfacePresentModesKHR}},
    {"vkCreateSwapchainKHR", {kFuncTypeDev, (void*)CreateSwapchainKHR}},
    {"vkGetSwapchainImagesKHR", {kFuncTypeDev, (void*)GetSwapchainImagesKHR}},
    {"vkAcquireNextImageKHR", {kFuncTypeDev, (void*)AcquireNextImageKHR}},
    {"vkQueuePresentKHR", {kFuncTypeDev, (void*)QueuePresentKHR}},
    {"vkGetDeviceGroupPresentCapabilitiesKHR", {kFuncTypeDev, (void*)GetDeviceGroupPresentCapabilitiesKHR}},
    {"vkGetDeviceGroupSurfacePresentModesKHR", {kFuncTypeDev, (void*)GetDeviceGroupSurfacePresentModesKHR}},
    {"vkGetPhysicalDevicePresentRectanglesKHR", {kFuncTypePdev, (void*)GetPhysicalDevicePresentRectanglesKHR}},
    {"vkAcquireNextImage2KHR", {kFuncTypeDev, (void*)AcquireNextImage2KHR}},
    {"vkGetPhysicalDeviceDisplayPropertiesKHR", {kFuncTypePdev, (void*)GetPhysicalDeviceDisplayPropertiesKHR}},
    {"vkGetPhysicalDeviceDisplayPlanePropertiesKHR", {kFuncTypePdev, (void*)GetPhysicalDeviceDisplayPlanePropertiesKHR}},
    {"vkGetDisplayPlaneSupportedDisplaysKHR", {kFuncTypePdev, (void*)GetDisplayPlaneSupportedDisplaysKHR}},
    {"vkGetDisplayModePropertiesKHR", {kFuncTypePdev, (void*)GetDisplayModePropertiesKHR}},
    {"vkCreateDisplayModeKHR", {kFuncTypePdev, (void*)CreateDisplayModeKHR}},
    {"vkGetDisplayPlaneCapabilitiesKHR", {kFuncTypePdev, (void*)GetDisplayPlaneCapabilitiesKHR}},
    {"vkCreateDisplayPlaneSurfaceKHR", {kFuncTypeInst, (void*)CreateDisplayPlaneSurfaceKHR}},
    {"vkCreateSharedSwapchainsKHR", {kFuncTypeDev, (void*)CreateSharedSwapchainsKHR}},
    {"vkGetMemoryFdKHR", {kFuncTypeDev, (void*)GetMemoryFdKHR}},
    {"vkGetMemoryFdPropertiesKHR", {kFuncTypeDev, (void*)GetMemoryFdPropertiesKHR}},
    {"vkImportSemaphoreFdKHR", {kFuncTypeDev, (void*)ImportSemaphoreFdKHR}},
    {"vkGetSemaphoreFdKHR", {kFuncTypeDev, (void*)GetSemaphoreFdKHR}},
    {"vkGetSwapchainStatusKHR", {kFuncTypeDev, (void*)GetSwapchainStatusKHR}},
    {"vkImportFenceFdKHR", {kFuncTypeDev, (void*)ImportFenceFdKHR}},
    {"vkGetFenceFdKHR", {kFuncTypeDev, (void*)GetFenceFdKHR}},
    {"vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR", {kFuncTypePdev, (void*)EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR}},
    {"vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR", {kFuncTypePdev, (void*)GetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR}},
    {"vkAcquireProfilingLockKHR", {kFuncTypeDev, (void*)AcquireProfilingLockKHR}},
    {"vkReleaseProfilingLockKHR", {kFuncTypeDev, (void*)ReleaseProfilingLockKHR}},
    {"vkGetPhysicalDeviceSurfaceCapabilities2KHR", {kFuncTypePdev, (void*)GetPhysicalDeviceSurfaceCapabilities2KHR}},
    {"vkGetPhysicalDeviceSurfaceFormats2KHR", {kFuncTypePdev, (void*)GetPhysicalDeviceSurfaceFormats2KHR}},
    {"vkGetPhysicalDeviceDisplayProperties2KHR", {kFuncTypePdev, (void*)GetPhysicalDeviceDisplayProperties2KHR}},
    {"vkGetPhysicalDeviceDisplayPlaneProperties2KHR", {kFuncTypePdev, (void*)GetPhysicalDeviceDisplayPlaneProperties2KHR}},
    {"vkGetDisplayModeProperties2KHR", {kFuncTypePdev, (void*)GetDisplayModeProperties2KHR}},
    {"vkGetDisplayPlaneCapabilities2KHR", {kFuncTypePdev, (void*)GetDisplayPlaneCapabilities2KHR}},
    {"vkGetPhysicalDeviceFragmentShadingRatesKHR", {kFuncTypePdev, (void*)GetPhysicalDeviceFragmentShadingRatesKHR}},
    {"vkCmdSetFragmentShadingRateKHR", {kFuncTypeDev, (void*)CmdSetFragmentShadingRateKHR}},
    {"vkCmdRefreshObjectsKHR", {kFuncTypeDev, (void*)CmdRefreshObjectsKHR}},
    {"vkGetPhysicalDeviceRefreshableObjectTypesKHR", {kFuncTypePdev, (void*)GetPhysicalDeviceRefreshableObjectTypesKHR}},
    {"vkCmdSetEvent2KHR", {kFuncTypeDev, (void*)CmdSetEvent2KHR}},
    {"vkCmdResetEvent2KHR", {kFuncTypeDev, (void*)CmdResetEvent2KHR}},
    {"vkCmdWaitEvents2KHR", {kFuncTypeDev, (void*)CmdWaitEvents2KHR}},
    {"vkCmdPipelineBarrier2KHR", {kFuncTypeDev, (void*)CmdPipelineBarrier2KHR}},
    {"vkCmdWriteTimestamp2KHR", {kFuncTypeDev, (void*)CmdWriteTimestamp2KHR}},
    {"vkQueueSubmit2KHR", {kFuncTypeDev, (void*)QueueSubmit2KHR}},
    {"vkCmdWriteBufferMarker2AMD", {kFuncTypeDev, (void*)CmdWriteBufferMarker2AMD}},
    {"vkGetQueueCheckpointData2NV", {kFuncTypeDev, (void*)GetQueueCheckpointData2NV}},
    {"vkCmdCopyBuffer2KHR", {kFuncTypeDev, (void*)CmdCopyBuffer2KHR}},
    {"vkCmdCopyImage2KHR", {kFuncTypeDev, (void*)CmdCopyImage2KHR}},
    {"vkCmdCopyBufferToImage2KHR", {kFuncTypeDev, (void*)CmdCopyBufferToImage2KHR}},
    {"vkCmdCopyImageToBuffer2KHR", {kFuncTypeDev, (void*)CmdCopyImageToBuffer2KHR}},
    {"vkCmdBlitImage2KHR", {kFuncTypeDev, (void*)CmdBlitImage2KHR}},
    {"vkCmdResolveImage2KHR", {kFuncTypeDev, (void*)CmdResolveImage2KHR}},
    {"vkReleaseDisplayEXT", {kFuncTypePdev, (void*)ReleaseDisplayEXT}},
    {"vkGetPhysicalDeviceSurfaceCapabilities2EXT", {kFuncTypePdev, (void*)GetPhysicalDeviceSurfaceCapabilities2EXT}},
    {"vkDisplayPowerControlEXT", {kFuncTypeDev, (void*)DisplayPowerControlEXT}},
    {"vkRegisterDeviceEventEXT", {kFuncTypeDev, (void*)RegisterDeviceEventEXT}},
    {"vkRegisterDisplayEventEXT", {kFuncTypeDev, (void*)RegisterDisplayEventEXT}},
    {"vkGetSwapchainCounterEXT", {kFuncTypeDev, (void*)GetSwapchainCounterEXT}},
    {"vkCmdSetDiscardRectangleEXT", {kFuncTypeDev, (void*)CmdSetDiscardRectangleEXT}},
    {"vkSetHdrMetadataEXT", {kFuncTypeDev, (void*)SetHdrMetadataEXT}},
    {"vkSetDebugUtilsObjectNameEXT", {kFuncTypeDev, (void*)SetDebugUtilsObjectNameEXT}},
    {"vkSetDebugUtilsObjectTagEXT", {kFuncTypeDev, (void*)SetDebugUtilsObjectTagEXT}},
    {"vkQueueBeginDebugUtilsLabelEXT", {kFuncTypeDev, (void*)QueueBeginDebugUtilsLabelEXT}},
    {"vkQueueEndDebugUtilsLabelEXT", {kFuncTypeDev, (void*)QueueEndDebugUtilsLabelEXT}},
    {"vkQueueInsertDebugUtilsLabelEXT", {kFuncTypeDev, (void*)QueueInsertDebugUtilsLabelEXT}},
    {"vkCmdBeginDebugUtilsLabelEXT", {kFuncTypeDev, (void*)CmdBeginDebugUtilsLabelEXT}},
    {"vkCmdEndDebugUtilsLabelEXT", {kFuncTypeDev, (void*)CmdEndDebugUtilsLabelEXT}},
    {"vkCmdInsertDebugUtilsLabelEXT", {kFuncTypeDev, (void*)CmdInsertDebugUtilsLabelEXT}},
    {"vkCreateDebugUtilsMessengerEXT", {kFuncTypeInst, (void*)CreateDebugUtilsMessengerEXT}},
    {"vkDestroyDebugUtilsMessengerEXT", {kFuncTypeInst, (void*)DestroyDebugUtilsMessengerEXT}},
    {"vkSubmitDebugUtilsMessageEXT", {kFuncTypeInst, (void*)SubmitDebugUtilsMessageEXT}},
    {"vkCmdSetSampleLocationsEXT", {kFuncTypeDev, (void*)CmdSetSampleLocationsEXT}},
    {"vkGetPhysicalDeviceMultisamplePropertiesEXT", {kFuncTypePdev, (void*)GetPhysicalDeviceMultisamplePropertiesEXT}},
    {"vkGetImageDrmFormatModifierPropertiesEXT", {kFuncTypeDev, (void*)GetImageDrmFormatModifierPropertiesEXT}},
    {"vkGetMemoryHostPointerPropertiesEXT", {kFuncTypeDev, (void*)GetMemoryHostPointerPropertiesEXT}},
    {"vkGetPhysicalDeviceCalibrateableTimeDomainsEXT", {kFuncTypePdev, (void*)GetPhysicalDeviceCalibrateableTimeDomainsEXT}},
    {"vkGetCalibratedTimestampsEXT", {kFuncTypeDev, (void*)GetCalibratedTimestampsEXT}},
    {"vkCreateHeadlessSurfaceEXT", {kFuncTypeInst, (void*)CreateHeadlessSurfaceEXT}},
    {"vkCmdSetLineStippleEXT", {kFuncTypeDev, (void*)CmdSetLineStippleEXT}},
    {"vkCmdSetCullModeEXT", {kFuncTypeDev, (void*)CmdSetCullModeEXT}},
    {"vkCmdSetFrontFaceEXT", {kFuncTypeDev, (void*)CmdSetFrontFaceEXT}},
    {"vkCmdSetPrimitiveTopologyEXT", {kFuncTypeDev, (void*)CmdSetPrimitiveTopologyEXT}},
    {"vkCmdSetViewportWithCountEXT", {kFuncTypeDev, (void*)CmdSetViewportWithCountEXT}},
    {"vkCmdSetScissorWithCountEXT", {kFuncTypeDev, (void*)CmdSetScissorWithCountEXT}},
    {"vkCmdBindVertexBuffers2EXT", {kFuncTypeDev, (void*)CmdBindVertexBuffers2EXT}},
    {"vkCmdSetDepthTestEnableEXT", {kFuncTypeDev, (void*)CmdSetDepthTestEnableEXT}},
    {"vkCmdSetDepthWriteEnableEXT", {kFuncTypeDev, (void*)CmdSetDepthWriteEnableEXT}},
    {"vkCmdSetDepthCompareOpEXT", {kFuncTypeDev, (void*)CmdSetDepthCompareOpEXT}},
    {"vkCmdSetDepthBoundsTestEnableEXT", {kFuncTypeDev, (void*)CmdSetDepthBoundsTestEnableEXT}},
    {"vkCmdSetStencilTestEnableEXT", {kFuncTypeDev, (void*)CmdSetStencilTestEnableEXT}},
    {"vkCmdSetStencilOpEXT", {kFuncTypeDev, (void*)CmdSetStencilOpEXT}},
    {"vkCmdSetVertexInputEXT", {kFuncTypeDev, (void*)CmdSetVertexInputEXT}},
#ifdef VK_USE_PLATFORM_SCI
    {"vkGetFenceSciSyncFenceNV", {kFuncTypeDev, (void*)GetFenceSciSyncFenceNV}},
#endif
#ifdef VK_USE_PLATFORM_SCI
    {"vkGetFenceSciSyncObjNV", {kFuncTypeDev, (void*)GetFenceSciSyncObjNV}},
#endif
#ifdef VK_USE_PLATFORM_SCI
    {"vkImportFenceSciSyncFenceNV", {kFuncTypeDev, (void*)ImportFenceSciSyncFenceNV}},
#endif
#ifdef VK_USE_PLATFORM_SCI
    {"vkImportFenceSciSyncObjNV", {kFuncTypeDev, (void*)ImportFenceSciSyncObjNV}},
#endif
#ifdef VK_USE_PLATFORM_SCI
    {"vkGetPhysicalDeviceSciSyncAttributesNV", {kFuncTypePdev, (void*)GetPhysicalDeviceSciSyncAttributesNV}},
#endif
#ifdef VK_USE_PLATFORM_SCI
    {"vkGetSemaphoreSciSyncObjNV", {kFuncTypeDev, (void*)GetSemaphoreSciSyncObjNV}},
#endif
#ifdef VK_USE_PLATFORM_SCI
    {"vkImportSemaphoreSciSyncObjNV", {kFuncTypeDev, (void*)ImportSemaphoreSciSyncObjNV}},
#endif
#ifdef VK_USE_PLATFORM_SCI
    {"vkGetMemorySciBufNV", {kFuncTypeDev, (void*)GetMemorySciBufNV}},
#endif
#ifdef VK_USE_PLATFORM_SCI
    {"vkGetPhysicalDeviceExternalMemorySciBufPropertiesNV", {kFuncTypePdev, (void*)GetPhysicalDeviceExternalMemorySciBufPropertiesNV}},
#endif
#ifdef VK_USE_PLATFORM_SCI
    {"vkGetPhysicalDeviceSciBufAttributesNV", {kFuncTypePdev, (void*)GetPhysicalDeviceSciBufAttributesNV}},
#endif
    {"vkCmdSetPatchControlPointsEXT", {kFuncTypeDev, (void*)CmdSetPatchControlPointsEXT}},
    {"vkCmdSetRasterizerDiscardEnableEXT", {kFuncTypeDev, (void*)CmdSetRasterizerDiscardEnableEXT}},
    {"vkCmdSetDepthBiasEnableEXT", {kFuncTypeDev, (void*)CmdSetDepthBiasEnableEXT}},
    {"vkCmdSetLogicOpEXT", {kFuncTypeDev, (void*)CmdSetLogicOpEXT}},
    {"vkCmdSetPrimitiveRestartEnableEXT", {kFuncTypeDev, (void*)CmdSetPrimitiveRestartEnableEXT}},
    {"vkCmdSetColorWriteEnableEXT", {kFuncTypeDev, (void*)CmdSetColorWriteEnableEXT}},
};


} // namespace vulkan_layer_chassis

// loader-layer interface v0, just wrappers since there is only a layer

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateInstanceExtensionProperties(const char *pLayerName, uint32_t *pCount,
                                                                                      VkExtensionProperties *pProperties) {
    return vulkan_layer_chassis::EnumerateInstanceExtensionProperties(pLayerName, pCount, pProperties);
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateInstanceLayerProperties(uint32_t *pCount,
                                                                                  VkLayerProperties *pProperties) {
    return vulkan_layer_chassis::EnumerateInstanceLayerProperties(pCount, pProperties);
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice, uint32_t *pCount,
                                                                                VkLayerProperties *pProperties) {
    // the layer command handles VK_NULL_HANDLE just fine internally
    assert(physicalDevice == VK_NULL_HANDLE);
    return vulkan_layer_chassis::EnumerateDeviceLayerProperties(VK_NULL_HANDLE, pCount, pProperties);
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice,
                                                                                    const char *pLayerName, uint32_t *pCount,
                                                                                    VkExtensionProperties *pProperties) {
    // the layer command handles VK_NULL_HANDLE just fine internally
    assert(physicalDevice == VK_NULL_HANDLE);
    return vulkan_layer_chassis::EnumerateDeviceExtensionProperties(VK_NULL_HANDLE, pLayerName, pCount, pProperties);
}

VK_LAYER_EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetDeviceProcAddr(VkDevice dev, const char *funcName) {
    return vulkan_layer_chassis::GetDeviceProcAddr(dev, funcName);
}

VK_LAYER_EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetInstanceProcAddr(VkInstance instance, const char *funcName) {
    return vulkan_layer_chassis::GetInstanceProcAddr(instance, funcName);
}

VK_LAYER_EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vk_layerGetPhysicalDeviceProcAddr(VkInstance instance,
                                                                                           const char *funcName) {
    return vulkan_layer_chassis::GetPhysicalDeviceProcAddr(instance, funcName);
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkNegotiateLoaderLayerInterfaceVersion(VkNegotiateLayerInterface *pVersionStruct) {
    assert(pVersionStruct != NULL);
    assert(pVersionStruct->sType == LAYER_NEGOTIATE_INTERFACE_STRUCT);

    // Fill in the function pointers if our version is at least capable of having the structure contain them.
    if (pVersionStruct->loaderLayerInterfaceVersion >= 2) {
        pVersionStruct->pfnGetInstanceProcAddr = vkGetInstanceProcAddr;
        pVersionStruct->pfnGetDeviceProcAddr = vkGetDeviceProcAddr;
        pVersionStruct->pfnGetPhysicalDeviceProcAddr = vk_layerGetPhysicalDeviceProcAddr;
    }

    return VK_SUCCESS;
}
