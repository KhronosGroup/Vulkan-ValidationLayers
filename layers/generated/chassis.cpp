
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
#include "command_counter.h"
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

using std::unordered_map;

static const VkLayerProperties global_layer = {
    OBJECT_LAYER_NAME, VK_LAYER_API_VERSION, 1, "LunarG validation Layer",
};

static const VkExtensionProperties instance_extensions[] = {{VK_EXT_DEBUG_REPORT_EXTENSION_NAME, VK_EXT_DEBUG_REPORT_SPEC_VERSION},
                                                            {VK_EXT_DEBUG_UTILS_EXTENSION_NAME, VK_EXT_DEBUG_UTILS_SPEC_VERSION}};
static const VkExtensionProperties device_extensions[] = {
    {VK_EXT_VALIDATION_CACHE_EXTENSION_NAME, VK_EXT_VALIDATION_CACHE_SPEC_VERSION},
    {VK_EXT_DEBUG_MARKER_EXTENSION_NAME, VK_EXT_DEBUG_MARKER_SPEC_VERSION},
    {VK_EXT_TOOLING_INFO_EXTENSION_NAME, VK_EXT_TOOLING_INFO_SPEC_VERSION}
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

extern const std::unordered_map<std::string, function_data> name_to_funcptr_map;

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

    // Add VOs to dispatch vector. Order here will be the validation dispatch order!
    auto thread_checker_obj = new ThreadSafety(nullptr);
    thread_checker_obj->RegisterValidationObject(!local_disables[thread_safety], api_version, report_data, local_object_dispatch);

    auto parameter_validation_obj = new StatelessValidation;
    parameter_validation_obj->RegisterValidationObject(!local_disables[stateless_checks], api_version, report_data, local_object_dispatch);

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

    // If handle wrapping is disabled via the ValidationFeatures extension, override build flag
    if (local_disables[handle_wrapping]) {
        wrap_handles = false;
    }

    // Init dispatch array and call registration functions
    bool skip = false;
    for (auto intercept : local_object_dispatch) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateInstance(pCreateInfo, pAllocator, pInstance);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : local_object_dispatch) {
        auto lock = intercept->write_lock();
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

    thread_checker_obj->FinalizeInstanceValidationObject(framework);
    object_tracker_obj->FinalizeInstanceValidationObject(framework);
    parameter_validation_obj->FinalizeInstanceValidationObject(framework);
    core_checks_obj->FinalizeInstanceValidationObject(framework);
    core_checks_obj->instance = *pInstance;
    core_checks_obj->instance_state = core_checks_obj;
    best_practices_obj->FinalizeInstanceValidationObject(framework);
    gpu_assisted_obj->FinalizeInstanceValidationObject(framework);
    debug_printf_obj->FinalizeInstanceValidationObject(framework);
    sync_validation_obj->FinalizeInstanceValidationObject(framework);

    for (auto intercept : framework->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCreateInstance(pCreateInfo, pAllocator, pInstance, result);
    }

    // Delete unused validation objects to avoid memory leak.
    std::vector<ValidationObject*> local_objs = {
        thread_checker_obj, object_tracker_obj, parameter_validation_obj,
        core_checks_obj, best_practices_obj, gpu_assisted_obj, debug_printf_obj,
        sync_validation_obj,
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

    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateDestroyInstance(instance, pAllocator);
        //if (skip) return; // WORKAROUD: does not currently work properly
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordDestroyInstance(instance, pAllocator);
    }

    layer_data->instance_dispatch_table.DestroyInstance(instance, pAllocator);

    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateDevice(gpu, pCreateInfo, pAllocator, pDevice);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : instance_interceptor->object_dispatch) {
        auto lock = intercept->write_lock();
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

    auto thread_safety_obj = new ThreadSafety(reinterpret_cast<ThreadSafety *>(instance_interceptor->GetValidationObject(instance_interceptor->object_dispatch, LayerObjectTypeThreading)));
    thread_safety_obj->InitDeviceValidationObject(!disables[thread_safety], instance_interceptor, device_interceptor);

    auto stateless_validation_obj = new StatelessValidation;
    stateless_validation_obj->InitDeviceValidationObject(!disables[stateless_checks], instance_interceptor, device_interceptor);

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

    // Delete unused validation objects to avoid memory leak.
    std::vector<ValidationObject *> local_objs = {
        thread_safety_obj, stateless_validation_obj, object_tracker_obj,
        core_checks_obj, best_practices_obj, gpu_assisted_obj, debug_printf_obj,
        sync_validation_obj,
    };
    for (auto obj : local_objs) {
        if (std::find(device_interceptor->object_dispatch.begin(), device_interceptor->object_dispatch.end(), obj) ==
            device_interceptor->object_dispatch.end()) {
            delete obj;
        }
    }

    for (auto intercept : instance_interceptor->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCreateDevice(gpu, pCreateInfo, pAllocator, pDevice, result);
    }

    device_interceptor->InitObjectDispatchVectors();

    DeviceExtensionWhitelist(device_interceptor, pCreateInfo, *pDevice);

    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyDevice(VkDevice device, const VkAllocationCallbacks *pAllocator) {
    dispatch_key key = get_dispatch_key(device);
    auto layer_data = GetLayerDataPtr(key, layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateDestroyDevice(device, pAllocator);
        if (skip) return;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordDestroyDevice(device, pAllocator);
    }

    layer_data->device_dispatch_table.DestroyDevice(device, pAllocator);

    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateGraphicsPipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, &(cgpl_state[intercept->container_type]));
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCreateGraphicsPipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, &(cgpl_state[intercept->container_type]));
    }

    auto usepCreateInfos = (!cgpl_state[LayerObjectTypeGpuAssisted].pCreateInfos) ? pCreateInfos : cgpl_state[LayerObjectTypeGpuAssisted].pCreateInfos;
    if (cgpl_state[LayerObjectTypeDebugPrintf].pCreateInfos) usepCreateInfos = cgpl_state[LayerObjectTypeDebugPrintf].pCreateInfos;

    VkResult result = DispatchCreateGraphicsPipelines(device, pipelineCache, createInfoCount, usepCreateInfos, pAllocator, pPipelines);

    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateComputePipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, &(ccpl_state[intercept->container_type]));
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCreateComputePipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, &(ccpl_state[intercept->container_type]));
    }

    auto usepCreateInfos = (!ccpl_state[LayerObjectTypeGpuAssisted].pCreateInfos) ? pCreateInfos : ccpl_state[LayerObjectTypeGpuAssisted].pCreateInfos;
    if (ccpl_state[LayerObjectTypeDebugPrintf].pCreateInfos) usepCreateInfos = ccpl_state[LayerObjectTypeDebugPrintf].pCreateInfos;

    VkResult result = DispatchCreateComputePipelines(device, pipelineCache, createInfoCount, usepCreateInfos, pAllocator, pPipelines);

    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCreateComputePipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, result, &(ccpl_state[intercept->container_type]));
    }
    return result;
}

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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateRayTracingPipelinesNV(device, pipelineCache, createInfoCount, pCreateInfos,
                                                                      pAllocator, pPipelines, &(crtpl_state[intercept->container_type]));
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCreateRayTracingPipelinesNV(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator,
                                                            pPipelines, &(crtpl_state[intercept->container_type]));
    }

    VkResult result = DispatchCreateRayTracingPipelinesNV(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);

    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCreateRayTracingPipelinesNV(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator,
                                                             pPipelines, result, &(crtpl_state[intercept->container_type]));
    }
    return result;
}

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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateRayTracingPipelinesKHR(device, deferredOperation, pipelineCache, createInfoCount, pCreateInfos,
                                                                      pAllocator, pPipelines, &(crtpl_state[intercept->container_type]));
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCreateRayTracingPipelinesKHR(device, deferredOperation, pipelineCache, createInfoCount, pCreateInfos, pAllocator,
                                                            pPipelines, &(crtpl_state[intercept->container_type]));
    }

    VkResult result = DispatchCreateRayTracingPipelinesKHR(device, deferredOperation, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);

    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCreateRayTracingPipelinesKHR(device, deferredOperation, pipelineCache, createInfoCount, pCreateInfos, pAllocator,
                                                             pPipelines, result, &(crtpl_state[intercept->container_type]));
    }
    return result;
}

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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout, &cpl_state);
    }
    VkResult result = DispatchCreatePipelineLayout(device, &cpl_state.modified_create_info, pAllocator, pPipelineLayout);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout, result);
    }
    return result;
}

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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateShaderModule(device, pCreateInfo, pAllocator, pShaderModule, &csm_state);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCreateShaderModule(device, pCreateInfo, pAllocator, pShaderModule, &csm_state);
    }
    VkResult result = DispatchCreateShaderModule(device, &csm_state.instrumented_create_info, pAllocator, pShaderModule);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCreateShaderModule(device, pCreateInfo, pAllocator, pShaderModule, result, &csm_state);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL AllocateDescriptorSets(
    VkDevice                                    device,
    const VkDescriptorSetAllocateInfo*          pAllocateInfo,
    VkDescriptorSet*                            pDescriptorSets) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;

    cvdescriptorset::AllocateDescriptorSetsData ads_state[LayerObjectTypeMaxEnum];

    for (auto intercept : layer_data->object_dispatch) {
        ads_state[intercept->container_type].Init(pAllocateInfo->descriptorSetCount);
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateAllocateDescriptorSets(device,
            pAllocateInfo, pDescriptorSets, &(ads_state[intercept->container_type]));
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordAllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets);
    }
    VkResult result = DispatchAllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordAllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets,
            result, &(ads_state[intercept->container_type]));
    }
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateBuffer(device, pCreateInfo, pAllocator, pBuffer);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCreateBuffer(device, pCreateInfo, pAllocator, pBuffer, &cb_state);
    }
    VkResult result = DispatchCreateBuffer(device, &cb_state.modified_create_info, pAllocator, pBuffer);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCreateBuffer(device, pCreateInfo, pAllocator, pBuffer, result);
    }
    return result;
}


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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceToolPropertiesEXT(physicalDevice, pToolCount, pToolProperties);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }

    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetPhysicalDeviceToolPropertiesEXT(physicalDevice, pToolCount, pToolProperties);
    }

    VkResult result = DispatchGetPhysicalDeviceToolPropertiesEXT(physicalDevice, pToolCount, pToolProperties);

    if (original_pToolProperties != nullptr) {
        pToolProperties = original_pToolProperties;
    }
    (*pToolCount)++;

    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetPhysicalDeviceToolPropertiesEXT(physicalDevice, pToolCount, pToolProperties, result);
    }
    return result;
}


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
        auto lock = validation_data->write_lock();
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
        auto lock = validation_data->write_lock();
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
        auto lock = validation_data->write_lock();
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
        auto lock = validation_data->write_lock();
        result = validation_data->CoreLayerGetValidationCacheDataEXT(device, validationCache, pDataSize, pData);
    }
    return result;

}


VKAPI_ATTR VkResult VKAPI_CALL EnumeratePhysicalDevices(
    VkInstance                                  instance,
    uint32_t*                                   pPhysicalDeviceCount,
    VkPhysicalDevice*                           pPhysicalDevices) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateEnumeratePhysicalDevices(instance, pPhysicalDeviceCount, pPhysicalDevices);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordEnumeratePhysicalDevices(instance, pPhysicalDeviceCount, pPhysicalDevices);
    }
    VkResult result = DispatchEnumeratePhysicalDevices(instance, pPhysicalDeviceCount, pPhysicalDevices);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceFeatures(physicalDevice, pFeatures);
        if (skip) return;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetPhysicalDeviceFeatures(physicalDevice, pFeatures);
    }
    DispatchGetPhysicalDeviceFeatures(physicalDevice, pFeatures);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceFormatProperties(physicalDevice, format, pFormatProperties);
        if (skip) return;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetPhysicalDeviceFormatProperties(physicalDevice, format, pFormatProperties);
    }
    DispatchGetPhysicalDeviceFormatProperties(physicalDevice, format, pFormatProperties);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceImageFormatProperties(physicalDevice, format, type, tiling, usage, flags, pImageFormatProperties);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetPhysicalDeviceImageFormatProperties(physicalDevice, format, type, tiling, usage, flags, pImageFormatProperties);
    }
    VkResult result = DispatchGetPhysicalDeviceImageFormatProperties(physicalDevice, format, type, tiling, usage, flags, pImageFormatProperties);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceProperties(physicalDevice, pProperties);
        if (skip) return;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetPhysicalDeviceProperties(physicalDevice, pProperties);
    }
    DispatchGetPhysicalDeviceProperties(physicalDevice, pProperties);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceQueueFamilyProperties(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
        if (skip) return;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetPhysicalDeviceQueueFamilyProperties(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
    }
    DispatchGetPhysicalDeviceQueueFamilyProperties(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetPhysicalDeviceQueueFamilyProperties(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
    }
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceMemoryProperties(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceMemoryProperties*           pMemoryProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceMemoryProperties(physicalDevice, pMemoryProperties);
        if (skip) return;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetPhysicalDeviceMemoryProperties(physicalDevice, pMemoryProperties);
    }
    DispatchGetPhysicalDeviceMemoryProperties(physicalDevice, pMemoryProperties);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetDeviceQueue(device, queueFamilyIndex, queueIndex, pQueue);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetDeviceQueue]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetDeviceQueue(device, queueFamilyIndex, queueIndex, pQueue);
    }
    DispatchGetDeviceQueue(device, queueFamilyIndex, queueIndex, pQueue);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetDeviceQueue]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateQueueSubmit(queue, submitCount, pSubmits, fence);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordQueueSubmit]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordQueueSubmit(queue, submitCount, pSubmits, fence);
    }
    VkResult result = DispatchQueueSubmit(queue, submitCount, pSubmits, fence);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordQueueSubmit]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordQueueSubmit(queue, submitCount, pSubmits, fence, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL QueueWaitIdle(
    VkQueue                                     queue) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(queue), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateQueueWaitIdle]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateQueueWaitIdle(queue);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordQueueWaitIdle]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordQueueWaitIdle(queue);
    }
    VkResult result = DispatchQueueWaitIdle(queue);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordQueueWaitIdle]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordQueueWaitIdle(queue, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL DeviceWaitIdle(
    VkDevice                                    device) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDeviceWaitIdle]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateDeviceWaitIdle(device);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDeviceWaitIdle]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordDeviceWaitIdle(device);
    }
    VkResult result = DispatchDeviceWaitIdle(device);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDeviceWaitIdle]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateAllocateMemory(device, pAllocateInfo, pAllocator, pMemory);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordAllocateMemory]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordAllocateMemory(device, pAllocateInfo, pAllocator, pMemory);
    }
    VkResult result = DispatchAllocateMemory(device, pAllocateInfo, pAllocator, pMemory);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordAllocateMemory]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordAllocateMemory(device, pAllocateInfo, pAllocator, pMemory, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL FreeMemory(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    const VkAllocationCallbacks*                pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateFreeMemory]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateFreeMemory(device, memory, pAllocator);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordFreeMemory]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordFreeMemory(device, memory, pAllocator);
    }
    DispatchFreeMemory(device, memory, pAllocator);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordFreeMemory]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordFreeMemory(device, memory, pAllocator);
    }
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateMapMemory(device, memory, offset, size, flags, ppData);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordMapMemory]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordMapMemory(device, memory, offset, size, flags, ppData);
    }
    VkResult result = DispatchMapMemory(device, memory, offset, size, flags, ppData);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordMapMemory]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateUnmapMemory(device, memory);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordUnmapMemory]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordUnmapMemory(device, memory);
    }
    DispatchUnmapMemory(device, memory);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordUnmapMemory]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateFlushMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordFlushMappedMemoryRanges]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordFlushMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
    }
    VkResult result = DispatchFlushMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordFlushMappedMemoryRanges]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateInvalidateMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordInvalidateMappedMemoryRanges]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordInvalidateMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
    }
    VkResult result = DispatchInvalidateMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordInvalidateMappedMemoryRanges]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetDeviceMemoryCommitment(device, memory, pCommittedMemoryInBytes);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetDeviceMemoryCommitment]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetDeviceMemoryCommitment(device, memory, pCommittedMemoryInBytes);
    }
    DispatchGetDeviceMemoryCommitment(device, memory, pCommittedMemoryInBytes);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetDeviceMemoryCommitment]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateBindBufferMemory(device, buffer, memory, memoryOffset);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordBindBufferMemory]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordBindBufferMemory(device, buffer, memory, memoryOffset);
    }
    VkResult result = DispatchBindBufferMemory(device, buffer, memory, memoryOffset);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordBindBufferMemory]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateBindImageMemory(device, image, memory, memoryOffset);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordBindImageMemory]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordBindImageMemory(device, image, memory, memoryOffset);
    }
    VkResult result = DispatchBindImageMemory(device, image, memory, memoryOffset);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordBindImageMemory]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetBufferMemoryRequirements(device, buffer, pMemoryRequirements);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetBufferMemoryRequirements]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetBufferMemoryRequirements(device, buffer, pMemoryRequirements);
    }
    DispatchGetBufferMemoryRequirements(device, buffer, pMemoryRequirements);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetBufferMemoryRequirements]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetImageMemoryRequirements(device, image, pMemoryRequirements);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetImageMemoryRequirements]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetImageMemoryRequirements(device, image, pMemoryRequirements);
    }
    DispatchGetImageMemoryRequirements(device, image, pMemoryRequirements);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetImageMemoryRequirements]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetImageMemoryRequirements(device, image, pMemoryRequirements);
    }
}

VKAPI_ATTR void VKAPI_CALL GetImageSparseMemoryRequirements(
    VkDevice                                    device,
    VkImage                                     image,
    uint32_t*                                   pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements*            pSparseMemoryRequirements) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetImageSparseMemoryRequirements]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetImageSparseMemoryRequirements(device, image, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetImageSparseMemoryRequirements]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetImageSparseMemoryRequirements(device, image, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
    }
    DispatchGetImageSparseMemoryRequirements(device, image, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetImageSparseMemoryRequirements]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetImageSparseMemoryRequirements(device, image, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
    }
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceSparseImageFormatProperties(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkImageType                                 type,
    VkSampleCountFlagBits                       samples,
    VkImageUsageFlags                           usage,
    VkImageTiling                               tiling,
    uint32_t*                                   pPropertyCount,
    VkSparseImageFormatProperties*              pProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceSparseImageFormatProperties(physicalDevice, format, type, samples, usage, tiling, pPropertyCount, pProperties);
        if (skip) return;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetPhysicalDeviceSparseImageFormatProperties(physicalDevice, format, type, samples, usage, tiling, pPropertyCount, pProperties);
    }
    DispatchGetPhysicalDeviceSparseImageFormatProperties(physicalDevice, format, type, samples, usage, tiling, pPropertyCount, pProperties);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetPhysicalDeviceSparseImageFormatProperties(physicalDevice, format, type, samples, usage, tiling, pPropertyCount, pProperties);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL QueueBindSparse(
    VkQueue                                     queue,
    uint32_t                                    bindInfoCount,
    const VkBindSparseInfo*                     pBindInfo,
    VkFence                                     fence) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(queue), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateQueueBindSparse]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateQueueBindSparse(queue, bindInfoCount, pBindInfo, fence);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordQueueBindSparse]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordQueueBindSparse(queue, bindInfoCount, pBindInfo, fence);
    }
    VkResult result = DispatchQueueBindSparse(queue, bindInfoCount, pBindInfo, fence);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordQueueBindSparse]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordQueueBindSparse(queue, bindInfoCount, pBindInfo, fence, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL CreateFence(
    VkDevice                                    device,
    const VkFenceCreateInfo*                    pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFence*                                    pFence) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCreateFence]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateFence(device, pCreateInfo, pAllocator, pFence);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateFence]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCreateFence(device, pCreateInfo, pAllocator, pFence);
    }
    VkResult result = DispatchCreateFence(device, pCreateInfo, pAllocator, pFence);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateFence]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateDestroyFence(device, fence, pAllocator);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroyFence]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordDestroyFence(device, fence, pAllocator);
    }
    DispatchDestroyFence(device, fence, pAllocator);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroyFence]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateResetFences(device, fenceCount, pFences);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordResetFences]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordResetFences(device, fenceCount, pFences);
    }
    VkResult result = DispatchResetFences(device, fenceCount, pFences);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordResetFences]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetFenceStatus(device, fence);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetFenceStatus]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetFenceStatus(device, fence);
    }
    VkResult result = DispatchGetFenceStatus(device, fence);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetFenceStatus]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateWaitForFences(device, fenceCount, pFences, waitAll, timeout);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordWaitForFences]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordWaitForFences(device, fenceCount, pFences, waitAll, timeout);
    }
    VkResult result = DispatchWaitForFences(device, fenceCount, pFences, waitAll, timeout);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordWaitForFences]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateSemaphore(device, pCreateInfo, pAllocator, pSemaphore);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateSemaphore]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCreateSemaphore(device, pCreateInfo, pAllocator, pSemaphore);
    }
    VkResult result = DispatchCreateSemaphore(device, pCreateInfo, pAllocator, pSemaphore);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateSemaphore]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateDestroySemaphore(device, semaphore, pAllocator);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroySemaphore]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordDestroySemaphore(device, semaphore, pAllocator);
    }
    DispatchDestroySemaphore(device, semaphore, pAllocator);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroySemaphore]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateEvent(device, pCreateInfo, pAllocator, pEvent);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateEvent]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCreateEvent(device, pCreateInfo, pAllocator, pEvent);
    }
    VkResult result = DispatchCreateEvent(device, pCreateInfo, pAllocator, pEvent);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateEvent]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateDestroyEvent(device, event, pAllocator);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroyEvent]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordDestroyEvent(device, event, pAllocator);
    }
    DispatchDestroyEvent(device, event, pAllocator);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroyEvent]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordDestroyEvent(device, event, pAllocator);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL GetEventStatus(
    VkDevice                                    device,
    VkEvent                                     event) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetEventStatus]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetEventStatus(device, event);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetEventStatus]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetEventStatus(device, event);
    }
    VkResult result = DispatchGetEventStatus(device, event);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetEventStatus]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateSetEvent(device, event);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordSetEvent]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordSetEvent(device, event);
    }
    VkResult result = DispatchSetEvent(device, event);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordSetEvent]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateResetEvent(device, event);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordResetEvent]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordResetEvent(device, event);
    }
    VkResult result = DispatchResetEvent(device, event);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordResetEvent]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateQueryPool(device, pCreateInfo, pAllocator, pQueryPool);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateQueryPool]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCreateQueryPool(device, pCreateInfo, pAllocator, pQueryPool);
    }
    VkResult result = DispatchCreateQueryPool(device, pCreateInfo, pAllocator, pQueryPool);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateQueryPool]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCreateQueryPool(device, pCreateInfo, pAllocator, pQueryPool, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyQueryPool(
    VkDevice                                    device,
    VkQueryPool                                 queryPool,
    const VkAllocationCallbacks*                pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDestroyQueryPool]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateDestroyQueryPool(device, queryPool, pAllocator);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroyQueryPool]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordDestroyQueryPool(device, queryPool, pAllocator);
    }
    DispatchDestroyQueryPool(device, queryPool, pAllocator);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroyQueryPool]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordDestroyQueryPool(device, queryPool, pAllocator);
    }
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetQueryPoolResults(device, queryPool, firstQuery, queryCount, dataSize, pData, stride, flags);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetQueryPoolResults]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetQueryPoolResults(device, queryPool, firstQuery, queryCount, dataSize, pData, stride, flags);
    }
    VkResult result = DispatchGetQueryPoolResults(device, queryPool, firstQuery, queryCount, dataSize, pData, stride, flags);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetQueryPoolResults]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateDestroyBuffer(device, buffer, pAllocator);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroyBuffer]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordDestroyBuffer(device, buffer, pAllocator);
    }
    DispatchDestroyBuffer(device, buffer, pAllocator);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroyBuffer]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateBufferView(device, pCreateInfo, pAllocator, pView);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateBufferView]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCreateBufferView(device, pCreateInfo, pAllocator, pView);
    }
    VkResult result = DispatchCreateBufferView(device, pCreateInfo, pAllocator, pView);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateBufferView]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateDestroyBufferView(device, bufferView, pAllocator);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroyBufferView]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordDestroyBufferView(device, bufferView, pAllocator);
    }
    DispatchDestroyBufferView(device, bufferView, pAllocator);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroyBufferView]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateImage(device, pCreateInfo, pAllocator, pImage);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateImage]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCreateImage(device, pCreateInfo, pAllocator, pImage);
    }
    VkResult result = DispatchCreateImage(device, pCreateInfo, pAllocator, pImage);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateImage]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateDestroyImage(device, image, pAllocator);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroyImage]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordDestroyImage(device, image, pAllocator);
    }
    DispatchDestroyImage(device, image, pAllocator);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroyImage]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetImageSubresourceLayout(device, image, pSubresource, pLayout);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetImageSubresourceLayout]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetImageSubresourceLayout(device, image, pSubresource, pLayout);
    }
    DispatchGetImageSubresourceLayout(device, image, pSubresource, pLayout);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetImageSubresourceLayout]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateImageView(device, pCreateInfo, pAllocator, pView);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateImageView]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCreateImageView(device, pCreateInfo, pAllocator, pView);
    }
    VkResult result = DispatchCreateImageView(device, pCreateInfo, pAllocator, pView);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateImageView]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateDestroyImageView(device, imageView, pAllocator);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroyImageView]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordDestroyImageView(device, imageView, pAllocator);
    }
    DispatchDestroyImageView(device, imageView, pAllocator);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroyImageView]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordDestroyImageView(device, imageView, pAllocator);
    }
}

VKAPI_ATTR void VKAPI_CALL DestroyShaderModule(
    VkDevice                                    device,
    VkShaderModule                              shaderModule,
    const VkAllocationCallbacks*                pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDestroyShaderModule]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateDestroyShaderModule(device, shaderModule, pAllocator);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroyShaderModule]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordDestroyShaderModule(device, shaderModule, pAllocator);
    }
    DispatchDestroyShaderModule(device, shaderModule, pAllocator);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroyShaderModule]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordDestroyShaderModule(device, shaderModule, pAllocator);
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreatePipelineCache(device, pCreateInfo, pAllocator, pPipelineCache);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreatePipelineCache]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCreatePipelineCache(device, pCreateInfo, pAllocator, pPipelineCache);
    }
    VkResult result = DispatchCreatePipelineCache(device, pCreateInfo, pAllocator, pPipelineCache);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreatePipelineCache]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateDestroyPipelineCache(device, pipelineCache, pAllocator);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroyPipelineCache]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordDestroyPipelineCache(device, pipelineCache, pAllocator);
    }
    DispatchDestroyPipelineCache(device, pipelineCache, pAllocator);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroyPipelineCache]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordDestroyPipelineCache(device, pipelineCache, pAllocator);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL GetPipelineCacheData(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    size_t*                                     pDataSize,
    void*                                       pData) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetPipelineCacheData]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPipelineCacheData(device, pipelineCache, pDataSize, pData);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetPipelineCacheData]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetPipelineCacheData(device, pipelineCache, pDataSize, pData);
    }
    VkResult result = DispatchGetPipelineCacheData(device, pipelineCache, pDataSize, pData);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetPipelineCacheData]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetPipelineCacheData(device, pipelineCache, pDataSize, pData, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL MergePipelineCaches(
    VkDevice                                    device,
    VkPipelineCache                             dstCache,
    uint32_t                                    srcCacheCount,
    const VkPipelineCache*                      pSrcCaches) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateMergePipelineCaches]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateMergePipelineCaches(device, dstCache, srcCacheCount, pSrcCaches);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordMergePipelineCaches]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordMergePipelineCaches(device, dstCache, srcCacheCount, pSrcCaches);
    }
    VkResult result = DispatchMergePipelineCaches(device, dstCache, srcCacheCount, pSrcCaches);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordMergePipelineCaches]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordMergePipelineCaches(device, dstCache, srcCacheCount, pSrcCaches, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyPipeline(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    const VkAllocationCallbacks*                pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDestroyPipeline]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateDestroyPipeline(device, pipeline, pAllocator);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroyPipeline]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordDestroyPipeline(device, pipeline, pAllocator);
    }
    DispatchDestroyPipeline(device, pipeline, pAllocator);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroyPipeline]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateDestroyPipelineLayout(device, pipelineLayout, pAllocator);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroyPipelineLayout]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordDestroyPipelineLayout(device, pipelineLayout, pAllocator);
    }
    DispatchDestroyPipelineLayout(device, pipelineLayout, pAllocator);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroyPipelineLayout]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateSampler(device, pCreateInfo, pAllocator, pSampler);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateSampler]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCreateSampler(device, pCreateInfo, pAllocator, pSampler);
    }
    VkResult result = DispatchCreateSampler(device, pCreateInfo, pAllocator, pSampler);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateSampler]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateDestroySampler(device, sampler, pAllocator);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroySampler]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordDestroySampler(device, sampler, pAllocator);
    }
    DispatchDestroySampler(device, sampler, pAllocator);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroySampler]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateDescriptorSetLayout(device, pCreateInfo, pAllocator, pSetLayout);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateDescriptorSetLayout]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCreateDescriptorSetLayout(device, pCreateInfo, pAllocator, pSetLayout);
    }
    VkResult result = DispatchCreateDescriptorSetLayout(device, pCreateInfo, pAllocator, pSetLayout);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateDescriptorSetLayout]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateDestroyDescriptorSetLayout(device, descriptorSetLayout, pAllocator);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroyDescriptorSetLayout]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordDestroyDescriptorSetLayout(device, descriptorSetLayout, pAllocator);
    }
    DispatchDestroyDescriptorSetLayout(device, descriptorSetLayout, pAllocator);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroyDescriptorSetLayout]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateDescriptorPool(device, pCreateInfo, pAllocator, pDescriptorPool);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateDescriptorPool]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCreateDescriptorPool(device, pCreateInfo, pAllocator, pDescriptorPool);
    }
    VkResult result = DispatchCreateDescriptorPool(device, pCreateInfo, pAllocator, pDescriptorPool);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateDescriptorPool]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCreateDescriptorPool(device, pCreateInfo, pAllocator, pDescriptorPool, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyDescriptorPool(
    VkDevice                                    device,
    VkDescriptorPool                            descriptorPool,
    const VkAllocationCallbacks*                pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDestroyDescriptorPool]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateDestroyDescriptorPool(device, descriptorPool, pAllocator);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroyDescriptorPool]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordDestroyDescriptorPool(device, descriptorPool, pAllocator);
    }
    DispatchDestroyDescriptorPool(device, descriptorPool, pAllocator);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroyDescriptorPool]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordDestroyDescriptorPool(device, descriptorPool, pAllocator);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL ResetDescriptorPool(
    VkDevice                                    device,
    VkDescriptorPool                            descriptorPool,
    VkDescriptorPoolResetFlags                  flags) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateResetDescriptorPool]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateResetDescriptorPool(device, descriptorPool, flags);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordResetDescriptorPool]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordResetDescriptorPool(device, descriptorPool, flags);
    }
    VkResult result = DispatchResetDescriptorPool(device, descriptorPool, flags);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordResetDescriptorPool]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateFreeDescriptorSets(device, descriptorPool, descriptorSetCount, pDescriptorSets);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordFreeDescriptorSets]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordFreeDescriptorSets(device, descriptorPool, descriptorSetCount, pDescriptorSets);
    }
    VkResult result = DispatchFreeDescriptorSets(device, descriptorPool, descriptorSetCount, pDescriptorSets);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordFreeDescriptorSets]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateUpdateDescriptorSets(device, descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordUpdateDescriptorSets]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordUpdateDescriptorSets(device, descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies);
    }
    DispatchUpdateDescriptorSets(device, descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordUpdateDescriptorSets]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateFramebuffer(device, pCreateInfo, pAllocator, pFramebuffer);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateFramebuffer]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCreateFramebuffer(device, pCreateInfo, pAllocator, pFramebuffer);
    }
    VkResult result = DispatchCreateFramebuffer(device, pCreateInfo, pAllocator, pFramebuffer);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateFramebuffer]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateDestroyFramebuffer(device, framebuffer, pAllocator);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroyFramebuffer]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordDestroyFramebuffer(device, framebuffer, pAllocator);
    }
    DispatchDestroyFramebuffer(device, framebuffer, pAllocator);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroyFramebuffer]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateRenderPass(device, pCreateInfo, pAllocator, pRenderPass);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateRenderPass]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCreateRenderPass(device, pCreateInfo, pAllocator, pRenderPass);
    }
    VkResult result = DispatchCreateRenderPass(device, pCreateInfo, pAllocator, pRenderPass);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateRenderPass]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateDestroyRenderPass(device, renderPass, pAllocator);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroyRenderPass]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordDestroyRenderPass(device, renderPass, pAllocator);
    }
    DispatchDestroyRenderPass(device, renderPass, pAllocator);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroyRenderPass]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetRenderAreaGranularity(device, renderPass, pGranularity);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetRenderAreaGranularity]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetRenderAreaGranularity(device, renderPass, pGranularity);
    }
    DispatchGetRenderAreaGranularity(device, renderPass, pGranularity);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetRenderAreaGranularity]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateCommandPool(device, pCreateInfo, pAllocator, pCommandPool);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateCommandPool]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCreateCommandPool(device, pCreateInfo, pAllocator, pCommandPool);
    }
    VkResult result = DispatchCreateCommandPool(device, pCreateInfo, pAllocator, pCommandPool);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateCommandPool]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCreateCommandPool(device, pCreateInfo, pAllocator, pCommandPool, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyCommandPool(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    const VkAllocationCallbacks*                pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDestroyCommandPool]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateDestroyCommandPool(device, commandPool, pAllocator);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroyCommandPool]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordDestroyCommandPool(device, commandPool, pAllocator);
    }
    DispatchDestroyCommandPool(device, commandPool, pAllocator);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroyCommandPool]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordDestroyCommandPool(device, commandPool, pAllocator);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL ResetCommandPool(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    VkCommandPoolResetFlags                     flags) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateResetCommandPool]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateResetCommandPool(device, commandPool, flags);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordResetCommandPool]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordResetCommandPool(device, commandPool, flags);
    }
    VkResult result = DispatchResetCommandPool(device, commandPool, flags);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordResetCommandPool]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateAllocateCommandBuffers(device, pAllocateInfo, pCommandBuffers);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordAllocateCommandBuffers]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordAllocateCommandBuffers(device, pAllocateInfo, pCommandBuffers);
    }
    VkResult result = DispatchAllocateCommandBuffers(device, pAllocateInfo, pCommandBuffers);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordAllocateCommandBuffers]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateFreeCommandBuffers(device, commandPool, commandBufferCount, pCommandBuffers);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordFreeCommandBuffers]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordFreeCommandBuffers(device, commandPool, commandBufferCount, pCommandBuffers);
    }
    DispatchFreeCommandBuffers(device, commandPool, commandBufferCount, pCommandBuffers);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordFreeCommandBuffers]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordFreeCommandBuffers(device, commandPool, commandBufferCount, pCommandBuffers);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL BeginCommandBuffer(
    VkCommandBuffer                             commandBuffer,
    const VkCommandBufferBeginInfo*             pBeginInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateBeginCommandBuffer]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateBeginCommandBuffer(commandBuffer, pBeginInfo);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordBeginCommandBuffer]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordBeginCommandBuffer(commandBuffer, pBeginInfo);
    }
    VkResult result = DispatchBeginCommandBuffer(commandBuffer, pBeginInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordBeginCommandBuffer]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordBeginCommandBuffer(commandBuffer, pBeginInfo, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL EndCommandBuffer(
    VkCommandBuffer                             commandBuffer) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateEndCommandBuffer]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateEndCommandBuffer(commandBuffer);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordEndCommandBuffer]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordEndCommandBuffer(commandBuffer);
    }
    VkResult result = DispatchEndCommandBuffer(commandBuffer);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordEndCommandBuffer]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateResetCommandBuffer(commandBuffer, flags);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordResetCommandBuffer]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordResetCommandBuffer(commandBuffer, flags);
    }
    VkResult result = DispatchResetCommandBuffer(commandBuffer, flags);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordResetCommandBuffer]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdBindPipeline]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline);
    }
    DispatchCmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdBindPipeline]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetViewport(commandBuffer, firstViewport, viewportCount, pViewports);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetViewport]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdSetViewport(commandBuffer, firstViewport, viewportCount, pViewports);
    }
    DispatchCmdSetViewport(commandBuffer, firstViewport, viewportCount, pViewports);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetViewport]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetScissor(commandBuffer, firstScissor, scissorCount, pScissors);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetScissor]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdSetScissor(commandBuffer, firstScissor, scissorCount, pScissors);
    }
    DispatchCmdSetScissor(commandBuffer, firstScissor, scissorCount, pScissors);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetScissor]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdSetScissor(commandBuffer, firstScissor, scissorCount, pScissors);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetLineWidth(
    VkCommandBuffer                             commandBuffer,
    float                                       lineWidth) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetLineWidth]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetLineWidth(commandBuffer, lineWidth);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetLineWidth]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdSetLineWidth(commandBuffer, lineWidth);
    }
    DispatchCmdSetLineWidth(commandBuffer, lineWidth);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetLineWidth]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetDepthBias(commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetDepthBias]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdSetDepthBias(commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
    }
    DispatchCmdSetDepthBias(commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetDepthBias]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdSetDepthBias(commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetBlendConstants(
    VkCommandBuffer                             commandBuffer,
    const float                                 blendConstants[4]) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetBlendConstants]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetBlendConstants(commandBuffer, blendConstants);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetBlendConstants]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdSetBlendConstants(commandBuffer, blendConstants);
    }
    DispatchCmdSetBlendConstants(commandBuffer, blendConstants);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetBlendConstants]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetDepthBounds(commandBuffer, minDepthBounds, maxDepthBounds);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetDepthBounds]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdSetDepthBounds(commandBuffer, minDepthBounds, maxDepthBounds);
    }
    DispatchCmdSetDepthBounds(commandBuffer, minDepthBounds, maxDepthBounds);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetDepthBounds]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetStencilCompareMask(commandBuffer, faceMask, compareMask);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetStencilCompareMask]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdSetStencilCompareMask(commandBuffer, faceMask, compareMask);
    }
    DispatchCmdSetStencilCompareMask(commandBuffer, faceMask, compareMask);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetStencilCompareMask]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetStencilWriteMask(commandBuffer, faceMask, writeMask);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetStencilWriteMask]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdSetStencilWriteMask(commandBuffer, faceMask, writeMask);
    }
    DispatchCmdSetStencilWriteMask(commandBuffer, faceMask, writeMask);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetStencilWriteMask]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetStencilReference(commandBuffer, faceMask, reference);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetStencilReference]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdSetStencilReference(commandBuffer, faceMask, reference);
    }
    DispatchCmdSetStencilReference(commandBuffer, faceMask, reference);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetStencilReference]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdBindDescriptorSets(commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdBindDescriptorSets]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdBindDescriptorSets(commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
    }
    DispatchCmdBindDescriptorSets(commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdBindDescriptorSets]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdBindIndexBuffer(commandBuffer, buffer, offset, indexType);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdBindIndexBuffer]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdBindIndexBuffer(commandBuffer, buffer, offset, indexType);
    }
    DispatchCmdBindIndexBuffer(commandBuffer, buffer, offset, indexType);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdBindIndexBuffer]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdBindVertexBuffers(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdBindVertexBuffers]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdBindVertexBuffers(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);
    }
    DispatchCmdBindVertexBuffers(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdBindVertexBuffers]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdDraw]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
    }
    DispatchCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdDraw]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdDrawIndexed]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    }
    DispatchCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdDrawIndexed]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdDrawIndirect(commandBuffer, buffer, offset, drawCount, stride);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdDrawIndirect]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdDrawIndirect(commandBuffer, buffer, offset, drawCount, stride);
    }
    DispatchCmdDrawIndirect(commandBuffer, buffer, offset, drawCount, stride);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdDrawIndirect]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdDrawIndexedIndirect(commandBuffer, buffer, offset, drawCount, stride);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdDrawIndexedIndirect]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdDrawIndexedIndirect(commandBuffer, buffer, offset, drawCount, stride);
    }
    DispatchCmdDrawIndexedIndirect(commandBuffer, buffer, offset, drawCount, stride);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdDrawIndexedIndirect]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdDispatch(commandBuffer, groupCountX, groupCountY, groupCountZ);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdDispatch]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdDispatch(commandBuffer, groupCountX, groupCountY, groupCountZ);
    }
    DispatchCmdDispatch(commandBuffer, groupCountX, groupCountY, groupCountZ);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdDispatch]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdDispatchIndirect(commandBuffer, buffer, offset);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdDispatchIndirect]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdDispatchIndirect(commandBuffer, buffer, offset);
    }
    DispatchCmdDispatchIndirect(commandBuffer, buffer, offset);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdDispatchIndirect]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdCopyBuffer]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
    }
    DispatchCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdCopyBuffer]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdCopyImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdCopyImage]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdCopyImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
    }
    DispatchCmdCopyImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdCopyImage]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdBlitImage]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter);
    }
    DispatchCmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdBlitImage]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdCopyBufferToImage]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
    }
    DispatchCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdCopyBufferToImage]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdCopyImageToBuffer]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);
    }
    DispatchCmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdCopyImageToBuffer]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdUpdateBuffer(commandBuffer, dstBuffer, dstOffset, dataSize, pData);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdUpdateBuffer]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdUpdateBuffer(commandBuffer, dstBuffer, dstOffset, dataSize, pData);
    }
    DispatchCmdUpdateBuffer(commandBuffer, dstBuffer, dstOffset, dataSize, pData);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdUpdateBuffer]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdFillBuffer(commandBuffer, dstBuffer, dstOffset, size, data);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdFillBuffer]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdFillBuffer(commandBuffer, dstBuffer, dstOffset, size, data);
    }
    DispatchCmdFillBuffer(commandBuffer, dstBuffer, dstOffset, size, data);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdFillBuffer]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdClearColorImage(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdClearColorImage]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdClearColorImage(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);
    }
    DispatchCmdClearColorImage(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdClearColorImage]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdClearDepthStencilImage(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdClearDepthStencilImage]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdClearDepthStencilImage(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges);
    }
    DispatchCmdClearDepthStencilImage(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdClearDepthStencilImage]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdClearAttachments(commandBuffer, attachmentCount, pAttachments, rectCount, pRects);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdClearAttachments]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdClearAttachments(commandBuffer, attachmentCount, pAttachments, rectCount, pRects);
    }
    DispatchCmdClearAttachments(commandBuffer, attachmentCount, pAttachments, rectCount, pRects);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdClearAttachments]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdResolveImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdResolveImage]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdResolveImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
    }
    DispatchCmdResolveImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdResolveImage]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetEvent(commandBuffer, event, stageMask);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetEvent]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdSetEvent(commandBuffer, event, stageMask);
    }
    DispatchCmdSetEvent(commandBuffer, event, stageMask);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetEvent]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdResetEvent(commandBuffer, event, stageMask);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdResetEvent]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdResetEvent(commandBuffer, event, stageMask);
    }
    DispatchCmdResetEvent(commandBuffer, event, stageMask);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdResetEvent]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdWaitEvents(commandBuffer, eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdWaitEvents]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdWaitEvents(commandBuffer, eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    }
    DispatchCmdWaitEvents(commandBuffer, eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdWaitEvents]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdPipelineBarrier]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    }
    DispatchCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdPipelineBarrier]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdBeginQuery(commandBuffer, queryPool, query, flags);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdBeginQuery]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdBeginQuery(commandBuffer, queryPool, query, flags);
    }
    DispatchCmdBeginQuery(commandBuffer, queryPool, query, flags);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdBeginQuery]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdEndQuery(commandBuffer, queryPool, query);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdEndQuery]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdEndQuery(commandBuffer, queryPool, query);
    }
    DispatchCmdEndQuery(commandBuffer, queryPool, query);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdEndQuery]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdResetQueryPool(commandBuffer, queryPool, firstQuery, queryCount);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdResetQueryPool]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdResetQueryPool(commandBuffer, queryPool, firstQuery, queryCount);
    }
    DispatchCmdResetQueryPool(commandBuffer, queryPool, firstQuery, queryCount);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdResetQueryPool]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdWriteTimestamp(commandBuffer, pipelineStage, queryPool, query);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdWriteTimestamp]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdWriteTimestamp(commandBuffer, pipelineStage, queryPool, query);
    }
    DispatchCmdWriteTimestamp(commandBuffer, pipelineStage, queryPool, query);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdWriteTimestamp]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdCopyQueryPoolResults(commandBuffer, queryPool, firstQuery, queryCount, dstBuffer, dstOffset, stride, flags);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdCopyQueryPoolResults]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdCopyQueryPoolResults(commandBuffer, queryPool, firstQuery, queryCount, dstBuffer, dstOffset, stride, flags);
    }
    DispatchCmdCopyQueryPoolResults(commandBuffer, queryPool, firstQuery, queryCount, dstBuffer, dstOffset, stride, flags);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdCopyQueryPoolResults]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdPushConstants(commandBuffer, layout, stageFlags, offset, size, pValues);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdPushConstants]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdPushConstants(commandBuffer, layout, stageFlags, offset, size, pValues);
    }
    DispatchCmdPushConstants(commandBuffer, layout, stageFlags, offset, size, pValues);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdPushConstants]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdBeginRenderPass]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);
    }
    DispatchCmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdBeginRenderPass]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdNextSubpass(
    VkCommandBuffer                             commandBuffer,
    VkSubpassContents                           contents) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdNextSubpass]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdNextSubpass(commandBuffer, contents);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdNextSubpass]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdNextSubpass(commandBuffer, contents);
    }
    DispatchCmdNextSubpass(commandBuffer, contents);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdNextSubpass]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdNextSubpass(commandBuffer, contents);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdEndRenderPass(
    VkCommandBuffer                             commandBuffer) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdEndRenderPass]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdEndRenderPass(commandBuffer);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdEndRenderPass]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdEndRenderPass(commandBuffer);
    }
    DispatchCmdEndRenderPass(commandBuffer);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdEndRenderPass]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdExecuteCommands(commandBuffer, commandBufferCount, pCommandBuffers);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdExecuteCommands]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdExecuteCommands(commandBuffer, commandBufferCount, pCommandBuffers);
    }
    DispatchCmdExecuteCommands(commandBuffer, commandBufferCount, pCommandBuffers);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdExecuteCommands]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateBindBufferMemory2(device, bindInfoCount, pBindInfos);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordBindBufferMemory2]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordBindBufferMemory2(device, bindInfoCount, pBindInfos);
    }
    VkResult result = DispatchBindBufferMemory2(device, bindInfoCount, pBindInfos);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordBindBufferMemory2]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateBindImageMemory2(device, bindInfoCount, pBindInfos);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordBindImageMemory2]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordBindImageMemory2(device, bindInfoCount, pBindInfos);
    }
    VkResult result = DispatchBindImageMemory2(device, bindInfoCount, pBindInfos);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordBindImageMemory2]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetDeviceGroupPeerMemoryFeatures(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetDeviceGroupPeerMemoryFeatures]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetDeviceGroupPeerMemoryFeatures(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
    }
    DispatchGetDeviceGroupPeerMemoryFeatures(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetDeviceGroupPeerMemoryFeatures]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetDeviceGroupPeerMemoryFeatures(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetDeviceMask(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    deviceMask) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetDeviceMask]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetDeviceMask(commandBuffer, deviceMask);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetDeviceMask]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdSetDeviceMask(commandBuffer, deviceMask);
    }
    DispatchCmdSetDeviceMask(commandBuffer, deviceMask);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetDeviceMask]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdDispatchBase(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdDispatchBase]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdDispatchBase(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
    }
    DispatchCmdDispatchBase(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdDispatchBase]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateEnumeratePhysicalDeviceGroups(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordEnumeratePhysicalDeviceGroups(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
    }
    VkResult result = DispatchEnumeratePhysicalDeviceGroups(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetImageMemoryRequirements2(device, pInfo, pMemoryRequirements);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetImageMemoryRequirements2]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetImageMemoryRequirements2(device, pInfo, pMemoryRequirements);
    }
    DispatchGetImageMemoryRequirements2(device, pInfo, pMemoryRequirements);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetImageMemoryRequirements2]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetBufferMemoryRequirements2(device, pInfo, pMemoryRequirements);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetBufferMemoryRequirements2]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetBufferMemoryRequirements2(device, pInfo, pMemoryRequirements);
    }
    DispatchGetBufferMemoryRequirements2(device, pInfo, pMemoryRequirements);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetBufferMemoryRequirements2]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetBufferMemoryRequirements2(device, pInfo, pMemoryRequirements);
    }
}

VKAPI_ATTR void VKAPI_CALL GetImageSparseMemoryRequirements2(
    VkDevice                                    device,
    const VkImageSparseMemoryRequirementsInfo2* pInfo,
    uint32_t*                                   pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2*           pSparseMemoryRequirements) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetImageSparseMemoryRequirements2]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetImageSparseMemoryRequirements2(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetImageSparseMemoryRequirements2]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetImageSparseMemoryRequirements2(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
    }
    DispatchGetImageSparseMemoryRequirements2(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetImageSparseMemoryRequirements2]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetImageSparseMemoryRequirements2(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
    }
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceFeatures2(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceFeatures2*                  pFeatures) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceFeatures2(physicalDevice, pFeatures);
        if (skip) return;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetPhysicalDeviceFeatures2(physicalDevice, pFeatures);
    }
    DispatchGetPhysicalDeviceFeatures2(physicalDevice, pFeatures);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetPhysicalDeviceFeatures2(physicalDevice, pFeatures);
    }
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceProperties2(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceProperties2*                pProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceProperties2(physicalDevice, pProperties);
        if (skip) return;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetPhysicalDeviceProperties2(physicalDevice, pProperties);
    }
    DispatchGetPhysicalDeviceProperties2(physicalDevice, pProperties);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceFormatProperties2(physicalDevice, format, pFormatProperties);
        if (skip) return;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetPhysicalDeviceFormatProperties2(physicalDevice, format, pFormatProperties);
    }
    DispatchGetPhysicalDeviceFormatProperties2(physicalDevice, format, pFormatProperties);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceImageFormatProperties2(physicalDevice, pImageFormatInfo, pImageFormatProperties);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetPhysicalDeviceImageFormatProperties2(physicalDevice, pImageFormatInfo, pImageFormatProperties);
    }
    VkResult result = DispatchGetPhysicalDeviceImageFormatProperties2(physicalDevice, pImageFormatInfo, pImageFormatProperties);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
        if (skip) return;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
    }
    DispatchGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
    }
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceMemoryProperties2(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceMemoryProperties2*          pMemoryProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceMemoryProperties2(physicalDevice, pMemoryProperties);
        if (skip) return;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetPhysicalDeviceMemoryProperties2(physicalDevice, pMemoryProperties);
    }
    DispatchGetPhysicalDeviceMemoryProperties2(physicalDevice, pMemoryProperties);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetPhysicalDeviceMemoryProperties2(physicalDevice, pMemoryProperties);
    }
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceSparseImageFormatProperties2(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo,
    uint32_t*                                   pPropertyCount,
    VkSparseImageFormatProperties2*             pProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceSparseImageFormatProperties2(physicalDevice, pFormatInfo, pPropertyCount, pProperties);
        if (skip) return;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetPhysicalDeviceSparseImageFormatProperties2(physicalDevice, pFormatInfo, pPropertyCount, pProperties);
    }
    DispatchGetPhysicalDeviceSparseImageFormatProperties2(physicalDevice, pFormatInfo, pPropertyCount, pProperties);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetPhysicalDeviceSparseImageFormatProperties2(physicalDevice, pFormatInfo, pPropertyCount, pProperties);
    }
}

VKAPI_ATTR void VKAPI_CALL TrimCommandPool(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    VkCommandPoolTrimFlags                      flags) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateTrimCommandPool]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateTrimCommandPool(device, commandPool, flags);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordTrimCommandPool]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordTrimCommandPool(device, commandPool, flags);
    }
    DispatchTrimCommandPool(device, commandPool, flags);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordTrimCommandPool]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordTrimCommandPool(device, commandPool, flags);
    }
}

VKAPI_ATTR void VKAPI_CALL GetDeviceQueue2(
    VkDevice                                    device,
    const VkDeviceQueueInfo2*                   pQueueInfo,
    VkQueue*                                    pQueue) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetDeviceQueue2]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetDeviceQueue2(device, pQueueInfo, pQueue);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetDeviceQueue2]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetDeviceQueue2(device, pQueueInfo, pQueue);
    }
    DispatchGetDeviceQueue2(device, pQueueInfo, pQueue);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetDeviceQueue2]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateSamplerYcbcrConversion(device, pCreateInfo, pAllocator, pYcbcrConversion);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateSamplerYcbcrConversion]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCreateSamplerYcbcrConversion(device, pCreateInfo, pAllocator, pYcbcrConversion);
    }
    VkResult result = DispatchCreateSamplerYcbcrConversion(device, pCreateInfo, pAllocator, pYcbcrConversion);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateSamplerYcbcrConversion]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateDestroySamplerYcbcrConversion(device, ycbcrConversion, pAllocator);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroySamplerYcbcrConversion]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordDestroySamplerYcbcrConversion(device, ycbcrConversion, pAllocator);
    }
    DispatchDestroySamplerYcbcrConversion(device, ycbcrConversion, pAllocator);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroySamplerYcbcrConversion]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordDestroySamplerYcbcrConversion(device, ycbcrConversion, pAllocator);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL CreateDescriptorUpdateTemplate(
    VkDevice                                    device,
    const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorUpdateTemplate*                 pDescriptorUpdateTemplate) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCreateDescriptorUpdateTemplate]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateDescriptorUpdateTemplate(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateDescriptorUpdateTemplate]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCreateDescriptorUpdateTemplate(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);
    }
    VkResult result = DispatchCreateDescriptorUpdateTemplate(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateDescriptorUpdateTemplate]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCreateDescriptorUpdateTemplate(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyDescriptorUpdateTemplate(
    VkDevice                                    device,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    const VkAllocationCallbacks*                pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDestroyDescriptorUpdateTemplate]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateDestroyDescriptorUpdateTemplate(device, descriptorUpdateTemplate, pAllocator);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroyDescriptorUpdateTemplate]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordDestroyDescriptorUpdateTemplate(device, descriptorUpdateTemplate, pAllocator);
    }
    DispatchDestroyDescriptorUpdateTemplate(device, descriptorUpdateTemplate, pAllocator);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroyDescriptorUpdateTemplate]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordDestroyDescriptorUpdateTemplate(device, descriptorUpdateTemplate, pAllocator);
    }
}

VKAPI_ATTR void VKAPI_CALL UpdateDescriptorSetWithTemplate(
    VkDevice                                    device,
    VkDescriptorSet                             descriptorSet,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    const void*                                 pData) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateUpdateDescriptorSetWithTemplate]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateUpdateDescriptorSetWithTemplate(device, descriptorSet, descriptorUpdateTemplate, pData);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordUpdateDescriptorSetWithTemplate]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordUpdateDescriptorSetWithTemplate(device, descriptorSet, descriptorUpdateTemplate, pData);
    }
    DispatchUpdateDescriptorSetWithTemplate(device, descriptorSet, descriptorUpdateTemplate, pData);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordUpdateDescriptorSetWithTemplate]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordUpdateDescriptorSetWithTemplate(device, descriptorSet, descriptorUpdateTemplate, pData);
    }
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceExternalBufferProperties(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalBufferInfo*   pExternalBufferInfo,
    VkExternalBufferProperties*                 pExternalBufferProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceExternalBufferProperties(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
        if (skip) return;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetPhysicalDeviceExternalBufferProperties(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
    }
    DispatchGetPhysicalDeviceExternalBufferProperties(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceExternalFenceProperties(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
        if (skip) return;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetPhysicalDeviceExternalFenceProperties(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
    }
    DispatchGetPhysicalDeviceExternalFenceProperties(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceExternalSemaphoreProperties(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
        if (skip) return;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetPhysicalDeviceExternalSemaphoreProperties(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
    }
    DispatchGetPhysicalDeviceExternalSemaphoreProperties(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetDescriptorSetLayoutSupport(device, pCreateInfo, pSupport);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetDescriptorSetLayoutSupport]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetDescriptorSetLayoutSupport(device, pCreateInfo, pSupport);
    }
    DispatchGetDescriptorSetLayoutSupport(device, pCreateInfo, pSupport);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetDescriptorSetLayoutSupport]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdDrawIndirectCount]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    }
    DispatchCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdDrawIndirectCount]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdDrawIndexedIndirectCount]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    }
    DispatchCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdDrawIndexedIndirectCount]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateRenderPass2(device, pCreateInfo, pAllocator, pRenderPass);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateRenderPass2]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCreateRenderPass2(device, pCreateInfo, pAllocator, pRenderPass);
    }
    VkResult result = DispatchCreateRenderPass2(device, pCreateInfo, pAllocator, pRenderPass);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateRenderPass2]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdBeginRenderPass2]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
    }
    DispatchCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdBeginRenderPass2]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdNextSubpass2(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdNextSubpass2]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdNextSubpass2(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
    }
    DispatchCmdNextSubpass2(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdNextSubpass2]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdNextSubpass2(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdEndRenderPass2(
    VkCommandBuffer                             commandBuffer,
    const VkSubpassEndInfo*                     pSubpassEndInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdEndRenderPass2]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdEndRenderPass2(commandBuffer, pSubpassEndInfo);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdEndRenderPass2]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdEndRenderPass2(commandBuffer, pSubpassEndInfo);
    }
    DispatchCmdEndRenderPass2(commandBuffer, pSubpassEndInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdEndRenderPass2]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateResetQueryPool(device, queryPool, firstQuery, queryCount);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordResetQueryPool]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordResetQueryPool(device, queryPool, firstQuery, queryCount);
    }
    DispatchResetQueryPool(device, queryPool, firstQuery, queryCount);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordResetQueryPool]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetSemaphoreCounterValue(device, semaphore, pValue);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetSemaphoreCounterValue]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetSemaphoreCounterValue(device, semaphore, pValue);
    }
    VkResult result = DispatchGetSemaphoreCounterValue(device, semaphore, pValue);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetSemaphoreCounterValue]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateWaitSemaphores(device, pWaitInfo, timeout);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordWaitSemaphores]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordWaitSemaphores(device, pWaitInfo, timeout);
    }
    VkResult result = DispatchWaitSemaphores(device, pWaitInfo, timeout);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordWaitSemaphores]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateSignalSemaphore(device, pSignalInfo);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordSignalSemaphore]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordSignalSemaphore(device, pSignalInfo);
    }
    VkResult result = DispatchSignalSemaphore(device, pSignalInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordSignalSemaphore]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetBufferDeviceAddress(device, pInfo);
        if (skip) return 0;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetBufferDeviceAddress]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetBufferDeviceAddress(device, pInfo);
    }
    VkDeviceAddress result = DispatchGetBufferDeviceAddress(device, pInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetBufferDeviceAddress]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetBufferOpaqueCaptureAddress(device, pInfo);
        if (skip) return 0;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetBufferOpaqueCaptureAddress]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetBufferOpaqueCaptureAddress(device, pInfo);
    }
    uint64_t result = DispatchGetBufferOpaqueCaptureAddress(device, pInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetBufferOpaqueCaptureAddress]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetDeviceMemoryOpaqueCaptureAddress(device, pInfo);
        if (skip) return 0;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetDeviceMemoryOpaqueCaptureAddress]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetDeviceMemoryOpaqueCaptureAddress(device, pInfo);
    }
    uint64_t result = DispatchGetDeviceMemoryOpaqueCaptureAddress(device, pInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetDeviceMemoryOpaqueCaptureAddress]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetDeviceMemoryOpaqueCaptureAddress(device, pInfo);
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateDestroySurfaceKHR(instance, surface, pAllocator);
        if (skip) return;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordDestroySurfaceKHR(instance, surface, pAllocator);
    }
    DispatchDestroySurfaceKHR(instance, surface, pAllocator);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, pSupported);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, pSupported);
    }
    VkResult result = DispatchGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, pSupported);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, pSurfaceCapabilities);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, pSurfaceCapabilities);
    }
    VkResult result = DispatchGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, pSurfaceCapabilities);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, pSurfaceFormatCount, pSurfaceFormats);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, pSurfaceFormatCount, pSurfaceFormats);
    }
    VkResult result = DispatchGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, pSurfaceFormatCount, pSurfaceFormats);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, pPresentModeCount, pPresentModes);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, pPresentModeCount, pPresentModes);
    }
    VkResult result = DispatchGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, pPresentModeCount, pPresentModes);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateSwapchainKHR(device, pCreateInfo, pAllocator, pSwapchain);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateSwapchainKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCreateSwapchainKHR(device, pCreateInfo, pAllocator, pSwapchain);
    }
    VkResult result = DispatchCreateSwapchainKHR(device, pCreateInfo, pAllocator, pSwapchain);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateSwapchainKHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCreateSwapchainKHR(device, pCreateInfo, pAllocator, pSwapchain, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroySwapchainKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    const VkAllocationCallbacks*                pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDestroySwapchainKHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateDestroySwapchainKHR(device, swapchain, pAllocator);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroySwapchainKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordDestroySwapchainKHR(device, swapchain, pAllocator);
    }
    DispatchDestroySwapchainKHR(device, swapchain, pAllocator);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroySwapchainKHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordDestroySwapchainKHR(device, swapchain, pAllocator);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL GetSwapchainImagesKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    uint32_t*                                   pSwapchainImageCount,
    VkImage*                                    pSwapchainImages) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetSwapchainImagesKHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetSwapchainImagesKHR(device, swapchain, pSwapchainImageCount, pSwapchainImages);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetSwapchainImagesKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetSwapchainImagesKHR(device, swapchain, pSwapchainImageCount, pSwapchainImages);
    }
    VkResult result = DispatchGetSwapchainImagesKHR(device, swapchain, pSwapchainImageCount, pSwapchainImages);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetSwapchainImagesKHR]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateAcquireNextImageKHR(device, swapchain, timeout, semaphore, fence, pImageIndex);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordAcquireNextImageKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordAcquireNextImageKHR(device, swapchain, timeout, semaphore, fence, pImageIndex);
    }
    VkResult result = DispatchAcquireNextImageKHR(device, swapchain, timeout, semaphore, fence, pImageIndex);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordAcquireNextImageKHR]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateQueuePresentKHR(queue, pPresentInfo);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordQueuePresentKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordQueuePresentKHR(queue, pPresentInfo);
    }
    VkResult result = DispatchQueuePresentKHR(queue, pPresentInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordQueuePresentKHR]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetDeviceGroupPresentCapabilitiesKHR(device, pDeviceGroupPresentCapabilities);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetDeviceGroupPresentCapabilitiesKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetDeviceGroupPresentCapabilitiesKHR(device, pDeviceGroupPresentCapabilities);
    }
    VkResult result = DispatchGetDeviceGroupPresentCapabilitiesKHR(device, pDeviceGroupPresentCapabilities);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetDeviceGroupPresentCapabilitiesKHR]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetDeviceGroupSurfacePresentModesKHR(device, surface, pModes);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetDeviceGroupSurfacePresentModesKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetDeviceGroupSurfacePresentModesKHR(device, surface, pModes);
    }
    VkResult result = DispatchGetDeviceGroupSurfacePresentModesKHR(device, surface, pModes);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetDeviceGroupSurfacePresentModesKHR]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDevicePresentRectanglesKHR(physicalDevice, surface, pRectCount, pRects);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetPhysicalDevicePresentRectanglesKHR(physicalDevice, surface, pRectCount, pRects);
    }
    VkResult result = DispatchGetPhysicalDevicePresentRectanglesKHR(physicalDevice, surface, pRectCount, pRects);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateAcquireNextImage2KHR(device, pAcquireInfo, pImageIndex);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordAcquireNextImage2KHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordAcquireNextImage2KHR(device, pAcquireInfo, pImageIndex);
    }
    VkResult result = DispatchAcquireNextImage2KHR(device, pAcquireInfo, pImageIndex);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordAcquireNextImage2KHR]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceDisplayPropertiesKHR(physicalDevice, pPropertyCount, pProperties);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetPhysicalDeviceDisplayPropertiesKHR(physicalDevice, pPropertyCount, pProperties);
    }
    VkResult result = DispatchGetPhysicalDeviceDisplayPropertiesKHR(physicalDevice, pPropertyCount, pProperties);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceDisplayPlanePropertiesKHR(physicalDevice, pPropertyCount, pProperties);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetPhysicalDeviceDisplayPlanePropertiesKHR(physicalDevice, pPropertyCount, pProperties);
    }
    VkResult result = DispatchGetPhysicalDeviceDisplayPlanePropertiesKHR(physicalDevice, pPropertyCount, pProperties);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetDisplayPlaneSupportedDisplaysKHR(physicalDevice, planeIndex, pDisplayCount, pDisplays);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetDisplayPlaneSupportedDisplaysKHR(physicalDevice, planeIndex, pDisplayCount, pDisplays);
    }
    VkResult result = DispatchGetDisplayPlaneSupportedDisplaysKHR(physicalDevice, planeIndex, pDisplayCount, pDisplays);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetDisplayModePropertiesKHR(physicalDevice, display, pPropertyCount, pProperties);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetDisplayModePropertiesKHR(physicalDevice, display, pPropertyCount, pProperties);
    }
    VkResult result = DispatchGetDisplayModePropertiesKHR(physicalDevice, display, pPropertyCount, pProperties);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateDisplayModeKHR(physicalDevice, display, pCreateInfo, pAllocator, pMode);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCreateDisplayModeKHR(physicalDevice, display, pCreateInfo, pAllocator, pMode);
    }
    VkResult result = DispatchCreateDisplayModeKHR(physicalDevice, display, pCreateInfo, pAllocator, pMode);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetDisplayPlaneCapabilitiesKHR(physicalDevice, mode, planeIndex, pCapabilities);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetDisplayPlaneCapabilitiesKHR(physicalDevice, mode, planeIndex, pCapabilities);
    }
    VkResult result = DispatchGetDisplayPlaneCapabilitiesKHR(physicalDevice, mode, planeIndex, pCapabilities);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateDisplayPlaneSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCreateDisplayPlaneSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    }
    VkResult result = DispatchCreateDisplayPlaneSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateSharedSwapchainsKHR(device, swapchainCount, pCreateInfos, pAllocator, pSwapchains);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateSharedSwapchainsKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCreateSharedSwapchainsKHR(device, swapchainCount, pCreateInfos, pAllocator, pSwapchains);
    }
    VkResult result = DispatchCreateSharedSwapchainsKHR(device, swapchainCount, pCreateInfos, pAllocator, pSwapchains);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateSharedSwapchainsKHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCreateSharedSwapchainsKHR(device, swapchainCount, pCreateInfos, pAllocator, pSwapchains, result);
    }
    return result;
}

#ifdef VK_USE_PLATFORM_XLIB_KHR

VKAPI_ATTR VkResult VKAPI_CALL CreateXlibSurfaceKHR(
    VkInstance                                  instance,
    const VkXlibSurfaceCreateInfoKHR*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateXlibSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCreateXlibSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    }
    VkResult result = DispatchCreateXlibSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCreateXlibSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface, result);
    }
    return result;
}

VKAPI_ATTR VkBool32 VKAPI_CALL GetPhysicalDeviceXlibPresentationSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    Display*                                    dpy,
    VisualID                                    visualID) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceXlibPresentationSupportKHR(physicalDevice, queueFamilyIndex, dpy, visualID);
        if (skip) return VK_FALSE;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetPhysicalDeviceXlibPresentationSupportKHR(physicalDevice, queueFamilyIndex, dpy, visualID);
    }
    VkBool32 result = DispatchGetPhysicalDeviceXlibPresentationSupportKHR(physicalDevice, queueFamilyIndex, dpy, visualID);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetPhysicalDeviceXlibPresentationSupportKHR(physicalDevice, queueFamilyIndex, dpy, visualID);
    }
    return result;
}
#endif // VK_USE_PLATFORM_XLIB_KHR

#ifdef VK_USE_PLATFORM_XCB_KHR

VKAPI_ATTR VkResult VKAPI_CALL CreateXcbSurfaceKHR(
    VkInstance                                  instance,
    const VkXcbSurfaceCreateInfoKHR*            pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateXcbSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCreateXcbSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    }
    VkResult result = DispatchCreateXcbSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCreateXcbSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface, result);
    }
    return result;
}

VKAPI_ATTR VkBool32 VKAPI_CALL GetPhysicalDeviceXcbPresentationSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    xcb_connection_t*                           connection,
    xcb_visualid_t                              visual_id) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceXcbPresentationSupportKHR(physicalDevice, queueFamilyIndex, connection, visual_id);
        if (skip) return VK_FALSE;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetPhysicalDeviceXcbPresentationSupportKHR(physicalDevice, queueFamilyIndex, connection, visual_id);
    }
    VkBool32 result = DispatchGetPhysicalDeviceXcbPresentationSupportKHR(physicalDevice, queueFamilyIndex, connection, visual_id);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetPhysicalDeviceXcbPresentationSupportKHR(physicalDevice, queueFamilyIndex, connection, visual_id);
    }
    return result;
}
#endif // VK_USE_PLATFORM_XCB_KHR

#ifdef VK_USE_PLATFORM_WAYLAND_KHR

VKAPI_ATTR VkResult VKAPI_CALL CreateWaylandSurfaceKHR(
    VkInstance                                  instance,
    const VkWaylandSurfaceCreateInfoKHR*        pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateWaylandSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCreateWaylandSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    }
    VkResult result = DispatchCreateWaylandSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCreateWaylandSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface, result);
    }
    return result;
}

VKAPI_ATTR VkBool32 VKAPI_CALL GetPhysicalDeviceWaylandPresentationSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    struct wl_display*                          display) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceWaylandPresentationSupportKHR(physicalDevice, queueFamilyIndex, display);
        if (skip) return VK_FALSE;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetPhysicalDeviceWaylandPresentationSupportKHR(physicalDevice, queueFamilyIndex, display);
    }
    VkBool32 result = DispatchGetPhysicalDeviceWaylandPresentationSupportKHR(physicalDevice, queueFamilyIndex, display);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetPhysicalDeviceWaylandPresentationSupportKHR(physicalDevice, queueFamilyIndex, display);
    }
    return result;
}
#endif // VK_USE_PLATFORM_WAYLAND_KHR

#ifdef VK_USE_PLATFORM_ANDROID_KHR

VKAPI_ATTR VkResult VKAPI_CALL CreateAndroidSurfaceKHR(
    VkInstance                                  instance,
    const VkAndroidSurfaceCreateInfoKHR*        pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateAndroidSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCreateAndroidSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    }
    VkResult result = DispatchCreateAndroidSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCreateAndroidSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface, result);
    }
    return result;
}
#endif // VK_USE_PLATFORM_ANDROID_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR

VKAPI_ATTR VkResult VKAPI_CALL CreateWin32SurfaceKHR(
    VkInstance                                  instance,
    const VkWin32SurfaceCreateInfoKHR*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateWin32SurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCreateWin32SurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    }
    VkResult result = DispatchCreateWin32SurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCreateWin32SurfaceKHR(instance, pCreateInfo, pAllocator, pSurface, result);
    }
    return result;
}

VKAPI_ATTR VkBool32 VKAPI_CALL GetPhysicalDeviceWin32PresentationSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceWin32PresentationSupportKHR(physicalDevice, queueFamilyIndex);
        if (skip) return VK_FALSE;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetPhysicalDeviceWin32PresentationSupportKHR(physicalDevice, queueFamilyIndex);
    }
    VkBool32 result = DispatchGetPhysicalDeviceWin32PresentationSupportKHR(physicalDevice, queueFamilyIndex);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetPhysicalDeviceWin32PresentationSupportKHR(physicalDevice, queueFamilyIndex);
    }
    return result;
}
#endif // VK_USE_PLATFORM_WIN32_KHR




VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceFeatures2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceFeatures2*                  pFeatures) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceFeatures2KHR(physicalDevice, pFeatures);
        if (skip) return;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetPhysicalDeviceFeatures2KHR(physicalDevice, pFeatures);
    }
    DispatchGetPhysicalDeviceFeatures2KHR(physicalDevice, pFeatures);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetPhysicalDeviceFeatures2KHR(physicalDevice, pFeatures);
    }
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceProperties2*                pProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceProperties2KHR(physicalDevice, pProperties);
        if (skip) return;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetPhysicalDeviceProperties2KHR(physicalDevice, pProperties);
    }
    DispatchGetPhysicalDeviceProperties2KHR(physicalDevice, pProperties);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetPhysicalDeviceProperties2KHR(physicalDevice, pProperties);
    }
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceFormatProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkFormatProperties2*                        pFormatProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceFormatProperties2KHR(physicalDevice, format, pFormatProperties);
        if (skip) return;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetPhysicalDeviceFormatProperties2KHR(physicalDevice, format, pFormatProperties);
    }
    DispatchGetPhysicalDeviceFormatProperties2KHR(physicalDevice, format, pFormatProperties);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetPhysicalDeviceFormatProperties2KHR(physicalDevice, format, pFormatProperties);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceImageFormatProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceImageFormatInfo2*     pImageFormatInfo,
    VkImageFormatProperties2*                   pImageFormatProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceImageFormatProperties2KHR(physicalDevice, pImageFormatInfo, pImageFormatProperties);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetPhysicalDeviceImageFormatProperties2KHR(physicalDevice, pImageFormatInfo, pImageFormatProperties);
    }
    VkResult result = DispatchGetPhysicalDeviceImageFormatProperties2KHR(physicalDevice, pImageFormatInfo, pImageFormatProperties);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetPhysicalDeviceImageFormatProperties2KHR(physicalDevice, pImageFormatInfo, pImageFormatProperties, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceQueueFamilyProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pQueueFamilyPropertyCount,
    VkQueueFamilyProperties2*                   pQueueFamilyProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceQueueFamilyProperties2KHR(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
        if (skip) return;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetPhysicalDeviceQueueFamilyProperties2KHR(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
    }
    DispatchGetPhysicalDeviceQueueFamilyProperties2KHR(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetPhysicalDeviceQueueFamilyProperties2KHR(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
    }
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceMemoryProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceMemoryProperties2*          pMemoryProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceMemoryProperties2KHR(physicalDevice, pMemoryProperties);
        if (skip) return;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetPhysicalDeviceMemoryProperties2KHR(physicalDevice, pMemoryProperties);
    }
    DispatchGetPhysicalDeviceMemoryProperties2KHR(physicalDevice, pMemoryProperties);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetPhysicalDeviceMemoryProperties2KHR(physicalDevice, pMemoryProperties);
    }
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceSparseImageFormatProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo,
    uint32_t*                                   pPropertyCount,
    VkSparseImageFormatProperties2*             pProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceSparseImageFormatProperties2KHR(physicalDevice, pFormatInfo, pPropertyCount, pProperties);
        if (skip) return;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetPhysicalDeviceSparseImageFormatProperties2KHR(physicalDevice, pFormatInfo, pPropertyCount, pProperties);
    }
    DispatchGetPhysicalDeviceSparseImageFormatProperties2KHR(physicalDevice, pFormatInfo, pPropertyCount, pProperties);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetPhysicalDeviceSparseImageFormatProperties2KHR(physicalDevice, pFormatInfo, pPropertyCount, pProperties);
    }
}


VKAPI_ATTR void VKAPI_CALL GetDeviceGroupPeerMemoryFeaturesKHR(
    VkDevice                                    device,
    uint32_t                                    heapIndex,
    uint32_t                                    localDeviceIndex,
    uint32_t                                    remoteDeviceIndex,
    VkPeerMemoryFeatureFlags*                   pPeerMemoryFeatures) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetDeviceGroupPeerMemoryFeaturesKHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetDeviceGroupPeerMemoryFeaturesKHR(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetDeviceGroupPeerMemoryFeaturesKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetDeviceGroupPeerMemoryFeaturesKHR(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
    }
    DispatchGetDeviceGroupPeerMemoryFeaturesKHR(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetDeviceGroupPeerMemoryFeaturesKHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetDeviceGroupPeerMemoryFeaturesKHR(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetDeviceMaskKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    deviceMask) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetDeviceMaskKHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetDeviceMaskKHR(commandBuffer, deviceMask);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetDeviceMaskKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdSetDeviceMaskKHR(commandBuffer, deviceMask);
    }
    DispatchCmdSetDeviceMaskKHR(commandBuffer, deviceMask);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetDeviceMaskKHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdSetDeviceMaskKHR(commandBuffer, deviceMask);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdDispatchBaseKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    baseGroupX,
    uint32_t                                    baseGroupY,
    uint32_t                                    baseGroupZ,
    uint32_t                                    groupCountX,
    uint32_t                                    groupCountY,
    uint32_t                                    groupCountZ) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdDispatchBaseKHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdDispatchBaseKHR(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdDispatchBaseKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdDispatchBaseKHR(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
    }
    DispatchCmdDispatchBaseKHR(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdDispatchBaseKHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdDispatchBaseKHR(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
    }
}



VKAPI_ATTR void VKAPI_CALL TrimCommandPoolKHR(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    VkCommandPoolTrimFlags                      flags) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateTrimCommandPoolKHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateTrimCommandPoolKHR(device, commandPool, flags);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordTrimCommandPoolKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordTrimCommandPoolKHR(device, commandPool, flags);
    }
    DispatchTrimCommandPoolKHR(device, commandPool, flags);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordTrimCommandPoolKHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordTrimCommandPoolKHR(device, commandPool, flags);
    }
}


VKAPI_ATTR VkResult VKAPI_CALL EnumeratePhysicalDeviceGroupsKHR(
    VkInstance                                  instance,
    uint32_t*                                   pPhysicalDeviceGroupCount,
    VkPhysicalDeviceGroupProperties*            pPhysicalDeviceGroupProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateEnumeratePhysicalDeviceGroupsKHR(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordEnumeratePhysicalDeviceGroupsKHR(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
    }
    VkResult result = DispatchEnumeratePhysicalDeviceGroupsKHR(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordEnumeratePhysicalDeviceGroupsKHR(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties, result);
    }
    return result;
}


VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceExternalBufferPropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalBufferInfo*   pExternalBufferInfo,
    VkExternalBufferProperties*                 pExternalBufferProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceExternalBufferPropertiesKHR(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
        if (skip) return;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetPhysicalDeviceExternalBufferPropertiesKHR(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
    }
    DispatchGetPhysicalDeviceExternalBufferPropertiesKHR(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetPhysicalDeviceExternalBufferPropertiesKHR(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
    }
}


#ifdef VK_USE_PLATFORM_WIN32_KHR

VKAPI_ATTR VkResult VKAPI_CALL GetMemoryWin32HandleKHR(
    VkDevice                                    device,
    const VkMemoryGetWin32HandleInfoKHR*        pGetWin32HandleInfo,
    HANDLE*                                     pHandle) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetMemoryWin32HandleKHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetMemoryWin32HandleKHR(device, pGetWin32HandleInfo, pHandle);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetMemoryWin32HandleKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetMemoryWin32HandleKHR(device, pGetWin32HandleInfo, pHandle);
    }
    VkResult result = DispatchGetMemoryWin32HandleKHR(device, pGetWin32HandleInfo, pHandle);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetMemoryWin32HandleKHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetMemoryWin32HandleKHR(device, pGetWin32HandleInfo, pHandle, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetMemoryWin32HandlePropertiesKHR(
    VkDevice                                    device,
    VkExternalMemoryHandleTypeFlagBits          handleType,
    HANDLE                                      handle,
    VkMemoryWin32HandlePropertiesKHR*           pMemoryWin32HandleProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetMemoryWin32HandlePropertiesKHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetMemoryWin32HandlePropertiesKHR(device, handleType, handle, pMemoryWin32HandleProperties);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetMemoryWin32HandlePropertiesKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetMemoryWin32HandlePropertiesKHR(device, handleType, handle, pMemoryWin32HandleProperties);
    }
    VkResult result = DispatchGetMemoryWin32HandlePropertiesKHR(device, handleType, handle, pMemoryWin32HandleProperties);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetMemoryWin32HandlePropertiesKHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetMemoryWin32HandlePropertiesKHR(device, handleType, handle, pMemoryWin32HandleProperties, result);
    }
    return result;
}
#endif // VK_USE_PLATFORM_WIN32_KHR


VKAPI_ATTR VkResult VKAPI_CALL GetMemoryFdKHR(
    VkDevice                                    device,
    const VkMemoryGetFdInfoKHR*                 pGetFdInfo,
    int*                                        pFd) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetMemoryFdKHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetMemoryFdKHR(device, pGetFdInfo, pFd);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetMemoryFdKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetMemoryFdKHR(device, pGetFdInfo, pFd);
    }
    VkResult result = DispatchGetMemoryFdKHR(device, pGetFdInfo, pFd);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetMemoryFdKHR]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetMemoryFdPropertiesKHR(device, handleType, fd, pMemoryFdProperties);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetMemoryFdPropertiesKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetMemoryFdPropertiesKHR(device, handleType, fd, pMemoryFdProperties);
    }
    VkResult result = DispatchGetMemoryFdPropertiesKHR(device, handleType, fd, pMemoryFdProperties);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetMemoryFdPropertiesKHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetMemoryFdPropertiesKHR(device, handleType, fd, pMemoryFdProperties, result);
    }
    return result;
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
#endif // VK_USE_PLATFORM_WIN32_KHR


VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceExternalSemaphorePropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo,
    VkExternalSemaphoreProperties*              pExternalSemaphoreProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceExternalSemaphorePropertiesKHR(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
        if (skip) return;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetPhysicalDeviceExternalSemaphorePropertiesKHR(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
    }
    DispatchGetPhysicalDeviceExternalSemaphorePropertiesKHR(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetPhysicalDeviceExternalSemaphorePropertiesKHR(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
    }
}


#ifdef VK_USE_PLATFORM_WIN32_KHR

VKAPI_ATTR VkResult VKAPI_CALL ImportSemaphoreWin32HandleKHR(
    VkDevice                                    device,
    const VkImportSemaphoreWin32HandleInfoKHR*  pImportSemaphoreWin32HandleInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateImportSemaphoreWin32HandleKHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateImportSemaphoreWin32HandleKHR(device, pImportSemaphoreWin32HandleInfo);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordImportSemaphoreWin32HandleKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordImportSemaphoreWin32HandleKHR(device, pImportSemaphoreWin32HandleInfo);
    }
    VkResult result = DispatchImportSemaphoreWin32HandleKHR(device, pImportSemaphoreWin32HandleInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordImportSemaphoreWin32HandleKHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordImportSemaphoreWin32HandleKHR(device, pImportSemaphoreWin32HandleInfo, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetSemaphoreWin32HandleKHR(
    VkDevice                                    device,
    const VkSemaphoreGetWin32HandleInfoKHR*     pGetWin32HandleInfo,
    HANDLE*                                     pHandle) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetSemaphoreWin32HandleKHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetSemaphoreWin32HandleKHR(device, pGetWin32HandleInfo, pHandle);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetSemaphoreWin32HandleKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetSemaphoreWin32HandleKHR(device, pGetWin32HandleInfo, pHandle);
    }
    VkResult result = DispatchGetSemaphoreWin32HandleKHR(device, pGetWin32HandleInfo, pHandle);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetSemaphoreWin32HandleKHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetSemaphoreWin32HandleKHR(device, pGetWin32HandleInfo, pHandle, result);
    }
    return result;
}
#endif // VK_USE_PLATFORM_WIN32_KHR


VKAPI_ATTR VkResult VKAPI_CALL ImportSemaphoreFdKHR(
    VkDevice                                    device,
    const VkImportSemaphoreFdInfoKHR*           pImportSemaphoreFdInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateImportSemaphoreFdKHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateImportSemaphoreFdKHR(device, pImportSemaphoreFdInfo);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordImportSemaphoreFdKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordImportSemaphoreFdKHR(device, pImportSemaphoreFdInfo);
    }
    VkResult result = DispatchImportSemaphoreFdKHR(device, pImportSemaphoreFdInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordImportSemaphoreFdKHR]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetSemaphoreFdKHR(device, pGetFdInfo, pFd);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetSemaphoreFdKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetSemaphoreFdKHR(device, pGetFdInfo, pFd);
    }
    VkResult result = DispatchGetSemaphoreFdKHR(device, pGetFdInfo, pFd);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetSemaphoreFdKHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetSemaphoreFdKHR(device, pGetFdInfo, pFd, result);
    }
    return result;
}


VKAPI_ATTR void VKAPI_CALL CmdPushDescriptorSetKHR(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipelineLayout                            layout,
    uint32_t                                    set,
    uint32_t                                    descriptorWriteCount,
    const VkWriteDescriptorSet*                 pDescriptorWrites) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdPushDescriptorSetKHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdPushDescriptorSetKHR(commandBuffer, pipelineBindPoint, layout, set, descriptorWriteCount, pDescriptorWrites);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdPushDescriptorSetKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdPushDescriptorSetKHR(commandBuffer, pipelineBindPoint, layout, set, descriptorWriteCount, pDescriptorWrites);
    }
    DispatchCmdPushDescriptorSetKHR(commandBuffer, pipelineBindPoint, layout, set, descriptorWriteCount, pDescriptorWrites);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdPushDescriptorSetKHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdPushDescriptorSetKHR(commandBuffer, pipelineBindPoint, layout, set, descriptorWriteCount, pDescriptorWrites);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdPushDescriptorSetWithTemplateKHR(
    VkCommandBuffer                             commandBuffer,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    VkPipelineLayout                            layout,
    uint32_t                                    set,
    const void*                                 pData) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdPushDescriptorSetWithTemplateKHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdPushDescriptorSetWithTemplateKHR(commandBuffer, descriptorUpdateTemplate, layout, set, pData);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdPushDescriptorSetWithTemplateKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdPushDescriptorSetWithTemplateKHR(commandBuffer, descriptorUpdateTemplate, layout, set, pData);
    }
    DispatchCmdPushDescriptorSetWithTemplateKHR(commandBuffer, descriptorUpdateTemplate, layout, set, pData);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdPushDescriptorSetWithTemplateKHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdPushDescriptorSetWithTemplateKHR(commandBuffer, descriptorUpdateTemplate, layout, set, pData);
    }
}





VKAPI_ATTR VkResult VKAPI_CALL CreateDescriptorUpdateTemplateKHR(
    VkDevice                                    device,
    const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorUpdateTemplate*                 pDescriptorUpdateTemplate) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCreateDescriptorUpdateTemplateKHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateDescriptorUpdateTemplateKHR(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateDescriptorUpdateTemplateKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCreateDescriptorUpdateTemplateKHR(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);
    }
    VkResult result = DispatchCreateDescriptorUpdateTemplateKHR(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateDescriptorUpdateTemplateKHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCreateDescriptorUpdateTemplateKHR(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyDescriptorUpdateTemplateKHR(
    VkDevice                                    device,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    const VkAllocationCallbacks*                pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDestroyDescriptorUpdateTemplateKHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateDestroyDescriptorUpdateTemplateKHR(device, descriptorUpdateTemplate, pAllocator);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroyDescriptorUpdateTemplateKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordDestroyDescriptorUpdateTemplateKHR(device, descriptorUpdateTemplate, pAllocator);
    }
    DispatchDestroyDescriptorUpdateTemplateKHR(device, descriptorUpdateTemplate, pAllocator);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroyDescriptorUpdateTemplateKHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordDestroyDescriptorUpdateTemplateKHR(device, descriptorUpdateTemplate, pAllocator);
    }
}

VKAPI_ATTR void VKAPI_CALL UpdateDescriptorSetWithTemplateKHR(
    VkDevice                                    device,
    VkDescriptorSet                             descriptorSet,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    const void*                                 pData) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateUpdateDescriptorSetWithTemplateKHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateUpdateDescriptorSetWithTemplateKHR(device, descriptorSet, descriptorUpdateTemplate, pData);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordUpdateDescriptorSetWithTemplateKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordUpdateDescriptorSetWithTemplateKHR(device, descriptorSet, descriptorUpdateTemplate, pData);
    }
    DispatchUpdateDescriptorSetWithTemplateKHR(device, descriptorSet, descriptorUpdateTemplate, pData);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordUpdateDescriptorSetWithTemplateKHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordUpdateDescriptorSetWithTemplateKHR(device, descriptorSet, descriptorUpdateTemplate, pData);
    }
}



VKAPI_ATTR VkResult VKAPI_CALL CreateRenderPass2KHR(
    VkDevice                                    device,
    const VkRenderPassCreateInfo2*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkRenderPass*                               pRenderPass) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCreateRenderPass2KHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateRenderPass2KHR(device, pCreateInfo, pAllocator, pRenderPass);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateRenderPass2KHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCreateRenderPass2KHR(device, pCreateInfo, pAllocator, pRenderPass);
    }
    VkResult result = DispatchCreateRenderPass2KHR(device, pCreateInfo, pAllocator, pRenderPass);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateRenderPass2KHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCreateRenderPass2KHR(device, pCreateInfo, pAllocator, pRenderPass, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL CmdBeginRenderPass2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkRenderPassBeginInfo*                pRenderPassBegin,
    const VkSubpassBeginInfo*                   pSubpassBeginInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdBeginRenderPass2KHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdBeginRenderPass2KHR(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdBeginRenderPass2KHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdBeginRenderPass2KHR(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
    }
    DispatchCmdBeginRenderPass2KHR(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdBeginRenderPass2KHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdBeginRenderPass2KHR(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdNextSubpass2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkSubpassBeginInfo*                   pSubpassBeginInfo,
    const VkSubpassEndInfo*                     pSubpassEndInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdNextSubpass2KHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdNextSubpass2KHR(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdNextSubpass2KHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdNextSubpass2KHR(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
    }
    DispatchCmdNextSubpass2KHR(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdNextSubpass2KHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdNextSubpass2KHR(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdEndRenderPass2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkSubpassEndInfo*                     pSubpassEndInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdEndRenderPass2KHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdEndRenderPass2KHR(commandBuffer, pSubpassEndInfo);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdEndRenderPass2KHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdEndRenderPass2KHR(commandBuffer, pSubpassEndInfo);
    }
    DispatchCmdEndRenderPass2KHR(commandBuffer, pSubpassEndInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdEndRenderPass2KHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdEndRenderPass2KHR(commandBuffer, pSubpassEndInfo);
    }
}


VKAPI_ATTR VkResult VKAPI_CALL GetSwapchainStatusKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetSwapchainStatusKHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetSwapchainStatusKHR(device, swapchain);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetSwapchainStatusKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetSwapchainStatusKHR(device, swapchain);
    }
    VkResult result = DispatchGetSwapchainStatusKHR(device, swapchain);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetSwapchainStatusKHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetSwapchainStatusKHR(device, swapchain, result);
    }
    return result;
}


VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceExternalFencePropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalFenceInfo*    pExternalFenceInfo,
    VkExternalFenceProperties*                  pExternalFenceProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceExternalFencePropertiesKHR(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
        if (skip) return;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetPhysicalDeviceExternalFencePropertiesKHR(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
    }
    DispatchGetPhysicalDeviceExternalFencePropertiesKHR(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetPhysicalDeviceExternalFencePropertiesKHR(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
    }
}


#ifdef VK_USE_PLATFORM_WIN32_KHR

VKAPI_ATTR VkResult VKAPI_CALL ImportFenceWin32HandleKHR(
    VkDevice                                    device,
    const VkImportFenceWin32HandleInfoKHR*      pImportFenceWin32HandleInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateImportFenceWin32HandleKHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateImportFenceWin32HandleKHR(device, pImportFenceWin32HandleInfo);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordImportFenceWin32HandleKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordImportFenceWin32HandleKHR(device, pImportFenceWin32HandleInfo);
    }
    VkResult result = DispatchImportFenceWin32HandleKHR(device, pImportFenceWin32HandleInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordImportFenceWin32HandleKHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordImportFenceWin32HandleKHR(device, pImportFenceWin32HandleInfo, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetFenceWin32HandleKHR(
    VkDevice                                    device,
    const VkFenceGetWin32HandleInfoKHR*         pGetWin32HandleInfo,
    HANDLE*                                     pHandle) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetFenceWin32HandleKHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetFenceWin32HandleKHR(device, pGetWin32HandleInfo, pHandle);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetFenceWin32HandleKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetFenceWin32HandleKHR(device, pGetWin32HandleInfo, pHandle);
    }
    VkResult result = DispatchGetFenceWin32HandleKHR(device, pGetWin32HandleInfo, pHandle);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetFenceWin32HandleKHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetFenceWin32HandleKHR(device, pGetWin32HandleInfo, pHandle, result);
    }
    return result;
}
#endif // VK_USE_PLATFORM_WIN32_KHR


VKAPI_ATTR VkResult VKAPI_CALL ImportFenceFdKHR(
    VkDevice                                    device,
    const VkImportFenceFdInfoKHR*               pImportFenceFdInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateImportFenceFdKHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateImportFenceFdKHR(device, pImportFenceFdInfo);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordImportFenceFdKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordImportFenceFdKHR(device, pImportFenceFdInfo);
    }
    VkResult result = DispatchImportFenceFdKHR(device, pImportFenceFdInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordImportFenceFdKHR]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetFenceFdKHR(device, pGetFdInfo, pFd);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetFenceFdKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetFenceFdKHR(device, pGetFdInfo, pFd);
    }
    VkResult result = DispatchGetFenceFdKHR(device, pGetFdInfo, pFd);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetFenceFdKHR]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(physicalDevice, queueFamilyIndex, pCounterCount, pCounters, pCounterDescriptions);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(physicalDevice, queueFamilyIndex, pCounterCount, pCounters, pCounterDescriptions);
    }
    VkResult result = DispatchEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(physicalDevice, queueFamilyIndex, pCounterCount, pCounters, pCounterDescriptions);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(physicalDevice, pPerformanceQueryCreateInfo, pNumPasses);
        if (skip) return;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(physicalDevice, pPerformanceQueryCreateInfo, pNumPasses);
    }
    DispatchGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(physicalDevice, pPerformanceQueryCreateInfo, pNumPasses);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(physicalDevice, pPerformanceQueryCreateInfo, pNumPasses);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL AcquireProfilingLockKHR(
    VkDevice                                    device,
    const VkAcquireProfilingLockInfoKHR*        pInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateAcquireProfilingLockKHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateAcquireProfilingLockKHR(device, pInfo);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordAcquireProfilingLockKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordAcquireProfilingLockKHR(device, pInfo);
    }
    VkResult result = DispatchAcquireProfilingLockKHR(device, pInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordAcquireProfilingLockKHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordAcquireProfilingLockKHR(device, pInfo, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL ReleaseProfilingLockKHR(
    VkDevice                                    device) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateReleaseProfilingLockKHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateReleaseProfilingLockKHR(device);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordReleaseProfilingLockKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordReleaseProfilingLockKHR(device);
    }
    DispatchReleaseProfilingLockKHR(device);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordReleaseProfilingLockKHR]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceSurfaceCapabilities2KHR(physicalDevice, pSurfaceInfo, pSurfaceCapabilities);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetPhysicalDeviceSurfaceCapabilities2KHR(physicalDevice, pSurfaceInfo, pSurfaceCapabilities);
    }
    VkResult result = DispatchGetPhysicalDeviceSurfaceCapabilities2KHR(physicalDevice, pSurfaceInfo, pSurfaceCapabilities);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceSurfaceFormats2KHR(physicalDevice, pSurfaceInfo, pSurfaceFormatCount, pSurfaceFormats);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetPhysicalDeviceSurfaceFormats2KHR(physicalDevice, pSurfaceInfo, pSurfaceFormatCount, pSurfaceFormats);
    }
    VkResult result = DispatchGetPhysicalDeviceSurfaceFormats2KHR(physicalDevice, pSurfaceInfo, pSurfaceFormatCount, pSurfaceFormats);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceDisplayProperties2KHR(physicalDevice, pPropertyCount, pProperties);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetPhysicalDeviceDisplayProperties2KHR(physicalDevice, pPropertyCount, pProperties);
    }
    VkResult result = DispatchGetPhysicalDeviceDisplayProperties2KHR(physicalDevice, pPropertyCount, pProperties);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceDisplayPlaneProperties2KHR(physicalDevice, pPropertyCount, pProperties);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetPhysicalDeviceDisplayPlaneProperties2KHR(physicalDevice, pPropertyCount, pProperties);
    }
    VkResult result = DispatchGetPhysicalDeviceDisplayPlaneProperties2KHR(physicalDevice, pPropertyCount, pProperties);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetDisplayModeProperties2KHR(physicalDevice, display, pPropertyCount, pProperties);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetDisplayModeProperties2KHR(physicalDevice, display, pPropertyCount, pProperties);
    }
    VkResult result = DispatchGetDisplayModeProperties2KHR(physicalDevice, display, pPropertyCount, pProperties);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetDisplayPlaneCapabilities2KHR(physicalDevice, pDisplayPlaneInfo, pCapabilities);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetDisplayPlaneCapabilities2KHR(physicalDevice, pDisplayPlaneInfo, pCapabilities);
    }
    VkResult result = DispatchGetDisplayPlaneCapabilities2KHR(physicalDevice, pDisplayPlaneInfo, pCapabilities);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetDisplayPlaneCapabilities2KHR(physicalDevice, pDisplayPlaneInfo, pCapabilities, result);
    }
    return result;
}





VKAPI_ATTR void VKAPI_CALL GetImageMemoryRequirements2KHR(
    VkDevice                                    device,
    const VkImageMemoryRequirementsInfo2*       pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetImageMemoryRequirements2KHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetImageMemoryRequirements2KHR(device, pInfo, pMemoryRequirements);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetImageMemoryRequirements2KHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetImageMemoryRequirements2KHR(device, pInfo, pMemoryRequirements);
    }
    DispatchGetImageMemoryRequirements2KHR(device, pInfo, pMemoryRequirements);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetImageMemoryRequirements2KHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetImageMemoryRequirements2KHR(device, pInfo, pMemoryRequirements);
    }
}

VKAPI_ATTR void VKAPI_CALL GetBufferMemoryRequirements2KHR(
    VkDevice                                    device,
    const VkBufferMemoryRequirementsInfo2*      pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetBufferMemoryRequirements2KHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetBufferMemoryRequirements2KHR(device, pInfo, pMemoryRequirements);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetBufferMemoryRequirements2KHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetBufferMemoryRequirements2KHR(device, pInfo, pMemoryRequirements);
    }
    DispatchGetBufferMemoryRequirements2KHR(device, pInfo, pMemoryRequirements);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetBufferMemoryRequirements2KHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetBufferMemoryRequirements2KHR(device, pInfo, pMemoryRequirements);
    }
}

VKAPI_ATTR void VKAPI_CALL GetImageSparseMemoryRequirements2KHR(
    VkDevice                                    device,
    const VkImageSparseMemoryRequirementsInfo2* pInfo,
    uint32_t*                                   pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2*           pSparseMemoryRequirements) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetImageSparseMemoryRequirements2KHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetImageSparseMemoryRequirements2KHR(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetImageSparseMemoryRequirements2KHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetImageSparseMemoryRequirements2KHR(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
    }
    DispatchGetImageSparseMemoryRequirements2KHR(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetImageSparseMemoryRequirements2KHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetImageSparseMemoryRequirements2KHR(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
    }
}



VKAPI_ATTR VkResult VKAPI_CALL CreateSamplerYcbcrConversionKHR(
    VkDevice                                    device,
    const VkSamplerYcbcrConversionCreateInfo*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSamplerYcbcrConversion*                   pYcbcrConversion) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCreateSamplerYcbcrConversionKHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateSamplerYcbcrConversionKHR(device, pCreateInfo, pAllocator, pYcbcrConversion);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateSamplerYcbcrConversionKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCreateSamplerYcbcrConversionKHR(device, pCreateInfo, pAllocator, pYcbcrConversion);
    }
    VkResult result = DispatchCreateSamplerYcbcrConversionKHR(device, pCreateInfo, pAllocator, pYcbcrConversion);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateSamplerYcbcrConversionKHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCreateSamplerYcbcrConversionKHR(device, pCreateInfo, pAllocator, pYcbcrConversion, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroySamplerYcbcrConversionKHR(
    VkDevice                                    device,
    VkSamplerYcbcrConversion                    ycbcrConversion,
    const VkAllocationCallbacks*                pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDestroySamplerYcbcrConversionKHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateDestroySamplerYcbcrConversionKHR(device, ycbcrConversion, pAllocator);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroySamplerYcbcrConversionKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordDestroySamplerYcbcrConversionKHR(device, ycbcrConversion, pAllocator);
    }
    DispatchDestroySamplerYcbcrConversionKHR(device, ycbcrConversion, pAllocator);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroySamplerYcbcrConversionKHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordDestroySamplerYcbcrConversionKHR(device, ycbcrConversion, pAllocator);
    }
}


VKAPI_ATTR VkResult VKAPI_CALL BindBufferMemory2KHR(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindBufferMemoryInfo*               pBindInfos) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateBindBufferMemory2KHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateBindBufferMemory2KHR(device, bindInfoCount, pBindInfos);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordBindBufferMemory2KHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordBindBufferMemory2KHR(device, bindInfoCount, pBindInfos);
    }
    VkResult result = DispatchBindBufferMemory2KHR(device, bindInfoCount, pBindInfos);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordBindBufferMemory2KHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordBindBufferMemory2KHR(device, bindInfoCount, pBindInfos, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL BindImageMemory2KHR(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindImageMemoryInfo*                pBindInfos) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateBindImageMemory2KHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateBindImageMemory2KHR(device, bindInfoCount, pBindInfos);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordBindImageMemory2KHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordBindImageMemory2KHR(device, bindInfoCount, pBindInfos);
    }
    VkResult result = DispatchBindImageMemory2KHR(device, bindInfoCount, pBindInfos);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordBindImageMemory2KHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordBindImageMemory2KHR(device, bindInfoCount, pBindInfos, result);
    }
    return result;
}

#ifdef VK_ENABLE_BETA_EXTENSIONS
#endif // VK_ENABLE_BETA_EXTENSIONS


VKAPI_ATTR void VKAPI_CALL GetDescriptorSetLayoutSupportKHR(
    VkDevice                                    device,
    const VkDescriptorSetLayoutCreateInfo*      pCreateInfo,
    VkDescriptorSetLayoutSupport*               pSupport) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetDescriptorSetLayoutSupportKHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetDescriptorSetLayoutSupportKHR(device, pCreateInfo, pSupport);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetDescriptorSetLayoutSupportKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetDescriptorSetLayoutSupportKHR(device, pCreateInfo, pSupport);
    }
    DispatchGetDescriptorSetLayoutSupportKHR(device, pCreateInfo, pSupport);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetDescriptorSetLayoutSupportKHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetDescriptorSetLayoutSupportKHR(device, pCreateInfo, pSupport);
    }
}


VKAPI_ATTR void VKAPI_CALL CmdDrawIndirectCountKHR(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdDrawIndirectCountKHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdDrawIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdDrawIndirectCountKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdDrawIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    }
    DispatchCmdDrawIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdDrawIndirectCountKHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdDrawIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdDrawIndexedIndirectCountKHR(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdDrawIndexedIndirectCountKHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdDrawIndexedIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdDrawIndexedIndirectCountKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdDrawIndexedIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    }
    DispatchCmdDrawIndexedIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdDrawIndexedIndirectCountKHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdDrawIndexedIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    }
}










VKAPI_ATTR VkResult VKAPI_CALL GetSemaphoreCounterValueKHR(
    VkDevice                                    device,
    VkSemaphore                                 semaphore,
    uint64_t*                                   pValue) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetSemaphoreCounterValueKHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetSemaphoreCounterValueKHR(device, semaphore, pValue);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetSemaphoreCounterValueKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetSemaphoreCounterValueKHR(device, semaphore, pValue);
    }
    VkResult result = DispatchGetSemaphoreCounterValueKHR(device, semaphore, pValue);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetSemaphoreCounterValueKHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetSemaphoreCounterValueKHR(device, semaphore, pValue, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL WaitSemaphoresKHR(
    VkDevice                                    device,
    const VkSemaphoreWaitInfo*                  pWaitInfo,
    uint64_t                                    timeout) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateWaitSemaphoresKHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateWaitSemaphoresKHR(device, pWaitInfo, timeout);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordWaitSemaphoresKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordWaitSemaphoresKHR(device, pWaitInfo, timeout);
    }
    VkResult result = DispatchWaitSemaphoresKHR(device, pWaitInfo, timeout);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordWaitSemaphoresKHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordWaitSemaphoresKHR(device, pWaitInfo, timeout, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL SignalSemaphoreKHR(
    VkDevice                                    device,
    const VkSemaphoreSignalInfo*                pSignalInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateSignalSemaphoreKHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateSignalSemaphoreKHR(device, pSignalInfo);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordSignalSemaphoreKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordSignalSemaphoreKHR(device, pSignalInfo);
    }
    VkResult result = DispatchSignalSemaphoreKHR(device, pSignalInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordSignalSemaphoreKHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordSignalSemaphoreKHR(device, pSignalInfo, result);
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceFragmentShadingRatesKHR(physicalDevice, pFragmentShadingRateCount, pFragmentShadingRates);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetPhysicalDeviceFragmentShadingRatesKHR(physicalDevice, pFragmentShadingRateCount, pFragmentShadingRates);
    }
    VkResult result = DispatchGetPhysicalDeviceFragmentShadingRatesKHR(physicalDevice, pFragmentShadingRateCount, pFragmentShadingRates);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetFragmentShadingRateKHR(commandBuffer, pFragmentSize, combinerOps);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetFragmentShadingRateKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdSetFragmentShadingRateKHR(commandBuffer, pFragmentSize, combinerOps);
    }
    DispatchCmdSetFragmentShadingRateKHR(commandBuffer, pFragmentSize, combinerOps);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetFragmentShadingRateKHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdSetFragmentShadingRateKHR(commandBuffer, pFragmentSize, combinerOps);
    }
}






VKAPI_ATTR VkDeviceAddress VKAPI_CALL GetBufferDeviceAddressKHR(
    VkDevice                                    device,
    const VkBufferDeviceAddressInfo*            pInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetBufferDeviceAddressKHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetBufferDeviceAddressKHR(device, pInfo);
        if (skip) return 0;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetBufferDeviceAddressKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetBufferDeviceAddressKHR(device, pInfo);
    }
    VkDeviceAddress result = DispatchGetBufferDeviceAddressKHR(device, pInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetBufferDeviceAddressKHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetBufferDeviceAddressKHR(device, pInfo, result);
    }
    return result;
}

VKAPI_ATTR uint64_t VKAPI_CALL GetBufferOpaqueCaptureAddressKHR(
    VkDevice                                    device,
    const VkBufferDeviceAddressInfo*            pInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetBufferOpaqueCaptureAddressKHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetBufferOpaqueCaptureAddressKHR(device, pInfo);
        if (skip) return 0;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetBufferOpaqueCaptureAddressKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetBufferOpaqueCaptureAddressKHR(device, pInfo);
    }
    uint64_t result = DispatchGetBufferOpaqueCaptureAddressKHR(device, pInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetBufferOpaqueCaptureAddressKHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetBufferOpaqueCaptureAddressKHR(device, pInfo);
    }
    return result;
}

VKAPI_ATTR uint64_t VKAPI_CALL GetDeviceMemoryOpaqueCaptureAddressKHR(
    VkDevice                                    device,
    const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetDeviceMemoryOpaqueCaptureAddressKHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetDeviceMemoryOpaqueCaptureAddressKHR(device, pInfo);
        if (skip) return 0;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetDeviceMemoryOpaqueCaptureAddressKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetDeviceMemoryOpaqueCaptureAddressKHR(device, pInfo);
    }
    uint64_t result = DispatchGetDeviceMemoryOpaqueCaptureAddressKHR(device, pInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetDeviceMemoryOpaqueCaptureAddressKHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetDeviceMemoryOpaqueCaptureAddressKHR(device, pInfo);
    }
    return result;
}


VKAPI_ATTR VkResult VKAPI_CALL CreateDeferredOperationKHR(
    VkDevice                                    device,
    const VkAllocationCallbacks*                pAllocator,
    VkDeferredOperationKHR*                     pDeferredOperation) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCreateDeferredOperationKHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateDeferredOperationKHR(device, pAllocator, pDeferredOperation);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateDeferredOperationKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCreateDeferredOperationKHR(device, pAllocator, pDeferredOperation);
    }
    VkResult result = DispatchCreateDeferredOperationKHR(device, pAllocator, pDeferredOperation);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateDeferredOperationKHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCreateDeferredOperationKHR(device, pAllocator, pDeferredOperation, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyDeferredOperationKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      operation,
    const VkAllocationCallbacks*                pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDestroyDeferredOperationKHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateDestroyDeferredOperationKHR(device, operation, pAllocator);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroyDeferredOperationKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordDestroyDeferredOperationKHR(device, operation, pAllocator);
    }
    DispatchDestroyDeferredOperationKHR(device, operation, pAllocator);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroyDeferredOperationKHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordDestroyDeferredOperationKHR(device, operation, pAllocator);
    }
}

VKAPI_ATTR uint32_t VKAPI_CALL GetDeferredOperationMaxConcurrencyKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      operation) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetDeferredOperationMaxConcurrencyKHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetDeferredOperationMaxConcurrencyKHR(device, operation);
        if (skip) return 0;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetDeferredOperationMaxConcurrencyKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetDeferredOperationMaxConcurrencyKHR(device, operation);
    }
    uint32_t result = DispatchGetDeferredOperationMaxConcurrencyKHR(device, operation);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetDeferredOperationMaxConcurrencyKHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetDeferredOperationMaxConcurrencyKHR(device, operation);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetDeferredOperationResultKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      operation) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetDeferredOperationResultKHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetDeferredOperationResultKHR(device, operation);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetDeferredOperationResultKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetDeferredOperationResultKHR(device, operation);
    }
    VkResult result = DispatchGetDeferredOperationResultKHR(device, operation);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetDeferredOperationResultKHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetDeferredOperationResultKHR(device, operation, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL DeferredOperationJoinKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      operation) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDeferredOperationJoinKHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateDeferredOperationJoinKHR(device, operation);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDeferredOperationJoinKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordDeferredOperationJoinKHR(device, operation);
    }
    VkResult result = DispatchDeferredOperationJoinKHR(device, operation);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDeferredOperationJoinKHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordDeferredOperationJoinKHR(device, operation, result);
    }
    return result;
}


VKAPI_ATTR VkResult VKAPI_CALL GetPipelineExecutablePropertiesKHR(
    VkDevice                                    device,
    const VkPipelineInfoKHR*                    pPipelineInfo,
    uint32_t*                                   pExecutableCount,
    VkPipelineExecutablePropertiesKHR*          pProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetPipelineExecutablePropertiesKHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPipelineExecutablePropertiesKHR(device, pPipelineInfo, pExecutableCount, pProperties);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetPipelineExecutablePropertiesKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetPipelineExecutablePropertiesKHR(device, pPipelineInfo, pExecutableCount, pProperties);
    }
    VkResult result = DispatchGetPipelineExecutablePropertiesKHR(device, pPipelineInfo, pExecutableCount, pProperties);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetPipelineExecutablePropertiesKHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetPipelineExecutablePropertiesKHR(device, pPipelineInfo, pExecutableCount, pProperties, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetPipelineExecutableStatisticsKHR(
    VkDevice                                    device,
    const VkPipelineExecutableInfoKHR*          pExecutableInfo,
    uint32_t*                                   pStatisticCount,
    VkPipelineExecutableStatisticKHR*           pStatistics) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetPipelineExecutableStatisticsKHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPipelineExecutableStatisticsKHR(device, pExecutableInfo, pStatisticCount, pStatistics);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetPipelineExecutableStatisticsKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetPipelineExecutableStatisticsKHR(device, pExecutableInfo, pStatisticCount, pStatistics);
    }
    VkResult result = DispatchGetPipelineExecutableStatisticsKHR(device, pExecutableInfo, pStatisticCount, pStatistics);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetPipelineExecutableStatisticsKHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetPipelineExecutableStatisticsKHR(device, pExecutableInfo, pStatisticCount, pStatistics, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetPipelineExecutableInternalRepresentationsKHR(
    VkDevice                                    device,
    const VkPipelineExecutableInfoKHR*          pExecutableInfo,
    uint32_t*                                   pInternalRepresentationCount,
    VkPipelineExecutableInternalRepresentationKHR* pInternalRepresentations) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetPipelineExecutableInternalRepresentationsKHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPipelineExecutableInternalRepresentationsKHR(device, pExecutableInfo, pInternalRepresentationCount, pInternalRepresentations);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetPipelineExecutableInternalRepresentationsKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetPipelineExecutableInternalRepresentationsKHR(device, pExecutableInfo, pInternalRepresentationCount, pInternalRepresentations);
    }
    VkResult result = DispatchGetPipelineExecutableInternalRepresentationsKHR(device, pExecutableInfo, pInternalRepresentationCount, pInternalRepresentations);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetPipelineExecutableInternalRepresentationsKHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetPipelineExecutableInternalRepresentationsKHR(device, pExecutableInfo, pInternalRepresentationCount, pInternalRepresentations, result);
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetEvent2KHR(commandBuffer, event, pDependencyInfo);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetEvent2KHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdSetEvent2KHR(commandBuffer, event, pDependencyInfo);
    }
    DispatchCmdSetEvent2KHR(commandBuffer, event, pDependencyInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetEvent2KHR]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdResetEvent2KHR(commandBuffer, event, stageMask);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdResetEvent2KHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdResetEvent2KHR(commandBuffer, event, stageMask);
    }
    DispatchCmdResetEvent2KHR(commandBuffer, event, stageMask);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdResetEvent2KHR]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdWaitEvents2KHR(commandBuffer, eventCount, pEvents, pDependencyInfos);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdWaitEvents2KHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdWaitEvents2KHR(commandBuffer, eventCount, pEvents, pDependencyInfos);
    }
    DispatchCmdWaitEvents2KHR(commandBuffer, eventCount, pEvents, pDependencyInfos);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdWaitEvents2KHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdWaitEvents2KHR(commandBuffer, eventCount, pEvents, pDependencyInfos);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdPipelineBarrier2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkDependencyInfoKHR*                  pDependencyInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdPipelineBarrier2KHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdPipelineBarrier2KHR(commandBuffer, pDependencyInfo);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdPipelineBarrier2KHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdPipelineBarrier2KHR(commandBuffer, pDependencyInfo);
    }
    DispatchCmdPipelineBarrier2KHR(commandBuffer, pDependencyInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdPipelineBarrier2KHR]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdWriteTimestamp2KHR(commandBuffer, stage, queryPool, query);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdWriteTimestamp2KHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdWriteTimestamp2KHR(commandBuffer, stage, queryPool, query);
    }
    DispatchCmdWriteTimestamp2KHR(commandBuffer, stage, queryPool, query);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdWriteTimestamp2KHR]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateQueueSubmit2KHR(queue, submitCount, pSubmits, fence);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordQueueSubmit2KHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordQueueSubmit2KHR(queue, submitCount, pSubmits, fence);
    }
    VkResult result = DispatchQueueSubmit2KHR(queue, submitCount, pSubmits, fence);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordQueueSubmit2KHR]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdWriteBufferMarker2AMD(commandBuffer, stage, dstBuffer, dstOffset, marker);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdWriteBufferMarker2AMD]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdWriteBufferMarker2AMD(commandBuffer, stage, dstBuffer, dstOffset, marker);
    }
    DispatchCmdWriteBufferMarker2AMD(commandBuffer, stage, dstBuffer, dstOffset, marker);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdWriteBufferMarker2AMD]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetQueueCheckpointData2NV(queue, pCheckpointDataCount, pCheckpointData);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetQueueCheckpointData2NV]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetQueueCheckpointData2NV(queue, pCheckpointDataCount, pCheckpointData);
    }
    DispatchGetQueueCheckpointData2NV(queue, pCheckpointDataCount, pCheckpointData);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetQueueCheckpointData2NV]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetQueueCheckpointData2NV(queue, pCheckpointDataCount, pCheckpointData);
    }
}




VKAPI_ATTR void VKAPI_CALL CmdCopyBuffer2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyBufferInfo2KHR*                 pCopyBufferInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdCopyBuffer2KHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdCopyBuffer2KHR(commandBuffer, pCopyBufferInfo);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdCopyBuffer2KHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdCopyBuffer2KHR(commandBuffer, pCopyBufferInfo);
    }
    DispatchCmdCopyBuffer2KHR(commandBuffer, pCopyBufferInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdCopyBuffer2KHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdCopyBuffer2KHR(commandBuffer, pCopyBufferInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdCopyImage2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyImageInfo2KHR*                  pCopyImageInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdCopyImage2KHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdCopyImage2KHR(commandBuffer, pCopyImageInfo);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdCopyImage2KHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdCopyImage2KHR(commandBuffer, pCopyImageInfo);
    }
    DispatchCmdCopyImage2KHR(commandBuffer, pCopyImageInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdCopyImage2KHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdCopyImage2KHR(commandBuffer, pCopyImageInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdCopyBufferToImage2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyBufferToImageInfo2KHR*          pCopyBufferToImageInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdCopyBufferToImage2KHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdCopyBufferToImage2KHR(commandBuffer, pCopyBufferToImageInfo);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdCopyBufferToImage2KHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdCopyBufferToImage2KHR(commandBuffer, pCopyBufferToImageInfo);
    }
    DispatchCmdCopyBufferToImage2KHR(commandBuffer, pCopyBufferToImageInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdCopyBufferToImage2KHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdCopyBufferToImage2KHR(commandBuffer, pCopyBufferToImageInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdCopyImageToBuffer2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyImageToBufferInfo2KHR*          pCopyImageToBufferInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdCopyImageToBuffer2KHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdCopyImageToBuffer2KHR(commandBuffer, pCopyImageToBufferInfo);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdCopyImageToBuffer2KHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdCopyImageToBuffer2KHR(commandBuffer, pCopyImageToBufferInfo);
    }
    DispatchCmdCopyImageToBuffer2KHR(commandBuffer, pCopyImageToBufferInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdCopyImageToBuffer2KHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdCopyImageToBuffer2KHR(commandBuffer, pCopyImageToBufferInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdBlitImage2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkBlitImageInfo2KHR*                  pBlitImageInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdBlitImage2KHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdBlitImage2KHR(commandBuffer, pBlitImageInfo);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdBlitImage2KHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdBlitImage2KHR(commandBuffer, pBlitImageInfo);
    }
    DispatchCmdBlitImage2KHR(commandBuffer, pBlitImageInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdBlitImage2KHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdBlitImage2KHR(commandBuffer, pBlitImageInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdResolveImage2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkResolveImageInfo2KHR*               pResolveImageInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdResolveImage2KHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdResolveImage2KHR(commandBuffer, pResolveImageInfo);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdResolveImage2KHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdResolveImage2KHR(commandBuffer, pResolveImageInfo);
    }
    DispatchCmdResolveImage2KHR(commandBuffer, pResolveImageInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdResolveImage2KHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdResolveImage2KHR(commandBuffer, pResolveImageInfo);
    }
}


VKAPI_ATTR VkResult VKAPI_CALL CreateDebugReportCallbackEXT(
    VkInstance                                  instance,
    const VkDebugReportCallbackCreateInfoEXT*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDebugReportCallbackEXT*                   pCallback) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateDebugReportCallbackEXT(instance, pCreateInfo, pAllocator, pCallback);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCreateDebugReportCallbackEXT(instance, pCreateInfo, pAllocator, pCallback);
    }
    VkResult result = DispatchCreateDebugReportCallbackEXT(instance, pCreateInfo, pAllocator, pCallback);
    layer_create_report_callback(layer_data->report_data, false, pCreateInfo, pAllocator, pCallback);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCreateDebugReportCallbackEXT(instance, pCreateInfo, pAllocator, pCallback, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyDebugReportCallbackEXT(
    VkInstance                                  instance,
    VkDebugReportCallbackEXT                    callback,
    const VkAllocationCallbacks*                pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateDestroyDebugReportCallbackEXT(instance, callback, pAllocator);
        if (skip) return;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordDestroyDebugReportCallbackEXT(instance, callback, pAllocator);
    }
    DispatchDestroyDebugReportCallbackEXT(instance, callback, pAllocator);
    layer_destroy_callback(layer_data->report_data, callback, pAllocator);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordDestroyDebugReportCallbackEXT(instance, callback, pAllocator);
    }
}

VKAPI_ATTR void VKAPI_CALL DebugReportMessageEXT(
    VkInstance                                  instance,
    VkDebugReportFlagsEXT                       flags,
    VkDebugReportObjectTypeEXT                  objectType,
    uint64_t                                    object,
    size_t                                      location,
    int32_t                                     messageCode,
    const char*                                 pLayerPrefix,
    const char*                                 pMessage) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateDebugReportMessageEXT(instance, flags, objectType, object, location, messageCode, pLayerPrefix, pMessage);
        if (skip) return;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordDebugReportMessageEXT(instance, flags, objectType, object, location, messageCode, pLayerPrefix, pMessage);
    }
    DispatchDebugReportMessageEXT(instance, flags, objectType, object, location, messageCode, pLayerPrefix, pMessage);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordDebugReportMessageEXT(instance, flags, objectType, object, location, messageCode, pLayerPrefix, pMessage);
    }
}








VKAPI_ATTR VkResult VKAPI_CALL DebugMarkerSetObjectTagEXT(
    VkDevice                                    device,
    const VkDebugMarkerObjectTagInfoEXT*        pTagInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDebugMarkerSetObjectTagEXT]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateDebugMarkerSetObjectTagEXT(device, pTagInfo);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDebugMarkerSetObjectTagEXT]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordDebugMarkerSetObjectTagEXT(device, pTagInfo);
    }
    VkResult result = DispatchDebugMarkerSetObjectTagEXT(device, pTagInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDebugMarkerSetObjectTagEXT]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordDebugMarkerSetObjectTagEXT(device, pTagInfo, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL DebugMarkerSetObjectNameEXT(
    VkDevice                                    device,
    const VkDebugMarkerObjectNameInfoEXT*       pNameInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDebugMarkerSetObjectNameEXT]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateDebugMarkerSetObjectNameEXT(device, pNameInfo);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDebugMarkerSetObjectNameEXT]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordDebugMarkerSetObjectNameEXT(device, pNameInfo);
    }
    layer_data->report_data->DebugReportSetMarkerObjectName(pNameInfo);
    VkResult result = DispatchDebugMarkerSetObjectNameEXT(device, pNameInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDebugMarkerSetObjectNameEXT]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordDebugMarkerSetObjectNameEXT(device, pNameInfo, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL CmdDebugMarkerBeginEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugMarkerMarkerInfoEXT*           pMarkerInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdDebugMarkerBeginEXT]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdDebugMarkerBeginEXT(commandBuffer, pMarkerInfo);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdDebugMarkerBeginEXT]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdDebugMarkerBeginEXT(commandBuffer, pMarkerInfo);
    }
    DispatchCmdDebugMarkerBeginEXT(commandBuffer, pMarkerInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdDebugMarkerBeginEXT]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdDebugMarkerBeginEXT(commandBuffer, pMarkerInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdDebugMarkerEndEXT(
    VkCommandBuffer                             commandBuffer) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdDebugMarkerEndEXT]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdDebugMarkerEndEXT(commandBuffer);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdDebugMarkerEndEXT]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdDebugMarkerEndEXT(commandBuffer);
    }
    DispatchCmdDebugMarkerEndEXT(commandBuffer);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdDebugMarkerEndEXT]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdDebugMarkerEndEXT(commandBuffer);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdDebugMarkerInsertEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugMarkerMarkerInfoEXT*           pMarkerInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdDebugMarkerInsertEXT]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdDebugMarkerInsertEXT(commandBuffer, pMarkerInfo);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdDebugMarkerInsertEXT]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdDebugMarkerInsertEXT(commandBuffer, pMarkerInfo);
    }
    DispatchCmdDebugMarkerInsertEXT(commandBuffer, pMarkerInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdDebugMarkerInsertEXT]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdDebugMarkerInsertEXT(commandBuffer, pMarkerInfo);
    }
}




VKAPI_ATTR void VKAPI_CALL CmdBindTransformFeedbackBuffersEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstBinding,
    uint32_t                                    bindingCount,
    const VkBuffer*                             pBuffers,
    const VkDeviceSize*                         pOffsets,
    const VkDeviceSize*                         pSizes) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdBindTransformFeedbackBuffersEXT]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdBindTransformFeedbackBuffersEXT(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdBindTransformFeedbackBuffersEXT]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdBindTransformFeedbackBuffersEXT(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes);
    }
    DispatchCmdBindTransformFeedbackBuffersEXT(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdBindTransformFeedbackBuffersEXT]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdBindTransformFeedbackBuffersEXT(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdBeginTransformFeedbackEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstCounterBuffer,
    uint32_t                                    counterBufferCount,
    const VkBuffer*                             pCounterBuffers,
    const VkDeviceSize*                         pCounterBufferOffsets) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdBeginTransformFeedbackEXT]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdBeginTransformFeedbackEXT(commandBuffer, firstCounterBuffer, counterBufferCount, pCounterBuffers, pCounterBufferOffsets);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdBeginTransformFeedbackEXT]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdBeginTransformFeedbackEXT(commandBuffer, firstCounterBuffer, counterBufferCount, pCounterBuffers, pCounterBufferOffsets);
    }
    DispatchCmdBeginTransformFeedbackEXT(commandBuffer, firstCounterBuffer, counterBufferCount, pCounterBuffers, pCounterBufferOffsets);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdBeginTransformFeedbackEXT]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdBeginTransformFeedbackEXT(commandBuffer, firstCounterBuffer, counterBufferCount, pCounterBuffers, pCounterBufferOffsets);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdEndTransformFeedbackEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstCounterBuffer,
    uint32_t                                    counterBufferCount,
    const VkBuffer*                             pCounterBuffers,
    const VkDeviceSize*                         pCounterBufferOffsets) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdEndTransformFeedbackEXT]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdEndTransformFeedbackEXT(commandBuffer, firstCounterBuffer, counterBufferCount, pCounterBuffers, pCounterBufferOffsets);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdEndTransformFeedbackEXT]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdEndTransformFeedbackEXT(commandBuffer, firstCounterBuffer, counterBufferCount, pCounterBuffers, pCounterBufferOffsets);
    }
    DispatchCmdEndTransformFeedbackEXT(commandBuffer, firstCounterBuffer, counterBufferCount, pCounterBuffers, pCounterBufferOffsets);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdEndTransformFeedbackEXT]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdEndTransformFeedbackEXT(commandBuffer, firstCounterBuffer, counterBufferCount, pCounterBuffers, pCounterBufferOffsets);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdBeginQueryIndexedEXT(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    query,
    VkQueryControlFlags                         flags,
    uint32_t                                    index) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdBeginQueryIndexedEXT]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdBeginQueryIndexedEXT(commandBuffer, queryPool, query, flags, index);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdBeginQueryIndexedEXT]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdBeginQueryIndexedEXT(commandBuffer, queryPool, query, flags, index);
    }
    DispatchCmdBeginQueryIndexedEXT(commandBuffer, queryPool, query, flags, index);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdBeginQueryIndexedEXT]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdBeginQueryIndexedEXT(commandBuffer, queryPool, query, flags, index);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdEndQueryIndexedEXT(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    query,
    uint32_t                                    index) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdEndQueryIndexedEXT]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdEndQueryIndexedEXT(commandBuffer, queryPool, query, index);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdEndQueryIndexedEXT]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdEndQueryIndexedEXT(commandBuffer, queryPool, query, index);
    }
    DispatchCmdEndQueryIndexedEXT(commandBuffer, queryPool, query, index);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdEndQueryIndexedEXT]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdEndQueryIndexedEXT(commandBuffer, queryPool, query, index);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdDrawIndirectByteCountEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    instanceCount,
    uint32_t                                    firstInstance,
    VkBuffer                                    counterBuffer,
    VkDeviceSize                                counterBufferOffset,
    uint32_t                                    counterOffset,
    uint32_t                                    vertexStride) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdDrawIndirectByteCountEXT]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdDrawIndirectByteCountEXT(commandBuffer, instanceCount, firstInstance, counterBuffer, counterBufferOffset, counterOffset, vertexStride);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdDrawIndirectByteCountEXT]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdDrawIndirectByteCountEXT(commandBuffer, instanceCount, firstInstance, counterBuffer, counterBufferOffset, counterOffset, vertexStride);
    }
    DispatchCmdDrawIndirectByteCountEXT(commandBuffer, instanceCount, firstInstance, counterBuffer, counterBufferOffset, counterOffset, vertexStride);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdDrawIndirectByteCountEXT]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdDrawIndirectByteCountEXT(commandBuffer, instanceCount, firstInstance, counterBuffer, counterBufferOffset, counterOffset, vertexStride);
    }
}


VKAPI_ATTR uint32_t VKAPI_CALL GetImageViewHandleNVX(
    VkDevice                                    device,
    const VkImageViewHandleInfoNVX*             pInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetImageViewHandleNVX]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetImageViewHandleNVX(device, pInfo);
        if (skip) return 0;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetImageViewHandleNVX]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetImageViewHandleNVX(device, pInfo);
    }
    uint32_t result = DispatchGetImageViewHandleNVX(device, pInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetImageViewHandleNVX]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetImageViewHandleNVX(device, pInfo);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetImageViewAddressNVX(
    VkDevice                                    device,
    VkImageView                                 imageView,
    VkImageViewAddressPropertiesNVX*            pProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetImageViewAddressNVX]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetImageViewAddressNVX(device, imageView, pProperties);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetImageViewAddressNVX]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetImageViewAddressNVX(device, imageView, pProperties);
    }
    VkResult result = DispatchGetImageViewAddressNVX(device, imageView, pProperties);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetImageViewAddressNVX]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetImageViewAddressNVX(device, imageView, pProperties, result);
    }
    return result;
}


VKAPI_ATTR void VKAPI_CALL CmdDrawIndirectCountAMD(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdDrawIndirectCountAMD]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdDrawIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdDrawIndirectCountAMD]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdDrawIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    }
    DispatchCmdDrawIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdDrawIndirectCountAMD]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdDrawIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdDrawIndexedIndirectCountAMD(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdDrawIndexedIndirectCountAMD]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdDrawIndexedIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdDrawIndexedIndirectCountAMD]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdDrawIndexedIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    }
    DispatchCmdDrawIndexedIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdDrawIndexedIndirectCountAMD]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdDrawIndexedIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    }
}






VKAPI_ATTR VkResult VKAPI_CALL GetShaderInfoAMD(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    VkShaderStageFlagBits                       shaderStage,
    VkShaderInfoTypeAMD                         infoType,
    size_t*                                     pInfoSize,
    void*                                       pInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetShaderInfoAMD]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetShaderInfoAMD(device, pipeline, shaderStage, infoType, pInfoSize, pInfo);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetShaderInfoAMD]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetShaderInfoAMD(device, pipeline, shaderStage, infoType, pInfoSize, pInfo);
    }
    VkResult result = DispatchGetShaderInfoAMD(device, pipeline, shaderStage, infoType, pInfoSize, pInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetShaderInfoAMD]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetShaderInfoAMD(device, pipeline, shaderStage, infoType, pInfoSize, pInfo, result);
    }
    return result;
}


#ifdef VK_USE_PLATFORM_GGP

VKAPI_ATTR VkResult VKAPI_CALL CreateStreamDescriptorSurfaceGGP(
    VkInstance                                  instance,
    const VkStreamDescriptorSurfaceCreateInfoGGP* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateStreamDescriptorSurfaceGGP(instance, pCreateInfo, pAllocator, pSurface);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCreateStreamDescriptorSurfaceGGP(instance, pCreateInfo, pAllocator, pSurface);
    }
    VkResult result = DispatchCreateStreamDescriptorSurfaceGGP(instance, pCreateInfo, pAllocator, pSurface);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCreateStreamDescriptorSurfaceGGP(instance, pCreateInfo, pAllocator, pSurface, result);
    }
    return result;
}
#endif // VK_USE_PLATFORM_GGP




VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceExternalImageFormatPropertiesNV(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkImageType                                 type,
    VkImageTiling                               tiling,
    VkImageUsageFlags                           usage,
    VkImageCreateFlags                          flags,
    VkExternalMemoryHandleTypeFlagsNV           externalHandleType,
    VkExternalImageFormatPropertiesNV*          pExternalImageFormatProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceExternalImageFormatPropertiesNV(physicalDevice, format, type, tiling, usage, flags, externalHandleType, pExternalImageFormatProperties);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetPhysicalDeviceExternalImageFormatPropertiesNV(physicalDevice, format, type, tiling, usage, flags, externalHandleType, pExternalImageFormatProperties);
    }
    VkResult result = DispatchGetPhysicalDeviceExternalImageFormatPropertiesNV(physicalDevice, format, type, tiling, usage, flags, externalHandleType, pExternalImageFormatProperties);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetPhysicalDeviceExternalImageFormatPropertiesNV(physicalDevice, format, type, tiling, usage, flags, externalHandleType, pExternalImageFormatProperties, result);
    }
    return result;
}


#ifdef VK_USE_PLATFORM_WIN32_KHR

VKAPI_ATTR VkResult VKAPI_CALL GetMemoryWin32HandleNV(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    VkExternalMemoryHandleTypeFlagsNV           handleType,
    HANDLE*                                     pHandle) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetMemoryWin32HandleNV]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetMemoryWin32HandleNV(device, memory, handleType, pHandle);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetMemoryWin32HandleNV]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetMemoryWin32HandleNV(device, memory, handleType, pHandle);
    }
    VkResult result = DispatchGetMemoryWin32HandleNV(device, memory, handleType, pHandle);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetMemoryWin32HandleNV]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetMemoryWin32HandleNV(device, memory, handleType, pHandle, result);
    }
    return result;
}
#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR
#endif // VK_USE_PLATFORM_WIN32_KHR


#ifdef VK_USE_PLATFORM_VI_NN

VKAPI_ATTR VkResult VKAPI_CALL CreateViSurfaceNN(
    VkInstance                                  instance,
    const VkViSurfaceCreateInfoNN*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateViSurfaceNN(instance, pCreateInfo, pAllocator, pSurface);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCreateViSurfaceNN(instance, pCreateInfo, pAllocator, pSurface);
    }
    VkResult result = DispatchCreateViSurfaceNN(instance, pCreateInfo, pAllocator, pSurface);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCreateViSurfaceNN(instance, pCreateInfo, pAllocator, pSurface, result);
    }
    return result;
}
#endif // VK_USE_PLATFORM_VI_NN






VKAPI_ATTR void VKAPI_CALL CmdBeginConditionalRenderingEXT(
    VkCommandBuffer                             commandBuffer,
    const VkConditionalRenderingBeginInfoEXT*   pConditionalRenderingBegin) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdBeginConditionalRenderingEXT]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdBeginConditionalRenderingEXT(commandBuffer, pConditionalRenderingBegin);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdBeginConditionalRenderingEXT]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdBeginConditionalRenderingEXT(commandBuffer, pConditionalRenderingBegin);
    }
    DispatchCmdBeginConditionalRenderingEXT(commandBuffer, pConditionalRenderingBegin);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdBeginConditionalRenderingEXT]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdBeginConditionalRenderingEXT(commandBuffer, pConditionalRenderingBegin);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdEndConditionalRenderingEXT(
    VkCommandBuffer                             commandBuffer) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdEndConditionalRenderingEXT]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdEndConditionalRenderingEXT(commandBuffer);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdEndConditionalRenderingEXT]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdEndConditionalRenderingEXT(commandBuffer);
    }
    DispatchCmdEndConditionalRenderingEXT(commandBuffer);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdEndConditionalRenderingEXT]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdEndConditionalRenderingEXT(commandBuffer);
    }
}


VKAPI_ATTR void VKAPI_CALL CmdSetViewportWScalingNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstViewport,
    uint32_t                                    viewportCount,
    const VkViewportWScalingNV*                 pViewportWScalings) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetViewportWScalingNV]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetViewportWScalingNV(commandBuffer, firstViewport, viewportCount, pViewportWScalings);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetViewportWScalingNV]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdSetViewportWScalingNV(commandBuffer, firstViewport, viewportCount, pViewportWScalings);
    }
    DispatchCmdSetViewportWScalingNV(commandBuffer, firstViewport, viewportCount, pViewportWScalings);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetViewportWScalingNV]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdSetViewportWScalingNV(commandBuffer, firstViewport, viewportCount, pViewportWScalings);
    }
}


VKAPI_ATTR VkResult VKAPI_CALL ReleaseDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateReleaseDisplayEXT(physicalDevice, display);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordReleaseDisplayEXT(physicalDevice, display);
    }
    VkResult result = DispatchReleaseDisplayEXT(physicalDevice, display);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordReleaseDisplayEXT(physicalDevice, display, result);
    }
    return result;
}

#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT

VKAPI_ATTR VkResult VKAPI_CALL AcquireXlibDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    Display*                                    dpy,
    VkDisplayKHR                                display) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateAcquireXlibDisplayEXT(physicalDevice, dpy, display);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordAcquireXlibDisplayEXT(physicalDevice, dpy, display);
    }
    VkResult result = DispatchAcquireXlibDisplayEXT(physicalDevice, dpy, display);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordAcquireXlibDisplayEXT(physicalDevice, dpy, display, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetRandROutputDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    Display*                                    dpy,
    RROutput                                    rrOutput,
    VkDisplayKHR*                               pDisplay) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetRandROutputDisplayEXT(physicalDevice, dpy, rrOutput, pDisplay);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetRandROutputDisplayEXT(physicalDevice, dpy, rrOutput, pDisplay);
    }
    VkResult result = DispatchGetRandROutputDisplayEXT(physicalDevice, dpy, rrOutput, pDisplay);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetRandROutputDisplayEXT(physicalDevice, dpy, rrOutput, pDisplay, result);
    }
    return result;
}
#endif // VK_USE_PLATFORM_XLIB_XRANDR_EXT


VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceSurfaceCapabilities2EXT(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    VkSurfaceCapabilities2EXT*                  pSurfaceCapabilities) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceSurfaceCapabilities2EXT(physicalDevice, surface, pSurfaceCapabilities);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetPhysicalDeviceSurfaceCapabilities2EXT(physicalDevice, surface, pSurfaceCapabilities);
    }
    VkResult result = DispatchGetPhysicalDeviceSurfaceCapabilities2EXT(physicalDevice, surface, pSurfaceCapabilities);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateDisplayPowerControlEXT(device, display, pDisplayPowerInfo);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDisplayPowerControlEXT]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordDisplayPowerControlEXT(device, display, pDisplayPowerInfo);
    }
    VkResult result = DispatchDisplayPowerControlEXT(device, display, pDisplayPowerInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDisplayPowerControlEXT]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateRegisterDeviceEventEXT(device, pDeviceEventInfo, pAllocator, pFence);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordRegisterDeviceEventEXT]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordRegisterDeviceEventEXT(device, pDeviceEventInfo, pAllocator, pFence);
    }
    VkResult result = DispatchRegisterDeviceEventEXT(device, pDeviceEventInfo, pAllocator, pFence);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordRegisterDeviceEventEXT]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateRegisterDisplayEventEXT(device, display, pDisplayEventInfo, pAllocator, pFence);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordRegisterDisplayEventEXT]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordRegisterDisplayEventEXT(device, display, pDisplayEventInfo, pAllocator, pFence);
    }
    VkResult result = DispatchRegisterDisplayEventEXT(device, display, pDisplayEventInfo, pAllocator, pFence);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordRegisterDisplayEventEXT]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetSwapchainCounterEXT(device, swapchain, counter, pCounterValue);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetSwapchainCounterEXT]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetSwapchainCounterEXT(device, swapchain, counter, pCounterValue);
    }
    VkResult result = DispatchGetSwapchainCounterEXT(device, swapchain, counter, pCounterValue);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetSwapchainCounterEXT]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetSwapchainCounterEXT(device, swapchain, counter, pCounterValue, result);
    }
    return result;
}


VKAPI_ATTR VkResult VKAPI_CALL GetRefreshCycleDurationGOOGLE(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    VkRefreshCycleDurationGOOGLE*               pDisplayTimingProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetRefreshCycleDurationGOOGLE]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetRefreshCycleDurationGOOGLE(device, swapchain, pDisplayTimingProperties);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetRefreshCycleDurationGOOGLE]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetRefreshCycleDurationGOOGLE(device, swapchain, pDisplayTimingProperties);
    }
    VkResult result = DispatchGetRefreshCycleDurationGOOGLE(device, swapchain, pDisplayTimingProperties);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetRefreshCycleDurationGOOGLE]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetRefreshCycleDurationGOOGLE(device, swapchain, pDisplayTimingProperties, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetPastPresentationTimingGOOGLE(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    uint32_t*                                   pPresentationTimingCount,
    VkPastPresentationTimingGOOGLE*             pPresentationTimings) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetPastPresentationTimingGOOGLE]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPastPresentationTimingGOOGLE(device, swapchain, pPresentationTimingCount, pPresentationTimings);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetPastPresentationTimingGOOGLE]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetPastPresentationTimingGOOGLE(device, swapchain, pPresentationTimingCount, pPresentationTimings);
    }
    VkResult result = DispatchGetPastPresentationTimingGOOGLE(device, swapchain, pPresentationTimingCount, pPresentationTimings);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetPastPresentationTimingGOOGLE]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetPastPresentationTimingGOOGLE(device, swapchain, pPresentationTimingCount, pPresentationTimings, result);
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetDiscardRectangleEXT(commandBuffer, firstDiscardRectangle, discardRectangleCount, pDiscardRectangles);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetDiscardRectangleEXT]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdSetDiscardRectangleEXT(commandBuffer, firstDiscardRectangle, discardRectangleCount, pDiscardRectangles);
    }
    DispatchCmdSetDiscardRectangleEXT(commandBuffer, firstDiscardRectangle, discardRectangleCount, pDiscardRectangles);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetDiscardRectangleEXT]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateSetHdrMetadataEXT(device, swapchainCount, pSwapchains, pMetadata);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordSetHdrMetadataEXT]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordSetHdrMetadataEXT(device, swapchainCount, pSwapchains, pMetadata);
    }
    DispatchSetHdrMetadataEXT(device, swapchainCount, pSwapchains, pMetadata);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordSetHdrMetadataEXT]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordSetHdrMetadataEXT(device, swapchainCount, pSwapchains, pMetadata);
    }
}

#ifdef VK_USE_PLATFORM_IOS_MVK

VKAPI_ATTR VkResult VKAPI_CALL CreateIOSSurfaceMVK(
    VkInstance                                  instance,
    const VkIOSSurfaceCreateInfoMVK*            pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateIOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCreateIOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface);
    }
    VkResult result = DispatchCreateIOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCreateIOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface, result);
    }
    return result;
}
#endif // VK_USE_PLATFORM_IOS_MVK

#ifdef VK_USE_PLATFORM_MACOS_MVK

VKAPI_ATTR VkResult VKAPI_CALL CreateMacOSSurfaceMVK(
    VkInstance                                  instance,
    const VkMacOSSurfaceCreateInfoMVK*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateMacOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCreateMacOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface);
    }
    VkResult result = DispatchCreateMacOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCreateMacOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface, result);
    }
    return result;
}
#endif // VK_USE_PLATFORM_MACOS_MVK




VKAPI_ATTR VkResult VKAPI_CALL SetDebugUtilsObjectNameEXT(
    VkDevice                                    device,
    const VkDebugUtilsObjectNameInfoEXT*        pNameInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateSetDebugUtilsObjectNameEXT]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateSetDebugUtilsObjectNameEXT(device, pNameInfo);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordSetDebugUtilsObjectNameEXT]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordSetDebugUtilsObjectNameEXT(device, pNameInfo);
    }
    layer_data->report_data->DebugReportSetUtilsObjectName(pNameInfo);
    VkResult result = DispatchSetDebugUtilsObjectNameEXT(device, pNameInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordSetDebugUtilsObjectNameEXT]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateSetDebugUtilsObjectTagEXT(device, pTagInfo);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordSetDebugUtilsObjectTagEXT]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordSetDebugUtilsObjectTagEXT(device, pTagInfo);
    }
    VkResult result = DispatchSetDebugUtilsObjectTagEXT(device, pTagInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordSetDebugUtilsObjectTagEXT]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateQueueBeginDebugUtilsLabelEXT(queue, pLabelInfo);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordQueueBeginDebugUtilsLabelEXT]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordQueueBeginDebugUtilsLabelEXT(queue, pLabelInfo);
    }
    BeginQueueDebugUtilsLabel(layer_data->report_data, queue, pLabelInfo);
    DispatchQueueBeginDebugUtilsLabelEXT(queue, pLabelInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordQueueBeginDebugUtilsLabelEXT]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordQueueBeginDebugUtilsLabelEXT(queue, pLabelInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL QueueEndDebugUtilsLabelEXT(
    VkQueue                                     queue) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(queue), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateQueueEndDebugUtilsLabelEXT]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateQueueEndDebugUtilsLabelEXT(queue);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordQueueEndDebugUtilsLabelEXT]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordQueueEndDebugUtilsLabelEXT(queue);
    }
    DispatchQueueEndDebugUtilsLabelEXT(queue);
    EndQueueDebugUtilsLabel(layer_data->report_data, queue);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordQueueEndDebugUtilsLabelEXT]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordQueueEndDebugUtilsLabelEXT(queue);
    }
}

VKAPI_ATTR void VKAPI_CALL QueueInsertDebugUtilsLabelEXT(
    VkQueue                                     queue,
    const VkDebugUtilsLabelEXT*                 pLabelInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(queue), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateQueueInsertDebugUtilsLabelEXT]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateQueueInsertDebugUtilsLabelEXT(queue, pLabelInfo);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordQueueInsertDebugUtilsLabelEXT]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordQueueInsertDebugUtilsLabelEXT(queue, pLabelInfo);
    }
    InsertQueueDebugUtilsLabel(layer_data->report_data, queue, pLabelInfo);
    DispatchQueueInsertDebugUtilsLabelEXT(queue, pLabelInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordQueueInsertDebugUtilsLabelEXT]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordQueueInsertDebugUtilsLabelEXT(queue, pLabelInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdBeginDebugUtilsLabelEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugUtilsLabelEXT*                 pLabelInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdBeginDebugUtilsLabelEXT]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdBeginDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdBeginDebugUtilsLabelEXT]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdBeginDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
    }
    DispatchCmdBeginDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdBeginDebugUtilsLabelEXT]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdBeginDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdEndDebugUtilsLabelEXT(
    VkCommandBuffer                             commandBuffer) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdEndDebugUtilsLabelEXT]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdEndDebugUtilsLabelEXT(commandBuffer);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdEndDebugUtilsLabelEXT]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdEndDebugUtilsLabelEXT(commandBuffer);
    }
    DispatchCmdEndDebugUtilsLabelEXT(commandBuffer);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdEndDebugUtilsLabelEXT]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdEndDebugUtilsLabelEXT(commandBuffer);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdInsertDebugUtilsLabelEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugUtilsLabelEXT*                 pLabelInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdInsertDebugUtilsLabelEXT]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdInsertDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdInsertDebugUtilsLabelEXT]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdInsertDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
    }
    DispatchCmdInsertDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdInsertDebugUtilsLabelEXT]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);
    }
    VkResult result = DispatchCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);
    layer_create_messenger_callback(layer_data->report_data, false, pCreateInfo, pAllocator, pMessenger);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
        if (skip) return;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
    }
    DispatchDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
    layer_destroy_callback(layer_data->report_data, messenger, pAllocator);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateSubmitDebugUtilsMessageEXT(instance, messageSeverity, messageTypes, pCallbackData);
        if (skip) return;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordSubmitDebugUtilsMessageEXT(instance, messageSeverity, messageTypes, pCallbackData);
    }
    DispatchSubmitDebugUtilsMessageEXT(instance, messageSeverity, messageTypes, pCallbackData);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordSubmitDebugUtilsMessageEXT(instance, messageSeverity, messageTypes, pCallbackData);
    }
}

#ifdef VK_USE_PLATFORM_ANDROID_KHR

VKAPI_ATTR VkResult VKAPI_CALL GetAndroidHardwareBufferPropertiesANDROID(
    VkDevice                                    device,
    const struct AHardwareBuffer*               buffer,
    VkAndroidHardwareBufferPropertiesANDROID*   pProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetAndroidHardwareBufferPropertiesANDROID]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetAndroidHardwareBufferPropertiesANDROID(device, buffer, pProperties);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetAndroidHardwareBufferPropertiesANDROID]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetAndroidHardwareBufferPropertiesANDROID(device, buffer, pProperties);
    }
    VkResult result = DispatchGetAndroidHardwareBufferPropertiesANDROID(device, buffer, pProperties);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetAndroidHardwareBufferPropertiesANDROID]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetAndroidHardwareBufferPropertiesANDROID(device, buffer, pProperties, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetMemoryAndroidHardwareBufferANDROID(
    VkDevice                                    device,
    const VkMemoryGetAndroidHardwareBufferInfoANDROID* pInfo,
    struct AHardwareBuffer**                    pBuffer) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetMemoryAndroidHardwareBufferANDROID]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetMemoryAndroidHardwareBufferANDROID(device, pInfo, pBuffer);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetMemoryAndroidHardwareBufferANDROID]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetMemoryAndroidHardwareBufferANDROID(device, pInfo, pBuffer);
    }
    VkResult result = DispatchGetMemoryAndroidHardwareBufferANDROID(device, pInfo, pBuffer);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetMemoryAndroidHardwareBufferANDROID]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetMemoryAndroidHardwareBufferANDROID(device, pInfo, pBuffer, result);
    }
    return result;
}
#endif // VK_USE_PLATFORM_ANDROID_KHR








VKAPI_ATTR void VKAPI_CALL CmdSetSampleLocationsEXT(
    VkCommandBuffer                             commandBuffer,
    const VkSampleLocationsInfoEXT*             pSampleLocationsInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetSampleLocationsEXT]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetSampleLocationsEXT(commandBuffer, pSampleLocationsInfo);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetSampleLocationsEXT]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdSetSampleLocationsEXT(commandBuffer, pSampleLocationsInfo);
    }
    DispatchCmdSetSampleLocationsEXT(commandBuffer, pSampleLocationsInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetSampleLocationsEXT]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceMultisamplePropertiesEXT(physicalDevice, samples, pMultisampleProperties);
        if (skip) return;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetPhysicalDeviceMultisamplePropertiesEXT(physicalDevice, samples, pMultisampleProperties);
    }
    DispatchGetPhysicalDeviceMultisamplePropertiesEXT(physicalDevice, samples, pMultisampleProperties);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetImageDrmFormatModifierPropertiesEXT(device, image, pProperties);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetImageDrmFormatModifierPropertiesEXT]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetImageDrmFormatModifierPropertiesEXT(device, image, pProperties);
    }
    VkResult result = DispatchGetImageDrmFormatModifierPropertiesEXT(device, image, pProperties);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetImageDrmFormatModifierPropertiesEXT]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetImageDrmFormatModifierPropertiesEXT(device, image, pProperties, result);
    }
    return result;
}





VKAPI_ATTR void VKAPI_CALL CmdBindShadingRateImageNV(
    VkCommandBuffer                             commandBuffer,
    VkImageView                                 imageView,
    VkImageLayout                               imageLayout) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdBindShadingRateImageNV]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdBindShadingRateImageNV(commandBuffer, imageView, imageLayout);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdBindShadingRateImageNV]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdBindShadingRateImageNV(commandBuffer, imageView, imageLayout);
    }
    DispatchCmdBindShadingRateImageNV(commandBuffer, imageView, imageLayout);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdBindShadingRateImageNV]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdBindShadingRateImageNV(commandBuffer, imageView, imageLayout);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetViewportShadingRatePaletteNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstViewport,
    uint32_t                                    viewportCount,
    const VkShadingRatePaletteNV*               pShadingRatePalettes) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetViewportShadingRatePaletteNV]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetViewportShadingRatePaletteNV(commandBuffer, firstViewport, viewportCount, pShadingRatePalettes);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetViewportShadingRatePaletteNV]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdSetViewportShadingRatePaletteNV(commandBuffer, firstViewport, viewportCount, pShadingRatePalettes);
    }
    DispatchCmdSetViewportShadingRatePaletteNV(commandBuffer, firstViewport, viewportCount, pShadingRatePalettes);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetViewportShadingRatePaletteNV]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdSetViewportShadingRatePaletteNV(commandBuffer, firstViewport, viewportCount, pShadingRatePalettes);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetCoarseSampleOrderNV(
    VkCommandBuffer                             commandBuffer,
    VkCoarseSampleOrderTypeNV                   sampleOrderType,
    uint32_t                                    customSampleOrderCount,
    const VkCoarseSampleOrderCustomNV*          pCustomSampleOrders) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetCoarseSampleOrderNV]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetCoarseSampleOrderNV(commandBuffer, sampleOrderType, customSampleOrderCount, pCustomSampleOrders);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetCoarseSampleOrderNV]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdSetCoarseSampleOrderNV(commandBuffer, sampleOrderType, customSampleOrderCount, pCustomSampleOrders);
    }
    DispatchCmdSetCoarseSampleOrderNV(commandBuffer, sampleOrderType, customSampleOrderCount, pCustomSampleOrders);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetCoarseSampleOrderNV]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdSetCoarseSampleOrderNV(commandBuffer, sampleOrderType, customSampleOrderCount, pCustomSampleOrders);
    }
}


VKAPI_ATTR VkResult VKAPI_CALL CreateAccelerationStructureNV(
    VkDevice                                    device,
    const VkAccelerationStructureCreateInfoNV*  pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkAccelerationStructureNV*                  pAccelerationStructure) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCreateAccelerationStructureNV]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateAccelerationStructureNV(device, pCreateInfo, pAllocator, pAccelerationStructure);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateAccelerationStructureNV]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCreateAccelerationStructureNV(device, pCreateInfo, pAllocator, pAccelerationStructure);
    }
    VkResult result = DispatchCreateAccelerationStructureNV(device, pCreateInfo, pAllocator, pAccelerationStructure);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateAccelerationStructureNV]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCreateAccelerationStructureNV(device, pCreateInfo, pAllocator, pAccelerationStructure, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyAccelerationStructureNV(
    VkDevice                                    device,
    VkAccelerationStructureNV                   accelerationStructure,
    const VkAllocationCallbacks*                pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDestroyAccelerationStructureNV]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateDestroyAccelerationStructureNV(device, accelerationStructure, pAllocator);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroyAccelerationStructureNV]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordDestroyAccelerationStructureNV(device, accelerationStructure, pAllocator);
    }
    DispatchDestroyAccelerationStructureNV(device, accelerationStructure, pAllocator);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroyAccelerationStructureNV]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordDestroyAccelerationStructureNV(device, accelerationStructure, pAllocator);
    }
}

VKAPI_ATTR void VKAPI_CALL GetAccelerationStructureMemoryRequirementsNV(
    VkDevice                                    device,
    const VkAccelerationStructureMemoryRequirementsInfoNV* pInfo,
    VkMemoryRequirements2KHR*                   pMemoryRequirements) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetAccelerationStructureMemoryRequirementsNV]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetAccelerationStructureMemoryRequirementsNV(device, pInfo, pMemoryRequirements);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetAccelerationStructureMemoryRequirementsNV]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetAccelerationStructureMemoryRequirementsNV(device, pInfo, pMemoryRequirements);
    }
    DispatchGetAccelerationStructureMemoryRequirementsNV(device, pInfo, pMemoryRequirements);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetAccelerationStructureMemoryRequirementsNV]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetAccelerationStructureMemoryRequirementsNV(device, pInfo, pMemoryRequirements);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL BindAccelerationStructureMemoryNV(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindAccelerationStructureMemoryInfoNV* pBindInfos) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateBindAccelerationStructureMemoryNV]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateBindAccelerationStructureMemoryNV(device, bindInfoCount, pBindInfos);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordBindAccelerationStructureMemoryNV]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordBindAccelerationStructureMemoryNV(device, bindInfoCount, pBindInfos);
    }
    VkResult result = DispatchBindAccelerationStructureMemoryNV(device, bindInfoCount, pBindInfos);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordBindAccelerationStructureMemoryNV]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordBindAccelerationStructureMemoryNV(device, bindInfoCount, pBindInfos, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL CmdBuildAccelerationStructureNV(
    VkCommandBuffer                             commandBuffer,
    const VkAccelerationStructureInfoNV*        pInfo,
    VkBuffer                                    instanceData,
    VkDeviceSize                                instanceOffset,
    VkBool32                                    update,
    VkAccelerationStructureNV                   dst,
    VkAccelerationStructureNV                   src,
    VkBuffer                                    scratch,
    VkDeviceSize                                scratchOffset) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdBuildAccelerationStructureNV]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdBuildAccelerationStructureNV(commandBuffer, pInfo, instanceData, instanceOffset, update, dst, src, scratch, scratchOffset);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdBuildAccelerationStructureNV]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdBuildAccelerationStructureNV(commandBuffer, pInfo, instanceData, instanceOffset, update, dst, src, scratch, scratchOffset);
    }
    DispatchCmdBuildAccelerationStructureNV(commandBuffer, pInfo, instanceData, instanceOffset, update, dst, src, scratch, scratchOffset);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdBuildAccelerationStructureNV]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdBuildAccelerationStructureNV(commandBuffer, pInfo, instanceData, instanceOffset, update, dst, src, scratch, scratchOffset);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdCopyAccelerationStructureNV(
    VkCommandBuffer                             commandBuffer,
    VkAccelerationStructureNV                   dst,
    VkAccelerationStructureNV                   src,
    VkCopyAccelerationStructureModeKHR          mode) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdCopyAccelerationStructureNV]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdCopyAccelerationStructureNV(commandBuffer, dst, src, mode);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdCopyAccelerationStructureNV]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdCopyAccelerationStructureNV(commandBuffer, dst, src, mode);
    }
    DispatchCmdCopyAccelerationStructureNV(commandBuffer, dst, src, mode);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdCopyAccelerationStructureNV]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdCopyAccelerationStructureNV(commandBuffer, dst, src, mode);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdTraceRaysNV(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    raygenShaderBindingTableBuffer,
    VkDeviceSize                                raygenShaderBindingOffset,
    VkBuffer                                    missShaderBindingTableBuffer,
    VkDeviceSize                                missShaderBindingOffset,
    VkDeviceSize                                missShaderBindingStride,
    VkBuffer                                    hitShaderBindingTableBuffer,
    VkDeviceSize                                hitShaderBindingOffset,
    VkDeviceSize                                hitShaderBindingStride,
    VkBuffer                                    callableShaderBindingTableBuffer,
    VkDeviceSize                                callableShaderBindingOffset,
    VkDeviceSize                                callableShaderBindingStride,
    uint32_t                                    width,
    uint32_t                                    height,
    uint32_t                                    depth) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdTraceRaysNV]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdTraceRaysNV(commandBuffer, raygenShaderBindingTableBuffer, raygenShaderBindingOffset, missShaderBindingTableBuffer, missShaderBindingOffset, missShaderBindingStride, hitShaderBindingTableBuffer, hitShaderBindingOffset, hitShaderBindingStride, callableShaderBindingTableBuffer, callableShaderBindingOffset, callableShaderBindingStride, width, height, depth);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdTraceRaysNV]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdTraceRaysNV(commandBuffer, raygenShaderBindingTableBuffer, raygenShaderBindingOffset, missShaderBindingTableBuffer, missShaderBindingOffset, missShaderBindingStride, hitShaderBindingTableBuffer, hitShaderBindingOffset, hitShaderBindingStride, callableShaderBindingTableBuffer, callableShaderBindingOffset, callableShaderBindingStride, width, height, depth);
    }
    DispatchCmdTraceRaysNV(commandBuffer, raygenShaderBindingTableBuffer, raygenShaderBindingOffset, missShaderBindingTableBuffer, missShaderBindingOffset, missShaderBindingStride, hitShaderBindingTableBuffer, hitShaderBindingOffset, hitShaderBindingStride, callableShaderBindingTableBuffer, callableShaderBindingOffset, callableShaderBindingStride, width, height, depth);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdTraceRaysNV]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdTraceRaysNV(commandBuffer, raygenShaderBindingTableBuffer, raygenShaderBindingOffset, missShaderBindingTableBuffer, missShaderBindingOffset, missShaderBindingStride, hitShaderBindingTableBuffer, hitShaderBindingOffset, hitShaderBindingStride, callableShaderBindingTableBuffer, callableShaderBindingOffset, callableShaderBindingStride, width, height, depth);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL GetRayTracingShaderGroupHandlesKHR(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    firstGroup,
    uint32_t                                    groupCount,
    size_t                                      dataSize,
    void*                                       pData) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetRayTracingShaderGroupHandlesKHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetRayTracingShaderGroupHandlesKHR(device, pipeline, firstGroup, groupCount, dataSize, pData);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetRayTracingShaderGroupHandlesKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetRayTracingShaderGroupHandlesKHR(device, pipeline, firstGroup, groupCount, dataSize, pData);
    }
    VkResult result = DispatchGetRayTracingShaderGroupHandlesKHR(device, pipeline, firstGroup, groupCount, dataSize, pData);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetRayTracingShaderGroupHandlesKHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetRayTracingShaderGroupHandlesKHR(device, pipeline, firstGroup, groupCount, dataSize, pData, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetRayTracingShaderGroupHandlesNV(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    firstGroup,
    uint32_t                                    groupCount,
    size_t                                      dataSize,
    void*                                       pData) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetRayTracingShaderGroupHandlesNV]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetRayTracingShaderGroupHandlesNV(device, pipeline, firstGroup, groupCount, dataSize, pData);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetRayTracingShaderGroupHandlesNV]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetRayTracingShaderGroupHandlesNV(device, pipeline, firstGroup, groupCount, dataSize, pData);
    }
    VkResult result = DispatchGetRayTracingShaderGroupHandlesNV(device, pipeline, firstGroup, groupCount, dataSize, pData);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetRayTracingShaderGroupHandlesNV]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetRayTracingShaderGroupHandlesNV(device, pipeline, firstGroup, groupCount, dataSize, pData, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetAccelerationStructureHandleNV(
    VkDevice                                    device,
    VkAccelerationStructureNV                   accelerationStructure,
    size_t                                      dataSize,
    void*                                       pData) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetAccelerationStructureHandleNV]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetAccelerationStructureHandleNV(device, accelerationStructure, dataSize, pData);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetAccelerationStructureHandleNV]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetAccelerationStructureHandleNV(device, accelerationStructure, dataSize, pData);
    }
    VkResult result = DispatchGetAccelerationStructureHandleNV(device, accelerationStructure, dataSize, pData);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetAccelerationStructureHandleNV]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetAccelerationStructureHandleNV(device, accelerationStructure, dataSize, pData, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL CmdWriteAccelerationStructuresPropertiesNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    accelerationStructureCount,
    const VkAccelerationStructureNV*            pAccelerationStructures,
    VkQueryType                                 queryType,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdWriteAccelerationStructuresPropertiesNV]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdWriteAccelerationStructuresPropertiesNV(commandBuffer, accelerationStructureCount, pAccelerationStructures, queryType, queryPool, firstQuery);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdWriteAccelerationStructuresPropertiesNV]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdWriteAccelerationStructuresPropertiesNV(commandBuffer, accelerationStructureCount, pAccelerationStructures, queryType, queryPool, firstQuery);
    }
    DispatchCmdWriteAccelerationStructuresPropertiesNV(commandBuffer, accelerationStructureCount, pAccelerationStructures, queryType, queryPool, firstQuery);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdWriteAccelerationStructuresPropertiesNV]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdWriteAccelerationStructuresPropertiesNV(commandBuffer, accelerationStructureCount, pAccelerationStructures, queryType, queryPool, firstQuery);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL CompileDeferredNV(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    shader) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCompileDeferredNV]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCompileDeferredNV(device, pipeline, shader);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCompileDeferredNV]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCompileDeferredNV(device, pipeline, shader);
    }
    VkResult result = DispatchCompileDeferredNV(device, pipeline, shader);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCompileDeferredNV]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCompileDeferredNV(device, pipeline, shader, result);
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetMemoryHostPointerPropertiesEXT(device, handleType, pHostPointer, pMemoryHostPointerProperties);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetMemoryHostPointerPropertiesEXT]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetMemoryHostPointerPropertiesEXT(device, handleType, pHostPointer, pMemoryHostPointerProperties);
    }
    VkResult result = DispatchGetMemoryHostPointerPropertiesEXT(device, handleType, pHostPointer, pMemoryHostPointerProperties);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetMemoryHostPointerPropertiesEXT]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetMemoryHostPointerPropertiesEXT(device, handleType, pHostPointer, pMemoryHostPointerProperties, result);
    }
    return result;
}


VKAPI_ATTR void VKAPI_CALL CmdWriteBufferMarkerAMD(
    VkCommandBuffer                             commandBuffer,
    VkPipelineStageFlagBits                     pipelineStage,
    VkBuffer                                    dstBuffer,
    VkDeviceSize                                dstOffset,
    uint32_t                                    marker) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdWriteBufferMarkerAMD]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdWriteBufferMarkerAMD(commandBuffer, pipelineStage, dstBuffer, dstOffset, marker);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdWriteBufferMarkerAMD]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdWriteBufferMarkerAMD(commandBuffer, pipelineStage, dstBuffer, dstOffset, marker);
    }
    DispatchCmdWriteBufferMarkerAMD(commandBuffer, pipelineStage, dstBuffer, dstOffset, marker);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdWriteBufferMarkerAMD]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdWriteBufferMarkerAMD(commandBuffer, pipelineStage, dstBuffer, dstOffset, marker);
    }
}



VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceCalibrateableTimeDomainsEXT(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pTimeDomainCount,
    VkTimeDomainEXT*                            pTimeDomains) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceCalibrateableTimeDomainsEXT(physicalDevice, pTimeDomainCount, pTimeDomains);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetPhysicalDeviceCalibrateableTimeDomainsEXT(physicalDevice, pTimeDomainCount, pTimeDomains);
    }
    VkResult result = DispatchGetPhysicalDeviceCalibrateableTimeDomainsEXT(physicalDevice, pTimeDomainCount, pTimeDomains);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetCalibratedTimestampsEXT(device, timestampCount, pTimestampInfos, pTimestamps, pMaxDeviation);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetCalibratedTimestampsEXT]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetCalibratedTimestampsEXT(device, timestampCount, pTimestampInfos, pTimestamps, pMaxDeviation);
    }
    VkResult result = DispatchGetCalibratedTimestampsEXT(device, timestampCount, pTimestampInfos, pTimestamps, pMaxDeviation);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetCalibratedTimestampsEXT]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetCalibratedTimestampsEXT(device, timestampCount, pTimestampInfos, pTimestamps, pMaxDeviation, result);
    }
    return result;
}




#ifdef VK_USE_PLATFORM_GGP
#endif // VK_USE_PLATFORM_GGP





VKAPI_ATTR void VKAPI_CALL CmdDrawMeshTasksNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    taskCount,
    uint32_t                                    firstTask) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdDrawMeshTasksNV]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdDrawMeshTasksNV(commandBuffer, taskCount, firstTask);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdDrawMeshTasksNV]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdDrawMeshTasksNV(commandBuffer, taskCount, firstTask);
    }
    DispatchCmdDrawMeshTasksNV(commandBuffer, taskCount, firstTask);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdDrawMeshTasksNV]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdDrawMeshTasksNV(commandBuffer, taskCount, firstTask);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdDrawMeshTasksIndirectNV(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    uint32_t                                    drawCount,
    uint32_t                                    stride) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdDrawMeshTasksIndirectNV]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdDrawMeshTasksIndirectNV(commandBuffer, buffer, offset, drawCount, stride);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdDrawMeshTasksIndirectNV]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdDrawMeshTasksIndirectNV(commandBuffer, buffer, offset, drawCount, stride);
    }
    DispatchCmdDrawMeshTasksIndirectNV(commandBuffer, buffer, offset, drawCount, stride);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdDrawMeshTasksIndirectNV]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdDrawMeshTasksIndirectNV(commandBuffer, buffer, offset, drawCount, stride);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdDrawMeshTasksIndirectCountNV(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdDrawMeshTasksIndirectCountNV]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdDrawMeshTasksIndirectCountNV(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdDrawMeshTasksIndirectCountNV]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdDrawMeshTasksIndirectCountNV(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    }
    DispatchCmdDrawMeshTasksIndirectCountNV(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdDrawMeshTasksIndirectCountNV]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdDrawMeshTasksIndirectCountNV(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    }
}




VKAPI_ATTR void VKAPI_CALL CmdSetExclusiveScissorNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstExclusiveScissor,
    uint32_t                                    exclusiveScissorCount,
    const VkRect2D*                             pExclusiveScissors) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetExclusiveScissorNV]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetExclusiveScissorNV(commandBuffer, firstExclusiveScissor, exclusiveScissorCount, pExclusiveScissors);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetExclusiveScissorNV]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdSetExclusiveScissorNV(commandBuffer, firstExclusiveScissor, exclusiveScissorCount, pExclusiveScissors);
    }
    DispatchCmdSetExclusiveScissorNV(commandBuffer, firstExclusiveScissor, exclusiveScissorCount, pExclusiveScissors);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetExclusiveScissorNV]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdSetExclusiveScissorNV(commandBuffer, firstExclusiveScissor, exclusiveScissorCount, pExclusiveScissors);
    }
}


VKAPI_ATTR void VKAPI_CALL CmdSetCheckpointNV(
    VkCommandBuffer                             commandBuffer,
    const void*                                 pCheckpointMarker) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetCheckpointNV]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetCheckpointNV(commandBuffer, pCheckpointMarker);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetCheckpointNV]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdSetCheckpointNV(commandBuffer, pCheckpointMarker);
    }
    DispatchCmdSetCheckpointNV(commandBuffer, pCheckpointMarker);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetCheckpointNV]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdSetCheckpointNV(commandBuffer, pCheckpointMarker);
    }
}

VKAPI_ATTR void VKAPI_CALL GetQueueCheckpointDataNV(
    VkQueue                                     queue,
    uint32_t*                                   pCheckpointDataCount,
    VkCheckpointDataNV*                         pCheckpointData) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(queue), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetQueueCheckpointDataNV]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetQueueCheckpointDataNV(queue, pCheckpointDataCount, pCheckpointData);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetQueueCheckpointDataNV]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetQueueCheckpointDataNV(queue, pCheckpointDataCount, pCheckpointData);
    }
    DispatchGetQueueCheckpointDataNV(queue, pCheckpointDataCount, pCheckpointData);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetQueueCheckpointDataNV]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetQueueCheckpointDataNV(queue, pCheckpointDataCount, pCheckpointData);
    }
}



VKAPI_ATTR VkResult VKAPI_CALL InitializePerformanceApiINTEL(
    VkDevice                                    device,
    const VkInitializePerformanceApiInfoINTEL*  pInitializeInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateInitializePerformanceApiINTEL]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateInitializePerformanceApiINTEL(device, pInitializeInfo);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordInitializePerformanceApiINTEL]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordInitializePerformanceApiINTEL(device, pInitializeInfo);
    }
    VkResult result = DispatchInitializePerformanceApiINTEL(device, pInitializeInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordInitializePerformanceApiINTEL]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordInitializePerformanceApiINTEL(device, pInitializeInfo, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL UninitializePerformanceApiINTEL(
    VkDevice                                    device) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateUninitializePerformanceApiINTEL]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateUninitializePerformanceApiINTEL(device);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordUninitializePerformanceApiINTEL]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordUninitializePerformanceApiINTEL(device);
    }
    DispatchUninitializePerformanceApiINTEL(device);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordUninitializePerformanceApiINTEL]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordUninitializePerformanceApiINTEL(device);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL CmdSetPerformanceMarkerINTEL(
    VkCommandBuffer                             commandBuffer,
    const VkPerformanceMarkerInfoINTEL*         pMarkerInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetPerformanceMarkerINTEL]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetPerformanceMarkerINTEL(commandBuffer, pMarkerInfo);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetPerformanceMarkerINTEL]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdSetPerformanceMarkerINTEL(commandBuffer, pMarkerInfo);
    }
    VkResult result = DispatchCmdSetPerformanceMarkerINTEL(commandBuffer, pMarkerInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetPerformanceMarkerINTEL]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdSetPerformanceMarkerINTEL(commandBuffer, pMarkerInfo, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL CmdSetPerformanceStreamMarkerINTEL(
    VkCommandBuffer                             commandBuffer,
    const VkPerformanceStreamMarkerInfoINTEL*   pMarkerInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetPerformanceStreamMarkerINTEL]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetPerformanceStreamMarkerINTEL(commandBuffer, pMarkerInfo);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetPerformanceStreamMarkerINTEL]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdSetPerformanceStreamMarkerINTEL(commandBuffer, pMarkerInfo);
    }
    VkResult result = DispatchCmdSetPerformanceStreamMarkerINTEL(commandBuffer, pMarkerInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetPerformanceStreamMarkerINTEL]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdSetPerformanceStreamMarkerINTEL(commandBuffer, pMarkerInfo, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL CmdSetPerformanceOverrideINTEL(
    VkCommandBuffer                             commandBuffer,
    const VkPerformanceOverrideInfoINTEL*       pOverrideInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetPerformanceOverrideINTEL]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetPerformanceOverrideINTEL(commandBuffer, pOverrideInfo);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetPerformanceOverrideINTEL]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdSetPerformanceOverrideINTEL(commandBuffer, pOverrideInfo);
    }
    VkResult result = DispatchCmdSetPerformanceOverrideINTEL(commandBuffer, pOverrideInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetPerformanceOverrideINTEL]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdSetPerformanceOverrideINTEL(commandBuffer, pOverrideInfo, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL AcquirePerformanceConfigurationINTEL(
    VkDevice                                    device,
    const VkPerformanceConfigurationAcquireInfoINTEL* pAcquireInfo,
    VkPerformanceConfigurationINTEL*            pConfiguration) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateAcquirePerformanceConfigurationINTEL]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateAcquirePerformanceConfigurationINTEL(device, pAcquireInfo, pConfiguration);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordAcquirePerformanceConfigurationINTEL]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordAcquirePerformanceConfigurationINTEL(device, pAcquireInfo, pConfiguration);
    }
    VkResult result = DispatchAcquirePerformanceConfigurationINTEL(device, pAcquireInfo, pConfiguration);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordAcquirePerformanceConfigurationINTEL]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordAcquirePerformanceConfigurationINTEL(device, pAcquireInfo, pConfiguration, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL ReleasePerformanceConfigurationINTEL(
    VkDevice                                    device,
    VkPerformanceConfigurationINTEL             configuration) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateReleasePerformanceConfigurationINTEL]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateReleasePerformanceConfigurationINTEL(device, configuration);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordReleasePerformanceConfigurationINTEL]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordReleasePerformanceConfigurationINTEL(device, configuration);
    }
    VkResult result = DispatchReleasePerformanceConfigurationINTEL(device, configuration);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordReleasePerformanceConfigurationINTEL]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordReleasePerformanceConfigurationINTEL(device, configuration, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL QueueSetPerformanceConfigurationINTEL(
    VkQueue                                     queue,
    VkPerformanceConfigurationINTEL             configuration) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(queue), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateQueueSetPerformanceConfigurationINTEL]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateQueueSetPerformanceConfigurationINTEL(queue, configuration);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordQueueSetPerformanceConfigurationINTEL]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordQueueSetPerformanceConfigurationINTEL(queue, configuration);
    }
    VkResult result = DispatchQueueSetPerformanceConfigurationINTEL(queue, configuration);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordQueueSetPerformanceConfigurationINTEL]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordQueueSetPerformanceConfigurationINTEL(queue, configuration, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetPerformanceParameterINTEL(
    VkDevice                                    device,
    VkPerformanceParameterTypeINTEL             parameter,
    VkPerformanceValueINTEL*                    pValue) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetPerformanceParameterINTEL]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPerformanceParameterINTEL(device, parameter, pValue);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetPerformanceParameterINTEL]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetPerformanceParameterINTEL(device, parameter, pValue);
    }
    VkResult result = DispatchGetPerformanceParameterINTEL(device, parameter, pValue);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetPerformanceParameterINTEL]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetPerformanceParameterINTEL(device, parameter, pValue, result);
    }
    return result;
}



VKAPI_ATTR void VKAPI_CALL SetLocalDimmingAMD(
    VkDevice                                    device,
    VkSwapchainKHR                              swapChain,
    VkBool32                                    localDimmingEnable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateSetLocalDimmingAMD]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateSetLocalDimmingAMD(device, swapChain, localDimmingEnable);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordSetLocalDimmingAMD]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordSetLocalDimmingAMD(device, swapChain, localDimmingEnable);
    }
    DispatchSetLocalDimmingAMD(device, swapChain, localDimmingEnable);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordSetLocalDimmingAMD]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordSetLocalDimmingAMD(device, swapChain, localDimmingEnable);
    }
}

#ifdef VK_USE_PLATFORM_FUCHSIA

VKAPI_ATTR VkResult VKAPI_CALL CreateImagePipeSurfaceFUCHSIA(
    VkInstance                                  instance,
    const VkImagePipeSurfaceCreateInfoFUCHSIA*  pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateImagePipeSurfaceFUCHSIA(instance, pCreateInfo, pAllocator, pSurface);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCreateImagePipeSurfaceFUCHSIA(instance, pCreateInfo, pAllocator, pSurface);
    }
    VkResult result = DispatchCreateImagePipeSurfaceFUCHSIA(instance, pCreateInfo, pAllocator, pSurface);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCreateImagePipeSurfaceFUCHSIA(instance, pCreateInfo, pAllocator, pSurface, result);
    }
    return result;
}
#endif // VK_USE_PLATFORM_FUCHSIA

#ifdef VK_USE_PLATFORM_METAL_EXT

VKAPI_ATTR VkResult VKAPI_CALL CreateMetalSurfaceEXT(
    VkInstance                                  instance,
    const VkMetalSurfaceCreateInfoEXT*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateMetalSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCreateMetalSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface);
    }
    VkResult result = DispatchCreateMetalSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCreateMetalSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface, result);
    }
    return result;
}
#endif // VK_USE_PLATFORM_METAL_EXT













VKAPI_ATTR VkDeviceAddress VKAPI_CALL GetBufferDeviceAddressEXT(
    VkDevice                                    device,
    const VkBufferDeviceAddressInfo*            pInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetBufferDeviceAddressEXT]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetBufferDeviceAddressEXT(device, pInfo);
        if (skip) return 0;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetBufferDeviceAddressEXT]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetBufferDeviceAddressEXT(device, pInfo);
    }
    VkDeviceAddress result = DispatchGetBufferDeviceAddressEXT(device, pInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetBufferDeviceAddressEXT]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetBufferDeviceAddressEXT(device, pInfo, result);
    }
    return result;
}





VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceCooperativeMatrixPropertiesNV(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkCooperativeMatrixPropertiesNV*            pProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceCooperativeMatrixPropertiesNV(physicalDevice, pPropertyCount, pProperties);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetPhysicalDeviceCooperativeMatrixPropertiesNV(physicalDevice, pPropertyCount, pProperties);
    }
    VkResult result = DispatchGetPhysicalDeviceCooperativeMatrixPropertiesNV(physicalDevice, pPropertyCount, pProperties);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetPhysicalDeviceCooperativeMatrixPropertiesNV(physicalDevice, pPropertyCount, pProperties, result);
    }
    return result;
}


VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pCombinationCount,
    VkFramebufferMixedSamplesCombinationNV*     pCombinations) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(physicalDevice, pCombinationCount, pCombinations);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(physicalDevice, pCombinationCount, pCombinations);
    }
    VkResult result = DispatchGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(physicalDevice, pCombinationCount, pCombinations);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(physicalDevice, pCombinationCount, pCombinations, result);
    }
    return result;
}



#ifdef VK_USE_PLATFORM_WIN32_KHR

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceSurfacePresentModes2EXT(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSurfaceInfo2KHR*      pSurfaceInfo,
    uint32_t*                                   pPresentModeCount,
    VkPresentModeKHR*                           pPresentModes) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceSurfacePresentModes2EXT(physicalDevice, pSurfaceInfo, pPresentModeCount, pPresentModes);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetPhysicalDeviceSurfacePresentModes2EXT(physicalDevice, pSurfaceInfo, pPresentModeCount, pPresentModes);
    }
    VkResult result = DispatchGetPhysicalDeviceSurfacePresentModes2EXT(physicalDevice, pSurfaceInfo, pPresentModeCount, pPresentModes);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetPhysicalDeviceSurfacePresentModes2EXT(physicalDevice, pSurfaceInfo, pPresentModeCount, pPresentModes, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL AcquireFullScreenExclusiveModeEXT(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateAcquireFullScreenExclusiveModeEXT]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateAcquireFullScreenExclusiveModeEXT(device, swapchain);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordAcquireFullScreenExclusiveModeEXT]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordAcquireFullScreenExclusiveModeEXT(device, swapchain);
    }
    VkResult result = DispatchAcquireFullScreenExclusiveModeEXT(device, swapchain);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordAcquireFullScreenExclusiveModeEXT]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordAcquireFullScreenExclusiveModeEXT(device, swapchain, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL ReleaseFullScreenExclusiveModeEXT(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateReleaseFullScreenExclusiveModeEXT]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateReleaseFullScreenExclusiveModeEXT(device, swapchain);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordReleaseFullScreenExclusiveModeEXT]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordReleaseFullScreenExclusiveModeEXT(device, swapchain);
    }
    VkResult result = DispatchReleaseFullScreenExclusiveModeEXT(device, swapchain);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordReleaseFullScreenExclusiveModeEXT]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordReleaseFullScreenExclusiveModeEXT(device, swapchain, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetDeviceGroupSurfacePresentModes2EXT(
    VkDevice                                    device,
    const VkPhysicalDeviceSurfaceInfo2KHR*      pSurfaceInfo,
    VkDeviceGroupPresentModeFlagsKHR*           pModes) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetDeviceGroupSurfacePresentModes2EXT]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetDeviceGroupSurfacePresentModes2EXT(device, pSurfaceInfo, pModes);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetDeviceGroupSurfacePresentModes2EXT]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetDeviceGroupSurfacePresentModes2EXT(device, pSurfaceInfo, pModes);
    }
    VkResult result = DispatchGetDeviceGroupSurfacePresentModes2EXT(device, pSurfaceInfo, pModes);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetDeviceGroupSurfacePresentModes2EXT]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetDeviceGroupSurfacePresentModes2EXT(device, pSurfaceInfo, pModes, result);
    }
    return result;
}
#endif // VK_USE_PLATFORM_WIN32_KHR


VKAPI_ATTR VkResult VKAPI_CALL CreateHeadlessSurfaceEXT(
    VkInstance                                  instance,
    const VkHeadlessSurfaceCreateInfoEXT*       pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateHeadlessSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCreateHeadlessSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface);
    }
    VkResult result = DispatchCreateHeadlessSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetLineStippleEXT(commandBuffer, lineStippleFactor, lineStipplePattern);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetLineStippleEXT]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdSetLineStippleEXT(commandBuffer, lineStippleFactor, lineStipplePattern);
    }
    DispatchCmdSetLineStippleEXT(commandBuffer, lineStippleFactor, lineStipplePattern);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetLineStippleEXT]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdSetLineStippleEXT(commandBuffer, lineStippleFactor, lineStipplePattern);
    }
}



VKAPI_ATTR void VKAPI_CALL ResetQueryPoolEXT(
    VkDevice                                    device,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateResetQueryPoolEXT]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateResetQueryPoolEXT(device, queryPool, firstQuery, queryCount);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordResetQueryPoolEXT]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordResetQueryPoolEXT(device, queryPool, firstQuery, queryCount);
    }
    DispatchResetQueryPoolEXT(device, queryPool, firstQuery, queryCount);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordResetQueryPoolEXT]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordResetQueryPoolEXT(device, queryPool, firstQuery, queryCount);
    }
}



VKAPI_ATTR void VKAPI_CALL CmdSetCullModeEXT(
    VkCommandBuffer                             commandBuffer,
    VkCullModeFlags                             cullMode) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetCullModeEXT]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetCullModeEXT(commandBuffer, cullMode);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetCullModeEXT]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdSetCullModeEXT(commandBuffer, cullMode);
    }
    DispatchCmdSetCullModeEXT(commandBuffer, cullMode);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetCullModeEXT]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdSetCullModeEXT(commandBuffer, cullMode);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetFrontFaceEXT(
    VkCommandBuffer                             commandBuffer,
    VkFrontFace                                 frontFace) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetFrontFaceEXT]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetFrontFaceEXT(commandBuffer, frontFace);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetFrontFaceEXT]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdSetFrontFaceEXT(commandBuffer, frontFace);
    }
    DispatchCmdSetFrontFaceEXT(commandBuffer, frontFace);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetFrontFaceEXT]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdSetFrontFaceEXT(commandBuffer, frontFace);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetPrimitiveTopologyEXT(
    VkCommandBuffer                             commandBuffer,
    VkPrimitiveTopology                         primitiveTopology) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetPrimitiveTopologyEXT]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetPrimitiveTopologyEXT(commandBuffer, primitiveTopology);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetPrimitiveTopologyEXT]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdSetPrimitiveTopologyEXT(commandBuffer, primitiveTopology);
    }
    DispatchCmdSetPrimitiveTopologyEXT(commandBuffer, primitiveTopology);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetPrimitiveTopologyEXT]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetViewportWithCountEXT(commandBuffer, viewportCount, pViewports);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetViewportWithCountEXT]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdSetViewportWithCountEXT(commandBuffer, viewportCount, pViewports);
    }
    DispatchCmdSetViewportWithCountEXT(commandBuffer, viewportCount, pViewports);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetViewportWithCountEXT]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetScissorWithCountEXT(commandBuffer, scissorCount, pScissors);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetScissorWithCountEXT]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdSetScissorWithCountEXT(commandBuffer, scissorCount, pScissors);
    }
    DispatchCmdSetScissorWithCountEXT(commandBuffer, scissorCount, pScissors);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetScissorWithCountEXT]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdBindVertexBuffers2EXT(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes, pStrides);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdBindVertexBuffers2EXT]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdBindVertexBuffers2EXT(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes, pStrides);
    }
    DispatchCmdBindVertexBuffers2EXT(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes, pStrides);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdBindVertexBuffers2EXT]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdBindVertexBuffers2EXT(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes, pStrides);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetDepthTestEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    depthTestEnable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetDepthTestEnableEXT]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetDepthTestEnableEXT(commandBuffer, depthTestEnable);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetDepthTestEnableEXT]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdSetDepthTestEnableEXT(commandBuffer, depthTestEnable);
    }
    DispatchCmdSetDepthTestEnableEXT(commandBuffer, depthTestEnable);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetDepthTestEnableEXT]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdSetDepthTestEnableEXT(commandBuffer, depthTestEnable);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetDepthWriteEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    depthWriteEnable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetDepthWriteEnableEXT]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetDepthWriteEnableEXT(commandBuffer, depthWriteEnable);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetDepthWriteEnableEXT]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdSetDepthWriteEnableEXT(commandBuffer, depthWriteEnable);
    }
    DispatchCmdSetDepthWriteEnableEXT(commandBuffer, depthWriteEnable);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetDepthWriteEnableEXT]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdSetDepthWriteEnableEXT(commandBuffer, depthWriteEnable);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetDepthCompareOpEXT(
    VkCommandBuffer                             commandBuffer,
    VkCompareOp                                 depthCompareOp) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetDepthCompareOpEXT]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetDepthCompareOpEXT(commandBuffer, depthCompareOp);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetDepthCompareOpEXT]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdSetDepthCompareOpEXT(commandBuffer, depthCompareOp);
    }
    DispatchCmdSetDepthCompareOpEXT(commandBuffer, depthCompareOp);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetDepthCompareOpEXT]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdSetDepthCompareOpEXT(commandBuffer, depthCompareOp);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetDepthBoundsTestEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    depthBoundsTestEnable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetDepthBoundsTestEnableEXT]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetDepthBoundsTestEnableEXT(commandBuffer, depthBoundsTestEnable);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetDepthBoundsTestEnableEXT]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdSetDepthBoundsTestEnableEXT(commandBuffer, depthBoundsTestEnable);
    }
    DispatchCmdSetDepthBoundsTestEnableEXT(commandBuffer, depthBoundsTestEnable);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetDepthBoundsTestEnableEXT]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdSetDepthBoundsTestEnableEXT(commandBuffer, depthBoundsTestEnable);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetStencilTestEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    stencilTestEnable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetStencilTestEnableEXT]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetStencilTestEnableEXT(commandBuffer, stencilTestEnable);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetStencilTestEnableEXT]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdSetStencilTestEnableEXT(commandBuffer, stencilTestEnable);
    }
    DispatchCmdSetStencilTestEnableEXT(commandBuffer, stencilTestEnable);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetStencilTestEnableEXT]) {
        auto lock = intercept->write_lock();
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
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetStencilOpEXT(commandBuffer, faceMask, failOp, passOp, depthFailOp, compareOp);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetStencilOpEXT]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdSetStencilOpEXT(commandBuffer, faceMask, failOp, passOp, depthFailOp, compareOp);
    }
    DispatchCmdSetStencilOpEXT(commandBuffer, faceMask, failOp, passOp, depthFailOp, compareOp);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetStencilOpEXT]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdSetStencilOpEXT(commandBuffer, faceMask, failOp, passOp, depthFailOp, compareOp);
    }
}



VKAPI_ATTR void VKAPI_CALL GetGeneratedCommandsMemoryRequirementsNV(
    VkDevice                                    device,
    const VkGeneratedCommandsMemoryRequirementsInfoNV* pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetGeneratedCommandsMemoryRequirementsNV]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetGeneratedCommandsMemoryRequirementsNV(device, pInfo, pMemoryRequirements);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetGeneratedCommandsMemoryRequirementsNV]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetGeneratedCommandsMemoryRequirementsNV(device, pInfo, pMemoryRequirements);
    }
    DispatchGetGeneratedCommandsMemoryRequirementsNV(device, pInfo, pMemoryRequirements);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetGeneratedCommandsMemoryRequirementsNV]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetGeneratedCommandsMemoryRequirementsNV(device, pInfo, pMemoryRequirements);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdPreprocessGeneratedCommandsNV(
    VkCommandBuffer                             commandBuffer,
    const VkGeneratedCommandsInfoNV*            pGeneratedCommandsInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdPreprocessGeneratedCommandsNV]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdPreprocessGeneratedCommandsNV(commandBuffer, pGeneratedCommandsInfo);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdPreprocessGeneratedCommandsNV]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdPreprocessGeneratedCommandsNV(commandBuffer, pGeneratedCommandsInfo);
    }
    DispatchCmdPreprocessGeneratedCommandsNV(commandBuffer, pGeneratedCommandsInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdPreprocessGeneratedCommandsNV]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdPreprocessGeneratedCommandsNV(commandBuffer, pGeneratedCommandsInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdExecuteGeneratedCommandsNV(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    isPreprocessed,
    const VkGeneratedCommandsInfoNV*            pGeneratedCommandsInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdExecuteGeneratedCommandsNV]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdExecuteGeneratedCommandsNV(commandBuffer, isPreprocessed, pGeneratedCommandsInfo);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdExecuteGeneratedCommandsNV]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdExecuteGeneratedCommandsNV(commandBuffer, isPreprocessed, pGeneratedCommandsInfo);
    }
    DispatchCmdExecuteGeneratedCommandsNV(commandBuffer, isPreprocessed, pGeneratedCommandsInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdExecuteGeneratedCommandsNV]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdExecuteGeneratedCommandsNV(commandBuffer, isPreprocessed, pGeneratedCommandsInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdBindPipelineShaderGroupNV(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipeline                                  pipeline,
    uint32_t                                    groupIndex) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdBindPipelineShaderGroupNV]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdBindPipelineShaderGroupNV(commandBuffer, pipelineBindPoint, pipeline, groupIndex);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdBindPipelineShaderGroupNV]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdBindPipelineShaderGroupNV(commandBuffer, pipelineBindPoint, pipeline, groupIndex);
    }
    DispatchCmdBindPipelineShaderGroupNV(commandBuffer, pipelineBindPoint, pipeline, groupIndex);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdBindPipelineShaderGroupNV]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdBindPipelineShaderGroupNV(commandBuffer, pipelineBindPoint, pipeline, groupIndex);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL CreateIndirectCommandsLayoutNV(
    VkDevice                                    device,
    const VkIndirectCommandsLayoutCreateInfoNV* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkIndirectCommandsLayoutNV*                 pIndirectCommandsLayout) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCreateIndirectCommandsLayoutNV]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateIndirectCommandsLayoutNV(device, pCreateInfo, pAllocator, pIndirectCommandsLayout);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateIndirectCommandsLayoutNV]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCreateIndirectCommandsLayoutNV(device, pCreateInfo, pAllocator, pIndirectCommandsLayout);
    }
    VkResult result = DispatchCreateIndirectCommandsLayoutNV(device, pCreateInfo, pAllocator, pIndirectCommandsLayout);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateIndirectCommandsLayoutNV]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCreateIndirectCommandsLayoutNV(device, pCreateInfo, pAllocator, pIndirectCommandsLayout, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyIndirectCommandsLayoutNV(
    VkDevice                                    device,
    VkIndirectCommandsLayoutNV                  indirectCommandsLayout,
    const VkAllocationCallbacks*                pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDestroyIndirectCommandsLayoutNV]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateDestroyIndirectCommandsLayoutNV(device, indirectCommandsLayout, pAllocator);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroyIndirectCommandsLayoutNV]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordDestroyIndirectCommandsLayoutNV(device, indirectCommandsLayout, pAllocator);
    }
    DispatchDestroyIndirectCommandsLayoutNV(device, indirectCommandsLayout, pAllocator);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroyIndirectCommandsLayoutNV]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordDestroyIndirectCommandsLayoutNV(device, indirectCommandsLayout, pAllocator);
    }
}








VKAPI_ATTR VkResult VKAPI_CALL CreatePrivateDataSlotEXT(
    VkDevice                                    device,
    const VkPrivateDataSlotCreateInfoEXT*       pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkPrivateDataSlotEXT*                       pPrivateDataSlot) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCreatePrivateDataSlotEXT]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreatePrivateDataSlotEXT(device, pCreateInfo, pAllocator, pPrivateDataSlot);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreatePrivateDataSlotEXT]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCreatePrivateDataSlotEXT(device, pCreateInfo, pAllocator, pPrivateDataSlot);
    }
    VkResult result = DispatchCreatePrivateDataSlotEXT(device, pCreateInfo, pAllocator, pPrivateDataSlot);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreatePrivateDataSlotEXT]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCreatePrivateDataSlotEXT(device, pCreateInfo, pAllocator, pPrivateDataSlot, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyPrivateDataSlotEXT(
    VkDevice                                    device,
    VkPrivateDataSlotEXT                        privateDataSlot,
    const VkAllocationCallbacks*                pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDestroyPrivateDataSlotEXT]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateDestroyPrivateDataSlotEXT(device, privateDataSlot, pAllocator);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroyPrivateDataSlotEXT]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordDestroyPrivateDataSlotEXT(device, privateDataSlot, pAllocator);
    }
    DispatchDestroyPrivateDataSlotEXT(device, privateDataSlot, pAllocator);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroyPrivateDataSlotEXT]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordDestroyPrivateDataSlotEXT(device, privateDataSlot, pAllocator);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL SetPrivateDataEXT(
    VkDevice                                    device,
    VkObjectType                                objectType,
    uint64_t                                    objectHandle,
    VkPrivateDataSlotEXT                        privateDataSlot,
    uint64_t                                    data) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateSetPrivateDataEXT]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateSetPrivateDataEXT(device, objectType, objectHandle, privateDataSlot, data);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordSetPrivateDataEXT]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordSetPrivateDataEXT(device, objectType, objectHandle, privateDataSlot, data);
    }
    VkResult result = DispatchSetPrivateDataEXT(device, objectType, objectHandle, privateDataSlot, data);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordSetPrivateDataEXT]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordSetPrivateDataEXT(device, objectType, objectHandle, privateDataSlot, data, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL GetPrivateDataEXT(
    VkDevice                                    device,
    VkObjectType                                objectType,
    uint64_t                                    objectHandle,
    VkPrivateDataSlotEXT                        privateDataSlot,
    uint64_t*                                   pData) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetPrivateDataEXT]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPrivateDataEXT(device, objectType, objectHandle, privateDataSlot, pData);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetPrivateDataEXT]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetPrivateDataEXT(device, objectType, objectHandle, privateDataSlot, pData);
    }
    DispatchGetPrivateDataEXT(device, objectType, objectHandle, privateDataSlot, pData);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetPrivateDataEXT]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetPrivateDataEXT(device, objectType, objectHandle, privateDataSlot, pData);
    }
}





VKAPI_ATTR void VKAPI_CALL CmdSetFragmentShadingRateEnumNV(
    VkCommandBuffer                             commandBuffer,
    VkFragmentShadingRateNV                     shadingRate,
    const VkFragmentShadingRateCombinerOpKHR    combinerOps[2]) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetFragmentShadingRateEnumNV]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetFragmentShadingRateEnumNV(commandBuffer, shadingRate, combinerOps);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetFragmentShadingRateEnumNV]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdSetFragmentShadingRateEnumNV(commandBuffer, shadingRate, combinerOps);
    }
    DispatchCmdSetFragmentShadingRateEnumNV(commandBuffer, shadingRate, combinerOps);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetFragmentShadingRateEnumNV]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdSetFragmentShadingRateEnumNV(commandBuffer, shadingRate, combinerOps);
    }
}





#ifdef VK_USE_PLATFORM_WIN32_KHR

VKAPI_ATTR VkResult VKAPI_CALL AcquireWinrtDisplayNV(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateAcquireWinrtDisplayNV(physicalDevice, display);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordAcquireWinrtDisplayNV(physicalDevice, display);
    }
    VkResult result = DispatchAcquireWinrtDisplayNV(physicalDevice, display);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordAcquireWinrtDisplayNV(physicalDevice, display, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetWinrtDisplayNV(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    deviceRelativeId,
    VkDisplayKHR*                               pDisplay) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetWinrtDisplayNV(physicalDevice, deviceRelativeId, pDisplay);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetWinrtDisplayNV(physicalDevice, deviceRelativeId, pDisplay);
    }
    VkResult result = DispatchGetWinrtDisplayNV(physicalDevice, deviceRelativeId, pDisplay);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetWinrtDisplayNV(physicalDevice, deviceRelativeId, pDisplay, result);
    }
    return result;
}
#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_DIRECTFB_EXT

VKAPI_ATTR VkResult VKAPI_CALL CreateDirectFBSurfaceEXT(
    VkInstance                                  instance,
    const VkDirectFBSurfaceCreateInfoEXT*       pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateDirectFBSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCreateDirectFBSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface);
    }
    VkResult result = DispatchCreateDirectFBSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCreateDirectFBSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface, result);
    }
    return result;
}

VKAPI_ATTR VkBool32 VKAPI_CALL GetPhysicalDeviceDirectFBPresentationSupportEXT(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    IDirectFB*                                  dfb) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetPhysicalDeviceDirectFBPresentationSupportEXT(physicalDevice, queueFamilyIndex, dfb);
        if (skip) return VK_FALSE;
    }
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetPhysicalDeviceDirectFBPresentationSupportEXT(physicalDevice, queueFamilyIndex, dfb);
    }
    VkBool32 result = DispatchGetPhysicalDeviceDirectFBPresentationSupportEXT(physicalDevice, queueFamilyIndex, dfb);
    for (auto intercept : layer_data->object_dispatch) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetPhysicalDeviceDirectFBPresentationSupportEXT(physicalDevice, queueFamilyIndex, dfb);
    }
    return result;
}
#endif // VK_USE_PLATFORM_DIRECTFB_EXT



VKAPI_ATTR VkResult VKAPI_CALL CreateAccelerationStructureKHR(
    VkDevice                                    device,
    const VkAccelerationStructureCreateInfoKHR* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkAccelerationStructureKHR*                 pAccelerationStructure) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCreateAccelerationStructureKHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCreateAccelerationStructureKHR(device, pCreateInfo, pAllocator, pAccelerationStructure);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateAccelerationStructureKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCreateAccelerationStructureKHR(device, pCreateInfo, pAllocator, pAccelerationStructure);
    }
    VkResult result = DispatchCreateAccelerationStructureKHR(device, pCreateInfo, pAllocator, pAccelerationStructure);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateAccelerationStructureKHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCreateAccelerationStructureKHR(device, pCreateInfo, pAllocator, pAccelerationStructure, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyAccelerationStructureKHR(
    VkDevice                                    device,
    VkAccelerationStructureKHR                  accelerationStructure,
    const VkAllocationCallbacks*                pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDestroyAccelerationStructureKHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateDestroyAccelerationStructureKHR(device, accelerationStructure, pAllocator);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroyAccelerationStructureKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordDestroyAccelerationStructureKHR(device, accelerationStructure, pAllocator);
    }
    DispatchDestroyAccelerationStructureKHR(device, accelerationStructure, pAllocator);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroyAccelerationStructureKHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordDestroyAccelerationStructureKHR(device, accelerationStructure, pAllocator);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdBuildAccelerationStructuresKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    infoCount,
    const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdBuildAccelerationStructuresKHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdBuildAccelerationStructuresKHR(commandBuffer, infoCount, pInfos, ppBuildRangeInfos);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdBuildAccelerationStructuresKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdBuildAccelerationStructuresKHR(commandBuffer, infoCount, pInfos, ppBuildRangeInfos);
    }
    DispatchCmdBuildAccelerationStructuresKHR(commandBuffer, infoCount, pInfos, ppBuildRangeInfos);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdBuildAccelerationStructuresKHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdBuildAccelerationStructuresKHR(commandBuffer, infoCount, pInfos, ppBuildRangeInfos);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdBuildAccelerationStructuresIndirectKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    infoCount,
    const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkDeviceAddress*                      pIndirectDeviceAddresses,
    const uint32_t*                             pIndirectStrides,
    const uint32_t* const*                      ppMaxPrimitiveCounts) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdBuildAccelerationStructuresIndirectKHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdBuildAccelerationStructuresIndirectKHR(commandBuffer, infoCount, pInfos, pIndirectDeviceAddresses, pIndirectStrides, ppMaxPrimitiveCounts);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdBuildAccelerationStructuresIndirectKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdBuildAccelerationStructuresIndirectKHR(commandBuffer, infoCount, pInfos, pIndirectDeviceAddresses, pIndirectStrides, ppMaxPrimitiveCounts);
    }
    DispatchCmdBuildAccelerationStructuresIndirectKHR(commandBuffer, infoCount, pInfos, pIndirectDeviceAddresses, pIndirectStrides, ppMaxPrimitiveCounts);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdBuildAccelerationStructuresIndirectKHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdBuildAccelerationStructuresIndirectKHR(commandBuffer, infoCount, pInfos, pIndirectDeviceAddresses, pIndirectStrides, ppMaxPrimitiveCounts);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL BuildAccelerationStructuresKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    uint32_t                                    infoCount,
    const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateBuildAccelerationStructuresKHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateBuildAccelerationStructuresKHR(device, deferredOperation, infoCount, pInfos, ppBuildRangeInfos);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordBuildAccelerationStructuresKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordBuildAccelerationStructuresKHR(device, deferredOperation, infoCount, pInfos, ppBuildRangeInfos);
    }
    VkResult result = DispatchBuildAccelerationStructuresKHR(device, deferredOperation, infoCount, pInfos, ppBuildRangeInfos);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordBuildAccelerationStructuresKHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordBuildAccelerationStructuresKHR(device, deferredOperation, infoCount, pInfos, ppBuildRangeInfos, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL CopyAccelerationStructureKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    const VkCopyAccelerationStructureInfoKHR*   pInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCopyAccelerationStructureKHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCopyAccelerationStructureKHR(device, deferredOperation, pInfo);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCopyAccelerationStructureKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCopyAccelerationStructureKHR(device, deferredOperation, pInfo);
    }
    VkResult result = DispatchCopyAccelerationStructureKHR(device, deferredOperation, pInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCopyAccelerationStructureKHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCopyAccelerationStructureKHR(device, deferredOperation, pInfo, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL CopyAccelerationStructureToMemoryKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCopyAccelerationStructureToMemoryKHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCopyAccelerationStructureToMemoryKHR(device, deferredOperation, pInfo);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCopyAccelerationStructureToMemoryKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCopyAccelerationStructureToMemoryKHR(device, deferredOperation, pInfo);
    }
    VkResult result = DispatchCopyAccelerationStructureToMemoryKHR(device, deferredOperation, pInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCopyAccelerationStructureToMemoryKHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCopyAccelerationStructureToMemoryKHR(device, deferredOperation, pInfo, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL CopyMemoryToAccelerationStructureKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCopyMemoryToAccelerationStructureKHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCopyMemoryToAccelerationStructureKHR(device, deferredOperation, pInfo);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCopyMemoryToAccelerationStructureKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCopyMemoryToAccelerationStructureKHR(device, deferredOperation, pInfo);
    }
    VkResult result = DispatchCopyMemoryToAccelerationStructureKHR(device, deferredOperation, pInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCopyMemoryToAccelerationStructureKHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCopyMemoryToAccelerationStructureKHR(device, deferredOperation, pInfo, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL WriteAccelerationStructuresPropertiesKHR(
    VkDevice                                    device,
    uint32_t                                    accelerationStructureCount,
    const VkAccelerationStructureKHR*           pAccelerationStructures,
    VkQueryType                                 queryType,
    size_t                                      dataSize,
    void*                                       pData,
    size_t                                      stride) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateWriteAccelerationStructuresPropertiesKHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateWriteAccelerationStructuresPropertiesKHR(device, accelerationStructureCount, pAccelerationStructures, queryType, dataSize, pData, stride);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordWriteAccelerationStructuresPropertiesKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordWriteAccelerationStructuresPropertiesKHR(device, accelerationStructureCount, pAccelerationStructures, queryType, dataSize, pData, stride);
    }
    VkResult result = DispatchWriteAccelerationStructuresPropertiesKHR(device, accelerationStructureCount, pAccelerationStructures, queryType, dataSize, pData, stride);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordWriteAccelerationStructuresPropertiesKHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordWriteAccelerationStructuresPropertiesKHR(device, accelerationStructureCount, pAccelerationStructures, queryType, dataSize, pData, stride, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL CmdCopyAccelerationStructureKHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyAccelerationStructureInfoKHR*   pInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdCopyAccelerationStructureKHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdCopyAccelerationStructureKHR(commandBuffer, pInfo);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdCopyAccelerationStructureKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdCopyAccelerationStructureKHR(commandBuffer, pInfo);
    }
    DispatchCmdCopyAccelerationStructureKHR(commandBuffer, pInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdCopyAccelerationStructureKHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdCopyAccelerationStructureKHR(commandBuffer, pInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdCopyAccelerationStructureToMemoryKHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdCopyAccelerationStructureToMemoryKHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdCopyAccelerationStructureToMemoryKHR(commandBuffer, pInfo);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdCopyAccelerationStructureToMemoryKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdCopyAccelerationStructureToMemoryKHR(commandBuffer, pInfo);
    }
    DispatchCmdCopyAccelerationStructureToMemoryKHR(commandBuffer, pInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdCopyAccelerationStructureToMemoryKHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdCopyAccelerationStructureToMemoryKHR(commandBuffer, pInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdCopyMemoryToAccelerationStructureKHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdCopyMemoryToAccelerationStructureKHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdCopyMemoryToAccelerationStructureKHR(commandBuffer, pInfo);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdCopyMemoryToAccelerationStructureKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdCopyMemoryToAccelerationStructureKHR(commandBuffer, pInfo);
    }
    DispatchCmdCopyMemoryToAccelerationStructureKHR(commandBuffer, pInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdCopyMemoryToAccelerationStructureKHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdCopyMemoryToAccelerationStructureKHR(commandBuffer, pInfo);
    }
}

VKAPI_ATTR VkDeviceAddress VKAPI_CALL GetAccelerationStructureDeviceAddressKHR(
    VkDevice                                    device,
    const VkAccelerationStructureDeviceAddressInfoKHR* pInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetAccelerationStructureDeviceAddressKHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetAccelerationStructureDeviceAddressKHR(device, pInfo);
        if (skip) return 0;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetAccelerationStructureDeviceAddressKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetAccelerationStructureDeviceAddressKHR(device, pInfo);
    }
    VkDeviceAddress result = DispatchGetAccelerationStructureDeviceAddressKHR(device, pInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetAccelerationStructureDeviceAddressKHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetAccelerationStructureDeviceAddressKHR(device, pInfo, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL CmdWriteAccelerationStructuresPropertiesKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    accelerationStructureCount,
    const VkAccelerationStructureKHR*           pAccelerationStructures,
    VkQueryType                                 queryType,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdWriteAccelerationStructuresPropertiesKHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdWriteAccelerationStructuresPropertiesKHR(commandBuffer, accelerationStructureCount, pAccelerationStructures, queryType, queryPool, firstQuery);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdWriteAccelerationStructuresPropertiesKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdWriteAccelerationStructuresPropertiesKHR(commandBuffer, accelerationStructureCount, pAccelerationStructures, queryType, queryPool, firstQuery);
    }
    DispatchCmdWriteAccelerationStructuresPropertiesKHR(commandBuffer, accelerationStructureCount, pAccelerationStructures, queryType, queryPool, firstQuery);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdWriteAccelerationStructuresPropertiesKHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdWriteAccelerationStructuresPropertiesKHR(commandBuffer, accelerationStructureCount, pAccelerationStructures, queryType, queryPool, firstQuery);
    }
}

VKAPI_ATTR void VKAPI_CALL GetDeviceAccelerationStructureCompatibilityKHR(
    VkDevice                                    device,
    const VkAccelerationStructureVersionInfoKHR* pVersionInfo,
    VkAccelerationStructureCompatibilityKHR*    pCompatibility) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetDeviceAccelerationStructureCompatibilityKHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetDeviceAccelerationStructureCompatibilityKHR(device, pVersionInfo, pCompatibility);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetDeviceAccelerationStructureCompatibilityKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetDeviceAccelerationStructureCompatibilityKHR(device, pVersionInfo, pCompatibility);
    }
    DispatchGetDeviceAccelerationStructureCompatibilityKHR(device, pVersionInfo, pCompatibility);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetDeviceAccelerationStructureCompatibilityKHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetDeviceAccelerationStructureCompatibilityKHR(device, pVersionInfo, pCompatibility);
    }
}

VKAPI_ATTR void VKAPI_CALL GetAccelerationStructureBuildSizesKHR(
    VkDevice                                    device,
    VkAccelerationStructureBuildTypeKHR         buildType,
    const VkAccelerationStructureBuildGeometryInfoKHR* pBuildInfo,
    const uint32_t*                             pMaxPrimitiveCounts,
    VkAccelerationStructureBuildSizesInfoKHR*   pSizeInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetAccelerationStructureBuildSizesKHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetAccelerationStructureBuildSizesKHR(device, buildType, pBuildInfo, pMaxPrimitiveCounts, pSizeInfo);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetAccelerationStructureBuildSizesKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetAccelerationStructureBuildSizesKHR(device, buildType, pBuildInfo, pMaxPrimitiveCounts, pSizeInfo);
    }
    DispatchGetAccelerationStructureBuildSizesKHR(device, buildType, pBuildInfo, pMaxPrimitiveCounts, pSizeInfo);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetAccelerationStructureBuildSizesKHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetAccelerationStructureBuildSizesKHR(device, buildType, pBuildInfo, pMaxPrimitiveCounts, pSizeInfo);
    }
}


VKAPI_ATTR void VKAPI_CALL CmdTraceRaysKHR(
    VkCommandBuffer                             commandBuffer,
    const VkStridedDeviceAddressRegionKHR*      pRaygenShaderBindingTable,
    const VkStridedDeviceAddressRegionKHR*      pMissShaderBindingTable,
    const VkStridedDeviceAddressRegionKHR*      pHitShaderBindingTable,
    const VkStridedDeviceAddressRegionKHR*      pCallableShaderBindingTable,
    uint32_t                                    width,
    uint32_t                                    height,
    uint32_t                                    depth) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdTraceRaysKHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdTraceRaysKHR(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable, width, height, depth);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdTraceRaysKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdTraceRaysKHR(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable, width, height, depth);
    }
    DispatchCmdTraceRaysKHR(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable, width, height, depth);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdTraceRaysKHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdTraceRaysKHR(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable, width, height, depth);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL GetRayTracingCaptureReplayShaderGroupHandlesKHR(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    firstGroup,
    uint32_t                                    groupCount,
    size_t                                      dataSize,
    void*                                       pData) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetRayTracingCaptureReplayShaderGroupHandlesKHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetRayTracingCaptureReplayShaderGroupHandlesKHR(device, pipeline, firstGroup, groupCount, dataSize, pData);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetRayTracingCaptureReplayShaderGroupHandlesKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetRayTracingCaptureReplayShaderGroupHandlesKHR(device, pipeline, firstGroup, groupCount, dataSize, pData);
    }
    VkResult result = DispatchGetRayTracingCaptureReplayShaderGroupHandlesKHR(device, pipeline, firstGroup, groupCount, dataSize, pData);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetRayTracingCaptureReplayShaderGroupHandlesKHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetRayTracingCaptureReplayShaderGroupHandlesKHR(device, pipeline, firstGroup, groupCount, dataSize, pData, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL CmdTraceRaysIndirectKHR(
    VkCommandBuffer                             commandBuffer,
    const VkStridedDeviceAddressRegionKHR*      pRaygenShaderBindingTable,
    const VkStridedDeviceAddressRegionKHR*      pMissShaderBindingTable,
    const VkStridedDeviceAddressRegionKHR*      pHitShaderBindingTable,
    const VkStridedDeviceAddressRegionKHR*      pCallableShaderBindingTable,
    VkDeviceAddress                             indirectDeviceAddress) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdTraceRaysIndirectKHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdTraceRaysIndirectKHR(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable, indirectDeviceAddress);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdTraceRaysIndirectKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdTraceRaysIndirectKHR(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable, indirectDeviceAddress);
    }
    DispatchCmdTraceRaysIndirectKHR(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable, indirectDeviceAddress);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdTraceRaysIndirectKHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdTraceRaysIndirectKHR(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable, indirectDeviceAddress);
    }
}

VKAPI_ATTR VkDeviceSize VKAPI_CALL GetRayTracingShaderGroupStackSizeKHR(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    group,
    VkShaderGroupShaderKHR                      groupShader) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetRayTracingShaderGroupStackSizeKHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateGetRayTracingShaderGroupStackSizeKHR(device, pipeline, group, groupShader);
        if (skip) return 0;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetRayTracingShaderGroupStackSizeKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordGetRayTracingShaderGroupStackSizeKHR(device, pipeline, group, groupShader);
    }
    VkDeviceSize result = DispatchGetRayTracingShaderGroupStackSizeKHR(device, pipeline, group, groupShader);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetRayTracingShaderGroupStackSizeKHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordGetRayTracingShaderGroupStackSizeKHR(device, pipeline, group, groupShader);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL CmdSetRayTracingPipelineStackSizeKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    pipelineStackSize) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetRayTracingPipelineStackSizeKHR]) {
        auto lock = intercept->read_lock();
        skip |= (const_cast<const ValidationObject*>(intercept))->PreCallValidateCmdSetRayTracingPipelineStackSizeKHR(commandBuffer, pipelineStackSize);
        if (skip) return;
    }
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetRayTracingPipelineStackSizeKHR]) {
        auto lock = intercept->write_lock();
        intercept->PreCallRecordCmdSetRayTracingPipelineStackSizeKHR(commandBuffer, pipelineStackSize);
    }
    DispatchCmdSetRayTracingPipelineStackSizeKHR(commandBuffer, pipelineStackSize);
    for (auto intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetRayTracingPipelineStackSizeKHR]) {
        auto lock = intercept->write_lock();
        intercept->PostCallRecordCmdSetRayTracingPipelineStackSizeKHR(commandBuffer, pipelineStackSize);
    }
}


// Map of intercepted ApiName to its associated function data
#ifdef _MSC_VER
#pragma warning( suppress: 6262 ) // VS analysis: this uses more than 16 kiB, which is fine here at global scope
#endif
const std::unordered_map<std::string, function_data> name_to_funcptr_map = {
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
    {"vkFreeMemory", {kFuncTypeDev, (void*)FreeMemory}},
    {"vkMapMemory", {kFuncTypeDev, (void*)MapMemory}},
    {"vkUnmapMemory", {kFuncTypeDev, (void*)UnmapMemory}},
    {"vkFlushMappedMemoryRanges", {kFuncTypeDev, (void*)FlushMappedMemoryRanges}},
    {"vkInvalidateMappedMemoryRanges", {kFuncTypeDev, (void*)InvalidateMappedMemoryRanges}},
    {"vkGetDeviceMemoryCommitment", {kFuncTypeDev, (void*)GetDeviceMemoryCommitment}},
    {"vkBindBufferMemory", {kFuncTypeDev, (void*)BindBufferMemory}},
    {"vkBindImageMemory", {kFuncTypeDev, (void*)BindImageMemory}},
    {"vkGetBufferMemoryRequirements", {kFuncTypeDev, (void*)GetBufferMemoryRequirements}},
    {"vkGetImageMemoryRequirements", {kFuncTypeDev, (void*)GetImageMemoryRequirements}},
    {"vkGetImageSparseMemoryRequirements", {kFuncTypeDev, (void*)GetImageSparseMemoryRequirements}},
    {"vkGetPhysicalDeviceSparseImageFormatProperties", {kFuncTypePdev, (void*)GetPhysicalDeviceSparseImageFormatProperties}},
    {"vkQueueBindSparse", {kFuncTypeDev, (void*)QueueBindSparse}},
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
    {"vkDestroyQueryPool", {kFuncTypeDev, (void*)DestroyQueryPool}},
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
    {"vkCreateShaderModule", {kFuncTypeDev, (void*)CreateShaderModule}},
    {"vkDestroyShaderModule", {kFuncTypeDev, (void*)DestroyShaderModule}},
    {"vkCreatePipelineCache", {kFuncTypeDev, (void*)CreatePipelineCache}},
    {"vkDestroyPipelineCache", {kFuncTypeDev, (void*)DestroyPipelineCache}},
    {"vkGetPipelineCacheData", {kFuncTypeDev, (void*)GetPipelineCacheData}},
    {"vkMergePipelineCaches", {kFuncTypeDev, (void*)MergePipelineCaches}},
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
    {"vkDestroyDescriptorPool", {kFuncTypeDev, (void*)DestroyDescriptorPool}},
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
    {"vkDestroyCommandPool", {kFuncTypeDev, (void*)DestroyCommandPool}},
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
    {"vkGetImageSparseMemoryRequirements2", {kFuncTypeDev, (void*)GetImageSparseMemoryRequirements2}},
    {"vkGetPhysicalDeviceFeatures2", {kFuncTypePdev, (void*)GetPhysicalDeviceFeatures2}},
    {"vkGetPhysicalDeviceProperties2", {kFuncTypePdev, (void*)GetPhysicalDeviceProperties2}},
    {"vkGetPhysicalDeviceFormatProperties2", {kFuncTypePdev, (void*)GetPhysicalDeviceFormatProperties2}},
    {"vkGetPhysicalDeviceImageFormatProperties2", {kFuncTypePdev, (void*)GetPhysicalDeviceImageFormatProperties2}},
    {"vkGetPhysicalDeviceQueueFamilyProperties2", {kFuncTypePdev, (void*)GetPhysicalDeviceQueueFamilyProperties2}},
    {"vkGetPhysicalDeviceMemoryProperties2", {kFuncTypePdev, (void*)GetPhysicalDeviceMemoryProperties2}},
    {"vkGetPhysicalDeviceSparseImageFormatProperties2", {kFuncTypePdev, (void*)GetPhysicalDeviceSparseImageFormatProperties2}},
    {"vkTrimCommandPool", {kFuncTypeDev, (void*)TrimCommandPool}},
    {"vkGetDeviceQueue2", {kFuncTypeDev, (void*)GetDeviceQueue2}},
    {"vkCreateSamplerYcbcrConversion", {kFuncTypeDev, (void*)CreateSamplerYcbcrConversion}},
    {"vkDestroySamplerYcbcrConversion", {kFuncTypeDev, (void*)DestroySamplerYcbcrConversion}},
    {"vkCreateDescriptorUpdateTemplate", {kFuncTypeDev, (void*)CreateDescriptorUpdateTemplate}},
    {"vkDestroyDescriptorUpdateTemplate", {kFuncTypeDev, (void*)DestroyDescriptorUpdateTemplate}},
    {"vkUpdateDescriptorSetWithTemplate", {kFuncTypeDev, (void*)UpdateDescriptorSetWithTemplate}},
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
    {"vkDestroySurfaceKHR", {kFuncTypeInst, (void*)DestroySurfaceKHR}},
    {"vkGetPhysicalDeviceSurfaceSupportKHR", {kFuncTypePdev, (void*)GetPhysicalDeviceSurfaceSupportKHR}},
    {"vkGetPhysicalDeviceSurfaceCapabilitiesKHR", {kFuncTypePdev, (void*)GetPhysicalDeviceSurfaceCapabilitiesKHR}},
    {"vkGetPhysicalDeviceSurfaceFormatsKHR", {kFuncTypePdev, (void*)GetPhysicalDeviceSurfaceFormatsKHR}},
    {"vkGetPhysicalDeviceSurfacePresentModesKHR", {kFuncTypePdev, (void*)GetPhysicalDeviceSurfacePresentModesKHR}},
    {"vkCreateSwapchainKHR", {kFuncTypeDev, (void*)CreateSwapchainKHR}},
    {"vkDestroySwapchainKHR", {kFuncTypeDev, (void*)DestroySwapchainKHR}},
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
#ifdef VK_USE_PLATFORM_XLIB_KHR
    {"vkCreateXlibSurfaceKHR", {kFuncTypeInst, (void*)CreateXlibSurfaceKHR}},
#endif
#ifdef VK_USE_PLATFORM_XLIB_KHR
    {"vkGetPhysicalDeviceXlibPresentationSupportKHR", {kFuncTypePdev, (void*)GetPhysicalDeviceXlibPresentationSupportKHR}},
#endif
#ifdef VK_USE_PLATFORM_XCB_KHR
    {"vkCreateXcbSurfaceKHR", {kFuncTypeInst, (void*)CreateXcbSurfaceKHR}},
#endif
#ifdef VK_USE_PLATFORM_XCB_KHR
    {"vkGetPhysicalDeviceXcbPresentationSupportKHR", {kFuncTypePdev, (void*)GetPhysicalDeviceXcbPresentationSupportKHR}},
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    {"vkCreateWaylandSurfaceKHR", {kFuncTypeInst, (void*)CreateWaylandSurfaceKHR}},
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    {"vkGetPhysicalDeviceWaylandPresentationSupportKHR", {kFuncTypePdev, (void*)GetPhysicalDeviceWaylandPresentationSupportKHR}},
#endif
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    {"vkCreateAndroidSurfaceKHR", {kFuncTypeInst, (void*)CreateAndroidSurfaceKHR}},
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
    {"vkCreateWin32SurfaceKHR", {kFuncTypeInst, (void*)CreateWin32SurfaceKHR}},
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
    {"vkGetPhysicalDeviceWin32PresentationSupportKHR", {kFuncTypePdev, (void*)GetPhysicalDeviceWin32PresentationSupportKHR}},
#endif
    {"vkGetPhysicalDeviceFeatures2KHR", {kFuncTypePdev, (void*)GetPhysicalDeviceFeatures2KHR}},
    {"vkGetPhysicalDeviceProperties2KHR", {kFuncTypePdev, (void*)GetPhysicalDeviceProperties2KHR}},
    {"vkGetPhysicalDeviceFormatProperties2KHR", {kFuncTypePdev, (void*)GetPhysicalDeviceFormatProperties2KHR}},
    {"vkGetPhysicalDeviceImageFormatProperties2KHR", {kFuncTypePdev, (void*)GetPhysicalDeviceImageFormatProperties2KHR}},
    {"vkGetPhysicalDeviceQueueFamilyProperties2KHR", {kFuncTypePdev, (void*)GetPhysicalDeviceQueueFamilyProperties2KHR}},
    {"vkGetPhysicalDeviceMemoryProperties2KHR", {kFuncTypePdev, (void*)GetPhysicalDeviceMemoryProperties2KHR}},
    {"vkGetPhysicalDeviceSparseImageFormatProperties2KHR", {kFuncTypePdev, (void*)GetPhysicalDeviceSparseImageFormatProperties2KHR}},
    {"vkGetDeviceGroupPeerMemoryFeaturesKHR", {kFuncTypeDev, (void*)GetDeviceGroupPeerMemoryFeaturesKHR}},
    {"vkCmdSetDeviceMaskKHR", {kFuncTypeDev, (void*)CmdSetDeviceMaskKHR}},
    {"vkCmdDispatchBaseKHR", {kFuncTypeDev, (void*)CmdDispatchBaseKHR}},
    {"vkTrimCommandPoolKHR", {kFuncTypeDev, (void*)TrimCommandPoolKHR}},
    {"vkEnumeratePhysicalDeviceGroupsKHR", {kFuncTypeInst, (void*)EnumeratePhysicalDeviceGroupsKHR}},
    {"vkGetPhysicalDeviceExternalBufferPropertiesKHR", {kFuncTypePdev, (void*)GetPhysicalDeviceExternalBufferPropertiesKHR}},
#ifdef VK_USE_PLATFORM_WIN32_KHR
    {"vkGetMemoryWin32HandleKHR", {kFuncTypeDev, (void*)GetMemoryWin32HandleKHR}},
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
    {"vkGetMemoryWin32HandlePropertiesKHR", {kFuncTypeDev, (void*)GetMemoryWin32HandlePropertiesKHR}},
#endif
    {"vkGetMemoryFdKHR", {kFuncTypeDev, (void*)GetMemoryFdKHR}},
    {"vkGetMemoryFdPropertiesKHR", {kFuncTypeDev, (void*)GetMemoryFdPropertiesKHR}},
    {"vkGetPhysicalDeviceExternalSemaphorePropertiesKHR", {kFuncTypePdev, (void*)GetPhysicalDeviceExternalSemaphorePropertiesKHR}},
#ifdef VK_USE_PLATFORM_WIN32_KHR
    {"vkImportSemaphoreWin32HandleKHR", {kFuncTypeDev, (void*)ImportSemaphoreWin32HandleKHR}},
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
    {"vkGetSemaphoreWin32HandleKHR", {kFuncTypeDev, (void*)GetSemaphoreWin32HandleKHR}},
#endif
    {"vkImportSemaphoreFdKHR", {kFuncTypeDev, (void*)ImportSemaphoreFdKHR}},
    {"vkGetSemaphoreFdKHR", {kFuncTypeDev, (void*)GetSemaphoreFdKHR}},
    {"vkCmdPushDescriptorSetKHR", {kFuncTypeDev, (void*)CmdPushDescriptorSetKHR}},
    {"vkCmdPushDescriptorSetWithTemplateKHR", {kFuncTypeDev, (void*)CmdPushDescriptorSetWithTemplateKHR}},
    {"vkCreateDescriptorUpdateTemplateKHR", {kFuncTypeDev, (void*)CreateDescriptorUpdateTemplateKHR}},
    {"vkDestroyDescriptorUpdateTemplateKHR", {kFuncTypeDev, (void*)DestroyDescriptorUpdateTemplateKHR}},
    {"vkUpdateDescriptorSetWithTemplateKHR", {kFuncTypeDev, (void*)UpdateDescriptorSetWithTemplateKHR}},
    {"vkCreateRenderPass2KHR", {kFuncTypeDev, (void*)CreateRenderPass2KHR}},
    {"vkCmdBeginRenderPass2KHR", {kFuncTypeDev, (void*)CmdBeginRenderPass2KHR}},
    {"vkCmdNextSubpass2KHR", {kFuncTypeDev, (void*)CmdNextSubpass2KHR}},
    {"vkCmdEndRenderPass2KHR", {kFuncTypeDev, (void*)CmdEndRenderPass2KHR}},
    {"vkGetSwapchainStatusKHR", {kFuncTypeDev, (void*)GetSwapchainStatusKHR}},
    {"vkGetPhysicalDeviceExternalFencePropertiesKHR", {kFuncTypePdev, (void*)GetPhysicalDeviceExternalFencePropertiesKHR}},
#ifdef VK_USE_PLATFORM_WIN32_KHR
    {"vkImportFenceWin32HandleKHR", {kFuncTypeDev, (void*)ImportFenceWin32HandleKHR}},
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
    {"vkGetFenceWin32HandleKHR", {kFuncTypeDev, (void*)GetFenceWin32HandleKHR}},
#endif
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
    {"vkGetImageMemoryRequirements2KHR", {kFuncTypeDev, (void*)GetImageMemoryRequirements2KHR}},
    {"vkGetBufferMemoryRequirements2KHR", {kFuncTypeDev, (void*)GetBufferMemoryRequirements2KHR}},
    {"vkGetImageSparseMemoryRequirements2KHR", {kFuncTypeDev, (void*)GetImageSparseMemoryRequirements2KHR}},
    {"vkCreateSamplerYcbcrConversionKHR", {kFuncTypeDev, (void*)CreateSamplerYcbcrConversionKHR}},
    {"vkDestroySamplerYcbcrConversionKHR", {kFuncTypeDev, (void*)DestroySamplerYcbcrConversionKHR}},
    {"vkBindBufferMemory2KHR", {kFuncTypeDev, (void*)BindBufferMemory2KHR}},
    {"vkBindImageMemory2KHR", {kFuncTypeDev, (void*)BindImageMemory2KHR}},
    {"vkGetDescriptorSetLayoutSupportKHR", {kFuncTypeDev, (void*)GetDescriptorSetLayoutSupportKHR}},
    {"vkCmdDrawIndirectCountKHR", {kFuncTypeDev, (void*)CmdDrawIndirectCountKHR}},
    {"vkCmdDrawIndexedIndirectCountKHR", {kFuncTypeDev, (void*)CmdDrawIndexedIndirectCountKHR}},
    {"vkGetSemaphoreCounterValueKHR", {kFuncTypeDev, (void*)GetSemaphoreCounterValueKHR}},
    {"vkWaitSemaphoresKHR", {kFuncTypeDev, (void*)WaitSemaphoresKHR}},
    {"vkSignalSemaphoreKHR", {kFuncTypeDev, (void*)SignalSemaphoreKHR}},
    {"vkGetPhysicalDeviceFragmentShadingRatesKHR", {kFuncTypePdev, (void*)GetPhysicalDeviceFragmentShadingRatesKHR}},
    {"vkCmdSetFragmentShadingRateKHR", {kFuncTypeDev, (void*)CmdSetFragmentShadingRateKHR}},
    {"vkGetBufferDeviceAddressKHR", {kFuncTypeDev, (void*)GetBufferDeviceAddressKHR}},
    {"vkGetBufferOpaqueCaptureAddressKHR", {kFuncTypeDev, (void*)GetBufferOpaqueCaptureAddressKHR}},
    {"vkGetDeviceMemoryOpaqueCaptureAddressKHR", {kFuncTypeDev, (void*)GetDeviceMemoryOpaqueCaptureAddressKHR}},
    {"vkCreateDeferredOperationKHR", {kFuncTypeDev, (void*)CreateDeferredOperationKHR}},
    {"vkDestroyDeferredOperationKHR", {kFuncTypeDev, (void*)DestroyDeferredOperationKHR}},
    {"vkGetDeferredOperationMaxConcurrencyKHR", {kFuncTypeDev, (void*)GetDeferredOperationMaxConcurrencyKHR}},
    {"vkGetDeferredOperationResultKHR", {kFuncTypeDev, (void*)GetDeferredOperationResultKHR}},
    {"vkDeferredOperationJoinKHR", {kFuncTypeDev, (void*)DeferredOperationJoinKHR}},
    {"vkGetPipelineExecutablePropertiesKHR", {kFuncTypeDev, (void*)GetPipelineExecutablePropertiesKHR}},
    {"vkGetPipelineExecutableStatisticsKHR", {kFuncTypeDev, (void*)GetPipelineExecutableStatisticsKHR}},
    {"vkGetPipelineExecutableInternalRepresentationsKHR", {kFuncTypeDev, (void*)GetPipelineExecutableInternalRepresentationsKHR}},
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
    {"vkCreateDebugReportCallbackEXT", {kFuncTypeInst, (void*)CreateDebugReportCallbackEXT}},
    {"vkDestroyDebugReportCallbackEXT", {kFuncTypeInst, (void*)DestroyDebugReportCallbackEXT}},
    {"vkDebugReportMessageEXT", {kFuncTypeInst, (void*)DebugReportMessageEXT}},
    {"vkDebugMarkerSetObjectTagEXT", {kFuncTypeDev, (void*)DebugMarkerSetObjectTagEXT}},
    {"vkDebugMarkerSetObjectNameEXT", {kFuncTypeDev, (void*)DebugMarkerSetObjectNameEXT}},
    {"vkCmdDebugMarkerBeginEXT", {kFuncTypeDev, (void*)CmdDebugMarkerBeginEXT}},
    {"vkCmdDebugMarkerEndEXT", {kFuncTypeDev, (void*)CmdDebugMarkerEndEXT}},
    {"vkCmdDebugMarkerInsertEXT", {kFuncTypeDev, (void*)CmdDebugMarkerInsertEXT}},
    {"vkCmdBindTransformFeedbackBuffersEXT", {kFuncTypeDev, (void*)CmdBindTransformFeedbackBuffersEXT}},
    {"vkCmdBeginTransformFeedbackEXT", {kFuncTypeDev, (void*)CmdBeginTransformFeedbackEXT}},
    {"vkCmdEndTransformFeedbackEXT", {kFuncTypeDev, (void*)CmdEndTransformFeedbackEXT}},
    {"vkCmdBeginQueryIndexedEXT", {kFuncTypeDev, (void*)CmdBeginQueryIndexedEXT}},
    {"vkCmdEndQueryIndexedEXT", {kFuncTypeDev, (void*)CmdEndQueryIndexedEXT}},
    {"vkCmdDrawIndirectByteCountEXT", {kFuncTypeDev, (void*)CmdDrawIndirectByteCountEXT}},
    {"vkGetImageViewHandleNVX", {kFuncTypeDev, (void*)GetImageViewHandleNVX}},
    {"vkGetImageViewAddressNVX", {kFuncTypeDev, (void*)GetImageViewAddressNVX}},
    {"vkCmdDrawIndirectCountAMD", {kFuncTypeDev, (void*)CmdDrawIndirectCountAMD}},
    {"vkCmdDrawIndexedIndirectCountAMD", {kFuncTypeDev, (void*)CmdDrawIndexedIndirectCountAMD}},
    {"vkGetShaderInfoAMD", {kFuncTypeDev, (void*)GetShaderInfoAMD}},
#ifdef VK_USE_PLATFORM_GGP
    {"vkCreateStreamDescriptorSurfaceGGP", {kFuncTypeInst, (void*)CreateStreamDescriptorSurfaceGGP}},
#endif
    {"vkGetPhysicalDeviceExternalImageFormatPropertiesNV", {kFuncTypePdev, (void*)GetPhysicalDeviceExternalImageFormatPropertiesNV}},
#ifdef VK_USE_PLATFORM_WIN32_KHR
    {"vkGetMemoryWin32HandleNV", {kFuncTypeDev, (void*)GetMemoryWin32HandleNV}},
#endif
#ifdef VK_USE_PLATFORM_VI_NN
    {"vkCreateViSurfaceNN", {kFuncTypeInst, (void*)CreateViSurfaceNN}},
#endif
    {"vkCmdBeginConditionalRenderingEXT", {kFuncTypeDev, (void*)CmdBeginConditionalRenderingEXT}},
    {"vkCmdEndConditionalRenderingEXT", {kFuncTypeDev, (void*)CmdEndConditionalRenderingEXT}},
    {"vkCmdSetViewportWScalingNV", {kFuncTypeDev, (void*)CmdSetViewportWScalingNV}},
    {"vkReleaseDisplayEXT", {kFuncTypePdev, (void*)ReleaseDisplayEXT}},
#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT
    {"vkAcquireXlibDisplayEXT", {kFuncTypePdev, (void*)AcquireXlibDisplayEXT}},
#endif
#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT
    {"vkGetRandROutputDisplayEXT", {kFuncTypePdev, (void*)GetRandROutputDisplayEXT}},
#endif
    {"vkGetPhysicalDeviceSurfaceCapabilities2EXT", {kFuncTypePdev, (void*)GetPhysicalDeviceSurfaceCapabilities2EXT}},
    {"vkDisplayPowerControlEXT", {kFuncTypeDev, (void*)DisplayPowerControlEXT}},
    {"vkRegisterDeviceEventEXT", {kFuncTypeDev, (void*)RegisterDeviceEventEXT}},
    {"vkRegisterDisplayEventEXT", {kFuncTypeDev, (void*)RegisterDisplayEventEXT}},
    {"vkGetSwapchainCounterEXT", {kFuncTypeDev, (void*)GetSwapchainCounterEXT}},
    {"vkGetRefreshCycleDurationGOOGLE", {kFuncTypeDev, (void*)GetRefreshCycleDurationGOOGLE}},
    {"vkGetPastPresentationTimingGOOGLE", {kFuncTypeDev, (void*)GetPastPresentationTimingGOOGLE}},
    {"vkCmdSetDiscardRectangleEXT", {kFuncTypeDev, (void*)CmdSetDiscardRectangleEXT}},
    {"vkSetHdrMetadataEXT", {kFuncTypeDev, (void*)SetHdrMetadataEXT}},
#ifdef VK_USE_PLATFORM_IOS_MVK
    {"vkCreateIOSSurfaceMVK", {kFuncTypeInst, (void*)CreateIOSSurfaceMVK}},
#endif
#ifdef VK_USE_PLATFORM_MACOS_MVK
    {"vkCreateMacOSSurfaceMVK", {kFuncTypeInst, (void*)CreateMacOSSurfaceMVK}},
#endif
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
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    {"vkGetAndroidHardwareBufferPropertiesANDROID", {kFuncTypeDev, (void*)GetAndroidHardwareBufferPropertiesANDROID}},
#endif
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    {"vkGetMemoryAndroidHardwareBufferANDROID", {kFuncTypeDev, (void*)GetMemoryAndroidHardwareBufferANDROID}},
#endif
    {"vkCmdSetSampleLocationsEXT", {kFuncTypeDev, (void*)CmdSetSampleLocationsEXT}},
    {"vkGetPhysicalDeviceMultisamplePropertiesEXT", {kFuncTypePdev, (void*)GetPhysicalDeviceMultisamplePropertiesEXT}},
    {"vkGetImageDrmFormatModifierPropertiesEXT", {kFuncTypeDev, (void*)GetImageDrmFormatModifierPropertiesEXT}},
    {"vkCreateValidationCacheEXT", {kFuncTypeDev, (void*)CreateValidationCacheEXT}},
    {"vkDestroyValidationCacheEXT", {kFuncTypeDev, (void*)DestroyValidationCacheEXT}},
    {"vkMergeValidationCachesEXT", {kFuncTypeDev, (void*)MergeValidationCachesEXT}},
    {"vkGetValidationCacheDataEXT", {kFuncTypeDev, (void*)GetValidationCacheDataEXT}},
    {"vkCmdBindShadingRateImageNV", {kFuncTypeDev, (void*)CmdBindShadingRateImageNV}},
    {"vkCmdSetViewportShadingRatePaletteNV", {kFuncTypeDev, (void*)CmdSetViewportShadingRatePaletteNV}},
    {"vkCmdSetCoarseSampleOrderNV", {kFuncTypeDev, (void*)CmdSetCoarseSampleOrderNV}},
    {"vkCreateAccelerationStructureNV", {kFuncTypeDev, (void*)CreateAccelerationStructureNV}},
    {"vkDestroyAccelerationStructureNV", {kFuncTypeDev, (void*)DestroyAccelerationStructureNV}},
    {"vkGetAccelerationStructureMemoryRequirementsNV", {kFuncTypeDev, (void*)GetAccelerationStructureMemoryRequirementsNV}},
    {"vkBindAccelerationStructureMemoryNV", {kFuncTypeDev, (void*)BindAccelerationStructureMemoryNV}},
    {"vkCmdBuildAccelerationStructureNV", {kFuncTypeDev, (void*)CmdBuildAccelerationStructureNV}},
    {"vkCmdCopyAccelerationStructureNV", {kFuncTypeDev, (void*)CmdCopyAccelerationStructureNV}},
    {"vkCmdTraceRaysNV", {kFuncTypeDev, (void*)CmdTraceRaysNV}},
    {"vkCreateRayTracingPipelinesNV", {kFuncTypeDev, (void*)CreateRayTracingPipelinesNV}},
    {"vkGetRayTracingShaderGroupHandlesKHR", {kFuncTypeDev, (void*)GetRayTracingShaderGroupHandlesKHR}},
    {"vkGetRayTracingShaderGroupHandlesNV", {kFuncTypeDev, (void*)GetRayTracingShaderGroupHandlesNV}},
    {"vkGetAccelerationStructureHandleNV", {kFuncTypeDev, (void*)GetAccelerationStructureHandleNV}},
    {"vkCmdWriteAccelerationStructuresPropertiesNV", {kFuncTypeDev, (void*)CmdWriteAccelerationStructuresPropertiesNV}},
    {"vkCompileDeferredNV", {kFuncTypeDev, (void*)CompileDeferredNV}},
    {"vkGetMemoryHostPointerPropertiesEXT", {kFuncTypeDev, (void*)GetMemoryHostPointerPropertiesEXT}},
    {"vkCmdWriteBufferMarkerAMD", {kFuncTypeDev, (void*)CmdWriteBufferMarkerAMD}},
    {"vkGetPhysicalDeviceCalibrateableTimeDomainsEXT", {kFuncTypePdev, (void*)GetPhysicalDeviceCalibrateableTimeDomainsEXT}},
    {"vkGetCalibratedTimestampsEXT", {kFuncTypeDev, (void*)GetCalibratedTimestampsEXT}},
    {"vkCmdDrawMeshTasksNV", {kFuncTypeDev, (void*)CmdDrawMeshTasksNV}},
    {"vkCmdDrawMeshTasksIndirectNV", {kFuncTypeDev, (void*)CmdDrawMeshTasksIndirectNV}},
    {"vkCmdDrawMeshTasksIndirectCountNV", {kFuncTypeDev, (void*)CmdDrawMeshTasksIndirectCountNV}},
    {"vkCmdSetExclusiveScissorNV", {kFuncTypeDev, (void*)CmdSetExclusiveScissorNV}},
    {"vkCmdSetCheckpointNV", {kFuncTypeDev, (void*)CmdSetCheckpointNV}},
    {"vkGetQueueCheckpointDataNV", {kFuncTypeDev, (void*)GetQueueCheckpointDataNV}},
    {"vkInitializePerformanceApiINTEL", {kFuncTypeDev, (void*)InitializePerformanceApiINTEL}},
    {"vkUninitializePerformanceApiINTEL", {kFuncTypeDev, (void*)UninitializePerformanceApiINTEL}},
    {"vkCmdSetPerformanceMarkerINTEL", {kFuncTypeDev, (void*)CmdSetPerformanceMarkerINTEL}},
    {"vkCmdSetPerformanceStreamMarkerINTEL", {kFuncTypeDev, (void*)CmdSetPerformanceStreamMarkerINTEL}},
    {"vkCmdSetPerformanceOverrideINTEL", {kFuncTypeDev, (void*)CmdSetPerformanceOverrideINTEL}},
    {"vkAcquirePerformanceConfigurationINTEL", {kFuncTypeDev, (void*)AcquirePerformanceConfigurationINTEL}},
    {"vkReleasePerformanceConfigurationINTEL", {kFuncTypeDev, (void*)ReleasePerformanceConfigurationINTEL}},
    {"vkQueueSetPerformanceConfigurationINTEL", {kFuncTypeDev, (void*)QueueSetPerformanceConfigurationINTEL}},
    {"vkGetPerformanceParameterINTEL", {kFuncTypeDev, (void*)GetPerformanceParameterINTEL}},
    {"vkSetLocalDimmingAMD", {kFuncTypeDev, (void*)SetLocalDimmingAMD}},
#ifdef VK_USE_PLATFORM_FUCHSIA
    {"vkCreateImagePipeSurfaceFUCHSIA", {kFuncTypeInst, (void*)CreateImagePipeSurfaceFUCHSIA}},
#endif
#ifdef VK_USE_PLATFORM_METAL_EXT
    {"vkCreateMetalSurfaceEXT", {kFuncTypeInst, (void*)CreateMetalSurfaceEXT}},
#endif
    {"vkGetBufferDeviceAddressEXT", {kFuncTypeDev, (void*)GetBufferDeviceAddressEXT}},
    {"vkGetPhysicalDeviceToolPropertiesEXT", {kFuncTypePdev, (void*)GetPhysicalDeviceToolPropertiesEXT}},
    {"vkGetPhysicalDeviceCooperativeMatrixPropertiesNV", {kFuncTypePdev, (void*)GetPhysicalDeviceCooperativeMatrixPropertiesNV}},
    {"vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV", {kFuncTypePdev, (void*)GetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV}},
#ifdef VK_USE_PLATFORM_WIN32_KHR
    {"vkGetPhysicalDeviceSurfacePresentModes2EXT", {kFuncTypePdev, (void*)GetPhysicalDeviceSurfacePresentModes2EXT}},
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
    {"vkAcquireFullScreenExclusiveModeEXT", {kFuncTypeDev, (void*)AcquireFullScreenExclusiveModeEXT}},
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
    {"vkReleaseFullScreenExclusiveModeEXT", {kFuncTypeDev, (void*)ReleaseFullScreenExclusiveModeEXT}},
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
    {"vkGetDeviceGroupSurfacePresentModes2EXT", {kFuncTypeDev, (void*)GetDeviceGroupSurfacePresentModes2EXT}},
#endif
    {"vkCreateHeadlessSurfaceEXT", {kFuncTypeInst, (void*)CreateHeadlessSurfaceEXT}},
    {"vkCmdSetLineStippleEXT", {kFuncTypeDev, (void*)CmdSetLineStippleEXT}},
    {"vkResetQueryPoolEXT", {kFuncTypeDev, (void*)ResetQueryPoolEXT}},
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
    {"vkGetGeneratedCommandsMemoryRequirementsNV", {kFuncTypeDev, (void*)GetGeneratedCommandsMemoryRequirementsNV}},
    {"vkCmdPreprocessGeneratedCommandsNV", {kFuncTypeDev, (void*)CmdPreprocessGeneratedCommandsNV}},
    {"vkCmdExecuteGeneratedCommandsNV", {kFuncTypeDev, (void*)CmdExecuteGeneratedCommandsNV}},
    {"vkCmdBindPipelineShaderGroupNV", {kFuncTypeDev, (void*)CmdBindPipelineShaderGroupNV}},
    {"vkCreateIndirectCommandsLayoutNV", {kFuncTypeDev, (void*)CreateIndirectCommandsLayoutNV}},
    {"vkDestroyIndirectCommandsLayoutNV", {kFuncTypeDev, (void*)DestroyIndirectCommandsLayoutNV}},
    {"vkCreatePrivateDataSlotEXT", {kFuncTypeDev, (void*)CreatePrivateDataSlotEXT}},
    {"vkDestroyPrivateDataSlotEXT", {kFuncTypeDev, (void*)DestroyPrivateDataSlotEXT}},
    {"vkSetPrivateDataEXT", {kFuncTypeDev, (void*)SetPrivateDataEXT}},
    {"vkGetPrivateDataEXT", {kFuncTypeDev, (void*)GetPrivateDataEXT}},
    {"vkCmdSetFragmentShadingRateEnumNV", {kFuncTypeDev, (void*)CmdSetFragmentShadingRateEnumNV}},
#ifdef VK_USE_PLATFORM_WIN32_KHR
    {"vkAcquireWinrtDisplayNV", {kFuncTypePdev, (void*)AcquireWinrtDisplayNV}},
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
    {"vkGetWinrtDisplayNV", {kFuncTypePdev, (void*)GetWinrtDisplayNV}},
#endif
#ifdef VK_USE_PLATFORM_DIRECTFB_EXT
    {"vkCreateDirectFBSurfaceEXT", {kFuncTypeInst, (void*)CreateDirectFBSurfaceEXT}},
#endif
#ifdef VK_USE_PLATFORM_DIRECTFB_EXT
    {"vkGetPhysicalDeviceDirectFBPresentationSupportEXT", {kFuncTypePdev, (void*)GetPhysicalDeviceDirectFBPresentationSupportEXT}},
#endif
    {"vkCreateAccelerationStructureKHR", {kFuncTypeDev, (void*)CreateAccelerationStructureKHR}},
    {"vkDestroyAccelerationStructureKHR", {kFuncTypeDev, (void*)DestroyAccelerationStructureKHR}},
    {"vkCmdBuildAccelerationStructuresKHR", {kFuncTypeDev, (void*)CmdBuildAccelerationStructuresKHR}},
    {"vkCmdBuildAccelerationStructuresIndirectKHR", {kFuncTypeDev, (void*)CmdBuildAccelerationStructuresIndirectKHR}},
    {"vkBuildAccelerationStructuresKHR", {kFuncTypeDev, (void*)BuildAccelerationStructuresKHR}},
    {"vkCopyAccelerationStructureKHR", {kFuncTypeDev, (void*)CopyAccelerationStructureKHR}},
    {"vkCopyAccelerationStructureToMemoryKHR", {kFuncTypeDev, (void*)CopyAccelerationStructureToMemoryKHR}},
    {"vkCopyMemoryToAccelerationStructureKHR", {kFuncTypeDev, (void*)CopyMemoryToAccelerationStructureKHR}},
    {"vkWriteAccelerationStructuresPropertiesKHR", {kFuncTypeDev, (void*)WriteAccelerationStructuresPropertiesKHR}},
    {"vkCmdCopyAccelerationStructureKHR", {kFuncTypeDev, (void*)CmdCopyAccelerationStructureKHR}},
    {"vkCmdCopyAccelerationStructureToMemoryKHR", {kFuncTypeDev, (void*)CmdCopyAccelerationStructureToMemoryKHR}},
    {"vkCmdCopyMemoryToAccelerationStructureKHR", {kFuncTypeDev, (void*)CmdCopyMemoryToAccelerationStructureKHR}},
    {"vkGetAccelerationStructureDeviceAddressKHR", {kFuncTypeDev, (void*)GetAccelerationStructureDeviceAddressKHR}},
    {"vkCmdWriteAccelerationStructuresPropertiesKHR", {kFuncTypeDev, (void*)CmdWriteAccelerationStructuresPropertiesKHR}},
    {"vkGetDeviceAccelerationStructureCompatibilityKHR", {kFuncTypeDev, (void*)GetDeviceAccelerationStructureCompatibilityKHR}},
    {"vkGetAccelerationStructureBuildSizesKHR", {kFuncTypeDev, (void*)GetAccelerationStructureBuildSizesKHR}},
    {"vkCmdTraceRaysKHR", {kFuncTypeDev, (void*)CmdTraceRaysKHR}},
    {"vkCreateRayTracingPipelinesKHR", {kFuncTypeDev, (void*)CreateRayTracingPipelinesKHR}},
    {"vkGetRayTracingCaptureReplayShaderGroupHandlesKHR", {kFuncTypeDev, (void*)GetRayTracingCaptureReplayShaderGroupHandlesKHR}},
    {"vkCmdTraceRaysIndirectKHR", {kFuncTypeDev, (void*)CmdTraceRaysIndirectKHR}},
    {"vkGetRayTracingShaderGroupStackSizeKHR", {kFuncTypeDev, (void*)GetRayTracingShaderGroupStackSizeKHR}},
    {"vkCmdSetRayTracingPipelineStackSizeKHR", {kFuncTypeDev, (void*)CmdSetRayTracingPipelineStackSizeKHR}},
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
